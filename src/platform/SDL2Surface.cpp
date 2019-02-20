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
#include "lxMsgQue.h"
#include "lxOs.h"

bool SDL2Surface::init(void) {
    m_lock = pthread_resource_lock_new();
    if(m_lock == NULL) {
        return false;
    }

    return true;
}

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

    LOG_ERROR_PRINT("[x:%d][y:%d][w:%d][h:%d]\n", m_rect.x, m_rect.y, m_rect.w, m_rect.h);
    m_texture = SDL_CreateTexture(m_render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, m_rect.w, m_rect.h);
    if(m_texture == NULL) {
        LOG_ERROR_PRINT("create SDL_CreateTexture failed:%s\n", SDL_GetError());
        return false;
    }

    init();

    return true;
}

bool SDL2Surface::releaseSurface(void) {
    SDL_Quit();
    pthread_resource_lock_delete(m_lock);

    return true;
}

bool SDL2Surface::showFrame(void * frame) {
    if(frame == NULL) {
        return NULL;
    }

    PMUTEX_LOCK(m_lock);
    m_frame = (YUVFrame_t *)frame;

    SDL_UpdateYUVTexture(m_texture, &m_rect,
        m_frame->Yplane, m_frame->Ypitch,
            m_frame->Uplane, m_frame->Upitch,
                m_frame->Vplane, m_frame->Vpitch);
    SDL_RenderClear(m_render);
    SDL_RenderCopy(m_render, m_texture, NULL, &m_rect);
    SDL_RenderPresent(m_render);
    PMUTEX_UNLOCK(m_lock);

    return true;
}

bool SDL2Surface::showText(const char * text) {
    if(text == NULL) {
        LOG_ERROR_PRINT("text is null\n");
        return false;
    }
    PMUTEX_LOCK(m_lock);


    PMUTEX_UNLOCK(m_lock);

    return true;
}

bool SDL2Surface::showPicture(const char * picpath) {
    if(picpath == NULL) {
        LOG_ERROR_PRINT("text is null\n");
        return false;
    }

    PMUTEX_LOCK(m_lock);


    PMUTEX_UNLOCK(m_lock);


    return true;
}
