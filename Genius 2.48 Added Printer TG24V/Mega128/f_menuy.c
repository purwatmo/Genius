#include <stdio.h>
#include <string.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "f_menu.h"
#include "LCD.h"
#include "KEYPAD.h"
#include "RTC.h"
#include "UART.h"
#include "FAT32.h"
#include "SPI.h"


#define _GET_POOLING	{if(__sec_sign) return; _f_pooling();}
#define _PRINT(x)		{if(x < 3) _uart_printf(_COM_PRINTER, 0, PSTR("  "));}
#define _SLEEP(x)		{if(x == 3) _delay_ms(300);}
#define _SLEEP			_delay_ms(700);
#define _SLEEP_2		_delay_ms(2000);
#define _SLEEP_4		_delay_ms(4000);

char xH = 0x82, yL = 0x5A;
char xL = 0x00, yH = 0x00;

unsigned char rcv_trans[620];
uint16_t	char_count=0;
uint8_t		f_rcv=0,f_odo=0;
char __nopol[15], __odo[15];

unsigned int transLength=0;
char IsInitHeader=0;



const unsigned char	__reprintloc[8][2] PROGMEM = {	{1, 6}, {2, 6}, {3, 6}, {4, 6},
													{1, 13}, {2, 13}, {3, 13}, {4, 13}};
const unsigned char	__idleloc[8] PROGMEM = {4, 5, 6, 7, 8, 9, 10, 11};
const unsigned char	__pumploc[8][2] PROGMEM = {{1, 5}, {2, 5}, {3, 5}, {4, 5},
												{1, 12}, {2, 12}, {3, 12}, {4, 12}};
const unsigned char	__prodloc[6][2] PROGMEM = {{1, 6}, {2, 6}, {1, 13},
												{2, 13}, {1, 20}, {2, 20}};
const unsigned char	__decloc[5][2] PROGMEM = {{1, 9}, {2, 9}, {3, 9}, {4, 9}, {1, 20}};
const unsigned char	__prntloc[5][2] PROGMEM = {{1, 8}, {2, 8}, {3, 8}, {4, 8}, {1, 19}};
const unsigned char	__prntlmt[5] PROGMEM = {5, 3, 4, 2, 15};
const unsigned char	__prntstr[5] PROGMEM = {1, 1, 0, 0, 0};
const unsigned char	__hostloc[3][2] PROGMEM = {{1, 10}, {2, 10}, {3, 10}};
const unsigned char __txttitle[8][10] PROGMEM = {	{"Header1 :"}, {"Header2 :"}, {"Header3 :"},
													{"Header4 :"}, {"Footer1 :"}, {"Footer2 :"},
													{"Footer3 :"}, {"Footer4 :"}};

unsigned char	EEMEM __rec_volume[8][6][15];
unsigned char	EEMEM __rec_amount[8][6][15];

unsigned char	EEMEM __spv_pass[_MAX_PASS + 1] = {"11111"};
unsigned char	EEMEM __sys_pass[_MAX_PASS + 1] = {"00000"};

unsigned char	EEMEM __opp_name[_MAX_OPP + 1] = {"Tukul"};

unsigned char	EEMEM __pump_map[8] = {1, 2, 0, 0, 0, 0, 0, 0};
unsigned char	EEMEM __nozz_map[8][6] = {	{5, 0, 0, 0, 0, 0}, {2, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}};

unsigned char	EEMEM __prt_baud = 1;
unsigned char	EEMEM __prt_scroll = 7;
unsigned char	EEMEM __prt_autocut = 2;
unsigned char	EEMEM __prt_logo = 0;
unsigned char	EEMEM __prt_papper = 3;

unsigned char	EEMEM __conn_host = 0;
unsigned char	EEMEM __date_time = 0;
unsigned char	EEMEM __notif_scr = 1;

unsigned char	EEMEM __dec_vol = 2;
unsigned char	EEMEM __dec_harga = 0;
unsigned char	EEMEM __dec_jumlah = 0;
unsigned char	EEMEM __dec_tvol = 2;
unsigned char	EEMEM __dec_tamount = 0;

unsigned char	EEMEM __prd_price[6][6] = {{"5100"}, {"4800"},
											{"4500"}, {"4500"},
											{"4500"}, {"4500"}};
unsigned char	EEMEM __prd_name[6][_MAX_NAME + 1] = {	{"Pertamax Plus"}, {"Pertamax"},
														{"Premium"}, {"Solar"},
														{"Bio Premium"}, {"Bio Solar"}};
unsigned char	EEMEM __prd_menu[6][8] = {	{"Pert+"}, {"Pert"},
											{"Premium"}, {"Solar"},
											{"BioPrem"}, {"BioSolr"}};

unsigned char	EEMEM __txt_rcpt[10][41];
										/* = {{"PT. Hanindo Automation  Solutions"},
											{"JL. RS. Fatmawati No.55"},
											{"Jakarta 12410"}, {"Ph. (021) 75900909"},
											{"Semoga Anda Puas"},
											{"Terimakasih - Selamat   Jalan"},
											{"\0"}, {"\0"}};
*/

void _scr_splash(void){
	_LCD_CLR;
	_delay_ms(20);
	_lcd_printf(1, 5, PSTR("IFTv5.13"));
	_lcd_printf(2, 5, PSTR("(P)+(C)2009, HAS"));
}


void _scr_idle(void){
	_LCD_CLR;
	_lcd_printf(1, 1, PSTR("IFTv5.13  "));
/*
	_lcd_printf(4, 1, PSTR("00 xxxxxxxx"));
	if(eeprom_read_byte(&__conn_host))
		_lcd_put(4, 2, '1');
	else
		_lcd_put(4, 2, '0');*/
}

void _scr_platno(void){
	_LCD_CLR;
	_lcd_printf(1, 1, PSTR("PLAT NO.:"));
	_lcd_printf(4, 1, PSTR("[*]Batal   [#]Lanjut"));
	_lcd_xy(2, 1);
}

void _scr_odometer(void){
	_LCD_CLR;
	_lcd_print(1, 1, "ODOMETER:");
	_lcd_print(4, 1, "[*]Batal   [#]Lanjut");
	_lcd_xy(2, 1);
}

void _scr_pump(void){
	_LCD_CLR;
	_lcd_print(1, 1, "1)P1 3)P3 5)P5 7)P7");
	_lcd_print(2, 1, "2)P1 4)P4 6)P6 8)P8");
	_lcd_print(4, 1, "*)Cancel");
}

void _scr_reprint(void){
	unsigned char	__i, __status, __x, __y;
	//const unsigned char	__charloc[8][2] PROGMEM = {{1, 6}, {2, 6}, {3, 6}, {4, 6},
	//									{1, 13}, {2, 13}, {3, 13}, {4, 13}};

	_LCD_CLR;
	_lcd_printf(1, 1, PSTR("1)P1:  5)P5:"));
	_lcd_printf(2, 1, PSTR("2)P2:  6)P6:"));
	_lcd_printf(3, 1, PSTR("3)P3:  7)P7:"));
	_lcd_printf(4, 1, PSTR("4)P4:  8)P8:  *)Exit"));

	for(__i = 0; __i < 8; __i++){
		__x = pgm_read_byte(&__reprintloc[__i][0]);//__charloc[__i][0];
		__y = pgm_read_byte(&__reprintloc[__i][1]);//__charloc[__i][1];
		__status = __station[__i].status;
		switch(__status){
			case 0x30:
				_lcd_put(__x, __y, 'i');
				break;
			case 0x31:
				_lcd_put(__x, __y, 'n');
				break;
			case 0x33:
				_lcd_put(__x, __y, 'd');
				break;
			case 0x34:
				_lcd_put(__x, __y, 'e');
				break;
			default:
				_lcd_put(__x, __y, 'x');
				break;
		}
	}
}

void _scr_noti(char* __str1, char* __str2){
	_LCD_CLR;
	
	_lcd_print(2, 5, __str1);
	_lcd_print(3, 5, __str2);
}

void _scr_proses(void){
	_LCD_CLR;
		_lcd_printf(1, 1, PSTR("SEDANG PROSESS......"));
}


void msg98(void){
	
	_uart(1, 1,0x01);
	_uart_printf(1, 0, PSTR("0103192.168.016.070192.168.016.1809802F968CFFB"));
	_uart(1, 1,0x02);
}

void msg98n(unsigned char no){
	
	_uart(1, 1,0x01);
	_uart_printf(1, 0, PSTR("0103192.168.016.070192.168.016.18098"));
	switch(no){
		case 1:_uart_printf(1, 0, PSTR("01"));
		break;
		case 2:_uart_printf(1, 0, PSTR("02"));
		break;
		case 3:_uart_printf(1, 0, PSTR("03"));
		break;
		case 4:_uart_printf(1, 0, PSTR("04"));
		break;
		case 5:_uart_printf(1, 0, PSTR("05"));
		break;
		case 6:_uart_printf(1, 0, PSTR("06"));
		break;
		case 7:_uart_printf(1, 0, PSTR("07"));
		break;
		case 8:_uart_printf(1, 0, PSTR("08"));
		break;
		}
	_uart_printf(1, 0, PSTR("F968CFFB"));
	_uart(1, 1,0x02);
}



char _msg_99_trans[7];
char _msg_99_shift[2];
char _msg_99_date[17];
char _msg_99_des[16];
char _msg_99_is[2];
char _msg_99_id[2];
char _msg_99_hrg[9];
char _msg_99_vol[9];
char _msg_99_tot[11];



//*************************************************************
//subrutine atur data
//
char struc_dat(void){
unsigned char c2=0,f_dp=0;
uint16_t c1;
/*
	_lcd_xy(3,1);
	_lcd(rcv_trans[37]);
	_lcd(rcv_trans[38]);
	_lcd(rcv_trans[39]);
	_lcd(rcv_trans[40]);
	_lcd(rcv_trans[41]);
	_lcd(rcv_trans[42]);
	_lcd(rcv_trans[43]);
*/
//	while(1);
	if ((rcv_trans[35]==0x39) & (rcv_trans[36]==0x39)) {

		for(c1=37;c1<43;c1++) {_msg_99_trans[c2]=rcv_trans[c1];c2++;}
		_msg_99_trans[6]=0x00;
		c2=0;_msg_99_shift[0]=rcv_trans[43];_msg_99_shift[1]=0x00;
		for(c1=254;c1<271;c1++) {_msg_99_date[c2]=rcv_trans[c1];c2++;}
		_msg_99_date[16]=0x00;c2=0;
		for(c1=68;c1<84;c1++) {_msg_99_des[c2]=rcv_trans[c1];c2++;}
		_msg_99_des[11]=0x00;
		_msg_99_is[0]=rcv_trans[63];_msg_99_is[1]=0x00;
		_msg_99_id[0]=rcv_trans[65];_msg_99_id[1]=0x00;
		c2=0;
		for(c1=83;c1<92;c1++) {
		if ((rcv_trans[c1]==0x30) & (f_dp==0)) f_dp=0;
		if ((rcv_trans[c1]!=0x30) & (f_dp==0)) f_dp=1;
		if (f_dp==1) {_msg_99_hrg[c2]=rcv_trans[c1];c2++;}
		if (rcv_trans[c1]=='.') {_msg_99_hrg[c2-1]=0x00;c2=93;}
		}
		c2=0;f_dp=0;
		for(c1=91;c1<100;c1++) {
		if ((rcv_trans[c1]==0x30) & (f_dp==0)) f_dp=0;
		if ((rcv_trans[c1]!=0x30) & (f_dp==0)) f_dp=1;
		if (f_dp==1) {_msg_99_vol[c2]=rcv_trans[c1];c2++;}
		}_msg_99_vol[8]=0x00;
		c2=0;f_dp=0;
		for(c1=99;c1<110;c1++) {
		if ((rcv_trans[c1]==0x30) & (f_dp==0)) f_dp=0;
		if ((rcv_trans[c1]!=0x30) & (f_dp==0)) f_dp=1;
		if (f_dp==1) {_msg_99_tot[c2]=rcv_trans[c1];c2++;}
		if (rcv_trans[c1]=='.') {_msg_99_tot[c2-1]=0x00;c2=93;}
		}
		return 0x01;
	}// if
	if ((rcv_trans[35]==0x30) & (rcv_trans[36]==0x30)) return 0x10;

	return 0x00;
}


