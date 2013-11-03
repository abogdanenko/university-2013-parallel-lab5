#include <algorithm> // generate, copy
#include <functional> // multiplies
#include <mpi.h>

#ifdef DEBUG
#include "debug.h"
#endif

#ifdef NORANDOM
#include "basisvector1generator.h"
#else
#include "randomcomplexgenerator.h"
#endif

#include "workerbase.h"
#include "applyoperator.h"

using std::multiplies;
using std::transform;
using std::copy;

WorkerBase::WorkerBase(const Args& args):
    ComputationBase(args)
{
    psi.resize(params.WorkerVectorSize());
}

void WorkerBase::InitVector()
{
    #ifdef DEBUG
    cout << IDENT(3) << "WorkerBase::InitVector()..." << endl;
    #endif

    #ifdef NORANDOM
    BasisVector1Generator gen;
    #else
    RandomComplexGenerator gen;
    #endif

    generate(psi.begin(), psi.end(), gen);
    NormalizeGlobal();

    #ifdef DEBUG
    cout << IDENT(3) << "WorkerBase::InitVector() return" << endl;
    #endif
}

void WorkerBase::NormalizeGlobal()
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

void WorkerBase::ApplyOperator()
{
    #ifdef DEBUG
    cout << IDENT(4) << "WorkerBase::ApplyOperator()..." << endl;
    #endif

    if (params.TargetQubitIsGlobal())
    {
        SwapWithPartner();
        ::ApplyOperator(psi, U, params.WorkerTargetQubit());
        SwapWithPartner();
    }
    else
    {
        ::ApplyOperator(psi, U, params.WorkerTargetQubit());
    }

    #ifdef DEBUG
    cout << IDENT(4) << "WorkerBase::ApplyOperator()... return" << endl;
    #endif
}

void WorkerBase::ApplyOperatorToEachQubit()
{
    #ifdef DEBUG
    cout << IDENT(3) << "WorkerBase::ApplyOperatorToEachQubit()..." << endl;
    #endif

    for (int target_qubit = 1; target_qubit <= args.QubitCount();
        target_qubit++)
    {
        params.SetTargetQubit(target_qubit);
        ApplyOperator();
    }

    #ifdef DEBUG
    cout << IDENT(3) << "WorkerBase::ApplyOperatorToEachQubit() return" << endl;
    #endif
}

void WorkerBase::SaveNoiselessVector()
{
    psi_noiseless = psi;
}

void WorkerBase::SwapWithPartner()
{
    #ifdef DEBUG
    cout << IDENT(3) << "WorkerBase::SwapWithPartner()... " << endl;
    #endif

    const auto middle = psi.begin() + params.WorkerVectorSize() / 2;
    const auto begin = params.TargetQubitValue() ? psi.begin() : middle;
    const auto end = params.TargetQubitValue() ? middle : psi.end();

    for (auto it = begin; it != end; it += params.BufSize())
    {
        MPI_Sendrecv_replace(
            &*it,
            params.BufSize() * sizeof(complexd),
            MPI_BYTE,
            params.PartnerRank(),
            tag,
            params.PartnerRank(),
            MPI_ANY_TAG,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
    }

    #ifdef DEBUG
        cout << IDENT(3) << "WorkerBase::SwapWithPartner() return" << endl;
    #endif
}
