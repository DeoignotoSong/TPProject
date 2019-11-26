#pragma once
class SlipperyInsState {
	std::ostringstream stream;
public:
	enum STATE_ENUM
	{
		NO_INFO= 0 , // 未查询到合约Info
		STARTED= 1 , // 合约报单
		ORDERED= 2 , // 合约报单成功
		ORDER_FAILED= 3 , // 合约报单失败
		RETRIVED= 4 , // 合约撤单
		DONE= 5 , // 合约成交
		UNTRADED= 6  // 合约报单成功后未成交
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