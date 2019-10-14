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
	//int ReqQryDepthMarketData(CThostFtdcQryDepthMarketDataField* pQryDepthMarketData, int nRequestID);
	///请求查询行情, 封装接口ReqQryDepthMarketData
	void queryDepthMarketData(string instrumentId, string exchangeId);
	// load 需要集合竞价的文件
	vector<string> loadInstruments();
	// 开启线程轮询开盘价、对手价
	bool startPollThread();
	// 开启线程查询订单状态
	bool startScanThread();
	void poll();
	// 构建order
	CThostFtdcInputOrderField composeInputOrder(string instrumentID, string exchangeID, bool buyIn, int vol, double price,
		char timeCondition, int reqId);
	// 构建集合竞价order
	CThostFtdcInputOrderField composeAuctionInputOrder(string instrumentID, string exchangeID, bool buyIn, int vol, double price, int reqId);
	// 构建滑点order
	CThostFtdcInputOrderField composeSlipInputOrder(string instrumentID, string exchangeID, bool buyIn, int vol, double price, int requestId);
	// 集合竞价下单，只执行一次，将instrumentOrderMap中的内容逐一下单
	void callAuction();
	// 判断retOrder是不是滑点下单
	bool isSlipOrder(CThostFtdcOrderField* order);
	// 通过合约单号获取交易所Id
	string getExchangeId(string instrumentId);
	// 查询已下单的合约成交情况
	void queryTrade(string instId, string exgId);
	// 发起查询ongoingIns 里所有合约单的状态
	void scanOngoingOrderStatus();
	// 报单查询请求。当客户端发出报单查询指令后，交易托管系统返回响应时，该方法会被调用
	void OnRspQryOrder(
		CThostFtdcOrderField* pOrder,
		CThostFtdcRspInfoField* pRspInfo,
		int nRequestID,
		bool bIsLast); 
	//报单操作应答。报单操作包括报单的撤销、报单的挂起、报单的激活、报单的修改。
	// 当撤单指令出错时候，会被调用
	void OnRspOrderAction(
		CThostFtdcOrderActionField* pOrderAction,
		CThostFtdcRspInfoField* pRspInfo,
		int nRequestID,
		bool bIsLast);
	// 报价操作错误回报。由交易托管系统主动通知客户端，该方法会被调用
	void OnErrRtnOrderAction(
		CThostFtdcOrderActionField* pOrderAction,
		CThostFtdcRspInfoField* pRspInfo);
	
private:
	CThostFtdcTraderApi* pUserTraderApi;
	CThostFtdcDepthMarketDataField* pDepthMarketData;
	int requestIndex;
	int insQueryId = 0;
	bool startPool = false;
	bool auctionOverFlag = false;
	// auctionOver 与 slipStart 之间有时间间隔
	bool slipStartFlag = false;
	int auctionLastReqId = 0;
	vector<string> allInstruments;
	// 用于记录 合约号与交易所的对应关系
	map<string, string> instrumentsExchange;
	// 记录不同合约的最新信息，仅记录有效合约单号
	map<string, InstrumentInfo> instrumentInfoMap;
	// 记录不同合约的交易信息,<instrumentId,>
	map<string, InstrumentOrderInfo> instrumentOrderMap;
	// 用于确认每次查询仅处理对应的回调一次
	map<int, string> onceQueryMarker;

	// 待下单的合约，集合竞价与滑点下单共用
	map<string,int> toInsertIns;
	// 已下单，待成交/撤回的单子。不可以与toInsertIns合并，需要获取CThostFtdcOrderField.OrderSysID 报单编号，用于查询状态
	map<string, CThostFtdcOrderField*> ongoingIns;
	// 已成交的合约
	map<string, int> bingoIns;
	// 已撤回的合约
	map<string, int> cancelledIns;
	
	
};