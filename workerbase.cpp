WorkerBase::WorkerBase(const Parser::Args args):
    ComputationBase(args)
{

}

void WorkerBase::InitRandom()
{
    srand(time(NULL));
    psi.resize(params.WorkerVectorSize());
    RandomComplexGenerator gen;
    generate(psi.begin(), psi.end(), gen);
    NormalizeGlobal();
}

void WorkerBase::NormalizeGlobal()
{
    long double local_sum = 0.0;
    for (vector<complexd>::const_iterator it = psi.begin(); it != psi.end(); it++)
    {
        local_sum += norm(*it);
    }

    long double global_sum;
    MPI_Allreduce(&local_sum, &global_sum, 1, MPI_LONG_DOUBLE, MPI_SUM,
        MPI_COMM_WORLD);

    const complexd coef = 1.0 / sqrt(global_sum);
    // multiply each element by coef
    transorm(psi.begin(), psi.end(), psi.begin(),
        bind1st(multiplies<complexd>(), coef));
}

void WorkerBase::ApplyOperator()
{
    ::ApplyOperator(psi, U, WorkerTargetQubit());
}

bool WorkerBase::ReceiveNextBuf()
{
    static vector<complexd>::iterator psi_it = psi.begin();
    if (psi_it == psi.end())
    {
        return false;
    }
    vector<complexd> buf(params.BufSize());

    MPI_Request request = MPI_REQUEST_NULL;

    MPI_Irecv(&buf[0], buf.size(), MPI_DOUBLE_COMPLEX, master_rank,
        MPI_ANY_TAG, MPI_COMM_WORLD, &request);

    MPI_Status status;
    MPI_Wait(&request, &status);
    psi_it = copy(buf.begin(), buf.end(), psi_it);
    return true;
}

bool WorkerBase::SendNextBuf()
{
    static vector<complexd>::iterator psi_it = psi.begin();
    if (psi_it == psi.end())
    {
        return false;
    }

    for (vector<complexd>::iterator it = buf.begin(); it != buf.end(); it++)
    {
        *it = *psi_it;
        psi_it++;
    }

    MPI_Request request = MPI_REQUEST_NULL;

    MPI_Isend(&buf[0], buf.size(), MPI_DOUBLE_COMPLEX, master_rank,
        MPI_ANY_TAG, MPI_COMM_WORLD, &request);

    return true;
}

