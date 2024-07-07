
#ifndef _MUXER_H
#define _MUXER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

typedef struct ST_VideoSetting_s
{
    MI_U16  u16RecW;
    MI_U16  u16RecH;
    MI_U32  u32RecBitRate;
    MI_U8   u8RecCodecType;
    MI_U16  u16SubRecW;
    MI_U16  u16SubRecH;
    MI_U32  u32SubRecBitRate;
    MI_U8   u8SubRecCodecType;
} ST_VideoSetting_t;

typedef struct ST_MuxerAudioFrame_s
{
    void * pvBuf;
    MI_U32 u32Len;
    MI_U64 u64Pts;
} ST_MuxerAudioFrame_t;

typedef struct ST_MuxerFileAttr_s
{
    MI_BOOL bUsed;
    MI_BOOL bInit;
    MI_BOOL bOpen;
    MI_U8   u8VencDev;
    MI_U8   u8VencChn;
    MI_S32  s32VencFd;
    MI_U16  u16W;
    MI_U16  u16H;
    MI_U32  u32BitRate;
    MI_U8   u8CodecType;
    void *  pvMuxerLib;
    MI_BOOL bAudioEnable;
    void *  pvAudioCodecHandler;
    MI_BOOL bFoundKey;
    char *  muxer_name;
} ST_MuxerFileAttr_t;

typedef struct ST_MuxerAttr_s
{
    MI_BOOL            bUsed;
    MI_BOOL            bCreate;
    MI_BOOL            bNeedInitPipe;
    MI_BOOL            bMuxerExit;
    MI_BOOL            bMuxerDone;
    ST_MuxerFileAttr_t stFileAttr;
} ST_MuxerAttr_t;

int mt_muxer_start(char *muxer_name);
int mt_muxer_stop(void);

#ifdef __cplusplus 
}
#endif
#endif
