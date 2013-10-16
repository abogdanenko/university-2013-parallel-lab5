#ifndef LOCALWORKER_H
#define LOCALWORKER_H

#include "workerbase.h"

class LocalWorker: public WorkerBase
{
    enum State
    {
        STATE_BEGIN,
        STATE_RECEIVE_MATRIX,
        STATE_RECEIVE_VECTOR,
        STATE_INIT_RANDOM,
        STATE_APPLY_OPERATOR,
        STATE_SEND_VECTOR,
        STATE_END
    };
    public:
    LocalWorker(const Args& args);
    void ReceiveMatrix();
    void Resume();
};

#endif
