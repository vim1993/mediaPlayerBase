#ifndef __FFMPEG_PLAYER_H__
#define __FFMPEG_PLAYER_H__

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
#include "type.h"
#include "lxMsgQue.h"
#include "ISurface.h"
#include "lxOs.h"

class FFMPEGPlayer : public IPortingPlayer {
    pthread_t m_pthid;

    char m_uri[1024];
    int m_playerStatus;
    msgque_obj * m_msgque;
    int videoStreamIndex;
    AVFormatContext * pAVFormatCt;
    AVCodecContext * pAVCodeCt;
    AVCodec * pAVCodec;
    AVFrame * pAVframe;
    AVFrame * pAVframeYUV;
    AVPacket * pAVPacket;
    unsigned char *out_buffer;

    SwsContext * m_pSwsCt;

    msgque_obj * m_surfacemsgque;
    ISurface * m_surface;

    timer_obj * m_timer;

    BOOLTYPE initSource(void);
    status_t parseFrame(void);
    static void * ffmpeg_msgproc(void *param);
    static void timer_notify(void * param);

    public:
        FFMPEGPlayer();
        ~FFMPEGPlayer();
        status_t setDataSource(const char * url);
        status_t prepare(void);
        status_t prepareSync(void);
        status_t start(void);
        status_t stop(void);
        status_t pause(void);
        status_t seekto(int msec);
        status_t reset(void);
        status_t setSurface(ISurface * surface);
};

#endif