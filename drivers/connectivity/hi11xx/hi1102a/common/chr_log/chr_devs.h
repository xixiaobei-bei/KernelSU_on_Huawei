

#ifndef __CHR_DEVS_H__
#define __CHR_DEVS_H__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/

#include <linux/debugfs.h>
#include <linux/skbuff.h>

/*****************************************************************************
  2 CHR性能配置
*****************************************************************************/

/*****************************************************************************
  3 宏定义
*****************************************************************************/
#define CHR_DEV_KMSG_PLAT "chrKmsgPlat"

#define CHR_LOG_ENABLE          (1)
#define CHR_LOG_DISABLE         (0)
#define CHR_ERRNO_QUEUE_MAX_LEN (20)
#define CHR_DEV_FRAME_START     (0x7E)
#define CHR_DEV_FRAME_END       (0x7E)
#define CHR_READ_SEMA           (1)

#ifdef CHR_DEBUG
#define CHR_DBG(s, args...)                                                 \
    do {                                                                    \
        /*lint -e515*/                                                      \
        /*lint -e516*/                                                      \
        printk(KERN_DEBUG KBUILD_MODNAME ":D]chr %s]" s, __func__, ##args); \
        /*lint +e515*/                                                      \
        /*lint +e516*/                                                      \
    } while (0)
#else
#define CHR_DBG(s, args...)
#endif
#define CHR_ERR(s, args...)                                               \
    do {                                                                  \
        /*lint -e515*/                                                    \
        /*lint -e516*/                                                    \
        printk(KERN_ERR KBUILD_MODNAME ":E]chr %s]" s, __func__, ##args); \
        /*lint +e515*/                                                    \
        /*lint +e516*/                                                    \
    } while (0)
#define CHR_WARNING(s, args...)                                               \
    do {                                                                      \
        /*lint -e515*/                                                        \
        /*lint -e516*/                                                        \
        printk(KERN_WARNING KBUILD_MODNAME ":W]chr %s]" s, __func__, ##args); \
        /*lint +e515*/                                                        \
        /*lint +e516*/                                                        \
    } while (0)
#define CHR_INFO(s, args...)                                                \
    do {                                                                    \
        /*lint -e515*/                                                      \
        /*lint -e516*/                                                      \
        printk(KERN_DEBUG KBUILD_MODNAME ":I]chr %s]" s, __func__, ##args); \
        /*lint +e515*/                                                      \
        /*lint +e516*/                                                      \
    } while (0)

#define CHR_MAGIC          'C'
#define CHR_MAX_NR         2
#define chr_ERRNO_WRITE_NR 1
#define CHR_ERRNO_WRITE    _IOW(CHR_MAGIC, 1, int32)
#define CHR_ERRNO_ASK      _IOW(CHR_MAGIC, 2, int32)

#define CHR_ID_MSK 1000000
#define CHR_HOST   1
#define CHR_DEVICE 0
#define CHR_HAL_RX_QUEUE_NUM 3
#define CHR_HAL_TX_RATE_MAX_NUM 4
#define CHR_HAL_TX_QUEUE_NUM 5
#define CHR_WLAN_WME_AC_BUTT 5
#define CHR_WLAN_TID_MAX_NUM 8
#define CHR_WIFI_DISCONNECT_EVENTID 909002022
#define CHR_WIFI_WEB_SLOW_EVENTID 909002025

enum CHR_ID_ENUM {
    CHR_WIFI = 909,
    CHR_BT = 913,
    CHR_GNSS = 910,
    CHR_ENUM
};

/*****************************************************************************
  3 数据类型定义
*****************************************************************************/
typedef unsigned char uint8;
typedef char int8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned long uint64;
typedef long int64;

/*****************************************************************************
  4 枚举类型定义
*****************************************************************************/

enum return_type {
    CHR_SUCC = 0,
    CHR_EFAIL,
};

/*****************************************************************************
  5 结构体定义
*****************************************************************************/
typedef struct {
    wait_queue_head_t errno_wait;
    struct sk_buff_head errno_queue;
    struct semaphore errno_sem;
} CHR_EVENT;
typedef struct {
    uint8 framehead;
    uint8 reserved[3];
    uint32 error;
    uint8 frametail;
} CHR_DEV_EXCEPTION_STRU;

typedef struct {
    uint32 errno;
    uint16 errlen;
    uint16 flag : 1;
    uint16 resv : 15;

} CHR_DEV_EXCEPTION_STRU_PARA;

typedef struct {
    uint32 chr_errno;
    uint16 chr_len;
    uint8 *chr_ptr;
} CHR_HOST_EXCEPTION_STRU;

typedef struct {
    char ac_fw_ver[20];
    char ac_ko_ver[20];
    char ac_dieid[20];
    uint8 en_p2p_mode;
    int16 s_cur_temp_state;
    uint8 resv[2];
} chr_dmac_common_info_stru;

typedef struct {
    uint32 ul_rx_rate;
    uint16 us_legacy;
    uint8 uc_mac_rate;
    uint8 uc_phy_rate;
    uint8 uc_mbps;
    uint8 uc_flags;
    uint8 uc_mcs;
    uint8 uc_nss;
    uint8 uc_bw;
    uint8 resv[3];
} chr_dmac_rate_info_stru;

typedef struct {
    int16      s_ant0_rssi;
    int16      s_free_power;
} chr_dmac_radio_link_quality_info_stru;

typedef struct {
    uint32 ul_tkipccmp_rep_fail_cnt;
    uint32 ul_tx_mpdu_cnt;
    uint32 ul_rx_passed_mpdu_cnt;
    uint32 ul_rx_failed_mpdu_cnt;
    uint32 ul_rx_tkipccmp_mic_fail_cnt;
    uint32 ul_key_search_fail_cnt;
    uint32 ul_phy_rx_dotb_ok_frm_cnt;
    uint32 ul_phy_rx_htvht_ok_frm_cnt;
    uint32 ul_phy_rx_lega_ok_frm_cnt;
    uint32 ul_phy_rx_dotb_err_frm_cnt;
    uint32 ul_phy_rx_htvht_err_frm_cnt;
    uint32 ul_phy_rx_lega_err_frm_cnt;
} hal_mac_key_statis_info;

typedef struct {
    hal_mac_key_statis_info st_mac_key_statis;
    uint32 ul_rx_normal_mdpu_succ_num;
    uint32 ul_rx_ampdu_succ_num;
    uint32 ul_tx_ppdu_succ_num;
    uint32 ul_rx_ppdu_fail_num;
    uint32 ul_tx_ppdu_fail_num;
} chr_dmac_chip_count_info_stru;

typedef struct {
    uint16 aus_rx_ppdu_num[CHR_HAL_RX_QUEUE_NUM];
    uint16 us_tx_free_ppdu_cnt;
    uint8 auc_tx_ppdu_num[CHR_HAL_TX_QUEUE_NUM];
    uint8 resv[3];
    uint16 aus_tx_mpdu_num[CHR_WLAN_TID_MAX_NUM];
} chr_dmac_queue_info_stru;

typedef struct {
    uint8 auc_txbf_mode[CHR_HAL_TX_RATE_MAX_NUM];
    uint8 auc_stbc_mode[CHR_HAL_TX_RATE_MAX_NUM];
    uint8 auc_tx_chain[CHR_HAL_TX_RATE_MAX_NUM];
    uint8 auc_phy_mode[CHR_HAL_TX_RATE_MAX_NUM];
    uint8 auc_edca_cwmax[CHR_HAL_TX_RATE_MAX_NUM];
    uint8 auc_edca_cwmin[CHR_HAL_TX_RATE_MAX_NUM];
    uint8 en_bandwidth;
    int8  ac_acc_limit[CHR_WLAN_WME_AC_BUTT];
    uint8 uc_ldpc_implement;
    uint8 uc_ldpc_activate;
} chr_dmac_send_mode_info_stru;

typedef struct {
    uint32 ul_tx_pkts_total;
    uint32 ul_tx_dropped;
    uint32 ul_tx_pkts_succ;
    uint32 ul_total_retry;
    uint32 ul_hw_tx_fail;
    uint32 ul_rx_replay;
    uint32 ul_rx_replay_droped;
    int32  l_rssi;
    uint32 ul_rx_total_pkts;
    uint32 ul_rx_drv_dropped;
    uint32 ul_hcc_flowctrl_miss;
    uint32 ul_txrate_legacy;
    uint32 ul_txrate_mcs;
    uint32 ul_txrate_flags;
    uint32 ul_txrate_nss;
    uint32 ul_rxrate_kbps;
} chr_dmac_txrx_pkts_stru;

typedef struct {
    uint32 ul_pm_on;
    uint32 ul_beacon_period;
    uint32 ul_dtim_period;
    uint32 ul_cpu_freq_level;
    uint8 bit_pm_more_data_expected: 1;
    uint8 bit_11k_enable : 1;
    uint8 bit_11v_enable : 1;
    uint8 bit_11r_enable : 1;
    uint8 bit_ap_11k_enable : 1;
    uint8 bit_ap_11v_enable : 1;
    uint8 bit_ap_11r_enable : 1;
    uint8 bit_resv : 1;
} chr_dmac_pm_info_stru;

typedef struct {
    CHR_DEV_EXCEPTION_STRU_PARA st_dev_exception_info;
    chr_dmac_common_info_stru st_common_info;
    chr_dmac_rate_info_stru st_chr_rate_info;
    chr_dmac_radio_link_quality_info_stru st_radio_link_quality_info;
    chr_dmac_chip_count_info_stru st_chip_count_info;
    chr_dmac_queue_info_stru st_queue_info;
    chr_dmac_send_mode_info_stru st_send_mode_info;
    chr_dmac_txrx_pkts_stru st_txrx_pkts;
    chr_dmac_pm_info_stru st_pm_info;
} chr_dmac_info_stru;

typedef struct {
    char ac_fw_ver[50];
    char ac_ko_ver[20];
    char ac_dieid[20];
    int16 s_cur_temp_state;
    char resv[2];
} chr_hmac_common_info_stru;

typedef struct {
    uint32 ul_rx_rate;
    uint16 us_legacy;
    uint8 uc_mac_rate;
    uint8 uc_phy_rate;
    uint8 uc_mbps;
    uint8 uc_flags;
    uint8 uc_mcs;
    uint8 uc_nss;
    uint8 uc_bw;
    uint8 resv[3];
} chr_hmac_rate_info_stru;

typedef struct {
    int16 s_ant0_rssi;
    int16 s_ant1_rssi;
    uint32 ul_duty_ratio;
    int8 c_ant0_snr;
    int8 c_chip_type;
    uint8 en_alg_distance_stat;
    uint8 en_adj_intf_state;
    uint8 en_co_intf_state;
    uint8 uc_bw;
    int16 s_noise;
    uint16 us_chload;
    uint8 auc_resv[2];
} chr_hmac_radio_link_quality_info_stru;

typedef struct {
    hal_mac_key_statis_info st_mac_key_statis;
    uint32 ul_rx_normal_mdpu_succ_num;
    uint32 ul_rx_ampdu_succ_num;
    uint32 ul_tx_ppdu_succ_num;
    uint32 ul_rx_ppdu_fail_num;
    uint32 ul_tx_ppdu_fail_num;
} chr_hmac_chip_count_info_stru;

typedef struct {
    uint8 en_sta_csa_fsm_cur_sta;
    uint8 en_sta_csa_fsm_pre_sta;
    uint8 en_bw_switch_fsm_cur_sta;
    uint8 en_bw_switch_fsm_pre_sta;
    uint8 en_hal_dev_fsm_cur_sta;
    uint8 en_hal_dev_fsm_pre_sta;
    uint8 en_hal_dev_sub_fsm_sta;
    uint8 en_hal_m2s_fsm_cur_sta;
    uint8 en_hal_m2s_fsm_pre_sta;
    uint8 en_m2s_type;
    uint8 st_m2s_mode;
    uint8 en_nss_num;
    uint8 uc_phy_chain;
    uint8 uc_single_tx_chain;
    uint8 uc_rf_chain;
    uint8 uc_phy2dscr_chain;
} chr_hmac_fsm_sta_stru;

typedef struct {
    uint16 aus_rx_ppdu_num[CHR_HAL_RX_QUEUE_NUM];
    uint8 auc_rx_free_ppdu_num[CHR_HAL_RX_QUEUE_NUM];
    uint8 auc_tx_ppdu_num[CHR_HAL_TX_QUEUE_NUM];
    uint8 auc_tx_ppdu_num_resv[1];
    uint16 us_tx_free_ppdu_cnt;
    uint16 aus_tx_mpdu_num[CHR_WLAN_TID_MAX_NUM];
    uint8 resv[3];
} chr_hmac_queue_info_stru;

typedef struct {
    uint8 auc_txbf_mode[CHR_HAL_TX_RATE_MAX_NUM];
    uint8 auc_stbc_mode[CHR_HAL_TX_RATE_MAX_NUM];
    uint8 auc_tx_ant[CHR_HAL_TX_RATE_MAX_NUM];
    uint8 auc_tx_chain[CHR_HAL_TX_RATE_MAX_NUM];
    uint8 auc_phy_mode[CHR_HAL_TX_RATE_MAX_NUM];
    uint8 auc_edca_cwmax[CHR_HAL_TX_RATE_MAX_NUM];
    uint8 auc_edca_cwmin[CHR_HAL_TX_RATE_MAX_NUM];
    uint8 en_bandwidth;
    int8  ac_acc_limit[CHR_WLAN_WME_AC_BUTT];
    uint8 uc_ldpc_implement;
    uint8 uc_ldpc_activate;
    uint8 uc_ldpc_vht_implement;
    uint8 resv[3];
} chr_hmac_send_mode_info_stru;

typedef struct {
    uint8 auc_mngpkt_sendstat[12];
} chr_hmac_mngpkt_send_stat_stru;

typedef struct {
    uint32 ul_phy_err_rpt;
    uint32 ul_rx_desc_status;
    uint32 ul_rpt_aver_evm_ant1;
    uint32 ul_rpt_aver_evm_ant0;
    uint32 ul_dotb_err_frm_num;
    uint32 ul_ht_err_frm_num;
    uint32 ul_vht_err_frm_num;
    uint32 ul_lega_err_frm_num;
    uint32 ul_rpt_phase_est;
    uint32 ul_rpt_2x2_chan_cond;
    uint32 ul_rpt_mac_auto_rst_cnt;
} chr_hmac_hal_phy_mac_info_stru;

typedef struct {
    uint16  bit_bt_on : 1,
            bit_bt_cali : 1,
            bit_bt_ps : 1,
            bit_bt_inquiry : 1,
            bit_bt_page : 1,
            bit_bt_acl : 1,
            bit_bt_a2dp : 1,
            bit_bt_sco : 1,
            bit_bt_data_trans : 1,
            bit_bt_acl_num : 3,
            bit_bt_link_role : 4;
} bt_status_stru;

typedef struct {
    uint16  bit_ble_on : 1,
            bit_ble_scan : 1,
            bit_ble_con : 1,
            bit_ble_adv : 1,
            bit_bt_transfer : 1,
            bit_bt_6slot : 2,
            bit_ble_init : 1,
            bit_bt_acl : 1,
            bit_bt_ldac : 1,
            bit_resv1 : 1,
            bit_bt_hid : 1,
            bit_ble_hid : 1,
            bit_resv2 : 1,
            bit_sco_notify : 1,
            bit_bt_ba : 1;
} btcoex_ble_status_stru;

typedef struct {
    uint8   bit_ps_on : 1,
            bit_delba_on : 1,
            bit_reply_cts : 1,
            bit_rsp_frame_ps : 1,
            bit_btcoex_wl0_tx_slv_on : 1,
            bit_resv : 3;
} hal_coex_sw_preempt_mode_stru;

typedef struct {
    bt_status_stru st_bt_status;
    btcoex_ble_status_stru st_ble_status;
    hal_coex_sw_preempt_mode_stru st_sw_preemt_mode;
} chr_hmac_btcoex_status_stru;

typedef struct {
    uint32 ul_tx_pkts_total;
    uint32 ul_tx_dropped;
    uint32 ul_tx_pkts_succ;
    uint32 ul_total_retry;
    uint32 ul_hw_tx_fail;
    uint32 ul_rx_replay;
    uint32 ul_rx_replay_droped;
    uint32 ul_rx_total_pkts;
    uint32 ul_rx_drv_dropped;
    uint32 ul_hcc_flowctrl_miss;
    uint32 ul_txrate_legacy;
    uint32 ul_txrate_mcs;
    uint32 ul_txrate_flags;
    uint32 ul_txrate_nss;
    uint32 ul_rxrate_kbps;
} chr_hmac_txrx_pkts_stru;

typedef struct {
    uint32 ul_pm_on;
    uint32 ul_beacon_period;
    uint32 ul_dtim_period;
    uint32 ul_duty_ratio;
    uint32 ul_tx_ratio_lp;
    uint32 ul_cpu_freq_level;
    uint32 ul_pm_activity_timeout;
    uint8 uc_pm_ps_mode;
    uint8 uc_pm_cur_state;
    uint8 uc_pm_pre_state;
    uint8 ul_pm_more_data_expected;
    uint8 uc_tx_chain;
    uint8 uc_rx_chain;
    uint8 bit_pm_more_data_expected : 1;
    uint8 bit_11k_enable : 1;
    uint8 bit_11v_enable : 1;
    uint8 bit_11r_enable : 1;
    uint8 bit_ap_11k_enable : 1;
    uint8 bit_ap_11v_enable : 1;
    uint8 bit_ap_11r_enable : 1;
    uint8 bit_resv : 1;
} chr_hmac_pm_info_stru;

typedef struct {
    CHR_DEV_EXCEPTION_STRU_PARA st_dev_exception_info;
    chr_hmac_common_info_stru st_common_info;
    chr_hmac_rate_info_stru st_chr_rate_info;
    chr_hmac_radio_link_quality_info_stru st_radio_link_quality_info;
    chr_hmac_chip_count_info_stru st_chip_count_info;
    chr_hmac_fsm_sta_stru st_chr_fsm_sta;
    chr_hmac_queue_info_stru st_queue_info;
    chr_hmac_send_mode_info_stru st_send_mode_info;
    chr_hmac_mngpkt_send_stat_stru st_mngpkt_send_stat;
    chr_hmac_hal_phy_mac_info_stru st_hal_phy_mac_chr_info;
    chr_hmac_btcoex_status_stru st_btcoex_status;
    chr_hmac_txrx_pkts_stru st_txrx_pkts;
    chr_hmac_pm_info_stru st_pm_info;
} chr_hmac_info_stru;

/* 函数声明 */
#ifdef CONFIG_HI1102_PLAT_HW_CHR
extern int chr_miscdevs_init(void);
extern void chr_miscdevs_exit(void);
#endif
#endif

