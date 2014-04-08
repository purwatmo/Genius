//Version
#define CODE_NAME "GeNiUs"
#define VERSION_NUM "2.49"
#define VERSION_DATE "15/03/2013"


/*

Release 2.49
 Local Account Menu 
       Timeout detection
 modified MAX_PRINTED_CHAR ->42
 Added Printer Logo Configuration
 Print Logo function


Release 2.48:
 Added Printer Configuration
 enum ePrinterType{PT_DEFAULT,PT_CUSTOM_TG02,PT_CUSTOM_CUBE,PT_CUSTOM_ESCPOS};
 
 modified prnlmt
 const unsigned char	__prntlmt[6] PROGMEM = {5, 3, 4, 2, 15,15};
 
 Added Print Init:PT_CUSTOM_ESCPOS



   
Release 2.47:
Standalone
 char PrintStandalone(char FIPAddr,char IsReprint)
 char zstrTransNum[7];
 void PrintUpdatePrintedTransNum(char iPumpID);

Release 2.46:
  - Derived from 2.42 Removed length checking for bank option 

 
Release 2.43:
   Standalone Added 5Digit Unit price for Gilbarco 6 digit
   -> Menu Product->Price-> Entry 5Digit Price will Automatically multiply Unit price and Amount

Release 2.40:
  Bug fix on Msg04 Print Acknowledge: Value didnt sent
  Add Master Reset: to Enter Master Reset, press 5 while restart the Power handle until Master Reset Menu Show.
  Enhanced EEPROM Saving, Genius will not write unless EEPROM value has changed

Release 2.39:
  Modified Reprint Method: Request Data from the Pump to get Latest Transaction

Release 2.38:
  Fixed Bug on LocalAccount Printing
Release 2.37:
 Enhanced Operator Name

Release 2.36:
 Added Operator Function 

Release 2.35:
 Enhanced EDC Function
 Enhance and Bug Fix Setting Header Footer
 
Release 2.34:
  Slave: PrintCash Removed ZeroLead Liter
         Enhanced COM Setting for EDC
  Standalone: Fixed Header Footer upto 40 Character
               
  
Release 2.32:
  Addedd COM3 direction setting: default 232
  
Release 2.31:
-Dynamic PumpID: PumpID 1 to 16


Release 2.30:
-Standalone
 Enhance PumpID Configuration
 Auto Decimal Setting Based on Pump Brand, 
 setting decimal menyesuaikan brand dispenser
 Totalizer Cut type CUBE/SAMSUNG
 
Release 2.29:
Added Features:
- Password Reset to Default '11111' and '00000'
  1.Press: 4264696 from Menu9 (Password Entry) press #
  2.Press '1'
  3.Call Hanindo Service Center to Generate 8 Digit Genius CODE
  4.Enter Genius Code, Press # to Continue
  5.Genius Ticket Printer Will Generate Key Stamp
  6.Tell Hanindo Service Center 8 Digit Key Stamp to Generate Restore Code
  7.Enter Restore Code based on KeyStamp given.
  8.Genius will Restore to Default Password once Restore Code successfully entered.
  
-Slave: Print Cash Bug after Local Account Fixed
-Standalone: Wayne Ticket Printer Added




*/

//IyanDefinition

#define True 1
#define False 0
#define TIMOUT_PRINTING 2
#define TIM_NO_RESPONSE 14
#define MASTER_PASSWORD "4264636"
#define PRN_PAPER_CUT 5

//PoolingPump
#define TRY_RESEND 2
#define SEND_TIMEOUT 5
#define DELAY_NEXT_PUMP 5
#define MAX_PUMP 4
#define NO_PUMP_COUNT_MAX 5
#define ACTIVE_PUMP 4

#define MAX_PRINTED_CHAR 42
#define BORDER_LENGTH (MAX_PRINTED_CHAR-2)

