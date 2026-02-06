

/* 头文件包含 */
#include "hisi_customize_wifi_hi1106.h"
#ifdef HISI_CONN_NVE_SUPPORT
#include "hisi_conn_nve_interface.h"
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/etherdevice.h>
#endif

#include "hisi_customize_wifi.h"
#include "hisi_ini.h"
#include "plat_type.h"
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "oal_sdio_comm.h"
#include "oal_hcc_host_if.h"
#include "oal_main.h"
#include "hisi_ini.h"
#include "plat_pm_wlan.h"
#include "plat_firmware.h"
#include "oam_ext_if.h"

#include "wlan_spec.h"
#include "wlan_chip_i.h"

#include "mac_hiex.h"

#include "wal_config.h"

#include "hmac_auto_adjust_freq.h"
#include "hmac_cali_mgmt.h"
#include "hmac_tx_data.h"


/* 终端头文件 */
#ifdef _PRE_CONFIG_READ_DYNCALI_E2PROM
#define hisi_nve_direct_access(...)  0
#else
#include <linux/mtd/hisi_nve_interface.h>
#endif


#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HISI_CUSTOMIZE_WIFI_HI1106_C

/*  host侧使用定制化参数分成两部分: g_wlan_cust(与03/05共结构体), g_cust_host */
cust_host_cfg_stru g_cust_host = {
    .wlan_ps_mode = WLAN_PS_MODE,
    .freq_lock = {
        .throughput_irq_high = WLAN_IRQ_TH_HIGH,
        .dma_latency_val = OAL_FALSE,
        .lock_cpu_th_high = 0,
        .lock_cpu_th_low = 0,
        .en_irq_affinity = OAL_FALSE,
        .throughput_irq_low = WLAN_IRQ_TH_LOW,
        .irq_pps_high = WLAN_IRQ_PPS_TH_HIGH,
        .irq_pps_low = WLAN_IRQ_PPS_TH_LOW,
    },
    .tcp_ack_buf = {
        .buf_en = OAL_TRUE,
        .throughput_high = WLAN_TCP_ACK_BUF_HIGH,
        .throughput_low = WLAN_TCP_ACK_BUF_LOW,
        .throughput_high_40m = WLAN_TCP_ACK_BUF_HIGH_40M,
        .throughput_low_40m = WLAN_TCP_ACK_BUF_LOW_40M,
        .throughput_high_80m = WLAN_TCP_ACK_BUF_HIGH_80M,
        .throughput_low_80m = WLAN_TCP_ACK_BUF_LOW_80M,
        .throughput_high_160m = WLAN_TCP_ACK_BUF_HIGH_160M,
        .throughput_low_160m = WLAN_TCP_ACK_BUF_LOW_160M,
        .buf_userctl_test_en = OAL_FALSE,
        .buf_userctl_high = WLAN_TCP_ACK_BUF_USERCTL_HIGH,
        .buf_userctl_low = WLAN_TCP_ACK_BUF_USERCTL_LOW,
    },
    .dyn_extlna = {
        .switch_en = OAL_FALSE,
        .throughput_high = WLAN_RX_DYN_BYPASS_EXTLNA_HIGH,
        .throughput_low = WLAN_RX_DYN_BYPASS_EXTLNA_LOW,
    },
    .tx_ampdu = {
        .ampdu_hw_en = OAL_FALSE,
        .throughput_high = WLAN_HW_AMPDU_TH_HIGH,
        .throughput_low = WLAN_HW_AMPDU_TH_LOW,
    },
    .tcp_ack_filt = {
        .filter_en = OAL_FALSE,
        .throughput_high = WLAN_TCP_ACK_FILTER_TH_HIGH,
        .throughput_low = WLAN_TCP_ACK_FILTER_TH_LOW,
    },
    .host_rx_ampdu_amsdu = OAL_FALSE,
    .rx_buffer_size = OAL_FALSE,
    .en_hmac_feature_switch = {0, 0, 1},
    .roam_switch = WLAN_ROAM_SWITCH_MODE,
    .wlan_11ac2g_switch = OAL_TRUE,
    .wlan_probe_resp_mode = 0x10,
};

/* 特性功能开关和能力定制化参数 */
mac_cust_feature_cap_sru g_cust_cap = {
    .fast_ps = {WLAN_MIN_FAST_PS_IDLE, WLAN_MIN_FAST_PS_IDLE, WLAN_AUTO_FAST_PS_SCREENON, WLAN_AUTO_FAST_PS_SCREENOFF},
    .wlan_device_pm_switch = OAL_TRUE,
    .radio_cap = {WLAN_INIT_DEVICE_RADIO_CAP, WLAN_INIT_DEVICE_RADIO_CAP},
    .autocali_switch = OAL_FALSE,
    .en_channel_width = WLAN_HAL0_BW_MAX_WIDTH,
    .en_ldpc_is_supp = WLAN_HAL0_LDPC_IS_EN,
    .dev_frequency = {
        {PPS_VALUE_0, FREQ_IDLE, {0}},
        {PPS_VALUE_1, FREQ_MIDIUM, {0}},
        {PPS_VALUE_2, FREQ_HIGHER, {0}},
        {PPS_VALUE_3, FREQ_HIGHEST, {0}}},
    .used_mem_for_start = WLAN_MEM_FOR_START,
    .used_mem_for_stop = WLAN_MEM_FOR_STOP,
    .sdio_assem_h2d = WLAN_SDIO_H2D_ASSEMBLE_COUNT_VAL,
    .sdio_assem_d2h = HISDIO_DEV2HOST_SCATT_MAX,
    .linkloss_th = {WLAN_LOSS_THRESHOLD_WLAN_BT, WLAN_LOSS_THRESHOLD_WLAN_DBAC, WLAN_LOSS_THRESHOLD_WLAN_NORMAL},
    .mbo_switch = 0,
    .dbac_dynamic_switch = 0,
    .hiex_cap = (uint8_t)WLAN_HIEX_DEV_CAP,
    .fast_mode = 0,
    .btcoex_threshold_mcm = {WLAN_BTCOEX_THRESHOLD_MCM_DOWN, WLAN_BTCOEX_THRESHOLD_MCM_UP},
    .ftm_cap = OAL_FALSE,
    .ftm_board_delay = OAL_FALSE,
    .trx_switch = WLAN_DEFAULT_TRX_SWITCH,
    .hcc_flowctrl_type = 0,
    .hcc_flowctrl_switch = OAL_FALSE,
    .aput_160m_switch = OAL_FALSE,
    .en_rx_stbc_is_supp = WLAN_HAL0_RX_STBC_IS_EN,
    .en_tx_stbc_is_supp = WLAN_HAL0_TX_STBC_IS_EN,
    .en_su_bfmer_is_supp = WLAN_HAL0_SU_BFER_IS_EN,
    .en_su_bfmee_is_supp = WLAN_HAL0_SU_BFEE_IS_EN,
    .en_mu_bfmer_is_supp = WLAN_HAL0_MU_BFER_IS_EN,
    .en_mu_bfmee_is_supp = WLAN_HAL0_MU_BFEE_IS_EN,
    .optimized_feature_mask = 0,
    .en_1024qam_is_supp = WLAN_HAL0_1024QAM_IS_EN,
    .hisi_priv_cap = 0, /* 私有IE能力，05通过该默认值获取，06会通过INI文件读取 */
    .download_rate_limit_pps = 0,
    .txopps_switch = WLAN_HAL0_TXOPPS_IS_EN,
    .nrcoex_params.nrcoex_switch = OAL_FALSE,         /* 默认nrcoex方案关闭 */
    .nrcoex_params.version_num = 1,                   /* 默认采用第1版方案 */
    .nrcoex_params.nrcoex_hw = OAL_FALSE,             /* 默认关闭硬解功能 */
    .nrcoex_params.advance_low_power = OAL_FALSE,     /* 默认不提前降功率 */
    .loglevel = OAM_LOG_DEFAULT_LEVEL, /* 默认日志级别WARNING */
};

/* 功率定制化参数 */
wlan_cust_pow_stru g_cust_pow;
/* 动态功率校准制化参数 */
mac_cust_dyn_pow_sru g_cust_dyn_pow = {
    .dpn_para = {
        { { { 0 } } },
    },
};
/* 校准相关制化参数 */
mac_cus_rf_cali_stru g_cust_rf_cali = {
    .dyn_cali_dscr_interval = {0}, /* 动态校准开关2.4g 5g 6g */
    .band_5g_enable = 1,
};
/* RF前端相关制化参数 */
mac_customize_rf_front_sru g_cust_rf_front = {
    .rf_loss_gain_db = {
        .gain_db_2g = {{0xF4, 0xF4, 0xF4, 0xF4}, {0xF4, 0xF4, 0xF4, 0xF4}, {0xF4, 0xF4, 0xF4, 0xF4}},
        .gain_db_5g = {{0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8},
            {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        .gain_db_6g = {{0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8},
            {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8},
            {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8},
            {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}},
#endif
    },
    .ext_rf = {
        {
            .lna_bypass_gain_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .lna_gain_db = {0x14, 0x14, 0x14, 0x14},
            .pa_gain_b0_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .pa_gain_b1_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .ext_switch_isexist = {1, 1, 1, 1},
            .ext_pa_isexist = {1, 1, 1, 1},
            .ext_lna_isexist = {1, 1, 1, 1},
            .lna_on2off_time_ns = {0x0276, 0x0276, 0x0276, 0x0276},
            .lna_off2on_time_ns = {0x0140, 0x0140, 0x0140, 0x0140},
        },
        {
            .lna_bypass_gain_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .lna_gain_db = {0x14, 0x14, 0x14, 0x14},
            .pa_gain_b0_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .pa_gain_b1_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .ext_switch_isexist = {1, 1, 1, 1},
            .ext_pa_isexist = {1, 1, 1, 1},
            .ext_lna_isexist = {1, 1, 1, 1},
            .lna_on2off_time_ns = {0x0276, 0x0276, 0x0276, 0x0276},
            .lna_off2on_time_ns = {0x0140, 0x0140, 0x0140, 0x0140},
        },
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        {
            .lna_bypass_gain_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .lna_gain_db = {0x14, 0x14, 0x14, 0x14},
            .pa_gain_b0_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .pa_gain_b1_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .ext_switch_isexist = {1, 1, 1, 1},
            .ext_pa_isexist = {1, 1, 1, 1},
            .ext_lna_isexist = {1, 1, 1, 1},
            .lna_on2off_time_ns = {0x0276, 0x0276, 0x0276, 0x0276},
            .lna_off2on_time_ns = {0x0140, 0x0140, 0x0140, 0x0140},
        },
#endif
    }
};

static wlan_cust_cfg_cmd g_wifi_cfg_host[] = {
    /* ROAM */
    {"roam_switch",     WLAN_CFG_HOST_ROAM_SWITCH, &g_wlan_cust.uc_roam_switch,
     sizeof(g_wlan_cust.uc_roam_switch), WLAN_CUST_NUM_1, OAL_FALSE},
    {"scan_orthogonal", WLAN_CFG_HOST_SCAN_ORTHOGONAL, &g_wlan_cust.uc_roam_scan_orthogonal,
     sizeof(g_wlan_cust.uc_roam_scan_orthogonal), WLAN_CUST_NUM_1, OAL_FALSE},
    {"trigger_b",       WLAN_CFG_HOST_TRIGGER_B, &g_wlan_cust.c_roam_trigger_b,
     sizeof(g_wlan_cust.c_roam_trigger_b), WLAN_CUST_NUM_1, OAL_FALSE},
    {"trigger_a",       WLAN_CFG_HOST_TRIGGER_A, &g_wlan_cust.c_roam_trigger_a,
     sizeof(g_wlan_cust.c_roam_trigger_a), WLAN_CUST_NUM_1, OAL_FALSE},
    {"delta_b",         WLAN_CFG_HOST_DELTA_B, &g_wlan_cust.c_roam_delta_b,
     sizeof(g_wlan_cust.c_roam_delta_b), WLAN_CUST_NUM_1, OAL_FALSE},
    {"delta_a",         WLAN_CFG_HOST_DELTA_A, &g_wlan_cust.c_roam_delta_a,
     sizeof(g_wlan_cust.c_roam_delta_a), WLAN_CUST_NUM_1, OAL_FALSE},
    {"dense_env_trigger_b", WLAN_CFG_HOST_DENSE_ENV_TRIGGER_B, &g_wlan_cust.c_dense_env_roam_trigger_b,
     sizeof(g_wlan_cust.c_dense_env_roam_trigger_b), WLAN_CUST_NUM_1, OAL_FALSE},
    {"dense_env_trigger_a", WLAN_CFG_HOST_DENSE_ENV_TRIGGER_A, &g_wlan_cust.c_dense_env_roam_trigger_a,
     sizeof(g_wlan_cust.c_dense_env_roam_trigger_a), WLAN_CUST_NUM_1, OAL_FALSE},
    {"scenario_enable",     WLAN_CFG_HOST_SCENARIO_ENABLE, &g_wlan_cust.uc_scenario_enable,
     sizeof(g_wlan_cust.uc_scenario_enable), WLAN_CUST_NUM_1, OAL_FALSE},
    {"candidate_good_rssi", WLAN_CFG_HOST_CANDIDATE_GOOD_RSSI, &g_wlan_cust.c_candidate_good_rssi,
     sizeof(g_wlan_cust.c_candidate_good_rssi), WLAN_CUST_NUM_1, OAL_FALSE},
    {"candidate_good_num",  WLAN_CFG_HOST_CANDIDATE_GOOD_NUM, &g_wlan_cust.uc_candidate_good_num,
     sizeof(g_wlan_cust.uc_candidate_good_num), WLAN_CUST_NUM_1, OAL_FALSE},
    {"candidate_weak_num",  WLAN_CFG_HOST_CANDIDATE_WEAK_NUM, &g_wlan_cust.uc_candidate_weak_num,
     sizeof(g_wlan_cust.uc_candidate_weak_num), WLAN_CUST_NUM_1, OAL_FALSE},
    {"interval_variable",   WLAN_CFG_HOST_INTERVAL_VARIABLE, &g_wlan_cust.us_roam_interval,
     sizeof(g_wlan_cust.us_roam_interval), WLAN_CUST_NUM_1, OAL_FALSE},
    /* 性能 */
    {"ampdu_tx_max_num",    WLAN_CFG_HOST_AMPDU_TX_MAX_NUM, &g_wlan_cust.ampdu_tx_max_num,
     sizeof(g_wlan_cust.ampdu_tx_max_num), WLAN_CUST_NUM_1, OAL_FALSE},
    {"lock_dma_latency", WLAN_CFG_HOST_DMA_LATENCY, &g_cust_host.freq_lock.dma_latency_val,
     sizeof(g_cust_host.freq_lock.dma_latency_val), WLAN_CUST_NUM_1, OAL_FALSE},

    {"lock_cpu_th_high", WLAN_CFG_HOST_LOCK_CPU_TH_HIGH, &g_cust_host.freq_lock.lock_cpu_th_high,
     sizeof(g_cust_host.freq_lock.lock_cpu_th_high), WLAN_CUST_NUM_1, OAL_FALSE},
    {"lock_cpu_th_low", WLAN_CFG_HOST_LOCK_CPU_TH_LOW, &g_cust_host.freq_lock.lock_cpu_th_low,
     sizeof(g_cust_host.freq_lock.lock_cpu_th_low), WLAN_CUST_NUM_1, OAL_FALSE},

    {"irq_affinity",       WLAN_CFG_HOST_IRQ_AFFINITY, &g_cust_host.freq_lock.en_irq_affinity,
     sizeof(g_cust_host.freq_lock.en_irq_affinity), WLAN_CUST_NUM_1, OAL_FALSE},
    {"cpu_id_th_low",      WLAN_CFG_HOST_IRQ_TH_LOW, &g_cust_host.freq_lock.throughput_irq_low,
     sizeof(g_cust_host.freq_lock.throughput_irq_low), WLAN_CUST_NUM_1, OAL_FALSE},
    {"cpu_id_th_high",     WLAN_CFG_HOST_IRQ_TH_HIGH, &g_cust_host.freq_lock.throughput_irq_high,
     sizeof(g_cust_host.freq_lock.throughput_irq_high), WLAN_CUST_NUM_1, OAL_FALSE},
    {"cpu_id_pps_th_low",  WLAN_CFG_HOST_IRQ_PPS_TH_LOW, &g_cust_host.freq_lock.irq_pps_low,
     sizeof(g_cust_host.freq_lock.irq_pps_low), WLAN_CUST_NUM_1, OAL_FALSE},
    {"cpu_id_pps_th_high", WLAN_CFG_HOST_IRQ_PPS_TH_HIGH, &g_cust_host.freq_lock.irq_pps_high,
     sizeof(g_cust_host.freq_lock.irq_pps_high), WLAN_CUST_NUM_1, OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    /* ó2?t??o?ê1?ü */
    {"hw_ampdu", WLAN_CFG_HOST_HW_AMPDU, &g_cust_host.tx_ampdu.ampdu_hw_en,
     sizeof(g_cust_host.tx_ampdu.ampdu_hw_en), WLAN_CUST_NUM_1, OAL_FALSE},
    {"hw_ampdu_th_l", WLAN_CFG_HOST_HW_AMPDU_TH_LOW, &g_cust_host.tx_ampdu.throughput_low,
     sizeof(g_cust_host.tx_ampdu.throughput_low), WLAN_CUST_NUM_1, OAL_FALSE},
    {"hw_ampdu_th_h", WLAN_CFG_HOST_HW_AMPDU_TH_HIGH, &g_cust_host.tx_ampdu.throughput_high,
     sizeof(g_cust_host.tx_ampdu.throughput_high), WLAN_CUST_NUM_1, OAL_FALSE},
#endif
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    {"tx_amsdu_ampdu", WLAN_CFG_HOST_AMPDU_AMSDU_SKB, &g_cust_host.large_amsdu.large_amsdu_en,
     sizeof(g_cust_host.large_amsdu.large_amsdu_en), WLAN_CUST_NUM_1, OAL_FALSE},
    {"tx_amsdu_ampdu_th_l", WLAN_CFG_HOST_AMSDU_AMPDU_TH_LOW, &g_cust_host.large_amsdu.throughput_low,
     sizeof(g_cust_host.large_amsdu.throughput_low), WLAN_CUST_NUM_1, OAL_FALSE},
    {"tx_amsdu_ampdu_th_m", WLAN_CFG_HOST_AMSDU_AMPDU_TH_MIDDLE, &g_cust_host.large_amsdu.throughput_middle,
     sizeof(g_st_tx_large_amsdu.us_amsdu_throughput_middle), WLAN_CUST_NUM_1, OAL_FALSE},
    {"tx_amsdu_ampdu_th_h", WLAN_CFG_HOST_AMSDU_AMPDU_TH_HIGH, &g_cust_host.large_amsdu.throughput_high,
     sizeof(g_cust_host.large_amsdu.throughput_high), WLAN_CUST_NUM_1, OAL_FALSE},
#endif
#ifdef _PRE_WLAN_TCP_OPT
    {"en_tcp_ack_filter",      WLAN_CFG_HOST_TCP_ACK_FILTER, &g_cust_host.tcp_ack_filt.filter_en,
     sizeof(g_cust_host.tcp_ack_filt.filter_en), WLAN_CUST_NUM_1, OAL_FALSE},
    {"rx_tcp_ack_filter_th_l", WLAN_CFG_HOST_TCP_ACK_FILTER_TH_LOW, &g_cust_host.tcp_ack_filt.throughput_low,
     sizeof(g_cust_host.tcp_ack_filt.throughput_low), WLAN_CUST_NUM_1, OAL_FALSE},
    {"rx_tcp_ack_filter_th_h", WLAN_CFG_HOST_TCP_ACK_FILTER_TH_HIGH, &g_cust_host.tcp_ack_filt.throughput_high,
     sizeof(g_cust_host.tcp_ack_filt.throughput_high), WLAN_CUST_NUM_1, OAL_FALSE},
#endif

    {"small_amsdu_switch",   WLAN_CFG_HOST_TX_SMALL_AMSDU, &g_cust_host.small_amsdu.en_switch,
     sizeof(g_cust_host.small_amsdu.en_switch), WLAN_CUST_NUM_1, OAL_FALSE},
    {"small_amsdu_th_h", WLAN_CFG_HOST_SMALL_AMSDU_HIGH,
     &g_cust_host.small_amsdu.throughput_high,
     sizeof(g_cust_host.small_amsdu.throughput_high), WLAN_CUST_NUM_1, OAL_FALSE},
    {"small_amsdu_th_l",     WLAN_CFG_HOST_SMALL_AMSDU_LOW,
     &g_cust_host.small_amsdu.throughput_low,
     sizeof(g_cust_host.small_amsdu.throughput_low), WLAN_CUST_NUM_1, OAL_FALSE},
    {"small_amsdu_pps_th_h", WLAN_CFG_HOST_SMALL_AMSDU_PPS_HIGH,
     &g_cust_host.small_amsdu.pps_high,
     sizeof(g_cust_host.small_amsdu.pps_high), WLAN_CUST_NUM_1, OAL_FALSE},
    {"small_amsdu_pps_th_l", WLAN_CFG_HOST_SMALL_AMSDU_PPS_LOW,
     &g_cust_host.small_amsdu.pps_low,
     sizeof(g_cust_host.small_amsdu.pps_low), WLAN_CUST_NUM_1, OAL_FALSE},

    {"tcp_ack_buf_switch", WLAN_CFG_HOST_TX_TCP_ACK_BUF, &g_cust_host.tcp_ack_buf.buf_en,
     sizeof(g_cust_host.tcp_ack_buf.buf_en), WLAN_CUST_NUM_1, OAL_FALSE},
    {"tcp_ack_buf_th_h", WLAN_CFG_HOST_TCP_ACK_BUF_HIGH,
     &g_cust_host.tcp_ack_buf.throughput_high,
     sizeof(g_cust_host.tcp_ack_buf.throughput_high), WLAN_CUST_NUM_1, OAL_FALSE},
    {"tcp_ack_buf_th_l", WLAN_CFG_HOST_TCP_ACK_BUF_LOW,
     &g_cust_host.tcp_ack_buf.throughput_low,
     sizeof(g_cust_host.tcp_ack_buf.throughput_low), WLAN_CUST_NUM_1, OAL_FALSE},
    {"tcp_ack_buf_th_h_40M",  WLAN_CFG_HOST_TCP_ACK_BUF_HIGH_40M,
     &g_cust_host.tcp_ack_buf.throughput_high_40m,
     sizeof(g_cust_host.tcp_ack_buf.throughput_high_40m), WLAN_CUST_NUM_1, OAL_FALSE},
    {"tcp_ack_buf_th_l_40M",  WLAN_CFG_HOST_TCP_ACK_BUF_LOW_40M,
     &g_cust_host.tcp_ack_buf.throughput_low_40m,
     sizeof(g_cust_host.tcp_ack_buf.throughput_low_40m), WLAN_CUST_NUM_1, OAL_FALSE},
    {"tcp_ack_buf_th_h_80M",  WLAN_CFG_HOST_TCP_ACK_BUF_HIGH_80M,
     &g_cust_host.tcp_ack_buf.throughput_high_80m,
     sizeof(g_cust_host.tcp_ack_buf.throughput_high_80m), WLAN_CUST_NUM_1, OAL_FALSE},
    {"tcp_ack_buf_th_l_80M",  WLAN_CFG_HOST_TCP_ACK_BUF_LOW_80M,
     &g_cust_host.tcp_ack_buf.throughput_low_80m,
     sizeof(g_cust_host.tcp_ack_buf.throughput_low_80m), WLAN_CUST_NUM_1, OAL_FALSE},
    {"tcp_ack_buf_th_h_160M", WLAN_CFG_HOST_TCP_ACK_BUF_HIGH_160M,
     &g_cust_host.tcp_ack_buf.throughput_high_160m,
     sizeof(g_cust_host.tcp_ack_buf.throughput_high_160m), WLAN_CUST_NUM_1, OAL_FALSE},
    {"tcp_ack_buf_th_l_160M", WLAN_CFG_HOST_TCP_ACK_BUF_LOW_160M,
     &g_cust_host.tcp_ack_buf.throughput_low_160m,
     sizeof(g_cust_host.tcp_ack_buf.throughput_low_160m), WLAN_CUST_NUM_1, OAL_FALSE},
    {"tcp_ack_buf_userctl_switch", WLAN_CFG_HOST_TX_TCP_ACK_BUF_USERCTL, &g_cust_host.tcp_ack_buf.buf_userctl_test_en,
     sizeof(g_cust_host.tcp_ack_buf.buf_userctl_test_en), WLAN_CUST_NUM_1, OAL_FALSE},
    {"tcp_ack_buf_userctl_h",      WLAN_CFG_HOST_TCP_ACK_BUF_USERCTL_HIGH, &g_cust_host.tcp_ack_buf.buf_userctl_high,
     sizeof(g_cust_host.tcp_ack_buf.buf_userctl_high), WLAN_CUST_NUM_1, OAL_FALSE},
    {"tcp_ack_buf_userctl_l",      WLAN_CFG_HOST_TCP_ACK_BUF_USERCTL_LOW, &g_cust_host.tcp_ack_buf.buf_userctl_low,
     sizeof(g_cust_host.tcp_ack_buf.buf_userctl_low), WLAN_CUST_NUM_1, OAL_FALSE},

    {"dyn_bypass_extlna_th_switch", WLAN_CFG_HOST_RX_DYN_BYPASS_EXTLNA, &g_cust_host.dyn_extlna.switch_en,
     sizeof(g_cust_host.dyn_extlna.switch_en), WLAN_CUST_NUM_1, OAL_FALSE},
    {"dyn_bypass_extlna_th_h",      WLAN_CFG_HOST_RX_DYN_BYPASS_EXTLNA_HIGH,
     &g_cust_host.dyn_extlna.throughput_high, sizeof(g_cust_host.dyn_extlna.throughput_high),
     WLAN_CUST_NUM_1, OAL_FALSE},
    {"dyn_bypass_extlna_th_l",      WLAN_CFG_HOST_RX_DYN_BYPASS_EXTLNA_LOW, &g_cust_host.dyn_extlna.throughput_low,
     sizeof(g_cust_host.dyn_extlna.throughput_low), WLAN_CUST_NUM_1, OAL_FALSE},

    {"rx_ampdu_amsdu", WLAN_CFG_HOST_RX_AMPDU_AMSDU_SKB, &g_cust_host.host_rx_ampdu_amsdu,
     sizeof(g_cust_host.host_rx_ampdu_amsdu), WLAN_CUST_NUM_1, OAL_FALSE},
    {"rx_ampdu_bitmap", WLAN_CFG_HOST_RX_AMPDU_BITMAP, &g_cust_host.rx_buffer_size,
     sizeof(g_cust_host.rx_buffer_size), WLAN_CUST_NUM_1, OAL_FALSE},

    {"ps_mode", WLAN_CFG_HOST_PS_MODE, &g_cust_host.wlan_ps_mode,
     sizeof(g_cust_host.wlan_ps_mode), WLAN_CUST_NUM_1, OAL_FALSE},
    /* SCAN */
    {"random_mac_addr_scan", WLAN_CFG_HOST_RANDOM_MAC_ADDR_SCAN, &g_wlan_cust.uc_random_mac_addr_scan,
     sizeof(g_wlan_cust.uc_random_mac_addr_scan), WLAN_CUST_NUM_1, OAL_FALSE},
    /* 11AC2G */
    {"11ac2g_enable",        WLAN_CFG_HOST_11AC2G_ENABLE, &g_cust_host.wlan_11ac2g_switch,
     sizeof(g_cust_host.wlan_11ac2g_switch), WLAN_CUST_NUM_1, OAL_FALSE},
    {"disable_capab_2ght40", WLAN_CFG_HOST_DISABLE_CAPAB_2GHT40, &g_wlan_cust.uc_disable_capab_2ght40,
     sizeof(g_wlan_cust.uc_disable_capab_2ght40), WLAN_CUST_NUM_1, OAL_FALSE},

    {"probe_resp_mode", WLAN_CFG_HOST_PROBE_RESP_MODE, &g_cust_host.wlan_probe_resp_mode,
     sizeof(g_cust_host.wlan_probe_resp_mode), WLAN_CUST_NUM_1, OAL_FALSE},
    {"miracast_enable", WLAN_CFG_HOST_MIRACAST_SINK_ENABLE,
     &g_cust_host.en_hmac_feature_switch[CUST_MIRACAST_SINK_SWITCH],
     sizeof(g_cust_host.en_hmac_feature_switch[CUST_MIRACAST_SINK_SWITCH]), WLAN_CUST_NUM_1, OAL_FALSE},
    {"reduce_miracast_log", WLAN_CFG_HOST_REDUCE_MIRACAST_LOG,
     &g_cust_host.en_hmac_feature_switch[CUST_MIRACAST_REDUCE_LOG_SWITCH],
     sizeof(g_cust_host.en_hmac_feature_switch[CUST_MIRACAST_REDUCE_LOG_SWITCH]), WLAN_CUST_NUM_1, OAL_FALSE},
    {"core_bind_cap", WLAN_CFG_HOST_CORE_BIND_CAP,
     &g_cust_host.en_hmac_feature_switch[CUST_CORE_BIND_SWITCH],
     sizeof(g_cust_host.en_hmac_feature_switch[CUST_CORE_BIND_SWITCH]), WLAN_CUST_NUM_1, OAL_FALSE},

    {"lte_gpio_check_switch", WLAN_CFG_HOST_LTE_GPIO_CHECK_SWITCH, &g_wlan_cust.lte_gpio_check_switch,
     sizeof(g_wlan_cust.lte_gpio_check_switch), WLAN_CUST_NUM_1, OAL_FALSE},
    {"ism_priority", WLAN_HOST_ATCMDSRV_ISM_PRIORITY, &g_wlan_cust.ism_priority,
     sizeof(g_wlan_cust.ism_priority), WLAN_CUST_NUM_1, OAL_FALSE},
    {"lte_rx", WLAN_HOST_ATCMDSRV_LTE_RX, &g_wlan_cust.lte_rx,
     sizeof(g_wlan_cust.lte_rx), WLAN_CUST_NUM_1, OAL_FALSE},
    {"lte_tx", WLAN_HOST_ATCMDSRV_LTE_TX, &g_wlan_cust.lte_tx,
     sizeof(g_wlan_cust.lte_tx), WLAN_CUST_NUM_1, OAL_FALSE},
    {"lte_inact", WLAN_HOST_ATCMDSRV_LTE_INACT, &g_wlan_cust.lte_inact,
     sizeof(g_wlan_cust.lte_inact), WLAN_CUST_NUM_1, OAL_FALSE},
    {"ism_rx_act", WLAN_HOST_ATCMDSRV_ISM_RX_ACT, &g_wlan_cust.ism_rx_act,
     sizeof(g_wlan_cust.ism_rx_act), WLAN_CUST_NUM_1, OAL_FALSE},
    {"bant_pri", WLAN_HOST_ATCMDSRV_BANT_PRI, &g_wlan_cust.bant_pri,
     sizeof(g_wlan_cust.bant_pri), WLAN_CUST_NUM_1, OAL_FALSE},
    {"bant_status", WLAN_HOST_ATCMDSRV_BANT_STATUS, &g_wlan_cust.bant_status,
     sizeof(g_wlan_cust.bant_status), WLAN_CUST_NUM_1, OAL_FALSE},
    {"want_pri", WLAN_HOST_ATCMDSRV_WANT_PRI, &g_wlan_cust.want_pri,
     sizeof(g_wlan_cust.want_pri), WLAN_CUST_NUM_1, OAL_FALSE},
    {"want_status", WLAN_HOST_ATCMDSRV_WANT_STATUS, &g_wlan_cust.want_status,
     sizeof(g_wlan_cust.want_status), WLAN_CUST_NUM_1, OAL_FALSE},

    {"lock_cpu_freq", WLAN_CFG_HOST_LOCK_CPU_FREQ, &g_cust_host.lock_max_cpu_freq,
     sizeof(g_cust_host.lock_max_cpu_freq), WLAN_CUST_NUM_1, OAL_FALSE},
    {"optimized_feature_mask",     WLAN_CFG_HOST_OPTIMIZED_FEATURE_SWITCH,
     &g_cust_cap.optimized_feature_mask, sizeof(g_cust_cap.optimized_feature_mask), WLAN_CUST_NUM_1, OAL_FALSE},

    {NULL, 0, 0, 0, 0, OAL_FALSE}
};

static wlan_cust_cfg_cmd g_wifi_cfg_rf_cali[] = {
    /* 动态校准 */
    {"dyn_cali_dscr_interval", WLAN_CFG_DYN_CALI_DSCR_ITERVL, &g_cust_rf_cali.dyn_cali_dscr_interval[WLAN_CALI_BAND_2G],
     sizeof(g_cust_rf_cali.dyn_cali_dscr_interval[WLAN_CALI_BAND_2G]) << BIT_OFFSET_1, WLAN_CUST_NUM_1, OAL_FALSE},
    {"far_dist_pow_gain_switch", WLAN_CFG_FAR_DIST_POW_GAIN_SWITCH, &g_cust_rf_front.far_dist_pow_gain_switch,
     sizeof(g_cust_rf_front.far_dist_pow_gain_switch), WLAN_CUST_NUM_1, OAL_FALSE},

    /* 2G RF前端插损 */
    {"rf_rx_insertion_loss_2g_b1", WLAN_CFG_RF_RX_INSERTION_LOSS_2G_BAND1,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_2g[WLAN_2G_BAND_IDX0][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_2g[WLAN_2G_BAND_IDX0][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_2g[WLAN_2G_BAND_IDX0]), OAL_FALSE},
    {"rf_rx_insertion_loss_2g_b2", WLAN_CFG_RF_RX_INSERTION_LOSS_2G_BAND2,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_2g[WLAN_2G_BAND_IDX1][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_2g[WLAN_2G_BAND_IDX1][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_2g[WLAN_2G_BAND_IDX1]), OAL_FALSE},
    {"rf_rx_insertion_loss_2g_b3", WLAN_CFG_RF_RX_INSERTION_LOSS_2G_BAND3,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_2g[WLAN_2G_BAND_IDX2][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_2g[WLAN_2G_BAND_IDX2][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_2g[WLAN_2G_BAND_IDX2]), OAL_FALSE},

    /* 5G RF前端插损 */
    {"rf_rx_insertion_loss_5g_b1", WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND1,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX0][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX0][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX0]), OAL_FALSE},
    {"rf_rx_insertion_loss_5g_b2", WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND2,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX1][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX1][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX1]), OAL_FALSE},
    {"rf_rx_insertion_loss_5g_b3", WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND3,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX2][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX2][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX2]), OAL_FALSE},
    {"rf_rx_insertion_loss_5g_b4", WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND4,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX3][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX3][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX3]), OAL_FALSE},
    {"rf_rx_insertion_loss_5g_b5", WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND5,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX4][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX4][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX4]), OAL_FALSE},
    {"rf_rx_insertion_loss_5g_b6", WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND6,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX5][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX5][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX5]), OAL_FALSE},
    {"rf_rx_insertion_loss_5g_b7", WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND7,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX6][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX6][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_5g[WLAN_5G_BAND_IDX6]), OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    /* 6G RF前端插损 */
    {"rf_rx_insertion_loss_6g_b0", WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND0,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX0][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX0][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX0]), OAL_FALSE},
    {"rf_rx_insertion_loss_6g_b1", WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND1,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX1][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX1][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX1]), OAL_FALSE},
    {"rf_rx_insertion_loss_6g_b2", WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND2,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX2][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX2][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX2]), OAL_FALSE},
    {"rf_rx_insertion_loss_6g_b3", WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND3,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX3][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX3][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX3]), OAL_FALSE},
    {"rf_rx_insertion_loss_6g_b4", WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND4,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX4][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX4][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX4]), OAL_FALSE},
    {"rf_rx_insertion_loss_6g_b5", WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND5,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX5][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX5][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX5]), OAL_FALSE},
    {"rf_rx_insertion_loss_6g_b6", WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND6,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX6][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX6][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX6]), OAL_FALSE},
    {"rf_rx_insertion_loss_6g_b7", WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND7,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX7][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX7][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX7]), OAL_FALSE},
    {"rf_rx_insertion_loss_6g_b8", WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND8,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX8][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX8][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX8]), OAL_FALSE},
    {"rf_rx_insertion_loss_6g_b9", WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND9,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX9][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX9][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX9]), OAL_FALSE},
    {"rf_rx_insertion_loss_6g_b10", WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND10,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX10][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX10][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX10]), OAL_FALSE},
    {"rf_rx_insertion_loss_6g_b11", WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND11,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX11][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX11][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX11]), OAL_FALSE},
    {"rf_rx_insertion_loss_6g_b12", WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND12,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX12][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX12][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX12]), OAL_FALSE},
    {"rf_rx_insertion_loss_6g_b13", WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND13,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX13][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX13][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX13]), OAL_FALSE},
    {"rf_rx_insertion_loss_6g_b14", WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND14,
     &g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX14][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX14][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[WLAN_6G_BAND_IDX14]), OAL_FALSE},
