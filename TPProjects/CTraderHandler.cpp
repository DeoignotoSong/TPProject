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

	std::cout << "�ͻ�����֤ = "  << b << endl;
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
	int vol = orderInfo.getVol();
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
	std::cout << "Connect Success......" << endl;
	// API���ӳɹ��󣬵��ÿͻ�����֤�ӿ�
	// �ͻ�����֤�ӿڻص�������OnRspAuthenticate
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
	
	// �ͻ�����֤�ɹ����û���¼
	// �û���¼�ص�������OnRspUserLogin
	int result = pUserTraderApi->ReqUserLogin(&userField, orderReqIndex++);

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

	// �û���¼�ɹ��󣬽�����ȷ�ϣ��ڿ�ʼÿ�ս���ǰ������Ҫ��ȷ�ϣ�ÿ��ȷ��һ�μ���
	// ������ȷ�ϻص�������OnRspSettlementInfoConfirm
	pUserTraderApi->ReqSettlementInfoConfirm(&confirmField, nRequestID++);
}


void CTraderHandler::beginQuery() {
	int operation = 0;
	std::cout << "������ѡ��Ĳ�����\n0.��ѯ�˻���\n1.��ѯ�ֲ֣�\n2.���Ͼ����µ���\n3.��Լ��ѯ����������";
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
			cout << instrumentId << " �����µ�һ��" << endl;
			++orderReqIndex;
			// ���Ͼ��۵�price����
			CThostFtdcInputOrderField order = composeAuctionInputOrder(instrumentId);
			result = pUserTraderApi->ReqOrderInsert(&order, orderReqIndex);
			if (0 == result) {
				auctionInsStateMap.insert(pair<string, AuctionInsState*>(instrumentId,
					new AuctionInsState(AuctionInsState::STATE_ENUM::STARTED, orderReqIndex)));
				break;
			}
			else {
				if (-1 == result) {
					cout << "���Ͼ����µ�����������ʧ��" << endl;
				}
				//-1,-2,-3 �����������һ��֮������
				this_thread::sleep_for(chrono::milliseconds(1000));
			}
		}
		if (result < 0) {
			cout << instrumentId << " �����µ�ʧ��" << endl;
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
	// ��¼��ǰ���ںν׶�
	curPhase = phase;
	for (auto iter = slipperyInsOrderMap.begin(); iter != slipperyInsOrderMap.end(); iter++) {
		string instrumentId = iter->first;
		if (instrumentInfoMap.find(instrumentId) == instrumentInfoMap.end()) {
			// ���instrumentId����instrumentInfoMap��˵����ѯ����ʧ��
			slipperyInsStateMap[instrumentId] = new SlipperyInsState(SlipperyInsState::STATE_ENUM::NO_INFO, -1);
			continue;
		}
		// �ڶ��׶Σ�ֻ��δ�ɽ��Ĳ��ֺ�Լ����
		if (phase == SlipperyPhase::PHASE_2) {
			auto pair = slipperyInsStateMap.find(instrumentId);
			if (pair == slipperyInsStateMap.end()
				|| pair->second->getState() != SlipperyInsState::UNTRADED) {
				continue;
			}
		}
		submitSlipperyOrder(instrumentId);
	}
}

