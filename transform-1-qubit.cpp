/* 
    Name: transform-1-qubit
    Author: Alexey Bogdanenko
    Contact: alex01@vpsbox.ru
    Date: Sep 2013
    Description: This program performs a unitary transform on one qubit of
        n-qubit system. Only pure states are considered so system state is
        represented by a vector 2**n complex numbers long.

*/

// disable Eigen's multi threading
#define EIGEN_DONT_PARALLELIZE

#include <Eigen/Eigen>

using Eigen::Matrix2cd;
using Eigen::VectorXcd;

// for n = 2**m returns m
int log2(const int n)
{
    int result = 0;
    int t = n;
    while (t >>= 1)
      result++;
    return result;
}

void Transform1Qubit(const VectorXcd& x, const Matrix2cd& U, const int k, VectorXcd& y)
{
    const int N = x.size();
    const int n = log2(N);
    const int mask = 1 << k;
    for (int i = 0; i < N; i++)
    {
        const int i_k = i & mask ? 1 : 0; // k-th bit of i
        const int i0 = i & ~ mask; // i with k-th bit set to 0
        const int i1 = i | mask; // i with k-th bit set to 1
        y[i] = U(i_k, 0) * x(i0) + U(i_k, 1) * x(i1);
    }
}

int main()
{
    VectorXcd x; // initial state vector
    VectorXcd y; // transformed state vector
    int k; // target qubit index
    Matrix2cd U; // transform matrix

    // read x, U from files

    y.resize(x.size());

    Transform1Qubit(x, U, k, y);

    // write y to file

    return 0;
}
