// CTraderHandler.h ͨ�� #include "ThostFtdcTraderApi.h"�̳�CThostFtdcTraderSpi
// ����������CThostFtdcTraderSpi��������ʵ��
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
	int b = pUserTraderApi->ReqAuthenticate(&authField, requestIndex++);

	std::cout << "�ͻ�����֤ = "  << b << endl;
}

// �������Ͼ��۱���
CThostFtdcInputOrderField CTraderHandler::composeAuctionInputOrder(string instrumentID, string exchangeID, bool buyIn, int vol, double price, int requestId) {
	return composeInputOrder(instrumentID, exchangeID, buyIn, vol, price, THOST_FTDC_TC_GFD, requestId);
}

// �������㱨��
CThostFtdcInputOrderField CTraderHandler::composeSlipInputOrder(string instrumentID, string exchangeID, bool buyIn, int vol, double price, int requestId) {
	return composeInputOrder(instrumentID, exchangeID, buyIn, vol, price, THOST_FTDC_TC_GFD, requestId);
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
	std::cout << "InstrumentID: " << inputOrderField.InstrumentID << std::endl;
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
		result = pUserTraderApi->ReqQryTradingAccount(&tradingAccountField, requestIndex++);

		break;
	case 1:
		strcpy_s(investorPositionField.BrokerID, getConfig("config", "BrokerID").c_str());
		strcpy_s(investorPositionField.InvestorID, getConfig("config", "InvestorID").c_str());

		// �����ѯ�˻��ֲ�
		result = pUserTraderApi->ReqQryInvestorPosition(&investorPositionField, requestIndex++);

		break;
	case 2:
		// 1. loadInstrumentId()������һ���߳���ѯ���µõ��������
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
	int result = pUserTraderApi->ReqQryTrade(&field, ++requestIndex);
}

