// Log, version 0.1: a simple logging class
#pragma once
#include <sstream>

enum TLogLevel {
	logERROR, logWARNING, logINFO, logDEBUG, logDEBUG1,
	logDEBUG2, logDEBUG3, logDEBUG4
};

template <typename OutputPolicy>
class Log
{
public:
	Log();
	virtual ~Log();
	std::ostringstream& Get(TLogLevel level);
public:
	static TLogLevel& ReportingLevel();
protected:
	std::ostringstream os;
private:
	Log(const Log&);
	Log& operator =(const Log&);
private:
	TLogLevel messageLevel;
};