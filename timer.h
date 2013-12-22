#ifndef TIMER_H
#define TIMER_H

class Timer
{
    double start;
    double sum;

    public:
    Timer();
    void Start();
    void Stop();
    double Total() const;
};

#endif