#endif
    /* 用于定制化计算PWR RF值的偏差 */
    {"rf_line_rf_pwr_ref_rssi_db_2g_20m_mult4", WLAN_CFG_RF_PWR_REF_RSSI_2G_20M_MULT4,
     &g_cust_rf_front.delta_pwr_ref.delta_rssi_2g[WLAN_2G_DELTA_RSSI_BAND_20M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.delta_pwr_ref.delta_rssi_2g[0][0]),
     ARRAY_SIZE(g_cust_rf_front.delta_pwr_ref.delta_rssi_2g[WLAN_2G_DELTA_RSSI_BAND_20M]), OAL_FALSE},
    {"rf_line_rf_pwr_ref_rssi_db_2g_40m_mult4",  WLAN_CFG_RF_PWR_REF_RSSI_2G_40M_MULT4,
     &g_cust_rf_front.delta_pwr_ref.delta_rssi_2g[WLAN_2G_DELTA_RSSI_BAND_40M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.delta_pwr_ref.delta_rssi_2g[1][0]),
     ARRAY_SIZE(g_cust_rf_front.delta_pwr_ref.delta_rssi_2g[WLAN_2G_DELTA_RSSI_BAND_40M]), OAL_FALSE},

    {"rf_line_rf_pwr_ref_rssi_db_5g_20m_mult4",  WLAN_CFG_RF_PWR_REF_RSSI_5G_20M_MULT4,
     &g_cust_rf_front.delta_pwr_ref.delta_rssi_5g[WLAN_5G_DELTA_RSSI_BAND_20M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.delta_pwr_ref.delta_rssi_5g[WLAN_5G_DELTA_RSSI_BAND_20M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.delta_pwr_ref.delta_rssi_5g[WLAN_5G_DELTA_RSSI_BAND_20M]), OAL_FALSE},
    {"rf_line_rf_pwr_ref_rssi_db_5g_40m_mult4",  WLAN_CFG_RF_PWR_REF_RSSI_5G_40M_MULT4,
     &g_cust_rf_front.delta_pwr_ref.delta_rssi_5g[WLAN_5G_DELTA_RSSI_BAND_40M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.delta_pwr_ref.delta_rssi_5g[WLAN_5G_DELTA_RSSI_BAND_40M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.delta_pwr_ref.delta_rssi_5g[WLAN_5G_DELTA_RSSI_BAND_40M]), OAL_FALSE},
    {"rf_line_rf_pwr_ref_rssi_db_5g_80m_mult4",  WLAN_CFG_RF_PWR_REF_RSSI_5G_80M_MULT4,
     &g_cust_rf_front.delta_pwr_ref.delta_rssi_5g[WLAN_5G_DELTA_RSSI_BAND_80M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.delta_pwr_ref.delta_rssi_5g[WLAN_5G_DELTA_RSSI_BAND_80M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.delta_pwr_ref.delta_rssi_5g[WLAN_5G_DELTA_RSSI_BAND_80M]), OAL_FALSE},
    {"rf_line_rf_pwr_ref_rssi_db_5g_160m_mult4",  WLAN_CFG_RF_PWR_REF_RSSI_5G_160M_MULT4,
     &g_cust_rf_front.delta_pwr_ref.delta_rssi_5g[WLAN_5G_DELTA_RSSI_BAND_160M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.delta_pwr_ref.delta_rssi_5g[WLAN_5G_DELTA_RSSI_BAND_160M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.delta_pwr_ref.delta_rssi_5g[WLAN_5G_DELTA_RSSI_BAND_160M]), OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"rf_line_rf_pwr_ref_rssi_db_6g_20m_mult4",  WLAN_CFG_RF_PWR_REF_RSSI_6G_20M_MULT4,
     &g_cust_rf_front.delta_pwr_ref.delta_rssi_6g[WLAN_5G_DELTA_RSSI_BAND_20M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.delta_pwr_ref.delta_rssi_6g[WLAN_5G_DELTA_RSSI_BAND_20M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.delta_pwr_ref.delta_rssi_6g[WLAN_5G_DELTA_RSSI_BAND_20M]), OAL_FALSE},
    {"rf_line_rf_pwr_ref_rssi_db_6g_40m_mult4",  WLAN_CFG_RF_PWR_REF_RSSI_6G_40M_MULT4,
     &g_cust_rf_front.delta_pwr_ref.delta_rssi_6g[WLAN_5G_DELTA_RSSI_BAND_40M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.delta_pwr_ref.delta_rssi_6g[WLAN_5G_DELTA_RSSI_BAND_40M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.delta_pwr_ref.delta_rssi_6g[WLAN_5G_DELTA_RSSI_BAND_40M]), OAL_FALSE},
    {"rf_line_rf_pwr_ref_rssi_db_6g_80m_mult4",  WLAN_CFG_RF_PWR_REF_RSSI_6G_80M_MULT4,
     &g_cust_rf_front.delta_pwr_ref.delta_rssi_6g[WLAN_5G_DELTA_RSSI_BAND_80M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.delta_pwr_ref.delta_rssi_6g[WLAN_5G_DELTA_RSSI_BAND_80M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.delta_pwr_ref.delta_rssi_6g[WLAN_5G_DELTA_RSSI_BAND_80M]), OAL_FALSE},
    {"rf_line_rf_pwr_ref_rssi_db_6g_160m_mult4",  WLAN_CFG_RF_PWR_REF_RSSI_6G_160M_MULT4,
     &g_cust_rf_front.delta_pwr_ref.delta_rssi_6g[WLAN_5G_DELTA_RSSI_BAND_160M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.delta_pwr_ref.delta_rssi_6g[WLAN_5G_DELTA_RSSI_BAND_160M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.delta_pwr_ref.delta_rssi_6g[WLAN_5G_DELTA_RSSI_BAND_160M]), OAL_FALSE},
#endif
    {"rf_rssi_amend_2g_20M", WLAN_CFG_RF_AMEND_RSSI_2G_20M,
     &g_cust_rf_front.rssi_amend_cfg.amend_rssi_2g[WLAN_2G_AMEND_RSSI_BAND_20M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rssi_amend_cfg.amend_rssi_2g[WLAN_2G_AMEND_RSSI_BAND_20M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rssi_amend_cfg.amend_rssi_2g[WLAN_2G_AMEND_RSSI_BAND_20M]), OAL_FALSE},
    {"rf_rssi_amend_2g_40M", WLAN_CFG_RF_AMEND_RSSI_2G_40M,
     &g_cust_rf_front.rssi_amend_cfg.amend_rssi_2g[WLAN_2G_AMEND_RSSI_BAND_40M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rssi_amend_cfg.amend_rssi_2g[WLAN_2G_AMEND_RSSI_BAND_40M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rssi_amend_cfg.amend_rssi_2g[WLAN_2G_AMEND_RSSI_BAND_40M]), OAL_FALSE},
    {"rf_rssi_amend_2g_11b", WLAN_CFG_RF_AMEND_RSSI_2G_11B,
     &g_cust_rf_front.rssi_amend_cfg.amend_rssi_2g[WLAN_2G_AMEND_RSSI_BAND_11B][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rssi_amend_cfg.amend_rssi_2g[WLAN_2G_AMEND_RSSI_BAND_11B][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rssi_amend_cfg.amend_rssi_2g[WLAN_2G_AMEND_RSSI_BAND_11B]), OAL_FALSE},

    {"rf_rssi_amend_5g_20M", WLAN_CFG_RF_AMEND_RSSI_5G_20M,
     &g_cust_rf_front.rssi_amend_cfg.amend_rssi_5g[WLAN_5G_AMEND_RSSI_BAND_20M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rssi_amend_cfg.amend_rssi_5g[WLAN_5G_AMEND_RSSI_BAND_20M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rssi_amend_cfg.amend_rssi_5g[WLAN_5G_AMEND_RSSI_BAND_20M]), OAL_FALSE},
    {"rf_rssi_amend_5g_40M", WLAN_CFG_RF_AMEND_RSSI_5G_40M,
     &g_cust_rf_front.rssi_amend_cfg.amend_rssi_5g[WLAN_5G_AMEND_RSSI_BAND_40M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rssi_amend_cfg.amend_rssi_5g[WLAN_5G_AMEND_RSSI_BAND_40M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rssi_amend_cfg.amend_rssi_5g[WLAN_5G_AMEND_RSSI_BAND_40M]), OAL_FALSE},
    {"rf_rssi_amend_5g_80M", WLAN_CFG_RF_AMEND_RSSI_5G_80M,
     &g_cust_rf_front.rssi_amend_cfg.amend_rssi_5g[WLAN_5G_AMEND_RSSI_BAND_80M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rssi_amend_cfg.amend_rssi_5g[WLAN_5G_AMEND_RSSI_BAND_80M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rssi_amend_cfg.amend_rssi_5g[WLAN_5G_AMEND_RSSI_BAND_80M]), OAL_FALSE},
    {"rf_rssi_amend_5g_160M", WLAN_CFG_RF_AMEND_RSSI_5G_160M,
     &g_cust_rf_front.rssi_amend_cfg.amend_rssi_5g[WLAN_5G_AMEND_RSSI_BAND_160M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rssi_amend_cfg.amend_rssi_5g[WLAN_5G_AMEND_RSSI_BAND_160M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rssi_amend_cfg.amend_rssi_5g[WLAN_5G_AMEND_RSSI_BAND_160M]), OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"rf_rssi_amend_6g_20M", WLAN_CFG_RF_AMEND_RSSI_6G_20M,
     &g_cust_rf_front.rssi_amend_cfg.amend_rssi_6g[WLAN_5G_AMEND_RSSI_BAND_20M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rssi_amend_cfg.amend_rssi_6g[WLAN_5G_AMEND_RSSI_BAND_20M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rssi_amend_cfg.amend_rssi_6g[WLAN_5G_AMEND_RSSI_BAND_20M]), OAL_FALSE},
    {"rf_rssi_amend_6g_40M", WLAN_CFG_RF_AMEND_RSSI_6G_40M,
     &g_cust_rf_front.rssi_amend_cfg.amend_rssi_6g[WLAN_5G_AMEND_RSSI_BAND_40M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rssi_amend_cfg.amend_rssi_6g[WLAN_5G_AMEND_RSSI_BAND_40M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rssi_amend_cfg.amend_rssi_6g[WLAN_5G_AMEND_RSSI_BAND_40M]), OAL_FALSE},
    {"rf_rssi_amend_6g_80M", WLAN_CFG_RF_AMEND_RSSI_6G_80M,
     &g_cust_rf_front.rssi_amend_cfg.amend_rssi_6g[WLAN_5G_AMEND_RSSI_BAND_80M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rssi_amend_cfg.amend_rssi_6g[WLAN_5G_AMEND_RSSI_BAND_80M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rssi_amend_cfg.amend_rssi_6g[WLAN_5G_AMEND_RSSI_BAND_80M]), OAL_FALSE},
    {"rf_rssi_amend_6g_160M", WLAN_CFG_RF_AMEND_RSSI_6G_160M,
     &g_cust_rf_front.rssi_amend_cfg.amend_rssi_6g[WLAN_5G_AMEND_RSSI_BAND_160M][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.rssi_amend_cfg.amend_rssi_6g[WLAN_5G_AMEND_RSSI_BAND_160M][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.rssi_amend_cfg.amend_rssi_6g[WLAN_5G_AMEND_RSSI_BAND_160M]), OAL_FALSE},
#endif

    /* fem */
    {"rf_lna_bypass_gain_db_2g", WLAN_CFG_RF_LNA_BYPASS_GAIN_DB_2G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].lna_bypass_gain_db[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].lna_bypass_gain_db[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].lna_bypass_gain_db), OAL_FALSE},
    {"rf_lna_gain_db_2g",        WLAN_CFG_RF_LNA_GAIN_DB_2G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].lna_gain_db[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].lna_bypass_gain_db[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].lna_gain_db), OAL_FALSE},
    {"rf_pa_db_b0_2g",           WLAN_CFG_RF_PA_GAIN_DB_B0_2G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].pa_gain_b0_db[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].pa_gain_b0_db[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].pa_gain_b0_db), OAL_FALSE},
    {"rf_pa_db_b1_2g",          WLAN_CFG_RF_PA_GAIN_DB_B1_2G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].pa_gain_b1_db[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].pa_gain_b1_db[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].pa_gain_b1_db), OAL_FALSE},
    {"rf_pa_db_lvl_2g",          WLAN_CFG_RF_PA_GAIN_LVL_2G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].pa_gain_lvl_num[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].pa_gain_lvl_num[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].pa_gain_lvl_num), OAL_FALSE},
    {"ext_switch_isexist_2g",    WLAN_CFG_EXT_SWITCH_ISEXIST_2G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].ext_switch_isexist[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].ext_switch_isexist[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].ext_switch_isexist), OAL_FALSE},
    {"ext_pa_isexist_2g",        WLAN_CFG_EXT_PA_ISEXIST_2G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].ext_pa_isexist[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].ext_pa_isexist[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].ext_pa_isexist), OAL_FALSE},
    {"ext_lna_isexist_2g",       WLAN_CFG_EXT_LNA_ISEXIST_2G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].ext_lna_isexist[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].ext_lna_isexist[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].ext_lna_isexist), OAL_FALSE},
    {"lna_on2off_time_ns_2g",    WLAN_CFG_LNA_ON2OFF_TIME_NS_2G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].lna_on2off_time_ns[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].lna_on2off_time_ns[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].lna_on2off_time_ns), OAL_FALSE},
    {"lna_off2on_time_ns_2g",    WLAN_CFG_LNA_OFF2ON_TIME_NS_2G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].lna_off2on_time_ns[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].lna_off2on_time_ns[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].lna_off2on_time_ns), OAL_FALSE},
    {"rf_lna_bypass_gain_db_5g", WLAN_CFG_RF_LNA_BYPASS_GAIN_DB_5G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].lna_bypass_gain_db[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].lna_bypass_gain_db[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].lna_bypass_gain_db), OAL_FALSE},
    {"rf_lna_gain_db_5g",        WLAN_CFG_RF_LNA_GAIN_DB_5G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].lna_gain_db[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].lna_gain_db[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].lna_gain_db), OAL_FALSE},
    {"rf_pa_db_b0_5g",           WLAN_CFG_RF_PA_GAIN_DB_B0_5G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].pa_gain_b0_db[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].pa_gain_b0_db[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].pa_gain_b0_db), OAL_FALSE},
    {"rf_pa_db_b1_5g",           WLAN_CFG_RF_PA_GAIN_DB_B1_5G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].pa_gain_b1_db[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].pa_gain_b1_db[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].pa_gain_b1_db), OAL_FALSE},
    {"rf_pa_db_lvl_5g",          WLAN_CFG_RF_PA_GAIN_LVL_5G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].pa_gain_lvl_num[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].pa_gain_lvl_num[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].pa_gain_lvl_num), OAL_FALSE},
    {"ext_switch_isexist_5g",    WLAN_CFG_EXT_SWITCH_ISEXIST_5G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].ext_switch_isexist[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].ext_switch_isexist[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].ext_switch_isexist), OAL_FALSE},
    {"ext_pa_isexist_5g",        WLAN_CFG_EXT_PA_ISEXIST_5G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].ext_pa_isexist[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].ext_pa_isexist[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].ext_pa_isexist), OAL_FALSE},
    {"ext_lna_isexist_5g",       WLAN_CFG_EXT_LNA_ISEXIST_5G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].ext_lna_isexist[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].ext_lna_isexist[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].ext_lna_isexist), OAL_FALSE},
    {"lna_on2off_time_ns_5g",    WLAN_CFG_LNA_ON2OFF_TIME_NS_5G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].lna_on2off_time_ns[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].lna_on2off_time_ns[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].lna_on2off_time_ns), OAL_FALSE},
    {"lna_off2on_time_ns_5g",    WLAN_CFG_LNA_OFF2ON_TIME_NS_5G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].lna_off2on_time_ns[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].lna_off2on_time_ns[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].lna_off2on_time_ns), OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"rf_lna_bypass_gain_db_6g", WLAN_CFG_RF_LNA_BYPASS_GAIN_DB_6G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].lna_bypass_gain_db[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].lna_bypass_gain_db[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].lna_bypass_gain_db), OAL_FALSE},
    {"rf_lna_gain_db_6g",        WLAN_CFG_RF_LNA_GAIN_DB_6G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].lna_gain_db[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].lna_gain_db[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].lna_gain_db), OAL_FALSE},
    {"rf_pa_db_b0_6g",           WLAN_CFG_RF_PA_GAIN_DB_B0_6G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].pa_gain_b0_db[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].pa_gain_b0_db[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].pa_gain_b0_db), OAL_FALSE},
    {"rf_pa_db_b1_6g",           WLAN_CFG_RF_PA_GAIN_DB_B1_6G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].pa_gain_b1_db[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].pa_gain_b1_db[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].pa_gain_b1_db), OAL_FALSE},
    {"rf_pa_db_lvl_6g",          WLAN_CFG_RF_PA_GAIN_LVL_6G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].pa_gain_lvl_num[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].pa_gain_lvl_num[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].pa_gain_lvl_num), OAL_FALSE},
    {"ext_switch_isexist_6g",    WLAN_CFG_EXT_SWITCH_ISEXIST_6G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].ext_switch_isexist[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].ext_switch_isexist[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].ext_switch_isexist), OAL_FALSE},
    {"ext_pa_isexist_6g",        WLAN_CFG_EXT_PA_ISEXIST_6G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].ext_pa_isexist[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].ext_pa_isexist[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].ext_pa_isexist), OAL_FALSE},
    {"ext_lna_isexist_6g",       WLAN_CFG_EXT_LNA_ISEXIST_6G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].ext_lna_isexist[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].ext_lna_isexist[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].ext_lna_isexist), OAL_FALSE},
    {"lna_on2off_time_ns_6g",    WLAN_CFG_LNA_ON2OFF_TIME_NS_6G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].lna_on2off_time_ns[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].lna_on2off_time_ns[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].lna_on2off_time_ns), OAL_FALSE},
    {"lna_off2on_time_ns_6g",    WLAN_CFG_LNA_OFF2ON_TIME_NS_6G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].lna_off2on_time_ns[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].lna_off2on_time_ns[WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].lna_off2on_time_ns), OAL_FALSE},
#endif
    {"fem_mode_2g",          WLAN_CFG_FEM_MODE_2G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].fem_mode,
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_2G].fem_mode), WLAN_CUST_NUM_1, OAL_FALSE},
    {"fem_mode_5g",          WLAN_CFG_FEM_MODE_5G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].fem_mode,
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_5G].fem_mode), WLAN_CUST_NUM_1, OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"fem_mode_6g",          WLAN_CFG_FEM_MODE_6G,
     &g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].fem_mode,
     sizeof(g_cust_rf_front.ext_rf[WLAN_CALI_BAND_6G].fem_mode), WLAN_CUST_NUM_1, OAL_FALSE},
#endif
    {"delta_cca_ed_high_20th_2g", WLAN_CFG_DELTA_CCA_ED_HIGH_20TH_2G, &g_cust_rf_front.delta_cca_ed_high_20th_2g,
     sizeof(g_cust_rf_front.delta_cca_ed_high_20th_2g), WLAN_CUST_NUM_1, OAL_FALSE},
    {"delta_cca_ed_high_40th_2g", WLAN_CFG_DELTA_CCA_ED_HIGH_40TH_2G, &g_cust_rf_front.delta_cca_ed_high_40th_2g,
     sizeof(g_cust_rf_front.delta_cca_ed_high_40th_2g), WLAN_CUST_NUM_1, OAL_FALSE},
    {"delta_cca_ed_high_20th_5g", WLAN_CFG_DELTA_CCA_ED_HIGH_20TH_5G, &g_cust_rf_front.delta_cca_ed_high_20th_5g,
     sizeof(g_cust_rf_front.delta_cca_ed_high_20th_5g), WLAN_CUST_NUM_1, OAL_FALSE},
    {"delta_cca_ed_high_40th_5g", WLAN_CFG_DELTA_CCA_ED_HIGH_40TH_5G, &g_cust_rf_front.delta_cca_ed_high_40th_5g,
     sizeof(g_cust_rf_front.delta_cca_ed_high_40th_5g), WLAN_CUST_NUM_1, OAL_FALSE},
    {"delta_cca_ed_high_80th_5g", WLAN_CFG_DELTA_CCA_ED_HIGH_80TH_5G, &g_cust_rf_front.delta_cca_ed_high_80th_5g,
     sizeof(g_cust_rf_front.delta_cca_ed_high_80th_5g), WLAN_CUST_NUM_1, OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"delta_cca_ed_high_20th_6g", WLAN_CFG_DELTA_CCA_ED_HIGH_20TH_6G, &g_cust_rf_front.delta_cca_ed_high_20th_6g,
     sizeof(g_cust_rf_front.delta_cca_ed_high_20th_6g), WLAN_CUST_NUM_1, OAL_FALSE},
    {"delta_cca_ed_high_40th_6g", WLAN_CFG_DELTA_CCA_ED_HIGH_40TH_6G, &g_cust_rf_front.delta_cca_ed_high_40th_6g,
     sizeof(g_cust_rf_front.delta_cca_ed_high_40th_6g), WLAN_CUST_NUM_1, OAL_FALSE},
    {"delta_cca_ed_high_80th_6g", WLAN_CFG_DELTA_CCA_ED_HIGH_80TH_6G, &g_cust_rf_front.delta_cca_ed_high_80th_6g,
     sizeof(g_cust_rf_front.delta_cca_ed_high_80th_6g), WLAN_CUST_NUM_1, OAL_FALSE},
#endif
    {NULL, 0, 0, 0, 0, OAL_FALSE},
};