//TotalPerProduct
#define TOTAL_VOLUME_LENGTH 15
#define TOTAL_MONEY_LENGTH  15
#define TOTAL_PRODUCT_ADDR  0

//Slave Message Definition
#define MSG_WAIT_TIMOUT 15000
#define TIM_LOCAL_ACCOUNT 12

#define MSG00_LENGTH 47
#define MSG03_LENGTH 77
#define MSG09_LENGTH 108
#define MSG11_LENGTH 615
#define MSG21_LENGTH 86
#define MSG23_LENGTH 145
#define MSG57_LENGTH 230
#define MSG81_LENGTH 438
#define MSG91_LENGTH 92
#define MSG99_LENGTH 408
#define MSG_NOT_SUCCESFULL_LENGTH 15
#define MSG_NONE 255

//Message Code
#define MSG_04 0x04
#define MSG_10 0x10
#define MSG_22 0x22
#define MSG_24 0x24
#define MSG_28 0x28
#define MSG_32 0x32
#define MSG_56 0x56
#define MSG_58 0x58
#define MSG_90 0x90
#define MSG_92 0x92
#define MSG_94 0x94
#define MSG_98 0x98


//Slave Delay
#define SPI_EDC_DELAY 50

//Print Param
#define PRINT_DELAY 1
#define PRINT_MARGIN 5

#define FILLING_TIMOUT 10

#define TICKER_DELAY 10
#define PRESSED_DELAY 10

#define TIM_DISPLAY 10
#define TIM_BUSY_PRINT 10
#define TIM_SEND 10
#define TIM_FREE_MESSAGE 10
#define SEQUENCE_TIMEOUT 10

//#define VERSION_NUM "2.34"
//#define VERSION_DATE "07/01/2011"
//V2.26
/*

*/

//v2.25

//V2.24


//V.2.23->Enhanced RemzeroLead

enum setClientIP{cipNone,cipInit,cipInputIP,cipExit,cipStoreIPblok};
enum stateLocalAccount{laNone,laInit,laSendID,laWaitMessage57,laConTimout,
                       laProcMessage57,laDispStatus,laDispValid,laDispValidInput,
					   laSelectFIP,laSelectFIPInput,laSelectProduct,laSelectProductInput,
					   laSelectBalanceType,laSelectBalanceTypeInput,laBalanceValue,
					   laBalanceValueInput,laOdometer,laOdometerInput,
					   laDataConfirm,laDataConfirmInput,laProceedTransaction,laViewStatus,
					   laWaitMessage00,laProcMessage00,laSuccessTransaction,laFailedTransaction,
					   laLicensePlate,laDisplayTransaction,laWaitFilling,laMenuTimeout,laDelayExit,laExitLocAcc};

enum FreePrintingState{fpNone,fpInit,fpInitHeader,fpLoadHeader,fpPrintHeader,
                       fpInitMessage,fpInitDuplicate,fpPrintDuplicate,fpPrintMessage,fpInitSpace,fpPrintMargin,
					   fpInitFooter,fpLoadFooter,fpPrintFooter,fpInitScroll,fpScrollPaper,fpPaperCut,fpFinishFreePrinting,
                       fpCheckPrintStatusHeader,fpCheckPrintStatusMessage,fpCheckPrintStatusFooter,fpCheckPrintStatusDuplicate,
					   fpLoadEndLine,fpPrintEndLine,fpCheckPrintEndLine,fpLoadOperatorName,fpPrintOperatorName,fpCheckPrintOperatorName
					   };

enum PrintIdleState{piIdle,piInit,piLoadHeader,piPrintHeader,piInitDuplicate,piPrintDuplicate,piCheckPrintStatusDuplicate,
                    piInitMessage,piFormatingMessage,piLoadMessage,piPrintMessage,piCheckPrintStatusHeader,
                    piCheckPrintStatusMessage,piCheckPrintStatusFooter,piLoadFooter,piPrintFooter,piScrollPaper,
					piInitScroll,piCheckPrintStatusScroll,piPaperCut,piFinishPrintIdle,
					piLoadOperatorName,piPrintOperatorName,piCheckPrintOperatorName
					};

