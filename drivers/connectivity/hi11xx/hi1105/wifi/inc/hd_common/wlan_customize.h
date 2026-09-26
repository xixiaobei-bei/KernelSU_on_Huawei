

#ifndef __WLAN_CUSTOMIZE_H__
#define __WLAN_CUSTOMIZE_H__

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "wlan_spec.h"
#include "hal_common_descriptor.h"
#include "hd_event.h"

/* 2 宏定义 */
#define MAC_NUM_2G_BAND    3 /* 2g band数 */
#define MAC_NUM_5G_BAND    7 /* 5g band数 */
#define MAC_NUM_6G_BAND    15 /* 5g band数 */

#define MAC_2G_CHANNEL_NUM 13
#define DEV_WORK_FREQ_LVL_NUM  4
#define CUS_DY_CALI_NUM_5G_BAND    5     /* 动态校准5g band1 2&3 4&5 6 7 */

#define RF_DYN_POW_BAND_NUM  4  /*  band2&3 4&5 6 7总4个  */
#define CUS_5G_BASE_PWR_NUM      5 /* 5g Base power 5个 band1 2&3 4&5 6 7 */
#define CUS_6G_BASE_PWR_NUM      15 /* 6g Base power 15个 band1~15 */
#define CUS_2G_BASE_PWR_NUM      1
#define HAL_CUS_NUM_FCC_CE_2G_PRO       3    /* 定制化2g FCC/CE 11B+OFDM_20M+OFDM_40M */
#define HAL_NUM_5G_20M_SIDE_BAND        6    /* 定制化5g边带数 */
#define HAL_NUM_5G_40M_SIDE_BAND        6
#define HAL_NUM_5G_80M_SIDE_BAND        5
#define HAL_NUM_5G_160M_SIDE_BAND       2
#define HAL_NUM_6G_20M_SIDE_BAND        8    /* 定制化6g边带数 */
#define HAL_NUM_6G_160M_SIDE_BAND       7

#define HAL_CUS_NUM_OF_SAR_PARAMS 8 /* 定制化降SAR参数 5G_BAND1~7 2.4G */
#define HAL_CUS_NUM_OF_6G_SAR_PARAMS 7 /* 6g定制化降SAR参数 */

#define MAC_DPD_CALI_CUS_PARAMS_NUM (7)
#define CUS_NUM_2G_DELTA_RSSI_NUM  2     /* 40M/20M */
#define CUS_NUM_5G_DELTA_RSSI_NUM  4     /* 160M/80M/40M/20M */
#define CUS_NUM_2G_AMEND_RSSI_NUM  3     /* 40M/20M/11B */
#define CUS_NUM_5G_AMEND_RSSI_NUM  4     /* 160M/80M/40M/20M */
#define CUS_2G_FILTER_NARROW_AMEND_RSSI_NUM 2 /* 11b(phymode 40M)/20M(phymode 40M) */
#define CUS_5G_FILTER_NARROW_AMEND_RSSI_NUM 3 /* 真实信号20M下的phymode 40/80/160 */

#define CUS_SAR_LVL_NUM            20 /* 定制化降SAR档位数 */

#define WLAN_2G_BAND_IDX0 0
#define WLAN_2G_BAND_IDX1 1
#define WLAN_2G_BAND_IDX2 2

#define WLAN_5G_BAND_IDX0 0
#define WLAN_5G_BAND_IDX1 1
#define WLAN_5G_BAND_IDX2 2
#define WLAN_5G_BAND_IDX3 3
#define WLAN_5G_BAND_IDX4 4
#define WLAN_5G_BAND_IDX5 5
#define WLAN_5G_BAND_IDX6 6

#define WLAN_6G_BAND_IDX0 0
#define WLAN_6G_BAND_IDX1 1
#define WLAN_6G_BAND_IDX2 2
#define WLAN_6G_BAND_IDX3 3
#define WLAN_6G_BAND_IDX4 4
#define WLAN_6G_BAND_IDX5 5
#define WLAN_6G_BAND_IDX6 6
#define WLAN_6G_BAND_IDX7 7
#define WLAN_6G_BAND_IDX8 8
#define WLAN_6G_BAND_IDX9 9
#define WLAN_6G_BAND_IDX10 10
#define WLAN_6G_BAND_IDX11 11
#define WLAN_6G_BAND_IDX12 12
#define WLAN_6G_BAND_IDX13 13
#define WLAN_6G_BAND_IDX14 14

