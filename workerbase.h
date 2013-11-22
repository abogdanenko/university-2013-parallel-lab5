#ifndef WORKERBASE_H
#define WORKERBASE_H

#include "computationbase.h"

class WorkerBase: protected ComputationBase
{
    void SwapWithPartner();
    void ApplyOperator();
    void NormalizeGlobal();
    Vector psi;
    protected:
    WorkerBase(const Args& args);
    void VectorInitRandom();
    void ApplyOperatorToEachQubit();
};

#endif
