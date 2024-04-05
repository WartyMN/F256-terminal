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
//#include "text.h"

// C includes
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// cc65 includes

/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/


/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/


/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/


/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/



// set up UART for serial comms
void Serial_InitUART(void);

// send 1-255 bytes to the UART serial connection
// returns # of bytes successfully sent (which may be less than number requested, in event of error, etc.)
uint8_t Serial_SendData(uint8_t* the_buffer, uint16_t buffer_size);

// send a byte over the UART serial connection
// if the UART send buffer does not have space for the byte, it will try for UART_MAX_SEND_ATTEMPTS then return an error
// returns false on any error condition
bool Serial_SendByte(uint8_t the_byte);

// get a byte from UART serial connection, or 1 ANSI sequence worth of bytes
uint8_t Serial_GetByte(void);



#endif /* SERIAL_H_ */
