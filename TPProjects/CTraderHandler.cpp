// CTraderHandler.h 通过 #include "ThostFtdcTraderApi.h"继承CThostFtdcTraderSpi
// 因而该类对其CThostFtdcTraderSpi方法进行实现
#include "CTraderHandler.h"
#include "FileReader.h"

CTraderHandler::CTraderHandler(CThostFtdcTraderApi* pUserTraderApi) {
	this->pUserTraderApi = pUserTraderApi;
	queryReqIndex = 0;
	orderReqIndex = 0;
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
	int b = pUserTraderApi->ReqAuthenticate(&authField, orderReqIndex++);

	LOG(INFO) << "客户端认证 = "  << b  ;
}

// 构建集合竞价报单
CThostFtdcInputOrderField CTraderHandler::composeAuctionInputOrder(string instrumentID) {
	InstrumentOrderInfo orderInfo = auctionInsOrderMap.find(instrumentID)->second;
	CThostFtdcDepthMarketDataField* lastestInfo = instrumentInfoMap.find(instrumentID)->second.getInfo();

	string exchangeID = getExchangeId(instrumentID);
	bool buyIn = orderInfo.buyOrSell();
	int vol = orderInfo.getVol();
	double price = lastestInfo->OpenPrice;
	int requestId = this->orderReqIndex;
	return composeInputOrder(instrumentID, exchangeID, buyIn, vol, price, THOST_FTDC_TC_GFD, requestId);
}

// 构建滑点报单
CThostFtdcInputOrderField CTraderHandler::composeSlipInputOrder(string instrumentID) {
	InstrumentOrderInfo orderInfo = slipperyInsOrderMap.find(instrumentID)->second;
	CThostFtdcDepthMarketDataField* lastestInfo = instrumentInfoMap.find(instrumentID)->second.getInfo();
	
	string exchangeID = getExchangeId(instrumentID);
	bool buyIn = orderInfo.buyOrSell();
	// 第二部分要求每次报单一手
	int vol = 1;
	double price = choosePrice(lastestInfo);
	int requestId = this->orderReqIndex;
	return composeInputOrder(instrumentID, exchangeID, buyIn, vol, price, THOST_FTDC_TC_GFD, requestId);
}

double CTraderHandler::choosePrice(CThostFtdcDepthMarketDataField* latestInfo) {
	double price = latestInfo->OpenPrice;
	switch (SlipperyPhase::getPhase()) {
	case(SlipperyPhase::PHASE_1): {
		// 开盘价
			break;
	}
	case(SlipperyPhase::PHASE_2): {
		// 对手价
		price = latestInfo->BidPrice1;
		break;
	}
	case(SlipperyPhase::PHASE_3): {
		// 对手价加高
		price = latestInfo->BidPrice1 + 0.0001;
		break;
	}
	}
	return price;
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
	LOG(INFO) << "Connect Success......"  ;
	// API连接成功后，调用客户端认证接口
	// 客户端认证接口回调函数：OnRspAuthenticate
	this->ReqAuthenticate();
}

void CTraderHandler::OnFrontDisconnected(int nReason)
{
	LOG(INFO) << "OnFrontDisconnected"  ;
}

void CTraderHandler::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField,
	CThostFtdcRspInfoField* pRspInfo, 
	int nRequestID, 
	bool bIsLast)
{
	LOG(INFO) << "Authenticate success......"  ;

	CThostFtdcReqUserLoginField userField = { 0 };

	strcpy_s(userField.BrokerID, getConfig("config", "BrokerID").c_str());
	strcpy_s(userField.UserID, getConfig("config", "InvestorID").c_str());
	strcpy_s(userField.Password, getConfig("config", "Password").c_str());
	
	// 客户端认证成功后，用户登录
	// 用户登录回调函数：OnRspUserLogin
	int result = pUserTraderApi->ReqUserLogin(&userField, orderReqIndex++);

	LOG(INFO) << result  ;
}

void CTraderHandler::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
	CThostFtdcRspInfoField* pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	LOG(INFO) << "Login success......"  ;

	char* tradingDay = (char*)pUserTraderApi->GetTradingDay();

	LOG(INFO) << "Trading Day: " << tradingDay  ;

	CThostFtdcSettlementInfoConfirmField confirmField = { 0 };
	strcpy_s(confirmField.BrokerID, getConfig("config", "BrokerID").c_str());
	strcpy_s(confirmField.InvestorID, getConfig("config", "InvestorID").c_str());

	// 用户登录成功后，结算结果确认，在开始每日交易前，必须要先确认，每日确认一次即可
	// 结算结果确认回调函数：OnRspSettlementInfoConfirm
	pUserTraderApi->ReqSettlementInfoConfirm(&confirmField, nRequestID++);
}


