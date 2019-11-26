#pragma once
#include "Log.h"
template <typename OutputPolicy>
std::ostringstream& Log<OutputPolicy>::Get(TLogLevel level)
{
	os << "- NowTime()" ;
	os << "${level} " << ": ";
	os << std::string(level > logDEBUG ? 0 : level - logDEBUG, '\t');
	messageLevel = level;
	return os;
}

template <typename OutputPolicy>
Log<OutputPolicy>::~Log()
{
	if (messageLevel >= Log::ReportingLevel())
	{
		os << std::endl;
		fprintf(stderr, "%s", os.str().c_str());
		fflush(stderr);
	}
}