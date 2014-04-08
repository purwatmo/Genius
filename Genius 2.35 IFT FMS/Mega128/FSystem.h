#include "Keypad.h"
#include "RTC.h"
#include "CommonUsed.h"

#define COM_PRINTER 0

#include <avr/eeprom.h>
#include <avr/pgmspace.h>

//IyanDefinition
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
#define MAX_PRINTED_CHAR 44
#define BORDER_LENGTH (MAX_PRINTED_CHAR-2)

//TotalPerProduct
#define TOTAL_VOLUME_LENGTH 15
#define TOTAL_MONEY_LENGTH  15
#define TOTAL_PRODUCT_ADDR  0

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

enum setClientIP{cipNone,cipInit,cipInputIP,cipExit,cipStoreIPblok};
enum stateLocalAccount{laNone,laInit,laSendID,laWaitMessage57,laConTimout,
                       laProcMessage57,laDispStatus,laDispValid,laDispValidInput,
					   laSelectFIP,laSelectFIPInput,laSelectProduct,laSelectProductInput,
					   laSelectBalanceType,laSelectBalanceTypeInput,laBalanceValue,
					   laBalanceValueInput,laOdometer,laOdometerInput,
					   laDataConfirm,laDataConfirmInput,laProceedTransaction,laViewStatus,
					   laWaitMessage00,laProcMessage00,laSuccessTransaction,laFailedTransaction,
					   laLicensePlate,laDisplayTransaction,laWaitFilling,laDelayExit,laExitLocAcc};

enum FreePrintingState{fpNone,fpInit,fpInitHeader,fpLoadHeader,fpPrintHeader,
                       fpInitMessage,fpInitDuplicate,fpPrintDuplicate,fpPrintMessage,fpInitSpace,fpPrintMargin,
					   fpInitFooter,fpLoadFooter,fpPrintFooter,fpInitScroll,fpScrollPaper,fpPaperCut,fpFinishFreePrinting,
                       fpCheckPrintStatusHeader,fpCheckPrintStatusMessage,fpCheckPrintStatusFooter,fpCheckPrintStatusDuplicate
					   };

enum PrintIdleState{piIdle,piInit,piLoadHeader,piPrintHeader,piInitDuplicate,piPrintDuplicate,piCheckPrintStatusDuplicate,
                    piInitMessage,piFormatingMessage,piLoadMessage,piPrintMessage,piCheckPrintStatusHeader,
                    piCheckPrintStatusMessage,piCheckPrintStatusFooter,piLoadFooter,piPrintFooter,piScrollPaper,
					piInitScroll,piCheckPrintStatusScroll,piPaperCut,piFinishPrintIdle};

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

enum ePrinterType{PT_DEFAULT,PT_CUSTOM_TG02,PT_CUSTOM_CUBE};
enum ePumpType{ptNone,ptInitMenu,ptSelectBrand,ptUpdated};

enum eMenuPumpId{mpInitPumpId,mpDisplayPumpId,mpSelectInput,mpSaveFailed,mpDisplayFailed,
                 mpSavingPumpId,mpSendConfigToSlave,mpDelaySaveConfig,
                 mpWaitSlaveReply,mpDelayExitPumpId,mpExitPumpId};

enum eMenuMaster{mmInitMaster,mmMasterSelect,mmRestorePasswordMenu,mmGeniusCodeEntry,
                 mmIsValidGeniusCode,mmDisplayInvalidCode,mmDelayDisplayInvalidCode,
				 mmDisplayKeyStamp,mmRestoreCodeEntry,mmIsValidRestoreCode,mmDisplayInvalidRestoreCode,
				 mmDelayInvalidRestoreCode,mmRestorePassword,
				 mmDisplaySuccess,mmChangePassword,mmDelayExit,mmExitMaster};

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


char PumpCountMax=0,ActivePump=0;
char IFType=IT_SLAVE;
char DispenserBrand=ST_NONE;
char IsNewPumpStatus=True;
char strPumpStatus[17]={"----------------"};
char CurrentPumpStatus[16]={0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0};
char PrintedStatus[16]={0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0};
char ReprintReady[16]={ 0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0};
char IsReprintTicket=False;
char iSequencePooling=0,IsNewPoolingSequence=False;
volatile char IsStandaloneAcknoledge=False,IsPoolingRestarted=False,IsSetPumpType=True;

char IsFreePrinting=False,IsCompleteFilling=False;

//Setting PumpPooling
char IsControlPooling=False;
char PoolCmd,PoolMsg;
char AcknoledgePump,AcknoledgeCommand;

// Config Protocol 
char IsConfigFlow=False;
//Beep 12
unsigned int TimBeep=0;
unsigned char ProcTimeOut=0,PumpID=0;

//Tim System
char TimTicker=0,TimPressed=0;
volatile char TimDisplay=0;
unsigned int TimSend=0;

//RFID System
char IsRFIDDetected=False;
char strRFID[10];

char IsNewPacket=False;

//SystemDateTime
char strSystemDate[9],strSystemTime[9];

//Printing System
char IsPrinting=False,IsPrintERROR=False,IsBusyIdlePrinting=False;
volatile char IsBusyPrint=False,IsBusyFreePrinting=False;
int TimPrintBusy=0;
//Buffer FreePrint
char PrintBuffer[401];//[405];
char cmdPrint=0;


// Serial Buffer 620 + 36 = 656
unsigned char rcv_trans[620];
unsigned char MsgCode=MSG_NONE;
unsigned int char_count=0;
unsigned int transLength=0,LengthMessage81=0,TimBackLight=0;


unsigned char	EEMEM DefSpvPassword[10] = {"11111"};
unsigned char	EEMEM DefSysPassword[10] = {"00000"};
         

unsigned char	EEMEM DefOperatorName[19] = {"Mr.Big"};

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
		 char EEMEM DefBaudrate[4]={br9600,br19200,br5787,br9600};

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
		 char EEMEM DefIFT_ID=1;