void CTraderHandler::beginQuery() {
	int operation = 0;
	LOG(INFO) << "请输入选择的操作（\n0.查询账户；\n1.查询持仓；\n2.集合竞价下单；\n3.合约查询样例；）：";
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
		result = pUserTraderApi->ReqQryTradingAccount(&tradingAccountField, orderReqIndex++);

		break;
	case 1:
		strcpy_s(investorPositionField.BrokerID, getConfig("config", "BrokerID").c_str());
		strcpy_s(investorPositionField.InvestorID, getConfig("config", "InvestorID").c_str());

		// 请求查询账户持仓
		result = pUserTraderApi->ReqQryInvestorPosition(&investorPositionField, orderReqIndex++);

		break;
	case 2:
		// 1. loadInstrumentId()，另起一个线程轮询更新得到最近报价
		// 2. use strategy to generate price
		orderReqIndex++;
		CThostFtdcInputOrderField inputOrderField = composeAuctionInputOrder("ag1912");
		result = pUserTraderApi->ReqOrderInsert(&inputOrderField, orderReqIndex);
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
		//LOG(INFO) << "start poll thread result" << ret  ;
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
	int result = pUserTraderApi->ReqQryTrade(&field, ++orderReqIndex);
}

void CTraderHandler::callAuction() {
	for (auto iter = auctionInsOrderMap.begin(); iter != auctionInsOrderMap.end(); iter++) {
		string instrumentId = iter->first;
		if (instrumentInfoMap.find(instrumentId) == instrumentInfoMap.end()) {
			// 如果instrumentId不在instrumentInfoMap，说明查询数据失败
			auctionInsStateMap.insert(pair<string, AuctionInsState*>(instrumentId,
				new AuctionInsState(AuctionInsState::STATE_ENUM::NO_INFO, -1)));
			continue;
		}
		int result = 0;
		int retry = 3;
		while (retry-->0)
		{
			LOG(INFO) << instrumentId << " 集合下单一次"  ;
			++orderReqIndex;
			// 集合竞价的price待定
			CThostFtdcInputOrderField order = composeAuctionInputOrder(instrumentId);
			result = pUserTraderApi->ReqOrderInsert(&order, orderReqIndex);
			if (0 == result) {
				LOG(INFO) << "Order ReqId: "<<orderReqIndex;
				auctionInsStateMap.insert(pair<string, AuctionInsState*>(instrumentId,
					new AuctionInsState(AuctionInsState::STATE_ENUM::STARTED, orderReqIndex)));
				break;
			}
			else {
				if (-1 == result) {
					LOG(INFO) << "集合竞价下单，网络连接失败"  ;
				}
				//-1,-2,-3 三种情况都等一秒之后重试
				this_thread::sleep_for(chrono::milliseconds(1000));
			}
		}
		if (result < 0) {
			LOG(INFO) << instrumentId << " 集合下单失败"  ;
		}
		this_thread::sleep_for(chrono::milliseconds(200));
	}
	// 记录最后一条集合竞价请求ID
	auctionLastReqId = orderReqIndex;
}

// 该方法会在新线程 startScanThread 中被调用
void CTraderHandler::callSlippery(SlipperyPhase::PHASE_ENUM phase) {
	chrono::system_clock::time_point tp;
	switch (phase){
	case(SlipperyPhase::PHASE_1): {
		tp = getSlipPhaseAStartTime();
		break;
	}
	case(SlipperyPhase::PHASE_2): {
		tp = getSlipPhaseBStartTime();
		break;
	}
	default: {
		// 该function只处理滑点下单的前两阶段
		return;
	}
	}
	this_thread::sleep_until(tp);
	if (phase == SlipperyPhase::PHASE_1) {
		LOG(INFO) << "=============Begin Second part PHASE_1=============";
	}
	else {
		LOG(INFO) << "=============Begin Second part PHASE_2=============";
	}
	// 记录当前处于何阶段
	curPhase = phase;
	for (auto iter = slipperyInsOrderMap.begin(); iter != slipperyInsOrderMap.end(); iter++) {
		string instrumentId = iter->first;
		InstrumentOrderInfo orderInfo = iter->second;
		LOG(INFO) << "IntrumentId: "<< instrumentId;
		if (instrumentInfoMap.find(instrumentId) == instrumentInfoMap.end()) {
			LOG(INFO) << "不存在于instrumentInfoMap中，不可下单";
			// 如果instrumentId不在instrumentInfoMap，说明查询数据失败
			for (size_t i = 1; i <= orderInfo.getVol(); i++)
			{
				// 对于合约信息都查询不到的单子，直接略过
				slipperyInsStateMap[instrumentId].insert(pair<int, SlipperyInsState*>
					(0-i, new SlipperyInsState(SlipperyInsState::STATE_ENUM::NO_INFO, -1)));
			}
			continue;
		}
		// 第一阶段，对所有合约下单
		if (phase == SlipperyPhase::PHASE_1) {
			// 每次下单一手
			for (size_t i = 0; i < orderInfo.getVol(); i++)
			{
				submitSlipperyOrder(instrumentId);
			}
		}
		// 第二阶段，只对未成交的部分合约处理
		if (phase == SlipperyPhase::PHASE_2) {
			auto pair = slipperyInsStateMap.find(instrumentId);
			if (pair == slipperyInsStateMap.end()) {
				LOG(INFO) << "不存在于slipperyInsStateMap中，不可下单";
				continue;
			}
			for (auto orderIter = pair->second.begin(); orderIter != pair->second.end(); orderIter++) {
				if (orderIter->second->getState() == SlipperyInsState::UNTRADED
				|| orderIter->second->getState() == SlipperyInsState::RETRIVED) {
					submitSlipperyOrder(instrumentId);
				}
				else {
					LOG(INFO) << "阶段二中，订单状态为："<< orderIter->second->getState() <<", 不下单";
				}
			}			
		}
	}
}

