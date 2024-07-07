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

#ifndef _AUDIO_CODEC_H_
#define _AUDIO_CODEC_H_

#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "libavcodec/avcodec.h"
#include "common.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct ST_AudioCodecAttr_s
{
    enum AVCodecID      eCodecId;
    enum AVSampleFormat eSampleFmt;
    MI_U32              u32BitRate;
    MI_U16              u16SampleRate;
    MI_U64              u64ChannelLayout;
} ST_AudioCodecAttr_t;

typedef struct ST_AudioCodecInput_s
{
    void * pvInBuf;
    MI_U32 u32Len;
    MI_U64 u64Pts;
    MI_U32 u32Samples;
} ST_AudioCodecInput_t;

typedef struct ST_AudioCodecOutput_s
{
    void * pvOutBuf;
    MI_U32 u32Len;
    MI_U64 u64Pts;
    void * pvPriv;
} ST_AudioCodecOutput_t;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

void * ST_AudioCodecOpen(ST_AudioCodecAttr_t *pstCodecAttr);
MI_S32 ST_AudioCodecClose(void *pHandler);
MI_S32 ST_AudioCodecGetParam(void *pHandler, AVCodecParameters *pCodecParam);
MI_S32 ST_AudioCodecGetFrame(void *pHandler, ST_AudioCodecInput_t *pstCodecInput,
                             ST_AudioCodecOutput_t *pstCodecOutput);
MI_S32 ST_AudioCodecReleaseFrame(ST_AudioCodecOutput_t *pstCodecOutput);

#ifdef __cplusplus 
}
#endif
#endif
