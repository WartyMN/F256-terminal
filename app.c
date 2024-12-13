/*
 * app.c
 *
 *  Created on: Jan 10, 2023
 *      Author: micahbly
 *
 *  A pseudo commander-style 2-column file manager
 *
 */
 


/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/


// project includes
#include "app.h"
#include "comm_buffer.h"
#include "general.h"
#include "keyboard.h"
#include "memory.h"
//#include "overlay_em.h"
#include "overlay_startup.h"
#include "text.h"
#include "screen.h"
#include "serial.h"
#include "strings.h"
#include "sys.h"

// C includes
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// cc65 includes
#include <device.h>
//#include <unistd.h>
#include <cc65.h>
#include "dirent.h"
#include "f256.h"
#include "api.h"
#include "kernel.h"



/*****************************************************************************/
/*                               Definitions                                 */
/*****************************************************************************/

#define CH_PROGRESS_BAR_FULL	CH_CHECKERBOARD
#define PROGRESS_BAR_Y			(COMM_BUFFER_FIRST_ROW - 4)
#define PROGRESS_BAR_START_X	TERM_PROGRESS_BAR_START_X
#define PROGRESS_BAR_WIDTH		10	// number of characters in progress bar
#define PROGRESS_BAR_DIVISOR	8	// number of slices in one char: divide % by this to get # of blocks to draw.
#define COLOR_PROGRESS_BAR		COLOR_CYAN

// key <> action mapping
#define CH_ALT_OFFSET			128	// MB 2024/12/09: I HAVE NO idea if this is even possible with microkernel, this was based on my keyboard input for F256e. puttiing in something for now so it can build
#define ACTION_CYCLE_COLOR		(CH_LC_C + CH_ALT_OFFSET)	// alt-c
#define ACTION_SELECT_FONT_FNX	(CH_LC_F + CH_ALT_OFFSET)	// alt-f
#define ACTION_SELECT_FONT_ANSI	(CH_LC_A + CH_ALT_OFFSET)	// alt-a
#define ACTION_SELECT_FONT_IBM	(CH_LC_I + CH_ALT_OFFSET)	// alt-i
#define ACTION_SET_TIME			(CH_LC_T + CH_ALT_OFFSET)	// alt-t
//#define ACTION_RECEIVE_YMODEM	(CH_LC_Y + CH_ALT_OFFSET)	// alt-y
//#define ACTION_ABORT_SESSION	(CH_ESC + CH_ALT_OFFSET)	// alt-ESC
#define ACTION_SET_BAUD_300		(CH_1 + CH_ALT_OFFSET)	// alt-1
#define ACTION_SET_BAUD_1200	(CH_2 + CH_ALT_OFFSET)	// alt-2
#define ACTION_SET_BAUD_2400	(CH_3 + CH_ALT_OFFSET)	// alt-3
#define ACTION_SET_BAUD_3600	(CH_4 + CH_ALT_OFFSET)	// alt-4
#define ACTION_SET_BAUD_4800	(CH_5 + CH_ALT_OFFSET)	// alt-5
#define ACTION_SET_BAUD_9600	(CH_6 + CH_ALT_OFFSET)	// alt-6
#define ACTION_SET_BAUD_19200	(CH_7 + CH_ALT_OFFSET)	// alt-7
#define ACTION_SET_BAUD_38400	(CH_8 + CH_ALT_OFFSET)	// alt-8
#define ACTION_SET_BAUD_57600	(CH_9 + CH_ALT_OFFSET)	// alt-9
#define ACTION_SET_BAUD_115200	(CH_0 + CH_ALT_OFFSET)	// alt-10

#define ACTION_RESET_UART		(CH_LC_R + CH_ALT_OFFSET)	// alt-r

#define ACTION_DEBUG_DUMP		(CH_LC_D + CH_ALT_OFFSET)	// alt-d

#define ACTION_TEST_CONNECTWIFI	(CH_LC_W + CH_ALT_OFFSET)	// alt-w
#define ACTION_TEST_CONNECTTFNS	(CH_UC_W + CH_ALT_OFFSET)	// alt-W
#define ACTION_TEST_MOUNTTFNS	(CH_LC_M + CH_ALT_OFFSET)	// alt-m



/*****************************************************************************/
/*                          File-scoped Variables                            */
/*****************************************************************************/