#define WLAN_2G_DELTA_RSSI_BAND_20M 0
#define WLAN_2G_DELTA_RSSI_BAND_40M 1

#define WLAN_5G_DELTA_RSSI_BAND_20M 0
#define WLAN_5G_DELTA_RSSI_BAND_40M 1
#define WLAN_5G_DELTA_RSSI_BAND_80M 2
#define WLAN_5G_DELTA_RSSI_BAND_160M 3

#define WLAN_2G_AMEND_RSSI_BAND_20M 0
#define WLAN_2G_AMEND_RSSI_BAND_40M 1
#define WLAN_2G_AMEND_RSSI_BAND_11B 2

#define WLAN_5G_AMEND_RSSI_BAND_20M 0
#define WLAN_5G_AMEND_RSSI_BAND_40M 1
#define WLAN_5G_AMEND_RSSI_BAND_80M 2
#define WLAN_5G_AMEND_RSSI_BAND_160M 3

#define WLAN_RF_DYN_POW_BAND_1   0
#define WLAN_RF_DYN_POW_BAND_2_3 1
#define WLAN_RF_DYN_POW_BAND_4_5 2
#define WLAN_RF_DYN_POW_BAND_6   3
#define WLAN_RF_DYN_POW_BAND_7   4

#define WLAN_2G_CHN_IDX0 0
#define WLAN_2G_CHN_IDX1 1
#define WLAN_2G_CHN_IDX2 2
#define WLAN_2G_CHN_IDX3 3
#define WLAN_2G_CHN_IDX4 4
#define WLAN_2G_CHN_IDX5 5
#define WLAN_2G_CHN_IDX6 6
#define WLAN_2G_CHN_IDX7 7
#define WLAN_2G_CHN_IDX8 8
#define WLAN_2G_CHN_IDX9 9
#define WLAN_2G_CHN_IDX10 10
#define WLAN_2G_CHN_IDX11 11
#define WLAN_2G_CHN_IDX12 12

#define CUS_DPD_CALI_PARAMS0 0
#define CUS_DPD_CALI_PARAMS1 1
#define CUS_DPD_CALI_PARAMS2 2
#define CUS_DPD_CALI_PARAMS3 3
#define CUS_DPD_CALI_PARAMS4 4
#define CUS_DPD_CALI_PARAMS5 5
#define CUS_DPD_CALI_PARAMS6 6

#define DPD_CHN1_CHN8_NUM 8
#define DPD_CHN9_CHN13_NUM 5

#define DEV_WORK_FREQ_LVL0 0
#define DEV_WORK_FREQ_LVL1 1
#define DEV_WORK_FREQ_LVL2 2
#define DEV_WORK_FREQ_LVL3 3

#define CUS_NUM_FCC_CE_2G_PRO           3    /* 定制化2g FCC/CE 11B+OFDM_20M+OFDM_40M */
#define CUS_NUM_5G_20M_SIDE_BAND        6    /* 定制化5g边带数 */
#define CUS_NUM_5G_40M_SIDE_BAND        6
#define CUS_NUM_5G_80M_SIDE_BAND        5
#define CUS_NUM_5G_160M_SIDE_BAND       2

#define CUS_GET_BOOL(_val)         ((!(_val)) ? OAL_FALSE : OAL_TRUE)
#define CUS_SET_BOOL(_val)         ((_val) = (!!(_val)))

#define CUS_CHECK_VAL(_val, _max, _min, _default)  \
    ((_val) = ((_val) <= (_max)  && (_val) >=(_min)) ? (_val) : (_default))

#define CUS_CHECK_MIN_VAL(_val, _min, _default) ((_val) = ((_val) >=(_min)) ? (_val) : (_default))
#define CUS_CHECK_MAX_VAL(_val, _max, _default) ((_val) = ((_val) <=(_max)) ? (_val) : (_default))

/* 3 枚举定义 */
/* 枚举变量个数是4的倍数:保持4字节对齐 */
typedef enum {
    WLAN_CFGID_SET_CUS_RF_CFG = 0, /* 前端rf相关定制化 */
    WLAN_CFGID_SET_CUS_RF_CALI = 1, /* rf校准相关定制化 */
    WLAN_CFGID_SET_CUS_CAP = 2,      /* 特性功能能力定制化 */
    WLAN_CFGID_SET_CUS_POW = 3,             /* 功率定制化 */
    WLAN_CFGID_SET_CUS_DYN_POW_CALI = 4,       /* 动态功率校准定制化 */
    WLAN_CFGID_SET_RF_CALI_RUN = 5,     /* 前端rf校准执行 */

    WLAN_CFGID_SET_CUS_BUTT
} cus_cust_cfgid_enum;

