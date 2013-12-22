#include <mpi.h>

#include "timer.h"

Timer::Timer():
    sum(0.0)
{

}

void Timer::Start()
{
    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();
}

void Timer::Stop()
{
    MPI_Barrier(MPI_COMM_WORLD);
    const double end = MPI_Wtime();
    const double delta = end - start;
    sum += delta;
}

double Timer::Total() const
{
    return sum;
}