enum stateMenuTicket{mtInit,mtPlatNo,mtInputPlatNo,mtOdometer,mtInputOdometer,mtFIP,mtInputFIP,mtSendMsg98,
                     mtInitWaitMessage99,mtWaitMessage99,mtMessage99Received,mtNoConnection,mtExitMenuTicket};

enum stateChangeMOP{cmInit,cmDisplayMOPOption,cmInputMOP,cmDispInputAccount,cmInputAccount,cmDispInputBankName,
                    cmInputBankName,cmInputVoucher,cmInputPumpTest,cmSelectFIP,cmSelectFIPInput,cmSelectBankName,
					cmSelectBankNameInput,
					cmInputBankSurcharge,cmFlowFIP,cmDispInputVoucher,cmDispInputPumpTest,cmDispBankSurcharge,
					cmDispCardTap,cmRFIDCardInput,cmProsesRFID,cmProsesVoucher,cmGenerateData,
					cmSendMessage32,cmWaitReplyMessage,cmDisplayFreeMessage,cmNoReply,cmDelayMOP,cmExitChangeMOP,
					cmFinishChangeMOP};

enum stateEDCTransaction{etInit,etInputEDC,etSelectFIP,etInitMessage90,etSendingMessage90,etWaitReply,
                         etSuccesEDC,etDisplayFreeMessage,etNoConnection,etDelayExit,etExitEDCTransaction};

enum stateLoyaltyMenu{mlInit,mlLoyaltySelect,mlUpdateLoyalty,mlSelectFIP,mlShowEnquiry,mlInputRFID,
                      mlShowProsesRFID,mlSendMessage22,mlSendMessage24,mlWaitReply,mlDisplayFreeMessage,
					  mlDispEnquiry,mlNoConnection,mlPressAnyKey,mlDelayExitLoyalty,mlExitLoyalty};

enum stateDispIdlePrint{diScan,diWaitNoBusy};

enum MenuResult{MENU_NONE,MENU_DONE,MENU_FAILED,MENU_RUN};
enum stateUserInput{uiInit,uiInput,uiClearDisplay,uiInputDisp,uiFinished};

enum ReplyFIP{FIP_DONE,FIP_NONE,FIP_CANCEL};
enum stateMenuFIP{efInit,efFIPInput,efExitFIPInput};

enum stateUSART{suInitConnection,suInit,suGetUSART0,suClearUSART0,suGetUSART1,suClearUSART1,suCloseConnection};

enum stateMenuAuthorization{maInit,maInputPassword,maMenuAdmin,maMenuMaster,maMenuSettings,maInvalidAuthorization,
                            maDelayExitAuthorization,maExitAuthorization};

enum PasswordResult{MP_NONE,MP_CANCEL,MP_INVALID,MP_VALID_MASTER,MP_VALID_ADMIN,MP_VALID_SYSTEM};
enum stateMenuPassword{mpInitPassword,mpInputPasword,mpProcessPassword,mpExit};


enum stateDisplayTicker{tiRight,tiDelayRight,tiLeft,tiDelayLeft};
enum stateMenuIdle{miInit,miScan,miDisplayProses,miWaitProses,miSendMessage98,miWaitPlease,miNoResponse,
                   miPrintStandalone,miDisplayNoTransaction,miWaitDisplayNoTransaction,
                   miWaitReady,miReady,miRunTicket,miRunAuth,miRunLocalAccount,miRunChangeMOP,
				   miRunEDC,miRunLoyalty,miRunReprint,miRunViewFreeMessage,miProcEDC,miRunTotalizer,miClearTotalizer,miRunTestChar,
				   miTestMsg56};

