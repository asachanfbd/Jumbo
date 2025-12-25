#ifndef SHAPES_H
#define SHAPES_H

#include <U8g2lib.h>
#include <math.h>

// Abstract Base Class
class Shape {
public:
  int x, y;
  Shape(int _x, int _y) : x(_x), y(_y) {}
  virtual void draw(U8G2 &u8g2) = 0;
};

// A Solid Circle (Disc)
class Disc : public Shape {
public:
  int radius;
  bool isBlack; // To "erase" pixels or draw pupils

  Disc(int _x, int _y, int _r, bool _isBlack = false)
      : Shape(_x, _y), radius(_r), isBlack(_isBlack) {}

  void draw(U8G2 &u8g2) override {
    u8g2.setDrawColor(isBlack ? 0 : 1);
    u8g2.drawDisc(x, y, radius);
    u8g2.setDrawColor(1); // Reset to default
  }
};

// A Rounded Rectangle (The Body)
class RoundedBox : public Shape {
public:
  int width, height, radius;

  RoundedBox(int _x, int _y, int _w, int _h, int _r)
      : Shape(_x, _y), width(_w), height(_h), radius(_r) {}

  void draw(U8G2 &u8g2) override {
    u8g2.drawRFrame(x, y, width, height, radius);
  }
};

// A Simple Line (For closed eyes)
class Line : public Shape {
public:
  int x2, y2;

  Line(int _x1, int _y1, int _x2, int _y2)
      : Shape(_x1, _y1), x2(_x2), y2(_y2) {}

  void draw(U8G2 &u8g2) override { u8g2.drawLine(x, y, x2, y2); }
};

// Infinity Symbol (Lemniscate of Bernoulli)
class InfinityShape : public Shape {
public:
  int size; // Controls the overall spread

  InfinityShape(int _x, int _y, int _size) : Shape(_x, _y), size(_size) {}

  void draw(U8G2 &u8g2) override {
    float step = 0.1;
    float oldX = x + (size * cos(0)) / (1 + sin(0) * sin(0));
    float oldY = y + (size * cos(0) * sin(0)) / (1 + sin(0) * sin(0));

    for (float t = step; t <= 2 * M_PI; t += step) {
      float den = 1 + sin(t) * sin(t);
      float newX = x + (size * cos(t)) / den;
      float newY = y + (size * cos(t) * sin(t)) / den;

      u8g2.drawLine((int)oldX, (int)oldY, (int)newX, (int)newY);

      oldX = newX;
      oldY = newY;
    }
  }
};

void drawCurve(U8G2 &u8g2, int x0, int y0, int x1, int y1, int x2, int y2) {
  // We divide the curve into 10 small straight lines
  float step = 0.1;
  float oldX = x0;
  float oldY = y0;

  for (float t = 0; t <= 1.0; t += step) {
    // Quadratic Bezier Formula
    float a = (1 - t) * (1 - t);
    float b = 2 * (1 - t) * t;
    float c = t * t;

    int newX = a * x0 + b * x1 + c * x2;
    int newY = a * y0 + b * y1 + c * y2;

    u8g2.drawLine(oldX, oldY, newX, newY);
    oldX = newX;
    oldY = newY;
  }
}

#endif