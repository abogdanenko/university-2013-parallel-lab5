struct ComputationParams
{
    int qubit_count;
    int target_qubit;
    int worker_count; 
    const int max_buf_size;
    public:

    ComputationParams():
        max_buf_size(1024)
    {
        
    }

    int WorkerCount() const
    {
        return worker_count; 
    }

    Index VectorSize() const
    {
        return 1L << qubit_count;
    }

    Index WorkerVectorSize() const
    {
        return VectorSize() / w; 
    }

    int BufSize() const
    {
        return min(max_buf_size, WorkerVectorSize() / 2);
    }

    int BufCount() const
    {
        return WorkerVectorSize() / BufSize();
    }

    int WorkerQubitCount() const
    {
        return intlog2(WorkerVectorSize());
    }

    int GlobalQubitCount() const
    {
        return qubit_count - WorkerQubitCount();
    }

    bool Split() const
    {
        return target_qubit < GlobalQubitCount();
    }

    int WorkerTargetQubit() const
    {
        return max(target_qubit - GlobalQubitCount(), 0);
    }

    Index SliceSize() const
    {
        return 1l << qubit_count - target_qubit;
    }

    Index SliceCount() const
    {
        return 1l << target_qubit;
    }

    int WorkersPerSlice() const
    {
        return worker_count / SliceCount();
    }
};
