#include "SensorManager.h"

// Keep the IRAM interrupt handler out of line. An inline COMDAT handler can
// place Xtensa literal pools after their l32r references with recent GCC.
void IRAM_ATTR SensorManager::echoEdge(void* arg) {
    auto* self = static_cast<SensorManager*>(arg);
    uint32_t now = micros();
    bool high = digitalRead(PIN_ECHO);
    portENTER_CRITICAL_ISR(&self->_echoMux);
    if (self->_armed) {
      if (now - self->_triggerUs > self->_echoTimeoutUs) {
        self->_completedMs = millis();
        self->_pulseUs = 0; self->_complete = true; self->_armed = false;
      } else if (high) { self->_riseUs = now; self->_rose = true; }
      else if (self->_rose) {
        self->_completedMs = millis();
        self->_pulseUs = now - self->_riseUs;
        self->_complete = true; self->_armed = false;
      }
    }
    portEXIT_CRITICAL_ISR(&self->_echoMux);
  }
