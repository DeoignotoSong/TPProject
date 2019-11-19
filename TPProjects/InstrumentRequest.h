#pragma once
#include <string>
using namespace std;
class InstrumentRequest
{
public:
	string instrument;
	int requestId;
	InstrumentRequest(string instrument, int requestId);
};

