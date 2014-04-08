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
    
	Interfacing 4x4 Keypad (Header)
	Controller	:	ATmega128 (8 Mhz internal)
	Compiler	:	AVR-GCC
	Version		:	1.1
	Author		:	Yofe Fegeanto, Jakarta (Indonesia)
					www.fegeanto.com
	Date		:	24 November 2009
*/

#ifndef	__KEYPAD_H__
#define __KEYPAD_H__

#define _KEY_PORT	PORTF
#define _KEY_MATRIX	PINF

#define	_KEY_ENTER	0xB7
#define	_KEY_CANCEL	0xE7

#define	_KEY_1		0xEE
#define	_KEY_2		0xDE
#define	_KEY_3		0xBE
#define	_KEY_4		0xED
#define	_KEY_5		0xDD
#define	_KEY_6		0xBD
#define	_KEY_7		0xEB
#define	_KEY_8		0xDB
#define	_KEY_9		0xBB
#define	_KEY_0		0xD7

#define	_KEY_TIKET	0x7E
#define	_KEY_MENU	0x7D
#define	_KEY_SHIFT	0x7B
#define	_KEY_CLEAR	0x77

#define	_KEY_NULL	0x00
#define	_KEY_FAIL	0xFF
#define	_KEY_RESET	0XAA

#define CURSOR_HIDE	{lcd_command(0x0c);}
#define CURSOR_SHOW	{lcd_command(0x0f);}
#define DISP_SHIFT	{lcd_command(0x05);}


unsigned char	__sec_count;
unsigned char	__sec_flag;
unsigned char	__sec_sign;
unsigned char	__sec_par;

#define _SEC_ON			{__sec_flag = 1; __sec_count = 0; __sec_sign = 0;}
#define _SEC_OFF		{__sec_flag = 0; __sec_sign = 1;}
#define _SEC_TICK		__sec_count++
#define _SEC_SET		{__sec_par = 1; __sec_flag = 1; __sec_count = 0; __sec_sign = 0;}
#define _SEC_RESET		{__sec_sign = 0; __sec_par = 0;}

// ---------------------------------------------------------------------------------------------------------
// Flag Control
// ---------------------------------------------------------------------------------------------------------
//extern char	__buf_anum;		// Buffer untuk menyimpan nilai terakhir penekanan tombol keypad
//extern char	__hit_count;		// Counter penekanan Keypad untuk tombol yang sama
//extern char __chr_count;		// Counter jumlah karakter yang disimpan
//extern char	__max_string;		// Jumlah maksimum karakter pada string untuk satu proses
//extern char	__caps_lock;		// Counter penekanan Keypad untuk tombol yang sama
//extern char __lock_num;
extern int TimMenuTimeout;

#define _STRING_NUMERIC		0
#define _STRING_ALPHANUM	1
#define _STRING_PASSWORD	2
#define _STRING_VALUE		3
#define _STRING_PUMP		4

// ---------------------------------------------------------------------------------------------------------
// Shift Control
// ---------------------------------------------------------------------------------------------------------
extern char	__key_shift;
extern char	__key_shfcnt;
extern char	__key_sfflck;


#define _SHIFT_TICK	__key_shfcnt++
#define _SHIFT_ON	__key_shift = 1
#define _SHIFT_OFF	__key_shift = 0
#define _SHIFT_LOCK	__key_sfflck = 1
#define _SHIFT_FREE	__key_sfflck = 0
#define _SHIFT_SET	{__key_shift = 1; __key_shfcnt = 0;}
#define _SHIFT_RST	{	__key_shift = 0; __key_shfcnt = 0; __buf_anum = 0; __hit_count = 0;  \
						__chr_count++; if(__chr_count == 20) lcd_xy(3, 1); _LCD_RIGHT; CURSOR_SHOW;}

#define _COUNTSHIFT	__key_shfcnt == 20



// ---------------------------------------------------------------------------------------------------------
// Light Control
// ---------------------------------------------------------------------------------------------------------
extern char	__key_light;
extern char	__key_lgtcnt;

#define _LIGHT_TICK	__key_lgtcnt++

#define _LIGHT_ON	__key_light = 1
#define _LIGHT_OFF	__key_light = 0

#define _LIGHT_SET	{__key_light = 1; __key_lgtcnt = 0; cbi(PORTG, 1);}
#define _LIGHT_RST	{__key_light = 0; __key_lgtcnt = 0; sbi(PORTG, 1);}

#define _ISLIGHT	__key_light == 1
#define _COUNTLIGHT	__key_lgtcnt == 150

// ---------------------------------------------------------------------------------------------------------
// Rollback Control
// ---------------------------------------------------------------------------------------------------------
/*extern char	__key_rollback;
extern char	__key_rbkcnt;
extern char	__key_parse;
extern char	__key_sign;
*/
// ---------------------------------------------------------------------------------------------------------
// Pad Loop Control
// ---------------------------------------------------------------------------------------------------------
/*extern char	__pad_buffer;
extern char	__pad_loop;
*/
#define	_PAD_FREE	0
#define	_PAD_SINGLE	1
#define	_PAD_MULTI	2

#define	_PAD_1st __pad_loop = 0
#define	_PAD_2nd __pad_loop = 1

#define	_IS1st __pad_loop == 0
#define	_IS2nd __pad_loop == 1

extern char	_key_hit(void);
extern char	_key_crr(char __key);
extern char	_key_btn(char __key);

extern char	_key_scan(char __select);
extern char	_table_alphanum(char __caps, char __key, char __hit);


#endif
