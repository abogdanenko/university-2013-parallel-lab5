Master::Master(const Parser::Args args):
    ComputationBase(args)
{

}

Master::IdleWorkersError::IdleWorkersError()
    runtime_error("Too many processes for given number of qubits.")
{

}

void Master::MatrixReadFromFile()
{
    // read U from file or stdin
    ifstream fs;
    istream& s = (args.MatrixFileName() == "-") ? cin :
        (fs.open(args.MatrixFileName().c_str()), fs);
    s >> U[0][0] >> U[0][1] >> U[1][0] >> U[1][1];
    /* Give control to local_worker so that he could receive input data and
       initialize his vector randomly if told to do so. */
    YieldToLocalWorker();
}

template <class WorkerBufTransferOp>
void Master::ForEachBufNoSplit(WorkerBufTransferOp op)
{
    for (int worker = 0; worker < params.WorkerCount(); worker++)
    {
        for (i = 0; i < params.BufCount(); i++)
        {
            op(worker);
        }
    }
}

template <class WorkerBufTransferOp>
void Master::ForEachBufSplit(WorkerBufTransferOp op)
{
    int worker = 0;
    for (int slice = 0; slice < params.SliceCount(); slice++)
    {
        for (int target_qubit_value = 0; target_qubit_value <= 1;
            target_qubit_value++)
        {
            for (int j = 0; j < params.WorkersPerSlice() / 2; j++)
            {
                for (int i = 0; i < params.BufCount() / 2; i++)
                {
                    op(worker);
                }
                worker++;
            }
            if (target_qubit_value == 0)
            {
                worker -= params.WorkersPerSlice() / 2;
            }
        }
    }
}

template <class WorkerBufTransferOp>
void Master::ForEachBuf(WorkerBufTransferOp op)
{
    if (params.Split())
    {
        ForEachBufSplit(op);
    }
    else
    {
        ForEachBufNoSplit(op);
    }
}

void Master::ReceiveBufFromWorkerToOstream(const int worker)
{
    // give control to local_worker so that he could send data
    if (worker == 0)
    {
        YieldToLocalWorker();
    }

    MPI_Irecv(buf, worker);
    MPI_Wait();
    out_it = copy(buf.begin(), buf.end(), out_it);
}

void Master::SendBufToWorkerFromIstream(const int worker)
{
    // copy to buf from in_it
    for (Buf::iterator it = buf.begin(); it != buf.end(); it++)
    {
        *it = *in_it;
        in_it++;
    }
    MPI_Isend(buf, worker);

    // give control to local_worker so that he could receive data
    if (worker == 0)
    {
        YieldToLocalWorker();
    }

    MPI_Wait();
}

void Master::VectorReadFromFile()
{
    // read x from file or stdin
    ifstream fs;
    istream& s = (args.VectorInputFileName() == "-") ? cin :
        (fs.open(args.VectorInputFileName().c_str()), fs);
    in_it = istream_iterator<complexd>(s);

    ForEachBuf(SendBufToWorkerFromIstream);
}

void Master::VectorWriteToFile()
{
    ofstream fs;
    ostream& s = (args.VectorOutputFileName() == "-") ? cout :
        (fs.open(args.VectorOutputFileName().c_str()), fs);

    out_it = ostream_iterator<complexd> (s, "\n");

    ForEachBuf(ReceiveBufFromWorkerToOstream);
}

void Master::WriteComputationTime()
{
    ofstream fs;
    ostream& s = (args.ComputationTimeFileName() == "-") ? cout :
        (fs.open(args.ComputationTimeFileName().c_str()), fs);
    s << timer.GetDelta() << endl;
}

void Master::DistributeInputData()
{
}

void Master::Run()
{
    MPI_Barrier();
    timer.Start();
    if (args.MatrixReadFromFileFlag())
    {
        MatrixReadFromFile();    
    }
    if (args.VectorReadFromFileFlag())
    {
        VectorReadFromFile();
    }
    if (args.VectorWriteToFileFlag())
    {
        VectorWriteToFile();
    }
    MPI_Barrier();
    timer.Stop();
    if (args.ComputationTimeWriteToFileFlag())
    {
        ComputationTimeWriteToFile();
    }
}

void Master::YieldToLocalWorker()
{
    local_worker.Resume();
}

