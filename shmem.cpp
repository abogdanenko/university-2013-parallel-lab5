#include <dislib.h>
#include <iterator> // std::distance
#include "shmem.h"

#ifdef DEBUG
#include "debug.h"
#endif

using std::distance;

int Shmem::HandlerNumber()
{
    return 1;
}

void Shmem::SetReceiveVector(const Vector::iterator& begin)
{
    #ifdef DEBUG
    cout << INDENT(4) << "Shmem::SetReceiveVector()..." << endl;
    #endif
    receive_begin = begin;
    #ifdef DEBUG
        cout << INDENT(4) << "Shmem::SetReceiveVector() return" << endl;
    #endif
}

void Shmem::SendVector(
    const Vector::iterator& begin,
    const Vector::iterator& end,
    const int dest_pe)
{
    #ifdef DEBUG
    cout << INDENT(4) << "Shmem::SendVector()..." << endl;
    #endif
    for (auto it = begin; it != end; it++)
    {
        const Index index = distance(begin, it);
        IndexElemPair p(*it, index);
        shmem_send(&p, HandlerNumber, sizeof(p), dest_pe);
    }
    #ifdef DEBUG
        cout << INDENT(4) << "Shmem::SendVector() return" << endl;
    #endif
}
