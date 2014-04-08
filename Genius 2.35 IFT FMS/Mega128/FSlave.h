#include "CommonUsed.h"


enum stateLoyaltyMenu{mlInit,mlLoyaltySelect,mlUpdateLoyalty,mlSelectFIP,mlShowEnquiry,mlInputRFID,
                      mlShowProsesRFID,mlSendMessage22,mlSendMessage24,mlWaitReply,mlDisplayFreeMessage,
					  mlDispEnquiry,mlNoConnection,mlPressAnyKey,mlDelayExitLoyalty,mlExitLoyalty};

//SlaveData
//Message System
char IsMessage00=False,IsMessage03,IsMessage99=False,IsMessage11=False,IsMessage21=False,IsMessage23=False;
char IsMessage09=False,IsMessage10=False,IsMessage57=False,IsMessage81=False,IsMessage91=False;

//Msg09
char strFreeMessageLine1[22],strFreeMessageLine2[22],strFreeMessageLine3[22],strFreeMessageLine4[22];


//Msg03
char strCardType[16],strAprovalCode[7],strInvoiceNumber[11],strApprovalCode[7],strDateTime[20];
//Msg32
char strPaymentType[3],strRef1[21],strRef2[21],strRef3[21],strRef4[21];
char strVoucherNum[21];
//Msg57 
char NozzleID,CardType;
char strCompName[21],strLicPlate[11];
//Msg58  
char strBalanceValue[14],strOdometer[10];
char strNozzle[3],strPresetType[2];
//MSG90
char EDCType;
//MSG91
char StatusEDC=0,strStatus[2],strSurcharge[10];
//MSG92
//char strInvoiceNumber[11];
char SeqNum=0;

char strIFT_ID[3],strSeqNum[3]; 
char strClientIP[17],strServerIP[17];

char strBalanceType[2];

//[LocalAccount]
//FIP Fueling Status
char nLocalAccount=0;
char LocalAccountFIP[5];

void sendMessage10();
void sendMessage22();
void sendMessage24();
void sendMessage28();
void sendMessage32();
void sendMessage56();
void sendMessage58();
void sendMessage90();
void sendMessage98(char PumpID);
void sendMessage92();
void sendMessage94();

char procMessage00();
char procMessage09();
void procMessage11();
char procMessage21();
char procMessage23();
char procMessage81();
char procMessage57();
void ProcMessage91();
char procMessage99();

char FMenuLocalAccount();
char FMenuChangeMOP();
char FMenuLoyalty();
char FMenuEDCTransaction();
char FViewFreeMessage();
