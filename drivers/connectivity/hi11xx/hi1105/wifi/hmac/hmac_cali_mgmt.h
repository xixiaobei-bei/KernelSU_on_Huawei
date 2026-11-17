

#ifndef __HMAC_CALI_MGMT_H__
#define __HMAC_CALI_MGMT_H__

/* 1 其他头文件包含 */
#include "frw_ext_if.h"

#include "hmac_vap.h"
#include "plat_cali.h"
#include "wlan_spec.h"
#include "wlan_cali.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CALI_MGMT_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/* 2 宏定义 */
#define DPD_CORRAM_DATA_NUM                512
#define DPD_CORRAM_DATA_LEN                1024
#define HI1103_DPD_OFFLINE_CALI_BW_NUM     2
#define HI1103_2G_DPD_CALI_CHANNEL_NUM_20M 4
#define HI1103_2G_DPD_CALI_CHANNEL_NUM_40M 3
#define HI1103_2G_DPD_CALI_CHANNEL_NUM     (HI1103_2G_DPD_CALI_CHANNEL_NUM_20M + HI1103_2G_DPD_CALI_CHANNEL_NUM_40M)

#define HI1103_DPD_CALI_TPC_LEVEL_NUM 2
#define HI1103_DPD_CALI_LUT_LENGTH    31

#define HI1103_DPD_OFFLINE_CALI_LUT_NUM (HI1103_2G_DPD_CALI_CHANNEL_NUM * HI1103_DPD_CALI_TPC_LEVEL_NUM)

#define HI1103_CALI_SISO_TXDC_GAIN_LVL_NUM      8 /* SISO tx dc补偿值档位数目 */
#define HI1103_CALI_MIMO_TXDC_GAIN_LVL_NUM      8 /* MIMO tx dc补偿值档位数目 */
#define HI1103_CALI_TXDC_GAIN_LVL_NUM           (HI1103_CALI_SISO_TXDC_GAIN_LVL_NUM +  \
                                                 HI1103_CALI_MIMO_TXDC_GAIN_LVL_NUM)
#define HI1103_CALI_IQ_TONE_NUM                 16
#define HI1103_CALI_RXDC_GAIN_LVL_NUM           8 /* rx dc补偿值档位数目 */
#define HI1103_CALI_ADC_CH_NUM                  4 /* 4路ADC */
#define HI1103_CALI_2G_OTHER_CHANNEL_NUM        1
#define HI1103_2G_CHANNEL_NUM                   3
#define HI1103_5G_20M_CHANNEL_NUM               7
#define HI1103_5G_80M_CHANNEL_NUM               7
#define HI1103_5G_160M_CHANNEL_NUM              2
#define HI1103_5G_CHANNEL_NUM                   (HI1103_5G_20M_CHANNEL_NUM +  \
                                                 HI1103_5G_80M_CHANNEL_NUM +  \
                                                 HI1103_5G_160M_CHANNEL_NUM)
#define HI1103_CALI_RXIQ_LS_FILTER_TAP_NUM_160M 17
#define HI1103_CALI_TXIQ_LS_FILTER_TAP_NUM      15
#define HI1103_CALI_RXIQ_LS_FILTER_TAP_NUM_320M (HI1103_CALI_RXIQ_LS_FILTER_TAP_NUM_160M)

#define HI1103_CALI_RXIQ_LS_FILTER_TAP_NUM_80M 15

#define HI1103_RXIQ_LSFIR_FEQ_NUM_320M 128
#define HI1103_CALI_RXIQ_LS_FILTER_FEQ_NUM_160M 64
#define HI1103_CALI_RXIQ_LS_FILTER_FEQ_NUM_80M  32

#define HI1103_CALI_TXIQ_LS_FILTER_FEQ_NUM_640M 256
#define HI1103_CALI_TXIQ_LS_FILTER_FEQ_NUM_320M 128
#define HI1103_CALI_TXIQ_LS_FILTER_FEQ_NUM_160M 64

