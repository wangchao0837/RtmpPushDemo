package com.example.rtmppushdemo

import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    var width: Int = 1280
    var height: Int = 720
    lateinit var livePusher: LivePusher
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        livePusher = LivePusher.Builder()
            .setWidth(width)
            .setHeight(height)
            .setBitrate(width * height * 3)
            .setFps(30)
            .setDefaultCameraId(0)
            .context(this)
            .setRtmpPath("rtmp://10.10.1.139/myapp/mystream")
            .build()

        livePusher.setTextureView(textureView);


    }

    fun onStartPush(view: View) {
        livePusher.startLive()
    }

    fun onStopPush(view: View) {
        livePusher.stopLive()
    }

}
