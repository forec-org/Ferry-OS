//
// Created by 王耀 on 2017/9/15.
//

#ifndef SDOS_TIMER_H
#define SDOS_TIMER_H

class Timer {
private:
    bool m_started;
    bool m_ticking;
    bool m_ready;
    long m_tick;
    long m_bound;
    int m_priority;
    void (*m_callback)(void *args, void *result = nullptr);
    void ticking();
public:
    Timer(int priority);
    ~Timer();
    void init(long bound, void (*callback)(void *args, void *result = nullptr));
    void start();
    void pause();
    void resume();
    void stop();
};

#endif //SDOS_TIMER_H
