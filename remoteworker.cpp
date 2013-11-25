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

void RemoteWorker::VectorSendToMaster() const
{
    #ifdef DEBUG
    cout << INDENT(1) << "RemoteWorker::VectorSendToMaster()..." << endl;
    #endif

    for (
        int worker = 1; // skip local_worker
        worker < shmem_n_pes();
        worker++)
    {
        // master sets receive address and flushes previous buffer
        ShmemBarrierAll();
        // receive address is set, buffer ready, can start transfer

        if (worker == shmem_my_pe())
        {
            Shmem::SendVector(psi.begin(), psi.end(), master_rank);
        }

        ShmemBarrierAll();
        // transfer complete, master can start copying data from buffer
    }

    #ifdef DEBUG
    cout << INDENT(1) << "RemoteWorker::VectorSendToMaster() return" << endl;
    #endif
}

void RemoteWorker::VectorReceiveFromMaster()
{
    #ifdef DEBUG
    cout << INDENT(1) << "RemoteWorker::VectorReceiveFromMaster()..." << endl;
    #endif

    for (
        int worker = 1; // skip local_worker
        worker < shmem_n_pes();
        worker++)
    {
        // repeat twice because psi is two buffers long
        for (int i = 0; i < 2; i++)
        {
            if (worker == shmem_my_pe())
            {
                const auto begin = psi.begin() + i * psi.size() / 2;
                Shmem::SetReceiveVector(begin);
            }

            // ready to receive if necessary at this point
            ShmemBarrierAll();

            // transfer done
            ShmemBarrierAll();
        }
    }

    #ifdef DEBUG
    cout << INDENT(1) << "RemoteWorker::VectorReceiveFromMaster() return" << endl;
    #endif
}

void RemoteWorker::Run()
{
    #ifdef DEBUG
    cout << "RemoteWorker::Run()..." << endl;
    #endif
    ShmemBarrierAll();

    ReceiveMatrix();
    if (args.VectorReadFromFileFlag())
    {
        VectorReceiveFromMaster();
    }
    else
    {
        VectorInitRandom();
    }
    ApplyOperatorToEachQubit();
    if (args.VectorWriteToFileFlag())
    {
        VectorSendToMaster();
    }

    ShmemBarrierAll();
    #ifdef DEBUG
    cout << "RemoteWorker::Run() return" << endl;
    #endif
}

