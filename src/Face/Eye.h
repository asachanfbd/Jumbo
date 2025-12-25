#ifndef EYE_H
#define EYE_H

#include "../Shapes.h"
#include <Arduino.h>
#include <math.h>

class Eye : public Shape {
public:
  int radius;
  bool isLeft; // true for Left Eye (Viewer's Perspective), false for Right
  int pupilRadius;
  int pupilOffsetX; // Relative to center
  int pupilOffsetY; // Relative to center

  enum EyeState { STATE_IDLE, STATE_CLOSING, STATE_CLOSED, STATE_OPENING };
  enum Expression {
    EXPR_ANGRY,
    EXPR_HAPPY,
    EXPR_SHOCKED,
    EXPR_SAD,
    EXPR_CALM,
    EXPR_SLEEP
  };

  struct EyelidParams {
    float topOuterOffset; // Factor of radius: -1.0 (top) to 1.0 (bottom)
    float topInnerOffset;
    float bottomOuterOffset;
    float bottomInnerOffset;
    float pupilScale; // 1.0 = normal, <1.0 = constrict, >1.0 = dilate
  };

  EyeState state;
  Expression currentExpr;

  // Animation / Morphing
  EyelidParams currentParams;
  EyelidParams startParams;
  EyelidParams targetParams;
  unsigned long animStartTime;
  int animDuration;
  bool isAnimating;

  float blinkPercent; // 0.0 (open) to 1.0 (closed)
  unsigned long lastStateChangeTime;
  int closeDuration; // ms
  int openDuration;  // ms
  int closedPause;   // ms

  Eye(int _x, int _y, int _r, bool _isLeft)
      : Shape(_x, _y), radius(_r), isLeft(_isLeft), state(STATE_IDLE),
        animDuration(500), isAnimating(false), blinkPercent(0.0),
        closeDuration(80), openDuration(80), closedPause(50) {

    // Default pupil size and position
    pupilRadius = _r / 1.6;
    pupilOffsetX = 0;
    pupilOffsetY = 0;

    // Initial Expression: ANGRY
    setExpression(EXPR_ANGRY, 0);
  }

  // Define the target shapes for each expression
  EyelidParams getParamsForExpression(Expression e) {
    EyelidParams p;
    // Default scale
    p.pupilScale = 1.0;

    if (e == EXPR_ANGRY) {
      // Top lid slants down inwards
      p.topOuterOffset = -0.8; // High up
      p.topInnerOffset = 0.2;  // Low down
      // Bottom lid open (pushed down)
      p.bottomOuterOffset = 1.2;
      p.bottomInnerOffset = 1.2;
    } else if (e == EXPR_HAPPY) {
      // "Laughter": Squinted from bottom up, top slightly down
      // Crescent shape pointing up
      p.topOuterOffset = -0.5;
      p.topInnerOffset = -0.5; // Top lid slightly relaxed/flat

      p.bottomOuterOffset = 0.1; // Cheek pushed up high
      p.bottomInnerOffset = 0.1;
    } else if (e == EXPR_SHOCKED) {
      // SHOCKED: Wide open eyes, small pupils
      // Lids pulled way back
      p.topOuterOffset = -1.2;
      p.topInnerOffset = -1.2;

      p.bottomOuterOffset = 1.2;
      p.bottomInnerOffset = 1.2;

      p.pupilScale = 0.5; // Constricted pupil
    } else if (e == EXPR_SAD) {
      // SAD: Puppy dog eyes
      // Outer corners droop down significantly
      // Inner corners go UP slightly or stay high

      p.topOuterOffset = 0.5;  // Droop down outer
      p.topInnerOffset = -0.8; // High inner (brows go up in middle)

      p.bottomOuterOffset = 0.5; // Droop down outer
      p.bottomInnerOffset = 0.8; // Lower inner

      p.pupilScale = 1.0;
    } else if (e == EXPR_CALM) {
      // CALM / SATISFIED
      // Relaxed eyelids. Not sleepy, but peaceful.
      // Top lid lowers slightly more than neutral
      p.topOuterOffset = -0.1;
      p.topInnerOffset = -0.1;

      // Bottom lid relaxed (pushed down)
      p.bottomOuterOffset = 0.8;
      p.bottomInnerOffset = 0.8;

      p.pupilScale = 1.0;
    } else if (e == EXPR_SLEEP) {
      // SLEEP: Eyes closed
      p.topOuterOffset = 0.8; // Fully down
      p.topInnerOffset = 0.8;
      p.bottomOuterOffset =
          0.8; // Lower lid meets it (or stays down if top comes all way)
      // If top goes to 0.8 (near bottom), and bottom is 0.8, they meet?
      // Let's set top to 1.0 (bottom edge) and bottom to 1.0.
      p.topOuterOffset = 1.0;
      p.topInnerOffset = 1.0;
      p.bottomOuterOffset = 1.0;
      p.bottomInnerOffset = 1.0;
      p.pupilScale = 1.0;
    }
    return p;
  }

  void setExpression(Expression e, int duration = 500) {
    currentExpr = e;
    targetParams = getParamsForExpression(e);
    startParams = currentParams; // Start from wherever we are
    animStartTime = millis();
    animDuration = duration;
    isAnimating = true;

    // If duration is 0, snap immediately
    if (duration == 0) {
      currentParams = targetParams;
      isAnimating = false;
    }
  }

  void blink() {
    if (state == STATE_IDLE) {
      state = STATE_CLOSING;
      lastStateChangeTime = millis();
    }
  }

