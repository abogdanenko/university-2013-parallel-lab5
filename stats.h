#ifndef STATS_H
#define STATS_H

#include "typedefs.h"

class Stats
{
    static Index send_op_counter;
    static Index send_data_counter;
    public:
    static void ResetCounters();
    static Index SendOpCounter();
    static Index SendDataCounter();
    static void SendOpCounterInc();
    static void SendDataCounterAdd(const Index size);
};

#endif
