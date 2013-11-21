#include <dislib.h>

#include "timer.h"

void Timer::Start()
{
    start = shmem_time();
}

void Timer::Stop()
{
    end = shmem_time();
}

double Timer::GetDelta() const
{
    return end - start;
}