//*************************************************************
//subrutin saat idle
//cek sana cek sini
//*************************************************************

unsigned char _menu_idle(void){
	unsigned char	__key, __num, __cnt = 0;
	char	__date[9];
	char	__time[9];
	char	f_ot=0,f_an=0,cntf=0,cntn=0;
	
	while(1){};

	
    //Send Msg10
	if  (IsInitHeader==0){
	      char_count=0;f_rcv=0;
          sendMessage10();
		  while (f_rcv!=1){
		      _lcd_printf(2,1,PSTR("Sending Message 10.."));
	          if (f_rcv==1){ 
		          procMessage11();
               }
	      }
       IsInitHeader=1;
  }


    _scr_idle();

	while(1){
		__key = _key_scan(1);
		__num = _key_btn(__key);
		if (__key ==_KEY_ENTER) {char_count=0;awal();}
		if (__key ==_KEY_TIKET) {char_count=0;_scr_proses();msg98n(1);}
		if (__key ==_KEY_MENU) {char_count=0;_scr_proses();msg98n(2);}
		if (__key ==_KEY_SHIFT) {char_count=0;_scr_proses();msg98n(3);}
		if (__key ==_KEY_CLEAR) {char_count=0;_scr_proses();msg98n(4);}

		
		if ((__num=='0')|(__num=='9')) break;
		if (f_rcv==1) {f_rcv=0;
			if (struc_dat()==0x01) {
			if (f_odo==0) _menu_print("","",1);
			if (f_odo==1) {_menu_print(__nopol, __odo,1);f_odo=0;}
			_scr_idle();}//if struc_dat
			if (struc_dat()==0x10) {
				_lcd_print(1, 1, "Tidak Ada Transaksi ");f_ot=1;cntn=0;}

		}//if f_rcv

			cntf++;
			if(cntf >= 5) {cntf=0;
				cntn++;
				if ((f_ot==1) & (cntn>=4)) {f_ot=0;
				_lcd_printf(1, 1, PSTR("IFTv5.13           "));}

				if (f_an==0) {f_an=1;_lcd_put(4, 1, '>');} else
				{f_an=0;_lcd_put(4, 1, '<');}
			}

		//*************************************************
		// jika seting d/t on munculkan date time
		//*************************************************
		if(eeprom_read_byte(&__date_time)){
			__cnt++;
			if(__cnt >= 100){
				_datetime(0, __date, __time);
				_lcd_print(2, 1, __date);
				__time[5] = '\0';
				_lcd_print(2, 10, __time);
				__cnt = 0;
				_delay_ms(5);
			}//if 250
		}//if eeprom


		_f_pooling();  // tempat komunikasi antar cpu

		_delay_ms(1000);
		_delay_ms(1000);
	}// while
	return __key;
}

//****************************************************************
// menu autorisasi
// untuk masuk dgn password untuk merubah parameter
//****************************************************************

void _menu_authorization(void){
	char __result = 0;

	__result = _menu_password();	//jika kembalinya 36
	if(__result==0x36){
		_SEC_SET;
		_menu_system();
		_SEC_RESET;
	}
	if(__result==0x63){
		_SEC_SET;
		_menu_supervisor();
		_SEC_RESET;
	}
	_SEC_RESET;
}

unsigned char _menu_password(void){
	char	__pad = 0, __ret = 3;
	char	__t_spv_pass[_MAX_PASS + 1];
	char	__t_sys_pass[_MAX_PASS + 1];

	__max_string = _MAX_PASS;	
	eeprom_read_block((void*) &__t_spv_pass, (const void*) &__spv_pass, _MAX_PASS + 1);
	eeprom_read_block((void*) &__t_sys_pass, (const void*) &__sys_pass, _MAX_PASS + 1);


	_LCD_CLR;
//	_lcd_print(3, 1, __t_sys_pass);
	_lcd_printf(1, 1, PSTR("PASSWORD:"));
	_lcd_printf(4, 1, PSTR("[*]Cancel   [#]Enter"));
	_lcd_xy(2, 1);

	while(1){
		__pad = _key_string(2);
		if(!__pad){
			if(strcmp(__buf_string, __t_sys_pass) == 0){
				__ret = 0x36;
				break;
			}
			else if(strcmp(__buf_string, __t_spv_pass) == 0){
				__ret = 0x63;
				break;
			}
			else{
				_lcd_print(2, 1, "                    ");
				_lcd_xy(2, 1);
				__chr_count = 0;
			}
		}
		else if(__pad){
			__ret = 0x00;
			break;
		}
		_f_pooling();
	}
	return __ret;
}

//**********************************************************************
// subrutin 
// untuk merubah parameter Pump
//**********************************************************************

void _menu_system(void){
	char __key;

	while(1){
		_LCD_CLR;
		_lcd_printf(1, 1, PSTR("1)Product  5)Printer"));
		_lcd_printf(2, 1, PSTR("2)Pump     6)Host"));
		_lcd_printf(3, 1, PSTR("3)Dec.     7)User"));
		_lcd_printf(4, 1, PSTR("4)Datetime *)Exit"));

		while(1){
			_GET_POOLING;
			__key = _key_scan(1);
			if(__key != _KEY_ENTER && __key != _KEY_TIKET && __key != _KEY_SHIFT && __key != _KEY_CLEAR && __key != _KEY_NULL){
				if(__key == _KEY_CANCEL)
					return;
				if(__key == _KEY_1){
					_menu_product();
					break;
				}
				if(__key == _KEY_2){
					_menu_pump();
					break;
				}
				if(__key == _KEY_3){
					_menu_dec();
					break;
				}
				if(__key == _KEY_4){
					_menu_datetime();
					break;
				}
				if(__key == _KEY_5){
					_menu_printer();
					break;
				}
				if(__key == _KEY_6){
					_menu_host();
					break;
				}
				if(__key == _KEY_7){
					_menu_user();
					break;
				}
				if(__key == _KEY_MENU){
					_menu_status();
					break;
				}
			}
		}
	}
}

void _menu_product(void){
	char __key, __bttn,  __menu_text[6][8], __tmp[100];
	unsigned char __i = 0;

	for(__i=0; __i<6; __i++)
		eeprom_read_block((void*) &__menu_text[__i], (const void*) &__prd_menu[__i], 8);
	
	while(1){
		_LCD_CLR;
		strcpy(__tmp, "1)");
		strcat(__tmp, __menu_text[0]);
		_lcd_print(1, 1, __tmp);
		strcpy(__tmp, "2)");
		strcat(__tmp, __menu_text[1]);
		_lcd_print(2, 1, __tmp);
		strcpy(__tmp, "3)");
		strcat(__tmp, __menu_text[2]);
		_lcd_print(3, 1, __tmp);
		strcpy(__tmp, "4)");
		strcat(__tmp, __menu_text[3]);
		_lcd_print(4, 1, __tmp);
		strcpy(__tmp, "5)");
		strcat(__tmp, __menu_text[4]);
		_lcd_print(1, 12, __tmp);
		strcpy(__tmp, "6)");
		strcat(__tmp, __menu_text[5]);
		_lcd_print(2, 12, __tmp);
		_lcd_print(4, 12, "*)Exit");

		while(1){
			_GET_POOLING;
			__key = _key_scan(1);
			__bttn = _key_btn(__key) - 0x30;
			if(__key == _KEY_CANCEL)
				return;
			if(__bttn >= 1 && __bttn <= 6){
				_menu_productchange(__bttn -1);
				break;
			}
		}
	}
}

void _menu_productchange(unsigned char __select){
	char __num[2], __tmp[100], __produk[17], __harga[6], __result, __key;

	eeprom_read_block((void*) &__produk, (const void*) &__prd_name[__select], 17);

	while(1){
		eeprom_read_block((void*) &__harga, (const void*) &__prd_price[__select], 6);

		_LCD_CLR;
		__num[0] = __select + 0x31;
		__num[1] = '\0';
		strcpy(__tmp, __num);
		strcat(__tmp, ")");
		strcat(__tmp, __produk);
		_lcd_print(1, 1, __tmp);
		strcpy(__tmp, __harga);
		_lcd_print(2, 1, __tmp);
		_lcd_printf(4, 1, PSTR("[*]Exit    [#]Change"));

		while(1){
			_GET_POOLING;
			__key = _key_scan(1);
			if(__key==_KEY_CANCEL)
				return;
			if(__key==_KEY_ENTER){
				__result = _menu_productinput();
				if(!__result){
					eeprom_write_block((const void*) &__buf_string, (void*) &__prd_price[__select], 6);
					//_spi_tx(0x26, _SPI_SLAVE, "Auth");
					break;
				}
				else
					break;
			}
		}
	}
}

unsigned char _menu_productinput(void){
	char __result;

	__max_string = 5;
	_lcd_printf(4, 1, PSTR("[*]Cancel    [#]Save"));
	while(1){
		_GET_POOLING;
		_lcd_print(2, 1, "                    ");
		_lcd_xy(2, 1);
		__result = _key_string(0);
		if(__result){
			__chr_count = 0;
			__buf_string[0] = '\0';
			return 1;
		}
		else
			return 0;
	}
	return 0;
}

//*******************************************************************
// subrutin memilih ID, Product
//*******************************************************************

void _menu_pump(void){
	char __key;

	while(1){
		_LCD_CLR;
		_lcd_printf(1, 1, PSTR("1)ID"));
		_lcd_printf(2, 1, PSTR("2)Product"));
		_lcd_printf(4, 1, PSTR("*)Exit"));

		while(1){
			_GET_POOLING;
			__key = _key_scan(1);
			if(__key == _KEY_CANCEL)
				return;
			if(__key == _KEY_1){
				_menu_pumpid();
				break;
			}
			if(__key == _KEY_2){
				_menu_pumpprod();
				break;
			}
		}
	}
}

//****************************************************************
//subrutin untuk pump id
// data disimpan di __pump_id[0-7]
//****************************************************************