static wlan_cust_cfg_cmd g_wifi_cfg_dyn_pow[] = {
    /* 产侧nvram参数 */
    {"nvram_pa2gccka0",  WLAN_CFG_DYN_POW_RATIO_PA2GCCKA0,
     &g_cust_dyn_pow.pa2gccka_para[WLAN_RF_CHANNEL_ZERO].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa2gccka_para[WLAN_RF_CHANNEL_ZERO].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) / sizeof(g_cust_dyn_pow.pa2gccka_para[WLAN_RF_CHANNEL_ZERO].l_pow_par2), OAL_FALSE},
    {"nvram_pa2ga0",     WLAN_CFG_DYN_POW_RATIO_PA2GA0,
     &g_cust_dyn_pow.pa2ga_para[WLAN_RF_CHANNEL_ZERO].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa2ga_para[WLAN_RF_CHANNEL_ZERO].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) / sizeof(g_cust_dyn_pow.pa2ga_para[WLAN_RF_CHANNEL_ZERO].l_pow_par2), OAL_FALSE},
    {"nvram_pa2g40a0",   WLAN_CFG_DYN_POW_RATIO_PA2G40A0,
     &g_cust_dyn_pow.pa2g40a_para[WLAN_RF_CHANNEL_ZERO].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa2g40a_para[WLAN_RF_CHANNEL_ZERO].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) / sizeof(g_cust_dyn_pow.pa2g40a_para[WLAN_RF_CHANNEL_ZERO].l_pow_par2), OAL_FALSE},
    {"nvram_pa5ga0",     WLAN_CFG_DYN_POW_RATIO_PA5GA0,
     &g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_ZERO][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_ZERO][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2),
     RF_DYN_POW_BAND_NUM * sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_ZERO][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2),
     OAL_FALSE},
    {"nvram_pa5ga0_low", WLAN_CFG_DYN_POW_RATIO_PA5GA0_LOW,
     &g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_ZERO][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_ZERO][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2),
     RF_DYN_POW_BAND_NUM * sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_ZERO][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2), OAL_FALSE},
    {"nvram_pa5ga0_band1", WLAN_CFG_DYN_POW_RATIO_PA5GA0_BAND1,
     &g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_ZERO][WLAN_RF_DYN_POW_BAND_1].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_ZERO][WLAN_RF_DYN_POW_BAND_1].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_ZERO][WLAN_RF_DYN_POW_BAND_1].l_pow_par2), OAL_FALSE},
    {"nvram_pa5ga0_band1_low", WLAN_CFG_DYN_POW_RATIO_PA5GA0_BAND1_LOW,
     &g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_ZERO][WLAN_RF_DYN_POW_BAND_1].l_pow_par2,
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_ZERO][WLAN_RF_DYN_POW_BAND_1].l_pow_par2), OAL_FALSE},

    {"nvram_pa2gccka1",  WLAN_CFG_DYN_POW_RATIO_PA2GCCKA1,
     &g_cust_dyn_pow.pa2gccka_para[WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa2gccka_para[WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) / sizeof(g_cust_dyn_pow.pa2gccka_para[WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa2ga1", WLAN_CFG_DYN_POW_RATIO_PA2GA1, &g_cust_dyn_pow.pa2ga_para[WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa2ga_para[WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) / sizeof(g_cust_dyn_pow.pa2ga_para[WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa2g40a1", WLAN_CFG_DYN_POW_RATIO_PA2G40A1, &g_cust_dyn_pow.pa2g40a_para[WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa2g40a_para[WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) / sizeof(g_cust_dyn_pow.pa2g40a_para[WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa5ga1", WLAN_CFG_DYN_POW_RATIO_PA5GA1,
     &g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_ONE][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_ONE][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2),
     RF_DYN_POW_BAND_NUM * sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_ONE][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2), OAL_FALSE},
    {"nvram_pa5ga1_low", WLAN_CFG_DYN_POW_RATIO_PA5GA1_LOW,
     &g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_ONE][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_ONE][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2),
     RF_DYN_POW_BAND_NUM * sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_ONE][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2), OAL_FALSE},
    {"nvram_pa5ga1_band1", WLAN_CFG_DYN_POW_RATIO_PA5GA1_BAND1,
     &g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_ONE][WLAN_RF_DYN_POW_BAND_1].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_ONE][WLAN_RF_DYN_POW_BAND_1].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_ONE][WLAN_RF_DYN_POW_BAND_1].l_pow_par2), OAL_FALSE},
    {"nvram_pa5ga1_band1_low", WLAN_CFG_DYN_POW_RATIO_PA5GA1_BAND1_LOW,
     &g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_ONE][WLAN_RF_DYN_POW_BAND_1].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_ONE][WLAN_RF_DYN_POW_BAND_1].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_ONE][WLAN_RF_DYN_POW_BAND_1].l_pow_par2), OAL_FALSE},
    {"nvram_pa2gccka2",  WLAN_CFG_DYN_POW_RATIO_PA2GCCKA2,
     &g_cust_dyn_pow.pa2gccka_para[WLAN_RF_CHANNEL_TWO].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa2gccka_para[WLAN_RF_CHANNEL_TWO].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) / sizeof(g_cust_dyn_pow.pa2gccka_para[WLAN_RF_CHANNEL_TWO].l_pow_par2), OAL_FALSE},
    {"nvram_pa2ga2",     WLAN_CFG_DYN_POW_RATIO_PA2GA2,
     &g_cust_dyn_pow.pa2ga_para[WLAN_RF_CHANNEL_TWO].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa2ga_para[WLAN_RF_CHANNEL_TWO].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) / sizeof(g_cust_dyn_pow.pa2ga_para[WLAN_RF_CHANNEL_TWO].l_pow_par2), OAL_FALSE},
    {"nvram_pa2g40a2",   WLAN_CFG_DYN_POW_RATIO_PA2G40A2,
     &g_cust_dyn_pow.pa2g40a_para[WLAN_RF_CHANNEL_TWO].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa2g40a_para[WLAN_RF_CHANNEL_TWO].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) / sizeof(g_cust_dyn_pow.pa2g40a_para[WLAN_RF_CHANNEL_TWO].l_pow_par2), OAL_FALSE},
    {"nvram_pa5ga2",     WLAN_CFG_DYN_POW_RATIO_PA5GA2,
     &g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_TWO][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_TWO][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2),
     RF_DYN_POW_BAND_NUM * sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_TWO][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2), OAL_FALSE},
    {"nvram_pa5ga2_low", WLAN_CFG_DYN_POW_RATIO_PA5GA2_LOW,
     &g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_TWO][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_TWO][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2),
     RF_DYN_POW_BAND_NUM * sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_TWO][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2), OAL_FALSE},
    {"nvram_pa5ga1_band1",     WLAN_CFG_DYN_POW_RATIO_PA5GA2_BAND1,
     &g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_TWO][WLAN_RF_DYN_POW_BAND_1].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_TWO][WLAN_RF_DYN_POW_BAND_1].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_TWO][WLAN_RF_DYN_POW_BAND_1].l_pow_par2), OAL_FALSE},
    {"nvram_pa5ga1_band1_low", WLAN_CFG_DYN_POW_RATIO_PA5GA2_BAND1_LOW,
     &g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_TWO][WLAN_RF_DYN_POW_BAND_1].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_TWO][WLAN_RF_DYN_POW_BAND_1].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_TWO][WLAN_RF_DYN_POW_BAND_1].l_pow_par2), OAL_FALSE},
    {"nvram_pa2gccka3",  WLAN_CFG_DYN_POW_RATIO_PA2GCCKA3,
     &g_cust_dyn_pow.pa2gccka_para[WLAN_RF_CHANNEL_THREE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa2gccka_para[WLAN_RF_CHANNEL_THREE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) / sizeof(g_cust_dyn_pow.pa2gccka_para[WLAN_RF_CHANNEL_THREE].l_pow_par2), OAL_FALSE},
    {"nvram_pa2ga2",     WLAN_CFG_DYN_POW_RATIO_PA2GA3,
     &g_cust_dyn_pow.pa2ga_para[WLAN_RF_CHANNEL_THREE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa2ga_para[WLAN_RF_CHANNEL_THREE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) / sizeof(g_cust_dyn_pow.pa2ga_para[WLAN_RF_CHANNEL_THREE].l_pow_par2), OAL_FALSE},
    {"nvram_pa2g40a3",   WLAN_CFG_DYN_POW_RATIO_PA2G40A3,
     &g_cust_dyn_pow.pa2g40a_para[WLAN_RF_CHANNEL_THREE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa2g40a_para[WLAN_RF_CHANNEL_THREE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) / sizeof(g_cust_dyn_pow.pa2g40a_para[WLAN_RF_CHANNEL_THREE].l_pow_par2), OAL_FALSE},
    {"nvram_pa5ga3",     WLAN_CFG_DYN_POW_RATIO_PA5GA3,
     &g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_THREE][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_THREE][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2),
     RF_DYN_POW_BAND_NUM * sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_THREE][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2), OAL_FALSE},
    {"nvram_pa5ga3_low", WLAN_CFG_DYN_POW_RATIO_PA5GA3_LOW,
     &g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_THREE][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_THREE][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2),
     RF_DYN_POW_BAND_NUM * sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_THREE][WLAN_RF_DYN_POW_BAND_2_3].l_pow_par2), OAL_FALSE},
    {"nvram_pa5ga3_band1",     WLAN_CFG_DYN_POW_RATIO_PA5GA3_BAND1,
     &g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_THREE][WLAN_RF_DYN_POW_BAND_1].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_THREE][WLAN_RF_DYN_POW_BAND_1].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa5ga_para[WLAN_RF_CHANNEL_THREE][WLAN_RF_DYN_POW_BAND_1].l_pow_par2), OAL_FALSE},
    {"nvram_pa5ga3_band1_low", WLAN_CFG_DYN_POW_RATIO_PA5GA3_BAND1_LOW,
     &g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_THREE][WLAN_RF_DYN_POW_BAND_1].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_THREE][WLAN_RF_DYN_POW_BAND_1].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa5ga_low_para[WLAN_RF_CHANNEL_THREE][WLAN_RF_DYN_POW_BAND_1].l_pow_par2), OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"nvram_pa6g_band0", WLAN_CFG_DYN_POW_RATIO_PA6G_BAND0,
     &g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX0][WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX0][WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX0][WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa6g_band1", WLAN_CFG_DYN_POW_RATIO_PA6G_BAND1,
     &g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX1][WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX1][WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX1][WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa6g_band2", WLAN_CFG_DYN_POW_RATIO_PA6G_BAND2,
     &g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX2][WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX2][WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX2][WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa6g_band3", WLAN_CFG_DYN_POW_RATIO_PA6G_BAND3,
     &g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX3][WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX3][WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX3][WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa6g_band4", WLAN_CFG_DYN_POW_RATIO_PA6G_BAND4,
     &g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX4][WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX4][WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX4][WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa6g_band5", WLAN_CFG_DYN_POW_RATIO_PA6G_BAND5,
     &g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX5][WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX5][WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX5][WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa6g_band6", WLAN_CFG_DYN_POW_RATIO_PA6G_BAND6,
     &g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX6][WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX6][WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX6][WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa6g_band7", WLAN_CFG_DYN_POW_RATIO_PA6G_BAND7,
     &g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX7][WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX7][WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX7][WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa6g_band8", WLAN_CFG_DYN_POW_RATIO_PA6G_BAND8,
     &g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX8][WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX8][WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX8][WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa6g_band9", WLAN_CFG_DYN_POW_RATIO_PA6G_BAND9,
     &g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX9][WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX9][WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX9][WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa6g_band10", WLAN_CFG_DYN_POW_RATIO_PA6G_BAND10,
     &g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX10][WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX10][WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX10][WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa6g_band11", WLAN_CFG_DYN_POW_RATIO_PA6G_BAND11,
     &g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX11][WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX11][WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX11][WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa6g_band12", WLAN_CFG_DYN_POW_RATIO_PA6G_BAND12,
     &g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX12][WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX12][WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX12][WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa6g_band13", WLAN_CFG_DYN_POW_RATIO_PA6G_BAND13,
     &g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX13][WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX13][WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX13][WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
    {"nvram_pa6g_band14", WLAN_CFG_DYN_POW_RATIO_PA6G_BAND14,
     &g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX14][WLAN_RF_CHANNEL_ONE].l_pow_par2,
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX14][WLAN_RF_CHANNEL_ONE].l_pow_par2),
     sizeof(hal_pwr_fit_para_stru) /
     sizeof(g_cust_dyn_pow.pa6ga_para[WLAN_6G_BAND_IDX14][WLAN_RF_CHANNEL_ONE].l_pow_par2), OAL_FALSE},
#endif
    {NULL, WLAN_CFG_DYN_POW_MUFREQ_2GCCK_C0, NULL, 0, 0, OAL_FALSE},
    {NULL, WLAN_CFG_DYN_POW_MUFREQ_2G20_C0, NULL, 0, 0, OAL_FALSE},
    {NULL, WLAN_CFG_DYN_POW_MUFREQ_2G40_C0, NULL, 0, 0, OAL_FALSE},
    {NULL, WLAN_CFG_DYN_POW_MUFREQ_5G160_C0, NULL, 0, 0, OAL_FALSE},

    {NULL, WLAN_CFG_DYN_POW_MUFREQ_2GCCK_C1, NULL, 0, 0, OAL_FALSE},
    {NULL, WLAN_CFG_DYN_POW_MUFREQ_2G20_C1, NULL, 0, 0, OAL_FALSE},
    {NULL, WLAN_CFG_DYN_POW_MUFREQ_2G40_C1, NULL, 0, 0, OAL_FALSE},
    {NULL, WLAN_CFG_DYN_POW_MUFREQ_5G160_C1, NULL, 0, 0, OAL_FALSE},

    {NULL, WLAN_CFG_DYN_POW_MUFREQ_2GCCK_C2, NULL, 0, 0, OAL_FALSE},
    {NULL, WLAN_CFG_DYN_POW_MUFREQ_2G20_C2, NULL, 0, 0, OAL_FALSE},
    {NULL, WLAN_CFG_DYN_POW_MUFREQ_2G40_C2, NULL, 0, 0, OAL_FALSE},
    {NULL, WLAN_CFG_DYN_POW_MUFREQ_5G160_C2, NULL, 0, 0, OAL_FALSE},

    {NULL, WLAN_CFG_DYN_POW_MUFREQ_2GCCK_C3, NULL, 0, 0, OAL_FALSE},
    {NULL, WLAN_CFG_DYN_POW_MUFREQ_2G20_C3, NULL, 0, 0, OAL_FALSE},
    {NULL, WLAN_CFG_DYN_POW_MUFREQ_2G40_C3, NULL, 0, 0, OAL_FALSE},
    {NULL, WLAN_CFG_DYN_POW_MUFREQ_5G160_C3, NULL, 0, 0, OAL_FALSE},

    /* DPN */
    {"dpn24g_11b_core0", WLAN_CFG_DYN_POW_2G_CORE0_DPN_11B,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B][WLAN_2G_CHN_IDX0],
     sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B]), OAL_FALSE},
    {"dpn24g_11b_core1", WLAN_CFG_DYN_POW_2G_CORE1_DPN_11B,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B][WLAN_2G_CHN_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B]), OAL_FALSE},
    {"dpn24g_11b_core2", WLAN_CFG_DYN_POW_2G_CORE2_DPN_11B,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B][WLAN_2G_CHN_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B]), OAL_FALSE},
    {"dpn24g_11b_core3", WLAN_CFG_DYN_POW_2G_CORE3_DPN_11B,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B][WLAN_2G_CHN_IDX0],
     sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B]), OAL_FALSE},
    {"dpn24g_20m_core0", WLAN_CFG_DYN_POW_2G_CORE0_DPN_OFDM_20M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20][WLAN_2G_CHN_IDX0],
     sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20]), OAL_FALSE},
    {"dpn24g_20m_core1", WLAN_CFG_DYN_POW_2G_CORE1_DPN_OFDM_20M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20][WLAN_2G_CHN_IDX0],
     sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20]), OAL_FALSE},
    {"dpn24g_20m_core2", WLAN_CFG_DYN_POW_2G_CORE2_DPN_OFDM_20M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20][WLAN_2G_CHN_IDX0],
     sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20]), OAL_FALSE},
    {"dpn24g_20m_core3", WLAN_CFG_DYN_POW_2G_CORE3_DPN_OFDM_20M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20][WLAN_2G_CHN_IDX0],
     sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20]), OAL_FALSE},
    {"dpn24g_40m_core0", WLAN_CFG_DYN_POW_2G_CORE0_DPN_OFDM_40M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40][WLAN_2G_CHN_IDX0],
     sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40]), OAL_FALSE},
    {"dpn24g_40m_core1", WLAN_CFG_DYN_POW_2G_CORE1_DPN_OFDM_40M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40][WLAN_2G_CHN_IDX0],
     sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40]), OAL_FALSE},
    {"dpn24g_40m_core2", WLAN_CFG_DYN_POW_2G_CORE2_DPN_OFDM_40M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40][WLAN_2G_CHN_IDX0],
     sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40]), OAL_FALSE},
    {"dpn24g_40m_core3", WLAN_CFG_DYN_POW_2G_CORE3_DPN_OFDM_40M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40][WLAN_2G_CHN_IDX0],
     sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40]), OAL_FALSE},
    {"dpn5g_20m_core0", WLAN_CFG_DYN_POW_5G_CORE0_DPN_20M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_20M][WLAN_5G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_20M]), OAL_FALSE},
    {"dpn5g_20m_core1", WLAN_CFG_DYN_POW_5G_CORE1_DPN_20M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_20M][WLAN_5G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_20M]), OAL_FALSE},
    {"dpn5g_20m_core2", WLAN_CFG_DYN_POW_5G_CORE2_DPN_20M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_20M][WLAN_5G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_20M]), OAL_FALSE},
    {"dpn5g_20m_core3", WLAN_CFG_DYN_POW_5G_CORE3_DPN_20M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_20M][WLAN_5G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_20M]), OAL_FALSE},
    {"dpn5g_40m_core0", WLAN_CFG_DYN_POW_5G_CORE0_DPN_40M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_40M][WLAN_5G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_40M]), OAL_FALSE},
    {"dpn5g_40m_core1", WLAN_CFG_DYN_POW_5G_CORE1_DPN_40M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_40M][WLAN_5G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_40M]), OAL_FALSE},
    {"dpn5g_40m_core2", WLAN_CFG_DYN_POW_5G_CORE2_DPN_40M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_40M][WLAN_5G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_40M]), OAL_FALSE},
    {"dpn5g_40m_core3", WLAN_CFG_DYN_POW_5G_CORE3_DPN_40M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_40M][WLAN_5G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_40M]), OAL_FALSE},
    {"dpn5g_80m_core0", WLAN_CFG_DYN_POW_5G_CORE0_DPN_80M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_80M][WLAN_5G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_80M]), OAL_FALSE},
    {"dpn5g_80m_core1", WLAN_CFG_DYN_POW_5G_CORE1_DPN_80M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_80M][WLAN_5G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_80M]), OAL_FALSE},
    {"dpn5g_80m_core2", WLAN_CFG_DYN_POW_5G_CORE2_DPN_80M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_80M][WLAN_5G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_80M]), OAL_FALSE},
    {"dpn5g_80m_core3", WLAN_CFG_DYN_POW_5G_CORE3_DPN_80M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_80M][WLAN_5G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_80M]), OAL_FALSE},
    {"dpn5g_160m_core0", WLAN_CFG_DYN_POW_5G_CORE0_DPN_160M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_160M][WLAN_5G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_160M]), OAL_FALSE},
    {"dpn5g_160m_core1", WLAN_CFG_DYN_POW_5G_CORE1_DPN_160M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_160M][WLAN_5G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_160M]), OAL_FALSE},
    {"dpn5g_160m_core2", WLAN_CFG_DYN_POW_5G_CORE2_DPN_160M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_160M][WLAN_5G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_160M]), OAL_FALSE},
    {"dpn5g_160m_core3", WLAN_CFG_DYN_POW_5G_CORE3_DPN_160M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_160M][WLAN_5G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_160M]), OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"dpn6g_20m_core0", WLAN_CFG_DYN_POW_6G_CORE0_DPN_20M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_20M][WLAN_6G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_20M]), OAL_FALSE},
    {"dpn6g_20m_core1", WLAN_CFG_DYN_POW_6G_CORE1_DPN_20M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_20M][WLAN_6G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_20M]), OAL_FALSE},
    {"dpn6g_20m_core2", WLAN_CFG_DYN_POW_6G_CORE2_DPN_20M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_20M][WLAN_6G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_20M]), OAL_FALSE},
    {"dpn6g_20m_core3", WLAN_CFG_DYN_POW_6G_CORE3_DPN_20M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_20M][WLAN_6G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_20M]), OAL_FALSE},
    {"dpn6g_40m_core0", WLAN_CFG_DYN_POW_6G_CORE0_DPN_40M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_40M][WLAN_6G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_40M]), OAL_FALSE},
    {"dpn6g_40m_core1", WLAN_CFG_DYN_POW_6G_CORE1_DPN_40M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_40M][WLAN_6G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_40M]), OAL_FALSE},
    {"dpn6g_40m_core2", WLAN_CFG_DYN_POW_6G_CORE2_DPN_40M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_40M][WLAN_6G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_40M]), OAL_FALSE},
    {"dpn6g_40m_core3", WLAN_CFG_DYN_POW_6G_CORE3_DPN_40M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_40M][WLAN_6G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_40M]), OAL_FALSE},
    {"dpn6g_80m_core0", WLAN_CFG_DYN_POW_6G_CORE0_DPN_80M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_80M][WLAN_6G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_80M]), OAL_FALSE},
    {"dpn6g_80m_core1", WLAN_CFG_DYN_POW_6G_CORE1_DPN_80M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_80M][WLAN_6G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_80M]), OAL_FALSE},
    {"dpn6g_80m_core2", WLAN_CFG_DYN_POW_6G_CORE2_DPN_80M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_80M][WLAN_6G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_80M]), OAL_FALSE},
    {"dpn6g_80m_core3", WLAN_CFG_DYN_POW_6G_CORE3_DPN_80M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_80M][WLAN_6G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_80M]), OAL_FALSE},
    {"dpn6g_160m_core0", WLAN_CFG_DYN_POW_6G_CORE0_DPN_160M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_160M][WLAN_6G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_160M]), OAL_FALSE},
    {"dpn6g_160m_core1", WLAN_CFG_DYN_POW_6G_CORE1_DPN_160M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_160M][WLAN_6G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_160M]), OAL_FALSE},
    {"dpn6g_160m_core2", WLAN_CFG_DYN_POW_6G_CORE2_DPN_160M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_160M][WLAN_6G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_160M]), OAL_FALSE},
    {"dpn6g_160m_core3", WLAN_CFG_DYN_POW_6G_CORE3_DPN_160M,
     &g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_160M][WLAN_6G_BAND_IDX0], sizeof(int8_t),
     ARRAY_SIZE(g_cust_dyn_pow.dpn_para[WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_160M]), OAL_FALSE},
#endif
    { NULL, WLAN_CFG_DYN_POW_PARAMS_BUTT, NULL, 0, 0, OAL_FALSE},
};

