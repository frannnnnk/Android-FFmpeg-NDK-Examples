package com.itdog.ffmpegdemo;

/**
 * Created by tianbei on 2017/3/21.
 */

public class NativeBridge {

    public static native String urlprotocolinfo();

    public static native String avformatinfo();

    public static native String avcodecinfo();

    public static native String avfilterinfo();

    public static native String configurationinfo();

    // Used to load the 'native-lib' library on application startup.
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