void _menu_pumpid(void){
	char 			__pump_id[8];
	unsigned char	__i, __x, __y, __key, __num, __buff[5];

	eeprom_read_block((void*) &__pump_id, (const void*) &__pump_map, 8);

	_LCD_CLR;
	_lcd_printf(1, 1, PSTR("1)1:   5)5:"));
	_lcd_printf(2, 1, PSTR("2)2:   6)6:"));
	_lcd_printf(3, 1, PSTR("3)3:   7)7:   #)Save"));
	_lcd_printf(4, 1, PSTR("4)4:   8)8:   *)Exit"));

	for(__i = 0; __i < 8; __i++){
		_f_inttostr(__buff, __pump_id[__i]);
		_f_punctuation(__buff, 0, 2, 0);
		__x = pgm_read_byte(&__pumploc[__i][0]);
		__y = pgm_read_byte(&__pumploc[__i][1]);
		_lcd_print(__x, __y, __buff);
	}

	while(1){
		//_GET_POOLING;
		__key = _key_scan(1);
		__num = _key_btn(__key);
		if(__key == _KEY_CANCEL)
			return;
		if(__key == _KEY_ENTER){
			for(__i = 0; __i < 8; __i++)
				__station[__i].status = 0xFF;
			eeprom_write_block((const void*) &__pump_id, (void*) &__pump_map, 8);
			_delay_ms(15);
			_c_pumpmap();
			return;
		}
		if(__num >= 0x31 && __num <= 0x38){
			if(__pump_id[__num - 0x31] == 16)
				__pump_id[__num - 0x31] = 0;
			else
				__pump_id[__num - 0x31]++;
			__x = pgm_read_byte(&__pumploc[__num - 0x31][0]);
			__y = pgm_read_byte(&__pumploc[__num - 0x31][1]);
			_f_inttostr(__buff, __pump_id[__num - 0x31]);
			_f_punctuation(__buff, 0, 2, 0);
			_lcd_print(__x, __y, __buff);
		}
	}
}

void _menu_pumpprod(void){
	char __key;

	while(1){
		_LCD_CLR;
		_lcd_printf(1, 1,  PSTR("1)P1 3)P3 5)P5 7)P7"));
		_lcd_printf(2, 1,  PSTR("2)P2 4)P4 6)P6 8)P8"));
		_lcd_printf(4, 1,  PSTR("*)Exit"));

		while(1){
			_GET_POOLING;
			__key = _key_scan(1);
			if(	__key == _KEY_1 || __key == _KEY_2 || __key == _KEY_3 ||
				__key == _KEY_4 || __key == _KEY_5 || __key == _KEY_6 ||
				__key == _KEY_7 || __key == _KEY_8){
				_menu_pumpprodinput(_key_btn(__key) - 0x31);
				break;
			}
			if(__key == _KEY_CANCEL)
				return;
		}
	}
}

//**********************************************************************
// subrutin merubah produk
// parameter di simpan di __pump_prod
//**********************************************************************

void _menu_pumpprodinput(unsigned char __select){
	char 			__pump_prod[6];
	unsigned char	__i, __x, __y, __key, __num, __buff[5];
	
	eeprom_read_block((void*) &__pump_prod, (const void*) &__nozz_map[__select], 6);

	_LCD_CLR;
	_lcd_printf(1, 1, PSTR("1)N1:  3)N3:  5)N5:"));
	_lcd_printf(2, 1, PSTR("2)N2:  4)N4:  6)N6:"));
	_lcd_printf(4, 1, PSTR("*)Exit        #)Save"));

	for(__i = 0; __i < 6; __i++){
		__x = pgm_read_byte(&__prodloc[__i][0]);
		__y = pgm_read_byte(&__prodloc[__i][1]);
		_f_inttostr(__buff, __pump_prod[__i]);
		_lcd_print(__x, __y, __buff);
	}

	while(1){
		_GET_POOLING;
		__key = _key_scan(1);
		__num = _key_btn(__key);
		if(__key == _KEY_CANCEL)
			return;
		if(__key == _KEY_ENTER){
			eeprom_write_block((const void*) &__pump_prod, (void*) &__nozz_map[__select], 6);
			return;
		}
		if(__num >= 0x31 && __num <= 0x36){
			if(__pump_prod[__num - 0x31] == 6)
				__pump_prod[__num - 0x31] = 0;
			else
				__pump_prod[__num - 0x31]++;
			__x = pgm_read_byte(&__prodloc[__num - 0x31][0]);
			__y = pgm_read_byte(&__prodloc[__num - 0x31][1]);
			_f_inttostr(__buff, __pump_prod[__num - 0x31]);
			_lcd_print(__x, __y, __buff);
		}
	}
}

//***********************************************************************
// subrutin merubah decimal
//***********************************************************************

void _menu_dec(void){
	char 			__dec[5];
	unsigned char	__i, __x, __y, __key, __num, __buff[5];

	__dec[0] = eeprom_read_byte(&__dec_harga);
	__dec[1] = eeprom_read_byte(&__dec_vol);
	__dec[2] = eeprom_read_byte(&__dec_jumlah);
	__dec[3] = eeprom_read_byte(&__dec_tvol);
	__dec[4] = eeprom_read_byte(&__dec_tamount);

	_LCD_CLR;
	_lcd_printf(1, 1, PSTR("1)Price:   5)T.Rp.:"));
	_lcd_printf(2, 1, PSTR("2)Vol. :"));
	_lcd_printf(3, 1, PSTR("3)Rp.  :   #)Save"));
	_lcd_printf(4, 1, PSTR("4)T.Vol:   *)Exit"));

	for(__i = 0; __i < 5; __i++){
		__x = pgm_read_byte(&__decloc[__i][0]);
		__y = pgm_read_byte(&__decloc[__i][1]);
		_f_inttostr(__buff, __dec[__i]);
		_lcd_print(__x, __y, __buff);
	}

	while(1){
		_GET_POOLING;
		__key = _key_scan(1);
		__num = _key_btn(__key);
		if(__key == _KEY_CANCEL)
			return;
		if(__key == _KEY_ENTER){
			eeprom_write_byte(&__dec_harga, __dec[0]);
			eeprom_write_byte(&__dec_vol, __dec[1]);
			eeprom_write_byte(&__dec_jumlah, __dec[2]);
			eeprom_write_byte(&__dec_tvol, __dec[3]);
			eeprom_write_byte(&__dec_tamount, __dec[4]);
			break;
		}
		if(__num >= 0x31 && __num <= 0x35){
			if(__dec[__num - 0x31] == 3)
				__dec[__num - 0x31] = 0;
			else
				__dec[__num - 0x31]++;
			__x = pgm_read_byte(&__decloc[__num - 0x31][0]);
			__y = pgm_read_byte(&__decloc[__num - 0x31][1]);
			_f_inttostr(__buff, __dec[__num - 0x31]);
			_lcd_print(__x, __y, __buff);
		}
	}
}

void _menu_datetime(void){
	int		__i = 0, __ii = 0;
	char	__key, __chr;
	char	__date[9];
	char	__time[9];
	char	__map[12][4] = {	{1,  8, 0, 3}, {1,  9, 0, 9}, {1, 11, 0, 1}, {1, 12, 0, 9},
								{1, 14, 0, 9}, {1, 15, 0, 9}, {2,  8, 0, 2}, {2,  9, 0, 9},
								{2, 11, 0, 5}, {2, 12, 0, 9}, {2, 14, 0, 5}, {2, 15, 0, 9}};

	_datetime(_DATETIME_READ, __date, __time);
	for(__i = 0, __ii = 0; __i < 6; __i++, __ii++){
		if(__i == 2 || __i == 4)
			__ii++;
		__map[__i][2] = __date[__ii];
	}
	for(__i = 6, __ii = 0; __i < 12; __i++, __ii++){
		if(__i == 8 || __i == 10)
			__ii++;
		__map[__i][2] = __time[__ii];
	}

	_LCD_CLR;
	_lcd_printf(1, 1, PSTR("DATE : "));
	_lcd_printf(2, 1, PSTR("TIME : "));
	_lcd_print(1, 8, __date);
	_lcd_print(2, 8, __time);
	_lcd_printf(4, 1, PSTR("*)Cancel      #)Save"));

	__i = 0;
	_lcd_xy(__map[__i][0], __map[__i][1]);
	CURSOR_SHOW;

	while(1){
		_GET_POOLING;
		__key = _key_scan(1);
		__chr = _key_btn(__key);
		if(__chr >= 0x30 && __chr <= 0x39){
			if(__i == 0 || __i == 2 || __i == 6 || __i == 8 || __i == 10){
				if((__chr - 0x30) <= __map[__i][3])
					goto CETAK;
				else
					goto LEWAT;
			}
			if(__i == 1){
				if((__map[0][2] - 0x30) < __map[0][3])
					goto CETAK;
				if((__map[0][2] - 0x30) >= __map[0][3]){
					if(__chr < 0x32)
						goto CETAK;
					else
						goto LEWAT;
				}
			}
			if(__i == 3){
				if((__map[2][2] - 0x30) < __map[2][3])
					goto CETAK;
				if((__map[2][2] - 0x30) >= __map[2][3]){
					if(__chr < 0x33)
						goto CETAK;
					else
						goto LEWAT;
				}
			}
			if(__i == 7){
				if((__map[6][2] - 0x30) < __map[6][3])
					goto CETAK;
				if((__map[6][2] - 0x30) >= __map[6][3]){
					if(__chr < 0x34)
						goto CETAK;
					else
						goto LEWAT;
				}
			}
CETAK:
			__map[__i][2] = __chr;
			_lcd(__chr);
			__i++;
			if(__i > 11)
				__i = 11;
			_lcd_xy(__map[__i][0], __map[__i][1]);
LEWAT:		;
		}
		if(__key == _KEY_SHIFT){
			__i++;
			if(__i > 11)
				__i = 11;
			_lcd_xy(__map[__i][0], __map[__i][1]);
		}
		if(__key == _KEY_CLEAR){
			__i--;
			if(__i <= 0)
				__i = 0;
			_lcd_xy(__map[__i][0], __map[__i][1]);
		}
		if(__key == _KEY_CANCEL)
			break;
		if(__key == _KEY_ENTER){
			for(__i = 0, __ii = 0; __i < 6; __i++, __ii++){
				if(__i == 2 || __i == 4)
					__ii++;
				__date[__ii] = __map[__i][2];
			}
			for(__i = 6, __ii = 0; __i < 12; __i++, __ii++){
				if(__i == 8 || __i == 10)
					__ii++;
				__time[__ii] = __map[__i][2];
			}
			_datetime(_DATETIME_WRITE, __date, __time);
			break;
		}
	}
	CURSOR_HIDE;
}

//*************************************************************************
//  ngeset printer

void _menu_printer(void){
	char 			__value[5];
	unsigned char	__i, __x, __y, __lmt, __start, __key, __num, __buff[5];

	__value[0] = eeprom_read_byte(&__prt_baud);
	__value[1] = eeprom_read_byte(&__prt_papper);
	__value[2] = eeprom_read_byte(&__prt_logo);
	__value[3] = eeprom_read_byte(&__prt_autocut);
	__value[4] = eeprom_read_byte(&__prt_scroll);

	_LCD_CLR;
	_lcd_printf(1, 1, PSTR("1)Baud:  5)Scroll:"));
	_lcd_printf(2, 1, PSTR("2)Size:"));
	_lcd_printf(3, 1, PSTR("3)Logo:  #)Save"));
	_lcd_printf(4, 1, PSTR("4)Cut :  *)Exit"));

	for(__i = 0; __i < 5; __i++){
		__x = pgm_read_byte(&__prntloc[__i][0]);
		__y = pgm_read_byte(&__prntloc[__i][1]);
		_f_inttostr(__buff, __value[__i]);
		if(__i == 4)
			_f_punctuation(__buff, 0, 2, 0);
		_lcd_print(__x, __y, __buff);
	}

	while(1){
		_GET_POOLING;
		__key = _key_scan(1);
		__num = _key_btn(__key);
		if(__key == _KEY_CANCEL)
			return;
		if(__key==_KEY_ENTER){
			eeprom_write_byte(&__prt_baud, __value[0]);
			eeprom_write_byte(&__prt_papper, __value[1]);
			eeprom_write_byte(&__prt_logo, __value[2]);
			eeprom_write_byte(&__prt_autocut, __value[3]);
			eeprom_write_byte(&__prt_scroll, __value[4]);

			switch(__value[0]){
				case 0:
					break;
				case 1:
					_uart_init(0, 9600);
					break;
				case 2:
					_uart_init(0, 19200);
					break;
				case 3:
					_uart_init(0, 38400);
					break;
				case 4:
					_uart_init(0, 57600);
					break;
				case 5:
					_uart_init(0, 115200);
					break;
			}
			break;
		}
		if(__num >= 0x31 && __num <= 0x35){
			__lmt = pgm_read_byte(&__prntlmt[__num - 0x31]);
			__start = pgm_read_byte(&__prntstr[__num - 0x31]);
			if(__value[__num - 0x31] == __lmt)
				__value[__num - 0x31] = __start;
			else
				__value[__num - 0x31]++;
			__x = pgm_read_byte(&__prntloc[__num - 0x31][0]);
			__y = pgm_read_byte(&__prntloc[__num - 0x31][1]);
			_f_inttostr(__buff, __value[__num - 0x31]);
			if(__num == 0x35)
				_f_punctuation(__buff, 0, 2, 0);
			_lcd_print(__x, __y, __buff);
		}
	}
}

