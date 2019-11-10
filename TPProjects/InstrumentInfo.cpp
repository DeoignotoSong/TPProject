#pragma  once
#include "InstrumentInfo.h"

InstrumentInfo::InstrumentInfo(int nRequestID, CThostFtdcDepthMarketDataField* pDepthMarketData){
	this->pDepthMarketData = pDepthMarketData;
	this->queryReqId = nRequestID;
}
InstrumentInfo::~InstrumentInfo() {}
bool InstrumentInfo::isLatestInfo(int requestId) {
	return requestId <= this->queryReqId;
}
void InstrumentInfo::updateInfo(int nRequestID, CThostFtdcDepthMarketDataField* pDepthMarketData) {
	this->pDepthMarketData = pDepthMarketData;
	this->queryReqId = nRequestID;
}
CThostFtdcDepthMarketDataField* InstrumentInfo::getInfo() {
	return this->pDepthMarketData;
}