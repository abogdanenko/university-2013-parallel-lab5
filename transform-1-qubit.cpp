/* 
    Name: transform-1-qubit
    Author: Alexey Bogdanenko
    Contact: alex01@vpsbox.ru
    Date: Sep 2013
    Description: This program performs a unitary transform on one qubit of
        n-qubit system. Only pure states are considered so system state is
        represented by a vector 2**n complex numbers long.

        See 'usage' string in main for invocation information.

*/

// disable Eigen's multi threading
#define EIGEN_DONT_PARALLELIZE

#include <Eigen/Eigen>
#include <fstream>
#include <complex>
#include <iterator>
#include <vector>
#include <unistd.h>
#include <ctype.h>
#include <iomanip>
#include <iostream>

using Eigen::Matrix2cd;
using Eigen::VectorXcd;
using std::ifstream;
using std::ofstream;
using std::istream_iterator;
using std::complex;
using std::string;
using std::vector;
using std::stringstream;
using std::hex;
using std::cout;
using std::cerr;
using std::endl;

typedef complex<double> complexd;

int string_to_int(const string s)
{
    int n;
    stringstream ss(s);
    ss >> n;
    return n;
}

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

int main(int argc, char** argv)
{
    string usage = string("Usage: ") + argv[0] + " [[-U operator_file] "
        "{-x state_vector_file | -n random_state_qubit_count} "
        "[-k target_qubit] [-t threads_count] [-y state_vector_output_file] "
        "[-T computation_time_output_file]]";

    double T = -1.0; // Transform1Qubit function computation time

    VectorXcd x; // initial state vector
    VectorXcd y; // transformed state vector
    int n = -1; // number of qubits for random state, -1 means 'not specified'
    // set default values
    Matrix2cd U = Matrix2cd::Identity(); // transform matrix
    int k = 0; // target qubit index
    int threads_count = 1;

    // parse options
    if (argc == 1)
    {
        cout << usage << endl;
    }

    // NULL means 'not specified by user'
    char* x_filename = NULL;
    char* y_filename = NULL;
    char* U_filename = NULL;
    char* T_filename = NULL;
  
    int c; // option character
    while ((c = getopt(argc, argv, ":U:x:n:k:t:y:T:")) != -1)
    {
        switch(c)
        {
            case 'U':
                U_filename = optarg;
                break;
            case 'x':
                x_filename = optarg;
                break;
            case 'n':
                n = string_to_int(optarg);
                break;
            case 'k':
                k = string_to_int(optarg);
                break;
            case 't':
                threads_count = string_to_int(optarg);
                break;
            case 'y':
                y_filename = optarg;
                break;
            case 'T':
                T_filename = optarg;
                break;
            case ':':
                cerr << "Option -" << optopt << " requires an argument."
                    << endl; 
                cout << usage << endl;
                return 1;
            case '?':
                if (isprint(optopt))
                {
                    cerr << "Unknown option `-" << optopt << "'." << endl;
                }
                else
                {
                    cerr << "Unknown option character `\\x" << hex << optopt <<
                        "'." << endl;
                }
                cout << usage << endl;
                return 1;
            default:
                cout << usage << endl;
                return 1;
        }
    }

    if (optind > argc)
    {
        cerr << "Extra non-option arguments found" << endl;
        cout << usage << endl;
        return 1;
    }

    if (x_filename == NULL && n == -1)
    {
        cerr << "State vector filename not specified and number of qubits for "
            "random state not specified" << endl;
        cout << usage << endl;
        return 1;
    }

    if (x_filename)
    {
        // read x from file
        ifstream f(x_filename);
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
    else
    {
        // set random n qubit state
        cerr << "not implemented" << endl;
        return 1;
    }

    if (U_filename)
    {
        // read U from file
        ifstream f(U_filename);
        f >> U(0, 0) >> U(0, 1) >> U(1, 0) >> U(1, 1);
    }

    y.resize(x.size());

    Transform1Qubit(x, U, k, y);

    if (y_filename)
    {
        // write y to file
        ofstream f(y_filename);
        f << y << endl;
    }

    if (T_filename)
    {
        // write T to file
        ofstream f(T_filename);
        f << T << endl;
    }

    return 0;
}