#define NUM_80M_SNGL_TONE_1_4_CIRCLE  16 /* DAC 80M时1/4周期1.25M单音点采样点个数 */
#define NUM_160M_SNGL_TONE_1_4_CIRCLE 16 /* DAC 80M时1/4周期2.5M单音点采样点个数 */
#define NUM_320M_SNGL_TONE_1_4_CIRCLE 32 /* DAC 160M时1/4周期2.5M单音点采样点个数 */
#define NUM_640M_SNGL_TONE_1_4_CIRCLE 64 /* DAC 320M时1/4周期2.5M单音点采样点个数 */

#define NUM_80M_SNGL_TONE_1_CIRCLE 2 /* DAC 80M时1/4周期1.25M单音点采样周期，1/4周期为一个单位 */
#define NUM_BW_SNGL_TONE_1_CIRCLE 4 /* DAC 不同带宽单音点采样一个周期，1/4周期为一个单位 */

#define HI1103_CALI_MATRIX_DATA_NUMS  23
#define HI1106_CALI_MATRIX_DATA_NUMS  1

#define HMAC_NETBUF_OFFSET  104

/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
typedef enum {
    HI1103_CALI_SISO,
    HI1103_CALI_MIMO,

    HI1103_CALI_CHAIN_NUM_BUTT,
} hi1103_rf_cali_chain_num_enum;

typedef struct {
    uint8_t uc_rx_gain_cmp_code; /* 仅pilot RF使用，C校准补偿值 */

    /* LODIV 暂时和rx gain复用结构 */
    uint8_t uc_rx_mimo_cmp;
    uint8_t uc_dpd_siso_cmp;
    uint8_t uc_dpd_mimo_cmp;
} hi1103_rx_gain_comp_val_stru;

typedef struct {
    uint16_t aus_analog_rxdc_siso_cmp[HI1103_CALI_RXDC_GAIN_LVL_NUM];
    uint16_t aus_analog_rxdc_mimo_extlna_cmp[HI1103_CALI_RXDC_GAIN_LVL_NUM];
    uint16_t aus_analog_rxdc_mimo_intlna_cmp[HI1103_CALI_RXDC_GAIN_LVL_NUM];
    uint16_t us_digital_rxdc_cmp_i;
    uint16_t us_digital_rxdc_cmp_q;
    int16_t s_cali_temperature;
    int16_t s_mimo_cali_temperature;
} hi1103_rx_dc_comp_val_stru;

typedef struct {
    uint8_t uc_ppa_cmp;
    uint8_t uc_atx_pwr_cmp;
    uint8_t uc_dtx_pwr_cmp;
    int8_t c_dp_init;
    int16_t s_2g_tx_ppa_dc;
    int16_t s_2g_tx_power_dc;
} hi1103_2g_tx_power_comp_val_stru;

typedef struct {
    uint8_t uc_ppa_cmp;
    uint8_t uc_mx_cmp;
    uint8_t uc_atx_pwr_cmp;
    uint8_t uc_dtx_pwr_cmp;
    int16_t s_5g_tx_power_dc;
    uint8_t auc_reserve[2]; // 2代表2个保留字节，需要4字节对齐
} hi1103_5g_tx_power_comp_val_stru;

typedef struct {
    uint8_t uc_ssb_cmp;
    uint8_t uc_buf0_cmp;
    uint8_t uc_buf1_cmp;
    uint8_t uc_resv;
} hi1103_logen_comp_val_stru;

typedef struct {
    uint8_t uc_classa_cmp;
    uint8_t uc_classb_cmp;
    uint8_t auc_reserve[2]; // 2代表2个保留字节，需要4字节对齐
} hi1103_pa_ical_val_stru;

typedef struct {
    uint16_t us_txdc_cmp_i;
    uint16_t us_txdc_cmp_q;
} hi1103_txdc_comp_val_stru;

typedef struct {
    uint8_t uc_ppf_val;
    uint8_t auc_reserve[3]; // 3代表3个保留字节，需要4字节对齐
} hi1103_ppf_comp_val_stru;

typedef struct {
    uint16_t us_txiq_cmp_p;
    uint16_t us_txiq_cmp_e;
} hi1103_txiq_comp_val_stru;

typedef struct {
    int32_t l_txiq_cmp_alpha_p;
    int32_t l_txiq_cmp_beta_p;
    int32_t l_txiq_cmp_alpha_n;
    int32_t l_txiq_cmp_beta_n;
} hi1103_new_txiq_comp_val_stru;