static uint8_t				app_progress_bar_char[8] = 
{
	CH_SPACE,
	CH_PROGRESS_BAR_CHECKER_CH1,
	CH_PROGRESS_BAR_CHECKER_CH1+1,
	CH_PROGRESS_BAR_CHECKER_CH1+2,
	CH_PROGRESS_BAR_CHECKER_CH1+3,
	CH_PROGRESS_BAR_CHECKER_CH1+4,
	CH_PROGRESS_BAR_CHECKER_CH1+5,
	CH_PROGRESS_BAR_CHECKER_CH1+6,
};



/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/

extern uint8_t*				global_uart_in_buffer;
extern uint16_t				global_uart_write_idx;

extern System*			global_system;

uint8_t						global_current_baud_config;		// index to global_baud_config[]

const baud_config	global_baud_config[10] = 
{
	{ACTION_SET_BAUD_115200,	UART_BAUD_DIV_115200,	ID_STR_MSG_SET_BAUD_115200,	ID_STR_BAUD_115200},
	{ACTION_SET_BAUD_300,		UART_BAUD_DIV_300,		ID_STR_MSG_SET_BAUD_300,	ID_STR_BAUD_300},
	{ACTION_SET_BAUD_1200,		UART_BAUD_DIV_1200,		ID_STR_MSG_SET_BAUD_1200,	ID_STR_BAUD_1200},
	{ACTION_SET_BAUD_2400,		UART_BAUD_DIV_2400,		ID_STR_MSG_SET_BAUD_2400,	ID_STR_BAUD_2400},
	{ACTION_SET_BAUD_3600,		UART_BAUD_DIV_3600,		ID_STR_MSG_SET_BAUD_3600,	ID_STR_BAUD_3600},
	{ACTION_SET_BAUD_4800,		UART_BAUD_DIV_4800, 	ID_STR_MSG_SET_BAUD_4800,	ID_STR_BAUD_4800},
	{ACTION_SET_BAUD_9600,		UART_BAUD_DIV_9600,		ID_STR_MSG_SET_BAUD_9600,	ID_STR_BAUD_9600},
	{ACTION_SET_BAUD_19200,		UART_BAUD_DIV_19200,	ID_STR_MSG_SET_BAUD_19200,	ID_STR_BAUD_19200},
	{ACTION_SET_BAUD_38400,		UART_BAUD_DIV_38400,	ID_STR_MSG_SET_BAUD_38400,	ID_STR_BAUD_38400},
	{ACTION_SET_BAUD_57600,		UART_BAUD_DIV_57600,	ID_STR_MSG_SET_BAUD_57600,	ID_STR_BAUD_57600},
};

bool					global_clock_is_visible;		// tracks whether or not the clock should be drawn. set to false when not showing main 2-panel screen.

TextDialogTemplate		global_dlg = 
{
	(SCREEN_NUM_COLS - APP_DIALOG_WIDTH)/2, 16, APP_DIALOG_WIDTH, APP_DIALOG_HEIGHT, 
	APP_DIALOG_STARTING_NUM_BUTTONS, NULL, NULL, 
	{NULL, NULL, NULL},
	{CH_RUNSTOP, CH_ENTER, CH_DEL},
	{COLOR_GREEN, COLOR_RED, COLOR_BLUE},
	{false, true, false},
};	// dialog we'll configure and re-use for different purposes


char					global_dlg_title[36];	// arbitrary
char					global_dlg_body_msg[70];	// arbitrary
char					global_dlg_button[3][10];	// arbitrary

char*					global_string_buff1 = (char*)STORAGE_STRING_BUFFER_1;
char*					global_string_buff2 = (char*)STORAGE_STRING_BUFFER_2;

char					global_temp_path_1_buffer[FILE_MAX_PATHNAME_SIZE];
char					global_temp_path_2_buffer[FILE_MAX_PATHNAME_SIZE] = "";
char*					global_temp_path_1 = global_temp_path_1_buffer;
char*					global_temp_path_2 = global_temp_path_2_buffer;

uint8_t					temp_screen_buffer_char[APP_DIALOG_BUFF_SIZE];	// WARNING HBD: don't make dialog box bigger than will fit!
uint8_t					temp_screen_buffer_attr[APP_DIALOG_BUFF_SIZE];	// WARNING HBD: don't make dialog box bigger than will fit!

extern uint8_t			zp_bank_num;
extern uint8_t			zp_x;
extern uint8_t			zp_y;
#pragma zpsym ("zp_bank_num");
#pragma zpsym ("zp_x");
#pragma zpsym ("zp_y");


