#include "stats.h"

Index Stats::send_op_counter;
Index Stats::send_data_counter;

void Stats::ResetCounters()
{
    send_op_counter = 0;
    send_data_counter = 0;
}

Index Stats::SendOpCounter()
{
    return send_op_counter;
}

Index Stats::SendDataCounter()
{
    return send_data_counter;
}

void Stats::SendOpCounterInc()
{
    send_op_counter++;
}

void Stats::SendDataCounterAdd(const Index size)
{
    send_data_counter += size;
}
