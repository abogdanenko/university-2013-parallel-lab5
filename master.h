#ifndef MASTER_H
#define MASTER_H

#include <stdexcept> // runtime_error

#include "workerbase.h"
#include "timer.h"

class Master: ComputationBase
{
    WorkerBase local_worker;
    Timer timer_init;
    Timer timer_transform;
    Timer timer_total;
    vector<double> fidelity;

    void OneMinusFidelityWriteToFile();
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
