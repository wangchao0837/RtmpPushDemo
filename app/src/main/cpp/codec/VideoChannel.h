//
// Created by 7invensun on 2019-12-10.
//

#ifndef RTMPPUSHDEMO_VIDEOCHANNEL_H
#define RTMPPUSHDEMO_VIDEOCHANNEL_H

#include <pthread.h>
#include <x264.h>
#include "../librtmp/rtmp.h"
#include "macro.h"
typedef void (*VideoCallback)(RTMPPacket *packet);

class VideoChannel {
public:
    VideoChannel();

    ~VideoChannel();

    void setVideoCallBack(VideoCallback videocallback);

    void setVideoEncInfo(int width, int height, int fps, int bitrate);

    void encodeData(
            int8_t *ydata, int ylen
    );


    void sendSPSPPS(uint8_t *sps, int sps_len, uint8_t *pps, int pps_len);

    void sendH264(int type, uint8_t *h264, int len);

public:
    pthread_mutex_t mutex;
    VideoCallback videoCallback;
    int mWidth;
    int mHeight;
    int mFps;
    int mBitrate;
    int ySize;
    int uvSize;
    x264_t *videoCodec = 0;
    x264_picture_t *pic_in = 0;


};


#endif //RTMPPUSHDEMO_VIDEOCHANNEL_H
