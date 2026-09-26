

#ifndef __WAL_LINUX_ATCMDSRV_H__
#define __WAL_LINUX_ATCMDSRV_H__

#include "oal_ext_if.h"
#include "wlan_types.h"
#include "wlan_spec.h"
#include "mac_vap.h"
#include "hmac_ext_if.h"
#include "wal_ext_if.h"
#include "wal_config.h"
#include "wal_linux_ioctl.h"
#include "hmac_cali_mgmt.h"
#include "hmac_scan.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_ATCMDSRV_H

#define WAL_ATCMDSRB_DBB_NUM_TIME (5 * OAL_TIME_HZ)
#define WAL_ATCMDSRB_GET_RX_PCKT  (5 * OAL_TIME_HZ)
#define WAL_ATCMDSRB_NORM_TIME    (1 * OAL_TIME_HZ)
#define FEM_FAIL_TIME             (3)

/* atcmdsrv私有命令宏定义 */
#define WAL_ATCMDSRV_IOCTL_DBB_LEN      12
#define WAL_ATCMDSRV_IOCTL_MAC_LEN      6
#define WAL_ATCMDSRV_IOCTL_COUNTRY_LEN  3
#define DP_INIT_EACH_CORE_NUM           13
#define DP_INIT_TWO_CORE_NUM            (DP_INIT_EACH_CORE_NUM * 2)
#define DP_INIT_FLAG_NUM                1
#define WAL_ATCMDSRV_IOCTL_DYN_INTV_LEN 3

#define WAL_ATCMDSRV_IOCTL_MODE_NUM     13
#define WAL_ATCMDSRV_IOCTL_DATARATE_NUM 15
#define WAL_ATCMDSRB_IOCTL_AL_TX_LEN    2000
#define WAL_ATCMDSRV_NV_WINVRAM_LENGTH  104 /* WINVRAM内存空间 */
#define WAL_ATCMDSRV_DIE_ID_LENGTH      16
#define WAL_ATCMDSRB_CHECK_FEM_PA       (5 * OAL_TIME_HZ)
#define WLAN_HT_ONLY_MODE_2G            WLAN_HT_ONLY_MODE + 3
#define WLAN_VHT_ONLY_MODE_2G           WLAN_VHT_ONLY_MODE + 3
#define WLAN_HE_MODE_5G                 WLAN_HE_MODE + 2
#define WLAN_HE_MODE_2G                 WLAN_HE_MODE + 3

/* lte共存对应的管脚号 */
#define WAL_ATCMDSRV_ISM_PRIORITY_NAME "ism_priority"
#define WAL_ATCMDSRV_LTE_RX_NAME       "lte_rx"
#define WAL_ATCMDSRV_LTE_TX_NAME       "lte_tx"
#define WAL_ATCMDSRV_LTE_INACT_NAME    "lte_inact"
#define WAL_ATCMDSRV_ISM_RX_ACT_NAME   "ism_rx_act"
#define WAL_ATCMDSRV_BANT_PRI_NAME     "bant_pri"
#define WAL_ATCMDSRV_BANT_STATUS_NAME  "bant_status"
#define WAL_ATCMDSRV_WANT_PRI_NAME     "want_pri"
#define WAL_ATCMDSRV_WANT_STATUS_NAME  "want_status"
#define WAL_ATCMDSRV_EFUSE_CHIP_ID_1103    0x03
#define WAL_ATCMDSRV_EFUSE_CHIP_ID_1105    0x05
#define WAL_ATCMDSRV_EFUSE_ADC_ERR_FLAG    0x0C
#define WAL_ATCMDSRV_EFUSE_BUFF_LEN        32
#define WAL_ATCMDSRV_EFUSE_REG_WIDTH       0x10
#define WAL_ATCMDSRV_CHANNEL_NUM           6
#define WAL_ATCMSRV_MIN_BSS_EXPIRATION_AGE 25  /* 单位:  秒 */
#define WAL_ATCMSRV_MAX_BSS_EXPIRATION_AGE 500 /* 单位:  秒 */

