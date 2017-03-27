package com.itdog.recordvideo;

/**
 * Created by tianbei on 2017/3/27.
 */

public class NativeBridge {

    public static native int init(byte[] fileName);

    public static native int start(byte[] yuvData);

    public static native int startAudio(byte[] audioData, int size);

    public static native int flush();

    public static native int close();

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
