/*$
  $ LikeOS Explorer
  $ Copyright (c) 2006 Microtrends Ltd., Nikolaos Rangos
  $
  $*/

#include "defs.h"
#include "window.h"
#include "isocd.h"

extern guiroot root;
extern shell Shell;

extern "C"
void SHLaunchExplorer() {
	window *w=new window();
	menu *me=new menu();
	combobox *c = new combobox();
	label *l = new label();
	struct __cdiso_directory dir[1024];
	int i,cnt,width=500,height=555;
	
	w->createwindow("LikeOS Explorer", width, height);
	w->invisible=0;
	w->controlscount=0;
	w->handle=1;
	w->z=1;
	
	listbox *li = new listbox();
	li->createlistbox("TEST", 5, 46+23, width-12, height-80);
	li->handle = 1;
	li->windowHandle = 1;
	li->refresh = 1;
	w->controls[w->controlscount]=(control*)li;
	w->controlscount++;	
	
	l->createlabel("Address", 5, 50);
	l->handle = 0;
	l->windowHandle = 1;
	l->refresh = 1;
	w->controls[w->controlscount]=(control*)l;
	w->controlscount++;	
	
	c->createcombobox("CDROM/", 70, 46, width-(32+70), 21);
	c->handle = 2;
	c->windowHandle = 1;
	c->refresh = 1;
	c->additem("PARTITION1/");
	c->additem("PARTITION2/");
	c->additem("PARTITION3/");
	c->additem("REMOVEABLE_DISK1/");
	w->controls[w->controlscount]=(control*)c;
	w->controlscount++;	
			
	me->createmenu("TEST", 5, 22, width, 22);
	me->handle=3;
	me->windowHandle=1;
	me->refresh=1;
	me->addtopmenu();
	me->additem("File",0,0);
	me->additem("Close",0,1);
	me->addtopmenu();
	me->additem("Edit",1,0);
	me->additem("Select",1,1);
	me->addtopmenu();
	me->additem("View",2,0);
	me->additem("Details",2,1);
	w->controls[w->controlscount]=(control*)me;
	w->controlscount++;	
	
	cnt = cdiso_getdirectory(dir);
	for (i=0;i<cnt;i++) {
		li->addItem((char*)dir[i].Identifier);
	}
	
	Shell.windows[Shell.windowscount] = w;
	Shell.windowscount++;
	
	Shell.windowWithFocus=2;
}
