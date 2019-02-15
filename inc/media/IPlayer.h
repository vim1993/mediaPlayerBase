#ifndef __IPLAYER_1_0_H__
#define __IPLAYER_1_0_H__

#include "IPlayerNotify.h"
#include "type.h"

class IPlayer {

    public:
        virtual status_t setDataSource(const char * url) = 0;
        virtual status_t prepare(void) = 0;
        virtual status_t prepareSync(void) = 0;
        virtual status_t start(void) = 0;
        virtual status_t stop(void) = 0;
        virtual status_t pause(void) = 0;
        virtual status_t seekto(int msec) = 0;
        virtual status_t reset(void) = 0;
        virtual status_t setListener(IPlayerListener & listener) = 0;

};

#endif