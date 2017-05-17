//
// Created by Administrator on 2017/5/16 0016.
//

#include <jni.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>

#include "com_itdog_android_ffmpeg_videoplayer_VideoPlayer.h"


#define LOGE(format, ...) __android_log_print(ANDROID_LOG_ERROR, "(>_<)", format, ##__VA_ARGS__);
#define LOGI(format, ...) __android_log_print(ANDROID_LOG_INFO, "(>_<)", format, ##__VA_ARGS__);


/*
 * Class:     com_itdog_android_ffmpeg_videoplayer_VideoPlayer
 * Method:    play
 * Signature: (Ljava/lang/Object;)I
 */
JNIEXPORT jint JNICALL Java_com_itdog_android_1ffmpeg_1videoplayer_VideoPlayer_play
        (JNIEnv *env, jclass clazz, jobject surface_holder) {

    // sdcard中的视频地址
    const char *filePath = "/sdcard/videos/sample.flv";

    av_register_all();
    AVFormatContext *pFormatContext = avformat_alloc_context();

    // Open video file
    if (avformat_open_input(&pFormatContext, filePath, NULL, NULL) != 0) {
        LOGI("Couldn't open file:%s\n", filePath);
        return -1;
    }

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
        LOGE("Couldn't find stream information.");
        return -1;
    }

    // Find first video stream
    int videoStream = -1, i;
    for (i = 0; i < pFormatContext->nb_streams; i++) {
        if (pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO &&
            videoStream < 0) {
            videoStream = i;
            break;
        }
    }
    if (videoStream == -1) {
        LOGE("Didn't find a video stream.");
        return -1;
    }

    // Get a pointer to the codec context for the video stream
    AVCodecContext *pCodecCtx = pFormatContext->streams[videoStream]->codec;

    // Find a decoder for the video stream
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        LOGE("Codec not found.");
        return -1;
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGE("Couldn't open codec");
        return -1;
    }

    // 获取Native window
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface_holder);

    // 获取视频宽高
    int videoWidth = pCodecCtx->width;
    int videoHeight = pCodecCtx->height;

    // 获取native window 的buffer大小，可自动拉伸
    ANativeWindow_setBuffersGeometry(nativeWindow, videoWidth, videoHeight,
                                     WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGE("Couldn't open codec.");
        return -1;
    }

    // Alloc a video frame
    AVFrame *pFrame = av_frame_alloc();

    // 用于渲染
    AVFrame *pFrameRGBA = av_frame_alloc();
    if (pFrameRGBA == NULL || pFrame == NULL) {
        LOGE("Could not alloc video frame.");
        return -1;
    }

    // Determine required buffer size and allocate buffer
    // buffer 中的数据是用来渲染的，并且格式为RGBA
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA,
                                            pCodecCtx->width, pCodecCtx->height, 1);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
                         pCodecCtx->width, pCodecCtx->height, 1);

    // 由于解码出来的帧格式不是RGBA的，在渲染之前需要进行格式转换
    struct SwsContext *swsContext = sws_getContext(pCodecCtx->width,
                                                   pCodecCtx->height,
                                                   pCodecCtx->pix_fmt,
                                                   pCodecCtx->width,
                                                   pCodecCtx->height,
                                                   AV_PIX_FMT_RGBA,
                                                   SWS_BILINEAR,
                                                   NULL,
                                                   NULL,
                                                   NULL);
    int frameFinished;
    AVPacket avPacket;
    while (av_read_frame(pFormatContext, &avPacket) >= 0) {

        // if this a packet from the video stream
        if (avPacket.stream_index == videoStream) {
            // decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &avPacket);
            if (frameFinished) {

                // lock native window buffer
                ANativeWindow_lock(nativeWindow, &windowBuffer, 0);

                // 格式转换
                sws_scale(swsContext, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height, pFrameRGBA->data,
                          pFrameRGBA->linesize);

                // 获取stride
                uint8_t *dst = (uint8_t *)windowBuffer.bits;
                int dstStride = windowBuffer.stride * 4;
                uint8_t *src = (uint8_t *) (pFrameRGBA->data[0]);
                int srcStride = pFrameRGBA->linesize[0];

                // 由于window的stride和帧的stride不同，因此需要逐行复制
                int h;
                for (h = 0; h < videoHeight; h++) {
                    memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
                }

                ANativeWindow_unlockAndPost(nativeWindow);
            }
        }
        av_packet_unref(&avPacket);
    }

    av_free(buffer);
    av_free(pFrameRGBA);
    av_free(pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatContext);
    return 0;
}