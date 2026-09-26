

#ifndef __WLAN_CALI_1106_H__
#define __WLAN_CALI_1106_H__

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "wlan_spec.h"
#include "wlan_customize.h"
#include "hal_common.h"
#include "hd_event.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 2 宏定义 */
/* 芯片配置寄存器数，宏不可修改 */
/* TRX IQ lut表RAM_SIZE: 260(depth)*32(width) */
#define HI1106_PHY_TXDC_COMP_LVL_NUM                4   /* tx dc补偿寄存器档位数 */
#define HI1106_PHY_TXIQ_COMP_LVL_NUM                4   /* tx iq补偿寄存器档位数目 */
#define HI1106_PHY_RXIQ_COMP_LVL_NUM                8   /* rx iq补偿寄存器档位数目 */
#define HI1106_CALI_IQ_QMC_QDC_COEF_NUM             10  /* 9阶系数 + 1个DC */

#define HI1106_CALI_IQ_QMC_SISO_COEF_ADDR_OFFSET    0
#define HI1106_CALI_IQ_QMC_MIMO_COEF_ADDR_OFFSET    40
#define HI1106_CALI_IQ_QDC_SISO_COEF_ADDR_OFFSET    80
#define HI1106_CALI_IQ_QDC_MIMO_COEF_ADDR_OFFSET    160
#define HI1106_CALI_IQ_QMC_FB_COEF_ADDR_OFFSET      240
#define HI1106_CALI_IQ_QDC_TX_COEF_ADDR_OFFSET      250

/* rx dc补偿值档位数目 */
#define HAL_CALI_RXDC_GAIN_LVL_NUM      8

#define WLAN_TX_DCIQ_CALI_LVL                 4     /* tx dc/tx iq补偿值档位数目 */
#define WLAN_RX_IQ_CALI_LVL                   8     /* rx iq补偿值档位数目 */

#define WLAN_RX_DC_LNA_LVL                    8

#define WLAN_IQ_QMC_QDC_COEF_NUM      10  /* 9阶系数 + 1个DC */

#define WLAN_DPD_EVEN_LUT_LENGTH      48
#define WLAN_DPD_ODD_LUT_LENGTH       48
#define WLAN_DPD_GLUE_LUT_LENGTH      43
#define WLAN_DPD_TPC_LEVEL_NUM        4
#define WLAN_DPD_RX_DC_FB_LVL         4

/* ABB CALI相关宏定义 */
#define WLAN_ABB_CALI_SUB_ADC_NUM_PER_IQ_SUB_CHN    4   /* 每一I路或者Q路SUB ADC的数目 */
/* 每个接收通道的SUB ADC数目,IQ两路和 */
#define WLAN_ABB_CALI_SUB_ADC_NUM_PER_CHN           (WLAN_ABB_CALI_SUB_ADC_NUM_PER_IQ_SUB_CHN * 2)
#define WLAN_ABB_CALI_CAP_WT_COMP_LEN               56
#define WLAN_ABB_CALI_INTER_ADC_GAIN_COMP_LEN       3
#define WLAN_ABB_CALI_INTER_ADC_OFFSET_COMP_LEN     3
#define WLAN_ABB_CALI_INTER_ADC_SKEW_COMP_LEN       2
#define WLAN_ABB_CALI_DAC_HIGH_LUT_LEN              32
#define WLAN_ABB_CALI_DAC_MID_LUT_LEN               16

/* 2G/5G RF划分子频段数目 */
#define WLAN_2G_RF_BAND_NUM      3
/* 2G/5G/6G 软件定义校准子频段数目 */
#define WLAN_2G_CALI_BAND_NUM      3
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
#define WLAN_5G_RF_BAND_NUM      15
#else
#define WLAN_5G_RF_BAND_NUM      7
#endif
#define WLAN_5G_20M_CALI_BAND_NUM  7
#define WLAN_5G_160M_CALI_BAND_NUM  2
#define WLAN_5G_CALI_SUB_BAND_NUM (WLAN_5G_20M_CALI_BAND_NUM + WLAN_5G_160M_CALI_BAND_NUM)
#define WLAN_6G_20M_CALI_BAND_NUM  15
#define WLAN_6G_160M_CALI_BAND_NUM  7
#define WLAN_6G_CALI_SUB_BAND_NUM (WLAN_6G_20M_CALI_BAND_NUM + WLAN_6G_160M_CALI_BAND_NUM)

/* 各带宽是否启动校准标志, bit3:0 160M|80M|40M|20M */
#define WLAN_RF_CALI_BW_MASK   0xD

static inline uint8_t wlan_cali_in_valid_range(uint16_t value, uint16_t start, uint16_t end)
{
    return (((value) >= (start)) && ((value) <= (end)));
}
#ifndef WCPU_CTL_RB_WCPU_CTL_GP_REG3_REG
#define WCPU_CTL_RB_WCPU_CTL_GP_REG3_REG 0x40100114
#endif
#define HI1106_CALI_WCTL_ADDR_REG WCPU_CTL_RB_WCPU_CTL_GP_REG3_REG
#define HI1106_CALI_MAX_MEM_SIZE (147120) //  reserved for cali data
#ifdef _PRE_WLAN_FEATURE_MEM_TX_BUGFIX
#define HI1106_DPD_CORRAM_DATA_LEN    6144
#else
#define HI1106_DPD_CORRAM_DATA_LEN    3072
#endif
#define HI1106_CALI_DPD_CORRAM_DATA_SIZE    (HI1106_DPD_CORRAM_DATA_LEN * sizeof(uint32_t))
#define HI1106_CALI_MATRIX_SIZE (HI1106_CALI_DPD_CORRAM_DATA_SIZE)
#define HI1106_CALI_DPD_CORRAM_DATA_OFST    0
#define HI1106_CALI_DATA_OFST    HI1106_CALI_DPD_CORRAM_DATA_SIZE

