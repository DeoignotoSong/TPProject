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
		
	case 4:
		cout << "req Idx is " << this->requestIndex << endl;
		break;
	case 3:
		queryDepthMarketData("ag1912", "SHFE");
		//bool ret = startPollThread();
		//cout << "start poll thread result" << ret << endl;
		break;
		
	}
	
}

void CTraderHandler::callAuction() {
	for (auto iter = instrumentOrderMap.begin(); iter != instrumentOrderMap.end(); iter++) {
		string instrumentId = iter->first;
		auto infoIter = instrumentInfoMap.find(instrumentId);
		if (infoIter == instrumentInfoMap.end()) {
			// 如果instrumentId不在instrumentInfoMap，说明查询数据失败
			continue;
		}
		InstrumentOrderInfo orderInfo = iter->second;
		string exchangeId = instrumentsExchange.find(instrumentId)->second;
		//  instrumentInfoMap如果没有加载完成，会存在找不到的异常
		CThostFtdcDepthMarketDataField* lastestInfo = instrumentInfoMap.find(instrumentId)->second.getInfo();
		ongoingInstruments.insert(pair<int, string>(++requestIndex, instrumentId));
		// 集合竞价的price待定
		CThostFtdcInputOrderField order = composeAuctionInputOrder(instrumentId, exchangeId, orderInfo.buyOrSell(), orderInfo.getVol(), lastestInfo->OpenPrice, requestIndex);
		int result = pUserTraderApi->ReqOrderInsert(&order, requestIndex);

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
	std::cout <<allInstruments.at(insQueryId % allInstruments.size()) << "  行情查询响应......" << std::endl;
	

	if (pRspInfo != nullptr) {
		if (pRspInfo != nullptr) {
			std::cout << "错误ID:" << pRspInfo->ErrorID << std::endl;
		}

		if (pRspInfo != nullptr) {
			std::cout << "错误消息:" << pRspInfo->ErrorMsg << std::endl;
		}
	}
	if (startPool) {
		cout << "本次查询属后台轮询查询" << endl;
	}
	std::cout << "请求序号:" << nRequestID << std::endl;
	if (pDepthMarketData != nullptr) {
		//cout << "success query one" << endl;
		map<string, InstrumentInfo>::iterator it = instrumentInfoMap.find(pDepthMarketData->InstrumentID);
		if (it != instrumentInfoMap.end()) {
			// find one in map
			InstrumentInfo preInfo = it->second;
			if (preInfo.isLatestInfo(nRequestID)) {
				//cout << "Already store the lastest info of " << pDepthMarketData->InstrumentID << endl;
			}
			else {
				preInfo.updateInfo(nRequestID, pDepthMarketData);
				//cout << "Already update the info of " << pDepthMarketData->InstrumentID << endl;
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
	
	insQueryId++;
	if (startPool) {
		int i = 0;
	}
	if (startPool || insQueryId < allInstruments.size()) {
		string instrument = allInstruments.at((insQueryId % allInstruments.size()));
		auto iter = instrumentsExchange.find(instrument);
		// 用于QPS控制
		std::this_thread::sleep_for(chrono::milliseconds(100));
		queryDepthMarketData(instrument, iter->second);
	}
	if (!startPool && insQueryId == allInstruments.size()) {
		startPool = true;
		cout << "===========后台轮询查询开始===========" << endl;
		startPollThread();
		cout << "===========集合竞价开始===========" << endl;
		callAuction();
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
// 而且貌似会被多次调用到
void CTraderHandler::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
	std::cout << "================================================================" << std::endl;
	std::cout << "OnRtnOrder is called" << std::endl;
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
	std::cout << "OnRtnTrade is called" << std::endl;
	std::cout << "交易成功" << std::endl;
	std::cout << "================================================================" << std::endl;
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
	std::cout << "================================================================" << std::endl;
}

// 报单录入应答。 当客户端发出过报单录入指令后， 交易托管系统返回响应时，该方法会被调用
void CTraderHandler::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "OnRspOrderInsert is called" << std::endl;
	std::cout << "================================================================" << std::endl;
	if (pRspInfo != nullptr) {
		std::cout << "错误代码" << pRspInfo->ErrorID << endl;
		std::cout << "错误信息" << pRspInfo->ErrorMsg << endl;
		auto iter = ongoingInstruments.find(nRequestID);
		// 如果报单失败，将该合约单从ongoing队列移到failed队列，等待后续重试
		if (iter != ongoingInstruments.end()) {
			failedInstruments.push_back(iter->second);
			ongoingInstruments.erase(nRequestID);
		}
	}
	std::cout << "================================================================" << std::endl;
}


void CTraderHandler::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo)
{
	std::cout << "报单错误" << std::endl;
	std::cout << "================================================================" << std::endl;
	if (pRspInfo != nullptr) {
		std::cout << "错误代码" << pRspInfo->ErrorID << endl;
		std::cout << "错误信息" << pRspInfo->ErrorMsg << endl;

	}
	std::cout << "================================================================" << std::endl;
}

void CTraderHandler::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "=============加载文件 doc1.log=============" << std::endl;
	loadInstruments();
	std::cout << "=============开始查询合约信息=============" << std::endl;
	string instrument = allInstruments.at(0);
	auto iter = instrumentsExchange.find(instrument);
	queryDepthMarketData(instrument, iter->second);
	//callSlippage();
	//beginQuery();
}


void CTraderHandler::callSlippage() {
	while (!failedInstruments.empty()) {
		string instrumentId = failedInstruments.at(failedInstruments.size()-1);
		failedInstruments.pop_back();
		if (bingoSlipInstruments.find(instrumentId) != bingoSlipInstruments.end()) {
			// 如果在bingoSlipInstruments中存在，说明已成功
			continue;
		}
		else {
			auto iter = instrumentOrderMap.find(instrumentId);
			InstrumentOrderInfo orderInfo = iter->second;
			string exchangeId = instrumentsExchange.find(instrumentId)->second;
			//  instrumentInfoMap如果没有加载完成，会存在找不到的异常
			CThostFtdcDepthMarketDataField* lastestInfo = instrumentInfoMap.find(instrumentId)->second.getInfo();
			ongoingInstruments.insert(pair<int, string>(++requestIndex, instrumentId));
			// 滑点订单与集合竞价的区别是？
			CThostFtdcInputOrderField order = composeAuctionInputOrder(instrumentId, exchangeId, orderInfo.buyOrSell(), orderInfo.getVol(), lastestInfo->OpenPrice, requestIndex);
			int result = pUserTraderApi->ReqOrderInsert(&order, requestIndex);
		}
	}
}

void CTraderHandler::queryDepthMarketData(string instrumentId, string exchangeId) {
	CThostFtdcQryDepthMarketDataField instrumentField = { 0 };

	strcpy_s(instrumentField.InstrumentID, instrumentId.c_str());

	strcpy_s(instrumentField.ExchangeID, exchangeId.c_str());

	// 投资者结算结果确认后，查询合约深度行情
	// 查询合约深度行情回调函数：OnRspQryDepthMarketData
	int result = pUserTraderApi->ReqQryDepthMarketData(&instrumentField, requestIndex++);
	//cout << "query ret: " << result << endl;
}


///请求查询行情
/**
0，代表成功。
-1，表示网络连接失败；
-2，表示未处理请求超过许可数；
-3，表示每秒发送请求数超过许可数。
**/
int CTraderHandler::ReqQryDepthMarketData(CThostFtdcQryDepthMarketDataField* pQryDepthMarketData, int nRequestID) {
	cout << "into ReqQryDepthMarketData" << endl;
	CThostFtdcQryDepthMarketDataField instrumentField = { 0 };

	strcpy_s(instrumentField.InstrumentID, pQryDepthMarketData->InstrumentID);

	strcpy_s(instrumentField.ExchangeID, pQryDepthMarketData->ExchangeID);

	// 投资者结算结果确认后，查询合约深度行情
	// 查询合约深度行情回调函数：OnRspQryDepthMarketData
	int result = pUserTraderApi->ReqQryDepthMarketData(&instrumentField, requestIndex++);
	std::cout << " query resCode:" << result << std::endl;
	return result;
}

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
