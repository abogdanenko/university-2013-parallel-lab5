class Parser
{
    const int argc;
    char** const argv;

    public:

    class Args
    {
        int n; // number of qubits for random state, -1 means 'not specified'
        int k; // target qubit index
        // NULL means 'not specified by user', "-" means 'write to stdout'
        char* x_filename;
        char* y_filename;
        char* U_filename;
        char* T_filename;
    };

    class ParseError: public runtime_error
    {
        public:
        ParseError(string const& msg);
    };

    Parser(const int argc, char** const argv);
    Args Parse();
    static void PrintUsage();
};


