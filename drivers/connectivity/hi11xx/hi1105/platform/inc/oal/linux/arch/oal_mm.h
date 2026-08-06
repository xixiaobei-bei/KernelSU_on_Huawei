

#ifndef __OAL_LINUX_MM_H__
#define __OAL_LINUX_MM_H__

/* 其他头文件包含 */
/*lint -e322*/
#include <linux/slab.h>
#include <linux/hardirq.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#ifdef _PRE_WLAN_CACHE_COHERENT_SUPPORT
#include <linux/dma-mapping.h>
#endif

/*lint +e322*/
/* 宏定义 */
typedef dma_addr_t oal_dma_addr;

/* 函数声明 */
/*
 * 函 数 名  : oal_memalloc
 * 功能描述  : 申请核心态的内存空间，并填充0。对于Linux操作系统而言，
 *             需要考虑中断上下文和内核上下文的不同情况(GFP_KERNEL和GFP_ATOMIC)。
 */
OAL_STATIC OAL_INLINE void *oal_memalloc(uint32_t ul_size)
{
    int32_t l_flags = GFP_KERNEL;
    void *puc_mem_space = NULL;

    /* 不睡眠或在中断程序中标志置为GFP_ATOMIC */
    if (in_interrupt() || irqs_disabled() || in_atomic()) {
        l_flags = GFP_ATOMIC;
    }

    if (unlikely(ul_size == 0)) {
        return NULL;
    }

    puc_mem_space = kmalloc(ul_size, l_flags);
    if (puc_mem_space == NULL) {
        return NULL;
    }

    return puc_mem_space;
}

/*
 * 函 数 名  : oal_mem_dma_blockalloc
 * 功能描述  : 尝试申请大块DMA内存，阻塞申请，直到申请成功，可以设置超时
 */
OAL_STATIC OAL_INLINE void *oal_mem_dma_blockalloc(uint32_t size, unsigned long timeout)
{
    gfp_t flags = 0;
    void *puc_mem_space = NULL;
    unsigned long timeout2, timeout1;

    if (unlikely(size == 0)) {
        return NULL;
    }

    /* 不睡眠或在中断程序中标志置为GFP_ATOMIC */
    if (in_interrupt() || irqs_disabled() || in_atomic()) {
        flags |= GFP_ATOMIC;

        return kmalloc(size, flags);
    } else {
        flags |= GFP_KERNEL;
    }

    flags |= __GFP_NOWARN;

    timeout2 = jiffies + msecs_to_jiffies(timeout);     /* ms */
    timeout1 = jiffies + msecs_to_jiffies(timeout / 2); /* 2分之一 */

    do {
        puc_mem_space = kmalloc(size, flags);
        if (likely(puc_mem_space != NULL)) {
            /* sucuess */
            return puc_mem_space;
        }

        if (!time_after(jiffies, timeout1)) {
            cpu_relax();
            continue; /* 未超时，继续 */
        }

        if (!time_after(jiffies, timeout2)) {
            msleep(1);
            continue; /* 长时间未申请到，开始让出调度 */
        } else {
            if (flags & __GFP_NOWARN) {
                /* 超时，清掉报警屏蔽标记，尝试最后一次 */
                flags &= ~__GFP_NOWARN;
                continue;
            } else {
                /* 超时返回失败 */
                break;
            }
        }
    } while (1);

    return NULL;
}

/*
 * 函 数 名  : oal_mem_dma_blockfree
 * 功能描述  : 释放DMA内存
 */
OAL_STATIC OAL_INLINE void oal_mem_dma_blockfree(void *puc_mem_space)
{
    if (likely(puc_mem_space != NULL)) {
        kfree(puc_mem_space);
    }
}

/*
 * 函 数 名  : oal_memtry_alloc
 * 功能描述  : 尝试申请大块内存，指定期望申请的最大和最小内存，
 *             返回实际申请的内存，申请失败返回NULL
 */