void CTraderHandler::callAuction() {
	for (auto iter = instrumentOrderMap.begin(); iter != instrumentOrderMap.end(); iter++) {
		string instrumentId = iter->first;
		string exchangeId = getExchangeId(instrumentId);

		auto infoIter = instrumentInfoMap.find(instrumentId);
		if (infoIter == instrumentInfoMap.end()) {
			// ���instrumentId����instrumentInfoMap��˵����ѯ����ʧ��
			continue;
		}
		InstrumentOrderInfo orderInfo = iter->second;
		CThostFtdcDepthMarketDataField* lastestInfo = instrumentInfoMap.find(instrumentId)->second.getInfo();
		// ���Ͼ��۵�price����
		CThostFtdcInputOrderField order = composeAuctionInputOrder(instrumentId, exchangeId, orderInfo.buyOrSell(), orderInfo.getVol(), lastestInfo->OpenPrice, requestIndex);
		int result = 0;
		int retry = 3;
		while (retry-->0)
		{
			cout << instrumentId << " �����µ�һ��" << endl;
			result = pUserTraderApi->ReqOrderInsert(&order, ++requestIndex);
			if (0 == result) {
				toInsertIns.insert(pair<string, int>(instrumentId, 1));
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
}

bool CTraderHandler::startPollThread() {
	try {
		// ��һ��ʵ�ڿ����������ϲ�����
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
		// ��һ��ʵ�ڿ����������ϲ�����
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
		// load���ݽ����ڴ�
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
	}
	std::cout << "�������:" << nRequestID << std::endl;
	if (pDepthMarketData != nullptr) {
		cout << pDepthMarketData->InstrumentID << "��ѯ������Ӧ" << endl;
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
				cout << "===========���Ͼ��ۿ�ʼ===========" << endl;
				callAuction();
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
	
	/**
	std::cout << "������:" << pDepthMarketData->TradingDay << std::endl;
	std::cout << "��Լ����:" << pDepthMarketData->InstrumentID << std::endl;
	std::cout << "����������:" << pDepthMarketData->ExchangeID << std::endl;
	std::cout << "��Լ�ڽ������Ĵ���:" << pDepthMarketData->ExchangeInstID << std::endl;
	std::cout << "���¼�:" << pDepthMarketData->LastPrice << std::endl;
	std::cout << "�ϴν����:" << pDepthMarketData->PreSettlementPrice << std::endl;
	std::cout << "������:" << pDepthMarketData->PreClosePrice << std::endl;
	std::cout << "��ֲ���:" << pDepthMarketData->PreOpenInterest << std::endl;
	std::cout << "����:" << pDepthMarketData->OpenPrice << std::endl;
	std::cout << "��߼�:" << pDepthMarketData->HighestPrice << std::endl;
	std::cout << "��ͼ�:" << pDepthMarketData->LowestPrice << std::endl;
	std::cout << "����:" << pDepthMarketData->Volume << std::endl;
	std::cout << "�ɽ����:" << pDepthMarketData->Turnover << std::endl;
	std::cout << "�ֲ���:" << pDepthMarketData->OpenInterest << std::endl;
	std::cout << "������:" << pDepthMarketData->ClosePrice << std::endl;
	std::cout << "���ν����:" << pDepthMarketData->SettlementPrice << std::endl;
	std::cout << "��ͣ���:" << pDepthMarketData->UpperLimitPrice << std::endl;
	std::cout << "��ͣ���:" << pDepthMarketData->LowerLimitPrice << std::endl;
	std::cout << "����ʵ��:" << pDepthMarketData->PreDelta << std::endl;
	std::cout << "����ʵ��:" << pDepthMarketData->CurrDelta << std::endl;
	std::cout << "����޸�ʱ��:" << pDepthMarketData->UpdateTime << std::endl;
	std::cout << "�����һ:" << pDepthMarketData->BidPrice1 << std::endl;
	std::cout << "������һ:" << pDepthMarketData->BidVolume1 << std::endl;
	std::cout << "������һ:" << pDepthMarketData->AskPrice1 << std::endl;
	std::cout << "������һ:" << pDepthMarketData->AskVolume1 << std::endl;
	std::cout << "����۶�:" << pDepthMarketData->BidPrice2 << std::endl;
	std::cout << "��������:" << pDepthMarketData->BidVolume2 << std::endl;
	std::cout << "�����۶�:" << pDepthMarketData->AskPrice2 << std::endl;
	std::cout << "��������:" << pDepthMarketData->AskVolume2 << std::endl;
	std::cout << "�������:" << pDepthMarketData->BidPrice3 << std::endl;
	std::cout << "��������:" << pDepthMarketData->BidVolume3 << std::endl;
	std::cout << "��������:" << pDepthMarketData->AskPrice3 << std::endl;
	std::cout << "��������:" << pDepthMarketData->AskVolume3 << std::endl;
	std::cout << "�������:" << pDepthMarketData->BidPrice4 << std::endl;
	std::cout << "��������:" << pDepthMarketData->BidVolume4 << std::endl;
	std::cout << "��������:" << pDepthMarketData->AskPrice4 << std::endl;
	std::cout << "��������:" << pDepthMarketData->AskVolume4 << std::endl;
	std::cout << "�������:" << pDepthMarketData->BidPrice5 << std::endl;
	std::cout << "��������:" << pDepthMarketData->BidVolume5 << std::endl;
	std::cout << "��������:" << pDepthMarketData->AskPrice5 << std::endl;
	std::cout << "��������:" << pDepthMarketData->AskVolume5 << std::endl;
	std::cout << "================================================================" << std::endl;
	**/
	//beginQuery();
}

void CTraderHandler::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "��ѯ�����˻���Ӧ......" << std::endl;
	if (pRspInfo != nullptr) {
		if (pRspInfo != nullptr) {
			std::cout << "����ID:" << pRspInfo->ErrorID << std::endl;
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

bool CTraderHandler::isSlipOrder(CThostFtdcOrderField* pOrder) {
	return auctionOverFlag && pOrder->RequestID > auctionLastReqId;
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
		if (ongoingIns.find(insId) == ongoingIns.end()) {
			if (toInsertIns.find(insId) != toInsertIns.end()) {
				toInsertIns.erase(insId);
			}
			else {
				cout << insId << " is not existed in toInsertIns, ongoingIns when OnRtnOrder is called and OrderSubmitStatus is THOST_FTDC_OSS_InsertSubmitted " << endl;
			}
			cout << insId << " �����ɹ�" << endl;
			ongoingIns.insert(pair<string, CThostFtdcOrderField*>(insId, pOrder));
		}
		if (0 == toInsertIns.size() && !auctionOverFlag) {
			cout << "���Ͼ��۽׶α������" << endl;
			auctionOverFlag = true;
			// ���Ͼ���֮����Ҫ�ȴ�һ��ʱ��
			this_thread::sleep_for(chrono::milliseconds(1000));
			cout << "��ʼɨ���Լ״̬" << endl;
			scanOngoingOrderStatus();
			slipStartFlag = true;
		}
	}

	// �����ɹ�
	// �����Զ�������ص����֣�pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertRejected, pOrder->OrderStatus == THOST_FTDC_OST_Canceled
	// �жϷ�������ȷ
	else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled) {
		// ��� pOrder->InstrumentID ����cancelledIns�����У���Ҫ�������������Ƴ���Ȼ������
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
			cout << insId << " �ѳ���" << endl;
			cancelledIns.insert(pair<string, int>(insId, 1));
		}
		if (slipStartFlag) {
			string exchangeId = getExchangeId(insId);

			auto infoIter = instrumentInfoMap.find(insId);
			if (infoIter == instrumentInfoMap.end()) {
				// ���instrumentId����instrumentInfoMap��˵����ѯ����ʧ��
				cout << insId << " ��instrumentInfoMap �в�����" << endl;
			}
			InstrumentOrderInfo orderInfo = instrumentOrderMap.find(insId)->second;
			CThostFtdcDepthMarketDataField* lastestInfo = instrumentInfoMap.find(insId)->second.getInfo();
			// �����price����
			CThostFtdcInputOrderField order = composeSlipInputOrder(insId, exchangeId, orderInfo.buyOrSell(), orderInfo.getVol(), lastestInfo->OpenPrice, requestIndex);
			int result = 0;
			int retry = 3;
			while (retry-- > 0)
			{
				cout << insId << " �����µ�һ��" << endl;
				result = pUserTraderApi->ReqOrderInsert(&order, ++requestIndex);
				if (0 == result) {
					toInsertIns.insert(pair<string, int>(insId, 1));
					break;
				}
				else {
					if (-1 == result) {
						cout << "�����µ�����������ʧ��" << endl;
					}
					//-1,-2,-3 �����������һ��֮������
					this_thread::sleep_for(chrono::milliseconds(1000));
				}
			}
			if (result < 0) {
				cout << insId << " �����µ�ʧ��" << endl;
				toInsertIns.insert(pair<string, int>(insId, 1));
			}
			this_thread::sleep_for(chrono::milliseconds(200));
			cancelledIns.erase(insId);
		}
	}
	// ��Լ���ɽ�
	// ��������ͣ�δ��֤
	else if(pOrder->OrderStatus == THOST_FTDC_OST_AllTraded){
		// ��� pOrder->InstrumentID ����bingoIns������
		if (bingoIns.find(insId) == bingoIns.end()) {
			// ���������toInsertIns������
			if (toInsertIns.find(insId) != toInsertIns.end()) {
				toInsertIns.erase(toInsertIns.find(insId));
			}// ���������ongoingIns������
			else if (ongoingIns.find(insId) != ongoingIns.end()) {
				ongoingIns.erase(ongoingIns.find(insId));
			}
			else {
				cout << insId << " is not existed in toInsertIns, ongoingIns, bingoIns when OnRtnOrder is called and OrderStatus is THOST_FTDC_OST_AllTraded " << endl;
			}
			bingoIns.insert(pair<string, int>(insId, 1));
			cout << insId << " �ѳɽ�" << endl;
		}
	}

	return;

	/*
	if (isSlipOrder(pOrder)) {
		std::cout << "���㱨���ɹ�" << std::endl;
		string instrument = pOrder->InstrumentID;
		auto it = bingoSlipInstruments.find(instrument);
		if (it == bingoSlipInstruments.end()) {
			// ���ظ����
			bingoSlipInstruments.insert(pair<string, int>(instrument, 1));
		}
	}
	else {
		std::cout << "���Ͼ��۱����ɹ�" << std::endl;
	}
	// ֻ��Ҫɾ������
	ongoingInstruments.erase(pOrder->RequestID);
	
	std::cout << "��Լ���룺" << pOrder->InstrumentID << endl;
	std::cout << "��������" << pOrder->Direction << endl;
	std::cout << "�۸�" << pOrder->LimitPrice << endl;
	std::cout << "������" << pOrder->VolumeTotalOriginal << endl;

	if (0 == ongoingInstruments.size()) {
		auctionLastReqId = pOrder->RequestID;
		cout << "==============���Ͼ��۱������==================" << endl;
		cout << "==============���㱨����ʼ==================" << endl;
		if (!auctionOverFlag) {
			callSlippage();
		}
		auctionOverFlag = true;
	}
	*/
	
	/*
	std::cout << "���͹�˾���룺" << pOrder->BrokerID << endl;
	std::cout << "Ͷ���ߴ��룺" << pOrder->InvestorID << endl;
	
	std::cout << "�������ã�" << pOrder->OrderRef << endl;
	std::cout << "�û����룺" << pOrder->UserID << endl;
	std::cout << "�����۸�������" << pOrder->OrderPriceType << endl;
	
	std::cout << "��Ͽ�ƽ��־��" << pOrder->CombOffsetFlag << endl;
	std::cout << "���Ͷ���ױ���־��" << pOrder->CombHedgeFlag << endl;
	
	
	std::cout << "��Ч�����ͣ�" << pOrder->TimeCondition << endl;
	std::cout << "GTD���ڣ�" << pOrder->GTDDate << endl;
	std::cout << "�ɽ������ͣ�" << pOrder->VolumeCondition << endl;
	std::cout << "��С�ɽ�����" << pOrder->MinVolume << endl;
	std::cout << "����������" << pOrder->ContingentCondition << endl;
	std::cout << "ֹ��ۣ�" << pOrder->StopPrice << endl;
	std::cout << "ǿƽԭ��" << pOrder->ForceCloseReason << endl;
	std::cout << "�Զ������־��" << pOrder->IsAutoSuspend << endl;
	std::cout << "ҵ��Ԫ��" << pOrder->BusinessUnit << endl;
	std::cout << "�����ţ�" << pOrder->RequestID << endl;
	std::cout << "���ر�����ţ�" << pOrder->OrderLocalID << endl;
	std::cout << "���������룺" << pOrder->ExchangeID << endl;
	std::cout << "��Ա���룺" << pOrder->ParticipantID << endl;
	std::cout << "�ͻ����룺" << pOrder->ClientID << endl;
	std::cout << "��Լ�ڽ������Ĵ��룺" << pOrder->ExchangeInstID << endl;
	std::cout << "����������Ա���룺" << pOrder->TraderID << endl;
	std::cout << "��װ��ţ�" << pOrder->InstallID << endl;
	std::cout << "�����ύ״̬��" << pOrder->OrderSubmitStatus << endl;
	std::cout << "������ʾ��ţ�" << pOrder->NotifySequence << endl;
	std::cout << "�����գ�" << pOrder->TradingDay << endl;
	std::cout << "�����ţ�" << pOrder->SettlementID << endl;
	std::cout << "������ţ�" << pOrder->OrderSysID << endl;
	std::cout << "������Դ��" << pOrder->OrderSource << endl;
	std::cout << "����״̬��" << pOrder->OrderStatus << endl;
	std::cout << "�������ͣ�" << pOrder->OrderType << endl;
	std::cout << "��ɽ�������" << pOrder->VolumeTraded << endl;
	std::cout << "ʣ��������" << pOrder->VolumeTotal << endl;
	std::cout << "�������ڣ�" << pOrder->InsertDate << endl;
	std::cout << "ί��ʱ�䣺" << pOrder->InsertTime << endl;
	std::cout << "����ʱ�䣺" << pOrder->ActiveTime << endl;
	std::cout << "����ʱ�䣺" << pOrder->SuspendTime << endl;
	std::cout << "����޸�ʱ�䣺" << pOrder->UpdateTime << endl;
	std::cout << "����ʱ�䣺" << pOrder->CancelTime << endl;
	std::cout << "����޸Ľ���������Ա���룺" << pOrder->ActiveTraderID << endl;
	std::cout << "�����Ա��ţ�" << pOrder->ClearingPartID << endl;
	std::cout << "��ţ�" << pOrder->SequenceNo << endl;
	std::cout << "ǰ�ñ�ţ�" << pOrder->FrontID << endl;
	std::cout << "�Ự��ţ�" << pOrder->SessionID << endl;
	std::cout << "�û��˲�Ʒ��Ϣ��" << pOrder->UserProductInfo << endl;
	std::cout << "״̬��Ϣ��" << pOrder->StatusMsg << endl;
	std::cout << "�û�ǿ����־��" << pOrder->UserForceClose << endl;
	std::cout << "�����û����룺" << pOrder->ActiveUserID << endl;
	std::cout << "���͹�˾������ţ�" << pOrder->BrokerOrderSeq << endl;
	std::cout << "��ر�����" << pOrder->RelativeOrderSysID << endl;
	std::cout << "֣�����ɽ�������" << pOrder->ZCETotalTradedVolume << endl;
	std::cout << "��������־��" << pOrder->IsSwapOrder << endl;
	std::cout << "Ӫҵ����ţ�" << pOrder->BranchID << endl;
	std::cout << "Ͷ�ʵ�Ԫ���룺" << pOrder->InvestUnitID << endl;
	std::cout << "�ʽ��˺ţ�" << pOrder->AccountID << endl;
	std::cout << "���ִ��룺" << pOrder->CurrencyID << endl;
	std::cout << "IP��ַ��" << pOrder->IPAddress << endl;
	std::cout << "Mac��ַ��" << pOrder->MacAddress << endl;
	*/
	std::cout << "================================================================" << std::endl;
}
// �ɽ��ر����������ɽ�ʱ�����й�ϵͳ��֪ͨ�ͻ��ˣ��÷����ᱻ����
void CTraderHandler::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
	cout << "================================================================" << std::endl;
	cout << "OnRtnTrade is called" << std::endl;
	string insId = pTrade->InstrumentID;
	// ��� pOrder->InstrumentID ����bingoIns������
	if (bingoIns.find(insId) == bingoIns.end()) {
		// ���������toInsertIns������
		if (toInsertIns.find(insId) != toInsertIns.end()) {
			toInsertIns.erase(toInsertIns.find(insId));
		}// ���������ongoingIns������
		else if (ongoingIns.find(insId) != ongoingIns.end()) {
			ongoingIns.erase(ongoingIns.find(insId));
		}
		else {
			cout << insId << " is not existed in toInsertIns, ongoingIns, bingoIns when OnRtnTrade is called " << endl;
		}
		bingoIns.insert(pair<string, int>(insId, 1));
		cout << insId << " �ѳɽ�" << endl;
	}
	/*
	std::cout << "���͹�˾���룺" << pTrade->BrokerID << endl;
	std::cout << "Ͷ���ߴ��룺" << pTrade->InvestorID << endl;
	std::cout << "��Լ���룺" << pTrade->InstrumentID << endl;
	std::cout << "�������ã�" << pTrade->OrderRef << endl;
	std::cout << "�û����룺" << pTrade->UserID << endl;
	std::cout << "���������룺" << pTrade->ExchangeID << endl;
	std::cout << "�ɽ���ţ�" << pTrade->TradeID << endl;
	std::cout << "��������" << pTrade->Direction << endl;
	std::cout << "������ţ�" << pTrade->OrderSysID << endl;
	std::cout << "��Ա���룺" << pTrade->ParticipantID << endl;
	std::cout << "�ͻ����룺" << pTrade->ClientID << endl;
	std::cout << "���׽�ɫ��" << pTrade->TradingRole << endl;
	std::cout << "��Լ�ڽ������Ĵ��룺" << pTrade->ExchangeInstID << endl;
	std::cout << "��ƽ��־��" << pTrade->OffsetFlag << endl;
	std::cout << "Ͷ���ױ���־��" << pTrade->HedgeFlag << endl;
	std::cout << "�۸�" << pTrade->Price << endl;
	std::cout << "������" << pTrade->Volume << endl;
	std::cout << "�ɽ�ʱ�ڣ�" << pTrade->TradeDate << endl;
	std::cout << "�ɽ�ʱ�䣺" << pTrade->TradeTime << endl;
	std::cout << "�ɽ����ͣ�" << pTrade->TradeType << endl;
	std::cout << "�ɽ�����Դ��" << pTrade->PriceSource << endl;
	std::cout << "����������Ա���룺" << pTrade->TraderID << endl;
	std::cout << "���ر�����ţ�" << pTrade->OrderLocalID << endl;
	std::cout << "�����Ա��ţ�" << pTrade->ClearingPartID << endl;
	std::cout << "ҵ��Ԫ��" << pTrade->BusinessUnit << endl;
	std::cout << "��ţ�" << pTrade->SequenceNo << endl;
	std::cout << "�����գ�" << pTrade->TradingDay << endl;
	std::cout << "�����ţ�" << pTrade->SettlementID << endl;
	std::cout << "���͹�˾������ţ�" << pTrade->BrokerOrderSeq << endl;
	std::cout << "�ɽ���Դ��" << pTrade->TradeSource << endl;
	std::cout << "Ͷ�ʵ�Ԫ���룺" << pTrade->InvestUnitID << endl;
	*/
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
		// ֱ�Ӵ�toInsertInsɾ��
		auto it = toInsertIns.find(pInputOrder->InstrumentID);
		if (it != toInsertIns.end()) {
			toInsertIns.erase(it);
		}
	}
	else if(pInputOrder != nullptr){
		// ���ǽ����δ��
		cout << "OnRspOrderInsert says insert " << pInputOrder->InstrumentID << " order success" << endl;
	}
	
	std::cout << "================================================================" << std::endl;
}

void CTraderHandler::scanOngoingOrderStatus() {
	for (map<string, CThostFtdcOrderField*>::iterator it = ongoingIns.begin(); it != ongoingIns.end(); it++) {
		CThostFtdcQryOrderField field = {0};
		// �ĵ�ע����˵������д BrokerID ������ȫ���б������� ����ʲô��˼
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
					cout << "ReqQryOrder encountered ��������ʧ��" << endl;
				}
				this_thread::sleep_for(chrono::milliseconds(1000));
			}
		}
		// �˴�����result=-2����ʾδ�������󳬹������
		if (result < 0) {
			cout << it->first << " ����״̬��ѯʧ��, result = "<<result << endl;
		}
		// ��ͣ0.2s ������QPS
		this_thread::sleep_for(chrono::milliseconds(200));
	}
}

