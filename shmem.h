#ifndef SHMEM_H
#define SHMEM_H

#include "typedefs.h"
#include "routines.h"

class Shmem
{
    friend void ShmemReceiveElem(int from, void* data, int sz);
    static Vector::iterator receive_first;
    public:
    static int HandlerNumber();
    static void SetReceiveVector(const Vector::iterator& first);
    static void SendVector(
        const Vector::const_iterator& first,
        const Vector::const_iterator& last,
        const int dest_pe);
};

#endif
