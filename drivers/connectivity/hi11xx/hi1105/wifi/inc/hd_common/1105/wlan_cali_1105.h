

#ifndef __WLAN_CALI_1105_H__
#define __WLAN_CALI_1105_H__

/* 1 其他头文件包含 */
#include "oal_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/* 2 宏定义 */
#define HI1105_2G_DPD_CALI_CHANNEL_NUM_20M 4
#define HI1105_2G_DPD_CALI_CHANNEL_NUM_40M 3
#define HI1105_2G_DPD_CALI_CHANNEL_NUM     (HI1105_2G_DPD_CALI_CHANNEL_NUM_20M + HI1105_2G_DPD_CALI_CHANNEL_NUM_40M)
#define HI1105_DPD_CALI_TPC_LEVEL_NUM      3
#define HI1105_DPD_CALI_LUT_LENGTH         31
#define HI1105_DPD_OFFLINE_CALI_LUT_NUM    (HI1105_2G_DPD_CALI_CHANNEL_NUM * HI1105_DPD_CALI_TPC_LEVEL_NUM)

#define HI1105_2G_CHAN_NUM      (3)
#define HI1105_5G_20M_CHAN_NUM  (7)
#define HI1105_5G_80M_CHANNEL_NUM  (7)
#define HI1105_5G_160M_CHANNEL_NUM (2)
#define HI1105_5G_CHANNEL_NUM      (HI1105_5G_20M_CHAN_NUM + HI1105_5G_80M_CHANNEL_NUM + HI1105_5G_160M_CHANNEL_NUM)
#define HI1105_CALI_IQ_TONE_NUM    (16)

/* 软件定义补偿值数组数，宏可修改 */
#define HI1105_CALI_RXDC_GAIN_LVL_NUM (8)       /* rx dc补偿值档位数目 */

#ifdef _PRE_WLAN_1103_ATE
#define HI1105_TXDC_LPF_DAC_GAIN_LVL_NUM   1
#define HI1105_TXDC_MIXBUF_GAIN_LVL_NUM    1
#define HI1105_2G_TXDC_MIXBUF_GAIN_LVL_NUM 1
#define HI1105_5G_TXDC_MIXBUF_GAIN_LVL_NUM 1
#define HI1105_CALI_TXIQ_GAIN_LVL_NUM 1
#define HI1105_CALI_RXIQ_GAIN_LVL_NUM 1
#define HI1105_2G_TXIQ_GAIN_LVL_NUM   1
#define HI1105_5G_TXIQ_GAIN_LVL_NUM   1
#else
#define HI1105_TXDC_LPF_DAC_GAIN_LVL_NUM   8 /* tx dc补偿值lpf/dac档位数目 */
#define HI1105_TXDC_MIXBUF_GAIN_LVL_NUM    4 /* tx dc补偿值mixbuf档位数目 */
#define HI1105_2G_TXDC_MIXBUF_GAIN_LVL_NUM 1 /*  2G mixbuf配置和TPC一致(配置0档),其他档位不做校准 */
#define HI1105_5G_TXDC_MIXBUF_GAIN_LVL_NUM 2 /*  5G mixbuf配置和TPC一致(配置0/1档),其他档位不做校准 */
#define HI1105_CALI_TXIQ_GAIN_LVL_NUM 4      /* tx iq补偿值档位数目 */
#define HI1105_CALI_RXIQ_GAIN_LVL_NUM 3      /* rx iq补偿值档位数目 */
#define HI1105_2G_TXIQ_GAIN_LVL_NUM   4      /*  2G iq 档位配置和TPC保持一致(配置0/1/2档)，只对lpf档位进行校准  */
#define HI1105_5G_TXIQ_GAIN_LVL_NUM   3      /*  5G iq 档位配置和TPC保持一致(配置0/1/2档)，对lpf,mixbuf档位进行校准 */
#endif

