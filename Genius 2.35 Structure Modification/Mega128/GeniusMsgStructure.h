union TCommonMessage{		
	struct TMessage99{//Message99: FIP Transactions Information
		char strTranNo[7];
		char strShift[2];
		char strDate[11];
		char strTime[9];
		char strIslandID[3];
		char strFIP_ID[3];
		char strProductID[3];
		char strDescription[16];
		char strPrice[9];
		char strVolume[9];
		char strAmount[11];
		char strMOPType[2];
		char strMOPName[21];
		char strCardID[21];
		char strCardHolder[41];
		char strBalanceTypePrint[26];
		char strBalance[14];
		char strMeterVolume[14];
		char strMeterAmount[14];
		char strCurrentTime[20];
		char strPrintCount[3];
		char strPrevPoints[9];
		char strGainPoints[9];
		char strLoyCardID[21];
		char strLoyCardHolder[31];
		char strLoyCurrentPoints[9];
		char strLoyCurrMonConsumeA[11];
		char strLoyCurrMonConsumeV[11];
		char strSurchargeDesc[21];
		char strSurchargeAmount[11];
		char strLoyRedeemPoints[9];
		char strLoyExpiry[11];
		char strCorporateID[21];
		char strCorporateName[31];
	}Message99;

	struct TMessage32{//Message32: Change MOP Request
		char strTranNo[7];
		char strShift[2];
		char strDate[11];
		char strTime[9];
		char strIslandID[3];
		char strFIP_ID[3];
		char strProductID[3];
		char strDescription[16];
		char strPrice[9];
		char strVolume[9];
		char strAmount[11];
		char strMOPType[2];
		char strMOPName[21];
		char strCardID[21];
		char strCardHolder[41];
		char strBalanceTypePrint[26];
		char strBalance[14];
		char strMeterVolume[14];
		char strMeterAmount[14];
		char strCurrentTime[20];
		char strPrintCount[3];
		char strPrevPoints[9];
		char strGainPoints[9];
		char strLoyCardID[21];
		char strLoyCardHolder[31];
		char strLoyCurrentPoints[9];
		char strLoyCurrMonConsumeA[11];
		char strLoyCurrMonConsumeV[11];
	}Message32;
	
	struct TMessage09{//Message09: Free Display to IFT
		char strFreeMessageLine1[21];
		char strFreeMessageLine2[21];
		char strFreeMessageLine3[21];
		char strFreeMessageLine4[21];
	}Message09;

	struct TMessage91{//Message91: EDC Transaction Info
		char strTranNo[7];
		char strFIP_ID[3];
		char strDescription[16];
		char strPrice[7];
		char strVolume[9];
		char strAmount[9];
		char strStatus[2];
		char strSurcharge[10];
	}Message91;

	struct TMessage02{//Message02: EDC Msg 02
		char strTranNo[7];
		char strFIP_ID[3];
		char strDescription[16];
		char strPrice[9];
		char strVolume[9];
		char strAmount[11];
		char strStatus[2];
	}Message02;

	struct TMessage03{//Message03: EDC Msg 03
		char strTranNo[7];
		char strFIP_ID[3];
		char strCardType[16];
		char strApprovalCode[7];
		char strInvoiceNumber[11];
		char strDateTime[15];
		char strStatus[2];
	}Message03;

	struct TMessage92{//Message92: EDC Aproval (IFT-AdvanZ)
		char strTranNo[7];
		char strFIP_ID[3];
		char strCardType[16];
		char strCardID[20];
		char strApprovalCode[7];
		char strInvoiceNumber[11];
		char strDateTime[15];
	}Message92;

	struct TMessage04{//Message04: EDC Void Info
		char strApprovalCode[7];
		char strInvoiceNumber[11];
		char strDateTime[15];
	}Message04;

	struct TMessage94{//Message94: IFT Void Info
		char strTranNo[7];
		char strFIP_ID[3];
		char strCardType[16];
		char strCardID[20];
		char strApprovalCode[7];
		char strInvoiceNumber[11];
		char strDateTime[15];
	}Message94;

	struct TMessage57{//Message57: Local Account Balance Information
		char strCardID[21];
		char strCardHolder[31];
		char strStatus[2];
		char strLoyCurrentPoints[9];
		char strLoyCurrMonConsumeA[11];
		char strLoyCurrMonConsumeV[11];
		char strDateTime[20];
		char strAmount[8];
		char strGainPoints[5];
	}Message57;
	
	struct TMessage11{//Message11: Startup Info
		char buffHeader[41];
		char CardKey[13];
		char BalanceKey[13];
		char strProductName[13];
		char strProductPrice[9];
	}Message11;
}AdvanZ;
