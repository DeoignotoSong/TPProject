#pragma once
#ifndef __TIMER_MNG_
#define __TIMER_MNG_

#define ull unsigned long long
constexpr auto __INF__ = 1e18;

using namespace std;

enum t_type { ONCE, CIRCLE };

class Timer_mng;
class Timer;

struct Heap_entry {
	ull time;
	Timer* timer;
};

struct Timer {
	t_type ttype;
	void* (*run)(void* args);
	void* arg;
	ull itvl;
	ull expires;
	int heapIndex;
	Timer();
	void Start(void* (*run)(void* args), void* arg, ull itvl, t_type ttype);
};

#endif
