#ifndef TIMER_H
#define TIMER_H

class Timer
{
    double start; 
    double end; 

    public:
    void TimerStart();
    void TimerStop();
    double GetDelta() const;
};

#endif
