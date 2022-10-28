#include "Timer.h"
#include <Arduino.h>

void Timer::startTimer(int msec) {
            timerStart = millis();
            timerTarget = msec;
        }
bool Timer::isTimerReady() {
            return (millis() - timerStart) > timerTarget;
        }