typedef enum {
    HAL_POW_RF_SEL_CHAIN_1 = 0, /* SISO power */
    HAL_POW_RF_SEL_CHAIN_2,     /* 2X2 MIMO power */
    HAL_POW_RF_SEL_CHAIN_3,     /* 3X3 MIMO power */
    HAL_POW_RF_SEL_CHAIN_4,     /* 4X4 MIMO power */
    HAL_POW_RF_SEL_CHAIN_BUTT
} hal_pow_rf_chain_sel_enum;

typedef enum {
    WLAN_CUST_NUM_1 = 1,
    WLAN_CUST_NUM_2,
    WLAN_CUST_NUM_3,
    WLAN_CUST_NUM_4,
} wlan_cfg_num_enum;

typedef enum {
    DYN_PWR_CUST_SNGL_MODE_11B = 0,
    DYN_PWR_CUST_SNGL_MODE_OFDM20,
    DYN_PWR_CUST_SNGL_MODE_OFDM40,
    DYN_PWR_CUST_2G_SNGL_MODE_BUTT,
} cust_dyn_2g_pwr_sngl_mode_enum;

typedef enum {
    /*  nvram_params0  */
    CUST_DELTA_POW_2P4G_11B_1M_2M,
    CUST_DELTA_POW_2P4G_11B_5M_11M,
    CUST_DELTA_POW_2P4G_11G_6M_9M,
    CUST_DELTA_POW_2P4G_11G_12M_18M,
    /*  nvram_params1  */
    CUST_DELTA_POW_2P4G_11G_24M_36M,
    CUST_DELTA_POW_2P4G_11G_48M,
    CUST_DELTA_POW_2P4G_11G_54M,
    CUST_DELTA_POW_2P4G_HT20_VHT20_MCS0,
    /*  nvram_params2  */
    CUST_DELTA_POW_2P4G_HT20_VHT20_MCS2_3,
    CUST_DELTA_POW_2P4G_HT20_VHT20_MCS4_5,
    CUST_DELTA_POW_2P4G_HT20_VHT20_MCS6,
    CUST_DELTA_POW_2P4G_HT20_VHT20_MCS7,
    /*  nvram_params3  */
    CUST_DELTA_POW_2P4G_VHT20_MCS8,
    CUST_DELTA_POW_2P4G_HT40_VHT40_MCS0_1,
    CUST_DELTA_POW_2P4G_HT40_VHT40_MCS2_3,
    CUST_DELTA_POW_2P4G_HT40_VHT40_MCS4_5,
    /*  nvram_params4  */
    CUST_DELTA_POW_2P4G_HT40_VHT40_MCS6,
    CUST_DELTA_POW_2P4G_HT40_VHT40_MCS7,
    CUST_DELTA_POW_2P4G_VHT40_MCS8,
    CUST_DELTA_POW_2P4G_VHT40_MCS9,
    /*  nvram_params5  */
    CUST_DELTA_POW_2P4G_HT40_MCS32,
    CUST_DELTA_POW_5G_11A_6M_9M,
    CUST_DELTA_POW_5G_11A_12M_18M,
    CUST_DELTA_POW_5G_11A_24M_36M,
    /*  nvram_params6  */
    CUST_DELTA_POW_5G_11A_48M,
    CUST_DELTA_POW_5G_11A_54M,
    CUST_DELTA_POW_5G_HT20_VHT20_MCS0_1,
    CUST_DELTA_POW_5G_HT20_VHT20_MCS2_3,
    /*  nvram_params7  */
    CUST_DELTA_POW_5G_HT20_VHT20_MCS4_5,
    CUST_DELTA_POW_5G_HT20_VHT20_MCS6,
    CUST_DELTA_POW_5G_HT20_VHT20_MCS7,
    CUST_DELTA_POW_5G_VHT20_MCS8,
    /*  nvram_params8  */
    CUST_DELTA_POW_5G_HT40_VHT40_MCS0_1,
    CUST_DELTA_POW_5G_HT40_VHT40_MCS2_3,
    CUST_DELTA_POW_5G_HT40_VHT40_MCS4_5,
    CUST_DELTA_POW_5G_HT40_VHT40_MCS6,
    /*  nvram_params9  */
    CUST_DELTA_POW_5G_HT40_VHT40_MCS7,
    CUST_DELTA_POW_5G_VHT40_MCS8,
    CUST_DELTA_POW_5G_HT40_MCS32,
    CUST_DELTA_POW_5G_VHT80_MCS0_1,
    /*  nvram_params10  */
    CUST_DELTA_POW_5G_VHT80_MCS2_3,
    CUST_DELTA_POW_5G_VHT80_MCS4_5,
    CUST_DELTA_POW_5G_VHT80_MCS6,
    CUST_DELTA_POW_5G_VHT80_MCS7,
    /*  nvram_params11  */
    CUST_DELTA_POW_5G_VHT80_MCS8,
    CUST_DELTA_POW_2P4G_VHT20_MCS9,
    CUST_DELTA_POW_2P4G_VHT20_MCS10,
    CUST_DELTA_POW_2P4G_VHT20_MCS11,
    /*  nvram_params12  */
    CUST_DELTA_POW_2P4G_VHT40_MCS10,
    CUST_DELTA_POW_2P4G_VHT40_MCS11,
    CUST_DELTA_POW_5G_VHT20_MCS9,
    CUST_DELTA_POW_5G_VHT20_MCS10,
    /*  nvram_params13  */
    CUST_DELTA_POW_5G_VHT20_MCS11,
    CUST_DELTA_POW_5G_VHT40_MCS10,
    CUST_DELTA_POW_5G_VHT40_MCS11,
    CUST_DELTA_POW_5G_VHT80_MCS10,
    /*  nvram_params14  */
    CUST_DELTA_POW_5G_VHT80_MCS11,
    CUST_DELTA_POW_5G_VHT160_MCS0,
    CUST_DELTA_POW_5G_VHT160_MCS1,
    CUST_DELTA_POW_5G_VHT160_MCS2,
    /*  nvram_params15  */
    CUST_DELTA_POW_5G_VHT160_MCS3,
    CUST_DELTA_POW_5G_VHT160_MCS4,
    CUST_DELTA_POW_5G_VHT160_MCS5,
    CUST_DELTA_POW_5G_VHT160_MCS6,
    /*  nvram_params16  */
    CUST_DELTA_POW_5G_VHT160_MCS7,
    CUST_DELTA_POW_5G_VHT160_MCS8,
    CUST_DELTA_POW_5G_VHT160_MCS9,
    CUST_DELTA_POW_5G_VHT160_MCS10,
    /*  nvram_params17  */
    CUST_DELTA_POW_5G_VHT160_MCS11,
    CUST_DELTA_POW_5G_VHT40_MCS9,
    CUST_DELTA_POW_5G_VHT80_MCS9,
    CUST_DELTA_POW_RSV1,
    CUST_NORMAL_DELTA_POW_RATE_BUTT = CUST_DELTA_POW_RSV1,
    /*  4字节对齐填充后BUTT  */
    CUST_NORMAL_DELTA_POW_RATE_ALGN_BUTT,
} wlan_cust_delta_pow_enum;
typedef enum {
    CUST_NRCOEX_RULE_0,
    CUST_NRCOEX_RULE_1,
    CUST_NRCOEX_RULE_2,
    CUST_NRCOEX_RULE_3,
    CUST_NRCOEX_RULE_BUTT
} wlan_cust_nrcoex_rule_index_enum;

