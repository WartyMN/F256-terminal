/*
 * serial.c
 *
 *  Created on: Apr 1, 2024
 *      Author: micahbly
 *
 *  - adapted from my f/manager F256 project's general.c routines on serial debugging, expanded to be basis of ANSI terminal emulator
 */


/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// project includes
#include "app.h"
#include "comm_buffer.h"
#include "debug.h"
#include "general.h"
#include "memory.h"
#include "screen.h"
#include "serial.h"
#include "strings.h"
#include "sys.h"
#include "text.h"

// C includes
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// F256 includes
#include "f256.h"
#include "kernel.h"


/*****************************************************************************/
/*                               Definitions                                 */
/*****************************************************************************/

#define TERMINAL_DEFAULT_BACK_COLOR		ANSI_COLOR_BLACK	// defined by ANSI. do not change.
#define TERMINAL_DEFAULT_FORE_COLOR		ANSI_COLOR_WHITE	// defined by ANSI. do not change.

#define UART_MAX_SEND_ATTEMPTS	1000
#define ANSI_MAX_SEQUENCE_LEN	128

#define ANSI_FUNCTION_CUU			'A'		// Cursor Up
#define ANSI_FUNCTION_CUD			'B'		// Cursor Down
#define ANSI_FUNCTION_CUF			'C'		// Cursor Forward
#define ANSI_FUNCTION_CUB			'D'		// Cursor Back
#define ANSI_FUNCTION_CNL			'E'		// Cursor Next Line
#define ANSI_FUNCTION_CPL			'F'		// Cursor Previous Line
#define ANSI_FUNCTION_CHA			'G'		// Cursor Horizontal Absolute
#define ANSI_FUNCTION_CUP			'H'		// Cursor Position
#define ANSI_FUNCTION_ED			'J'		// Erase in Display
#define ANSI_FUNCTION_EL			'K'		// Erase in Line
#define ANSI_FUNCTION_SU			'S'		// Scroll Up (page)
#define ANSI_FUNCTION_SD			'T'		// Scroll Down (page)
#define ANSI_FUNCTION_HVP			'f'		// Horizontal Vertical Position
#define ANSI_FUNCTION_SGR			'm'		// Select Graphic Rendition
#define ANSI_FUNCTION_DSR			'n'		// Device Status Report
#define ANSI_FUNCTION_CLEAR			'U'		//  Clear the screen with the "normal" attribute and home the cursor
#define ANSI_FUNCTION_SAVECURPOS	's'		// save current cursor position
#define ANSI_FUNCTION_RESTORECURPOS	'u'		// restore cursor position from last saven
#define ANSI_FUNCTION_PRIVHIDEMOUSE	'h'		// ?1000h is a private ANSI combo for "hide mouse pointer"
#define ANSI_FUNCTION_PRIVSHOWMOUSE	'l'		// ?1000l is a private ANSI combo for "show mouse pointer"



/*****************************************************************************/
/*                          File-scoped Variables                            */
/*****************************************************************************/

static uint8_t			ansi_sequence_storage[ANSI_MAX_SEQUENCE_LEN + 1];
static uint8_t*			ansi_sequence = ansi_sequence_storage;
static uint8_t			ansi_phase = 0;	// 0 = not started; 1=ESC, 2=bracket (full on), 3=done
static bool				ansi_bold_mode = false;	// need to track bold mode between SGR commands as well as within one

static uint8_t			serial_x;	// text coords need to maintained separately from
static uint8_t			serial_y;	//  global text engine because buffer update/etc will affect global ones
static uint8_t			serial_save_x;	// in case BBS instructs save cursor pos
static uint8_t			serial_save_y;	// in case BBS instructs save cursor pos

static uint8_t			serial_fg_color = TERMINAL_DEFAULT_FORE_COLOR;
static uint8_t			serial_bg_color = TERMINAL_DEFAULT_BACK_COLOR;
static uint8_t			serial_current_pref_color = ANSI_COLOR_BRIGHT_RED;			// user's preferred foreground color. ANSI will override.

// F256JR/K colors, used for both fore- and background colors in Text mode
// in C256 & F256, these are 8 bit values; in A2560s, they are 32 bit values, and endianness matters
const static uint8_t ansi_text_color_lut[64] = 
{
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xAA, 0x00,			
	0x00, 0xAA, 0x00, 0x00,
	0x00, 0x55, 0xAA, 0x00,
	0xAA, 0x00, 0x00, 0x00,
	0xAA, 0x00, 0xAA, 0x00,
	0xAA, 0xAA, 0x00, 0x00,
	0xAA, 0xAA, 0xAA, 0x00,

	0x55, 0x55, 0x55, 0x00,
	0x55, 0x55, 0xFF, 0x00,
	0x55, 0xFF, 0x55, 0x00,
	0x55, 0xFF, 0xFF, 0x00,
	0xFF, 0x55, 0x55, 0x00,
	0xFF, 0x55, 0xFF, 0x00,
	0xFF, 0xFF, 0x55, 0x00,
	0xFF, 0xFF, 0xFF, 0x00,
};

/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/

uint8_t*				global_uart_in_buffer = (uint8_t*)UART_BUFFER_START_ADDR;
uint16_t				global_uart_write_idx;
uint16_t				global_uart_read_idx;

extern char*			global_string_buff1;
extern char*			global_string_buff2;


extern uint8_t			zp_bank_num;
extern uint8_t			zp_x;
extern uint8_t			zp_y;
#pragma zpsym ("zp_bank_num");
#pragma zpsym ("zp_x");
#pragma zpsym ("zp_y");


/*****************************************************************************/
/*                       Private Function Prototypes                         */
/*****************************************************************************/

// set UART chip to DLAB mode
void Serial_SetDLAB(void);

// turn off DLAB mode on UART chip
void Serial_ClearDLAB(void);

// process a byte from the serial port, including checking for ANSI sequences and printing to screen
void Serial_ProcessByte(uint8_t the_byte);

// print a byte to screen, from the serial port
void Serial_PrintByte(uint8_t the_byte);

// process the ANSI sequence stored in ansi_sequence_storage
void Serial_ProcessANSI(void);

