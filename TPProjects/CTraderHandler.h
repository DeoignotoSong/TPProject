#pragma once
// CThostFtdcTraderSpi 在ThostFtdcTraderApi.h 中定义
// CTraderHandler 对CThostFtdcTraderSpi 进行继承
#include "ThostFtdcTraderApi.h"
#include "getconfig.h"
#include <string.h>
#include <iostream>

using namespace std;

class CTraderHandler : public CThostFtdcTraderSpi {
public:
	CTraderHandler(CThostFtdcTraderApi* pUserTraderApi);
	~CTraderHandler();

	void ReqAuthenticate();
	void OnFrontConnected();
	void OnFrontDisconnected(int nReason);
	void OnRspAuthenticate(
		CThostFtdcRspAuthenticateField* pRspAuthenticateField, 
		CThostFtdcRspInfoField* pRspInfo,
		int nRequestID, 
		bool bIsLast);
	void OnRspUserLogin(
		CThostFtdcRspUserLoginField* pRspUserLogin,
		CThostFtdcRspInfoField* pRspInfo,	
		int nRequestID,
		bool bIsLast);
	void OnRspQryDepthMarketData(
		CThostFtdcDepthMarketDataField* pDepthMarketData,
		CThostFtdcRspInfoField* pRspInfo,
		int nRequestID,
		bool bIsLast
	);
	void OnRspQryTradingAccount(
		CThostFtdcTradingAccountField* pTradingAccount,
		CThostFtdcRspInfoField* pRspInfo,
		int nRequestID,
		bool bIsLast
	);
	void OnRtnOrder(
		CThostFtdcOrderField* pOrder
	);
	void OnRtnTrade(
		CThostFtdcTradeField* pTrade
	);
	void OnRspOrderInsert(
		CThostFtdcInputOrderField* pInputOrder,
		CThostFtdcRspInfoField* pRspInfo,
		int nRequestID,
		bool bIsLast
	);
	void OnErrRtnOrderInsert(
		CThostFtdcInputOrderField* pInputOrder, 
		CThostFtdcRspInfoField* pRspInfo
	);
	void OnRspSettlementInfoConfirm(
		CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
		CThostFtdcRspInfoField* pRspInfo,
		int nRequestID,
		bool bIsLast
	);
	void beginQuery();
	void OnRspQryInvestorPosition(
		CThostFtdcInvestorPositionField* pInvestorPosition, 
		CThostFtdcRspInfoField* pRspInfo, 
		int nRequestID, 
		bool bIsLast
	);
private:
	CThostFtdcTraderApi* pUserTraderApi;
	CThostFtdcDepthMarketDataField* pDepthMarketData;
	int requestIndex;
};