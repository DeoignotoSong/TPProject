#pragma  once
#include "SlipperyInsState.h"

SlipperyInsState::SlipperyInsState(STATE_ENUM state, int reqId) {
	this->state = state;
	this->latestReqId = reqId;
}
SlipperyInsState::~SlipperyInsState() {}
void SlipperyInsState::updateOnReq(STATE_ENUM state, int reqId) {
	this->state = state;
	this->latestReqId = reqId;
}
void SlipperyInsState::updateOnResp(STATE_ENUM state, int respId) {
	this->state = state;
	this->latestRespId = respId;
}
int SlipperyInsState::getLatestReqId() {
	return this->latestReqId;
}
SlipperyInsState::STATE_ENUM SlipperyInsState::getState() {
	return this->state;
}
