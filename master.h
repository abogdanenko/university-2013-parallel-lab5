#ifndef MASTER_H
#define MASTER_H

#include <stdexcept> // runtime_error
#include <iterator>

#include "localworker.h"
#include "timer.h"

using std::istream_iterator;
using std::ostream_iterator;

class Master;
typedef void (Master::* WorkerBufTransferOp)(int);

class Master: public ComputationBase
{
    LocalWorker local_worker;
    Timer timer; // measure computation time

    istream_iterator<complexd>* in_it;
    ostream_iterator<complexd>* out_it;

    /*
        Give control to local_worker so that he could
        do some computation or data transfer.
    */
    void YieldToLocalWorker();
    void MatrixReadFromFile();

    void ForEachBufNoSplit(WorkerBufTransferOp op);
    void ForEachBufSplit(WorkerBufTransferOp op);
    void ForEachBuf(WorkerBufTransferOp op);

    void ReceiveBufFromWorkerToOstream(const int worker);
    void SendBufToWorkerFromIstream(const int worker);
    void VectorReadFromFile();
    void VectorWriteToFile();
    void ComputationTimeWriteToFile();
    public:
    Master(const Args& args);
    class IdleWorkersError: public runtime_error
    {
        public:
        IdleWorkersError();
    };
    void Run();
};

#endif
