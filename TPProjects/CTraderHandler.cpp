// CTraderHandler.h 通过 #include "ThostFtdcTraderApi.h"继承CThostFtdcTraderSpi
// 因而该类对其CThostFtdcTraderSpi方法进行实现
#include "CTraderHandler.h"
#include "FileReader.h"

CTraderHandler::CTraderHandler(CThostFtdcTraderApi* pUserTraderApi) {
	this->pUserTraderApi = pUserTraderApi;
	requestIndex = 0;
}

CTraderHandler::~CTraderHandler() {
	pUserTraderApi->Release();
}

string CTraderHandler::getExchangeId(string instrumentId) {
	auto iter = instrumentsExchange.find(instrumentId);
	if (iter == instrumentsExchange.end()) {
		return "";
	}
	else {
		return iter->second;
	}
}

// 客户端认证
void CTraderHandler::ReqAuthenticate()
{
	CThostFtdcReqAuthenticateField authField = { 0 };

	strcpy_s(authField.AuthCode, getConfig("config", "AuthCode").c_str());
	strcpy_s(authField.AppID, getConfig("config", "AppID").c_str());
	strcpy_s(authField.BrokerID, getConfig("config", "BrokerID").c_str());
	strcpy_s(authField.UserID, getConfig("config", "InvestorID").c_str());

	// 客户端认证
	// 回调函数：OnRspAuthenticate
	int b = pUserTraderApi->ReqAuthenticate(&authField, requestIndex++);

	std::cout << "客户端认证 = "  << b << endl;
}

// 构建集合竞价报单
CThostFtdcInputOrderField CTraderHandler::composeAuctionInputOrder(string instrumentID, string exchangeID, bool buyIn, int vol, double price, int requestId) {
	return composeInputOrder(instrumentID, exchangeID, buyIn, vol, price, THOST_FTDC_TC_GFD, requestId);
}

// 构建滑点报单
CThostFtdcInputOrderField CTraderHandler::composeSlipInputOrder(string instrumentID, string exchangeID, bool buyIn, int vol, double price, int requestId) {
	return composeInputOrder(instrumentID, exchangeID, buyIn, vol, price, THOST_FTDC_TC_GFD, requestId);
}

// 构建报单
CThostFtdcInputOrderField CTraderHandler::composeInputOrder(string instrumentID, string exchangeID, bool buyIn, int vol, double price,
	char timeCondition, int requestId) {
	CThostFtdcInputOrderField inputOrderField;
	//将某一块内存中的内容全部设置为指定的值，初始化ord
	memset(&inputOrderField, 0, sizeof(inputOrderField));
	// 经纪公司代号
	strcpy_s(inputOrderField.BrokerID, getConfig("config", "BrokerID").c_str());
	// 投资者代号
	strcpy_s(inputOrderField.InvestorID, getConfig("config", "InvestorID").c_str());
	///交易所代码
	strcpy_s(inputOrderField.ExchangeID, exchangeID.c_str());
	// 合约代号
	strcpy_s(inputOrderField.InstrumentID, instrumentID.c_str());
	std::cout << "InstrumentID: " << inputOrderField.InstrumentID << std::endl;
	// 用户代号
	strcpy_s(inputOrderField.UserID, getConfig("config", "InvestorID").c_str());
	// 报单引用
	strcpy_s(inputOrderField.OrderRef, "");
	///买卖方向
	if (buyIn) {
		inputOrderField.Direction = THOST_FTDC_D_Buy;
	}
	else {
		inputOrderField.Direction = THOST_FTDC_D_Sell;
	}
	///组合开平标志
	inputOrderField.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	///组合投机套保标志
	inputOrderField.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	///数量
	inputOrderField.VolumeTotalOriginal = vol;
	///成交量类型
	inputOrderField.VolumeCondition = THOST_FTDC_VC_AV;
	///最小成交量
	inputOrderField.MinVolume = 1;
	///触发条件
	inputOrderField.ContingentCondition = THOST_FTDC_CC_Immediately;
	///止损价
	inputOrderField.StopPrice = 0;
	///强平原因
	inputOrderField.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	///自动挂起标志
	inputOrderField.IsAutoSuspend = 0;
	///价格
	inputOrderField.LimitPrice = price;
	///报单价格条件
	inputOrderField.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	///有效期类型，用于规定是否为集合竞价
	inputOrderField.TimeCondition = timeCondition;
	inputOrderField.RequestID = requestId;

	return inputOrderField;
}

void CTraderHandler::OnFrontConnected()
{
	std::cout << "Connect Success......" << endl;
	// API连接成功后，调用客户端认证接口
	// 客户端认证接口回调函数：OnRspAuthenticate
	this->ReqAuthenticate();
}

void CTraderHandler::OnFrontDisconnected(int nReason)
{
	std::cout << "OnFrontDisconnected" << endl;
}

void CTraderHandler::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField,
	CThostFtdcRspInfoField* pRspInfo, 
	int nRequestID, 
	bool bIsLast)
{
	std::cout << "Authenticate success......" << endl;

	CThostFtdcReqUserLoginField userField = { 0 };

	strcpy_s(userField.BrokerID, getConfig("config", "BrokerID").c_str());
	strcpy_s(userField.UserID, getConfig("config", "InvestorID").c_str());
	strcpy_s(userField.Password, getConfig("config", "Password").c_str());
	
	// 客户端认证成功后，用户登录
	// 用户登录回调函数：OnRspUserLogin
	int result = pUserTraderApi->ReqUserLogin(&userField, requestIndex++);

	std::cout << result << endl;
}

void CTraderHandler::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
	CThostFtdcRspInfoField* pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	std::cout << "Login success......" << endl;

	char* tradingDay = (char*)pUserTraderApi->GetTradingDay();

	std::cout << "Trading Day: " << tradingDay << endl;

	CThostFtdcSettlementInfoConfirmField confirmField = { 0 };
	strcpy_s(confirmField.BrokerID, getConfig("config", "BrokerID").c_str());
	strcpy_s(confirmField.InvestorID, getConfig("config", "InvestorID").c_str());

	// 用户登录成功后，结算结果确认，在开始每日交易前，必须要先确认，每日确认一次即可
	// 结算结果确认回调函数：OnRspSettlementInfoConfirm
	pUserTraderApi->ReqSettlementInfoConfirm(&confirmField, nRequestID++);
}


