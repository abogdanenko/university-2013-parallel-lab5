#ifdef DEBUG
#include "debug.h"

using std::setw;

#ifdef WAITFORGDB
#include <unistd.h> // sleep
void WaitForGdb()
{
    const int t = 5; // seconds
    int flag = 1;
    while (flag)
    {
        sleep(t);
    }
}
#endif // WAITFORGDB

void PrintBarrierCount()
{
    static int count = 0;
    const int len = 20;
    const char c = '-';
    const string line (len, c);
    cout
        << line
        << "barrier "
        << setw(6)
        << count
        << endl;

    count++;
}

#endif // DEBUG
