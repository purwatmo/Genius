//Version
#define CODE_NAME "GeNiUs"

#define VERSION_NUM "2.35"
#define VERSION_DATE "14/02/2011"

#define SPV_DEFAULT_PASS "11111"
#define SYS_DEFAULT_PASS "00000"

#define _RETURN			{_uart_print(_COM_PRINTER, 1, "");}
#define _PARTIAL_CUT	{_uart(_COM_PRINTER, 1, 0x1B); _uart(_COM_PRINTER, 1, 0x6D);}
#define _TOTAL_CUT		{_uart(_COM_PRINTER, 1, 0x1B); _uart(_COM_PRINTER, 1, 0x69);}
#define _BOLD_FONT		{_uart(_COM_PRINTER, 1, 0x1B); _uart(_COM_PRINTER, 1, 0x47); \
						_uart(_COM_PRINTER, 1, 0x01);}
#define _REGULAR_FONT	{_uart(_COM_PRINTER, 1, 0x1B); _uart(_COM_PRINTER, 1, 0x47); \
						_uart(_COM_PRINTER, 1, 0x00);}
#define _DOUBLE_FONT	{_uart(_COM_PRINTER, 1, 0x1D); _uart(_COM_PRINTER, 1, 0x21); \
						_uart(_COM_PRINTER, 1, 0x11);}
#define _SINGLE_FONT	{_uart(_COM_PRINTER, 1, 0x1D); _uart(_COM_PRINTER, 1, 0x21); \
						_uart(_COM_PRINTER, 1, 0x01);}

#define _SPACING		{_uart(_COM_PRINTER, 1, 0x1B); _uart(_COM_PRINTER, 1, 0x30);}

#define _MAX_PASS		10
#define _MAX_OPP		18
#define _MAX_TEXT		38
#define _MAX_NAME		15

unsigned char	__timeout;
unsigned char	__timer;
unsigned char	__timercount;

char IsPowerOn;//=False;

#define	_TIMER_RESET	__timercount = 0
#define	_TIMER_ON		__timer = 1
#define	_TIMER_OFF		__timer = 0
#define	_TIMEOUT		0
#define	_TIME_IN 		__timeout = 1
#define	_TIME_OUT		__timeout = 0

void DisplayScreenIdle(void);

void _menu_dec(void);
void _menu_pump(void);
void _menu_host(void);
void _menu_tiket(void);
void _menu_pumpid(void);
void menu_product();
void _menu_printer(void);
void _menu_pumpprod(void);
void _menu_datetime(void);
void _menu_pumpprodinput(unsigned char __select);
void FMenuPumpLabel();


//-------Iyan func----------
//System
void systemSlave();
void systemMaster();
void systemRestart();
char systemForceType();

void DisplayIdle();
void DisplayDateTime();
void DisplayTicker();
void DisplayQueueFIP();
void FBackLight();
void BackLightTrig();



char FMenuSettings();
char FSettingProduct();
char FSettingPump();
char FSettingDec();
char FSettingDatetime();
char FSettingPrinter();
char FSettingHost();
char FSettingOperator();
char FSettingSystem();
char FSettingPumpID();

//Ticket
char FMenuTicket();//PrintTicket

//Main
void FMenuIdle();
char FMenuAuthorization();
char FMenuPassword();

//Slave
/*
char FMenuLocalAccount();
char FMenuChangeMOP();
char FMenuLoyalty();
char FMenuEDCTransaction();
*/

//Stadalone
char FMenuShift();
char FCloseShift(char ShiftType);
char FCloseDay();
char FLockPump();

//MenuAdmin
char FMenuMaster();
char FMenuAdmin();
//MenuSettings:

char FMenuReprint();

void DisplayPumpStatus();



void GeneratePrintInit();
void SetBaudRate(char ComAddr,char brMap);
int GetBaudrate(char brSetting);
void InitComport();
void InitializeConnection();
void UpdateClientIP();
void UpdateServerIP();
void UpdateIFT_ID();
void UpdateSeqNum();
void UpdateCardID();
void FillChar(char *strMemory, unsigned int Length,char data);

