#ifndef __TXTCTRL_H
#define __TXTCTRL_H

struct textcontrol_t {
	unsigned char *bitmap;
	unsigned char *keyboard_buffer;
	void *command_callback;
	unsigned int scroll;
};

#endif
