#ifndef COMPUTATIONPARAMS_H
#define COMPUTATIONPARAMS_H

#include "typedefs.h"

class ComputationParams
{
    const int qubit_count;
    const int target_qubit;
    const int worker_count; 
    const int max_buf_size;

    static int MPIGetWorldSize();

    public:
    ComputationParams(const int qubit_count, const int target_qubit);
    int WorkerCount() const;
    Index VectorSize() const;
    Index WorkerVectorSize() const;
    int BufSize() const;
    int BufCount() const;
    int WorkerQubitCount() const;
    int GlobalQubitCount() const;
    bool Split() const;
    int WorkerTargetQubit() const;
    Index SliceSize() const;
    int SliceCount() const;
    int WorkersPerSlice() const;
};

#endif
