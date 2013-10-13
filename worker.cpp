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

void RemoteWorker::ReceiveVector()
{

}

void RemoteWorker::Run()
{
    WaitForGoAheadOrAbort();
    while (current_state != STATE_END)
    {
        current_state = Transition(current_state);
        Transition();
    }
    Barrier();
}

void LocalWorker::Run()
{
    while (current_state != STATE_END && !time_to_yield)
    {
        time_to_yield = false;
        Transition();
    }
}

void Worker::Transition()
{
    switch (current_state)
    {
        case STATE_RECEIVE_INPUT_DATA:
            ReceiveInputData();
            if (random)
            {
                current_state = STATE_INIT_RANDOM;
            }
            else
            {
                current_state = STATE_RECEIVE_MY_SLICE;
                YieldToMaster();
            }
            break;
        case STATE_INIT_RANDOM:
            InitRandom();
            current_state = APPLY_OPERATOR;
            break;
        case STATE_RECEIVE_MY_SLICE:
            ReceiveMySlice();
            if (whole_vector_received)
            {
                current_state = STATE_APPLY_OPERATOR;
            }
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
            break;
        case STATE_SEND_MY_SLICE:
            SendMySlice();
            if (all_slices_sent)
            {
                current_state = STATE_END;
            }
            break;
        default:
            break;
    }
}
