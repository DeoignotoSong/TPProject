#include <cstdio>
#include <cstring>
#include <time.h>
#include <sys/timeb.h>
#include <vector>
#include "getconfig.h"

#ifndef __TIMER_MNG_
#define __TIMER_MNG_

#define ull unsigned long long 
#define __INF__ 1e18

using namespace std;

enum t_type { ONCE, CIRCLE };

class Timer_mng;
class Timer;

struct Heap_entry {		//����Ŀ
	unsigned long long time;
	Timer* timer;
};
struct Timer {
	t_type ttype;		//�������ͣ�һ�λ���ѭ��
	void* (*run)(void* args);	//�ص�����
	void* arg;					//�����������
	ull itvl;					//ʱ��Ƭ����(ms)
	ull expires;				//����ʱ��
	int heapIndex;				//�ڶ��е��±�
	Timer();					//��ʼ��
	void Start(void* (*run)(void* args), void* arg, ull itvl, t_type ttype);
	void OnTimer();				//��ʣ��ʱ��		
};
struct Timer_mng {
	vector<Heap_entry*>heap;		//��
	~Timer_mng();					//��������
	void DetectTimers();			//����Ƿ�ʱ
	void AddTimer(Timer* timer);	//��������һ����ʱ��
	void RemoveTimer(Timer* timer);	//�Ƴ���ʱ��
	void UpHeap(int idx);			//�Ѳ��������ϸ���
	void DownHeap(int idx);			//�Ѳ��������¸���
	void SwapHeap(int idx1, int idx2);	//�������е�����Ԫ��
};

ull Get_now();			//�õ���ǰʱ��(ms)

#endif
