/**
* @file     mRing.cpp
* @brief    
* @details  
* @author   tengym
* @date     2021-11-1
* @version  V1.0
* @copyright    Copyright (c) 2050
* 
* @attention
* @par Modify Log:
* <table>
* <tr><th>Date        <th>Version  <th>Author  <th>Description
* <tr><td>2021/11/1  <td>1.0      <td>tengym  <td>First Version
* </table>
*
*/
#include <pthread.h>
#include <stdlib.h>
#include "mRing.h"

typedef enum
{
    MEM_INIT_SIZE = 0,
    MEM_INIT_FRAME_NUM,
    MEM_INIT_FRAME_SIZE,
    MEM_INIT_MAX_NUM
} MemInit_E;

typedef enum
{
    MEM_STATUS_STOP = 0,
    MEM_STATUS_START
} MemStatus_E;

typedef struct
{
    unsigned long long read_frame_seq;              /**< ��ָ�����*/
    unsigned long long write_frame_seq;             /**< дָ�����*/
    unsigned long long write_offset;                /**< ȡ��Ϊдָ�������ʵ�ʵ�ַƫ����*/
    unsigned long long read_offset;                 /**< ȡ��Ϊ��ָ�������ʵ�ʵ�ַƫ����*/
    unsigned int status;                            /**< mem״̬λ*/
    unsigned char *pframe_head;                     /**< ʵ��֡ͷ��ŵ�ַ*/
    unsigned char *pdata;                           /**< ʵ�����ݴ�ŵ�ַ*/
} MemHead_t;

//mem�ڴ��С, ֡��, ÿ֡��ŵ����size
const unsigned int g_mem_init[MEM_NAME_MAX][MEM_INIT_MAX_NUM] = {
    {1024*300*110, 100, 1024*300}, 
    {1024*10*110, 100, 1024*10}, 
};
static unsigned char* g_mem[MEM_NAME_MAX];
static pthread_mutex_t g_mem_mutex[MEM_NAME_MAX];

static void mem_mutex_lock(int name_type)
{
	//pthread_mutex_lock(&g_mem_mutex[name_type]);
}

static void mem_mutex_unlock(int name_type)
{
	//pthread_mutex_unlock(&g_mem_mutex[name_type]);
}

int mem_put_frame(unsigned int type_name, FrameInfo_t *pframe)
{
    if(NULL == pframe || NULL == pframe->pdata || type_name >= MEM_NAME_MAX
        || NULL == g_mem[type_name] || pframe->frame_head.data_len > g_mem_init[type_name][MEM_INIT_FRAME_SIZE])
    {
        return -1;
    }

    mem_mutex_lock(type_name);

    unsigned int frame_max_num = g_mem_init[type_name][MEM_INIT_FRAME_NUM];
    unsigned int mem_max_size = g_mem_init[type_name][MEM_INIT_SIZE];
    unsigned int max_frame_len = g_mem_init[type_name][MEM_INIT_FRAME_SIZE];

    MemHead_t* MemHead = (MemHead_t*)g_mem[type_name];
    if(MemHead->status == MEM_STATUS_STOP)
    {
        mem_mutex_unlock(type_name);
        return -1;
    }

    FrameHead_t* WriteFrameHead = (FrameHead_t*)(MemHead->pframe_head + ((MemHead->write_frame_seq % frame_max_num) * sizeof(FrameHead_t)));

    unsigned long long real_write_offset = MemHead->write_offset % mem_max_size;
    unsigned long long skip_write_offset = 0;
    unsigned int real_copy_len = pframe->frame_head.data_len + pframe->frame_head.extend_head_len;
    
    if(real_write_offset + real_copy_len >= mem_max_size)
    {
        skip_write_offset = mem_max_size - real_write_offset;
        real_write_offset = 0;
    }
    
    //дָ��׷��
    if( ((MemHead->write_frame_seq - MemHead->read_frame_seq) >= (frame_max_num - frame_max_num/10)) || 
         ((MemHead->write_offset + skip_write_offset + real_copy_len + max_frame_len*2) > (MemHead->read_offset + mem_max_size)) )
    {
        mem_mutex_unlock(type_name);
        return -2;
    }

    //printf("=========writeoffset:%llu realw_offset:%llu readoffset:%llu realr_offset:%llu len=%d\n", 
    //MemHead->write_offset, real_write_offset, MemHead->read_offset, MemHead->read_offset%mem_max_size, pframe->frame_head.data_len);

    memcpy(WriteFrameHead, &pframe->frame_head, sizeof(FrameHead_t));
    memcpy(MemHead->pdata + real_write_offset + pframe->frame_head.extend_head_len, pframe->pdata, pframe->frame_head.data_len);
    
    WriteFrameHead->offset = MemHead->write_offset + skip_write_offset;
    WriteFrameHead->data_len = real_copy_len;
    MemHead->write_offset = WriteFrameHead->offset + WriteFrameHead->data_len;
    MemHead->write_frame_seq++;

    mem_mutex_unlock(type_name);
    return 0;
}

