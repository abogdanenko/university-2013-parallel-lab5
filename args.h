#ifndef ARGS_H
#define ARGS_H

#include <string>

using std::string;

class Args
{
    friend class Parser;

    int qubit_count;
    double epsilon;
    // NULL means 'not specified by user', "-" means 'write to stdout'
    char* fidelity_filename;
    char* computation_time_filename;

    public:

    Args();
    int QubitCount() const;
    double Epsilon() const;
    string FidelityFileName() const;
    bool FidelityWriteToFileFlag() const;
    string ComputationTimeFileName() const;
    bool ComputationTimeWriteToFileFlag() const;
};

#endif
