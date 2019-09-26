#pragma once
// CThostFtdcTraderSpi 在ThostFtdcTraderApi.h 中定义
// CTraderHandler 对CThostFtdcTraderSpi 进行继承
#include "ThostFtdcTraderApi.h"
#include "getconfig.h"
#include "InstrumentInfo.h"
#include "InstrumentOrderInfo.h"
#include "Utils.h"
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

	///请求查询行情
	int ReqQryDepthMarketData(CThostFtdcQryDepthMarketDataField* pQryDepthMarketData, int nRequestID);
	///请求查询行情, 封装接口ReqQryDepthMarketData
	void queryDepthMarketData(string instrumentId, string exchangeId);
	// load 需要集合竞价的文件
	vector<string> loadInstruments();
	// 开启线程轮询开盘价、对手价
	bool startPollThread();
	void poll();
	// 构建order
	CThostFtdcInputOrderField composeInputOrder(string instrumentID, string exchangeID, bool buyIn, int vol, double price,
		char timeCondition, int reqId);
	// 构建集合竞价order
	CThostFtdcInputOrderField composeAuctionInputOrder(string instrumentID, string exchangeID, bool buyIn, int vol, double price, int reqId);
	// 集合竞价下单，只执行一次，将instrumentOrderMap中的内容逐一下单
	void callAuction();
	// 滑点订单，重复对failedInstruments进行下单
	void callSlippage();
private:
	CThostFtdcTraderApi* pUserTraderApi;
	CThostFtdcDepthMarketDataField* pDepthMarketData;
	int requestIndex;

	vector<string> allInstruments;
	// 用于记录 合约号与交易所的对应关系
	map<string, string> instrumentsExchange;
	// 记录不同合约的最新信息
	map<string, InstrumentInfo> instrumentInfoMap;
	// 记录不同合约的交易信息,<instrumentId,>
	map<string, InstrumentOrderInfo> instrumentOrderMap;
	// 记录交易失败的合约单号
	vector<string> failedInstruments;
	// 记录交易过程中的合约单号, <reqId,instrument>
	map<int, string> ongoingInstruments;
	
};