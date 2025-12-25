#ifndef JUMBO_H
#define JUMBO_H

#include "FaceFeatures.h"

class Jumbo {
  private:
    int x, y;
    int facesize; 
    
    // Using the new EveEye class
    EveEye leftEye;
    EveEye rightEye;
    
    unsigned long lastBlinkTime;
    bool isBlinking;

  public:
    Jumbo(int _x, int _y, int size) 
      : x(_x), y(_y), facesize(size), 
      
        // Pass 'true' for Left Eye, 'false' for Right Eye
        leftEye(_x + (size * 0.25), _y + (size * 0.45), size * 0.18, size * 0.28, true),
        rightEye(_x + (size * 0.75), _y + (size * 0.45), size * 0.18, size * 0.28, false),
        
        lastBlinkTime(0),
        isBlinking(false)
    {}

    // New Look Logic: Move the WHOLE eye group
    void lookAt(int targetX, int targetY) {
       // Center of the face
       int centerX = x + (facesize / 2); 
       int centerY = y + (facesize / 2); 

       // Calculate distance from center
       int dx = targetX - centerX;
       int dy = targetY - centerY;

       // Scale movement: We don't want the eyes to leave the face.
       // Move 1 pixel for every 10 pixels of target distance.
       // Limit max movement to +/- 5 pixels.
       int offsetX = constrain(dx / 10, -5, 5);
       int offsetY = constrain(dy / 10, -4, 4);

       leftEye.setOffset(offsetX, offsetY);
       rightEye.setOffset(offsetX, offsetY);
    }
    
    void setMood(Mood m) {
      leftEye.setMood(m);
      rightEye.setMood(m);
      // No mouth to update!
    }

    void update() {
       unsigned long now = millis();
       if (!isBlinking && now - lastBlinkTime > (unsigned long)random(2000, 5000)) {
          isBlinking = true;
          lastBlinkTime = now;
       }
       if (isBlinking && now - lastBlinkTime > 150) {
          isBlinking = false;
          lastBlinkTime = now;
       }
       leftEye.isOpen = !isBlinking;
       rightEye.isOpen = !isBlinking;
    }

    void draw(U8G2 &u8g2) {
       leftEye.draw(u8g2);
       rightEye.draw(u8g2);
       // No mouth to draw!
    }
};

#endif