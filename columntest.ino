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
  
  Serial.println("=== SINGLE VERTICAL LINE TEST ===");
  Serial.println("Moving a continuous vertical line from LEFT to RIGHT");
  Serial.println("Total columns: 32");
  Serial.println();
}

void loop() {
  // Move line from LEFT to RIGHT across all modules
  for (int col = 0; col < display.getTotalColumns(); col++) {
    display.clear();
    
    // Draw a single continuous vertical line at current column
    for (int row = 0; row < 8; row++) {
      display.setPixel(col, row, true);
    }
    
    display.update();
    
    Serial.print("Line at column ");
    Serial.print(col);
    if (col == 0) Serial.println(" (LEFT EDGE)");
    else if (col == display.getTotalColumns() - 1) Serial.println(" (RIGHT EDGE)");
    else Serial.println();
    
    delay(100);
  }
  
  Serial.println();
  Serial.println("Line reached RIGHT edge! Restarting from LEFT...");
  Serial.println();
  
  delay(2000);
}