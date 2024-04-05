/*
 * app.h
 *
 *  Created on: Jan 10, 2023
 *      Author: micahbly
 *
 *  A pseudo commander-style 2-column file manager
 *
 */
 
#ifndef FILE_MANAGER_H_
#define FILE_MANAGER_H_

/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/


// project includes

// C includes
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
//#include <string.h>



/*****************************************************************************/
/*                               Definitions                                 */
/*****************************************************************************/

#define PARAM_COPY_TO_EM			true	// App_EMDataCopy() parameter
#define PARAM_COPY_FROM_EM			false	// App_EMDataCopy() parameter

// hide __fastcall_ from everything but CC65 (to squash some warnings in LSP/BBEdit)
#ifndef __CC65__
	#define __fastcall__ 
#endif

#ifndef NULL
#  define NULL 0
#endif

#define MAJOR_VERSION	0
#define MINOR_VERSION	1
#define UPDATE_VERSION	1

#define VERSION_NUM_X	0
#define VERSION_NUM_Y	24

#define APP_DIALOG_WIDTH					42
#define APP_DIALOG_HEIGHT					7
#define APP_DIALOG_STARTING_NUM_BUTTONS		2
#define APP_DIALOG_BUFF_SIZE				((APP_DIALOG_WIDTH+2) * (APP_DIALOG_HEIGHT+2))	// for the temp char and color buffs when opening a window, this is how much mem we'll reserve for each

#define FILE_MAX_FILENAME_SIZE_CBM	(16+1)	// CBM DOS defined
#define FILE_MAX_FILENAME_SIZE		(40+1)	// in F256 kernel, total path can't be longer than 255 chars.
#define FILE_MAX_PATHNAME_SIZE		(255)	// in F256 kernel, total path can't be longer than 255 chars.
#define FILE_MAX_APPFILE_INFO_SIZE	255		// for info panel display about mime/app type, the max # of bytes to display
#define FILE_MAX_TEXT_PREVIEW_SIZE	255		// for info panel previews, the max # of bytes to read in and display


/*****************************************************************************/
/*                             MEMORY LOCATIONS                              */
/*****************************************************************************/

// temp storage for data outside of normal cc65 visibility - extra memory!
#define STORAGE_GETSTRING_BUFFER			0x0400	// interbank buffer to store individual strings retrieved from EM
#define STORAGE_GETSTRING_BUFFER_LEN		256	// 1-page buffer. see cc65 memory config file. this is outside cc65 space.
#define STORAGE_FILE_BUFFER_1				0x0500	// interbank buffer for file reading operations
#define STORAGE_FILE_BUFFER_1_LEN			256	// 1-page buffer. see cc65 memory config file. this is outside cc65 space.
#define STORAGE_STRING_BUFFER_1				(STORAGE_FILE_BUFFER_1 + STORAGE_FILE_BUFFER_1_LEN)	// temp string merge/etc buff
#define STORAGE_STRING_BUFFER_1_LEN			204	// 204b buffer. see cc65 memory config file. this is outside cc65 space.
#define STORAGE_STRING_BUFFER_2				(STORAGE_STRING_BUFFER_1 + STORAGE_STRING_BUFFER_1_LEN)	// temp string merge/etc buff
#define STORAGE_STRING_BUFFER_1_LEN			204	// 204b buffer. 
#define STORAGE_TEMP_UNUSED_1B				(STORAGE_STRING_BUFFER_2 + STORAGE_STRING_BUFFER_2_LEN)	// 799 is hard coded, so this is just noting that we have 1 unused byte here.


#define STRING_STORAGE_SLOT                0x06
#define STRING_STORAGE_VALUE               0x12
#define STRING_STORAGE_PHYS_ADDR           0x24000



/*****************************************************************************/
/*                           App-wide color choices                          */
/*****************************************************************************/

#define APP_FOREGROUND_COLOR		COLOR_BRIGHT_BLUE
#define APP_BACKGROUND_COLOR		COLOR_BLACK
#define APP_ACCENT_COLOR			COLOR_BLUE