#define CALI_DATA_NETBUF_SEND_LEN 1400
/* 3 枚举定义 */
typedef enum {
    HI1106_CALI_SISO,
    HI1106_CALI_MIMO,

    HI1106_CALI_CHAIN_NUM_BUTT,
} hi1106_rf_cali_chain_num_enum;
typedef uint8_t hi1106_rf_cali_chain_num_enum_uint8;

typedef enum {
    WLAN_ABB_CALI                = 0,
    WLAN_RF_CALI_RC              = 1,
    WLAN_RF_CALI_R               = 2,
    WLAN_RF_CALI_C               = 3,
    WLAN_RF_CALI_PPF             = 4,
    WLAN_RF_CALI_LOGEN_SSB       = 5,
    WLAN_RF_CALI_LO_BF           = 6,
    WLAN_RF_CALI_LO_INVOLT_SWING = 7,
    WLAN_RF_CALI_PA_CURRENT      = 8,
    WLAN_RF_CALI_LNA_BLK_DCOC    = 9,
    WLAN_RF_CALI_RX_DC           = 10,
    WLAN_RF_CALI_MIMO_RX_DC      = 11,
    WLAN_RF_CALI_DPD_RX_DC       = 12,
    WLAN_RF_CALI_TX_PWR          = 13,
    WLAN_RF_CALI_TX_LO           = 14,
    WLAN_RF_CALI_MIMO_TX_LO      = 15,
    WLAN_RF_CALI_TX_IQ           = 16,
    WLAN_RF_CALI_MIMO_TX_IQ      = 17,
    WLAN_RF_CALI_RX_IQ           = 18,
    WLAN_RF_CALI_MIMO_RX_IQ      = 19,
    WLAN_RF_CALI_IIP2            = 20,
    WLAN_RF_CALI_DPD             = 21,
    WLAN_RF_CALI_DFS_RX_DC       = 22,
    WLAN_RF_CALI_TYPE_BUTT,

    WLAN_RF_CALI_CHANNEL_START    = 31,
    WLAN_RF_CALI_CHANNEL_BUTT
} wlan_rf_cali_type_enum;
typedef uint8_t wlan_rf_cali_type_enum_uint8;

typedef enum {
    WLAN_ONLINE_RXDC_CALI   = 0,
    WLAN_ONLINE_PWR_CALI    = 1,
    WLAN_ONLINE_TXIQ_CALI   = 2,
    WLAN_ONLINE_RXIQ_CALI   = 3,
    WLAN_ONLINE_DPD_CALI    = 4,

    WLAN_RF_ONLINE_CALI_TYPE_BUTT,
} wlan_rf_online_cali_type_enum;

/* RF通道枚举 */
typedef enum {
    WLAN_SINGLE_STREAM_0 = 0,  /* C0 */
    WLAN_SINGLE_STREAM_1 = 1,  /* C1 */
    WLAN_SINGLE_STREAM_2 = 2,  /* C2 */
    WLAN_SINGLE_STREAM_3 = 3,  /* C3 */
    WLAN_STREAM_TYPE_BUTT = 4, /* 代表无效的通道类型，规格宏请采用WLAN_RF_STREAM_NUMS */
} wlan_stream_type_enum;
typedef uint8_t wlan_stream_type_enum_uint8;

typedef enum {
    WLAN_NORMAL_CHN = 0,    /* 主路当前信道 */
    WLAN_DBDC_CHN = 1,      /* 辅路当前信道 */
    WLAN_20M_ALL_CHN = 2,   /* 20M扫描与通道有关的部分 */
    WLAN_20M_COMMON = 3,    /* 20M扫描与通道无关的部分 */

    WLAN_H2D_CALI_DATA_TYPE_BUTT
} wlan_h2d_cali_data_type_enum;
typedef uint8_t wlan_h2d_cali_data_type_enum_uint8;

/* 校准数据上传枚举变量 */
typedef enum {
    WLAN_CALI_DATA_C0_2G_20M = 0,
    WLAN_CALI_DATA_C0_2G_40M = 1,
    WLAN_CALI_DATA_C0_5G_20M = 2,
    WLAN_CALI_DATA_C0_5G_40M = 3,
    WLAN_CALI_DATA_C0_5G_80M = 4,
    WLAN_CALI_DATA_C0_5G_160M = 5,
    WLAN_CALI_DATA_C0_6G_20M = 6,
    WLAN_CALI_DATA_C0_6G_40M = 7,
    WLAN_CALI_DATA_C0_6G_80M = 8,
    WLAN_CALI_DATA_C0_6G_160M = 9,
    WLAN_CALI_DATA_C0_BUTT = 10,

    WLAN_CALI_DATA_C1_2G_20M = WLAN_CALI_DATA_C0_BUTT,
    WLAN_CALI_DATA_C1_2G_40M = 11,
    WLAN_CALI_DATA_C1_5G_20M = 12,
    WLAN_CALI_DATA_C1_5G_40M = 13,
    WLAN_CALI_DATA_C1_5G_80M = 14,
    WLAN_CALI_DATA_C1_5G_160M = 15,
    WLAN_CALI_DATA_C1_6G_20M = 16,
    WLAN_CALI_DATA_C1_6G_40M = 17,
    WLAN_CALI_DATA_C1_6G_80M = 18,
    WLAN_CALI_DATA_C1_6G_160M = 19,

    WLAN_CALI_DATA_C2_2G_20M = 20,
    WLAN_CALI_DATA_C2_2G_40M = 12,
    WLAN_CALI_DATA_C2_5G_20M = 22,
    WLAN_CALI_DATA_C2_5G_40M = 23,
    WLAN_CALI_DATA_C2_5G_80M = 24,
    WLAN_CALI_DATA_C2_5G_160M = 25,
    WLAN_CALI_DATA_C2_6G_20M = 26,
    WLAN_CALI_DATA_C2_6G_40M = 27,
    WLAN_CALI_DATA_C2_6G_80M = 28,
    WLAN_CALI_DATA_C2_6G_160M = 29,

    WLAN_CALI_DATA_C3_2G_20M = 30,
    WLAN_CALI_DATA_C3_2G_40M = 31,
    WLAN_CALI_DATA_C3_5G_20M = 32,
    WLAN_CALI_DATA_C3_5G_40M = 33,
    WLAN_CALI_DATA_C3_5G_80M = 34,
    WLAN_CALI_DATA_C3_5G_160M = 35,
    WLAN_CALI_DATA_C3_6G_20M = 36,
    WLAN_CALI_DATA_C3_6G_40M = 37,
    WLAN_CALI_DATA_C3_6G_80M = 38,
    WLAN_CALI_DATA_C3_6G_160M = 39,
    WLAN_CALI_DATA_DBDC = 40,
    WLAN_CALI_DATA_COMMON = 41,
    WLAN_D2H_CALI_DATA_TYPE_BUTT
} wlan_d2h_cali_data_type_enum;
typedef uint8_t wlan_d2h_cali_data_type_enum_uint8;

