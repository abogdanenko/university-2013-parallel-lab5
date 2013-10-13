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

template <class UnaryOperation>
void Master::IterateNoSplit(UnaryOperation op)
{
    for (int peer = 0; peer < np; peer++)
    {
        for (i = 0; i < bufs_per_peer_count; i++)
        {
            op(peer);
        }
    }
}

template <class UnaryOperation>
void Master::IterateSplit(UnaryOperation op)
{
    for (int slice = 0; slice < slices_count; slice++)
    {
        // we get the same peer twice
        for (int peer = FirstPeer(slice); peer < EndPeer(slice); peer++)
        {
            for (int i = 0; i < bufs_per_peer / 2; j++)
            {
                op(peer);
            }
        }
    }
}

void Master::ReceiveBufFromPeerToOstream(const int peer)
{
    if (peer == 0)
    {
        local_worker.SendNextBuf();
    }
    MPI_Irecv(buf, peer);
    MPI_Wait();
    copy(buf.begin(), buf.end(), out_it);
}

void Master::SendBufToPeerFromIstream(const int peer)
{
    copy(in_it, in_it + buf.size(), buf.begin());
    MPI_Isend(buf, peer);
    if (peer == 0)
    {
        local_worker.ReceiveNextBuf();
    }
    MPI_Wait();
}

void Master::VectorReadFromFile()
{
    if (x_filename)
    {
        // read x from file or stdin
        ifstream fs;
        istream& s = (string(x_filename) == "-") ? cin :
            (fs.open(x_filename), fs);
        in_it = istream_iterator<complexd>(s);

        if (split_slices_between_workers)
        {
            IterateSplit(SendBufToPeerFromIstream);
        }
        else
        {
            IterateNoSplit(SendBufToPeerFromIstream);
        }
    }
}

void Master::VectorWriteToFile()
{
    if (y_filename)
    {
        ofstream fs;
        ostream& s = (string(y_filename) == "-") ? cout :
            (fs.open(y_filename), fs);

        out_it = ostream_iterator<complexd> (s, "\n");

        if (split_slices_between_workers)
        {
            IterateSplit(ReceiveBufFromPeerToOstream);
        }
        else
        {
            IterateNoSplit(ReceiveBufFromPeerToOstream);
        }
    }
}

void Master::WriteComputationTime()
{
    if (T_filename)
    {
        ofstream fs;
        ostream& s = (string(T_filename) == "-") ? cout :
            (fs.open(T_filename), fs);
        s << timer.GetDelta() << endl;
    }
}

void Master::DistributeInputData()
{
    // transform matrix
    vector< vector<complexd> > U(
        vector< vector<complexd> >(2, vector<complexd>(2)))
    PrepareOperator(U);

    if (x_filename)
    {
        VectorReadFromFile();
    }
    else
    {
        local_worker.InitRandom();
    }
}

void Master::Run()
{
    if (argc == 1)
    {
        Parser::PrintUsage();
        AbortWorkers();
    }
    else
    {
        // implicit barrier here to measure time consistently
        SendBlockingGoAhead();
        // All processes have started and are standing by
        timer.Start();
        DistributeInputData();
        ManageLocalWorker();
        ReceiveAndWriteResults();
        // implicit barrier here to measure time consistently
        WaitForRemoteWorkers();
        timer.Stop();
        WriteComputationTime();
    }
}

