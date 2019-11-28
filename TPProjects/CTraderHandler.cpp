// CTraderHandler.h 通过 #include "ThostFtdcTraderApi.h"继承CThostFtdcTraderSpi
// 因而该类对其CThostFtdcTraderSpi方法进行实现
#include "CTraderHandler.h"
#include "FileReader.h"

CTraderHandler::CTraderHandler(CThostFtdcTraderApi* pUserTraderApi) {
	this->pUserTraderApi = pUserTraderApi;
	queryReqIndex = 0;
	orderReqIndex = 0;

	errno_t err;
	if ((err = fopen_s(&debugLogFile, "logs/Debug.log", "a")) != 0) {
		cout<< "CANNOT create Debug.log" <<endl;
	}

	if ((err = fopen_s(&infoLogFile, "logs/Info.log", "a")) != 0) {
		cout  << "CANNOT create Info.log" << endl;
	}

	if ((err = fopen_s(&errorLogFile, "logs/Error.log", "a")) != 0) {
		cout << "CANNOT create Error.log" << endl;
	}
}

CTraderHandler::~CTraderHandler() {
	pUserTraderApi->Release();
	if (!debugLogFile) {
		fclose(debugLogFile);
	}
	if (!infoLogFile) {
		fclose(infoLogFile);
	}
	if (!errorLogFile) {
		fclose(errorLogFile);
	}
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
	ostringstream os;
	os<< "客户端认证 = " << b;
	LOG_INFO(os) ;
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
	ostringstream os;
	os << "当前报价\n合约：" << instrumentID << "\nAskPrice: " << lastestInfo->AskPrice1 << "\nBidPrice: " << lastestInfo->BidPrice1 << "\n";
	os << "下单信息\n下单合约：" << instrumentID << "\n下单价格：" << price << "\n下单数：" << vol << "\n买卖方向：" << (buyIn ? "买" : "卖") << "\n";
	LOG_INFO(os);
	return composeInputOrder(instrumentID, exchangeID, buyIn, vol, price, THOST_FTDC_TC_GFA, requestId);
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
	ostringstream os;
	os << "当前报价\n合约：" << instrumentID << "\nAskPrice: " << lastestInfo->AskPrice1 << "\nBidPrice: " << lastestInfo->BidPrice1 << "\n";
	os << "下单信息\n下单合约：" << instrumentID << "\n下单价格：" << price << "\n下单数：" << vol << "\n买卖方向：" << (buyIn ? "买" : "卖") << "\n";
	LOG_INFO(os);
	return composeInputOrder(instrumentID, exchangeID, buyIn, vol, price, THOST_FTDC_TC_IOC, requestId);
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
		price = latestInfo->BidPrice1 + atof(getConfig("config", "premium").c_str());
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
	ostringstream os;
	os << "Connect Success......";
	LOG_INFO(os) ;
	// API连接成功后，调用客户端认证接口
	// 客户端认证接口回调函数：OnRspAuthenticate
	this->ReqAuthenticate();
}

void CTraderHandler::OnFrontDisconnected(int nReason)
{
	ostringstream os;
	os << "OnFrontDisconnected" ;
	LOG_INFO(os);
}

void CTraderHandler::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField,
	CThostFtdcRspInfoField* pRspInfo, 
	int nRequestID, 
	bool bIsLast)
{
	ostringstream os;
	os << "Authenticate success......";

	CThostFtdcReqUserLoginField userField = { 0 };

	strcpy_s(userField.BrokerID, getConfig("config", "BrokerID").c_str());
	strcpy_s(userField.UserID, getConfig("config", "InvestorID").c_str());
	strcpy_s(userField.Password, getConfig("config", "Password").c_str());
	
	// 客户端认证成功后，用户登录
	// 用户登录回调函数：OnRspUserLogin
	int result = pUserTraderApi->ReqUserLogin(&userField, orderReqIndex++);
	os << result;
	LOG_INFO(os)  ;
}

void CTraderHandler::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
	CThostFtdcRspInfoField* pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	ostringstream os;
	os<< "Login success......" ;

	char* tradingDay = (char*)pUserTraderApi->GetTradingDay();
	os << "Trading Day: " << tradingDay;
	LOG_INFO(os) ;

	CThostFtdcSettlementInfoConfirmField confirmField = { 0 };
	strcpy_s(confirmField.BrokerID, getConfig("config", "BrokerID").c_str());
	strcpy_s(confirmField.InvestorID, getConfig("config", "InvestorID").c_str());

	// 用户登录成功后，结算结果确认，在开始每日交易前，必须要先确认，每日确认一次即可
	// 结算结果确认回调函数：OnRspSettlementInfoConfirm
	pUserTraderApi->ReqSettlementInfoConfirm(&confirmField, nRequestID++);
}


