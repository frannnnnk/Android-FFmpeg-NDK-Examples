package com.itdog.android_ffmpeg_videoplayer;

/**
 * Created by Administrator on 2017/5/16 0016.
 */

public class VideoPlayer {

    public static native int play(Object surface);

    static {
        System.loadLibrary("videoplayer");
    }

}
