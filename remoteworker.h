#ifndef REMOTEWORKER_H
#define REMOTEWORKER_H

#include "workerbase.h"

class RemoteWorker: public WorkerBase
{
    public:
    void ReceiveMatrix();
    void Run();
};

#endif
