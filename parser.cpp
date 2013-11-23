#include <iostream>
#include <sstream> // ostringstream
#include <cctype>
#include <unistd.h> // getopt, optind, optarg

#include "parser.h"
#include "routines.h"

using std::cout;
using std::endl;
using std::hex;
using std::ostringstream;

Parser::ParseError::ParseError(const string& msg):
    runtime_error(msg)
{

}

Parser::Parser(const int argc, char** const argv):
    argc(argc),
    argv(argv)
{

}

void Parser::PrintUsage()
{
    cout << "Usage: transform-each-qubit-shmem ["
            "-n qubit_count "
            "[-e epsilon] "
            "[-U operator_file] "
            "[-x state_vector_file] "
            "[-y state_vector_output_file] "
            "[-t computation_time_output_file]"
        "]" << endl;
}

Args Parser::Parse()
{
    Args result;
    ostringstream oss;
    int c; // option character
    while ((c = getopt(argc, argv, ":n:e:i:t:U:x:y:")) != -1)
    {
        switch(c)
        {
            case 'n':
                result.qubit_count = string_to_number<int>(optarg);
                break;
            case 'e':
                result.epsilon = string_to_number<double>(optarg);
                break;
            case 't':
                result.computation_time_filename = optarg;
                break;
            case 'U':
                result.matrix_filename = optarg;
                break;
            case 'x':
                result.vector_input_filename = optarg;
                break;
            case 'y':
                result.vector_output_filename = optarg;
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

    if (result.qubit_count == -1)
    {
        throw ParseError("Number of qubits not specified");
    }

    return result;
}