void _menu_host(void){
	char 			__value[3];
	unsigned char	__i, __x, __y, __key, __num, __buff[5];

	__value[0] = eeprom_read_byte(&__conn_host);
	__value[1] = eeprom_read_byte(&__date_time);
	__value[2] = eeprom_read_byte(&__notif_scr);

	_LCD_CLR;
	_lcd_printf(1, 1, PSTR("1)Host  :"));
	_lcd_printf(2, 1, PSTR("2)D/T   :"));
	_lcd_printf(3, 1, PSTR("3)Notif :"));
	_lcd_printf(4, 1, PSTR("*)Exit        #)Save"));

	for(__i = 0; __i < 3; __i++){
		__x = pgm_read_byte(&__hostloc[__i][0]);
		__y = pgm_read_byte(&__hostloc[__i][1]);
		_f_inttostr(__buff, __value[__i]);
		_lcd_print(__x, __y, __buff);
	}

	while(1){
		_GET_POOLING;
		__key = _key_scan(1);
		__num = _key_btn(__key);
		if(__key == _KEY_CANCEL)
			return;
		if(__key==_KEY_ENTER){
			eeprom_write_byte(&__conn_host, __value[0]);
			eeprom_write_byte(&__date_time, __value[1]);
			eeprom_write_byte(&__notif_scr, __value[2]);
			break;
		}
		if(__num >= 0x31 && __num <= 0x33){
			if(__value[__num - 0x31] == 1)
				__value[__num - 0x31] = 0;
			else
				__value[__num - 0x31]++;
			__x = pgm_read_byte(&__hostloc[__num - 0x31][0]);
			__y = pgm_read_byte(&__hostloc[__num - 0x31][1]);
			_f_inttostr(__buff, __value[__num - 0x31]);
			_lcd_print(__x, __y, __buff);
		}
	}
}

void _menu_user(void){
	unsigned char	__key;
	unsigned char	__t_spv_pass[_MAX_PASS + 1];
	unsigned char	__t_sys_pass[_MAX_PASS + 1];

	while(1){
		eeprom_read_block((void*) &__t_spv_pass, (const void*) &__spv_pass, _MAX_PASS + 1);
		eeprom_read_block((void*) &__t_sys_pass, (const void*) &__sys_pass, _MAX_PASS + 1);
		
		_LCD_CLR;
		_lcd_printf(1, 1, PSTR("1)L1:"));
		_lcd_printf(2, 1, PSTR("2)L2:"));
		_lcd_print(1, 6, __t_spv_pass);
		_lcd_print(2, 6, __t_sys_pass);
		_lcd_printf(4, 1, PSTR("*)Exit"));

		while(1){
			_GET_POOLING;
			__key = _key_scan(1);
			if(__key == _KEY_CANCEL)
				return;
			if(__key == _KEY_1){
				_menu_setpassword(_SPV_PASS);
				break;
			}
			if(__key == _KEY_2){
				_menu_setpassword(_SYS_PASS);
				break;
			}
		}
	}
}

void _menu_setpassword(unsigned char __level){
	unsigned char	__result = 0;
	unsigned char	__buff[5];

	__max_string = _MAX_PASS;
	while(1){
		_LCD_CLR;
		_lcd_printf(1, 1, PSTR("PASSWORD (MAX   ):"));
		_f_inttostr(__buff, _MAX_PASS);
		_f_punctuation(__buff, 0, 2, 0);
		_lcd_print(1, 15, __buff);
		_lcd_printf(4, 1, PSTR("*)Exit        #)Save"));
		_lcd_xy(2, 1);

		_GET_POOLING;
		__result = _key_string(2);
		if(__result)
			return;
		else
			switch(__level){
				case _SYS_PASS:
					eeprom_write_block((const void*) &__buf_string, (void*) &__sys_pass, _MAX_PASS + 1);
					return;
					break;
				case _SPV_PASS:
					eeprom_write_block((const void*) &__buf_string, (void*) &__spv_pass, _MAX_PASS + 1);
					return;
					break;
			}
	}
}

void _menu_supervisor(void){
	char __key;

	while(1){
		_LCD_CLR;
		_lcd_printf(1, 1, PSTR("1)Admin"));
		_lcd_printf(2, 1, PSTR("2)Settings"));
		_lcd_printf(4, 1, PSTR("*)Exit"));
		
		while(1){
			_GET_POOLING;
			__key = _key_scan(1);
			if(__key == _KEY_CANCEL)
				return;
			if(__key == _KEY_1){
				_menu_admin();
				break;
			}
			if(__key == _KEY_2){
				_menu_settings();
				break;
			}
		}
	}
}

void _menu_admin(void){
	char __key, __num,  __x, __chr, __result;
	char __t_opp_name[_MAX_OPP + 1];
	char __addr[2] = {"0\0"};
	unsigned char __i;

	while(1){
		eeprom_read_block((void*) &__t_opp_name, (const void*) &__opp_name, _MAX_OPP + 1);

		_LCD_CLR;
		_lcd_printf(1, 1, PSTR("1)Close Shift"));
		_lcd_printf(2, 1, PSTR("2)Re-Print"));
		_lcd_printf(3, 1, PSTR("3)User("));
		_lcd_printf(4, 1, PSTR("*)Exit"));
		for(__i = 0, __x = 8; __x < 20; __i++, __x++){
			if(__x == 20){
				_lcd_put(3, __x, ')');
				break;
			}
			else{
				__chr = __t_opp_name[__i];
				if(__chr == 0x20 || __chr == '\0'){
					_lcd_put(3, __x, ')');
					break;
				}
				else
					_lcd_put(3, __x, __chr);
			}
		}
		
		while(1){
			_GET_POOLING;
			__key = _key_scan(1);
			if(__key == _KEY_CANCEL)
				return;
			if(__key == _KEY_1){
				_f_totalizer();
				break;
			}
			if(__key == _KEY_2){
				while(1){
					_scr_reprint();
					while(1){
						__result = _key_scan(1);
						__num = _key_btn(__result) - 0x31;
						if(__num >= 0 && __num <= 7){
							//_menu_print("", "", __num + 1);
							__addr[0] = eeprom_read_byte(&__pump_map[__num]) + 0x3F;
							_spi_tx(_LAST_TRANSACT, _SPI_SLAVE, __addr);
							_delay_ms(600);
							_f_recdata();
							break;
						}
						if(__result == _KEY_CANCEL)
							break;
					}
					if(__result == _KEY_CANCEL)
						break;
				}
				break;
			}
			if(__key == _KEY_3){
				_menu_operator();
				break;
			}
		}
	}
}

void _menu_operator(void){
	char __result;

	__max_string = _MAX_OPP;
	__chr_count = 0;
	__buf_string[0] = '\0';

	_LCD_CLR;
	_lcd_printf(1, 1, PSTR("OPERATOR :"));
	_lcd_printf(4, 1, PSTR("*)Cancel      #)Save"));
	_lcd_xy(2, 1);
	__result = _key_string(1);
	if(__result==0)
		eeprom_write_block((const void*) &__buf_string, (void*) &__opp_name, _MAX_OPP + 1);
}

void _menu_settings(void){
	char __key;

	while(1){
		_LCD_CLR;
		_lcd_printf(1, 1, PSTR("1)Header"));
		_lcd_printf(2, 1, PSTR("2)Footer"));
		_lcd_printf(3, 1, PSTR("3)Password"));
		_lcd_printf(4, 1, PSTR("*)Exit"));
		
		while(1){
			_GET_POOLING;
			__key = _key_scan(1);
			if(__key == _KEY_CANCEL)
				return;
			if(__key == _KEY_1){
				_menu_header();
				break;
			}
			if(__key == _KEY_2){
				_menu_footer();
				break;
			}
			if(__key == _KEY_3){
				_menu_setpassword(_SPV_PASS);
				break;
			}
		}
	}
}

void _menu_header(void){
	char __key;

	while(1){
		_LCD_CLR;
		_lcd_printf(1, 1, PSTR("1)Header1  3)Header3"));
		_lcd_printf(2, 1, PSTR("2)Header2  4)Header4"));
		_lcd_printf(4, 1, PSTR("*)Exit"));
		
		while(1){
			_GET_POOLING;
			__key = _key_scan(1);
			if(__key == _KEY_CANCEL)
				return;
			if(__key == _KEY_1){
				_menu_text(0);
				break;
			}
			if(__key == _KEY_2){
				_menu_text(1);
				break;
			}
			if(__key == _KEY_3){
				_menu_text(2);
				break;
			}
			if(__key == _KEY_4){
				_menu_text(3);
				break;
			}
		}
	}
}

void _menu_footer(void){
	char __key;

	while(1){
		_LCD_CLR;
		_lcd_printf(1, 1, PSTR("1)Footer1  3)Footer3"));
		_lcd_printf(2, 1, PSTR("2)Footer2  4)Footer4"));
		_lcd_printf(4, 1, PSTR("*)Exit"));
		
		while(1){
			_GET_POOLING;
			__key = _key_scan(1);
			if(__key == _KEY_CANCEL)
				return;
			if(__key == _KEY_1){
				_menu_text(4);
				break;
			}
			if(__key == _KEY_2){
				_menu_text(5);
				break;
			}
			if(__key == _KEY_3){
				_menu_text(6);
				break;
			}
			if(__key == _KEY_4){
				_menu_text(7);
				break;
			}
		}
	}
}

void _menu_text(unsigned char __select){
	unsigned char	__result;
	unsigned char	__title[10];

	__max_string = _MAX_TEXT;
	__chr_count = 0;
	__buf_string[0] = '\0';

	for(unsigned char __i = 0; __i < 10; __i ++)
		__title[__i] = pgm_read_byte(&__txttitle[__select][__i]);

	_LCD_CLR;
	_lcd_print(1, 1, __title);
	_lcd_printf(4, 1, PSTR("*)Exit        #)Save"));
	_lcd_xy(2, 1);
	__result = _key_string(1);
	if(__result==0)
		eeprom_write_block((const void*) &__buf_string, (void*) &__txt_rcpt[__select], _MAX_TEXT + 1);
}

//************************************************************************
//buat print  platno dan odo ************************************** 
//
//************************************************************************