void CTraderHandler::submitSlipperyOrder(string instrumentId) {
	int result = 0;
	int retry = 3;
	while (retry-- > 0)
	{
		cout << instrumentId << " �ڶ������µ�һ��" << endl;
		++orderReqIndex;
		CThostFtdcInputOrderField order = composeSlipInputOrder(instrumentId);
		result = pUserTraderApi->ReqOrderInsert(&order, orderReqIndex);
		if (0 == result) {
			slipperyInsStateMap[instrumentId] = 
				new SlipperyInsState(SlipperyInsState::STATE_ENUM::STARTED, orderReqIndex);
			break;
		}
		else {
			if (-1 == result) {
				cout << "�ڶ������µ�����������ʧ��" << endl;
			}
			//-1,-2,-3 �����������һ��֮������
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
	if (result < 0) {
		cout << instrumentId << " �ڶ������µ�ʧ��" << endl;
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
		cerr << "startPollThread failed" << endl;
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
		cerr << "startScanThread failed" << endl;
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
		cerr << "startScanThread failed" << endl;
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
		cerr << "startScanThread failed" << endl;
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
		cerr << "startScanThread failed" << endl;
		return false;
	}
	return true;
}
void CTraderHandler::slipPhaseCEntrance() {
	this_thread::sleep_until(getSlipPhaseCStartTime());
	curPhase = SlipperyPhase::PHASE_3;
	slipPhaseCProcess();
}
void CTraderHandler::slipPhaseCProcess() {
	bool orderSubmit = false;
	for (auto state = slipperyInsStateMap.begin(); state != slipperyInsStateMap.end(); state++) {
		// ��δmatch�ı���, ����
		if (state->second->getState() == SlipperyInsState::ORDERED) {
			cancelInstrument(state->first);
			orderSubmit = true;
			break;
		}//�ѳ��غ�ȷ��δ�ɵ��ı��������µ�
		else if (state->second->getState() == SlipperyInsState::RETRIVED
			|| state->second->getState() == SlipperyInsState::UNTRADED) {
			submitSlipperyOrder(state->first);
			orderSubmit = true;
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
	queryDepthMarketData(instrument, iter->second);
}

vector<string> CTraderHandler::loadInstruments() {
	vector<string> content;
	// load doc1.log into auctionInsOrderMap. doc1�洢���Ͼ��۵ĺ�Լ��
	bool readSucc = loadFile2Vector("doc1.log", content);
	if (!readSucc) {
		cout << "load doc1.log FAILED" << endl;
	}
	else {
		cout << "load doc1.log succeed!" << endl;
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
		cout << "load doc2.log FAILED" << endl;
	}
	else {
		cout << "load doc2.log succeed!" << endl;
		// load���ݽ����ڴ�
		for (size_t i = 0; i < content.size(); i++)
		{
			vector<string> arr = split(content.at(i), ",");
			allInstruments.push_back(arr.at(1));
			instrumentsExchange[arr.at(1)] = arr.at(2);
			slipperyInsOrderMap.insert(pair<string, InstrumentOrderInfo>(arr.at(1), InstrumentOrderInfo(arr.at(3))));
		}
	}
	return content;
}

// �����ѯ������Ӧ�����ͻ��˷��������ѯ����ָ��󣬽����й�ϵͳ������Ӧʱ���÷����ᱻ���á�
void CTraderHandler::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	this->pDepthMarketData = pDepthMarketData;
	if (pRspInfo != nullptr) {
		if (pRspInfo != nullptr) {
			std::cout << "����ID:" << pRspInfo->ErrorID << std::endl;
		}

		if (pRspInfo != nullptr) {
			std::cout << "������Ϣ:" << pRspInfo->ErrorMsg << std::endl;
		}
		return;
	}
	std::cout << "OnRspQryDepthMarketData ReqId:" << nRequestID << std::endl;
	if (pDepthMarketData != nullptr) {
		cout << pDepthMarketData->InstrumentID << "��ѯ������Ӧ" << endl;
		auto it = instrumentInfoMap.find(pDepthMarketData->InstrumentID);
		if (it != instrumentInfoMap.end()) {
			// find one in map
			InstrumentInfo preInfo = it->second;
			if (!preInfo.isLatestInfo(nRequestID)) {
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
				cout << "����ѯ�����\n===========��̨��ѯ��ѯ��ʼ===========" << endl;
				startPollThread();
				this_thread::sleep_until(getAuctionStartTime());
				cout << "===========���Ͼ��ۿ�ʼ===========" << endl;
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
	cout << "��ѯ�����˻���Ӧ......" << endl;
	if (pRspInfo != nullptr) {
		if (pRspInfo != nullptr) {
			cout << "����ID:" << pRspInfo->ErrorID << endl;
		}

		if (pRspInfo != nullptr) {
			std::cout << "������Ϣ:" << pRspInfo->ErrorMsg << std::endl;
		}
	}
	std::cout << "�������:" << nRequestID << std::endl;
	std::cout << "IsLast:" << bIsLast << std::endl;

	std::cout << "================================================================" << std::endl;
	std::cout << "���͹�˾���룺" << pTradingAccount->BrokerID << endl;
	std::cout << "Ͷ�����˺ţ�" << pTradingAccount->AccountID << endl;
	std::cout << "�����ʽ�" << pTradingAccount->Available << endl;
	std::cout << "����" << pTradingAccount->Deposit << endl;
	std::cout << "�����" << pTradingAccount->Withdraw << endl;
	std::cout << "================================================================" << std::endl;

	//beginQuery();
}

// �����ر������ͻ��˽��б���¼�롢��������������ԭ���粿�ֳɽ������±���״̬�����仯ʱ�������й�ϵͳ������֪ͨ�ͻ��ˣ��÷����ᱻ����
// insertOrder, order traded, order canceled �����ܻص��ú���
void CTraderHandler::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
	std::cout << "================================================================" << std::endl;
	std::cout << "OnRtnOrder is called" << std::endl;

	string insId = pOrder->InstrumentID;
	cout << "instrumentid is " << insId << endl;
	cout << "OrderSubmitStatus is " << pOrder->OrderSubmitStatus << endl;
	cout << "OrderStatus is " << pOrder->OrderStatus << endl;

	// �����ɹ�
	// ��֤���ֱ������һ�λص���pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted, pOrder->OrderStatus == THOST_FTDC_OST_Unknown
	// �жϷ�������ȷ
	if (pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted) {
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
			slipperyRtnOrderMap[insId] = pOrder;
			//slipperyInsStateMap�д���
			if (slipperyInsStateMap.find(insId) != slipperyInsStateMap.end()) {
				auto insState = slipperyInsStateMap.find(insId)->second;
				if (insState->getLatestReqId() == pOrder->RequestID // �ص���Ӧ��reqId == ����״̬��reqId
					&& (insState->getState() == SlipperyInsState::STATE_ENUM::STARTED
						|| insState->getState() == SlipperyInsState::STATE_ENUM::RETRIVED))  // Ҫ�����һ��״̬��UNSTARTED����RETRIVED
				{ // ����slipperyInsStateMap�е�state �� respId
					insState->updateOnResp(SlipperyInsState::STATE_ENUM::ORDERED, pOrder->RequestID);
				}
			}
			else {
				// NOT POSSIBLE
				slipperyInsStateMap.insert(pair<string, SlipperyInsState*>(insId,
					new SlipperyInsState(SlipperyInsState::STATE_ENUM::ORDERED, pOrder->RequestID)));
			}
		}

/*
		if (0 == auctionIns.size() && !auctionOverFlag) {
			cout << "���Ͼ��۽׶α������" << endl;
			auctionOverFlag = true;
			// ���Ͼ���֮����Ҫ�ȴ�һ��ʱ��
			this_thread::sleep_until(getSlipPhaseAStartTime());
			cout << "��ʼɨ���Լ״̬" << endl;
			scanSlipperyOrderState();
			slipStartFlag = true;
		}
		*/
	}

	// �����ɹ�
	// �����Զ�������ص����֣�pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertRejected, pOrder->OrderStatus == THOST_FTDC_OST_Canceled
	// �жϷ�������ȷ
	else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled) {
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
	return;
}
// �ɽ��ر����������ɽ�ʱ�����й�ϵͳ��֪ͨ�ͻ��ˣ��÷����ᱻ����
// ������Ϊ���÷��������õ�ʱ��OnRtnOrderҲ�ᱻ���õ������Ը÷����в���ʵ��
void CTraderHandler::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
	cout << "================================================================" << std::endl;
	cout << "OnRtnTrade is called" << std::endl;
	std::cout << "================================================================" << std::endl;
}

// ����¼��Ӧ�� ���ͻ��˷���������¼��ָ��� �����й�ϵͳ������Ӧʱ���÷����ᱻ����
void CTraderHandler::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "OnRspOrderInsert is called" << std::endl;
	std::cout << "================================================================" << std::endl;	
	if (pRspInfo != nullptr) {
		std::cout << "OnRspOrderInsert says it's failed" << std::endl;
		std::cout << "�������" << pRspInfo->ErrorID << endl;
		std::cout << "������Ϣ" << pRspInfo->ErrorMsg << endl;
	}
	else if(pInputOrder != nullptr){
		// ���ǽ����δ��
		cout << "OnRspOrderInsert says insert " << pInputOrder->InstrumentID << " order success" << endl;

	}
	
	std::cout << "================================================================" << std::endl;
}

void CTraderHandler::scanSlipperyOrderState() {
	bool needScan = false;
	string insId;
	for (auto it = slipperyInsStateMap.begin(); it != slipperyInsStateMap.end(); it++) {
		if (it->second->getState() != SlipperyInsState::STARTED
			&& it->second->getState() != SlipperyInsState::ORDERED
			&& it->second->getState() != SlipperyInsState::RETRIVED) {//����״̬��Ҫscan
			continue;
		}
		else { //ֻ��Ҫ�ҵ�һ����Ҫscan�ĺ�Լ�ͽ���Query���ȴ��ص���������call�ú���
			needScan = true;
			insId = it->first;
			break;
		}
	}
	// ���needScan == false, ��ζ��slipperyInsStateMap�����еĺ�Լ����NO_INFO, ORDER_FAILED, DONE״̬
	// ����ɵڶ����ֱ�������, ���߳̿ɽ���
	if (!needScan) {
		terminate();
	}// ���needScan == true����ζ�����к�Լ��Ҫscan
	else
	{
		CThostFtdcOrderField* order = slipperyRtnOrderMap.find(insId)->second;
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
					cout << "ReqQryOrder encountered ��������ʧ��" << endl;
				}
				this_thread::sleep_for(chrono::milliseconds(1000));
			}
		}
		// �˴�����result=-2����ʾδ�������󳬹������
		if (result < 0) {
			cout << insId << " ����״̬��ѯʧ��, result = " << result << endl;
		}
		// ��ͣ0.2s ������QPS
		this_thread::sleep_for(chrono::milliseconds(200));
	}
}