enum stMenuSetting{msInit,msDisplayPage,msDisplayPage1,msDisplayPage2,msSelection,msSelectionPage1,
                   msSelectionPage2,msMenuSettingNextPage,msMenuSettingBackPage,msMenuSettingProduct,
				   msMenuSettingPump,msMenuSettingDec,msMenuSettingDatetime,msMenuSettingPrinter,
				   msMenuSettingHost,msMenuSettingOperator,msMenuSettingSystem,msMenuSettingPumpPooling,
				   msMenuSettingExit};

enum eSettingSystem{ssInitSettingSystem,ssMenuSelect,ssComSettings,ssBaudrateInput,ssPumpPoolingSettings,ssExitSystemSettings};
				   
enum stateFreeMessage{fmInit,fmDisplayFreeMessage,fmDelayViewMesage,fmFinishFreeMessage};

enum stateReprint{rtInit,rtStandaloneFIP,rtValidPassword,rtInvalidPassword,rtTimDisplayInvalid,rtDisplayNoTransaction,
                  rtStandaloneInputFIP,rtFIP,rtRFID,rtInputRFID,rtSendMessage28,rtWaitReply,rtNoConnection,rtDelayExitReprint,rtExitReprint};

enum stateMenuProduct{mpInitProduct,mpChangeProduct,mpDispPrice,mpIsEdit,mpEditPrice,mpEditProductName,mpExitMenuProduct};

enum TTypeUI{UI_NONE,UI_NUMBER_R,UI_NUMBER_L,UI_ALPHANUM_R,UI_NUM_PASSWORD,UI_ALPHANUM_PASSWORD};
enum TResultUI{USER_NONE,USER_CANCEL,USER_INPUT,USER_OK,USER_ENTRY,USER_NO_DATA,USER_FULL_ENTRY};

enum StatusLocalAccount{LA_NONE,LA_INVALID,LA_VALID,LA_LIMITED};
enum ReplyMsg00{MSG00_NACK,MSG00_ACK,MSG00_CARD_BLOCK,
                MSG00_NO_FIP,MSG00_NO_PRINT,MSG00_STATUS_QUERY,
				MSG00_NEW_PRICE,MSG00_SALE_INFO,MSG00_INVALID_PRODUCT,
				MSG00_SALES_PORT};

enum ReplyMsg09{MSG09_NACK,MSG09_ACK,MSG09_MESSAGE99};

enum ReplyMsg57{MSG57_NONE,MSG57_INVALID,MSG57_VALID,MSG57_LIMITED};
enum ReplyMsg99{MSG99_NONE,MSG99_NOTRANS,MSG57_TRANSACTION_OK};

enum TypeMOP{MOP_CASH,MOP_LOCAL_ACCOUNT,MOP_CREDIT_CARD,MOP_DEBIT_CARD,MOP_STORED_VALUE_CARD,
             MOP_LOYALTY,MOP_LOYALTY_LOCAL_ACCOUNT,MOP_VOUCHER,MOP_PUMP_TEST,MOP_VOID_CARD};

enum TypePayment{PAY_CASH,PAY_ACCOUNT,PAY_BANK,PAY_VOUCHER,PAY_PUMPTEST,PAY_NONE};



enum eStandaloneType{ST_NONE,ST_GILBARCO,ST_WAYNE_DART,ST_TATSUNO,ST_LG};
enum eBaudRateValue{brNone,br9600,br19200,br5787};

enum eGenTransData{GS_NONE,GS_GENERATED};
enum eTransactionStatus{TS_NONE,TS_VOID,TS_NEW,TS_OLD};
enum eIFType{IT_NONE,IT_SLAVE,IT_STANDALONE};

