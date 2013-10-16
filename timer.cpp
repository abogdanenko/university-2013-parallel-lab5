#include <mpi.h>

#include "timer.h"

void Timer::Start()
{
    start = MPI_Wtime();
}

void Timer::Stop()
{
    end = MPI_Wtime();
}

double Timer::GetDelta() const
{
    return end - start;
}


