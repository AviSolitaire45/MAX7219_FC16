#include <SPI.h>
#include "MAX7219_FC16.h"

// Pin definitions
#define CS_PIN 10
#define NUM_MODULES 4

// Create display object
MAX7219_FC16 display(CS_PIN, NUM_MODULES);

// Rain drop structure
struct RainDrop {
    int column;
    int row;
    int speed;
    bool active;
};

// Rain effect parameters
#define MAX_DROPS 15
RainDrop drops[MAX_DROPS];
int rainSpeed = 70;

// Initialize a new rain drop at TOP (which is row 7 for inverted display)
void initRainDrop(int index) {
    drops[index].column = random(0, display.getTotalColumns());
    drops[index].row = 7;  // Start at TOP (row 7 because display is inverted)
    drops[index].speed = random(1, 3);  // Speed 1-2
    drops[index].active = true;
}

// Rain effect for INVERTED display - falls DOWNWARDS
void updateRain() {
    display.beginFrame();
    display.clearFrame();
    
    for (int i = 0; i < MAX_DROPS; i++) {
        if (drops[i].active) {
            // Draw drop at current position
            display.setPixelFrame(drops[i].column, drops[i].row, true);
            
            // Create trail effect above the drop (towards row 7)
            if (drops[i].row < 7) {
                display.setPixelFrame(drops[i].column, drops[i].row + 1, true);
            }
            if (drops[i].row < 6) {
                display.setPixelFrame(drops[i].column, drops[i].row + 2, true);
            }
            
            // Move drop DOWN (decrease row number because row 0 is bottom)
            drops[i].row -= drops[i].speed;
            
            // Check if drop reached bottom (row 0)
            if (drops[i].row < 0) {
                // Splash effect at bottom (row 0)
                display.setPixelFrame(drops[i].column, 0, true);
                if (drops[i].column > 0) {
                    display.setPixelFrame(drops[i].column - 1, 0, true);
                }
                if (drops[i].column < display.getTotalColumns() - 1) {
                    display.setPixelFrame(drops[i].column + 1, 0, true);
                }
                
                // Reinitialize drop at top (row 7)
                initRainDrop(i);
            }
        }
    }
    
    // Add random new drops
    if (random(0, 100) < 10) {
        for (int i = 0; i < MAX_DROPS; i++) {
            if (!drops[i].active) {
                initRainDrop(i);
                break;
            }
        }
    }
    
    display.commitFrame();
}

// Heavy rain for inverted display
void updateHeavyRain() {
    display.beginFrame();
    display.clearFrame();
    
    for (int i = 0; i < MAX_DROPS; i++) {
        if (drops[i].active) {
            // Draw main drop
            display.setPixelFrame(drops[i].column, drops[i].row, true);
            
            // Longer trail above
            int trailLength = drops[i].speed + 1;
            for (int t = 1; t <= trailLength; t++) {
                int trailRow = drops[i].row + t;  // Trail ABOVE (towards row 7)
                if (trailRow <= 7 && trailRow >= 0) {
                    if (random(0, 100) > 30) {
                        display.setPixelFrame(drops[i].column, trailRow, true);
                    }
                }
            }
            
            // Move down (decrease row)
            drops[i].row -= drops[i].speed;
            
            // Reset at bottom (row 0)
            if (drops[i].row < 0) {
                // Splash effect
                for (int splash = -1; splash <= 1; splash++) {
                    int splashCol = drops[i].column + splash;
                    if (splashCol >= 0 && splashCol < display.getTotalColumns()) {
                        display.setPixelFrame(splashCol, 0, true);
                    }
                }
                initRainDrop(i);
            }
        }
    }
    
    // Occasional lightning
    if (random(0, 1000) < 3) {
        uint8_t oldBrightness = 8;
        display.setIntensity(15);
        display.commitFrame();
        delay(30);
        display.setIntensity(oldBrightness);
    }
    
    display.commitFrame();
}

// Rain with ripple effect for inverted display
void updateRainWithRipple() {
    static int rippleColumn = -1;
    static int rippleFrame = 0;
    
    display.beginFrame();
    display.clearFrame();
    
    for (int i = 0; i < MAX_DROPS; i++) {
        if (drops[i].active) {
            display.setPixelFrame(drops[i].column, drops[i].row, true);
            
            // Move down (decrease row)
            drops[i].row -= drops[i].speed;
            
            // Hit bottom (row 0)
            if (drops[i].row < 0) {
                rippleColumn = drops[i].column;
                rippleFrame = 0;
                initRainDrop(i);
            }
        }
    }
    
    // Draw ripple at bottom (row 0)
    if (rippleColumn >= 0 && rippleFrame < 6) {
        int radius = rippleFrame / 2;
        for (int offset = -radius; offset <= radius; offset++) {
            int col = rippleColumn + offset;
            if (col >= 0 && col < display.getTotalColumns()) {
                if (radius == 0) {
                    display.setPixelFrame(col, 0, true);
                } else if (radius == 1) {
                    if (random(0, 100) > 40) {
                        display.setPixelFrame(col, 0, true);
                    }
                } else if (radius == 2) {
                    if (random(0, 100) > 70) {
                        display.setPixelFrame(col, 0, true);
                    }
                }
            }
        }
        rippleFrame++;
        if (rippleFrame >= 6) {
            rippleColumn = -1;
        }
    }
    
    display.commitFrame();
}

void setup() {
    Serial.begin(115200);
    Serial.println("Rain Effect for INVERTED Display");
    Serial.println("Rows: 7 = TOP, 0 = BOTTOM");
    
    // Initialize display
    display.begin(8);
    display.clear();
    display.update();
    
    delay(1000);
    
    // Initialize drops
    for (int i = 0; i < MAX_DROPS; i++) {
        drops[i].active = false;
    }
    
    // Start with initial drops at top (row 7)
    for (int i = 0; i < 8; i++) {
        initRainDrop(i);
        delay(30);
    }
    
    Serial.println("Rain falling from TOP to BOTTOM!");
}

void loop() {
    static int effectMode = 0;
    static unsigned long lastModeSwitch = 0;
    
    // Change effect every 15 seconds
    if (millis() - lastModeSwitch > 15000) {
        lastModeSwitch = millis();
        effectMode = (effectMode + 1) % 3;
        
        // Reset drops
        for (int i = 0; i < MAX_DROPS; i++) {
            drops[i].active = false;
        }
        for (int i = 0; i < 8; i++) {
            initRainDrop(i);
        }
    }
    
    // Update based on mode
    switch (effectMode) {
        case 0:
            updateRain();
            break;
        case 1:
            updateHeavyRain();
            break;
        case 2:
            updateRainWithRipple();
            break;
    }
    
    delay(rainSpeed);
}