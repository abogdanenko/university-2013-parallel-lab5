#include <stdlib.h> // srand, rand
#include <time.h> // time

#include "randomcomplexgenerator.h"

/*
    Pseudo-random number generator

    Returns real numbers between 0 and 1
    copied from
    http://en.wikipedia.org/wiki/Random_number_generation#cite_ref-6
*/
double RandomComplexGenerator::random01d()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    unsigned r = (m_z << 16) + m_w;  // 32-bit result 
    return double (r) / 0xffffffff;
}

RandomComplexGenerator::RandomComplexGenerator()
{
    srand(time(NULL));
    m_z = rand();
    m_w = rand();
}

complexd RandomComplexGenerator::operator()()
{
    double re = random01d() - 0.5;
    double im = random01d() - 0.5;
    return complexd(re, im);
}

