#include <mpi.h>

#include "basisvector1generator.h"
#include "computationbase.h"

BasisVector1Generator::BasisVector1Generator():
    first_call(true)
{
}

complexd BasisVector1Generator::operator()()
{
    complexd x(0.0, 0.0);

    if (first_call)
    {
        first_call = false;

        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        if (rank == ComputationBase::master_rank)
        {
            x = complexd(1.0, 0.0);
        }
    }

    return x;
}

