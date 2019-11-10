#pragma  once
#include "AuctionInsState.h"

AuctionInsState::AuctionInsState(STATE_ENUM state, int reqId) {
	this->state = state;
	this->latestReqId = reqId;
}
AuctionInsState::~AuctionInsState() {}
void AuctionInsState::updateOnReq(STATE_ENUM state, int reqId) {
	this->state = state;
	this->latestReqId = reqId;
}
void AuctionInsState::updateOnResp(STATE_ENUM state, int respId) {
	this->state = state;
	this->latestRespId = respId;
}
int AuctionInsState::getLatestReqId() {
	return this->latestReqId;
}
AuctionInsState::STATE_ENUM AuctionInsState::getState() {
	return this->state;
}