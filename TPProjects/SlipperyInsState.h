#pragma once
class SlipperyInsState {
	std::ostringstream stream;
public:
	enum STATE_ENUM
	{
		NO_INFO= 0 , // δ��ѯ����ԼInfo
		STARTED= 1 , // ��Լ����
		ORDERED= 2 , // ��Լ�����ɹ�
		ORDER_FAILED= 3 , // ��Լ����ʧ��
		RETRIVED= 4 , // ��Լ����
		DONE= 5 , // ��Լ�ɽ�
		UNTRADED= 6  // ��Լ�����ɹ���δ�ɽ�
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