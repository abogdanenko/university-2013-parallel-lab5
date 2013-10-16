#include <mpi.h>

#include "master.h"

Master::Master(const Args args):
    ComputationBase(args),
    local_worker(args)
{

}

Master::IdleWorkersError::IdleWorkersError()
    runtime_error("Too many processes for given number of qubits.")
{

}

void Master::YieldToLocalWorker()
{
    local_worker.Resume();
}

void Master::MatrixReadFromFile()
{
    // read matrix from file or stdin
    ifstream fs;
    istream& s = (args.MatrixFileName() == "-") ? cin :
        (fs.open(args.MatrixFileName().c_str()), fs);
    
    vector<complexd> buf(4);

    buf[0] << s;
    buf[1] << s;
    buf[2] << s;
    buf[3] << s;

    U[0][0] = buf[0];
    U[0][1] = buf[1];
    U[1][0] = buf[2];
    U[1][1] = buf[3];

    MPI_Request request = MPI_REQUEST_NULL;

    MPI_Isend(&buf[0], 4, MPI_DOUBLE_COMPLEX, 0, MPI_ANY_TAG, MPI_COMM_SELF,
        &request);
    YieldToLocalWorker();
    MPI_Bcast(&buf[0], 4, MPI_DOUBLE_COMPLEX, 0, MPI_COMM_WORLD);
}

template <class WorkerBufTransferOp>
void Master::ForEachBufNoSplit(WorkerBufTransferOp op)
{
    for (int worker = 0; worker < params.WorkerCount(); worker++)
    {
        for (int i = 0; i < params.BufCount(); i++)
        {
            op(worker);
        }
    }
}

template <class WorkerBufTransferOp>
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
                    op(worker);
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

template <class WorkerBufTransferOp>
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
    vector<complexd> buf(params.BufSize());
    // give control to local_worker so that he could send data
    if (worker == 0)
    {
        YieldToLocalWorker();
    }

    MPI_Status status;
    MPI_Request request = MPI_REQUEST_NULL;

    MPI_Irecv(&buf[0], buf.size(), MPI_DOUBLE_COMPLEX, worker, MPI_ANY_TAG,
        MPI_COMM_WORLD, &request);
    MPI_Wait(&request, &status);
    out_it = copy(buf.begin(), buf.end(), out_it);
}

void Master::SendBufToWorkerFromIstream(const int worker)
{
    vector<complexd> buf(params.BufSize());
    // copy to buf from in_it
    for (vector<complexd>::iterator it = buf.begin(); it != buf.end(); it++)
    {
        *it = *in_it;
        in_it++;
    }

    MPI_Status status;
    MPI_Request request = MPI_REQUEST_NULL;

    MPI_Isend(&buf[0], buf.size(), MPI_DOUBLE_COMPLEX, worker, MPI_ANY_TAG,
        MPI_COMM_WORLD, &request);
    MPI_Wait(&request, &status);

    // give control to local_worker so that he could receive data
    if (worker == 0)
    {
        YieldToLocalWorker();
    }
}

void Master::VectorReadFromFile()
{
    // read state vector from file or stdin
    ifstream fs;
    istream& s = (args.VectorInputFileName() == "-") ? cin :
        (fs.open(args.VectorInputFileName().c_str()), fs);
    in_it = istream_iterator<complexd>(s);

    ForEachBuf(SendBufToWorkerFromIstream);
}

void Master::VectorWriteToFile()
{
    ofstream fs;
    ostream& s = (args.VectorOutputFileName() == "-") ? cout :
        (fs.open(args.VectorOutputFileName().c_str()), fs);

    out_it = ostream_iterator<complexd> (s, "\n");

    ForEachBuf(ReceiveBufFromWorkerToOstream);
}

void Master::WriteComputationTime()
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