void CTraderHandler::submitSlipperyOrder(string instrumentId) {
	int result = 0;
	int retry = 3;
	while (retry-- > 0)
	{
		LOG(INFO) << instrumentId << "下单一次"  ;
		++orderReqIndex;
		CThostFtdcInputOrderField order = composeSlipInputOrder(instrumentId);
		result = pUserTraderApi->ReqOrderInsert(&order, orderReqIndex);
		if (0 == result) {
			LOG(INFO) << "Order ReqId: "<< orderReqIndex;
			slipperyInsStateMap[instrumentId][orderReqIndex] =
				new SlipperyInsState(SlipperyInsState::STATE_ENUM::STARTED, orderReqIndex);
			break;
		}
		else {
			if (-1 == result) {
				LOG(INFO) << "下单，网络连接失败"  ;
			}
			//-1,-2,-3 三种情况都等一秒之后重试
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
	if (result < 0) {
		LOG(INFO) << instrumentId << "下单失败"  ;
	}
	this_thread::sleep_for(chrono::milliseconds(200));
}

bool CTraderHandler::startPollThread() {
	try {
		// 这一行实在看不懂，网上查来的
		thread t(&CTraderHandler::poll,this);
		//t.join();
		t.detach();
	}
	catch (exception ex) {
		cerr << "startPollThread failed"  ;
		return false;
	}
	return true;
}

bool CTraderHandler::startScanThread() {
	try {
		// 这一行实在看不懂，网上查来的
		thread t(&CTraderHandler::scanSlipperyOrderState, this);
		//t.join();
		t.detach();
	}
	catch (exception ex) {
		cerr << "startScanThread failed"  ;
		return false;
	}
	return true;
}

bool CTraderHandler::startSlipPhaseAThread() {
	try {
		// 这一行实在看不懂，网上查来的
		thread t(&CTraderHandler::callSlippery, this, SlipperyPhase::PHASE_1);
		//t.join();
		t.detach();
	}
	catch (exception ex) {
		cerr << "startScanThread failed"  ;
		return false;
	}
	return true;
}

bool CTraderHandler::startSlipPhaseBThread() {
	try {
		// 这一行实在看不懂，网上查来的
		thread t(&CTraderHandler::callSlippery, this, SlipperyPhase::PHASE_2);
		//t.join();
		t.detach();
	}
	catch (exception ex) {
		cerr << "startScanThread failed"  ;
		return false;
	}
	return true;
}

bool CTraderHandler::startSlipPhaseCThread() {
	try {
		// 这一行实在看不懂，网上查来的
		thread t(&CTraderHandler::slipPhaseCEntrance, this);
		//t.join();
		t.detach();
	}
	catch (exception ex) {
		cerr << "startScanThread failed"  ;
		return false;
	}
	return true;
}
void CTraderHandler::slipPhaseCEntrance() {
	this_thread::sleep_until(getSlipPhaseCStartTime());
	LOG(INFO) << "=============Begin Second part PHASE_3=============";
	curPhase = SlipperyPhase::PHASE_3;
	slipPhaseCProcess();
}
void CTraderHandler::slipPhaseCProcess() {
	bool orderSubmit = false;
	for (auto stateMap = slipperyInsStateMap.begin(); stateMap != slipperyInsStateMap.end(); stateMap++) {
		for (auto orderItem = stateMap->second.begin(); orderItem != stateMap->second.end(); orderItem++) {
			// 暂未match的报单, 撤回
			if (orderItem->second->getState() == SlipperyInsState::ORDERED) {
				cancelInstrument(orderItem->first);
				orderSubmit = true;
				break;
			}//已撤回和确定未成单的报单，再下单。此类型报单需要对map进行修改，需要加锁
			else if (orderItem->second->getState() == SlipperyInsState::RETRIVED
				|| orderItem->second->getState() == SlipperyInsState::UNTRADED
				|| orderItem->second->getState() == SlipperyInsState::ORDER_FAILED) {
				mtx.lock();
				LOG(INFO) << "slipPhaseCProcess LOCKED for " << stateMap->first;
				slipperyInsStateMap[stateMap->first].erase(orderItem->first);
				submitSlipperyOrder(stateMap->first);
				mtx.unlock();
				LOG(INFO) << "slipPhaseCProcess UNLOCKED for " << stateMap->first;
				orderSubmit = true;
				break;
			}
		}
		if (orderSubmit) {
			break;
		}
	}
	// 如果没有合约需要改变，该线程关闭
	if (!orderSubmit) {
		terminate();
	}
}

void CTraderHandler::poll() {
	string instrument = allInstruments.at(insQueryId%allInstruments.size());
	auto iter = instrumentsExchange.find(instrument);
	LOG(DEBUG) << "back query begin in thread: "<<this_thread::get_id();
	queryDepthMarketData(instrument, iter->second);
}

vector<string> CTraderHandler::loadInstruments() {
	vector<string> content;
	// load doc1.log into auctionInsOrderMap. doc1存储集合竞价的合约单
	bool readSucc = loadFile2Vector("doc1.log", content);
	if (!readSucc) {
		LOG(INFO) << "load doc1.log FAILED"  ;
	}
	else {
		LOG(INFO) << "load doc1.log succeed!"  ;
		// load数据进入内存
		for (size_t i = 0; i < content.size(); i++)
		{
			vector<string> arr = split(content.at(i), ",");
			allInstruments.push_back(arr.at(1));
			instrumentsExchange[arr.at(1)] = arr.at(2);
			auctionInsOrderMap.insert(pair<string, InstrumentOrderInfo>(arr.at(1), InstrumentOrderInfo(arr.at(3))));
		}
	}

	content.clear();
	// load doc2.log into slipperyInsOrderMap. doc2存储集合竞价的合约单
	readSucc = loadFile2Vector("doc2.log", content);
	if (!readSucc) {
		LOG(INFO) << "load doc2.log FAILED"  ;
	}
	else {
		LOG(INFO) << "load doc2.log succeed!"  ;
		// load数据进入内存
		for (size_t i = 0; i < content.size(); i++)
		{
			vector<string> arr = split(content.at(i), ",");
			allInstruments.push_back(arr.at(1));
			instrumentsExchange[arr.at(1)] = arr.at(2);
			slipperyInsOrderMap.insert(pair<string, InstrumentOrderInfo>(arr.at(1), InstrumentOrderInfo(arr.at(3))));
			slipperyInsStateMap.insert(pair<string, unordered_map<int, SlipperyInsState*>>(arr.at(1), unordered_map<int, SlipperyInsState*>()));
		}
	}
	return content;
}

// 请求查询行情响应。当客户端发出请求查询行情指令后，交易托管系统返回响应时，该方法会被调用。
void CTraderHandler::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	LOG(DEBUG) << "OnRspQryDepthMarketData is called in " << this_thread::get_id();
	this->pDepthMarketData = pDepthMarketData;
	if (pRspInfo != nullptr) {
		if (pRspInfo != nullptr) {
			LOG(DEBUG) << "错误ID:" << pRspInfo->ErrorID  ;
		}

		if (pRspInfo != nullptr) {
			LOG(DEBUG) << "错误消息:" << pRspInfo->ErrorMsg  ;
		}
		return;
	}
	LOG(DEBUG) << "OnRspQryDepthMarketData :" << nRequestID  ;
	if (pDepthMarketData != nullptr) {
		LOG(DEBUG) << pDepthMarketData->InstrumentID << "获取行情数据"  ;
		auto it = instrumentInfoMap.find(pDepthMarketData->InstrumentID);
		if (it != instrumentInfoMap.end()) {
			// find one in map
			InstrumentInfo preInfo = it->second;
			if (!preInfo.isLatestInfo(nRequestID)) {
				preInfo.updateInfo(nRequestID, pDepthMarketData);
				//LOG(INFO) << "Update the info of " << pDepthMarketData->InstrumentID  ;
			}
		}
		else {
			// not find
			InstrumentInfo info(nRequestID, pDepthMarketData);
			instrumentInfoMap.insert(pair<string, InstrumentInfo>(pDepthMarketData->InstrumentID, info));
			//LOG(INFO) << "Add the info of " << pDepthMarketData->InstrumentID  ;
		}
	}
	else {
		LOG(DEBUG) <<"GET Nothing as resp when nRequest: "<< nRequestID  ;
	}

	LOG(DEBUG) << "================================================================"  ;
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
				LOG(INFO) << "首轮询价完毕\n===========后台轮询查询开始==========="  ;
				startPollThread();
				LOG(INFO) << "===========集合竞价开始==========="  ;
				callAuction();
				startSlipPhaseAThread();
				startSlipPhaseBThread();
				startSlipPhaseCThread();
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
	//beginQuery();
}

void CTraderHandler::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	LOG(INFO) << "查询交易账户响应......"  ;
	if (pRspInfo != nullptr) {
		if (pRspInfo != nullptr) {
			LOG(INFO) << "错误ID:" << pRspInfo->ErrorID  ;
		}

		if (pRspInfo != nullptr) {
			LOG(INFO) << "错误消息:" << pRspInfo->ErrorMsg  ;
		}
	}
	LOG(INFO) << "请求序号:" << nRequestID  ;
	LOG(INFO) << "IsLast:" << bIsLast  ;

	LOG(INFO) << "================================================================"  ;
	LOG(INFO) << "经纪公司代码：" << pTradingAccount->BrokerID  ;
	LOG(INFO) << "投资者账号：" << pTradingAccount->AccountID  ;
	LOG(INFO) << "可用资金：" << pTradingAccount->Available  ;
	LOG(INFO) << "入金金额：" << pTradingAccount->Deposit  ;
	LOG(INFO) << "出金金额：" << pTradingAccount->Withdraw  ;
	LOG(INFO) << "================================================================"  ;

	//beginQuery();
}

