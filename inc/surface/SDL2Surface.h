#ifndef __SDL2_SURFACE_H__
#define __SDL2_SURFACE_H__

#include "ISurface.h"

class SDL2Surface : public ISurface {
    surfaceRect m_rect;
    SDL_Window * m_windows;
    SDL_Renderer * m_render;
    SDL_Texture * m_texture;

    public:
        bool createSurface(surfaceRect & rect);
        bool releaseSurface(void);
        SDL2Surface() {};
        ~SDL2Surface() {};
};

#endif