/*****************************************************************************/
/*                       Private Function Prototypes                         */
/*****************************************************************************/

// draw/refresh title bar and items that appear in the title bar
void App_DrawTitleBar(void);

// Switch font
void App_ChangeUIFont(font_choice the_font);

// initialize various objects - once per run
void App_Initialize(void);

// main loop when terminal comms mode is on
void App_MainLoop(void);

// have serial change baud rate and show msg and label
void App_ChangeBaudRate(uint8_t new_config_index);

		

/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/


// initialize various objects - once per run
void App_Initialize(void)
{
	// we want 80x30 screen, not 80x60. 25 for ANSI display, 1 for title at top, 4 four status/comms at bottom
	Sys_SetTextPixelHeight(false, true);

	Text_ClearScreen(APP_FOREGROUND_COLOR, APP_BACKGROUND_COLOR);
	
	Sys_SwitchFontSet(PARAM_USE_SECONDARY_FONT_SLOT);
	App_ChangeUIFont(FONT_IBM_ANSI);

	// initialize serial port for terminal comms
	Serial_InitUART(ACTION_SET_BAUD_4800 - ACTION_SET_BAUD_115200);
	Serial_InitANSIColors();
	App_ChangeBaudRate(ACTION_SET_BAUD_4800 - ACTION_SET_BAUD_115200);	// ACTION_SET_BAUD_9600 - ACTION_SET_BAUD_115200
	
	Buffer_Clear();

	// show info about the host F256 and environment, as well as copyright, version of this app
	App_LoadOverlay(OVERLAY_SCREEN);
	Screen_ShowAppAboutInfo();

	// set up the dialog template we'll use throughout the app
	global_dlg.title_text_ = global_dlg_title;
	global_dlg.body_text_ = global_dlg_body_msg;
	global_dlg.btn_label_[0] = global_dlg_button[0];
	global_dlg.btn_label_[1] = global_dlg_button[1];
}


