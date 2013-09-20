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
#include <fstream>
#include <complex>
#include <iterator>
#include <vector>

using Eigen::Matrix2cd;
using Eigen::VectorXcd;
using std::ifstream;
using std::ofstream;
using std::istream_iterator;
using std::complex;
using std::string;
using std::vector;

typedef complex<double> complexd;

void Transform1Qubit(const VectorXcd& x, const Matrix2cd& U, const int k, VectorXcd& y)
{
    const int mask = 1 << k;
    for (int i = 0; i < x.size(); i++)
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
    Matrix2cd U; // transform matrix
    int k; // target qubit index

    // todo: parse command line arguments
    string x_filename = "vector-0";
    string y_filename = "result";
    string U_filename = "not";
    k = 0;

    // read x from file
    {
        // ifstream f(x_filename);
        ifstream f(x_filename.c_str());
        istream_iterator<complexd> start(f);
        istream_iterator<complexd> eos; // end of stream iterator
        vector<complexd> x_stl(start, eos);
        const int n = x_stl.size();
        x.resize(n);
        for (int i = 0; i < n; i++)
        {
            x(i) = x_stl[i];
        }
    }

    // read U from file
    {
        ifstream f(U_filename.c_str());
        f >> U(0, 0) >> U(0, 1) >> U(1, 0) >> U(1, 1);
    }

    y.resize(x.size());

    Transform1Qubit(x, U, k, y);

    // write y to file
    {
        ofstream f(y_filename.c_str());
        f << y;
    }

    return 0;
}