typedef struct {
    int32_t l_txiq_cmp_alpha;
    int32_t l_txiq_cmp_beta;
    int16_t as_txiq_comp_ls_filt[HI1103_CALI_TXIQ_LS_FILTER_TAP_NUM];
    uint8_t auc_resev[2]; // 2代表2个保留字节，需要4字节对齐
} hi1103_new_txiq_time_comp_val_stru;

typedef struct {
    int32_t l_rxiq_cmp_alpha;
    int32_t l_rxiq_cmp_beta;
    int16_t as_rxiq_comp_ls_filt[HI1103_CALI_RXIQ_LS_FILTER_TAP_NUM_160M];
    int16_t as_rxiq_bw80m_siso_comp_ls_filt[HI1103_CALI_RXIQ_LS_FILTER_TAP_NUM_160M];
} hi1103_new_rxiq_comp_val_stru;

typedef struct {
    /* 乘2为IQ两路数据 */
    uint8_t uc_cmp_cali[HI1103_CALI_ADC_CH_NUM << BIT_OFFSET_1];
    uint32_t cap_cali_w1[HI1103_CALI_ADC_CH_NUM << BIT_OFFSET_1];
    uint32_t cap_cali_w2[HI1103_CALI_ADC_CH_NUM << BIT_OFFSET_1];
    uint32_t cap_cali_w3[HI1103_CALI_ADC_CH_NUM << BIT_OFFSET_1];
    uint32_t cap_cali_w4[HI1103_CALI_ADC_CH_NUM << BIT_OFFSET_1];
    uint32_t cap_cali_w5[HI1103_CALI_ADC_CH_NUM << BIT_OFFSET_1];
    int32_t l_moec[HI1103_CALI_ADC_CH_NUM << BIT_OFFSET_1];
    uint16_t us_mgec[HI1103_CALI_ADC_CH_NUM << BIT_OFFSET_1];
} hi1103_adc_cali_param_stru;

typedef struct {
    hi1103_rx_dc_comp_val_stru st_cali_rx_dc_cmp[HI1103_CALI_2G_OTHER_CHANNEL_NUM];
    hi1103_rx_gain_comp_val_stru st_cali_rx_gain_cmp[HI1103_CALI_2G_OTHER_CHANNEL_NUM];
    hi1103_logen_comp_val_stru st_cali_logen_cmp[HI1103_2G_CHANNEL_NUM];
    hi1103_2g_tx_power_comp_val_stru st_cali_tx_power_cmp_2g[HI1103_2G_CHANNEL_NUM];
    hi1103_txdc_comp_val_stru ast_txdc_cmp_val[HI1103_2G_CHANNEL_NUM][HI1103_CALI_TXDC_GAIN_LVL_NUM];
#ifdef _PRE_WLAN_NEW_IQ
    hi1103_new_txiq_time_comp_val_stru ast_new_txiq_time_cmp_val[HI1103_2G_CHANNEL_NUM];
    hi1103_new_rxiq_comp_val_stru ast_new_rxiq_cmp_val[HI1103_2G_CHANNEL_NUM];
#endif
} hi1103_2gcali_param_stru;

typedef struct {
    hi1103_rx_dc_comp_val_stru st_cali_rx_dc_cmp[HI1103_CALI_2G_OTHER_CHANNEL_NUM];
    hi1103_txdc_comp_val_stru ast_txdc_cmp_val[HI1103_2G_CHANNEL_NUM][HI1103_CALI_TXDC_GAIN_LVL_NUM];
#ifdef _PRE_WLAN_NEW_IQ
    hi1103_new_txiq_time_comp_val_stru ast_new_txiq_time_cmp_val[HI1103_2G_CHANNEL_NUM];
    hi1103_new_rxiq_comp_val_stru ast_new_rxiq_cmp_val[HI1103_2G_CHANNEL_NUM];
#endif
} hi1103_2g_dbdc_cali_param_stru;

