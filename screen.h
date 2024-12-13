/*
 * screen.h
 *
 *  Created on: Jan 11, 2023
 *      Author: micahbly
 */

#ifndef SCREEN_H_
#define SCREEN_H_

/* about this class
 *
 *** things a screen needs to be able to do
 *
 * draw the file manager screen
 * draw individual buttons
 * update visual state of individual buttons
 *
 *** things a screen has
 *
 *
 */

/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

//#include <string.h>

#include "app.h"
#include "text.h"


#include <stdbool.h>
#include <stdint.h>


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define TERM_BODY_Y1					0
#define TERM_BODY_HEIGHT				25	// UI_PANEL_OUTER_HEIGHT
#define TERM_BODY_Y2					(TERM_BODY_Y1 + TERM_BODY_HEIGHT - 1)

#define TERM_BODY_WIDTH					80
#define TERM_BODY_X1					0
#define TERM_BODY_X2					(TERM_BODY_X1 + TERM_BODY_WIDTH - 1)

#define TITLE_BAR_Y						(TERM_BODY_Y2 + 1)

#define TERM_PROGRESS_BAR_START_X		20
#define TERM_PROGRESS_BAR_START_Y		TITLE_BAR_Y
#define TERM_PROGRESS_BAR_WIDTH			10
#define TERM_BAUD_X1					48
#define TERM_DATE_X1					62
#define TERM_ERROR_OE_X					38		// single-char spot for overrun error flag
#define TERM_ERROR_PE_X					(TERM_ERROR_OE_X + 1)		// single-char spot for parity error flag
#define TERM_ERROR_FE_X					(TERM_ERROR_PE_X + 1)		// single-char spot for framing error flag
#define TERM_ERROR_BI_X					(TERM_ERROR_FE_X + 1)		// single-char spot for break interrupt error flag
#define TERM_ERROR_ERR_X				(TERM_ERROR_BI_X + 1)		// single-char spot for overall error flag

#define CH_PROGRESS_BAR_SOLID_CH1		134		// for drawing progress bars that use solid bars, this is the first char (least filled in)
#define CH_PROGRESS_BAR_CHECKER_CH1		207		// for drawing progress bars that use checkerboard bars, this is the first char (least filled in)
#define CH_INVERSE_SPACE				7		// useful for progress bars as a slot fully used up
#define CH_CHECKERBOARD					199		// useful for progress bars as a slot fully used up
#define CH_UNDERSCORE					148		// this is one line up from a pure underscore, but works if text right under it. 0x5f	// '_'
#define CH_OVERSCORE					0x0e	// opposite of '_'


/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/

/*****************************************************************************/
/*                                 Structs                                   */
/*****************************************************************************/

typedef struct BBS_Panel
{
	uint8_t		x1_;
	uint8_t		y1_;
	uint8_t		x2_;
	uint8_t		y2_;
	uint8_t		width_;
} BBS_Panel;

typedef struct UI_Button
{
	uint8_t		id_;
	uint8_t		x1_;
	uint8_t		y1_;
	uint8_t		string_id_;
	bool		active_;	// 0-disabled/inactive, 1-enabled/active
	bool		changed_;	// set to true when the active/inactive state has changed compared to previous render. set to false after rendering
	uint8_t		key_;		// the keyboard code (foenix ascii) for the key that activates the menu
} UI_Button;


/*****************************************************************************/
/*                       Public Function Prototypes                          */
/*****************************************************************************/

// display information about f/manager
void Screen_ShowAppAboutInfo(void);

// show user a dialog and have them enter a string
// if a prefilled string is not needed, set starter_string to an empty string
// set max_len to the maximum number of bytes/characters that should be collected from user
// returns NULL if user cancels out of dialog, or returns a path to the string the user provided
char* Screen_GetStringFromUser(char* dialog_title, char* dialog_body, char* starter_string, uint8_t max_len);

// show user a 2 button confirmation dialog and have them click a button
// returns true if user selected the "positive" button, or false if they selected the "negative" button
bool Screen_ShowUserTwoButtonDialog(char* dialog_title, uint8_t dialog_body_string_id, uint8_t positive_btn_label_string_id, uint8_t negative_btn_label_string_id);


#endif /* SCREEN_H_ */
