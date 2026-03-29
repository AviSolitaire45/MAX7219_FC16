#include "MAX7219_FC16.h"
#include <ctype.h>
#include <string.h>

// Constructor
MAX7219_FC16::MAX7219_FC16(int csPin, int numModules) {
    _csPin = csPin;
    _numModules = numModules;
    _totalColumns = numModules * 8;
    _brightness = 8;
    
    // Initialize scroll state
    _scrollState.active = false;
    _scrollState.text = nullptr;
    
    // Allocate display buffer
    _displayBuffer = new uint8_t*[_totalColumns];
    _frameBuffer = new uint8_t*[_totalColumns];
    
    for (int i = 0; i < _totalColumns; i++) {
        _displayBuffer[i] = new uint8_t[8];
        _frameBuffer[i] = new uint8_t[8];
        for (int j = 0; j < 8; j++) {
            _displayBuffer[i][j] = 0;
            _frameBuffer[i][j] = 0;
        }
    }
}

// Destructor
MAX7219_FC16::~MAX7219_FC16() {
    for (int i = 0; i < _totalColumns; i++) {
        delete[] _displayBuffer[i];
        delete[] _frameBuffer[i];
    }
    delete[] _displayBuffer;
    delete[] _frameBuffer;
}

// Initialize the display with professional sequence
void MAX7219_FC16::begin(uint8_t intensity) {
    pinMode(_csPin, OUTPUT);
    SPI.begin();
    
    digitalWrite(_csPin, HIGH);
    delay(100);
    
    // Professional initialization sequence
    sendToAll(REG_SHUTDOWN, 0x00);     // Enter shutdown mode
    delay(10);
    sendToAll(REG_DISPLAYTEST, 0x00);  // Exit test mode
    sendToAll(REG_DECODEMODE, 0x00);   // No decode for matrix
    setIntensity(intensity);            // Set brightness
    sendToAll(REG_SCANLIMIT, 0x07);     // Scan all 8 digits
    clear();                            // Clear display
    sendToAll(REG_SHUTDOWN, 0x01);      // Exit shutdown mode
}

// Send command to all modules
void MAX7219_FC16::sendToAll(uint8_t cmd, uint8_t data) {
    digitalWrite(_csPin, LOW);
    for (int i = 0; i < _numModules; i++) {
        SPI.transfer(cmd);
        SPI.transfer(data);
    }
    digitalWrite(_csPin, HIGH);
    delayMicroseconds(1);
}

// Clear the display buffer
void MAX7219_FC16::clear() {
    for (int c = 0; c < _totalColumns; c++) {
        for (int r = 0; r < 8; r++) {
            _displayBuffer[c][r] = 0;
        }
    }
}

// Clear a specific buffer
void MAX7219_FC16::clearBuffer(uint8_t** buffer) {
    for (int c = 0; c < _totalColumns; c++) {
        for (int r = 0; r < 8; r++) {
            buffer[c][r] = 0;
        }
    }
}

// Double-buffering methods
void MAX7219_FC16::beginFrame() {
    // Copy current display to frame buffer
    for (int col = 0; col < _totalColumns; col++) {
        for (int row = 0; row < 8; row++) {
            _frameBuffer[col][row] = _displayBuffer[col][row];
        }
    }
}

void MAX7219_FC16::endFrame() {
    // Nothing needed, kept for symmetry
}

void MAX7219_FC16::clearFrame() {
    clearBuffer(_frameBuffer);
}

void MAX7219_FC16::setPixelFrame(int col, int row, bool state) {
    if (col < 0 || col >= _totalColumns) return;
    if (row < 0 || row >= 8) return;
    _frameBuffer[col][row] = state ? 1 : 0;
}

void MAX7219_FC16::setColumnFrame(int col, uint8_t value) {
    if (col < 0 || col >= _totalColumns) return;
    
    for (int row = 0; row < 8; row++) {
        _frameBuffer[col][row] = (value >> (7 - row)) & 1;
    }
}

void MAX7219_FC16::commitFrame() {
    // Atomic swap - flicker-free update
    uint8_t** temp = _displayBuffer;
    _displayBuffer = _frameBuffer;
    _frameBuffer = temp;
    update();
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
    delayMicroseconds(1);
}

// Set brightness
void MAX7219_FC16::setIntensity(uint8_t intensity) {
    _brightness = constrain(intensity, 0, 15);
    sendToAll(REG_INTENSITY, _brightness);
}

// Shutdown/Wakeup
void MAX7219_FC16::shutdown() {
    sendToAll(REG_SHUTDOWN, 0x00);
}

void MAX7219_FC16::wakeup() {
    sendToAll(REG_SHUTDOWN, 0x01);
}

