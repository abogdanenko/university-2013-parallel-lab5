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
    Vector psi;
    Vector psi_noiseless;
    protected:
    WorkerBase(const Args& args);
    complexd ScalarProduct() const;
    void VectorInitRandom();
    void ApplyOperatorToEachQubit();
    void SwapVectors();
};

#endif
