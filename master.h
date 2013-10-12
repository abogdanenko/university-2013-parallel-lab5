class Master
{
    LocalWorker local_worker;
    Parser::Args args; // parsed program arguments
    Timer timer; // measure computation time

    public:
   
    void Init(const Parser::Args &args);
    void Run();
    void AbortWorkers();
    void DistributeInputData();
    void ManageLocalWorker();
    void ReceiveAndWriteResults();
    void WriteComputationTime();
};

