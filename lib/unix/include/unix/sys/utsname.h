#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H



struct utsname
{
	char  *sysname;
	char  *nodename;
	char  *release;
	char  *version;
	char  *machine;
};



#ifdef __cplusplus
extern "C" {
#endif
		

int uname(struct utsname *);


#ifdef __cplusplus
}
#endif
		


#endif

