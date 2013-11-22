#include "computationbase.h"

ComputationBase::ComputationBase(const Args& args):
    args(args),
    params(args.QubitCount()),
    U(IdentityMatrix())
{
}

Matrix ComputationBase::IdentityMatrix()
{
    Matrix m(2, Vector(2));
    m[0][0] = 1.0;
    m[0][1] = 0.0;
    m[1][0] = 0.0;
    m[1][1] = 1.0;
    return m;
}
