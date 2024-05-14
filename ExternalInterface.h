#pragma once

#include "Types.h"
#include <stdint.h>

class ExternalInterface
{
public:

	ExternalInterface() = default;
	virtual ~ExternalInterface() {}

	virtual bool InitWindow(uint32_t window_width, uint32_t window_height, uint32_t game_width, uint32_t game_height) = 0;
	virtual void RenderFrame(const byte* const buffer) = 0;
	virtual void Quit() = 0;

};
