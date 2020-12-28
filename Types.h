#pragma once
#include <stdint.h>

// The Z80 was officially described as supporting 16 - bit(64 KB) memory addressing, and 8 - bit(256 ports) I / O - addressing.
using byte = uint8_t;  // 1 byte
using word = uint16_t; // 2 bytes