void CTraderHandler::beginQuery() {
	int operation = 0;
	ostringstream os;
	os << "请输入选择的操作（\n0.查询账户；\n1.查询持仓；\n2.集合竞价下单；\n3.合约查询样例；)：";
	LOG_INFO(os);
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
		break;
	case 3:
		queryDepthMarketData("ag1912", "SHFE");
		//bool ret = startQueryThread();
		////LOG_INFO( "start backgroundQuery thread result" + ret  ;
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
	ostringstream os;
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
			os << instrumentId << " 集合下单一次";
			LOG_INFO(os);
			++orderReqIndex;
			// 集合竞价的price待定
			CThostFtdcInputOrderField order = composeAuctionInputOrder(instrumentId);
			result = pUserTraderApi->ReqOrderInsert(&order, orderReqIndex);
			if (0 == result) {
				clearStream(os);
				os<<"Order ReqId: "<<orderReqIndex;
				LOG_INFO(os);
				auctionInsStateMap.insert(pair<string, AuctionInsState*>(instrumentId,
					new AuctionInsState(AuctionInsState::STATE_ENUM::STARTED, orderReqIndex)));
				break;
			}
			else {
				if (-1 == result) {
					clearStream(os);
					os << "集合竞价下单，网络连接失败";
					LOG_ERROR( os ) ;
				}
				//-1,-2,-3 三种情况都等一秒之后重试
				this_thread::sleep_for(chrono::milliseconds(1000));
			}
		}
		if (result < 0) {
			clearStream(os);
			os << instrumentId << " 集合下单失败";
			LOG_ERROR( os ) ;
		}
		this_thread::sleep_for(chrono::milliseconds(200));
	}
	// 记录最后一条集合竞价请求ID
	auctionLastReqId = orderReqIndex;
}

// 该方法会在新线程 startScanThread 中被调用
void CTraderHandler::callSlippery(SlipperyPhase::PHASE_ENUM phase) {
	ostringstream os;
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
		os<< "=============Begin Second part PHASE_1=============";
	}
	else {
		 os<<"=============Begin Second part PHASE_2=============";
	}
	LOG_INFO(os);
	// 记录当前处于何阶段
	curPhase = phase;
	for (auto iter = slipperyInsOrderMap.begin(); iter != slipperyInsOrderMap.end(); iter++) {
		string instrumentId = iter->first;
		InstrumentOrderInfo orderInfo = iter->second;
		//LOG_INFO( "IntrumentId: "+ instrumentId);
		if (instrumentInfoMap.find(instrumentId) == instrumentInfoMap.end()) {
			//LOG_INFO( "不存在于instrumentInfoMap中，不可下单");
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
				phaseILastReqId = orderReqIndex;
			}
		}
		// 第二阶段，只对未成交的部分合约处理
		if (phase == SlipperyPhase::PHASE_2) {
			auto pair = slipperyInsStateMap.find(instrumentId);
			if (pair == slipperyInsStateMap.end()) {
				clearStream(os);
				os << "不存在于slipperyInsStateMap中，不可下单";
				LOG_INFO(os);
				continue;
			}
			vector<int> delList;
			for (auto orderIter = pair->second.begin(); orderIter != pair->second.end(); orderIter++) {
				if (orderIter->first > phaseIILastReqId) {
					continue;
				}
				if (orderIter->second->getState() == SlipperyInsState::UNTRADED
				|| orderIter->second->getState() == SlipperyInsState::RETRIVED) {
					//下单前需要将旧的reqId从slipperyInsStateMap中删除
					delList.push_back(orderIter->first);
				}
				else {
					clearStream(os);
					os << "阶段二中，订单状态为：" << orderIter->second->getState() << ", 不下单";
					LOG_INFO(os);
				}
			}
			for (auto delIter = delList.begin(); delIter != delList.end(); ++delIter) {
				pair->second.erase(*delIter);
				submitSlipperyOrder(instrumentId);
				phaseIILastReqId = orderReqIndex;
			}
		}
	}
}

