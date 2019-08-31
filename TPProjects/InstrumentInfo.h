#pragma once
#include "ThostFtdcMdApi.h"
// 用于承接每次合约查询的结果
class InstrumentInfo
{
private:
	int requestId;
	CThostFtdcDepthMarketDataField* pDepthMarketData;
public:
	InstrumentInfo(int nRequestID, CThostFtdcDepthMarketDataField* pDepthMarketData);
	~InstrumentInfo();
	bool isLatestInfo(int requestId);
	void updateInfo(int nRequestID, CThostFtdcDepthMarketDataField* pDepthMarketData);
};