typedef struct {
    hi1103_rx_dc_comp_val_stru st_cali_rx_dc_cmp;
    hi1103_rx_gain_comp_val_stru st_cali_rx_gain_cmp;
    hi1103_logen_comp_val_stru st_cali_logen_cmp;
    hi1103_5g_tx_power_comp_val_stru st_cali_tx_power_cmp_5g;
    hi1103_ppf_comp_val_stru st_ppf_cmp_val;
    hi1103_txdc_comp_val_stru ast_txdc_cmp_val[HI1103_CALI_TXDC_GAIN_LVL_NUM];
#ifdef _PRE_WLAN_NEW_IQ
    hi1103_new_txiq_time_comp_val_stru ast_new_txiq_time_cmp_val[HI1103_CALI_CHAIN_NUM_BUTT];
    hi1103_new_rxiq_comp_val_stru st_new_rxiq_cmp_val;
#endif
} hi1103_5gcali_param_stru;

typedef struct {
    hi1103_2gcali_param_stru st_2gcali_param[HI1103_2G_CHANNEL_NUM];
    hi1103_5gcali_param_stru st_5gcali_param[HI1103_5G_CHANNEL_NUM];
} hi1103_wifi_cali_param_stru;

typedef struct {
    uint16_t cali_time;
    uint16_t bit_temperature : 3,
               uc_5g_chan_idx1 : 5,
               uc_5g_chan_idx2 : 5,
               bit_rev : 3;
} hi1103_update_cali_channel_stru;

typedef struct {
    uint8_t uc_rc_cmp_code;
    uint8_t uc_r_cmp_code;
    /* 重要: MPW2和PILOT RF公用, mpw2代表c校准值; pilot代表800M rejection补偿code，根据C code计算得到 */
    uint8_t uc_c_cmp_code;
    uint8_t uc_20m_rc_cmp_code;
} hi1103_rc_r_c_cali_param_stru;

#ifdef _PRE_WLAN_ONLINE_DPD
typedef struct {
    uint32_t aul_dpd_even_lut[HI1103_DPD_OFFLINE_CALI_LUT_NUM][HI1103_DPD_CALI_LUT_LENGTH];
    uint32_t aul_dpd_odd_lut[HI1103_DPD_OFFLINE_CALI_LUT_NUM][HI1103_DPD_CALI_LUT_LENGTH];
} hi1103_dpd_cali_lut_stru;
#endif

typedef struct {
    uint32_t dog_tag;
    hi1103_2gcali_param_stru ast_2gcali_param;
    hi1103_5gcali_param_stru ast_5gcali_param[HI1103_5G_CHANNEL_NUM];
#ifdef _PRE_WLAN_ONLINE_DPD
    hi1103_dpd_cali_lut_stru st_dpd_cali_data;
#endif
    hi1103_ppf_comp_val_stru st_165chan_ppf_comp;
    hi1103_update_cali_channel_stru st_cali_update_info;
    uint32_t check_hw_status;
    hi1103_pa_ical_val_stru st_pa_ical_cmp;
    hi1103_rc_r_c_cali_param_stru st_rc_r_c_cali_data;
    hi1103_2g_dbdc_cali_param_stru ast_2g_dbdc_cali_param;
    int16_t s_2g_idet_gm;
    oal_bool_enum_uint8 en_use_lpf_10m_for_2grx_20m;
    oal_bool_enum_uint8 en_save_all;
    uint8_t uc_last_cali_fail_status;
    oal_bool_enum_uint8 en_need_close_fem_cali;
    uint8_t auc_resv[2]; // 2代表2个保留字节，需要4字节对齐
    hi1103_adc_cali_param_stru st_adc_cali_data;
} hi1103_cali_param_stru;

// added for bt 20dbm cali
typedef struct {
    uint8_t uc_atx_pwr_cmp;
    uint8_t uc_dtx_pwr_cmp;
    int16_t s_2g_tx_power_dc;
} hi1103_bt20dbm_txpwr_cal_stru;
typedef struct {
    int32_t l_txiq_cmp_alpha;
    int32_t l_txiq_cmp_beta;
    hi1103_bt20dbm_txpwr_cal_stru st_20dbmtxpwr;
    hi1103_txdc_comp_val_stru ast_txdc_cmp_val[8];  // 8代表8个档位，wifi siso tx档位
} hi1103_bt20dbm_txcal_val_stru;
typedef struct {
    hi1103_bt20dbm_txcal_val_stru st_bt20dbm_txcali_param[HI1103_2G_CHANNEL_NUM];
} hi1103_bt20dbm_txcal_param_stru;

