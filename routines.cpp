#include "routines.h"
#include "shmem.h"
#include <dislib.h>
#include <time.h> // time
#include <unistd.h> // getpid

#ifdef DEBUG
#include <iomanip> // setw, setfill
#include "debug.h"
#endif

#ifdef DEBUG
using std::hex;
using std::setfill;
using std::setw;
using std::string;
using std::endl;
#endif

void ShmemReceiveElem(int from, void* data, int sz)
{
    #ifdef DEBUG
    cout << "::ShmemReceiveElem()..." << endl;
    #endif
    const IndexElemPair* p = (IndexElemPair*) data;
    *(Shmem::receive_first + p->first) = p->second;
    #ifdef DEBUG
        cout << INDENT(1) << "Index = " << p->first
            << ", Value = " << p->second << endl;
        cout << "::ShmemReceiveElem() return" << endl;
    #endif
}

void ShmemBarrierAll()
{
    #ifdef DEBUG
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
    #endif

    shmem_barrier_all();
}

Matrix MatrixMultiply(const Matrix& A, const Matrix& B)
{
    const int rows_A = A.size();
    const int columns_A = A[0].size();
    const int columns_B = B[0].size();
    const int rows = rows_A;
    const int columns = columns_B;
    const Vector row(columns, complexd(0.0, 0.0));
    Matrix C(rows, row);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            for (int k = 0; k < columns_A; k++)
            {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return C;
}

unsigned GetUniqueSeed()
{
    #ifdef DEBUG
    cout << "GetUniqueSeed()..." << endl;
    #endif

    unsigned pid = (unsigned) getpid();

    // something like 0x10000
    const unsigned half = 1
        << (8 * sizeof(unsigned) / 2); // half bit length

    // something like 0xpprrtttt
    const unsigned seed = ((pid % 0x100) * 0x100
        + shmem_my_pe() % 0x100) * half + time(NULL) % half;

    #ifdef DEBUG
    cout << INDENT(1) << "seed = " << "0x" << setfill('0')
        << setw(2 * sizeof(unsigned)) // two hex digits per byte
        << hex << seed << endl;
    cout << "GetUniqueSeed() return" << endl;
    #endif
    return seed;
}
