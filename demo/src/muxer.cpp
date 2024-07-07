/// @file muxer.cpp
/// @brief 
/// @author tengym <tengym@gospell.com>
/// 0.01_0
/// @date 2024-06-30
#include <sys/queue.h>
#include "audio_codec.h"
#include "mRing.h"
#include "muxer.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavformat/avformat.h"
#include "libavcodec/codec_par.h"
#include "libavutil/intreadwrite.h"
#ifdef __cplusplus 
}
#endif

#define HEVC_NAL_UNIT_IDR (19)
#define AVC_NAL_UNIT_IDR  (5)

#define CODEC_TYPE_H265 (0)
#define CODEC_TYPE_H264 (1)

typedef struct
{
    uint8_t bKeyFrame;
    uint8_t *pu8Addr;
    uint32_t u32Len;
    uint64_t u64Pts;
} MT_VENC_Stream_t;

typedef struct ST_MuxerLib_s
{
    AVFormatContext *  pstAVFmtCtx;
    AVStream *         pstAudioStream;
    AVStream *         pstVideoStream;
    uint8_t *          pu8VideoExtraData;
    AVCodecParameters *pstAudioCodecParam;
    MI_U16             u16W;
    MI_U16             u16H;
    MI_U32             u32BitRate;
    MI_U8              u8CodecType;
    MI_U64             u64Pts;
    MI_U64             u64VideoBasePts;
    MI_U64             u64AudioBasePts;
    MI_U32             u32VideoFrameCnt;
    MI_U32             u32AudioFrameCnt;
    MI_BOOL            bVideoDone;
    MI_BOOL            bAudioDone;
} ST_MuxerLib_t;

typedef struct ST_MuxerFrame_s
{
    void *    pvBuf;
    MI_U32    u32BufSize;
    MI_U64    u64Pts;
    MI_BOOL   bKeyFrame;
    AVStream *pstStream;
} ST_MuxerFrame_t;

static ST_MuxerAttr_t gstMuxerAttr = {0};
static int g_key_flag = 0;

static MI_S32 _MuxerLibOpen(char *pszFileName, ST_MuxerLib_t *pstMuxLib)
{
    AVFormatContext *pstAVFmtCtx    = NULL;
    AVDictionary *   stFormatOpts   = NULL;
    AVStream *       pstAudioStream = NULL;
    AVStream *       pstVideoStream = NULL;

    /* allocate the output media context */
    avformat_alloc_output_context2(&pstAVFmtCtx, NULL, NULL, (char *)pszFileName);
    if (!pstAVFmtCtx)
    {
        DBG_ERR("Could not deduce output format from file extension.\n");
        return -1;
    }

    DBG_INFO("FmtCtx = %p\n", pstAVFmtCtx);

    /* video stream idx */
    pstVideoStream = avformat_new_stream(pstAVFmtCtx, NULL);
    if (!pstVideoStream)
    {
        DBG_ERR("Failed allocating video stream\n");
        return -1;
    }

    pstVideoStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    pstVideoStream->codecpar->codec_id = AV_CODEC_ID_H264;
        //pstMuxLib->u8CodecType == CODEC_TYPE_H265 ? AV_CODEC_ID_HEVC : AV_CODEC_ID_H264;
    //pstVideoStream->codecpar->codec_tag = pstMuxLib->u8CodecType == CODEC_TYPE_H265 ? MKTAG('h', 'v', 'c', '1') : 0;
    pstVideoStream->codecpar->codec_tag = 0;
    pstVideoStream->codecpar->format    = AV_PIX_FMT_YUV420P;
    pstVideoStream->codecpar->bit_rate  = 1000000;
    pstVideoStream->codecpar->width     = 1280;
    pstVideoStream->codecpar->height    = 720;
#if 0
    pstVideoStream->codecpar->bit_rate  = pstMuxLib->u32BitRate;
    pstVideoStream->codecpar->width     = pstMuxLib->u16W;
    pstVideoStream->codecpar->height    = pstMuxLib->u16H;
#endif
    pstVideoStream->id                  = 0;
    pstVideoStream->time_base.num       = 1;
    pstVideoStream->time_base.den       = 90000;
    av_dict_set(&pstVideoStream->metadata, "handler_name", "SStar Video", 0);

    if (pstMuxLib->pstAudioCodecParam)
    {
        /* audio stream idx */
        pstAudioStream = avformat_new_stream(pstAVFmtCtx, NULL);
        if (!pstAudioStream)
        {
            DBG_ERR("Failed allocating audio stream\n");
            return -1;
        }

        /* copy the stream parameters to the muxer */
        memcpy(pstAudioStream->codecpar, pstMuxLib->pstAudioCodecParam, sizeof(AVCodecParameters));
        pstAudioStream->id            = 1;
        pstAudioStream->time_base.num = 1;
        pstAudioStream->time_base.den = 16000;
        free(pstMuxLib->pstAudioCodecParam);
    }

    av_dump_format(pstAVFmtCtx, 0, pszFileName, 1);

    /* open the output file, if needed */
    if (avio_open2(&pstAVFmtCtx->pb, pszFileName, AVIO_FLAG_WRITE, NULL, NULL) < 0)
    {
        DBG_ERR("Could not open '%s'\n", pszFileName);
        return -1;
    }

    /* Write the stream header, if any. */
    av_dict_set_int(&stFormatOpts, "use_editlist", 0, 0);

    //av_dict_set(&stFormatOpts,"movflags","faststart",0);

    if (avformat_write_header(pstAVFmtCtx, &stFormatOpts) < 0)
    {
        DBG_ERR("Error occurred when opening output file\n");
        return -1;
    }

    av_dict_free(&stFormatOpts);

    pstMuxLib->pstAVFmtCtx    = pstAVFmtCtx;
    pstMuxLib->pstAudioStream = pstAudioStream;
    pstMuxLib->pstVideoStream = pstVideoStream;

    return 0;
}

