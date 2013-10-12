class Timer
{
    double start; 
    double end; 

    public:
    void TimerStart();
    void TimerStop();
    double GetDelta() const;
};

