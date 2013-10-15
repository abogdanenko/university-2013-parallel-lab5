class WorkerBase
{
    /* Slice of initial state vector, split into two halves. First half
       corresponds to k-th qubit state 0, second half corresponds to target
       qubit state 1. The vector is modified in-place, so result is also stored
       here  */
    vector<complexd> x; 
    vector< vector<complexd> > U; // transform matrix

    public:
    ApplyOperator();
    ReceiveInstructions();
    ReceiveInputData();
    SendResults();
};

