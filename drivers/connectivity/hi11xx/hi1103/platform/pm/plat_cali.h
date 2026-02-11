

#ifndef __PLAT_CALI_H__
#define __PLAT_CALI_H__

/* 头文件包含 */
#include "oal_types.h"
#include "oal_util.h"

#ifdef WIN32
#include "plat_type.h"
#endif

/* 宏定义 */
#define OAL_2G_CHANNEL_NUM            3
#define OAL_5G_20M_CHANNEL_NUM        7
#define OAL_5G_80M_CHANNEL_NUM        7
#define OAL_5G_160M_CHANNEL_NUM       2
#define OAL_5G_CHANNEL_NUM            (OAL_5G_20M_CHANNEL_NUM + OAL_5G_80M_CHANNEL_NUM + OAL_5G_160M_CHANNEL_NUM)
#define OAL_5G_DEVICE_CHANNEL_NUM     7
#define OAL_CALI_HCC_BUF_NUM          3
#define OAL_CALI_HCC_BUF_SIZE         1500
#define OAL_CALI_IQ_TONE_NUM          16
#define OAL_CALI_TXDC_GAIN_LVL_NUM    16 /* tx dc补偿值档位数目 */
#define OAL_BT_RF_FEQ_NUM             79 /* total Rf frequency number */
#define OAL_BT_CHANNEL_NUM            8  /* total Rf frequency number */
#define OAL_BT_POWER_CALI_CHANNEL_NUM 3
#define OAL_BT_NVRAM_DATA_LENGTH      104
#define OAL_BT_NVRAM_NAME             "BTCALNV"
#define OAL_BT_NVRAM_NUMBER           352

#define WIFI_2_4G_ONLY              0x2424
#define SYS_EXCEP_REBOOT            0xC7C7
#define OAL_CALI_PARAM_ADDITION_LEN 8
#define OAL_5G_IQ_CALI_TONE_NUM     8

#define CHECK_5G_ENABLE "radio_cap_0"

/* wifi校准buf长度 */
#define RF_CALI_DATA_BUF_LEN             0x4bb0
#define RF_SINGLE_CHAN_CALI_DATA_BUF_LEN (RF_CALI_DATA_BUF_LEN >> 1)
/* 校准结构体大小 */
#define OAL_SINGLE_CALI_DATA_STRU_LEN (RF_CALI_DATA_BUF_LEN + 4)
#define OAL_DOUBLE_CALI_DATA_STRU_LEN (OAL_SINGLE_CALI_DATA_STRU_LEN << 1)

/* 1105wifi校准buf长度 */
#define HI1105_CALI_DATA_BUF_LEN             0x9db0
#define HI1105_SINGLE_CHAN_CALI_DATA_BUF_LEN (HI1105_CALI_DATA_BUF_LEN >> 1)

/* 1105校准结构体大小 */
#define OAL_SOLO_CALI_DATA_STRU_LEN (HI1105_CALI_DATA_BUF_LEN + 4)
#define OAL_MIMO_CALI_DATA_STRU_LEN (OAL_SOLO_CALI_DATA_STRU_LEN << 1)

/* 全局变量定义 */
extern oal_uint32 cali_update_channel_info;
extern oal_uint8 netdev_is_open_etc;

/* STRUCT 定义 */
typedef struct {
    oal_uint16 us_analog_rxdc_cmp;
    oal_uint16 us_digital_rxdc_cmp_i;
    oal_uint16 us_digital_rxdc_cmp_q;
    oal_uint8 auc_reserve[2];
} oal_rx_dc_comp_val_stru;

typedef struct {
    oal_uint8 upc_ppa_cmp;
    oal_int8 ac_atx_pwr_cmp;
    oal_uint8 dtx_pwr_cmp;
    oal_uint8 auc_reserve[1];
} oal_2G_tx_power_comp_val_stru;

typedef struct {
    oal_uint8 upc_ppa_cmp;
    oal_uint8 upc_mx_cmp;
    oal_int8 ac_atx_pwr_cmp;
    oal_uint8 dtx_pwr_cmp;
} oal_5G_tx_power_comp_val_stru;

typedef struct {
    oal_uint16 us_txdc_cmp_i;
    oal_uint16 us_txdc_cmp_q;
} oal_txdc_comp_val_stru;

typedef struct {
    oal_uint8 uc_ppf_val;
    oal_uint8 auc_reserve[3];
} oal_ppf_comp_val_stru;

typedef struct {
    oal_uint16 us_txiq_cmp_p;
    oal_uint16 us_txiq_cmp_e;
} oal_txiq_comp_val_stru;

typedef struct {
    oal_uint16 ul_cali_time;
    oal_uint16 bit_temperature : 3,
               uc_5g_chan_idx1 : 5,
               uc_5g_chan_idx2 : 5,
               en_update_bt : 3;
} oal_update_cali_channel_stru;

typedef struct {
    oal_uint32 ul_wifi_2_4g_only;
    oal_uint32 ul_excep_reboot;
    oal_uint32 ul_reserve[OAL_CALI_PARAM_ADDITION_LEN];
} oal_cali_param_addition_stru;