// Moves the cursor n (default 1) cells in the given direction.
// If the cursor is already at the edge of the screen, this has no effect.
void Serial_ANSICursorUp(uint8_t the_count);

// Moves the cursor n (default 1) cells in the given direction.
// If the cursor is already at the edge of the screen, this has no effect.
void Serial_ANSICursorDown(uint8_t the_count);

// Moves the cursor n (default 1) cells in the given direction.
// If the cursor is already at the edge of the screen, this has no effect.
void Serial_ANSICursorLeft(uint8_t the_count);

// Moves the cursor n (default 1) cells in the given direction.
// If the cursor is already at the edge of the screen, this has no effect.
void Serial_ANSICursorRight(uint8_t the_count);

// Moves cursor to beginning of the line n (default 1) lines down.
void Serial_ANSICursorNextLine(uint8_t the_count);

// scrolls down one line or an entire page
void Serial_ANSIScrollDown(bool scroll_page);

// Moves cursor to beginning of the line n (default 1) lines up.
void Serial_ANSICursorPreviousLine(uint8_t the_count);

// ANSI CHA
// Moves the cursor to column n (default 1)
void Serial_ANSICursorSetXPos(uint8_t the_count);

// ANSI CUP: CSI n ; m H
// Moves the cursor to row n, column m. 
// The values are 1-based, and default to 1 (top left corner) if omitted. 
// A sequence such as CSI ;5H is a synonym for CSI 1;5H
//   CSI 17;H is the same as CSI 17H and CSI 17;1H
void Serial_ANSICursorSetXYPos();

// ANSI HVP: CSI n ; m f
// Moves the cursor to row n, column m. 
// Same as CUP, but counts as a format effector function (like CR or LF) rather than an editor function (like CUD or CNL). 
// The values are 1-based, and default to 1 (top left corner) if omitted. 
void Serial_ANSICursorMoveToXY();

// ANSI clear
// clears the screen setting attributs to normal. homes the cursor
void Serial_ANSIClear(void);

// ANSI ED - Erase in Display
// Clears part of the screen. 
//   If n is 0 (or missing), clear from cursor to end of screen. 
//   If n is 1, clear from cursor to beginning of the screen. 
//   If n is 2, clear entire screen (and moves cursor to upper left on DOS ANSI.SYS).
//   If n is 3, clear entire screen and delete all lines saved in the scrollback buffer
void Serial_ANSIEraseInDisplay(uint8_t the_count);

// ANSI EL - Erase in Line
// Erases part of the line. 
//   If n is 0 (or missing), clear from cursor to the end of the line.
//   If n is 1, clear from cursor to beginning of the line. 
//   If n is 2, clear entire line.
//   Cursor position does not change.
void Serial_ANSIEraseInLine(uint8_t the_count);

// Device Status Report - 6n
// when requested by host computer, we need to respond with info about our terminal
// ESC[n;mR, where n is the row and m is the column.
// unofficial 255n seems to be used to send back terminal size. e.g, 24;80
void Serial_ANSISendDSR(uint8_t the_count);

// ANSI function handler for SGR: Select Graphic Rendition
// ANSI sequence to process is already in ansi_sequence (ESC [ is not included)
// the_len contains the length of the sequence, not including the final command character. 
void Serial_ANSIHandleSGR(uint8_t the_len);
	

/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/

// 10    poke 1,0:poke $D633,128:poke $D630,31:poke $D631,5:poke $D632,0:cls 
// Serial_SetDLAB()
// UART_BASE = 31 . no idea what this does. update: sets baud rate when DLAB is 1. 31+5*256=UART_BAUD_DIV_1200
// UART_IER = 5. part of setting baud rate when DLAB is 1.
// UART_IIR = 0. no idea what this does.
// 20    poke $D633,128:poke $D633,3:poke $D632,231:poke $D631,0:print "Fterm5:"
// UART_LCR = 128. UART_DLAB_MASK. Serial_SetDLAB() again.
//  UART_LCR = 3. UART_DATA_BITS. Myabe sets "8 bits". UPDATE: set 8,1,n, no break signal.
//      UPDATE: R8(UART_LCR) = UART_DATA_BITS | UART_STOP_BITS | UART_PARITY | UART_NO_BRK_SIG; = 3+0+0+0
// UART_IIR=231. no idea. 0b11100111
// UART_IIR=0. no idea.
// 30    reg=peek($D635):reg=reg&1:if reg=1:ichar=peek($D630):goto 50:endif 
// x=UART_LSR. mask for data available. 
// char=UART_BASE : get data.
// 40    ochar=inkey():if ochar=0:goto 30:else :poke $D630,ochar:goto 30:endif 
// 50    if ichar=7:ping :goto 30:else :print chr$(ichar);:goto 30:endif 

// set UART chip to DLAB mode
void Serial_SetDLAB(void)
{
	Sys_SwapIOPage(VICKY_IO_PAGE_REGISTERS);
	R8(UART_LCR) = R8(UART_LCR) | UART_DLAB_MASK;
	Sys_RestoreIOPage();
}


// turn off DLAB mode on UART chip
void Serial_ClearDLAB(void)
{
	Sys_SwapIOPage(VICKY_IO_PAGE_REGISTERS);
	R8(UART_LCR) = R8(UART_LCR) & (~UART_DLAB_MASK);
	Sys_RestoreIOPage();
}
	

// Moves the cursor n (default 1) cells in the given direction.
// If the cursor is already at the edge of the screen, this has no effect.
void Serial_ANSICursorUp(uint8_t the_count)
{
	while (serial_y > TERM_BODY_Y1 && the_count > 0)
	{
		serial_y--;
		the_count--;
	}
	
	Text_SetXY(serial_x, serial_y);
}


// Moves the cursor n (default 1) cells in the given direction.
// If the cursor is already at the edge of the screen, this has no effect.
void Serial_ANSICursorDown(uint8_t the_count)
{
	while (serial_y < TERM_BODY_Y2 && the_count > 0)
	{
		serial_y++;
		the_count--;
	}
	
	Text_SetXY(serial_x, serial_y);
}


