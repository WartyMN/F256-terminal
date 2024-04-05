/*
 * screen.c
 *
 *  Created on: Jan 11, 2023
 *      Author: micahbly
 *
 *  Routines for drawing and updating the UI elements
 *
 */



/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// project includes
#include "screen.h"
#include "app.h"
#include "comm_buffer.h"
#include "general.h"
#include "kernel.h"
#include "keyboard.h"
#include "memory.h"
#include "sys.h"
#include "text.h"
#include "strings.h"

// C includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// F256 includes
#include "f256.h"




/*****************************************************************************/
/*                               Definitions                                 */
/*****************************************************************************/

#define UI_BYTE_SIZE_OF_APP_TITLEBAR	240	// 3 x 80 rows for the title at top


/*****************************************************************************/
/*                           File-scope Variables                            */
/*****************************************************************************/

#pragma data-name ("OVERLAY_SCREEN")

static UI_Button		uibutton[NUM_BUTTONS] =
{
	{BUTTON_ID_SET_BAUD,		UI_MIDDLE_AREA_START_X,		UI_MIDDLE_AREA_DEV_CMD_Y,		ID_STR_APP_SET_BAUD,		UI_BUTTON_STATE_ACTIVE,		UI_BUTTON_STATE_CHANGED,	ACTION_SET_BAUD	}, 
	{BUTTON_ID_DIAL_FOENIX_BBS,	UI_MIDDLE_AREA_START_X,		UI_MIDDLE_AREA_DEV_CMD_Y + 1,	ID_STR_APP_DIAL_FOENIX_BBS,	UI_BUTTON_STATE_ACTIVE,		UI_BUTTON_STATE_CHANGED,	ACTION_DIAL_FOENIX_BBS	}, 
	{BUTTON_ID_DIAL_BBS,		UI_MIDDLE_AREA_START_X,		UI_MIDDLE_AREA_DEV_CMD_Y + 2,	ID_STR_APP_DIAL_BBS,		UI_BUTTON_STATE_ACTIVE,		UI_BUTTON_STATE_CHANGED,	ACTION_DIAL_BBS	}, 
	{BUTTON_ID_ABOUT,			UI_MIDDLE_AREA_START_X,		UI_MIDDLE_AREA_DEV_CMD_Y + 3,	ID_STR_APP_ABOUT,			UI_BUTTON_STATE_ACTIVE,		UI_BUTTON_STATE_CHANGED,	ACTION_ABOUT	}, 
	{BUTTON_ID_QUIT,			UI_MIDDLE_AREA_START_X,		UI_MIDDLE_AREA_DEV_CMD_Y + 4,	ID_STR_APP_QUIT,			UI_BUTTON_STATE_ACTIVE,		UI_BUTTON_STATE_CHANGED,	ACTION_QUIT	}, 
};
 
static uint8_t			screen_titlebar[UI_BYTE_SIZE_OF_APP_TITLEBAR] = 
{
	148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,
7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,141,142,143,144,145,146,147,32,32,32,102,47,116,101,114,109,32,0x46,0x32,0x35,0x36,32,32,140,139,138,137,136,135,134,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,
};


/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/

extern bool					global_clock_is_visible;		// tracks whether or not the clock should be drawn. set to false when not showing main 2-panel screen.

extern char*				global_string[NUM_STRINGS];
extern char*				global_string_buff1;
extern char*				global_string_buff2;

extern TextDialogTemplate	global_dlg;	// dialog we'll configure and re-use for different purposes
extern char					global_dlg_title[36];	// arbitrary
extern char					global_dlg_body_msg[70];	// arbitrary
extern char					global_dlg_button[3][10];	// arbitrary
extern uint8_t				temp_screen_buffer_char[APP_DIALOG_BUFF_SIZE];	// WARNING HBD: don't make dialog box bigger than will fit!
extern uint8_t				temp_screen_buffer_attr[APP_DIALOG_BUFF_SIZE];	// WARNING HBD: don't make dialog box bigger than will fit!

extern uint8_t				zp_bank_num;
extern uint8_t				io_bank_value_kernel;	// stores value for the physical bank pointing to C000-DFFF whenever we change it, so we can restore it.

#pragma zpsym ("zp_bank_num");


/*****************************************************************************/
/*                       Private Function Prototypes                         */
/*****************************************************************************/

