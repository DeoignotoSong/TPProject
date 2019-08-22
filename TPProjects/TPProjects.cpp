// TPProjects.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "ThostFtdcMdApi.h"
#include "CMdHandler.h"
#include "CTraderHandler.h"
#include "ThostFtdcTraderApi.h"
#include <Windows.h>
#include <direct.h>
#include "getconfig.h"
#include <thread>
#include <chrono>

using namespace std;

void thread_task() {
	try {
		CThostFtdcMdApi* m_tApi = CThostFtdcMdApi::CreateFtdcMdApi("./flow/");

		CMdHandler ph(m_tApi);

		m_tApi->RegisterSpi(&ph);

		m_tApi->RegisterFront((char*)"tcp://218.202.237.33:10112");

		m_tApi->Init();

		m_tApi->Join();
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			ph.subscribe((char*)"a1909");
		}
	}
	catch (const std::exception& exc) {
		std::cerr << exc.what();
	}
}

int main()
{
	// 为啥放到最后就失败？估计原因是主线程结束？
	std::thread one(thread_task);
	// 保存生成的log文件的文件夹路径
	string logFilePath = getConfig("config", "LogFilesPath");

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

	CThostFtdcTraderApi *m_pApi = CThostFtdcTraderApi::CreateFtdcTraderApi(logFilePath.c_str());

	CTraderHandler sh(m_pApi);

	m_pApi->RegisterSpi(&sh);

	m_pApi->SubscribePrivateTopic(THOST_TERT_QUICK);
	m_pApi->SubscribePublicTopic(THOST_TERT_QUICK);

	m_pApi->RegisterFront((char*)getConfig("config", "FrontAddr").c_str());
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
