#include "applyoperator.h"
#include "routines.h"

void ApplyOperator(vector<complexd>& psi,
    const vector< vector<complexd> >& U,
    const int k)
{
    const Index N = psi.size();
    const int n = intlog2(N);
    const Index mask = 1L << (n - k);
    
    for (Index i = 0; i < N; i++)
    {
        // bit of i corresponding to target qubit
        if (i & mask == 0)
        {
            const Index i0 = i & ~ mask; // clear selected bit
            const Index i1 = i | mask; // set selected bit

            const complexd a = psi[i0];
            const complexd b = psi[i1];

            psi[i0] = U[0][0] * a + U[0][1] * b;
            psi[i1] = U[1][0] * a + U[1][1] * b;
        }
    }
}

