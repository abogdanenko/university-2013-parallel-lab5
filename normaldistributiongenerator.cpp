#include <stdlib.h> // srand, rand
#include <time.h> // time

#include "normaldistributiongenerator.h"

NormalDistributionGenerator::NormalDistributionGenerator()
{
    srand(time(NULL));
}

double NormalDistributionGenerator::operator()()
{
    const int n = 12;
    double s = -0.5 * n;
    for (int i = 0; i < n; i++)
    {
        const double uniform01 = (double) rand() / RAND_MAX;
        s += uniform01;
    }
    return s;
}

