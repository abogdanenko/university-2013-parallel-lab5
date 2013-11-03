#include <mpi.h>
#include <fstream>
#include <iterator>

#include "master.h"

#ifdef DEBUG
#include "debug.h"
#endif

using std::ofstream;
using std::ostream;
using std::ostream_iterator;

using std::cin;
using std::cout;
using std::endl;

Master::Master(const Args& args):
    ComputationBase(args),
    local_worker(args),
    fidelity(args.IterationCount())
{
    #ifdef DEBUG
    cout << "Master::Master()..." << endl;
    params.PrintAll();
    cout << "Master::Master() return" << endl;
    #endif
}

Master::IdleWorkersError::IdleWorkersError():
    runtime_error("Too many processes for given number of qubits.")
{

}

void Master::BroadcastMatrix()
{
    #ifdef DEBUG
    cout << IDENT(1) << "Master::BroadcastMatrix()..." << endl;
    #endif

    Vector buf = {
        U[0][0],
        U[0][1],
        U[1][0],
        U[1][1]
    };

    local_worker.U = U;

    MPI_Bcast(&buf[0], buf.size() * sizeof(complexd), MPI_BYTE, master_rank,
        MPI_COMM_WORLD);

    #ifdef DEBUG
    cout << IDENT(1) << "Master::BroadcastMatrix() return" << endl;
    #endif
}

void Master::ComputationTimeWriteToFile()
{
    ofstream fs;
    ostream& s = (args.ComputationTimeFileName() == "-") ? cout :
        (fs.open(args.ComputationTimeFileName().c_str()), fs);
    s << timer.GetDelta() << endl;
}

void Master::FidelityWriteToFile()
{
    ofstream fs;
    ostream& s = (args.FidelityFileName() == "-") ? cout :
        (fs.open(args.FidelityFileName().c_str()), fs);
    ostream_iterator<complexd> out_it (s, "\n");
    copy(fidelity.begin(), fidelity.end(), out_it);
}

void Master::Run()
{
    #ifdef DEBUG
    cout << "Master::Run()..." << endl;
    #endif

    MPI_Barrier(MPI_COMM_WORLD);
    timer.Start();

    local_worker.InitVector();
    local_worker.ApplyOperatorToEachQubit();
    local_worker.SaveNoiselessVector();
    for (auto it = fidelity.begin(); it != fidelity.end(); it++)
    {
        InitMatrix();
        BroadcastMatrix();
        local_worker.InitVector();
        local_worker.ApplyOperatorToEachQubit();
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

