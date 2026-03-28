#include <SPI.h>
#include "MAX7219_FC16.h"

// Create display object: CS pin 10, 4 modules
MAX7219_FC16 display(10, 4);

// Time variables
int hours = 12;
int minutes = 0;
int seconds = 0;
unsigned long lastTick = 0;
bool colonBlink = true;
unsigned long lastColonBlink = 0;

// Seconds dots variables
bool secondsDots[2] = {true, true};  // Two dots for seconds
unsigned long lastSecondsDotBlink = 0;
int secondsDotPattern = 0;  // 0=both on, 1=left on, 2=right on, 3=both off

// Clock mode
enum ClockMode {
    MODE_NORMAL,      // Normal font (5x8) with double seconds dots
    MODE_TINY,        // Tiny font (3x8)
    MODE_BIG          // Big numbers (requires 2 modules per digit)
};

ClockMode currentMode = MODE_NORMAL;
unsigned long lastModeChange = 0;

void setup() {
    Serial.begin(9600);
    Serial.println("MAX7219_FC16 Digital Clock with Double Seconds Dots");
    Serial.println("===================================================");
    
    // Initialize display
    display.begin();
    display.setIntensity(0x08);  // Medium brightness
    
    // Set initial time
    hours = 12;
    minutes = 0;
    seconds = 0;
    lastTick = millis();
    lastColonBlink = millis();
    lastSecondsDotBlink = millis();
    lastModeChange = millis();
    
    // Show startup animation
    startupAnimation();
}

void loop() {
    updateTime();           // Update time using millis()
    updateColonBlink();     // Blink colon every second
    updateSecondsDots();    // Update seconds dots animation
    updateClockMode();      // Auto-switch modes every 10 seconds
    displayTime();          // Display current time
    
    delay(50);  // Small delay to prevent flickering
}

// Update time using millis() (software timer)
void updateTime() {
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastTick >= 1000) {
        lastTick = currentMillis;
        seconds++;
        
        if (seconds >= 60) {
            seconds = 0;
            minutes++;
            
            if (minutes >= 60) {
                minutes = 0;
                hours++;
                
                if (hours >= 24) {
                    hours = 0;
                }
            }
        }
        
        // Print to serial for debugging
        Serial.print("Time: ");
        printTime();
        Serial.println();
    }
}

// Update colon blink state
void updateColonBlink() {
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastColonBlink >= 500) {
        lastColonBlink = currentMillis;
        colonBlink = !colonBlink;
    }
}

// Update seconds dots animation (2 dots)
void updateSecondsDots() {
    unsigned long currentMillis = millis();
    
    // Update every 250ms for smooth animation
    if (currentMillis - lastSecondsDotBlink >= 250) {
        lastSecondsDotBlink = currentMillis;
        
        // Cycle through patterns: both on -> left on -> right on -> both off
        secondsDotPattern = (secondsDotPattern + 1) % 4;
        
        switch (secondsDotPattern) {
            case 0:
                secondsDots[0] = true;
                secondsDots[1] = true;
                break;
            case 1:
                secondsDots[0] = true;
                secondsDots[1] = false;
                break;
            case 2:
                secondsDots[0] = false;
                secondsDots[1] = true;
                break;
            case 3:
                secondsDots[0] = false;
                secondsDots[1] = false;
                break;
        }
    }
}

// Update clock mode every 10 seconds
void updateClockMode() {
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastModeChange >= 10000) {
        lastModeChange = currentMillis;
        
        // Cycle through modes
        switch (currentMode) {
            case MODE_NORMAL:
                currentMode = MODE_TINY;
                Serial.println("Switched to TINY mode");
                break;
            case MODE_TINY:
                currentMode = MODE_BIG;
                Serial.println("Switched to BIG mode");
                break;
            case MODE_BIG:
                currentMode = MODE_NORMAL;
                Serial.println("Switched to NORMAL mode with seconds dots");
                break;
        }
        
        // Clear display when changing modes
        display.clear();
    }
}

// Display time based on current mode
void displayTime() {
    display.clear();
    
    switch (currentMode) {
        case MODE_NORMAL:
            displayTimeNormal();
            break;
        case MODE_TINY:
            displayTimeTiny();
            break;
        case MODE_BIG:
            displayTimeBig();
            break;
    }
    
    display.update();
}

// Normal font display (5x8 characters) with double seconds dots
void displayTimeNormal() {
    int totalWidth = display.getTotalColumns();
    int startX = (totalWidth - 36) / 2;  // Center the time (5 chars * 6 columns = 30 + 6 for seconds dots)
    
    // Display hours
    if (hours < 10) {
        drawCharWithSpace(startX, '0', FONT_NORMAL);
        drawCharWithSpace(startX + 6, '0' + hours, FONT_NORMAL);
    } else {
        drawCharWithSpace(startX, '0' + (hours / 10), FONT_NORMAL);
        drawCharWithSpace(startX + 6, '0' + (hours % 10), FONT_NORMAL);
    }
    
    // Display colon (HH:MM colon)
    if (colonBlink) {
        drawColon(startX + 12, FONT_NORMAL);
    }
    
    // Display minutes
    drawCharWithSpace(startX + 14, '0' + (minutes / 10), FONT_NORMAL);
    drawCharWithSpace(startX + 20, '0' + (minutes % 10), FONT_NORMAL);
    
    // Display double seconds dots
    drawSecondsDots(startX + 26, FONT_NORMAL);
}