// Moves the cursor n (default 1) cells in the given direction.
// If the cursor is already at the edge of the screen, this has no effect.
void Serial_ANSICursorLeft(uint8_t the_count)
{
	while (serial_x > 0 && the_count > 0)
	{
		serial_x--;
		the_count--;
	}
	
	Text_SetXY(serial_x, serial_y);
}


// Moves the cursor n (default 1) cells in the given direction.
// If the cursor is already at the edge of the screen, this has no effect.
void Serial_ANSICursorRight(uint8_t the_count)
{
	while (serial_x < TERM_BODY_X2 && the_count > 0)
	{
		serial_x++;
		the_count--;
	}
	
	Text_SetXY(serial_x, serial_y);
}


// Moves cursor to beginning of the line n (default 1) lines down.
void Serial_ANSICursorNextLine(uint8_t the_count)
{
	serial_x = TERM_BODY_X1;
	
	while (serial_y < TERM_BODY_Y2 && the_count > 0)
	{
		Text_ScrollTextAndAttrRowsUp(TERM_BODY_Y1+1, TERM_BODY_Y2);
		Text_FillBox(TERM_BODY_X1, TERM_BODY_Y2, TERM_BODY_X2, TERM_BODY_Y2, CH_SPACE, serial_fg_color, serial_bg_color);
		serial_y++;
		the_count--;
	}

	Text_SetXY(serial_x, serial_y);
}


// scrolls down one line or an entire page
void Serial_ANSIScrollDown(bool scroll_page)
{
	uint8_t		the_count;
	
	serial_x = TERM_BODY_X1;
	
	if (scroll_page == true)
	{
		the_count = TERM_BODY_HEIGHT;
	}
	else
	{
		the_count = 1;
	}
	
	while (serial_y > TERM_BODY_Y1 && the_count > 0)
	{
		serial_y--;
		the_count--;
	}

	Text_SetXY(serial_x, serial_y);
}


// Moves cursor to beginning of the line n (default 1) lines up.
void Serial_ANSICursorPreviousLine(uint8_t the_count)
{
	serial_x = 0;
	
	while (serial_y > TERM_BODY_Y1 && the_count > 0)
	{
		serial_y--;
		the_count--;
	}

	Text_SetXY(serial_x, serial_y);
}


// scrolls up one line or an entire page
void Serial_ANSIScrollUp(bool scroll_page)
{
	uint8_t		the_count;
	
	serial_x = TERM_BODY_X1;
	
	if (scroll_page == true)
	{
		the_count = TERM_BODY_HEIGHT;
	}
	else
	{
		the_count = 1;
	}
	
	while (serial_y < TERM_BODY_Y2 && the_count > 0)
	{
		Text_ScrollTextAndAttrRowsUp(TERM_BODY_Y1+1, TERM_BODY_Y2);
		Text_FillBox(TERM_BODY_X1, TERM_BODY_Y2, TERM_BODY_X2, TERM_BODY_Y2, CH_SPACE, serial_fg_color, serial_bg_color);
		serial_y++;
		the_count--;
	}

	Text_SetXY(serial_x, serial_y);
}


// ANSI CHA
// Moves the cursor to column n (default 1)
void Serial_ANSICursorSetXPos(uint8_t the_count)
{
	// LOGIC:
	//   ANSI Positions are 1-based, not 0-based!
	
	if (the_count == 0)
	{
		return;
	}
	
	--the_count;
	
	if (the_count <= TERM_BODY_X2)
	{
		serial_x = the_count;
	}
	else
	{
		serial_x = TERM_BODY_X2;
	}

	Text_SetXY(serial_x, serial_y);
}


// ANSI CUP: CSI n ; m H
// Moves the cursor to row n, column m. 
// The values are 1-based, and default to 1 (top left corner) if omitted. 
// A sequence such as CSI ;5H is a synonym for CSI 1;5H
//   CSI 17;H is the same as CSI 17H and CSI 17;1H
void Serial_ANSICursorSetXYPos()
{
	uint8_t		the_x;
	uint8_t		the_y;
	char*		this_token;
	char*		splitter;
	
	// LOGIC:
	//   

// sprintf(global_string_buff2, "%s", ansi_sequence);

	// check that a column value was provided. if left out, it defeaults to 1 (y=0)
	splitter = strchr((char*)ansi_sequence, ';');
	
	if (splitter == NULL)
	{
		// row was left out. set to 1 as that's the default value.
		the_y = 1;
		//the_x = atoi((char*)ansi_sequence) + 1;
		this_token = strtok((char*)ansi_sequence, (const char*)"H");
		the_x = atoi(this_token);
	}
	else
	{		
		// tokenize to get first and second values
		this_token = strtok((char*)ansi_sequence, (const char*)";");

		the_y = atoi(this_token);
		this_token = strtok(NULL, (const char*)";");
		the_x = atoi(this_token);
		//the_x = atoi((char*)ansi_sequence);
	}
	
	// if value was left out, atoi will have returned 0, but default is 1.
	if (the_x == 0)	the_x = 1;
	if (the_y == 0) the_y = 1;
	
// sprintf(global_string_buff1, "CUP x,y=%d,%d ('%s')", the_x, the_y, global_string_buff2);
// Buffer_NewMessage((global_string_buff1));

	// account for 0-based vs 1-based
	the_y--;
	the_x--;
	
	// account for fact our screen doesn't start at 0
	the_y += TERM_BODY_Y1;
	
	if (the_y < TERM_BODY_Y2)
	{
		serial_y = the_y;
	}
	else
	{
		serial_y = TERM_BODY_Y2;
	}
	
	if (the_x < TERM_BODY_X2)
	{
		serial_x = the_x;
	}
	else
	{
		serial_x = TERM_BODY_X2;
	}

	Text_SetXY(serial_x, serial_y);
}


