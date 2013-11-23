#include "args.h"

Args::Args():
    qubit_count(-1),
    epsilon(0.0),
    vector_input_filename(NULL),
    vector_output_filename(NULL),
    matrix_filename(NULL),
    computation_time_filename(NULL)
{

}

int Args::QubitCount() const
{
    return qubit_count;
}

double Args::Epsilon() const
{
    return epsilon;
}

string Args::ComputationTimeFileName() const
{
    return computation_time_filename;
}

bool Args::ComputationTimeWriteToFileFlag() const
{
    return computation_time_filename;
}

string Args::MatrixFileName() const
{
    return matrix_filename;
}

bool Args::MatrixReadFromFileFlag() const
{
    return matrix_filename;
}

string Args::VectorInputFileName() const
{
    return vector_input_filename;
}

string Args::VectorOutputFileName() const
{
    return vector_output_filename;
}

bool Args::VectorReadFromFileFlag() const
{
    return vector_input_filename;
}

bool Args::VectorWriteToFileFlag() const
{
    return vector_output_filename;
}

