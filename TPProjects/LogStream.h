#pragma once
#include <cstdio>
#include <string>
#include <mutex>
using namespace std;

class LogStream {
private:
	FILE* logFile;
	ostringstream* stream;
	mutex mtx;
public:
	LogStream(FILE* logFile, mutex mtx);
	~LogStream();

	template <typename T>
	ostringstream* operator<<(T const& value)
	{
		*stream << value;
		return stream;
	}
};