// ANSI HVP: CSI n ; m f
// Moves the cursor to row n, column m. 
// Same as CUP, but counts as a format effector function (like CR or LF) rather than an editor function (like CUD or CNL). 
// The values are 1-based, and default to 1 (top left corner) if omitted. 
void Serial_ANSICursorMoveToXY()
{
	uint8_t		the_x;
	uint8_t		the_y;
	char*		this_token;
	char*		splitter;
	
	// LOGIC:
	//   
// sprintf(global_string_buff2, "%s", ansi_sequence);

	// check that a column value was provided. if left out, it defeaults to 1 (y=0)
	splitter = strchr((char*)ansi_sequence, ';');
	
	if (splitter == NULL)
	{
		// row was left out. set to 1 as that's the default value.
		the_y = 1;
		//the_x = atoi((char*)ansi_sequence) + 1;
		this_token = strtok((char*)ansi_sequence, (const char*)"f");
		the_x = atoi(this_token);
	}
	else
	{		
		// tokenize to get first and second values
		this_token = strtok((char*)ansi_sequence, (const char*)";");

		the_y = atoi(this_token);
		this_token = strtok(NULL, (const char*)";");
		the_x = atoi(this_token);
		//the_x = atoi((char*)ansi_sequence);
	}
	
	// if value was left out, atoi will have returned 0, but default is 1.
	if (the_x == 0)	the_x = 1;
	if (the_y == 0) the_y = 1;
	
// sprintf(global_string_buff1, "HVP x,y=%d,%d ('%s')", the_x, the_y, global_string_buff2);
// Buffer_NewMessage((global_string_buff1));

	// account for 0-based vs 1-based
	the_y--;
	the_x--;
	
	// account for fact our screen doesn't start at 0
	the_y += TERM_BODY_Y1;
	
	if (the_y <= TERM_BODY_Y2)
	{
		serial_y = the_y;
	}
	else
	{
		the_y -= serial_y;
		
		while (serial_y < TERM_BODY_Y2 && the_y > TERM_BODY_Y1)
		{
			Text_ScrollTextAndAttrRowsUp(TERM_BODY_Y1+1, TERM_BODY_Y2);
			Text_FillBox(TERM_BODY_X1, TERM_BODY_Y2, TERM_BODY_X2, TERM_BODY_Y2, CH_SPACE, serial_fg_color, serial_bg_color);
			serial_y++;
			the_y--;
		}
	}
	
	if (the_x <= TERM_BODY_X2)
	{
		serial_x = the_x;
	}
	else
	{
		serial_x = TERM_BODY_X2;
	}

	Text_SetXY(serial_x, serial_y);
}


// ANSI clear
// clears the screen setting attributs to normal. homes the cursor
void Serial_ANSIClear(void)
{
	serial_fg_color = TERMINAL_DEFAULT_FORE_COLOR;
	serial_bg_color = TERMINAL_DEFAULT_BACK_COLOR;
	
	Text_FillBox(TERM_BODY_X1, TERM_BODY_Y1, TERM_BODY_X2, TERM_BODY_Y2, CH_SPACE, serial_fg_color, serial_bg_color);

	serial_x = TERM_BODY_X1;
	serial_y = TERM_BODY_Y1;

	Text_SetXY(serial_x, serial_y);
}


// ANSI ED - Erase in Display
// Clears part of the screen. 
//   If n is 0 (or missing), clear from cursor to end of screen. 
//   If n is 1, clear from cursor to beginning of the screen. 
//   If n is 2, clear entire screen (and moves cursor to upper left on DOS ANSI.SYS).
//   If n is 3, clear entire screen and delete all lines saved in the scrollback buffer
void Serial_ANSIEraseInDisplay(uint8_t the_count)
{
	switch (the_count)
	{
		case 0:
			// clear from cursor to end of screen
			Text_FillBox(TERM_BODY_X1, serial_y, TERM_BODY_X2, TERM_BODY_Y2, CH_SPACE, serial_fg_color, serial_bg_color);
			serial_x = TERM_BODY_X1;
			break;
		
		case 1:
			// clear from cursor to beginning of the screen. 
			Text_FillBox(TERM_BODY_X1, TERM_BODY_Y1, TERM_BODY_X2, serial_y, CH_SPACE, serial_fg_color, serial_bg_color);
			serial_x = TERM_BODY_X1;
			break;
			
		case 2:
		case 3:
			// clear entire screen
			Text_FillBox(TERM_BODY_X1, TERM_BODY_Y1, TERM_BODY_X2, TERM_BODY_Y2, CH_SPACE, serial_fg_color, serial_bg_color);
			serial_x = TERM_BODY_X1;
			serial_y = TERM_BODY_Y1;
			break;
			
		default:
			return;
	}

	Text_SetXY(serial_x, serial_y);
}


// ANSI EL - Erase in Line
// Erases part of the line. 
//   If n is 0 (or missing), clear from cursor to the end of the line.
//   If n is 1, clear from cursor to beginning of the line. 
//   If n is 2, clear entire line.
//   Cursor position does not change.
void Serial_ANSIEraseInLine(uint8_t the_count)
{
	switch (the_count)
	{
		case 0:
			// clear from cursor to the end of the line
			Text_FillBox(serial_x + 1, serial_y, TERM_BODY_X2, serial_y, CH_SPACE, serial_fg_color, serial_bg_color);
			break;
		
		case 1:
			// clear from cursor to beginning of the screen. 
			Text_FillBox(TERM_BODY_X1, serial_y, serial_x - 1, serial_y, CH_SPACE, serial_fg_color, serial_bg_color);
			break;
			
		case 2:
			// clear entire line
			Text_FillBox(TERM_BODY_X1, serial_y, TERM_BODY_X2, serial_y, CH_SPACE, serial_fg_color, serial_bg_color);
			break;
			
		default:
			return;
	}

	Text_SetXY(serial_x, serial_y);
}


