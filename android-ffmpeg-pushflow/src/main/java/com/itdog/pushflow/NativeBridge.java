package com.itdog.pushflow;

/**
 * Created by Administrator on 2017/3/23 0023.
 */

public class NativeBridge {

    public static native void pushFlow(String inputUrl, String outputUrl);

    static {
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("postproc-54");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
        System.loadLibrary("native-lib");
    }
}