// main loop when terminal comms mode is on
void App_MainLoop(void)
{
	uint8_t				user_input;
	bool				exit_main_loop = false;
	bool				success;
	
	// main loop
	while (! exit_main_loop)
	{
		// turn ON cursor - seems to turn itself off when kernal detects cursor position has changed. 
		Sys_EnableTextModeCursor(true);

		// clear terminal area in middle of screen
		Text_FillBox(TERM_BODY_X1, TERM_BODY_Y1, TERM_BODY_X2, TERM_BODY_Y2, CH_SPACE, COLOR_ORANGE, COLOR_BLACK);

		// prep for re-outputting serial to top part of serial panel area
		Text_SetXY(TERM_BODY_X1, TERM_BODY_Y1);	

		//DEBUG: track software stack pointer
		//sprintf(global_string_buffer, "sp: %x%x", *(char*)0x52, *(char*)0x51);
		//Buffer_NewMessage(global_string_buffer);

		do
		{
			Serial_ReadUART();
			Serial_ProcessAvailableData();

			user_input = Keyboard_GetKeyIfPressed();
// Text_SetXY(0,0);
// Text_SetChar(user_input);
			
			if (user_input > 0)
			{
				//sprintf(global_string_buff1, "input: %x (%d)", user_input, user_input);
				//Buffer_NewMessage(global_string_buff1);

				if (user_input == (uint8_t)ACTION_SELECT_FONT_FNX)
				{
					App_ChangeUIFont(FONT_STD);
					Buffer_NewMessage(General_GetString(ID_STR_MSG_SELECT_FONT_FOENIX));
				}
				else if (user_input == (uint8_t)ACTION_SELECT_FONT_ANSI)
				{
					App_ChangeUIFont(FONT_STD_ANSI);
					Buffer_NewMessage(General_GetString(ID_STR_MSG_SELECT_FONT_ANSI));
				}
				else if (user_input == (uint8_t)ACTION_SELECT_FONT_IBM)
				{
					App_ChangeUIFont(FONT_IBM_ANSI);
					Buffer_NewMessage(General_GetString(ID_STR_MSG_SELECT_FONT_IBM));
				}
				else if (user_input >= ACTION_SET_BAUD_115200 && user_input <= ACTION_SET_BAUD_57600)
				{
					App_ChangeBaudRate(user_input - ACTION_SET_BAUD_115200);
				}
				else if (user_input == ACTION_CYCLE_COLOR)
				{
					Serial_CycleForegroundColor();
				}
// 				else if (user_input == ACTION_RECEIVE_YMODEM)
// 				{
// 					Buffer_NewMessage("Starting YModem receive...");
// 					Serial_StartYModemReceive();
// 				}
				else if (user_input == ACTION_SET_TIME)
				{
					General_Strlcpy((char*)&global_dlg_title, General_GetString(ID_STR_DLG_SET_CLOCK_TITLE), COMM_BUFFER_MAX_STRING_LEN);
					General_Strlcpy((char*)&global_dlg_body_msg, General_GetString(ID_STR_DLG_SET_CLOCK_BODY), APP_DIALOG_WIDTH);
					global_string_buff2[0] = 0;	// clear whatever string had been in this buffer before
					
					success = Text_DisplayTextEntryDialog(&global_dlg, (char*)&temp_screen_buffer_char, (char*)&temp_screen_buffer_attr, global_string_buff2, 14, APP_ACCENT_COLOR, APP_FOREGROUND_COLOR, APP_BACKGROUND_COLOR); //YY-MM-DD HH-MM = 14
					
					if (success)
					{
						// user entered a date/time string, now try to parse and save it.
						success = Sys_UpdateRTC(global_string_buff2);
						App_DisplayTime();
					}
				}
				else if (user_input == ACTION_RESET_UART)
				{
					Serial_InitUART(global_baud_config[global_current_baud_config].divisor_);
				}

// 2024/12/11 MB: need to make version of serial debug dump that works with microkernel. trivial, but work. 

// 				else if (user_input == ACTION_DEBUG_DUMP)
// 				{
// 					if (Serial_DebugDump() == true)
// 					{
// 						Buffer_NewMessage(General_GetString(ID_STR_MSG_DEBUG_DUMP));
// 					}
// 					else
// 					{
// 						Buffer_NewMessage(General_GetString(ID_STR_ERROR_GENERIC_DISK));
// 					}
// 				}


// 2024/08/02 MB: did not succeed in getting either of the below approaches to work. the +++ATH thing definitely ruins the call, but modem still says connected.
// 				else if (user_input == ACTION_ABORT_SESSION)
// 				{
// 					// user wants to abort connection. 
// 					// TODO: confirm with dialog
// 
// 					// hang up by dropping DTR for at least 2 seconds
// 					//sprintf(global_string_buff1, "trying to hang up. DTR=%x, will be=%x", R8(UART_MCR), R8(UART_MCR) | FLAG_UART_MCR_DTR);
// 					sprintf(global_string_buff1, "trying to hang up...");
// 					Buffer_NewMessage(global_string_buff1);
// 					//R8(UART_MCR) = R8(UART_MCR) | FLAG_UART_MCR_DTR;
// 
// 					sprintf(global_string_buff1, "%s%c", (char*)"+++ATH", 13);	// needs return for modem to see it.
// 					Serial_SendData((uint8_t*)global_string_buff1, 7);	// then need to wait for "OK", then send "ATH"
// 
// 					return;
// 				}
				else if (user_input == ACTION_TEST_CONNECTWIFI)
				{
					Serial_SendData((uint8_t*)"AT+CWJAP_CUR=\"CenturyLink3346\",\"4fccre38f4d64k\"", 47);
					Serial_SendByte(0x0d);
					Serial_SendByte(0x0a);
				}
				else if (user_input == ACTION_TEST_CONNECTTFNS)
				{
					Serial_SendData((uint8_t*)"AT+CIPSTART=\"TCP\",\"tnfs.fujinet.online\",16384", 45);
					Serial_SendByte(0x0d);
					Serial_SendByte(0x0a);
				}
				else if (user_input == ACTION_TEST_MOUNTTFNS)
				{
					// To mount A: anonymously, using version 1.2 of the protocol:
					// 0x0000 0x00 0x00 0x02 0x01 A: 0x00 0x00 0x00
					Serial_SendData((uint8_t*)"AT+CIPSENDBUF=10", 16);
					Serial_SendByte(0x0d);
					Serial_SendByte(0x0a);
					//for (uint32_t i=0; i < 256*256*256; i++) {};
					Keyboard_GetChar();
					Serial_SendByte(0x00);
					Serial_SendByte(0x00);
					Serial_SendByte(0x00);
					Serial_SendByte(0x00);
					Serial_SendByte(0x02);
					Serial_SendByte(0x01);
					Serial_SendByte('/');
					Serial_SendByte(0x00);
					Serial_SendByte(0x00);
					Serial_SendByte(0x00);
					
				}
				else
				{
					Serial_SendByte(user_input);
					
					//sprintf(global_string_buff1, "key typed='%c', %x, %u", user_input, user_input, user_input);
					//Buffer_NewMessage(global_string_buff1);
					
					//Text_SetChar(user_input);
				}
			}

		} while (1==1);	//(user_input != '+');				
	} // while for exit loop
	
	// normal returns all handled above. this is a catch-all
	return;
}