// Device Status Report - 6n
// when requested by host computer, we need to respond with info about our terminal
// ESC[n;mR, where n is the row and m is the column.
// unofficial 255n seems to be used to send back terminal size. e.g, 24;80
void Serial_ANSISendDSR(uint8_t the_count)
{
	uint16_t		the_len;
// 	uint16_t		result;
	
	if (the_count == 6)
	{
		sprintf(global_string_buff1, "%c[%d;%dR", CH_ESC, (serial_y+1) - TERM_BODY_Y1, serial_x+1);
	}
	else
	{
		sprintf(global_string_buff1, "%c[%02d;%02dR", CH_ESC, TERM_BODY_HEIGHT, TERM_BODY_WIDTH);
	}
	
	the_len = strlen(global_string_buff1);
	Serial_SendData((uint8_t*)global_string_buff1, the_len);
// 	if ( (result = Serial_SendData((uint8_t*)global_string_buff1, the_len)) != the_len )
// 	{
// 		sprintf(global_string_buff2, "SendDSR fail: tried to send %d bytes, %d reported sent for '%s'", the_len, result, global_string_buff1);
// 	}
// 	else
// 	{
// 		sprintf(global_string_buff2, "SendDSR success: sent %d bytes for '%s'", the_len, global_string_buff1);
// 	}
// 	
// 	Buffer_NewMessage(global_string_buff2);
}


// ANSI function handler for SGR: Select Graphic Rendition
// ANSI sequence to process is already in ansi_sequence (ESC [ is not included)
// the_len contains the length of the sequence, not including the final command character. 
void Serial_ANSIHandleSGR(uint8_t the_len)
{
	uint8_t			temp;
	int16_t			this_color_code;
	char*			this_token;
	char*			splitter;
	
	// LOGIC:
	//   Wikipedia: The control sequence CSI n m, named Select Graphic Rendition (SGR), sets display attributes. Several attributes can be set in the same sequence, separated by semicolons.[21] Each display attribute remains in effect until a following occurrence of SGR resets it.[5] If no codes are given, CSI m is treated as CSI 0 m (reset / normal).
	//   
	//   foreground color codes are 30-37 for normal, or 90-97 if bright/bold. 
	//   background color codes are 40-47, or 100-107
	//   colors can be made bold with a '1;' sequence before. 
	//   a lot of this encoding won't be supportable on an F256 using text mode (underline, framed, etc.)
// sprintf(global_string_buff2, "('%s'), start bold mode=%x", ansi_sequence, ansi_bold_mode);
	
	// work through the command from left to right, allowing for multiple params
	// check we're not dealing with a singleton value
	splitter = strchr((char*)ansi_sequence, ';');
	
	if (splitter == NULL)
	{
		//this_token = (char*)ansi_sequence;
		this_token = strtok((char*)ansi_sequence, (const char*)"m");
//sprintf(global_string_buff2, "(solo token='%s'), start bold mode=%x", this_token, ansi_bold_mode);
	}
	else
	{		
		this_token = strtok((char*)ansi_sequence, (const char*)";");
	}
		
	while (this_token != NULL)
	{
		this_color_code = atoi(this_token);
		
		//if (this_color_code == 0)
		if (this_token[0] == '0' && strlen(this_token) == 1)
		{
			// 0 = reset background and foreground color to default
			serial_fg_color = TERMINAL_DEFAULT_FORE_COLOR;
			serial_bg_color = TERMINAL_DEFAULT_BACK_COLOR;
			ansi_bold_mode = false;
		}
		else if (this_color_code == 1)
		{
			// 1 = make the next code(s) bright/bold
			ansi_bold_mode = true;
			
			if (serial_fg_color < 8)
			{
				serial_fg_color += 8;
			}
		}
		else if (this_color_code == 2)
		{
			// 2 = regular (non-bright) foreground
			ansi_bold_mode = false;
		}
		else if (this_color_code == 3 || this_color_code == 7)
		{
			// 3 = italic. Represent on F256 as inverse video.
			// 7 = Inverse video. Swap foreground and background colors
			temp = serial_fg_color;
			serial_fg_color = serial_bg_color;
			serial_bg_color = temp;
		}
		else if (this_color_code == 4)
		{
			// 4 = Underline. no easy way to represent this on F256			
		}
		else if (this_color_code == 5 || this_color_code == 6)
		{
			// 5 = Slow blink (<150x/min). no easy way to represent this on F256
			// 6 = Rapid blink. 
		}
		else if (this_color_code == 8)
		{
			// 8 = invisible text (fore=back)
			serial_fg_color = serial_bg_color;
		}
		else if (this_color_code == 9)
		{
			// 9 = crossed-out. no easy way to represent this on F256			
		}
		else
		{
			if (this_color_code > 99)
			{
				//ansi_bold_mode = true;
				// bright / bold background color. does not affect future boldness for foreground.
				serial_bg_color = (this_color_code - 100) + 8;
			}
			else if (this_color_code > 89)
			{
				// bright / bold foreground color. does not affect future boldness for foreground.
				ansi_bold_mode = true;
				serial_fg_color = (this_color_code - 90) + 8;
			}
			else if (this_color_code > 39)
			{
				serial_bg_color = this_color_code - 40;
			}
			else if (this_color_code > 29)
			{
				serial_fg_color = this_color_code - 30;
				
				if (ansi_bold_mode == true)
				{
					serial_fg_color += 8;
				}				
			}
			else
			{
				sprintf(global_string_buff1, "SGR unhandled code '%s' (colorcode=%d) %s", ansi_sequence, this_color_code, global_string_buff2);
				Buffer_NewMessage((global_string_buff1));
			}
		}
		
		this_token = strtok(NULL, (const char*)";");
	}
// sprintf(global_string_buff1, "SGR fg/bg=%d,%d; bold=%x %s", serial_fg_color, serial_bg_color, bold, global_string_buff2);
// Buffer_NewMessage((global_string_buff1));
}