// 报单回报。当客户端进行报单录入、报单操作及其它原因（如部分成交）导致报单状态发生变化时，交易托管系统会主动通知客户端，该方法会被调用
// insertOrder, order traded, order canceled 均可能回调该函数
void CTraderHandler::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
	LOG(INFO) << "================================================================"  ;
	LOG(INFO) << "OnRtnOrder is called"  ;

	string insId = pOrder->InstrumentID;
	int reqId = pOrder->RequestID;
	LOG(INFO) << "instrumentid is " << insId  ;
	LOG(INFO) << "reqId is " << reqId;
	LOG(INFO) << "OrderSubmitStatus is " << pOrder->OrderSubmitStatus  ;
	LOG(INFO) << "OrderStatus is " << pOrder->OrderStatus  ;

	LOG(INFO) << "OrderRef: " << pOrder->OrderRef;
	LOG(INFO) << "OrderLocalID: " << pOrder->OrderLocalID;
	LOG(INFO) << "OrderSysID: " << pOrder->OrderSysID;
	LOG(INFO) << "OrderSource: " << pOrder->OrderSource;
	LOG(INFO) << "SequenceNo: " << pOrder->SequenceNo;
	LOG(INFO) << "FrontID: " << pOrder->FrontID;
	

	// 报单成功
	// 验证发现报单后第一次回调：pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted, pOrder->OrderStatus == THOST_FTDC_OST_Unknown
	// 判断方法待明确
	if (pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted) {
		LOG(INFO) << "我们认为=报单成功";
		if (pOrder->RequestID <= auctionLastReqId) {// 集合竞价订单回调
			// auctionInsStateMap中存在
			if (auctionInsStateMap.find(insId) != auctionInsStateMap.end()) {
				auto insState = auctionInsStateMap.find(insId)->second;
				if (insState->getLatestReqId() == pOrder->RequestID // 回调对应的reqId == 最新状态的reqId
					&& insState->getState() == AuctionInsState::STATE_ENUM::STARTED)  // 要求最近一次状态是UNSTARTED
				{ // 更新auctionInsStateMap中的state 和 respId
					insState->updateOnResp(AuctionInsState::STATE_ENUM::ORDERED, pOrder->RequestID);
				}
			}
			else {
				// NOT POSSIBLE
				auctionInsStateMap.insert(pair<string, AuctionInsState*>(insId,
					new AuctionInsState(AuctionInsState::STATE_ENUM::ORDERED, pOrder->RequestID)));
			}
		}// 滑点订单回调
		else{
			slipperyRtnOrderMap[reqId] = pOrder;
			//slipperyInsStateMap中存在
			if (slipperyInsStateMap[insId].find(pOrder->RequestID) != slipperyInsStateMap[insId].end()) {
				auto insState = slipperyInsStateMap[insId].find(pOrder->RequestID)->second;
				if (insState->getLatestReqId() == pOrder->RequestID // 回调对应的reqId == 最新状态的reqId
					&& (insState->getState() == SlipperyInsState::STATE_ENUM::STARTED
						|| insState->getState() == SlipperyInsState::STATE_ENUM::RETRIVED))  // 要求最近一次状态是UNSTARTED或者RETRIVED
				{ // 更新slipperyInsStateMap中的state 和 respId
					insState->updateOnResp(SlipperyInsState::STATE_ENUM::ORDERED, pOrder->RequestID);
				}
			}
			else {
				// NOT POSSIBLE
				LOG(INFO) << "NOT IMPOSSIBLE HAPPENS!!!";
				slipperyInsStateMap[insId].insert(pair<int, SlipperyInsState*>(pOrder->RequestID,
					new SlipperyInsState(SlipperyInsState::STATE_ENUM::ORDERED, pOrder->RequestID)));
			}
		}
	}

	// 撤单成功
	// 报单自动撤销后回调发现：pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertRejected, pOrder->OrderStatus == THOST_FTDC_OST_Canceled
	// 判断方法待明确
	else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled) {
		LOG(INFO) << "我们认为=撤单成功";
		if (pOrder->RequestID <= auctionLastReqId) { // 判断该回调对应的req是集合竞价下单
			if (auctionInsStateMap.find(pOrder->InstrumentID) != auctionInsStateMap.end()) {
				auto insState = auctionInsStateMap.find(pOrder->InstrumentID)->second;
				insState->updateOnResp(AuctionInsState::STATE_ENUM::CANCELED , pOrder->RequestID);
			}
		}// 滑点订单回调
		else {
			actionIfSlipperyCanceled(insId, pOrder->RequestID);
			//  如果处于第二部分报单的第三阶段，call slipPhaseCProcess，继续检查slipperyInsStateMap其它合约
			if (curPhase == SlipperyPhase::PHASE_3) {
				slipPhaseCProcess();
			}
		}
	}
	// 合约单成交
	// 看代码解释，未验证
	else if(pOrder->OrderStatus == THOST_FTDC_OST_AllTraded){
		LOG(INFO) << "我们认为=合约成交";
		if (pOrder->RequestID <= auctionLastReqId) { // 集合竞价下单回调
			if (auctionInsStateMap.find(pOrder->InstrumentID) != auctionInsStateMap.end()) {
				auto insState = auctionInsStateMap.find(pOrder->InstrumentID)->second;
				insState->updateOnResp(AuctionInsState::STATE_ENUM::DONE, pOrder->RequestID);
			}
			else {
				// NOT POSSIBLE
				auctionInsStateMap.insert(pair<string, AuctionInsState*>(insId,
					new AuctionInsState(AuctionInsState::STATE_ENUM::DONE, pOrder->RequestID)));
			}
		}
		else{// 滑点订单回调
			actionIfSlipperyTraded(insId, pOrder->RequestID);
			//  如果处于第二部分报单的第三阶段，call slipPhaseCProcess，继续检查slipperyInsStateMap其它合约
			if (curPhase == SlipperyPhase::PHASE_3) {
				slipPhaseCProcess();
			}
		}
	}
	else {
		LOG(INFO) << "我们未处理该状态";
	}
	return;
}
// 成交回报。当发生成交时交易托管系统会通知客户端，该方法会被调用
// 个人认为，该方法被调用到时，OnRtnOrder也会被调用到，所以该方法中不做实现
void CTraderHandler::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
	LOG(INFO) << "================================================================"  ;
	LOG(INFO) << "OnRtnTrade is called"  ;
	LOG(INFO) << "================================================================"  ;
}