void CTraderHandler::beginQuery() {
	int operation = 0;
	std::cout << "请输入选择的操作（\n0.查询账户；\n1.查询持仓；\n2.集合竞价下单；\n3.合约查询样例；）：";
	std::cin >> operation;

	int result = 0;
	CThostFtdcQryTradingAccountField tradingAccountField = { 0 };
	CThostFtdcQryInvestorPositionField investorPositionField = { 0 };
	CThostFtdcInputOrderField inputOrderField = { 0 };

	switch (operation)
	{
	case 0:
		strcpy_s(tradingAccountField.BrokerID, getConfig("config", "BrokerID").c_str());
		strcpy_s(tradingAccountField.InvestorID, getConfig("config", "InvestorID").c_str());
		strcpy_s(tradingAccountField.CurrencyID, "CNY");
		//请求查询资金账户
		result = pUserTraderApi->ReqQryTradingAccount(&tradingAccountField, requestIndex++);

		break;
	case 1:
		strcpy_s(investorPositionField.BrokerID, getConfig("config", "BrokerID").c_str());
		strcpy_s(investorPositionField.InvestorID, getConfig("config", "InvestorID").c_str());

		// 请求查询账户持仓
		result = pUserTraderApi->ReqQryInvestorPosition(&investorPositionField, requestIndex++);

		break;
	case 2:
		// 1. loadInstrumentId()，另起一个线程轮询更新得到最近报价
		// 2. use strategy to generate price
		requestIndex++;
		CThostFtdcInputOrderField inputOrderField = composeAuctionInputOrder("ag1912", "SHFE", 1, 10, 100,requestIndex);
		result = pUserTraderApi->ReqOrderInsert(&inputOrderField, requestIndex);
		/*
		vector<string> instrumentIds = loadInstrumentId();
		while (!instrumentIds.empty()) {
			string item = instrumentIds.back();
			// eg. 5,cs1909,3
			string instrumentId = extractIntrumentId(item);
			CThostFtdcInputOrderField inputOrderField = composeInputOrder(pDepthMarketData, instrumentId);
			// 客户端发出报单录入请求
			result = pUserTraderApi->ReqOrderInsert(&inputOrderField, requestIndex++);
			instrumentIds.pop_back();
		}
		*/
		break;
	case 3:
		queryDepthMarketData("ag1912", "SHFE");
		//bool ret = startPollThread();
		//cout << "start poll thread result" << ret << endl;
		break;
		
	}
	
}

void CTraderHandler::queryTrade(string instId, string exgId) {
	CThostFtdcQryTradeField field = CThostFtdcQryTradeField();
	strcpy_s(field.BrokerID, getConfig("config", "BrokerID").c_str());
	strcpy_s(field.InvestorID, getConfig("config", "InvestorID").c_str());
	strcpy_s(field.InstrumentID, instId.c_str());
	strcpy_s(field.ExchangeID, exgId.c_str());
	// 应该TraderID是required
	int result = pUserTraderApi->ReqQryTrade(&field, ++requestIndex);
}

void CTraderHandler::callAuction() {
	for (auto iter = instrumentOrderMap.begin(); iter != instrumentOrderMap.end(); iter++) {
		string instrumentId = iter->first;
		string exchangeId = getExchangeId(instrumentId);

		auto infoIter = instrumentInfoMap.find(instrumentId);
		if (infoIter == instrumentInfoMap.end()) {
			// 如果instrumentId不在instrumentInfoMap，说明查询数据失败
			continue;
		}
		InstrumentOrderInfo orderInfo = iter->second;
		CThostFtdcDepthMarketDataField* lastestInfo = instrumentInfoMap.find(instrumentId)->second.getInfo();
		// 集合竞价的price待定
		CThostFtdcInputOrderField order = composeAuctionInputOrder(instrumentId, exchangeId, orderInfo.buyOrSell(), orderInfo.getVol(), lastestInfo->OpenPrice, requestIndex);
		int result = 0;
		int retry = 3;
		while (retry-->0)
		{
			cout << instrumentId << " 集合下单一次" << endl;
			result = pUserTraderApi->ReqOrderInsert(&order, ++requestIndex);
			if (0 == result) {
				toInsertIns.insert(pair<string, int>(instrumentId, 1));
				break;
			}
			else {
				if (-1 == result) {
					cout << "集合竞价下单，网络连接失败" << endl;
				}
				//-1,-2,-3 三种情况都等一秒之后重试
				this_thread::sleep_for(chrono::milliseconds(1000));
			}
		}
		if (result < 0) {
			cout << instrumentId << " 集合下单失败" << endl;
		}
		this_thread::sleep_for(chrono::milliseconds(200));
	}
}

bool CTraderHandler::startPollThread() {
	try {
		// 这一行实在看不懂，网上查来的
		thread t(&CTraderHandler::poll,this);
		//t.join();
		t.detach();
	}
	catch (exception ex) {
		cerr << "start poll thread failed" << endl;
		return false;
	}
	return true;
}

bool CTraderHandler::startScanThread() {
	try {
		// 这一行实在看不懂，网上查来的
		thread t(&CTraderHandler::scanOngoingOrderStatus, this);
		//t.join();
		t.detach();
	}
	catch (exception ex) {
		cerr << "start poll thread failed" << endl;
		return false;
	}
	return true;
}

void CTraderHandler::poll() {
	string instrument = allInstruments.at(insQueryId%allInstruments.size());
	auto iter = instrumentsExchange.find(instrument);
	queryDepthMarketData(instrument, iter->second);
}

vector<string> CTraderHandler::loadInstruments() {
	vector<string> content;
	bool readSucc = loadFile2Vector("doc1.log", content);
	if (!readSucc) {
		cout << "load Instrument Doc FAILED" << endl;
	}
	else {
		cout << "load Instrument Doc succeed!" << endl;
		// load数据进入内存
		for (size_t i = 0; i < content.size(); i++)
		{
			vector<string> arr = split(content.at(i), ",");
			allInstruments.push_back(arr.at(1));
			instrumentsExchange.insert(pair<string, string>(arr.at(1), arr.at(2)));
			instrumentOrderMap.insert(pair<string, InstrumentOrderInfo>(arr.at(1), InstrumentOrderInfo(arr.at(3))));
		}
	}
	return content;
}