static wlan_cust_cfg_cmd g_wifi_cfg_pow[] = {
    {"nvram_params0",  NVRAM_PARA_INDEX_0,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_2P4G_11B_1M_2M], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params1",  NVRAM_PARA_INDEX_1,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_2P4G_11G_24M_36M], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params2",  NVRAM_PARA_INDEX_2, &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_2P4G_HT20_VHT20_MCS2_3],
      sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params3",  NVRAM_PARA_INDEX_3,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_2P4G_VHT20_MCS8], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params4",  NVRAM_PARA_INDEX_4,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_2P4G_HT40_VHT40_MCS6], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params5",  NVRAM_PARA_INDEX_5,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_2P4G_HT40_MCS32], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params6",  NVRAM_PARA_INDEX_6,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_5G_11A_48M], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params7",  NVRAM_PARA_INDEX_7,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_5G_HT20_VHT20_MCS4_5], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params8",  NVRAM_PARA_INDEX_8,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_5G_HT40_VHT40_MCS0_1], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params9",  NVRAM_PARA_INDEX_9,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_5G_HT40_VHT40_MCS7], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params10", NVRAM_PARA_INDEX_10,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_5G_VHT80_MCS2_3], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params11", NVRAM_PARA_INDEX_11,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_5G_VHT80_MCS8], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params12", NVRAM_PARA_INDEX_12,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_2P4G_VHT40_MCS10], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params13", NVRAM_PARA_INDEX_13,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_5G_VHT20_MCS11], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params14", NVRAM_PARA_INDEX_14,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_5G_VHT80_MCS11], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params15", NVRAM_PARA_INDEX_15,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_5G_VHT160_MCS3], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params16", NVRAM_PARA_INDEX_16,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_5G_VHT160_MCS7], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    {"nvram_params17", NVRAM_PARA_INDEX_17,
     &g_cust_pow.delt_txpwr_params[CUST_DELTA_POW_5G_VHT160_MCS11], sizeof(uint32_t), WLAN_CUST_NUM_1, OAL_FALSE},
    /* 5G cali upper upc limit */
    {"max_upc_limit", NVRAM_PARA_INDEX_IQ_MAX_UPC, &g_cust_pow.upper_upc_5g_params[0],
     sizeof(g_cust_pow.upper_upc_5g_params[0]), ARRAY_SIZE(g_cust_pow.upper_upc_5g_params), OAL_FALSE},
    {"overrun_backoff_pow", NVRAM_PARA_BACKOFF_POW, &g_cust_pow.backoff_pow_5g_params[0],
     sizeof(g_cust_pow.backoff_pow_5g_params[0]), ARRAY_SIZE(g_cust_pow.backoff_pow_5g_params), OAL_FALSE},
    /* 2G low pow amend */
    {"dsss_low_pow_amend_2g", NVRAM_PARA_DSSS_2G_LOW_POW_AMEND, &g_cust_pow.dsss_low_pow_amend_2g[0],
     sizeof(g_cust_pow.dsss_low_pow_amend_2g[0]), ARRAY_SIZE(g_cust_pow.dsss_low_pow_amend_2g), OAL_FALSE},
    {"ofdm_low_pow_amend_2g", NVRAM_PARA_OFDM_2G_LOW_POW_AMEND, &g_cust_pow.ofdm_low_pow_amend_2g[0],
     sizeof(g_cust_pow.ofdm_low_pow_amend_2g[0]), ARRAY_SIZE(g_cust_pow.ofdm_low_pow_amend_2g), OAL_FALSE},
    {NULL, NVRAM_PARA_TXPWR_INDEX_BUTT, NULL, 0, 0, OAL_FALSE},

    {"nvram_max_txpwr_base_2p4g_c0", NVRAM_PARA_BASE_TXPWR_2G_C0,
     &g_cust_pow.txpwr_base_2g_params[WLAN_RF_CHANNEL_ZERO][WLAN_2G_BAND_IDX0],
     sizeof(g_cust_pow.txpwr_base_2g_params[WLAN_RF_CHANNEL_ZERO][WLAN_2G_BAND_IDX0]),
     ARRAY_SIZE(g_cust_pow.txpwr_base_2g_params[WLAN_RF_CHANNEL_ZERO]), OAL_FALSE},
    {"nvram_max_txpwr_base_2p4g_c1", NVRAM_PARA_BASE_TXPWR_2G_C1,
     &g_cust_pow.txpwr_base_2g_params[WLAN_RF_CHANNEL_ONE][WLAN_2G_BAND_IDX0],
     sizeof(g_cust_pow.txpwr_base_2g_params[WLAN_RF_CHANNEL_ONE][WLAN_2G_BAND_IDX0]),
     ARRAY_SIZE(g_cust_pow.txpwr_base_2g_params[WLAN_RF_CHANNEL_ONE]), OAL_FALSE},
    {"nvram_max_txpwr_base_2p4g_c2", NVRAM_PARA_BASE_TXPWR_2G_C2,
     &g_cust_pow.txpwr_base_2g_params[WLAN_RF_CHANNEL_TWO][WLAN_2G_BAND_IDX0],
     sizeof(g_cust_pow.txpwr_base_2g_params[WLAN_RF_CHANNEL_TWO][WLAN_2G_BAND_IDX0]),
     ARRAY_SIZE(g_cust_pow.txpwr_base_2g_params[WLAN_RF_CHANNEL_TWO]), OAL_FALSE},
    {"nvram_max_txpwr_base_2p4g_c3", NVRAM_PARA_BASE_TXPWR_2G_C3,
     &g_cust_pow.txpwr_base_2g_params[WLAN_RF_CHANNEL_THREE][WLAN_2G_BAND_IDX0],
     sizeof(g_cust_pow.txpwr_base_2g_params[WLAN_RF_CHANNEL_THREE][WLAN_2G_BAND_IDX0]),
     ARRAY_SIZE(g_cust_pow.txpwr_base_2g_params[WLAN_RF_CHANNEL_THREE]), OAL_FALSE},
    {"nvram_max_txpwr_base_5g_c0", NVRAM_PARA_BASE_TXPWR_5G_C0,
     &g_cust_pow.txpwr_base_5g_params[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     sizeof(g_cust_pow.txpwr_base_5g_params[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0]),
     ARRAY_SIZE(g_cust_pow.txpwr_base_5g_params[WLAN_RF_CHANNEL_ZERO]), OAL_FALSE},
    {"nvram_max_txpwr_base_5g_c1", NVRAM_PARA_BASE_TXPWR_5G_C1,
     &g_cust_pow.txpwr_base_5g_params[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     sizeof(g_cust_pow.txpwr_base_5g_params[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0]),
     ARRAY_SIZE(g_cust_pow.txpwr_base_5g_params[WLAN_RF_CHANNEL_ONE]), OAL_FALSE},
    {"nvram_max_txpwr_base_5g_c2", NVRAM_PARA_BASE_TXPWR_5G_C2,
     &g_cust_pow.txpwr_base_5g_params[WLAN_RF_CHANNEL_TWO][WLAN_5G_BAND_IDX0],
     sizeof(g_cust_pow.txpwr_base_5g_params[WLAN_RF_CHANNEL_TWO][WLAN_5G_BAND_IDX0]),
     ARRAY_SIZE(g_cust_pow.txpwr_base_5g_params[WLAN_RF_CHANNEL_TWO]), OAL_FALSE},
    {"nvram_max_txpwr_base_5g_c3", NVRAM_PARA_BASE_TXPWR_5G_C3,
     &g_cust_pow.txpwr_base_5g_params[WLAN_RF_CHANNEL_THREE][WLAN_5G_BAND_IDX0],
     sizeof(g_cust_pow.txpwr_base_5g_params[WLAN_RF_CHANNEL_THREE][WLAN_5G_BAND_IDX0]),
     ARRAY_SIZE(g_cust_pow.txpwr_base_5g_params[WLAN_RF_CHANNEL_THREE]), OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"nvram_max_txpwr_base_6g_c0", NVRAM_PARA_BASE_TXPWR_6G_C0,
     &g_cust_pow.txpwr_base_6g_params[WLAN_RF_CHANNEL_ZERO][WLAN_6G_BAND_IDX0],
     sizeof(g_cust_pow.txpwr_base_6g_params[WLAN_RF_CHANNEL_ZERO][WLAN_6G_BAND_IDX0]),
     ARRAY_SIZE(g_cust_pow.txpwr_base_6g_params[WLAN_RF_CHANNEL_ZERO]), OAL_FALSE},
    {"nvram_max_txpwr_base_6g_c1", NVRAM_PARA_BASE_TXPWR_6G_C1,
     &g_cust_pow.txpwr_base_6g_params[WLAN_RF_CHANNEL_ONE][WLAN_6G_BAND_IDX0],
     sizeof(g_cust_pow.txpwr_base_6g_params[WLAN_RF_CHANNEL_ONE][WLAN_6G_BAND_IDX0]),
     ARRAY_SIZE(g_cust_pow.txpwr_base_6g_params[WLAN_RF_CHANNEL_ONE]), OAL_FALSE},
    {"nvram_max_txpwr_base_6g_c2", NVRAM_PARA_BASE_TXPWR_6G_C2,
     &g_cust_pow.txpwr_base_6g_params[WLAN_RF_CHANNEL_TWO][WLAN_6G_BAND_IDX0],
     sizeof(g_cust_pow.txpwr_base_6g_params[WLAN_RF_CHANNEL_TWO][WLAN_6G_BAND_IDX0]),
     ARRAY_SIZE(g_cust_pow.txpwr_base_6g_params[WLAN_RF_CHANNEL_TWO]), OAL_FALSE},
    {"nvram_max_txpwr_base_6g_c3", NVRAM_PARA_BASE_TXPWR_6G_C3,
     &g_cust_pow.txpwr_base_6g_params[WLAN_RF_CHANNEL_THREE][WLAN_6G_BAND_IDX0],
     sizeof(g_cust_pow.txpwr_base_6g_params[WLAN_RF_CHANNEL_THREE][WLAN_6G_BAND_IDX0]),
     ARRAY_SIZE(g_cust_pow.txpwr_base_6g_params[WLAN_RF_CHANNEL_THREE]), OAL_FALSE},
#endif
    {NULL, NVRAM_PARA_BASE_INDEX_BUTT, NULL, 0, 0, OAL_FALSE},
    {"nvram_2g_delt_max_base_txpwr", NVRAM_PARA_2G_DELT_BASE_POWER_23,
     &g_cust_pow.delt_txpwr_base_params[WLAN_CALI_BAND_2G][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_pow.delt_txpwr_base_params[WLAN_CALI_BAND_2G][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_pow.delt_txpwr_base_params[WLAN_CALI_BAND_2G]), OAL_FALSE},
    {"nvram_5g_delt_max_base_txpwr", NVRAM_PARA_5G_DELT_BASE_POWER_23,
     &g_cust_pow.delt_txpwr_base_params[WLAN_CALI_BAND_5G][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_pow.delt_txpwr_base_params[WLAN_CALI_BAND_5G][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_pow.delt_txpwr_base_params[WLAN_CALI_BAND_5G]), OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"nvram_6g_delt_max_base_txpwr", NVRAM_PARA_6G_DELT_BASE_POWER_23,
     &g_cust_pow.delt_txpwr_base_params[WLAN_CALI_BAND_6G][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_pow.delt_txpwr_base_params[WLAN_CALI_BAND_6G][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_pow.delt_txpwr_base_params[WLAN_CALI_BAND_6G]), OAL_FALSE},
#endif
    {NULL, NVRAM_PARA_INDEX_24_RSV, NULL, 0, 0, OAL_FALSE},
    /* FCC C0 */
    {"fcc_side_band_txpwr_limit_5g_20m_c0",   NVRAM_PARA_FCC_5G_20M_C0, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_5g_40m_c0",   NVRAM_PARA_FCC_5G_40M_C0, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_5g_80m_c0",   NVRAM_PARA_FCC_5G_80M_C0, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_5g_160m_c0",  NVRAM_PARA_FCC_5G_160M_C0, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch1_c0",  NVRAM_PARA_FCC_2P4_C0_CH1, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch2_c0",  NVRAM_PARA_FCC_2P4_C0_CH2, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch3_c0",  NVRAM_PARA_FCC_2P4_C0_CH3, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch4_c0",  NVRAM_PARA_FCC_2P4_C0_CH4, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch5_c0",  NVRAM_PARA_FCC_2P4_C0_CH5, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch6_c0",  NVRAM_PARA_FCC_2P4_C0_CH6, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch7_c0",  NVRAM_PARA_FCC_2P4_C0_CH7, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch8_c0",  NVRAM_PARA_FCC_2P4_C0_CH8, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch9_c0",  NVRAM_PARA_FCC_2P4_C0_CH9, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch10_c0", NVRAM_PARA_FCC_2P4_C0_CH10, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch11_c0", NVRAM_PARA_FCC_2P4_C0_CH11, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch12_c0", NVRAM_PARA_FCC_2P4_C0_CH12, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch13_c0", NVRAM_PARA_FCC_2P4_C0_CH13, NULL, 0, 0, OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"fcc_side_band_txpwr_limit_6g_20m_c0",   NVRAM_PARA_FCC_6G_20M_C0, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_6g_40m_c0",   NVRAM_PARA_FCC_6G_40M_C0, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_6g_80m_c0",   NVRAM_PARA_FCC_6G_80M_C0, NULL, 0, 0, OAL_FALSE},
    {"fcc_sub_side_band_txpwr_limit_6g_20m_c0",   NVRAM_PARA_FCC_SUB_SIDE_6G_20M_C0, NULL, 0, 0, OAL_FALSE},
    {"fcc_sub_side_band_txpwr_limit_6g_40m_c0",   NVRAM_PARA_FCC_SUB_SIDE_6G_40M_C0, NULL, 0, 0, OAL_FALSE},
    {"fcc_sub_side_band_txpwr_limit_6g_80m_c0",   NVRAM_PARA_FCC_SUB_SIDE_6G_80M_C0, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_6g_160m_c0",  NVRAM_PARA_FCC_6G_160M_C0, NULL, 0, 0, OAL_FALSE},
#endif
    {NULL, NVRAM_PARA_FCC_C0_END_INDEX_BUTT,  NULL, 0, 0, OAL_FALSE},

    /* FCC C1 */
    {"fcc_side_band_txpwr_limit_5g_20m_c1", NVRAM_PARA_FCC_5G_20M_C1, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_5g_40m_c1", NVRAM_PARA_FCC_5G_40M_C1, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_5g_80m_c1", NVRAM_PARA_FCC_5G_80M_C1, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_5g_160m_c1",  NVRAM_PARA_FCC_5G_160M_C1, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch1_c1",  NVRAM_PARA_FCC_2P4_C1_CH1, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch2_c1",  NVRAM_PARA_FCC_2P4_C1_CH2, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch3_c1",  NVRAM_PARA_FCC_2P4_C1_CH3, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch4_c1",  NVRAM_PARA_FCC_2P4_C1_CH4, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch5_c1",  NVRAM_PARA_FCC_2P4_C1_CH5, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch6_c1",  NVRAM_PARA_FCC_2P4_C1_CH6, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch7_c1",  NVRAM_PARA_FCC_2P4_C1_CH7, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch8_c1",  NVRAM_PARA_FCC_2P4_C1_CH8, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch9_c1",  NVRAM_PARA_FCC_2P4_C1_CH9, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch10_c1", NVRAM_PARA_FCC_2P4_C1_CH10, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch11_c1", NVRAM_PARA_FCC_2P4_C1_CH11, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch12_c1", NVRAM_PARA_FCC_2P4_C1_CH12, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch13_c1", NVRAM_PARA_FCC_2P4_C1_CH13, NULL, 0, 0, OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"fcc_side_band_txpwr_limit_6g_20m_c1",   NVRAM_PARA_FCC_6G_20M_C1, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_6g_40m_c1",   NVRAM_PARA_FCC_6G_40M_C1, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_6g_80m_c1",   NVRAM_PARA_FCC_6G_80M_C1, NULL, 0, 0, OAL_FALSE},
    {"fcc_sub_side_band_txpwr_limit_6g_20m_c1",   NVRAM_PARA_FCC_SUB_SIDE_6G_20M_C1, NULL, 0, 0, OAL_FALSE},
    {"fcc_sub_side_band_txpwr_limit_6g_40m_c1",   NVRAM_PARA_FCC_SUB_SIDE_6G_40M_C1, NULL, 0, 0, OAL_FALSE},
    {"fcc_sub_side_band_txpwr_limit_6g_80m_c1",   NVRAM_PARA_FCC_SUB_SIDE_6G_80M_C1, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_6g_160m_c1",  NVRAM_PARA_FCC_6G_160M_C1, NULL, 0, 0, OAL_FALSE},
#endif
    {NULL, NVRAM_PARA_FCC_C1_END_INDEX_BUTT,  NULL, 0, 0, OAL_FALSE},

    /* FCC C2 */
    {"fcc_side_band_txpwr_limit_5g_20m_c2", NVRAM_PARA_FCC_5G_20M_C2, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_5g_40m_c2", NVRAM_PARA_FCC_5G_40M_C2, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_5g_80m_c2", NVRAM_PARA_FCC_5G_80M_C2, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_5g_160m_c2",  NVRAM_PARA_FCC_5G_160M_C2, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch1_c2",  NVRAM_PARA_FCC_2P4_C2_CH1, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch2_c2",  NVRAM_PARA_FCC_2P4_C2_CH2, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch3_c2",  NVRAM_PARA_FCC_2P4_C2_CH3, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch4_c2",  NVRAM_PARA_FCC_2P4_C2_CH4, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch5_c2",  NVRAM_PARA_FCC_2P4_C2_CH5, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch6_c2",  NVRAM_PARA_FCC_2P4_C2_CH6, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch7_c2",  NVRAM_PARA_FCC_2P4_C2_CH7, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch8_c2",  NVRAM_PARA_FCC_2P4_C2_CH8, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch9_c2",  NVRAM_PARA_FCC_2P4_C2_CH9, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch10_c2", NVRAM_PARA_FCC_2P4_C2_CH10, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch11_c2", NVRAM_PARA_FCC_2P4_C2_CH11, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch12_c2", NVRAM_PARA_FCC_2P4_C2_CH12, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch13_c2", NVRAM_PARA_FCC_2P4_C2_CH13, NULL, 0, 0, OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"fcc_side_band_txpwr_limit_6g_20m_c2",   NVRAM_PARA_FCC_6G_20M_C2, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_6g_40m_c2",   NVRAM_PARA_FCC_6G_40M_C2, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_6g_80m_c2",   NVRAM_PARA_FCC_6G_80M_C2, NULL, 0, 0, OAL_FALSE},
    {"fcc_sub_side_band_txpwr_limit_6g_20m_c2",   NVRAM_PARA_FCC_SUB_SIDE_6G_20M_C2, NULL, 0, 0, OAL_FALSE},
    {"fcc_sub_side_band_txpwr_limit_6g_40m_c2",   NVRAM_PARA_FCC_SUB_SIDE_6G_40M_C2, NULL, 0, 0, OAL_FALSE},
    {"fcc_sub_side_band_txpwr_limit_6g_80m_c2",   NVRAM_PARA_FCC_SUB_SIDE_6G_80M_C2, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_6g_160m_c2",  NVRAM_PARA_FCC_6G_160M_C2, NULL, 0, 0, OAL_FALSE},
#endif
    {NULL, NVRAM_PARA_FCC_C2_END_INDEX_BUTT,  NULL, 0, 0, OAL_FALSE},

    /* FCC C3 */
    {"fcc_side_band_txpwr_limit_5g_20m_c3", NVRAM_PARA_FCC_5G_20M_C3, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_5g_40m_c3", NVRAM_PARA_FCC_5G_40M_C3, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_5g_80m_c3", NVRAM_PARA_FCC_5G_80M_C3, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_5g_160m_c3", NVRAM_PARA_FCC_5G_160M_C3, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch1_c3", NVRAM_PARA_FCC_2P4_C3_CH1, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch2_c3", NVRAM_PARA_FCC_2P4_C3_CH2, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch3_c3", NVRAM_PARA_FCC_2P4_C3_CH3, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch4_c3", NVRAM_PARA_FCC_2P4_C3_CH4, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch5_c3", NVRAM_PARA_FCC_2P4_C3_CH5, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch6_c3", NVRAM_PARA_FCC_2P4_C3_CH6, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch7_c3", NVRAM_PARA_FCC_2P4_C3_CH7, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch8_c3", NVRAM_PARA_FCC_2P4_C3_CH8, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch9_c3", NVRAM_PARA_FCC_2P4_C3_CH9, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch10_c3", NVRAM_PARA_FCC_2P4_C3_CH10, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch11_c3", NVRAM_PARA_FCC_2P4_C3_CH11, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch12_c3", NVRAM_PARA_FCC_2P4_C3_CH12, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_24g_ch13_c3", NVRAM_PARA_FCC_2P4_C3_CH13, NULL, 0, 0, OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"fcc_side_band_txpwr_limit_6g_20m_c3",   NVRAM_PARA_FCC_6G_20M_C3, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_6g_40m_c3",   NVRAM_PARA_FCC_6G_40M_C3, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_6g_80m_c3",   NVRAM_PARA_FCC_6G_80M_C3, NULL, 0, 0, OAL_FALSE},
    {"fcc_sub_side_band_txpwr_limit_6g_20m_c3",   NVRAM_PARA_FCC_SUB_SIDE_6G_20M_C3, NULL, 0, 0, OAL_FALSE},
    {"fcc_sub_side_band_txpwr_limit_6g_40m_c3",   NVRAM_PARA_FCC_SUB_SIDE_6G_40M_C3, NULL, 0, 0, OAL_FALSE},
    {"fcc_sub_side_band_txpwr_limit_6g_80m_c3",   NVRAM_PARA_FCC_SUB_SIDE_6G_80M_C3, NULL, 0, 0, OAL_FALSE},
    {"fcc_side_band_txpwr_limit_6g_160m_c3",  NVRAM_PARA_FCC_6G_160M_C3, NULL, 0, 0, OAL_FALSE},
#endif
    {NULL, NVRAM_PARA_FCC_C3_END_INDEX_BUTT,  NULL, 0, 0, OAL_FALSE},

    {"ce_side_band_txpwr_limit_5g_20m_c0",   NVRAM_PARA_CE_5G_20M_C0, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_5g_40m_c0",   NVRAM_PARA_CE_5G_40M_C0, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_5g_80m_c0",   NVRAM_PARA_CE_5G_80M_C0, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_5g_160m_c0",  NVRAM_PARA_CE_5G_160M_C0, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch1_c0",  NVRAM_PARA_CE_2P4_C0_CH1, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch2_c0",  NVRAM_PARA_CE_2P4_C0_CH2, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch3_c0",  NVRAM_PARA_CE_2P4_C0_CH3, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch4_c0",  NVRAM_PARA_CE_2P4_C0_CH4, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch5_c0",  NVRAM_PARA_CE_2P4_C0_CH5, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch6_c0",  NVRAM_PARA_CE_2P4_C0_CH6, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch7_c0",  NVRAM_PARA_CE_2P4_C0_CH7, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch8_c0",  NVRAM_PARA_CE_2P4_C0_CH8, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch9_c0",  NVRAM_PARA_CE_2P4_C0_CH9, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch10_c0", NVRAM_PARA_CE_2P4_C0_CH10, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch11_c0", NVRAM_PARA_CE_2P4_C0_CH11, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch12_c0", NVRAM_PARA_CE_2P4_C0_CH12, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch13_c0", NVRAM_PARA_CE_2P4_C0_CH13, NULL, 0, 0, OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"ce_side_band_txpwr_limit_6g_20m_c0",   NVRAM_PARA_CE_6G_20M_C0, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_6g_40m_c0",   NVRAM_PARA_CE_6G_40M_C0, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_6g_80m_c0",   NVRAM_PARA_CE_6G_80M_C0, NULL, 0, 0, OAL_FALSE},
    {"ce_sub_side_band_txpwr_limit_6g_20m_c0",   NVRAM_PARA_CE_SUB_SIDE_6G_20M_C0, NULL, 0, 0, OAL_FALSE},
    {"ce_sub_side_band_txpwr_limit_6g_40m_c0",   NVRAM_PARA_CE_SUB_SIDE_6G_40M_C0, NULL, 0, 0, OAL_FALSE},
    {"ce_sub_side_band_txpwr_limit_6g_80m_c0",   NVRAM_PARA_CE_SUB_SIDE_6G_80M_C0, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_6g_160m_c0",  NVRAM_PARA_CE_6G_160M_C0, NULL, 0, 0, OAL_FALSE},
#endif
    {NULL,                        NVRAM_PARA_CE_C0_END_INDEX_BUTT, NULL, 0, 0, OAL_FALSE},

    {"ce_side_band_txpwr_limit_5g_20m_c1",   NVRAM_PARA_CE_5G_20M_C1, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_5g_40m_c1",   NVRAM_PARA_CE_5G_40M_C1, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_5g_80m_c1",   NVRAM_PARA_CE_5G_80M_C1, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_5g_160m_c1",  NVRAM_PARA_CE_5G_160M_C1, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch1_c1",  NVRAM_PARA_CE_2P4_C1_CH1, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch2_c1",  NVRAM_PARA_CE_2P4_C1_CH2, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch3_c1",  NVRAM_PARA_CE_2P4_C1_CH3, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch4_c1",  NVRAM_PARA_CE_2P4_C1_CH4, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch5_c1",  NVRAM_PARA_CE_2P4_C1_CH5, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch6_c1",  NVRAM_PARA_CE_2P4_C1_CH6, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch7_c1",  NVRAM_PARA_CE_2P4_C1_CH7, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch8_c1",  NVRAM_PARA_CE_2P4_C1_CH8, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch9_c1",  NVRAM_PARA_CE_2P4_C1_CH9, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch10_c1", NVRAM_PARA_CE_2P4_C1_CH10, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch11_c1", NVRAM_PARA_CE_2P4_C1_CH11, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch12_c1", NVRAM_PARA_CE_2P4_C1_CH12, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch13_c1", NVRAM_PARA_CE_2P4_C1_CH13, NULL, 0, 0, OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"ce_side_band_txpwr_limit_6g_20m_c1",   NVRAM_PARA_CE_6G_20M_C1, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_6g_40m_c1",   NVRAM_PARA_CE_6G_40M_C1, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_6g_80m_c1",   NVRAM_PARA_CE_6G_80M_C1, NULL, 0, 0, OAL_FALSE},
    {"ce_sub_side_band_txpwr_limit_6g_20m_c1",   NVRAM_PARA_CE_SUB_SIDE_6G_20M_C1, NULL, 0, 0, OAL_FALSE},
    {"ce_sub_side_band_txpwr_limit_6g_40m_c1",   NVRAM_PARA_CE_SUB_SIDE_6G_40M_C1, NULL, 0, 0, OAL_FALSE},
    {"ce_sub_side_band_txpwr_limit_6g_80m_c1",   NVRAM_PARA_CE_SUB_SIDE_6G_80M_C1, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_6g_160m_c1",  NVRAM_PARA_CE_6G_160M_C1, NULL, 0, 0, OAL_FALSE},
#endif
    {NULL, NVRAM_PARA_CE_C1_END_INDEX_BUTT, NULL, 0, 0, OAL_FALSE},

    {"ce_side_band_txpwr_limit_5g_20m_c2",   NVRAM_PARA_CE_5G_20M_C2, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_5g_40m_c2",   NVRAM_PARA_CE_5G_40M_C2, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_5g_80m_c2",   NVRAM_PARA_CE_5G_80M_C2, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_5g_160m_c2",  NVRAM_PARA_CE_5G_160M_C2, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch1_c2",  NVRAM_PARA_CE_2P4_C2_CH1, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch2_c2",  NVRAM_PARA_CE_2P4_C2_CH2, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch3_c2",  NVRAM_PARA_CE_2P4_C2_CH3, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch4_c2",  NVRAM_PARA_CE_2P4_C2_CH4, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch5_c2",  NVRAM_PARA_CE_2P4_C2_CH5, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch6_c2",  NVRAM_PARA_CE_2P4_C2_CH6, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch7_c2",  NVRAM_PARA_CE_2P4_C2_CH7, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch8_c2",  NVRAM_PARA_CE_2P4_C2_CH8, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch9_c2",  NVRAM_PARA_CE_2P4_C2_CH9, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch10_c2", NVRAM_PARA_CE_2P4_C2_CH10, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch11_c2", NVRAM_PARA_CE_2P4_C2_CH11, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch12_c2", NVRAM_PARA_CE_2P4_C2_CH12, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch13_c2", NVRAM_PARA_CE_2P4_C2_CH13, NULL, 0, 0, OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"ce_side_band_txpwr_limit_6g_20m_c2",   NVRAM_PARA_CE_6G_20M_C2, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_6g_40m_c2",   NVRAM_PARA_CE_6G_40M_C2, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_6g_80m_c2",   NVRAM_PARA_CE_6G_80M_C2, NULL, 0, 0, OAL_FALSE},
    {"ce_sub_side_band_txpwr_limit_6g_20m_c2",   NVRAM_PARA_CE_SUB_SIDE_6G_20M_C2, NULL, 0, 0, OAL_FALSE},
    {"ce_sub_side_band_txpwr_limit_6g_40m_c2",   NVRAM_PARA_CE_SUB_SIDE_6G_40M_C2, NULL, 0, 0, OAL_FALSE},
    {"ce_sub_side_band_txpwr_limit_6g_80m_c2",   NVRAM_PARA_CE_SUB_SIDE_6G_80M_C2, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_6g_160m_c2",  NVRAM_PARA_CE_6G_160M_C2, NULL, 0, 0, OAL_FALSE},
#endif
    {NULL, NVRAM_PARA_CE_C2_END_INDEX_BUTT, NULL, 0, 0, OAL_FALSE},

    {"ce_side_band_txpwr_limit_5g_20m_c3",   NVRAM_PARA_CE_5G_20M_C3, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_5g_40m_c3",   NVRAM_PARA_CE_5G_40M_C3, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_5g_80m_c3",   NVRAM_PARA_CE_5G_80M_C3, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_5g_160m_c3",  NVRAM_PARA_CE_5G_160M_C3, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch1_c3",  NVRAM_PARA_CE_2P4_C3_CH1, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch2_c3",  NVRAM_PARA_CE_2P4_C3_CH2, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch3_c3",  NVRAM_PARA_CE_2P4_C3_CH3, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch4_c3",  NVRAM_PARA_CE_2P4_C3_CH4, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch5_c3",  NVRAM_PARA_CE_2P4_C3_CH5, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch6_c3",  NVRAM_PARA_CE_2P4_C3_CH6, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch7_c3",  NVRAM_PARA_CE_2P4_C3_CH7, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch8_c3",  NVRAM_PARA_CE_2P4_C3_CH8, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch9_c3",  NVRAM_PARA_CE_2P4_C3_CH9, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch10_c3", NVRAM_PARA_CE_2P4_C3_CH10, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch11_c3", NVRAM_PARA_CE_2P4_C3_CH11, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch12_c3", NVRAM_PARA_CE_2P4_C3_CH12, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_24g_ch13_c3", NVRAM_PARA_CE_2P4_C3_CH13, NULL, 0, 0, OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"ce_side_band_txpwr_limit_6g_20m_c3",   NVRAM_PARA_CE_6G_20M_C3, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_6g_40m_c3",   NVRAM_PARA_CE_6G_40M_C3, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_6g_80m_c3",   NVRAM_PARA_CE_6G_80M_C3, NULL, 0, 0, OAL_FALSE},
    {"ce_sub_side_band_txpwr_limit_6g_20m_c3",   NVRAM_PARA_CE_SUB_SIDE_6G_20M_C3, NULL, 0, 0, OAL_FALSE},
    {"ce_sub_side_band_txpwr_limit_6g_40m_c3",   NVRAM_PARA_CE_SUB_SIDE_6G_40M_C3, NULL, 0, 0, OAL_FALSE},
    {"ce_sub_side_band_txpwr_limit_6g_80m_c3",   NVRAM_PARA_CE_SUB_SIDE_6G_80M_C3, NULL, 0, 0, OAL_FALSE},
    {"ce_side_band_txpwr_limit_6g_160m_c3",  NVRAM_PARA_CE_6G_160M_C3, NULL, 0, 0, OAL_FALSE},
#endif
    {NULL, NVRAM_PARA_CE_C3_END_INDEX_BUTT, NULL, 0, 0, OAL_FALSE},

    /* SAR C0 */
    {"sar_txpwr_ctrl_lvl1_c0",  NVRAM_PARA_SAR_LVL1_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl2_c0",  NVRAM_PARA_SAR_LVL2_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl3_c0",  NVRAM_PARA_SAR_LVL3_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl4_c0",  NVRAM_PARA_SAR_LVL4_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl5_c0",  NVRAM_PARA_SAR_LVL5_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl6_c0",  NVRAM_PARA_SAR_LVL6_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl7_c0",  NVRAM_PARA_SAR_LVL7_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl8_c0",  NVRAM_PARA_SAR_LVL8_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl9_c0",  NVRAM_PARA_SAR_LVL9_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl10_c0", NVRAM_PARA_SAR_LVL10_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl11_c0", NVRAM_PARA_SAR_LVL11_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl12_c0", NVRAM_PARA_SAR_LVL12_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl13_c0", NVRAM_PARA_SAR_LVL13_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl14_c0", NVRAM_PARA_SAR_LVL14_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl15_c0", NVRAM_PARA_SAR_LVL15_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl16_c0", NVRAM_PARA_SAR_LVL16_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl17_c0", NVRAM_PARA_SAR_LVL17_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl18_c0", NVRAM_PARA_SAR_LVL18_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl19_c0", NVRAM_PARA_SAR_LVL19_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl20_c0", NVRAM_PARA_SAR_LVL20_C0, NULL, 0, 0, OAL_FALSE},
    {NULL, NVRAM_PARA_C0_SAR_END_INDEX_BUTT, NULL, 0, 0, OAL_FALSE},

    /* SAR C1 */
    {"sar_txpwr_ctrl_lvl1_c1",  NVRAM_PARA_SAR_LVL1_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl2_c1",  NVRAM_PARA_SAR_LVL2_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl3_c1",  NVRAM_PARA_SAR_LVL3_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl4_c1",  NVRAM_PARA_SAR_LVL4_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl5_c1",  NVRAM_PARA_SAR_LVL5_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl6_c1",  NVRAM_PARA_SAR_LVL6_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl7_c1",  NVRAM_PARA_SAR_LVL7_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl8_c1",  NVRAM_PARA_SAR_LVL8_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl9_c1",  NVRAM_PARA_SAR_LVL9_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl10_c1", NVRAM_PARA_SAR_LVL10_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl11_c1", NVRAM_PARA_SAR_LVL11_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl12_c1", NVRAM_PARA_SAR_LVL12_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl13_c1", NVRAM_PARA_SAR_LVL13_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl14_c1", NVRAM_PARA_SAR_LVL14_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl15_c1", NVRAM_PARA_SAR_LVL15_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl16_c1", NVRAM_PARA_SAR_LVL16_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl17_c1", NVRAM_PARA_SAR_LVL17_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl18_c1", NVRAM_PARA_SAR_LVL18_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl19_c1", NVRAM_PARA_SAR_LVL19_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl20_c1", NVRAM_PARA_SAR_LVL20_C1, NULL, 0, 0, OAL_FALSE},
    {NULL, NVRAM_PARA_C1_SAR_END_INDEX_BUTT, NULL, 0, 0, OAL_FALSE},

    /* SAR C2 */
    {"sar_txpwr_ctrl_lvl1_c2",  NVRAM_PARA_SAR_LVL1_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl2_c2",  NVRAM_PARA_SAR_LVL2_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl3_c2",  NVRAM_PARA_SAR_LVL3_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl4_c2",  NVRAM_PARA_SAR_LVL4_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl5_c2",  NVRAM_PARA_SAR_LVL5_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl6_c2",  NVRAM_PARA_SAR_LVL6_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl7_c2",  NVRAM_PARA_SAR_LVL7_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl8_c2",  NVRAM_PARA_SAR_LVL8_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl9_c2",  NVRAM_PARA_SAR_LVL9_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl10_c2", NVRAM_PARA_SAR_LVL10_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl11_c2", NVRAM_PARA_SAR_LVL11_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl12_c2", NVRAM_PARA_SAR_LVL12_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl13_c2", NVRAM_PARA_SAR_LVL13_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl14_c2", NVRAM_PARA_SAR_LVL14_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl15_c2", NVRAM_PARA_SAR_LVL15_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl16_c2", NVRAM_PARA_SAR_LVL16_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl17_c2", NVRAM_PARA_SAR_LVL17_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl18_c2", NVRAM_PARA_SAR_LVL18_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl19_c2", NVRAM_PARA_SAR_LVL19_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl20_c2", NVRAM_PARA_SAR_LVL20_C2, NULL, 0, 0, OAL_FALSE},
    {NULL, NVRAM_PARA_C2_SAR_END_INDEX_BUTT, NULL, 0, 0, OAL_FALSE},

    /* SAR C3 */
    {"sar_txpwr_ctrl_lvl1_c3",  NVRAM_PARA_SAR_LVL1_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl2_c3",  NVRAM_PARA_SAR_LVL2_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl3_c3",  NVRAM_PARA_SAR_LVL3_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl4_c3",  NVRAM_PARA_SAR_LVL4_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl5_c3",  NVRAM_PARA_SAR_LVL5_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl6_c3",  NVRAM_PARA_SAR_LVL6_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl7_c3",  NVRAM_PARA_SAR_LVL7_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl8_c3",  NVRAM_PARA_SAR_LVL8_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl9_c3",  NVRAM_PARA_SAR_LVL9_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl10_c3", NVRAM_PARA_SAR_LVL10_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl11_c3", NVRAM_PARA_SAR_LVL11_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl12_c3", NVRAM_PARA_SAR_LVL12_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl13_c3", NVRAM_PARA_SAR_LVL13_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl14_c3", NVRAM_PARA_SAR_LVL14_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl15_c3", NVRAM_PARA_SAR_LVL15_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl16_c3", NVRAM_PARA_SAR_LVL16_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl17_c3", NVRAM_PARA_SAR_LVL17_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl18_c3", NVRAM_PARA_SAR_LVL18_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl19_c3", NVRAM_PARA_SAR_LVL19_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl20_c3", NVRAM_PARA_SAR_LVL20_C3, NULL, 0, 0, OAL_FALSE},
    {NULL, NVRAM_PARA_C3_SAR_END_INDEX_BUTT, NULL, 0, 0, OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        /* 6G SAR C0 */
    {"sar_txpwr_ctrl_lvl1_c0",  NVRAM_PARA_SAR_LVL1_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl2_c0",  NVRAM_PARA_SAR_LVL2_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl3_c0",  NVRAM_PARA_SAR_LVL3_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl4_c0",  NVRAM_PARA_SAR_LVL4_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl5_c0",  NVRAM_PARA_SAR_LVL5_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl6_6g_c0",  NVRAM_PARA_SAR_LVL6_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl7_6g_c0",  NVRAM_PARA_SAR_LVL7_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl8_6g_c0",  NVRAM_PARA_SAR_LVL8_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl9_6g_c0",  NVRAM_PARA_SAR_LVL9_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl10_6g_c0", NVRAM_PARA_SAR_LVL10_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl11_6g_c0", NVRAM_PARA_SAR_LVL11_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl12_6g_c0", NVRAM_PARA_SAR_LVL12_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl13_6g_c0", NVRAM_PARA_SAR_LVL13_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl14_6g_c0", NVRAM_PARA_SAR_LVL14_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl15_6g_c0", NVRAM_PARA_SAR_LVL15_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl16_6g_c0", NVRAM_PARA_SAR_LVL16_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl17_6g_c0", NVRAM_PARA_SAR_LVL17_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl18_6g_c0", NVRAM_PARA_SAR_LVL18_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl19_6g_c0", NVRAM_PARA_SAR_LVL19_6G_C0, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl20_6g_c0", NVRAM_PARA_SAR_LVL20_6G_C0, NULL, 0, 0, OAL_FALSE},
    {NULL, NVRAM_PARA_6G_C0_SAR_END_INDEX_BUTT, NULL, 0, 0, OAL_FALSE},

    /* 6G SAR C1 */
    {"sar_txpwr_ctrl_lvl1_6g_c1",  NVRAM_PARA_SAR_LVL1_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl2_6g_c1",  NVRAM_PARA_SAR_LVL2_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl3_6g_c1",  NVRAM_PARA_SAR_LVL3_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl4_6g_c1",  NVRAM_PARA_SAR_LVL4_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl5_6g_c1",  NVRAM_PARA_SAR_LVL5_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl6_6g_c1",  NVRAM_PARA_SAR_LVL6_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl7_6g_c1",  NVRAM_PARA_SAR_LVL7_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl8_6g_c1",  NVRAM_PARA_SAR_LVL8_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl9_6g_c1",  NVRAM_PARA_SAR_LVL9_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl10_6g_c1", NVRAM_PARA_SAR_LVL10_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl11_6g_c1", NVRAM_PARA_SAR_LVL11_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl12_6g_c1", NVRAM_PARA_SAR_LVL12_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl13_6g_c1", NVRAM_PARA_SAR_LVL13_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl14_6g_c1", NVRAM_PARA_SAR_LVL14_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl15_6g_c1", NVRAM_PARA_SAR_LVL15_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl16_6g_c1", NVRAM_PARA_SAR_LVL16_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl17_6g_c1", NVRAM_PARA_SAR_LVL17_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl18_6g_c1", NVRAM_PARA_SAR_LVL18_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl19_6g_c1", NVRAM_PARA_SAR_LVL19_6G_C1, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl20_6g_c1", NVRAM_PARA_SAR_LVL20_6G_C1, NULL, 0, 0, OAL_FALSE},
    {NULL, NVRAM_PARA_6G_C1_SAR_END_INDEX_BUTT, NULL, 0, 0, OAL_FALSE},

    /* 6G SAR C2 */
    {"sar_txpwr_ctrl_lvl1_6g_c2",  NVRAM_PARA_SAR_LVL1_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl2_6g_c2",  NVRAM_PARA_SAR_LVL2_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl3_6g_c2",  NVRAM_PARA_SAR_LVL3_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl4_6g_c2",  NVRAM_PARA_SAR_LVL4_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl5_6g_c2",  NVRAM_PARA_SAR_LVL5_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl6_6g_c2",  NVRAM_PARA_SAR_LVL6_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl7_6g_c2",  NVRAM_PARA_SAR_LVL7_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl8_6g_c2",  NVRAM_PARA_SAR_LVL8_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl9_6g_c2",  NVRAM_PARA_SAR_LVL9_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl10_6g_c2", NVRAM_PARA_SAR_LVL10_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl11_6g_c2", NVRAM_PARA_SAR_LVL11_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl12_6g_c2", NVRAM_PARA_SAR_LVL12_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl13_6g_c2", NVRAM_PARA_SAR_LVL13_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl14_6g_c2", NVRAM_PARA_SAR_LVL14_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl15_6g_c2", NVRAM_PARA_SAR_LVL15_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl16_6g_c2", NVRAM_PARA_SAR_LVL16_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl17_6g_c2", NVRAM_PARA_SAR_LVL17_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl18_6g_c2", NVRAM_PARA_SAR_LVL18_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl19_6g_c2", NVRAM_PARA_SAR_LVL19_6G_C2, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl20_6g_c2", NVRAM_PARA_SAR_LVL20_6G_C2, NULL, 0, 0, OAL_FALSE},
    {NULL, NVRAM_PARA_6G_C2_SAR_END_INDEX_BUTT, NULL, 0, 0, OAL_FALSE},

    /* 6G SAR C3 */
    {"sar_txpwr_ctrl_lvl1_6g_c3",  NVRAM_PARA_SAR_LVL1_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl2_6g_c3",  NVRAM_PARA_SAR_LVL2_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl3_6g_c3",  NVRAM_PARA_SAR_LVL3_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl4_6g_c3",  NVRAM_PARA_SAR_LVL4_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl5_6g_c3",  NVRAM_PARA_SAR_LVL5_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl6_6g_c3",  NVRAM_PARA_SAR_LVL6_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl7_6g_c3",  NVRAM_PARA_SAR_LVL7_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl8_6g_c3",  NVRAM_PARA_SAR_LVL8_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl9_6g_c3",  NVRAM_PARA_SAR_LVL9_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl10_6g_c3", NVRAM_PARA_SAR_LVL10_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl11_6g_c3", NVRAM_PARA_SAR_LVL11_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl12_6g_c3", NVRAM_PARA_SAR_LVL12_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl13_6g_c3", NVRAM_PARA_SAR_LVL13_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl14_6g_c3", NVRAM_PARA_SAR_LVL14_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl15_6g_c3", NVRAM_PARA_SAR_LVL15_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl16_6g_c3", NVRAM_PARA_SAR_LVL16_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl17_6g_c3", NVRAM_PARA_SAR_LVL17_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl18_6g_c3", NVRAM_PARA_SAR_LVL18_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl19_6g_c3", NVRAM_PARA_SAR_LVL19_6G_C3, NULL, 0, 0, OAL_FALSE},
    {"sar_txpwr_ctrl_lvl20_6g_c3", NVRAM_PARA_SAR_LVL20_6G_C3, NULL, 0, 0, OAL_FALSE},
    {NULL, NVRAM_PARA_6G_C3_SAR_END_INDEX_BUTT, NULL, 0, 0, OAL_FALSE},
#endif
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    {"tas_ant_switch_en", NVRAM_PARA_TAS_ANT_SWITCH_EN, &g_tas_switch_en[WLAN_RF_CHANNEL_ZERO],
     sizeof(g_tas_switch_en[WLAN_RF_CHANNEL_ZERO]), ARRAY_SIZE(g_tas_switch_en), OAL_FALSE},
    {"tas_txpwr_ctrl_2g", NVRAM_PARA_TAS_PWR_CTRL_2G,
     &g_cust_pow.tas_ctrl_params[WLAN_CALI_BAND_2G][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_pow.tas_ctrl_params[WLAN_CALI_BAND_2G][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_pow.tas_ctrl_params[WLAN_CALI_BAND_2G]), OAL_FALSE},
    {"tas_txpwr_ctrl_5g", NVRAM_PARA_TAS_PWR_CTRL_5G,
     &g_cust_pow.tas_ctrl_params[WLAN_CALI_BAND_5G][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_pow.tas_ctrl_params[WLAN_CALI_BAND_5G][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_pow.tas_ctrl_params[WLAN_CALI_BAND_5G]), OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"tas_txpwr_ctrl_6g", NVRAM_PARA_TAS_PWR_CTRL_6G,
     &g_cust_pow.tas_ctrl_params[WLAN_CALI_BAND_6G][WLAN_RF_CHANNEL_ZERO],
     sizeof(g_cust_pow.tas_ctrl_params[WLAN_CALI_BAND_6G][WLAN_RF_CHANNEL_ZERO]),
     ARRAY_SIZE(g_cust_pow.tas_ctrl_params[WLAN_CALI_BAND_6G]), OAL_FALSE},
#endif
#endif
    {"5g_max_pow_high_band_fcc_ce", NVRAM_PARA_5G_FCC_CE_HIGH_BAND_MAX_PWR, &g_cust_pow.fcc_ce_max_pwr_for_5g_high_band,
     sizeof(g_cust_pow.fcc_ce_max_pwr_for_5g_high_band), WLAN_CUST_NUM_1, OAL_FALSE},
    {"cfg_tpc_ru_pow_2g_20m",  NVRAM_PARA_TPC_RU_POWER_2G_20M,
     &g_cust_pow.full_bw_ru_pow_2g[WLAN_BW_CAP_20M].ru_pwr_2g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.full_bw_ru_pow_2g[WLAN_BW_CAP_20M].ru_pwr_2g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.full_bw_ru_pow_2g[WLAN_BW_CAP_20M].ru_pwr_2g), OAL_FALSE},
    {"cfg_tpc_ru_pow_2g_40m",  NVRAM_PARA_TPC_RU_POWER_2G_40M,
     &g_cust_pow.full_bw_ru_pow_2g[WLAN_BW_CAP_40M].ru_pwr_2g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.full_bw_ru_pow_2g[WLAN_BW_CAP_40M].ru_pwr_2g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.full_bw_ru_pow_2g[WLAN_BW_CAP_40M].ru_pwr_2g), OAL_FALSE},
    {"cfg_tpc_ru_pow_5g_20m",  NVRAM_PARA_TPC_RU_POWER_5G_20M,
     &g_cust_pow.full_bw_ru_pow_5g[WLAN_BW_CAP_20M].ru_pwr_5g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.full_bw_ru_pow_5g[WLAN_BW_CAP_20M].ru_pwr_5g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.full_bw_ru_pow_5g[WLAN_BW_CAP_20M].ru_pwr_5g), OAL_FALSE},
    {"cfg_tpc_ru_pow_5g_40m",  NVRAM_PARA_TPC_RU_POWER_5G_40M,
     &g_cust_pow.full_bw_ru_pow_5g[WLAN_BW_CAP_40M].ru_pwr_5g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.full_bw_ru_pow_5g[WLAN_BW_CAP_40M].ru_pwr_5g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.full_bw_ru_pow_5g[WLAN_BW_CAP_40M].ru_pwr_5g), OAL_FALSE},
    {"cfg_tpc_ru_pow_5g_80m",  NVRAM_PARA_TPC_RU_POWER_5G_80M,
     &g_cust_pow.full_bw_ru_pow_5g[WLAN_BW_CAP_80M].ru_pwr_5g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.full_bw_ru_pow_5g[WLAN_BW_CAP_80M].ru_pwr_5g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.full_bw_ru_pow_5g[WLAN_BW_CAP_80M].ru_pwr_5g), OAL_FALSE},
    {"cfg_tpc_ru_pow_5g_160m", NVRAM_PARA_TPC_RU_POWER_5G_160M,
     &g_cust_pow.full_bw_ru_pow_5g[WLAN_BW_CAP_160M].ru_pwr_5g,
     sizeof(g_cust_pow.full_bw_ru_pow_5g[WLAN_BW_CAP_160M].ru_pwr_5g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.full_bw_ru_pow_5g[WLAN_BW_CAP_160M].ru_pwr_5g), OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"cfg_tpc_ru_pow_6g_20m",  NVRAM_PARA_TPC_RU_POWER_6G_20M,
     &g_cust_pow.full_bw_ru_pow_6g[WLAN_BW_CAP_20M].ru_pwr_6g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.full_bw_ru_pow_6g[WLAN_BW_CAP_20M].ru_pwr_6g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.full_bw_ru_pow_6g[WLAN_BW_CAP_20M].ru_pwr_6g), OAL_FALSE},
    {"cfg_tpc_ru_pow_6g_40m",  NVRAM_PARA_TPC_RU_POWER_6G_40M,
     &g_cust_pow.full_bw_ru_pow_6g[WLAN_BW_CAP_40M].ru_pwr_6g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.full_bw_ru_pow_6g[WLAN_BW_CAP_40M].ru_pwr_6g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.full_bw_ru_pow_6g[WLAN_BW_CAP_40M].ru_pwr_6g), OAL_FALSE},
    {"cfg_tpc_ru_pow_6g_80m",  NVRAM_PARA_TPC_RU_POWER_6G_80M,
     &g_cust_pow.full_bw_ru_pow_6g[WLAN_BW_CAP_80M].ru_pwr_6g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.full_bw_ru_pow_6g[WLAN_BW_CAP_80M].ru_pwr_6g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.full_bw_ru_pow_6g[WLAN_BW_CAP_80M].ru_pwr_6g), OAL_FALSE},
    {"cfg_tpc_ru_pow_6g_160m", NVRAM_PARA_TPC_RU_POWER_6G_160M,
     &g_cust_pow.full_bw_ru_pow_6g[WLAN_BW_CAP_160M].ru_pwr_6g,
     sizeof(g_cust_pow.full_bw_ru_pow_6g[WLAN_BW_CAP_160M].ru_pwr_6g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.full_bw_ru_pow_6g[WLAN_BW_CAP_160M].ru_pwr_6g), OAL_FALSE},
#endif
    {"cfg_tpc_ru_max_pow_2g_c0",    NVRAM_PARA_TPC_RU_MAX_POWER_2G_C0,
     &g_cust_pow.tpc_tb_ru_2g_max_pow[HAL_POW_RF_SEL_CHAIN_1].ru_pwr_2g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.tpc_tb_ru_2g_max_pow[HAL_POW_RF_SEL_CHAIN_1].ru_pwr_2g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.tpc_tb_ru_2g_max_pow[HAL_POW_RF_SEL_CHAIN_1].ru_pwr_2g), OAL_FALSE},
    {"cfg_tpc_ru_max_pow_2g_c1", NVRAM_PARA_TPC_RU_MAX_POWER_2G_C1,
     &g_cust_pow.tpc_tb_ru_2g_max_pow[HAL_POW_RF_SEL_CHAIN_2].ru_pwr_2g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.tpc_tb_ru_2g_max_pow[HAL_POW_RF_SEL_CHAIN_2].ru_pwr_2g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.tpc_tb_ru_2g_max_pow[HAL_POW_RF_SEL_CHAIN_2].ru_pwr_2g), OAL_FALSE},
    {"cfg_tpc_ru_max_pow_2g_c2", NVRAM_PARA_TPC_RU_MAX_POWER_2G_C2,
     &g_cust_pow.tpc_tb_ru_2g_max_pow[HAL_POW_RF_SEL_CHAIN_3].ru_pwr_2g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.tpc_tb_ru_2g_max_pow[HAL_POW_RF_SEL_CHAIN_3].ru_pwr_2g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.tpc_tb_ru_2g_max_pow[HAL_POW_RF_SEL_CHAIN_3].ru_pwr_2g), OAL_FALSE},
    {"cfg_tpc_ru_max_pow_2g_c3", NVRAM_PARA_TPC_RU_MAX_POWER_2G_C3,
     &g_cust_pow.tpc_tb_ru_2g_max_pow[HAL_POW_RF_SEL_CHAIN_4].ru_pwr_2g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.tpc_tb_ru_2g_max_pow[HAL_POW_RF_SEL_CHAIN_4].ru_pwr_2g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.tpc_tb_ru_2g_max_pow[HAL_POW_RF_SEL_CHAIN_4].ru_pwr_2g), OAL_FALSE},
    {"cfg_tpc_ru_max_pow_5g_c0",    NVRAM_PARA_TPC_RU_MAX_POWER_5G_C0,
     &g_cust_pow.tpc_tb_ru_5g_max_pow[HAL_POW_RF_SEL_CHAIN_1].ru_pwr_5g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.tpc_tb_ru_5g_max_pow[HAL_POW_RF_SEL_CHAIN_1].ru_pwr_5g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.tpc_tb_ru_5g_max_pow[HAL_POW_RF_SEL_CHAIN_1].ru_pwr_5g), OAL_FALSE},
    {"cfg_tpc_ru_max_pow_5g_c1", NVRAM_PARA_TPC_RU_MAX_POWER_5G_C1,
     &g_cust_pow.tpc_tb_ru_5g_max_pow[HAL_POW_RF_SEL_CHAIN_2].ru_pwr_5g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.tpc_tb_ru_5g_max_pow[HAL_POW_RF_SEL_CHAIN_2].ru_pwr_5g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.tpc_tb_ru_5g_max_pow[HAL_POW_RF_SEL_CHAIN_2].ru_pwr_5g), OAL_FALSE},
    {"cfg_tpc_ru_max_pow_5g_c2", NVRAM_PARA_TPC_RU_MAX_POWER_5G_C2,
     &g_cust_pow.tpc_tb_ru_5g_max_pow[HAL_POW_RF_SEL_CHAIN_3].ru_pwr_5g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.tpc_tb_ru_5g_max_pow[HAL_POW_RF_SEL_CHAIN_3].ru_pwr_5g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.tpc_tb_ru_5g_max_pow[HAL_POW_RF_SEL_CHAIN_3].ru_pwr_5g), OAL_FALSE},
    {"cfg_tpc_ru_max_pow_5g_c3", NVRAM_PARA_TPC_RU_MAX_POWER_5G_C3,
     &g_cust_pow.tpc_tb_ru_5g_max_pow[HAL_POW_RF_SEL_CHAIN_4].ru_pwr_5g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.tpc_tb_ru_5g_max_pow[HAL_POW_RF_SEL_CHAIN_4].ru_pwr_5g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.tpc_tb_ru_5g_max_pow[HAL_POW_RF_SEL_CHAIN_4].ru_pwr_5g), OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"cfg_tpc_ru_max_pow_6g_c0",    NVRAM_PARA_TPC_RU_MAX_POWER_6G_C0,
     &g_cust_pow.tpc_tb_ru_6g_max_pow[HAL_POW_RF_SEL_CHAIN_1].ru_pwr_6g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.tpc_tb_ru_6g_max_pow[HAL_POW_RF_SEL_CHAIN_1].ru_pwr_6g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.tpc_tb_ru_6g_max_pow[HAL_POW_RF_SEL_CHAIN_1].ru_pwr_6g), OAL_FALSE},
    {"cfg_tpc_ru_max_pow_6g_c1", NVRAM_PARA_TPC_RU_MAX_POWER_6G_C1,
     &g_cust_pow.tpc_tb_ru_6g_max_pow[HAL_POW_RF_SEL_CHAIN_2].ru_pwr_6g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.tpc_tb_ru_6g_max_pow[HAL_POW_RF_SEL_CHAIN_2].ru_pwr_6g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.tpc_tb_ru_6g_max_pow[HAL_POW_RF_SEL_CHAIN_2].ru_pwr_6g), OAL_FALSE},
    {"cfg_tpc_ru_max_pow_6g_c2", NVRAM_PARA_TPC_RU_MAX_POWER_6G_C2,
     &g_cust_pow.tpc_tb_ru_6g_max_pow[HAL_POW_RF_SEL_CHAIN_3].ru_pwr_6g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.tpc_tb_ru_6g_max_pow[HAL_POW_RF_SEL_CHAIN_3].ru_pwr_6g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.tpc_tb_ru_6g_max_pow[HAL_POW_RF_SEL_CHAIN_3].ru_pwr_6g), OAL_FALSE},
    {"cfg_tpc_ru_max_pow_6g_c3", NVRAM_PARA_TPC_RU_MAX_POWER_6G_C3,
     &g_cust_pow.tpc_tb_ru_6g_max_pow[HAL_POW_RF_SEL_CHAIN_4].ru_pwr_6g[WLAN_HE_RU_SIZE_26],
     sizeof(g_cust_pow.tpc_tb_ru_6g_max_pow[HAL_POW_RF_SEL_CHAIN_4].ru_pwr_6g[WLAN_HE_RU_SIZE_26]),
     ARRAY_SIZE(g_cust_pow.tpc_tb_ru_6g_max_pow[HAL_POW_RF_SEL_CHAIN_4].ru_pwr_6g), OAL_FALSE},
#endif
    {"cfg_tpc_ru_max_pow_2g", NVRAM_PARA_TPC_RU_MAX_POWER_2G, &g_cust_pow.tpc_tb_ru_max_power[WLAN_CALI_BAND_2G],
     sizeof(g_cust_pow.tpc_tb_ru_max_power[WLAN_CALI_BAND_2G]), WLAN_CUST_NUM_1, OAL_FALSE},
    {"cfg_tpc_ru_max_pow_5g", NVRAM_PARA_TPC_RU_MAX_POWER_5G, &g_cust_pow.tpc_tb_ru_max_power[WLAN_CALI_BAND_5G],
     sizeof(g_cust_pow.tpc_tb_ru_max_power[WLAN_CALI_BAND_5G]), WLAN_CUST_NUM_1, OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"cfg_tpc_ru_max_pow_6g", NVRAM_PARA_TPC_RU_MAX_POWER_6G, &g_cust_pow.tpc_tb_ru_max_power[WLAN_CALI_BAND_6G],
     sizeof(g_cust_pow.tpc_tb_ru_max_power[WLAN_CALI_BAND_6G]), WLAN_CUST_NUM_1, OAL_FALSE},
#endif
};

static wlan_cust_cfg_cmd g_wifi_cfg_cap[] = {
    /* 校准开关 */
    {"cali_policy", WLAN_CFG_CAP_CALI_POLICY, &g_cust_cap.cali_policy, sizeof(g_cust_cap.cali_policy),
     WLAN_CUST_NUM_1, OAL_FALSE},
    {"cali_mask_2g", WLAN_CFG_CAP_CALI_MASK_2G, &g_cust_cap.cali_switch.cali_mask[WLAN_CALI_BAND_2G],
     sizeof(g_cust_cap.cali_switch.cali_mask[WLAN_CALI_BAND_2G]), WLAN_CUST_NUM_1, OAL_FALSE},
    {"cali_mask_5g", WLAN_CFG_CAP_CALI_MASK_5G, &g_cust_cap.cali_switch.cali_mask[WLAN_CALI_BAND_5G],
     sizeof(g_cust_cap.cali_switch.cali_mask[WLAN_CALI_BAND_5G]), WLAN_CUST_NUM_1, OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {"cali_mask_6g", WLAN_CFG_CAP_CALI_MASK_6G, &g_cust_cap.cali_switch.cali_mask[WLAN_CALI_BAND_6G],
     sizeof(g_cust_cap.cali_switch.cali_mask[WLAN_CALI_BAND_6G]), WLAN_CUST_NUM_1, OAL_FALSE},
#endif
    {"online_cali_mask", WLAN_CFG_CAP_CALI_ONLINE_MASK, &g_cust_cap.cali_switch.online_cali_mask,
     sizeof(g_cust_cap.cali_switch.online_cali_mask), WLAN_CUST_NUM_1, OAL_FALSE},
    /*
     * #bit0:开wifi重新校准 bit1:开wifi重新上传 bit2:开机校准 bit3:动态校准调平Debug
     * #bit4:不读取NV区域的数据(1:不读取 0：读取)
     */
    {"cali_data_mask", WLAN_CFG_CAP_CALI_DATA_MASK,
     &g_cust_cap.cali_data_mask, sizeof(g_cust_cap.cali_data_mask), WLAN_CUST_NUM_1, OAL_FALSE},
    {"bw_max_width", WLAN_CFG_CAP_BW_MAX_WITH,
     &g_cust_cap.en_channel_width, sizeof(g_cust_cap.en_channel_width), WLAN_CUST_NUM_1, OAL_FALSE},
    {"ldpc_coding",  WLAN_CFG_CAP_LDPC_CODING,
     &g_cust_cap.en_ldpc_is_supp, sizeof(g_cust_cap.en_ldpc_is_supp), WLAN_CUST_NUM_1, OAL_FALSE},
    {"rx_stbc",      WLAN_CFG_CAP_RX_STBC,
      &g_cust_cap.en_rx_stbc_is_supp, sizeof(g_cust_cap.en_rx_stbc_is_supp), WLAN_CUST_NUM_1, OAL_FALSE},
    {"tx_stbc",      WLAN_CFG_CAP_TX_STBC,
     &g_cust_cap.en_tx_stbc_is_supp, sizeof(g_cust_cap.en_tx_stbc_is_supp), WLAN_CUST_NUM_1, OAL_FALSE},
    {"su_bfer",      WLAN_CFG_CAP_SU_BFER,
     &g_cust_cap.en_su_bfmer_is_supp, sizeof(g_cust_cap.en_su_bfmer_is_supp), WLAN_CUST_NUM_1, OAL_FALSE},
    {"su_bfee",      WLAN_CFG_CAP_SU_BFEE,
     &g_cust_cap.en_su_bfmee_is_supp, sizeof(g_cust_cap.en_su_bfmee_is_supp), WLAN_CUST_NUM_1, OAL_FALSE},
    {"mu_bfer",      WLAN_CFG_CAP_MU_BFER,
     &g_cust_cap.en_mu_bfmer_is_supp, sizeof(g_cust_cap.en_mu_bfmer_is_supp), WLAN_CUST_NUM_1, OAL_FALSE},
    {"mu_bfee",      WLAN_CFG_CAP_MU_BFEE,
     &g_cust_cap.en_mu_bfmee_is_supp, sizeof(g_cust_cap.en_mu_bfmee_is_supp), WLAN_CUST_NUM_1, OAL_FALSE},
    {"1024qam_en", WLAN_CFG_CAP_1024_QAM,
     &g_cust_cap.en_1024qam_is_supp, sizeof(g_cust_cap.en_1024qam_is_supp), WLAN_CUST_NUM_1, OAL_FALSE},
    {"hisi_priv_cap", WLAN_CFG_CAP_HISI_PRIV_IE,
     &g_cust_cap.hisi_priv_cap, sizeof(g_cust_cap.hisi_priv_cap), WLAN_CUST_NUM_1, OAL_FALSE},
    {"voe_switch_mask",           WLAN_CFG_CAP_VOE_SWITCH, &g_cust_cap.voe_switch_mask,
     sizeof(g_cust_cap.voe_switch_mask),   WLAN_CUST_NUM_1, OAL_FALSE},
    {"11ax_switch_mask",          WLAN_CFG_CAP_11AX_SWITCH, &g_cust_cap.wifi_11ax_switch_mask,
     sizeof(g_cust_cap.wifi_11ax_switch_mask),   WLAN_CUST_NUM_1, OAL_FALSE},
    {"htc_switch_mask",           WLAN_CFG_CAP_HTC_SWITCH, &g_cust_cap.htc_switch_mask,
     sizeof(g_cust_cap.htc_switch_mask),   WLAN_CUST_NUM_1, OAL_FALSE},
    {"multi_bssid_switch_mask",   WLAN_CFG_CAP_MULTI_BSSID_SWITCH, &g_cust_cap.mult_bssid_switch_mask,
     sizeof(g_cust_cap.mult_bssid_switch_mask),   WLAN_CUST_NUM_1, OAL_FALSE},
    {"dynamic_dbac_adjust_mask", WLAN_CFG_CAP_DYNAMIC_DBAC_SWITCH, &g_cust_cap.dbac_dynamic_switch,
     sizeof(g_cust_cap.dbac_dynamic_switch),   WLAN_CUST_NUM_1, OAL_FALSE},

    /* DBDC */
    {"radio_cap",     WLAN_CFG_CAP_DBDC_RADIO,
     &g_cust_cap.radio_cap[0], sizeof(g_cust_cap.radio_cap[0]), ARRAY_SIZE(g_cust_cap.radio_cap), OAL_FALSE},
    {"dbdc_cap",     WLAN_CFG_CAP_DBDC,
     &g_cust_cap.dbdc_cap, sizeof(g_cust_cap.dbdc_cap), WLAN_CUST_NUM_1, OAL_FALSE},
    {"fastscan_switch", WLAN_CFG_CAP_FASTSCAN_SWITCH,
     &g_cust_cap.fastscan_switch, sizeof(g_cust_cap.fastscan_switch), WLAN_CUST_NUM_1, OAL_FALSE},
    /* RSSI天线切换 */
    {"rssi_ant_switch", WLAN_CFG_CAP_ANT_SWITCH,
     &g_cust_cap.rssi_switch_ant, sizeof(g_cust_cap.rssi_switch_ant), WLAN_CUST_NUM_1, OAL_FALSE},
     /* 国家码自学习功能开关 */
    {"countrycode_selfstudy", WLAN_CFG_CAP_COUNRTYCODE_SELFSTUDY_CFG,
     &g_cust_cap.country_self_study, sizeof(g_cust_cap.country_self_study), WLAN_CUST_NUM_1, OAL_FALSE},
     /* 多通道退化能力 */
    {"degrade_cap", WLAN_CFG_CAP_DEGRADATION_CAP,
     &g_cust_cap.degrade_cap, sizeof(g_cust_cap.degrade_cap), WLAN_CUST_NUM_1, OAL_FALSE},

    /* MCM */
    {"chann_radio_cap", WLAN_CFG_CAP_RADIO_CAP,
    &g_cust_cap.chn_radio_cap, sizeof(g_cust_cap.chn_radio_cap), WLAN_CUST_NUM_1, OAL_FALSE},
    {"chann_radio_cap_6g", WLAN_CFG_CAP_RADIO_CAP_6G, &g_cust_cap.chn_radio_cap_6g,
     sizeof(g_cust_cap.chn_radio_cap_6g), WLAN_CUST_NUM_1, OAL_FALSE},
    {"mcm_custom_function_mask", WLAN_CFG_CAP_MCM_CUSTOM_FUNCTION_MASK,
     &g_cust_cap.mcm_custom_func_mask, sizeof(g_cust_cap.mcm_custom_func_mask), WLAN_CUST_NUM_1, OAL_FALSE},
    {"mcm_function_mask", WLAN_CFG_CAP_MCM_FUNCTION_MASK,
     &g_cust_cap.mcm_func_mask, sizeof(g_cust_cap.mcm_func_mask), WLAN_CUST_NUM_1, OAL_FALSE},
    {"linkloss_threshold_fixed", WLAN_CFG_CAP_LINKLOSS_THRESHOLD_FIXED,
     &g_cust_cap.linkloss_threshold_fixed, sizeof(g_cust_cap.linkloss_threshold_fixed), WLAN_CUST_NUM_1, OAL_FALSE},
    {"aput_support_160m", WLAN_CFG_CAP_APUT_160M_ENABLE,
     &g_cust_cap.aput_160m_switch, sizeof(g_cust_cap.aput_160m_switch), WLAN_CUST_NUM_1, OAL_FALSE},
    {"radar_isr_forbid", WLAN_CFG_CAP_RADAR_ISR_FORBID,
     &g_cust_cap.radar_isr_forbid, sizeof(g_cust_cap.radar_isr_forbid), WLAN_CUST_NUM_1, OAL_FALSE},
    {"download_rate_limit_pps", WLAN_CFG_CAP_DOWNLOAD_RATE_LIMIT_PPS,
     &g_cust_cap.download_rate_limit_pps, sizeof(g_cust_cap.download_rate_limit_pps), WLAN_CUST_NUM_1, OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    {"txopps_switch", WLAN_CFG_CAP_TXOPPS_SWITCH,
     &g_cust_cap.txopps_switch, sizeof(g_cust_cap.txopps_switch), WLAN_CUST_NUM_1, OAL_FALSE},
#endif
    {"over_temper_protect_threshold",   WLAN_CFG_CAP_OVER_TEMPER_PROTECT_THRESHOLD,
     &g_cust_cap.over_temper_protect.over_temper_protect_th,
     sizeof(g_cust_cap.over_temper_protect.over_temper_protect_th), WLAN_CUST_NUM_1, OAL_FALSE},
    {"over_temp_pro_enable", WLAN_CFG_CAP_OVER_TEMP_PRO_ENABLE,
     &g_cust_cap.over_temper_protect.en_over_temp_pro,
     sizeof(g_cust_cap.over_temper_protect.en_over_temp_pro), WLAN_CUST_NUM_1, OAL_FALSE},
    {"over_temp_pro_reduce_pwr_enable", WLAN_CFG_CAP_OVER_TEMP_PRO_REDUCE_PWR_ENABLE,
     &g_cust_cap.over_temper_protect.en_reduce_pwr,
     sizeof(g_cust_cap.over_temper_protect.en_reduce_pwr), WLAN_CUST_NUM_1, OAL_FALSE},
    {"over_temp_pro_safe_th", WLAN_CFG_CAP_OVER_TEMP_PRO_SAFE_TH,
     &g_cust_cap.over_temper_protect.over_temp_pro_safe_th,
     sizeof(g_cust_cap.over_temper_protect.over_temp_pro_safe_th), WLAN_CUST_NUM_1, OAL_FALSE},
    {"over_temp_pro_over_th", WLAN_CFG_CAP_OVER_TEMP_PRO_OVER_TH,
     &g_cust_cap.over_temper_protect.over_temp_pro_over_th,
     sizeof(g_cust_cap.over_temper_protect.over_temp_pro_over_th), WLAN_CUST_NUM_1, OAL_FALSE},
    {"over_temp_pro_pa_off_th", WLAN_CFG_CAP_OVER_TEMP_PRO_PA_OFF_TH,
     &g_cust_cap.over_temper_protect.over_temp_pro_pa_off_th,
     sizeof(g_cust_cap.over_temper_protect.over_temp_pro_pa_off_th), WLAN_CUST_NUM_1, OAL_FALSE},

    {"dsss2ofdm_dbb_pwr_bo_val",   WLAN_CAP_DSSS2OFDM_DBB_PWR_BO_VAL,
     &g_cust_cap.dsss2ofdm_dbb_pwr_bo_val, sizeof(g_cust_cap.dsss2ofdm_dbb_pwr_bo_val), WLAN_CUST_NUM_1, OAL_FALSE},
    {"evm_fail_pll_reg_fix",       WLAN_CFG_CAP_EVM_PLL_REG_FIX,
     &g_cust_cap.evm_pll_reg_fix, sizeof(g_cust_cap.evm_pll_reg_fix), WLAN_CUST_NUM_1, OAL_FALSE},
    {"ac_priv_mask", WLAN_CFG_CAP_AC_SUSPEND,
     &g_cust_cap.ac_suspend_mask, sizeof(g_cust_cap.ac_suspend_mask), WLAN_CUST_NUM_1, OAL_FALSE},
    {"dyn_bypass_extlna_enable", WLAN_CFG_CAP_DYN_BYPASS_EXTLNA,
     &g_cust_cap.dyn_extlna_bypass, sizeof(g_cust_cap.dyn_extlna_bypass), WLAN_CUST_NUM_1, OAL_FALSE},
    {"ctrl_frame_tx_chain", WLAN_CFG_CAP_CTRL_FRAME_TX_CHAIN,
     &g_cust_cap.frame_tx_chain_ctl, sizeof(g_cust_cap.frame_tx_chain_ctl), WLAN_CUST_NUM_1, OAL_FALSE},
    {"11b_double_chain_bo_pow", WLAN_CFG_CAP_CTRL_11B_DOUBLE_CHAIN_BO_POW,
     &g_cust_cap.double_chain_11b_bo_pow, sizeof(g_cust_cap.double_chain_11b_bo_pow), WLAN_CUST_NUM_1, OAL_FALSE},
    {"hcc_flowctrl_type",  WLAN_CFG_CAP_HCC_FLOWCTRL_TYPE,
     &g_cust_cap.hcc_flowctrl_type, sizeof(g_cust_cap.hcc_flowctrl_type), WLAN_CUST_NUM_1, OAL_FALSE},
#ifdef _PRE_WLAN_FEATURE_MBO
    {"mbo_switch_mask", WLAN_CFG_CAP_MBO_SWITCH, &g_cust_cap.mbo_switch,
     sizeof(g_cust_cap.mbo_switch),   WLAN_CUST_NUM_1, OAL_FALSE},
#endif

    {"dc_flowctl_switch", WLAN_CFG_CAP_DC_FLOWCTL_SWITCH,
     &g_cust_cap.dc_flowctl_switch, sizeof(g_cust_cap.dc_flowctl_switch), WLAN_CUST_NUM_1, OAL_FALSE},
    {"phy_cap_mask", WLAN_CFG_CAP_PHY_CAP_SWITCH,
     &g_cust_cap.phy_cap_mask, sizeof(g_cust_cap.phy_cap_mask), WLAN_CUST_NUM_1, OAL_FALSE},
    {"hal_ps_rssi_param",          WLAN_CFG_CAP_HAL_PS_RSSI_PARAM,
     &g_cust_cap.ps_rssi_param, sizeof(g_cust_cap.ps_rssi_param), WLAN_CUST_NUM_1, OAL_FALSE},
    {"hal_ps_pps_param",           WLAN_CFG_CAP_HAL_PS_PPS_PARAM,
     &g_cust_cap.ps_pps_param, sizeof(g_cust_cap.ps_pps_param), WLAN_CUST_NUM_1, OAL_FALSE},

    {"hiex_cap", WLAN_CFG_CAP_HIEX_CAP, &g_cust_cap.hiex_cap,
     sizeof(g_cust_cap.hiex_cap),   WLAN_CUST_NUM_1, OAL_FALSE},

    {"fem_backoff_pow", WLAN_CFG_CAP_FEM_DELT_POW,
     &g_cust_cap.fem_pow.uc_fem_delt_pow, sizeof(g_cust_cap.fem_pow.uc_fem_delt_pow), WLAN_CUST_NUM_1, OAL_FALSE},
    {"tpc_adj_pow_start_idx_by_fem", WLAN_CFG_CAP_FEM_ADJ_TPC_TBL_START_IDX,
     &g_cust_cap.fem_pow.uc_tpc_adj_pow_start_idx, sizeof(g_cust_cap.fem_pow.uc_tpc_adj_pow_start_idx),
     WLAN_CUST_NUM_1, OAL_FALSE},
    {"powermgmt_switch", WLAN_CFG_CAP_POWERMGMT_SWITCH,
    &g_cust_cap.wlan_device_pm_switch, sizeof(g_cust_cap.wlan_device_pm_switch), WLAN_CUST_NUM_1, OAL_FALSE},
    {"min_fast_ps_idle", WLAN_CFG_CAP_MIN_FAST_PS_IDLE,
     &g_cust_cap.fast_ps.wlan_min_fast_ps_idle,
     sizeof(g_cust_cap.fast_ps.wlan_min_fast_ps_idle), WLAN_CUST_NUM_1, OAL_FALSE},
    {"max_fast_ps_idle", WLAN_CFG_CAP_MAX_FAST_PS_IDLE,
     &g_cust_cap.fast_ps.wlan_max_fast_ps_idle,
     sizeof(g_cust_cap.fast_ps.wlan_max_fast_ps_idle), WLAN_CUST_NUM_1, OAL_FALSE},
    {"auto_fast_ps_thresh_screen_on",  WLAN_CFG_CAP_AUTO_FAST_PS_THRESH_SCREENON,
     &g_cust_cap.fast_ps.wlan_auto_ps_thresh_screen_on,
     sizeof(g_cust_cap.fast_ps.wlan_auto_ps_thresh_screen_on), WLAN_CUST_NUM_1, OAL_FALSE},
    {"auto_fast_ps_thresh_screen_off", WLAN_CFG_CAP_AUTO_FAST_PS_THRESH_SCREENOFF,
     &g_cust_cap.fast_ps.wlan_auto_ps_thresh_screen_off,
     sizeof(g_cust_cap.fast_ps.wlan_auto_ps_thresh_screen_off), WLAN_CUST_NUM_1, OAL_FALSE},

    /* 可维可测 */
    {"loglevel", WLAN_CFG_CAP_LOGLEVEL,
     &g_cust_cap.loglevel, sizeof(g_cust_cap.loglevel), WLAN_CUST_NUM_1, OAL_FALSE},
    {"used_mem_for_start",      WLAN_CFG_CAP_USED_MEM_FOR_START,
     &g_cust_cap.used_mem_for_start, sizeof(g_cust_cap.used_mem_for_start), WLAN_CUST_NUM_1, OAL_FALSE},
    {"used_mem_for_stop", WLAN_CFG_CAP_USED_MEM_FOR_STOP,
     &g_cust_cap.used_mem_for_stop, sizeof(g_cust_cap.used_mem_for_stop), WLAN_CUST_NUM_1, OAL_FALSE},
    {"sdio_d2h_assemble_count", WLAN_CFG_CAP_SDIO_D2H_ASSEMBLE_COUNT,
     &g_cust_cap.sdio_assem_d2h, sizeof(g_cust_cap.sdio_assem_d2h), WLAN_CUST_NUM_1, OAL_FALSE},
    {"sdio_h2d_assemble_count", WLAN_CFG_CAP_SDIO_H2D_ASSEMBLE_COUNT,
     &g_cust_cap.sdio_assem_h2d, sizeof(g_cust_cap.sdio_assem_h2d), WLAN_CUST_NUM_1, OAL_FALSE},
    /* LINKLOSS */
    {"link_loss_threshold_bt",     WLAN_CFG_CAP_LINK_LOSS_THRESHOLD_BT,
     &g_cust_cap.linkloss_th[WLAN_LINKLOSS_MODE_BT],
     sizeof(g_cust_cap.linkloss_th[WLAN_LINKLOSS_MODE_BT]), WLAN_CUST_NUM_1, OAL_FALSE},
    {"link_loss_threshold_dbac",   WLAN_CFG_CAP_LINK_LOSS_THRESHOLD_DBAC,
     &g_cust_cap.linkloss_th[WLAN_LINKLOSS_MODE_DBAC],
     sizeof(g_cust_cap.linkloss_th[WLAN_LINKLOSS_MODE_DBAC]), WLAN_CUST_NUM_1, OAL_FALSE},
    {"link_loss_threshold_normal", WLAN_CFG_CAP_LINK_LOSS_THRESHOLD_NORMAL,
     &g_cust_cap.linkloss_th[WLAN_LINKLOSS_MODE_NORMAL],
     sizeof(g_cust_cap.linkloss_th[WLAN_LINKLOSS_MODE_NORMAL]), WLAN_CUST_NUM_1, OAL_FALSE},

    /* 自动调频 */
    {"pss_threshold_level_0",  WLAN_CFG_CAP_PSS_THRESHOLD_LEVEL_0,
     &g_cust_cap.dev_frequency[DEV_WORK_FREQ_LVL0].speed_level,
     sizeof(g_cust_cap.dev_frequency[DEV_WORK_FREQ_LVL0].speed_level), WLAN_CUST_NUM_1, OAL_FALSE},
    {"pss_threshold_level_1",  WLAN_CFG_CAP_PSS_THRESHOLD_LEVEL_1,
     &g_cust_cap.dev_frequency[DEV_WORK_FREQ_LVL1].speed_level,
     sizeof(g_cust_cap.dev_frequency[DEV_WORK_FREQ_LVL1].speed_level), WLAN_CUST_NUM_1, OAL_FALSE},
    {"pss_threshold_level_2",  WLAN_CFG_CAP_PSS_THRESHOLD_LEVEL_2,
     &g_cust_cap.dev_frequency[DEV_WORK_FREQ_LVL2].speed_level,
     sizeof(g_cust_cap.dev_frequency[DEV_WORK_FREQ_LVL2].speed_level), WLAN_CUST_NUM_1, OAL_FALSE},
    {"pss_threshold_level_3",  WLAN_CFG_CAP_PSS_THRESHOLD_LEVEL_3,
     &g_cust_cap.dev_frequency[DEV_WORK_FREQ_LVL3].speed_level,
      sizeof(g_cust_cap.dev_frequency[DEV_WORK_FREQ_LVL3].speed_level), WLAN_CUST_NUM_1, OAL_FALSE},

    {"device_type_level_0",    WLAN_CFG_CAP_DEVICE_TYPE_LEVEL_0,
     &g_cust_cap.dev_frequency[DEV_WORK_FREQ_LVL0].cpu_freq_type,
     sizeof(g_cust_cap.dev_frequency[DEV_WORK_FREQ_LVL0].cpu_freq_type), WLAN_CUST_NUM_1, OAL_FALSE},
    {"device_type_level_1",    WLAN_CFG_CAP_DEVICE_TYPE_LEVEL_1,
     &g_cust_cap.dev_frequency[DEV_WORK_FREQ_LVL1].cpu_freq_type,
     sizeof(g_cust_cap.dev_frequency[DEV_WORK_FREQ_LVL1].cpu_freq_type), WLAN_CUST_NUM_1, OAL_FALSE},
    {"device_type_level_2",    WLAN_CFG_CAP_DEVICE_TYPE_LEVEL_2,
     &g_cust_cap.dev_frequency[DEV_WORK_FREQ_LVL2].cpu_freq_type,
     sizeof(g_cust_cap.dev_frequency[DEV_WORK_FREQ_LVL2].cpu_freq_type), WLAN_CUST_NUM_1, OAL_FALSE},
    {"device_type_level_3",    WLAN_CFG_CAP_DEVICE_TYPE_LEVEL_3,
     &g_cust_cap.dev_frequency[DEV_WORK_FREQ_LVL3].cpu_freq_type,
     sizeof(g_cust_cap.dev_frequency[DEV_WORK_FREQ_LVL3].cpu_freq_type), WLAN_CUST_NUM_1, OAL_FALSE},
    /* btcoex mcm rssi */
    {"btcoex_threshold_mcm_down", WLAN_CFG_CAP_BTCOEX_THRESHOLD_MCM_DOWN,
     &g_cust_cap.btcoex_threshold_mcm[WLAN_BTCOEX_RSSI_MCM_DOWN],
     sizeof(g_cust_cap.btcoex_threshold_mcm[WLAN_BTCOEX_RSSI_MCM_DOWN]), WLAN_CUST_NUM_1, OAL_FALSE},
    {"btcoex_threshold_mcm_up",   WLAN_CFG_CAP_BTCOEX_THRESHOLD_MCM_UP,
     &g_cust_cap.btcoex_threshold_mcm[WLAN_BTCOEX_RSSI_MCM_UP],
     sizeof(g_cust_cap.btcoex_threshold_mcm[WLAN_BTCOEX_RSSI_MCM_UP]), WLAN_CUST_NUM_1, OAL_FALSE},
    {"ftm_cap", WLAN_CFG_CAP_FTM, &g_cust_cap.ftm_cap, sizeof(g_cust_cap.ftm_cap), WLAN_CUST_NUM_1, OAL_FALSE},
    {"ftm_board_delay", WLAN_CFG_CAP_FTM_BOARD_DELAY, &g_cust_cap.ftm_board_delay,
     sizeof(g_cust_cap.ftm_board_delay), WLAN_CUST_NUM_1, OAL_FALSE},

    {"only_fast_mode", WLAN_CFG_CAP_FAST_MODE, &g_cust_cap.fast_mode,
     sizeof(g_cust_cap.fast_mode), WLAN_CUST_NUM_1, OAL_FALSE},
    {"trx_switch", WLAN_CFG_CAP_TRX_SWITCH, &g_cust_cap.trx_switch, sizeof(g_cust_cap.trx_switch),
     WLAN_CUST_NUM_1, OAL_FALSE},

#ifdef _PRE_WLAN_FEATURE_NRCOEX
    { "nrcoex_enable", WLAN_CFG_CAP_NRCOEX_ENABLE, &g_cust_cap.nrcoex_params.nrcoex_switch,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_switch), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_version", WLAN_CFG_CAP_NRCOEX_VERSION, &g_cust_cap.nrcoex_params.version_num,
      sizeof(g_cust_cap.nrcoex_params.version_num), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_hw", WLAN_CFG_CAP_NRCOEX_HW, &g_cust_cap.nrcoex_params.nrcoex_hw,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_hw), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_advance_low_power", WLAN_CFG_CAP_NRCOEX_ADVANCE_LOW_POWER, &g_cust_cap.nrcoex_params.advance_low_power,
      sizeof(g_cust_cap.nrcoex_params.advance_low_power), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule0_freq", WLAN_CFG_CAP_NRCOEX_RULE0_FREQ,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].freq,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].freq), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule0_40m_20m_gap0", WLAN_CFG_CAP_NRCOEX_RULE0_40M_20M_GAP0,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].relative_freq_gap0_40_20,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].relative_freq_gap0_40_20),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule0_160m_80m_gap0", WLAN_CFG_CAP_NRCOEX_RULE0_160M_80M_GAP0,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].relative_freq_gap0_160_80,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].relative_freq_gap0_160_80),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule0_40m_20m_gap1", WLAN_CFG_CAP_NRCOEX_RULE0_40M_20M_GAP1,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].relative_freq_gap1_40_20,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].relative_freq_gap1_40_20),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule0_160m_80m_gap1", WLAN_CFG_CAP_NRCOEX_RULE0_160M_80M_GAP1,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].relative_freq_gap1_160_80,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].relative_freq_gap1_160_80),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule0_40m_20m_gap2", WLAN_CFG_CAP_NRCOEX_RULE0_40M_20M_GAP2,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].relative_freq_gap2_40_20,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].relative_freq_gap2_40_20),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule0_160m_80m_gap2", WLAN_CFG_CAP_NRCOEX_RULE0_160M_80M_GAP2,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].relative_freq_gap2_160_80,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].relative_freq_gap2_160_80),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule0_smallgap0_act", WLAN_CFG_CAP_NRCOEX_RULE0_SMALLGAP0_ACT,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].limit_power_level0,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].limit_power_level0), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule0_gap01_act", WLAN_CFG_CAP_NRCOEX_RULE0_GAP01_ACT,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].limit_power_level1,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].limit_power_level1), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule0_gap12_act", WLAN_CFG_CAP_NRCOEX_RULE0_GAP12_ACT,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].limit_power_level2,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].limit_power_level2), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule0_rxslot_rssi", WLAN_CFG_CAP_NRCOEX_RULE0_RXSLOT_RSSI,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].rssi_threshold,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_0].rssi_threshold), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule1_freq", WLAN_CFG_CAP_NRCOEX_RULE1_FREQ,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].freq,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].freq), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule1_40m_20m_gap0", WLAN_CFG_CAP_NRCOEX_RULE1_40M_20M_GAP0,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].relative_freq_gap0_40_20,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].relative_freq_gap0_40_20),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule1_160m_80m_gap0", WLAN_CFG_CAP_NRCOEX_RULE1_160M_80M_GAP0,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].relative_freq_gap0_160_80,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].relative_freq_gap0_160_80),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule1_40m_20m_gap1", WLAN_CFG_CAP_NRCOEX_RULE1_40M_20M_GAP1,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].relative_freq_gap1_40_20,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].relative_freq_gap1_40_20),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule1_160m_80m_gap1", WLAN_CFG_CAP_NRCOEX_RULE1_160M_80M_GAP1,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].relative_freq_gap1_160_80,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].relative_freq_gap1_160_80),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule1_40m_20m_gap2", WLAN_CFG_CAP_NRCOEX_RULE1_40M_20M_GAP2,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].relative_freq_gap2_40_20,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].relative_freq_gap2_40_20),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule1_160m_80m_gap2", WLAN_CFG_CAP_NRCOEX_RULE1_160M_80M_GAP2,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].relative_freq_gap2_160_80,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].relative_freq_gap2_160_80),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule1_smallgap0_act", WLAN_CFG_CAP_NRCOEX_RULE1_SMALLGAP0_ACT,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].limit_power_level0,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].limit_power_level0), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule1_gap01_act", WLAN_CFG_CAP_NRCOEX_RULE1_GAP01_ACT,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].limit_power_level1,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].limit_power_level1), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule1_gap12_act", WLAN_CFG_CAP_NRCOEX_RULE1_GAP12_ACT,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].limit_power_level2,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].limit_power_level2), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule1_rxslot_rssi", WLAN_CFG_CAP_NRCOEX_RULE1_RXSLOT_RSSI,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].rssi_threshold,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_1].rssi_threshold), WLAN_CUST_NUM_1, OAL_FALSE },

    { "nrcoex_rule2_freq", WLAN_CFG_CAP_NRCOEX_RULE2_FREQ,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].freq,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].freq), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule2_40m_20m_gap0", WLAN_CFG_CAP_NRCOEX_RULE2_40M_20M_GAP0,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].relative_freq_gap0_40_20,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].relative_freq_gap0_40_20),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule2_160m_80m_gap0", WLAN_CFG_CAP_NRCOEX_RULE2_160M_80M_GAP0,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].relative_freq_gap0_160_80,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].relative_freq_gap0_160_80),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule2_40m_20m_gap1", WLAN_CFG_CAP_NRCOEX_RULE2_40M_20M_GAP1,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].relative_freq_gap1_40_20,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].relative_freq_gap1_40_20),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule2_160m_80m_gap1", WLAN_CFG_CAP_NRCOEX_RULE2_160M_80M_GAP1,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].relative_freq_gap1_160_80,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].relative_freq_gap1_160_80),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule2_40m_20m_gap2", WLAN_CFG_CAP_NRCOEX_RULE2_40M_20M_GAP2,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].relative_freq_gap2_40_20,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].relative_freq_gap2_40_20),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule2_160m_80m_gap2", WLAN_CFG_CAP_NRCOEX_RULE2_160M_80M_GAP2,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].relative_freq_gap2_160_80,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].relative_freq_gap2_160_80),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule2_smallgap0_act", WLAN_CFG_CAP_NRCOEX_RULE2_SMALLGAP0_ACT,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].limit_power_level0,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].limit_power_level0), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule2_gap01_act", WLAN_CFG_CAP_NRCOEX_RULE2_GAP01_ACT,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].limit_power_level1,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].limit_power_level1), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule2_gap12_act", WLAN_CFG_CAP_NRCOEX_RULE2_GAP12_ACT,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].limit_power_level2,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].limit_power_level2), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule2_rxslot_rssi", WLAN_CFG_CAP_NRCOEX_RULE2_RXSLOT_RSSI,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].rssi_threshold,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_2].rssi_threshold), WLAN_CUST_NUM_1, OAL_FALSE },

    { "nrcoex_rule3_freq", WLAN_CFG_CAP_NRCOEX_RULE3_FREQ,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].freq,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].freq), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule3_40m_20m_gap0", WLAN_CFG_CAP_NRCOEX_RULE3_40M_20M_GAP0,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].relative_freq_gap0_40_20,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].relative_freq_gap0_40_20),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule3_160m_80m_gap0", WLAN_CFG_CAP_NRCOEX_RULE3_160M_80M_GAP0,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].relative_freq_gap0_160_80,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].relative_freq_gap0_160_80),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule3_40m_20m_gap1", WLAN_CFG_CAP_NRCOEX_RULE3_40M_20M_GAP1,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].relative_freq_gap1_40_20,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].relative_freq_gap1_40_20),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule3_160m_80m_gap1", WLAN_CFG_CAP_NRCOEX_RULE3_160M_80M_GAP1,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].relative_freq_gap1_160_80,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].relative_freq_gap1_160_80),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule3_40m_20m_gap2", WLAN_CFG_CAP_NRCOEX_RULE3_40M_20M_GAP2,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].relative_freq_gap2_40_20,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].relative_freq_gap2_40_20),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule3_160m_80m_gap2", WLAN_CFG_CAP_NRCOEX_RULE3_160M_80M_GAP2,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].relative_freq_gap2_160_80,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].relative_freq_gap2_160_80),
      WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule3_smallgap0_act", WLAN_CFG_CAP_NRCOEX_RULE3_SMALLGAP0_ACT,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].limit_power_level0,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].limit_power_level0), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule3_gap01_act", WLAN_CFG_CAP_NRCOEX_RULE3_GAP01_ACT,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].limit_power_level1,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].limit_power_level1), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule3_gap12_act", WLAN_CFG_CAP_NRCOEX_RULE3_GAP12_ACT,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].limit_power_level2,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].limit_power_level2), WLAN_CUST_NUM_1, OAL_FALSE },
    { "nrcoex_rule3_rxslot_rssi", WLAN_CFG_CAP_NRCOEX_RULE3_RXSLOT_RSSI,
      &g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].rssi_threshold,
      sizeof(g_cust_cap.nrcoex_params.nrcoex_rule[CUST_NRCOEX_RULE_3].rssi_threshold), WLAN_CUST_NUM_1, OAL_FALSE },
