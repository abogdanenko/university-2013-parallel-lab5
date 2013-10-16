int string_to_int(const string& s)
{
    int n;
    stringstream ss(s);
    ss >> n;
    return n;
}

Parser::ParseError::ParseError(const string& msg):
    runtime_error(msg)
{

}

Parser::Args::Args():
    n(-1), 
    k(1), 
    vector_input_filename(NULL),
    vector_output_filename(NULL),
    matrix_filename(NULL),
    computation_time_filename(NULL)
{

}

string Parser::Args::MatrixFileName() const
{
    return matrix_filename;
}

string Parser::Args::VectorInputFileName() const
{
    return vector_input_filename;
}

string Parser::Args::VectorOutputFileName() const
{
    return vector_output_filename;
}

string Parser::Args::ComputationTimeFileName() const
{
    return computation_time_filename;
}

string Parser::Args::MatrixFileName() const
{
    return matrix_filename;
}

bool Parser::Args::MatrixReadFromFileFlag() const
{
    return matrix_filename;
}

bool Parser::Args::VectorReadFromFileFlag() const
{
    return vector_input_filename;
}

bool Parser::Args::VectorWriteToFileFlag() const
{
    return vector_output_filename;
}

bool Parser::Args::ComputationTimeWriteToFileFlag() const
{
    return computation_time_filename;
}

void Parser::PrintUsage()
{
    cout << "Usage: transform-1-qubit [[-U operator_file] "
        "{-x state_vector_file | -n random_state_qubits_count} "
        "[-k target_qubit] [-y state_vector_output_file] "
        "[-T computation_time_output_file]]" << endl;
}

Parser::Args Parser::Parse()
{
    Args result;
    ostringstream oss;
    int c; // option character
    while ((c = getopt(argc, argv, ":U:x:n:k:t:y:T:")) != -1)
    {
        switch(c)
        {
            case 'U':
                result.matrix_filename = optarg;
                break;
            case 'x':
                result.vector_input_filename = optarg;
                break;
            case 'n':
                result.qubit_count = string_to_int(optarg);
                break;
            case 'k':
                result.target_qubit = string_to_int(optarg);
                break;
            case 'y':
                result.vector_output_filename = optarg;
                break;
            case 'T':
                result.computation_time_filename = optarg;
                break;
            case ':':
                oss << "Option -" << char(optopt) << " requires an argument.";
                throw ParseError(oss.str());
            case '?':
                if (isprint(optopt))
                {
                    oss << "Unknown option `-" << char(optopt) << "'.";
                }
                else
                {
                    oss << "Unknown option character `\\x" << hex << optopt <<
                        "'.";
                }
                throw ParseError(oss.str());
            default:
                throw ParseError("An error occured while parsing options.");
        }
    }

    if (optind > argc)
    {
        throw ParseError("Extra non-option arguments found");
    }

    if (n == -1)
    {
        throw ParseError("Number of qubits not specified");
    }
    return result;
}