// 请求查询行情响应。当客户端发出请求查询行情指令后，交易托管系统返回响应时，该方法会被调用。
void CTraderHandler::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	this->pDepthMarketData = pDepthMarketData;
	if (pRspInfo != nullptr) {
		if (pRspInfo != nullptr) {
			std::cout << "错误ID:" << pRspInfo->ErrorID << std::endl;
		}

		if (pRspInfo != nullptr) {
			std::cout << "错误消息:" << pRspInfo->ErrorMsg << std::endl;
		}
	}
	std::cout << "请求序号:" << nRequestID << std::endl;
	if (pDepthMarketData != nullptr) {
		cout << pDepthMarketData->InstrumentID << "查询行情响应" << endl;
		map<string, InstrumentInfo>::iterator it = instrumentInfoMap.find(pDepthMarketData->InstrumentID);
		if (it != instrumentInfoMap.end()) {
			// find one in map
			InstrumentInfo preInfo = it->second;
			if (preInfo.isLatestInfo(nRequestID)) {
				//cout << "Already store the lastest info of " << pDepthMarketData->InstrumentID << endl;
			}
			else {
				preInfo.updateInfo(nRequestID, pDepthMarketData);
				//cout << "Update the info of " << pDepthMarketData->InstrumentID << endl;
			}
		}
		else {
			// not find
			InstrumentInfo info(nRequestID, pDepthMarketData);
			instrumentInfoMap.insert(pair<string, InstrumentInfo>(pDepthMarketData->InstrumentID, info));
			//cout << "Add the info of " << pDepthMarketData->InstrumentID << endl;
		}
	}
	std::cout << "================================================================" << std::endl;
	// 第一轮查询
	if (!startPool) {
		// 如果nRequestID对应请求不是第一次回调，直接返回
		if (onceQueryMarker.find(nRequestID) == onceQueryMarker.end()) {
			return;
		}
		else {
			// 如果是nRequestID对应请求的第一次回调，首先删除
			onceQueryMarker.erase(onceQueryMarker.find(nRequestID));
			insQueryId++;
			if (insQueryId < allInstruments.size()) {
				string instrument = allInstruments.at((insQueryId));
				// 用于QPS控制
				std::this_thread::sleep_for(chrono::milliseconds(100));
				queryDepthMarketData(instrument, getExchangeId(instrument));
			}
			else if (insQueryId == allInstruments.size()) {
				startPool = true;
				cout << "首轮询价完毕\n===========后台轮询查询开始===========" << endl;
				startPollThread();
				cout << "===========集合竞价开始===========" << endl;
				callAuction();
			}
		}
	}// 后台查询
	else {
		// 如果nRequestID对应请求不是第一次回调，直接返回
		if (onceQueryMarker.find(nRequestID) == onceQueryMarker.end()) {
			return;
		}
		else {
			// 如果是nRequestID对应请求的第一次回调，首先删除
			onceQueryMarker.erase(onceQueryMarker.find(nRequestID));
			insQueryId++;
			string instrument = allInstruments.at((insQueryId % allInstruments.size()));
			// 用于QPS控制
			std::this_thread::sleep_for(chrono::milliseconds(500));
			 queryDepthMarketData(instrument, getExchangeId(instrument));
		}
	}
	
	/**
	std::cout << "交易日:" << pDepthMarketData->TradingDay << std::endl;
	std::cout << "合约代码:" << pDepthMarketData->InstrumentID << std::endl;
	std::cout << "交易所代码:" << pDepthMarketData->ExchangeID << std::endl;
	std::cout << "合约在交易所的代码:" << pDepthMarketData->ExchangeInstID << std::endl;
	std::cout << "最新价:" << pDepthMarketData->LastPrice << std::endl;
	std::cout << "上次结算价:" << pDepthMarketData->PreSettlementPrice << std::endl;
	std::cout << "昨收盘:" << pDepthMarketData->PreClosePrice << std::endl;
	std::cout << "昨持仓量:" << pDepthMarketData->PreOpenInterest << std::endl;
	std::cout << "今开盘:" << pDepthMarketData->OpenPrice << std::endl;
	std::cout << "最高价:" << pDepthMarketData->HighestPrice << std::endl;
	std::cout << "最低价:" << pDepthMarketData->LowestPrice << std::endl;
	std::cout << "数量:" << pDepthMarketData->Volume << std::endl;
	std::cout << "成交金额:" << pDepthMarketData->Turnover << std::endl;
	std::cout << "持仓量:" << pDepthMarketData->OpenInterest << std::endl;
	std::cout << "今收盘:" << pDepthMarketData->ClosePrice << std::endl;
	std::cout << "本次结算价:" << pDepthMarketData->SettlementPrice << std::endl;
	std::cout << "涨停板价:" << pDepthMarketData->UpperLimitPrice << std::endl;
	std::cout << "跌停板价:" << pDepthMarketData->LowerLimitPrice << std::endl;
	std::cout << "昨虚实度:" << pDepthMarketData->PreDelta << std::endl;
	std::cout << "今虚实度:" << pDepthMarketData->CurrDelta << std::endl;
	std::cout << "最后修改时间:" << pDepthMarketData->UpdateTime << std::endl;
	std::cout << "申买价一:" << pDepthMarketData->BidPrice1 << std::endl;
	std::cout << "申买量一:" << pDepthMarketData->BidVolume1 << std::endl;
	std::cout << "申卖价一:" << pDepthMarketData->AskPrice1 << std::endl;
	std::cout << "申卖量一:" << pDepthMarketData->AskVolume1 << std::endl;
	std::cout << "申买价二:" << pDepthMarketData->BidPrice2 << std::endl;
	std::cout << "申买量二:" << pDepthMarketData->BidVolume2 << std::endl;
	std::cout << "申卖价二:" << pDepthMarketData->AskPrice2 << std::endl;
	std::cout << "申卖量二:" << pDepthMarketData->AskVolume2 << std::endl;
	std::cout << "申买价三:" << pDepthMarketData->BidPrice3 << std::endl;
	std::cout << "申买量三:" << pDepthMarketData->BidVolume3 << std::endl;
	std::cout << "申卖价三:" << pDepthMarketData->AskPrice3 << std::endl;
	std::cout << "申卖量三:" << pDepthMarketData->AskVolume3 << std::endl;
	std::cout << "申买价四:" << pDepthMarketData->BidPrice4 << std::endl;
	std::cout << "申买量四:" << pDepthMarketData->BidVolume4 << std::endl;
	std::cout << "申卖价四:" << pDepthMarketData->AskPrice4 << std::endl;
	std::cout << "申卖量四:" << pDepthMarketData->AskVolume4 << std::endl;
	std::cout << "申买价五:" << pDepthMarketData->BidPrice5 << std::endl;
	std::cout << "申买量五:" << pDepthMarketData->BidVolume5 << std::endl;
	std::cout << "申卖价五:" << pDepthMarketData->AskPrice5 << std::endl;
	std::cout << "申卖量五:" << pDepthMarketData->AskVolume5 << std::endl;
	std::cout << "================================================================" << std::endl;
	**/
	//beginQuery();
}

