#include <mpi.h>

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
    cout << IDENT(1) << "RemoteWorker::ReceiveMatrix()..." << endl;
    #endif
    Vector buf(4);

    MPI_Bcast(&buf[0], buf.size() * sizeof(complexd), MPI_BYTE, master_rank,
        MPI_COMM_WORLD);

    U[0][0] = buf[0];
    U[0][1] = buf[1];
    U[1][0] = buf[2];
    U[1][1] = buf[3];
    #ifdef DEBUG
    cout << IDENT(1) << "RemoteWorker::ReceiveMatrix() return" << endl;
    #endif
}

void RemoteWorker::Run()
{
    #ifdef DEBUG
    cout << "RemoteWorker::Run()..." << endl;
    #endif
    MPI_Barrier(MPI_COMM_WORLD);
    if (args.MatrixReadFromFileFlag())
    {
        ReceiveMatrix();
    }
    if (args.VectorReadFromFileFlag())
    {
        while (ReceiveNextBuf())
        {

        }
    }
    else
    {
        InitRandom();
    }
    ApplyOperator();
    if (args.VectorWriteToFileFlag())
    {
        while (SendNextBuf())
        {

        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    #ifdef DEBUG
    cout << "RemoteWorker::Run() return" << endl;
    #endif
}