typedef enum {
    WLAN_BUS_SDIO, /* SDIO总线类型 */
    WLAN_BUS_PCIE, /* PCIE总线类型 */

    WLAN_BUS_TYPE_BUTT,
} wlan_bus_type_enum;
typedef uint8_t wlan_bus_type_enum_uint8;

typedef struct {
    uint16_t dyn_cali_dscr_interval[WLAN_CALI_BAND_BUTT]; /* 动态校准开关2.4g 5g 6g */
#ifndef _PRE_WLAN_FEATURE_6G_EXTEND
    uint16_t reserv;
#endif

    uint8_t band_5g_enable;
    uint8_t resv[3];  /* 保留字段:3字节 */
} mac_cus_rf_cali_stru;

typedef struct {
    uint32_t over_temper_protect_th;

    uint8_t en_over_temp_pro;
    uint8_t en_reduce_pwr;
    uint8_t over_temp_pro_safe_th;
    uint8_t over_temp_pro_over_th;

    uint8_t over_temp_pro_pa_off_th;
    uint8_t rsv[3];    /* 保留字段:3字节 */
}over_temper_stru;

typedef struct {
    uint32_t speed_level;    /* 吞吐量门限 */
    uint8_t cpu_freq_type; /* device主频类型 */
    uint8_t uc_rsv[3];    /* 保留字段:3字节 */
} dev_work_lvl_stru;

