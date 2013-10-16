#include "workerbase.h"

class RemoteWorker: public WorkerBase
{
    public:
    void ReceiveMatrix();
    void Run();
};

