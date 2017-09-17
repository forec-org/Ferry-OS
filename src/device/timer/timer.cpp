//
// Created by 王耀 on 2017/9/16.
//

#include "../../../include/device/timer/timer.h"

Timer::Timer(int priority): m_priority(priority) {
    m_tick = m_bound = 0;
    m_started = false;
    m_ready = false;
    m_ticking = false;
}

Timer::~Timer() {
    // TODO
}

void Timer::init(long bound, void (*callback)(void *, void *)) {
    m_bound = bound;
    m_callback = callback;
    m_tick = 0;
    m_ready = false;
    m_ticking = false;
    m_started = false;
}

void Timer::start() {
    // TODO
    m_ticking = true;
}

void Timer::pause() {
    // TODO
}

void Timer::resume() {
    // TODO
}

void Timer::stop() {
    // TODO
}