

#ifndef __WLAN_CALI_H__
#define __WLAN_CALI_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

//#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_HOST)
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"

/*****************************************************************************
  2 宏定义
*************************************************************************/
#ifdef _PRE_WLAN_ONLINE_DPD
#define HI1105_2G_DPD_CALI_CHANNEL_NUM_20M (4)
#define HI1105_2G_DPD_CALI_CHANNEL_NUM_40M (3)
#define HI1105_2G_DPD_CALI_CHANNEL_NUM (HI1105_2G_DPD_CALI_CHANNEL_NUM_20M+HI1105_2G_DPD_CALI_CHANNEL_NUM_40M)
#define HI1105_DPD_CALI_TPC_LEVEL_NUM (2)
#define HI1105_DPD_CALI_LUT_LENGTH (31)
#define HI1105_DPD_OFFLINE_CALI_LUT_NUM (HI1105_2G_DPD_CALI_CHANNEL_NUM * HI1105_DPD_CALI_TPC_LEVEL_NUM)
#endif

#define HI1105_2G_CHANNEL_NUM         (3)

#define HI1105_5G_20M_CHANNEL_NUM     (7)
#define HI1105_5G_80M_CHANNEL_NUM     (7)
#define HI1105_5G_160M_CHANNEL_NUM    (2)
#define HI1105_5G_CHANNEL_NUM         (HI1105_5G_20M_CHANNEL_NUM + HI1105_5G_80M_CHANNEL_NUM + HI1105_5G_160M_CHANNEL_NUM)
#define HI1105_CALI_IQ_TONE_NUM       (16)

#define HI1105_CALI_RXDC_GAIN_LVL_NUM (8)            /* rx dc补偿值档位数目 */

#define HI1105_TXDC_LPF_DAC_GAIN_LVL_NUM 8
#define HI1105_TXDC_MIXBUF_GAIN_LVL_NUM 4

#define HI1105_CALI_ADC_CH_NUM    4            /* 4路ADC */

#define HI1105_CALI_TXIQ_LS_FIR_NUM     7
#define HI1105_CALI_TXIQ_GAIN_LVL_NUM   4

#define HI1105_CALI_RXIQ_LS_FIR_NUM     7
#define HI1105_CALI_RXIQ_GAIN_LVL_NUM   3

/* 校准信道个数定义*/
#define HI1105_CALI_2G_OTHER_CHANNEL_NUM        1    /* g_ast_2g_other_cali_channel校准信道数 */
#define HI1105_CALI_5G_OTHER_CHANNEL_NUM        1    /* g_ast_5g_other_cali_channel校准信道数 */

#define HI1105_CALI_IQ_LS_FILTER_TAP_NUM     7

#define HI1105_CALI_RXIQ_LS_FILTER_FEQ_NUM_320M    64
#define HI1105_CALI_RXIQ_LS_FILTER_FEQ_NUM_160M    32
#define HI1105_CALI_RXIQ_LS_FILTER_FEQ_NUM_80M     16
#define HI1105_CALI_RXIQ_LS_FILTER_FEQ_NUM_40M     16
#define HI1105_CALI_RXIQ_LS_FILTER_FEQ_NUM_80M_FPGA  32

#define HI1105_CALI_TXIQ_LS_FILTER_FEQ_NUM_640M    128
#define HI1105_CALI_TXIQ_LS_FILTER_FEQ_NUM_320M    64
#define HI1105_CALI_TXIQ_LS_FILTER_FEQ_NUM_160M    32
#define HI1105_CALI_TXIQ_LS_FILTER_FEQ_NUM_80M     32
#define HI1105_CALI_TXIQ_LS_FILTER_FEQ_NUM_160M_FPGA    64

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum
{
    HI1105_CALI_SISO,
    HI1105_CALI_MIMO,

    HI1105_CALI_CHAIN_NUM_BUTT,
}hi1105_rf_cali_chain_num_enum;
typedef oal_uint8 hi1105_rf_cali_chain_num_enum_uint8;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

/* 复数结构 */
typedef struct
{
    oal_int32 l_real;
    oal_int32 l_imag;
}hi1105_complex_stru;

typedef struct
{
    oal_uint8   uc_rx_gain_cmp_code;     /* 仅pilot RF使用，C校准补偿值 */

    /* LODIV 暂时和rx gain复用结构 */
    oal_uint8   uc_rx_mimo_cmp;
    oal_uint8   uc_dpd_siso_cmp;
    oal_uint8   uc_dpd_mimo_cmp;
}hi1105_rx_gain_comp_val_stru;

