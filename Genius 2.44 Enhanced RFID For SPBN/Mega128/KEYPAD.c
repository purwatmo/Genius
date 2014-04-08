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
    
	Interfacing 4x4 Keypad
	Controller	:	ATmega128 (8 Mhz internal)
	Compiler	:	AVR-GCC
	Version		:	1.1
	Author		:	Yofe Fegeanto, Jakarta (Indonesia)
					www.fegeanto.com
	Date		:	24 November 2009
*/

#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <compat/deprecated.h>
#include <string.h>

#include "keypad.h"
#include "LCD.h"
#include "SPI.h"

// ---------------------------------------------------------------------------------------------------------
// Flag Control
// ---------------------------------------------------------------------------------------------------------
/*
char __hit_count = 0;		// Counter penekanan Keypad untuk tombol yang sama
char  __chr_count = 0;		// Counter jumlah karakter yang disimpan
char __buf_string[20];		// Buffer untuk menyimpan string sebanyak maksimal 80 karakter
char __max_string = 0;		// Jumlah maksimum karakter pada string untuk satu proses
char __caps_lock = 0;		// Counter penekanan Keypad untuk tombol yang sama
char __key_shift = 0;
char __key_shfcnt = 0;
char __key_sfflck = 0;
char __key_light = 0;
char __key_lgtcnt = 0;
char __key_rollback = 0;
char __key_rbkcnt = 0;
char __key_parse = 0;
char __key_sign = 0;
char __pad_buffer = 0;
char __pad_loop = 0;
*/
char __lock_num = 0;
char __key_light = 0;
char __key_lgtcnt = 0;
char __pad_loop = 0;
char __key_shift = 0;
char __key_shfcnt = 0;
 int TimMenuTimeout=0;



const unsigned char __alphanum[2][10][6] PROGMEM = {{{'0', ' ','.', '\0'}, 
                                                     {'1', '.', ',', '?', '!', '\0'},
											         {'2', 'a', 'b', 'c', '\0'}, 
													 {'3', 'd', 'e', 'f', '\0'},
											         {'4', 'g', 'h', 'i', '\0'}, 
										             {'5', 'j', 'k', 'l', '\0'},
											         {'6', 'm', 'n', 'o', '\0'}, 
											         {'7', 'p', 'q', 'r', 's', '\0'},
											         {'8', 't', 'u', 'v', '\0'}, 
											         {'9', 'w', 'x', 'y', 'z', '\0'}},

											{        {'0', ' ','.', '\0'}, 
											         {'1', '&', '@', '+', '-', '\0'},
											         {'2', 'A', 'B', 'C', '\0'}, 
													 {'3', 'D', 'E', 'F', '\0'},
											         {'4', 'G', 'H', 'I', '\0'}, 
													 {'5', 'J', 'K', 'L', '\0'},
											         {'6', 'M', 'N', 'O', '\0'}, 
													 {'7', 'P', 'Q', 'R', 'S', '\0'},
											         {'8', 'T', 'U', 'V', '\0'}, 
													 {'9', 'W', 'X', 'Y', 'Z', '\0'}}};
const unsigned char __maxchar[10] PROGMEM = {2, 4, 3, 3, 3, 3, 3, 4, 3, 4};

char _key_hit(void){
	unsigned char	i, keyhit=0x00;
	unsigned char	outmap[4] = {0xef, 0xdf, 0xbf, 0x7f};

	for(i=0; i<4; i++){
		_delay_ms(1);
		_KEY_PORT = outmap[i];
		_delay_ms(1);
		keyhit = _KEY_MATRIX;
		if (keyhit!=outmap[i]) 
		    break;
	}
	return keyhit;
}

char _key_crr(char __key){
	if(	__key != _KEY_1 && __key != _KEY_2 &&
		__key != _KEY_3 && __key != _KEY_4 &&
		__key != _KEY_5 && __key != _KEY_6 &&
		__key != _KEY_7 && __key != _KEY_8 &&
		__key != _KEY_9 && __key != _KEY_0 &&
		__key != _KEY_CANCEL && __key != _KEY_ENTER &&
		__key != _KEY_TIKET && __key != _KEY_MENU &&
		__key != _KEY_SHIFT && __key != _KEY_CLEAR)
		return _KEY_NULL;
	return __key;
}

