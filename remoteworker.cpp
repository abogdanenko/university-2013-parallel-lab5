void RemoteWorker::ReceiveMatrix()
{
    vector<complexd> buf(4);

    MPI_Bcast(&buf[0], 4, MPI_DOUBLE_COMPLEX, master_rank, MPI_COMM_WORLD);

    U[0][0] = buf[0];
    U[0][1] = buf[1];
    U[1][0] = buf[2];
    U[1][1] = buf[3];
}

void RemoteWorker::Run()
{
    if (args.MatrixReadFromFileFlag())
    {
        ReceiveMatrix();
    }
    if (args.VectorReadFromFileFlag())
    {
        while (ReceiveNextBuf())
        {

        }
    }
    else
    {
        InitRandom();
    }
    ApplyOperator();
    if (args.VectorWriteToFileFlag())
    {
        while (SendNextBuf())
        {

        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
}