  void update() {
    unsigned long now = millis();

    // 1. Handle Expression Morphing
    if (isAnimating) {
      float t = (float)(now - animStartTime) / animDuration;
      if (t >= 1.0) {
        t = 1.0;
        isAnimating = false;
        currentParams = targetParams;
      }

      // Lerp parameters
      // A simple linear interpolation for each float
      currentParams.topOuterOffset =
          startParams.topOuterOffset +
          (targetParams.topOuterOffset - startParams.topOuterOffset) * t;
      currentParams.topInnerOffset =
          startParams.topInnerOffset +
          (targetParams.topInnerOffset - startParams.topInnerOffset) * t;
      currentParams.bottomOuterOffset =
          startParams.bottomOuterOffset +
          (targetParams.bottomOuterOffset - startParams.bottomOuterOffset) * t;
      currentParams.bottomInnerOffset =
          startParams.bottomInnerOffset +
          (targetParams.bottomInnerOffset - startParams.bottomInnerOffset) * t;

      // Lerp Pupil Scale
      currentParams.pupilScale =
          startParams.pupilScale +
          (targetParams.pupilScale - startParams.pupilScale) * t;
    }

    // Force closed if SLEEP
    if (currentExpr == EXPR_SLEEP) {
      state = STATE_CLOSED;
      blinkPercent = 1.0;
      return; // Skip normal blink state machine
    }

    // 2. Handle Blinking
    switch (state) {
    case STATE_IDLE:
      blinkPercent = 0.0;
      break;

    case STATE_CLOSING: {
      float progress = (float)(now - lastStateChangeTime) / closeDuration;
      if (progress >= 1.0) {
        progress = 1.0;
        state = STATE_CLOSED;
        lastStateChangeTime = now;
      }
      blinkPercent = progress;
      break;
    }

    case STATE_CLOSED:
      blinkPercent = 1.0;
      if (now - lastStateChangeTime >= (unsigned long)closedPause) {
        state = STATE_OPENING;
        lastStateChangeTime = now;
      }
      break;

    case STATE_OPENING: {
      float progress = (float)(now - lastStateChangeTime) / openDuration;
      if (progress >= 1.0) {
        progress = 1.0;
        state = STATE_IDLE;
      }
      blinkPercent = 1.0 - progress;
      break;
    }
    }
  }

  void lookAt(int targetX, int targetY) {
    // Simple look logic: constrain pupil within the eye
    int dx = targetX - x;
    int dy = targetY - y;
    float dist = sqrt(dx * dx + dy * dy);
    int maxDist = radius - pupilRadius - 2;

    if (dist > maxDist && dist > 0) {
      float ratio = maxDist / dist;
      pupilOffsetX = dx * ratio;
      pupilOffsetY = dy * ratio;
    } else {
      pupilOffsetX = dx;
      pupilOffsetY = dy;
    }
  }

  void draw(U8G2 &u8g2) override {
    // Animation: Laughter Jiggle
    // If Happy, add a subtle random Y offset to the whole eye or pupil?
    // "create a laughter expression... clearly show the laughter animation"
    // Let's bounce the whole eye content slightly up and down
    int bounceY = 0;
    if (currentExpr == EXPR_HAPPY && !isAnimating) {
      // Fast subtle bounce
      bounceY = (sin(millis() / 50.0) * 1.5); // +/- 1 or 2 pixels
    }

    int drawX = x;
    int drawY = y + bounceY;

    // 1. Draw the Sclera
    u8g2.setDrawColor(1);
    u8g2.drawDisc(drawX, drawY, radius);

    // 2. Draw the Pupil
    if (blinkPercent < 1.0) {
      u8g2.setDrawColor(0);
      // Use the current interpolated pupil scale
      int currentPupilRadius = (int)(pupilRadius * currentParams.pupilScale);
      u8g2.drawDisc(drawX + pupilOffsetX, drawY + pupilOffsetY,
                    currentPupilRadius);
    }

    // 3. Draw Eyelids (Masks)
    u8g2.setDrawColor(0); // Black

    // We assume "Inner" is towards nose, "Outer" towards ear.
    // Left Eye: Inner is Right side, Outer is Left side.
    // Right Eye: Inner is Left side, Outer is Right side.

    int tOuterY = drawY + (radius * currentParams.topOuterOffset);
    int tInnerY = drawY + (radius * currentParams.topInnerOffset);
    int bOuterY = drawY + (radius * currentParams.bottomOuterOffset);
    int bInnerY = drawY + (radius * currentParams.bottomInnerOffset);

    int outerX, innerX;

    if (isLeft) {
      outerX = drawX - radius - 5;
      innerX = drawX + radius + 5;
    } else {
      innerX = drawX - radius - 5;
      outerX = drawX + radius + 5;
    }

    // Top Mask Polygon
    int maskTopY = drawY - radius - 10;
    // Four points: TopOuter, TopInner, InnerLidPoint, OuterLidPoint
    // We draw a box/polygon clearing everything above the lid line.
    u8g2.drawTriangle(outerX, maskTopY, innerX, maskTopY, innerX, tInnerY);
    u8g2.drawTriangle(outerX, maskTopY, innerX, tInnerY, outerX, tOuterY);

    // Bottom Mask Polygon
    int maskBottomY = drawY + radius + 10;
    // Clearing everything below the bottom lid line
    u8g2.drawTriangle(outerX, maskBottomY, innerX, maskBottomY, innerX,
                      bInnerY);
    u8g2.drawTriangle(outerX, maskBottomY, innerX, bInnerY, outerX, bOuterY);

    // 4. Draw Blink Eyelid (Box)
    if (blinkPercent > 0.0) {
      u8g2.setDrawColor(0);
      int boxHeight = (2 * radius) * blinkPercent;
      int boxTop = drawY - radius;
      int boxWidth = (2 * radius) + 4;
      u8g2.drawBox(drawX - radius - 2, boxTop, boxWidth, boxHeight);
    }
  }
};

#endif