void CTraderHandler::actionIfSlipperyTraded(string instrumentId, int reqId) {
	if (slipperyInsStateMap.find(instrumentId) != slipperyInsStateMap.end()) {
		auto insState = slipperyInsStateMap.find(instrumentId)->second;
		// Traded��ĳһ��Լ����̬������Ҫcheck֮ǰ��״̬
		insState->updateOnResp(SlipperyInsState::STATE_ENUM::DONE, reqId);
	}
	else {
		// NOT POSSIBLE
		slipperyInsStateMap.insert(pair<string, SlipperyInsState*>(instrumentId,
			new SlipperyInsState(SlipperyInsState::STATE_ENUM::DONE, reqId)));
	}
	slipperyFinishedIns[instrumentId] = 1;
}

void CTraderHandler::actionIfSlipperyCanceled(string instrumentId, int reqId) {
	if (slipperyInsStateMap.find(instrumentId) != slipperyInsStateMap.end()) {
		auto insState = slipperyInsStateMap.find(instrumentId)->second;
		if (insState->getLatestReqId() == reqId // �ص���Ӧ��reqId == ����״̬��reqId
			&& (insState->getState() == SlipperyInsState::STATE_ENUM::STARTED
				|| insState->getState() == SlipperyInsState::STATE_ENUM::ORDERED))  // Ҫ�����һ��״̬��UNSTARTED����ORDERED
		{ // ����slipperyInsStateMap�е�state �� respId
			insState->updateOnResp(SlipperyInsState::STATE_ENUM::RETRIVED, reqId);
		}
	}
}

