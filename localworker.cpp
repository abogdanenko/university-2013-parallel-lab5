void LocalWorker::Resume()
{
    static State state = STATE_RECEIVE_INPUT_DATA;
    // suspend execution, yield control to master
    bool suspend = false; 

    while (state != STATE_END && !suspend)
    {
        switch (state)
        {
            case STATE_RECEIVE_INPUT_DATA:
                ReceiveInputData();
                if (random)
                {
                    InitRandom();
                    state = STATE_APPLY_OPERATOR;
                    suspend = false;
                }
                else
                {
                    state = STATE_RECEIVE_MY_SLICE;
                    suspend = true;
                }
                break;
            case STATE_INIT_RANDOM:
                InitRandom();
                state = STATE_APPLY_OPERATOR;
                suspend = false;
                break;
            case STATE_RECEIVE_MY_SLICE:
                if (ReceiveNextBuf())
                {
                    suspend = true;
                }
                else
                {
                    state = STATE_APPLY_OPERATOR;
                    suspend = false;
                }
                break;
            case STATE_APPLY_OPERATOR:
                ApplyOperator();
                if (write_vector_to_file)
                {
                    state = STATE_SEND_MY_SLICE;
                }
                else
                {
                    state = STATE_END;
                }
                suspend = false;
                break;
            case STATE_SEND_MY_SLICE:
                if (SendNextBuf())
                {
                    suspend = true;
                }
                else
                {
                    state = STATE_END;
                    suspend = false;
                }
                break;
            default:
                throw runtime_exception("Bad LocalWorker state");
                break;
        }
    }
}

