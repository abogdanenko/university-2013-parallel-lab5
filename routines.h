#ifndef ROUTINES_H
#define ROUTINES_H

#include <sstream> // stringstream

using std::stringstream;

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

template <class Number>
Number string_to_number(const string& s)
{
    Number n;
    stringstream ss(s);
    ss >> n;
    return n;
}

#endif