static MI_S32 _MuxerSetAttr(void)
{
    ST_MuxerAttr_t *pstMuxerAttr = &gstMuxerAttr;

    ST_MuxerFileAttr_t *pstMuxerFileAttr = &pstMuxerAttr->stFileAttr;
    //if (pstMuxerFileAttr->bUsed)
    {
        pstMuxerFileAttr->pvMuxerLib = malloc(sizeof(ST_MuxerLib_t));
        if (pstMuxerFileAttr->pvMuxerLib == NULL)
        {
            DBG_ERR("FAIL\n");
            return -1;
        }
        memset(pstMuxerFileAttr->pvMuxerLib, 0, sizeof(ST_MuxerLib_t));

        //TAILQ_INIT(&pstMuxerFileAttr->audioQueue);
        //pthread_mutex_init(&pstMuxerFileAttr->audioQueueLock, NULL);
        pstMuxerFileAttr->bInit = TRUE;
    }

    return 0;
}

static MI_S32 _MuxerOpenFile(ST_MuxerAttr_t *pstMuxerAttr)
{
    ST_MuxerFileAttr_t *pstMuxerFileAttr = &pstMuxerAttr->stFileAttr;
    //if (pstMuxerFileAttr->bUsed && pstMuxerFileAttr->bInit)
    {
        char *             pszFileName = pstMuxerFileAttr->muxer_name;
        ST_MuxerLib_t *    pstMuxLib = (ST_MuxerLib_t *)pstMuxerFileAttr->pvMuxerLib;
        AVCodecParameters *pstAudioCodecParam;

        pstMuxLib->u16W        = pstMuxerFileAttr->u16W;
        pstMuxLib->u16H        = pstMuxerFileAttr->u16H;
        pstMuxLib->u32BitRate  = pstMuxerFileAttr->u32BitRate;
        pstMuxLib->u8CodecType = pstMuxerFileAttr->u8CodecType;

        if (pstMuxerFileAttr->bAudioEnable)
        {
            pstAudioCodecParam = (AVCodecParameters *)malloc(sizeof(AVCodecParameters));
            if (pstAudioCodecParam == NULL)
            {
                DBG_ERR("Malloc Fail\n");
                return -1;
            }
            memset(pstAudioCodecParam, 0, sizeof(AVCodecParameters));
            if (ST_AudioCodecGetParam(pstMuxerFileAttr->pvAudioCodecHandler, pstAudioCodecParam) < 0)
            {
                DBG_ERR("Get Audio Codec Fail\n");
                free(pstAudioCodecParam);
                return -1;
            }
            pstMuxLib->pstAudioCodecParam = pstAudioCodecParam;
        }
        else
        {
            pstMuxLib->pstAudioCodecParam = NULL;
        }

        if (_MuxerLibOpen(pszFileName, pstMuxLib) < 0)
        {
            return -1;
        }

        pstMuxerFileAttr->bOpen = TRUE;
    }

    return 0;
}

