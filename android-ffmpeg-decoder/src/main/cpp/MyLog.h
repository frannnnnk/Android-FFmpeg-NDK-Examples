//
// Created by Administrator on 2017/4/8 0008.
//

#ifndef ANDROID_FFMPEG_SAMPLES_MASTER_MYLOG_H
#define ANDROID_FFMPEG_SAMPLES_MASTER_MYLOG_H


#include <jni.h>
#include <android/log.h>

#define LOG_TAG "itdog"

#define LOGI(format, ...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, format "\n", ##__VA_ARGS__)
#define LOGE(format, ...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, format "\n", ##__VA_ARGS__)

#endif //ANDROID_FFMPEG_SAMPLES_MASTER_MYLOG_H
