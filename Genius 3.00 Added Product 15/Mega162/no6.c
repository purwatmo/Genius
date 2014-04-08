/*
Mega162.c ->Pump Pooling Controller 
          Gilbarco,WayneDart
		  Last Updated 02/01/2011
*/

//DEBUG MODE ONLY/*

//#define DEBUG_WAYNE_RECEIVE
#define DEBUG_WAYNE_POOLING
//#define DEBUG_GILBARCO_POOLING
//#define DEBUG_GILBARCO_PUMP_CMD
//#define DEBUG_CMD_TERM
//#define DEBUG_GILBARCO_RESPONSE
//#define DEBUG_PUMPID
//#define DEBUG_WAYNE_STATE
//#define DEBUG_TRANS_FLOW
//#define DEBUG_TOTAL_FLOW 
//#define DEBUG_PUMP_STATUS_FLOW 


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

#include "lcdn.h"
#include "SPI.h"
#include "UART.h"
#include "no6.h"

volatile char IFType=IT_SLAVE;
volatile char StandaloneType=ST_GILBARCO;
char DigitSize=DS_6DIGIT;

char CMDResponse=0,PumpID=0;
char IsStatusReceived=False,IsMoneyReceived=False,IsTransaction=False;
char IsRequestTransInfo[16];
char IsTotalizer=False,IsRequestTotalizerInfo[16];

char IsStopPoolSequence=False;

volatile char IsRestartPooling=True,IsTotalizerACK=False,PumpAck;
char NoPumpCount[16];
char PumpLock[16];

char stReceiveCom0=rcIdle;

//system Serial
//char RxBuffer0[50]
char rxBufferLength=0;
//Status                  
char PumpStatus[16];
char zPumpStatus[16];

//PumpData Rx
char rxPumpId,NozzleId,GradeId,strVolume[10],strAmount[10],strUnitPrice[10],strCurrentMoney[10];
char TGradeId;

//Pooling Pump Data
char iPoolingID=0,txPumpID=0,StatusPump,iSend,nSend,PumpAddr=1,nNoPump,ScanPumpMax,IsNoPump=True,TPoolTimeout,TDelayNextPump;

//Wayne
char CommandSeqID[8],WayneRxSequence=0x00,MsgInfo=MI_NONE;
volatile char WayneReply=WR_NONE;
char WayneRxPumpID=0,ActiveNozzle=0x00;
char IsPacketReceived=False,PacketInfo=PV_NONE;
char xCRC[2],rxNozzle;
char MaxAuthVolume[4]={0x99,0x99,0x99,0x99};

char WayneRxBuffer[80];

struct TTotalizer{
       struct TGrade{
              char strVolume[15];
	          char strMoney[15];
       }TotalGrade[6];       
}PumpTotalizer; 

unsigned int TimExp=0;
char strSend[20];

char CMDResponse;
unsigned int TimSend,TimReceive,TimDelayNextPump=0;

char Com0ReceiveCount=0;
char Com0Buffer[50];
char zDataBuffer[12];

char IsRFID=0,strRFID[9];
char strSerial[20],strSerial1[20];

char BlinkRate;
char MaxPumpScanned=0;

//Configuration
char EEMEM DefIFType=IT_STANDALONE;
char EEMEM DefStandaloneType=ST_GILBARCO;//ST_WAYNE_DART;//
char EEMEM DefDigitSize=DS_6DIGIT;
char EEMEM DefPumpMaxCount=3;
char EEMEM DefTerminalDebug=True;

char EEMEM DefBaudrate[2]={br9600,br9600};
char EEMEM DefMaxPumpPooling=MAX_PUMP;//4
char EEMEM DefNoPumpCountMax=NO_PUMP_COUNT_MAX;//3
char EEMEM DefSendCount=TRY_RESEND;
char EEMEM DefPoolTimeout=SEND_TIMEOUT; 
char EEMEM DefDelayNextPump=DELAY_NEXT_PUMP;
char EEMEM DefSequenceTimeout=SEQUENCE_TIMEOUT;
char EEMEM DefPumpID[16]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
char EEMEM DefNozzleCount=3;

char EEMEM DefHGMode=HM_232;

char SequencePool=0;//LivePooling Detection
char TimWatchSequence,SequenceTimeout;
char PumpTransDigit=6,PumpTotalDigit=8;

ISR(TIMER0_OVF_vect) {
    //TCNT1H=0xFD;//Ov:10ms
	//TCNT1L=0xC0;
    static char TimerExp=0;	     
    TimerExp++;
	if ((TimerExp%BlinkRate)==0){
		PORTE ^= 0x04;
	}
	TimExp++;
	if ((TimExp%2)==0) TimSend++;
	if ((TimExp%100)==0) TimWatchSequence++;
	TimReceive++;
	TimDelayNextPump++;
}

ISR(USART0_RXC_vect){
    char dataRX0;
	char strSend[20];
	dataRX0=UDR0;
    //_uart(1,1,dataRX0);
    //15702 15588 15694

	//_uart(0,1,dataRX0);

	switch(IFType){
	case IT_SLAVE:
	     systemMaster();
	     _spi(dataRX0);

         //_uart(0,1,dataRX0);
		 //_uart(1,1,dataRX0);

	     systemSlave();
	     break;
    case IT_STANDALONE:
	     switch(StandaloneType){
		 case ST_GILBARCO:
		      GilbarcoOnReceive(dataRX0);	
			  break;
		 case ST_WAYNE_DART:
		      WayneOnReceive(dataRX0);	              
			  break;
		 }
	     break;
	}
	//sprintf_P(strSend,PSTR("%.2x"),dataRX0);
			  //_uart_print(1,1,strSend);
	/*

    if ((IsStandAlone==False)||(IFType==IT_SLAVE)){
	    systemMaster();
	    _spi(dataRX0);
	    systemSlave();
		}
     if (IFType==IT_STANDALONE){
	     switch(StandaloneType){
		 case ST_GILBARCO:
		      GilbarcoOnReceive(dataRX0);	
			  break;
		 case ST_WAYNE_DART:
		      WayneOnReceive(dataRX0);	
              //sprintf_P(strSend,PSTR("%.2x"),dataRX0);
			  //_uart_print(1,1,strSend);
			  break;
		 }
     }
	 */		 
}

ISR(USART1_RXC_vect){
	char dataRX1;
    dataRX1=UDR1;


   if (IFType==IT_SLAVE){
	   systemMaster();
	   _spi(dataRX1);
	   systemSlave();
	   }
	   
	//_uart(1,1,dataRX1);
	//OnReceive1(dataRX1);
}

ISR(SPI_STC_vect){
char dataSPI;
     dataSPI=SPDR;
	 //ScanEDCFlow(dataSPI);
	 /*
	 switch(IFType){
	 case IT_SLAVE:
	      _uart(0,1,dataSPI);
	      break;
	 case IT_STANDALONE:
          ScanStandaloneFlow(dataSPI);
	      break;
	 }
	 */
	 
	 if (IFType==IT_SLAVE)//EDC Line Protocol
	    _uart(0,1,dataSPI);
	 ScanStandaloneFlow(dataSPI);	 
}

void main (){
	SystemInit();

   //SystemComLevel(CL_232);
   //_uart_init(0,5787);

 //  SystemComLevel(CL_TTL);
 //  _uart_init(0,4800);

	while(1){	   
	
	   switch(IFType){
	   case IT_SLAVE:
	        break;
	   case IT_STANDALONE:
	        systemAntiFreeze();//15714 15724			
			switch(StandaloneType){
			case ST_GILBARCO:
			     FPoolingPump();
			     break;
            case ST_WAYNE_DART:
                 FPoolingPump2(); 
			     break;			
			}
	        break;
	   }	   
	   //_uart(0,1,'A');
	   //_delay_ms(500);
	}
}

void DoNothing(){

}

