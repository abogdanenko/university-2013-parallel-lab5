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

    string MatrixFileName() const;
    string VectorInputFileName() const;
    string VectorOutputFileName() const;
    string ComputationTimeFileName() const;
    string MatrixFileName() const;
    bool MatrixReadFromFileFlag() const;
    bool VectorReadFromFileFlag() const;
    bool VectorWriteToFileFlag() const;
    bool ComputationTimeWriteToFileFlag() const;
};

