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
#include "serial.h"
#include "app.h"
#include "comm_buffer.h"
#include "general.h"
#include "memory.h"
#include "text.h"
#include "screen.h"
#include "strings.h"
#include "sys.h"

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


#define UART_MAX_SEND_ATTEMPTS	1000


/*****************************************************************************/
/*                          File-scoped Variables                            */
/*****************************************************************************/


/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


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

// print a byte to screen, from the serial port
void Serial_PrintByte(uint8_t the_byte);



/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/

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
	


/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/





// set up UART for serial comms
void Serial_InitUART(void)
{
	Sys_SwapIOPage(VICKY_IO_PAGE_REGISTERS);
	R8(UART_LCR) = UART_DATA_BITS | UART_STOP_BITS | UART_PARITY | UART_NO_BRK_SIG;
	Serial_SetDLAB();
	R16(UART_DLL) = UART_BAUD_DIV_19200;
	Serial_ClearDLAB();
	Sys_RestoreIOPage();
}


// send a byte over the UART serial connection
// if the UART send buffer does not have space for the byte, it will try for UART_MAX_SEND_ATTEMPTS then return an error
// returns false on any error condition
bool Serial_SendByte(uint8_t the_byte)
{
	uint8_t		error_check;
	bool		uart_in_buff_is_empty = false;
	uint16_t	num_tries = 0;
	
	Sys_SwapIOPage(VICKY_IO_PAGE_REGISTERS);
	
	error_check = R8(UART_LSR) & UART_ERROR_MASK;
	
	if (error_check > 0)
	{
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
	
	for (i=0; i <= buffer_size; i++)
	{
		the_byte = the_buffer[i];
		
		if (Serial_SendByte(the_byte) == false)
		{
			return i;
		}
	}
	
	// add a line return if we got this far
	Serial_SendByte(0x0D);
	
	return i;
}


// get a byte from UART serial connection, or 1 ANSI sequence worth of bytes
uint8_t Serial_GetByte(void)
{
	bool		ansi_sequence_started = false;
	bool		data_available = false;
	bool		read_more = true;
	uint8_t		the_byte = 0;
	uint8_t		error_code;

	Sys_SwapIOPage(VICKY_IO_PAGE_REGISTERS);
		
// 	while (read_more == true)
// 	{
		error_code = R8(UART_LSR) & UART_ERROR_MASK;
		
		if (error_code > 0)
		{
			sprintf(global_string_buff1, "serial error %x", error_code);
			Buffer_NewMessage(global_string_buff1);
// 			read_more = false;
		}
		else
		{
			while ( (R8(UART_LSR) & UART_DATA_AVAILABLE) > 0)
			{
				//the_byte = R8(UART_BASE);
				Serial_PrintByte(R8(UART_BASE));
			}
// 			else
// 			{
// 				read_more = false;
// 			}
		}
// 	}
	
	Sys_RestoreIOPage();
	
	return the_byte;
}


// print a byte to screen, from the serial port
void Serial_PrintByte(uint8_t the_byte)
{
	uint8_t				fore_color = COLOR_ORANGE;
	uint8_t				back_color = COLOR_BLACK;

	if (the_byte == 0)
	{
		// error condition
		return;
	}
	else
	{
		//out(the_byte);
		// TODO: dedicated routine to display chars, handling movement back to nex tline, scroll screen, etc. 
		
		if (the_byte == 13)
		{
			zp_x = 0;
		}
		else if (the_byte == 10)
		{
			if (zp_y >= UI_VIEW_PANEL_BODY_Y2)
			{
				Text_ScrollTextAndAttrRowsUp(UI_VIEW_PANEL_BODY_Y1+1, UI_VIEW_PANEL_BODY_Y2);
				Text_FillBox(UI_LEFT_PANEL_BODY_X1, UI_VIEW_PANEL_BODY_Y2, UI_LEFT_PANEL_BODY_X2, UI_VIEW_PANEL_BODY_Y2, CH_SPACE, fore_color, back_color);
			}
			else
			{
				++zp_y;
			}
		}
		else
		{
			Text_SetCharAndColorAtXY(zp_x++, zp_y, the_byte, fore_color, back_color);
			
			if (zp_x > 79)
			{
				zp_x = 0;
				
				if (zp_y >= UI_VIEW_PANEL_BODY_Y2)
				{
					Text_ScrollTextAndAttrRowsUp(UI_VIEW_PANEL_BODY_Y1+1, UI_VIEW_PANEL_BODY_Y2);
					Text_FillBox(UI_LEFT_PANEL_BODY_X1, UI_VIEW_PANEL_BODY_Y2, UI_LEFT_PANEL_BODY_X2, UI_VIEW_PANEL_BODY_Y2, CH_SPACE, fore_color, back_color);
				}
				else
				{
					++zp_y;
				}
			}
		}
	}

	// update cursor position in VICKY so flashing cursor shows where we are
	asm("stz $01");
	(__A__ = *(uint8_t*)ZP_Y, asm("sta $d016"));
	(__A__ = *(uint8_t*)ZP_X, asm("sta $d014"));
}

// 25998 rem "get a byte from serial port if available, and store in in_byte" 
// 25999 rem "die on any error"
// 26000 proc getbyte()
// 26005 avail=0:in_byte=0:keepgetting=1:seq_started=0:seq$=""
// 26010 while keepgetting=1
// 26015   error=peek(ser_lsr)&is_error
// 26020   if error>0
// 26025     print:print "*** error on receive: "error:print:keepgetting=0
// 26030   else
// 26035     avail=peek(ser_lsr)&data_available
// 26040     if avail>0
// 26045       in_byte=peek(uart)
// 26050       if (seq_started=0) & (in_byte<>27)
// 26055         keepgetting=0:rem "normal text"
// 26060       else
// 26065         rem "either new ANSI sequence started or we were in one"
// 
// 26070         if seq_started=1
// 26075           seq$ = seq$ + chr$(in_byte)
// 
// 26080           if ((in_byte>64) & )in_byte<91)) | ((in_byte>96) & (in_byte<123))
// 26085             rem "sequence is now complete and judged valid"
// 26090             seq_started=0
// 26095             processSequence(seq$)
// 26100             seq_started=0:seq$="":in_byte=0:rem "so that we don't try to print it on return
// 26105             keepgetting=0
// 26110           else
// 26115             rem "make sure that sequence starts esc + ["
// 26120             if (in_byte=91) & (len(seq$)<>0))
// 26125               rem "invalid sequence, we needed [ in 1st slot"
// 26130               print seq$:rem "if invalid, it may just be text, print it all"
// 26135               seq_started=0:seq$="":in_byte=0:keepgetting=0
// 26140             endif
// 26145           endif
// 
// 26150         else
// 26155           rem "we weren't in a sequence, but just got esc."
// 26160           seq_started=0
// 26165         endif
// 
// 26170       endif
// 26200     else
// 26205       keepgetting=0
// 26210     endif 
// 26215   endif
// 26220 wend
// 26225 endproc 
