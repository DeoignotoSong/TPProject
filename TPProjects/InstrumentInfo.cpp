#pragma  once
#include "InstrumentInfo.h"

InstrumentInfo::InstrumentInfo(int nRequestID, CThostFtdcDepthMarketDataField* pDepthMarketData){
	this->pDepthMarketData = pDepthMarketData;
	this->requestId = nRequestID;
}
InstrumentInfo::~InstrumentInfo() {}
bool InstrumentInfo::isLatestInfo(int requestId) {
	return requestId < this->requestId;
}
void InstrumentInfo::updateInfo(int nRequestID, CThostFtdcDepthMarketDataField* pDepthMarketData) {
	this->pDepthMarketData = pDepthMarketData;
	this->requestId = nRequestID;
}