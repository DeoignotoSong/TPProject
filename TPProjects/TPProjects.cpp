// TPProjects.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "ThostFtdcMdApi.h"
#include "CMdHandler.h"
#include "CTraderHandler.h"
#include "ThostFtdcTraderApi.h"
#include <Windows.h>

using namespace std;

int main()
{
	CThostFtdcTraderApi *m_pApi = CThostFtdcTraderApi::CreateFtdcTraderApi("./flow/");

	CTraderHandler sh(m_pApi);

	m_pApi->RegisterSpi(&sh);

	m_pApi->SubscribePrivateTopic(THOST_TERT_QUICK);
	m_pApi->SubscribePublicTopic(THOST_TERT_QUICK);

	m_pApi->RegisterFront((char*)"tcp://218.202.237.33 :10102");
	cout << m_pApi->GetApiVersion() << endl;
	m_pApi->Init();

	/*
	CThostFtdcMdApi* m_tApi = CThostFtdcMdApi::CreateFtdcMdApi("./flow/");

	CMdHandler ph(m_tApi);

	m_tApi->RegisterSpi(&ph);

	m_tApi->RegisterFront((char *)"tcp://218.202.237.33:10112");

	m_tApi->Init();

	m_tApi->Join();
	*/
	// m_tApi->Release();
}
