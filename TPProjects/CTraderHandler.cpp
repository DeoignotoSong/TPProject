// CTraderHandler.h ͨ�� #include "ThostFtdcTraderApi.h"�̳�CThostFtdcTraderSpi
// ����������CThostFtdcTraderSpi��������ʵ��
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
	ostringstream os;
	os<< "�ͻ�����֤ = " << b;
	LOG_INFO(os) ;
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
	ostringstream os;
	os << "��ǰ����\n��Լ��" << instrumentID << "\nAskPrice: " << lastestInfo->AskPrice1 << "\nBidPrice: " << lastestInfo->BidPrice1 << "\n";
	os << "�µ���Ϣ\n�µ���Լ��" << instrumentID << "\n�µ��۸�" << price << "\n�µ�����" << vol << "\n��������" << (buyIn ? "��" : "��") << "\n";
	LOG_INFO(os);
	return composeInputOrder(instrumentID, exchangeID, buyIn, vol, price, THOST_FTDC_TC_GFA, requestId);
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
	ostringstream os;
	os << "��ǰ����\n��Լ��" << instrumentID << "\nAskPrice: " << lastestInfo->AskPrice1 << "\nBidPrice: " << lastestInfo->BidPrice1 << "\n";
	os << "�µ���Ϣ\n�µ���Լ��" << instrumentID << "\n�µ��۸�" << price << "\n�µ�����" << vol << "\n��������" << (buyIn ? "��" : "��") << "\n";
	LOG_INFO(os);
	return composeInputOrder(instrumentID, exchangeID, buyIn, vol, price, THOST_FTDC_TC_IOC, requestId);
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
		price = latestInfo->BidPrice1 + atof(getConfig("config", "premium").c_str());
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
	ostringstream os;
	os << "Connect Success......";
	LOG_INFO(os) ;
	// API���ӳɹ��󣬵��ÿͻ�����֤�ӿ�
	// �ͻ�����֤�ӿڻص�������OnRspAuthenticate
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
	
	// �ͻ�����֤�ɹ����û���¼
	// �û���¼�ص�������OnRspUserLogin
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

	// �û���¼�ɹ��󣬽�����ȷ�ϣ��ڿ�ʼÿ�ս���ǰ������Ҫ��ȷ�ϣ�ÿ��ȷ��һ�μ���
	// ������ȷ�ϻص�������OnRspSettlementInfoConfirm
	pUserTraderApi->ReqSettlementInfoConfirm(&confirmField, nRequestID++);
}


void CTraderHandler::beginQuery() {
	int operation = 0;
	ostringstream os;
	os << "������ѡ��Ĳ�����\n0.��ѯ�˻���\n1.��ѯ�ֲ֣�\n2.���Ͼ����µ���\n3.��Լ��ѯ������)��";
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
	// Ӧ��TraderID��required
	int result = pUserTraderApi->ReqQryTrade(&field, ++orderReqIndex);
}

void CTraderHandler::callAuction() {
	ostringstream os;
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
			os << instrumentId << " �����µ�һ��";
			LOG_INFO(os);
			++orderReqIndex;
			// ���Ͼ��۵�price����
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
					os << "���Ͼ����µ�����������ʧ��";
					LOG_ERROR( os ) ;
				}
				//-1,-2,-3 �����������һ��֮������
				this_thread::sleep_for(chrono::milliseconds(1000));
			}
		}
		if (result < 0) {
			clearStream(os);
			os << instrumentId << " �����µ�ʧ��";
			LOG_ERROR( os ) ;
		}
		this_thread::sleep_for(chrono::milliseconds(200));
	}
	// ��¼���һ�����Ͼ�������ID
	auctionLastReqId = orderReqIndex;
}

