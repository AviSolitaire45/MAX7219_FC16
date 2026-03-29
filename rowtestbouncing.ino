#include <SPI.h>
#include "MAX7219_FC16.h"

#define CS_PIN 10
#define NUM_MODULES 4

MAX7219_FC16 display(CS_PIN, NUM_MODULES);

void setup() {
  display.begin(8);
  display.clear();
  display.update();
  delay(1000);
}

void loop() {
  // Bouncing line effect
  static int direction = -1;  // -1 = down, 1 = up
  static int currentRow = 7;  // Start at top
  
  display.clear();
  
  // Draw line at current row across all modules
  display.setRow(currentRow, 0xFF);
  display.update();
  
  // Update position
  currentRow += direction;
  
  // Reverse direction at boundaries
  if (currentRow < 0) {
    currentRow = 1;
    direction = 1;
  } else if (currentRow > 7) {
    currentRow = 6;
    direction = -1;
  }
  
  delay(80);
}