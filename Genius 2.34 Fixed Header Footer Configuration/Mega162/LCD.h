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
    
	LCD Routine - Header
	Controller	:	ATmega128 (8 Mhz internal)
	Compiler	:	AVR-GCC
	Version		:	1.1
	Author		:	Yofe Fegeanto, Jakarta (Indonesia)
					www.fegeanto.com
	Date		:	24 November 2009
*/

#ifndef	__LCD_H__
#define __LCD_H__

#define _LCD_PORT	PORTC
#define _LCD_DDR	DDRC

#define _LCD_E		1
#define _LCD_RW		2
#define _LCD_RS		3
#define _LCD_D4		4
#define _LCD_D5		5
#define _LCD_D6		6
#define _LCD_D7		7

#define _LIGHT_PORT	PORTG
#define _LIGHT_DDR	DDRG
#define _LIGHT_PIN	1

#define _LCD_CLR	_lcd_command(1 << 0) 
#define _LCD_HOME	_lcd_command(1 << 1)
#define _LCD_LEFT	_lcd_command(0x10)
#define _LCD_RIGHT	_lcd_command(0x14)

void	_lcd_init(void);
void	_lcd_command(unsigned char);
void	_lcd(unsigned char);
void	_lcd_string(char*);
void	_lcd_xy(unsigned char, unsigned char);
void	_lcd_print(unsigned char, unsigned char, char*);
void	_lcd_printf(unsigned char __x, unsigned char __y, char *__string);
void	_lcd_put(unsigned char, unsigned char, unsigned char);
void	_lcd_cgram(unsigned char, char*);

#endif
