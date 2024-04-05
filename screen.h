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
#include <stdint.h>


/*****************************************************************************/
/*                            Macro Definitions                              */
/*****************************************************************************/

#define PARAM_ONLY_RENDER_CHANGED_ITEMS		true	// parameter for Screen_RenderMenu
#define PARAM_RENDER_ALL_MENU_ITEMS			false	// parameter for Screen_RenderMenu

// there are 12 buttons which can be accessed with the same code
#define NUM_BUTTONS					5

// DEVICE actions
#define BUTTON_ID_SET_BAUD			0
#define BUTTON_ID_DIAL_FOENIX_BBS	(BUTTON_ID_SET_BAUD + 1)
#define BUTTON_ID_DIAL_BBS			(BUTTON_ID_DIAL_FOENIX_BBS + 1)
#define BUTTON_ID_ABOUT				(BUTTON_ID_DIAL_BBS + 1)
#define BUTTON_ID_QUIT				(BUTTON_ID_ABOUT + 1)

#define UI_BUTTON_STATE_INACTIVE	false
#define UI_BUTTON_STATE_ACTIVE		true

#define UI_BUTTON_STATE_UNCHANGED	false
#define UI_BUTTON_STATE_CHANGED		true

#define UI_MIDDLE_AREA_START_X			35
#define UI_MIDDLE_AREA_START_Y			14
#define UI_MIDDLE_AREA_WIDTH			10

#define UI_MIDDLE_AREA_DEV_MENU_Y		(UI_MIDDLE_AREA_START_Y + 2)
#define UI_MIDDLE_AREA_DEV_CMD_Y		(UI_MIDDLE_AREA_DEV_MENU_Y + 3)

#define UI_PANEL_INNER_WIDTH			80
#define UI_PANEL_OUTER_WIDTH			80
#define UI_PANEL_INNER_HEIGHT			40
#define UI_PANEL_OUTER_HEIGHT			(UI_PANEL_INNER_HEIGHT + 2)
#define UI_PANEL_TAB_WIDTH				50
#define UI_PANEL_TAB_HEIGHT				3

#define UI_VIEW_PANEL_TITLE_TAB_Y1		3
#define UI_VIEW_PANEL_TITLE_TAB_HEIGHT	UI_PANEL_TAB_HEIGHT
#define UI_VIEW_PANEL_TITLE_TAB_Y2		(UI_VIEW_PANEL_TITLE_TAB_Y1 + UI_VIEW_PANEL_TITLE_TAB_HEIGHT - 1)
#define UI_VIEW_PANEL_HEADER_Y			(UI_VIEW_PANEL_TITLE_TAB_Y2 + 2)
#define UI_VIEW_PANEL_BODY_Y1			6
#define UI_VIEW_PANEL_BODY_HEIGHT		UI_PANEL_OUTER_HEIGHT
#define UI_VIEW_PANEL_BODY_Y2			(UI_VIEW_PANEL_BODY_Y1 + UI_VIEW_PANEL_BODY_HEIGHT - 1)

#define UI_VIEW_PANEL_BODY_WIDTH		UI_PANEL_OUTER_WIDTH
#define UI_LEFT_PANEL_TITLE_TAB_X1		0
#define UI_LEFT_PANEL_TITLE_TAB_WIDTH	UI_PANEL_TAB_WIDTH
#define UI_LEFT_PANEL_TITLE_TAB_X2		(UI_LEFT_PANEL_TITLE_TAB_X1 + UI_LEFT_PANEL_TITLE_TAB_WIDTH - 1)
#define UI_LEFT_PANEL_BODY_X1			0
#define UI_LEFT_PANEL_BODY_X2			(UI_LEFT_PANEL_BODY_X1 + UI_VIEW_PANEL_BODY_WIDTH - 1)

#define UI_FULL_PATH_LINE_Y				(UI_VIEW_PANEL_BODY_Y2 + 1)	// row, not in any boxes, under file panels, above comms panel, for showing full path of a file.

#define UI_COPY_PROGRESS_Y				(UI_MIDDLE_AREA_FILE_CMD_Y)
#define UI_COPY_PROGRESS_LEFTMOST		(UI_MIDDLE_AREA_START_X + 3)
#define UI_COPY_PROGRESS_RIGHTMOST		(UI_COPY_PROGRESS_LEFTMOST + 5)

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

// set up screen variables and draw screen for first time
void Screen_Render(void);

// Get user input and vet it against the menu items that are currently enabled
// returns 0 if the key pressed was for a disabled menu item
// returns the key pressed if it matched an enabled menu item, or if wasn't a known (to Screen) input. This lets App still allow for cursor keys, etc, which aren't represented by menu items
uint8_t Screen_GetValidUserInput(void);

// determine which menu items should active, which inactive
// sets inactive/active, and flags any that changed since last evaluation
// does not render
void Screen_UpdateMenuStates(void);

// renders the menu items, as either active or inactive, as appropriate. 
// active/inactive and changed/not changed must previously have been set
// if sparse_render is true, only those items that have a different enable decision since last render will be re-rendered. Set sparse_render to false if drawing menu for first time or after clearing screen, etc. 
void Screen_RenderMenu(bool sparse_render);

// display information about f/manager
void Screen_ShowAppAboutInfo(void);

// draw just the 3 column headers in the specified panel
// if for_disk is true, will use name/type/size. if false, will use name/bank num/addr
void Screen_DrawPanelHeader(uint8_t panel_id, bool for_disk);

// show user a dialog and have them enter a string
// if a prefilled string is not needed, set starter_string to an empty string
// set max_len to the maximum number of bytes/characters that should be collected from user
// returns NULL if user cancels out of dialog, or returns a path to the string the user provided
char* Screen_GetStringFromUser(char* dialog_title, char* dialog_body, char* starter_string, uint8_t max_len);

// show user a 2 button confirmation dialog and have them click a button
// returns true if user selected the "positive" button, or false if they selected the "negative" button
bool Screen_ShowUserTwoButtonDialog(char* dialog_title, uint8_t dialog_body_string_id, uint8_t positive_btn_label_string_id, uint8_t negative_btn_label_string_id);


#endif /* SCREEN_H_ */