// 报单录入应答。 当客户端发出过报单录入指令后， 交易托管系统返回响应时，该方法会被调用
void CTraderHandler::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	LOG(INFO) << "================================================================"  ;	
	LOG(INFO) << "OnRspOrderInsert is called"  ;
	LOG(INFO) << "nRequestID: " << nRequestID;
	LOG(INFO) << "nRequestID: " << nRequestID;
	if (pRspInfo != nullptr) {
		LOG(INFO) << "OnRspOrderInsert says it's failed"  ;
		LOG(INFO) << "错误代码" << pRspInfo->ErrorID  ;
		LOG(INFO) << "错误信息" << pRspInfo->ErrorMsg  ;
	}
	else if(pInputOrder != nullptr){
		// 但是结果中未有
		LOG(INFO) << "pOrder.ReqId " << pInputOrder->RequestID << " order success";
		LOG(INFO) << "OnRspOrderInsert says insert " << pInputOrder->InstrumentID << " order success"  ;

	}
	
	LOG(INFO) << "================================================================"  ;
}

void CTraderHandler::scanSlipperyOrderState() {
	bool needScan = false;
	string insId;
	int reqId;
	mtx.lock();
	LOG(INFO) << "scanSlipperyOrderState locked";
	for (auto it = slipperyInsStateMap.begin(); it != slipperyInsStateMap.end(); it++) {
		for (auto itemIt = it->second.begin(); itemIt != it->second.end(); itemIt++) {
			if (itemIt->second->getState() != SlipperyInsState::STARTED
				&& itemIt->second->getState() != SlipperyInsState::ORDERED
				&& itemIt->second->getState() != SlipperyInsState::RETRIVED) {//仅三状态需要scan
				continue;
			}
			else { //只需要找到一个需要scan的合约就进行Query，等待回调函数中再call该函数
				needScan = true;
				insId = it->first;
				reqId = itemIt->first;
				break;
			}
		}
		if (needScan) {
			break;
		}
	}
	mtx.unlock();
	LOG(INFO) << "scanSlipperyOrderState unlocked";
	// 如果needScan == false, 意味着slipperyInsStateMap中所有的合约处于NO_INFO, ORDER_FAILED, DONE状态
	// 已完成第二部分报单任务, 该线程可结束
	if (!needScan) {
		terminate();
	}// 如果needScan == true，意味着仍有合约需要scan
	else
	{
		CThostFtdcOrderField* order = slipperyRtnOrderMap[reqId];
		CThostFtdcQryOrderField field = { 0 };
		// 文档注释里说，“不写 BrokerID 可以收全所有报单。” 不懂什么意思
		strcpy_s(field.BrokerID, getConfig("config", "BrokerID").c_str());
		strcpy_s(field.InvestorID, getConfig("config", "InvestorID").c_str());
		strcpy_s(field.InstrumentID, insId.c_str());
		strcpy_s(field.OrderSysID, order->OrderSysID);
		int result = 0;
		int retry = 3;
		while (--retry > 0) {
			result = pUserTraderApi->ReqQryOrder(&field, ++orderReqIndex);
			if (0 == result) {
				break;
			}
			else {
				if (result == -1) {
					LOG(INFO) << "ReqQryOrder encountered 网络连接失败"  ;
				}
				this_thread::sleep_for(chrono::milliseconds(1000));
			}
		}
		// 此处经常result=-2，表示未处理请求超过许可数
		if (result < 0) {
			LOG(INFO) << insId << " 订单状态查询失败, result = " << result  ;
		}
		// 暂停0.2s 用来控QPS
		this_thread::sleep_for(chrono::milliseconds(200));
	}
}

