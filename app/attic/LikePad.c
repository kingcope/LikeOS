/*
	LikeOS Terminal
	Copyright (C) 2005 Nikolaos Rangos
*/

#include "wnd.h"
#include "txtCtrl.h"

void LikePad_Command_Callback(struct textcontrol_t *txtctrl, char *command) {	
	if (strcmp(command, "uname") == 0) {
		strcat(txtctrl->keyboard_buffer, "Like OS v1.0 - Copyright (C) Nikolaos Rangos\r");
	} else {
		strcat(txtctrl->keyboard_buffer, "Unknown command\r");
	}
}

void Launch_LikePad() {
	struct WND *window1,*window2,*window3;
	
	window1=NewWindow("LikePad Terminal", 300, 200, 640, 480);
	addTextControl(window1, 0, 0, 640, 480, LikePad_Command_Callback);

	window2=NewWindow("LikePad Terminal", 100, 100, 640, 480);
	addTextControl(window2, 0, 0, 640, 480, LikePad_Command_Callback);	
					
	update();
}
