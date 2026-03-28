#include "MAX7219_FC16.h"
#include <ctype.h>

// Constructor
MAX7219_FC16::MAX7219_FC16(int csPin, int numModules) {
    _csPin = csPin;
    _numModules = numModules;
    _totalColumns = numModules * 8;
    
    // Allocate display buffer
    _displayBuffer = new uint8_t*[_totalColumns];
    for (int i = 0; i < _totalColumns; i++) {
        _displayBuffer[i] = new uint8_t[8];
        for (int j = 0; j < 8; j++) {
            _displayBuffer[i][j] = 0;
        }
    }
}

// Initialize the display
void MAX7219_FC16::begin(uint8_t intensity) {
    pinMode(_csPin, OUTPUT);
    SPI.begin();
    
    sendToAll(REG_SHUTDOWN, 0x01);
    sendToAll(REG_DECODEMODE, 0x00);
    sendToAll(REG_SCANLIMIT, 0x07);
    sendToAll(REG_INTENSITY, intensity);
    sendToAll(REG_DISPLAYTEST, 0x00);
    
    clear();
    update();
}

// Send command to all modules
void MAX7219_FC16::sendToAll(uint8_t cmd, uint8_t data) {
    digitalWrite(_csPin, LOW);
    for (int i = 0; i < _numModules; i++) {
        SPI.transfer(cmd);
        SPI.transfer(data);
    }
    digitalWrite(_csPin, HIGH);
}

// Clear the display buffer
void MAX7219_FC16::clear() {
    for (int c = 0; c < _totalColumns; c++) {
        for (int r = 0; r < 8; r++) {
            _displayBuffer[c][r] = 0;
        }
    }
}

// Set a specific pixel
void MAX7219_FC16::setPixel(int col, int row, bool state) {
    if (col < 0 || col >= _totalColumns) return;
    if (row < 0 || row >= 8) return;
    _displayBuffer[col][row] = state ? 1 : 0;
}

// Set entire column
void MAX7219_FC16::setColumn(int col, uint8_t value) {
    if (col < 0 || col >= _totalColumns) return;
    
    for (int row = 0; row < 8; row++) {
        _displayBuffer[col][row] = (value >> (7 - row)) & 1;
    }
}

// Set entire row
void MAX7219_FC16::setRow(int row, uint8_t value) {
    if (row < 0 || row >= 8) return;
    
    for (int col = 0; col < _totalColumns; col++) {
        _displayBuffer[col][row] = (value >> (7 - (col % 8))) & 1;
    }
}

// Update physical display from buffer
void MAX7219_FC16::update() {
    for (int row = 0; row < 8; row++) {
        digitalWrite(_csPin, LOW);
        
        for (int module = _numModules - 1; module >= 0; module--) {
            uint8_t rowData = 0;
            
            for (int col = 0; col < 8; col++) {
                int globalCol = module * 8 + col;
                if (_displayBuffer[globalCol][row]) {
                    rowData |= (1 << col);
                }
            }
            
            SPI.transfer(REG_DIGIT0 + row);
            SPI.transfer(rowData);
        }
        
        digitalWrite(_csPin, HIGH);
    }
}

// Get font index for character
int MAX7219_FC16::getFontIndex(char c) {
    c = toupper(c);
    
    if (c == ' ') return 0;
    if (c >= 'A' && c <= 'Z') return c - 'A' + 1;
    if (c >= '0' && c <= '9') return 30 + (c - '0');
    if (c == '.') return 27;
    if (c == '\'') return 28;
    if (c == ':') return 29;
    
    return 0;
}

// Get number font index
int MAX7219_FC16::getNumberFontIndex(int num) {
    if (num >= 0 && num <= 9) return 30 + num;
    return 30; // Return 0 if invalid
}

// Draw a character with specified font size
void MAX7219_FC16::drawChar(int pos, char c, FontSize size) {
    if (size == FONT_TINY) {
        uint16_t charData[3];
        getTinyFontChar(c, charData);
        
        for (int i = 0; i < 3; i++) {
            setColumn(pos + i, charData[i] & 0xFF);
        }
    } 
    else if (size == FONT_NORMAL) {
        uint8_t charData[5];
        getNormalFontChar(c, charData);
        
        for (int i = 0; i < 5; i++) {
            setColumn(pos + i, charData[i]);
        }
    }
}

// Draw a number with specified font size
void MAX7219_FC16::drawNumber(int pos, int num, FontSize size) {
    if (size == FONT_TINY) {
        char c = '0' + num;
        drawChar(pos, c, FONT_TINY);
    }
    else if (size == FONT_NORMAL) {
        char c = '0' + num;
        drawChar(pos, c, FONT_NORMAL);
    }
    else if (size == FONT_BIG) {
        if (num >= 0 && num <= 9) {
            uint8_t charData[20];
            getBigFontChar(num, charData);
            
            for (int i = 0; i < 20; i++) {
                setColumn(pos + i, charData[i]);
            }
        }
    }
}

// Draw text at position
void MAX7219_FC16::drawText(int pos, const char* text, FontSize size) {
    int cursor = pos;
    int charWidth = (size == FONT_TINY) ? 4 : 6; // 3 columns + 1 space for tiny, 5 columns + 1 space for normal
    
    for (int i = 0; text[i]; i++) {
        drawChar(cursor, text[i], size);
        cursor += charWidth;
    }
}

// Scroll text animation
void MAX7219_FC16::scrollText(const char* text, int speedDelay, FontSize size) {
    int charWidth = (size == FONT_TINY) ? 4 : 6;
    int width = strlen(text) * charWidth;
    
    for (int pos = _totalColumns; pos >= -width; pos--) {
        clear();
        drawText(pos, text, size);
        update();
        delay(speedDelay);
    }
}

// Show module numbers for testing
void MAX7219_FC16::showModuleNumbers(int delayMs) {
    clear();
    
    for (int m = 0; m < _numModules; m++) {
        drawNumber(m * 8 + 1, m, FONT_NORMAL);
    }
    
    update();
    delay(delayMs);
}

// Set brightness (0x00 to 0x0F)
void MAX7219_FC16::setIntensity(uint8_t intensity) {
    sendToAll(REG_INTENSITY, intensity);
}

// Shutdown display (power saving)
void MAX7219_FC16::shutdown() {
    sendToAll(REG_SHUTDOWN, 0x00);
}

// Wake up display
void MAX7219_FC16::wakeup() {
    sendToAll(REG_SHUTDOWN, 0x01);
}

// Draw invader sprite (0-11 for different invader types)
void MAX7219_FC16::drawInvader(int pos, int invaderType) {
    if (invaderType < 0 || invaderType >= 12) return;
    
    // Invader sprites start at index 67 in the normal font
    uint8_t invaderData[5];
    int invaderIndex = 67 + invaderType;
    
    for (int i = 0; i < 5; i++) {
        invaderData[i] = pgm_read_byte(&myfont[invaderIndex][i]);
        setColumn(pos + i, invaderData[i]);
    }
}