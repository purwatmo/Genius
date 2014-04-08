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
    
	RTC routine
	Controller	:	ATmega128/162 (8 Mhz internal)
	Compiler	:	AVR-GCC
	Version		:	1.1
	Author		:	Yofe Fegeanto, Jakarta (Indonesia)
					www.fegeanto.com
	Date		:	24 November 2009
*/

#include <compat/twi.h>

#include "RTC.h"

unsigned char _rtc(int __addr, int __len, char* __datetime, unsigned char __dir){
	int				__twcr;
	unsigned char	__i = 0;
	
	if(__dir){
		TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
		_rtc_response;
		if(TW_STATUS != TW_START)
			return 1;

		TWDR = _DS1307W;
		TWCR = _BV(TWINT) | _BV(TWEN);
		_rtc_response;
		if(TW_STATUS != TW_MT_SLA_ACK)
			return 1;

		TWDR = __addr;
		TWCR = _BV(TWINT) | _BV(TWEN);
		_rtc_response;
		if(TW_STATUS != TW_MT_DATA_ACK)
			return 1;

		for(; __len > 0; __len--){
			TWDR = __datetime[__i++];
			TWCR = _BV(TWINT) | _BV(TWEN);
			_rtc_response;
			if(TW_STATUS == TW_MT_DATA_NACK)
				return 1;
		}

		TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
	}
	else{
		TWCR =_BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
		_rtc_response;
		if(TW_STATUS != TW_START)
			return 1;

		TWDR = _DS1307W;
		TWCR = _BV(TWINT) | _BV(TWEN);
		_rtc_response;
		if(TW_STATUS != TW_MT_SLA_ACK)
			return 1;

		TWDR = __addr;
		TWCR = _BV(TWINT) | _BV(TWEN);
		_rtc_response;
		if(TW_STATUS != TW_MT_DATA_ACK)
			return 1;

		TWCR =_BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
		_rtc_response;
		if(TW_STATUS != TW_REP_START)
			return 1;

		TWDR = _DS1307R;
		TWCR =_BV(TWINT) | _BV(TWEN);
		_rtc_response;
		if(TW_STATUS != TW_MR_SLA_ACK)
			return 1;

		for(__twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWEA); __len > 0; __len--){
			if(__len == 1)
				__twcr = _BV(TWINT) | _BV(TWEN);
			TWCR = __twcr;
			_rtc_response;
			if(TW_STATUS == TW_MR_DATA_NACK)
				__len = 0;
			if(TW_STATUS == TW_MR_DATA_ACK)
				__datetime[__i++] = TWDR;
		}
		TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
	}

    return 0;
}

void _datetime(unsigned char __mode, char* __date, char* __time){
	char	__datetime[20];

	if(__mode){
		__datetime[4] = (((__date[0] - 0x30) & 0x0F) << 4) + ((__date[1] - 0x30) & 0x0F);
		__datetime[5] = (((__date[3] - 0x30) & 0x0F) << 4) + ((__date[4] - 0x30) & 0x0F);
		__datetime[6] = (((__date[6] - 0x30) & 0x0F) << 4) + ((__date[7] - 0x30) & 0x0F);

		__datetime[2] = (((__time[0] - 0x30) & 0x0F) << 4) + ((__time[1] - 0x30) & 0x0F);
		__datetime[1] = (((__time[3] - 0x30) & 0x0F) << 4) + ((__time[4] - 0x30) & 0x0F);
		__datetime[0] = (((__time[6] - 0x30) & 0x0F) << 4) + ((__time[7] - 0x30) & 0x0F);
		_rtc(0, 8, __datetime, __mode);
	}
	else
		if(_rtc(0, 8, __datetime, __mode) != 1){
			__date[0] = (__datetime[4] >> 4) + 0x30;
			__date[1] = (__datetime[4] & 0x0F) + 0x30;
			__date[2] = '/';
			__date[3] = (__datetime[5] >> 4) + 0x30;
			__date[4] = (__datetime[5] & 0x0F) + 0x30;
			__date[5] = '/';
			__date[6] = (__datetime[6] >> 4) + 0x30;
			__date[7] = (__datetime[6] & 0x0F) + 0x30;
			__date[8] = '\0';

			__time[0] = (__datetime[2] >> 4) + 0x30;
			__time[1] = (__datetime[2] & 0x0F) + 0x30;
			__time[2] = ':';
			__time[3] = (__datetime[1] >> 4) + 0x30;
			__time[4] = (__datetime[1] & 0x0F) + 0x30;
			__time[5] = ':';
			__time[6] = (__datetime[0] >> 4) + 0x30;
			__time[7] = (__datetime[0] & 0x0F) + 0x30;
			__time[8] = '\0';
		}
}

unsigned char _day(void){
	char	__datetime[20];

	if(_rtc(0, 8, __datetime, 0) != 1)
		return __datetime[3] & 0x0F;
	return 0;
}