/* 校准RF带宽枚举 */
typedef enum {
    HI1106_RF_CALI_BW_20M, /* 目前10M LPF BW带宽不单独校准, 用20M LPF BW校准值 */
    HI1106_RF_CALI_BW_2G_BUTT,
    HI1106_RF_CALI_BW_40M = HI1106_RF_CALI_BW_2G_BUTT,
    HI1106_RF_CALI_BW_80M,
    HI1106_RF_CALI_BW_5G_BUTT,

    HI1106_RF_CALI_BW_BUTT = HI1106_RF_CALI_BW_5G_BUTT,
} hi1106_rf_cali_bandwidth_enum;
typedef uint8_t hi1106_rf_cali_bandwidth_enum_uint8;

/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
typedef struct {
    uint32_t qdc_qmc_coef[HI1106_CALI_IQ_QMC_QDC_COEF_NUM];
} hi1106_qdc_qmc_coef_stru;

typedef struct {
    uint16_t us_cali_time;
    uint16_t bit_temperature : 3,
               uc_5g_chan_idx1 : 5,
               uc_5g_chan_idx2 : 5,
               bit_rev : 3;
} hi1106_update_cali_channel_stru;

struct hi1106_cali_param_tag {
    uint32_t dog_tag;
    hi1106_update_cali_channel_stru st_cali_update_info;
    uint32_t check_hw_status;
    oal_bool_enum_uint8 en_save_all;
    uint8_t uc_last_cali_fail_status;
    uint8_t uc_cur_cali_policy;
    oal_bool_enum_uint8 en_need_close_fem_cali;
};
typedef struct hi1106_cali_param_tag hi1106_cali_param_stru;

typedef struct {
    uint32_t inner_adc_offset_code_i;
    uint32_t inner_adc_offset_code_q;
    uint32_t inner_adc_cap_wt[WLAN_ABB_CALI_CAP_WT_COMP_LEN];
    uint32_t inter_adc_gain[WLAN_ABB_CALI_INTER_ADC_GAIN_COMP_LEN];
    uint32_t inter_adc_offset_code[WLAN_ABB_CALI_INTER_ADC_OFFSET_COMP_LEN];
    uint32_t inter_adc_skew_code[WLAN_ABB_CALI_INTER_ADC_SKEW_COMP_LEN];
    uint32_t dac_high_lut_coeff[WLAN_ABB_CALI_DAC_HIGH_LUT_LEN];
    uint32_t dac_mid_lut_coeff[WLAN_ABB_CALI_DAC_MID_LUT_LEN];
} wlan_cali_abb_para_stru;

typedef struct {
    uint16_t rc_cmp_code;
    uint16_t r_cmp_code;
    uint16_t c_cmp_code;
    uint16_t rc_20m_cmp_code;
} wlan_cali_rc_r_c_para_stru;

typedef struct {
    uint8_t ssb_ppf_code;
    uint8_t trx_ppf_code;   /* 06 rf不使用,共平台5x需要 */
    uint8_t resv[2];  /* 对齐 2 */
} wlan_cali_ppf_para_stru;

typedef struct {
    uint8_t lo_ssb_tune;
    uint8_t lo_vb_tune;
    uint8_t lo_buf_tune;
    uint8_t lo_buf_tune_other;  /* 另一个通道的LO_BUF校正值 */
} wlan_cali_logen_para_stru;

typedef struct {
    uint8_t tx_lo;
    uint8_t tx_dpd_lo_tx;
    uint8_t tx_dpd_lo_dpd;
    uint8_t rx_lo;
} wlan_cali_lodiv_para_stru;

typedef struct {
    uint8_t fine_code;
    uint8_t resv[3];  /* 对齐 3 */
} wlan_cali_lna_blk_para_stru;

typedef struct {
    uint8_t  uc_vco_ldo_index;
    uint8_t  uc_trim_buf2lo_code;
    uint8_t  auc_resev[2];  /* 对齐 2 */
}wlan_cali_involt_swing_para_stru;

typedef struct {
    uint8_t code_i;
    uint8_t code_q;
    uint8_t resv[2];  /* 对齐 2 */
} wlan_cali_iip2_para_stru;

typedef struct {
    // RX DC
    uint8_t analog_rxdc_cmp_intlna_i[WLAN_RX_DC_LNA_LVL];
    uint8_t analog_rxdc_cmp_intlna_q[WLAN_RX_DC_LNA_LVL];
    uint8_t analog_rxdc_cmp_extlna_i[WLAN_RX_DC_LNA_LVL];
    uint8_t analog_rxdc_cmp_extlna_q[WLAN_RX_DC_LNA_LVL];
    uint16_t digital_rxdc_cmp_i;
    uint16_t digital_rxdc_cmp_q;
} wlan_cali_rxdc_para_stru;

typedef struct {
    uint8_t analog_dpd_rxdc_cmp_i[WLAN_DPD_RX_DC_FB_LVL];
    uint8_t analog_dpd_rxdc_cmp_q[WLAN_DPD_RX_DC_FB_LVL];
    uint16_t digital_dpd_rxdc_cmp_i;
    uint16_t digital_dpd_rxdc_cmp_q;
    int16_t cali_temperature;
    uint16_t resv;
} wlan_cali_dpd_rxdc_para_stru;

