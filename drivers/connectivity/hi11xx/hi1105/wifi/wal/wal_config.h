

#ifndef __WAL_CONFIG_H__
#define __WAL_CONFIG_H__

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "wlan_types.h"
#include "mac_vap.h"
#include "frw_ext_if.h"
#include "securec.h"
#include "securectype.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_CONFIG_H
/* 2 宏定义 */
typedef uint32_t (*wal_config_get_func)(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
typedef uint32_t (*wal_config_set_func)(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);

#define WAL_MSG_REQ_RESP_MEM_FREE(_st_msg_request)    \
    do {                                              \
        if (NULL != (_st_msg_request).pst_resp_mem) { \
            oal_free((_st_msg_request).pst_resp_mem); \
        }                                             \
    } while (0)

/* 填写配置消息头 */
#define WAL_CFG_MSG_HDR_INIT(_pst_cfg_msg_hdr, _en_type, _us_len, _uc_sn) \
    do {                                                                  \
        (_pst_cfg_msg_hdr)->en_msg_type = (_en_type);                     \
        (_pst_cfg_msg_hdr)->us_msg_len = (_us_len);                       \
        (_pst_cfg_msg_hdr)->uc_msg_sn = (_uc_sn);                         \
    } while (0)

/* 填写write msg消息头 */
#define WAL_WRITE_MSG_HDR_INIT(_pst_write_msg, _en_wid, _us_len) \
    do {                                                         \
        (_pst_write_msg)->en_wid = (_en_wid);                    \
        (_pst_write_msg)->us_len = (_us_len);                    \
    } while (0)
#define WAL_RECV_CMD_NEED_RESP(_pst_msg, _us_need_response)          \
    do {                                                             \
        if ((_pst_msg)->st_msg_hdr.en_msg_type == WAL_MSG_TYPE_QUERY) { \
            /* need response */                                      \
            (_us_need_response) = OAL_TRUE;                             \
        }                                                            \
    } while (0)

typedef struct {
    uintptr_t request_address;
} wal_msg_rep_hdr;

/* 获取msg序列号宏 */
extern oal_atomic g_wal_config_seq_num;
#define WAL_GET_MSG_SN() (oal_atomic_inc_return(&g_wal_config_seq_num))

#define WAL_MSG_WRITE_MSG_HDR_LENGTH (sizeof(wal_msg_hdr_stru))
#define WAL_MSG_REP_HDR_LENGTH (sizeof(wal_msg_rep_hdr))
#define WAL_MSG_WRITE_MAX_LEN \
    (WLAN_MEM_EVENT_SIZE2 - FRW_EVENT_HDR_LEN - FRW_IPC_MSG_HEADER_LENGTH - \
    (WAL_MSG_WRITE_MSG_HDR_LENGTH << BIT_OFFSET_1) -  WAL_MSG_REP_HDR_LENGTH - OAL_MEM_INFO_SIZE - OAL_DOG_TAG_SIZE)

#define WAL_ALWAYS_TX_PACK_LEN (4000 - 28) /* 1024 */
#define WAL_BCAST_MAC_ADDR     255
#define WAL_MAX_RATE_NUM       16

/* 3 枚举定义 */
/* 配置消息类型 */
typedef enum {
    WAL_MSG_TYPE_QUERY,    /* 查询 */
    WAL_MSG_TYPE_WRITE,    /* 设置 */
    WAL_MSG_TYPE_RESPONSE, /* 返回 */

    WAL_MSG_TYPE_BUTT
} wal_msg_type_enum;

/* 速率集种类，常发使用 */
typedef enum {
    WAL_RF_TEST_11B_LEGACY_RATES,
    WAL_RF_TEST_20M_NORMAL_RATES,
    WAL_RF_TEST_20M_SHORT_GI_RATES,
    WAL_RF_TEST_40M_NORMAL_RATES,
    WAL_RF_TEST_40M_SHORT_GI_RATES,

    WAL_RF_TEST_RATES_BUTT
} wal_rf_test_enum;
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
/* 配置消息头 */
typedef struct {
    uint8_t en_msg_type;               /* msg type:W or Q 枚举定义wal_msg_type_enum */
    uint8_t uc_msg_sn;                 /* msg 序列号 */
    uint16_t us_msg_len;               /* msg 长度 */
} wal_msg_hdr_stru;

/* 配置消息 */
typedef struct {
    wal_msg_hdr_stru st_msg_hdr; /* 配置消息头 */
    uint8_t auc_msg_data[4];   /* 4代表4字节对齐，配置消息payload */
} wal_msg_stru;

typedef struct {
    mac_vap_stru *pst_mac_vap;
    int8_t pc_param[4]; /* 4代表4字节对齐，查询或配置信息 */
} wal_event_stru;

/* write消息格式 */
typedef struct {
    wlan_cfgid_enum_uint16 en_wid;
    uint16_t us_len;
    uint8_t auc_value[WAL_MSG_WRITE_MAX_LEN];
} wal_msg_write_stru;

/* write消息时的返回消息 */
typedef struct {
    wlan_cfgid_enum_uint16 en_wid;
    uint8_t auc_resv[2]; // 2代表保留字节，4字节对齐
    uint32_t err_code; /* write消息返回的错误码 */
} wal_msg_write_rsp_stru;

/* response消息格式，与Write消息格式相同 */
typedef wal_msg_write_stru wal_msg_rsp_stru;

/* query消息格式:2字节WID x N */
typedef struct {
    wlan_cfgid_enum_uint16 en_wid;
} wal_msg_query_stru;

/* WID对应的操作 */
typedef struct {
    wlan_cfgid_enum_uint16 en_cfgid; /* wid枚举 */
    oal_bool_enum_uint8 en_reset;    /* 是否复位 */
    uint8_t auc_resv[1];
    wal_config_get_func p_get_func; /* get函数 */
    wal_config_set_func p_set_func; /* set函数 */
} wal_wid_op_stru;

/* WMM SET消息格式 */
typedef struct {
    wlan_cfgid_enum_uint16 en_cfg_id;
    uint8_t uc_resv[2]; // 2代表保留字节，4字节对齐
    uint32_t ac;
    uint32_t value;
} wal_msg_wmm_stru;

/* WMM query消息格式:2字节WID x N */
typedef struct {
    wlan_cfgid_enum_uint16 en_wid;
    uint8_t uc_resv[2]; // 2代表保留字节数，结构体4字节对齐
    uint32_t ac;
} wal_msg_wmm_query_stru;

/* WID request struct */
typedef struct {
    oal_dlist_head_stru pst_entry;
    uintptr_t request_address;
    void *pst_resp_mem;
    uint32_t resp_len;
    uint32_t ret;
} wal_msg_request_stru;

#define DECLARE_WAL_MSG_REQ_STRU(name) wal_msg_request_stru name;
#define WAL_MSG_REQ_STRU_INIT(name)    \
    do {                                                                             \
        memset_s ((void *)(&(name)), sizeof((name)), 0, sizeof((name))); \
        (name).request_address = (uintptr_t)&(name);                       \
    } while (0)

typedef struct {
    oal_dlist_head_stru st_head;
    oal_spin_lock_stru st_lock;
    oal_wait_queue_head_stru st_wait_queue;
    uint32_t count;
} wal_msg_queue;

/* 8 UNION定义 */
/* 9 宏定义 */
#define WAL_MSG_HDR_LENGTH sizeof(wal_msg_hdr_stru)
#define WAL_MSG_WID_LENGTH sizeof(wlan_cfgid_enum_uint16)
extern const wal_wid_op_stru g_ast_board_wid_op_debug[];

/* 10 函数声明 */
oal_net_device_stru* wal_config_get_netdev(const int8_t *pc_name, int32_t arrayLen);
uint32_t wal_config_process_pkt(frw_event_mem_stru *pst_event_mem);
uint32_t wal_config_get_wmm_params(oal_net_device_stru *pst_net_dev, uint8_t *puc_param);
uint32_t wal_config_get_sta_11h_abillty(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);

void wal_cfg_msg_task_sched(void);
int32_t wal_set_msg_response_by_addr(uintptr_t addr, void *pst_resp_mem, uint32_t resp_ret,
    uint32_t uc_rsp_len);
void wal_msg_request_add_queue(wal_msg_request_stru *pst_msg);
void wal_msg_request_remove_queue(wal_msg_request_stru *pst_msg);
void wal_msg_queue_init(void);
uint32_t wal_get_request_msg_count(void);

void wal_send_cali_matrix_data(oal_net_device_stru *pst_net_dev);
uint32_t wal_check_and_release_msg_resp(wal_msg_stru *pst_rsp_msg);
uint32_t wal_config_softap_mimo_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t wal_send_custom_data(mac_vap_stru *pst_mac_vap, uint16_t len, uint8_t *param, uint8_t syn_id);
uint32_t wal_config_get_debug_wid_arrysize(void);
uint32_t wal_config_alg_cfg_param_host_entry(mac_vap_stru *pst_mac_vap,
    uint16_t us_input_len, uint8_t *puc_input);

#endif /* end of wal_config.h */
