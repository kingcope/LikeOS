#ifndef _PWD_H
#define _PWD_H


struct passwd
{
	char    *pw_name;   //user's login name
	uid_t    pw_uid;    //numerical user ID
	gid_t    pw_gid;    //numerical group ID
	char    *pw_dir;    //initial working directory
	char    *pw_shell;  //program to use as shell
};


#ifndef _HAVE_UID_T
#define _HAVE_UID_T
typedef	int32_t		uid_t;
#endif

#ifndef _HAVE_GID_T
#define _HAVE_GID_T
typedef	int32_t		gid_t;
#endif



#ifdef __cplusplus
extern "C" {
#endif

struct passwd *getpwnam(const char *);
struct passwd *getpwuid(uid_t);
int            getpwnam_r(const char *, struct passwd *, char *,
				                   size_t, struct passwd **);
int            getpwuid_r(uid_t, struct passwd *, char *,
				                   size_t, struct passwd **);
void           endpwent(void);
struct passwd *getpwent(void);
void           setpwent(void);

#ifdef __cplusplus
}
#endif



#endif