typedef struct {
    uint8_t wlan_min_fast_ps_idle;
    uint8_t wlan_max_fast_ps_idle;
    uint8_t wlan_auto_ps_thresh_screen_on;
    uint8_t wlan_auto_ps_thresh_screen_off;
} cust_fast_pow_save_stru;
/* 校准所需要使用的定制化 */
typedef struct {
    uint32_t cali_mask[WLAN_CALI_BAND_BUTT]; /* offline校准开关, 2G/5G */
    uint32_t online_cali_mask; /* online校准开关 */
} wlan_cali_ini_custom_stru;

typedef struct {
    uint32_t freq;  /* 高16bit为频率的上限, 低16bit为频率的下限 */
    uint32_t relative_freq_gap0_40_20; /* wifi中心频点到最近modem边沿的第0档带宽高16bit为40M带宽低16bit为20M带宽 */
    uint32_t relative_freq_gap0_160_80; /* wifi中心频点到最近modem边沿的第0档带宽高16bit为160M带宽低16bit为80M带宽 */
    uint32_t relative_freq_gap1_40_20; /* wifi中心频点到最近modem边沿的第1档带宽高16bit为40M带宽低16bit为20M带宽 */
    uint32_t relative_freq_gap1_160_80; /* wifi中心频点到最近modem边沿的第1档带宽高16bit为160M带宽低16bit为80M带宽 */
    uint32_t relative_freq_gap2_40_20; /* wifi中心频点到最近modem边沿的第2档带宽高16bit为40M带宽低16bit为20M带宽 */
    uint32_t relative_freq_gap2_160_80; /* wifi中心频点到最近modem边沿的第2档带宽高16bit为160M带宽低16bit为80M带宽 */
    uint32_t limit_power_level0;  /* 第0档带宽范围内采用此档功率限制值160M/80M/40M/20M分别占8bit */
    uint32_t limit_power_level1;  /* 第1档带宽范围内采用此档功率限制值160M/80M/40M/20M分别占8bit */
    uint32_t limit_power_level2;  /* 第2档带宽范围内采用此档功率限制值160M/80M/40M/20M分别占8bit */
    uint32_t rssi_threshold;  /* rssi的门限值160M/80M/40M/20M分别占8bit,历史问题这里记录的是rssi门限的绝对值 */
} wlan_nrcoex_rule_stru;

typedef struct {
    oal_bool_enum_uint8 nrcoex_switch; /* NRCOEX定制化开关 */
    uint8_t version_num;
    oal_bool_enum_uint8 nrcoex_hw;
    oal_bool_enum_uint8 advance_low_power;
    wlan_nrcoex_rule_stru nrcoex_rule[CUST_NRCOEX_RULE_BUTT]; /* 不同频率范围下规则不同所以需要多组规则 */
} wlan_nrcoex_params_stru;

typedef struct {
    uint8_t uc_fem_delt_pow;
    uint8_t uc_tpc_adj_pow_start_idx;
    uint8_t auc_resv[NUM_2_BYTES];
} dmac_ax_fem_pow_ctl_stru;

