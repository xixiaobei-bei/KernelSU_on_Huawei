

#ifndef __HMAC_DSCR_TH_OPT_H__
#define __HMAC_DSCR_TH_OPT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "hmac_device.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DSCR_TH_OPT_H
/* 2 宏定义 */
#define HMAC_LARGE_DSCR_TH_LEVEL  6                            /* 大包描述符门限数组大小 */
#define HMAC_SMALL_DSCR_TH_LEVEL  6                            /* 小包描述符门限数组大小 */
#define HMAC_LARGE_DSCR_PPS_LEVEL HMAC_LARGE_DSCR_TH_LEVEL - 1 /* 大包trx PPS差值数组大小 */
#define HMAC_SMALL_DSCR_PPS_LEVEL HMAC_SMALL_DSCR_TH_LEVEL - 1 /* 小包trx PPS差值数组大小 */

/* 3 枚举定义 */
/* 4 全局变量声明 */
extern oal_uint32 g_ul_large_start_th;
extern oal_uint32 g_ul_large_interval;
extern oal_uint32 g_ul_small_start_th;
extern oal_uint32 g_ul_small_interval;

/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
extern oal_void hmac_rx_dscr_th_opt(oal_uint32 ul_tx_throughput_mbps,
                                    oal_uint32 ul_rx_throughput_mbps,
                                    oal_uint32 ul_tx_pps,
                                    oal_uint32 ul_rx_pps);
extern oal_void hmac_rx_dscr_th_init(hmac_device_stru *pst_hmac_device);
extern oal_void hmac_rx_dscr_th_deinit(hmac_device_stru *pst_hmac_device);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
