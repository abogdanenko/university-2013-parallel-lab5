#ifndef REMOTEWORKER_H
#define REMOTEWORKER_H

#include "workerbase.h"

class RemoteWorker: public WorkerBase
{
    public:
    RemoteWorker(const Args& args);
    void ReceiveMatrix();
    void Run();
};

#endif
