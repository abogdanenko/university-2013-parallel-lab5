#ifndef MASTER_H
#define MASTER_H

#include <stdexcept> // runtime_error

#include "localworker.h"
#include "timer.h"

class Master;
typedef void (Master::* WorkerBufTransferOp)(int);

class Master: ComputationBase
{
    LocalWorker local_worker;
    Timer timer; // measure computation time

    void AddNoiseToMatrix();
    void BroadcastMatrix();
    void ComputationTimeWriteToFile();
    public:
    Master(const Args& args);
    class IdleWorkersError: public runtime_error
    {
        public:
        IdleWorkersError();
    };
    void Run();
};

#endif