typedef struct
{
    oal_uint16  aus_analog_rxdc_siso_cmp[HI1105_CALI_RXDC_GAIN_LVL_NUM];
    oal_uint16  aus_analog_rxdc_mimo_extlna_cmp[HI1105_CALI_RXDC_GAIN_LVL_NUM];
    oal_uint16  aus_analog_rxdc_mimo_intlna_cmp[HI1105_CALI_RXDC_GAIN_LVL_NUM];
    oal_uint16  us_digital_rxdc_cmp_i;
    oal_uint16  us_digital_rxdc_cmp_q;
    oal_int16   s_cali_temperature;
    oal_int16   s_mimo_cali_temperature;
}hi1105_rx_dc_comp_val_stru;

typedef struct
{
    oal_uint8   uc_ppa_cmp;
    oal_uint8   uc_atx_pwr_cmp;
    oal_uint8   uc_dtx_pwr_cmp;
    oal_int8    c_dp_init;
    oal_int16   s_2g_tx_ppa_dc;
    oal_int16   s_2g_tx_power_dc;
}hi1105_2G_tx_power_comp_val_stru;

typedef struct
{
    oal_uint8   uc_ppa_cmp;
    oal_uint8   uc_mx_cmp;
    oal_uint8   uc_atx_pwr_cmp;
    oal_uint8   uc_dtx_pwr_cmp;
    oal_int16   s_5g_tx_power_dc;
    oal_uint8   auc_reserve[2];
}hi1105_5G_tx_power_comp_val_stru;

typedef struct
{
    oal_uint8   uc_ssb_cmp;
    oal_uint8   uc_buf0_cmp;
    oal_uint8   uc_buf1_cmp;
    oal_uint8   uc_resv;
}hi1105_logen_comp_val_stru;

typedef struct
{
    oal_uint8   uc_classa_cmp;
    oal_uint8   uc_classb_cmp;
    oal_uint8   auc_reserve[2];
}hi1105_pa_ical_val_stru;

typedef struct
{
    oal_uint16  us_txdc_cmp_i[HI1105_TXDC_MIXBUF_GAIN_LVL_NUM][HI1105_TXDC_LPF_DAC_GAIN_LVL_NUM];
    oal_uint16  us_txdc_cmp_q[HI1105_TXDC_MIXBUF_GAIN_LVL_NUM][HI1105_TXDC_LPF_DAC_GAIN_LVL_NUM];
}hi1105_txdc_comp_val_stru;

typedef struct
{
    oal_uint8   uc_ppf_val;
    oal_uint8   auc_reserve[3];
}hi1105_ppf_comp_val_stru;

typedef struct
{
    oal_int16  as_txiq_comp_ls_fir_i[HI1105_CALI_TXIQ_GAIN_LVL_NUM][HI1105_CALI_TXIQ_LS_FIR_NUM];
    oal_int16  as_txiq_comp_ls_fir_q[HI1105_CALI_TXIQ_GAIN_LVL_NUM][HI1105_CALI_TXIQ_LS_FIR_NUM];
}hi1105_new_txiq_comp_stru;

typedef struct
{
    oal_int32  l_rxiq_cmp_alpha[HI1105_CALI_RXIQ_GAIN_LVL_NUM];
    oal_int32  l_rxiq_cmp_beta[HI1105_CALI_RXIQ_GAIN_LVL_NUM];
    oal_int16  as_rxiq_comp_ls_fir_i[HI1105_CALI_RXIQ_GAIN_LVL_NUM][HI1105_CALI_RXIQ_LS_FIR_NUM];
    oal_int16  as_rxiq_comp_ls_fir_q[HI1105_CALI_RXIQ_GAIN_LVL_NUM][HI1105_CALI_RXIQ_LS_FIR_NUM];
}hi1105_new_rxiq_comp_stru;

typedef struct
{
    hi1105_rx_dc_comp_val_stru         ast_cali_rx_dc_cmp[HI1105_CALI_2G_OTHER_CHANNEL_NUM];
    hi1105_rx_gain_comp_val_stru       ast_cali_rx_gain_cmp[HI1105_CALI_2G_OTHER_CHANNEL_NUM];
    hi1105_logen_comp_val_stru         ast_cali_logen_cmp[HI1105_2G_CHANNEL_NUM];
    hi1105_2G_tx_power_comp_val_stru   ast_cali_tx_power_cmp_2G[HI1105_2G_CHANNEL_NUM];
    hi1105_txdc_comp_val_stru          ast_txdc_cmp_val[HI1105_2G_CHANNEL_NUM][HI1105_CALI_CHAIN_NUM_BUTT];
#ifdef _PRE_WLAN_NEW_IQ
    hi1105_new_txiq_comp_stru          ast_new_txiq_cmp[HI1105_2G_CHANNEL_NUM][HI1105_CALI_CHAIN_NUM_BUTT];
    hi1105_new_rxiq_comp_stru          ast_new_rxiq_cmp[HI1105_2G_CHANNEL_NUM][HI1105_CALI_CHAIN_NUM_BUTT];
#endif
}hi1105_2Gcali_param_stru;

