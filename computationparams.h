#ifndef COMPUTATIONPARAMS_H
#define COMPUTATIONPARAMS_H

#include "typedefs.h"

class ComputationParams
{
    // these params don't change during execution
    int qubit_count;
    int global_qubit_count;
    int most_significant_local_qubit;

    Index worker_vector_size;

    // these params change every time target_qubit changes
    int target_qubit;
    int worker_target_qubit;
    bool target_qubit_is_global;

    // these params are defined only when target qubit is global
    int target_qubit_value;
    int partner_rank;

    public:
    ComputationParams(const int qubit_count);
    void SetTargetQubit(const int target_qubit);

    // these params don't change during execution
    Index WorkerVectorSize() const;

    // these params change every time target_qubit changes
    int WorkerTargetQubit() const;
    bool TargetQubitIsGlobal() const;

    // these params are defined only when target qubit is global
    int TargetQubitValue() const;
    int PartnerRank() const;

    #ifdef DEBUG
    void PrintAll() const;
    #endif
};

#endif
