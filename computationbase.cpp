#include "computationbase.h"

ComputationBase::ComputationBase(const Args& args):
    args(args),
    params(args.QubitCount())
{
}

Matrix ComputationBase::HadamardMatrix()
{
    Matrix m(2, Vector(2));
    const complexd elem = 1.0 / sqrt(2.0);
    m[0][0] = elem;
    m[0][1] = elem;
    m[1][0] = elem;
    m[1][1] = -1.0 * elem;

    return m;
}
