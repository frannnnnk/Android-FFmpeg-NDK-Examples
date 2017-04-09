//
// Created by Administrator on 2017/4/8 0008.
//

#ifndef ANDROID_FFMPEG_SAMPLES_MASTER_H264YUV_FRAME_H
#define ANDROID_FFMPEG_SAMPLES_MASTER_H264YUV_FRAME_H

#include <stdint.h>

typedef struct H264FrameDef {

    uint32_t length;
    uint8_t *dataBuffer;

} H264FrameDef;


typedef struct H264YUV_Frame {

    uint32_t width;
    uint32_t height;

    H264FrameDef luma;
    H264FrameDef chromaB;
    H264FrameDef chromaR;

} H264YUV_Frame;


#endif //ANDROID_FFMPEG_SAMPLES_MASTER_H264YUV_FRAME_H
