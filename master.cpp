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

void Master::DistributeInputData()
{
    // transform matrix
    vector< vector<complexd> > U(
        vector< vector<complexd> >(2, vector<complexd>(2)))
    PrepareOperator(U);

    if (x_filename)
    {
        // read x from file or stdin
        ifstream fs;
        istream& s = (string(x_filename) == "-") ? cin :
            (fs.open(x_filename), fs);
        istream_iterator<complexd> in_it(s);
        if (split_slices_between_workers)
        {
            ReadAndSendSplit(in_it);
        }
        else
        {
            ReadAndSendNoSplit(in_it);
        }
    }
    else
    {
        // set random n qubit state
        MPI_Reduce_receive(sum);

        // Normalize x
        const double coef = 1.0 / sqrt(sum);
        MPI_Broadcast_send(coef);
    }
}

void Master::ReceiveAndWriteSplit(const ostream_iterator<complexd>& f)
{
    for (int slice = 0; slice < slices_count; slice++)
    {
        // we get the same peer twice
        for (int peer = FirstPeer(slice); peer < EndPeer(slice); peer++)
        {
            for (int i = 0; i < bufs_per_peer / 2; j++)
            {
                if (peer == 0)
                {
                    local_worker.SendNextBuf();
                }
                MPI_Irecv(buf, peer);
                MPI_Wait();
                copy(buf.begin(), buf.end(), out_it);
            }
        }
    }
}

void Master::ReceiveAndWriteNoSplit(const ostream_iterator<complexd>& f)
{
    for (int peer = 0; peer < np; peer++)
    {
        if (peer == 0)
        {
            local_worker.SendNextBuf();
        }
        for (i = 0; i < bufs_per_peer_count; i++)
        {
            MPI_Irecv(buf, peer);
            MPI_Wait();
            copy(buf.begin(), buf.end(), out_it);
        }
    }
}

void Master::ReceiveAndWriteResults()
{
    if (y_filename)
    {
        ofstream fs;
        ostream& s = (string(y_filename) == "-") ? cout :
            (fs.open(y_filename), fs);
        ostream_iterator<complexd> out_it (s, "\n");

        if (split_slices_between_workers)
        {
            ReceiveAndWriteSplit(out_it);
        }
        else
        {
            ReceiveAndWriteNoSplit(out_it);
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

