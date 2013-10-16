#include <mpi.h>

#include "timer.h"

void Timer::TimerStart()
{
    start = MPI_Wtime();
}

void Timer::TimerStop()
{
    end = MPI_Wtime();
}

void Timer::GetDelta()
{
    return end_time - start_time;
}


