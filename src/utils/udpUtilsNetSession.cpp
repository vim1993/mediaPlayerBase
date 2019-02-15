#include <iostream>

#include "type.h"
#include "lxOs.h"
#include "lxlog.h"

#include "IUtilsNetSession.h"
#include "udpUtilsNetSession.h"

using namespace std;

bool udpUtilsNetSession::createSession(unsigned int udpport, const char * ip) {

    return true;
}

bool udpUtilsNetSession::createSession(const char * url) {

    return true;
}

bool udpUtilsNetSession::sendData(const char * data, unsigned int datalen) {

    return true;
}

bool udpUtilsNetSession::recvData(char * pBuffer, unsigned int bufferlen) {

    return true;
}

bool udpUtilsNetSession::resetSession(void) {

    return true;
}

udpUtilsNetSession::udpUtilsNetSession() {

}

udpUtilsNetSession::udpUtilsNetSession(const char * url) {

}

udpUtilsNetSession::udpUtilsNetSession(unsigned int udpport, const char * ip) {

}

udpUtilsNetSession::~udpUtilsNetSession() {

}