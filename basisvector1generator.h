#ifndef BASISVECTOR1GENERATOR_H
#define BASISVECTOR1GENERATOR_H

#include "typedefs.h" // complexd

class BasisVector1Generator
{
    bool first_call;
    public:
    BasisVector1Generator();
    complexd operator()();
};

#endif
