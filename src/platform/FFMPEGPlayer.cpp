#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

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
#include "lxOs.h"

status_t FFMPEGPlayer::parseFrame(void) {
    if(m_playerStatus != STATUS_START) {
        return RET_FAILED;
    }

    YUVFrame_t yuvframe = {0};

    int iret = -1;
    int got_picture_ptr = -1;
    while(1) {
        if(av_read_frame(pAVFormatCt, pAVPacket) == 0) {
            if(pAVPacket->stream_index == videoStreamIndex) {
                iret = avcodec_decode_video2(pAVCodeCt, pAVframe, &got_picture_ptr, pAVPacket);
                if(iret < 0) {
                    continue;
                }

                if(got_picture_ptr > 0) {
                    sws_scale(m_pSwsCt, pAVframe->data, pAVframe->linesize, 0, pAVframe->height, pAVframeYUV->data, pAVframeYUV->linesize);

                    yuvframe.Yplane = pAVframeYUV->data[0];
                    yuvframe.Ypitch = pAVframeYUV->linesize[0];
                    yuvframe.Uplane = pAVframeYUV->data[1];
                    yuvframe.Upitch = pAVframeYUV->linesize[1];
                    yuvframe.Vplane = pAVframeYUV->data[2];
                    yuvframe.Vpitch = pAVframeYUV->linesize[2];

                    m_surface->showFrame(&yuvframe);
                }
            }

            return RET_OK;
        }
    }
    return RET_FAILED;
}

void * FFMPEGPlayer::ffmpeg_msgproc(void * param) {
    playerMsg_t * msg;
    FFMPEGPlayer * pThis = (FFMPEGPlayer *)param;
    while(1) {
        msg = (playerMsg_t *)pThis->m_msgque->pop_front(pThis->m_msgque);
        if(msg == NULL) {
            LOG_ERROR_PRINT("msg_que error\n");
            break;
        }

        switch(msg->msgid) {
            case PLAYER_PARSE:
                pThis->parseFrame();
                break;

            case PLAYER_STOP:
                pThis->m_timer->release_timer(pThis->m_timer);
                break;

            case PLAYER_PAUSE:
                pThis->m_timer->pause_timer(pThis->m_timer);
                break;
        }

        if(msg->msgid == PLAYER_STOP) {
            pThis->m_msgque->release_buffer(pThis->m_msgque, msg);
            break;
        }

        pThis->m_msgque->release_buffer(pThis->m_msgque, msg);
    }

    return NULL;
}

void FFMPEGPlayer::timer_notify(void * param) {
    if(param == NULL) {
        LOG_ERROR_PRINT("timer_notify ...NULL\n");
        return;
    }

    FFMPEGPlayer * pThis = (FFMPEGPlayer *)param;
    playerMsg_t msg = {0};
    msg.msgid = PLAYER_PARSE;
    pThis->m_msgque->push_back(pThis->m_msgque, &msg, sizeof(playerMsg_t));
    return;
}


status_t FFMPEGPlayer::setDataSource(const char * url) {
    if(url == NULL) {
        LOG_DEBUG_PRINT("[%s][%d]! source is null\n");
    }
    memcpy(m_uri, url, strlen(url));
    m_playerStatus = STATUS_DATASOURCE;

    return RET_OK;
}

status_t FFMPEGPlayer::prepare(void) {

    if(m_playerStatus != STATUS_DATASOURCE) {
        this->m_notify(PLAYERMSG_SOURCE_PREPARE_FAILED, -1);
        return RET_FAILED;
    }

    int avindex = 0;
    int extcode = -1;
    int iret = RET_FAILED;
    if(initSource() != BOOL_TRUE) {
        extcode = 1;
        goto exit;
    }

    av_register_all();
    avformat_network_init();
    pAVFormatCt = avformat_alloc_context();
    if(pAVFormatCt == NULL) {
        extcode = 2;
        goto exit;
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
        extcode = 3;
        goto exit;
    }

    pAVCodeCt = pAVFormatCt->streams[videoStreamIndex]->codec;
    pAVCodec = avcodec_find_decoder(pAVCodeCt->codec_id);
    if(pAVCodec == NULL) {
        extcode = 4;
        goto exit;
    }

    if(avcodec_open2(pAVCodeCt, pAVCodec, NULL) != 0) {
        extcode = 5;
        goto exit;
    }

    pAVPacket = av_packet_alloc();
    if(pAVPacket == NULL) {
        extcode = 6;
        goto exit;
    }

    pAVframe = av_frame_alloc();
    if(pAVframe == NULL) {
        extcode = 7;
        goto exit;
    }

    pAVframeYUV = av_frame_alloc();
    if(pAVframeYUV == NULL) {
        extcode = 8;
        goto exit;
    }

    out_buffer =(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,  pAVCodeCt->width, pAVCodeCt->height,1));
    av_image_fill_arrays(pAVframeYUV->data, pAVframeYUV->linesize,out_buffer,
                AV_PIX_FMT_YUV420P,pAVCodeCt->width, pAVCodeCt->height,1);

    m_pSwsCt = sws_getContext(pAVCodeCt->width, pAVCodeCt->height, pAVCodeCt->pix_fmt,
                                m_surface->getSurfaceRect()->w, m_surface->getSurfaceRect()->h, AV_PIX_FMT_YUV420P,
                                SWS_BICUBIC, NULL, NULL, NULL);
    if(!m_pSwsCt)
    {
        extcode = 9;
        goto exit;
    }

    av_dump_format(pAVFormatCt, 0, m_uri, 0);

    iret = RET_OK;
