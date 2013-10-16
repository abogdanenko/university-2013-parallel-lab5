#include <stdexcept> // runtime_error

#include "localworker.h"
#include "timer.h"

class Master: public ComputationBase
{
    LocalWorker local_worker;
    Timer timer; // measure computation time

    Master(const Args args);

    /*
        Give control to local_worker so that he could
        do some computation or data transfer.
    */
    void YieldToLocalWorker();
    void MatrixReadFromFile();

    template <class WorkerBufTransferOp>
    void ForEachBufNoSplit(WorkerBufTransferOp op);

    template <class WorkerBufTransferOp>
    void ForEachBufSplit(WorkerBufTransferOp op);

    template <class WorkerBufTransferOp>
    void ForEachBuf(WorkerBufTransferOp op);

    void ReceiveBufFromWorkerToOstream(const int worker);
    void SendBufToWorkerFromIstream(const int worker);
    void VectorReadFromFile();
    void VectorWriteToFile();
    void WriteComputationTime();
    public:
    class IdleWorkersError: public runtime_error
    {
        public:
        IdleWorkersError();
    };
    void Run();
};

