#ifndef MASTER_H
#define MASTER_H

#include <stdexcept> // runtime_error

#include "workerbase.h"
#include "timer.h"

class Master: ComputationBase
{
    WorkerBase local_worker;
    Timer timer; // measure computation time

    void VectorReadFromFile();
    void VectorWriteToFile();
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
