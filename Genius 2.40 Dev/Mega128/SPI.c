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
	Controller	:	ATmega128/162 (8 Mhz internal)
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

#include "SPI.h"
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
			SPCR = (1 << SPIE) | (1 << SPE);
		else
			SPCR = (1 << SPE);
	}
}

void _spi_enable(unsigned char __select){
	if(__select == 0){
		sbi(_SLAVE_PORT, _SLAVE_PIN);
		sbi(_MMC_PORT, _MMC_PIN);
	}
	if(__select == 1){
		sbi(_MMC_PORT, _MMC_PIN);
		_delay_ms(5);
		cbi(_SLAVE_PORT, _SLAVE_PIN);
	}
	if(__select == 2){
		sbi(_SLAVE_PORT, _SLAVE_PIN);
		_delay_ms(5);
		cbi(_MMC_PORT, _MMC_PIN);
	}
}

unsigned char _spi(unsigned char __data){
	SPDR = __data;
	while(!(SPSR & (1 << SPIF)));
	return SPDR;
}
