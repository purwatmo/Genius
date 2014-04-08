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
char __buf_anum = 0;		// Buffer untuk menyimpan nilai terakhir penekanan tombol keypad
char __hit_count = 0;		// Counter penekanan Keypad untuk tombol yang sama

char  __chr_count = 0;		// Counter jumlah karakter yang disimpan
char __buf_string[20];		// Buffer untuk menyimpan string sebanyak maksimal 80 karakter

char __max_string = 0;		// Jumlah maksimum karakter pada string untuk satu proses

char __caps_lock = 0;		// Counter penekanan Keypad untuk tombol yang sama
char __lock_num = 0;

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
		if (keyhit!=outmap[i]) break;
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
	return __keychar;
}

void _light_ticker(void){
	if(__key_light == 1){
		__key_lgtcnt++;
		if(__key_lgtcnt == 150){
		   __key_light = 0; __key_lgtcnt = 0; sbi(PORTG, 1);
		   }
	}
}

void _shift_ticker(void){
	if(_ISSHIFT){
        _SHIFT_TICK;
		if((_COUNTSHIFT & (__chr_count < __max_string)))
			_SHIFT_RST;
	}
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
				//_spi_tx(0x24, _SPI_SLAVE, "~");
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

char _key_string(char __mode){
	unsigned char	 __chr = 0, __i = 0;
	uint8_t  __key = 0;

	if(__mode==0){
		CURSOR_SHOW;
		while(1){
			__key = _key_scan(_PAD_SINGLE);
			if(__key==_KEY_NULL || __key==_KEY_TIKET || __key==_KEY_MENU || __key==_KEY_SHIFT);
			else if(__key==_KEY_CANCEL){
				__chr_count = 0;
				CURSOR_HIDE;
				_SHIFT_OFF;//
				return 1;
			}
			else if(__key==_KEY_ENTER){
				__chr_count = 0;
				CURSOR_HIDE;
				_SHIFT_OFF;//
				return 0;
			}
			else if(__key==_KEY_CLEAR){
				if(__chr_count>0){
					_LCD_LEFT;
					_lcd(0x10);
					_LCD_LEFT;
					__buf_string[__chr_count-1] = '\0';
					__chr_count--;
				}
			}
			else{
				if(__chr_count<__max_string){
					_lcd(_table_alphanum(0, __key, 0));
					__buf_string[__chr_count] = _table_alphanum(0, __key, 0);
					__buf_string[__chr_count + 1] = '\0';
					__chr_count++;
				}
			}
		}
	}
	if(__mode==_STRING_ALPHANUM){
		while(1){
		   CURSOR_SHOW; 
			__key = _key_scan(_PAD_MULTI);
			if(__key == _KEY_NULL || __key == _KEY_TIKET || __key == _KEY_MENU);
			else if(__key == _KEY_CANCEL){
				CURSOR_HIDE;
				_SHIFT_OFF;
				return 1;
			}
			else if(__key == _KEY_ENTER){
				CURSOR_HIDE;
				_SHIFT_OFF;
				return 0;
			}
			else if(__key == _KEY_SHIFT){
				__caps_lock = __caps_lock ^ 1;
			}
			else if(__key == _KEY_CLEAR){
				if(__chr_count > 0){
				if(!__key_shift){
					_LCD_LEFT;
					_lcd(0x10);
					_LCD_LEFT;
					__buf_string[__chr_count - 1] = '\0';
					__chr_count--;
					if(__chr_count == 19){
						lcd_put(2, 20, 0x10);
						lcd_xy(2, 20);
					}
					_SHIFT_OFF;
					CURSOR_SHOW;
					//_SHIFT_FREE;
				}
				}
			}
			else{
				if(__chr_count < __max_string){
					if(__key != __buf_anum){
						__buf_anum = __key;
						__hit_count = 0;
						if(__key_shift == 1 && __buf_anum != 0){
							_LCD_RIGHT;
							__chr_count++;
							if(__chr_count == 20)
								lcd_xy(3, 1);
						}
					}
					else{
						__hit_count++;
					}
					__chr = _table_alphanum(__caps_lock, __key, __hit_count);
					if(__chr == '\0'){
						__hit_count = 0;
						__chr = _table_alphanum(__caps_lock, __key, __hit_count);
					}
					if(__chr_count < __max_string){
						_lcd(__chr);
						_LCD_LEFT;
						__buf_string[__chr_count] = __chr;
						__buf_string[__chr_count + 1] = '\0';
					}
				}
				CURSOR_SHOW;
				//else _SHIFT_LOCK;
			}
		}
	}
	if(__mode==_STRING_PASSWORD){
		CURSOR_SHOW;
		while(1){
			__key = _key_scan(_PAD_SINGLE);
			if(__key==_KEY_NULL || __key==_KEY_TIKET || __key==_KEY_MENU || __key==_KEY_SHIFT);
			else if(__key==_KEY_CANCEL){
				__chr_count = 0;
				CURSOR_HIDE;
				return 1;
			}
			else if(__key==_KEY_ENTER){
				__chr_count = 0;
				CURSOR_HIDE;
				return 0;
			}
			else if(__key==_KEY_CLEAR){
				if(__chr_count>0){
					_LCD_LEFT;
					_lcd(0x10);
					_LCD_LEFT;
					__buf_string[__chr_count-1] = '\0';
					__chr_count--;
				}
			}
			else{
				if(__chr_count<__max_string){
					_lcd(0x2a);
					__buf_string[__chr_count] = _table_alphanum(0, __key, 0);
					__buf_string[__chr_count + 1] = '\0';
					__chr_count++;
				}
			}
		}
	}
	if(__mode==_STRING_VALUE){
		for(__i=0; __i<__max_string; __i++)
			_lcd('0');
		while(1){
			__key = _key_scan(_PAD_SINGLE);
			if(__key==_KEY_NULL || __key==_KEY_TIKET || __key==_KEY_MENU || __key==_KEY_SHIFT || __key==_KEY_CLEAR);
			else if(__key==_KEY_CANCEL){
				__chr_count = 0;
				CURSOR_HIDE;
				return 1;
			}
			else if(__key==_KEY_ENTER){
				__chr_count = 0;
				CURSOR_HIDE;
				return 0;
			}
			else{
				for(__i=0; __i<__max_string - 1; __i++)
					__buf_string[__i] = __buf_string[__i + 1];
				__buf_string[__max_string - 1] = _table_alphanum(0, __key, 0);
				__buf_string[__max_string] = '\0';
				for(__i=0; __i<__max_string; __i++)
					_LCD_LEFT;
				for(__i=0; __i<__max_string; __i++)
					_lcd(__buf_string[__i]);
			}
		}
	}
	return 0;
}
