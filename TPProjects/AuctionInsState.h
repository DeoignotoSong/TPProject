#pragma once
class AuctionInsState {
public:
	enum STATE_ENUM
	{
		NO_INFO, // 未查询到合约Info
		STARTED, // 合约报单
		ORDERED, // 合约报单成功
		ORDER_FAILED, // 合约报单失败
		DONE, // 合约成交
		CANCELED // 合约取消
	};

private:
	STATE_ENUM state;
	int latestReqId;
	int latestRespId;

public:
	AuctionInsState(STATE_ENUM state, int reqId);
	~AuctionInsState();
	void updateOnReq(STATE_ENUM state, int reqId);
	void updateOnResp(STATE_ENUM state, int respId);
	int getLatestReqId();
	STATE_ENUM getState();
};