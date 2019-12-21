package com.example.rtmppushdemo

interface OnChangeListener {
    fun onSizeChange(width: Int, height: Int)
}


interface OnPreviewListener {
    fun onPreviewFrame(ydata:ByteArray,yLen:Int)
}