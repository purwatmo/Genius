void _f_inttostr(char* __string, unsigned long __value){
	char			__flag = 0, __i = 0, __count;
	unsigned long	__num, __devider = 1000000000;
	int				__tmp;

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

void intToStr(char X, char *str){
     char R,P,S;
	 R=X/100;
	 P=(X%100)/10;
	 S=X-(R*100)-(P*10);
	 if (X>=100){
	     str[0]=('0'+R);
	     str[1]=('0'+P);
	     str[2]=('0'+S);
	     str[3]=0;

	 }else 
	 if ((X>=10)&&(X>100)){
	     str[0]=('0'+P);
	     str[1]=('0'+S);
	     str[2]=0;
	 }else 
	 if (X<=10){
	     str[0]=('0'+S);
	     str[1]=0;
	 }
}

void charToHex(char X, char *Result){
     Result[0]=Str(High(X));
	 Result[1]=Str(Low(X));
	 Result[2]=0;
	 
}

char strToInt(char *str){
     char Result;
     Result=(((str[0]-'0')*10) +(str[0]-'0'));
	 return Result;
}

char Low(char X){
     char Result;
	 Result=(0x0F&X);
	 return Result;
}
char High(char X){
     char Result;
	 Result=((X>>4)&(0x0F));
	 return Result;
}
char Str(char H){
unsigned char Conv=0;
        if ((H>=0)&&(H<=9)) Conv='0'+H;
        else
        if ((H>=0x0A)&&(H<=0x0F)) Conv='A'+(H-10);    
        return (Conv);
}

char strInvoiceNumber[11];
void _f_punctuation(char* __string, unsigned char __mode, unsigned char __length, unsigned char __decimal);
void _f_inttostr(char* __string, unsigned long __value);
