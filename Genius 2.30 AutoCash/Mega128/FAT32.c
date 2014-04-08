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
    
	FAT32 Routine
	Controller	:	ATmega128/162 (8 Mhz internal)
	Compiler	:	AVR-GCC
	Version		:	1.1
	Author		:	Yofe Fegeanto, Jakarta (Indonesia)
					www.fegeanto.com
	Date		:	24 November 2009
*/

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "FAT32.h"
#include "MMC.h"
#include "UART.h"

unsigned char _mmc_getbootsector(void){
	struct __byte_sector*		__bpb;
	struct __mbr_info*			__mbr;
	struct __partition_info*	__partition;
	unsigned long				__datasector;

	__unusedsectors = 0;

	_mmc_readsingleblock(0);
	__bpb = (struct __byte_sector*) __mmc_buffer;

	if(__bpb->jumpboot[0] != 0xE9 && __bpb->jumpboot[0] != 0xEB){
		__mbr = (struct __mbr_info*) __mmc_buffer;

		if(__mbr->signature != 0xAA55)
			return 1;

		__partition = (struct __partition_info*)(__mbr->partitiondata);
		__unusedsectors = __partition->firstsector;

		_mmc_readsingleblock(__partition->firstsector);
		__bpb = (struct __byte_sector*) __mmc_buffer;
		if(__bpb->jumpboot[0] != 0xE9 && __bpb->jumpboot[0] != 0xEB)
			return 1; 
	}

	__bytespersector = __bpb->bytespersector;
	__sectorpercluster = __bpb->sectorpercluster;
	__reservedsectorcount = __bpb->reservedsectorcount;
	__rootcluster = __bpb->rootcluster;
	__firstdatasector = __bpb->hiddensectors + __reservedsectorcount + (__bpb->numberoffats * __bpb->fatsize_f32);

	__datasector = __bpb->totalsectors_f32
				- __bpb->reservedsectorcount
				- (__bpb->numberoffats * __bpb->fatsize_f32);
	__totalclusters = __datasector / __sectorpercluster;

	if((_mmc_freecluster(_MMC_TOTAL_FREE, _MMC_GET, 0)) > __totalclusters)
		__freeclustercountupdated = 0;
	else
		__freeclustercountupdated = 1;
	return 0;
}

unsigned long _mmc_getfirstsector(unsigned long __cluster){
	return (((__cluster - 2) * __sectorpercluster) + __firstdatasector);
}

unsigned long _mmc_nextcluster(unsigned long __cluster,
								unsigned char __mode,
                                unsigned long __entry){
	unsigned int	__offset;
	unsigned long*	__value;
	unsigned long	__sector;
	unsigned char	__retry = 0;

	__sector = __unusedsectors + __reservedsectorcount + ((__cluster * 4) / __bytespersector);

	__offset = (unsigned int)((__cluster * 4) % __bytespersector);

	while(__retry < 10){
		if(!_mmc_readsingleblock(__sector))
			break;
		__retry++;
	}

	__value = (unsigned long*) &__mmc_buffer[__offset];

	if(__mode == _MMC_GET)
		return ((*__value) & 0x0FFFFFFF);

	*__value = __entry;

	_mmc_writesingleblock(__sector);

	return (0);
}

unsigned long _mmc_freecluster(unsigned char __choice, unsigned char __mode, unsigned long __entry){
	struct __fat_sector*	__info = (struct __fat_sector*) &__mmc_buffer;
	unsigned char			__error;

	_mmc_readsingleblock(__unusedsectors + 1);

	if(	(__info->leadsignature != 0x41615252) ||
		(__info->structuresignature != 0x61417272) ||
		(__info->trailsignature !=0xAA550000))
		return 0xFFFFFFFF;

	if(__mode == _MMC_GET){
		if(__choice == _MMC_TOTAL_FREE)
			return (__info->freeclustercount);
		else
			return (__info->nextfreecluster);
	}
	else{
		if(__choice == _MMC_TOTAL_FREE)
			__info->freeclustercount = __entry;
		else
			__info->nextfreecluster = __entry;
 
		__error = _mmc_writesingleblock(__unusedsectors + 1);
	}
	return 0xFFFFFFFF;
}

struct __directory* _mmc_findfiles(unsigned char* __filename){
	unsigned long		__cluster, __sector, __first;
	struct __directory*	__dir;
	unsigned int		__i;
	unsigned char		__j;

	__cluster = __rootcluster;

