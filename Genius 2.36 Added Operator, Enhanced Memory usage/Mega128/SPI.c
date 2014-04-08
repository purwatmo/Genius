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

unsigned char	__spi_char;
unsigned char	__spi_i;
unsigned char	__spi_isstart = 0;
unsigned char	__spi_ismessage = 0;

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

/*
**	PROTOCOL FUNCTION
**
**	SOT  (Start of Transmission)	:	0x7e
**	EOT  (End of Transmission)		:	0x7D
**	STX  (Start of Text)			:	0x02
**	ETX  (End of Text)				:	0x03
**	ACK  (Acknowledge)				:	0x13
**	NACK (Negative Acknowledge)		:	0x27
**  method send command				:	0x55
*/


unsigned char _spi_rx(unsigned char __select){
	unsigned char __i = 0;

	_spi_enable(__select);

		__spi_char = _spi(0x41);
		if(__spi_char==0xff) return 0;
		if(__spi_char==0x55) return 1;
		if(__spi_char==0x02) return 2;
	_spi_enable(_SPI_NONE);
	return 21;
}

unsigned char _spi_tx(unsigned char __command, unsigned char __select, char* __message){
	_spi_enable(__select);
	_spi(0x7B);
	_spi(__command);

	if(strlen(__message) > 0){
		_spi(0x28);
		while(*__message)
			_spi(*__message++);
		_spi(0x29);
	}

	_spi(0x7D);
	_spi_enable(_SPI_NONE);
	return 0;
}

unsigned char _spi_txnum(unsigned char __command, unsigned char __select, char* __message, unsigned char __num){
	unsigned char	__i;

	_spi_enable(__select);
	_spi(0x02);

	if(__num > 0){
		for(__i = 0; __i < __num; __i++) _spi(__message[__i]);
				}// for

	_spi(0x03);
	_spi_enable(_SPI_NONE);
	return 0;
}

unsigned char _spi_interrupt(void){
	__spi_char = SPDR;

	if(__spi_char == 0xFF){
		if(__spi_istransmit){
			SPDR = __command[__spi_i];
			if(__command[__spi_i] == 0x7D){
				__spi_i = 0;
				__spi_istransmit = 0;
			}
			else{
				__spi_i++;
				if(__spi_i == _MAX_COMMAND)
					__spi_istransmit = 0;
			}
		}
		else
			SPDR = 0xFF;
	}
	else{
		if(__spi_char == 0x7D && __spi_isstart){
			__spi_isstart = 0;
			__spi_ismessage = 0;
			__spi_i = 0;
			__command_flag = 1;
		}
		if(__spi_isstart && __spi_ismessage){
			if(__spi_char != 0x29){
				__package[__spi_i] = __spi_char;
				__package[__spi_i + 1] = '\0';
				__spi_i++;
				if(__spi_i == _MAX_PACKAGE)
					__spi_i = 0;
			}
		}
		if(__spi_isstart && !__spi_ismessage){
			if(__spi_char == 0x28){
				__spi_ismessage = 1;
				__spi_i = 0;
			}
			else
				__command_set = __spi_char;
		}
		if(__spi_char == 0x7B && !__spi_isstart){
			__spi_isstart = 1;
			__spi_ismessage = 0;
		}
	}

	return 0;
}

void _spi_command(unsigned char __cmd, char* __cell){
	unsigned char __i = 0;

	__command[__i++] = 0x7B;
	__command[__i++] = __cmd;

	if(strlen(__cell) > 0){
		__command[__i++] = 0x28;

		while(*__cell)
			__command[__i++] = *__cell++;

		__command[__i++] = 0x29;
	}

	__command[__i++] = 0x7D;
	__command[__i++] = '\0';
}
