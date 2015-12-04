#include "Timer.h"

using namespace Engine1;

double Timer::timerFrequencyInKHz = getTimerFrequencyInKHz( );

double Timer::getTimerFrequencyInKHz( ) {
	LARGE_INTEGER timerFrequency;
	QueryPerformanceFrequency( &timerFrequency );

	double timerFrequencyInKHz = double( timerFrequency.QuadPart ) / 1000.0;

	return timerFrequencyInKHz;
}

double Timer::lapse( const Timer& timerLater, const Timer& timerEarlier ) {
	return ( timerLater.time.QuadPart - timerEarlier.time.QuadPart ) / timerFrequencyInKHz;
}

Timer::Timer() {
	QueryPerformanceCounter( &time );
}


Timer::~Timer() {}

void Timer::reset() {
	QueryPerformanceCounter( &time );
}