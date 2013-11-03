#include <mpi.h>

#ifdef DEBUG
#include "debug.h"
#endif

#include "computationparams.h"
#include "routines.h"

using std::min;

ComputationParams::ComputationParams(const int qubit_count):
    max_buf_size(1024),
    qubit_count(qubit_count),
    target_qubit(-1)
{
    int worker_count;
    MPI_Comm_size(MPI_COMM_WORLD, &worker_count);

    const Index vector_size      = 1L << qubit_count;
    worker_vector_size           = vector_size / worker_count;
    buf_size                     = min((Index) max_buf_size,
                                       worker_vector_size / 2);
    const int worker_qubit_count = intlog2(worker_vector_size);
    global_qubit_count           = qubit_count - worker_qubit_count;
    most_significant_local_qubit = global_qubit_count + 1;
}

void ComputationParams::SetTargetQubit(const int target_qubit)
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    this->target_qubit     = target_qubit;
    target_qubit_is_global = target_qubit < most_significant_local_qubit;
    if (target_qubit_is_global)
    {
        worker_target_qubit = 1;
        const Index mask    = 1L << (global_qubit_count - target_qubit);
        target_qubit_value  = (rank & mask) ? 1 : 0;
        partner_rank        = rank ^ mask;
    }
    else
    {
        worker_target_qubit = 1 + target_qubit - most_significant_local_qubit;
    }
}

Index ComputationParams::WorkerVectorSize() const
{
    return worker_vector_size;
}

int ComputationParams::BufSize() const
{
    return buf_size;
}

bool ComputationParams::TargetQubitIsGlobal() const
{
    return target_qubit_is_global;
}

int ComputationParams::WorkerTargetQubit() const
{
    return worker_target_qubit;
}

int ComputationParams::TargetQubitValue() const
{
    return target_qubit_value;
}

int ComputationParams::PartnerRank() const
{
    return partner_rank;
}

#ifdef DEBUG
void ComputationParams::PrintAll() const
{
    const int I = 5;
    cout << IDENT(I - 1) << "ComputationParams::PrintAll()..." << endl;
    // these params don't change during execution
    cout << IDENT(I) << "qubit_count = " << qubit_count << endl;
    cout << IDENT(I) << "global_qubit_count = " << global_qubit_count << endl;
    cout << IDENT(I) << "most_significant_local_qubit = " << most_significant_local_qubit << endl;
    cout << IDENT(I) << "worker_vector_size = " << worker_vector_size << endl;
    cout << IDENT(I) << "max_buf_size = " << max_buf_size << endl;
    cout << IDENT(I) << "buf_size = " << buf_size << endl;

    // these params change every time target_qubit changes
    if (target_qubit != -1)
    {
        cout << IDENT(I) << "target_qubit = " << target_qubit << endl;
        cout << IDENT(I) << "worker_target_qubit = " << worker_target_qubit
            << endl;
        cout << IDENT(I) << "target_qubit_is_global = "
            << target_qubit_is_global << endl;

        // these params are defined only when target qubit is global
        if (target_qubit_is_global)
        {
            cout << IDENT(I) << "target_qubit_value = " << target_qubit_value
                << endl;
            cout << IDENT(I) << "partner_rank = " << partner_rank << endl;
        }
    }
    cout << IDENT(I - 1) << "ComputationParams::PrintAll() return" << endl;
}
#endif