#define WAL_ATCMDSRV_WIFI_MIN_TXPOWER 5  /* 单位:dBm */
#define WAL_ATCMDSRV_WIFI_MAX_TXPOWER 14 /* 单位:dBm */

extern uint8_t g_uc_dev_lte_gpio_level;
extern uint16_t g_us_efuse_buffer[WAL_ATCMDSRV_EFUSE_BUFF_LEN];

/* atcmdsrv枚举类型 */
enum WAL_ATCMDSRV_IOCTL_CMD {
    WAL_ATCMDSRV_IOCTL_CMD_WI_FREQ_SET = 0, /*  */
    WAL_ATCMDSRV_IOCTL_CMD_WI_POWER_SET,    /*  */
    WAL_ATCMDSRV_IOCTL_CMD_MODE_SET,        /*  */
    WAL_ATCMDSRV_IOCTL_CMD_DATARATE_SET,
    WAL_ATCMDSRV_IOCTL_CMD_BAND_SET,
    WAL_ATCMDSRV_IOCTL_CMD_ALWAYS_TX_SET,
    WAL_ATCMDSRV_IOCTL_CMD_DBB_GET,
    WAL_ATCMDSRV_IOCTL_CMD_HW_STATUS_GET,
    WAL_ATCMDSRV_IOCTL_CMD_ALWAYS_RX_SET,
    WAL_ATCMDSRV_IOCTL_CMD_HW_ADDR_SET,
    WAL_ATCMDSRV_IOCTL_CMD_RX_PCKG_GET,
    WAL_ATCMDSRV_IOCTL_CMD_PM_SWITCH,
    WAL_ATCMDSRV_IOCTL_CMD_RX_RSSI,
    WAL_ATCMDSRV_IOCTL_CMD_CHIPCHECK_SET,
    WAL_ATCMDSRV_IOCTL_CMD_CHIPCHECK_RESULT,
    WAL_ATCMDSRV_IOCTL_CMD_CHIPCHECK_TIME,
    WAL_ATCMDSRV_IOCTL_CMD_UART_LOOP_SET,
    WAL_ATCMDSRV_IOCTL_CMD_SDIO_LOOP_SET,
    WAL_ATCMDSRV_IOCTL_CMD_RD_CALDATA, /* read caldata from dts */
    WAL_ATCMDSRV_IOCTL_CMD_SET_CALDATA,
    WAL_ATCMDSRV_IOCTL_CMD_EFUSE_CHECK,
    WAL_ATCMDSRV_IOCTL_CMD_SET_ANT,
    WAL_ATCMDSRV_IOCTL_CMD_DIEID_INFORM,
    WAL_ATCMDSRV_IOCTL_CMD_SET_COUNTRY,
    WAL_ATCMDSRV_IOCTL_CMD_GET_UPCCODE,
    WAL_ATCMDSRV_IOCTL_CMD_WIPIN_TEST,
    WAL_ATCMDSRV_IOCTL_CMD_PMU_CHECK,
    WAL_ATCMDSRV_IOCTL_CMD_SET_MIMO_CHANNEL, /* set mimo channel */
    WAL_ATCMDSRV_IOCTL_CMD_GET_DP_INIT,
    WAL_ATCMDSRV_IOCTL_CMD_GET_PDET_PARAM,
    WAL_ATCMDSRV_IOCTL_CMD_IO_TEST,
    WAL_ATCMDSRV_IOCTL_CMD_PCIE_TEST,
    WAL_ATCMDSRV_IOCTL_CMD_PCIE_SDIO_SET,
    WAL_ATCMDSRV_IOCTL_CMD_DYN_INTERVAL,
    WAL_ATCMDSRV_IOCTL_CMD_PT_STE,
    WAL_ATCMDSRV_IOCTL_CMD_TAS_ANT_SET,
    WAL_ATCMDSRV_IOCTL_CMD_SELFCALI_INTERVAL,
    WAL_ATCMDSRV_IOCTL_CMD_SET_BSS_EXPIRE_AGE, /* 设置扫描结果老化时间 */
    WAL_ATCMDSRV_IOCTL_CMD_GET_CONN_INFO,      /* 获取连接信息 */
    WAL_ATCMDSRV_IOCTL_CMD_SET_RUTEST,

