

#ifndef __HISI_CUSTOMIZE_WIFI_H__
#define __HISI_CUSTOMIZE_WIFI_H__

#include "wlan_customize.h"
#include "hal_common.h"
#include "wlan_chip.h"

#define COUNTRY_CODE_LEN  3    /* 国家码位数 */
#define STR_COUNTRY_CODE "country_code"

#define CUS_ROAM_SCAN_ORTHOGONAL_DEFAULT    4

/* 同device侧定义 */
#define WLAN_DEV_ALL_ENABLE             1 /* 平台开低功耗+业务控rf和mac pa */
#define WLAN_DEV_LIGHT_SLEEP_SWITCH_EN  4 /* 平台关深睡+业务控mac pa+rf常开 */

/* 开机校准MASK */
#define CALI_FIST_POWER_ON_MASK BIT(2)
/* 读取NVRAM MASK */
#define CUST_READ_NVRAM_MASK BIT(4)

#define CUS_NUM_OF_SAR_PARAMS           8  /* 定制化降SAR参数 5G_BAND1~7 2.4G */
#define CUS_NUM_OF_SAR_PER_BAND_PAR_NUM 5  /* 每个band降SAR定制项数 */
#define CUS_NUM_OF_SAR_LVL              20 /* 定制化降SAR档位数 */
#define CUS_NUM_OF_SAR_ONE_PARAM_NUM    4

/* 动态校准参数个数,2.4g 4个(ofdm 20/40 11b cw),5g 5*2(high & low)个band,2.4g(ppa cw) */
#define DY_CALI_PARAMS_NUM        15
#define DY_CALI_PARAMS_BASE_NUM   9 /* 动态校准参数个数,2.4g 4个(ofdm 20/40 11b cw),5g 5(high)个band */
#define DY_CALI_PARAMS_TIMES      3 /* 动态校准参数二次项系数个数 */
#define DY_CALI_FIT_PRECISION_A1  6
#define DY_CALI_FIT_PRECISION_A0  16
#define CUS_MAX_BASE_TXPOWER_VAL  250                 /* 最大基准发送功率的最大有效值 */
#define CUS_MIN_BASE_TXPOWER_VAL  50                  /* 最小基准发送功率的最大有效值 */
#define DY_2G_CALI_PARAMS_NUM     4 /* 动态校准参数个数,2.4g */
#define DY_CALI_NUM_5G_BAND       5 /* 动态校准5g band1 2&3 4&5 6 7 */
#define CUS_BASE_PWR_NUM_5G       DY_CALI_NUM_5G_BAND /* 5g Base power 5个 band1 2&3 4&5 6 7 */
#define CUS_BASE_PWR_NUM_2G       1

#define CUS_PARAMS_LEN_MAX    104  /* 定制项最大长度 */

