

#ifndef __HMAC_AUTO_DDR_FREQ_H__
#define __HMAC_AUTO_DDR_FREQ_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "mac_vap.h"
#include "dmac_ext_if.h"
#include "hmac_vap.h"
#include "hmac_config.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_AUTO_DDR_FREQ_H

/* 2 宏定义 */
#define HMAC_DDR_MAX_FREQ 3000000000 /* 向内核申请较大值, 确保DDR可跑到最大频率 */

/* 3 枚举定义 */
typedef enum {
    HMAC_AUTO_DDR_FREQ_ADD_REQUEST,
    HMAC_AUTO_DDR_FREQ_REMOVE_REQUEST,
    HMAC_AUTO_DDR_FREQ_RETURN,

    HMAC_AUTO_DDR_FREQ_UPDATE_BUTT,
} hmac_auto_ddr_freq_update_enum;
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
typedef struct {
    oal_uint32 ul_auto_ddr_freq_pps_th_high;     /* 使能高DDR频率的 pps阈值 */
    oal_uint32 ul_auto_ddr_freq_pps_th_low;      /* 使能低DDR频率的 pps阈值 */
    oal_bool_enum_uint8 en_ddr_freq_updated;     /* 是否已向内核申请DDR频率 */
    oal_bool_enum_uint8 en_auto_ddr_freq_switch; /* 动态DDR调频功能总开关 */
    struct pm_qos_request *pst_auto_ddr_freq;    /* DDR频率申请结构体 */
} hmac_auto_ddr_freq_mgmt_stru;

extern hmac_auto_ddr_freq_mgmt_stru g_st_auto_ddr_freq_mgmt;

/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)
extern oal_void hmac_auto_ddr_init(oal_void);
extern oal_void hmac_auto_ddr_exit(oal_void);
extern oal_void hmac_auto_set_ddr_freq(oal_uint32 ul_total_pps);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_arp_offload.h */
