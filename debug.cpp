#ifdef WAITFORGDB
#include "debug.h"
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
#endif