void Screen_DrawUI(void);


/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/

// clear screen and draw main UI
void Screen_DrawUI(void)
{
	
	Text_ClearScreen(APP_FOREGROUND_COLOR, APP_BACKGROUND_COLOR);
	
	// draw the title bar at top. 3x80
	Text_CopyMemBoxLinearBuffer((uint8_t*)&screen_titlebar, 0, 0, 79, 2, SCREEN_COPY_TO_SCREEN, SCREEN_FOR_TEXT_CHAR);
	Text_FillBoxAttrOnly(0, 0, 79, 0, APP_ACCENT_COLOR, APP_BACKGROUND_COLOR);
	Text_FillBoxAttrOnly(0, 2, 79, 2, APP_ACCENT_COLOR, APP_BACKGROUND_COLOR);
	Text_InvertBox(48, 1, 54, 1);	// right-hand side vertical bars need to be inversed to grow from thin to fat


	// draw menu
	Text_DrawHLine(UI_MIDDLE_AREA_START_X, UI_MIDDLE_AREA_DEV_MENU_Y, UI_MIDDLE_AREA_WIDTH, CH_UNDERSCORE, MENU_ACCENT_COLOR, APP_BACKGROUND_COLOR, CHAR_AND_ATTR);
	Text_DrawStringAtXY(UI_MIDDLE_AREA_START_X, UI_MIDDLE_AREA_DEV_MENU_Y + 1, General_GetString(ID_STR_MENU_APP), MENU_FOREGROUND_COLOR, APP_BACKGROUND_COLOR);
	Text_DrawHLine(UI_MIDDLE_AREA_START_X, UI_MIDDLE_AREA_DEV_MENU_Y + 2, UI_MIDDLE_AREA_WIDTH, CH_OVERSCORE, MENU_ACCENT_COLOR, APP_BACKGROUND_COLOR, CHAR_AND_ATTR);
		
	// also draw the comms area
	Buffer_DrawCommunicationArea();
	Buffer_RefreshDisplay();
}



/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/




// Get user input and vet it against the menu items that are currently enabled
// returns ACTION_INVALID_INPUT if the key pressed was for a disabled menu item
// returns the key pressed if it matched an enabled menu item, or if wasn't a known (to Screen) input. This lets App still allow for cursor keys, etc, which aren't represented by menu items
uint8_t Screen_GetValidUserInput(void)
{
	uint8_t		user_input;
	uint8_t		i;
	
	user_input = Keyboard_GetChar();

	// check input against active menu items
	for (i = 0; i < NUM_BUTTONS; i++)
	{
		//DEBUG_OUT(("%s %d: btn %i change=%u, active=%u, %s", __func__ , __LINE__, i, uibutton[i].changed_, uibutton[i].active_, General_GetString(uibutton[i].string_id_)));
		
		// check if the key entered matches the key for any menu items
		if (uibutton[i].key_ == user_input)
		{
			// found a match, but is this menu enabled or disabled?
			if (uibutton[i].active_ == true)
			{
				// valid entry. 
				return user_input;
			}
			else
			{
				// invalid entry
				return ACTION_INVALID_INPUT;
			}
		}
	}

	// if still here, it wasn't tied to a menu item. May still be of interest to App, so return it.
	return user_input;
}


// determine which menu items should active, which inactive
// sets inactive/active, and flags any that changed since last evaluation
// does not render
void Screen_UpdateMenuStates(void)
{
	
	return;
}


// renders the menu items, as either active or inactive, as appropriate. 
// active/inactive and changed/not changed must previously have been set
// if sparse_render is true, only those items that have a different enable decision since last render will be re-rendered. Set sparse_render to false if drawing menu for first time or after clearing screen, etc. 
void Screen_RenderMenu(bool sparse_render)
{
	uint8_t		i;
	uint8_t		x1;
	uint8_t		y1;
	uint8_t		text_color;

	// draw buttons
	for (i = 0; i < NUM_BUTTONS; i++)
	{
		//DEBUG_OUT(("%s %d: btn %i change=%u, active=%u, %s", __func__ , __LINE__, i, uibutton[i].changed_, uibutton[i].active_, General_GetString(uibutton[i].string_id_)));
		
		if (uibutton[i].changed_ == true || sparse_render == false)
		{
			text_color = (uibutton[i].active_ == true ? MENU_FOREGROUND_COLOR : MENU_INACTIVE_COLOR);
			x1 = uibutton[i].x1_;
			y1 = uibutton[i].y1_;
			Text_DrawHLine(UI_MIDDLE_AREA_START_X, y1, UI_MIDDLE_AREA_WIDTH, CH_SPACE, text_color, MENU_BACKGROUND_COLOR, CHAR_AND_ATTR);
			Text_DrawStringAtXY(x1, y1, General_GetString(uibutton[i].string_id_), text_color, MENU_BACKGROUND_COLOR);
			uibutton[i].changed_ = false;
		}
	}
}