void CTraderHandler::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "查询交易账户响应......" << std::endl;
	if (pRspInfo != nullptr) {
		if (pRspInfo != nullptr) {
			std::cout << "错误ID:" << pRspInfo->ErrorID << std::endl;
		}

		if (pRspInfo != nullptr) {
			std::cout << "错误消息:" << pRspInfo->ErrorMsg << std::endl;
		}
	}
	std::cout << "请求序号:" << nRequestID << std::endl;
	std::cout << "IsLast:" << bIsLast << std::endl;

	std::cout << "================================================================" << std::endl;
	std::cout << "经纪公司代码：" << pTradingAccount->BrokerID << endl;
	std::cout << "投资者账号：" << pTradingAccount->AccountID << endl;
	std::cout << "可用资金：" << pTradingAccount->Available << endl;
	std::cout << "入金金额：" << pTradingAccount->Deposit << endl;
	std::cout << "出金金额：" << pTradingAccount->Withdraw << endl;
	std::cout << "================================================================" << std::endl;

	//beginQuery();
}

bool CTraderHandler::isSlipOrder(CThostFtdcOrderField* pOrder) {
	return auctionOverFlag && pOrder->RequestID > auctionLastReqId;
}

// 报单回报。当客户端进行报单录入、报单操作及其它原因（如部分成交）导致报单状态发生变化时，交易托管系统会主动通知客户端，该方法会被调用
// insertOrder, order traded, order canceled 均可能回调该函数
void CTraderHandler::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
	std::cout << "================================================================" << std::endl;
	std::cout << "OnRtnOrder is called" << std::endl;

	string insId = pOrder->InstrumentID;
	cout << "instrumentid is " << insId << endl;
	cout << "OrderSubmitStatus is " << pOrder->OrderSubmitStatus << endl;
	cout << "OrderStatus is " << pOrder->OrderStatus << endl;

	// 报单成功
	// 验证发现报单后第一次回调：pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted, pOrder->OrderStatus == THOST_FTDC_OST_Unknown
	// 判断方法待明确
	if (pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted) {
		if (ongoingIns.find(insId) == ongoingIns.end()) {
			if (toInsertIns.find(insId) != toInsertIns.end()) {
				toInsertIns.erase(insId);
			}
			else {
				cout << insId << " is not existed in toInsertIns, ongoingIns when OnRtnOrder is called and OrderSubmitStatus is THOST_FTDC_OSS_InsertSubmitted " << endl;
			}
			cout << insId << " 报单成功" << endl;
			ongoingIns.insert(pair<string, CThostFtdcOrderField*>(insId, pOrder));
		}
		if (0 == toInsertIns.size() && !auctionOverFlag) {
			cout << "集合竞价阶段报单完成" << endl;
			auctionOverFlag = true;
			// 集合竞价之后需要等待一段时间
			this_thread::sleep_for(chrono::milliseconds(1000));
			cout << "开始扫描合约状态" << endl;
			scanOngoingOrderStatus();
			slipStartFlag = true;
		}
	}

	// 撤单成功
	// 报单自动撤销后回调发现：pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertRejected, pOrder->OrderStatus == THOST_FTDC_OST_Canceled
	// 判断方法待明确
	else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled) {
		// 如果 pOrder->InstrumentID 不在cancelledIns队列中，需要从其他队列中移除，然后添入
		if (cancelledIns.find(insId) == cancelledIns.end()) {
			if (ongoingIns.find(insId) != ongoingIns.end()) {
				ongoingIns.erase(ongoingIns.find(insId));
			}
			else if (toInsertIns.find(insId) != toInsertIns.end()) {
				toInsertIns.erase(toInsertIns.find(insId));
			}
			else {
				cout << insId << " is not existed in toInsertIns, ongoingIns, cancelledIns when OnRtnOrder is called and OrderStatus is THOST_FTDC_OST_Canceled " << endl;
			}
			cout << insId << " 已撤单" << endl;
			cancelledIns.insert(pair<string, int>(insId, 1));
		}
		if (slipStartFlag) {
			string exchangeId = getExchangeId(insId);

			auto infoIter = instrumentInfoMap.find(insId);
			if (infoIter == instrumentInfoMap.end()) {
				// 如果instrumentId不在instrumentInfoMap，说明查询数据失败
				cout << insId << " 在instrumentInfoMap 中不存在" << endl;
			}
			InstrumentOrderInfo orderInfo = instrumentOrderMap.find(insId)->second;
			CThostFtdcDepthMarketDataField* lastestInfo = instrumentInfoMap.find(insId)->second.getInfo();
			// 滑点的price待定
			CThostFtdcInputOrderField order = composeSlipInputOrder(insId, exchangeId, orderInfo.buyOrSell(), orderInfo.getVol(), lastestInfo->OpenPrice, requestIndex);
			int result = 0;
			int retry = 3;
			while (retry-- > 0)
			{
				cout << insId << " 滑点下单一次" << endl;
				result = pUserTraderApi->ReqOrderInsert(&order, ++requestIndex);
				if (0 == result) {
					toInsertIns.insert(pair<string, int>(insId, 1));
					break;
				}
				else {
					if (-1 == result) {
						cout << "滑点下单，网络连接失败" << endl;
					}
					//-1,-2,-3 三种情况都等一秒之后重试
					this_thread::sleep_for(chrono::milliseconds(1000));
				}
			}
			if (result < 0) {
				cout << insId << " 滑点下单失败" << endl;
				toInsertIns.insert(pair<string, int>(insId, 1));
			}
			this_thread::sleep_for(chrono::milliseconds(200));
			cancelledIns.erase(insId);
		}
	}
	// 合约单成交
	// 看代码解释，未验证
	else if(pOrder->OrderStatus == THOST_FTDC_OST_AllTraded){
		// 如果 pOrder->InstrumentID 不在bingoIns队列中
		if (bingoIns.find(insId) == bingoIns.end()) {
			// 如果存在于toInsertIns队列中
			if (toInsertIns.find(insId) != toInsertIns.end()) {
				toInsertIns.erase(toInsertIns.find(insId));
			}// 如果存在于ongoingIns队列中
			else if (ongoingIns.find(insId) != ongoingIns.end()) {
				ongoingIns.erase(ongoingIns.find(insId));
			}
			else {
				cout << insId << " is not existed in toInsertIns, ongoingIns, bingoIns when OnRtnOrder is called and OrderStatus is THOST_FTDC_OST_AllTraded " << endl;
			}
			bingoIns.insert(pair<string, int>(insId, 1));
			cout << insId << " 已成交" << endl;
		}
	}

	return;

	/*
	if (isSlipOrder(pOrder)) {
		std::cout << "滑点报单成功" << std::endl;
		string instrument = pOrder->InstrumentID;
		auto it = bingoSlipInstruments.find(instrument);
		if (it == bingoSlipInstruments.end()) {
			// 不重复添加
			bingoSlipInstruments.insert(pair<string, int>(instrument, 1));
		}
	}
	else {
		std::cout << "集合竞价报单成功" << std::endl;
	}
	// 只需要删除即可
	ongoingInstruments.erase(pOrder->RequestID);
	
	std::cout << "合约代码：" << pOrder->InstrumentID << endl;
	std::cout << "买卖方向：" << pOrder->Direction << endl;
	std::cout << "价格：" << pOrder->LimitPrice << endl;
	std::cout << "数量：" << pOrder->VolumeTotalOriginal << endl;

	if (0 == ongoingInstruments.size()) {
		auctionLastReqId = pOrder->RequestID;
		cout << "==============集合竞价报单完成==================" << endl;
		cout << "==============滑点报单开始==================" << endl;
		if (!auctionOverFlag) {
			callSlippage();
		}
		auctionOverFlag = true;
	}
	*/
	
	/*
	std::cout << "经纪公司代码：" << pOrder->BrokerID << endl;
	std::cout << "投资者代码：" << pOrder->InvestorID << endl;
	
	std::cout << "报单引用：" << pOrder->OrderRef << endl;
	std::cout << "用户代码：" << pOrder->UserID << endl;
	std::cout << "报单价格条件：" << pOrder->OrderPriceType << endl;
	
	std::cout << "组合开平标志：" << pOrder->CombOffsetFlag << endl;
	std::cout << "组合投机套保标志：" << pOrder->CombHedgeFlag << endl;
	
	
	std::cout << "有效期类型：" << pOrder->TimeCondition << endl;
	std::cout << "GTD日期：" << pOrder->GTDDate << endl;
	std::cout << "成交量类型：" << pOrder->VolumeCondition << endl;
	std::cout << "最小成交量：" << pOrder->MinVolume << endl;
	std::cout << "触发条件：" << pOrder->ContingentCondition << endl;
	std::cout << "止损价：" << pOrder->StopPrice << endl;
	std::cout << "强平原因：" << pOrder->ForceCloseReason << endl;
	std::cout << "自动挂起标志：" << pOrder->IsAutoSuspend << endl;
	std::cout << "业务单元：" << pOrder->BusinessUnit << endl;
	std::cout << "请求编号：" << pOrder->RequestID << endl;
	std::cout << "本地报单编号：" << pOrder->OrderLocalID << endl;
	std::cout << "交易所代码：" << pOrder->ExchangeID << endl;
	std::cout << "会员代码：" << pOrder->ParticipantID << endl;
	std::cout << "客户代码：" << pOrder->ClientID << endl;
	std::cout << "合约在交易所的代码：" << pOrder->ExchangeInstID << endl;
	std::cout << "交易所交易员代码：" << pOrder->TraderID << endl;
	std::cout << "安装编号：" << pOrder->InstallID << endl;
	std::cout << "报单提交状态：" << pOrder->OrderSubmitStatus << endl;
	std::cout << "报单提示序号：" << pOrder->NotifySequence << endl;
	std::cout << "交易日：" << pOrder->TradingDay << endl;
	std::cout << "结算编号：" << pOrder->SettlementID << endl;
	std::cout << "报单编号：" << pOrder->OrderSysID << endl;
	std::cout << "报单来源：" << pOrder->OrderSource << endl;
	std::cout << "报单状态：" << pOrder->OrderStatus << endl;
	std::cout << "报单类型：" << pOrder->OrderType << endl;
	std::cout << "今成交数量：" << pOrder->VolumeTraded << endl;
	std::cout << "剩余数量：" << pOrder->VolumeTotal << endl;
	std::cout << "报单日期：" << pOrder->InsertDate << endl;
	std::cout << "委托时间：" << pOrder->InsertTime << endl;
	std::cout << "激活时间：" << pOrder->ActiveTime << endl;
	std::cout << "挂起时间：" << pOrder->SuspendTime << endl;
	std::cout << "最后修改时间：" << pOrder->UpdateTime << endl;
	std::cout << "撤销时间：" << pOrder->CancelTime << endl;
	std::cout << "最后修改交易所交易员代码：" << pOrder->ActiveTraderID << endl;
	std::cout << "结算会员编号：" << pOrder->ClearingPartID << endl;
	std::cout << "序号：" << pOrder->SequenceNo << endl;
	std::cout << "前置编号：" << pOrder->FrontID << endl;
	std::cout << "会话编号：" << pOrder->SessionID << endl;
	std::cout << "用户端产品信息：" << pOrder->UserProductInfo << endl;
	std::cout << "状态信息：" << pOrder->StatusMsg << endl;
	std::cout << "用户强评标志：" << pOrder->UserForceClose << endl;
	std::cout << "操作用户代码：" << pOrder->ActiveUserID << endl;
	std::cout << "经纪公司报单编号：" << pOrder->BrokerOrderSeq << endl;
	std::cout << "相关报单：" << pOrder->RelativeOrderSysID << endl;
	std::cout << "郑商所成交数量：" << pOrder->ZCETotalTradedVolume << endl;
	std::cout << "互换单标志：" << pOrder->IsSwapOrder << endl;
	std::cout << "营业部编号：" << pOrder->BranchID << endl;
	std::cout << "投资单元代码：" << pOrder->InvestUnitID << endl;
	std::cout << "资金账号：" << pOrder->AccountID << endl;
	std::cout << "币种代码：" << pOrder->CurrencyID << endl;
	std::cout << "IP地址：" << pOrder->IPAddress << endl;
	std::cout << "Mac地址：" << pOrder->MacAddress << endl;
	*/
	std::cout << "================================================================" << std::endl;
}
// 成交回报。当发生成交时交易托管系统会通知客户端，该方法会被调用
void CTraderHandler::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
	cout << "================================================================" << std::endl;
	cout << "OnRtnTrade is called" << std::endl;
	string insId = pTrade->InstrumentID;
	// 如果 pOrder->InstrumentID 不在bingoIns队列中
	if (bingoIns.find(insId) == bingoIns.end()) {
		// 如果存在于toInsertIns队列中
		if (toInsertIns.find(insId) != toInsertIns.end()) {
			toInsertIns.erase(toInsertIns.find(insId));
		}// 如果存在于ongoingIns队列中
		else if (ongoingIns.find(insId) != ongoingIns.end()) {
			ongoingIns.erase(ongoingIns.find(insId));
		}
		else {
			cout << insId << " is not existed in toInsertIns, ongoingIns, bingoIns when OnRtnTrade is called " << endl;
		}
		bingoIns.insert(pair<string, int>(insId, 1));
		cout << insId << " 已成交" << endl;
	}
	/*
	std::cout << "经纪公司代码：" << pTrade->BrokerID << endl;
	std::cout << "投资者代码：" << pTrade->InvestorID << endl;
	std::cout << "合约代码：" << pTrade->InstrumentID << endl;
	std::cout << "报单引用：" << pTrade->OrderRef << endl;
	std::cout << "用户代码：" << pTrade->UserID << endl;
	std::cout << "交易所代码：" << pTrade->ExchangeID << endl;
	std::cout << "成交编号：" << pTrade->TradeID << endl;
	std::cout << "买卖方向：" << pTrade->Direction << endl;
	std::cout << "报单编号：" << pTrade->OrderSysID << endl;
	std::cout << "会员代码：" << pTrade->ParticipantID << endl;
	std::cout << "客户代码：" << pTrade->ClientID << endl;
	std::cout << "交易角色：" << pTrade->TradingRole << endl;
	std::cout << "合约在交易所的代码：" << pTrade->ExchangeInstID << endl;
	std::cout << "开平标志：" << pTrade->OffsetFlag << endl;
	std::cout << "投机套保标志：" << pTrade->HedgeFlag << endl;
	std::cout << "价格：" << pTrade->Price << endl;
	std::cout << "数量：" << pTrade->Volume << endl;
	std::cout << "成交时期：" << pTrade->TradeDate << endl;
	std::cout << "成交时间：" << pTrade->TradeTime << endl;
	std::cout << "成交类型：" << pTrade->TradeType << endl;
	std::cout << "成交价来源：" << pTrade->PriceSource << endl;
	std::cout << "交易所交易员代码：" << pTrade->TraderID << endl;
	std::cout << "本地报单编号：" << pTrade->OrderLocalID << endl;
	std::cout << "结算会员编号：" << pTrade->ClearingPartID << endl;
	std::cout << "业务单元：" << pTrade->BusinessUnit << endl;
	std::cout << "序号：" << pTrade->SequenceNo << endl;
	std::cout << "交易日：" << pTrade->TradingDay << endl;
	std::cout << "结算编号：" << pTrade->SettlementID << endl;
	std::cout << "经纪公司报单编号：" << pTrade->BrokerOrderSeq << endl;
	std::cout << "成交来源：" << pTrade->TradeSource << endl;
	std::cout << "投资单元代码：" << pTrade->InvestUnitID << endl;
	*/
	std::cout << "================================================================" << std::endl;
}