typedef struct {
    uint16_t  ppa_cmp;
    uint16_t  mx_cap_cmp;
    uint16_t  ppa_cap_cmp;
    uint16_t  rsv;
} wlan_cali_txpwr_para_stru;

typedef struct {
    uint16_t  txdc_cmp_i[WLAN_TX_DCIQ_CALI_LVL];
    uint16_t  txdc_cmp_q[WLAN_TX_DCIQ_CALI_LVL];
} wlan_cali_txdc_para_stru;

typedef struct {
    uint32_t coef[WLAN_IQ_QMC_QDC_COEF_NUM];
} wlan_cali_iq_coef_stru;

typedef struct {
    uint8_t vga_code;
    uint8_t att_code;
    uint8_t reserved[2];  /* 2, 对齐 */
} wlan_cali_iq_gain_cfg_stru;

typedef struct {
    wlan_cali_iq_coef_stru qmc_siso;
    wlan_cali_iq_coef_stru qmc_mimo;
    /* 保存offline txiq校准调整的增益配置, 用于online iq校准 */
    wlan_cali_iq_gain_cfg_stru gain_cfg;
} wlan_cali_txiq_para_per_lvl_stru;

typedef struct {
    wlan_cali_txiq_para_per_lvl_stru qmc_data[WLAN_TX_DCIQ_CALI_LVL];
    wlan_cali_iq_coef_stru qmc_fb;
} wlan_cali_txiq_para_per_freq_stru;

typedef struct {
    wlan_cali_iq_coef_stru qdc_siso[WLAN_RX_IQ_CALI_LVL];
    wlan_cali_iq_coef_stru qdc_mimo[WLAN_RX_IQ_CALI_LVL];
    wlan_cali_iq_coef_stru qdc_tx;
} wlan_cali_rxiq_para_per_freq_stru;

typedef struct {
    uint32_t dpd_even_lut[WLAN_DPD_EVEN_LUT_LENGTH];
    uint32_t dpd_odd_lut[WLAN_DPD_ODD_LUT_LENGTH];
    uint32_t dpd_glue_lut[WLAN_DPD_GLUE_LUT_LENGTH];
    uint8_t vga_gain;
    uint8_t resv[3];  /* 对齐 3 */
} wlan_cali_dpd_lut_stru;

typedef struct {
    wlan_cali_dpd_lut_stru dpd_cali_para[WLAN_DPD_TPC_LEVEL_NUM];
} wlan_cali_dpd_para_stru;

typedef struct {
    uint16_t center_freq_start;
    uint16_t center_freq_end;
} wlan_cali_div_band_stru;

/* 单个带宽单个信道基准校准数据结构体 */
typedef struct {
    wlan_cali_rxdc_para_stru  rxdc_cali_data;
    wlan_cali_rxdc_para_stru  rxdc_mimo_cali_data;
    wlan_cali_dpd_rxdc_para_stru  dpd_rxdc_cali_data;
    wlan_cali_txdc_para_stru  txdc_cali_data;
    wlan_cali_txdc_para_stru  txdc_mimo_cali_data;
    wlan_cali_txpwr_para_stru txpwr_cali_data;
    wlan_cali_txiq_para_per_freq_stru txiq_cali_data;
    wlan_cali_rxiq_para_per_freq_stru rxiq_cali_data;
    wlan_cali_iip2_para_stru iip2_cali_data;
    wlan_cali_dpd_para_stru  dpd_cali_data;
} wlan_cali_basic_para_stru;

typedef struct {
    wlan_cali_rc_r_c_para_stru rc_r_c_cali_data;
    wlan_cali_lna_blk_para_stru lna_blk_cali_data;
    wlan_cali_logen_para_stru logen_cali_data[WLAN_2G_CALI_BAND_NUM];
    wlan_cali_basic_para_stru cali_data[WLAN_2G_CALI_BAND_NUM];
} wlan_cali_2g_20m_save_stru;
typedef struct {
    wlan_cali_basic_para_stru cali_data[WLAN_2G_CALI_BAND_NUM];
} wlan_cali_2g_40m_save_stru;

typedef struct {
    wlan_cali_rc_r_c_para_stru rc_r_c_cali_data;
    wlan_cali_lna_blk_para_stru lna_blk_cali_data;
    wlan_cali_ppf_para_stru ppf_cali_data[WLAN_5G_20M_CALI_BAND_NUM];
    wlan_cali_logen_para_stru logen_cali_data[WLAN_5G_20M_CALI_BAND_NUM];
    uint8_t pa_current_data[WLAN_5G_20M_CALI_BAND_NUM];
    uint8_t resv;
    wlan_cali_basic_para_stru cali_data[WLAN_5G_20M_CALI_BAND_NUM];
} wlan_cali_5g_20m_save_stru;

typedef struct {
    wlan_cali_basic_para_stru cali_data[WLAN_5G_20M_CALI_BAND_NUM];
} wlan_cali_5g_40m_save_stru;

typedef struct {
    wlan_cali_basic_para_stru cali_data[WLAN_5G_20M_CALI_BAND_NUM];
    wlan_cali_dpd_para_stru dpd_cali_data_side_plus[WLAN_5G_20M_CALI_BAND_NUM];
    wlan_cali_dpd_para_stru dpd_cali_data_side_minus[WLAN_5G_20M_CALI_BAND_NUM];
} wlan_cali_5g_80m_save_stru;

typedef struct {
    wlan_cali_basic_para_stru cali_data[WLAN_5G_160M_CALI_BAND_NUM];
    wlan_cali_dpd_para_stru dpd_cali_data_side_plus[WLAN_5G_160M_CALI_BAND_NUM];
    wlan_cali_dpd_para_stru dpd_cali_data_side_minus[WLAN_5G_160M_CALI_BAND_NUM];
} wlan_cali_5g_160m_save_stru;
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
typedef struct {
    wlan_cali_rc_r_c_para_stru rc_r_c_cali_data;
    wlan_cali_lna_blk_para_stru lna_blk_cali_data;
    wlan_cali_ppf_para_stru ppf_cali_data[WLAN_6G_20M_CALI_BAND_NUM];
    wlan_cali_logen_para_stru logen_cali_data[WLAN_6G_20M_CALI_BAND_NUM];
    uint8_t  pa_current_data[WLAN_6G_20M_CALI_BAND_NUM];
    uint8_t  resv;
    wlan_cali_basic_para_stru cali_data[WLAN_6G_20M_CALI_BAND_NUM];
} wlan_cali_6g_20m_save_stru;