void InitMem(){
     //IFType=IT_SLAVE;
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

void ScanStandaloneFlow(char xData){//<STX>[CMD][MSG]<ETX>: 0x05 0x06
static char zFlowData[4];           //MODE CMD--> 0x01:SLAVE        MSG--> 
     char slaveCmd,slaveMsg;        //            0x02:STANDALONE  		 0x00:NONE 0x01:GILBARCO 0x02:TATSUNO 0x03:LG 0x04:WYNE
     char ComPort,ComBaud,i;          //          0x03:DIAGNOSTIC
	 unsigned int brValue;
	 char strSend[20];
     slaveCmd=0;slaveMsg=0;         //            0x04:Totalizer
	 zFlowData[3]=zFlowData[2];     //            0x05:Baudrate
	 zFlowData[2]=zFlowData[1];     //            0x06:DebugTerminal:-->dtOn/dtOff
	 zFlowData[1]=zFlowData[0];
	 zFlowData[0]=xData;
	 //Scan SatndaloneMode Switch Command
	 if((zFlowData[0]==0x06)&(zFlowData[3]==0x05)){
         slaveCmd=zFlowData[2];
         slaveMsg=zFlowData[1];

		 switch(slaveCmd){
		 case SC_SLAVE:		      
		      IFType=IT_SLAVE;
			  BlinkRate=20;
			  eeprom_write_byte(&DefIFType,IFType);
			  #ifdef DEBUG_CMD_TERM 
			  TerminalSendf(1,PSTR("Slave"));		
			  #endif
			  SystemSetSlave();			  		 
		      break;
         case SC_STANDALONE:
		      InitPumpData();		      
		      IFType=IT_STANDALONE;
			  BlinkRate=5;
			  eeprom_write_byte(&DefIFType,IFType);
			  #ifdef DEBUG_CMD_TERM 
			  TerminalSendf(1,PSTR("Standalone"));
			  #endif
			  StandaloneType=eeprom_read_byte(&DefStandaloneType);
              //StandaloneType=ST_WAYNE_DART;
              SystemSetDispenser(StandaloneType);
			  IsRestartPooling=True;
			  //_uart_init(0,GetBaudrate(eeprom_read_byte(&DefBaudrate[0])));
			  //if (slaveMsg<=0x04)StandaloneType=slaveMsg;

		      break;
         case SC_SET_PUMP_TYPE:
		      IFType=IT_STANDALONE;
			  BlinkRate=5;
              StandaloneType=slaveMsg;
		      eeprom_write_byte(&DefStandaloneType,StandaloneType);			  		      
			  sprintf_P(strSend,PSTR("Pump:%d"),StandaloneType);
              SystemSetDispenser(StandaloneType);
			  #ifdef DEBUG_CMD_TERM 
			  _uart_printf(1,1,PSTR("SC_SET_PUMP_TYPE"));
			  _uart_print(1,1,strSend);
			  #endif
			  IsRestartPooling=True;
		      break;
         case SC_TOTALIZER:
		      #ifdef DEBUG_CMD_TERM 
		      _uart_printf(1,1,PSTR("SC_TOTALIZER"));
			  #endif
		      if ((slaveMsg>=1)&&(slaveMsg<=16)){
		           IsRequestTotalizerInfo[(slaveMsg&0x0F)]=True;
				  }
			  else if (slaveMsg==PUMP_ALL){
		           #ifdef DEBUG_CMD_TERM 
				   _uart_printf(1,1,PSTR("PUMPALL"));
				   #endif
				   for (i=1;i<=16;i++)IsRequestTotalizerInfo[(i&0x0F)]=True;
				  }				  	     
		      break; 
         case SC_BAUDRATE:
		      ComPort=(slaveMsg>>4)-3;//3..4
			  ComBaud=(slaveMsg&0x0F);//0,1,2,3
			  brValue=GetBaudrate(ComBaud);
			  eeprom_write_byte(&DefBaudrate[ComPort],ComBaud);
			  #ifdef DEBUG_CMD_TERM 
 			  sprintf_P(strSend,PSTR("COM:%d,%i"),ComPort,brValue);
			  TerminalSend(1,strSend);
			  #endif
			  _uart_init(ComPort,brValue);
		      break;
		 case SC_TRANSACTION:
		          if (slaveMsg<=16)IsRequestTransInfo[slaveMsg&0x0F]=True;
				  else 
				  if (slaveMsg==PUMP_ALL){
				      for(i=1;i<=16;i++){
					      IsRequestTransInfo[i&0x0F]=True;
					  }  
				  }
              #ifdef DEBUG_CMD_TERM 
				 sprintf_P(strSend,PSTR("ReqTrans:%d"),slaveMsg);
				 TerminalSend(1,strSend);
              #endif
		      break;     
         case SC_PUMP_LOCK:
		          if (slaveMsg<=16)PumpLock[slaveMsg&0x0F]=True;
				  else 
				  if (slaveMsg==PUMP_ALL){
				      for(i=1;i<=16;i++){
					      PumpLock[i&0x0F]=True;
					  }  
				  }
               #ifdef DEBUG_CMD_TERM 
				 sprintf_P(strSend,PSTR("PumpLock:%d"),slaveMsg);
				 TerminalSend(1,strSend);
              #endif
		      break;
         case SC_PUMP_UNLOCK:
		          if (slaveMsg<=16)PumpLock[slaveMsg&0x0F]=False;
			   	  else 
			   	  if (slaveMsg==PUMP_ALL){
			   	      for(i=1;i<=16;i++){
			   		      PumpLock[i&0x0F]=False;
			   		  }  
			   	  }
               #ifdef DEBUG_CMD_TERM 
			   	 sprintf_P(strSend,PSTR("PumpUnLock:%d"),slaveMsg);
				 TerminalSend(1,strSend);			 
               #endif
		       break;
          case SC_TOTALIZER_ACK:
		       IsTotalizerACK=True;
			   PumpAck=slaveMsg;
		       break;
		  case SC_SET_POOLING_NO_PUMP_COUNT:
		       #ifdef DEBUG_CMD_TERM 
			   sprintf_P(strSend,PSTR("NoPumpCount:%d"),slaveMsg);
			   TerminalSend(1,strSend);			 
			   #endif
		       eeprom_write_byte(&DefNoPumpCountMax,slaveMsg);
			   IsRestartPooling=True;
		       break;
		  case SC_SET_POOLING_MAX_PUMP:
		       #ifdef DEBUG_CMD_TERM 
			   sprintf_P(strSend,PSTR("MaxPump:%d"),slaveMsg);
			   TerminalSend(1,strSend);			 
			   #endif
		       eeprom_write_byte(&DefMaxPumpPooling,slaveMsg);
			   IsRestartPooling=True;
		       break;
		  case SC_SET_POOLING_SEND:		      
		       #ifdef DEBUG_CMD_TERM 
			   sprintf_P(strSend,PSTR("PoolTrySend:%d"),slaveMsg);
			   TerminalSend(1,strSend);			 		        
			   #endif
		       eeprom_write_byte(&DefSendCount,slaveMsg);
			   IsRestartPooling=True;
		       break;
          case SC_SET_POOLING_TIMEOUT:
		       #ifdef DEBUG_CMD_TERM 
			   sprintf_P(strSend,PSTR("PoolTimeout:%d"),slaveMsg);
			   TerminalSend(1,strSend);			 
			   #endif
		       eeprom_write_byte(&DefPoolTimeout,slaveMsg);
			   IsRestartPooling=True;
		       break;
          case SC_SET_POOLING_DELAY_NEXT_PUMP:
		       #ifdef DEBUG_CMD_TERM 
			   sprintf_P(strSend,PSTR("DelayNextPump:%d"),slaveMsg);
			   TerminalSend(1,strSend);			 
			   #endif
		       eeprom_write_byte(&DefDelayNextPump,slaveMsg);
			   IsRestartPooling=True;
		       break;
  		  case SC_GET_POOLING_NO_PUMP_COUNT:
		       slaveMsg=eeprom_read_byte(&DefNoPumpCountMax);
		       break;
		  case SC_GET_POOLING_MAX_PUMP:
		       slaveMsg=eeprom_read_byte(&DefMaxPumpPooling);
		       break;
		  case SC_GET_POOLING_SEND:		            
		       slaveMsg=eeprom_read_byte(&DefSendCount);
		       break;		 
          case SC_SEQUENCE_TIMEOUT:
		       #ifdef DEBUG_CMD_TERM 
			   sprintf_P(strSend,PSTR("PoolTrySend:%d"),slaveMsg);
			   TerminalSend(1,strSend);			 		        
			   #endif 
		       eeprom_write_byte(&DefSequenceTimeout,slaveMsg);
			   SequenceTimeout=eeprom_read_byte(&DefSequenceTimeout);
			   IsRestartPooling=True;		       
		       break;
          case SC_SET_PUMPID:
		       eeprom_write_byte(&DefPumpID[slaveMsg>>4],(slaveMsg&0x0F));		       
		       #ifdef DEBUG_CMD_TERM 
			   sprintf_P(strSend,PSTR("SC_SET_PUMPID:[%.2X]"),slaveMsg);
			   TerminalSend(1,strSend);			 		        
			   #endif
               IsRestartPooling=True;
               break; 
		  case SC_CLEAR_PUMPID:
               eeprom_write_byte(&DefPumpID[slaveMsg],0);
		       #ifdef DEBUG_CMD_TERM 
			   sprintf_P(strSend,PSTR("SC_CLEAR_PUMPID:[%.2X]"),slaveMsg);
			   TerminalSend(1,strSend);			 		        
			   #endif
               IsRestartPooling=True;
		       break;
          case SC_STOP_POOL_SEQUENCE:
		       IsStopPoolSequence=True;
		       break; 
          case SC_START_POOL_SEQUENCE:
		       IsStopPoolSequence=False;
		       break; 
          case SC_HGM_MODE:
		       eeprom_write_byte(&DefHGMode,slaveMsg);
			   SystemComLevel(eeprom_read_byte(&DefHGMode));
			   
		       break;
		 }
       //AcknoledgeCommand
       SendCommandAcknoledge(slaveCmd,slaveMsg);
	 }//EndIf	 
}

void SendCommandAcknoledge(char AckCommand,char AckData){
	 systemMaster();
	 SendSPI(0x09);
	 SendSPI(AckCommand);
	 SendSPI(AckData);
	 SendSPI(0x0A);
	 systemSlave();
}

void TerminalSendf(char Com,char *strSendf){
   if (eeprom_read_byte(&DefTerminalDebug)){
       while(pgm_read_byte(&(*strSendf)))
	         _uart(Com, 1, pgm_read_byte(&(*strSendf++)));
	}
}

void TerminalSend(char Com,char *strSend){
   if (eeprom_read_byte(&DefTerminalDebug)){
       _uart_print(Com,1,strSend);
	}
}

char GetIFType(){
char Result=IT_NONE;
     

return Result;
}

void SystemSetSlave(){
     SystemComLevel(CL_232);
     //_uart_setting(1,GetBaudrate(eeprom_read_byte(&DefBaudrate[1])),8,P_NONE,1);
	 //_uart_init(0,GetBaudrate(eeprom_read_byte(&DefBaudrate[0])));	

	 _uart_init(0,9600);	
	 _uart_init(1,9600);	
}

//enum eBaudRateValue{brNone,br9600,br19200,br5787};

void SystemSetDispenser(char TDispenserBrand){
     unsigned int brValue;

	 switch(TDispenserBrand){
	 case ST_NONE:
	      //_uart_setting(0,GetBaudrate(eeprom_read_byte(&DefBaudrate[0])),8,P_NONE,1);
		  _uart_init(0,9600);
		  _uart_init(1,9600);
          SystemComLevel(CL_232);
	      break;
	 case ST_GILBARCO:
	      if (eeprom_read_byte(&DefHGMode)==HM_TTL)
		       SystemComLevel(CL_TTL);
		  else SystemComLevel(CL_232);
	      
		  eeprom_write_byte(&DefBaudrate[0],br5787);
		  brValue=GetBaudrate(eeprom_read_byte(&DefBaudrate[0]));
	      //_uart_setting(0,brValue,8,P_NONE,1);
		  
          _uart_init(0,5787);
	      break;
	 case ST_WAYNE_DART:		       
	 	  SystemComLevel(CL_485);
          eeprom_write_byte(&DefBaudrate[0],br9600);
		  brValue=GetBaudrate(eeprom_read_byte(&DefBaudrate[0]));
	      _uart_setting(0,brValue,8,P_ODD,1);
	      break;
	 case ST_TATSUNO:
	 	  SystemComLevel(CL_485);
          eeprom_write_byte(&DefBaudrate[0],br9600);
		  brValue=GetBaudrate(eeprom_read_byte(&DefBaudrate[0]));
	      _uart_setting(0,brValue,8,P_EVEN,1);
	      break;
	 case ST_LG:
	      break;			  
	 }
	 InitPumpData();
}



void SystemComLevel(char ComLevel){
	 sbi(PORTA,0);  // rs232 off
	 sbi(PORTA,3);  // rs485 off
	 switch(ComLevel){
	 case CL_485:
		  //sbi(PORTA,0);  // rs232 off
		  cbi(PORTA,3);  // rs485 on	 
	      break;
     case CL_232:
		  cbi(PORTA,0);  // rs232 on
		  //sbi(PORTA,3);  // rs485 off
	      break;	 
	 }
}

void SystemInit(){
	PORTA=0xFF;	DDRA=0xFF;
	PORTE=0xFF;	DDRE=0xFF;
	DDRB= 0b01001111;//SPI Slave Input Pin-->SCK,MOSI
	PORTB=0b11111111;
	DDRD=0x00;PORTD=0b11111111;
	//Slave 128_SS_High
    sbi(DDRD,2);sbi(PORTD,2);
	_spi_init(0,1);
	InitSystemTimer();
	sei();
	_uart_init(1,GetBaudrate(eeprom_read_byte(&DefBaudrate[1])));
    IFType=eeprom_read_byte(&DefIFType);
    SequenceTimeout=eeprom_read_byte(&DefSequenceTimeout);
	MaxPumpScanned=eeprom_read_byte(&DefPumpMaxCount);		

	switch(IFType){
	case IT_SLAVE:
	     BlinkRate=20;
	     SystemSetSlave();
	     break;
	case IT_STANDALONE:
	     BlinkRate=5;
		 StandaloneType=eeprom_read_byte(&DefStandaloneType);
         SystemSetDispenser(StandaloneType);
	     break;	
	}
	InitPumpData();
	IsStopPoolSequence=False;
	StartupInfo();
	System485(DIR_RX);//ReceiveMode
}

void StatePrintf(char *strState){
     //TerminalSendf(1,strState);
}

//15694 95.8% -> 14746 90.0%

void systemAntiFreeze(){
static char zSequence=0;

	if (IFType==IT_STANDALONE){
	    if (SequencePool>zSequence){
		    TimWatchSequence=0;
		}
        if (TimWatchSequence>SequenceTimeout){
		    TimWatchSequence=0; 
		    //SendAcknoledge(SC_FREEZE,SequenceTimeout);
		    IsRestartPooling=True;
		}
		zSequence=SequencePool;
	}
}

unsigned int CRC_Wayne(unsigned int crc, char a){
    char i;
unsigned int xCRC;
    xCRC=crc;
    xCRC ^= a;
    for (i = 0; i < 8; i++){//++i){
        if (xCRC& 1) xCRC= (xCRC >> 1) ^ 0xA001;//1010 0000 0000 0001
        else xCRC= (xCRC>> 1);
    }
    return xCRC;
}

void sys_delay(unsigned int dV){
}

void System485(char Dir){//DIR_TX, DIR_RX	 
     switch(Dir){
	 case DIR_TX:
	      sbi(PORTB,1);
		  sbi(DDRB,1);
		  _delay_ms(10);
	      break;
	 case DIR_RX:
          _delay_ms(7);
		  sbi(DDRB,1);
		  cbi(PORTB,1);		  
	      break;	 
	 }
}

//enum eLineStatus{LS_NONE,LS_RX,LS_TX};
void WayneSendChar(char xData){
     System485(DIR_TX);//TransmitMode
	 _uart(0,1,xData);
	 System485(DIR_RX);//ReceiveMode
}

void WayneTestSend(){
}

void FWayneSendBuffer(char *Buffer, char nLength){
char i;
     for(i=0;i<nLength;i++){
	     WayneSendChar(Buffer[i]);
	 }
}

void FWayneSendCommand(char Command, char SequenceCmd, char xPumpID, char NozzleID){
char STX_ID,SEQ,MSG_ID[2],NOZZLE_ID,VALUE[6];
int tCRC16;
char strSend[30];
char CmdBuffer[30];
     
	 rxBufferLength=0;
     switch(Command){
	 case CMD_STATUS://5[1] 20 FA
	      /*
	      STX_ID=0x50|(0x0F&(xPumpID));
		  WayneSendChar(STX_ID);
	      WayneSendChar(0x20);
	      WayneSendChar(0xFA);
		  */
		  CmdBuffer[0]=0x50|(0x0F&(xPumpID));
		  CmdBuffer[1]=0x20;
		  CmdBuffer[2]=0xFA;
		  FWayneSendBuffer(CmdBuffer,3);
	      break;
     case CMD_ACK://5[1] C0 FA
	      /*
	      STX_ID=0x50|(0x0F&(xPumpID));
		  SEQ=0xC0|(0x0F&SequenceCmd);
	      WayneSendChar(STX_ID);
	      WayneSendChar(SEQ);
	      WayneSendChar(0xFA);
		  */
		  //sprintf_P(strSend,PSTR("[%.2X %.2X %.2X ]"),STX_ID,SEQ,0xFA);
          //_uart_print(1,1,strSend);
		  CmdBuffer[0]=0x50|(0x0F&(xPumpID));
		  CmdBuffer[1]=0xC0|(0x0F&SequenceCmd);
		  CmdBuffer[2]=0xFA;
		  FWayneSendBuffer(CmdBuffer,3);
	      break;
     case CMD_AUTH_1://[51 31 01 01 05 63 63 03 FA]//Auth1 ->9
          tCRC16=0;
		  CmdBuffer[0]=0x50|(0x0F&xPumpID);tCRC16=CRC_Wayne(tCRC16,CmdBuffer[0]);
		  CmdBuffer[1]=0x30|(0x0F&SequenceCmd);tCRC16=CRC_Wayne(tCRC16,CmdBuffer[1]);
		  CmdBuffer[2]=0x01;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[2]);
		  CmdBuffer[3]=0x01;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[3]);
		  CmdBuffer[4]=0x05;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[4]);
		  CmdBuffer[5]=tCRC16;
		  CmdBuffer[6]=tCRC16>>8;
		  CmdBuffer[7]=0x03;
		  CmdBuffer[8]=0xFA;
          FWayneSendBuffer(CmdBuffer,9);
	      break;
     case CMD_AUTH_2://[51 32 01 01 00 A3 24 03 FA]//Auth2 ->9
          tCRC16=0;
		  CmdBuffer[0]=0x50|(0x0F&xPumpID);tCRC16=CRC_Wayne(tCRC16,CmdBuffer[0]);
		  CmdBuffer[1]=0x30|(0x0F&SequenceCmd);tCRC16=CRC_Wayne(tCRC16,CmdBuffer[1]);
		  CmdBuffer[2]=0x01;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[2]);
		  CmdBuffer[3]=0x01;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[3]);
		  CmdBuffer[4]=0x00;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[4]);
		  CmdBuffer[5]=tCRC16;
		  CmdBuffer[6]=tCRC16>>8;
		  CmdBuffer[7]=0x03;
		  CmdBuffer[8]=0xFA;
          FWayneSendBuffer(CmdBuffer,9);
	      break;
     case CMD_AUTH_3://Preset Auth Max Volume
          tCRC16=0;//[51 33 04 04 99 99 99 99 63 3E 03 FA]//Auth3 ->12
		  CmdBuffer[0]=0x50|(0x0F&xPumpID);tCRC16=CRC_Wayne(tCRC16,CmdBuffer[0]);
		  CmdBuffer[1]=0x30|(0x0F&SequenceCmd);tCRC16=CRC_Wayne(tCRC16,CmdBuffer[1]);
		  CmdBuffer[2]=0x04;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[2]);
		  CmdBuffer[3]=0x04;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[3]);
		  CmdBuffer[4]=MaxAuthVolume[0];tCRC16=CRC_Wayne(tCRC16,CmdBuffer[4]);
		  CmdBuffer[5]=MaxAuthVolume[1];tCRC16=CRC_Wayne(tCRC16,CmdBuffer[5]);
		  CmdBuffer[6]=MaxAuthVolume[2];tCRC16=CRC_Wayne(tCRC16,CmdBuffer[6]);
		  CmdBuffer[7]=MaxAuthVolume[3];tCRC16=CRC_Wayne(tCRC16,CmdBuffer[7]);
		  CmdBuffer[8]=tCRC16;
		  CmdBuffer[9]=tCRC16>>8;
		  CmdBuffer[10]=0x03;
		  CmdBuffer[11]=0xFA;
          FWayneSendBuffer(CmdBuffer,12);
	      break;
     case CMD_AUTH_4://[51 34 02 01 01 01 01 06 5C BF 03 FA]//Auth4 Nozzle1 ->12
	 /*
	 [51 34 02 01 01 01 01 06 5C BF 03 FA]//Auth4 Nozzle1
     [51 34 02 01 02 01 01 06 5C FB 03 FA]//Auth4 Nozzle2

	 */
          tCRC16=0;
		  CmdBuffer[0]=0x50|(0x0F&xPumpID);tCRC16=CRC_Wayne(tCRC16,CmdBuffer[0]);
		  CmdBuffer[1]=0x30|(0x0F&SequenceCmd);tCRC16=CRC_Wayne(tCRC16,CmdBuffer[1]);
		  CmdBuffer[2]=0x02;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[2]);
		  CmdBuffer[3]=0x01;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[3]);
		  CmdBuffer[4]=(NozzleID&0x0F);tCRC16=CRC_Wayne(tCRC16,CmdBuffer[4]);
		  CmdBuffer[5]=0x01;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[5]);
		  CmdBuffer[6]=0x01;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[6]);
		  CmdBuffer[7]=0x06;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[7]);
		  CmdBuffer[8]=tCRC16;
		  CmdBuffer[9]=tCRC16>>8;
		  CmdBuffer[10]=0x03;
		  CmdBuffer[11]=0xFA;
          FWayneSendBuffer(CmdBuffer,12);
	      break;     
     case CMD_TOTALIZER://[51] [37] [08 01] [02] [F2 2B] [03 FA] 
          tCRC16=0;     //[51   30   08 01   01   B3 5E   03 FA] //Total N1
		  CmdBuffer[0]=0x50|(0x0F&xPumpID);tCRC16=CRC_Wayne(tCRC16,CmdBuffer[0]);
		  CmdBuffer[1]=0x30|(0x0F&SequenceCmd);tCRC16=CRC_Wayne(tCRC16,CmdBuffer[1]);
		  CmdBuffer[2]=0x08;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[2]);
		  CmdBuffer[3]=0x01;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[3]);
		  CmdBuffer[4]=(NozzleID&0x0F);tCRC16=CRC_Wayne(tCRC16,CmdBuffer[4]);
		  CmdBuffer[5]=tCRC16;
		  CmdBuffer[6]=tCRC16>>8;
		  CmdBuffer[7]=0x03;
		  CmdBuffer[8]=0xFA;
          FWayneSendBuffer(CmdBuffer,9);
	      break;
     case CMD_TRANSACTION:	
	 /*
  	   [51 30 [01 01] 00 01 01 04 01 01 03 E4 2E [03 FA] //Get Last Status

	 */
          tCRC16=0;
		  CmdBuffer[0]=0x50|(0x0F&xPumpID);tCRC16=CRC_Wayne(tCRC16,CmdBuffer[0]);
		  CmdBuffer[1]=0x30|(0x0F&SequenceCmd);tCRC16=CRC_Wayne(tCRC16,CmdBuffer[1]);
		  CmdBuffer[2]=0x01;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[2]);
		  CmdBuffer[3]=0x01;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[3]);
		  CmdBuffer[4]=0x00;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[4]);
		  CmdBuffer[5]=0x01;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[5]);
		  CmdBuffer[6]=0x01;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[6]);
		  CmdBuffer[7]=0x04;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[7]);
		  CmdBuffer[8]=0x01;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[8]);
		  CmdBuffer[9]=0x01;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[9]);
		  CmdBuffer[10]=0x03;tCRC16=CRC_Wayne(tCRC16,CmdBuffer[10]);
		  CmdBuffer[11]=tCRC16;
		  CmdBuffer[12]=tCRC16>>8;
		  CmdBuffer[13]=0x03;
		  CmdBuffer[14]=0xFA;
          FWayneSendBuffer(CmdBuffer,15);
	      break;
     case CMD_REQ_GLOBAL_STATUS_2:
  	      break;
	 }	 
}

