BaseWorker::BaseWorker():
    U(vector< vector<complexd> >(2, vector<complexd>(2)))
{

}

void BaseWorker::InitRandom()
{
    srand(time(NULL));
    x.resize(params.WorkerVectorSize());
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
    ::ApplyOperator(x, U, WorkerTargetQubit());
}

void BaseWorker::ReceiveInputData()
{
    ReceiveMatrix();
    ReceiveFlags();
    ReceiveNumericalParams();
}

bool BaseWorker::ReceiveNextBuf()
{
    static vector<complexd>::iterator it = x.begin();
    if (it == x.end())
    {
        return false;
    }
    MPI_IReceive(buf);
    MPI_Wait();
    it = copy(buf.begin(), buf.end(), it);
    return true;
}

bool BaseWorker::SendNextBuf()
{
    static vector<complexd>::iterator it = x.begin();
    if (it == x.end())
    {
        return false;
    }
    const vector<complexd>::iterator next = it + buf.size();
    copy(it, next, buf.begin());
    it = next;
    MPI_ISend(buf);
    MPI_Wait();
    return true;
}

