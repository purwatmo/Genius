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

#ifndef _MMC_H_
#define _MMC_H_

#define _COM_HOST	1

#define _MMC_GO_IDLE_STATE            0
#define _MMC_SEND_OP_COND             1
#define _MMC_SEND_CSD                 9
#define _MMC_STOP_TRANSMISSION        12
#define _MMC_SEND_STATUS              13
#define _MMC_SET_BLOCK_LEN            16
#define _MMC_READ_SINGLE_BLOCK        17
#define _MMC_READ_MULTIPLE_BLOCKS     18
#define _MMC_WRITE_SINGLE_BLOCK       24
#define _MMC_WRITE_MULTIPLE_BLOCKS    25
#define _MMC_ERASE_BLOCK_START_ADDR   32
#define _MMC_ERASE_BLOCK_END_ADDR     33
#define _MMC_ERASE_SELECTED_BLOCKS    38
#define _MMC_CRC_ON_OFF               59

#define _MMC_ON     1
#define _MMC_OFF    0

volatile unsigned long __startblock, __totalblocks; 
volatile unsigned char __mmc_buffer[512];

unsigned char _mmc_init(void);
unsigned char _mmc_command(unsigned char, unsigned long);
unsigned char _mmc_readsingleblock(unsigned long);
unsigned char _mmc_writesingleblock(unsigned long);
unsigned char _mmc_readmultipleblock (unsigned long, unsigned long);
unsigned char _mmc_writemultipleblock(unsigned long, unsigned long);
unsigned char _mmc_erase (unsigned long, unsigned long);

#endif
