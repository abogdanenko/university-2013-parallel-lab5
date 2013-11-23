#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <complex>
#include <vector>
#include <utility> // std::pair

using std::vector;
using std::complex;
using std::pair;

typedef complex<double> complexd;
typedef vector<complexd> Vector;
typedef Vector::size_type Index;
typedef vector<Vector> Matrix;
typedef pair<Index, complexd> IndexElemPair;

#endif
