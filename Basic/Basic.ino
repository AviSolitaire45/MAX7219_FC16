#include <SPI.h>
#include "MAX7219_FC16.h"

// Create display object: CS pin 10, 4 modules
MAX7219_FC16 display(10, 4);

void setup() {
    display.begin();  // Initialize with default brightness
    
    // Show module numbers as a test
    display.showModuleNumbers(3000);
}

void loop() {
    // Scroll text with normal font
    display.scrollText("HELLO WORLD JOKE    ", 50, FONT_NORMAL);
    display.scrollText("FC16 WORKING    ", 50, FONT_NORMAL);
    
    // Scroll text with tiny font
    display.scrollText("tiny font test    ", 50, FONT_TINY);
    
    // Display some invader sprites
    display.clear();
    
    // Use getNumModules() instead of accessing private _numModules
    for (int i = 0; i < 4 && i < display.getNumModules(); i++) {
        display.drawInvader(i * 8, i % 12);
    }
    
    display.update();
    delay(2000);
}