// Draw double seconds dots
void drawSecondsDots(int pos, FontSize size) {
    if (size == FONT_TINY) {
        // Tiny font seconds dots
        if (secondsDots[0]) {
            display.setPixel(pos + 1, 3, true);
        }
        if (secondsDots[1]) {
            display.setPixel(pos + 1, 5, true);
        }
    } else {
        // Normal font seconds dots (two dots side by side)
        // First dot
        if (secondsDots[0]) {
            display.setPixel(pos + 2, 2, true);
            display.setPixel(pos + 2, 5, true);
        }
        
        // Second dot (3 pixels to the right)
        if (secondsDots[1]) {
            display.setPixel(pos + 4, 2, true);
            display.setPixel(pos + 4, 5, true);
        }
        
        // Optional: Add text "SS" or "SEC" below dots
        // Uncomment to show "SS" text below seconds dots
        /*
        if (seconds < 10) {
            display.drawChar(pos + 1, '0' + seconds, FONT_TINY);
        } else {
            display.drawChar(pos, '0' + (seconds / 10), FONT_TINY);
            display.drawChar(pos + 3, '0' + (seconds % 10), FONT_TINY);
        }
        */
    }
}

// Tiny font display (3x8 characters)
void displayTimeTiny() {
    int totalWidth = display.getTotalColumns();
    int startX = (totalWidth - 24) / 2;  // Center the time (5 chars * 4 columns = 20 + 4 for seconds)
    
    // Display hours
    if (hours < 10) {
        drawCharWithSpace(startX, '0', FONT_TINY);
        drawCharWithSpace(startX + 4, '0' + hours, FONT_TINY);
    } else {
        drawCharWithSpace(startX, '0' + (hours / 10), FONT_TINY);
        drawCharWithSpace(startX + 4, '0' + (hours % 10), FONT_TINY);
    }
    
    // Display colon
    if (colonBlink) {
        drawColon(startX + 8, FONT_TINY);
    }
    
    // Display minutes
    drawCharWithSpace(startX + 10, '0' + (minutes / 10), FONT_TINY);
    drawCharWithSpace(startX + 14, '0' + (minutes % 10), FONT_TINY);
    
    // Display seconds in tiny mode
    if (seconds < 10) {
        display.drawChar(startX + 18, '0' + seconds, FONT_TINY);
    } else {
        display.drawChar(startX + 18, '0' + (seconds / 10), FONT_TINY);
        display.drawChar(startX + 22, '0' + (seconds % 10), FONT_TINY);
    }
}

// Big font display (requires 2 modules per digit)
void displayTimeBig() {
    int totalColumns = display.getTotalColumns();
    
    // Hours
    if (hours < 10) {
        // Single digit hour - display blank for first digit
        drawBigNumber(0, 10);  // 10 = blank/space
        drawBigNumber(10, hours);
    } else {
        drawBigNumber(0, hours / 10);
        drawBigNumber(10, hours % 10);
    }
    
    // Colon (custom big colon)
    if (colonBlink) {
        drawBigColon(20);
    }
    
    // Minutes
    drawBigNumber(22, minutes / 10);
    drawBigNumber(32, minutes % 10);
}

// Helper function to draw character with spacing
void drawCharWithSpace(int pos, char c, FontSize size) {
    int charWidth = (size == FONT_TINY) ? 3 : 5;
    display.drawChar(pos, c, size);
    
    // Add space after character (except last)
    for (int i = 0; i < 1; i++) {
        display.setColumn(pos + charWidth + i, 0);
    }
}

// Draw colon for normal/tiny fonts
void drawColon(int pos, FontSize size) {
    if (size == FONT_TINY) {
        // Tiny colon (2 dots)
        display.setPixel(pos + 1, 3, true);
        display.setPixel(pos + 1, 5, true);
    } else {
        // Normal colon (2 dots)
        display.setPixel(pos + 2, 2, true);
        display.setPixel(pos + 2, 5, true);
    }
}

// Draw big number for big font mode
void drawBigNumber(int pos, int num) {
    if (num >= 0 && num <= 9) {
        display.drawNumber(pos, num, FONT_BIG);
    }
}

// Draw big colon for big font mode
void drawBigColon(int pos) {
    // Create a custom big colon by drawing dots
    for (int i = 0; i < 8; i++) {
        display.setPixel(pos + 2, i, false);  // Clear column
        display.setPixel(pos + 3, i, false);
    }
    
    // Draw two large dots
    for (int i = 0; i < 3; i++) {
        display.setPixel(pos + 2, 2 + i, true);   // Top dot
        display.setPixel(pos + 3, 2 + i, true);
        display.setPixel(pos + 2, 5 + i, true);   // Bottom dot
        display.setPixel(pos + 3, 5 + i, true);
    }
}

// Print time to serial monitor
void printTime() {
    if (hours < 10) Serial.print("0");
    Serial.print(hours);
    Serial.print(":");
    if (minutes < 10) Serial.print("0");
    Serial.print(minutes);
    Serial.print(":");
    if (seconds < 10) Serial.print("0");
    Serial.print(seconds);
}

// Startup animation
void startupAnimation() {
    display.clear();
    
    // Flash all LEDs
    for (int i = 0; i < 3; i++) {
        for (int col = 0; col < display.getTotalColumns(); col++) {
            display.setColumn(col, 0xFF);
        }
        display.update();
        delay(200);
        
        display.clear();
        display.update();
        delay(200);
    }
    
    // Show "CLOCK" text
    display.drawText(0, "CLOCK", FONT_NORMAL);
    display.update();
    delay(1500);
    
    // Show seconds dots animation demo
    display.clear();
    display.drawText(0, "SECONDS", FONT_NORMAL);
    display.update();
    delay(1500);
    
    display.clear();
    display.drawText(0, "DOTS", FONT_NORMAL);
    display.update();
    delay(1000);
    
    display.clear();
    display.update();
}

// Function to manually set time (call from serial)
void setTime(int h, int m, int s) {
    hours = h;
    minutes = m;
    seconds = s;
    Serial.print("Time set to: ");
    printTime();
    Serial.println();
}