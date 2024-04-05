/*
 * f256jr.h
 *
 *  Created on: November 29, 2022
 *      Author: micahbly
 */

#ifndef F256JR_H_
#define F256JR_H_



/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

#include "api.h"
#include <stdint.h>


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/


// adapted from vinz67
#define R8(x)						*((volatile uint8_t* const)(x))			// make sure we read an 8 bit byte; for VICKY registers, etc.
#define P8(x)						(volatile uint8_t* const)(x)			// make sure we read an 8 bit byte; for VICKY registers, etc.
#define R16(x)						*((volatile uint16_t* const)(x))		// make sure we read an 16 bit byte; for RNG etc.


// ** F256jr MMU
#define MMU_MEM_CTRL					0x0000	// bit 0-1: activate LUT (exit editing); 4-5 LUT to be edited; 7: activate edit mode
#define MMU_IO_CTRL						0x0001	// bits 0-1: IO page; bit 2: disable IO

#define BANK_IO							0x06	// 0xc000-0xdfff
#define BANK_KERNAL						0x07	// 0xe000-0xffff


// ** F256jr - Tiny VICKY

#define TEXT_COL_COUNT_FOR_PLOTTING		80	// regardless of visible cols (between borders), VRAM is fixed at 80 cols across.
#define TEXT_ROW_COUNT_FOR_PLOTTING		60	// regardless of visible rows (between borders), VRAM is fixed at 60 rows up/down.

#define TEXT_ROW_COUNT_60HZ			60
#define TEXT_ROW_COUNT_70HZ			50

#define TEXT_FONT_WIDTH				8	// for text mode, the width of the fixed-sized font chars
#define TEXT_FONT_HEIGHT			8	// for text mode, the height of the fixed-sized font chars.
#define TEXT_FONT_BYTE_SIZE			(8*256)

#define VIDEO_MODE_FREQ_BIT			0x01	//!> the bits in the 2nd byte of the system control register that define video mode (resolution). if this bit is set, resolution is 70hz 320x200 (text mode 80*25). if clar, is 60hz 630*240
#define VIDEO_MODE_DOUBLE_X_BIT		0x02	//!> the bits in the 2nd byte of the system control register control text mode doubling in horizontal. if set, text mode chars are doubled in size, producing 40 chars across
#define VIDEO_MODE_DOUBLE_Y_BIT		0x04	//!> the bits in the 3rd byte of the system control register control text mode doubling in vertical. if set, text mode chars are doubled in size, producing 25 or 30 chars down

#define GAMMA_MODE_DIPSWITCH_BIT	0x20	//!>the bits in the 2nd byte of the system control register reflect dip switch setting for control gamma correction on/off
#define GAMMA_MODE_ONOFF_BITS		0x03	//!>the bits in the 3rd byte of the system control register control gamma correction on/off


// Tiny VICKY I/O pages
#define VICKY_IO_PAGE_REGISTERS			0	// Low level I/O registers
#define VICKY_IO_PAGE_FONT_AND_LUTS		1	// Text display font memory and graphics color LUTs
#define VICKY_IO_PAGE_CHAR_MEM			2	// Text display character matrix
#define VICKY_IO_PAGE_ATTR_MEM			3	// Text display color matrix