// 报单录入应答。 当客户端发出过报单录入指令后， 交易托管系统返回响应时，该方法会被调用
void CTraderHandler::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "OnRspOrderInsert is called" << std::endl;
	std::cout << "================================================================" << std::endl;	
	if (pRspInfo != nullptr) {
		std::cout << "OnRspOrderInsert says it's failed" << std::endl;
		std::cout << "错误代码" << pRspInfo->ErrorID << endl;
		std::cout << "错误信息" << pRspInfo->ErrorMsg << endl;
		// 直接从toInsertIns删除
		auto it = toInsertIns.find(pInputOrder->InstrumentID);
		if (it != toInsertIns.end()) {
			toInsertIns.erase(it);
		}
	}
	else if(pInputOrder != nullptr){
		// 但是结果中未有
		cout << "OnRspOrderInsert says insert " << pInputOrder->InstrumentID << " order success" << endl;
	}
	
	std::cout << "================================================================" << std::endl;
}

void CTraderHandler::scanOngoingOrderStatus() {
	for (map<string, CThostFtdcOrderField*>::iterator it = ongoingIns.begin(); it != ongoingIns.end(); it++) {
		CThostFtdcQryOrderField field = {0};
		// 文档注释里说，“不写 BrokerID 可以收全所有报单。” 不懂什么意思
		strcpy_s(field.BrokerID, getConfig("config", "BrokerID").c_str());
		strcpy_s(field.InvestorID, getConfig("config", "InvestorID").c_str());
		strcpy_s(field.InstrumentID, it->first.c_str());
		strcpy_s(field.OrderSysID, it->second->OrderSysID);
		int result = 0;
		int retry = 3;
		while (--retry > 0) {
			result = pUserTraderApi->ReqQryOrder(&field, ++requestIndex);
			if (0 == result) {
				break;
			}
			else {
				if (result == -1) {
					cout << "ReqQryOrder encountered 网络连接失败" << endl;
				}
				this_thread::sleep_for(chrono::milliseconds(1000));
			}
		}
		// 此处经常result=-2，表示未处理请求超过许可数
		if (result < 0) {
			cout << it->first << " 订单状态查询失败, result = "<<result << endl;
		}
		// 暂停0.2s 用来控QPS
		this_thread::sleep_for(chrono::milliseconds(200));
	}
}

