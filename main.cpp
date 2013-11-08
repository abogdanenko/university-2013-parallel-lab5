/*
    Name: fidelity-mpi
    Author: Alexey Bogdanenko
    Contact: alex01@vpsbox.ru
    Date: Nov 2013
    Description: This program performs hadamard transform on each qubit of
        n-qubit system, then adds noise to the transform matrix and performs
        the transform again. The transform with noise is performed a number of
        times. The number of iterations is specified by the user.

        Only pure states are considered so system state is represented by a
        vector 2**n complex numbers long.

        First, result vector is computed by applying (noiseless) transform
        to initial vector. Then, on each iteration, a different result vector
        is computed by applying noisy transform to initial vector.

        On each iteration scalar product is taken of these two result vectors.
        Fidelity is computed as square of absolute value of the scalar product.

        See Parser::PrintUsage function for invocation information.

        This is program uses MPI to speed up each iteration and allow for
        larger vectors.

        The iterations are not performed in parallel.

*/

#include <mpi.h>
#include <cstdlib> // EXIT_FAILURE, EXIT_SUCCESS
#include <stdlib.h> // srand

#ifdef WAITFORGDB
#include "debug.h"
#endif

#include "computationbase.h"
#include "parser.h"
#include "remoteworker.h"
#include "master.h"
#include "routines.h"

using std::cerr;
using std::endl;

int main(int argc, char** argv)
{
    #ifdef WAITFORGDB
    WaitForGdb();
    #endif
    int rank;
    int world_size;
    int exit_code = EXIT_SUCCESS;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    srand(GetUniqueSeed());

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

