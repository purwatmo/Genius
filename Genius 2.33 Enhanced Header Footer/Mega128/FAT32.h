/*
    ________                            _____        __________
   /\  _____\ ____     ____     ____   / ___ \      /\____  ___\     code
   \ \ \____// __ \   / __ \   / __ \ /\ \_/\ \   __\/__ /\ \__/______
    \ \ \____\ \Z\ \ /\ \/\ \ /\ \Z\ \\ \ \\ \ \ /\ \/  \\ \ \ /\  __ \
     \ \  __ \\  ___\\ \ \_\ \\ \  ___\\ \ \\_\ \\ \  /\ \\ \ \\ \ \/\ \
      \ \ \__/ \ \__/_\ \____ \\ \ \__/_\ \  ___ \\ \ \ \ \\ \ \\ \ \_\ \
       \ \_\  \ \_____\\/___/\ \\ \_____\\ \_\_/\ \\ \_\ \_\\ \_\\ \_____\
        \/_/   \/_____/_____\_\ \\/_____/ \/_/ \/_/ \/_/\/_/ \/_/ \/_____/
                   /\___________/                        corgito ergo sum.
                   \/__________/
    
	FAT32 Routine [Header]
	Controller	:	ATmega128/162 (8 Mhz internal)
	Compiler	:	AVR-GCC
	Version		:	1.1
	Author		:	Yofe Fegeanto, Jakarta (Indonesia)
					www.fegeanto.com
	Date		:	24 November 2009

	Memory Used=???
*/

#ifndef _FAT32_H_
#define _FAT32_H_

struct __mbr_info{
	unsigned char	nothing[446];
	unsigned char	partitiondata[64];
	unsigned int	signature;
};

struct __partition_info{ 				
	unsigned char	status;
	unsigned char 	headstart;
	unsigned int	cylsecstart;
	unsigned char	type;
	unsigned char	headend;
	unsigned int	cylsectend;
	unsigned long	firstsector;
	unsigned long	sectorstotal;
};

struct __byte_sector{
	unsigned char	jumpboot[3];
	unsigned char	oemname[8];
	unsigned int	bytespersector;
	unsigned char	sectorpercluster;
	unsigned int	reservedsectorcount;
	unsigned char	numberoffats;
	unsigned int	rootentrycount;
	unsigned int	totalsectors_f16;
	unsigned char	mediatype;
	unsigned int	fatsize_f16;
	unsigned int	sectorspertrack;
	unsigned int	numberofheads;
	unsigned long	hiddensectors;
	unsigned long	totalsectors_f32;
	unsigned long	fatsize_f32;
	unsigned int	extflags;
	unsigned int	fsversion;
	unsigned long	rootcluster;
	unsigned int	fsinfo;
	unsigned int	backupbootsector;
	unsigned char	reserved[12];
	unsigned char	drivenumber;
	unsigned char	isreserved;
	unsigned char	bootsignature;
	unsigned long	volumeid;
	unsigned char	volumelabel[11];
	unsigned char	filesystemtype[8];
	unsigned char	bootdata[420];
	unsigned int	bootendsignature;
};

struct __fat_sector{
	unsigned long	leadsignature;
	unsigned char	reserved1[480];
	unsigned long	structuresignature;
	unsigned long	freeclustercount;
	unsigned long	nextfreecluster;
	unsigned char	reserved2[12];
	unsigned long	trailsignature;
};

struct __directory{
	unsigned char	name[11];
	unsigned char	attrib;
	unsigned char	ntreserved;
	unsigned char	timetenth;
	unsigned int	createtime;
	unsigned int	createdate;
	unsigned int	lastaccessdate;
	unsigned int	firstclusterhigh;
	unsigned int	writetime;
	unsigned int	writedate;
	unsigned int	firstclusterlow;
	unsigned long	filesize;
};

#define _ATTR_READ_ONLY	0x01
#define _ATTR_HIDDEN	0x02
#define _ATTR_SYSTEM	0x04
#define _ATTR_VOLUME_ID	0x08
#define _ATTR_DIRECTORY	0x10
#define _ATTR_ARCHIVE	0x20
#define _ATTR_LONG_NAME	0x0f

#define _MMC_DIR_ENTRY	0x32
#define _MMC_EMPTY		0x00
#define _MMC_DELETED	0xe5
#define _MMC_GET		0
#define _MMC_SET		1
#define _MMC_READ		0
#define _MMC_VERIFY		1
#define _MMC_ADD		0
#define _MMC_REMOVE		1
#define _MMC_TOTAL_FREE	1
#define _MMC_NEXT_FREE	2
#define _MMC_EOF		0x0FFFFFFF

volatile unsigned long	__firstdatasector, __rootcluster, __totalclusters;
volatile unsigned int	__bytespersector, __sectorpercluster, __reservedsectorcount;
unsigned long			__unusedsectors, __appendfilesector, __appendfilelocation, __filesize, __appendstartcluster;
unsigned char			__freeclustercountupdated;

struct __directory* _mmc_findfiles(unsigned char*);

void	_mmc_write(unsigned char*, char*);
void	_mmc_freememory(unsigned char, unsigned long);

unsigned char	_mmc_read(unsigned char, unsigned char*);
unsigned char	_mmc_convertfilename(unsigned char*);
unsigned char	_mmc_getbootsector(void);

unsigned long	_mmc_getfirstsector(unsigned long);
unsigned long	_mmc_freecluster(unsigned char, unsigned char, unsigned long);
unsigned long	_mmc_nextcluster(unsigned long,unsigned char, unsigned long);
unsigned long	_mmc_searchnext(unsigned long);

#endif
