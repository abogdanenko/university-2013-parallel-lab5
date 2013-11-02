#ifndef LOCALWORKER_H
#define LOCALWORKER_H

#include "workerbase.h"

class LocalWorker: WorkerBase
{
    friend class Master;
    public:
    LocalWorker(const Args& args);
};

#endif
