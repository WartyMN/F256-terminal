/*
 * memory.h
 *
 *  Created on: December 4, 2022
 *      Author: micahbly
 */

#ifndef MEMORY_H_
#define MEMORY_H_




/* about this class
 *
 * this header represents a set of assembly functions in memory.asm
 * the functions in this header file are all related to moving memory between physical F256jr RAM and MMU-mapped 6502 RAM
 * these functions neeed to be in the MAIN segment so they are always available
 * all functions that modify the LUT will reset it to its original configurate before exiting
 *
 */

/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

#include "app.h"


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define ZP_BANK_SLOT		0x10	// zero-page address holding the LUT slot to be modified (0-7) (eg, if 0, will be $08,if 1, $09, etc.)
#define ZP_BANK_NUM			0x11	// zero-page address holding the new LUT bank# to be set in the ZP_BANK_SLOT
#define ZP_OLD_BANK_NUM		0x12	// zero-page address holding the original LUT bank # before being changed
#define ZP_TO_ADDR			0x13	// zero-page address holding address to copy to, for ML copy routine (3b)
#define ZP_FROM_ADDR		0x16	// zero-page address holding address to copy from, for ML copy routine (3b)
#define ZP_COPY_LEN			0x19	// zero-page address holding number of bytes to copy, for ML copy routine (3b)
#define ZP_PHYS_ADDR_LO		0x1C	// zero-page address pointing to a 20-bit physical memory address
#define ZP_PHYS_ADDR_MI		0x1D	// zero-page address pointing to a 20-bit physical memory address
#define ZP_PHYS_ADDR_HI		0x1E	// zero-page address pointing to a 20-bit physical memory address
#define ZP_CPU_ADDR_LO		0x1F	// zero-page address pointing to a 16-bit address in 6502 memory space (virtual 64k)
#define ZP_CPU_ADDR_HI		0x20	// zero-page address pointing to a 16-bit address in 6502 memory space (virtual 64k)
#define ZP_X				0x21	// zero-page address holding the x coord for serial-to-screen output
#define ZP_Y				0x22	// zero-page address holding the y coord for serial-to-screen output
#define ZP_SEARCH_LOC_BANK	0x23	// zero-page address holding the current search location's bank number
#define ZP_TEMP_1			0x24	// zero-page address we will use for temp variable storage in assembly routines
#define ZP_OTHER_PARAM		0x25	// zero-page address we will use for communicating 1 byte to/from assembly routines
#define ZP_OLD_IO_PAGE		0x26	// zero-page address holding the original IO page # before being changed
//#define ZP_X				0x13	// zero-page address we will use for passing X coordinate to assembly routines
//#define ZP_Y				0x14	// zero-page address we will use for passing Y coordinate to assembly routines
//#define ZP_SCREEN_ID		0x15	// zero-page address we will use for passing the screen ID to assembly routines


// starting point for all storage to extended memory. if larger than 8K, increment as necessary
#define EM_STORAGE_START_CPU_ADDR			0xA000		// when copying file data to EM, the starting CPU address (16 bit)
#define EM_STORAGE_START_PHYS_ADDR			0x28000		// when copying file data to EM, the starting physical address (20 bit)
//#define EM_STORAGE_START_SLOT				0x06		// the 0-7 local CPU slot to map it into - i/o + kernel#2 slot
#define EM_STORAGE_START_SLOT				0x05		// the 0-7 local CPU slot to map it into - overlay slot
#define EM_STORAGE_START_PHYS_BANK_NUM		0x14		// the system physical bank number/slot where EM storage starts for us.

#define STORAGE_INTERBANK_BUFFER		0x0400	// 1-page buffer. see cc65 memory config file. this is outside cc65 space.
#define STORAGE_INTERBANK_BUFFER_LEN	0x0100	// 1-page buffer. see cc65 memory config file. this is outside cc65 space.
#define STORAGE_UART_BUFFER				0x0500	// outside of cc65 memory space. see config file. 
#define STORAGE_UART_BUFFER_LEN			0x299	// = 665. purely based on memory map config so that startup is at 799. just because that matches f/manager jr. all changeable. 

#define UART_BUFFER_START_ADDR			STORAGE_UART_BUFFER
#define UART_BUFFER_SIZE				STORAGE_UART_BUFFER_LEN

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

// call to a routine in memory.asm that modifies the MMU LUT to bring the specified bank of physical memory into the CPU's RAM space
// set zp_bank_num before calling.
// returns the slot that had been mapped previously
uint8_t __fastcall__ Memory_SwapInNewBank(uint8_t the_bank_slot);

// call to a routine in memory.asm that modifies the MMU LUT to bring the back the previously specified bank of physical memory into the CPU's RAM space
// relies on a previous routine having set ZP_OLD_BANK_NUM. Should be called after Memory_SwapInNewBank(), when finished with the new bank
// set zp_bank_num before calling.
void __fastcall__ Memory_RestorePreviousBank(uint8_t the_bank_slot);

// call to a routine in memory.asm that returns whatever is currently mapped in the specified MMU slot
// set zp_bank_num before calling.
// returns the slot that had been mapped previously
uint8_t __fastcall__ Memory_GetMappedBankNum(void);

// call to a routine in memory.asm that writes an illegal opcode followed by address of debug buffer
// that is a simple to the f256jr emulator to write the string at the debug buffer out to the console
//void __fastcall__ Memory_DebugOut(void);

// call to a routine in memory.asm that copies specified number of bytes from src to dst
// set zp_to_addr, zp_from_addr, zp_copy_len before calling.
// credit: http://6502.org/source/general/memory_move.html
// void __fastcall__ Memory_Copy(void);

// call to a routine in memory.asm that copies specified number of bytes from src to dst
// set zp_to_addr, zp_from_addr, zp_copy_len before calling.
// this version uses the F256's DMA capabilities to copy, so addresses can be 24 bit (system memory, not CPU memory)
// in other words, no need to page either dst or src into CPU space
//void __fastcall__ Memory_CopyWithDMA(void);

// call to a routine in memory.asm that fills the specified number of bytes to the dst
// set zp_to_addr, zp_copy_len to num bytes to fill, and zp_other_byte to the fill value before calling.
// this version uses the F256's DMA capabilities to fill, so addresses can be 24 bit (system memory, not CPU memory)
// in other words, no need to page either dst into CPU space
//void __fastcall__ Memory_FillWithDMA(void);


#endif /* MEMORY_H_ */
