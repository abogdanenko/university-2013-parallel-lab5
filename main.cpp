/*
    Name: transform-each-qubit-shmem
    Author: Alexey Bogdanenko
    Contact: alex01@vpsbox.ru
    Date: Nov 2013
    Description: This program performs a unitary transform on each qubit of
        n-qubit system. The transform is obtained by adding noise to the matrix
        specified by user.

        Only pure states are considered so system state is represented by a
        vector 2**n complex numbers long.

        Input vector is generated randomly or read from file.

        The output vector is written to a file.

        See Parser::PrintUsage function for invocation information.

        This program uses a parallel algorithm to speed up the transform and
        allow for larger vectors.

        A specific shmem implementation (DISLIB) is used for communication
        between processes.
*/

#include <dislib.h>
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