// �÷����������߳� startScanThread �б�����
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
		// ��functionֻ�������µ���ǰ���׶�
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
	// ��¼��ǰ���ںν׶�
	curPhase = phase;
	for (auto iter = slipperyInsOrderMap.begin(); iter != slipperyInsOrderMap.end(); iter++) {
		string instrumentId = iter->first;
		InstrumentOrderInfo orderInfo = iter->second;
		//LOG_INFO( "IntrumentId: "+ instrumentId);
		if (instrumentInfoMap.find(instrumentId) == instrumentInfoMap.end()) {
			//LOG_INFO( "��������instrumentInfoMap�У������µ�");
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
				phaseILastReqId = orderReqIndex;
			}
		}
		// �ڶ��׶Σ�ֻ��δ�ɽ��Ĳ��ֺ�Լ����
		if (phase == SlipperyPhase::PHASE_2) {
			auto pair = slipperyInsStateMap.find(instrumentId);
			if (pair == slipperyInsStateMap.end()) {
				clearStream(os);
				os << "��������slipperyInsStateMap�У������µ�";
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
					//�µ�ǰ��Ҫ���ɵ�reqId��slipperyInsStateMap��ɾ��
					delList.push_back(orderIter->first);
				}
				else {
					clearStream(os);
					os << "�׶ζ��У�����״̬Ϊ��" << orderIter->second->getState() << ", ���µ�";
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
		os << instrumentId << "�µ�һ�� in Thread-" << this_thread::get_id();
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
				os << "�µ�����������ʧ��";
				LOG_ERROR(os)  ;
			}
			//-1,-2,-3 �����������һ��֮������
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
	if (result < 0) {
		unRepliedReq = -1;
		clearStream(os);
		os << instrumentId << "�µ�ʧ��";
		LOG_INFO( os ) ;
	}
	this_thread::sleep_for(chrono::milliseconds(200));
}

bool CTraderHandler::startQueryThread() {
	try {
		// ��һ��ʵ�ڿ����������ϲ�����
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
		// ��һ��ʵ�ڿ����������ϲ�����
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
		// ��һ��ʵ�ڿ����������ϲ�����
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
		// ��һ��ʵ�ڿ����������ϲ�����
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
		// ��һ��ʵ�ڿ����������ϲ�����
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
		os<<"���к�Լ��Ծ�ı���״̬����\nInstrument: " << stateMap->first;
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
	os << "��ʼ�����׶μ�� in Thread-"<<this_thread::get_id();
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
				// ��δmatch�ı���, ����
				if (orderItem->second->getState() == SlipperyInsState::ORDERED) {
					cancelInstrument(orderItem->first);
					orderSubmit = true;
					break;
				}//�ѳ��غ�ȷ��δ�ɵ��ı��������µ��������ͱ�����Ҫ��map�����޸ģ���Ҫ����
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
		// ���û�к�Լ��Ҫ�ı䣬����ѭ�����
		if (!orderSubmit) {
			break;
		}
	}

	printSlipperyInsStateMap();
	clearStream(os);
	os << "�����׶���ɣ�close";
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
	// load doc1.log into auctionInsOrderMap. doc1�洢���Ͼ��۵ĺ�Լ��
	bool readSucc = loadFile2Vector("doc1.log", content);
	if (!readSucc) {
		os << "load doc1.log FAILED";
		LOG_ERROR(os);
	}
	else {
		clearStream(os);
		os << "load doc1.log succeed!";
		LOG_INFO(os);
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
		clearStream(os);
		os << "load doc2.log FAILED";
		LOG_ERROR(os);
	}
	else {
		clearStream(os);
		os << "load doc2.log succeed!";
		LOG_INFO(os);
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
	ostringstream os;
	releaseProcessLock(nRequestID);
	this->pDepthMarketData = pDepthMarketData;
	if (pRspInfo != nullptr) {
		if (pRspInfo != nullptr) {
			os << "����ID:" << pRspInfo->ErrorID;
		}

		if (pRspInfo != nullptr) {
			os<<"������Ϣ:" <<pRspInfo->ErrorMsg ;
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
	// ��һ�ֲ�ѯ
	if (!runningFlag) {
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
				runningFlag = true;
				clearStream(os);
				os << "����ѯ�����\n===========��̨��ѯ��ѯ��ʼ===========";
				LOG_INFO(os) ;
				startQueryThread();
				clearStream(os);
				os << "===========���Ͼ��ۿ�ʼ===========";
				LOG_INFO(os ) ;
				callAuction();
				startSlipPhaseAThread();
				startSlipPhaseBThread();
				startSlipPhaseCThread();
			}
		}
	}/*// ��̨��ѯ
	else {
		// ���nRequestID��Ӧ�����ǵ�һ�λص���ֱ�ӷ���
		if (onceQueryMarker.find(nRequestID) == onceQueryMarker.end()) {
			return;
		}
		else {
			// �����nRequestID��Ӧ����ĵ�һ�λص�������ɾ��
			onceQueryMarker.erase(onceQueryMarker.find(nRequestID));
			insQueryId++;
			string instrument = allInstruments.at(insQueryId % allInstruments.size());
			// ����QPS����
			std::this_thread::sleep_for(chrono::milliseconds(1000));
			queryDepthMarketData(instrument, getExchangeId(instrument));
		}
	}*/
}

void CTraderHandler::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	//LOG_INFO( "��ѯ�����˻���Ӧ......" ) ;
	if (pRspInfo != nullptr) {
		if (pRspInfo != nullptr) {
			//LOG_INFO( "����ID:" + pRspInfo->ErrorID ) ;
		}

		if (pRspInfo != nullptr) {
			//LOG_INFO( "������Ϣ:" + pRspInfo->ErrorMsg)  ;
		}
	}
}

