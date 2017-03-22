#include <jni.h>
#include <string>

#include "com_itdog_ffmpegdemo_NativeBridge.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
}


extern "C"
jstring
Java_com_itdog_ffmpegdemo_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


/*
 * Class:     com_itdog_ffmpegdemo_NativeBridge
 * Method:    urlprotocolinfo
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_itdog_ffmpegdemo_NativeBridge_urlprotocolinfo
        (JNIEnv *env, jclass thiz) {
    char info[40000] = {0};
    av_register_all();

    struct URLProtocol *pup = NULL;
    //Input
    struct URLProtocol **p_temp = &pup;
    avio_enum_protocols((void **) p_temp, 0);
    while ((*p_temp) != NULL) {
        sprintf(info, "%s[In ][%10s]\n", info, avio_enum_protocols((void **) p_temp, 0));
    }
    pup = NULL;
    //Output
    avio_enum_protocols((void **) p_temp, 1);
    while ((*p_temp) != NULL) {
        sprintf(info, "%s[Out][%10s]\n", info, avio_enum_protocols((void **) p_temp, 1));
    }

    //LOGE("%s", info);
    return env->NewStringUTF(info);
}

/*
 * Class:     com_itdog_ffmpegdemo_NativeBridge
 * Method:    avformatinfo
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_itdog_ffmpegdemo_NativeBridge_avformatinfo
        (JNIEnv *env, jclass thiz) {
    char info[4000] = {0};

    av_register_all();
    AVInputFormat *if_temp = av_iformat_next(NULL);
    AVOutputFormat *of_temp = av_oformat_next(NULL);

    while (if_temp != NULL) {
        snprintf(info, sizeof(info), "%s[In ][%10s]\n", info, if_temp->name);
        if_temp = if_temp->next;
    }

    while (of_temp != NULL) {
        snprintf(info, sizeof(info), "%s[Out ][%10s]\n", info, of_temp->name);
        of_temp = of_temp->next;
    }

    return env->NewStringUTF(info);
}

/*
 * Class:     com_itdog_ffmpegdemo_NativeBridge
 * Method:    avcodecinfo
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_itdog_ffmpegdemo_NativeBridge_avcodecinfo
        (JNIEnv *env, jclass thiz) {
    char info[4000] = {0};

    av_register_all();
    AVCodec *c_temp = av_codec_next(NULL);
    while (c_temp != NULL) {

        if (c_temp->decode != NULL) {
            snprintf(info, sizeof(info), "%s[Dec]", info);
        } else {
            snprintf(info, sizeof(info), "%s[Enc]", info);
        }

        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                snprintf(info, sizeof(info), "%s[VIDEO]", info);
                break;
            case AVMEDIA_TYPE_AUDIO:
                snprintf(info, sizeof(info), "%s[AUDIO]", info);
                break;
            default:
                snprintf(info, sizeof(info), "%s[OTHER]", info);
                break;
        }

        snprintf(info, sizeof(info), "%s[%10s]\n", info, c_temp->name);

        c_temp = c_temp->next;
    }

    return env->NewStringUTF(info);
}

/*
 * Class:     com_itdog_ffmpegdemo_NativeBridge
 * Method:    avfilterinfo
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_itdog_ffmpegdemo_NativeBridge_avfilterinfo
        (JNIEnv *env, jclass thiz) {
    char info[4000] = {0};

    avfilter_register_all();
    AVFilter * f_temp = (AVFilter *)avfilter_next(NULL);

    while(f_temp != NULL) {
        snprintf(info, sizeof(info), "%s[%10s]\n", info, f_temp->name);
        f_temp = f_temp -> next;
    }

    return env->NewStringUTF(info);
}

/*
 * Class:     com_itdog_ffmpegdemo_NativeBridge
 * Method:    configurationinfo
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_itdog_ffmpegdemo_NativeBridge_configurationinfo
        (JNIEnv *env, jclass thiz){

    char info[10000] = {0};

    av_register_all();

    snprintf(info, sizeof(info), "%s\n", avcodec_configuration());

    return env->NewStringUTF(info);
}