void ExtractValue(char *Source,char FirstPos,char nCount,char*Dest){//WayneRxBuffer,13,16-13,strVolume);
     char i;
	 for(i=0;i<nCount;i++){
	     Dest[2*i]=(Source[FirstPos+i]>>4)+'0';
	     Dest[(2*i)+1]=(Source[FirstPos+i]&0x0F)+'0';	 
	 }Dest[2*nCount]=0;
}

void WayneOnReceive(char WayneDataIn){
     static char MsgID[2],rxValue[10];
     static unsigned int tCRC=0xFFFF;
     char i,strSend[40],rxNozzleID=0;
     static char zReceive=wrWaitSTX;
     static rcvBuffer[11];

		WayneRxBuffer[rxBufferLength]=WayneDataIn;// 03 FA
		if (rxBufferLength<70)
		    rxBufferLength++;
        
		if ((WayneRxBuffer[rxBufferLength-2]==0x03)&&(WayneDataIn==0xFA)){
			WayneRxSequence=WayneRxBuffer[1]&0x0F;
			//Message Identification
			MsgID[0]=WayneRxBuffer[2];
			MsgID[1]=WayneRxBuffer[3];
			#ifdef DEBUG_WAYNE_RECEIVE
			sprintf_P(strSend,PSTR("MSG[%.2X %.2X] L:%d, Seq:%d "),MsgID[0],MsgID[1],rxBufferLength,WayneRxSequence);
			_uart_print(1,1,strSend);
			#endif
			
			/*   0  1  2  3  4  5  6  7  8  9 
			  0 51 3E 01 01 00 03 04 00 46 00 
			  1 02 02 08 00 00 01 54 00 00 70 
			  2 84 03 04 00 46 00 02 09 05 00 
			  3 00 02 00 00 53 66 03 FA 
			*/
            MsgInfo=MI_UNKNOWN;
            if ((MsgID[0]==0x06)&&(MsgID[1]==0x1F)&&(rxBufferLength>=57)){
			     MsgInfo=MI_TOTALIZER;
			     rxNozzleID=(WayneRxBuffer[37]&0x0F);
				 ExtractValue(WayneRxBuffer,38,5,PumpTotalizer.TotalGrade[rxNozzleID-1].strVolume);		
				 RemZeroLead(PumpTotalizer.TotalGrade[rxNozzleID].strVolume);
				 #ifdef DEBUG_WAYNE_RECEIVE
			     sprintf_P(strSend,PSTR("Totalizer%d:[V:%s]"),rxNozzleID,PumpTotalizer.TotalGrade[rxNozzleID].strVolume);
			     _uart_print(1,1,strSend);
				 #endif

			}
            if ((MsgID[0]==0x01)&&(MsgID[1]==0x01)&&(rxBufferLength>=38)){//Last Transaction
			     //Vol:13-16 Amount:17-20
				 MsgInfo=MI_LAST_TRANSACTION;
				 ExtractValue(WayneRxBuffer,13,4,strVolume);
				 ExtractValue(WayneRxBuffer,17,4,strAmount);
				 RemZeroLead(strVolume);
				 RemZeroLead(strAmount);
				 #ifdef DEBUG_WAYNE_RECEIVE
			     sprintf_P(strSend,PSTR("Trans:[V:%s][A:%s]"),strVolume,strAmount);
			     _uart_print(1,1,strSend);
				 #endif

			}   // 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18
			    //51 34 02 08 00 00 01 88 00 00 84 60 01 01 05 38 4E 03 FA 
			if ((MsgID[0]==0x02)&&(MsgID[1]==0x08)&&(rxBufferLength>=19)){//Last Transaction
			     //Vol:13-16 Amount:17-20
				 MsgInfo=MI_LAST_TRANSACTION;
				 ExtractValue(WayneRxBuffer,4,4,strVolume);
				 ExtractValue(WayneRxBuffer,8,4,strAmount);
				 RemZeroLead(strVolume);
				 RemZeroLead(strAmount);
				 #ifdef DEBUG_WAYNE_RECEIVE
			     sprintf_P(strSend,PSTR("Trans:[V:%s][A:%s]"),strVolume,strAmount);
			     _uart_print(1,1,strSend);
				 #endif
			}			

            if ((MsgID[0]==0x03)&&(MsgID[1]==0x04)){//PriceNozzleInfo
			     if ((WayneRxBuffer[7]>>4)==0x01){
				      MsgInfo=MI_NOZZLE_UP;
					  #ifdef DEBUG_WAYNE_RECEIVE 
					  _uart_printf(1,1,PSTR("-WR_MESSAGE Nozzle Up-"));
					  #endif
					  }
                 else
			     if ((WayneRxBuffer[7]>>4)==0x00){
				      //Price
					  ExtractValue(WayneRxBuffer,4,3,strUnitPrice);
					  RemZeroLead(strUnitPrice);
					  #ifdef DEBUG_WAYNE_RECEIVE
			          sprintf_P(strSend,PSTR("Price%s"),strUnitPrice);
			          _uart_print(1,1,strSend);
					  #endif
				      MsgInfo=MI_NOZZLE_DOWN;
					  #ifdef DEBUG_WAYNE_RECEIVE 
					  _uart_printf(1,1,PSTR("-WR_MESSAGE Nozzle Down-"));
					  #endif
					  }
            ActiveNozzle=WayneRxBuffer[7]&0x0F;
            }
        WayneReply=WR_MESSAGE;
		rxBufferLength=0;	
		}else
		if ((WayneRxBuffer[rxBufferLength-2]==0x70)&&(WayneRxBuffer[rxBufferLength-1]==0xFA)){
		
		    #ifdef DEBUG_WAYNE_RECEIVE 
			_uart_printf(1,1,PSTR("-WR_READY-"));
			#endif
		    WayneReply=WR_READY;
		    rxBufferLength=0;
		}
		if (((WayneRxBuffer[rxBufferLength-3]&0xF0)==0x50)&&(WayneRxBuffer[rxBufferLength-1]==0xFA)){
              if ((WayneRxBuffer[rxBufferLength-2]&0xF0)==0xC0){
		         #ifdef DEBUG_WAYNE_RECEIVE 
				 _uart_printf(1,1,PSTR("-WR_ACK-"));
				 #endif
		         WayneReply=WR_ACK;
			 }else if ((WayneRxBuffer[rxBufferLength-2]&0xF0)==0x50){
		         #ifdef DEBUG_WAYNE_RECEIVE 
				 _uart_printf(1,1,PSTR("-WR_NACK-"));
				 #endif
		         WayneReply=WR_NACK;
			 }
             rxBufferLength=0;
		}
}