void _menu_tiket(void){
	char __result, __caps, __num;


	__max_string = 11;
	__chr_count = 0;
	__buf_string[0] = '\0';
	__caps = __caps_lock;
	__caps_lock = 1;

	_scr_platno();
	__result = _key_string(1);
	if(__result==0)
		strcpy(__nopol, __buf_string);
	else
		return;
	__caps_lock = __caps;

	__max_string = 10;
	__chr_count = 0;
	__buf_string[0] = '\0';

	_scr_odometer();
	__result = _key_string(0);
	if(__result==0)
		strcpy(__odo, __buf_string); 
	else
		return;

	_scr_pump();
	while(1){
		__result = _key_scan(1);
		__num = _key_btn(__result) - 0x30;
		if(__num >= 1 && __num <= 8){
				f_odo=1;
				_scr_proses();msg98n(__num);//*********************** odo
			break;
		}
		if(__result == _KEY_CANCEL){
			break;
		}
	}
}



//***********************************************************************
//cetak bon
//parameter  platno, odo, tombol
//***********************************************************************

void _menu_print(char* __platno, char* __odo,unsigned char __bttn){


	char __num, __i, __papper, __prd, __date[15], __time[10];
	char __t_txt_rcpt[_MAX_TEXT + 1],v_pjg,t_tem[5],v1_pjg;
	char t1_tem[5],t2_tem[5],v2_pjg;
	uint8_t  c1,c_pjg;

//	prt[0]=0x1b;prt[1]=0x21;prt[2]=0x10;prt[3]=0x00;
//	_uart_print(_COM_PRINTER, 1,prt );

	__papper = eeprom_read_byte(&__prt_papper);

	//header 1
	eeprom_read_block((void*) &__t_txt_rcpt, (const void*) &__txt_rcpt[0], _MAX_TEXT + 1);
	if(__t_txt_rcpt[0] != '\0'){
		_uart_print(_COM_PRINTER, 1, __t_txt_rcpt);
		_SLEEP;
	}

	//header 2
	eeprom_read_block((void*) &__t_txt_rcpt, (const void*) &__txt_rcpt[1], _MAX_TEXT + 1);
	if(__t_txt_rcpt[0] != '\0'){
		_uart_print(_COM_PRINTER, 1, __t_txt_rcpt);
		_SLEEP;
	}

	//header 3
	eeprom_read_block((void*) &__t_txt_rcpt, (const void*) &__txt_rcpt[2], _MAX_TEXT + 1);
	if(__t_txt_rcpt[0] != '\0'){
		_uart_print(_COM_PRINTER, 1, __t_txt_rcpt);
		_SLEEP;
		_SLEEP;
		_SLEEP;
	}
	//header 4
	eeprom_read_block((void*) &__t_txt_rcpt, (const void*) &__txt_rcpt[3], _MAX_TEXT + 1);
	if(__t_txt_rcpt[0] != '\0'){
		_uart_print(_COM_PRINTER, 1, __t_txt_rcpt);
		_SLEEP;
		_SLEEP;
		_SLEEP;
	}

	_RETURN;
	_SLEEP;
	_SLEEP;


	_uart_printf(_COM_PRINTER, 0, PSTR("Shift: "));
	_uart_print(_COM_PRINTER, 0,_msg_99_shift);
	_uart_printf(_COM_PRINTER, 0, PSTR(" "));
	_uart_printf(_COM_PRINTER, 0, PSTR("No.Trans:"));
	_SLEEP_4;
	_SLEEP_4;
	_SLEEP_4;
	_uart_print(_COM_PRINTER, 1,_msg_99_trans);

	_SLEEP_2;
	_uart_printf(_COM_PRINTER, 0, PSTR("Waktu: "));
	_uart_print(_COM_PRINTER, 1,_msg_99_date);

	_uart_printf(_COM_PRINTER, 1, PSTR("------------------------"));
	_SLEEP_2;


	_uart_printf(_COM_PRINTER, 0,     PSTR("Pompa     : ["));
	_uart_print(_COM_PRINTER, 0,_msg_99_is);

	_uart_print(_COM_PRINTER, 0, "]-");
	_uart_print(_COM_PRINTER, 1,_msg_99_id);
	_SLEEP;
	_SLEEP;
	_SLEEP;
	
	eeprom_read_block((void*) &__t_txt_rcpt, (const void*) &__prd_name[__prd - 1], _MAX_NAME + 1);
	_uart_printf(_COM_PRINTER, 0,     PSTR("Produk    : "));
	_uart_print(_COM_PRINTER, 1,_msg_99_des);

	_SLEEP_2;
	_SLEEP;
	_SLEEP;

	_uart_printf(_COM_PRINTER, 0,     PSTR("Harga/L   : Rp."));
	_SLEEP_2;
//	_uart_print(_COM_PRINTER, 1,_msg_99_hrg);
//===============================================================
//harga
//
	t2_tem[0]=0x00;t1_tem[0]=0x00;
	v_pjg=strlen(_msg_99_hrg);
	v1_pjg=v_pjg-3;c1=0;
	for (c_pjg=v1_pjg;c_pjg<=v_pjg;c_pjg++) {
			t_tem[c1]=_msg_99_hrg[c_pjg];c1++;}
		t_tem[c1]=0x00;

	if (v1_pjg>3) {
	v1_pjg=v_pjg-3;c1=0;
	for (c_pjg=v1_pjg;c_pjg<=v_pjg;c_pjg++) {
			t1_tem[c1]=_msg_99_hrg[c_pjg];c1++;}
		t1_tem[c1]=0x00;
		}//if v1_pjg
	if (v1_pjg<=3) {c1=0;
	for (c_pjg=0;c_pjg<v1_pjg;c_pjg++) {
			t1_tem[c1]=_msg_99_hrg[c_pjg];c1++;}
			t1_tem[c1]=0x00;
		_uart_print(_COM_PRINTER, 0,t1_tem);
	_uart_printf(_COM_PRINTER, 0, PSTR(","));
		_uart_print(_COM_PRINTER, 1,t_tem);
		}//
	_SLEEP_2;

	_uart_printf(_COM_PRINTER, 0,     PSTR("Jml Liter : "));
	_uart_print(_COM_PRINTER, 0,_msg_99_vol);
	_uart_printf(_COM_PRINTER, 0, PSTR(" L"));
	_RETURN;
	_SLEEP;
	_SLEEP;
	_SLEEP;

	_uart_printf(_COM_PRINTER, 0,     PSTR("Jml Rupiah: Rp."));
	t2_tem[0]=0x00;t1_tem[0]=0x00;
//	strcpy(_msg_99_tot,"1223000");
	v_pjg=strlen(_msg_99_tot);
	v1_pjg=v_pjg-3;c1=0;
	for (c_pjg=v1_pjg;c_pjg<=v_pjg;c_pjg++) {
			t_tem[c1]=_msg_99_tot[c_pjg];c1++;}
		t_tem[c1]=0x00;

	if (v1_pjg>3) {
	v2_pjg=v1_pjg;v1_pjg=v1_pjg-3;c1=0;
	for (c_pjg=v1_pjg;c_pjg<v2_pjg;c_pjg++) {
			t2_tem[c1]=_msg_99_tot[c_pjg];c1++;}
		t2_tem[c1]=0x00;
		}//if v1_pjg
	if (v1_pjg<=3) {c1=0;
	for (c_pjg=0;c_pjg<v1_pjg;c_pjg++) {
			t1_tem[c1]=_msg_99_tot[c_pjg];c1++;}
			t1_tem[c1]=0x00;
		}//
	_uart_print(_COM_PRINTER, 0,t1_tem);
	_uart_printf(_COM_PRINTER, 0, PSTR(","));
	c1=strlen(t2_tem);
	if (c1>0) {_uart_print(_COM_PRINTER, 0,t2_tem);
	_uart_printf(_COM_PRINTER, 0, PSTR(","));}
	_uart_print(_COM_PRINTER, 1,t_tem);

	_SLEEP_2;

	if(strlen(__platno) > 0){
		_uart_printf(_COM_PRINTER, 0, PSTR("No.Polisi : "));
		_uart_print(_COM_PRINTER, 1, __platno);
		_SLEEP;
	}

	if(strlen(__odo) > 0){
		_uart_printf(_COM_PRINTER, 0, PSTR("Odometer  : "));
		_uart_print(_COM_PRINTER, 1, __odo);
		_SLEEP;
	}

	_uart_printf(_COM_PRINTER, 1, PSTR("------------------------"));
	_RETURN;
	_SLEEP_4;


/*
	eeprom_read_block((void*) &__t_txt_rcpt, (const void*) &__opp_name, _MAX_OPP + 1);
	if(__t_txt_rcpt[0] != '\0'){
		_uart_printf(_COM_PRINTER, 0, PSTR("Operator : "));
		_uart_print(_COM_PRINTER, 1, __t_txt_rcpt);
		_RETURN;
		_SLEEP;
	}
*/
	eeprom_read_block((void*) &__t_txt_rcpt, (const void*) &__txt_rcpt[4], _MAX_TEXT + 1);
	if(__t_txt_rcpt[0] != '\0'){
		_uart_print(_COM_PRINTER, 1, __t_txt_rcpt);
		_SLEEP;
	}
	eeprom_read_block((void*) &__t_txt_rcpt, (const void*) &__txt_rcpt[5], _MAX_TEXT + 1);
	if(__t_txt_rcpt[0] != '\0'){
		_uart_print(_COM_PRINTER, 1, __t_txt_rcpt);
		_SLEEP;
	}
	eeprom_read_block((void*) &__t_txt_rcpt, (const void*) &__txt_rcpt[6], _MAX_TEXT + 1);
	if(__t_txt_rcpt[0] != '\0'){
		_uart_print(_COM_PRINTER, 1, __t_txt_rcpt);
		_SLEEP;
	}
	eeprom_read_block((void*) &__t_txt_rcpt, (const void*) &__txt_rcpt[7], _MAX_TEXT + 1);
	if(__t_txt_rcpt[0] != '\0'){
		_uart_print(_COM_PRINTER, 1, __t_txt_rcpt);
		_SLEEP;
	}

	__num = eeprom_read_byte(&__prt_scroll);
	for(__i = 0; __i < __num; __i++)
		_RETURN;

	__num = eeprom_read_byte(&__prt_autocut);
	if(__num  == 1)
		_PARTIAL_CUT;
	if(__num  == 2)
		_TOTAL_CUT;
}


