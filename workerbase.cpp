WorkerBase::WorkerBase(const Parser::Args args):
    ComputationBase(args)
{

}

void WorkerBase::InitRandom()
{
    srand(time(NULL));
    x.resize(params.WorkerVectorSize());
    RandomComplexGenerator gen;
    generate(x.begin(), x.end(), gen);
    NormalizeGlobal();
}

void WorkerBase::NormalizeGlobal()
{
    long double local_sum = 0.0;
    for (vector<complexd>::const_iterator it = x.begin(); it != x.end(); it++)
    {
        local_sum += norm(*it);
    }

    long double global_sum;
    MPI_Allreduce(&local_sum, &global_sum, 1, MPI_LONG_DOUBLE, MPI_SUM,
        MPI_COMM_WORLD);

    const complexd coef = 1.0 / sqrt(global_sum);
    // multiply each element by coef
    transorm(x.begin(), x.end(), x.begin(),
        bind1st(multiplies<complexd>(), coef));
}

void WorkerBase::ApplyOperator()
{
    ::ApplyOperator(x, U, WorkerTargetQubit());
}

bool WorkerBase::ReceiveNextBuf()
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

bool WorkerBase::SendNextBuf()
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

