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
static AVStream *video_st, *audio_st;
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

//https://github.com/shanquanqiang/SqqFinalRecord/blob/master/jni/com_example_sqqfinalrecord_FfmpegHelper.c

int init_audio();

int init_video();

/*
 * Class:     com_itdog_recordvideo_NativeBridge
 * Method:    init
 * Signature: ([B)I
 */
JNIEXPORT jint JNICALL Java_com_itdog_recordvideo_NativeBridge_init
        (JNIEnv *env, jclass thiz, jbyteArray fileName) {
    LOGI("init started...");

    file_dir = (char *) (env->GetByteArrayElements(fileName, 0));

    yuv_width = width;
    yuv_height = height;
    y_length = width * height;
    uv_length = width * height / 4;

    av_register_all();
    p_bit_stream_ctx = av_bitstream_filter_init("aac_adtstoasc");

    // 初始化输出格式上下文
    avformat_alloc_output_context2(&p_ofmt_ctx, NULL, "flv", file_dir);
    if (init_video() != 0) {
        return -1;
    }

    if (init_audio() != 0) {
        return -1;
    }

    // open output URL, set before avformat_write_header for muxing
    if (avio_open(&p_ofmt_ctx->pb, file_dir, AVIO_FLAG_READ_WRITE) < 0) {
        LOGE("failed to open output file.\n");
        return -1;
    }

    avformat_write_header(p_ofmt_ctx, NULL);

    start_time = av_gettime();

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
        return -1;
    }

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

    AVDictionary *param = 0;
    av_dict_set(&param, "preset", "veryfast", 0);
    av_dict_set(&param, "tune", "zerolatency", 0);

    if (avcodec_open2(p_codec_ctx, p_codec, &param)) {
        LOGE("failed to open video encoder.");
        return -1;
    }

    video_st = avformat_new_stream(p_ofmt_ctx, p_codec);
    if (video_st == NULL) {
        return -1;
    }

    video_st->time_base.num = 1;
    video_st->time_base.den = 30;
    video_st->codec = p_codec_ctx;

    return 0;
}

int init_audio() {

    p_codec_a = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (p_codec_a == NULL) {
        LOGE("can't find audio encoder!");
        return -1;
    }

    p_codec_ctx_a = avcodec_alloc_context3(p_codec_a);
    p_codec_ctx_a->channels = 2;
    p_codec_ctx_a->channel_layout = av_get_default_channel_layout(p_codec_ctx_a->channels);
    p_codec_ctx_a->sample_fmt = AV_SAMPLE_FMT_S16;
    p_codec_ctx_a->bit_rate = 64000;
    p_codec_ctx_a->time_base.num = 1;
    p_codec_ctx_a->time_base.den = p_codec_ctx_a->sample_rate;
    p_codec_ctx_a->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    if (p_ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
        p_codec_ctx_a->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(p_codec_ctx_a, p_codec_a, NULL) < 0) {
        LOGE("failed to open audio encoder.");
        return -1;
    }

    audio_st = avformat_new_stream(p_ofmt_ctx, p_codec_a);
    if (audio_st == NULL) {
        return -1;
    }

    audio_st->time_base.num = 1;
    audio_st->time_base.den = p_codec_ctx_a->sample_rate;
    audio_st->codec = p_codec_ctx_a;

    return 0;
}