typedef struct {
    wlan_cali_basic_para_stru cali_data[WLAN_5G_20M_CALI_BAND_NUM];
} wlan_cali_6g_40m_save_stru;

typedef struct {
    wlan_cali_basic_para_stru cali_data[WLAN_6G_20M_CALI_BAND_NUM];
    wlan_cali_dpd_para_stru  dpd_cali_data_side_plus[WLAN_6G_20M_CALI_BAND_NUM];
    wlan_cali_dpd_para_stru  dpd_cali_data_side_minus[WLAN_6G_20M_CALI_BAND_NUM];
} wlan_cali_6g_80m_save_stru;

typedef struct {
    wlan_cali_basic_para_stru cali_data[WLAN_6G_160M_CALI_BAND_NUM];
    wlan_cali_dpd_para_stru  dpd_cali_data_side_plus[WLAN_6G_160M_CALI_BAND_NUM];
    wlan_cali_dpd_para_stru  dpd_cali_data_side_minus[WLAN_6G_160M_CALI_BAND_NUM];
} wlan_cali_6g_160m_save_stru;
#endif

/* dbdc单个带宽单信道基准校准数据结构体 */
typedef struct {
    wlan_cali_rxdc_para_stru  rxdc_cali_data;
    wlan_cali_rxdc_para_stru  rxdc_mimo_cali_data;
    wlan_cali_txdc_para_stru  txdc_cali_data;
    wlan_cali_txdc_para_stru  txdc_mimo_cali_data;
    wlan_cali_txiq_para_per_freq_stru txiq_cali_data;
    wlan_cali_rxiq_para_per_freq_stru rxiq_cali_data;
} wlan_cali_dbdc_basic_para_stru;

typedef struct {
    wlan_cali_dbdc_basic_para_stru  cali_data_20[WLAN_2G_CALI_BAND_NUM];
} wlan_cali_2g_dbdc_para_stru;

typedef struct {
    wlan_cali_dbdc_basic_para_stru cali_data_20[WLAN_5G_20M_CALI_BAND_NUM];
    wlan_cali_dbdc_basic_para_stru cali_data_80[WLAN_5G_20M_CALI_BAND_NUM];
    wlan_cali_dbdc_basic_para_stru cali_data_160[WLAN_5G_160M_CALI_BAND_NUM];
} wlan_cali_5g_dbdc_para_stru;

#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
typedef struct {
    wlan_cali_dbdc_basic_para_stru cali_data_20[WLAN_6G_20M_CALI_BAND_NUM];
    wlan_cali_dbdc_basic_para_stru cali_data_80[WLAN_6G_20M_CALI_BAND_NUM];
    wlan_cali_dbdc_basic_para_stru cali_data_160[WLAN_6G_160M_CALI_BAND_NUM];
} wlan_cali_6g_dbdc_para_stru;
#endif

/* 校准策略/控制参数 */
typedef struct {
    hi1106_update_cali_channel_stru cali_update_info;
    uint32_t check_hw_status[HD_EVENT_RF_NUM];

    oal_bool_enum_uint8 en_save_all;
    uint8_t cur_cali_policy;
    oal_bool_enum_uint8 en_need_close_fem_cali;
    uint8_t last_cali_fail_status;
} wlan_cali_ctl_para_stru;

/* DBDC和非通道相关校准数据结构体 */
typedef struct {
    wlan_cali_abb_para_stru abb_cali_data[HD_EVENT_RF_NUM];     /* abb校准一次校准4个通道，需要放在common上传 */
    wlan_cali_lodiv_para_stru lodiv_cali_data[HD_EVENT_RF_NUM]; /* lodiv校准一次校准4个通道，需要放在common上传 */
    wlan_cali_ctl_para_stru cali_ctl_data;
} wlan_cali_common_para_stru;

typedef struct {
    wlan_cali_2g_dbdc_para_stru cali_data_2g_dbdc;
    wlan_cali_5g_dbdc_para_stru cali_data_5g_dbdc;
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    wlan_cali_6g_dbdc_para_stru cali_data_6g_dbdc;
#endif
} wlan_cali_dbdc_para_stru;

/* 20M带宽iq校准数据:只保存一档数据 */
typedef struct {
    wlan_cali_txiq_para_per_lvl_stru qmc_data;
    wlan_cali_iq_coef_stru qmc_fb;
} wlan_cali_txiq_20m_para_per_freq_stru;

/* 20M带宽dpd校准数据:只保存一档数据 */
typedef struct {
    wlan_cali_dpd_lut_stru dpd_cali_para;
} wlan_cali_dpd_20m_para_stru;

typedef struct {
    wlan_cali_rxdc_para_stru  rxdc_cali_data;
    wlan_cali_dpd_rxdc_para_stru  dpd_rxdc_cali_data;
    wlan_cali_txdc_para_stru txdc_cali_data;
    wlan_cali_txpwr_para_stru txpwr_cali_data;
    wlan_cali_txiq_20m_para_per_freq_stru txiq_cali_data;
    wlan_cali_rxiq_para_per_freq_stru rxiq_cali_data;
    wlan_cali_iip2_para_stru iip2_cali_data;
    wlan_cali_dpd_20m_para_stru  dpd_cali_data;
} wlan_cali_20m_basic_para_stru;

/* 扫描(20m)2G校准数据结构体 */
typedef struct {
    wlan_cali_20m_basic_para_stru chn_cali_data[WLAN_2G_CALI_BAND_NUM];
    wlan_cali_rc_r_c_para_stru rc_r_c_cali_data;
    wlan_cali_lna_blk_para_stru lna_blk_cali_data;
    wlan_cali_logen_para_stru logen_cali_data[WLAN_2G_CALI_BAND_NUM];
    wlan_cali_lodiv_para_stru lodiv_cali_data;
} wlan_cali_2g_20m_para_stru;