char CalcLRC(char xLRC,char DataIn);
void SendEDCMessage();

void ViewCardID();

void system_beep(unsigned int tBeep);
void systemEDC();
void systemPrinting();



void EDCSendByte(char EDCData);

void SendPrint(char xSend,char xSendLead);
void FreePrinting();
void PrintIdle();
void InitPrinter();
void PrintDoubleHeight();
void PrintNormalHeight();

void ShowMessage(char *Message);




char GetMessageID(char *strMessageFlow);//MesgID StandaloneMessageFlow-> ['01']-->[0x01]

char FSubMenuAdmin();
char FMenuAdminSettings();
char FMenuSettingHeader();
char FMenuSettingFooter();
char FMenuSettingPassword();
char FMenuSettingClientIP();
char FMenuSettingServerIP();




char menu_FIP(char *xFIP,char *sFIPUsed);

void zeroIP(unsigned char Val,char *StrResult);

void ScanHiddenKeyFlow(char KeyIn);



char SaveToEEPROM(char *Src,char *Dest,unsigned int Length);

void Tab(char *sTab, char nTab);
void IdentifyMessage(char STX,unsigned int Length);

//String Processing
char SpaceOnly(char *string);
void AddCharLag(char *String,char CharAdded,unsigned char Size);
void AddCharLead(char *String,char CharAdded,unsigned char Size);
void AddZeroLead(char *String,unsigned char Size);
void AddZeroLag(char *String,unsigned char Size);
void AddSpaceLead(char *String,unsigned char Size);
void AddSpaceLag(char *String,unsigned char Size);

void FormatDate(char FmtYear, char *Date);
void FormatCurrency(char *strCurrency);
void FormatVolume(char *strRawVolume);
void FormatMoney(char *strRawMoney);
void FormatPrice(char *strRawPrice);
void FormatDecimal(char *strRawData, char DecimalCfg);
void FormatTotalizerMoney(char *strRawMoney);
void FormatTotalizerVolume(char *strRawVolume);

char SelectMark(char InMark);

void SendPoolingCommand(char plCmd,char plMsg);
void SendSlaveCommand(char SlaveCommand,char SlaveMessage);
void ScanEDCFlow(char data);
void ScanRFIDFlow(char data);
void PstrCopy(char *Dest,char *Source);

void CarriegeReturn();

char UserInput(char TypeUI, char xPos, char yPos,char *strResult, unsigned int MaxValue, char MaxLength);

void TestLocalAccount();
char GetLocAccStatus(char paramMessage57);
void RemSpaceLag(char *Spaced);//Remove Spaced "ABCD    "-->"ABCD"
void RemZeroLead(char *Zeroed);//Remove Zeroed"0001234"-->"1234"
void leadingZero(char Val,char *StrResult);
void RemSpaceLead(char *Spaced);
void RemDecimal(char *strDecimal);
void StrPosCopy(char *Source, char *Dest,unsigned int IdxSource, unsigned int Length);
void StrPosPaste(char *Source, char *Dest,unsigned int IdxSource, unsigned int Length);
char CharPosCopy(char *Source, unsigned int IdxSource);
void StringCopyPos(char *Source,char *Dest,char SrcPos,char Length);

void StrAlignCenter(char *Source, unsigned int Length);

void SaveTotalizerCurrentToLast();
void StoreStandaloneTransData(char *strRawTransData);
void GenerateTransactionNum(char *sTransNumber);

void GetProductPrice(char *sPrice,char xPumpID,char xNozzle);
void GetProductName(char GradeId,char *strProductName);
void SaveTotalProduct(char GradeId, char *strDVolume, char *strDMoney);
void ClearList(char *strList);
void AddList(char GradeId, char *strList);