//***************************************************************
//ngeprint untuk status
//
void _menu_status(void){
	unsigned char	__i, __ii, __num, __papper;
	char	__date[9];
	char	__time[9];

	_SPACING;
	__papper = eeprom_read_byte(&__prt_papper);

	_BOLD_FONT;
	_uart_printf(_COM_PRINTER, 1, PSTR("HANINDO IFT STAND ALONE "));
	_REGULAR_FONT;
	_RETURN;
	_SLEEP(__papper);
	
	_uart_printf(_COM_PRINTER, 1, PSTR("FIRMWARE      : v5.13"));
	_RETURN;
	_SLEEP(__papper);

	for(__i = 0; __i < 8; __i++){
		__num = eeprom_read_byte(&__pump_map[__i]);
		if(__num != 0){
			_uart_printf(_COM_PRINTER, 0, PSTR("PUMP "));
			_uart(_COM_PRINTER, 1, __i + 0x31);
			_uart_printf(_COM_PRINTER, 0, PSTR(" ADDRESS: 4"));
			_uart(_COM_PRINTER, 1, __num + 0x2F);
			_RETURN;
		}
	}

	_RETURN;
	_SLEEP(__papper);

	for(__i = 0; __i < 8; __i++){
		for(__ii = 0; __ii < 6; __ii++){
			__num = eeprom_read_byte(&__nozz_map[__i][__ii]);
			if(__num != 0){
				_uart_printf(_COM_PRINTER, 0, PSTR("P"));
				_uart(_COM_PRINTER, 1, __i + 0x31);
				_uart_print(_COM_PRINTER, 0, ".");
				_uart(_COM_PRINTER, 1, __ii + 0x31);
				_uart_printf(_COM_PRINTER, 0, PSTR(" PRODUCT  : "));
				_uart(_COM_PRINTER, 1, __num + 0x30);
				_RETURN;
			}
		}
	}

	_RETURN;
	_SLEEP(__papper);

	_uart_printf(_COM_PRINTER, 0, PSTR("DECIMAL PRICE : "));
	_uart(_COM_PRINTER, 1, eeprom_read_byte(&__dec_harga) + 0x30);
	_RETURN;
	_SLEEP(__papper);

	_uart_printf(_COM_PRINTER, 0, PSTR("DECIMAL VOLUME: "));
	_uart(_COM_PRINTER, 1, eeprom_read_byte(&__dec_vol) + 0x30);
	_RETURN;
	_SLEEP(__papper);

	_uart_printf(_COM_PRINTER, 0, PSTR("DECIMAL AMOUNT: "));
	_uart(_COM_PRINTER, 1, eeprom_read_byte(&__dec_jumlah) + 0x30);
	_RETURN;
	_SLEEP(__papper);

	_uart_printf(_COM_PRINTER, 0, PSTR("DECIMAL T.VOL : "));
	_uart(_COM_PRINTER, 1, eeprom_read_byte(&__dec_tvol) + 0x30);
	_RETURN;

	_uart_printf(_COM_PRINTER, 0, PSTR("DECIMAL T.AMNT: "));
	_uart(_COM_PRINTER, 1, eeprom_read_byte(&__dec_tamount) + 0x30);
	_RETURN;

	_RETURN;
	_SLEEP(__papper);

	_datetime(0, __date, __time);
	_uart_printf(_COM_PRINTER, 0, PSTR("DATE SET      : "));
	_uart_print(_COM_PRINTER, 1, __date);
	_uart_printf(_COM_PRINTER, 0, PSTR("TIME SET      : "));
	_uart_print(_COM_PRINTER, 1, __time);
	_RETURN;
	_SLEEP(__papper);

	_uart_printf(_COM_PRINTER, 0, PSTR("BAUD RATE     : "));
	_uart(_COM_PRINTER, 1, eeprom_read_byte(&__prt_baud) + 0x30);
	_RETURN;
	_SLEEP(__papper);

	_uart_printf(_COM_PRINTER, 0, PSTR("PAPPER SIZE   : "));
	_uart(_COM_PRINTER, 1, eeprom_read_byte(&__prt_papper) + 0x30);
	_RETURN;
	_SLEEP(__papper);

	_uart_printf(_COM_PRINTER, 0, PSTR("LOGO NUMBER   : "));
	_uart(_COM_PRINTER, 1, eeprom_read_byte(&__prt_logo) + 0x30);
	_RETURN;
	_SLEEP(__papper);

	_uart_printf(_COM_PRINTER, 0, PSTR("AUTOCUT MODE  : "));
	_uart(_COM_PRINTER, 1, eeprom_read_byte(&__prt_autocut) + 0x30);
	_RETURN;
	_SLEEP(__papper);

	_uart_printf(_COM_PRINTER, 0, PSTR("SCROLL NUMBER : "));
	_uart(_COM_PRINTER, 1, eeprom_read_byte(&__prt_scroll) + 0x30);
	_RETURN;

	_RETURN;
	_SLEEP(__papper);

	_uart_printf(_COM_PRINTER, 0, PSTR("HOST STATUS   : "));
	_uart(_COM_PRINTER, 1, eeprom_read_byte(&__conn_host) + 0x30);
	_RETURN;
	_SLEEP(__papper);

	__num = eeprom_read_byte(&__prt_scroll);
	for(__i = 0; __i < __num; __i++)
		_RETURN;

	__num = eeprom_read_byte(&__prt_autocut);
	if(__num  == 1)
		_PARTIAL_CUT;
	if(__num  == 2)
		_TOTAL_CUT;
}

void _f_totalizer(void){
	char	__pump[8];
	char	__date[11];
	char	__time[9];
	char	__buf[512];
	char	__tmp[100];
	char	__addr[2] = {"0\0"};
	char	__t_txt_rcpt[_MAX_TEXT + 1];
	char	__t_cvolume[15], __t_camount[15];
	char	__t_ovolume[15], __t_oamount[15];
	char	__t_tvolume[15], __t_tamount[15];
	char	__t_volume[15], __t_amount[15];

	unsigned char	__i, __ii, __iii;
	unsigned char	__count, __chr;
	unsigned char	__pumpnum;
	unsigned char	__papper, __len = 13;
	unsigned char	__num, __flag;
	unsigned long	__volume, __amount;

//char	__dummy[2][50] = {	{"aaa651111  00022222229999999000000020000000090000000002222222999999900000002000000009000000"},
//							{"aaa6511    0000200000000900000000001545000006952500"}};

	__num = eeprom_read_byte(&__prt_logo);
	if(__num > 0){
		_uart(_COM_PRINTER, 1, 0x1C); _uart(_COM_PRINTER, 1, 0x70);
		_uart(_COM_PRINTER, 1, __num); _uart(_COM_PRINTER, 1, 0);
		_SLEEP;
		_RETURN;
		_SLEEP;
	}

	eeprom_read_block((void*) &__t_txt_rcpt, (const void*) &__txt_rcpt[0], _MAX_TEXT + 1);
	if(__t_txt_rcpt[0] != '\0'){
		_uart_print(_COM_PRINTER, 1, __t_txt_rcpt);
		_SLEEP;
	}
	eeprom_read_block((void*) &__t_txt_rcpt, (const void*) &__txt_rcpt[1], _MAX_TEXT + 1);
	if(__t_txt_rcpt[0] != '\0'){
		_uart_print(_COM_PRINTER, 1, __t_txt_rcpt);
		_SLEEP;
	}
	eeprom_read_block((void*) &__t_txt_rcpt, (const void*) &__txt_rcpt[2], _MAX_TEXT + 1);
	if(__t_txt_rcpt[0] != '\0'){
		_uart_print(_COM_PRINTER, 1, __t_txt_rcpt);
		_SLEEP;
	}
	eeprom_read_block((void*) &__t_txt_rcpt, (const void*) &__txt_rcpt[3], _MAX_TEXT + 1);
	if(__t_txt_rcpt[0] != '\0'){
		_uart_print(_COM_PRINTER, 1, __t_txt_rcpt);
		_SLEEP;
	}

	_RETURN;
	_SLEEP;

	_uart_printf(_COM_PRINTER, 1, PSTR("LAPORAN SHIFT"));
	_RETURN;
	_SLEEP;

	_datetime(0, __date, __time);
	_uart_print(_COM_PRINTER, 0, __date);
	_uart_printf(_COM_PRINTER, 0, PSTR(" "));
	_uart_print(_COM_PRINTER, 1, __time);
	_SLEEP;

	strcpy(__buf, "<record>\r");
	strcat(__buf, __date);
	strcat(__buf, " ");
	strcat(__buf, __time);
	strcat(__buf, "\r");

	eeprom_read_block((void*) &__pump, (const void*) &__pump_map, 8);

	__volume = 0;
	__amount = 0;
	for(__pumpnum = 0; __pumpnum < 8; __pumpnum++){
		if(__pump[__pumpnum] != 0){
			__addr[0] = __pump[__pumpnum] + 0x3F;

			_spi_tx(_TOTALIZE, _SPI_SLAVE, __addr);
			_delay_ms(100);

			for(__i = 0; __i < 8; __ii++){
				__num = _spi_rx(_SPI_SLAVE);
				if(!__num)
					break;
				_delay_ms(15);
			}
			if(__num)
				continue;
			//strcpy(__package, __dummy[__pumpnum]);

			__count = 0;
			for(__i = 5; __i < 11; __i++){
				if(__package[__i] == 0x20){
					__t_cvolume[0] = '\0';
					__t_camount[0] = '\0';
				}
				else{
					__t_ovolume[0] = '\0';
					__t_oamount[0] = '\0';
					__t_ovolume[1] = '\0';
					__t_oamount[1] = '\0';

					eeprom_read_block((void*) &__t_ovolume, (const void*) &__rec_volume[__pumpnum][__i - 5], 15);
					eeprom_read_block((void*) &__t_oamount, (const void*) &__rec_amount[__pumpnum][__i - 5], 15);

					__flag =0;
					for(__ii = 0, __iii = (__count * 20) + 11; __iii < (__count * 20) + 21; __iii++){
						__chr = __package[__iii];
						if(!__flag && __chr != 0x30)
							__flag = 1;
						if(__flag){
							__t_cvolume[__ii++] = __chr;
							__t_cvolume[__ii + 1] = '\0';
						}
					}

					__flag = 0;
					for(__ii = 0, __iii = (__count * 20) + 21; __iii < (__count * 20) + 31; __iii++){
						__chr = __package[__iii];
						if(!__flag && __chr != 0x30)
							__flag = 1;
						if(__flag){
							__t_camount[__ii++] = __chr;
							__t_camount[__ii + 1] = '\0';
						}
					}

					eeprom_write_block((const void*) &__t_cvolume, (void*) &__rec_volume[__pumpnum][__i - 5], 15);
					eeprom_write_block((const void*) &__t_camount, (void*) &__rec_amount[__pumpnum][__i - 5], 15);

					__num = eeprom_read_byte(&__nozz_map[__pumpnum][__i - 5]);
					if(__num == 0)
						continue;

					_uart(_COM_PRINTER, 1, 218);
					for(__ii = 0; __ii < 22; __ii++)
						_uart(_COM_PRINTER, 1, 196);
					_uart(_COM_PRINTER, 1, 191);
					_RETURN;
					_SLEEP;

					eeprom_read_block((void*) &__t_txt_rcpt, (const void*) &__prd_name[__num - 1], _MAX_NAME + 1);
					strcpy(__tmp, "");
					_f_chrcat(__tmp, 179);
					_f_strcat(__tmp, "P");
					_f_chrcat(__tmp, __pumpnum + 0x31);
					_f_strcat(__tmp, ".");
					_f_chrcat(__tmp, __count + 0x31);
					_f_strcat(__tmp, " - ");
					_f_strcat(__tmp, __t_txt_rcpt);

					__num = strlen(__tmp);
					for(__ii = 0; __ii < 22 - (__num - 1); __ii++)
						_f_chrcat(__tmp, 0x20);
					_f_chrcat(__tmp, 179);
					_uart_print(_COM_PRINTER, 1, __tmp);
					_SLEEP;

					_uart(_COM_PRINTER, 1, 195);
					for(__ii = 0; __ii < 22; __ii++)
						_uart(_COM_PRINTER, 1, 196);
					_uart(_COM_PRINTER, 1, 180);
					_RETURN;
					_SLEEP;

					__volume += _f_strtoint(__t_cvolume) - _f_strtoint(__t_ovolume);
					__amount += _f_strtoint(__t_camount) - _f_strtoint(__t_oamount);
					_f_inttostr(__t_tvolume, _f_strtoint(__t_cvolume) - _f_strtoint(__t_ovolume));
					_f_inttostr(__t_tamount, _f_strtoint(__t_camount) - _f_strtoint(__t_oamount));

					strcat(__buf, "P");
					__addr[0] = __pumpnum + 0x31;
					strcat(__buf, __addr);
					strcat(__buf, ".");
					__addr[0] = __count + 0x31;
					strcat(__buf, __addr);
					strcat(__buf, " ");
					strcat(__buf, __t_txt_rcpt);
					strcat(__buf, " ");
					strcat(__buf, __t_tvolume);
					strcat(__buf, " ");
					strcat(__buf, __t_tamount);
					strcat(__buf, "\r");

					_f_punctuation(__t_cvolume, 2, __len, eeprom_read_byte(&__dec_tvol));
					_f_punctuation(__t_ovolume, 2, __len, eeprom_read_byte(&__dec_tvol));
					_f_punctuation(__t_tvolume, 2, __len, eeprom_read_byte(&__dec_tvol));
					_f_punctuation(__t_camount, 2, __len, eeprom_read_byte(&__dec_tamount));
					_f_punctuation(__t_oamount, 2, __len, eeprom_read_byte(&__dec_tamount));
					_f_punctuation(__t_tamount, 2, __len, eeprom_read_byte(&__dec_tamount));

					_uart(_COM_PRINTER, 1, 179);
					_uart_printf(_COM_PRINTER, 0, PSTR("Volume                "));
					_uart(_COM_PRINTER, 1, 179);
					_RETURN;
					_SLEEP;

					_uart(_COM_PRINTER, 1, 179);
					_uart_printf(_COM_PRINTER, 0, PSTR("Tutup    "));
					_uart_print(_COM_PRINTER, 0, __t_cvolume);
					_uart(_COM_PRINTER, 1, 179);
					_RETURN;
					_SLEEP;

					_uart(_COM_PRINTER, 1, 179);
					_uart_printf(_COM_PRINTER, 0, PSTR("Buka     "));
					_uart_print(_COM_PRINTER, 0, __t_ovolume);
					_uart(_COM_PRINTER, 1, 179);
					_RETURN;
					_SLEEP;

					_uart(_COM_PRINTER, 1, 179);
					_uart_printf(_COM_PRINTER, 0, PSTR("         -------------"));
					_uart(_COM_PRINTER, 1, 179);
					_RETURN;
					_SLEEP;

					_uart(_COM_PRINTER, 1, 179);
					_BOLD_FONT;
					_uart_printf(_COM_PRINTER, 0, PSTR("         "));
					_uart_print(_COM_PRINTER, 0, __t_tvolume);
					_REGULAR_FONT;
					_uart(_COM_PRINTER, 1, 179);
					_RETURN;
					_SLEEP;

					_uart(_COM_PRINTER, 1, 179);
					_uart_printf(_COM_PRINTER, 0, PSTR("Amount                "));
					_uart(_COM_PRINTER, 1, 179);
					_RETURN;
					_SLEEP;

					_uart(_COM_PRINTER, 1, 179);
					_uart_printf(_COM_PRINTER, 0, PSTR("Tutup    "));
					_uart_print(_COM_PRINTER, 0, __t_camount);
					_uart(_COM_PRINTER, 1, 179);
					_RETURN;
					_SLEEP;

					_uart(_COM_PRINTER, 1, 179);
					_uart_printf(_COM_PRINTER, 0, PSTR("Buka     "));
					_uart_print(_COM_PRINTER, 0, __t_oamount);
					_uart(_COM_PRINTER, 1, 179);
					_RETURN;
					_SLEEP;

					_uart(_COM_PRINTER, 1, 179);
					_uart_printf(_COM_PRINTER, 0, PSTR("         -------------"));
					_uart(_COM_PRINTER, 1, 179);
					_RETURN;
					_SLEEP;

					_uart(_COM_PRINTER, 1, 179);
					_BOLD_FONT;
					_uart_printf(_COM_PRINTER, 0, PSTR("         "));
					_uart_print(_COM_PRINTER, 0, __t_tamount);
					_REGULAR_FONT;
					_uart(_COM_PRINTER, 1, 179);
					_RETURN;
					_SLEEP;

					_uart(_COM_PRINTER, 1, 192);
					for(__ii = 0; __ii < 22; __ii++)
						_uart(_COM_PRINTER, 1, 196);
					_uart(_COM_PRINTER, 1, 217);
					_RETURN;
					_SLEEP;

					__count++;
				}
			}
		}
	}

	_uart(_COM_PRINTER, 1, 218);
	for(__ii = 0; __ii < 22; __ii++)
		_uart(_COM_PRINTER, 1, 196);
	_uart(_COM_PRINTER, 1, 191);
	_RETURN;
	_SLEEP;

	_uart(_COM_PRINTER, 1, 179);
	_uart_printf(_COM_PRINTER, 0, PSTR("Total                 "));
	_uart(_COM_PRINTER, 1, 179);
	_RETURN;
	_SLEEP;

	_uart(_COM_PRINTER, 1, 195);
	for(__ii = 0; __ii < 22; __ii++)
		_uart(_COM_PRINTER, 1, 196);
	_uart(_COM_PRINTER, 1, 180);
	_RETURN;
	_SLEEP;

	_f_inttostr(__t_volume, __volume);
	strcat(__buf, "Total ");
	strcat(__buf, __t_volume);
	strcat(__buf, " ");
	_f_punctuation(__t_volume, 2, __len, eeprom_read_byte(&__dec_tvol));

	_uart(_COM_PRINTER, 1, 179);
	_uart_printf(_COM_PRINTER, 0, PSTR("Volume   "));
	_BOLD_FONT;
	_uart_print(_COM_PRINTER, 0, __t_volume);
	_REGULAR_FONT;
	_uart(_COM_PRINTER, 1, 179);
	_RETURN;
	_SLEEP;

	_f_inttostr(__t_amount, __amount);
	strcat(__buf, __t_amount);
	strcat(__buf, "\r");
	_f_punctuation(__t_amount, 2, __len, eeprom_read_byte(&__dec_tamount));

	_uart(_COM_PRINTER, 1, 179);
	_uart_printf(_COM_PRINTER, 0, PSTR("Amount   "));
	_BOLD_FONT;
	_uart_print(_COM_PRINTER, 0, __t_amount);
	_REGULAR_FONT;
	_uart(_COM_PRINTER, 1, 179);
	_RETURN;
	_SLEEP;

	_uart(_COM_PRINTER, 1, 192);
	for(__ii = 0; __ii < 22; __ii++)
		_uart(_COM_PRINTER, 1, 196);
	_uart(_COM_PRINTER, 1, 217);
	_RETURN;
	_SLEEP;

	__num = eeprom_read_byte(&__prt_scroll);
	for(__ii = 0; __ii < __num; __ii++)
		_RETURN;

	__num = eeprom_read_byte(&__prt_autocut);
	if(__num  == 1)
		_PARTIAL_CUT;
	if(__num  == 2)
		_TOTAL_CUT;

	strcat(__buf, "</record>\r\r~");
	if(eeprom_read_byte(&__conn_host))
		_f_record("TOTAL.TXT", __buf);
}

