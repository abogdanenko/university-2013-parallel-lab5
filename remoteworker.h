class RemoteWorker: public WorkerBase
{
    public:
    static const int GO_AHEAD = 0;
    Run();
    WaitForGoAheadOrAbort();
};

