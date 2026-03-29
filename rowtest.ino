#include <SPI.h>
#include "MAX7219_FC16.h"

#define CS_PIN 10
#define NUM_MODULES 4  // 4 modules = 32 columns total

MAX7219_FC16 display(CS_PIN, NUM_MODULES);

void setup() {
  Serial.begin(115200);
  display.begin(8);
  display.clear();
  display.update();
  
  delay(1000);
  
  Serial.println("=== SINGLE HORIZONTAL LINE TEST ===");
  Serial.println("Moving a continuous line across ALL 4 modules");
  Serial.println("Total columns: 32");
  Serial.println();
}

void loop() {
  // Move line from TOP (row 7) to BOTTOM (row 0)
  for (int row = 7; row >= 0; row--) {
    display.clear();
    
    // Draw a single continuous horizontal line at current row
    // This will span across all 4 modules automatically
    for (int col = 0; col < display.getTotalColumns(); col++) {
      display.setPixel(col, row, true);
    }
    
    display.update();
    
    Serial.print("Line at row ");
    Serial.print(row);
    if (row == 7) Serial.println(" (TOP)");
    else if (row == 0) Serial.println(" (BOTTOM)");
    else Serial.println();
    
    delay(500);
  }
  
  Serial.println();
  Serial.println("Line reached BOTTOM! Restarting from TOP...");
  Serial.println();
  
  delay(2000);
}