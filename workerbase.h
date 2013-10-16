#ifndef WORKERBASE_H
#define WORKERBASE_H

#include "computationbase.h"

class WorkerBase: public ComputationBase
{
    protected:
    /* Slice of initial state vector, split into two halves. First half
       corresponds to k-th qubit state 0, second half corresponds to target
       qubit state 1. The vector is modified in-place, so result is also stored
       here  */
    vector<complexd> psi; 

    public:
    WorkerBase(const Args& args);
    void InitRandom();
    void NormalizeGlobal();
    void ApplyOperator();
    bool ReceiveNextBuf();
    bool SendNextBuf();
};

#endif
