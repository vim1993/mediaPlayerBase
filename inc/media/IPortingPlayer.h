#ifndef __IPORTING_PLAYER_1_0_H__
#define __IPORTING_PLAYER_1_0_H__

#include "IPlayerNotify.h"
#include "type.h"

typedef enum player_status {
    STATUS_OK,
    STATUS_DATASOURCE,
    STATUS_PREPARE,
    STATUS_START,
    STATUS_STOP,
    STATUS_SEEK,
    STATUS_PAUSE,
    STATUS_RESET,
    STATUS_IDLE,
    STATUS_ERROR
}player_status_e;

class IPortingPlayer {

    protected:
        notify_callback_f m_notify;

        void sendListenerMsg(unsigned int msgcode, int extcode) {
            if(m_notify != NULL) {
                m_notify(msgcode, extcode);
            }

            return;
        };

        status_t setListener(notify_callback_f    fnotify) {
            if(fnotify == NULL) {
                return RET_FAILED;
            }

            m_notify = fnotify;

            return RET_OK;
        }

    public:
        virtual status_t setDataSource(const char * url) = 0;
        virtual status_t prepare(void) = 0;
        virtual status_t prepareSync(void) = 0;
        virtual status_t start(void) = 0;
        virtual status_t stop(void) = 0;
        virtual status_t pause(void) = 0;
        virtual status_t seekto(int msec) = 0;
        virtual status_t reset(void) = 0;
};

#endif
