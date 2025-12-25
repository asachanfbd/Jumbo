#ifndef SEQUENCE_TYPES_H
#define SEQUENCE_TYPES_H

#include <Arduino.h>

struct SequenceStep {
  String expression;     // e.g., "happy", "sad"
  float beepDuration;    // seconds
  String text;           // Display Text
  float displayDuration; // seconds
};

#endif