    WAL_ATCMDSRV_IOCTL_CMD_TEST_BUTT
};
typedef enum {
    ATCMDSRV_WIFI_DISCONNECT, /*  */
    ATCMDSRV_WIFI_CONNECTED,  /*  */
} atcmdsrv_wifi_conn_info_enum;
typedef uint8_t atcmdsrv_wifi_conn_info_enum_uint8;

struct wal_atcmdsrv_wifi_connect_info {
    atcmdsrv_wifi_conn_info_enum_uint8 en_status;
    uint8_t auc_ssid[WLAN_SSID_MAX_LEN];
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    int8_t c_rssi;
};

struct wal_atcmdsrv_wifi_tx_power_range {
    uint8_t uc_min;
    uint8_t uc_max;
};

/* 1102 使用atcmdsrv 下发命令 */
typedef struct wal_atcmdsrv_wifi_priv_cmd {
    /* 校验位,取值1102,与其他平台区别开来 */
    int32_t l_verify;
    int32_t cmd; /* 命令号 */
    union {
        int32_t l_freq;
        int32_t l_userpow;
        int32_t l_pow;
        int32_t l_mode;
        int32_t l_datarate;
        int32_t l_bandwidth;
        int32_t l_awalys_tx;
        int32_t l_fem_pa_status;
        int32_t l_awalys_rx;
        int32_t l_rx_pkcg;
        int32_t l_pm_switch;
        int32_t l_rx_rssi;
        int32_t l_chipcheck_result;
        uint64_t ull_chipcheck_time;
        int32_t l_uart_loop_set;
        int32_t l_wifi_chan_loop_set;
        int32_t l_efuse_check_result;
        int32_t l_set_ant;
        int32_t l_upc_code;
        int32_t l_pin_status;
        int32_t l_pmu_status;
        int32_t l_mimo_channel;
        uint8_t auc_mac_addr[WAL_ATCMDSRV_IOCTL_MAC_LEN];        /* 6 */
        int8_t auc_dbb[WAL_ATCMDSRV_IOCTL_DBB_LEN];              /* 12 */
        uint8_t auc_caldata[WAL_ATCMDSRV_NV_WINVRAM_LENGTH];     /* 104 */
        uint16_t die_id[WAL_ATCMDSRV_DIE_ID_LENGTH];             /*  16 */
        int8_t auc_country_code[WAL_ATCMDSRV_IOCTL_COUNTRY_LEN]; /* 3 */
        int8_t ac_dp_init[DP_INIT_FLAG_NUM + DP_INIT_TWO_CORE_NUM];
        int8_t ac_pd_tran_param[WAL_ATCMDSRV_NV_WINVRAM_LENGTH];
        int16_t s_wkup_io_status;
        int16_t s_pcie_status;
        int16_t s_pcie_sdio_set;
        int8_t c_dyn_interval[WAL_ATCMDSRV_IOCTL_DYN_INTV_LEN];
        int32_t l_pt_set;
        int32_t l_tas_ant_set;
        int32_t l_runingtest_mode;
        int32_t l_selfcali_interval_set;
        uint32_t bss_expire_age;                          /* 产线扫描结果老化时间 */
        struct wal_atcmdsrv_wifi_connect_info st_connect_info; /* WiFi 连接信息 */
        st_wal_atcmdsrv_wifi_rumode_info st_ru_info;
    } pri_data;
} wal_atcmdsrv_wifi_priv_cmd_stru;
/*
 * EFUSE异常检查位域说明
 *        位域名称               位域起始位  位域与以下位段映射关系
 *   (1): DIE_ID                 [154:  0]   die_id_0,die_id1,die_id2,die_id3,die_id4
 *   (2): Reserve0               [159:155]   reserve0
 *   (3): CHIP ID                [167:160]   chip_id
 *   (4): Reserve1               [170:168]   reserve1
 *   (5): CHIP FUNCTION Value    [202:171]   chip_function_value_low,chip_function_value_high
 *   (6): ADC                    [206:203]   adc
 *   (7): Reserve2               [207:207]   reserve2
 *   (8): BCPU                   [208:208]   bcpu
 *   (9): Reserve3               [227:209]   reserve3_low,reserve3_high
 *   (10):PMU TRIM Value         [247:228]   pmu_trim_value
 *   (11):NFC PUN TRIM Value     [253:248]   nfc_pmu_trim_value
 *   (12):Reserve4               [255:254]   reserve4
 */