char _key_btn(char __key){
	char __keychar = 0;
	
	switch (__key){
		case 0xEE:
			__keychar = 0x31; //1
			break;
		case 0xDE:
			__keychar = 0x32; //2
			break;
		case 0xBE:
			__keychar = 0x33; //3
			break;
		case 0xED:
			__keychar = 0x34; //4
			break;
		case 0xDD:
			__keychar = 0x35; //5
			break;
		case 0xBD:
			__keychar = 0x36; //6
			break;
		case 0xEB:
			__keychar = 0x37; //7
			break;
		case 0xDB:
			__keychar = 0x38; //8
			break;
		case 0xBB:
			__keychar = 0x39; //9
			break;
		case 0xD7:
			__keychar = 0x30; //0
			break;
		case 0xE7:
			__keychar = 0x2a; //*
			break;
		case 0xB7:
			__keychar = 0x23; //#
			break;
		case 0x7E:
			__keychar = 0xF1; //Menu 1
			break;
		case 0x7D:
			__keychar = 0xF2; //Menu 2
			break;
		case 0x7B:
			__keychar = 0xF3; //Menu 3
			break;
		case 0x77:
			__keychar = 0xF4; //Menu 4
			break;
		default:
			__keychar = 0x00;
			break;

	}
	if (__keychar != 0x00)
	    TimMenuTimeout=0;

	return __keychar;
}





char _key_scan(char __select){
	char	__key = 0;

	if(__select==_PAD_SINGLE || __select==_PAD_MULTI){
		__key = _key_hit();
		__key = _key_crr(__key);

		if(__key!=_KEY_NULL){
			_LIGHT_SET;
			if(__sec_par)
				_SEC_ON;

			if(_IS1st){
				if(	__key == _KEY_CANCEL || __key == _KEY_ENTER)
					if(__lock_num)
						__lock_num = 0;
				_PAD_2nd;
				_delay_ms(5);
				return __key;
			}
			if(_IS2nd){
				if(__select==_PAD_MULTI){
					if(	__key==_KEY_0 || __key==_KEY_1 || __key==_KEY_2 || __key==_KEY_3||
						__key==_KEY_4 || __key==_KEY_5 || __key==_KEY_6 ||
						__key==_KEY_7 || __key==_KEY_8 || __key==_KEY_9){
						if(__lock_num){
							_SHIFT_SET;
							CURSOR_HIDE;
						}
						else
							__lock_num = 1;
					}
				}
				do{
					__key = _key_hit();
					__key = _key_crr(__key);
				}while(__key!=_KEY_NULL);
				_PAD_1st;
				_delay_ms(5);
				return _KEY_NULL;
			}
		}
	}
	_PAD_1st;
	return __key;
}

char _table_alphanum(char __caps, char __key, char __hit){
unsigned char __retchar = 0;

	switch(__key){
		case 0xEE:
			__key = 1; //1
			break;
		case 0xDE:
			__key = 2; //2
			break;
		case 0xBE:
			__key = 3; //3
			break;
		case 0xED:
			__key = 4; //4
			break;
		case 0xDD:
			__key = 5; //5
			break;
		case 0xBD:
			__key = 6; //6
			break;
		case 0xEB:
			__key = 7; //7
			break;
		case 0xDB:
			__key = 8; //8
			break;
		case 0xBB:
			__key = 9; //9
			break;
		case 0xD7:
			__key = 0; //0
			break;
		default:
			__key = 0;
			break;	
	}

	if(__hit <= pgm_read_byte(&__maxchar[__key]))
		__retchar = pgm_read_byte(&__alphanum[__caps][__key][__hit]);
	else __retchar = '\0';

	return __retchar;
}


