#ifndef WORKERBASE_H
#define WORKERBASE_H

#include "computationbase.h"

class WorkerBase: public ComputationBase
{
    friend class Master;
    void NormalizeGlobal();
    protected:
    Vector psi;
    Vector psi_noiseless;

    public:
    WorkerBase(const Args& args);
    ReceiveMatrix();
    void InitVector();
    complexd ScalarProduct();
    void ApplyOperator();
    void SaveNoiselessVector();
    bool ReceiveNextBuf();
    bool SendNextBuf();
};

#endif
