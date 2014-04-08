#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <compat/deprecated.h>
#include <stdio.h>
#include <string.h>
#include "UART.h"

#include "MMC.h"
#include "SPI.h"

#define MMC_CS_ASSERT     spi_enable(SPI_MMC)
#define MMC_CS_DEASSERT   spi_enable(SPI_NONE)

unsigned char mmc_init(void){
	unsigned char i, response, retry = 0 ;

	MMC_CS_ASSERT;
	do{
		for(i = 0; i < 10; i++)
			spi(0xFF);
		response = mmc_command(MMC_GO_IDLE_STATE, 0);
		retry++;
		if(retry > 0xFE)
			return 1;
	}while(response != 0x01);

	MMC_CS_DEASSERT;
	spi(0xFF);
	spi(0xFF);

	retry = 0;

	do{
		response = mmc_command(MMC_SEND_OP_COND, 0);
		response = mmc_command(MMC_SEND_OP_COND, 0);
		retry++;
		if(retry > 0xFE)
			return 1;
	}while(response);

	mmc_command(MMC_CRC_ON_OFF, MMC_OFF);
	mmc_command(MMC_SET_BLOCK_LEN, 512);

	return 0;
}

unsigned char mmc_command(unsigned char cmd, unsigned long arg){
	unsigned char response, retry = 0;

	MMC_CS_ASSERT;

	spi(cmd | 0x40);
	spi(arg >> 24);
	spi(arg >> 16);
	spi(arg >> 8);
	spi(arg);
	spi(0x95);

	while((response = spi(0xFF)) == 0xFF)
		if(retry++ > 0xFE)
			break;

	spi(0xFF);
	MMC_CS_DEASSERT;

	return response;
}

unsigned char mmc_erase(unsigned long startblock, unsigned long totalblocks){
	unsigned char response;

	response = mmc_command(MMC_ERASE_BLOCK_START_ADDR, startblock << 9);
	if(response != 0x00)
		return response;

	response = mmc_command(MMC_ERASE_BLOCK_END_ADDR,(startblock + totalblocks - 1) << 9);
	if(response != 0x00)
		return response;

	response = mmc_command(MMC_ERASE_SELECTED_BLOCKS, 0);
	if(response != 0x00)
		return response;

	return 0;
}

unsigned char mmc_readsingleblock(unsigned long startblock){
	unsigned char response;
	unsigned int i, retry = 0;

	response = mmc_command(MMC_READ_SINGLE_BLOCK, startblock << 9);

	if(response != 0x00)
		return response;

	MMC_CS_ASSERT;

	retry = 0;
	while(spi(0xFF) != 0xFE)
		if(retry++ > 0xFFFE){
			MMC_CS_DEASSERT;
			return 1;
		}

	for(i = 0; i < 512; i++)
		MMC_Buffer[i] = spi(0xFF);

	spi(0xFF);
	spi(0xFF);

	spi(0xFF);
	MMC_CS_DEASSERT;

	return 0;
}

unsigned char mmc_writesingleblock(unsigned long startblock){
	unsigned char response;
	unsigned int i, retry=0;

	response = mmc_command(MMC_WRITE_SINGLE_BLOCK, startblock << 9);
	if(response != 0x00)
		return response;

	MMC_CS_ASSERT;

	spi(0xFE);

	for(i = 0; i < 512; i++)
		spi(MMC_Buffer[i]);

	spi(0xFF);
	spi(0xFF);

	response = spi(0xFF);

	if((response & 0x1F) != 0x05){
	  MMC_CS_DEASSERT;
	  return response;
	}

	while(!spi(0xFF))
		if(retry++ > 0xFFFE){
			MMC_CS_DEASSERT;
			return 1;
		}

	MMC_CS_DEASSERT;
	spi(0xFF);
	MMC_CS_ASSERT;

	while(!spi(0xFF))
		if(retry++ > 0xFFFE){
			MMC_CS_DEASSERT;
			return 1;
		}
	MMC_CS_DEASSERT;

	return 0;
}

//----------------------
unsigned char mmc_reset(){
unsigned char i;
   
   MMC_CS_DEASSERT;

   for (i=0;i<10;i++){
        write_SPI(0xFF);
   }
                                 
  MMC_CS_ASSERT;

   write_SPI(0x40);              
   for (i=0;i<4;i++) write_SPI(0x00);         
   write_SPI(0x95);
   
   SPDR=0xFF;

   for (i=0;i<9;i++){
        if (SPDR==0xFF) 
		    write_SPI(0xFF);
		else break;
   }
   MMC_CS_DEASSERT;  
   return SPDR;//01

}



unsigned char mmc_initialize(){
unsigned char xsdinit, ysdinit,i,j;
char Result;

   Result=MMC_NONE;
   for(i=0;i<250;i++){
       write_SPI(0xFF);
       MMC_CS_ASSERT;
       write_SPI(0x41);
       for(j=0;j<4;j++)write_SPI(0x00);      
	   write_SPI(0xFF);
       SPDR=0xFF;
	   for(j=0;j<9;j++)write_SPI(0xFF);
	   MMC_CS_DEASSERT;
       if (SPDR==0){
	       Result=MMC_INITIALIZED;
	       break;
	   }
   }   
   return Result;
}


