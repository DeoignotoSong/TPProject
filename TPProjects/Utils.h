#pragma once
#include <string>
#include <vector>
#include <chrono>
#include "getconfig.h"
using namespace std;

vector<string> split(const string& s, const string& seperator);
chrono::system_clock::time_point getNextStartTime(int hour, int minute, int sec);
chrono::system_clock::time_point getAuctionStartTime();
chrono::system_clock::time_point getSlipperyPhaseStartTime(string phaseName);
chrono::system_clock::time_point getSlipPhaseAStartTime();
chrono::system_clock::time_point getSlipPhaseBStartTime();
chrono::system_clock::time_point getSlipPhaseCStartTime();
bool inThisPeriod(string phasePeriod, tm time);
void clearStream(ostringstream& stream);
string* getLogPostfix();
bool isEndNow();