#include "Timer.h"

using namespace Engine1;

double Timer::timerFrequencyInKHz = getTimerFrequencyInKHz( );

double Timer::getTimerFrequencyInKHz( ) {
	LARGE_INTEGER timerFrequency;
	QueryPerformanceFrequency( &timerFrequency );

	double frequencyInKHz = double( timerFrequency.QuadPart ) / 1000.0;

	return frequencyInKHz;
}

double Timer::lapse( const Timer& timerLater, const Timer& timerEarlier ) {
	return ( timerLater.m_time.QuadPart - timerEarlier.m_time.QuadPart ) / timerFrequencyInKHz;
}

Timer::Timer() {
	QueryPerformanceCounter( &m_time );
}


Timer::~Timer() {}

void Timer::reset() {
	QueryPerformanceCounter( &m_time );
}