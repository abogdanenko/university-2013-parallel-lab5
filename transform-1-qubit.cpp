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
#include <algorithm>
#include <omp.h>

using std::ostream;
using std::istream;
using std::ifstream;
using std::ofstream;
using std::istream_iterator;
using std::ostream_iterator;
using std::complex;
using std::string;
using std::vector;
using std::stringstream;
using std::ostringstream;
using std::hex;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::runtime_error;
using std::copy;

typedef complex<double> complexd;
typedef vector<complexd>::size_type Index;

// for n = 2**m returns m
template <class Integer>
int intlog2(const Integer n)
{
    int result = 0;
    Integer t = n;
    while (t >>= 1)
      result++;
    return result;
}

int string_to_int(const string& s)
{
    int n;
    stringstream ss(s);
    ss >> n;
    return n;
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
    vector<complexd> x; // initial state vector
    vector<complexd> y; // transformed state vector
    vector< vector<complexd> > U; // transform matrix

    int n; // number of qubits for random state, -1 means 'not specified'
    int k; // target qubit index
    int threads_count;
    // computation time
    double start_time; 
    double end_time; 

    // NULL means 'not specified by user', "-" means 'write to stdout'
    char* x_filename;
    char* y_filename;
    char* U_filename;
    char* T_filename;

    public:
    class ParseError: public runtime_error
    {
        public:
        ParseError(string const& msg);
    };
    
    class ThreadError: public runtime_error
    {
        public:
        ThreadError();
    };

    Transform1Qubit();
    void ParseOptions(const int argc, char** const argv);
    void PrintUsage();
    void PrepareInputData();
    void ApplyOperator();
    void WriteResults();
    void TimerStart();
    void TimerStop();
};

Transform1Qubit::ParseError::ParseError(string const& msg):
    runtime_error(msg)
{

}

Transform1Qubit::ThreadError::ThreadError():
    runtime_error("Failed to spawn requested number of threads")
{

}
 
Transform1Qubit::Transform1Qubit():
    U(vector< vector<complexd> >(2, vector<complexd>(2))),
    n(-1), 
    k(1), 
    threads_count(1),
    x_filename(NULL),
    y_filename(NULL),
    U_filename(NULL),
    T_filename(NULL)
{
    srand(time(NULL));

    U[0][0] = 1;
    U[0][1] = 0;
    U[1][0] = 0;
    U[1][1] = 1;
}

void Transform1Qubit::PrintUsage()
{
    cout << "Usage: transform-1-qubit [[-U operator_file] "
        "{-x state_vector_file | -n random_state_qubits_count} "
        "[-k target_qubit] [-t threads_count] [-y state_vector_output_file] "
        "[-T computation_time_output_file]]" << endl;
}

void Transform1Qubit::TimerStart()
{
    start_time = omp_get_wtime();
}

void Transform1Qubit::TimerStop()
{
    end_time = omp_get_wtime();
}

void Transform1Qubit::ApplyOperator()
{
    const int n = intlog2(x.size());
    const Index mask = 1L << (n - k); // k-th most significant bit
    y.resize(x.size());
    const Index N = x.size();
    
    int runtime_threads;
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            runtime_threads = omp_get_num_threads();            
        }

        #pragma omp for
        for (Index i = 0; i < N; i++)
        {
            // bit of i corresponding to k-th qubit ("selected bit")
            const int i_k = i & mask ? 1 : 0; 
            const Index i0 = i & ~ mask; // clear selected bit
            const Index i1 = i | mask; // set selected bit
            y[i] = U[i_k][0] * x[i0] + U[i_k][1] * x[i1];
        }
    }
    
    if (runtime_threads < threads_count)
    {
        throw ThreadError();
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
                oss << "Option -" << char(optopt) << " requires an argument.";
                throw ParseError(oss.str());
            case '?':
                if (isprint(optopt))
                {
                    oss << "Unknown option `-" << char(optopt) << "'.";
                }
                else
                {
                    oss << "Unknown option character `\\x" << hex << optopt <<
                        "'.";
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
    omp_set_num_threads(threads_count);

    if (U_filename)
    {
        // read U
        ifstream fs;
        istream& s = (string(U_filename) == "-") ? cin :
            (fs.open(U_filename), fs);
        s >> U[0][0] >> U[0][1] >> U[1][0] >> U[1][1];
    }

    if (x_filename)
    {
        // read x
        ifstream fs;
        istream& s = (string(x_filename) == "-") ? cin :
            (fs.open(x_filename), fs);
        istream_iterator<complexd> start(s);
        istream_iterator<complexd> eos; // end of stream iterator

        x.assign(start, eos); // read all numbers
    }
    else
    {
        /* set random n qubit state
        Pick a point on hypersphere as shown here [1]. 
        [1] http://mathworld.wolfram.com/HyperspherePointPicking.html
        */
        Index N = 1L << n;
        x.resize(N);
        long double sum = 0.0;

        int runtime_threads;
        #pragma omp parallel
        {
            #pragma omp single nowait
            {
                runtime_threads = omp_get_num_threads();            
            }

            #pragma omp for
            for (Index i = 0; i < N; i++)
            {
                const complexd elem(normal_random(), normal_random());
                x[i] = elem;
            }
        }

        #pragma omp parallel
        {
            #pragma omp single nowait
            {
                runtime_threads = omp_get_num_threads();            
            }
   
            #pragma omp for reduction(+:sum)
            for (Index i = 0; i < N; i++)
            {
                sum += norm(x[i]);
            }
        }

        if (runtime_threads < threads_count)
        {
            throw ThreadError();
        }

        // Normalize x
        const double coef = 1.0 / sqrt(sum);
        #pragma omp parallel
        {
            #pragma omp single nowait
            {
                runtime_threads = omp_get_num_threads();            
            }

            #pragma omp for
            for (Index i = 0; i < N; i++)
            {
                x[i] *= coef;
            }
        }
   
        if (runtime_threads < threads_count)
        {
            throw ThreadError();
        }
    }
}

void Transform1Qubit::WriteResults()
{
    if (y_filename)
    {
        ofstream fs;
        ostream& s = (string(y_filename) == "-") ? cout :
            (fs.open(y_filename), fs);
        ostream_iterator<complexd> out_it (s, "\n");
        copy(y.begin(), y.end(), out_it);
    }

    if (T_filename)
    {
        ofstream fs;
        ostream& s = (string(T_filename) == "-") ? cout :
            (fs.open(T_filename), fs);
        TimerStop();
        s << end_time - start_time << endl;
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
            t.TimerStart();
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
    catch (Transform1Qubit::ThreadError& e)
    {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

