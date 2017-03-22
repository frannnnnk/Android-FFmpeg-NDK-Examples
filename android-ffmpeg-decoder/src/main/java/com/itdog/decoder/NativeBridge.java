package com.itdog.decoder;

/**
 * Created by tianbei on 2017/3/22.
 */

public class NativeBridge {

    public static native int getVersion();

    public static native int decode(String inputUrl, String outputUrl);

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
