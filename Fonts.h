#ifndef Fonts_h
#define Fonts_h

#include <Arduino.h>

// Font size enumeration
enum FontSize {
    FONT_TINY = 0,    // 3 columns per character
    FONT_NORMAL = 1,  // 5 columns per character
    FONT_BIG = 2      // 20 columns per character
};

// Normal font (79 characters, 5 columns each)
extern const int font_count;
extern const uint8_t myfont[79][5] PROGMEM;

// Big font (10 characters, 20 columns each)
extern const int bigfont_count;
extern const uint8_t mybigfont[10][20] PROGMEM;

// Tiny font (42 characters, 3 columns each)
extern const int tinyfont_count;
extern const uint16_t mytinyfont[42][3] PROGMEM;

// Helper functions to get character data
void getNormalFontChar(char c, uint8_t* buffer);
void getBigFontChar(int num, uint8_t* buffer);
void getTinyFontChar(char c, uint16_t* buffer);

#endif