void mmc_read(unsigned long int sectoraddress){
   unsigned char xsdread, ysdread;
   unsigned int xsdrcount;
   
   sbi(MMC_PORT, MMC_PIN);
   sectoraddress*=0x200;  
   for(ysdread=0;ysdread<250;ysdread++){
      write_SPI(0xFF);
      _delay_us(5);
      //MMC_CS_ASSERT;
	  cbi(MMC_PORT, MMC_PIN);
      _delay_us(5);
      write_SPI(0x51);
      xsdread=(sectoraddress>>24)&0xFF;
      write_SPI(xsdread);
      xsdread=(sectoraddress>>16)&0xFF;
      write_SPI(xsdread);
      xsdread=(sectoraddress>>8)&0xFF;
      write_SPI(xsdread);
      xsdread=sectoraddress&0xFF;
      write_SPI(xsdread);
      write_SPI(0xFF);
      SPDR=0xFF;
	  for (xsdread=0;xsdread<9;xsdread++){
	       if (SPDR==0xFF) 
		       write_SPI(0xFF); 
		   else break;
	  }      
      if (SPDR==0){
	      uart_printf(0,1,PSTR("Read Response OK"));
	      break;      
	   }else uart_printf(0,1,PSTR("Read Response Failed"));
    }//EndFor
       
   for(ysdread=0;ysdread<250;ysdread++){
      SPDR=0xFF;
      for(xsdread=0;xsdread<9;xsdread++){
         if (SPDR==0xFF) write_SPI(0xFF); 
		 else break;         
         }
      if (SPDR==0xFE) break;      
    }
   
   for(xsdrcount=0;xsdrcount<512;xsdrcount++){
       write_SPI(0xFF);
	   MMC_Buffer[xsdrcount]=SPDR;//Store Read data to MMC_Data	  
   }   
   write_SPI(0xFF);
   write_SPI(0xFF);
   _delay_us(5);
   //MMC_CS_DEASSERT;
   sbi(MMC_PORT, MMC_PIN);
}

void mmc_write(unsigned long int sectoraddress){
   unsigned char xsdwrite, ysdwrite;
   unsigned int xsdwcount;
   char strSend[30];

   uart_printf(1,1,PSTR("-mmc_write-"));

   sbi(MMC_PORT, MMC_PIN);
   sectoraddress*=0x200;
   for(ysdwrite=0;ysdwrite<250;ysdwrite++){
      write_SPI(0xFF);
      _delay_us(5);
      MMC_CS_ASSERT;
	  cbi(MMC_PORT, MMC_PIN);

      _delay_us(5);
      write_SPI(0x58);
      xsdwrite=(sectoraddress>>24)&0xFF;
      write_SPI(xsdwrite);
      xsdwrite=(sectoraddress>>16)&0xFF;
      write_SPI(xsdwrite);
      xsdwrite=(sectoraddress>>8)&0xFF;
      write_SPI(xsdwrite);
      xsdwrite=sectoraddress&0xFF;
      write_SPI(xsdwrite);
      write_SPI(0xFF);
      SPDR=0xFF;
      xsdwrite=0;
	  //Wait MMC Busy OK->SPDR=0
	  for(xsdwrite=0;xsdwrite<9;xsdwrite++){
         if (SPDR==0xFF) write_SPI(0xFF); 
		 else break;
        }
      sprintf_P(strSend,PSTR("SPDR=%.2X"),SPDR);
	  uart_print(0,1,strSend);

	   
      if (SPDR==0){
          uart_printf(0,1,PSTR("Cmd Response OK")); 
	      break;
	  }
    }

   for (xsdwrite=0;xsdwrite<9;xsdwrite++){
        write_SPI(0xFF);
    }
   write_SPI(0xFE);
   
   for(xsdwcount=0;xsdwcount<512;xsdwcount++){
      write_SPI(MMC_Buffer[xsdwcount]);
    }
   write_SPI(0xFF);
   write_SPI(0xFF);
   
   for (ysdwrite=0;ysdwrite<250;ysdwrite++){
      SPDR=0xFF;
      for(xsdwrite=0;xsdwrite<9;xsdwrite++){
         if (SPDR==0xFF) write_SPI(0xFF); 
		 else break;
        }
      xsdwrite=SPDR&0x0F;
      if (xsdwrite==0x05){
          sprintf_P(strSend,PSTR("Write Sucessfull"));
	      uart_print(0,1,strSend);
	      break;
	  }else{
	      sprintf_P(strSend,PSTR("MMC Write Failed"));
	      uart_print(0,1,strSend);
	  }

      ysdwrite++;
    }

   SPDR=0xFF;
   for(ysdwrite=0;ysdwrite<250;ysdwrite++){
      write_SPI(0xFF);
      if (SPDR==0x00) break;      
    }
	
   SPDR=0xFF;   
   for (ysdwrite=0;ysdwrite<250;ysdwrite++){
      write_SPI(0xFF);
      if (SPDR!=0x00) break;
      }
   MMC_CS_DEASSERT;
   //sbi(MMC_PORT, MMC_PIN);
}

