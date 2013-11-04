#ifndef ROUTINES_H
#define ROUTINES_H

#include <sstream> // stringstream

#include "typedefs.h"

using std::stringstream;
using std::string;

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

complexd ScalarProduct(const Vector& a, const Vector& b);
Matrix MatrixMultiply(const Matrix& A, const Matrix& B);
void CommWorldSrand();

#endif
