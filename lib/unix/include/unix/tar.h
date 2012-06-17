#ifndef _TAR_H
#define _TAR_H


#ifdef __cplusplus
extern "C" {
#endif


// General definitions:

#define		TMAGIC		"ustar"
#define		TMAGLEN		6
#define		TVERSION	"00"
#define		TVERSLEN	2

// Typeflag field definitions:


#define		REGTYPE 	'0' 	//Regular file.
#define		AREGTYPE 	'\0' 	//Regular file.
#define		LNKTYPE 	'1' 	//Link.
#define		SYMTYPE 	'2' 	//Symbolic link.
#define		CHRTYPE 	'3' 	//Character special.
#define		BLKTYPE 	'4' 	//Block special.
#define		DIRTYPE 	'5' 	//Directory.
#define		FIFOTYPE 	'6' 	//FIFO special.
#define		CONTTYPE 	'7' 	//Reserved.

// Mode field bit definitions (octal):

#define		TSUID 		04000 	// Set UID on execution.
#define		TSGID 		02000 	// Set GID on execution.
#define		TSVTX 		01000 	// On directories, restricted deletion flag.
#define		TUREAD 		00400 	// Read by owner.
#define		TUWRITE		00200 	// Write by owner special.
#define		TUEXEC 		00100 	// Execute/search by owner.
#define		TGREAD 		00040 	// Read by group.
#define		TGWRITE		00020 	// Write by group.
#define		TGEXEC 		00010 	// Execute/search by group.
#define		TOREAD 		00004 	// Read by other.
#define		TOWRITE		00002 	// Write by other.
#define		TOEXEC 		00001 	//Execute/search by other.
		
#ifdef __cplusplus
}
#endif


#endif

