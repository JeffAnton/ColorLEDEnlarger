#include <Arduino.h>
#include "task.h"

/*
 * Trivial Cooperative Tasks
 * Copyright 2020 Jeffrey Anton
 * See LICENSE file
 */

void
Task::poll(unsigned long m)
{
    if (runable(m))
      run();
}

TimedTask::TimedTask(unsigned long i)
 : interval(i)
{
    nextrun = millis() + i;
}

bool
TimedTask::runable(unsigned long m)
{
  if (nextrun <= m) {
    nextrun += interval;
    return true;
  }
  return false;
}
