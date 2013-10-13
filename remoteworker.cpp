void RemoteWorker::Run()
{
    BroadCastReceive(command);
    if (command == GO_AHEAD)
    {
        ReceiveInputData();
        if (random)
        {
            InitRandom();
        }
        else
        {
            while (ReceiveNextBuf())
            {

            }
        }
        ApplyOperator();
        if (write_vector_to_file)
        {
            while (SendNextBuf())
            {

            }
        }
        Barrier();
    }
}