/* 定制化默认值 */
#define WLAN_ROAM_SWITCH_MODE             1
#define WLAN_SCAN_ORTHOGONAL_VAL          4
#define WLAN_TRIGGER_B_VAL                (-70)
#define WLAN_TRIGGER_A_VAL                (-70)
#define WLAN_DELTA_B_VAL                  10
#define WLAN_DELTA_A_VAL                  10
#define WLAN_MEM_FOR_START                45
#define WLAN_MEM_FOR_STOP                 25
#define WLAN_RX_ACK_LIMIT_VAL             10
#define WLAN_SDIO_H2D_ASSEMBLE_COUNT_VAL  16
#define WLAN_LOSS_THRESHOLD_WLAN_BT       80
#define WLAN_LOSS_THRESHOLD_WLAN_DBAC     80
#define WLAN_LOSS_THRESHOLD_WLAN_NORMAL   40
#define WLAN_IRQ_TH_HIGH                  250
#define WLAN_IRQ_TH_LOW                   150
#define WLAN_IRQ_PPS_TH_HIGH              25000
#define WLAN_IRQ_PPS_TH_LOW               5000
#define WLAN_HW_AMPDU_TH_LOW              200
#define WLAN_HW_AMPDU_TH_HIGH             300
#define WLAN_DEFAULT_TRX_SWITCH           0
#define WLAN_MAX_TRX_SWITCH               3
#define WLAN_AMSDU_AMPDU_TH_HIGH          300
#define WLAN_AMSDU_AMPDU_TH_LOW           200
#define WLAN_AMSDU_AMPDU_TH_MIDDLE        250
#define WLAN_TCP_ACK_FILTER_TH_HIGH       60
#define WLAN_TCP_ACK_FILTER_TH_LOW        20
#define WLAN_SMALL_AMSDU_HIGH             300
#define WLAN_SMALL_AMSDU_LOW              200
#define WLAN_SMALL_AMSDU_PPS_HIGH         25000
#define WLAN_SMALL_AMSDU_PPS_LOW          5000
#define WLAN_TCP_ACK_BUF_HIGH             90
#define WLAN_TCP_ACK_BUF_LOW              50
#define WLAN_TCP_ACK_BUF_HIGH_40M         300
#define WLAN_TCP_ACK_BUF_LOW_40M          150
#define WLAN_TCP_ACK_BUF_HIGH_80M         550
#define WLAN_TCP_ACK_BUF_LOW_80M          450
#define WLAN_TCP_ACK_BUF_HIGH_160M        800
#define WLAN_TCP_ACK_BUF_LOW_160M         700
#define WLAN_TCP_ACK_BUF_USERCTL_HIGH     30
#define WLAN_TCP_ACK_BUF_USERCTL_LOW      20
#define WLAN_RX_DYN_BYPASS_EXTLNA_HIGH    100
#define WLAN_RX_DYN_BYPASS_EXTLNA_LOW     50
#define WLAN_PS_MODE                      1
#define WLAN_MIN_FAST_PS_IDLE             1
#define WLAN_MAX_FAST_PS_IDLE             10
#define WLAN_AUTO_FAST_PS_SCREENON        5
#define WLAN_AUTO_FAST_PS_SCREENOFF       5
#define WLAN_BTCOEX_THRESHOLD_MCM_DOWN      (-45) /* 修改此值需要同步修改device */
#define WLAN_BTCOEX_THRESHOLD_MCM_UP        (-75) /* 修改此值需要同步修改device */
#define WLAN_RF_RSSI_AMEND_TH_HIGH   10
#define WLAN_RF_RSSI_AMEND_TH_LOW    (-10)
#define WLAN_DDR_CAHNL_FREQ 8640 /* 向内核申请非415M DDR频率 */
#define WLAN_HIEX_DEV_CAP 0x1000

#define RF_LINE_TXRX_GAIN_DB_2G_MIN (-100)
#define RF_LINE_TXRX_GAIN_DB_MAX     40
#define RF_LINE_TXRX_GAIN_DB_5G_MIN (-48)


/* 定制化HOST全局变量结构体 */
typedef struct {
    /* ba tx 聚合数 */
    unsigned int ampdu_tx_max_num;
    /* 漫游 */
    unsigned char uc_roam_switch;
    unsigned char uc_roam_scan_orthogonal;
    signed char c_roam_trigger_b;
    signed char c_roam_trigger_a;
    signed char c_roam_delta_b;
    signed char c_roam_delta_a;
    /* 漫游场景识别 */
    signed char c_dense_env_roam_trigger_b;
    signed char c_dense_env_roam_trigger_a;
    oal_bool_enum_uint8 uc_scenario_enable;
    signed char c_candidate_good_rssi;
    unsigned char uc_candidate_good_num;
    unsigned char uc_candidate_weak_num;
    /* 漫游间隔动态触发 */
    uint16_t us_roam_interval;
    /* scan */
    unsigned char uc_random_mac_addr_scan;
    /* capab */
    unsigned char uc_disable_capab_2ght40;
    unsigned int lte_gpio_check_switch;
    unsigned int ism_priority;
    unsigned int lte_rx;
    unsigned int lte_tx;
    unsigned int lte_inact;
    unsigned int ism_rx_act;
    unsigned int bant_pri;
    unsigned int bant_status;
    unsigned int want_pri;
    unsigned int want_status;
} wlan_customize_stru;
extern wlan_customize_stru g_wlan_cust;

