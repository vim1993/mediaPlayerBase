#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "SDL2/SDL.h"

#ifdef __cplusplus
}
#endif

#include "ISurface.h"
#include "SDL2Surface.h"
#include "type.h"
#include "lxlog.h"

bool SDL2Surface::createSurface(surfaceRect & rect) {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        LOG_ERROR_PRINT("createSurface failed\n");
        return false;
    }

    memcpy(&m_rect, &rect, sizeof(surfaceRect));

    m_windows = SDL_CreateWindow("media", m_rect.x, m_rect.y, m_rect.w, m_rect.h, SDL_WINDOW_OPENGL);
    if(m_windows == NULL) {
        LOG_ERROR_PRINT("create surface failed\n");
        return false;
    }

    m_render = SDL_CreateRenderer(m_windows, -1, 0);
    if(m_render == NULL) {
        LOG_ERROR_PRINT("create SDL_CreateRenderer failed\n");
        return false;
    }

    m_texture = SDL_CreateTexture(m_render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, m_rect.w, m_rect.y);
    if(m_texture == NULL) {
        LOG_ERROR_PRINT("create SDL_CreateTexture failed\n");
        return false;
    }

    return true;
}

bool SDL2Surface::releaseSurface(void) {
    SDL_Quit();
}
