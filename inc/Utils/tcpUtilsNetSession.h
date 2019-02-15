#ifndef __TCP_SESSION_H__
#define __TCP_SESSION_H__

#include "IUtilsNetSession.h"

class tcpUtilsNetSession : public IUtilsNetSession {

    int m_sockfd;

    public:
        bool createSession(unsigned int udpport, const char * ip = NULL);
        bool createSession(const char * url);
        bool sendData(const char * data, unsigned int datalen);
        bool recvData(char * pBuffer, unsigned int bufferlen);
        bool resetSession(void);

        tcpUtilsNetSession();
        tcpUtilsNetSession(const char * url);
        tcpUtilsNetSession(unsigned int udpport, const char * ip = NULL);

        ~tcpUtilsNetSession();
};

#endif