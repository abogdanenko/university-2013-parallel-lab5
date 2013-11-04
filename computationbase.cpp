#include "computationbase.h"

ComputationBase::ComputationBase(const Args& args):
    args(args),
    params(args.QubitCount())
{
}

Matrix ComputationBase::HadamardMatrix()
{
    const double elem = 1.0 / sqrt(2.0);
    const Vector row = {elem, elem};
    Matrix m(2, row);
    m[1][1] *= -1.0;
    return m;
}
