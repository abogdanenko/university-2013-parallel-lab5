#include <algorithm> // generate, copy
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
#include "shmem.h"

using std::copy;

WorkerBase::WorkerBase(const Args& args):
    ComputationBase(args)
{
    psi.resize(params.WorkerVectorSize());
    buffer.resize(psi.size() / 2);
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
    for (auto x: psi)
    {
        sum += norm(x);
    }

    shmem_double_allsum(&sum);

    const complexd coef = 1.0 / sqrt(sum);
    // multiply each element by coef
    for (auto &x: psi)
    {
        x *= coef;
    }
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

    const auto middle = psi.begin() + psi.size() / 2;
    const auto begin = params.TargetQubitValue() ? psi.begin() : middle;
    const auto end = params.TargetQubitValue() ? middle : psi.end();

    Shmem::SetReceiveVector(buffer.begin());

    // make sure partner is ready to receive before sending
    shmem_barrier_all();

    Shmem::SendVector(begin, end, params.PartnerRank());
    shmem_barrier_all();
    copy(buffer.begin(), buffer.end(), begin);

    #ifdef DEBUG
        cout << INDENT(3) << "WorkerBase::SwapWithPartner() return" << endl;
    #endif
}
