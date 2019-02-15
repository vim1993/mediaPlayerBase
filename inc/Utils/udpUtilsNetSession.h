#ifndef __UDP_SESSION_H__
#define __UDP_SESSION_H__

#include "IUtilsNetSession.h"

class udpUtilsNetSession : public IUtilsNetSession {

    int m_sockfd;

    public:

        bool createSession(unsigned int udpport, const char * ip = NULL);
        bool createSession(const char * url);
        bool sendData(const char * data, unsigned int datalen);
        bool recvData(char * pBuffer, unsigned int bufferlen);
        bool resetSession(void);

        udpUtilsNetSession();
        udpUtilsNetSession(const char * url);
        udpUtilsNetSession(unsigned int udpport, const char * ip = NULL);

        ~udpUtilsNetSession();
};

#endif

