#include <mpi.h>

#ifdef DEBUG
#include <iostream>
#endif

#include "computationparams.h"
#include "routines.h"

using std::min;
using std::max;

#ifdef DEBUG
using std::cout;
using std::endl;
#endif

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

int ComputationParams::MostSignificantLocalQubit() const
{
    const int worker_qubit_count = intlog2(WorkerVectorSize()); 
    const int global_qubit_count = qubit_count - worker_qubit_count ;
    return global_qubit_count + 1;
}

bool ComputationParams::Split() const
{
    // global qubits are going to be modified
    return target_qubit < MostSignificantLocalQubit();
}

int ComputationParams::WorkerTargetQubit() const
{
    if (Split())
    {
        return 1;
    }
    else
    {
        return 1 + target_qubit - MostSignificantLocalQubit();
    }
}

int ComputationParams::SliceCount() const
{
    return 1l << target_qubit - 1;
}

int ComputationParams::WorkersPerSlice() const
{
    return worker_count / SliceCount();
}

#ifdef DEBUG
void ComputationParams::PrintAll() const
{
    cout << "qubit_count = " << qubit_count << endl;
    cout << "target_qubit = " << target_qubit << endl;
    cout << "worker_count = " << worker_count << endl;
    cout << "max_buf_size = " << max_buf_size << endl;
    cout << "MostSignificantLocalQubit() = " << MostSignificantLocalQubit() << endl;
    cout << "WorkerCount() = " << WorkerCount() << endl;
    cout << "VectorSize() = " << VectorSize() << endl;
    cout << "WorkerVectorSize() = " << WorkerVectorSize() << endl;
    cout << "BufSize() = " << BufSize() << endl;
    cout << "BufCount() = " << BufCount() << endl;
    cout << "Split() = " << Split() << endl;
    cout << "WorkerTargetQubit() = " << WorkerTargetQubit() << endl;
    if (Split())
    {
        cout << "SliceCount() = " << SliceCount() << endl;
        cout << "WorkersPerSlice() = " << WorkersPerSlice() << endl;
    }
}
#endif