#define HI1105_IQ_CALI_TIME_NUM   4         /*  为节省校准时间, 开关WIFI IQ校准不同档位，分多次校准 */

/* 芯片配置寄存器数，宏不可修改 */
/* C0/C1 TXDC补偿寄存器各64个 = 2(MIMO/SISO) * 8(LPF_DAC_GAIN_LVL_NUM) * 4(MIXBUF_GAIN_LVL_NUM) */
/* C0/C1 TXIQ补偿寄存器各56个 = 2(MIMO/SISO) * 7(LS_FIR_NUM) * 4(COMP_GAIN_LVL_NUM) */
/* C0/C1 RXIQ补偿寄存器各42个 = 2(MIMO/SISO) * 7(LS_FIR_NUM) * 3(COMP_GAIN_LVL_NUM) */
#define HI1105_PHY_TXDC_COMP_LPF_DAC_GAIN_LVL_NUM   8   /* tx dc补偿寄存器lpf/dac档位数 */
#define HI1105_PHY_TXDC_COMP_MIXBUF_GAIN_LVL_NUM    4   /* tx dc补偿寄存器mixbuf档位数 */
#define HI1105_TXIQ_LSFIR_NUM   7
#define HI1105_PHY_TXIQ_COMP_GAIN_LVL_NUM 4             /* tx iq补偿寄存器档位数 */
#define HI1105_RXIQ_LSFIR_NUM   7
#define HI1105_PHY_RXIQ_COMP_GAIN_LVL_NUM 3             /* rx iq补偿寄存器档位数 */

#define HI1105_CALI_ADC_CH_NUM 4 /* 4路ADC */
#define HI1105_CALI_IQ_CH_NUM 2
/* 校准信道个数定义 */
#define HI1105_CALI_2G_OTHER_CHANNEL_NUM 1 /* g_ast_2g_other_cali_channel校准信道数 */
#define HI1105_CALI_5G_OTHER_CHANNEL_NUM 1 /* g_ast_5g_other_cali_channel校准信道数 */

#define HI1105_CALI_IQ_LS_FILTER_TAP_NUM 7

#define HI1105_RXIQ_LSFIR_FEQ_NUM_320M     64
#define HI1105_RXIQ_LSFIR_FEQ_NUM_160M     32
#define HI1105_RXIQ_LSFIR_FEQ_NUM_80M      16
#define HI1105_RXIQ_LSFIR_FEQ_NUM_40M      16
#define HI1105_RXIQ_LSFIR_FEQ_NUM_80M_FPGA 32

#define HI1105_TXIQ_LSFIR_FEQ_NUM_640M      128
#define HI1105_TXIQ_LSFIR_FEQ_NUM_320M      64
#define HI1105_TXIQ_LSFIR_FEQ_NUM_160M      32
#define HI1105_TXIQ_LSFIR_FEQ_NUM_80M       32
#define HI1105_TXIQ_LSFIR_FEQ_NUM_160M_FPGA 64

/* 3 枚举定义 */
typedef enum {
    HI1105_CALI_SISO,
    HI1105_CALI_MIMO,

    HI1105_CALI_CHAIN_NUM_BUTT,
} hi1105_rf_cali_chain_num_enum;

/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
/* 复数结构 */
typedef struct {
    int32_t l_real;
    int32_t l_imag;
} hi1105_complex_stru;