// have serial change baud rate and show msg and label
void App_ChangeBaudRate(uint8_t new_config_index)
{
	global_current_baud_config = new_config_index;
	
	Serial_SetBaud(global_baud_config[global_current_baud_config].divisor_);

	App_EnterStealthTextUpdateMode();
	Buffer_NewMessage(General_GetString(global_baud_config[global_current_baud_config].msg_string_id_));	
	Text_DrawStringAtXY(TERM_BAUD_X1, TITLE_BAR_Y, General_GetString(global_baud_config[global_current_baud_config].lbl_string_id_), COLOR_BRIGHT_BLUE, COLOR_BLACK);
	App_ExitStealthTextUpdateMode();
}


// saves current cursor position and turns off visible cursor during non-serial UI updates
// call this when redrawing UI, updating baud display, etc, where you don't want cursor to leave terminal area
void App_EnterStealthTextUpdateMode(void)
{
	Sys_EnableTextModeCursor(false);
	Serial_ANSICursorSave();
}


// restores saved cursor position and turns on visible cursor after non-serial UI updates
// call this after redrawing UI, updating baud display, etc, where you don't want cursor to leave terminal area
void App_ExitStealthTextUpdateMode(void)
{
	Serial_ANSICursorRestore();
	Sys_EnableTextModeCursor(true);
}


/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/


// Draws the progress bar frame on the screen
void App_ShowProgressBar(void)
{
	Text_DrawHLine(TERM_PROGRESS_BAR_START_X, PROGRESS_BAR_Y - 1, TERM_PROGRESS_BAR_WIDTH, CH_UNDERSCORE, MENU_ACCENT_COLOR, APP_BACKGROUND_COLOR, CHAR_AND_ATTR);
	Text_DrawHLine(TERM_PROGRESS_BAR_START_X, PROGRESS_BAR_Y,     TERM_PROGRESS_BAR_WIDTH, CH_SPACE,      MENU_ACCENT_COLOR, APP_BACKGROUND_COLOR, CHAR_AND_ATTR);
	Text_DrawHLine(TERM_PROGRESS_BAR_START_X, PROGRESS_BAR_Y + 1, TERM_PROGRESS_BAR_WIDTH, CH_OVERSCORE,  MENU_ACCENT_COLOR, APP_BACKGROUND_COLOR, CHAR_AND_ATTR);
}

// Hides the progress bar frame on the screen
void App_HideProgressBar(void)
{
	Text_FillBox(TERM_PROGRESS_BAR_START_X, PROGRESS_BAR_Y - 1, (TERM_PROGRESS_BAR_START_X + TERM_PROGRESS_BAR_WIDTH - 1), PROGRESS_BAR_Y + 1, CH_SPACE, MENU_ACCENT_COLOR, APP_BACKGROUND_COLOR);
}


// draws the 'bar' part of the progress bar, according to a % complete passed (0-100 integer)
void App_UpdateProgressBar(uint8_t progress_bar_total)
{
	// logic:
	//  - has access to 40 positions worth of status: 5 characters each with 8 slots
	//  - so 2.5% complete = |, 5% = ||, 25%=||||||||  || (imagine PETSCII) 
	//  - draws spaces for not-yet-reached char spaces
	//  - integer division (with non-signed) goes to floor. so 15/8 = 1 r7. 1 is the number of full blocks, 7 (minus 1) is the index to the partial block char

	uint8_t		i;
	uint8_t		full_blocks;
	uint8_t		the_char_code;
	uint8_t		progress_bar_char_index;

	progress_bar_total = (progress_bar_total > 100) ? 100 : progress_bar_total;
	
	full_blocks = progress_bar_total / PROGRESS_BAR_DIVISOR;
	progress_bar_char_index = progress_bar_total % PROGRESS_BAR_DIVISOR; // remainders will be 0-7: index to app_progress_bar_char[]

	for (i = 0; i < PROGRESS_BAR_WIDTH; i++)
	{
		if (i < full_blocks)
		{
			the_char_code = CH_PROGRESS_BAR_FULL;
		}
		else if (i == full_blocks && progress_bar_char_index > 0)
		{
			the_char_code = app_progress_bar_char[progress_bar_char_index];
		}
		else
		{
			the_char_code = CH_SPACE;
		}

		Text_SetCharAndColorAtXY(PROGRESS_BAR_START_X + i, PROGRESS_BAR_Y, the_char_code, COLOR_PROGRESS_BAR, COLOR_BLACK);
	}
}


