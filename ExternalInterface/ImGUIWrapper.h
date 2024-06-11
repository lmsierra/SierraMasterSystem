#pragma once

#include "SDL.h"
#include "Types.h"

class ImGUIWrapper
{
public:
    static void Init(const class SDLInterface* sdl_interface);
    static void Render(const class SDLInterface* sdl_interface);
    static void Tick(float DeltaTime);
    static void ProcessEvent(const SDL_Event* event);
    static void NewFrame();
    static void DrawRegisters(const class Z80* z80);
    static void Shutdown();

private:
    static void AddBinaryText(const char* format, const byte value);
    static void AddRegisterText(const char* title, const word value);
};

