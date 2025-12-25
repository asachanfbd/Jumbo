#ifndef JUMBOCONTROLLER_H
#define JUMBOCONTROLLER_H

#include "../Face/Eye.h"
#include "../Sequence/SequenceQueue.h"
#include "../SoundManager.h"
#include "../TextBox.h"
#include <Arduino.h>
#include <U8g2lib.h>

class JumboController {
private:
  U8G2 &u8g2; // Reference to display driver

  // Eyes
  Eye leftEye;
  Eye rightEye;

  // Text Box
  TextBox statusBox;
  TextBox captionBox;

  // Sound
  SoundManager buzzer;

  // Logic State
  SequenceQueue &queue; // Reference to the shared queue

  bool isPlayingStep;
  unsigned long stepStartTime;
  // Cache current step properties
  float currentDisplayDuration;

  // Helper to Convert String to Expression Enum
  Eye::Expression getExpressionFromString(String s) {
    s.toLowerCase();
    if (s == "happy")
      return Eye::EXPR_HAPPY;
    if (s == "shocked")
      return Eye::EXPR_SHOCKED;
    if (s == "sad")
      return Eye::EXPR_SAD;
    if (s == "calm")
      return Eye::EXPR_CALM;
    if (s == "sleep")
      return Eye::EXPR_SLEEP;
    return Eye::EXPR_ANGRY;
  }

public:
  JumboController(U8G2 &_u8g2, SequenceQueue &_queue, int buzzerPin)
      : u8g2(_u8g2), queue(_queue), leftEye(32, 26, 20, true),
        rightEye(96, 26, 20, false), statusBox(0, 0, 12, 128, ALIGN_CENTER),
        captionBox(0, 50, 12, 128, ALIGN_LEFT), buzzer(buzzerPin),
        isPlayingStep(false), stepStartTime(0) {
    // Initial State
    leftEye.setExpression(Eye::EXPR_SLEEP, 0);
    rightEye.setExpression(Eye::EXPR_SLEEP, 0);
    statusBox.setText("Booting...");
  }

  void begin() {
    // Any init logic
  }

  void update() {
    unsigned long now = millis();
    SequenceStep currentStep;

    // 1. Check State Machine
    if (isPlayingStep) {
      // Check if duration expired
      if (now - stepStartTime >=
          (unsigned long)(currentDisplayDuration * 1000)) {
        // Step Finished
        isPlayingStep = false;
        queue.pop(); // Remove the finished step
      }
    }

    // 2. Try to start next step if idle
    if (!isPlayingStep) {
      if (queue.peek(currentStep)) {
        // START NEW STEP
        isPlayingStep = true;
        stepStartTime = now;
        currentDisplayDuration = currentStep.displayDuration;

        // Apply Effects
        Eye::Expression expr = getExpressionFromString(currentStep.expression);
        leftEye.setExpression(expr, 500);
        rightEye.setExpression(expr, 500);

        captionBox.setText(currentStep.text);

        if (currentStep.beepDuration > 0) {
          buzzer.beep((int)(currentStep.beepDuration * 1000));
        }
      } else {
        // IDLE / SLEEP STATE
        // If we became empty just now, go to sleep
        // We can check current expression to see if we need to switch
        if (leftEye.currentExpr != Eye::EXPR_SLEEP) {
          leftEye.setExpression(Eye::EXPR_SLEEP, 1000);
          rightEye.setExpression(Eye::EXPR_SLEEP, 1000);
          statusBox.setText("Sleeping...");
        }
      }
    }

    // 3. Update Components
    // Blink logic only if NOT sleeping
    if (leftEye.currentExpr != Eye::EXPR_SLEEP) {
      if (random(0, 1000) < 15) { // 1.5% chance
        leftEye.blink();
        rightEye.blink();
      }
    }

    leftEye.update();
    rightEye.update();
    buzzer.update();
  }

  void setExpression(Eye::Expression e, int duration) {
    leftEye.setExpression(e, duration);
    rightEye.setExpression(e, duration);
  }

  void setText(String s) { statusBox.setText(s); }

  void draw() {
    u8g2.clearBuffer();
    leftEye.draw(u8g2);
    rightEye.draw(u8g2);

    if (isPlayingStep) {
      captionBox.draw(u8g2);
    } else {
      statusBox.draw(u8g2);
    }

    u8g2.sendBuffer();
  }

  void forceSleep() {
    // 1. Set Eyes to Sleep immediately
    leftEye.setExpression(Eye::EXPR_SLEEP, 0);
    rightEye.setExpression(Eye::EXPR_SLEEP, 0);

    // 2. Set Status Text
    statusBox.setText("Sleeping...");

    // 3. Force Draw immediately to update screen before loop pauses
    draw();
  }
};

#endif