/* ADC校准寄存器值
1. CMP
    0x400A0F08  RO C0_I_SUB0 [7:2]   C0_I_SUB1 [23:18]
    0x400A0F18  RO C0_I_SUB2 [7:2]   C0_I_SUB3 [23:18]
    0x400A0F38  RO C0_Q_SUB0 [7:2]   C0_Q_SUB1 [23:18]
    0x400A0F48  RO C0_Q_SUB2 [7:2]   C0_Q_SUB3 [23:18]

    0x400A0F68  RO C1_I_SUB0 [7:2]   C1_I_SUB1 [23:18]
    0x400A0F78  RO C1_I_SUB2 [7:2]   C1_I_SUB3 [23:18]
    0x400A0F98  RO C1_Q_SUB0 [7:2]   C1_Q_SUB1 [23:18]
    0x400A0FA8  RO C1_Q_SUB2 [7:2]   C1_Q_SUB3 [23:18]
2. CAP
    0x400A0280~0x400A029C  ch0_sub0_w1~ch0_sub7_w1 [16:0]
    0x400A02A0~0x400A02BC  ch1_sub0_w1~ch1_sub7_w1 [16:0]

    0x400A02C0~0x400A02DC  ch0_sub0_w2~ch0_sub7_w2 [16:0]
    0x400A02E0~0x400A02FC  ch1_sub0_w2~ch1_sub7_w2 [16:0]

    0x400A0300~0x400A031C  ch0_sub0_w3~ch0_sub7_w3 [16:0]
    0x400A0320~0x400A033C  ch1_sub0_w3~ch1_sub7_w3 [16:0]

    0x400A0340~0x400A035C  ch0_sub0_w4~ch0_sub7_w4 [16:0]
    0x400A0360~0x400A037C  ch1_sub0_w4~ch1_sub7_w4 [16:0]

    0x400A0380~0x400A039C  ch0_sub0_w5~ch0_sub7_w5 [16:0]
    0x400A03A0~0x400A03BC  ch1_sub0_w5~ch1_sub7_w5 [16:0]
3. INTR_GAIN
    0x400A0580~0x400A059C  INTR_GAIN_CAL_GAIN0~INTR_GAIN_CAL_GAIN7   [31:20]:1/Gain值 [18:0]计算的Gain值上报,无符号数据，整数位11，7位小数位

    0x400A05A0~0x400A05BC  INTR_GAIN_CAL_GAIN10~INTR_GAIN_CAL_GAIN17 [31:20]:1/Gain值 [18:0]计算的Gain值上报,无符号数据，整数位11，7位小数位
4. MOEC
    0x400A0640~0x400A065C  CH0_I/Q_moec_0~3 [16:0]

    0x400A0660~0x400A067C  CH1_I/Q_moec_0~3 [16:0]
5. MGEC
    0x400A07A0  MGEC_GAIN_CAL_C0_I_1 [16:0]  MGEC_GAIN_CAL_C0_I_2 [31:16]
    0x400A07A4  MGEC_GAIN_CAL_C0_Q_1 [16:0]  MGEC_GAIN_CAL_C0_Q_2 [31:16]
    0x400A07A8  MGEC_GAIN_CAL_C1_I_1 [16:0]  MGEC_GAIN_CAL_C1_I_2 [31:16]
    0x400A07AC  MGEC_GAIN_CAL_C1_Q_1 [16:0]  MGEC_GAIN_CAL_C1_Q_2 [31:16]

    0x400A07C0  MGEC_GAIN_CAL_C0_I_3 [16:0]  MGEC_GAIN_CAL_C0_I_Reserved [31:16]
    0x400A07C4  MGEC_GAIN_CAL_C0_Q_3 [16:0]  MGEC_GAIN_CAL_C0_Q_Reserved [31:16]
    0x400A07C8  MGEC_GAIN_CAL_C1_I_3 [16:0]  MGEC_GAIN_CAL_C1_I_Reserved [31:16]
    0x400A07CC  MGEC_GAIN_CAL_C1_Q_3 [16:0]  MGEC_GAIN_CAL_C1_Q_Reserved [31:16]
*/
typedef struct {
    /* 乘2为IQ两路数据 */
    uint8_t uc_cmp_cali[HI1105_CALI_ADC_CH_NUM * HI1105_CALI_IQ_CH_NUM];
    uint32_t cap_cali_w1[HI1105_CALI_ADC_CH_NUM * HI1105_CALI_IQ_CH_NUM];
    uint32_t cap_cali_w2[HI1105_CALI_ADC_CH_NUM * HI1105_CALI_IQ_CH_NUM];
    uint32_t cap_cali_w3[HI1105_CALI_ADC_CH_NUM * HI1105_CALI_IQ_CH_NUM];
    uint32_t cap_cali_w4[HI1105_CALI_ADC_CH_NUM * HI1105_CALI_IQ_CH_NUM];
    uint32_t cap_cali_w5[HI1105_CALI_ADC_CH_NUM * HI1105_CALI_IQ_CH_NUM];
    int32_t l_moec[HI1105_CALI_ADC_CH_NUM * HI1105_CALI_IQ_CH_NUM];
    uint16_t us_mgec[HI1105_CALI_ADC_CH_NUM * HI1105_CALI_IQ_CH_NUM];
} hi1105_adc_cali_param_stru;

