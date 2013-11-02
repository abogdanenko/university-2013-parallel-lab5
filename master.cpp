#include <mpi.h>
#include <fstream>

#include "master.h"

#ifdef DEBUG
#include "debug.h"
#endif

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
    cout << "Master::Master()..." << endl;
    #endif
    #ifdef DEBUG
    params.PrintAll();
    #endif
    fidelity.resize(params.IterationCount());
    #ifdef DEBUG
    cout << "Master::Master() return" << endl;
    #endif
}

Master::IdleWorkersError::IdleWorkersError():
    runtime_error("Too many processes for given number of qubits.")
{

}

void Master::MatrixReadFromFile()
{
    #ifdef DEBUG
    cout << IDENT(1) << "Master::MatrixReadFromFile()..." << endl;
    #endif
    // read matrix from file or stdin
    ifstream fs;
    istream& s = (args.MatrixFileName() == "-") ? cin :
        (fs.open(args.MatrixFileName().c_str()), fs);

    Vector buf(4);

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
    #ifdef DEBUG
    cout << IDENT(1) << "Master::MatrixReadFromFile() return" << endl;
    #endif
}

void Master::ForEachBufNoSplit(WorkerBufTransferOp op)
{
    for (int worker = 0; worker < params.WorkerCount(); worker++)
    {
        for (int i = 0; i < params.BufCount(); i++)
        {
            #ifdef DEBUG
            cout << IDENT(2) << "Transfer with worker " << worker << "..."
                << endl;
            #endif
            (this->*op)(worker);
            #ifdef DEBUG
            cout << IDENT(2) << "Transfer with worker " << worker << " DONE"
                << endl;
            #endif
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
            for (int j = 0; j < params.WorkersPerSlice(); j++)
            {
                for (int i = 0; i < params.BufCount() / 2; i++)
                {
                    #ifdef DEBUG
                    cout << IDENT(2) << "Transfer with worker " << worker
                        << "..." << endl;
                    #endif
                    (this->*op)(worker);
                    #ifdef DEBUG
                    cout << IDENT(2) << "Transfer with worker " << worker
                        << " DONE" << endl;
                    #endif
                }
                worker++;
            }
            if (target_qubit_value == 0)
            {
                worker -= params.WorkersPerSlice();
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

    Vector buf(params.BufSize());
    MPI_Request request;
    MPI_Irecv(&buf[0], buf.size() * sizeof(complexd), MPI_BYTE, worker,
        MPI_ANY_TAG, MPI_COMM_WORLD, &request);

    MPI_Wait(&request, MPI_STATUS_IGNORE);

    *out_it = copy(buf.begin(), buf.end(), *out_it);
}

void Master::SendBufToWorkerFromIstream(const int worker)
{
    #ifdef DEBUG
    cout << IDENT(3) << "Master::SendBufToWorkerFromIstream()..." << endl;
    #endif

    Vector buf(params.BufSize());
    // copy to buf from in_it
    for (auto it = buf.begin(); it != buf.end(); it++)
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

    #ifdef DEBUG
    cout << IDENT(3) << "Master::SendBufToWorkerFromIstream() return" << endl;
    #endif
}

void Master::VectorReadFromFile()
{
    #ifdef DEBUG
    cout << IDENT(1) << "Master::VectorReadFromFile()..." << endl;
    #endif
    // read state vector from file or stdin
    ifstream fs;
    istream& s = (args.VectorInputFileName() == "-") ? cin :
        (fs.open(args.VectorInputFileName().c_str()), fs);
    in_it = new istream_iterator<complexd>(s);

    ForEachBuf(&Master::SendBufToWorkerFromIstream);
    delete in_it;
    #ifdef DEBUG
    cout << IDENT(1) << "Master::VectorReadFromFile() return" << endl;
    #endif
}

void Master::VectorWriteToFile()
{
    #ifdef DEBUG
    cout << IDENT(1) << "Master::VectorWriteToFile()..." << endl;
    #endif
    ofstream fs;
    ostream& s = (args.VectorOutputFileName() == "-") ? cout :
        (fs.open(args.VectorOutputFileName().c_str()), fs);

    out_it = new ostream_iterator<complexd>(s, "\n");

    ForEachBuf(&Master::ReceiveBufFromWorkerToOstream);
    delete out_it;
    #ifdef DEBUG
    cout << IDENT(1) << "Master::VectorWriteToFile() return" << endl;
    #endif
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
    #ifdef DEBUG
    cout << "Master::Run()..." << endl;
    #endif
    MPI_Barrier(MPI_COMM_WORLD);
    timer.Start();

    local_worker.InitVector();
    local_worker.ApplyOperator();
    local_worker.SaveNoiselessVector();
    for (auto it = fidelity.begin(); it != fidelity.end(); it++)
    {
        InitMatrix();
        BroadcastMatrix();
        local_worker.InitVector();
        local_worker.ApplyOperator();
        auto s = local_worker.ScalarProduct();
        vector<double> sum = {s.real(), s.imag()};
        MPI_Reduce(MPI_IN_PLACE, &sum.front(), 2, MPI_DOUBLE, MPI_SUM,
            master_rank, MPI_COMM_WORLD);
        *it = (complexd (sum[0], sum[1])).norm();
    }
    if (args.FidelityWriteToFileFlag())
    {
        FidelityWriteToFile();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    timer.Stop();
    if (args.ComputationTimeWriteToFileFlag())
    {
        ComputationTimeWriteToFile();
    }
    #ifdef DEBUG
    cout << "Master::Run() return" << endl;
    #endif
}

