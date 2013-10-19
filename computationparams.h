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
    int MostSignificantLocalQubit() const;
    Index VectorSize() const;

    public:
    ComputationParams(const int qubit_count, const int target_qubit);
    int WorkerCount() const;
    Index WorkerVectorSize() const;
    int WorkerTargetQubit() const;

    int BufSize() const;
    int BufCount() const;

    bool Split() const;
    int SliceCount() const;
    int WorkersPerSlice() const;

    #ifdef DEBUG
    void PrintAll() const;
    #endif
};

#endif
