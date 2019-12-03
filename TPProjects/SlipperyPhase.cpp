#pragma  once
#include "Utils.h"
class SlipperyPhase {
public:
	enum PHASE_ENUM {
		PHASE_1,
		PHASE_2,
		PHASE_3,
		OUT_OF_PHASE,
		PHASE_LAG
	};
	static SlipperyPhase::PHASE_ENUM getPhase() {
		time_t now = time(NULL);
		tm now_tm;
		localtime_s(&now_tm, &now);
		string phasePeriod = getConfig("config", "SlipperyPhase1");
		int phase1State = inThisPeriod(phasePeriod, now_tm);
		if (0 == phase1State) {
			return SlipperyPhase::PHASE_ENUM::PHASE_1;
		}
		phasePeriod = getConfig("config", "SlipperyPhase2");
		if (0 == inThisPeriod(phasePeriod, now_tm)) {
			return SlipperyPhase::PHASE_ENUM::PHASE_2;
		}
		phasePeriod = getConfig("config", "SlipperyPhase3");
		int phase3State = inThisPeriod(phasePeriod, now_tm);
		if (0 == phase3State) {
			return SlipperyPhase::PHASE_ENUM::PHASE_3;
		}
		if (phase1State * phase3State < 0) {
			return SlipperyPhase::PHASE_ENUM::PHASE_LAG;
		}
		return SlipperyPhase::PHASE_ENUM::OUT_OF_PHASE;

	}
};