OAL_STATIC OAL_INLINE void *oal_memtry_alloc(uint32_t request_maxsize, uint32_t request_minsize,
                                             uint32_t *actual_size)
{
    gfp_t flags = 0;
    uint32_t is_atomic = 0;
    void *puc_mem_space = NULL;
    uint32_t request_size;

    if (WARN_ON(actual_size == NULL)) {
        return NULL;
    }

    *actual_size = 0;

    /* 不睡眠或在中断程序中标志置为GFP_ATOMIC */
    if (in_interrupt() || in_atomic() || irqs_disabled()) {
        is_atomic = 1;
    }

    flags |= __GFP_NOWARN | GFP_ATOMIC;

    request_size = ((request_maxsize >= request_minsize) ? request_maxsize : request_minsize);

    do {
        if (request_size <= request_minsize) {
            if (is_atomic) {
                flags &= ~__GFP_NOWARN;
            } else {
                flags = GFP_KERNEL;
            }
        }

        puc_mem_space = kmalloc(request_size, flags);
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
        request_size = ((request_size >= request_minsize) ? request_size : request_minsize);
    } while (1);

    return NULL;
}

#ifdef _PRE_WLAN_CACHE_COHERENT_SUPPORT
/*
 * 函 数 名  : oal_mem_uncache_alloc
 * 功能描述  : 一致性内存(非cache)分配函数
 * 输入参数  : 待分配的mem的大小(size),返回的物理地址
 * 输出参数  : 返回的地址块物理地址的首地址(phy addr)
 * 返 回 值  : 分配后的虚拟地址
 */
OAL_STATIC OAL_INLINE void *oal_mem_uncache_alloc(uint32_t ul_size, uint32_t *pul_phy_addr)
{
    int32_t l_flags = GFP_KERNEL;
    void *puc_mem_space = NULL;
    uint32_t ul_dma_real_addr;

    if (oal_unlikely(pul_phy_addr == NULL)) {
        oal_warn_on(1);
        return NULL;
    }

    /* 不睡眠或在中断程序中标志置为GFP_ATOMIC */
    if (in_interrupt() || irqs_disabled()) {
        l_flags = GFP_ATOMIC;
    }

    puc_mem_space = dma_alloc_coherent(NULL, ul_size, &dma_real_addr, l_flags);

    /* 保存非cache内存的物理地址 */
    *pul_phy_addr = (uint32_t)ul_dma_real_addr;

    if (puc_mem_space == NULL) {
        return NULL;
    }

    return puc_mem_space;
}

/*
 * 函 数 名  : oal_mem_uncache_free
 * 功能描述  : 一致性内存(非cache)释放函数
 * 输入参数  : 待分配的mem的大小(size),分配内存的虚拟地址(p_buf),
 *             对应的物理地址(ul_dma_addr)
 */
OAL_STATIC OAL_INLINE void oal_mem_uncache_free(uint32_t ul_size, void *p_buf, uint32_t ul_dma_addr)
{
    dma_free_coherent(NULL, ul_size, p_buf, ul_dma_addr);
}

/*
 * 函 数 名  : oal_dma_map_single
 * 功能描述  : 刷cache接口，将cache无效掉或者将cache中的内容同步到DDR
 * 输入参数  : 第一个参数:struct device,第二个参数:需要cache的指针，第三个参数:cache操作的size大小
 *             第四个参数:方向(0:双方向;1:将cache同步到DDR;2:将cache无效)
 * 返 回 值  : cache相关指针映射的物理地址
 */
OAL_STATIC OAL_INLINE uint32_t oal_dma_map_single(struct device *pst_dev, void *p_buf,
                                                  uint32_t ul_size, oal_direction_uint8 uc_dir)
{
    return dma_map_single(pst_dev, p_buf, ul_size, uc_dir);
}

/*
 * 函 数 名  : oal_dma_unmap_single
 * 功能描述  : 该接口主要作为map之后映射的地址取消映射，同时对于包含L1、L2级cache，最好配之前map，配后在unmap
 * 输入参数  : 第一个参数:struct device,第二个参数:需要cache的指针映射的物理地址，第三个参数:cache操作的size大小
 *             第四个参数:方向(0:双方向;1:将cache同步到DDR;2:将cache无效)
 */
OAL_STATIC OAL_INLINE void oal_dma_unmap_single(struct device *pst_dev, oal_dma_addr ul_addr,
                                                uint32_t ul_size, oal_direction_uint8 uc_dir)
{
    dma_unmap_single(pst_dev, ul_addr, ul_size, uc_dir);
}
#endif

/*
 * 函 数 名  : oal_free
 * 功能描述  : 释放核心态的内存空间。
 * 输入参数  : pbuf:需释放的内存地址
 */
OAL_STATIC OAL_INLINE void oal_free(void *p_buf)
{
    kfree(p_buf);
}

#endif /* end of oal_mm.h */