/* 扫描(20m)5G校准数据结构体 */
typedef struct {
    wlan_cali_20m_basic_para_stru chn_cali_data[WLAN_5G_20M_CALI_BAND_NUM];
    wlan_cali_rc_r_c_para_stru rc_r_c_cali_data;
    wlan_cali_lna_blk_para_stru lna_blk_cali_data;
    wlan_cali_ppf_para_stru ppf_cali_data[WLAN_5G_20M_CALI_BAND_NUM];
    wlan_cali_logen_para_stru logen_cali_data[WLAN_5G_20M_CALI_BAND_NUM];
    uint8_t  pa_current_data[WLAN_5G_20M_CALI_BAND_NUM];
    uint8_t  resv;
} wlan_cali_5g_20m_para_stru;

#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
/* 扫描(20m)6G校准数据结构体 */
typedef struct {
    wlan_cali_20m_basic_para_stru chn_cali_data[WLAN_6G_20M_CALI_BAND_NUM];
    wlan_cali_rc_r_c_para_stru rc_r_c_cali_data;
    wlan_cali_lna_blk_para_stru lna_blk_cali_data;
    wlan_cali_ppf_para_stru ppf_cali_data[WLAN_6G_20M_CALI_BAND_NUM];
    wlan_cali_logen_para_stru logen_cali_data[WLAN_6G_20M_CALI_BAND_NUM];
    uint8_t  pa_current_data[WLAN_6G_20M_CALI_BAND_NUM];
    uint8_t  resv;
} wlan_cali_6g_20m_para_stru;
#endif

/* 扫描(20m)单个通道校准数据结构体 */
typedef struct {
    wlan_cali_abb_para_stru abb_cali_data;
    wlan_cali_2g_20m_para_stru cali_data_2g;
    wlan_cali_5g_20m_para_stru cali_data_5g;
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    wlan_cali_6g_20m_para_stru cali_data_6g;
#endif
} wlan_cali_rf_20m_para_stru;

/* 扫描(20m)comm数据结构体(不区分通道) */
typedef struct {
    oal_bool_enum_uint8 en_save_all;
    uint8_t cur_cali_policy;
    oal_bool_enum_uint8 en_need_close_fem_cali;
    uint8_t rsv;
} wlan_cali_data_20m_common_para_stru;

/* 单通道校准数据结构体 */
typedef struct {
    wlan_cali_2g_20m_save_stru cali_data_2g_20m;
    wlan_cali_5g_20m_save_stru cali_data_5g_20m;
    wlan_cali_5g_80m_save_stru cali_data_5g_80m;
    wlan_cali_5g_160m_save_stru cali_data_5g_160m;
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    wlan_cali_6g_20m_save_stru cali_data_6g_20m;
    wlan_cali_6g_80m_save_stru cali_data_6g_80m;
    wlan_cali_6g_160m_save_stru cali_data_6g_160m;
#endif
} wlan_cali_rf_para_stru;

/* host校准数据结构体 */
typedef struct {
    wlan_cali_rf_para_stru rf_cali_data[HD_EVENT_RF_NUM];
    wlan_cali_dbdc_para_stru dbdc_cali_data[HD_EVENT_SLAVE_RF_NUM];
    wlan_cali_common_para_stru common_cali_data;
} wlan_cali_data_para_stru;

/* 单信道校准数据结构体 */
typedef struct {
    wlan_cali_basic_para_stru cali_data;
    wlan_cali_dpd_para_stru  dpd_cali_data_side_plus;
    wlan_cali_dpd_para_stru  dpd_cali_data_side_minus;
} wlan_cali_chn_stru;

/* 定制化 power ref 2g 5g 6g配置参数 */
/* customize rf front cfg struct */
typedef struct {
    /* 2g */
    int8_t gain_db_2g[MAC_NUM_2G_BAND][HD_EVENT_RF_NUM];
    /* 5g */
    int8_t gain_db_5g[MAC_NUM_5G_BAND][HD_EVENT_RF_NUM];
    /* 6g */
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    int8_t gain_db_6g[MAC_NUM_6G_BAND][HD_EVENT_RF_NUM];
#endif
} mac_cust_rf_loss_gain_db_stru;

typedef struct {
    int8_t delta_rssi_2g[CUS_NUM_2G_DELTA_RSSI_NUM][HD_EVENT_RF_NUM];
    int8_t delta_rssi_5g[CUS_NUM_5G_DELTA_RSSI_NUM][HD_EVENT_RF_NUM];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    int8_t delta_rssi_6g[CUS_NUM_5G_DELTA_RSSI_NUM][HD_EVENT_RF_NUM];
#endif
} mac_cus_rf_delta_pwr_ref_stru;

typedef struct {
    int8_t amend_rssi_2g[CUS_NUM_2G_AMEND_RSSI_NUM][HD_EVENT_RF_NUM];
    int8_t amend_rssi_5g[CUS_NUM_5G_AMEND_RSSI_NUM][HD_EVENT_RF_NUM];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    int8_t amend_rssi_6g[CUS_NUM_5G_AMEND_RSSI_NUM][HD_EVENT_RF_NUM];
#endif
} mac_custom_rf_amend_rssi_stru;