/* Note: 1103 has 512bits efuse, need to adapt this stuct */
typedef struct efuse_8_9 {
    uint32_t rsvd_0 : 3;
    uint32_t leakage : 7;
    uint32_t efuse_rd_8 : 6;
    uint32_t efuse_rd_9 : 16;
} efuse_8_9_reg;
typedef struct efuse_10_11 {
    uint32_t chip_id : 8;
    uint32_t chip_ver : 3;
    uint32_t wifi : 1;
    uint32_t bt : 1;
    uint32_t fm : 1;
    uint32_t gnss : 1;
    uint32_t ir : 1;
    uint32_t nfc : 1;
    uint32_t zigbee : 1;
    uint32_t rsvd_0 : 14;
} efuse_10_11_reg;
typedef struct efuse_16_17 {
    uint32_t pcs_common_clocks : 1;
    uint32_t vreg_bypass : 1;
    uint32_t phy_los_bias : 3;
    uint32_t phy_rx0_eq : 3;
    uint32_t phy_los_lvl : 5;
    uint32_t phy_tx_vboost_lvl : 3;
    uint32_t phy_ref_clkdiv2 : 1;
    uint32_t pcs_tx_gen : 6;
    uint32_t phy_tx0_term_ofst : 5;
    uint32_t phy_mpll_multi : 4;
} efuse_16_17_reg;
typedef struct efuse_18_19 {
    uint32_t phy_mpll_multi : 3;
    uint32_t pcs_tx_swing_full : 7;
    uint32_t pcst_tx_dee_gen1 : 6;
    uint32_t pcs_tx_dee_gen2 : 6;
    uint32_t pcs_tx_swing_low : 7;
    uint32_t pcie_efuse_sel : 1;
    uint32_t rsvd0 : 2;
} efuse_18_19_reg;
typedef struct efuse_20_21 {
    uint32_t ibas_leak_trimq_c0 : 2;
    uint32_t ibas_leak_trimi_c0 : 2;
    uint32_t qrefv0 : 5;
    uint32_t irefv0 : 5;
    uint32_t rsvd0 : 2;
    uint32_t ibas_leak_trimq_c1 : 2;
    uint32_t ibas_leak_trimi_c1 : 2;
    uint32_t qrefv1 : 5;
    uint32_t irefv1 : 5;
    uint32_t rsvd1 : 2;
} efuse_20_21_reg;
typedef struct efuse_22_23 {
    uint32_t irefv : 5;
    uint32_t trim_i : 2;
    uint32_t trim_q : 2;
    uint32_t rsvd0 : 7;
    uint32_t d_2g_c0 : 8;
    uint32_t g_2g_c0 : 6;
    uint32_t rsvd1 : 2;
} efuse_22_23_reg;
typedef struct efuse_24_25 {
    uint32_t d_5g_c0 : 8;
    uint32_t g_5g_c0 : 6;
    uint32_t rsvd0 : 2;
    uint32_t d_2g_c1 : 8;
    uint32_t g_2g_c1 : 6;
    uint32_t rsvd1 : 2;
} efuse_24_25_reg;
typedef struct efuse_26_27 {
    uint32_t d_5g_c1 : 8;
    uint32_t g_5g_c1 : 6;
    uint32_t rsvd0 : 2;
    uint32_t efuse_rd_27 : 16;
} efuse_26_27_reg;
typedef struct efuse_28_29 {
    uint32_t bbldo0_c0_trim : 3;
    uint32_t bbldo0_c1_trim : 3;
    uint32_t bbldo1_c0_trim : 3;
    uint32_t bbldo1_c1_trim : 3;
    uint32_t bbldo3_timr : 3;
    uint32_t rsvd0 : 1;
    uint32_t bbldo4_trim : 3;
    uint32_t rsvd1 : 5;
    uint32_t ir_trim : 4;
    uint32_t rsvd2 : 4;
} efuse_28_29_reg;