//*****************************************************************
//tempat ngambil data dari slave
//

void _f_recdata(void){
	char	__buff[150];
	char	__pump[2] = {"0"};
	char	__nozzle[2] = {"0"};
	char	__product[2] = {"0"};
	char	__prd;
	char	__date[9];
	char	__time[9];
	unsigned char __i, __ii, __start, __chr,__sto;

	__sto = _f_getbutton(__package[0]) - 1;
	if(__command_set == _PUMP_STATUS){
		__station[__sto].status = __package[2];
		__station[__sto].nozzle = __package[1];
	}

	if(__command_set == _LAST_TRANSACT){
		__prd = eeprom_read_byte(&__nozz_map[__sto][__package[1] - 0x31]);

		__station[__sto].status = __package[2];//0x34;
		__station[__sto].nozzle = __package[1];//__nozz;
		__station[__sto].product = __prd;

		_datetime(0, __date, __time);
		strcpy(__station[__sto].date, __date);
		strcpy(__station[__sto].time, __time);

		__start = 0;
		__ii = 0;
		for(__i = 3; __i < 9; __i++){
			__chr = __package[__i];
			if(__chr != 0x30 && !__start)
				__start = 1;
			if(__start){
				__station[__sto].volume[__ii++] = __chr;
				__station[__sto].volume[__ii] = '\0';
			}
		}
		__start = 0;
		__ii = 0;
		for(__i = 9; __i < 13; __i++){
			__chr = __package[__i];
			if(__chr != 0x30 && !__start)
				__start = 1;
			if(__start){
				__station[__sto].price[__ii++] = __chr;
				__station[__sto].price[__ii] = '\0';
			}
		}
		__start = 0;
		__ii = 0;
		for(__i = 13; __i < 19; __i++){
			__chr = __package[__i];
			if(__chr!=0x30 && !__start)
				__start = 1;
			if(__start){
				__station[__sto].total[__ii++] = __chr;
				__station[__sto].total[__ii] = '\0';
			}
		}

		if(__transact == 9999)
			__transact = 1;
		else
			__transact++;
		_f_inttostr(__buff, __transact);
		strcpy(__station[__sto].transact, __buff);

		__pump[0] = __sto + 0x31;
		__nozzle[0] = __station[__sto].nozzle;
		__product[0] = __station[__sto].product + 0x30;

		strcpy(__buff, __station[__sto].date);
		strcat(__buff, " ");
		strcat(__buff, __station[__sto].time);
		strcat(__buff, " ");
		strcat(__buff, __station[__sto].transact);
		strcat(__buff, " ");
		strcat(__buff, __pump);
		strcat(__buff, " ");
		strcat(__buff, __nozzle);
		strcat(__buff, " ");
		strcat(__buff, __product);
		strcat(__buff, " ");
		strcat(__buff, __station[__sto].volume);
		strcat(__buff, " ");
		strcat(__buff, __station[__sto].price);
		strcat(__buff, " ");
		strcat(__buff, __station[__sto].total);
		strcat(__buff, "\r~");

		if(eeprom_read_byte(&__conn_host))
			_f_record("TRANS.TXT", __buff);

		_f_punctuation(__station[__sto].transact, 0, 4, 0);
		_f_punctuation(__station[__sto].total, 2, 7, eeprom_read_byte(&__dec_jumlah));
		_f_punctuation(__station[__sto].price, 2, 7, eeprom_read_byte(&__dec_harga));
		_f_punctuation(__station[__sto].volume, 1, 0, eeprom_read_byte(&__dec_vol));

		__pump_idle[__sto] = 1;
		__idle_count[__sto] = 0;
	}
}

void _f_record(char* __filename, char* __data){
	unsigned char	__i;
	unsigned char	__file[13];

	for(__i=0; __i<13; __i++)
		__file[__i] = 0x00;
	strcpy(__file, __filename);
	_mmc_write(__file, __data);
}

//**************************************************************************
//subrutin pengambilan data antar cpu							************
//
//**************************************************************************
void _f_pooling(void){
//	if (_spi_rx(_SPI_SLAVE)==0) _lcd_print(2, 1, "LOST");
//	if (_spi_rx(_SPI_SLAVE)==1) _lcd_print(2, 1, "IDLE");
//		_f_recdata();
}