typedef struct {
    uint8_t uc_rx_gain_cmp_code; /* 仅pilot RF使用，C校准补偿值 */

    /* LODIV 暂时和rx gain复用结构 */
    uint8_t uc_rx_mimo_cmp;
    uint8_t uc_dpd_siso_cmp;
    uint8_t uc_dpd_mimo_cmp;
} hi1105_rx_gain_comp_val_stru;

typedef struct {
    uint16_t aus_analog_rxdc_siso_cmp[HI1105_CALI_RXDC_GAIN_LVL_NUM];
    uint16_t aus_analog_rxdc_mimo_extlna_cmp[HI1105_CALI_RXDC_GAIN_LVL_NUM];
    uint16_t aus_analog_rxdc_mimo_intlna_cmp[HI1105_CALI_RXDC_GAIN_LVL_NUM];
    uint16_t us_digital_rxdc_cmp_i;
    uint16_t us_digital_rxdc_cmp_q;
    int16_t s_cali_temperature;
    int16_t s_mimo_cali_temperature;
} hi1105_rx_dc_comp_val_stru;

typedef struct {
    uint16_t us_ppa_cmp;
    uint16_t us_atx_pwr_cmp;
    int16_t s_2g_tx_ppa_dc;
    int16_t s_2g_tx_power_dc;
    int8_t c_dp_init;
    uint8_t uc_dtx_pwr_cmp;
    uint8_t auc_rsv[2]; // 2表示结构体需要四字节对齐
} hi1105_2g_tx_power_comp_val_stru;

typedef struct {
    uint16_t us_ppa_cmp;
    int16_t s_5g_tx_power_dc;
    uint16_t us_atx_pwr_cmp;
    uint8_t uc_dtx_pwr_cmp;
    uint8_t uc_mx_cmp;
} hi1105_5g_tx_power_comp_val_stru;

typedef struct {
    uint8_t uc_ssb_cmp;
    uint8_t uc_buf0_cmp;
    uint8_t uc_buf1_cmp;
    uint8_t uc_resv;
} hi1105_logen_comp_val_stru;

typedef struct {
    uint8_t uc_classa_cmp;
    uint8_t uc_classb_cmp;
    uint8_t auc_reserve[2]; // 2表示结构体需要四字节对齐
} hi1105_pa_ical_val_stru;

typedef struct {
    uint16_t aus_txdc_cmp_i[HI1105_TXDC_LPF_DAC_GAIN_LVL_NUM];
    uint16_t aus_txdc_cmp_q[HI1105_TXDC_LPF_DAC_GAIN_LVL_NUM];
} hi1105_txdc_comp_val_stru;

typedef struct {
    uint8_t uc_ppf_val;
    uint8_t uc_logen_ssb_ppf;
    uint8_t auc_reserve[2]; // 2表示结构体需要四字节对齐
} hi1105_ppf_comp_val_stru;