static int _ParseHevcExtraData(uint8_t *pBuffer, int size)
{
    uint8_t *p;

    if (AV_RB32(pBuffer) != 0x00000001)
    {
        return 0;
    }

    for (p = pBuffer; p < pBuffer + size - 4; p++)
    {
        if (AV_RB32(p) == 0x00000001)
        {
            if ((*(p + 4) >> 1 & 0x3F) == HEVC_NAL_UNIT_IDR)
            {
                return (p - pBuffer);
            }
            p += 4;
        }
    }

    return 0;
}

static MI_S32 _MuxerLibClose(ST_MuxerLib_t *pstMuxLib)
{
    AVFormatContext *pstAVFmtCtx = pstMuxLib->pstAVFmtCtx;

    /* Write the trailer, if any. The trailer must be written before you
     * close the CodecContexts open when you wrote the header; otherwise
     * av_write_trailer() may try to use memory that was freed on
     * av_codec_close(). */
    av_write_trailer(pstAVFmtCtx);

    /* Close the output file, if needed */
    avio_closep(&pstAVFmtCtx->pb);

    // Set NULL and free by app.
    pstMuxLib->pstVideoStream->codecpar->extradata = NULL;

    /* free the stream */
    avformat_free_context(pstAVFmtCtx);

    if (pstMuxLib->pu8VideoExtraData)
    {
        free(pstMuxLib->pu8VideoExtraData);
    }

    DBG_INFO("FmtCtx [%p] Vid Cnt [%d] Aud Cnt [%d] Vid Base [%llu] Aud Base [%llu]\n", pstAVFmtCtx,
             pstMuxLib->u32VideoFrameCnt, pstMuxLib->u32AudioFrameCnt, pstMuxLib->u64VideoBasePts,
             pstMuxLib->u64AudioBasePts);

    return 0;
}

static MI_S32 _MuxerLibWriteVideo(ST_MuxerLib_t *pstMuxLib, ST_MuxerFrame_t *pstMuxFrame)
{
    AVPacket   pkt              = {0};
    AVRational src_tb           = {1, 1000000};
    MI_U32     u32ExtraDataSize = 0;

#if 0
    if (pstMuxLib->bVideoDone)
    {
        return 0;
    }
#endif

    av_init_packet(&pkt);

    pkt.stream_index = pstMuxFrame->pstStream->id;
    pkt.data         = (uint8_t *)pstMuxFrame->pvBuf;
    pkt.size         = pstMuxFrame->u32BufSize;
    pkt.pos          = -1;

    if (pstMuxFrame->bKeyFrame)
    {
        pkt.flags |= AV_PKT_FLAG_KEY;
        if (pstMuxLib->pu8VideoExtraData == NULL)
        {
            u32ExtraDataSize             = _ParseHevcExtraData(pkt.data, pkt.size);
            pstMuxLib->pu8VideoExtraData = (uint8_t *)malloc(u32ExtraDataSize);
            if (pstMuxLib->pu8VideoExtraData == NULL)
            {
                DBG_ERR("alloc fail\n");
                return -1;
            }
            memcpy(pstMuxLib->pu8VideoExtraData, pkt.data, u32ExtraDataSize);
            pstMuxFrame->pstStream->codecpar->extradata_size = u32ExtraDataSize;
            pstMuxFrame->pstStream->codecpar->extradata      = pstMuxLib->pu8VideoExtraData;
        }
    }

    if (pstMuxLib->pu8VideoExtraData)
    {
        if (pstMuxLib->u64VideoBasePts == 0)
        {
            pstMuxLib->u64VideoBasePts = pstMuxFrame->u64Pts;
        }

        pkt.duration = 0;
        pkt.pts = pkt.dts = pstMuxFrame->u64Pts - pstMuxLib->u64VideoBasePts;

        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(&pkt, src_tb, pstMuxFrame->pstStream->time_base);
        // DBG_VERBOSE("Mux [%p] Vid [%llu]\n", pstMuxLib->pstAVFmtCtx, pkt.pts);
        if (av_write_frame(pstMuxLib->pstAVFmtCtx, &pkt) < 0)
        {
            DBG_ERR("write fail\n");
            return -1;
        }
    }

#if 0
    if (pstMuxFrame->u64Pts - pstMuxLib->u64VideoBasePts >= gu64RecLimitTime * 1000000)
    {
        DBG_VERBOSE("Mux [%p] Vid Done\n", pstMuxLib->pstAVFmtCtx);
        pstMuxLib->bVideoDone = TRUE;
    }
#endif

    pstMuxLib->u32VideoFrameCnt++;
    return 0;
}

