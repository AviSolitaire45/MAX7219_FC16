#include <SPI.h>
#include "MAX7219_FC16.h"

#define CS_PIN 10
#define NUM_MODULES 4

MAX7219_FC16 display(CS_PIN, NUM_MODULES);

void setup() {
  Serial.begin(115200);
  display.begin(8);
  display.clear();
  display.update();
  
  delay(1000);
  
  Serial.println("=== ACCUMULATING COLUMNS TEST ===");
  Serial.println("Columns accumulate from LEFT to RIGHT");
  Serial.println();
}

void loop() {
  // Clear at start
  display.clear();
  display.update();
  delay(500);
  
  Serial.println("Accumulating columns from LEFT to RIGHT...");
  
  // Accumulate columns from left to right
  for (int col = 0; col < display.getTotalColumns(); col++) {
    // Draw vertical line at current column
    for (int row = 0; row < 8; row++) {
      display.setPixel(col, row, true);
    }
    display.update();
    
    Serial.print("Added column ");
    Serial.println(col);
    delay(150);
  }
  
  Serial.println("All columns now lit!");
  delay(2000);
  
  // Clear and repeat
  display.clear();
  display.update();
  delay(1000);
}