#pragma  once
#include "Utils.h"
class SlipperyPhase {
public:
	enum PHASE_ENUM {
		PHASE_1,
		PHASE_2,
		PHASE_3,
		OUT_OF_PHASE
	};
	static SlipperyPhase::PHASE_ENUM getPhase() {
		time_t now = time(NULL);
		tm now_tm;
		localtime_s(&now_tm, &now);
		string phasePeriod = getConfig("config", "SlipperyPhase1");
		if (inThisPeriod(phasePeriod, now_tm)) {
			return SlipperyPhase::PHASE_ENUM::PHASE_1;
		}
		phasePeriod = getConfig("config", "SlipperyPhase2");
		if (inThisPeriod(phasePeriod, now_tm)) {
			return SlipperyPhase::PHASE_ENUM::PHASE_2;
		}
		phasePeriod = getConfig("config", "SlipperyPhase3");
		if (inThisPeriod(phasePeriod, now_tm)) {
			return SlipperyPhase::PHASE_ENUM::PHASE_3;
		}
		return SlipperyPhase::PHASE_ENUM::OUT_OF_PHASE;

	}
};
