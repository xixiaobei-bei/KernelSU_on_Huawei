

#ifndef __OAM_LINUX_NETLINK_H__
#define __OAM_LINUX_NETLINK_H__

/* 其他头文件包含 */
#include "oal_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAM_LINUX_NETLINK_H
/* 宏定义 */
#if (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_WS835DMB)
#define OAM_NETLINK_ID 29  // 1151honor835 修改成29，防止和其他产品的ko加载以及业务运行时创建的netlink产生冲突
#elif (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_E5) ||  \
      (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_CPE)
#define OAM_NETLINK_ID 26  // E5 修改成26
#elif (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_5630HERA)
#define OAM_NETLINK_ID 26  // HERA 修改成26
#else
#define OAM_NETLINK_ID 25
#endif

/* STRUCT定义 */
typedef struct {
    oal_sock_stru *pst_nlsk;
    uint32_t ul_pid;
} oam_netlink_stru;

typedef struct {
    uint32_t (*p_oam_sdt_func)(uint8_t *puc_data, uint32_t ul_len);
    uint32_t (*p_oam_hut_func)(uint8_t *puc_data, uint32_t ul_len);
    uint32_t (*p_oam_alg_func)(uint8_t *puc_data, uint32_t ul_len);
    uint32_t (*p_oam_daq_func)(uint8_t *puc_data, uint32_t ul_len);
    uint32_t (*p_oam_reg_func)(uint8_t *puc_data, uint32_t ul_len);
    uint32_t (*p_oam_acs_func)(uint8_t *puc_data, uint32_t ul_len);
    uint32_t (*p_oam_psta_func)(uint8_t *puc_data, uint32_t ul_len);
} oam_netlink_proto_ops;

/* 函数声明 */
extern uint32_t oam_netlink_kernel_create(void);
extern void oam_netlink_kernel_release(void);
#endif /* end of oam_linux_netlink.h */