#endif
    {NULL, 0, NULL, 0, 0, OAL_FALSE}
};

oal_bool_enum_uint8 hwifi_get_cust_read_status(int32_t cus_tag, int32_t cfg_id)
{
    wlan_cust_cfg_cmd *pgast_wifi_config = NULL;
    int32_t cust_cfg_butt = 0;

    switch (cus_tag) {
        case CUS_TAG_HOST:
            pgast_wifi_config = g_wifi_cfg_host;
            cust_cfg_butt = WLAN_CFG_HOST_BUTT;
            break;
        case CUS_TAG_POW:
            pgast_wifi_config = g_wifi_cfg_pow;
            cust_cfg_butt = NVRAM_PARA_PWR_INDEX_BUTT;
            break;
        case CUS_TAG_RF:
            pgast_wifi_config = g_wifi_cfg_rf_cali;
            cust_cfg_butt = WLAN_CFG_RF_CALI_BUTT;
            break;
        case CUS_TAG_DYN_POW:
            pgast_wifi_config = g_wifi_cfg_dyn_pow;
            cust_cfg_butt = WLAN_CFG_DYN_POW_PARAMS_BUTT;
            break;
        case CUS_TAG_CAP:
            pgast_wifi_config = g_wifi_cfg_cap;
            cust_cfg_butt = WLAN_CFG_CAP_BUTT;
            break;

        default:
            oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_config_init_etc tag number[0x%x] not correct!", cus_tag);
            return OAL_FALSE;
    }

    if (cfg_id >= cust_cfg_butt) {
        return OAL_FALSE;
    }
    return pgast_wifi_config[cfg_id].en_value_state;
}

