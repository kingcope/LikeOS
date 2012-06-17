/*$
  $ LikeOS Builder
  $ Copyright (c) 2006 Microtrends Ltd., Nikolaos Rangos
  $
  $*/

#include "defs.h"
#include "window.h"
#include "isocd.h"

extern guiroot root;
extern shell Shell;

extern "C"
void SHLaunchBuilder() {
	window *w=new window();
	window *w2=new window();
	int width=400;
	int height=600;
	int width2=640;
	int height2=480;
	container *cc2 = new container();
	container *cc3 = new container();
	menu *m=new menu();
	
	w->createwindow("LikeOS Builder", width, height);
	w->invisible=0;
	w->controlscount=0;
	w->handle=2;
	w->z=2;

	w2->createwindow("LikeOS Builder - Form1", width2, height2);
	w2->invisible=0;
	w2->controlscount=0;
	w2->handle=3;
	w2->z=3;
		
	cc2->createcontainer("Widgets", 15, 60, width-30, height-85);
	cc2->handle = 0;
	cc2->windowHandle = 2;
	cc2->refresh = 1;
	w->controls[w->controlscount]=(control*)cc2;
	w->controlscount++;
		
	m->createmenu("TEST", 5, 22, width, 22);
	m->handle=2;
	m->windowHandle=2;
	m->refresh=1;
	m->addtopmenu();
	m->additem("File",0,0);
	m->additem("Close",0,1);
	m->addtopmenu();
	m->additem("Edit",1,0);
	m->additem("Select",1,1);
	m->addtopmenu();
	m->additem("View",2,0);
	m->additem("Details",2,1);
	w->controls[w->controlscount]=(control*)m;
	w->controlscount++;		
	
	Shell.windows[Shell.windowscount] = w;
	Shell.windowscount++;
	
	Shell.windows[Shell.windowscount] = w2;
	Shell.windowscount++;
	
	Shell.windowWithFocus=4;
}
