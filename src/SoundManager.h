#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <Arduino.h>

class SoundManager {
  private:
    int pin;
    unsigned long beepStartTime;
    int beepDuration;
    bool isBeeping;
    bool initialized;

    void init() {
      if (!initialized) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW); // Ensure it starts silent (Active Low/High check)
        initialized = true;
      }
    }

  public:
    SoundManager(int _pin) : pin(_pin), isBeeping(false), initialized(false) {}

    // REMOVED 'frequency' because active buzzers only have one tone!
    void beep(int duration) {
      init();
      
      if (isBeeping) return; 
      
      digitalWrite(pin, HIGH); // Turn sound ON
      beepStartTime = millis();
      beepDuration = duration;
      isBeeping = true;
    }

    void update() {
      if (isBeeping && (millis() - beepStartTime >= (unsigned long)beepDuration)) {
        digitalWrite(pin, LOW); // Turn sound OFF
        isBeeping = false;
      }
    }
};

#endif