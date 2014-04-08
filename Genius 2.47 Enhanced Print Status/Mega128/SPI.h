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
    
	SPI Routine [Header]
	Controller	:	ATmega128/162 (8 Mhz internal)
	Compiler	:	AVR-GCC
	Version		:	1.3
	Author		:	Yofe Fegeanto, Jakarta (Indonesia)
					www.fegeanto.com
	Date		:	24 November 2009
*/

#ifndef __SPI_H__
#define __SPI_H__

#define _SPI_DDR		DDRB
#define _SPI_MOSI		DDB2
#define _SPI_MISO		DDB3
#define _SPI_SCK		DDB1

#define _SLAVE_PORT		PORTA
#define _SLAVE_PIN		3

#define _MMC_PORT		PORTE
#define _MMC_PIN		3

#define _SPI_TX			1
#define _SPI_RX			0

#define _SPI_NONE		0
#define _SPI_SLAVE		1
#define _SPI_MMC		2

#define _SPI_DISABLE	0
#define _SPI_ENABLE		1

#define _SPI_READ		0
#define _SPI_WRITE		1

void _spi_init(unsigned char, unsigned char);
void _spi_enable(unsigned char);
unsigned char	_spi(unsigned char);

#endif
