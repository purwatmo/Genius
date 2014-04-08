
#include "FSystem.h"
#include "FSlave.h"
#include "FTicket.h"

void UpdateIFT_ID(){
     char IdIFT;
	 IdIFT=(eeprom_read_byte(&DefIFT_ID)%100);
	 sprintf_P(strIFT_ID,PSTR("%.2d"),IdIFT);
}

void UpdateSeqNum(){
     if (SeqNum<100)SeqNum++;
	 else SeqNum=0;
	 leadingZero(SeqNum,strSeqNum);
} 

void UpdateClientIP(){     
     char i,IP_blok[4];
	 char strIP[4][5];
	 //192.168.010.002
     eeprom_read_block((void*)&IP_blok,(const void*)&DefClientIP,4);

	 for(i=0;i<4;i++){
	     zeroIP(IP_blok[i],strIP[i]);
	 }
     sprintf_P(strClientIP,PSTR("%s.%s.%s.%s"),strIP[0],strIP[1],strIP[2],strIP[3]);
}
void UpdateServerIP(){     
     char i,IP_blok[4];
	 char strIP[4][4];
	 //192.168.016.180
     eeprom_read_block((void*)&IP_blok,(const void*)&DefServerIP,4);

	 for(i=0;i<4;i++){
	     zeroIP(IP_blok[i],strIP[i]);
	 }
		 sprintf_P(strServerIP,PSTR("%s.%s.%s.%s"),strIP[0],strIP[1],strIP[2],strIP[3]);
}



/*Subrutine Msg04*/
void sendMessage04(){   //      <STX>[IFTID][Seq][No][SrceIP][DestIP][MsgCode][ReceiptNo][Value][Checksum][ETX]
                        //Msg04: <01>[01][03][192.168.000.101][192.168.000.001][04][000001]0F968CFFB]<02>
	 char strSend[60];
	 _uart(1, 1,0x01);
	 UpdateIFT_ID(); //ReadIFT_ID
	 UpdateSeqNum(); //UpdateSeqNum SeqNum++
	 UpdateClientIP();//ReadSourceIP
	 UpdateServerIP();//ReadDestIP
	 //strTranNo = strReceiptNum
	 sprintf_P(strSend,PSTR("%s%s%s%s04%s1F968CFFB"),strIFT_ID,strSeqNum,strClientIP,strServerIP,strTranNo);
     _uart_print(1, 0,strSend);
	 _uart(1, 1,0x02);
}

