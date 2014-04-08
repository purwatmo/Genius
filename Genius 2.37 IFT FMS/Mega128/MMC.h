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
    
	MMC/SD CARD Routine - Header
	Controller	:	ATmega162/128 (8 Mhz internal)
	Compiler	:	AVR-GCC
	Version		:	1.3
	Author		:	Yofe Fegeanto, Jakarta (Indonesia)
					www.fegeanto.com
	Date		:	24 November 2009
*/

#ifndef MMC_H_
#define MMC_H_

#define _COM_HOST	1

#define MMC_GO_IDLE_STATE            0
#define MMC_SEND_OP_COND             1
#define MMC_SEND_CSD                 9
#define MMC_STOP_TRANSMISSION        12
#define MMC_SEND_STATUS              13
#define MMC_SET_BLOCK_LEN            16
#define MMC_READ_SINGLE_BLOCK        17
#define MMC_READ_MULTIPLE_BLOCKS     18
#define MMC_WRITE_SINGLE_BLOCK       24
#define MMC_WRITE_MULTIPLE_BLOCKS    25
#define MMC_ERASE_BLOCK_START_ADDR   32
#define MMC_ERASE_BLOCK_END_ADDR     33
#define MMC_ERASE_SELECTED_BLOCKS    38
#define MMC_CRC_ON_OFF               59

#define MMC_ON     1
#define MMC_OFF    0

// CD CardDetect PORTG.4
// WP PORTG.3


enum eMMC{MMC_NONE,MMC_INITIALIZED};
enum eCardState{CS_NONE,CS_INSERTED,CS_REMOVED,CS_INITIALIZED};

volatile unsigned long __startblock, __totalblocks; 
volatile unsigned char MMC_Buffer[512];

unsigned char mmc_init(void);
unsigned char mmc_command(unsigned char, unsigned long);
unsigned char mmc_readsingleblock(unsigned long);
unsigned char mmc_writesingleblock(unsigned long);
unsigned char mmc_readmultipleblock (unsigned long, unsigned long);
unsigned char mmc_writemultipleblock(unsigned long, unsigned long);
unsigned char mmc_erase (unsigned long, unsigned long);


unsigned char mmc_reset();
unsigned char mmc_initialize();
void mmc_read(unsigned long int sectoraddress);
void mmc_write(unsigned long int sectoraddress);

#endif
