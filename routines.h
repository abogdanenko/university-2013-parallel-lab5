#ifndef ROUTINES_H
#define ROUTINES_H

// for n = 2**m returns m
template <class Integer>
int intlog2(const Integer n)
{
    int result = 0;
    Integer t = n;
    while (t >>= 1)
      result++;
    return result;
}

#endif
