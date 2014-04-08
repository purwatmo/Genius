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
    
	MMC/SD CARD Routine
	Controller	:	ATmega162/128 (8 Mhz internal)
	Compiler	:	AVR-GCC
	Version		:	1.3
	Author		:	Yofe Fegeanto, Jakarta (Indonesia)
					www.fegeanto.com
	Date		:	24 November 2009
*/

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "MMC.h"
#include "SPI.h"

#define _MMC_CS_ASSERT     _spi_enable(_SPI_MMC)
#define _MMC_CS_DEASSERT   _spi_enable(_SPI_NONE)

unsigned char _mmc_init(void){
	unsigned char __i, __response, __retry = 0 ;

	_MMC_CS_ASSERT;
	do{
		for(__i = 0; __i < 10; __i++)
			_spi(0xFF);
		__response = _mmc_command(_MMC_GO_IDLE_STATE, 0);
		__retry++;
		if(__retry > 0xFE)
			return 1;
	}while(__response != 0x01);

	_MMC_CS_DEASSERT;
	_spi(0xFF);
	_spi(0xFF);

	__retry = 0;

	do{
		__response = _mmc_command(_MMC_SEND_OP_COND, 0);
		__response = _mmc_command(_MMC_SEND_OP_COND, 0);
		__retry++;
		if(__retry > 0xFE)
			return 1;
	}while(__response);

	_mmc_command(_MMC_CRC_ON_OFF, _MMC_OFF);
	_mmc_command(_MMC_SET_BLOCK_LEN, 512);

	return 0;
}

unsigned char _mmc_command(unsigned char __cmd, unsigned long __arg){
	unsigned char __response, __retry = 0;

	_MMC_CS_ASSERT;

	_spi(__cmd | 0x40);
	_spi(__arg >> 24);
	_spi(__arg >> 16);
	_spi(__arg >> 8);
	_spi(__arg);
	_spi(0x95);

	while((__response = _spi(0xFF)) == 0xFF)
		if(__retry++ > 0xFE)
			break;

	_spi(0xFF);
	_MMC_CS_DEASSERT;

	return __response;
}

unsigned char _mmc_erase(unsigned long __startblock, unsigned long __totalblocks){
	unsigned char __response;

	__response = _mmc_command(_MMC_ERASE_BLOCK_START_ADDR, __startblock << 9);
	if(__response != 0x00)
		return __response;

	__response = _mmc_command(_MMC_ERASE_BLOCK_END_ADDR,(__startblock + __totalblocks - 1) << 9);
	if(__response != 0x00)
		return __response;

	__response = _mmc_command(_MMC_ERASE_SELECTED_BLOCKS, 0);
	if(__response != 0x00)
		return __response;

	return 0;
}

unsigned char _mmc_readsingleblock(unsigned long __startblock){
	unsigned char __response;
	unsigned int __i, __retry = 0;

	__response = _mmc_command(_MMC_READ_SINGLE_BLOCK, __startblock << 9);

	if(__response != 0x00)
		return __response;

	_MMC_CS_ASSERT;

	__retry = 0;
	while(_spi(0xFF) != 0xFE)
		if(__retry++ > 0xFFFE){
			_MMC_CS_DEASSERT;
			return 1;
		}

	for(__i = 0; __i < 512; __i++)
		__mmc_buffer[__i] = _spi(0xFF);

	_spi(0xFF);
	_spi(0xFF);

	_spi(0xFF);
	_MMC_CS_DEASSERT;

	return 0;
}

unsigned char _mmc_writesingleblock(unsigned long __startblock){
	unsigned char __response;
	unsigned int __i, __retry=0;

	__response = _mmc_command(_MMC_WRITE_SINGLE_BLOCK, __startblock << 9);
	if(__response != 0x00)
		return __response;

	_MMC_CS_ASSERT;

	_spi(0xFE);

	for(__i = 0; __i < 512; __i++)
		_spi(__mmc_buffer[__i]);

	_spi(0xFF);
	_spi(0xFF);

	__response = _spi(0xFF);

	if((__response & 0x1F) != 0x05){
	  _MMC_CS_DEASSERT;
	  return __response;
	}

	while(!_spi(0xFF))
		if(__retry++ > 0xFFFE){
			_MMC_CS_DEASSERT;
			return 1;
		}

	_MMC_CS_DEASSERT;
	_spi(0xFF);
	_MMC_CS_ASSERT;

	while(!_spi(0xFF))
		if(__retry++ > 0xFFFE){
			_MMC_CS_DEASSERT;
			return 1;
		}
	_MMC_CS_DEASSERT;

	return 0;
}