typedef struct {
    uint16_t lna_on2off_time_ns[HD_EVENT_RF_NUM]; /* LNA开到LNA关的时间(ns) */
    uint16_t lna_off2on_time_ns[HD_EVENT_RF_NUM]; /* LNA关到LNA开的时间(ns) */

    int8_t lna_bypass_gain_db[HD_EVENT_RF_NUM];   /* 外部LNA bypass时的增益(dB) */
    int8_t lna_gain_db[HD_EVENT_RF_NUM];          /* 外部LNA增益(dB) */
    int8_t pa_gain_b0_db[HD_EVENT_RF_NUM];        /* 外部PA b0 增益(dB) */
    uint8_t ext_switch_isexist[HD_EVENT_RF_NUM]; /* 是否使用外部switch */

    uint8_t ext_pa_isexist[HD_EVENT_RF_NUM];                  /* 是否使用外部pa */
    oal_fem_lp_state_enum_uint8 en_fem_lp_enable[HD_EVENT_RF_NUM]; /* 是否支持fem低功耗标志 */
    int8_t c_fem_spec_value[HD_EVENT_RF_NUM];                    /* fem spec功率点 */
    uint8_t ext_lna_isexist[HD_EVENT_RF_NUM];                 /* 是否使用外部lna */

    int8_t pa_gain_b1_db[HD_EVENT_RF_NUM];     /* 外部PA b1增益(dB) */
    uint8_t pa_gain_lvl_num[HD_EVENT_RF_NUM]; /* 外部PA 增益档位数 */
    uint16_t fem_mode; /* FEM控制真值表 */
    uint8_t resv[2];
} mac_custom_ext_rf_stru;

typedef struct {
    mac_cust_rf_loss_gain_db_stru rf_loss_gain_db;           /* 2.4g 5g 插损 */
    mac_custom_ext_rf_stru ext_rf[WLAN_CALI_BAND_BUTT];   /* 2.4g 5g fem */
    mac_cus_rf_delta_pwr_ref_stru delta_pwr_ref;         /* delta_rssi */
    mac_custom_rf_amend_rssi_stru rssi_amend_cfg;       /* rssi_amend */

    /* 注意，如果修改了对应的位置，需要同步修改函数: hal_config_custom_rf  */
    int8_t delta_cca_ed_high_20th_2g;
    int8_t delta_cca_ed_high_40th_2g;
    int8_t delta_cca_ed_high_20th_5g;
    int8_t delta_cca_ed_high_40th_5g;

    int8_t delta_cca_ed_high_80th_5g;
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    int8_t delta_cca_ed_high_20th_6g;
    int8_t delta_cca_ed_high_40th_6g;
    int8_t delta_cca_ed_high_80th_6g;
#else
    int8_t cca_resv[3]; /* 保留字段:3字节 */
#endif
    uint8_t far_dist_pow_gain_switch;           /* 超远距离功率增益开关 */
    int8_t resv[3];
} mac_customize_rf_front_sru;

typedef struct {
    hal_pwr_fit_para_stru pa2gccka_para[HD_EVENT_RF_NUM];
    hal_pwr_fit_para_stru pa2ga_para[HD_EVENT_RF_NUM];
    hal_pwr_fit_para_stru pa2g40a_para[HD_EVENT_RF_NUM];

    hal_pwr_fit_para_stru pa5ga_para[HD_EVENT_RF_NUM][CUS_5G_BASE_PWR_NUM];
    hal_pwr_fit_para_stru pa5ga_low_para[HD_EVENT_RF_NUM][CUS_5G_BASE_PWR_NUM];
    /* 6G定制化band过多，每个band的4个通道数据在同一项定制化中 */
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    hal_pwr_fit_para_stru pa6ga_para[CUS_6G_BASE_PWR_NUM][HD_EVENT_RF_NUM];
#endif
    cus_dy_cali_dpn_stru dpn_para[HD_EVENT_RF_NUM];
    int16_t extre_point_val[CUS_DY_CALI_NUM_5G_BAND];
    int8_t rsv[2];    /* 保留字段:2字节 */
}mac_cust_dyn_pow_sru;

/* 4字节对齐 */
typedef struct {
    int8_t delt_txpwr_params[CUST_NORMAL_DELTA_POW_RATE_ALGN_BUTT];
    uint8_t upper_upc_5g_params[HD_EVENT_RF_NUM];
    uint8_t backoff_pow_5g_params[HD_EVENT_RF_NUM];
    uint8_t dsss_low_pow_amend_2g[HD_EVENT_RF_NUM];
    uint8_t ofdm_low_pow_amend_2g[HD_EVENT_RF_NUM];

    uint8_t txpwr_base_2g_params[HD_EVENT_RF_NUM][CUS_2G_BASE_PWR_NUM];
    uint8_t txpwr_base_5g_params[HD_EVENT_RF_NUM][CUS_5G_BASE_PWR_NUM];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    uint8_t txpwr_base_6g_params[HD_EVENT_RF_NUM][CUS_6G_BASE_PWR_NUM];
#endif
    int8_t delt_txpwr_base_params[WLAN_CALI_BAND_BUTT][HD_EVENT_RF_NUM]; /* MIMO相对SISO的base power差值 */
    /* FCC/CE边带功率定制项 */
    hal_cfg_custom_fcc_txpwr_limit_stru fcc_ce_param[HD_EVENT_RF_NUM];
    /* SAR CTRL */
    wlan_cust_sar_cfg_stru sar_ctrl_params[CUS_SAR_LVL_NUM];
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    /* TAS CTRL */
    uint8_t tas_ctrl_params[WLAN_CALI_BAND_BUTT][HD_EVENT_RF_NUM];
#endif
    wlan_cust_2g_ru_pow_stru  full_bw_ru_pow_2g[WLAN_BW_CAP_80M];
    wlan_cust_5g_ru_pow_stru  full_bw_ru_pow_5g[WLAN_BW_CAP_BUTT];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    wlan_cust_6g_ru_pow_stru  full_bw_ru_pow_6g[WLAN_BW_CAP_BUTT];
#endif
    wlan_cust_2g_tpc_ru_pow_stru tpc_tb_ru_2g_max_pow[HAL_POW_RF_SEL_CHAIN_BUTT];
    wlan_cust_5g_tpc_ru_pow_stru tpc_tb_ru_5g_max_pow[HAL_POW_RF_SEL_CHAIN_BUTT];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    wlan_cust_6g_tpc_ru_pow_stru tpc_tb_ru_6g_max_pow[HAL_POW_RF_SEL_CHAIN_BUTT];
#endif
    uint8_t tpc_tb_ru_max_power[WLAN_CALI_BAND_BUTT];
    uint8_t rsv[2];    /* 保留字段:2字节 */

    uint32_t fcc_ce_max_pwr_for_5g_high_band;
}wlan_cust_pow_stru;

