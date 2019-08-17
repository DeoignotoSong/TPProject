#pragma once
#include <iostream>
#include <string.h>
#include "ThostFtdcMdApi.h"
#include "getconfig.h"

using namespace std;

class CMdHandler : public CThostFtdcMdSpi
{
public:
	CMdHandler(CThostFtdcMdApi* pUserMdApi);
	~CMdHandler();
private:
	CThostFtdcMdApi* pUserMdApi;
	int requestIndex;

	void OnFrontConnected();
	void OnRspUserLogin(
		CThostFtdcRspUserLoginField* pRspUserLogin,
		CThostFtdcRspInfoField* pRspInfo,
		int nRequestID,
		bool bIsLast
	);
	void OnRspSubMarketData(
		CThostFtdcSpecificInstrumentField* pSpecificInstrument,
		CThostFtdcRspInfoField* pRspInfo,
		int nRequestID,
		bool bIsLast
	);
	void OnRtnDepthMarketData(
		CThostFtdcDepthMarketDataField* pDepthMarketData
	);
};