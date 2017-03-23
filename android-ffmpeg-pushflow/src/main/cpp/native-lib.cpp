
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
#include <libavformat/avformat.h>
}

#include <jni.h>
#include <android/log.h>

#include "com_itdog_pushflow_NativeBridge.h"

#define LOGI(format, ...) __android_log_print(ANDROID_LOG_INFO, "(>_<)", format, ##__VA_ARGS__)
#define LOGE(format, ...) __android_log_print(ANDROID_LOG_ERROR, "(>_<)", format, ##__VA_ARGS__)


void CustomLog(void *ptr, int level, const char *fmt, va_list va) {

    FILE *fp = fopen("/storage/emulated/0/av_log.txt", "a+");
    if (fp != NULL) {

        vfprintf(fp, fmt, va);
        fflush(fp);
        fclose(fp);

    }

}

/*
 * Class:     com_itdog_pushflow_NativeBridge
 * Method:    pushFlow
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_itdog_pushflow_NativeBridge_pushFlow
        (JNIEnv *env, jclass thiz, jstring inputUrl, jstring outputUrl) {
    LOGI("function %s called.", __FUNCTION__);

    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmtContext = NULL, *ofmtContext = NULL;
    AVPacket avPacket;
    AVStream *inStream;
    AVStream *outStream;
    int videoIndex = -1;
    int ret, i;
    char inputFilePath[1024] = {0};
    char outputUrlPath[1024] = {0};
    int64_t startTime;
    int frameIndex = 0;

    snprintf(inputFilePath, sizeof(inputFilePath), "%s", env->GetStringUTFChars(inputUrl, NULL));
    snprintf(outputUrlPath, sizeof(outputUrlPath), "%s", env->GetStringUTFChars(outputUrl, NULL));

    av_log_set_callback(CustomLog);
    av_register_all();
    avformat_network_init();

    if ((ret = avformat_open_input(&ifmtContext, inputFilePath, 0, 0)) < 0) {
        LOGE("couldn't open file %s.", inputFilePath);
        goto END;
    }

    if ((ret = avformat_find_stream_info(ifmtContext, 0)) < 0) {
        LOGE("failed to retrieve input stream information\n");
        goto END;
    }


    for (i = 0; i < ifmtContext->nb_streams; i++) {
        if (ifmtContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            break;
        }
    }

    avformat_alloc_output_context2(&ofmtContext, NULL, "flv", outputUrlPath);
    if (ofmtContext == NULL) {
        LOGE("couldn't create output context!");
        ret = AVERROR_UNKNOWN;
        goto END;
    }

    ofmt = ofmtContext->oformat;
    for (i = 0; i < ifmtContext->nb_streams; i++) {

        inStream = ifmtContext->streams[i];
        outStream = avformat_new_stream(ofmtContext, inStream->codec->codec);
        if (!outStream) {
            LOGE("failed to allocate output stream!\n");
            ret = AVERROR_UNKNOWN;
            goto END;
        }

        ret = avcodec_copy_context(outStream->codec, inStream->codec);
        if (ret < 0) {
            LOGE("failed to copy context from input to output stream codec context.\n");
            goto END;
        }

        outStream->codec->codec_tag = 0;
        if (ofmtContext->oformat->flags & AVFMT_GLOBALHEADER) {
            outStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }

    }

    if(!(ofmt->flags & AVFMT_NOFILE)) {

        ret = avio_open(&ofmtContext->pb, outputUrlPath, AVIO_FLAG_WRITE);
        if(ret < 0) {

            char error[128] = { 0 };
            av_strerror(ret, error, 128);
            LOGE("could not open output URL %s.", outputUrlPath);
            goto END;

        }

    }

    // Write file header
    ret = avformat_write_header(ofmtContext, NULL);
    if(ret < 0) {
        LOGE("error occured when opening output url.");
        goto END;
    }

    startTime = av_gettime();
    while(1) {

        ret = av_read_frame(ifmtContext, &avPacket);
        if(ret < 0) {
            break;
        }

        if(avPacket.pts == AV_NOPTS_VALUE) {

            AVRational timeBase1 = ifmtContext->streams[videoIndex]->time_base;
            int64_t calcDuration = (double) AV_TIME_BASE /
                    av_q2d(ifmtContext->streams[videoIndex]->r_frame_rate);
            avPacket.pts = (double) (frameIndex * calcDuration) / (double)(av_q2d(timeBase1) * AV_TIME_BASE);
            avPacket.dts = avPacket.pts;
            avPacket.duration = (double) calcDuration / (double) ( av_q2d(timeBase1) * AV_TIME_BASE );

        }

        if(avPacket.stream_index == videoIndex) {

            AVRational timeBase = ifmtContext->streams[videoIndex]->time_base;
            AVRational timeBaseQ = {1, AV_TIME_BASE};
            int64_t  ptsTime = av_rescale_q(avPacket.dts, timeBase, timeBaseQ);
            int64_t  nowTime = av_gettime() - startTime;
            if(ptsTime > nowTime) {
                av_usleep(ptsTime - nowTime);
            }

        }

        inStream = ifmtContext->streams[avPacket.stream_index];
        outStream = ofmtContext->streams[avPacket.stream_index];
        avPacket.pts = av_rescale_q_rnd(avPacket.pts, inStream->time_base, outStream->time_base,
                                        (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        avPacket.dts = av_rescale_q_rnd(avPacket.dts, inStream->time_base, outStream->time_base,
                                        (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        avPacket.duration = av_rescale_q(avPacket.duration, inStream->time_base, outStream->time_base);
        avPacket.pos = -1;

        if(avPacket.stream_index == videoIndex) {
            LOGI("Send %8d video frames to outpt URL.\n", frameIndex);
            frameIndex++;
        }

        ret = av_interleaved_write_frame(ofmtContext, &avPacket);
        if(ret < 0) {

            LOGE("Error muxing packet!.\n");
            break;

        }

        av_free_packet(&avPacket);
    }

    av_write_trailer(ofmtContext);

END:
    avformat_close_input(&ifmtContext);
    if (ofmtContext && !(ofmtContext->flags & AVFMT_NOFILE)) {
        avio_close(ofmtContext->pb);
    }
    avformat_free_context(ofmtContext);

    if (ret < 0 && ret != AVFMT_NOFILE && ret != AVERROR_EOF) {
        char error[128] = { 0 };
        av_strerror(ret, error, 128);
        LOGE("error occured, ret = %d. reason = %s\n", ret, error);
        return -1;
    }

    return 0;
}