enum eSlaveCommand{SC_NONE,SC_SLAVE,SC_STANDALONE,SC_DIAGNOSTIC,SC_TRANSACTION,SC_TOTALIZER,SC_BAUDRATE,SC_DEBUGTERMINAL,
                   SC_SET_POOLINGPUMP,SC_PUMP_LOCK,SC_PUMP_UNLOCK,SC_TRANSACTION_ACK,SC_TRANSACTION_NACK,SC_TOTALIZER_ACK,SC_TOTALIZER_NACK,
				   SC_GET_POOLING_NO_PUMP_COUNT,SC_GET_POOLING_MAX_PUMP,SC_GET_POOLING_SEND,SC_GET_POOLING_TIMEOUT,SC_GET_POOLING_DELAY_NEXT_PUMP,
				   SC_SET_POOLING_NO_PUMP_COUNT,SC_SET_POOLING_MAX_PUMP,SC_SET_POOLING_SEND,SC_SET_POOLING_TIMEOUT,SC_SET_POOLING_DELAY_NEXT_PUMP,
				   SC_LIVE_SEQUENCE,SC_POOL_RESTARTED,SC_SEQUENCE_TIMEOUT,SC_SET_PUMP_TYPE,
				   SC_SET_PUMPID,SC_CLEAR_PUMPID,SC_STOP_POOL_SEQUENCE,SC_START_POOL_SEQUENCE,
				   SC_HGM_MODE
				   };

                  //  0          1        2        3        4        5        6        7         8         9        10    
enum PumpResponse{PUMP_ERROR,PUMP_ST1,PUMP_ST2,PUMP_ST3,PUMP_ST4,PUMP_ST5,PUMP_OFF,PUMP_CALL,PUMP_AUTH,PUMP_BUSY,
                  PUMP_PEOT,PUMP_FEOT,PUMP_STOP,PUMP_SEND_DATA,PUMP_NONE,PUMP_ERR1,PS_PRINT_READY,
				  PS_PRINTED,PS_NO_DATA,PS_VOID,PS_TOTALIZER,PS_FINISH_TOTALIZER,PS_NONE,
				  PW_NONE,PW_DISCONNECT,PW_ONLINE,PW_PRICE_UPDATED,PW_CALL,PW_AUTHORIZED,PW_END_DELIVERY
				  };

enum ePumpID{PUMP_16,PUMP_1,PUMP_2,PUMP_3,PUMP_4,PUMP_5,PUMP_6,PUMP_7,PUMP_8,PUMP_9,PUMP_10,PUMP_11,PUMP_12,PUMP_13,PUMP_14,
             PUMP_15,PUMP_17,PUMP_18,PUMP_ALL};

enum eTotalizer{TOTALIZER_LAST,TOTALIZER_NOW};
enum eTypeTotalizer{TVOLUME,TMONEY,TNONE};

enum eOperation{TMINUS,TPLUS,TMULTIPLY,TDIVIDE};

enum eMenuAdmin{maInitAdmin,maSelectOptions,maMenuReprint,maMenuAdminConfig,maMenuCloseShift,maMenuCloseDay,maMenuAdminSettings,maExitMenuAdmin};

enum eMenuAdminSettings{asInitMenu,asAdminSettingsOption,asAdminSettingHeader,asAdminSettingFooter,
                        asAdminSettingPassword,asAdminSettingClientIP,asAdminSettingServerIP,asExitAdminSetting};

enum eMenuShift{msInitMenuShift,msSelectShift,msCloseShift,msCloseDay,msLockPump,msExitShift};

enum eSettingDec{sdInitDisplay,sdSelectKey1,sdInitDisplay2,sdSelectKey2,sdTestInput,sdExitSettingDecimal};

enum eGenerateReport{grScanAction,grInitData,grCreateReportHeader,grWaitPrinted1,grGenerateLabel,grWaitLabelPrinted,
                     grGenerateReportData,grNextPump,grCreateReportTotalizer,grWaitPrinted2,grCreateReportFooter,
					 grWaitPrinted3,grFinishGenerateReport};
