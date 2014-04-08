//#define DEBUG_PRINT_IDLE_STATE
//#define DEBUG_GENIUS_CODE
//#define DEBUG_FREM_ZERO_LEAD

#include <stdio.h>
#include <string.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <compat/deprecated.h>
#include <util/crc16.h>

#include "f_menu.h"
#include "LCD.h"
#include "KEYPAD.h"
#include "RTC.h"
#include "UART.h"
//#include "FAT32.h"
#include "SPI.h"

/*
union TCommonMessage{
	 struct TMessage99{
	      char strBalance[20];
		  char strOdometer[10];
	 }Message99;

	 struct TMessage09{
	      char strFreeMessageLine1[20];
		  char strFreeMessageLine2[20];
		  char strFreeMessageLine3[20];
		  char strFreeMessageLine4[20];
	 }Message09;

     }CommonUsed;
*/

//PORTB.4-->SPI SS TCP/IP
char PumpCountMax=0,ActivePump=0;
char IFType=IT_SLAVE;
char DispenserBrand=ST_NONE;
char IsNewPumpStatus=True;
char strPumpStatus[17]={"----------------"};
char CurrentPumpStatus[16]={0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0};
char PrintedStatus[16]={0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0};
char ReprintReady[16]={ 1,1,1,1,0,0,0,0,
                        0,0,0,0,0,0,0,0};

char IsReprintTicket=False;
char iSequencePooling=0,IsNewPoolingSequence=False;
volatile char IsStandaloneAcknoledge=False,IsPoolingRestarted=False,IsSetPumpType=True;

//Setting PumpPooling
char IsControlPooling=False;
char PoolCmd,PoolMsg;
char AcknoledgePump,AcknoledgeCommand;

// Config Protocol 
char IsConfigFlow=False;

//Standalone Totalizer Report
char strDeltaMoney[15],strDeltaVolume[15];
char strTotalMoney[20],strTotalVolume[20];
char IsGenerateReport=False;
char IsFinishPrintingTotalizer=False;

// Serial Buffer 620 + 36 = 656
unsigned char rcv_trans[620];
unsigned char MsgCode=MSG_NONE;
//Scan RFID Flow Data
char strSerialFlow[20];
char IsNewPacket=False;

unsigned int char_count=0;
unsigned int transLength=0,LengthMessage81=0;//,TimBackLight=0;
//char DataUSART0,DataUSART1,IsSPIBusy;

//[LocalAccount]
//FIP Fueling Status
char nLocalAccount=0;
char LocalAccountFIP[5];

//Tim System
char TimTicker=0,TimPressed=0;
volatile char TimDisplay=0;
//unsigned int iLoop=0;


//Display System *Incomplete
//char strShowMessage[20];
char IsShowMessage=False,IsClearMessage=False,IsWaitDisplay=False;
char IsViewFillingFIP=False;

//EDC System
char IsSendMessageEDC=False;
char IsRFIDDetected=False;
char IsEDCApproved=False,IsPrintApprovalCode=False,IsVoidTransaction=False;

//SystemDateTime
char strSystemDate[9],strSystemTime[9];
volatile char TimSend;

//Standalone Data
char CurrentShift;
//unsigned long Totalizer;
char strStandReceived[30];
char IsStandAloneDetected=False,IsStandaloneTrans=False;

//Message System
//char IsErrorTCPIP=False;
char IsMessage00=False,IsMessage03,IsMessage99=False,IsMessage11=False,IsMessage21=False,IsMessage23=False;
char IsMessage09=False,IsMessage10=False,IsMessage57=False,IsMessage81=False,IsMessage91=False;


char IsFreePrinting=False,IsCompleteFilling=False;
char IsAdvanzStartupInfo=0,IsNoTransaction=False;
char IsPrinting=False,IsBusyMsg11=False;

//char strIFT_ID[3],strSeqNum[3]; 
char SeqNum=0;

//char strClientIP[17],strServerIP[17];
char strReceiptNum[7];
char strProduct[15],strBalanceType[2],strBalanceCode[2];

//PrintGenerator
struct PumpData{
       char Grade;
	   char Nozzle;
	   char Shift;
	   char strTransNum[7];
	   char strTransDate[9];
	   char strTransTime[9];
       char Price[7];
       char Volume[9];
       char Money[9];
       }RecPumpData[8];//SLave1 standalone=8
                      //Pump Last/Now Grade [Data]:0xXX
       char EEMEM TotalVolume[2][8][6][6];
       char EEMEM TotalMoney[2][8][6][6];

	   char ShiftDateTime[20];

//GeniusProtocol
char IsTotalizerReceived=False;
char IncomingTransaction[2];
char ConfigCommand=CC_NONE;

//Printing System
char IsPrintERROR=False,IsBusyIdlePrinting=False;
volatile char IsBusyPrint=False,IsBusyFreePrinting=False;
char cmdPrint=0;
char TimPrintBusy=0;
char PrintBuffer[401];//[405];
char strPrint[45];

//MasterPassword
char strGeniusCode[11],strKeyStamp[11],strRestoreCode[11];
//Msg03
char strCardType[16],strAprovalCode[7],strInvoiceNumber[11],strApprovalCode[7],strDateTime[20];

//Msg57 
char NozzleID,CardType;
char strCompName[21],strLicPlate[11];

//Msg58  
char strBalanceValue[14],strOdometer[10];
char strNozzle[3],strPresetType[2];

//Msg32
char strPaymentType[3],strRef1[21],strRef2[21],strRef3[21],strRef4[21];
char strVoucherNum[21];

//Msg09
char strFreeMessageLine1[22],strFreeMessageLine2[22],strFreeMessageLine3[22],strFreeMessageLine4[22];

//MSG90
char EDCType;

//MSG91
char StatusEDC=0,strStatus[2],strSurcharge[10];

//Msg99 361 bytes
char strTranNo[7],strShift[2],strDate[11],strTime[9],strIslandID[3];
char strFIP_ID[3],strProductID[3],strDescription[16],strPrice[9],strVolume[9],strAmount[11];
char strMOPType[2],strMOPName[21],strCardID[21],strCardHolder[41],strBalanceTypePrint[26],strBalance[18];
char strMeterVolume[14],strMeterAmount[14],strCurrentTime[20],strPrintCount[3],strPrevPoints[9];
char strGainPoints[9],strLoyCardID[21],strLoyCardHolder[31],strLoyCurrentPoints[9];
char strLoyCurrMonConsumeA[11],strLoyCurrMonConsumeV[11],strSurchargeDesc[21],strSurchargeAmount[11];
char strLoyRedeemPoints[9],strLoyExpiry[11],strCorporateID[21],strCorporateName[31];
char MOPType=0,Shift=0;

//LocalAccount 1
unsigned char TimLocAcc=0;

//RFID 11
char strRFID[10];
//unsigned int iRFID=0;

//Beep 12
unsigned char ProcTimeOut=0,TimBeep=0;
unsigned char PumpID=0;

const unsigned char	__prodloc[6][2] PROGMEM = {{1, 6}, {2, 6}, {1, 13},
												{2, 13}, {1, 20}, {2, 20}};
const unsigned char	__prntloc[6][2] PROGMEM = {{1, 8}, {2, 8}, {3, 8}, {4, 8}, {1, 19},{2,19}};
const unsigned char	__prntlmt[6] PROGMEM = {2, 3, 4, 2, 15,15};
const unsigned char	__prntstr[6] PROGMEM = {1, 1, 0, 0, 0, 0};
const unsigned char	__hostloc[3][2] PROGMEM = {{1, 10}, {2, 10}, {3, 10}};


char EEMEM DefIFT_ID=1;

unsigned char	EEMEM DefSpvPassword[10] = {"11111"};
unsigned char	EEMEM DefSysPassword[10] = {"00000"};
unsigned char	EEMEM DefOperatorName[19] ={"  "};

         char	EEMEM DefClientIP[4] ={192,168,16,70};//{192,168,0,79};//{192,168,1,221}; //
         char	EEMEM DefServerIP[4] ={192,168,16,180};//{192,168,0,220};//{192,168,1,100}; //
                                           //0  1  2  3  4  5  6  7  8  9 
const unsigned char MaxKeyHit[10] PROGMEM = {2, 4, 3, 3, 3, 3, 3, 4, 3, 4};

unsigned char	EEMEM DefPrinterType =PT_CUSTOM_TG02;
unsigned char	EEMEM DefPrintScrollEnd = 7;//5;
unsigned char	EEMEM DefPrintScrollSpace = 5;//2;
unsigned char	EEMEM DefPrintAutoCut = 2;
unsigned char	EEMEM DefPrintLogo = 0;
unsigned char	EEMEM DefPrintSize = 3;

unsigned char	EEMEM DefConnectionHost = 0;
unsigned char	EEMEM DefShowDateTime = 1;
unsigned char	EEMEM DefNotifScreen = 1;

unsigned char	EEMEM DefDecimalPrice = 0;
unsigned char	EEMEM DefDecimalVolume = 3;
unsigned char	EEMEM DefDecimalMoney = 0;
unsigned char	EEMEM DefDecimalTotalVolume =2;
unsigned char	EEMEM DefDecimalTotalMoney = 0;
         char	EEMEM DefDecimalMark = ',';
         char	EEMEM DefCurrencyMark ='.';
		 char   EEMEM DefMarkMap[5]={',','.',' ','/','-'};

                      //FIP_ID 
unsigned char	EEMEM DefPumpMap[8]   = {1, 2, 0, 0, 0, 0, 0, 0};
unsigned char	EEMEM DefPumpLabel[8] = {1, 2, 0, 0, 0, 0, 0, 0};

                                        // N1 N2 N3 N4 N5 N6
unsigned char	EEMEM DefNozzleMap [8][6] = {{1, 0, 0, 0, 0, 0}, //FIP 01
                                             {1, 0, 0, 0, 0, 0}, //FIP 02
										     {0, 0, 0, 0, 0, 0}, //FIP 03
										     {0, 0, 0, 0, 0, 0}, //FIP 04
										     {0, 0, 0, 0, 0, 0}, //FIP 05
										     {0, 0, 0, 0, 0, 0}, //FIP 06
										     {0, 0, 0, 0, 0, 0}, //FIP 07
										     {0, 0, 0, 0, 0, 0}, //FIP 08
										     };


unsigned char	EEMEM DefProductPrice[6][9]={{"6500"},
                                             {"7250"},
											 {"4500"}, 
											 {"6500"},
											 {"6500"}, 
											 {"4500"}};

unsigned char	EEMEM DefProductName[6][13] ={{"Pert+   "}, 
                                              {"Pertamax"},
											  {"Premium "}, 
											  {"Solar   "},
											  {"BioSolr "},
											  {"Diesel  "},
											  };

         char	EEMEM DefBankName[4][11] ={{"BCA"},
                                   {"Mandiri"},
								   {"BNI"},
								   {"BRI"},
								   };
								   
         char EEMEM DefPrintInitialize=False;
		 char EEMEM DefInitIFT=IT_SLAVE;
		 char EEMEM DefDispenserBrand=ST_GILBARCO;//ST_WAYNE_DART;

		 char EEMEM DefTransactionNumber[7]={"000000"};
		 char EEMEM DefShift=1;
		 char EEMEM DefBaudrate[4]={br9600,br9600,br5787,br9600};

		 char EEMEM DefLastShiftDateTime[20];
		 char CurrentShiftDateTime[20];

         //PoolingPumpSetting
		 char EEMEM DefPoolingPumpMax=MAX_PUMP;
		 char EEMEM DefPoolingNoPumpCount=NO_PUMP_COUNT_MAX;
		 char EEMEM DefPoolingTryResend=TRY_RESEND;
		 char EEMEM DefPoolingSendTimeout=SEND_TIMEOUT;
		 char EEMEM DefPoolingDelayNextPump=DELAY_NEXT_PUMP;
		 char EEMEM DefActivePump=ACTIVE_PUMP;
		 char EEMEM DefSequenceTimeout=SEQUENCE_TIMEOUT;
										   
         char EEMEM DefLoyaltyLabel[11][20]={{"Card ID"},
		                                     {"Card Holder"},
											 {"Corp ID"},
											 {"Corp Name"},
											 {"Gain Points"},
											 {"Prev Points"},
											 {"Curr Points"},
											 {"Expiry"},
											 {"Total Redeem"},
											 {"Month Cons V"},
											 {"Month Cons A"}      
		                                    };

unsigned char	EEMEM DefHeaderFooter[10][41]={{"    PT. HANINDO AUTOMATION SOLUTIONS    "},//1
                                               {"        JL. RS Fatmawati No.55          "},//2
                                               {"            Jakarta Selatan             "},//3
                                               {"                                        "},//4
                                               {"                                        "},//5
                                               {"                                        "},//6
                                               {"             Terima Kasih               "},//1
                                               {"            Selamat  Jalan              "},//2
                                               {"      Semoga Selamat Sampai Tujuan      "},//3
                                               {"                                        "} //4                                              
											  };
         char EEMEM DefPrintMoney=True;
		 char EEMEM DefHGMode=HM_232;//HM_TTL,HM_232,HM_485

char strDispenserName1[] PROGMEM = "N/A";
char strDispenserName2[] PROGMEM = "Gilbarco";
char strDispenserName3[] PROGMEM = "Wayne DART";
char strDispenserName4[] PROGMEM = "Tatsuno";
char strDispenserName5[] PROGMEM = "LG";

PGM_P DefListDispenserName[] PROGMEM ={
      strDispenserName1,
	  strDispenserName2,
	  strDispenserName3,
	  strDispenserName4,
	  strDispenserName5,
};

ISR(TIMER1_COMPA_vect){//Timer Overflow 1ms
	if(__key_light == 1){
		__key_lgtcnt++;
		if(__key_lgtcnt == 150){
		   __key_light = 0; 
		   __key_lgtcnt = 0; 
		   sbi(PORTG, 1);
		   }
	}

	//Beep
	if (TimBeep>0){
	    TimBeep--;
		DDRB=(DDRB|0b00100000);
	    PORTB=(PORTB&0b11011111);
	    }
	else{PORTB=(PORTB|~PORTB);
		}
    //LocalAccount
	TimLocAcc++;
	//DisplaTicker
    TimTicker++;
	TimPressed++;
	if ((TimPressed%5)==0){
	    PORTE = PORTE^0x04;
	    TimDisplay++;
		TimPrintBusy++;
	}
	if ((TimPressed%PRESSED_DELAY)==0){
	     ProcTimeOut++;
	}
	TimSend++;
}

void FMenuIdle(){
	static char stMenuIdle=miInit,ButtonID=0;
	       char PrintStandaloneResult=PS_NONE;
	       char KeyPressed=0,KeyChar=0;
	       char lcdteks[20];

	switch(stMenuIdle){
	case miInit:
         DisplayScreenIdle();
		 stMenuIdle=miScan;		
	     break;
	case miScan:
         //Scan Key Pressed 
		 KeyPressed=_key_scan(1);
		 KeyChar= _key_btn(KeyPressed);       

		 switch(KeyPressed){
		 case _KEY_TIKET:system_beep(2);ButtonID=1;stMenuIdle=miDisplayProses;break;
		 case _KEY_MENU: system_beep(2);ButtonID=2;stMenuIdle=miDisplayProses;break;
		 case _KEY_SHIFT:system_beep(2);ButtonID=3;stMenuIdle=miDisplayProses;break;
		 case _KEY_CLEAR:system_beep(2);ButtonID=4;stMenuIdle=miDisplayProses;break;//stMenuIdle=miTestMsg56;break;//miDisplayProses;break;
		 case _KEY_0:    system_beep(2);           stMenuIdle=miRunTicket;    break;
	   //case _KEY_1:    system_beep(2);           stMenuIdle=miRunTotalizer; break;
		 case _KEY_2: if(IFType==IT_SLAVE) {system_beep(2); stMenuIdle=miRunEDC;}       break;
	   //case _KEY_3:    system_beep(2);           stMenuIdle=miClearTotalizer;break;
		 case _KEY_4: if(IFType==IT_SLAVE) {system_beep(2);stMenuIdle=miRunReprint;}   break;
		 case _KEY_5: if(IFType==IT_SLAVE) {system_beep(2);stMenuIdle=miRunLoyalty;}   break;
		 case _KEY_6: if(IFType==IT_SLAVE) {system_beep(2);stMenuIdle=miRunChangeMOP;} break;
		 //case _KEY_8:    system_beep(2);           stMenuIdle=miRunTestChar;   break;
		 case _KEY_9:    system_beep(2);stMenuIdle=miRunAuth;      break;
		 }
	     break;
    case miTestMsg56:
	     sprintf_P(strCardID,PSTR("4356A31A"));
	     sendMessage56();
         stMenuIdle=miDisplayProses;
	     break;
	case miDisplayProses:
	     if (IsBusyIdlePrinting==False){
		     lcd_printf(3,1,PSTR("SedangProses"));
	         TimDisplay=0;
			 stMenuIdle=miWaitProses;
         }else stMenuIdle=miScan;
	     break;
    case miWaitProses:
	     if (TimDisplay>1){
		     if (IFType==IT_SLAVE)stMenuIdle=miSendMessage98;
			 else
		     if (IFType==IT_STANDALONE)stMenuIdle=miPrintStandalone;
		 }
	     break;
    case miPrintStandalone:
	     PrintStandaloneResult=PrintStandalone(ButtonID,False);
	     if (PrintStandaloneResult==PS_PRINTED)stMenuIdle=miReady;
		 else
		 if (PrintStandaloneResult==PS_NO_DATA)stMenuIdle=miDisplayNoTransaction;
	     break;
    case miDisplayNoTransaction:
	     lcd_printf(3, 1,PSTR("Tidak Ada Transaksi "));
         TimDisplay=0;
		 stMenuIdle=miWaitDisplayNoTransaction;
	     break;
    case miWaitDisplayNoTransaction:
	     if (TimDisplay>1){
	         lcd_printf(3,1,PSTR("Ready...            "));
		     stMenuIdle=miWaitReady;
			 TimDisplay=0;
		 }
	     break;
    case miSendMessage98:
	     lcd_printf(3,1,PSTR("Please Wait..       "));
	     sendMessage98(ButtonID);
		 ProcTimeOut=0;
		 stMenuIdle=miWaitPlease;
	     break;
    case miWaitPlease:	     
         if (ProcTimeOut>TIM_NO_RESPONSE){
	         lcd_printf(3,1,PSTR("No Response..       "));
		     stMenuIdle=miNoResponse;
			 TimDisplay=0;		 
		 }
		 if ((IsMessage99==True)||(IsMessage00==True)){
		     ClearMem(strOdometer);
		     ClearMem(strLicPlate);
	         lcd_printf(3,1,PSTR("Ready...            "));
		     stMenuIdle=miWaitReady;
			 TimDisplay=0;
			 }
	     break;
    case miNoResponse:
         if (TimDisplay>1){
	         lcd_printf(3,1,PSTR("Ready...            "));
		     stMenuIdle=miWaitReady;
			 TimDisplay=0;
			 }	     
	     break;
    case miWaitReady:
         if (TimDisplay>1){
	         lcd_printf(3,1,PSTR("                "));
		     stMenuIdle=miReady;
			 TimDisplay=0;
			 }	     
	     break;
    case miReady:
	     DisplayScreenIdle();
		 if (IFType==IT_STANDALONE)IsNewPumpStatus=True;
		 stMenuIdle=miScan;
	     break;
	case miRunTicket://Slave,Standalone
		 if (FMenuTicket()==MENU_DONE)
		     stMenuIdle=miReady;
	     break;
	case miRunAuth://Slave,Standalone
		 if (FMenuAuthorization()==MENU_DONE)
		     stMenuIdle=miReady;
	     break;
		 
    case miRunLocalAccount://Slave
		 if (FMenuLocalAccount()==MENU_DONE)
		     stMenuIdle=miReady;
	     break;  
    case miRunChangeMOP://Slave
	     if (FMenuChangeMOP()==MENU_DONE)
		     stMenuIdle=miReady;
	     break;
    case miRunEDC://Slave
	     if (FMenuEDCTransaction()==MENU_DONE)
		     stMenuIdle=miReady;
         break;
    case miRunLoyalty://Slave
	     if (FMenuLoyalty()==MENU_DONE)
		     stMenuIdle=miReady;
	     break;
    case miRunReprint://Slave,Standalone
	     if (FMenuReprint()==MENU_DONE)
		     stMenuIdle=miReady;
	     break;
    case miRunViewFreeMessage://Slave
	     if (FViewFreeMessage()==MENU_DONE)
		     stMenuIdle=miReady;
	     break;
    case miClearTotalizer://Standalone
	     lcd_printf(3,1,PSTR("Clear Data Totalizer"));
	     ResetTotalizer(TOTALIZER_LAST);
         ResetTotalizer(TOTALIZER_NOW);
         stMenuIdle=miReady;	      
	     break;
    case miRunTotalizer://Standalone
	     lcd_printf(3,1,PSTR("TotalizerAll     "));
	     SendSlaveCommand(SC_TOTALIZER,PUMP_ALL);
		 stMenuIdle=miReady;	      
	     break;
    case miRunTestChar:	     
 	     if (FTestChar()==MENU_DONE)
         stMenuIdle=miReady;
	     break;
	}
//----------SYSTEM_PROC--------------------------------

        //Bank InfoUpdated
        if (IsMessage21==True){
		    IsMessage21=False;
			procMessage21();

        }
        //Transaction Status
        if (IsMessage00==True){
		    IsMessage00=False;
			if (procMessage00()==MSG00_NO_PRINT){
			    IsNoTransaction=True;
				}
		}
  
		//Send EDC Message Information;
		if (IsSendMessageEDC==True){
		    IsSendMessageEDC=False;
			SendEDCMessage();
		}
		if (IsEDCApproved==True){
		    IsEDCApproved=False;
            sendMessage92();
		}
		if (IsVoidTransaction==True){
		    IsVoidTransaction=False;
            sendMessage94();
		}


        //Message99 Detection 
        if (IsMessage99==True){
	        IsMessage99=False;
		    procMessage99();
		    IsPrinting=True;
		}
		//No Transaction
		if (IsNoTransaction==True){
			IsNoTransaction=False;
			lcd_printf(3, 1,PSTR("Tidak Ada Transaksi "));
		}
        
		//Display Idle
		if (stMenuIdle==miScan){//||(stMenuIdle=miWaitPlease)){ 
		    DisplayIdle();			
			}        

        //Display FreeMessage
		if (stMenuIdle==miScan){ 
		    if (IsMessage09==True){
			    IsMessage09=False;
				procMessage09();
				stMenuIdle=miRunViewFreeMessage;
			   }
			}

        //LocalAccount Scanning
		if ((stMenuIdle==miScan)&&(IsRFIDDetected==True)){
		    IsRFIDDetected=False;
		    system_beep(2);           
			stMenuIdle=miRunLocalAccount;
		}
        //Print Spooling
		if (IsMessage81==True){
		    IsMessage81=False;
			//Spooling HFCS 0000 : Header, Footer, Copy , Scrool [Copy:16x max]
			cmdPrint=procMessage81();
			IsFreePrinting=True;
         }

    //SystemService
      systemGenerateReport();
      systemPrinting();
	  systemEDC();
	  systemConfigProtocol();
	  //systemGeniusProtocol(); //AKR-->Protocol disabled
}

void StoreStandaloneTotalizerData(char *strRawTransData){//Sending FlowSPI_Protocol <STX>   [MsgID][PumpID][Volume1][Amount1][Volume2][Amount2][Volume3][Amount3][Volume4][Amount4][Volume5][Amount5][Volume6][Amount6]<ETX>
	                                                 //                           [0x50,0x05]  02     01    0000000  0000000  0000000  0000000  0000200  0000000  0000200  0000000  0000000  0000000 [0x06,0x60] = 4+8*2*6=76
     char strPumpID[3],iPumpID,iGrade;
	 char strGVolume[15],strGMoney[15];
	 char strSend[60];
	 int i;
	 char FIPAddr;

	 //for(i=0;i<strlen(strRawTransData);i++){
	 //    uart(1,1,strRawTransData[i]); 
	 // }

     StrPosCopy(strRawTransData,strPumpID,2,2);
	 //IdIFT(strPumpID);
	 iPumpID=atoi(strPumpID);  
	 
	 FIPAddr=GetFIPAddr(iPumpID);
	 if (FIPAddr>0){
	     FIPAddr=FIPAddr-1;
		 //uart_printf(1,1,PSTR("Totalizer:"));

		 for (iGrade=1;iGrade<=6;iGrade++){          
			  StrPosCopy(strRawTransData,strGVolume,(4+((iGrade-1)*24)),12);
			  StrPosCopy(strRawTransData,strGMoney,(16+((iGrade-1)*24)),12);
			  
			  SetTotalizerData(TVOLUME,TOTALIZER_NOW,FIPAddr,iGrade,strGVolume);
			  SetTotalizerData(TMONEY,TOTALIZER_NOW,FIPAddr,iGrade,strGMoney);
			  //sprintf_P(strSend,PSTR("Nozzle:%d Volume:%s Money:%s"),iGrade,strGVolume,strGMoney);
			  //uart_print(0,1,strSend);
		 }	
	}
     SendSlaveCommand(SC_TOTALIZER_ACK,iPumpID);
	 UpdateStandaloneStatus((iPumpID&0x0F),PS_TOTALIZER);	 
}
         
			                                         //      STX     MSGID  PumpID              6        8        8
void StoreStandaloneTransData(char *strRawTransData){//  [0x50,0x05]  01      01      1       004500   00000200  00000012 [0x06,0x60] = 24
     char iPumpID=0,strPumpID[3];
	 char PNozzle,PGrade,strPPU[7],strPVolume[10],strPMoney[10];
	 char TransactionStatus=TS_NONE;
	 char strTransDate[9],strTransTime[9];
	 char strSend[30];
	 char TransDigit;
	 char FIPAddr;
      
	 StrPosCopy(strRawTransData,strPumpID,2,2);
	 RemZeroLead(strPumpID);
	 iPumpID=atoi(strPumpID); 
	 
	 FIPAddr=GetFIPAddr(iPumpID);
	 if (FIPAddr>0){
	     FIPAddr=FIPAddr-1;
	     //01 01 1 1 004500 0000055 0002475 
		 PNozzle=CharPosCopy(strRawTransData,4)-'0';
		 PGrade=CharPosCopy(strRawTransData,5)-'0';

		 FillChar(strPPU,sizeof(strPPU),0);
		 FillChar(strPVolume,sizeof(strPVolume),0);
		 FillChar(strPMoney,sizeof(strPMoney),0);

		 StrPosCopy(strRawTransData,strPPU,6,6);	 
		 StrPosCopy(strRawTransData,strPVolume,12,8);
		 StrPosCopy(strRawTransData,strPMoney,20,8);

		 TransDigit=Ord(CharPosCopy(strRawTransData,28));
		 //Normalize7Digit
		 //sprintf_P(strSend,PSTR("Digit: %d"),TransDigit);
		 //uart_print(1,1,strSend);
		 if  (TransDigit==8){
			  StrPosCopy(strPMoney,strPMoney,0,strlen(strPMoney)-1);	 
		 }
		 RemZeroLead(strPPU);
		 RemZeroLead(strPVolume);
		 RemZeroLead(strPMoney);
		 
		 //sprintf_P(strSend,PSTR("Money: %s"),strPMoney);
		 //uart_print(1,1,strSend);


		 //Void Detection
		 if (atoi(strPMoney)==0)TransactionStatus=TS_VOID;
		 else TransactionStatus=TS_NEW;
		 
		 if (TransactionStatus==TS_VOID){//RejectData
			 UpdateStandaloneStatus((iPumpID&0x0F),PS_VOID);
		 }
		 else 
		 if (TransactionStatus=TS_NEW){ 
			 //Update ReprintReady
			 ReprintReady[FIPAddr]=True;//Ready for Printing
			  
			 FormatPrice(strPPU);
			 FormatMoney(strPMoney);
			 FormatVolume(strPVolume);

			 //uart_print(1,1,strPVolume);
			 
			 GenerateTransactionNum(strTranNo);		 
			 sprintf_P(strTransDate,PSTR("%s"),strSystemDate);
			 sprintf_P(strTransTime,PSTR("%s"),strSystemTime);
			 sprintf_P(strShift,PSTR("%d"),eeprom_read_byte(&DefShift));
					 
			 //TransNum,Date,Time
			 StrPosCopy(strTranNo,RecPumpData[FIPAddr].strTransNum,0,strlen(strTranNo));
			 StrPosCopy(strTransDate,RecPumpData[FIPAddr].strTransDate,0,strlen(strTransDate));
			 StrPosCopy(strTransTime,RecPumpData[FIPAddr].strTransTime,0,strlen(strTransTime));
			 //Grade Nozzle Shift
			 RecPumpData[FIPAddr].Grade=PGrade;
			 RecPumpData[FIPAddr].Nozzle=PNozzle;
			 RecPumpData[FIPAddr].Shift=atoi(strShift);
			 //Price Volume Money
			 StrPosCopy(strPPU,RecPumpData[FIPAddr].Price,0,strlen(strPPU));
			 StrPosCopy(strPVolume,RecPumpData[FIPAddr].Volume,0,strlen(strPVolume));
			 StrPosCopy(strPMoney,RecPumpData[FIPAddr].Money,0,strlen(strPMoney));

			 UpdateStandaloneStatus((iPumpID&0x0F),PS_PRINT_READY);
			 
			 SetIncomingTransStatus(iPumpID,TS_NEW);		 

			 SendSlaveCommand(SC_TRANSACTION_ACK,iPumpID);
		 }		 
	 }    
     //uart_print(0,1,strRawTransData);
}


char RePrintStandalone(char FIPAddr){
     char Result=PS_NONE;

   return Result;
}

char PrintStandalone(char FIPAddr,char IsReprint){
     char iPumpID,Result=PS_NONE;
	 char PProductID[6];//,PPumpID[8];
	 char LFIPAddr;
	      
	 Result=PS_NO_DATA;

//	 eeprom_read_block((void*) &PPumpID, (const void*) &DefPumpMap, 8);
	 iPumpID=GetPumpID(FIPAddr);//PPumpID[FIPAddr-1];
     LFIPAddr=GetFIPAddr(iPumpID);

     if (LFIPAddr>0){
	     LFIPAddr=LFIPAddr-1;
	     if ((IsReprint==True)||(iPumpID>0)&&(strPumpStatus[LFIPAddr]==GetPumpStatusLabel(PS_PRINT_READY))){    	 
		     eeprom_read_block((void*) &PProductID, (const void*) &DefNozzleMap[FIPAddr-1], 6);		 
		     if (((ReprintReady[FIPAddr-1]==True))||(IsReprint==False)){
			       if (IsReprint==True)IsReprintTicket=True;
			       GenerateStandaloneTransData(iPumpID,PProductID);		 			   
	               Result=PS_PRINTED;
			 }//else Result=PS_NO_DATA;
		 }
		 //else Result=PS_NO_DATA;    		    
	 }	
   return Result;
}

void GenerateTransactionNum(char *sTransNumber){//Create and Save TransactionNumber to EEPROM
     char i,PTransNum[7],cNum,xNum=0,xAdd=0,Length=0;
	 FillChar(PTransNum,sizeof(PTransNum),0);//"999999"->"000000"
     eeprom_read_block((void*) &PTransNum, (const void*) &DefTransactionNumber,sizeof(DefTransactionNumber));
     xAdd=1;
	 xNum=0;
	 Length=strlen(PTransNum);

	 for(i=0;i<Length;i++){//[000009] 999999 123456
	     xNum=PTransNum[Length-i-1]-'0';		 
		 cNum='0'+((xNum+xAdd)%10);
		 xAdd=((xNum+xAdd)/10);
		 PTransNum[Length-i-1]=cNum;
		 sTransNumber[Length-i-1]=cNum;
	 }
	 sTransNumber[Length]=0;
	 PTransNum[Length]=0;

     eeprom_write_block((const void*)&PTransNum,(void*)&DefTransactionNumber,sizeof(DefTransactionNumber));
}


char GenerateStandaloneTransData(char xPumpID, char *PNozzle){//[1,2,3..16]->[1,2,3..0]
	 char ProductName[15];//,strPPU[7],strPVolume[8],strPMoney[8];
     char iPumpID,iProdID=0,TermID;//,PPumpID[8];
     char Result=GS_NONE;
	 char strSend[30];
	 char FIPAddr;

	 iPumpID=(xPumpID&0x0F);
	 FIPAddr=GetFIPAddr(iPumpID);
     if (FIPAddr>0){
	     FIPAddr=FIPAddr-1;
		 iProdID=PNozzle[RecPumpData[FIPAddr].Grade];

		 if (iProdID>0) eeprom_read_block((void*) &ProductName, (const void*) &DefProductName[iProdID-1],sizeof(DefProductName[iProdID-1]));
		 else sprintf_P(ProductName,PSTR("N/A"));

		 TermID=eeprom_read_byte(&DefIFT_ID);

		 //Shift,TransNum,Date,Time
		 sprintf_P(strShift,PSTR("%d"),RecPumpData[FIPAddr].Shift);
		 sprintf_P(strTranNo,PSTR("%s"),RecPumpData[FIPAddr].strTransNum);
	     sprintf_P(strDate,PSTR("%s"),RecPumpData[FIPAddr].strTransDate);
		 sprintf_P(strTime,PSTR("%s"),RecPumpData[FIPAddr].strTransTime);	 

	     //TermID,FIP_ID,Product
		 sprintf_P(strIslandID,PSTR("%d"),TermID);
		 sprintf_P(strFIP_ID,PSTR("%.2d"),xPumpID);
	     sprintf_P(strDescription,PSTR("%s"),ProductName);

		 //Price Volume Money
		 sprintf_P(strPrice,PSTR("%s"),RecPumpData[FIPAddr].Price);
	     sprintf_P(strVolume,PSTR("%s"),RecPumpData[FIPAddr].Volume);
		 sprintf_P(strAmount,PSTR("%s"),RecPumpData[FIPAddr].Money);

		 MOPType=MOP_CASH;
	     IsPrinting=True;	 
		 Result=GS_GENERATED;
	 }
	 //ProductName	 
	 //iProdID=PNozzle[RecPumpData[iPumpID].Nozzle];
	 //BAsedOnGrade
   return Result;
}


char GetPumpStatusLabel(char xPumpStatus){
     char Result;

	 switch(xPumpStatus){
	 case PUMP_ERROR: 
	      Result='E';
		  break;
	 case PUMP_ST1: 
	      Result='1';
		  break;
	 case PUMP_ST2: 
	 	  Result='2';
		  break;
     case PUMP_ST3:
	 	  Result='3';
		  break;	
	 case PUMP_ST4: 
          Result='4';
		  break;
	 case PUMP_ST5: 
	 	  Result='5';
		  break;
	 case PUMP_OFF: 
	 case PW_ONLINE:
	 	  Result='i';
		  break;
     case PW_CALL:
	 case PUMP_CALL: 
	 	  Result='n';
		  break;     
	 case PUMP_AUTH: 
	 	  Result='a';
		  break;
     case PW_AUTHORIZED:
	 case PUMP_BUSY: 
	 	  Result='d';
		  break;
	 case PUMP_PEOT: 
	 	  Result='O';
		  break;
     case PW_END_DELIVERY:
	 case PUMP_FEOT: 
	 	  Result='F';
		  break;
	 case PUMP_STOP: 
	 	  Result='S';
		  break;
	 case PUMP_NONE:
	 case PW_DISCONNECT:
	 	  Result='-';
		  break;
     case PS_PRINT_READY:
	      Result='P';
 	      break;
     case PS_PRINTED:
	      Result='I';
 	      break;
     case PS_VOID:
	      Result='V';
	      break;
     case PS_TOTALIZER:
	      Result='T';
	      break;
     case PS_FINISH_TOTALIZER:
	      Result='t';
	      break;

	 default:
          Result='x';
		  break;         	 
	 }
  return Result;	 
}

void DisplayPumpStatus(){
     char SPump[20],lcdteks[20];

	 if (IsNewPumpStatus==True){
	     IsNewPumpStatus=False;
		 BackLightTrig();
		 StrPosCopy(strPumpStatus,SPump,0,PumpCountMax);
		 sprintf_P(lcdteks,PSTR("%s"),SPump);
         lcd_print(4,1,lcdteks);
     	 }
}

void UpdateStandaloneStatus(char xPumpID,char xPumpStatus){//xPumpID: (1-16)&0x0F
char strSend[20],iPumpID; //[xxxxxxxxxPx]
char iAddr;

	 if (xPumpID<16){
	     DispenserBrand=eeprom_read_byte(&DefDispenserBrand);
	     switch(DispenserBrand){
	     case ST_GILBARCO:	
		      /*     
			  if (xPumpID>0)
			      iPumpID=xPumpID-1;
			  else iPumpID=15; 
			  */
			  iPumpID=xPumpID; 
	          break;
	     case ST_WAYNE_DART:
              iPumpID=xPumpID;
	          break;				   
	     }

		 iAddr=GetFIPAddr(iPumpID);
		 if (iAddr>0){
		     iAddr=iAddr-1;
			 if (strPumpStatus[iAddr]!=GetPumpStatusLabel(PS_PRINT_READY)){//iFdiiiiiiiiiiiii
		         if (xPumpStatus!=PS_PRINT_READY)
				     CurrentPumpStatus[iAddr]=xPumpStatus;
			     strPumpStatus[iAddr]=GetPumpStatusLabel(xPumpStatus);
                 if (xPumpStatus==PS_FINISH_TOTALIZER)
				     strPumpStatus[iAddr]=GetPumpStatusLabel(CurrentPumpStatus[iAddr]);
		     }else if ((strPumpStatus[iAddr]==GetPumpStatusLabel(PS_PRINT_READY))||(xPumpStatus==PUMP_FEOT)||(xPumpStatus==PS_PRINTED)||(xPumpStatus==PS_TOTALIZER)){//P
			     if (xPumpStatus==PS_PRINTED){
				 //New

				     //if (CurrentPumpStatus[iAddr]!=GetPumpStatusLabel(PS_PRINT_READY))
			             strPumpStatus[iAddr]=GetPumpStatusLabel(CurrentPumpStatus[iAddr]);
					 //else {
					 //    CurrentPumpStatus[iAddr]=PUMP_OFF;
					 //    strPumpStatus[iAddr]=GetPumpStatusLabel(PUMP_OFF);
					 //}

				  }
			     else CurrentPumpStatus[iAddr]=xPumpStatus;			 
			 if (xPumpStatus==PUMP_FEOT){
			     CurrentPumpStatus[iAddr]=xPumpStatus;
			     strPumpStatus[iAddr]=GetPumpStatusLabel(CurrentPumpStatus[iAddr]);
				 }
             else if (xPumpStatus==PS_TOTALIZER){
			      strPumpStatus[iAddr]=GetPumpStatusLabel(xPumpStatus);
				 }
		 }		 	     
         strPumpStatus[16]=0;
	     IsNewPumpStatus=True;		 
		 }		 
	 }
}

void SendPoolingCommand(char plCmd,char plMsg){
	 PoolCmd=plCmd;
	 PoolMsg=plMsg;
     IsControlPooling=True;
}
void SendSlaveCommand(char SlaveCommand,char SlaveMessage){     
	 systemMaster();
	 _spi(0x05);
	 _spi(SlaveCommand);
	 _spi(SlaveMessage);
	 _spi(0x06);
	 systemSlave();
}

void ScanStandaloneFlow(char data){  //<STX>[PumpID][STA]<ETX> --> 0x07 0xF1 0xF2 0x08
//static char IsStandaloneStatus=False;//<STX>[PumpID][ProductID][UnitPrice][Volume][Amount] <ETX>
//static char IsStandaloneTransInfo=False;
static char zDataFlow[5];
static unsigned int iFlow=0;
       char MixData,DataPumpID,DataPumpStatus;
	   char strSend[10],MsgIDx=0;

      DataPumpID=0;
      DataPumpStatus=0;
	  MixData=0;
      //Shift data
      zDataFlow[4]=zDataFlow[3];
	  zDataFlow[3]=zDataFlow[2];
	  zDataFlow[2]=zDataFlow[1];
      zDataFlow[1]=zDataFlow[0];
      zDataFlow[0]=data;

      //Scan UpdatedPumpStatus Flow
	  //<STX>[ID][STA]<ETX>: 0x07[ID STA]0x08
	  if((zDataFlow[2]==0x07)&&(zDataFlow[0]==0x08)){
          MixData=zDataFlow[1];
	      DataPumpID=((MixData&0xF0)>>4);
		  DataPumpStatus=(MixData&0x0F);
		  UpdateStandaloneStatus(DataPumpID,DataPumpStatus);
		  if (DataPumpStatus!=PUMP_NONE)IsStandAloneDetected=True;
		  //sprintf_P(strSend,PSTR("PS:%d Id:%d"),MixData,DataPumpID);
		  //uart_print(0,1,strSend);
	  }

	  //Scan Transaction Completion FEOT
	  if (IsStandaloneTrans==True){
	      strStandReceived[iFlow]=data;
		  iFlow++;
	  }
	  if((zDataFlow[1]==0x05)&&(zDataFlow[0]==0x50)){
	      IsStandaloneTrans=True;
		  iFlow=0;
	  }
	  if((zDataFlow[1]==0x06)&&(zDataFlow[0]==0x60)){
	      IsStandaloneTrans=False;
	      strStandReceived[iFlow]=0;

		  //uart_print(1,1,strStandReceived);

		  MsgIDx=GetMessageID(strStandReceived);
          if (MsgIDx==0x01)StoreStandaloneTransData(strStandReceived);
		  else
          if (MsgIDx==0x02)StoreStandaloneTotalizerData(strStandReceived);
	  }
	  //AcknoledgeCommand
	  if ((zDataFlow[3]==0x09)&&(zDataFlow[0]==0x0A)){
	      
	      AcknoledgePump=zDataFlow[1];
	      AcknoledgeCommand=zDataFlow[2];	  
		  switch(AcknoledgeCommand){
		  case SC_LIVE_SEQUENCE:
		       iSequencePooling=AcknoledgePump;
			   IsNewPoolingSequence=True;
			   //SendPoolingCommand()
			   if (IsControlPooling==True){
			       IsControlPooling=False;
				   SendSlaveCommand(PoolCmd,PoolMsg);
                }
		       break;
          case SC_TOTALIZER:
		       IsTotalizerReceived=True;
		       break;
		  case SC_POOL_RESTARTED:
		       IsPoolingRestarted=True;
		       break;
          case SC_SET_PUMP_TYPE:
		       IsSetPumpType=True;
		       break;
		  }
       IsStandaloneAcknoledge=True;
	  }
}

char GetMessageID(char *strMessageFlow){
     char Result,strMsgID[3];
	 StrPosCopy(strMessageFlow,strMsgID,0,2);
	 RemZeroLead(strMsgID);
	 Result=atoi(strMsgID);
  return Result;
}

void ScanEDCFlow(char data){
static char IsEDCFlow=False;
static unsigned int nFlow=0;
     char EDCMsgCode=0; 
     char i;


	 //uart(0,1,data);

     //EDC-MSG
     if (data==0x02){
	     IsEDCFlow=True;
		 nFlow=0;
	 }//FillMessage
     if (IsEDCFlow==True){
	     rcv_trans[nFlow]=data;
		 nFlow++;
	 }//CloseMessage
	 if ((data==0x03)||(nFlow>=MSG03_LENGTH)){
	     //for(i=0;i<nFlow;i++){uart(0,1,rcv_trans[i]);}
	     IsEDCFlow=False;
		 EDCMsgCode=((rcv_trans[1]-'0')*10)+(rcv_trans[2]-'0');

		 if (EDCMsgCode==0x03){
	         StrPosCopy(rcv_trans,strTranNo,3,6);
             StrPosCopy(rcv_trans,strFIP_ID,9,2);
             StrPosCopy(rcv_trans,strCardType,11,15);
             StrPosCopy(rcv_trans,strCardID,26,19);
             StrPosCopy(rcv_trans,strApprovalCode,45,6);
             StrPosCopy(rcv_trans,strInvoiceNumber,51,10);
             StrPosCopy(rcv_trans,strDateTime,61,14);
		     IsEDCApproved=True;//Sending Message 92
			 IsPrintApprovalCode=True;
	    }else
		 if (EDCMsgCode==0x04){
             StrPosCopy(rcv_trans,strApprovalCode,3,6);
             StrPosCopy(rcv_trans,strInvoiceNumber,9,10);
             StrPosCopy(rcv_trans,strDateTime,19,14);
			 IsVoidTransaction=True;
	    }
    }//MSGDetection
}

void ScanHiddenKeyFlow(char KeyIn){
}


void ScanRFIDFlow(char data){//                   12 11 10 9  8  7  6  5  4  3  2  1  0
     char i=0,strSerial[20]; //Shift Data Serial; 02 32 33 44 31 30 41 36 43 0D 0A 0A 03 
	                                                //2 50 51 65 68 68 68 54 67 13 10 3

	 for(i=0;i<19;i++){
        strSerialFlow[19-i]=strSerialFlow[18-i];   
	 }  strSerialFlow[0]=data;
	 //Detection
	 if ((strSerialFlow[11]==0x02)&&(strSerialFlow[1]==0x0A)&&(strSerialFlow[0]==0x03)){
		for(i=0;i<8;i++) strRFID[i]=strSerialFlow[10-i];
        strRFID[8]=0;
	    IsRFIDDetected=True;
		BackLightTrig();
        //sprintf_P(strSerial,PSTR("RFID:%s"),strRFID);
        //uart_print(0,1,strSerial);
	 }
}

void IdentifyMessage(char STX,unsigned int Length){
   if (STX==0x01){
	    if ((Length==MSG00_LENGTH)||(MsgCode==0)) IsMessage00=True;//47
		else
	    if ((Length==MSG09_LENGTH)||(MsgCode==9)) IsMessage09=True;//615
		else
	    if ((Length==MSG11_LENGTH)||(MsgCode==11)) IsMessage11=True;//615
		else
	    if ((Length==MSG23_LENGTH)||(MsgCode==23)) IsMessage23=True;//145
		else
	    if ((Length==MSG57_LENGTH)||(MsgCode==57)) IsMessage57=True;//230
		else
	    if ((Length==MSG99_LENGTH)||(MsgCode==99)) IsMessage99=True;//378
		else
	    if ((Length==MSG81_LENGTH)||(MsgCode==81)) IsMessage81=True;//426
		else
	    if ((Length==MSG91_LENGTH)||(MsgCode==91)) IsMessage91=True;//426
		else
	    if ((Length==MSG21_LENGTH)||(MsgCode==21)) IsMessage21=True;//426
        //EDC AdvanZ Respond
		if (IsMessage91==True){
		    IsMessage91=False;
		    ProcMessage91();
			IsSendMessageEDC=True;
		  }

//	  sprintf(SerialSend,"Length:%i",Length);	 
//	  uart_print(1,0,SerialSend);
	 }
}

ISR(SPI_STC_vect){
char dataSPI;
//char Reply=0;
    //uart(0,1,SPDR);
	dataSPI=SPDR;
	ScanRFIDFlow(dataSPI);
	if (IFType==IT_SLAVE)ScanEDCFlow(dataSPI);
	else
	if (IFType==IT_STANDALONE)ScanStandaloneFlow(dataSPI);
}

ISR(USART0_RX_vect){
	char dataRX0,IsSPI=False;
	dataRX0= UDR0;
	//PrintBusyDetection
	if ((IsBusyIdlePrinting==True)||(IsBusyFreePrinting==True)){
	    if (dataRX0==19)IsBusyPrint=True;
		else 
	    if (dataRX0==17)IsBusyPrint=False;	
	}
}


char ScanCommand(char *strFlow,char *fCommand){
     char Result;
	 Result=False;
   return Result;
}

void SaveConfigParameter(){
char i,j;
char strEEPROM[50],xEEPROM;
unsigned int StrPos;
char strSend[20];
     
	 sprintf_P(strSend,PSTR("Length:%d"),transLength);
	 uart_print(1,1,strSend);

     StrPos=0;
	 uart_printf(1,1,PSTR("<Saving>")); 
     //HeaderFooter
	 for(i=0;i<10;i++){
         FillChar(strEEPROM,0,sizeof(strEEPROM));   
		 StrPosCopy(rcv_trans,strEEPROM,(StrPos+(40*i)),40);
		 strEEPROM[40]=0;
		 eeprom_write_block((const void*) &strEEPROM, (void*) &DefHeaderFooter[i],41);
		 //uart_print(1,1,strEEPROM);
	 }
	 //ProductName
	 StrPos=(40*10);
	 for(i=0;i<6;i++){
         FillChar(strEEPROM,0,sizeof(strEEPROM));   
		 StrPosCopy(rcv_trans,strEEPROM,(StrPos+(12*i)),12);
		 RemSpaceLag(strEEPROM);
		 strEEPROM[strlen(strEEPROM)]=0;
		 eeprom_write_block((const void*) &strEEPROM, (void*) &DefProductName[i],sizeof(DefProductName[i]));
		 //eeprom_write_byte(DefProductName[i][12],0);		 
		// uart_print(1,1,strEEPROM);

	 }    
	 //ProductPrice
	 StrPos=(40*10)+(12*6);
	 for(i=0;i<6;i++){
         FillChar(strEEPROM,0,sizeof(strEEPROM));   
		 StrPosCopy(rcv_trans,strEEPROM,(StrPos+(5*i)),5);
		 RemSpaceLag(strEEPROM);
		 eeprom_write_block((const void*) &strEEPROM, (void*) &DefProductPrice[i],sizeof(DefProductPrice[i]));
		 //eeprom_write_byte(DefProductPrice[i][8],0);		 
		// uart_print(1,1,strEEPROM);

	 }    
	 //PumpID Config
	 StrPos=(40*10)+(12*6)+(5*6);
	 for(i=0;i<8;i++){
         FillChar(strEEPROM,0,sizeof(strEEPROM));   
		 StrPosCopy(rcv_trans,strEEPROM,(StrPos+(2*i)),2);
		 RemZeroLead(strEEPROM);
		 xEEPROM=atoi(strEEPROM);
         eeprom_write_byte(&DefPumpMap[i],xEEPROM);
		 sprintf_P(strSend,PSTR("%d"),xEEPROM);
		// uart_print(1,1,strSend); 

	 }    
	 //PumpNozzle
	 StrPos=(40*10)+(12*6)+(5*6)+(2*8);
	 for (i=0;i<8;i++){
	     for (j=0;j<6;j++){
              xEEPROM=Ord(CharPosCopy(rcv_trans,StrPos+(i*6+j)));
		      eeprom_write_byte(&DefNozzleMap[i][j],xEEPROM);
       //       uart(1,1,Chr(xEEPROM));
		 }	 
	 }
	 //PrintNoMoney
	 StrPos=(40*10)+(12*6)+(5*6)+(2*8)+(6*8);
	 xEEPROM=Ord(CharPosCopy(rcv_trans,StrPos));
	 if (xEEPROM==0)xEEPROM=False;
	 else xEEPROM=True;
	 eeprom_write_byte(&DefPrintMoney,xEEPROM);
//	 uart(1,1,Chr(xEEPROM));

	 //TermID
	 StrPos=(40*10)+(12*6)+(5*6)+(2*8)+(6*8)+1;
     FillChar(strEEPROM,0,sizeof(strEEPROM));   
	 StrPosCopy(rcv_trans,strEEPROM,StrPos,2);
	 RemZeroLead(strEEPROM);
	 xEEPROM=atoi(strEEPROM);
	 eeprom_write_byte(&DefIFT_ID,xEEPROM);
	 sprintf_P(strSend,PSTR("%d"),xEEPROM);
//	 uart_print(1,1,strSend); 

	 //PumpLabel
	 StrPos=(40*10)+(12*6)+(5*6)+(2*8)+(6*8)+1+2;
	 for(i=0;i<8;i++){
         FillChar(strEEPROM,0,sizeof(strEEPROM));   
		 StrPosCopy(rcv_trans,strEEPROM,(StrPos+(2*i)),2);
		 RemZeroLead(strEEPROM);
		 xEEPROM=atoi(strEEPROM);
         eeprom_write_byte(&DefPumpLabel[i],xEEPROM);
		 sprintf_P(strSend,PSTR("%d"),xEEPROM);
//		 uart_print(1,1,strSend); 
	 }    
	 uart_printf(1,1,PSTR("<OK>")); 
}



void systemConfigProtocol(){
     static char stConfigProtocol=cpWaitSend;
	 switch(stConfigProtocol){
	 case cpWaitSend:
	      switch(ConfigCommand){
		  case CC_SEND_CONFIG:
		       ConfigCommand=CC_NONE;
			   stConfigProtocol=cpSendingParameter;
		       break;
		  case CC_SAVE_CONFIG:
		       ConfigCommand=CC_NONE;
			   stConfigProtocol=cpSavingParameter;
		       break;
          default:
		       ConfigCommand=CC_NONE;
		       break;
		  }
	      break;
     case cpSendingParameter:
	      SendConfigParamater();
          stConfigProtocol=cpWaitSend;
	      break;
     case cpSavingParameter:
          SaveConfigParameter();
          stConfigProtocol=cpWaitSend;
	      break;
	 }
}

void SendConfigParamater(){
char i,j,strSend[60];
char strEEPROM[41],xEEPROM;

     //Start
	 uart_printf(1,0,PSTR("<"));
     //Header Footer 400
	 for (i=0;i<10;i++){
	      eeprom_read_block((void*) &strEEPROM, (const void*) &DefHeaderFooter[i],sizeof(DefHeaderFooter[i]));
		  FillChar(strSend,0,sizeof(strSend));
	      sprintf_P(strSend,PSTR("%s"),strEEPROM);
		  AddSpaceLag(strSend,40);
		  uart_print(1,0,strSend);
	 }
	 //ProductName 72 
	 for (i=0;i<6;i++){
		 eeprom_read_block((void*) &strEEPROM, (const void*) &DefProductName[i],sizeof(DefProductName[i]));
		 FillChar(strSend,0,sizeof(strSend));
	     sprintf_P(strSend,PSTR("%s"),strEEPROM);
		 AddSpaceLag(strSend,12);
		 uart_print(1,0,strSend);
	 }
	 //ProductPrice 30
	 for (i=0;i<6;i++){
		 eeprom_read_block((void*) &strEEPROM, (const void*) &DefProductPrice[i],sizeof(DefProductPrice[i]));
		 FillChar(strSend,0,sizeof(strSend));
	     sprintf_P(strSend,PSTR("%s"),strEEPROM);
		 AddSpaceLag(strSend,5);
		 uart_print(1,0,strSend);
	 }
	 //PumpID Config 8
	 for (i=0;i<8;i++){
	      xEEPROM=eeprom_read_byte(&DefPumpMap[i]);
		  if (xEEPROM>=100)xEEPROM=0;
		  FillChar(strSend,0,sizeof(strSend));
	      sprintf_P(strSend,PSTR("%.2d"),xEEPROM);
		  AddSpaceLag(strSend,2);
		 uart_print(1,0,strSend);
	 }
	 //PumpID NozzleConfig 48
	 for (i=0;i<8;i++){
	      for (j=0;j<6;j++){
		       xEEPROM=eeprom_read_byte(&DefNozzleMap[i][j]);     
			   if (xEEPROM>=10)xEEPROM=0;
			   sprintf_P(strSend,PSTR("%d"),xEEPROM);
			   uart_print(1,0,strSend);
		  }
	 }
	 //PrintNoMoney
	 xEEPROM=eeprom_read_byte(&DefPrintMoney);
	 if (xEEPROM==True){
		 sprintf_P(strSend,PSTR("1"));
	 }else{
	     sprintf_P(strSend,PSTR("0"));
	 }uart_print(1,0,strSend);
	 //TermId
     xEEPROM=eeprom_read_byte(&DefIFT_ID);     
     sprintf_P(strSend,PSTR("%.2d"),xEEPROM);
     uart_print(1,0,strSend);
	 //PumpLabel
     for (i=0;i<8;i++){
	      xEEPROM=eeprom_read_byte(&DefPumpLabel[i]);
		  if (xEEPROM>=100)xEEPROM=0;
		  FillChar(strSend,0,sizeof(strSend));
	      sprintf_P(strSend,PSTR("%.2d"),xEEPROM);
		  AddSpaceLag(strSend,2);
		 uart_print(1,0,strSend);
	 }
	 uart_printf(1,1,PSTR(">"));
	 //End
}

void ConfigProtocol(char dataIn){
     static char Conflow[7];
    //Configuration Data Request 
	if ((Conflow[6]=='i')&&(Conflow[5]=='C')&&(Conflow[4]=='o')&&(Conflow[3]=='n')&&(Conflow[2]=='f')&&(Conflow[1]=='?')&&(Conflow[0]==0x0D)&&(dataIn==0x0A)){
	     //SendConfigParamater();        
		 ConfigCommand=CC_SEND_CONFIG;
	}

	if (IsConfigFlow==True){        
	    rcv_trans[char_count]=dataIn;
	    char_count++; 
	}
	if ((Conflow[0]==0x04)&&(dataIn==0x40)){
	     IsConfigFlow=True;
		 char_count=0;
	}
	//SavingData
	if ((Conflow[0]==0x05)&&(dataIn==0x50)){
	     transLength=char_count;
		 char_count=0;
	     IsConfigFlow=False;
		 ConfigCommand=CC_SAVE_CONFIG;
		 //SaveConfigParameter();
	}

	//Shifting
	Conflow[6]=Conflow[5];
	Conflow[5]=Conflow[4];
	Conflow[4]=Conflow[3];
	Conflow[3]=Conflow[2];
	Conflow[2]=Conflow[1];
	Conflow[1]=Conflow[0];
	Conflow[0]=dataIn;

}

// NotSuccessful! 


ISR(USART1_RX_vect){
	char dataTX1,serialSend[12];
	static char IsAdvanZProtocol=False;
	unsigned int i;
	dataTX1 = UDR1;
    //Not Successfull!
	//NewPacket
	if (IsNewPacket==True){
	    if (dataTX1==0x01){
		    char_count=0;
            IsNewPacket=False;
			IsAdvanZProtocol=True;
			}
	}
    //Save data to buffer if AdvanZ MSG
	if (IsAdvanZProtocol==True){
	    rcv_trans[char_count]=dataTX1;
	    char_count++; 
	}

	if (dataTX1==0x02){
		transLength=char_count;
		char_count=0;
		IsAdvanZProtocol=False;

		//MessageIdentification
        if (MsgCode!=MSG_NONE)MsgCode=MSG_NONE;
	    MsgCode=((rcv_trans[35]-'0')*10)+(rcv_trans[36]-'0');
     
	    IdentifyMessage(rcv_trans[0],transLength);
		IsNewPacket=True;

		if ((rcv_trans[35]=='8')&&(rcv_trans[36]=='1')){
		   LengthMessage81=transLength-12-44;
		   IsMessage81=True;
		   IsCompleteFilling=True;
		}
        //Testing Serial In COM1
        //for(i=0;i<transLength;i++){
        //   uart(0,1,rcv_trans[i]);
       // 	}
//        		sprintf_P(serialSend,PSTR("Length:%d "),transLength);
  //      		uart_print(0,1,serialSend);
		

	}//Endif(dataTX1==0x02)

	//ConfigDetection: iConf?<0D><0A>
	if (IsNewPacket!=True)
	    ConfigProtocol(dataTX1);
	//GeniusDetection: [CMD][MSG][CSUM][0x0D]
	if ((IFType==IT_STANDALONE)&&(IsNewPacket!=True))
	     GeniusProtocol(dataTX1);		
    //uart(0,1,dataTX1);
	//uart(1,1,_spi(dataTX1)); //Testing SPI
}


void system_beep(unsigned int tBeep){
      DDRB= (DDRB|0b00100000);
	 PORTB=(PORTB&0b11011111);
	 TimBeep=tBeep;
}

void systemRestart(){
     char strRestart[1];
	 char iRestart;
	 
	 iRestart=0;
	 while(iRestart<100){
	       strRestart[iRestart]=iRestart;
	       iRestart++;	 
	 }
}

char systemForceType(){
     char Result,KeyChar;
	 lcd_printf(4,1,PSTR("[1]Slave [2]Standalone"));
	 system_beep(1);
	 _delay_ms(200);
	 system_beep(2);
	 while(1){
	    KeyChar=_key_btn(_key_scan(1));
		if ((KeyChar=='1')||(KeyChar=='2')){
		    Result=KeyChar;
			break;
		}
	 }
	return Result;
}

void system_stop(){
	 char __key,__num;
	 lcd_printf(4,1,PSTR("Press any key..     "));
	 system_beep(5);
	 while(1){
		__key = _key_scan(1);
		__num = _key_btn(__key);
		if ((__num>='0')&&(__num<='9')||(IsStandAloneDetected==True)) 
		break;
	 }
}

void DisplayScreenIdle(void){
char strCodeName[10],strVersion[10],lcdteks[20];
    sprintf_P(strCodeName,PSTR(CODE_NAME)); 
    sprintf_P(strVersion,PSTR(VERSION_NUM)); 
    sprintf_P(lcdteks,PSTR("%s %s"),strCodeName,strVersion); 
	lcd_clear();
	lcd_print(1,1,lcdteks);
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

void UpdateClientIP(){     
/*
     char i,IP_blok[4];
	 char strIP[4][5];
	 //192.168.010.002
     eeprom_read_block((void*)&IP_blok,(const void*)&DefClientIP,4);

	 for(i=0;i<4;i++){
	     zeroIP(IP_blok[i],strIP[i]);
	 }
     sprintf_P(strClientIP,PSTR("%s.%s.%s.%s"),strIP[0],strIP[1],strIP[2],strIP[3]);
	 */
}
void UpdateServerIP(){     
/*
     char i,IP_blok[4];
	 char strIP[4][4];
	 //192.168.016.180
     eeprom_read_block((void*)&IP_blok,(const void*)&DefServerIP,4);

	 for(i=0;i<4;i++){
	     zeroIP(IP_blok[i],strIP[i]);
	 }
		 sprintf_P(strServerIP,PSTR("%s.%s.%s.%s"),strIP[0],strIP[1],strIP[2],strIP[3]);
		 */
}

void UpdateIFT_ID(){
    /* char IdIFT;
	 IdIFT=(eeprom_read_byte(&DefIFT_ID)%100);
	 sprintf_P(strIFT_ID,PSTR("%.2d"),IdIFT);
	 */
}

void UpdateSeqNum(){
/*
     if (SeqNum<100)SeqNum++;
	 else SeqNum=0;
	 leadingZero(SeqNum,strSeqNum);
	 */
} 

void UpdateCardID(){
     char i,LengthID;
	 LengthID=strlen(strRFID);  //           +12345678  
	 for(i=0;i<20;i++){         //SSSSSSSSSSSS12345678
	    if (i<(20-LengthID))strCardID[i]=' ';
		else strCardID[i]=strRFID[i-(20-LengthID)];
	 }strCardID[20]=0;

}

void IFTSendMessage(char MsgCode){//<STX>[IFTID][Seq][No][SrceIP][DestIP][MsgCode]
     char IdIFT;
	 char strSeqNum[3];
	 char ReadIP[4];
	 char strSend[60];

     //STX
     uart(1, 1,0x01);
	 //[IFTID]
	 IdIFT=(eeprom_read_byte(&DefIFT_ID)%100);
	 sprintf_P(strSend,PSTR("%.2d"),IdIFT);
	 uart_print(1, 0,strSend);
     //[Seq]
	 SeqNum=((SeqNum+1)%100);
	 sprintf_P(strSend,PSTR("%.2d"),SeqNum);	 
	 uart_print(1, 0,strSend);
	 //[ClientIP]
     eeprom_read_block((void*)&ReadIP,(const void*)&DefClientIP,4);
	 sprintf_P(strSend,PSTR("%.3d.%.3d.%.3d.%.3d"),ReadIP[0],ReadIP[1],ReadIP[2],ReadIP[3]);
	 uart_print(1, 0,strSend);
	 //[ServerIP]
     eeprom_read_block((void*)&ReadIP,(const void*)&DefServerIP,4);
	 sprintf_P(strSend,PSTR("%.3d.%.3d.%.3d.%.3d"),ReadIP[0],ReadIP[1],ReadIP[2],ReadIP[3]);
	 uart_print(1, 0,strSend);
	 //[MsgCode]
	 sprintf_P(strSend,PSTR("%.2X"),MsgCode);	 
	 uart_print(1, 0,strSend);

     switch(MsgCode){
	 case MSG_04://Transaction Number
	      sprintf_P(strSend,PSTR("%s1"),strTranNo);
	      uart_print(1, 0,strSend);
	      break;
	 case MSG_10://No Additional Parameter
	      break;
	 case MSG_22://strCardID
	      UpdateCardID();
	      sprintf_P(strSend,PSTR("%s"),strCardID);
	      uart_print(1, 0,strSend);
	      break;
	 case MSG_24://strCardID, FIP_ID
	      UpdateCardID();
	      sprintf_P(strSend,PSTR("%s%s"),strCardID,strFIP_ID);
	      uart_print(1, 0,strSend);
	      break;

	 case MSG_28://strCardID, FIP_ID
	      UpdateCardID();
	      sprintf_P(strSend,PSTR("%s%s"),strFIP_ID,strCardID);
	      uart_print(1, 0,strSend);
	      break;
	 case MSG_32://Change MOP
	      UpdateCardID();
	      sprintf_P(strSend,PSTR("%s%s"),strFIP_ID,strPaymentType);
	      uart_print(1,0,strSend);
		  sprintf_P(strSend,PSTR("%s%s"),strRef1,strRef2);
		  uart_print(1,0,strSend);
		  sprintf_P(strSend,PSTR("%s%s"),strRef3,strRef4);
		  uart_print(1,0,strSend);
	      break;

	 case MSG_56://strCardID, FIP_ID
	      UpdateCardID();
	      sprintf_P(strSend,PSTR("%s"),strCardID);
	      uart_print(1, 0,strSend);
	      break;
	 case MSG_58://Request Local Account Transaction
	      UpdateCardID();
		  AddSpaceLead(strBalanceValue,13);
		  AddSpaceLead(strOdometer,10);
		  CardType=0;
		  sprintf_P(strSend,PSTR("%s%s%d"),strCardID,strFIP_ID,NozzleID);
		  uart_print(1,0,strSend);
		  sprintf_P(strSend,PSTR("%s%s%d"),strBalanceType,strBalanceValue,CardType);
		  uart_print(1,0,strSend);
		  uart_printf(1,0,PSTR("F0000000E123456FFFFF"));
		  sprintf_P(strSend,PSTR("%s"),strOdometer);
		  uart_print(1,0,strSend);
		  break;
	 case MSG_90://Request EDC
	      sprintf_P(strSend,PSTR("%s%s"),strFIP_ID,strRef1);
	      uart_print(1, 0,strSend);
	      break;
	 case MSG_92://EDC Approval
		  sprintf_P(strSend,PSTR("%s%s%s%s"),strTranNo,strFIP_ID,strCardType,strCardID);
	      uart_print(1,0,strSend);	    
		  sprintf_P(strSend,PSTR("%s%s%s"),strApprovalCode,strInvoiceNumber,strDateTime);
	      uart_print(1,0,strSend);
	      break;	 
	 case MSG_98://Request Transaction Info
	      sprintf_P(strSend,PSTR("%s"),strFIP_ID);
	      uart_print(1, 0,strSend);
	      break;

	 }     
     //[Checksum]
	 uart_printf(1,0,PSTR("F968CFFB"));
	 //ETX
	 uart(1, 1,0x02);
}


/*Subrutine Msg04*/
void sendMessage04(){   //      <STX>[IFTID][Seq][No][SrceIP][DestIP][MsgCode][ReceiptNo][Value][Checksum][ETX]
                        //Msg04: <01>[01][03][192.168.000.101][192.168.000.001][04][000001]0F968CFFB]<02>
/*						
	 char strSend[60];
	 uart(1, 1,0x01);
	 UpdateIFT_ID(); //ReadIFT_ID
	 UpdateSeqNum(); //UpdateSeqNum SeqNum++
	 UpdateClientIP();//ReadSourceIP
	 UpdateServerIP();//ReadDestIP
	 //strTranNo = strReceiptNum
	 sprintf_P(strSend,PSTR("%s%s%s%s04%s1F968CFFB"),strIFT_ID,strSeqNum,strClientIP,strServerIP,strTranNo);
     uart_print(1, 0,strSend);
	 uart(1, 1,0x02);
	 */IFTSendMessage(MSG_04);
}

/*Subrutine Msg10*/
void sendMessage10(){//Msg10: <01>[0103192.168.016.070192.168.016.18010F968CFFB]<02>
/*
	char strSend[60];
	uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	sprintf_P(strSend,PSTR("%s%s%s%s10F968CFFB"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    uart_print(1, 0,strSend);
	uart(1, 1,0x02);
	IsNewPacket=True;
	*/
	IsNewPacket=True;
	IFTSendMessage(MSG_10);
}

void sendMessage22(){//Msg22: <01>[0103192.168.016.070192.168.016.18022[CardID]F968CFFB]<02>
/*
	char strSend[60];
	uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	sprintf_P(strSend,PSTR("%s%s%s%s22"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    uart_print(1, 0,strSend);
	UpdateCardID();
	sprintf_P(strSend,PSTR("%sF968CFFB"),strCardID);
    uart_print(1, 0,strSend);
	uart(1, 1,0x02);
	*/IFTSendMessage(MSG_22);
}

void sendMessage24(){//Msg24: <01>[0103192.168.016.070192.168.016.18022[CardID]F968CFFB]<02>
/*
	char strSend[60];
	uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	sprintf_P(strSend,PSTR("%s%s%s%s24"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    uart_print(1, 0,strSend);
	UpdateCardID();
	sprintf_P(strSend,PSTR("%s%sF968CFFB"),strCardID,strFIP_ID);
    uart_print(1, 0,strSend);
	uart(1, 1,0x02);
	*/IFTSendMessage(MSG_24);
}

void sendMessage28(){//Msg28: <01>[0103192.168.016.070192.168.016.18024[FIP][CardID]F968CFFB]<02>
/*
	char strSend[60];
	uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	sprintf_P(strSend,PSTR("%s%s%s%s28"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    uart_print(1, 0,strSend);
	UpdateCardID();
	sprintf_P(strSend,PSTR("%s%sF968CFFB"),strFIP_ID,strCardID);
    uart_print(1, 0,strSend);
	uart(1, 1,0x02);
	*/IFTSendMessage(MSG_28);
}



void sendMessage32(){//Msg32: <01>[ID][Seq][SrcIP][DestIP][MsgCode][FIP][PaymentType][Ref1][Ref2][Ref3][Ref4]<02>
/*	char strSend[80];
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	uart(1, 1,0x01);
	sprintf_P(strSend,PSTR("%s%s%s%s32"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    uart_print(1, 0,strSend);
	sprintf_P(strSend,PSTR("%s%s%s%s"),strFIP_ID,strPaymentType,strRef1,strRef2);
    uart_print(1, 0,strSend);
	sprintf_P(strSend,PSTR("%s%s"),strRef3,strRef4);
    uart_print(1, 0,strSend);
	sprintf_P(strSend,PSTR("F968CFFB"));
    uart_print(1, 0,strSend);
	uart(1, 1,0x02);
*/IFTSendMessage(MSG_32);
}

void sendMessage56(){
/*	char strSend[60];
	uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	UpdateCardID();  //ReadCardID

	sprintf_P(strSend,PSTR("%s%s%s"),strIFT_ID,strSeqNum,strClientIP);
    uart_print(1, 0,strSend);
	sprintf_P(strSend,PSTR("%s56"),strServerIP);
    uart_print(1, 0,strSend);
    sprintf_P(strSend,PSTR("%s"),strCardID);
    uart_print(1, 0,strSend);	

	uart_printf(1,0,PSTR("AF968CFFB"));
	uart(1, 1,0x02);
	*/
	IFTSendMessage(MSG_56);
}

void sendMessage58(){
/*	char strSend[80];
	uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	UpdateCardID();  //ReadCardID
	sprintf_P(strSend,PSTR("%s%s%s%s58"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    uart_print(1, 0,strSend);
	AddSpaceLead(strBalanceValue,13);
	AddSpaceLead(strOdometer,10);
	sprintf_P(strSend,PSTR("%s%s%d%s%s"),strCardID,strFIP_ID,NozzleID,strBalanceType,strBalanceValue);
    uart_print(1, 0,strSend);

	CardType=0;
	sprintf_P(strSend,PSTR("%dF0000000E123456FFFFF%sE9445512"),CardType,strOdometer);
    uart_print(1, 0,strSend);
	uart(1, 1,0x02);
	*/
	IFTSendMessage(MSG_58);
}

void sendMessage90(){
/*	char strSend[80];
	uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	sprintf_P(strSend,PSTR("%s%s%s%s90"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    uart_print(1, 0,strSend);
	sprintf_P(strSend,PSTR("%s%s"),strFIP_ID,strRef1);
    uart_print(1, 0,strSend);
	sprintf_P(strSend,PSTR("E9445512"));
    uart_print(1, 0,strSend);
	uart(1, 1,0x02);
	*/
	IFTSendMessage(MSG_90);
}

void sendMessage92(){
/*	char strSend[80];
	uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	sprintf_P(strSend,PSTR("%s%s%s%s92"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    uart_print(1, 0,strSend);
    //uart_print(0, 0,strSend);
	sprintf_P(strSend,PSTR("%s%s%s%s"),strTranNo,strFIP_ID,strCardType,strCardID);
    uart_print(1, 0,strSend);
    //uart_print(0, 0,strSend);
	sprintf_P(strSend,PSTR("%s%s%s"),strApprovalCode,strInvoiceNumber,strDateTime);
    uart_print(1, 0,strSend);
    //uart_print(0, 0,strSend);
	sprintf_P(strSend,PSTR("E9445512"));
    //uart_print(0, 0,strSend);
    uart_print(1, 0,strSend);
	uart(1, 1,0x02);
	*/
	IFTSendMessage(MSG_92);
}

void sendMessage94(){//Void Transaction Message
/*	char strSend[80];
	uart(1, 1,0x01);
	UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	sprintf_P(strSend,PSTR("%s%s%s%s94"),strIFT_ID,strSeqNum,strClientIP,strServerIP);
    uart_print(1, 0,strSend);
    //uart_print(0, 0,strSend);
	sprintf_P(strSend,PSTR("%s%s"),strInvoiceNumber,strDateTime);
    uart_print(1, 0,strSend);
	sprintf_P(strSend,PSTR("E9445512"));
    //uart_print(0, 0,strSend);
    uart_print(1, 0,strSend);
	uart(1, 1,0x02);
	*/
	IFTSendMessage(MSG_94);
}

void sendMessage98(char FIPAddr){
/*
	char __pump_id[8];
	char strSend[50];
	char xFIP_ID;
	
	uart(1, 1,0x01);
    UpdateIFT_ID(); //ReadIFT_ID
	UpdateSeqNum(); //UpdateSeqNum SeqNum++
	UpdateClientIP();//ReadSourceIP
	UpdateServerIP();//ReadDestIP
	eeprom_read_block((void*) &__pump_id, (const void*) &DefPumpMap, 8);
	xFIP_ID=__pump_id[PumpID-1];
	//if (xFIP_ID>16)xFIP_ID=16;
	sprintf_P(strSend,PSTR("%s%s%s%s98%.2dF968CFFB"),strIFT_ID,strSeqNum,strClientIP,strServerIP,xFIP_ID);
	uart_print(1, 0,strSend);
	uart(1, 1,0x02);
*/
    sprintf_P(strFIP_ID,PSTR("%.2d"),GetPumpID(FIPAddr));
	IFTSendMessage(MSG_98);
	IsBusyMsg11=True;
}




void ViewCardID(){//+12345678 -->12345678
     char i;
	 for(i=0;i<8;i++){         //00000000000012345678
		strCardID[i]=strRFID[i];
	 }strCardID[8]=0;     
}
//Message 56


void StringCopy(char *Source,char *Dest,char Length){
     char i;
	 for (i=0;i<Length;i++){
	     Dest[i]=Source[i];
	 }Dest[Length]=0;
}
void StringCopyPos(char *Source,char *Dest,char SrcPos,char Length){
     char i;
	 for (i=0;i<Length;i++){
	     Dest[i]=Source[SrcPos+i];
	 }Dest[Length]=0;
}

void TestLocalAccount(){

}

void FormatDecimal(char *strRawData, char DecimalCfg){// 9-> 0,009 0,234 123 
     char i,iPos,CommaPos,Length=0;                 //1->0,01 
	 char strDecimalFormated[20];
	 char DecPointMark;
     if (DecimalCfg>0){    
		 DecPointMark=eeprom_read_byte(&DefDecimalMark);	 
		 Length=strlen(strRawData);

		 if (Length>DecimalCfg)CommaPos=Length-DecimalCfg;
		 else CommaPos=1;


		 if (Length<=DecimalCfg){// 123 -> 0123
			 AddZeroLead(strRawData,(DecimalCfg+1));
			 //AddZeroLead(strRawData,(DecimalCfg+2));
			 Length=strlen(strRawData);
			 CommaPos=1;
			 }

		 
		 iPos=0;
		 for(i=0;i<Length;i++){
			 if (i==CommaPos){
				 strDecimalFormated[iPos]=DecPointMark;
				 iPos++;
			}
			strDecimalFormated[iPos]=strRawData[i];
			//strDecimalFormated[iPos+1]=0;
			iPos++;
		 }strDecimalFormated[iPos]=0;
		 
         //uart_print(1,1,strDecimalFormated); 

		 Length=strlen(strDecimalFormated);
		 for(i=0;i<Length;i++){
			 strRawData[i]=strDecimalFormated[i];
		 }strRawData[Length]=0;
	 }
          //uart_print(1,1,strRawData); 
}

void FormatPrice(char *strRawPrice){
     char DecCfg;
     DecCfg=eeprom_read_byte(&DefDecimalPrice);
	 FormatDecimal(strRawPrice,DecCfg);
}

void FormatMoney(char *strRawMoney){
     char DecCfg;
     DecCfg=eeprom_read_byte(&DefDecimalMoney);
	 FormatDecimal(strRawMoney,DecCfg);
}

void FormatVolume(char *strRawVolume){//96->0,96 
     char DecCfg;
     DecCfg=eeprom_read_byte(&DefDecimalVolume);
	 FormatDecimal(strRawVolume,DecCfg);
}

void FormatTotalizerMoney(char *strRawMoney){
     char DecCfg;
     DecCfg=eeprom_read_byte(&DefDecimalTotalMoney);
	 FormatDecimal(strRawMoney,DecCfg);
}

void FormatTotalizerVolume(char *strRawVolume){//96->0,96 
     char DecCfg;
     DecCfg=eeprom_read_byte(&DefDecimalTotalVolume);
	 FormatDecimal(strRawVolume,DecCfg);
}


void FormatCurrency(char *strCurrency){// 5000000.00 5,000,000.00
     char i,dotPos=0,iPos=0,Length=0,nComa=0;//cPos=0,;
	 char fmtCurrency[20];
	 char CurrencyMark,DecimalMark;

	 CurrencyMark=eeprom_read_byte(&DefCurrencyMark);
	 DecimalMark=eeprom_read_byte(&DefDecimalMark);

	 Length=strlen(strCurrency);//123,456
	 dotPos=Length;
	 nComa=0;
	 FillChar(fmtCurrency,sizeof(fmtCurrency),0);
	 //Detection
	 for(i=0;i<Length;i++){// 123,456 
	     if (strCurrency[i]==DecimalMark) dotPos=i;//3
		 if (strCurrency[i]==CurrencyMark) nComa++;//0
	 }

	 if((nComa==0)&&(dotPos>3)){		 
         iPos=0;
	     for(i=0;i<dotPos;i++){
	         if ((((dotPos-i)%3)==0)&&(i<(dotPos))&&(i>0)){ 
		        fmtCurrency[iPos]=CurrencyMark;
		        iPos++;
		        }
		        fmtCurrency[iPos]=strCurrency[i];
			    iPos++;
			    fmtCurrency[iPos]=0;
	         }
         
	     for(i=dotPos;i<Length;i++){
		        fmtCurrency[iPos]=strCurrency[i];
			    iPos++;
			    fmtCurrency[iPos]=0;
	         }
	      for(i=0;i<strlen(fmtCurrency);i++){
	          strCurrency[i]=fmtCurrency[i];
			  strCurrency[i+1]=0;
	      }
     }      
}

void BackLightTrig(){    
     __key_light = 1; 
	 __key_lgtcnt = 0; 
	 PORTG=PORTG&0b11111101;
}


char FMenuLocalAccount(){
static char stLocalAccount=laInit,LocAccStatus=0;       
static char iPos=0,iWait=0,KeyCharLast=' ',BalanceType=0,strFIP[3];//,FIPResult;
static char iValuePos=0,iNozzle=0;
static char ProductID[6];//,FIP[8];//ValueChar[10]       
static char FIP_Used=0,ProdId=0,IsFullAuthorized=False;
static unsigned int iLoop=0; 


       char KeyPressed=0,KeyChar=0;
       char lcdteks[20],uiResult,FIPResult;
       char ProductName[11],strName[21];//,strPumpL[3],strPumpR[3];
       char Result;

	   Result=MENU_NONE;
	                                 //|   Local Account   |          
	       switch(stLocalAccount){   //|ID: ACD12345       |
	       case laInit:              //|Process ...        |
		        //Process RFID Data  //|                   |
				//UpdateCardID();
                BackLightTrig();ViewCardID();
	            lcd_clear();
	            lcd_printf(1,1,PSTR("   Local Account   "));
	            sprintf_P(lcdteks,PSTR("ID:%s"),strCardID);lcd_print(2,1,lcdteks);
                lcd_printf(3,1,PSTR("Proses"));
				iPos=0;//iLoop=0;
				IsFullAuthorized=False;
		        stLocalAccount=laSendID;
				break;         
           case laSendID://sendMessage56 
				IsMessage57=False;
                sendMessage56();
				iWait=0;iPos=0;
				//iLoop=0;
				TimSend=0;
				stLocalAccount=laWaitMessage57;
		        break;
           case laWaitMessage57:
                /*
				if(TimSend<TIM_LOCAL_ACCOUNT){
                   lcd_put(3,(8+(TimSend%4)),'.');

				}else if(TimSend>TIM_LOCAL_ACCOUNT){					     
						 TimLocAcc=0;
						 system_beep(2);
						 stLocalAccount=laConTimout;
				}
				*/
				
		        iLoop++;
				if ((iLoop%MSG_WAIT_TIMOUT)==0){
				   if (iPos<5){
					   lcd_put(3,(8+iPos),'.');
				       iPos++;
					   }
				   else{
				       iPos=0;
					   lcd_printf(3,(9+iPos),PSTR("       "));
					   iWait++;
					   }
				}
				if (iWait>3){
				    stLocalAccount=laConTimout;
					TimLocAcc=0;
					system_beep(2);
				    }
                 
				if (IsMessage57==True){
				    IsMessage57=False;
				    stLocalAccount=laProcMessage57;
					}
		        break;
           case laConTimout:
		        //IsErrorTCPIP=True;
				lcd_printf(3,1,PSTR("Sending Failed      "));
				lcd_printf(4,1,PSTR("TCP/IP Error        "));
                if (TimLocAcc>2)  stLocalAccount=laExitLocAcc;
		        break;
           case laProcMessage57://Process Message
				lcd_printf(3,1,PSTR("Data Received       "));
				LocAccStatus=GetLocAccStatus(procMessage57());
				TimLocAcc=0;
				stLocalAccount=laDispStatus;
		        break;
           case laDispStatus://Display Status
		        if (LocAccStatus==LA_INVALID){
					lcd_printf(3,1,PSTR("Tidak Terdaftar     "));    
					TimLocAcc=0;
				    stLocalAccount=laDelayExit;
				}
				else
                if (LocAccStatus==LA_VALID) {
				    stLocalAccount=laDispValid; 
					lcd_clear();
					}
				else
                if (LocAccStatus==LA_LIMITED){
					lcd_printf(3,1,PSTR("Kartu Terbatas      "));    
					TimLocAcc=0;
				    stLocalAccount=laDelayExit;
				}
                if (TimLocAcc>5)stLocalAccount=laExitLocAcc;
		        break;
           case laDispValid://Display VALID ID
		        lcd_clear();
				sprintf_P(strBalanceValue,PSTR("%s"),strBalance);
				FormatCurrency(strBalance);

		        StringCopy(strCardHolder,strName,20);  				                                                        
		        sprintf_P(lcdteks,PSTR("%s"),strCompName);               lcd_print(1,1,lcdteks);   //[CompName]
                //sprintf(lcdteks,"%s %s",strCardID,strName);    lcd_print(2,1,lcdteks);   //[Card ID][Card Holder]
				sprintf_P(lcdteks,PSTR("%s"),strName);                   lcd_print(2,1,lcdteks);   //[Card ID][Card Holder]
				sprintf_P(lcdteks,PSTR("%s %s"),strLicPlate,strBalance); lcd_print(3,1,lcdteks);   //[LicPlate][Balance] 
	            lcd_printf(4,1,PSTR("[*]Batal       [#]OK"));   //[*]Batal       [#]OK
				stLocalAccount=laDispValidInput;
		        break;
           case laDispValidInput:
                KeyPressed=_key_scan(1);
				if (KeyPressed==_KEY_ENTER) stLocalAccount=laSelectFIP;
				else
				if (KeyPressed==_KEY_CANCEL)stLocalAccount=laExitLocAcc;
		        break;
           case laSelectFIP:      
			    FIPResult=menu_FIP(&FIP_Used,strFIP);

			    if ((FIPResult==FIP_DONE)&&(FIP_Used>0)){
                    sprintf_P(strFIP,PSTR("%.2d"),FIP_Used);
					sprintf_P(strFIP_ID,PSTR("%.2d"),FIP_Used);
                    //Load Product Info
					if (GetFIPAddr(FIP_Used)>0) 
					    eeprom_read_block((void*) &ProductID, (const void*) &DefNozzleMap[GetFIPAddr(FIP_Used)-1], 6);
				    stLocalAccount=laSelectProduct;
				}
			    else if (FIPResult==FIP_CANCEL){
				    stLocalAccount=laDispValid;
				}
		        break; 
           case laSelectFIPInput:

		        break;
           case laSelectProduct://NozzleID
				lcd_clear();
		        sprintf_P(lcdteks,PSTR("Pompa-%s"),strFIP);lcd_print(1,1,lcdteks);
				//Not MPD->Single Product
				if ((ProductID[0]>=1)&&(ProductID[0]<=6)&&
				    (ProductID[1]==0)&&(ProductID[2]==0)&&(ProductID[3]==0)){
                     //Load 1st Product      					 
					 ProdId=ProductID[0];
					 eeprom_read_block((void*) &ProductName, (const void*) &DefProductName[ProdId-1],10);
					 StrPosCopy(ProductName,ProductName,0,8);
					 sprintf_P(strProduct,PSTR("%s"),ProductName);
                     stLocalAccount=laSelectBalanceType;
					}else
                {
				//MultiProductDisplay Select:
				//Load Nozzle Configuration
				for(iNozzle=0;iNozzle<4;iNozzle++){
                    //Masalah 0-->void
				    ProdId=ProductID[iNozzle];
					if ((ProdId>=1)&&(ProdId<=6)){
				        eeprom_read_block((void*) &ProductName, (const void*) &DefProductName[ProdId-1],10);
						StrPosCopy(ProductName,ProductName,0,8);
					    sprintf_P(lcdteks,PSTR("%d.%s"),(iNozzle+1),ProductName);
                    }
					else{
					       sprintf_P(lcdteks,PSTR("     "));					
					}
				    lcd_print(2+(iNozzle%2),1+(iNozzle/2)*11,lcdteks);
				}
	            lcd_printf(4,1,PSTR("[*]Back             "));   //"[*]Back             "				
				stLocalAccount=laSelectProductInput;
				}
		        break; 
           case laSelectProductInput:
				KeyPressed=_key_scan(1);
			    KeyChar=_key_btn(KeyPressed);
				if ((KeyChar>='1')&&(KeyChar<='4')){
				    iNozzle=KeyChar-'1';
				    eeprom_read_block((void*) &ProductName, (const void*) &DefProductName[ProductID[iNozzle]-1],10);
					sprintf_P(strProduct,PSTR("%s"),ProductName);
					NozzleID=iNozzle+1;
                    stLocalAccount=laSelectBalanceType;
					lcd_clear();
					}                
				if (KeyPressed==_KEY_CANCEL)stLocalAccount=laSelectFIP;//Back To Pump Selection
		        break;
           case laSelectBalanceType:
		        lcd_clear();
		        sprintf_P(lcdteks,PSTR("P%s-%s "),strFIP,strProduct);      
				 lcd_print(1,1,lcdteks);                        //"P01-Pertamax        "                                                                      
		        lcd_printf(2,1,PSTR("[1]Volume           "));   //"[1]Premium          "                                                                      
				lcd_printf(3,1,PSTR("[2]Amount           "));   //"[2]Pertamax         "
				lcd_printf(4,1,PSTR("[*]Back    [#]Next  "));   //"[*]Back             "						        
	            //lcd_printf(4,1,PSTR("[*]Back    [#]Next  "));   //"[*]Back             "						        
                stLocalAccount=laSelectBalanceTypeInput;   
		        break;
           case laSelectBalanceTypeInput:
				KeyPressed=_key_scan(1);
			    KeyChar=_key_btn(KeyPressed);
				if ((KeyChar>='1')&&(KeyChar<='2')){
                    if(KeyChar=='1'){
					   BalanceType=1;
					   sprintf_P(strBalanceType,PSTR("V"));
					   }
					else
                    if(KeyChar=='2'){
					   BalanceType=2;
                       sprintf_P(strBalanceType,PSTR("A"));
					   }
				    stLocalAccount=laBalanceValue;  
					}              
				if (KeyPressed==_KEY_CANCEL){
				    if ((ProductID[0]>=1)&&(ProductID[0]<=6)&&(ProductID[1]==0)&&(ProductID[2]==0)&&(ProductID[3]==0))
					     stLocalAccount=laSelectFIP;//Back To FIP Select Non MPD
                    else stLocalAccount=laSelectProduct;//Back To Product Selection MPD
					lcd_clear();
					}
                else
				if (KeyPressed==_KEY_ENTER){
				    BalanceType=1;
					sprintf_P(strBalanceType,PSTR("A"));
					//sprintf_P(strBalanceValue,PSTR("999"));
					IsFullAuthorized=True;
                    stLocalAccount=laOdometer;//FullAuthorized
					}

		        break;
           case laBalanceValue:
		        lcd_clear();
		        if(BalanceType==1){
		           sprintf_P(lcdteks,PSTR("P%s-%s "),strFIP,strProduct);      
				    lcd_print(1,1,lcdteks);                        //"P01-Pertamax        "                                                                      
		           lcd_printf(2,1,PSTR("[1]Volume:_         "));   //"[1]Volume:_         "                                                                      
				   lcd_printf(3,1,PSTR("                    "));   //"                    "
	               lcd_printf(4,1,PSTR("[*]Back  [#]OK      "));   //"[*]Back  [#]OK     "						        
				   }
                else
		        if(BalanceType==2){
		           sprintf_P(lcdteks,PSTR("P%s-%s "),strFIP,strProduct);      
				   lcd_print(1,1,lcdteks);                         //"P01-Pertamax        "                                                                      
		           lcd_printf(2,1,PSTR("                    "));   //"                    "                                                                      
				   lcd_printf(3,1,PSTR("[2]Amount:_         "));   //"[2]Amount:_         "
	               lcd_printf(4,1,PSTR("[*]Back  [#]OK      "));   //"[*]Back  [#]OK      "			        
				   }
                iLoop=0;
				iValuePos=0;
				//ValueChar[iValuePos]=' ';
		        stLocalAccount=laBalanceValueInput;  
		        break; 
           case laBalanceValueInput:
		        uiResult=UserInput(UI_NUMBER_R,(1+BalanceType),11,strBalanceValue,0,10); 
				if (uiResult==USER_CANCEL){
				    lcd_clear();
					_delay_ms(100);		        
				    stLocalAccount=laSelectBalanceType;		        
					}
				else
				if (uiResult==USER_OK){				    
				    stLocalAccount=laOdometer;
					lcd_clear();
					_delay_ms(100);		        
					}
		   /*
			    //Blinking 50% _      BalanceValue Volume=999  Amount=65536
			    iLoop++;
			    //GetKeyPressed
			    KeyPressed=_key_scan(1);
			    KeyChar=_key_btn(KeyPressed);
				if (((KeyChar>='0')&&(KeyChar<='9')&&(iValuePos<6)&&(BalanceType==2))||//Max Rp.999999
				   ((KeyChar>='0')&&(KeyChar<='9')&&(iValuePos<3)&&(BalanceType==1))){ //Max       999L
				    ValueChar[iValuePos]=KeyChar;
					iValuePos++;
					lcd_xy(1+BalanceType,(10+iValuePos));_lcd(ValueChar[iValuePos-1]); 
					_delay_ms(200);
					}
			    if ((iLoop%2000)==0){
				     lcd_xy((1+BalanceType),(11+iValuePos));_lcd('_'); 
			    }
			    if ((iLoop%2000)==1000){
				     lcd_xy((1+BalanceType),(11+iValuePos));_lcd(' '); 
			    }
				if (KeyPressed==_KEY_CANCEL){
				    lcd_clear();
					_delay_ms(100);		        
				    stLocalAccount=laSelectBalanceType;		        
					}
				else
				if (KeyPressed==_KEY_ENTER){				    
                    ValueChar[iValuePos]=0;
					sprintf_P(strBalanceValue,PSTR("%s"),ValueChar);
				    stLocalAccount=laOdometer;
					lcd_clear();
					_delay_ms(100);		        
					}
*/
		        break;
           case laOdometer://GetOdometer			
		        lcd_clear();
				if (IsFullAuthorized==True){
				    lcd_printf(1,1,PSTR("Pump Product   Full"));
					sprintf_P(lcdteks,PSTR("  %s %s "),strFIP,strProduct);    lcd_print(2,1,lcdteks);    //"P01  Pertamax       "
		            sprintf_P(lcdteks,PSTR("  Tank"));            lcd_print(2,14,lcdteks);   //"P01  Pertamax 500000"
				}
                else{
		            if (BalanceType==1) lcd_printf(1,1,PSTR("Pump Product Volume"));
                    if (BalanceType==2) lcd_printf(1,1,PSTR("Pump Product Amount"));//"Pump Product  Amount" 
					sprintf_P(lcdteks,PSTR("  %s %s "),strFIP,strProduct);    lcd_print(2,1,lcdteks);    //"P01  Pertamax       "
		            sprintf_P(lcdteks,PSTR("%s "),strBalanceValue);            lcd_print(2,14,lcdteks);   //"P01  Pertamax 500000"

				}
				lcd_printf(3,1,PSTR("Odometer:_       "));                                //"Odometer:_          "
	            lcd_printf(4,1,PSTR("[*]Back  [#]OK   "));                                //"[*]Back  [#]OK      "						        
				ClearMem(strOdometer);
		        stLocalAccount=laOdometerInput;
		        break;
           case laOdometerInput:
				uiResult=UserInput(UI_NUMBER_R,3,10,strOdometer,0,10);
				if (uiResult==USER_OK){
				    if ((strlen(strOdometer)==1)&&(strOdometer[0]=='0'))
					    strOdometer[0]=0;
				    stLocalAccount=laDataConfirm;
					}
			    else
			    if (uiResult==USER_CANCEL){
				    if (IsFullAuthorized==True)
					     stLocalAccount=laSelectBalanceType;
					else stLocalAccount=laBalanceValue;		        
				 }
		        break;
           case laDataConfirm: 
		        lcd_clear();
		        StringCopyPos(strCardHolder,strName,9,10);                                          //Data Confirmation   
                sprintf_P(lcdteks,PSTR("%s "),strName);                  lcd_print(1,1,lcdteks);    //"Iyan The Man        "
		        sprintf_P(lcdteks,PSTR("ID: %s"),strCardID);             lcd_print(2,1,lcdteks);    //"ID1CCDA565 OD:123456"
				sprintf_P(lcdteks,PSTR("%s  %s "),strFIP,strProduct);    lcd_print(3,1,lcdteks);    //"P01  Pertamax 500000"
		        if (IsFullAuthorized==True){
                    lcd_printf(3,15,PSTR(" Full"));
				}
				else {
				    sprintf_P(lcdteks,PSTR("%s "),strBalanceValue);          lcd_print(3,15,lcdteks);   //"[*]Back [0]ESC [#]OK " 
					}
	            lcd_printf(4,1,PSTR("[*]Back [0]ESC [#]OK"));    
                stLocalAccount=laDataConfirmInput;
                //while(1){};   
		        break;
           case laDataConfirmInput:
                KeyPressed=_key_scan(1);
                if (KeyPressed==_KEY_CANCEL)stLocalAccount=laOdometer;		        
				else
				if (KeyPressed==_KEY_ENTER)stLocalAccount=laProceedTransaction;
				else
				if (KeyPressed==_KEY_0){
				   //ShowCancel
				   TimLocAcc=0;
                   lcd_clear();
				   lcd_printf(1,1,PSTR("Cancel Transaction  "));    //"Cancel Transaction"
				   stLocalAccount=laDelayExit;
				   }
		        break;
           case laProceedTransaction:
		        sendMessage58();
                RemZeroLead(strCardID);
                RemZeroLead(strBalanceValue);
				sprintf_P(lcdteks,PSTR("P%s %s "),strFIP,strProduct);       lcd_print(1,1,lcdteks);    //"P01  Pertamax 500000"
		        sprintf_P(lcdteks,PSTR("%s"),strBalanceValue);              lcd_print(1,15,lcdteks);   //"ID1CCDA565          " 
                lcd_printf(3,1,PSTR("Mohon Tunggu        "));                                         //"Mohon Tunggu ...    "
		        lcd_printf(4,1,PSTR("                    "));                                  //"                    "
				IsMessage00=False;
				TimLocAcc=0;
				iLoop=0;iPos=0;iWait=0;
				stLocalAccount=laWaitMessage00;
		        break;
           case laWaitMessage00:
                iLoop++;
				if ((iLoop%MSG_WAIT_TIMOUT)==0){
				   if (iPos<5){
				       iPos++;
					   lcd_xy(3,(14+iPos));_lcd('.');
					   }
				   else{
				       iPos=0;
					   lcd_printf(3,(14+iPos),PSTR("       "));
					   iWait++;
					   }
				}
				if (iWait>5){
				    stLocalAccount=laConTimout;
					TimLocAcc=0;
					system_beep(2);
				    }
				if (IsMessage00==True){
                    IsMessage00=False;
				    stLocalAccount=laProcMessage00;
					}
		        break;
           case laProcMessage00:
		        LocAccStatus=procMessage00();
                lcd_clear();
		        switch(LocAccStatus){
				case MSG00_NACK:
                     lcd_printf(2,1,PSTR("Authorisasi Gagal   "));
					 system_beep(2); 
					 stLocalAccount=laDelayExit;
				     break;
                case MSG00_ACK:
				     lcd_printf(2,1,PSTR("Authorisasi Berhasil"));
				     LocalAccountFIP[nLocalAccount]=FIP_Used;
					 nLocalAccount++;
					 IsViewFillingFIP=True;
					 stLocalAccount=laViewStatus;
				     break;
                case MSG00_NO_FIP:
                     lcd_printf(2,1,PSTR("FIP Tidak Siap    "));
					 system_beep(2);
					 stLocalAccount=laDelayExit;
				     break;
				}
				TimLocAcc=0;
				ClearMem(strOdometer);
	            ClearMem(strLicPlate);
		        break;
           case laViewStatus:
                if (TimLocAcc>20)stLocalAccount=laExitLocAcc;//laDisplayTransaction;
		        break; 
           case laDisplayTransaction:
		        lcd_clear();
                sprintf_P(lcdteks,PSTR("P%s %s "),strFIP,strProduct); lcd_print(1,1,lcdteks);    //"P01  Pertamax 500000"
		        sprintf_P(lcdteks,PSTR("%s"),strBalanceValue);        lcd_print(1,15,lcdteks);   //"ID1CCDA565          " 
                RemSpaceLead(strCardID);
				sprintf_P(lcdteks,PSTR("ID: %s"),strCardID);          lcd_print(2,1,lcdteks);
				TimLocAcc=0;
		        IsCompleteFilling=False;
				stLocalAccount=laWaitFilling;
		        break; 
           case laWaitFilling:
				if ((IsCompleteFilling==True)||(TimLocAcc>FILLING_TIMOUT)){
				    IsCompleteFilling=False;
				    stLocalAccount=laSuccessTransaction;
					TimLocAcc=0;
					}
		        break;
           case laSuccessTransaction:
				lcd_clear();lcd_printf(2,1,PSTR("Transaksi Selesai"));
				//Show Status FIP
				sprintf_P(lcdteks,PSTR("Fueling @FIP:#%d"),FIP_Used);
				lcd_print(3,1,lcdteks);

				TimLocAcc=0;
				stLocalAccount=laDelayExit;
		        break;
           case laFailedTransaction:
		        TimLocAcc=0;
		        //Show Failed Report
				stLocalAccount=laDelayExit;
		        break;
           case laDelayExit:
		        if (TimLocAcc>15)stLocalAccount=laExitLocAcc;
		        break;
           case laExitLocAcc:
		        Result=MENU_DONE;
				stLocalAccount=laInit;
				lcd_clear();
		        break;
	       }//EndSwitch
return Result;
}





void GeneratePrintInit(){//Create Initialize Print 
     char strVernum[6],strVerdate[12];
	 sprintf_P(strVernum,PSTR(VERSION_NUM));
	 sprintf_P(strVerdate,PSTR(VERSION_DATE));
     sprintf_P(PrintBuffer,PSTR("\nGeNiUs Ticket Printer \n Version:%s\n Last Update %s \n Hanindo Automation Solutions \n www.hanindogroup.com\n\n\n\n\n\n\n\n"),strVernum,strVerdate);
	 uart_print(0,1,PrintBuffer);
	 LengthMessage81=strlen(PrintBuffer);
	 //Spooling HFCx 0000
	 cmdPrint=0b00010000|(1<<PRN_PAPER_CUT);
	 IsFreePrinting=True;
}

void SetBaudRate(char ComAddr,char brMap){//Com1..Com4
unsigned int brValue=9600;
char brMessage=0;

     if ((ComAddr>=1)&&(ComAddr<=2)){
	     brValue=GetBaudrate(brMap);
	      uart_init((ComAddr-1),brValue);
	 }else
     if ((ComAddr>=3)&&(ComAddr<=4)){
	     //[COM][Baud]
		 brMessage=((ComAddr<<4)|(0x0F&brMap));
		 if (iSequencePooling>0)SendPoolingCommand(SC_BAUDRATE,brMessage);
		 else SendSlaveCommand(SC_BAUDRATE,brMessage);
		 
	 }
}

int GetBaudrate(char brSetting){
int Result=0;
     switch(brSetting){
     case brNone:
	      Result=9600;//Default
	      break;
	 case br9600: 
	      Result=9600;
	      break;
	 case br19200:
	      Result=19200;
	      break;
	 case br5787:
	      Result=5787;
     	  break;	 
	 }
   return Result;
}

void InitComport(){
     char brMap,i=0,lcdteks[20];	 
	 int bValue;
	 lcd_printf(3, 1, PSTR("Initialize COM ..."));
     for(i=0;i<4;i++){
	     brMap=eeprom_read_byte(&DefBaudrate[i]);
		 bValue=GetBaudrate(brMap);
		 if (bValue==5787)bValue=12213;
		 sprintf_P(lcdteks,PSTR("COM%d:%i           "),i+1,bValue);
		 lcd_print(4,1,lcdteks);
		 SetBaudRate(i+1,brMap);
 	     TimDisplay=0;
	     while(TimDisplay<2){
		     if (TimDisplay>1)break;
		 };

	 }
}

void InitMemory(){
     PumpCountMax=eeprom_read_byte(&DefPoolingPumpMax);
	 ActivePump=eeprom_read_byte(&DefActivePump);
}

void InitializeConnection(){
	unsigned int tConnect=0;
	char ForceType=IT_NONE,iCon=0,iTry=0,KeyPressed;
	char lcdteks[20];
	IFType=eeprom_read_byte(&DefInitIFT);
	DispenserBrand=eeprom_read_byte(&DefDispenserBrand);

	//SendSlaveCommand(SC_STOP_POOL_SEQUENCE,0);

    //Send Msg10
	if  (IFType==IT_SLAVE){
	     SendSlaveCommand(SC_SLAVE,ST_NONE);
		 char_count=0;
		 if (IsPowerOn==True){
		     IsPowerOn=False;
		     _delay_ms(8000);
			 }
         sendMessage10();
         sendMessage10();
         sendMessage10();
         

         lcd_printf(4,1,PSTR("Connect         "));
		 lcd_printf(4,1,PSTR("Connect"));
		 //EstablishConnection TCP/IP
		 IsMessage11=False;
		  //SlaveIdentification		  
		 while (IsMessage11==False){
		        lcd_put(4,(8+iCon),'.');
                tConnect++;
				__key_lgtcnt=0;
			    if ((tConnect%200)==0)iCon++;
			    if (iCon>8){
			        lcd_printf(4,1,PSTR("Connect         "));
					
				    sendMessage10();
				    SendSlaveCommand(SC_SLAVE,ST_NONE);
			        iCon=0;
				    iTry++;}
			    if (iTry>3){
			        //IsErrorTCPIP=True;
				    IsAdvanzStartupInfo=False;		
					IFType=IT_STANDALONE;
				    break;
                KeyPressed=_key_scan(1);
		        if (KeyPressed==_KEY_CANCEL)
				    break;

			    }
            }//EndWhile
		}

	if (IFType==IT_STANDALONE){
        IsStandAloneDetected=False;		
		SendSlaveCommand(SC_STANDALONE,DispenserBrand);
		tConnect=0,iCon=0,iTry=0;
	    lcd_clear();
	    lcd_printf(1,1,PSTR("Scan Pump"));
		strcpy_P(lcdteks,(PGM_P)pgm_read_word(&(DefListDispenserName[eeprom_read_byte(&DefDispenserBrand)])));
		sprintf_P(lcdteks,PSTR("%s"),lcdteks);
	    lcd_print(2,1,lcdteks);

	    while (IsStandAloneDetected==False){
		       lcd_put(4,(1+iCon),'.');
		       tConnect++;
		       if ((tConnect%100)==0)iCon++;
		       if (iCon>16){
			       __key_lgtcnt=0;
			       lcd_printf(4,1,PSTR("                   "));
				   //IsStandaloneAcknoledge=False;
		           SendSlaveCommand(SC_STANDALONE,DispenserBrand);
                   //InitStandalone();

			       DisplayPumpStatus();
			       iCon=0;
			       iTry++;}
		           if (iTry>3) break; 
	    }
	}//EndIf
	if(IsMessage11==True){
	   lcd_printf(4,1,PSTR("Connected           "));
	   eeprom_write_byte(&DefInitIFT,IT_SLAVE);
	   IFType=IT_SLAVE;
	   _delay_ms(3000);
	   //IsErrorTCPIP=False;
	   procMessage11();
	   IsAdvanzStartupInfo=True;
	}
	if(IsStandAloneDetected==True){
	   lcd_printf(4,1,PSTR("PumpFound"));
	   eeprom_write_byte(&DefInitIFT,IT_STANDALONE);
	   IFType=IT_STANDALONE;
	}
	//PrintStatus
	if (eeprom_read_byte(&DefPrintInitialize)==True)GeneratePrintInit();
	if ((IsStandAloneDetected==False)&&(IsMessage11==False)){		
		 lcd_printf(2,1,PSTR("TCP/IP Error"));
		 lcd_printf(3,1,PSTR("No Pump Found"));
		 _delay_ms(2000);
		 ForceType=systemForceType();
		 if (ForceType=='1'){
		     IFType=IT_SLAVE;
			 SendSlaveCommand(SC_SLAVE,ST_NONE);
         }
		 else
		 if (ForceType=='2'){
		     IFType=IT_STANDALONE;
			 SendSlaveCommand(SC_STANDALONE,DispenserBrand);

			 //SendPoolingCommand(SC_SET_POOLING_MAX_PUMP,eeprom_read_byte(&DefPoolingPumpMax));

			 //InitStandalone();
         }
		 eeprom_write_byte(&DefInitIFT,IFType);
	}
	InitMemory();
	//SendSlaveCommand(SC_START_POOL_SEQUENCE,0);
	
}


void InitStandalone(){
/*
char stInitStandalone=isSendType;
char RunInitStandalone=True;

RunInitStandalone=True;
stInitStandalone=isSendType;

while (RunInitStandalone==True){
     switch(stInitStandalone){
	 case isSendType:
	      IsStandaloneAcknoledge=False;
          SendSlaveCommand(SC_STANDALONE,DispenserBrand);
		  TimSend=0;
		  stInitStandalone=isWaitAcknoledge1;
	      break;
     case isWaitAcknoledge1:
          if (IsStandaloneAcknoledge==True){
		      IsStandaloneAcknoledge=False;
		      stInitStandalone=isSendPumpConfig;
		  }
     case isSendPumpConfig:
          SendSlaveCommand(SC_SET_POOLING_MAX_PUMP,eeprom_read_byte(&DefPoolingPumpMax));
		  TimSend=0;
		  stInitStandalone=isWaitAcknoledge2;
	      break;
     case isWaitAcknoledge2:
          if (IsStandaloneAcknoledge==True){
		      IsStandaloneAcknoledge=False;
		      stInitStandalone=isFinishInitStandalone;
		  }
	      break;
     case isFinishInitStandalone:
	      RunInitStandalone=False;
	      break;
	 }
  }
  */
}


void DisplayQueueFIP(){// FIP:#1#2#3#4#5

}

void DisplayDateTime(){
char lcdteks[20];
	 _datetime(0, strSystemDate, strSystemTime);
	 sprintf_P(lcdteks,PSTR("%s %s"),strSystemDate,strSystemTime);
	 lcd_print(2, 1, lcdteks);
}

void DisplayTicker(){
/*
static char stDisplayTicker=tiRight;
	switch(stDisplayTicker){
	case tiRight:
	     lcd_put(4, 1, '>');
		 TimTicker=0;
         stDisplayTicker=tiDelayRight;
	break;
	case tiDelayRight:
         if (TimTicker>TICKER_DELAY)stDisplayTicker=tiLeft;	     
	     break;
	case tiLeft:
	     lcd_put(4, 1, '<');
         TimTicker=0;
         stDisplayTicker=tiDelayLeft;
	break;
	case tiDelayLeft:
         if (TimTicker>TICKER_DELAY)stDisplayTicker=tiRight;	     
	     break;
	}
*/
    if (TimTicker>(TICKER_DELAY*2+1)){
	    lcd_put(4, 1, '>');
		TimTicker=0;
	}
    if (TimTicker==TICKER_DELAY){
	    lcd_put(4, 1, '<');
		TimTicker++;
	}

}

char FTestChar(){
char lcdteks[20],Result;
char i=0;
     Result=MENU_NONE;
     sprintf_P(lcdteks,PSTR("D:%d C:%c  "),i,i);
     lcd_print(1,1,lcdteks);
	 if (i==0xFF)Result=MENU_DONE;
	 i++;
     _delay_ms(6500);
  return Result;
}

void DisplayStandaloneSequence(char x,char y, char PoolingSequence){
     char MapSequence[8];

     //sprintf_P(MapSequence,PSTR("-\|/-|/"));
	 MapSequence[0]='-';
	 MapSequence[1]='`';//0x5C;
	 MapSequence[2]='|';
	 MapSequence[3]='/';
	 MapSequence[4]='-';
	 MapSequence[5]=164;//0x5C;
	 MapSequence[6]='|';
	 MapSequence[7]='/';
	 if (IsPoolingRestarted==False)lcd_put(x,y,MapSequence[(PoolingSequence%8)]);     
	 else if (IsPoolingRestarted==True){
	          IsPoolingRestarted=False;
	          lcd_put(x,y,'X');     
			  }

}

void DisplayIdle(){
static unsigned int iLoopDisplayIdle=0;
static char stDispIdle=diScan;
     char i,iDisp;
     char lcdteks[20],sFIP[3];

     iLoopDisplayIdle++;
     //Setting DateTime ON

	if ((iLoopDisplayIdle%50)==0){
	   if (IFType==IT_STANDALONE)DisplayPumpStatus();
	   if(eeprom_read_byte(&DefShowDateTime)) DisplayDateTime();
       else DisplayTicker();
	   //Display Standalone Sequnece
	   
	   if((IFType==IT_STANDALONE)&&(IsNewPoolingSequence==True)){
	       IsNewPoolingSequence=False;
		   DisplayStandaloneSequence(4,18,iSequencePooling);
	   }


	 }
     //DisplayPumpStatus Standalone Mode

	 //Display Printing FIP
	 switch (stDispIdle){
	 case diScan:
	      if (IsBusyIdlePrinting==True){
		       sprintf_P(lcdteks,PSTR("Printing FIP%s "),strFIP_ID);
			   lcd_print(3,1,lcdteks);
			   stDispIdle=diWaitNoBusy;
		  }         
	      break;
     case diWaitNoBusy:
	      if (IsBusyIdlePrinting==False){
		       sprintf_P(lcdteks,PSTR("                    "));
			   lcd_print(3,1,lcdteks);
			   stDispIdle=diScan;
			   if (nLocalAccount>0)IsViewFillingFIP=True;
		  }
	      break;     
	 }
	 //Display Filling FIP
	 if (IsViewFillingFIP==True){
	     IsViewFillingFIP=False;
		 if (nLocalAccount>0){
		     lcd_printf(3,1,PSTR("@FIP:"));
		     for (iDisp=0;iDisp<nLocalAccount;iDisp++){
			      //leadingZero(LocalAccountFIP[iDisp],sFIP);
				  sprintf_P(sFIP,PSTR("%.2d"),LocalAccountFIP[iDisp]);
			      sprintf_P(lcdteks,PSTR("#%s"),sFIP);
			      lcd_print(3,6+(iDisp*3),lcdteks);
		     }
		 }else{
		 lcd_printf(3,1,PSTR("                    "));
		 }    
	 }
	 //Filling @FIP 
     if (IsCompleteFilling==True){
	     IsCompleteFilling=False;
	     if (nLocalAccount>0){
			 for(i=0;i<nLocalAccount;i++){//Shift data
			     LocalAccountFIP[i]=LocalAccountFIP[i+1];
			 }
			 nLocalAccount--;
             IsViewFillingFIP=True; 
		 }
	 }
}

void ShowMessage(char *Message){//Display Message on Line3
     char i;
}


char UserInput(char TypeUI,char xPos, char yPos,char *strResult, unsigned int MaxValue, char MaxLength){
     char Result;
	 static char UserInputResult=USER_NONE;
	 static char stUserInput=uiInit,xChar=0,yChar=0,iValuePos=0,zKeyChar,zAlphaChar,AlphaChar,IsDelete=False;
	 static char iHit=0,IsShifted=False,IsSameKey=False,IsNextKey=False,IsNewKey=False,IsFirst=False;//,KeyByte=0;
	 static unsigned int iLoop=0,KeyTimeout;//,TimerPressed=0,NewKeyTimeout;
	        unsigned int NumbValue=0;
	 char KeyPressed=0,KeyChar=0,iDisp=0;
     Result=USER_NONE;
	 switch(stUserInput){// 100 ->199 500 90
	 case uiInit:
	      iLoop=0;
		  stUserInput=uiInput;
		  xChar=xPos;yChar=yPos;
          iValuePos=0;
		  KeyTimeout=0;
		  IsSameKey=False;
		  IsNextKey=False;
		  IsNewKey=False;
		  IsShifted=True;
		  zKeyChar=' ';
		  iHit=0;
		  IsFirst=False;
		  IsDelete=False;
		  Result=USER_NO_DATA;
		  stUserInput=uiInput;
	      break;
	 case uiInput:
	      KeyPressed=_key_scan(1);                  //  _  ABC DEF GHI JKL MNO PQRS TUV WXYZ
		  KeyChar= _key_btn(KeyPressed);            //  1   2   3   4   5   6   7    8   9
		  if (KeyPressed==_KEY_SHIFT){
		      if (IsShifted==False)IsShifted=True;
			  else
		      if (IsShifted==True)IsShifted=False;
			  }
          
		  if (IsSameKey==True){
		      KeyTimeout++;
              if (KeyTimeout>1000){
			      if (TypeUI==UI_ALPHANUM_PASSWORD)lcd_put(xChar,yChar,'*');

		          IsNextKey=True;
				  iLoop=0;
			     }
              }
            
 
 		  //if (((KeyChar>='0')&&(KeyChar<='9')&&(iValuePos<=MaxLength))||(IsNextKey==True)){
		  if (((KeyChar>='0')&&(KeyChar<='9')&&(iValuePos<MaxLength))||(IsNextKey==True)){
		       _delay_ms(250);
		       strResult[iValuePos]=KeyChar;
		       strResult[iValuePos+1]=0;

			   if ((TypeUI==UI_NUMBER_R)||(TypeUI==UI_NUMBER_L)||(TypeUI==UI_NUM_PASSWORD)) 
			        NumbValue=atoi(strResult);
			   else NumbValue=0;
				
			   if ((TypeUI==UI_ALPHANUM_R)||(TypeUI==UI_ALPHANUM_PASSWORD)){
			       if (KeyChar!=zKeyChar){
					   IsSameKey=False;
					   IsNewKey=True;
				       iHit=0;
					   iLoop=1000;
				   }
                   else{
				   if (IsNewKey==True){
					   IsNewKey=False;
					   yChar--;
					   if (iValuePos>0) iValuePos--;
					   }
				   KeyTimeout=0;
				   IsSameKey=True;
				   if (iHit<pgm_read_byte(&MaxKeyHit[(zKeyChar-'0')])) 
				        iHit++;
                   else iHit=0;
				   iLoop=1000;

                   zAlphaChar=AlphaChar;
			       AlphaChar=_table_alphanum(IsShifted,KeyPressed,iHit);			   
                   
				   if (IsNextKey==True){
                       AlphaChar=zAlphaChar;       
					   IsNextKey=False;
					   IsSameKey=False;
				   }   
				   strResult[iValuePos]=AlphaChar;
		           strResult[iValuePos+1]=0;
				 }//endElse 
				 
			   }//EndAlphaNum

			 if (TypeUI==UI_NUMBER_L){
			    if ((NumbValue<=MaxValue)||(MaxValue==0)){
				     iValuePos++;
					 stUserInput=uiInputDisp;
				} 
			 }
  
			   if ((NumbValue<=MaxValue)||(MaxValue==0)){
			       if ((TypeUI==UI_NUMBER_R)||(TypeUI==UI_ALPHANUM_R)||
				       (TypeUI==UI_NUM_PASSWORD)||(TypeUI==UI_ALPHANUM_PASSWORD)){
				      if (IsSameKey==False){
					      IsFirst=True;
					      iValuePos++;
						  //FullEntry
						  if (iValuePos==MaxLength)Result=USER_FULL_ENTRY;
					      yChar++;
						  //yChar=(yChar+1;//Max Display

						  }
                  stUserInput=uiInputDisp;
				  }
			   }
              zKeyChar=KeyChar;
             Result=USER_ENTRY;
             }//EndKeyChar
			 
                 
              if ((TypeUI==UI_NUMBER_R)||(TypeUI==UI_NUMBER_L)||(TypeUI==UI_NUM_PASSWORD)){
                   if((NumbValue>MaxValue)||(iValuePos==MaxLength)){
			           if (MaxValue>0)system_beep(1);
				   }
                 }
		  
		  //CANCEL OK
		  if (KeyPressed==_KEY_CANCEL){
		      if (iValuePos>0){
			      iLoop=1999;
				  stUserInput=uiClearDisplay;
				  }
			  else{
			      UserInputResult=USER_CANCEL;
			      stUserInput=uiFinished;
				  }
		  }
		  else {
		  if (KeyPressed==_KEY_ENTER){
		      strResult[iValuePos]=0;
			  if ((TypeUI==UI_NUMBER_R)||(TypeUI==UI_NUMBER_L)){
			       if (iValuePos==0)sprintf_P(strResult,PSTR("0"));
			       RemZeroLead(strResult);
				  }
		      UserInputResult=USER_OK;
		      stUserInput=uiFinished;
			  }
		  }//EndElse

		  if ((iLoop%2000)==0){
		      if (IsSameKey==True)
			      //lcd_put(xChar,yChar,AlphaChar); 
				  lcd_put(xPos+((yPos+iValuePos-1)/20),yPos+(iValuePos%20),AlphaChar); 
			  else {
			      if((TypeUI==UI_ALPHANUM_PASSWORD)&&(iValuePos>0))lcd_put(xChar,yChar-1,'*'); 
			      //lcd_put(xChar,yChar,'_'); 
				  lcd_put(xPos+((yPos+iValuePos-1)/20),yPos+(iValuePos%20),'_'); 

				  if (IsFirst==True){
				      IsFirst=False;
				      zKeyChar=0;
					  }
				  }
			  }
          if ((iLoop%2000)==1000){
		      if (IsSameKey==True){
			      if (TypeUI!=UI_NUM_PASSWORD)
				       //lcd_put(xChar,yChar,AlphaChar); 
					   lcd_put(xPos+((yPos+iValuePos-1)/20),yChar%21,AlphaChar); 
					   
				  else lcd_put(xChar,yChar,'*'); 
			  }
			  //else lcd_put(xChar,yChar,' '); 
			  else lcd_put(xPos+((yPos+iValuePos-1)/20),yPos+(iValuePos%20),' '); 			  
			  }
          iLoop++;
          break;
     case uiClearDisplay:
          if (TypeUI==UI_NUMBER_L){
		      lcd_put(xChar,(yChar-iValuePos),' '); // 123_ 1_
		      strResult[iValuePos]=0;
		      iValuePos--;
			  }
          else
		  if ((TypeUI==UI_NUMBER_R)||(TypeUI==UI_ALPHANUM_R)||
		      (TypeUI==UI_NUM_PASSWORD)||(TypeUI==UI_ALPHANUM_PASSWORD)){
		      //lcd_put(xChar,yChar,' '); // 123_ 1_
			  lcd_put(xPos+((yPos+iValuePos-1)/20),yPos+(iValuePos%20),' '); // 123_ 1_
			  
		      strResult[iValuePos]=0;
		      iValuePos--;
			  yChar--;
			  IsDelete=True;
		  }
		  if (iValuePos==0)Result=USER_NO_DATA;		  
		  stUserInput=uiInputDisp;//Redraw
	      break;      
     case uiInputDisp:
	      if(TypeUI==UI_ALPHANUM_PASSWORD){
			  if (iValuePos>0){
                  lcd_put(xChar,(yChar-2),'*');
			      if (IsDelete==True){
				      IsDelete=False;
			          lcd_put(xChar,(yChar-1),'*');
			          }
				  else lcd_put(xChar,(yChar-1),strResult[iValuePos-1]);
			      }
			  else{ if(IsDelete==True){
			           IsDelete=False;
					   lcd_put(xChar,(yChar-1),'*');
			           }
			        else lcd_put(xChar,(yChar-1),strResult[iValuePos]);
			  }
		  }else
	      if (TypeUI==UI_NUMBER_L){
	          for(iDisp=0;iDisp<iValuePos;iDisp++){
				  lcd_put(xChar,(yChar-(iValuePos-iDisp)),strResult[iDisp]);
		      }
		  }else
	      if ((TypeUI==UI_NUMBER_R)||(TypeUI==UI_ALPHANUM_R)){
		      
			  //if (iValuePos>0) lcd_put(xChar,(yChar-1),strResult[iValuePos-1]);
			  if (iValuePos>0) lcd_put(xPos+((yPos+iValuePos-1)/21),yPos+((iValuePos-1)%20),strResult[iValuePos-1]);			  
			  else{ if ((yChar-1)>=yPos)lcd_put(xChar,(yChar-1),strResult[iValuePos]);
			  }
		  }else
		  //DisplayAsterik *
		  if (TypeUI==UI_NUM_PASSWORD){
		      lcd_put(xChar,(yChar-1),'*');
		  }

          stUserInput=uiInput;
	      break;	 
     case uiFinished:
	      Result=UserInputResult;
	      stUserInput=uiInit;
	      break;
	 }
	 return Result;
}





void systemSlave(){
    _spi_enable(_SPI_NONE);	
  	_spi_init(0, 1);         //Slave
	sbi(DDRB,3);             //MISO Output
	sbi(DDRB,3);sbi(PORTB,3);//MISO Output
	cbi(DDRB,2);sbi(PORTB,2);//MOSI Input
	cbi(DDRB,1);sbi(PORTB,1);//SCK  Input
}

void systemMaster(){
  	_spi_init(1, 0);         //Master
    _spi_enable(_SPI_SLAVE); //Enable SS 
	cbi(DDRB,3);sbi(PORTB,3);//MISO Input
	sbi(DDRB,2);             //MOSI Output
}

void EDCSendByte(char EDCData){
     systemMaster();	 
     _spi(EDCData);
     systemSlave();
}

char CalcLRC(char xLRC,char DataIn){
     char Result;
	 Result=xLRC^DataIn;
     return(Result);
}

void SendEDCMessage(){
char i,xCRC,SerialEDC[60];

     //GenerateData
	 //sprintf_P(strAmount,PSTR("%d"),100);//Testing Only
	 //AddZeroLead(strAmount,8);
	 //uart_printf(0,0,PSTR("StrStatus:"));uart_print(0,1,strStatus);
     sprintf_P(SerialEDC,PSTR("02%s%s%s%s%s%s%s"),strTranNo,strFIP_ID,strDescription,strPrice,strVolume,strAmount,strStatus);     

	 //SendingData
	 xCRC=0;EDCSendByte(0x02);
	 xCRC=CalcLRC(xCRC,0x02);	      
	 for (i=0;i<strlen(SerialEDC);i++){
          xCRC=CalcLRC(xCRC,SerialEDC[i]);
          EDCSendByte(SerialEDC[i]);
		  //uart(0,1,SerialEDC[i]);
		  _delay_ms(SPI_EDC_DELAY);//min:8
	 }
     EDCSendByte(0x03);
	 xCRC=CalcLRC(xCRC,0x03);	      
	 EDCSendByte(xCRC);
}

void systemEDC(){//EDC Handler
     

}

void systemPrinting(){
	 FreePrinting();
	 PrintIdle();
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


char menu_FIP(char *xFIP,char *sFIPUsed){
static char stEnterFIP=efInit;
static char KeyCharLast=' ',FIP_Used=0;
static unsigned int iLoop;

       char KeyPressed=0,KeyChar;
       char Result=FIP_NONE;
       char FIP[8],strPumpL[3],strPumpR[3],lcdteks[20];

       switch(stEnterFIP){
 	   case efInit:
	 	    eeprom_read_block((void*) &FIP, (const void*) &DefPumpMap, 8);
		    lcd_clear(); 
		    for (iLoop=0;iLoop<8;iLoop++){
			     if (FIP[iLoop]>99) FIP[iLoop]=0;
		    }
		    for(iLoop=0;iLoop<4;iLoop++){
		        leadingZero(FIP[iLoop],strPumpL);leadingZero(FIP[iLoop+4],strPumpR);
                sprintf_P(lcdteks,PSTR("%d.P%s | %d.P%s"),(iLoop+1),strPumpL,(iLoop+5),strPumpR);
			    lcd_print((iLoop+1),1,lcdteks);
			}
		    lcd_printf(1,15,PSTR("FIP:_"));    //"1.P01 | 5.P05 FIP:_ "                                                                      
		    lcd_printf(2,15,PSTR("     "));    //"2.P02 | 6.P05       "
		    lcd_printf(3,15,PSTR("#)OK "));    //"3.P03 | 7.P07 #)OK  "
	        lcd_printf(4,15,PSTR("*)Exit"));   //"4.P04 | 8.P08 *)Exit"				
		    iLoop=0;
		    stEnterFIP=efFIPInput;
	        break;
	   case efFIPInput:
		    iLoop++;
		    KeyPressed=_key_scan(1);
		    KeyChar=_key_btn(KeyPressed);
		    if ((KeyChar>='1')&&(KeyChar<='8')){
		 	     if (KeyCharLast!=KeyChar){
				     KeyCharLast=KeyChar;
				     iLoop=1000;
                 }
		    }
 		    if ((iLoop%2000)==0){
		        lcd_xy(1,19);_lcd('_'); 
			   }
           if ((iLoop%2000)==1000){
			    lcd_xy(1,19);_lcd(KeyCharLast); 
			   }

		   if (((KeyChar>='1')&&(KeyChar<='8'))||(KeyPressed==_KEY_ENTER)&&(KeyCharLast!=' ')){ 
		         FIP_Used=FIP[KeyCharLast-'1'];
				 if (FIP_Used<=99){
                     //leadingZero(FIP_Used,sFIPUsed);
					 //xFIP[0]=FIP_Used;
					 *xFIP=FIP_Used;
					 sprintf_P(sFIPUsed,PSTR("%.2d"),FIP_Used);

					 }
				 Result=FIP_DONE;
                 stEnterFIP=efExitFIPInput;
		   }
		   else
		   if (KeyPressed==_KEY_CANCEL){
               Result=FIP_CANCEL;
			   stEnterFIP=efExitFIPInput;
		   }
	       break;
      case efExitFIPInput:
           stEnterFIP=efInit;
	       break;
	 }
return Result;
}

char FMenuReprint(){
static char stReprint=rtInit;
static char KeyPressed=0,FIP_Used=0;
       char lcdteks[20],FIPResult,KeyChar;//,FIP_USED;
	   char PassResult,Result=MENU_NONE,ReprintResult;

     Result=MENU_NONE;
	 switch(stReprint){
	 case rtInit:
	      lcd_clear();
          if (IFType==IT_SLAVE)stReprint=rtFIP;
		  else
		  if (IFType==IT_STANDALONE)stReprint=rtStandaloneFIP;//rtValidPassword;//rtStandaloneFIP;
	      break;
     case rtValidPassword:
	      PassResult=FMenuPassword();
	      if (PassResult==MP_VALID_ADMIN)stReprint=rtStandaloneFIP;
		  else
	      if (PassResult==MP_VALID_SYSTEM)stReprint=rtInvalidPassword;
          else
	      if (PassResult==MP_INVALID)stReprint=rtInvalidPassword;
		  else
	      if (PassResult==MP_CANCEL)stReprint=rtExitReprint;
	      break;
     case rtInvalidPassword:
          TimDisplay=0;
		  system_beep(1);
		  lcd_printf(3,1,PSTR("Access Denied"));
          stReprint=rtTimDisplayInvalid;
	      break;
     case rtTimDisplayInvalid:
	      if (TimDisplay>2)stReprint=rtExitReprint;
	      break;
     case rtStandaloneFIP:
	      _scr_pump();
		  stReprint=rtStandaloneInputFIP;
	      break;
     case rtStandaloneInputFIP:
		  KeyChar=_key_btn(_key_scan(1));
		  if ((KeyChar>='1')&&(KeyChar<='8')){
		       SendPoolingCommand(SC_TRANSACTION,GetPumpID(KeyChar-'0'));
			   TimSend=0;
			   while(TimSend<6){};
		       ReprintResult=PrintStandalone(KeyChar-'0',True);
			   if (ReprintResult==PS_NO_DATA)stReprint=rtDisplayNoTransaction;
			   else if (ReprintResult==PS_PRINTED)stReprint=rtExitReprint;               
			   //stReprint=rtExitReprint;
		  }else if (KeyChar=='*')stReprint=rtExitReprint;
	      break;
     case rtDisplayNoTransaction:
	      lcd_printf(3, 1,PSTR("Tidak Ada Transaksi "));
		  system_beep(1);
          TimDisplay=0;
		  stReprint=rtTimDisplayInvalid;
	      break;
	 case rtFIP:
	      FIPResult=menu_FIP(&FIP_Used,strFIP_ID);
		  if (FIPResult==FIP_DONE){
		      stReprint=rtRFID;
			  }
		  else
		  if (FIPResult==FIP_CANCEL)stReprint=rtExitReprint;
	      break;
	 case rtRFID:
	      lcd_clear();
		  sprintf_P(lcdteks,PSTR("RePrint FIP:%s "),strFIP_ID);lcd_print(1,1,lcdteks);
		  lcd_printf(2,1,PSTR(  "Tap Supervisor Card"));
          lcd_printf(4,1,PSTR(  "[*]Back    [#]Exit"));
		  IsRFIDDetected=False;
		  stReprint=rtInputRFID;
	      break;
	 case rtInputRFID:
	      if (IsRFIDDetected==True){
		      IsRFIDDetected=False;
		      stReprint=rtSendMessage28;
		  }
	 	  KeyPressed=_key_scan(1);
		  if (KeyPressed==_KEY_CANCEL){
		      stReprint=rtFIP;
			  }
		  else
		  if (KeyPressed==_KEY_ENTER)stReprint=rtExitReprint;
	      break;
     case rtSendMessage28:
		  sendMessage28();
		  TimSend=0;
		  stReprint=rtWaitReply;
		  break;
     case rtWaitReply:
	      if (TimSend>TIM_SEND*3)stReprint=rtNoConnection;
	      if (IsMessage99==True){
		      stReprint=rtExitReprint;
		  }
	      break;
     case rtNoConnection:
	      lcd_clear();
		  lcd_printf(2,1,PSTR("TCP/IP ERROR "));
		  lcd_printf(3,1,PSTR("No Connection"));
		  TimDisplay=5;
		  stReprint=rtDelayExitReprint;
	      break;
     case rtDelayExitReprint:
	      if(TimDisplay>=10)stReprint=rtExitReprint;
	      break;
	 case rtExitReprint:
	      stReprint=rtInit;
		  Result=MENU_DONE;
	      break;
	 }
  return Result;
}

char FMenuLoyalty(){
static char stLoyalty=mlInit;
static char FIP_Used=0,IsLoyaltyUpdate=False;
       char lcdteks[20];
       char FIPResult,FIP_USED;
	   char Result=MENU_NONE;
	   char KeyPressed=0,KeyChar;

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

char FMenuEDCTransaction(){
static char stEtransaction=etInit;
static char FIP_Used=0;
       char lcdteks[20],Result=MENU_NONE;
       char KeyPressed=0,KeyChar,FIPResult;

     Result=MENU_NONE;
	 switch(stEtransaction){
	 case etInit:
	      lcd_clear();
		  lcd_printf(1,1,PSTR("Select Card Type"));
          lcd_printf(1,1,PSTR("1.Debit/Flash Card  "));
		  lcd_printf(2,1,PSTR("2.Credit Card       "));
	      lcd_printf(4,1,PSTR("[*]Back             "));
		  stEtransaction=etInputEDC;
	      break;
     case etInputEDC:
	 	  KeyPressed=_key_scan(1);
		  KeyChar=_key_btn(KeyPressed);
		  if ((KeyChar>='1')&&(KeyChar<='2')){
		       EDCType=KeyChar-'0';
		       stEtransaction=etSelectFIP;
		     }
		  if (KeyPressed==_KEY_CANCEL)stEtransaction=etExitEDCTransaction;
	      break;
	 case etSelectFIP:
	      FIPResult=menu_FIP(FIP_Used,strFIP_ID);
		  if (FIPResult==FIP_DONE)stEtransaction=etInitMessage90;
		  else
		  if (FIPResult==FIP_CANCEL)stEtransaction=etInit;
	      break;
     case etInitMessage90:
	      FillChar(strRef1,sizeof(strRef1),0);
	      if (EDCType==1){
		      AddSpaceLag(strRef1,20);
          }
		  else
	      if (EDCType==2){
		      sprintf_P(strRef1,PSTR("CREDITCARD"));
			  AddSpaceLag(strRef1,20);
		  }	 
	      stEtransaction=etSendingMessage90;
	      break;
     case etSendingMessage90:
          sendMessage90();
		  TimSend=0;
          stEtransaction=etWaitReply;
	      break;
     case etWaitReply:
	      if (TimSend>TIM_SEND*2)stEtransaction=etNoConnection;
		  if ((IsMessage91==True)||(IsMessage09==True))
		       stEtransaction=etSuccesEDC;
	      break;
     case etSuccesEDC:
	      stEtransaction=etDisplayFreeMessage;
	      break;
     case etDisplayFreeMessage:
	      if (IsMessage09==True){
		      IsMessage09=False;
	          procMessage09();
		      lcd_clear();
		      lcd_print(1,1,strFreeMessageLine1);
		      lcd_print(2,1,strFreeMessageLine2);
		      lcd_print(3,1,strFreeMessageLine3);
			  lcd_print(4,1,strFreeMessageLine4);
			  TimDisplay=0;
			  }
		  stEtransaction=etDelayExit;
	      break;
     case etNoConnection:
	      lcd_clear();
		  TimDisplay=0;
		  stEtransaction=etDelayExit;
	      break;
     case etDelayExit:
	      if (TimDisplay>8)stEtransaction=etExitEDCTransaction;
	      break;
     case etExitEDCTransaction:
	      stEtransaction=etInit;
	      Result=MENU_DONE;
	      break;
	 }
   return Result;
}

char FMenuChangeMOP(){
static char stChangeMOP=cmInit;
static char FIP_Used=0;//,ValueChar[10];
static char BankIdx=0;       
static char iPos=0,iWait=0;
static unsigned int iLoop=0;

       char lcdteks[20],i,msgResult;       
	   char strBankName[11],strSurcharge[4],Result;
       char uiResult,KeyPressed=0,KeyChar,FIPResult;

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
	      //uiResult=UserInput(UI_NUMBER_L,2,14,ValueChar,100,3);
		  uiResult=UserInput(UI_NUMBER_L,2,14,strSurcharge,100,3);
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



char FMenuAuthorization(){
char Result=MENU_NONE,PassResult=MP_NONE,SubMenu=MENU_NONE;
static char stMenuAuthorization=maInit,PassTry=0;
     switch(stMenuAuthorization){
	 case maInit:
	      PassTry=0;
		  stMenuAuthorization=maInputPassword;
	      break;
	 case maInputPassword:
	      PassResult=FMenuPassword();
		  switch(PassResult){
		  case MP_VALID_ADMIN:
		       stMenuAuthorization=maMenuAdmin;
		       break;
		  case MP_VALID_SYSTEM:
		       stMenuAuthorization=maMenuSettings;
		       break;
		  case MP_INVALID:
		       stMenuAuthorization=maInvalidAuthorization;
		       break;
		  case MP_VALID_MASTER:
		       stMenuAuthorization=maMenuMaster;
		       break;
		  case MP_CANCEL:
		       stMenuAuthorization=maExitAuthorization;
		       break;
		  }
		  /*
	      if (PassResult==MP_VALID_ADMIN)stMenuAuthorization=maMenuAdmin;
		  else
	      if (PassResult==MP_VALID_SYSTEM)stMenuAuthorization=maMenuSettings;
          else
	      if (PassResult==MP_INVALID)stMenuAuthorization=maInvalidAuthorization;
          else
		  if (PassResult==MP_VALID_MASTER)stMenuAuthorization=maMenuMaster;
		  else
	      if (PassResult==MP_CANCEL)stMenuAuthorization=maExitAuthorization;
		  */
	      break;
	 case maMenuMaster:
	      SubMenu=FMenuMaster();
		  if (SubMenu==MENU_DONE)stMenuAuthorization=maExitAuthorization;
	      break;
	 case maMenuAdmin:
          SubMenu=FMenuAdmin();
		  if (SubMenu==MENU_DONE)stMenuAuthorization=maExitAuthorization;          
		  break;
	 case maMenuSettings:
	      SubMenu=FMenuSettings();
		  if (SubMenu==MENU_DONE)stMenuAuthorization=maExitAuthorization;
          break;	 
	 case maInvalidAuthorization:
	      PassTry++;
	      TimDisplay=0;
		  system_beep(1);
		  lcd_printf(3,1,PSTR("Access Denied"));
		  stMenuAuthorization=maDelayExitAuthorization;
          break;	 
	 case maDelayExitAuthorization:
	      if (TimDisplay>2){
		      if (PassTry<3)stMenuAuthorization=maInputPassword;
			  else stMenuAuthorization=maExitAuthorization;
		  }
	      break;
	 case maExitAuthorization:
	      stMenuAuthorization=maInit;
	      Result=MENU_DONE;
	      break;
	 }
   return Result;	 
}



char FMenuPassword(){
static char stMenuPasword=mpInitPassword,strPassword[10],PassStatus=MP_NONE;;
       char Result=MP_NONE,strMasterPass[10],strSystemPass[10],strAdminPass[10];
       char uiResult;

     Result=MP_NONE;
     switch(stMenuPasword){
	 case mpInitPassword:
	      lcd_clear();
		  lcd_printf(1,1,PSTR("PASSWORD:"));
		  lcd_printf(2,1,PSTR("_"));
		  lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
		  FillChar(strPassword,0,sizeof(strPassword));
		  stMenuPasword=mpInputPasword;
	      break;
     case mpInputPasword:
	 	      uiResult=UserInput(UI_NUM_PASSWORD,2,1,strPassword,0,8);
		  if (uiResult==USER_OK){
		      stMenuPasword=mpProcessPassword;
			  }
		  else
		  if (uiResult==USER_CANCEL){
		      PassStatus=MP_CANCEL;
		      stMenuPasword=mpExit;
			  }
		  else
		  if (uiResult==USER_ENTRY)lcd_printf(4,1,PSTR("[*]Back     [#]Enter"));
		  else
		  if (uiResult==USER_NO_DATA)lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
          break;	 
	 case mpProcessPassword:
		      
	      sprintf_P(strMasterPass,PSTR(MASTER_PASSWORD)); 
	      eeprom_read_block((void*) &strAdminPass, (const void*) &DefSpvPassword, 10);
	      eeprom_read_block((void*) &strSystemPass, (const void*) &DefSysPassword, 10);
		  
		  if (strcmp(strMasterPass,strPassword)==0)
		       PassStatus=MP_VALID_MASTER;
		  else
	      if (strcmp(strAdminPass,strPassword)==0){
		       PassStatus=MP_VALID_ADMIN;
               //lcd_printf(3,1,PSTR("Admin"));
               //_delay_ms(5000);
			   }
		  else if (strcmp(strSystemPass,strPassword)==0){
		       PassStatus=MP_VALID_SYSTEM;
			   //lcd_printf(3,1,PSTR("System"));
               //_delay_ms(5000);
			   }
		  else PassStatus=MP_INVALID;
		  stMenuPasword=mpExit;
	      break;
	 case mpExit:
	      //lcd_print(3,1,strPassword);
		  //_delay_ms(5000);
	      stMenuPasword=mpInitPassword;
	      Result=PassStatus;
	      break;
	 }
   return Result;
}


char GeniusCalc(char cOption, char valA, char valB){
     char valC_H,valC_L,xValA,xValB,Result;
	 Result=0;
     switch(cOption){
	 case G_PLUS:                  // 5 + 8 = 1 + 3 = 4
	      valC_H=(valA+valB)/10;   //  
		  valC_L=(valA+valB)%10;   //
	      Result=valC_H+valC_L;
	      break;
	 case G_MULTY:                 
	      if (valA>0)xValA=valA;
		  else xValA=1;
	      if (valB>0)xValB=valB;
		  else xValB=1;
	      valC_H=(xValA*xValB)/10;   //  
		  valC_L=(xValA*xValB)%10;   //
	      Result=GeniusCalc(G_PLUS,valC_H,valC_L);
	      break;
	 case G_MINUS:
	      
	      break;
	 }
   return Result;
}

void WrapCode(char *strRawCode){
     char i,seedKey;
	 char strTemp[15];
	 //62678677 ->68846445
	 seedKey=0;
	 for (i=0;i<strlen(strRawCode);i++){
	      strTemp[i]='0'+GeniusCalc(G_PLUS,(strRawCode[i]-'0'),seedKey);
		  seedKey=strRawCode[i]-'0';
	 }strTemp[strlen(strRawCode)]=0;
     sprintf_P(strRawCode,PSTR("%s"),strTemp);
}

void GenerateGeniusCode(char *srcDate, char cSeed, char *strDestCode){
     /*          050111
	    srcDate: 05012011 
	    seed[S]: 66666666
		-------------------		         
		    1st->62678677 
	     WrapC ->68846545
	 */ 
	 char i,GCalcOpt,AddYear[3],SYear[5];
	 char strSend[20];

   #ifdef DEBUG_GENIUS_CODE
	    sprintf_P(strSend,PSTR("Date[%s]"),srcDate);
		uart_print(1,1,strSend);
     #endif
    

	 StrPosCopy(srcDate,AddYear,strlen(srcDate)-2,2);

    #ifdef DEBUG_GENIUS_CODE
	    sprintf_P(strSend,PSTR("Year:[%s]"),AddYear);
		uart_print(1,1,strSend);
     #endif

	 sprintf_P(SYear,PSTR("20%s"),AddYear);
	 StrPosPaste(SYear,srcDate,strlen(srcDate)-2,strlen(SYear));




	 #ifdef DEBUG_GENIUS_CODE
	    sprintf_P(strSend,PSTR("NewDate[%s]"),srcDate);
		uart_print(1,1,strSend);
     #endif

	 sprintf_P(strDestCode,PSTR("%c"),cSeed);     
	 GCalcOpt=G_PLUS;
	 for (i=1;i<strlen(srcDate);i++){
	      strDestCode[i]='0'+GeniusCalc(GCalcOpt,(cSeed-'0'),(srcDate[i]-'0'));	 
	 }strDestCode[strlen(srcDate)]=0;

	 #ifdef DEBUG_GENIUS_CODE
       sprintf_P(strSend,PSTR("1st:%s"),strDestCode);
	   uart_print(1,1,strSend);     
	 #endif

	 WrapCode(strDestCode);
	 #ifdef DEBUG_GENIUS_CODE
	   sprintf_P(strSend,PSTR("Wrap:%s"),strDestCode);
	   uart_print(1,1,strSend);     
	 #endif
}

void RemoveChar(char *strSource, char cRem){
     char i,iAdd,sTemp[20];
     iAdd=0;
	 for(i=0;i<strlen(strSource);i++){
	     if (strSource[i]!=cRem){
		     sTemp[iAdd]=strSource[i];
			 iAdd++;
          }
	 }sTemp[iAdd]=0;
	 sprintf_P(strSource,PSTR("%s"),sTemp);
}

char ValidateGeniusCode(char *sDate, char *sGenCode){//==GC_VALID
     char Result=GC_NONE;
	 char sAutoGen[10],strSend[20];
     Result=GC_NONE;

         #ifdef DEBUG_GENIUS_CODE
		 sprintf_P(strSend,PSTR("[%s]"),sGenCode); 
         uart_print(1,1,strSend);
		 #endif
	 if (strlen(sGenCode)==8){//Length musti  8
	     //sDate: 05012001		 
		 GenerateGeniusCode(sDate,sGenCode[0],sAutoGen);

         #ifdef DEBUG_GENIUS_CODE
		 sprintf_P(strSend,PSTR("%s|%s"),sAutoGen,sGenCode); 
         uart_print(1,1,strSend);
		 #endif

         if (strcmp(sAutoGen,sGenCode)==0)
	        Result=GC_VALID;	 
         else Result=GC_INVALID;
	 }


   return Result;
}

void GenerateKeyStamp(char *sTime, char *sGCode, char *strKeyStamp){//
	 /*  sTime : 11452584
	     sGCode: 68846445 [S]
		        ----------
       KeyStamp: 68623252
	             65585577
	 */
     char seedIdx,sSeed[10],sAdd[3],i;
     
	 seedIdx=(sTime[5]-'0')%8; 
	 sAdd[0]=sGCode[seedIdx];
     seedIdx=(sTime[6]-'0')%8;
	 sAdd[1]=sGCode[seedIdx];
	 sAdd[2]=0;

	 sprintf_P(sSeed,PSTR("%s%s"),sTime,sAdd);
     for(i=0;i<strlen(sGCode);i++){
	     strKeyStamp[i]='0'+GeniusCalc(G_MULTY,(sSeed[i]-'0'),(sGCode[i]-'0'));		 
	 }strKeyStamp[strlen(sGCode)]=0;
     WrapCode(strKeyStamp);
}

char ValidateRestoreCode(char *sKeyStamp, char *sRestoreCode){//==RC_VALID
     char Result=RC_NONE;
	 char i,nSum;
	 char strSend[30];

	 //  KeyStamp: [6]5585577 WrapCode6x
	 //sprintf_P(strSend,PSTR("KeyStamp:%s"),sKeyStamp);
	 //uart_print(1,1,strSend);

	 nSum=sKeyStamp[0]-'0';
	 for (i=0;i<nSum;i++){
         WrapCode(sKeyStamp);
	 }

	 //sprintf_P(strSend,PSTR("WrapStamp:%s"),sKeyStamp);
	 //uart_print(1,1,strSend);
     
	 //sprintf_P(strSend,PSTR("WrapStamp:%s"),sRestoreCode);
	 //uart_print(1,1,strSend);

	 if (strcmp(sKeyStamp,sRestoreCode)==0)
	     Result=RC_VALID;	 
     else Result=RC_INVALID;

   return Result;
}

char FMenuMaster(){ 
static char stMenuMaster=mmInitMaster;
     char KeyPressed,KeyChar,Result=MENU_NONE;
	 char uiResult,lcdteks[20],PTime[10],PDate[10],strNewPassword[10];
	 char strSend[20];

	 switch(stMenuMaster){
	 case mmInitMaster:
	      lcd_clear();
		  lcd_printf(1,1,PSTR("-System Admin Level-"));
		  lcd_printf(2,1,PSTR("1.Restore Password  "));
		  lcd_printf(3,1,PSTR("2.Master Reset      "));
		  lcd_printf(4,1,PSTR("[*]Exit             "));

		  //uart_printf(1,1,PSTR("-System Admin Level-"));

		  stMenuMaster=mmMasterSelect;
	      break;
     case mmMasterSelect:
	 	  KeyPressed=_key_scan(1);
		  KeyChar=_key_btn(KeyPressed);
          switch(KeyChar){
		  case '1':
               stMenuMaster=mmRestorePasswordMenu;
		       break;
		  case '2':
		       stMenuMaster=mmMasterReset;
		       break;
          case '*':
		       stMenuMaster=mmExitMaster;
		       break;
		  }
	      break;
     case mmMasterReset:
		  
	      _datetime(0, strSystemDate, strSystemTime);
	      sprintf_P(PTime,PSTR("%s"),strSystemTime);
		  sprintf_P(strGeniusCode,PSTR("22345678"));		  
		  RemoveChar(PTime,':');sprintf_P(strKeyStamp,PSTR(""));
		  GenerateKeyStamp(PTime,strGeniusCode,strKeyStamp);

		  lcd_clear();
		  lcd_printf(1,1,PSTR("-Master Reset-"));
		  sprintf_P(lcdteks,PSTR("Seed Code:%s"),strKeyStamp);
		  lcd_print(2,1,lcdteks);
		  lcd_printf(3,1,   PSTR("ResetCode:_         "));
		  lcd_printf(4,1,   PSTR("[*]Cancel   [#]Enter"));		          
	      stMenuMaster=mmResetCodeEntry;
	      break;
     case mmResetCodeEntry:
          uiResult=UserInput(UI_NUMBER_R,3,11,strRestoreCode,0,9);
		  switch(uiResult){
		  case USER_OK:
               stMenuMaster=mmIsValidResetCode;
		       break;
          case USER_CANCEL:
		       stMenuMaster=mmExitMaster;
		       break;
		  case USER_ENTRY:
		       lcd_printf(4,1,PSTR("[*]Back     [#]Enter"));
		       break;
          case USER_NO_DATA:
		       lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
		       break;
		  }	      
	      break;
     case mmIsValidResetCode:
	      if (ValidateRestoreCode(strKeyStamp,strRestoreCode)==RC_VALID){
		      stMenuMaster=mmMasterResetExec;
		  }else stMenuMaster=mmDisplayInvalidResetCode;
	      break;
     case mmMasterResetExec:
	      lcd_clear();
	      lcd_printf(1,1,PSTR("Reset EEPROM"));
	      lcd_printf(2,1,PSTR("Please Wait.."));
	      MasterReset();
          stMenuMaster=mmSuccesfullReset;
	      break;
     case mmSuccesfullReset:
	      lcd_clear();
		  system_beep(1);
	      lcd_printf(2,1,PSTR("Reset Complete"));
	      TimDisplay=0; 
		  stMenuMaster=mmDelayDisplayComplete;
	      break;
     case mmDisplayInvalidResetCode:
          system_beep(1);
          lcd_printf(3,1,PSTR("Invalid Code        "));
          TimDisplay=0; 
		  stMenuMaster=mmDelayDisplayInvalidResetCode;
	      break;
     case mmDelayDisplayInvalidResetCode:
          if (TimDisplay>5)stMenuMaster=mmExitMaster;
	      break;
     
	 case mmDelayDisplayComplete:
          if (TimDisplay>5)stMenuMaster=mmExitMaster;
	      break;

     case mmRestorePasswordMenu:	      
	      lcd_clear();
		  lcd_printf(1,1,PSTR("Enter GeNiUs Code   "));
		  lcd_printf(2,1,PSTR("_                   "));
		  lcd_printf(4,4,PSTR("[*]Cancel   [#]Enter"));
		  sprintf_P(strGeniusCode,PSTR(""));
          stMenuMaster=mmGeniusCodeEntry;
	      break;
     case mmGeniusCodeEntry:
	 	  uiResult=UserInput(UI_NUMBER_R,2,1,strGeniusCode,0,10);
		  switch(uiResult){
		  case USER_OK:               
			   //uart_printf(1,0,PSTR("GeniusCode: "));
			   //uart_print(1,1,strGeniusCode);
		       _datetime(0, strSystemDate, strSystemTime);
               stMenuMaster=mmIsValidGeniusCode;
		       break;
          case USER_CANCEL:
		       stMenuMaster=mmExitMaster;
		       break;
		  case USER_ENTRY:
		       lcd_printf(4,1,PSTR("[*]Back     [#]Enter"));
		       break;
          case USER_NO_DATA:
		       lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
		       break;
		  }
	      break;
     case mmIsValidGeniusCode:
	      //GetDate
		      sprintf_P(PDate,PSTR("%s"),strSystemDate);
			  RemoveChar(PDate,'/');
		  if (ValidateGeniusCode(PDate,strGeniusCode)==GC_VALID){

		  	  sprintf_P(PTime,PSTR("%s"),strSystemTime);
			  RemoveChar(PTime,':');sprintf_P(strKeyStamp,PSTR(""));

              //sprintf_P(strSend,PSTR("Time:%s"),PTime);
			  // uart_print(1,1,strSend);

		      GenerateKeyStamp(PTime,strGeniusCode,strKeyStamp);			  
			  stMenuMaster=mmDisplayKeyStamp;
		  }else stMenuMaster=mmDisplayInvalidCode;
	      break;
     case mmDisplayInvalidCode:
	      system_beep(1);
          lcd_printf(3,1,PSTR("Invalid Code        "));
          TimDisplay=0; 
		  stMenuMaster=mmDelayDisplayInvalidCode;
	      break;

     case mmDelayDisplayInvalidCode:
	      if (TimDisplay>5)stMenuMaster=mmRestorePasswordMenu;
	      break;
     case mmDisplayKeyStamp:
	      lcd_clear();
		  sprintf_P(lcdteks,PSTR("Key Stamp:%s "),strKeyStamp);
		  lcd_print(1,1,lcdteks);
		  lcd_printf(2,1,PSTR("Enter Restore Code  "));
		  lcd_printf(3,1,PSTR("_                   "));
		  lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
          stMenuMaster=mmRestoreCodeEntry;
	      break;
     case mmRestoreCodeEntry:
	 	  uiResult=UserInput(UI_NUMBER_R,3,1,strRestoreCode,0,10);
		  switch(uiResult){
		  case USER_OK:
               stMenuMaster=mmIsValidRestoreCode;
		       break;
          case USER_CANCEL:
		       stMenuMaster=mmExitMaster;
		       break;
		  case USER_ENTRY:
		       lcd_printf(4,1,PSTR("[*]Back     [#]Enter"));
		       break;
          case USER_NO_DATA:
		       lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
		       break;
		  }
	      break;
     case mmIsValidRestoreCode:
	      if (ValidateRestoreCode(strKeyStamp,strRestoreCode)==RC_VALID){
		      stMenuMaster=mmRestorePassword;
		  }else stMenuMaster=mmDisplayInvalidRestoreCode;
	      break;
     case mmRestorePassword:
	      sprintf_P(strNewPassword,PSTR(SPV_DEFAULT_PASS));
		  eeprom_write_block((const void*) &strNewPassword,(void*) &DefSpvPassword,sizeof(DefSpvPassword));
		  sprintf_P(strNewPassword,PSTR(SYS_DEFAULT_PASS));
		  eeprom_write_block((const void*) &strNewPassword,(void*) &DefSysPassword,sizeof(DefSysPassword));	      		  
	      stMenuMaster=mmDisplaySuccess;
	      break;
     case mmDisplayInvalidRestoreCode:
	      lcd_printf(3,1,PSTR("Invalid Restore  "));
		  TimDisplay=0;
          stMenuMaster=mmDelayInvalidRestoreCode;
	      break;
     case mmDelayInvalidRestoreCode:
	      if (TimDisplay>5)stMenuMaster=mmDisplayKeyStamp;
	      break;
     case mmDisplaySuccess:
	      //lcd_clear();
		  lcd_printf(3,1,PSTR("Password Restored"));
		  system_beep(1);
		  TimDisplay=0;
          stMenuMaster=mmDelayExit;
	      break;
     case mmDelayExit:
	      if (TimDisplay>=5)stMenuMaster=mmExitMaster;
	      break;
     case mmExitMaster:
	      stMenuMaster=mmInitMaster;
	      Result=MENU_DONE;
	      break;
	 }
  return Result;

}

char FMenuAdmin(){
static char stMenuAdmin=maInitAdmin;
	 char SubMenu,KeyPressed,KeyChar,Result=MENU_NONE;
     
	 switch(stMenuAdmin){
	 case maInitAdmin:
		  lcd_clear();
		  lcd_printf(1,1,PSTR("1)RePrint           "));
		  lcd_printf(2,1,PSTR("2)CloseShift        "));
		  lcd_printf(3,1,PSTR("3)CloseDay          "));		  
		  lcd_printf(4,1,PSTR("4)Settings   *)Exit "));
	      stMenuAdmin=maSelectOptions;
	      break;
	 case maSelectOptions:
          KeyPressed=_key_scan(1);
		  KeyChar=_key_btn(KeyPressed);
		  switch (KeyChar){
		  case '1':
		       stMenuAdmin=maMenuReprint;//maMenuAdminConfig;
		       break;  
		  case '2':
		       if (IFType==IT_STANDALONE)
			       stMenuAdmin=maMenuCloseShift;
               else stMenuAdmin=maInitAdmin;
		       break;  
		  case '3':
		       if (IFType==IT_STANDALONE)stMenuAdmin=maMenuCloseDay;
               else stMenuAdmin=maInitAdmin;
		       break;  
		  case '4':
		  	   stMenuAdmin=maMenuAdminSettings;
		       break;  
		  case '*':
    		   stMenuAdmin=maExitMenuAdmin;
		       break;  		  
		  }		  
	      break;
     case maMenuReprint:
	      SubMenu=FMenuReprint();
		  if (SubMenu==MENU_DONE)stMenuAdmin=maExitMenuAdmin;
	      break;
	 case maMenuAdminConfig://ChangePassword[],RePrint,PumpTest
	      SubMenu=FSubMenuAdmin();
		  if (SubMenu==MENU_DONE)stMenuAdmin=maInitAdmin;
          break;	 
	 case maMenuCloseShift:
	 	  SubMenu=FCloseShift(CONTINUE_SHIFT);
		  if (SubMenu==MENU_DONE)stMenuAdmin=maExitMenuAdmin;
	      break;
     case maMenuCloseDay://maMenuCloseShift,maMenuCloseDay
	 	  SubMenu=FCloseShift(NEW_SHIFT);
		  if (SubMenu==MENU_DONE)stMenuAdmin=maExitMenuAdmin;
	      break;
     case maMenuAdminSettings:
	      SubMenu=FMenuAdminSettings();
		  if (SubMenu==MENU_DONE)stMenuAdmin=maInitAdmin;	      
	      break;
     case maExitMenuAdmin:
	      stMenuAdmin=maInitAdmin;	      
	      Result=MENU_DONE;
          break;	 
	 }
   return Result;
}

char FSubMenuAdmin(){
     char Result;
	 Result=MENU_DONE;
   return Result;
}



char FMenuShift(){//Close Shift,Close Day
static char stMenuShift=msInitMenuShift;
     char SubMenu,KeyChar,KeyPressed;
     char Result;

Result=MENU_NONE;
     switch(stMenuShift){
	 case msInitMenuShift:
	      lcd_clear();
		  lcd_printf(1,1,PSTR("1.Close Shift"));
		  lcd_printf(2,1,PSTR("2.Close Day  "));
		  lcd_printf(3,1,PSTR("3.Lock Pump  "));
		  lcd_printf(4,1,PSTR("*)Exit"));
		  stMenuShift=msSelectShift;
	      break;
     case msSelectShift:
          KeyPressed=_key_scan(1);
		  KeyChar=_key_btn(KeyPressed);
          switch(KeyChar){
		  case '1':
		       SubMenu=MENU_NONE;
		       stMenuShift=msCloseShift;
		       break; 
		  case '2':
		       SubMenu=MENU_NONE;
		       stMenuShift=msCloseDay;
		       break; 
		  case '3':
		       SubMenu=MENU_NONE;
		       stMenuShift=msLockPump;
		       break; 
		  case '*':
		       stMenuShift=msExitShift;
		       break; 			   		  
		  }
		  break;
     case msCloseShift:
	      SubMenu=FCloseShift(CONTINUE_SHIFT);
		  if (SubMenu==MENU_DONE)stMenuShift=msInitMenuShift;
		  //IsGenerateReport=True;		  
	      break;
     case msCloseDay:
	      SubMenu=FCloseShift(NEW_SHIFT);
		  if (SubMenu==MENU_DONE)stMenuShift=msInitMenuShift;
	      break;
     case msLockPump:
	      SubMenu=FLockPump();
		  if (SubMenu==MENU_DONE)stMenuShift=msInitMenuShift;
	      break;
     case msExitShift:
          stMenuShift=msInitMenuShift;
	      Result=MENU_DONE;
	      break;	 
	 } 
   return Result;
}

char CountNoPumpSatus(char *strPumpStatusTotalizer){
char i,Result;
     Result=0;
	 for (i=0;i<PumpCountMax;i++){
	      if (strPumpStatusTotalizer[i]==GetPumpStatusLabel(PUMP_NONE)){
		      Result++;		  
		  }	 
	 }
  return Result;
}
char CountTotalizerSatus(char *strPumpStatusTotalizer){
char i,Result;
     Result=0;
	 for (i=0;i<strlen(strPumpStatusTotalizer);i++){
	      if (strPumpStatusTotalizer[i]==GetPumpStatusLabel(PS_TOTALIZER)){
		      Result++;		  
		  }	 
	 }
  return Result;
}


char FCloseShift(char ShiftType){//SHIFT_NONE,NEW_SHIFT,CONTINUE_SHIFT
static char stCloseShift=csInitCloseShift;
static char IsPumpBusy=False;

     char strSend[30],lcdteks[20];
     char Result=MENU_NONE;
	 char KeyPressed;
	 char FIPAddr;


	 Result=MENU_NONE;
	 switch(stCloseShift){
	 case csInitCloseShift:
	      ActivePump=eeprom_read_byte(&DefActivePump);
		  IsPumpBusy=False;
		  //FindBusy Pump
		  for (FIPAddr=0;FIPAddr<ActivePump;FIPAddr++){		       
		       if ((strPumpStatus[FIPAddr]!=GetPumpStatusLabel(PUMP_NONE))&&(strPumpStatus[FIPAddr]!=GetPumpStatusLabel(PUMP_OFF))&&(strPumpStatus[FIPAddr]!=GetPumpStatusLabel(PUMP_BUSY)))
				    UpdateStandaloneStatus(GetPumpID(FIPAddr),PUMP_OFF);
					
		       if (strPumpStatus[FIPAddr]==GetPumpStatusLabel(PS_TOTALIZER))
			        UpdateStandaloneStatus(GetPumpID(FIPAddr),PUMP_OFF);
					
               if (strPumpStatus[FIPAddr]==GetPumpStatusLabel(PUMP_BUSY))
			       IsPumpBusy=True;
			   }

          stCloseShift=csSendTotalizerALL;
	      break;
	 case csInitLockingPump:
	      /*
	      lcd_clear();
		  lcd_printf(1,1,PSTR("Locking Pump"));
          xPumpID=1;
		  DisplayPumpStatus();          
          stCloseShift=csLockPump;
		  */
	      break;
     case csLockPump:
	      /*
	      if ((strPumpStatus[xPumpID-1]!=GetPumpStatusLabel(PUMP_BUSY))&&
		      (strPumpStatus[xPumpID-1]!=GetPumpStatusLabel(PUMP_NONE))){
	           SendSlaveCommand(SC_PUMP_LOCK,xPumpID);
			  }
			  */
	      break;
     case csWaitPumpLocked:	     
	      break;
     case csSendTotalizerALL:
	      lcd_clear();
		  lcd_printf(1,1,PSTR("Totalizer.."));
		  
		  sprintf_P(lcdteks,PSTR("Wait Pump:%d "),(ActivePump-CountTotalizerSatus(strPumpStatus)));
		  lcd_print(2,1,lcdteks);


          IsNewPumpStatus=True;
		  DisplayPumpStatus(); 
		  //SendCommand
	      //SendSlaveCommand(SC_TOTALIZER,PUMP_ALL);          
		  SendPoolingCommand(SC_TOTALIZER,PUMP_ALL);
		  stCloseShift=csDisplayPumpStatus;
	      break;
     case csDisplayPumpStatus:
		  DisplayPumpStatus(); 
		  TimDisplay=0;         
          stCloseShift=csWaitTotalizerComplete;
	      break;
     case csWaitTotalizerComplete:
	      KeyPressed=_key_scan(1);
		  if (IsNewPumpStatus==True){
			  sprintf_P(lcdteks,PSTR("Wait Pump:%d "),(ActivePump-CountTotalizerSatus(strPumpStatus)));
			  lcd_print(2,1,lcdteks);
		  	  stCloseShift=csDisplayPumpStatus;	 
		  }
		  if ((CountTotalizerSatus(strPumpStatus)>=ActivePump)&&(TimDisplay>5)){
		      for (FIPAddr=0;FIPAddr<ActivePump;FIPAddr++)
			       UpdateStandaloneStatus(GetPumpID(FIPAddr),PUMP_OFF);

		      stCloseShift=csGenerateReport;		      
			  }
          //sprintf_P(lcdteks,PSTR("TimSend:%d"),TimDisplay);
		  //lcd_print(3,1,lcdteks);

          if ((TimDisplay>15)&&(IsPumpBusy!=True)){
		      if(CountNoPumpSatus(strPumpStatus)>=eeprom_read_byte(&DefActivePump)){
			  //lcd_printf(2,1,PSTR("NoPumpFound  ")); 
			  //sprintf_P(lcdteks,PSTR("NoPumpFound:%d"),CountNoPumpSatus(strPumpStatus));
			  sprintf_P(lcdteks,PSTR("Error - No Pump:%d "),ActivePump-CountNoPumpSatus(strPumpStatus));
			  lcd_print(3,1,lcdteks);
			  
			  for (FIPAddr=0;FIPAddr<ActivePump;FIPAddr++){
		           if (strPumpStatus[FIPAddr]==GetPumpStatusLabel(PS_TOTALIZER))
			           UpdateStandaloneStatus(GetPumpID(FIPAddr),PUMP_OFF);
			  }

			  system_beep(1);
			  TimDisplay=0;
			  stCloseShift=csNoPumpFound;		     
			  }			  
		  }
		  if (KeyPressed==_KEY_CANCEL){
			  sprintf_P(lcdteks,PSTR("Cancel"));
			  lcd_print(3,1,lcdteks);
			  system_beep(1);
			  TimDisplay=0;
			  stCloseShift=csNoPumpFound;		     		  
		  }
	      break;
     case csNoPumpFound:
	      if (TimDisplay>TIM_DISPLAY){
		      stCloseShift=csFinishCloseShift;
		  }	      
	      break;
     case csGenerateReport:
		  lcd_printf(1,1,PSTR("Printing Totalizer"));
          _datetime(0, strSystemDate, strSystemTime);
		  sprintf_P(CurrentShiftDateTime,PSTR("%s %s"),strSystemDate,strSystemTime);
		  IsGenerateReport=True;
		  IsFinishPrintingTotalizer=False;
		  stCloseShift=csWaitPrintTotalizerComplete;
	      break;
     case csWaitTotalizerALL:
	      break;
     case csWaitPrintTotalizerComplete:
	      if (IsFinishPrintingTotalizer==True){
		      IsFinishPrintingTotalizer=False;
		      stCloseShift=csDumpShift;
		  }
	      break;
     case csDumpShift://Increment CurrentShift save Current to Last
		  lcd_printf(1,1,PSTR("Saving ShiftData.. "));
	      if (ShiftType==CONTINUE_SHIFT)//Shift=Shift+1
		      eeprom_write_byte(&DefShift,eeprom_read_byte(&DefShift)+1);	 
          else if (ShiftType==NEW_SHIFT)//Shift=1
		      eeprom_write_byte(&DefShift,1);	 

          SaveTotalizerCurrentToLast();
          eeprom_write_block((const void*) &CurrentShiftDateTime, (void*) &DefLastShiftDateTime,sizeof(DefLastShiftDateTime));
		  stCloseShift=csFinishCloseShift;
	      break;
     case csFinishCloseShift:
          stCloseShift=csInitCloseShift;
		  Result=MENU_DONE;
	      break;
	 }
	 //uart_printf(0,1,PSTR("Close Shift"));	
   return Result;
}

char FCloseDay(){

}
char FLockPump(){

}

char FMenuAdminSettings(){
     static char stAdminSettings=asInitMenu;
	 char SubMenu,Result,KeyChar;
	 Result=MENU_NONE;
	 switch(stAdminSettings){
	 case asInitMenu:
          lcd_clear();
		  if (IFType==IT_SLAVE){
		      lcd_printf(1,1, PSTR("1)Header    4)Client"));
		      lcd_printf(2,1, PSTR("2)Footer    5)Server"));
		  }else
		  if (IFType==IT_STANDALONE){
		      lcd_printf(1,1, PSTR("1)Header            "));
		      lcd_printf(2,1, PSTR("2)Footer            "));
		  }
		  lcd_printf(3,1, PSTR("3)Password          "));
		  lcd_printf(4,1, PSTR("*)Exit"));
		  stAdminSettings=asAdminSettingsOption;
		  break;
     case asAdminSettingsOption:
	      KeyChar=_key_btn(_key_scan(1));
		  switch(KeyChar){
		  case '1':stAdminSettings=asAdminSettingHeader;
		       break;
		  case '2':stAdminSettings=asAdminSettingFooter;
		       break;
		  case '3':stAdminSettings=asAdminSettingPassword;
		       break;
		  case '*':stAdminSettings=asExitAdminSetting;
		       break;			   
		  }
		  if (IFType==IT_SLAVE){
			  switch(KeyChar){
			  case '4':stAdminSettings=asAdminSettingClientIP;
				   break;
			  case '5':stAdminSettings=asAdminSettingServerIP;
				   break;
			  }
		  }
	      break;
	 case asAdminSettingHeader:
	      SubMenu=FMenuSettingHeader();
	      if (SubMenu==MENU_DONE)stAdminSettings=asInitMenu;
	      break;
	 case asAdminSettingFooter:
	      SubMenu=FMenuSettingFooter();
	      if (SubMenu==MENU_DONE)stAdminSettings=asInitMenu;
	      break;
	 case asAdminSettingPassword:
	      SubMenu=FMenuSettingPassword();
	      if (SubMenu==MENU_DONE)stAdminSettings=asInitMenu;	 
	      break;
     case asAdminSettingClientIP:
	      SubMenu=FMenuSettingClientIP();
	      if (SubMenu==MENU_DONE)stAdminSettings=asInitMenu;	 
	      break;
	 case asAdminSettingServerIP:
	      SubMenu=FMenuSettingServerIP();
	      if (SubMenu==MENU_DONE)stAdminSettings=asInitMenu;	 
	      break;
     case asExitAdminSetting:
	      stAdminSettings=asInitMenu;
	      Result=MENU_DONE;
	      break;
	 }
   return Result;
}

char FMenuSettingHeader(){
static char stSettingHeader=shInitHeader,HeaderIdx=0;
     char Result,uiResult,KeyChar;
	 char lcdteks[20];
	 char strHeaderFooter[50];
	 
	 Result=MENU_NONE;
     
	 switch (stSettingHeader){
	 case shInitHeader:
	      lcd_clear();
		  lcd_printf(1,1,PSTR("1)Header1  4)Header4"));
		  lcd_printf(2,1,PSTR("2)Header2  5)Header5"));
		  lcd_printf(3,1,PSTR("3)Header3  6)Header6"));
		  lcd_printf(4,1,PSTR("*)Exit              "));
		  stSettingHeader=shHeaderSelect;
	      break;
     case shHeaderSelect:
	      KeyChar=_key_btn(_key_scan(1));
		  if ((KeyChar>='1')&&(KeyChar<='6')){
		       HeaderIdx=KeyChar-'1';//
			   lcd_clear();
			   sprintf_P(lcdteks,PSTR("Edit Header%d "),HeaderIdx+1);
			   lcd_print(1,1,lcdteks);
			   lcd_printf(4,1,PSTR("[*]Back  [#]OK"));
			   FillChar(PrintBuffer,sizeof(PrintBuffer),0);
			   stSettingHeader=shEditHeader1;
		  }
		  if(KeyChar=='*')stSettingHeader=shExitSettingHeader;
	      break;
     case shEditHeader1:
          //uiResult=UserInput(UI_ALPHANUM_R,2,1,strFreeMessageLine1,0,20);
		  uiResult=UserInput(UI_ALPHANUM_R,2,1,PrintBuffer,0,40);
		  if (uiResult==USER_OK){
			  lcd_clear();
			  AddSpaceLag(PrintBuffer,40);
			  StrPosCopy(PrintBuffer,strFreeMessageLine1,0,20);
			  StrPosCopy(PrintBuffer,strFreeMessageLine2,20,20);

			  lcd_print(1,1,strFreeMessageLine1);			  
			  lcd_print(2,1,strFreeMessageLine2);			  
			  
			  lcd_printf(3,1,PSTR("Align Center?"));
			  lcd_printf(4,1,PSTR("[*]No  [#]Yes"));
		      stSettingHeader=shIsAlignCenter;
		  }
		  else
		  if (uiResult==USER_CANCEL)stSettingHeader=shInitHeader;
	      break;
     case shIsAlignCenter:
	      KeyChar=_key_btn(_key_scan(1));
          if(KeyChar=='#'){
		      //AlignCenter			  
			  StrAlignCenter(PrintBuffer,40);
			  stSettingHeader=shIsSaveHeader;
		  } else if(KeyChar=='*')stSettingHeader=shIsSaveHeader;		 
	      break;
     case shIsSaveHeader:
	      lcd_clear(); 
		  StrPosCopy(PrintBuffer,strFreeMessageLine1,0,20);
		  StrPosCopy(PrintBuffer,strFreeMessageLine2,20,20);
          lcd_print(1,1,strFreeMessageLine1);
		  lcd_print(2,1,strFreeMessageLine2);
		  sprintf_P(lcdteks,PSTR("Save Header%d ?"),HeaderIdx+1);
		  lcd_print(3,1,lcdteks);
		  lcd_printf(4,1,PSTR("[*]No [#]Yes"));		      
	      stSettingHeader=shSaveHeaderQuestions;      
	      break;    
     case shSaveHeaderQuestions:
	      KeyChar=_key_btn(_key_scan(1));
	      if(KeyChar=='#')stSettingHeader=shSaveHeader;
		  else
          if(KeyChar=='*')stSettingHeader=shInitHeader;
	      break;
     case shSaveHeader:
	      sprintf_P(strHeaderFooter,PSTR("%s%s"),strFreeMessageLine1,strFreeMessageLine2);
	      eeprom_write_block((const void*) &strHeaderFooter, (void*) &DefHeaderFooter[HeaderIdx],40);
		  //eeprom_write_block((const void*) &PrintBuffer, (void*) &DefHeaderFooter[HeaderIdx],40);
          stSettingHeader=shInitHeader;
	      break;
     case shExitSettingHeader:
          stSettingHeader=shInitHeader;
	      Result=MENU_DONE;
	      break;	 
	 }
   //_menu_header();   
   return Result;
}

char FMenuSettingFooter(){
static char stSettingHeader=shInitHeader,HeaderIdx=0;
     char Result,uiResult;
	 char lcdteks[20];
	 char strHeaderFooter[50];
	 char KeyChar;
	 
	 Result=MENU_NONE;
     
	 switch (stSettingHeader){
	 case shInitHeader:
	      lcd_clear();
		  lcd_printf(1,1,PSTR("1)Footer1  3)Footer3"));
		  lcd_printf(2,1,PSTR("2)Footer2  4)Footer4"));
		  lcd_printf(3,1,PSTR("                    "));
		  lcd_printf(4,1,PSTR("*)Exit              "));
		  stSettingHeader=shHeaderSelect;
	      break;
      case shHeaderSelect:
	      KeyChar=_key_btn(_key_scan(1));
		  if ((KeyChar>='1')&&(KeyChar<='6')){
		       HeaderIdx=KeyChar-'1';//
			   lcd_clear();
			   sprintf_P(lcdteks,PSTR("Edit Footer%d "),HeaderIdx+1);
			   lcd_print(1,1,lcdteks);
			   lcd_printf(4,1,PSTR("[*]Back  [#]OK"));
			   FillChar(PrintBuffer,sizeof(PrintBuffer),0);
			   stSettingHeader=shEditHeader1;
		  }
		  if(KeyChar=='*')stSettingHeader=shExitSettingHeader;
	      break;
     case shEditHeader1:
          //uiResult=UserInput(UI_ALPHANUM_R,2,1,strFreeMessageLine1,0,20);
		  uiResult=UserInput(UI_ALPHANUM_R,2,1,PrintBuffer,0,40);
		  if (uiResult==USER_OK){
			  lcd_clear();
			  AddSpaceLag(PrintBuffer,40);
			  StrPosCopy(PrintBuffer,strFreeMessageLine1,0,20);
			  StrPosCopy(PrintBuffer,strFreeMessageLine2,20,20);

			  lcd_print(1,1,strFreeMessageLine1);			  
			  lcd_print(2,1,strFreeMessageLine2);			  
			  
			  lcd_printf(3,1,PSTR("Align Center?"));
			  lcd_printf(4,1,PSTR("[*]No  [#]Yes"));
		      stSettingHeader=shIsAlignCenter;
		  }
		  else
		  if (uiResult==USER_CANCEL)stSettingHeader=shInitHeader;
	      break;
     case shIsAlignCenter:
	      KeyChar=_key_btn(_key_scan(1));
          if(KeyChar=='#'){
		      //AlignCenter			  
			  StrAlignCenter(PrintBuffer,40);
			  stSettingHeader=shIsSaveHeader;
		  } else if(KeyChar=='*')stSettingHeader=shIsSaveHeader;		 
	      break;
     case shIsSaveHeader:
	      lcd_clear(); 
		  StrPosCopy(PrintBuffer,strFreeMessageLine1,0,20);
		  StrPosCopy(PrintBuffer,strFreeMessageLine2,20,20);
          lcd_print(1,1,strFreeMessageLine1);
		  lcd_print(2,1,strFreeMessageLine2);
		  sprintf_P(lcdteks,PSTR("Save Footer%d ?"),HeaderIdx+1);
		  lcd_print(3,1,lcdteks);
		  lcd_printf(4,1,PSTR("[*]No [#]Yes"));		      
	      stSettingHeader=shSaveHeaderQuestions;      
	      break;       
     case shSaveHeaderQuestions:
          KeyChar=_key_btn(_key_scan(1));
	      if(KeyChar=='#')stSettingHeader=shSaveHeader;
		  else
          if(KeyChar=='*')stSettingHeader=shInitHeader;
	      break;
     case shSaveHeader:
	      sprintf_P(strHeaderFooter,PSTR("%s%s"),strFreeMessageLine1,strFreeMessageLine2);
	      eeprom_write_block((const void*) &strHeaderFooter, (void*) &DefHeaderFooter[HeaderIdx+6],40);
		  //eeprom_write_block((const void*) &PrintBuffer, (void*) &DefHeaderFooter[HeaderIdx+6],40);

          stSettingHeader=shInitHeader;
	      break;
     case shExitSettingHeader:
          stSettingHeader=shInitHeader;
	      Result=MENU_DONE;
	      break;	 
	 }
   return Result;
}
char FMenuSettingPassword(){
     char Result;
   //_menu_password();
   Result=MENU_DONE;
   return Result;
}





char FMenuSettingClientIP(){
static char stClientIP=cipInit,IPchar[3];
static char i,Result,x,y,iInput,IP_blok[4];
static unsigned int iLoop,iBlok;
     char keyChar,keyPressed=0;
	 char lcdteks[20];	 

	 //1---------1---------
     //Client IP
     //Old: 192.168.123.000
	 //New:   _.   .   .
	 //--------------------
	 //*)cancel    #)next
     Result=MENU_NONE;
	 switch(stClientIP){
		  case cipInit:
			   lcd_clear();
	           lcd_printf(1,1,PSTR("Client IP"));
			   eeprom_read_block((void*)&IP_blok,(const void*)&DefClientIP,4);
	           sprintf_P(lcdteks,PSTR("Old: %d.%d.%d.%d"),IP_blok[0],IP_blok[1],IP_blok[2],IP_blok[3]);
	           lcd_print(2,1,lcdteks);
			   lcd_printf(3,1,PSTR("New:   _.   .   .   "));
			   lcd_printf(4,1,PSTR("[*]cancel  [#]next  "));
               iLoop=0;
			   y=3,x=8;
			   iInput=0; iBlok=0;
			   IPchar[0]='0';IPchar[1]='0';IPchar[2]='0';
			   stClientIP=cipInputIP; 
		       break;
          case cipInputIP:
			   //Blinking 60% _ 
			   iLoop++;
			   if ((iLoop%2000)==0){
			      lcd_put(y,(x+(iBlok*4)),'_'); 
			   }
			   if ((iLoop%2000)==1000){
			      lcd_put(y,(x+(iBlok*4)),' '); 
			   }
			   //GetKeyPressed
			   keyPressed=_key_scan(1);
			   keyChar=_key_btn(keyPressed);
               if ((keyChar>='0')&&(keyChar<='9')){
			       _delay_ms(200);
			       IPchar[iInput]=keyChar;
				   iInput++;                    
				   for (i=1;i<(iInput+1);i++){
				       lcd_xy(y,((x+(iBlok*4))-(3-i)));_lcd(IPchar[i-1]);
				   }
                   //NextInput
				   if (iInput>=3){
				       IP_blok[iBlok]=((IPchar[0]-'0')*100)+((IPchar[1]-'0')*10)+((IPchar[2]-'0'));
					   IPchar[0]='0';IPchar[1]='0';IPchar[2]='0';
					   //Reposition Value
					   lcd_printf(y,((x+(iBlok*4))-iInput),PSTR("   "));
					   sprintf_P(lcdteks,PSTR("%d"),IP_blok[iBlok]);
                       lcd_print(y,1+((x+(iBlok*4))-strlen(lcdteks)),lcdteks);
					   if (iBlok>0)lcd_printf(y,1+((x+((iBlok-1)*4))),PSTR("."));
					   iInput=0;
					   iBlok++;

				   }
				   if (iBlok>=4)stClientIP=cipStoreIPblok;
			   }

			   if (keyPressed==_KEY_CANCEL){
			       stClientIP=cipExit; 
			   }
			   else
			   if (keyPressed==_KEY_ENTER){
			       //NextInput
			       if (iBlok<4){
				       if (iInput==1)IP_blok[iBlok]=((IPchar[0]-'0'));
					   if (iInput==2)IP_blok[iBlok]=((IPchar[0]-'0')*10)+(IPchar[1]-'0');
					   if (iInput==3)IP_blok[iBlok]=((IPchar[0]-'0')*100)+((IPchar[1]-'0')*10)+((IPchar[2]-'0'));
				       
					   IPchar[0]='0';IPchar[1]='0';IPchar[2]='0';
					   //Reposition Value
					   lcd_printf(y,((x+(iBlok*4))-3),PSTR("   "));
					   sprintf_P(lcdteks,PSTR("%d"),IP_blok[iBlok]);
                       lcd_print(y,1+((x+(iBlok*4))-strlen(lcdteks)),lcdteks);

					   if (iBlok>0)lcd_printf(y,1+((x+((iBlok-1)*4))),PSTR("."));
				       iInput=0;
					   iBlok++;	   
				   }
				   if (iBlok>=4)stClientIP=cipStoreIPblok;
			   }
		       break;
          case cipStoreIPblok://UpdateIPblok
			   eeprom_write_block((const void*) &IP_blok,(const void*) &DefClientIP,4);
			   stClientIP=cipExit;
		       break; 
          case cipExit://Cancel IPConfig
		       stClientIP=cipInit;
		       Result=MENU_DONE;
		       break;     
		  }//EndSwitch	 
    return Result;
}

char FMenuSettingServerIP(){
static char stClientIP=cipInit,IPchar[3];
static char i,Result,x,y,iInput,IP_blok[4];
static unsigned int iLoop,iBlok;
     char keyPressed=0,keyChar;
     char lcdteks[20];

	 //1---------1---------
     //Server IP
     //Old: 192.168.123.000
	 //New:   _.   .   .
	 //--------------------
	 //*)cancel    #)next
	 Result=MENU_NONE;
     switch(stClientIP){
		  case cipInit:
			   lcd_clear();
	           lcd_printf(1,1,PSTR("Server IP"));
			   eeprom_read_block((void*)&IP_blok,(const void*)&DefServerIP,4);
	           sprintf_P(lcdteks,PSTR("Old: %d.%d.%d.%d"),IP_blok[0],IP_blok[1],IP_blok[2],IP_blok[3]);
	           lcd_print(2,1,lcdteks);
			   lcd_printf(3,1,PSTR("New:   _.   .   .   "));
			   lcd_printf(4,1,PSTR("[*]cancel  [#]next  "));
               iLoop=0;
			   y=3,x=8;
			   iInput=0; iBlok=0;
			   IPchar[0]='0';IPchar[1]='0';IPchar[2]='0';
			   stClientIP=cipInputIP; 
		       break;
          case cipInputIP:
			   //Blinking 60% _ 
			   iLoop++;
			   if ((iLoop%2000)==0){
			      lcd_put(y,(x+(iBlok*4)),'_'); 
			   }
			   if ((iLoop%2000)==1000){
			      lcd_put(y,(x+(iBlok*4)),' '); 
			   }
			   //GetKeyPressed
			   keyPressed=_key_scan(1);
			   keyChar=_key_btn(keyPressed);
               if ((keyChar>='0')&&(keyChar<='9')){
			       _delay_ms(200);
			       IPchar[iInput]=keyChar;
				   iInput++;                    
				   for (i=1;i<(iInput+1);i++){
				       lcd_xy(y,((x+(iBlok*4))-(3-i)));_lcd(IPchar[i-1]);
				   }
                   //NextInput
				   if (iInput>=3){
				       IP_blok[iBlok]=((IPchar[0]-'0')*100)+((IPchar[1]-'0')*10)+((IPchar[2]-'0'));
					   IPchar[0]='0';IPchar[1]='0';IPchar[2]='0';
					   //Reposition Value
					   lcd_printf(y,((x+(iBlok*4))-iInput),PSTR("   "));
					   sprintf_P(lcdteks,PSTR("%d"),IP_blok[iBlok]);
                       lcd_print(y,1+((x+(iBlok*4))-strlen(lcdteks)),lcdteks);
					   if (iBlok>0)lcd_printf(y,1+((x+((iBlok-1)*4))),PSTR("."));
					   iInput=0;
					   iBlok++;

				   }
				   if (iBlok>=4)stClientIP=cipStoreIPblok;
			   }

			   if (keyPressed==_KEY_CANCEL){
			       stClientIP=cipExit; 
			   }
			   else
			   if (keyPressed==_KEY_ENTER){
			       //NextInput
			       if (iBlok<4){
				       if (iInput==1)IP_blok[iBlok]=((IPchar[0]-'0'));
					   if (iInput==2)IP_blok[iBlok]=((IPchar[0]-'0')*10)+(IPchar[1]-'0');
					   if (iInput==3)IP_blok[iBlok]=((IPchar[0]-'0')*100)+((IPchar[1]-'0')*10)+((IPchar[2]-'0'));
				       
					   IPchar[0]='0';IPchar[1]='0';IPchar[2]='0';
					   //Reposition Value
					   lcd_printf(y,((x+(iBlok*4))-3),PSTR("   "));
					   sprintf_P(lcdteks,PSTR("%d"),IP_blok[iBlok]);
                       lcd_print(y,1+((x+(iBlok*4))-strlen(lcdteks)),lcdteks);

					   if (iBlok>0)lcd_printf(y,1+((x+((iBlok-1)*4))),PSTR("."));
				       iInput=0;
					   iBlok++;	   
				   }
				   if (iBlok>=4)stClientIP=cipStoreIPblok;
			   }
		       break;
          case cipStoreIPblok:
		       //UpdateIPblok
			   eeprom_write_block((const void*) &IP_blok,(const void*) &DefServerIP,4);
			   stClientIP=cipExit;
		       break; 
          case cipExit://Cancel IPConfig
			   stClientIP=cipInit;
			   Result=MENU_DONE;
		       break;     
		  }//EndSwitch	 
	return Result;
}



char FMenuSettings(){
char Result=MENU_NONE;
static char stMenuSettings=msInit,PageSetting=1;
     char KeyPressed,KeyChar,SubMenu;

     switch(stMenuSettings){
	 case msInit:
		  lcd_clear();
		  PageSetting=1;
		  stMenuSettings=msDisplayPage;
		  break;
	 case msDisplayPage:
	      stMenuSettings=msDisplayPage+PageSetting;
          break; 	 
	 case msDisplayPage1:
		  lcd_printf(1, 1, PSTR("1)Product  5)Printer"));
		  lcd_printf(2, 1, PSTR("2)Pump     6)Host   "));
		  lcd_printf(3, 1, PSTR("3)Decimal  7)Next   "));
		  lcd_printf(4, 1, PSTR("4)Datetime *)Exit   "));
		  stMenuSettings=msSelection;
	      break;
	 case msDisplayPage2:
		  lcd_printf(1, 1, PSTR("1)Operator          "));
		  lcd_printf(2, 1, PSTR("2)System            "));
		  lcd_printf(3, 1, PSTR("3)PumpPooling       "));
		  lcd_printf(4, 1, PSTR("*)Back              "));
		  stMenuSettings=msSelection;
	      break;
	 case msSelection:
	      stMenuSettings=msSelection+PageSetting;
          break;	 
	 case msSelectionPage1:
		  KeyPressed=_key_scan(1);
		  KeyChar=_key_btn(KeyPressed);
		  switch(KeyChar){
		  case '1':
		       stMenuSettings=msMenuSettingProduct;
		       break;
		  case '2': 
		       stMenuSettings=msMenuSettingPump;
		       break;
		  case '3':
		       stMenuSettings=msMenuSettingDec;
		       break;
		  case '4':
		       stMenuSettings=msMenuSettingDatetime;
		       break;
		  case '5':
		       stMenuSettings=msMenuSettingPrinter;
		       break;
		  case '6':
		       stMenuSettings=msMenuSettingHost;
		       break;
		  case '7':
		       stMenuSettings=msMenuSettingNextPage;
		       break;
		  case '*':
		       stMenuSettings=msMenuSettingExit;
		       break;			   
		  }
	      break;
	 case msSelectionPage2:
	 	  KeyPressed=_key_scan(1);
		  KeyChar=_key_btn(KeyPressed);
		  switch(KeyChar){
		  case '1':
		       stMenuSettings=msMenuSettingOperator;
		       break;
		  case '2':
		       stMenuSettings=msMenuSettingSystem;
		       break;
		  case '3':
		       stMenuSettings=msMenuSettingPumpPooling;
		       break;
		  case '*':
		       stMenuSettings=msMenuSettingBackPage;
		       break;		  
		  }
	      break;
		  
	 case msMenuSettingNextPage:
	      if (PageSetting<2)PageSetting++;
		  stMenuSettings=msDisplayPage;
	      break;
	 case msMenuSettingBackPage:
	      if (PageSetting>1)PageSetting--;
		  stMenuSettings=msDisplayPage;
	      break;
//---Sub Menu Operations--------------------------------------------		  
     case msMenuSettingProduct:
	      SubMenu=FSettingProduct();
		  if (SubMenu==MENU_DONE)stMenuSettings=msDisplayPage;
	      break;
	 case msMenuSettingPump:
	      SubMenu=FSettingPump();
		  if (SubMenu==MENU_DONE)stMenuSettings=msDisplayPage;
	      break;
	 case msMenuSettingDec:
	      SubMenu=FSettingDec();
		  if (SubMenu==MENU_DONE)stMenuSettings=msDisplayPage;
	      break;
     case msMenuSettingDatetime:
          SubMenu=FSettingDatetime();
		  if (SubMenu==MENU_DONE)stMenuSettings=msDisplayPage;
	      break;
	 case msMenuSettingPrinter: 
	      SubMenu=FSettingPrinter();
		  if (SubMenu==MENU_DONE)stMenuSettings=msDisplayPage;
	      break;
	 case msMenuSettingHost:
	      SubMenu=FSettingHost();
		  if (SubMenu==MENU_DONE)stMenuSettings=msDisplayPage;
	      break;
     case msMenuSettingOperator:
          SubMenu=FSettingOperator();
		  if (SubMenu==MENU_DONE)stMenuSettings=msDisplayPage;
	      break;
	 case msMenuSettingSystem:
	      SubMenu=FSettingSystem();
		  if (SubMenu==MENU_DONE)stMenuSettings=msDisplayPage;
	      break;	 
     case msMenuSettingPumpPooling:
	      SubMenu=FSettingPumpPooling();
		  if (SubMenu==MENU_DONE)stMenuSettings=msDisplayPage;
	      break;
//------------------------------------------------------------------
     case msMenuSettingExit:
	      stMenuSettings=msInit;
	      Result=MENU_DONE;
	      break;
	 }
   return Result;
}

void menu_product(){

}

char FSettingProduct(){//Using strDescription
static char stMenuProduct=mpInitProduct,ProdID;//,IsMenuProductExit=False;
//static char strPrice[9];
     char KeyPressed,KeyChar,uiResult=USER_NONE;
     char i;//,x,y;
	 char strProductName[13],lcdteks[20];
	 char Result=MENU_NONE;
	 
     Result=MENU_NONE;
     switch(stMenuProduct){
	 case mpInitProduct:
	      //DisplayProductName
		  lcd_clear();
		  //lcd_printf(1,1,PSTR("Product:"));
	      for(i=0;i<6;i++){//234 
	          eeprom_read_block((void*) &strProductName, (const void*) &DefProductName[i], 13);
	          sprintf_P(lcdteks,PSTR("%d)%s"),(i+1),strProductName);
			  lcd_print((((i)%4)+1),(((i)/4)*10)+1,lcdteks);
		  }
		  lcd_printf(4,11,PSTR("*)Back"));
          stMenuProduct=mpChangeProduct;
	      break;
     case mpChangeProduct:
		  KeyPressed=_key_scan(1);
		  KeyChar=_key_btn(KeyPressed);
		  if ((KeyChar>='1')&&(KeyChar<='6')){
		      ProdID=KeyChar-'1';
			  stMenuProduct=mpDispPrice;
		  }
		  if (KeyPressed==_KEY_CANCEL){
		      stMenuProduct=mpExitMenuProduct;
		  }
	      break;
     case mpDispPrice:
	 	  eeprom_read_block((void*) &strProductName, (const void*) &DefProductName[ProdID], 13);
		  eeprom_read_block((void*) &strPrice, (const void*) &DefProductPrice[ProdID], 9);
		  sprintf_P(lcdteks,PSTR("1)%s"),strProductName);
		  lcd_clear();lcd_print(1,1,lcdteks);
		  sprintf_P(lcdteks,PSTR("2)%s"),strPrice);
		  lcd_print(2,1,lcdteks);
		  lcd_printf(4,1,PSTR("*)Back       "));
		  stMenuProduct=mpIsEdit;
	      break;
     case mpIsEdit:
		  KeyPressed=_key_scan(1);
		  KeyChar=_key_btn(KeyPressed);
		  if (KeyChar=='*'){
		      stMenuProduct=mpInitProduct;
		  }else
		  if (KeyChar=='1'){
		      eeprom_read_block((void*) &strProductName, (const void*) &DefProductName[ProdID], 13);
		      sprintf_P(lcdteks,PSTR("Old:%s"),strProductName);
			  lcd_printf(1,1,PSTR("Edit Product Name   "));
		       lcd_print(2,1,lcdteks);
			  lcd_printf(3,1,PSTR("New:_"));
		      lcd_printf(4,1,PSTR("*)Back    #)OK      "));
		      stMenuProduct=mpEditProductName;
		  }else
		  if (KeyChar=='2'){
		      uiResult=USER_NONE;
		      sprintf_P(lcdteks,PSTR("Old:%s"),strPrice);
		       lcd_print(2,1,lcdteks);
			  lcd_printf(3,1,PSTR("New:_"));
		      lcd_printf(4,1,PSTR("*)Back    #)OK      "));
		      stMenuProduct=mpEditPrice;
		  }
	      break;
     case mpEditProductName:
	      uiResult=UserInput(UI_ALPHANUM_R,3,5,strDescription,0,10);
		  if (uiResult==USER_OK){
		      eeprom_write_block((const void*)&strDescription,(void*)&DefProductName[ProdID], 13);
		      stMenuProduct=mpInitProduct;
			  }
		  else
		  if (uiResult==USER_CANCEL)stMenuProduct=mpDispPrice;
	      break;
     case mpEditPrice:
	      uiResult=UserInput(UI_NUMBER_R,3,5,strPrice,0,7);
		  if (uiResult==USER_OK){
		      eeprom_write_block((const void*)&strPrice,(void*)&DefProductPrice[ProdID], 9);
		      stMenuProduct=mpDispPrice;//stMenuProduct=mpInit;
			  }
		  else
		  if (uiResult==USER_CANCEL)stMenuProduct=mpDispPrice;
	      break;
     case mpExitMenuProduct:
	      stMenuProduct=mpInitProduct;
	      Result=MENU_DONE;
	      break;
	 }
   return Result;
}

char FSettingPump(){
//static char stSettingPump=spInitSettingPump;
/*       char KeyPressed,KeyChar;


	 switch(stSettingPump){
	 case spInitSettingPump:
	      lcd_clear();
		  lcd_printf(1,1, PSTR("1.ID     "));
		  lcd_printf(2,1, PSTR("2.Product"));
		  if (IFType==IT_STANDALONE){		    
		      lcd_printf(4,1, PSTR("3.Brand  *)Exit"));
		  }
		  stSettingPump=spSelectPumpSetting;
	      break;
     case spSelectPumpSetting:
	      KeyPressed=key_scan(1);
		  KeyChar=_key_btn(KeyPressed);


	      break;
	 
	 }
*/
	 char __key,Result;
		lcd_clear();_delay_ms(10);
		lcd_printf(1,1, PSTR("1)ID     "));
		lcd_printf(2,1, PSTR("2)Product"));
		if (IFType==IT_STANDALONE){
		    lcd_printf(3,1, PSTR("3)Label  "));
		    lcd_printf(4,1, PSTR("4)Brand  *)Exit"));
		}

		while(1){
			__key = _key_scan(1);
			if(__key == _KEY_CANCEL)
				break;
			if(__key == _KEY_1){
				_menu_pumpid();
				break;
			}
			if(__key == _KEY_2){
				_menu_pumpprod();
				break;
			}

			if((__key == _KEY_3)&&(IFType==IT_STANDALONE)){
			    FMenuPumpLabel();
				break;
			}
			if((__key == _KEY_4)&&(IFType==IT_STANDALONE)){
			    FMenuPumpType();
				break;
			}

		}

	Result=MENU_DONE;
	return Result;
}

char FSettingDec(){
static char stSettingDecimal=sdInitDisplay;
     char PDecimalConfig[4],Addr,KeyChar,Result;
	 char lcdteks[20];

     Result=MENU_NONE;
	 switch(stSettingDecimal){
	 case sdInitDisplay:
	      lcd_clear();         //"12345678901234567890
		  PDecimalConfig[0]=eeprom_read_byte(&DefDecimalPrice);
		  PDecimalConfig[1]=eeprom_read_byte(&DefDecimalVolume);
		  PDecimalConfig[2]=eeprom_read_byte(&DefDecimalMoney);

		  sprintf_P(lcdteks,PSTR("1.Price :%d"),PDecimalConfig[0]);
		  lcd_print(1,1,lcdteks);
		  sprintf_P(lcdteks,PSTR("2.Volume:%d"),PDecimalConfig[1]);
		  lcd_print(2,1,lcdteks);
		  sprintf_P(lcdteks,PSTR("3.Money :%d"),PDecimalConfig[2]);
		  lcd_print(3,1,lcdteks);
		  lcd_printf(4,1,PSTR("[*]Back  [#]Next"));
		  stSettingDecimal=sdSelectKey1;
		  break;
     case sdSelectKey1:
          KeyChar=_key_btn(_key_scan(1));
		  if ((KeyChar>='1')&&(KeyChar<='3')){
		      Addr=KeyChar-'1';
		  	  PDecimalConfig[Addr]=eeprom_read_byte(&DefDecimalPrice+Addr);
			  PDecimalConfig[Addr]=(PDecimalConfig[Addr]+1)%4;

			  eeprom_write_byte(&DefDecimalPrice+Addr,PDecimalConfig[Addr]);
              stSettingDecimal=sdInitDisplay;
		  }else if (KeyChar=='*') stSettingDecimal=sdExitSettingDecimal;
		  else if (KeyChar=='#') stSettingDecimal=sdInitDisplay2;
          break;
     case sdInitDisplay2:
		  PDecimalConfig[0]=eeprom_read_byte(&DefDecimalTotalVolume);
		  PDecimalConfig[1]=eeprom_read_byte(&DefDecimalTotalMoney);
		  PDecimalConfig[2]=eeprom_read_byte(&DefDecimalMark);
		  PDecimalConfig[3]=eeprom_read_byte(&DefCurrencyMark);
		  sprintf_P(lcdteks,PSTR("1.T.Volume:%d "),PDecimalConfig[0]);
		  lcd_print(1,1,lcdteks);
		  sprintf_P(lcdteks,PSTR("2.T.Money :%d "),PDecimalConfig[1]);
		  lcd_print(2,1,lcdteks);
		  sprintf_P(lcdteks,PSTR("3.Decimal :%c "),PDecimalConfig[2]);
		  lcd_print(3,1,lcdteks);
		  sprintf_P(lcdteks,PSTR("4.Sparator:%c "),PDecimalConfig[3]);
		  lcd_print(4,1,lcdteks);
		  lcd_printf(4,14,PSTR("*)Back"));
	      stSettingDecimal=sdSelectKey2;
	      break;
     case sdSelectKey2:
          KeyChar=_key_btn(_key_scan(1));
		  if ((KeyChar>='1')&&(KeyChar<='2')){
		      Addr=KeyChar-'1';
		  	  PDecimalConfig[Addr]=eeprom_read_byte(&DefDecimalTotalVolume+Addr);
		      PDecimalConfig[Addr]=(PDecimalConfig[Addr]+1)%4;

			  eeprom_write_byte(&DefDecimalTotalVolume+Addr,PDecimalConfig[Addr]);
              stSettingDecimal=sdInitDisplay2;
		  }else if ((KeyChar>='3')&&(KeyChar<='4')){
		      Addr=KeyChar-'1';
		  	  PDecimalConfig[Addr]=eeprom_read_byte(&DefDecimalTotalVolume+Addr);
			  PDecimalConfig[Addr]=SelectMark(PDecimalConfig[Addr]);
			  eeprom_write_byte(&DefDecimalTotalVolume+Addr,PDecimalConfig[Addr]);
              stSettingDecimal=sdInitDisplay2;
		  }else if (KeyChar=='*') stSettingDecimal=sdInitDisplay;
/*		  else if (KeyChar=='5'){
		          lcd_clear();
		          stSettingDecimal=sdTestInput;
				  }*/
          break;
     case sdTestInput:
	      //if (TestUserInput()==MENU_DONE)
		  stSettingDecimal=sdInitDisplay2;
	      break;
	 case sdExitSettingDecimal:
          stSettingDecimal=sdInitDisplay;
		  Result=MENU_DONE;
	      break;
	 }
    return Result;
}

char SelectMark(char InMark){
char i,Result,PMark[5],Length;
     eeprom_read_block((void*)&PMark,(const void*)&DefMarkMap, sizeof(DefMarkMap));
	 Length=sizeof(DefMarkMap);
	 Result=InMark;
     for(i=0;i<Length;i++){
	     if (InMark==PMark[i])
		     Result=PMark[(i+1)%Length];
	 }
     return Result;
}

char FSettingDatetime(){
     _menu_datetime();
     return MENU_DONE;
}
char FSettingPrinter(){
     _menu_printer();
	 return MENU_DONE;
}
char FSettingHost(){
     _menu_host();
	 return MENU_DONE;
}

char FSettingOperator(){//Change Active Operator, Change Password
     static char stSettingOperator=soMenuOption;
	 static char PassType=PT_NONE;
	        char KeyPressed,KeyChar,lcdteks[20],strPassword[15];
			char strOperatorName[18];
			char uiResult=USER_NONE,Result=MENU_NONE;

	 Result=MENU_NONE;
	 switch(stSettingOperator){
	 case soMenuOption:
	      lcd_clear();
		  lcd_printf(1,1,PSTR("1.Change Operator"));
		  lcd_printf(2,1,PSTR("2.Change Password"));
		  lcd_printf(4,1,PSTR("[*]Back          "));
          stSettingOperator=soMenuOptionInput;
	      break;
     case soMenuOptionInput:
	      KeyPressed=_key_scan(1);
		  KeyChar=_key_btn(KeyPressed);
		  switch(KeyChar){
		  case '1':
		       stSettingOperator=soChangeOperatorInit;
		       break;
          case '2':
		       stSettingOperator=soChangePasswordInit;
		       break;
          case '*':
		       stSettingOperator=soExitSettingOperator;
		       break;
		  }
	      break;
     case soChangeOperatorInit:
	      lcd_clear();		  
		  eeprom_read_block((void*) &strOperatorName, (const void*) &DefOperatorName,18);
		  StrPosCopy(strOperatorName,strOperatorName,0,15);

		  sprintf_P(lcdteks,PSTR("%s"),strOperatorName);
		  lcd_printf(1,1,PSTR("Name:"));
		  lcd_print (1,6,lcdteks);
		  lcd_printf(2,1,PSTR("New :_              "));
		  lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
          stSettingOperator=soOperatorNameInput;
	      break;
     case soOperatorNameInput:
          uiResult=USER_NONE;
	      uiResult=UserInput(UI_ALPHANUM_R,2,6,strFreeMessageLine1,0,16);
		  switch(uiResult){
		  case USER_OK:
		       lcd_clear();
			   StrPosCopy(strFreeMessageLine1,strOperatorName,0,15);
		       sprintf_P(lcdteks,PSTR("%s"),strOperatorName);
			   lcd_printf(1,1,PSTR("New Operator:"));
			   lcd_print (2,1,lcdteks);
			   lcd_printf(3,1,PSTR("Save?"));
			   lcd_printf(4,1,PSTR("[*]No    [#]Yes"));
               stSettingOperator=soIsSaveOperatorName;
		       break;
		  case USER_CANCEL:
               stSettingOperator=soMenuOption; 
		       break;
          case USER_ENTRY:
		       lcd_printf(4,1,PSTR("[*]Back     [#]Enter"));
		       break;
          case USER_NO_DATA:
		       lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
		       break;
		  }
	      break;
     case soIsSaveOperatorName:
          KeyPressed=_key_scan(1);
		  KeyChar=_key_btn(KeyPressed);
		  switch(KeyChar){
          case '*':
		       stSettingOperator=soMenuOption;
		       break;
          case '#':
		       StrPosCopy(strFreeMessageLine1,strOperatorName,0,15);
               AddSpaceLag(strOperatorName,18); 
		       eeprom_write_block((const void*) &strOperatorName, (void*) &DefOperatorName,18);
		       lcd_printf(3,1,PSTR("Saved "));
			   TimDisplay=0;
			   system_beep(1);
		       stSettingOperator=soDelayDisplaySaved;
		       break;
		  }	      
		  break;
     case soChangePasswordInit:
	      lcd_clear();
		  lcd_printf(1,1,PSTR("-Change Password-"));
		  lcd_printf(2,1,PSTR("1.Supervisor"));
		  lcd_printf(3,1,PSTR("2.Administrator"));
		  lcd_printf(4,1,PSTR("[*]Back     "));
          stSettingOperator=soChangePasswordInput; 
	      break;
     case soChangePasswordInput:
          KeyPressed=_key_scan(1);
		  KeyChar=_key_btn(KeyPressed);
		  switch(KeyChar){
		  case '1':		 
		       PassType=PT_SUPERVISOR;
		       stSettingOperator=soOldPasswordDisplay;
		       break;
          case '2':		       
		       PassType=PT_ADMINISTRATOR;
		       stSettingOperator=soOldPasswordDisplay;
		       break;
          case '*':
		       stSettingOperator=soMenuOption;
		       break;
		  }	      
	      break;
     case soOldPasswordDisplay:
	 	  lcd_clear();
	      if (PassType==PT_SUPERVISOR){
		      lcd_printf(1,1,PSTR("-Supervisor-"));              
		  }else if (PassType==PT_ADMINISTRATOR){		       
		       lcd_printf(1,1,PSTR("-Administrator-"));			   
		  }
		      lcd_printf(2,1,PSTR("Old:_"));
			  lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));

          stSettingOperator=soOldPasswordEntry;
	      break;
     case soOldPasswordEntry:
          uiResult=USER_NONE;
          uiResult=UserInput(UI_NUM_PASSWORD,2,5,strFreeMessageLine1,0,8);           
		  switch(uiResult){
		  case USER_OK:
		       lcd_printf(2,1,PSTR("New:_               "));
			   lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
		       stSettingOperator=soNewPasswordEntry1;
		       break;
		  case USER_CANCEL:
               stSettingOperator=soMenuOption; 
		       break;
          case USER_ENTRY:
		       lcd_printf(4,1,PSTR("[*]Back     [#]Enter"));
		       break;
          case USER_NO_DATA:
		       lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
		       break;
		  }
	      break;
     case soNewPasswordEntry1:
          uiResult=USER_NONE;
          uiResult=UserInput(UI_NUM_PASSWORD,2,5,strFreeMessageLine2,0,8);           
		  switch(uiResult){
		  case USER_OK:
		       lcd_printf(3,1,PSTR("New:_               "));
			   lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
		       stSettingOperator=soNewPasswordEntry2;
		       break;
		  case USER_CANCEL:		       
			   stSettingOperator=soOldPasswordDisplay;
		       break;
          case USER_ENTRY:
		       lcd_printf(4,1,PSTR("[*]Back     [#]Enter"));
		       break;
          case USER_NO_DATA:
		       lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
		       break;
		  }	      
	      break;
     case soNewPasswordEntry2:
          uiResult=USER_NONE;
          uiResult=UserInput(UI_NUM_PASSWORD,3,5,strFreeMessageLine3,0,8);           
		  switch(uiResult){
		  case USER_OK:
		       lcd_clear();
               lcd_printf(2,1,PSTR("Validating.."));
			   TimDisplay=0;
		       stSettingOperator=soDispValidatePassword;
		       break;
		  case USER_CANCEL:
               lcd_clear();
               lcd_printf(2,1,PSTR("New:_               "));
			   lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
		       stSettingOperator=soNewPasswordEntry1;
		       break;
          case USER_ENTRY:
		       lcd_printf(4,1,PSTR("[*]Back     [#]Enter"));
		       break;
          case USER_NO_DATA:
		       lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
		       break;
		  }	      
	      break;
     case soDispValidatePassword:
	      if (TimDisplay>3)stSettingOperator=soValidatePassword;
	      break;
     case soValidatePassword:
          if (PassType==PT_SUPERVISOR) eeprom_read_block((void*) &strPassword, (const void*) &DefSysPassword, 10);		      			   
		  else 
		  if (PassType==PT_ADMINISTRATOR) eeprom_read_block((void*) &strPassword, (const void*) &DefSpvPassword, 10);    
		  
		  //Check Old Validity
		  if (strcmp(strPassword,strFreeMessageLine1)==0){//OldMessage Valid
              if (strcmp(strFreeMessageLine2,strFreeMessageLine3)==0){
                  
			      if (PassType==PT_SUPERVISOR) eeprom_write_block((const void*) &strFreeMessageLine2, (void*) &DefSysPassword, 10);		      			   
		          else 
		          if (PassType==PT_ADMINISTRATOR) eeprom_write_block((const void*) &strFreeMessageLine2, (void*) &DefSpvPassword, 10);    

			      system_beep(1);
				  lcd_clear();
			      lcd_printf(2,1,PSTR("     Completed      "));
			      lcd_printf(3,1,PSTR("   Password Saved   "));    
				  TimDisplay=0;
				  stSettingOperator=soDelayDisplaySaved;
			  }else {
			  system_beep(2);
		      TimDisplay=0;
		      lcd_clear();
		      lcd_printf(2,1,PSTR("      ERROR         "));
		      lcd_printf(3,1,PSTR("Invalid New Password"));
		      stSettingOperator=soDisplayInvalidPassword;
			  }
		  }else {
		   //InvalidOld Password
		   system_beep(2);
		   TimDisplay=0;
		   lcd_clear();
		   lcd_printf(2,1,PSTR("      ERROR         "));
		   lcd_printf(3,1,PSTR("Invalid Old Password"));
		   stSettingOperator=soDisplayInvalidPassword;
		   }
	      break;
     case soDisplayInvalidPassword:
	      if (TimDisplay>6)stSettingOperator=soMenuOption;
	      break;
     case soDelayDisplaySaved:
	      if (TimDisplay>4)stSettingOperator=soMenuOption;
	      break;
	 case soExitSettingOperator:
	      stSettingOperator=soMenuOption;
	      Result=MENU_DONE;
	      break;
	 }
     return Result;
}

char FSettingSystem(){
static char stSettingSytem=ssInitSettingSystem;
       char KeyPressed,KeyChar,brVal,i;  
       char SubMenu,Result=MENU_NONE;
	   char HGMode;
       char lcdteks[20];
	   int bValue;

     switch(stSettingSytem){
	 case ssInitSettingSystem:
	      IFType=eeprom_read_byte(&DefInitIFT);
		  HGMode=eeprom_read_byte(&DefHGMode);

	      if (IFType==IT_NONE)sprintf_P(lcdteks,PSTR("1.Mode:None"));
		  else
	      if (IFType==IT_SLAVE)sprintf_P(lcdteks,PSTR("1.Mode:Slave"));
		  else
	      if (IFType==IT_STANDALONE)sprintf_P(lcdteks,PSTR("1.Mode:Standalone"));

	      lcd_clear();
		  lcd_print(1,1,lcdteks);
		  lcd_printf(2,1,PSTR("2.Baudrate"));

		  if (HGMode==HM_TTL)lcd_printf(3,1,PSTR("3.COM3:TTL"));
		  else
		  if (HGMode==HM_232)lcd_printf(3,1,PSTR("3.COM3:232"));
		  else
		  if (HGMode==HM_485)lcd_printf(3,1,PSTR("3.COM3:485"));		  
		  lcd_printf(4,1,PSTR("[*]Back      "));
		  stSettingSytem=ssMenuSelect;
	      break;
     case ssMenuSelect:
          KeyPressed=_key_scan(1);
		  KeyChar=_key_btn(KeyPressed);
		  if (KeyChar=='1'){
		      IFType=((IFType+1)%3);
			  SendSlaveCommand(IFType,DispenserBrand);
			  eeprom_write_byte(&DefInitIFT,IFType);
			  stSettingSytem=ssInitSettingSystem;
		  }
		  else
		  if (KeyChar=='2'){
		      stSettingSytem=ssComSettings;
		  }
		  else
		  if (KeyChar=='3'){
		      HGMode=eeprom_read_byte(&DefHGMode);
		      HGMode=((HGMode+1)%3);
			  SendSlaveCommand(SC_HGM_MODE,HGMode);
			  eeprom_write_byte(&DefHGMode,HGMode);
			  stSettingSytem=ssInitSettingSystem;
		  }
		  //else
		  if (KeyChar=='*'){
		      stSettingSytem=ssExitSystemSettings;
		  }		  
		  break;
	 case ssComSettings:
	      lcd_clear();
		  lcd_printf(1,1,PSTR("     -Baudrate-     "));

		  for (i=0;i<4;i++){
		       bValue=GetBaudrate(eeprom_read_byte(&DefBaudrate[i]));
			   if (bValue==5787)bValue=12213;
		       sprintf_P(lcdteks,PSTR("COM%d:%d"),i+1,bValue);
			   lcd_print((2+(i%2)),1+(i/2*11),lcdteks);
		  }
		  lcd_printf(4,1,PSTR("[*]Back     "));
		  stSettingSytem=ssBaudrateInput;
	      break;
     case ssBaudrateInput:
	      KeyPressed=_key_scan(1);
		  KeyChar=_key_btn(KeyPressed);
		  if ((KeyChar>='1')&&(KeyChar<='4')){
		       brVal=eeprom_read_byte(&DefBaudrate[KeyChar-'1']);
			   brVal=((brVal+1)%3)+1;
			   SetBaudRate(KeyChar-'0',brVal);
			   eeprom_write_byte(&DefBaudrate[KeyChar-'1'],brVal);
               stSettingSytem=ssComSettings;
		  }else if (KeyChar=='*')
		      stSettingSytem=ssExitSystemSettings;		  	
	      break;
	 case ssPumpPoolingSettings:
          SubMenu=FSettingPumpPooling();
		  if (SubMenu==MENU_DONE)stSettingSytem=ssInitSettingSystem;
	      break;
     case ssExitSystemSettings:
	      stSettingSytem=ssInitSettingSystem;
	      Result=MENU_DONE;
	      break;
	 }
     return Result;
}

char FSettingPumpPooling(){//Parameter: PumpCount,
//SC_GET_POOLING_NO_PUMP_COUNT,SC_GET_POOLING_MAX_PUMP,SC_GET_POOLING_SEND,
//SC_SET_POOLING_NO_PUMP_COUNT,SC_SET_POOLING_MAX_PUMP,SC_SET_POOLING_SEND,

char Result=MENU_NONE;
static char stPumpPooling=ppInitMenu;
	   char i,lcdteks[20];
	   char KeyChar,Addr=0,PPoolingSetting[6];
     
	 Result=MENU_NONE;
     switch(stPumpPooling){
	 case ppInitMenu:
          lcd_clear();
		  for (i=0;i<6;i++){
		      PPoolingSetting[i]=eeprom_read_byte(&DefPoolingPumpMax+i);
		  }

		  sprintf_P(lcdteks,PSTR("1.MaxPump:%d"),PPoolingSetting[0]);lcd_print(1,1,lcdteks);
          sprintf_P(lcdteks,PSTR("2.nNoPump:%d"),PPoolingSetting[1]);lcd_print(2,1,lcdteks);
          sprintf_P(lcdteks,PSTR("3.TrySend:%d"),PPoolingSetting[2]);lcd_print(3,1,lcdteks);
          sprintf_P(lcdteks,PSTR("4.TimPool:%d"),PPoolingSetting[3]);lcd_print(4,1,lcdteks);
          sprintf_P(lcdteks,PSTR("5.Dly:%d"),PPoolingSetting[4]);lcd_print(1,13,lcdteks);
          sprintf_P(lcdteks,PSTR("6.Act:%d"),PPoolingSetting[5]);lcd_print(2,13,lcdteks);
          sprintf_P(lcdteks,PSTR("[*]Exit"));lcd_print(4,13,lcdteks);

          stPumpPooling=ppPoolingSettingInput;
	      break;
     case ppDisplaySequence:
          IsNewPoolingSequence=False;
		  DisplayStandaloneSequence(3,14,iSequencePooling);
          stPumpPooling=ppPoolingSettingInput;
	      break;
     case ppPoolingSettingInput:
	      if(IsNewPoolingSequence==True)stPumpPooling=ppDisplaySequence;

          KeyChar=_key_btn(_key_scan(1));
		  if ((KeyChar>='1')&&(KeyChar<='6')){
		      Addr=KeyChar-'1';
			  TimSend=0;
			  IsStandaloneAcknoledge=False;
          }
		  switch(KeyChar){
		  case '1'://MaxPump
		       PPoolingSetting[Addr]=((PPoolingSetting[Addr]+1)%17);
			   if (PPoolingSetting[Addr]==0)PPoolingSetting[Addr]=1;
			   //SendSlaveCommand(SC_SET_POOLING_MAX_PUMP,PPoolingSetting[Addr]);
			   SendPoolingCommand(SC_SET_POOLING_MAX_PUMP,PPoolingSetting[Addr]);

			   eeprom_write_byte(&DefPoolingPumpMax+Addr,PPoolingSetting[Addr]);
			   PumpCountMax=PPoolingSetting[Addr];
			   //stPumpPooling=ppInitMenu;
			   stPumpPooling=ppWaitPoolingRespond;
		       break;
		  case '2'://NoPump
		       PPoolingSetting[Addr]=((PPoolingSetting[Addr]+1)%21);
			   //SendSlaveCommand(SC_SET_POOLING_NO_PUMP_COUNT,PPoolingSetting[Addr]);
			   SendPoolingCommand(SC_SET_POOLING_NO_PUMP_COUNT,PPoolingSetting[Addr]);

			   eeprom_write_byte(&DefPoolingPumpMax+Addr,PPoolingSetting[Addr]);
			   //stPumpPooling=ppInitMenu;
			   stPumpPooling=ppWaitPoolingRespond;
		       break;
		  case '3'://TrySend
		       PPoolingSetting[Addr]=((PPoolingSetting[Addr]+1)%21);
			   if (PPoolingSetting[Addr]==0)PPoolingSetting[Addr]=1;
			   //SendSlaveCommand(SC_SET_POOLING_SEND,PPoolingSetting[Addr]);
			   SendPoolingCommand(SC_SET_POOLING_SEND,PPoolingSetting[Addr]);

			   eeprom_write_byte(&DefPoolingPumpMax+Addr,PPoolingSetting[Addr]);
			   //stPumpPooling=ppInitMenu;
			   stPumpPooling=ppWaitPoolingRespond;
		       break;
		  case '4'://TimPool
		       PPoolingSetting[Addr]=((PPoolingSetting[Addr]+1)%21);
			   //SendSlaveCommand(SC_SET_POOLING_TIMEOUT,PPoolingSetting[Addr]);
			   SendPoolingCommand(SC_SET_POOLING_TIMEOUT,PPoolingSetting[Addr]);

			   eeprom_write_byte(&DefPoolingPumpMax+Addr,PPoolingSetting[Addr]);
			   //stPumpPooling=ppInitMenu;
			   stPumpPooling=ppWaitPoolingRespond;
		       break;
		  case '5'://DelayNextPump
		       PPoolingSetting[Addr]=((PPoolingSetting[Addr]+1)%41); 
			   //SendSlaveCommand(SC_SET_POOLING_DELAY_NEXT_PUMP,PPoolingSetting[Addr]);
			   SendPoolingCommand(SC_SET_POOLING_DELAY_NEXT_PUMP,PPoolingSetting[Addr]);

			   eeprom_write_byte(&DefPoolingPumpMax+Addr,PPoolingSetting[Addr]);
			   //stPumpPooling=ppInitMenu;
               stPumpPooling=ppWaitPoolingRespond;
		       break;
		  case '6'://ActivePump
		       PPoolingSetting[Addr]=((PPoolingSetting[Addr]+1)%(1+eeprom_read_byte(&DefPoolingPumpMax))); 			       
			   eeprom_write_byte(&DefPoolingPumpMax+Addr,PPoolingSetting[Addr]);
			   stPumpPooling=ppInitMenu;
		       break;
		  case '*'://Exit
		       stPumpPooling=ppExitSettingPooling;		            
		       break;
		  }  
	      break;
     case ppWaitPoolingRespond:	    
	      if ((IsStandaloneAcknoledge==True)||(TimSend>5))stPumpPooling=ppInitMenu;
	      break;
     case ppExitSettingPooling:
          stPumpPooling=ppInitMenu;
	      Result=MENU_DONE;
	      break;
    }
return Result;
}

//String Processing
void AddCharLag(char *String,char CharAdded,unsigned char Size){
char i,Length;
     Length=strlen(String);
  if (Length<Size){   
     for(i=Length;i<Size;i++){
	    String[i]=CharAdded;
	 }String[Size]=0;
  }

}

void AddCharLead(char *String,char CharAdded,unsigned char Size){//
     char i,Length,strAdded[30];
     Length=strlen(String);

	 if (Size>Length){
         for(i=0;i<Size;i++){
	         strAdded[i]=CharAdded;
	     }strAdded[Size]=0;
	     //Copy
         for(i=(Size-Length);i<Size;i++){
	         strAdded[i]=String[i-(Size-Length)];
	     }strAdded[Size]=0;
	     //Spaced
         for(i=0;i<Size;i++){
	         String[i]=strAdded[i];
	     }String[Size]=0;
	 }
}



void AddSpaceLag(char *String,unsigned char Size){// [CREDITCARD             ]
char i,Length;
     Length=strlen(String);
  if (Length<Size){   
     for(i=Length;i<Size;i++){
	    String[i]=' ';
	 }String[Size]=0;
  }
}

void AddSpaceLead(char *String,unsigned char Size){//
     char i,Length,strAdded[50];
     Length=strlen(String);

	 if (Size>Length){
         for(i=0;i<Size;i++){
	         strAdded[i]=' ';
	     }strAdded[Size]=0;
	     //Copy
         for(i=(Size-Length);i<Size;i++){
	         strAdded[i]=String[i-(Size-Length)];
	     }strAdded[Size]=0;
	     //Spaced
         for(i=0;i<Size;i++){
	         String[i]=strAdded[i];
	     }String[Size]=0;
	 }
}

void AddZeroLag(char *String,unsigned char Size){// [123]->12300
char i,Length;
     Length=strlen(String);
  if (Length<Size){   
     for(i=Length;i<Size;i++){
	    String[i]='0';
	 }String[Size]=0;
  }
}


void AddZeroLead(char *String,unsigned char Size){// 1234 ->0000001234
     char i,Length,strAdded[30];
     Length=strlen(String);

	 if (Size>Length){
         for(i=0;i<Size;i++){
	         strAdded[i]='0';
	     }strAdded[Size]=0;
	     //Copy
         for(i=(Size-Length);i<Size;i++){
	         strAdded[i]=String[i-(Size-Length)];
	     }strAdded[Size]=0;
	     //Zeroed
         for(i=0;i<Size;i++){
	         String[i]=strAdded[i];
	     }String[Size]=0;
	 }
}

void leadingZero(char Val,char *StrResult){
     sprintf_P(StrResult,PSTR("%.2d"),Val);

}
void _scr_pump(void){
	 //char strPumpL[3],strPumpR[3];
	 char __pump_id[8];
	 char i;
     char lcdteks[20];
	 lcd_clear();_delay_ms(10);
	 //Display
		eeprom_read_block((void*) &__pump_id, (const void*) &DefPumpMap, 8);
        for (i=0;i<4;i++){
             //leadingZero(__pump_id[i],strPumpL);
			 //leadingZero(__pump_id[i+4],strPumpR);
             //sprintf_P(lcdteks,PSTR("%d.P%s | %.P%s  "),(i+1),strPumpL,(i+5),strPumpR);
			 sprintf_P(lcdteks,PSTR("%d.P%.2d | %d.P%.2d "),(i+1),__pump_id[i],(i+5),__pump_id[i+4]);
			 lcd_print((i+1),1,lcdteks);
		}
        lcd_printf(4,15,PSTR("*)Exit"));
}



void _menu_pump(void){
	char __key;

	while(1){
		lcd_clear();_delay_ms(10);
		lcd_printf(1, 1, PSTR("1)ID"));
		lcd_printf(2, 1, PSTR("2)Product"));
		lcd_printf(4, 1, PSTR("*)Exit"));

		while(1){
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






void FMenuPumpType(){
     char stPumpType=ptInitMenu;
	 char lcdteks[20];
	 char KeyChar,KeyPressed,IdxType; 
	 char IsRunPumpType=True;
	 char DispenserBrandName[20];

     stPumpType=ptInitMenu;
	 IsRunPumpType=True;
	 while(IsRunPumpType==True){
		  switch(stPumpType){
		  case ptInitMenu:
		       lcd_clear();
			   lcd_printf(1,1,PSTR("-Dispenser Brand-"));
			   IdxType=eeprom_read_byte(&DefDispenserBrand);
			   //Brand Selection
			   if (IdxType>=3) IdxType=0;
               strcpy_P(DispenserBrandName,(PGM_P)pgm_read_word(&(DefListDispenserName[IdxType])));
			   sprintf_P(lcdteks,PSTR("%d.%s"),IdxType+1,DispenserBrandName);
			   lcd_print(2,1,lcdteks);
			   lcd_printf(4,1,PSTR("[*]Exit  [#]Change "));
			   if (IFType==IT_STANDALONE)
			       SendSlaveCommand(SC_STOP_POOL_SEQUENCE,0);
			   stPumpType=ptSelectBrand;
	 		   break;
          case ptSelectBrand:
		       KeyPressed = _key_scan(1);
		       KeyChar    = _key_btn(KeyPressed);

			   if (KeyChar=='#'){
			       DispenserBrand=eeprom_read_byte(&DefDispenserBrand);
				   if (DispenserBrand<ST_WAYNE_DART)
				        DispenserBrand++;
				   else DispenserBrand=ST_NONE;
                   //Apply DecimalSetting  
				   SetDispenser(DispenserBrand);
			       eeprom_write_byte(&DefDispenserBrand,DispenserBrand);
				   //SendPoolingCommand(SC_SET_PUMP_TYPE,eeprom_read_byte(&DefDispenserBrand));
				   SendSlaveCommand(SC_SET_PUMP_TYPE,eeprom_read_byte(&DefDispenserBrand));
				   IsSetPumpType=True;//False;
			       stPumpType=ptUpdated;			   
			   }else
			   if (KeyChar=='*'){
			       IsRunPumpType=False;
				   if (IFType==IT_STANDALONE)SendSlaveCommand(SC_START_POOL_SEQUENCE,0);
			   }
		       break;
          case ptUpdated:
		       if (IsSetPumpType==True){
			       stPumpType=ptInitMenu;
			   }
		       break;		         
	      }
	 }     
}

void FMenuPumpLabel(){
	char PPumpLabel[8];
	unsigned char i, xPos,yPos,KeyPressed,KeyChar,strLabel[5];
	static char MaxCountId=16;

	eeprom_read_block((void*) &PPumpLabel, (const void*) &DefPumpLabel, 8);
	MaxCountId=99;
	lcd_clear();_delay_ms(10);
	lcd_printf(1, 1, PSTR("1)1:   5)5:"));
	lcd_printf(2, 1, PSTR("2)2:   6)6:"));
	lcd_printf(3, 1, PSTR("3)3:   7)7:   #)Save"));
	lcd_printf(4, 1, PSTR("4)4:   8)8:   *)Exit"));

	for(i=0;i< 8;i++){
	    sprintf_P(strLabel,PSTR("%.2d"),PPumpLabel[i]);
		xPos=1+(i%4);
		yPos=5+(i/4)*8;
        lcd_print(xPos,yPos,strLabel);
	}

	while(1){
		KeyPressed = _key_scan(1);
		KeyChar    = _key_btn(KeyPressed);
		if(KeyPressed == _KEY_CANCEL)break;
		else
		if(KeyPressed == _KEY_ENTER){
			eeprom_write_block((const void*) &PPumpLabel, (void*) &DefPumpLabel, 8);
			break;
		}
		if ((KeyChar>='1') && (KeyChar<= '8')){
		     i=(KeyChar-'1');
			if(PPumpLabel[i] == MaxCountId)
			   PPumpLabel[i] = 0;
			else
			   PPumpLabel[i]++;
            xPos=1+(i%4);
		    yPos=5+(i/4)*8;
			sprintf_P(strLabel,PSTR("%.2d"),PPumpLabel[i]);
            lcd_print(xPos,yPos,strLabel);
		}
	}
}

void IncValue(char *Value,char MinValue,char MaxValue){
     if ((*Value)<MaxValue){
	     (*Value)++;
	 }else *Value=MinValue;
}

void DecValue(char *Value,char MinValue,char MaxValue){
     if ((*Value)>MinValue){
	     (*Value)--;
	 }else *Value=MaxValue;
}

char FSettingPumpID(){
     static char stMenuPumpID=mpInitPumpId,IsShift=False;
	 char iLoop,Result,KeyChar,KeyPressed,i,j,CheckPump;
	 char lcdteks[20];
	 static char Idx,PPumpId[8],MsgPumpId,PumpCount;
     
	 Result=MENU_NONE;
	 switch (stMenuPumpID){
	 case mpInitPumpId:	      
	      //Disable PumpPooling
		  if (IFType==IT_STANDALONE)SendSlaveCommand(SC_STOP_POOL_SEQUENCE,0);
		  eeprom_read_block((void*) &PPumpId, (const void*) &DefPumpMap,8);
          stMenuPumpID=mpDisplayPumpId;
	      break;
	 case mpDisplayPumpId:
	      lcd_clear();		  
		  for (iLoop=0;iLoop<4;iLoop++){
			  sprintf_P(lcdteks,PSTR("%d.P%.2d | %d.P%.2d"),(iLoop+1),PPumpId[iLoop],(iLoop+5),PPumpId[iLoop+4]);
			  lcd_print((iLoop+1),1,lcdteks);
          }
          lcd_printf(3,15,PSTR("*)Back"));
	      lcd_printf(4,15,PSTR("#)Save"));
          stMenuPumpID=mpSelectInput;
	      break;
     case mpSelectInput:
	      KeyPressed=_key_scan(1);
	      KeyChar=_key_btn(KeyPressed);
          if ((KeyChar>='1') && (KeyChar<='8')){
		      //eeprom_read_block((void*) &PPumpId, (const void*) &DefPumpMap,8);
			  switch(IFType){
			  case IT_SLAVE:
			       if (IsShift==False)IncValue(&PPumpId[KeyChar-'1'],0,99);//PPumpId[KeyChar-'1']=((PPumpId[KeyChar-'1']+1)%100);
				   else
				   if (IsShift==True)DecValue(&PPumpId[KeyChar-'1'],0,99);//PPumpId[KeyChar-'1']=100-(100-((PPumpId[KeyChar-'1']+1)%100));
			       break;
              case IT_STANDALONE:
			       if (IsShift==False)IncValue(&PPumpId[KeyChar-'1'],0,16);//PPumpId[KeyChar-'1']=((PPumpId[KeyChar-'1']+1)%16);
				   else
				   if (IsShift==True)DecValue(&PPumpId[KeyChar-'1'],0,16);//PPumpId[KeyChar-'1']=17-(17-((PPumpId[KeyChar-'1']+1)%17));
			       break;			  
			  }
			  //Redraw
			  for (iLoop=0;iLoop<4;iLoop++){
				  sprintf_P(lcdteks,PSTR("%d.P%.2d | %d.P%.2d"),(iLoop+1),PPumpId[iLoop],(iLoop+5),PPumpId[iLoop+4]);
				  lcd_print((iLoop+1),1,lcdteks);
	          }
		  }else if (KeyChar=='#'){
		      //Evaluate same PumpID
			  PumpCount=0;
			  for (i=0;i<8;i++){
			       CheckPump=PPumpId[i];
				   if (CheckPump>0){
				      for(j=0;j<8;j++){					      
					      if ((i!=j)&&(CheckPump==PPumpId[j]))
						       PumpCount++;
					  }
				    }				     
				  }			       			  			  
			  if (PumpCount>0) stMenuPumpID=mpSaveFailed;
			  else stMenuPumpID=mpSavingPumpId;
		  }	
		  else if (KeyChar=='*'){
		      stMenuPumpID=mpExitPumpId;
		  }
		  if (KeyPressed==_KEY_SHIFT){
		      if (IsShift==False)IsShift=True;
			  else
			  if (IsShift==True)IsShift=False;
		  }
	      break;
     case mpSaveFailed:
	      lcd_clear();
		  lcd_printf(1,1,PSTR("      Warning!      "));
		  lcd_printf(2,1,PSTR("  Duplicate PumpID  "));
		  lcd_printf(3,1,PSTR("    Not Allowed     "));		  
		  system_beep(2);
	      TimDisplay=0;
	      stMenuPumpID=mpDisplayFailed;
          break;	 
     case mpDisplayFailed:
	      if (TimDisplay>6) stMenuPumpID=mpDisplayPumpId;
	      break;
     case mpSavingPumpId:
	      eeprom_write_block((const void*) &PPumpId, (void*) &DefPumpMap, 8);
		  if (IFType==IT_STANDALONE){
		      //SendConfig to Slave
			  lcd_clear();
              lcd_printf(1,1,PSTR("-PumpID-"));
              lcd_printf(2,1,PSTR("Saving.."));
			  Idx=0;
			  PumpCount=0;
              eeprom_read_block((void*) &PPumpId, (const void*) &DefPumpMap,8);
              stMenuPumpID=mpSendConfigToSlave;
		  }else if (IFType==IT_SLAVE){
		      //SendConfig to Slave              
              stMenuPumpID=mpExitPumpId;
		  }
	      break;
     case mpSendConfigToSlave:
	      IsStandaloneAcknoledge=False;
	      if (PPumpId[Idx]!=0){
		      PumpCount++;
		      MsgPumpId=((Idx<<4)|(0x0F&PPumpId[Idx]));//[Idx][PumpId]
	         //SendPoolingCommand(SC_SET_PUMPID,MsgPumpId);
			  SendSlaveCommand(SC_SET_PUMPID,MsgPumpId);
		  }else SendSlaveCommand(SC_CLEAR_PUMPID,Idx);//SendPoolingCommand(SC_CLEAR_PUMPID,Idx);
		  sprintf_P(lcdteks,PSTR("Id[%d]=%.2d  "),Idx+1,PPumpId[Idx]);
		  lcd_print(3,1,lcdteks);
          stMenuPumpID=mpWaitSlaveReply;
	      break;
     case mpWaitSlaveReply:
	      KeyPressed=_key_scan(1);
	      KeyChar=_key_btn(KeyPressed);
		  if (KeyChar=='*'){
		      system_beep(2);
		      lcd_printf(2,1,PSTR("Cancel     "));
			  TimDisplay=0;
		      stMenuPumpID=mpDelayExitPumpId;
			  }
          if (IsStandaloneAcknoledge==True){
		      Idx++;
		      if (Idx<8){//Finish, Send MaxPumpCount
			      eeprom_write_byte(&DefPoolingPumpMax,PumpCount);
				  PumpCountMax=eeprom_read_byte(&DefPoolingPumpMax);
                  SendPoolingCommand(SC_SET_POOLING_MAX_PUMP,PumpCountMax);
			      TimDisplay=0;		 
				  stMenuPumpID=mpDelaySaveConfig;
			  }else{lcd_printf(2,1,PSTR("Completed"));
					TimDisplay=0;
				    stMenuPumpID=mpDelayExitPumpId;
				   }
			  }		  
	      break;
     case mpDelaySaveConfig:
	      if (TimDisplay>2)stMenuPumpID=mpSendConfigToSlave;
	      break;
     case mpDelayExitPumpId:
	      if (TimDisplay>8)stMenuPumpID=mpExitPumpId;
	      break;
     case mpExitPumpId:
	      if (IFType==IT_STANDALONE)
		      SendSlaveCommand(SC_START_POOL_SEQUENCE,0);
	      Result=MENU_DONE;
          stMenuPumpID=mpInitPumpId;
	      break;
	 }
   return Result;
}

void _menu_pumpid(void){
	while(1){
	   if (FSettingPumpID()==MENU_DONE){
	       break;
	   }	   
	}
}

void _menu_pumpprod(void){
	char KeyPressed,KeyChar;
	while(1){
		lcd_clear();
		_scr_pump();
		while(1){
			KeyPressed=_key_scan(1);
		    KeyChar=_key_btn(KeyPressed);

			if ((KeyChar>='1')&&(KeyChar<='8')){
			     _menu_pumpprodinput(KeyChar-'1');
				break;
			}else if (KeyChar=='*'){
			     return;
			}
			/*
			if( __key == _KEY_1 || __key == _KEY_2 || __key == _KEY_3 ||
				__key == _KEY_4 || __key == _KEY_5 || __key == _KEY_6 ||
				__key == _KEY_7 || __key == _KEY_8){
				_menu_pumpprodinput(_key_btn(__key) - 0x31);
				break;
			}
			if(__key == _KEY_CANCEL)
				return;
				*/
		}
	}
}

void _menu_pumpprodinput(unsigned char __select){
	char 			__pump_prod[6],__pump_id[8];
	unsigned char	i, __x, __y, __key, __num, __buff[5];
	char lcdteks[20];

	eeprom_read_block((void*) &__pump_id, (const void*) &DefPumpMap, 8);
	eeprom_read_block((void*) &__pump_prod, (const void*) &DefNozzleMap[__select], 6);

	lcd_clear();_delay_ms(10);
    sprintf_P(lcdteks, PSTR("Product FIP%.2d"),__pump_id[__select]);
    lcd_print(1, 1,lcdteks);

	lcd_printf(2, 1, PSTR("1)N1:  3)N3:  5)N5:"));
	lcd_printf(3, 1, PSTR("2)N2:  4)N4:  6)N6:"));
	lcd_printf(4, 1, PSTR("*)Exit        #)Save"));

	for(i=0;i<6;i++){
		__x = pgm_read_byte(&__prodloc[i][0]);
		__y = pgm_read_byte(&__prodloc[i][1]);

        sprintf_P(__buff,PSTR("%d"),__pump_prod[i]);
		lcd_print(__x+1, __y, __buff);
		//sprintf_P(lcdteks,PSTR("%d.N%d:%d "),i+1,i+1,__pump_prod[i]);
		//lcd_print(2+(i%2),(i/2)*6,lcdteks);
	}

	while(1){
		
		__key = _key_scan(1);
		__num = _key_btn(__key);
		if(__key == _KEY_CANCEL)
			return;
		if(__key == _KEY_ENTER){
			eeprom_write_block((const void*) &__pump_prod, (void*) &DefNozzleMap[__select], 6);
			return;
		}
		if(__num >= '1' && __num <= '6'){
			if(__pump_prod[__num - '1'] == 6)
				__pump_prod[__num - '1'] = 0;
			else
				__pump_prod[__num - '1']++;

			__x = pgm_read_byte(&__prodloc[__num - 0x31][0]);
			__y = pgm_read_byte(&__prodloc[__num - 0x31][1]);
			sprintf_P(__buff,PSTR("%d"),__pump_prod[__num - 0x31]);
			//_f_inttostr(__buff, __pump_prod[__num - 0x31]);
			lcd_print(__x+1, __y, __buff);
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

	lcd_clear();_delay_ms(10);
	lcd_printf(1, 1, PSTR("DATE : "));
	lcd_printf(2, 1, PSTR("TIME : "));
	lcd_print(1, 8, __date);
	lcd_print(2, 8, __time);
	lcd_printf(4, 1, PSTR("*)Cancel      #)Save"));

	__i = 0;
	lcd_xy(__map[__i][0], __map[__i][1]);
	CURSOR_SHOW;

	while(1){
		
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
			lcd_xy(__map[__i][0], __map[__i][1]);
LEWAT:		;
		}
		if(__key == _KEY_SHIFT){
			__i++;
			if(__i > 11)
				__i = 11;
			lcd_xy(__map[__i][0], __map[__i][1]);
		}
		if(__key == _KEY_CLEAR){
			__i--;
			if(__i <= 0)
				__i = 0;
			lcd_xy(__map[__i][0], __map[__i][1]);
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
	char 			__value[6];
	char	__i, __x, __y, __lmt, __start, __key, __num, __buff[6];
//	char lcdteks[20];

	__value[0] = eeprom_read_byte(&DefPrinterType);
	__value[1] = eeprom_read_byte(&DefPrintSize);
	__value[2] = eeprom_read_byte(&DefPrintLogo);
	__value[3] = eeprom_read_byte(&DefPrintAutoCut);
	__value[4] = eeprom_read_byte(&DefPrintScrollEnd);
	__value[5] = eeprom_read_byte(&DefPrintScrollSpace);

	lcd_clear();
	lcd_printf(1, 1, PSTR("1)Type:  5)Scroll:"));
	lcd_printf(2, 1, PSTR("2)Size:  6)Space :"));
	lcd_printf(3, 1, PSTR("3)Logo:  #)Save"));
	lcd_printf(4, 1, PSTR("4)Cut :  *)Exit"));

	for(__i = 0; __i < 6; __i++){
		__x = pgm_read_byte(&__prntloc[__i][0]);
		__y = pgm_read_byte(&__prntloc[__i][1]);
        sprintf_P(__buff,PSTR("%d"),__value[__i]);
		//_f_inttostr(__buff, __value[__i]);
		if(__i == 4) sprintf_P(__buff,PSTR("%.2d"),__value[__i]);
		//_f_punctuation(__buff, 0, 2, 0);
		lcd_print(__x, __y, __buff);
	}
    //sprintf_P(lcdteks,PSTR("6)Space :%.2d"),__value[5]);
	//lcd_print(2,10,lcdteks);

	while(1){	
		__key = _key_scan(1);
		__num = _key_btn(__key);
		if(__key == _KEY_CANCEL)
			return;
		if(__key==_KEY_ENTER){
			eeprom_write_byte(&DefPrinterType, __value[0]);
			eeprom_write_byte(&DefPrintSize, __value[1]);
			eeprom_write_byte(&DefPrintLogo, __value[2]);
			eeprom_write_byte(&DefPrintAutoCut, __value[3]);
			eeprom_write_byte(&DefPrintScrollEnd, __value[4]);
			eeprom_write_byte(&DefPrintScrollSpace, __value[5]);

		}
		if(__num >= 0x31 && __num <= 0x36){
			__lmt = pgm_read_byte(&__prntlmt[__num - 0x31]);
			__start = pgm_read_byte(&__prntstr[__num - 0x31]);
			if(__value[__num - 0x31] == __lmt)
				__value[__num - 0x31] = __start;
			else
				__value[__num - 0x31]++;
			__x = pgm_read_byte(&__prntloc[__num - 0x31][0]);
			__y = pgm_read_byte(&__prntloc[__num - 0x31][1]);
			sprintf_P(__buff,PSTR("%d"),__value[__num - 0x31]);
			//_f_inttostr(__buff, __value[__num - 0x31]);
			if((__num == 0x35)||(__num == 0x36))
			   sprintf_P(__buff,PSTR("%.2d"),__value[__num - 0x31]);
				//_f_punctuation(__buff, 0, 2, 0);

			lcd_print(__x, __y, __buff);
		}
	}
}

void _menu_host(void){
	char 			__value[4];
	unsigned char	__i, __x, __y, __key, __num, __buff[5];
	char lcdteks[20];
	char TermID;

	__value[0] = eeprom_read_byte(&DefPrintMoney);
	__value[1] = eeprom_read_byte(&DefShowDateTime);
	__value[2] = eeprom_read_byte(&DefNotifScreen);
	TermID= eeprom_read_byte(&DefIFT_ID);


	lcd_clear();_delay_ms(10);
	lcd_printf(1, 1, PSTR("1)Money :"));
	lcd_printf(2, 1, PSTR("2)D/T   :"));
	lcd_printf(3, 1, PSTR("3)Notif :     *)Exit"));
    sprintf_P(lcdteks,PSTR("4)TermID:%d "),TermID);
	lcd_print(4, 1,lcdteks);
	lcd_printf(4, 15, PSTR("#)Save"));

	for(__i = 0; __i < 3; __i++){
		__x = pgm_read_byte(&__hostloc[__i][0]);
		__y = pgm_read_byte(&__hostloc[__i][1]);
		sprintf_P(__buff,PSTR("%d"),__value[__i]);
		//_f_inttostr(__buff, __value[__i]);
		lcd_print(__x, __y, __buff);
	}

	while(1){
		__key = _key_scan(1);
		__num = _key_btn(__key);

		if(__key == _KEY_CANCEL)
			return;
		if(__key==_KEY_ENTER){
			eeprom_write_byte(&DefPrintMoney, __value[0]);
			eeprom_write_byte(&DefShowDateTime, __value[1]);
			eeprom_write_byte(&DefNotifScreen, __value[2]);
			eeprom_write_byte(&DefIFT_ID,TermID);
			break;
		}
		if(__num >= 0x31 && __num <= 0x33){
			if(__value[__num - 0x31] == 1)
				__value[__num - 0x31] = 0;
			else
				__value[__num - 0x31]++;
			__x = pgm_read_byte(&__hostloc[__num - 0x31][0]);
			__y = pgm_read_byte(&__hostloc[__num - 0x31][1]);
			sprintf_P(__buff,PSTR("%d"),__value[__num - 0x31]);
			//_f_inttostr(__buff, __value[__num - 0x31]);
			lcd_print(__x, __y, __buff);
		}else
		if (__num=='4'){
		    if (TermID<=99)TermID++;
			else TermID=1;
	     sprintf_P(lcdteks,PSTR("4)TermID:%d "),TermID);
		 lcd_print(4, 1,lcdteks);lcd_printf(4, 15, PSTR("#)Save"));
		 
		}

	}
}
void zeroIP(unsigned char Val,char *StrResult){
     unsigned char R,P,S;
	 if (Val>=100){
	    R=(Val/100);
        P=((Val%100)/10);
		S=Val-((R*100)+(P*10)); 
        StrResult[0]='0'+R;//+(Val/100);
	    StrResult[1]='0'+P;//+((Val%100)/10);
		StrResult[2]='0'+S;//+(Val-((Val/100)*100)-((Val%100)/10));
		StrResult[3]=0;
		}else     
	 if ((Val>=10)&&(Val<100)){
        StrResult[0]='0';
	    StrResult[1]=('0'+(Val/10));
		StrResult[2]=('0'+(Val%10));
		StrResult[3]=0;
		}
     else
	 if (Val<10){
	    StrResult[0]='0';
		StrResult[1]='0';
	    StrResult[2]='0'+Val;
	    StrResult[3]=0;
		}   
}

char FMenuTicket(){
static char stMenuTicket=mtInit;
static char iPos,iSend,FIP_Used,zFIP_Used;
static unsigned int iLoop=0;
     char uiResult=USER_NONE,KeyPressed,KeyChar;
     char Result=MENU_NONE,lcdteks[20];

    Result=MENU_NONE;   
    switch(stMenuTicket){
	case mtInit:
		 stMenuTicket=mtPlatNo;
	     break;
    case mtPlatNo:
	     lcd_clear();
	     lcd_printf(1,1,PSTR("Input Plat No: "));
	     lcd_printf(2,1,PSTR("_"));
	     lcd_printf(4,1,PSTR("[*]Cancel  [#]Enter "));
		 uiResult=USER_NONE;
		 stMenuTicket=mtInputPlatNo;
	     break;
    case mtInputPlatNo:
	     uiResult=UserInput(UI_ALPHANUM_R,2,1,strLicPlate,0,10);
		 if (uiResult==USER_OK)stMenuTicket=mtOdometer;
		 else
		 if (uiResult==USER_CANCEL)stMenuTicket=mtExitMenuTicket;
         else
		 if (uiResult==USER_ENTRY)lcd_printf(4,1,PSTR("[*]Back     [#]Enter"));
		 else
		 if (uiResult==USER_NO_DATA)lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
	     break;
    case mtOdometer:
	     lcd_clear();
		 sprintf_P(lcdteks,PSTR("Plat No: %s "),strLicPlate);
	     lcd_print(1,1,lcdteks);
	     lcd_printf(2,1,PSTR("Odometer:_ "));
	     lcd_printf(4,1,PSTR("[*]Cancel  [#]Enter "));
		 uiResult=USER_NONE;
		 stMenuTicket=mtInputOdometer;
	     break;
    case mtInputOdometer:
	     uiResult=UserInput(UI_NUMBER_R,2,10,strOdometer,0,10);
		 if (uiResult==USER_OK)stMenuTicket=mtFIP;
		 else
		 if (uiResult==USER_CANCEL)stMenuTicket=mtPlatNo;
		 else
		 if (uiResult==USER_ENTRY)lcd_printf(4,1,PSTR("[*]Back     [#]Enter"));
		 else
		 if (uiResult==USER_NO_DATA)lcd_printf(4,1,PSTR("[*]Cancel   [#]Enter"));
	     break;
    case mtFIP:
	     lcd_clear();
		 _scr_pump();
		 stMenuTicket=mtInputFIP;
	     break;
    case mtInputFIP:
	     KeyPressed=_key_scan(1);
		 KeyChar=_key_btn(KeyPressed);
		 if ((KeyChar>='1')&&(KeyChar<='8')){
		     FIP_Used=KeyChar-'0';
			 zFIP_Used=FIP_Used;
		     stMenuTicket=mtSendMsg98;
		 }
		 if (KeyPressed==_KEY_CANCEL){
             stMenuTicket=mtOdometer;
		 }else
		 if (KeyPressed==_KEY_ENTER){
		     FIP_Used=zFIP_Used;
             stMenuTicket=mtInitWaitMessage99;
		 }
	     break;	
    case mtInitWaitMessage99:
	     lcd_clear();
		 lcd_printf(2,1,PSTR("Send Request"));
		 lcd_printf(3,1,PSTR("Proses"));
         stMenuTicket=mtSendMsg98;
	     break;
    case mtSendMsg98:
	     iPos=0;
		 iSend=0;
		 IsMessage99=False;
		 TimSend=0;
		 iLoop=0;
		 if (IFType==IT_SLAVE)sendMessage98(FIP_Used);
		 else 
		 if (IFType==IT_STANDALONE)PrintStandalone(FIP_Used,False);
         stMenuTicket=mtExitMenuTicket;//mtInitWaitMessage99;
	     break;
    case mtWaitMessage99:
	     /*
		 iLoop++;
		 if ((iLoop%MSG_WAIT_TIMOUT)==0){
		     if (iPos<5){
		         lcd_put(3,(7+iPos),'.');
				 iPos++;
			 }else
		     if (iPos<5){
			     iPos=0;
			     lcd_printf(3,(7+iPos),PSTR("         "));
			     iSend++;
		         if (iSend>5){
				     stMenuTicket=mtNoConnection;
					 TimLocAcc=0;
					 system_beep(2);
				  }else {
				         iSend=0;TimSend=0;
	                     stMenuTicket=mtSendMsg98;
				  }
			 }
		 }
		 */
		 if (IsMessage99==True){ 
		    stMenuTicket=mtMessage99Received;
		 }
	     break;
    case mtMessage99Received:
	     stMenuTicket=mtExitMenuTicket;
		 break;
    case mtNoConnection:
	     lcd_clear();
		 lcd_printf(2,1,PSTR("Error No Connection"));
		 system_beep(2);
		 _delay_ms(2000);
         stMenuTicket=mtExitMenuTicket;
	     break;
    case mtExitMenuTicket:
	     Result=MENU_DONE;
	     stMenuTicket=mtInit;
	     break;
	}
  return Result;
}

void clearString(char *str){
     int i;
	 for(i=0;i<strlen(str);i++){
	    str[i]=0;
	 }
}



char GetLocAccStatus(char paramMessage57){
     char Result;
	 Result=LA_NONE;
     if (paramMessage57==MSG57_INVALID)Result=LA_INVALID;
	 else
     if (paramMessage57==MSG57_VALID)Result=LA_VALID;
	 else
     if (paramMessage57==MSG57_LIMITED)Result=LA_LIMITED;
	 return Result;
}
/*
void RemSpaceLag(char *Spaced){//Remove Space Character 1234SSSSS L=9
unsigned char i=0,Length=0,PosSpaced=0;
     Length=strlen(Spaced);
     for(i=0;i<Length;i++){
	     if ((Spaced[Length-i-1]==' ')&&(Spaced[Length-i]==' ')){
		      Spaced[Length-i-1]=0;
			  PosSpaced=Length-i-1;
			  }
         else break;
	 }
	 if (Spaced[PosSpaced]==' ')Spaced[PosSpaced]=0;
	 if (Spaced[0]==' ')Spaced[0]=0;
	 Spaced[strlen(Spaced)]=0;
}
*/
                                                      //012345678
void RemSpaceLag(char *Spaced){//Remove Space Character 1234SSSSS L=9 i=0 i=7
unsigned char i=0,Length=0,PosSpaced=0;
     Length=strlen(Spaced);
     for(i=0;i<(Length-1);i++){
	     if ((Spaced[Length-i-2]==' ')&&(Spaced[Length-i-1]==' ')){
		      Spaced[Length-i-1]=0;
			  PosSpaced=Length-i-2;
			  }
         else break;
	 }
	 if (Spaced[PosSpaced]==' ')Spaced[PosSpaced]=0;
	 if (Spaced[0]==' ')Spaced[0]=0;
	 Spaced[strlen(Spaced)]=0;
}

void RemSpaceLead(char *Zeroed){//Remove Space Character SSSSSS1234 12340234
unsigned char i=0,Length=0,ZeroPos=0,IsFound=False;;
     ZeroPos=0;Length=strlen(Zeroed);
	 if ((Length>1)&&(Zeroed[0]==' ')){
	     for(i=0;i<Length;i++){
		     if ((Zeroed[i]==' ')&&(Zeroed[i+1]==' ')&&(IsFound==False)) ZeroPos++;
	         if((Zeroed[i]==' ')&&(Zeroed[i+1]!=' '))IsFound=True;
		 }ZeroPos++;
	     for(i=0;i<strlen(Zeroed);i++){
		     Zeroed[i]=Zeroed[i+ZeroPos];
		 }
		 //Clearence
		 for(i=(Length-ZeroPos);i<Length;i++){
		     Zeroed[i]=0;
		 }
     }
}

void FTestRemZero(){
#ifdef DEBUG_FREM_ZERO_LEAD
	char strSend[50],strTest[20];

	     sprintf_P(strTest,PSTR("00012300"));
		 uart_print(0,1,strTest);
	     RemZeroLead(strTest);  
		 uart_print(0,1,strTest);

	     sprintf_P(strTest,PSTR("00,012300"));
		 uart_print(0,1,strTest);
	     RemZeroLead(strTest);  
		 uart_print(0,1,strTest);

	     sprintf_P(strTest,PSTR("00.012300"));
		 uart_print(0,1,strTest);
	     RemZeroLead(strTest);  
		 uart_print(0,1,strTest);


	     sprintf_P(strTest,PSTR("012300"));
		 uart_print(0,1,strTest);
	     RemZeroLead(strTest);  
		 uart_print(0,1,strTest);


	     sprintf_P(strTest,PSTR("0000"));
		 uart_print(0,1,strTest);
	     RemZeroLead(strTest);  
		 uart_print(0,1,strTest);


	     sprintf_P(strTest,PSTR("00"));
		 uart_print(0,1,strTest);
	     RemZeroLead(strTest);  
		 uart_print(0,1,strTest);

	     sprintf_P(strTest,PSTR("0"));
		 uart_print(0,1,strTest);
	     RemZeroLead(strTest);  
		 uart_print(0,1,strTest);

	while (1){};
  #endif
}

void RemZeroLead(char *Zeroed){//Remove Zero Character 00001004 000050000 0 
unsigned char i=0,Length=0,ZeroPos=0,IsFound=False;;
     IsFound=False;
     ZeroPos=0;Length=strlen(Zeroed);
  if ((Length>1)&&(Zeroed[0]=='0')){
	 if (Zeroed[0]=='0'){
         for(i=0;i<Length;i++){//00000000->0
	         if ((Zeroed[i]=='0')&&(Zeroed[i+1]=='0')&&(IsFound==False)) ZeroPos++;
             if ((Zeroed[i]=='0')&&(Zeroed[i+1]!='0')){
			     if ((Zeroed[i+1]=='.')||(Zeroed[i+1]==','))ZeroPos--;
		         IsFound=True;
			     break;			  
			     }
	      }

		  if (IsFound==True)ZeroPos++;
		  if (ZeroPos>=Length)ZeroPos=Length-1;
          for(i=0;i<(Length-ZeroPos);i++){
	          Zeroed[i]=Zeroed[i+ZeroPos];
		      Zeroed[i+1]=0;
	   }//Zeroed[Length-ZeroPos]=0;
	 }     
   }
}

              //Fmt:DATE_LONG_YEAR,DATE_SHORT_YEAR //0123456789    0123456789 
void FormatDate(char FmtYear, char *Date){         //2010/06/16 -> 16/06/2010
     char i,sDate[11];

     sDate[0]=Date[8];
	 sDate[1]=Date[9];
     sDate[2]=Date[7];
	 sDate[3]=Date[5];
     sDate[4]=Date[6];
	 sDate[5]=Date[4];
	 switch(FmtYear){
	 case DATE_LONG_YEAR:
	 	  for(i=0;i<4;i++)sDate[i+6]=Date[i];
          sDate[10]=0;
	      break;
	 case DATE_SHORT_YEAR:
	      for(i=0;i<2;i++)sDate[i+6]=Date[i+2];
          sDate[8]=0;
	      break;
	 }
	 //replace
	 for(i=0;i<strlen(sDate);i++)Date[i]=sDate[i];
}

void StrAlignCenter(char *Source, unsigned int Length){
char nAddedSpace,srcLength;
     RemSpaceLag(Source);
	 RemSpaceLead(Source);
	 srcLength=strlen(Source);
     nAddedSpace=(Length-srcLength)/2;
     AddSpaceLead(Source,srcLength+nAddedSpace);
	 AddSpaceLag(Source,Length);
}

void StrPosCopy(char *Source, char *Dest,unsigned int IdxSource, unsigned int Length){
unsigned int i;
     for(i=0;i<Length;i++){
	    Dest[i]=Source[IdxSource+i];
	 }Dest[Length]=0;
}
void StrPosPaste(char *Source, char *Dest,unsigned int IdxSource, unsigned int Length){
unsigned int i;
     for(i=0;i<Length;i++){
	    Dest[IdxSource+i]=Source[i];
	 }Dest[IdxSource+Length]=0;
}
char CharPosCopy(char *Source, unsigned int IdxSource){
     char Result;
     Result=Source[IdxSource];
	 return Result;
}

/*
MSG00_NACK,MSG00_ACK,MSG00_CARD_BLOCK,
                MSG00_NO_FIP,MSG00_NO_PRINT,MSG00_STATUS_QUERY,
				MSG00_NEW_PRICE,MSG00_SALE_INFO,MSG00_INVALID_PRODUCT,
				MSG00_SALES_PORT
*/
char procMessage00(){
     char Result,strSend[10];
	 Result=MSG00_NACK;
	 //Message57
	 if((rcv_trans[0]==0x01)&&(transLength==MSG00_LENGTH)){
	     //Reply
		 Result=(CharPosCopy(rcv_trans,37)-'0');
		 //sprintf(strSend,"L=%i",transLength);
		 //uart_print(0,1,strSend);
		 //sprintf(strSend,"R=%d",Result);
		 //uart_print(0,1,strSend);
		 //*/
		 }
     return Result;
}



char SaveToEEPROM(char *Src,char *Dest,unsigned int Length){
     char strCompare[40];
	 //Read Previous Data
     eeprom_read_block((void*)&Dest,(const void*)&strCompare, Length);
     //if 
     eeprom_write_block((const void*)&Src,(void*)&Dest, Length);
}

char procMessage09(){
     char Result;
     Result=(CharPosCopy(rcv_trans,39)-'0');
       StrPosCopy(rcv_trans,strFreeMessageLine1,39,20);
       StrPosCopy(rcv_trans,strFreeMessageLine2,59,20);
       StrPosCopy(rcv_trans,strFreeMessageLine3,79,20);
       StrPosCopy(rcv_trans,strFreeMessageLine4,99,20);
	 return Result;
}

char procMessage21(){
     char i,Result,strBankName[11];
	 Result=0;
	 for(i=0;i<4;i++){
         StrPosCopy(rcv_trans,strBankName,(37+(i*10)),10);
		 //Result=((Result<<1)|SaveToEEPROM(strBankName,DefBankName[i],11)); 
		 eeprom_write_block((const void*)&strBankName, (void*)&DefBankName[i], 11);
	 }
	 return Result;
}

char procMessage23(){
     char Result=0;
	 StrPosCopy(rcv_trans,strCardID,37,20);
     StrPosCopy(rcv_trans,strCardHolder,57,30);
     StrPosCopy(rcv_trans,strStatus,87,1);
     StrPosCopy(rcv_trans,strLoyCurrentPoints,88,8);
     StrPosCopy(rcv_trans,strLoyCurrMonConsumeA,96,10);
     StrPosCopy(rcv_trans,strLoyCurrMonConsumeV,106,10);
     StrPosCopy(rcv_trans,strDateTime,116,19);
     StrPosCopy(rcv_trans,strAmount,135,7);
     StrPosCopy(rcv_trans,strGainPoints,142,4);
     return Result;
}

char procMessage57(){
     char i,Result,serialSend[20];
	 Result=MSG57_NONE;
	 //Message57
	 
	     //Card Status
		 Result=(CharPosCopy(rcv_trans,57)-'0')+1;
		 //CardID
		 StrPosCopy(strCardID,rcv_trans,37,20);
		 RemSpaceLead(strCardID);
	     //Card Holder
		 StrPosCopy(rcv_trans,strCardHolder,60,40);
		 RemSpaceLag(strCardHolder);
		 //BalanceTypePrint
		 StrPosCopy(rcv_trans,strBalanceTypePrint,154-2,25);
		 RemSpaceLag(strBalanceTypePrint);
         //BalanceTypeCode
		 StrPosCopy(rcv_trans,strBalanceCode,177,1);
		 //Balance
		 StrPosCopy(rcv_trans,strBalance,178,13);
		 RemSpaceLead(strBalance);
		 //LicPlate
		 StrPosCopy(rcv_trans,strLicPlate,193-2,10);
		 RemSpaceLag(strLicPlate);
		 //CompName
		 StrPosCopy(rcv_trans,strCompName,203-2,20);
		 RemSpaceLag(strCompName);
         //Test

		 /*
		 uart_printf(0,1,PSTR("procMessage57()"));

         sprintf_P(serialSend,PSTR("Result:%d"),Result);
		 uart_print(0,1,serialSend);
		 
		 uart_print(0,1,strCardID);
		 uart_print(0,1,strCardHolder);
		 uart_print(0,1,strBalanceCode);
		 uart_print(0,1,strBalanceTypePrint);
		 uart_print(0,1,strBalance);
		 uart_print(0,1,strLicPlate);
		 uart_print(0,1,strCompName);
		 
uart_print(0,1,strCardID);
*/	
     return Result;
}

char procMessage81(){// Result: HFCS0000
     char Result=0,PrintCopy=0;//
	 char lcdteks[20];
	 //PrintHeader
     if ((CharPosCopy(rcv_trans,37))=='Y') Result=(Result|(1<<7));
     //PrintFooter
	 if ((CharPosCopy(rcv_trans,38))=='Y') Result=(Result|(1<<6));
	 //PaperCut
     if ((CharPosCopy(rcv_trans,39))=='Y') Result=(Result|(1<<5));
	 //Scrool
	 Result=(Result|(1<<4));

	 //Copies
	 PrintCopy=(CharPosCopy(rcv_trans,40)-'0');
     if (PrintCopy<=16) Result=Result|PrintCopy;
	 //Spooling
     StrPosCopy(rcv_trans,PrintBuffer,44,LengthMessage81);
	 //sprintf(lcdteks,"cmdPrint:%d ",Result);
     //uart_print(0,1,lcdteks);

	 //while(1){};
	 return Result;
}




void procMessage11(){
     unsigned int i;	 
     char buffHeader[50],strReadEEPROM[50];
	 char strProductName[13],strProductPrice[9],strTime[12],strDate[10];

     //Update Datetime
	 StrPosCopy(rcv_trans,strDate,43,10);//2004/09/14 19:05:36
	 FormatDate(DATE_SHORT_YEAR,strDate);
	 StrPosCopy(rcv_trans,strTime,54,8);
	 _datetime(_DATETIME_WRITE,strDate,strTime);
	      
	 //HeaderFooter
	 for(i=0;i<10;i++){
	     FillChar(buffHeader,0,sizeof(buffHeader));   
         StrPosCopy(rcv_trans,buffHeader,62+(i*40),40);
		 eeprom_read_block((void*) &strReadEEPROM, (const void*) &DefHeaderFooter[i],41);
	     if (strcmp(strReadEEPROM,buffHeader)!=0)
		     eeprom_write_block((const void*) &buffHeader, (void*) &DefHeaderFooter[i], 41);
	 }
     /*
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
	 */
	 //ProductName
	 for(i=0;i<6;i++){
	    StrPosCopy(rcv_trans,strProductName,(486+(i*12)),12);
		RemSpaceLag(strProductName);
		eeprom_read_block((void*) &strReadEEPROM, (const void*) &DefProductName[i],12);
		if (strcmp(strReadEEPROM,strProductName)!=0)
		    eeprom_write_block((const void*) &strProductName, (void*) &DefProductName[i], 12);
	 }
	 //ProductPrice
	 for(i=0;i<6;i++){
        StrPosCopy(rcv_trans,strProductPrice,(558+(i*8)),8);
		RemSpaceLag(strProductPrice);
		RemZeroLead(strProductPrice);
		RemDecimal(strProductPrice);

		eeprom_read_block((void*) &strReadEEPROM, (const void*) &DefProductPrice[i], 8);
		if (strcmp(strReadEEPROM,strProductPrice)!=0)
		    eeprom_write_block((const void*) &strProductPrice, (void*) &DefProductPrice[i], 8);
	 }
}



void RemDecimal(char *strDecimal){//4500.00-->4500
char i,DecPointMark;
     DecPointMark=eeprom_read_byte(&DefDecimalMark);	 

     for(i=0;i<strlen(strDecimal);i++){
	     if (strDecimal[i]==DecPointMark)
		     strDecimal[i]=0;
	 }
}

char SpaceOnly(char *string){
     char Result,nChar=0;
	 unsigned int i=0;
	 nChar=0;
	 Result=False;
	 for(i=0;i<strlen(string);i++){
	    if(string[i]!=' ') nChar++;
	 }
	 if (nChar>0) Result=False;
	 else Result=True;

	 return Result;
}


void PaperCut(){
     char CutType;
     CutType= eeprom_read_byte(&DefPrintAutoCut);
     uart(_COM_PRINTER, 1, 0x1B);
	 if(CutType== 1) { uart(_COM_PRINTER, 1, 0x6D);}
	 if(CutType== 2) {uart(_COM_PRINTER, 1, 0x69);}

}

void FillChar(char *strMemory, unsigned int Length,char data){
     unsigned int i;
	 for (i=0;i<Length;i++){
	     strMemory[i]=data;
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

void Tab(char *sTab, char nTab){
     char i;
	 for(i=0;i<nTab;i++){
	     sTab[i]=' ';
	 }sTab[nTab]=0;
}

//void SetPrinterCharacterWidth

void PrintDoubleHeight(){
char PrinterType;
     PrinterType=eeprom_read_byte(&DefPrinterType);
	 if (PrinterType==PT_CUSTOM_CUBE){
	     uart(_COM_PRINTER, 1, 0x1D);uart(_COM_PRINTER, 1, 0x21);uart(_COM_PRINTER, 1, 0x01);
		 }
}

void PrintNormalHeight(){
char PrinterType;
     PrinterType=eeprom_read_byte(&DefPrinterType);
	 if (PrinterType==PT_CUSTOM_CUBE){
	     uart(_COM_PRINTER, 1, 0x1D);uart(_COM_PRINTER, 1, 0x21);uart(_COM_PRINTER, 1, 0x00);
		 }
}

void InitPrinter(){
char PrinterType;
     PrinterType=eeprom_read_byte(&DefPrinterType);
    //Custom Printer TG02
	//Density Lowest: 1D 7C 00
	if (PrinterType==PT_CUSTOM_TG02){
		uart(_COM_PRINTER, 1, 0x1D);uart(_COM_PRINTER, 1, 0x7C);uart(_COM_PRINTER, 1, 0x00);
		//Double Strike ON : 1B 47 01
		uart(_COM_PRINTER, 1, 0x1B);uart(_COM_PRINTER, 1, 0x47);uart(_COM_PRINTER, 1, 0x01);
		//Font Setting: 1D 21 01
		uart(_COM_PRINTER, 1, 0x1D);uart(_COM_PRINTER, 1, 0x21);uart(_COM_PRINTER, 1, 0x01);   
		}
    else
	if (PrinterType==PT_CUSTOM_CUBE){
        //Print Density 0%
		uart(_COM_PRINTER, 1, 0x1D);uart(_COM_PRINTER, 1, 0x7C);uart(_COM_PRINTER, 1, 0x04);
		//Double Strike OFF : 1B 47 00
		uart(_COM_PRINTER, 1, 0x1B);uart(_COM_PRINTER, 1, 0x47);uart(_COM_PRINTER, 1, 0x00);
		//Font Setting: 1D 21 01
		uart(_COM_PRINTER, 1, 0x1D);uart(_COM_PRINTER, 1, 0x21);uart(_COM_PRINTER, 1, 0x00);   
		}
}

void PrintIdle(){
static char stPrintIdle=piIdle;

static char iHeader=0,iFooter=0,iScroll=0,nScroll=0;
static char iMessage=0,MessageLine=0,IsSignedField=False;
static unsigned int iSend=0,LSend=0,iLoop=0;
static char iPrinted=0,PrintCopy=0;
       char FIPAddr;
	   char strOperatorName[20];

#ifdef DEBUG_PRINT_IDLE_STATE
       char strSend[20];
	   static char zstPrintIdle=piIdle;
#endif
     //Monitoring
	 #ifdef DEBUG_PRINT_IDLE_STATE
     if(stPrintIdle!=zstPrintIdle){
	    zstPrintIdle=stPrintIdle;
		sprintf_P(strSend,PSTR("Pidle:%d"),stPrintIdle);
		uart_print(1,1,strSend);
	 }
	 #endif

     //Normalize PrintIdleState
     if ((IsPrinting==True)&&(stPrintIdle!=piIdle)){
	     stPrintIdle=piIdle;
	 }

     switch(stPrintIdle){
	 case piIdle:
	      if (IsPrinting==True){
		      IsPrinting=False;
			  IsBusyIdlePrinting=True;
			  IsBusyPrint=False;
			  iPrinted=0;
			  PrintCopy=0;
		      stPrintIdle=piInit;
			  InitPrinter();
			  IsSignedField=False;
			  }
	      break;
	 case piInit:
	      iHeader=0;
		  CarriegeReturn();
		  stPrintIdle=piLoadHeader;
	      break;
     case piLoadHeader:
	      eeprom_read_block((void*) &strPrint, (const void*) &DefHeaderFooter[iHeader], sizeof(DefHeaderFooter[iHeader]));
		  iHeader++;
		  if (iHeader<6){
		      if (SpaceOnly(strPrint)==True){
			      stPrintIdle=piLoadHeader;
				  }	      
			  else{
			      stPrintIdle=piPrintHeader;
			      iSend=0;
				  iLoop=0;
				  //PrintDoubleHeight on 1st Header
				  if (iHeader==1) PrintDoubleHeight();
				  else 
				  if (iHeader==2) PrintNormalHeight();
			   }
		  }else{stPrintIdle=piInitDuplicate; //stPrintIdle=piInitMessage;//
		        //else stPrintIdle=piInitMessage;
		  }
	      break;
	 case piPrintHeader:
		  if (iSend<40){
		      iLoop++;
			  if ((iLoop%PRINT_DELAY)==0){
				 TimPrintBusy=0;
				 stPrintIdle=piCheckPrintStatusHeader;
				 }
			  }
          else{ 
		     stPrintIdle=piLoadHeader;
			 CarriegeReturn();
		  }
	      break;
     case piCheckPrintStatusHeader:
	      if (IsBusyPrint==False){
		      uart(_COM_PRINTER,1,strPrint[iSend]);
		      iSend++;
		      stPrintIdle=piPrintHeader;
		  }
		  if (TimPrintBusy>TIM_BUSY_PRINT){
		      IsPrintERROR=True;
		      stPrintIdle=piFinishPrintIdle;
			  }
	      break;

     case piInitDuplicate:
          if ((iPrinted>0)||(IsReprintTicket==True)){
		      IsReprintTicket=False;
		      CarriegeReturn(); 
		      sprintf_P(strPrint,PSTR("             DUPLICATE COPY        "));
		      }
          else{           sprintf_P(strPrint,PSTR(" "));
		      }
		  iSend=0;
		  iLoop=0;
		  LSend=strlen(strPrint);
          stPrintIdle=piPrintDuplicate; 
	      break;
     case piPrintDuplicate:
		  if (iSend<LSend){
		      iLoop++;
			  if((iLoop%PRINT_DELAY)==0){
				 TimPrintBusy=0;
                 stPrintIdle=piCheckPrintStatusDuplicate;
				 }
			  }
          else {
		     iSend=0;
		     stPrintIdle=piInitMessage;
			 }
	      break;
     case piCheckPrintStatusDuplicate:
	      if (IsBusyPrint==False){
		      uart(_COM_PRINTER,1,strPrint[iSend]);
			  _delay_ms(10);
		      iSend++;
		      stPrintIdle=piPrintDuplicate;
		  }
		  if (TimPrintBusy>TIM_BUSY_PRINT){
		      IsPrintERROR=True;
		      stPrintIdle=piFinishPrintIdle;
			  }
	      break;

     case piInitMessage:
	      iSend=0;iLoop=0;
		  iMessage=0;
		  switch(MOPType){
		  case MOP_CASH:
		       MessageLine=10;
			   PrintCopy=1;
		       break;
          case MOP_CREDIT_CARD:
		       MessageLine=10;
			   PrintCopy=2;
		       break;
          case MOP_DEBIT_CARD:
		       MessageLine=20;
			   PrintCopy=2;
		       break;
          case MOP_LOCAL_ACCOUNT:
		       MessageLine=20;
			   PrintCopy=2;
		       break;
          case MOP_VOUCHER:
		       MessageLine=20;
			   PrintCopy=2;
		       break;
          case MOP_PUMP_TEST:
		       MessageLine=12;
			   PrintCopy=2;
		       break;
          case MOP_VOID_CARD:
		       MessageLine=10;
			   PrintCopy=1;
		       break;          
          default:
		       MessageLine=10;
			   PrintCopy=1;
		       break; 
		  }
		  //LoyaltyDetection
          if ((strlen(strLoyCardID)>0)&&(SpaceOnly(strLoyCardID)==False)){
		       MessageLine=33;
		  }
          //stPrintIdle=piLoadMessage;
		  stPrintIdle=piFormatingMessage;
	      break;
     case piFormatingMessage:
	      if (iPrinted==0){

		      FormatDate(DATE_LONG_YEAR,strDate);
              RemZeroLead(strIslandID);
			  RemZeroLead(strFIP_ID);
		      RemZeroLead(strPrice);              
              if (IFType==IT_SLAVE)
			      RemZeroLead(strVolume);
              RemZeroLead(strAmount);

			  RemDecimal(strPrice);
			  RemDecimal(strAmount);
	  
			  FormatCurrency(strPrice);
			  FormatCurrency(strAmount);
              if (IFType==IT_SLAVE){
		          RemSpaceLag(strCardID);
		          RemSpaceLag(strCardHolder);
		          RemSpaceLag(strMOPName);
		          RemSpaceLag(strLoyCardHolder);
		          RemSpaceLag(strCorporateID);
		          RemSpaceLag(strCorporateName);
		          RemSpaceLag(strLoyRedeemPoints);
		          RemSpaceLag(strLoyCurrMonConsumeV);
				  RemSpaceLag(strLoyCurrMonConsumeA);
		          FormatCurrency(strLoyCurrMonConsumeV);              
		          FormatCurrency(strLoyCurrMonConsumeA);
			   }
		  }
          stPrintIdle=piLoadMessage;
	      break;
     case piLoadMessage:
	      switch(iMessage){
		  case 0:
		       CarriegeReturn();
		       sprintf_P(strPrint,PSTR("      Shift: %s  No.Trans: %s "),strShift,strTranNo);
		       break;
		  case 1:
		       //FormatDate(strDate);
			   CarriegeReturn();
		       sprintf_P(strPrint,PSTR("      Waktu: %s %s "),strDate,strTime);
		       break;
		  case 2:
		       CarriegeReturn();
		       sprintf_P(strPrint,PSTR("    ------------------------------"));
		       break;
		  case 3:
		       CarriegeReturn();
			   //RemZeroLead(strIslandID);
			   //RemZeroLead(strFIP_ID);
		       sprintf_P(strPrint,PSTR("      Pulau/Pompa : [%s]-%s"),strIslandID,strFIP_ID);
		       break;
		  case 4:
		       CarriegeReturn();
			   sprintf_P(strPrint,PSTR("      Produk      : %s"),strDescription);
		       break;
		  case 5:
		       //RemZeroLead(strPrice);
               //FormatCurrency(strPrice);
               if (eeprom_read_byte(&DefPrintMoney)==True){
			   	   CarriegeReturn();
			       sprintf_P(strPrint,PSTR("      Harga/L     : Rp.%s"),strPrice);
			   }else sprintf_P(strPrint,PSTR(""));

		       break;
		  case 6:
		       //RemZeroLead(strVolume);
		       CarriegeReturn();
			   sprintf_P(strPrint,PSTR("      Jml Liter   : %s L"),strVolume);
		       break;
		  case 7:
		       //RemZeroLead(strAmount);		       
			   //FormatCurrency(strAmount);
               if (eeprom_read_byte(&DefPrintMoney)==True){
			   	   CarriegeReturn();
			       sprintf_P(strPrint,PSTR("      Jml Rupiah  : Rp.%s"),strAmount);
			   }else sprintf_P(strPrint,PSTR(""));

		       break;
		  case 8:
		       if ((strlen(strLicPlate)>0)&&(SpaceOnly(strLicPlate)==False)){
			       CarriegeReturn();
			       sprintf_P(strPrint,PSTR("      No.Polisi   : %s"),strLicPlate);
                   ClearMem(strLicPlate);
				   }
               else ClearMem(strPrint);
		       break;
		  case 9:
		       if ((strlen(strOdometer)>0)&&(SpaceOnly(strOdometer)==False)){
			       CarriegeReturn();
		           sprintf_P(strPrint,PSTR("      Odometer    : %s"),strOdometer);
				   ClearMem(strOdometer);
			   }else ClearMem(strPrint);
		       break;
		  case 10:
		       CarriegeReturn();
		       sprintf_P(strPrint,PSTR("    ------------------------------"));
		       break;
          //ChangeMOPPrint
		  case 11:
			   if ((MOPType==MOP_LOCAL_ACCOUNT)||(MOPType==MOP_DEBIT_CARD)||(MOPType==MOP_LOYALTY_LOCAL_ACCOUNT)){
			       //RemSpaceLag(strCardID);
				   CarriegeReturn();
			       sprintf_P(strPrint,PSTR("    Kartu ID : %s"),strCardID);
				   IsSignedField=True;
				   }
	           else
			   if (MOPType==MOP_VOUCHER){
			       //RemSpaceLag(strCardID);
				   CarriegeReturn();
			       sprintf_P(strPrint,PSTR("    Voucher No: %s"),strVoucherNum);
				   IsSignedField=True;
				   }
			   else
			   if (MOPType==MOP_PUMP_TEST) {
			       CarriegeReturn();
			       sprintf_P(strPrint,PSTR("          *** PUMP TEST ***       "));
				   IsSignedField=False;
				   }
               else ClearMem(strPrint);
		       break;
          case 12:
			   if (MOPType==MOP_LOCAL_ACCOUNT) {
			       //RemSpaceLag(strCardHolder);
				   CarriegeReturn();
			       sprintf_P(strPrint,PSTR("    Nama     : %s"),strCardHolder);
				   }
               else 
               if (MOPType==MOP_DEBIT_CARD) {//EDCApprovalCode
			       if (IsPrintApprovalCode==True){
				       IsPrintApprovalCode=False;
					   FillChar(strCardHolder,0,sizeof(strCardHolder));
					   sprintf_P(strCardHolder,PSTR("%s  "),strApprovalCode);
				   }
			       //RemSpaceLag(strCardHolder);
				   CarriegeReturn();
			       sprintf_P(strPrint,PSTR("    Appr Code: %s"),strCardHolder);
				   }
               else
			   if (MOPType==MOP_VOUCHER){
			       CarriegeReturn();
			       sprintf_P(strPrint,PSTR("  "));
				   }
			   else
			   if (MOPType==MOP_PUMP_TEST) {
			       CarriegeReturn();
			       sprintf_P(strPrint,PSTR("                 "));
				   }
               else ClearMem(strPrint);
		       break;
          case 13:
		       if (IsSignedField==True){
			       CarriegeReturn();
			       sprintf_P(strPrint,PSTR("                      "));
               }else ClearMem(strPrint);
		       break;
          case 14:
		       if (IsSignedField==True){
			       CarriegeReturn();
			       sprintf_P(strPrint,PSTR("                      "));
               }else ClearMem(strPrint);
		       break;
          case 15:
		       if (IsSignedField==True){
			       //CarriegeReturn();
			       //sprintf_P(strPrint,PSTR("                      "));
				   ClearMem(strPrint);
               }else ClearMem(strPrint);
		       break;
          case 16:
		       if (IsSignedField==True){
			       //CarriegeReturn();
			       //sprintf_P(strPrint,PSTR("                      "));
				   ClearMem(strPrint);
               }else ClearMem(strPrint);
		       break;
          case 17:
		       if (IsSignedField==True){
			       CarriegeReturn();
			       sprintf_P(strPrint,PSTR("    Tanda Tangan  (______________)"));
               }else ClearMem(strPrint);
		       break;
          case 18:
		       if (IsSignedField==True){
			       IsSignedField=False;
			       //CarriegeReturn();
			       //sprintf_P(strPrint,PSTR("                      "));
				   ClearMem(strPrint);
               }else ClearMem(strPrint);
		       break;
          case 19://PrintMOP Name
		       if (MOPType!=MOP_CASH){
			       CarriegeReturn();
			       //RemSpaceLag(strMOPName);
			       sprintf_P(strPrint,PSTR("    .%s"),strMOPName);
			   }else ClearMem(strPrint);
		       break;
		  case 20:
		       if (MOPType!=MOP_CASH){
			       CarriegeReturn();
		           sprintf_P(strPrint,PSTR("    ------------------------------"));
			   }else ClearMem(strPrint);
		       break;          
          //Loyalty
		  case 21:
		       CarriegeReturn();
		       sprintf_P(strPrint,PSTR("         LOYALTY INFORMATION      "));
		       break; 			            
		  case 22:
		       CarriegeReturn();
		       sprintf_P(strPrint,PSTR("     Card ID      : %s"),strLoyCardID);
		       break;
		  case 23:
		       CarriegeReturn();
			   //RemSpaceLag(strLoyCardHolder);
		       sprintf_P(strPrint,PSTR("     Card Holder  : %s "),strLoyCardHolder);
		       break;
		  case 24:
		       if ((strlen(strCorporateID)>0)&&(SpaceOnly(strCorporateID)==False)){
		           CarriegeReturn();
				   //RemSpaceLag(strCorporateID);
		           sprintf_P(strPrint,PSTR("     Corp ID      : %s"),strCorporateID);
			   }else ClearMem(strPrint);
		       break;
		  case 25:
		       if ((strlen(strCorporateName)>0)&&(SpaceOnly(strCorporateName)==False)){
		           CarriegeReturn();
				   //RemSpaceLag(strCorporateName);
		           sprintf_P(strPrint,PSTR("     Corp Name    : %s"),strCorporateName);
			   }else ClearMem(strPrint);
		       break;
		  case 26:
		       CarriegeReturn();
		       sprintf_P(strPrint,PSTR("     Gain Points  : %s"),strGainPoints);
		       break;
		  case 27:
		       CarriegeReturn();
		       sprintf_P(strPrint,PSTR("     Prev Points  : %s"),strPrevPoints);
		       break;
		  case 28:
		       CarriegeReturn();
		       sprintf_P(strPrint,PSTR("     Curr Points  : %s"),strLoyCurrentPoints);
		       break;
		  case 29:
		       CarriegeReturn();
		       sprintf_P(strPrint,PSTR("     Expiry       : %s"),strLoyExpiry);
		       break;
		  case 30:
		       CarriegeReturn();
			   //RemSpaceLag(strLoyRedeemPoints);
		       sprintf_P(strPrint,PSTR("     Total Redeem : %s"),strLoyRedeemPoints);
		       break;
		  case 31:
		       CarriegeReturn();
			   //RemSpaceLag(strLoyCurrMonConsumeV);
			   //FormatCurrency(strLoyCurrMonConsumeV);
		       sprintf_P(strPrint,PSTR("     Month Cons V : %s L"),strLoyCurrMonConsumeV);
		       break;
		  case 32:
		       CarriegeReturn();
			   //RemSpaceLag(strLoyCurrMonConsumeA);
			   //FormatCurrency(strLoyCurrMonConsumeA);
		       sprintf_P(strPrint,PSTR("     Month Cons A : Rp.%s"),strLoyCurrMonConsumeA);
		       break;
		  case 33://EndOfLoyalty
		       CarriegeReturn();
		       sprintf_P(strPrint,PSTR("    ------------------------------"));
		       break;
		  case 34:
		       CarriegeReturn();
		       sprintf_P(strPrint,PSTR("     Surcharge Dsc: %s"),strSurchargeDesc);
		       break;
		  case 35:
		       CarriegeReturn();
		       sprintf_P(strPrint,PSTR("     Surcharge Amt: %s"),strSurchargeAmount);
		       break;
		  case 36:
		       CarriegeReturn();
		       sprintf_P(strPrint,PSTR("    ------------------------------"));
		       break;
		  }
		  iLoop=0;iSend=0;LSend=strlen(strPrint);
          stPrintIdle=piPrintMessage;
	      break;
     case piPrintMessage:
	      if (iSend<LSend){
		      iLoop++;
			  if((iLoop%PRINT_DELAY)==0){
				 TimPrintBusy=0;
				 stPrintIdle=piCheckPrintStatusMessage;
				 }
			  }
          else{iMessage++;
		      if (iMessage>MessageLine){
			      iFooter=0;				  
				  CarriegeReturn();
				  stPrintIdle=piLoadOperatorName;//piLoadFooter;
			  }
			  else stPrintIdle=piLoadMessage;
			  }
	      break;
     case piCheckPrintStatusMessage:
	      if (IsBusyPrint==False){
		      uart(_COM_PRINTER,1,strPrint[iSend]);
		      iSend++;
		      stPrintIdle=piPrintMessage;
		  }
		  if (TimPrintBusy>TIM_BUSY_PRINT){
		      IsPrintERROR=True;
		      stPrintIdle=piFinishPrintIdle;
			  }
	      break;


	//Added Operator Name:
	 case piLoadOperatorName:
	      //CheckEmpty
		  eeprom_read_block((void*) &strOperatorName, (const void*) &DefOperatorName,18);
		  StrPosCopy(strOperatorName,strOperatorName,0,15);
		  if (strlen(strOperatorName)>0){
			  if (SpaceOnly(strOperatorName)!=True){		      				  
				  sprintf_P(strPrint,PSTR("     Operator: %s"),strOperatorName);			   
			      iSend=0;iLoop=0;
				  LSend=strlen(strPrint);
				  stPrintIdle=piPrintOperatorName;
			  }else stPrintIdle=piLoadFooter;
          }  
          else stPrintIdle=piLoadFooter;
	      break;
	 case piPrintOperatorName:
		  if (iSend<LSend){
		      iLoop++;
			  if((iLoop%PRINT_DELAY)==0){
				  TimPrintBusy=0;
				  stPrintIdle=piCheckPrintOperatorName;
				}
			  }
          else {
			  CarriegeReturn();
			  CarriegeReturn();
			  stPrintIdle=piLoadFooter;
			  }
	      break;
	 case piCheckPrintOperatorName:
	      if (IsBusyPrint==False){
		      uart(_COM_PRINTER,1,strPrint[iSend]);
		      iSend++;
		      stPrintIdle=piPrintOperatorName;
		  }
		  if (TimPrintBusy>TIM_BUSY_PRINT){
		      IsPrintERROR=True;
		      stPrintIdle=piFinishPrintIdle;
			  }
	      break;

	 	       
     case piLoadFooter:
	      FillChar(strPrint,0,sizeof(strPrint));
	      eeprom_read_block((void*) &strPrint, (const void*) &DefHeaderFooter[6+iFooter], 40);
		  iFooter++;
		  if (iFooter<=4){
		      if (SpaceOnly(strPrint)==True){
			      stPrintIdle=piLoadFooter;
				  }
			  else{
			      iSend=0;
				  iLoop=0;
				  stPrintIdle=piPrintFooter;
				  //RemSpaceLag(strPrint);
				  //CarriegeReturn();
			      }
		  }else{stPrintIdle=piInitScroll;
		        //iPrinted++;
			}
	      break;
     case piPrintFooter:
		  if (iSend<40){
		      iLoop++;
			  if((iLoop%PRINT_DELAY)==0){
				TimPrintBusy=0;
				stPrintIdle=piCheckPrintStatusFooter;
				}
			  }
          else {
		      stPrintIdle=piLoadFooter;
			  CarriegeReturn();
			  }
	      break;
     case piCheckPrintStatusFooter:
	      if (IsBusyPrint==False){
		      uart(_COM_PRINTER,1,strPrint[iSend]);
		      iSend++;
		      stPrintIdle=piPrintFooter;
		  }
		  if (TimPrintBusy>TIM_BUSY_PRINT){
		      IsPrintERROR=True;
		      stPrintIdle=piFinishPrintIdle;
			  }
	      break;

     case piInitScroll:
	      iPrinted++;
		  iScroll=0;
		  iLoop=0;
		  if (iPrinted<PrintCopy){
		      nScroll=eeprom_read_byte(&DefPrintScrollSpace);
		  }else if (iPrinted>=PrintCopy){		  	  
		      nScroll=eeprom_read_byte(&DefPrintScrollEnd);
		  }
		  /*
	      //if ((PrintCopy==1)||(iPrinted>=PrintCopy)&&(eeprom_read_byte(&DefPrinterType)==PT_CUSTOM_CUBE)){
		  if ((eeprom_read_byte(&DefPrinterType)==PT_CUSTOM_CUBE)||(iPrinted>=PrintCopy)){
		  	  iScroll=0;
			  iLoop=0;
		      nScroll=eeprom_read_byte(&DefPrintScrollEnd);
		      stPrintIdle=piScrollPaper;              
			  IsBusyPrint=False;
			  uart_printf(0,1,PSTR("----Scrolled1----"));
			 }
		  else //if ((eeprom_read_byte(&DefPrinterType)==PT_CUSTOM_TG02)&&(iPrinted<PrintCopy)){
		  if (eeprom_read_byte(&DefPrinterType)==PT_CUSTOM_TG02){
		      uart_printf(0,1,PSTR("----Scrolled2-----"));

		      stPrintIdle=piPaperCut;
		  }*/
          stPrintIdle=piScrollPaper;              
	      break;
     case piScrollPaper:
	      iLoop++;
		  if (iLoop%PRINT_DELAY==0){
			  TimPrintBusy=0;
			  stPrintIdle=piCheckPrintStatusScroll;
			  }
	      if (iScroll>nScroll)stPrintIdle=piPaperCut;
	      break;
     case piCheckPrintStatusScroll:
	      if (IsBusyPrint==False){
		      iScroll++;
		      CarriegeReturn();
		      stPrintIdle=piScrollPaper;
		  }
		  if (TimPrintBusy>TIM_BUSY_PRINT){
		      IsPrintERROR=True;
		      stPrintIdle=piFinishPrintIdle;
			  }
	      break;
     case piPaperCut:
	      //sprintf_P(strSend,PSTR("i:%d Copy:%d"),iPrinted,PrintCopy);
		  //uart_print(0,1,strSend);

          if (iPrinted>=PrintCopy){
		      stPrintIdle=piFinishPrintIdle;
			  }
		  else {CarriegeReturn();
		        CarriegeReturn();
		        stPrintIdle=piInit;
		       }
		  PaperCut();
	      break;
     case piFinishPrintIdle:	      
	      switch(IFType){
		  case IT_SLAVE:
		       sendMessage04();
		       break;
		  case IT_STANDALONE:		  
		       UpdateStandaloneStatus((atoi(strFIP_ID)&0x0F),PS_PRINTED);
			   //FIPAddr=GetFIPAddr(atoi(strFIP_ID)&0x0F)-1;
			   //CurrentPumpStatus[FIPAddr]=PUMP_OFF;
			   //strPumpStatus[FIPAddr]=GetPumpStatusLabel(PUMP_OFF);			   
			   //IsNewPumpStatus=True;//UpdateDisplay
		       break;
		  }
		  IsBusyIdlePrinting=False;
          stPrintIdle=piIdle;
	      break;	 
	 }
}

void CarriegeReturn(){
     uart(_COM_PRINTER, 1, 0x0D);     
     uart(_COM_PRINTER, 1, 0x0A);
}

void PstrCopy(char *Dest,char *Source){
     char i=0;
	 for(i=0;i<strlen(Source);i++){
	    Dest[i]=pgm_read_byte(&Source[i]);
	 }
}

void SendPrint(char xSend,char xSendLead){
static char zSend;     
	 if (xSend==0x19){
	     if (zSend==xSendLead)uart(_COM_PRINTER,1,xSendLead);
		 else uart(_COM_PRINTER,1,' ');	 
	 } 
	 else uart(_COM_PRINTER,1,xSend);
	 zSend=xSend;
}

void FreePrinting(){
static char stFreePrinting=fpInit;
static char iPrinted=0,iHeader=0,iFooter=0,PrintCopy=0,iMargin=0,iScroll=0,nScroll=0;
static unsigned int iSend=0,LSend=0,iLoop=0;
       char strOperatorName[20];

     //Normalize FreePrintIdleState
     if ((IsFreePrinting==True)&&(stFreePrinting!=fpInit)){
	     stFreePrinting=fpInit;
	 }
	 switch (stFreePrinting){
     case fpInit:
	      //uart_printf(0,1,PSTR("fpInit"));
	      if (IsFreePrinting==True){
		      IsFreePrinting=False;
			  IsBusyFreePrinting=True;
			  IsBusyPrint=False;
			  iPrinted=0,iFooter=0;

			  PrintCopy=(cmdPrint&0x0F);

			  //cmdPrint=cmdPrint|0b00100000;
			  //PrintCopy=2;

			  stFreePrinting=fpInitHeader;
			  InitPrinter();
			  }
	      break;
     case fpInitHeader:
	      //uart_printf(0,1,PSTR("fpInitHeader"));
		  iHeader=0;
	      if (((cmdPrint&0b10000000)>>7)==1){
		      stFreePrinting=fpLoadHeader;
			  }
		  else
	      if (((cmdPrint&0b10000000)>>7)==0) stFreePrinting=fpInitMessage;	      
	      break;
     case fpLoadHeader:
	      //uart_printf(0,1,PSTR("fpLoadHeader"));
          //Header 1
	      eeprom_read_block((void*) &strPrint, (const void*) &DefHeaderFooter[iHeader], 40);
		  iHeader++;
		  if (iHeader<6){
		      if (SpaceOnly(strPrint)==True){
			      stFreePrinting=fpLoadHeader;
				  }	      
			  else{
			      stFreePrinting=fpPrintHeader;
			      iSend=0;
				  iLoop=0;
				  //PrintDoubleHeight on 1st Header
				  if (iHeader==1) PrintDoubleHeight();
				  else 
				  if (iHeader==2) PrintNormalHeight();
			   }
		  }else 
		  {stFreePrinting=fpInitMessage;
		  }
	      break;
     case fpPrintHeader:
	      //uart_printf(0,1,PSTR("fpPrintHeader"));
		  if (iSend<40){
		      iLoop++;
			  if ((iLoop%PRINT_DELAY)==0){
			     //uart(_COM_PRINTER,1,strPrint[iSend]);
		         //iSend++;
				 TimPrintBusy=0;
				 stFreePrinting=fpCheckPrintStatusHeader;
				 }
			  }
          else{ 
		     stFreePrinting=fpLoadHeader;
			 CarriegeReturn();
		  }
	      break;

     case fpCheckPrintStatusHeader:
	      if (IsBusyPrint==False){
		      uart(_COM_PRINTER,1,strPrint[iSend]);
		      iSend++;
		      stFreePrinting=fpPrintHeader;
		  }
		  if (TimPrintBusy>TIM_BUSY_PRINT){
		      IsPrintERROR=True;
		      stFreePrinting=fpFinishFreePrinting;
			  }
	      break;
     case fpInitMessage:
	      //uart_printf(0,1,PSTR("fpInitMessage"));
	      iSend=0;iLoop=0;
          stFreePrinting=fpPrintMessage; 
		  CarriegeReturn();
		  if (iPrinted>0) stFreePrinting=fpInitDuplicate;
		  else stFreePrinting=fpPrintMessage; 
	      break;
     case fpInitDuplicate:
	      //uart_printf(0,1,PSTR("fpInitDuplicate"));
          if (iPrinted>0)sprintf_P(strPrint,PSTR("             DUPLICATE COPY        "));
          else           sprintf_P(strPrint,PSTR("                                   "));
		  iSend=0;
		  iLoop=0;
		  LSend=strlen(strPrint);
          stFreePrinting=fpPrintDuplicate; 
	      break;
     case fpPrintDuplicate:
	      //uart_printf(0,1,PSTR("fpPrintDuplicate"));
		  if (iSend<LSend){
		      iLoop++;
			  if((iLoop%PRINT_DELAY)==0){
			     //uart(_COM_PRINTER,1,strPrint[iSend]);
				 //iSend++;
				 TimPrintBusy=0;
                 stFreePrinting=fpCheckPrintStatusDuplicate;
				 }
			  }
          else {
		     iSend=0;
		     stFreePrinting=fpPrintMessage;
			 CarriegeReturn();	      
			 }
	      break;
     case fpCheckPrintStatusDuplicate:
	      if (IsBusyPrint==False){
		      uart(_COM_PRINTER,1,strPrint[iSend]);
		      iSend++;
		      stFreePrinting=fpPrintDuplicate;
		  }
		  if (TimPrintBusy>TIM_BUSY_PRINT){
		      IsPrintERROR=True;
		      stFreePrinting=fpFinishFreePrinting;
			  }
	      break;

     case fpPrintMessage:
		  if (iSend<LengthMessage81){
		      iLoop++;
			  if((iLoop%PRINT_DELAY)==0){
			     TimPrintBusy=0;
				 stFreePrinting=fpCheckPrintStatusMessage;
				 }
			  }
          else stFreePrinting=fpLoadEndLine;//fpInitFooter;
	      break;
     
     case fpCheckPrintStatusMessage:
	      if (IsBusyPrint==False){
              if ((PrintBuffer[iSend]!=0x0D)||(PrintBuffer[iSend]!=0x0A))
			      SendPrint(PrintBuffer[iSend],PrintBuffer[iSend+1]);
			     // uart(_COM_PRINTER,1,PrintBuffer[iSend]);
              if ((PrintBuffer[iSend]==0x0D)||(PrintBuffer[iSend+1]==0x0A))
			     CarriegeReturn();
             iSend++;
		     stFreePrinting=fpPrintMessage;
		  }
		  if (TimPrintBusy>TIM_BUSY_PRINT){
		      IsPrintERROR=True;
		      stFreePrinting=fpFinishFreePrinting;
			  }
	      break;
     //SpaceAdded
	 case fpInitSpace:
	      iLoop=0;
		  iMargin=0;
		  stFreePrinting=fpPrintMargin;
	      break;
	 case fpPrintMargin:
		  if (iMargin<PRINT_MARGIN){
		      iMargin++;
		      iLoop++;
	          if ((iLoop%PRINT_DELAY)==0)uart(_COM_PRINTER,1,' ');
			 }
          else stFreePrinting=fpPrintMessage;
	      break;
     
     //Added Operator Name:
	 case fpLoadEndLine:
		  eeprom_read_block((void*) &strOperatorName, (const void*) &DefOperatorName,18);
          StrPosCopy(strOperatorName,strOperatorName,0,15);
          if (strlen(strOperatorName)>0){
		  	  if (SpaceOnly(strOperatorName)!=True){
			      sprintf_P(strPrint,PSTR("---------------------------------"));
			      //AddSpaceLead(strPrint,(strlen(strPrint)+PRINT_MARGIN));
			      iSend=0;
				  iLoop=0;
				  LSend=strlen(strPrint);
				  stFreePrinting=fpPrintEndLine;	
			   }else stFreePrinting=fpInitFooter;
		  }else stFreePrinting=fpInitFooter;
	      break;

	 case fpPrintEndLine:
		  if (iSend<LSend){
		      iLoop++;
			  if((iLoop%PRINT_DELAY)==0){
				  TimPrintBusy=0;
				  stFreePrinting=fpCheckPrintEndLine;
				}
			  }
          else {
			  CarriegeReturn();
			  stFreePrinting=fpLoadOperatorName;
			  }
	      break;
	 case fpCheckPrintEndLine:
	      if (IsBusyPrint==False){
		      uart(_COM_PRINTER,1,strPrint[iSend]);
		      iSend++;
		      stFreePrinting=fpPrintEndLine;
		  }
		  if (TimPrintBusy>TIM_BUSY_PRINT){
		      IsPrintERROR=True;
		      stFreePrinting=fpFinishFreePrinting;
			  }
	      break;

	 case fpLoadOperatorName:
		  eeprom_read_block((void*) &strOperatorName, (const void*) &DefOperatorName,18);
		  StrPosCopy(strOperatorName,strOperatorName,0,15);
		  sprintf_P(strPrint,PSTR("Operator: %s"),strOperatorName);
	      iSend=0;
		  iLoop=0;
		  LSend=strlen(strPrint);
		  stFreePrinting=fpPrintOperatorName;	
	      break;
	 case fpPrintOperatorName:
		  if (iSend<LSend){
		      iLoop++;
			  if((iLoop%PRINT_DELAY)==0){
				  TimPrintBusy=0;
				  stFreePrinting=fpCheckPrintOperatorName;
				}
			  }
          else {
			  CarriegeReturn();
			  CarriegeReturn();
			  stFreePrinting=fpInitFooter;
			  }
	      break;
	 case fpCheckPrintOperatorName:
	      if (IsBusyPrint==False){
		      uart(_COM_PRINTER,1,strPrint[iSend]);
		      iSend++;
		      stFreePrinting=fpPrintOperatorName;
		  }
		  if (TimPrintBusy>TIM_BUSY_PRINT){
		      IsPrintERROR=True;
		      stFreePrinting=fpFinishFreePrinting;
			  }
	      break;

     case fpInitFooter:
	      //uart_printf(0,1,PSTR("fpInitFooter"));
          if (((cmdPrint&0b01000000)>>6)==1){
		      stFreePrinting=fpLoadFooter;
			  iFooter=0;
			  }
		  else
	      if (((cmdPrint&0b01000000)>>6)==0) stFreePrinting=fpPaperCut;//fpInitScroll;	                
	      break;
     case fpLoadFooter:
	 	  //uart_printf(0,1,PSTR("fpLoadFooter"));
          //Footer 1
	      eeprom_read_block((void*) &strPrint, (const void*) &DefHeaderFooter[6+iFooter], 40);
		  iFooter++;
		  if (iFooter<4){
		      if (SpaceOnly(strPrint)==True){
			      stFreePrinting=fpLoadFooter;
				  }
			  else{
			      iSend=0;
				  iLoop=0;
				  stFreePrinting=fpPrintFooter;
			      }
		  }else{
		    CarriegeReturn();
			iPrinted++;

			//if (eeprom_read_byte(&DefPrinterType)==PT_CUSTOM_CUBE){
			    stFreePrinting=fpInitScroll;
			//}else stFreePrinting=fpPaperCut;

			}
	      break;
     case fpPrintFooter:
		  if (iSend<40){
		      iLoop++;
			  if((iLoop%PRINT_DELAY)==0){
		        //uart(_COM_PRINTER,1,strPrint[iSend]);
		        //iSend++;
				TimPrintBusy=0;
				stFreePrinting=fpCheckPrintStatusFooter;
				}
			  }
          else {
		      stFreePrinting=fpLoadFooter;
			  CarriegeReturn();
			  }
	      break;
     
	 case fpCheckPrintStatusFooter:
	      if (IsBusyPrint==False){
		      uart(_COM_PRINTER,1,strPrint[iSend]);
		      iSend++;
		      stFreePrinting=fpPrintFooter;
		  }
		  if (TimPrintBusy>TIM_BUSY_PRINT){
		      IsPrintERROR=True;
		      stFreePrinting=fpFinishFreePrinting;
			  }
	      break;

     case fpInitScroll:
	      iScroll=0;
		  iSend=0;
		  if (iPrinted<PrintCopy)
		      nScroll=eeprom_read_byte(&DefPrintScrollSpace);
		  else if (iPrinted>=PrintCopy)
		  	  nScroll=eeprom_read_byte(&DefPrintScrollEnd);

          stFreePrinting=fpScrollPaper;
	      break;
     case fpScrollPaper:
	      if (iScroll>nScroll)stFreePrinting=fpPaperCut;
		  iSend++;
		  if (iSend%PRINT_DELAY==0){
		      CarriegeReturn();
			  iScroll++;
			  }
	      break;
     case fpPaperCut:
		  //uart_printf(0,1,PSTR("fpPaperCut"));
		  //iPrinted++;
          if (iPrinted<PrintCopy){
		      CarriegeReturn();
              stFreePrinting=fpInitHeader;
			  }
          else{
		      if (IFType==IT_SLAVE)sendMessage04();
		      stFreePrinting=fpFinishFreePrinting;
			  }
          
		  if (((cmdPrint&0b00100000)>>5)==1){
		      if (iPrinted==0){
			      CarriegeReturn();_delay_ms(50);
				 }
              //HFCS 0000 : Header, Footer, Copy , Scrool [Copy:16x max]
		      if ((iPrinted==PrintCopy)){
			      // if (((cmdPrint&0b00010000)>>4)==1){
				  CarriegeReturn();_delay_ms(50);
				  CarriegeReturn();_delay_ms(50);
				  CarriegeReturn();_delay_ms(50);
				  CarriegeReturn();_delay_ms(50);
				   //}
				 }
		      PaperCut();
		   }
	      break;
     case fpFinishFreePrinting:
	 	  //uart_printf(0,1,PSTR("fpFinished"));
		  IsBusyFreePrinting=False;
	      stFreePrinting=fpInit;
		  iPrinted=0;
	      break;
	 }//EndCase
}

//-------------------Totalizer-----------------------------------------
//TAddr{TOTALIZER_LAST,TOTALIZER_NOW}
//TType:{TNONE,TVOLUME,TMONEY};
void ResetTotalizer(char TAddr){//Clear All TotalizerData;
     char iPump,iGrade;
     char strValue[9];
	 char FIPAddr;
	 
	 sprintf_P(strValue,PSTR("00000000"));
	 for (iPump=1;iPump<=16;iPump++){//Pump
	      FIPAddr=GetFIPAddr(iPump);
		  if (FIPAddr>0){
		      FIPAddr=FIPAddr-1;
			  for(iGrade=1;iGrade<=6;iGrade++){
				  SetTotalizerData(TVOLUME,TAddr,FIPAddr,iGrade,strValue);
				  SetTotalizerData(TMONEY,TAddr,FIPAddr,iGrade,strValue);
			  }
		  }
	 }
}
                                       //12byte->6Byte         0 1 2 3  
void StrToRaw(char *Source,char *Dest){//Source: "1234567890AB"
     char i,xA,xB;                //Dest  :  12345678 
	 char xRaw=0;
	 for (i=0;i<6;i++){
	      xA=Ord(Source[(2*i)]);
		  xB=(0x0F&Ord(Source[(2*i)+1]));
	      xRaw=((xA<<4) | xB);
	      Dest[i]=xRaw;
	 }	      
}
void RawToStr(char *Source,char *Dest){//Source:   1234567890AB
     unsigned char i,xRaw,cA,cB;               //Dest  :  "12345678" 
	 for (i=0;i<6;i++){
	      cA=Chr(Source[i]>>4);
	      cB=Chr(Source[i]&0x0F);
          Dest[2*i]=cA;
          Dest[(2*i)+1]=cB;
	 }Dest[12]=0;
}

//SetTotalizerData(TVOLUME,TOTALIZER_NOW,1,1,strVolume)

void SaveTotalizerCurrentToLast(){
     char iPump,iGrade;
     char strValue[15];
     char FIPAddr;
	 
	 for (iPump=1;iPump<=16;iPump++){//Pump
	      FIPAddr=GetFIPAddr(iPump);
		  if (FIPAddr>0){
		      FIPAddr=FIPAddr-1;
			  for(iGrade=1;iGrade<=6;iGrade++){
				  FillChar(strValue,sizeof(strValue),0);
				  GetTotalizerData(TVOLUME,TOTALIZER_NOW,FIPAddr,iGrade,strValue);
				  SetTotalizerData(TVOLUME,TOTALIZER_LAST,FIPAddr,iGrade,strValue);
				  FillChar(strValue,sizeof(strValue),0);
				  GetTotalizerData(TMONEY,TOTALIZER_NOW,FIPAddr,iGrade,strValue);
				  SetTotalizerData(TMONEY,TOTALIZER_LAST,FIPAddr,iGrade,strValue);
			  }
		  }
	 }
}

void SetTotalizerData(char TType, char TAddr, char xPumpAddr, char xGradeAddr, char *strValue){// 0 1 1 "00000000"
     char rawValue[6],iPumpAddr,iGrade;
	 char strPValue[15];
	 char Idx;
	 char strSend[30];
	 //Convert String to RawData
	 if (strlen(strValue)<=12){
	     sprintf_P(strPValue,PSTR("%s"),strValue);
		 if (strlen(strPValue)<12)AddZeroLead(strPValue,12);
	 }
	 else {//Lebih dari 8: 0123456789
	     Idx=strlen(strValue)-12;
	     StrPosCopy(strValue,strPValue,Idx,12);
	 }

	 iPumpAddr=(xPumpAddr&0x0F);
	 if ((xGradeAddr>=1)&&(xGradeAddr<=6))
	     iGrade=xGradeAddr-1;
	 StrToRaw(strPValue,rawValue);

	 if (TType==TVOLUME)eeprom_write_block((const void*)&rawValue,(void*)&(TotalVolume[TAddr][iPumpAddr][iGrade]), sizeof(rawValue));
	 else 
	 if (TType==TMONEY)eeprom_write_block((const void*)&rawValue,(void*)&(TotalMoney[TAddr][iPumpAddr][iGrade]), sizeof(rawValue));
}



void GetTotalizerData(char TType, char TAddr, char xPumpAddr, char xGradeAddr, char *strValue){// 0 1 1 "0000000"
     char rawValue[6],iPumpAddr,iGrade;

	 iPumpAddr=(xPumpAddr&0x0F);
	 if ((xGradeAddr>=1)&&(xGradeAddr<=6))
	     iGrade=xGradeAddr-1;
     
//TotalVolume[2][16][6][6];
//GetTotalizerData(TVOLUME,TOTALIZER_LAST,PumpNum,xGrade,strLastVolume);
	 if (TType==TVOLUME)eeprom_read_block((void*) &rawValue, (const void*) &(TotalVolume[TAddr][iPumpAddr][iGrade]), sizeof(rawValue));
	 else 
	 if (TType==TMONEY)eeprom_read_block((void*) &rawValue, (const void*) &(TotalMoney[TAddr][iPumpAddr][iGrade]), sizeof(rawValue));
	 //Convert RawData to String 
	 RawToStr(rawValue,strValue);
}

char CalcMinus(char A, char B){
     signed char xC;//,xA,xB;
	 char Result;
	 if (A>=B) xC=((A-'0')-(B-'0'));
	 else xC=10+((A-'0')-(B-'0'));//
	 Result='0'+xC;
  return Result;
}

char CalcPlus(char A, char B){
     signed char xA,xB,xC;
	 char Result;
	 xC=((A-'0')+(B-'0'));
     Result='0'+(xC%10);  
   return Result;
}

char IsZerroAll(char *strZerro){
     char i,Length,nZerro=0,Result;
	 Length=strlen(strZerro);
	 nZerro=0;
	 Result=False;
	 for(i=0;i<Length;i++){
	     if (strZerro[Length-i]=='0')nZerro++;
	 }if (nZerro==Length) Result=True;
  return Result;
}

char Chr(char X){//Return Char Value
     char Result='0';
	 if ((X>=0)&&(X<=9)){
	    Result='0'+X;
	 }
	return Result;
}
char Ord(char c){//Return Ordinal Numbers
     char Result=0;
	 if ((c>='0')&&(c<='9')){
	    Result=c-'0';
	 }
	return Result;
}

char IsMoreThan(char *strA, char *strB){
     char i,LengthA,LengthB,Result=False;
	 char strSend[20];	 
     //Check Length
	 LengthA=strlen(strA);
	 LengthB=strlen(strB);
	 Result=False;
	 if (LengthA>LengthB)Result=True;
	 else 
	 if (LengthA==LengthB){          //456755 
	     for(i=0;i<LengthA;i++){    //456410   
		     if (Ord(strA[i])>Ord(strB[i])){
			     Result=True;
				 break;
			 }else if (Ord(strA[i])<Ord(strB[i])){
			     Result=False;
				 break;
			 }			 		 
		 }
	 }
  return Result;
}


void FTestCalculation(){
/*
static char stTestCalc=tcInitData,uiResult=USER_NONE;
static char InputA[20],InputB[20],OutputC[20];
     char lcdteks[20],KeyChar;
     
	 switch(stTestCalc){
	 case tcInitData:
	      lcd_clear();
		  lcd_printf(1,1,PSTR("Operation Multiply"));
		  lcd_printf(2,1,PSTR("InA:_"));
		  FillChar(InputA,sizeof(InputA),0);
		  FillChar(InputA,sizeof(InputB),0);
		  FillChar(InputA,sizeof(OutputC),0);

		  uiResult=USER_NONE;
          stTestCalc=tcInputA;
	      break;
     case tcInputA:
	      uiResult=UserInput(UI_ALPHANUM_R,2,5,InputA,0,15);
	      if (uiResult==USER_OK)stTestCalc=tcDispInputA;
	      break;
     case tcDispInputA:
	      sprintf_P(lcdteks,PSTR("InA:%s"),InputA);
		  lcd_print(2,1,lcdteks);
		  lcd_printf(3,1,PSTR("InB:_"));
		  uiResult=USER_NONE;
          stTestCalc=tcInputB;
	      break;
     case tcInputB:
	 	  uiResult=UserInput(UI_ALPHANUM_R,3,5,InputB,0,15);
	      if (uiResult==USER_OK)stTestCalc=tcCalcualte;
	      break;
     case tcCalcualte:
	      sprintf_P(lcdteks,PSTR("InB:%s"),InputB);
		  lcd_print(3,1,lcdteks);
		  //StrCalc(TMINUS,InputA,InputB,OutputC);
		  //StrCalc(TPLUS,InputA,InputB,OutputC);
		  StrCalc(TMULTIPLY,InputA,InputB,OutputC);
	      sprintf_P(lcdteks,PSTR("A+B:%s"),OutputC);
		  lcd_print(4,1,lcdteks);
          stTestCalc=tcWaitEnter;
	      break;
     case tcWaitEnter:
	      KeyChar= _key_btn(_key_scan(1));       
		  if (KeyChar=='#')stTestCalc=tcInitData;
		  else
		  if (KeyChar=='*')stTestCalc=tcInitData;
	      break;	 
	 }
*/
}

char GetMinusPos(char *strNumber){
char i,Result;
     Result=0;
     for(i=0;i<strlen(strNumber);i++){
	     if (strNumber[i]=='-'){
		     Result=i+1;
			 break;
		 }
	 }
  return Result;     
}

char IsMinus(char *strNumber){
char i,Result=False;
     Result=False;
	 if (GetMinusPos(strNumber)>0){
	     Result=True;
	 }     
  return Result;
}

void RemoveMinus(char *strNumber){
     char MinPos,Length;

     if (IsMinus(strNumber)==True){
	     MinPos=GetMinusPos(strNumber);
		 Length=strlen(strNumber);
		 StrPosCopy(strNumber,strNumber,MinPos,(Length-MinPos));	      
	 }
}

void NormalizeOverflow(char *strOverflowed){
char i,Length,strMaxValue[20];//-99999999

     Length=strlen(strOverflowed);//-999453
     for(i=0;i<Length;i++){//123456
	     strMaxValue[i]='0'+((Length-i)/Length);
	 }strMaxValue[Length]=0;
     
	 //if (NinePos<Length-4)
	 StrCalc(TPLUS,strMaxValue,strOverflowed,strOverflowed);
}
                                                                   //     1111111 
void StrCalc(char TOperation, char *strA , char *strB, char *strC){//  A: 00000000
     char i,j,lenA,lenB,FixLen,zMin,newC,xLead;//,xResult;            //  B: 00000021-
	 char tmpA[20],tmpB[20],tmpC[20],Result[20],iPos=0;            //   ----------- 
	 char strSend[40],TCalc;	  							       //  C: 00000019
     char IsMinA,IsMinB,IsNegative,IsSwap;
	 //long long valA,valB,valC;
	 
	 IsMinA=False;
	 IsMinB=False;
	 IsNegative=False;
	 FillChar(tmpA,sizeof(tmpA),0);
	 FillChar(tmpB,sizeof(tmpB),0);
	 FillChar(Result,sizeof(Result),0);
                                            // -5    -5     5    5
	                                        // -6 -   6 -  -6 -  6 -
                                            // -5+6   
	 sprintf_P(tmpA,PSTR("%s"),strA);       // 
	 sprintf_P(tmpB,PSTR("%s"),strB);  

     if (IsMinus(tmpA)==True){
	     RemoveMinus(tmpA);
		 IsMinA=True;
	 }
     if (IsMinus(tmpB)==True){
	     RemoveMinus(tmpB);
		 IsMinB=True;
	 }    	

     lenA=strlen(tmpA);
     lenB=strlen(tmpB);
	 if (lenA<lenB)FixLen=lenB;
	 else FixLen=lenA;
     /*
	 sprintf_P(strSend,PSTR("A:%s"),strA);	 uart_print(0,1,strSend);
	 sprintf_P(strSend,PSTR("B:%s"),strB);	 uart_print(0,1,strSend);
     */

     AddZeroLead(tmpA,FixLen+1);
     AddZeroLead(tmpB,FixLen+1);
	 /*
	 sprintf_P(strSend,PSTR("A:%s"),strA);
	 uart_print(0,1,strSend);
	 sprintf_P(strSend,PSTR("B:%s"),strB);
	 uart_print(0,1,strSend);
	 */

     lenA=strlen(tmpA);
     lenB=strlen(tmpB);
     IsNegative=False;
	 
	 IsNegative=False;
	 IsSwap=False;

	 if (IsMoreThan(tmpB,tmpA)==True){
	     IsSwap=True;
         sprintf_P(tmpC,PSTR("%s"),tmpA);
		 sprintf_P(tmpA,PSTR("%s"),tmpB);
		 sprintf_P(tmpB,PSTR("%s"),tmpC);
	 }
    
	if (TOperation==TMINUS){
	    TCalc=TOperation;
		IsNegative=False;
		if (IsSwap==True){
		    if ((IsMinA==True)&&(IsMinB==True)){IsNegative=False;TCalc=TMINUS;}
            else if ((IsMinA==True)&&(IsMinB==False)){IsNegative=True;TCalc=TPLUS;}
            else if ((IsMinA==False)&&(IsMinB==True)){IsNegative=False;TCalc=TPLUS;}
            else if ((IsMinA==False)&&(IsMinB==False)){IsNegative=True;TCalc=TMINUS;}
		}else
		if (IsSwap==False){
		    if ((IsMinA==True)&&(IsMinB==True)){IsNegative=True;TCalc=TMINUS;}
            else if ((IsMinA==True)&&(IsMinB==False)){IsNegative=True;TCalc=TPLUS;}
            else if ((IsMinA==False)&&(IsMinB==True)){IsNegative=False;TCalc=TPLUS;}
            else if ((IsMinA==False)&&(IsMinB==False)){IsNegative=False;TCalc=TMINUS;}
		}
	 }else
	if (TOperation==TPLUS){
	    TCalc=TOperation;
		IsNegative=False;
		if (IsSwap==True){
		    if ((IsMinA==True)&&(IsMinB==True)){IsNegative=True;TCalc=TPLUS;}
            else if ((IsMinA==True)&&(IsMinB==False)){IsNegative=False;TCalc=TMINUS;}
            else if ((IsMinA==False)&&(IsMinB==True)){IsNegative=True;TCalc=TMINUS;}
            else if ((IsMinA==False)&&(IsMinB==False)){IsNegative=False;TCalc=TPLUS;}
		}else
		if (IsSwap==False){
		    if ((IsMinA==True)&&(IsMinB==True)){IsNegative=True;TCalc=TPLUS;}
            else if ((IsMinA==True)&&(IsMinB==False)){IsNegative=True;TCalc=TMINUS;}
            else if ((IsMinA==False)&&(IsMinB==True)){IsNegative=False;TCalc=TMINUS;}
            else if ((IsMinA==False)&&(IsMinB==False)){IsNegative=False;TCalc=TPLUS;}
		}
	 }

	 /*sprintf_P(strSend,PSTR("A':%s"),tmpA);
	 uart_print(0,1,strSend);
	 sprintf_P(strSend,PSTR("B':%s"),tmpB);
	 uart_print(0,1,strSend);
*/

     zMin=0;
	 newC=0;
	 xLead=0;
   // A: 100000
   // B: 000001 -
   // -----------
   // C: 099999

	 if (TCalc==TMINUS){
	     for(i=0;i<lenA;i++){		 
		     if (Ord(tmpA[lenA-i-1])>=Ord(tmpB[lenB-i-1])){
			 //Cukup
			     Result[lenA-i-1]=Chr(Ord(tmpA[lenA-i-1])-Ord(tmpB[lenB-i-1]));
			 }else
			 if (Ord(tmpA[lenA-i-1])<Ord(tmpB[lenB-i-1])){
			 //Pinjaman
			     for(j=i+1;j<lenA;j++){
				     if (tmpA[lenA-j-1]=='0')tmpA[lenA-j-1]='9';
					 else
					 if (tmpA[lenA-j-1]!='0'){
					     tmpA[lenA-j-1]=Chr(Ord(tmpA[lenA-j-1])-1);
						 break;
					 }				 
				 }
				 Result[lenA-i-1]=Chr(10+Ord(tmpA[lenA-i-1])-Ord(tmpB[lenB-i-1]));
			 }
		 }Result[lenA]=0;		    
       RemZeroLead(Result);
//	 sprintf_P(strSend,PSTR("C':%s"),Result);
//	 uart_print(0,1,strSend);
		 
		 FixLen=strlen(Result);
		 iPos=0;
		 for(i=0;i<FixLen;i++){
		     if ((i==0)&&(IsNegative==True)){
			     strC[iPos]='-';
				 iPos++;
			 }
		     strC[iPos]=Result[i];
			 iPos++;
		 }strC[iPos]=0;
	 }
	 else                     //         1
	 if (TCalc==TPLUS){  //A:099999999
	     zMin=0;              //B:000000001
	     for(i=0;i<lenA;i++){ //C:       00
		     newC=(Ord(tmpA[lenA-i-1])+Ord(tmpB[lenB-i-1]));			 
			 if (newC<10){
			     Result[i]=Chr(newC);
				 zMin=newC/10;
			 }else if (newC>=10){
                 //Lebih
				 Result[i]=Chr(newC%10);
				 zMin=newC/10;
			     for(j=i+1;j<lenA;j++){
				     if ((Ord(tmpA[lenA-j-1])+zMin)>=10){
					     tmpA[lenA-j-1]=Chr((Ord(tmpA[lenA-j-1])+zMin)%10);
						 zMin=1;
					 }else{
					     tmpA[lenA-j-1]=Chr((Ord(tmpA[lenA-j-1])+zMin));
						 zMin=0;	
						 break;			     					 
					 }					 
				 }//EndFor j                
			   }//End else
			 }//EndFor

		  //Result[lenA]==Chr(zMin); ???
		  Result[lenA]=Chr(zMin);
		  Result[lenA+1]=0;
		 /*
		 FixLen=strlen(Result);
		 for(i=0;i<FixLen;i++){
		     strC[i]=Result[FixLen-i-1];
		 }strC[FixLen]=0;
*/
         FixLen=strlen(Result);
		 iPos=0;
		 for(i=0;i<FixLen;i++){
		     if ((i==0)&&(IsNegative==True)){
			     strC[iPos]='-';
				 iPos++;
			 }
		     strC[iPos]=Result[FixLen-i-1];
			 iPos++;
		 }strC[iPos]=0;

       RemZeroLead(strC);

  //    sprintf_P(strSend,PSTR("C':%s"),strC);
//	 uart_print(0,1,strSend);

	 }//EndIf 

	 if (TOperation==TMULTIPLY){
	     //strC=strA*strB
/*
		 RemZeroLead(strA);
		 RemZeroLead(strB);
		 valA=atol(strA);
		 valB=atol(strB);
		 valC=valA*valB;		 
         ltoa(valC,strC,10);
		 */
		 CalcMultiply(strA,strB,strC);
	 }
}

void CalcSegmen(char *strMain, char cNum, char *strResult){
     /*     3 
	     12356
		     5 x
       --------
         61725
	 */
	 char i,lenR,xCalc,xRes,xResNext,tmpResult[20];

	 xResNext=0;
     xRes=0;

	 for(i=0;i<strlen(strMain);i++){           
         xCalc=(strMain[strlen(strMain)-1-i]-'0')*(cNum-'0')+xResNext;//Hasil
		 xRes=xCalc%10;
		 xResNext=xCalc/10;
	     tmpResult[i]='0'+xRes;	 
		 tmpResult[i+1]=0;
	 }
	 if (xResNext>0){
	     lenR=strlen(tmpResult);
	     tmpResult[lenR]='0'+xResNext;
		 tmpResult[lenR+1]=0;	 
	 }
	 lenR=strlen(tmpResult);
	 for (i=0;i<lenR;i++){
	     strResult[i]=tmpResult[lenR-1-i];
	 }
	 strResult[lenR]=0;
}


void CalcMultiply(char *strA,char *strB,char *strC){
     /*
	 strA   12345678
	 strB		9801 x
              -------
            12345678 ->seg1
		    00000000 ->seg2
		    98765424 ->seg3
	      1111111902 ->seg4
----------------------
	 
	 */
	 char i,lenB;
	 char prevSeg[20],currSeg[20];

     RemZeroLead(strA);
     RemZeroLead(strB);
	 lenB=strlen(strB);
	 sprintf_P(prevSeg,PSTR("0"));
	 sprintf_P(currSeg,PSTR("0"));

	 for(i=0;i<strlen(strB);i++){
	     //
         CalcSegmen(strA,strB[lenB-1-i],currSeg);
         AddZeroLag(currSeg,strlen(currSeg)+i);
	     StrCalc(TPLUS,prevSeg,currSeg,prevSeg);
	 }
	 sprintf_P(strC,PSTR("%s"),prevSeg);
}

void GetTabSpace(signed char TabLength, char *strTab){
     char i,nTab;
	 if (TabLength>0){
		 nTab=TabLength;//%40;
		 for(i=0;i<nTab;i++){
		     strTab[i]=' ';
		 }strTab[nTab]=0;
     }else{
	 strTab[0]=' ';
	 strTab[1]=0;
	 }
}

//Spooling HFCS 0000 : Header, Footer, Copy , Scrool [Copy:16x max]
//			cmdPrint=procMessage81();
void systemGenerateReport(){

static char stGenerateReport=grScanAction;
static char xPump,xNozzle,xGrade,PumpNum,PumpNozzle,GradeUsed,GradeList[10];
static unsigned int RepPos=0;
       char strReport[80];
       char PPumpID[8],PProductID[6];
       char strLastVolume[15],strLastMoney[15],strCurrentVolume[15],strCurrentMoney[15];
	   char strSend[40],LastShiftDateTime[20];
	   char strTabSpace[20],strTabSpace2[20];
	   char iTotal;//,PadLength;
	   char sPrice[10],strProductPrice[20];
	   char FIPAddr;

     switch(stGenerateReport){
	 case grScanAction://Wait for Complete incoming Totalizer data	      
		  if (IsGenerateReport==True){
		      IsGenerateReport=False;
			  IsFinishPrintingTotalizer=False;
			  stGenerateReport=grInitData;
		  }
	      break;
	 case grInitData:
	      xPump=1;
		  xNozzle=1;
		  RepPos=0;
		  FillChar(PrintBuffer,sizeof(PrintBuffer),0);
		  FillChar(strReport,sizeof(strReport),0);

		  FillChar(strCurrentVolume,sizeof(strCurrentVolume),0);
		  FillChar(strCurrentMoney,sizeof(strCurrentMoney),0);

		  FillChar(strTotalVolume,sizeof(strTotalVolume),0);
		  FillChar(strTotalMoney,sizeof(strTotalMoney),0);


		  FillChar(strDeltaMoney,sizeof(strDeltaMoney),0);
		  FillChar(strDeltaVolume,sizeof(strDeltaVolume),0);
		  sprintf_P(strShift,PSTR("%d"),eeprom_read_byte(&DefShift));

          stGenerateReport=grCreateReportHeader;
	      break;
	 case grCreateReportHeader://Border: btTopLeft,btTopCenter,btTopRight,btMiddleLeft,btMiddleCenter,btMiddleRight,btBottomLeft,btBottomCenter,btBottomRight,btVertical,btHorizontal
		  eeprom_read_block((void*) &LastShiftDateTime, (const void*) &DefLastShiftDateTime, sizeof(DefLastShiftDateTime));
          
		  InserBorder(btTopLeft,PrintBuffer,1,&RepPos);InserBorder(btHorizontal,PrintBuffer,BORDER_LENGTH,&RepPos);InserBorder(btTopRight,PrintBuffer,1,&RepPos);InserBorder(btNewLine,PrintBuffer,1,&RepPos);
	      sprintf_P(strReport,PSTR("        Laporan Tutup Shift: %s      "),strShift);CreateReport(strReport,PrintBuffer,&RepPos);
		  InserBorder(btMiddleLeft,PrintBuffer,1,&RepPos);InserBorder(btHorizontal,PrintBuffer,BORDER_LENGTH,&RepPos);InserBorder(btMiddleRight,PrintBuffer,1,&RepPos);InserBorder(btNewLine,PrintBuffer,1,&RepPos);     
	      sprintf_P(strReport,PSTR("Terminal ID : %.2d                   "),eeprom_read_byte(&DefIFT_ID));CreateReport(strReport,PrintBuffer,&RepPos);
	      sprintf_P(strReport,PSTR("Awal  Shift : %s "),LastShiftDateTime);             CreateReport(strReport,PrintBuffer,&RepPos);
	      sprintf_P(strReport,PSTR("Akhir Shift : %s "),CurrentShiftDateTime);      CreateReport(strReport,PrintBuffer,&RepPos);
	      //sprintf_P(strReport,PSTR("Transaksi   : %s "),DeltaTransaction);      CreateReport(strReport,PrintBuffer,&RepPos);
          InserBorder(btBottomLeft,PrintBuffer,1,&RepPos);InserBorder(btHorizontal,PrintBuffer,BORDER_LENGTH,&RepPos);InserBorder(btBottomRight,PrintBuffer,1,&RepPos);InserBorder(btNewLine,PrintBuffer,1,&RepPos);
		  //---------------

		  //InserBorder(btTopLeft,PrintBuffer,1,&RepPos);InserBorder(btHorizontal,PrintBuffer,BORDER_LENGTH,&RepPos);InserBorder(btTopRight,PrintBuffer,1,&RepPos);InserBorder(btNewLine,PrintBuffer,1,&RepPos);          
		  //PrintMoney 
		  /*
	      if (eeprom_read_byte(&DefPrintMoney)==True)sprintf_P(strReport,PSTR("PUMP-PRODUCT   VOLUME(L)    RUPIAH(RP)"));         
	      else sprintf_P(strReport,PSTR("PUMP-PRODUCT   VOLUME(L)              "));CreateReport(strReport,PrintBuffer,&RepPos);
          InserBorder(btBottomLeft,PrintBuffer,1,&RepPos);InserBorder(btHorizontal,PrintBuffer,BORDER_LENGTH,&RepPos);InserBorder(btBottomRight,PrintBuffer,1,&RepPos);//InserBorder(btNewLine,PrintBuffer,1,&RepPos);         		 
          */
		  ClearList(GradeList);
		  cmdPrint=0b10000000;//PrintHeader
		  LengthMessage81=RepPos+1;
		  IsFreePrinting=True;
          IsBusyFreePrinting=True;
		  		  
          stGenerateReport=grWaitPrinted1;
	      break;
     case grWaitPrinted1:
          if (IsBusyFreePrinting==False)
		      stGenerateReport=grGenerateLabel;//grGenerateReportData;//grFinishGenerateReport;
	      break;
     case grGenerateLabel:
          RepPos=0;
          InserBorder(btTopLeft,PrintBuffer,1,&RepPos);InserBorder(btHorizontal,PrintBuffer,BORDER_LENGTH,&RepPos);InserBorder(btTopRight,PrintBuffer,1,&RepPos);InserBorder(btNewLine,PrintBuffer,1,&RepPos);          
	      if (eeprom_read_byte(&DefPrintMoney)==True)
		       sprintf_P(strReport,PSTR("PUMP-PRODUCT   VOLUME(L)    RUPIAH(RP)"));         
	      else sprintf_P(strReport,PSTR("PUMP-PRODUCT   VOLUME(L)              "));CreateReport(strReport,PrintBuffer,&RepPos);
          InserBorder(btBottomLeft,PrintBuffer,1,&RepPos);InserBorder(btHorizontal,PrintBuffer,BORDER_LENGTH,&RepPos);InserBorder(btBottomRight,PrintBuffer,1,&RepPos);//InserBorder(btNewLine,PrintBuffer,1,&RepPos);         		 
          
		  cmdPrint=0b00000000;
		  LengthMessage81=RepPos+1;

		  IsFreePrinting=True;
	      IsBusyFreePrinting=True;

		  stGenerateReport=grWaitLabelPrinted;
	      break;
     case grWaitLabelPrinted:
          if (IsBusyFreePrinting==False)
		      stGenerateReport=grGenerateReportData;//grFinishGenerateReport;
	      break;
     case grGenerateReportData:
	      //Generate: PumpNum, Nozzle, Product
	      if ((xPump>=1)&&(xPump<=8)){
		      eeprom_read_block((void*) &PPumpID, (const void*) &DefPumpMap, 8);
			  PumpNum=PPumpID[xPump-1];
			  if (PumpNum>0){
				  eeprom_read_block((void*) &PProductID, (const void*) &DefNozzleMap[xPump-1], 6);
				  xGrade=PProductID[xNozzle-1];

				  if (xGrade>0){
				      GetProductName(xGrade,strProduct);
                      GradeUsed=xGrade;
					  xGrade=xNozzle;//
					  PumpNozzle=xNozzle;
					  xNozzle++;
					  stGenerateReport=grCreateReportTotalizer;
				  }
				  else{sprintf_P(strProduct,PSTR("N/A"));			  
					   xNozzle++;
					   if (xNozzle>6)stGenerateReport=grNextPump;
					  }
				}else stGenerateReport=grNextPump;
			  }
	      break;
     case grNextPump:
	      xNozzle=1;
	      xPump++;
	      if (xPump>8)stGenerateReport=grCreateReportFooter;
		  else stGenerateReport=grGenerateReportData;
	      break;
     case grCreateReportTotalizer:

          RepPos=0;
		  FillChar(PrintBuffer,sizeof(PrintBuffer),0);
		  FillChar(strReport,sizeof(strReport),0);

		  FillChar(strDeltaMoney,sizeof(strDeltaMoney),0);
		  FillChar(strDeltaVolume,sizeof(strDeltaVolume),0);

		  FillChar(strLastVolume,sizeof(strLastVolume),0);
		  FillChar(strLastMoney,sizeof(strLastMoney),0);

		  FillChar(strCurrentVolume,sizeof(strCurrentVolume),0);
		  FillChar(strCurrentMoney,sizeof(strCurrentMoney),0);

          FIPAddr=GetFIPAddr(PumpNum); 
		  if (FIPAddr>0){
		      FIPAddr=FIPAddr-1;		  
			  GetTotalizerData(TVOLUME,TOTALIZER_LAST,FIPAddr,PumpNozzle,strLastVolume);
			  GetTotalizerData(TMONEY,TOTALIZER_LAST,FIPAddr,PumpNozzle,strLastMoney);

			  GetTotalizerData(TVOLUME,TOTALIZER_NOW,FIPAddr,PumpNozzle,strCurrentVolume);
			  GetTotalizerData(TMONEY,TOTALIZER_NOW,FIPAddr,PumpNozzle,strCurrentMoney);
		  }

		  StrCalc(TMINUS,strCurrentVolume,strLastVolume,strDeltaVolume);

		  if (IsMinus(strDeltaVolume)==True)
		      NormalizeOverflow(strDeltaVolume);

		  //Calculate Wayne Estimated Total Money
		  if (eeprom_read_byte(&DefDispenserBrand)==ST_WAYNE_DART){
              GetProductPrice(sPrice,PumpNum,PumpNozzle);//Money = Price x Volume
			  StrCalc(TMULTIPLY,sPrice,strDeltaVolume,strDeltaMoney);
			  
		  }else StrCalc(TMINUS,strCurrentMoney,strLastMoney,strDeltaMoney);

		  if (IsMinus(strDeltaMoney)==True)
		      NormalizeOverflow(strDeltaMoney);

		  AddList(GradeUsed,GradeList);
		  
		  StrCalc(TPLUS,strTotalVolume,strDeltaVolume,strTotalVolume);
		  StrCalc(TPLUS,strTotalMoney,strDeltaMoney,strTotalMoney);


		  RemZeroLead(strDeltaMoney);
		  RemZeroLead(strCurrentMoney);
		  RemZeroLead(strLastMoney);

          RemZeroLead(strDeltaVolume);
          RemZeroLead(strCurrentVolume);
          RemZeroLead(strLastVolume);

		  FormatTotalizerMoney(strDeltaMoney);
		  FormatTotalizerMoney(strCurrentMoney);
		  FormatTotalizerMoney(strLastMoney);

		  FormatTotalizerVolume(strDeltaVolume);
		  FormatTotalizerVolume(strLastVolume);
		  FormatTotalizerVolume(strCurrentVolume); 

          FormatCurrency(strDeltaMoney);
		  FormatCurrency(strCurrentMoney);
		  FormatCurrency(strLastMoney);

		  FormatCurrency(strDeltaVolume);		  
		  FormatCurrency(strLastVolume);
          FormatCurrency(strCurrentVolume);


          InserBorder(btTopLeft,PrintBuffer,1,&RepPos);InserBorder(btHorizontal,PrintBuffer,BORDER_LENGTH,&RepPos);InserBorder(btTopRight,PrintBuffer,1,&RepPos);InserBorder(btNewLine,PrintBuffer,1,&RepPos);          
          if (eeprom_read_byte(&DefDispenserBrand)==ST_WAYNE_DART){
		      GetProductPrice(sPrice,PumpNum,PumpNozzle);
			  sprintf_P(strProductPrice,PSTR("Harga: Rp.%s"),sPrice);

		      GetTabSpace(((17+10-strlen(strProduct))-strlen(strProductPrice)),strTabSpace2);
		      sprintf_P(strReport,PSTR("P%d.%d - %s %s %s"),PumpNum,PumpNozzle,strProduct,strTabSpace2,strProductPrice);
		  }
		  else sprintf_P(strReport,PSTR("P%d.%d - %s "),PumpNum,PumpNozzle,strProduct);
		  CreateReport(strReport,PrintBuffer,&RepPos);
          InserBorder(btMiddleLeft,PrintBuffer,1,&RepPos);InserBorder(btHorizontal,PrintBuffer,BORDER_LENGTH,&RepPos);InserBorder(btMiddleRight,PrintBuffer,1,&RepPos);InserBorder(btNewLine,PrintBuffer,1,&RepPos);          

		  GetTabSpace((17-strlen(strCurrentVolume)),strTabSpace);
		  GetTabSpace((15-strlen(strCurrentMoney)),strTabSpace2);
		  //PrintMoney
		  if (eeprom_read_byte(&DefPrintMoney)==True) sprintf_P(strReport,PSTR("Akhir:%s%s%s%s"),strTabSpace,strCurrentVolume,strTabSpace2,strCurrentMoney);//CreateReport(strReport,PrintBuffer,&RepPos);
		  else sprintf_P(strReport,PSTR("Akhir:%s%s"),strTabSpace,strCurrentVolume);
		  CreateReport(strReport,PrintBuffer,&RepPos);

		  GetTabSpace((17-strlen(strLastVolume)),strTabSpace);
		  GetTabSpace((15-strlen(strLastMoney)),strTabSpace2);
		  //PrintMoney
		  if (eeprom_read_byte(&DefPrintMoney)==True)sprintf_P(strReport,PSTR("Awal :%s%s%s%s"),strTabSpace,strLastVolume,strTabSpace2,strLastMoney);//CreateReport(strReport,PrintBuffer,&RepPos);
		  else sprintf_P(strReport,PSTR("Awal :%s%s"),strTabSpace,strLastVolume);
		  CreateReport(strReport,PrintBuffer,&RepPos);

		  GetTabSpace(12,strTabSpace);
          //PrintMoney
          if (eeprom_read_byte(&DefPrintMoney)==True)sprintf_P(strReport,PSTR("%s-----------   -------------"),strTabSpace);//CreateReport(strReport,PrintBuffer,&RepPos);
		  else sprintf_P(strReport,PSTR("%s-----------                "),strTabSpace);CreateReport(strReport,PrintBuffer,&RepPos);

		  GetTabSpace((23-strlen(strDeltaVolume)),strTabSpace);
		  GetTabSpace((15-strlen(strDeltaMoney)),strTabSpace2);
          //PrintMoney
		  if (eeprom_read_byte(&DefPrintMoney)==True)sprintf_P(strReport,PSTR("%s%s%s%s"),strTabSpace,strDeltaVolume,strTabSpace2,strDeltaMoney);//CreateReport(strReport,PrintBuffer,&RepPos);
          else sprintf_P(strReport,PSTR("%s%s"),strTabSpace,strDeltaVolume);CreateReport(strReport,PrintBuffer,&RepPos);

          InserBorder(btBottomLeft,PrintBuffer,1,&RepPos);InserBorder(btHorizontal,PrintBuffer,BORDER_LENGTH,&RepPos);InserBorder(btBottomRight,PrintBuffer,1,&RepPos);InserBorder(btNewLine,PrintBuffer,1,&RepPos);


          cmdPrint=0b00000000;
		  LengthMessage81=RepPos+1;

		  IsFreePrinting=True;
	      IsBusyFreePrinting=True;

	      stGenerateReport=grWaitPrinted2;
	      break;
     case grWaitPrinted2:
	 
          if (IsBusyFreePrinting==False)stGenerateReport=grGenerateReportData;
	      break;
     case grCreateReportFooter:
          RepPos=0;
		  FillChar(PrintBuffer,sizeof(PrintBuffer),0);
		  FillChar(strReport,sizeof(strReport),0);

		  RemZeroLead(strTotalMoney);
		  RemZeroLead(strTotalVolume);

		  FormatTotalizerMoney(strTotalMoney);
		  FormatCurrency(strTotalMoney);
		  FormatTotalizerVolume(strTotalVolume);
		  FormatCurrency(strTotalVolume);

          //Test
		  //RemZeroLead(strTotalMoney);
		  //RemZeroLead(strTotalVolume);

		  InserBorder(btTopLeft,PrintBuffer,1,&RepPos);InserBorder(btHorizontal,PrintBuffer,BORDER_LENGTH,&RepPos);InserBorder(btTopRight,PrintBuffer,1,&RepPos);InserBorder(btNewLine,PrintBuffer,1,&RepPos);          
		  GetTabSpace((17-strlen(strTotalVolume)),strTabSpace);
		  GetTabSpace((15-strlen(strTotalMoney)),strTabSpace2);
          //PrintMoney
		  if (eeprom_read_byte(&DefPrintMoney)==True)sprintf_P(strReport,PSTR("TOTAL %s%s%s%s"),strTabSpace,strTotalVolume,strTabSpace2,strTotalMoney);//CreateReport(strReport,PrintBuffer,&RepPos);
		  else sprintf_P(strReport,PSTR("TOTAL %s%s"),strTabSpace,strTotalVolume);CreateReport(strReport,PrintBuffer,&RepPos);

		  InserBorder(btMiddleLeft,PrintBuffer,1,&RepPos);InserBorder(btHorizontal,PrintBuffer,BORDER_LENGTH,&RepPos);InserBorder(btMiddleRight,PrintBuffer,1,&RepPos);InserBorder(btNewLine,PrintBuffer,1,&RepPos);     	      

		  for(iTotal=0;iTotal<strlen(GradeList);iTotal++){
		      xGrade=Ord(GradeList[iTotal]);
			  if (xGrade>0){
			      GetProductName(xGrade,strProduct);
				  
				  FillChar(strTotalVolume,sizeof(strTotalVolume),0);
				  FillChar(strTotalMoney,sizeof(strTotalMoney),0);

				  for(xPump=1;xPump<=8;xPump++){
				      eeprom_read_block((void*) &PPumpID, (const void*) &DefPumpMap, 8);
			          PumpNum=PPumpID[xPump-1];
					  if (PumpNum>0){
						  for (xNozzle=1;xNozzle<=6;xNozzle++){
							  eeprom_read_block((void*) &PProductID, (const void*) &DefNozzleMap[xPump-1], 6);
							  //Found Grade
						      if (PProductID[xNozzle-1]==xGrade){

						          FillChar(strDeltaMoney,sizeof(strDeltaMoney),0);
								  FillChar(strDeltaVolume,sizeof(strDeltaVolume),0);
								  FillChar(strLastVolume,sizeof(strLastVolume),0);
								  FillChar(strLastMoney,sizeof(strLastMoney),0);
								  FillChar(strCurrentVolume,sizeof(strCurrentVolume),0);
								  FillChar(strCurrentMoney,sizeof(strCurrentMoney),0);

								  PumpNozzle=xNozzle;
								  
								  FIPAddr=GetFIPAddr(PumpNum); 
								  if (FIPAddr>0){
									  FIPAddr=FIPAddr-1;							  							  							      
									  GetTotalizerData(TVOLUME,TOTALIZER_LAST,FIPAddr,PumpNozzle,strLastVolume);
									  GetTotalizerData(TMONEY,TOTALIZER_LAST,FIPAddr,PumpNozzle,strLastMoney);

									  GetTotalizerData(TVOLUME,TOTALIZER_NOW,FIPAddr,PumpNozzle,strCurrentVolume);
									  GetTotalizerData(TMONEY,TOTALIZER_NOW,FIPAddr,PumpNozzle,strCurrentMoney);
								  }

								  StrCalc(TMINUS,strCurrentVolume,strLastVolume,strDeltaVolume);
								  if (IsMinus(strDeltaVolume)==True)
								      NormalizeOverflow(strDeltaVolume);


								  //Calculate Wayne Estimated Total Money
								  if (eeprom_read_byte(&DefDispenserBrand)==ST_WAYNE_DART){
						              GetProductPrice(sPrice,PumpNum,PumpNozzle);//Money = Price x Volume
									  StrCalc(TMULTIPLY,sPrice,strDeltaVolume,strDeltaMoney);
								  }else StrCalc(TMINUS,strCurrentMoney,strLastMoney,strDeltaMoney);
								  //StrCalc(TMINUS,strCurrentMoney,strLastMoney,strDeltaMoney);
								  if (IsMinus(strDeltaMoney)==True)
								      NormalizeOverflow(strDeltaMoney);
		  
								  StrCalc(TPLUS,strTotalVolume,strDeltaVolume,strTotalVolume);
								  StrCalc(TPLUS,strTotalMoney,strDeltaMoney,strTotalMoney);
							  }
						  }
					  }				  
				  }

				  RemZeroLead(strTotalMoney);
				  RemZeroLead(strTotalVolume);

				  FormatTotalizerMoney(strTotalMoney);
				  FormatCurrency(strTotalMoney);

				  FormatTotalizerVolume(strTotalVolume);
				  FormatCurrency(strTotalVolume);

		          //Test
				 // RemZeroLead(strTotalMoney);
				 // RemZeroLead(strTotalVolume);


			      GetTabSpace((21-strlen(strProduct)-strlen(strTotalVolume)),strTabSpace);
			      GetTabSpace((15-strlen(strTotalMoney)),strTabSpace2);

				  //PrintMoney
		          if (eeprom_read_byte(&DefPrintMoney)==True)sprintf_P(strReport,PSTR("%d.%s%s%s%s%s"),iTotal+1,strProduct,strTabSpace,strTotalVolume,strTabSpace2,strTotalMoney);
				  else sprintf_P(strReport,PSTR("%d.%s%s%s"),iTotal+1,strProduct,strTabSpace,strTotalVolume);

				  CreateReport(strReport,PrintBuffer,&RepPos);
               }
		  }
          InserBorder(btBottomLeft,PrintBuffer,1,&RepPos);InserBorder(btHorizontal,PrintBuffer,BORDER_LENGTH,&RepPos);InserBorder(btBottomRight,PrintBuffer,1,&RepPos);InserBorder(btNewLine,PrintBuffer,1,&RepPos);
		  InserBorder(btNewLine,PrintBuffer,1,&RepPos);InserBorder(btNewLine,PrintBuffer,1,&RepPos);InserBorder(btNewLine,PrintBuffer,1,&RepPos);InserBorder(btNewLine,PrintBuffer,1,&RepPos);InserBorder(btNewLine,PrintBuffer,1,&RepPos);


          cmdPrint=0b00010000|(1<<PRN_PAPER_CUT);
		  LengthMessage81=RepPos+1;
		  IsFreePrinting=True;
	      IsBusyFreePrinting=True; 
         	       
	      stGenerateReport=grWaitPrinted3;
	      break;
     case grWaitPrinted3:
          if (IsBusyFreePrinting==False)stGenerateReport=grFinishGenerateReport;
	      break;
     case grFinishGenerateReport:
	      IsFinishPrintingTotalizer=True;
          stGenerateReport=grScanAction;
	      break;
	 }
}
//SaveTotalProduct(xGrade,strDeltaVolume,strDeltaMoney);
void GetProductPrice(char *sPrice,char xPumpID,char xNozzle){
char i,PPumpID[8],PProdID[6],strPrice[9],iPumpID,xProductID;
     sprintf_P(sPrice,PSTR(""));
     //FindPumpID
	 eeprom_read_block((void*) &PPumpID, (const void*) &DefPumpMap,8);
	 iPumpID=0;
	 for(i=0;i<8;i++){
	     if (PPumpID[i]==xPumpID){
		     iPumpID=i;
			 break;
		 }	 
	 }
	 if ((iPumpID>=0)&&(xNozzle<6)){
         eeprom_read_block((void*) &PProdID,(const void*) &DefNozzleMap[iPumpID],6);
         xProductID=PProdID[xNozzle-1];
		 //Price
		 eeprom_read_block((void*) &strPrice, (const void*) &DefProductPrice[xProductID-1], 9);
		 RemDecimal(strPrice);
		 sprintf_P(sPrice,PSTR("%s"),strPrice);
	 }    
	 //Clear Decimal

}

void GetProductName(char GradeId,char *strProductName){
char SProductName[12],i,Length;
     FillChar(SProductName,sizeof(SProductName),0); 
	 //eeprom_read_block((void*) &SProductName, (const void*) &DefProductName[GradeId-1],10);
	 if (GradeId>0) eeprom_read_block((void*) &SProductName, (const void*) &DefProductName[GradeId-1],10);
	 else sprintf_P(SProductName,PSTR("N/A"));

     Length=strlen(SProductName);
	 if (Length>10)Length=10;

	 for(i=0;i<Length;i++){
	     strProductName[i]=SProductName[i];
	 }strProductName[Length]=0;

	 AddSpaceLag(strProductName,10);
}

//AddListTotal(xGrade,strRef1);
void ClearList(char *strList){
     FillChar(strList,strlen(strList),0);
}
void AddList(char GradeId, char *strList){
     char i,iPos,Length,IsFound=False;	 
	 if ((GradeId>=1)&&((GradeId<=6))){
		 iPos=0;
		 IsFound=False;	
		 Length=strlen(strList);
	     if (Length>0){
			 for (i=0;i<Length;i++){//"123"
			      //uart(1,1,strList[i]);
			      iPos++;
			      if (strList[i]==Chr(GradeId)){
				      IsFound=True;
					  break;
				  }
			 }
	     }
		 if (IsFound==False){
		     strList[iPos]=Chr(GradeId);
		     strList[iPos+1]=0;
		 }
     }
	 //uart_print(1,1,strList);

}

char GetBorderValue(char BoderType){
     char Result=0;
	 switch (BoderType){
		case btTopLeft: 
		     Result=0xDA;
			 break;
		case btTopCenter: 
		     Result=0xC2;
			 break;
		case btTopRight:
		     Result=0xBF;		 
			 break;
		case btMiddleLeft: 
			 Result=0xC3;
			 break;
		case btMiddleRight: 
			 Result=0xB4;
			 break;
		case btBottomLeft: 
		     Result=0xC0;
			 break;
		case btBottomCenter: 
			 Result=0xC1;
			 break;
		case btBottomRight: 
			 Result=0xD9;
			 break;
        case btMiddleCenter:
		     Result=0xC5;
		     break;
		case btVertical: 
			 Result=0xB3;
			 break;
		case btHorizontal:
			 Result=0xC4;
			 break;	 
        case btNewLine:
		     Result=0x0D;
			 break;	 
	 }
   return Result;
}

void InserBorder(char BorderType, char *strPrnBuffer,char nLength,unsigned int *Pos){
     char i;//,BorderValue=0;
	 unsigned int StartPos,iPos=0;
	 
	 iPos=*Pos;
	 StartPos=iPos;
	 for (i=0;i<nLength;i++){
	      strPrnBuffer[iPos]=GetBorderValue(BorderType);
		  iPos++;
	 }
	 //Verify CharPosCopy
	 //for (i=0;i<nLength;i++){
	 //     if (strPrnBuffer[i+StartPos]!=GetBorderValue(BorderType)){
//		      strPrnBuffer[i+StartPos]=GetBorderValue(BorderType);
//		  }
//	 }
     *Pos=iPos;
}

void CreateReport(char *strData, char *strPrnBuffer, unsigned int *Pos){
     unsigned int iPos,PosResult,Length;
	 char i,strSend[20];
	 Length=strlen(strData);
	 if (Length>BORDER_LENGTH)Length=BORDER_LENGTH;
     
	 iPos=*Pos;
	 strPrnBuffer[iPos]=GetBorderValue(btVertical);
     iPos++;

	 for (i=0;i<Length;i++){	      
	      strPrnBuffer[iPos]=strData[i];	      	 
		  iPos++;
	 }
	 if (Length<BORDER_LENGTH){
	     for (i=0;i<(BORDER_LENGTH-Length);i++){	      
	          strPrnBuffer[iPos]=' ';
		      iPos++;
	     }	    
	 }
	 strPrnBuffer[iPos]=GetBorderValue(btVertical);
     iPos++;
     //PosResult=*Pos+Length;
	 strPrnBuffer[iPos]=0x0D; 
	 iPos++;

   //*Pos=PosResult+1;
   *Pos=iPos;
    //sprintf_P(strSend,PSTR("Pos:%d"),*Pos);
	//uart_print(0,1,strSend);
}


char TestUserInput(){
/*
static char strNumber[15];
char lcdteks[20],strSend[30];
char SVolume[15],SMoney[15],STotalVolume[15],STotalMoney[15];
char Result;

char uiResult;
	uiResult=UserInput(UI_ALPHANUM_R,2,1,strNumber,0,10);
	Result=MENU_NONE;
    if (uiResult==USER_OK){
	    sprintf_P(lcdteks,PSTR("Data:%s"),strNumber);
	   lcd_print(3,1,lcdteks);
	   //FormatCurrency(strNumber);
	   //FormatMoney(strNumber);
	   //RemZeroLead(strNumber);
	   sprintf_P(SVolume,PSTR("%s"),strNumber);
	   sprintf_P(SMoney,PSTR("%s"),strNumber);
	   sprintf_P(STotalVolume,PSTR("%s"),strNumber);
	   sprintf_P(STotalMoney,PSTR("%s"),strNumber);

	   sprintf_P(lcdteks,PSTR("Formated:%s"),strNumber);
       lcd_print(4,1,lcdteks);
	   sprintf_P(strSend,PSTR("Raw:%s"),strNumber);
	   uart_print(0,1,strSend);

	   RemoveMinus(strNumber);

	   sprintf_P(strSend,PSTR("Minus:%s"),strNumber);
	   uart_print(0,1,strSend);

       
	   RemZeroLead(strNumber);
	   sprintf_P(strSend,PSTR("Removed:%s"),strNumber);
	   uart_print(0,1,strSend);

RemZeroLead(SVolume);
RemZeroLead(SMoney);
RemZeroLead(STotalVolume);
RemZeroLead(STotalMoney);
       
	   
	   FormatVolume(SVolume); 
	   FormatMoney(SMoney);
	   FormatTotalizerMoney(STotalVolume);
	   FormatTotalizerVolume(STotalMoney);

	   sprintf_P(strSend,PSTR("Volume:%s"),SVolume);
	   uart_print(0,1,strSend);
	   sprintf_P(strSend,PSTR("Money:%s"),SMoney);
	   uart_print(0,1,strSend);
	   sprintf_P(strSend,PSTR("TVolume:%s"),STotalVolume);
	   uart_print(0,1,strSend);
	   sprintf_P(strSend,PSTR("TMoney:%s"),STotalMoney);
	   uart_print(0,1,strSend);

	   FormatCurrency(SMoney);
	   FormatCurrency(STotalMoney);
    
	   sprintf_P(strSend,PSTR("FmtMoney:%s"),SMoney);
	   uart_print(0,1,strSend);
	   sprintf_P(strSend,PSTR("FmtTMoney:%s"),STotalMoney);
	   uart_print(0,1,strSend);


   	
	}else    
	if (uiResult==USER_CANCEL){
	   lcd_clear();
	   FillChar(lcdteks,sizeof(strNumber),0);
       Result=MENU_DONE;   
	   //lcd_printf(1,1,PSTR("InputNumber:"));
       //lcd_printf(2,1,PSTR("_         "));
	}
  return Result;
  */
}
		

//Genius Protocol Version:1.0 AKR
int Pos(char *Substr, char *String){//1245:4645
    char i,j,CountMatch;
	char Length,subLen,PosFound;

	Length=strlen(String);
	subLen=strlen(Substr);
	PosFound=Length;
	for(i=0;i<Length;i++){
	    if (String[i]==Substr[0]){
		    CountMatch=0;
			for(j=0;j<subLen;j++){
			    //notFound
			    if (Substr[j]==String[i+j]){
                    CountMatch++;
				}else break;							
			}
			if (CountMatch>=subLen){
			    PosFound=i;
			    break;  
			}
		}
	}
  return  PosFound;
}


void GetParameter(char *GFlow,char FLength,char *GCmd,char *GPumpID, char *GeniCSum){
                         //C01:217<0x0D> 
						 //T01:239<0x0D> 
						 //S01:238<0x0D> 
						 //R01:237<0x0D> 
						 //P01A1234567890:237<0x0D> 
char i,xCmd,xPumpID,xCsum;
char sPumpID[3],sCSum[3];
char SGeniusFlow[30];
char strSend[20],strSub[3];
int SumLength;

     //TransposeFlow
     for (i=0;i<FLength;i++){	 
	     SGeniusFlow[i]=GFlow[FLength-i-1];
	 }SGeniusFlow[FLength]=0;

	 xCmd=CharPosCopy(SGeniusFlow,0);
	 *GCmd=xCmd;
	 StrPosCopy(SGeniusFlow,sPumpID,1,2);
	 xPumpID=atoi(sPumpID);
	 *GPumpID=xPumpID;
	 sprintf_P(strSub,PSTR(":"));
	 if (Pos(strSub,SGeniusFlow)<FLength){
	     SumLength=FLength-Pos(strSub,SGeniusFlow);
	 	 StrPosCopy(SGeniusFlow,sCSum,Pos(strSub,SGeniusFlow)+1,SumLength);
		 xCsum=atoi(sCSum);
	     *GeniCSum=xCsum;
	 }
	 //TestSend
/*	 sprintf_P(strSend,PSTR("%s"),SGeniusFlow);
	 uart_print(1,1,strSend);

	 sprintf_P(strSend,PSTR("%c"),xCmd);
	 uart_print(1,1,strSend);
	 sprintf_P(strSend,PSTR("%d"),xPumpID);
	 uart_print(1,1,strSend);
	 sprintf_P(strSend,PSTR("%s"),sCSum);
	 uart_print(1,1,strSend);
*/
}

char IsGeniusCommand(char GCommand){
     char i,Result,CommandList[10];
     Result=False;
	 sprintf_P(CommandList,PSTR("CTSRP"));
	 for(i=0;i<strlen(CommandList);i++){
	    if (CommandList[i]==GCommand){
		    Result=True;
			break;
			}
	 }
   return Result;	 
}

//enum eGeniusProtocolCommand{GP_PUMP_STATUS,GP_PUMP_LAST_TRANSACTION,GP_PUMP_STOP,GP_PUMP_RESUME,GP_PUMP_PRESET};

void GeniusProtocol(char dataIn){
     static char Geniflow[20],FlowLength=0,IsGeniusFlow=False;
	 char i,GeniCmd,GeniPumpID,GeniCSum;//,sCmd[7];
	 char strSend[20];

    //uart(1,1,dataIn);

	if (dataIn==0x0D){	
	    GetParameter(Geniflow,FlowLength,&GeniCmd,&GeniPumpID,&GeniCSum);
		FlowLength=0;
		IsGeniusFlow=False;
		FillChar(strSend,0,sizeof(strSend));
		sprintf_P(strSend,PSTR("%c%.2d:"),GeniCmd,GeniPumpID);
		//uart_print(1,1,strSend);


		if ((SumChecksum(strSend))==GeniCSum){
		     switch(GeniCmd){
			 case 'C'://PumpStatus
			      GeniusSendPumpStatus(GeniPumpID);
			      break;
             case 'T'://Transaction Request: T01:239<0D>
			      GeniusSendLastTransaction(GeniPumpID);
			      break; 
			 case 'S'://StopPump
			      GeniusSendStopPump(GeniPumpID);
			      break;
			 case 'R'://ResumePump
			      GeniusSendResumePump(GeniPumpID);
			      break;
			 case 'P'://PumpPreset
			      GeniusSendPumpPreset(GeniPumpID);
			      break;
			 }
		}
	}
	//Shifting
	for(i=19;i>0;i--){
	    Geniflow[i]=Geniflow[i-1];	
	}Geniflow[0]=dataIn;

	if (IsGeniusCommand(dataIn)==True)IsGeniusFlow=True;
	if (IsGeniusFlow==True)FlowLength++;

/*
	Geniflow[9]=Geniflow[8];
	Geniflow[8]=Geniflow[7];
	Geniflow[7]=Geniflow[6];
	Geniflow[6]=Geniflow[5];
	Geniflow[5]=Geniflow[4];
	Geniflow[4]=Geniflow[3];
	Geniflow[3]=Geniflow[2];
	Geniflow[2]=Geniflow[1];
	Geniflow[1]=Geniflow[0];
	Geniflow[0]=dataIn;
*/
}

char SumChecksum(char *strChecked){
     unsigned int SigmaSum=0;
	 char i,Result;
	 for(i=0;i<strlen(strChecked);i++){
	     SigmaSum=SigmaSum+strChecked[i];
	 }
	 Result=(SigmaSum%255);
	 if (Result==0)Result=255;
   return Result;
}

void NormalizeDecimal(char CurrentDecimal, char NewDecimal, char *Result){// 01234567,89
/*
char DeltaDec=0,Length,SResult[20];
     
	 Length=strlen(Result);
     if (CurrentDecimal==NewDecimal){// 01234567,89
	 }else 
	 if (CurrentDecimal<NewDecimal){// 1234567,890
	     DeltaDec=NewDecimal-CurrentDecimal;	           
         StrPosCopy(Result,SResult,DeltaDec,Length-DeltaDec);
		 AddZeroLag(SResult,Length);
		 Length=strlen(SResult);
		 StrPosCopy(SResult,Result,0,Length);
	 }else if (CurrentDecimal>NewDecimal){// 0001234567 
	     DeltaDec=CurrentDecimal-NewDecimal;
		 StrPosCopy(Result,SResult,DeltaDec,Length-DeltaDec);
		 	           
	 
     }
*/
     FormatDecimal(Result,CurrentDecimal);
}

void GetTransactionVolume(char iPumpID,char *Result){
     AddZeroLead(Result,10);
     sprintf_P(Result,PSTR("%s"),RecPumpData[iPumpID&0x0F].Volume); 
	 //NormalizeDecimal(eeprom_read_byte(&DefDecimalVolume),3,Result);
}
void GetTransactionMoney(char iPumpID,char *Result){
     AddZeroLead(Result,10);
	 sprintf_P(Result,PSTR("%s"),RecPumpData[iPumpID&0x0F].Money);
	 //NormalizeDecimal(eeprom_read_byte(&DefDecimalMoney),3,Result);
}

void GetTotalizerVolume(char iPumpID,char iGrade,char *Result){
     char FIPAddr;
     AddZeroLead(Result,11);
	 FIPAddr=GetFIPAddr(iPumpID);
	 if (FIPAddr>0){
	     FIPAddr=FIPAddr-1;	     
	     GetTotalizerData(TVOLUME,TOTALIZER_NOW,FIPAddr,iGrade,Result);
	 }
	 //NormalizeDecimal(eeprom_read_byte(&DefDecimalTotalVolume),2,Result);
}
void GetTotalizerMoney(char iPumpID,char iGrade, char *Result){
     char FIPAddr;
     AddZeroLead(Result,13); 
	 FIPAddr=GetFIPAddr(iPumpID);
	 if (FIPAddr>0){
	    FIPAddr=FIPAddr-1;
	    GetTotalizerData(TMONEY,TOTALIZER_NOW,FIPAddr,iGrade,Result);
	}
	 //NormalizeDecimal(eeprom_read_byte(&DefDecimalTotalMoney),2,Result);
}


void ComposeDatetime(char *Result){// 2010/10/01 16:27:44
	 char sYear[3],sMonth[3],sDay[3];
	 char sHour[3],sMinute[3],sSecond[3];
     
     StrPosCopy(strSystemDate,sDay,0,2);
     StrPosCopy(strSystemDate,sMonth,3,2);
     StrPosCopy(strSystemDate,sYear,6,2);

     StrPosCopy(strSystemTime,sHour,0,2);
     StrPosCopy(strSystemTime,sMinute,3,2);
     StrPosCopy(strSystemTime,sSecond,6,2);
	 sprintf_P(Result,PSTR("20%s/%s/%s %s:%s:%s"),sYear,sMonth,sDay,sHour,sMinute,sSecond);
}


char GetPumpID(char FIPAddr){//Find Stored PumpID in PumpMap
     char PPumpID[8],Result;     
     eeprom_read_block((void*) &PPumpID, (const void*) &DefPumpMap,8);
	 Result=0;
	 if ((FIPAddr>0) && (FIPAddr<8)){
	    Result=PPumpID[FIPAddr-1];
	 }
   return Result;
}

char GetFIPAddr(char iPumpID){//Find FIP Addr based on PumpID respectively: 
char i,PPumpID[8],Result;     //FIP1..FIP8
	 Result=0; 
	 eeprom_read_block((void*) &PPumpID, (const void*) &DefPumpMap,8);
     for(i=0;i<8;i++){
	     if (PPumpID[i]==iPumpID){
		     Result=i+1;
			 break;
		 }
	 }
   return Result;
}

void GeniusSendPumpStatus(char iPumpID){
     
}
void GeniusSendStopPump(char iPumpID){
}
void GeniusSendResumePump(char iPumpID){
}
void GeniusSendPumpPreset(char iPumpID){
}


void GeniusSendLastTransaction(char iPumpID){
     char SVolume[15],SMoney[15];
	 char STotalVolume[15],STotalMoney[15];
	 char SDatetime[20];
     char strSend[90],cSum;
	 char FIPAddr,PProductID[6],iNozzle,iProdID;
     
	 //Init
	 FillChar(SVolume,sizeof(SVolume),0);
	 FillChar(SMoney,sizeof(SMoney),0);
	 FillChar(STotalVolume,sizeof(STotalVolume),0);
	 FillChar(STotalMoney,sizeof(STotalMoney),0);
	      

	 //Generate
	 FIPAddr=GetFIPAddr(iPumpID);
     eeprom_read_block((void*) &PProductID, (const void*) &DefNozzleMap[FIPAddr-1], 6);
	 iNozzle=RecPumpData[iPumpID&0x0F].Grade;
	 iProdID=PProductID[iNozzle];

	 GetProductName(iProdID,strProduct);
	 GetTransactionVolume(iPumpID,SVolume);
	 GetTransactionMoney(iPumpID,SMoney);
	 GetTotalizerVolume(iPumpID,iNozzle,STotalVolume);
	 GetTotalizerMoney(iPumpID,iNozzle,STotalMoney);
	 
     ComposeDatetime(SDatetime);

	 //Compose
	 sprintf_P(strSend,PSTR("T%.2d%.2d%s%s%s%s%s%s:"),iPumpID,iNozzle,strProduct,SDatetime,SVolume,SMoney,STotalVolume,STotalMoney);
	 cSum=SumChecksum(strSend);
	 uart_print(1,0,strSend);
	 FillChar(strSend,sizeof(strSend),0);      
	 sprintf_P(strSend,PSTR("%d"),cSum);
	 uart_print(1,1,strSend);	 
}


void SetIncomingTransStatus(char iPumpID,char xTransStatus){
char iStatus,xMaskA,xMaskB,xMaskC,xPumpID;
	 xPumpID=(iPumpID&0x0F);
     iStatus=IncomingTransaction[xPumpID/8];
	 if (xTransStatus==TS_NEW){
	     iStatus=(iStatus|(1<<(xPumpID%8)));	 
		 IncomingTransaction[xPumpID/8]=iStatus;
	 }else if (xTransStatus==TS_OLD){
		 xMaskA=(0xFE<<(xPumpID%8));
		 xMaskB=(~xMaskA)>>1;
		 xMaskC=xMaskA|xMaskB;
		 iStatus=iStatus&xMaskC;
		 IncomingTransaction[iPumpID/8]=iStatus;
	 }
}
char GetIncomingTransStatus(char iPumpID){
     char xPumpID,Result;
	 xPumpID=(iPumpID&0x0F);
	 Result=TS_NONE;
     if((IncomingTransaction[xPumpID/8]>>(xPumpID%8))==1){
	     Result=TS_NEW;
	 }else
     if((IncomingTransaction[xPumpID/8]>>(xPumpID%8))==0){
	     Result=TS_OLD;
	 }
   return Result;
}

void systemGeniusProtocol(){
static char stGeniusProtocol=gpInitScan;
static char iPumpID;
	 
	 switch(stGeniusProtocol){
	 case gpInitScan:
	      iPumpID=1;
          stGeniusProtocol=gpScanNewTransaction;
	      break;
	 case gpScanNewTransaction:
	      if (GetIncomingTransStatus(PumpID)==TS_NEW){
		      IsTotalizerReceived=False;
		      stGeniusProtocol=gpRequestTotalizer;
		  }
	      break;
     case gpRequestTotalizer:
	      SendPoolingCommand(SC_TOTALIZER,iPumpID);
		  TimSend=0;
		  stGeniusProtocol=gpWaitRequestedTotalizer;
	      break;
     case gpWaitRequestedTotalizer:
	      if ((IsTotalizerReceived==True)&&(AcknoledgePump==iPumpID)){
		      IsTotalizerReceived=False;
			  stGeniusProtocol=gpSendTransactionData;
		  }
	      if (TimSend>TIM_SEND*2)stGeniusProtocol=gpNextPumpScan;
	      break;
     case gpSendTransactionData:
          GeniusSendLastTransaction(iPumpID);
		  SetIncomingTransStatus(iPumpID,TS_OLD);
		  
		  TimSend=0;
          stGeniusProtocol=gpDelayNextPumpScan;
	      break;
     case gpDelayNextPumpScan:
          if (TimSend>1)stGeniusProtocol=gpNextPumpScan;
	      break;
     case gpNextPumpScan:
          if (iPumpID<=eeprom_read_byte(&DefPoolingPumpMax)){
		      iPumpID++;
              stGeniusProtocol=gpScanNewTransaction;
		  }
	      break;
	 }
}

void SetDispenser(char DispType){
     char CurrentDispenser;
	 char i,DecimalSetting[5];
     CurrentDispenser=eeprom_read_byte(&DefDispenserBrand);
	 if (CurrentDispenser!=DispType){
	     eeprom_write_byte(&DefDispenserBrand,DispType);
	 }
     //Apply Setting for Pump respectively
	 switch(DispType){
	 case ST_GILBARCO:
          DecimalSetting[0]=0;//Price
		  DecimalSetting[1]=3;//Volume
		  DecimalSetting[2]=0;//Money
		  DecimalSetting[3]=2;//TotalVolume
		  DecimalSetting[4]=0;//TotalMoney 
	      break;
	 case ST_WAYNE_DART:
          DecimalSetting[0]=0;//Price
		  DecimalSetting[1]=2;//Volume
		  DecimalSetting[2]=0;//Money
		  DecimalSetting[3]=2;//TotalVolume
		  DecimalSetting[4]=2;//TotalMoney 
	      break;	 
	 }
	 //ApplyChanges
     for (i=0;i<5;i++){
	     if (eeprom_read_byte(&DefDecimalPrice+i)!=DecimalSetting[i]){
		     eeprom_write_byte(&DefDecimalPrice+i,DecimalSetting[i]);
		 }
	 }
}

void MasterReset(){
     char strEEP[50];
	 char xArray[10];
     //Restore All Stored EEPROM data to the Default
     sprintf_P(strEEP,PSTR("  "));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefOperatorName, sizeof(DefOperatorName));

	 xArray[0]=192;	 xArray[1]=168; 	 xArray[2]=16;	 xArray[3]=70; 
	 eeprom_write_block((const void*) &xArray, (void*) &DefClientIP, 4);

	 xArray[0]=192;	 xArray[1]=168; 	 xArray[2]=16;	 xArray[3]=180; 
	 eeprom_write_block((const void*) &xArray, (void*) &DefServerIP, 4);

     eeprom_write_byte(&DefPrinterType,PT_CUSTOM_TG02);

	 eeprom_write_byte(&DefPrintScrollEnd,7);
	 eeprom_write_byte(&DefPrintScrollSpace,5);
	 eeprom_write_byte(&DefPrintAutoCut,2);
	 eeprom_write_byte(&DefPrintLogo,0);
	 eeprom_write_byte(&DefPrintSize,3);

	 eeprom_write_byte(&DefConnectionHost,0);
	 eeprom_write_byte(&DefShowDateTime,1);
	 eeprom_write_byte(&DefNotifScreen,1);

	 eeprom_write_byte(&DefDecimalPrice,0);
	 eeprom_write_byte(&DefDecimalVolume,3);
	 eeprom_write_byte(&DefDecimalMoney,0);
	 eeprom_write_byte(&DefDecimalTotalVolume,2);
	 eeprom_write_byte(&DefDecimalTotalMoney,0);
	 eeprom_write_byte(&DefDecimalMark,',');
	 eeprom_write_byte(&DefCurrencyMark,'.');

	 xArray[0]=',';	 xArray[1]='.'; 	 xArray[2]=' ';	 xArray[3]='/';  xArray[4]='-'; 
	 eeprom_write_block((const void*) &xArray, (void*) &DefMarkMap, 5);

	 xArray[0]=1;	 xArray[1]=2; 	 xArray[2]=0;	 xArray[3]=0; xArray[4]=0;	 xArray[5]=0; xArray[6]=0;	 	 
	 eeprom_write_block((const void*) &xArray, (void*) &DefPumpMap, 8);
	 eeprom_write_block((const void*) &xArray, (void*) &DefPumpLabel, 8);

	 xArray[0]=1;	 xArray[1]=2; 	 xArray[2]=0;	 xArray[3]=0; xArray[4]=0;	 xArray[5]=0; 
	 eeprom_write_block((const void*) &xArray, (void*) &DefNozzleMap[0], 6);
	 eeprom_write_block((const void*) &xArray, (void*) &DefNozzleMap[1], 6);

	 sprintf_P(strEEP,PSTR("6500"));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefProductPrice[0], sizeof(DefProductPrice[0]));
	 sprintf_P(strEEP,PSTR("7250"));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefProductPrice[1], sizeof(DefProductPrice[1]));
	 sprintf_P(strEEP,PSTR("4500"));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefProductPrice[2], sizeof(DefProductPrice[2]));
	 sprintf_P(strEEP,PSTR("6500"));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefProductPrice[3], sizeof(DefProductPrice[3]));
     sprintf_P(strEEP,PSTR("4500"));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefProductPrice[4], sizeof(DefProductPrice[4]));
	 sprintf_P(strEEP,PSTR("6500"));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefProductPrice[5], sizeof(DefProductPrice[5]));

	 sprintf_P(strEEP,PSTR("Pert+   "));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefProductName[0], sizeof(DefProductName[0]));
	 sprintf_P(strEEP,PSTR("Pertamax"));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefProductName[1], sizeof(DefProductName[1]));
     sprintf_P(strEEP,PSTR("Premium "));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefProductName[2], sizeof(DefProductName[2]));
     sprintf_P(strEEP,PSTR("Solar   "));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefProductName[3], sizeof(DefProductName[3]));
     sprintf_P(strEEP,PSTR("BioSolr "));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefProductName[4], sizeof(DefProductName[4]));
     sprintf_P(strEEP,PSTR("Diesel  "));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefProductName[5], sizeof(DefProductName[5]));

	 sprintf_P(strEEP,PSTR("BCA"));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefBankName[0], sizeof(DefBankName[0]));
	 sprintf_P(strEEP,PSTR("Mandiri"));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefBankName[1], sizeof(DefBankName[1]));
	 sprintf_P(strEEP,PSTR("BNI"));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefBankName[2], sizeof(DefBankName[2]));
	 sprintf_P(strEEP,PSTR("BRI"));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefBankName[3], sizeof(DefBankName[3]));

	 eeprom_write_byte(&DefPrintInitialize,False);
	 eeprom_write_byte(&DefInitIFT,IT_SLAVE);
	 eeprom_write_byte(&DefDispenserBrand,ST_GILBARCO);//ST_WAYNE_DART;

	 sprintf_P(strEEP,PSTR("000000"));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefTransactionNumber, sizeof(DefTransactionNumber));

     eeprom_write_byte(&DefShift,1);

	 xArray[0]=br9600;	 xArray[1]=br9600; 	 xArray[2]=br5787;	 xArray[3]=br9600;
	 eeprom_write_block((const void*) &xArray, (void*) &DefBaudrate, 4);

     eeprom_write_byte(&DefPoolingPumpMax,MAX_PUMP);
     eeprom_write_byte(&DefPoolingNoPumpCount,NO_PUMP_COUNT_MAX);
     eeprom_write_byte(&DefPoolingTryResend,TRY_RESEND);
     eeprom_write_byte(&DefPoolingSendTimeout,SEND_TIMEOUT);
     eeprom_write_byte(&DefPoolingDelayNextPump,DELAY_NEXT_PUMP);
     eeprom_write_byte(&DefActivePump,ACTIVE_PUMP);
     eeprom_write_byte(&DefSequenceTimeout,SEQUENCE_TIMEOUT);

     eeprom_write_byte(&DefPrintMoney,True);
	 eeprom_write_byte(&DefHGMode,HM_232);//HM_TTL,HM_232,HM_485

	 sprintf_P(strEEP,PSTR("    PT. HANINDO AUTOMATION SOLUTIONS    "));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefHeaderFooter[0],41);
	 sprintf_P(strEEP,PSTR("        JL. RS Fatmawati No.55          "));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefHeaderFooter[1],41);
	 sprintf_P(strEEP,PSTR("            Jakarta Selatan             "));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefHeaderFooter[2],41);
	 sprintf_P(strEEP,PSTR("                                        "));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefHeaderFooter[3],41);
	 sprintf_P(strEEP,PSTR("                                        "));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefHeaderFooter[4],41);
	 sprintf_P(strEEP,PSTR("                                        "));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefHeaderFooter[5],41);

	 sprintf_P(strEEP,PSTR("             Terima Kasih               "));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefHeaderFooter[6],41);
	 sprintf_P(strEEP,PSTR("            Selamat  Jalan              "));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefHeaderFooter[7],41);
	 sprintf_P(strEEP,PSTR("      Semoga Selamat Sampai Tujuan      "));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefHeaderFooter[8],41);
	 sprintf_P(strEEP,PSTR("                                        "));
	 eeprom_write_block((const void*)&strEEP, (void*) &DefHeaderFooter[9],41);
	 
	 eeprom_write_byte(&DefIFT_ID,1);
}

//----------------MMC--------------


