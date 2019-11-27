
#pragma once
#include "Output2FILE.h"

Output2FILE::Output2FILE(FILE* pFile)
{
	this->pStream = pFile;
}
void Output2FILE::Output(const std::string& msg)
{
	mtx.lock();
	if (!pStream)
		return;
	//fprintf(pStream,"[]", msg.c_str());
	fflush(pStream);
	mtx.unlock();
	fclose(pStream);
}