#ifndef WORKERBASE_H
#define WORKERBASE_H

#include "computationbase.h"

class WorkerBase: protected ComputationBase
{
    friend class Master;
    void SwapWithPartner();
    void ApplyOperator();
    void NormalizeGlobal();
    Vector buffer;
    protected:
    Vector psi;
    WorkerBase(const Args& args);
    void VectorInitRandom();
    void ApplyOperatorToEachQubit();
};

#endif