void FPoolingPump2(){
     static char zPooling,stPoolingPump=pwInit,nPoolTotalizer=0,IsTotalizerBusy=False,iAuthCmd=0;
	 char strSend[30];

	 //Monitoring
	 if (zPooling!=stPoolingPump){
	     zPooling=stPoolingPump;
		 #ifdef DEBUG_WAYNE_POOLING
         sprintf_P(strSend,PSTR("WPool:%d"),stPoolingPump);
	 	 _uart_print(1,1,strSend);
		 #endif
	 }
     //Restarting
	 if (IsRestartPooling==True){
	     IsRestartPooling=False;
		 SendCommandAcknoledge(SC_POOL_RESTARTED,stPoolingPump);
		 _uart_printf(1,1,PSTR("PoolRestarted"));
	     stPoolingPump=pwInit;
	 }
	 switch(stPoolingPump){
	 case pwInit:
		  nNoPump=eeprom_read_byte(&DefNoPumpCountMax);
		  ScanPumpMax=eeprom_read_byte(&DefMaxPumpPooling);
		  nSend=eeprom_read_byte(&DefSendCount);
		  nPoolTotalizer=eeprom_read_byte(&DefNozzleCount);
		  TPoolTimeout=eeprom_read_byte(&DefPoolTimeout);
		  TDelayNextPump=eeprom_read_byte(&DefDelayNextPump);
		  
		  if (IsStopPoolSequence!=True)
		      SendCommandAcknoledge(SC_LIVE_SEQUENCE,SequencePool);
		  SequencePool++;
		  iPoolingID=0;		  
		  IsTotalizerBusy=False;
		  stPoolingPump=pwInitPumpAddr;
		  break;
     case pwInitPumpAddr:
		  txPumpID=eeprom_read_byte(&DefPumpID[iPoolingID]);
          txPumpID=txPumpID-1; 

          #ifdef DEBUG_PUMPID
	       sprintf_P(strSend,PSTR("TxPumpID:%d"),txPumpID);
		   _uart_print(1,1,strSend);
		  #endif

		  //txPumpID=2-1; 
		  //SendCommandAcknoledge(SC_LIVE_SEQUENCE,SequencePool);
		  SequencePool++;
          iSend=0;		  
	      stPoolingPump=pwScanStatus;
          break;	 
     case pwScanStatus:
	      WayneReply=WR_NONE;
	      FWayneSendCommand(CMD_STATUS,0,txPumpID,0);
		  TimSend=0;
		  stPoolingPump=pwWaitScanReply;
	      break;
     case pwWaitScanReply:
	      //Timeout->NoPump		  
          if (TimSend>TPoolTimeout){
		      iSend++;
			  if (iSend<nSend)stPoolingPump=pwScanStatus;//Retry
			  else
			  if (iSend>=nSend)stPoolingPump=pwNoPump;//Pump Not Available
		  }		  
	      
		  switch(WayneReply){
		  case WR_READY:
			   if ((PumpStatus[txPumpID]==PW_NONE)||(PumpStatus[txPumpID]==PW_DISCONNECT)){
			        IsRequestTransInfo[txPumpID]=False;
			        PumpStatus[txPumpID]=PW_ONLINE;
				  }
               //_uart_printf(1,1,PSTR("WR_READY"));
               stPoolingPump=pwUpdatePumpStatus;
		       break;
          case WR_MESSAGE:
		       stPoolingPump=pwReplyACK;
		       break;
		  }
	      break;
	 case pwNoPump:
	      NoPumpCount[txPumpID&0x0F]++;
		  if (NoPumpCount[txPumpID&0x0F]>nNoPump){
		      NoPumpCount[txPumpID&0x0F]=0;
	          PumpStatus[txPumpID]=PW_DISCONNECT;
              stPoolingPump=pwUpdatePumpStatus;
			  }
          else stPoolingPump=pwInitDelayNextPump;
	      break;
     case pwUpdatePumpStatus:
	      PumpAddr=txPumpID;
	 	  if (PumpStatus[PumpAddr]!=zPumpStatus[PumpAddr]){
		      zPumpStatus[PumpAddr]=PumpStatus[PumpAddr];
		      stPoolingPump=pwSendPumpStatus;
		  }else stPoolingPump=pwNextAction;
	      break;
     case pwSendPumpStatus:
          PumpAddr=txPumpID;
	      SendPumpStatusFlow(txPumpID+1,PumpStatus[PumpAddr]);
		  //sprintf_P(strSend,PSTR("WR:%d MI:%d"),WayneReply,MsgInfo);
		  //_uart_print(1,1,strSend);
	      stPoolingPump=pwNextAction;
	      break;
     case pwNextAction:
	      //Switch Action Based Pump State
		  PumpAddr=txPumpID;
		  switch(PumpStatus[PumpAddr]){
		  case PW_DISCONNECT:
		       switch(WayneReply){
			   case WR_MESSAGE:
			        switch(MsgInfo){
					case MI_NOZZLE_DOWN:
						 IsRequestTransInfo[txPumpID]=True;
						 PumpStatus[txPumpID]=PW_ONLINE;
					     break;
					case MI_NOZZLE_UP:
					     PumpStatus[PumpAddr]=PW_CALL;
						 iAuthCmd=0;
					     break;
					case MI_TOTALIZER:
                         PumpStatus[txPumpID]=PW_ONLINE; 
					     break;
                    default:
					     PumpStatus[txPumpID]=PW_ONLINE;
					     break;
					}stPoolingPump=pwUpdatePumpStatus;
					/*
			        if (MsgInfo==MI_NOZZLE_DOWN){
			            IsRequestTransInfo[txPumpID]=True;
						PumpStatus[txPumpID]=PW_ONLINE;
					}else 
					if (MsgInfo==MI_NOZZLE_UP){
					    PumpStatus[PumpAddr]=PW_CALL;
						iAuthCmd=0;
					}*/				    
					
                    break;
               case WR_NONE:
			   		stPoolingPump=pwInitDelayNextPump;
					break;
			   }
		       break;
          case PW_ONLINE:
		       //sprintf_P(strSend,PSTR("ONLine:%d"),WayneReply);
			   //_uart_print(1,1,strSend);
		       switch(WayneReply){
			   case WR_READY:
			        if (IsRequestTransInfo[txPumpID]==True){
					    iSend=0;
					    stPoolingPump=pwSendTransactionRequest;
					}if (IsRequestTotalizerInfo[txPumpID]==True){
					    iSend=0;
						ActiveNozzle=1;
						stPoolingPump=pwSendTotalizerRequest;
					}else stPoolingPump=pwInitDelayNextPump;

			        break;
			   case WR_MESSAGE:
			        switch(MsgInfo){
					case MI_NOZZLE_UP:
					     PumpStatus[PumpAddr]=PW_CALL;
						 iAuthCmd=0;
						 stPoolingPump=pwUpdatePumpStatus;
					     break;
					case MI_LAST_TRANSACTION:
                         if (IsRequestTransInfo[txPumpID]==True)
						     stPoolingPump=pwSendTransMessage;
                         else stPoolingPump=pwInitDelayNextPump;
					     break;
					case MI_NOZZLE_DOWN:
                         IsRequestTransInfo[txPumpID]=True;
						 stPoolingPump=pwInitDelayNextPump;
					     break;
                    case MI_TOTALIZER:
					     if (ActiveNozzle<4){
						     iSend=0;
						     ActiveNozzle++;
						     stPoolingPump=pwSendTotalizerRequest;
						 }else stPoolingPump=pwSendTotalizerInfo;
					     break;
					}
			        break;
			   case WR_NACK:
			        break;
			   }
		       break;
          case PW_CALL:
		       iSend=0; 
			   stPoolingPump=pwSendAuthorizeCommand;               			   
		       break;
           
          case PW_AUTHORIZED:
               if (WayneReply==WR_READY){
			           stPoolingPump=pwInitDelayNextPump;
			   }else if (WayneReply==WR_MESSAGE){
			       if (MsgInfo==MI_NOZZLE_DOWN){
				       PumpStatus[PumpAddr]=PW_ONLINE;
					   iSend=0;
					   stPoolingPump=pwSendTransactionRequest;
				   }else stPoolingPump=pwScanStatus;
			   }
		       break;		       
          case PW_END_DELIVERY:
		       break;
		  }
	      break;
     //Request last Transaction
	 case pwSendTransactionRequest:	 
	      //sprintf_P(strSend,PSTR("Last Transcation"));
		  //_uart_print(1,1,strSend);
		  FWayneSendCommand(CMD_TRANSACTION,CommandSeqID[txPumpID],txPumpID,ActiveNozzle);          
		  TimSend=0;
          stPoolingPump=pwWaitTransACK;
	      break;
     case pwWaitTransACK:
	 	  //Timeout->NoPump		  
          if (TimSend>TPoolTimeout){
		      stPoolingPump=pwRetrySendTransRequest;
		  }
          //Acknowledge
	      if (WayneReply==WR_ACK){
		      CommandSeqID[txPumpID]++;
			  //stPoolingPump=pwSendTransMessage;
			  stPoolingPump=pwScanStatus;
		  }else if (WayneReply==WR_NACK){
		      CommandSeqID[txPumpID]=0;
              stPoolingPump=pwSendTransactionRequest;
		  }
	      break;
     case pwRetrySendTransRequest:
	      iSend++;
		  if (iSend<nSend)stPoolingPump=pwSendTransactionRequest;
		  else
		  if (iSend>=nSend)stPoolingPump=pwNoPump;
	      break;

     case pwSendTransMessage:
          rxPumpId=txPumpID;
		  NozzleId=ActiveNozzle;
		  GradeId=ActiveNozzle;
		  PumpTransDigit=6;
	      SendTransFlow(txPumpID+1,rxPumpId+1,NozzleId,GradeId,strUnitPrice,strVolume,strAmount,PumpTransDigit);
		  IsRequestTransInfo[txPumpID]=False;
		  //IsRequestTotalizerInfo[txPumpID]=True;
          stPoolingPump=pwInitDelayNextPump;
	      break; 
	 //Totalizer
	 case pwSendTotalizerRequest:
	      FWayneSendCommand(CMD_TOTALIZER,CommandSeqID[txPumpID],txPumpID,ActiveNozzle);          
		  TimSend=0;
          stPoolingPump=pwWaitTotalizerACK;
	      break;
     case pwWaitTotalizerACK:
	       //Timeout->NoPump		  
          if (TimSend>TPoolTimeout){
		      stPoolingPump=pwRetrySendTotalizerRequest;
		  }
          //Acknowledge
	      if (WayneReply==WR_ACK){
		      CommandSeqID[txPumpID]++;
			  stPoolingPump=pwScanStatus;
		  }else if (WayneReply==WR_NACK){
		      CommandSeqID[txPumpID]=0;
              stPoolingPump=pwSendTotalizerRequest;
		  }
	      break;
     case pwRetrySendTotalizerRequest:
	      iSend++;
		  if (iSend<nSend)stPoolingPump=pwSendTotalizerRequest;
		  else
		  if (iSend>=nSend)stPoolingPump=pwNoPump;
	      break;
     case pwSendTotalizerInfo:
	      PumpTotalDigit=10;
	      SendTotalizerFlow(txPumpID+1);
		  IsTotalizerACK=False;//True;//
		  stPoolingPump=pwWaitTotalizerInfoACK;
	      break;
     case pwWaitTotalizerInfoACK:
	      if (IsTotalizerACK==True){//SC_TOTALIZER_ACK
              IsRequestTotalizerInfo[PumpAddr]=False;
	          TimDelayNextPump=0;
		      stPoolingPump=pwInitDelayNextPump;
		  }
	      break;
     //Authorization
     case pwSendAuthorizeCommand:
	      #ifdef DEBUG_WAYNE_STATE 
		  sprintf_P(strSend,PSTR("Authorize%d"),iAuthCmd+1);
		  _uart_print(1,1,strSend);
		  #endif
		  FWayneSendCommand((CMD_AUTH_1+iAuthCmd),CommandSeqID[txPumpID],txPumpID,ActiveNozzle);          
		  TimSend=0;
          stPoolingPump=pwWaitAuthACK;
	      break;
     case pwWaitAuthACK:
	 	  //Timeout->NoPump		  
          if (TimSend>TPoolTimeout){
		      stPoolingPump=pwRetrySendAuthorizeCommand;
		  }		  
	      //Acknowledge
	      if (WayneReply==WR_ACK){
		      iAuthCmd++;
			  if (iAuthCmd>=4){//Complete ACK
			      PumpStatus[PumpAddr]=PW_AUTHORIZED;
				  IsRequestTransInfo[txPumpID]=True;
			      stPoolingPump=pwScanStatus;
			  }else {
			   CommandSeqID[txPumpID]++;
			   stPoolingPump=pwSendAuthorizeCommand;
			   }
		  }else if (WayneReply==WR_NACK){
		      CommandSeqID[txPumpID]=0;
              stPoolingPump=pwRetrySendAuthorizeCommand;
		  }
	      break;
     case pwRetrySendAuthorizeCommand:
	      iSend++;
		  if (iSend<nSend)stPoolingPump=pwSendAuthorizeCommand;
		  else
		  if (iSend>=nSend)stPoolingPump=pwNoPump;
	      break;

	 //GeneralStatus
	 case pwSendGeneralStatus:
	      
	      break;
	 //Price Config	  
	 case pwSendPriceConfig:
	      stPoolingPump=pwWaitACK; 
	      break;
	 case pwWaitACK:
	      PumpStatus[PumpAddr]=PW_PRICE_UPDATED;
		  stPoolingPump=pwUpdatePumpStatus;
          break;	 
	 //Acknowledge--------------
	 case pwReplyACK:
	      FWayneSendCommand(CMD_ACK,WayneRxSequence,txPumpID,0);
		  stPoolingPump=pwScanMessage;
	      break;	 
	 case pwScanMessage:
	      if (PumpStatus[txPumpID]==PW_NONE){
			  PumpStatus[txPumpID]=PW_ONLINE;
		  }		  
	      stPoolingPump=pwUpdatePumpStatus;
	      break;		  
		  		 		  
     //Next Pump----------------
     case pwInitDelayNextPump:
	      TimDelayNextPump=0;
          stPoolingPump=pwDelayNextPump;
	      break;
     case pwDelayNextPump:
	      if (TimDelayNextPump>=TDelayNextPump)//20
		      stPoolingPump=pwNextPump;
	      break;
     case pwNextPump:	      
          iPoolingID++;
	      if (iPoolingID<ScanPumpMax){
			  txPumpID=eeprom_read_byte(&DefPumpID[iPoolingID]);
			  if (txPumpID>0){
			      txPumpID=txPumpID-1;
			      iSend=0;TimSend=0;
			      stPoolingPump=pwScanStatus;
			   }else stPoolingPump=pwNextPump;
		  } else stPoolingPump=pwInit;//pwInitPumpAddr;

          #ifdef DEBUG_PUMPID
	       sprintf_P(strSend,PSTR("TxPumpID:%d"),txPumpID);
		   _uart_print(1,1,strSend);
		  #endif
		  
		  //stPoolingPump=pwInitPumpAddr;
	      break;
	 }
}