/* 函数声明 */
extern oal_int32 get_cali_count_etc(oal_uint32 *count);
extern oal_int32 get_bfgx_cali_data_etc(oal_uint8 *buf, oal_uint32 *len, oal_uint32 buf_len);
extern void *get_cali_data_buf_addr_etc(void);
extern oal_int32 cali_data_buf_malloc_etc(void);
extern void cali_data_buf_free_etc(void);

/* add for hi1103 bfgx */
/* enum定义不能超过BFGX_BT_CUST_INI_SIZE/4 (128) */
typedef enum {
    BFGX_CFG_INI_BT_MAXPOWER = 0,
    BFGX_CFG_INI_BT_EDRPOW_OFFSET,
    BFGX_CFG_INI_BT_BLEPOW_OFFSET,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_REF_NUM,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND1,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND2,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND3,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND4,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND5,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND6,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND7,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND8,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_FRE1,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_FRE2,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_FRE3,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_FRE4,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_FRE5,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_FRE6,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_FRE7,
    BFGX_CFG_INI_BT_CALI_TXPWR_PA_FRE8,
    BFGX_CFG_INI_BT_CALI_BT_TONE_AMP_GRADE,
    BFGX_CFG_INI_BT_RXDC_BAND,
    BFGX_CFG_INI_BT_DBB_SCALING_SATURATION,
    BFGX_CFG_INI_BT_PRODUCTLINE_UPCCODE_SEARCH_MAX,
    BFGX_CFG_INI_BT_PRODUCTLINE_UPCCODE_SEARCH_MIN,
    BFGX_CFG_INI_BT_DYNAMICSARCTRL_BT,
    BFGX_CFG_INI_BT_POWOFFSBT,
    BFGX_CFG_INI_BT_ELNA_2G_BT,
    BFGX_CFG_INI_BT_RXISOBTELNABYP,
    BFGX_CFG_INI_BT_RXGAINBTELNA,
    BFGX_CFG_INI_BT_RXBTEXTLOSS,
    BFGX_CFG_INI_BT_ELNA_ON2OFF_TIME_NS,
    BFGX_CFG_INI_BT_ELNA_OFF2ON_TIME_NS,
    BFGX_CFG_INI_BT_HIPOWER_MODE,
    BFGX_CFG_INI_BT_FEM_CONTROL,
    BFGX_CFG_INI_BT_FEATURE_32K_CLOCK,
    BFGX_CFG_INI_BT_FEATURE_LOG,
    BFGX_CFG_INI_BT_CALI_SWTICH_ALL,
    BFGX_CFG_INI_BT_ANT_NUM_BT,
    BFGX_CFG_INI_BT_POWER_LEVEL_CONTROL,
    BFGX_CFG_INI_BT_COUNTRY_CODE,
    BFGX_CFG_INI_BT_RESERVED1,
    BFGX_CFG_INI_BT_RESERVED2,
    BFGX_CFG_INI_BT_RESERVED3,
    BFGX_CFG_INI_BT_DEDICATED_ANTENNA,
    BFGX_CFG_INI_BT_RESERVED5,
    BFGX_CFG_INI_BT_RESERVED6,
    BFGX_CFG_INI_BT_RESERVED7,
    BFGX_CFG_INI_BT_RESERVED8,
    BFGX_CFG_INI_BT_RESERVED9,
    BFGX_CFG_INI_BT_RESERVED10,

    BFGX_CFG_INI_BUTT
} BFGX_CFG_INI;

typedef struct {
    char *name;
    int32 init_value;
} bfgx_ini_cmd;

/* 以下5个宏定义，如果要修改长度，需要同步修改device的宏定义 */
#define BFGX_BT_CALI_DATA_SIZE             492
#define WIFI_CALI_DATA_FOR_FM_RC_CODE_SIZE 20
#define BFGX_NV_DATA_SIZE                  128
#define BFGX_BT_CUST_INI_SIZE              512
#define WIFI_CALI_DATA_FOR_BT              896
/* 考虑结构体总体长度考虑SDIO下载长度512对齐特性，这里长度为2048 */
typedef struct {
    oal_uint8 auc_bfgx_data[BFGX_BT_CALI_DATA_SIZE];
    oal_uint8 auc_wifi_rc_code_data[WIFI_CALI_DATA_FOR_FM_RC_CODE_SIZE];
    oal_uint8 auc_nv_data[BFGX_NV_DATA_SIZE];
    oal_uint8 auc_bt_cust_ini_data[BFGX_BT_CUST_INI_SIZE];
    oal_uint8 auc_wifi_cali_for_bt_data[WIFI_CALI_DATA_FOR_BT];
} bfgx_cali_data_stru;

#define BFGX_CALI_DATA_BUF_LEN (sizeof(bfgx_cali_data_stru))

extern struct completion cali_recv_done;

extern int32 bfgx_customize_init(void);
extern void *bfgx_get_cali_data_buf(uint32 *pul_len);
extern void *bfgx_get_nv_data_buf(uint32 *pul_len);
extern void *bfgx_get_cust_ini_data_buf(uint32 *pul_len);
extern void *wifi_get_bfgx_rc_data_buf_addr(uint32 *pul_len);
extern void *wifi_get_bt_cali_data_buf(uint32 *pul_len);

#endif /* end of plat_cali.h */
