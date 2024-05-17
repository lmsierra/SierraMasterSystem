#pragma once
#include <stdint.h>

// The Z80 was officially described as supporting 16 - bit(64 KB) memory addressing, and 8 - bit(256 ports) I / O - addressing.
using byte = uint8_t;  // 1 byte
using word = uint16_t; // 2 bytes

// SMS uses a 6 bit color format (--BBGGRR), but we use 8 bit per component for rendering
struct RGBColor
{
    RGBColor() = delete;
    RGBColor(byte _R, byte _G, byte _B) : R(_R), G(_G), B(_B) {}

    static RGBColor GetFromSMSColor(byte color)
    {
        // 255 bits into 3 posible color values
        constexpr byte conversion_factor = 255 / 3;
        
        const byte R = (color & 0b00000011) * conversion_factor;
        const byte G = ((color >> 2) & 0b00000011) * conversion_factor;
        const byte B = ((color >> 4) & 0b00000011) * conversion_factor;

        return RGBColor(R, G, B);
    }

    byte R;
    byte G;
    byte B;
};