enum eTestCal{tcInitData,tcInputA,tcDispInputA,tcInputB,tcCalcualte,tcWaitEnter};

enum eBorderType{btNone,btTopLeft,btTopCenter,btTopRight,
                        btMiddleLeft,btMiddleCenter,btMiddleRight,
                        btBottomLeft,btBottomCenter,btBottomRight,
						btVertical,btHorizontal,btNewLine};
enum eCloseShift{csInitCloseShift,csInitLockingPump,csLockPump,csWaitPumpLocked,csSendTotalizerALL,
                 csDisplayPumpStatus,csWaitTotalizerComplete,csNoPumpFound,csGenerateReport,csWaitTotalizerALL,
				 csWaitPrintTotalizerComplete,csFinishCloseShift,csDumpShift};

enum eShiftType{SHIFT_NONE,NEW_SHIFT,CONTINUE_SHIFT};
enum eInitStandalone{isSendType,isWaitAcknoledge1,isSendPumpConfig,isWaitAcknoledge2,isFinishInitStandalone};

enum eSettingPooling{ppInitMenu,ppDisplaySequence,ppPoolingSettingInput,ppWaitPoolingRespond,ppExitSettingPooling};

enum eSettingHeader{shInitHeader,shHeaderSelect,shEditHeader1,shEditHeader2,shSaveHeaderQuestions,shIsAlignCenter,shIsSaveHeader,shSaveHeader,shExitSettingHeader};

enum eGeniusProtocol{gpInitScan,gpScanNewTransaction,gpRequestTotalizer,gpWaitRequestedTotalizer,gpSendTransactionData,gpDelayNextPumpScan,gpNextPumpScan};

enum ePrinterType{PT_DEFAULT,PT_CUSTOM_TG02,PT_CUSTOM_CUBE,PT_CUSTOM_ESCPOS};
enum ePumpType{ptNone,ptInitMenu,ptSelectBrand,ptUpdated};

enum eMenuPumpId{mpInitPumpId,mpDisplayPumpId,mpSelectInput,mpSaveFailed,mpDisplayFailed,
                 mpSavingPumpId,mpSendConfigToSlave,mpDelaySaveConfig,
                 mpWaitSlaveReply,mpDelayExitPumpId,mpExitPumpId};

enum eMenuMaster{mmInitMaster,mmMasterSelect,mmRestorePasswordMenu,mmGeniusCodeEntry,
                 mmIsValidGeniusCode,mmDisplayInvalidCode,mmDelayDisplayInvalidCode,
				 mmDisplayKeyStamp,mmRestoreCodeEntry,mmIsValidRestoreCode,mmDisplayInvalidRestoreCode,
				 mmDelayInvalidRestoreCode,mmRestorePassword,
				 mmDisplaySuccess,mmChangePassword,mmDelayExit,mmExitMaster,
				 mmMasterReset,mmResetCodeEntry,mmIsValidResetCode,mmMasterResetExec,mmSuccesfullReset,mmDisplayInvalidResetCode,
				 mmDelayDisplayInvalidResetCode,mmDelayDisplayComplete};

enum eRestoreValidResult{ RC_NONE,RC_VALID,RC_INVALID};
enum eGeniusCodeValidResult{GC_NONE,GC_VALID,GC_INVALID};
enum eGeniusCalcOption{G_PLUS,G_MULTY,G_MINUS};
enum eDateFormat{DATE_LONG_YEAR,DATE_SHORT_YEAR};

enum eConfigProtocol{cpWaitSend,cpSendingParameter,cpSavingParameter};
enum eConfigCommand{CC_NONE,CC_SAVE_CONFIG,CC_SEND_CONFIG};

enum eHGMode{HM_TTL,HM_232,HM_485};