// �����ر������ͻ��˽��б���¼�롢��������������ԭ���粿�ֳɽ�)���±���״̬�����仯ʱ�������й�ϵͳ������֪ͨ�ͻ��ˣ��÷����ᱻ����
// insertOrder, order traded, order canceled �����ܻص��ú���
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

	// �����ɹ�
	// ��֤���ֱ������һ�λص���pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted, pOrder->OrderStatus == THOST_FTDC_OST_Unknown
	// �жϷ�������ȷ
	if (pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted && pOrder->OrderStatus == THOST_FTDC_OST_Unknown) {
		clearStream(os);
		os << "������Ϊ=�����ɹ�";
		LOG_INFO( os);
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
				//LOG_ERROR( "NOT IMPOSSIBLE HAPPENS!!!");
				slipperyInsStateMap[insId].insert(pair<int, SlipperyInsState*>(pOrder->RequestID,
					new SlipperyInsState(SlipperyInsState::STATE_ENUM::ORDERED, pOrder->RequestID)));
			}
		}
	}

	// �����ɹ�
	// �����Զ�������ص����֣�pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertRejected, pOrder->OrderStatus == THOST_FTDC_OST_Canceled
	// �жϷ�������ȷ
	else if ((pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertRejected && pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
		|| (pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted && pOrder->OrderStatus == THOST_FTDC_OST_Canceled)) {
		clearStream(os);
		os << "������Ϊ=�����ɹ���ɽ�ʧ��";
		LOG_INFO(os);
		if (pOrder->RequestID <= auctionLastReqId) { // �жϸûص���Ӧ��req�Ǽ��Ͼ����µ�
			if (auctionInsStateMap.find(pOrder->InstrumentID) != auctionInsStateMap.end()) {
				auto insState = auctionInsStateMap.find(pOrder->InstrumentID)->second;
				insState->updateOnResp(AuctionInsState::STATE_ENUM::CANCELED , pOrder->RequestID);
			}
		}// ���㶩���ص�
		else {
			actionIfSlipperyCanceled(insId, pOrder->RequestID);
			//  ������ڵڶ����ֱ����ĵ����׶Σ�call slipPhaseCProcess���������slipperyInsStateMap������Լ
			/*
			if (curPhase == SlipperyPhase::PHASE_3) {
				slipPhaseCProcess();
			}*/
		}
	}
	// ��Լ���ɽ�
	// ��������ͣ�δ��֤
	else if(pOrder->OrderStatus == THOST_FTDC_OST_AllTraded){
		clearStream(os);
		os << "������Ϊ=��Լ�ɽ�";
		LOG_INFO(os);
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
			/*
			if (curPhase == SlipperyPhase::PHASE_3) {
				slipPhaseCProcess();
			}*/
		}
	}
	else if (pOrder->OrderSubmitStatus == THOST_FTDC_OSS_Accepted && pOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing) {
		clearStream(os);
		os << "������Ϊ=�����ڵȴ��ɽ�";
		LOG_INFO(os);
	}
	else {
		clearStream(os);
		os << "����δ�����״̬";
		LOG_INFO(os);
	}
	return;
}
// �ɽ��ر����������ɽ�ʱ�����й�ϵͳ��֪ͨ�ͻ��ˣ��÷����ᱻ����
// ������Ϊ���÷��������õ�ʱ��OnRtnOrderҲ�ᱻ���õ������Ը÷����в���ʵ��
void CTraderHandler::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
	//LOG_INFO( "================================================================" ) ;
	//LOG_INFO( "OnRtnTrade is called"  );
	//LOG_INFO( "================================================================" ) ;
}

