#ifndef __MDRV_COMMON_MALLOC_H__
#define __MDRV_COMMON_MALLOC_H__
#ifdef __cplusplus
extern "C"
{
#endif

/**
 * flags, for mdrv_mem_alloc
 */
#define __MEM_ALIGN_008 0x01u /**< 8字节对齐*/
#define __MEM_ALIGN_016 0x02u/**< 16字节对齐*/
#define __MEM_ALIGN_032 0x04u/**< 32字节对齐*/
#define __MEM_ALIGN_064 0x08u/**< 64字节对齐*/
#define __MEM_ALIGN_128 0x10u/**< 128字节对齐*/
#define __MEM_ALIGN_256 0x20u/**< 256字节对齐*/
#define __MEM_ALIGN_512 0x40u/**< 512字节对齐*/
#define __MEM_ALIGN_1K  0x80u/**< 1K字节对齐*/
#define __MEM_ALIGN_MSK 0xffu

#define __MEM_DMA       0x100u/*分配uncached内存*/

/*****************************************************************************
 *  函 数 名  : mdrv_reserved_mem_malloc
 *  功能描述  : 分配预定区内存，该区域内存只分配不释放
 *  输入参数  :
 *              size: 分配内存大小
 *              request_id: 需求ID
 *
 *  输出参数  :
 *  返 回 值  :  NULL   分配失败。
 *               其他   已分配内存地址。
 *
 ******************************************************************************/
void* mdrv_reserved_mem_malloc(unsigned int size, enum MALLOC_RESERVED_REQ_ID request_id);
 
 /*****************************************************************************
  *  函 数 名  : mdrv_cached_mem_malloc
  *  功能描述  : 分配cached内存
  *  输入参数  :
  * 			 size: 分配内存大小
  * 			 request_id: 需求ID
  *
  *  输出参数  :
  *  返 回 值  :  NULL	 分配失败。
  * 			  其他	 已分配内存地址。
  *
  ******************************************************************************/
void* mdrv_cached_mem_malloc(unsigned int size); 
 
/*****************************************************************************
*  函 数 名  : mdrv_cached_mem_free
*  功能描述  : 释放指定cached内存。
*  输入参数  :
* 			 addr: 内存首地址
*
*  输出参数  : 无
*  返 回 值  : 无
*
******************************************************************************/
void mdrv_cached_mem_free(void* addr); 

/*****************************************************************************
 *  函 数 名  : mdrv_uncached_mem_malloc
 *  功能描述  : 分配uncached内存
 *  输入参数  :
 *              size: 分配内存大小
 *              request_id: 需求ID
 *
 *  输出参数  :
 *  返 回 值  :  NULL   分配失败。
 *               其他   已分配内存地址。
 *
 ******************************************************************************/
void* mdrv_uncached_mem_malloc(unsigned int size); 

/*****************************************************************************
 *  函 数 名  : mdrv_uncached_mem_free
 *  功能描述  : 释放指定uncached内存。
 *  输入参数  :
 *              addr: 内存首地址
 *
 *  输出参数  : 无
 *  返 回 值  : 无
 *
 ******************************************************************************/
int mdrv_uncached_mem_free(void* addr); 

/*****************************************************************************
 *  函 数 名  : mdrv_mem_alloc
 *  功能描述  : 分配指定属性内存
 *  输入参数  :
 *              size: 分配内存大小
 *              flag: 属性配置
 *
 *  输出参数  :
 *  返 回 值  :  NULL   分配失败。
 *               其他   已分配内存地址。
 *
 ******************************************************************************/
void* mdrv_mem_alloc(unsigned int size, unsigned int flag);

/*****************************************************************************
 *  函 数 名  : mdrv_mem_free
 *  功能描述  : 释放指定内存。
 *  输入参数  :
 *              addr: 内存首地址
 *
 *  输出参数  : 无
 *  返 回 值  : 无
 *
 ******************************************************************************/
int mdrv_mem_free(void* addr);

#ifdef __cplusplus
}
#endif
#endif
