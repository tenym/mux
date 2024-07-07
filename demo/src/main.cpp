#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include "MtAACEncoderAPI.h"
#include "video_codec.h"
#include "mRing.h"
#include "muxer.h"

using namespace std;

static pthread_t g_task_video_pid;
static pthread_t g_task_audio_pid;
static char g_video_file[128];
static char g_audio_file[128];
static char g_muxer_file[128];

static int TestG711ToAAC(char *audio_name)
{
    int active = 0;
    InitParam initParam;
    initParam.u32AudioSamplerate=8000;
    initParam.ucAudioChannel=1;
    initParam.u32PCMBitSize=16;
    initParam.ucAudioCodec = Law_ALaw;
    //initParam.ucAudioCodec = Law_ULaw;

    Mt_Handle handle = Mt_AACEncoder_Init(initParam);
    char* infilename = (char *)audio_name;

    FILE* fpIn = fopen(infilename, "rb");
    if(NULL == fpIn)
    {
        printf("%s:[%d] open %s file failed\n",__FUNCTION__,__LINE__,infilename);
        return -1;
    }

    int gBytesRead = 0;
    int bG711ABufferSize = 500;
    int bAACBufferSize = 4*bG711ABufferSize;//提供足够大的缓冲区
    unsigned char *pbG711ABuffer = (unsigned char *)malloc(bG711ABufferSize *sizeof(unsigned char));
    unsigned char *pbAACBuffer = (unsigned char*)malloc(bAACBufferSize * sizeof(unsigned char));  
    unsigned int out_len = 0;

    FrameInfo_t pframe = {0};
    int count = 0;
    while((gBytesRead = fread(pbG711ABuffer, 1, bG711ABufferSize, fpIn)) >0)
    {    
        if(Mt_AACEncoder_Encode(handle, pbG711ABuffer, gBytesRead, pbAACBuffer, &out_len) > 0)
        {
            pframe.frame_head.data_len = out_len;
            pframe.pdata = pbAACBuffer;
            mem_put_frame(MEM_NAME_AUDIO, &pframe);
            count++;
            usleep(1000*1000/15);
        }
    }
    printf("aud_count:%d.\n", count);

    Mt_AACEncoder_Release(handle);

    free(pbG711ABuffer);
    free(pbAACBuffer);
    fclose(fpIn);
    return 0;
}


static void *test_video(void*)
{
    //!< Read h264 and  push ring buffer
    video_codec_read(g_video_file);
}

static void *test_audio(void*)
{
    //!< g711 conver to aac and  push ring buffer
    TestG711ToAAC(g_audio_file);
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("%s video.h264 audio.g711 muxer.mp4\n", argv[0]);
        return -1;
    }
    sprintf(g_video_file, argv[1]);
    sprintf(g_audio_file, argv[2]);
    sprintf(g_muxer_file, argv[3]);

    //init ring buffer
    mem_init();

    pthread_create(&g_task_video_pid, NULL, test_video, NULL);
    pthread_detach(g_task_video_pid);

    pthread_create(&g_task_audio_pid, NULL, test_audio, NULL);
    pthread_detach(g_task_audio_pid);

    //!< read h264 and aac from ring buffer, and pack to mp4 
    mt_muxer_start(g_muxer_file);

    return 0;
}

