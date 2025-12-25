#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include "Config.h"
#include "Manager/JumboController.h"
#include "Network/APIClient.h"
#include "Sequence/SequenceQueue.h"

// U8g2 Constructor
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, D1, D2);

// 1. Shared Sequence Queue
SequenceQueue sequenceQueue;

// 2. Jumbo Controller (Owns Eyes, Display, Buzzer)
JumboController controller(u8g2, sequenceQueue, D5); // Buzzer on D5

// 3. API Client (Fetches data into Queue)
APIClient apiClient(sequenceQueue);

// 4. Standby State
bool isStandby = false;
unsigned long lastButtonPress = 0;

void setup() {
  // Initialize Display
  u8g2.begin();

  // Initialize Flash Button
  pinMode(FLASH_BUTTON_PIN, INPUT_PULLUP);

  // Initialize Components
  controller.begin();

  // Wire up granular debug logging
  apiClient.setStatusCallback([](const String &msg) {
    controller.setText(msg);
    controller.draw();
    delay(10); // Yield to let display refresh
  });

  apiClient.begin();
}

void loop() {
  // 1. Handle Sleep/Resume Button
  if (digitalRead(FLASH_BUTTON_PIN) == LOW) {
    if (millis() - lastButtonPress > 500) {
      isStandby = !isStandby;
      lastButtonPress = millis();

      if (isStandby) {
        // Entering Sleep
        controller.forceSleep();
      } else {
        // Waking Up
        // No special action needed, loop will resume updates
        // Maybe restore "IP..." or "Wake" text?
        // Let's just let it resume whatever was happening or go to idle
        controller.setText("Resuming...");
      }
    }
  }

  if (isStandby) {
    // In Standby, we don't update network or controller logic
    // We just keep drawing the static sleep frame (handled by controller state)
    // Actually controller.draw() is called at end, so we just skip updates
    delay(100); // Slow down loop to save power/cpu
  } else {
    // 2. Regular Updates
    apiClient.update();
    controller.update();
  }

  // 3. Override Text during Boot (Only if not sleeping to avoid fighting)

  // 3. Override Text during Boot
  if (!apiClient.isBootComplete()) {
    controller.setText(apiClient.getBootStatus());
  }

  // 4. Draw Frame
  controller.draw();
}