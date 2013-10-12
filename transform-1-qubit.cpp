/* 
    Name: transform-1-qubit
    Author: Alexey Bogdanenko
    Contact: alex01@vpsbox.ru
    Date: Oct 2013
    Description: This program performs a unitary transform on one qubit of
        n-qubit system. Only pure states are considered so system state is
        represented by a vector 2**n complex numbers long.

        See Parser::PrintUsage function for invocation information.

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

class BaseWorker
{
    /* Slice of initial state vector, split into two halves. First half
       corresponds to k-th qubit state 0, second half corresponds to target
       qubit state 1. The vector is modified in-place, so result is also stored
       here  */
    vector<complexd> x; 
    vector< vector<complexd> > U; // transform matrix

    public:
    ApplyOperator();
};

class RemoteWorker: public BaseWorker
{
    public:
    static const int GO_AHEAD = 0;
    Run();
    WaitForGoAheadOrAbort();
    ReceiveInstructions();
    ReceiveInputData();
    SendResults();
};

class LocalWorker: public BaseWorker
{
    public:
    Init(n, k);
    SendNextBuf();
}

class Master
{
    LocalWorker local_worker;
    Parser::Args args; // parsed program arguments
    Timer timer; // measure computation time

    public:
   
    void Init(const Parser::Args &args);
    void Run();
    void AbortWorkers();
    void DistributeInputData();
    void ManageLocalWorker();
    void ReceiveAndWriteResults();
    void WriteComputationTime();
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

Worker::Worker():
    U(vector< vector<complexd> >(2, vector<complexd>(2)))
{
    srand(time(NULL));
}

class RandomComplexGenerator()
{
    unsigned m_w;
    unsigned m_z;
    inline double random01d()
    public:
    RandomComplexGenerator();
    static complexd operator()();
};

/*
    Pseudo-random number generator

    Returns real numbers between 0 and 1
    copied from
    http://en.wikipedia.org/wiki/Random_number_generation#cite_ref-6
*/
double RandomComplexGenerator::random01d()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    unsigned r = (m_z << 16) + m_w;  // 32-bit result 
    return double (r) / 0xffffffff;
}

RandomComplexGenerator::RandomComplexGenerator():
    next_seed1(rand()),
    next_seed2(rand())
{
}

complexd RandomComplexGenerator::operator()()
{
    double re = random01d() - 0.5;
    double im = random01d() - 0.5;
    return complexd(re, im);
}

void BaseWorker::InitRandom()
{
    x.resize(N);
    RandomComplexGenerator gen;
    generate(x.begin(), x.end(), gen);

    // normalize
    long double sum = 0.0;
    for (vector<complexd>::const_iterator it = x.begin(); it != x.end(); it++)
    {
        sum += norm(*it);
    }
    MPI_Reduce_send(sum);
    long double coef;
    MPI_Broadcast_receive(coef);
    // "x = coef * x"
    transorm(x.begin(), x.end(), x.begin(),
        bind1st(multiplies<complexd>(), (complexd) coef));
}

void BaseWorker::ApplyOperator()
{
    const Index N = x.size() / 2;
    for (Index i = 0; i < N; i++)
    {
        const Index j = i + N;
        const complexd a = x[i];
        const complexd b = x[j];
        x[i] = U[0][0] * a + U[0][1] * b;
        x[j] = U[1][0] * a + U[1][1] * b;
    }
}

void Master::PrepareOperator()
{
    if (U_filename)
    {
        // read U from file or stdin
        ifstream fs;
        istream& s = (string(U_filename) == "-") ? cin :
            (fs.open(U_filename), fs);
        s >> U[0][0] >> U[0][1] >> U[1][0] >> U[1][1];
    }
    else
    {
        // Assign identity matrix to U
        U[0][0] = 1;
        U[0][1] = 0;
        U[1][0] = 0;
        U[1][1] = 1;
    }
}

void Master::DistributeInputData()
{
    // transform matrix
    vector< vector<complexd> > U(
        vector< vector<complexd> >(2, vector<complexd>(2)))
    PrepareOperator(U);

    if (x_filename)
    {
        // read x from file or stdin
        ifstream fs;
        istream& s = (string(x_filename) == "-") ? cin :
            (fs.open(x_filename), fs);
        istream_iterator<complexd> in_it(s);
        if (split_slices_between_workers)
        {
            ReadAndSendSplit(in_it);
        }
        else
        {
            ReadAndSendNoSplit(in_it);
        }
    }
    else
    {
        // set random n qubit state
        MPI_Reduce_receive(sum);

        // Normalize x
        const double coef = 1.0 / sqrt(sum);
        MPI_Broadcast_send(coef);
    }
}

void Master::ReceiveAndWriteSplit(const ostream_iterator<complexd>& f)
{
    for (int slice = 0; slice < slices_count; slice++)
    {
        // we get the same peer twice
        for (int peer = FirstPeer(slice); peer < EndPeer(slice); peer++)
        {
            for (int i = 0; i < bufs_per_peer / 2; j++)
            {
                if (peer == 0)
                {
                    local_worker.SendNextBuf();
                }
                MPI_Irecv(buf, peer);
                MPI_Wait();
                copy(buf.begin(), buf.end(), out_it);
            }
        }
    }
}

void Master::ReceiveAndWriteNoSplit(const ostream_iterator<complexd>& f)
{
    for (int peer = 0; peer < np; peer++)
    {
        if (peer == 0)
        {
            local_worker.SendNextBuf();
        }
        for (i = 0; i < bufs_per_peer_count; i++)
        {
            MPI_Irecv(buf, peer);
            MPI_Wait();
            copy(buf.begin(), buf.end(), out_it);
        }
    }
}

void Master::ReceiveAndWriteResults()
{
    if (y_filename)
    {
        ofstream fs;
        ostream& s = (string(y_filename) == "-") ? cout :
            (fs.open(y_filename), fs);
        ostream_iterator<complexd> out_it (s, "\n");

        if (split_slices_between_workers)
        {
            ReceiveAndWriteSplit(out_it);
        }
        else
        {
            ReceiveAndWriteNoSplit(out_it);
        }
    }
}

void Master::WriteComputationTime()
{
    if (T_filename)
    {
        ofstream fs;
        ostream& s = (string(T_filename) == "-") ? cout :
            (fs.open(T_filename), fs);
        s << timer.GetDelta() << endl;
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
        // implicit barrier here to measure time consistently
        SendBlockingGoAhead();
        // All processes have started and are standing by
        timer.Start();
        DistributeInputData();
        ManageLocalWorker();
        ReceiveAndWriteResults();
        // implicit barrier here to measure time consistently
        WaitForRemoteWorkers();
        timer.Stop();
        WriteComputationTime();
    }
}

void RemoteWorker::Run()
{
    if (WaitForGoAheadOrAbort() == GO_AHEAD)
    {
        ReceiveInputData();
        ApplyOperator();
        SendResults();
        SendCompletionNotification();
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
        RemoteWorker worker();
        worker.Run();
    }
    MPI_Finalize();
    return exit_code;
}

