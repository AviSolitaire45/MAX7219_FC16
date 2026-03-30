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

// Date variables (using dummy date)
int day = 15;
int month = 3;
int year = 2026;
int dayOfWeek = 6;  // 0=Sunday, 1=Monday, 2=Tuesday, 3=Wednesday, 4=Thursday, 5=Friday, 6=Saturday

// Scroll display state
enum DisplayMode {
    MODE_CLOCK,
    MODE_DATE_SCROLL
};

DisplayMode currentMode = MODE_CLOCK;
unsigned long modeSwitchTime = 0;
const unsigned long CLOCK_DISPLAY_DURATION = 12000;  // Show clock for 12 seconds
const unsigned long DATE_SCROLL_DURATION = 3000;      // Scroll date for 3 seconds

// Scroll text
char dateText[40];  // Increased size to accommodate day of week
bool scrollStarted = false;

void setup() {
    // Initialize display
    display.begin(8);  // Brightness 0-15 (8 is good)
    display.clear();
    display.update();
    
    Serial.begin(9600);
    Serial.println("Digital Clock Started with Date Scroll (including day of week)");
    Serial.println("Clock displays for 12 seconds, then scrolls date for 3 seconds");
    
    // Build initial date text
    buildDateText();
    
    // Force initial display
    displayTime();
    
    // Initialize mode timing
    modeSwitchTime = millis();
}

void buildDateText() {
    const char* months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", 
                             "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    const char* days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    
    // Format: "SUN 15 MAR 2026"
    sprintf(dateText, "%s %02d %s %04d", days[dayOfWeek], day, months[month - 1], year);
    
    Serial.print("Date text built: ");
    Serial.println(dateText);
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
            // For demo purposes, update day of week when hour changes
            // In a real application, you'd update this based on actual date
            dayOfWeek = (dayOfWeek + 1) % 7;
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
    int totalWidth = 26;
    int totalColumns = 32;
    int leftMargin = (totalColumns - totalWidth) / 2;  // (32 - 26) / 2 = 3
    
    // Balanced layout with 3 columns margin on each side
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
    
    Serial.print("Clock displayed - ");
    printSerialTime();
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

void printDate() {
    const char* days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    Serial.print("Date: ");
    Serial.print(days[dayOfWeek]);
    Serial.print(" ");
    Serial.print(day);
    Serial.print("/");
    Serial.print(month);
    Serial.print("/");
    Serial.println(year);
}

void startDateScroll() {
    // Rebuild date text (in case date changes)
    buildDateText();
    
    // Stop any existing scroll
    if (display.isScrolling()) {
        display.stopScroll();
    }
    
    // Clear display before starting scroll
    display.clear();
    display.update();
    
    // Start scrolling with speed delay 60ms, normal font, loop once
    // The scroll will automatically update using display.updateScroll()
    display.startScrollText(dateText, 60, FONT_NORMAL, false);
    scrollStarted = true;
    
    Serial.print("Date scroll started: ");
    Serial.println(dateText);
}

void stopDateScroll() {
    if (display.isScrolling()) {
        display.stopScroll();
    }
    scrollStarted = false;
    display.clear();
    display.update();
    Serial.println("Date scroll stopped");
}

void updateDisplayMode() {
    unsigned long currentTime = millis();
    
    switch(currentMode) {
        case MODE_CLOCK:
            // Check if it's time to switch to date scroll
            if (currentTime - modeSwitchTime >= CLOCK_DISPLAY_DURATION) {
                Serial.println("Switching to date scroll mode");
                currentMode = MODE_DATE_SCROLL;
                modeSwitchTime = currentTime;
                startDateScroll();
            }
            break;
            
        case MODE_DATE_SCROLL:
            // CRITICAL: Update scroll animation continuously
            // This must be called frequently to animate the scroll
            if (scrollStarted) {
                display.updateScroll();
                
                // Check if scrolling is complete
                if (!display.isScrolling()) {
                    // Scroll finished, switch back to clock mode
                    Serial.println("Scroll complete, switching back to clock mode");
                    currentMode = MODE_CLOCK;
                    modeSwitchTime = currentTime;
                    scrollStarted = false;
                    
                    // Force immediate clock display
                    dotsVisible = true;  // Reset dots to visible
                    display.clear();
                    displayTime();
                }
            } else {
                // Fallback - if scroll wasn't started properly
                currentMode = MODE_CLOCK;
                modeSwitchTime = currentTime;
            }
            break;
    }
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Update time every second (1000 ms)
    if (currentMillis - lastSecondUpdate >= 1000) {
        lastSecondUpdate = currentMillis;
        updateTime();
        printSerialTime();
        printDate();  // Also print date for debugging
        
        // Only update clock display if we're in clock mode
        if (currentMode == MODE_CLOCK) {
            displayTime();  // Update display when time changes
        }
    }
    
    // Update dots blinking every 500 ms (only in clock mode)
    if (currentMillis - lastBlinkUpdate >= 500) {
        lastBlinkUpdate = currentMillis;
        if (currentMode == MODE_CLOCK) {
            dotsVisible = !dotsVisible;
            displayTime();  // Refresh display with new dot state
        }
    }
    
    // Handle display mode switching and scroll updates
    updateDisplayMode();
    
    // Small delay to prevent overwhelming the processor
    delay(5);  // Reduced delay for smoother scrolling
}