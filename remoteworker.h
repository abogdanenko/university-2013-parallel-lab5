#ifndef REMOTEWORKER_H
#define REMOTEWORKER_H

#include "workerbase.h"

class RemoteWorker: WorkerBase
{
    void ReceiveMatrix();
    void VectorReceiveFromMaster();
    void VectorSendToMaster() const;
    public:
    RemoteWorker(const Args& args);
    void Run();
};

#endif
