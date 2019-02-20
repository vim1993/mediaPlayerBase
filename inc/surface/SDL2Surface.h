#ifndef __SDL2_SURFACE_H__
#define __SDL2_SURFACE_H__

#include "ISurface.h"
#include "lxlog.h"
#include "lxOs.h"
#include "lxMsgQue.h"
#include "lxQue.h"

class SDL2Surface : public ISurface {


    pthread_t m_pthid;
    YUVFrame_t * m_frame;
    mutex_lock * m_lock;
    SDL_Rect m_rect;
    SDL_Window * m_windows;
    SDL_Renderer * m_render;
    SDL_Texture * m_texture;

    bool init(void);

    public:

        bool createSurface(surfaceRect & rect);
        bool releaseSurface(void);
        bool showFrame(void * frame);
        bool showText(const char * text);
        bool showPicture(const char * picpath);

        const surfaceRect * getSurfaceRect(void) const      {
            return (surfaceRect *)&m_rect;
        }

        SDL2Surface() {};
        ~SDL2Surface() {};
};

#endif