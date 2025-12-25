#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <U8g2lib.h>

enum TextAlign { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT };

class TextBox {
private:
  int x, y;
  int targetHeight;
  int width;
  TextAlign align;
  String text;

  // Internal pointer to the specific font data
  const uint8_t *fontData;

  // Helper: Pick a standard font based on requested pixel height
  void selectFont() {
    if (targetHeight < 10) {
      fontData = u8g2_font_tom_thumb_4x6_t_all;
    } else if (targetHeight < 15) {
      fontData = u8g2_font_profont12_tf;
    } else if (targetHeight < 22) {
      fontData = u8g2_font_profont17_tf;
    } else {
      fontData = u8g2_font_profont29_tf;
    }
  }

public:
  // Constructor
  // Width defaults to 0 (Auto), Align defaults to LEFT
  TextBox(int _x, int _y, int _height, int _width = 0,
          TextAlign _align = ALIGN_LEFT)
      : x(_x), y(_y), targetHeight(_height), width(_width), align(_align) {

    text = "";
    selectFont(); // Pick the best font immediately
  }

  void setText(String t) { text = t; }

  // You can also change properties on the fly if needed
  void setAlignment(TextAlign a) { align = a; }

  void draw(U8G2 &u8g2) {
    if (text.length() == 0)
      return;

    u8g2.setFont(fontData);
    u8g2.setDrawColor(1);
    u8g2.setFontMode(1); // Transparent

    int lineHeight = targetHeight; // Approximate line height
    int currentY = y + lineHeight; // Baseline of first line

    // If no width constraint, draw single line
    if (width <= 0) {
      u8g2.drawStr(x, currentY, text.c_str());
      return;
    }

    // Word Wrapping Logic
    String currentLine = "";
    String remaining = text;

    while (remaining.length() > 0) {
      int spaceIndex = remaining.indexOf(' ');
      String word;
      if (spaceIndex == -1) {
        word = remaining;
        remaining = "";
      } else {
        word = remaining.substring(0, spaceIndex);
        remaining = remaining.substring(spaceIndex + 1);
      }

      String testLine =
          currentLine.length() > 0 ? currentLine + " " + word : word;
      if (u8g2.getStrWidth(testLine.c_str()) <= width) {
        currentLine = testLine;
      } else {
        // Draw the full line we had before adding this overflow word
        int drawX = x;
        int strWidth = u8g2.getStrWidth(currentLine.c_str());

        if (align == ALIGN_CENTER)
          drawX += (width - strWidth) / 2;
        else if (align == ALIGN_RIGHT)
          drawX += (width - strWidth);

        u8g2.drawStr(drawX, currentY, currentLine.c_str());

        // Move down
        currentY += lineHeight;
        currentLine = word; // Start new line with the word that didn't fit
      }
    }
    // Draw the final remainder line
    if (currentLine.length() > 0) {
      int drawX = x;
      int strWidth = u8g2.getStrWidth(currentLine.c_str());
      if (align == ALIGN_CENTER)
        drawX += (width - strWidth) / 2;
      else if (align == ALIGN_RIGHT)
        drawX += (width - strWidth);
      u8g2.drawStr(drawX, currentY, currentLine.c_str());
    }
  }
};

#endif