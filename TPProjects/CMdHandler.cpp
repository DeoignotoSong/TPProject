#include "CMdHandler.h"

CMdHandler::CMdHandler(CThostFtdcMdApi* pUserMdApi)
{
	this->pUserMdApi = pUserMdApi;

	this->requestIndex = 0;
}

CMdHandler::~CMdHandler() {
	pUserMdApi->Release();
}

void CMdHandler::OnFrontConnected() {
	std::cout << "Connect Success......" << endl;

	CThostFtdcReqUserLoginField userFields = { 0 };

	strcpy_s(userFields.BrokerID, "9999");
	strcpy_s(userFields.UserID, "125013");
	strcpy_s(userFields.Password, "Song1227");

	pUserMdApi->ReqUserLogin(&userFields, requestIndex++);
}

void CMdHandler::OnRspUserLogin(
	CThostFtdcRspUserLoginField* pRspUserLogin, 
	CThostFtdcRspInfoField* pRspInfo, 
	int nRequestID, 
	bool bIsLast)
{
	std::cout << "Login Success......" << std::endl;

	char** ppInstrumentID = new char* [50];

	ppInstrumentID[0] = (char*) "a1909";

	pUserMdApi->SubscribeMarketData(ppInstrumentID, requestIndex++);
}

void CMdHandler::OnRspSubMarketData(
	CThostFtdcSpecificInstrumentField* pSpecificInstrument,
	CThostFtdcRspInfoField* pRspInfo, 
	int nRequestID, 
	bool bIsLast)
{
	std::cout << "Subscribe Success" << std::endl;
}

void CMdHandler::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData)
{
	std::cout << "================================================================" << std::endl;
	std::cout << "������:" << pDepthMarketData->TradingDay << std::endl;
	std::cout << "��Լ����:" << pDepthMarketData->InstrumentID << std::endl;
	std::cout << "����������:" << pDepthMarketData->ExchangeID << std::endl;
	std::cout << "��Լ�ڽ������Ĵ���:" << pDepthMarketData->ExchangeInstID << std::endl;
	std::cout << "���¼�:" << pDepthMarketData->LastPrice << std::endl;
	std::cout << "�ϴν����:" << pDepthMarketData->PreSettlementPrice << std::endl;
	std::cout << "������:" << pDepthMarketData->PreClosePrice << std::endl;
	std::cout << "��ֲ���:" << pDepthMarketData->PreOpenInterest << std::endl;
	std::cout << "����:" << pDepthMarketData->OpenPrice << std::endl;
	std::cout << "��߼�:" << pDepthMarketData->HighestPrice << std::endl;
	std::cout << "��ͼ�:" << pDepthMarketData->LowestPrice << std::endl;
	std::cout << "����:" << pDepthMarketData->Volume << std::endl;
	std::cout << "�ɽ����:" << pDepthMarketData->Turnover << std::endl;
	std::cout << "�ֲ���:" << pDepthMarketData->OpenInterest << std::endl;
	std::cout << "������:" << pDepthMarketData->ClosePrice << std::endl;
	std::cout << "���ν����:" << pDepthMarketData->SettlementPrice << std::endl;
	std::cout << "��ͣ���:" << pDepthMarketData->UpperLimitPrice << std::endl;
	std::cout << "��ͣ���:" << pDepthMarketData->LowerLimitPrice << std::endl;
	std::cout << "����ʵ��:" << pDepthMarketData->PreDelta << std::endl;
	std::cout << "����ʵ��:" << pDepthMarketData->CurrDelta << std::endl;
	std::cout << "����޸�ʱ��:" << pDepthMarketData->UpdateTime << std::endl;
	std::cout << "================================================================" << std::endl;

	pUserMdApi->Release();
}
