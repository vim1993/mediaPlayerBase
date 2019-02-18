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
    char *out_buffer;

    msgque_obj * m_surfacemsgque;

    BOOLTYPE initSource(void);

    static void * ffmpeg_start(void *param);

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
        status_t setSurface(msgque_obj * surface);
};

#endif