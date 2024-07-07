#include <libavformat/avformat.h>
#include "mRing.h"
#include "muxer.h"

static int push_packet(AVPacket *packet) 
{
    FrameInfo_t pframe = {0};

    pframe.frame_head.data_len = packet->size;

    if (packet->flags & AV_PKT_FLAG_KEY) 
    {
        //printf("Key frame at PTS %" PRId64 "\n", packet->pts);
        pframe.frame_head.frame_type = 1;
    }
    pframe.pdata = packet->data;

    //printf("in:%d.\n", packet->size);
    mem_put_frame(MEM_NAME_VIDEO, &pframe);
    return 0;
}

int video_codec_read(char *p) 
{
    const char *input_filename = p;
    AVFormatContext *format_context = NULL;
    int video_stream_index = -1;
    AVPacket packet;
    
    if (avformat_open_input(&format_context, input_filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open input file '%s'\n", input_filename);
        return -1;
    }
    
    if (avformat_find_stream_info(format_context, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        return -1;
    }
    
    for (int i = 0; i < format_context->nb_streams; i++) {
        if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    
    if (video_stream_index == -1) {
        fprintf(stderr, "Could not find video stream\n");
        return -1;
    }
    
    int count = 0;
    while (av_read_frame(format_context, &packet) >= 0) 
    {
        if (packet.stream_index == video_stream_index) 
        {
            if (push_packet(&packet) < 0) 
            {
                fprintf(stderr, "Buffer is full, dropping packet\n");
                av_packet_unref(&packet);
            }
            usleep(1000*1000/15);
            count++;
        } 
        else 
        {
            av_packet_unref(&packet);
        }
    }
    printf("\nvideo_count:%d.\n", count);
    
    avformat_close_input(&format_context);

    mt_muxer_stop();
    return 0;
}

