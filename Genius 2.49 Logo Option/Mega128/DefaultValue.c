DefSpvPassword[10] = {"11111"};
DefSysPassword[10] = {"00000"};
DefOperatorName[19] = {"Mr.Big"};
DefClientIP[4] ={192,168,16,70};//{192,168,0,79};//{192,168,1,221}; //
DefServerIP[4] ={192,168,16,180};//{192,168,0,220};//{192,168,1,100}; //


DefPrinterType =PT_CUSTOM_TG02;
DefPrintScrollEnd = 7;//5;
DefPrintScrollSpace = 5;//2;
DefPrintAutoCut = 2;
DefPrintLogo = 0;
DefPrintSize = 3;

DefConnectionHost = 0;
DefShowDateTime = 1;
DefNotifScreen = 1;

DefDecimalPrice = 0;
DefDecimalVolume = 3;
DefDecimalMoney = 0;
DefDecimalTotalVolume =2;
DefDecimalTotalMoney = 0;
DefDecimalMark = ',';
DefCurrencyMark ='.';
DefMarkMap[5]={',','.',' ','/','-'};

                      //FIP_ID 
DefPumpMap[8]   = {1, 2, 0, 0, 0, 0, 0, 0};
DefPumpLabel[8] = {1, 2, 0, 0, 0, 0, 0, 0};

                                        // N1 N2 N3 N4 N5 N6
DefNozzleMap [8][6] = {{1, 0, 0, 0, 0, 0}, //FIP 01
                                             {1, 0, 0, 0, 0, 0}, //FIP 02
										     {0, 0, 0, 0, 0, 0}, //FIP 03
										     {0, 0, 0, 0, 0, 0}, //FIP 04
										     {0, 0, 0, 0, 0, 0}, //FIP 05
										     {0, 0, 0, 0, 0, 0}, //FIP 06
										     {0, 0, 0, 0, 0, 0}, //FIP 07
										     {0, 0, 0, 0, 0, 0}, //FIP 08
										     };


DefProductPrice[6][9]={{"6500"},{"7250"},{"4500"}, 
											 {"6500"},
											 {"6500"}, 
											 {"4500"}};

DefProductName[6][13] ={{"Pert+   "}, 
                                              {"Pertamax"},
											  {"Premium "}, 
											  {"Solar   "},
											  {"BioSolr "},
											  {"Diesel  "},
											  };

DefBankName[4][11] ={{"BCA"},
                                   {"Mandiri"},
								   {"BNI"},
								   {"BRI"},
								   };
								   
DefPrintInitialize=False;
DefInitIFT=IT_SLAVE;
DefDispenserBrand=ST_GILBARCO;//ST_WAYNE_DART;
DefTransactionNumber[7]={"000000"};
DefShift=1;
DefBaudrate[4]={br9600,br19200,br5787,br9600};
DefLastShiftDateTime[20];
CurrentShiftDateTime[20];

         //PoolingPumpSetting
DefPoolingPumpMax=MAX_PUMP;
DefPoolingNoPumpCount=NO_PUMP_COUNT_MAX;
DefPoolingTryResend=TRY_RESEND;
DefPoolingSendTimeout=SEND_TIMEOUT;
DefPoolingDelayNextPump=DELAY_NEXT_PUMP;
DefActivePump=ACTIVE_PUMP;
DefSequenceTimeout=SEQUENCE_TIMEOUT;

DefHeaderFooter[10][41]={{"    PT. HANINDO AUTOMATION SOLUTIONS    "},//1
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
DefPrintMoney=True;
DefHGMode=HM_232;//HM_TTL,HM_232,HM_485
DefIFT_ID=1;