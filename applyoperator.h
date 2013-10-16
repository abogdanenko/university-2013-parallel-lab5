#ifndef APPLYOPERATOR_H
#define APPLYOPERATOR_H

#include "typedefs.h"

using std::vector;

void ApplyOperator(vector<complexd>& psi,
    const vector< vector<complexd> >& U,
    const int k);

#endif