exit:
    if(iret == RET_FAILED) {
        m_playerStatus = STATUS_ERROR;
        this->m_notify(PLAYERMSG_SOURCE_PREPARE_FAILED, extcode);
        return RET_FAILED;
    }

    this->m_notify(PLAYERMSG_SOURCE_PREPARE_OK, RET_OK);
    m_playerStatus = STATUS_PREPARE;
    return RET_OK;
}

status_t FFMPEGPlayer::prepareSync(void) {
    return RET_OK;
}

status_t FFMPEGPlayer::start(void) {
    if(m_playerStatus == STATUS_PREPARE) {
        m_timer->start_timer(m_timer, 20, timer_notify, this);
    } else if(m_playerStatus != STATUS_ERROR || m_playerStatus != STATUS_IDLE) {
        m_timer->restart_timer(m_timer);
    }

    this->m_notify(PLAYERMSG_SOURCE_START_OK, RET_OK);

    m_playerStatus = STATUS_START;
    return RET_OK;
}

status_t FFMPEGPlayer::stop(void) {

    playerMsg_t msg = {0};
    msg.msgid = PLAYER_STOP;
    m_msgque->push_back(m_msgque, &msg, sizeof(playerMsg_t));
    m_playerStatus = STATUS_STOP;
    this->m_notify(PLAYERMSG_SOURCE_STOP_OK, RET_OK);

    return RET_OK;
}

status_t FFMPEGPlayer::pause(void) {

    playerMsg_t msg = {0};
    msg.msgid = PLAYER_PAUSE;
    m_msgque->push_back(m_msgque, &msg, sizeof(playerMsg_t));
    m_playerStatus = STATUS_PAUSE;
    this->m_notify(PLAYERMSG_SOURCE_PAUSE_OK, RET_OK);

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

BOOLTYPE FFMPEGPlayer::initSource(void) {
    if(pthread_create(&m_pthid, NULL, ffmpeg_msgproc, this) != 0) {
        LOG_ERROR_PRINT("start ffmpeg pthread failed\n");
        return BOOL_FALSE;
    }

    m_msgque = NEW(msgque_obj);
    if(m_msgque == NULL) {
        return BOOL_FALSE;
    }

    m_timer = NEW(timer_obj);
    if(m_timer == NULL) {
        return BOOL_FALSE;
    }

    return BOOL_TRUE;
}

status_t FFMPEGPlayer::setSurface(ISurface * surface) {
    if(surface == NULL) {
        return RET_FAILED;
    }

    m_surface = surface;

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
    m_msgque = NULL;
    memset(m_uri, 0x00, sizeof(m_uri));

    LOG_ERROR_PRINT("FFMPEGPlayer construct...\n");
}

FFMPEGPlayer::~FFMPEGPlayer() {
    if(m_msgque) {
        DELETE(msgque_obj, m_msgque);
    }

    if(m_timer) {
        DELETE(timer_obj, m_timer);
    }

    pthread_join(m_pthid, NULL);

    if(m_pSwsCt) {
        sws_freeContext(m_pSwsCt);
    }
    if(pAVframe) {
        av_frame_free(&pAVframe);
    }

    if(pAVframeYUV) {
        av_frame_free(&pAVframeYUV);
    }

    if(out_buffer) {
        av_free(out_buffer);
    }

    if(pAVPacket) {
        av_packet_free(&pAVPacket);
    }

    if(pAVCodeCt) {
        avcodec_close(pAVCodeCt);
    }

    avformat_network_deinit();

    if(pAVFormatCt) {
        avformat_close_input(&pAVFormatCt);
    }
}
