#include <mpi.h>
#include <fstream>

#include "master.h"

using std::ifstream;
using std::ofstream;
using std::istream;
using std::ostream;
using std::istream_iterator;
using std::ostream_iterator;

using std::cin;
using std::cout;
using std::endl;

Master::Master(const Args& args):
    ComputationBase(args),
    local_worker(args)
{
    #ifdef DEBUG
    params.PrintAll();
    #endif
}

Master::IdleWorkersError::IdleWorkersError():
    runtime_error("Too many processes for given number of qubits.")
{

}

void Master::MatrixReadFromFile()
{
    // read matrix from file or stdin
    ifstream fs;
    istream& s = (args.MatrixFileName() == "-") ? cin :
        (fs.open(args.MatrixFileName().c_str()), fs);
    
    vector<complexd> buf(4);

    s >> buf[0];
    s >> buf[1];
    s >> buf[2];
    s >> buf[3];

    U[0][0] = buf[0];
    U[0][1] = buf[1];
    U[1][0] = buf[2];
    U[1][1] = buf[3];

    local_worker.U = U;

    MPI_Bcast(&buf[0], buf.size() * sizeof(complexd), MPI_BYTE, master_rank,
        MPI_COMM_WORLD);
}

void Master::ForEachBufNoSplit(WorkerBufTransferOp op)
{
    for (int worker = 0; worker < params.WorkerCount(); worker++)
    {
        for (int i = 0; i < params.BufCount(); i++)
        {
            (this->*op)(worker);
        }
    }
}

void Master::ForEachBufSplit(WorkerBufTransferOp op)
{
    int worker = 0;
    for (int slice = 0; slice < params.SliceCount(); slice++)
    {
        for (int target_qubit_value = 0; target_qubit_value <= 1;
            target_qubit_value++)
        {
            for (int j = 0; j < params.WorkersPerSlice() / 2; j++)
            {
                for (int i = 0; i < params.BufCount() / 2; i++)
                {
                    (this->*op)(worker);
                }
                worker++;
            }
            if (target_qubit_value == 0)
            {
                worker -= params.WorkersPerSlice() / 2;
            }
        }
    }
}

void Master::ForEachBuf(WorkerBufTransferOp op)
{
    if (params.Split())
    {
        ForEachBufSplit(op);
    }
    else
    {
        ForEachBufNoSplit(op);
    }
}

void Master::ReceiveBufFromWorkerToOstream(const int worker)
{
    if (worker == 0)
    {
        local_worker.SendNextBuf();
    }

    vector<complexd> buf(params.BufSize());
    MPI_Request request;
    MPI_Irecv(&buf[0], buf.size() * sizeof(complexd), MPI_BYTE, worker,
        MPI_ANY_TAG, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE);
    *out_it = copy(buf.begin(), buf.end(), *out_it);
}

void Master::SendBufToWorkerFromIstream(const int worker)
{
    vector<complexd> buf(params.BufSize());
    // copy to buf from in_it
    for (vector<complexd>::iterator it = buf.begin(); it != buf.end(); it++)
    {
        *it = **in_it;
        (*in_it)++;
    }

    MPI_Request request;
    MPI_Isend(&buf[0], buf.size() * sizeof(complexd), MPI_BYTE, worker, tag,
        MPI_COMM_WORLD, &request);

    if (worker == 0)
    {
        local_worker.ReceiveNextBuf();
    }

    MPI_Wait(&request, MPI_STATUS_IGNORE);
}

void Master::VectorReadFromFile()
{
    // read state vector from file or stdin
    ifstream fs;
    istream& s = (args.VectorInputFileName() == "-") ? cin :
        (fs.open(args.VectorInputFileName().c_str()), fs);
    in_it = new istream_iterator<complexd>(s);

    ForEachBuf(&Master::SendBufToWorkerFromIstream);
    delete in_it;
}

void Master::VectorWriteToFile()
{
    ofstream fs;
    ostream& s = (args.VectorOutputFileName() == "-") ? cout :
        (fs.open(args.VectorOutputFileName().c_str()), fs);

    out_it = new ostream_iterator<complexd>(s, "\n");

    ForEachBuf(&Master::ReceiveBufFromWorkerToOstream);
    delete out_it;
}

void Master::ComputationTimeWriteToFile()
{
    ofstream fs;
    ostream& s = (args.ComputationTimeFileName() == "-") ? cout :
        (fs.open(args.ComputationTimeFileName().c_str()), fs);
    s << timer.GetDelta() << endl;
}

void Master::Run()
{
    MPI_Barrier(MPI_COMM_WORLD);
    timer.Start();
    if (args.MatrixReadFromFileFlag())
    {
        MatrixReadFromFile();    
    }
    if (args.VectorReadFromFileFlag())
    {
        VectorReadFromFile();
    }
    else
    {
        local_worker.InitRandom();
    }
    local_worker.ApplyOperator();
    if (args.VectorWriteToFileFlag())
    {
        VectorWriteToFile();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    timer.Stop();
    if (args.ComputationTimeWriteToFileFlag())
    {
        ComputationTimeWriteToFile();
    }
}