// process a byte from the serial port, including checking for ANSI sequences and printing to screen
void Serial_ProcessByte(uint8_t the_byte)
{
	if (the_byte == 0)
	{
		// error condition
		return;
	}
	else
	{
		// TODO: dedicated routine to display chars, handling movement back to next line, scroll screen, etc. 
		
		if (ansi_phase == 0 && the_byte != CH_ESC)
		{
			// normal text, not part of ANSI sequence
			Serial_PrintByte(the_byte);
		}
		else
		{
			// are we in an ANSI sequence, or just kicking one off?
			if (ansi_phase == 0 && the_byte == CH_ESC)
			{
				ansi_phase = 1;
				ansi_sequence = ansi_sequence_storage;
			}
			else if (ansi_phase == 1)
			{
				if (the_byte == CH_LBRACKET)
				{
					// now we have full starting sequence: ESC+bracket
					ansi_phase = 2;
				}
				else
				{
					// turns out this wasn't ANSI after all.
					ansi_phase = 0;
					Serial_PrintByte(the_byte);					
				}
			}
			else
			{
				// we were already in an ANSI sequence. collect.
				if ( (the_byte > CH_AT && the_byte < CH_LBRACKET) || (the_byte > CH_LSQUOTE && the_byte < CH_LCBRACKET) )
				{
					// terminated marker for an ANSI sequence is an upper or lower case alpha char
					*ansi_sequence++ = the_byte;
					*ansi_sequence = 0;
					ansi_phase = 0;
					Serial_ProcessANSI();
					ansi_sequence = ansi_sequence_storage;
				}
				else
				{
					// some part of the body of the sequence
					*ansi_sequence++ = the_byte;
				}
			}
		}
	}
}


// print a byte to screen, from the serial port
void Serial_PrintByte(uint8_t the_byte)
{
	bool		update_vicky_curs_pos = true;

	// reset text engine location, in case it has changed due to other action
	Text_SetXY(serial_x, serial_y);
	
	if (the_byte == CH_ENTER)
	{
		serial_x = TERM_BODY_X1;
	}
	else if (the_byte == CH_LF || the_byte == CH_FF)
	{
		if (serial_y >= TERM_BODY_Y2)
		{
			Text_ScrollTextAndAttrRowsUp(TERM_BODY_Y1+1, TERM_BODY_Y2);
			Text_FillBox(TERM_BODY_X1, TERM_BODY_Y2, TERM_BODY_X2, TERM_BODY_Y2, CH_SPACE, serial_fg_color, serial_bg_color);
		}
		else
		{
			++serial_y;
		}
	}
	else if (the_byte == CH_BKSP && serial_x > TERM_BODY_X1)
	{
		// backspace in ASCII. not sure if right thing is to move back or delete prev char and move back
		--serial_x;
	}
	else
	{
		Text_SetCharAndColor(the_byte, serial_fg_color, serial_bg_color);
		serial_x++;	// test lib moved ahead, but locally we need to know if wrapping happened.
		update_vicky_curs_pos = false;

		if (serial_x > TERM_BODY_X2)
		{
			serial_x = TERM_BODY_X2;
		}
		
// 		// DEBUG - print everything in the in-buffer
// 		memcpy(global_string_buff1, global_uart_in_buffer, global_uart_read_idx);
// 		global_string_buff1[global_uart_read_idx+1] = 0;
// 		Buffer_NewMessage(global_string_buff1);
// 		// DEBUG
	}
	
	// update cursor position in VICKY so flashing cursor shows where we are
	if (update_vicky_curs_pos == true)
	{
		Text_SetXY(serial_x, serial_y);
	}
}


// process the ANSI sequence stored in ansi_sequence_storage
void Serial_ProcessANSI(void)
{
	uint8_t			the_len;
	uint8_t			ansi_function;
	uint8_t			the_count;	// for single-number functions, will hold converted number
	ansi_action		the_action = ANSI_UNRECOGNIZED;
	
	// LOGIC:
	//   ANSI CSI (Control Sequence Introducer) sequences consist of ESC + [ + params + function code
	//   The function code is a single byte, case sensitive
	//   the parameters are read left to right, and can be stacked with semicolons
	//
	//   design here is to first distinguish which function it is by checking last letter,
	//   then...
	
	ansi_sequence = ansi_sequence_storage;
	
	the_len = General_Strnlen((char*)ansi_sequence, ANSI_MAX_SEQUENCE_LEN + 1);
	ansi_function = ansi_sequence[the_len-1];
	--the_len; // so we can pass this to handler functions and not have it see the command code
	the_count = atoi((char*)ansi_sequence);	// won't work for all functions, but that's fine.
	
	switch (ansi_function)
	{
		case ANSI_FUNCTION_CUU:
			// Cursor Up
			Serial_ANSICursorUp(the_count);
			break;
		
		case ANSI_FUNCTION_SU:
			// Scroll Up
			Serial_ANSIScrollUp(false);
			break;
		
		case ANSI_FUNCTION_CUD:
			// Cursor Down
			Serial_ANSICursorDown(the_count);
			break;
		
		case ANSI_FUNCTION_SD:
			// Scroll Down
			Serial_ANSIScrollUp(false);
			break;
		
		case ANSI_FUNCTION_CUF:
			// Cursor Forward
			Serial_ANSICursorRight(the_count);
			break;
		
		case ANSI_FUNCTION_CUB:
			// Cursor Back
			Serial_ANSICursorLeft(the_count);
			break;
		
		case ANSI_FUNCTION_CNL:
			// Cursor Next Line
			Serial_ANSICursorNextLine(the_count);
			break;
		
		case ANSI_FUNCTION_CPL:
			// Cursor Previous Line
			Serial_ANSICursorPreviousLine(the_count);
			break;
		
		case ANSI_FUNCTION_CHA:
			// Cursor Horizontal Absolute
			Serial_ANSICursorSetXPos(the_count);
			break;
		
		case ANSI_FUNCTION_CUP:
			// Cursor Position
			Serial_ANSICursorSetXYPos();
			break;

		case ANSI_FUNCTION_SAVECURPOS:
			// save current cursor position
			Serial_ANSICursorSave();
			break;
		
		case ANSI_FUNCTION_RESTORECURPOS:
			// restore cursor position from last saven
			Serial_ANSICursorRestore();
			break;
		
		case ANSI_FUNCTION_CLEAR:
			Serial_ANSIClear();
			break;
			
		case ANSI_FUNCTION_ED:
			// Erase in Display
			Serial_ANSIEraseInDisplay(the_count);
			break;
		
		case ANSI_FUNCTION_EL:
			// Erase in Line
			Serial_ANSIEraseInLine(the_count);
			break;
		
		case ANSI_FUNCTION_HVP:
			// Horizontal Vertical Position
			Serial_ANSICursorMoveToXY();
			break;
		
		case ANSI_FUNCTION_SGR:
			// Select Graphic Rendition
			Serial_ANSIHandleSGR(the_len);
			break;
		
		case ANSI_FUNCTION_DSR:
			// Device Status Report
			Serial_ANSISendDSR(the_count);
			break;
		
		case ANSI_FUNCTION_PRIVHIDEMOUSE:
		case ANSI_FUNCTION_PRIVSHOWMOUSE:
			// private functions we will not attempt to parse
			break;
			
		default:
			// unknown (to f/term) ANSI functions: capture in the buffer
			Buffer_NewMessage((char*)ansi_sequence);
			break;
	}
}
// stuff found I didn't detect:
// 8;25;80t	: Ps = 8 ;  height ;  width ⇒  Resize the text area to given height and width in characters.  Omitted parameters reuse the current height or width.  Zero parameters use the display's height or width
// 1;25r	: ESC [ <top> ; <bottom> r    set scroll region


