Worker::Worker():
    U(vector< vector<complexd> >(2, vector<complexd>(2)))
{
    srand(time(NULL));
}

void BaseWorker::InitRandom()
{
    x.resize(N);
    RandomComplexGenerator gen;
    generate(x.begin(), x.end(), gen);

    // normalize
    long double local_sum = 0.0;
    for (vector<complexd>::const_iterator it = x.begin(); it != x.end(); it++)
    {
        local_sum += norm(*it);
    }
    MPI_Reduce_all(local_sum, global_sum);
    const long double coef = 1.0 / sqrt(global_sum);
    // "x = coef * x"
    transorm(x.begin(), x.end(), x.begin(),
        bind1st(multiplies<complexd>(), (complexd) coef));
}

void BaseWorker::ApplyOperator()
{
    const Index N = x.size() / 2;
    for (Index i = 0; i < N; i++)
    {
        const Index j = i + N;
        const complexd a = x[i];
        const complexd b = x[j];
        x[i] = U[0][0] * a + U[0][1] * b;
        x[j] = U[1][0] * a + U[1][1] * b;
    }
}

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

LocalWorker::LocalWorker():
    current_state(STATE_RECEIVE_INPUT_DATA)
{

}

void LocalWorker::Run()
{
    bool yield_to_master = false;

    while (current_state != STATE_END && !yield_to_master)
    {
        switch (current_state)
        {
            case STATE_RECEIVE_INPUT_DATA:
                ReceiveInputData();
                if (random)
                {
                    InitRandom();
                    current_state = STATE_APPLY_OPERATOR;
                    yield_to_master = false;
                }
                else
                {
                    current_state = STATE_RECEIVE_MY_SLICE;
                    yield_to_master = true;
                }
                break;
            case STATE_INIT_RANDOM:
                InitRandom();
                current_state = STATE_APPLY_OPERATOR;
                yield_to_master = false;
                break;
            case STATE_RECEIVE_MY_SLICE:
                ReceiveNextBuf();
                if (received_all_bufs)
                {
                    current_state = STATE_APPLY_OPERATOR;
                }
                yield_to_master = true;
                break;
            case STATE_APPLY_OPERATOR:
                ApplyOperator();
                if (write_vector_to_file)
                {
                    current_state = STATE_SEND_MY_SLICE;
                }
                else
                {
                    current_state = STATE_END;
                }
                yield_to_master = false;
                break;
            case STATE_SEND_MY_SLICE:
                SendNextBuf();
                if (sent_all_bufs)
                {
                    current_state = STATE_END;
                    yield_to_master = false;
                }
                else
                {
                    yield_to_master = true;
                }
                break;
            default:
                throw runtime_exception("Bad LocalWorker state");
                break;
        }
    }
}

void BaseWorker::ReceiveInputData()
{
    ReceiveMatrix();
    ReceiveFlags();
    ReceiveNumericalParams();
}

void BaseWorker::ReceiveBuf()
{
    i = bufs_received_count;
    ReceiveBuf();
    MPI_IReceive(buf);
    MPI_Wait();
    copy(buf.begin(), buf.end(), x.begin() + i * buf.size());
    if (!split_slices_between_workers)
    {
        SplitBuf(buf, buf0, buf1);
        copy(buf0.begin(), buf0.end(), x.begin() + i * buf0.size());    
        copy(buf1.begin(), buf1.end(), x.begin() + x.size() / 2 + i * buf0.size());
    }
}