void CTraderHandler::cancelInstrument(string instrumentId) {
	auto pOrder = slipperyRtnOrderMap.find(instrumentId)->second;
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
			break;
		}
		if (result < 0) {
			if (-1 == result) {
				cout << "call ReqOrderAction ��������ʧ��" << endl;
			}
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
	// �������Σ�����ָ���ʧ��
	if (result < 0) {
		cout << "�������Σ����� " << instrumentId << " ָ���ʧ��" << endl;
	}
}

// ����Ի��㶩�����ᷢ�𱨵���ѯ
// ������ѯ���󡣵��ͻ��˷���������ѯָ��󣬽����й�ϵͳ������Ӧʱ���÷����ᱻ����
void CTraderHandler::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	cout << "OnRspQryOrder is called" << endl;
	if (pRspInfo != nullptr) {
		std::cout << "������룺" << pRspInfo->ErrorID << endl;
		std::cout << "������Ϣ��" << pRspInfo->ErrorMsg << endl;
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
			slipperyRtnOrderMap[pOrder->InstrumentID] = pOrder;
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
	cout << "OnRspOrderAction is called" << endl;
	if (pRspInfo != nullptr) {
		cout << "������룺" << pRspInfo->ErrorID << endl;
		cout << "������Ϣ��" << pRspInfo->ErrorMsg << endl;
	}
	if (pOrderAction != nullptr) {
		string insId = pOrderAction->InstrumentID;
		// �����ϲ��ᱻ�������ݲ�д�����߼�
		cout << insId << endl;
	}
}

