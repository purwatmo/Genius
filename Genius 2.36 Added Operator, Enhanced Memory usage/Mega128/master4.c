//Genius V2.20
//LastUpdated:28/08/2010
//Fcloseshift

#define F_CPU 14745600UL
#define False 0
#define True 1

#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <compat/deprecated.h>
#include <compat/twi.h>
#include <util/delay.h>
#include "LCD.h"
#include "KEYPAD.h"
#include "SPI.h"
#include "RTC.h"
#include "UART.h"
#include "f_menu.h"
//#include "MMC.h"
//#include "FAT32.h"

#define DELAY_TIM_DISPLAY 300

//Master4 =1+20++2+1
char IsMMCExist=False;

unsigned int iExt;
void SystemInit();


//Program Utama
int main(){
	SystemInit();
	while(1){
	//FTestChar();
	//TestUserInput();
    FMenuIdle();
//	FSettingPumpID();
     //FTestCalculation();
	 //FTestRemZero();
	 //system_beep(1);
	};
}


void systemOnReset(){
     IsPowerOn=False;
     if (MCUCSR & 1){   // Power-on Reset
         MCUCSR=0;lcd_printf(1,1,PSTR("PowerOn"));//Wiznet Blm Ready
		 IsPowerOn=True;
         }
     else 
     if (MCUCSR & 2){   // External Reset
         MCUCSR=0;lcd_printf(1,1,PSTR("External"));
        }
     else 
     if (MCUCSR & 4){   // Brown-Out Reset
         MCUCSR=0;lcd_printf(1,1,PSTR("BrownOut"));
         }
     else		  // Watchdog Reset
        {
         lcd_printf(1,1,PSTR("WatchDog"));
        };
     MCUCSR=0;		  
	 _delay_ms(1200);
}


void SystemInit(){
	unsigned int __delay =300;
	lcd_init();
	lcd_clear();
	BackLightTrig();
	systemOnReset();

	lcd_printf(4, 1, PSTR("Initialize... "));
	_delay_ms(__delay);
	PORTA = 0XFF;		// Buffer for PORTA
	DDRA = 0xFF;		// Pin 3 out (SS _SPI_SLAVE)
	sbi(PORTA, 3);		// Disable SS _SPI_SLAVE

	//Buzzer PORTB.5
	PORTB = 0xFF;DDRB = 0xFF;
	PORTD = 0b00001101;
    DDRD =  0b00001001;

	TWBR = 0xFF;
    //MMC
	PORTE = 0x0E;		// Buffer for PORTE
	DDRE = 0x0E;		// Pin 3 out (SS _SPI_MMC), pin 2 out (HB)
	sbi(PORTE, 3);		// Disable SS _SPI_MMC
	cbi(PORTE, 2);		// HB on

	//Keypad
	PORTF = 0xFF;		// Buffer for PORTF
	DDRF = 0xF0;		// 4 bit row, 4 bit column

  	_spi_init(0,1);//Slave
	sbi(DDRB,3);sbi(PORTB,3);//MISO Output
	cbi(DDRB,2);sbi(PORTB,2);//MOSI Input
	cbi(DDRB,1);sbi(PORTB,1);//SCK  Input

	TCCR1B |= (1 << WGM12);
	TIMSK |= (1 << OCIE1A);
	sei();          //1/14745600=0,06781684028uS *1400 = 100uS 65535-1400+1=64136
	OCR1A   = 15624;//49911 -->0,33847384982639 ms
	TCCR1B |= ((1 << CS10) | (1 << CS11));

	SendSlaveCommand(SC_SLAVE,ST_NONE);
	
	InitComport();
	_LIGHT_SET;
	InitPrinter();
    InitializeConnection();
	lcd_clear(); 
}