typedef struct {
    dev_work_lvl_stru dev_frequency[DEV_WORK_FREQ_LVL_NUM];   /*  device工作频率  */
    cust_fast_pow_save_stru fast_ps;
    dmac_ax_fem_pow_ctl_stru fem_pow;
    uint32_t cali_policy;
    wlan_cali_ini_custom_stru cali_switch;
    uint32_t voe_switch_mask;
    uint32_t wifi_11ax_switch_mask;
    uint32_t mult_bssid_switch_mask;
    uint32_t htc_switch_mask;
    uint32_t hiex_cap;
    uint32_t trx_switch;
    uint32_t ps_rssi_param;
    uint32_t ps_pps_param;
    uint32_t ftm_board_delay;
    uint32_t hisi_priv_cap;
    uint64_t cali_data_h_phy_addr;  /* host侧校准数据保存地址 */
    over_temper_stru over_temper_protect;

    wlan_bw_cap_enum_uint8 en_channel_width;
    oal_bool_enum_uint8 en_ldpc_is_supp;     /* 是否支持接收LDPC编码的包 */
    oal_bool_enum_uint8 en_rx_stbc_is_supp;  /* 是否支持stbc接收,支持2个空间流 */
    oal_bool_enum_uint8 en_tx_stbc_is_supp;  /* 是否支持最少2x1 STBC发送 */

    oal_bool_enum_uint8 en_su_bfmer_is_supp; /* 是否支持单用户beamformer */
    oal_bool_enum_uint8 en_su_bfmee_is_supp; /* 是否支持单用户beamformee */
    oal_bool_enum_uint8 en_mu_bfmer_is_supp; /* 是否支持多用户beamformer */
    oal_bool_enum_uint8 en_mu_bfmee_is_supp; /* 是否支持多用户beamformee */

    uint8_t radio_cap[WLAN_DEVICE_MAX_NUM_PER_CHIP];
    uint8_t mcm_func_mask;
    uint8_t rssi_switch_ant;

    uint8_t degrade_cap;
    uint8_t chn_radio_cap;
    uint8_t chn_radio_cap_6g;
    uint8_t mcm_custom_func_mask;

    uint8_t country_self_study;  /* 支持国家码自学习功能 */
    oal_bool_enum_uint8 en_1024qam_is_supp; /* 支持1024QAM速率 */
    uint8_t res[2];

    uint16_t used_mem_for_start;
    uint16_t used_mem_for_stop;

    uint8_t sdio_assem_d2h;
    uint8_t sdio_assem_h2d;
    uint8_t linkloss_threshold_fixed;
    uint8_t aput_160m_switch;

    int16_t dsss2ofdm_dbb_pwr_bo_val;
    uint16_t download_rate_limit_pps;

    uint8_t ac_suspend_mask;
    uint8_t dyn_extlna_bypass;
    int8_t frame_tx_chain_ctl;
    uint8_t double_chain_11b_bo_pow;

    uint8_t hcc_flowctrl_type;
    uint8_t hcc_flowctrl_switch;
    uint8_t autocali_switch;
    uint8_t txopps_switch;

    uint8_t phy_cap_mask;
    uint8_t wlan_device_pm_switch;
    uint8_t fast_mode;
    uint8_t optimized_feature_mask;

    uint8_t dc_flowctl_switch;
    uint8_t mbo_switch;
    uint8_t fastscan_switch;

    uint8_t linkloss_th[WLAN_LINKLOSS_MODE_BUTT];
    uint8_t ftm_cap;

    uint8_t btcoex_threshold_mcm[WLAN_BTCOEX_RSSI_MCM_BUTT];
    uint8_t radar_isr_forbid;
    uint8_t dbac_dynamic_switch;

    uint8_t evm_pll_reg_fix;
    uint8_t cali_data_mask;
    uint8_t loglevel;
    uint8_t dbdc_cap; /* 表示板级方案支持的特殊DBDC类型 */
    wlan_nrcoex_params_stru nrcoex_params;
}mac_cust_feature_cap_sru;

typedef struct {
    int8_t dpn_2g[DYN_PWR_CUST_2G_SNGL_MODE_BUTT][MAC_2G_CHANNEL_NUM];
    int8_t rsv;
    int8_t dpn_5g[WLAN_BW_CAP_80PLUS80][MAC_NUM_5G_BAND];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    int8_t dpn_6g[WLAN_BW_CAP_80PLUS80][MAC_NUM_6G_BAND];
#endif
} cus_dy_cali_dpn_stru;