void CTraderHandler::OnErrRtnOrderAction(
	CThostFtdcOrderActionField* pOrderAction,
	CThostFtdcRspInfoField* pRspInfo) {
	cout << "OnErrRtnOrderAction is called" << endl;
	if (pRspInfo != nullptr) {
		cout << "������룺" << pRspInfo->ErrorID << endl;
		cout << "������Ϣ��" << pRspInfo->ErrorMsg << endl;
	}
	if (pOrderAction != nullptr) {
		string insId = pOrderAction->InstrumentID;
		// �����ϲ��ᱻ�������ݲ�д�����߼�
		cout << insId << endl;
	}
}

void CTraderHandler::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo)
{
	std::cout << "OnErrRtnOrderInsert is called" << std::endl;
	std::cout << "================================================================" << std::endl;
	if (pRspInfo != nullptr) {
		std::cout << "�������" << pRspInfo->ErrorID << endl;
		std::cout << "������Ϣ" << pRspInfo->ErrorMsg << endl;
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
			if (slipperyInsStateMap.find(insId) != slipperyInsStateMap.end()) {
				auto state = slipperyInsStateMap.find(insId)->second;
				state->updateOnResp(SlipperyInsState::ORDER_FAILED, reqId);
			}
			else {
				// NOT POSSIBLE
				slipperyInsStateMap[insId] = new SlipperyInsState(SlipperyInsState::ORDER_FAILED, reqId);
			}
		}
	}
	std::cout << "================================================================" << std::endl;
}

