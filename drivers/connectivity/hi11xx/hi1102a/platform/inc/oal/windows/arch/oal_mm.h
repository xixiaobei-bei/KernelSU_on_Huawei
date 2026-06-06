

#ifndef __OAL_WINDOWS_MM_H__
#define __OAL_WINDOWS_MM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include <windows.h>

/*****************************************************************************
  2 宏定义
*****************************************************************************/

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  5 消息头定义
*****************************************************************************/

/*****************************************************************************
  6 消息定义
*****************************************************************************/

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
/*
 * 函 数 名  : oal_memalloc
 * 功能描述  : 申请内存空间。
 * 输入参数  : ul_size: alloc mem size
 * 输出参数  : 无
 * 返 回 值  : alloc mem addr
 */
OAL_INLINE oal_void *oal_memalloc(oal_uint32 ul_size)
{
    oal_void *p_mem_space;

    p_mem_space = malloc(ul_size);

    return p_mem_space;
}

/*
 * 函 数 名  : oal_memtry_alloc
 * 功能描述  : 尝试申请大块内存，指定期望申请的最大和最小内存，
 *             返回实际申请的内存，申请失败返回NULL
 * 输入参数  :
 * 输出参数  : 无
 * 返 回 值  : alloc mem addr
 */
OAL_STATIC OAL_INLINE oal_void *oal_memtry_alloc(oal_uint32 request_maxsize, oal_uint32 request_minsize,
                                                 oal_uint32 *actual_size)
{
    oal_void *puc_mem_space;
    oal_uint32 request_size;

    if (actual_size == NULL) {
        return NULL;
    }

    *actual_size = 0;

    request_size = (request_maxsize > request_minsize) ? request_maxsize : request_minsize;

    do {
        puc_mem_space = oal_memalloc(request_size);
        if (puc_mem_space != NULL) {
            *actual_size = request_size;
            return puc_mem_space;
        }

        if (request_size <= request_minsize) {
            /* 以最小SIZE申请依然失败返回NULL */
            break;
        }

        /* 申请失败, 折半重新申请 */
        request_size = request_size >> 1;
        request_size = (request_size > request_minsize) ? request_size : request_minsize;
    } while (1);

    return NULL;
}

/*
 * 函 数 名  : oal_free
 * 功能描述  : 释放核心态的内存空间。
 * 输入参数  : pbuf:需释放的内存地址
 * 输出参数  : 无
 * 返 回 值  : 无
 */
OAL_INLINE oal_void oal_free(oal_void *p_buf)
{
    free(p_buf);
}

/*
 * 函 数 名  : oal_mem_dma_blockalloc
 * 功能描述  : 尝试申请大块DMA内存，阻塞申请，直到申请成功，可以设置超时
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : alloc mem addr
 */
OAL_STATIC OAL_INLINE oal_void *oal_mem_dma_blockalloc(oal_uint32 size, oal_ulong timeout)
{
    OAL_REFERENCE(timeout);
    return oal_memalloc(size);
}

/*
 * 函 数 名  : oal_mem_dma_blockfree
 * 功能描述  : 释放DMA内存
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : alloc mem addr
 */
OAL_STATIC OAL_INLINE oal_void oal_mem_dma_blockfree(oal_void *puc_mem_space)
{
    oal_free(puc_mem_space);
}

/*
 * 函 数 名  : oal_memcopy
 * 功能描述  : 复制内存。不允许两块内存重叠。
 * 输入参数  : *p_dst: 内存复制目的地址
 *             *p_src: 内存复制源地址
 *             ul_size: 复制内存大小
 * 输出参数  : 无
 * 返 回 值  : 无
 */
OAL_INLINE oal_void oal_memcopy(oal_void *p_dst, const oal_void *p_src, oal_uint32 ul_size)
{
    memcpy(p_dst, p_src, ul_size);
}

/*
 * 函 数 名  : oal_memmove
 * 功能描述  : 复制内存。允许两块内存重叠。
 * 输入参数  : *p_dst: mem move destination addr

 *             *p_src: mem move source addr
 *             ul_size: mem move size
 * 输出参数  : 无
 * 返 回 值  : 无
 */
OAL_INLINE oal_void oal_memmove(oal_void *p_dst, const oal_void *p_src, oal_uint32 ul_size)
{
    memmove(p_dst, p_src, ul_size);
}

/*
 * 函 数 名  : oal_memset
 * 功能描述  : 内存填充。
 * 输入参数  : *p_buf: 内存设置地址
 *             uc_char: char be set to mem
 *             ul_size: size of mem be set
 * 输出参数  : 无
 * 返 回 值  : 无
 */
OAL_INLINE oal_void oal_memset(oal_void *p_buf, oal_int32 l_data, oal_uint32 ul_size)
{
    memset(p_buf, l_data, ul_size);
}

OAL_INLINE errno_t memset_s(void *dest, size_t destMax, int c, size_t count)
{
    (void)destMax;
    memset(dest, c, count);
}

#endif /* end of oal_mm.h */

