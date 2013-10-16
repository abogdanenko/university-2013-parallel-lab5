#include <mpi.h>

#include "localworker.h"

LocalWorker::LocalWorker(const Args& args):
    WorkerBase(args)
{

}

void LocalWorker::ReceiveMatrix()
{
    vector<complexd> buf(4);

    MPI_Status status;
    MPI_Request request = MPI_REQUEST_NULL;

    MPI_Irecv(&buf[0], 4, MPI_DOUBLE_COMPLEX, master_rank, MPI_ANY_TAG,
        MPI_COMM_SELF, &request);
    MPI_Wait(&request, &status);

    U[0][0] = buf[0];
    U[0][1] = buf[1];
    U[1][0] = buf[2];
    U[1][1] = buf[3];
}

void LocalWorker::Resume()
{
    static State state = STATE_BEGIN;
    // suspend execution, yield control to master
    bool suspend = false; 

    while (state != STATE_END && !suspend)
    {
        switch (state)
        {
            case STATE_BEGIN:
                if (args.MatrixReadFromFileFlag())
                {
                    state = STATE_RECEIVE_MATRIX;
                }
                else
                {
                    if (args.VectorReadFromFileFlag())
                    {
                        state = STATE_RECEIVE_VECTOR;
                        suspend = true;
                    }
                    else
                    {
                        state = STATE_INIT_RANDOM;
                    }
                }
                break;
            case STATE_RECEIVE_MATRIX:
                ReceiveMatrix();
                if (args.VectorReadFromFileFlag())
                {
                    state = STATE_RECEIVE_VECTOR;
                    suspend = true;
                }
                else
                {
                    state = STATE_INIT_RANDOM;
                }
                break;
            case STATE_INIT_RANDOM:
                InitRandom();
                state = STATE_APPLY_OPERATOR;
                break;
            case STATE_RECEIVE_VECTOR:
                if (ReceiveNextBuf())
                {
                    suspend = true;
                }
                else
                {
                    state = STATE_APPLY_OPERATOR;
                }
                break;
            case STATE_APPLY_OPERATOR:
                ApplyOperator();
                if (args.VectorWriteToFileFlag())
                {
                    state = STATE_SEND_VECTOR;
                }
                else
                {
                    state = STATE_END;
                }
                break;
            case STATE_SEND_VECTOR:
                if (SendNextBuf())
                {
                    suspend = true;
                }
                else
                {
                    state = STATE_END;
                }
                break;
            default:
                throw runtime_error("Bad LocalWorker state");
                break;
        }
    }
}

