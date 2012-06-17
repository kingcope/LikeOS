/*
$$ ISO9660 Filesystem Driver - unfinished, reads root directory
$$ Copyright (C) 2006 Nikolaos Rangos
*/

#include <defs.h>
#include <atapi.h>
#include <isofs.h>

struct __cdiso_pvd *pvd=(struct __cdiso_pvd*)0;

/*
$$ Read Primary Volume Descriptor
*/
void cdiso_readpvd() {
	pvd=(struct __cdiso_pvd*) kalloc(sizeof(struct __cdiso_pvd));
	atapi_readsectors(16, pvd, sizeof(struct __cdiso_pvd));
}

char *getSystemIdentifier() {
	if (pvd == NULL) cdiso_readpvd();
	pvd->SystemIdentifier[31]=0;
	return pvd->SystemIdentifier;
}

char *getVolumeIdentifier() {
	if (pvd == NULL) cdiso_readpvd();

	pvd->VolumeIdentifier[31]=0;
	return pvd->VolumeIdentifier;
}

/*
$$ Retrieve files in a directory, Be aware, this function is a HACK!!
*/

#define MAXSECTORSPERDIR 20

int cdiso_getdirectory(struct __cdiso_directory2 **dir) {
	struct __cdiso_pathtable rootpath;
	struct __cdiso_directory2 rootdir;
	unsigned char *buf=(unsigned char*)kalloc(2048);
	unsigned char *p;
	unsigned char *np;
	unsigned int filecnt=0;
	unsigned char k, v,found=0,found2=0;
	unsigned int cnt;
	unsigned int sector=0;
	unsigned int nfsid;

	if (pvd == NULL) {
		cdiso_readpvd();
	}

	atapi_readsectors(pvd->NumFirstSecInFirstLEPathTable, &rootpath, sizeof(struct __cdiso_pathtable));

if (rootpath.NameLength == 1) {

	nfsid=rootpath.NumFirstSectorInDir;

	for (k=0;k<MAXSECTORSPERDIR;k++) {
		atapi_readsectors(nfsid+sector, buf, 2048);
		sector++;

		cnt=0;
		p=buf;
		if (*p==0) break;
		while (cnt<2048) {
			memcpy(&rootdir, p, sizeof(struct __cdiso_directory2));
			if (rootdir.NumBytesInRecord == 0) break;

			np=rootdir.Identifier;
			v=0;
			found=0;found2=0;
			while (v<12) {
				if ((*np==';')) {
					*np=0;
					found = 1;
					break;
				}

				if ((*np==0)) {
					found = 1;
					break;
				}

				v++;
				np++;
			}

			if (found == 1) {
				found2=1;
				for (v=0;v<strlen(rootdir.Identifier);v++) {
					if (!isprint(rootdir.Identifier[v])) {
						found2=0;
						break;
					}
				}
			}

			if (strlen(rootdir.Identifier) == 0) {
				found2 = 0;
			}

			if (found2 == 1) {
				memcpy(&dir[filecnt][0], &rootdir, sizeof(struct __cdiso_directory2));
				filecnt++;
			}

			cnt += rootdir.NumBytesInRecord;
			p += rootdir.NumBytesInRecord;
			if (rootdir.NumBytesInRecord == 0) break;
		}
	}
}

endlabel:

	kfree(buf);
	return filecnt;
}

/*
$$ Read One File From Root Directory
*/
void cdiso_getfile(char *path, struct __cdiso_directory *file) {
	struct __cdiso_pathtable rootpath;
	struct __cdiso_directory rootdir;
	unsigned char *buf=(char*)kalloc(5*2048);
	unsigned char *p;
	unsigned char *np;
	unsigned char k, found=0;
	unsigned int cnt;
	unsigned int sector=0;

	if (pvd == NULL) {
		cdiso_readpvd();
	}

	atapi_readsectors(pvd->NumFirstSecInFirstLEPathTable,
					  &rootpath,
					  sizeof(struct __cdiso_pathtable));

if (rootpath.NameLength == 1) {
	while (1) {
		p=buf;
		atapi_readsectors(rootpath.NumFirstSectorInDir+sector, p, 2048);
		sector++;

		p=buf;
		cnt=0;
		while(1) {
			memcpy(&rootdir, buf, sizeof(struct __cdiso_directory));
			np=rootdir.Identifier;

			k=0;
			while(k<255) {
				if (*np==';') {
					*np=0;break;
				}
				k++;
				np++;
			}

			if (strcmp(rootdir.Identifier, path) == 0) {
				found = 1;
				break;
			}

			buf+=rootdir.NumBytesInRecord;
			cnt+=rootdir.NumBytesInRecord;
			if (rootdir.NumBytesInRecord == 0) break;
			if (cnt >= 2048) break;
		}
		if (found==1) break;
	}
}

	memcpy(file, &rootdir, sizeof(struct __cdiso_directory));
	kfree(buf);
}

unsigned int cdiso_getfilesize(char *path) {
	struct __cdiso_directory *file = kalloc(sizeof(struct __cdiso_directory));
	unsigned int length;

	cdiso_getfile(path,	file);
	length=(unsigned int)file->NumBytesFileData;
	kfree(file);
	return length;
}

void cdiso_readfile(char *path, unsigned char *buffer) {
	struct __cdiso_directory *file = kalloc(sizeof(struct __cdiso_directory));
	unsigned int sector;
	unsigned int startsector;
	unsigned int read;
	unsigned char *p;
	unsigned int length;

	cdiso_getfile(path,	file);
	length=(unsigned int)file->NumBytesFileData;
	startsector=(unsigned int)file->NumFirstSectorFile;

	if (length <= 2048) {
		atapi_readsectors(startsector, buffer, length);
	} else {
		p=buffer;
		sector=0;
		read=0;
		do {
			atapi_readsectors(startsector+sector, p, 2048);
			p+=2048;
			read+=2048;
			sector++;
		} while (read+2048 < length);

		sector++;
		atapi_readsectors(startsector+sector, p, length-read);
	}

	kfree(file);
}

void cdiso_readsector(struct __cdiso_directory *file, unsigned char *buffer, unsigned int sector) {
	unsigned int startsector;
	startsector=(unsigned int)file->NumFirstSectorFile+sector;
	atapi_readsectors(startsector, buffer, 2048);
}
