#include <mpi.h>
#include <fstream>

#include "master.h"
#include "routines.h"
#include "normaldistributiongenerator.h"

#ifdef DEBUG
#include "debug.h"
#endif

using std::ofstream;
using std::ostream;

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

void Master::InitMatrix()
{
    #ifdef DEBUG
    cout << INDENT(1) << "Master::InitMatrix()..." << endl;
    #endif

    NormalDistributionGenerator gen;
    const double xi = gen();
    const double theta = args.Epsilon() * xi;
    const complexd c = cos(theta);
    const complexd s = sin(theta);
    const Vector row1 = {c, s};
    const Vector row2 = {-1.0 * s, c};
    const Matrix U_theta = {row1, row2};

    U = MatrixMultiply(HadamardMatrix(), U_theta);

    #ifdef DEBUG
    cout << INDENT(2) << "xi = " << xi << endl;
    cout << INDENT(2) << "theta = " << theta << endl;
    #endif

    #ifdef DEBUG
    cout << INDENT(1) << "Master::InitMatrix() return" << endl;
    #endif
}

void Master::BroadcastMatrix()
{
    #ifdef DEBUG
    cout << INDENT(1) << "Master::BroadcastMatrix()..." << endl;
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
    cout << INDENT(1) << "Master::BroadcastMatrix() return" << endl;
    #endif
}

void Master::ComputationTimeWriteToFile()
{
    ofstream fs;
    ostream& s = (args.ComputationTimeFileName() == "-") ? cout :
        (fs.open(args.ComputationTimeFileName().c_str()), fs);
    s << timer_total.Total() << endl;
    s << timer_init.Total() << endl;
    s << timer_transform.Total() << endl;
}

void Master::OneMinusFidelityWriteToFile()
{
    ofstream fs;
    ostream& s = (args.FidelityFileName() == "-") ? cout :
        (fs.open(args.FidelityFileName().c_str()), fs);
    for (auto it = fidelity.begin(); it != fidelity.end(); it++)
    {
        s << 1.0 - *it << endl;
    }
}

void Master::Run()
{
    #ifdef DEBUG
    cout << "Master::Run()..." << endl;
    #endif

    timer_total.Start();

    for (auto it = fidelity.begin(); it != fidelity.end(); it++)
    {
        timer_init.Start();
        local_worker.InitVectors();
        timer_init.Stop();

        local_worker.U = HadamardMatrix();

        timer_transform.Start();
        local_worker.ApplyOperatorToEachQubit();
        timer_transform.Stop();

        local_worker.SwapVectors();

        InitMatrix();
        BroadcastMatrix();

        timer_transform.Start();
        local_worker.ApplyOperatorToEachQubit();
        timer_transform.Stop();

        auto s = local_worker.ScalarProduct();
        vector<double> sum = {s.real(), s.imag()};
        MPI_Reduce(MPI_IN_PLACE, &sum.front(), 2, MPI_DOUBLE, MPI_SUM,
            master_rank, MPI_COMM_WORLD);
        *it = norm(complexd (sum[0], sum[1]));
    }

    timer_total.Stop();

    if (args.FidelityWriteToFileFlag())
    {
        OneMinusFidelityWriteToFile();
    }

    if (args.ComputationTimeWriteToFileFlag())
    {
        ComputationTimeWriteToFile();
    }

    #ifdef DEBUG
    cout << "Master::Run() return" << endl;
    #endif
}
