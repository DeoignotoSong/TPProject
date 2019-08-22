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

	strcpy_s(userFields.BrokerID, getConfig("config", "BrokerID").c_str());
	strcpy_s(userFields.UserID, getConfig("config", "UserID").c_str());
	strcpy_s(userFields.Password, getConfig("config", "Password").c_str());

	pUserMdApi->ReqUserLogin(&userFields, requestIndex++);
}

void CMdHandler::OnRspUserLogin(
	CThostFtdcRspUserLoginField* pRspUserLogin, 
	CThostFtdcRspInfoField* pRspInfo, 
	int nRequestID, 
	bool bIsLast)
{
	std::cout << "Login Success......" << std::endl;
}

void CMdHandler::subscribe(char* id) {
	std::cout << "subscribe id:" << id << std::endl;
	char** ppInstrumentID = new char* [50];
	ppInstrumentID[0] = id;

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
	std::cout << "CMdHandler.OnRtnDepthMarketData is called" << std::endl;
	//std::cout << "交易日:" << pDepthMarketData->TradingDay << std::endl;
	//std::cout << "合约代码:" << pDepthMarketData->InstrumentID << std::endl;
	//std::cout << "交易所代码:" << pDepthMarketData->ExchangeID << std::endl;
	//std::cout << "合约在交易所的代码:" << pDepthMarketData->ExchangeInstID << std::endl;
	std::cout << "最新价:" << pDepthMarketData->LastPrice << std::endl;
	//std::cout << "上次结算价:" << pDepthMarketData->PreSettlementPrice << std::endl;
	//std::cout << "昨收盘:" << pDepthMarketData->PreClosePrice << std::endl;
	//std::cout << "昨持仓量:" << pDepthMarketData->PreOpenInterest << std::endl;
	//std::cout << "今开盘:" << pDepthMarketData->OpenPrice << std::endl;
	//std::cout << "最高价:" << pDepthMarketData->HighestPrice << std::endl;
	//std::cout << "最低价:" << pDepthMarketData->LowestPrice << std::endl;
	//std::cout << "数量:" << pDepthMarketData->Volume << std::endl;
	//std::cout << "成交金额:" << pDepthMarketData->Turnover << std::endl;
	//std::cout << "持仓量:" << pDepthMarketData->OpenInterest << std::endl;
	//std::cout << "今收盘:" << pDepthMarketData->ClosePrice << std::endl;
	//std::cout << "本次结算价:" << pDepthMarketData->SettlementPrice << std::endl;
	//std::cout << "涨停板价:" << pDepthMarketData->UpperLimitPrice << std::endl;
	//std::cout << "跌停板价:" << pDepthMarketData->LowerLimitPrice << std::endl;
	//std::cout << "昨虚实度:" << pDepthMarketData->PreDelta << std::endl;
	//std::cout << "今虚实度:" << pDepthMarketData->CurrDelta << std::endl;
	//std::cout << "最后修改时间:" << pDepthMarketData->UpdateTime << std::endl;
	std::cout << "================================================================" << std::endl;

	pUserMdApi->Release();
}