typedef struct {
    int32_t l_txiq_cmp_alpha[HI1105_CALI_TXIQ_GAIN_LVL_NUM];
    int32_t l_txiq_cmp_beta[HI1105_CALI_TXIQ_GAIN_LVL_NUM];
    int16_t as_txiq_comp_ls_fir_i[HI1105_CALI_TXIQ_GAIN_LVL_NUM][HI1105_TXIQ_LSFIR_NUM];
    int16_t as_txiq_comp_ls_fir_q[HI1105_CALI_TXIQ_GAIN_LVL_NUM][HI1105_TXIQ_LSFIR_NUM];
} hi1105_new_txiq_comp_stru;

typedef struct {
    int32_t l_rxiq_cmp_alpha[HI1105_CALI_RXIQ_GAIN_LVL_NUM];
    int32_t l_rxiq_cmp_beta[HI1105_CALI_RXIQ_GAIN_LVL_NUM];
    int16_t as_rxiq_comp_ls_fir_i[HI1105_CALI_RXIQ_GAIN_LVL_NUM][HI1105_RXIQ_LSFIR_NUM];
    int16_t as_rxiq_comp_ls_fir_q[HI1105_CALI_RXIQ_GAIN_LVL_NUM][HI1105_RXIQ_LSFIR_NUM];
} hi1105_new_rxiq_comp_stru;

typedef struct {
    hi1105_rx_dc_comp_val_stru ast_cali_rx_dc_cmp[HI1105_CALI_2G_OTHER_CHANNEL_NUM];
    hi1105_rx_gain_comp_val_stru ast_cali_rx_gain_cmp[HI1105_CALI_2G_OTHER_CHANNEL_NUM];
    hi1105_logen_comp_val_stru ast_cali_logen_cmp[HI1105_2G_CHAN_NUM];
    hi1105_2g_tx_power_comp_val_stru ast_cali_tx_power_cmp_2g[HI1105_2G_CHAN_NUM];
    hi1105_txdc_comp_val_stru ast_txdc_cmp_val[HI1105_2G_CHAN_NUM];
#ifdef _PRE_WLAN_NEW_IQ
    hi1105_new_txiq_comp_stru ast_new_txiq_cmp[HI1105_2G_CHAN_NUM][HI1105_CALI_CHAIN_NUM_BUTT];
    hi1105_new_rxiq_comp_stru ast_new_rxiq_cmp[HI1105_2G_CHAN_NUM][HI1105_CALI_CHAIN_NUM_BUTT];
#endif
} hi1105_2gcali_param_stru;

typedef struct {
    hi1105_rx_dc_comp_val_stru ast_cali_rx_dc_cmp[HI1105_CALI_2G_OTHER_CHANNEL_NUM];
    hi1105_txdc_comp_val_stru ast_txdc_cmp_val[HI1105_2G_CHAN_NUM];
#ifdef _PRE_WLAN_NEW_IQ
    hi1105_new_txiq_comp_stru ast_new_txiq_cmp[HI1105_2G_CHAN_NUM][HI1105_CALI_CHAIN_NUM_BUTT];
    hi1105_new_rxiq_comp_stru ast_new_rxiq_cmp[HI1105_2G_CHAN_NUM][HI1105_CALI_CHAIN_NUM_BUTT];
#endif
} hi1105_2g_dbdc_cali_param_stru;

typedef struct {
    hi1105_rx_dc_comp_val_stru st_cali_rx_dc_cmp;
    hi1105_rx_gain_comp_val_stru st_cali_rx_gain_cmp;
    hi1105_logen_comp_val_stru st_cali_logen_cmp;
    hi1105_5g_tx_power_comp_val_stru st_cali_tx_power_cmp_5g;
    hi1105_ppf_comp_val_stru st_ppf_cmp_val;
    hi1105_txdc_comp_val_stru ast_txdc_cmp_val[HI1105_TXDC_MIXBUF_GAIN_LVL_NUM];
#ifdef _PRE_WLAN_NEW_IQ
    hi1105_new_txiq_comp_stru ast_new_txiq_cmp[HI1105_CALI_CHAIN_NUM_BUTT];
    hi1105_new_rxiq_comp_stru ast_new_rxiq_cmp[HI1105_CALI_CHAIN_NUM_BUTT];
#endif
} hi1105_5gcali_param_stru;

