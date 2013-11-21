#include <dislib.h>

#ifdef DEBUG
#include "debug.h"
#endif

#include "remoteworker.h"

RemoteWorker::RemoteWorker(const Args& args):
    WorkerBase(args)
{

}

void RemoteWorker::ReceiveMatrix()
{
    #ifdef DEBUG
    cout << INDENT(1) << "RemoteWorker::ReceiveMatrix()..." << endl;
    #endif
    Vector buf(4);

    MPI_Bcast(&buf[0], buf.size() * sizeof(complexd), MPI_BYTE, master_rank,
        MPI_COMM_WORLD);

    U[0][0] = buf[0];
    U[0][1] = buf[1];
    U[1][0] = buf[2];
    U[1][1] = buf[3];
    #ifdef DEBUG
    cout << INDENT(1) << "RemoteWorker::ReceiveMatrix() return" << endl;
    #endif
}

void RemoteWorker::Run()
{
    #ifdef DEBUG
    cout << "RemoteWorker::Run()..." << endl;
    #endif
    shmem_barrier_all();
    {
        InitVectors();

        U = HadamardMatrix();
        ApplyOperatorToEachQubit();

        SwapVectors();

        ReceiveMatrix();
        ApplyOperatorToEachQubit();

        auto sp = ScalarProduct();
        vector<double> sendbuf = {sp.real(), sp.imag()};
        MPI_Reduce(&sendbuf.front(), NULL, 2, MPI_DOUBLE, MPI_SUM, master_rank,
            MPI_COMM_WORLD);
    }
    shmem_barrier_all();
    #ifdef DEBUG
    cout << "RemoteWorker::Run() return" << endl;
    #endif
}

