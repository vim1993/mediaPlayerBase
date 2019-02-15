#ifndef __I_Utils_NET_SESSION_H__
#define __I_Utils_NET_SESSION_H__

#include <iostream>

#include "type.h"

using namespace std;

class IUtilsNetSession {

    protected:
        string m_ip;
        string m_port;
        string m_uri;

        int parse_uri(const char * uri) {
            if(uri == NULL) {
                return RET_FAILED;
            }

            m_uri = uri;

            size_t pos = m_uri.find("//");
            if(pos == string::npos) {
                return -1;
            }

            string substr = m_uri.substr(pos+2);
            pos = substr.find("/");
            substr = substr.substr(0, pos);
            pos = substr.find(":");
            m_ip = substr.substr(0, pos);
            m_port = substr.substr(pos);

            return RET_OK;
        };

    public:
        virtual bool createSession(unsigned int udpport, const char * ip = NULL) = 0;
        virtual bool createSession(const char * url) = 0;
        virtual bool sendData(const char * data, unsigned int datalen) = 0;
        virtual bool recvData(char * pBuffer, unsigned int bufferlen) = 0;
        virtual bool resetSession(void) = 0;
};

#endif