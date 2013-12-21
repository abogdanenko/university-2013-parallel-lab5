/*
    Name: fidelity-shmem
    Author: Alexey Bogdanenko
    Contact: alex01@vpsbox.ru
    Date: Dec 2013
    Description: This program performs hadamard transform on each qubit of
        n-qubit system, then adds noise and performs the transform again. The
        transform is performed a number of times. The number of iterations is
        specified by user. Initial state is randomly generated.

        Only pure states are considered so system state is represented by a
        vector 2**n complex numbers long.

        On each iteration scalar product is taken of the two result vectors.
        Fidelity is computed as square of absolute value of the scalar product.

        See Parser::PrintUsage function for invocation information.

        This program uses a parallel algorithm to speed up the transform and
        allow for larger vectors.

        A specific shmem implementation (DISLIB) is used for communication
        between processes.
*/

#include <dislib.h>
#include <cstdlib> // EXIT_FAILURE, EXIT_SUCCESS
#include <stdlib.h> // srand
#include <iostream> // std::cout, std::cerr

#ifdef WAITFORGDB
#include "debug.h"
#endif

#include "computationbase.h"
#include "parser.h"
#include "remoteworker.h"
#include "master.h"
#include "routines.h"
#include "shmem.h"

using std::cerr;
using std::endl;

int main(int argc, char** argv)
{
    #ifdef WAITFORGDB
    WaitForGdb();
    #endif
    int exit_code = EXIT_SUCCESS;

    shmem_init(&argc, &argv);
    shmem_register_handler(ShmemReceiveElem, Shmem::HandlerNumber());

    srand(GetUniqueSeed());

    try
    {
        if (argc == 1)
        {
            if (shmem_my_pe() == ComputationBase::master_rank)
            {
                Parser::PrintUsage();
            }
        }
        else
        {
            Parser parser(argc, argv);
            Args args = parser.Parse();
            const int vector_size = 1L << args.QubitCount();
            if (shmem_n_pes() * 2 > vector_size)
            {
                throw Master::IdleWorkersError();
            }
            if (shmem_my_pe() == ComputationBase::master_rank)
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
        if (shmem_my_pe() == ComputationBase::master_rank)
        {
            cerr << e.what() << endl;
            Parser::PrintUsage();
        }
        exit_code = EXIT_FAILURE;
    }
    catch (Master::IdleWorkersError& e)
    {
        if (shmem_my_pe() == ComputationBase::master_rank)
        {
            cerr << e.what() << endl;
        }
        exit_code = EXIT_FAILURE;
    }
    shmem_finalize();
    return exit_code;
}