void CTraderHandler::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "=============�����ļ� doc1.log & doc2.log =============" << std::endl;
	loadInstruments();
	std::cout << "=============��ʼ��ѯ��Լ��Ϣ=============" << std::endl;

	// ���ļ��ĵ�һ����Լ����ʼ
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

	// Ͷ���߽�����ȷ�Ϻ󣬲�ѯ��Լ�������
	// ��ѯ��Լ�������ص�������OnRspQryDepthMarketData
	int result = pUserTraderApi->ReqQryDepthMarketData(&instrumentField, ++queryReqIndex);
	if (0 == result) {
		// ���Ͳ�ѯ�ɹ�
		onceQueryMarker.insert(pair<int, string>(orderReqIndex, instrumentId));
		return;
	}
	else {
		// ���Ͳ�ѯʧ��
		if (-1 == result) {
			//-1����ʾ��������ʧ��
			cout << "��������ʧ�ܣ����º�Լ��Ϣ��ѯʧ��" << endl;
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
	std::cout << "��ѯ�ֲֳɹ�" << endl;

	if (pRspInfo != nullptr) {
		std::cout << "������룺" << pRspInfo->ErrorID << endl;
		std::cout << "������Ϣ��" << pRspInfo->ErrorMsg << endl;
	}
	std::cout << "================================================================" << std::endl;
	if (pInvestorPosition != nullptr) {

		std::cout << "��Լ���룺" << pInvestorPosition->InstrumentID << std::endl;
		std::cout << "���͹�˾���룺" << pInvestorPosition->BrokerID << std::endl;
		std::cout << "Ͷ���ߴ��룺" << pInvestorPosition->InvestorID << std::endl;
		std::cout << "�ֲֶ�շ���" << pInvestorPosition->PosiDirection << std::endl;
		std::cout << "Ͷ���ױ���־��" << pInvestorPosition->HedgeFlag << std::endl;
		std::cout << "�ֲ����ڣ�" << pInvestorPosition->PositionDate << std::endl;
		std::cout << "���ճֲ֣�" << pInvestorPosition->YdPosition << std::endl;
		std::cout << "���ճֲ֣�" << pInvestorPosition->Position << std::endl;
		std::cout << "��ͷ���᣺" << pInvestorPosition->LongFrozen << std::endl;
		std::cout << "��ͷ���᣺" << pInvestorPosition->ShortFrozen << std::endl;
		std::cout << "���ֶ����" << pInvestorPosition->LongFrozenAmount << std::endl;
		std::cout << "���ֶ����" << pInvestorPosition->ShortFrozenAmount << std::endl;
		std::cout << "��������" << pInvestorPosition->OpenVolume << std::endl;
		std::cout << "ƽ������" << pInvestorPosition->CloseVolume << std::endl;
		std::cout << "���ֽ�" << pInvestorPosition->OpenAmount << std::endl;
		std::cout << "ƽ�ֽ�" << pInvestorPosition->CloseAmount << std::endl;
		std::cout << "�ֲֳɱ���" << pInvestorPosition->PositionCost << std::endl;
		std::cout << "�ϴ�ռ�õı�֤��" << pInvestorPosition->PreMargin << std::endl;
		std::cout << "ռ�õı�֤��" << pInvestorPosition->UseMargin << std::endl;
		std::cout << "����ı�֤��" << pInvestorPosition->FrozenMargin << std::endl;
		std::cout << "������ʽ�" << pInvestorPosition->FrozenCash << std::endl;
		std::cout << "����������ѣ�" << pInvestorPosition->FrozenCommission << std::endl;
		std::cout << "�ʽ��" << pInvestorPosition->CashIn << std::endl;
		std::cout << "�����ѣ�" << pInvestorPosition->Commission << std::endl;
		std::cout << "ƽ��ӯ����" << pInvestorPosition->CloseProfit << std::endl;
		std::cout << "�ֲ�ӯ����" << pInvestorPosition->PositionProfit << std::endl;
		std::cout << "�ϴν���ۣ�" << pInvestorPosition->PreSettlementPrice << std::endl;
		std::cout << "���ν���ۣ�" << pInvestorPosition->SettlementPrice << std::endl;
		std::cout << "�����գ�" << pInvestorPosition->TradingDay << std::endl;
		std::cout << "�����ţ�" << pInvestorPosition->SettlementID << std::endl;
		std::cout << "���ֳɱ���" << pInvestorPosition->OpenCost << std::endl;
		std::cout << "��������֤��" << pInvestorPosition->ExchangeMargin << std::endl;
		std::cout << "��ϳɽ��γɵĳֲ֣�" << pInvestorPosition->CombPosition << std::endl;
		std::cout << "��϶�ͷ���᣺" << pInvestorPosition->CombLongFrozen << std::endl;
		std::cout << "��Ͽ�ͷ���᣺" << pInvestorPosition->CombShortFrozen << std::endl;
		std::cout << "���ն���ƽ��ӯ����" << pInvestorPosition->CloseProfitByDate << std::endl;
		std::cout << "��ʶԳ�ƽ��ӯ����" << pInvestorPosition->CloseProfitByTrade << std::endl;
		std::cout << "���ճֲ֣�" << pInvestorPosition->TodayPosition << std::endl;
		std::cout << "��֤���ʣ�" << pInvestorPosition->MarginRateByMoney << std::endl;
		std::cout << "��֤����(������)��" << pInvestorPosition->MarginRateByVolume << std::endl;
		std::cout << "ִ�ж��᣺" << pInvestorPosition->StrikeFrozen << std::endl;
		std::cout << "ִ�ж����" << pInvestorPosition->StrikeFrozenAmount << std::endl;
		std::cout << "����ִ�ж��᣺" << pInvestorPosition->AbandonFrozen << std::endl;
		std::cout << "���������룺" << pInvestorPosition->ExchangeID << std::endl;
		std::cout << "ִ�ж������֣�" << pInvestorPosition->YdStrikeFrozen << std::endl;
		std::cout << "Ͷ�ʵ�Ԫ���룺" << pInvestorPosition->InvestUnitID << std::endl;
	}
	std::cout << "================================================================" << std::endl;
}
