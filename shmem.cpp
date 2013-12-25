#include <dislib.h>
#include <iterator> // std::distance
#include "shmem.h"
#include "stats.h"

#ifdef DEBUG
#include "debug.h"
#endif

using std::distance;

Vector::iterator Shmem::receive_first;

int Shmem::HandlerNumber()
{
    return 1;
}

void Shmem::SetReceiveVector(const Vector::iterator& first)
{
    #ifdef DEBUG
    cout << INDENT(4) << "Shmem::SetReceiveVector()..." << endl;
    #endif
    receive_first = first;
    #ifdef DEBUG
        cout << INDENT(4) << "Shmem::SetReceiveVector() return" << endl;
    #endif
}

void Shmem::SendVector(
    const Vector::const_iterator& first,
    const Vector::const_iterator& last,
    const int dest_pe)
{
    #ifdef DEBUG
    cout << INDENT(4) << "Shmem::SendVector()..." << endl;
    #endif
    for (auto it = first; it != last; it++)
    {
        const Index index = distance(first, it);
        IndexElemPair p(index, *it);
        #ifdef DEBUG
        cout << INDENT(5) << "Index = " << p.first
            << ", Value = " << p.second << endl;
        #endif
        shmem_send(&p, HandlerNumber(), sizeof(p), dest_pe);
        Stats::SendOpCounterInc();
        Stats::SendDataCounterAdd(sizeof(p));
    }
    #ifdef DEBUG
        cout << INDENT(4) << "Shmem::SendVector() return" << endl;
    #endif
}
