
#ifndef _COMMON_H
#define _COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

/// data type unsigned char, data length 1 byte
typedef unsigned char MI_U8; // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short MI_U16; // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned int MI_U32; // 4 bytes
/// data type unsigned int, data length 8 byte
typedef unsigned long long MI_U64; // 8 bytes
/// data type signed char, data length 1 byte
typedef signed char MI_S8; // 1 byte
/// data type signed short, data length 2 byte
typedef signed short MI_S16; // 2 bytes
/// data type signed int, data length 4 byte
typedef signed int MI_S32; // 4 bytes
/// data type signed int, data length 8 byte
typedef signed long long MI_S64; // 8 bytes
/// data type float, data length 4 byte
typedef float MI_FLOAT; // 4 bytes
/// data type 64bit physical address
typedef unsigned long long MI_PHY; // 8 bytes
/// data type pointer content
typedef unsigned long MI_VIRT; // 4 bytes when 32bit toolchain, 8 bytes when 64bit toolchain.

typedef unsigned char MI_BOOL;

#if !defined(TRUE) && !defined(FALSE)
#define TRUE                1
#define FALSE               0
#endif

#ifndef DBG_ERR
#define DBG_ERR(fmt, args...)                                                                               \
    do                                                                                                      \
    {                                                                                                       \
        printf("[ERR]:%s[%d]:" fmt, __FUNCTION__, __LINE__, ##args); \
    } while (0)
#endif

#ifndef DBG_WARN
#define DBG_WARN(fmt, args...)                                                                                  \
    do                                                                                                          \
    {                                                                                                           \
        printf("[WARN]:%s[%d]:" fmt, __FUNCTION__, __LINE__, ##args); \
    } while (0)
#endif

#ifndef DBG_INFO
#define DBG_INFO(fmt, args...)                                                          \
    do                                                                                  \
    {                                                                                   \
        printf("[INFO]:%s[%d]:" fmt, __FUNCTION__, __LINE__, ##args); \
    } while (0)
#endif

#ifndef DBG_VERBOSE
#define DBG_VERBOSE(fmt, args...)                                                                             \
    do                                                                                                        \
    {                                                                                                         \
        printf("[DBG]:%s[%d]:" fmt, __FUNCTION__, __LINE__, ##args); \
    } while (0)
#endif

#if (DEBUG == 0)
#undef DBG_VERBOSE
#define DBG_VERBOSE(fmt, args...)
#endif
#ifdef __cplusplus 
}
#endif
#endif
