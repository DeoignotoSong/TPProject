#pragma once
// CThostFtdcTraderSpi 在ThostFtdcTraderApi.h 中定义
// CTraderHandler 对CThostFtdcTraderSpi 进行继承
#include "ThostFtdcTraderApi.h"
#include "getconfig.h"
#include "InstrumentInfo.h"
#include "InstrumentOrderInfo.h"
#include "AuctionInsState.h"
#include "SlipperyInsState.h"
#include "SlipperyPhase.cpp"
#include "Utils.h"
#include <string.h>
#include <iostream>
#include <chrono>    // std::chrono::seconds
#include <thread>    // std::thread, std::this_thread::sleep_for
#include <unordered_map>
#include <mutex>
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define LOG_DEBUG(str) logDebug(__FILENAME__, __LINE__, str)
#define LOG_INFO(str) logInfo(__FILENAME__, __LINE__, str)
#define LOG_ERROR(str) logError(__FILENAME__, __LINE__, str)
using namespace std;

class CTraderHandler : public CThostFtdcTraderSpi {
mutex logMtx;
mutex mtx;
mutex reqQueueMtx;
// when unRepliedReq less than 0, it means there is no unRepliedReq, and we can send a request
volatile int unRepliedReq = -1;

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
	bool startQueryThread();
	// 开启线程查询订单状态
	bool startScanThread();
	void backgroundQuery();
	// 开启线程滑点下单第一部分
	bool startSlipPhaseAThread();
	// 开启线程滑点下单第二部分
	bool startSlipPhaseBThread();
	// 开启线程滑点下单第三部分
	bool startSlipPhaseCThread();
	void slipPhaseCEntrance();
	// 线程滑点下单第三部分实体逻辑
	void slipPhaseCProcess();
	// 构建order
	CThostFtdcInputOrderField composeInputOrder(string instrumentID, string exchangeID, bool buyIn, int vol, double price,
		char timeCondition, int reqId);
	// 构建集合竞价order
	CThostFtdcInputOrderField composeAuctionInputOrder(string instrumentID);
	// 构建滑点order
	CThostFtdcInputOrderField composeSlipInputOrder(string instrumentID);
	// 提交滑点订单
	void submitSlipperyOrder(string instrumentId);
	// 价格生成器，根据时间和lastestInfo生成最合适的价格
	double choosePrice(CThostFtdcDepthMarketDataField* lastestInfo, bool buyIn);
	// 集合竞价下单，只执行一次，将auctionInsOrderMap中的内容逐一下单
	void callAuction();
	// 滑点下单，即第二部分下单，将slipperyInsOrderMap中的内容
	void callSlippery(SlipperyPhase::PHASE_ENUM phase);
	// 通过合约单号获取交易所Id
	string getExchangeId(string instrumentId);
	// 查询已下单的合约成交情况
	void queryTrade(string instId, string exgId);
	// scan slipperyInsStateMap 所有合约单的状态
	void scanSlipperyOrderState();
	// query a request 查询某一报单状态
	void queryOrderState(int reqId, string insId);
	// 撤销滑点报单
	void cancelInstrument(int reqId);
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
	void actionIfSlipperyTraded(string instrumentId, int reqId);
	void actionIfSlipperyCanceled(string instrumentId, int reqId);
	void printSlipperyInsStateMap();
	void waitForProcess();
	void releaseProcessLock(int reqId);
	void logDebug(const char* file, int line, ostringstream& stream);
	void logInfo(const char* file, int line, ostringstream& stream);
	void logError(const char* file, int line, ostringstream& stream);
	void log(FILE* logFile, const char* codeFile, int line, const char* msg);
	
private:
	CThostFtdcTraderApi* pUserTraderApi;
	CThostFtdcDepthMarketDataField* pDepthMarketData;
	int queryReqIndex;
	int orderReqIndex;
	int insQueryId = 0;
	volatile bool beginBackgroundQuery = false;
	bool endBackgroundQuery = false;
	SlipperyPhase::PHASE_ENUM curPhase = SlipperyPhase::OUT_OF_PHASE;
	int auctionLastReqId = 0;
	int phaseILastReqId = 0;
	int phaseIILastReqId = 0;
	vector<string> allInstruments;
	// 用于记录 合约号与交易所的对应关系
	unordered_map<string, string> instrumentsExchange;
	// 记录不同合约的最新信息，仅记录有效合约单号
	unordered_map<string, InstrumentInfo> instrumentInfoMap;
	// 用于确认每次查询仅处理对应的回调一次
	unordered_map<int, string> onceQueryMarker;

	// 记录不同的合约，需要买or卖，操作量多少
	// 记录参与集合竞价的合约
	unordered_map<string, InstrumentOrderInfo> auctionInsOrderMap;
	// 记录参与滑点下单的合约
	unordered_map<string, InstrumentOrderInfo> slipperyInsOrderMap;

	// 记录不同合约的订单状态，下单成功与否，成交与否
	// 记录参与集合竞价的合约状态
	unordered_map<string, AuctionInsState*> auctionInsStateMap;
	// 记录参与滑点下单的合约状态
	// 由于第二部分需要每笔下单一手，所以需要记录每一手的状态。first=orderId, second=state
	// 第二部分的scan阶段和报单阶段，要对slipperyInsStateMap的读写加锁
	unordered_map<string, unordered_map<int, SlipperyInsState*>> slipperyInsStateMap;

	// 需要获取CThostFtdcOrderField.OrderSysID 报单编号，用于查询状态
	// 同上，需要记录每一手order的内容。first=orderId, second=orderField
	unordered_map<int, CThostFtdcOrderField*> slipperyRtnOrderMap;

	FILE* debugLogFile;
	FILE* infoLogFile;
	FILE* errorLogFile;
	
};