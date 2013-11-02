#include <mpi.h>

#ifdef DEBUG
#include "debug.h"
#endif

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

ComputationParams::ComputationParams(const int qubit_count):
    qubit_count(qubit_count),
    worker_count(MPIGetWorldSize()),
    max_buf_size(1024)
{

}

void ComputationParams::SetTargetQubit(const int target_qubit)
{
    this->target_qubit = target_qubit;
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
    const int global_qubit_count = qubit_count - worker_qubit_count;
    return global_qubit_count + 1;
}

bool ComputationParams::TargetQubitIsGlobal() const
{
    return target_qubit < MostSignificantLocalQubit();
}

int ComputationParams::WorkerTargetQubit() const
{
    if (TargetQubitIsGlobal())
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
    return 1L << (target_qubit - 1);
}

int ComputationParams::WorkersPerSlice() const
{
    return worker_count / SliceCount();
}

#ifdef DEBUG
void ComputationParams::PrintAll() const
{
    cout << IDENT(1) << "qubit_count = " << qubit_count << endl;
    cout << IDENT(1) << "target_qubit = " << target_qubit << endl;
    cout << IDENT(1) << "worker_count = " << worker_count << endl;
    cout << IDENT(1) << "max_buf_size = " << max_buf_size << endl;
    cout << IDENT(1) << "MostSignificantLocalQubit() = "
        << MostSignificantLocalQubit() << endl;
    cout << IDENT(1) << "WorkerCount() = " << WorkerCount() << endl;
    cout << IDENT(1) << "VectorSize() = " << VectorSize() << endl;
    cout << IDENT(1) << "WorkerVectorSize() = " << WorkerVectorSize() << endl;
    cout << IDENT(1) << "BufSize() = " << BufSize() << endl;
    cout << IDENT(1) << "BufCount() = " << BufCount() << endl;
    cout << IDENT(1) << "TargetQubitIsGlobal() = " << TargetQubitIsGlobal() << endl;
    cout << IDENT(1) << "WorkerTargetQubit() = " << WorkerTargetQubit()
        << endl;
    if (TargetQubitIsGlobal())
    {
        cout << IDENT(1) << "SliceCount() = " << SliceCount() << endl;
        cout << IDENT(1) << "WorkersPerSlice() = " << WorkersPerSlice()
            << endl;
    }
}
#endif
