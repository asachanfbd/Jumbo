#ifndef MOODMANAGER_H
#define MOODMANAGER_H

#include <Arduino.h>
#include "FaceFeatures.h" // To access the Mood enum

class MoodManager {
  private:
    int buttonPin;
    int currentMoodIndex;
    
    // Debounce variables to prevent "double clicks"
    unsigned long lastPressTime;
    int lastState;

  public:
    MoodManager(int pin) {
      buttonPin = pin;
      currentMoodIndex = 0; // Start at HAPPY (0)
      lastPressTime = 0;
      lastState = HIGH; // Button is usually HIGH (Unpressed)
    }

    void begin() {
      // D3 (Flash Button) needs internal pull-up resistor
      pinMode(buttonPin, INPUT_PULLUP); 
    }

    // Call this in your loop. It returns the NEW mood if changed, or -1 if nothing happened.
    int checkInput() {
      int reading = digitalRead(buttonPin);
      unsigned long now = millis();

      // Detect a "Press" (falling edge: HIGH -> LOW)
      // We also enforce a 200ms "cooldown" so you can't cycle too fast
      if (lastState == HIGH && reading == LOW && (now - lastPressTime > 200)) {
        
        lastPressTime = now;
        
        // Cycle the Mood
        // There are 5 moods (0 to 4). (HAPPY, NEUTRAL, SAD, ANGRY, SLEEPY)
        currentMoodIndex++;
        if (currentMoodIndex > 4) {
          currentMoodIndex = 0; // Loop back to HAPPY
        }
        
        lastState = reading; // Remember it's pressed
        return currentMoodIndex;
      }
      
      // Reset state when button is released
      if (reading == HIGH) {
        lastState = HIGH;
      }

      return -1; // No change
    }
    
    // Helper to convert Integer back to Mood Enum
    Mood getCurrentMood() {
      return (Mood)currentMoodIndex;
    }
};

#endif