// set up screen variables and draw screen for first time
void Screen_Render(void)
{
	global_clock_is_visible = true;
	Text_ClearScreen(APP_FOREGROUND_COLOR, APP_BACKGROUND_COLOR);
	Screen_DrawUI();
}


// display information about f/manager
void Screen_ShowAppAboutInfo(void)
{
	// show app name, version, and credit
	sprintf(global_string_buff1, General_GetString(ID_STR_ABOUT_FTERM), CH_COPYRIGHT, MAJOR_VERSION, MINOR_VERSION, UPDATE_VERSION);
	Buffer_NewMessage(global_string_buff1);
	
	// also show current bytes free
	sprintf(global_string_buff1, General_GetString(ID_STR_N_BYTES_FREE), _heapmemavail());
	Buffer_NewMessage(global_string_buff1);
}


// show user a dialog and have them enter a string
// if a prefilled string is not needed, set starter_string to an empty string
// set max_len to the maximum number of bytes/characters that should be collected from user
// returns NULL if user cancels out of dialog, or returns a path to the string the user provided
char* Screen_GetStringFromUser(char* dialog_title, char* dialog_body, char* starter_string, uint8_t max_len)
{
	bool				success;
	uint8_t				orig_dialog_width;
	uint8_t				temp_dialog_width;
	
	// copy title and body text
	General_Strlcpy((char*)&global_dlg_title, dialog_title, 36);
	General_Strlcpy((char*)&global_dlg_body_msg, dialog_body, 70);

	// copy the starter string into the edit buffer so user can edit
	General_Strlcpy(global_string_buff2, starter_string, max_len + 1);
	
	// adjust dialog width temporarily, if necessary and possible
	orig_dialog_width = global_dlg.width_;
	temp_dialog_width = General_Strnlen(starter_string,  max_len) + 2; // +2 is for box draw chars
	
	DEBUG_OUT(("%s %d: orig_dialog_width=%u, temp width=%u, max_len=%u, starter='%s'", __func__ , __LINE__, orig_dialog_width, temp_dialog_width, max_len, starter_string));
	
	if (temp_dialog_width < orig_dialog_width)
	{
		temp_dialog_width = orig_dialog_width - 2;
	}
	else
	{
		global_dlg.width_ = temp_dialog_width;
		temp_dialog_width -= 2;
	}
	
	success = Text_DisplayTextEntryDialog(&global_dlg, (char*)&temp_screen_buffer_char, (char*)&temp_screen_buffer_attr, global_string_buff2, temp_dialog_width);

	// restore normal dialog width
	global_dlg.width_ = orig_dialog_width;

	// did user enter a name?
	if (success == false)
	{
		return NULL;
	}
	
	return global_string_buff2;
}


// show user a 2 button confirmation dialog and have them click a button
// returns true if user selected the "positive" button, or false if they selected the "negative" button
bool Screen_ShowUserTwoButtonDialog(char* dialog_title, uint8_t dialog_body_string_id, uint8_t positive_btn_label_string_id, uint8_t negative_btn_label_string_id)
{
	// copy title, body text, and buttons
	General_Strlcpy((char*)&global_dlg_title, dialog_title, 36);
	General_Strlcpy((char*)&global_dlg_body_msg, General_GetString(dialog_body_string_id), 70);
	General_Strlcpy((char*)&global_dlg_button[0], General_GetString(negative_btn_label_string_id), 10);
	General_Strlcpy((char*)&global_dlg_button[1], General_GetString(positive_btn_label_string_id), 10);
					
	global_dlg.num_buttons_ = 2;

	return Text_DisplayDialog(&global_dlg, (char*)&temp_screen_buffer_char, (char*)&temp_screen_buffer_attr);
}

