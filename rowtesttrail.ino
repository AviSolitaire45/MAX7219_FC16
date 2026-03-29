
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
  
  Serial.println("=== SINGLE HORIZONTAL LINE WITH TRAIL ===");
  Serial.println("Line moves from TOP to BOTTOM with fading trail");
  Serial.println();
}

void loop() {
  // Multiple passes with trail effect
  for (int pass = 0; pass < 3; pass++) {
    
    for (int row = 7; row >= 0; row--) {
      display.beginFrame();
      display.clearFrame();
      
      // Draw main line at current row (full brightness)
      for (int col = 0; col < display.getTotalColumns(); col++) {
        display.setPixelFrame(col, row, true);
      }
      
      // Draw trail lines above (dimmer effect using dithering)
      if (row + 1 <= 7) {
        for (int col = 0; col < display.getTotalColumns(); col++) {
          display.setPixelFrame(col, row + 1, true);
        }
      }
      
      if (row + 2 <= 7) {
        // Trail with 50% intensity (dithering pattern)
        for (int col = 0; col < display.getTotalColumns(); col++) {
          if (col % 2 == 0) {  // Only every other pixel
            display.setPixelFrame(col, row + 2, true);
          }
        }
      }
      
      display.commitFrame();
      
      Serial.print("Main line at row ");
      Serial.println(row);
      
      delay(150);
    }
    
    delay(1000);
  }
  
  // Flash effect at bottom to show completion
  for (int flash = 0; flash < 3; flash++) {
    display.clear();
    // Draw bottom line (row 0)
    for (int col = 0; col < display.getTotalColumns(); col++) {
      display.setPixel(col, 0, true);
    }
    display.update();
    delay(100);
    display.clear();
    display.update();
    delay(100);
  }
  
  delay(2000);
}