typedef struct {
    wlan_stream_type_enum_uint8 stream_type;
    wlan_cali_band_enum_uint8 wlan_band;
    wlan_bandwidth_type_enum_uint8 band_width;
    oal_bool_enum_uint8 is_master;
    wlan_bus_type_enum_uint8 bus_type;
    wlan_d2h_cali_data_type_enum_uint8 data_type;
    uint8_t cali_run_times;
    uint8_t resv;
}wlan_rf_cali_cfg_stru;

/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */

static inline uint8_t wlan_rf_cali_div_cali_2g_band_idx(wlan_rf_cali_type_enum_uint8 rf_cali_type,
    wlan_bandwidth_type_enum_uint8 bw, uint16_t center_freq)
{
    uint8_t subband_idx;
    uint16_t sub_band_len = WLAN_2G_CALI_BAND_NUM;
    wlan_cali_div_band_stru sub_band_range_2g[] = { { 2412, 2432 }, { 2437, 2457 }, { 2462, 2484 } };

    for (subband_idx = 0; subband_idx < sub_band_len; subband_idx++) {
        if (wlan_cali_in_valid_range(center_freq, sub_band_range_2g[subband_idx].center_freq_start,
            sub_band_range_2g[subband_idx].center_freq_end)) {
            break;
        }
    }

    /* 当配置异常时，本地配置为band1 */
    subband_idx = (subband_idx >= sub_band_len) ? 0x1 : subband_idx;
    return subband_idx;
}


static inline uint8_t wlan_rf_cali_div_cali_5g_band_idx(wlan_rf_cali_type_enum_uint8 rf_cali_type,
    wlan_bandwidth_type_enum_uint8 bw, uint16_t center_freq)
{
    uint8_t subband_idx;
    wlan_cali_div_band_stru *sub_band_range = NULL;
    uint16_t sub_band_len;
    wlan_cali_div_band_stru sub_band_range_5g[] = {
        { 4920, 4980 }, { 5170, 5240 }, { 5260, 5320 }, { 5500, 5560 }, { 5580, 5640 }, { 5660, 5720 }, { 5745, 5825 },
    };
    wlan_cali_div_band_stru sub_band_160m_range[] = { { 4920, 5320 }, { 5500, 5825 } };

    if (((rf_cali_type >= WLAN_RF_CALI_RX_DC && rf_cali_type <= WLAN_RF_CALI_DPD_RX_DC) ||
        (rf_cali_type >= WLAN_RF_CALI_TX_LO && rf_cali_type <= WLAN_RF_CALI_DPD)) && (bw == WLAN_BANDWIDTH_160)) {
        sub_band_range = sub_band_160m_range;
        sub_band_len = WLAN_5G_160M_CALI_BAND_NUM;
    } else {
        sub_band_range = sub_band_range_5g;
        sub_band_len = WLAN_5G_20M_CALI_BAND_NUM;
    }

    for (subband_idx = 0; subband_idx < sub_band_len; subband_idx++) {
        if (wlan_cali_in_valid_range(center_freq, sub_band_range[subband_idx].center_freq_start,
            sub_band_range[subband_idx].center_freq_end)) {
            break;
        }
    }

    /* 当配置异常时，本地配置为band1 */
    subband_idx = (subband_idx >= sub_band_len) ? 0x1 : subband_idx;
    return subband_idx;
}


static inline uint8_t wlan_rf_cali_div_cali_6g_band_idx(wlan_rf_cali_type_enum_uint8 rf_cali_type,
    wlan_bandwidth_type_enum_uint8 bw, uint16_t center_freq)
{
    uint8_t subband_idx;
    wlan_cali_div_band_stru *sub_band_range = NULL;
    uint16_t sub_band_len;
    wlan_cali_div_band_stru sub_band_range_6g[] = {
        { 5845, 5925 }, { 5945, 6015 }, { 6025, 6095 }, { 6105, 6175 }, { 6185, 6255 }, { 6265, 6335 },
        { 6345, 6415 }, { 6425, 6495 }, { 6505, 6575 }, { 6585, 6655 }, { 6665, 6735 }, { 6745, 6815 },
        { 6825, 6895 }, { 6905, 6975 }, { 6985, 7055 },
    };
    wlan_cali_div_band_stru sub_band_160m_range[] = {
        { 5945, 6095 }, { 6105, 6255 }, { 6265, 6415 }, { 6425, 6575 }, { 6585, 6735 }, { 6745, 6895 }, { 6905, 7055 },
    };

    if (((rf_cali_type >= WLAN_RF_CALI_RX_DC && rf_cali_type <= WLAN_RF_CALI_DPD_RX_DC) ||
        (rf_cali_type >= WLAN_RF_CALI_TX_LO && rf_cali_type <= WLAN_RF_CALI_DPD)) && (bw == WLAN_BANDWIDTH_160)) {
        sub_band_range = sub_band_160m_range;
        sub_band_len = WLAN_6G_160M_CALI_BAND_NUM;
    } else {
        sub_band_range = sub_band_range_6g;
        sub_band_len = WLAN_6G_20M_CALI_BAND_NUM;
    }

    for (subband_idx = 0; subband_idx < sub_band_len; subband_idx++) {
        if (wlan_cali_in_valid_range(center_freq, sub_band_range[subband_idx].center_freq_start,
            sub_band_range[subband_idx].center_freq_end)) {
            break;
        }
    }

    /* 当配置异常时，本地配置为band1 */
    subband_idx = (subband_idx >= sub_band_len) ? 0x1 : subband_idx;
    return subband_idx;
}

static inline wlan_cali_band_enum_uint8 wlan_get_cali_band(mac_channel_stru *channel_info)
{
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    if ((channel_info->en_band == WLAN_BAND_5G) && (channel_info->ext6g_band)) {
        return WLAN_CALI_BAND_6G;
    }
#endif
    return channel_info->en_band;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hal_cali.h */
