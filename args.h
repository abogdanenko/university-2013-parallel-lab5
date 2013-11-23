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
    char* vector_input_filename;
    char* vector_output_filename;
    char* matrix_filename;
    char* computation_time_filename;

    public:

    Args();
    int QubitCount() const;
    double Epsilon() const;
    string ComputationTimeFileName() const;
    bool ComputationTimeWriteToFileFlag() const;
    string MatrixFileName() const;
    bool MatrixReadFromFileFlag() const;
    string VectorInputFileName() const;
    string VectorOutputFileName() const;
    bool VectorReadFromFileFlag() const;
    bool VectorWriteToFileFlag() const;
};

#endif
