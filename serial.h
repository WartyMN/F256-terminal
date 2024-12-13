//! @file serial.h

/*
 * serial.h
 *
 *  Created on: Apr 1, 2024
 *      Author: micahbly
 *
 */



#ifndef SERIAL_H_
#define SERIAL_H_


/* about this class
 *
 *
 *
 *** things this class needs to be able to do
 * initialize the UART chip for serial comms
 * send a byte over the serial port
 * send a string over the serial port
 * received a byte from the serial port
 *
 *** things objects of this class have
 *
 *
 */


/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// project includes

// C includes
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// cc65 includes

/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define NUM_ANSI_CODES			19

//#define UART_BUFFER_SIZE		8192	// size of the circular buffer offloading serial data
//#define UART_BUFFER_MASK		(UART_BUFFER_SIZE - 1)

// ANSI color codes
#define ANSI_COLOR_BLACK			(uint8_t)0x00
#define ANSI_COLOR_RED				(uint8_t)0x01
#define ANSI_COLOR_GREEN			(uint8_t)0x02
#define ANSI_COLOR_YELLOW			(uint8_t)0x03
#define ANSI_COLOR_BLUE				(uint8_t)0x04
#define ANSI_COLOR_MAGENTA			(uint8_t)0x05
#define ANSI_COLOR_CYAN				(uint8_t)0x06
#define ANSI_COLOR_WHITE			(uint8_t)0x07

#define ANSI_COLOR_BRIGHT_BLACK		(uint8_t)0x08
#define ANSI_COLOR_BRIGHT_RED		(uint8_t)0x09
#define ANSI_COLOR_BRIGHT_GREEN		(uint8_t)0x0a
#define ANSI_COLOR_BRIGHT_YELLOW	(uint8_t)0x0b
#define ANSI_COLOR_BRIGHT_BLUE		(uint8_t)0x0c
#define ANSI_COLOR_BRIGHT_MAGENTA	(uint8_t)0x0d
#define ANSI_COLOR_BRIGHT_CYAN		(uint8_t)0x0e
#define ANSI_COLOR_BRIGHT_WHITE		(uint8_t)0x0f

/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/

typedef enum ansi_action
{
	ANSI_FG_BLACK				= 0,	
	ANSI_FG_RED 				,
	ANSI_FG_GREEN				,
	ANSI_FG_YELLOW				,
	ANSI_FG_BLUE				,
	ANSI_FG_MAGENTA				,
	ANSI_FG_CYAN				,
	ANSI_FG_WHITE				,
	ANSI_FG_DEFAULT				,
	ANSI_FG_RESET				,
	ANSI_BG_BLACK				,	
	ANSI_BG_RED 				,
	ANSI_BG_GREEN				,
	ANSI_BG_YELLOW				,
	ANSI_BG_BLUE				,
	ANSI_BG_MAGENTA				,
	ANSI_BG_CYAN				,
	ANSI_BG_WHITE				,
	ANSI_BG_DEFAULT				,
	ANSI_BG_RESET				,
	
	ANSI_UNRECOGNIZED			,
} ansi_action;


/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/

typedef struct ANSIcode {
	char*			code_;
	ansi_action		action_;
} ANSIcode;

/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/


// set up ANSI colors
void Serial_InitANSIColors(void);

// set up UART for serial comms
void Serial_InitUART(void);

// change baud rate
// new_baud_rate_divisor must be UART_BAUD_DIV_4800, UART_BAUD_DIV_9600, etc.
void Serial_SetBaud(uint16_t new_baud_rate_divisor);

// send 1-255 bytes to the UART serial connection
// returns # of bytes successfully sent (which may be less than number requested, in event of error, etc.)
uint8_t Serial_SendData(uint8_t* the_buffer, uint16_t buffer_size);

// send a byte over the UART serial connection
// if the UART send buffer does not have space for the byte, it will try for UART_MAX_SEND_ATTEMPTS then return an error
// returns false on any error condition
bool Serial_SendByte(uint8_t the_byte);

// Check for serial data in the UART and transfer to the circular buffer for later processing
// Returns true on data available or on no data, but no error. Returns false on any error. 
bool Serial_ReadUART(void);

// Check for available data in the UART circular buffer and process any that are available.
// returns false if no bytes were available
bool Serial_ProcessAvailableData(void);

// flush the in (Rx) buffer
// resets circular buffer pointers so that any not-yet-processed bytes are forgotten about
void Serial_FlushInBuffer(void);

// save current cursor position
void Serial_ANSICursorSave(void);

// restore cursor position from last save
void Serial_ANSICursorRestore(void);

// cycle to the next foreground color, updating every cell in the terminal screen
void Serial_CycleForegroundColor(void);

// // start YMODEM file receive
// bool Serial_StartYModemReceive(void);



// debug function - dump serial buffer to disk
bool Serial_DebugDump(void);


#endif /* SERIAL_H_ */
