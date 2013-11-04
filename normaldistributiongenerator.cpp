#include <stdlib.h> // rand

#include "normaldistributiongenerator.h"

double NormalDistributionGenerator::operator()() const
{
    const int n = 12;
    double s = -0.5 * n;
    for (int i = 0; i < n; i++)
    {
        const double uniform01 = ((double) rand()) / RAND_MAX;
        s += uniform01;
    }
    return s;
}
