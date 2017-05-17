package com.itdog.android_ffmpeg_videoplayer;

/**
 * Created by Administrator on 2017/5/16 0016.
 */

public class VideoPlayer {

    public static native int play(Object surface);

    static {
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("postproc-54");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
        System.loadLibrary("videoplayer");
    }

}
