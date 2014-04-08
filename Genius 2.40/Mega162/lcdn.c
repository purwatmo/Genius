//=========================================================
// nama file	: lcdn.c
// judul		: 4 bit LCd interface c file
// by			: kristian sutikno
// Created		: 04-02-2007
// dimodifikasi dari Scienceprog.com - Copyright (C) 2007
//=========================================================
/*
#include	"lcdn.h"
#include	<util/delay.h>
#include	<avr/io.h>
#include <compat/deprecated.h> // 
//Sends Char to LCD
void LCD_send_char(unsigned char ch) 
{
	LDP=(ch&0xf0);	//tulis nible atas
	sbi(LCP,LCD_RS); // data
	sbi(LCP,LCD_E);	//e0		
	cbi(LCP,LCD_E);	//e1
	LDP=((ch&0x0f)<<4); //tulis nible bawah
	sbi(LCP,LCD_RS); // data
	sbi(LCP,LCD_E);	//e0		
	cbi(LCP,LCD_E);	//e1
	_delay_ms(1);
	}
//Sends Command to LCD
void LCD_send_command(unsigned char cmd) 
{
	LDP=(cmd&0xf0);	//tulis nible atas
	cbi(LCP,LCD_RS); //perintah/instruksi
	sbi(LCP,LCD_E);	//e0		
	_delay_ms(1);
	cbi(LCP,LCD_E);	//e1
	_delay_ms(1);
	LDP=((cmd&0x0f)<<4); //nible bawah
	cbi(LCP,LCD_RS); //perintah/instruksi
	sbi(LCP,LCD_E);	//e0		
	_delay_ms(1);
	cbi(LCP,LCD_E);	//e1
	_delay_ms(2);
}
void LCD_init(void) //Inisilaisasi LCD
{
	_delay_ms(15);
	LDP=0x00;
	LDDR=0xff;  // out semua
	LDP=0x30;			// nilai awal 4-bit 2-line 4x8
	sbi(LCP,LCD_E);	//e0
	_delay_ms(1);
	cbi(LCP,LCD_E);	//e1
	_delay_ms(5);
	LDP=0x30;			// nilai awal
	sbi(LCP,LCD_E);	//e0		
	_delay_ms(1);
	cbi(LCP,LCD_E);	//e1
	_delay_ms(1);
	LDP=0x30;			// nilai awal
	sbi(LCP,LCD_E);	//e0		
	_delay_ms(1);
	cbi(LCP,LCD_E);	//e1
	_delay_ms(1);
	LDP=0x20;			// operasi 4 bit
	sbi(LCP,LCD_E);	//e0		
	_delay_ms(1);
	cbi(LCP,LCD_E);	//e1
	_delay_ms(1);
	LCD_send_command(0x28); // 4 bit
	LCD_send_command(0x0c); //disply on, cursor off, bling off
	LCD_send_command(0x06); //shift inc, unsivible
}
void LCD_string(char* str){
     unsigned char x=0;
	while(str[x]!=0){_delay_ms(2);
			LCD_send_char(str[x]);
			x++;}
}



*/
