#ifndef ROUTINES_H
#define ROUTINES_H

#include <sstream> // stringstream

#include "typedefs.h"

using std::stringstream;
using std::string;

void ShmemReceiveElem(int from, void* data, int sz);

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

Matrix MatrixMultiply(const Matrix& A, const Matrix& B);
// get seed based on current time, process pid and rank
unsigned GetUniqueSeed();

#endif
