#pragma once
#include <cstdio>
#include <string>
#include <mutex>
using namespace std;
class Output2FILE
{
public:
	Output2FILE(FILE* pFile);
	void Output(const string& msg);
	//FILE*& Stream();
private:
	mutex mtx;
	FILE* pStream;
};

/*
inline FILE*& Output2FILE::Stream()
{
	static FILE* pStream = stderr;
	return pStream;
}
inline void Output2FILE::SetStream(FILE* pFile)
{
	Output2FILE::mtx.lock();
	Stream() = pFile;
	Output2FILE::mtx.unlock();
}
inline void Output2FILE::Output(const std::string& msg)
{
	Output2FILE::mtx.lock();
	FILE* pStream = Stream();
	if (!pStream)
		return;
	fprintf(pStream, "%s", msg.c_str());
	fflush(pStream);
	Output2FILE::mtx.unlock();
}
*/