//#define DEBUG_GILBARCO_POOLING 

void FPoolingPump(){//DefPoolTimeout,DefDelayNextPump
static char zPooling,stPoolingPump=ppInit;
//static char txPumpID=0,StatusPump,iSend,nSend,PumpAddr=1,nNoPump,ScanPumpMax,IsNoPump=True,TPoolTimeout,TDelayNextPump;
       char CurrentAmount[15];
	   char strSend[20];

     //Monitoring
	 if (zPooling!=stPoolingPump){
	     zPooling=stPoolingPump;
		 #ifdef DEBUG_GILBARCO_POOLING
         sprintf_P(strSend,PSTR("<pl:%.2d>"),stPoolingPump);
	 	 _uart_print(1,1,strSend);
		 #endif
	 }
     
	 if (IsRestartPooling==True){
	     IsRestartPooling=False;
		 SendCommandAcknoledge(SC_POOL_RESTARTED,stPoolingPump);
		 _uart_printf(1,1,PSTR("PoolRestarted"));
	     stPoolingPump=ppInit;
	 }
	 switch(stPoolingPump){
	 case ppInit:
	      //StatePrintf(PSTR("1"));
	      //txPumpID=1;
		  iPoolingID=0;
		  txPumpID=eeprom_read_byte(&DefPumpID[iPoolingID]);

		  iSend=0;
		  TimSend=0;          
		  nNoPump=eeprom_read_byte(&DefNoPumpCountMax);
		  ScanPumpMax=eeprom_read_byte(&DefMaxPumpPooling);
		  nSend=eeprom_read_byte(&DefSendCount);
		  if (IsStopPoolSequence!=True)
              SendCommandAcknoledge(SC_LIVE_SEQUENCE,SequencePool);
          SequencePool++;
		  TPoolTimeout=eeprom_read_byte(&DefPoolTimeout);
		  TDelayNextPump=eeprom_read_byte(&DefDelayNextPump);		  
		  stPoolingPump=ppGetStatus;
	      break;
     case ppNextPump:
          iPoolingID++;
	      if (iPoolingID<ScanPumpMax){
			  txPumpID=eeprom_read_byte(&DefPumpID[iPoolingID]);
			  if (txPumpID>0){			      
			      iSend=0;TimSend=0;
			      stPoolingPump=ppGetStatus;
			   }else stPoolingPump=ppNextPump;
		  }else stPoolingPump=ppInit;
	      break;
     case ppGetStatus:

          #ifdef DEBUG_PUMPID
           sprintf_P(strSend,PSTR("TxPumpID:%d"),txPumpID);
	 	   _uart_print(1,1,strSend);
		  #endif

          //StatePrintf(PSTR("3"));
	      IsStatusReceived=False;
		  IsNoPump=False;
		  SetReceiveLine(rcIdle);
		  stPoolingPump=ppWaitIdle;
     case ppWaitIdle:
          //StatePrintf(PSTR("4"));
	      if (GetReceiveLine()==rcIdle)
		      stPoolingPump=ppSendStatusRequest;
          else stPoolingPump=ppGetStatus;
	      break;
     case ppSendStatusRequest:
          //StatePrintf(PSTR("5"));
	      PumpCommand(txPumpID,CMD_STATUS);
		  TimSend=0;
          stPoolingPump=ppWaitReplyStatus;
	      break;
     case ppNoPump:
          NoPumpCount[txPumpID&0x0F]++;
		  if (NoPumpCount[txPumpID&0x0F]>nNoPump){
		      NoPumpCount[txPumpID&0x0F]=0;
	          IsNoPump=True;
              stPoolingPump=ppUpdatePumpStatus;
			  }
          else stPoolingPump=ppInitDelayNextPump;
	      break;
     case ppWaitReplyStatus:	      
          //StatePrintf(PSTR("7"));
          if (TimSend>TPoolTimeout){//TPoolTimeout
		      iSend++;
		      if (iSend<nSend)stPoolingPump=ppGetStatus;
			  else
		      if (iSend>nSend)stPoolingPump=ppNoPump;//Pump Not Available
		  }
          if (IsStatusReceived==True){
		      if ((txPumpID&0x0F)==PumpID){
			      stPoolingPump=ppUpdatePumpStatus;
			  }else stPoolingPump=ppDifferentPumpID;

			  if ((CMDResponse==0x0F)&&(PumpID==0x0F)&&(iSend<nSend)){
			      stPoolingPump=ppGetStatus;
			  }
		  }
	      break;
     case ppDifferentPumpID:
          //StatePrintf(PSTR("8"));
	      //Dianggap Error NoPump
          stPoolingPump=ppNoPump;
	      break;
		                     //PumpStatus[1..0]= FEDCBA98 76543210
     case ppUpdatePumpStatus://                = 
          //StatePrintf(PSTR("9"));
	      PumpAddr=(txPumpID&0x0F);
		  if (IsNoPump==False)PumpStatus[PumpAddr]=CMDResponse;
		  else 
		  if (IsNoPump==True)PumpStatus[PumpAddr]=PUMP_NONE;
          
		  //SendIfUpdated
		  if (PumpStatus[PumpAddr]!=zPumpStatus[PumpAddr]){
		      zPumpStatus[PumpAddr]=PumpStatus[PumpAddr];
		      stPoolingPump=ppSendPumpStatus;
		  }else stPoolingPump=ppScanResponse;
          break;
     case ppSendPumpStatus://UpdatePumpStatusToMaster;
          //StatePrintf(PSTR("10"));
	      PumpAddr=(txPumpID&0x0F);
	      SendPumpStatusFlow(txPumpID,PumpStatus[PumpAddr]);
	      stPoolingPump=ppScanResponse;
	      break;
     case ppScanResponse:
          //StatePrintf(PSTR("11"));
		  PumpAddr=(txPumpID&0x0F);
		  switch(CMDResponse){
		  case PUMP_OFF:               
		       if (IsRequestTransInfo[PumpAddr]==True)
			       stPoolingPump=ppRequestTransData;
               else 
		       if (IsRequestTotalizerInfo[PumpAddr]==True){
			        stPoolingPump=ppRequestTotalizerData;
				}	
               else stPoolingPump=ppNextPump;
		       break;
          case PUMP_CALL:
 			   if (PumpLock[PumpAddr]==False){
			   	 //  if (IsRequestTransInfo[PumpAddr]==False) 
				 //      IsRequestTransInfo[PumpAddr]=True;
			       stPoolingPump=ppReplyAuth;               
			   } 
		       break;

          case PUMP_PEOT://Endeavor 776 terbaru
		  case PUMP_FEOT:
		       if (IsRequestTransInfo[PumpAddr]==True)
			       stPoolingPump=ppRequestTransData;
               else stPoolingPump=ppInitDelayNextPump;
		       break;
		  case PUMP_BUSY:
               if (IsRequestTransInfo[PumpAddr]==False)
			       IsRequestTransInfo[PumpAddr]=True;
		       //stPoolingPump=ppRequestRealTimeMoney;
			   stPoolingPump=ppInitDelayNextPump;
		       break;
		  case PUMP_AUTH:
               stPoolingPump=ppInitDelayNextPump;
		       break;		  
          default:
               stPoolingPump=ppInitDelayNextPump;
		       break;
		  }
		  //Existing Pump
		  if (IsNoPump==True){
		      IsNoPump=False;
			  stPoolingPump=ppInitDelayNextPump;
		  }
	      break;
     case ppReplyAuth:
          //StatePrintf(PSTR("11"));
	      SetReceiveLine(rcIdle);
	      if (GetReceiveLine()==rcIdle){
			  iSend=0;
		      stPoolingPump=ppSendAuthorize;
			  }
		  else stPoolingPump=ppReplyAuth;
	      break;
     case ppSendAuthorize:
          //StatePrintf(PSTR("12"));
	      TimSend=0;
		  IsStatusReceived=False;
	      PumpCommand(txPumpID,CMD_AUTHORIZE);
		  stPoolingPump=ppWaitAuthorized;
	      break;
     case ppWaitAuthorized:
          //StatePrintf(PSTR("13"));
	      if (TimSend>TPoolTimeout){
		  	  iSend++;
		      if (iSend<nSend)stPoolingPump=ppSendAuthorize;
			  else
		      if (iSend>nSend)stPoolingPump=ppNoPump;//Pump Not Available
		  }
	      if (IsStatusReceived==True){
		      IsStatusReceived=False;
			  //Authorized
			  _uart_printf(1,1,PSTR("Authorized"));

		      if ((txPumpID&0x0F)==PumpID)stPoolingPump=ppUpdatePumpStatus;
			  else stPoolingPump=ppDifferentPumpID;
		  }
	      break;
     case ppRequestRealTimeMoney:
          //StatePrintf(PSTR("14"));
		  Com0ReceiveCount=0;
	      SetReceiveLine(rcRealTimeMoney);
	      if (GetReceiveLine()==rcRealTimeMoney){
			  iSend=0;
		      stPoolingPump=ppSendMoneyReq;
			  }
		  else stPoolingPump=ppRequestRealTimeMoney;
	      break;
     case ppSendMoneyReq:
          //StatePrintf(PSTR("15"));
	 	  TimSend=0;
	      IsMoneyReceived=False;
	      PumpCommand(txPumpID,CMD_REALTIME_MONEY);
		  stPoolingPump=ppWaitMoneyReq;
	      break;
     case ppWaitMoneyReq:
          //StatePrintf(PSTR("16"));
	      if (IsMoneyReceived==False){
		  	  if (TimSend>TPoolTimeout){
			      iSend++;
		          if (iSend<nSend)stPoolingPump=ppSendMoneyReq;
			      else
		          if (iSend>nSend)stPoolingPump=ppNoPump;//Pump Not Available
			  }
		  }else
	      if (IsMoneyReceived==True){
		      IsMoneyReceived=False;
			  PumpAddr=(txPumpID&0x0F);
			  //SendCurrentMoney(txPumpID,strCurrentMoney);
			  stPoolingPump=ppMoneyRequestCompleted;
		  }
	      break;
     case ppMoneyRequestCompleted:
          //StatePrintf(PSTR("17"));
	      TimDelayNextPump=0;
		  stPoolingPump=ppDelayNextPump;
	      break;    
     case ppInitDelayNextPump:
	      TimDelayNextPump=0;
		  stPoolingPump=ppDelayNextPump;
	      break;
     case ppDelayNextPump:
          //StatePrintf(PSTR("18"));
	      if (TimDelayNextPump>=TDelayNextPump)
		      stPoolingPump=ppNextPump;	      
	      break;
     case ppIsRequestTransInfo:
          PumpAddr=(txPumpID&0x0F);
	      if (IsRequestTransInfo[PumpAddr]!=False){
		      stPoolingPump=ppRequestTransData;		  
		  }else{
		  stPoolingPump=ppIsRequestTotalizerInfo;
		  }
	      break;
     case ppRequestTransData:
          //StatePrintf(PSTR("19"));
	      SetReceiveLine(rcInitTransaction);
	      if (GetReceiveLine()==rcInitTransaction){
			  iSend=0;
		      stPoolingPump=ppSendTransReq;
			  }
		  else stPoolingPump=ppRequestTransData;
	      break;
     case ppSendTransReq:
          //StatePrintf(PSTR("20"));
	 	  TimSend=0;
	      IsTransaction=False;
	      PumpCommand(txPumpID,CMD_TRANSACTION_DATA);		  
		  stPoolingPump=ppWaitTransReq;
	      break;
	 case ppWaitTransReq:
          //StatePrintf(PSTR("21"));
		  if (IsTransaction==True){
		      PumpAddr=(txPumpID&0x0F);
		      stPoolingPump=ppTransRequestCompleted;
			  }
		  if (TimSend>(TPoolTimeout*2)){
		  	  iSend++;
		      if (iSend<nSend)stPoolingPump=ppSendTransReq;
			  else
		      if (iSend>nSend)stPoolingPump=ppDelayNextPump;	      
		  }
	      break;
	 case ppTransRequestCompleted:
          //StatePrintf(PSTR("21"));
		  stPoolingPump=ppSendTransInfo;
	      break;
     case ppSendTransInfo:
	      PumpAddr=(txPumpID&0x0F);	      
		  SendTransFlow(txPumpID,rxPumpId,NozzleId,GradeId,strUnitPrice,strVolume,strAmount,PumpTransDigit);
		  IsRequestTransInfo[PumpAddr]=False;
	      TimDelayNextPump=0;		  
		  stPoolingPump=ppDelayNextPump;
	      break;     
     case ppIsRequestTotalizerInfo:
          //StatePrintf(PSTR("23"));
	      PumpAddr=(txPumpID&0x0F);
	      if (IsRequestTotalizerInfo[PumpAddr]==True){
		      stPoolingPump=ppRequestTotalizerData;		  
		  }else{
		  TimDelayNextPump=0; 
		  stPoolingPump=ppNextPump;
		  }
	      break;

     case ppRequestTotalizerData:
          //StatePrintf(PSTR("24"));
	      SetReceiveLine(rcInitTotalizer);
	      if (GetReceiveLine()==rcInitTotalizer){
			  iSend=0;
		      stPoolingPump=ppSendTotalizerReq;
			  }
		  else stPoolingPump=ppRequestTotalizerData;
	      break;
     case ppSendTotalizerReq:
          //StatePrintf(PSTR("25"));
	 	  TimSend=0;
	      IsTotalizer=False;
	      PumpCommand(txPumpID,CMD_TOTALIZER);
		  stPoolingPump=ppWaitTotalizerReq;
	      break;
	 case ppWaitTotalizerReq:
          //StatePrintf(PSTR("26"));
		  if (IsTotalizer==True){
		  	  IsTotalizer=False;
			  stPoolingPump=ppTotalizerRequestCompleted;
		  }
          //if (TimSend>WAIT_TOTALIZER_TIMEOUT){
		  //    stPoolingPump=ppDelayNextPump;
		  //}
		  if (TimSend>(TPoolTimeout*5)){
		  	  iSend++;
		      if (iSend<nSend)stPoolingPump=ppSendTotalizerReq;
			  else
		      if (iSend>nSend)stPoolingPump=ppDelayNextPump;	      
		  }
	      break;
	 case ppTotalizerRequestCompleted:
          //StatePrintf(PSTR("27"));																																																																																																																																		
		  stPoolingPump=ppSendTotalizerInfo;
	      break;
     case ppSendTotalizerInfo:
          //StatePrintf(PSTR("28"));
	      PumpAddr=(txPumpID&0x0F);
		  SendTotalizerFlow(txPumpID);
		  IsTotalizerACK=False;
		  //IsRequestTotalizerInfo[PumpAddr]=False;
	      //TimDelayNextPump=0;
		  stPoolingPump=ppWaitTotalizerACK;
	      break;
     case ppWaitTotalizerACK:
	      if (IsTotalizerACK==True){//SC_TOTALIZER_ACK
              IsRequestTotalizerInfo[PumpAddr]=False;
	          TimDelayNextPump=0;
		      stPoolingPump=ppDelayNextPump;               
		  }
	      break;
	 }
}

