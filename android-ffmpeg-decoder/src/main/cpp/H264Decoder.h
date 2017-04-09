//
// Created by Administrator on 2017/4/8 0008.
//

#ifndef ANDROID_FFMPEG_SAMPLES_MASTER_H264DECODER_H
#define ANDROID_FFMPEG_SAMPLES_MASTER_H264DECODER_H

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>

#include "H264YUV_Frame.h"

typedef void (*H264Decoder_CallBack)(int code, H264YUV_Frame * yuv_frame);

#define SUCCESS (0)

#define ERR_FIND_DECODER (-1)
#define ERR_ALLOC_DECODER_CTX (-2)
#define ERR_OPEN_DECODER (-3)
#define ERR_ALLOC_FRAME (-4)
#define ERR_ALLOC_BUFFER (-5)

class H264Decoder {

private:
    AVCodec *pCodec;
    AVCodecContext *pCodecCtx;
    AVFrame *pVideoFrame;
    AVPacket avPacket;

    H264Decoder_CallBack callBack;

    void copyDecodeFrame(uint8_t * src, uint8_t * dst, int lineSize, int width, int height);

public:

    H264Decoder();

    ~H264Decoder();

    int init();

    int decodeH264Frames(uint8_t *inputBuffer, int len);

    void setDecodeCallBack(H264Decoder_CallBack callBack);
};


#endif //ANDROID_FFMPEG_SAMPLES_MASTER_H264DECODER_H