void CTraderHandler::submitSlipperyOrder(string instrumentId) {
	ostringstream os;
	int result = 0;
	int retry = 3;
	os << "thread-" << this_thread::get_id() << " calls waitForProcess";
	LOG_DEBUG(os);
	waitForProcess();
	while (retry-- > 0)
	{
		++orderReqIndex;
		clearStream(os);
		os << instrumentId << "下单一次 in Thread-" << this_thread::get_id();
		LOG_INFO( os);
		CThostFtdcInputOrderField order = composeSlipInputOrder(instrumentId);
		result = pUserTraderApi->ReqOrderInsert(&order, orderReqIndex);
		if (0 == result) {
			clearStream(os);
			os << "set unRepliedReq " << orderReqIndex;
			LOG_DEBUG( os);
			unRepliedReq = orderReqIndex;
			slipperyInsStateMap[instrumentId][orderReqIndex] =
				new SlipperyInsState(SlipperyInsState::STATE_ENUM::STARTED, orderReqIndex);
			break;
		}
		else {
			if (-1 == result) {
				clearStream(os);
				os << "下单，网络连接失败";
				LOG_ERROR(os)  ;
			}
			//-1,-2,-3 三种情况都等一秒之后重试
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
	if (result < 0) {
		unRepliedReq = -1;
		clearStream(os);
		os << instrumentId << "下单失败";
		LOG_INFO( os ) ;
	}
	this_thread::sleep_for(chrono::milliseconds(200));
}

bool CTraderHandler::startQueryThread() {
	try {
		// 这一行实在看不懂，网上查来的
		thread t(&CTraderHandler::backgroundQuery,this);
		//t.join();
		t.detach();
	}
	catch (exception ex) {
		//LOG_ERROR( "startPollThread failed" ) ;
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
		//LOG_ERROR( "startScanThread failed")  ;
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
		//LOG_ERROR( "startSlipPhaseAThread failed" ) ;
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
		//LOG_ERROR( "startSlipPhaseBThread failed" ) ;
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
		//LOG_ERROR( "startSlipPhaseCThread failed" ) ;
		return false;
	}
	return true;
}
void CTraderHandler::slipPhaseCEntrance() {
	this_thread::sleep_until(getSlipPhaseCStartTime());
	ostringstream os;
	os << "=============Begin Second part PHASE_3=============";
	LOG_INFO(os );
	curPhase = SlipperyPhase::PHASE_3;
	slipPhaseCProcess();
	startScanThread();
}

void CTraderHandler::printSlipperyInsStateMap() {
	ostringstream os;
	for (auto stateMap = slipperyInsStateMap.begin(); stateMap != slipperyInsStateMap.end(); stateMap++) {
		os<<"所有合约活跃的报单状态如下\nInstrument: " << stateMap->first;
		for (auto orderItem = stateMap->second.begin(); orderItem != stateMap->second.end(); orderItem++) {
			os<< "\t\tReqId: " << orderItem->first << "--->" << "state: "<<orderItem->second->getState();
		}
	}
	LOG_INFO(os);
}

void CTraderHandler::waitForProcess()
{
	ostringstream os;
	reqQueueMtx.lock();
	os << "Thread-" << this_thread::get_id() << " takes waitForProcess lock";
	LOG_DEBUG(os);
	while (unRepliedReq >= 0) {
		this_thread::sleep_for(chrono::milliseconds(200));
	}
	unRepliedReq = 0;
	reqQueueMtx.unlock();
	clearStream(os);
	os << "Thread-" << this_thread::get_id() << " release waitForProcess lock";
	LOG_DEBUG(os);
}

void CTraderHandler::releaseProcessLock(int reqId)
{
	if (unRepliedReq == reqId) {
		unRepliedReq = -1;
	}
}

void CTraderHandler::logDebug(const char* file, int line, ostringstream& stream)
{
	log(debugLogFile, file, line, stream.str().c_str());
}

void CTraderHandler::logInfo(const char* file, int line, ostringstream& stream)
{
	log(infoLogFile, file, line, stream.str().c_str());
}

void CTraderHandler::logError(const char* file, int line, std::ostringstream& stream)
{
	log(errorLogFile, file, line, stream.str().c_str());
}

void CTraderHandler::log(FILE* logFile, const char* codeFile, int line, const char* msg)
{
	logMtx.lock();
	fprintf(stdout, "%s [%s: %d] %s\n", __TIME__, codeFile, line, msg);
	fprintf(logFile, "%s [%s: %d] %s\n", __TIME__, codeFile, line, msg);
	fflush(stdout);
	fflush(logFile);
	logMtx.unlock();
}

void CTraderHandler::slipPhaseCProcess() {
	ostringstream os;
	os << "开始第三阶段检测 in Thread-"<<this_thread::get_id();
	LOG_INFO(os);
	while (true) {
		bool orderSubmit = false;
		for (auto stateMap = slipperyInsStateMap.begin(); stateMap != slipperyInsStateMap.end(); stateMap++) {
			if (SlipperyPhase::getPhase() == SlipperyPhase::OUT_OF_PHASE) {
				clearStream(os);
				os << "Time is up, stop Phase 3";
				LOG_INFO(os);
				return;
			}
			vector<int> delList;
			mtx.lock();
			for (auto orderItem = stateMap->second.begin(); orderItem != stateMap->second.end(); orderItem++) {
				//LOG_INFO( "Instrument " + stateMap->first + ", reqId is " + orderItem->first + ", state is " + orderItem->second->getState());
				// 暂未match的报单, 撤回
				if (orderItem->second->getState() == SlipperyInsState::ORDERED) {
					cancelInstrument(orderItem->first);
					orderSubmit = true;
					break;
				}//已撤回和确定未成单的报单，再下单。此类型报单需要对map进行修改，需要加锁
				else if (orderItem->second->getState() == SlipperyInsState::RETRIVED
					|| orderItem->second->getState() == SlipperyInsState::UNTRADED
					|| orderItem->second->getState() == SlipperyInsState::ORDER_FAILED) {
					delList.push_back(orderItem->first);
					submitSlipperyOrder(stateMap->first);
					orderSubmit = true;
					break;
				}
			}
			for (auto delIter = delList.begin(); delIter != delList.end(); delIter++) {
				slipperyInsStateMap[stateMap->first].erase(*delIter);
			}
			mtx.unlock();
			if (orderSubmit) {
				break;
			}
		}
		// 如果没有合约需要改变，跳出循环检查
		if (!orderSubmit) {
			break;
		}
	}

	printSlipperyInsStateMap();
	clearStream(os);
	os << "第三阶段完成，close";
	LOG_INFO(os);
	terminate();
}

void CTraderHandler::backgroundQuery() {
	while (runningFlag) {
		insQueryId++;
		if (insQueryId >= allInstruments.size()) {
			insQueryId = 0;
		}
		string instrument = allInstruments.at(insQueryId);
		auto iter = instrumentsExchange.find(instrument);
		queryDepthMarketData(instrument, iter->second);
	}
	
}

vector<string> CTraderHandler::loadInstruments() {
	ostringstream os;
	vector<string> content;
	// load doc1.log into auctionInsOrderMap. doc1存储集合竞价的合约单
	bool readSucc = loadFile2Vector("doc1.log", content);
	if (!readSucc) {
		os << "load doc1.log FAILED";
		LOG_ERROR(os);
	}
	else {
		clearStream(os);
		os << "load doc1.log succeed!";
		LOG_INFO(os);
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
		clearStream(os);
		os << "load doc2.log FAILED";
		LOG_ERROR(os);
	}
	else {
		clearStream(os);
		os << "load doc2.log succeed!";
		LOG_INFO(os);
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
	ostringstream os;
	releaseProcessLock(nRequestID);
	this->pDepthMarketData = pDepthMarketData;
	if (pRspInfo != nullptr) {
		if (pRspInfo != nullptr) {
			os << "错误ID:" << pRspInfo->ErrorID;
		}

		if (pRspInfo != nullptr) {
			os<<"错误消息:" <<pRspInfo->ErrorMsg ;
		}
			LOG_ERROR(os ) ;
		return;
	}
	if (pDepthMarketData != nullptr) {
		auto it = instrumentInfoMap.find(pDepthMarketData->InstrumentID);
		if (it != instrumentInfoMap.end()) {
			// find one in map
			InstrumentInfo preInfo = it->second;
			if (!preInfo.isLatestInfo(nRequestID)) {
				preInfo.updateInfo(nRequestID, pDepthMarketData);
			}
		}
		else {
			// not find
			InstrumentInfo info(nRequestID, pDepthMarketData);
			instrumentInfoMap.insert(pair<string, InstrumentInfo>(pDepthMarketData->InstrumentID, info));
		}
	}
	else {
		clearStream(os);
		os << "GET Nothing as resp when nRequest: " << nRequestID;
		LOG_ERROR(os)  ;
	}
	// 第一轮查询
	if (!runningFlag) {
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
				runningFlag = true;
				clearStream(os);
				os << "首轮询价完毕\n===========后台轮询查询开始===========";
				LOG_INFO(os) ;
				startQueryThread();
				clearStream(os);
				os << "===========集合竞价开始===========";
				LOG_INFO(os ) ;
				callAuction();
				startSlipPhaseAThread();
				startSlipPhaseBThread();
				startSlipPhaseCThread();
			}
		}
	}/*// 后台查询
	else {
		// 如果nRequestID对应请求不是第一次回调，直接返回
		if (onceQueryMarker.find(nRequestID) == onceQueryMarker.end()) {
			return;
		}
		else {
			// 如果是nRequestID对应请求的第一次回调，首先删除
			onceQueryMarker.erase(onceQueryMarker.find(nRequestID));
			insQueryId++;
			string instrument = allInstruments.at(insQueryId % allInstruments.size());
			// 用于QPS控制
			std::this_thread::sleep_for(chrono::milliseconds(1000));
			queryDepthMarketData(instrument, getExchangeId(instrument));
		}
	}*/
}

void CTraderHandler::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	//LOG_INFO( "查询交易账户响应......" ) ;
	if (pRspInfo != nullptr) {
		if (pRspInfo != nullptr) {
			//LOG_INFO( "错误ID:" + pRspInfo->ErrorID ) ;
		}

		if (pRspInfo != nullptr) {
			//LOG_INFO( "错误消息:" + pRspInfo->ErrorMsg)  ;
		}
	}
}

