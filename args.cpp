#include "args.h"

Args::Args():
    qubit_count(-1),
    epsilon(0.0),
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
