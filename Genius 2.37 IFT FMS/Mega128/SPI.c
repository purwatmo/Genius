#include <avr/io.h>
#include <util/delay.h>
#include <compat/deprecated.h>

#include "SPI.h"
void spi_init(unsigned char mode, unsigned char interrupt){
	if(mode){
		SPI_DDR = (1 << SPI_MOSI) | (1 << SPI_SCK);
		if(interrupt)
			SPCR = (1 << SPIE) | (1 << SPE) | (1 << MSTR) | (1 << SPR1);
		else
			SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1);
	}
	else{
		SPI_DDR = (1 << SPI_MISO);
		if(interrupt)
			SPCR = (1 << SPIE) | (1 << SPE);
		else
			SPCR = (1 << SPE);
	}
}

void spi_enable(unsigned char select){
	if(select == SPI_NONE){
		sbi(SLAVE_PORT, SLAVE_PIN);
		sbi(MMC_PORT, MMC_PIN);
	}
	if(select == SPI_SLAVE){
		sbi(MMC_PORT, MMC_PIN);
		_delay_ms(5);
		cbi(SLAVE_PORT, SLAVE_PIN);
	}
	if(select == SPI_MMC){
		sbi(SLAVE_PORT, SLAVE_PIN);
		_delay_ms(5);
		cbi(MMC_PORT, MMC_PIN);
	}
}

unsigned char spi(unsigned char data){
	SPDR = data;
	while(!(SPSR & (1 << SPIF)));
	return SPDR;
}

void write_SPI(char dataspi){
     SPDR = dataspi;
     while ((SPSR&0x80)!=0x80);//wait for data transfer to be completed
}
