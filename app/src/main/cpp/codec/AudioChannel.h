//
// Created by 7invensun on 2019-12-11.
//

#ifndef RTMPPUSHDEMO_AUDIOCHANNEL_H
#define RTMPPUSHDEMO_AUDIOCHANNEL_H


#include "../librtmp/rtmp.h"
#include "macro.h"
#include "../include/faac.h"

typedef void (*AudioCallback)(RTMPPacket *packet);

class AudioChannel {
public:
    AudioChannel();

    ~AudioChannel();

    void setAudioCallBack(AudioCallback videocallback);

    void setAudioEncInfo(int sampleRate,int channels);

    void encodeData(
            signed char *data
    );

    int getInputSamples();

    RTMPPacket *getAudioTag();


public:
    faacEncHandle audioCodec = 0;
    AudioCallback audioCallback;
    int sampleRate;
    int channels = 1;
    unsigned char *buffer = 0;
    unsigned long inputBuffer;
    unsigned long maxOutBuffer;
    pthread_mutex_t mutex;

};


#endif //RTMPPUSHDEMO_AUDIOCHANNEL_H
