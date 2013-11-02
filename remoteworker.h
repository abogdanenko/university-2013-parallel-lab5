#ifndef REMOTEWORKER_H
#define REMOTEWORKER_H

#include "workerbase.h"

class RemoteWorker: WorkerBase
{
    void ReceiveMatrix();
    public:
    RemoteWorker(const Args& args);
    void Run();
};

#endif
