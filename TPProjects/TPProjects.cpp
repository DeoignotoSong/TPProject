// TPProjects.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "ThostFtdcMdApi.h"
#include "CMdHandler.h"
#include "CTraderHandler.h"
#include "ThostFtdcTraderApi.h"
#include <Windows.h>
#include <direct.h>

using namespace std;

int main()
{
	// 保存生成的log文件的文件夹路径
	string logFilePath = "./flow/";

	// 创建保存生成的log文件的文件夹
	// folderCreatedResult：
	// 0：创建成功
	// -1：文件夹存在，创建失败
	int folderCreatedResult = _mkdir(logFilePath.c_str());

	if (folderCreatedResult == 0) {
		cout << "Create log directory success." << endl;
	}
	else {
		cout << "Log directory is already existed." << endl;
	}

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
