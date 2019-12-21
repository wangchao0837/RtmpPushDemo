//
// Created by 7invensun on 2019-12-11.
//

#include <cstring>
#include <pthread.h>
#include "AudioChannel.h"

AudioChannel::AudioChannel() {
    pthread_mutex_init(&mutex, 0);
}

AudioChannel::~AudioChannel() {
    pthread_mutex_destroy(&mutex);
}

void AudioChannel::encodeData(signed char *data) {
    pthread_mutex_lock(&mutex);
    int bytelen = faacEncEncode(audioCodec, reinterpret_cast<int32_t *>(data), inputBuffer, buffer,
                                maxOutBuffer);
    if (bytelen > 0) {
        int bodySize = 2 + bytelen;
        RTMPPacket *packet = new RTMPPacket;
        RTMPPacket_Alloc(packet, bodySize);
        //双声道
        packet->m_body[0] = 0xAF;
        if (channels == 1) {
            packet->m_body[0] = 0xAE;
        }
        //编码出的声音 都是 0x01
        packet->m_body[1] = 0x01;
        //图片数据
        memcpy(&packet->m_body[2], buffer, bytelen);

        packet->m_hasAbsTimestamp = 0;
        packet->m_nBodySize = bodySize;
        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
        packet->m_nChannel = 0x11;
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
        audioCallback(packet);
    }

    pthread_mutex_unlock(&mutex);

}

void AudioChannel::setAudioCallBack(AudioCallback audiocallback) {
    this->audioCallback = audiocallback;
}

int AudioChannel::getInputSamples() {
    return inputBuffer;
}

RTMPPacket *AudioChannel::getAudioTag() {
    pthread_mutex_lock(&mutex);
    unsigned char *buf;
    unsigned long len;

    faacEncGetDecoderSpecificInfo(audioCodec, &buf, &len);
    int bodySize = 2 + len;

    RTMPPacket *packet = new RTMPPacket();

    RTMPPacket_Alloc(packet, bodySize);

    packet->m_body[0] = 0xAF;
    if (channels == 1) {
        packet->m_body[0] = 0xAE;
    }
    packet->m_body[1] = 0x00;

    //图片数据
    memcpy(&packet->m_body[2], buf, len);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodySize;
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nChannel = 0x11;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    pthread_mutex_unlock(&mutex);

    return packet;

}

void AudioChannel::setAudioEncInfo(int sampleRate, int channels) {

    pthread_mutex_lock(&mutex);
    LOGE("AAA setAudioEncInfo");
    this->sampleRate = sampleRate;
    this->channels = channels;

    audioCodec = faacEncOpen(sampleRate, channels, &inputBuffer, &maxOutBuffer);

    faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(audioCodec);

    config->mpegVersion = MPEG4;

    config->inputFormat = FAAC_INPUT_16BIT;

    config->aacObjectType = LOW;

    config->inputFormat = FAAC_INPUT_16BIT;

    config->outputFormat = 0;

    faacEncSetConfiguration(audioCodec, config);

    buffer = new unsigned char[maxOutBuffer];
    pthread_mutex_unlock(&mutex);


}
