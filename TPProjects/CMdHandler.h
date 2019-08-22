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
// 由于subscribe成员函数不是继承自CThostFtdcMdSpi，所以默认private，需要声明为public
public:
	void OnFrontConnected();
	void OnRspUserLogin(
		CThostFtdcRspUserLoginField* pRspUserLogin,
		CThostFtdcRspInfoField* pRspInfo,
		int nRequestID,
		bool bIsLast
	);
	
	void subscribe(char* id);
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