void InitPumpData(){
char i;
char strSerial[20];
     if (StandaloneType==ST_GILBARCO){
	     for(i=0;i<16;i++){
		     IsRequestTransInfo[i]=False;
			 IsRequestTotalizerInfo[i]=False;
			 zPumpStatus[i]=PUMP_NONE;
			 PumpStatus[i]=PUMP_NONE;
			 NoPumpCount[i]=0;
			 PumpLock[i]=False;
		 }
     }else if (StandaloneType==ST_WAYNE_DART){
	     for(i=0;i<16;i++){
		     IsRequestTransInfo[i]=False;
			 IsRequestTotalizerInfo[i]=False;
			 zPumpStatus[i]=PW_NONE;
			 PumpStatus[i]=PW_NONE;
			 NoPumpCount[i]=0;
			 PumpLock[i]=False;
		 }
     }    	 
}

void SendPumpStatusFlow(char xPumpID,char xPumpStatus){//
     char tmpStatus,strSend[15];
     //UpdateStatusPump
	 //<STX>[ID][STA]<ETX>: 0x07[ID-STA]0x08
	 tmpStatus=((xPumpID<<4) | (0x0F & xPumpStatus));
    
	 #ifdef DEBUG_PUMP_STATUS_FLOW 
	  sprintf_P(strSend,PSTR("P:%d,S:%d"),xPumpID,xPumpStatus);
	 _uart_print(1,1,strSend);
	 #endif

	 systemMaster();
     SendSPI(0x07);//STX
     SendSPI(tmpStatus);//xIDPump
	 SendSPI(0x08);//ETX

	 systemSlave();
}

