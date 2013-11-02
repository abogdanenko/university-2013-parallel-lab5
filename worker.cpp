#include <algorithm> // generate, copy
#include <functional> // multiplies
#include <mpi.h>

#ifdef DEBUG
#include "debug.h"
#endif

#include "worker.h"
#include "randomcomplexgenerator.h"
#include "applyoperator.h"

using std::multiplies;
using std::transform;
using std::copy;

Worker::Worker(const Args& args):
    ComputationBase(args)
{
    psi.resize(params.WorkerVectorSize());
}

void Worker::InitRandom()
{
    #ifdef DEBUG
    cout << IDENT(3) << "Worker::InitRandom()..." << endl;
    #endif
    RandomComplexGenerator gen;
    generate(psi.begin(), psi.end(), gen);
    NormalizeGlobal();
    #ifdef DEBUG
    cout << IDENT(3) << "Worker::InitRandom() return" << endl;
    #endif
}

void Worker::NormalizeGlobal()
{
    long double local_sum = 0.0;
    for (auto it = psi.begin(); it != psi.end(); it++)
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

void Worker::ApplyOperator()
{
    #ifdef DEBUG
    cout << IDENT(3) << "Worker::ApplyOperator()..." << endl;
    #endif

    ::ApplyOperator(psi, U, params.WorkerTargetQubit());

    #ifdef DEBUG
    cout << IDENT(3) << "Worker::ApplyOperator() return" << endl;
    #endif
}

bool Worker::ReceiveNextBuf()
{
    #ifdef DEBUG
    cout << IDENT(3) << "Worker::ReceiveNextBuf()..." << endl;
    #endif

    static auto it = psi.begin();
    if (it == psi.end())
    {
        #ifdef DEBUG
        cout << IDENT(3) << "Worker::ReceiveNextBuf() return false"
            << endl;
        #endif

        return false;
    }

    MPI_Recv(&*it, params.BufSize() * sizeof(complexd), MPI_BYTE, master_rank,
        MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    it += params.BufSize();

    #ifdef DEBUG
        cout << IDENT(3) << "Worker::ReceiveNextBuf() return true" << endl;
    #endif

    return true;
}

bool Worker::SendNextBuf()
{
    #ifdef DEBUG
    cout << IDENT(3) << "Worker::SendNextBuf()... " << endl;
    #endif

    static auto it = psi.begin();
    if (it == psi.end())
    {
        #ifdef DEBUG
        cout << IDENT(3) << "Worker::SendNextBuf() return false" << endl;
        #endif

        return false;
    }

    MPI_Request request;

    MPI_Isend(&*it, params.BufSize() * sizeof(complexd), MPI_BYTE, master_rank,
        tag, MPI_COMM_WORLD, &request);

    it += params.BufSize();

    #ifdef DEBUG
        cout << IDENT(3) << "Worker::SendNextBuf() return true" << endl;
    #endif

    return true;
}

