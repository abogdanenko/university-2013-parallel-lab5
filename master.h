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
    Timer timer_init;
    Timer timer_transform;
    Timer timer_total;

    void InitMatrix();
    void BroadcastMatrix();
    void OneMinusFidelityWriteToFile();
    void ComputationTimeWriteToFile();
    void StatsWriteToFile();
    vector<double> fidelity;
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
