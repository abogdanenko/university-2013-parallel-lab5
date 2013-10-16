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
    void ReceiveMatrix();
    void Resume();
};

