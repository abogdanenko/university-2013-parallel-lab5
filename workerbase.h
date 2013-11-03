#ifndef WORKERBASE_H
#define WORKERBASE_H

#include "computationbase.h"

class WorkerBase: protected ComputationBase
{
    void SwapWithPartner();
    void ApplyOperator();
    void NormalizeGlobal();
    Vector psi;
    Vector psi_noiseless;
    protected:
    WorkerBase(const Args& args);
    complexd ScalarProduct() const;
    void InitVector();
    void ApplyOperatorToEachQubit();
    void SaveNoiselessVector();
};

#endif
