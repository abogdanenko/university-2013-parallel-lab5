#include "computationbase.h"

ComputationBase::ComputationBase(const Args& args):
    args(args),
    params(args.QubitCount(), args.TargetQubit()),
    U( vector< vector<complexd> >(2, vector<complexd>(2)))
{
    // Assign identity matrix to U
    U[0][0] = 1;
    U[0][1] = 0;
    U[1][0] = 0;
    U[1][1] = 1;
}

