#pragma once
// CThostFtdcTraderSpi ��ThostFtdcTraderApi.h �ж���
// CTraderHandler ��CThostFtdcTraderSpi ���м̳�
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

	///�����ѯ����
	//int ReqQryDepthMarketData(CThostFtdcQryDepthMarketDataField* pQryDepthMarketData, int nRequestID);
	///�����ѯ����, ��װ�ӿ�ReqQryDepthMarketData
	void queryDepthMarketData(string instrumentId, string exchangeId);
	// load ��Ҫ���Ͼ��۵��ļ�
	vector<string> loadInstruments();
	// �����߳���ѯ���̼ۡ����ּ�
	bool startQueryThread();
	// �����̲߳�ѯ����״̬
	bool startScanThread();
	void backgroundQuery();
	// �����̻߳����µ���һ����
	bool startSlipPhaseAThread();
	// �����̻߳����µ��ڶ�����
	bool startSlipPhaseBThread();
	// �����̻߳����µ���������
	bool startSlipPhaseCThread();
	void slipPhaseCEntrance();
	// �̻߳����µ���������ʵ���߼�
	void slipPhaseCProcess();
	// ����order
	CThostFtdcInputOrderField composeInputOrder(string instrumentID, string exchangeID, bool buyIn, int vol, double price,
		char timeCondition, int reqId);
	// �������Ͼ���order
	CThostFtdcInputOrderField composeAuctionInputOrder(string instrumentID);
	// ��������order
	CThostFtdcInputOrderField composeSlipInputOrder(string instrumentID);
	// �ύ���㶩��
	void submitSlipperyOrder(string instrumentId);
	// �۸�������������ʱ���lastestInfo��������ʵļ۸�
	double choosePrice(CThostFtdcDepthMarketDataField* lastestInfo, bool buyIn);
	// ���Ͼ����µ���ִֻ��һ�Σ���auctionInsOrderMap�е�������һ�µ�
	void callAuction();
	// �����µ������ڶ������µ�����slipperyInsOrderMap�е�����
	void callSlippery(SlipperyPhase::PHASE_ENUM phase);
	// ͨ����Լ���Ż�ȡ������Id
	string getExchangeId(string instrumentId);
	// ��ѯ���µ��ĺ�Լ�ɽ����
	void queryTrade(string instId, string exgId);
	// scan slipperyInsStateMap ���к�Լ����״̬
	void scanSlipperyOrderState();
	// query a request ��ѯĳһ����״̬
	void queryOrderState(int reqId, string insId);
	// �������㱨��
	void cancelInstrument(int reqId);
	// ������ѯ���󡣵��ͻ��˷���������ѯָ��󣬽����й�ϵͳ������Ӧʱ���÷����ᱻ����
	void OnRspQryOrder(
		CThostFtdcOrderField* pOrder,
		CThostFtdcRspInfoField* pRspInfo,
		int nRequestID,
		bool bIsLast); 
	//��������Ӧ�𡣱����������������ĳ����������Ĺ��𡢱����ļ���������޸ġ�
	// ������ָ�����ʱ�򣬻ᱻ����
	void OnRspOrderAction(
		CThostFtdcOrderActionField* pOrderAction,
		CThostFtdcRspInfoField* pRspInfo,
		int nRequestID,
		bool bIsLast);
	// ���۲�������ر����ɽ����й�ϵͳ����֪ͨ�ͻ��ˣ��÷����ᱻ����
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
	bool runningFlag = false;
	SlipperyPhase::PHASE_ENUM curPhase = SlipperyPhase::OUT_OF_PHASE;
	int auctionLastReqId = 0;
	int phaseILastReqId = 0;
	int phaseIILastReqId = 0;
	vector<string> allInstruments;
	// ���ڼ�¼ ��Լ���뽻�����Ķ�Ӧ��ϵ
	unordered_map<string, string> instrumentsExchange;
	// ��¼��ͬ��Լ��������Ϣ������¼��Ч��Լ����
	unordered_map<string, InstrumentInfo> instrumentInfoMap;
	// ����ȷ��ÿ�β�ѯ�������Ӧ�Ļص�һ��
	unordered_map<int, string> onceQueryMarker;

	// ��¼��ͬ�ĺ�Լ����Ҫ��or��������������
	// ��¼���뼯�Ͼ��۵ĺ�Լ
	unordered_map<string, InstrumentOrderInfo> auctionInsOrderMap;
	// ��¼���뻬���µ��ĺ�Լ
	unordered_map<string, InstrumentOrderInfo> slipperyInsOrderMap;

	// ��¼��ͬ��Լ�Ķ���״̬���µ��ɹ���񣬳ɽ����
	// ��¼���뼯�Ͼ��۵ĺ�Լ״̬
	unordered_map<string, AuctionInsState*> auctionInsStateMap;
	// ��¼���뻬���µ��ĺ�Լ״̬
	// ���ڵڶ�������Ҫÿ���µ�һ�֣�������Ҫ��¼ÿһ�ֵ�״̬��first=orderId, second=state
	// �ڶ����ֵ�scan�׶κͱ����׶Σ�Ҫ��slipperyInsStateMap�Ķ�д����
	unordered_map<string, unordered_map<int, SlipperyInsState*>> slipperyInsStateMap;

	// ��Ҫ��ȡCThostFtdcOrderField.OrderSysID ������ţ����ڲ�ѯ״̬
	// ͬ�ϣ���Ҫ��¼ÿһ��order�����ݡ�first=orderId, second=orderField
	unordered_map<int, CThostFtdcOrderField*> slipperyRtnOrderMap;

	FILE* debugLogFile;
	FILE* infoLogFile;
	FILE* errorLogFile;
	
};