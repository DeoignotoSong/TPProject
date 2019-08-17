#include "CTraderHandler.h"

CTraderHandler::CTraderHandler(CThostFtdcTraderApi* pUserTraderApi) {
	this->pUserTraderApi = pUserTraderApi;

	requestIndex = 0;
}

CTraderHandler::~CTraderHandler() {
	pUserTraderApi->Release();
}

void CTraderHandler::ReqAuthenticate()
{
	CThostFtdcReqAuthenticateField authField = { 0 };

	strcpy_s(authField.AuthCode, "0000000000000000");
	strcpy_s(authField.AppID, "simnow_client_test");
	strcpy_s(authField.BrokerID, "9999");
	strcpy_s(authField.UserID, "125013");

	int b = pUserTraderApi->ReqAuthenticate(&authField, requestIndex++);

	std::cout << "�ͻ�����֤ = "  << b << endl;
}

void CTraderHandler::OnFrontConnected()
{
	std::cout << "Connect Success......" << endl;

	this->ReqAuthenticate();
}

void CTraderHandler::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField,
	CThostFtdcRspInfoField* pRspInfo, 
	int nRequestID, 
	bool bIsLast)
{
	std::cout << "Authenticate success......" << endl;

	CThostFtdcReqUserLoginField userField = { 0 };

	strcpy_s(userField.BrokerID, "9999");
	strcpy_s(userField.UserID, "125013");
	strcpy_s(userField.Password, "Song1227");

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
	strcpy_s(confirmField.BrokerID, "9999");
	strcpy_s(confirmField.InvestorID, "125013");
	pUserTraderApi->ReqSettlementInfoConfirm(&confirmField, nRequestID++);
}

void CTraderHandler::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "��ѯ������Ӧ......" << std::endl;
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

	int operation = 0;

	std::cout << "������ѡ��Ĳ�����\n0.��ѯ�˻���\n1.��ѯ�ֲ֣�\n2.�µ�������";
	std::cin >> operation;

	int result = 0;
	CThostFtdcQryTradingAccountField tradingAccountField = { 0 };
	CThostFtdcQryInvestorPositionField investorPositionField = { 0 };
	CThostFtdcInputOrderField inputOrderField = { 0 };

	switch (operation)
	{
	case 0:
		strcpy_s(tradingAccountField.BrokerID, "9999");
		strcpy_s(tradingAccountField.InvestorID, "125013");
		strcpy_s(tradingAccountField.CurrencyID, "CNY");

		result = pUserTraderApi->ReqQryTradingAccount(&tradingAccountField, requestIndex++);

		break;
	case 1:
		strcpy_s(investorPositionField.BrokerID, "9999");
		strcpy_s(investorPositionField.InvestorID, "125013");

		result = pUserTraderApi->ReqQryInvestorPosition(&investorPositionField, requestIndex++);

		break;
	case 2:
		strcpy_s(inputOrderField.BrokerID, "9999");
		strcpy_s(inputOrderField.InvestorID, "125013");
		strcpy_s(inputOrderField.ExchangeID, "SHFE");
		strcpy_s(inputOrderField.InstrumentID, pDepthMarketData->InstrumentID);
		strcpy_s(inputOrderField.UserID, "125013");
		strcpy_s(inputOrderField.OrderRef, "");
		inputOrderField.Direction = THOST_FTDC_D_Buy;
		inputOrderField.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
		inputOrderField.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
		inputOrderField.VolumeTotalOriginal = 1;
		inputOrderField.VolumeCondition = THOST_FTDC_VC_AV;
		inputOrderField.MinVolume = 1;
		inputOrderField.ContingentCondition = THOST_FTDC_CC_Immediately;
		inputOrderField.StopPrice = 0;
		inputOrderField.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
		inputOrderField.IsAutoSuspend = 0;
		
		inputOrderField.LimitPrice = pDepthMarketData->BidPrice1;
		inputOrderField.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
		inputOrderField.TimeCondition = THOST_FTDC_TC_GFD;

		result = pUserTraderApi->ReqOrderInsert(&inputOrderField, requestIndex++);

		break;
	}
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
}

