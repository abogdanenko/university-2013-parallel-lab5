#ifndef TIMER_H
#define TIMER_H

class Timer
{
    double start; 
    double end; 

    public:
    void Start();
    void Stop();
    double GetDelta() const;
};

#endif
