# include "SDLInterface.h"

#include "Types.h"

// Suggested in stb_image.h header
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool SDLInterface::InitWindow(uint32_t window_width, uint32_t window_height, uint32_t system_width, uint32_t system_height)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        return false;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);

    // cache properties
    cached_width = system_width;
    cached_height = system_height;

    // create window
    m_window = SDL_CreateWindow("SierraMasterSystem", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_INPUT_FOCUS);
    
    // create and init renderer
    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    SDL_RenderSetLogicalSize(m_renderer, system_width, system_height);

    // create texture
    m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, system_width, system_height);

    InitIcon("Images/Logo.png");

    return true;
}

void SDLInterface::RenderFrame(const byte* const buffer)
{
    assert(m_renderer != nullptr);
    assert(m_texture != nullptr);

    SDL_UpdateTexture(m_texture, NULL, buffer, cached_width * sizeof(byte) * 3);
    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
    SDL_RenderPresent(m_renderer);
}

void SDLInterface::InitIcon(const char* ImagePath)
{
    const int required_comp = STBI_rgb_alpha;
    int width  = 0;
    int height = 0;
    int comp   = 0;
    
    if (stbi_uc* icon = stbi_load(ImagePath, &width, &height, &comp, required_comp))
    {
        SDL_Surface* surface = SDL_CreateRGBSurfaceFrom((void*)icon, 16, 16, 32, 4 * width, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
        SDL_SetWindowIcon(m_window, surface);
    }
}

void SDLInterface::Quit()
{
    SDL_Quit();
}
