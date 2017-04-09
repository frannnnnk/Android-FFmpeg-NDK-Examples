//
// Created by Administrator on 2017/4/8 0008.
//

#include "H264Decoder.h"

#include "MyLog.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

H264Decoder::H264Decoder() {

    pCodec = NULL;
    pCodecCtx = NULL;
    pVideoFrame = NULL;
    memset(&avPacket, 0, sizeof(AVPacket));

    av_register_all();
    avcodec_register_all();

}

H264Decoder::~H264Decoder() {

    if (pVideoFrame != NULL) {
        av_frame_free(&pVideoFrame);
        pVideoFrame = NULL;
    }

    if (pCodecCtx != NULL) {
        avcodec_close(pCodecCtx);
        avcodec_free_context(&pCodecCtx);
        pCodecCtx = NULL;
    }

}

int H264Decoder::init() {
    int retCode = SUCCESS;
    pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (NULL == pCodec) {
        LOGE("H264Decoder: cant find h264 decoder.");
        retCode = ERR_FIND_DECODER;
        goto LABEL_ERR_FIND_DECODER;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (NULL == pCodecCtx) {
        LOGE("H264Decoder: alloc h264 codec context failure.");
        retCode = ERR_ALLOC_DECODER_CTX;
        goto LABEL_ERR_ALLOC_DECODER_CTX;
    }

    int ret = avcodec_open2(pCodecCtx, pCodec, NULL);
    if (ret < 0) {
        LOGE("H264Decoder: open h264 decoder failure.");
        retCode = ERR_OPEN_DECODER;
        goto LABEL_ERR_OPEN_DECODER;
    }

    pVideoFrame = av_frame_alloc();
    if (pVideoFrame == NULL) {
        LOGE("H264Decoder: alloc av frame failure.");
        retCode = ERR_ALLOC_FRAME;
        goto LABEL_ERR_ALLOC_FRAME;
    }

    LABEL_ERR_ALLOC_FRAME:
    avcodec_close(pCodecCtx);
    LABEL_ERR_OPEN_DECODER:
    avcodec_free_context(&pCodecCtx);
    LABEL_ERR_ALLOC_DECODER_CTX:
    LABEL_ERR_FIND_DECODER:
    return retCode;
}

int H264Decoder::decodeH264Frames(uint8_t *inputBuffer, int len) {

    int gotPicPtr = 0;
    H264YUV_Frame h264Frame;

    memset(&h264Frame, 0, sizeof(H264YUV_Frame));
    av_init_packet(&avPacket);
    avPacket.data = (uint8_t *) inputBuffer;
    avPacket.size = len;

    int result = avcodec_decode_video2(pCodecCtx, pVideoFrame, &gotPicPtr, &avPacket);

    if (gotPicPtr) {
        uint32_t lumaLength =
                (pCodecCtx->height) * (MIN(pVideoFrame->linesize[0], pCodecCtx->width));
        uint32_t chromBLength =
                (pCodecCtx->height / 2) * (MIN(pVideoFrame->linesize[1], pCodecCtx->width / 2));
        uint32_t chromRLength =
                (pCodecCtx->height / 2) * (MIN(pVideoFrame->linesize[2], pCodecCtx->width / 2));

        h264Frame.luma.length = lumaLength;
        h264Frame.chromaB.length = chromBLength;
        h264Frame.chromaR.length = chromRLength;

        h264Frame.luma.dataBuffer = (uint8_t *) malloc(lumaLength);
        if (h264Frame.luma.dataBuffer == NULL) {
            if (this->callBack != NULL) {
                callBack(ERR_ALLOC_BUFFER, NULL);
            }
            goto LABEL_ERR_ALLOC_LUMA;
        }

        h264Frame.chromaB.dataBuffer = (uint8_t *) malloc(chromBLength);
        if (h264Frame.chromaB.dataBuffer == NULL) {
            if (this->callBack != NULL) {
                callBack(ERR_ALLOC_BUFFER, NULL);
            }
            goto LABEL_ERR_ALLOC_CHROMAB;
        }

        h264Frame.chromaR.dataBuffer = (uint8_t *) malloc(chromRLength);
        if (h264Frame.chromaR.dataBuffer == NULL) {
            if (this->callBack != NULL) {
                callBack(ERR_ALLOC_BUFFER, NULL);
            }
            goto LABEL_ERR_ALLOC_CHROMAR;
        }

        copyDecodeFrame(pVideoFrame->data[0], h264Frame.luma.dataBuffer, pVideoFrame->linesize[0],
                        pCodecCtx->width, pCodecCtx->height);
        copyDecodeFrame(pVideoFrame->data[1], h264Frame.chromaB.dataBuffer,
                        pVideoFrame->linesize[1],
                        pCodecCtx->width / 2, pCodecCtx->height / 2);
        copyDecodeFrame(pVideoFrame->data[2], h264Frame.chromaR.dataBuffer,
                        pVideoFrame->linesize[2],
                        pCodecCtx->width / 2, pCodecCtx->height / 2);

        h264Frame.width = pCodecCtx->width;
        h264Frame.height = pCodecCtx->height;

        if (this->callBack != NULL) {
            callBack(SUCCESS, &h264Frame);
        }

        free(h264Frame.chromaR.dataBuffer);
        LABEL_ERR_ALLOC_CHROMAR:
        free(h264Frame.chromaB.dataBuffer);
        LABEL_ERR_ALLOC_CHROMAB:
        free(h264Frame.luma.dataBuffer);
        LABEL_ERR_ALLOC_LUMA:;
    }

    av_free_packet(&avPacket);

    return 0;
}

void H264Decoder::setDecodeCallBack(H264Decoder_CallBack callBack) {
    this->callBack = callBack;
}

void H264Decoder::copyDecodeFrame(uint8_t *src, uint8_t *dst, int lineSize, int width, int height) {
    width = MIN(lineSize, width);
    for (int i = 0; i < height; i++) {
        memcpy(dst, src, width);
        dst += width;
        src += lineSize;
    }
}