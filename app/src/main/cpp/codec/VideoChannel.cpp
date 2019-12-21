//
// Created by 7invensun on 2019-12-10.
//
//#include <x264.h>
#include <cstdio>
#include "VideoChannel.h"

VideoChannel::VideoChannel() {
    pthread_mutex_init(&mutex, 0);
}

int a = 0;
int b = 0;

VideoChannel::~VideoChannel() {
    pthread_mutex_destroy(&mutex);
    if (videoCodec) {
        x264_encoder_close(videoCodec);
        videoCodec = 0;
    }
    if (pic_in) {
        x264_picture_clean(pic_in);
        DELETE(pic_in);
    }
}

void VideoChannel::setVideoCallBack(VideoCallback videocallback) {
    this->videoCallback = videocallback;
}

void VideoChannel::setVideoEncInfo(int width, int height, int fps, int bitrate) {
    pthread_mutex_lock(&mutex);

    mWidth = width;
    mHeight = height;
    mFps = fps;
    mBitrate = bitrate;

    ySize = mWidth * mHeight;
    uvSize = ySize / 4;

    if (videoCodec) {
        x264_encoder_close(videoCodec);
        videoCodec = 0;
    }

    if (pic_in) {
        x264_picture_clean(pic_in);
        DELETE(pic_in);
    }

    x264_param_t param;

    x264_param_default_preset(&param, "ultrafast", "zerolatency");
    //设置idc ，值越大编码数据越大，解码也就越清晰
    param.i_level_idc = 52;
    //指定输入I420
    param.i_csp = X264_CSP_I420;

    param.i_width = width;

    param.i_height = height;
    //0：表示编码无b帧
    param.i_bframe = 0;
    //平衡
    param.rc.i_rc_method = X264_RC_ABR;

    param.rc.i_bitrate = bitrate / 1000;

    param.rc.i_vbv_max_bitrate = bitrate / 1000 * 1.2;

    param.rc.i_vbv_buffer_size = bitrate / 1000;

    param.i_fps_num = fps;

    param.i_fps_den = 1;

    param.i_timebase_den = param.i_fps_num;
    param.i_timebase_num = param.i_fps_den;

    param.b_vfr_input = 0;

    param.i_keyint_max = fps * 2;

    param.b_repeat_headers = 1;

    param.i_threads = 1;

    x264_param_apply_profile(&param, "baseline");

    videoCodec = x264_encoder_open(&param);

    pic_in = new x264_picture_t;

    x264_picture_alloc(pic_in, X264_CSP_I420, width, height);
    pthread_mutex_unlock(&mutex);


}

void VideoChannel::encodeData(int8_t *ydata, int ylen) {
    pthread_mutex_lock(&mutex);
    LOGE("encodeData");

    if (b == 0) {
        FILE *stream;
        if ((stream = fopen("sdcard/h264", "wb")) == NULL) {
            LOGE("Cannot open output file.\n");
        }
        fwrite(ydata, mWidth * mHeight*3/2, 1, stream);
        fclose(stream);
        b = 1;
    }


    memcpy(pic_in->img.plane[0], ydata, mWidth * mHeight);
    memcpy(pic_in->img.plane[1], ydata + mWidth * mHeight, mWidth * mHeight / 4);
    memcpy(pic_in->img.plane[2], ydata + mWidth * mHeight * 5 / 4, mWidth * mHeight / 4);


    x264_nal_t *pp_nal;
    int pi_nal;
    x264_picture_t pic_out;
    x264_encoder_encode(videoCodec, &pp_nal, &pi_nal, pic_in, &pic_out);

    int sps_len;
    int pps_len;
    uint8_t sps[100];
    uint8_t pps[100];

    for (int i = 0; i < pi_nal; ++i) {
        if (pp_nal[i].i_type == NAL_SPS) {
            sps_len = pp_nal[i].i_payload - 4;
            memcpy(sps, pp_nal[i].p_payload + 4, sps_len);
        } else if (pp_nal[i].i_type == NAL_PPS) {
            pps_len = pp_nal[i].i_payload - 4;
            memcpy(pps, pp_nal[i].p_payload + 4, pps_len);
            sendSPSPPS(sps, sps_len, pps, pps_len);

        } else {
            sendH264(pp_nal[i].i_type, pp_nal[i].p_payload, pp_nal->i_payload);
        }
    }


    pthread_mutex_unlock(&mutex);
}

void VideoChannel::sendSPSPPS(uint8_t *sps, int sps_len, uint8_t *pps, int pps_len) {
    int body_size = 13 + sps_len + 3 + pps_len;
    RTMPPacket *packet = new RTMPPacket();
    RTMPPacket_Alloc(packet, body_size);

    int i = 0;
    packet->m_body[i++] = 0x17;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x01;
    packet->m_body[i++] = sps[1];
    packet->m_body[i++] = sps[2];
    packet->m_body[i++] = sps[3];
    packet->m_body[i++] = 0xFF;
    packet->m_body[i++] = 0xE1;
    packet->m_body[i++] = (sps_len >> 8) & 0xff;
    packet->m_body[i++] = (sps_len) & 0xff;
    memcpy(&packet->m_body[i], sps, sps_len);
    i += sps_len;
    packet->m_body[i++] = 0x01;
    packet->m_body[i++] = (pps_len >> 8) & 0xff;
    packet->m_body[i++] = (pps_len) & 0xff;
    memcpy(&packet->m_body[i], pps, pps_len);


    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = body_size;
    packet->m_nChannel = 10;
    packet->m_nTimeStamp = 0;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;




//    if (a == 0) {
//
//
//
//        FILE *stream;
//        if ((stream = fopen("sdcard/sps", "wb")) == NULL) {
//            LOGE("Cannot open output file.\n");
//        }
//        fwrite(packet->m_body, body_size, 1, stream);
//        a = 1;
//    }


    videoCallback(packet);

}

void VideoChannel::sendH264(int type, uint8_t *h264, int len) {
    if (h264[2] == 0x00) {
        len -= 4;
        h264 += 4;
    } else {
        len -= 3;
        h264 += 3;
    }
    int bodySize = 9 + len;

    RTMPPacket *packet = new RTMPPacket;
    //
    RTMPPacket_Alloc(packet, bodySize);

    packet->m_body[0] = 0x27;
    if (type == NAL_SLICE_IDR) {
        packet->m_body[0] = 0x17;
    }
    packet->m_body[1] = 0x01;

    packet->m_body[2] = 0x00;

    packet->m_body[3] = 0x00;

    packet->m_body[4] = 0x00;

    packet->m_body[5] = (len >> 24) & 0xff;
    packet->m_body[6] = (len >> 16) & 0xff;
    packet->m_body[7] = (len >> 8) & 0xff;
    packet->m_body[8] = (len) & 0xff;

    memcpy(&packet->m_body[9], h264, len);

    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = bodySize;
    packet->m_nChannel = 0x10;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
//    if (b == 0) {
//
//
//        FILE *stream;
//        if ((stream = fopen("sdcard/ih264", "wb")) == NULL) {
//            LOGE("Cannot open output file.\n");
//        }
//        fwrite(packet->m_body, bodySize, 1, stream);
//    }

    videoCallback(packet);


};