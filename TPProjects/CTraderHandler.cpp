// CTraderHandler.h ͨ�� #include "ThostFtdcTraderApi.h"�̳�CThostFtdcTraderSpi
// ����������CThostFtdcTraderSpi��������ʵ��
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

// �ͻ�����֤
void CTraderHandler::ReqAuthenticate()
{
	CThostFtdcReqAuthenticateField authField = { 0 };

	strcpy_s(authField.AuthCode, getConfig("config", "AuthCode").c_str());
	strcpy_s(authField.AppID, getConfig("config", "AppID").c_str());
	strcpy_s(authField.BrokerID, getConfig("config", "BrokerID").c_str());
	strcpy_s(authField.UserID, getConfig("config", "InvestorID").c_str());

	// �ͻ�����֤
	// �ص�������OnRspAuthenticate
	int b = pUserTraderApi->ReqAuthenticate(&authField, orderReqIndex++);

	LOG(INFO) << "�ͻ�����֤ = "  << b  ;
}

// �������Ͼ��۱���
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

// �������㱨��
CThostFtdcInputOrderField CTraderHandler::composeSlipInputOrder(string instrumentID) {
	InstrumentOrderInfo orderInfo = slipperyInsOrderMap.find(instrumentID)->second;
	CThostFtdcDepthMarketDataField* lastestInfo = instrumentInfoMap.find(instrumentID)->second.getInfo();
	
	string exchangeID = getExchangeId(instrumentID);
	bool buyIn = orderInfo.buyOrSell();
	// �ڶ�����Ҫ��ÿ�α���һ��
	int vol = 1;
	double price = choosePrice(lastestInfo);
	int requestId = this->orderReqIndex;
	return composeInputOrder(instrumentID, exchangeID, buyIn, vol, price, THOST_FTDC_TC_GFD, requestId);
}

double CTraderHandler::choosePrice(CThostFtdcDepthMarketDataField* latestInfo) {
	double price = latestInfo->OpenPrice;
	switch (SlipperyPhase::getPhase()) {
	case(SlipperyPhase::PHASE_1): {
		// ���̼�
			break;
	}
	case(SlipperyPhase::PHASE_2): {
		// ���ּ�
		price = latestInfo->BidPrice1;
		break;
	}
	case(SlipperyPhase::PHASE_3): {
		// ���ּۼӸ�
		price = latestInfo->BidPrice1 + 0.0001;
		break;
	}
	}
	return price;
}

// ��������
CThostFtdcInputOrderField CTraderHandler::composeInputOrder(string instrumentID, string exchangeID, bool buyIn, int vol, double price,
	char timeCondition, int requestId) {
	CThostFtdcInputOrderField inputOrderField;
	//��ĳһ���ڴ��е�����ȫ������Ϊָ����ֵ����ʼ��ord
	memset(&inputOrderField, 0, sizeof(inputOrderField));
	// ���͹�˾����
	strcpy_s(inputOrderField.BrokerID, getConfig("config", "BrokerID").c_str());
	// Ͷ���ߴ���
	strcpy_s(inputOrderField.InvestorID, getConfig("config", "InvestorID").c_str());
	///����������
	strcpy_s(inputOrderField.ExchangeID, exchangeID.c_str());
	// ��Լ����
	strcpy_s(inputOrderField.InstrumentID, instrumentID.c_str());
	// �û�����
	strcpy_s(inputOrderField.UserID, getConfig("config", "InvestorID").c_str());
	// ��������
	strcpy_s(inputOrderField.OrderRef, "");
	///��������
	if (buyIn) {
		inputOrderField.Direction = THOST_FTDC_D_Buy;
	}
	else {
		inputOrderField.Direction = THOST_FTDC_D_Sell;
	}
	///��Ͽ�ƽ��־
	inputOrderField.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	///���Ͷ���ױ���־
	inputOrderField.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	///����
	inputOrderField.VolumeTotalOriginal = vol;
	///�ɽ�������
	inputOrderField.VolumeCondition = THOST_FTDC_VC_AV;
	///��С�ɽ���
	inputOrderField.MinVolume = 1;
	///��������
	inputOrderField.ContingentCondition = THOST_FTDC_CC_Immediately;
	///ֹ���
	inputOrderField.StopPrice = 0;
	///ǿƽԭ��
	inputOrderField.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	///�Զ������־
	inputOrderField.IsAutoSuspend = 0;
	///�۸�
	inputOrderField.LimitPrice = price;
	///�����۸�����
	inputOrderField.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	///��Ч�����ͣ����ڹ涨�Ƿ�Ϊ���Ͼ���
	inputOrderField.TimeCondition = timeCondition;
	inputOrderField.RequestID = requestId;

	return inputOrderField;
}