/*
 * 函 数 名  : hwifi_config_init_etc
 * 功能描述  : 定制化参数赋值接口
 *             读取ini文件，更新 host_init_params_etc 全局数组
 */
int32_t hwifi_set_cust_list_val(int32_t *param_list, uint16_t list_len, uint8_t *data_addr, uint8_t data_size)
{
    uint8_t param_idx;
    if (data_size > sizeof(int)) {
        oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_cpy_cust_val: data size[%d] not support!", data_size);
        oal_io_print("hwifi_cpy_cust_val: data size[%d] not support!", data_size);
        return INI_FAILED;
    }

    for (param_idx = 0; param_idx < list_len; param_idx++) {
        if (memcpy_s(data_addr, data_size, &param_list[param_idx], data_size) != EOK) {
            return INI_FAILED;
        }
        data_addr += data_size;
    }
    return INI_SUCC;
}

/*
 * 函 数 名  : hwifi_get_cust_item_list
 * 功能描述  : 从字符串中分割定制化项列表值
 * 参数  :  cust_param 字符串参数
            param_list 定制化参数列表
            list_len 参数列表长度
            max_len  列表最大长度

 */
static uint32_t hwifi_get_cust_item_list(uint8_t *cust_param, int32_t *param_list,
    uint16_t *list_len, uint16_t max_len)
{
    int8_t *pc_token = NULL;
    int8_t *pc_ctx = NULL;
    int8_t *pc_end = ";";
    int8_t *pc_sep = ",";
    uint16_t us_param_num = 0;
    uint8_t cust_param_str[CUS_PARAMS_LEN_MAX] = {0};
    int32_t ret;

    if (memcpy_s(cust_param_str, CUS_PARAMS_LEN_MAX, cust_param, OAL_STRLEN(cust_param)) != EOK) {
        return OAL_FAIL;
    }

    pc_token = oal_strtok(cust_param_str, pc_end, &pc_ctx);
    if (pc_token == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_config_sepa_coefficient_from_param read get null value check!");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pc_token = oal_strtok(pc_token, pc_sep, &pc_ctx);
    /* 获取定制化系数 */
    while (pc_token != NULL) {
        if (us_param_num == max_len) {
            oam_error_log2(0, OAM_SF_CUSTOM,
                           "hwifi_config_sepa_coefficient_from_param::nv or ini param is too many idx[%d] Max[%d]",
                           us_param_num, max_len);
            return OAL_FAIL;
        }

        /* 将字符串转换成数字 */
        if (!strncmp(pc_token, "0x", strlen("0x")) || !strncmp(pc_token, "0X", strlen("0X"))) {
            ret = sscanf_s(pc_token, "%x", &param_list[us_param_num]);
        } else {
            ret = sscanf_s(pc_token, "%d", &param_list[us_param_num]);
        }

        if (ret <= 0) {
            return OAL_FAIL;
        }

        pc_token = oal_strtok(NULL, pc_sep, &pc_ctx);
        us_param_num++;
    }

    *list_len = us_param_num;
    return OAL_SUCC;
}
/*
 * 函 数 名  : hwifi_config_get_cust_val
 * 功能描述  : 定制化项参数读取
 */
static int32_t hwifi_config_get_cust_val(wlan_cust_cfg_cmd *cfg_cmd)
{
    int32_t ret = INI_FAILED;
    uint8_t cust_params[CUS_PARAMS_LEN_MAX] = {0};
    uint16_t us_per_param_num = 0;
    int32_t nv_params[CUS_PARAMS_LEN_MAX] = {0};

    if ((cfg_cmd->data_addr == NULL) || (cfg_cmd->name == NULL) || cfg_cmd->data_num == 0) {
        oam_info_log4(0, OAM_SF_CUSTOM, "hwifi_config_get_cust_val::name[%p] cfg id[%d] data[%p] NULL or no data[%d]!",
            (uintptr_t)cfg_cmd->name, cfg_cmd->case_entry, (uintptr_t)cfg_cmd->data_addr, cfg_cmd->data_num);
        return INI_FAILED;
    }

    ret = get_cust_conf_string(INI_MODU_WIFI, cfg_cmd->name, cust_params, CUS_PARAMS_LEN_MAX - 1);
    if (ret == INI_SUCC) {
        if (hwifi_get_cust_item_list(cust_params, nv_params, &us_per_param_num, cfg_cmd->data_num) == OAL_SUCC) {
            ret = hwifi_set_cust_list_val(nv_params, us_per_param_num, cfg_cmd->data_addr, cfg_cmd->data_type);
        } else {
            ret = INI_FAILED;
        }

        return ret;
    } else {
        oal_io_print("hwifi_config_get_cust_val::tag %s read %s failed ret[%d] check ini files!",
            INI_MODU_WIFI, cfg_cmd->name, ret);
    }

    return ret;
}

int8_t hwifi_check_pwr_ref_delta_val(int8_t pwr_ref_delta)
{
    int32_t ret;
    if (pwr_ref_delta > CUST_PWR_REF_DELTA_HI) {
        ret = CUST_PWR_REF_DELTA_HI;
    } else if (pwr_ref_delta < CUST_PWR_REF_DELTA_LO) {
        ret = CUST_PWR_REF_DELTA_LO;
    } else {
        ret = pwr_ref_delta;
    }
    return ret;
}

int32_t hwifi_1106_uint_val_check(wlan_cust_range_stru *cust_check, uint8_t data_type)
{
    uint32_t val = 0;
    uint32_t read_val;
    uint32_t max_val = (uint32_t)cust_check->max_val;
    uint32_t min_val = (uint32_t)cust_check->min_val;
    uint32_t default_val = (uint32_t)cust_check->default_val;
    uint32_t len;
    uint8_t *addr = (uint8_t *)cust_check->data_addr;

    if (data_type == CUST_CFG_UINT8) {
        len = sizeof(uint8_t);
    } else if (data_type == CUST_CFG_UINT16) {
        len = sizeof(uint16_t);
    } else {
        len = sizeof(uint32_t);
    }
    memcpy_s(&val, len, addr, len);
    read_val = val;

    switch (cust_check->check_type) {
        case CUST_CHECK_MAX:
            CUS_CHECK_MAX_VAL(val, max_val, default_val);
            break;
        case CUST_CHECK_MIN:
            CUS_CHECK_MIN_VAL(val, min_val, default_val);
            break;
        case CUST_CHECK_VAL:
            CUS_CHECK_VAL(val, max_val, min_val, default_val);
            break;
        default:
            oam_error_log3(0, OAM_SF_ANY, "{hwifi_1106_uint32_val_check:: check type[%d] max %d min %d not support!}",
                cust_check->check_type, cust_check->max_val, cust_check->min_val);
            break;
    }

    memcpy_s(addr, len, &val, len);
    if (read_val != val) {
        oam_error_log4(0, OAM_SF_ANY, "{hwifi_1106_uint32_val_check::read_val[%x] val [%x] max val[%x] min val [%x]!}",
            read_val, val, max_val, min_val);

        return OAL_FAIL;
    }
    return OAL_SUCC;
}

int32_t hwifi_1106_int_val_check(wlan_cust_range_stru *cust_check, uint8_t data_type)
{
    int32_t val = 0;
    int32_t read_val;
    int32_t max_val = (int32_t)cust_check->max_val;
    int32_t min_val = (int32_t)cust_check->min_val;
    int32_t default_val = (int32_t)cust_check->default_val;
    uint32_t len;
    uint8_t *addr = (uint8_t *)cust_check->data_addr;
    if (data_type == CUST_CFG_INT8) {
        len = sizeof(int8_t);
    } else if (data_type == CUST_CFG_INT16) {
        len = sizeof(int16_t);
    } else {
        len = sizeof(int32_t);
    }
    memcpy_s((uint8_t *)&val, len, addr, len);
    read_val = val;

    switch (cust_check->check_type) {
        case CUST_CHECK_MAX:
            CUS_CHECK_MAX_VAL(val, max_val, default_val);
            break;
        case CUST_CHECK_MIN:
            CUS_CHECK_MIN_VAL(val, min_val, default_val);
            break;
        case CUST_CHECK_VAL:
            CUS_CHECK_VAL(val, max_val, min_val, default_val);
            break;
        default:
            oam_error_log3(0, OAM_SF_ANY, "{hwifi_1106_int_val_check:: check type[%d] max %d min %d not support!}",
                cust_check->check_type, cust_check->max_val, cust_check->min_val);
            break;
    }

    memcpy_s(addr, len, (uint8_t *)&val, len);
    if (read_val != val) {
        oam_error_log4(0, OAM_SF_ANY, "{hwifi_1106_int_val_check::read_val[%d] val [%d] max val[%d] min val [%d]!}",
            read_val, val, max_val, min_val);

        return OAL_FAIL;
    }
    return OAL_SUCC;
}

static void hwifi_1106_cust_val_tbl_check(wlan_cust_range_stru *range_tbl, uint32_t len)
{
    uint32_t data_idx;
    int32_t ret = OAL_SUCC;

    for (data_idx = 0; data_idx < len; data_idx++) {
        switch (range_tbl[data_idx].data_type) {
            case CUST_CFG_INT8:
            case CUST_CFG_INT16:
            case CUST_CFG_INT32:
                ret = hwifi_1106_int_val_check(&range_tbl[data_idx], range_tbl[data_idx].data_type);
                break;

            case CUST_CFG_UINT8:
            case CUST_CFG_UINT16:
            case CUST_CFG_UINT32:
                ret = hwifi_1106_uint_val_check(&range_tbl[data_idx], range_tbl[data_idx].data_type);
                break;

            default:
                oam_error_log4(0, OAM_SF_ANY,
                    "{hwifi_1106_cust_val_tbl_check::data idx[%d] check type[%d] max %d min %d not support!}",
                    data_idx, range_tbl[data_idx].check_type, range_tbl[data_idx].max_val,
                    range_tbl[data_idx].min_val);
                break;
        }
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_ANY, "{hwifi_1106_cust_val_tbl_check::data idx[%d] set invalid value!}", data_idx);
        }
    }
}
/* 定制化项有效范围检查 */
STATIC wlan_cust_range_stru g_host_cfg_range[] = {
    {&g_wlan_cust.uc_roam_switch, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_wlan_cust.uc_roam_scan_orthogonal, CUST_CFG_UINT8, CUST_CHECK_MIN,
        0, 1, CUS_ROAM_SCAN_ORTHOGONAL_DEFAULT},
    {&g_wlan_cust.ampdu_tx_max_num, CUST_CFG_UINT32, CUST_CHECK_VAL,
        WLAN_AMPDU_TX_MAX_NUM_1106, 1, WLAN_AMPDU_TX_MAX_NUM_1106},
    {&g_cust_host.freq_lock.en_irq_affinity, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.freq_lock.throughput_irq_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_IRQ_TH_LOW},
    {&g_cust_host.freq_lock.throughput_irq_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_IRQ_TH_HIGH},
    {&g_cust_host.freq_lock.irq_pps_low, CUST_CFG_UINT32, CUST_CHECK_MIN, 0, 1, WLAN_IRQ_PPS_TH_LOW},
    {&g_cust_host.freq_lock.irq_pps_high, CUST_CFG_UINT32, CUST_CHECK_MIN, 0, 1, WLAN_IRQ_PPS_TH_HIGH},
    {&g_cust_host.tx_ampdu.ampdu_hw_en, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.large_amsdu.large_amsdu_en, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.large_amsdu.throughput_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_AMSDU_AMPDU_TH_LOW},
    {&g_cust_host.large_amsdu.throughput_middle, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_AMSDU_AMPDU_TH_MIDDLE},
    {&g_cust_host.large_amsdu.throughput_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_AMSDU_AMPDU_TH_HIGH},
    {&g_cust_host.tx_ampdu.throughput_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_HW_AMPDU_TH_HIGH},
    {&g_cust_host.tx_ampdu.throughput_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_HW_AMPDU_TH_LOW},
    {&g_cust_host.tcp_ack_filt.filter_en, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.tcp_ack_filt.throughput_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_IRQ_TH_LOW},
    {&g_cust_host.tcp_ack_filt.throughput_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_FILTER_TH_HIGH},
    {&g_cust_host.small_amsdu.en_switch, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.small_amsdu.throughput_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_SMALL_AMSDU_HIGH},
    {&g_cust_host.small_amsdu.throughput_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_SMALL_AMSDU_LOW},
    {&g_cust_host.small_amsdu.pps_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_SMALL_AMSDU_PPS_LOW},
    {&g_cust_host.small_amsdu.pps_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_SMALL_AMSDU_PPS_HIGH},
    {&g_cust_host.tcp_ack_buf.buf_en, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.tcp_ack_buf.throughput_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_HIGH},
    {&g_cust_host.tcp_ack_buf.throughput_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_LOW},
    {&g_cust_host.tcp_ack_buf.throughput_high_40m, CUST_CFG_UINT16,
     CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_HIGH_40M},
    {&g_cust_host.tcp_ack_buf.throughput_low_40m, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_LOW_40M},
    {&g_cust_host.tcp_ack_buf.throughput_high_80m, CUST_CFG_UINT16,
     CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_HIGH_80M},
    {&g_cust_host.tcp_ack_buf.throughput_low_80m, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_LOW_80M},
    {&g_cust_host.tcp_ack_buf.throughput_high_160m, CUST_CFG_UINT16,
     CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_HIGH_160M},
    {&g_cust_host.tcp_ack_buf.buf_userctl_test_en, CUST_CFG_UINT16, CUST_CHECK_MIN, OAL_TRUE, OAL_FALSE, OAL_FALSE},
    {&g_cust_host.tcp_ack_buf.buf_userctl_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_USERCTL_HIGH},
    {&g_cust_host.tcp_ack_buf.buf_userctl_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_USERCTL_LOW},
    {&g_wlan_cust.uc_random_mac_addr_scan, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_wlan_cust.uc_disable_capab_2ght40, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.en_hmac_feature_switch[CUST_MIRACAST_SINK_SWITCH], CUST_CFG_UINT8,
     CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.en_hmac_feature_switch[CUST_MIRACAST_REDUCE_LOG_SWITCH], CUST_CFG_UINT8,
     CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.en_hmac_feature_switch[CUST_CORE_BIND_SWITCH], CUST_CFG_UINT8,
     CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_wlan_cust.lte_gpio_check_switch, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.dyn_extlna.switch_en, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.dyn_extlna.throughput_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_RX_DYN_BYPASS_EXTLNA_HIGH},
    {&g_cust_host.dyn_extlna.throughput_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_RX_DYN_BYPASS_EXTLNA_LOW},
    {&g_cust_host.wlan_11ac2g_switch, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
};
static void hwifi_host_cust_check_1106(void)
{
    hwifi_1106_cust_val_tbl_check(g_host_cfg_range, ARRAY_SIZE(g_host_cfg_range));
}
void hwifi_cust_rf_rssi_check_1106(void)
{
    uint8_t chan_idx;
    uint8_t rf_idx;
    for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
        for (chan_idx = 0; chan_idx < CUS_NUM_2G_DELTA_RSSI_NUM; ++chan_idx) {
            g_cust_rf_front.delta_pwr_ref.delta_rssi_2g[chan_idx][rf_idx] =
                hwifi_check_pwr_ref_delta_val(g_cust_rf_front.delta_pwr_ref.delta_rssi_2g[chan_idx][rf_idx]);
        }
        for (chan_idx = 0; chan_idx < CUS_NUM_5G_DELTA_RSSI_NUM; ++chan_idx) {
                g_cust_rf_front.delta_pwr_ref.delta_rssi_5g[chan_idx][rf_idx] =
                    hwifi_check_pwr_ref_delta_val(g_cust_rf_front.delta_pwr_ref.delta_rssi_5g[chan_idx][rf_idx]);
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
                g_cust_rf_front.delta_pwr_ref.delta_rssi_6g[chan_idx][rf_idx] =
                    hwifi_check_pwr_ref_delta_val(g_cust_rf_front.delta_pwr_ref.delta_rssi_6g[chan_idx][rf_idx]);
#endif
        }
    }

    /* RSSI amend */
    for (chan_idx = 0; chan_idx < CUS_NUM_2G_AMEND_RSSI_NUM; ++chan_idx) {
        for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
            CUS_CHECK_VAL(g_cust_rf_front.rssi_amend_cfg.amend_rssi_2g[chan_idx][rf_idx], WLAN_RF_RSSI_AMEND_TH_HIGH,
                WLAN_RF_RSSI_AMEND_TH_LOW, 0x0);
        }
    }

    for (chan_idx = 0; chan_idx < CUS_NUM_5G_AMEND_RSSI_NUM; ++chan_idx) {
        for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
            CUS_CHECK_VAL(g_cust_rf_front.rssi_amend_cfg.amend_rssi_5g[chan_idx][rf_idx], WLAN_RF_RSSI_AMEND_TH_HIGH,
                WLAN_RF_RSSI_AMEND_TH_LOW, 0x0);
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
            CUS_CHECK_VAL(g_cust_rf_front.rssi_amend_cfg.amend_rssi_6g[chan_idx][rf_idx], WLAN_RF_RSSI_AMEND_TH_HIGH,
                WLAN_RF_RSSI_AMEND_TH_LOW, 0x0);
#endif
        }
    }
}

