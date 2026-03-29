#include <SPI.h>
#include "MAX7219_FC16.h"

// Pin definitions
#define CS_PIN 10        // Chip select pin for MAX7219
#define NUM_MODULES 4    // Number of FC16 modules in your display chain

// Create display object
MAX7219_FC16 display(CS_PIN, NUM_MODULES);

void setup() {
  Serial.begin(115200);
  Serial.println("MAX7219 FC16 Display Demo Starting...");
  
  // Initialize display with medium brightness (0-15)
  display.begin(8);
  
  // Clear any existing content
  display.clear();
  display.update();
  
  delay(500);
  
  // Optional: Test modules by showing module numbers
  Serial.println("Showing module numbers...");
  display.showModuleNumbers(2000);
  
  // Clear the display
  display.clear();
  display.update();
  delay(500);
  
  // DEMO 1: Static text with normal font
  Serial.println("Demo 1: Static text with normal font");
  display.clear();
  
  // Draw text starting at column 2 (centered with 4 modules = 32 columns)
  display.drawText(2, "HELLO", FONT_NORMAL);
  display.update();
  delay(3000);
  
  // Clear and show different text
  display.clear();
  display.drawText(0, "ARDUINO", FONT_NORMAL);
  display.update();
  delay(3000);
  
  // DEMO 2: Scrolling text (non-blocking)
  Serial.println("Demo 2: Scrolling text - will run for 10 seconds");
  
  // Start scrolling text from right to left
  // Parameters: text, speed delay (ms), font size, loop (true = infinite)
  display.startScrollText("WELCOME TO MAX7219 FC16 DISPLAY! ", 50, FONT_NORMAL, true);
  
  // Let it scroll for 10 seconds
  unsigned long startTime = millis();
  while (millis() - startTime < 10000) {
    display.updateScroll();  // Must call this in loop for animation
    delay(10);               // Small delay for stability
  }
  
  // Stop scrolling
  display.stopScroll();
  display.clear();
  display.update();
  delay(500);
  
  // DEMO 3: Multiple lines of static text
  Serial.println("Demo 3: Multiple lines with different content");
  display.clear();
  
  // Draw different text at different positions
  display.drawText(0, "SCORE:", FONT_NORMAL);
  display.drawNumber(36, 1234, FONT_NORMAL);
  display.update();
  delay(3000);
  
  // DEMO 4: Mixed content
  display.clear();
  display.drawText(0, "TIME:", FONT_NORMAL);
  display.drawNumber(30, 42, FONT_NORMAL);
  display.update();
  delay(3000);
  
  // DEMO 5: Alternative scrolling method (blocking - kept for compatibility)
  Serial.println("Demo 5: Blocking scroll example");
  display.clear();
  display.update();
  delay(500);
  
  // This blocks until scrolling completes
  display.scrollText("BLOCKING SCROLL - ONCE! ", 60, FONT_NORMAL);
  delay(1000);
  
  Serial.println("All demos completed!");
  Serial.println("Starting final infinite scroll demo...");
}

void loop() {
  // Example of continuous scrolling with the non-blocking method
  // This will run forever, scrolling the message
  static bool scrollStarted = false;
  
  if (!scrollStarted) {
    // Start scrolling a longer message
    display.startScrollText(">>> SCROLLING MESSAGE! PRESS RESET TO RESTART <<< ", 45, FONT_NORMAL, true);
    scrollStarted = true;
  }
  
  // Important: Must call updateScroll() in loop for animation
  display.updateScroll();
  
  // You can do other tasks here while scrolling continues
  // For example, blink an LED, read sensors, etc.
  delay(10);
}