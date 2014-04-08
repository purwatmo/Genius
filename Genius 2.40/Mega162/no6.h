#define TRY_RESEND 2
#define SEND_TIMEOUT 10
#define DELAY_NEXT_PUMP 5
#define MAX_PUMP 4
#define NO_PUMP_COUNT_MAX 3
#define SEQUENCE_TIMEOUT 10


//PumpPooling

#define MSG_TRANSACTION_TIMEOUT 200
#define WAIT_TRANS_TIMEOUT 5
#define WAIT_TOTALIZER_TIMEOUT 20



                  //  0          1        2        3        4        5        6        7         8           9           
enum PumpResponse{PUMP_ERROR,PUMP_ST1,PUMP_ST2,PUMP_ST3,PUMP_ST4,PUMP_ST5, PUMP_OFF,PUMP_CALL,  PUMP_AUTH, PUMP_BUSY,
                  PUMP_PEOT,PUMP_FEOT,PUMP_STOP,PUMP_SEND_DATA,PUMP_NONE,PUMP_ERR1,PS_PRINT_READY,PS_PRINTED,				  
				  };
                  //  0     1    2    3    4    5        6        7    8         9            
enum eWayneStatus{PW_NONE,PW_1,PW_2,PW_3,PW_4,PW_5,PW_ONLINE,PW_CALL,PW_8,PW_AUTHORIZED,
                  PW_10,PW_END_DELIVERY,PW_12,PW_13,PW_DISCONNECT,PW_PRICE_UPDATED};

				 // 0          1             2     3             4                       5             6
enum PumpCommand{CMD_STATUS,CMD_AUTHORIZE,CMD_2,CMD_PUMP_STOP,CMD_TRANSACTION_DATA,CMD_TOTALIZER,CMD_REALTIME_MONEY,CMD_ACK,
                 CMD_REQ_GLOBAL_STATUS,CMD_REQ_GLOBAL_STATUS_2,CMD_AUTH_1,CMD_AUTH_2,CMD_AUTH_3,CMD_AUTH_4,CMD_TRANSACTION};

enum ePumpID{PUMP_16,PUMP_1,PUMP_2,PUMP_3,PUMP_4,PUMP_5,PUMP_6,PUMP_7,PUMP_8,PUMP_9,PUMP_10,PUMP_11,PUMP_12,PUMP_13,PUMP_14,
             PUMP_15,PUMP_17,PUMP_18,PUMP_ALL};

enum PoolingPumpState{ppInit,ppNextPump,ppGetStatus,ppWaitIdle,ppSendStatusRequest,
                      ppNoPump,ppWaitReplyStatus,ppUpdatePumpStatus,ppReplyAuth,
					  ppDifferentPumpID,ppRequestTransData,ppRequestRealTimeMoney,
					  ppSendAuthorize,ppWaitAuthorized,ppSendMoneyReq,
					  ppWaitMoneyReq,ppMoneyRequestCompleted,ppSendTransReq,ppWaitTransReq,
					  ppTransRequestCompleted,ppInitDelayNextPump,ppDelayNextPump,ppSendTransInfo,
					  ppSendPumpStatus,ppScanResponse,ppIsRequestTransInfo,ppIsRequestTotalizerInfo,
					  ppRequestTotalizerData,ppSendTotalizerReq,ppWaitTotalizerReq,
					  ppTotalizerRequestCompleted,ppSendTotalizerInfo,ppWaitTotalizerACK
					  };
                  // 0        


enum ePacketValidity{PV_NONE,PV_NOT_VALID,PV_VALID};
enum eWayneReceive{wrWaitSTX, wrWaitSEQ, wrIdleStatus, wrWaitMsgID1, wrWaitMsgID2, wrSwitchMessage, wrPriceNozzleInfo, wrNozzleInfo, wrSaveCRC, wrValidateCRC, wrWaitETX,wrDelivery,wrTransaction,wrTotalizer};
enum eMessageInformation{MI_NONE,MI_STATUS_IDLE,MI_NOZZLE_UP,MI_NOZZLE_DOWN,MI_DELIVERY,MI_TRANSACTION,MI_LAST_TRANSACTION,MI_UNKNOWN,MI_TOTALIZER,MI_AUTHORIZED};
enum eResponse{WP_NONE,WP_IDLE};


enum eIFType{IT_NONE,IT_SLAVE,IT_STANDALONE};
enum eDigitSize{DS_NONE,DS_6DIGIT,DS_7DIGIT};

