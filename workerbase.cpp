#include <algorithm> // generate, copy
#include <functional> // multiplies
#include <dislib.h>

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
#include "routines.h"

using std::multiplies;
using std::transform;
using std::copy;

WorkerBase::WorkerBase(const Args& args):
    ComputationBase(args)
{
    psi.resize(params.WorkerVectorSize());
    buffer.resize(params.WorkerVectorSize() / 2);
}

void WorkerBase::VectorInitRandom()
{
    #ifdef DEBUG
    cout << INDENT(1) << "WorkerBase::VectorInitRandom()..." << endl;
    #endif

    #ifdef NORANDOM
    BasisVector1Generator gen;
    #else
    RandomComplexGenerator gen;
    #endif

    generate(psi.begin(), psi.end(), gen);
    NormalizeGlobal();

    #ifdef DEBUG
    cout << INDENT(1) << "WorkerBase::VectorInitRandom() return" << endl;
    #endif
}

void WorkerBase::NormalizeGlobal()
{
    double sum = 0.0;
    for (auto it = psi.begin(); it != psi.end(); it++)
    {
        sum += norm(*it);
    }

    shmem_double_allsum(&sum);

    const complexd coef = 1.0 / sqrt(sum);
    // multiply each element by coef
    transform(psi.begin(), psi.end(), psi.begin(),
        bind1st(multiplies<complexd>(), coef));
}

void WorkerBase::ApplyOperator()
{
    #ifdef DEBUG
    cout << INDENT(2) << "WorkerBase::ApplyOperator()..." << endl;
    params.PrintAll();
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
    cout << INDENT(2) << "WorkerBase::ApplyOperator()... return" << endl;
    #endif
}

void WorkerBase::ApplyOperatorToEachQubit()
{
    #ifdef DEBUG
    cout << INDENT(1) << "WorkerBase::ApplyOperatorToEachQubit()..." << endl;
    #endif

    for (int target_qubit = 1; target_qubit <= args.QubitCount();
        target_qubit++)
    {
        params.SetTargetQubit(target_qubit);
        ApplyOperator();
    }

    #ifdef DEBUG
    cout << INDENT(1) << "WorkerBase::ApplyOperatorToEachQubit() return"
        << endl;
    #endif
}

void WorkerBase::SwapWithPartner()
{
    #ifdef DEBUG
    cout << INDENT(3) << "WorkerBase::SwapWithPartner()..." << endl;
    #endif

    const auto middle = psi.begin() + buffer.size();
    const auto begin = params.TargetQubitValue() ? psi.begin() : middle;

    ShmemTransfer s;

    s.Receive(&buffer.front());
    s.Send(
        &*begin, // data pointer
        buffer.size() * sizeof(complexd), // data size in bytes
        params.PartnerRank()); // destination rank
    shmem_barrier_all();
    copy(buffer.begin(), buffer.end(), begin);

    #ifdef DEBUG
        cout << INDENT(3) << "WorkerBase::SwapWithPartner() return" << endl;
    #endif
}
