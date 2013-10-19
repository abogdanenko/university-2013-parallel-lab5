#include "applyoperator.h"
#include "routines.h"

#ifdef DEBUG
#include <iostream>
#include <iterator>
#include <algorithm>
#endif

#ifdef DEBUG
using std::cout;
using std::endl;
using std::copy;
using std::ostream_iterator;
#endif

#ifdef DEBUG
void PrintVector(const vector<complexd>& psi)
{
    ostream_iterator<complexd> out_it (cout, "\n");
    copy(psi.begin(), psi.end(), out_it);
}

void PrintMatrix(const vector< vector<complexd> >& U)
{
    cout << U[0][0] << "    " << U[0][1] << endl;
    cout << U[1][0] << "    " << U[1][1] << endl;
}
#endif

void ApplyOperator(vector<complexd>& psi,
    const vector< vector<complexd> >& U,
    const int k)
{
    const Index N = psi.size();
    const int n = intlog2(N);
    const Index mask = 1L << (n - k);
    
    #ifdef DEBUG
    cout << "before transform psi:" << endl;
    PrintVector(psi);
    cout << "Matrix U:" << endl;
    PrintMatrix(U);
    cout << "k = " << k << endl;
    #endif

    for (Index i = 0; i < N; i++)
    {
        // bit of i corresponding to target qubit
        if ((i & mask) == 0)
        {
            const Index i0 = i & ~ mask; // clear selected bit
            const Index i1 = i | mask; // set selected bit

            const complexd a = psi[i0];
            const complexd b = psi[i1];

            psi[i0] = U[0][0] * a + U[0][1] * b;
            psi[i1] = U[1][0] * a + U[1][1] * b;
        }
    }
    #ifdef DEBUG
    cout << "after transform psi:" << endl;
    PrintVector(psi);
    #endif
}

