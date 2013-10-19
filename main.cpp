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

#include <mpi.h>
#include <cstdlib> // EXIT_FAILURE, EXIT_SUCCESS

#include "computationbase.h"
#include "parser.h"
#include "remoteworker.h"
#include "master.h"

using std::cerr;
using std::endl;

int main(int argc, char** argv)
{
    int rank;
    int world_size;
    int exit_code = EXIT_SUCCESS;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    try
    {
        if (argc == 1)
        {
            if (rank == ComputationBase::master_rank)
            {
                Parser::PrintUsage();
            }
        }
        else
        {
            Parser parser(argc, argv);
            Args args = parser.Parse();
            const int vector_size = 1L << args.QubitCount();
            if (world_size * 2 > vector_size)
            {
                throw Master::IdleWorkersError();
            }
            if (rank == ComputationBase::master_rank)
            {
                Master master(args);
                master.Run();
            }
            else
            {
                RemoteWorker worker(args);
                worker.Run();
            }
        }
    }
    catch (Parser::ParseError& e)
    {
        if (rank == ComputationBase::master_rank)
        {
            cerr << e.what() << endl;
            Parser::PrintUsage();
        }
        exit_code = EXIT_FAILURE;
    }
    catch (Master::IdleWorkersError& e)
    {
        if (rank == ComputationBase::master_rank)
        {
            cerr << e.what() << endl;
        }
        exit_code = EXIT_FAILURE;
    }
    MPI_Finalize();
    return exit_code;
}