void CTraderHandler::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	cout << "OnRspQryOrder is called" << endl;
	if (pRspInfo != nullptr) {
		std::cout << "错误代码：" << pRspInfo->ErrorID << endl;
		std::cout << "错误信息：" << pRspInfo->ErrorMsg << endl;
	}
	if (pOrder != nullptr) {
		string insId = pOrder->InstrumentID;
		// ongoingIns 中合约状态迁移
		if (ongoingIns.find(insId) != ongoingIns.end()) {
			
			// 已成交
			if(THOST_FTDC_OST_AllTraded == pOrder->OrderStatus) {
				ongoingIns.erase(ongoingIns.find(insId));
				bingoIns.insert(pair<string, int>(insId, 1));
			}// 已撤单
			else if (THOST_FTDC_OST_Canceled == pOrder->OrderStatus) {
				ongoingIns.erase(ongoingIns.find(insId));
				cancelledIns.insert(pair<string, int>(insId, 1));
			}
			// 否则手动撤单
			else {
				CThostFtdcInputOrderActionField field = {};
				strcpy_s(field.BrokerID, getConfig("config", "BrokerID").c_str());
				strcpy_s(field.InvestorID, getConfig("config", "InvestorID").c_str());
				strcpy_s(field.OrderSysID, pOrder->OrderSysID);
				field.ActionFlag = THOST_FTDC_AF_Delete;
				field.FrontID = pOrder->FrontID;
				field.SessionID = pOrder->SessionID;
				strcpy_s(field.OrderRef, pOrder->OrderRef);
				strcpy_s(field.ExchangeID, pOrder->ExchangeID);
				int result = 0;
				int retry = 3;
				while (retry-- > 0) {
					result = pUserTraderApi->ReqOrderAction(&field, ++requestIndex);
					if (0 == result) {
						break;
					}
					if (result < 0) {
						if (-1 == result) {
							cout << "call ReqOrderAction 网络连接失败" << endl;
						}
						this_thread::sleep_for(chrono::milliseconds(1000));
					}
				}
				// 重试三次，撤单指令发送失败
				if (result < 0) {
					cout << "重试三次，撤单 "<<insId<<" 指令发送失败" << endl;
					// 撤单失败，输出记录，不再继续处理
					ongoingIns.erase(ongoingIns.find(insId));
				}
			}

		}
	}
}

