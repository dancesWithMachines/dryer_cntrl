#ifndef HWTIMER_H
#define HWTIMER_H

#include "pico/time.h"
#include <Arduino.h>

typedef void (*TimerCallback)();

class HWTimer {
  public:
    void start(uint8_t hours, TimerCallback cb);
    void stop();
    String getTimeLeft();

  private:
    static constexpr uint64_t tickMs = 60000; // 1 minute
    TimerCallback userCallback = nullptr;
    volatile bool running = false;
    alarm_id_t alarmId = -1;
    uint64_t elapsedMs = 0;
    uint64_t targetMs = 0;
    

    static int64_t alarm_callback(alarm_id_t id, void *user_data);
};

#endif
