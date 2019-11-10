#pragma  once
#include "Utils.h"
#include <iostream>
using namespace std;

vector<string> split(const string& s, const string& c)
{
	vector<string> v;
	string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));

	return v;
}

chrono::system_clock::time_point getNextStartTime(int hour, int minute, int second) {
	time_t now = time(NULL);
	tm now_tm;
	localtime_s(&now_tm, &now);
	struct tm today_tm = { second, minute, hour, now_tm.tm_mday, now_tm.tm_mon, now_tm.tm_year };
	chrono::system_clock::time_point today_tp = chrono::system_clock::from_time_t(mktime(&today_tm));
	chrono::system_clock::time_point output_tp;
	// earlier than 08:30 AM
	if (hour >= now_tm.tm_hour && minute >= now_tm.tm_min) {
		output_tp = today_tp;
	}// later than 08:30 AM
	else {
		output_tp = today_tp + chrono::hours(24);
	}
	return output_tp;
}



// 暂时只考虑白天场，因为不清楚夜场的开始时间，不知如何分界
chrono::system_clock::time_point getAuctionStartTime() {
	string startTime = getConfig("config", "AucionStartTime");
	vector<string> time = split(startTime, ":");
	int hour = stoi(time.at(0));
	int minute = stoi(time.at(1));
	return getNextStartTime(hour, minute, 0);
}

chrono::system_clock::time_point getSlipperyPhaseStartTime(string phaseName) {
	string phasePeriod = getConfig("config", phaseName);
	vector<string> periodStr = split(phasePeriod, "-");
	// 处理21:00:00
	string startStr = periodStr.at(0);
	vector<string> startArr = split(startStr, ":");
	int sHour = stoi(startArr.at(0));
	int sMinute = stoi(startArr.at(1));
	int sSecond = stoi(startArr.at(2));
	return getNextStartTime(sHour, sMinute, sSecond);
}

chrono::system_clock::time_point getSlipPhaseAStartTime() {
	return getSlipperyPhaseStartTime("SlipperyPhase1");
}

chrono::system_clock::time_point getSlipPhaseBStartTime() {
	return getSlipperyPhaseStartTime("SlipperyPhase2");
}

chrono::system_clock::time_point getSlipPhaseCStartTime() {
	return getSlipperyPhaseStartTime("SlipperyPhase3");
}

// phasePeriod: 21:00:00-21:00:30
bool inThisPeriod(string phasePeriod, tm time) {
	vector<string> periodStr = split(phasePeriod,"-");
	// 处理21:00:00
	string startStr = periodStr.at(0);
	vector<string> startArr = split(startStr, ":");
	int sHour = stoi(startArr.at(0));
	int sMinute = stoi(startArr.at(1));
	int sSecond = stoi(startArr.at(2));
	// 处理21:00:30
	string endStr = periodStr.at(1);
	vector<string> endArr = split(endStr, ":");
	int eHour = stoi(endArr.at(0));
	int eMinute = stoi(endArr.at(1));
	int eSecond = stoi(endArr.at(2));
	//判断是否在其中
	if (time.tm_hour >= sHour && time.tm_hour <= eHour
		&& time.tm_min >= sMinute && time.tm_min <= eMinute
		&& time.tm_sec >= sSecond && time.tm_sec <= eSecond) {
		return true;
	}
	else {
		return false;
	}
}
