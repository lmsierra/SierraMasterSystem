#pragma once
#include <stdint.h>

// The Z80 was officially described as supporting 16 - bit(64 KB) memory addressing, and 8 - bit(256 ports) I / O - addressing.
using byte = uint8_t;  // 1 byte
using word = uint16_t; // 2 bytes

struct RGBColor
{
    RGBColor() = delete;
    RGBColor(byte _R, byte _G, byte _B) : R(_R), G(_G), B(_B) {}

private:
    byte R;
    byte G;
    byte B;
};

// From: http://bifi.msxnet.org/msxnet/tech/tms9918a.txt
static const RGBColor OriginalColorPalette[16]
{
    RGBColor(  0,   0,   0), // 0 - Transparent
    RGBColor(  0,   0,   0), // 1 - Black
    RGBColor( 33, 200,  66), // 2 - Medium Green
    RGBColor( 94, 220, 120), // 3 - Light green
    RGBColor( 84,  85, 237), // 4 - Dark Blue
    RGBColor(125, 118, 252), // 5 - Light Blue
    RGBColor(212,  82,  77), // 6 - Dark red
    RGBColor( 66, 235, 245), // 7 - Cyan
    RGBColor(252,  85,  84), // 8 - Medium red
    RGBColor(255, 121, 120), // 9 - Light red
    RGBColor(212, 193,  84), // A - Dark yellow
    RGBColor(230, 206, 128), // B - Light yellow
    RGBColor( 33, 176,  59), // C - Dark green
    RGBColor(201,  91, 186), // D - Magenta
    RGBColor(204, 204, 204), // E - Gray
    RGBColor(255, 255, 255), // F - White
};

