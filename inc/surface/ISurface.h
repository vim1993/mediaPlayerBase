#ifndef __ISURFACE_H__
#define __ISURFACE_H__

#include <iostream>

#include "type.h"
using namespace std;

typedef struct surfaceRect {
    i32int x;
    i32int y;
    i32int w;
    i32int h;
}surfaceRect, *PRECT;

typedef struct surfaceMsg {
    u32int msgid;
    void * data;
}surfaceMsg;

enum surfaceMsgID_e {
    SURFACE_FLUSH,
    SURFACE_FRAME,
    SURFACE_PIC,
    SURFACE_TEXT
};

typedef void (*surfaceChangedListener)(void);

class ISurface {
    protected:
        surfaceChangedListener m_notify;
        bool surfaceChangedCallBack(void) const{
            if(m_notify) {
                m_notify();
                return true;
            }

            return false;
        }

        void registerSurfaceChangedListener(surfaceChangedListener listener) {
            m_notify = listener;
        };

    public:
        virtual bool createSurface(surfaceRect & rect) = 0;
        virtual bool releaseSurface(void) = 0;
        virtual bool showFrame(void * frame) = 0;
        virtual bool showText(const char * text) = 0;
        virtual bool showPicture(const char * picpath) = 0;
        virtual const surfaceRect * getSurfaceRect(void) const = 0;
};

#endif