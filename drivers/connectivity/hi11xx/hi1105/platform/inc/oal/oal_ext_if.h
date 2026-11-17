

#ifndef __OAL_EXT_IF_H__
#define __OAL_EXT_IF_H__

/* 其他头文件包含 */
#include "oal_types.h"
#include "oal_util.h"
#include "oal_hardware.h"
#include "oal_schedule.h"
#include "oal_bus_if.h"
#include "oal_net.h"
#include "oal_list.h"
#include "oal_queue.h"
#include "oal_workqueue.h"
#include "arch/oal_ext_if.h"
#include "oal_thread.h"
#include "oal_aes.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_exception_rst.h"
#endif
#include "oal_fsm.h"

/* 宏定义 */
#define basic_value_is_hex(_auc_str) (((_auc_str)[0] == '0') && (((_auc_str)[1] == 'x') || ((_auc_str)[1] == 'X')))
/* 枚举定义 */
typedef enum {
    OAL_TRACE_ENTER_FUNC,
    OAL_TRACE_EXIT_FUNC,

    OAL_TRACE_DIRECT_BUTT
} oal_trace_direction_enum;
typedef uint8_t oal_trace_direction_enum_uint8;

/* 黑名单模式 */
typedef enum {
    CS_BLACKLIST_MODE_NONE,  /* 关闭         */
    CS_BLACKLIST_MODE_BLACK, /* 黑名单       */
    CS_BLACKLIST_MODE_WHITE, /* 白名单       */

    CS_BLACKLIST_MODE_BUTT
} cs_blacklist_mode_enum;
typedef uint8_t cs_blacklist_mode_enum_uint8;

#ifdef _PRE_WLAN_REPORT_WIFI_ABNORMAL
// 上报驱动异常状态，并期望上层解决
typedef enum {
    OAL_ABNORMAL_FRW_TIMER_BROKEN = 0,  // frw定时器断链
    OAL_ABNORMAL_OTHER = 1,             // 其他异常

    OAL_ABNORMAL_BUTT
} oal_wifi_abnormal_reason_enum;

typedef enum {
    OAL_ACTION_RESTART_VAP = 0,  // 重启vap，上层暂未实现
    OAL_ACTION_REBOOT = 1,       // 重启板子

    OAL_ACTION_BUTT
} oal_product_action_enum;

#define oal_report_wifi_abnormal(_l_reason, _l_action, _p_arg, _l_size)
#endif

typedef enum {
    OAL_WIFI_STA_LEAVE = 0,  // STA 离开
    OAL_WIFI_STA_JOIN = 1,   // STA 加入

    OAL_WIFI_BUTT
} oal_wifi_sta_action_report_enum;
#define oal_wifi_report_sta_action(_ul_ifindex, _ul_eventID, _p_arg, _l_size)

#define OAL_WIFI_COMPILE_OPT_CNT 10
typedef struct {
    uint8_t feature_m2s_modem_is_open : 1;
    uint8_t feature_priv_closed_is_open : 1;
    uint8_t feature_hiex_is_open : 1;
    uint8_t rx_listen_ps_is_open : 1;
    uint8_t feature_11ax_is_open : 1;
    uint8_t feature_twt_is_open : 1;
    uint8_t feature_11ax_er_su_dcm_is_open : 1;
    uint8_t feature_11ax_uora_is_supported : 1;

    uint8_t feature_ftm_is_open : 1;
    uint8_t feature_psm_dfx_ext_is_open : 1;
    uint8_t feature_wow_opt_is_open : 1; // 06使用
    uint8_t feature_dual_wlan_is_supported : 1;
    uint8_t feature_ht_self_cure_is_open : 1; /* 05解决 ht sig_len 问题 */
    uint8_t feature_slave_ax_is_support : 1;
    uint8_t longer_than_16_he_sigb_support : 1;

    uint32_t p2p_go_max_user_num;   /* P2P GO最大关联用户数 */
    uint32_t max_asoc_user;         /* 1个chip支持的最大关联用户数 */
    uint32_t max_active_user;       /* 1个chip支持的最大激活用户数 */
    uint32_t max_user_limit;        /* board资源最大用户数，包括单播和组播用户 */
    uint32_t invalid_user_id;       /* 与tx dscr和CB字段,申请user idx三者同时对应,无效user id取全board最大用户LIMIT */
    uint32_t max_tx_ba;             /* 支持的建立tx ba 的最大个数 */
    uint32_t max_rx_ba;             /* 支持的建立rx ba 的最大个数 */
    uint32_t other_bss_id;          /* hal other bss id */
    uint32_t max_tx_ampdu_num;      /* tx聚合数上限 */

    uint8_t max_rf_num; /* 芯片最大天线数 */
    uint8_t feature_txq_ns_support : 1; /* 支持网络切片队列 */
    uint8_t resv1 : 7;
    uint8_t resv[2];
} oal_wlan_cfg_stru;

/* 全局变量声明 */
#ifndef _PRE_LINUX_TEST
extern const oal_wlan_cfg_stru *g_wlan_spec_cfg;
#else
extern oal_wlan_cfg_stru *g_wlan_spec_cfg;
#endif
/* 函数声明 */
extern int32_t oal_main_init(void);
extern void oal_main_exit(void);
extern uint8_t oal_chip_get_device_num(uint32_t ul_chip_ver);
extern uint8_t oal_board_get_service_vap_start_id(void);

#endif /* end of oal_ext_if.h */
