class RemoteWorker: public BaseWorker
{
    public:
    static const int GO_AHEAD = 0;
    Run();
    WaitForGoAheadOrAbort();
};

