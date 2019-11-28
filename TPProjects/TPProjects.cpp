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
#include "FileReader.h"
#include "InstrumentOrderInfo.h"
#include "Utils.h"
#include <unordered_map>
// https://github.com/amrayn/easyloggingpp#getting-started
#include <errno.h>

using namespace std;

int main()
{
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

	while (true) {
		cout << "waiting for Auction Start Time" << endl;
		this_thread::sleep_until(getAuctionStartTime());
		cout << "time is ready" << endl;
		// 创建Trader实例
		CThostFtdcTraderApi* m_pApi = CThostFtdcTraderApi::CreateFtdcTraderApi(logFilePath.c_str());

		// 绑定Trader实例
		CTraderHandler sh(m_pApi);

		// 注册Trader实例
		m_pApi->RegisterSpi(&sh);

		// 订阅公有流
		m_pApi->SubscribePrivateTopic(THOST_TERT_QUICK);
		// 订阅私有流
		m_pApi->SubscribePublicTopic(THOST_TERT_QUICK);

		// 注册前台
		m_pApi->RegisterFront((char*)getConfig("config", "FrontAddr").c_str());

		// 获取当前接口版本
		cout << m_pApi->GetApiVersion() << endl;


		m_pApi->Init();
		m_pApi->Release();
	}
}