typedef struct {
    hi1105_2gcali_param_stru ast_2gcali_param[HI1105_2G_CHAN_NUM];
    hi1105_5gcali_param_stru ast_5gcali_param[HI1105_5G_CHANNEL_NUM];
} hi1105_wifi_cali_param_stru;

typedef struct {
    uint16_t us_cali_time;
    uint16_t bit_temperature : 3,
               uc_5g_chan_idx1 : 5,
               uc_5g_chan_idx2 : 5,
               bit_rev : 3;
} hi1105_update_cali_channel_stru;

typedef struct {
    uint8_t uc_rc_cmp_code;
    uint8_t uc_r_cmp_code; /* 保存PMU的原始5bit R code */
    /* 重要: MPW2和PILOT RF公用, mpw2代表c校准值; pilot代表800M rejection补偿code，根据C code计算得到 */
    uint8_t uc_c_cmp_code;
    uint8_t uc_20m_rc_cmp_code;
} hi1105_rc_r_c_cali_param_stru;

#ifdef _PRE_WLAN_ONLINE_DPD
typedef struct {
    uint32_t aul_dpd_even_lut[HI1105_DPD_OFFLINE_CALI_LUT_NUM][HI1105_DPD_CALI_LUT_LENGTH];
    uint32_t aul_dpd_odd_lut[HI1105_DPD_OFFLINE_CALI_LUT_NUM][HI1105_DPD_CALI_LUT_LENGTH];
} hi1105_dpd_cali_lut_stru;
#endif

struct hi1105_cali_param_tag {
    uint32_t dog_tag;
    hi1105_2gcali_param_stru st_2gcali_param;
    hi1105_5gcali_param_stru ast_5gcali_param[HI1105_5G_CHANNEL_NUM];
#ifdef _PRE_WLAN_NEW_IQ
    hi1105_new_rxiq_comp_stru ast_20m_rxiq_cmp[HI1105_2G_CHAN_NUM + HI1105_5G_20M_CHAN_NUM][HI1105_CALI_CHAIN_NUM_BUTT];
    hi1105_new_rxiq_comp_stru ast_20m_rxiq_dbdc_cmp[HI1105_2G_CHAN_NUM][HI1105_CALI_CHAIN_NUM_BUTT];
#endif
#ifdef _PRE_WLAN_ONLINE_DPD
    hi1105_dpd_cali_lut_stru st_dpd_cali_data;
#endif
    hi1105_ppf_comp_val_stru st_165chan_ppf_comp;
    hi1105_update_cali_channel_stru st_cali_update_info;
    uint32_t check_hw_status;
    hi1105_pa_ical_val_stru st_pa_ical_cmp;
    hi1105_rc_r_c_cali_param_stru st_rc_r_c_cali_data;
    hi1105_2g_dbdc_cali_param_stru st_2g_dbdc_cali_param;
    int16_t s_2g_idet_gm;
    oal_bool_enum_uint8 en_use_lpf_10m_for_2grx_20m;
    oal_bool_enum_uint8 en_save_all;
    uint8_t uc_last_cali_fail_status;
    uint8_t uc_vdet_dc_comp;
    uint8_t uc_cur_cali_policy;
    oal_bool_enum_uint8 en_need_close_fem_cali;
    hi1105_adc_cali_param_stru st_adc_cali_data;
    uint8_t rx_en_invalid;
    uint8_t rsv[3]; /* 保留字段:3字节 */
};

typedef struct hi1105_cali_param_tag hi1105_cali_param_stru;

/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hal_cali.h */