#define BUFFER_FOREGROUND_COLOR		COLOR_CYAN
#define BUFFER_BACKGROUND_COLOR		COLOR_BLACK
#define BUFFER_ACCENT_COLOR			COLOR_BRIGHT_BLUE

#define PANEL_FOREGROUND_COLOR		COLOR_GREEN
#define PANEL_BACKGROUND_COLOR		COLOR_BLACK
#define PANEL_ACCENT_COLOR			COLOR_BRIGHT_BLUE

#define LIST_ACTIVE_COLOR			COLOR_BRIGHT_GREEN
#define LIST_INACTIVE_COLOR			COLOR_GREEN

#define LIST_HEADER_COLOR			COLOR_BRIGHT_WHITE

#define MENU_FOREGROUND_COLOR		COLOR_CYAN
#define MENU_INACTIVE_COLOR			COLOR_BRIGHT_BLUE
#define MENU_BACKGROUND_COLOR		COLOR_BLACK
#define MENU_ACCENT_COLOR			COLOR_BRIGHT_BLUE

#define DIALOG_FOREGROUND_COLOR		COLOR_BRIGHT_YELLOW
#define DIALOG_BACKGROUND_COLOR		COLOR_BLUE
#define DIALOG_ACCENT_COLOR			COLOR_BRIGHT_BLUE
#define DIALOG_AFFIRM_COLOR			COLOR_GREEN
#define DIALOG_CANCEL_COLOR			COLOR_RED

#define FILE_CONTENTS_FOREGROUND_COLOR	COLOR_BRIGHT_GREEN
#define FILE_CONTENTS_BACKGROUND_COLOR	COLOR_BLACK
#define FILE_CONTENTS_ACCENT_COLOR		COLOR_GREEN


/*****************************************************************************/
/*                                   Command Keys                            */
/*****************************************************************************/


// key codes for user input
#define ACTION_INVALID_INPUT		255	// this will represent illegal keyboard command by user

#define ACTION_CANCEL				CH_ESC
#define ACTION_CANCEL_ALT			CH_RUNSTOP
#define ACTION_CONFIRM				CH_ENTER

// navigation keys
#define MOVE_UP						CH_CURS_UP
#define MOVE_RIGHT					CH_CURS_RIGHT
#define MOVE_DOWN					CH_CURS_DOWN
#define MOVE_LEFT					CH_CURS_LEFT

// file and memory bank actions
#define ACTION_SELECT				CH_ENTER // numpad key
#define ACTION_DELETE				CH_DEL

#define ACTION_SET_BAUD				'b'
#define ACTION_DIAL_FOENIX_BBS		'f'
#define ACTION_DIAL_BBS				'd'
#define ACTION_ABOUT				'a' 
#define ACTION_QUIT					'q'

#define ACTION_HELP					'?' // numpad key



/*****************************************************************************/
/*                                 Error Codes                               */
/*****************************************************************************/

