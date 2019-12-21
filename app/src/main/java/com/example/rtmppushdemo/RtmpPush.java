package com.example.rtmppushdemo;

public class RtmpPush {
    static {
        System.loadLibrary("rtmpPush");
    }

    /**
     * 创建x264编码类和faac编码类
     */
    public static native void native_init();

    /**
     * 连接rtmp服务器，推流开始
     * @param path
     */
    public static native void native_start(String path);

    /**
     * 初始化x264配置
     * @param width
     * @param height
     * @param fps
     * @param bitrate
     */
    public static native void native_setVideoEncInfo(int width, int height, int fps, int bitrate);

    /**
     * yuv输入编码，推流
     * @param ydata
     * @param yLen
     */
    public static native void native_pushVideo(
            byte[] ydata, int yLen
    );

    public static native void native_stop();

    public static native void native_release();

    /**
     * 初始化faac配置
     * @param sampleRateInHz
     * @param channels
     */
    public static native void native_setAudioEncInfo(int sampleRateInHz, int channels);

    /**
     * 获取faac 输入buffer的大小，配置audioRecord的输入buffer大小
     * @return
     */
    public static native int getInputSamples();

    /**
     * pcm编码aac，推流
     * @param data
     */
    public static native void native_pushAudio(byte[] data);

}
