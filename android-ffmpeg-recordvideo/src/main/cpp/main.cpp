extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/log.h>

#include "com_itdog_recordvideo_NativeBridge.h"
}

#include <stdio.h>
#include <stdlib.h>
#include <jni.h>
#include <android/log.h>

#define LOGI(format, ...) \
    __android_log_print(ANDROID_LOG_INFO, "(>_<)", format, ##__VA_ARGS__)
#define LOGE(format, ...) \
    __android_log_print(ANDROID_LOG_ERROR, "(>_<)", format, ##__VA_ARGS__)


static AVBitStreamFilterContext *p_bit_stream_ctx;
static AVFormatContext *p_ofmt_ctx;
static AVCodec *p_codec, *p_codec_a;
static AVCodecContext *p_codec_ctx, *p_codec_ctx_a;
static AVStream *p_stream, *p_stream_a;
static AVPacket env_pkt, env_pkt_a;
static AVFrame *p_frame_yuv, *p_frame;

static char *file_dir;
static int width, height;
static int frame_cnt = 0;
static int frame_cnt_a = 0;
static int nb_samples = 0;
static int yuv_width;
static int yuv_height;
static int y_length;
static int uv_length;
static int64_t start_time;
static int aud_pts;
static int vid_pts;
static int frame_size;


#define ERR_VIDEO_ENCODER_NOT_FOUND (-1)

/*
 * Class:     com_itdog_recordvideo_NativeBridge
 * Method:    init
 * Signature: ([B)I
 */
JNIEXPORT jint JNICALL Java_com_itdog_recordvideo_NativeBridge_init
        (JNIEnv *env, jclass thiz, jbyteArray fileName) {
    LOGI("init started...");


    return 0;
}

/*
 * Class:     com_itdog_recordvideo_NativeBridge
 * Method:    start
 * Signature: ([B)I
 */
JNIEXPORT jint JNICALL Java_com_itdog_recordvideo_NativeBridge_start
        (JNIEnv *env, jclass thiz, jbyteArray yuvData) {

    return 0;
}

/*
 * Class:     com_itdog_recordvideo_NativeBridge
 * Method:    startAudio
 * Signature: ([BI)I
 */
JNIEXPORT jint JNICALL Java_com_itdog_recordvideo_NativeBridge_startAudio
        (JNIEnv *env, jclass thiz, jbyteArray audioData, jint size) {

    return 0;
}

/*
 * Class:     com_itdog_recordvideo_NativeBridge
 * Method:    flush
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_itdog_recordvideo_NativeBridge_flush
        (JNIEnv *env, jclass thiz) {

    return 0;
}

/*
 * Class:     com_itdog_recordvideo_NativeBridge
 * Method:    close
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_itdog_recordvideo_NativeBridge_close
        (JNIEnv *env, jclass thiz) {

    return 0;
}

int init_video() {

    p_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (p_codec == NULL) {
        LOGE("can't find video encoder.");
        return ERR_VIDEO_ENCODER_NOT_FOUND;
    }

    //https://github.com/shanquanqiang/SqqFinalRecord/blob/master/jni/com_example_sqqfinalrecord_FfmpegHelper.c
    p_codec_ctx = avcodec_alloc_context3(p_codec);
    p_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    p_codec_ctx->width = width;
    p_codec_ctx->height = height;
    p_codec_ctx->time_base.num = 1;
    p_codec_ctx->time_base.den = 30;
    p_codec_ctx->bit_rate = 800000;
    p_codec_ctx->gop_size = 250;
    if (p_ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
        p_codec_ctx->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }
    p_codec_ctx->qmin = 10;
    p_codec_ctx->qmax = 51;
    p_codec_ctx->max_b_frames = 3;

    AVDictionary * param = 0;
    av_dict_set(&param, "preset", "veryfast", 0);



}