/* FCC/CE边带功率定制项 */
typedef struct {
    uint8_t fcc_txpwr_limit_5g_20m[HAL_NUM_5G_20M_SIDE_BAND];
    uint8_t fcc_txpwr_limit_5g_40m[HAL_NUM_5G_40M_SIDE_BAND];
    uint8_t fcc_txpwr_limit_5g_80m[HAL_NUM_5G_80M_SIDE_BAND];
    uint8_t fcc_txpwr_limit_5g_160m[HAL_NUM_5G_160M_SIDE_BAND];
    uint8_t rsv_5g;

    uint8_t fcc_txpwr_limit_2g[WLAN_2G_SUB_BAND_NUM][HAL_CUS_NUM_FCC_CE_2G_PRO];
    uint8_t rsv_2g;

#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    uint8_t fcc_txpwr_limit_6g_20m[HAL_NUM_6G_20M_SIDE_BAND];  /* 边带fcc功率参数 */
    uint8_t fcc_txpwr_limit_6g_40m[HAL_NUM_6G_20M_SIDE_BAND];
    uint8_t fcc_txpwr_limit_6g_80m[HAL_NUM_6G_20M_SIDE_BAND];
    uint8_t fcc_sub_txpwr_limit_6g_20m[HAL_NUM_6G_20M_SIDE_BAND]; /* 次边带fcc功率参数 */
    uint8_t fcc_sub_txpwr_limit_6g_40m[HAL_NUM_6G_20M_SIDE_BAND];
    uint8_t fcc_sub_txpwr_limit_6g_80m[HAL_NUM_6G_20M_SIDE_BAND];

    uint8_t fcc_txpwr_limit_6g_160m[HAL_NUM_6G_160M_SIDE_BAND];
    uint8_t rsv_6g;
#endif
} hal_cfg_custom_fcc_txpwr_limit_stru;

typedef struct {
    uint8_t sar_ctrl_params[HD_EVENT_RF_NUM][HAL_CUS_NUM_OF_SAR_PARAMS];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    uint8_t sar_ctrl_6g_params[HD_EVENT_RF_NUM][HAL_CUS_NUM_OF_6G_SAR_PARAMS];
#endif
} wlan_cust_sar_cfg_stru;

typedef struct {
    uint8_t ru_pwr_2g[WLAN_HE_RU_SIZE_996];
    int8_t  rsv[3]; /* 保留字段:3字节 */
} wlan_cust_2g_ru_pow_stru;

typedef struct {
    uint8_t ru_pwr_5g[WLAN_HE_RU_SIZE_BUTT];
    int8_t  rsv;
} wlan_cust_5g_ru_pow_stru;
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
typedef struct {
    uint8_t ru_pwr_6g[WLAN_HE_RU_SIZE_BUTT];
    int8_t  rsv;
} wlan_cust_6g_ru_pow_stru;
#endif
typedef struct {
    uint8_t ru_pwr_2g[WLAN_HE_RU_SIZE_996];
    int8_t  rsv[3]; /* 保留字段:3字节 */
} wlan_cust_2g_tpc_ru_pow_stru;

typedef struct {
    uint8_t ru_pwr_5g[WLAN_HE_RU_SIZE_BUTT];
    int8_t  rsv;
} wlan_cust_5g_tpc_ru_pow_stru;
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
typedef struct {
    uint8_t ru_pwr_6g[WLAN_HE_RU_SIZE_BUTT];
    int8_t  rsv;
} wlan_cust_6g_tpc_ru_pow_stru;
#endif

typedef union {
    /* 与定制化一一对应，用union可避免组帧时移位等位操作 */
    struct {
        uint32_t bit_1024qam           : 1,  /* Support 1024QAM */
                 bit_4096qam           : 1,  /* Support 4096QAM */
                 bit_sgi_400ns         : 1,  /* Support 11ax 0.4SGI */
                 bit_vht_3nss_80m_mcs6 : 1,  /* Support 11ac 3nss 80M mcs6 */
                 bit_resv : 28;
    } bits;
    uint32_t hisi_priv_value;
} mac_hisi_priv_cap_union;

#pragma pack(push, 1)
/* FCC/CE边带功率定制项:4字节对齐 */
typedef struct {
    uint8_t auc_5g_fcc_txpwr_limit_params_20m[CUS_NUM_5G_20M_SIDE_BAND];
    uint8_t auc_5g_fcc_txpwr_limit_params_40m[CUS_NUM_5G_40M_SIDE_BAND];
    uint8_t auc_5g_fcc_txpwr_limit_params_80m[CUS_NUM_5G_80M_SIDE_BAND];
    uint8_t auc_5g_fcc_txpwr_limit_params_160m[CUS_NUM_5G_160M_SIDE_BAND];
    uint8_t auc_2g_fcc_txpwr_limit_params[MAC_2G_CHANNEL_NUM][CUS_NUM_FCC_CE_2G_PRO];
    uint8_t rsv[2]; /* 保留字段:2字节 */
}wlan_cust_cfg_custom_fcc_ce_txpwr_limit_stru;
#pragma pack(pop)

#endif /* end of wlan_customize.h */
