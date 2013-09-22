/* 
    Name: transform-1-qubit
    Author: Alexey Bogdanenko
    Contact: alex01@vpsbox.ru
    Date: Sep 2013
    Description: This program performs a unitary transform on one qubit of
        n-qubit system. Only pure states are considered so system state is
        represented by a vector 2**n complex numbers long.

        See Transform1Qubit::PrintUsage function for invocation information.

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
#include <stdexcept>

using Eigen::Matrix2cd;
using Eigen::VectorXcd;
using std::ostream;
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

// for n = 2**m returns m
int intlog2(const int n)
{
    int result = 0;
    int t = n;
    while (t >>= 1)
      result++;
    return result;
}

int string_to_int(const string s)
{
    int n;
    stringstream ss(s);
    ss >> n;
    return n;
}

// If filename is "-", write obj to stdout, otherwise write obj to file.
template <class T>
void WriteToFileOrStdout(const T& obj, const string& filename)
{
    ostream* fp;
    ofstream fout;
    if (filename == "-")
    {
        fp = &cout;
    }
    else
    {
        fout.open(filename.c_str());
        fp = &fout;
    }
    *fp << obj << endl;
}

// returns normally distributed random number
double normal_random()
{
    /*
    Gaussian random variables are calculated as described here [1].
    [1] "http://en.wikipedia.org/wiki/Normal_distribution
    #Generating_values_from_normal_distribution"
    */
    const int n = 12;
    double sum = 0;
    for (int i = 0; i < n; i++)
    {
        sum += ((double) rand()) / RAND_MAX - 0.5;
    }
    return sum;
}

class Transform1Qubit
{
    VectorXcd x; // initial state vector
    VectorXcd y; // transformed state vector
    Matrix2cd U; // transform matrix
    int n; // number of qubits for random state, -1 means 'not specified'
    int k; // target qubit index
    int threads_count;
    double T; // ApplyOperator method computation time

    // NULL means 'not specified by user', "-" means 'write to stdout'
    char* x_filename;
    char* y_filename;
    char* U_filename;
    char* T_filename;

    public:
    class ParseError: public runtime_error
    {
        public:
        ParseError(string const& msg):
            runtime_error(msg)
        {
    
        }
    
    };
    
    Transform1Qubit();
    void ParseOptions(const int argc, char** const argv);
    void PrintUsage();
    void PrepareInputData();
    void ApplyOperator();
    void WriteResults();
};

Transform1Qubit::Transform1Qubit():
    U(Matrix2cd::Identity()), 
    n(-1), 
    k(1), 
    threads_count(1),
    T(-1.0),
    x_filename(NULL),
    y_filename(NULL),
    U_filename(NULL),
    T_filename(NULL)
{
    srand(time(NULL));
}


void Transform1Qubit::PrintUsage()
{
    cout << "Usage: transform-1-qubit [[-U operator_file] "
        "{-x state_vector_file | -n random_state_qubits_count} "
        "[-k target_qubit] [-t threads_count] [-y state_vector_output_file] "
        "[-T computation_time_output_file]]" << endl;
}

void Transform1Qubit::ApplyOperator()
{
    const int n = intlog2(x.size());
    const int mask = 1 << (n - k); // k-th most significant bit
    y.resize(x.size());
    for (int i = 0; i < x.size(); i++)
    {
        // bit of i corresponding to k-th qubit ("selected bit")
        const int i_k = i & mask ? 1 : 0; 
        const int i0 = i & ~ mask; // clear selected bit
        const int i1 = i | mask; // set selected bit
        y(i) = U(i_k, 0) * x(i0) + U(i_k, 1) * x(i1);
    }
}

void Transform1Qubit::ParseOptions(const int argc, char** const argv)
{
    ostringstream oss;
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
                oss << "Option -" << optopt << " requires an argument.";
                throw ParseError(oss.str());
            case '?':
                if (isprint(optopt))
                {
                    oss << "Unknown option `-" << optopt << "'.";
                }
                else
                {
                    oss << "Unknown option character `\\x" << hex << optopt << "'.";
                }
                throw ParseError(oss.str());
            default:
                throw ParseError("An error occured while parsing options.");
        }
    }

    if (optind > argc)
    {
        throw ParseError("Extra non-option arguments found");
    }

    if (x_filename == NULL && n == -1)
    {
        throw ParseError("State vector filename not specified and number of "
            "qubits for random state not specified");
    }
}

void Transform1Qubit::PrepareInputData()
{
    if (U_filename)
    {
        // read U from file
        ifstream f(U_filename);
        f >> U(0, 0) >> U(0, 1) >> U(1, 0) >> U(1, 1);
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
        /* set random n qubit state
        Pick a point on hypersphere as shown here [1]. 
        [1] http://mathworld.wolfram.com/HyperspherePointPicking.html
        */
        int N = 1;
        N <<= n; // N = 2**n;
        x.resize(N);
        for (int i = 0; i < N; i++)
        {
            x(i) = complexd(normal_random(), normal_random());
        }
        x.normalize();
    }

}

void Transform1Qubit::WriteResults()
{
    if (T_filename)
    {
        WriteToFileOrStdout(T, T_filename);
    }

    if (y_filename)
    {
        WriteToFileOrStdout(y, y_filename);
    }
}

int main(int argc, char** argv)
{
    Transform1Qubit t;
    try
    {
        if (argc == 1)
        {
            t.PrintUsage();
        }
        else
        {
            t.ParseOptions(argc, argv);
            t.PrepareInputData();
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

