void Master::PrepareOperator()
{
    if (U_filename)
    {
        // read U from file or stdin
        ifstream fs;
        istream& s = (string(U_filename) == "-") ? cin :
            (fs.open(U_filename), fs);
        s >> U[0][0] >> U[0][1] >> U[1][0] >> U[1][1];
    }
    else
    {
        // Assign identity matrix to U
        U[0][0] = 1;
        U[0][1] = 0;
        U[1][0] = 0;
        U[1][1] = 1;
    }
}

template <class WorkerBufTransferOp>
void Master::ForEachBuf(WorkerBufTransferOp op)
{
    if (split)
    {
        const Index slice_size = 1 << k;
        const Index slice_count = 1 << (n - k);
        const int workers_per_slice = worker_count / slice_count;

        int worker = 0;
        for (int slice = 0; slice < slices_count; slice++)
        {
            for (int i_k = 0; i_k <= 1; i_k++)
            {
                for (int j = 0; j < workers_per_slice / 2; j++)
                {
                    for (int i = 0; i < buf_count / 2; i++)
                    {
                        op(worker);
                    }
                    worker++;
                }
                if (i_k == 0)
                {
                    worker -= workers_per_slice / 2;
                }
            }
        }
    }
    else
    {
        for (int worker = 0; worker < worker_count; worker++)
        {
            for (i = 0; i < buf_count; i++)
            {
                op(worker);
            }
        }
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
    istream& s = (string(x_filename) == "-") ? cin :
        (fs.open(x_filename), fs);
    in_it = istream_iterator<complexd>(s);

    ForEachBuf(SendBufToWorkerFromIstream);
}

void Master::VectorWriteToFile()
{
    ofstream fs;
    ostream& s = (string(y_filename) == "-") ? cout :
        (fs.open(y_filename), fs);

    out_it = ostream_iterator<complexd> (s, "\n");

    ForEachBuf(ReceiveBufFromWorkerToOstream);
}

void Master::WriteComputationTime()
{
    ofstream fs;
    ostream& s = (string(T_filename) == "-") ? cout :
        (fs.open(T_filename), fs);
    s << timer.GetDelta() << endl;
}

void Master::DistributeInputData()
{
    // transform matrix
    vector< vector<complexd> > U(
        vector< vector<complexd> >(2, vector<complexd>(2)))
    PrepareOperator(U);
}

void Master::Run()
{
    if (argc == 1)
    {
        Parser::PrintUsage();
        BroadcastAbort();
    }
    else
    {
        BroadcastGoAhead();
        timer.Start();
        DistributeInputData();
        /* Give control to local_worker so that he could receive input data and
           initialize his vector randomly if told to do so. */
        YieldToLocalWorker();
        if (x_filename)
        {
            VectorReadFromFile();
        }
        if (y_filename)
        {
            VectorWriteToFile();
        }
        MPI_Barrier();
        timer.Stop();
        if (T_filename)
        {
            WriteComputationTime();
        }
    }
}

void Master::YieldToLocalWorker()
{
    local_worker->Resume();
}