void SendTransFlow(char xPumpID, char rxPumpId,char xNozzleID,char xProductID, char *sUnitPrice, char *sVolume,char * sAmount,char TransDigit){
     char i,xNozzle,xPrd,xPump,strSend[30];

	                         //  [0x50,0x05]  01       01       1        3       004500   00000200  00000000 [0x06,0x60] = 24
	 //Sending FlowSPI_Protocol     <STX>   [MsgID][PumpID][Nozzle][ProductID][UnitPrice][Volume][Amount] <ETX>
	 AddZeroLead(sUnitPrice,6);//         Wyne    01 02 2 0 004600 00000254 00011684
     AddZeroLead(sVolume,8);
     AddZeroLead(sAmount,8);
	 
	 if (xNozzleID>=4)xNozzle=4;
	 else xNozzle=xNozzleID;
	 
	 if (xProductID>=4)xPrd=4;
	 else xPrd=xProductID;

	 if (xPumpID>=16)xPump=16;
	 else xPump=xPumpID;

	 sprintf_P(strSend,PSTR("01%.2d%.1d%.1d%s%s%s"),xPump,xNozzle,xPrd,sUnitPrice,sVolume,sAmount);
	 systemMaster();
	 SendSPI(0x05);//STX
	 SendSPI(0x50);//STX
	 for(i=0;i<strlen(strSend);i++){
	     SendSPI(strSend[i]);
		 #ifdef DEBUG_TRANS_FLOW 
		 _uart(1,1,strSend[i]);
		 #endif
	 }
	 //TransDigit
	 sprintf_P(strSend,PSTR("%d"),TransDigit);
     SendStrSPI(strSend);

	 SendSPI(0x06);//ETX
	 SendSPI(0x60);//ETX
	 systemSlave();
}

void SendStrSPI(char *strSendSPI){
char i;
     for(i=0;i<strlen(strSendSPI);i++){
	    SendSPI(strSendSPI[i]);
        //_uart(1,1,strSendSPI[i]);
	 } 
}

void SendTotalizerFlow(char xPumpID){
char i,xPump,strSend[40];
char strVol[10],strMon[10];
	 //                           [0x50,0x05]  02     01    0000000  0000000  0000000  0000000  0000200  0000000  0000200  0000000  0000000  0000000 [0x06,0x60] = 4+8*2*6=76
	 //Sending FlowSPI_Protocol     <STX>   [MsgID][PumpID][Volume1][Amount1][Volume2][Amount2][Volume3][Amount3][Volume4][Amount4][Volume5][Amount5][Volume6][Amount6]<ETX>
	 if (xPumpID>=16)xPump=16;
	 else xPump=xPumpID;
                                                
	 systemMaster();
	 SendSPI(0x05);//STX
	 SendSPI(0x50);//STX
	 FillChar(strSend,sizeof(strSend),0);
	
	 sprintf_P(strSend,PSTR("02%.2d"),xPump);
     SendStrSPI(strSend);

     for (i=0;i<6;i++){
	      //FixingData V:[00000000] M:[00000000]
		  AddZeroLead(PumpTotalizer.TotalGrade[i].strMoney,12);
		  AddZeroLead(PumpTotalizer.TotalGrade[i].strVolume,12);
		  FillChar(strSend,sizeof(strSend),0);
		  sprintf_P(strSend,PSTR("%s%s"),PumpTotalizer.TotalGrade[i].strVolume,PumpTotalizer.TotalGrade[i].strMoney);
          SendStrSPI(strSend);
		  #ifdef DEBUG_TOTAL_FLOW 
		  _uart_print(1,1,strSend);
		  #endif
	 }
	 //TotalizerDigit
	 sprintf_P(strSend,PSTR("%d"),PumpTotalDigit);
     SendStrSPI(strSend);

	 SendSPI(0x06);//ETX
	 SendSPI(0x60);//ETX
	 systemSlave();
	 _delay_ms(1);//DelayProses
}

void uart_init() {
/*
     UCSR1B=_BV(RXCIE1) | _BV(RXEN1) | _BV(TXEN1);
     UCSR1C=(1<<UCSZ11) | (1<<UCSZ10);
     UBRR1H=0;
     UBRR1L=95;
     UCSR0B=_BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);

     UBRR0H=0;
     UBRR0L=158;// baud 5787
     UCSR0C=(1<<URSEL0) | (1<<UPM01) | (1<<UCSZ01) | (1<<UCSZ00); // UPM01 untuk set even parity
     sei();
	 */
}

void InitSystemTimer(){
	TCCR0 |= (1 << CS02) | (1 << CS00);
	TIMSK |= _BV(TOIE0);
}

char GetPumpID(char data){
     char xPumpID=0;
	 xPumpID=(data&0x0F);
	 return xPumpID;
}

char GetResponse(char data){
     char Response=0;
	 Response=((data&0xF0)>>4);
	 return Response;
}

void PumpCommand(char IDPump, char Command){
     char DataSend;
	 char strSend[20];

	 DataSend=((Command<<4)|(IDPump&0x0F));

	 #ifdef DEBUG_GILBARCO_PUMP_CMD
	 sprintf_P(strSend,PSTR("PumpCmd:%.2X"),DataSend);
	 _uart_print(1,1,strSend);
	 #endif


     _uart(0,1,DataSend);	 
     //_uart(1,1,DataSend);
}

void systemMaster(){
    _spi_init(1, 0);         //SPI Master
	cbi(DDRB,6);sbi(PORTB,6);//MISO Input
    sbi(DDRB,4);sbi(DDRB,5); //MOSI Output
	sbi(DDRB,7);sbi(PORTB,7);//SCK  Output    
	sbi(DDRD,2);cbi(PORTD,2);//128-SS LOW
	//_delay_ms(10);
}


