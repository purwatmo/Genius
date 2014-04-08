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

#include "UART.h"

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
			loop_until_bit_is_set(UCSR1A, RXC);
			return UDR1;
		}
	}
	else{
		if(__dir){
			loop_until_bit_is_set(UCSR0A, UDRE0);
			UDR0 = __chr;
		}
		else{
			loop_until_bit_is_set(UCSR0A,  RXC);
			return UDR0;
		}

	}

	return 1;
}

void _uart_print(unsigned char __com, unsigned char __ret, char *__str){
	if(strlen(__str)>0)
		while(*__str)
		     _uart(__com, 1, *__str++);

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

void Int2Str(char* __string, unsigned long __value){
	char			__flag = 0, __i = 0, __count;
	unsigned long	__num, __devider = 1000000000;
	int				__tmp;

	if(__value == 0){
		__string[0] = '0';
		__string[1] = '\0';
		return;
	}

	__num = __value;
	for(__count = 0; __count < 10; __count++){
		if(__num >= __devider){
			__tmp = (int)(__num / __devider);
			__num = __num % __devider;
			__string[__i++] = __tmp + 0x30;
			if(!__flag)
				__flag = 1;
		}
		else
			if(__flag)
				__string[__i++] = 0x30;
		__devider = __devider / 10;
	}
	__string[__i] = '\0';
}

unsigned long Str2Int(char* __string){
	unsigned char	__i, __len;
	unsigned long	__multiplier = 1, __retval = 0;

	__len = strlen(__string);

	if(__len > 0 && __len < 11){
		for(__i = 0; __i < __len; __i++)
			if(__string[__i] < 0x30 || __string[__i] > 0x39)
				return 0;
		if(__len == 10){
			if(__string[0] < 0x30 || __string[0] > 0x32)
				return 0;
			if(__string[0] == 0x32){
				for(__i = 1; __i < __len; __i++)
					if(__string[__i] < 0x30 || __string[__i] > 0x30)
						return 0;
			}
		}
		for(__i = 0; __i < __len - 1; __i++)
			__multiplier *= 10;
		for(__i = 0; __i < __len; __i++, __multiplier /= 10)
			__retval += ((unsigned long)(__string[__i] - 0x30) * __multiplier);
	}
	return __retval;
}
