#pragma once
class SlipperyInsState {
public:
	enum STATE_ENUM
	{
		NO_INFO, // δ��ѯ����ԼInfo
		STARTED, // ��Լ����
		ORDERED, // ��Լ�����ɹ�
		ORDER_FAILED, // ��Լ����ʧ��
		RETRIVED, // ��Լ����
		DONE, // ��Լ�ɽ�
		UNTRADED // ��Լ�����ɹ���δ�ɽ�
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