typedef struct efuse_30_31 {
    uint32_t rsvd0 : 5;
    uint32_t abb_bbldo2_trim : 3;
    uint32_t pmu_ref_bg_trim2_1 : 8;
    uint32_t pmu_ref_bg_trim2_2 : 4;
    uint32_t pmu_ref_bg_trim1 : 12;
} efuse_30_31_reg;

typedef struct efuse_bits {
    uint32_t efuse_rd_0_1;
    uint32_t efuse_rd_2_3;
    uint32_t efuse_rd_4_5;
    uint32_t efuse_rd_6_7;
    efuse_8_9_reg efuse_rd_8_9;
    efuse_10_11_reg efuse_rd_10_11;
    uint32_t efuse_rd_12_13;
    uint32_t efuse_rd_14_15;
    efuse_16_17_reg efuse_rd_16_17;
    efuse_18_19_reg efuse_rd_18_19;
    efuse_20_21_reg efuse_rd_20_21;
    efuse_22_23_reg efuse_rd_22_23;
    efuse_24_25_reg efuse_rd_24_25;
    efuse_26_27_reg efuse_rd_26_27;
    efuse_28_29_reg efuse_rd_28_29;
    efuse_30_31_reg efuse_rd_30_31;
} wal_efuse_bits;
/************************** end ***************************/
static inline uint8_t wal_atcmdsrv_get_hex_char(uint8_t val)
{
    // 9/10代表数字9、10，16进制如果超过9，转化为大写A-F
    return ((val > 9) ? ((val - 10) + 'A') : (val + '0'));
}
int32_t wal_atcmdsrv_wifi_priv_cmd(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr, int32_t cmd);
int32_t wal_atcmsrv_ioctl_get_hw_status(oal_net_device_stru *pst_net_dev,
                                        wal_atcmdsrv_wifi_priv_cmd_stru *pst_priv_cmd);
void wal_atcmsrv_ioctl_get_fem_pa_status(oal_net_device_stru *pst_net_dev, int32_t *pl_fem_pa_status);
int32_t wlan_device_mem_check(int32_t l_runing_test_mode);
int32_t wlan_device_mem_check_result(unsigned long long *time);
int32_t conn_test_uart_loop(char *param);
int32_t conn_test_wifi_chan_loop(const char *param);
int32_t conn_test_hcc_chann_switch(const char *new_dev);
int32_t hwifi_config_init(int32_t);
uint32_t wal_regdomain_get_channel_5g(uint32_t start_freq, uint32_t end_freq);
int32_t hi1103_pcie_ip_test(int32_t test_count);
int32_t hi1103_dev_io_test(void);
int32_t wal_atcmsrv_ioctl_get_rx_pckg_old(oal_net_device_stru *pst_net_dev,
                                          int32_t *pl_rx_pckg_succ_num);

#ifdef _PRE_WLAN_FEATURE_11D
int32_t wal_regdomain_update_for_dfs(oal_net_device_stru *pst_net_dev, int8_t *pc_country);
int32_t wal_regdomain_update(oal_net_device_stru *pst_net_dev, int8_t *pc_country);
#endif
#endif /* end of wal_linux_ioctl.h */