typedef struct
{
    hi1105_rx_dc_comp_val_stru     ast_cali_rx_dc_cmp[HI1105_CALI_2G_OTHER_CHANNEL_NUM];
    //hi1105_logen_comp_val_stru         st_cali_logen_cmp;
    hi1105_txdc_comp_val_stru      ast_txdc_cmp_val[HI1105_2G_CHANNEL_NUM][HI1105_CALI_CHAIN_NUM_BUTT];
#ifdef _PRE_WLAN_NEW_IQ
    hi1105_new_txiq_comp_stru      ast_new_txiq_cmp[HI1105_2G_CHANNEL_NUM][HI1105_CALI_CHAIN_NUM_BUTT];
    hi1105_new_rxiq_comp_stru      ast_new_rxiq_cmp[HI1105_2G_CHANNEL_NUM][HI1105_CALI_CHAIN_NUM_BUTT];
#endif
}hi1105_2G_dbdc_cali_param_stru;

typedef struct
{
    hi1105_rx_dc_comp_val_stru         st_cali_rx_dc_cmp;
    hi1105_rx_gain_comp_val_stru       st_cali_rx_gain_cmp;
    hi1105_logen_comp_val_stru         st_cali_logen_cmp;
    hi1105_5G_tx_power_comp_val_stru   st_cali_tx_power_cmp_5G;
    hi1105_ppf_comp_val_stru           st_ppf_cmp_val;
    hi1105_txdc_comp_val_stru          ast_txdc_cmp_val[HI1105_CALI_CHAIN_NUM_BUTT];
#ifdef _PRE_WLAN_NEW_IQ
    hi1105_new_txiq_comp_stru          ast_new_txiq_cmp[HI1105_CALI_CHAIN_NUM_BUTT];
    hi1105_new_rxiq_comp_stru          ast_new_rxiq_cmp[HI1105_CALI_CHAIN_NUM_BUTT];
#endif
}hi1105_5Gcali_param_stru;

typedef struct
{
    hi1105_2Gcali_param_stru ast_2Gcali_param[HI1105_2G_CHANNEL_NUM];
    hi1105_5Gcali_param_stru ast_5Gcali_param[HI1105_5G_CHANNEL_NUM];
}hi1105_wifi_cali_param_stru;

typedef struct
{
    oal_uint16  us_cali_time;
    oal_uint16  bit_temperature     : 3,
                uc_5g_chan_idx1     : 5,
                uc_5g_chan_idx2     : 5,
                bit_rev             : 3;
}hi1105_update_cali_channel_stru;

typedef struct
{
    oal_uint8              uc_rc_cmp_code;
    oal_uint8              uc_r_cmp_code;       /* 保存PMU的原始5bit R code */
    oal_uint8              uc_c_cmp_code;       /* 重要: MPW2和PILOT RF公用, mpw2代表c校准值; pilot代表800M rejection补偿code，根据C code计算得到 */
    oal_uint8              uc_20M_rc_cmp_code;
}hi1105_rc_r_c_cali_param_stru;

#ifdef _PRE_WLAN_ONLINE_DPD
typedef struct
{
    oal_uint32 aul_dpd_even_lut[HI1105_DPD_OFFLINE_CALI_LUT_NUM][HI1105_DPD_CALI_LUT_LENGTH];
    oal_uint32 aul_dpd_odd_lut[HI1105_DPD_OFFLINE_CALI_LUT_NUM][HI1105_DPD_CALI_LUT_LENGTH];

}hi1105_dpd_cali_lut_stru;
#endif

struct hi1105_cali_param_tag
{
    oal_uint32                      ul_dog_tag;
    hi1105_2Gcali_param_stru        st_2Gcali_param;
    hi1105_5Gcali_param_stru        ast_5Gcali_param[HI1105_5G_CHANNEL_NUM];

#ifdef _PRE_WLAN_ONLINE_DPD
    hi1105_dpd_cali_lut_stru        st_dpd_cali_data;
#endif
    hi1105_ppf_comp_val_stru        st_165chan_ppf_comp;
    hi1105_update_cali_channel_stru st_cali_update_info;
    oal_uint32                      ul_check_hw_status;
    hi1105_pa_ical_val_stru         st_pa_ical_cmp;
    hi1105_rc_r_c_cali_param_stru   st_rc_r_c_cali_data;
    hi1105_2G_dbdc_cali_param_stru  st_2g_dbdc_cali_param;
    oal_int16                       s_2g_idet_gm;
    oal_bool_enum_uint8             en_use_lpf_10M_for_2grx_20m;
    oal_bool_enum_uint8             en_save_all;
    oal_uint8                       uc_last_cali_fail_status;
    oal_uint8                       auc_resv[3];
};

typedef struct hi1105_cali_param_tag hi1105_cali_param_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
//#endif /*  end of if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_HOST)  */
#endif /* end of hal_cali.h */
