#include <mpi.h>

#include "computationparams.h"
#include "routines.h"

using std::min;
using std::max;

int ComputationParams::MPIGetWorldSize()
{
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);        
    return size;
}

ComputationParams::ComputationParams(const int qubit_count, const int target_qubit):
    qubit_count(qubit_count),
    target_qubit(target_qubit),
    worker_count(MPIGetWorldSize()),
    max_buf_size(1024)
{
    
}

int ComputationParams::WorkerCount() const
{
    return worker_count; 
}

Index ComputationParams::VectorSize() const
{
    return 1L << qubit_count;
}

Index ComputationParams::WorkerVectorSize() const
{
    return VectorSize() / worker_count; 
}

int ComputationParams::BufSize() const
{
    return min((Index) max_buf_size, WorkerVectorSize() / 2);
}

int ComputationParams::BufCount() const
{
    return WorkerVectorSize() / BufSize();
}

int ComputationParams::WorkerQubitCount() const
{
    return intlog2(WorkerVectorSize());
}

int ComputationParams::GlobalQubitCount() const
{
    return qubit_count - WorkerQubitCount();
}

bool ComputationParams::Split() const
{
    return target_qubit < GlobalQubitCount();
}

int ComputationParams::WorkerTargetQubit() const
{
    return max(target_qubit - GlobalQubitCount(), 0);
}

Index ComputationParams::SliceSize() const
{
    return 1l << qubit_count - target_qubit;
}

Index ComputationParams::SliceCount() const
{
    return 1l << target_qubit;
}

int ComputationParams::WorkersPerSlice() const
{
    return worker_count / SliceCount();
}
