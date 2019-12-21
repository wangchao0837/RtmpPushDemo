package com.example.rtmppushdemo

import android.media.AudioFormat
import android.media.AudioRecord
import android.media.MediaRecorder
import android.util.Log
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors

class AudioChannel(livePusher: LivePusher) {


    val livePusher: LivePusher
    val executor: ExecutorService
    val channels: Int = 1
    val sampleRate: Int = 44100
    val inputSample: Int
    val minBufferSize: Int
    var isLive: Boolean = false
    var audioRecord: AudioRecord? = null

    init {


        this.livePusher = livePusher;

        executor = Executors.newSingleThreadExecutor()
        val channelConfig: Int
        if (channels == 1) {
            channelConfig = AudioFormat.CHANNEL_IN_MONO
        } else {
            channelConfig = AudioFormat.CHANNEL_IN_STEREO
        }

        livePusher.native_setAudioEncInfo(sampleRate, channels);


        inputSample = livePusher.getInputSamples() * 2

        minBufferSize =
            AudioRecord.getMinBufferSize(
                sampleRate,
                channelConfig,
                AudioFormat.ENCODING_PCM_16BIT
            ) * 2


        audioRecord = AudioRecord(
            MediaRecorder.AudioSource.MIC,
            sampleRate,
            channelConfig,
            AudioFormat.ENCODING_PCM_16BIT,
            if (minBufferSize > inputSample) {
                minBufferSize
            } else {
                inputSample
            }
        )

    }


    fun startLive() {
        isLive = true
        executor.submit(AudioTask())
    }

    fun release() {
        audioRecord!!.release()
    }

    fun stopLive() {
        isLive = false
    }


    inner class AudioTask : Runnable {
        override fun run() {
            audioRecord!!.startRecording()
            val bytes = ByteArray(inputSample)
            while (isLive) {
                val read = audioRecord!!.read(bytes, 0, bytes.size)
                if (read > 0) {
                    livePusher.native_pushAudio(bytes);
                }
            }
        }
    }


}