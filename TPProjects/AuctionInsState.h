#pragma once
class AuctionInsState {
public:
	enum STATE_ENUM
	{
		NO_INFO, // δ��ѯ����ԼInfo
		STARTED, // ��Լ����
		ORDERED, // ��Լ�����ɹ�
		ORDER_FAILED, // ��Լ����ʧ��
		DONE, // ��Լ�ɽ�
		CANCELED // ��Լȡ��
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