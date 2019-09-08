#pragma once
// CThostFtdcTraderSpi ��ThostFtdcTraderApi.h �ж���
// CTraderHandler ��CThostFtdcTraderSpi ���м̳�
#include "ThostFtdcTraderApi.h"
#include "getconfig.h"
#include "InstrumentInfo.h"
#include <string.h>
#include <iostream>
#include <chrono>    // std::chrono::seconds
#include <thread>    // std::thread, std::this_thread::sleep_for
#include <map>

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

	///�����ѯ����
	int ReqQryDepthMarketData(CThostFtdcQryDepthMarketDataField* pQryDepthMarketData, int nRequestID);
	///�����ѯ����, ��װ�ӿ�ReqQryDepthMarketData
	void queryDepthMarketData(string instrumentId, string exchangeId);
	// load ��Ҫ���Ͼ��۵��ļ�
	vector<string> loadInstrumentId();
	// ���ļ����ݽ��н���
	string extractInstrumentId(string rawstr);
	// �����߳���ѯ���̼ۡ����ּ�
	bool startPollThread();
	void poll();
	// ����order
	CThostFtdcInputOrderField composeInputOrder(string instrumentID, string exchangeID, int direction, int vol, double price,
		char timeCondition);
	// �������Ͼ���order
	CThostFtdcInputOrderField composeAuctionInputOrder(string instrumentID, string exchangeID, int direction, int vol, double price);
	// ���ɺ�Լ�뽻�����Ķ�Ӧ��ϵ
	void fillInstrumentExchangeMap();
private:
	CThostFtdcTraderApi* pUserTraderApi;
	CThostFtdcDepthMarketDataField* pDepthMarketData;
	int requestIndex;
	vector<string> allInstruments;
	map<string, string> instrumentsExchange;
	map<string, InstrumentInfo> instrumentInfoMap;
};