/*
Thost 收到撤单指令，如果没有通过参数校验，拒绝接受撤单指令。用户就会收到
OnRspOrderAction 消息，其中包含了错误编码和错误消息
*/
void CTraderHandler::OnRspOrderAction(CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	cout << "OnRspOrderAction is called" << endl;
	if (pRspInfo != nullptr) {
		cout << "错误代码：" << pRspInfo->ErrorID << endl;
		cout << "错误信息：" << pRspInfo->ErrorMsg << endl;
	}
	if (pOrderAction != nullptr) {
		string insId = pOrderAction->InstrumentID;
		// 理论上不会被调到，暂不写处理逻辑
		cout << insId << endl;
	}
}

void CTraderHandler::OnErrRtnOrderAction(
	CThostFtdcOrderActionField* pOrderAction,
	CThostFtdcRspInfoField* pRspInfo) {
	cout << "OnErrRtnOrderAction is called" << endl;
	if (pRspInfo != nullptr) {
		cout << "错误代码：" << pRspInfo->ErrorID << endl;
		cout << "错误信息：" << pRspInfo->ErrorMsg << endl;
	}
	if (pOrderAction != nullptr) {
		string insId = pOrderAction->InstrumentID;
		// 理论上不会被调到，暂不写处理逻辑
		cout << insId << endl;
	}
}

void CTraderHandler::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo)
{
	std::cout << "OnErrRtnOrderInsert is called" << std::endl;
	std::cout << "================================================================" << std::endl;
	if (pRspInfo != nullptr) {
		std::cout << "错误代码" << pRspInfo->ErrorID << endl;
		std::cout << "错误信息" << pRspInfo->ErrorMsg << endl;
		// 直接从toInsertIns删除
		auto it = toInsertIns.find(pInputOrder->InstrumentID);
		if (it != toInsertIns.end()) {
			toInsertIns.erase(it);
		}
	}
	std::cout << "================================================================" << std::endl;
}

void CTraderHandler::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "=============加载文件 doc1.log=============" << std::endl;
	loadInstruments();
	std::cout << "=============开始查询合约信息=============" << std::endl;

	// 从文件的第一条合约单开始
	string instrument = allInstruments.at(0);
	queryDepthMarketData(instrument,getExchangeId(instrument));

	//callSlippage();
	//beginQuery();
}


void CTraderHandler::queryDepthMarketData(string instrumentId, string exchangeId) {
	cout << "query id: " << instrumentId << endl;
	CThostFtdcQryDepthMarketDataField instrumentField = { 0 };

	strcpy_s(instrumentField.InstrumentID, instrumentId.c_str());

	strcpy_s(instrumentField.ExchangeID, exchangeId.c_str());

	// 投资者结算结果确认后，查询合约深度行情
	// 查询合约深度行情回调函数：OnRspQryDepthMarketData
	int result = pUserTraderApi->ReqQryDepthMarketData(&instrumentField, ++requestIndex);
	if (0 == result) {
		// 发送查询成功
		onceQueryMarker.insert(pair<int, string>(requestIndex, instrumentId));
		return;
	}
	else {
		// 发送查询失败
		if (-1 == result) {
			//-1，表示网络连接失败
			cout << "网络连接失败，导致合约信息查询失败" << endl;
		}
		else {
			// -2，表示未处理请求超过许可数；
			// -3，表示每秒发送请求数超过许可数
			// 等待1s 后重试
			this_thread::sleep_for(chrono::milliseconds(1000));
			queryDepthMarketData(instrumentId, exchangeId);
		}
	}
}


