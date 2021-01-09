#pragma once
#include <stdint.h>

// The Z80 was officially described as supporting 16 - bit(64 KB) memory addressing, and 8 - bit(256 ports) I / O - addressing.
using byte = uint8_t;  // 1 byte
using word = uint16_t; // 2 bytes

struct Color
{
	Color(byte rgb) : value(rgb) {}

	byte R()   { return value & 0b00000011; }
	byte G()   { return value & 0b00001100; }
	byte B()   { return value & 0b00110000; }
	byte RGB() { return value; }

private:
	byte value;
};
