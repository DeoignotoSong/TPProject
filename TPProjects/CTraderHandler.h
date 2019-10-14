#pragma once
// CThostFtdcTraderSpi ��ThostFtdcTraderApi.h �ж���
// CTraderHandler ��CThostFtdcTraderSpi ���м̳�
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
	// ����order
	CThostFtdcInputOrderField composeInputOrder(string instrumentID, string exchangeID, bool buyIn, int vol, double price,
		char timeCondition, int reqId);
	// �������Ͼ���order
	CThostFtdcInputOrderField composeAuctionInputOrder(string instrumentID, string exchangeID, bool buyIn, int vol, double price, int reqId);
	// ��������order
	CThostFtdcInputOrderField composeSlipInputOrder(string instrumentID, string exchangeID, bool buyIn, int vol, double price, int requestId);
	// ���Ͼ����µ���ִֻ��һ�Σ���instrumentOrderMap�е�������һ�µ�
	void callAuction();
	// �ж�retOrder�ǲ��ǻ����µ�
	bool isSlipOrder(CThostFtdcOrderField* order);
	// ͨ����Լ���Ż�ȡ������Id
	string getExchangeId(string instrumentId);
	// ��ѯ���µ��ĺ�Լ�ɽ����
	void queryTrade(string instId, string exgId);
	// �����ѯongoingIns �����к�Լ����״̬
	void scanOngoingOrderStatus();
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
	
private:
	CThostFtdcTraderApi* pUserTraderApi;
	CThostFtdcDepthMarketDataField* pDepthMarketData;
	int requestIndex;
	int insQueryId = 0;
	bool startPool = false;
	bool auctionOverFlag = false;
	// auctionOver �� slipStart ֮����ʱ����
	bool slipStartFlag = false;
	int auctionLastReqId = 0;
	vector<string> allInstruments;
	// ���ڼ�¼ ��Լ���뽻�����Ķ�Ӧ��ϵ
	map<string, string> instrumentsExchange;
	// ��¼��ͬ��Լ��������Ϣ������¼��Ч��Լ����
	map<string, InstrumentInfo> instrumentInfoMap;
	// ��¼��ͬ��Լ�Ľ�����Ϣ,<instrumentId,>
	map<string, InstrumentOrderInfo> instrumentOrderMap;
	// ����ȷ��ÿ�β�ѯ�������Ӧ�Ļص�һ��
	map<int, string> onceQueryMarker;

	// ���µ��ĺ�Լ�����Ͼ����뻬���µ�����
	map<string,int> toInsertIns;
	// ���µ������ɽ�/���صĵ��ӡ���������toInsertIns�ϲ�����Ҫ��ȡCThostFtdcOrderField.OrderSysID ������ţ����ڲ�ѯ״̬
	map<string, CThostFtdcOrderField*> ongoingIns;
	// �ѳɽ��ĺ�Լ
	map<string, int> bingoIns;
	// �ѳ��صĺ�Լ
	map<string, int> cancelledIns;
	
	
};