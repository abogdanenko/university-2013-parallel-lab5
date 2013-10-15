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

int main(int argc, char** argv)
{
    int rank;
    int exit_code = EXIT_SUCCESS;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    try
    {
        Parser parser(argc, argv);
        Parser::Args args = parser.Parse();
        const int vector_size = 1l << args.QubitCount();
        if (args.WorkerCount() * 2 > vector_size)
        {
            throw Master::IdleWorkersError();
        }
        if (rank == 0)
        {
            Master master(args);
            master.Run();
        }
        else
        {
            RemoteWorker worker(args.QubitCount(), args.TargetQubit());
            worker.Run();
        }
    }
    catch (Parser::ParseError& e)
    {
        if (rank == 0)
        {
            cerr << e.what() << endl;
            Parser::PrintUsage();
        }
        exit_code = EXIT_FAILURE;
    }
    catch (Master::IdleWorkersError()& e)
    {
        if (rank == 0)
        {
            cerr << e.what() << endl;
        }
        exit_code = EXIT_FAILURE;
    }
    MPI_Finalize();
    return exit_code;
}

