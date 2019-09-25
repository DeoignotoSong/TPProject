#pragma once
#include <string>
using namespace std;
// 用于记录每个合约的下单内容
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