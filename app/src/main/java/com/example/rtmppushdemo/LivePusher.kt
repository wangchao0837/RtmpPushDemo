package com.example.rtmppushdemo

import android.content.Context

class LivePusher {

    companion object {
        init {
            System.loadLibrary("rtmpPush")
        }
    }

    var isLive: Boolean = false


    public lateinit var builder: Builder
    private lateinit var videoChannel: VideoChannel
    private lateinit var audioChannel: AudioChannel


    public constructor(builder: Builder) {
        this.builder = builder
        native_init()
        videoChannel = VideoChannel(this)
        audioChannel = AudioChannel(this)
    }


    fun setTextureView(textureView: AutoFitTextureView?) {
        videoChannel.setTextureView(textureView);
    }

    fun startLive() {
        native_start(builder.path);
        isLive = true
        videoChannel.startLive()
        audioChannel.startLive();
    }


    fun stopLive() {
        isLive = false;
        native_stop()
    }

    fun release() {
        native_release()
        if (videoChannel != null)
            videoChannel.closeCamera()
        if (audioChannel != null)
            audioChannel.release();
    }

    fun native_init() {
        RtmpPush.native_init()
    }

    fun native_start(path: String) {
        RtmpPush.native_start(path)
    }

    fun native_setVideoEncInfo(width: Int, height: Int, fps: Int, bitrate: Int) {
        RtmpPush.native_setVideoEncInfo(width, height, fps, bitrate)
    }

    fun native_pushVideo(
        ydata: ByteArray, yLen: Int
    ) {
        RtmpPush.native_pushVideo(ydata, yLen)
    }

    fun native_stop() {
        videoChannel.stopLive()
        audioChannel.stopLive()
        RtmpPush.native_stop()
    }

    fun native_release() {
        RtmpPush.native_release()
    }

    fun native_setAudioEncInfo(sampleRateInHz: Int, channels: Int) {
        RtmpPush.native_setAudioEncInfo(sampleRateInHz, channels)
    }

    fun getInputSamples(): Int {
        return RtmpPush.getInputSamples()
    }

    fun native_pushAudio(data: ByteArray) {
        return RtmpPush.native_pushAudio(data)
    }

    class Builder {

        var width: Int = 0
        var height: Int = 0
        var bitrate: Int = 0
        var fps: Int = 0
        var cameraId: Int = 0
        lateinit var path: String
        lateinit var context: Context

        fun setWidth(width: Int): Builder {
            this.width = width;
            return this
        }

        fun setHeight(height: Int): Builder {
            this.height = height;
            return this
        }

        fun setBitrate(bitrate: Int): Builder {
            this.bitrate = bitrate;
            return this
        }

        fun setFps(fps: Int): Builder {
            this.fps = fps;
            return this
        }

        fun setDefaultCameraId(cameraId: Int): Builder {
            this.cameraId = cameraId;
            return this
        }

        fun context(context: Context): Builder {
            this.context = context;
            return this
        }

        fun setRtmpPath(path: String): Builder {
            this.path = path
            return this;
        }

        fun build(): LivePusher {
            return LivePusher(this);
        }


    }
}



