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
  // Move line from TOP (row 7) to BOTTOM (row 0)
  for (int row = 7; row >= 0; row--) {
    // setRow automatically spans across all modules
    // 0xFF means all 8 columns in each module are lit
    display.setRow(row, 0xFF);
    display.update();
    delay(300);
  }
  
  delay(1000);
  
  // Move line from BOTTOM to TOP
  for (int row = 0; row <= 7; row++) {
    display.setRow(row, 0xFF);
    display.update();
    delay(300);
  }
  
  delay(1000);
}