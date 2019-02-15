#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "SDL2/SDL.h"

#ifdef __cplusplus
}
#endif

#include "IPortingPlayer.h"
#include "FFMPEGPlayer.h"
#include "type.h"
#include "lxlog.h"

status_t FFMPEGPlayer::setDataSource(const char * url) {
    if(url == NULL) {
        LOG_DEBUG_PRINT("[%s][%d]! source is null\n");
    }
    memcpy(m_uri, url, strlen(url));
    m_playerStatus = STATUS_DATASOURCE;

    return RET_OK;
}

status_t FFMPEGPlayer::prepare(void) {
    int avindex = 0;

    av_register_all();
    pAVFormatCt = avformat_alloc_context();
    if(pAVFormatCt == NULL) {
        m_playerStatus = STATUS_ERROR;
        this->m_notify(PLAYERMSG_SOURCE_PREPARE_FAILED, RET_FAILED);
        return RET_FAILED;
    }

    avformat_open_input(&pAVFormatCt, m_uri, NULL, NULL);
    avformat_find_stream_info(pAVFormatCt, NULL);

    for(avindex = 0 ;avindex < pAVFormatCt->nb_streams; avindex++) {
        if(pAVFormatCt->streams[avindex]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = avindex;
            break;
        }
    }

    if(videoStreamIndex == -1) {
        m_playerStatus = STATUS_ERROR;
        this->m_notify(PLAYERMSG_SOURCE_PREPARE_FAILED, RET_FAILED);
        return RET_FAILED;
    }

    pAVCodeCt = pAVFormatCt->streams[videoStreamIndex]->codec;
    pAVCodec = avcodec_find_decoder(pAVCodeCt->codec_id);
    if(pAVCodec == NULL) {
        m_playerStatus = STATUS_ERROR;
        this->m_notify(PLAYERMSG_SOURCE_PREPARE_FAILED, RET_FAILED);
        return RET_FAILED;
    }

    if(avcodec_open2(pAVCodeCt, pAVCodec, NULL) != 0) {
        m_playerStatus = STATUS_ERROR;
        this->m_notify(PLAYERMSG_SOURCE_PREPARE_FAILED, RET_FAILED);
        return RET_FAILED;
    }

    this->m_notify(PLAYERMSG_SOURCE_PREPARE_OK, RET_OK);
    return RET_OK;
}

status_t FFMPEGPlayer::prepareSync(void) {
    return RET_OK;
}

status_t FFMPEGPlayer::start(void) {
    return RET_OK;
}

status_t FFMPEGPlayer::stop(void) {
    return RET_OK;
}

status_t FFMPEGPlayer::pause(void) {
    return RET_OK;
}

status_t FFMPEGPlayer::seekto(int msec) {
    return RET_OK;
}

status_t FFMPEGPlayer::reset(void) {
    memset(m_uri, 0x00, sizeof(m_uri));
    m_playerStatus = STATUS_IDLE;

    return RET_OK;
}

FFMPEGPlayer::FFMPEGPlayer()
    : m_playerStatus(STATUS_IDLE)
    , videoStreamIndex(UNVAILED_HANDLE) {

    this->m_notify = NULL;;
    pAVFormatCt = NULL;
    pAVCodeCt = NULL;
    pAVCodec = NULL;
    pAVframe = NULL;
    pAVframeYUV = NULL;
    pAVPacket = NULL;
    out_buffer = NULL;
    memset(m_uri, 0x00, sizeof(m_uri));
}

FFMPEGPlayer::~FFMPEGPlayer() {

}