	while(1){
		__first = _mmc_getfirstsector (__cluster);

		for(__sector = 0; __sector < __sectorpercluster; __sector++){
			_mmc_readsingleblock (__first + __sector);

			for(__i = 0; __i < __bytespersector; __i += 32){
				__dir = (struct __directory*) &__mmc_buffer[__i];

				if(__dir->name[0] == _MMC_EMPTY)
					return 0;   
				if((__dir->name[0] != _MMC_DELETED) && (__dir->attrib != _ATTR_LONG_NAME)){
					for(__j = 0; __j < 11; __j++)
						if(__dir->name[__j] != __filename[__j])
							break;
					if(__j == 11){
						__appendfilesector = __first + __sector;
						__appendfilelocation = __i;
						__appendstartcluster = (((unsigned long) __dir->firstclusterhigh) << 16) | __dir->firstclusterlow;
						__filesize = __dir->filesize;
						return (__dir);
					}
				}
			}
		}

		__cluster = (_mmc_nextcluster(__cluster, _MMC_GET, 0));

		if(__cluster > 0x0FFFFFF6)
			return 0;
		if(__cluster == 0)
			return 0;
	}
	return 0;
}

unsigned char _mmc_read(unsigned char __flag, unsigned char* __filename){
	struct __directory*	__dir;
	unsigned long		__cluster, __byte = 0, __filesize, __first;
	unsigned int		__k;
	unsigned char		__j, __error;

	__error = _mmc_convertfilename(__filename);
	if(__error)
		return 2;

	__dir = _mmc_findfiles(__filename);
	if(__dir == 0) 
		return (0);

	if(__flag == _MMC_VERIFY)
		return (1);

	__cluster = (((unsigned long) __dir->firstclusterhigh) << 16) | __dir->firstclusterlow;

	__filesize = __dir->filesize;

	_uart_print(_COM_HOST, 1, "");
	_uart_print(_COM_HOST, 1, "");

	while(1){
		__first = _mmc_getfirstsector(__cluster);

		for(__j = 0; __j < __sectorpercluster; __j++){
			_mmc_readsingleblock(__first + __j);

			for(__k = 0; __k < 512; __k++){
				_uart(_COM_HOST, 1, __mmc_buffer[__k]);
				if((__byte++) >= __filesize)
					return 0;
			}
		}
		__cluster = _mmc_nextcluster(__cluster, _MMC_GET, 0);
		if(__cluster == 0){
			_uart_print(_COM_HOST, 0, "Error in getting __cluster");
				return 0;
		}
	}
	return 0;
}

unsigned char _mmc_convertfilename(unsigned char* __filename){
	unsigned char	__file[11];
	unsigned char	__j, __k;

	for(__j = 0; __j < 12; __j++)
		if(__filename[__j] == '.')
			break;

	if(__j > 8)
		return 1;

	for(__k = 0; __k < __j; __k++)
		__file[__k] = __filename[__k];

	for(__k = __j; __k <= 7; __k++)
		__file[__k] = ' ';

	__j++;
	for(__k = 8; __k < 11; __k++){
		if(__filename[__j] != 0)
			__file[__k] = __filename[__j++];
		else
			while(__k < 11)
				__file[__k++] = ' ';
	}

	for(__j = 0; __j < 11; __j++)
		if((__file[__j] >= 0x61) && (__file[__j] <= 0x7A))
			__file[__j] -= 0x20;

	for(__j = 0; __j < 11; __j++)
		__filename[__j] = __file[__j];

	return 0;
}