char RePrintStandalone(char FIPAddr);
char PrintStandalone(char FIPAddr,char IsReprint);



void systemGenerateReport();
void CreateReport(char *strData, char *strPrnBuffer, unsigned int *Pos);
char GetBorderValue(char BoderType);
void InserBorder(char BorderType, char *strPrnBuffer,char nLength,unsigned int *Pos);

char CalcMinus(char A, char B);
char CalcPlus(char A, char B);
char IsZerroAll(char *strZerro);         
void NormalizeOverflow(char *strOverflowed);
void StrCalc(char TOperation, char *strA , char *strB, char *strC);
void NormalizeOverFlow(char *strOverflowed);

void ResetTotalizer(char TAddr);
void StrToRaw(char *Source,char *Dest);
void RawToStr(char *Source,char *Dest);
char CountNoPumpSatus(char *strPumpStatusTotalizer);
char CountTotalizerSatus(char *strPumpStatusTotalizer);

void SetTotalizerData(char TType, char TAddr, char xPumpAddr, char xGradeAddr, char *strValue);
void GetTotalizerData(char TType, char TAddr, char xPumpAddr, char xGradeAddr, char *strValue);

void ScanStandaloneFlow(char data);
void UpdateStandaloneStatus(char xPumpID,char xPumpStatus);

void DisplayStandaloneSequence(char x,char y, char PoolingSequence);

char FSettingPumpPooling();
char Chr(char X);
char Ord(char c);
char IsMoreThan(char *strA, char *strB);
void InitMemory();
void FTestCalculation();
char TestUserInput();

void GetTabSpace(signed char TabLength, char *strTab);

void InitStandalone();

char GetMinusPos(char *strNumber);
char IsMinus(char *strNumber);
void RemoveMinus(char *strNumber);

char FTestChar();

//ConfigProtocol
void ConfigProtocol(char dataIn);
void SendConfigParamater();
void SaveConfigParameter();

//GeniusProtocol
void GetParameter(char *GFlow,char FLength,char *GCmd,char *GPumpID, char *GeniCSum);
char IsGeniusCommand(char GCommand);
char SumChecksum(char *strChecked);
void NormalizeDecimal(char CurrentDecimal,char NewDecimal, char *Result);
void GetTransactionVolume(char iPumpID,char *Result);
void GetTransactionMoney(char iPumpID,char *Result);
void GetTotalizerVolume(char iPumpID,char iGrade,char *Result);
void GetTotalizerMoney(char iPumpID,char iGrade,char *Result);
void ComposeDatetime(char *Result);
char GetFIPAddr(char iPumpID);
char GetPumpID(char FIPAddr);

void GeniusProtocol(char dataIn);
void GeniusSendPumpStatus(char iPumpID);
void GeniusSendStopPump(char iPumpID);
void GeniusSendResumePump(char iPumpID);
void GeniusSendPumpPreset(char iPumpID);
void GeniusSendLastTransaction(char iPumpID);
void SetIncomingTransStatus(char iPumpID,char xTransStatus);
char GetIncomingTransStatus(char iPumpID);
void systemGeniusProtocol();
void systemConfigProtocol();

void FTestRemZero();

void IncValue(char *Value,char MinValue,char MaxValue);
void DecValue(char *Value,char MinValue,char MaxValue);

void CalcSegmen(char *strMain, char cNum, char *strResult);
void CalcMultiply(char *strA,char *strB,char *strC);

//Genius Code Generation
void RemoveChar(char *strSource, char cRem);
char GeniusCalc(char cOption, char valA, char valB);
void GenerateGeniusCode(char *srcDate, char cSeed, char *strDestCode);
char ValidateGeniusCode(char *sDate, char *sGenCode);
char ValidateRestoreCode(char *sKeyStamp, char *sRestoreCode);
void WrapCode(char *strRawCode);
void GenerateKeyStamp(char *sTime, char *sGCode, char *strKeyStamp);

void SetDispenser(char DispType);

void MasterReset();

