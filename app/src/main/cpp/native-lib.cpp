#include <jni.h>
#include <string>
#include <x264.h>
#include "librtmp/rtmp.h"
#include "codec/VideoChannel.h"
#include "codec/safe_queue.h"
#include "codec/AudioChannel.h"


VideoChannel *videoChannel = 0;
AudioChannel *audioChannel = 0;
SafeQueue<RTMPPacket *> packets;
int isStart = 0;
pthread_t pid;
uint32_t startTime;
int readyPush = 0;

void callback(RTMPPacket *packet) {
    if (packet) {
        packet->m_nTimeStamp = RTMP_GetTime() - startTime;
        packets.push(packet);
    }
}

void releasePackets(RTMPPacket *&packet) {
    if (packet) {
        RTMPPacket_Free(packet);
        DELETE(packet);
        packet = 0;
    }
}

void *task_start(void *args) {
    char *url = static_cast<char *>(args);
    RTMP *rtmp = 0;
    do {
        rtmp = RTMP_Alloc();

        if (!rtmp) {
            LOGE("alloc rtmp失败");
            break;
        }

        RTMP_Init(rtmp);
        int ret = RTMP_SetupURL(rtmp, url);
        if (!ret) {
            LOGE("设置地址失败:%s", url);
            break;
        }

        rtmp->Link.timeout = 5;
        RTMP_EnableWrite(rtmp);

        ret = RTMP_Connect(rtmp, 0);

        if (!ret) {
            LOGE("连接服务器失败:%s", url);
            break;
        }

        ret = RTMP_ConnectStream(rtmp, 0);
        if (!ret) {
            LOGE("连接流失败");
            break;
        }
        //获取当前开始时间
        startTime = RTMP_GetTime();
        readyPush = 1;
        packets.setWork(1);

        callback(audioChannel->getAudioTag());
        RTMPPacket *packet = 0;
        //循环从queue中读取rtmppacket推送
        while (readyPush) {
            ret = packets.pop(packet);
            if (!readyPush) {
                break;
            }
            if (!ret) {
                continue;
            }

            packet->m_nInfoField2 = rtmp->m_stream_id;
            ret = RTMP_SendPacket(rtmp, packet, 1);
            releasePackets(packet);
            if (!ret) {
                LOGE("发送失败");
                break;
            }
        }
        releasePackets(packet);
    } while (0);

    isStart = 0;
    readyPush = 0;
    packets.setWork(0);
    packets.clear();
    if (rtmp) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }
    DELETE (url);

    return 0;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_rtmppushdemo_RtmpPush_native_1init(JNIEnv *env, jclass instance) {

    videoChannel = new VideoChannel();
    videoChannel->setVideoCallBack(callback);

    audioChannel = new AudioChannel();
    audioChannel->setAudioCallBack(callback);
    packets.setReleaseCallback(releasePackets);

}extern "C"
JNIEXPORT void JNICALL
Java_com_example_rtmppushdemo_RtmpPush_native_1setVideoEncInfo(JNIEnv *env, jclass clazz,
                                                               jint width, jint height, jint fps,
                                                               jint bitrate) {

    if (videoChannel) {
        videoChannel->setVideoEncInfo(width, height, fps, bitrate);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_rtmppushdemo_RtmpPush_native_1start(JNIEnv *env, jclass clazz, jstring path) {
    if (isStart) {
        return;
    }

    isStart = 1;
    const char *_path = env->GetStringUTFChars(path, 0);

    char *url = new char[strlen(_path) + 1];
    strcpy(url, _path);
    pthread_create(&pid, 0, task_start, url);
    env->ReleaseStringUTFChars(path, _path);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_rtmppushdemo_RtmpPush_native_1pushVideo(JNIEnv *env, jclass clazz,
                                                         jbyteArray ydata, jint y_len) {
    if (!videoChannel || !readyPush) {
        return;
    }
    jbyte *datay = env->GetByteArrayElements(ydata, NULL);

    videoChannel->encodeData(datay, y_len);
    env->ReleaseByteArrayElements(ydata, datay, 0);



}extern "C"
JNIEXPORT void JNICALL
Java_com_example_rtmppushdemo_RtmpPush_native_1stop(JNIEnv *env, jclass clazz) {
    readyPush = 0;
    isStart = 0;
    packets.setWork(0);
    packets.clear();

}extern "C"
JNIEXPORT void JNICALL
Java_com_example_rtmppushdemo_RtmpPush_native_1release(JNIEnv *env, jclass clazz) {
    DELETE(videoChannel);
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_rtmppushdemo_RtmpPush_native_1setAudioEncInfo(JNIEnv *env, jclass thiz,
                                                               jint sample_rate_in_hz,
                                                               jint channels) {
    if (!audioChannel)
        return;
    audioChannel->setAudioEncInfo(sample_rate_in_hz, channels);

}extern "C"
JNIEXPORT jint JNICALL
Java_com_example_rtmppushdemo_RtmpPush_getInputSamples(JNIEnv *env, jclass thiz) {
    if (!audioChannel) {
        return 0;
    }

    return audioChannel->getInputSamples();

}extern "C"
JNIEXPORT void JNICALL
Java_com_example_rtmppushdemo_RtmpPush_native_1pushAudio(JNIEnv *env, jclass thiz,
                                                         jbyteArray data) {

    if (!audioChannel) {
        return;
    }

    jbyte *audio_data = env->GetByteArrayElements(data, 0);

    audioChannel->encodeData(audio_data);

    env->ReleaseByteArrayElements(data, audio_data, 0);


}