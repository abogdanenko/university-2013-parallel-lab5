LocalWorker::LocalWorker():
    current_state(STATE_RECEIVE_INPUT_DATA)
{

}

void LocalWorker::Resume()
{
    // suspend execution, yield control to master
    bool suspend = false; 

    while (current_state != STATE_END && !suspend)
    {
        switch (current_state)
        {
            case STATE_RECEIVE_INPUT_DATA:
                ReceiveInputData();
                if (random)
                {
                    InitRandom();
                    current_state = STATE_APPLY_OPERATOR;
                    suspend = false;
                }
                else
                {
                    current_state = STATE_RECEIVE_MY_SLICE;
                    suspend = true;
                }
                break;
            case STATE_INIT_RANDOM:
                InitRandom();
                current_state = STATE_APPLY_OPERATOR;
                suspend = false;
                break;
            case STATE_RECEIVE_MY_SLICE:
                if (ReceiveNextBuf())
                {
                    suspend = true;
                }
                else
                {
                    current_state = STATE_APPLY_OPERATOR;
                    suspend = false;
                }
                break;
            case STATE_APPLY_OPERATOR:
                ApplyOperator();
                if (write_vector_to_file)
                {
                    current_state = STATE_SEND_MY_SLICE;
                }
                else
                {
                    current_state = STATE_END;
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
                    current_state = STATE_END;
                    suspend = false;
                }
                break;
            default:
                throw runtime_exception("Bad LocalWorker state");
                break;
        }
    }
}