// 报单回报。当客户端进行报单录入、报单操作及其它原因（如部分成交)导致报单状态发生变化时，交易托管系统会主动通知客户端，该方法会被调用
// insertOrder, order traded, order canceled 均可能回调该函数
void CTraderHandler::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
	ostringstream os;
	os <<"========";
	os <<"OnRtnOrder is called in Thread-" << this_thread::get_id();
	string insId = pOrder->InstrumentID;
	int reqId = pOrder->RequestID;
	releaseProcessLock(reqId);
	
	os << "\ninstrumentid is\t" << insId 
		<< "\npOrder->RequestID is\t" << reqId
		<< "\nOrderSubmitStatus is\t" << pOrder->OrderSubmitStatus 
		<< "\nOrderStatus is\t" << pOrder->OrderStatus
		<< "\nOrderLocalID is\t " << pOrder->OrderLocalID;
	LOG_INFO(os);

	// 报单成功
	// 验证发现报单后第一次回调：pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted, pOrder->OrderStatus == THOST_FTDC_OST_Unknown
	// 判断方法待明确
	if (pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted && pOrder->OrderStatus == THOST_FTDC_OST_Unknown) {
		clearStream(os);
		os << "我们认为=报单成功";
		LOG_INFO( os);
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
				//LOG_ERROR( "NOT IMPOSSIBLE HAPPENS!!!");
				slipperyInsStateMap[insId].insert(pair<int, SlipperyInsState*>(pOrder->RequestID,
					new SlipperyInsState(SlipperyInsState::STATE_ENUM::ORDERED, pOrder->RequestID)));
			}
		}
	}

	// 撤单成功
	// 报单自动撤销后回调发现：pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertRejected, pOrder->OrderStatus == THOST_FTDC_OST_Canceled
	// 判断方法待明确
	else if ((pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertRejected && pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
		|| (pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted && pOrder->OrderStatus == THOST_FTDC_OST_Canceled)) {
		clearStream(os);
		os << "我们认为=报单成功后成交失败";
		LOG_INFO(os);
		if (pOrder->RequestID <= auctionLastReqId) { // 判断该回调对应的req是集合竞价下单
			if (auctionInsStateMap.find(pOrder->InstrumentID) != auctionInsStateMap.end()) {
				auto insState = auctionInsStateMap.find(pOrder->InstrumentID)->second;
				insState->updateOnResp(AuctionInsState::STATE_ENUM::CANCELED , pOrder->RequestID);
			}
		}// 滑点订单回调
		else {
			actionIfSlipperyCanceled(insId, pOrder->RequestID);
			//  如果处于第二部分报单的第三阶段，call slipPhaseCProcess，继续检查slipperyInsStateMap其它合约
			/*
			if (curPhase == SlipperyPhase::PHASE_3) {
				slipPhaseCProcess();
			}*/
		}
	}
	// 合约单成交
	// 看代码解释，未验证
	else if(pOrder->OrderStatus == THOST_FTDC_OST_AllTraded){
		clearStream(os);
		os << "我们认为=合约成交";
		LOG_INFO(os);
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
			/*
			if (curPhase == SlipperyPhase::PHASE_3) {
				slipPhaseCProcess();
			}*/
		}
	}
	else if (pOrder->OrderSubmitStatus == THOST_FTDC_OSS_Accepted && pOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing) {
		clearStream(os);
		os << "我们认为=报单在等待成交";
		LOG_INFO(os);
	}
	else {
		clearStream(os);
		os << "我们未处理该状态";
		LOG_INFO(os);
	}
	return;
}
// 成交回报。当发生成交时交易托管系统会通知客户端，该方法会被调用
// 个人认为，该方法被调用到时，OnRtnOrder也会被调用到，所以该方法中不做实现
void CTraderHandler::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
	//LOG_INFO( "================================================================" ) ;
	//LOG_INFO( "OnRtnTrade is called"  );
	//LOG_INFO( "================================================================" ) ;
}

