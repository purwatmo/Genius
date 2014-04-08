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
    
	LCD Routine
	Controller	:	ATmega128 (8 Mhz internal)
	Compiler	:	AVR-GCC
	Version		:	1.1
	Author		:	Yofe Fegeanto, Jakarta (Indonesia)
					www.fegeanto.com
	Date		:	24 November 2009
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <compat/deprecated.h>
#include <stdio.h>
#include <string.h>

#include "LCD.h"
#include "UART.h"

#define _LCD_BUSY	_delay_ms(4);

void lcd_init(void){
	_delay_ms(15);
	_LCD_PORT = 0x00;
	_LCD_DDR |= (1 << _LCD_D7) | (1 << _LCD_D6) | (1 << _LCD_D5) | (1 << _LCD_D4);
	_LCD_DDR |= (1 << _LCD_E) | (1 << _LCD_RW) | (1 << _LCD_RS);

	_LCD_PORT = 0x30;
	sbi(_LCD_PORT, _LCD_E);
	_LCD_BUSY;
	cbi(_LCD_PORT, _LCD_E);
	_LCD_BUSY;

	_LCD_PORT = 0x30;
	sbi(_LCD_PORT, _LCD_E);
	_LCD_BUSY;
	cbi(_LCD_PORT, _LCD_E);
	_LCD_BUSY;

	_LCD_PORT = 0x30;
	sbi(_LCD_PORT, _LCD_E);
	_LCD_BUSY;
	cbi(_LCD_PORT, _LCD_E);
	_LCD_BUSY;

	_LCD_PORT = 0x20;
	sbi(_LCD_PORT, _LCD_E);
	_LCD_BUSY;
	cbi(_LCD_PORT, _LCD_E);
	_LCD_BUSY;

	_LIGHT_PORT = 0x01;
	_LIGHT_DDR |= (1 << _LIGHT_PIN);

	lcd_command(0x28);
	lcd_command(0x0c);
	lcd_command(0x06);
}

void lcd_command(unsigned char __chr){
	_LCD_PORT = __chr & 0xF0;
	cbi(_LCD_PORT, _LCD_RS);
	sbi(_LCD_PORT, _LCD_E);
	_LCD_BUSY;
	cbi(_LCD_PORT, _LCD_E);
	_LCD_BUSY;

	_LCD_PORT = (__chr & 0x0F) << 4;
	cbi(_LCD_PORT, _LCD_RS);
	sbi(_LCD_PORT, _LCD_E);
	_LCD_BUSY;
	cbi(_LCD_PORT, _LCD_E);
	_LCD_BUSY;
	_LCD_BUSY;
	if (__chr==0x01) _delay_ms(50);

}

void _lcd(unsigned char __chr){
	_LCD_PORT =((__chr & 0xF0));
	sbi(_LCD_PORT, _LCD_RS);
	sbi(_LCD_PORT, _LCD_E);
	cbi(_LCD_PORT, _LCD_E);

	_LCD_PORT = (((__chr & 0x0F) << 4));
	sbi(_LCD_PORT, _LCD_RS);
	sbi(_LCD_PORT, _LCD_E);
	cbi(_LCD_PORT, _LCD_E);
	_LCD_BUSY;
	_delay_ms(1);
}

void lcd_clear(){
     lcd_printf(1,1,PSTR("                    "));
     lcd_printf(2,1,PSTR("                    "));
     lcd_printf(3,1,PSTR("                    "));
     lcd_printf(4,1,PSTR("                    "));
}

void lcd_string(char *__string){
	while(*__string)
		_lcd(*__string++);
}

void lcd_xy(unsigned char __x, unsigned char __y){
	switch(__x){
		case 1:
			lcd_command(0x80 + __y - 1);
			break;
		case 2:
			lcd_command(0xC0 + __y - 1);
			break;
		case 3:
			lcd_command(0x94 + __y - 1);
			break;
		case 4:
			lcd_command(0xD4 + __y - 1);
			break;
	}
}

void ClearMem(char *string){
     char i;
	 for (i=0;i<strlen(string);i++){
          string[i]=0;
	 }     
}

void lcd_print(unsigned char __x, unsigned char __y, char *__string){
    char yPos,iPos=0;//Modified by Iyan string [20] maks
	lcd_xy(__x, __y);
	iPos=0;
	while((*__string)&&(iPos<=(20-__y))){
		_lcd(*__string);__string++;
		iPos++;
		}    
}

void lcd_printf(unsigned char __x, unsigned char __y, char *__string){
    char iPos=0;
	lcd_xy(__x, __y);
	iPos=0;
	while((pgm_read_byte(&(*__string)))&&(iPos<=(20-__y))){
		_lcd(pgm_read_byte(&(*__string)));
		__string++;
		iPos++;
		}
}

void lcd_put(unsigned char __x, unsigned char __y, unsigned char __chr){
	lcd_xy(__x, __y);
	_lcd(__chr);
}

void lcd_cgram(unsigned char location, char *ptr){
	unsigned char i;
    if(location < 8){
		lcd_command(0x40 + (location * 8));
		for(i = 0; i < 8; i++)
			_lcd(ptr[i]);
	}
}