void CTraderHandler::actionIfSlipperyTraded(string instrumentId, int reqId) {
	switch (curPhase)
	{
	case(SlipperyPhase::PHASE_1): {
		LOG(INFO) << "第二部分报单阶段一成交";
		break;
	}
	case(SlipperyPhase::PHASE_2): {
		LOG(INFO) << "第二部分报单阶段二成交";
		break;
	}
	case(SlipperyPhase::PHASE_3): {
		LOG(INFO) << "第二部分报单阶段三成交";
		break;
	}
	default:
		break;
	}
	LOG(INFO) << instrumentId <<" 已成交";
	if (slipperyInsStateMap[instrumentId].find(reqId) != slipperyInsStateMap[instrumentId].end()) {
		auto insState = slipperyInsStateMap.find(instrumentId)->second;
		// Traded是某一合约的终态，不需要check之前的状态
		slipperyInsStateMap[instrumentId][reqId]->updateOnResp(SlipperyInsState::STATE_ENUM::DONE, reqId);
	}
	else {
		// NOT POSSIBLE
		LOG(ERROR) << "NOT POSSIBLE HAPPENS!!!";
		slipperyInsStateMap[instrumentId][reqId] = new SlipperyInsState(SlipperyInsState::STATE_ENUM::DONE, reqId);
	}
}

