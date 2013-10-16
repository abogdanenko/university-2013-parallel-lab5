#include "computationbase.h"

ComputationBase::ComputationBase(const Parser::Args& args):
    args(args),
    params(args.qubit_count, args.target_qubit),
    U( vector< vector<complexd> >(2, vector<complexd>(2)))
{
    // Assign identity matrix to U
    U[0][0] = 1;
    U[0][1] = 0;
    U[1][0] = 0;
    U[1][1] = 1;
}

