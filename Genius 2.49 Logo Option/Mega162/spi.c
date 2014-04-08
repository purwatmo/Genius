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
    
	SPI Routine
	Controller	:	ATmega162/128 (8 Mhz internal)
	Compiler	:	AVR-GCC
	Version		:	1.3
	Author		:	Yofe Fegeanto, Jakarta (Indonesia)
					www.fegeanto.com
	Date		:	24 November 2009
*/

#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <compat/deprecated.h>
#include <avr/interrupt.h>

#include "SPI.h"

enum SPI_TYPE{SLAVE,MASTER};
enum SPI_MODE{POOLING,INTERUPT};


void _spi_init(unsigned char __mode, unsigned char __interrupt){
	if(__mode){
		_SPI_DDR = (1 << _SPI_MOSI) | (1 << _SPI_SCK);
		if(__interrupt)
			SPCR = (1 << SPIE) | (1 << SPE) | (1 << MSTR) | (1 << SPR1);
		else
			SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1);
	}
	else{

		_SPI_DDR = (1 << _SPI_MISO);
		if(__interrupt)
			{SPCR = (1 << SPIE) | (1 << SPE);sei();}
		else
			SPCR = (1 << SPE);
			
	}
}

void _spi_enable(unsigned char __status, unsigned char __select){
	if(__status){
		if(__select)
			cbi(_MMC_PORT, _MMC_PIN);
		else
			cbi(_SLAVE_PORT, _SLAVE_PIN);
	}
	else{
		if(__select)
			sbi(_MMC_PORT, _MMC_PIN);
		else
			sbi(_SLAVE_PORT, _SLAVE_PIN);
	}
}

unsigned char _spi(unsigned char __data){   
	SPDR = __data;
	while(!(SPSR & (1 << SPIF)));
	return SPDR;
}

unsigned char _spi_rx(unsigned char __select){
//	return 1;
}

unsigned char _spi_tx(unsigned char __command, unsigned char __select, char *__message){
//	_spi_enable(_SPI_ENABLE, __select);
//	_spi_enable(_SPI_DISABLE, __select);
//	return 0;
}

unsigned char _spi_interrupt(void){
	//return 0;
}

void _spi_command(unsigned char _cmd, char *_cell){
	/*unsigned char __i = 0;

	__command[__i++] = 0x7B;
	__command[__i++] = _cmd;
	
	if(strlen(_cell) > 0){
		__command[__i++] = 0x28;

		while(*_cell)
			__command[__i++] = *_cell++;

		__command[__i++] = 0x29;
	}

	__command[__i++] = 0x7D;
	__command[__i++] = '\0';
	*/
}