// // copy 256b chunks of data between specified 6502 addr and the fixed address range in EM, without bank switching
// // page_num is used to calculate distance from the base EM address
// // set to_em to true to copy from CPU space to EM, or false to copy from EM to specified CPU addr. PARAM_COPY_TO_EM/PARAM_COPY_FROM_EM
// void App_EMDataCopyDMA(uint8_t* cpu_addr, uint8_t page_num, bool to_em)
// {
// 	uint32_t	em_addr;	// physical memory address (20 bit)
// 	uint8_t		zp_em_addr_base;
// 	uint8_t		zp_cpu_addr_base;
// 	
// 
// 	// LOGIC:
// 	//   DMA will be used to copy directly from extended memory: no bank switching takes place
// 	//   sys address is physical machine 20-bit address.
// 	//   sys address is always relative to EM_STORAGE_START_PHYS_ADDR ($28000), based on page_num passed
// 	//     eg, if page_num=0, it is EM_STORAGE_START_PHYS_ADDR, if page_num is 8 it is EM_STORAGE_START_PHYS_ADDR + 8*256
// 	//   cpu address is $0000-$FFFF range that CPU can access
// 	
// 	if (to_em == true)
// 	{
// 		zp_em_addr_base = ZP_TO_ADDR;
// 		zp_cpu_addr_base = ZP_FROM_ADDR;
// 	}
// 	else
// 	{
// 		zp_em_addr_base = ZP_FROM_ADDR;
// 		zp_cpu_addr_base = ZP_TO_ADDR;
// 	}
// 	
// 	// add the offset to the base address for EM data get to the right place for this chunk's copy
// 	em_addr = (uint32_t)EM_STORAGE_START_PHYS_ADDR + (page_num * 256);
// 	
// 	//sprintf(global_string_buff1, "DMA copy page_num=%u, em_addr=%lu", page_num, em_addr);
// 	//Buffer_NewMessage(global_string_buff1);
// 	
// 	// set up the 20-bit address as either to/from
// 	*(uint16_t*)zp_em_addr_base = em_addr & 0xFFFF;
// 	*(uint8_t*)(zp_em_addr_base + 2) = ((uint32_t)EM_STORAGE_START_PHYS_ADDR >> 16) & 0xFF;
// 
// 	// set up the 16-bit address as either to/from
// 	*(char**)zp_cpu_addr_base = (char*)cpu_addr;
// 	*(uint8_t*)(zp_cpu_addr_base + 2) = 0;	// this buffer is in local / CPU memory, so: 0x00 0500 (etc)
// 
// 	// set copy length to 256 bytes
// 	*(char**)ZP_COPY_LEN = (char*)STORAGE_FILE_BUFFER_1_LEN;	
// 	*(uint8_t*)(ZP_COPY_LEN + 2) = 0;
// 
// 	//sprintf(global_string_buff1, "ZP_TO_ADDR=%02x,%02x,%02x; ZP_FROM_ADDR=%02x,%02x,%02x; ZP_COPY_LEN=%02x,%02x,%02x; ", *(uint8_t*)(ZP_TO_ADDR+0), *(uint8_t*)(ZP_TO_ADDR+1), *(uint8_t*)(ZP_TO_ADDR+2), *(uint8_t*)(ZP_FROM_ADDR+0), *(uint8_t*)(ZP_FROM_ADDR+1), *(uint8_t*)(ZP_FROM_ADDR+2), *(uint8_t*)(ZP_COPY_LEN+0), *(uint8_t*)(ZP_COPY_LEN+1), *(uint8_t*)(ZP_COPY_LEN+2));
// 	//Buffer_NewMessage(global_string_buff1);
// 
// 	Sys_SwapIOPage(VICKY_IO_PAGE_REGISTERS);	
//  	Memory_CopyWithDMA();	
// 	Sys_RestoreIOPage();
// }