void CTraderHandler::OnFrontConnected()
{
	LOG(INFO) << "Connect Success......"  ;
	// API���ӳɹ��󣬵��ÿͻ�����֤�ӿ�
	// �ͻ�����֤�ӿڻص�������OnRspAuthenticate
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
	
	// �ͻ�����֤�ɹ����û���¼
	// �û���¼�ص�������OnRspUserLogin
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

	// �û���¼�ɹ��󣬽�����ȷ�ϣ��ڿ�ʼÿ�ս���ǰ������Ҫ��ȷ�ϣ�ÿ��ȷ��һ�μ���
	// ������ȷ�ϻص�������OnRspSettlementInfoConfirm
	pUserTraderApi->ReqSettlementInfoConfirm(&confirmField, nRequestID++);
}


void CTraderHandler::beginQuery() {
	int operation = 0;
	LOG(INFO) << "������ѡ��Ĳ�����\n0.��ѯ�˻���\n1.��ѯ�ֲ֣�\n2.���Ͼ����µ���\n3.��Լ��ѯ����������";
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
		//�����ѯ�ʽ��˻�
		result = pUserTraderApi->ReqQryTradingAccount(&tradingAccountField, orderReqIndex++);

		break;
	case 1:
		strcpy_s(investorPositionField.BrokerID, getConfig("config", "BrokerID").c_str());
		strcpy_s(investorPositionField.InvestorID, getConfig("config", "InvestorID").c_str());

		// �����ѯ�˻��ֲ�
		result = pUserTraderApi->ReqQryInvestorPosition(&investorPositionField, orderReqIndex++);

		break;
	case 2:
		// 1. loadInstrumentId()������һ���߳���ѯ���µõ��������
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
			// �ͻ��˷�������¼������
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
	// Ӧ��TraderID��required
	int result = pUserTraderApi->ReqQryTrade(&field, ++orderReqIndex);
}

void CTraderHandler::callAuction() {
	for (auto iter = auctionInsOrderMap.begin(); iter != auctionInsOrderMap.end(); iter++) {
		string instrumentId = iter->first;
		if (instrumentInfoMap.find(instrumentId) == instrumentInfoMap.end()) {
			// ���instrumentId����instrumentInfoMap��˵����ѯ����ʧ��
			auctionInsStateMap.insert(pair<string, AuctionInsState*>(instrumentId,
				new AuctionInsState(AuctionInsState::STATE_ENUM::NO_INFO, -1)));
			continue;
		}
		int result = 0;
		int retry = 3;
		while (retry-->0)
		{
			LOG(INFO) << instrumentId << " �����µ�һ��"  ;
			++orderReqIndex;
			// ���Ͼ��۵�price����
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
					LOG(INFO) << "���Ͼ����µ�����������ʧ��"  ;
				}
				//-1,-2,-3 �����������һ��֮������
				this_thread::sleep_for(chrono::milliseconds(1000));
			}
		}
		if (result < 0) {
			LOG(INFO) << instrumentId << " �����µ�ʧ��"  ;
		}
		this_thread::sleep_for(chrono::milliseconds(200));
	}
	// ��¼���һ�����Ͼ�������ID
	auctionLastReqId = orderReqIndex;
}