/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/



// set up ANSI colors
void Serial_InitANSIColors(void)
{
	// set standard color LUTs for text mode
	Sys_SwapIOPage(VICKY_IO_PAGE_REGISTERS);
	memcpy((uint8_t*)(TEXT_FORE_LUT), &ansi_text_color_lut, 64);
	memcpy((uint8_t*)(TEXT_BACK_LUT), &ansi_text_color_lut, 64);
	Sys_RestoreIOPage();
}


// set up UART for serial comms
void Serial_InitUART(void)
{
 	uint8_t		junk;

	Sys_SwapIOPage(VICKY_IO_PAGE_REGISTERS);
	R8(UART_LCR) = UART_DATA_BITS | UART_STOP_BITS | UART_PARITY | UART_NO_BRK_SIG;
	Serial_SetDLAB();
	R16(UART_DLL) = UART_BAUD_DIV_9600;
	Serial_ClearDLAB();

	// no idea what this does. copying blindly from EMWhite's BASIC example
	R8(UART_IIR) = 231;
	R8(UART_IER) = 0;	// probably turns off interrupts

	// Read and clear status registers
	junk = R8(UART_LSR);
	junk = R8(UART_MSR);

	Sys_RestoreIOPage();
	
	serial_x = TERM_BODY_X1;
	serial_y = TERM_BODY_Y1;
}


// change baud rate
// new_baud_rate_divisor must be UART_BAUD_DIV_4800, UART_BAUD_DIV_9600, etc.
void Serial_SetBaud(uint16_t new_baud_rate_divisor)
{
	Sys_SwapIOPage(VICKY_IO_PAGE_REGISTERS);

	Serial_SetDLAB();
	R16(UART_DLL) = new_baud_rate_divisor;
	Serial_ClearDLAB();

	Sys_RestoreIOPage();
}


// send a byte over the UART serial connection
// if the UART send buffer does not have space for the byte, it will try for UART_MAX_SEND_ATTEMPTS then return an error
// returns false on any error condition
bool Serial_SendByte(uint8_t the_byte)
{
	uint8_t		error_code;
	bool		uart_in_buff_is_empty = false;
	uint16_t	num_tries = 0;
	
	Sys_SwapIOPage(VICKY_IO_PAGE_REGISTERS);
	
	error_code = R8(UART_LSR) & UART_ERROR_MASK;
	
	if (error_code > 0)
	{
		App_EnterStealthTextUpdateMode();
		
		if (error_code & FLAG_UART_LSR_OE)
		{
			Text_SetCharAtXY(TERM_ERROR_OE_X-10, TITLE_BAR_Y, 'O');
		}
		
		if (error_code & FLAG_UART_LSR_PE)
		{
			Text_SetCharAtXY(TERM_ERROR_PE_X-10, TITLE_BAR_Y, 'P');
		}
		
		if (error_code & FLAG_UART_LSR_FE)
		{
			Text_SetCharAtXY(TERM_ERROR_FE_X-10, TITLE_BAR_Y, 'F');
		}
		
		if (error_code & FLAG_UART_LSR_BI)
		{
			Text_SetCharAtXY(TERM_ERROR_BI_X-10, TITLE_BAR_Y, 'B');
		}
		
		if (error_code & FLAG_UART_LSR_ERR)
		{
			Text_SetCharAtXY(TERM_ERROR_ERR_X-10, TITLE_BAR_Y, 'E');
		}
		
		// try to clear error. 
		error_code = R8(UART_LSR);
		error_code = R8(UART_MSR);
		
		App_ExitStealthTextUpdateMode();

		goto error;
	}
	
	while (uart_in_buff_is_empty == false && num_tries < UART_MAX_SEND_ATTEMPTS)
	{
		uart_in_buff_is_empty = R8(UART_LSR) & UART_THR_IS_EMPTY;
		++num_tries;
	};
	
	if (uart_in_buff_is_empty == true)
	{
		goto error;
	}
	
	R8(UART_THR) = the_byte;
	
	Sys_RestoreIOPage();
	
	return true;
	
	error:
		Sys_RestoreIOPage();
		return false;
}


// send 1-255 bytes to the UART serial connection
// returns # of bytes successfully sent (which may be less than number requested, in event of error, etc.)
uint8_t Serial_SendData(uint8_t* the_buffer, uint16_t buffer_size)
{
	uint16_t	i;
	uint8_t		the_byte;
	
	if (buffer_size > 256)
	{
		return 0;
	}
	
	for (i=0; i < buffer_size; i++)
	{
		the_byte = the_buffer[i];
		
		if (Serial_SendByte(the_byte) == false)
		{
			return i;
		}
	}
	
	return i;
}