enum ePasswordType{PT_NONE,PT_SUPERVISOR,PT_ADMINISTRATOR};
enum eSettingOperator{soMenuOption,soMenuOptionInput,soChangeOperatorInit,soOperatorNameInput,soIsSaveOperatorName,
                      soChangePasswordInit,soChangePasswordInput,soOldPasswordDisplay,soOldPasswordEntry,
					  soNewPasswordEntry1,soNewPasswordEntry2,soDispValidatePassword,soValidatePassword,
					  soDisplayInvalidPassword,soDelayDisplaySaved,soExitSettingOperator};

#define SPV_DEFAULT_PASS "11111"
#define SYS_DEFAULT_PASS "00000"


#ifndef	__F_MENU_H__
#define __F_MENU_H__

#define _SLAVE_STATUS	0x20
#define	_PUMP_STATUS	0x21
#define _LAST_TRANSACT	0x22
#define _TOTALIZE		0x23
#define _PUMP_SETTINGS	0x24
#define _PUMP_AUTHORIZE	0x25
#define _PRODUCT_PRIZE	0x26
#define _PUMP_DEF		0xCA
#define _PUMP_START		0xCE

#define _ACK			0x13
#define _NACK			0x27

#define	_SPV_PASS		0
#define	_SYS_PASS		1

#define	CAPS_ON			1
#define	CAPS_OFF		0

#define _COM_PRINTER	0
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
void _scr_pump(void);

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
void FMenuIdle();
char FMenuTicket();
char FMenuLocalAccount();
char FMenuChangeMOP();
char FMenuLoyalty();
char FMenuEDCTransaction();

char FMenuAuthorization();
char FMenuPassword();
//MenuAdmin
char FMenuMaster();
char FMenuAdmin();
//MenuSettings:
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

char FMenuReprint();

void DisplayPumpStatus();

void DisplayIdle();
void DisplayDateTime();
void DisplayTicker();
void DisplayQueueFIP();
void BackLightTrig();

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

void systemSlave();
void systemMaster();
void systemRestart();
char systemForceType();

void EDCSendByte(char EDCData);

void SendPrint(char xSend,char xSendLead);
void FreePrinting();

void PrintIdle();
void InitPrinter();
void PrintDoubleHeight();
void PrintNormalHeight();

//Updated v2.43B
void PrintUpdatePrintedTransNum(char iPumpID);

void ShowMessage(char *Message);

char FViewFreeMessage();


char GetMessageID(char *strMessageFlow);//MesgID StandaloneMessageFlow-> ['01']-->[0x01]

char FSubMenuAdmin();
char FMenuAdminSettings();
char FMenuSettingHeader();
char FMenuSettingFooter();
char FMenuSettingPassword();
char FMenuSettingClientIP();
char FMenuSettingServerIP();

char FMenuShift();
char FCloseShift(char ShiftType);
char FCloseDay();
char FLockPump();


char menu_FIP(char *xFIP,char *sFIPUsed);

void zeroIP(unsigned char Val,char *StrResult);

void ScanHiddenKeyFlow(char KeyIn);

void IFTSendMessage(char MsgCode);

void sendMessage10();
void sendMessage22();
void sendMessage24();
void sendMessage28();
void sendMessage32();
void sendMessage56();
void sendMessage58();
void sendMessage90();
void sendMessage98(char FIPAddr);
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
void StoreStandaloneTotalizerData(char *strRawTransData);
void StoreStandaloneTransData(char *strRawTransData);
void GenerateTransactionNum(char *sTransNumber);

void GetProductPrice(char *sPrice,char xPumpID,char xNozzle);
void GetProductName(char GradeId,char *strProductName);
void SaveTotalProduct(char GradeId, char *strDVolume, char *strDMoney);
void ClearList(char *strList);
void AddList(char GradeId, char *strList);

char RePrintStandalone(char FIPAddr);
char PrintStandalone(char FIPAddr,char IsReprint);

char GenerateStandaloneTransData(char xPumpID, char *PNozzle);

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
char GetPumpStatusLabel(char xPumpStatus);
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


#endif