/*
 * 不接受cust下发的切换国家码的命令
 * 1、ini文件中国家码被配置成99
 * 2、维测需要:使用hipriv命令修改过国家码
 */
typedef struct {
    oal_bool_enum_uint8 en_country_code_ingore_ini_flag;
    oal_bool_enum_uint8 en_country_code_ingore_hipriv_flag;
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
    oal_bool_enum_uint8 en_country_code_study_ingore_hipriv_flag;
#endif
} wlan_cust_country_code_ingore_flag_stru;
extern wlan_cust_country_code_ingore_flag_stru g_cust_country_code_ignore_flag;

typedef enum {
    CUS_POW_TX_CHAIN_MIMO = 0,
    CUS_POW_TX_CHAIN_SISO_0,
    CUS_POW_TX_CHAIN_SISO_1,

    CUS_POW_TX_CHAIN_BUTT
} cus_pow_chain_sel_no_enum;
/* 1字节对齐 */
#pragma pack(push, 1)
typedef struct {
    uint8_t auc_tb_ru_5g_max_power[CUS_POW_TX_CHAIN_BUTT][WLAN_HE_RU_SIZE_BUTT];
} wlan_cust_5g_rupwr_limit_stru;

struct wlan_cust_nvram {
    int8_t ac_delt_txpwr_params[NUM_OF_NV_MAX_TXPOWER];
    int8_t ac_dpd_delt_txpwr_params[NUM_OF_NV_DPD_MAX_TXPOWER];
    int8_t ac_11b_delt_txpwr_params[NUM_OF_NV_11B_DELTA_TXPOWER];
    uint8_t auc_fem_off_iq_cal_pow_params[NUM_OF_IQ_CAL_POWER];
    uint8_t auc_5g_upper_upc_params[NUM_OF_NV_5G_UPPER_UPC];
    int8_t ac_2g_low_pow_amend_params[NUM_OF_NV_2G_LOW_POW_DELTA_VAL];
    uint8_t auc_2g_txpwr_base_params[WLAN_RF_CHANNEL_NUMS][CUS_BASE_PWR_NUM_2G];
    uint8_t auc_5g_txpwr_base_params[WLAN_RF_CHANNEL_NUMS][CUS_BASE_PWR_NUM_5G];
    /* MIMO下最大发射功率差 */
    int8_t ac_delt_txpwr_base_params[WLAN_RF_CHANNEL_NUMS][WLAN_BAND_BUTT];

    /* FCC/CE边带功率定制项 */
    wlan_cust_cfg_custom_fcc_ce_txpwr_limit_stru ast_fcc_ce_param[WLAN_RF_CHANNEL_NUMS];
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    /* TAS CTRL */
    uint8_t auc_tas_ctrl_params[WLAN_RF_CHANNEL_NUMS][WLAN_BAND_BUTT];
#endif
    /* 5g ce/fcc国家高band的最大发射功率 */
    uint8_t uc_5g_max_pwr_fcc_ce_for_high_band;
    uint8_t auc_5g_iq_cali_lpf_params[NUM_OF_NV_5G_LPF_LVL];
    int8_t  ac_fullbandwidth_to_ru_power_5g[WLAN_BW_CAP_BUTT][WLAN_HE_RU_SIZE_BUTT];
    int8_t  ac_fullbandwidth_to_ru_power_2g[WLAN_BW_CAP_80M][WLAN_HE_RU_SIZE_996];
#if defined(_PRE_WLAN_FEATURE_11AX)
    uint8_t auc_tpc_tb_ru_2g_max_power[CUS_POW_TX_CHAIN_BUTT][WLAN_HE_RU_SIZE_996];
    wlan_cust_5g_rupwr_limit_stru auc_tpc_tb_ru_5g_max_power[MAC_NUM_5G_BAND];
    uint8_t auc_tpc_tb_ru_max_power[WLAN_BAND_BUTT];
#endif
    uint8_t auc_rsv[1];
} __OAL_DECLARE_PACKED;
typedef struct wlan_cust_nvram wlan_cust_nvram_params;
#pragma pack(pop)

