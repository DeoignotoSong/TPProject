#pragma once
#include <string>
using namespace std;
// ���ڼ�¼ÿ����Լ���µ�����
class InstrumentOrderInfo {
private:
	bool buyIn;
	int volumn;
public:
	InstrumentOrderInfo(string rawVol);
	~InstrumentOrderInfo();
	bool buyOrSell();
	int getVol();
};