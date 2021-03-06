#include <dislib.h>

#ifdef DEBUG
#include "debug.h"
#endif

#include "remoteworker.h"
#include "shmem.h"

RemoteWorker::RemoteWorker(const Args& args):
    WorkerBase(args)
{

}

void RemoteWorker::ReceiveMatrix()
{
    #ifdef DEBUG
    cout << INDENT(1) << "RemoteWorker::ReceiveMatrix()..." << endl;
    #endif

    for (auto& row: U)
    {
        for (auto& elem: row)
        {
            vector<double> complex_array(2);
            for (auto& x: complex_array)
            {
                shmem_double_toall(&x, master_rank);
            }
            elem = complexd(complex_array[0], complex_array[1]);
        }
    }

    #ifdef DEBUG
    cout << INDENT(1) << "RemoteWorker::ReceiveMatrix() return" << endl;
    #endif
}

void RemoteWorker::Run()
{
    #ifdef DEBUG
    cout << "RemoteWorker::Run()..." << endl;
    #endif

    ShmemBarrierAll(); // timer_total
    for (int i = 0; i < args.IterationCount(); i++)
    {
        ShmemBarrierAll(); // timer_init
        VectorInitRandom();
        ShmemBarrierAll(); // timer_init

        U = HadamardMatrix();

        ShmemBarrierAll(); // timer_transform
        ApplyOperatorToEachQubit();
        ShmemBarrierAll(); // timer_transform

        SwapVectors();
        ReceiveMatrix();

        ShmemBarrierAll(); // timer_transform
        ApplyOperatorToEachQubit();
        ShmemBarrierAll(); // timer_transform

        complexd  sp = ScalarProduct();
        double real = sp.real();
        double imag = sp.imag();
        shmem_double_allsum(&real);
        shmem_double_allsum(&imag);
    }
    ShmemBarrierAll(); // timer_total

    #ifdef DEBUG
    cout << "RemoteWorker::Run() return" << endl;
    #endif
}