static MI_S32 _MuxerLibWriteAudio(ST_MuxerLib_t *pstMuxLib, ST_MuxerFrame_t *pstMuxFrame)
{
    AVPacket pkt = {0};

#if 0
    if (pstMuxLib->bAudioDone)
    {
        return 0;
    }
#endif

    av_init_packet(&pkt);
    pkt.stream_index = pstMuxFrame->pstStream->id;
    pkt.data         = (uint8_t *)pstMuxFrame->pvBuf;
    pkt.size         = pstMuxFrame->u32BufSize;
    pkt.pos          = -1;

    if (pstMuxLib->u64AudioBasePts == 0)
    {
        pstMuxLib->u64AudioBasePts = pstMuxFrame->u64Pts;
    }

    pkt.duration = 1024;
    pkt.pts = pkt.dts = 1024 * pstMuxLib->u32AudioFrameCnt;
    // DBG_VERBOSE("Mux [%p] Aud [%llu]\n", pstMuxLib->pstAVFmtCtx, pstMuxFrame->u64Pts);
    if (av_write_frame(pstMuxLib->pstAVFmtCtx, &pkt) < 0)
    {
        DBG_ERR("write fail\n");
        return -1;
    }

#if 0
    if (pkt.pts >= gu64RecLimitTime * 16000)
    {
        DBG_VERBOSE("Mux [%p] Aud Done\n", pstMuxLib->pstAVFmtCtx);
        pstMuxLib->bAudioDone = TRUE;
    }
#endif

    pstMuxLib->u32AudioFrameCnt++;
    return 0;
}

static MI_S32 _MuxerWriteVideo(ST_MuxerFileAttr_t *pstMuxerFileAttr, MT_VENC_Stream_t *pstStream)
{
    ST_MuxerLib_t * pstMuxLib = (ST_MuxerLib_t *)pstMuxerFileAttr->pvMuxerLib;
    ST_MuxerFrame_t stMuxFrame;

    stMuxFrame.u64Pts    = pstStream->u64Pts;
    stMuxFrame.pstStream = pstMuxLib->pstVideoStream;
    stMuxFrame.bKeyFrame = pstStream->bKeyFrame;

    stMuxFrame.pvBuf      = pstStream->pu8Addr;
    stMuxFrame.u32BufSize = pstStream->u32Len;

    return _MuxerLibWriteVideo(pstMuxLib, &stMuxFrame);
}

static MI_S32 _MuxerWriteAudio(ST_MuxerFileAttr_t *pstMuxerFileAttr, ST_MuxerAudioFrame_t *pstFrame)
{
    ST_MuxerLib_t * pstMuxLib = (ST_MuxerLib_t *)pstMuxerFileAttr->pvMuxerLib;
    ST_MuxerFrame_t stMuxFrame;

    stMuxFrame.u64Pts     = pstFrame->u64Pts;
    stMuxFrame.pstStream  = pstMuxLib->pstAudioStream;
    stMuxFrame.bKeyFrame  = FALSE;
    stMuxFrame.pvBuf      = pstFrame->pvBuf;
    stMuxFrame.u32BufSize = pstFrame->u32Len;
    return _MuxerLibWriteAudio(pstMuxLib, &stMuxFrame);
}

static MI_S32 _MuxerCloseFile(ST_MuxerAttr_t *pstMuxerAttr)
{
    ST_MuxerFileAttr_t *pstMuxerFileAttr = &pstMuxerAttr->stFileAttr;
    //if (pstMuxerFileAttr->bUsed && pstMuxerFileAttr->bInit && pstMuxerFileAttr->bOpen)
    {
        _MuxerLibClose((ST_MuxerLib_t *)pstMuxerFileAttr->pvMuxerLib);
        pstMuxerFileAttr->bOpen = FALSE;
        free(pstMuxerFileAttr->pvMuxerLib);
        pstMuxerFileAttr->bInit = FALSE;
    }

    return 0;
}

