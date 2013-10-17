#include <algorithm> // generate, copy
#include <functional> // multiplies
#include <mpi.h>

#include "workerbase.h"
#include "randomcomplexgenerator.h"
#include "applyoperator.h"

using std::multiplies;
using std::transform;
using std::copy;

WorkerBase::WorkerBase(const Args& args):
    ComputationBase(args)
{
    psi.resize(params.WorkerVectorSize());
}

void WorkerBase::InitRandom()
{
    RandomComplexGenerator gen;
    generate(psi.begin(), psi.end(), gen);
    NormalizeGlobal();
}

void WorkerBase::NormalizeGlobal()
{
    long double local_sum = 0.0;
    for (vector<complexd>::const_iterator it = psi.begin(); it != psi.end(); it++)
    {
        local_sum += norm(*it);
    }

    long double global_sum;
    MPI_Allreduce(&local_sum, &global_sum, 1, MPI_LONG_DOUBLE, MPI_SUM,
        MPI_COMM_WORLD);

    const complexd coef = 1.0 / sqrt(global_sum);
    // multiply each element by coef
    transform(psi.begin(), psi.end(), psi.begin(),
        bind1st(multiplies<complexd>(), coef));
}

void WorkerBase::ApplyOperator()
{
    ::ApplyOperator(psi, U, params.WorkerTargetQubit());
}

bool WorkerBase::ReceiveNextBuf()
{
    static vector<complexd>::iterator it = psi.begin();
    if (it == psi.end())
    {
        return false;
    }
    MPI_Request request = MPI_REQUEST_NULL;

    MPI_Irecv(&*it, params.BufSize(), MPI_DOUBLE_COMPLEX, master_rank,
        MPI_ANY_TAG, MPI_COMM_WORLD, &request);

    it += params.BufSize();
    return true;
}

bool WorkerBase::SendNextBuf()
{
    static vector<complexd>::iterator it = psi.begin();
    if (it == psi.end())
    {
        return false;
    }

    MPI_Request request = MPI_REQUEST_NULL;

    MPI_Isend(&*it, params.BufSize(), MPI_DOUBLE_COMPLEX, master_rank,
        tag, MPI_COMM_WORLD, &request);

    it += params.BufSize();
    return true;
}

