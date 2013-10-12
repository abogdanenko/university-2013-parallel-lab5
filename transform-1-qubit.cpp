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

