

#ifndef __HMAC_MAIN_H__
#define __HMAC_MAIN_H__

/* 1 其他头文件包含 */
#include "oam_ext_if.h"
#include "hmac_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MAIN_H

/* 填写HMAC到DMAC配置同步消息头 */
#define HMAC_INIT_SYN_MSG_HDR(_pst_syn_msg, _en_syn_id, _us_len) \
    do {                                                         \
        (_pst_syn_msg)->en_syn_id = (_en_syn_id);                \
        (_pst_syn_msg)->us_len = (_us_len);                      \
    } while (0)

/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
typedef struct {
    uint32_t time_stamp;
} hmac_timeout_event_stru;

typedef struct {
    uint32_t cfg_id;
    uint32_t ac;
    uint32_t value;
} hmac_config_wmm_para_stru;

typedef struct {
    struct semaphore st_rxdata_sema;
    oal_task_stru *pst_rxdata_thread;
    oal_spin_lock_stru st_lock;
    oal_wait_queue_head_stru st_rxdata_wq;
    oal_netbuf_head_stru st_rxdata_netbuf_head;
    uint32_t pkt_loss_cnt;
    oal_bool_enum_uint8 en_rxthread_enable;
    uint8_t uc_allowed_cpus;
} hmac_rxdata_thread_stru;

#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
typedef struct {
    uint32_t max_cpu_freq;
    uint32_t valid;
}hisi_max_cpu_freq;
#endif
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
uint32_t hmac_event_fsm_register(void);
void hmac_main_exit(void);
int32_t hmac_main_init(void);

uint32_t hmac_sdt_up_reg_val(frw_event_mem_stru *pst_event_mem);

#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
uint32_t hmac_sdt_recv_sample_cmd(mac_vap_stru *pst_mac_vap, uint8_t *puc_buf,
                                  uint16_t us_len);
uint32_t hmac_sdt_up_sample_data(frw_event_mem_stru *pst_event_mem);

#endif
uint32_t hmac_sdt_up_dpd_data(frw_event_mem_stru *pst_event_mem);
#ifdef _PRE_WLAN_FEATURE_APF
uint32_t hmac_apf_program_report_event(frw_event_mem_stru *pst_event_mem);
#endif

void hmac_rxdata_netbuf_enqueue(oal_netbuf_stru *pst_netbuf);
void hmac_rxdata_sched(void);
oal_bool_enum_uint8 hmac_get_rxthread_enable(void);

uint32_t hmac_proc_query_response_event(mac_vap_stru *pst_mac_vap, uint8_t uc_len,
                                        uint8_t *puc_param);
int32_t hmac_rxdata_polling(struct napi_struct *pst_napi, int32_t l_data);
hmac_rxdata_thread_stru *hmac_get_rxdata_thread_addr(void);

extern oal_wakelock_stru g_st_hmac_wakelock;
#define hmac_wake_lock()   oal_wake_lock(&g_st_hmac_wakelock)
#define hmac_wake_unlock() oal_wake_unlock(&g_st_hmac_wakelock)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_main */
