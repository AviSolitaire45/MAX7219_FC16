#ifndef MAX7219_FC16_h
#define MAX7219_FC16_h

#include <Arduino.h>
#include <SPI.h>
#include "Fonts.h"

class MAX7219_FC16 {
public:
    // Constructor
    MAX7219_FC16(int csPin, int numModules);
    
    // Initialization
    void begin(uint8_t intensity = 0x08);
    
    // Display control
    void clear();
    void update();
    
    // Individual pixel/column control
    void setPixel(int col, int row, bool state);
    void setColumn(int col, uint8_t value);
    void setRow(int row, uint8_t value);
    
    // Text rendering with different font sizes
    void drawChar(int pos, char c, FontSize size = FONT_NORMAL);
    void drawNumber(int pos, int num, FontSize size = FONT_NORMAL);
    void drawText(int pos, const char* text, FontSize size = FONT_NORMAL);
    
    // Animation
    void scrollText(const char* text, int speedDelay, FontSize size = FONT_NORMAL);
    
    // Module test
    void showModuleNumbers(int delayMs = 3000);
    
    // Brightness control (0x00 to 0x0F)
    void setIntensity(uint8_t intensity);
    
    // Shutdown/Startup
    void shutdown();
    void wakeup();
    
    // Special character drawing (invader sprites)
    void drawInvader(int pos, int invaderType);
    
    // Get number of modules (public accessor)
    int getNumModules() const { return _numModules; }
    
    // Get total columns (public accessor)
    int getTotalColumns() const { return _totalColumns; }
    
private:
    int _csPin;
    int _numModules;
    int _totalColumns;
    uint8_t** _displayBuffer;
    
    // Low-level communication
    void sendToAll(uint8_t cmd, uint8_t data);
    
    // Font helpers
    int getFontIndex(char c);
    int getNumberFontIndex(int num);
    
    // Register definitions
    static const uint8_t REG_NOOP        = 0x00;
    static const uint8_t REG_DIGIT0      = 0x01;
    static const uint8_t REG_DECODEMODE  = 0x09;
    static const uint8_t REG_INTENSITY   = 0x0A;
    static const uint8_t REG_SCANLIMIT   = 0x0B;
    static const uint8_t REG_SHUTDOWN    = 0x0C;
    static const uint8_t REG_DISPLAYTEST = 0x0F;
};

#endif