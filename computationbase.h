#include "parser.h"
#include "computationparams.h"
#include "typedefs.h"

class ComputationBase
{
    Parser::Args args;
    ComputationParams params;
    vector< vector<complexd> > U;
    public:
    static const int master_rank = 0;
    ComputationBase(const Parser::Args& args);
};
