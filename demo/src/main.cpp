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

    while (1) sleep(1);

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

