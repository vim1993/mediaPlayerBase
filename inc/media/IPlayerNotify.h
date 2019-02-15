#ifndef __IPLAYER_NOTIFY_1_0_H__
#define __IPLAYER_NOTIFY_1_0_H__

#include "type.h"
#include "lxlog.h"

typedef enum playermsg_e {
    PLAYERMSG_SOURCE_OK,
    PLAYERMSG_SOURCE_FAILED,
    PLAYERMSG_SOURCE_PREPARE_OK,
    PLAYERMSG_SOURCE_PREPARE_FAILED,
    PLAYERMSG_SOURCE_START_OK,
    PLAYERMSG_SOURCE_START_FAILED,
    PLAYERMSG_SOURCE_STOP_OK,
    PLAYERMSG_SOURCE_STOP_FAILED,
    PLAYERMSG_SOURCE_PAUSE_OK,
    PLAYERMSG_SOURCE_PAUSE_FAILED,
    PLAYERMSG_SOURCE_RESET_OK,
    PLAYERMSG_SOURCE_RESET_FAILED
}playermsg_e;

class PlayerMsg {
    unsigned int m_msgcode;
    int m_msgExtCode;

    public:
        PlayerMsg() {

        };

        PlayerMsg(unsigned int msgcode, int msgExtCode)
            : m_msgcode(msgcode)
            , m_msgExtCode(msgExtCode) {

            };

        unsigned int getMsgCode(void) const {
            return m_msgcode;
        };

        int getMsgExtCode(void) const {
            return m_msgExtCode;
        };

        void setMsgCode(unsigned int msgcode) {
            m_msgcode = msgcode;
        };

        void setMsgExtCode(int msgExtCode) {
            m_msgExtCode = msgExtCode;
        };
};

class IPlayerNotify {
    public:
        virtual void playerMsgNotify() = 0;
};

class IPlayerListener {
    public:
        virtual void playerListener(unsigned int msgcode, int msgExtCode) {

        };
};

typedef void (*notify_callback_f)(int msg, int ext1);

#endif