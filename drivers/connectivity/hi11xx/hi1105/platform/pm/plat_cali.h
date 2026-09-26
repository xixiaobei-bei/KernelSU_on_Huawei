

#ifndef __PLAT_CALI_H__
#define __PLAT_CALI_H__

/* 头文件包含 */
#include "plat_type.h"
#include "oal_types.h"
#include "oal_util.h"
#include "1106/wlan_cali_1106.h"

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
#define OAL_DOUBLE_CALI_DATA_STRU_LEN (OAL_SINGLE_CALI_DATA_STRU_LEN)

/* 1105wifi校准buf长度 */
#define HI1105_CALI_DATA_BUF_LEN             0xb380
#define HI1105_SINGLE_CHAN_CALI_DATA_BUF_LEN (HI1105_CALI_DATA_BUF_LEN >> 1)

/* 1105校准结构体大小 */
#define OAL_SOLO_CALI_DATA_STRU_LEN (HI1105_CALI_DATA_BUF_LEN + 4)
#define OAL_MIMO_CALI_DATA_STRU_LEN (OAL_SOLO_CALI_DATA_STRU_LEN)

/* 1106wifi校准buf长度 */
#define HI1106_MIMO_CALI_DATA_STRU_LEN (sizeof(wlan_cali_data_para_stru))

/* 全局变量定义 */
extern uint32_t g_cali_update_channel_info;
extern uint8_t g_netdev_is_open;

/* STRUCT 定义 */
typedef struct {
    uint16_t us_analog_rxdc_cmp;
    uint16_t us_digital_rxdc_cmp_i;
    uint16_t us_digital_rxdc_cmp_q;
    uint8_t auc_reserve[2]; // 2字节保留对齐
} oal_rx_dc_comp_val_stru;

typedef struct {
    uint16_t us_txdc_cmp_i;
    uint16_t us_txdc_cmp_q;
} oal_txdc_comp_val_stru;

typedef struct {
    uint8_t uc_ppf_val;
    uint8_t auc_reserve[3]; // 3字节保留对齐
} oal_ppf_comp_val_stru;

typedef struct {
    uint16_t us_txiq_cmp_p;
    uint16_t us_txiq_cmp_e;
} oal_txiq_comp_val_stru;

typedef struct {
    uint16_t ul_cali_time;
    uint16_t bit_temperature : 3,
               uc_5g_chan_idx1 : 5,
               uc_5g_chan_idx2 : 5,
               en_update_bt : 3;
} oal_update_cali_channel_stru;

typedef struct {
    uint32_t ul_wifi_2_4g_only;
    uint32_t ul_excep_reboot;
    uint32_t ul_reserve[OAL_CALI_PARAM_ADDITION_LEN];
} oal_cali_param_addition_stru;

/* 函数声明 */
extern int32_t get_cali_count(uint32_t *count);
extern int32_t get_bfgx_cali_data(uint8_t *buf, uint32_t *len, uint32_t buf_len);
extern void *get_cali_data_buf_addr(void);
extern uint64_t get_cali_data_buf_phy_addr(void);
extern int32_t cali_data_buf_malloc(void);
extern void cali_data_buf_free(void);

typedef struct {
    char *name;
    int32_t init_value;
} bfgx_ini_cmd;

/* 以下5个宏定义，如果要修改长度，需要同步修改device的宏定义 */
#define BFGX_BT_CALI_DATA_SIZE             492
#define WIFI_CALI_DATA_FOR_FM_RC_CODE_SIZE 20
#define BFGX_NV_DATA_SIZE                  128
#define BFGX_BT_CUST_INI_SIZE              512
#define WIFI_CALI_DATA_FOR_BT              896
/* 考虑结构体总体长度考虑SDIO下载长度512对齐特性，这里长度为2048 */
typedef struct {
    uint8_t auc_bfgx_data[BFGX_BT_CALI_DATA_SIZE];
    uint8_t auc_wifi_rc_code_data[WIFI_CALI_DATA_FOR_FM_RC_CODE_SIZE];
    uint8_t auc_nv_data[BFGX_NV_DATA_SIZE];
    uint8_t auc_bt_cust_ini_data[BFGX_BT_CUST_INI_SIZE];
    uint8_t auc_wifi_cali_for_bt_data[WIFI_CALI_DATA_FOR_BT];
} bfgx_cali_data_stru;

#define BFGX_CALI_DATA_BUF_LEN (sizeof(bfgx_cali_data_stru))

extern struct completion g_cali_recv_done;

int32_t bfgx_customize_init(void);
void *bfgx_get_cali_data_buf(uint32_t *pul_len);
void *wifi_get_bfgx_rc_data_buf_addr(uint32_t *pul_len);
void *wifi_get_bt_cali_data_buf(uint32_t *pul_len);
int32_t bfgx_cali_data_init(void);

#endif /* end of plat_cali.h */
