#ifndef FACEFEATURES_H
#define FACEFEATURES_H

#include "Shapes.h"

enum Mood { HAPPY, NEUTRAL, SAD, ANGRY, SLEEPY };

class EveEye {
  private:
    int baseX, baseY;       // Home Position
    int currentX, currentY; // Animated Position
    int width, height;      // Size (Radii)
    bool isLeftEye;         // Helper to know which way to tilt angles
    Mood currentMood;

    // --- INTERNAL EXPRESSION FUNCTIONS ---

    void drawHappyMask(U8G2 &u8g2) {
      // HAPPY: Inverted Arch (Crescent)
      // Math: Draw a Black Circle that overlaps the bottom 50% of the eye
      // We push the masking circle UP slightly to make the smile thinner/sharper
      int maskRadiusX = width * 1.3; // Wider than the eye
      int maskRadiusY = height * 1.1;
      
      // Calculate offset: The higher we place this, the thinner the eye becomes
      int yOffset = height * 0.5; 

      u8g2.setDrawColor(0); // Black/Erase
      u8g2.drawFilledEllipse(currentX, currentY + yOffset + height, maskRadiusX, maskRadiusY);
    }

    void drawAngryMask(U8G2 &u8g2) {
      // ANGRY: Sharp inner slant
      // Math: We draw a Black Triangle to slice off the top-inner corner.
      
      u8g2.setDrawColor(0); // Erase

      // Define vertices for the cutting triangle
      // Point 1: Top Center of eye
      int x1 = currentX; 
      int y1 = currentY - height - 2;

      // Point 2: Inner Edge (Left side for Left Eye, Right side for Right Eye)
      // This determines the "severity" of the angle
      int x2 = isLeftEye ? (currentX + width + 5) : (currentX - width - 5);
      int y2 = currentY; // Cut down to the middle

      // Point 3: Top Corner (Far out) to close the triangle shape
      int x3 = isLeftEye ? (currentX + width + 5) : (currentX - width - 5);
      int y3 = currentY - height - 10;

      u8g2.drawTriangle(x1, y1, x2, y2, x3, y3);
    }

    void drawSadMask(U8G2 &u8g2) {
      // SAD: Drooping outer corners (Puppy dog eyes)
      // Math: Similar to Angry, but we cut the OUTER top corner.

      u8g2.setDrawColor(0); // Erase

      // Point 1: Top Center (Start slightly lower for sadness)
      int x1 = currentX;
      int y1 = currentY - height + 2; 

      // Point 2: Outer Edge
      int x2 = isLeftEye ? (currentX - width - 5) : (currentX + width + 5);
      int y2 = currentY + (height/2); // Slope down further

      // Point 3: Top Outer Corner to close triangle
      int x3 = isLeftEye ? (currentX - width - 5) : (currentX + width + 5);
      int y3 = currentY - height - 10;

      u8g2.drawTriangle(x1, y1, x2, y2, x3, y3);
    }

    void drawSleepyMask(U8G2 &u8g2) {
       // SLEEPY: Heavy Eyelids
       // Math: A simple flat box covering the top 60%
       u8g2.setDrawColor(0);
       u8g2.drawBox(currentX - width, currentY - height, (width * 2) + 1, height + (height/3));
    }

  public:
    bool isOpen;

    // Updated Constructor: Now needs 'isLeft' to calculate angles correctly
    EveEye(int _x, int _y, int _w, int _h, bool _isLeft) 
      : baseX(_x), baseY(_y), width(_w), height(_h), isLeftEye(_isLeft),
        currentMood(NEUTRAL), isOpen(true) {
        currentX = _x;
        currentY = _y;
    }

    void setOffset(int dx, int dy) {
      currentX = baseX + dx;
      currentY = baseY + dy;
    }

    void setMood(Mood m) { currentMood = m; }

    void draw(U8G2 &u8g2) {
      if (!isOpen) {
        // Blink line
        u8g2.setDrawColor(1);
        u8g2.drawLine(currentX - width, currentY, currentX + width, currentY);
        return;
      }

      // 1. Draw Base Eye (White Oval)
      u8g2.setDrawColor(1);
      u8g2.drawFilledEllipse(currentX, currentY, width, height);

      // 2. Apply The Specific Mask
      switch (currentMood) {
        case HAPPY:   drawHappyMask(u8g2); break;
        case ANGRY:   drawAngryMask(u8g2); break;
        case SAD:     drawSadMask(u8g2); break;
        case SLEEPY:  drawSleepyMask(u8g2); break;
        case NEUTRAL: 
          // Neutral can just be the pure oval, or a tiny flat cut on top
          // u8g2.setDrawColor(0);
          // u8g2.drawBox(currentX-width, currentY-height-2, width*2, 4);
          break;
      }
      
      // Reset color for next object
      u8g2.setDrawColor(1); 
    }
};

#endif