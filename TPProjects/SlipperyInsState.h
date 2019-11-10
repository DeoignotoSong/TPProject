#pragma once
class SlipperyInsState {
public:
	enum STATE_ENUM
	{
		NO_INFO, // 未查询到合约Info
		STARTED, // 合约报单
		ORDERED, // 合约报单成功
		ORDER_FAILED, // 合约报单失败
		RETRIVED, // 合约撤单
		DONE, // 合约成交
		UNTRADED // 合约报单成功后未成交
	};
private:
	STATE_ENUM state;
	int latestReqId;
	int latestRespId;
public:
	SlipperyInsState(STATE_ENUM state, int reqId);
	~SlipperyInsState();
	void updateOnReq(STATE_ENUM state, int reqId);
	void updateOnResp(STATE_ENUM state, int respId);
	int getLatestReqId();
	STATE_ENUM getState();
};