void hwifi_cust_rf_check_1106(void)
{
    uint8_t chan_idx;
    uint8_t rf_idx;

    CUS_CHECK_MIN_VAL(g_cust_host.freq_lock.throughput_irq_low, 1, WLAN_IRQ_TH_LOW);

    /* 配置: fem口到天线口的负增益 */
    for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
        for (chan_idx = 0; chan_idx < MAC_NUM_2G_BAND; ++chan_idx) {
            /* 获取各2p4g 各band 0.25db及0.1db精度的线损值 */
            CUS_CHECK_VAL(g_cust_rf_front.rf_loss_gain_db.gain_db_2g[chan_idx][rf_idx], RF_LINE_TXRX_GAIN_DB_MAX,
                RF_LINE_TXRX_GAIN_DB_2G_MIN, 0xF4);
        }

        for (chan_idx = 0; chan_idx < MAC_NUM_5G_BAND; ++chan_idx) {
            /* 获取各5g 各band 0.25db及0.1db精度的线损值 */
            CUS_CHECK_VAL(g_cust_rf_front.rf_loss_gain_db.gain_db_5g[chan_idx][rf_idx], RF_LINE_TXRX_GAIN_DB_MAX,
                RF_LINE_TXRX_GAIN_DB_MAX, 0xF8);
        }
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        for (chan_idx = 0; chan_idx < MAC_NUM_6G_BAND; ++chan_idx) {
            /* 获取各5g 各band 0.25db及0.1db精度的线损值 */
            CUS_CHECK_MAX_VAL(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[chan_idx][rf_idx], 0, 0xF8);
        }
#endif
    }

    for (chan_idx = 0; chan_idx < WLAN_CALI_BAND_BUTT; ++chan_idx) {
        for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
            CUS_SET_BOOL(g_cust_rf_front.ext_rf[chan_idx].ext_switch_isexist[rf_idx]);
            CUS_SET_BOOL(g_cust_rf_front.ext_rf[chan_idx].ext_pa_isexist[rf_idx]);
            CUS_SET_BOOL(g_cust_rf_front.ext_rf[chan_idx].ext_lna_isexist[rf_idx]);
        }
    }

    hwifi_cust_rf_rssi_check_1106();
}

/*
 * 函 数 名  : hwifi_cfg_fcc_ce_5g_high_band_txpwr_nvram
 * 功能描述  : FCC/CE 5G高band认证参数刷新
 */
static void hwifi_cfg_fcc_ce_5g_high_band_txpwr_nvram(void)
{
    /* 获取管制域信息 */
    regdomain_enum_uint8 regdomain_type = hwifi_get_regdomain_from_country_code((uint8_t *)g_wifi_country_code);

    g_cust_pow.fcc_ce_max_pwr_for_5g_high_band = (uint8_t)((regdomain_type == REGDOMAIN_ETSI) ?
        cus_get_low_16bits(g_cust_pow.fcc_ce_max_pwr_for_5g_high_band) :
        cus_get_high_16bits((g_cust_pow.fcc_ce_max_pwr_for_5g_high_band)));
}

static void hwifi_base_pow_max_val_check(uint8_t rf_idx, wlan_cali_band_enum_uint8 freq_idx)
{
    uint8_t param_idx;
    uint8_t param_num[WLAN_CALI_BAND_BUTT] = {ARRAY_SIZE(g_cust_pow.txpwr_base_2g_params[WLAN_RF_CHANNEL_ZERO]),
        ARRAY_SIZE(g_cust_pow.txpwr_base_5g_params[WLAN_RF_CHANNEL_ZERO]),
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        ARRAY_SIZE(g_cust_pow.txpwr_base_6g_params[WLAN_RF_CHANNEL_ZERO]),
#endif
    };
    uint8_t *para_addr[WLAN_CALI_BAND_BUTT] = {g_cust_pow.txpwr_base_2g_params[rf_idx],
        g_cust_pow.txpwr_base_5g_params[rf_idx],
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        g_cust_pow.txpwr_base_6g_params[rf_idx],
#endif
    };
    uint8_t *pow = para_addr[freq_idx];

    for (param_idx = 0; param_idx < param_num[freq_idx]; param_idx++) {
        if (cus_val_invalid(pow[param_idx], CUS_MAX_BASE_TXPOWER_VAL, CUS_MIN_BASE_TXPOWER_VAL)) {
            oam_error_log4(0, OAM_SF_CUSTOM,
                "hwifi_cfg_max_txpwr_base_check freq[%d] rf[%d] band[%d] base pow out of range[%d]!",
                freq_idx, rf_idx, param_idx, CUS_MAX_BASE_TXPOWER_VAL);
            pow[param_idx] = CUS_MAX_BASE_TXPOWER_VAL;
        }
    }
}

static void hwifi_cfg_max_txpwr_base_check(void)
{
    uint8_t rf_idx;
    uint8_t freq_idx;

    /* 参数合理性检查 */
    for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
        for (freq_idx = WLAN_BAND_2G; freq_idx < WLAN_BAND_BUTT; freq_idx++) {
            hwifi_base_pow_max_val_check(rf_idx, freq_idx);
        }
    }
}

void hwifi_cust_pow_check_1106(void)
{
    hwifi_cfg_fcc_ce_5g_high_band_txpwr_nvram();
    hwifi_cfg_max_txpwr_base_check();
}

static void hwifi_cust_device_perf_check(void)
{
    /* SDIO FLOWCTRL */
    if (g_cust_cap.sdio_assem_h2d >= 1 && g_cust_cap.sdio_assem_h2d <= HISDIO_HOST2DEV_SCATT_MAX) {
        g_hcc_assemble_count = g_cust_cap.sdio_assem_h2d;
    } else {
        oam_error_log2(0, OAM_SF_ANY,
            "{hwifi_cust_device_perf_check::sdio_assem_h2d[%d] out of range(0,%d], check value in ini file!}\r\n",
            g_cust_cap.sdio_assem_h2d, HISDIO_DEV2HOST_SCATT_MAX);
    }

    /* SDIO ASSEMBLE COUNT:D2H */
    if (g_cust_cap.sdio_assem_d2h == 0 || g_cust_cap.sdio_assem_d2h > HISDIO_DEV2HOST_SCATT_MAX) {
        g_cust_cap.sdio_assem_d2h = HISDIO_DEV2HOST_SCATT_MAX;
        oam_error_log2(0, OAM_SF_ANY,
            "{hwifi_cust_device_perf_check::sdio_assem_d2h[%d] out of range(0,%d], check value in ini file!}\r\n",
            g_cust_cap.sdio_assem_d2h, HISDIO_DEV2HOST_SCATT_MAX);
    }
}

static void hwifi_cust_trx_switch_check(void)
{
    hi110x_board_info *bd_info = NULL;
    bd_info = get_hi110x_board_info();

    CUS_CHECK_MAX_VAL(g_cust_cap.trx_switch, WLAN_MAX_TRX_SWITCH, WLAN_DEFAULT_TRX_SWITCH);
    /* SDIO 只支持device tx和device rx */
    if (bd_info->wlan_download_channel == MODE_SDIO && g_cust_cap.trx_switch != WLAN_DEFAULT_TRX_SWITCH) {
        oam_error_log1(0, OAM_SF_ANY, "{hwifi_cust_trx_switch_check::SDIO can not set host tx/rx! trx_switch[%d]}",
            g_cust_cap.trx_switch);
        g_cust_cap.trx_switch = WLAN_DEFAULT_TRX_SWITCH;
    }
}

void hwifi_cust_cap_check_1106(void)
{
    g_wlan_host_pm_switch = (g_cust_cap.wlan_device_pm_switch == WLAN_DEV_ALL_ENABLE ||
        g_cust_cap.wlan_device_pm_switch == WLAN_DEV_LIGHT_SLEEP_SWITCH_EN) ? OAL_TRUE : OAL_FALSE;

    CUS_SET_BOOL(g_cust_cap.mbo_switch);
    CUS_SET_BOOL(g_cust_cap.dbac_dynamic_switch);

    hwifi_cust_trx_switch_check();
    hwifi_cust_device_perf_check();
}
static void hwifi_1106_cust_dyn_pow_dpn_check(void)
{
    uint8_t rf_idx;
    uint8_t band_idx;
    uint8_t mode_idx;
    for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
        for (mode_idx = 0; mode_idx < DYN_PWR_CUST_2G_SNGL_MODE_BUTT; mode_idx++) {
            for (band_idx = 0; band_idx < MAC_2G_CHANNEL_NUM; band_idx++) {
                CUS_CHECK_VAL(g_cust_dyn_pow.dpn_para[rf_idx].dpn_2g[mode_idx][band_idx],
                              CUS_DY_CALI_2G_VAL_DPN_MAX, CUS_DY_CALI_2G_VAL_DPN_MIN, 0);
            }
        }

        for (mode_idx = 0; mode_idx < WLAN_BW_CAP_80PLUS80; mode_idx++) {
            for (band_idx = 0; band_idx < MAC_NUM_5G_BAND; band_idx++) {
                CUS_CHECK_VAL(g_cust_dyn_pow.dpn_para[rf_idx].dpn_5g[mode_idx][band_idx],
                              CUS_DY_CALI_5G_VAL_DPN_MAX, CUS_DY_CALI_5G_VAL_DPN_MIN, 0);
            }
        }
    }
}

void hwifi_cust_dyn_pow_check_1106(void)
{
    hwifi_1106_cust_dyn_pow_dpn_check();
}

static void hwifi_check_custom_param_1106(int32_t cus_tag)
{
    switch (cus_tag) {
        case CUS_TAG_HOST:
            hwifi_host_cust_check_1106();
            break;
        case CUS_TAG_POW:
            hwifi_cust_pow_check_1106();
            break;
        case CUS_TAG_RF:
            hwifi_cust_rf_check_1106();
            break;
        case CUS_TAG_DYN_POW:
            hwifi_cust_dyn_pow_check_1106();
            break;
        case CUS_TAG_CAP:
            hwifi_cust_cap_check_1106();
            break;
        default:
            oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_config_init_etc tag number[0x%x] not correct!", cus_tag);
    }
};

uint8_t *g_fcc_ce_data_addr[] = {
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_20m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_40m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_80m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_160m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX0][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX1][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX2][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX3][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX4][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX5][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX6][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX7][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX8][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX9][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX10][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX11][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX12][0],
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_6g_20m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_6g_40m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_6g_80m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_sub_txpwr_limit_6g_20m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_sub_txpwr_limit_6g_40m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_sub_txpwr_limit_6g_80m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_6g_160m[0],
#endif
};

uint8_t g_fcc_ce_data_num[] = {
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_20m),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_40m),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_80m),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_160m),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX0]),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX1]),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX2]),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX3]),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX4]),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX5]),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX6]),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX7]),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX8]),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX9]),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX10]),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX11]),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX12]),
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_6g_20m),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_6g_40m),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_6g_80m),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_sub_txpwr_limit_6g_20m),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_sub_txpwr_limit_6g_40m),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_sub_txpwr_limit_6g_80m),
    ARRAY_SIZE(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_6g_160m),
#endif
};

/*
 * 函 数 名  : hwifi_config_init_fcc_ce_txpwr_cmd
 * 功能描述  : FCC/CE认证项，配置参数刷新
 */
static void hwifi_config_init_fcc_ce_txpwr_cmd(uint8_t rf_idx)
{
    int16_t cfg_id;
    regdomain_enum regdomain_type;
    int16_t start_idx;
    int16_t end_idx;
    int16_t fcc_start_id[] = {NVRAM_PARA_FCC_C0_START_INDEX,
        NVRAM_PARA_FCC_C1_START_INDEX, NVRAM_PARA_FCC_C2_START_INDEX, NVRAM_PARA_FCC_C3_START_INDEX};
    int16_t fcc_end_id[] = {NVRAM_PARA_FCC_C0_END_INDEX_BUTT,
        NVRAM_PARA_FCC_C1_END_INDEX_BUTT, NVRAM_PARA_FCC_C2_END_INDEX_BUTT, NVRAM_PARA_FCC_C3_END_INDEX_BUTT};
    int16_t ce_start_id[] = {NVRAM_PARA_CE_C0_START_INDEX,
        NVRAM_PARA_CE_C1_START_INDEX, NVRAM_PARA_CE_C2_START_INDEX, NVRAM_PARA_CE_C3_START_INDEX};
    int16_t ce_end_id[] = {NVRAM_PARA_CE_C0_END_INDEX_BUTT,
        NVRAM_PARA_CE_C1_END_INDEX_BUTT, NVRAM_PARA_CE_C1_END_INDEX_BUTT, NVRAM_PARA_CE_C1_END_INDEX_BUTT};

    /* 清空FCC/CE配置参数 */
    for (cfg_id = fcc_start_id[rf_idx]; cfg_id < fcc_end_id[rf_idx]; cfg_id++) {
        g_wifi_cfg_pow[cfg_id].data_addr = NULL;
        g_wifi_cfg_pow[cfg_id].data_type = 0;
        g_wifi_cfg_pow[cfg_id].data_num = 0;
    }
    for (cfg_id = ce_start_id[rf_idx]; cfg_id < ce_start_id[rf_idx]; cfg_id++) {
        g_wifi_cfg_pow[cfg_id].data_addr = NULL;
        g_wifi_cfg_pow[cfg_id].data_type = 0;
        g_wifi_cfg_pow[cfg_id].data_num = 0;
    }

    /* 获取管制域信息 */
    regdomain_type = hwifi_get_regdomain_from_country_code((uint8_t *)g_wifi_country_code);
    if (regdomain_type == REGDOMAIN_ETSI) {
        start_idx = ce_start_id[rf_idx];
        end_idx = ce_end_id[rf_idx];
    } else {
        start_idx = fcc_start_id[rf_idx];
        end_idx = fcc_end_id[rf_idx];
    }
    /* 根据国家码刷新对应值 */
    for (cfg_id = start_idx; cfg_id < end_idx; cfg_id++) {
        g_wifi_cfg_pow[cfg_id].data_addr =
            g_fcc_ce_data_addr[cfg_id - start_idx] + sizeof(hal_cfg_custom_fcc_txpwr_limit_stru) * rf_idx;
        g_wifi_cfg_pow[cfg_id].data_type = sizeof(uint8_t);
        g_wifi_cfg_pow[cfg_id].data_num = g_fcc_ce_data_num[cfg_id - start_idx];
    }
}
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
/*
 * 函 数 名  : hwifi_config_init_6g_sar_ctrl_cmd
 * 功能描述  : 更新sar命令相关参数
 */
static void hwifi_config_init_6g_sar_ctrl_cmd(uint8_t rf_idx)
{
    int16_t cfg_id;
    int16_t sar_start_id[] = {NVRAM_PARA_6G_C0_SAR_START_INDEX,
        NVRAM_PARA_6G_C1_SAR_START_INDEX, NVRAM_PARA_6G_C2_SAR_START_INDEX, NVRAM_PARA_6G_C3_SAR_START_INDEX};
    int16_t sar_end_id[] = {NVRAM_PARA_6G_C0_SAR_END_INDEX_BUTT,
        NVRAM_PARA_6G_C1_SAR_END_INDEX_BUTT, NVRAM_PARA_6G_C2_SAR_END_INDEX_BUTT, NVRAM_PARA_6G_C3_SAR_END_INDEX_BUTT};
    int16_t start_idx = sar_start_id[rf_idx];
    int16_t end_idx = sar_end_id[rf_idx];

    for (cfg_id = start_idx; cfg_id < end_idx; cfg_id++) {
        g_wifi_cfg_pow[cfg_id].data_addr =
            &g_cust_pow.sar_ctrl_params[cfg_id - start_idx].sar_ctrl_6g_params[rf_idx][0];
        g_wifi_cfg_pow[cfg_id].data_type =
            sizeof(g_cust_pow.sar_ctrl_params[cfg_id - start_idx].sar_ctrl_6g_params[rf_idx][0]);
        g_wifi_cfg_pow[cfg_id].data_num =
            ARRAY_SIZE(g_cust_pow.sar_ctrl_params[cfg_id - start_idx].sar_ctrl_6g_params[rf_idx]);
    }
}
#endif
/*
 * 函 数 名  : hwifi_config_init_sar_ctrl_cmd
 * 功能描述  : 更新sar命令相关参数
 */
static void hwifi_config_init_sar_ctrl_cmd(uint8_t rf_idx)
{
    int16_t cfg_id;
    int16_t sar_start_id[] = {NVRAM_PARA_C0_SAR_START_INDEX,
        NVRAM_PARA_C1_SAR_START_INDEX, NVRAM_PARA_C2_SAR_START_INDEX, NVRAM_PARA_C3_SAR_START_INDEX};
    int16_t sar_end_id[] = {NVRAM_PARA_C0_SAR_END_INDEX_BUTT,
        NVRAM_PARA_C1_SAR_END_INDEX_BUTT, NVRAM_PARA_C2_SAR_END_INDEX_BUTT, NVRAM_PARA_C3_SAR_END_INDEX_BUTT};
    int16_t start_idx = sar_start_id[rf_idx];
    int16_t end_idx = sar_end_id[rf_idx];

    for (cfg_id = start_idx; cfg_id < end_idx; cfg_id++) {
        g_wifi_cfg_pow[cfg_id].data_addr =
            &g_cust_pow.sar_ctrl_params[cfg_id - start_idx].sar_ctrl_params[rf_idx][0];
        g_wifi_cfg_pow[cfg_id].data_type =
            sizeof(g_cust_pow.sar_ctrl_params[cfg_id - start_idx].sar_ctrl_params[rf_idx][0]);
        g_wifi_cfg_pow[cfg_id].data_num =
            ARRAY_SIZE(g_cust_pow.sar_ctrl_params[cfg_id - start_idx].sar_ctrl_params[rf_idx]);
    }
}

/*
 * 函 数 名  : hwifi_cust_init_pow_nvram_cfg
 * 功能描述  : 更新pow命令相关参数
 */
static void hwifi_cust_init_pow_nvram_cfg(void)
{
    uint8_t rf_idx;
    for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
        hwifi_config_init_fcc_ce_txpwr_cmd(rf_idx);
        hwifi_config_init_sar_ctrl_cmd(rf_idx);
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        hwifi_config_init_6g_sar_ctrl_cmd(rf_idx);
#endif
    }
}

/*
 * 函 数 名  : hwifi_config_init_1106
 * 功能描述  : netdev open 调用的定制化总入口
 *             读取ini文件，更新定制化结构体成员数值
 */
int32_t hwifi_config_init_1106(int32_t cus_tag)
{
    int32_t cfg_id;
    wlan_cust_cfg_cmd *wifi_cust_cmd = NULL;
    int32_t wlan_cfg_butt = 0;

    switch (cus_tag) {
        case CUS_TAG_HOST:
            wifi_cust_cmd = g_wifi_cfg_host;
            wlan_cfg_butt = oal_array_size(g_wifi_cfg_host);
            break;
        case CUS_TAG_POW:
            hwifi_cust_init_pow_nvram_cfg();
            wifi_cust_cmd = g_wifi_cfg_pow;
            wlan_cfg_butt = oal_array_size(g_wifi_cfg_pow);
            break;
        case CUS_TAG_RF:
            wifi_cust_cmd = g_wifi_cfg_rf_cali;
            wlan_cfg_butt = oal_array_size(g_wifi_cfg_rf_cali);
            break;
        case CUS_TAG_DYN_POW:
            wifi_cust_cmd = g_wifi_cfg_dyn_pow;
            wlan_cfg_butt = oal_array_size(g_wifi_cfg_dyn_pow);
            break;
        case CUS_TAG_CAP:
            wifi_cust_cmd = g_wifi_cfg_cap;
            wlan_cfg_butt = oal_array_size(g_wifi_cfg_cap);
            break;

        default:
            oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_config_init_etc tag number[0x%x] not correct!", cus_tag);
            return INI_FAILED;
    }

    for (cfg_id = 0; cfg_id < wlan_cfg_butt; cfg_id++) {
        /* 获取ini的配置值 */
        wifi_cust_cmd[cfg_id].en_value_state =
            hwifi_config_get_cust_val(&wifi_cust_cmd[cfg_id]) == INI_SUCC ? OAL_TRUE : OAL_FALSE;
    }
    /* 检查定制化项取值范围 */
    hwifi_check_custom_param_1106(cus_tag);
    return INI_SUCC;
}

#define WLAN_CFG_DTS_NVRAM_END 100 /* 1106产测功能待适配 */
static uint8_t g_cust_nvram_info[WLAN_CFG_DTS_NVRAM_END][CUS_PARAMS_LEN_MAX] = {{0}}; /* NVRAM数组，1106产测功能待适配 */

static int32_t hwifi_cust_read_nvram_flag_1106(void)
{
    int32_t ret;
    oal_bool_enum_uint8 en_get_nvram_data_flag = OAL_FALSE;

    ret = hwifi_config_get_cust_val(&g_wifi_cfg_cap[WLAN_CFG_CAP_CALI_DATA_MASK]);
    if (ret == INI_SUCC) {
        en_get_nvram_data_flag = !(CUST_READ_NVRAM_MASK & g_cust_cap.cali_data_mask);
        if (!en_get_nvram_data_flag) {
            oal_io_print("hwifi_cust_read_nvram_flag_1106::get_nvram_data_flag[%d] to abandon nvram data!!\r\n",
                         g_cust_cap.cali_data_mask);
            memset_s(g_cust_nvram_info, sizeof(g_cust_nvram_info), 0, sizeof(g_cust_nvram_info));
        }
    }
    return en_get_nvram_data_flag;
}

static void hwifi_cust_get_nvram_params_1106(uint8_t *cust_nvram_info, uint8_t *buffer_cust_nvram_tmp, int32_t *params)
{
    /* 产测功能待适配 */
}
/*
 * 函 数 名  : hwifi_custom_host_read_dyn_cali_nvram
 * 功能描述  : 包括读取nvram中的dpint和校准系数值
 */
