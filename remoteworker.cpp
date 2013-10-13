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
            while (!received_all_bufs)
            {
                ReceiveNextBuf();
            }
        }
        ApplyOperator();
        if (write_vector_to_file)
        {
            while (!sent_all_bufs)
            {
                SendNextBuf();
            }
        }
        Barrier();
    }
}

