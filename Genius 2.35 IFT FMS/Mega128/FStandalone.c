#include "FStandalone.h"
#include "FSystem.h"
#include "FTicket.h"

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
		 //_uart_print(1,1,strSend);
		 if  (TransDigit==8){
			  StrPosCopy(strPMoney,strPMoney,0,strlen(strPMoney)-1);	 
		 }
		 RemZeroLead(strPPU);
		 RemZeroLead(strPVolume);
		 RemZeroLead(strPMoney);
		 
		 //sprintf_P(strSend,PSTR("Money: %s"),strPMoney);
		 //_uart_print(1,1,strSend);


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

			 //_uart_print(1,1,strPVolume);
			 
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
     //_uart_print(0,1,strRawTransData);
}

void StoreStandaloneTotalizerData(char *strRawTransData){//Sending FlowSPI_Protocol <STX>   [MsgID][PumpID][Volume1][Amount1][Volume2][Amount2][Volume3][Amount3][Volume4][Amount4][Volume5][Amount5][Volume6][Amount6]<ETX>
	                                                 //                           [0x50,0x05]  02     01    0000000  0000000  0000000  0000000  0000200  0000000  0000200  0000000  0000000  0000000 [0x06,0x60] = 4+8*2*6=76
     char strPumpID[3],iPumpID,iGrade;
	 char strGVolume[15],strGMoney[15];
	 char strSend[60];
	 int i;
	 char FIPAddr;

	 //for(i=0;i<strlen(strRawTransData);i++){
	 //    _uart(1,1,strRawTransData[i]); 
	 // }

     StrPosCopy(strRawTransData,strPumpID,2,2);
	 //IdIFT(strPumpID);
	 iPumpID=atoi(strPumpID);  
	 
	 FIPAddr=GetFIPAddr(iPumpID);
	 if (FIPAddr>0){
	     FIPAddr=FIPAddr-1;
		 //_uart_printf(1,1,PSTR("Totalizer:"));

		 for (iGrade=1;iGrade<=6;iGrade++){          
			  StrPosCopy(strRawTransData,strGVolume,(4+((iGrade-1)*24)),12);
			  StrPosCopy(strRawTransData,strGMoney,(16+((iGrade-1)*24)),12);
			  
			  SetTotalizerData(TVOLUME,TOTALIZER_NOW,FIPAddr,iGrade,strGVolume);
			  SetTotalizerData(TMONEY,TOTALIZER_NOW,FIPAddr,iGrade,strGMoney);
			  //sprintf_P(strSend,PSTR("Nozzle:%d Volume:%s Money:%s"),iGrade,strGVolume,strGMoney);
			  //_uart_print(0,1,strSend);
		 }	
	}
     SendSlaveCommand(SC_TOTALIZER_ACK,iPumpID);
	 UpdateStandaloneStatus((iPumpID&0x0F),PS_TOTALIZER);	 
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
