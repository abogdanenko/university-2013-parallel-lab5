#include "timer.h"
#include <routines.h>

Timer::Timer():
    sum(0.0)
{

}

void Timer::Start()
{
    ShmemBarrierAll();
    start = shmem_time();
}

void Timer::Stop()
{
    ShmemBarrierAll();
    const double end = shmem_time();
    const double delta = end - start;
    sum += delta;
}

double Timer::Total() const
{
    return sum;
}
