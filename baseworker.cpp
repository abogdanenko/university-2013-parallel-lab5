BaseWorker::BaseWorker():
    bufs_received_count(0),
    bufs_sent_count(0),
    U(vector< vector<complexd> >(2, vector<complexd>(2)))
{
}

void BaseWorker::InitRandom()
{
    srand(time(NULL));
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

void BaseWorker::ReceiveInputData()
{
    ReceiveMatrix();
    ReceiveFlags();
    ReceiveNumericalParams();
}

bool BaseWorker::ReceiveNextBuf()
{
    if (bufs_received_count == bufs_per_worker)
    {
        return false;
    }
    MPI_IReceive(buf);
    MPI_Wait();
    if (split_slices_between_workers)
    {
        copy(buf.begin(), buf.end(),
            x.begin() + bufs_received_count * buf.size());
    }
    else
    {
        SplitBuf(buf, buf0, buf1);
        copy(buf0.begin(), buf0.end(),
            x.begin() + bufs_received_count * buf0.size());    
        copy(buf1.begin(), buf1.end(),
            x.begin() + x.size() / 2 + bufs_received_count * buf0.size());
    }
    bufs_received_count++;
    return true;
}

bool BaseWorker::SendNextBuf()
{
    if (bufs_sent_count == bufs_per_worker)
    {
        return false;
    }
    if (split_slices_between_workers)
    {
        copy(x.begin() + bufs_received_count * buf.size(),
            x.begin() + (bufs_received_count + 1) * buf.size(), buf.begin());
    }
    else
    {
        copy(x.begin() + bufs_received_count * buf0.size(),
            x.begin() + (bufs_received_count + 1) * buf0.size(), buf0.begin());
        copy(x.begin() + x.size() / 2 + bufs_received_count * buf0.size(),
            x.begin() + x.size() / 2 + (bufs_received_count + 1) * buf0.size(), buf0.begin());
        MergeBuf(buf, buf0, buf1);
    }
    MPI_ISend(buf);
    MPI_Wait();
    bufs_sent_count++;
    return true;
}

