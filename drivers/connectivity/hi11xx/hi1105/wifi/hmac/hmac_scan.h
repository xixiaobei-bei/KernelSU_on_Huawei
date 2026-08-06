

#ifndef __HMAC_SCAN_H__
#define __HMAC_SCAN_H__

#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_vap.h"
#include "mac_device.h"
#include "hmac_device.h"
#include "mac_ie.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SCAN_H

/* 扫描到的bss的老化时间，小于此值，下发新扫描请求时，不删除此bss信息 */
#define HMAC_SCAN_MAX_SCANNED_BSS_EXPIRE 25000 /* 25000 milliseconds */

/* 扫描结果中有效性高的时间范围，可用于漫游目标的挑选等操作 */
#define HMAC_SCAN_MAX_VALID_SCANNED_BSS_EXPIRE 5000 /* 5000 milliseconds */

/* 扫描到的bss的rssi老化时间 */
#define HMAC_SCAN_MAX_SCANNED_RSSI_EXPIRE 400       /* 400 milliseconds */

#define HMAC_INIT_SCAN_TIMEOUT_MS 10000
/* 扫描到的bss的rssi信号门限，门限以下不考虑对obss的影响 */
#define HMAC_OBSS_RSSI_TH (-85)

/* 使能、禁止扫描的调试命令 */
typedef enum {
    HMAC_BGSCAN_DISABLE = 0,
    HMAC_BGSCAN_ENABLE = 1,
    HMAC_SCAN_DISABLE = 2,

    HMAC_BUTT
} hmac_scan_state_enum;

typedef enum {
    MAC_TRY_INIT_SCAN_VAP_UP,
    MAC_TRY_INIT_SCAN_SET_CHANNEL,
    MAC_TRY_INIT_SCAN_START_DBAC,
    MAC_TRY_INIT_SCAN_RESCAN,

    MAC_TRY_INIT_SCAN_BUTT
} mac_try_init_scan_type;

extern uint32_t g_pd_bss_expire_time;

/* HMAC SCAN对外接口 */
void hmac_scan_print_scanned_bss_info(uint8_t device_id);

mac_bss_dscr_stru *hmac_scan_find_scanned_bss_dscr_by_index(uint8_t device_id, uint32_t bss_index);
hmac_scanned_bss_info *hmac_scan_find_scanned_bss_by_bssid(hmac_bss_mgmt_stru *bss_mgmt, uint8_t *bssid);
void *hmac_scan_get_scanned_bss_by_bssid(mac_vap_stru *mac_vap, uint8_t *mac_addr);
void hmac_scan_clean_scan(hmac_scan_stru *scan);
uint32_t hmac_scan_proc_scanned_bss(frw_event_mem_stru *event_mem);
uint32_t hmac_scan_proc_scan_comp_event(frw_event_mem_stru *event_mem);
uint32_t hmac_scan_proc_scan_req_event_exception(hmac_vap_stru *hmac_vap, void *params);
uint32_t hmac_scan_proc_scan_req_event(hmac_vap_stru *pst_hmac_vap, void *params);
uint32_t hmac_scan_proc_sched_scan_req_event(hmac_vap_stru *hmac_vap, void *params);
void hmac_scan_init(hmac_device_stru *hmac_device);
void hmac_scan_exit(hmac_device_stru *hmac_device);
uint32_t hmac_scan_process_chan_result_event(frw_event_mem_stru *event_mem);
uint32_t hmac_bgscan_enable(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_init_scan_cancel_timer(hmac_device_stru *hmac_dev);
uint32_t hmac_scan_start_dbac(mac_device_stru *dev);

void hmac_scan_set_sour_mac_addr_in_probe_req(hmac_vap_stru *hmac_vap,
                                              uint8_t *sour_mac_addr,
                                              oal_bool_enum_uint8 en_is_rand_mac_addr_scan,
                                              oal_bool_enum_uint8 en_is_p2p0_scan);

uint32_t  hmac_scan_check_is_dispatch_scan_req(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device);
uint32_t hmac_scan_proc_scan_result_handle(hmac_vap_stru *hmac_vap,
                                           oal_netbuf_stru *netbuf,
                                           uint16_t buf_len,
                                           mac_multi_bssid_frame_info_stru *mbss_frame_info);
void hmac_scan_param_convert_ex(mac_scan_req_ex_stru *scan_req_h2d_ex, mac_scan_req_stru *scan_params);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_scan.h */
