

/* 头文件包含 */
#include "hisi_customize_wifi_hi1103.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/etherdevice.h>
#endif

#include "oal_hcc_host_if.h"
#include "oal_main.h"
#include "hisi_ini.h"
#include "plat_cali.h"
#include "plat_pm_wlan.h"
#include "plat_firmware.h"
#include "oam_ext_if.h"

#include "wlan_spec.h"
#include "wlan_chip_i.h"
#include "hisi_customize_wifi.h"

#include "mac_hiex.h"

#include "wal_config.h"
#include "wal_linux_ioctl.h"

#include "hmac_auto_adjust_freq.h"
#include "hmac_tx_data.h"
#include "hmac_cali_mgmt.h"

/* 终端头文件 */
#ifdef _PRE_CONFIG_READ_DYNCALI_E2PROM
#ifdef CONFIG_ARCH_PLATFORM
#define nve_direct_access_interface(...)  0
#else
#define hisi_nve_direct_access(...)  0
#endif
#else
#ifdef CONFIG_ARCH_PLATFORM
#include <linux/mtd/nve_ap_kernel_interface.h>
#else
#include <linux/mtd/hisi_nve_interface.h>
#endif
#endif


#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HISI_CUSTOMIZE_WIFI_HI1103_C

/* 全局变量定义 */
OAL_STATIC int32_t g_host_init_params[WLAN_CFG_INIT_BUTT] = {0}; /* ini定制化参数数组 */
OAL_STATIC int32_t g_dts_params[WLAN_CFG_DTS_BUTT] = {0};        /* dts定制化参数数组 */

OAL_STATIC uint8_t g_wifi_mac[WLAN_MAC_ADDR_LEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
OAL_STATIC int32_t g_nvram_init_params[NVRAM_PARAMS_PWR_INDEX_BUTT] = {0};         /* ini文件中NV参数数组 */
OAL_STATIC wlan_customize_private_stru g_priv_cust_params[WLAN_CFG_PRIV_BUTT] = {{ 0, 0 }}; /* 私有定制化参数数组 */

OAL_STATIC wlan_init_cust_nvram_params g_cust_nv_params = {{{0}}}; /* 最大发送功率定制化数组 */

/* 产测定制化参数数组 */
wlan_customize_pwr_fit_para_stru g_pro_line_params[WLAN_RF_CHANNEL_NUMS][DY_CALI_PARAMS_NUM] = {{{0}}};
OAL_STATIC uint8_t g_cust_nvram_info[WLAN_CFG_DTS_NVRAM_END][CUS_PARAMS_LEN_MAX] = {{0}}; /* NVRAM数组 */

static int16_t g_gs_extre_point_vals[WLAN_RF_CHANNEL_NUMS][DY_CALI_NUM_5G_BAND] = {{0}};

oal_bool_enum_uint8 g_en_fact_cali_completed = OAL_FALSE;

OAL_STATIC wlan_cfg_cmd g_wifi_config_dts[] = {
    /* 校准 */
    { "cali_txpwr_pa_dc_ref_2g_val_chan1",  WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN1 },
    { "cali_txpwr_pa_dc_ref_2g_val_chan2",  WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN2 },
    { "cali_txpwr_pa_dc_ref_2g_val_chan3",  WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN3 },
    { "cali_txpwr_pa_dc_ref_2g_val_chan4",  WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN4 },
    { "cali_txpwr_pa_dc_ref_2g_val_chan5",  WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN5 },
    { "cali_txpwr_pa_dc_ref_2g_val_chan6",  WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN6 },
    { "cali_txpwr_pa_dc_ref_2g_val_chan7",  WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN7 },
    { "cali_txpwr_pa_dc_ref_2g_val_chan8",  WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN8 },
    { "cali_txpwr_pa_dc_ref_2g_val_chan9",  WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN9 },
    { "cali_txpwr_pa_dc_ref_2g_val_chan10", WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN10 },
    { "cali_txpwr_pa_dc_ref_2g_val_chan11", WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN11 },
    { "cali_txpwr_pa_dc_ref_2g_val_chan12", WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN12 },
    { "cali_txpwr_pa_dc_ref_2g_val_chan13", WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN13 },

    { "cali_txpwr_pa_dc_ref_5g_val_band1", WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND1 },
    { "cali_txpwr_pa_dc_ref_5g_val_band2", WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND2 },
    { "cali_txpwr_pa_dc_ref_5g_val_band3", WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND3 },
    { "cali_txpwr_pa_dc_ref_5g_val_band4", WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND4 },
    { "cali_txpwr_pa_dc_ref_5g_val_band5", WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND5 },
    { "cali_txpwr_pa_dc_ref_5g_val_band6", WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND6 },
    { "cali_txpwr_pa_dc_ref_5g_val_band7", WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND7 },
    { "cali_tone_amp_grade",               WLAN_CFG_DTS_CALI_TONE_AMP_GRADE },
    /* DPD校准定制化 */
    { "dpd_cali_ch_core0",          WLAN_CFG_DTS_DPD_CALI_CH_CORE0 },
    { "dpd_use_cail_ch_idx0_core0", WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX0_CORE0 },
    { "dpd_use_cail_ch_idx1_core0", WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX1_CORE0 },
    { "dpd_use_cail_ch_idx2_core0", WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX2_CORE0 },
    { "dpd_use_cail_ch_idx3_core0", WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX3_CORE0 },
    { "dpd_cali_20m_del_pow_core0", WLAN_CFG_DTS_DPD_CALI_20M_DEL_POW_CORE0 },
    { "dpd_cali_40m_del_pow_core0", WLAN_CFG_DTS_DPD_CALI_40M_DEL_POW_CORE0 },
    { "dpd_cali_ch_core1",          WLAN_CFG_DTS_DPD_CALI_CH_CORE1 },
    { "dpd_use_cail_ch_idx0_core1", WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX0_CORE1 },
    { "dpd_use_cail_ch_idx1_core1", WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX1_CORE1 },
    { "dpd_use_cail_ch_idx2_core1", WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX2_CORE1 },
    { "dpd_use_cail_ch_idx3_core1", WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX3_CORE1 },
    { "dpd_cali_20m_del_pow_core1", WLAN_CFG_DTS_DPD_CALI_20M_DEL_POW_CORE1 },
    { "dpd_cali_40m_del_pow_core1", WLAN_CFG_DTS_DPD_CALI_40M_DEL_POW_CORE1 },
    /* 动态校准 */
    { "dyn_cali_dscr_interval", WLAN_CFG_DTS_DYN_CALI_DSCR_ITERVL },
    { "dyn_cali_opt_switch",    WLAN_CFG_DTS_DYN_CALI_OPT_SWITCH },
    { "gm0_dB10_amend",         WLAN_CFG_DTS_DYN_CALI_GM0_DB10_AMEND },
    /* DPN 40M 20M 11b */
    { "dpn24g_ch1_core0",  WLAN_CFG_DTS_2G_CORE0_DPN_CH1 },
    { "dpn24g_ch2_core0",  WLAN_CFG_DTS_2G_CORE0_DPN_CH2 },
    { "dpn24g_ch3_core0",  WLAN_CFG_DTS_2G_CORE0_DPN_CH3 },
    { "dpn24g_ch4_core0",  WLAN_CFG_DTS_2G_CORE0_DPN_CH4 },
    { "dpn24g_ch5_core0",  WLAN_CFG_DTS_2G_CORE0_DPN_CH5 },
    { "dpn24g_ch6_core0",  WLAN_CFG_DTS_2G_CORE0_DPN_CH6 },
    { "dpn24g_ch7_core0",  WLAN_CFG_DTS_2G_CORE0_DPN_CH7 },
    { "dpn24g_ch8_core0",  WLAN_CFG_DTS_2G_CORE0_DPN_CH8 },
    { "dpn24g_ch9_core0",  WLAN_CFG_DTS_2G_CORE0_DPN_CH9 },
    { "dpn24g_ch10_core0", WLAN_CFG_DTS_2G_CORE0_DPN_CH10 },
    { "dpn24g_ch11_core0", WLAN_CFG_DTS_2G_CORE0_DPN_CH11 },
    { "dpn24g_ch12_core0", WLAN_CFG_DTS_2G_CORE0_DPN_CH12 },
    { "dpn24g_ch13_core0", WLAN_CFG_DTS_2G_CORE0_DPN_CH13 },
    { "dpn5g_core0_b0",    WLAN_CFG_DTS_5G_CORE0_DPN_B0 },
    { "dpn5g_core0_b1",    WLAN_CFG_DTS_5G_CORE0_DPN_B1 },
    { "dpn5g_core0_b2",    WLAN_CFG_DTS_5G_CORE0_DPN_B2 },
    { "dpn5g_core0_b3",    WLAN_CFG_DTS_5G_CORE0_DPN_B3 },
    { "dpn5g_core0_b4",    WLAN_CFG_DTS_5G_CORE0_DPN_B4 },
    { "dpn5g_core0_b5",    WLAN_CFG_DTS_5G_CORE0_DPN_B5 },
    { "dpn5g_core0_b6",    WLAN_CFG_DTS_5G_CORE0_DPN_B6 },
    { "dpn24g_ch1_core1",  WLAN_CFG_DTS_2G_CORE1_DPN_CH1 },
    { "dpn24g_ch2_core1",  WLAN_CFG_DTS_2G_CORE1_DPN_CH2 },
    { "dpn24g_ch3_core1",  WLAN_CFG_DTS_2G_CORE1_DPN_CH3 },
    { "dpn24g_ch4_core1",  WLAN_CFG_DTS_2G_CORE1_DPN_CH4 },
    { "dpn24g_ch5_core1",  WLAN_CFG_DTS_2G_CORE1_DPN_CH5 },
    { "dpn24g_ch6_core1",  WLAN_CFG_DTS_2G_CORE1_DPN_CH6 },
    { "dpn24g_ch7_core1",  WLAN_CFG_DTS_2G_CORE1_DPN_CH7 },
    { "dpn24g_ch8_core1",  WLAN_CFG_DTS_2G_CORE1_DPN_CH8 },
    { "dpn24g_ch9_core1",  WLAN_CFG_DTS_2G_CORE1_DPN_CH9 },
    { "dpn24g_ch10_core1", WLAN_CFG_DTS_2G_CORE1_DPN_CH10 },
    { "dpn24g_ch11_core1", WLAN_CFG_DTS_2G_CORE1_DPN_CH11 },
    { "dpn24g_ch12_core1", WLAN_CFG_DTS_2G_CORE1_DPN_CH12 },
    { "dpn24g_ch13_core1", WLAN_CFG_DTS_2G_CORE1_DPN_CH13 },
    { "dpn5g_core1_b0",    WLAN_CFG_DTS_5G_CORE1_DPN_B0 },
    { "dpn5g_core1_b1",    WLAN_CFG_DTS_5G_CORE1_DPN_B1 },
    { "dpn5g_core1_b2",    WLAN_CFG_DTS_5G_CORE1_DPN_B2 },
    { "dpn5g_core1_b3",    WLAN_CFG_DTS_5G_CORE1_DPN_B3 },
    { "dpn5g_core1_b4",    WLAN_CFG_DTS_5G_CORE1_DPN_B4 },
    { "dpn5g_core1_b5",    WLAN_CFG_DTS_5G_CORE1_DPN_B5 },
    { "dpn5g_core1_b6",    WLAN_CFG_DTS_5G_CORE1_DPN_B6 },
    { NULL, 0 }
};

OAL_STATIC wlan_cfg_cmd g_wifi_config_priv[] = {
    /* 校准开关 */
    { "cali_mask", WLAN_CFG_PRIV_CALI_MASK },
    /*
     * #bit0:开wifi重新校准 bit1:开wifi重新上传 bit2:开机校准 bit3:动态校准调平Debug
     * #bit4:不读取NV区域的数据(1:不读取 0：读取)
     */
    { "cali_data_mask", WLAN_CFG_PRIV_CALI_DATA_MASK },
    { "cali_auto_cali_mask", WLAN_CFG_PRIV_CALI_AUTOCALI_MASK },
    { "bw_max_width", WLAN_CFG_PRIV_BW_MAX_WITH },
    { "ldpc_coding",  WLAN_CFG_PRIV_LDPC_CODING },
    { "rx_stbc",      WLAN_CFG_PRIV_RX_STBC },
    { "tx_stbc",      WLAN_CFG_PRIV_TX_STBC },
    { "su_bfer",      WLAN_CFG_PRIV_SU_BFER },
    { "su_bfee",      WLAN_CFG_PRIV_SU_BFEE },
    { "mu_bfer",      WLAN_CFG_PRIV_MU_BFER },
    { "mu_bfee",      WLAN_CFG_PRIV_MU_BFEE },

    { "11n_txbf", WLAN_CFG_PRIV_11N_TXBF },

    { "1024qam_en", WLAN_CFG_PRIV_1024_QAM },
    /* DBDC */
    { "radio_cap_0",     WLAN_CFG_PRIV_DBDC_RADIO_0 },
    { "radio_cap_1",     WLAN_CFG_PRIV_DBDC_RADIO_1 },
    { "fastscan_switch", WLAN_CFG_PRIV_FASTSCAN_SWITCH },

    /* RSSI天线切换 */
    { "rssi_ant_switch", WLAN_CFG_ANT_SWITCH },

     /* 国家码自学习功能开关 */
    { "countrycode_selfstudy", WLAN_CFG_PRIV_COUNRTYCODE_SELFSTUDY_CFG},

    { "m2s_function_mask_ext", WLAN_CFG_PRIV_M2S_FUNCTION_EXT_MASK},
    { "m2s_function_mask", WLAN_CFG_PRIV_M2S_FUNCTION_MASK },

    { "mcm_custom_function_mask", WLAN_CFG_PRIV_MCM_CUSTOM_FUNCTION_MASK},
    { "mcm_function_mask", WLAN_CFG_PRIV_MCM_FUNCTION_MASK },
    { "linkloss_threshold_fixed", WLAN_CFG_PRRIV_LINKLOSS_THRESHOLD_FIXED },

    { "aput_support_160m", WLAN_CFG_APUT_160M_ENABLE},
    { "radar_isr_forbid", WLAN_CFG_RADAR_ISR_FORBID},

    { "download_rate_limit_pps", WLAN_CFG_PRIV_DOWNLOAD_RATE_LIMIT_PPS },
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    { "txopps_switch", WLAN_CFG_PRIV_TXOPPS_SWITCH },
#endif
    { "over_temper_protect_threshold",   WLAN_CFG_PRIV_OVER_TEMPER_PROTECT_THRESHOLD },
    { "over_temp_pro_enable",            WLAN_CFG_PRIV_OVER_TEMP_PRO_ENABLE },
    { "over_temp_pro_reduce_pwr_enable", WLAN_CFG_PRIV_OVER_TEMP_PRO_REDUCE_PWR_ENABLE },
    { "over_temp_pro_safe_th",           WLAN_CFG_PRIV_OVER_TEMP_PRO_SAFE_TH },
    { "over_temp_pro_over_th",           WLAN_CFG_PRIV_OVER_TEMP_PRO_OVER_TH },
    { "over_temp_pro_pa_off_th",         WLAN_CFG_PRIV_OVER_TEMP_PRO_PA_OFF_TH },

    { "dsss2ofdm_dbb_pwr_bo_val",   WLAN_DSSS2OFDM_DBB_PWR_BO_VAL },
    { "evm_fail_pll_reg_fix",       WLAN_CFG_PRIV_EVM_PLL_REG_FIX },
    { "voe_switch_mask",            WLAN_CFG_PRIV_VOE_SWITCH },
    { "11ax_switch_mask",           WLAN_CFG_PRIV_11AX_SWITCH },
    { "htc_switch_mask",            WLAN_CFG_PRIV_HTC_SWITCH },
    { "multi_bssid_switch_mask",    WLAN_CFG_PRIV_MULTI_BSSID_SWITCH},
    { "ac_priv_mask",               WLAN_CFG_PRIV_AC_SUSPEND},
    { "dyn_bypass_extlna_enable",   WLAN_CFG_PRIV_DYN_BYPASS_EXTLNA },
    { "ctrl_frame_tx_chain",        WLAN_CFG_PRIV_CTRL_FRAME_TX_CHAIN },
    { "upc_cali_code_for_18dBm_c0", WLAN_CFG_PRIV_CTRL_UPC_FOR_18DBM_CO },
    { "upc_cali_code_for_18dBm_c1", WLAN_CFG_PRIV_CTRL_UPC_FOR_18DBM_C1 },
    { "11b_double_chain_bo_pow",    WLAN_CFG_PRIV_CTRL_11B_DOUBLE_CHAIN_BO_POW },
    { "hcc_flowctrl_type",          WLAN_CFG_PRIV_HCC_FLOWCTRL_TYPE },
    { "lock_cpu_freq",              WLAN_CFG_PRIV_LOCK_CPU_FREQ },
    { "mbo_switch_mask",            WLAN_CFG_PRIV_MBO_SWITCH},
    { "dynamic_dbac_adjust_mask",   WLAN_CFG_PRIV_DYNAMIC_DBAC_SWITCH},
    { "dc_flowctl_switch",          WLAN_CFG_PRIV_DC_FLOWCTL_SWITCH },
    { "phy_cap_mask",               WLAN_CFG_PRIV_PHY_CAP_SWITCH},
    { "hal_ps_rssi_param",          WLAN_CFG_PRIV_HAL_PS_RSSI_PARAM},
    { "hal_ps_pps_param",           WLAN_CFG_PRIV_HAL_PS_PPS_PARAM},
    { "optimized_feature_mask",     WLAN_CFG_PRIV_OPTIMIZED_FEATURE_SWITCH},
    { "ddr_switch_mask",            WLAN_CFG_PRIV_DDR_SWITCH},

    { "fem_backoff_pow",              WLAN_CFG_PRIV_FEM_DELT_POW},
    { "tpc_adj_pow_start_idx_by_fem", WLAN_CFG_PRIV_FEM_ADJ_TPC_TBL_START_IDX},
    { "hiex_cap",                     WLAN_CFG_PRIV_HIEX_CAP},
    { "ftm_cap",                      WLAN_CFG_PRIV_FTM_CAP},
    { "wlan_feature_miracast_sink",   WLAN_CFG_PRIV_MIRACAST_SINK },
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    { "mcast_ampdu_enable",           WLAN_CFG_PRIV_MCAST_AMPDU_ENABLE},
#endif

    { NULL,                 0 }
};

/* kunpeng eeprom */
#ifdef _PRE_CONFIG_READ_DYNCALI_E2PROM
OAL_STATIC wlan_cfg_nv_map_handler g_wifi_nvram_cfg_handler[WLAN_CFG_DTS_NVRAM_END] = {
    {"WITXCCK",  "pa2gccka0",   HWIFI_CFG_NV_WITXNVCCK_NUMBER,       WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA0, {0}, 0x100},
    {"WINVRAM",  "pa2ga0",      HWIFI_CFG_NV_WINVRAM_NUMBER,         WLAN_CFG_NVRAM_RATIO_PA2GA0,        {0}, 0x0},
    {"WITXLC0", "pa2g40a0",    HWIFI_CFG_NV_WITXL2G5G0_NUMBER,      WLAN_CFG_DTS_NVRAM_RATIO_PA2G40A0,  {0}, 0x400},
    {"WINVRAM",  "pa5ga0",      HWIFI_CFG_NV_WINVRAM_NUMBER,         WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0,    {0}, 0x80},
    {"WITXCCK",  "pa2gccka1",   HWIFI_CFG_NV_WITXNVCCK_NUMBER,       WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA1, {0}, 0x180},
    {"WITXRF1",  "pa2ga1",      HWIFI_CFG_NV_WITXNVC1_NUMBER,        WLAN_CFG_DTS_NVRAM_RATIO_PA2GA1,    {0}, 0x200},
    {"WITXLC1",  "pa2g40a1",    HWIFI_CFG_NV_WITXL2G5G1_NUMBER,      WLAN_CFG_DTS_NVRAM_RATIO_PA2G40A1,  {0}, 0x480},
    {"WITXRF1",  "pa5ga1",      HWIFI_CFG_NV_WITXNVC1_NUMBER,        WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1,    {0}, 0x280},
    {"WIC0_5GLOW",  "pa5glowa0",  HWIFI_CFG_NV_WITXNVBWC0_NUMBER,    WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW,  {0}, 0x300},
    {"WIC1_5GLOW",  "pa5glowa1",  HWIFI_CFG_NV_WITXNVBWC1_NUMBER,    WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_LOW,  {0}, 0x380},
    // DPN
    {"WIC0CCK",  "mf2gccka0",   HWIFI_CFG_NV_MUFREQ_CCK_C0_NUMBER,   WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C0,  {0}, 0x0},
    {"WC0OFDM",  "mf2ga0",      HWIFI_CFG_NV_MUFREQ_2G20_C0_NUMBER,  WLAN_CFG_DTS_NVRAM_MUFREQ_2G20_C0,   {0}, 0x0},
    {"C02G40M",  "mf2g40a0",    HWIFI_CFG_NV_MUFREQ_2G40_C0_NUMBER,  WLAN_CFG_DTS_NVRAM_MUFREQ_2G40_C0,   {0}, 0x0},
    {"WIC1CCK",  "mf2gccka1",   HWIFI_CFG_NV_MUFREQ_CCK_C1_NUMBER,   WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C1,  {0}, 0x0},
    {"WC1OFDM",  "mf2ga1",      HWIFI_CFG_NV_MUFREQ_2G20_C1_NUMBER,  WLAN_CFG_DTS_NVRAM_MUFREQ_2G20_C1,   {0}, 0x0},
    {"C12G40M",  "mf2g40a1",    HWIFI_CFG_NV_MUFREQ_2G40_C1_NUMBER,  WLAN_CFG_DTS_NVRAM_MUFREQ_2G40_C1,   {0}, 0x0},
    {"WIC0_5G160M", "dpn160c0", HWIFI_CFG_NV_MUFREQ_5G160_C0_NUMBER, WLAN_CFG_DTS_NVRAM_MUFREQ_5G160_C0, {0}, 0x500},
    {"WIC1_5G160M", "dpn160c1", HWIFI_CFG_NV_MUFREQ_5G160_C1_NUMBER, WLAN_CFG_DTS_NVRAM_MUFREQ_5G160_C1, {0}, 0x580},
};
#else
OAL_STATIC wlan_cfg_nv_map_handler g_wifi_nvram_cfg_handler[WLAN_CFG_DTS_NVRAM_END] = {
    { "WITXCCK",    "pa2gccka0", HWIFI_CFG_NV_WITXNVCCK_NUMBER,  WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA0,  {0}},
    { "WINVRAM",    "pa2ga0",    HWIFI_CFG_NV_WINVRAM_NUMBER,    WLAN_CFG_NVRAM_RATIO_PA2GA0,         {0}},
    { "WITXLC0",    "pa2g40a0",  HWIFI_CFG_NV_WITXL2G5G0_NUMBER, WLAN_CFG_DTS_NVRAM_RATIO_PA2G40A0,   {0}},
    { "WINVRAM",    "pa5ga0",    HWIFI_CFG_NV_WINVRAM_NUMBER,    WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0,     {0}},
    { "WITXCCK",    "pa2gccka1", HWIFI_CFG_NV_WITXNVCCK_NUMBER,  WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA1,  {0}},
    { "WITXRF1",    "pa2ga1",    HWIFI_CFG_NV_WITXNVC1_NUMBER,   WLAN_CFG_DTS_NVRAM_RATIO_PA2GA1,     {0}},
    { "WITXLC1",    "pa2g40a1",  HWIFI_CFG_NV_WITXL2G5G1_NUMBER, WLAN_CFG_DTS_NVRAM_RATIO_PA2G40A1,   {0}},
    { "WITXRF1",    "pa5ga1",    HWIFI_CFG_NV_WITXNVC1_NUMBER,   WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1,     {0}},
    { "W5GLOW0",    "pa5glowa0", HWIFI_CFG_NV_WITXNVBWC0_NUMBER, WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW, {0}},
    { "W5GLOW1",    "pa5glowa1", HWIFI_CFG_NV_WITXNVBWC1_NUMBER, WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_LOW, {0}},
    // DPN
    { "WIC0CCK",     "mf2gccka0", HWIFI_CFG_NV_MUFREQ_CCK_C0_NUMBER,   WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C0, {0}},
    { "WC0OFDM",     "mf2ga0",    HWIFI_CFG_NV_MUFREQ_2G20_C0_NUMBER,  WLAN_CFG_DTS_NVRAM_MUFREQ_2G20_C0,  {0}},
    { "C02G40M",     "mf2g40a0",  HWIFI_CFG_NV_MUFREQ_2G40_C0_NUMBER,  WLAN_CFG_DTS_NVRAM_MUFREQ_2G40_C0,  {0}},
    { "WIC1CCK",     "mf2gccka1", HWIFI_CFG_NV_MUFREQ_CCK_C1_NUMBER,   WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C1, {0}},
    { "WC1OFDM",     "mf2ga1",    HWIFI_CFG_NV_MUFREQ_2G20_C1_NUMBER,  WLAN_CFG_DTS_NVRAM_MUFREQ_2G20_C1,  {0}},
    { "C12G40M",     "mf2g40a1",  HWIFI_CFG_NV_MUFREQ_2G40_C1_NUMBER,  WLAN_CFG_DTS_NVRAM_MUFREQ_2G40_C1,  {0}},
    { "W160MC0",     "dpn160c0",  HWIFI_CFG_NV_MUFREQ_5G160_C0_NUMBER, WLAN_CFG_DTS_NVRAM_MUFREQ_5G160_C0, {0}},
    { "W160MC1",     "dpn160c1",  HWIFI_CFG_NV_MUFREQ_5G160_C1_NUMBER, WLAN_CFG_DTS_NVRAM_MUFREQ_5G160_C1, {0}},
};
#endif

OAL_STATIC wlan_cfg_cmd g_wifi_config_cmds[] = {
    /* ROAM */
    { "roam_switch",         WLAN_CFG_INIT_ROAM_SWITCH },
    { "scan_orthogonal",     WLAN_CFG_INIT_SCAN_ORTHOGONAL },
    { "trigger_b",           WLAN_CFG_INIT_TRIGGER_B },
    { "trigger_a",           WLAN_CFG_INIT_TRIGGER_A },
    { "delta_b",             WLAN_CFG_INIT_DELTA_B },
    { "delta_a",             WLAN_CFG_INIT_DELTA_A },
    { "dense_env_trigger_b", WLAN_CFG_INIT_DENSE_ENV_TRIGGER_B },
    { "dense_env_trigger_a", WLAN_CFG_INIT_DENSE_ENV_TRIGGER_A },
    { "scenario_enable",     WLAN_CFG_INIT_SCENARIO_ENABLE },
    { "candidate_good_rssi", WLAN_CFG_INIT_CANDIDATE_GOOD_RSSI },
    { "candidate_good_num",  WLAN_CFG_INIT_CANDIDATE_GOOD_NUM },
    { "candidate_weak_num",  WLAN_CFG_INIT_CANDIDATE_WEAK_NUM },
    { "interval_variable",   WLAN_CFG_INIT_INTERVAL_VARIABLE },

    /* 性能 */
    { "ampdu_tx_max_num",        WLAN_CFG_INIT_AMPDU_TX_MAX_NUM },
    { "used_mem_for_start",      WLAN_CFG_INIT_USED_MEM_FOR_START },
    { "used_mem_for_stop",       WLAN_CFG_INIT_USED_MEM_FOR_STOP },
    { "rx_ack_limit",            WLAN_CFG_INIT_RX_ACK_LIMIT },
    { "sdio_d2h_assemble_count", WLAN_CFG_INIT_SDIO_D2H_ASSEMBLE_COUNT },
    { "sdio_h2d_assemble_count", WLAN_CFG_INIT_SDIO_H2D_ASSEMBLE_COUNT },
    /* LINKLOSS */
    { "link_loss_threshold_bt",     WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_BT },
    { "link_loss_threshold_dbac",   WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_DBAC },
    { "link_loss_threshold_normal", WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_NORMAL },
    /* 自动调频 */
    { "pss_threshold_level_0",  WLAN_CFG_INIT_PSS_THRESHOLD_LEVEL_0 },
    { "cpu_freq_limit_level_0", WLAN_CFG_INIT_CPU_FREQ_LIMIT_LEVEL_0 },
    { "ddr_freq_limit_level_0", WLAN_CFG_INIT_DDR_FREQ_LIMIT_LEVEL_0 },
    { "pss_threshold_level_1",  WLAN_CFG_INIT_PSS_THRESHOLD_LEVEL_1 },
    { "cpu_freq_limit_level_1", WLAN_CFG_INIT_CPU_FREQ_LIMIT_LEVEL_1 },
    { "ddr_freq_limit_level_1", WLAN_CFG_INIT_DDR_FREQ_LIMIT_LEVEL_1 },
    { "pss_threshold_level_2",  WLAN_CFG_INIT_PSS_THRESHOLD_LEVEL_2 },
    { "cpu_freq_limit_level_2", WLAN_CFG_INIT_CPU_FREQ_LIMIT_LEVEL_2 },
    { "ddr_freq_limit_level_2", WLAN_CFG_INIT_DDR_FREQ_LIMIT_LEVEL_2 },
    { "pss_threshold_level_3",  WLAN_CFG_INIT_PSS_THRESHOLD_LEVEL_3 },
    { "cpu_freq_limit_level_3", WLAN_CFG_INIT_CPU_FREQ_LIMIT_LEVEL_3 },
    { "ddr_freq_limit_level_3", WLAN_CFG_INIT_DDR_FREQ_LIMIT_LEVEL_3 },
    { "device_type_level_0",    WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_0 },
    { "device_type_level_1",    WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_1 },
    { "device_type_level_2",    WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_2 },
    { "device_type_level_3",    WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_3 },
    /* 修改DMA latency,避免cpu进入过深的idle state */
    { "lock_dma_latency",  WLAN_CFG_PRIV_DMA_LATENCY },
    { "lock_cpu_th_high",           WLAN_CFG_PRIV_LOCK_CPU_TH_HIGH },
    { "lock_cpu_th_low",            WLAN_CFG_PRIV_LOCK_CPU_TH_LOW },
    /* 收发中断动态绑核 */
    { "irq_affinity",       WLAN_CFG_INIT_IRQ_AFFINITY },
    { "cpu_id_th_low",      WLAN_CFG_INIT_IRQ_TH_LOW },
    { "cpu_id_th_high",     WLAN_CFG_INIT_IRQ_TH_HIGH },
    { "cpu_id_pps_th_low",  WLAN_CFG_INIT_IRQ_PPS_TH_LOW },
    { "cpu_id_pps_th_high", WLAN_CFG_INIT_IRQ_PPS_TH_HIGH },
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    /* 硬件聚合使能 */
    { "hw_ampdu",      WLAN_CFG_INIT_HW_AMPDU },
    { "hw_ampdu_th_l", WLAN_CFG_INIT_HW_AMPDU_TH_LOW },
    { "hw_ampdu_th_h", WLAN_CFG_INIT_HW_AMPDU_TH_HIGH },
#endif
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    { "tx_amsdu_ampdu",      WLAN_CFG_INIT_AMPDU_AMSDU_SKB },
    { "tx_amsdu_ampdu_th_l", WLAN_CFG_INIT_AMSDU_AMPDU_TH_LOW },
    { "tx_amsdu_ampdu_th_m", WLAN_CFG_INIT_AMSDU_AMPDU_TH_MIDDLE },
    { "tx_amsdu_ampdu_th_h", WLAN_CFG_INIT_AMSDU_AMPDU_TH_HIGH },
#endif
#ifdef _PRE_WLAN_TCP_OPT
    { "en_tcp_ack_filter",      WLAN_CFG_INIT_TCP_ACK_FILTER },
    { "rx_tcp_ack_filter_th_l", WLAN_CFG_INIT_TCP_ACK_FILTER_TH_LOW },
    { "rx_tcp_ack_filter_th_h", WLAN_CFG_INIT_TCP_ACK_FILTER_TH_HIGH },
#endif

    { "small_amsdu_switch",   WLAN_CFG_INIT_TX_SMALL_AMSDU },
    { "small_amsdu_th_h",     WLAN_CFG_INIT_SMALL_AMSDU_HIGH },
    { "small_amsdu_th_l",     WLAN_CFG_INIT_SMALL_AMSDU_LOW },
    { "small_amsdu_pps_th_h", WLAN_CFG_INIT_SMALL_AMSDU_PPS_HIGH },
    { "small_amsdu_pps_th_l", WLAN_CFG_INIT_SMALL_AMSDU_PPS_LOW },

    { "tcp_ack_buf_switch",    WLAN_CFG_INIT_TX_TCP_ACK_BUF },
    { "tcp_ack_buf_th_h",      WLAN_CFG_INIT_TCP_ACK_BUF_HIGH },
    { "tcp_ack_buf_th_l",      WLAN_CFG_INIT_TCP_ACK_BUF_LOW },
    { "tcp_ack_buf_th_h_40M",  WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_40M },
    { "tcp_ack_buf_th_l_40M",  WLAN_CFG_INIT_TCP_ACK_BUF_LOW_40M },
    { "tcp_ack_buf_th_h_80M",  WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_80M },
    { "tcp_ack_buf_th_l_80M",  WLAN_CFG_INIT_TCP_ACK_BUF_LOW_80M },
    { "tcp_ack_buf_th_h_160M", WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_160M },
    { "tcp_ack_buf_th_l_160M", WLAN_CFG_INIT_TCP_ACK_BUF_LOW_160M },

    { "tcp_ack_buf_userctl_switch", WLAN_CFG_INIT_TX_TCP_ACK_BUF_USERCTL },
    { "tcp_ack_buf_userctl_h",      WLAN_CFG_INIT_TCP_ACK_BUF_USERCTL_HIGH },
    { "tcp_ack_buf_userctl_l",      WLAN_CFG_INIT_TCP_ACK_BUF_USERCTL_LOW },

    { "dyn_bypass_extlna_th_switch", WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA },
    { "dyn_bypass_extlna_th_h",      WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_HIGH },
    { "dyn_bypass_extlna_th_l",      WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_LOW },

    { "rx_ampdu_amsdu", WLAN_CFG_INIT_RX_AMPDU_AMSDU_SKB },
    { "rx_ampdu_bitmap", WLAN_CFG_INIT_RX_AMPDU_BITMAP },

    /* 低功耗 */
    { "powermgmt_switch", WLAN_CFG_INIT_POWERMGMT_SWITCH },

    { "ps_mode",                        WLAN_CFG_INIT_PS_MODE },
    { "min_fast_ps_idle",               WLAN_CFG_INIT_MIN_FAST_PS_IDLE },
    { "max_fast_ps_idle",               WLAN_CFG_INIT_MAX_FAST_PS_IDLE },
    { "auto_fast_ps_thresh_screen_on",  WLAN_CFG_INIT_AUTO_FAST_PS_THRESH_SCREENON },
    { "auto_fast_ps_thresh_screen_off", WLAN_CFG_INIT_AUTO_FAST_PS_THRESH_SCREENOFF },
    /* 可维可测 */
    { "loglevel", WLAN_CFG_INIT_LOGLEVEL },
    /* 2G RF前端插损 */
    { "rf_rx_insertion_loss_2g_b1", WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND1 },
    { "rf_rx_insertion_loss_2g_b2", WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND2 },
    { "rf_rx_insertion_loss_2g_b3", WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND3 },
    /* 5G RF前端插损 */
    { "rf_rx_insertion_loss_5g_b1", WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND1 },
    { "rf_rx_insertion_loss_5g_b2", WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND2 },
    { "rf_rx_insertion_loss_5g_b3", WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND3 },
    { "rf_rx_insertion_loss_5g_b4", WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND4 },
    { "rf_rx_insertion_loss_5g_b5", WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND5 },
    { "rf_rx_insertion_loss_5g_b6", WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND6 },
    { "rf_rx_insertion_loss_5g_b7", WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND7 },
    /* 用于定制化计算PWR RF值的偏差 */
    { "rf_line_rf_pwr_ref_rssi_db_2g_c0_mult4",  WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C0_MULT4 },
    { "rf_line_rf_pwr_ref_rssi_db_2g_c1_mult4",  WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C1_MULT4 },
    { "rf_line_rf_pwr_ref_rssi_db_5g_c0_mult4",  WLAN_CFG_INIT_RF_PWR_REF_RSSI_5G_C0_MULT4 },
    { "rf_line_rf_pwr_ref_rssi_db_5g_c1_mult4",  WLAN_CFG_INIT_RF_PWR_REF_RSSI_5G_C1_MULT4 },

    {"rf_rssi_amend_2g_c0", WLAN_CFG_INIT_RF_AMEND_RSSI_2G_C0},
    {"rf_rssi_amend_2g_c1", WLAN_CFG_INIT_RF_AMEND_RSSI_2G_C1},
    {"rf_rssi_amend_5g_c0", WLAN_CFG_INIT_RF_AMEND_RSSI_5G_C0},
    {"rf_rssi_amend_5g_c1", WLAN_CFG_INIT_RF_AMEND_RSSI_5G_C1},

    /* fem */
    { "rf_lna_bypass_gain_db_2g", WLAN_CFG_INIT_RF_LNA_BYPASS_GAIN_DB_2G },
    { "rf_lna_gain_db_2g",        WLAN_CFG_INIT_RF_LNA_GAIN_DB_2G },
    { "rf_pa_db_b0_2g",           WLAN_CFG_INIT_RF_PA_GAIN_DB_B0_2G },
    { "rf_pa_db_b1_2g",           WLAN_CFG_INIT_RF_PA_GAIN_DB_B1_2G },
    { "rf_pa_db_lvl_2g",          WLAN_CFG_INIT_RF_PA_GAIN_LVL_2G },
    { "ext_switch_isexist_2g",    WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_2G },
    { "ext_pa_isexist_2g",        WLAN_CFG_INIT_EXT_PA_ISEXIST_2G },
    { "ext_lna_isexist_2g",       WLAN_CFG_INIT_EXT_LNA_ISEXIST_2G },
    { "lna_on2off_time_ns_2g",    WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_2G },
    { "lna_off2on_time_ns_2g",    WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_2G },
    { "rf_lna_bypass_gain_db_5g", WLAN_CFG_INIT_RF_LNA_BYPASS_GAIN_DB_5G },
    { "rf_lna_gain_db_5g",        WLAN_CFG_INIT_RF_LNA_GAIN_DB_5G },
    { "rf_pa_db_b0_5g",           WLAN_CFG_INIT_RF_PA_GAIN_DB_B0_5G },
    { "rf_pa_db_b1_5g",           WLAN_CFG_INIT_RF_PA_GAIN_DB_B1_5G },
    { "rf_pa_db_lvl_5g",          WLAN_CFG_INIT_RF_PA_GAIN_LVL_5G },
    { "ext_switch_isexist_5g",    WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_5G },
    { "ext_pa_isexist_5g",        WLAN_CFG_INIT_EXT_PA_ISEXIST_5G },
    { "ext_lna_isexist_5g",       WLAN_CFG_INIT_EXT_LNA_ISEXIST_5G },
    { "lna_on2off_time_ns_5g",    WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_5G },
    { "lna_off2on_time_ns_5g",    WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_5G },
    /* SCAN */
    { "random_mac_addr_scan", WLAN_CFG_INIT_RANDOM_MAC_ADDR_SCAN },
    /* 11AC2G */
    { "11ac2g_enable",        WLAN_CFG_INIT_11AC2G_ENABLE },
    { "disable_capab_2ght40", WLAN_CFG_INIT_DISABLE_CAPAB_2GHT40 },
    { "dual_antenna_enable",  WLAN_CFG_INIT_DUAL_ANTENNA_ENABLE }, /* 双天线开关 */

    { "probe_resp_mode", WLAN_CFG_INIT_PROBE_RESP_MODE},
    { "miracast_enable", WLAN_CFG_INIT_MIRACAST_SINK_ENABLE},
    { "reduce_miracast_log", WLAN_CFG_INIT_REDUCE_MIRACAST_LOG},
    { "core_bind_cap", WLAN_CFG_INIT_CORE_BIND_CAP},
    {"only_fast_mode", WLAN_CFG_INIT_FAST_MODE},

    /* sta keepalive cnt th */
    { "sta_keepalive_cnt_th", WLAN_CFG_INIT_STA_KEEPALIVE_CNT_TH }, /* 动态功率校准 */

    { "far_dist_pow_gain_switch", WLAN_CFG_INIT_FAR_DIST_POW_GAIN_SWITCH },
    { "far_dist_dsss_scale_promote_switch", WLAN_CFG_INIT_FAR_DIST_DSSS_SCALE_PROMOTE_SWITCH },
    { "chann_radio_cap", WLAN_CFG_INIT_CHANN_RADIO_CAP },

    { "lte_gpio_check_switch",    WLAN_CFG_LTE_GPIO_CHECK_SWITCH }, /* lte?????? */
    { "ism_priority",             WLAN_ATCMDSRV_ISM_PRIORITY },
    { "lte_rx",                   WLAN_ATCMDSRV_LTE_RX },
    { "lte_tx",                   WLAN_ATCMDSRV_LTE_TX },
    { "lte_inact",                WLAN_ATCMDSRV_LTE_INACT },
    { "ism_rx_act",               WLAN_ATCMDSRV_ISM_RX_ACT },
    { "bant_pri",                 WLAN_ATCMDSRV_BANT_PRI },
    { "bant_status",              WLAN_ATCMDSRV_BANT_STATUS },
    { "want_pri",                 WLAN_ATCMDSRV_WANT_PRI },
    { "want_status",              WLAN_ATCMDSRV_WANT_STATUS },
    { "tx5g_upc_mix_gain_ctrl_1", WLAN_TX5G_UPC_MIX_GAIN_CTRL_1 },
    { "tx5g_upc_mix_gain_ctrl_2", WLAN_TX5G_UPC_MIX_GAIN_CTRL_2 },
    { "tx5g_upc_mix_gain_ctrl_3", WLAN_TX5G_UPC_MIX_GAIN_CTRL_3 },
    { "tx5g_upc_mix_gain_ctrl_4", WLAN_TX5G_UPC_MIX_GAIN_CTRL_4 },
    { "tx5g_upc_mix_gain_ctrl_5", WLAN_TX5G_UPC_MIX_GAIN_CTRL_5 },
    { "tx5g_upc_mix_gain_ctrl_6", WLAN_TX5G_UPC_MIX_GAIN_CTRL_6 },
    { "tx5g_upc_mix_gain_ctrl_7", WLAN_TX5G_UPC_MIX_GAIN_CTRL_7 },
    /* 定制化RF部分PA偏置寄存器 */
    { "tx2g_pa_gate_236", WLAN_TX2G_PA_GATE_VCTL_REG236 },
    { "tx2g_pa_gate_237", WLAN_TX2G_PA_GATE_VCTL_REG237 },
    { "tx2g_pa_gate_238", WLAN_TX2G_PA_GATE_VCTL_REG238 },
    { "tx2g_pa_gate_239", WLAN_TX2G_PA_GATE_VCTL_REG239 },
    { "tx2g_pa_gate_240", WLAN_TX2G_PA_GATE_VCTL_REG240 },
    { "tx2g_pa_gate_241", WLAN_TX2G_PA_GATE_VCTL_REG241 },
    { "tx2g_pa_gate_242", WLAN_TX2G_PA_GATE_VCTL_REG242 },
    { "tx2g_pa_gate_243", WLAN_TX2G_PA_GATE_VCTL_REG243 },
    { "tx2g_pa_gate_244", WLAN_TX2G_PA_GATE_VCTL_REG244 },

    { "tx2g_pa_gate_253",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG253 },
    { "tx2g_pa_gate_254",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG254 },
    { "tx2g_pa_gate_255",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG255 },
    { "tx2g_pa_gate_256",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG256 },
    { "tx2g_pa_gate_257",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG257 },
    { "tx2g_pa_gate_258",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG258 },
    { "tx2g_pa_gate_259",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG259 },
    { "tx2g_pa_gate_260",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG260 },
    { "tx2g_pa_gate_261",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG261 },
    { "tx2g_pa_gate_262",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG262 },
    { "tx2g_pa_gate_263",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG263 },
    { "tx2g_pa_gate_264",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG264 },
    { "tx2g_pa_gate_265",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG265 },
    { "tx2g_pa_gate_266",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG266 },
    { "tx2g_pa_gate_267",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG267 },
    { "tx2g_pa_gate_268",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG268 },
    { "tx2g_pa_gate_269",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG269 },
    { "tx2g_pa_gate_270",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG270 },
    { "tx2g_pa_gate_271",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG271 },
    { "tx2g_pa_gate_272",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG272 },
    { "tx2g_pa_gate_273",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG273 },
    { "tx2g_pa_gate_274",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG274 },
    { "tx2g_pa_gate_275",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG275 },
    { "tx2g_pa_gate_276",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG276 },
    { "tx2g_pa_gate_277",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG277 },
    { "tx2g_pa_gate_278",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG278 },
    { "tx2g_pa_gate_279",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG279 },
    { "tx2g_pa_gate_280_band1", WLAN_TX2G_PA_VRECT_GATE_THIN_REG280_BAND1 },
    { "tx2g_pa_gate_281",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG281 },
    { "tx2g_pa_gate_282",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG282 },
    { "tx2g_pa_gate_283",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG283 },
    { "tx2g_pa_gate_284",       WLAN_TX2G_PA_VRECT_GATE_THIN_REG284 },
    { "tx2g_pa_gate_280_band2", WLAN_TX2G_PA_VRECT_GATE_THIN_REG280_BAND2 },
    { "tx2g_pa_gate_280_band3", WLAN_TX2G_PA_VRECT_GATE_THIN_REG280_BAND3 },
    { "delta_cca_ed_high_20th_2g", WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_2G },
    { "delta_cca_ed_high_40th_2g", WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_2G },
    { "delta_cca_ed_high_20th_5g", WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_5G },
    { "delta_cca_ed_high_40th_5g", WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_5G },
    { "delta_cca_ed_high_80th_5g", WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_80TH_5G },
    { "voe_switch_mask",           WLAN_CFG_INIT_VOE_SWITCH },
    { "11ax_switch_mask",          WLAN_CFG_INIT_11AX_SWITCH },
    { "htc_switch_mask",           WLAN_CFG_INIT_HTC_SWITCH },
    { "multi_bssid_switch_mask",   WLAN_CFG_INIT_MULTI_BSSID_SWITCH},
    /* ldac m2s rssi */
    { "ldac_threshold_m2s", WLAN_CFG_INIT_LDAC_THRESHOLD_M2S },
    { "ldac_threshold_s2m", WLAN_CFG_INIT_LDAC_THRESHOLD_S2M },
    /* btcoex mcm rssi */
    { "btcoex_threshold_mcm_down", WLAN_CFG_INIT_BTCOEX_THRESHOLD_MCM_DOWN },
    { "btcoex_threshold_mcm_up",   WLAN_CFG_INIT_BTCOEX_THRESHOLD_MCM_UP },
#ifdef _PRE_WLAN_FEATURE_NRCOEX
    /* 5g nr coex */
    {"nrcoex_enable",                   WLAN_CFG_INIT_NRCOEX_ENABLE},
    {"nrcoex_rule0_freq",               WLAN_CFG_INIT_NRCOEX_RULE0_FREQ},
    {"nrcoex_rule0_40m_20m_gap0",       WLAN_CFG_INIT_NRCOEX_RULE0_40M_20M_GAP0},
    {"nrcoex_rule0_160m_80m_gap0",      WLAN_CFG_INIT_NRCOEX_RULE0_160M_80M_GAP0},
    {"nrcoex_rule0_40m_20m_gap1",       WLAN_CFG_INIT_NRCOEX_RULE0_40M_20M_GAP1},
    {"nrcoex_rule0_160m_80m_gap1",      WLAN_CFG_INIT_NRCOEX_RULE0_160M_80M_GAP1},
    {"nrcoex_rule0_40m_20m_gap2",       WLAN_CFG_INIT_NRCOEX_RULE0_40M_20M_GAP2},
    {"nrcoex_rule0_160m_80m_gap2",      WLAN_CFG_INIT_NRCOEX_RULE0_160M_80M_GAP2},
    {"nrcoex_rule0_smallgap0_act",      WLAN_CFG_INIT_NRCOEX_RULE0_SMALLGAP0_ACT},
    {"nrcoex_rule0_gap01_act",          WLAN_CFG_INIT_NRCOEX_RULE0_GAP01_ACT},
    {"nrcoex_rule0_gap12_act",          WLAN_CFG_INIT_NRCOEX_RULE0_GAP12_ACT},
    {"nrcoex_rule0_rxslot_rssi",        WLAN_CFG_INIT_NRCOEX_RULE0_RXSLOT_RSSI},
    {"nrcoex_rule1_freq",               WLAN_CFG_INIT_NRCOEX_RULE1_FREQ},
    {"nrcoex_rule1_40m_20m_gap0",       WLAN_CFG_INIT_NRCOEX_RULE1_40M_20M_GAP0},
    {"nrcoex_rule1_160m_80m_gap0",      WLAN_CFG_INIT_NRCOEX_RULE1_160M_80M_GAP0},
    {"nrcoex_rule1_40m_20m_gap1",       WLAN_CFG_INIT_NRCOEX_RULE1_40M_20M_GAP1},
    {"nrcoex_rule1_160m_80m_gap1",      WLAN_CFG_INIT_NRCOEX_RULE1_160M_80M_GAP1},
    {"nrcoex_rule1_40m_20m_gap2",       WLAN_CFG_INIT_NRCOEX_RULE1_40M_20M_GAP2},
    {"nrcoex_rule1_160m_80m_gap2",      WLAN_CFG_INIT_NRCOEX_RULE1_160M_80M_GAP2},
    {"nrcoex_rule1_smallgap0_act",      WLAN_CFG_INIT_NRCOEX_RULE1_SMALLGAP0_ACT},
    {"nrcoex_rule1_gap01_act",          WLAN_CFG_INIT_NRCOEX_RULE1_GAP01_ACT},
    {"nrcoex_rule1_gap12_act",          WLAN_CFG_INIT_NRCOEX_RULE1_GAP12_ACT},
    {"nrcoex_rule1_rxslot_rssi",        WLAN_CFG_INIT_NRCOEX_RULE1_RXSLOT_RSSI},
    {"nrcoex_rule2_freq",               WLAN_CFG_INIT_NRCOEX_RULE2_FREQ},
    {"nrcoex_rule2_40m_20m_gap0",       WLAN_CFG_INIT_NRCOEX_RULE2_40M_20M_GAP0},
    {"nrcoex_rule2_160m_80m_gap0",      WLAN_CFG_INIT_NRCOEX_RULE2_160M_80M_GAP0},
    {"nrcoex_rule2_40m_20m_gap1",       WLAN_CFG_INIT_NRCOEX_RULE2_40M_20M_GAP1},
    {"nrcoex_rule2_160m_80m_gap1",      WLAN_CFG_INIT_NRCOEX_RULE2_160M_80M_GAP1},
    {"nrcoex_rule2_40m_20m_gap2",       WLAN_CFG_INIT_NRCOEX_RULE2_40M_20M_GAP2},
    {"nrcoex_rule2_160m_80m_gap2",      WLAN_CFG_INIT_NRCOEX_RULE2_160M_80M_GAP2},
    {"nrcoex_rule2_smallgap0_act",      WLAN_CFG_INIT_NRCOEX_RULE2_SMALLGAP0_ACT},
    {"nrcoex_rule2_gap01_act",          WLAN_CFG_INIT_NRCOEX_RULE2_GAP01_ACT},
    {"nrcoex_rule2_gap12_act",          WLAN_CFG_INIT_NRCOEX_RULE2_GAP12_ACT},
    {"nrcoex_rule2_rxslot_rssi",        WLAN_CFG_INIT_NRCOEX_RULE2_RXSLOT_RSSI},
    {"nrcoex_rule3_freq",               WLAN_CFG_INIT_NRCOEX_RULE3_FREQ},
    {"nrcoex_rule3_40m_20m_gap0",       WLAN_CFG_INIT_NRCOEX_RULE3_40M_20M_GAP0},
    {"nrcoex_rule3_160m_80m_gap0",      WLAN_CFG_INIT_NRCOEX_RULE3_160M_80M_GAP0},
    {"nrcoex_rule3_40m_20m_gap1",       WLAN_CFG_INIT_NRCOEX_RULE3_40M_20M_GAP1},
    {"nrcoex_rule3_160m_80m_gap1",      WLAN_CFG_INIT_NRCOEX_RULE3_160M_80M_GAP1},
    {"nrcoex_rule3_40m_20m_gap2",       WLAN_CFG_INIT_NRCOEX_RULE3_40M_20M_GAP2},
    {"nrcoex_rule3_160m_80m_gap2",      WLAN_CFG_INIT_NRCOEX_RULE3_160M_80M_GAP2},
    {"nrcoex_rule3_smallgap0_act",      WLAN_CFG_INIT_NRCOEX_RULE3_SMALLGAP0_ACT},
    {"nrcoex_rule3_gap01_act",          WLAN_CFG_INIT_NRCOEX_RULE3_GAP01_ACT},
    {"nrcoex_rule3_gap12_act",          WLAN_CFG_INIT_NRCOEX_RULE3_GAP12_ACT},
    {"nrcoex_rule3_rxslot_rssi",        WLAN_CFG_INIT_NRCOEX_RULE3_RXSLOT_RSSI},
#endif
#ifdef _PRE_WLAN_FEATURE_MBO
    {"mbo_switch_mask",                 WLAN_CFG_INIT_MBO_SWITCH},
#endif
    { "dynamic_dbac_adjust_mask",       WLAN_CFG_INIT_DYNAMIC_DBAC_SWITCH},
    { "ddr_freq",                       WLAN_CFG_INIT_DDR_FREQ},
    { "hiex_cap",                       WLAN_CFG_INIT_HIEX_CAP},
    { "ftm_cap",                        WLAN_CFG_INIT_FTM_CAP},
    { "rf_filter_narrow_rssi_amend_2g_c0", WLAN_CFG_INIT_RF_FILTER_NARROW_RSSI_AMEND_2G_C0},
    { "rf_filter_narrow_rssi_amend_2g_c1", WLAN_CFG_INIT_RF_FILTER_NARROW_RSSI_AMEND_2G_C1},
    { "rf_filter_narrow_rssi_amend_5g_c0", WLAN_CFG_INIT_RF_FILTER_NARROW_RSSI_AMEND_5G_C0},
    { "rf_filter_narrow_rssi_amend_5g_c1", WLAN_CFG_INIT_RF_FILTER_NARROW_RSSI_AMEND_5G_C1},

#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    { "mcast_ampdu_enable",             WLAN_CFG_INIT_MCAST_AMPDU_ENABLE},
#endif
    { NULL,         0 }
};

OAL_STATIC wlan_cfg_cmd g_nvram_config_ini[NVRAM_PARAMS_PWR_INDEX_BUTT] = {
    { "nvram_params0",  NVRAM_PARAMS_INDEX_0 },
    { "nvram_params1",  NVRAM_PARAMS_INDEX_1 },
    { "nvram_params2",  NVRAM_PARAMS_INDEX_2 },
    { "nvram_params3",  NVRAM_PARAMS_INDEX_3 },
    { "nvram_params4",  NVRAM_PARAMS_INDEX_4 },
    { "nvram_params5",  NVRAM_PARAMS_INDEX_5 },
    { "nvram_params6",  NVRAM_PARAMS_INDEX_6 },
    { "nvram_params7",  NVRAM_PARAMS_INDEX_7 },
    { "nvram_params8",  NVRAM_PARAMS_INDEX_8 },
    { "nvram_params9",  NVRAM_PARAMS_INDEX_9 },
    { "nvram_params10", NVRAM_PARAMS_INDEX_10 },
    { "nvram_params11", NVRAM_PARAMS_INDEX_11 },
    { "nvram_params12", NVRAM_PARAMS_INDEX_12 },
    { "nvram_params13", NVRAM_PARAMS_INDEX_13 },
    { "nvram_params14", NVRAM_PARAMS_INDEX_14 },
    { "nvram_params15", NVRAM_PARAMS_INDEX_15 },
    { "nvram_params16", NVRAM_PARAMS_INDEX_16 },
    { "nvram_params17", NVRAM_PARAMS_INDEX_17 },
    { "nvram_params59", NVRAM_PARAMS_INDEX_DPD_0 },
    { "nvram_params60", NVRAM_PARAMS_INDEX_DPD_1 },
    { "nvram_params61", NVRAM_PARAMS_INDEX_DPD_2 },
    /* 11B & OFDM delta power */
    { "nvram_params62", NVRAM_PARAMS_INDEX_11B_OFDM_DELT_POW },
    /* 5G cali upper upc limit */
    { "nvram_params63", NVRAM_PARAMS_INDEX_IQ_MAX_UPC },
    /* 2G low pow amend */
    { "nvram_params64",                  NVRAM_PARAMS_INDEX_2G_LOW_POW_AMEND },
    { NULL,                      NVRAM_PARAMS_TXPWR_INDEX_BUTT },
    { "nvram_max_txpwr_base_2p4g",       NVRAM_PARAMS_INDEX_19 },
    { "nvram_max_txpwr_base_5g",         NVRAM_PARAMS_INDEX_20 },
    { "nvram_max_txpwr_base_2p4g_slave", NVRAM_PARAMS_INDEX_21 },
    { "nvram_max_txpwr_base_5g_slave",   NVRAM_PARAMS_INDEX_22 },
    { NULL,                      NVRAM_PARAMS_BASE_INDEX_BUTT },
    { "nvram_delt_max_base_txpwr",       NVRAM_PARAMS_INDEX_DELT_BASE_POWER_23 },
    { NULL,                      NVRAM_PARAMS_INDEX_24_RSV },
    /* FCC */
    { "fcc_side_band_txpwr_limit_5g_20m_0", NVRAM_PARAMS_INDEX_25 },
    { "fcc_side_band_txpwr_limit_5g_20m_1", NVRAM_PARAMS_INDEX_26 },
    { "fcc_side_band_txpwr_limit_5g_40m_0", NVRAM_PARAMS_INDEX_27 },
    { "fcc_side_band_txpwr_limit_5g_40m_1", NVRAM_PARAMS_INDEX_28 },
    { "fcc_side_band_txpwr_limit_5g_80m_0", NVRAM_PARAMS_INDEX_29 },
    { "fcc_side_band_txpwr_limit_5g_80m_1", NVRAM_PARAMS_INDEX_30 },
    { "fcc_side_band_txpwr_limit_5g_160m",  NVRAM_PARAMS_INDEX_31 },
    { "fcc_side_band_txpwr_limit_24g_ch1",  NVRAM_PARAMS_INDEX_32 },
    { "fcc_side_band_txpwr_limit_24g_ch2",  NVRAM_PARAMS_INDEX_33 },
    { "fcc_side_band_txpwr_limit_24g_ch3",  NVRAM_PARAMS_INDEX_34 },
    { "fcc_side_band_txpwr_limit_24g_ch4",  NVRAM_PARAMS_INDEX_35 },
    { "fcc_side_band_txpwr_limit_24g_ch5",  NVRAM_PARAMS_INDEX_36 },
    { "fcc_side_band_txpwr_limit_24g_ch6",  NVRAM_PARAMS_INDEX_37 },
    { "fcc_side_band_txpwr_limit_24g_ch7",  NVRAM_PARAMS_INDEX_38 },
    { "fcc_side_band_txpwr_limit_24g_ch8",  NVRAM_PARAMS_INDEX_39 },
    { "fcc_side_band_txpwr_limit_24g_ch9",  NVRAM_PARAMS_INDEX_40 },
    { "fcc_side_band_txpwr_limit_24g_ch10", NVRAM_PARAMS_INDEX_41 },
    { "fcc_side_band_txpwr_limit_24g_ch11", NVRAM_PARAMS_INDEX_42 },
    { "fcc_side_band_txpwr_limit_24g_ch12", NVRAM_PARAMS_INDEX_43 },
    { "fcc_side_band_txpwr_limit_24g_ch13", NVRAM_PARAMS_INDEX_44 },
    { NULL,                         NVRAM_PARAMS_FCC_END_INDEX_BUTT },
    /* CE */
    { "ce_side_band_txpwr_limit_5g_20m_0", NVRAM_PARAMS_INDEX_CE_0 },
    { "ce_side_band_txpwr_limit_5g_20m_1", NVRAM_PARAMS_INDEX_CE_1 },
    { "ce_side_band_txpwr_limit_5g_40m_0", NVRAM_PARAMS_INDEX_CE_2 },
    { "ce_side_band_txpwr_limit_5g_40m_1", NVRAM_PARAMS_INDEX_CE_3 },
    { "ce_side_band_txpwr_limit_5g_80m_0", NVRAM_PARAMS_INDEX_CE_4 },
    { "ce_side_band_txpwr_limit_5g_80m_1", NVRAM_PARAMS_INDEX_CE_5 },
    { "ce_side_band_txpwr_limit_5g_160m",  NVRAM_PARAMS_INDEX_CE_6 },
    { "ce_side_band_txpwr_limit_24g_ch1",  NVRAM_PARAMS_INDEX_CE_7 },
    { "ce_side_band_txpwr_limit_24g_ch2",  NVRAM_PARAMS_INDEX_CE_8 },
    { "ce_side_band_txpwr_limit_24g_ch3",  NVRAM_PARAMS_INDEX_CE_9 },
    { "ce_side_band_txpwr_limit_24g_ch4",  NVRAM_PARAMS_INDEX_CE_10 },
    { "ce_side_band_txpwr_limit_24g_ch5",  NVRAM_PARAMS_INDEX_CE_11 },
    { "ce_side_band_txpwr_limit_24g_ch6",  NVRAM_PARAMS_INDEX_CE_12 },
    { "ce_side_band_txpwr_limit_24g_ch7",  NVRAM_PARAMS_INDEX_CE_13 },
    { "ce_side_band_txpwr_limit_24g_ch8",  NVRAM_PARAMS_INDEX_CE_14 },
    { "ce_side_band_txpwr_limit_24g_ch9",  NVRAM_PARAMS_INDEX_CE_15 },
    { "ce_side_band_txpwr_limit_24g_ch10", NVRAM_PARAMS_INDEX_CE_16 },
    { "ce_side_band_txpwr_limit_24g_ch11", NVRAM_PARAMS_INDEX_CE_17 },
    { "ce_side_band_txpwr_limit_24g_ch12", NVRAM_PARAMS_INDEX_CE_18 },
    { "ce_side_band_txpwr_limit_24g_ch13", NVRAM_PARAMS_INDEX_CE_19 },
    { NULL,                        NVRAM_PARAMS_CE_END_INDEX_BUTT },
    /* FCC */
    { "fcc_side_band_txpwr_limit_5g_20m_0_c1", NVRAM_PARAMS_INDEX_25_C1 },
    { "fcc_side_band_txpwr_limit_5g_20m_1_c1", NVRAM_PARAMS_INDEX_26_C1 },
    { "fcc_side_band_txpwr_limit_5g_40m_0_c1", NVRAM_PARAMS_INDEX_27_C1 },
    { "fcc_side_band_txpwr_limit_5g_40m_1_c1", NVRAM_PARAMS_INDEX_28_C1 },
    { "fcc_side_band_txpwr_limit_5g_80m_0_c1", NVRAM_PARAMS_INDEX_29_C1 },
    { "fcc_side_band_txpwr_limit_5g_80m_1_c1", NVRAM_PARAMS_INDEX_30_C1 },
    { "fcc_side_band_txpwr_limit_5g_160m_c1",  NVRAM_PARAMS_INDEX_31_C1 },
    { "fcc_side_band_txpwr_limit_24g_ch1_c1",  NVRAM_PARAMS_INDEX_32_C1 },
    { "fcc_side_band_txpwr_limit_24g_ch2_c1",  NVRAM_PARAMS_INDEX_33_C1 },
    { "fcc_side_band_txpwr_limit_24g_ch3_c1",  NVRAM_PARAMS_INDEX_34_C1 },
    { "fcc_side_band_txpwr_limit_24g_ch4_c1",  NVRAM_PARAMS_INDEX_35_C1 },
    { "fcc_side_band_txpwr_limit_24g_ch5_c1",  NVRAM_PARAMS_INDEX_36_C1 },
    { "fcc_side_band_txpwr_limit_24g_ch6_c1",  NVRAM_PARAMS_INDEX_37_C1 },
    { "fcc_side_band_txpwr_limit_24g_ch7_c1",  NVRAM_PARAMS_INDEX_38_C1 },
    { "fcc_side_band_txpwr_limit_24g_ch8_c1",  NVRAM_PARAMS_INDEX_39_C1 },
    { "fcc_side_band_txpwr_limit_24g_ch9_c1",  NVRAM_PARAMS_INDEX_40_C1 },
    { "fcc_side_band_txpwr_limit_24g_ch10_c1", NVRAM_PARAMS_INDEX_41_C1 },
    { "fcc_side_band_txpwr_limit_24g_ch11_c1", NVRAM_PARAMS_INDEX_42_C1 },
    { "fcc_side_band_txpwr_limit_24g_ch12_c1", NVRAM_PARAMS_INDEX_43_C1 },
    { "fcc_side_band_txpwr_limit_24g_ch13_c1", NVRAM_PARAMS_INDEX_44_C1 },
    { NULL,                            NVRAM_PARAMS_FCC_C1_END_INDEX_BUTT },
    /* CE */
    { "ce_side_band_txpwr_limit_5g_20m_0_c1", NVRAM_PARAMS_INDEX_CE_0_C1 },
    { "ce_side_band_txpwr_limit_5g_20m_1_c1", NVRAM_PARAMS_INDEX_CE_1_C1 },
    { "ce_side_band_txpwr_limit_5g_40m_0_c1", NVRAM_PARAMS_INDEX_CE_2_C1 },
    { "ce_side_band_txpwr_limit_5g_40m_1_c1", NVRAM_PARAMS_INDEX_CE_3_C1 },
    { "ce_side_band_txpwr_limit_5g_80m_0_c1", NVRAM_PARAMS_INDEX_CE_4_C1 },
    { "ce_side_band_txpwr_limit_5g_80m_1_c1", NVRAM_PARAMS_INDEX_CE_5_C1 },
    { "ce_side_band_txpwr_limit_5g_160m_c1",  NVRAM_PARAMS_INDEX_CE_6_C1 },
    { "ce_side_band_txpwr_limit_24g_ch1_c1",  NVRAM_PARAMS_INDEX_CE_7_C1 },
    { "ce_side_band_txpwr_limit_24g_ch2_c1",  NVRAM_PARAMS_INDEX_CE_8_C1 },
    { "ce_side_band_txpwr_limit_24g_ch3_c1",  NVRAM_PARAMS_INDEX_CE_9_C1 },
    { "ce_side_band_txpwr_limit_24g_ch4_c1",  NVRAM_PARAMS_INDEX_CE_10_C1 },
    { "ce_side_band_txpwr_limit_24g_ch5_c1",  NVRAM_PARAMS_INDEX_CE_11_C1 },
    { "ce_side_band_txpwr_limit_24g_ch6_c1",  NVRAM_PARAMS_INDEX_CE_12_C1 },
    { "ce_side_band_txpwr_limit_24g_ch7_c1",  NVRAM_PARAMS_INDEX_CE_13_C1 },
    { "ce_side_band_txpwr_limit_24g_ch8_c1",  NVRAM_PARAMS_INDEX_CE_14_C1 },
    { "ce_side_band_txpwr_limit_24g_ch9_c1",  NVRAM_PARAMS_INDEX_CE_15_C1 },
    { "ce_side_band_txpwr_limit_24g_ch10_c1", NVRAM_PARAMS_INDEX_CE_16_C1 },
    { "ce_side_band_txpwr_limit_24g_ch11_c1", NVRAM_PARAMS_INDEX_CE_17_C1 },
    { "ce_side_band_txpwr_limit_24g_ch12_c1", NVRAM_PARAMS_INDEX_CE_18_C1 },
    { "ce_side_band_txpwr_limit_24g_ch13_c1", NVRAM_PARAMS_INDEX_CE_19_C1 },
    { NULL,                           NVRAM_PARAMS_CE_C1_END_INDEX_BUTT },
    /* SAR */
    { "sar_txpwr_ctrl_5g_band1_0", NVRAM_PARAMS_INDEX_45 },
    { "sar_txpwr_ctrl_5g_band2_0", NVRAM_PARAMS_INDEX_46 },
    { "sar_txpwr_ctrl_5g_band3_0", NVRAM_PARAMS_INDEX_47 },
    { "sar_txpwr_ctrl_5g_band4_0", NVRAM_PARAMS_INDEX_48 },
    { "sar_txpwr_ctrl_5g_band5_0", NVRAM_PARAMS_INDEX_49 },
    { "sar_txpwr_ctrl_5g_band6_0", NVRAM_PARAMS_INDEX_50 },
    { "sar_txpwr_ctrl_5g_band7_0", NVRAM_PARAMS_INDEX_51 },
    { "sar_txpwr_ctrl_2g_0",       NVRAM_PARAMS_INDEX_52 },
    { "sar_txpwr_ctrl_5g_band1_1", NVRAM_PARAMS_INDEX_53 },
    { "sar_txpwr_ctrl_5g_band2_1", NVRAM_PARAMS_INDEX_54 },
    { "sar_txpwr_ctrl_5g_band3_1", NVRAM_PARAMS_INDEX_55 },
    { "sar_txpwr_ctrl_5g_band4_1", NVRAM_PARAMS_INDEX_56 },
    { "sar_txpwr_ctrl_5g_band5_1", NVRAM_PARAMS_INDEX_57 },
    { "sar_txpwr_ctrl_5g_band6_1", NVRAM_PARAMS_INDEX_58 },
    { "sar_txpwr_ctrl_5g_band7_1", NVRAM_PARAMS_INDEX_59 },
    { "sar_txpwr_ctrl_2g_1",       NVRAM_PARAMS_INDEX_60 },
    { "sar_txpwr_ctrl_5g_band1_2", NVRAM_PARAMS_INDEX_61 },
    { "sar_txpwr_ctrl_5g_band2_2", NVRAM_PARAMS_INDEX_62 },
    { "sar_txpwr_ctrl_5g_band3_2", NVRAM_PARAMS_INDEX_63 },
    { "sar_txpwr_ctrl_5g_band4_2", NVRAM_PARAMS_INDEX_64 },
    { "sar_txpwr_ctrl_5g_band5_2", NVRAM_PARAMS_INDEX_65 },
    { "sar_txpwr_ctrl_5g_band6_2", NVRAM_PARAMS_INDEX_66 },
    { "sar_txpwr_ctrl_5g_band7_2", NVRAM_PARAMS_INDEX_67 },
    { "sar_txpwr_ctrl_2g_2",       NVRAM_PARAMS_INDEX_68 },
    { "sar_txpwr_ctrl_5g_band1_3", NVRAM_PARAMS_INDEX_69 },
    { "sar_txpwr_ctrl_5g_band2_3", NVRAM_PARAMS_INDEX_70 },
    { "sar_txpwr_ctrl_5g_band3_3", NVRAM_PARAMS_INDEX_71 },
    { "sar_txpwr_ctrl_5g_band4_3", NVRAM_PARAMS_INDEX_72 },
    { "sar_txpwr_ctrl_5g_band5_3", NVRAM_PARAMS_INDEX_73 },
    { "sar_txpwr_ctrl_5g_band6_3", NVRAM_PARAMS_INDEX_74 },
    { "sar_txpwr_ctrl_5g_band7_3", NVRAM_PARAMS_INDEX_75 },
    { "sar_txpwr_ctrl_2g_3",       NVRAM_PARAMS_INDEX_76 },
    { "sar_txpwr_ctrl_5g_band1_4", NVRAM_PARAMS_INDEX_77 },
    { "sar_txpwr_ctrl_5g_band2_4", NVRAM_PARAMS_INDEX_78 },
    { "sar_txpwr_ctrl_5g_band3_4", NVRAM_PARAMS_INDEX_79 },
    { "sar_txpwr_ctrl_5g_band4_4", NVRAM_PARAMS_INDEX_80 },
    { "sar_txpwr_ctrl_5g_band5_4", NVRAM_PARAMS_INDEX_81 },
    { "sar_txpwr_ctrl_5g_band6_4", NVRAM_PARAMS_INDEX_82 },
    { "sar_txpwr_ctrl_5g_band7_4", NVRAM_PARAMS_INDEX_83 },
    { "sar_txpwr_ctrl_2g_4",       NVRAM_PARAMS_INDEX_84 },
    { NULL,                NVRAM_PARAMS_SAR_END_INDEX_BUTT },
    {"sar_txpwr_ctrl_5g_band1_0_c1", NVRAM_PARAMS_INDEX_45_C1},
    {"sar_txpwr_ctrl_5g_band2_0_c1", NVRAM_PARAMS_INDEX_46_C1},
    {"sar_txpwr_ctrl_5g_band3_0_c1", NVRAM_PARAMS_INDEX_47_C1},
    {"sar_txpwr_ctrl_5g_band4_0_c1", NVRAM_PARAMS_INDEX_48_C1},
    {"sar_txpwr_ctrl_5g_band5_0_c1", NVRAM_PARAMS_INDEX_49_C1},
    {"sar_txpwr_ctrl_5g_band6_0_c1", NVRAM_PARAMS_INDEX_50_C1},
    {"sar_txpwr_ctrl_5g_band7_0_c1", NVRAM_PARAMS_INDEX_51_C1},
    {"sar_txpwr_ctrl_2g_0_c1",       NVRAM_PARAMS_INDEX_52_C1},
    {"sar_txpwr_ctrl_5g_band1_1_c1", NVRAM_PARAMS_INDEX_53_C1},
    {"sar_txpwr_ctrl_5g_band2_1_c1", NVRAM_PARAMS_INDEX_54_C1},
    {"sar_txpwr_ctrl_5g_band3_1_c1", NVRAM_PARAMS_INDEX_55_C1},
    {"sar_txpwr_ctrl_5g_band4_1_c1", NVRAM_PARAMS_INDEX_56_C1},
    {"sar_txpwr_ctrl_5g_band5_1_c1", NVRAM_PARAMS_INDEX_57_C1},
    {"sar_txpwr_ctrl_5g_band6_1_c1", NVRAM_PARAMS_INDEX_58_C1},
    {"sar_txpwr_ctrl_5g_band7_1_c1", NVRAM_PARAMS_INDEX_59_C1},
    {"sar_txpwr_ctrl_2g_1_c1",       NVRAM_PARAMS_INDEX_60_C1},
    {"sar_txpwr_ctrl_5g_band1_2_c1", NVRAM_PARAMS_INDEX_61_C1},
    {"sar_txpwr_ctrl_5g_band2_2_c1", NVRAM_PARAMS_INDEX_62_C1},
    {"sar_txpwr_ctrl_5g_band3_2_c1", NVRAM_PARAMS_INDEX_63_C1},
    {"sar_txpwr_ctrl_5g_band4_2_c1", NVRAM_PARAMS_INDEX_64_C1},
    {"sar_txpwr_ctrl_5g_band5_2_c1", NVRAM_PARAMS_INDEX_65_C1},
    {"sar_txpwr_ctrl_5g_band6_2_c1", NVRAM_PARAMS_INDEX_66_C1},
    {"sar_txpwr_ctrl_5g_band7_2_c1", NVRAM_PARAMS_INDEX_67_C1},
    {"sar_txpwr_ctrl_2g_2_c1",       NVRAM_PARAMS_INDEX_68_C1},
    {"sar_txpwr_ctrl_5g_band1_3_c1", NVRAM_PARAMS_INDEX_69_C1},
    {"sar_txpwr_ctrl_5g_band2_3_c1", NVRAM_PARAMS_INDEX_70_C1},
    {"sar_txpwr_ctrl_5g_band3_3_c1", NVRAM_PARAMS_INDEX_71_C1},
    {"sar_txpwr_ctrl_5g_band4_3_c1", NVRAM_PARAMS_INDEX_72_C1},
    {"sar_txpwr_ctrl_5g_band5_3_c1", NVRAM_PARAMS_INDEX_73_C1},
    {"sar_txpwr_ctrl_5g_band6_3_c1", NVRAM_PARAMS_INDEX_74_C1},
    {"sar_txpwr_ctrl_5g_band7_3_c1", NVRAM_PARAMS_INDEX_75_C1},
    {"sar_txpwr_ctrl_2g_3_c1",       NVRAM_PARAMS_INDEX_76_C1},
    {"sar_txpwr_ctrl_5g_band1_4_c1", NVRAM_PARAMS_INDEX_77_C1},
    {"sar_txpwr_ctrl_5g_band2_4_c1", NVRAM_PARAMS_INDEX_78_C1},
    {"sar_txpwr_ctrl_5g_band3_4_c1", NVRAM_PARAMS_INDEX_79_C1},
    {"sar_txpwr_ctrl_5g_band4_4_c1", NVRAM_PARAMS_INDEX_80_C1},
    {"sar_txpwr_ctrl_5g_band5_4_c1", NVRAM_PARAMS_INDEX_81_C1},
    {"sar_txpwr_ctrl_5g_band6_4_c1", NVRAM_PARAMS_INDEX_82_C1},
    {"sar_txpwr_ctrl_5g_band7_4_c1", NVRAM_PARAMS_INDEX_83_C1},
    {"sar_txpwr_ctrl_2g_4_c1",       NVRAM_PARAMS_INDEX_84_C1},
    {NULL,                   NVRAM_PARAMS_SAR_C1_END_INDEX_BUTT },

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    { "tas_ant_switch_en", NVRAM_PARAMS_TAS_ANT_SWITCH_EN },
    { "tas_txpwr_ctrl_params", NVRAM_PARAMS_TAS_PWR_CTRL },
#endif
    { "5g_max_pow_high_band_fcc_ce", NVRAM_PARAMS_5G_FCC_CE_HIGH_BAND_MAX_PWR },
    { "5g_iq_cali_lpf_lvl",          NVRAM_PARAMS_INDEX_IQ_LPF_LVL},

    { "cfg_tpc_ru_pow_5g_0",         NVRAM_CFG_TPC_RU_POWER_20M_5G},
    { "cfg_tpc_ru_pow_5g_1",         NVRAM_CFG_TPC_RU_POWER_40M_5G},
    { "cfg_tpc_ru_pow_5g_2",         NVRAM_CFG_TPC_RU_POWER_80M_L_5G},
    { "cfg_tpc_ru_pow_5g_3",         NVRAM_CFG_TPC_RU_POWER_80M_H_5G},
    { "cfg_tpc_ru_pow_5g_4",         NVRAM_CFG_TPC_RU_POWER_160M_L_5G},
    { "cfg_tpc_ru_pow_5g_5",         NVRAM_CFG_TPC_RU_POWER_160M_H_5G},
    { "cfg_tpc_ru_pow_2g_0",         NVRAM_CFG_TPC_RU_POWER_20M_2G},
    { "cfg_tpc_ru_pow_2g_1",         NVRAM_CFG_TPC_RU_POWER_40M_0_2G},
    { "cfg_tpc_ru_pow_2g_2",         NVRAM_CFG_TPC_RU_POWER_40M_1_2G},

    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_2g",      NVRAM_CFG_TPC_RU_MAX_POWER_0_2G_MIMO },
    { "cfg_tpc_ru_max_pow_484_mimo_2g",                NVRAM_CFG_TPC_RU_MAX_POWER_1_2G_MIMO },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_2g_c0",   NVRAM_CFG_TPC_RU_MAX_POWER_0_2G_C0 },
    { "cfg_tpc_ru_max_pow_484_siso_2g_c0",             NVRAM_CFG_TPC_RU_MAX_POWER_1_2G_C0 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_2g_c1",   NVRAM_CFG_TPC_RU_MAX_POWER_0_2G_C1 },
    { "cfg_tpc_ru_max_pow_484_siso_2g_c1",             NVRAM_CFG_TPC_RU_MAX_POWER_1_2G_C1 },

    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b1",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B1 },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b1",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B1 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b1",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B1 },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b1",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B1 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b1",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B1 },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b1",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B1 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b2",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B2 },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b2",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B2 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b2",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B2 },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b2",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B2 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b2",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B2 },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b2",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B2 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b3",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B3 },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b3",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B3 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b3",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B3 },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b3",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B3 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b3",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B3 },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b3",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B3 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b4",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B4 },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b4",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B4 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b4",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B4 },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b4",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B4 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b4",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B4 },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b4",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B4 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b5",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B5 },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b5",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B5 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b5",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B5 },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b5",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B5 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b5",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B5 },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b5",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B5 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b6",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B6 },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b6",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B6 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b6",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B6 },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b6",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B6 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b6",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B6 },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b6",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B6 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b7",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B7 },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b7",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B7 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b7",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B7 },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b7",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B7 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b7",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B7 },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b7",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B7 },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b1_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B1_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b1_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B1_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b1_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B1_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b1_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B1_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b1_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B1_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b1_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B1_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b2_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B2_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b2_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B2_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b2_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B2_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b2_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B2_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b2_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B2_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b2_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B2_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b3_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B3_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b3_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B3_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b3_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B3_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b3_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B3_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b3_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B3_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b3_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B3_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b4_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B4_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b4_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B4_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b4_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B4_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b4_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B4_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b4_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B4_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b4_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B4_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b5_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B5_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b5_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B5_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b5_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B5_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b5_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B5_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b5_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B5_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b5_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B5_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b6_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B6_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b6_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B6_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b6_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B6_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b6_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B6_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b6_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B6_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b6_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B6_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b7_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B7_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b7_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B7_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b7_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B7_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b7_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B7_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b7_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B7_CE },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b7_ce",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B7_CE },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b1_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B1_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b1_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B1_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b1_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B1_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b1_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B1_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b1_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B1_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b1_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B1_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b2_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B2_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b2_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B2_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b2_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B2_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b2_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B2_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b2_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B2_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b2_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B2_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b3_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B3_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b3_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B3_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b3_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B3_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b3_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B3_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b3_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B3_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b3_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B3_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b4_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B4_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b4_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B4_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b4_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B4_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b4_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B4_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b4_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B4_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b4_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B4_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b5_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B5_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b5_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B5_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b5_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B5_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b5_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B5_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b5_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B5_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b5_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B5_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b6_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B6_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b6_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B6_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b6_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B6_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b6_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B6_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b6_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B6_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b6_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B6_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_mimo_5g_b7_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B7_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_mimo_5g_b7_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_MIMO_B7_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c0_b7_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C0_B7_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c0_b7_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C0_B7_FCC },
    { "cfg_tpc_ru_max_pow_242_106_52_26_siso_5g_c1_b7_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_C1_B7_FCC },
    { "cfg_tpc_ru_max_pow_1992_996_484_siso_5g_c1_b7_fcc",    NVRAM_CFG_TPC_RU_MAX_POWER_1_5G_C1_B7_FCC },

    { "cfg_tpc_ru_max_pow_2g",      NVRAM_CFG_TPC_RU_MAX_POWER_2G },
    { "cfg_tpc_ru_max_pow_5g",      NVRAM_CFG_TPC_RU_MAX_POWER_5G },
};

OAL_STATIC wlan_cfg_cmd g_nvram_pro_line_config_ini[] = {
    /* 产侧nvram参数 */
    { "nvram_pa2gccka0",  WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA0 },
    { "nvram_pa2ga0",     WLAN_CFG_NVRAM_RATIO_PA2GA0 },
    { "nvram_pa2g40a0",   WLAN_CFG_DTS_NVRAM_RATIO_PA2G40A0 },
    { "nvram_pa5ga0",     WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0 },
    { "nvram_pa2gccka1",  WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA1 },
    { "nvram_pa2ga1",     WLAN_CFG_DTS_NVRAM_RATIO_PA2GA1 },
    { "nvram_pa2g40a1",   WLAN_CFG_DTS_NVRAM_RATIO_PA2G40A1 },
    { "nvram_pa5ga1",     WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1 },
    { "nvram_pa5ga0_low", WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW },
    { "nvram_pa5ga1_low", WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_LOW },

    { NULL, WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C0 },
    { NULL, WLAN_CFG_DTS_NVRAM_MUFREQ_2G20_C0 },
    { NULL, WLAN_CFG_DTS_NVRAM_MUFREQ_2G40_C0 },
    { NULL, WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C1 },
    { NULL, WLAN_CFG_DTS_NVRAM_MUFREQ_2G20_C1 },
    { NULL, WLAN_CFG_DTS_NVRAM_MUFREQ_2G40_C1 },
    { NULL, WLAN_CFG_DTS_NVRAM_MUFREQ_5G160_C0 },
    { NULL, WLAN_CFG_DTS_NVRAM_MUFREQ_5G160_C1 },

    { "nvram_pa5ga0_band1",     WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1 },
    { "nvram_pa5ga1_band1",     WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_BAND1 },
    { "nvram_pa2gcwa0",         WLAN_CFG_DTS_NVRAM_RATIO_PA2GCWA0 },
    { "nvram_pa2gcwa1",         WLAN_CFG_DTS_NVRAM_RATIO_PA2GCWA1 },
    { "nvram_pa5ga0_band1_low", WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1_LOW },
    { "nvram_pa5ga1_band1_low", WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_BAND1_LOW },

    { "nvram_ppa2gcwa0", WLAN_CFG_DTS_NVRAM_RATIO_PPA2GCWA0 },
    { "nvram_ppa2gcwa1", WLAN_CFG_DTS_NVRAM_RATIO_PPA2GCWA1 },

    { NULL, WLAN_CFG_DTS_NVRAM_PARAMS_BUTT },
};

static uint32_t hwifi_config_init_dts_main(oal_net_device_stru *cfg_net_dev);
static void *hwifi_get_nvram_params(void);
static void *hwifi_get_init_nvram_params(void);

/*
 * 函 数 名  : original_value_for_nvram_params
 * 功能描述  : 最大发射功率定制化参数初值处理
 */
OAL_STATIC void original_value_for_nvram_params(void)
{
    uint32_t uc_param_idx;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_0] = 0x0000F6F6;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_1] = 0xFBE7F1FB;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_2] = 0xE7F1F1FB;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_3] = 0xECF6F6D8;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_4] = 0xD8D8E2EC;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_5] = 0x000000E2;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_6] = 0x0000F1F6;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_7] = 0xE2ECF600;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_8] = 0xF1FBFBFB;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_9] = 0x00F1D3EA;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_10] = 0xE7EC0000;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_11] = 0xC9CED3CE;
    /*  2.4g 5g 20M mcs9 */
    g_nvram_init_params[NVRAM_PARAMS_INDEX_12] = 0xD8DDCED3;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_13] = 0xC9C9CED3;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_14] = 0x000000C4;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_15] = 0xEC000000;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_16] = 0xC9CECEE7;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_17] = 0x000000C4;
    /* DPD 打开时高阶速率功率 */
    g_nvram_init_params[NVRAM_PARAMS_INDEX_DPD_0] = 0xE2ECEC00;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_DPD_1] = 0xE2E200E2;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_DPD_2] = 0x0000C4C4;
    /* 11B和OFDM功率差 */
    g_nvram_init_params[NVRAM_PARAMS_INDEX_11B_OFDM_DELT_POW] = 0xA0A00000;
    /* 5G功率和IQ校准UPC上限值 */
    g_nvram_init_params[NVRAM_PARAMS_INDEX_IQ_MAX_UPC] = 0xD8D83030;
    for (uc_param_idx = NVRAM_PARAMS_FCC_START_INDEX;
         uc_param_idx < NVRAM_PARAMS_SAR_C1_END_INDEX_BUTT; uc_param_idx++) {
        /* FCC/CE/SAR功率认证 */
        g_nvram_init_params[uc_param_idx] = 0xFFFFFFFF;
    }
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    g_nvram_init_params[NVRAM_PARAMS_TAS_ANT_SWITCH_EN] = 0x0;
    g_nvram_init_params[NVRAM_PARAMS_TAS_PWR_CTRL] = 0x0;
#endif
    g_nvram_init_params[NVRAM_PARAMS_5G_FCC_CE_HIGH_BAND_MAX_PWR] = 0x00FA00FA;
    g_nvram_init_params[NVRAM_PARAMS_INDEX_IQ_LPF_LVL] = 0x00001111;
}

/*
 * 函 数 名  : original_value_for_dts_params
 * 功能描述  : dts定制化参数初值处理
 */
OAL_STATIC void original_value_for_dts_params(void)
{
    /* 校准 */
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN1] = WLAN_CALI_TXPWR_REF_2G_CH1_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN2] = WLAN_CALI_TXPWR_REF_2G_CH2_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN3] = WLAN_CALI_TXPWR_REF_2G_CH3_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN4] = WLAN_CALI_TXPWR_REF_2G_CH4_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN5] = WLAN_CALI_TXPWR_REF_2G_CH5_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN6] = WLAN_CALI_TXPWR_REF_2G_CH6_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN7] = WLAN_CALI_TXPWR_REF_2G_CH7_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN8] = WLAN_CALI_TXPWR_REF_2G_CH8_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN9] = WLAN_CALI_TXPWR_REF_2G_CH9_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN10] = WLAN_CALI_TXPWR_REF_2G_CH10_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN11] = WLAN_CALI_TXPWR_REF_2G_CH11_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN12] = WLAN_CALI_TXPWR_REF_2G_CH12_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN13] = WLAN_CALI_TXPWR_REF_2G_CH13_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND1] = WLAN_CALI_TXPWR_REF_5G_BAND1_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND2] = WLAN_CALI_TXPWR_REF_5G_BAND2_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND3] = WLAN_CALI_TXPWR_REF_5G_BAND3_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND4] = WLAN_CALI_TXPWR_REF_5G_BAND4_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND5] = WLAN_CALI_TXPWR_REF_5G_BAND5_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND6] = WLAN_CALI_TXPWR_REF_5G_BAND6_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND7] = WLAN_CALI_TXPWR_REF_5G_BAND7_VAL;
    g_dts_params[WLAN_CFG_DTS_CALI_TONE_AMP_GRADE] = WLAN_CALI_TONE_GRADE_AMP;
    /* DPD校准定制化 */
    g_dts_params[WLAN_CFG_DTS_DPD_CALI_CH_CORE0] = 0x641DA71,
    g_dts_params[WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX0_CORE0] = 0x11110000,
    g_dts_params[WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX1_CORE0] = 0x33222,
    g_dts_params[WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX2_CORE0] = 0x22211000,
    g_dts_params[WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX3_CORE0] = 0x2,
    g_dts_params[WLAN_CFG_DTS_DPD_CALI_20M_DEL_POW_CORE0] = 0x00000000,
    g_dts_params[WLAN_CFG_DTS_DPD_CALI_40M_DEL_POW_CORE0] = 0x00000000,
    g_dts_params[WLAN_CFG_DTS_DPD_CALI_CH_CORE1] = 0x641DA71,
    g_dts_params[WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX0_CORE1] = 0x11110000,
    g_dts_params[WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX1_CORE1] = 0x33222,
    g_dts_params[WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX2_CORE1] = 0x22211000,
    g_dts_params[WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX3_CORE1] = 0x2,
    g_dts_params[WLAN_CFG_DTS_DPD_CALI_20M_DEL_POW_CORE1] = 0x00000000,
    g_dts_params[WLAN_CFG_DTS_DPD_CALI_40M_DEL_POW_CORE1] = 0x00000000,
    g_dts_params[WLAN_CFG_DTS_DYN_CALI_DSCR_ITERVL] = 0x0;
}

/*
 * 函 数 名  : host_auto_freq_params_init
 * 功能描述  : 给定制化参数全局数组 g_host_init_params中自动调频相关参数赋初值
 *             ini文件读取失败时用初值
 */
OAL_STATIC void host_auto_freq_params_init(void)
{
    g_host_init_params[WLAN_CFG_INIT_PSS_THRESHOLD_LEVEL_0] = PPS_VALUE_0;
    g_host_init_params[WLAN_CFG_INIT_CPU_FREQ_LIMIT_LEVEL_0] = CPU_MIN_FREQ_VALUE_0;
    g_host_init_params[WLAN_CFG_INIT_DDR_FREQ_LIMIT_LEVEL_0] = DDR_MIN_FREQ_VALUE_0;
    g_host_init_params[WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_0] = FREQ_IDLE;
    g_host_init_params[WLAN_CFG_INIT_PSS_THRESHOLD_LEVEL_1] = PPS_VALUE_1;
    g_host_init_params[WLAN_CFG_INIT_CPU_FREQ_LIMIT_LEVEL_1] = CPU_MIN_FREQ_VALUE_1;
    g_host_init_params[WLAN_CFG_INIT_DDR_FREQ_LIMIT_LEVEL_1] = DDR_MIN_FREQ_VALUE_1;
    g_host_init_params[WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_1] = FREQ_MIDIUM;
    g_host_init_params[WLAN_CFG_INIT_PSS_THRESHOLD_LEVEL_2] = PPS_VALUE_2;
    g_host_init_params[WLAN_CFG_INIT_CPU_FREQ_LIMIT_LEVEL_2] = CPU_MIN_FREQ_VALUE_2;
    g_host_init_params[WLAN_CFG_INIT_DDR_FREQ_LIMIT_LEVEL_2] = DDR_MIN_FREQ_VALUE_2;
    g_host_init_params[WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_2] = FREQ_HIGHER;
    g_host_init_params[WLAN_CFG_INIT_PSS_THRESHOLD_LEVEL_3] = PPS_VALUE_3;
    g_host_init_params[WLAN_CFG_INIT_CPU_FREQ_LIMIT_LEVEL_3] = CPU_MIN_FREQ_VALUE_3;
    g_host_init_params[WLAN_CFG_INIT_DDR_FREQ_LIMIT_LEVEL_3] = DDR_MIN_FREQ_VALUE_3;
    g_host_init_params[WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_3] = FREQ_HIGHEST;
}

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
/*
 * 函 数 名  : host_amsdu_th_params_init
 * 功能描述  : 给定制化参数全局数组 g_host_init_params中amsdu聚合门限相关参数赋初值
 *             ini文件读取失败时用初值
 */
OAL_STATIC void host_amsdu_th_params_init(void)
{
    g_host_init_params[WLAN_CFG_INIT_AMPDU_AMSDU_SKB] = OAL_FALSE;
    g_host_init_params[WLAN_CFG_INIT_AMSDU_AMPDU_TH_HIGH]   = WLAN_AMSDU_AMPDU_TH_HIGH;
    g_host_init_params[WLAN_CFG_INIT_AMSDU_AMPDU_TH_MIDDLE] = WLAN_AMSDU_AMPDU_TH_MIDDLE;
    g_host_init_params[WLAN_CFG_INIT_AMSDU_AMPDU_TH_LOW]    = WLAN_AMSDU_AMPDU_TH_LOW;
}
#endif


/*
 * 函 数 名  : host_params_init_first
 * 功能描述  : 给定制化参数全局数组 g_host_init_params 附初值
 *             ini文件读取失败时用初值
 */
OAL_STATIC void host_params_performance_init(void)
{
    /* 动态绑PCIE中断 */
    g_host_init_params[WLAN_CFG_PRIV_DMA_LATENCY] = OAL_FALSE;
    g_host_init_params[WLAN_CFG_PRIV_LOCK_CPU_TH_HIGH] = 0;
    g_host_init_params[WLAN_CFG_PRIV_LOCK_CPU_TH_LOW] = 0;
    g_host_init_params[WLAN_CFG_INIT_IRQ_AFFINITY] = OAL_FALSE;
    g_host_init_params[WLAN_CFG_INIT_IRQ_TH_HIGH] = WLAN_IRQ_TH_HIGH;
    g_host_init_params[WLAN_CFG_INIT_IRQ_TH_LOW] = WLAN_IRQ_TH_LOW;
    g_host_init_params[WLAN_CFG_INIT_IRQ_PPS_TH_HIGH] = WLAN_IRQ_PPS_TH_HIGH;
    g_host_init_params[WLAN_CFG_INIT_IRQ_PPS_TH_LOW] = WLAN_IRQ_PPS_TH_LOW;
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    /* 硬件聚合定制化项 */
    g_host_init_params[WLAN_CFG_INIT_HW_AMPDU] = OAL_FALSE;
    g_host_init_params[WLAN_CFG_INIT_HW_AMPDU_TH_HIGH] = WLAN_HW_AMPDU_TH_HIGH;
    g_host_init_params[WLAN_CFG_INIT_HW_AMPDU_TH_LOW] = WLAN_HW_AMPDU_TH_LOW;
#endif

#ifdef _PRE_WLAN_TCP_OPT
    g_host_init_params[WLAN_CFG_INIT_TCP_ACK_FILTER] = OAL_FALSE;
    g_host_init_params[WLAN_CFG_INIT_TCP_ACK_FILTER_TH_HIGH] = WLAN_TCP_ACK_FILTER_TH_HIGH;
    g_host_init_params[WLAN_CFG_INIT_TCP_ACK_FILTER_TH_LOW] = WLAN_TCP_ACK_FILTER_TH_LOW;
#endif

    g_host_init_params[WLAN_CFG_INIT_TX_SMALL_AMSDU] = OAL_TRUE;
    g_host_init_params[WLAN_CFG_INIT_SMALL_AMSDU_HIGH] = WLAN_SMALL_AMSDU_HIGH;
    g_host_init_params[WLAN_CFG_INIT_SMALL_AMSDU_LOW] = WLAN_SMALL_AMSDU_LOW;
    g_host_init_params[WLAN_CFG_INIT_SMALL_AMSDU_PPS_HIGH] = WLAN_SMALL_AMSDU_PPS_HIGH;
    g_host_init_params[WLAN_CFG_INIT_SMALL_AMSDU_PPS_LOW] = WLAN_SMALL_AMSDU_PPS_LOW;

    g_host_init_params[WLAN_CFG_INIT_TX_TCP_ACK_BUF] = OAL_TRUE;
    g_host_init_params[WLAN_CFG_INIT_TCP_ACK_BUF_HIGH] = WLAN_TCP_ACK_BUF_HIGH;
    g_host_init_params[WLAN_CFG_INIT_TCP_ACK_BUF_LOW] = WLAN_TCP_ACK_BUF_LOW;
    g_host_init_params[WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_40M] = WLAN_TCP_ACK_BUF_HIGH_40M;
    g_host_init_params[WLAN_CFG_INIT_TCP_ACK_BUF_LOW_40M] = WLAN_TCP_ACK_BUF_LOW_40M;
    g_host_init_params[WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_80M] = WLAN_TCP_ACK_BUF_HIGH_80M;
    g_host_init_params[WLAN_CFG_INIT_TCP_ACK_BUF_LOW_80M] = WLAN_TCP_ACK_BUF_LOW_80M;
    g_host_init_params[WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_160M] = WLAN_TCP_ACK_BUF_HIGH_160M;
    g_host_init_params[WLAN_CFG_INIT_TCP_ACK_BUF_LOW_160M] = WLAN_TCP_ACK_BUF_LOW_160M;

    g_host_init_params[WLAN_CFG_INIT_TX_TCP_ACK_BUF_USERCTL] = OAL_FALSE;
    g_host_init_params[WLAN_CFG_INIT_TCP_ACK_BUF_USERCTL_HIGH] = WLAN_TCP_ACK_BUF_USERCTL_HIGH;
    g_host_init_params[WLAN_CFG_INIT_TCP_ACK_BUF_USERCTL_LOW] = WLAN_TCP_ACK_BUF_USERCTL_LOW;

    g_host_init_params[WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA] = OAL_FALSE;
    g_host_init_params[WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_HIGH] = WLAN_RX_DYN_BYPASS_EXTLNA_HIGH;
    g_host_init_params[WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_LOW] = WLAN_RX_DYN_BYPASS_EXTLNA_LOW;
    g_host_init_params[WLAN_CFG_INIT_RX_AMPDU_AMSDU_SKB] = OAL_FALSE;
    g_host_init_params[WLAN_CFG_INIT_RX_AMPDU_BITMAP] = OAL_FALSE;
    g_host_init_params[WLAN_CFG_INIT_HIEX_CAP] = WLAN_HIEX_DEV_CAP;
    g_host_init_params[WLAN_CFG_INIT_SDIO_D2H_ASSEMBLE_COUNT] = HISDIO_DEV2HOST_SCATT_MAX;
    g_host_init_params[WLAN_CFG_INIT_SDIO_H2D_ASSEMBLE_COUNT] = WLAN_SDIO_H2D_ASSEMBLE_COUNT_VAL;
    g_host_init_params[WLAN_CFG_INIT_FTM_CAP] = OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    g_host_init_params[WLAN_CFG_INIT_MCAST_AMPDU_ENABLE] = OAL_FALSE;
#endif
}
/*
 * 功能描述  : 给定制化参数全局数组 g_host_init_params 附初值
 *             ini文件读取失败时用初值
 */
OAL_STATIC void host_btcoex_rssi_th_params_init(void)
{
    /* ldac m2s rssi */
    g_host_init_params[WLAN_CFG_INIT_LDAC_THRESHOLD_M2S] = WLAN_BTCOEX_THRESHOLD_MCM_DOWN; /* 默认最大门限，不支持 */
    g_host_init_params[WLAN_CFG_INIT_LDAC_THRESHOLD_S2M] = WLAN_BTCOEX_THRESHOLD_MCM_UP;

    /* mcm btcoex rssi */
    g_host_init_params[WLAN_CFG_INIT_BTCOEX_THRESHOLD_MCM_DOWN] = WLAN_BTCOEX_THRESHOLD_MCM_DOWN; /* 默认最大门限，不支持 */
    g_host_init_params[WLAN_CFG_INIT_BTCOEX_THRESHOLD_MCM_UP] = WLAN_BTCOEX_THRESHOLD_MCM_UP;
}

static void host_param_init_netaccess_param(void)
{
    /* ROAM */
    g_host_init_params[WLAN_CFG_INIT_ROAM_SWITCH] = WLAN_ROAM_SWITCH_MODE;
    g_host_init_params[WLAN_CFG_INIT_SCAN_ORTHOGONAL] = WLAN_SCAN_ORTHOGONAL_VAL;
    g_host_init_params[WLAN_CFG_INIT_TRIGGER_B] = WLAN_TRIGGER_B_VAL;
    g_host_init_params[WLAN_CFG_INIT_TRIGGER_A] = WLAN_TRIGGER_A_VAL;
    g_host_init_params[WLAN_CFG_INIT_DELTA_B] = WLAN_DELTA_B_VAL;
    g_host_init_params[WLAN_CFG_INIT_DELTA_A] = WLAN_DELTA_A_VAL;
}

static void host_param_init_performance_param(void)
{
    /* 性能 */
    g_host_init_params[WLAN_CFG_INIT_AMPDU_TX_MAX_NUM] = WLAN_AMPDU_TX_MAX_NUM;
    g_host_init_params[WLAN_CFG_INIT_USED_MEM_FOR_START] = WLAN_MEM_FOR_START;
    g_host_init_params[WLAN_CFG_INIT_USED_MEM_FOR_STOP] = WLAN_MEM_FOR_STOP;
    g_host_init_params[WLAN_CFG_INIT_RX_ACK_LIMIT] = WLAN_RX_ACK_LIMIT_VAL;
    /* LINKLOSS */
    g_host_init_params[WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_BT] = WLAN_LOSS_THRESHOLD_WLAN_BT;
    g_host_init_params[WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_DBAC] = WLAN_LOSS_THRESHOLD_WLAN_DBAC;
    g_host_init_params[WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_NORMAL] = WLAN_LOSS_THRESHOLD_WLAN_NORMAL;
}

static void host_param_init_rf_param(void)
{
    uint8_t param_idx;

    /* 2G RF前端 */
    g_host_init_params[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND1] = 0xF4F4;
    g_host_init_params[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND2] = 0xF4F4;
    g_host_init_params[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND3] = 0xF4F4;
    /* 5G RF前端 */
    g_host_init_params[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND1] = 0xF8F8;
    g_host_init_params[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND2] = 0xF8F8;
    g_host_init_params[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND3] = 0xF8F8;
    g_host_init_params[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND4] = 0xF8F8;
    g_host_init_params[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND5] = 0xF8F8;
    g_host_init_params[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND6] = 0xF8F8;
    g_host_init_params[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND7] = 0xF8F8;
    /* fem */
    g_host_init_params[WLAN_CFG_INIT_RF_LNA_BYPASS_GAIN_DB_2G] = 0xFFF4FFF4;
    g_host_init_params[WLAN_CFG_INIT_RF_LNA_GAIN_DB_2G] = 0x00140014;
    g_host_init_params[WLAN_CFG_INIT_RF_PA_GAIN_DB_B0_2G] = 0xFFF4FFF4;
    g_host_init_params[WLAN_CFG_INIT_RF_PA_GAIN_DB_B1_2G] = 0xFFF4FFF4;
    g_host_init_params[WLAN_CFG_INIT_RF_PA_GAIN_LVL_2G] = 0x00010001;
    g_host_init_params[WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_2G] = 0x00010001;
    g_host_init_params[WLAN_CFG_INIT_EXT_PA_ISEXIST_2G] = 0x00010001;
    g_host_init_params[WLAN_CFG_INIT_EXT_LNA_ISEXIST_2G] = 0x00010001;
    g_host_init_params[WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_2G] = 0x02760276;
    g_host_init_params[WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_2G] = 0x01400140;
    g_host_init_params[WLAN_CFG_INIT_RF_LNA_BYPASS_GAIN_DB_5G] = 0xFFF4FFF4;
    g_host_init_params[WLAN_CFG_INIT_RF_LNA_GAIN_DB_5G] = 0x00140014;
    g_host_init_params[WLAN_CFG_INIT_RF_PA_GAIN_DB_B0_5G] = 0xFFF4FFF4;
    g_host_init_params[WLAN_CFG_INIT_RF_PA_GAIN_DB_B1_5G] = 0xFFF4FFF4;
    g_host_init_params[WLAN_CFG_INIT_RF_PA_GAIN_LVL_5G] = 0x00010001;
    g_host_init_params[WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_5G] = 0x00010001;
    g_host_init_params[WLAN_CFG_INIT_EXT_PA_ISEXIST_5G] = 0x00010001;
    g_host_init_params[WLAN_CFG_INIT_EXT_LNA_ISEXIST_5G] = 0x00010001;
    g_host_init_params[WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_5G] = 0x02760276;
    g_host_init_params[WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_5G] = 0x01400140;
    /* 用于定制化计算PWR RF值的偏差 */
    for (param_idx = WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C0_MULT4;
         param_idx <= WLAN_CFG_INIT_RF_AMEND_RSSI_5G_C1; param_idx++) {
        g_host_init_params[param_idx] = 0;
    }
}

static void host_param_init_keepalive_param(void)
{
    /* sta keepalive cnt th */
    g_host_init_params[WLAN_CFG_INIT_STA_KEEPALIVE_CNT_TH] = WLAN_STA_KEEPALIVE_CNT_TH;
    g_host_init_params[WLAN_CFG_INIT_FAR_DIST_POW_GAIN_SWITCH] = 1;
    g_host_init_params[WLAN_CFG_INIT_FAR_DIST_DSSS_SCALE_PROMOTE_SWITCH] = 1;
    g_host_init_params[WLAN_CFG_INIT_CHANN_RADIO_CAP] = 0xF;

    g_host_init_params[WLAN_CFG_LTE_GPIO_CHECK_SWITCH] = 0;
    g_host_init_params[WLAN_ATCMDSRV_ISM_PRIORITY] = 0;
    g_host_init_params[WLAN_ATCMDSRV_LTE_RX] = 0;
    g_host_init_params[WLAN_ATCMDSRV_LTE_TX] = 0;
    g_host_init_params[WLAN_ATCMDSRV_LTE_INACT] = 0;
    g_host_init_params[WLAN_ATCMDSRV_ISM_RX_ACT] = 0;
    g_host_init_params[WLAN_ATCMDSRV_BANT_PRI] = 0;
    g_host_init_params[WLAN_ATCMDSRV_BANT_STATUS] = 0;
    g_host_init_params[WLAN_ATCMDSRV_WANT_PRI] = 0;
    g_host_init_params[WLAN_ATCMDSRV_WANT_STATUS] = 0;
    g_host_init_params[WLAN_TX5G_UPC_MIX_GAIN_CTRL_1] = 0;
    g_host_init_params[WLAN_TX5G_UPC_MIX_GAIN_CTRL_2] = 0;
    g_host_init_params[WLAN_TX5G_UPC_MIX_GAIN_CTRL_3] = 0;
    g_host_init_params[WLAN_TX5G_UPC_MIX_GAIN_CTRL_4] = 0;
    g_host_init_params[WLAN_TX5G_UPC_MIX_GAIN_CTRL_5] = 0;
    g_host_init_params[WLAN_TX5G_UPC_MIX_GAIN_CTRL_6] = 0;
    g_host_init_params[WLAN_TX5G_UPC_MIX_GAIN_CTRL_7] = 0;
}

static void host_param_init_pa_gate_param(void)
{
    /* PA bias */
    g_host_init_params[WLAN_TX2G_PA_GATE_VCTL_REG236] = 0x12081208;
    g_host_init_params[WLAN_TX2G_PA_GATE_VCTL_REG237] = 0x2424292D;
    g_host_init_params[WLAN_TX2G_PA_GATE_VCTL_REG238] = 0x24242023;
    g_host_init_params[WLAN_TX2G_PA_GATE_VCTL_REG239] = 0x24242020;
    g_host_init_params[WLAN_TX2G_PA_GATE_VCTL_REG240] = 0x24242020;
    g_host_init_params[WLAN_TX2G_PA_GATE_VCTL_REG241] = 0x24241B1B;
    g_host_init_params[WLAN_TX2G_PA_GATE_VCTL_REG242] = 0x24241B1B;
    g_host_init_params[WLAN_TX2G_PA_GATE_VCTL_REG243] = 0x24241B1B;
    g_host_init_params[WLAN_TX2G_PA_GATE_VCTL_REG244] = 0x24241B1B;

    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG253] = 0x14141414;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG254] = 0x13131313;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG255] = 0x12121212;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG256] = 0x12121212;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG257] = 0x12121212;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG258] = 0x12121212;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG259] = 0x12121212;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG260] = 0x12121212;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG261] = 0x0F0F0F0F;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG262] = 0x0D0D0D0D;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG263] = 0x0A0B0A0B;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG264] = 0x0A0A0A0A;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG265] = 0x0A0A0A0A;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG266] = 0x0A0A0A0A;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG267] = 0x0A0A0A0A;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG268] = 0x0A0A0A0A;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG269] = 0x0F0F0F0F;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG270] = 0x0D0D0D0D;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG271] = 0x0A0B0A0B;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG272] = 0x0A0A0A0A;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG273] = 0x0A0A0A0A;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG274] = 0x0A0A0A0A;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG275] = 0x0A0A0A0A;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG276] = 0x0A0A0A0A;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG277] = 0x0D0D0D0D;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG278] = 0x0D0D0D0D;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG279] = 0x0D0D0A0B;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG280_BAND1] = 0x0D0D0A0A;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG280_BAND2] = 0x0D0D0A0A;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG280_BAND3] = 0x0D0D0A0A;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG281] = 0x0D0D0A0A;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG282] = 0x0D0D0A0A;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG283] = 0x0D0D0A0A;
    g_host_init_params[WLAN_TX2G_PA_VRECT_GATE_THIN_REG284] = 0x0D0D0A0A;
}

#ifdef _PRE_WLAN_FEATURE_NRCOEX
static void host_param_init_nrcoex_param(void)
{
    /* 5g nr coex */
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_ENABLE]                  = 0;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE0_FREQ]              = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE0_40M_20M_GAP0]      = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE0_160M_80M_GAP0]     = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE0_40M_20M_GAP1]      = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE0_160M_80M_GAP1]     = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE0_40M_20M_GAP2]      = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE0_160M_80M_GAP2]     = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE0_SMALLGAP0_ACT]     = 0xFFFFFFFF;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE0_GAP01_ACT]         = 0xFFFFFFFF;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE0_GAP12_ACT]         = 0xFFFFFFFF;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE0_RXSLOT_RSSI]       = 0xFFFFFFFF;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE1_FREQ]              = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE1_40M_20M_GAP0]      = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE1_160M_80M_GAP0]     = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE1_40M_20M_GAP1]      = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE1_160M_80M_GAP1]     = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE1_40M_20M_GAP2]      = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE1_160M_80M_GAP2]     = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE1_SMALLGAP0_ACT]     = 0xFFFFFFFF;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE1_GAP01_ACT]         = 0xFFFFFFFF;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE1_GAP12_ACT]         = 0xFFFFFFFF;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE1_RXSLOT_RSSI]       = 0xFFFFFFFF;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE2_FREQ]              = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE2_40M_20M_GAP0]      = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE2_160M_80M_GAP0]     = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE2_40M_20M_GAP1]      = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE2_160M_80M_GAP1]     = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE2_40M_20M_GAP2]      = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE2_160M_80M_GAP2]     = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE2_SMALLGAP0_ACT]     = 0xFFFFFFFF;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE2_GAP01_ACT]         = 0xFFFFFFFF;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE2_GAP12_ACT]         = 0xFFFFFFFF;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE2_RXSLOT_RSSI]       = 0xFFFFFFFF;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE3_FREQ]              = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE3_40M_20M_GAP0]      = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE3_160M_80M_GAP0]     = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE3_40M_20M_GAP1]      = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE3_160M_80M_GAP1]     = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE3_40M_20M_GAP2]      = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE3_160M_80M_GAP2]     = 0x00000000;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE3_SMALLGAP0_ACT]     = 0xFFFFFFFF;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE3_GAP01_ACT]         = 0xFFFFFFFF;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE3_GAP12_ACT]         = 0xFFFFFFFF;
    g_host_init_params[WLAN_CFG_INIT_NRCOEX_RULE3_RXSLOT_RSSI]       = 0xFFFFFFFF;
}
#endif

/*
 * 函 数 名  : host_params_init_first
 * 功能描述  : 给定制化参数全局数组 g_host_init_params 附初值
 *             ini文件读取失败时用初值
 */
OAL_STATIC void host_params_init_first(void)
{
    host_param_init_netaccess_param();

    host_param_init_performance_param();
    /* 自动调频 */
    host_auto_freq_params_init();

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    host_amsdu_th_params_init();
#endif

    host_params_performance_init();

    /* 低功耗 */
    g_host_init_params[WLAN_CFG_INIT_POWERMGMT_SWITCH] = OAL_TRUE;
    g_host_init_params[WLAN_CFG_INIT_PS_MODE] = WLAN_PS_MODE;
    g_host_init_params[WLAN_CFG_INIT_MIN_FAST_PS_IDLE] = WLAN_MIN_FAST_PS_IDLE;
    g_host_init_params[WLAN_CFG_INIT_MAX_FAST_PS_IDLE] = WLAN_MAX_FAST_PS_IDLE;
    g_host_init_params[WLAN_CFG_INIT_AUTO_FAST_PS_THRESH_SCREENON] = WLAN_AUTO_FAST_PS_SCREENON;
    g_host_init_params[WLAN_CFG_INIT_AUTO_FAST_PS_THRESH_SCREENOFF] = WLAN_AUTO_FAST_PS_SCREENOFF;

    /* 可维可测 */
    /* 日志级别 */
    g_host_init_params[WLAN_CFG_INIT_LOGLEVEL] = OAM_LOG_LEVEL_WARNING;

    host_param_init_rf_param();

    /* SCAN */
    g_host_init_params[WLAN_CFG_INIT_RANDOM_MAC_ADDR_SCAN] = 1;
    /* 11AC2G */
    g_host_init_params[WLAN_CFG_INIT_11AC2G_ENABLE] = 1;
    g_host_init_params[WLAN_CFG_INIT_DISABLE_CAPAB_2GHT40] = 0;
    g_host_init_params[WLAN_CFG_INIT_DUAL_ANTENNA_ENABLE] = 0;

    /* miracast */
    g_host_init_params[WLAN_CFG_INIT_PROBE_RESP_MODE] = 0x10;
    g_host_init_params[WLAN_CFG_INIT_MIRACAST_SINK_ENABLE] = 0;
    g_host_init_params[WLAN_CFG_INIT_REDUCE_MIRACAST_LOG] = 0;
    g_host_init_params[WLAN_CFG_INIT_CORE_BIND_CAP] = 1;
    g_host_init_params[WLAN_CFG_INIT_FAST_MODE] = 0;

    host_param_init_keepalive_param();

    host_param_init_pa_gate_param();

    host_btcoex_rssi_th_params_init();
    /* connect */
    g_host_init_params[WLAN_CFG_INIT_DDR_FREQ] = WLAN_DDR_CAHNL_FREQ;

#ifdef _PRE_WLAN_FEATURE_NRCOEX
    host_param_init_nrcoex_param();
#endif
}

/*
 * 函 数 名  : hwifi_custom_adapt_device_ini_freq_param
 * 功能描述  : 初始化device侧定制化ini频率相关的配置项
 */
OAL_STATIC int32_t hwifi_custom_adapt_device_ini_freq_param(uint8_t *puc_data, uint32_t *pul_data_len)
{
    config_device_freq_h2d_stru st_device_freq_data = {0};
    uint8_t uc_index;
    int32_t l_val;
    uint32_t cfg_id;
    int32_t l_ret;
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};
    const uint32_t dev_data_buff_size = sizeof(st_device_freq_data.st_device_data) /
                                             sizeof(st_device_freq_data.st_device_data[0]);

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_freq_param puc_data is NULL last data_len[%d].}",
                       *pul_data_len);
        return INI_FAILED;
    }

    st_syn_msg.en_syn_id = CUSTOM_CFGID_INI_FREQ_ID;

    for (uc_index = 0, cfg_id = WLAN_CFG_INIT_PSS_THRESHOLD_LEVEL_0; uc_index < dev_data_buff_size; uc_index++) {
        l_val = hwifi_get_init_value(CUS_TAG_INI, cfg_id);
        if (PPS_VALUE_0 <= l_val && l_val <= PPS_VALUE_3) {
            st_device_freq_data.st_device_data[uc_index].speed_level = (uint32_t)l_val;
            cfg_id += WLAN_CFG_ID_OFFSET;
        } else {
            oam_error_log1(0, OAM_SF_CFG,
                           "{hwifi_custom_adapt_device_ini_freq_param get wrong PSS_THRESHOLD_LEVEL[%d]!}", l_val);
            return OAL_FALSE;
        }
    }

    for (uc_index = 0, cfg_id = WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_0; uc_index < dev_data_buff_size; uc_index++) {
        l_val = hwifi_get_init_value(CUS_TAG_INI, cfg_id);
        if (l_val >= FREQ_IDLE && l_val <= FREQ_HIGHEST) {
            st_device_freq_data.st_device_data[uc_index].cpu_freq_level = (uint32_t)l_val;
            cfg_id++;
        } else {
            oam_error_log1(0, OAM_SF_CFG,
                           "{hwifi_custom_adapt_device_ini_freq_param get wrong DEVICE_TYPE_LEVEL [%d]!}", l_val);
            return OAL_FALSE;
        }
    }
    st_device_freq_data.uc_set_type = FREQ_SYNC_DATA;

    st_syn_msg.len = sizeof(st_device_freq_data);

    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_data_len), &st_syn_msg, CUSTOM_MSG_DATA_HDR_LEN);
    l_ret += memcpy_s(puc_data + CUSTOM_MSG_DATA_HDR_LEN,
                      (WLAN_LARGE_NETBUF_SIZE - *pul_data_len - CUSTOM_MSG_DATA_HDR_LEN),
                      &st_device_freq_data,
                      sizeof(st_device_freq_data));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_freq_param::memcpy_s fail[%d]. data_len[%d]}",
                       l_ret, *pul_data_len);
        *pul_data_len += (st_syn_msg.len + CUSTOM_MSG_DATA_HDR_LEN);
        return OAL_FAIL;
    }

    *pul_data_len += (st_syn_msg.len + CUSTOM_MSG_DATA_HDR_LEN);
    oam_warning_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_freq_param da_len[%d].}", *pul_data_len);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hwifi_custom_adapt_ini_device_perf_param
 * 功能描述  : 性能device定制化参数初始化
 */
OAL_STATIC void hwifi_custom_adapt_device_ini_perf_param(uint8_t *puc_data, uint32_t *pul_data_len)
{
    const uint32_t itoa_len = 5; /* 整数转字符串之后的长度 */
    int8_t ac_tmp[8];            /* 整数转字符串之后的buff为最大8B */
    uint8_t uc_sdio_assem_h2d;
    uint8_t uc_sdio_assem_d2h;
    int32_t l_ret;
    config_device_perf_h2d_stru st_device_perf;
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_perf_param puc_data is NULL last data_len[%d].}",
                       *pul_data_len);
        return;
    }

    memset_s(ac_tmp, sizeof(ac_tmp), 0, sizeof(ac_tmp));
    memset_s(&st_device_perf, sizeof(st_device_perf), 0, sizeof(st_device_perf));

    st_syn_msg.en_syn_id = CUSTOM_CFGID_INI_PERF_ID;

    /* SDIO FLOWCTRL */
    // device侧做合法性判断
    oal_itoa(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_USED_MEM_FOR_START),
             st_device_perf.ac_used_mem_param, itoa_len);
    oal_itoa(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_USED_MEM_FOR_STOP), ac_tmp, itoa_len);
    st_device_perf.ac_used_mem_param[OAL_STRLEN(st_device_perf.ac_used_mem_param)] = ' ';
    l_ret = memcpy_s(st_device_perf.ac_used_mem_param + OAL_STRLEN(st_device_perf.ac_used_mem_param),
                     (sizeof(st_device_perf.ac_used_mem_param) - OAL_STRLEN(st_device_perf.ac_used_mem_param)),
                     ac_tmp, OAL_STRLEN(ac_tmp));
    if (l_ret != EOK) {
        oam_error_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_perf_param memcpy_s failed[%d].}", l_ret);
        return;
    }

    st_device_perf.ac_used_mem_param[OAL_STRLEN(st_device_perf.ac_used_mem_param)] = '\0';

    /* SDIO ASSEMBLE COUNT:H2D */
    uc_sdio_assem_h2d = (uint8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_SDIO_H2D_ASSEMBLE_COUNT);
    // 判断值的合法性
    if (uc_sdio_assem_h2d >= 1 && uc_sdio_assem_h2d <= HISDIO_HOST2DEV_SCATT_MAX) {
        g_hcc_assemble_count = uc_sdio_assem_h2d;
    } else {
        oam_error_log2(0, OAM_SF_ANY, "{hwifi_custom_adapt_device_ini_perf_param::sdio_assem_h2d[%d] out of range(0,%d]\
            check value in ini file!}\r\n", uc_sdio_assem_h2d, HISDIO_HOST2DEV_SCATT_MAX);
    }

    /* SDIO ASSEMBLE COUNT:D2H */
    uc_sdio_assem_d2h = (uint8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_SDIO_D2H_ASSEMBLE_COUNT);
    // 判断值的合法性
    if (uc_sdio_assem_d2h >= 1 && uc_sdio_assem_d2h <= HISDIO_DEV2HOST_SCATT_MAX) {
        st_device_perf.uc_sdio_assem_d2h = uc_sdio_assem_d2h;
    } else {
        st_device_perf.uc_sdio_assem_d2h = HISDIO_DEV2HOST_SCATT_MAX;
        oam_error_log2(0, OAM_SF_ANY, "{hwifi_custom_adapt_device_ini_perf_param::sdio_assem_d2h[%d] out of range(0,%d]\
            check value in ini file!}\r\n", uc_sdio_assem_d2h, HISDIO_DEV2HOST_SCATT_MAX);
    }

    st_syn_msg.len = sizeof(st_device_perf);

    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_data_len), &st_syn_msg, CUSTOM_MSG_DATA_HDR_LEN);
    l_ret += memcpy_s(puc_data + CUSTOM_MSG_DATA_HDR_LEN,
                      (WLAN_LARGE_NETBUF_SIZE - *pul_data_len - CUSTOM_MSG_DATA_HDR_LEN),
                      &st_device_perf, sizeof(st_device_perf));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_perf_param::memcpy_s fail[%d]. data_len[%d]}",
                       l_ret, *pul_data_len);
        *pul_data_len += (sizeof(st_device_perf) + CUSTOM_MSG_DATA_HDR_LEN);
        return;
    }

    *pul_data_len += (sizeof(st_device_perf) + CUSTOM_MSG_DATA_HDR_LEN);

    oam_warning_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_perf_param::da_len[%d].}", *pul_data_len);
}

/*
 * 函 数 名  : hwifi_custom_adapt_device_ini_end_param
 * 功能描述  : 配置定制化参数结束标志
 */
OAL_STATIC void hwifi_custom_adapt_device_ini_end_param(uint8_t *puc_data, uint32_t *pul_data_len)
{
    int32_t l_ret;
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_end_param:NULL data_len[%d].}", *pul_data_len);
        return;
    }

    st_syn_msg.en_syn_id = CUSTOM_CFGID_INI_ENDING_ID;
    st_syn_msg.len = 0;

    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_data_len), &st_syn_msg, sizeof(st_syn_msg));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_end_param::memcpy_s fail[%d]. data_len[%d]}",
                       l_ret, *pul_data_len);
        *pul_data_len += sizeof(st_syn_msg);
        return;
    }

    *pul_data_len += sizeof(st_syn_msg);

    oam_warning_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_end_param::da_len[%d].}", *pul_data_len);
}

/*
 * 函 数 名  : hwifi_custom_adapt_device_ini_linkloss_param
 * 功能描述  : linkloss门限定制化
 */
OAL_STATIC void hwifi_custom_adapt_device_ini_linkloss_param(uint8_t *puc_data, uint32_t *pul_data_len)
{
    uint8_t ast_threshold[WLAN_LINKLOSS_MODE_BUTT] = {0};
    int32_t  l_ret;

    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_linkloss_param::puc_data is NULL data_len[%d].}",
                       *pul_data_len);
        return;
    }

    st_syn_msg.en_syn_id = CUSTOM_CFGID_INI_LINKLOSS_ID;

    ast_threshold[WLAN_LINKLOSS_MODE_BT] =
        (uint8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_BT);
    ast_threshold[WLAN_LINKLOSS_MODE_DBAC] =
        (uint8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_DBAC);
    ast_threshold[WLAN_LINKLOSS_MODE_NORMAL] =
        (uint8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_NORMAL);

    st_syn_msg.len = sizeof(ast_threshold);

    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_data_len), &st_syn_msg, CUSTOM_MSG_DATA_HDR_LEN);
    l_ret += memcpy_s(puc_data + CUSTOM_MSG_DATA_HDR_LEN,
                      (WLAN_LARGE_NETBUF_SIZE - *pul_data_len - CUSTOM_MSG_DATA_HDR_LEN),
                      &ast_threshold, sizeof(ast_threshold));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_ini_linkloss_param::memcpy_s fail[%d]. data_len[%d]}", l_ret, *pul_data_len);
        *pul_data_len += (sizeof(ast_threshold) + CUSTOM_MSG_DATA_HDR_LEN);
        return;
    }

    *pul_data_len += (sizeof(ast_threshold) + CUSTOM_MSG_DATA_HDR_LEN);

    oam_warning_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_linkloss_param::da_len[%d].}", *pul_data_len);
}

/*
 * 函 数 名  : hwifi_custom_adapt_device_ini_ldac_m2s_rssi_param
 * 功能描述  : ldac m2s rssi门限定制化
 */
OAL_STATIC void hwifi_custom_adapt_device_ini_ldac_m2s_rssi_param(uint8_t *puc_data, uint32_t *pul_data_len)
{
    int8_t ast_ldac_m2s_rssi_threshold[WLAN_M2S_LDAC_RSSI_BUTT] = { 0, 0 }; /* 当前m2s和s2m门限，后续扩展再添加枚举 */
    int32_t l_ret;

    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_ini_ldac_m2s_rssi_param::puc_data is NULL data_len[%d].}", *pul_data_len);
        return;
    }

    st_syn_msg.en_syn_id = CUSTOM_CFGID_PRIV_INI_LDAC_M2S_TH_ID;

    ast_ldac_m2s_rssi_threshold[WLAN_M2S_LDAC_RSSI_TO_SISO] =
        (int8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LDAC_THRESHOLD_M2S);
    ast_ldac_m2s_rssi_threshold[WLAN_M2S_LDAC_RSSI_TO_MIMO] =
        (int8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LDAC_THRESHOLD_S2M);

    st_syn_msg.len = sizeof(ast_ldac_m2s_rssi_threshold);

    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_data_len), &st_syn_msg, CUSTOM_MSG_DATA_HDR_LEN);
    l_ret += memcpy_s(puc_data + CUSTOM_MSG_DATA_HDR_LEN,
                      (WLAN_LARGE_NETBUF_SIZE - *pul_data_len - CUSTOM_MSG_DATA_HDR_LEN),
                      &ast_ldac_m2s_rssi_threshold,
                      sizeof(ast_ldac_m2s_rssi_threshold));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_ini_ldac_m2s_rssi_param:memcpy_s fail[%d],data_len[%d]}", l_ret, *pul_data_len);
        *pul_data_len += (sizeof(ast_ldac_m2s_rssi_threshold) + CUSTOM_MSG_DATA_HDR_LEN);
        return;
    }

    *pul_data_len += (sizeof(ast_ldac_m2s_rssi_threshold) + CUSTOM_MSG_DATA_HDR_LEN);

    oam_warning_log3(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_ldac_m2s_rssi_param::da_len[%d], \
        m2s[%d], s2m[%d].}", *pul_data_len,
        ast_ldac_m2s_rssi_threshold[WLAN_M2S_LDAC_RSSI_TO_SISO],
        ast_ldac_m2s_rssi_threshold[WLAN_M2S_LDAC_RSSI_TO_MIMO]);
}

/*
 * 函 数 名  : hwifi_custom_adapt_device_ini_btcoex_mcm_rssi_param
 * 功能描述  : btcoex mcm rssi门限定制化
 */
OAL_STATIC void hwifi_custom_adapt_device_ini_btcoex_mcm_rssi_param(uint8_t *puc_data, uint32_t *pul_data_len)
{
    int8_t ast_ldac_m2s_rssi_threshold[WLAN_BTCOEX_RSSI_MCM_BUTT] = { 0, 0 }; /* 当前m2s和s2m门限，后续扩展再添加枚举 */
    int32_t ret;

    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_ini_btcoex_mcm_rssi_param::puc_data is NULL data_len[%d].}", *pul_data_len);
        return;
    }

    st_syn_msg.en_syn_id = CUSTOM_CFGID_PRIV_INI_BTCOEX_MCM_TH_ID;

    ast_ldac_m2s_rssi_threshold[WLAN_BTCOEX_RSSI_MCM_DOWN] =
        (int8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_BTCOEX_THRESHOLD_MCM_DOWN);
    ast_ldac_m2s_rssi_threshold[WLAN_BTCOEX_RSSI_MCM_UP] =
        (int8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_BTCOEX_THRESHOLD_MCM_UP);

    st_syn_msg.len = sizeof(ast_ldac_m2s_rssi_threshold);

    ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_data_len), &st_syn_msg, CUSTOM_MSG_DATA_HDR_LEN);
    ret += memcpy_s(puc_data + CUSTOM_MSG_DATA_HDR_LEN,
        (WLAN_LARGE_NETBUF_SIZE - *pul_data_len - CUSTOM_MSG_DATA_HDR_LEN),
        &ast_ldac_m2s_rssi_threshold, sizeof(ast_ldac_m2s_rssi_threshold));
    if (ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_ini_btcoex_mcm_rssi_param:memcpy_s fail[%d],data_len[%d]}", ret, *pul_data_len);
        *pul_data_len += (sizeof(ast_ldac_m2s_rssi_threshold) + CUSTOM_MSG_DATA_HDR_LEN);
        return;
    }

    *pul_data_len += (sizeof(ast_ldac_m2s_rssi_threshold) + CUSTOM_MSG_DATA_HDR_LEN);

    oam_warning_log3(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_btcoex_mcm_rssi_param::da_len[%d], \
        mcm down[%d], mcm up[%d].}", *pul_data_len, ast_ldac_m2s_rssi_threshold[WLAN_BTCOEX_RSSI_MCM_DOWN],
        ast_ldac_m2s_rssi_threshold[WLAN_BTCOEX_RSSI_MCM_UP]);
}

#ifdef _PRE_WLAN_FEATURE_NRCOEX
/*
 * 函 数 名  : hwifi_custom_adapt_device_ini_nrcoex_param
 * 功能描述  : nr coex干扰表定制化
*/
OAL_STATIC void hwifi_custom_adapt_device_ini_nrcoex_param(uint8_t *puc_data, uint32_t *pul_data_len)
{
    hmac_to_dmac_cfg_custom_data_stru   st_syn_msg = {0};
    nrcoex_cfg_info_stru st_nrcoex_ini_info = {0};
    int32_t*           p_tmp_value = &st_nrcoex_ini_info.un_nrcoex_rule_data[0].st_nrcoex_rule_data.freq;
    uint32_t           cfg_id;
    int32_t            l_ret;

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_ini_nrcoex_param::puc_data is NULL data_len[%d].}", *pul_data_len);
        return;
    }

    st_syn_msg.en_syn_id = CUSTOM_CFGID_PRIV_INI_NRCOEX_ID;

    st_nrcoex_ini_info.uc_nrcoex_enable = (uint8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_NRCOEX_ENABLE);

    for (cfg_id = WLAN_CFG_INIT_NRCOEX_RULE0_FREQ; cfg_id <= WLAN_CFG_INIT_NRCOEX_RULE3_RXSLOT_RSSI; cfg_id++) {
        if (p_tmp_value > (&st_nrcoex_ini_info.un_nrcoex_rule_data[DMAC_WLAN_NRCOEX_INTERFERE_RULE_NUM - 1].st_nrcoex_rule_data.l_rxslot_rssi)) {
            oam_error_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_nrcoex_param:overflow, cfg_id[%d]}", cfg_id);
            return;
        }
        *p_tmp_value = hwifi_get_init_value(CUS_TAG_INI, cfg_id);
        p_tmp_value++;
    }

    st_syn_msg.len = sizeof(st_nrcoex_ini_info);

    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_data_len), &st_syn_msg, CUSTOM_MSG_DATA_HDR_LEN);
    l_ret += memcpy_s(puc_data + CUSTOM_MSG_DATA_HDR_LEN,
        (WLAN_LARGE_NETBUF_SIZE - *pul_data_len - CUSTOM_MSG_DATA_HDR_LEN),
        &st_nrcoex_ini_info, sizeof(st_nrcoex_ini_info));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_ini_nrcoex_param::memcpy_s fail[%d]. data_len[%d]}", l_ret, *pul_data_len);
        *pul_data_len += (sizeof(st_nrcoex_ini_info) + CUSTOM_MSG_DATA_HDR_LEN);
        return;
    }

    *pul_data_len += (sizeof(st_nrcoex_ini_info) + CUSTOM_MSG_DATA_HDR_LEN);

    oam_warning_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_nrcoex_param::da_len[%d].}", *pul_data_len);
}
#endif

/*
 * 函 数 名  : hwifi_custom_adapt_device_ini_pm_switch_param
 * 功能描述  : 低功耗定制化
 */
OAL_STATIC void hwifi_custom_adapt_device_ini_pm_switch_param(uint8_t *puc_data, uint32_t *pul_data_len)
{
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg;
    int32_t  l_ret;

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_ini_pm_switch_param::puc_data is NULL data_len[%d].}", *pul_data_len);
        return;
    }

    st_syn_msg.en_syn_id = CUSTOM_CFGID_INI_PM_SWITCH_ID;

    g_wlan_device_pm_switch = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_POWERMGMT_SWITCH);
    g_wlan_host_pm_switch = (g_wlan_device_pm_switch == WLAN_DEV_ALL_ENABLE ||
                          g_wlan_device_pm_switch == WLAN_DEV_LIGHT_SLEEP_SWITCH_EN) ? OAL_TRUE : OAL_FALSE;

    st_syn_msg.auc_msg_body[0] = g_wlan_device_pm_switch;
    st_syn_msg.len = sizeof(st_syn_msg) - CUSTOM_MSG_DATA_HDR_LEN;

    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_data_len), &st_syn_msg, sizeof(st_syn_msg));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_ini_pm_switch_param::memcpy_s fail[%d]. data_len[%d]}", l_ret, *pul_data_len);
        *pul_data_len += sizeof(st_syn_msg);
        return;
    }

    *pul_data_len += sizeof(st_syn_msg);

    oam_warning_log3(0, OAM_SF_CFG,
                     "{hwifi_custom_adapt_device_ini_pm_switch_param::da_len[%d].device[%d]host[%d]pm switch}",
                     *pul_data_len, g_wlan_device_pm_switch, g_wlan_host_pm_switch);
}

/*
 * 函 数 名  : hwifi_custom_adapt_device_ini_fast_ps_check_cnt
 * 功能描述  : max ps mode check cnt定制化，20ms定时器检查几次，即调整idle超时时间
 */
OAL_STATIC void hwifi_custom_adapt_device_ini_fast_ps_check_cnt(uint8_t *puc_data, uint32_t *pul_data_len)
{
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg;
    int32_t  l_ret;

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_ini_fast_ps_check_cnt::puc_data is NULL data_len[%d].}", *pul_data_len);
        return;
    }

    st_syn_msg.en_syn_id = CUSTOM_CFGID_INI_PS_FAST_CHECK_CNT_ID;

    st_syn_msg.auc_msg_body[BYTE_OFFSET_0] = g_wlan_min_fast_ps_idle;
    st_syn_msg.auc_msg_body[BYTE_OFFSET_1] = g_wlan_max_fast_ps_idle;
    st_syn_msg.auc_msg_body[BYTE_OFFSET_2] = g_wlan_auto_ps_screen_on;
    st_syn_msg.auc_msg_body[BYTE_OFFSET_3] = g_wlan_auto_ps_screen_off;
    st_syn_msg.len = sizeof(st_syn_msg) - CUSTOM_MSG_DATA_HDR_LEN;

    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_data_len), &st_syn_msg, sizeof(st_syn_msg));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_ini_fast_ps_check_cnt::memcpy_s fail[%d]. data_len[%d]}", l_ret, *pul_data_len);
        *pul_data_len += sizeof(st_syn_msg);
        return;
    }

    *pul_data_len += sizeof(st_syn_msg);

    oam_warning_log4(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_fast_ps_check_cnt:fast_ps idle min/max[%d/%d], \
        auto_ps thresh screen on/off[%d/%d]}", g_wlan_min_fast_ps_idle, g_wlan_max_fast_ps_idle,
        g_wlan_auto_ps_screen_on, g_wlan_auto_ps_screen_off);
}

/*
 * BUCK stay in fastmode
 */
OAL_STATIC void hwifi_custom_adapt_device_ini_fast_mode(uint8_t *data, uint32_t *data_len)
{
    errno_t ret;
    hmac_to_dmac_cfg_custom_data_stru syn_msg = {0};

    if (data == NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_fast_mode:: data_len[%d].}", *data_len);
        return;
    }

    syn_msg.en_syn_id = CUSTOM_CFGID_INI_FAST_MODE;
    syn_msg.auc_msg_body[0] = (uint8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_FAST_MODE);
    syn_msg.len = sizeof(syn_msg) - CUSTOM_MSG_DATA_HDR_LEN;
    ret = memcpy_s(data, (WLAN_LARGE_NETBUF_SIZE - *data_len), &syn_msg, sizeof(syn_msg));
    if (ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_ini_fast_mode:: memcpy_s fail[%d]. data_len[%d]}", ret, *data_len);
        *data_len += sizeof(syn_msg);
        return;
    }

    *data_len += sizeof(syn_msg);

    oam_warning_log2(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_fast_mode:fast_mode = [%d], data_len = [%d]}",
                     syn_msg.auc_msg_body[0], *data_len);
}

/*
 * 函 数 名  : hwifi_custom_adapt_device_priv_ini_radio_cap_param
 * 功能描述  : 下发私有动态/静态dbdc配置定制化项到device
 */
OAL_STATIC int32_t hwifi_custom_adapt_device_priv_ini_radio_cap_param(uint8_t *puc_data, uint32_t *pul_data_len)
{
    int32_t ret;
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};
    int32_t l_priv_value = 0;
    uint8_t uc_cmd_idx;
    uint8_t uc_device_idx;
    uint8_t auc_wlan_service_device_per_chip[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP] = { WLAN_INIT_DEVICE_RADIO_CAP };

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_priv_ini_radio_cap_param::puc_data is NULL data_len[%d].}", *pul_data_len);
        return OAL_FAIL;
    }

    /* 为了不影响host device初始化，这里重新获取定制化文件读到的值 */
    uc_cmd_idx = WLAN_CFG_PRIV_DBDC_RADIO_0;
    for (uc_device_idx = 0; uc_device_idx < WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP; uc_device_idx++) {
        ret = hwifi_get_init_priv_value(uc_cmd_idx++, &l_priv_value);
        if (ret == OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{hwifi_custom_adapt_device_priv_ini_radio_cap_param::WLAN_CFG_PRIV_DBDC_RADIO_0 [%d]}", l_priv_value);
            auc_wlan_service_device_per_chip[uc_device_idx] = (uint8_t)(uint32_t)l_priv_value;
        }
    }

    st_syn_msg.en_syn_id = CUSTOM_CFGID_PRIV_INI_RADIO_CAP_ID;
    st_syn_msg.len = sizeof(auc_wlan_service_device_per_chip);

    ret = memcpy_s(puc_data, WLAN_LARGE_NETBUF_SIZE, &st_syn_msg, CUSTOM_MSG_DATA_HDR_LEN);
    ret += memcpy_s(puc_data + CUSTOM_MSG_DATA_HDR_LEN, (WLAN_LARGE_NETBUF_SIZE - CUSTOM_MSG_DATA_HDR_LEN),
        auc_wlan_service_device_per_chip, sizeof(auc_wlan_service_device_per_chip));
    if (ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_priv_ini_radio_cap_param:memcpy_s fail[%d].data_len[%d]}", ret, *pul_data_len);
        *pul_data_len += (sizeof(auc_wlan_service_device_per_chip) + CUSTOM_MSG_DATA_HDR_LEN);
        return OAL_FAIL;
    }

    *pul_data_len += (sizeof(auc_wlan_service_device_per_chip) + CUSTOM_MSG_DATA_HDR_LEN);

    oam_warning_log2(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_priv_ini_radio_cap_param::da_len[%d] radio_cap_0[%d].}",
                     *pul_data_len, auc_wlan_service_device_per_chip[0]);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
/*
 * 函 数 名  : hwifi_custom_adapt_device_priv_ini_download_pm_param
 * 功能描述  : 下发私有开机rx listen ps rssi定制化项到device
 */
OAL_STATIC int32_t hwifi_custom_adapt_device_priv_ini_hal_ps_rssi_param(uint8_t *puc_data,
                                                                        uint32_t *pul_data_len)
{
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};
    int32_t l_ret;
    int32_t l_priv_val = 0;
    uint32_t hal_ps_rssi_params = 0;

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_priv_ini_hal_ps_rssi_param::puc_data NULL data_len[%d].}", *pul_data_len);
        return OAL_FAIL;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_HAL_PS_RSSI_PARAM, &l_priv_val);
    if (l_ret == OAL_SUCC) {
        hal_ps_rssi_params = (uint32_t)l_priv_val;
        oal_io_print("hwifi_custom_adapt_device_priv_ini_hal_ps_rssi_param::read hal_ps_rssi_params[%d]l_ret[%d]\r\n",
                     hal_ps_rssi_params, l_ret);
    } else {
        return OAL_FAIL;
    }

    st_syn_msg.en_syn_id = CUSTOM_CFGID_PRIV_INI_HAL_PS_RSSI_ID;
    st_syn_msg.len = sizeof(hal_ps_rssi_params);

    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_data_len), &st_syn_msg, CUSTOM_MSG_DATA_HDR_LEN);
    l_ret += memcpy_s(puc_data + CUSTOM_MSG_DATA_HDR_LEN,
                      (WLAN_LARGE_NETBUF_SIZE - *pul_data_len - CUSTOM_MSG_DATA_HDR_LEN),
                      &hal_ps_rssi_params, sizeof(hal_ps_rssi_params));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG,
                       "{hwifi_custom_adapt_device_priv_ini_hal_ps_rssi_param::memcpy_s fail[%d]. data_len[%d]}",
                       l_ret, *pul_data_len);
        *pul_data_len += (sizeof(hal_ps_rssi_params) + CUSTOM_MSG_DATA_HDR_LEN);

        return OAL_FAIL;
    }

    *pul_data_len += (sizeof(hal_ps_rssi_params) + CUSTOM_MSG_DATA_HDR_LEN);

    oam_warning_log2(0, OAM_SF_CFG,
                     "{hwifi_custom_adapt_device_priv_ini_hal_ps_rssi_param::da_len[%d] hal_pps_rssi_params [%X].}",
                     *pul_data_len, hal_ps_rssi_params);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hwifi_custom_adapt_device_priv_ini_download_pm_param
 * 功能描述  : 下发私有开机rx listen ps pps定制化项到device
 */
OAL_STATIC int32_t hwifi_custom_adapt_device_priv_ini_hal_ps_pps_param(uint8_t *puc_data,
                                                                       uint32_t *pul_data_len)
{
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};
    int32_t l_ret;
    int32_t l_priv_val = 0;
    uint32_t hal_ps_pps_params = 0;

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_priv_ini_hal_ps_pps_param::puc_data NULL data_len[%d].}", *pul_data_len);
        return OAL_FAIL;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_HAL_PS_PPS_PARAM, &l_priv_val);
    if (l_ret == OAL_SUCC) {
        hal_ps_pps_params = (uint32_t)l_priv_val;
        oal_io_print("hwifi_custom_adapt_device_priv_ini_hal_ps_pps_param::read hal_ps_pps_params[%d]l_ret[%d]\r\n",
                     hal_ps_pps_params, l_ret);
    } else {
        return OAL_FAIL;
    }

    st_syn_msg.en_syn_id = CUSTOM_CFGID_PRIV_INI_HAL_PS_PPS_ID;
    st_syn_msg.len = sizeof(hal_ps_pps_params);

    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_data_len), &st_syn_msg, CUSTOM_MSG_DATA_HDR_LEN);
    l_ret += memcpy_s(puc_data + CUSTOM_MSG_DATA_HDR_LEN,
                      (WLAN_LARGE_NETBUF_SIZE - *pul_data_len - CUSTOM_MSG_DATA_HDR_LEN),
                      &hal_ps_pps_params, sizeof(hal_ps_pps_params));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG,
                       "{hwifi_custom_adapt_device_priv_ini_hal_ps_pps_param::memcpy_s fail[%d]. data_len[%d]}",
                       l_ret, *pul_data_len);
        *pul_data_len += (sizeof(hal_ps_pps_params) + CUSTOM_MSG_DATA_HDR_LEN);

        return OAL_FAIL;
    }

    *pul_data_len += (sizeof(hal_ps_pps_params) + CUSTOM_MSG_DATA_HDR_LEN);

    oam_warning_log2(0, OAM_SF_CFG,
                     "{hwifi_custom_adapt_device_priv_ini_hal_ps_pps_param::da_len[%d] hal_ps_pps_params [%X].}",
                     *pul_data_len, hal_ps_pps_params);

    return OAL_SUCC;
}

#endif

/*
 * 函 数 名  : hwifi_custom_adapt_device_priv_ini_hiex_cap
 * 功能描述  : 下发私有开机hiex cap定制化项到device
 */
OAL_STATIC int32_t hwifi_custom_adapt_device_priv_ini_hiex_cap(uint8_t *puc_data,
    uint32_t *pul_data_len)
{
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};
    int32_t l_ret;
    int32_t l_priv_val = 0;
    uint32_t hiex_cap = 0;

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_priv_ini_hiex_cap::puc_data NULL data_len[%d].}",
            *pul_data_len);
        return OAL_FAIL;
    }
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_HIEX_CAP, &l_priv_val);
    if (l_ret == OAL_SUCC) {
        hiex_cap = (uint32_t)l_priv_val;
        oal_io_print("hwifi_custom_adapt_device_priv_ini_hiex_cap::read hiex_cap[%d]l_ret[%d]\r\n", hiex_cap, l_ret);
    } else {
        return OAL_FAIL;
    }

    st_syn_msg.en_syn_id = CUSTOM_CFGID_PRIV_INI_HIEX_CAP_ID;
    st_syn_msg.len = sizeof(hiex_cap);
    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_data_len), &st_syn_msg, CUSTOM_MSG_DATA_HDR_LEN);
    l_ret += memcpy_s(puc_data + CUSTOM_MSG_DATA_HDR_LEN,
        (WLAN_LARGE_NETBUF_SIZE - *pul_data_len - CUSTOM_MSG_DATA_HDR_LEN), &hiex_cap, sizeof(hiex_cap));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_priv_ini_hiex_cap::memcpy_s fail[%d] data_len[%d]}",
            l_ret, *pul_data_len);
        *pul_data_len += (sizeof(hiex_cap) + CUSTOM_MSG_DATA_HDR_LEN);
        return OAL_FAIL;
    }

    *pul_data_len += (sizeof(hiex_cap) + CUSTOM_MSG_DATA_HDR_LEN);
    oam_warning_log2(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_priv_ini_hiex_cap::data_len[%d] hiex_cap[0x%X]}",
        *pul_data_len, hiex_cap);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hwifi_custom_adapt_device_priv_ini_temper_thread_param
 * 功能描述  : 下发过温保护配置定制化项到device
 */
OAL_STATIC int32_t hwifi_custom_adapt_device_priv_ini_temper_thread_param(uint8_t *puc_data,
                                                                          uint32_t *pul_data_len)
{
    int32_t l_ret;
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};
    int32_t l_priv_val = 0;
    uint32_t over_temp_protect_thread;

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_priv_ini_temper_thread_param::puc_data is NULL data_len[%d].}", *pul_data_len);
        return OAL_FAIL;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_OVER_TEMPER_PROTECT_THRESHOLD, &l_priv_val);
    if (l_ret == OAL_SUCC) {
        over_temp_protect_thread = (uint32_t)l_priv_val;
        oal_io_print("hwifi_custom_adapt_device_priv_ini_temper_thread_param::read over_temp_protect_thread[%d]\r\n",
                     over_temp_protect_thread);
    } else {
        return OAL_FAIL;
    }

    st_syn_msg.en_syn_id = CUSTOM_CFGID_PRIV_INI_OVER_TEMPER_PROTECT_THRESHOLD_ID;
    st_syn_msg.len = sizeof(over_temp_protect_thread);

    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_data_len), &st_syn_msg, CUSTOM_MSG_DATA_HDR_LEN);
    l_ret += memcpy_s(puc_data + CUSTOM_MSG_DATA_HDR_LEN,
                      (WLAN_LARGE_NETBUF_SIZE - *pul_data_len - CUSTOM_MSG_DATA_HDR_LEN),
                      &over_temp_protect_thread,
                      sizeof(over_temp_protect_thread));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_priv_ini_temper_thread_param::memcpy_s fail[%d]. \
            data_len[%d]}", l_ret, *pul_data_len);
        *pul_data_len += (sizeof(over_temp_protect_thread) + CUSTOM_MSG_DATA_HDR_LEN);
        return OAL_FAIL;
    }

    *pul_data_len += (sizeof(over_temp_protect_thread) + CUSTOM_MSG_DATA_HDR_LEN);

    oam_warning_log2(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_priv_ini_temper_thread_param::da_len[%d] \
        over_temp_protect_thread[0x%x].}", *pul_data_len, over_temp_protect_thread);

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE void hwifi_custom_adapt_hcc_flowctrl_type(hmac_to_dmac_cfg_custom_data_stru *pst_syn_msg,
                                                                uint8_t *puc_priv_cfg_value)
{
    if (hcc_bus_flowctrl_init(*puc_priv_cfg_value) != OAL_SUCC) {
        /* GPIO流控中断注册失败，强制device使用SDIO流控(type = 0) */
        *puc_priv_cfg_value = 0;
    }
    pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_HCC_FLOWCTRL_TYPE_ID;
    oal_io_print("hwifi_custom_adapt_mac_device_priv_ini_param::sdio_flow_ctl_type[0x%x].\r\n",
                 *puc_priv_cfg_value);
}

OAL_STATIC void hwifi_custom_adapt_priv_ini_param_extend_etc(
    hmac_to_dmac_cfg_custom_data_stru *pst_syn_msg, wlan_cfg_priv_id_uint8 uc_cfg_id)
{
    switch (uc_cfg_id) {
        case WLAN_CFG_PRIV_PHY_CAP_SWITCH:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_PHY_CAP_SWITCH_ID;
            break;
        case WLAN_CFG_PRIV_OPTIMIZED_FEATURE_SWITCH:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_OPTIMIZED_FEATURE_SWITCH_ID;
            break;
        case WLAN_CFG_PRIV_DDR_SWITCH:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_DDR_SWITCH_ID;
            break;
        case WLAN_CFG_PRIV_FTM_CAP:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_FTM_CAP_ID;
            break;

        default:
            break;
    }
}

OAL_STATIC void hwifi_custom_adapt_priv_ini_param_extend(hmac_to_dmac_cfg_custom_data_stru *pst_syn_msg,
                                                         wlan_cfg_priv_id_uint8 uc_cfg_id,
                                                         uint8_t *puc_priv_cfg_value)
{
    switch (uc_cfg_id) {
        case WLAN_CFG_PRIV_DYN_BYPASS_EXTLNA:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_DYN_BYPASS_EXTLNA_ID;
            break;
        case WLAN_CFG_PRIV_CTRL_FRAME_TX_CHAIN:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_CTRL_FRAME_TX_CHAIN_ID;
            break;
        case WLAN_CFG_PRIV_CTRL_UPC_FOR_18DBM_CO:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_CTRL_UPC_FOR_18DBM_C0_ID;
            break;
        case WLAN_CFG_PRIV_CTRL_UPC_FOR_18DBM_C1:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_CTRL_UPC_FOR_18DBM_C1_ID;
            break;
        case WLAN_CFG_PRIV_CTRL_11B_DOUBLE_CHAIN_BO_POW:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_CTRL_11B_DOUBLE_CHAIN_BO_POW_ID;
            break;
        case WLAN_CFG_RADAR_ISR_FORBID:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_RADAR_ISR_FORBID_ID;
            break;
        case WLAN_CFG_PRIV_HCC_FLOWCTRL_TYPE:
            hwifi_custom_adapt_hcc_flowctrl_type(pst_syn_msg, puc_priv_cfg_value);
            break;
        case WLAN_CFG_PRIV_MBO_SWITCH:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_MBO_SWITCH_ID;
            break;
        case WLAN_CFG_PRIV_DYNAMIC_DBAC_SWITCH:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_DYNAMIC_DBAC_ID;
            break;
        case WLAN_CFG_PRIV_DC_FLOWCTL_SWITCH:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_DC_FLOWCTRL_ID;
            break;
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
        case WLAN_CFG_PRIV_MCAST_AMPDU_ENABLE:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_MCAST_AMPDU_ENABLE_ID;
            break;
#endif
        case WLAN_CFG_PRIV_MIRACAST_SINK:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_MIRACAST_SINK;
            break;

        default:
            break;
    }
    oal_io_print("hwifi_custom_adapt_priv_ini_param_extend::syn_id[%d] val[%d].\r\n",
        pst_syn_msg->en_syn_id, *puc_priv_cfg_value);
    hwifi_custom_adapt_priv_ini_param_extend_etc(pst_syn_msg, uc_cfg_id);
}

OAL_STATIC void hwifi_custom_adapt_priv_11ax_feature_ini_param(hmac_to_dmac_cfg_custom_data_stru *pst_syn_msg,
                                                               wlan_cfg_priv_id_uint8 uc_cfg_id,
                                                               uint8_t uc_priv_cfg_value)
{
    switch (uc_cfg_id) {
        case WLAN_CFG_PRIV_VOE_SWITCH:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_VOE_SWITCH_ID;
            oal_io_print("hwifi_custom_adapt_mac_device_priv_ini_param::voe switch[%d].\r\n", uc_priv_cfg_value);
            break;
        case WLAN_CFG_PRIV_11AX_SWITCH:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_11AX_SWITCH_ID;
            break;
        case WLAN_CFG_PRIV_HTC_SWITCH:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_HTC_SWITCH_ID;
            break;
        case WLAN_CFG_PRIV_MULTI_BSSID_SWITCH:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_MBSSID_SWITCH_ID;
            break;
        case WLAN_CFG_PRIV_AC_SUSPEND:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_AC_SUSPEND_ID;
            break;
        case WLAN_CFG_PRIV_DYNAMIC_DBAC_SWITCH:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_DYNAMIC_DBAC_ID;
            break;
        default:
            break;
    }
}

OAL_STATIC void hwifi_custom_adapt_priv_feature_ini_param(hmac_to_dmac_cfg_custom_data_stru *pst_syn_msg,
                                                          wlan_cfg_priv_id_uint8 uc_cfg_id, uint8_t *puc_priv_cfg_value)
{
    switch (uc_cfg_id) {
        case WLAN_CFG_PRIV_COUNRTYCODE_SELFSTUDY_CFG:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_COUNTRYCODE_SELFSTUDY_CFG_ID;
            oal_io_print("hwifi_custom_adapt_mac_device_priv_ini_param::country code self study[%d].\r\n",
                         *puc_priv_cfg_value);
            break;
        case WLAN_CFG_PRIV_M2S_FUNCTION_MASK:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_M2S_FUNCTION_MASK_ID;
            oal_io_print("hwifi_custom_adapt_mac_device_priv_ini_param::m2s_mask[0x%x].\r\n", *puc_priv_cfg_value);
            break;

        case WLAN_CFG_PRIV_M2S_FUNCTION_EXT_MASK:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_M2S_FUNCTION_EXT_MASK_ID;
            oal_io_print("hwifi_custom_adapt_mac_device_priv_ini_param::m2s_mask_ext[0x%x].\r\n", *puc_priv_cfg_value);
            break;
        case WLAN_CFG_PRIV_MCM_FUNCTION_MASK:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_MCM_FUNCTION_MASK_ID;
            oal_io_print("hwifi_custom_adapt_mac_device_priv_ini_param::mcm_msk[0x%x].\r\n", *puc_priv_cfg_value);
            break;

        case WLAN_CFG_PRIV_MCM_CUSTOM_FUNCTION_MASK:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_MCM_CUSTOM_FUNCTION_MASK_ID;
            oal_io_print("hwifi_custom_adapt_mac_device_priv_ini_param::mcm_custom[0x%x].\r\n", *puc_priv_cfg_value);
            break;
        case WLAN_CFG_PRIV_FASTSCAN_SWITCH:
            pst_syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_FASTSCAN_SWITCH_ID;
            oal_io_print("hwifi_custom_adapt_mac_device_priv_ini_param::fastcan [0x%x].\r\n", *puc_priv_cfg_value);
            break;
        default:
            break;
    }
}

/*
 * 函 数 名  : hwifi_custom_adapt_priv_ini_param_extend
 * 功能描述  : 解圈复杂度拆分函数
 */
OAL_STATIC void hwifi_custom_adapt_priv_ini_extend(hmac_to_dmac_cfg_custom_data_stru *pst_syn_msg,
                                                   wlan_cfg_priv_id_uint8 uc_cfg_id,
                                                   uint8_t *puc_priv_cfg_value)
{
    switch (uc_cfg_id) {
        case WLAN_CFG_PRIV_DYN_BYPASS_EXTLNA:
        case WLAN_CFG_PRIV_CTRL_FRAME_TX_CHAIN:
        case WLAN_CFG_PRIV_CTRL_UPC_FOR_18DBM_CO:
        case WLAN_CFG_PRIV_CTRL_UPC_FOR_18DBM_C1:
        case WLAN_CFG_PRIV_CTRL_11B_DOUBLE_CHAIN_BO_POW:
        case WLAN_CFG_RADAR_ISR_FORBID:
        case WLAN_CFG_PRIV_HCC_FLOWCTRL_TYPE:
        case WLAN_CFG_PRIV_MBO_SWITCH:
        case WLAN_CFG_PRIV_DC_FLOWCTL_SWITCH:
        case WLAN_CFG_PRIV_PHY_CAP_SWITCH:
        case WLAN_CFG_PRIV_OPTIMIZED_FEATURE_SWITCH:
        case WLAN_CFG_PRIV_DDR_SWITCH:
        case WLAN_CFG_PRIV_FTM_CAP:
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
        case WLAN_CFG_PRIV_MCAST_AMPDU_ENABLE:
#endif
        case WLAN_CFG_PRIV_MIRACAST_SINK:
            hwifi_custom_adapt_priv_ini_param_extend(pst_syn_msg, uc_cfg_id, puc_priv_cfg_value);
            break;
        default:
            break;
    }
}

static void hwifi_priv_ini_param_set_ldpc_codeing_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                     uint8_t cfg_id,
                                                     uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_LDPC_CODING_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::ldpc coding[%d].\r\n", *priv_cfg_value);
}

static void hwifi_priv_ini_param_set_bw_max_with_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                    uint8_t cfg_id,
                                                    uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_BW_MAX_WITH_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::max_bw[%d].\r\n", *priv_cfg_value);
}

static void hwifi_priv_ini_param_set_rx_stbc_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                uint8_t cfg_id,
                                                uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_RX_STBC_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::rx_stbc[%d].\r\n", *priv_cfg_value);
}

static void hwifi_priv_ini_param_set_tx_stbc_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                uint8_t cfg_id,
                                                uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_TX_STBC_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::tx_stbc[%d].\r\n", *priv_cfg_value);
}

static void hwifi_priv_ini_param_set_su_bfer_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                uint8_t cfg_id,
                                                uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_SU_BFER_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::su bfer[%d].\r\n", *priv_cfg_value);
}

#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
static void hwifi_priv_ini_param_set_mcast_ampdu_enable_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                           uint8_t cfg_id,
                                                           uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = WLAN_CFG_PRIV_MCAST_AMPDU_ENABLE;
    oal_io_print("hwifi_priv_ini_param_set_mcast_ampdu_enable_id::mcast_ampdu_enable[%d]", *priv_cfg_value);
}
#endif

static void hwifi_priv_ini_param_set_su_bfee_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                uint8_t cfg_id,
                                                uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_SU_BFEE_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::su bfee[%d].\r\n", *priv_cfg_value);
}

static void hwifi_priv_ini_param_set_mu_bfer_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                uint8_t cfg_id,
                                                uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_MU_BFER_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::mu bfer[%d].\r\n", *priv_cfg_value);
}

static void hwifi_priv_ini_param_set_mu_bfee_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                uint8_t cfg_id,
                                                uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_MU_BFEE_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::mu bfee[%d].\r\n", *priv_cfg_value);
}

static void hwifi_priv_ini_param_set_11n_txbf_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                 uint8_t cfg_id,
                                                 uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_11N_TXBF_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::11n txbf[%d].\r\n", *priv_cfg_value);
}

static void hwifi_priv_ini_param_set_1024_qam_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                 uint8_t cfg_id,
                                                 uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_1024_QAM_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::1024qam[%d].\r\n", *priv_cfg_value);
}

static void hwifi_priv_ini_param_set_cali_data_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                  uint8_t cfg_id,
                                                  uint8_t *priv_cfg_value)
{
    /* 开机默认打开校准数据上传下发 */
    *priv_cfg_value = hwifi_custom_cali_ini_param(*priv_cfg_value);
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_CALI_DATA_MASK_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::g_wlan_open_cnt[%d] \
        priv_cali_data_up_down[0x%x].", g_wlan_open_cnt, *priv_cfg_value);
}

static void hwifi_priv_ini_param_set_autocali_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                 uint8_t cfg_id,
                                                 uint8_t *priv_cfg_value)
{
    /* 开机默认不打开开机校准 */
    *priv_cfg_value = (g_custom_cali_done == OAL_FALSE) ? OAL_FALSE : *priv_cfg_value;
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_AUTOCALI_MASK_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::g_custom_cali_done[%d]\
        auto_cali_mask[0x%x].\r\n", g_custom_cali_done, *priv_cfg_value);
}

static void hwifi_priv_ini_param_set_priv_feature_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                     uint8_t cfg_id,
                                                     uint8_t *priv_cfg_value)
{
    hwifi_custom_adapt_priv_feature_ini_param(syn_msg, cfg_id, priv_cfg_value);
}

static void hwifi_priv_ini_param_set_ant_switch_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                   uint8_t cfg_id,
                                                   uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_ANT_SWITCH_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::ant switch[%d].\r\n", *priv_cfg_value);
}

static void hwifi_priv_ini_param_set_linkloss_threshold_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                           uint8_t cfg_id,
                                                           uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_LINKLOSS_THRESHOLD_FIXED_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::linkloss threshold fixed[%d].\r\n",
                 *priv_cfg_value);
}

static void hwifi_priv_ini_param_set_txopps_switch_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                      uint8_t cfg_id,
                                                      uint8_t *priv_cfg_value)
{
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_TXOPPS_SWITCH_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::uc_priv_cfg_value[0x%x].\r\n", *priv_cfg_value);
#endif
}

static void hwifi_priv_ini_param_set_pro_enable_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                   uint8_t cfg_id,
                                                   uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_TEMP_PRO_ENABLE_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::temp pro enable[%d].\r\n", *priv_cfg_value);
}

static void hwifi_priv_ini_param_set_pro_reduce_enable_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                          uint8_t cfg_id,
                                                          uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_TEMP_PRO_REDUCE_PWR_ENABLE_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::temp pro reduce pwr enable[%d].\r\n",
                 *priv_cfg_value);
}
static void hwifi_priv_ini_param_set_pro_safe_th_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                    uint8_t cfg_id,
                                                    uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_TEMP_PRO_SAFE_TH_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::temp pro safe th[%d].\r\n", *priv_cfg_value);
}
static void hwifi_priv_ini_param_set_pro_over_th_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                    uint8_t cfg_id,
                                                    uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_TEMP_PRO_OVER_TH_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::temp pro over th[%d].\r\n", *priv_cfg_value);
}
static void hwifi_priv_ini_param_set_pro_pa_off_th_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                      uint8_t cfg_id,
                                                      uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_TEMP_PRO_PA_OFF_TH_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::temp pro pa off th[%d].\r\n", *priv_cfg_value);
}
static void hwifi_priv_ini_param_set_evm_pll_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                uint8_t cfg_id,
                                                uint8_t *priv_cfg_value)
{
    syn_msg->en_syn_id = CUSTOM_CFGID_PRIV_INI_EVM_PLL_REG_FIX_ID;
    oal_io_print("hwifi_custom_adapt_priv_ini_param::temp pro safe th[%d].\r\n", *priv_cfg_value);
}
static void hwifi_priv_ini_param_set_11ax_feature_id(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                                     uint8_t cfg_id,
                                                     uint8_t *priv_cfg_value)
{
    hwifi_custom_adapt_priv_11ax_feature_ini_param(syn_msg, cfg_id, *priv_cfg_value);
}

typedef struct {
    uint8_t cfg_id;
    void (*hwifi_priv_ini_param_set_id)(hmac_to_dmac_cfg_custom_data_stru *syn_msg,
                                        uint8_t cfg_id,
                                        uint8_t *priv_cfg_value);
} hwifi_priv_ini_param_ops;
/*
 * 函 数 名  : hwifi_custom_adapt_priv_ini_param
 * 功能描述  : 下发私有开机device配置定制化项到device
 */
OAL_STATIC int32_t hwifi_custom_adapt_priv_ini_param(uint8_t uc_cfg_id, uint8_t *puc_data, uint32_t *pul_len)
{
    int32_t l_ret;
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};
    int32_t l_priv_val = 0;
    uint8_t uc_priv_cfg_value;
    uint8_t idx;
    const hwifi_priv_ini_param_ops hwifi_priv_ini_param_ops_table[] = {
        { WLAN_CFG_PRIV_LDPC_CODING,                     hwifi_priv_ini_param_set_ldpc_codeing_id },
        { WLAN_CFG_PRIV_BW_MAX_WITH,                     hwifi_priv_ini_param_set_bw_max_with_id },
        { WLAN_CFG_PRIV_RX_STBC,                         hwifi_priv_ini_param_set_rx_stbc_id },
        { WLAN_CFG_PRIV_TX_STBC,                         hwifi_priv_ini_param_set_tx_stbc_id },
        { WLAN_CFG_PRIV_SU_BFER,                         hwifi_priv_ini_param_set_su_bfer_id },
        { WLAN_CFG_PRIV_SU_BFEE,                         hwifi_priv_ini_param_set_su_bfee_id },
        { WLAN_CFG_PRIV_MU_BFER,                         hwifi_priv_ini_param_set_mu_bfer_id },
        { WLAN_CFG_PRIV_MU_BFEE,                         hwifi_priv_ini_param_set_mu_bfee_id },
        { WLAN_CFG_PRIV_11N_TXBF,                        hwifi_priv_ini_param_set_11n_txbf_id },
        { WLAN_CFG_PRIV_1024_QAM,                        hwifi_priv_ini_param_set_1024_qam_id },
        { WLAN_CFG_PRIV_CALI_DATA_MASK,                  hwifi_priv_ini_param_set_cali_data_id },
        { WLAN_CFG_PRIV_CALI_AUTOCALI_MASK,              hwifi_priv_ini_param_set_autocali_id },
        { WLAN_CFG_PRIV_M2S_FUNCTION_MASK,               hwifi_priv_ini_param_set_priv_feature_id },
        { WLAN_CFG_PRIV_M2S_FUNCTION_EXT_MASK,           hwifi_priv_ini_param_set_priv_feature_id },
        { WLAN_CFG_PRIV_MCM_FUNCTION_MASK,               hwifi_priv_ini_param_set_priv_feature_id },
        { WLAN_CFG_PRIV_MCM_CUSTOM_FUNCTION_MASK,        hwifi_priv_ini_param_set_priv_feature_id },
        { WLAN_CFG_PRIV_COUNRTYCODE_SELFSTUDY_CFG,       hwifi_priv_ini_param_set_priv_feature_id },
        { WLAN_CFG_PRIV_FASTSCAN_SWITCH,                 hwifi_priv_ini_param_set_priv_feature_id },
        { WLAN_CFG_ANT_SWITCH,                           hwifi_priv_ini_param_set_ant_switch_id },
        { WLAN_CFG_PRRIV_LINKLOSS_THRESHOLD_FIXED,       hwifi_priv_ini_param_set_linkloss_threshold_id },
        { WLAN_CFG_PRIV_TXOPPS_SWITCH,                   hwifi_priv_ini_param_set_txopps_switch_id },
        { WLAN_CFG_PRIV_OVER_TEMP_PRO_ENABLE,            hwifi_priv_ini_param_set_pro_enable_id },
        { WLAN_CFG_PRIV_OVER_TEMP_PRO_REDUCE_PWR_ENABLE, hwifi_priv_ini_param_set_pro_reduce_enable_id },
        { WLAN_CFG_PRIV_OVER_TEMP_PRO_SAFE_TH,           hwifi_priv_ini_param_set_pro_safe_th_id },
        { WLAN_CFG_PRIV_OVER_TEMP_PRO_OVER_TH,           hwifi_priv_ini_param_set_pro_over_th_id },
        { WLAN_CFG_PRIV_OVER_TEMP_PRO_PA_OFF_TH,         hwifi_priv_ini_param_set_pro_pa_off_th_id },
        { WLAN_CFG_PRIV_EVM_PLL_REG_FIX,                 hwifi_priv_ini_param_set_evm_pll_id },
        { WLAN_CFG_PRIV_VOE_SWITCH,                      hwifi_priv_ini_param_set_11ax_feature_id },
        { WLAN_CFG_PRIV_11AX_SWITCH,                     hwifi_priv_ini_param_set_11ax_feature_id },
        { WLAN_CFG_PRIV_HTC_SWITCH,                      hwifi_priv_ini_param_set_11ax_feature_id },
        { WLAN_CFG_PRIV_MULTI_BSSID_SWITCH,              hwifi_priv_ini_param_set_11ax_feature_id },
        { WLAN_CFG_PRIV_AC_SUSPEND,                      hwifi_priv_ini_param_set_11ax_feature_id },
        { WLAN_CFG_PRIV_DYNAMIC_DBAC_SWITCH,             hwifi_priv_ini_param_set_11ax_feature_id },
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
        { WLAN_CFG_PRIV_MCAST_AMPDU_ENABLE,              hwifi_priv_ini_param_set_mcast_ampdu_enable_id},
#endif
    };

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_priv_ini_param::puc_data NULL data_len[%d]}", *pul_len);
        return OAL_FAIL;
    }

    l_ret = hwifi_get_init_priv_value(uc_cfg_id, &l_priv_val);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "hwifi_custom_adapt_priv_ini_param::get_init_priv fail cfg_id[%d]", uc_cfg_id);
        return OAL_FAIL;
    }

    uc_priv_cfg_value = (uint8_t)(uint32_t)l_priv_val;

    for (idx = 0; idx < (sizeof(hwifi_priv_ini_param_ops_table) / sizeof(hwifi_priv_ini_param_ops)); ++idx) {
        if (hwifi_priv_ini_param_ops_table[idx].cfg_id == uc_cfg_id) {
            hwifi_priv_ini_param_ops_table[idx].hwifi_priv_ini_param_set_id(&st_syn_msg, uc_cfg_id, &uc_priv_cfg_value);
            break;
        }
    }

    hwifi_custom_adapt_priv_ini_extend(&st_syn_msg, uc_cfg_id, &uc_priv_cfg_value);

    st_syn_msg.len = sizeof(uc_priv_cfg_value);
    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_len), &st_syn_msg, CUSTOM_MSG_DATA_HDR_LEN);
    l_ret += memcpy_s(puc_data + CUSTOM_MSG_DATA_HDR_LEN, (WLAN_LARGE_NETBUF_SIZE - *pul_len -
        CUSTOM_MSG_DATA_HDR_LEN), &uc_priv_cfg_value, sizeof(uc_priv_cfg_value));
    *pul_len += (sizeof(uc_priv_cfg_value) + CUSTOM_MSG_DATA_HDR_LEN);

    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_priv_ini_param::memcpy_s fail[%d]. data_len[%d]}", l_ret, *pul_len);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : hwifi_custom_adapt_device_priv_ini_dsss2ofdm_dbb_pwr_bo_param
 * 功能描述  : 下发私有定制11b的回退功率值到device
 */
OAL_STATIC int32_t hwifi_custom_adapt_device_priv_ini_dsss2ofdm_dbb_pwr_bo_param(uint8_t *puc_data,
                                                                                 uint32_t *pul_data_len)
{
    int32_t l_ret;
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};
    int32_t l_priv_val = 0;
    int16_t l_dsss2ofdm_dbb_pwr_bo;

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_priv_ini_dsss2ofdm_dbb_pwr_bo_param::puc_data is NULL data_len[%d].}",
            *pul_data_len);
        return OAL_FAIL;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_DSSS2OFDM_DBB_PWR_BO_VAL, &l_priv_val);
    if (l_ret != OAL_SUCC) {
        return OAL_FAIL;
    }

    l_dsss2ofdm_dbb_pwr_bo = (int16_t)l_priv_val;
    st_syn_msg.en_syn_id = CUSTOM_CFGID_PRIV_INI_DSSS2OFDM_DBB_PWR_BO_VAL_ID;
    st_syn_msg.len = sizeof(l_dsss2ofdm_dbb_pwr_bo);

    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_data_len), &st_syn_msg, CUSTOM_MSG_DATA_HDR_LEN);
    l_ret += memcpy_s(puc_data + CUSTOM_MSG_DATA_HDR_LEN,
                      (WLAN_LARGE_NETBUF_SIZE - *pul_data_len - CUSTOM_MSG_DATA_HDR_LEN),
                      &l_dsss2ofdm_dbb_pwr_bo,
                      sizeof(l_dsss2ofdm_dbb_pwr_bo));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_priv_ini_dsss2ofdm_dbb_pwr_bo_param::memcpy_s fail[%d]. data_len[%d]}",
            l_ret, *pul_data_len);
        *pul_data_len += (sizeof(l_dsss2ofdm_dbb_pwr_bo) + CUSTOM_MSG_DATA_HDR_LEN);
        return OAL_FAIL;
    }

    *pul_data_len += (sizeof(l_dsss2ofdm_dbb_pwr_bo) + CUSTOM_MSG_DATA_HDR_LEN);

    oam_warning_log2(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_priv_ini_dsss2ofdm_dbb_pwr_bo_param::da_len[%d] \
        l_dsss2ofdm_dbb_pwr_bo[0x%x].}", *pul_data_len, l_dsss2ofdm_dbb_pwr_bo);
    return OAL_SUCC;
}

/*
 * 函 数 名  : hwifi_custom_adapt_device_priv_ini_cali_mask_param
 * 功能描述  : 下发私有开机校准配置定制化项到device
 */
OAL_STATIC int32_t hwifi_custom_adapt_device_priv_ini_cali_mask_param(uint8_t *puc_data, uint32_t *pul_data_len)
{
    int32_t l_ret;
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};
    int32_t l_priv_val = 0;
    uint32_t cali_mask;
#ifdef _PRE_PHONE_RUNMODE_FACTORY
    int32_t l_chip_type = get_hi110x_subchip_type();
#endif

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_priv_ini_cali_mask_param::puc_data is NULL data_len[%d].}", *pul_data_len);
        return OAL_FAIL;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_CALI_MASK, &l_priv_val);
    if (l_ret == OAL_SUCC) {
        cali_mask = (uint32_t)l_priv_val;
#ifdef _PRE_PHONE_RUNMODE_FACTORY
    if (l_chip_type == BOARD_VERSION_HI1106) {
        cali_mask = (cali_mask & (~CALI_HI1106_MIMO_MASK)) | CALI_MUL_TIME_CALI_MASK;
    }else if (l_chip_type == BOARD_VERSION_HI1105) {
        cali_mask = (cali_mask & (~CALI_MIMO_MASK)) | CALI_MUL_TIME_CALI_MASK;
    }
#endif
        oal_io_print("hwifi_custom_adapt_device_priv_ini_cali_mask_param::read cali_mask[%d]l_ret[%d]\r\n",
                     cali_mask, l_ret);
    } else {
        return OAL_FAIL;
    }

    st_syn_msg.en_syn_id = CUSTOM_CFGID_PRIV_INI_CALI_MASK_ID;
    st_syn_msg.len = sizeof(cali_mask);

    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_data_len), &st_syn_msg, CUSTOM_MSG_DATA_HDR_LEN);
    l_ret += memcpy_s(puc_data + CUSTOM_MSG_DATA_HDR_LEN,
        (WLAN_LARGE_NETBUF_SIZE - *pul_data_len - CUSTOM_MSG_DATA_HDR_LEN), &cali_mask, sizeof(cali_mask));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG,
                       "{hwifi_custom_adapt_device_priv_ini_cali_mask_param::memcpy_s fail[%d]. data_len[%d]}",
                       l_ret, *pul_data_len);
        *pul_data_len += (sizeof(cali_mask) + CUSTOM_MSG_DATA_HDR_LEN);

        return OAL_FAIL;
    }

    *pul_data_len += (sizeof(cali_mask) + CUSTOM_MSG_DATA_HDR_LEN);

    oam_warning_log2(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_priv_ini_cali_mask_param::da_len[%d] cali_mask[0x%x].}",
                     *pul_data_len, cali_mask);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hwifi_custom_adapt_device_priv_ini_download_pm_param
 * 功能描述  : 下发私有开机校准配置定制化项到device
 */
OAL_STATIC int32_t hwifi_custom_adapt_device_priv_ini_download_pm_param(uint8_t *puc_data, uint32_t *pul_data_len)
{
    int32_t l_ret;
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};
    int32_t l_priv_val = 0;
    uint16_t us_download_rate_limit_pps;

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_priv_ini_download_pm_param::puc_data is NULL data_len[%d]}", *pul_data_len);
        return OAL_FAIL;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_DOWNLOAD_RATE_LIMIT_PPS, &l_priv_val);
    if (l_ret == OAL_SUCC) {
        us_download_rate_limit_pps = (uint16_t)(uint32_t)l_priv_val;
        oal_io_print("hwifi_custom_adapt_device_priv_ini_download_pm_param::read download_rate_limit_pps[%d] \
            l_ret[%d]\r\n", us_download_rate_limit_pps, l_ret);
    } else {
        return OAL_FAIL;
    }

    st_syn_msg.en_syn_id = CUSTOM_CFGID_PRIV_INI_DOWNLOAD_RATELIMIT_PPS;
    st_syn_msg.len = sizeof(us_download_rate_limit_pps);

    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - *pul_data_len), &st_syn_msg, CUSTOM_MSG_DATA_HDR_LEN);
    l_ret += memcpy_s(puc_data + CUSTOM_MSG_DATA_HDR_LEN,
                      (WLAN_LARGE_NETBUF_SIZE - *pul_data_len - CUSTOM_MSG_DATA_HDR_LEN),
                      &us_download_rate_limit_pps, sizeof(us_download_rate_limit_pps));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG,
                       "{hwifi_custom_adapt_device_priv_ini_download_pm_param::memcpy_s fail[%d]. data_len[%d]}",
                       l_ret, *pul_data_len);
        *pul_data_len += (sizeof(us_download_rate_limit_pps) + CUSTOM_MSG_DATA_HDR_LEN);

        return OAL_FAIL;
    }

    *pul_data_len += (sizeof(us_download_rate_limit_pps) + CUSTOM_MSG_DATA_HDR_LEN);

    oam_warning_log2(0, OAM_SF_CFG,
                     "{hwifi_custom_adapt_device_priv_ini_download_pm_param::da_len[%d] download_rate_limit [%d]pps.}",
                     *pul_data_len, us_download_rate_limit_pps);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hwifi_custom_adapt_device_ini_param
 * 功能描述  : ini device侧上电前定制化参数适配
 */
OAL_STATIC int32_t hwifi_custom_adapt_device_ini_param(uint8_t *puc_data)
{
    uint32_t data_len = 0;

    if (puc_data == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_param::puc_data is NULL.}");
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
    /* 自动调频 */
    if (hwifi_custom_adapt_device_ini_freq_param(puc_data + data_len, &data_len) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_CFG, "hwifi_custom_adapt_device_ini_param::custom_adapt_device_ini_freq_param fail");
    }

    /* 性能 */
    hwifi_custom_adapt_device_ini_perf_param(puc_data + data_len, &data_len);

    /* linkloss */
    hwifi_custom_adapt_device_ini_linkloss_param(puc_data + data_len, &data_len);

    /* 低功耗 */
    hwifi_custom_adapt_device_ini_pm_switch_param(puc_data + data_len, &data_len);

    /* fast ps mode 检查次数 */
    hwifi_custom_adapt_device_ini_fast_ps_check_cnt(puc_data + data_len, &data_len);

    /* BUCK stay in fastmode */
    hwifi_custom_adapt_device_ini_fast_mode(puc_data + data_len, &data_len);

    /* ldac m2s rssi门限 */
    hwifi_custom_adapt_device_ini_ldac_m2s_rssi_param(puc_data + data_len, &data_len);

    /* ldac m2s rssi门限 */
    hwifi_custom_adapt_device_ini_btcoex_mcm_rssi_param(puc_data + data_len, &data_len);

#ifdef _PRE_WLAN_FEATURE_NRCOEX
    /* nr coex 定制化参数 */
    hwifi_custom_adapt_device_ini_nrcoex_param(puc_data + data_len, &data_len);
#endif

    /* 结束 */
    hwifi_custom_adapt_device_ini_end_param(puc_data + data_len, &data_len);

    return data_len;
}


/*
 * 函 数 名  : hwifi_custom_adapt_device_priv_ini_param_extend
 * 功能描述  : ini device侧上电前定制化参数适配extend
 */
OAL_STATIC void hwifi_custom_adapt_device_priv_ini_param_extend(uint8_t *puc_data, uint32_t data_len, uint32_t *pul_len)
{
    int32_t l_ret;

    hwifi_custom_adapt_device_priv_ini_temper_thread_param(puc_data + data_len, &data_len);
    hwifi_custom_adapt_device_priv_ini_dsss2ofdm_dbb_pwr_bo_param(puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_EVM_PLL_REG_FIX, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_VOE_SWITCH, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_11AX_SWITCH, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_HTC_SWITCH, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_MULTI_BSSID_SWITCH, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_AC_SUSPEND, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_DYNAMIC_DBAC_SWITCH, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_DYN_BYPASS_EXTLNA, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_CTRL_FRAME_TX_CHAIN, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_CTRL_UPC_FOR_18DBM_CO, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_CTRL_UPC_FOR_18DBM_C1, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_CTRL_11B_DOUBLE_CHAIN_BO_POW, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_HCC_FLOWCTRL_TYPE, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_MBO_SWITCH, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_DC_FLOWCTL_SWITCH, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_PHY_CAP_SWITCH, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_OPTIMIZED_FEATURE_SWITCH, puc_data + data_len, &data_len);
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_MCAST_AMPDU_ENABLE, puc_data + data_len, &data_len);
#endif
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_MIRACAST_SINK, puc_data + data_len, &data_len);

    l_ret = hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_FTM_CAP, puc_data + data_len, &data_len);
    if (l_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_priv_ini_param_extend::FTM ini fail.}");
    }

#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
    hwifi_custom_adapt_device_priv_ini_hal_ps_rssi_param(puc_data + data_len, &data_len);
    hwifi_custom_adapt_device_priv_ini_hal_ps_pps_param(puc_data + data_len, &data_len);
#endif
    l_ret = hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_DDR_SWITCH, puc_data + data_len, &data_len);
    if (l_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_priv_ini_param_extend::ini fail.}");
    }
    l_ret = hwifi_custom_adapt_device_priv_ini_hiex_cap(puc_data + data_len, &data_len);
    if (l_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_priv_ini_param_extend::hiex cap ini fail}");
    }

    *pul_len = data_len;
}


OAL_STATIC int32_t hwifi_custom_adapt_device_priv_ini_fem_pow_ctl_param(uint8_t *puc_data,
    uint32_t *pul_data_len)
{
    int32_t l_ret;
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};
    int32_t l_priv_fem_ctl_value = 0;
    int32_t l_priv_fem_adj_value = 0;
    dmac_ax_fem_pow_ctl_stru st_fem_pow_ctl = {0};

    if (puc_data == NULL) {
        oam_error_log1(0, OAM_SF_CUSTOM, "{hwifi_custom_adapt_device_priv_ini_fem_pow_ctl_param::puc_data is \
        NULL data_len[%d].}", *pul_data_len);
        return OAL_FAIL;
    }

    /* 为了私有定制化值 */
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_FEM_DELT_POW, &l_priv_fem_ctl_value);
    l_ret += hwifi_get_init_priv_value(WLAN_CFG_PRIV_FEM_ADJ_TPC_TBL_START_IDX, &l_priv_fem_adj_value);
    if (l_ret != OAL_SUCC) {
        return OAL_FAIL;
    }
    st_fem_pow_ctl.uc_fem_delt_pow = l_priv_fem_ctl_value;
    st_fem_pow_ctl.uc_tpc_adj_pow_start_idx = l_priv_fem_adj_value;
    st_syn_msg.en_syn_id = CUSTOM_CFGID_PRIV_FEM_POW_CTL_ID;
    st_syn_msg.len = sizeof(st_fem_pow_ctl);

    l_ret = memcpy_s(puc_data, WLAN_LARGE_NETBUF_SIZE - *pul_data_len, &st_syn_msg, CUSTOM_MSG_DATA_HDR_LEN);
    l_ret += memcpy_s(puc_data + CUSTOM_MSG_DATA_HDR_LEN,
                      (WLAN_LARGE_NETBUF_SIZE - *pul_data_len) - CUSTOM_MSG_DATA_HDR_LEN,
                      &st_fem_pow_ctl, sizeof(st_fem_pow_ctl));
    if (l_ret != EOK) {
        oam_error_log3(0, OAM_SF_CUSTOM, "{hwifi_custom_adapt_device_priv_ini_fem_pow_ctl_param::memcpy_s fail[%d]. \
        data_len[%d] MAX_len[%d]}", l_ret, *pul_data_len, WLAN_LARGE_NETBUF_SIZE);
        return OAL_FAIL;
    }

    *pul_data_len += sizeof(st_fem_pow_ctl) + CUSTOM_MSG_DATA_HDR_LEN;
    oam_warning_log3(0, OAM_SF_CFG,
                     "{hwifi_custom_adapt_device_priv_ini_fem_pow_ctl_param::len[%d]delt_pow[%d] start_idx[%d].}",
                     *pul_data_len, st_fem_pow_ctl.uc_fem_delt_pow, st_fem_pow_ctl.uc_tpc_adj_pow_start_idx);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hwifi_custom_adapt_device_priv_ini_param
 * 功能描述  : ini device侧上电前定制化参数适配
 */
OAL_STATIC int32_t hwifi_custom_adapt_device_priv_ini_param(uint8_t *puc_data)
{
    uint32_t data_len = 0;

    if (puc_data == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_priv_ini_param::puc_data is NULL.}");
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
    /* 私有定制化 */
    hwifi_custom_adapt_device_priv_ini_radio_cap_param(puc_data, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_BW_MAX_WITH, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_LDPC_CODING, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_RX_STBC, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_TX_STBC, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_SU_BFER, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_SU_BFEE, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_MU_BFER, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_MU_BFEE, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_11N_TXBF, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_1024_QAM, puc_data + data_len, &data_len);
    hwifi_custom_adapt_device_priv_ini_cali_mask_param(puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_CALI_DATA_MASK, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_CALI_AUTOCALI_MASK, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_OVER_TEMP_PRO_ENABLE, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_OVER_TEMP_PRO_REDUCE_PWR_ENABLE,
                                      puc_data + data_len, &data_len);

    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_OVER_TEMP_PRO_SAFE_TH, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_OVER_TEMP_PRO_OVER_TH, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_OVER_TEMP_PRO_PA_OFF_TH, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_FASTSCAN_SWITCH, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_ANT_SWITCH, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_M2S_FUNCTION_MASK, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_M2S_FUNCTION_EXT_MASK, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_MCM_FUNCTION_MASK, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_MCM_CUSTOM_FUNCTION_MASK, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRRIV_LINKLOSS_THRESHOLD_FIXED, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_COUNRTYCODE_SELFSTUDY_CFG, puc_data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_RADAR_ISR_FORBID, puc_data + data_len, &data_len);
    hwifi_custom_adapt_device_priv_ini_download_pm_param(puc_data + data_len, &data_len);
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_TXOPPS_SWITCH, puc_data + data_len, &data_len);
#endif
    hwifi_custom_adapt_device_priv_ini_param_extend(puc_data, data_len, &data_len);
    hwifi_custom_adapt_device_priv_ini_fem_pow_ctl_param(puc_data + data_len, &data_len);
    oam_warning_log2(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_priv_ini_param::da_len[%d]MAX[%d].}",
                     data_len, WLAN_LARGE_NETBUF_SIZE);
    return data_len;
}

static int32_t hwifi_hcc_custom_get_data(uint16_t syn_id, oal_netbuf_stru *netbuf)
{
    uint32_t data_len = 0;
    uint8_t *netbuf_data = (uint8_t *)oal_netbuf_data(netbuf);

    switch (syn_id) {
        case CUSTOM_CFGID_INI_ID:
            /* INI hmac to dmac 配置项 */
            data_len = hwifi_custom_adapt_device_ini_param(netbuf_data);
            break;
        case CUSTOM_CFGID_PRIV_INI_ID:
            /* 私有定制化配置项 */
            data_len = hwifi_custom_adapt_device_priv_ini_param(netbuf_data);
            break;
        default:
            oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_custom_ini_data_buf::unknown us_syn_id[%d]", syn_id);
            break;
    }

    return data_len;
}

/*
 * 函 数 名  : hwifi_hcc_custom_ini_data_buf
 * 功能描述  : 下发定制化配置命令
 */
OAL_STATIC int32_t hwifi_hcc_custom_ini_data_buf(uint16_t us_syn_id)
{
    oal_netbuf_stru *pst_netbuf = NULL;
    uint32_t data_len;
    int32_t l_ret;
    uint32_t max_data_len = hcc_get_max_buf_len();

    struct hcc_transfer_param st_hcc_transfer_param = {0};
    struct hcc_handler *hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hwifi_hcc_custom_ini_data_buf hcc::is is null");
        return -OAL_EFAIL;
    }
    pst_netbuf = hwifi_hcc_custom_netbuf_alloc();
    if (pst_netbuf == NULL) {
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 组netbuf */
    data_len = hwifi_hcc_custom_get_data(us_syn_id, pst_netbuf);
    if (data_len > max_data_len) {
        oam_error_log2(0, OAM_SF_CFG,
            "hwifi_hcc_custom_ini_data_buf::got wrong data_len[%d] max_len[%d]", data_len, max_data_len);
        oal_netbuf_free(pst_netbuf);
        return OAL_FAIL;
    }

    if (data_len == 0) {
        oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_custom_ini_data_buf::data is null us_syn_id[%d]", us_syn_id);
        oal_netbuf_free(pst_netbuf);
        return OAL_SUCC;
    }

    if ((pst_netbuf->data_len) || (pst_netbuf->data == NULL)) {
        oal_io_print("len:%d\r\n", pst_netbuf->data_len);
        oal_netbuf_free(pst_netbuf);
        return OAL_FAIL;
    }

    oal_netbuf_put(pst_netbuf, data_len);
    hcc_hdr_param_init(&st_hcc_transfer_param, HCC_ACTION_TYPE_CUSTOMIZE,
                       us_syn_id,
                       0,
                       HCC_FC_WAIT,
                       DATA_HI_QUEUE);

    l_ret = (uint32_t)hcc_tx(hcc, pst_netbuf, &st_hcc_transfer_param);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_custom_ini_data_buf fail ret[%d]", l_ret);
        oal_netbuf_free(pst_netbuf);
    }

    return l_ret;
}

int32_t hwifi_read_conf_from(uint8_t *puc_buffer_cust_nvram, uint8_t uc_idx)
{
    int32_t l_ret;
#ifdef _PRE_CONFIG_READ_DYNCALI_E2PROM
#ifdef _PRE_SUSPORT_OEMINFO
    l_ret = is_hitv_miniproduct();
    if (l_ret == OAL_SUCC) {
        l_ret = read_conf_from_oeminfo(puc_buffer_cust_nvram, CUS_PARAMS_LEN_MAX,
            g_wifi_nvram_cfg_handler[uc_idx].eeprom_offset);
    } else {
        l_ret = read_conf_from_eeprom(puc_buffer_cust_nvram, CUS_PARAMS_LEN_MAX,
            g_wifi_nvram_cfg_handler[uc_idx].eeprom_offset);
    }
#else
    l_ret = read_conf_from_eeprom(puc_buffer_cust_nvram, CUS_PARAMS_LEN_MAX,
        g_wifi_nvram_cfg_handler[uc_idx].eeprom_offset);
#endif
#else
    l_ret = read_conf_from_nvram(puc_buffer_cust_nvram, CUS_PARAMS_LEN_MAX,
        g_wifi_nvram_cfg_handler[uc_idx].nv_map_idx, g_wifi_nvram_cfg_handler[uc_idx].puc_nv_name);
#endif
    return l_ret;
}

static int32_t hwifi_set_cust_nvram_info(uint8_t *cust_nvram_info, int8_t **pc_token, int32_t *pl_params,
                                         int8_t *pc_ctx, uint8_t idx)
{
    uint8_t *pc_end = ";";
    uint8_t *pc_sep = ",";
    uint8_t param_idx;
    uint8_t times_idx = 0;
    *(cust_nvram_info + (idx * CUS_PARAMS_LEN_MAX * sizeof(uint8_t)) + OAL_STRLEN(*pc_token)) = *pc_end;

     /* 拟合系数获取检查 */
    if (idx <= WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_LOW) {
        /* 二次参数合理性检查 */
        *pc_token = oal_strtok(*pc_token, pc_sep, &pc_ctx);
        param_idx = 0;
        /* 获取定制化系数 */
        while (*pc_token != NULL) {
            /* 将字符串转换成10进制数 */
            *(pl_params + param_idx) = (int32_t)oal_strtol(*pc_token, NULL, 10);
            *pc_token = oal_strtok(NULL, pc_sep, &pc_ctx);
            param_idx++;
        }
        if (param_idx % DY_CALI_PARAMS_TIMES) {
            oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::check NV id[%d]!",
                           g_wifi_nvram_cfg_handler[idx].nv_map_idx);
            memset_s(cust_nvram_info + (idx * CUS_PARAMS_LEN_MAX * sizeof(uint8_t)),
                     CUS_PARAMS_LEN_MAX * sizeof(uint8_t), 0, CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
            return OAL_FALSE;
        }
        times_idx = param_idx / DY_CALI_PARAMS_TIMES;
        /* 二次项系数非0检查 */
        while (times_idx--) {
            if (pl_params[(times_idx)*DY_CALI_PARAMS_TIMES] == 0) {
                oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::check NV id[%d]!",
                               g_wifi_nvram_cfg_handler[idx].nv_map_idx);
                memset_s(cust_nvram_info + (idx * CUS_PARAMS_LEN_MAX * sizeof(uint8_t)),
                         CUS_PARAMS_LEN_MAX * sizeof(uint8_t), 0, CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
                break;
            }
        }

        return OAL_TRUE;
    }
    return OAL_FALSE;
}

static void hwifi_init_custnvram_and_debug_nvram_cfg(uint8_t *cust_nvram_info, uint8_t idx)
{
    memset_s(cust_nvram_info + (idx * CUS_PARAMS_LEN_MAX * sizeof(uint8_t)),
             CUS_PARAMS_LEN_MAX * sizeof(uint8_t), 0,
             CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
    oal_io_print("hwifi_custom_host_read_dyn_cali_nvram::NVRAM get fail NV id[%d] name[%s] para[%s]!\r\n",
                 g_wifi_nvram_cfg_handler[idx].nv_map_idx,
                 g_wifi_nvram_cfg_handler[idx].puc_nv_name,
                 g_wifi_nvram_cfg_handler[idx].puc_param_name);
}

static uint32_t hwifi_read_conf_and_set_cust_nvram(uint8_t *buffer_cust_nvram_tmp, uint8_t idx,
                                                   uint8_t *cust_nvram_info, int32_t *params,
                                                   oal_bool_enum_uint8 *tmp_fact_cali_completed)
{
    int32_t ret;
    uint8_t *end = ";";
    int8_t *str = NULL;
    int8_t *ctx = NULL;
    int8_t *token = NULL;

    ret = hwifi_read_conf_from(buffer_cust_nvram_tmp, idx);
    if (ret != INI_SUCC) {
        hwifi_init_custnvram_and_debug_nvram_cfg(cust_nvram_info, idx);
        return OAL_TRUE;
    }

    str = OAL_STRSTR(buffer_cust_nvram_tmp, g_wifi_nvram_cfg_handler[idx].puc_param_name);
    if (str == NULL) {
        hwifi_init_custnvram_and_debug_nvram_cfg(cust_nvram_info, idx);
        return OAL_TRUE;
    }

    /* 获取等号后面的实际参数 */
    str += (OAL_STRLEN(g_wifi_nvram_cfg_handler[idx].puc_param_name) + 1);
    token = oal_strtok(str, end, &ctx);
    if (token == NULL) {
        oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::get null value check NV id[%d]!",
                       g_wifi_nvram_cfg_handler[idx].nv_map_idx);
        hwifi_init_custnvram_and_debug_nvram_cfg(cust_nvram_info, idx);
        return OAL_TRUE;
    }

    ret = memcpy_s(cust_nvram_info + (idx * CUS_PARAMS_LEN_MAX * sizeof(uint8_t)),
                   (WLAN_CFG_DTS_NVRAM_END - idx) * CUS_PARAMS_LEN_MAX * sizeof(uint8_t),
                   token, OAL_STRLEN(token));
    if (ret != EOK) {
        oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::memcpy_s fail[%d]!", ret);
        return OAL_FALSE;
    }

    if (hwifi_set_cust_nvram_info(cust_nvram_info, &token, params, ctx, idx) == OAL_TRUE) {
        *tmp_fact_cali_completed = OAL_TRUE;
    }

    return OAL_TRUE;
}

/*
 * 函 数 名  : hwifi_custom_host_read_dyn_cali_nvram
 * 功能描述  : 包括读取nvram中的dpint和校准系数值
 */
OAL_STATIC int32_t hwifi_custom_host_read_dyn_cali_nvram(void)
{
    int32_t l_ret;
    uint8_t uc_idx;
    int32_t l_priv_value;
    oal_bool_enum_uint8 en_get_nvram_data_flag = OAL_FALSE;
    uint8_t *puc_buffer_cust_nvram_tmp = NULL;
    int32_t *pl_params = NULL;
    uint8_t *puc_g_cust_nvram_info = NULL; /* NVRAM数组 */
    oal_bool_enum_uint8 tmp_en_fact_cali_completed = OAL_FALSE;

    /* 判断定制化中是否使用nvram中的动态校准参数 */
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_CALI_DATA_MASK, &l_priv_value);
    if (l_ret == OAL_SUCC) {
        en_get_nvram_data_flag = !!(CUST_READ_NVRAM_MASK & (uint32_t)l_priv_value);
        if (en_get_nvram_data_flag) {
            oal_io_print("hwifi_custom_host_read_dyn_cali_nvram::get_nvram_data_flag[%d] to abandon nvram data!!\r\n",
                         l_priv_value);
            memset_s(g_cust_nvram_info, sizeof(g_cust_nvram_info), 0, sizeof(g_cust_nvram_info));
            return INI_FILE_TIMESPEC_UNRECONFIG;
        }
    }

    puc_buffer_cust_nvram_tmp = (uint8_t *)os_kzalloc_gfp(CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
    if (puc_buffer_cust_nvram_tmp == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM,
                       "hwifi_custom_host_read_dyn_cali_nvram::puc_buffer_cust_nvram_tmp mem alloc fail!");
        memset_s(g_cust_nvram_info, sizeof(g_cust_nvram_info), 0, sizeof(g_cust_nvram_info));
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    pl_params = (int32_t *)os_kzalloc_gfp(DY_CALI_PARAMS_NUM * DY_CALI_PARAMS_TIMES * sizeof(int32_t));
    if (pl_params == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::ps_params mem alloc fail!");
        os_mem_kfree(puc_buffer_cust_nvram_tmp);
        memset_s(g_cust_nvram_info, sizeof(g_cust_nvram_info), 0, sizeof(g_cust_nvram_info));
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    puc_g_cust_nvram_info = (uint8_t *)os_kzalloc_gfp(WLAN_CFG_DTS_NVRAM_END *
                                                      CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
    if (puc_g_cust_nvram_info == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::puc_g_cust_nvram_info mem alloc fail!");
        os_mem_kfree(puc_buffer_cust_nvram_tmp);
        os_mem_kfree(pl_params);
        memset_s(g_cust_nvram_info, sizeof(g_cust_nvram_info), 0, sizeof(g_cust_nvram_info));
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    memset_s(puc_buffer_cust_nvram_tmp,
             CUS_PARAMS_LEN_MAX * sizeof(uint8_t),
             0,
             CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
    memset_s(pl_params,
             DY_CALI_PARAMS_NUM * DY_CALI_PARAMS_TIMES * sizeof(int32_t),
             0,
             DY_CALI_PARAMS_NUM * DY_CALI_PARAMS_TIMES * sizeof(int32_t));
    memset_s(puc_g_cust_nvram_info,
             WLAN_CFG_DTS_NVRAM_END * CUS_PARAMS_LEN_MAX * sizeof(uint8_t),
             0,
             WLAN_CFG_DTS_NVRAM_END * CUS_PARAMS_LEN_MAX * sizeof(uint8_t));

    /* 拟合系数 */
    for (uc_idx = WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA0; uc_idx < WLAN_CFG_DTS_NVRAM_END; uc_idx++) {
        if (hwifi_read_conf_and_set_cust_nvram(puc_buffer_cust_nvram_tmp, uc_idx, puc_g_cust_nvram_info,
                                               pl_params, &tmp_en_fact_cali_completed) != OAL_TRUE) {
            break;
        }
    }

    g_en_fact_cali_completed = tmp_en_fact_cali_completed;

    os_mem_kfree(puc_buffer_cust_nvram_tmp);
    os_mem_kfree(pl_params);

    /* 检查NVRAM是否修改 */
    if (oal_memcmp(puc_g_cust_nvram_info, g_cust_nvram_info, sizeof(g_cust_nvram_info)) == 0) {
        os_mem_kfree(puc_g_cust_nvram_info);
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    if (memcpy_s(g_cust_nvram_info, sizeof(g_cust_nvram_info),
                 puc_g_cust_nvram_info, WLAN_CFG_DTS_NVRAM_END * CUS_PARAMS_LEN_MAX * sizeof(uint8_t)) != EOK) {
        os_mem_kfree(puc_g_cust_nvram_info);
        return INI_FAILED;
    }

    os_mem_kfree(puc_g_cust_nvram_info);

    return INI_NVRAM_RECONFIG;
}

uint8_t *hwifi_get_nvram_param(uint32_t nvram_param_idx)
{
    return g_cust_nvram_info[nvram_param_idx];
}

/*
 * 函 数 名  : custom_host_read_cfg_init
 * 功能描述  : 首次读取定制化配置文件总入口
 */
uint32_t hwifi_custom_host_read_cfg_init_1103(void)
{
    int32_t l_nv_read_ret;
    int32_t l_ini_read_ret;

    hwifi_get_country_code();
    /* 先获取私有定制化项 */
    if (hwifi_config_init(CUS_TAG_PRIV_INI) != INI_SUCC) {
        oam_error_log0(0, OAM_SF_CFG, "hwifi_custom_host_read_cfg_init::hwifi_config_init fail");
    }

    /* 读取nvram参数是否修改 */
    l_nv_read_ret = hwifi_custom_host_read_dyn_cali_nvram();
    /* 检查定制化文件中的产线配置是否修改 */
    l_ini_read_ret = ini_file_check_conf_update();
    if (l_ini_read_ret || l_nv_read_ret) {
        oam_warning_log0(0, OAM_SF_CFG, "hwifi_custom_host_read_cfg_init config is updated");
        hwifi_config_init(CUS_TAG_PRO_LINE_INI);
    }

    if (l_ini_read_ret == INI_FILE_TIMESPEC_UNRECONFIG) {
        oam_warning_log0(0, OAM_SF_CFG, "hwifi_custom_host_read_cfg_init file is not updated");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (hwifi_config_init(CUS_TAG_DTS) != INI_SUCC) {
        oam_error_log0(0, OAM_SF_CFG, "hwifi_custom_host_read_cfg_init::hwifi_config_init fail");
    }
    l_ini_read_ret = hwifi_config_init(CUS_TAG_NV);
    if (oal_unlikely(l_ini_read_ret != OAL_SUCC)) {
        oal_io_print("hwifi_custom_host_read_cfg_init NV fail l_ret[%d].\r\n", l_ini_read_ret);
    }

    if (hwifi_config_init(CUS_TAG_INI) != INI_SUCC) {
        oam_error_log0(0, OAM_SF_CFG, "hwifi_custom_host_read_cfg_init::hwifi_config_init fail");
    }

    /* 启动完成后，输出打印 */
    oal_io_print("hwifi_custom_host_read_cfg_init finish!\r\n");

    return OAL_SUCC;
}

/*
 * 函 数 名  : hwifi_hcc_customize_h2d_data_cfg
 * 功能描述  : 协议栈初始化前定制化配置入口
 */
uint32_t hwifi_hcc_customize_h2d_data_cfg_1103(void)
{
    int32_t l_ret;

    /* wifi上电时重读定制化配置 */
    l_ret = hwifi_custom_host_read_cfg_init_1103();
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "hwifi_hcc_customize_h2d_data_cfg data ret[%d]", l_ret);
    }

    // 如果不成功，返回失败
    l_ret = hwifi_hcc_custom_ini_data_buf(CUSTOM_CFGID_PRIV_INI_ID);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_customize_h2d_data_cfg priv data fail, ret[%d]", l_ret);
        return INI_FAILED;
    }

    l_ret = hwifi_hcc_custom_ini_data_buf(CUSTOM_CFGID_INI_ID);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_customize_h2d_data_cfg ini data fail, ret[%d]", l_ret);
        return INI_FAILED;
    }
    return INI_SUCC;
}

/*
 * 函 数 名  : hwifi_config_fcc_ce_5g_high_band_txpwr_nvram
 * 功能描述  : FCC/CE 5G高band认证
 */
OAL_STATIC void hwifi_config_fcc_ce_5g_high_band_txpwr_nvram(regdomain_enum regdomain_type)
{
    uint8_t uc_5g_max_pwr_for_high_band;
    int32_t l_val = g_nvram_init_params[NVRAM_PARAMS_5G_FCC_CE_HIGH_BAND_MAX_PWR];
    wlan_cust_nvram_params *pst_g_cust_nv_params = hwifi_get_nvram_params();

    /* FCC/CE 5G 高band的最大发射功率 */
    if (get_cust_conf_int32(INI_MODU_WIFI,
                            g_nvram_config_ini[NVRAM_PARAMS_5G_FCC_CE_HIGH_BAND_MAX_PWR].name,
                            &l_val) != INI_SUCC) {
        /* 读取失败时,使用初始值 */
        l_val = g_nvram_init_params[NVRAM_PARAMS_5G_FCC_CE_HIGH_BAND_MAX_PWR];
    }

    uc_5g_max_pwr_for_high_band = (uint8_t)((regdomain_type == REGDOMAIN_ETSI) ?
                                              cus_get_low_16bits(l_val) : cus_get_high_16bits(l_val));
    /* 参数有效性检查 */
    if (cus_val_invalid(uc_5g_max_pwr_for_high_band, CUS_MAX_BASE_TXPOWER_VAL, CUS_MIN_BASE_TXPOWER_VAL)) {
        oam_warning_log1(0, OAM_SF_CFG, "hwifi_config_init_nvram read 5g_max_pow_high_band[%d] failed!", l_val);
        uc_5g_max_pwr_for_high_band = CUS_MAX_BASE_TXPOWER_VAL;
    }

    pst_g_cust_nv_params->uc_5g_max_pwr_fcc_ce_for_high_band = uc_5g_max_pwr_for_high_band;
}

/*
 * 函 数 名  : hwifi_config_init_iq_lpf_nvram_param
 * 功能描述  : iq校准lpf档位
 */
OAL_STATIC void hwifi_config_init_iq_lpf_nvram_param(void)
{
    int32_t l_nvram_params = 0;
    int32_t l_cfg_id = NVRAM_PARAMS_INDEX_IQ_LPF_LVL;
    int32_t l_ret;
    wlan_cust_nvram_params *pst_g_cust_nv_params = hwifi_get_nvram_params();

    l_ret = get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[l_cfg_id].name, &l_nvram_params);
    if (l_ret != INI_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "hwifi_config_init_iq_lpf_nvram_param read id[%d] from ini failed!", l_cfg_id);

        /* 读取失败时,使用初始值 */
        l_nvram_params = g_nvram_init_params[l_cfg_id];
    }

    /*  5g iq cali lvl  */
    l_ret += memcpy_s(pst_g_cust_nv_params->auc_5g_iq_cali_lpf_params,
                      NUM_OF_NV_5G_LPF_LVL, &l_nvram_params, sizeof(l_nvram_params));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "hwifi_config_init_iq_lpf_nvram_param memcpy failed!");
        return;
    }
}
/*
 * 函 数 名  : hwifi_config_init_fcc_ce_txpwr_nvram
 * 功能描述  : FCC/CE认证
 */
OAL_STATIC int32_t hwifi_config_init_fcc_ce_txpwr_nvram(uint8_t uc_chn_idx)
{
    int32_t l_ret = INI_FAILED;
    uint8_t uc_cfg_id;
    uint8_t uc_param_idx = 0;
    int32_t *pl_nvram_params = NULL;
    int32_t *pl_fcc_ce_txpwr_limit_params = NULL;
    regdomain_enum regdomain_type;
    uint8_t uc_param_len;
    uint8_t uc_start_idx = 0;
    uint8_t uc_end_idx = 0;
    wlan_cust_nvram_params *pst_g_cust_nv_params = hwifi_get_nvram_params();  /* 最大发送功率定制化数组 */
    wlan_cust_cfg_custom_fcc_ce_txpwr_limit_stru *pst_fcc_ce_param = NULL;

    /* 获取管制域信息 */
    regdomain_type = hwifi_get_regdomain_from_country_code((uint8_t *)g_wifi_country_code);
    /* 根据管制域信息选择下发FCC还是CE参数 */
    hwifi_config_fcc_ce_5g_high_band_txpwr_nvram(regdomain_type);

    if (uc_chn_idx == WLAN_RF_CHANNEL_ZERO) {
        if (regdomain_type == REGDOMAIN_ETSI) {
            uc_start_idx = NVRAM_PARAMS_CE_START_INDEX;
            uc_end_idx = NVRAM_PARAMS_CE_END_INDEX_BUTT;
        } else {
            uc_start_idx = NVRAM_PARAMS_FCC_START_INDEX;
            uc_end_idx = NVRAM_PARAMS_FCC_END_INDEX_BUTT;
        }
    } else {
        if (regdomain_type == REGDOMAIN_ETSI) {
            uc_start_idx = NVRAM_PARAMS_CE_C1_START_INDEX;
            uc_end_idx = NVRAM_PARAMS_CE_C1_END_INDEX_BUTT;
        } else {
            uc_start_idx = NVRAM_PARAMS_FCC_C1_START_INDEX;
            uc_end_idx = NVRAM_PARAMS_FCC_C1_END_INDEX_BUTT;
        }
    }

    uc_param_len = (uc_end_idx - uc_start_idx) * sizeof(int32_t);
    pl_fcc_ce_txpwr_limit_params = (int32_t *)os_kzalloc_gfp(uc_param_len);
    if (pl_fcc_ce_txpwr_limit_params == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_config_init_fcc_txpwr_nvram::pl_nvram_params mem alloc fail!");
        return INI_FAILED;
    }
    memset_s(pl_fcc_ce_txpwr_limit_params, uc_param_len, 0, uc_param_len);
    pl_nvram_params = pl_fcc_ce_txpwr_limit_params;

    for (uc_cfg_id = uc_start_idx; uc_cfg_id < uc_end_idx; uc_cfg_id++) {
        l_ret = get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[uc_cfg_id].name, pl_nvram_params + uc_param_idx);
        oal_io_print("{hwifi_config_init_fcc_txpwr_nvram params[%d]=0x%x!\r\n}",
                     uc_param_idx, pl_nvram_params[uc_param_idx]);

        if (l_ret != INI_SUCC) {
            oam_warning_log1(0, OAM_SF_CFG, "hwifi_config_init_nvram read id[%d] from ini failed!", uc_cfg_id);
            /* 读取失败时,使用初始值 */
            pl_nvram_params[uc_param_idx] = g_nvram_init_params[uc_cfg_id];
        }
        uc_param_idx++;
    }

    pst_fcc_ce_param = &pst_g_cust_nv_params->ast_fcc_ce_param[uc_chn_idx];
    /* 5g */
    l_ret = memcpy_s(pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_20m,
        sizeof(pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_20m), pl_nvram_params, sizeof(int32_t));
    pl_nvram_params++;
    /* 偏移已经拷贝过数据的4字节，剩余长度是buff总长度减去已拷贝过数据的6-4=2字节 */
    l_ret += memcpy_s(pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_20m + sizeof(int32_t),
                      (sizeof(pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_20m) - sizeof(int32_t)),
                      pl_nvram_params, 2 * sizeof(uint8_t)); /* 2字节 */
    pl_nvram_params++;
    l_ret += memcpy_s(pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_40m,
        sizeof(pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_40m), pl_nvram_params, sizeof(int32_t));
    pl_nvram_params++;
    /* 偏移已经拷贝过数据的4字节，剩余长度是buff总长度减去已拷贝过数据的6-4=2字节 */
    l_ret += memcpy_s(pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_40m + sizeof(int32_t),
                      (sizeof(pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_40m) - sizeof(int32_t)),
                      pl_nvram_params, 2 * sizeof(uint8_t)); /* 2字节 */
    pl_nvram_params++;
    l_ret += memcpy_s(pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_80m,
        sizeof(pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_80m), pl_nvram_params, sizeof(int32_t));
    pl_nvram_params++;
    l_ret += memcpy_s(pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_80m + sizeof(int32_t),
                      (sizeof(pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_80m) - sizeof(int32_t)),
                      pl_nvram_params, sizeof(uint8_t));
    pl_nvram_params++;
    l_ret += memcpy_s(pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_160m,
                      sizeof(pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_160m), pl_nvram_params,
                      CUS_NUM_5G_160M_SIDE_BAND * sizeof(uint8_t));
    if (l_ret != EOK) {
        oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_config_init_fcc_txpwr_nvram::memcpy_s fail[%d]!", l_ret);
        os_mem_kfree(pl_fcc_ce_txpwr_limit_params);
        return INI_FAILED;
    }

    /* 2.4g */
    for (uc_cfg_id = 0; uc_cfg_id < MAC_2G_CHANNEL_NUM; uc_cfg_id++) {
        pl_nvram_params++;
        l_ret = memcpy_s(pst_fcc_ce_param->auc_2g_fcc_txpwr_limit_params[uc_cfg_id],
            CUS_NUM_FCC_CE_2G_PRO * sizeof(uint8_t), pl_nvram_params, CUS_NUM_FCC_CE_2G_PRO * sizeof(uint8_t));
        if (l_ret != EOK) {
            oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_config_init_fcc_txpwr_nvram::memcpy_s fail[%d]!", l_ret);
            os_mem_kfree(pl_fcc_ce_txpwr_limit_params);
            return INI_FAILED;
        }
    }

    os_mem_kfree(pl_fcc_ce_txpwr_limit_params);
    return INI_SUCC;
}

/*
 * 函 数 名  : hwifi_config_check_sar_ctrl_nvram
 * 功能描述  : 降SAR参数检查
 */
OAL_STATIC void hwifi_config_check_sar_ctrl_nvram(uint8_t *puc_nvram_params, uint8_t uc_cfg_id,
                                                  uint8_t uc_band_id, uint8_t uc_chn_idx)
{
    uint8_t uc_sar_lvl_idx;
    wlan_init_cust_nvram_params *pst_g_cust_nv_params = hwifi_get_init_nvram_params();

    for (uc_sar_lvl_idx = 0; uc_sar_lvl_idx < CUS_NUM_OF_SAR_ONE_PARAM_NUM; uc_sar_lvl_idx++) {
        /* 定制项检查 */
        if (puc_nvram_params[uc_sar_lvl_idx] <= CUS_MIN_OF_SAR_VAL) {
            oam_error_log4(0, OAM_SF_CUSTOM, "hwifi_config_check_sar_ctrl_nvram::uc_cfg_id[%d]uc_band_id[%d] val[%d] \
                abnormal check ini file for chn[%d]!", uc_cfg_id, uc_band_id, puc_nvram_params[uc_sar_lvl_idx],
                uc_chn_idx);
            puc_nvram_params[uc_sar_lvl_idx] = 0xFF;
        }

        if (uc_chn_idx == WLAN_RF_CHANNEL_ZERO) {
            pst_g_cust_nv_params->st_sar_ctrl_params[uc_sar_lvl_idx + uc_cfg_id *
                CUS_NUM_OF_SAR_ONE_PARAM_NUM][uc_band_id].auc_sar_ctrl_params_c0 = puc_nvram_params[uc_sar_lvl_idx];
        } else {
            pst_g_cust_nv_params->st_sar_ctrl_params[uc_sar_lvl_idx + uc_cfg_id *
                CUS_NUM_OF_SAR_ONE_PARAM_NUM][uc_band_id].auc_sar_ctrl_params_c1 = puc_nvram_params[uc_sar_lvl_idx];
        }
    }
}

/*
 * 函 数 名  : hwifi_config_init_sar_ctrl_nvram
 * 功能描述  : 降SAR
 */
OAL_STATIC int32_t hwifi_config_init_sar_ctrl_nvram(uint8_t uc_chn_idx)
{
    int32_t  l_ret = INI_FAILED;
    uint8_t  uc_cfg_id;
    uint8_t  uc_band_id;
    uint8_t  uc_cus_id = uc_chn_idx == WLAN_RF_CHANNEL_ZERO ? NVRAM_PARAMS_SAR_START_INDEX :
                                                                NVRAM_PARAMS_SAR_C1_START_INDEX;
    uint32_t nvram_params = 0;
    uint8_t  auc_nvram_params[CUS_NUM_OF_SAR_ONE_PARAM_NUM];

    for (uc_cfg_id = 0; uc_cfg_id < CUS_NUM_OF_SAR_PER_BAND_PAR_NUM; uc_cfg_id++) {
        for (uc_band_id = 0; uc_band_id < CUS_NUM_OF_SAR_PARAMS; uc_band_id++) {
            l_ret = get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[uc_cus_id].name, &nvram_params);
            if (l_ret != INI_SUCC) {
                oam_warning_log1(0, OAM_SF_CUSTOM, "hwifi_config_init_sar_ctrl_nvram read id[%d] from ini failed!",
                                 uc_cus_id);
                /* 读取失败时,使用初始值 */
                nvram_params = 0xFFFFFFFF;
            }
            oal_io_print("{hwifi_config_init_sar_ctrl_nvram::chn[%d] params %s 0x%x!\r\n}",
                         uc_chn_idx, g_nvram_config_ini[uc_cus_id].name, nvram_params);
            if (memcpy_s(auc_nvram_params, sizeof(auc_nvram_params),
                         &nvram_params, sizeof(nvram_params)) != EOK) {
                oam_error_log3(0, OAM_SF_CUSTOM,
                               "hwifi_config_init_sar_ctrl_nvram::uc_cfg_id[%d]band_id[%d]param[%d] set failed!",
                               uc_cfg_id, uc_band_id, nvram_params);
                return INI_FAILED;
            }

            /* 定制项检查 */
            hwifi_config_check_sar_ctrl_nvram(auc_nvram_params, uc_cfg_id, uc_band_id, uc_chn_idx);
            uc_cus_id++;
        }
    }
    return INI_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
/*
 * 函 数 名  : hwifi_config_init_tas_ctrl_nvram
 * 功能描述  : tas发射功率
 */
OAL_STATIC int32_t hwifi_config_init_tas_ctrl_nvram(void)
{
    int32_t l_ret;
    uint8_t uc_band_idx;
    uint8_t uc_rf_idx;
    uint32_t nvram_params = 0;
    int8_t ac_tas_ctrl_params[WLAN_BAND_BUTT][WLAN_RF_CHANNEL_NUMS] = {{0}};
    wlan_cust_nvram_params *pst_g_cust_nv_params = hwifi_get_nvram_params();

    l_ret = get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[NVRAM_PARAMS_TAS_ANT_SWITCH_EN].name,
                                &nvram_params);
    if (l_ret == INI_SUCC) {
        oam_warning_log1(0, OAM_SF_CUSTOM, "hwifi_config_init_tas_ctrl_nvram g_tas_switch_en[%d]!", nvram_params);
        g_tas_switch_en[WLAN_RF_CHANNEL_ZERO] = (oal_bool_enum_uint8)cus_get_low_16bits(nvram_params);
        g_tas_switch_en[WLAN_RF_CHANNEL_ONE] = (oal_bool_enum_uint8)cus_get_high_16bits(nvram_params);
    }

    l_ret = get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[NVRAM_PARAMS_TAS_PWR_CTRL].name, &nvram_params);
    oal_io_print("{hwifi_config_init_tas_ctrl_nvram params[%d]=0x%x!\r\n}", NVRAM_PARAMS_TAS_PWR_CTRL, nvram_params);
    if (l_ret != INI_SUCC) {
        oam_warning_log1(0, OAM_SF_CUSTOM, "hwifi_config_init_tas_ctrl_nvram read id[%d] from ini failed!",
                         NVRAM_PARAMS_TAS_PWR_CTRL);
        /* 读取失败时,使用初始值 */
        nvram_params = g_nvram_init_params[NVRAM_PARAMS_TAS_PWR_CTRL];
    }

    if (memcpy_s(ac_tas_ctrl_params, sizeof(ac_tas_ctrl_params),
                 &nvram_params, sizeof(nvram_params)) != EOK) {
        return INI_FAILED;
    }

    for (uc_band_idx = 0; uc_band_idx < WLAN_BAND_BUTT; uc_band_idx++) {
        for (uc_rf_idx = 0; uc_rf_idx < WLAN_RF_CHANNEL_NUMS; uc_rf_idx++) {
            if ((ac_tas_ctrl_params[uc_band_idx][uc_rf_idx] > CUS_MAX_OF_TAS_PWR_CTRL_VAL) ||
                (ac_tas_ctrl_params[uc_rf_idx][uc_rf_idx] < 0)) {
                oam_error_log4(0, OAM_SF_CUSTOM, "hwifi_config_init_tas_ctrl_nvram::band[%d] rf[%d] nvram_params[%d],\
                    val[%d] out of the \normal check ini file!", uc_band_idx, uc_rf_idx, nvram_params,
                    ac_tas_ctrl_params[uc_rf_idx][uc_rf_idx]);
                ac_tas_ctrl_params[uc_band_idx][uc_rf_idx] = 0;
            }
            pst_g_cust_nv_params->auc_tas_ctrl_params[uc_rf_idx][uc_band_idx] =
                (uint8_t)ac_tas_ctrl_params[uc_band_idx][uc_rf_idx];
        }
    }

    return INI_SUCC;
}
#endif

/*
 * 函 数 名  : hwifi_config_sepa_coefficient_from_param
 * 功能描述  : 从字符串中分割二次系数项
 */
OAL_STATIC uint32_t hwifi_config_sepa_coefficient_from_param(uint8_t *puc_cust_param_info, int32_t *pl_coe_params,
                                                             uint16_t *pus_param_num, uint16_t us_max_idx)
{
    int8_t *pc_token = NULL;
    int8_t *pc_ctx = NULL;
    int8_t *pc_end = ";";
    int8_t *pc_sep = ",";
    uint16_t us_param_num = 0;
    uint8_t auc_cust_param[CUS_PARAMS_LEN_MAX];

    if (memcpy_s(auc_cust_param, CUS_PARAMS_LEN_MAX, puc_cust_param_info, OAL_STRLEN(puc_cust_param_info)) != EOK) {
        return OAL_FAIL;
    }

    pc_token = oal_strtok(auc_cust_param, pc_end, &pc_ctx);
    if (pc_token == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_config_sepa_coefficient_from_param read get null value check!");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pc_token = oal_strtok(pc_token, pc_sep, &pc_ctx);
    /* 获取定制化系数 */
    while (pc_token != NULL) {
        if (us_param_num == us_max_idx) {
            oam_error_log2(0, OAM_SF_CUSTOM,
                           "hwifi_config_sepa_coefficient_from_param::nv or ini param is too many idx[%d] Max[%d]",
                           us_param_num, us_max_idx);
            return OAL_FAIL;
        }
        /* 将字符串转换成10进制数 */
        *(pl_coe_params + us_param_num) = (int32_t)oal_strtol(pc_token, NULL, 10);
        pc_token = oal_strtok(NULL, pc_sep, &pc_ctx);
        us_param_num++;
    }

    *pus_param_num = us_param_num;
    return OAL_SUCC;
}

/*
 * 函 数 名  : hwifi_get_max_txpwr_base
 * 功能描述  : 获取定制化基准发射功率
 */
OAL_STATIC void hwifi_get_max_txpwr_base(const char *tag, uint8_t uc_nvram_base_param_idx,
                                         uint8_t *puc_txpwr_base_params, uint8_t uc_param_num)
{
    uint8_t uc_param_idx;
    int32_t l_ret = INI_FAILED;
    uint8_t *puc_base_pwr_params = NULL;
    uint16_t us_per_param_num = 0;
    int32_t l_nv_params[DY_CALI_NUM_5G_BAND] = {0};

    puc_base_pwr_params = (uint8_t *)os_kzalloc_gfp(CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
    if (puc_base_pwr_params == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_get_max_txpwr_base::puc_base_pwr_params mem alloc fail!");
        l_ret = INI_FAILED;
    } else {
        memset_s(puc_base_pwr_params,
                 CUS_PARAMS_LEN_MAX * sizeof(uint8_t),
                 0,
                 CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
        l_ret = get_cust_conf_string(tag, g_nvram_config_ini[uc_nvram_base_param_idx].name,
                                     puc_base_pwr_params, CUS_PARAMS_LEN_MAX - 1);
        if (l_ret == INI_SUCC) {
            if ((hwifi_config_sepa_coefficient_from_param(puc_base_pwr_params, l_nv_params,
                                                          &us_per_param_num, uc_param_num) == OAL_SUCC) &&
                (us_per_param_num == uc_param_num)) {
                /* 参数合理性检查 */
                for (uc_param_idx = 0; uc_param_idx < uc_param_num; uc_param_idx++) {
                    if (cus_val_invalid(l_nv_params[uc_param_idx],
                        CUS_MAX_BASE_TXPOWER_VAL, CUS_MIN_BASE_TXPOWER_VAL)) {
                        oam_error_log3(0, OAM_SF_CUSTOM,
                            "hwifi_get_max_txpwr_base read %dth from ini val[%d] out of range replaced by [%d]!",
                            uc_nvram_base_param_idx, l_nv_params[uc_param_idx], CUS_MAX_BASE_TXPOWER_VAL);
                        *(puc_txpwr_base_params + uc_param_idx) = CUS_MAX_BASE_TXPOWER_VAL;
                    } else {
                        *(puc_txpwr_base_params + uc_param_idx) = (uint8_t)l_nv_params[uc_param_idx];
                    }
                }
                os_mem_kfree(puc_base_pwr_params);
                return;
            }
        } else {
            oam_error_log2(0, OAM_SF_CUSTOM,
                           "hwifi_get_max_txpwr_base::read %dth failed ret[%d] check ini files!",
                           uc_nvram_base_param_idx, l_ret);
        }
    }

    if (l_ret != INI_SUCC) {
        /* 失败默认使用初始值 */
        for (uc_param_idx = 0; uc_param_idx < uc_param_num; uc_param_idx++) {
            *(puc_txpwr_base_params + uc_param_idx) = CUS_MAX_BASE_TXPOWER_VAL;
        }

        oam_error_log2(0, OAM_SF_CFG, "hwifi_get_max_txpwr_base read failed ret[%d] replaced by ini_val[%d]!",
                       l_ret, CUS_MAX_BASE_TXPOWER_VAL);
    }
    os_mem_kfree(puc_base_pwr_params);
    return;
}

/*
 * 函 数 名  : hwifi_get_max_txpwr_base_delt_val
 * 功能描述  : 获取mimo定制化基准发射功率差
 */
static void hwifi_get_max_txpwr_base_delt_val(uint8_t uc_cus_id,
                                              int8_t *pc_txpwr_base_delt_params,
                                              uint8_t uc_param_num)
{
    uint8_t  uc_param_idx;
    int32_t  nvram_params  = 0;

    memset_s(pc_txpwr_base_delt_params, uc_param_num, 0, uc_param_num);
    if (INI_SUCC == get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[uc_cus_id].name, &nvram_params)) {
        if (memcpy_s(pc_txpwr_base_delt_params, uc_param_num, &nvram_params, sizeof(nvram_params)) != EOK) {
            oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_get_max_txpwr_base_delt_val::copy failed!");
            return;
        }

        /* 参数合理性检查 */
        for (uc_param_idx = 0; uc_param_idx < uc_param_num; uc_param_idx++) {
            if (cus_val_invalid(pc_txpwr_base_delt_params[uc_param_idx], 0, CUS_MAX_BASE_TXPOWER_DELT_VAL_MIN)) {
                        oam_error_log1(0, OAM_SF_CUSTOM,
                                       "hwifi_get_max_txpwr_base_delt_val::ini val[%d] out of range replaced by zero!",
                                       pc_txpwr_base_delt_params[uc_param_idx]);
                        pc_txpwr_base_delt_params[uc_param_idx] = 0;
            }
        }
    }

    return;
}


static void hwifi_get_cfg_delt_ru_pow_params(void)
{
    int32_t l_cfg_idx_one = 0;
    int32_t l_cfg_idx_two = 0;
    wlan_cust_nvram_params *pst_g_cust_nv_params = hwifi_get_nvram_params();

    for (l_cfg_idx_one = 0; l_cfg_idx_one < WLAN_BW_CAP_BUTT; l_cfg_idx_one++) {
        for (l_cfg_idx_two = 0; l_cfg_idx_two < WLAN_HE_RU_SIZE_BUTT; l_cfg_idx_two++) {
            oal_io_print("%s[%d][%d]:%d \n", "5g_cfg_tpc_ru_pow", l_cfg_idx_one, l_cfg_idx_two,
                pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g[l_cfg_idx_one][l_cfg_idx_two]);
        }
    }
    for (l_cfg_idx_one = 0; l_cfg_idx_one < WLAN_BW_CAP_80M; l_cfg_idx_one++) {
        for (l_cfg_idx_two = 0; l_cfg_idx_two < WLAN_HE_RU_SIZE_996; l_cfg_idx_two++) {
            oal_io_print("%s[%d][%d]:%d \n", "2g_cfg_tpc_ru_pow", l_cfg_idx_one, l_cfg_idx_two,
                pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_2g[l_cfg_idx_one][l_cfg_idx_two]);
        }
    }
}


static void hwifi_get_delt_ru_pow_val_5g(void)
{
    int32_t l_nvram_params = 0;
    int32_t l_ret;
    int32_t l_cfg_idx_one;
    int32_t l_cfg_idx_two;
    wlan_cust_nvram_params *pst_g_cust_nv_params = hwifi_get_nvram_params();

    /* 20m 26~242tone ru_power */
    if (get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[NVRAM_CFG_TPC_RU_POWER_20M_5G].name, &l_nvram_params) != INI_SUCC) {
        l_nvram_params = g_nvram_init_params[NVRAM_CFG_TPC_RU_POWER_20M_5G];
    }
    l_ret = memcpy_s(pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g[WLAN_BW_CAP_20M], WLAN_HE_RU_SIZE_BUTT, \
                     &l_nvram_params, sizeof(l_nvram_params));

    /* 40m 26~242tone ru_power */
    if (get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[NVRAM_CFG_TPC_RU_POWER_40M_5G].name, &l_nvram_params) != INI_SUCC) {
        l_nvram_params = g_nvram_init_params[NVRAM_CFG_TPC_RU_POWER_40M_5G];
    }
    l_ret += memcpy_s(pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g[WLAN_BW_CAP_40M], WLAN_HE_RU_SIZE_BUTT, \
                      &l_nvram_params, sizeof(l_nvram_params));

    /* 40m 484tone/80m 26~106tone ru_power */
    if (get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[NVRAM_CFG_TPC_RU_POWER_80M_L_5G].name, &l_nvram_params) != INI_SUCC) {
        l_nvram_params = g_nvram_init_params[NVRAM_CFG_TPC_RU_POWER_80M_L_5G];
    }
    pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g[WLAN_BW_CAP_40M][WLAN_HE_RU_SIZE_484] = cus_get_first_byte(l_nvram_params);
    pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g[WLAN_BW_CAP_80M][WLAN_HE_RU_SIZE_26] = cus_get_second_byte(l_nvram_params);
    pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g[WLAN_BW_CAP_80M][WLAN_HE_RU_SIZE_52] = cus_get_third_byte(l_nvram_params);
    pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g[WLAN_BW_CAP_80M][WLAN_HE_RU_SIZE_106] = cus_get_fourth_byte(l_nvram_params);

    /* 80m 242~996tone/160m 26tone ru_power */
    if (get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[NVRAM_CFG_TPC_RU_POWER_80M_H_5G].name, &l_nvram_params) != INI_SUCC) {
        l_nvram_params = g_nvram_init_params[NVRAM_CFG_TPC_RU_POWER_80M_H_5G];
    }
    pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g[WLAN_BW_CAP_80M][WLAN_HE_RU_SIZE_242] = cus_get_first_byte(l_nvram_params);
    pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g[WLAN_BW_CAP_80M][WLAN_HE_RU_SIZE_484] = cus_get_second_byte(l_nvram_params);
    pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g[WLAN_BW_CAP_80M][WLAN_HE_RU_SIZE_996] = cus_get_third_byte(l_nvram_params);
    pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g[WLAN_BW_CAP_160M][WLAN_HE_RU_SIZE_26] = cus_get_fourth_byte(l_nvram_params);

    /* 160m 52~996tone ru_power */
    if (get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[NVRAM_CFG_TPC_RU_POWER_160M_L_5G].name, &l_nvram_params) != INI_SUCC) {
        l_nvram_params = g_nvram_init_params[NVRAM_CFG_TPC_RU_POWER_160M_L_5G];
    }
    l_ret += memcpy_s(&(pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g[WLAN_BW_CAP_160M][WLAN_HE_RU_SIZE_52]),
                      (WLAN_HE_RU_SIZE_BUTT - WLAN_HE_RU_SIZE_52), &l_nvram_params, sizeof(l_nvram_params));
    if (get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[NVRAM_CFG_TPC_RU_POWER_160M_H_5G].name, &l_nvram_params) != INI_SUCC) {
        l_nvram_params = g_nvram_init_params[NVRAM_CFG_TPC_RU_POWER_160M_H_5G];
    }
    l_ret += memcpy_s(&(pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g[WLAN_BW_CAP_160M][WLAN_HE_RU_SIZE_996]),
                      (WLAN_HE_RU_SIZE_BUTT - WLAN_HE_RU_SIZE_996), &l_nvram_params, sizeof(uint8_t)*2); /* 2字节 */
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_get_delt_ru_pow_val_5g::memcpy failed!");
        memset_s(pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g, sizeof(int8_t) * WLAN_BW_CAP_BUTT * WLAN_HE_RU_SIZE_BUTT, 0, \
                 sizeof(int8_t)*WLAN_BW_CAP_BUTT * WLAN_HE_RU_SIZE_BUTT);
        return;
    }

    /* 参数有效性检查 */
    for (l_cfg_idx_one = 0; l_cfg_idx_one < WLAN_BW_CAP_BUTT; l_cfg_idx_one++) {
        for (l_cfg_idx_two = 0; l_cfg_idx_two < WLAN_HE_RU_SIZE_BUTT; l_cfg_idx_two++) {
            if (cus_val_invalid(pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g[l_cfg_idx_one][l_cfg_idx_two],
                                CUS_RU_DELT_POW_MAX, CUS_RU_DELT_POW_MIN)) {
                oam_error_log4(0, OAM_SF_CUSTOM, "hwifi_get_delt_ru_pow_val_5g:cfg bw[%d]ru[%d] val[%d] invalid! max[%d]",
                               l_cfg_idx_one, l_cfg_idx_two, pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g[l_cfg_idx_one][l_cfg_idx_two],
                               CUS_RU_DELT_POW_MAX);
                pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g[l_cfg_idx_one][l_cfg_idx_two] = 0;
            }
        }
    }

    return;
}


static void hwifi_get_delt_ru_pow_val_2g(void)
{
    int32_t l_nvram_params = 0;
    int32_t l_ret;
    int32_t l_cfg_idx_one;
    int32_t l_cfg_idx_two;
    wlan_cust_nvram_params *pst_g_cust_nv_params = hwifi_get_nvram_params();

    /* 20m 26~242tone ru_power */
    if (get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[NVRAM_CFG_TPC_RU_POWER_20M_2G].name, &l_nvram_params) != INI_SUCC) {
        l_nvram_params = g_nvram_init_params[NVRAM_CFG_TPC_RU_POWER_20M_2G];
    }
    l_ret = memcpy_s(pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_2g[WLAN_BW_CAP_20M], WLAN_HE_RU_SIZE_996,
                     &l_nvram_params, sizeof(l_nvram_params));

    /* 40m 26~242tone ru_power */
    if (get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[NVRAM_CFG_TPC_RU_POWER_40M_0_2G].name, &l_nvram_params) != INI_SUCC) {
        l_nvram_params = g_nvram_init_params[NVRAM_CFG_TPC_RU_POWER_40M_0_2G];
    }
    l_ret += memcpy_s(pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_2g[WLAN_BW_CAP_40M], WLAN_HE_RU_SIZE_996,
                      &l_nvram_params, sizeof(l_nvram_params));

    /* 40m 484tone ru_power */
    if (get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[NVRAM_CFG_TPC_RU_POWER_40M_1_2G].name, &l_nvram_params) != INI_SUCC) {
        l_nvram_params = g_nvram_init_params[NVRAM_CFG_TPC_RU_POWER_40M_1_2G];
    }
    pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_2g[WLAN_BW_CAP_40M][WLAN_HE_RU_SIZE_484] = cus_get_first_byte(l_nvram_params);

    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_get_delt_ru_pow_val_2g::memcpy failed!");
        memset_s(pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_5g, sizeof(int8_t) * WLAN_BW_CAP_80M * WLAN_HE_RU_SIZE_996, \
                 0, sizeof(int8_t) * WLAN_BW_CAP_80M * WLAN_HE_RU_SIZE_996);
        return;
    }

    /* 参数有效性检查 */
    for (l_cfg_idx_one = 0; l_cfg_idx_one < WLAN_BW_CAP_80M; l_cfg_idx_one++) {
        for (l_cfg_idx_two = 0; l_cfg_idx_two < WLAN_HE_RU_SIZE_996; l_cfg_idx_two++) {
            if (cus_val_invalid(pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_2g[l_cfg_idx_one][l_cfg_idx_two],
                                CUS_RU_DELT_POW_MAX, CUS_RU_DELT_POW_MIN)) {
                oam_error_log4(0, OAM_SF_CUSTOM, "hwifi_get_delt_ru_pow_val_2g:cfg bw[%d]ru[%d] val[%d] invalid! max[%d]",
                               l_cfg_idx_one, l_cfg_idx_two, pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_2g[l_cfg_idx_one][l_cfg_idx_two],
                               CUS_RU_DELT_POW_MAX);
                pst_g_cust_nv_params->ac_fullbandwidth_to_ru_power_2g[l_cfg_idx_one][l_cfg_idx_two] = 0;
            }
        }
    }

    return;
}

#if defined(_PRE_WLAN_FEATURE_11AX)


static int32_t hwifi_get_5g_ru_max_ru_pow_val(wlan_cust_nvram_params *g_cust_nv_params)
{
    int32_t nvram_params = 0;
    int32_t ret = 0;
    uint32_t idx;
    uint32_t band_idx;
    uint32_t nvram_idx;
    regdomain_enum_uint8 regdomain_type;

    /* 根据管制域信息选择下发FCC还是CE参数 */
    regdomain_type = hwifi_get_regdomain_from_country_code((uint8_t *)g_wifi_country_code);
    nvram_idx = (regdomain_type == REGDOMAIN_ETSI) ? NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B1_CE : \
                (regdomain_type == REGDOMAIN_FCC) ? NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B1_FCC : \
                NVRAM_CFG_TPC_RU_MAX_POWER_0_5G_MIMO_B1;

    for (band_idx = 0; band_idx < MAC_NUM_5G_BAND; band_idx++) {
        for (idx = CUS_POW_TX_CHAIN_MIMO; idx < CUS_POW_TX_CHAIN_BUTT; idx++) {
            if (get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[nvram_idx].name, &nvram_params) != INI_SUCC) {
                nvram_params = 0xFFFFFFFF;
            }
            ret += memcpy_s(g_cust_nv_params->auc_tpc_tb_ru_5g_max_power[band_idx].auc_tb_ru_5g_max_power[idx],
                WLAN_HE_RU_SIZE_BUTT, &nvram_params, sizeof(nvram_params));
            nvram_idx++;
            if (get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[nvram_idx].name, &nvram_params) != INI_SUCC) {
                nvram_params = 0xFFFFFF;
            }
            ret += memcpy_s(&g_cust_nv_params->auc_tpc_tb_ru_5g_max_power[band_idx].auc_tb_ru_5g_max_power\
                // 1992_996_484,3个ru的定制化项
                [idx][WLAN_HE_RU_SIZE_484], (WLAN_HE_RU_SIZE_BUTT - WLAN_HE_RU_SIZE_484),
                &nvram_params, 3 * sizeof(uint8_t));
            nvram_idx++;
        }
    }

    return ret;
}


static void hwifi_get_total_max_ru_pow_val(wlan_cust_nvram_params *pst_g_cust_nv_params)
{
    uint32_t idx;
    uint32_t nvram_idx;
    int32_t l_nvram_params = 0;

    nvram_idx = NVRAM_CFG_TPC_RU_MAX_POWER_2G;
    for (idx = WLAN_BAND_2G; idx < WLAN_BAND_BUTT; idx++) {
        if (get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[nvram_idx].name, &l_nvram_params) != INI_SUCC) {
            l_nvram_params = 0xFF;
        }
        pst_g_cust_nv_params->auc_tpc_tb_ru_max_power[idx] = cus_get_first_byte(l_nvram_params);
        nvram_idx++;
    }
}


static void hwifi_get_max_ru_pow_val(void)
{
    int32_t l_nvram_params = 0;
    int32_t l_ret = 0;
    wlan_cust_nvram_params *cust_nvram_params = hwifi_get_nvram_params();
    uint32_t idx;
    uint32_t nvram_idx = NVRAM_CFG_TPC_RU_MAX_POWER_0_2G_MIMO;

    for (idx = CUS_POW_TX_CHAIN_MIMO; idx < CUS_POW_TX_CHAIN_BUTT; idx++) {
        if (get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[nvram_idx].name, &l_nvram_params) != INI_SUCC) {
            l_nvram_params = 0xFFFFFFFF;
        }
        l_ret += memcpy_s(cust_nvram_params->auc_tpc_tb_ru_2g_max_power[idx], WLAN_HE_RU_SIZE_996,
                          &l_nvram_params, sizeof(l_nvram_params));
        nvram_idx++;
        if (get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[nvram_idx].name, &l_nvram_params) != INI_SUCC) {
            l_nvram_params = 0xFF;
        }
        cust_nvram_params->auc_tpc_tb_ru_2g_max_power[idx][WLAN_HE_RU_SIZE_484] = cus_get_first_byte(l_nvram_params);
        nvram_idx++;
    }
    hwifi_get_5g_ru_max_ru_pow_val(cust_nvram_params);
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_get_max_ru_pow_val::memcpy failed!");
        memset_s(cust_nvram_params->auc_tpc_tb_ru_2g_max_power, sizeof(cust_nvram_params->auc_tpc_tb_ru_2g_max_power),
                 0xFF, sizeof(cust_nvram_params->auc_tpc_tb_ru_2g_max_power));
        memset_s(cust_nvram_params->auc_tpc_tb_ru_5g_max_power, sizeof(cust_nvram_params->auc_tpc_tb_ru_5g_max_power),
                 0xFF, sizeof(cust_nvram_params->auc_tpc_tb_ru_5g_max_power));
    }

    hwifi_get_total_max_ru_pow_val(cust_nvram_params);
}
#endif


OAL_STATIC OAL_INLINE void hwifi_config_init_ru_pow_nvram(void)
{
    hwifi_get_delt_ru_pow_val_5g();
    hwifi_get_delt_ru_pow_val_2g();

#if defined(_PRE_WLAN_FEATURE_11AX)
    hwifi_get_max_ru_pow_val();
#endif
}

/*
 * 函 数 名  : hwifi_config_init_nvram
 * 功能描述  : handle nvram customize params
 */
OAL_STATIC int32_t hwifi_config_init_nvram(void)
{
    int32_t l_ret = INI_FAILED;
    int32_t l_cfg_id;
    int32_t al_nvram_params[NVRAM_PARAMS_TXPWR_INDEX_BUTT] = {0};
    int32_t l_val;
    wlan_cust_nvram_params *pst_g_cust_nv_params = hwifi_get_nvram_params();

    memset_s(&g_cust_nv_params, sizeof(g_cust_nv_params), 0, sizeof(g_cust_nv_params));

    /* read nvm failed or data not exist or country_code updated, read ini:cust_spec > cust_common > default */
    /* find plat tag */
    for (l_cfg_id = NVRAM_PARAMS_INDEX_0; l_cfg_id < NVRAM_PARAMS_TXPWR_INDEX_BUTT; l_cfg_id++) {
        l_ret = get_cust_conf_int32(INI_MODU_WIFI, g_nvram_config_ini[l_cfg_id].name, &al_nvram_params[l_cfg_id]);
        oam_info_log2(0, OAM_SF_CFG, "{hwifi_config_init_nvram aul_nvram_params[%d]=0x%x!}",
                      l_cfg_id, al_nvram_params[l_cfg_id]);

        if (l_ret != INI_SUCC) {
            oam_warning_log1(0, OAM_SF_CFG, "hwifi_config_init_nvram read id[%d] from ini failed!", l_cfg_id);

            /* 读取失败时,使用初始值 */
            al_nvram_params[l_cfg_id] = g_nvram_init_params[l_cfg_id];
        }
    }

    l_ret = memcpy_s(pst_g_cust_nv_params->ac_delt_txpwr_params,
                     NUM_OF_NV_MAX_TXPOWER,
                     al_nvram_params,
                     NUM_OF_NV_MAX_TXPOWER);
    l_ret += memcpy_s(pst_g_cust_nv_params->ac_dpd_delt_txpwr_params, NUM_OF_NV_DPD_MAX_TXPOWER,
        al_nvram_params + NVRAM_PARAMS_INDEX_DPD_0, NUM_OF_NV_DPD_MAX_TXPOWER);

    l_val = cus_get_low_16bits(*(al_nvram_params + NVRAM_PARAMS_INDEX_11B_OFDM_DELT_POW));
    l_ret += memcpy_s(pst_g_cust_nv_params->ac_11b_delt_txpwr_params, NUM_OF_NV_11B_DELTA_TXPOWER,
                      &l_val, NUM_OF_NV_11B_DELTA_TXPOWER);
    /* FEM OFF IQ CALI POW */
    l_val = cus_get_high_16bits(*(al_nvram_params + NVRAM_PARAMS_INDEX_11B_OFDM_DELT_POW));
    l_ret += memcpy_s(pst_g_cust_nv_params->auc_fem_off_iq_cal_pow_params,
                      sizeof(pst_g_cust_nv_params->auc_fem_off_iq_cal_pow_params),
                      &l_val, sizeof(pst_g_cust_nv_params->auc_fem_off_iq_cal_pow_params));

    l_ret += memcpy_s(pst_g_cust_nv_params->auc_5g_upper_upc_params, NUM_OF_NV_5G_UPPER_UPC,
                      al_nvram_params + NVRAM_PARAMS_INDEX_IQ_MAX_UPC,
                      NUM_OF_NV_5G_UPPER_UPC);

    l_ret += memcpy_s(pst_g_cust_nv_params->ac_2g_low_pow_amend_params, NUM_OF_NV_2G_LOW_POW_DELTA_VAL,
                      al_nvram_params + NVRAM_PARAMS_INDEX_2G_LOW_POW_AMEND,
                      NUM_OF_NV_2G_LOW_POW_DELTA_VAL);
    if (l_ret != EOK) {
        oam_error_log1(0, OAM_SF_CFG, "hwifi_config_init_nvram read from ini failed[%d]!", l_ret);
        return INI_FAILED;
    }

    for (l_cfg_id = 0; l_cfg_id < NUM_OF_NV_2G_LOW_POW_DELTA_VAL; l_cfg_id++) {
        if (cus_abs(pst_g_cust_nv_params->ac_2g_low_pow_amend_params[l_cfg_id]) > CUS_2G_LOW_POW_AMEND_ABS_VAL_MAX) {
            pst_g_cust_nv_params->ac_2g_low_pow_amend_params[l_cfg_id] = 0;
        }
    }

    /* 基准功率 */
    hwifi_get_max_txpwr_base(INI_MODU_WIFI, NVRAM_PARAMS_INDEX_19,
                             pst_g_cust_nv_params->auc_2g_txpwr_base_params[WLAN_RF_CHANNEL_ZERO], CUS_BASE_PWR_NUM_2G);
    hwifi_get_max_txpwr_base(INI_MODU_WIFI, NVRAM_PARAMS_INDEX_20,
                             pst_g_cust_nv_params->auc_5g_txpwr_base_params[WLAN_RF_CHANNEL_ZERO], CUS_BASE_PWR_NUM_5G);
    hwifi_get_max_txpwr_base(INI_MODU_WIFI, NVRAM_PARAMS_INDEX_21,
                             pst_g_cust_nv_params->auc_2g_txpwr_base_params[WLAN_RF_CHANNEL_ONE], CUS_BASE_PWR_NUM_2G);
    hwifi_get_max_txpwr_base(INI_MODU_WIFI, NVRAM_PARAMS_INDEX_22,
                             pst_g_cust_nv_params->auc_5g_txpwr_base_params[WLAN_RF_CHANNEL_ONE], CUS_BASE_PWR_NUM_5G);
    /* FCC/CE/SAR */
    for (l_cfg_id = 0; l_cfg_id < WLAN_RF_CHANNEL_NUMS; l_cfg_id++) {
        hwifi_config_init_fcc_ce_txpwr_nvram(l_cfg_id);
        hwifi_config_init_sar_ctrl_nvram(l_cfg_id);
    }
    hwifi_get_max_txpwr_base_delt_val(NVRAM_PARAMS_INDEX_DELT_BASE_POWER_23,
                                      &pst_g_cust_nv_params->ac_delt_txpwr_base_params[0][0],
                                      sizeof(pst_g_cust_nv_params->ac_delt_txpwr_base_params));

    /* extend */
    hwifi_config_init_iq_lpf_nvram_param();

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    /* TAS */
    hwifi_config_init_tas_ctrl_nvram();
#endif
    hwifi_config_init_ru_pow_nvram();

    oam_info_log0(0, OAM_SF_CFG, "hwifi_config_init_nvram read from ini success!");
    return INI_SUCC;
}

/*
 * 函 数 名  : hwifi_config_init_private_custom
 * 功能描述  : 初始化私有定制全局变量数组
 */
OAL_STATIC int32_t hwifi_config_init_private_custom(void)
{
    int32_t l_cfg_id;
    int32_t l_ret = INI_FAILED;

    for (l_cfg_id = 0; l_cfg_id < WLAN_CFG_PRIV_BUTT; l_cfg_id++) {
        /* 获取 private 的配置值 */
        l_ret = get_cust_conf_int32(INI_MODU_WIFI, g_wifi_config_priv[l_cfg_id].name,
                                    &(g_priv_cust_params[l_cfg_id].l_val));
        if (l_ret == INI_FAILED) {
            g_priv_cust_params[l_cfg_id].en_value_state = OAL_FALSE;
            continue;
        }
        g_priv_cust_params[l_cfg_id].en_value_state = OAL_TRUE;
    }

    oam_warning_log0(0, OAM_SF_CFG, "hwifi_config_init_private_custom read from ini success!");

    return INI_SUCC;
}

/*
 * 函 数 名  : hwifi_config_get_switch_point_5g
 * 功能描述  : 根据ini文件获取5G二次曲线功率切换点
 */
OAL_STATIC void hwifi_config_get_5g_curv_switch_point(uint8_t *puc_ini_pa_params, uint32_t cfg_id)
{
    int32_t l_ini_params[CUS_NUM_5G_BW * DY_CALI_PARAMS_TIMES] = {0};
    uint16_t us_ini_param_num = 0;
    uint8_t uc_secon_ratio_idx = 0;
    uint8_t uc_param_idx;
    uint8_t uc_chain_idx;
    int16_t *ps_extre_point_val = NULL;

    if ((cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0) || (cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1)) {
        uc_chain_idx = WLAN_RF_CHANNEL_ZERO;
    } else if ((cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1) || (cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_BAND1)) {
        uc_chain_idx = WLAN_RF_CHANNEL_ONE;
    } else {
        return;
    }

    /* 获取拟合系数项 */
    if (hwifi_config_sepa_coefficient_from_param(puc_ini_pa_params, l_ini_params, &us_ini_param_num,
                                                 sizeof(l_ini_params) / sizeof(int32_t)) != OAL_SUCC ||
        (us_ini_param_num % DY_CALI_PARAMS_TIMES)) {
        oam_error_log2(0, OAM_SF_CUSTOM,
                       "hwifi_config_get_5g_curv_switch_point::ini is unsuitable,num of ini[%d] cfg_id[%d]!",
                       us_ini_param_num, cfg_id);
        return;
    }

    ps_extre_point_val = g_gs_extre_point_vals[uc_chain_idx];
    us_ini_param_num /= DY_CALI_PARAMS_TIMES;
    if (cfg_id <= WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1) {
        if (us_ini_param_num != CUS_NUM_5G_BW) {
            oam_error_log2(0, OAM_SF_CUSTOM,
                "hwifi_config_get_5g_curv_switch_point::ul_cfg_id[%d] us_ini_param_num[%d]", cfg_id, us_ini_param_num);
            return;
        }
        ps_extre_point_val++;
    } else {
        if (us_ini_param_num != 1) {
            oam_error_log2(0, OAM_SF_CUSTOM,
                "hwifi_config_get_5g_curv_switch_point::ul_cfg_id[%d] us_ini_param_num[%d]", cfg_id, us_ini_param_num);
            return;
        }
    }

    /* 计算5g曲线switch point */
    for (uc_param_idx = 0; uc_param_idx < us_ini_param_num; uc_param_idx++) {
        *(ps_extre_point_val + uc_param_idx) = (int16_t)hwifi_dyn_cali_get_extre_point(l_ini_params +
                                               uc_secon_ratio_idx);
        oal_io_print("hwifi_config_get_5g_curv_switch_point::extre power[%d] param_idx[%d] cfg_id[%d]!\r\n",
                     *(ps_extre_point_val + uc_param_idx), uc_param_idx, cfg_id);
        oal_io_print("hwifi_config_get_5g_curv_switch_point::param[%d %d] uc_secon_ratio_idx[%d]!\r\n",
            (l_ini_params + uc_secon_ratio_idx)[0], (l_ini_params + uc_secon_ratio_idx)[1], uc_secon_ratio_idx);
        uc_secon_ratio_idx += DY_CALI_PARAMS_TIMES;
    }

    return;
}

/*
 * 函 数 名  : hwifi_config_nvram_second_coefficient_check
 * 功能描述  : 检查修正nvram中的二次系数是否合理
 */
OAL_STATIC uint32_t hwifi_config_nvram_second_coefficient_check(uint8_t *puc_g_cust_nvram_info,
                                                                uint8_t *puc_ini_pa_params,
                                                                uint32_t cfg_id,
                                                                int16_t *ps_5g_delt_power)
{
    int32_t l_ini_params[CUS_NUM_5G_BW * DY_CALI_PARAMS_TIMES] = {0};
    int32_t l_nv_params[CUS_NUM_5G_BW * DY_CALI_PARAMS_TIMES] = {0};
    uint16_t us_ini_param_num = 0;
    uint16_t us_nv_param_num = 0;
    uint8_t uc_secon_ratio_idx = 0;
    uint8_t uc_param_idx;

    /* 获取拟合系数项 */
    if (hwifi_config_sepa_coefficient_from_param(puc_g_cust_nvram_info, l_nv_params, &us_nv_param_num,
                                                 sizeof(l_nv_params) / sizeof(int16_t)) != OAL_SUCC ||
        (us_nv_param_num % DY_CALI_PARAMS_TIMES) ||
        hwifi_config_sepa_coefficient_from_param(puc_ini_pa_params, l_ini_params, &us_ini_param_num,
                                                 sizeof(l_ini_params) / sizeof(int16_t)) != OAL_SUCC ||
        (us_ini_param_num % DY_CALI_PARAMS_TIMES) || (us_nv_param_num != us_ini_param_num)) {
        oam_error_log2(0, OAM_SF_CUSTOM,
            "hwifi_config_nvram_second_coefficient_check::nvram or ini is unsuitable,num of nv and ini[%d %d]!",
            us_nv_param_num, us_ini_param_num);
        return OAL_FAIL;
    }

    us_nv_param_num /= DY_CALI_PARAMS_TIMES;
    /* 检查nv和ini中二次系数是否匹配 */
    for (uc_param_idx = 0; uc_param_idx < us_nv_param_num; uc_param_idx++) {
        if (l_ini_params[uc_secon_ratio_idx] != l_nv_params[uc_secon_ratio_idx]) {
            oam_warning_log4(0, OAM_SF_CUSTOM, "hwifi_config_nvram_second_coefficient_check::nvram get mismatch value \
                idx[%d %d] val are [%d] and [%d]!", uc_param_idx, uc_secon_ratio_idx, l_ini_params[uc_secon_ratio_idx],
                l_nv_params[uc_secon_ratio_idx]);

            /* 量产后二次系数以nvram中为准，刷新NV中的二次拟合曲线切换点 */
            hwifi_config_get_5g_curv_switch_point(puc_g_cust_nvram_info, cfg_id);
            uc_secon_ratio_idx += DY_CALI_PARAMS_TIMES;
            continue;
        }

        if ((cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0) || (cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1)) {
            /* 计算产线上的delt power */
            *(ps_5g_delt_power + uc_param_idx) = hwifi_get_5g_pro_line_delt_pow_per_band(
                l_nv_params + uc_secon_ratio_idx, l_ini_params + uc_secon_ratio_idx);
            oal_io_print("hwifi_config_nvram_second_coefficient_check::delt power[%d] param_idx[%d] cfg_id[%d]!\r\n",
                         *(ps_5g_delt_power + uc_param_idx), uc_param_idx, cfg_id);
        }
        uc_secon_ratio_idx += DY_CALI_PARAMS_TIMES;
    }

    return OAL_SUCC;
}

static int32_t hwifi_config_get_cust_string(uint32_t cfg_id, uint8_t *nv_pa_params)
{
    uint32_t cfg_id_tmp;
    if (get_cust_conf_string(INI_MODU_WIFI, g_nvram_pro_line_config_ini[cfg_id].name,
                             nv_pa_params, CUS_PARAMS_LEN_MAX - 1) == INI_FAILED) {
        if (oal_value_eq_any4(cfg_id, WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW,
                              WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_LOW,
                              WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1_LOW,
                              WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_BAND1_LOW)) {
            cfg_id_tmp = ((cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1_LOW) ?
                              WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1 :
                              (cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW) ?
                              WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0 :
                              (cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_BAND1_LOW) ?
                              WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_BAND1 :
                              (cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_LOW) ?
                              WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1 : cfg_id);
            get_cust_conf_string(INI_MODU_WIFI, g_nvram_pro_line_config_ini[cfg_id_tmp].name,
                                 nv_pa_params, CUS_PARAMS_LEN_MAX - 1);
        } else {
            oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_config_get_cust_string read, check id[%d] exists!", cfg_id);
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

static uint8_t hwifi_config_set_cali_param_2g_and_5g(int32_t *pl_params, int16_t *s_5g_delt_power)
{
    uint8_t rf_idx;
    uint8_t cali_param_idx;
    uint8_t delt_pwr_idx = 0;
    uint8_t idx = 0;
    for (rf_idx = 0; rf_idx < WLAN_RF_CHANNEL_NUMS; rf_idx++) {
        for (cali_param_idx = 0; cali_param_idx < DY_CALI_PARAMS_BASE_NUM; cali_param_idx++) {
            if (cali_param_idx == (DY_2G_CALI_PARAMS_NUM - 1)) {
                /* band1 & CW */
                cali_param_idx += PRO_LINE_2G_TO_5G_OFFSET;
            }
            g_pro_line_params[rf_idx][cali_param_idx].l_pow_par2 = pl_params[idx++];
            g_pro_line_params[rf_idx][cali_param_idx].l_pow_par1 = pl_params[idx++];
            g_pro_line_params[rf_idx][cali_param_idx].l_pow_par0 = pl_params[idx++];
        }
    }

    /* 5g band2&3 4&5 6 7 low power */
    for (rf_idx = 0; rf_idx < WLAN_RF_CHANNEL_NUMS; rf_idx++) {
        delt_pwr_idx = 0;
        for (cali_param_idx = DY_CALI_PARAMS_BASE_NUM + 1;
             cali_param_idx < DY_CALI_PARAMS_NUM - 1; cali_param_idx++) {
            g_pro_line_params[rf_idx][cali_param_idx].l_pow_par2 = pl_params[idx++];
            g_pro_line_params[rf_idx][cali_param_idx].l_pow_par1 = pl_params[idx++];
            g_pro_line_params[rf_idx][cali_param_idx].l_pow_par0 = pl_params[idx++];

            cus_flush_nv_ratio_by_delt_pow(g_pro_line_params[rf_idx][cali_param_idx].l_pow_par2,
                                           g_pro_line_params[rf_idx][cali_param_idx].l_pow_par1,
                                           g_pro_line_params[rf_idx][cali_param_idx].l_pow_par0,
                                           *(s_5g_delt_power + rf_idx * CUS_NUM_5G_BW + delt_pwr_idx));
            delt_pwr_idx++;
        }
    }
    return idx;
}

static void hwifi_config_set_cali_param_left_num(int32_t *pl_params, uint8_t idx)
{
    uint8_t rf_idx;
    /* band1 & CW */
    for (rf_idx = 0; rf_idx < WLAN_RF_CHANNEL_NUMS; rf_idx++) {
        g_pro_line_params[rf_idx][DY_2G_CALI_PARAMS_NUM].l_pow_par2 = pl_params[idx++];
        g_pro_line_params[rf_idx][DY_2G_CALI_PARAMS_NUM].l_pow_par1 = pl_params[idx++];
        g_pro_line_params[rf_idx][DY_2G_CALI_PARAMS_NUM].l_pow_par0 = pl_params[idx++];
    }
    for (rf_idx = 0; rf_idx < WLAN_RF_CHANNEL_NUMS; rf_idx++) {
        g_pro_line_params[rf_idx][DY_2G_CALI_PARAMS_NUM - 1].l_pow_par2 = pl_params[idx++];
        g_pro_line_params[rf_idx][DY_2G_CALI_PARAMS_NUM - 1].l_pow_par1 = pl_params[idx++];
        g_pro_line_params[rf_idx][DY_2G_CALI_PARAMS_NUM - 1].l_pow_par0 = pl_params[idx++];
    }
    for (rf_idx = 0; rf_idx < WLAN_RF_CHANNEL_NUMS; rf_idx++) {
        /* 5g band1 low power */
        /* band1产线不校准 */
        g_pro_line_params[rf_idx][DY_CALI_PARAMS_BASE_NUM].l_pow_par2 = pl_params[idx++];
        g_pro_line_params[rf_idx][DY_CALI_PARAMS_BASE_NUM].l_pow_par1 = pl_params[idx++];
        g_pro_line_params[rf_idx][DY_CALI_PARAMS_BASE_NUM].l_pow_par0 = pl_params[idx++];
    }

    for (rf_idx = 0; rf_idx < WLAN_RF_CHANNEL_NUMS; rf_idx++) {
        /* 2g cw ppa */
        g_pro_line_params[rf_idx][DY_CALI_PARAMS_NUM - 1].l_pow_par2 = pl_params[idx++];
        g_pro_line_params[rf_idx][DY_CALI_PARAMS_NUM - 1].l_pow_par1 = pl_params[idx++];
        g_pro_line_params[rf_idx][DY_CALI_PARAMS_NUM - 1].l_pow_par0 = pl_params[idx++];
    }
}

static uint32_t hwifi_nvram_param_check_second_coefficient(uint8_t *nv_pa_params, uint8_t *cust_nvram_info,
                                                           int16_t *delt_power_5g, uint32_t cfg_id)
{
    /* 先取nv中的参数值,为空则从ini文件中读取 */
    if (OAL_STRLEN(cust_nvram_info)) {
        /* NVRAM二次系数异常保护 */
        if (hwifi_config_nvram_second_coefficient_check(cust_nvram_info, nv_pa_params, cfg_id, delt_power_5g +
            (cfg_id < WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA1 ? WLAN_RF_CHANNEL_ZERO : WLAN_RF_CHANNEL_ONE)) == OAL_SUCC) {
            /* 手机如果low part为空,则取ini中的系数,并根据产测结果修正;否则直接从nvram中取得 */
            if ((cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW) &&
                (oal_memcmp(cust_nvram_info, nv_pa_params, OAL_STRLEN(cust_nvram_info)))) {
                memset_s(delt_power_5g + WLAN_RF_CHANNEL_ZERO, CUS_NUM_5G_BW * sizeof(int16_t),
                         0, CUS_NUM_5G_BW * sizeof(int16_t));
            }
            if ((cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_LOW) &&
                (oal_memcmp(cust_nvram_info, nv_pa_params, OAL_STRLEN(cust_nvram_info)))) {
                memset_s(delt_power_5g + WLAN_RF_CHANNEL_ONE, CUS_NUM_5G_BW * sizeof(int16_t),
                         0, CUS_NUM_5G_BW * sizeof(int16_t));
            }

            if (memcpy_s(nv_pa_params, CUS_PARAMS_LEN_MAX * sizeof(uint8_t),
                         cust_nvram_info, OAL_STRLEN(cust_nvram_info)) != EOK) {
                return OAL_FAIL;
            }
        } else {
            return OAL_FAIL;
        }
    } else {
        /* 提供产线第一次上电校准初始值 */
        if (memcpy_s(cust_nvram_info, CUS_PARAMS_LEN_MAX,
                     nv_pa_params, OAL_STRLEN(nv_pa_params)) != EOK) {
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

static uint32_t hwifi_coefficient_check_and_set(uint8_t **cust_nvram_info, uint8_t *nv_pa_params,
                                                int32_t *params, int16_t *delt_power_5g,
                                                uint16_t *param_num)
{
    uint32_t cfg_id;
    uint16_t per_param_num = 0;
    uint16_t param_len = WLAN_RF_CHANNEL_NUMS * DY_CALI_PARAMS_TIMES * DY_CALI_PARAMS_NUM * sizeof(int32_t);
    for (cfg_id = WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA0; cfg_id < WLAN_CFG_DTS_NVRAM_PARAMS_BUTT; cfg_id++) {
        /* 二次拟合系数 */
        if ((cfg_id >= WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C0) && (cfg_id < WLAN_CFG_DTS_NVRAM_END)) {
            /* DPN */
            continue;
        }

        if (hwifi_config_get_cust_string(cfg_id, nv_pa_params) != OAL_SUCC) {
            return OAL_FAIL;
        }

        /* 获取ini中的二次拟合曲线切换点 */
        hwifi_config_get_5g_curv_switch_point(nv_pa_params, cfg_id);

        if (cfg_id <= WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_LOW) {
            *cust_nvram_info = hwifi_get_nvram_param(cfg_id);
            if (hwifi_nvram_param_check_second_coefficient(nv_pa_params, *cust_nvram_info,
                                                           delt_power_5g, cfg_id) != OAL_SUCC) {
                return OAL_FAIL;
            }
        }

        if (hwifi_config_sepa_coefficient_from_param(nv_pa_params, params + *param_num,
                                                     &per_param_num, param_len - *param_num) != OAL_SUCC ||
                                                     (per_param_num % DY_CALI_PARAMS_TIMES)) {
            oam_error_log3(0, OAM_SF_CUSTOM,
                "hwifi_coefficient_check_and_set read get wrong value,len[%d] check id[%d] exists per_param_num[%d]!",
                OAL_STRLEN(*cust_nvram_info), cfg_id, per_param_num);
            return OAL_FAIL;
        }
        *param_num += per_param_num;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : hwifi_config_init_dy_cali_custom
 * 功能描述  : 获取定制化文件和二次产测系数
 */
OAL_STATIC uint32_t hwifi_config_init_dy_cali_custom(void)
{
    uint32_t ret;
    uint8_t uc_idx = 0;
    uint16_t us_param_num = 0;
    int16_t s_5g_delt_power[WLAN_RF_CHANNEL_NUMS][CUS_NUM_5G_BW] = {{0}};
    uint8_t *puc_g_cust_nvram_info = NULL;
    uint8_t *puc_nv_pa_params = NULL;
    int32_t *pl_params = NULL;
    uint16_t us_param_len = WLAN_RF_CHANNEL_NUMS * DY_CALI_PARAMS_TIMES *
                              DY_CALI_PARAMS_NUM * sizeof(int32_t);

    puc_nv_pa_params = (uint8_t *)os_kzalloc_gfp(CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
    if (puc_nv_pa_params == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_config_init_dy_cali_custom::puc_nv_pa_params mem alloc fail!");
        return OAL_FAIL;
    }

    pl_params = (int32_t *)os_kzalloc_gfp(us_param_len);
    if (pl_params == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_config_init_dy_cali_custom::ps_params mem alloc fail!");
        os_mem_kfree(puc_nv_pa_params);
        return OAL_FAIL;
    }

    memset_s(puc_nv_pa_params,
             CUS_PARAMS_LEN_MAX * sizeof(uint8_t),
             0,
             CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
    memset_s(pl_params, us_param_len, 0, us_param_len);

    ret = hwifi_coefficient_check_and_set(&puc_g_cust_nvram_info, puc_nv_pa_params, pl_params,
                                          (int16_t *)s_5g_delt_power, &us_param_num);

    os_mem_kfree(puc_nv_pa_params);

    if (ret == OAL_FAIL) {
        /* 置零防止下发到device */
        memset_s(g_pro_line_params, sizeof(g_pro_line_params), 0, sizeof(g_pro_line_params));
    } else {
        if (us_param_num != us_param_len / sizeof(int32_t)) {
            oam_error_log1(0, OAM_SF_CUSTOM,
                           "hwifi_config_init_dy_cali_custom read get wrong ini value num[%d]!", us_param_num);
            memset_s(g_pro_line_params, sizeof(g_pro_line_params), 0, sizeof(g_pro_line_params));
            os_mem_kfree(pl_params);
            return OAL_FAIL;
        }

        uc_idx = hwifi_config_set_cali_param_2g_and_5g(pl_params, (int16_t *)s_5g_delt_power);

        hwifi_config_set_cali_param_left_num(pl_params, uc_idx);
    }

    os_mem_kfree(pl_params);
    return ret;
}

/*
 * 函 数 名  : hwifi_config_init
 * 功能描述  : netdev open 调用的定制化总入口
 *             读取ini文件，更新 g_host_init_params 全局数组
 */
int32_t hwifi_config_init(int32_t cus_tag)
{
    int32_t l_cfg_id;
    int32_t l_ret = INI_FAILED;
    int32_t l_ori_val;
    wlan_cfg_cmd *pgast_wifi_config = NULL;
    int32_t *pgal_params = NULL;
    int32_t l_cfg_value = 0;
    int32_t l_wlan_cfg_butt;

    switch (cus_tag) {
        case CUS_TAG_NV:
            original_value_for_nvram_params();
            return hwifi_config_init_nvram();
        case CUS_TAG_INI:
            host_params_init_first();
            pgast_wifi_config = g_wifi_config_cmds;
            pgal_params = g_host_init_params;
            l_wlan_cfg_butt = WLAN_CFG_INIT_BUTT;
            break;
        case CUS_TAG_DTS:
            original_value_for_dts_params();
            pgast_wifi_config = g_wifi_config_dts;
            pgal_params = g_dts_params;
            l_wlan_cfg_butt = WLAN_CFG_DTS_BUTT;
            break;
        case CUS_TAG_PRIV_INI:
            return hwifi_config_init_private_custom();
        case CUS_TAG_PRO_LINE_INI:
            return hwifi_config_init_dy_cali_custom();
        default:
            oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_config_init tag number[0x%x] not correct!", cus_tag);
            return INI_FAILED;
    }

    for (l_cfg_id = 0; l_cfg_id < l_wlan_cfg_butt; l_cfg_id++) {
        /* 获取ini的配置值 */
        l_ret = get_cust_conf_int32(INI_MODU_WIFI, pgast_wifi_config[l_cfg_id].name, &l_cfg_value);
        if (l_ret == INI_FAILED) {
            oam_info_log2(0, OAM_SF_CUSTOM, "hwifi_config_init read ini file cfg_id[%d]tag[%d] not exist!",
                          l_cfg_id, cus_tag);
            continue;
        }
        l_ori_val = pgal_params[pgast_wifi_config[l_cfg_id].case_entry];
        pgal_params[pgast_wifi_config[l_cfg_id].case_entry] = l_cfg_value;
    }

    return INI_SUCC;
}

#ifndef _PRE_PRODUCT_HI3751V811
/*
 * 函 数 名  : char2byte
 * 功能描述  : 统计值，判断有无读取到mac地址
 */
OAL_STATIC uint32_t char2byte(const char *strori, char *outbuf)
{
    int i = 0;
    uint8_t temp = 0;
    uint32_t sum = 0;
    uint8_t *ptr_out = (uint8_t *)outbuf;
    const int l_loop_times = 12; /* 单字节遍历是不是正确的mac地址:xx:xx:xx:xx:xx:xx */

    for (i = 0; i < l_loop_times; i++) {
        if (isdigit(strori[i])) {
            temp = strori[i] - '0';
        } else if (islower(strori[i])) {
            temp = (strori[i] - 'a') + 10; /* 加10为了保证'a'~'f'分别对应10~15 */
        } else if (isupper(strori[i])) {
            temp = (strori[i] - 'A') + 10; /* 加10为了保证'A'~'F'分别对应10~15 */
        }
        sum += temp;
        /* 为了组成正确的mac地址:xx:xx:xx:xx:xx:xx */
        if (i % 2 == 0) { /* 对2取余 */
            ptr_out[i / 2] |= (temp << BIT_OFFSET_4); /* 除以2 */
        } else {
            ptr_out[i / 2] |= temp; /* 除以2 */
        }
    }

    return sum;
}
#endif


#ifdef _PRE_PRODUCT_HI3751V811
#define WIFI_2G_MAC_TYPE    (1)
#define WIFI_5G_MAC_TYPE    (2)
#define WIFI_P2P_MAC_TYPE   (3)
#define BT_MAC_TYPE         (4)

extern int32_t get_board_mac(uint8_t *puc_buf, uint8_t type);
#endif

#if defined(_PRE_CONFIG_READ_DYNCALI_E2PROM) && defined(_PRE_CONFIG_READ_E2PROM_MAC)
/*
 * 函 数 名  : hwifi_get_mac_addr_drveprom
 * 功能描述  : 从e2prom中获取mac地址，用宏_PRE_CONFIG_READ_E2PROM_MACADDR区分PC和大屏
 */
int32_t hwifi_get_mac_addr_drveprom(uint8_t *puc_buf)
{
#define READ_CHECK_MAX_CNT    20
#define READ_CHECK_WAIT_TIME  50

    char original_mac_addr[NUM_12_BYTES] = {0};
    int32_t ret = INI_FAILED;
    int32_t i;
    unsigned int offset = 0;
    unsigned int bit_len = 12;
    uint32_t sum = 0;

    if (g_wifi_mac[MAC_ADDR_0] != 0 || g_wifi_mac[MAC_ADDR_1]  != 0 || g_wifi_mac[MAC_ADDR_2]  != 0 ||
        g_wifi_mac[MAC_ADDR_3] != 0 || g_wifi_mac[MAC_ADDR_4] != 0 || g_wifi_mac[MAC_ADDR_5] != 0) {
        if (memcpy_s(puc_buf, WLAN_MAC_ADDR_LEN, g_wifi_mac, WLAN_MAC_ADDR_LEN) != EOK) {
            return INI_FAILED;
        }

        ini_warning("hwifi_get_mac_addr_drveprom get MAC from g_wifi_mac SUCC\n");
        return INI_SUCC;
    }

    for (i = 0; i < READ_CHECK_MAX_CNT; i++) {
        if (ini_eeprom_read("MACWLAN", offset, original_mac_addr, bit_len) == INI_SUCC) {
            ini_warning("hwifi_get_mac_addr_drveprom get MAC from EEPROM SUCC\n");
            ret = INI_SUCC;
            break;
        }

        msleep(READ_CHECK_WAIT_TIME);
    }

    if (ret == INI_SUCC) {
        oal_io_print("hwifi_get_mac_addr_drveprom ini_eeprom_read return success\n");
        sum = char2byte(original_mac_addr, (char *)puc_buf);
        if (sum != 0) {
            ini_warning("hwifi_get_mac_addr_drveprom get MAC from EEPROM: mac=" MACFMT "\n", ini_mac2str(puc_buf));
            if (memcpy_s(g_wifi_mac, WLAN_MAC_ADDR_LEN, puc_buf, WLAN_MAC_ADDR_LEN) != EOK) {
                ini_warning("hwifi_get_mac_addr_drveprom memcpy_s g_wifi_mac fail\n");
            }
        } else {
            ini_warning("hwifi_get_mac_addr_drveprom get MAC from EEPROM is not char,use random mac\n");
            random_ether_addr(puc_buf);
            puc_buf[1] = 0x11;
            puc_buf[BYTE_OFFSET_2] = 0x03;
        }
    } else {
        oal_io_print("hwifi_get_mac_addr ini_eeprom_read return fail,use random mac\n");
        chr_exception_report(CHR_READ_EEPROM_ERROR_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_INIT, CHR_PLAT_DRV_ERROR_EEPROM_READ_INIT);
        random_ether_addr(puc_buf);
        puc_buf[1] = 0x11;
        puc_buf[BYTE_OFFSET_2] = 0x03;
    }

    return ret;
}
#endif

/*
 * 函 数 名  : hwifi_get_mac_addr
 * 功能描述  : 从e2prom中获取mac地址，用宏_PRE_CONFIG_READ_E2PROM_MACADDR区分PC和大屏
 */
#if defined(_PRE_CONFIG_READ_DYNCALI_E2PROM) && (!defined(_PRE_CONFIG_READ_E2PROM_MAC))
int32_t hwifi_get_mac_addr(uint8_t *puc_buf)
{
    if (puc_buf == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hwifi_get_mac_addr::buf is NULL!");
        return INI_FAILED;
    }

    if (get_board_mac(puc_buf, WIFI_2G_MAC_TYPE) == 0) {
        ini_warning("hwifi_get_mac_addr get MAC from NV: mac="MACFMT"\n", ini_mac2str(puc_buf));
        if (memcpy_s(g_wifi_mac, WLAN_MAC_ADDR_LEN, puc_buf, WLAN_MAC_ADDR_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "hwifi_get_mac_addr::memcpy_s mac to g_wifi_mac failed");
        }
    } else {
        random_ether_addr(puc_buf);
        puc_buf[1] = 0x11;
        puc_buf[BYTE_OFFSET_2] = 0x02;
        ini_warning("hwifi_get_mac_addr get random mac: mac="MACFMT"\n", ini_mac2str(puc_buf));
    }
    return INI_SUCC;
}

#elif defined(_PRE_CONFIG_READ_DYNCALI_E2PROM) && defined(_PRE_CONFIG_READ_E2PROM_MAC)
int32_t hwifi_get_mac_addr(uint8_t *puc_buf)
{
    return hwifi_get_mac_addr_drveprom(puc_buf);
}

#else
int32_t hwifi_get_mac_addr(uint8_t *puc_buf)
{
#ifdef CONFIG_ARCH_PLATFORM
    struct opt_nve_info_user st_info;
#else
    struct hisi_nve_info_user st_info;
#endif
    int32_t l_ret;
    uint32_t sum = 0;

    if (puc_buf == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hwifi_get_mac_addr::buf is NULL!");
        return INI_FAILED;
    }

    memset_s(puc_buf, WLAN_MAC_ADDR_LEN, 0, WLAN_MAC_ADDR_LEN);

    memset_s(&st_info, sizeof(st_info), 0, sizeof(st_info));

    st_info.nv_number = NV_WLAN_NUM;  // nve item

    if (strcpy_s(st_info.nv_name, sizeof(st_info.nv_name), "MACWLAN") != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hwifi_get_mac_addr:: strcpy_s failed.");
        return INI_FAILED;
    }

    st_info.valid_size = NV_WLAN_VALID_SIZE;
    st_info.nv_operation = NV_READ;

    if (g_wifi_mac[0] != 0 || g_wifi_mac[1]  != 0 || g_wifi_mac[BYTE_OFFSET_2]  != 0 || g_wifi_mac[BYTE_OFFSET_3] != 0
        || g_wifi_mac[BYTE_OFFSET_4] != 0 || g_wifi_mac[BYTE_OFFSET_5] != 0) {
        if (memcpy_s(puc_buf, WLAN_MAC_ADDR_LEN, g_wifi_mac, WLAN_MAC_ADDR_LEN) != EOK) {
            return INI_FAILED;
        }

        return INI_SUCC;
    }

#ifdef CONFIG_ARCH_PLATFORM
    l_ret = nve_direct_access_interface(&st_info);
#else
    l_ret = hisi_nve_direct_access(&st_info);
#endif
    if (!l_ret) {
        sum = char2byte(st_info.nv_data, (char *)puc_buf);
        if (sum != 0) {
            ini_warning("hwifi_get_mac_addr get MAC from NV: mac=" MACFMT "\n", ini_mac2str(puc_buf));
            if (memcpy_s(g_wifi_mac, WLAN_MAC_ADDR_LEN, puc_buf, WLAN_MAC_ADDR_LEN) != EOK) {
                return INI_FAILED;
            }
        } else {
            random_ether_addr(puc_buf);
            puc_buf[BYTE_OFFSET_1] = 0x11;
            puc_buf[BYTE_OFFSET_2] = 0x02;
        }
    } else {
        random_ether_addr(puc_buf);
        puc_buf[BYTE_OFFSET_1] = 0x11;
        puc_buf[BYTE_OFFSET_2] = 0x02;
    }

    return INI_SUCC;
}
#endif

int32_t hwifi_get_init_value(int32_t cus_tag, int32_t cfg_id)
{
    int32_t *pgal_params = NULL;
    int32_t l_wlan_cfg_butt;

    if (cus_tag == CUS_TAG_INI) {
        pgal_params = &g_host_init_params[0];
        l_wlan_cfg_butt = WLAN_CFG_INIT_BUTT;
    } else if (cus_tag == CUS_TAG_DTS) {
        pgal_params = &g_dts_params[0];
        l_wlan_cfg_butt = WLAN_CFG_DTS_BUTT;
    } else {
        oam_error_log1(0, OAM_SF_ANY, "hwifi_get_init_value tag number[0x%2x] not correct!", cus_tag);
        return INI_FAILED;
    }

    if (cfg_id < 0 || l_wlan_cfg_butt <= cfg_id) {
        oam_error_log2(0, OAM_SF_ANY, "hwifi_get_init_value cfg id[%d] out of range, max cfg id is:%d",
                       cfg_id, (l_wlan_cfg_butt - 1));
        return INI_FAILED;
    }

    return pgal_params[cfg_id];
}

int32_t hwifi_get_init_priv_value(int32_t l_cfg_id, int32_t *pl_priv_value)
{
    if (l_cfg_id < 0 || l_cfg_id >= WLAN_CFG_PRIV_BUTT) {
        oam_error_log2(0, OAM_SF_ANY, "hwifi_get_init_priv_value cfg id[%d] out of range, max[%d]",
                       l_cfg_id, WLAN_CFG_PRIV_BUTT - 1);
        return OAL_FAIL;
    }

    if (g_priv_cust_params[l_cfg_id].en_value_state == OAL_FALSE) {
        return OAL_FAIL;
    }

    *pl_priv_value = g_priv_cust_params[l_cfg_id].l_val;

    return OAL_SUCC;
}

/*
 * 函 数 名  : hwifi_get_init_nvram_params
 * 功能描述  : 获取定制化NV数据结构体
 */
static void *hwifi_get_init_nvram_params(void)
{
    return &g_cust_nv_params;
}

/*
 * 函 数 名  : hwifi_get_nvram_params
 * 功能描述  : 获取定制化最大发射功率和对应的scaling值
 */
static void *hwifi_get_nvram_params(void)
{
    return &g_cust_nv_params.st_pow_ctrl_custom_param;
}

/*
 * 函 数 名  : hwifi_get_cfg_pow_ctrl_params
 * 功能描述  : host查看ini定制化参数维测命令
 */
OAL_STATIC void hwifi_get_cfg_pow_ctrl_params(uint8_t uc_chn_idx)
{
    int32_t l_cfg_idx_one = 0;
    int32_t l_cfg_idx_two = 0;
    wlan_cust_nvram_params *pst_g_cust_nv_params = hwifi_get_nvram_params();
    wlan_cust_cfg_custom_fcc_ce_txpwr_limit_stru *pst_fcc_ce_param;

    pst_fcc_ce_param = &pst_g_cust_nv_params->ast_fcc_ce_param[uc_chn_idx];
    for (l_cfg_idx_one = 0; l_cfg_idx_one < CUS_NUM_5G_20M_SIDE_BAND; l_cfg_idx_one++) {
        oal_io_print("%s[%d] \t [config:%d]\n", "fcc_txpwr_limit_5g:20M side_band", l_cfg_idx_one,
                     pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_20m[l_cfg_idx_one]);
    }

    for (l_cfg_idx_one = 0; l_cfg_idx_one < CUS_NUM_5G_40M_SIDE_BAND; l_cfg_idx_one++) {
        oal_io_print("%s[%d] \t [config:%d]\n", "fcc_txpwr_limit_5g:40M side_band", l_cfg_idx_one,
                     pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_40m[l_cfg_idx_one]);
    }
    for (l_cfg_idx_one = 0; l_cfg_idx_one < CUS_NUM_5G_80M_SIDE_BAND; l_cfg_idx_one++) {
        oal_io_print("%s[%d] \t [config:%d]\n", "fcc_txpwr_limit_5g:80M side_band", l_cfg_idx_one,
                     pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_80m[l_cfg_idx_one]);
    }
    for (l_cfg_idx_one = 0; l_cfg_idx_one < CUS_NUM_5G_160M_SIDE_BAND; l_cfg_idx_one++) {
        oal_io_print("%s[%d] \t [config:%d]\n", "fcc_txpwr_limit_5g:160M side_band", l_cfg_idx_one,
                     pst_fcc_ce_param->auc_5g_fcc_txpwr_limit_params_160m[l_cfg_idx_one]);
    }
    for (l_cfg_idx_one = 0; l_cfg_idx_one < MAC_2G_CHANNEL_NUM; l_cfg_idx_one++) {
        for (l_cfg_idx_two = 0; l_cfg_idx_two < CUS_NUM_FCC_CE_2G_PRO; l_cfg_idx_two++) {
            oal_io_print("%s[%d] [%d] \t [config:%d]\n", "fcc_txpwr_limit_2g: chan", l_cfg_idx_one, l_cfg_idx_two,
                         pst_fcc_ce_param->auc_2g_fcc_txpwr_limit_params[l_cfg_idx_one][l_cfg_idx_two]);
        }
    }
}

/*
 * 函 数 名  : hwifi_get_cfg_iq_lpf_lvl_params
 * 功能描述  : host查看ini定制化参数维测命令
 */
OAL_STATIC void hwifi_get_cfg_iq_lpf_lvl_params(void)
{
    int32_t l_cfg_idx_one = 0;
    wlan_cust_nvram_params *pst_g_cust_nv_params = hwifi_get_nvram_params();

    for (l_cfg_idx_one = 0; l_cfg_idx_one < NUM_OF_NV_5G_LPF_LVL; ++l_cfg_idx_one) {
        oal_io_print("%s%d \t [config:%d]\n", "5g_iq_cali_lpf_lvl", l_cfg_idx_one,
                     pst_g_cust_nv_params->auc_5g_iq_cali_lpf_params[l_cfg_idx_one]);
    }
}

/*
 * 函 数 名  : hwifi_get_cfg_params
 * 功能描述  : host查看ini定制化参数维测命令
 */
void hwifi_get_cfg_params(void)
{
    int32_t l_cfg_idx_one = 0;
    int32_t l_cfg_idx_two = 0;
    wlan_cust_nvram_params *pst_g_cust_nv_params = hwifi_get_nvram_params(); /* 最大发送功率定制化数组 */
    wlan_init_cust_nvram_params *pst_init_g_cust_nv_params = hwifi_get_init_nvram_params();

    oal_io_print("\nhwifi_get_cfg_params\n");

    // CUS_TAG_INI
    for (l_cfg_idx_one = 0; l_cfg_idx_one < WLAN_CFG_INIT_BUTT; ++l_cfg_idx_one) {
        oal_io_print("%s \t [config:0x%x]\n", g_wifi_config_cmds[l_cfg_idx_one].name,
                     g_host_init_params[l_cfg_idx_one]);
    }

    // CUS_TAG_TXPWR
    for (l_cfg_idx_one = 0; l_cfg_idx_one < NUM_OF_NV_MAX_TXPOWER; ++l_cfg_idx_one) {
        oal_io_print("%s%d \t [config:%d]\n", "delt_txpwr_params", l_cfg_idx_one,
                     pst_g_cust_nv_params->ac_delt_txpwr_params[l_cfg_idx_one]);
    }

    for (l_cfg_idx_one = 0; l_cfg_idx_one < NUM_OF_NV_DPD_MAX_TXPOWER; ++l_cfg_idx_one) {
        oal_io_print("%s%d \t [config:%d]\n", "delt_dpd_txpwr_params",
                     l_cfg_idx_one, pst_g_cust_nv_params->ac_dpd_delt_txpwr_params[l_cfg_idx_one]);
    }

    for (l_cfg_idx_one = 0; l_cfg_idx_one < NUM_OF_NV_11B_DELTA_TXPOWER; ++l_cfg_idx_one) {
        oal_io_print("%s%d \t [config:%d]\n", "delt_11b_txpwr_params", l_cfg_idx_one,
                     pst_g_cust_nv_params->ac_11b_delt_txpwr_params[l_cfg_idx_one]);
    }

    for (l_cfg_idx_one = 0; l_cfg_idx_one < WLAN_RF_CHANNEL_NUMS; ++l_cfg_idx_one) {
        oal_io_print("%s%d \t [config:%d]\n", "5g_IQ_cali_pow", l_cfg_idx_one,
                     pst_g_cust_nv_params->auc_fem_off_iq_cal_pow_params[l_cfg_idx_one]);
    }

    for (l_cfg_idx_one = 0; l_cfg_idx_one < NUM_OF_NV_5G_UPPER_UPC; ++l_cfg_idx_one) {
        oal_io_print("%s%d \t [config:%d]\n", "5g_upper_upc_params", l_cfg_idx_one,
                     pst_g_cust_nv_params->auc_5g_upper_upc_params[l_cfg_idx_one]);
    }

    for (l_cfg_idx_one = 0; l_cfg_idx_one < NUM_OF_NV_2G_LOW_POW_DELTA_VAL; ++l_cfg_idx_one) {
        oal_io_print("%s%d \t [config:%d]\n", "2g_low_pow_amend_val", l_cfg_idx_one,
                     pst_g_cust_nv_params->ac_2g_low_pow_amend_params[l_cfg_idx_one]);
    }

    for (l_cfg_idx_one = 0; l_cfg_idx_one < WLAN_RF_CHANNEL_NUMS; l_cfg_idx_one++) {
        for (l_cfg_idx_two = 0; l_cfg_idx_two < CUS_BASE_PWR_NUM_2G; l_cfg_idx_two++) {
            oal_io_print("%s[%d][%d] \t [config:%d]\n", "2G base_pwr_params", l_cfg_idx_one,
                         l_cfg_idx_two, pst_g_cust_nv_params->auc_2g_txpwr_base_params[l_cfg_idx_one][l_cfg_idx_two]);
        }
        for (l_cfg_idx_two = 0; l_cfg_idx_two < CUS_BASE_PWR_NUM_5G; l_cfg_idx_two++) {
            oal_io_print("%s[%d][%d] \t [config:%d]\n", "5G base_pwr_params", l_cfg_idx_one,
                         l_cfg_idx_two, pst_g_cust_nv_params->auc_5g_txpwr_base_params[l_cfg_idx_one][l_cfg_idx_two]);
        }
    }

    for (l_cfg_idx_one = 0; l_cfg_idx_one < WLAN_RF_CHANNEL_NUMS; l_cfg_idx_one++) {
        /* FCC/CE */
        oal_io_print("%s \t [RF:%d]\n", "hwifi_get_cfg_pow_ctrl_params", l_cfg_idx_one);
        hwifi_get_cfg_pow_ctrl_params(l_cfg_idx_one);
    }

    for (l_cfg_idx_one = 0; l_cfg_idx_one < CUS_NUM_OF_SAR_LVL; l_cfg_idx_one++) {
        for (l_cfg_idx_two = 0; l_cfg_idx_two < CUS_NUM_OF_SAR_PARAMS; l_cfg_idx_two++) {
            oal_io_print("%s[%d][%d] \t [config:C0 %d C1 %d]\n", "sar_ctrl_params: lvl", l_cfg_idx_one, l_cfg_idx_two,
                pst_init_g_cust_nv_params->st_sar_ctrl_params[l_cfg_idx_one][l_cfg_idx_two].auc_sar_ctrl_params_c0,
                pst_init_g_cust_nv_params->st_sar_ctrl_params[l_cfg_idx_one][l_cfg_idx_two].auc_sar_ctrl_params_c1);
        }
    }

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    for (l_cfg_idx_one = 0; l_cfg_idx_one < WLAN_RF_CHANNEL_NUMS; l_cfg_idx_one++) {
        for (l_cfg_idx_two = 0; l_cfg_idx_two < WLAN_BAND_BUTT; l_cfg_idx_two++) {
            oal_io_print("%s[%d][%d] \t [config:%d]\n", "tas_ctrl_params: lvl", l_cfg_idx_one, l_cfg_idx_two,
                         pst_g_cust_nv_params->auc_tas_ctrl_params[l_cfg_idx_one][l_cfg_idx_two]);
        }
    }
#endif

    oal_io_print("%s \t [config:%d]\n", g_nvram_config_ini[NVRAM_PARAMS_5G_FCC_CE_HIGH_BAND_MAX_PWR].name,
                 pst_g_cust_nv_params->uc_5g_max_pwr_fcc_ce_for_high_band);
    // CUS_TAG_DTS
    for (l_cfg_idx_one = 0; l_cfg_idx_one < WLAN_CFG_DTS_BUTT; ++l_cfg_idx_one) {
        oal_io_print("%s \t [config:0x%x]\n", g_wifi_config_dts[l_cfg_idx_one].name,
                     g_dts_params[l_cfg_idx_one]);
    }
    hwifi_get_cfg_iq_lpf_lvl_params();

    /* pro line */
    for (l_cfg_idx_one = 0; l_cfg_idx_one < WLAN_RF_CHANNEL_NUMS; l_cfg_idx_one++) {
        for (l_cfg_idx_two = 0; l_cfg_idx_two < DY_CALI_PARAMS_NUM; l_cfg_idx_two++) {
            oal_io_print("%s CORE[%d]para_idx[%d]::{%d, %d, %d}\n", "g_pro_line_params: ",
                         l_cfg_idx_one, l_cfg_idx_two,
                         g_pro_line_params[l_cfg_idx_one][l_cfg_idx_two].l_pow_par2,
                         g_pro_line_params[l_cfg_idx_one][l_cfg_idx_two].l_pow_par1,
                         g_pro_line_params[l_cfg_idx_one][l_cfg_idx_two].l_pow_par0);
        }
    }
    /* NVRAM */
    oal_io_print("%s : { %d }\n", "en_nv_dp_init_is_null: ", g_en_nv_dp_init_is_null);
    for (l_cfg_idx_one = 0; l_cfg_idx_one < WLAN_CFG_DTS_NVRAM_END; l_cfg_idx_one++) {
        oal_io_print("%s para_idx[%d] name[%s]::DATA{%s}\n", "dp init & ratios nvram_param: ", l_cfg_idx_one,
                     g_wifi_nvram_cfg_handler[l_cfg_idx_one].puc_param_name,
                     hwifi_get_nvram_param(l_cfg_idx_one));
    }

    /* RU DELT POW  */
    hwifi_get_cfg_delt_ru_pow_params();
}


uint32_t hwifi_config_init_nvram_main_1103(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint16_t us_offset = sizeof(wlan_cust_nvram_params); /* 包括4个基准功率 */

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_NVRAM_PARAM, us_offset);
    l_ret = memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value), hwifi_get_nvram_params(), us_offset);
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hwifi_config_init_nvram_main_1103::memcpy fail!");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_cfg_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + us_offset,
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{hwifi_config_init_nvram_main_1103::err [%d]!}", l_ret);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


void hwifi_config_cpu_freq_ini_param_1103(void)
{
    int32_t l_val;

#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_PRIV_DMA_LATENCY);
    g_freq_lock_control.uc_lock_dma_latency = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    g_freq_lock_control.dma_latency_value = (uint16_t)l_val;
    oal_io_print("DMA latency[%d]\r\n", g_freq_lock_control.uc_lock_dma_latency);
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_PRIV_LOCK_CPU_TH_HIGH);
    g_freq_lock_control.us_lock_cpu_th_high = (uint16_t)l_val;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_PRIV_LOCK_CPU_TH_LOW);
    g_freq_lock_control.us_lock_cpu_th_low = (uint16_t)l_val;
    oal_io_print("CPU freq high[%d] low[%d]\r\n", g_freq_lock_control.us_lock_cpu_th_high,
        g_freq_lock_control.us_lock_cpu_th_low);
#endif

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_IRQ_AFFINITY);
    g_freq_lock_control.en_irq_affinity = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_IRQ_TH_HIGH);
    g_freq_lock_control.us_throughput_irq_high = (l_val > 0) ? (uint16_t)l_val : WLAN_IRQ_TH_HIGH;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_IRQ_TH_LOW);
    g_freq_lock_control.us_throughput_irq_low = (l_val > 0) ? (uint16_t)l_val : WLAN_IRQ_TH_LOW;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_IRQ_PPS_TH_HIGH);
    g_freq_lock_control.irq_pps_high = (l_val > 0) ? (uint32_t)l_val : WLAN_IRQ_PPS_TH_HIGH;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_IRQ_PPS_TH_LOW);
    g_freq_lock_control.irq_pps_low = (l_val > 0) ? (uint32_t)l_val : WLAN_IRQ_PPS_TH_LOW;
    g_freq_lock_control.en_userctl_bindcpu = OAL_FALSE; /* 用户指定绑核命令默认不开启 */
    g_freq_lock_control.uc_userctl_irqbind = 0;
    g_freq_lock_control.uc_userctl_threadbind = 0;
    oal_io_print("irq affinity enable[%d]High_th[%u]Low_th[%u]\r\n", g_freq_lock_control.en_irq_affinity,
        g_freq_lock_control.us_throughput_irq_high, g_freq_lock_control.us_throughput_irq_low);
}


OAL_STATIC void hwifi_set_voe_custom_param(void)
{
    uint32_t val;

    val = (uint32_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_VOE_SWITCH);
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11k = (val & BIT0) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11v = (val & BIT1) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11r = (val & BIT2) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11r_ds = (val & BIT3) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_adaptive11r = (val & BIT4) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_nb_rpt_11k = (val & BIT5) ? OAL_TRUE : OAL_FALSE;

    return;
}


OAL_STATIC void hwifi_config_host_roam_global_ini_param(void)
{
    int32_t l_val;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_ROAM_SWITCH);
    g_wlan_cust.uc_roam_switch = (0 == l_val || 1 == l_val) ?
        (uint8_t)l_val : g_wlan_cust.uc_roam_switch;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_SCAN_ORTHOGONAL);
        g_wlan_cust.uc_roam_scan_orthogonal = (1 <= l_val) ?
    (uint8_t)l_val : g_wlan_cust.uc_roam_scan_orthogonal;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TRIGGER_B);
    g_wlan_cust.c_roam_trigger_b = (int8_t)l_val;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TRIGGER_A);
    g_wlan_cust.c_roam_trigger_a = (int8_t)l_val;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_B);
    g_wlan_cust.c_roam_delta_b = (int8_t)l_val;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_A);
    g_wlan_cust.c_roam_delta_a = (int8_t)l_val;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DENSE_ENV_TRIGGER_B);
    g_wlan_cust.c_dense_env_roam_trigger_b = (int8_t)l_val;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DENSE_ENV_TRIGGER_A);
    g_wlan_cust.c_dense_env_roam_trigger_a = (int8_t)l_val;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_SCENARIO_ENABLE);
    g_wlan_cust.uc_scenario_enable = (uint8_t)l_val;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_CANDIDATE_GOOD_RSSI);
    g_wlan_cust.c_candidate_good_rssi = (int8_t)l_val;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_CANDIDATE_GOOD_NUM);
    g_wlan_cust.uc_candidate_good_num = (uint8_t)l_val;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_CANDIDATE_WEAK_NUM);
    g_wlan_cust.uc_candidate_weak_num = (uint8_t)l_val;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_INTERVAL_VARIABLE);
    g_wlan_cust.us_roam_interval = (uint16_t)l_val;
}

OAL_STATIC void hwifi_config_tcp_ack_buf_ini_param(void)
{
    int32_t l_val;
    mac_tcp_ack_buf_switch_stru *tcp_ack_buf_switch = mac_vap_get_tcp_ack_buf_switch();

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TX_TCP_ACK_BUF);
    tcp_ack_buf_switch->uc_ini_tcp_ack_buf_en = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_HIGH);
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high = (l_val > 0) ? (uint16_t)l_val : WLAN_TCP_ACK_BUF_HIGH;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_LOW);
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low = (l_val > 0) ? (uint16_t)l_val : 30; /* 吞吐量30 */
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_40M);
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high_40M = (l_val > 0) ? (uint16_t)l_val : WLAN_TCP_ACK_BUF_HIGH_40M;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_LOW_40M);
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low_40M = (l_val > 0) ? (uint16_t)l_val : WLAN_TCP_ACK_BUF_LOW_40M;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_80M);
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high_80M = (l_val > 0) ? (uint16_t)l_val : WLAN_TCP_ACK_BUF_HIGH_80M;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_LOW_80M);
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low_80M = (l_val > 0) ? (uint16_t)l_val : WLAN_TCP_ACK_BUF_LOW_80M;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_160M);
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high_160M = (l_val > 0) ?
                                                              (uint16_t)l_val : WLAN_TCP_ACK_BUF_HIGH_160M;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_LOW_160M);
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low_160M = (l_val > 0) ? (uint16_t)l_val : WLAN_TCP_ACK_BUF_LOW_160M;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TX_TCP_ACK_BUF_USERCTL);
    tcp_ack_buf_switch->uc_ini_tcp_ack_buf_userctl_test_en = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    /* 上层默认设置TCP ack上门限30M */
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_USERCTL_HIGH);
    tcp_ack_buf_switch->us_tcp_ack_buf_userctl_high = (l_val > 0) ? (uint16_t)l_val : WLAN_TCP_ACK_BUF_USERCTL_HIGH;
     /* 上层默认设置TCP ack上门限20M */
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_USERCTL_LOW);
    tcp_ack_buf_switch->us_tcp_ack_buf_userctl_low = (l_val > 0) ? (uint16_t)l_val : WLAN_TCP_ACK_BUF_USERCTL_LOW;
    oal_io_print("TCP ACK BUF en[%d],high/low:20M[%d]/[%d],40M[%d]/[%d],80M[%d]/[%d],160M[%d]/[%d],\
        TCP ACK BUF USERCTL[%d], userctl[%d]/[%d]",
        tcp_ack_buf_switch->uc_ini_tcp_ack_buf_en,
        tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high,
        tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low,
        tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high_40M,
        tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low_40M,
        tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high_80M,
        tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low_80M,
        tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high_160M,
        tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low_160M,
        tcp_ack_buf_switch->uc_ini_tcp_ack_buf_userctl_test_en,
        tcp_ack_buf_switch->us_tcp_ack_buf_userctl_high,
        tcp_ack_buf_switch->us_tcp_ack_buf_userctl_low);
}

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU


OAL_STATIC void hwifi_config_host_amsdu_th_ini_param(void)
{
    int32_t l_val;
    mac_small_amsdu_switch_stru *small_amsdu_switch = mac_vap_get_small_amsdu_switch();

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_AMPDU_AMSDU_SKB);
    g_st_tx_large_amsdu.uc_host_large_amsdu_en = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_AMSDU_AMPDU_TH_HIGH);
    g_st_tx_large_amsdu.us_amsdu_throughput_high = (l_val > 0) ? (uint16_t)l_val : 500;   /* 500是高聚合量 */
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_AMSDU_AMPDU_TH_MIDDLE);
    g_st_tx_large_amsdu.us_amsdu_throughput_middle = (l_val > 0) ? (uint16_t)l_val : 100; /* 100是中等聚合量 */
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_AMSDU_AMPDU_TH_LOW);
    g_st_tx_large_amsdu.us_amsdu_throughput_low = (l_val > 0) ? (uint16_t)l_val : 50;     /* 50是低聚合量 */
    oal_io_print("ampdu+amsdu lareg amsdu en[%d],high[%d],low[%d],middle[%d]\r\n",
        g_st_tx_large_amsdu.uc_host_large_amsdu_en, g_st_tx_large_amsdu.us_amsdu_throughput_high,
        g_st_tx_large_amsdu.us_amsdu_throughput_low, g_st_tx_large_amsdu.us_amsdu_throughput_middle);

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TX_SMALL_AMSDU);
    small_amsdu_switch->uc_ini_small_amsdu_en = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_SMALL_AMSDU_HIGH);
    small_amsdu_switch->us_small_amsdu_throughput_high = (l_val > 0) ? (uint16_t)l_val : WLAN_SMALL_AMSDU_HIGH;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_SMALL_AMSDU_LOW);
    small_amsdu_switch->us_small_amsdu_throughput_low = (l_val > 0) ? (uint16_t)l_val : WLAN_SMALL_AMSDU_LOW;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_SMALL_AMSDU_PPS_HIGH);
    small_amsdu_switch->us_small_amsdu_pps_high = (l_val > 0) ? (uint16_t)l_val : WLAN_SMALL_AMSDU_PPS_HIGH;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_SMALL_AMSDU_PPS_LOW);
    small_amsdu_switch->us_small_amsdu_pps_low = (l_val > 0) ? (uint16_t)l_val : WLAN_SMALL_AMSDU_PPS_LOW;
    oal_io_print("SMALL AMSDU SWITCH en[%d],high[%d],low[%d]\r\n", small_amsdu_switch->uc_ini_small_amsdu_en,
        small_amsdu_switch->us_small_amsdu_throughput_high, small_amsdu_switch->us_small_amsdu_throughput_low);
}
#endif


OAL_STATIC void hwifi_config_performance_ini_param(void)
{
    int32_t l_val;
    mac_rx_buffer_size_stru *rx_buffer_size = mac_vap_get_rx_buffer_size();

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_AMPDU_TX_MAX_NUM);
    g_wlan_cust.ampdu_tx_max_num = (g_wlan_spec_cfg->max_tx_ampdu_num >= l_val && 1 <= l_val) ?
        (uint32_t)l_val : g_wlan_cust.ampdu_tx_max_num;
    oal_io_print("hwifi_config_host_global_ini_param::ampdu_tx_max_num:%d", g_wlan_cust.ampdu_tx_max_num);

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_HW_AMPDU);
    g_st_ampdu_hw.uc_ampdu_hw_en = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_HW_AMPDU_TH_HIGH);
    g_st_ampdu_hw.us_throughput_high = (l_val > 0) ? (uint16_t)l_val : WLAN_HW_AMPDU_TH_HIGH;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_HW_AMPDU_TH_LOW);
    g_st_ampdu_hw.us_throughput_low = (l_val > 0) ? (uint16_t)l_val : WLAN_HW_AMPDU_TH_LOW;
    oal_io_print("ampdu_hw enable[%d]H[%u]L[%u]\r\n", g_st_ampdu_hw.uc_ampdu_hw_en,
        g_st_ampdu_hw.us_throughput_high, g_st_ampdu_hw.us_throughput_low);
#endif

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    hwifi_config_host_amsdu_th_ini_param();
#endif
#ifdef _PRE_WLAN_TCP_OPT
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_FILTER);
    g_st_tcp_ack_filter.uc_tcp_ack_filter_en = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_FILTER_TH_HIGH);
    g_st_tcp_ack_filter.us_rx_filter_throughput_high = (l_val > 0) ? (uint16_t)l_val : 50; /* 吞吐量50 */
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_FILTER_TH_LOW);
    g_st_tcp_ack_filter.us_rx_filter_throughput_low = (l_val > 0) ? (uint16_t)l_val : WLAN_TCP_ACK_FILTER_TH_LOW;
    oal_io_print("tcp ack filter en[%d],high[%d],low[%d]\r\n", g_st_tcp_ack_filter.uc_tcp_ack_filter_en,
        g_st_tcp_ack_filter.us_rx_filter_throughput_high, g_st_tcp_ack_filter.us_rx_filter_throughput_low);
#endif

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RX_AMPDU_AMSDU_SKB);
    g_uc_host_rx_ampdu_amsdu = (l_val > 0) ? (uint8_t)l_val : OAL_FALSE;
    oal_io_print("Rx:ampdu+amsdu skb en[%d]\r\n", g_uc_host_rx_ampdu_amsdu);

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RX_AMPDU_BITMAP);
    rx_buffer_size->en_rx_ampdu_bitmap_ini = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    rx_buffer_size->us_rx_buffer_size = l_val;
    oal_io_print("Rx:ampdu bitmap size[%d]\r\n", l_val);
}

static void hwifi_config_host_global_11ax_ini_param(void)
{
    int32_t l_val;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_11AX_SWITCH);
    g_pst_mac_device_capability[0].en_11ax_switch = (((uint32_t)l_val & 0x0F) & BIT0) ? OAL_TRUE : OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_11AX
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_11ax_aput_switch =
        (((uint32_t)l_val & 0x0F) & BIT1) ? OAL_TRUE : OAL_FALSE;

    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_ignore_non_he_cap_from_beacon =
            (((uint32_t)l_val & 0x0F) & BIT2) ? OAL_TRUE : OAL_FALSE;

    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_11ax_aput_he_cap_switch =
        (((uint32_t)l_val & 0x0F) & BIT3) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_twt_responder_support =
        (((uint32_t)l_val & 0xFF) & BIT4) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_twt_requester_support =
        (((uint32_t)l_val & 0xFF) & BIT5) ? OAL_TRUE : OAL_FALSE;
#endif

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_MULTI_BSSID_SWITCH);
    g_pst_mac_device_capability[0].bit_multi_bssid_switch = (((uint32_t)l_val & 0x0F) & BIT0) ? OAL_TRUE : OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_11AX
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_HTC_SWITCH);
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_htc_include =
        (((uint32_t)l_val & 0x0F) & BIT0) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_om_in_data =
        (((uint32_t)l_val & 0x0F) & BIT1) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_rom_cap_switch =
        (((uint32_t)l_val & 0x0F) & BIT2) ? OAL_TRUE : OAL_FALSE;
#endif
}


OAL_STATIC void hwifi_config_dmac_freq_ini_param(void)
{
    uint32_t cfg_id;
    uint32_t val;
    int32_t l_cfg_value;
    int8_t *pc_tmp;
    host_speed_freq_level_stru ast_host_speed_freq_level_tmp[NUM_4_BITS];
    device_speed_freq_level_stru ast_device_speed_freq_level_tmp[NUM_4_BITS];
    uint8_t uc_flag = OAL_FALSE;
    uint8_t uc_index;
    int32_t l_ret = EOK;

    /******************************************** 自动调频 ********************************************/
    /* config g_host_speed_freq_level */
    pc_tmp = (int8_t *)&ast_host_speed_freq_level_tmp;

    for (cfg_id = WLAN_CFG_INIT_PSS_THRESHOLD_LEVEL_0; cfg_id <= WLAN_CFG_INIT_DDR_FREQ_LIMIT_LEVEL_3; ++cfg_id) {
        val = hwifi_get_init_value(CUS_TAG_INI, cfg_id);
        *(uint32_t *)pc_tmp = val;
        pc_tmp += BYTE_OFFSET_4;
    }

    /* config g_device_speed_freq_level */
    pc_tmp = (int8_t *)&ast_device_speed_freq_level_tmp;
    for (cfg_id = WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_0; cfg_id <= WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_3; ++cfg_id) {
        l_cfg_value = hwifi_get_init_value(CUS_TAG_INI, cfg_id);
        if (oal_value_in_valid_range(l_cfg_value, FREQ_IDLE, FREQ_HIGHEST)) {
            *pc_tmp = l_cfg_value;
            pc_tmp += BIT_OFFSET_4;
        } else {
            uc_flag = OAL_TRUE;
            break;
        }
    }

    if (!uc_flag) {
        l_ret += memcpy_s(&g_host_speed_freq_level, sizeof(g_host_speed_freq_level),
                          &ast_host_speed_freq_level_tmp, sizeof(ast_host_speed_freq_level_tmp));
        l_ret += memcpy_s(&g_device_speed_freq_level, sizeof(g_device_speed_freq_level),
                          &ast_device_speed_freq_level_tmp, sizeof(ast_device_speed_freq_level_tmp));
        if (l_ret != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "hwifi_config_host_global_ini_param::memcpy fail!");
            return;
        }

        for (uc_index = 0; uc_index < HOST_SPEED_FREQ_LEVEL_BUTT; uc_index++) {
            oam_warning_log4(0, OAM_SF_ANY, "{hwifi_config_host_global_ini_param::ul_speed_level = %d, \
                min_cpu_freq = %d, min_ddr_freq = %d, uc_device_type = %d}\r\n",
                g_host_speed_freq_level[uc_index].speed_level,
                g_host_speed_freq_level[uc_index].min_cpu_freq,
                g_host_speed_freq_level[uc_index].min_ddr_freq,
                g_device_speed_freq_level[uc_index].uc_device_type);
        }
    }
}

OAL_STATIC void hwifi_config_bypass_extlna_ini_param(void)
{
    int32_t l_val;
    mac_rx_dyn_bypass_extlna_stru *rx_dyn_bypass_extlna_switch = NULL;
    rx_dyn_bypass_extlna_switch = mac_vap_get_rx_dyn_bypass_extlna_switch();

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA);
    rx_dyn_bypass_extlna_switch->uc_ini_en = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    rx_dyn_bypass_extlna_switch->uc_cur_status = OAL_TRUE; /* 默认低功耗场景 */
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_HIGH);
    rx_dyn_bypass_extlna_switch->us_throughput_high = (l_val > 0) ? (uint16_t)l_val : WLAN_RX_DYN_BYPASS_EXTLNA_HIGH;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_LOW);
    rx_dyn_bypass_extlna_switch->us_throughput_low = (l_val > 0) ? (uint16_t)l_val : WLAN_RX_DYN_BYPASS_EXTLNA_LOW;

    oal_io_print("DYN_BYPASS_EXTLNA SWITCH en[%d],high[%d],low[%d]\r\n", rx_dyn_bypass_extlna_switch->uc_ini_en,
        rx_dyn_bypass_extlna_switch->us_throughput_high, rx_dyn_bypass_extlna_switch->us_throughput_low);
}


static void hwifi_config_factory_lte_gpio_ini_param(void)
{
    int32_t val;

    val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_LTE_GPIO_CHECK_SWITCH);
    g_wlan_cust.lte_gpio_check_switch = (uint32_t) !!val;
    val = hwifi_get_init_value(CUS_TAG_INI, WLAN_ATCMDSRV_ISM_PRIORITY);
    g_wlan_cust.ism_priority = (uint32_t)val;
    val = hwifi_get_init_value(CUS_TAG_INI, WLAN_ATCMDSRV_LTE_RX);
    g_wlan_cust.lte_rx = (uint32_t)val;
    val = hwifi_get_init_value(CUS_TAG_INI, WLAN_ATCMDSRV_LTE_TX);
    g_wlan_cust.lte_tx = (uint32_t)val;
    val = hwifi_get_init_value(CUS_TAG_INI, WLAN_ATCMDSRV_LTE_INACT);
    g_wlan_cust.lte_inact = (uint32_t)val;
    val = hwifi_get_init_value(CUS_TAG_INI, WLAN_ATCMDSRV_ISM_RX_ACT);
    g_wlan_cust.ism_rx_act = (uint32_t)val;
    val = hwifi_get_init_value(CUS_TAG_INI, WLAN_ATCMDSRV_BANT_PRI);
    g_wlan_cust.bant_pri = (uint32_t)val;
    val = hwifi_get_init_value(CUS_TAG_INI, WLAN_ATCMDSRV_BANT_STATUS);
    g_wlan_cust.bant_status = (uint32_t)val;
    val = hwifi_get_init_value(CUS_TAG_INI, WLAN_ATCMDSRV_WANT_PRI);
    g_wlan_cust.want_pri = (uint32_t)val;
    val = hwifi_get_init_value(CUS_TAG_INI, WLAN_ATCMDSRV_WANT_STATUS);
    g_wlan_cust.want_status = (uint32_t)val;
}


OAL_STATIC void hwifi_config_host_global_ini_param_extend(void)
{
    int32_t l_val;

#ifdef _PRE_WLAN_FEATURE_MBO
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_MBO_SWITCH);
    g_uc_mbo_switch = !!l_val;
#endif

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DYNAMIC_DBAC_SWITCH);
    g_uc_dbac_dynamic_switch = !!l_val;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DDR_FREQ);
    g_ddr_freq = (uint32_t)l_val;

#ifdef _PRE_WLAN_FEATURE_HIEX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_HIEX_CAP);
        if (memcpy_s(&g_st_default_hiex_cap, sizeof(mac_hiex_cap_stru), &l_val,
            sizeof(l_val)) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "hwifi_config_host_global_ini_param::hiex cap memcpy fail!");
        }
    }
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    if (g_wlan_spec_cfg->feature_ftm_is_open) {
        l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_FTM_CAP);
        g_mac_ftm_cap = (uint8_t)l_val;
    }
#endif
}



void hwifi_config_host_global_ini_param_1103(void)
{
    int32_t l_val;

    /******************************************** 漫游 ********************************************/
    hwifi_config_host_roam_global_ini_param();

    /******************************************** 性能 ********************************************/
    wlan_chip_cpu_freq_ini_param_init();

    hwifi_config_tcp_ack_buf_ini_param();

    hwifi_config_dmac_freq_ini_param();

    hwifi_config_bypass_extlna_ini_param();

    hwifi_config_performance_ini_param();

#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    /******************************************** 组播聚合 ********************************************/
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_MCAST_AMPDU_ENABLE);
    g_mcast_ampdu_cfg.mcast_ampdu_enable = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
#endif
    /******************************************** 随机MAC地址扫描 ********************************************/
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RANDOM_MAC_ADDR_SCAN);
    g_wlan_cust.uc_random_mac_addr_scan = !!l_val;

    /******************************************** CAPABILITY ********************************************/
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DISABLE_CAPAB_2GHT40);
    g_wlan_cust.uc_disable_capab_2ght40 = (uint8_t) !!l_val;
    /********************************************factory_lte_gpio_check ********************************************/
    hwifi_config_factory_lte_gpio_ini_param();

    hwifi_config_host_global_ini_param_extend();

    hwifi_set_voe_custom_param();
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        hwifi_config_host_global_11ax_ini_param();
    }
#endif
    return;
}

OAL_STATIC uint32_t hwifi_cfg_front_end_set_2g_rf(mac_cfg_customize_rf *pst_customize_rf)
{
    uint8_t uc_idx; /* 结构体数组下标 */
    int32_t l_mult4;
    int8_t c_mult4_rf[NUM_2_BYTES];
    /* 配置: 2g rf */
    for (uc_idx = 0; uc_idx < MAC_NUM_2G_BAND; ++uc_idx) {
        /* 获取各2p4g 各band 0.25db及0.1db精度的线损值 */
        l_mult4 = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND_START + uc_idx);
        /* rf0 */
        c_mult4_rf[0] = (int8_t)cus_get_first_byte(l_mult4);
        /* rf1 */
        c_mult4_rf[1] = (int8_t)cus_get_second_byte(l_mult4);
        if (cus_val_valid(c_mult4_rf[0], RF_LINE_TXRX_GAIN_DB_MAX, RF_LINE_TXRX_GAIN_DB_2G_MIN) &&
            cus_val_valid(c_mult4_rf[1], RF_LINE_TXRX_GAIN_DB_MAX, RF_LINE_TXRX_GAIN_DB_2G_MIN)) {
            pst_customize_rf->ast_rf_gain_db_rf[0].ac_gain_db_2g[uc_idx].c_rf_gain_db_mult4 = c_mult4_rf[0];
            pst_customize_rf->ast_rf_gain_db_rf[1].ac_gain_db_2g[uc_idx].c_rf_gain_db_mult4 = c_mult4_rf[1];
        } else {
            /* 值超出有效范围 */
            oam_error_log2(0, OAM_SF_CFG,
                "{hwifi_cfg_front_end_set_2g_rf::ini_id[%d]value out of range, 2g mult4[0x%0x}!}",
                WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND_START + uc_idx, l_mult4);
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t hwifi_cfg_front_end_set_5g_rf(mac_cfg_customize_rf *pst_customize_rf)
{
    uint8_t uc_idx; /* 结构体数组下标 */
    int32_t l_mult4;
    int8_t c_mult4_rf[NUM_2_BYTES];
    /* 配置: 5g rf */
    /* 配置: fem口到天线口的负增益 */
    for (uc_idx = 0; uc_idx < MAC_NUM_5G_BAND; ++uc_idx) {
        /* 获取各5g 各band 0.25db及0.1db精度的线损值 */
        l_mult4 = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND_START + uc_idx);
        c_mult4_rf[0] = (int8_t)cus_get_first_byte(l_mult4);
        c_mult4_rf[1] = (int8_t)cus_get_second_byte(l_mult4);
        if (c_mult4_rf[0] <= RF_LINE_TXRX_GAIN_DB_MAX && c_mult4_rf[1] <= RF_LINE_TXRX_GAIN_DB_MAX) {
            pst_customize_rf->ast_rf_gain_db_rf[0].ac_gain_db_5g[uc_idx].c_rf_gain_db_mult4 = c_mult4_rf[0];
            pst_customize_rf->ast_rf_gain_db_rf[1].ac_gain_db_5g[uc_idx].c_rf_gain_db_mult4 = c_mult4_rf[1];
        } else {
            /* 值超出有效范围 */
            oam_error_log2(0, OAM_SF_CFG,
                "{hwifi_cfg_front_end_set_5g_rf::ini_id[%d]value out of range, 5g mult4[0x%0x}}",
                WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND_START + uc_idx, l_mult4);
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}



OAL_STATIC uint32_t hwifi_cfg_front_end_value_range_check(mac_cfg_customize_rf *pst_customize_rf,
    int32_t l_wlan_band, int32_t l_rf_db_min)
{
    return ((pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ZERO].c_lna_bypass_gain_db < l_rf_db_min ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ZERO].c_lna_bypass_gain_db > 0 ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b0_db < l_rf_db_min ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b0_db > 0 ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b1_db < l_rf_db_min ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b1_db > 0 ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ZERO].uc_pa_gain_lvl_num == 0 ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ZERO].uc_pa_gain_lvl_num > MAC_EXT_PA_GAIN_MAX_LVL ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ZERO].c_lna_gain_db < LNA_GAIN_DB_MIN ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ZERO].c_lna_gain_db > LNA_GAIN_DB_MAX) ||
        (pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ONE].c_lna_bypass_gain_db < l_rf_db_min ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ONE].c_lna_bypass_gain_db > 0 ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ONE].c_pa_gain_b0_db < l_rf_db_min ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ONE].c_pa_gain_b0_db > 0 ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ONE].c_pa_gain_b1_db < l_rf_db_min ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ONE].c_pa_gain_b1_db > 0 ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ONE].uc_pa_gain_lvl_num == 0 ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ONE].uc_pa_gain_lvl_num > MAC_EXT_PA_GAIN_MAX_LVL ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ONE].c_lna_gain_db < LNA_GAIN_DB_MIN ||
        pst_customize_rf->ast_ext_rf[l_wlan_band][WLAN_RF_CHANNEL_ONE].c_lna_gain_db > LNA_GAIN_DB_MAX));
}

OAL_STATIC void hwifi_cfg_front_end_2g_rf0_fem(mac_cfg_customize_rf *pst_customize_rf)
{
    /* 2g 外部fem */
    /* RF0 */
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].c_lna_bypass_gain_db =
        (int8_t)cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_BYPASS_GAIN_DB_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].c_lna_gain_db =
        (int8_t)cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_GAIN_DB_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b0_db =
        (int8_t)cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B0_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b1_db =
        (int8_t)cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B1_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].uc_pa_gain_lvl_num =
        (int8_t)cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_LVL_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].uc_ext_switch_isexist =
        (uint8_t) !!cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].uc_ext_pa_isexist =
        (uint8_t) !!cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_EXT_PA_ISEXIST_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].uc_ext_lna_isexist =
        (uint8_t)cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_EXT_LNA_ISEXIST_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].us_lna_on2off_time_ns =
        (uint16_t)cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].us_lna_off2on_time_ns =
        (uint16_t)cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_2G));
}

OAL_STATIC void hwifi_cfg_front_end_2g_rf1_fem(mac_cfg_customize_rf *pst_customize_rf)
{
    /* RF1 */
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_lna_bypass_gain_db =
        (int8_t)cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_BYPASS_GAIN_DB_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_lna_gain_db =
        (int8_t)cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_GAIN_DB_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b0_db =
        (int8_t)cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B0_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b1_db =
        (int8_t)cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B1_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].uc_pa_gain_lvl_num =
        (int8_t)cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_LVL_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].uc_ext_switch_isexist =
        (uint8_t) !!cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].uc_ext_pa_isexist =
        (uint8_t) !!cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_EXT_PA_ISEXIST_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].uc_ext_lna_isexist =
        (uint8_t)cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_EXT_LNA_ISEXIST_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].us_lna_on2off_time_ns =
        (uint16_t)cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].us_lna_off2on_time_ns =
        (uint16_t)cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_2G));
}

OAL_STATIC uint32_t hwifi_cfg_front_end_2g_fem(mac_cfg_customize_rf *pst_customize_rf)
{
    uint8_t uc_idx;

    /* RF0 */
    hwifi_cfg_front_end_2g_rf0_fem(pst_customize_rf);
    /* RF1 */
    hwifi_cfg_front_end_2g_rf1_fem(pst_customize_rf);

    if (hwifi_cfg_front_end_value_range_check(pst_customize_rf, WLAN_BAND_2G, RF_LINE_TXRX_GAIN_DB_2G_MIN)) {
        /* 值超出有效范围 */
        oam_error_log4(0, OAM_SF_CFG,
            "{hwifi_cfg_front_end_2g_fem:2g gain db out of range! rf0 lna_bypass[%d] pa_b0[%d] lna gain[%d] pa_b1[%d]}",
            pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_lna_bypass_gain_db,
            pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b0_db,
            pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_lna_gain_db,
            pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b1_db);
        return OAL_FAIL;
    }
    /* 2g定制化RF部分PA偏置寄存器  */
    for (uc_idx = 0; uc_idx < CUS_RF_PA_BIAS_REG_NUM; uc_idx++) {
        pst_customize_rf->aul_2g_pa_bias_rf_reg[uc_idx] =
        (uint32_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_TX2G_PA_GATE_VCTL_REG236 + uc_idx);
    }
    return OAL_SUCC;
}


OAL_STATIC uint32_t hwifi_cfg_front_end_adjustment_range_check(int8_t c_delta_cca_ed_high_20th_2g,
    int8_t c_delta_cca_ed_high_40th_2g, int8_t c_delta_cca_ed_high_20th_5g,
    int8_t c_delta_cca_ed_high_40th_5g, int8_t c_delta_cca_ed_high_80th_5g)
{
    return (cus_delta_cca_ed_high_th_out_of_range(c_delta_cca_ed_high_20th_2g) ||
            cus_delta_cca_ed_high_th_out_of_range(c_delta_cca_ed_high_40th_2g) ||
            cus_delta_cca_ed_high_th_out_of_range(c_delta_cca_ed_high_20th_5g) ||
            cus_delta_cca_ed_high_th_out_of_range(c_delta_cca_ed_high_40th_5g) ||
            cus_delta_cca_ed_high_th_out_of_range(c_delta_cca_ed_high_80th_5g));
}

OAL_STATIC void hwifi_cfg_front_end_5g_rf0_fem(mac_cfg_customize_rf *pst_customize_rf)
{
    /* 5g 外部fem */
    /* RF0 */
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_lna_bypass_gain_db =
        (int8_t)cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_BYPASS_GAIN_DB_5G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_lna_gain_db =
        (int8_t)cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_GAIN_DB_5G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b0_db =
        (int8_t)cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B0_5G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b1_db =
        (int8_t)cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B1_5G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].uc_pa_gain_lvl_num =
        (int8_t)cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_LVL_5G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].uc_ext_switch_isexist =
        (uint8_t) !!cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_5G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].uc_ext_pa_isexist =
        (uint8_t) !!(cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_EXT_PA_ISEXIST_5G)) &
        EXT_PA_ISEXIST_5G_MASK);
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].en_fem_lp_enable =
        (oal_fem_lp_state_enum_uint8)((cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI,
            WLAN_CFG_INIT_EXT_PA_ISEXIST_5G)) & EXT_FEM_LP_STATUS_MASK) >> EXT_FEM_LP_STATUS_OFFSET);
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_fem_spec_value =
        (int8_t)((cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI,
            WLAN_CFG_INIT_EXT_PA_ISEXIST_5G)) & EXT_FEM_FEM_SPEC_MASK) >> EXT_FEM_FEM_SPEC_OFFSET);
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].uc_ext_lna_isexist =
        (uint8_t)cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_EXT_LNA_ISEXIST_5G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].us_lna_on2off_time_ns =
        (uint16_t)cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_5G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].us_lna_off2on_time_ns =
        (uint16_t)cus_get_low_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_5G));
}
OAL_STATIC void hwifi_cfg_front_end_5g_rf1_fem(mac_cfg_customize_rf *pst_customize_rf)
{
    /* 5g 外部fem */
    /* RF1 */
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_lna_bypass_gain_db =
        (int8_t)cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_BYPASS_GAIN_DB_5G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_lna_gain_db =
        (int8_t)cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_GAIN_DB_5G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b0_db =
        (int8_t)cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B0_5G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b1_db =
        (int8_t)cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B1_5G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].uc_pa_gain_lvl_num =
        (int8_t)cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_LVL_5G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].uc_ext_switch_isexist =
        (uint8_t) !!cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_5G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].uc_ext_pa_isexist =
        (uint8_t) !!(cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_EXT_PA_ISEXIST_5G)) &
        EXT_PA_ISEXIST_5G_MASK);
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].en_fem_lp_enable =
        (oal_fem_lp_state_enum_uint8)((cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI,
            WLAN_CFG_INIT_EXT_PA_ISEXIST_5G)) & EXT_FEM_LP_STATUS_MASK) >> EXT_FEM_LP_STATUS_OFFSET);
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_fem_spec_value =
        (int8_t)((cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_EXT_PA_ISEXIST_5G)) &
        EXT_FEM_FEM_SPEC_MASK) >> EXT_FEM_FEM_SPEC_OFFSET);
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].uc_ext_lna_isexist =
        (uint8_t)cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_EXT_LNA_ISEXIST_5G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].us_lna_on2off_time_ns =
        (uint16_t)cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_5G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].us_lna_off2on_time_ns =
        (uint16_t)cus_get_high_16bits(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_5G));
}

OAL_STATIC uint32_t hwifi_cfg_front_end_5g_fem(mac_cfg_customize_rf *pst_customize_rf)
{
    uint8_t uc_idx;
    /* RF0 */
    hwifi_cfg_front_end_5g_rf0_fem(pst_customize_rf);
    /* RF1 */
    hwifi_cfg_front_end_5g_rf1_fem(pst_customize_rf);

    /* 5g upc mix_bf_gain_ctl for P10 */
    for (uc_idx = 0; uc_idx < MAC_NUM_5G_BAND; uc_idx++) {
        pst_customize_rf->aul_5g_upc_mix_gain_rf_reg[uc_idx] =
            (uint32_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_TX5G_UPC_MIX_GAIN_CTRL_1 + uc_idx);
    }

    if (hwifi_cfg_front_end_value_range_check(pst_customize_rf, WLAN_BAND_5G, RF_LINE_TXRX_GAIN_DB_5G_MIN)) {
        /* 值超出有效范围 */
        oam_error_log4(0, OAM_SF_CFG,
            "{hwifi_cfg_front_end_5g_fem:2g gain db out of range! rf0 lna_bypass[%d] pa_b0[%d] lna gain[%d] pa_b1[%d]}",
            pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_lna_bypass_gain_db,
            pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b0_db,
            pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_lna_gain_db,
            pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b1_db);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/* 配置: cca能量门限调整值 */
OAL_STATIC void hwifi_set_cca_energy_thrsehold(mac_cfg_customize_rf *pst_customize_rf)
{
    int8_t c_delta_cca_ed_high_20th_2g =
        (int8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_2G);
    int8_t c_delta_cca_ed_high_40th_2g =
        (int8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_2G);
    int8_t c_delta_cca_ed_high_20th_5g =
        (int8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_5G);
    int8_t c_delta_cca_ed_high_40th_5g =
        (int8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_5G);
    int8_t c_delta_cca_ed_high_80th_5g =
        (int8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_80TH_5G);
    /* 检查每一项的调整幅度是否超出最大限制 */
    if (hwifi_cfg_front_end_adjustment_range_check(c_delta_cca_ed_high_20th_2g, c_delta_cca_ed_high_40th_2g,
                                                   c_delta_cca_ed_high_20th_5g, c_delta_cca_ed_high_40th_5g,
                                                   c_delta_cca_ed_high_80th_5g)) {
        oam_error_log4(0, OAM_SF_ANY,
            "{hwifi_set_cca_energy_thrsehold::one or more delta cca ed high threshold out of range \
            [delta_20th_2g=%d, delta_40th_2g=%d, delta_20th_5g=%d, delta_40th_5g=%d], please check the value!}",
            c_delta_cca_ed_high_20th_2g, c_delta_cca_ed_high_40th_2g,
            c_delta_cca_ed_high_20th_5g, c_delta_cca_ed_high_40th_5g);
        /* set 0 */
        pst_customize_rf->c_delta_cca_ed_high_20th_2g = 0;
        pst_customize_rf->c_delta_cca_ed_high_40th_2g = 0;
        pst_customize_rf->c_delta_cca_ed_high_20th_5g = 0;
        pst_customize_rf->c_delta_cca_ed_high_40th_5g = 0;
        pst_customize_rf->c_delta_cca_ed_high_80th_5g = 0;
    } else {
        pst_customize_rf->c_delta_cca_ed_high_20th_2g = c_delta_cca_ed_high_20th_2g;
        pst_customize_rf->c_delta_cca_ed_high_40th_2g = c_delta_cca_ed_high_40th_2g;
        pst_customize_rf->c_delta_cca_ed_high_20th_5g = c_delta_cca_ed_high_20th_5g;
        pst_customize_rf->c_delta_cca_ed_high_40th_5g = c_delta_cca_ed_high_40th_5g;
        pst_customize_rf->c_delta_cca_ed_high_80th_5g = c_delta_cca_ed_high_80th_5g;
    }
}


static int8_t hwifi_check_pwr_ref_delta(int8_t c_pwr_ref_delta)
{
    int8_t c_ret = 0;
    if (c_pwr_ref_delta > WAL_HIPRIV_PWR_REF_DELTA_HI) {
        c_ret = WAL_HIPRIV_PWR_REF_DELTA_HI;
    } else if (c_pwr_ref_delta < WAL_HIPRIV_PWR_REF_DELTA_LO) {
        c_ret = WAL_HIPRIV_PWR_REF_DELTA_LO;
    } else {
        c_ret = c_pwr_ref_delta;
    }

    return c_ret;
}


static void hwifi_cfg_pwr_ref_delta(mac_cfg_customize_rf *pst_customize_rf)
{
    uint8_t uc_rf_idx;
    wlan_cfg_init cfg_id;
    int32_t l_pwr_ref_delta;
    mac_cfg_custom_delta_pwr_ref_stru *pst_delta_pwr_ref = NULL;
    mac_cfg_custom_amend_rssi_stru *pst_rssi_amend_ref = NULL;
    int8_t *rssi = NULL;

    for (uc_rf_idx = 0; uc_rf_idx < WLAN_RF_CHANNEL_NUMS; uc_rf_idx++) {
        pst_delta_pwr_ref = &pst_customize_rf->ast_delta_pwr_ref_cfg[uc_rf_idx];
        /* 2G 20M/40M */
        cfg_id = (uc_rf_idx == WLAN_RF_CHANNEL_ZERO) ?
            WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C0_MULT4 : WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C1_MULT4;
        l_pwr_ref_delta = hwifi_get_init_value(CUS_TAG_INI, cfg_id);
        rssi = pst_delta_pwr_ref->c_cfg_delta_pwr_ref_rssi_2g;
        rssi[0] = hwifi_check_pwr_ref_delta((int8_t)cus_get_first_byte(l_pwr_ref_delta));
        rssi[1] = hwifi_check_pwr_ref_delta((int8_t)cus_get_second_byte(l_pwr_ref_delta));
        /* 5G 20M/40M/80M/160M */
        cfg_id = (uc_rf_idx == WLAN_RF_CHANNEL_ZERO) ?
            WLAN_CFG_INIT_RF_PWR_REF_RSSI_5G_C0_MULT4 : WLAN_CFG_INIT_RF_PWR_REF_RSSI_5G_C1_MULT4;
        l_pwr_ref_delta = hwifi_get_init_value(CUS_TAG_INI, cfg_id);
        rssi = pst_delta_pwr_ref->c_cfg_delta_pwr_ref_rssi_5g;
        rssi[0] = hwifi_check_pwr_ref_delta((int8_t)cus_get_first_byte(l_pwr_ref_delta));
        rssi[1] = hwifi_check_pwr_ref_delta((int8_t)cus_get_second_byte(l_pwr_ref_delta));
        rssi[BYTE_OFFSET_2] = hwifi_check_pwr_ref_delta((int8_t)cus_get_third_byte(l_pwr_ref_delta));
        rssi[BYTE_OFFSET_3] = hwifi_check_pwr_ref_delta((int8_t)cus_get_fourth_byte(l_pwr_ref_delta));

        /* RSSI amend */
        pst_rssi_amend_ref = &pst_customize_rf->ast_rssi_amend_cfg[uc_rf_idx];
        cfg_id = (uc_rf_idx == WLAN_RF_CHANNEL_ZERO) ?
            WLAN_CFG_INIT_RF_AMEND_RSSI_2G_C0 : WLAN_CFG_INIT_RF_AMEND_RSSI_2G_C1;
        l_pwr_ref_delta = hwifi_get_init_value(CUS_TAG_INI, cfg_id);
        rssi = pst_rssi_amend_ref->ac_cfg_delta_amend_rssi_2g;
        rssi[0] = cus_val_valid((int8_t)cus_get_first_byte(l_pwr_ref_delta), WLAN_RF_RSSI_AMEND_TH_HIGH,
            WLAN_RF_RSSI_AMEND_TH_LOW) ? (int8_t)cus_get_first_byte(l_pwr_ref_delta) : 0;
        rssi[1] = cus_val_valid((int8_t)cus_get_second_byte(l_pwr_ref_delta), WLAN_RF_RSSI_AMEND_TH_HIGH,
            WLAN_RF_RSSI_AMEND_TH_LOW) ? (int8_t)cus_get_second_byte(l_pwr_ref_delta) : 0;
        rssi[BYTE_OFFSET_2] = cus_val_valid((int8_t)cus_get_third_byte(l_pwr_ref_delta), WLAN_RF_RSSI_AMEND_TH_HIGH,
            WLAN_RF_RSSI_AMEND_TH_LOW) ? (int8_t)cus_get_third_byte(l_pwr_ref_delta) : 0;
        cfg_id = (uc_rf_idx == WLAN_RF_CHANNEL_ZERO) ?
            WLAN_CFG_INIT_RF_AMEND_RSSI_5G_C0 : WLAN_CFG_INIT_RF_AMEND_RSSI_5G_C1;
        l_pwr_ref_delta = hwifi_get_init_value(CUS_TAG_INI, cfg_id);
        rssi = pst_rssi_amend_ref->ac_cfg_delta_amend_rssi_5g;
        rssi[0] = cus_val_valid((int8_t)cus_get_first_byte(l_pwr_ref_delta), WLAN_RF_RSSI_AMEND_TH_HIGH,
            WLAN_RF_RSSI_AMEND_TH_LOW) ? (int8_t)cus_get_first_byte(l_pwr_ref_delta) : 0;
        rssi[1] = cus_val_valid((int8_t)cus_get_second_byte(l_pwr_ref_delta), WLAN_RF_RSSI_AMEND_TH_HIGH,
            WLAN_RF_RSSI_AMEND_TH_LOW) ? (int8_t)cus_get_second_byte(l_pwr_ref_delta) : 0;
        rssi[BYTE_OFFSET_2] = cus_val_valid((int8_t)cus_get_third_byte(l_pwr_ref_delta), WLAN_RF_RSSI_AMEND_TH_HIGH,
            WLAN_RF_RSSI_AMEND_TH_LOW) ? (int8_t)cus_get_third_byte(l_pwr_ref_delta) : 0;
        rssi[BYTE_OFFSET_3] = cus_val_valid((int8_t)cus_get_fourth_byte(l_pwr_ref_delta), WLAN_RF_RSSI_AMEND_TH_HIGH,
            WLAN_RF_RSSI_AMEND_TH_LOW) ? (int8_t)cus_get_fourth_byte(l_pwr_ref_delta) : 0;
    }
}


static int8_t hwifi_get_valid_amend_rssi_val(int8_t pwr_ref_delta)
{
    int8_t rssi_amend_val;
    rssi_amend_val = cus_val_valid(pwr_ref_delta, WLAN_RF_RSSI_AMEND_TH_HIGH, WLAN_RF_RSSI_AMEND_TH_LOW) ?
        pwr_ref_delta : 0;
    return rssi_amend_val;
}


static void hwifi_cfg_filter_narrow_ref_delta(mac_cfg_customize_rf *pst_customize_rf)
{
    uint8_t uc_rf_idx;
    wlan_cfg_init cfg_id;
    int32_t l_pwr_ref_delta;
    mac_cfg_custom_filter_narrow_amend_rssi_stru *filter_narrow_rssi_amend = NULL;

    for (uc_rf_idx = 0; uc_rf_idx < WLAN_RF_CHANNEL_NUMS; uc_rf_idx++) {
        filter_narrow_rssi_amend = &pst_customize_rf->filter_narrow_rssi_amend[uc_rf_idx];
        cfg_id = (uc_rf_idx == WLAN_RF_CHANNEL_ZERO) ? WLAN_CFG_INIT_RF_FILTER_NARROW_RSSI_AMEND_2G_C0 :
            WLAN_CFG_INIT_RF_FILTER_NARROW_RSSI_AMEND_2G_C1;
        l_pwr_ref_delta = hwifi_get_init_value(CUS_TAG_INI, cfg_id);
        filter_narrow_rssi_amend->filter_narrowing_amend_rssi_2g[0] =
            hwifi_get_valid_amend_rssi_val((int8_t)cus_get_first_byte(l_pwr_ref_delta));
        filter_narrow_rssi_amend->filter_narrowing_amend_rssi_2g[1] =
            hwifi_get_valid_amend_rssi_val((int8_t)cus_get_second_byte(l_pwr_ref_delta));
        cfg_id = (uc_rf_idx == WLAN_RF_CHANNEL_ZERO) ? WLAN_CFG_INIT_RF_FILTER_NARROW_RSSI_AMEND_5G_C0 :
            WLAN_CFG_INIT_RF_FILTER_NARROW_RSSI_AMEND_5G_C1;
        l_pwr_ref_delta = hwifi_get_init_value(CUS_TAG_INI, cfg_id);
        filter_narrow_rssi_amend->filter_narrowing_amend_rssi_5g[0] =
            hwifi_get_valid_amend_rssi_val((int8_t)cus_get_first_byte(l_pwr_ref_delta));
        filter_narrow_rssi_amend->filter_narrowing_amend_rssi_5g[1] =
            hwifi_get_valid_amend_rssi_val((int8_t)cus_get_second_byte(l_pwr_ref_delta));
        filter_narrow_rssi_amend->filter_narrowing_amend_rssi_5g[2] = /* rssi修正5G phy mode160M 数组下标2 */
            hwifi_get_valid_amend_rssi_val((int8_t)cus_get_third_byte(l_pwr_ref_delta));
    }
}

OAL_STATIC uint32_t hwifi_cfg_front_end(uint8_t *puc_param)
{
    mac_cfg_customize_rf *pst_customize_rf;

    pst_customize_rf = (mac_cfg_customize_rf *)puc_param;
    memset_s(pst_customize_rf, sizeof(mac_cfg_customize_rf), 0, sizeof(mac_cfg_customize_rf));

    /* 配置: 2g rf */
    if (hwifi_cfg_front_end_set_2g_rf(pst_customize_rf) != OAL_SUCC) {
        return OAL_FAIL;
    }

    hwifi_cfg_pwr_ref_delta(pst_customize_rf);
    hwifi_cfg_filter_narrow_ref_delta(pst_customize_rf);

    /* 通道radio cap */
    pst_customize_rf->uc_chn_radio_cap = (uint8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_CHANN_RADIO_CAP);

    /* 2g 外部fem */
    if (hwifi_cfg_front_end_2g_fem(pst_customize_rf) != OAL_SUCC) {
        return OAL_FAIL;
    }

    if (OAL_TRUE == mac_device_check_5g_enable_per_chip()) {
        /* 配置: 5g rf */
        /* 配置: fem口到天线口的负增益 */
        if (hwifi_cfg_front_end_set_5g_rf(pst_customize_rf) != OAL_SUCC) {
            return OAL_FAIL;
        }
        /* 5g 外部fem */
        /* RF0 */
        if (hwifi_cfg_front_end_5g_fem(pst_customize_rf) != OAL_SUCC) {
            return OAL_FAIL;
        }
    }

    pst_customize_rf->uc_far_dist_pow_gain_switch =
        (uint8_t) !!hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_FAR_DIST_POW_GAIN_SWITCH);
    pst_customize_rf->uc_far_dist_dsss_scale_promote_switch =
        (uint8_t) !!hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_FAR_DIST_DSSS_SCALE_PROMOTE_SWITCH);

    /* 配置: cca能量门限调整值 */
    hwifi_set_cca_energy_thrsehold(pst_customize_rf);

    return OAL_SUCC;
}


OAL_STATIC void hwifi_config_init_ini_rf(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    uint16_t us_event_len = sizeof(mac_cfg_customize_rf);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_RF, us_event_len);

    /*lint -e774*/
    /* 定制化下发不能超过事件内存长 */
    if (us_event_len > WAL_MSG_WRITE_MAX_LEN) {
        oam_error_log2(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::event size[%d] larger than msg size[%d]!}",
                       us_event_len, WAL_MSG_WRITE_MAX_LEN);
        return;
    }
    /*lint +e774*/
    /*  */
    ret = hwifi_cfg_front_end(st_write_msg.auc_value);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::front end rf wrong value, not send cfg!}");
        return;
    }

    /* 如果所有参数都在有效范围内，则下发配置值 */
    ret = (uint32_t)wal_send_cfg_event(pst_cfg_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + \
        us_event_len, (uint8_t *)&st_write_msg,  OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::EVENT[wal_send_cfg_event] fail[%d]!}", ret);
    }
}



OAL_STATIC void hwifi_config_init_ini_log(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    int32_t l_loglevel;

    /* log_level */
    l_loglevel = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LOGLEVEL);
    if (l_loglevel < OAM_LOG_LEVEL_ERROR ||
        l_loglevel > OAM_LOG_LEVEL_INFO) {
        oam_error_log3(0, OAM_SF_ANY, "{hwifi_config_init_ini_clock::loglevel[%d] out of range[%d,%d], check ini file}",
                       l_loglevel, OAM_LOG_LEVEL_ERROR, OAM_LOG_LEVEL_INFO);
        return;
    }

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_ALL_LOG_LEVEL, sizeof(int32_t));
    *((int32_t *)(st_write_msg.auc_value)) = l_loglevel;
    l_ret = wal_send_cfg_event(pst_cfg_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{hwifi_config_init_ini_log::return err code[%d]!}\r\n", l_ret);
    }
}


OAL_STATIC void hwifi_config_init_ini_main_1103(oal_net_device_stru *pst_cfg_net_dev)
{
    /* 国家码 */
    hwifi_config_init_ini_country(pst_cfg_net_dev);
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
    /* 自学习国家码初始化 */
    hwifi_config_selfstudy_init_country(pst_cfg_net_dev);
#endif
    /* 可维可测 */
    hwifi_config_init_ini_log(pst_cfg_net_dev);
    /* RF */
    hwifi_config_init_ini_rf(pst_cfg_net_dev);
}

void wal_send_cali_data_1103(oal_net_device_stru *cfg_net_dev)
{
    hmac_send_cali_data_1103(oal_net_dev_priv(cfg_net_dev), WLAN_20M_ALL_CHN);
}

void wal_send_cali_data_1105(oal_net_device_stru *cfg_net_dev)
{
    hmac_send_cali_data_1105(oal_net_dev_priv(cfg_net_dev), WLAN_20M_ALL_CHN);
}


uint32_t wal_custom_cali_1103(void)
{
    oal_net_device_stru *pst_net_dev;
    uint32_t ret;

    pst_net_dev = wal_config_get_netdev("Hisilicon0", OAL_STRLEN("Hisilicon0"));  // 通过cfg vap0来下c0 c1校准
    if (oal_warn_on(pst_net_dev == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    } else {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(pst_net_dev);
        oam_warning_log0(0, OAM_SF_ANY, "{wal_custom_cali::the net_device is already exist!}");
    }

    if (hwifi_config_init_nvram_main_1103(pst_net_dev)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_custom_cali::init_nvram fail!}");
    }

    hwifi_config_init_ini_main_1103(pst_net_dev);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (g_custom_cali_done == OAL_TRUE) {
        /* 校准数据下发 */
        wlan_chip_send_cali_data(pst_net_dev);
    } else {
        g_custom_cali_done = OAL_TRUE;
    }

    wal_send_cali_matrix_data(pst_net_dev);
#endif

    /* 下发参数 */
    ret = hwifi_config_init_dts_main(pst_net_dev);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_custom_cali:init_dts_main fail!}");
    }

    return ret;
}

static uint32_t hwifi_cfg_init_dts_cus_cali_2g_txpwr_pa_dc_ref(mac_cus_dts_cali_stru *pst_cus_cali)
{
    uint32_t uc_idx;
    int32_t l_val;
    int16_t s_ref_val_ch1, s_ref_val_ch0;

    /* 2G REF: 分13个信道 */
    for (uc_idx = 0; uc_idx < 13; uc_idx++) {
        l_val = hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_START + uc_idx);
        s_ref_val_ch1 = (int16_t)cus_get_high_16bits(l_val);
        s_ref_val_ch0 = (int16_t)cus_get_low_16bits(l_val);
        /* 2G判断参考值先不判断<0, 待与RF同事确认, TBD */
        if (s_ref_val_ch0 <= CALI_TXPWR_PA_DC_REF_MAX) {
            pst_cus_cali->ast_cali[0].aus_cali_txpwr_pa_dc_ref_2g_val_chan[uc_idx] = s_ref_val_ch0;
        } else {
            /* 值超出有效范围 */
            oam_error_log2(0, OAM_SF_ANY, "{hwifi_cfg_init_dts_cus_cali::dts 2g ref id[%d]value[%d] out of range!}",
                           WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_START + uc_idx, s_ref_val_ch0);  //lint !e571
            return OAL_FAIL;
        }
        /* 02不需要适配双通道 */
        if (s_ref_val_ch1 <= CALI_TXPWR_PA_DC_REF_MAX) {
            pst_cus_cali->ast_cali[1].aus_cali_txpwr_pa_dc_ref_2g_val_chan[uc_idx] = s_ref_val_ch1;
        } else {
            /* 值超出有效范围 */
            oam_error_log2(0, OAM_SF_ANY, "{hwifi_cfg_init_dts_cus_cali::dts ch1 2g ref id[%d]value[%d] invalid!}",
                           WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_START + uc_idx, s_ref_val_ch1);  //lint !e571
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

static uint32_t hwifi_cfg_init_dts_cus_cali_5g_txpwr_pa_dc_ref(mac_cus_dts_cali_stru *pst_cus_cali)
{
    uint32_t uc_idx;
    int32_t l_val;
    int16_t s_ref_val_ch1, s_ref_val_ch0;

    /* 5G REF: 分7个band */
    for (uc_idx = 0; uc_idx < 7; ++uc_idx) {
        l_val = hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_START + uc_idx);
        s_ref_val_ch1 = (int16_t)cus_get_high_16bits(l_val);
        s_ref_val_ch0 = (int16_t)cus_get_low_16bits(l_val);
        if (s_ref_val_ch0 >= 0 && s_ref_val_ch0 <= CALI_TXPWR_PA_DC_REF_MAX) {
            pst_cus_cali->ast_cali[0].aus_cali_txpwr_pa_dc_ref_5g_val_band[uc_idx] = s_ref_val_ch0;
        } else {
            /* 值超出有效范围 */
            oam_error_log2(0, OAM_SF_ANY, "{hwifi_cfg_init_dts_cus_cali::dts 5g ref id[%d]val[%d] invalid}",
                WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_START + uc_idx, s_ref_val_ch0);  //lint !e571
            return OAL_FAIL;
        }
        if (s_ref_val_ch1 >= 0 && s_ref_val_ch1 <= CALI_TXPWR_PA_DC_REF_MAX) {
            pst_cus_cali->ast_cali[1].aus_cali_txpwr_pa_dc_ref_5g_val_band[uc_idx] = s_ref_val_ch1;
        } else {
            /* 值超出有效范围 */
            oam_error_log2(0, OAM_SF_ANY, "{hwifi_cfg_init_dts_cus_cali::dts ch1 5g ref id[%d]val[%d] invalid!}",
                WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_START + uc_idx, s_ref_val_ch1);  //lint !e571
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}



OAL_STATIC uint32_t hwifi_cfg_init_dts_cus_cali(mac_cus_dts_cali_stru *pst_cus_cali, uint8_t uc_5g_band_enable)
{
    int32_t l_val;
    uint8_t uc_idx; /* 结构体数组下标 */
    uint8_t uc_gm_opt;

    /** 配置: TXPWR_PA_DC_REF **/
    if (hwifi_cfg_init_dts_cus_cali_2g_txpwr_pa_dc_ref(pst_cus_cali) != OAL_SUCC) {
        return OAL_FAIL;
    }

    if (uc_5g_band_enable) {
        if (hwifi_cfg_init_dts_cus_cali_5g_txpwr_pa_dc_ref(pst_cus_cali) != OAL_SUCC) {
            return OAL_FAIL;
        }
    }

    /* 配置BAND 5G ENABLE */
    pst_cus_cali->uc_band_5g_enable = !!uc_5g_band_enable;

    /* 配置单音幅度档位 */
    pst_cus_cali->uc_tone_amp_grade =
        (uint8_t)hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_CALI_TONE_AMP_GRADE);

    /* 配置DPD校准参数 */
    for (uc_idx = 0; uc_idx < MAC_DPD_CALI_CUS_PARAMS_NUM; uc_idx++) {
        /* 通道0 */
        l_val = hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_DPD_CALI_START + uc_idx);
        pst_cus_cali->ast_dpd_cali_para[0].aul_dpd_cali_cus_dts[uc_idx] = l_val;
        /* 通道1 */
        l_val = hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_DPD_CALI_START +
            uc_idx + MAC_DPD_CALI_CUS_PARAMS_NUM);
        pst_cus_cali->ast_dpd_cali_para[1].aul_dpd_cali_cus_dts[uc_idx] = l_val;
    }

    /* 配置动态校准参数 */
    l_val = hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_DYN_CALI_DSCR_ITERVL);
    pst_cus_cali->aus_dyn_cali_dscr_interval[WLAN_BAND_2G] = (uint16_t)((uint32_t)l_val & 0x0000FFFF);

    if (uc_5g_band_enable) {
        pst_cus_cali->aus_dyn_cali_dscr_interval[WLAN_BAND_5G] =
            (uint16_t)(((uint32_t)l_val & 0xFFFF0000) >> BIT_OFFSET_16);
    }

    l_val = hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_DYN_CALI_OPT_SWITCH);
    uc_gm_opt = ((uint32_t)l_val & BIT2) >> NUM_1_BITS;

    if (((uint32_t)l_val & 0x3) >> 1) {
        /* 自适应选择 */
        l_val = !g_en_fact_cali_completed;
    } else {
        l_val = (int32_t)((uint32_t)l_val & BIT0);
    }

    pst_cus_cali->en_dyn_cali_opt_switch = (uint32_t)l_val | uc_gm_opt;

    l_val = hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_DYN_CALI_GM0_DB10_AMEND);
    pst_cus_cali->gm0_db10_amend[WLAN_RF_CHANNEL_ZERO] = (int16_t)cus_get_low_16bits(l_val);
    pst_cus_cali->gm0_db10_amend[WLAN_RF_CHANNEL_ONE] = (int16_t)cus_get_high_16bits(l_val);

    return OAL_SUCC;
}


OAL_STATIC uint32_t hwifi_config_init_dts_cali(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    mac_cus_dts_cali_stru st_cus_cali;
    uint32_t offset = 0;
    oal_bool_enum en_5g_band_enable; /* mac device是否支持5g能力 */

    if (oal_warn_on(pst_cfg_net_dev == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hwifi_config_init_dts_cali::pst_cfg_net_dev is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(&st_cus_cali, sizeof(mac_cus_dts_cali_stru), 0, sizeof(mac_cus_dts_cali_stru));
    /* 检查硬件是否需要使能5g */
    en_5g_band_enable = mac_device_check_5g_enable_per_chip();

    /* 配置校准参数TXPWR_PA_DC_REF */
    ret = hwifi_cfg_init_dts_cus_cali(&st_cus_cali, en_5g_band_enable);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_CFG, "{hwifi_config_init_dts_cali::init dts cus cali failed ret[%d]!}", ret);
        return ret;
    }

    /* 如果所有参数都在有效范围内，则下发配置值 */
    if (EOK != memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                        (int8_t *)&st_cus_cali, sizeof(mac_cus_dts_cali_stru))) {
        oam_error_log0(0, OAM_SF_CFG, "hwifi_config_init_dts_cali::memcpy fail!");
        return OAL_FAIL;
    }
    offset += sizeof(mac_cus_dts_cali_stru);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_DTS_CALI, offset);
    ret = wal_send_cfg_event(pst_cfg_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + offset,
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{hwifi_config_init_dts_cali::wal_send_cfg_event failed, ret[%d]!}", ret);
        return ret;
    }

    oam_warning_log0(0, OAM_SF_CFG, "{hwifi_config_init_dts_cali::wal_send_cfg_event send succ}");

    return OAL_SUCC;
}


OAL_STATIC uint32_t hwifi_cfg_init_cus_5g_160m_dpn_cali(mac_cus_dy_cali_param_stru *dyn_cali_param,
    int8_t *pc_ctx, uint8_t rf_idx)
{
    uint8_t  uc_dpn_5g_nv_id = WLAN_CFG_DTS_NVRAM_MUFREQ_5G160_C0;
    uint8_t  nv_pa_params[CUS_PARAMS_LEN_MAX] = { 0 };
    int8_t   dpn_5g_nv[OAL_5G_160M_CHANNEL_NUM];
    uint8_t *pc_end = ";";
    uint8_t *pc_sep = ",";
    int8_t  *pc_token = NULL;
    uint8_t *cust_nvram_info = NULL;
    uint8_t  num_idx;
    int32_t  val;

    cust_nvram_info = hwifi_get_nvram_param(uc_dpn_5g_nv_id);
    uc_dpn_5g_nv_id++;
    if (OAL_STRLEN(cust_nvram_info)) {
        memset_s(nv_pa_params, sizeof(nv_pa_params), 0, sizeof(nv_pa_params));
        if (memcpy_s(nv_pa_params, sizeof(nv_pa_params),
                     cust_nvram_info, OAL_STRLEN(cust_nvram_info)) != EOK) {
            oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_cfg_init_cus_5g_160m_dpn_cali::memcpy fail!");
        }
        pc_token = oal_strtok(nv_pa_params, pc_end, &pc_ctx);
        pc_token = oal_strtok(pc_token, pc_sep, &pc_ctx);
        num_idx = 0;
        while ((pc_token != NULL)) {
            if (num_idx >= OAL_5G_160M_CHANNEL_NUM) {
                num_idx++;
                break;
            }
            val = oal_strtol(pc_token, NULL, 10) / 10; /* 10表示十进制 */
            pc_token = oal_strtok(NULL, pc_sep, &pc_ctx);
            if (oal_value_not_in_valid_range(val, CUS_DY_CALI_5G_VAL_DPN_MIN, CUS_DY_CALI_5G_VAL_DPN_MAX)) {
                oam_error_log3(0, OAM_SF_CUSTOM, "{hwifi_cfg_init_cus_dyn_cali::nvram 2g dpn val[%d]\
                    unexpect:idx[%d] num_idx[%d}}", val, MAC_NUM_5G_BAND, num_idx);
                val = 0;
            }
            dpn_5g_nv[num_idx] = (int8_t)val;
            num_idx++;
        }

        if (num_idx != OAL_5G_160M_CHANNEL_NUM) {
            oam_error_log2(0, OAM_SF_CUSTOM,
                "{hwifi_cfg_init_cus_dyn_cali::nvram 2g dpn num unexpected id[%d] rf[%d}}", MAC_NUM_5G_BAND, rf_idx);
            return OAL_FAIL;
        }
        /* 5250  5570 */
        for (num_idx = 0; num_idx < OAL_5G_160M_CHANNEL_NUM; num_idx++) {
            dyn_cali_param->ac_dy_cali_5g_dpn_params[num_idx + 1][WLAN_BW_CAP_160M] += dpn_5g_nv[num_idx];
        }
    }
    return OAL_SUCC;
}

OAL_STATIC void hwifi_cfg_init_cus_dpn_cali(mac_cus_dy_cali_param_stru *dyn_cali_param, int8_t *pc_ctx, uint8_t rf_idx)
{
    uint8_t  idx, num_idx;
    int32_t  val;
    uint8_t *cust_nvram_info = NULL;
    uint8_t  dpn_2g_nv_id = WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C0;
    uint8_t  nv_pa_params[CUS_PARAMS_LEN_MAX] = { 0 };
    int8_t   ac_dpn_nv[HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_CW][MAC_2G_CHANNEL_NUM];
    uint8_t *pc_end = ";";
    uint8_t *pc_sep = ",";
    int8_t  *pc_token = NULL;

    for (idx = HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_11B;
        idx <= HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_OFDM40; idx++) {
        /* 获取产线计算DPN值修正 */
        cust_nvram_info = hwifi_get_nvram_param(dpn_2g_nv_id);
        dpn_2g_nv_id++;

        if (0 == OAL_STRLEN(cust_nvram_info)) {
            continue;
        }

        memset_s(nv_pa_params, sizeof(nv_pa_params), 0, sizeof(nv_pa_params));
        if (memcpy_s(nv_pa_params, sizeof(nv_pa_params), cust_nvram_info, OAL_STRLEN(cust_nvram_info)) != EOK) {
            oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_cfg_init_cus_dpn_cali::memcpy fail!");
        }
        pc_token = oal_strtok(nv_pa_params, pc_end, &pc_ctx);
        pc_token = oal_strtok(pc_token, pc_sep, &pc_ctx);
        num_idx = 0;
        while ((pc_token != NULL)) {
            if (num_idx >= MAC_2G_CHANNEL_NUM) {
                num_idx++;
                break;
            }
            val = oal_strtol(pc_token, NULL, 10) / 10; /* 10表示十进制 */
            pc_token = oal_strtok(NULL, pc_sep, &pc_ctx);
            if (oal_value_not_in_valid_range(val, CUS_DY_CALI_2G_VAL_DPN_MIN, CUS_DY_CALI_2G_VAL_DPN_MAX)) {
                oam_error_log3(0, OAM_SF_CUSTOM, "{hwifi_cfg_init_cus_dyn_cali::nvram 2g dpn val[%d]\
                    unexpected idx[%d] num_idx[%d}!}", val, idx, num_idx);
                val = 0;
            }
            ac_dpn_nv[idx][num_idx] = (int8_t)val;
            num_idx++;
        }

        if (num_idx != MAC_2G_CHANNEL_NUM) {
            oam_error_log2(0, OAM_SF_CUSTOM,
                "{hwifi_cfg_init_cus_dyn_cali::nvram 2g dpn num is unexpect uc_id[%d] rf[%d}}", idx, rf_idx);
            continue;
        }

        for (num_idx = 0; num_idx < MAC_2G_CHANNEL_NUM; num_idx++) {
            dyn_cali_param->ac_dy_cali_2g_dpn_params[num_idx][idx] += ac_dpn_nv[idx][num_idx];
        }
    }
}


OAL_STATIC uint32_t hwifi_cfg_init_cus_dyn_cali(mac_cus_dy_cali_param_stru *puc_dyn_cali_param, int num)
{
    int32_t  val, ret;
    uint8_t  uc_idx = 0;
    uint8_t  uc_rf_idx, uc_dy_cal_param_idx;
    uint8_t  uc_cfg_id = WLAN_CFG_DTS_2G_CORE0_DPN_CH1;
    int8_t  *pc_ctx = NULL;

    for (uc_rf_idx = 0; uc_rf_idx < num; uc_rf_idx++) {
        puc_dyn_cali_param->uc_rf_id = uc_rf_idx;

        /* 动态校准二次项系数入参检查 */
        for (uc_dy_cal_param_idx = 0; uc_dy_cal_param_idx < DY_CALI_PARAMS_NUM; uc_dy_cal_param_idx++) {
            if (!g_pro_line_params[uc_rf_idx][uc_dy_cal_param_idx].l_pow_par2) {
                oam_error_log1(0, OAM_SF_CUSTOM, "{hwifi_cfg_init_cus_dyn_cali::unexpect val[%d] s_pow_par2[0]!}",
                    uc_dy_cal_param_idx);
                return OAL_FAIL;
            }
        }
        ret = memcpy_s(puc_dyn_cali_param->al_dy_cali_base_ratio_params,
                       sizeof(puc_dyn_cali_param->al_dy_cali_base_ratio_params),
                       g_pro_line_params[uc_rf_idx], sizeof(puc_dyn_cali_param->al_dy_cali_base_ratio_params));

        ret += memcpy_s(puc_dyn_cali_param->al_dy_cali_base_ratio_ppa_params,
                        sizeof(puc_dyn_cali_param->al_dy_cali_base_ratio_ppa_params),
                        &g_pro_line_params[uc_rf_idx][CUS_DY_CALI_PARAMS_NUM],
                        sizeof(puc_dyn_cali_param->al_dy_cali_base_ratio_ppa_params));

        ret += memcpy_s(puc_dyn_cali_param->as_extre_point_val, sizeof(puc_dyn_cali_param->as_extre_point_val),
                        g_gs_extre_point_vals[uc_rf_idx], sizeof(puc_dyn_cali_param->as_extre_point_val));

        /* DPN */
        for (uc_idx = 0; uc_idx < MAC_2G_CHANNEL_NUM; uc_idx++) {
            val = hwifi_get_init_value(CUS_TAG_DTS, uc_cfg_id + uc_idx);
            ret += memcpy_s(puc_dyn_cali_param->ac_dy_cali_2g_dpn_params[uc_idx],
                CUS_DY_CALI_DPN_PARAMS_NUM * sizeof(int8_t), &val, CUS_DY_CALI_DPN_PARAMS_NUM * sizeof(int8_t));
        }
        uc_cfg_id += MAC_2G_CHANNEL_NUM;
        hwifi_cfg_init_cus_dpn_cali(puc_dyn_cali_param, pc_ctx, uc_rf_idx);

        for (uc_idx = 0; uc_idx < MAC_NUM_5G_BAND; uc_idx++) {
            val = hwifi_get_init_value(CUS_TAG_DTS, uc_cfg_id + uc_idx);
            ret += memcpy_s(puc_dyn_cali_param->ac_dy_cali_5g_dpn_params[uc_idx],
                CUS_DY_CALI_DPN_PARAMS_NUM * sizeof(int8_t), &val, CUS_DY_CALI_DPN_PARAMS_NUM * sizeof(int8_t));
        }
        uc_cfg_id += MAC_NUM_5G_BAND;

        /* 5G 160M DPN */
        if (hwifi_cfg_init_cus_5g_160m_dpn_cali(puc_dyn_cali_param, pc_ctx, uc_rf_idx) != OAL_SUCC) {
            continue;
        }
        puc_dyn_cali_param++;
    }
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_cfg_init_cus_dyn_cali::memcpy fail!");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


OAL_STATIC void hwifi_config_init_cus_dyn_cali(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    uint32_t offset = 0;
    mac_cus_dy_cali_param_stru st_dy_cus_cali[WLAN_RF_CHANNEL_NUMS];
    uint8_t uc_rf_id;
    mac_cus_dy_cali_param_stru *pst_dy_cus_cali = NULL;
    wal_msg_stru *pst_rsp_msg = NULL;
    wal_msg_write_rsp_stru *pst_write_rsp_msg = NULL;
    int32_t l_ret;

    if (oal_warn_on(pst_cfg_net_dev == NULL)) {
        return;
    }

    /* 配置动态校准参数TXPWR_PA_DC_REF */
    memset_s(st_dy_cus_cali, sizeof(mac_cus_dy_cali_param_stru) * WLAN_RF_CHANNEL_NUMS,
             0, sizeof(mac_cus_dy_cali_param_stru) * WLAN_RF_CHANNEL_NUMS);

    ret = hwifi_cfg_init_cus_dyn_cali(st_dy_cus_cali, WLAN_RF_CHANNEL_NUMS);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return;
    }

    for (uc_rf_id = 0; uc_rf_id < WLAN_RF_CHANNEL_NUMS; uc_rf_id++) {
        pst_dy_cus_cali = &st_dy_cus_cali[uc_rf_id];
        pst_rsp_msg = NULL;

        /* 如果所有参数都在有效范围内，则下发配置值 */
        l_ret = memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                         (int8_t *)pst_dy_cus_cali, sizeof(mac_cus_dy_cali_param_stru));
        if (l_ret != EOK) {
            oam_error_log0(0, OAM_SF_CFG, "hwifi_config_init_cus_dyn_cali::memcpy fail!");
            return;
        }

        offset = sizeof(mac_cus_dy_cali_param_stru);
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_DYN_CALI_PARAM, offset);

        ret = wal_send_cfg_event(pst_cfg_net_dev, WAL_MSG_TYPE_WRITE,
            WAL_MSG_WRITE_MSG_HDR_LENGTH + offset, (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
        if (oal_unlikely(ret != OAL_SUCC)) {
            return;
        }

        if (pst_rsp_msg != NULL) {
            pst_write_rsp_msg = (wal_msg_write_rsp_stru *)(pst_rsp_msg->auc_msg_data);
            if (pst_write_rsp_msg->err_code != OAL_SUCC) {
                oam_error_log2(0, OAM_SF_SCAN, "{wal_check_and_release_msg_resp::detect err code:[%u],wid:[%u]}",
                               pst_write_rsp_msg->err_code, pst_write_rsp_msg->en_wid);
                oal_free(pst_rsp_msg);
                return;
            }

            oal_free(pst_rsp_msg);
        }
    }

    return;
}

static uint32_t hwifi_config_init_dts_main(oal_net_device_stru *cfg_net_dev)
{
    uint32_t ret = OAL_SUCC;

    /* 下发动态校准参数 */
    hwifi_config_init_cus_dyn_cali(cfg_net_dev);

    /* 校准 */
    if (OAL_SUCC != hwifi_config_init_dts_cali(cfg_net_dev)) {
        return OAL_FAIL;
    }
    /* 校准放到第一个进行 */
    return ret;
}


uint32_t hwifi_get_sar_ctrl_params_1103(uint8_t lvl_num, uint8_t *data_addr,
    uint16_t *data_len, uint16_t dest_len)
{
    *data_len = sizeof(wlan_cust_sar_ctrl_stru) * CUS_NUM_OF_SAR_PARAMS;
    if ((lvl_num <= CUS_NUM_OF_SAR_LVL) && (lvl_num > 0)) {
        lvl_num--;
        if (EOK != memcpy_s(data_addr, dest_len, g_cust_nv_params.st_sar_ctrl_params[lvl_num], *data_len)) {
            oam_error_log0(0, OAM_SF_CFG, "hwifi_get_sar_ctrl_params_1103::memcpy fail!");
            return OAL_FAIL;
        }
    } else {
        memset_s(data_addr, dest_len, 0xFF, dest_len);
    }
    return OAL_SUCC;
}