// // copy 256b chunks of data between specified 6502 addr and the fixed address range in EM, using bank switching -- no DMA
// // em_bank_num is used to derive the base EM address
// // page_num is used to calculate distance from the base EM address
// // set to_em to true to copy from CPU space to EM, or false to copy from EM to specified CPU addr. PARAM_COPY_TO_EM/PARAM_COPY_FROM_EM
// void App_EMDataCopy(uint8_t* cpu_addr, uint8_t em_bank_num, uint8_t page_num, bool to_em)
// {
// 	//uint32_t	em_addr;			// physical memory address (20 bit)
// 	uint8_t		em_slot;			// 00-7F are valid slots, but our dedicated EM storage starts at bank $14
// 	uint8_t*	em_cpu_addr;		// A000-BFFF: the specific target address within the CPU address space that the EM memory has been banked into
// 	uint8_t		previous_overlay_bank_num;
// 	
// 
// 	// LOGIC:
// 	//   The overlay bank will be temporarily swapped out, and the required EM bank swapped in (tried to swap out kernel#2/IO bank, but froze up)
// 	//   required bank # can be calculated by taking system address and dividing by 8192. eg, 0x40000 / 0x2000 = bank 0x20
// 	//   sys address is physical machine 20-bit address.
// 	//   sys address is relative to em_bank_num, based on page_num passed
// 	//     eg, if em_bank_num=$14 (EM_STORAGE_START_PHYS_ADDR ($28000=bank 14)) and page_num=0, it is $28000, if page_num is 7 it is $28000 + 7*256
// 	//   cpu address is $0000-$FFFF range that CPU can access
// 	
// 	// add the offset to the base address get to the right place for this chunk's copy
// 	em_slot = em_bank_num + (page_num / 32);
// 	//em_addr = (uint32_t)(em_bank_num * 8192) + (page_num * 256));
// 	
// 	// CPU addr has to keep recycling space between A000-BFFF, so when chunk num is >32, need to get it back down under 32
// 	em_cpu_addr = (uint8_t*)((uint16_t)EM_STORAGE_START_CPU_ADDR + ((page_num % 32) * 256));
// 	
// 	//sprintf(global_string_buff1, "EM copy page_num=%u, em_slot=%u, em_cpu_addr=%p", page_num, em_slot, em_cpu_addr);
// 	//Buffer_NewMessage(global_string_buff1);
// 	
// 	// map the required EM bank into the overlay bank temporarily
// 	zp_bank_num = em_slot;
// 	previous_overlay_bank_num = Memory_SwapInNewBank(EM_STORAGE_START_SLOT);
// 	
// 	// copy data to/from
// 	if (to_em == true)
// 	{
// 		memcpy(em_cpu_addr, cpu_addr, 256);
// 	}
// 	else
// 	{
// 		memcpy(cpu_addr, em_cpu_addr, 256);
// 	}
// 	
// 	// map whatever overlay had been in place, back in place
// 	zp_bank_num = previous_overlay_bank_num;
// 	previous_overlay_bank_num = Memory_SwapInNewBank(EM_STORAGE_START_SLOT);	
// }