void CTraderHandler::actionIfSlipperyCanceled(string instrumentId, int reqId) {
	if (slipperyInsStateMap[instrumentId].find(reqId) != slipperyInsStateMap[instrumentId].end()) {
		auto insState = slipperyInsStateMap[instrumentId][reqId];
		if (insState->getLatestReqId() == reqId // 回调对应的reqId == 最新状态的reqId
			&& (insState->getState() == SlipperyInsState::STATE_ENUM::STARTED
				|| insState->getState() == SlipperyInsState::STATE_ENUM::ORDERED))  // 要求最近一次状态是UNSTARTED或者ORDERED
		{ // 更新slipperyInsStateMap中的state 和 respId
			insState->updateOnResp(SlipperyInsState::STATE_ENUM::RETRIVED, reqId);
		}
	}
}

void CTraderHandler::cancelInstrument(int reqId) {
	LOG(INFO) << "撤回合约ReqId " << reqId;
	if (slipperyRtnOrderMap.find(reqId) == slipperyRtnOrderMap.end()) {
		LOG(ERROR) << "ReqId " << reqId << " 在slipperyRtnOrderMap中缺失";
		return;
	}
	auto pOrder = slipperyRtnOrderMap[reqId];
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
		result = pUserTraderApi->ReqOrderAction(&field, ++orderReqIndex);
		if (0 == result) {
			// 由于仅在订单处于ORDERED状态下，才需要发起撤单操作
			// 认为撤单中也是处于ORDERED状态下，所以成功发起撤单后不需要更改状态
			break;
		}
		if (result < 0) {
			if (-1 == result) {
				LOG(INFO) << "call ReqOrderAction 网络连接失败"  ;
			}
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
	// 重试三次，撤单指令发送失败
	if (result < 0) {
		LOG(INFO) << "重试三次，撤单 " << reqId << " 指令发送失败"  ;
	}
}

// 仅针对滑点订单，会发起报单查询
// 报单查询请求。当客户端发出报单查询指令后，交易托管系统返回响应时，该方法会被调用
void CTraderHandler::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	LOG(INFO) << "OnRspQryOrder is called"  ;
	if (pRspInfo != nullptr) {
		LOG(INFO) << "错误代码：" << pRspInfo->ErrorID  ;
		LOG(INFO) << "错误信息：" << pRspInfo->ErrorMsg  ;
	}
	if (pOrder != nullptr) {
		string insId = pOrder->InstrumentID;
		// 已成交
		if (THOST_FTDC_OST_AllTraded == pOrder->OrderStatus) {
			actionIfSlipperyTraded(insId, pOrder->RequestID);
		}// 已撤单
		else if (THOST_FTDC_OST_Canceled == pOrder->OrderStatus) {
			actionIfSlipperyCanceled(insId, pOrder->RequestID);
		}
		// 否则记录订单信息，用于撤单
		else {
			slipperyRtnOrderMap[nRequestID] = pOrder;
		}
	}
	//再次scan, 直到所有报单状态到达终态
	scanSlipperyOrderState();
}