// Check for serial data in the UART and transfer to the circular buffer for later processing
// Returns true on data available or on no data, but no error. Returns false on any error. 
bool Serial_ReadUART(void)
{
	uint8_t		error_code;
	bool		success = true;
	
	Sys_SwapIOPage(VICKY_IO_PAGE_REGISTERS);
		
	error_code = R8(UART_LSR) & UART_ERROR_MASK;

	if (error_code > 0)
	{
		App_EnterStealthTextUpdateMode();
		
		if (error_code & FLAG_UART_LSR_OE)
		{
			Text_SetCharAtXY(TERM_ERROR_OE_X, TITLE_BAR_Y, 'O');
		}
		
		if (error_code & FLAG_UART_LSR_PE)
		{
			Text_SetCharAtXY(TERM_ERROR_PE_X, TITLE_BAR_Y, 'P');
		}
		
		if (error_code & FLAG_UART_LSR_FE)
		{
			Text_SetCharAtXY(TERM_ERROR_FE_X, TITLE_BAR_Y, 'F');
		}
		
		if (error_code & FLAG_UART_LSR_BI)
		{
			Text_SetCharAtXY(TERM_ERROR_BI_X, TITLE_BAR_Y, 'B');
		}
		
		if (error_code & FLAG_UART_LSR_ERR)
		{
			Text_SetCharAtXY(TERM_ERROR_ERR_X, TITLE_BAR_Y, 'E');
		}
		
		// try to clear error. 
		error_code = R8(UART_LSR);
		error_code = R8(UART_MSR);
		
		//sprintf(global_string_buff1, "serial error %x", error_code);
		//Buffer_NewMessage(global_string_buff1);
		success = false;
		
		App_ExitStealthTextUpdateMode();
	}
	else
	{
		while ( (R8(UART_LSR) & UART_DATA_AVAILABLE) > 0)
		{
			global_uart_in_buffer[global_uart_write_idx++] = R8(UART_BASE);
				
			if (global_uart_write_idx > UART_BUFFER_SIZE)
			{
				global_uart_write_idx = 0;
			}
		}
	}
	
	Sys_RestoreIOPage();
	
	return success;
}


// Check for available data in the UART circular buffer and process any that are available.
// returns false if no bytes were available
bool Serial_ProcessAvailableData(void)
{
	if (global_uart_read_idx == global_uart_write_idx)
	{
		// nothing in receive buffer
		return false;
	}
	else
	{
		while ( global_uart_read_idx != global_uart_write_idx )
		{
			Serial_ProcessByte(global_uart_in_buffer[global_uart_read_idx++]);
			
			if (global_uart_read_idx > UART_BUFFER_SIZE)
			{
				global_uart_read_idx = 0;
			}
		}
	}
	
	return true;
}


// get a byte from UART serial connection, or 1 ANSI sequence worth of bytes
// flush the in (Rx) buffer
// resets circular buffer pointers so that any not-yet-processed bytes are forgotten about
void Serial_FlushInBuffer(void)
{
	global_uart_read_idx = 0;
	global_uart_write_idx = 0;
}


// save current cursor position
void Serial_ANSICursorSave(void)
{
	serial_save_x = serial_x;
	serial_save_y = serial_y;
}


// restore cursor position from last save
void Serial_ANSICursorRestore(void)
{
	serial_x = serial_save_x;
	serial_y = serial_save_y;
	Text_SetXY(serial_x, serial_y);
}


// cycle to the next foreground color, updating every cell in the terminal screen
void Serial_CycleForegroundColor(void)
{
	serial_current_pref_color++;
	
	if (serial_current_pref_color > 15)
	{
		serial_current_pref_color = 1;		// you aren't allowed to select black on black
	}
	
	Text_FillBoxAttrOnly(TERM_BODY_X1, TERM_BODY_Y1, TERM_BODY_X2, TERM_BODY_Y2, serial_current_pref_color, COLOR_BLACK);
	serial_fg_color = serial_current_pref_color;
}


// // debug function - dump serial buffer to disk
// bool Serial_DebugDump(void)
// {
// 	static uint8_t	dump_num;
// 
// 	uint8_t				i;
// 	uint32_t			percent_read;
// 	bool				success;
// 	char*				the_name;
// 	char				temp_path_buffer[32];
// 	uint8_t*			the_buffer = (uint8_t*)UART_BUFFER_START_ADDR;
// 	unsigned int		s_bytes_written_to_disk = 0;
// 	FRESULT				the_result;
// 	FIL					the_target_handle;
// 	
// 	dump_num++;
// 	
// 	
// 	// copy memory bank to file on disk
// 
// 	// set up a 'what's the file name?' dialog box
// 	General_Strlcpy(global_string_buff1, "Dump serial buffer to disk", 70);
// 	
// 	// copy the current bank name into the edit buffer so user can edit
// 	sprintf(global_string_buff2, "serial_dump_%02X.bin", dump_num);
// 	
// 	the_name = Screen_GetStringFromUser(global_string_buff1, "Enter file name", global_string_buff2, FILE_MAX_FILENAME_SIZE);
// 	
// 	if (the_name == NULL)
// 	{
// 		return false;
// 	}
// 
// 	General_CreateFilePathFromFolderAndFile(temp_path_buffer, "0:", the_name, FILE_MAX_PATHNAME_SIZE);
// Buffer_NewMessage(temp_path_buffer);
// 
// 	// Get a target file handle for Writing
// 	the_result = f_open(&the_target_handle, temp_path_buffer, FA_WRITE | FA_CREATE_ALWAYS);
// 	
// 	if (the_result != FR_OK)
// 	{
// 		//sprintf(global_string_buff1, "file '%s' could not be opened for copying into", the_target_file_path);
// 		//Buffer_NewMessage(global_string_buff1);
// 		return false;
// 	}
// 
// 	the_result = f_write(&the_target_handle, the_buffer, UART_BUFFER_SIZE, &s_bytes_written_to_disk);
// 
// 	if (the_result != FR_OK)
// 	{
// 		//sprintf(global_string_buff1, "file '%s': got error writing", the_target_file_path);
// 		//Buffer_NewMessage(global_string_buff1);
// 		f_close(&the_target_handle);
// 		return false;
// 	}
// 	
// 	if ( s_bytes_written_to_disk < STORAGE_FILE_BUFFER_LEN )
// 	{
// 		// error or disk full
// 		//sprintf(global_string_buff1, "%s %d: file '%s': tried to write %d, only wrote %d", __func__ , __LINE__, the_target_file_path, s_bytes_read_from_disk, s_bytes_written_to_disk);
// 		//Buffer_NewMessage(global_string_buff1);
// 		f_close(&the_target_handle);
// 		return false;
// 	}				
// 	
// 	f_close(&the_target_handle);
// 	
// 	success = true;
// 
// 	return success;
// }