void CTraderHandler::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
	std::cout << "�����ɹ�" << std::endl;
	std::cout << "================================================================" << std::endl;
	std::cout << "���͹�˾���룺" << pOrder->BrokerID << endl;
	std::cout << "Ͷ���ߴ��룺" << pOrder->InvestorID << endl;
	std::cout << "��Լ���룺" << pOrder->InstrumentID << endl;
	std::cout << "�������ã�" << pOrder->OrderRef << endl;
	std::cout << "�û����룺" << pOrder->UserID << endl;
	std::cout << "�����۸�������" << pOrder->OrderPriceType << endl;
	std::cout << "��������" << pOrder->Direction << endl;
	std::cout << "��Ͽ�ƽ��־��" << pOrder->CombOffsetFlag << endl;
	std::cout << "���Ͷ���ױ���־��" << pOrder->CombHedgeFlag << endl;
	std::cout << "�۸�" << pOrder->LimitPrice << endl;
	std::cout << "������" << pOrder->VolumeTotalOriginal << endl;
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
	std::cout << "================================================================" << std::endl;
}

void CTraderHandler::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
	std::cout << "���׳ɹ�" << std::endl;
	std::cout << "================================================================" << std::endl;
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
	std::cout << "================================================================" << std::endl;
}

void CTraderHandler::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "��������" << std::endl;
	std::cout << "================================================================" << std::endl;
	if (pRspInfo != nullptr) {
		std::cout << "�������" << pRspInfo->ErrorID << endl;
		std::cout << "������Ϣ" << pRspInfo->ErrorMsg << endl;
	}
	std::cout << "================================================================" << std::endl;
}

void CTraderHandler::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo)
{
	std::cout << "��������" << std::endl;
	std::cout << "================================================================" << std::endl;
	if (pRspInfo != nullptr) {
		std::cout << "�������" << pRspInfo->ErrorID << endl;
		std::cout << "������Ϣ" << pRspInfo->ErrorMsg << endl;
	}
	std::cout << "================================================================" << std::endl;
}

void CTraderHandler::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "Ͷ���߽�����ȷ��" << std::endl;

	CThostFtdcQryDepthMarketDataField instrumentField = { 0 };

	strcpy_s(instrumentField.InstrumentID, "ag1912");

	strcpy_s(instrumentField.ExchangeID, "SHFE");

	int result = pUserTraderApi->ReqQryDepthMarketData(&instrumentField, requestIndex++);
}

