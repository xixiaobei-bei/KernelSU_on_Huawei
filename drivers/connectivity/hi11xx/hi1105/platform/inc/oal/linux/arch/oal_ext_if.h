

#ifndef __OAL_LINUX_EXT_IF_H__
#define __OAL_LINUX_EXT_IF_H__

#include "platform_oneimage_define.h"

/* 宏定义 */
/* 用此宏修饰的函数，在函数入口和出口处，编译器不会插入调用__cyg_profile_func_enter和__cyg_profile_func_exit的代码 */
#if defined(_PRE_PC_LINT)
#define ATTR_OAL_NO_FUNC_TRACE
#elif (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define ATTR_OAL_NO_FUNC_TRACE __attribute__((__no_instrument_function__))
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
#define oal_reinit_completion(x) reinit_completion(&x)
#define IRQF_DISABLED      0
#endif

/* 全局变量声明 */
extern oal_mempool_info_to_sdt_stru g_mempool_info;

/* OTHERS定义 */
typedef uint32_t (*oal_stats_info_up_to_sdt)(uint8_t en_pool_id,
                                             uint16_t us_mem_total_cnt,
                                             uint16_t us_mem_used_cnt,
                                             uint8_t uc_subpool_id,
                                             uint16_t us_total_cnt,
                                             uint16_t us_free_cnt);
typedef uint32_t (*oal_memblock_info_up_to_sdt)(uint8_t *puc_origin_data,
                                                uint8_t uc_user_cnt,
                                                oal_mem_pool_id_enum_uint8 en_pool_id,
                                                uint8_t uc_subpool_id,
                                                uint16_t us_len,
                                                uint32_t ul_file_id,
                                                uint32_t ul_alloc_line_num);

/* 函数声明 */
extern uint32_t oal_mempool_info_to_sdt_register(oal_stats_info_up_to_sdt p_up_mempool_info,
                                                 oal_memblock_info_up_to_sdt p_up_memblock_info);

#endif /* end of oal_ext_if.h */
