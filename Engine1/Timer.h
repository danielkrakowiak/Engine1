#pragma once

#include <Windows.h>

namespace Engine1
{
    class Timer
    {
        public:
        // In milliseconds.
        static double getElapsedTime( const Timer& timerLater, const Timer& timerEarlier );

        Timer();
        ~Timer();
        void reset();

        void operator =(const Timer& timer);

        private:
        static double getTimerFrequencyInKHz();
        static double timerFrequencyInKHz;


        LARGE_INTEGER m_time;

        // Copying is not allowed.
        Timer( const Timer& ) = delete;
    };
}

