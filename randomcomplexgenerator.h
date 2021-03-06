#ifndef RANDOMCOMPLEXGENERATOR_H
#define RANDOMCOMPLEXGENERATOR_H

#include "typedefs.h" // complexd

class RandomComplexGenerator
{
    unsigned m_w;
    unsigned m_z;
    inline double random01d();
    public:
    RandomComplexGenerator();
    complexd operator()();
};

#endif
