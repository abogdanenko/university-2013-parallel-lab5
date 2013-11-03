#ifndef NORMALDISTRIBUTIONGENERATOR_H
#define NORMALDISTRIBUTIONGENERATOR_H

#include "typedefs.h" // complexd

class NormalDistributionGenerator
{
    public:
    NormalDistributionGenerator();
    double operator()();
};

#endif
