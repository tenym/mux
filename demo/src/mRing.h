/**
* @file     .h
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

#ifndef __MRING_H__
#define __MRING_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <string.h>

typedef enum
{
    MEM_NAME_VIDEO = 0,
    MEM_NAME_AUDIO,
    MEM_NAME_MAX,
} MemName_E;

typedef struct
{
    unsigned int frame_type;
    unsigned int data_len;
    unsigned int extend_head_len;

    unsigned long long pts;
    unsigned long long offset;
} FrameHead_t;

typedef struct
{
    FrameHead_t frame_head;
    unsigned char* pdata;
} FrameInfo_t;

int mem_init(void);
int mem_deinit(void);
int mem_put_frame(unsigned int type_name, FrameInfo_t *pframe);
int mem_get_frame(unsigned int type_name, FrameInfo_t *pframe);

#ifdef __cplusplus 
}
#endif
#endif