// Draw character to specific buffer (internal)
void MAX7219_FC16::drawCharToBuffer(int pos, char c, FontSize size, uint8_t** buffer) {
    if (size == FONT_TINY) {
        uint16_t charData[3];
        getTinyFontChar(c, charData);
        
        for (int i = 0; i < 3; i++) {
            if (pos + i >= 0 && pos + i < _totalColumns) {
                uint8_t colData = charData[i] & 0xFF;
                for (int row = 0; row < 8; row++) {
                    buffer[pos + i][row] = (colData >> (4 - row)) & 1;
                }
            }
        }
    } 
    else if (size == FONT_NORMAL) {
        uint8_t charData[5];
        getNormalFontChar(c, charData);
        
        for (int i = 0; i < 5; i++) {
            if (pos + i >= 0 && pos + i < _totalColumns) {
                for (int row = 0; row < 8; row++) {
                    buffer[pos + i][row] = (charData[i] >> (7 - row)) & 1;
                }
            }
        }
    }
}

// Draw a character
void MAX7219_FC16::drawChar(int pos, char c, FontSize size) {
    drawCharToBuffer(pos, c, size, _displayBuffer);
}

// Draw a number
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
                if (pos + i < _totalColumns) {
                    for (int row = 0; row < 8; row++) {
                        _displayBuffer[pos + i][row] = (charData[i] >> (7 - row)) & 1;
                    }
                }
            }
        }
    }
}

// Draw text at position
void MAX7219_FC16::drawText(int pos, const char* text, FontSize size) {
    int cursor = pos;
    int charWidth = (size == FONT_TINY) ? 4 : 6;
    
    for (int i = 0; text[i]; i++) {
        drawChar(cursor, text[i], size);
        cursor += charWidth;
    }
}

// Start non-blocking scroll animation
void MAX7219_FC16::startScrollText(const char* text, int speedDelay, FontSize size, bool loop) {
    _scrollState.charWidth = (size == FONT_TINY) ? 4 : 6;
    _scrollState.textWidth = strlen(text) * _scrollState.charWidth;
    _scrollState.text = text;
    _scrollState.size = size;
    _scrollState.speedDelay = speedDelay;
    _scrollState.position = _totalColumns;
    _scrollState.lastUpdate = 0;
    _scrollState.active = true;
    _scrollState.loop = loop;
}

// Update scroll animation - call this in loop()
void MAX7219_FC16::updateScroll() {
    if (!_scrollState.active) return;
    
    if (millis() - _scrollState.lastUpdate >= _scrollState.speedDelay) {
        _scrollState.lastUpdate = millis();
        
        // Use double buffering for smooth animation
        beginFrame();
        clearFrame();
        
        // Draw current frame
        int cursor = _scrollState.position;
        int textLen = strlen(_scrollState.text);
        
        for (int i = 0; i < textLen; i++) {
            int colPos = cursor + (i * _scrollState.charWidth);
            
            // Draw if character is visible
            if (colPos < _totalColumns && colPos + 5 > 0) {
                drawCharToBuffer(colPos, _scrollState.text[i], _scrollState.size, _frameBuffer);
            }
        }
        
        // Commit the frame (flicker-free)
        commitFrame();
        
        // Move position
        _scrollState.position--;
        
        // Check if scrolling is complete
        if (_scrollState.position < -_scrollState.textWidth) {
            if (_scrollState.loop) {
                _scrollState.position = _totalColumns;
            } else {
                _scrollState.active = false;
            }
        }
    }
}

// Stop scrolling
void MAX7219_FC16::stopScroll() {
    _scrollState.active = false;
}

// Check if scrolling is active
bool MAX7219_FC16::isScrolling() {
    return _scrollState.active;
}

// Legacy blocking scroll (kept for compatibility)
void MAX7219_FC16::scrollText(const char* text, int speedDelay, FontSize size) {
    int charWidth = (size == FONT_TINY) ? 4 : 6;
    int width = strlen(text) * charWidth;
    
    for (int pos = _totalColumns; pos >= -width; pos--) {
        beginFrame();
        clearFrame();
        drawText(pos, text, size);
        commitFrame();
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

// Draw invader sprite
void MAX7219_FC16::drawInvader(int pos, int invaderType) {
    if (invaderType < 0 || invaderType >= 12) return;
    
    uint8_t invaderData[5];
    int invaderIndex = 67 + invaderType;
    
    // Add bounds check for font array
    if (invaderIndex < 0 || invaderIndex >= font_count) return;
    
    for (int i = 0; i < 5; i++) {
        invaderData[i] = pgm_read_byte(&myfont[invaderIndex][i]);
        setColumn(pos + i, invaderData[i]);
    }
}

// Get font index for character
int MAX7219_FC16::getFontIndex(char c) {
    c = toupper(c);
    
    if (c == ' ') return 0;
    if (c >= 'A' && c <= 'Z') return c - 'A' + 1;
    if (c >= '0' && c <= '9') return 31 + (c - '0');
    if (c == '.') return 27;
    if (c == '\'') return 28;
    if (c == ':') return 29;
    
    return 0;
}

// Get number font index
int MAX7219_FC16::getNumberFontIndex(int num) {
    if (num >= 0 && num <= 9) return 31 + num;
    return 31;
}