void _f_inttostr(char* __string, unsigned long __value){
	char			__flag = 0, __count;
	unsigned long	__num, __devider = 1000000000;
	int				__tmp;
	unsigned char  __i = 0;

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

void _f_punctuation(char* __string, unsigned char __mode, unsigned char __length, unsigned char __decimal){
	char __buff[15] = {"000000000000000"};
	char  __point = 0,  __len;
	unsigned char __i, __ii,__counter = 0;

	__len = strlen(__string);
	if(__mode == 0){
		__buff[__length] = '\0';
		for(__i = __length, __ii = __len - 1; __i > (__length - __len); __i--, __ii--)
			__buff[__i - 1] = __string[__ii];
		strcpy(__string, __buff);
	}
	else{
		if(__decimal > 0){
			for(__i = 0; __i < __decimal; __i++){
				if(__i < __len)
					__buff[__counter++] = __string[__len - 1 - __i];
				else
					__buff[__counter++] = '0';
			}
			__buff[__counter++] = ',';
		}
		if(__decimal > 0 && __len <= 2)
			__buff[__counter++] = '0';
		else{
			if(__len > 1){
				for(	__i = 0, __ii = __len - 1 - __decimal; __i < __len - __decimal;
						__i++, __point++, __ii--){
					if(__point == 3){
						__point = 0;
						__buff[__counter++] = '.';
					}
					__buff[__counter++] = __string[__ii];
				}
			}
			else
				__buff[__counter++] = '0';
		}
		if(__mode == 1){
			for(__i = 0; __i < __counter; __i++)
				__string[__i] = __buff[__counter - 1 - __i];
			__string[__counter] = '\0';
		}
		if(__mode == 2){
			for(__i = 0; __i < __length; __i++)
				__string[__i] = 0x20;
			for(__i = 0, __ii = __length - 1; __i < __counter; __i++, __ii--)
				__string[__ii] = __buff[__i];
			__string[__length] = '\0';
		}
	}
}

unsigned char _f_getbutton(unsigned char __addr){
	char __button = 0;
	char __pump[8];

	eeprom_read_block((void*) &__pump, (const void*) &__pump_map, 8);
	
	while(1)
		if(__pump[__button++] == __addr - 0x3F)
			break;

	return __button;
}

unsigned long _f_strtoint(char* __string){
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

void tester(void){
	char __cut[2][3] = {{0x1B, 0x6D, '\0'}, {0x1B, 0x69, '\0'}};

	_uart(_COM_PRINTER, 1, 0x1B); _uart(_COM_PRINTER, 1, 0xFA);
	_uart(_COM_PRINTER, 1, 2);
	_uart(_COM_PRINTER, 1, xL); _uart(_COM_PRINTER, 1, xH);
	_uart(_COM_PRINTER, 1, yH); _uart(_COM_PRINTER, 1, yL);
	_RETURN;
	_RETURN;
	_RETURN;
	_RETURN;
	_RETURN;
	_RETURN;
	_uart_print(_COM_PRINTER, 0, __cut[1]);
}
unsigned char _f_getbaudrate(void){
	return eeprom_read_byte(&__prt_baud);
}

//**********************************************************
// ngirim ke slave pump yang active

void _c_pumpmap(void){
	unsigned char __buff[8];

	eeprom_read_block((void*) &__buff, (const void*) &__pump_map, 8);
	_spi_txnum(_PUMP_DEF, _SPI_SLAVE, __buff, 8);
}

void _f_strcat(char* __text, char* __string){
	unsigned char	__i;

	__i = strlen(__text);

	while(*__string){
		__text[__i++] = *__string++;
	}
	__text[__i] = '\0';
}

void _f_chrcat(char* __text, char __char){
	unsigned char	__i;

	__i = strlen(__text);

	__text[__i++] = __char;
	__text[__i] = '\0';
}

void _f_totfirst(void){
	char	__pump[8];
	char	__date[9];
//	char	__time[9];
//	char	__buf[512];
	char	__addr[2] = {"0\0"};
	char	__t_txt_rcpt[_MAX_TEXT + 1];
	char	__t_cvolume[15], __t_camount[15];
	char	__t_tvolume[15], __t_tamount[15];
	char	__t_volume[15], __t_amount[15];

	unsigned char	__i, __ii, __iii;
	unsigned char	__count, __chr;
	unsigned char	__pumpnum;
//	unsigned char	__papper, __len = 10;
	unsigned char	__num, __flag;

	eeprom_read_block((void*) &__pump, (const void*) &__pump_map, 8);

	for(__pumpnum = 0; __pumpnum < 8; __pumpnum++){
		if(__pump[__pumpnum] != 0){
			__addr[0] = __pump[__pumpnum] + 0x3F;

			_spi_tx(_TOTALIZE, _SPI_SLAVE, __addr);
			_delay_ms(100);

			for(__i = 0; __i < 8; __ii++){
				__num = _spi_rx(_SPI_SLAVE);
				if(!__num)
					break;
				_delay_ms(15);
			}
			if(__num)
				continue;

			__count = 0;
			for(__i = 5; __i < 11; __i++){
				if(__package[__i] == 0x20){
					__t_cvolume[0] = '\0';
					__t_camount[0] = '\0';
				}
				else{
					__flag =0;
					for(__ii = 0, __iii = (__count * 20) + 11; __iii < (__count * 20) + 21; __iii++){
						__chr = __package[__iii];
						if(!__flag && __chr != 0x30)
							__flag = 1;
						if(__flag){
							__t_cvolume[__ii++] = __chr;
							__t_cvolume[__ii + 1] = '\0';
						}
					}

					__flag = 0;
					for(__ii = 0, __iii = (__count * 20) + 21; __iii < (__count * 20) + 31; __iii++){
						__chr = __package[__iii];
						if(!__flag && __chr != 0x30)
							__flag = 1;
						if(__flag){
							__t_camount[__ii++] = __chr;
							__t_camount[__ii + 1] = '\0';
						}
					}

					__count++;
				}
			}
		}
	}
}

unsigned long _f_strtolong(char* __string, char __nibble){
	unsigned char	__i, __ii, __len, __dec = 8;
	char	__buff[10] = {"00000000"};

	__len = strlen(__string);

	for(__i = __len, __ii = (__dec * 2) - 1; __i > 0; __i--, __ii--){
		if(__nibble){
			if(__ii <= __dec - 1)
				__buff[__ii] = __string[__i - 1];
		}
		else{
			if(__ii > __dec - 1)
				__buff[__ii - __dec] = __string[__i - 1];
		}
	}

	return	_f_strtoint(__buff);
}

ISR(USART1_RX_vect){
	char			__chr;
	__chr = UDR1;
	rcv_trans[char_count]=__chr;
//	_lcd(__chr);
	char_count++;
	if (__chr==0x02) {f_rcv=1; transLength=char_count;char_count=0;}
    

}

void system_stop(){
     char __key,__num; 
     while(1){
//	   __key = _key_scan(1);
//	   __num = _key_btn(__key);
//		if ((__num>='0')&&(__num<='9')) break;

	 };
}

void showString(char *strDisp){
     static char i=1;
	 char __key,__num;
	 char lcdteks[20];
	 //char IsExit=0;
	 _LCD_CLR;
	 //_lcd_print(i,1,strDisp);
	 //_delay_ms(2000);
	 //i++;
	 
     for (i=0;i<strlen(strDisp);i++){
	   _lcd_xy((1+(i/20)),(1+(i%20)));
	   _lcd(strDisp[i]);
	   }
     sprintf(lcdteks,"Length:%d",strlen(strDisp));
     _lcd_print(3,1,lcdteks);
	 system_stop();



     /*
	 while(1){
		__key = _key_scan(1);
		__num = _key_btn(__key);
		if ((__num>='0')&&(__num<='9')) break;
	    
	 }
	 */
}

/*Subrutine Msg10*/

void sendMessage10(){
     _uart(1, 1,0x01);
	 _uart_printf(1, 0, PSTR("0103192.168.016.070192.168.016.18010F968CFFB"));
	 _uart(1, 1,0x02);
}



void clearString(char *str){
     int i;
	 for(i=0;i<strlen(str);i++){
	    str[i]=0;
	 }
}

void procMessage11(){
     unsigned int i;
     char buffHeader[41];
	 char lcdteks[40];
     clearString(buffHeader);


     if (f_rcv==1){
	     f_rcv=0;
         for (i=1;i<=strlen(transLength);i++){
		     //IFT ID
			 //if ((i>=2)&&(i<4)){}
		     //Header1
			 if ((i>=63)&&(i<103)){
			    buffHeader[i-63]=rcv_trans[i-1];			 
				if (i==102){
				   buffHeader[40]="\0";
				   eeprom_write_block((const void*) &buffHeader, (void*) &__txt_rcpt[0], 40);
				   }
			 }
		     //Header2
			 if ((i>=103)&&(i<143)){
			    buffHeader[i-103]=rcv_trans[i-1];
				if (i==142){
				   buffHeader[40]="\0";
				   showString(buffHeader);
				   eeprom_write_block((const void*) &buffHeader, (void*) &__txt_rcpt[1], 40);                   
				   }
			 }
		     //Header3
			 if ((i>=143)&&(i<183)){
			    buffHeader[i-143]=rcv_trans[i-1];
				if (i==182){
				   buffHeader[40]="\0";
				   //showString(buffHeader);
                   eeprom_write_block((const void*) &buffHeader, (void*) &__txt_rcpt[2], 40);
				   }
			 }

		     //Header4
			 if ((i>=183)&&(i<223)){
			    buffHeader[i-183]=rcv_trans[i-1];
				 
				if (i==222){
				   buffHeader[40]="\0";
				   while (1){};

				   showString(buffHeader);
                   eeprom_write_block((const void*) &buffHeader, (void*) &__txt_rcpt[3], 40);
				   }
			 }
		     //Header5
			 if ((i>=223)&&(i<263)){
			    buffHeader[i-223]=rcv_trans[i-1];
				if (i==262){
				   buffHeader[40]="\0";
				   showString(buffHeader);
                   eeprom_write_block((const void*) &buffHeader, (void*) &__txt_rcpt[4], 40);
 				   }
			 }
		     //Header6
			 if ((i>=263)&&(i<303)){
			    buffHeader[i-263]=rcv_trans[i-1];
				if (i==302){
				   buffHeader[40]="\0";
				   showString(buffHeader);
                   eeprom_write_block((const void*) &buffHeader, (void*) &__txt_rcpt[5], 40);
				   }
			 }
		     //Footer1
			 if ((i>=303)&&(i<343)){
			    buffHeader[i-303]=rcv_trans[i-1];
				if (i==342){
				   buffHeader[40]="\0";
				   showString(buffHeader);
                   eeprom_write_block((const void*) &buffHeader, (void*) &__txt_rcpt[6], 40);
				   }
			 }
		     //Footer2
			 if ((i>=343)&&(i<383)){
			    buffHeader[i-343]=rcv_trans[i-1];
				if (i==382){
				   buffHeader[40]="\0";
				   showString(buffHeader);
                   eeprom_write_block((const void*) &buffHeader, (void*) &__txt_rcpt[7], 40);
				   }
			 }
		     //Footer3
			 if ((i>=383)&&(i<423)){
			    buffHeader[i-383]=rcv_trans[i-1];
				if (i==422){
				   buffHeader[40]="\0";
 				   showString(buffHeader);
                   eeprom_write_block((const void*) &buffHeader, (void*) &__txt_rcpt[8], 40);
				   }
			 }
			 //Footer4
			 if ((i>=423)&&(i<463)){
			    buffHeader[i-423]=rcv_trans[i-1];
				if (i==462){
				   buffHeader[40]="\0";
				   showString(buffHeader);
                   eeprom_write_block((const void*) &buffHeader, (void*) &__txt_rcpt[9], 40);
				   }
			 }
		 
		 }//endFor 
	 }//endif
}

