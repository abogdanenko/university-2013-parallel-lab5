#include <dislib.h>
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
{
    #ifdef DEBUG
    cout << "Master::Master()..." << endl;
    params.PrintAll();
    cout << "Master::Master() return" << endl;
    #endif
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

    s >> U[0][0];
    s >> U[0][1];
    s >> U[1][0];
    s >> U[1][1];

    #ifdef DEBUG
    cout << IDENT(1) << "Master::MatrixReadFromFile() return" << endl;
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

    // read data for local_worker
    for (auto& x: local_worker.psi)
    {
        s >> x;
    }

    auto& buffer = local_worker.buffer; // share buffer with local_worker

    for (
        int worker = 1; // skip local_worker
        worker < params.WorkerCount();
        worker++)
    {
        #ifdef DEBUG
        cout << IDENT(2) << "Transfer with worker " << worker << "..."
            << endl;
        #endif

        // repeat twice because psi is two buffers long
        for (int i = 0; i < 1; i++)
        {
            for (auto& x : buffer)
            {
                s >> x;
            }

            ShmemSendArray(&buffer.front(), buffer.size(), worker);
            shmem_barrier_all();
        }

        #ifdef DEBUG
        cout << IDENT(2) << "Transfer with worker " << worker << " DONE"
            << endl;
        #endif
    }

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

    out_it = ostream_iterator<complexd>(s, "\n");

    // write local_worker data
    copy(local_worker.psi.begin(), local_worker.psi.end(), out_it);

    auto& buffer = local_worker.psi; // use local_worker's array as buffer

    for (
        int worker = 1; // skip local_worker
        worker < params.WorkerCount();
        worker++)
    {
        #ifdef DEBUG
        cout << IDENT(2) << "Transfer with worker " << worker << "..."
            << endl;
        #endif


        ShmemSetReceiveAddr(&buffer.front());
        shmem_barrier_all();
        copy(buffer.begin(), buffer.end(), out_it);

        #ifdef DEBUG
        cout << IDENT(2) << "Transfer with worker " << worker << " DONE"
            << endl;
        #endif
    }

    #ifdef DEBUG
    cout << IDENT(1) << "Master::VectorWriteToFile() return" << endl;
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
    const Vector row1 = {c, s};
    const Vector row2 = {-1.0 * s, c};
    const Matrix U_theta = {row1, row2};

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
            const vector<double> complex_array = {elem.real(), elem.imag()};
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
    s << timer.GetDelta() << endl;
}

void Master::Run()
{
    #ifdef DEBUG
    cout << "Master::Run()..." << endl;
    #endif

    shmem_barrier_all();
    timer.Start();
    if (args.MatrixReadFromFileFlag())
    {
        MatrixReadFromFile();
    }
    AddNoiseToMatrix();
    BroadCastMatrix();
    if (args.VectorReadFromFileFlag())
    {
        VectorReadFromFile();
    }
    else
    {
        local_worker.InitRandom();
    }

    local_worker.ApplyOperatorToEachQubit();

    if (args.VectorWriteToFileFlag())
    {
        VectorWriteToFile();
    }
    shmem_barrier_all();
    timer.Stop();
    if (args.ComputationTimeWriteToFileFlag())
    {
        ComputationTimeWriteToFile();
    }

    #ifdef DEBUG
    cout << "Master::Run() return" << endl;
    #endif
}

