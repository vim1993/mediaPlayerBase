#ifndef __ISURFACE_H__
#define __ISURFACE_H__

#include <iostream>

#include "type.h"
using namespace std;

typedef struct surfaceRect {
    uint x;
    uint y;
    uint w;
    uint h;
}surfaceRect, *PRECT;

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

};

#endif