int mem_get_frame(unsigned int type_name, FrameInfo_t *pframe)
{
    if(NULL == pframe || type_name >= MEM_NAME_MAX || NULL == g_mem[type_name])
    {
        return -1;
    }

    mem_mutex_lock(type_name);

    unsigned int frame_max_num = g_mem_init[type_name][MEM_INIT_FRAME_NUM];
    unsigned int mem_max_size = g_mem_init[type_name][MEM_INIT_SIZE];

    MemHead_t* MemHead = (MemHead_t*)g_mem[type_name];
    //�Ѿ��õ�����֡
    if(MemHead->read_frame_seq == MemHead->write_frame_seq)
    {
        mem_mutex_unlock(type_name);
        return -1;
    }

    FrameHead_t* ReadFrameHead = (FrameHead_t*)(MemHead->pframe_head + ((MemHead->read_frame_seq % frame_max_num) * sizeof(FrameHead_t)));

    unsigned long long real_read_offset = ReadFrameHead->offset % mem_max_size;
    unsigned long long skip_read_offset = 0;
    if(real_read_offset + ReadFrameHead->data_len >= mem_max_size)
    {
        skip_read_offset = mem_max_size - real_read_offset;
        printf("=============mem_get_frame error ReadFrameHead->offset(%llu) real_read_offset(%llu) len(%d) max(%d)\n",
            ReadFrameHead->offset, real_read_offset, ReadFrameHead->data_len, mem_max_size);
        real_read_offset = 0;
    }
    
    memcpy(&pframe->frame_head, ReadFrameHead, sizeof(FrameHead_t));
    pframe->pdata = MemHead->pdata + real_read_offset;

    MemHead->read_offset = ReadFrameHead->offset + skip_read_offset + ReadFrameHead->data_len;
    MemHead->read_frame_seq++;
    
    mem_mutex_unlock(type_name);
    return 0;
}

int mem_init(void)
{
    int i = 0;
    for(i = 0; i < MEM_NAME_MAX; i++)
    {
        if(NULL == g_mem[i])
        {
            int memlen = sizeof(MemHead_t) + sizeof(FrameHead_t)*g_mem_init[i][MEM_INIT_FRAME_NUM] + g_mem_init[i][MEM_INIT_SIZE];
            g_mem[i] = (unsigned char *)calloc(1, memlen);
        }
        pthread_mutex_init(&g_mem_mutex[i], NULL);
        MemHead_t* MemHead = (MemHead_t*)(g_mem[i]);
        MemHead->read_frame_seq = MemHead->write_frame_seq = MemHead->write_offset = MemHead->read_offset = 0;
        MemHead->status = MEM_STATUS_START;
        MemHead->pframe_head = (unsigned char*)MemHead + sizeof(MemHead_t);
        MemHead->pdata = (unsigned char*)MemHead + sizeof(MemHead_t) + sizeof(FrameHead_t)*g_mem_init[i][MEM_INIT_FRAME_NUM];
    }

    return 0;
}

int mem_deinit(void)
{
    int i = 0;
    for(i = 0; i < MEM_NAME_MAX; i++)
    {
        mem_mutex_lock(i);
        free(g_mem[i]);
        mem_mutex_unlock(i);
    }
    return 0;
}
