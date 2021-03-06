#ifndef COMPUTATIONBASE_H
#define COMPUTATIONBASE_H

#include "parser.h"
#include "computationparams.h"
#include "typedefs.h"

class ComputationBase
{
    protected:
    Args args;
    ComputationParams params;
    Matrix U;
    static Matrix HadamardMatrix();
    ComputationBase(const Args& args);
    public:
    static const int master_rank = 0;
};

#endif