static uint64_t get_reltime(void)
{
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (tp.tv_sec * 1000000ULL + tp.tv_nsec / 1000ULL);
}

static int audio_mux_init(void)
{
    ST_AudioCodecAttr_t stCodecAttr;
#if 0
    stCodecAttr.eCodecId      = AV_CODEC_ID_AAC;
    stCodecAttr.eSampleFmt    = AV_SAMPLE_FMT_S16;
    stCodecAttr.u32BitRate    = 0;
    stCodecAttr.u16SampleRate = 44100; //TODO
    stCodecAttr.u64ChannelLayout = AV_CH_LAYOUT_MONO;
#endif

    gstMuxerAttr.stFileAttr.pvAudioCodecHandler = ST_AudioCodecOpen(&stCodecAttr);
    if (gstMuxerAttr.stFileAttr.pvAudioCodecHandler == NULL)
    {
        return -1;
    }

    return 0;
}

static int muxer_start(ST_VideoSetting_t* pstVideoSetting)
{
    FrameInfo_t pframe_video = {0};
    FrameInfo_t pframe_audio = {0};

    MT_VENC_Stream_t  stStream;
    ST_MuxerAttr_t *pstMuxerAttr = &gstMuxerAttr;
    ST_MuxerFileAttr_t *pstMuxerFileAttr = &pstMuxerAttr->stFileAttr;

    pstMuxerFileAttr->bAudioEnable = TRUE;
    if (pstMuxerFileAttr->bAudioEnable)
    {
        audio_mux_init();
    }

    _MuxerSetAttr();
    _MuxerOpenFile(pstMuxerAttr);

    while (!pstMuxerAttr->bMuxerExit)
    {
        if (0 != mem_get_frame(MEM_NAME_VIDEO, &pframe_video))
        {
            usleep(1000);
            continue;
        }            

        if (!pstMuxerFileAttr->bFoundKey)
        {
            if (!pframe_video.frame_head.frame_type)
            {
                usleep(1000);
                continue;
            }
            pstMuxerFileAttr->bFoundKey = true;
        }            
        //printf("len:%d.\n", pframe_video.frame_head.data_len);

        stStream.pu8Addr    = pframe_video.pdata;
        stStream.u32Len     = pframe_video.frame_head.data_len;
        stStream.bKeyFrame  = pframe_video.frame_head.frame_type;
        stStream.u64Pts     = get_reltime();
        
        _MuxerWriteVideo(pstMuxerFileAttr, &stStream);

        if (pstMuxerFileAttr->bAudioEnable)
        {
            ST_MuxerAudioFrame_t stAudioFrame;

            if (0 == mem_get_frame(MEM_NAME_AUDIO, &pframe_audio))
            {
                stAudioFrame.pvBuf  = pframe_audio.pdata;
                stAudioFrame.u32Len = pframe_audio.frame_head.data_len;
                stAudioFrame.u64Pts = get_reltime();
                _MuxerWriteAudio(pstMuxerFileAttr, &stAudioFrame);
            }            
        }
    }

    _MuxerCloseFile(pstMuxerAttr);
    return 0;
}

static int muxer_stop(void)
{
    ST_MuxerAttr_t *pstMuxerAttr = &gstMuxerAttr;

    if (pstMuxerAttr->bCreate)
    {
        pstMuxerAttr->bMuxerExit = TRUE;
    }

    return 0;
}

int mt_muxer_start(char *muxer_name)
{
    ST_VideoSetting_t stVideoSetting;
    stVideoSetting.u16RecW              = 1920;
    stVideoSetting.u16RecH              = 1080;
    stVideoSetting.u32RecBitRate        = 3000000;
    stVideoSetting.u8RecCodecType       = CODEC_TYPE_H264;

    ST_MuxerAttr_t *pstMuxerAttr = &gstMuxerAttr;
    if (!pstMuxerAttr->bCreate)
    {
        ST_MuxerFileAttr_t *pstMuxerFileAttr = &pstMuxerAttr->stFileAttr;
        pstMuxerFileAttr->muxer_name = muxer_name;

        pstMuxerAttr->bCreate    = TRUE;
        pstMuxerAttr->bMuxerExit = FALSE;
        muxer_start(&stVideoSetting);
    }
    return 0;
}

int mt_muxer_stop(void)
{
    muxer_stop();
    return 0;
}
