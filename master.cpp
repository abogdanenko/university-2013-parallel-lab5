#include <dislib.h>
#include <fstream>
#include <iostream> // std::cin, std::cout

#include "master.h"
#include "routines.h"
#include "normaldistributiongenerator.h"
#include "shmem.h"
#include "stats.h"

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
    fidelity(args.IterationCount()),
    local_worker(args)
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

void Master::AddNoiseToMatrix()
{
    #ifdef DEBUG
    cout << INDENT(1) << "Master::AddNoiseToMatrix()..." << endl;
    #endif

    NormalDistributionGenerator gen;
    const double xi = gen();
    const double theta = args.Epsilon() * xi;
    const complexd c = cos(theta);
    const complexd s = sin(theta);
    Matrix U_theta(2, Vector(2));

    U_theta [0][0] = c;
    U_theta [0][1] = s;
    U_theta [1][0] = -1.0 * s;
    U_theta [1][1] = c;

    U = MatrixMultiply(U, U_theta);

    #ifdef DEBUG
    cout << INDENT(2) << "xi = " << xi << endl;
    cout << INDENT(2) << "theta = " << theta << endl;
    #endif

    #ifdef DEBUG
    cout << INDENT(1) << "Master::AddNoiseToMatrix() return" << endl;
    #endif
}

void Master::BroadcastMatrix()
{
    #ifdef DEBUG
    cout << INDENT(1) << "Master::BroadcastMatrix()..." << endl;
    #endif

    local_worker.U = U;

    for (auto row: U)
    {
        for (auto elem: row)
        {
            vector<double> complex_array(2);
            complex_array[0] = elem.real();
            complex_array[1] = elem.imag();
            for (auto x: complex_array)
            {
                shmem_double_toall(&x, master_rank);
            }
        }
    }

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

void Master::StatsWriteToFile()
{
    ofstream fs;
    ostream& s = (args.StatsFileName() == "-") ? cout :
        (fs.open(args.StatsFileName().c_str()), fs);
    s << Stats::SendOpCounter() * shmem_n_pes() << endl;
    s << Stats::SendDataCounter() * shmem_n_pes() << endl;
}

void Master::OneMinusFidelityWriteToFile()
{
    ofstream fs;
    ostream& s = (args.FidelityFileName() == "-") ? cout :
        (fs.open(args.FidelityFileName().c_str()), fs);
    for (auto x: fidelity)
    {
        s << 1.0 - x << endl;
    }
}

void Master::Run()
{
    #ifdef DEBUG
    cout << "Master::Run()..." << endl;
    #endif

    Stats::ResetCounters();

    timer_total.Start();

    for (auto& f: fidelity)
    {
        timer_init.Start();
        local_worker.VectorInitRandom();
        timer_init.Stop();

        U = HadamardMatrix();
        local_worker.U = U;

        timer_transform.Start();
        local_worker.ApplyOperatorToEachQubit();
        timer_transform.Stop();

        local_worker.SwapVectors();

        AddNoiseToMatrix();
        BroadcastMatrix();

        timer_transform.Start();
        local_worker.ApplyOperatorToEachQubit();
        timer_transform.Stop();

        const complexd sp = local_worker.ScalarProduct();
        double real = sp.real();
        double imag = sp.imag();
        shmem_double_allsum(&real);
        shmem_double_allsum(&imag);
        const complexd sp_sum = complexd(real, imag);

        f = norm(sp_sum);
    }

    timer_total.Stop();

    if (args.ComputationTimeWriteToFileFlag())
    {
        ComputationTimeWriteToFile();
    }

    if (args.FidelityWriteToFileFlag())
    {
        OneMinusFidelityWriteToFile();
    }

    if (args.StatsWriteToFileFlag())
    {
        StatsWriteToFile();
    }

    #ifdef DEBUG
    cout << "Master::Run() return" << endl;
    #endif
}
