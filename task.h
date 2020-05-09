#ifndef TASK_H
#define TASK_H

/*
 * Trivial Cooperative Tasks
 * Copyright 2020 Jeffrey Anton
 * See LICENSE file
 */
class Task {
  public:
    virtual bool runable(unsigned long) = 0;
    virtual void run() = 0;
    void poll(unsigned long m);
};

class TimedTask : public Task {
    unsigned long nextrun, interval;
  public:
    TimedTask(unsigned long i = 1000);
    bool runable(unsigned long m);
    void setinterval(unsigned int i) { interval = i; }
};

#endif
