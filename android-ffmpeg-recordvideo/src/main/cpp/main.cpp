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
static int width = 600, height = 800;
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

int flush_encoder_a();

int flush_encoder();

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
    avcodec_register_all();
    p_bit_stream_ctx = av_bitstream_filter_init("aac_adtstoasc");

    // 初始化输出格式上下文
    avformat_alloc_output_context2(&p_ofmt_ctx, NULL, "flv", file_dir);

    if (init_audio() != 0) {
        return -1;
    }

    if (init_video() != 0) {
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
        (JNIEnv *env, jclass thiz, jbyteArray yuv) {

    int ret;
    int enc_got_frame;
    int i = 0;

    p_frame_yuv = av_frame_alloc();
    int picture_size = avpicture_get_size(AV_PIX_FMT_YUV420P, p_codec_ctx->width,
                                          p_codec_ctx->height);
    uint8_t *out_buffer = (uint8_t *) av_malloc(picture_size);
    avpicture_fill((AVPicture *) p_frame_yuv, out_buffer, AV_PIX_FMT_YUV420P, p_codec_ctx->width,
                   p_codec_ctx->height);

    jbyte *in = (jbyte *) env->GetByteArrayElements(yuv, 0);
    memcpy(p_frame_yuv->data[0], in, y_length);
    env->ReleaseByteArrayElements(yuv, in, 0);

    for (i = 0; i < uv_length; i++) {
        *(p_frame_yuv->data[2] + i) = *(in + y_length + i * 2);
        *(p_frame_yuv->data[1] + i) = *(in + y_length + i * 2 + 1);
    }

    p_frame_yuv->format = AV_PIX_FMT_YUV420P;
    p_frame_yuv->width = yuv_width;
    p_frame_yuv->height = yuv_height;

    env_pkt.data = NULL;
    env_pkt.size = 0;
    av_init_packet(&env_pkt);

    ret = avcodec_encode_video2(p_codec_ctx, &env_pkt, p_frame_yuv, &enc_got_frame);
    av_frame_free(&p_frame_yuv);

    if (enc_got_frame == 1) {
        LOGI("Succeed to encode video frame: %5d\tsize:%5d\n", frame_cnt, env_pkt.size);
        frame_cnt++;
        env_pkt.stream_index = video_st->index;

        // Write PTS
        AVRational time_base = p_ofmt_ctx->streams[0]->time_base;

        // 表示一帧30s
        AVRational r_frame_rate1 = {30, 1};
        AVRational time_base_q = AV_TIME_BASE_Q;

        // Duration between 2 frames (us) 两帧之间的时间间隔，这里的单位是微妙
        int64_t calc_duration = (double) (AV_TIME_BASE) * (1 / av_q2d(r_frame_rate1));
        int64_t timett = av_gettime();
        int64_t now_time = timett - start_time;
        vid_pts = now_time;
        env_pkt.pts = av_rescale_q(now_time, time_base_q, time_base);
        env_pkt.dts = env_pkt.pts;
        env_pkt.duration = av_rescale_q(calc_duration, time_base_q, time_base);
        env_pkt.pos = -1;

        ret = av_interleaved_write_frame(p_ofmt_ctx, &env_pkt);
        av_free_packet(&env_pkt);
    }

    return 0;
}

/*
 * Class:     com_itdog_recordvideo_NativeBridge
 * Method:    startAudio
 * Signature: ([BI)I
 */
JNIEXPORT jint JNICALL Java_com_itdog_recordvideo_NativeBridge_startAudio
        (JNIEnv *env, jclass thiz, jbyteArray audio_data, jint data_size) {
    int ret;
    int enc_got_frame;
    int i = 0;

    p_frame = av_frame_alloc();
    p_frame->nb_samples = p_codec_ctx_a->frame_size;
    frame_size = p_frame->nb_samples;
    p_frame->format = p_codec_ctx_a->sample_fmt;
    p_frame->channel_layout = p_codec_ctx_a->channel_layout;
    p_frame->sample_rate = p_codec_ctx_a->sample_rate;

    int size = av_samples_get_buffer_size(NULL, p_codec_ctx_a->channels,
                                          p_codec_ctx_a->frame_size, p_codec_ctx_a->sample_fmt, 1);
    uint8_t *frame_buf = (uint8_t *) av_malloc(size * 4);
    avcodec_fill_audio_frame(p_frame, p_codec_ctx_a->channel_layout, p_codec_ctx_a->sample_fmt,
                             (const uint8_t *) frame_buf, size, 1);
    jbyte *in = (jbyte *) env->GetByteArrayElements(audio_data, 0);
    if (memcpy(frame_buf, in, data_size) <= 0) {
        LOGE("Failed to read raw data!");
        return -1;
    }
    p_frame->data[0] = frame_buf;
    env->ReleaseByteArrayElements(audio_data, in, 0);

    env_pkt_a.data = NULL;
    env_pkt_a.size = 0;
    av_init_packet(&env_pkt_a);
    nb_samples += p_frame->nb_samples;

    ret = avcodec_encode_audio2(p_codec_ctx_a, &env_pkt_a, p_frame, &enc_got_frame);
    av_frame_free(&p_frame);
    if (enc_got_frame == 1) {
        LOGI("Succeed to encode audio frame: %5d\tsize:%5d\tbufsize:%5d\n", frame_cnt_a,
             env_pkt_a.size, size);
        frame_cnt_a++;
        env_pkt_a.stream_index = audio_st->index;
        av_bitstream_filter_filter(p_bit_stream_ctx, p_codec_ctx_a, NULL, &env_pkt_a.data,
                                   &env_pkt_a.size, env_pkt_a.data, env_pkt_a.size, 0);

        // Write PTS
        AVRational time_base = p_ofmt_ctx->streams[audio_st->index]->time_base;

        // 表示一帧30秒
        AVRational r_frame_rate1 = {p_codec_ctx_a->sample_rate, 1};
        AVRational time_base_q = AV_TIME_BASE_Q;

        // Duration between 2 frames
        int64_t calc_duration = (double) (AV_TIME_BASE) * (1 / av_q2d(r_frame_rate1));

        // Parameters
        int64_t timett = av_gettime();
        int64_t now_time = timett - start_time;
        env_pkt_a.pts = av_rescale_q(now_time, time_base_q, time_base);
        env_pkt_a.dts = env_pkt_a.pts;
        env_pkt_a.duration = av_rescale_q(calc_duration, time_base_q, time_base);
        env_pkt_a.pos = -1;

        // 延时
        ret = av_interleaved_write_frame(p_ofmt_ctx, &env_pkt_a);
        av_free_packet(&env_pkt_a);
    }

    return 0;
}

/*
 * Class:     com_itdog_recordvideo_NativeBridge
 * Method:    flush
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_itdog_recordvideo_NativeBridge_flush
        (JNIEnv *env, jclass thiz) {

    flush_encoder();
    flush_encoder_a();

    LOGI("Flush end!\n");

    // Write file trailer
    av_write_trailer(p_ofmt_ctx);
    return 0;
}

/*
 * Class:     com_itdog_recordvideo_NativeBridge
 * Method:    close
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_itdog_recordvideo_NativeBridge_close
        (JNIEnv *env, jclass thiz) {

    if (video_st) {
        avcodec_close(video_st->codec);
    }

    if (audio_st) {
        avcodec_close(audio_st->codec);
    }

    avio_close(p_ofmt_ctx->pb);
    avformat_free_context(p_ofmt_ctx);

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

    p_codec_a = avcodec_find_encoder();
    if (p_codec_a == NULL) {
        LOGE("can't find audio encoder!");
        return -1;
    }

    p_codec_ctx_a = avcodec_alloc_context3(p_codec_a);
    p_codec_ctx_a->codec_id = AV_CODEC_ID_AAC;
    p_codec_ctx_a->sample_fmt = AV_SAMPLE_FMT_S16;
    p_codec_ctx_a->sample_rate = 44100; // 44100 8000
    p_codec_ctx_a->channels = 2;
    p_codec_ctx_a->channel_layout = av_get_default_channel_layout(
            p_codec_ctx_a->channels);;
    p_codec_ctx_a->bit_rate = 64000;

    p_codec_ctx_a->time_base.num = 1;
    p_codec_ctx_a->time_base.den = p_codec_ctx_a->sample_rate;
    p_codec_ctx_a->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    if (p_ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
        p_codec_ctx_a->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    int ret;
    if ((ret = avcodec_open2(p_codec_ctx_a, p_codec_a, NULL)) < 0) {
        char error[128] = { 0 };
        av_strerror(ret, error, 128);
        LOGE("error occured, ret = %d. reason = %s\n", ret, error);
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

int flush_encoder() {
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if (!(p_ofmt_ctx->streams[0]->codec->codec->capabilities & CODEC_CAP_DELAY)) {
        return 0;
    }

    while (1) {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_video2(p_ofmt_ctx->streams[0]->codec, &enc_pkt, NULL, &got_frame);
        if (ret < 0) {
            break;
        }

        if (!got_frame) {
            ret = 0;
            break;
        }

        LOGI("Flush encoder: succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
        frame_cnt++;

        // Write PTS
        AVRational time_base = p_ofmt_ctx->streams[0]->time_base;
        AVRational r_frame_rate1 = {30, 1};
        AVRational time_base_q = {1, AV_TIME_BASE};

        // Duration between 2 frames
        int64_t calc_duration = (double) (AV_TIME_BASE) * (1 / av_q2d(r_frame_rate1));

        // Parameters
        int64_t timett = av_gettime();
        int64_t now_time = timett - start_time;
        enc_pkt.pts = av_rescale_q(now_time, time_base_q, time_base);
        enc_pkt.dts = enc_pkt.pts;
        enc_pkt.duration = av_rescale_q(calc_duration, time_base_q, time_base);
        enc_pkt.pos = -1;

        // mux encoded frame
        ret = av_interleaved_write_frame(p_ofmt_ctx, &env_pkt);
        if (ret < 0) {
            break;
        }
    }
}

int flush_encoder_a() {
    int ret;
    int got_frame;
    AVPacket enc_pkt_a;
    if (!(p_ofmt_ctx->streams[audio_st->index]->codec->codec->capabilities * CODEC_CAP_DELAY)) {
        return 0;
    }

    while (1) {
        enc_pkt_a.data = NULL;
        enc_pkt_a.size = 0;
        av_init_packet(&enc_pkt_a);
        ret = avcodec_encode_audio2(p_ofmt_ctx->streams[audio_st->index]->codec, &enc_pkt_a, NULL,
                                    &got_frame);
        av_frame_free(NULL);
        if (ret < 0) {
            break;
        }

        if (!got_frame) {
            ret = 0;
            break;
        }

        LOGE("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt_a.size);
        nb_samples += frame_size;
        av_bitstream_filter_filter(p_bit_stream_ctx, p_ofmt_ctx->streams[audio_st->index]->codec,
                                   NULL,
                                   &enc_pkt_a.data, &enc_pkt_a.size, enc_pkt_a.data, enc_pkt_a.size,
                                   0);

        // Write PTS
        AVRational time_base = p_ofmt_ctx->streams[audio_st->index]->time_base;

        // 表示一帧30秒
        AVRational r_frame_rate1 = {p_codec_ctx_a->sample_rate, 1};
        AVRational time_base_q = AV_TIME_BASE_Q;

        // Duration between 2 frames
        int64_t calc_duration = (double) (AV_TIME_BASE) * (1 / av_q2d(r_frame_rate1));

        // Parameters
        int64_t timett = av_gettime();
        int64_t now_time = timett - start_time;
        enc_pkt_a.pts = av_rescale_q(now_time, time_base_q, time_base);
        enc_pkt_a.dts = enc_pkt_a.pts;
        enc_pkt_a.duration = av_rescale_q(calc_duration, time_base_q, time_base);
        enc_pkt_a.pos = -1;

        ret = av_interleaved_write_frame(p_ofmt_ctx, &enc_pkt_a);
        if (ret < 0) {
            break;
        }

        return 1;
    }
}



