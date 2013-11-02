#ifndef WORKER_H
#define WORKER_H

#include "computationbase.h"

class Worker: public ComputationBase
{
    friend class Master;
    void NormalizeGlobal();
    protected:
    Vector psi;
    Vector psi_noiseless;

    public:
    Worker(const Args& args);
    ReceiveMatrix();
    void InitVector();
    complexd ScalarProduct();
    void ApplyOperator();
    void ApplyOperatorToEachQubit();
    void SaveNoiselessVector();
    bool ReceiveNextBuf();
    bool SendNextBuf();
};

#endif
