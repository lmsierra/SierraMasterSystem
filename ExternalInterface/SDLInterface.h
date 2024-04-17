#pragma once

#include "ExternalInterface.h"
#include "SDL.h"

class SDLInterface : public ExternalInterface
{
public:
    SDLInterface() = default;

    virtual bool InitWindow(uint32_t window_width, uint32_t window_height, uint32_t game_width, uint32_t game_height) override;
    virtual void RenderFrame(const byte* const buffer) override;
    virtual void Quit() override;

private:
    void InitIcon(const char* ImagePath);

private:
    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture*  m_texture  = nullptr;

    uint32_t cached_width;
    uint32_t cached_height;
};