// Tiny VICKY I/O page 0 addresses
#define VICKY_BASE_ADDRESS				0xd000		// Tiny VICKY offset/first register
#define VICKY_MASTER_CTRL_REG_L			0xd000		// Tiny VICKY Master Control Register - low - graphic mode/text mode/etc.
#define VICKY_MASTER_CTRL_REG_H			0xd001		// Tiny VICKY Master Control Register - high - screen res, etc.
#define VICKY_GAMMA_CTRL_REG			0xd002		// Tiny VICKY Gamma Control Register
#define VICKY_BORDER_CTRL_REG			0xd004		// Tiny VICKY Border Control Register
#define VICKY_BORDER_COLOR_B			0xd005		// Tiny VICKY Border Color Blue
#define VICKY_BORDER_COLOR_G			0xd006		// Tiny VICKY Border Color Green
#define VICKY_BORDER_COLOR_R			0xd007		// Tiny VICKY Border Color Red
#define VICKY_BORDER_X_SIZE				0xd008		// Tiny VICKY Border X size in pixels
#define VICKY_BORDER_Y_SIZE				0xd009		// Tiny VICKY Border Y size in pixels
#define VICKY_BACKGROUND_COLOR_B		0xd00d		// Tiny VICKY background color Blue
#define VICKY_BACKGROUND_COLOR_G		0xd00e		// Tiny VICKY background color Green
#define VICKY_BACKGROUND_COLOR_R		0xd00f		// Tiny VICKY background color Red
#define VICKY_TEXT_CURSOR_ENABLE		0xd010		// bit 2 is enable/disable
#define BITMAP_CTRL						0xd100		//!> bitmap control register	
#define BITMAP_L0_VRAM_ADDR_L			0xd101		//!> bitmap VRAM address pointer)		
#define BITMAP_L0_VRAM_ADDR_M			0xd102		//!> bitmap VRAM address pointer)		
#define BITMAP_L0_VRAM_ADDR_H			0xd103		//!> bitmap VRAM address pointer)		
#define TILE_CTRL						0xd200		//!> tile control register		


// ** serial comms related

#define UART_BASE						0xd630		// starting point of serial-related registers
#define UART_RBR						(UART_BASE + 0)
#define UART_IER						(UART_BASE + 1)
#define UART_IIR						(UART_BASE + 2)
#define UART_LCR						(UART_BASE + 3)
#define UART_MCR						(UART_BASE + 4)
#define UART_LSR						(UART_BASE + 5)
#define UART_MSR						(UART_BASE + 6)
#define UART_SCR						(UART_BASE + 7)

#define UART_THR						(UART_BASE + 0)	// write register when DLAB=0
#define UART_FCR						(UART_BASE + 2)	// write register when DLAB=0
#define UART_DLL						(UART_BASE + 0)	// read/write register when DLAB=1
#define UART_DLM						(UART_BASE + 1)	// read/write register when DLAB=1

#define UART_BAUD_DIV_300		5244	// divisor for 300 baud
#define UART_BAUD_DIV_600		2622	// divisor for 600 baud
#define UART_BAUD_DIV_1200		1311	// divisor for 1200 baud
#define UART_BAUD_DIV_1800		874		// divisor for 1800 baud
#define UART_BAUD_DIV_2000		786		// divisor for 2000 baud
#define UART_BAUD_DIV_2400		655		// divisor for 2400 baud
#define UART_BAUD_DIV_3600		437		// divisor for 3600 baud
#define UART_BAUD_DIV_4800		327		// divisor for 4800 baud
#define UART_BAUD_DIV_9600		163		// divisor for 9600 baud
#define UART_BAUD_DIV_19200		81		// divisor for 19200 baud
#define UART_BAUD_DIV_38400		40		// divisor for 38400 baud
#define UART_BAUD_DIV_57600		27		// divisor for 57600 baud
#define UART_BAUD_DIV_115200	13		// divisor for 115200 baud

#define UART_DATA_BITS			0b00000011	// 8 bits
#define UART_STOP_BITS			0			// 1 stop bit
#define UART_PARITY				0			// no parity
#define UART_BRK_SIG			0b01000000
#define UART_NO_BRK_SIG			0b00000000
#define UART_DLAB_MASK			0b10000000
#define UART_THR_IS_EMPTY		0b00100000
#define UART_THR_EMPTY_IDLE		0b01000000
#define UART_DATA_AVAILABLE		0b00000001
#define UART_ERROR_MASK			0b10011110



#define VICKY_PS2_INTERFACE				0xd640