void systemSlave(){
	_spi_init(0, 1);         //SPI Slave
	sbi(DDRB,6);sbi(PORTB,6);//MISO Output
    cbi(DDRB,5);sbi(PORTB,5);//MOSI Input
	cbi(DDRB,7);sbi(PORTB,7);//SCK  Input
	sbi(DDRD,2);sbi(PORTD,2);//128-SS HIGH
	//_delay_ms(10);
}

void SendSPI(char xSPI){
     _spi(xSPI);
	 //_delay_ms(10);
}

void ShiftArray(char *strShifted, unsigned int nCount){

}

void systemServiceSPI(){
}

void StrReverse(char *strSource){// 0054->4500
     char i,Length,strResult[20];
	 Length=strlen(strSource);
	 for(i=0;i<Length;i++){
	    strResult[i]=strSource[Length-i-1];
	 }strResult[Length]=0;
	 //CopyResult
	 for(i=0;i<strlen(strResult);i++){
	    strSource[i]=strResult[i];
	    strSource[i+1]=0;
	 }
}

void SaveTransactionData(char data){
     static char SavePipeline=spNone;
	 static char IdxData=0;
	 switch(SavePipeline){
     case spPumpIdentifier:
	      if (IdxData==1)rxPumpId=FilterBCD(data);
		  else
	      if (IdxData==2)NozzleId=FilterBCD(data);
          IdxData++;
      	  break;
     case spProductGrade:
	      if (IdxData==0)GradeId=FilterBCD(data);
		  IdxData++;
      	  break;
     case spProductPrice:	       
          if (data!=0xF9){		      
		      strUnitPrice[IdxData]=BCD2Char(FilterBCD(data));
              strUnitPrice[IdxData+1]=0;
	          IdxData++;
			  }
	  	  break;
     case spProductVolume:
          if (data!=0xFA){//Maks 7 Digit 0000123
              strVolume[IdxData]=BCD2Char(FilterBCD(data));
              strVolume[IdxData+1]=0;
	          IdxData++;
		      }
	  	  break;
     case spProductMoney:
          if (data!=0xFB){
              strAmount[IdxData]=BCD2Char(FilterBCD(data));
              strAmount[IdxData+1]=0;
              IdxData++;
		      }
	  	  break;
     case spLRC:
	  	  break;	 
	 }

	 //Pipeline detection
	 if (data==0xF8){
	     SavePipeline=spPumpIdentifier;
		 IdxData=0;
	 }else if (data==0xF6){
	     SavePipeline=spProductGrade;
		 IdxData=0;
	 }else if (data==0xF7){
	     SavePipeline=spProductPrice;
		 IdxData=0;
	 }else if (data==0xF9){
	     SavePipeline=spProductVolume;
		 IdxData=0;
	 }else if (data==0xFA){
	     SavePipeline=spProductMoney;
		 IdxData=0;
	 }else if (data==0xFB){
	     PumpTransDigit=IdxData;
	     SavePipeline=spLRC;
		 IdxData=0;
	 }
}

void SaveTotalizerData(char data){
     static char SavePipeline=spNone;
	 static char IdxData=0;

	 switch(SavePipeline){
	 case spTotalizerGrade:
	      if (IdxData==0){
		      TGradeId=FilterBCD(data);
			  //_uart(1,1,BCD2Char(TGradeId));
			  if (TGradeId>=5)TGradeId=5;
			  }
          IdxData++; 
	      break;
	 case spTotalizerVolume:
          if ((data&0xF0)==0xE0){
		      PumpTotalizer.TotalGrade[TGradeId].strVolume[IdxData]=BCD2Char(FilterBCD(data));
			  //_uart(1,1,PumpTotalizer.TotalGrade[TGradeId].strVolume[IdxData]);

		      PumpTotalizer.TotalGrade[TGradeId].strVolume[IdxData+1]=0;
	          //TVolume[TGradeId][IdxData]=BCD2Char(FilterBCD(data));
		      //TVolume[TGradeId][IdxData+1]=0;
		      IdxData++;
		  }
	      break;
	 case spTotalizerMoney:
          if ((data&0xF0)==0xE0){
	          //TMoney[TGradeId][IdxData]=BCD2Char(FilterBCD(data));
		      //TMoney[TGradeId][IdxData+1]=0;
		      PumpTotalizer.TotalGrade[TGradeId].strMoney[IdxData]=BCD2Char(FilterBCD(data));
			  //_uart(1,1,PumpTotalizer.TotalGrade[TGradeId].strMoney[IdxData]);
		      PumpTotalizer.TotalGrade[TGradeId].strMoney[IdxData+1]=0;
  		      IdxData++;
          }
	      break;		  
	 }	 
	//Grade Data Next Select 
	if (data==0xF6){
	    IdxData=0;
	    SavePipeline=spTotalizerGrade;
		//_uart(1,1,0x0D);
		}
    else
	if (data==0xF9){
	    SavePipeline=spTotalizerVolume;
		IdxData=0;
		//_uart(1,1,0x0D);
		//_uart(1,1,'V');
		}
	else
	if (data==0xFA){
	    PumpTotalDigit=IdxData;
	    SavePipeline=spTotalizerMoney;
		IdxData=0;
		//_uart(1,1,0x0D);
		//_uart(1,1,'M');
		}
    else
	if (data==0xF4)SavePipeline=spTotalizerPPU1;
	else
	if (data==0xF5)SavePipeline=spTotalizerPPU2;
	
}

void GilbarcoOnReceive(char data){
char strSend[30],chrData,i;
static char iReceive=0;
    ShiftData(data);
	 
    switch(stReceiveCom0){
	case rcIdle:
	     CMDResponse=GetResponse(data);
		 PumpID=GetPumpID(data);
		 #ifdef DEBUG_GILBARCO_RESPONSE
		 sprintf_P(strSend,PSTR("Response:%.2X"),data);
		 _uart_print(1,1,strSend);
		 #endif
		 IsStatusReceived=True;

	     break;	
	case rcRealTimeMoney:	     
	     strCurrentMoney[Com0ReceiveCount]=BCD2Char(FilterBCD(data));
	     strCurrentMoney[Com0ReceiveCount+1]=0;
		 TimReceive=0;
		 Com0ReceiveCount++;
	     if ((Com0ReceiveCount>6)||(TimReceive>MSG_TRANSACTION_TIMEOUT))
		     IsMoneyReceived=True;
	     break;	
    case rcInitTransaction://<STX>
	     if (data==0xFF){
		    iReceive=0;
			TimReceive=0;
		    stReceiveCom0=rcSaveTransactionMessage;
		 }
	     break;
    case rcInitTotalizer://<STX>
	     if (data==0xFF){
		    iReceive=0;
			TimReceive=0;
		    stReceiveCom0=rcSaveTotalizerMessage;
		 }
	     break;
    case rcSaveTotalizerMessage:
	     //_uart(1,1,data);
	     SaveTotalizerData(data);
	     if ((data==0xF0)||(TimReceive>MSG_TRANSACTION_TIMEOUT)){//Proses
             //ReverseData
			 //_uart_printf(1,1,PSTR("Totalizer: "));
			 for (i=0;i<6;i++){
			 	  StrReverse(PumpTotalizer.TotalGrade[i].strMoney);
				  StrReverse(PumpTotalizer.TotalGrade[i].strVolume);
				  //_uart_print(1,1,PumpTotalizer.TotalGrade[i].strMoney);
				  //_uart_print(1,1,PumpTotalizer.TotalGrade[i].strVolume);
             }
		     stReceiveCom0=rcIdle;
			 IsTotalizer=True;
			 }		 
         break;	
    case rcSaveTransactionMessage:
	     //_uart(1,1,data);
	     SaveTransactionData(data);
	     if ((data==0xF0)||(TimReceive>MSG_TRANSACTION_TIMEOUT)){//Proses
			  StrReverse(strUnitPrice);
			  StrReverse(strVolume);
			  StrReverse(strAmount);

			  //sprintf_P(strSend,PSTR("%s"),strAmount);
			  //_uart_print(1,1,strSend);

              stReceiveCom0=rcIdle;
			  IsTransaction=True;
			 }
	     break;
	}
}

void OnReceive1(char data){
     //char i=0;
	 //Shift Data Serial; 02 32 33 44 31 30 41 36 43 0D 0A 0A 03 
/*
	 for(i=0;i<19;i++){
        strSerial[19-i]=strSerial[18-i];   
	 }
     strSerial[0]=data;
	 //Detection
	 if ((strSerial[0]==0x03)){
	    IsRFID=1;
		for(i=0;i<8;i++) strRFID[i]=strSerial[10-i];
            strRFID[8]=0;
     sprintf_P(strSerial,PSTR("RFID:%s"),strRFID);
     _uart_print(1,0,strSerial);
	 }
	 */
}

void UpdateMoney(char *Dest, char *Src , unsigned int Length){
     unsigned char i;
	 for(i=0;i<Length;i++){
	     Dest[i]=Src[Length-i];
	 }
	 Dest[Length]=0;
}

char FilterBCD(char data){
     char Result;
	 Result=(data&0x0F);
	 return Result;
}

char BCD2Char(char data){
     char Result;
	 Result=0;
	 if ((data>=0)&&(data<=9)) 
	     Result='0'+data;
     else Result='0';
	 return Result;
}

void ShiftData(char data){//data [0]..[10]
     char i;
	 for(i=11;i>0;i--){//0..10
	    zDataBuffer[i]=zDataBuffer[i-1];
	 }
	 zDataBuffer[0]=data;
}

void SetReceiveLine(char RecLine){
     stReceiveCom0=RecLine;
}

char GetReceiveLine(){
     return stReceiveCom0;
}



void StrPosCopy(char *Source, char *Dest,unsigned int IdxSource, unsigned int Length){
unsigned int i;
     for(i=0;i<Length;i++){
	    Dest[i]=Source[IdxSource+i];
	 }Dest[Length]=0;
}

char CharPosCopy(char *Source, unsigned int IdxSource){
     char Result;
          Result=Source[IdxSource];
	 return Result;
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

void AddZeroLead(char *String,unsigned char Size){// 1234 ->0000001234
     char i,Length,strAdded[20];
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

char HexToChar(char xHex){
char Result='0';
     if (xHex<10) Result='0'+xHex;
	 else 
	 if (xHex<16) Result='A'+(xHex-10); 
	 else Result='0';
     return Result;
}

//Copy Reverse Filtered BCD String
void StrPosCopyReverse(char *Source, char *Dest,unsigned int IdxSource, unsigned int Length){
     unsigned int i;
	 for (i=0;i<Length;i++){
	      Dest[i]=BCD2Char(FilterBCD(Source[IdxSource-i]));
	 }Dest[Length]=0;
}


void StartupInfo(){
    _uart_printf(1,1,PSTR("Initialized"));
}

void uartGilbarco(){
}

void FillChar(char *strMemory, unsigned int Length,char data){
     unsigned int i;
	 for (i=0;i<Length;i++){
	     strMemory[i]=data;
	 }
}
