#include "CommonUsed.h"
#include <avr/eeprom.h>

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


//Standalone Totalizer Report
char strDeltaMoney[15],strDeltaVolume[15];
char strTotalMoney[20],strTotalVolume[20];

void StoreStandaloneTotalizerData(char *strRawTransData);
char GetPumpStatusLabel(char xPumpStatus);
char GenerateStandaloneTransData(char xPumpID, char *PNozzle);
