#pragma once
#include "Log.h"
#include "Output2FILE.h"

inline FILE*& Output2FILE::StreamImpl()
{
	static FILE* pStream = stderr;
	return pStream;
}
inline void Output2FILE::SetStream(FILE* pFile)
{
	mtx.lock();
	StreamImpl() = pFile;
	mtx.unlock();
}
inline void Output2FILE::Output(const std::string& msg)
{
	mtx.lock();
	FILE* pStream = StreamImpl();
	if (!pStream)
		return;
	fprintf(pStream, "%s", msg.c_str());
	fflush(pStream);
	mtx.unlock();
}

typedef Log<Output2FILE> FILELog;
#define FILE_LOG(level) \
if (level > FILELog::ReportingLevel() || !Output2FILE::Stream()) ; \
else FILELog().Get(messageLevel)