#ifndef SHMEM_H
#define SHMEM_H

#include "typedefs.h"
#include "routines.h"

class Shmem
{
    static Vector::iterator receive_begin;
    public:
    static int HandlerNumber();
    static void SetReceiveVector(const Vector::iterator& begin);
    static void SendVector(
        const Vector::iterator& begin,
        const Vector::iterator& end,
        const int dest_pe);
};

#endif