enum eSlaveCommand{SC_NONE,SC_SLAVE,SC_STANDALONE,SC_DIAGNOSTIC,SC_TRANSACTION,SC_TOTALIZER,SC_BAUDRATE,SC_DEBUGTERMINAL,
                   SC_SET_POOLINGPUMP,SC_PUMP_LOCK,SC_PUMP_UNLOCK,SC_TRANSACTION_ACK,SC_TRANSACTION_NACK,SC_TOTALIZER_ACK,SC_TOTALIZER_NACK,
				   SC_GET_POOLING_NO_PUMP_COUNT,SC_GET_POOLING_MAX_PUMP,SC_GET_POOLING_SEND,SC_GET_POOLING_TIMEOUT,SC_GET_POOLING_DELAY_NEXT_PUMP,
				   SC_SET_POOLING_NO_PUMP_COUNT,SC_SET_POOLING_MAX_PUMP,SC_SET_POOLING_SEND,SC_SET_POOLING_TIMEOUT,SC_SET_POOLING_DELAY_NEXT_PUMP,
				   SC_LIVE_SEQUENCE,SC_POOL_RESTARTED,SC_SEQUENCE_TIMEOUT,SC_SET_PUMP_TYPE,
				   SC_SET_PUMPID,SC_CLEAR_PUMPID,SC_STOP_POOL_SEQUENCE,SC_START_POOL_SEQUENCE,
				   SC_HGM_MODE
				   };

enum eStandaloneType{ST_NONE,ST_GILBARCO,ST_WAYNE_DART,ST_TATSUNO,ST_LG};

enum eBaudRateValue{brNone,br9600,br19200,br5787};
enum eDebugTerminal{dtNone,dtOFF,dtON};

enum pipelineTotalizer{spNone,spTotalizerGrade,spTotalizerVolume,spTotalizerMoney,spPumpIdentifier,spProductGrade,spProductPrice,spProductVolume,spTotalizerPPU1,spTotalizerPPU2,spProductMoney,spLRC};

enum RecvCom0State{rcNone,rcIdle,rcRealTimeMoney,rcInitTransaction,rcInitTotalizer,rcSaveTotalizerMessage,rcSaveTransactionMessage};

enum BooleanResult{False,True};

enum eComLevel{CL_TTL,CL_232,CL_485};
enum e485Dir{DIR_NONE,DIR_TX,DIR_RX};
enum eWayneACK{WA_NONE,WA_ACK,WA_NACK};
enum eHGMode{HM_TTL,HM_232,HM_485};

char GetPumpID(char data);
char GetResponse(char data);
void PumpCommand(char IDPump, char Command);

void GilbarcoOnReceive(char data);
void OnReceive1(char data);
void FPoolingPump();
char FilterBCD(char data);
char BCD2Char(char data);
void UpdateMoney(char *Dest, char *Src , unsigned int Length);
void ShiftData(char data);
void StrPosCopy(char *Source, char *Dest,unsigned int IdxSource, unsigned int Length);
char CharPosCopy(char *Source, unsigned int IdxSource);

void SystemInit();
void systemServiceSPI();
void systemSlave();
void systemMaster();
void SendStrSPI(char *strSendSPI);
void SendSPI(char xSPI);
void ShiftArray(char *strShifted, unsigned int nCount);
char GetReceiveLine();
void SetReceiveLine(char RecLine);

void SendTotalizerFlow(char xPumpID);
//void SendTransFlow(char xPumpID, char rxPumpId,char xNozzleID,char xProductID, char *sUnitPrice, char *sVolume,char * sAmount);
void SendTransFlow(char xPumpID, char rxPumpId,char xNozzleID,char xProductID, char *sUnitPrice, char *sVolume,char * sAmount,char TransDigit);
void SendPumpStatusFlow(char xPumpID,char xPumpStatus);
void RemZeroLead(char *Zeroed);
void AddZeroLead(char *String,unsigned char Size);
char HexToChar(char xHex);
void StrPosCopyReverse(char *Source, char *Dest,unsigned int IdxSource, unsigned int Length);
void InitMem();
void StartupInfo();
void DoNothing();
void SaveTransactionData(char data);
void SaveTotalizerData(char data);
void uartGilbarco();
void StatePrintf(char *strState);
void StrReverse(char *strSource);

void ScanStandaloneFlow(char xData);
void InitPumpData();
void InitSystemTimer();

void TerminalSendf(char Com,char *strSendf);
void TerminalSend(char Com,char *strSend);

int GetBaudrate(char brSetting);
void FillChar(char *strMemory, unsigned int Length,char data);
void SendCommandAcknoledge(char AckCommand,char AckData);
void systemAntiFreeze();

void SystemComLevel(char ComLevel);
void SystemSetSlave();
void SystemSetDispenser(char TDispenserBrand);

//Wayne
unsigned int CRC_Wayne(unsigned int crc, char a);
void ExtractValue(char *Source,char FirstPos,char nCount,char*Dest);
void WayneOnReceive(char WayneDataIn);
void FWayneSendBuffer(char *Buffer, char nLength);
void FWayneSendCommand(char Command, char SequenceCmd, char xPumpID, char NozzleID);
void WayneSendChar(char xData);
void WayneTestSend();
void System485(char Dir);//DIR_TX, DIR_RX
