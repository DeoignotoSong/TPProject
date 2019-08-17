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

	std::cout << "客户端认证 = "  << b << endl;
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
	std::cout << "查询行情响应......" << std::endl;
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

	int operation = 0;

	std::cout << "请输入选择的操作（\n0.查询账户；\n1.查询持仓；\n2.下单；）：";
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
}

void CTraderHandler::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
	std::cout << "报单成功" << std::endl;
	std::cout << "================================================================" << std::endl;
	std::cout << "经纪公司代码：" << pOrder->BrokerID << endl;
	std::cout << "投资者代码：" << pOrder->InvestorID << endl;
	std::cout << "合约代码：" << pOrder->InstrumentID << endl;
	std::cout << "报单引用：" << pOrder->OrderRef << endl;
	std::cout << "用户代码：" << pOrder->UserID << endl;
	std::cout << "报单价格条件：" << pOrder->OrderPriceType << endl;
	std::cout << "买卖方向：" << pOrder->Direction << endl;
	std::cout << "组合开平标志：" << pOrder->CombOffsetFlag << endl;
	std::cout << "组合投机套保标志：" << pOrder->CombHedgeFlag << endl;
	std::cout << "价格：" << pOrder->LimitPrice << endl;
	std::cout << "数量：" << pOrder->VolumeTotalOriginal << endl;
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
	std::cout << "================================================================" << std::endl;
}

void CTraderHandler::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
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

void CTraderHandler::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "报单错误" << std::endl;
	std::cout << "================================================================" << std::endl;
	if (pRspInfo != nullptr) {
		std::cout << "错误代码" << pRspInfo->ErrorID << endl;
		std::cout << "错误信息" << pRspInfo->ErrorMsg << endl;
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
	std::cout << "投资者结算结果确认" << std::endl;

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
	std::cout << "查询持仓成功" << endl;

	if (pRspInfo != nullptr) {
		std::cout << "错误代码" << pRspInfo->ErrorID << endl;
		std::cout << "错误信息" << pRspInfo->ErrorMsg << endl;
	}
	std::cout << "================================================================" << std::endl;
	if (pInvestorPosition != nullptr) {

		std::cout << "合约代码" << pInvestorPosition->InstrumentID << std::endl;
		std::cout << "经纪公司代码" << pInvestorPosition->BrokerID << std::endl;
		std::cout << "投资者代码" << pInvestorPosition->InvestorID << std::endl;
		std::cout << "持仓多空方向" << pInvestorPosition->PosiDirection << std::endl;
		std::cout << "投机套保标志" << pInvestorPosition->HedgeFlag << std::endl;
		std::cout << "持仓日期" << pInvestorPosition->PositionDate << std::endl;
		std::cout << "上日持仓" << pInvestorPosition->YdPosition << std::endl;
		std::cout << "今日持仓" << pInvestorPosition->Position << std::endl;
		std::cout << "多头冻结" << pInvestorPosition->LongFrozen << std::endl;
		std::cout << "空头冻结" << pInvestorPosition->ShortFrozen << std::endl;
		std::cout << "开仓冻结金额" << pInvestorPosition->LongFrozenAmount << std::endl;
		std::cout << "开仓冻结金额" << pInvestorPosition->ShortFrozenAmount << std::endl;
		std::cout << "开仓量" << pInvestorPosition->OpenVolume << std::endl;
		std::cout << "平仓量" << pInvestorPosition->CloseVolume << std::endl;
		std::cout << "开仓金额" << pInvestorPosition->OpenAmount << std::endl;
		std::cout << "平仓金额" << pInvestorPosition->CloseAmount << std::endl;
		std::cout << "持仓成本" << pInvestorPosition->PositionCost << std::endl;
		std::cout << "上次占用的保证金" << pInvestorPosition->PreMargin << std::endl;
		std::cout << "占用的保证金" << pInvestorPosition->UseMargin << std::endl;
		std::cout << "冻结的保证金" << pInvestorPosition->FrozenMargin << std::endl;
		std::cout << "冻结的资金" << pInvestorPosition->FrozenCash << std::endl;
		std::cout << "冻结的手续费" << pInvestorPosition->FrozenCommission << std::endl;
		std::cout << "资金差额" << pInvestorPosition->CashIn << std::endl;
		std::cout << "手续费" << pInvestorPosition->Commission << std::endl;
		std::cout << "平仓盈亏" << pInvestorPosition->CloseProfit << std::endl;
		std::cout << "持仓盈亏" << pInvestorPosition->PositionProfit << std::endl;
		std::cout << "上次结算价" << pInvestorPosition->PreSettlementPrice << std::endl;
		std::cout << "本次结算价" << pInvestorPosition->SettlementPrice << std::endl;
		std::cout << "交易日" << pInvestorPosition->TradingDay << std::endl;
		std::cout << "结算编号" << pInvestorPosition->SettlementID << std::endl;
		std::cout << "开仓成本" << pInvestorPosition->OpenCost << std::endl;
		std::cout << "交易所保证金" << pInvestorPosition->ExchangeMargin << std::endl;
		std::cout << "组合成交形成的持仓" << pInvestorPosition->CombPosition << std::endl;
		std::cout << "组合多头冻结" << pInvestorPosition->CombLongFrozen << std::endl;
		std::cout << "组合空头冻结" << pInvestorPosition->CombShortFrozen << std::endl;
		std::cout << "逐日盯市平仓盈亏" << pInvestorPosition->CloseProfitByDate << std::endl;
		std::cout << "逐笔对冲平仓盈亏" << pInvestorPosition->CloseProfitByTrade << std::endl;
		std::cout << "今日持仓" << pInvestorPosition->TodayPosition << std::endl;
		std::cout << "保证金率" << pInvestorPosition->MarginRateByMoney << std::endl;
		std::cout << "保证金率(按手数)" << pInvestorPosition->MarginRateByVolume << std::endl;
		std::cout << "执行冻结" << pInvestorPosition->StrikeFrozen << std::endl;
		std::cout << "执行冻结金额" << pInvestorPosition->StrikeFrozenAmount << std::endl;
		std::cout << "放弃执行冻结" << pInvestorPosition->AbandonFrozen << std::endl;
		std::cout << "交易所代码" << pInvestorPosition->ExchangeID << std::endl;
		std::cout << "执行冻结的昨仓" << pInvestorPosition->YdStrikeFrozen << std::endl;
		std::cout << "投资单元代码" << pInvestorPosition->InvestUnitID << std::endl;
	}
	std::cout << "================================================================" << std::endl;
}
