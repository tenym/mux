/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "audio_codec.h"

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

void *ST_AudioCodecOpen(ST_AudioCodecAttr_t *pstCodecAttr)
{
    AVCodec *       pstCodec;
    AVCodecContext *pstCodecCtx;

#if 0
    if (pstCodecAttr->eCodecId != AV_CODEC_ID_AAC)
    {
        DBG_ERR("Not support codec [%d]\n", pstCodecAttr->eCodecId);
        return NULL;
    }

    pstCodec = avcodec_find_encoder_by_name("libfdk_aac");
    if (!pstCodec)
    {
        DBG_ERR("Could not find encoder\n");
        return NULL;
    }
#endif
    //pstCodec = avcodec_find_decoder(AV_CODEC_ID_PCM_ALAW);
    pstCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!pstCodec)
    {
        DBG_ERR("Could not find decoder\n");
        return NULL;
    }

    pstCodecCtx = avcodec_alloc_context3(pstCodec);
    if (!pstCodecCtx)
    {
        DBG_ERR("Could not alloc an encoding context\n");
        return NULL;
    }

#if 0
    pstCodecCtx->codec_type     = AVMEDIA_TYPE_AUDIO;
    pstCodecCtx->codec_id       = pstCodecAttr->eCodecId;
    pstCodecCtx->sample_fmt     = pstCodecAttr->eSampleFmt;
    pstCodecCtx->bit_rate       = pstCodecAttr->u32BitRate;
    pstCodecCtx->sample_rate    = pstCodecAttr->u16SampleRate;
    pstCodecCtx->channel_layout = pstCodecAttr->u64ChannelLayout;
    pstCodecCtx->channels       = av_get_channel_layout_nb_channels(pstCodecCtx->channel_layout);
    pstCodecCtx->time_base.num  = 1;
    pstCodecCtx->time_base.den  = 1000000;
    pstCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
#endif
    pstCodecCtx->channels = 2;
    pstCodecCtx->channel_layout = av_get_default_channel_layout(2);
    pstCodecCtx->sample_rate = 8000;
    pstCodecCtx->sample_fmt = pstCodec->sample_fmts[0];
    pstCodecCtx->bit_rate = 24000;

    if (avcodec_open2(pstCodecCtx, pstCodec, NULL) < 0)
    {
        DBG_ERR("Could not open audio codec\n");
        avcodec_free_context(&pstCodecCtx);
        return NULL;
    }

    DBG_VERBOSE("Open codec [%p]\n", pstCodecCtx);
    return pstCodecCtx;
}

MI_S32 ST_AudioCodecClose(void *pHandler)
{
    AVCodecContext *pstCodecCtx = pHandler;
    if (pstCodecCtx)
    {
        DBG_VERBOSE("Close codec [%p]\n", pstCodecCtx);
        avcodec_close(pstCodecCtx);
        avcodec_free_context(&pstCodecCtx);
        return 0;
    }

    return -1;
}

MI_S32 ST_AudioCodecGetFrame(void *pHandler, ST_AudioCodecInput_t *pstCodecInput, ST_AudioCodecOutput_t *pstCodecOutput)
{
    AVCodecContext *pstCodecCtx = pHandler;
    AVFrame         stAvFrame   = {0};
    AVPacket *      pstAvPkt;
    MI_S32          s32Ret;

    if (pstCodecCtx && pstCodecInput && pstCodecOutput)
    {
        stAvFrame.data[0]        = pstCodecInput->pvInBuf;
        stAvFrame.linesize[0]    = pstCodecInput->u32Len;
        stAvFrame.extended_data  = stAvFrame.data;
        stAvFrame.pts            = pstCodecInput->u64Pts;
        stAvFrame.format         = pstCodecCtx->sample_fmt;
        stAvFrame.channel_layout = pstCodecCtx->channel_layout;
        stAvFrame.sample_rate    = pstCodecCtx->sample_rate;
        stAvFrame.nb_samples     = pstCodecInput->u32Samples;
        s32Ret                   = avcodec_send_frame(pstCodecCtx, &stAvFrame);
        if (s32Ret < 0)
        {
            DBG_ERR("Send fail\n");
            return -1;
        }

        pstAvPkt = av_packet_alloc();
        s32Ret   = avcodec_receive_packet(pstCodecCtx, pstAvPkt);
        if (s32Ret < 0)
        {
            DBG_ERR("Recv fail\n");
            av_packet_free(&pstAvPkt);
            return -1;
        }

        pstCodecOutput->pvOutBuf = pstAvPkt->data;
        pstCodecOutput->u32Len   = pstAvPkt->size;
        pstCodecOutput->u64Pts   = pstCodecInput->u64Pts;
        pstCodecOutput->pvPriv   = pstAvPkt;
        // DBG_VERBOSE("Snd [%llu] Get [%p] [%p] [%d] [%llu]\n", pstCodecInput->u64Pts, pstAvPkt, pstAvPkt->data,
        //             pstAvPkt->size, pstAvPkt->pts);
        return 0;
    }

    return -1;
}

MI_S32 ST_AudioCodecReleaseFrame(ST_AudioCodecOutput_t *pstCodecOutput)
{
    if (pstCodecOutput)
    {
        AVPacket *pstAvPkt = pstCodecOutput->pvPriv;
        av_packet_free(&pstAvPkt);
        return 0;
    }

    return -1;
}

MI_S32 ST_AudioCodecGetParam(void *pHandler, AVCodecParameters *pCodecParam)
{
    AVCodecContext *pstCodecCtx = pHandler;
    DBG_VERBOSE("Get codec [%p] info, param [%p]\n", pstCodecCtx, pCodecParam);
    if (pstCodecCtx)
    {
        if (avcodec_parameters_from_context(pCodecParam, pstCodecCtx) < 0)
        {
            DBG_ERR("Could not get param\n");
            return -1;
        }
        return 0;
    }

    return -1;
}