///请求查询行情
/**
0，代表成功。
-1，表示网络连接失败；
-2，表示未处理请求超过许可数；
-3，表示每秒发送请求数超过许可数。
int CTraderHandler::ReqQryDepthMarketData(CThostFtdcQryDepthMarketDataField* pQryDepthMarketData, int nRequestID) {
	int result = pUserTraderApi->ReqQryDepthMarketData(pQryDepthMarketData, requestIndex++);
	std::cout << " query resCode:" << result << std::endl;
	return result;
}
**/

void CTraderHandler::OnRspQryInvestorPosition(
	CThostFtdcInvestorPositionField* pInvestorPosition,
	CThostFtdcRspInfoField* pRspInfo,
	int nRequestID,
	bool bIsLast
)
{
	std::cout << "查询持仓成功" << endl;

	if (pRspInfo != nullptr) {
		std::cout << "错误代码：" << pRspInfo->ErrorID << endl;
		std::cout << "错误信息：" << pRspInfo->ErrorMsg << endl;
	}
	std::cout << "================================================================" << std::endl;
	if (pInvestorPosition != nullptr) {

		std::cout << "合约代码：" << pInvestorPosition->InstrumentID << std::endl;
		std::cout << "经纪公司代码：" << pInvestorPosition->BrokerID << std::endl;
		std::cout << "投资者代码：" << pInvestorPosition->InvestorID << std::endl;
		std::cout << "持仓多空方向：" << pInvestorPosition->PosiDirection << std::endl;
		std::cout << "投机套保标志：" << pInvestorPosition->HedgeFlag << std::endl;
		std::cout << "持仓日期：" << pInvestorPosition->PositionDate << std::endl;
		std::cout << "上日持仓：" << pInvestorPosition->YdPosition << std::endl;
		std::cout << "今日持仓：" << pInvestorPosition->Position << std::endl;
		std::cout << "多头冻结：" << pInvestorPosition->LongFrozen << std::endl;
		std::cout << "空头冻结：" << pInvestorPosition->ShortFrozen << std::endl;
		std::cout << "开仓冻结金额：" << pInvestorPosition->LongFrozenAmount << std::endl;
		std::cout << "开仓冻结金额：" << pInvestorPosition->ShortFrozenAmount << std::endl;
		std::cout << "开仓量：" << pInvestorPosition->OpenVolume << std::endl;
		std::cout << "平仓量：" << pInvestorPosition->CloseVolume << std::endl;
		std::cout << "开仓金额：" << pInvestorPosition->OpenAmount << std::endl;
		std::cout << "平仓金额：" << pInvestorPosition->CloseAmount << std::endl;
		std::cout << "持仓成本：" << pInvestorPosition->PositionCost << std::endl;
		std::cout << "上次占用的保证金：" << pInvestorPosition->PreMargin << std::endl;
		std::cout << "占用的保证金：" << pInvestorPosition->UseMargin << std::endl;
		std::cout << "冻结的保证金：" << pInvestorPosition->FrozenMargin << std::endl;
		std::cout << "冻结的资金：" << pInvestorPosition->FrozenCash << std::endl;
		std::cout << "冻结的手续费：" << pInvestorPosition->FrozenCommission << std::endl;
		std::cout << "资金差额：" << pInvestorPosition->CashIn << std::endl;
		std::cout << "手续费：" << pInvestorPosition->Commission << std::endl;
		std::cout << "平仓盈亏：" << pInvestorPosition->CloseProfit << std::endl;
		std::cout << "持仓盈亏：" << pInvestorPosition->PositionProfit << std::endl;
		std::cout << "上次结算价：" << pInvestorPosition->PreSettlementPrice << std::endl;
		std::cout << "本次结算价：" << pInvestorPosition->SettlementPrice << std::endl;
		std::cout << "交易日：" << pInvestorPosition->TradingDay << std::endl;
		std::cout << "结算编号：" << pInvestorPosition->SettlementID << std::endl;
		std::cout << "开仓成本：" << pInvestorPosition->OpenCost << std::endl;
		std::cout << "交易所保证金：" << pInvestorPosition->ExchangeMargin << std::endl;
		std::cout << "组合成交形成的持仓：" << pInvestorPosition->CombPosition << std::endl;
		std::cout << "组合多头冻结：" << pInvestorPosition->CombLongFrozen << std::endl;
		std::cout << "组合空头冻结：" << pInvestorPosition->CombShortFrozen << std::endl;
		std::cout << "逐日盯市平仓盈亏：" << pInvestorPosition->CloseProfitByDate << std::endl;
		std::cout << "逐笔对冲平仓盈亏：" << pInvestorPosition->CloseProfitByTrade << std::endl;
		std::cout << "今日持仓：" << pInvestorPosition->TodayPosition << std::endl;
		std::cout << "保证金率：" << pInvestorPosition->MarginRateByMoney << std::endl;
		std::cout << "保证金率(按手数)：" << pInvestorPosition->MarginRateByVolume << std::endl;
		std::cout << "执行冻结：" << pInvestorPosition->StrikeFrozen << std::endl;
		std::cout << "执行冻结金额：" << pInvestorPosition->StrikeFrozenAmount << std::endl;
		std::cout << "放弃执行冻结：" << pInvestorPosition->AbandonFrozen << std::endl;
		std::cout << "交易所代码：" << pInvestorPosition->ExchangeID << std::endl;
		std::cout << "执行冻结的昨仓：" << pInvestorPosition->YdStrikeFrozen << std::endl;
		std::cout << "投资单元代码：" << pInvestorPosition->InvestUnitID << std::endl;
	}
	std::cout << "================================================================" << std::endl;
}