typedef struct {
    wlan_cust_nvram_params st_pow_ctrl_custom_param;
    /* SAR CTRL */
    wlan_cust_sar_ctrl_stru st_sar_ctrl_params[CUS_NUM_OF_SAR_LVL][CUS_NUM_OF_SAR_PARAMS];
} wlan_init_cust_nvram_params;

typedef unsigned char countrycode_t[COUNTRY_CODE_LEN];
typedef struct {
    regdomain_enum_uint8 en_regdomain;
    countrycode_t auc_country_code;
} countryinfo_stru;

extern char g_wifi_country_code[COUNTRY_CODE_LEN];
extern oal_bool_enum_uint8 g_en_nv_dp_init_is_null; /* NVRAM中dp init置空标志 */
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
extern oal_bool_enum_uint8 g_tas_switch_en[HAL_HOST_MAX_RF_NUM];
#endif
extern uint8_t g_wlan_open_cnt;
extern oal_bool_enum_uint8 g_wlan_cal_intvl_enable;
extern oal_bool_enum_uint8 g_wlan_cal_disable_switch;

/* 判断当前值是否在合理范围 */
#define cus_val_invalid(val, upper_val, low_val) (((val) > (upper_val)) || ((val) < (low_val)))
#define cus_val_valid(val, upper_val, low_val) (((val) <= (upper_val)) && ((val) >= (low_val)))

/* 取16位数 */
#define cus_get_low_16bits(val)  ((uint16_t)((uint32_t)(val) & 0x0000FFFF))
#define cus_get_high_16bits(val) ((uint16_t)(((uint32_t)(val) & 0xFFFF0000) >> 16))
/* 取8位数 */
#define cus_get_first_byte(val)  (((uint32_t)(val)) & 0xFF)
#define cus_get_second_byte(val) (((uint32_t)(val) >> 8) & 0xFF)
#define cus_get_third_byte(val)  (((uint32_t)(val) >> 16) & 0xFF)
#define cus_get_fourth_byte(val) (((uint32_t)(val) >> 24) & 0xFF)

/* 定制化公共函数 */
char *hwifi_get_country_code(void);
void hwifi_set_country_code(const char *, const unsigned int);
int hwifi_is_regdomain_changed(const countrycode_t, const countrycode_t);
regdomain_enum_uint8 hwifi_get_regdomain_from_country_code(const countrycode_t country_code);
void hwifi_config_init_ini_country(oal_net_device_stru *pst_cfg_net_dev);
void hwifi_config_selfstudy_init_country(oal_net_device_stru *pst_cfg_net_dev);
int hwifi_get_mac_addr(unsigned char *);
int32_t hwifi_read_conf_from(uint8_t *puc_buffer_cust_nvram, uint8_t uc_idx);
uint8_t hwifi_custom_cali_ini_param(uint8_t uc_priv_cfg_value);
oal_netbuf_stru* hwifi_hcc_custom_netbuf_alloc(void);

/* 1103 对外定制化接口 */
uint8_t *hwifi_get_nvram_param(uint32_t nvram_param_idx); /* WTBD:当前只是1103 使用，需要优化 */

#endif  // hisi_customize_wifi.h

