#pragma  once
#include "InstrumentInfo.h"

InstrumentInfo::InstrumentInfo(int nRequestID, CThostFtdcDepthMarketDataField* respData){
	strcpy_s(this->pDepthMarketData.InstrumentID, respData->InstrumentID);
	this->pDepthMarketData.AskPrice1 = respData->AskPrice1;
	this->pDepthMarketData.BidPrice1 = respData->BidPrice1;
	this->pDepthMarketData.OpenPrice = respData->OpenPrice;
	this->pDepthMarketData.ClosePrice = respData->ClosePrice;
	this->pDepthMarketData.LastPrice = respData->LastPrice;
	this->queryReqId = nRequestID;
}
InstrumentInfo::~InstrumentInfo() {}
bool InstrumentInfo::isLatestInfo(int requestId) {
	return requestId <= this->queryReqId;
}
void InstrumentInfo::updateInfo(int nRequestID, CThostFtdcDepthMarketDataField* respData) {
	this->pDepthMarketData.AskPrice1 = respData->AskPrice1;
	this->pDepthMarketData.BidPrice1 = respData->BidPrice1;
	this->pDepthMarketData.OpenPrice = respData->OpenPrice;
	this->pDepthMarketData.ClosePrice = respData->ClosePrice;
	this->pDepthMarketData.LastPrice = respData->LastPrice;
	this->queryReqId = nRequestID;
}
CThostFtdcDepthMarketDataField* InstrumentInfo::getInfo() {
	return &(this->pDepthMarketData);
}