#ifndef ARGS_H
#define ARGS_H

#include <string>

using std::string;

class Args
{
    friend class Parser;

    int qubit_count; 
    int target_qubit; 
    // NULL means 'not specified by user', "-" means 'write to stdout'
    char* vector_input_filename;
    char* vector_output_filename;
    char* matrix_filename;
    char* computation_time_filename;
    
    public:

    Args();
    int QubitCount() const;
    int TargetQubit() const;
    string MatrixFileName() const;
    string VectorInputFileName() const;
    string VectorOutputFileName() const;
    string ComputationTimeFileName() const;
    bool MatrixReadFromFileFlag() const;
    bool VectorReadFromFileFlag() const;
    bool VectorWriteToFileFlag() const;
    bool ComputationTimeWriteToFileFlag() const;
};

#endif
