#ifndef LZ77_ARCHPROJ_TIMER
#define LZ77_ARCHPROJ_TIMER

#include <Windows.h>

//Для тестирования скорости работы функций
class Timer
{
private:
	__int64 freq64;
	LARGE_INTEGER start_t, end_t;
public:
	Timer()
	{
		LARGE_INTEGER large;
		QueryPerformanceFrequency(&large);
		freq64 =  large.QuadPart/1000;//
	}
	void reset()
	{
		QueryPerformanceCounter(&start_t);
	}
	float cpuTime()
	{
		QueryPerformanceCounter(&end_t);
		__int64 start64 = start_t.QuadPart;
		__int64 end64 = end_t.QuadPart;
		return (float)(end64-start64)/(float)freq64;
	}
};

#endif