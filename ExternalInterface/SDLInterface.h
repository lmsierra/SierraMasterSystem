#pragma once

#include "ExternalInterface.h"
#include "SDL.h"

class SDLInterface
{
public:
    SDLInterface() = default;

    bool InitWindow(uint32_t window_width, uint32_t window_height, uint32_t game_width, uint32_t game_height);
    void RenderFrame(const byte* const buffer);
    void Quit();
    bool ExitRequested(const SDL_Event& event);

    inline SDL_Window* GetWindow() const { return m_window; }
    inline SDL_Renderer* GetRenderer() const { return m_renderer; }

private:
    void InitIcon(const char* ImagePath);

private:
    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture*  m_texture  = nullptr;

    uint32_t cached_width;
    uint32_t cached_height;
};
