/* 
    Name: transform-1-qubit
    Author: Alexey Bogdanenko
    Contact: alex01@vpsbox.ru
    Date: Sep 2013
    Description: This program performs a unitary transform on one qubit of
        n-qubit system. Only pure states are considered so system state is
        represented by a vector 2**n complex numbers long.

        See PrintUsage function for invocation information.

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
#include <time.h>
#include <stdlib.h>

using Eigen::Matrix2cd;
using Eigen::VectorXcd;
using std::ifstream;
using std::ofstream;
using std::istream_iterator;
using std::complex;
using std::string;
using std::vector;
using std::stringstream;
using std::ostringstream;
using std::hex;
using std::cout;
using std::cerr;
using std::endl;
using std::runtime_error;

typedef complex<double> complexd;

void PrintUsage()
{
    cout << "Usage: transform-1-qubit [[-U operator_file] "
        "{-x state_vector_file | -n random_state_qubit_count} "
        "[-k target_qubit] [-t threads_count] [-y state_vector_output_file] "
        "[-T computation_time_output_file]]" << endl;
}

class ParseError: runtime_error
{
    public:
    ParseError(string const& msg):
        runtime_error(msg)
    {

    }

}

int string_to_int(const string s)
{
    int n;
    stringstream ss(s);
    ss >> n;
    return n;
}

// returns normally distributed random number
double normal_random()
{
   const int n = 12;
   double sum = 0;
   for (int i = 0; i < n; i++)
   {
       sum += ((double) rand()) / RAND_MAX - 0.5;
   }
   return sum;
}

void ApplyOperator()
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

void ParseOptions()
{
    // NULL means 'not specified by user'
    char* x_filename = NULL;
    char* y_filename = NULL;
    char* U_filename = NULL;
    char* T_filename = NULL;
  
    string msg;
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
                msg = string("Option -") + optopt + " requires an argument.";
                throw ParseError(msg);
            case '?':
                if (isprint(optopt))
                {
                    msg = string("Unknown option `-") + optopt + "'.";
                }
                else
                {
                    ostringstream oss;
                    oss << "Unknown option character `\\x" << hex << optopt << "'.";
                    msg = oss.str();
                }
                throw ParseError(msg);
            default:
                throw ParseError("An error occured while parsing options.");
        }
    }

    if (optind > argc)
    {
        msg = "Extra non-option arguments found";
        throw ParseError(msg);
    }

    if (x_filename == NULL && n == -1)
    {
        msg =  "State vector filename not specified and number of qubits for "
            "random state not specified";
        throw ParseError(msg);
    }
    return true;
}

int main(int argc, char** argv)
{
    Transform1Qubit t;
    try
    {
        t.ParseOptions(argc, argv);
        if (argc == 1)
        {
            t.PrintUsage();
        }
        else
        {
            t.InitStateVector();
            t.ApplyOperator();
            t.WriteResults();
        }
    }
    catch (Transform1Qubit::ParseError& e)
    {
        cerr << e.what() << endl;
        t.PrintUsage();
        return EXIT_FAILURE;

    }
    return EXIT_SUCCESS;
}

// extra code to be moved to Transform1Qubit class
{    
    double T = -1.0; // Transform1Qubit function computation time

    VectorXcd x; // initial state vector
    VectorXcd y; // transformed state vector
    int n = -1; // number of qubits for random state, -1 means 'not specified'
    // set default values
    Matrix2cd U = Matrix2cd::Identity(); // transform matrix
    int k = 0; // target qubit index
    int threads_count = 1;

    srand(time(NULL));

    parse_options(argc, argv, U_filename, x_filename, n, k, t, y_filename,
        T_filename);

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
        int N = 1;
        N <<= n; // N = 2**n;
        x.resize(N);
        for (int i = 0; i < N; i++)
        {
            x(i) = complexd(normal_random(), normal_random());
        }
        x.normalize();
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