#define RTC_SECONDS						0xd690		//  654: second digit, 3210: 1st digit
#define RTC_MINUTES						0xd692		//  654: second digit, 3210: 1st digit
#define RTC_HOURS						0xd694		//   54: second digit, 3210: 1st digit
#define RTC_DAY							0xd696		//   54: second digit, 3210: 1st digit
#define RTC_MONTH						0xd699		//    4: second digit, 3210: 1st digit
#define RTC_YEAR						0xd69a		// 7654: second digit, 3210: 1st digit
#define RTC_CONTROL						0xd69e		// set bit 3 to disable update of reg, to read secs. 
#define RANDOM_NUM_GEN_LOW				0xd6a4		// both the SEEDL and the RNDL (depends on bit 1 of RND_CTRL)
#define RANDOM_NUM_GEN_HI				0xd6a5		// both the SEEDH and the RNDH (depends on bit 1 of RND_CTRL)
#define RANDOM_NUM_GEN_ENABLE			0xd6a6		// bit 0: enable/disable. bit 1: seed mode on/off. "RND_CTRL"

#define MACHINE_ID_REGISTER				0xd6a7		// will be '2' for F256JR
#define MACHINE_PCB_ID_0				0xd6a8
#define MACHINE_PCB_ID_1				0xd6a9
#define MACHINE_PCB_MAJOR				0xd6eb		// error in manual? this and next 4 all show same addr. changing here to go up by 1.
#define MACHINE_PCB_MINOR				0xd6ec
#define MACHINE_PCB_DAY					0xd6ed
#define MACHINE_PCB_MONTH				0xd6ef
#define MACHINE_PCB_YEAR				0xd6f0
#define MACHINE_FPGA_SUBV_LOW			0xd6aa		// CHSV0 chip subversion in BCD (low)
#define MACHINE_FPGA_SUBV_HI			0xd6ab		// CHSV1 chip subversion in BCD (high)
#define MACHINE_FPGA_VER_LOW			0xd6ac		// CHV0 chip version in BCD (low)
#define MACHINE_FPGA_VER_HI				0xd6ad		// CHV1 chip version in BCD (high)
#define MACHINE_FPGA_NUM_LOW			0xd6ae		// CHN0 chip number in BCD (low)
#define MACHINE_FPGA_NUM_HI				0xd6af		// CHN1 chip number in BCD (high)

#define DMA_CTRL						0xdf00		// VICKY's DMA control register
#define DMA_STATUS						0xdf01		// DMA status register (Read Only)
#define DMA_SRC_ADDR					0xdf04		// Source address (system bus - 3 byte)
#define DMA_DST_ADDR					0xdf08		// Destination address (system bus - 3 byte)
#define DMA_COUNT						0xdf0c		// Number of bytes to fill or copy - must be EVEN

#define TEXT_FORE_LUT					0xd800		// FG_CHAR_LUT_PTR	Text Foreground Look-Up Table
#define TEXT_BACK_LUT					0xd840		// BG_CHAR_LUT_PTR	Text Background Look-Up Table

// Tiny VICKY I/O page 1 addresses
#define FONT_MEMORY_BANK0				0xc000		// FONT_MEMORY_BANK0	FONT Character Graphic Mem (primary)
#define FONT_MEMORY_BANK1				0xc800		// FONT_MEMORY_BANK1	FONT Character Graphic Mem (secondary)
#define VICKY_CLUT0						0xd000		// each addition LUT is 400 offset from here
#define VICKY_CLUT1						(VICKY_CLUT0 + 0x400)	// each addition LUT is 400 offset from here
#define VICKY_CLUT2						(VICKY_CLUT1 + 0x400)	// each addition LUT is 400 offset from here
#define VICKY_CLUT3						(VICKY_CLUT2 + 0x400)	// each addition LUT is 400 offset from here

// Tiny VICKY I/O page 2 addresses
#define VICKY_TEXT_CHAR_RAM					(char*)0xc000			// in I/O page 2

// Tiny VICKY I/O page 3 addresses
#define VICKY_TEXT_ATTR_RAM					(char*)0xc000			// in I/O page 3