#define ERROR_NO_ERROR													0
#define ERROR_NO_FILES_IN_FILE_LIST										101
#define ERROR_PANEL_WAS_NULL											102
#define ERROR_PANEL_ROOT_FOLDER_WAS_NULL								103
#define ERROR_PANEL_TARGET_FOLDER_WAS_NULL								104
#define ERROR_FOLDER_WAS_NULL											105
#define ERROR_FILE_WAS_NULL												106
#define ERROR_COULD_NOT_OPEN_DIR										107
#define ERROR_COULD_NOT_CREATE_NEW_FILE_OBJECT							108
#define ERROR_FOLDER_FILE_WAS_NULL										109
#define ERROR_NO_CONNECTED_DRIVES_FOUND									110
#define ERROR_FILE_TO_DESTROY_WAS_NULL									111
#define ERROR_DESTROY_ALL_FOLDER_WAS_NULL								112
#define ERROR_FILE_DUPLICATE_FAILED										113
#define ERROR_FOLDER_TO_DESTROY_WAS_NULL								114
#define ERROR_SET_CURR_ROW_FOLDER_WAS_NULL								115
#define ERROR_GET_CURR_ROW_FOLDER_WAS_NULL								116
#define ERROR_SET_FILE_SEL_BY_ROW_PANEL_WAS_NULL						117
#define ERROR_FILE_MARK_SELECTED_FILE_WAS_NULL							118
#define ERROR_FILE_MARK_UNSELECTED_FILE_WAS_NULL						119
#define ERROR_PANEL_INIT_FOLDER_WAS_NULL								120
#define ERROR_COPY_FILE_SOURCE_FOLDER_WAS_NULL							121
#define ERROR_COPY_FILE_TARGET_FOLDER_WAS_NULL							122
#define ERROR_POPULATE_FILES_FOLDER_WAS_NULL							123
#define ERROR_COULD_NOT_CREATE_ROOT_FOLDER_FILE							124
#define ERROR_COULD_NOT_CREATE_ROOT_FOLDER_OBJ							125
#define ERROR_MEMSYS_GET_CURR_ROW_FOLDER_WAS_NULL						126
#define ERROR_BANK_MARK_SELECTED_BANK_WAS_NULL							127
#define ERROR_BANK_MARK_UNSELECTED_BANK_WAS_NULL						128
#define ERROR_BANK_TO_DESTROY_WAS_NULL									129
#define ERROR_PANEL_INIT_MEMSYS_WAS_NULL								130
#define ERROR_DESTROY_ALL_MEMSYS_WAS_NULL								131
#define ERROR_COULD_NOT_CREATE_OR_RESET_MEMSYS_OBJ						132

#define ERROR_DEFINE_ME													255



/*****************************************************************************/
/*                                  Overlays                                 */
/*****************************************************************************/

#define OVERLAY_CPU_BANK					0x05	// overlays are always loaded into bank 5
#define OVERLAY_START_ADDR					0xA000	// in CPU memory space, the start of overlay memory

// overlays defs are just the physical bank num the overlay code is stored in
#define OVERLAY_SCREEN			0x08
#define OVERLAY_STARTUP			0x09
//#define OVERLAY_3				0x0A
//#define OVERLAY_4			0x0B
//#define OVERLAY_5		0x0C
//#define OVERLAY_6					0x0D
//#define OVERLAY_7					0x0E
//#define OVERLAY_8					0x0F
//#define OVERLAY_9					0x10
//#define OVERLAY_10					0x11

#define CUSTOM_FONT_PHYS_ADDR              0x3A000	// temporary buffer for loading in a font?
#define CUSTOM_FONT_SLOT                   0x05
#define CUSTOM_FONT_VALUE                  0x1D

/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/


/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/

// Draws the progress bar frame on the screen
void App_ShowProgressBar(void);

// Hides the progress bar frame on the screen
void App_HideProgressBar(void);

// draws the 'bar' part of the progress bar, according to a % complete passed (0-100 integer)
void App_UpdateProgressBar(uint8_t progress_bar_total);

// // copy 256b chunks of data between specified 6502 addr and the fixed address range in EM, using bank switching -- no DMA
// // em_bank_num is used to derive the base EM address
// // page_num is used to calculate distance from the base EM address
// // set to_em to true to copy from CPU space to EM, or false to copy from EM to specified CPU addr. PARAM_COPY_TO_EM/PARAM_COPY_FROM_EM
// void App_EMDataCopy(uint8_t* cpu_addr, uint8_t em_bank_num, uint8_t page_num, bool to_em);

// read the real time clock and display it
void App_DisplayTime(void);

// display error message, wait for user to confirm, and exit
void App_Exit(uint8_t the_error_number);

// Brings the requested overlay into memory
void App_LoadOverlay(uint8_t the_overlay_em_bank_number);


#endif /* FILE_MANAGER_H_ */