void CTraderHandler::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	cout << "OnRspQryOrder is called" << endl;
	if (pRspInfo != nullptr) {
		std::cout << "������룺" << pRspInfo->ErrorID << endl;
		std::cout << "������Ϣ��" << pRspInfo->ErrorMsg << endl;
	}
	if (pOrder != nullptr) {
		string insId = pOrder->InstrumentID;
		// ongoingIns �к�Լ״̬Ǩ��
		if (ongoingIns.find(insId) != ongoingIns.end()) {
			
			// �ѳɽ�
			if(THOST_FTDC_OST_AllTraded == pOrder->OrderStatus) {
				ongoingIns.erase(ongoingIns.find(insId));
				bingoIns.insert(pair<string, int>(insId, 1));
			}// �ѳ���
			else if (THOST_FTDC_OST_Canceled == pOrder->OrderStatus) {
				ongoingIns.erase(ongoingIns.find(insId));
				cancelledIns.insert(pair<string, int>(insId, 1));
			}
			// �����ֶ�����
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
							cout << "call ReqOrderAction ��������ʧ��" << endl;
						}
						this_thread::sleep_for(chrono::milliseconds(1000));
					}
				}
				// �������Σ�����ָ���ʧ��
				if (result < 0) {
					cout << "�������Σ����� "<<insId<<" ָ���ʧ��" << endl;
					// ����ʧ�ܣ������¼�����ټ�������
					ongoingIns.erase(ongoingIns.find(insId));
				}
			}

		}
	}
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
		// ֱ�Ӵ�toInsertInsɾ��
		auto it = toInsertIns.find(pInputOrder->InstrumentID);
		if (it != toInsertIns.end()) {
			toInsertIns.erase(it);
		}
	}
	std::cout << "================================================================" << std::endl;
}

void CTraderHandler::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "=============�����ļ� doc1.log=============" << std::endl;
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
	int result = pUserTraderApi->ReqQryDepthMarketData(&instrumentField, ++requestIndex);
	if (0 == result) {
		// ���Ͳ�ѯ�ɹ�
		onceQueryMarker.insert(pair<int, string>(requestIndex, instrumentId));
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
