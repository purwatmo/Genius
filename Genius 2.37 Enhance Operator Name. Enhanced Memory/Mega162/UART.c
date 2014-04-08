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
    
	UART routine
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
#include <string.h>
#include <avr/io.h>

#include "UART.h"

//_uart_setting(0,GetBaudrate(eeprom_read_byte(&DefBaudrate[0])),8,P_EVEN,1);

void _uart_setting(char Port,unsigned long Baudrate,char DataSet,char ParitySet, char StopBitSet){
unsigned long BaudrateValue;
     char RegB,RegC;
	 BaudrateValue= (unsigned long)((_CPU_DEFAULT + (Baudrate* 8)) / (Baudrate* 16) - 1);
     RegB=0;RegC=0;
	 RegB = (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0);
     RegC = (1<<URSEL0);
   
     //ParitySet
	 switch(ParitySet){
	 case P_EVEN:
	      RegC=RegC|(1<<UPM01);
	      break;
     case P_ODD:
	      RegC=RegC|(1<<UPM00)|(1<<UPM01);
	      break;	 
	 }
     //StopBit
      if (StopBitSet==2)
	      RegC=RegC|(1<<USBS0);

		  //DataSet
		  switch (DataSet){
		  case 5:
		       break;
		  case 6:
		       RegC=RegC|(1<<UCSZ00);
		       break;
		  case 7:
		       RegC=RegC|(1<<UCSZ01);
		       break;
		  case 8:
		       RegC=RegC|(1<<UCSZ00)|(1<<UCSZ01);
		       break;
		  case 9:
               RegB=RegB| (1<<UCSZ02);
		       RegC=RegC| (1<<UCSZ00)|(1<<UCSZ01);
		       break;
		  }

	 switch (Port){
	 case 0:		  
		  //Write
		  //Register

	      //BaudrateSetting
          UBRR0H=BaudrateValue>>8;
		  UBRR0L=BaudrateValue;		
		  //Register
		  UCSR0B=RegB; 
		  UCSR0C=RegC; 
	      break;
     case 1:
		  //Write
		  //BaudrateSetting
          UBRR1H=BaudrateValue>>8;
		  UBRR1L=BaudrateValue;		

		  //Register
		  UCSR1B=RegB; 
		  UCSR1C=RegC;
	      break;
	 }  
}

void _uart_init(unsigned char __com, unsigned long __baudrate){
	if(__com){
		UCSR1B = _BV(RXCIE1) | _BV(RXEN1) | _BV(TXEN1);
		UCSR1C = _BV(UCSZ11) | _BV(UCSZ10);
	}
	else{
		UCSR0B = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);
		UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
	}
    

	_uart_baudrate(__com, __baudrate);

	if (__baudrate==5787){
	    UCSR0B=_BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);
	    UCSR0C=(1<<URSEL0) | (1<<UPM01) | (1<<UCSZ01) | (1<<UCSZ00); // UPM01 untuk set even parity
	    UBRR0H=0;UBRR0L=158;// baud 5787       
	}
}

void _uart_baudrate(unsigned char __com, unsigned long __baudrate){
	unsigned long __br;

	__br = (unsigned long)((_CPU_DEFAULT + (__baudrate * 8)) / (__baudrate * 16) - 1);

	if(__com){
		UBRR1L = __br;
		UBRR1H = __br >> 8;
	}
	else{
		UBRR0L = __br;
		UBRR0H = __br >> 8;
	}
}

unsigned char _uart(unsigned char __com, unsigned char __dir, unsigned char __chr){
	if(__com){
		if(__dir){
			loop_until_bit_is_set(UCSR1A, UDRE1);
			UDR1 = __chr;
		}
		else{
			loop_until_bit_is_set(UCSR1A, RXC1);
			return UDR1;
		}
	}
	else{
		if(__dir){
			loop_until_bit_is_set(UCSR0A, UDRE0);
			UDR0 = __chr;
		}
		else{
			loop_until_bit_is_set(UCSR0A,  RXC0);
			return UDR0;
		}

	}

	return 1;
}

void _uart_print(unsigned char __com, unsigned char __ret, char *__str){
	if(strlen(__str)>0)
		while(*__str) _uart(__com, 1, *__str++);

	if(__ret){
		_uart(__com, 1, 0x0D);
		_uart(__com, 1, 0x0A);
	}
	_delay_ms(5);
}

void _uart_printf(unsigned char __com, unsigned char __ret, char *__str){
	while(pgm_read_byte(&(*__str)))
		_uart(__com, 1, pgm_read_byte(&(*__str++)));

	if(__ret){
		_uart(__com, 1, 0x0D);
		_uart(__com, 1, 0x0A);
	}
	_delay_ms(15);
}
