//=========================================================
// nama file	: lcd.h
// judul		: 4 bit LCd interface header file
// by			: kristian
// Created		: 04-01-2009
// dimodifikasi dari Scienceprog.com - Copyright (C) 2007
//=========================================================
#ifndef LCD_H	//definisi agar tidak saling rancu
#define LCD_H	//dengan cara mendefinikan nama file_H 
#define LCD_RS	0 	//MCU pin connected to LCD RS
#define LCD_RW	1 	//MCU pin connected to LCD R/W
#define LCD_E	2	//MCU pin connected to LCD E
#define LCD_D4	4	//MCU pin connected to LCD D3
#define LCD_D5	5	//MCU pin connected to LCD D4
#define LCD_D6	6	//MCU pin connected to LCD D5
#define LCD_D7	7	//MCU pin connected to LCD D6
#define LDP 	PORTC		//MCU port connected to LCD data pins
#define LCP 	PORTC		//MCU port connected to LCD control pins
#define LDDR 	DDRC
//MCU direction register for port connected to LCD data pins
#define LCDR DDRC	
//MCU direction register for port connected to LCD control pins
#define LCD_CLR	0	
#define LCD_HOME 1	
void LCD_send_char(unsigned char);
void LCD_send_command(unsigned char);	
void LCD_init(void);		
void LCDstring(unsigned char*);	
#endif
