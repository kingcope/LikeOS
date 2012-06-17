#include <defs.h>
#include <paging.h>
#include <physalloc.h>
#include <stdio.h>
#include <svgagui.h>
#include <task.h>
#include <isofs.h>

void terminal_dispatch(GuiObject * obj, char *command)
{
	
	if (strcmp(command, "uname") == 0) {
		strcat(obj->buffer, "LikeOS 2007 ALPHA\n");
		set_browser_text(obj, obj->buffer);
		update_browser(obj);
	}
}
