/* 
    Name: transform-1-qubit
    Author: Alexey Bogdanenko
    Contact: alex01@vpsbox.ru
    Date: Oct 2013
    Description: This program performs a unitary transform on one qubit of
        n-qubit system. Only pure states are considered so system state is
        represented by a vector 2**n complex numbers long.

        See Transform1Qubit::PrintUsage function for invocation information.

        This is program uses MPI to speed up computation and allow for larger
        input.

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

/*
    Pseudo-random number generator

    Returns real numbers between 0 and 1
    copied from
    http://en.wikipedia.org/wiki/Random_number_generation#cite_ref-6
*/
inline double random01d(unsigned& m_w, unsigned& m_z)
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    unsigned r = (m_z << 16) + m_w;  /* 32-bit result */
    return double (r) / 0xffffffff;
}

class Parser
{
    const int argc;
    char** const argv;

    public:

    class Args
    {
        int n; // number of qubits for random state, -1 means 'not specified'
        int k; // target qubit index
        // NULL means 'not specified by user', "-" means 'write to stdout'
        char* x_filename;
        char* y_filename;
        char* U_filename;
        char* T_filename;
    };

    class ParseError: public runtime_error
    {
        public:
        ParseError(string const& msg);
    };

    Parser(const int argc, char** const argv);
    Args Parse();
    static void PrintUsage();
};


class Transform1Qubit
{
    vector<complexd> x; // initial state vector
    vector<complexd> y; // transformed state vector
    vector< vector<complexd> > U; // transform matrix

    Parser::Args args; // parsed program arguments

    // computation time
    double start_time; 
    double end_time; 

    public:
   
    Transform1Qubit();
    void Init(const Parser::Args &args);
    void PrepareInputData();
    void ApplyOperator();
    void WriteResults();
    void TimerStart();
    void TimerStop();
};

Parser::ParseError::ParseError(string const& msg):
    runtime_error(msg)
{

}

Parser::Args::Args():
    n(-1), 
    k(1), 
    x_filename(NULL),
    y_filename(NULL),
    U_filename(NULL),
    T_filename(NULL)
{

}

void Parser::PrintUsage()
{
    cout << "Usage: transform-1-qubit [[-U operator_file] "
        "{-x state_vector_file | -n random_state_qubits_count} "
        "[-k target_qubit] [-y state_vector_output_file] "
        "[-T computation_time_output_file]]" << endl;
}

Parser::Args Parser::Parse()
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

Transform1Qubit::Transform1Qubit():
    U(vector< vector<complexd> >(2, vector<complexd>(2)))
{
    srand(time(NULL));

    U[0][0] = 1;
    U[0][1] = 0;
    U[1][0] = 0;
    U[1][1] = 1;
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
    
    #pragma omp parallel
    {
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
    
}

void Transform1Qubit::PrepareInputData()
{

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
        // set random n qubit state

        Index N = 1L << n;
        x.resize(N);
        long double sum = 0.0;

        #pragma omp parallel
        {
            unsigned next_seed1 = rand();
            unsigned next_seed2 = rand();
   
            #pragma omp for 
            for (Index i = 0; i < N; i++)
            {
                double re = random01d(next_seed1, next_seed2) - 0.5;
                double im = random01d(next_seed1, next_seed2) - 0.5;
                x[i] = complexd(re, im);
            }
    
        }

        #pragma omp parallel
        {
  
            #pragma omp for reduction(+:sum)
            for (Index i = 0; i < N; i++)
            {
                sum += norm(x[i]);
            }
        }

        // Normalize x
        const double coef = 1.0 / sqrt(sum);
        #pragma omp parallel
        {
            #pragma omp for
            for (Index i = 0; i < N; i++)
            {
                x[i] *= coef;
            }
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

void Master::Run()
{
    if (argc == 1)
    {
        Parser::PrintUsage();
        AbortWorkers();
    }
    else
    {
        Timer.Start();
        DistributeInputData();
        ManageLocalWorker();
        ReceiveAndWriteResults();
        Timer.Stop();
        WriteComputationTime();
    }
}

void Worker::Run()
{
    if (WaitForGoAhead())
    {
        ReceiveInstructions();
        ReceiveInputData();
        ApplyOperator();
        SendResults();
    }
}

int main(int argc, char** argv)
{
    int rank;
    int exit_code = EXIT_SUCCESS;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        Parser parser(argc, argv);
        Master master();
        try
        {
            Parser::Args args = parser.Parse();
            master.Init(args);
            master.Run();
        }
        catch (Parser::ParseError& e)
        {
            cerr << e.what() << endl;
            Parser::PrintUsage();
            master.AbortWorkers();
            exit_code = EXIT_FAILURE;
        }
    }
    else
    {
        Worker worker();
        worker.Run();
    }
    MPI_Finalize();
    return exit_code;
}

