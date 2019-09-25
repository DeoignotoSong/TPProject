#pragma  once
# include "InstrumentOrderInfo.h"

InstrumentOrderInfo::InstrumentOrderInfo(string rawVol) {
	int tmp = atoi(rawVol.c_str());
	if (tmp < 0) {
		this->buyIn = false;
		this->volumn = 0 - tmp;
	}
	else {
		this->buyIn = true;
		this->volumn = tmp;
	}
}
InstrumentOrderInfo::~InstrumentOrderInfo() {};
bool InstrumentOrderInfo::buyOrSell() {
	return this->buyIn;
};
int InstrumentOrderInfo::getVol() {
	return this->volumn;
};