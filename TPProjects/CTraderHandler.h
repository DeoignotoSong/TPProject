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
#include <map>
#include <unordered_map>

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
	//int ReqQryDepthMarketData(CThostFtdcQryDepthMarketDataField* pQryDepthMarketData, int nRequestID);
	///�����ѯ����, ��װ�ӿ�ReqQryDepthMarketData
	void queryDepthMarketData(string instrumentId, string exchangeId);
	// load ��Ҫ���Ͼ��۵��ļ�
	vector<string> loadInstruments();
	// �����߳���ѯ���̼ۡ����ּ�
	bool startPollThread();
	// �����̲߳�ѯ����״̬
	bool startScanThread();
	void poll();
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
	double choosePrice(CThostFtdcDepthMarketDataField* lastestInfo);
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
	// �������㱨��
	void cancelInstrument(string instrumentId);
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
	
private:
	CThostFtdcTraderApi* pUserTraderApi;
	CThostFtdcDepthMarketDataField* pDepthMarketData;
	int queryReqIndex;
	int orderReqIndex;
	int insQueryId = 0;
	bool startPool = false;
	SlipperyPhase::PHASE_ENUM curPhase = SlipperyPhase::OUT_OF_PHASE;
	int auctionLastReqId = 0;
	vector<string> allInstruments;
	// ���ڼ�¼ ��Լ���뽻�����Ķ�Ӧ��ϵ
	map<string, string> instrumentsExchange;
	// ��¼��ͬ��Լ��������Ϣ������¼��Ч��Լ����
	map<string, InstrumentInfo> instrumentInfoMap;
	// ����ȷ��ÿ�β�ѯ�������Ӧ�Ļص�һ��
	map<int, string> onceQueryMarker;

	// ��¼��ͬ�ĺ�Լ����Ҫ��or��������������
	// ��¼���뼯�Ͼ��۵ĺ�Լ
	map<string, InstrumentOrderInfo> auctionInsOrderMap;
	// ��¼���뻬���µ��ĺ�Լ
	map<string, InstrumentOrderInfo> slipperyInsOrderMap;

	// ��¼��ͬ��Լ�Ķ���״̬���µ��ɹ���񣬳ɽ����
	// ��¼���뼯�Ͼ��۵ĺ�Լ״̬
	unordered_map<string, AuctionInsState*> auctionInsStateMap;
	// ��¼���뻬���µ��ĺ�Լ״̬
	unordered_map<string, SlipperyInsState*> slipperyInsStateMap;

	// ��Ҫ��ȡCThostFtdcOrderField.OrderSysID ������ţ����ڲ�ѯ״̬
	unordered_map<string, CThostFtdcOrderField*> slipperyRtnOrderMap;
	unordered_map<string, int> slipperyFinishedIns;
	
	// �ѳ��صĺ�Լ
	map<string, int> cancelledIns;
	
};