// �÷����������߳� startScanThread �б�����
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
		// ��functionֻ�������µ���ǰ���׶�
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
	// ��¼��ǰ���ںν׶�
	curPhase = phase;
	for (auto iter = slipperyInsOrderMap.begin(); iter != slipperyInsOrderMap.end(); iter++) {
		string instrumentId = iter->first;
		InstrumentOrderInfo orderInfo = iter->second;
		LOG(INFO) << "IntrumentId: "<< instrumentId;
		if (instrumentInfoMap.find(instrumentId) == instrumentInfoMap.end()) {
			LOG(INFO) << "��������instrumentInfoMap�У������µ�";
			// ���instrumentId����instrumentInfoMap��˵����ѯ����ʧ��
			for (size_t i = 1; i <= orderInfo.getVol(); i++)
			{
				// ���ں�Լ��Ϣ����ѯ�����ĵ��ӣ�ֱ���Թ�
				slipperyInsStateMap[instrumentId].insert(pair<int, SlipperyInsState*>
					(0-i, new SlipperyInsState(SlipperyInsState::STATE_ENUM::NO_INFO, -1)));
			}
			continue;
		}
		// ��һ�׶Σ������к�Լ�µ�
		if (phase == SlipperyPhase::PHASE_1) {
			// ÿ���µ�һ��
			for (size_t i = 0; i < orderInfo.getVol(); i++)
			{
				submitSlipperyOrder(instrumentId);
			}
		}
		// �ڶ��׶Σ�ֻ��δ�ɽ��Ĳ��ֺ�Լ����
		if (phase == SlipperyPhase::PHASE_2) {
			auto pair = slipperyInsStateMap.find(instrumentId);
			if (pair == slipperyInsStateMap.end()) {
				LOG(INFO) << "��������slipperyInsStateMap�У������µ�";
				continue;
			}
			for (auto orderIter = pair->second.begin(); orderIter != pair->second.end(); orderIter++) {
				if (orderIter->second->getState() == SlipperyInsState::UNTRADED
				|| orderIter->second->getState() == SlipperyInsState::RETRIVED) {
					submitSlipperyOrder(instrumentId);
				}
				else {
					LOG(INFO) << "�׶ζ��У�����״̬Ϊ��"<< orderIter->second->getState() <<", ���µ�";
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
		LOG(INFO) << instrumentId << "�µ�һ��"  ;
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
				LOG(INFO) << "�µ�����������ʧ��"  ;
			}
			//-1,-2,-3 �����������һ��֮������
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
	if (result < 0) {
		LOG(INFO) << instrumentId << "�µ�ʧ��"  ;
	}
	this_thread::sleep_for(chrono::milliseconds(200));
}

bool CTraderHandler::startPollThread() {
	try {
		// ��һ��ʵ�ڿ����������ϲ�����
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
		// ��һ��ʵ�ڿ����������ϲ�����
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
		// ��һ��ʵ�ڿ����������ϲ�����
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
		// ��һ��ʵ�ڿ����������ϲ�����
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
		// ��һ��ʵ�ڿ����������ϲ�����
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
			// ��δmatch�ı���, ����
			if (orderItem->second->getState() == SlipperyInsState::ORDERED) {
				cancelInstrument(orderItem->first);
				orderSubmit = true;
				break;
			}//�ѳ��غ�ȷ��δ�ɵ��ı��������µ��������ͱ�����Ҫ��map�����޸ģ���Ҫ����
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
	// ���û�к�Լ��Ҫ�ı䣬���̹߳ر�
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
	// load doc1.log into auctionInsOrderMap. doc1�洢���Ͼ��۵ĺ�Լ��
	bool readSucc = loadFile2Vector("doc1.log", content);
	if (!readSucc) {
		LOG(INFO) << "load doc1.log FAILED"  ;
	}
	else {
		LOG(INFO) << "load doc1.log succeed!"  ;
		// load���ݽ����ڴ�
		for (size_t i = 0; i < content.size(); i++)
		{
			vector<string> arr = split(content.at(i), ",");
			allInstruments.push_back(arr.at(1));
			instrumentsExchange[arr.at(1)] = arr.at(2);
			auctionInsOrderMap.insert(pair<string, InstrumentOrderInfo>(arr.at(1), InstrumentOrderInfo(arr.at(3))));
		}
	}

	content.clear();
	// load doc2.log into slipperyInsOrderMap. doc2�洢���Ͼ��۵ĺ�Լ��
	readSucc = loadFile2Vector("doc2.log", content);
	if (!readSucc) {
		LOG(INFO) << "load doc2.log FAILED"  ;
	}
	else {
		LOG(INFO) << "load doc2.log succeed!"  ;
		// load���ݽ����ڴ�
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

// �����ѯ������Ӧ�����ͻ��˷��������ѯ����ָ��󣬽����й�ϵͳ������Ӧʱ���÷����ᱻ���á�
void CTraderHandler::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	LOG(DEBUG) << "OnRspQryDepthMarketData is called in " << this_thread::get_id();
	this->pDepthMarketData = pDepthMarketData;
	if (pRspInfo != nullptr) {
		if (pRspInfo != nullptr) {
			LOG(DEBUG) << "����ID:" << pRspInfo->ErrorID  ;
		}

		if (pRspInfo != nullptr) {
			LOG(DEBUG) << "������Ϣ:" << pRspInfo->ErrorMsg  ;
		}
		return;
	}
	LOG(DEBUG) << "OnRspQryDepthMarketData :" << nRequestID  ;
	if (pDepthMarketData != nullptr) {
		LOG(DEBUG) << pDepthMarketData->InstrumentID << "��ȡ��������"  ;
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
	// ��һ�ֲ�ѯ
	if (!startPool) {
		// ���nRequestID��Ӧ�����ǵ�һ�λص���ֱ�ӷ���
		if (onceQueryMarker.find(nRequestID) == onceQueryMarker.end()) {
			return;
		}
		else {
			// �����nRequestID��Ӧ����ĵ�һ�λص�������ɾ��
			onceQueryMarker.erase(onceQueryMarker.find(nRequestID));
			insQueryId++;
			if (insQueryId < allInstruments.size()) {
				string instrument = allInstruments.at((insQueryId));
				// ����QPS����
				std::this_thread::sleep_for(chrono::milliseconds(100));
				queryDepthMarketData(instrument, getExchangeId(instrument));
			}
			else if (insQueryId == allInstruments.size()) {
				startPool = true;
				LOG(INFO) << "����ѯ�����\n===========��̨��ѯ��ѯ��ʼ==========="  ;
				startPollThread();
				LOG(INFO) << "===========���Ͼ��ۿ�ʼ==========="  ;
				callAuction();
				startSlipPhaseAThread();
				startSlipPhaseBThread();
				startSlipPhaseCThread();
			}
		}
	}// ��̨��ѯ
	else {
		// ���nRequestID��Ӧ�����ǵ�һ�λص���ֱ�ӷ���
		if (onceQueryMarker.find(nRequestID) == onceQueryMarker.end()) {
			return;
		}
		else {
			// �����nRequestID��Ӧ����ĵ�һ�λص�������ɾ��
			onceQueryMarker.erase(onceQueryMarker.find(nRequestID));
			insQueryId++;
			string instrument = allInstruments.at((insQueryId % allInstruments.size()));
			// ����QPS����
			std::this_thread::sleep_for(chrono::milliseconds(500));
			queryDepthMarketData(instrument, getExchangeId(instrument));
		}
	}
	//beginQuery();
}

void CTraderHandler::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	LOG(INFO) << "��ѯ�����˻���Ӧ......"  ;
	if (pRspInfo != nullptr) {
		if (pRspInfo != nullptr) {
			LOG(INFO) << "����ID:" << pRspInfo->ErrorID  ;
		}

		if (pRspInfo != nullptr) {
			LOG(INFO) << "������Ϣ:" << pRspInfo->ErrorMsg  ;
		}
	}
	LOG(INFO) << "�������:" << nRequestID  ;
	LOG(INFO) << "IsLast:" << bIsLast  ;

	LOG(INFO) << "================================================================"  ;
	LOG(INFO) << "���͹�˾���룺" << pTradingAccount->BrokerID  ;
	LOG(INFO) << "Ͷ�����˺ţ�" << pTradingAccount->AccountID  ;
	LOG(INFO) << "�����ʽ�" << pTradingAccount->Available  ;
	LOG(INFO) << "����" << pTradingAccount->Deposit  ;
	LOG(INFO) << "�����" << pTradingAccount->Withdraw  ;
	LOG(INFO) << "================================================================"  ;

	//beginQuery();
}

// �����ر������ͻ��˽��б���¼�롢��������������ԭ���粿�ֳɽ������±���״̬�����仯ʱ�������й�ϵͳ������֪ͨ�ͻ��ˣ��÷����ᱻ����
// insertOrder, order traded, order canceled �����ܻص��ú���
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
	

	// �����ɹ�
	// ��֤���ֱ������һ�λص���pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted, pOrder->OrderStatus == THOST_FTDC_OST_Unknown
	// �жϷ�������ȷ
	if (pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted) {
		LOG(INFO) << "������Ϊ=�����ɹ�";
		if (pOrder->RequestID <= auctionLastReqId) {// ���Ͼ��۶����ص�
			// auctionInsStateMap�д���
			if (auctionInsStateMap.find(insId) != auctionInsStateMap.end()) {
				auto insState = auctionInsStateMap.find(insId)->second;
				if (insState->getLatestReqId() == pOrder->RequestID // �ص���Ӧ��reqId == ����״̬��reqId
					&& insState->getState() == AuctionInsState::STATE_ENUM::STARTED)  // Ҫ�����һ��״̬��UNSTARTED
				{ // ����auctionInsStateMap�е�state �� respId
					insState->updateOnResp(AuctionInsState::STATE_ENUM::ORDERED, pOrder->RequestID);
				}
			}
			else {
				// NOT POSSIBLE
				auctionInsStateMap.insert(pair<string, AuctionInsState*>(insId,
					new AuctionInsState(AuctionInsState::STATE_ENUM::ORDERED, pOrder->RequestID)));
			}
		}// ���㶩���ص�
		else{
			slipperyRtnOrderMap[reqId] = pOrder;
			//slipperyInsStateMap�д���
			if (slipperyInsStateMap[insId].find(pOrder->RequestID) != slipperyInsStateMap[insId].end()) {
				auto insState = slipperyInsStateMap[insId].find(pOrder->RequestID)->second;
				if (insState->getLatestReqId() == pOrder->RequestID // �ص���Ӧ��reqId == ����״̬��reqId
					&& (insState->getState() == SlipperyInsState::STATE_ENUM::STARTED
						|| insState->getState() == SlipperyInsState::STATE_ENUM::RETRIVED))  // Ҫ�����һ��״̬��UNSTARTED����RETRIVED
				{ // ����slipperyInsStateMap�е�state �� respId
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

	// �����ɹ�
	// �����Զ�������ص����֣�pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertRejected, pOrder->OrderStatus == THOST_FTDC_OST_Canceled
	// �жϷ�������ȷ
	else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled) {
		LOG(INFO) << "������Ϊ=�����ɹ�";
		if (pOrder->RequestID <= auctionLastReqId) { // �жϸûص���Ӧ��req�Ǽ��Ͼ����µ�
			if (auctionInsStateMap.find(pOrder->InstrumentID) != auctionInsStateMap.end()) {
				auto insState = auctionInsStateMap.find(pOrder->InstrumentID)->second;
				insState->updateOnResp(AuctionInsState::STATE_ENUM::CANCELED , pOrder->RequestID);
			}
		}// ���㶩���ص�
		else {
			actionIfSlipperyCanceled(insId, pOrder->RequestID);
			//  ������ڵڶ����ֱ����ĵ����׶Σ�call slipPhaseCProcess���������slipperyInsStateMap������Լ
			if (curPhase == SlipperyPhase::PHASE_3) {
				slipPhaseCProcess();
			}
		}
	}
	// ��Լ���ɽ�
	// ��������ͣ�δ��֤
	else if(pOrder->OrderStatus == THOST_FTDC_OST_AllTraded){
		LOG(INFO) << "������Ϊ=��Լ�ɽ�";
		if (pOrder->RequestID <= auctionLastReqId) { // ���Ͼ����µ��ص�
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
		else{// ���㶩���ص�
			actionIfSlipperyTraded(insId, pOrder->RequestID);
			//  ������ڵڶ����ֱ����ĵ����׶Σ�call slipPhaseCProcess���������slipperyInsStateMap������Լ
			if (curPhase == SlipperyPhase::PHASE_3) {
				slipPhaseCProcess();
			}
		}
	}
	else {
		LOG(INFO) << "����δ�����״̬";
	}
	return;
}
// �ɽ��ر����������ɽ�ʱ�����й�ϵͳ��֪ͨ�ͻ��ˣ��÷����ᱻ����
// ������Ϊ���÷��������õ�ʱ��OnRtnOrderҲ�ᱻ���õ������Ը÷����в���ʵ��
void CTraderHandler::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
	LOG(INFO) << "================================================================"  ;
	LOG(INFO) << "OnRtnTrade is called"  ;
	LOG(INFO) << "================================================================"  ;
}

// ����¼��Ӧ�� ���ͻ��˷���������¼��ָ��� �����й�ϵͳ������Ӧʱ���÷����ᱻ����
void CTraderHandler::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	LOG(INFO) << "================================================================"  ;	
	LOG(INFO) << "OnRspOrderInsert is called"  ;
	LOG(INFO) << "nRequestID: " << nRequestID;
	LOG(INFO) << "nRequestID: " << nRequestID;
	if (pRspInfo != nullptr) {
		LOG(INFO) << "OnRspOrderInsert says it's failed"  ;
		LOG(INFO) << "�������" << pRspInfo->ErrorID  ;
		LOG(INFO) << "������Ϣ" << pRspInfo->ErrorMsg  ;
	}
	else if(pInputOrder != nullptr){
		// ���ǽ����δ��
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
				&& itemIt->second->getState() != SlipperyInsState::RETRIVED) {//����״̬��Ҫscan
				continue;
			}
			else { //ֻ��Ҫ�ҵ�һ����Ҫscan�ĺ�Լ�ͽ���Query���ȴ��ص���������call�ú���
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
	// ���needScan == false, ��ζ��slipperyInsStateMap�����еĺ�Լ����NO_INFO, ORDER_FAILED, DONE״̬
	// ����ɵڶ����ֱ�������, ���߳̿ɽ���
	if (!needScan) {
		terminate();
	}// ���needScan == true����ζ�����к�Լ��Ҫscan
	else
	{
		CThostFtdcOrderField* order = slipperyRtnOrderMap[reqId];
		CThostFtdcQryOrderField field = { 0 };
		// �ĵ�ע����˵������д BrokerID ������ȫ���б������� ����ʲô��˼
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
					LOG(INFO) << "ReqQryOrder encountered ��������ʧ��"  ;
				}
				this_thread::sleep_for(chrono::milliseconds(1000));
			}
		}
		// �˴�����result=-2����ʾδ�������󳬹������
		if (result < 0) {
			LOG(INFO) << insId << " ����״̬��ѯʧ��, result = " << result  ;
		}
		// ��ͣ0.2s ������QPS
		this_thread::sleep_for(chrono::milliseconds(200));
	}
}

void CTraderHandler::actionIfSlipperyTraded(string instrumentId, int reqId) {
	switch (curPhase)
	{
	case(SlipperyPhase::PHASE_1): {
		LOG(INFO) << "�ڶ����ֱ����׶�һ�ɽ�";
		break;
	}
	case(SlipperyPhase::PHASE_2): {
		LOG(INFO) << "�ڶ����ֱ����׶ζ��ɽ�";
		break;
	}
	case(SlipperyPhase::PHASE_3): {
		LOG(INFO) << "�ڶ����ֱ����׶����ɽ�";
		break;
	}
	default:
		break;
	}
	LOG(INFO) << instrumentId <<" �ѳɽ�";
	if (slipperyInsStateMap[instrumentId].find(reqId) != slipperyInsStateMap[instrumentId].end()) {
		auto insState = slipperyInsStateMap.find(instrumentId)->second;
		// Traded��ĳһ��Լ����̬������Ҫcheck֮ǰ��״̬
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
		if (insState->getLatestReqId() == reqId // �ص���Ӧ��reqId == ����״̬��reqId
			&& (insState->getState() == SlipperyInsState::STATE_ENUM::STARTED
				|| insState->getState() == SlipperyInsState::STATE_ENUM::ORDERED))  // Ҫ�����һ��״̬��UNSTARTED����ORDERED
		{ // ����slipperyInsStateMap�е�state �� respId
			insState->updateOnResp(SlipperyInsState::STATE_ENUM::RETRIVED, reqId);
		}
	}
}

void CTraderHandler::cancelInstrument(int reqId) {
	LOG(INFO) << "���غ�ԼReqId " << reqId;
	if (slipperyRtnOrderMap.find(reqId) == slipperyRtnOrderMap.end()) {
		LOG(ERROR) << "ReqId " << reqId << " ��slipperyRtnOrderMap��ȱʧ";
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
			// ���ڽ��ڶ�������ORDERED״̬�£�����Ҫ���𳷵�����
			// ��Ϊ������Ҳ�Ǵ���ORDERED״̬�£����Գɹ����𳷵�����Ҫ����״̬
			break;
		}
		if (result < 0) {
			if (-1 == result) {
				LOG(INFO) << "call ReqOrderAction ��������ʧ��"  ;
			}
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
	// �������Σ�����ָ���ʧ��
	if (result < 0) {
		LOG(INFO) << "�������Σ����� " << reqId << " ָ���ʧ��"  ;
	}
}

// ����Ի��㶩�����ᷢ�𱨵���ѯ
// ������ѯ���󡣵��ͻ��˷���������ѯָ��󣬽����й�ϵͳ������Ӧʱ���÷����ᱻ����
void CTraderHandler::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	LOG(INFO) << "OnRspQryOrder is called"  ;
	if (pRspInfo != nullptr) {
		LOG(INFO) << "������룺" << pRspInfo->ErrorID  ;
		LOG(INFO) << "������Ϣ��" << pRspInfo->ErrorMsg  ;
	}
	if (pOrder != nullptr) {
		string insId = pOrder->InstrumentID;
		// �ѳɽ�
		if (THOST_FTDC_OST_AllTraded == pOrder->OrderStatus) {
			actionIfSlipperyTraded(insId, pOrder->RequestID);
		}// �ѳ���
		else if (THOST_FTDC_OST_Canceled == pOrder->OrderStatus) {
			actionIfSlipperyCanceled(insId, pOrder->RequestID);
		}
		// �����¼������Ϣ�����ڳ���
		else {
			slipperyRtnOrderMap[nRequestID] = pOrder;
		}
	}
	//�ٴ�scan, ֱ�����б���״̬������̬
	scanSlipperyOrderState();
}

/*
Thost �յ�����ָ����û��ͨ������У�飬�ܾ����ܳ���ָ��û��ͻ��յ�
OnRspOrderAction ��Ϣ�����а����˴������ʹ�����Ϣ
*/
void CTraderHandler::OnRspOrderAction(CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	LOG(INFO) << "OnRspOrderAction is called"  ;
	if (pRspInfo != nullptr) {
		LOG(INFO) << "������룺" << pRspInfo->ErrorID  ;
		LOG(INFO) << "������Ϣ��" << pRspInfo->ErrorMsg  ;
	}
	if (pOrderAction != nullptr) {
		string insId = pOrderAction->InstrumentID;
		// �����ϲ��ᱻ�������ݲ�д�����߼�
		LOG(INFO) << insId  ;
	}
}

void CTraderHandler::OnErrRtnOrderAction(
	CThostFtdcOrderActionField* pOrderAction,
	CThostFtdcRspInfoField* pRspInfo) {
	LOG(INFO) << "OnErrRtnOrderAction is called"  ;
	if (pRspInfo != nullptr) {
		LOG(INFO) << "������룺" << pRspInfo->ErrorID  ;
		LOG(INFO) << "������Ϣ��" << pRspInfo->ErrorMsg  ;
	}
	if (pOrderAction != nullptr) {
		string insId = pOrderAction->InstrumentID;
		// �����ϲ��ᱻ�������ݲ�д�����߼�
		LOG(INFO) << insId  ;
	}
}

void CTraderHandler::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo)
{
	LOG(INFO) << "OnErrRtnOrderInsert is called"  ;
	LOG(INFO) << "================================================================"  ;
	if (pRspInfo != nullptr) {
		LOG(INFO) << "�������" << pRspInfo->ErrorID  ;
		LOG(INFO) << "������Ϣ" << pRspInfo->ErrorMsg  ;
	}

	if (pInputOrder != nullptr) {
		string insId = pInputOrder->InstrumentID;
		int reqId = pInputOrder->RequestID;
		if (reqId <= auctionLastReqId) { //���Ͼ����µ�ʧ��
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
				// ORDER_FAILED����̬�������µ�reqId�����
				slipperyInsStateMap[insId][reqId]->updateOnResp(SlipperyInsState::ORDER_FAILED, reqId);
			}
			else {
				// NOT POSSIBLE
				slipperyInsStateMap[insId][reqId] = new SlipperyInsState(SlipperyInsState::ORDER_FAILED, reqId);
			}
			//  ������ڵڶ����ֱ����ĵ����׶Σ�call slipPhaseCProcess���������slipperyInsStateMap������Լ
			if (curPhase == SlipperyPhase::PHASE_3) {
				slipPhaseCProcess();
			}
		}
	}
	LOG(INFO) << "================================================================"  ;
}

void CTraderHandler::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	LOG(INFO) << "=============�����ļ� doc1.log & doc2.log ============="  ;
	loadInstruments();
	LOG(INFO) << "=============��ʼ��ѯ��Լ��Ϣ============="  ;

	// ���ļ��ĵ�һ����Լ����ʼ
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

	// Ͷ���߽�����ȷ�Ϻ󣬲�ѯ��Լ�������
	// ��ѯ��Լ�������ص�������OnRspQryDepthMarketData
	int result = pUserTraderApi->ReqQryDepthMarketData(&instrumentField, ++queryReqIndex);
	if (0 == result) {
		// ���Ͳ�ѯ�ɹ�
		onceQueryMarker.insert(pair<int, string>(queryReqIndex, instrumentId));
		return;
	}
	else {
		// ���Ͳ�ѯʧ��
		if (-1 == result) {
			//-1����ʾ��������ʧ��
			LOG(DEBUG) << "��������ʧ�ܣ����º�Լ��Ϣ��ѯʧ��"  ;
		}
		else {
			// -2����ʾδ�������󳬹��������
			// -3����ʾÿ�뷢�����������������
			// �ȴ�1s ������
			this_thread::sleep_for(chrono::milliseconds(1000));
			queryDepthMarketData(instrumentId, exchangeId);
		}
	}
}


///�����ѯ����
/**
0������ɹ���
-1����ʾ��������ʧ�ܣ�
-2����ʾδ�������󳬹��������
-3����ʾÿ�뷢�������������������
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
	LOG(INFO) << "��ѯ�ֲֳɹ�"  ;

	if (pRspInfo != nullptr) {
		LOG(INFO) << "������룺" << pRspInfo->ErrorID  ;
		LOG(INFO) << "������Ϣ��" << pRspInfo->ErrorMsg  ;
	}
	LOG(INFO) << "================================================================"  ;
	if (pInvestorPosition != nullptr) {
	}
	LOG(INFO) << "================================================================"  ;
}
