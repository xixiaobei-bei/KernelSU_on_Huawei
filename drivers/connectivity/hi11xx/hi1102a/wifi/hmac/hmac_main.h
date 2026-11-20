

#ifndef __HMAC_MAIN_H__
#define __HMAC_MAIN_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oam_ext_if.h"
#include "hmac_ext_if.h"
#include "dmac_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MAIN_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define hmac_err_log(_uc_vap_id, _puc_string)
#define hmac_err_log1(_uc_vap_id, _puc_string, _l_para1)
#define hmac_err_log2(_uc_vap_id, _puc_string, _l_para1, _l_para2)
#define hmac_err_log3(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3)
#define hmac_err_log4(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3, _l_para4)
#define hmac_err_var(_uc_vap_id, _c_fmt, ...)

#define hmac_warning_log(_uc_vap_id, _puc_string)
#define hmac_warning_log1(_uc_vap_id, _puc_string, _l_para1)
#define hmac_warning_log2(_uc_vap_id, _puc_string, _l_para1, _l_para2)
#define hmac_warning_log3(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3)
#define hmac_warning_log4(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3, _l_para4)
#define hmac_warning_var(_uc_vap_id, _c_fmt, ...)

#define hmac_info_log(_uc_vap_id, _puc_string)
#define hmac_info_log1(_uc_vap_id, _puc_string, _l_para1)
#define hmac_info_log2(_uc_vap_id, _puc_string, _l_para1, _l_para2)
#define hmac_info_log3(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3)
#define hmac_info_log4(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3, _l_para4)
#define hmac_info_var(_uc_vap_id, _c_fmt, ...)

/* 填写HMAC到DMAC配置同步消息头 */
#define hmac_init_syn_msg_hdr(_pst_syn_msg, _en_syn_id, _us_len) \
    do {                                            \
        (_pst_syn_msg)->en_syn_id = (_en_syn_id);   \
        (_pst_syn_msg)->us_len = (_us_len);   \
    } while (0)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern mac_board_stru g_st_hmac_board;

/*****************************************************************************
  5 消息头定义
*****************************************************************************/
/*****************************************************************************
  6 消息定义
*****************************************************************************/
/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    oal_uint32  ul_time_stamp;
}hmac_timeout_event_stru;

typedef struct {
    oal_uint32  ul_cfg_id;
    oal_uint32  ul_ac;
    oal_uint32  ul_value;
}hmac_config_wmm_para_stru;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
typedef struct {
    struct semaphore            st_rxdata_sema;
    oal_task_stru              *pst_rxdata_thread;
    oal_spin_lock_stru          st_lock;
    oal_wait_queue_head_stru    st_rxdata_wq;
    oal_netbuf_head_stru        st_rxdata_netbuf_head;
    oal_uint32                  ul_pkt_loss_cnt;
    oal_bool_enum_uint8         en_rxthread_enable;
}hmac_rxdata_thread_stru;
#endif

#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
typedef struct {
    oal_uint32 max_cpu_freq;
    oal_uint32 valid;
} hisi_max_cpu_freq;
#endif

/*****************************************************************************
  8 UNION定义
*****************************************************************************/
/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
/*****************************************************************************
  10 函数声明
*****************************************************************************/
#if defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
extern oal_int32 hmac_tx_event_pkts_info_print(oal_void *data, char *buf, oal_int32 buf_len);
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
extern oal_void hmac_hcc_adapt_init(oal_void);
extern oal_void hmac_hcc_adapt_deinit(oal_void);
#endif
extern oal_uint32  hmac_event_fsm_register(oal_void);
extern oal_void  hmac_main_exit(oal_void);
extern oal_int32  hmac_main_init(oal_void);
extern oal_uint32  hmac_config_send_event(
                mac_vap_stru                     *pst_mac_vap,
                wlan_cfgid_enum_uint16            en_cfg_id,
                oal_uint16                        us_len,
                oal_uint8                        *puc_param);
extern oal_uint32  hmac_sdt_up_reg_val(frw_event_mem_stru  *pst_event_mem);

#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE)
extern oal_uint32  hmac_sdt_recv_sample_cmd(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_buf, oal_uint16 us_len);
extern oal_uint32  hmac_sdt_up_sample_data(frw_event_mem_stru  *pst_event_mem);

#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
extern oal_void  hmac_rxdata_netbuf_enqueue(oal_netbuf_stru  *pst_netbuf);
extern oal_void  hmac_rxdata_sched(oal_void);
extern oal_bool_enum_uint8 hmac_get_rxthread_enable(oal_void);
#endif

extern oal_uint32  hmac_proc_query_response_event(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  hmac_config_query_chr_ext_info_rsp_event(
    mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);

#ifdef _PRE_WLAN_FEATURE_APF
extern oal_uint32  hmac_apf_program_report_event(frw_event_mem_stru  *pst_event_mem);
#endif

extern oal_wakelock_stru   g_st_hmac_wakelock;
#define hmac_wake_lock()  oal_wake_lock(&g_st_hmac_wakelock)
#define hmac_wake_unlock()  oal_wake_unlock(&g_st_hmac_wakelock)
extern oal_int32   hmac_rxdata_polling(struct napi_struct *pst_napi, oal_int32 l_weight);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_main */
