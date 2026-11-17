

#ifndef __WAL_EXT_IF_H__
#define __WAL_EXT_IF_H__

/* 1 其他头文件包含 */
#include "oal_types.h"
#include "oal_schedule.h"
#include "wlan_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_EXT_IF_H

/* 2 宏定义 */
#define WAL_SDT_MEM_MAX_LEN 32 /* SDT读写内存的最大长度 */

/* 3 枚举定义 */
/* offload时，表示哪一个核的枚举 */
typedef enum {
    WAL_OFFLOAD_CORE_MODE_HMAC,
    WAL_OFFLOAD_CORE_MODE_DMAC,

    WAL_OFFLOAD_CORE_MODE_BUTT
} wal_offload_core_mode_enum;
typedef uint8_t wal_offload_core_mode_enum_uint8;
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
/* SDT上传接口的注册函数类型 */
typedef int32_t (*wal_sdt_up_func)(oal_netbuf_stru *pst_netbuf);

/*lint -e958*/ /*lint -e959*/
/* 读取和设置全局变量的结构体 */
#define WAL_GLB_VAR_NAME_LEN  31
#define WAL_GLB_VAR_VAL_LEN  128
typedef struct {
    uint8_t uc_mode; /* 读写模式 */
    uint16_t us_len; /* 读写长度 */
    uint8_t en_offload_core_mode;
    uint8_t auc_global_value_name[WAL_GLB_VAR_NAME_LEN + 1];
    uint8_t auc_global_value[WAL_GLB_VAR_VAL_LEN];
} wal_sdt_global_var_stru;
/*lint -e958*/ /*lint -e959*/
/* 读取和设置寄存器的结构体 */
typedef struct {
    uint8_t auc_vapname[MAC_NET_DEVICE_NAME_LENGTH];  /* vap名称  16  */
    uint32_t addr;
    uint16_t us_len;
    uint8_t uc_mode;
    uint8_t auc_resv[1];
    uint32_t reg_val;
} wal_sdt_reg_frame_stru;

/* 读取和设置内存的结构体 */
typedef struct {
    uintptr_t addr;
    uint16_t us_len;
    uint8_t uc_mode;
    wal_offload_core_mode_enum_uint8 en_offload_core_mode;
    uint8_t auc_data[WAL_SDT_MEM_MAX_LEN];
} wal_sdt_mem_frame_stru;

/* 维测，设置某一种具体event上报开关的参数结构体 */
typedef struct {
    int32_t l_event_type;
    int32_t l_param;
} wal_specific_event_type_param_stru;

/* 8 UNION定义 */
/* 9 OTHERS定义 */
extern uint8_t g_custom_cali_done; /* 标识是否是上电第一次校准 */

/* 10 函数声明 */
int32_t wal_main_init(void);
void wal_main_exit(void);
int32_t wal_recv_memory_cmd(uint8_t *puc_buf, uint16_t us_len);
int32_t wal_recv_global_var_cmd(uint8_t *puc_buf, uint16_t us_len);
int32_t wal_recv_reg_cmd(uint8_t *puc_buf, uint16_t us_len);
int32_t wal_recv_config_cmd(uint8_t *puc_buf, uint16_t us_len);

uint32_t wal_hipriv_get_mac_addr(int8_t *pc_param,
                                 uint8_t auc_mac_addr[],
                                 uint32_t *pul_total_offset);

uint32_t wal_hipriv_register_inetaddr_notifier(void);
uint32_t wal_hipriv_unregister_inetaddr_notifier(void);
uint32_t wal_hipriv_register_inet6addr_notifier(void);
uint32_t wal_hipriv_unregister_inet6addr_notifier(void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wal_ext_if.h */