// 报单录入应答。 当客户端发出过报单录入指令后， 交易托管系统返回响应时，该方法会被调用
void CTraderHandler::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	ostringstream os;
	os <<"================================================================" ;	
	os << "OnRspOrderInsert is called" ;
	os << "nRequestID: " << nRequestID<<endl;
	if (pRspInfo != nullptr) {
		//LOG_INFO( "OnRspOrderInsert says it's failed" ) ;
		//LOG_INFO( "错误代码" + pRspInfo->ErrorID  );
		//LOG_INFO( "错误信息" + pRspInfo->ErrorMsg  );
	}
	if(pInputOrder != nullptr){
		// 但是结果中未有
		os << "pOrder.ReqId " << pInputOrder->RequestID << " order success\n";
		os << "OnRspOrderInsert says insert " << pInputOrder->InstrumentID << " order success\n"  ;
		releaseProcessLock(pInputOrder->RequestID);
	}
	
	LOG_INFO( os ) ;
}

void CTraderHandler::scanSlipperyOrderState() {
	ostringstream os;
	string insId;
	int reqId;
	while (true) {
		bool needScan = false;
		for (auto it = slipperyInsStateMap.begin(); it != slipperyInsStateMap.end(); it++) {
			if (SlipperyPhase::OUT_OF_PHASE == SlipperyPhase::getPhase()) {
				clearStream(os);
				os << "Time is up, stop scaning order state";
				LOG_INFO(os);
				return;
			}
			mtx.lock();
			for (auto itemIt = it->second.begin(); itemIt != it->second.end(); itemIt++) {
				
				if (itemIt->second->getState() != SlipperyInsState::ORDERED
					// && itemIt->second->getState() != SlipperyInsState::STARTED
					&& itemIt->second->getState() != SlipperyInsState::RETRIVED) {
					// 仅两状态需要scan
					// STARTED状态不查询，因为该状态下还未形成报单，没有对应的order信息可获取
					continue;
				}
				else { //只需要找到一个需要scan的合约就进行Query，等待回调函数中再call该函数
					needScan = true;
					insId = it->first;
					reqId = itemIt->first;
					break;
				}
			}
			mtx.unlock();
			if (needScan) {
				queryOrderState(reqId, insId);
				break;
			}
		}

	// 如果needScan == false, 意味着slipperyInsStateMap中所有的合约处于NO_INFO, ORDER_FAILED, DONE状态
	// 已完成第二部分报单任务, 该线程可结束
		if (!needScan) {
			break;
		}
	}
}

