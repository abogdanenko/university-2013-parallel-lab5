#ifndef REMOTEWORKER_H
#define REMOTEWORKER_H

#include "worker.h"

class RemoteWorker: public Worker
{
    public:
    RemoteWorker(const Args& args);
    void ReceiveMatrix();
    void Run();
};

#endif