int32_t hwifi_cust_get_nvram_1106(void)
{
    uint8_t *buffer_cust_nvram_tmp = NULL;
    int32_t *params = NULL;
    uint8_t *cust_nvram_info = NULL; /* NVRAM数组 */

    /* 判断定制化中是否使用nvram中的动态校准参数 */
    if (!(hwifi_cust_read_nvram_flag_1106())) {
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    buffer_cust_nvram_tmp = (uint8_t *)os_kzalloc_gfp(CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
    if (buffer_cust_nvram_tmp == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_cust_get_nvram_1106::buffer_cust_nvram_tmp mem alloc fail!");
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    params = (int32_t *)os_kzalloc_gfp(DY_CALI_PARAMS_NUM * DY_CALI_PARAMS_TIMES * sizeof(int32_t));
    if (params == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_cust_get_nvram_1106::params mem alloc fail!");
        os_mem_kfree(buffer_cust_nvram_tmp);
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    cust_nvram_info = (uint8_t *)os_kzalloc_gfp(WLAN_CFG_DTS_NVRAM_END * CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
    if (cust_nvram_info == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_cust_get_nvram_1106::cust_nvram_info mem alloc fail!");
        os_mem_kfree(buffer_cust_nvram_tmp);
        os_mem_kfree(params);
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    hwifi_cust_get_nvram_params_1106(cust_nvram_info, buffer_cust_nvram_tmp, params);

    os_mem_kfree(buffer_cust_nvram_tmp);
    os_mem_kfree(params);

    /* 检查NVRAM是否修改 */
    if (oal_memcmp(cust_nvram_info, g_cust_nvram_info, sizeof(g_cust_nvram_info)) == 0) {
        os_mem_kfree(cust_nvram_info);
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    if (memcpy_s(g_cust_nvram_info, sizeof(g_cust_nvram_info),
                 cust_nvram_info, sizeof(g_cust_nvram_info)) != EOK) {
        os_mem_kfree(cust_nvram_info);
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    os_mem_kfree(cust_nvram_info);
    return INI_NVRAM_RECONFIG;
}

/* 读定制化配置文件&NVRAM */
uint32_t hwifi_custom_host_read_cfg_init_1106(void)
{
    int32_t ini_read_ret;
    int32_t l_nv_read_ret;
#ifdef HISI_CONN_NVE_SUPPORT
    int32_t conn_nv_init_ret;
    /* conn_nve 初始化 */
    conn_nv_init_ret = conn_nve_init_wifi_nvdata();
    /* 检查conn_nve初始化结果 */
    if (conn_nv_init_ret) {
        oam_warning_log0(0, OAM_SF_CFG, "conn_nve_init_wifi_nvdata failed");
    }
#endif
    /* 读取nvram参数是否修改 */
    l_nv_read_ret = hwifi_cust_get_nvram_1106();

    /* 检查定制化文件中的产线配置是否修改 */
    ini_read_ret = ini_file_check_conf_update();
    if (ini_read_ret || l_nv_read_ret) {
        oam_warning_log2(0, OAM_SF_CFG,
            "hwifi_custom_host_read_cfg_init_1106 config is updated. ini_read_ret %d, nv_read_ret ",
            ini_read_ret, l_nv_read_ret);
        hwifi_config_init_1106(CUS_TAG_DYN_POW);
    }

    if (ini_read_ret == INI_FILE_TIMESPEC_UNRECONFIG) {
        oam_warning_log0(0, OAM_SF_CFG, "hwifi_custom_host_read_cfg_init_1106 ini file is not updated");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    oam_warning_log0(0, OAM_SF_CFG, "hwifi_custom_host_read_cfg_init_1106 config is updated");

    ini_read_ret += hwifi_config_init_1106(CUS_TAG_HOST);
    ini_read_ret += hwifi_config_init_1106(CUS_TAG_CAP);
    ini_read_ret += hwifi_config_init_1106(CUS_TAG_RF);
    ini_read_ret += hwifi_config_init_1106(CUS_TAG_POW);
    if (oal_unlikely(ini_read_ret != OAL_SUCC)) {
        oal_io_print("hwifi_custom_host_read_cfg_init_1106 NV fail l_ret[%d].\r\n", ini_read_ret);
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : hwifi_custom_adapt_device_init_param_1106
 * 功能描述  : 配置定制化参数结束标志，下发到device触发device init流程
 */
static int32_t hwifi_custom_adapt_device_init_param_1106(uint8_t *puc_data)
{
    uint32_t data_len = 0;
    int32_t l_ret;
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};

    if (puc_data == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_init_param_1106::puc_data is NULL.}");
        return INI_FAILED;
    }
    /*
     * 发送消息的格式如下:
     * +-------------------------------------------------------------------+
     * | CFGID0    |DATA0 Length| DATA0 Value | ......................... |
     * +-------------------------------------------------------------------+
     * | 4 Bytes   |4 Byte      | DATA  Length| ......................... |
     * +-------------------------------------------------------------------+
     */
    st_syn_msg.en_syn_id = CUSTOM_CFGID_INI_ENDING_ID;
    st_syn_msg.len = 0;

    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - data_len), &st_syn_msg, sizeof(st_syn_msg));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_init_param_1106::memcpy_s fail[%d]. data_len[%d]}",
                       l_ret, data_len);
    }
    data_len += sizeof(st_syn_msg);

    return data_len;
}

static void hwifi_custom_adapt_hcc_flowctrl_type_1106(uint8_t *priv_cfg_value)
{
    if (hcc_bus_flowctrl_init(*priv_cfg_value) != OAL_SUCC) {
        /* GPIO流控中断注册失败，强制device使用SDIO流控(type = 0) */
        *priv_cfg_value = 0;
    }
    oal_io_print("hwifi_custom_adapt_hcc_flowctrl_type_1106::sdio_flow_ctl_type[0x%x].\r\n",
                 *priv_cfg_value);
}
/*
 * 函 数 名  : hwifi_rf_cali_data_host_addr_init
 * 功能描述  : host校准数据内存初始化
 */
void hwifi_rf_cali_data_host_addr_init_1106(void)
{
    uint64_t host_phy_addr;
    uint64_t host_dev_addr = 0;
    hi110x_board_info *bd_info = get_hi110x_board_info();
    if (bd_info->wlan_download_channel != MODE_PCIE) {
        return;
    }

    g_cust_cap.cali_data_h_phy_addr = 0;
    host_phy_addr = get_cali_data_buf_phy_addr();
    if (host_phy_addr == 0) {
        oam_error_log0(0, OAM_SF_CFG, "{hwifi_rf_cali_data_h_addr_init:: mem alloc fail!} \n");
        return;
    }
    pcie_if_hostca_to_devva(0, host_phy_addr, &host_dev_addr);
    g_cust_cap.cali_data_h_phy_addr = host_dev_addr;
    oal_io_print("{hwifi_rf_cali_data_host_addr_init succ!} \n");
}
static int32_t hwifi_hcc_custom_get_data_1106(uint16_t syn_id, oal_netbuf_stru *netbuf)
{
    uint32_t data_len = 0;
    int32_t ret = EOK;
    uint8_t *netbuf_data = (uint8_t *)oal_netbuf_data(netbuf);

    switch (syn_id) {
        case CUSTOM_CFGID_INI_ID:
            /* 填充device初始化参数 */
            data_len = hwifi_custom_adapt_device_init_param_1106(netbuf_data);
            break;
        case CUSTOM_CFGID_CAP_ID:
            /* 开机默认打开校准数据上传下发 */
            g_cust_cap.cali_data_mask = hwifi_custom_cali_ini_param(g_cust_cap.cali_data_mask);
            if (hwifi_get_cust_read_status(CUS_TAG_CAP, WLAN_CFG_CAP_HCC_FLOWCTRL_TYPE)) {
                g_cust_cap.hcc_flowctrl_switch = OAL_TRUE;
                hwifi_custom_adapt_hcc_flowctrl_type_1106(&g_cust_cap.hcc_flowctrl_type);
            }
            data_len = sizeof(g_cust_cap);
            ret = memcpy_s(netbuf_data, WLAN_LARGE_NETBUF_SIZE, (uint8_t *)(&g_cust_cap), data_len);
            break;
        default:
            oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_custom_get_data_1106::unknown us_syn_id[%d]", syn_id);
            break;
    }

    if (ret != EOK) {
        oam_error_log3(0, OAM_SF_CFG,
                       "{hwifi_hcc_custom_get_data_1106::memcpy_s fail[%d] syn_id[%d] data_len[%d]}",
                       ret, syn_id, data_len);
    }
    return data_len;
}

/*
 * 函 数 名  : hwifi_hcc_custom_ini_data_buf_1106
 * 功能描述  : 下发定制化配置命令
 */
static int32_t hwifi_hcc_custom_ini_data_buf_1106(uint16_t us_syn_id)
{
    oal_netbuf_stru *pst_netbuf = NULL;
    uint32_t data_len;
    int32_t l_ret;
    uint32_t max_data_len = hcc_get_max_buf_len();
    struct hcc_transfer_param st_hcc_transfer_param = {0};
    struct hcc_handler *hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hwifi_hcc_custom_ini_data_buf_1106 hcc::is is null");
        return -OAL_EFAIL;
    }
    pst_netbuf = hwifi_hcc_custom_netbuf_alloc();
    if (pst_netbuf == NULL) {
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 组netbuf */
    data_len = hwifi_hcc_custom_get_data_1106(us_syn_id, pst_netbuf);
    if (data_len > max_data_len) {
        oam_error_log2(0, OAM_SF_CFG, "hwifi_hcc_custom_ini_data_buf_1106::got wrong data_len[%d] max_len[%d]",
            data_len, max_data_len);
        oal_netbuf_free(pst_netbuf);
        return OAL_FAIL;
    }

    if (data_len == 0) {
        oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_custom_ini_data_buf_1106::data is null us_syn_id[%d]", us_syn_id);
        oal_netbuf_free(pst_netbuf);
        return OAL_SUCC;
    }

    if ((pst_netbuf->data_len) || (pst_netbuf->data == NULL)) {
        oal_io_print("netbuf:0x%lx, len:%d\r\n", (uintptr_t)pst_netbuf, pst_netbuf->data_len);
        oal_netbuf_free(pst_netbuf);
        return OAL_FAIL;
    }

    oal_netbuf_put(pst_netbuf, data_len);
    hcc_hdr_param_init(&st_hcc_transfer_param, HCC_ACTION_TYPE_CUSTOMIZE, us_syn_id, 0, HCC_FC_WAIT, DATA_HI_QUEUE);

    l_ret = (uint32_t)hcc_tx(hcc, pst_netbuf, &st_hcc_transfer_param);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log2(0, OAM_SF_CFG, "hwifi_hcc_custom_ini_data_buf_1106 fail ret[%d]pst_netbuf[0x%lx]",
                       l_ret, (uintptr_t)pst_netbuf);
        oal_netbuf_free(pst_netbuf);
    }

    return l_ret;
}

uint32_t hwifi_hcc_customize_h2d_data_cfg_1106(void)
{
    int32_t l_ret;

    /* wifi上电时重读定制化配置 */
    l_ret = hwifi_custom_host_read_cfg_init_1106();
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "hwifi_hcc_customize_h2d_data_cfg_1106 data ret[%d]", l_ret);
    }

    /* 下发功能相关定制化 */
    l_ret = hwifi_hcc_custom_ini_data_buf_1106(CUSTOM_CFGID_CAP_ID);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_customize_h2d_data_cfg_1106 cap data fail, ret[%d]", l_ret);
        return INI_FAILED;
    }
    /* 下发device init命令 */
    l_ret = hwifi_hcc_custom_ini_data_buf_1106(CUSTOM_CFGID_INI_ID);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_customize_h2d_data_cfg_1106 init device fail, ret[%d]", l_ret);
        return INI_FAILED;
    }

    return INI_SUCC;
}

#define WAL_CUST_DATA_SEND_LEN ((WLAN_LARGE_NETBUF_SIZE) - HMAC_NETBUF_OFFSET)

static uint32_t hwifi_custom_data_send(oal_net_device_stru *cfg_net_dev,
    uint8_t *cust_param, int32_t param_len, uint8_t cfg_id)
{
    uint32_t ret = OAL_SUCC;

    uint16_t data_len;
    int32_t remain_len = param_len;
    uint8_t *data = cust_param;
    mac_vap_stru *mac_vap = oal_net_dev_priv(cfg_net_dev);

    while (remain_len > 0) {
        data_len = oal_min(remain_len, WAL_CUST_DATA_SEND_LEN);

        /* 如果所有参数都在有效范围内，则下发配置值 */
        ret += wal_send_custom_data(mac_vap, data_len, data, cfg_id);
        if (oal_unlikely(ret != OAL_SUCC)) {
            oam_error_log2(0, OAM_SF_ANY, "{hwifi_custom_data_send::cutsom data cfg id[id] fail[%d]!}", cfg_id, ret);
        }
        data += data_len;
        remain_len -= data_len;
    }

    return ret;
}

/*
 * 函 数 名  : hwifi_cust_rf_front_data_send
 * 功能描述  : hw 2g 5g 前端定制化
 */
static uint32_t hwifi_cust_rf_front_data_send(oal_net_device_stru *cfg_net_dev)
{
    return hwifi_custom_data_send(cfg_net_dev, (uint8_t *)(&g_cust_rf_front),
        sizeof(g_cust_rf_front), WLAN_CFGID_SET_CUS_RF_CFG);
}

static uint32_t hwifi_cust_rf_cali_data_send(oal_net_device_stru *cfg_net_dev)
{
    g_cust_rf_cali.band_5g_enable = mac_device_check_5g_enable_per_chip();
    return hwifi_custom_data_send(cfg_net_dev, (uint8_t *)(&g_cust_rf_cali),
        sizeof(g_cust_rf_cali), WLAN_CFGID_SET_CUS_RF_CALI);
}
static uint32_t hwifi_cust_nv_pow_data_send(oal_net_device_stru *cfg_net_dev)
{
    return hwifi_custom_data_send(cfg_net_dev, (uint8_t *)(&g_cust_pow),
        sizeof(g_cust_pow), WLAN_CFGID_SET_CUS_POW);
}

static uint32_t hwifi_cust_nv_dyn_pow_data_send(oal_net_device_stru *cfg_net_dev)
{
    return hwifi_custom_data_send(cfg_net_dev, (uint8_t *)(&g_cust_dyn_pow),
        sizeof(g_cust_dyn_pow), WLAN_CFGID_SET_CUS_DYN_POW_CALI);
}
/*
 * 函 数 名  : hwifi_cust_rf_cali_event_send
 * 功能描述  : 校准触发事件下发
 */
static uint32_t hwifi_cust_rf_cali_event_send(oal_net_device_stru *cfg_net_dev)
{
    hi110x_board_info *bd_info = get_hi110x_board_info();
    wlan_bus_type_enum_uint8 bus_type = WLAN_BUS_PCIE; /* 总线类型标记 */
    if (bd_info->wlan_download_channel != MODE_PCIE) {
        bus_type = WLAN_BUS_SDIO;
    }
    return hwifi_custom_data_send(cfg_net_dev, &bus_type, sizeof(wlan_bus_type_enum_uint8), WLAN_CFGID_SET_RF_CALI_RUN);
}

uint32_t hwifi_config_init_nvram_main_1106(oal_net_device_stru *pst_cfg_net_dev)
{
    hwifi_cust_nv_pow_data_send(pst_cfg_net_dev);
    return OAL_SUCC;
}

static void hwifi_config_init_ini_main_1106(oal_net_device_stru *cfg_net_dev)
{
    hwifi_cust_nv_pow_data_send(cfg_net_dev);

    /* 国家码 */
    hwifi_config_init_ini_country(cfg_net_dev);
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
    /* 自学习国家码初始化 */
    hwifi_config_selfstudy_init_country(cfg_net_dev);
#endif
    hwifi_cust_rf_front_data_send(cfg_net_dev);
    hwifi_cust_nv_dyn_pow_data_send(cfg_net_dev);
}

void wal_send_cali_data_1106(oal_net_device_stru *cfg_net_dev)
{
    hmac_set_20m_cali_data();
    hmac_send_cali_data_1106(oal_net_dev_priv(cfg_net_dev), WLAN_20M_ALL_CHN);
    hmac_send_cali_data_1106(oal_net_dev_priv(cfg_net_dev), WLAN_20M_COMMON);
}

uint32_t wal_custom_cali_1106(void)
{
    oal_net_device_stru *net_dev;
    uint32_t ret = OAL_SUCC;

    net_dev = wal_config_get_netdev("Hisilicon0", OAL_STRLEN("Hisilicon0"));  // 通过cfg vap0来下c0 c1校准
    if (net_dev == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    } else {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(net_dev);
        oam_warning_log0(0, OAM_SF_ANY, "{wal_custom_cali::the net_device is already exist!}");
    }

    hwifi_config_init_ini_main_1106(net_dev);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (g_custom_cali_done == OAL_TRUE) {
        /* 校准数据下发 */
        wlan_chip_send_cali_data(net_dev);
    } else {
        g_custom_cali_done = OAL_TRUE;
    }

    wal_send_cali_matrix_data(net_dev);
#endif
    /* 下发参数 */
    ret += hwifi_cust_rf_cali_data_send(net_dev);
    ret += hwifi_cust_rf_cali_event_send(net_dev);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_1106_custom_cali:init_dts_main fail!}");
    }

    return ret;
}

static void hwifi_config_bypass_extlna_ini_param_1106(void)
{
    mac_rx_dyn_bypass_extlna_stru *rx_dyn_bypass_extlna_switch = NULL;
    rx_dyn_bypass_extlna_switch = mac_vap_get_rx_dyn_bypass_extlna_switch();

    rx_dyn_bypass_extlna_switch->uc_ini_en = g_cust_host.dyn_extlna.switch_en;
    rx_dyn_bypass_extlna_switch->uc_cur_status = OAL_TRUE; /* 默认低功耗场景 */
    rx_dyn_bypass_extlna_switch->us_throughput_high = g_cust_host.dyn_extlna.throughput_high;
    rx_dyn_bypass_extlna_switch->us_throughput_low = g_cust_host.dyn_extlna.throughput_low;
}

static void hwifi_config_host_global_ini_param_extend_1106(void)
{
#ifdef _PRE_WLAN_FEATURE_MBO
    g_uc_mbo_switch = g_cust_cap.mbo_switch;
#endif
    g_uc_dbac_dynamic_switch = g_cust_cap.dbac_dynamic_switch;

#ifdef _PRE_WLAN_FEATURE_HIEX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        if (memcpy_s(&g_st_default_hiex_cap, sizeof(mac_hiex_cap_stru), &g_cust_cap.hiex_cap,
            sizeof(mac_hiex_cap_stru)) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "hwifi_config_host_global_ini_param_extend_1106::hiex cap memcpy fail!");
        }
    }
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    if (g_wlan_spec_cfg->feature_ftm_is_open) {
        g_mac_ftm_cap = g_cust_cap.ftm_cap;
    }
#endif
}


static void hwifi_config_dmac_freq_ini_param_1106(void)
{
    uint8_t uc_flag = OAL_TRUE;
    uint8_t uc_index;
    /******************************************** 自动调频 ********************************************/
    for (uc_index = 0; uc_index < DEV_WORK_FREQ_LVL_NUM; ++uc_index) {
        if (g_cust_cap.dev_frequency[uc_index].cpu_freq_type > FREQ_HIGHEST) {
            uc_flag = OAL_FALSE;
            break;
        }
    }

    if (uc_flag) {
        for (uc_index = 0; uc_index < DEV_WORK_FREQ_LVL_NUM; ++uc_index) {
            g_host_speed_freq_level[uc_index].speed_level = g_cust_cap.dev_frequency[uc_index].speed_level;
            g_device_speed_freq_level[uc_index].uc_device_type = g_cust_cap.dev_frequency[uc_index].cpu_freq_type;
        }
    }
}

static void hwifi_config_host_amsdu_th_ini_param_1106(void)
{
    mac_small_amsdu_switch_stru *small_amsdu_switch = mac_vap_get_small_amsdu_switch();

    g_st_tx_large_amsdu.uc_host_large_amsdu_en = g_cust_host.large_amsdu.large_amsdu_en;
    g_st_tx_large_amsdu.us_amsdu_throughput_high = g_cust_host.large_amsdu.throughput_high;
    g_st_tx_large_amsdu.us_amsdu_throughput_middle = g_cust_host.large_amsdu.throughput_middle;
    g_st_tx_large_amsdu.us_amsdu_throughput_low = g_cust_host.large_amsdu.throughput_low;
    oal_io_print("ampdu+amsdu lareg amsdu en[%d],high[%d],low[%d],middle[%d]\r\n",
        g_st_tx_large_amsdu.uc_host_large_amsdu_en, g_st_tx_large_amsdu.us_amsdu_throughput_high,
        g_st_tx_large_amsdu.us_amsdu_throughput_low, g_st_tx_large_amsdu.us_amsdu_throughput_middle);

    small_amsdu_switch->uc_ini_small_amsdu_en = g_cust_host.small_amsdu.en_switch;
    small_amsdu_switch->us_small_amsdu_throughput_high = g_cust_host.small_amsdu.throughput_high;
    small_amsdu_switch->us_small_amsdu_throughput_low = g_cust_host.small_amsdu.throughput_low;
    small_amsdu_switch->us_small_amsdu_pps_high = g_cust_host.small_amsdu.pps_high;
    small_amsdu_switch->us_small_amsdu_pps_low = g_cust_host.small_amsdu.pps_low;
    oal_io_print("SMALL AMSDU SWITCH en[%d],high[%d],low[%d]\r\n",
        small_amsdu_switch->uc_ini_small_amsdu_en, small_amsdu_switch->us_small_amsdu_throughput_high,
        small_amsdu_switch->us_small_amsdu_throughput_low);
}

static void hwifi_config_performance_ini_param_1106(void)
{
    mac_rx_buffer_size_stru *rx_buffer_size = mac_vap_get_rx_buffer_size();

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    g_st_ampdu_hw.uc_ampdu_hw_en = g_cust_host.tx_ampdu.ampdu_hw_en;
    g_st_ampdu_hw.us_throughput_high = g_cust_host.tx_ampdu.throughput_high;
    g_st_ampdu_hw.us_throughput_low = g_cust_host.tx_ampdu.throughput_low;
    oal_io_print("ampdu_hw enable[%d]H[%u]L[%u]\r\n", g_st_ampdu_hw.uc_ampdu_hw_en, g_st_ampdu_hw.us_throughput_high,
        g_st_ampdu_hw.us_throughput_low);
#endif

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    hwifi_config_host_amsdu_th_ini_param_1106();
#endif
#ifdef _PRE_WLAN_TCP_OPT
    g_st_tcp_ack_filter.uc_tcp_ack_filter_en = g_cust_host.tcp_ack_filt.filter_en;
    g_st_tcp_ack_filter.us_rx_filter_throughput_high = g_cust_host.tcp_ack_filt.throughput_high;
    g_st_tcp_ack_filter.us_rx_filter_throughput_low = g_cust_host.tcp_ack_filt.throughput_low;
    oal_io_print("tcp ack filter en[%d],high[%d],low[%d]\r\n", g_st_tcp_ack_filter.uc_tcp_ack_filter_en,
        g_st_tcp_ack_filter.us_rx_filter_throughput_high, g_st_tcp_ack_filter.us_rx_filter_throughput_low);
#endif

    g_uc_host_rx_ampdu_amsdu = g_cust_host.host_rx_ampdu_amsdu;
    oal_io_print("Rx:ampdu+amsdu skb en[%d]\r\n", g_uc_host_rx_ampdu_amsdu);

    rx_buffer_size->us_rx_buffer_size = g_cust_host.rx_buffer_size;
    rx_buffer_size->en_rx_ampdu_bitmap_ini = (g_cust_host.rx_buffer_size > 0) ? OAL_TRUE : OAL_FALSE;
    oal_io_print("Rx:ampdu bitmap size[%d]\r\n", rx_buffer_size->us_rx_buffer_size);
}

static void hwifi_config_tcp_ack_buf_ini_param_1106(void)
{
    mac_tcp_ack_buf_switch_stru *tcp_ack_buf_switch = mac_vap_get_tcp_ack_buf_switch();

    tcp_ack_buf_switch->uc_ini_tcp_ack_buf_en = g_cust_host.tcp_ack_buf.buf_en;
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high = g_cust_host.tcp_ack_buf.throughput_high;
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low = g_cust_host.tcp_ack_buf.throughput_low;
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high_40M = g_cust_host.tcp_ack_buf.throughput_high_40m;
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low_40M = g_cust_host.tcp_ack_buf.throughput_low_40m;
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high_80M = g_cust_host.tcp_ack_buf.throughput_high_80m;
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low_80M = g_cust_host.tcp_ack_buf.throughput_low_80m;
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high_160M = g_cust_host.tcp_ack_buf.throughput_high_160m;
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low_160M = g_cust_host.tcp_ack_buf.buf_userctl_test_en;
    tcp_ack_buf_switch->uc_ini_tcp_ack_buf_userctl_test_en = g_cust_host.tcp_ack_buf.buf_en;
    tcp_ack_buf_switch->us_tcp_ack_buf_userctl_high = g_cust_host.tcp_ack_buf.buf_userctl_high;
    tcp_ack_buf_switch->us_tcp_ack_buf_userctl_low = g_cust_host.tcp_ack_buf.buf_userctl_low;

    oal_io_print("TCP ACK BUF en[%d],high/low:20M[%d]/[%d],40M[%d]/[%d],80M[%d]/[%d],160M[%d]/[%d]\
        TCP ACK BUF USERCTL[%d], userctl[%d]/[%d]",
        tcp_ack_buf_switch->uc_ini_tcp_ack_buf_en, tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high,
        tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low, tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high_40M,
        tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low_40M, tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high_80M,
        tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low_80M, tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high_160M,
        tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low_160M, tcp_ack_buf_switch->uc_ini_tcp_ack_buf_userctl_test_en,
        tcp_ack_buf_switch->us_tcp_ack_buf_userctl_high, tcp_ack_buf_switch->us_tcp_ack_buf_userctl_low);
}


static void hwifi_set_voe_custom_param_1106(void)
{
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11k = (g_cust_cap.voe_switch_mask & BIT0) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11v = (g_cust_cap.voe_switch_mask & BIT1) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11r = (g_cust_cap.voe_switch_mask & BIT2) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11r_ds =
        (g_cust_cap.voe_switch_mask & BIT3) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_adaptive11r =
        (g_cust_cap.voe_switch_mask & BIT4) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_nb_rpt_11k =
        (g_cust_cap.voe_switch_mask & BIT5) ? OAL_TRUE : OAL_FALSE;

    return;
}

void hwifi_config_host_global_11ax_ini_param_1106(void)
{
    g_pst_mac_device_capability[0].en_11ax_switch =
        (g_cust_cap.wifi_11ax_switch_mask & BIT0) ? OAL_TRUE : OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_11AX
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_11ax_aput_switch =
        (g_cust_cap.wifi_11ax_switch_mask & BIT1) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_ignore_non_he_cap_from_beacon =
        (g_cust_cap.wifi_11ax_switch_mask & BIT2) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_11ax_aput_he_cap_switch =
        (g_cust_cap.wifi_11ax_switch_mask & BIT3) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_twt_responder_support =
        (g_cust_cap.wifi_11ax_switch_mask & BIT4) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_twt_requester_support =
        (g_cust_cap.wifi_11ax_switch_mask & BIT5) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_btwt_requester_support =
        (g_cust_cap.wifi_11ax_switch_mask & BIT6) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_flex_twt_support =
        (g_cust_cap.wifi_11ax_switch_mask & BIT7) ? OAL_TRUE : OAL_FALSE;
#endif

    g_pst_mac_device_capability[0].bit_multi_bssid_switch =
        (g_cust_cap.mult_bssid_switch_mask & BIT0) ? OAL_TRUE : OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_11AX
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_htc_include =
        (g_cust_cap.htc_switch_mask & BIT0) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_om_in_data =
        (g_cust_cap.htc_switch_mask & BIT1) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_rom_cap_switch =
        (g_cust_cap.htc_switch_mask & BIT2) ? OAL_TRUE : OAL_FALSE;
#endif
oal_io_print("hwifi_config_host_global_11ax_ini_param_1106 en[%d],mult bssid[%d],rom cap[%d]\r\n",
    g_pst_mac_device_capability[0].en_11ax_switch, g_pst_mac_device_capability[0].bit_multi_bssid_switch,
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_rom_cap_switch);
}


void hwifi_config_host_global_ini_param_1106(void)
{
    mac_rx_dyn_bypass_extlna_stru *rx_dyn_bypass_extlna_switch = mac_vap_get_rx_dyn_bypass_extlna_switch();

    /******************************************** 性能 ********************************************/
    wlan_chip_cpu_freq_ini_param_init();

    hwifi_config_tcp_ack_buf_ini_param_1106();

    hwifi_config_dmac_freq_ini_param_1106();

    hwifi_config_bypass_extlna_ini_param_1106();
    oal_io_print("DYN_BYPASS_EXTLNA SWITCH en[%d],high[%d],low[%d]\r\n", rx_dyn_bypass_extlna_switch->uc_ini_en,
        rx_dyn_bypass_extlna_switch->us_throughput_high, rx_dyn_bypass_extlna_switch->us_throughput_low);

    hwifi_config_performance_ini_param_1106();

    hwifi_config_host_global_ini_param_extend_1106();

    hwifi_set_voe_custom_param_1106();
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        hwifi_config_host_global_11ax_ini_param_1106();
    }
#endif
    return;
}



void hwifi_config_cpu_freq_ini_param_1106(void)
{
#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
    g_freq_lock_control.uc_lock_dma_latency = (g_cust_host.freq_lock.dma_latency_val > 0) ?  OAL_TRUE : OAL_FALSE;
    g_freq_lock_control.dma_latency_value = g_cust_host.freq_lock.dma_latency_val;
    oal_io_print("DMA latency[%d]\r\n", g_freq_lock_control.uc_lock_dma_latency);
    g_freq_lock_control.us_lock_cpu_th_high = g_cust_host.freq_lock.lock_cpu_th_high;
    g_freq_lock_control.us_lock_cpu_th_low = g_cust_host.freq_lock.lock_cpu_th_low;
#endif

    g_freq_lock_control.en_irq_affinity = g_cust_host.freq_lock.en_irq_affinity;
    g_freq_lock_control.us_throughput_irq_high = g_cust_host.freq_lock.throughput_irq_high;
    g_freq_lock_control.us_throughput_irq_low = g_cust_host.freq_lock.throughput_irq_low;
    g_freq_lock_control.irq_pps_high = g_cust_host.freq_lock.irq_pps_high;
    g_freq_lock_control.irq_pps_low = g_cust_host.freq_lock.irq_pps_low;
    oal_io_print("hwifi_config_cpu_freq_ini_param_1106 irq affinity enable[%d]High_th[%u]Low_th[%u]\r\n",
        g_freq_lock_control.en_irq_affinity, g_freq_lock_control.us_throughput_irq_high,
        g_freq_lock_control.us_throughput_irq_low);
}

/*
 * 函 数 名  : hwifi_config_init_etc
 * 功能描述  : 定制化读取结果打印
 */
static void hwifi_print_cust_val(wlan_cust_cfg_cmd *cfg_cmd)
{
    uint8_t idx;
    uint8_t *c_data = NULL;
    uint32_t *l_data = NULL;
    uint16_t *s_data = NULL;

    if (cfg_cmd->data_addr == NULL || cfg_cmd->name == NULL) {
        oal_io_print("hwifi_print_cust_val::cmd [%d] cmd_name or data_addr is NULL!", cfg_cmd->case_entry);
        return;
    }
    oal_io_print("hwifi_print_cust_val:name %s id %d = ", cfg_cmd->name, cfg_cmd->case_entry);

    if (cfg_cmd->data_type == sizeof(uint8_t)) {
        c_data = (uint8_t *)cfg_cmd->data_addr;
    } else if (cfg_cmd->data_type == sizeof(uint16_t)) {
        s_data = (uint16_t *)(cfg_cmd->data_addr);
    } else if (cfg_cmd->data_type == sizeof(uint32_t)) {
        l_data = (uint32_t *)(cfg_cmd->data_addr);
    }

    for (idx = 0; idx < cfg_cmd->data_num; idx++) {
        if (cfg_cmd->data_type == sizeof(uint8_t)) {
            oal_io_print("[0x%x]", *c_data);
            c_data++;
        } else if (cfg_cmd->data_type == sizeof(uint16_t)) {
            oal_io_print("[0x%x]", *s_data);
            s_data++;
        } else if (cfg_cmd->data_type == sizeof(uint32_t)) {
            oal_io_print("[0x%x]", *l_data);
            l_data++;
        }
    }
}

static void hwifi_show_customize_info(wlan_cust_cfg_cmd *wlan_cust_cfg, uint32_t num)
{
    uint32_t i;
    for (i = 0; i < num; i++) {
        hwifi_print_cust_val(&wlan_cust_cfg[i]);
    }
}

void hwifi_show_customize_info_1106(void)
{
    oal_io_print("\nhwifi_show_customize_info_1106\n");
    oal_io_print("CUSTOM_INFO:host_info");
    hwifi_show_customize_info(g_wifi_cfg_host, oal_array_size(g_wifi_cfg_host));
    oal_io_print("CUSTOM_INFO:cap_info");
    hwifi_show_customize_info(g_wifi_cfg_cap, oal_array_size(g_wifi_cfg_cap));
    oal_io_print("CUSTOM_INFO:rf_cali_info");
    hwifi_show_customize_info(g_wifi_cfg_rf_cali, oal_array_size(g_wifi_cfg_rf_cali));
    oal_io_print("CUSTOM_INFO:pow_info");
    hwifi_show_customize_info(g_wifi_cfg_pow, oal_array_size(g_wifi_cfg_pow));
    oal_io_print("CUSTOM_INFO:dyn_pow_info");
    hwifi_show_customize_info(g_wifi_cfg_dyn_pow, oal_array_size(g_wifi_cfg_dyn_pow));
}


uint32_t hwifi_get_sar_ctrl_params_1106(uint8_t lvl_num, uint8_t *data_addr,
    uint16_t *data_len, uint16_t dest_len)
{
    *data_len = sizeof(wlan_cust_sar_cfg_stru);
    if ((lvl_num <= CUS_NUM_OF_SAR_LVL) && (lvl_num > 0)) {
        lvl_num--;
        if (EOK != memcpy_s(data_addr, dest_len, &g_cust_pow.sar_ctrl_params[lvl_num], *data_len)) {
            oam_error_log0(0, OAM_SF_CFG, "hwifi_get_sar_ctrl_params_1106::memcpy fail!");
            return OAL_FAIL;
        }
    } else {
        memset_s(data_addr, dest_len, 0xFF, dest_len);
    }
    return OAL_SUCC;
}