/*
Thost 收到撤单指令，如果没有通过参数校验，拒绝接受撤单指令。用户就会收到
OnRspOrderAction 消息，其中包含了错误编码和错误消息
*/
void CTraderHandler::OnRspOrderAction(CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	LOG(INFO) << "OnRspOrderAction is called"  ;
	if (pRspInfo != nullptr) {
		LOG(INFO) << "错误代码：" << pRspInfo->ErrorID  ;
		LOG(INFO) << "错误信息：" << pRspInfo->ErrorMsg  ;
	}
	if (pOrderAction != nullptr) {
		string insId = pOrderAction->InstrumentID;
		// 理论上不会被调到，暂不写处理逻辑
		LOG(INFO) << insId  ;
	}
}

void CTraderHandler::OnErrRtnOrderAction(
	CThostFtdcOrderActionField* pOrderAction,
	CThostFtdcRspInfoField* pRspInfo) {
	LOG(INFO) << "OnErrRtnOrderAction is called"  ;
	if (pRspInfo != nullptr) {
		LOG(INFO) << "错误代码：" << pRspInfo->ErrorID  ;
		LOG(INFO) << "错误信息：" << pRspInfo->ErrorMsg  ;
	}
	if (pOrderAction != nullptr) {
		string insId = pOrderAction->InstrumentID;
		// 理论上不会被调到，暂不写处理逻辑
		LOG(INFO) << insId  ;
	}
}

void CTraderHandler::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo)
{
	LOG(INFO) << "OnErrRtnOrderInsert is called"  ;
	LOG(INFO) << "================================================================"  ;
	if (pRspInfo != nullptr) {
		LOG(INFO) << "错误代码" << pRspInfo->ErrorID  ;
		LOG(INFO) << "错误信息" << pRspInfo->ErrorMsg  ;
	}

	if (pInputOrder != nullptr) {
		string insId = pInputOrder->InstrumentID;
		int reqId = pInputOrder->RequestID;
		if (reqId <= auctionLastReqId) { //集合竞价下单失败
			if (auctionInsStateMap.find(insId) != auctionInsStateMap.end()) {
				auto state = auctionInsStateMap.find(insId)->second;
				state->updateOnResp(AuctionInsState::ORDER_FAILED, reqId);
			}
			else {
				// NOT POSSIBLE
				auctionInsStateMap[insId] = new AuctionInsState(AuctionInsState::ORDER_FAILED, reqId);
			}
		}
		else {
			if (slipperyInsStateMap[insId].find(reqId) != slipperyInsStateMap[insId].end()) {
				// ORDER_FAILED是终态，重新下单reqId会更新
				slipperyInsStateMap[insId][reqId]->updateOnResp(SlipperyInsState::ORDER_FAILED, reqId);
			}
			else {
				// NOT POSSIBLE
				slipperyInsStateMap[insId][reqId] = new SlipperyInsState(SlipperyInsState::ORDER_FAILED, reqId);
			}
			//  如果处于第二部分报单的第三阶段，call slipPhaseCProcess，继续检查slipperyInsStateMap其它合约
			if (curPhase == SlipperyPhase::PHASE_3) {
				slipPhaseCProcess();
			}
		}
	}
	LOG(INFO) << "================================================================"  ;
}

void CTraderHandler::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	LOG(INFO) << "=============加载文件 doc1.log & doc2.log ============="  ;
	loadInstruments();
	LOG(INFO) << "=============开始查询合约信息============="  ;

	// 从文件的第一条合约单开始
	string instrument = allInstruments.at(0);
	queryDepthMarketData(instrument,getExchangeId(instrument));

	//callSlippage();
	//beginQuery();
}


void CTraderHandler::queryDepthMarketData(string instrumentId, string exchangeId) {
	LOG(DEBUG) << "queryDepthMarketData instrumentID: " << instrumentId  ;
	thread::id id = this_thread::get_id();
	CThostFtdcQryDepthMarketDataField instrumentField = { 0 };

	strcpy_s(instrumentField.InstrumentID, instrumentId.c_str());

	strcpy_s(instrumentField.ExchangeID, exchangeId.c_str());

	// 投资者结算结果确认后，查询合约深度行情
	// 查询合约深度行情回调函数：OnRspQryDepthMarketData
	int result = pUserTraderApi->ReqQryDepthMarketData(&instrumentField, ++queryReqIndex);
	if (0 == result) {
		// 发送查询成功
		onceQueryMarker.insert(pair<int, string>(queryReqIndex, instrumentId));
		return;
	}
	else {
		// 发送查询失败
		if (-1 == result) {
			//-1，表示网络连接失败
			LOG(DEBUG) << "网络连接失败，导致合约信息查询失败"  ;
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
	LOG(INFO) << " query resCode:" << result  ;
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
	LOG(INFO) << "查询持仓成功"  ;

	if (pRspInfo != nullptr) {
		LOG(INFO) << "错误代码：" << pRspInfo->ErrorID  ;
		LOG(INFO) << "错误信息：" << pRspInfo->ErrorMsg  ;
	}
	LOG(INFO) << "================================================================"  ;
	if (pInvestorPosition != nullptr) {
	}
	LOG(INFO) << "================================================================"  ;
}
