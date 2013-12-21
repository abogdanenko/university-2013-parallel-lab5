#include <dislib.h>
#include <fstream>
#include <iostream> // std::cin, std::cout
#include <iterator> // std::ostream_iterator

#include "master.h"
#include "routines.h"
#include "normaldistributiongenerator.h"
#include "shmem.h"

#ifdef DEBUG
#include "debug.h"
#endif

using std::ifstream;
using std::ofstream;
using std::istream;
using std::ostream;
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
    params.PrintAll();
    cout << "Master::Master() return" << endl;
    #endif
}

void Master::VectorReadFromFile()
{
    #ifdef DEBUG
    cout << INDENT(1) << "Master::VectorReadFromFile()..." << endl;
    #endif

    // read state vector from file or stdin
    ifstream fs;

    // throw exceptions if unable to read the file for some reason
    fs.exceptions(ifstream::failbit | ifstream::badbit);

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
        worker < shmem_n_pes();
        worker++)
    {
        #ifdef DEBUG
        cout << INDENT(2) << "Transfer with worker " << worker << "..."
            << endl;
        #endif

        // repeat twice because psi is two buffers long
        for (int i = 0; i < 2; i++)
        {
            for (auto& x : buffer)
            {
                s >> x;
            }

            // make sure remote worker is ready to receive
            ShmemBarrierAll();

            Shmem::SendVector(buffer.begin(), buffer.end(), worker);

            // transfer done
            ShmemBarrierAll();
        }

        #ifdef DEBUG
        cout << INDENT(2) << "Transfer with worker " << worker << " DONE"
            << endl;
        #endif
    }

    #ifdef DEBUG
    cout << INDENT(1) << "Master::VectorReadFromFile() return" << endl;
    #endif
}

void Master::VectorWriteToFile()
{
    #ifdef DEBUG
    cout << INDENT(1) << "Master::VectorWriteToFile()..." << endl;
    #endif

    // write the vector to file or stdout
    ofstream fs;

    // throw exceptions if unable to write to the file for some reason
    fs.exceptions(ofstream::failbit | ofstream::badbit);

    ostream& s = (args.VectorOutputFileName() == "-") ? cout :
        (fs.open(args.VectorOutputFileName().c_str()), fs);

    ostream_iterator<complexd> out_it(s, "\n");

    // write local_worker data
    copy(local_worker.psi.begin(), local_worker.psi.end(), out_it);

    auto& buffer = local_worker.psi; // use local_worker's array as buffer
    Shmem::SetReceiveVector(buffer.begin());

    for (
        int worker = 1; // skip local_worker
        worker < shmem_n_pes();
        worker++)
    {
        #ifdef DEBUG
        cout << INDENT(2) << "Transfer with worker " << worker << "..."
            << endl;
        #endif

        ShmemBarrierAll();
        // remote worker starts transfer

        ShmemBarrierAll();
        // transfer complete

        copy(buffer.begin(), buffer.end(), out_it);

        #ifdef DEBUG
        cout << INDENT(2) << "Transfer with worker " << worker << " DONE"
            << endl;
        #endif
    }

    #ifdef DEBUG
    cout << INDENT(1) << "Master::VectorWriteToFile() return" << endl;
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
    Matrix U_theta(IdentityMatrix());

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
    s << timer.GetDelta() << endl;
}

void Master::Run()
{
    #ifdef DEBUG
    cout << "Master::Run()..." << endl;
    #endif

    ShmemBarrierAll();
    timer.Start();
    AddNoiseToMatrix();
    BroadcastMatrix();
    if (args.VectorReadFromFileFlag())
    {
        VectorReadFromFile();
    }
    else
    {
        local_worker.VectorInitRandom();
    }

    local_worker.ApplyOperatorToEachQubit();

    if (args.VectorWriteToFileFlag())
    {
        VectorWriteToFile();
    }
    ShmemBarrierAll();
    timer.Stop();
    if (args.ComputationTimeWriteToFileFlag())
    {
        ComputationTimeWriteToFile();
    }

    #ifdef DEBUG
    cout << "Master::Run() return" << endl;
    #endif
}

