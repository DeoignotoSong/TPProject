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

chrono::system_clock::time_point getNextStartTime() {
	time_t now = time(NULL);
	struct tm now_tm;
	localtime_s(&now_tm, &now);
	struct tm today_tm = { 0, 30, 8, now_tm.tm_mday, now_tm.tm_mon, now_tm.tm_year };
	chrono::system_clock::time_point today_tp = chrono::system_clock::from_time_t(mktime(&today_tm));
	chrono::system_clock::time_point output_tp;
	// earlier than 08:30 AM
	if (8 >= now_tm.tm_hour && 30 >= now_tm.tm_min) {
		output_tp = today_tp;
	}// later than 08:30 AM
	else {
		output_tp = today_tp + chrono::hours(24);
	}
	return output_tp;
}