// ����¼��Ӧ�� ���ͻ��˷���������¼��ָ��� �����й�ϵͳ������Ӧʱ���÷����ᱻ����
void CTraderHandler::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	ostringstream os;
	os <<"================================================================" ;	
	os << "OnRspOrderInsert is called" ;
	os << "nRequestID: " << nRequestID<<endl;
	if (pRspInfo != nullptr) {
		//LOG_INFO( "OnRspOrderInsert says it's failed" ) ;
		//LOG_INFO( "�������" + pRspInfo->ErrorID  );
		//LOG_INFO( "������Ϣ" + pRspInfo->ErrorMsg  );
	}
	if(pInputOrder != nullptr){
		// ���ǽ����δ��
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
					// ����״̬��Ҫscan
					// STARTED״̬����ѯ����Ϊ��״̬�»�δ�γɱ�����û�ж�Ӧ��order��Ϣ�ɻ�ȡ
					continue;
				}
				else { //ֻ��Ҫ�ҵ�һ����Ҫscan�ĺ�Լ�ͽ���Query���ȴ��ص���������call�ú���
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

	// ���needScan == false, ��ζ��slipperyInsStateMap�����еĺ�Լ����NO_INFO, ORDER_FAILED, DONE״̬
	// ����ɵڶ����ֱ�������, ���߳̿ɽ���
		if (!needScan) {
			break;
		}
	}
}

void CTraderHandler::queryOrderState(int reqId, string insId) {
	ostringstream os;
	CThostFtdcOrderField* order = slipperyRtnOrderMap[reqId];
	CThostFtdcQryOrderField field = { 0 };
	// �ĵ�ע����˵������д BrokerID ������ȫ���б������� ����ʲô��˼
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
				os << "ReqQryOrder encountered ��������ʧ��";
				LOG_ERROR(os);
			}
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
	// �˴�����result=-2����ʾδ�������󳬹������
	if (result < 0) {
		clearStream(os);
		os << insId << " ����״̬��ѯʧ��, result = " << result;
		LOG_INFO(os);
		unRepliedReq = -1;
		// then wait for a while
		this_thread::sleep_for(chrono::milliseconds(1000));
	}
	// ��ͣ0.2s ������QPS
	this_thread::sleep_for(chrono::milliseconds(200));
}

void CTraderHandler::actionIfSlipperyTraded(string instrumentId, int reqId) {
	ostringstream os;
	os << instrumentId << "\treqId-" << reqId << "�ڵڶ�����";
    if(reqId <= phaseILastReqId){
		os << "�׶�һ�ɽ�";
	}
	else if(reqId <= phaseIILastReqId){
		os << "�׶ζ��ɽ�";
	}
	else {
		os << "�׶����ɽ�";
	}
	LOG_INFO(os);
	if (slipperyInsStateMap[instrumentId].find(reqId) != slipperyInsStateMap[instrumentId].end()) {
		auto insState = slipperyInsStateMap.find(instrumentId)->second;
		// Traded��ĳһ��Լ����̬������Ҫcheck֮ǰ��״̬
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
		if (insState->getLatestReqId() == reqId // �ص���Ӧ��reqId == ����״̬��reqId
			&& (insState->getState() == SlipperyInsState::STATE_ENUM::STARTED
				|| insState->getState() == SlipperyInsState::STATE_ENUM::ORDERED))  // Ҫ�����һ��״̬��UNSTARTED����ORDERED
		{ // ����slipperyInsStateMap�е�state �� respId
			insState->updateOnResp(SlipperyInsState::STATE_ENUM::RETRIVED, reqId);
		}
	}
}

