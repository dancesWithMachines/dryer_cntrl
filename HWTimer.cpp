#include "HWTimer.h"

int64_t HWTimer::alarm_callback(alarm_id_t id, void *user_data) {
    HWTimer* self = static_cast<HWTimer*>(user_data);
    self->elapsedMs += tickMs;

    if (self->elapsedMs >= self->targetMs) {
        self->running = false;
        if (self->userCallback)
          self->userCallback();
        return 0; // Stop
    } else {
        return tickMs * 1000; // Schedule next tick
    }
}

void HWTimer::start(uint8_t hours, TimerCallback cb) {
    stop();
    elapsedMs = 0;
    targetMs = static_cast<uint64_t>(hours) * 60 * 60 * 1000;
    userCallback = cb;
    running = true;

    alarmId = add_alarm_in_us(tickMs * 1000, alarm_callback, this, true);
}

void HWTimer::stop() {
    if (alarmId >= 0) {
        cancel_alarm(alarmId);
        alarmId = -1;
    }
    running = false;
}

String HWTimer::getTimeLeft() {
    if (!running)
      return String("00:00");

    uint64_t remainingMs = (targetMs > elapsedMs) ? (targetMs - elapsedMs) : 0;
    uint32_t minutesLeft = remainingMs / 60000;
    uint8_t hours = minutesLeft / 60;
    uint8_t minutes = minutesLeft % 60;

    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", hours, minutes);
    return String(buf);
}
