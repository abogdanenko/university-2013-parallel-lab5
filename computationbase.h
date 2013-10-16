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
    vector< vector<complexd> > U;
    public:
    static const int master_rank = 0;
    ComputationBase(const Args& args);
};

#endif