void CTraderHandler::queryOrderState(int reqId, string insId) {
	ostringstream os;
	CThostFtdcOrderField* order = slipperyRtnOrderMap[reqId];
	CThostFtdcQryOrderField field = { 0 };
	// 文档注释里说，“不写 BrokerID 可以收全所有报单。” 不懂什么意思
	strcpy_s(field.BrokerID, getConfig("config", "BrokerID").c_str());
	strcpy_s(field.InvestorID, getConfig("config", "InvestorID").c_str());
	strcpy_s(field.InstrumentID, insId.c_str());
	strcpy_s(field.OrderSysID, order->OrderSysID);
	int result = 0;
	int retry = 3;
	waitForProcess();
	while (--retry > 0) {
		result = pUserTraderApi->ReqQryOrder(&field, ++orderReqIndex);
		if (0 == result) {
			unRepliedReq = orderReqIndex;
			break;
		}
		else {
			if (result == -1) {
				os << "ReqQryOrder encountered 网络连接失败";
				LOG_ERROR(os);
			}
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
	// 此处经常result=-2，表示未处理请求超过许可数
	if (result < 0) {
		clearStream(os);
		os << insId << " 订单状态查询失败, result = " << result;
		LOG_INFO(os);
		unRepliedReq = -1;
		// then wait for a while
		this_thread::sleep_for(chrono::milliseconds(1000));
	}
	// 暂停0.2s 用来控QPS
	this_thread::sleep_for(chrono::milliseconds(200));
}

void CTraderHandler::actionIfSlipperyTraded(string instrumentId, int reqId) {
	ostringstream os;
	os << instrumentId << "\treqId-" << reqId << "在第二部分";
    if(reqId <= phaseILastReqId){
		os << "阶段一成交";
	}
	else if(reqId <= phaseIILastReqId){
		os << "阶段二成交";
	}
	else {
		os << "阶段三成交";
	}
	LOG_INFO(os);
	if (slipperyInsStateMap[instrumentId].find(reqId) != slipperyInsStateMap[instrumentId].end()) {
		auto insState = slipperyInsStateMap.find(instrumentId)->second;
		// Traded是某一合约的终态，不需要check之前的状态
		slipperyInsStateMap[instrumentId][reqId]->updateOnResp(SlipperyInsState::STATE_ENUM::DONE, reqId);
	}
	else {
		// NOT POSSIBLE
		clearStream(os);
		os << "NOT POSSIBLE HAPPENS!!!";
		LOG_ERROR(os);
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
	ostringstream os;
	os << "(经过多次实验，该function不容易被触发) 主动撤回合约ReqId " << reqId <<"\n";
	if (slipperyRtnOrderMap.find(reqId) == slipperyRtnOrderMap.end()) {
		os << "ReqId " << reqId << " 在slipperyRtnOrderMap中缺失\n";
		LOG_INFO(os);
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
	os <<"thread-" << this_thread::get_id() << " calls waitForProcess\n";
	LOG_INFO(os);
	waitForProcess();
	while (retry-- > 0) {
		result = pUserTraderApi->ReqOrderAction(&field, ++orderReqIndex);
		if (0 == result) {
			clearStream(os);
			os << "set unRepliedReq " << orderReqIndex;
			LOG_INFO(os);
			unRepliedReq = orderReqIndex;
			// 由于仅在订单处于ORDERED状态下，才需要发起撤单操作
			// 认为撤单中也是处于ORDERED状态下，所以成功发起撤单后不需要更改状态
			break;
		}
		if (result < 0) {
			if (-1 == result) {
				clearStream(os);
				os << "call ReqOrderAction 网络连接失败";
				LOG_ERROR(os )  ;
			}
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
	// 重试三次，撤单指令发送失败
	if (result < 0) {
		unRepliedReq = -1;
		clearStream(os);
		os << "重试三次，撤单 " << reqId << " 指令发送失败";
		LOG_ERROR( os ) ;
	}
}

// 仅针对滑点订单，会发起报单查询
// 报单查询请求。当客户端发出报单查询指令后，交易托管系统返回响应时，该方法会被调用
void CTraderHandler::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	ostringstream os;
	if (pRspInfo != nullptr) {
		os <<"错误代码：" << pRspInfo->ErrorID  ;
		os << "错误信息：" << pRspInfo->ErrorMsg ;
		LOG_ERROR(os);
	}
	if (pOrder != nullptr) {
		releaseProcessLock(nRequestID);
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
}

/*
Thost 收到撤单指令，如果没有通过参数校验，拒绝接受撤单指令。用户就会收到
OnRspOrderAction 消息，其中包含了错误编码和错误消息
*/
void CTraderHandler::OnRspOrderAction(CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	//LOG_INFO( "OnRspOrderAction is called" ) ;
	if (pRspInfo != nullptr) {
		//LOG_INFO( "错误代码：" + pRspInfo->ErrorID ) ;
		//LOG_INFO( "错误信息：" + pRspInfo->ErrorMsg ) ;
	}
	if (pOrderAction != nullptr) {
		string insId = pOrderAction->InstrumentID;
		// 理论上不会被调到，暂不写处理逻辑
		//LOG_INFO( ""+insId);
		releaseProcessLock(pOrderAction->RequestID);
	}
}

void CTraderHandler::OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo) {
	//LOG_INFO( "OnErrRtnOrderAction is called" ) ;
	if (pRspInfo != nullptr) {
		//LOG_INFO( "错误代码：" + pRspInfo->ErrorID  );
		//LOG_INFO( "错误信息：" + pRspInfo->ErrorMsg  );
	}
	if (pOrderAction != nullptr) {
		string insId = pOrderAction->InstrumentID;
		// 理论上不会被调到，暂不写处理逻辑
		//LOG_INFO( insId);
		releaseProcessLock(pOrderAction->RequestID);
	}
}

void CTraderHandler::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo)
{
	//LOG_INFO( "OnErrRtnOrderInsert is called" ) ;
	//LOG_INFO( "================================================================"  );
	if (pRspInfo != nullptr) {
		//LOG_INFO( "错误代码" + pRspInfo->ErrorID ) ;
		//LOG_INFO( "错误信息" + pRspInfo->ErrorMsg ) ;
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
			/*
			if (curPhase == SlipperyPhase::PHASE_3) {
				slipPhaseCProcess();
			}*/
		}
		releaseProcessLock(reqId);
	}
	//LOG_INFO( "================================================================" ) ;
}

void CTraderHandler::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	//LOG_INFO( "=============加载文件 doc1.log & doc2.log =============" ) ;
	loadInstruments();
	//LOG_INFO( "=============开始查询合约信息=============" ) ;

	// 从文件的第一条合约单开始
	string instrument = allInstruments.at(0);
	queryDepthMarketData(instrument,getExchangeId(instrument));

	//callSlippage();
	//beginQuery();
}


void CTraderHandler::queryDepthMarketData(string instrumentId, string exchangeId) {
	CThostFtdcQryDepthMarketDataField instrumentField = { 0 };
	strcpy_s(instrumentField.InstrumentID, instrumentId.c_str());
	strcpy_s(instrumentField.ExchangeID, exchangeId.c_str());
	// 投资者结算结果确认后，查询合约深度行情
	// 查询合约深度行情回调函数：OnRspQryDepthMarketData
	ostringstream os;
	waitForProcess();
	int result = 0;
	int retry = 3;
	while (--retry >= 0) {
		++queryReqIndex;
		result = pUserTraderApi->ReqQryDepthMarketData(&instrumentField, queryReqIndex);
		if (0 == result) {
			// 发送查询成功
			unRepliedReq = queryReqIndex;
			onceQueryMarker.insert(pair<int, string>(queryReqIndex, instrumentId));
			return;
		}
		else {
			// 发送查询失败
			if (-1 == result) {
				//-1，表示网络连接失败
				clearStream(os);
				os << "网络连接失败，导致合约信息查询失败";
				LOG_ERROR(os);
			}
			else {
				// -2，表示未处理请求超过许可数；
				// -3，表示每秒发送请求数超过许可数
				// 等待1s 后重试
				this_thread::sleep_for(chrono::milliseconds(1000));
				//queryDepthMarketData(instrumentId, exchangeId);
			}
		}
	}
	if (result < 0) {
		unRepliedReq = -1;
	}
}

void CTraderHandler::OnRspQryInvestorPosition(
	CThostFtdcInvestorPositionField* pInvestorPosition,
	CThostFtdcRspInfoField* pRspInfo,
	int nRequestID,
	bool bIsLast
)
{
	//LOG_INFO( "查询持仓成功" ) ;

	if (pRspInfo != nullptr) {
		//LOG_INFO( "错误代码：" + pRspInfo->ErrorID ) ;
		//LOG_INFO( "错误信息：" + pRspInfo->ErrorMsg ) ;
	}
	//LOG_INFO( "================================================================" ) ;
	if (pInvestorPosition != nullptr) {
	}
	//LOG_INFO( "================================================================" ) ;
}
