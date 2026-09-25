

#ifndef __OAL_WINDOWS_MM_H__
#define __OAL_WINDOWS_MM_H__

/* 其他头文件包含 */
#include <windows.h>

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
 */
OAL_STATIC OAL_INLINE oal_void *oal_memtry_alloc(oal_uint32 request_maxsize, oal_uint32 request_minsize,
                                                 oal_uint32 *actual_size)
{
    oal_void *puc_mem_space = NULL;
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

OAL_INLINE oal_void oal_free(oal_void *p_buf)
{
    free(p_buf);
}

/*
 * 函 数 名  : oal_mem_dma_blockalloc
 * 功能描述  : 尝试申请大块DMA内存，阻塞申请，直到申请成功，可以设置超时
 */
OAL_STATIC OAL_INLINE oal_void *oal_mem_dma_blockalloc(oal_uint32 size, oal_ulong timeout)
{
    OAL_REFERENCE(timeout);
    return oal_memalloc(size);
}

/*
 * 函 数 名  : oal_mem_dma_blockfree
 * 功能描述  : 释放DMA内存
 */
OAL_STATIC OAL_INLINE oal_void oal_mem_dma_blockfree(oal_void *puc_mem_space)
{
    oal_free(puc_mem_space);
}

OAL_INLINE errno_t memset_s(void *dest, size_t destMax, int c, size_t count)
{
    (void)destMax;
    memset(dest, c, count);
}

#endif /* end of oal_mm.h */