#define GRAPHICS_MODE_TEXT		0x01	// 0b00000001	Enable the Text Mode
#define GRAPHICS_MODE_TEXT_OVER	0x02	// 0b00000010	Enable the Overlay of the text mode on top of Graphic Mode (the Background Color is ignored)
#define GRAPHICS_MODE_GRAPHICS	0x04	// 0b00000100	Enable the Graphic Mode
#define GRAPHICS_MODE_EN_BITMAP	0x08	// 0b00001000	Enable the Bitmap Module In Vicky
#define GRAPHICS_MODE_EN_TILE	0x10	// 0b00010000	Enable the Tile Module in Vicky
#define GRAPHICS_MODE_EN_SPRITE	0x20	// 0b00100000	Enable the Sprite Module in Vicky
#define GRAPHICS_MODE_EN_GAMMA	0x40	// 0b01000000	Enable the GAMMA correction - The Analog and DVI have different color values, the GAMMA is great to correct the difference
#define GRAPHICS_MODE_DIS_VIDEO	0x80	// 0b10000000	This will disable the Scanning of the Video information in the 4Meg of VideoRAM hence giving 100% bandwidth to the CPU


// VICKY RESOLUTION FLAGS
#define VICKY_RES_320X240_FLAGS		0x00	// 0b00000000
#define VICKY_PIX_DOUBLER_FLAGS		0x02	// 0b00000001
#define VICKY_RES_320X200_FLAGS		0x03	// 0b00000011

#define RES_320X200		0
#define RES_320X240		1

// machine model numbers - for decoding s_sys_info.model - value read from MACHINE_ID_REGISTER (see above)
#define MACHINE_C256_FMX		0	///< for s_sys_info.model
#define MACHINE_C256_U			1	///< for s_sys_info.model
#define MACHINE_F256_JR			0x02	///< for s_sys_info.model
#define MACHINE_C256_GENX		4	///< for s_sys_info.model
#define MACHINE_C256_UPLUS		5	///< for s_sys_info.model
#define MACHINE_A2560U_PLUS		6	///< for s_sys_info.model
#define MACHINE_A2560X			7	///< for s_sys_info.model
#define MACHINE_A2560U			9	///< for s_sys_info.model
#define MACHINE_F256K			0x12	///< for s_sys_info.model
#define MACHINE_A2560K			0x0b	///< for s_sys_info.model

#define MACHINE_MODEL_MASK		0x1F		

typedef uint8_t	ColorIdx;


/*****************************************************************************/
/*                              Key Definitions                              */
/*****************************************************************************/

#define CH_F1      		0x81	
#define CH_F2      		0x82	
#define CH_F3      		0x83	
#define CH_F4      		0x84	
#define CH_F5      		0x85	
#define CH_F6      		0x86	
#define CH_F7      		0x87	
#define CH_F8     		0x88	
#define CH_F9      		0x89	
#define CH_F10     		0x8a	
#define CH_F11     		0x8b	
#define CH_F12    		0x8c	
#define CH_CURS_UP      0x10
#define CH_CURS_DOWN    0x0e
#define CH_CURS_LEFT    0x02
#define CH_CURS_RIGHT   0x06
#define CH_DEL          0x08
#define CH_ENTER        13
#define CH_ESC          27
#define CH_TAB          9
#define CH_SPACE		32
#define CH_RUNSTOP		3
#define CH_COPYRIGHT	215
#define CH_ZERO			48		// this is just so we can do match to convert a char '5' into an integer 5, etc.
#define CH_NINE			57		// for number ranges when converting user input to numbers

/*****************************************************************************/
/*                             Named Characters                              */
/*****************************************************************************/

#define SC_HLINE        150
#define SC_VLINE        130
#define SC_ULCORNER     160
#define SC_URCORNER     161
#define SC_LLCORNER     162
#define SC_LRCORNER     163
#define SC_ULCORNER_RND 188
#define SC_URCORNER_RND 189
#define SC_LLCORNER_RND 190
#define SC_LRCORNER_RND 191
#define SC_CHECKERED	199
#define SC_T_DOWN		155 // T-shape pointing down
#define SC_T_UP			157 // T-shape pointing up
#define SC_T_LEFT		158 // T-shape pointing left
#define SC_T_RIGHT		154 // T-shape pointing right
#define SC_T_JUNCTION	156 // + shape meeting of 4 lines

/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/


/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/

typedef struct DateTime {
    uint8_t			year;
    uint8_t			month;
    uint8_t			day;
    uint8_t			hour;
    uint8_t			min;
    uint8_t			sec;
} DateTime;

/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/





#endif /* F256JR_H_ */
