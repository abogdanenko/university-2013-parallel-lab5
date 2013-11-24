#ifndef SHMEM_H
#define SHMEM_H

#include "typedefs.h"
#include "routines.h"

class Shmem
{
    friend void ShmemReceiveElem(int from, void* data, int sz);
    static Vector::iterator receive_begin;
    public:
    static int HandlerNumber();
    static void SetReceiveVector(const Vector::iterator& begin);
    static void SendVector(
        const Vector::const_iterator& begin,
        const Vector::const_iterator& end,
        const int dest_pe);
};

#endif
