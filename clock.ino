#include "MAX7219_FC16.h"

// Pin definitions
#define CS_PIN 10      // Chip select pin for MAX7219
#define NUM_MODULES 4  // Number of FC-16 modules (32 columns total)

// Create display object
MAX7219_FC16 display(CS_PIN, NUM_MODULES);

// Variables for time keeping
int hour = 12;
int minute = 0;
int second = 0;
unsigned long lastSecondUpdate = 0;
unsigned long lastBlinkUpdate = 0;
bool dotsVisible = true;
bool timeChanged = true;  // Force initial display update

void setup() {
    // Initialize display
    display.begin(2);  // Brightness 0-15 (8 is good)
    display.clear();
    display.update();
    
    Serial.begin(9600);
    Serial.println("Digital Clock Started");
    Serial.println("HH:MM with blinking dots");
    
    // Force initial display
    displayTime();
}

void updateTime() {
    // Increment time every second
    second++;
    if (second >= 60) {
        second = 0;
        minute++;
        if (minute >= 60) {
            minute = 0;
            hour++;
            if (hour > 12) {
                hour = 1;
            }
        }
    }
    timeChanged = true;  // Mark that time has changed
}

void displayTime() {
    // Clear the entire display first
    display.clear();
    
    // Convert hour to two-digit format (01-12)
    int hourDisplay = hour;
    int hourTens = hourDisplay / 10;
    int hourUnits = hourDisplay % 10;
    
    // Convert minute to two-digit format
    int minuteTens = minute / 10;
    int minuteUnits = minute % 10;
    
    // Calculate total width of the time display
    // Format: H1 + space + H2 + space + colon(2) + space + M1 + space + M2
    // Width: 5 + 1 + 5 + 1 + 2 + 1 + 5 + 1 + 5 = 26 columns
    
    int totalWidth = 26;
    int totalColumns = 32;
    int leftMargin = (totalColumns - totalWidth) / 2;  // (32 - 26) / 2 = 3
    
    // Balanced layout with 3 columns margin on each side:
    // Left margin: 3 columns
    // [H1: 5 cols][space:1][H2: 5 cols][space:1][:: 2 cols][space:1][M1: 5 cols][space:1][M2: 5 cols]
    // Right margin: 3 columns
    
    // Column positions:
    // Left margin: 0-2
    // H1: 3-7
    // Space: 8
    // H2: 9-13
    // Space: 14
    // Colon: 15-16
    // Space: 17
    // M1: 18-22
    // Space: 23
    // M2: 24-28
    // Right margin: 29-31
    
    int pos = leftMargin;  // Start at column 3
    
    // Hour tens digit
    if (hourTens > 0) {
        drawDigit(pos, hourTens);
    } else {
        drawBlank(pos);  // Blank space for leading zero
    }
    pos += 6;  // 5 columns digit + 1 column space
    
    // Hour units digit
    drawDigit(pos, hourUnits);
    pos += 6;  // 5 columns digit + 1 column space
    
    // Colon (2 columns)
    if (dotsVisible) {
        display.setPixel(pos, 2, true);
        display.setPixel(pos, 5, true);
        display.setPixel(pos + 1, 2, true);
        display.setPixel(pos + 1, 5, true);
    }
    pos += 3;  // 2 columns colon + 1 column space
    
    // Minute tens digit
    drawDigit(pos, minuteTens);
    pos += 6;  // 5 columns digit + 1 column space
    
    // Minute units digit
    drawDigit(pos, minuteUnits);
    
    // Update the physical display
    display.update();
    
    Serial.print("Display updated - Left margin: ");
    Serial.print(leftMargin);
    Serial.println(" columns");
}

// Helper function to draw a digit at specific column
void drawDigit(int startCol, int digit) {
    uint8_t charData[5];
    getNormalFontChar('0' + digit, charData);
    
    for (int i = 0; i < 5; i++) {
        if (startCol + i < display.getTotalColumns()) {
            display.setColumn(startCol + i, charData[i]);
        }
    }
}

// Helper function to draw blank space
void drawBlank(int startCol) {
    for (int i = 0; i < 5; i++) {
        if (startCol + i < display.getTotalColumns()) {
            display.setColumn(startCol + i, 0);
        }
    }
}

void printSerialTime() {
    Serial.print("Time: ");
    Serial.print(hour);
    Serial.print(":");
    if (minute < 10) Serial.print("0");
    Serial.print(minute);
    Serial.print(":");
    if (second < 10) Serial.print("0");
    Serial.println(second);
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Update time every second (1000 ms)
    if (currentMillis - lastSecondUpdate >= 1000) {
        lastSecondUpdate = currentMillis;
        updateTime();
        printSerialTime();
        displayTime();  // Update display when time changes
    }
    
    // Update dots blinking every 500 ms
    if (currentMillis - lastBlinkUpdate >= 500) {
        lastBlinkUpdate = currentMillis;
        dotsVisible = !dotsVisible;
        displayTime();  // Refresh display with new dot state
    }
    
    // Small delay to prevent overwhelming the processor
    delay(10);
}