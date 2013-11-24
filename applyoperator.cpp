#include "applyoperator.h"
#include "routines.h"

#ifdef DEBUG
#include "debug.h"
#endif

#ifdef DEBUG
void PrintVector(const Vector& psi)
{
    for (auto x: psi)
    {
        cout << INDENT(5) << x << endl;
    }
}

void PrintMatrix(const Matrix& U)
{
    cout << INDENT(5) << U[0][0] << " " << U[0][1] << endl;
    cout << INDENT(5) << U[1][0] << " " << U[1][1] << endl;
}
#endif

void ApplyOperator(Vector& psi, const Matrix& U, const int k)
{
    const Index N = psi.size();
    const int n = intlog2(N);
    const Index mask = 1L << (n - k);

    #ifdef DEBUG
    cout << INDENT(3) << "ApplyOperator()..." << endl;
    cout << INDENT(4) << "psi:" << endl;
    PrintVector(psi);
    cout << INDENT(4) << "Matrix U:" << endl;
    PrintMatrix(U);
    cout << INDENT(4) << "target_qubit = " << k << endl;
    cout << INDENT(4) << "Applying operator..." << endl;
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
    cout << INDENT(4) << "Applying operator DONE" << endl;
    cout << INDENT(4) << "psi:" << endl;
    PrintVector(psi);
    cout << INDENT(3) << "ApplyOperator() return" << endl;
    #endif
}

