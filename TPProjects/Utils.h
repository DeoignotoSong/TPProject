#pragma once
#include <string>
#include <vector>
#include <chrono>
using namespace std;

vector<string> split(const string& s, const string& seperator);
chrono::system_clock::time_point getNextStartTime();