/*Subrutine Msg10*/
void sendMessage10(){//Msg10: <01>[0103192.168.016.070192.168.016.18010F968CFFB]<02>
	char strSend[60];
	_uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	sprintf_P(strSend,PSTR("%s%s%s%s10F968CFFB"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    _uart_print(1, 0,strSend);
	_uart(1, 1,0x02);
	IsNewPacket=True;
}

void sendMessage22(){//Msg22: <01>[0103192.168.016.070192.168.016.18022[CardID]F968CFFB]<02>
	char strSend[60];
	_uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	sprintf_P(strSend,PSTR("%s%s%s%s22"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    _uart_print(1, 0,strSend);
	UpdateCardID();
	sprintf_P(strSend,PSTR("%sF968CFFB"),strCardID);
    _uart_print(1, 0,strSend);
	_uart(1, 1,0x02);
}

void sendMessage24(){//Msg24: <01>[0103192.168.016.070192.168.016.18022[CardID]F968CFFB]<02>
	char strSend[60];
	_uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	sprintf_P(strSend,PSTR("%s%s%s%s24"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    _uart_print(1, 0,strSend);
	UpdateCardID();
	sprintf_P(strSend,PSTR("%s%sF968CFFB"),strCardID,strFIP_ID);
    _uart_print(1, 0,strSend);
	_uart(1, 1,0x02);
}

void sendMessage28(){//Msg28: <01>[0103192.168.016.070192.168.016.18024[FIP][CardID]F968CFFB]<02>
	char strSend[60];
	_uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	sprintf_P(strSend,PSTR("%s%s%s%s28"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    _uart_print(1, 0,strSend);
	UpdateCardID();
	sprintf_P(strSend,PSTR("%s%sF968CFFB"),strFIP_ID,strCardID);
    _uart_print(1, 0,strSend);
	_uart(1, 1,0x02);
}



void sendMessage32(){//Msg32: <01>[ID][Seq][SrcIP][DestIP][MsgCode][FIP][PaymentType][Ref1][Ref2][Ref3][Ref4]<02>
	char strSend[80];
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	_uart(1, 1,0x01);
	sprintf_P(strSend,PSTR("%s%s%s%s32"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    _uart_print(1, 0,strSend);
	sprintf_P(strSend,PSTR("%s%s%s%s"),strFIP_ID,strPaymentType,strRef1,strRef2);
    _uart_print(1, 0,strSend);
	sprintf_P(strSend,PSTR("%s%s"),strRef3,strRef4);
    _uart_print(1, 0,strSend);
	sprintf_P(strSend,PSTR("F968CFFB"));
    _uart_print(1, 0,strSend);
	_uart(1, 1,0x02);

}

void sendMessage56(){
	char strSend[60];
	_uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	UpdateCardID();  //ReadCardID

	sprintf_P(strSend,PSTR("%s%s%s"),strIFT_ID,strSeqNum,strClientIP);
    _uart_print(1, 0,strSend);
	sprintf_P(strSend,PSTR("%s56"),strServerIP);
    _uart_print(1, 0,strSend);
    sprintf_P(strSend,PSTR("%s"),strCardID);
    _uart_print(1, 0,strSend);	

	_uart_printf(1,0,PSTR("AF968CFFB"));
	_uart(1, 1,0x02);
}

void sendMessage58(){
	char strSend[80];
	_uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	UpdateCardID();  //ReadCardID
	sprintf_P(strSend,PSTR("%s%s%s%s58"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    _uart_print(1, 0,strSend);
	AddSpaceLead(strBalanceValue,13);
	AddSpaceLead(strOdometer,10);
	sprintf_P(strSend,PSTR("%s%s%d%s%s"),strCardID,strFIP_ID,NozzleID,strBalanceType,strBalanceValue);
    _uart_print(1, 0,strSend);

	CardType=0;
	sprintf_P(strSend,PSTR("%dF0000000E123456FFFFF%sE9445512"),CardType,strOdometer);
    _uart_print(1, 0,strSend);
	_uart(1, 1,0x02);
}

void sendMessage90(){
	char strSend[80];
	_uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	sprintf_P(strSend,PSTR("%s%s%s%s90"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    _uart_print(1, 0,strSend);
	sprintf_P(strSend,PSTR("%s%s"),strFIP_ID,strRef1);
    _uart_print(1, 0,strSend);
	sprintf_P(strSend,PSTR("E9445512"));
    _uart_print(1, 0,strSend);
	_uart(1, 1,0x02);
}

void sendMessage92(){
	char strSend[80];
	_uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	sprintf_P(strSend,PSTR("%s%s%s%s92"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    _uart_print(1, 0,strSend);
    //_uart_print(0, 0,strSend);
	sprintf_P(strSend,PSTR("%s%s%s%s"),strTranNo,strFIP_ID,strCardType,strCardID);
    _uart_print(1, 0,strSend);
    //_uart_print(0, 0,strSend);
	sprintf_P(strSend,PSTR("%s%s%s"),strApprovalCode,strInvoiceNumber,strDateTime);
    _uart_print(1, 0,strSend);
    //_uart_print(0, 0,strSend);
	sprintf_P(strSend,PSTR("E9445512"));
    //_uart_print(0, 0,strSend);
    _uart_print(1, 0,strSend);
	_uart(1, 1,0x02);
}

void sendMessage94(){//Void Transaction Message
	char strSend[80];
	_uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	sprintf_P(strSend,PSTR("%s%s%s%s94"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    _uart_print(1, 0,strSend);
    //_uart_print(0, 0,strSend);
	sprintf_P(strSend,PSTR("%s%s"),strInvoiceNumber,strDateTime);
    _uart_print(1, 0,strSend);
	sprintf_P(strSend,PSTR("E9445512"));
    //_uart_print(0, 0,strSend);
    _uart_print(1, 0,strSend);
	_uart(1, 1,0x02);
}


char procMessage99(){//<STX>[IFT IDSeq N Srce IPDest IPMsg CodeTran NoShiftDateTimeIsland IDFIP IDProduct IDDescriptionPriceVolumeAmountMOP TypeMOP NameCard IDCard HolderBalance Type Balance MeterVolume MeterAmount Current TimePrint Count Checksum ETX
     char Result;
	 Result=MSG99_NONE;
	 //Message99
	 if((rcv_trans[0]==0x01)&&(transLength>=378)){
       StrPosCopy(rcv_trans,strTranNo,37,6);//Transaction Number
       StrPosCopy(rcv_trans,strShift,43,1);
       Shift=(CharPosCopy(rcv_trans,43)-'0');
       StrPosCopy(rcv_trans,strDate,44,10);
       StrPosCopy(rcv_trans,strTime,54,8);
       StrPosCopy(rcv_trans,strIslandID,62,2);
       StrPosCopy(rcv_trans,strFIP_ID,64,2);
       StrPosCopy(rcv_trans,strProductID,66,2);
       StrPosCopy(rcv_trans,strDescription,68,15);
       
	   //FillChar(strPrice,sizeof(strPrice),0);
       StrPosCopy(rcv_trans,strPrice,83,8);

	   //FillChar(strVolume,sizeof(strVolume),0);
       StrPosCopy(rcv_trans,strVolume,91,8);

	   //FillChar(strAmount,sizeof(strAmount),0);
       StrPosCopy(rcv_trans,strAmount,99,10);

       //StrPosCopy(rcv_trans,strMOPType,109,1);
       MOPType=(CharPosCopy(rcv_trans,109)-'0');
       StrPosCopy(rcv_trans,strMOPName,110,20);
       StrPosCopy(rcv_trans,strCardID,130,20);
       StrPosCopy(rcv_trans,strCardHolder,150,40);
       StrPosCopy(rcv_trans,strBalanceTypePrint,190,25);
       StrPosCopy(rcv_trans,strBalance,215,13);
       StrPosCopy(rcv_trans,strMeterVolume,228,13);
       StrPosCopy(rcv_trans,strMeterAmount,241,13);
       StrPosCopy(rcv_trans,strCurrentTime,254,19);
       StrPosCopy(rcv_trans,strPrintCount,273,2);
       //Loyalty
	   StrPosCopy(rcv_trans,strPrevPoints,275,8);
       StrPosCopy(rcv_trans,strGainPoints,283,8);
       StrPosCopy(rcv_trans,strLoyCardID,291,20);
       StrPosCopy(rcv_trans,strLoyCardHolder,311,30);
       StrPosCopy(rcv_trans,strLoyCurrentPoints,341,8);
       StrPosCopy(rcv_trans,strLoyCurrMonConsumeA,349,10);
       StrPosCopy(rcv_trans,strLoyCurrMonConsumeV,359,10);
       StrPosCopy(rcv_trans,strSurchargeDesc,369,20);
       StrPosCopy(rcv_trans,strSurchargeAmount,389,10);
       StrPosCopy(rcv_trans,strLoyRedeemPoints,399,8);
       StrPosCopy(rcv_trans,strLoyExpiry,407,10);
       StrPosCopy(rcv_trans,strCorporateID,417,20);
       StrPosCopy(rcv_trans,strCorporateName,437,30);
	   //


	   //FIP Detection LocalAccount Null Filling
	   if ((nLocalAccount>0)&&(LocalAccountFIP[0]==atoi(strFIP_ID))){
	        IsCompleteFilling=True;
	   }

	  }
     return Result;     
}

void procMessage11(){
     unsigned int i;
	 unsigned long ProdPrice=0;
     char buffHeader[41];
	 char strProductName[13],strProductPrice[9],strTime[12],strDate[10];

     //Update Datetime
	 StrPosCopy(rcv_trans,strDate,43,10);//2004/09/14 19:05:36
	 FormatDate(DATE_SHORT_YEAR,strDate);
	 StrPosCopy(rcv_trans,strTime,54,8);
	 _datetime(_DATETIME_WRITE,strDate,strTime);
	  
     clearString(buffHeader);
	 FillChar(buffHeader,0,sizeof(buffHeader));   
	 StrPosCopy(rcv_trans,buffHeader,62,40);
	 eeprom_write_block((const void*) &buffHeader, (void*) &DefHeaderFooter[0], 40);
     StrPosCopy(rcv_trans,buffHeader,102,40);
	 eeprom_write_block((const void*) &buffHeader, (void*) &DefHeaderFooter[1], 40);
     StrPosCopy(rcv_trans,buffHeader,142,40);
     eeprom_write_block((const void*) &buffHeader, (void*) &DefHeaderFooter[2], 40);
     StrPosCopy(rcv_trans,buffHeader,182,40);
     eeprom_write_block((const void*) &buffHeader, (void*) &DefHeaderFooter[3], 40);
     StrPosCopy(rcv_trans,buffHeader,222,40);
     eeprom_write_block((const void*) &buffHeader, (void*) &DefHeaderFooter[4], 40);
     StrPosCopy(rcv_trans,buffHeader,262,40);
     eeprom_write_block((const void*) &buffHeader, (void*) &DefHeaderFooter[5], 40);
     StrPosCopy(rcv_trans,buffHeader,302,40);
     eeprom_write_block((const void*) &buffHeader, (void*) &DefHeaderFooter[6], 40);
     StrPosCopy(rcv_trans,buffHeader,342,40);
     eeprom_write_block((const void*) &buffHeader, (void*) &DefHeaderFooter[7], 40);
     StrPosCopy(rcv_trans,buffHeader,382,40);
     eeprom_write_block((const void*) &buffHeader, (void*) &DefHeaderFooter[8], 40);
     StrPosCopy(rcv_trans,buffHeader,422,40);
     eeprom_write_block((const void*) &buffHeader, (void*) &DefHeaderFooter[9], 40);
	 //ProductName
	 for(i=0;i<6;i++){
	    StrPosCopy(rcv_trans,strProductName,(486+(i*12)),12);
		RemSpaceLag(strProductName);
        eeprom_write_block((const void*) &strProductName, (void*) &DefProductName[i], 12);
	 }
	 //ProductPrice
	 for(i=0;i<6;i++){
        StrPosCopy(rcv_trans,strProductPrice,(558+(i*8)),8);
		RemSpaceLag(strProductPrice);
		RemZeroLead(strProductPrice);
		RemDecimal(strProductPrice);
        eeprom_write_block((const void*) &strProductPrice, (void*) &DefProductPrice[i], 8);
	 }
}
void ProcMessage91(){
     StrPosCopy(rcv_trans,strTranNo,37,6);
     StrPosCopy(rcv_trans,strFIP_ID,43,2);
     StrPosCopy(rcv_trans,strDescription,45,15);
     StrPosCopy(rcv_trans,strPrice,60,6);
     StrPosCopy(rcv_trans,strVolume,66,8);
     StrPosCopy(rcv_trans,strAmount,74,8); 
	 StrPosCopy(rcv_trans,strStatus,82,1);
     StrPosCopy(rcv_trans,strSurcharge,83,9);    
}

char FMenuLoyalty(){
static char stLoyalty=mlInit;
static char KeyPressed=0,FIP_Used=0,IsLoyaltyUpdate=False;
       char lcdteks[20];
       char FIPResult,FIP_USED;
	   char Result=MENU_NONE;
	   char KeyChar;

     Result=MENU_NONE;
	 switch(stLoyalty){
	 case mlInit:
	      lcd_clear();
		  lcd_printf(1,1,PSTR("  Menu Loyalty   "));
          lcd_printf(1,1,PSTR("1.Enquiry        "));
		  lcd_printf(2,1,PSTR("2.Loyalty Update "));
	      lcd_printf(4,1,PSTR("[*]Back          "));
		  IsLoyaltyUpdate=False;
		  stLoyalty=mlLoyaltySelect;
	      break;
	 case mlLoyaltySelect:
	 	  KeyPressed=_key_scan(1);
		  if (KeyPressed==_KEY_1){
		      IsLoyaltyUpdate=False;
		      stLoyalty=mlShowEnquiry;
			  }
		  else
		  if (KeyPressed==_KEY_2){
		      IsLoyaltyUpdate=True;
			  stLoyalty=mlSelectFIP;
			  }
		  if (KeyPressed==_KEY_CANCEL)stLoyalty=mlExitLoyalty;
	      break;
     case mlSelectFIP:
	      FIPResult=menu_FIP(&FIP_Used,strFIP_ID);
		  if (FIPResult==FIP_DONE)stLoyalty=mlUpdateLoyalty;
		  else
		  if (FIPResult==FIP_CANCEL)stLoyalty=mlInit;
	      break;
     case mlUpdateLoyalty: 
	      lcd_clear();
		  sprintf_P(lcdteks,PSTR("FIP:%s"),strFIP_ID);
		  lcd_printf(1,1,PSTR("Tap Kartu RFID"));
		  lcd_print(2,1,lcdteks);
          lcd_printf(4,1,PSTR("[*]Back    [#]Exit"));
		  IsRFIDDetected=False;
	      stLoyalty=mlInputRFID;
	      break;
	 case mlShowEnquiry:
	      lcd_clear();
		  lcd_printf(1,1,PSTR("Tap Kartu RFID"));
          lcd_printf(4,1,PSTR("[*]Back    [#]Exit"));
		  IsRFIDDetected=False;
		  stLoyalty=mlInputRFID;
	      break;
     case mlInputRFID:
	      if (IsRFIDDetected==True){
		      IsRFIDDetected=False;
		      stLoyalty=mlShowProsesRFID;
		  }
	 	  KeyPressed=_key_scan(1);
		  if (KeyPressed==_KEY_CANCEL){
		      if (IsLoyaltyUpdate==True)
			       stLoyalty=mlSelectFIP;
			  else stLoyalty=mlInit;
			  }
		  else
		  if (KeyPressed==_KEY_ENTER)stLoyalty=mlExitLoyalty;
	      break;
     case mlShowProsesRFID:
	      lcd_clear();
		  sprintf_P(lcdteks,PSTR("ID:%s"),strRFID);
		  lcd_print (1,1,lcdteks);	      
		  lcd_printf(2,1,PSTR("Proses"));	      
		  if (IsLoyaltyUpdate==True)stLoyalty=mlSendMessage24;
		  else stLoyalty=mlSendMessage22;
	      break;
     case mlSendMessage22:
	      sendMessage22();
		  TimSend=0;		  
          stLoyalty=mlWaitReply;
	      break;
     case mlSendMessage24:
	      sendMessage24();
		  TimSend=0;		  
          stLoyalty=mlWaitReply;
	      break;
     case mlWaitReply:
	      if (TimSend>(TIM_SEND*3))
		      stLoyalty=mlNoConnection;
	      if (IsMessage23==True){
		      IsMessage23=False;
			  procMessage23();
			  stLoyalty=mlDispEnquiry;
		  }
	      if (IsMessage09==True){
		      IsMessage09=False;
	          procMessage09();
              stLoyalty=mlDisplayFreeMessage;
		  }
	      if (IsMessage99==True){
              stLoyalty=mlDelayExitLoyalty;
		  }
	      break;
     case mlDisplayFreeMessage:
		  lcd_clear();
		  lcd_print(1,1,strFreeMessageLine1);
		  lcd_print(2,1,strFreeMessageLine2);
		  lcd_print(3,1,strFreeMessageLine3);
		  lcd_print(4,1,strFreeMessageLine4);
		  TimDisplay=0;
		  stLoyalty=mlDelayExitLoyalty;
	      break;
     case mlDispEnquiry:	      
	      lcd_clear();
		  RemSpaceLag(strCardID);RemSpaceLag(strCardHolder);
		  sprintf_P(lcdteks,PSTR("%s:%s"),strCardID,strCardHolder);
		  lcd_print(1,1,lcdteks);RemSpaceLag(strLoyCurrentPoints);
		  sprintf_P(lcdteks,PSTR("Points:%s"),strLoyCurrentPoints);
		  lcd_print(2,1,lcdteks);RemSpaceLag(strLoyCurrentPoints);
		  sprintf_P(lcdteks,PSTR("CM Amt:%s"),strLoyCurrMonConsumeA);
		  lcd_print(3,1,lcdteks);RemSpaceLag(strLoyCurrentPoints);
		  sprintf_P(lcdteks,PSTR("Cm Vol:%s"),strLoyCurrMonConsumeV);
		  lcd_print(4,1,lcdteks);
		  TimDisplay=0;

		  if (eeprom_read_byte(&DefNotifScreen)==1) {
		      stLoyalty=mlPressAnyKey;
			  lcd_printf(2,1,PSTR("Press Any Key"));
          } else stLoyalty=mlDelayExitLoyalty;
		  break;
     case mlPressAnyKey:
	      KeyPressed=_key_scan(1);
	      KeyChar=_key_btn(KeyPressed);
	      switch(KeyChar){		  
		  case '#':
		  case '*':
               stLoyalty=mlExitLoyalty;
		       break;		  
		  }
	      break;
     case mlNoConnection:
	      lcd_clear();
		  lcd_printf(2,1,PSTR("TCP/IP ERROR "));
		  lcd_printf(3,1,PSTR("No Connection"));
		  TimDisplay=5;
		  stLoyalty=mlDelayExitLoyalty;
	      break;
     case mlDelayExitLoyalty:
          if(TimDisplay>=10)stLoyalty=mlExitLoyalty;
	      break;
	 case mlExitLoyalty:
	      lcd_clear();
	      stLoyalty=mlInit;
		  Result=MENU_DONE;
	      break;
	 }
	 return Result;
}

char FMenuChangeMOP(){
static char stChangeMOP=cmInit,FIPResult;
static char iPos=0,iWait=0,KeyChar;//,KeyCharLast=' ',BalanceType=0,strFIP[3];
static char KeyPressed=0,FIP_Used=0,ValueChar[10];//,iValuePos=0,iNozzle=0;
       char lcdteks[20],i,msgResult;
       //char strName[21],FIP[8],strPumpL[3],strPumpR[3];;
static unsigned int iLoop=0;
	   char strBankName[11],strSurcharge[4],Result;
static char BankIdx=0,uiResult;//,Surcharge=0;
     
Result=MENU_NONE; 

	 switch(stChangeMOP){
	 case cmInit:
	      MOPType=PAY_NONE;
		  stChangeMOP=cmDisplayMOPOption;
	      break;
	 case cmDisplayMOPOption: //12345678901234567890
	      lcd_clear();
	      lcd_printf(1,1,PSTR("   Select Payment   "));
	      lcd_printf(2,1,PSTR("1.Account 3.Voucher "));
	      lcd_printf(3,1,PSTR("2.Bank    4.PumpTest"));
	      lcd_printf(4,1,PSTR("[*]Back             "));
          stChangeMOP=cmInputMOP;		 
		  break;
	 case cmInputMOP:
	      KeyPressed=_key_scan(1);
		  KeyChar=_key_btn(KeyPressed);
		  if ((KeyChar>='1')&&(KeyChar<='4')){
              switch(KeyChar){
		        case '1':MOPType=PAY_ACCOUNT; stChangeMOP=cmSelectFIP;         break;
		        case '2':MOPType=PAY_BANK;    stChangeMOP=cmSelectBankName;    break;
		        case '3':MOPType=PAY_VOUCHER; stChangeMOP=cmDispInputVoucher;  break;
		        case '4':MOPType=PAY_PUMPTEST;stChangeMOP=cmSelectFIP;         break;
		     }
		  }
		  if (KeyPressed==_KEY_CANCEL) stChangeMOP=cmExitChangeMOP;
	      break;
	 case cmDispInputVoucher://Enter Voucher number
          lcd_clear();
		  lcd_printf(1,1,PSTR("Kode Voucher"));
		  lcd_printf(2,1,PSTR("_"));
		  lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
		  FillChar(strVoucherNum,sizeof(strVoucherNum),0);
          stChangeMOP=cmInputVoucher;
		  uiResult=USER_NONE;
	      break;
     case cmInputVoucher:
          uiResult=UserInput(UI_NUMBER_R,2,1,strVoucherNum,0,13);
	      if (uiResult==USER_OK){
		     stChangeMOP=cmSelectFIP;
			 }
		  else
	      if (uiResult==USER_CANCEL)stChangeMOP=cmDisplayMOPOption;
		  else
		  if (uiResult==USER_ENTRY)lcd_printf(4,1,PSTR("[*]Back     [#]Enter"));
		  else
		  if (uiResult==USER_NO_DATA)lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
	      break;
	 case cmSelectFIP:
	      FIPResult=menu_FIP(&FIP_Used,strFIP_ID);
		  if (FIPResult==FIP_DONE){
		      //sprintf_P(strFIP_ID,PSTR("%.2d"),FIP_Used);
		      stChangeMOP=cmFlowFIP;
		  }
		  else
		  if (FIPResult==FIP_CANCEL){
		      if (MOPType==PAY_ACCOUNT)stChangeMOP=cmDisplayMOPOption;
		      if (MOPType==PAY_BANK)stChangeMOP=cmSelectBankName;
		      if (MOPType==PAY_VOUCHER)stChangeMOP=cmDispInputVoucher;
		      if (MOPType==PAY_PUMPTEST)stChangeMOP=cmDisplayMOPOption;		  
		  }

		  break; 
     case cmSelectFIPInput:
		
	      break;
		  //Load Bank Information
	 case cmSelectBankName:
	      lcd_clear();
		  lcd_printf(1,1,PSTR("Select Bank"));
		  iLoop=0;
	      for(iLoop=0;iLoop<4;iLoop++){
			  eeprom_read_block((void*) &strBankName, (const void*) &DefBankName[iLoop], 11);
			  if (strlen(strBankName<=10)){
			     sprintf_P(lcdteks,PSTR("%d.%s"),(iLoop+1),strBankName);
			     lcd_print((2+(iLoop%2)),(1+((iLoop/2)*12)),lcdteks);
				 }
		  }
	      lcd_printf(4,1,PSTR("[*]Back"));
		  stChangeMOP=cmSelectBankNameInput;
	      break;
     case cmSelectBankNameInput:
		  KeyPressed=_key_scan(1);
		  KeyChar=_key_btn(KeyPressed);
          if ((KeyChar>='1')&&(KeyChar<='4')){
		      BankIdx=KeyChar-'1';
			  stChangeMOP=cmSelectFIP;
		  }
		  if (KeyPressed==_KEY_CANCEL) stChangeMOP=cmDisplayMOPOption;

	      break;
     case cmDispBankSurcharge:
	      lcd_clear();
		  eeprom_read_block((void*) &strBankName, (const void*) &DefBankName[BankIdx], 11);
		  sprintf_P(lcdteks,PSTR("%s"),strBankName);
		  lcd_print(1,1,lcdteks);
		  lcd_printf(2,1,PSTR("Surcharge:   _%"));
	      lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
          stChangeMOP=cmInputBankSurcharge;
	      break;
	 case cmInputBankSurcharge:
	      uiResult=UserInput(UI_NUMBER_L,2,14,ValueChar,100,3);
	      if (uiResult==USER_OK)stChangeMOP=cmGenerateData;
		  else
	      if (uiResult==USER_CANCEL)stChangeMOP=cmSelectBankName;
		  else
		  if (uiResult==USER_ENTRY)lcd_printf(4,1,PSTR("[*]Back     [#]Enter"));
		  else
		  if (uiResult==USER_NO_DATA)lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
	      break;
     case cmFlowFIP:
          switch(MOPType){
		  case PAY_ACCOUNT: stChangeMOP=cmDispCardTap;       break;
		  case PAY_BANK:    stChangeMOP=cmDispBankSurcharge; break;
		  case PAY_VOUCHER: stChangeMOP=cmProsesVoucher;      break;
		  case PAY_PUMPTEST:stChangeMOP=cmDispCardTap;       break;
		  }
	      break;
     case cmDispCardTap:
	      lcd_clear();
	      lcd_printf(1,1,PSTR("Tap Kartu RFID"));
		  sprintf_P(lcdteks,PSTR("FIP%s"),strFIP_ID);
		  lcd_print(2,1,lcdteks);
          lcd_printf(4,1,PSTR("[*]Back    [#]Exit"));
		  IsRFIDDetected=False;
          stChangeMOP=cmRFIDCardInput;
	      break;
	 case cmRFIDCardInput:
		  KeyPressed=_key_scan(1);
		  KeyChar=_key_btn(KeyPressed);
		  if (KeyPressed==_KEY_ENTER){
		      stChangeMOP=cmExitChangeMOP;
		  }
		  else
		  if (KeyPressed==_KEY_CANCEL){
   		      stChangeMOP=cmSelectFIP;
			 }
          if (IsRFIDDetected==True){
		      IsRFIDDetected=False;
			   stChangeMOP=cmProsesRFID;
			 }
	      break;
     case cmProsesRFID:
	      ViewCardID();
          sprintf_P(lcdteks,PSTR("ID:%s"),strCardID);lcd_print(2,1,lcdteks);
          lcd_printf(3,1,PSTR("Proses"));
		  stChangeMOP=cmGenerateData;
	      break;
     case cmProsesVoucher:
          lcd_clear();
          lcd_printf(1,1,PSTR("-MOP Voucher-")); 
          lcd_printf(3,1,PSTR("Proses"));
		  stChangeMOP=cmGenerateData;
	      break;

     case cmGenerateData:
	      FillChar(strRef1,sizeof(strRef1),0);
	      FillChar(strRef2,sizeof(strRef2),0);
	      FillChar(strRef3,sizeof(strRef3),0);
	      FillChar(strRef4,sizeof(strRef4),0);
          switch(MOPType){
		  case PAY_ACCOUNT:
		       sprintf_P(strRef1,PSTR("%s"),strCardID); 
		       break;
		  case PAY_BANK:
		  	   eeprom_read_block((void*) &strBankName, (const void*) &DefBankName[BankIdx], 11);
		       sprintf_P(lcdteks,PSTR("%s"),strBankName);
		       sprintf_P(strRef1,PSTR("%s"),strBankName); 
		       sprintf_P(strRef4,PSTR("%s"),strSurcharge); 
		       break;
		  case PAY_VOUCHER://Load data Ref1=Voucher on cmVoucherInput
		  	   sprintf_P(strRef1,PSTR("%s"),strVoucherNum); 
		       break;
		  case PAY_PUMPTEST:
		  	   sprintf_P(strRef1,PSTR("PUMP TEST")); 
		  	   sprintf_P(strRef2,PSTR("%s"),strCardID); 	       
		       break;
		  }
		  leadingZero(MOPType,strPaymentType);
		  AddSpaceLead(strRef1,20);
		  AddSpaceLead(strRef2,20);
		  AddSpaceLead(strRef3,20);
		  AddSpaceLead(strRef4,20);
		  iWait=0;
		  IsMessage09=False;
		  stChangeMOP=cmSendMessage32;	      
	      break;
     case cmSendMessage32://SendMessage32
	      sendMessage32();
		  iLoop=0;
		  iPos=0;
		  lcd_printf(3,1,PSTR("Please Wait"));
		  stChangeMOP=cmWaitReplyMessage;	      
	      break;
     case cmWaitReplyMessage:
          iLoop++;
		  if ((iLoop%MSG_WAIT_TIMOUT)==0){
			 if (iPos<5){
				 lcd_xy(3,(13+iPos));_lcd('.');
				 iPos++;
				}
			 else{
				 iPos=0;
				 lcd_printf(3,(13+iPos),PSTR("       "));
				 //Resend Message32
				 if (iWait<5)stChangeMOP=cmSendMessage32;
				iWait++;
				}
		  }
		  if (iWait>5)stChangeMOP=cmNoReply;
		  if (IsMessage09==True)stChangeMOP=cmDisplayFreeMessage;
		  if (IsMessage99==True)stChangeMOP=cmFinishChangeMOP;
		  break;
     case cmDisplayFreeMessage:
	      IsMessage09=False;
	      msgResult=procMessage09();
		  lcd_clear();
		  lcd_print(1,1,strFreeMessageLine1);
		  lcd_print(2,1,strFreeMessageLine2);
		  lcd_print(3,1,strFreeMessageLine3);
		  lcd_print(4,1,strFreeMessageLine4);
		  TimDisplay=0;
          stChangeMOP=cmDelayMOP;
	      break;
     case cmNoReply:
	      lcd_clear();
		  lcd_printf(2,1,PSTR("No Reply    "));
	      lcd_printf(3,1,PSTR("TCP/IP Error"));
		  TimDisplay=0;
          stChangeMOP=cmDelayMOP;
	      break;
     case cmDelayMOP:
	      if (TimDisplay>9)stChangeMOP=cmExitChangeMOP;
	      break;
     case cmExitChangeMOP:
	      stChangeMOP=cmFinishChangeMOP;
	      break;
     case cmFinishChangeMOP:
	      stChangeMOP=cmInit;
		  Result=MENU_DONE;
	      break;
	 }
  return Result;
}


 char FViewFreeMessage(){
		static char stFreeMessage=fmInit;
		       char Result=MENU_NONE;
     
			 Result=MENU_NONE;
			 switch(stFreeMessage){
			 case fmInit:
			      stFreeMessage=fmDisplayFreeMessage;
			      break;
		     case fmDisplayFreeMessage:
				  lcd_clear();
				  lcd_print(1,1,strFreeMessageLine1);
				  lcd_print(2,1,strFreeMessageLine2);
				  lcd_print(3,1,strFreeMessageLine3);
				  lcd_print(4,1,strFreeMessageLine4);
			      TimDisplay=0;
			      stFreeMessage=fmDelayViewMesage;
			      break;
			 case fmDelayViewMesage:  
			      if (TimDisplay>TIM_FREE_MESSAGE)stFreeMessage=fmFinishFreeMessage;
			      break;
		     case fmFinishFreeMessage:
		          stFreeMessage=fmInit;
			      Result=MENU_DONE;
			      break;
			 }
		  return Result;
		}


