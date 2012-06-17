#ifndef __CDISO_H
#define __CDISO_H

#ifdef __cplusplus
extern "C" {
#endif

#define SECTORSIZE 2048
#define NUMNULLSECTORS 16	/*Number of zero filled sectors*/

/*Primary Volume Descriptor
** Sector 16 (including zero sector)
************************************/
struct __cdiso_pvd {
	UCHAR one;
	UCHAR VolumeDescriptorSetTerminator[6];
	UCHAR zero;
	UCHAR SystemIdentifier[32];
	UCHAR VolumeIdentifier[32];
	UCHAR zeros1[8];
	long long int NumberOfSectors;			/*both endian double word*/	
	UCHAR zeros2[32];
	UINT VolumeSetSize;			/*always 1*/
	UINT VolumeSequenceNumber; 	/*always 1*/
	UINT SectorSize;				/* always 2048*/
	long long int PathTableLength;			/*in bytes, both endian double word*/
	UINT NumFirstSecInFirstLEPathTable;	/*as a little endian double word*/
	UINT NumFirstSecInSecondLEPathTable;	/*as a little endian double word,
													  or zero if there is no second LE path table*/
	UINT NumFirstSecInFirstBEPathTable;	/*as a big endian double word*/
	UINT NumFirstSecInSecondBEPathTable;	/*as a big endian double word,
													  or zero if there is no second BE path table*/
	UCHAR RootDirectoryRecord[34];
	UCHAR VolumeSetIdentifier[128];
	UCHAR PublisherIdentifier[128];
	UCHAR DataPreparerIdentifier[128];
	UCHAR ApplicationIdentifier[128];
	UCHAR COPYRIGHTFileIdentifier[37]; 		/*$$$*/
	UCHAR AbstractFileIdentifier[37];
	UCHAR BibiliographicalFileIdentifier[37];
	UCHAR DateTimeVolumeCreation[17];
	UCHAR DateTimeMostRecentModification[17];
	UCHAR DateTimeVolumeExpires[17];
	UCHAR DateTimeVOlumeEffective[17];
	UCHAR one2;
	UCHAR zero2;
	UCHAR Reserved[512];
	UCHAR zeros3[653];
} __attribute__ ((packed));

/*Path Tables $$ Crosses Sectors*/
struct __cdiso_pathtable {
	UCHAR NameLength;	/*N or 1 for the root directory*/
	UCHAR zero;
	UINT NumFirstSectorInDir;
	USHORT NumParentDirRecord;	/*or 1 for the root directory (begins at one not zero) LEVEL OF CURDIR=NumParentDirRecord+1*/
	UCHAR Name[31];	/*SIZE: NameLength / or zero for the root directory*/
	UCHAR PaddingByte; /*if Name is odd, this field contains a zero; if
                 		 Name is even, this field is omitted*/
} __attribute__ ((packed));

/* Directories or Files $$ !Crosses Sectors
** . and .. are directories in isofs
*/
struct __cdiso_directory {
	UCHAR NumBytesInRecord;	/*has to be even*/
	UCHAR zero;
	long long int NumFirstSectorFile;/*both endian double word*/
	long long int NumBytesFileData;	/*or length of directory*/
	UCHAR YearsSinceUNIX; /*number of years since 1900*/
	UCHAR Month;		/*where 1=January, 2=February, etc.*/
	UCHAR DayOfMonth;	/*in the range from 1 to 31*/
	UCHAR Hour;			/*in the range from 0 to 23*/
	UCHAR Minute;		/*in the range from 0 to 59*/
	UCHAR Second;		/*in the range from 0 to 59*/
	UCHAR GMT;
	UCHAR Flags;
/*FLAGS:
                 bit     value
                 ------  ------------------------------------------
                 0 (LS)  0 for a normal file, 1 for a hidden file
                 1       0 for a file, 1 for a directory
                 2       0 [1 for an associated file]
                 3       0 [1 for record format specified]
                 4       0 [1 for permissions specified]
                 5       0
                 6       0
                 7 (MS)  0 [1 if not the final record for the file]
*/
	UCHAR zero2;
	UCHAR zero3;
	UINT one;
	UCHAR IdentifierLength;
	UCHAR Identifier[16];
	UCHAR PaddingByte;	/*XXX*/	
	/*Unspecified field for system use omitted*/
} __attribute__ ((packed));





//-------------------------------------------------------------------------------------------------------

struct __cdiso_directory2 {
	UCHAR NumBytesInRecord;	/*has to be even*/
	UCHAR zero;
	long long int NumFirstSectorFile;/*both endian double word*/
	long long int NumBytesFileData;	/*or length of directory*/
	UCHAR YearsSinceUNIX; /*number of years since 1900*/
	UCHAR Month;		/*where 1=January, 2=February, etc.*/
	UCHAR DayOfMonth;	/*in the range from 1 to 31*/
	UCHAR Hour;			/*in the range from 0 to 23*/
	UCHAR Minute;		/*in the range from 0 to 59*/
	UCHAR Second;		/*in the range from 0 to 59*/
	UCHAR GMT;
	UCHAR Flags;
/*FLAGS:
                 bit     value
                 ------  ------------------------------------------
                 0 (LS)  0 for a normal file, 1 for a hidden file
                 1       0 for a file, 1 for a directory
                 2       0 [1 for an associated file]
                 3       0 [1 for record format specified]
                 4       0 [1 for permissions specified]
                 5       0
                 6       0
                 7 (MS)  0 [1 if not the final record for the file]
*/
	UCHAR zero2;
	UCHAR zero3;
	UINT one;
	UCHAR IdentifierLength;
	UCHAR Identifier[38];
} __attribute__ ((packed));

unsigned int cdiso_getfilesize(char *path);
void cdiso_readfile(char *path, unsigned char *buffer);
int cdiso_getdirectory(struct __cdiso_directory2 **dir);
void cdiso_getfile(char *path, struct __cdiso_directory *file);
void cdiso_readsector(struct __cdiso_directory *file, unsigned char *buffer, unsigned int sector);

#ifdef __cplusplus
}
#endif

#endif