void _mmc_write(unsigned char* __filename, char* __text){
	unsigned char		__j, __data, __error, __filecreated = 0, __start = 0, __append = 0, __sector, __ii;
	unsigned int		__i, __high, __low;
	struct __directory*	__dir;
	unsigned long		__cluster, __next, __previous, __first, __count, __memory;

	__j = _mmc_read(_MMC_VERIFY, __filename);

	if(__j == 1){
		__append = 1;
		__cluster = __appendstartcluster;
		__count=0;
		while(1){
			__next = _mmc_nextcluster(__cluster, _MMC_GET, 0);
			if(__next == _MMC_EOF)
				break;
			__cluster = __next;
			__count++;
		}

		__sector = (__filesize - (__count * __sectorpercluster * __bytespersector)) / __bytespersector;
		__start = 1;
	}
	else if(__j == 2) 
		return;
	else{
		__cluster = _mmc_freecluster(_MMC_NEXT_FREE, _MMC_GET, 0);
		if(__cluster > __totalclusters)
			__cluster = __rootcluster;

		__cluster = _mmc_searchnext(__cluster);
		if(__cluster == 0)
			return;

		_mmc_nextcluster(__cluster, _MMC_SET, _MMC_EOF);

		__high = (unsigned int)((__cluster & 0xFFFF0000) >> 16 );
		__low = (unsigned int)( __cluster & 0x0000FFFF);
		__filesize = 0;
	}

	while(1){
		if(__start){
			__start = 0;
			__startblock = _mmc_getfirstsector(__cluster) + __sector;
			_mmc_readsingleblock(__startblock);
			__i = __filesize % __bytespersector;
			__j = __sector;
		}
		else{
			__startblock = _mmc_getfirstsector(__cluster);
			__i=0;
			__j=0;
		}

		__ii = 0;
		do{
			__data = __text[__ii++];
			__mmc_buffer[__i++] = __data;
			__filesize++;
			if(__data == '\r'){
				__mmc_buffer[__i++] = '\n';
				__filesize++;
			}

			if(__i == 512){
				__i=0;
				__error = _mmc_writesingleblock(__startblock);
				__j++;
				if(__j == __sectorpercluster){
					__j = 0;
					break;
				}
				__startblock++; 
			}
		}while(__data != '~');

		if(__data == '~'){
			__filesize--;
			__i--;
			for(; __i < 512; __i++)
				__mmc_buffer[__i]= 0x00;
			__error = _mmc_writesingleblock(__startblock);

			break;
		} 

		__previous = __cluster;

		__cluster = _mmc_searchnext(__previous);

		if(__cluster == 0)
			return;

		_mmc_nextcluster(__previous, _MMC_SET, __cluster);
		_mmc_nextcluster(__cluster, _MMC_SET, _MMC_EOF);
	}        

	_mmc_freecluster(_MMC_NEXT_FREE, _MMC_SET, __cluster);

	if(__append){
		_mmc_readsingleblock(__appendfilesector);    
		__dir = (struct __directory *) &__mmc_buffer[__appendfilelocation]; 
		__memory = __filesize - __dir->filesize;
		__dir->filesize = __filesize;
		_mmc_writesingleblock(__appendfilesector);
		_mmc_freememory(_MMC_REMOVE, __memory);
  
		return;
	}

	__previous = __rootcluster;

	while(1){
		__first = _mmc_getfirstsector(__previous);

		for(__sector = 0; __sector < __sectorpercluster; __sector++){
			_mmc_readsingleblock(__first + __sector);

			for(__i = 0; __i < __bytespersector; __i += 32){
				__dir = (struct __directory *) &__mmc_buffer[__i];

				if(__filecreated){
					__dir->name[0] = 0x00;
					return;
				}

				if((__dir->name[0] == _MMC_EMPTY) || (__dir->name[0] == _MMC_DELETED)){
					for(__j = 0; __j < 11; __j++)
						__dir->name[__j] = __filename[__j];
					__dir->attrib = _ATTR_ARCHIVE;
					__dir->ntreserved = 0;
					__dir->timetenth = 0;
					__dir->createtime = 0x9684;
					__dir->createdate = 0x3a37;
					__dir->lastaccessdate = 0x3a37;
					__dir->writetime = 0x9684;
					__dir->writedate = 0x3a37;
					__dir->firstclusterhigh = __high;
					__dir->firstclusterlow = __low;
					__dir->filesize = __filesize;

					_mmc_writesingleblock(__first + __sector);
					__filecreated = 1;

					_mmc_freememory(_MMC_REMOVE, __filesize);
				}
			}
		}

		__cluster = _mmc_nextcluster(__previous, _MMC_GET, 0);

		if(__cluster > 0x0FFFFFF6){
			if(__cluster == _MMC_EOF){
				__cluster = _mmc_searchnext(__previous);
				_mmc_nextcluster(__previous, _MMC_SET, __cluster);
				_mmc_nextcluster(__cluster, _MMC_SET, _MMC_EOF);
			} 
			else
				return;
		}
		if(__cluster == 0)
			return;

		__previous = __cluster;
	}
	return;
}

unsigned long _mmc_searchnext(unsigned long __start){
	unsigned long	__cluster, *__value, __sector;
	unsigned char	__i;

	__start -=  (__start % 128);
	for(__cluster = __start; __cluster < __totalclusters; __cluster += 128){
		__sector = __unusedsectors + __reservedsectorcount + ((__cluster * 4) / __bytespersector);
		_mmc_readsingleblock(__sector);
		for(__i = 0; __i < 128; __i++){
			__value = (unsigned long *) &__mmc_buffer[__i*4];
			if(((*__value) & 0x0fffffff) == 0)
				return (__cluster+__i);
		}  
	} 

	return 0;
}

void _mmc_freememory(unsigned char __flag, unsigned long __size){
	unsigned long	__free;

	if((__size % 512) == 0)
		__size = __size / 512;
	else
		__size = (__size / 512) +1;
	if((__size % 8) == 0)
		__size = __size / 8;
	else
		__size = (__size / 8) +1;

	if(__freeclustercountupdated){
		__free = _mmc_freecluster(_MMC_TOTAL_FREE, _MMC_GET, 0);
		if(__flag == _MMC_ADD)
			__free = __free + __size;
		else
			__free = __free - __size;
		_mmc_freecluster(_MMC_TOTAL_FREE, _MMC_SET, __free);
	}
}