void CTraderHandler::cancelInstrument(int reqId) {
	ostringstream os;
	os << "(�������ʵ�飬��function�����ױ�����) �������غ�ԼReqId " << reqId <<"\n";
	if (slipperyRtnOrderMap.find(reqId) == slipperyRtnOrderMap.end()) {
		os << "ReqId " << reqId << " ��slipperyRtnOrderMap��ȱʧ\n";
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
			// ���ڽ��ڶ�������ORDERED״̬�£�����Ҫ���𳷵�����
			// ��Ϊ������Ҳ�Ǵ���ORDERED״̬�£����Գɹ����𳷵�����Ҫ����״̬
			break;
		}
		if (result < 0) {
			if (-1 == result) {
				clearStream(os);
				os << "call ReqOrderAction ��������ʧ��";
				LOG_ERROR(os )  ;
			}
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
	// �������Σ�����ָ���ʧ��
	if (result < 0) {
		unRepliedReq = -1;
		clearStream(os);
		os << "�������Σ����� " << reqId << " ָ���ʧ��";
		LOG_ERROR( os ) ;
	}
}

// ����Ի��㶩�����ᷢ�𱨵���ѯ
// ������ѯ���󡣵��ͻ��˷���������ѯָ��󣬽����й�ϵͳ������Ӧʱ���÷����ᱻ����
void CTraderHandler::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	ostringstream os;
	if (pRspInfo != nullptr) {
		os <<"������룺" << pRspInfo->ErrorID  ;
		os << "������Ϣ��" << pRspInfo->ErrorMsg ;
		LOG_ERROR(os);
	}
	if (pOrder != nullptr) {
		releaseProcessLock(nRequestID);
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
}

/*
Thost �յ�����ָ����û��ͨ������У�飬�ܾ����ܳ���ָ��û��ͻ��յ�
OnRspOrderAction ��Ϣ�����а����˴������ʹ�����Ϣ
*/
void CTraderHandler::OnRspOrderAction(CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	//LOG_INFO( "OnRspOrderAction is called" ) ;
	if (pRspInfo != nullptr) {
		//LOG_INFO( "������룺" + pRspInfo->ErrorID ) ;
		//LOG_INFO( "������Ϣ��" + pRspInfo->ErrorMsg ) ;
	}
	if (pOrderAction != nullptr) {
		string insId = pOrderAction->InstrumentID;
		// �����ϲ��ᱻ�������ݲ�д�����߼�
		//LOG_INFO( ""+insId);
		releaseProcessLock(pOrderAction->RequestID);
	}
}

void CTraderHandler::OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo) {
	//LOG_INFO( "OnErrRtnOrderAction is called" ) ;
	if (pRspInfo != nullptr) {
		//LOG_INFO( "������룺" + pRspInfo->ErrorID  );
		//LOG_INFO( "������Ϣ��" + pRspInfo->ErrorMsg  );
	}
	if (pOrderAction != nullptr) {
		string insId = pOrderAction->InstrumentID;
		// �����ϲ��ᱻ�������ݲ�д�����߼�
		//LOG_INFO( insId);
		releaseProcessLock(pOrderAction->RequestID);
	}
}

void CTraderHandler::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo)
{
	//LOG_INFO( "OnErrRtnOrderInsert is called" ) ;
	//LOG_INFO( "================================================================"  );
	if (pRspInfo != nullptr) {
		//LOG_INFO( "�������" + pRspInfo->ErrorID ) ;
		//LOG_INFO( "������Ϣ" + pRspInfo->ErrorMsg ) ;
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
	//LOG_INFO( "=============�����ļ� doc1.log & doc2.log =============" ) ;
	loadInstruments();
	//LOG_INFO( "=============��ʼ��ѯ��Լ��Ϣ=============" ) ;

	// ���ļ��ĵ�һ����Լ����ʼ
	string instrument = allInstruments.at(0);
	queryDepthMarketData(instrument,getExchangeId(instrument));

	//callSlippage();
	//beginQuery();
}


void CTraderHandler::queryDepthMarketData(string instrumentId, string exchangeId) {
	CThostFtdcQryDepthMarketDataField instrumentField = { 0 };
	strcpy_s(instrumentField.InstrumentID, instrumentId.c_str());
	strcpy_s(instrumentField.ExchangeID, exchangeId.c_str());
	// Ͷ���߽�����ȷ�Ϻ󣬲�ѯ��Լ�������
	// ��ѯ��Լ�������ص�������OnRspQryDepthMarketData
	ostringstream os;
	waitForProcess();
	int result = 0;
	int retry = 3;
	while (--retry >= 0) {
		++queryReqIndex;
		result = pUserTraderApi->ReqQryDepthMarketData(&instrumentField, queryReqIndex);
		if (0 == result) {
			// ���Ͳ�ѯ�ɹ�
			unRepliedReq = queryReqIndex;
			onceQueryMarker.insert(pair<int, string>(queryReqIndex, instrumentId));
			return;
		}
		else {
			// ���Ͳ�ѯʧ��
			if (-1 == result) {
				//-1����ʾ��������ʧ��
				clearStream(os);
				os << "��������ʧ�ܣ����º�Լ��Ϣ��ѯʧ��";
				LOG_ERROR(os);
			}
			else {
				// -2����ʾδ�������󳬹��������
				// -3����ʾÿ�뷢�����������������
				// �ȴ�1s ������
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
	//LOG_INFO( "��ѯ�ֲֳɹ�" ) ;

	if (pRspInfo != nullptr) {
		//LOG_INFO( "������룺" + pRspInfo->ErrorID ) ;
		//LOG_INFO( "������Ϣ��" + pRspInfo->ErrorMsg ) ;
	}
	//LOG_INFO( "================================================================" ) ;
	if (pInvestorPosition != nullptr) {
	}
	//LOG_INFO( "================================================================" ) ;
}
