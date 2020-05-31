#include "user_settings.h"





UserSettingsStruct2 UserSettings;



void attach_display_settings()
{
	display_system_brightness = &UserSettings.ScreenBrightness;
	ScreenOrient = UserSettings.ScreenOrient;
	hide_drawing_layers = &UserSettings.HideDrawingLayers;
}




void UserSettingsInit()
{
	attach_display_settings();
	
	UserSettings.ScreenBrightness = 100;
	UserSettings.ScreenOrient = 1;
	UserSettings.HideDrawingLayers = 1;
}