package com.example.rtmppushdemo

import android.content.Context

class VideoChannel(livePusher: LivePusher) : OnPreviewListener,
    OnChangeListener {

    var isLive: Boolean = false
    var livePusher: LivePusher
    var width: Int
    var height: Int
    var bitrate: Int
    var fps: Int
    var cameraId: Int
    var context: Context
    lateinit var cameraHelper: CameraHelper

    init {
        this.livePusher = livePusher;
        this.cameraId = livePusher.builder.cameraId;
        this.fps = livePusher.builder.fps
        this.bitrate = livePusher.builder.bitrate
        this.height = livePusher.builder.height
        this.width = livePusher.builder.width
        this.context = livePusher.builder.context
        cameraHelper = CameraHelper(context, width, height, cameraId);
    }

    fun setTextureView(textureView: AutoFitTextureView?) {
        cameraHelper.setTextureView(textureView!!)
        cameraHelper.setChangeSizeListener(this);
        cameraHelper.setOnPreViewFrame(this);
    }

    fun closeCamera() {
        cameraHelper.closeCamera();
    }


    override fun onSizeChange(width: Int, height: Int) {
        livePusher.native_setVideoEncInfo(width, height, fps, bitrate)
    }

    override fun onPreviewFrame(
        ydata: ByteArray, yLen: Int
    ) {
        if (isLive) {
            livePusher.native_pushVideo(ydata, yLen)
        }
    }

    fun startLive() {
        cameraHelper.startLive();
        isLive = true
    }


    fun stopLive() {
        isLive = false
        cameraHelper.stopLive();
    }

}