typedef struct {
    uint16_t us_atx_pwr_cmp;
    int16_t s_2g_tx_power_dc;
    uint8_t uc_dtx_pwr_cmp;
    uint8_t auc_rsv[3]; // 3代表3个保留字节，需要4字节对齐
} hi1105_bt20dbm_txpwr_cal_stru;
typedef struct {
    int32_t l_txiq_cmp_alpha[HI1105_CALI_TXIQ_GAIN_LVL_NUM];
    int32_t l_txiq_cmp_beta[HI1105_CALI_TXIQ_GAIN_LVL_NUM];
    hi1105_bt20dbm_txpwr_cal_stru st_20dbmtxpwr;
    hi1105_txdc_comp_val_stru st_txdc_cmp_val;
} hi1105_bt20dbm_txcal_val_stru;
typedef struct {
    hi1105_bt20dbm_txcal_val_stru st_bt20dbm_txcali_param[HI1103_2G_CHANNEL_NUM];
} hi1105_bt20dbm_txcal_param_stru;

typedef struct {
    uint16_t us_atx_pwr_cmp;
    int16_t s_2g_tx_power_dc;
    uint8_t uc_dtx_pwr_cmp;
    uint8_t rsv1;
    uint16_t rsv2;
} hi1106_bt20dbm_txpwr_cal_stru;
typedef struct {
    hi1106_bt20dbm_txpwr_cal_stru st_20dbmtxpwr;
    wlan_cali_txdc_para_stru st_txdc_cmp_val;
} hi1106_bt20dbm_txcal_val_stru;
typedef struct {
    hi1106_bt20dbm_txcal_val_stru st_bt20dbm_txcali_param[HI1103_2G_CHANNEL_NUM];
} hi1106_bt20dbm_txcal_param_stru;

// end added of 20dbm cali
uint32_t hmac_save_cali_event(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_send_cali_data(mac_vap_stru *mac_vap);
uint32_t hmac_send_cali_matrix_data(mac_vap_stru *mac_vap);
void hmac_get_cali_data_for_bt20dbm(void);
void hmac_get_1105_cali_data_for_bt20dbm(void);
void hmac_get_1106_cali_data_for_bt20dbm(void);
void hmac_set_close_fem_cali_status(oal_bool_enum_uint8 en_switch);
/* 校准区分芯片回调函数 */
uint32_t hmac_send_cali_matrix_data_1103(mac_vap_stru *mac_vap);
uint32_t hmac_send_cali_matrix_data_1105(mac_vap_stru *mac_vap);
uint32_t hmac_send_cali_matrix_data_1106(mac_vap_stru *mac_vap);
uint32_t hmac_send_cali_data_1103(mac_vap_stru *mac_vap, wlan_h2d_cali_data_type_enum_uint8 en_type);
uint32_t hmac_send_cali_data_1105(mac_vap_stru *mac_vap, wlan_h2d_cali_data_type_enum_uint8 en_type);
uint32_t hmac_send_cali_data_1106(mac_vap_stru *mac_vap, wlan_h2d_cali_data_type_enum_uint8 en_type);
uint32_t hmac_save_cali_event_1103(frw_event_mem_stru *event_mem);
uint32_t hmac_save_cali_event_1105(frw_event_mem_stru *event_mem);
uint32_t hmac_save_cali_event_1106(frw_event_mem_stru *event_mem);
void hmac_set_20m_cali_data(void);
void hmac_update_cur_chn_cali_data_1106(mac_vap_stru *mac_vap, wlan_h2d_cali_data_type_enum_uint8 data_type);
int32_t hmac_cali_ops_init(void);
int8_t hmac_get_wlan_first_power_on_cali_flag(void);
void hmac_set_wlan_first_powon_mark(oal_bool_enum_uint8 status);
oal_completion *hmac_get_wlan_cali_completed_addr(void);
void hmac_first_powon_cali_completed(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_mgmt_classifier.h */