// read the real time clock and display it
void App_DisplayTime(void)
{
	// LOGIC: 
	//   f256jr has a built in real time clock (RTC)
	//   it works like this:
	//     1) you enable it with bit 0 of RND_CTRL
	//     2) you turn on seed mode by setting bit 1 of RND_CTRL to 1.
	//     3) you populate RNDL and RNDH with a seed
	//     4) you turn off see mode by unsetting bit 1 of RND_CTRL.
	//     5) you get random numbers by reading RNDL and RNDH. every time you read them, it repopulates them. 
	//     6) resulting 16 bit number you divide by 65336 (RAND_MAX_FOENIX) to get a number 0-1. 
	//   I will use the real time clock to seed the number generator
	//  The clock should only be visible and updated when the main 2-panel screen is displayed
	
	
	if (global_clock_is_visible != true)
	{
		return;
	}
	
	// need to have vicky registers available
	Sys_SwapIOPage(VICKY_IO_PAGE_REGISTERS);
	
	// stop RTC from updating external registers. Required!
	R8(RTC_CONTROL) = MASK_RTC_CTRL_UTI; // stop it from updating external registers
	
	// get year/month/day/hours/mins/second from RTC
	// below is subtly wrong, and i dno't care. don't need the datetime in a struct anyway.
	//global_datetime->year = R8(RTC_YEAR) & 0x0F + ((R8(RTC_YEAR) & 0xF0) >> 4) * 10;
	//global_datetime->month = R8(RTC_MONTH) & 0x0F + ((R8(RTC_MONTH) & 0x20) >> 4) * 10;
	//global_datetime->day = R8(RTC_DAY) & 0x0F + ((R8(RTC_DAY) & 0x30) >> 4) * 10;
	//global_datetime->hour = R8(RTC_HOURS) & 0x0F + ((R8(RTC_HOURS) & 0x30) >> 4) * 10;
	//global_datetime->min = R8(RTC_MINUTES) & 0x0F + ((R8(RTC_MINUTES) & 0x70) >> 4) * 10;
	//global_datetime->sec = R8(RTC_SECONDS) & 0x0F + ((R8(RTC_SECONDS) & 0x70) >> 4) * 10;

	sprintf(global_string_buff1, "20%02X-%02X-%02X %02X:%02X", R8(RTC_YEAR), R8(RTC_MONTH), R8(RTC_DAY), R8(RTC_HOURS), R8(RTC_MINUTES));
	
	// reset timer control to daylight savings, 24 hr model, and not battery saving mode, and clear UTI
	R8(RTC_CONTROL) = (MASK_RTC_CTRL_DSE | MASK_RTC_CTRL_12_24 | MASK_RTC_CTRL_STOP);

	Sys_DisableIOBank();
	
	// draw at upper/right edge of screen, on app title bar.
	App_EnterStealthTextUpdateMode();
	Text_DrawStringAtXY(TERM_DATE_X1, TITLE_BAR_Y, global_string_buff1, COLOR_BRIGHT_YELLOW, COLOR_BLACK);
	App_ExitStealthTextUpdateMode();
}


// Brings the requested overlay into memory
void App_LoadOverlay(uint8_t the_overlay_em_bank_number)
{
	zp_bank_num = the_overlay_em_bank_number;
	Memory_SwapInNewBank(OVERLAY_CPU_BANK);
}


// if ending on error: display error message, wait for user to confirm, and exit
// if no error, just exit
void App_Exit(uint8_t the_error_number)
{
	if (the_error_number != ERROR_NO_ERROR)
	{
		App_LoadOverlay(OVERLAY_SCREEN);
		sprintf(global_string_buff1, General_GetString(ID_STR_MSG_FATAL_ERROR), the_error_number);
		Screen_ShowUserTwoButtonDialog(
			global_string_buff1, 
			ID_STR_MSG_FATAL_ERROR_BODY, 
			ID_STR_DLG_OK, 
			ID_STR_DLG_OK
			);
	}

	// close log file if debugging flags were passed
	#if defined LOG_LEVEL_1 || defined LOG_LEVEL_2 || defined LOG_LEVEL_3 || defined LOG_LEVEL_4 || defined LOG_LEVEL_5
		General_LogCleanUp();
	#endif
	
	// turn cursor back on
	Sys_EnableTextModeCursor(true);
	
	R8(0xD6A2) = 0xDE;
	R8(0xD6A3) = 0xAD;
	R8(0xD6A0) = 0xF0;
	R8(0xD6A0) = 0x00;
	asm("JMP ($FFFC)");
}




int main(void)
{
	kernel_init();

	App_LoadOverlay(OVERLAY_STARTUP);
	
	if (Sys_InitSystem() == false)
	{
		App_Exit(0);
	}
	
	Sys_SetBorderSize(0, 0); // want all 80 cols and 60 rows!
	
	// initialize the comm buffer - do this before drawing UI or garbage will get written into comms area
	Buffer_Initialize();
	
	// initialize the random number generator embedded in the Vicky
	Startup_InitializeRandomNumGen();
	
	// set up pointers to string data that is in EM
	Startup_LoadString();

	// clear screen and draw logo
	Startup_ShowLogo();
	
	App_LoadOverlay(OVERLAY_SCREEN);
	
	App_Initialize();
	
	Keyboard_InitiateMinuteHand();
	App_DisplayTime();
	
	App_MainLoop();
	
	// restore screen, etc.
	App_Exit(ERROR_NO_ERROR);
	
	return 0;
}