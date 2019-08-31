#pragma once
#include "ThostFtdcMdApi.h"
// ���ڳн�ÿ�κ�Լ��ѯ�Ľ��
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