void CTraderHandler::OnRspQryInvestorPosition(
	CThostFtdcInvestorPositionField* pInvestorPosition,
	CThostFtdcRspInfoField* pRspInfo,
	int nRequestID,
	bool bIsLast
)
{
	std::cout << "��ѯ�ֲֳɹ�" << endl;

	if (pRspInfo != nullptr) {
		std::cout << "�������" << pRspInfo->ErrorID << endl;
		std::cout << "������Ϣ" << pRspInfo->ErrorMsg << endl;
	}
	std::cout << "================================================================" << std::endl;
	if (pInvestorPosition != nullptr) {

		std::cout << "��Լ����" << pInvestorPosition->InstrumentID << std::endl;
		std::cout << "���͹�˾����" << pInvestorPosition->BrokerID << std::endl;
		std::cout << "Ͷ���ߴ���" << pInvestorPosition->InvestorID << std::endl;
		std::cout << "�ֲֶ�շ���" << pInvestorPosition->PosiDirection << std::endl;
		std::cout << "Ͷ���ױ���־" << pInvestorPosition->HedgeFlag << std::endl;
		std::cout << "�ֲ�����" << pInvestorPosition->PositionDate << std::endl;
		std::cout << "���ճֲ�" << pInvestorPosition->YdPosition << std::endl;
		std::cout << "���ճֲ�" << pInvestorPosition->Position << std::endl;
		std::cout << "��ͷ����" << pInvestorPosition->LongFrozen << std::endl;
		std::cout << "��ͷ����" << pInvestorPosition->ShortFrozen << std::endl;
		std::cout << "���ֶ�����" << pInvestorPosition->LongFrozenAmount << std::endl;
		std::cout << "���ֶ�����" << pInvestorPosition->ShortFrozenAmount << std::endl;
		std::cout << "������" << pInvestorPosition->OpenVolume << std::endl;
		std::cout << "ƽ����" << pInvestorPosition->CloseVolume << std::endl;
		std::cout << "���ֽ��" << pInvestorPosition->OpenAmount << std::endl;
		std::cout << "ƽ�ֽ��" << pInvestorPosition->CloseAmount << std::endl;
		std::cout << "�ֲֳɱ�" << pInvestorPosition->PositionCost << std::endl;
		std::cout << "�ϴ�ռ�õı�֤��" << pInvestorPosition->PreMargin << std::endl;
		std::cout << "ռ�õı�֤��" << pInvestorPosition->UseMargin << std::endl;
		std::cout << "����ı�֤��" << pInvestorPosition->FrozenMargin << std::endl;
		std::cout << "������ʽ�" << pInvestorPosition->FrozenCash << std::endl;
		std::cout << "�����������" << pInvestorPosition->FrozenCommission << std::endl;
		std::cout << "�ʽ���" << pInvestorPosition->CashIn << std::endl;
		std::cout << "������" << pInvestorPosition->Commission << std::endl;
		std::cout << "ƽ��ӯ��" << pInvestorPosition->CloseProfit << std::endl;
		std::cout << "�ֲ�ӯ��" << pInvestorPosition->PositionProfit << std::endl;
		std::cout << "�ϴν����" << pInvestorPosition->PreSettlementPrice << std::endl;
		std::cout << "���ν����" << pInvestorPosition->SettlementPrice << std::endl;
		std::cout << "������" << pInvestorPosition->TradingDay << std::endl;
		std::cout << "������" << pInvestorPosition->SettlementID << std::endl;
		std::cout << "���ֳɱ�" << pInvestorPosition->OpenCost << std::endl;
		std::cout << "��������֤��" << pInvestorPosition->ExchangeMargin << std::endl;
		std::cout << "��ϳɽ��γɵĳֲ�" << pInvestorPosition->CombPosition << std::endl;
		std::cout << "��϶�ͷ����" << pInvestorPosition->CombLongFrozen << std::endl;
		std::cout << "��Ͽ�ͷ����" << pInvestorPosition->CombShortFrozen << std::endl;
		std::cout << "���ն���ƽ��ӯ��" << pInvestorPosition->CloseProfitByDate << std::endl;
		std::cout << "��ʶԳ�ƽ��ӯ��" << pInvestorPosition->CloseProfitByTrade << std::endl;
		std::cout << "���ճֲ�" << pInvestorPosition->TodayPosition << std::endl;
		std::cout << "��֤����" << pInvestorPosition->MarginRateByMoney << std::endl;
		std::cout << "��֤����(������)" << pInvestorPosition->MarginRateByVolume << std::endl;
		std::cout << "ִ�ж���" << pInvestorPosition->StrikeFrozen << std::endl;
		std::cout << "ִ�ж�����" << pInvestorPosition->StrikeFrozenAmount << std::endl;
		std::cout << "����ִ�ж���" << pInvestorPosition->AbandonFrozen << std::endl;
		std::cout << "����������" << pInvestorPosition->ExchangeID << std::endl;
		std::cout << "ִ�ж�������" << pInvestorPosition->YdStrikeFrozen << std::endl;
		std::cout << "Ͷ�ʵ�Ԫ����" << pInvestorPosition->InvestUnitID << std::endl;
	}
	std::cout << "================================================================" << std::endl;
}
