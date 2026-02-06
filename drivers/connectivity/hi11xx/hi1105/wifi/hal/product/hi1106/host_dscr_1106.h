

#ifndef __HOST_DSCR_1106_H__
#define __HOST_DSCR_1106_H__

#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "hal_common.h"
#include "host_hal_device.h"
#include "oam_ext_if.h"
#include "mac_device.h"
#include "mac_user.h"
#include "securec.h"
#include "pcie_linux.h"

#define HAL_RX_MSDU_NORMAL_RING_NUM0 256
#define HAL_RX_MSDU_NORMAL_RING_NUM1 1024
#define LARGE_NETBUF_SIZE            1580
#define SMALL_NETBUF_SIZE            1500
#define ALRX_NETBUF_SIZE             4112

#define HAL_RX_MSDU_SMALL_RING_NUM0 512
#define HAL_RX_MSDU_SMALL_RING_NUM1 1024
#define HAL_HOST_MONITOR_COUNT_2G   128
#define HAL_HOST_MONITOR_COUNT_5G   256
#define HAL_RX_ENTRY_SIZE             8
#define HAL_RX_DSCR_LEN              48

/* 为了防止rx complete ring overrun, complete ring的深度设置为最大值 */
#define HAL_RX_COMPLETE_RING_MAX 1024

#define HAL_BA_INFO_COUNT 64

typedef struct {
    uint32_t bit_user_id     : 12,
             bit_tid         : 4,
             bit_tx_ba_ssn   : 12,
             bit_resv        : 3,
             bit_ba_info_vld : 1;
    uint32_t reserved;
    uint32_t ba_bitmap[HAL_BA_BITMAP_SIZE];
} hi1106_tx_ba_info_dscr_stru;

#define BA_INFO_DATA_SIZE sizeof(hi1106_tx_ba_info_dscr_stru)

typedef struct {
    uint32_t ppdu_desc_host_addr_lsb; /* ppdu desc addr仅硬件使用 */
    uint32_t ppdu_desc_host_addr_msb;
    uint32_t pn_lsb;
    uint32_t pn_msb   : 16,
             seq_num  : 12,
             pn_vld   : 1,
             sn_vld   : 1,
             resv1    : 2;
    uint32_t tx_count : 8,
             resv2    : 24;
    uint32_t resv3;
} hi1106_tx_msdu_dscr_stru;

/* 接收描述符定义 */
typedef struct tag_hi1106_rx_mpdu_desc_stru {
    /* word0~1 */
    uint32_t  ppdu_host_buf_addr_lsb;
    uint32_t  ppdu_host_buf_addr_msb;

    /* word2 */
    uint32_t  bit_start_seqnum            :   12;
    uint32_t  bit_mcast_bcast             :   1;
    uint32_t  bit_eosp                    :   1;
    uint32_t  bit_is_amsdu                :   1;
    uint32_t  bit_first_sub_msdu          :   1;
    uint32_t  bit_sub_msdu_num            :   8;
    uint32_t  bit_is_ampdu                :   1;
    uint32_t  bit_ba_session_vld          :   1;
    uint32_t  bit_bar_flag                :   1;
    uint32_t  bit_reserved                :   1;
    uint32_t  bit_rx_status               :   4;

    /* word3 */
    uint32_t  bit_process_flag            :   3;
    uint32_t  bit_reserved1               :   1;
    uint32_t  bit_release_start_seqnum    :   12;
    uint32_t  bit_release_end_seqnum      :   12;
    uint32_t  bit_reserved2               :   3;
    uint32_t  bit_release_is_valid        :   1;

    /* word4 */
    uint32_t  pn_lsb;

    /* word5 */
    uint32_t  bit_pn_msb                  :   16;
    uint32_t  bit_frame_control           :   16;

    /* word6 */
    uint32_t  bit_user_id                 :   12;
    uint32_t  bit_band_id                 :   2;
    uint32_t  bit_reserved3               :   2;
    uint32_t  bit_vap_id                  :   8;
    uint32_t  bit_tid                     :   4;
    uint32_t  bit_cipher_type             :   4;

    /* word7 */
    uint32_t  bit_fragment_num            :   4;
    uint32_t  bit_sequence_num            :   12;
    uint32_t  bit_packet_len              :   16;

    /* word8 */
    uint32_t  bit_dst_user_id             :   12;
    uint32_t  bit_dst_band_id             :   2;
    uint32_t  bit_ptlcs_valid             :   1;
    uint32_t  bit_ptlcs_pass              :   1;
    uint32_t  bit_dst_vap_id              :   8;
    uint32_t  bit_frame_format            :   4;
    uint32_t  bit_ipv4cs_valid            :   1;
    uint32_t  bit_ipv4cs_pass             :   1;
    uint32_t  bit_reserved6               :   1;
    uint32_t  bit_dst_is_valid            :   1;

    /* word9 */
    uint32_t  bit_ptlcs_val               :   16;
    uint32_t  bit_ipv4cs_val              :   16;

    /* word 10~11 */
    uint32_t  next_sub_msdu_addr_lsb;
    uint32_t  next_sub_msdu_addr_msb;
} hi1106_rx_mpdu_desc_stru;

typedef struct {
    uint32_t base_lsb;
    uint32_t base_msb;
    uint32_t size;
    uint32_t csi_pro_reg;
    uint32_t chn_set;
    uint32_t rx_ctrl_reg;
    uint32_t white_addr_msb;
    uint32_t white_addr_lsb;
    uint32_t location_info;
    uint32_t csi_info_lsb;
    uint32_t csi_info_msb;
} hal_csi_regs_info;

#pragma pack(push, 1)

struct  hi1106_ftm_word0 {
    uint32_t bit_freq_bw : 8;       /* [7..0]    */
    uint32_t bit_rpt_info_len : 16; /* [23..8]  */
    uint32_t bit_frame_type : 6;    /* [29..24]  */
    uint32_t bit_dbg_mode : 1;      /* [30]  */
    uint32_t bit_reserved : 1;      /* [31]  */
    int8_t   rssi_lltf_ch[HAL_HOST_MAX_RF_NUM];
} __OAL_DECLARE_PACKED;
typedef struct  hi1106_ftm_word0 hi1106_ftm_word0_stru;

struct hi1106_ftm_word1 {
    int8_t   snr_ant[HAL_HOST_MAX_RF_NUM];
    int8_t   rssi_hltf[HAL_HOST_MAX_RF_NUM];
} __OAL_DECLARE_PACKED;
typedef struct hi1106_ftm_word1 hi1106_ftm_word1_stru;

typedef struct {
    uint16_t agc_code_ant : 8; /* [8..0]    */
    uint16_t reserved : 8;        /* [15..9]  */
} hi1106_ftm_agc_code_stru;

struct hi1106_ftm_word2 {
    hi1106_ftm_agc_code_stru st_agc_code_ant[HAL_HOST_MAX_RF_NUM];
} __OAL_DECLARE_PACKED;
typedef struct hi1106_ftm_word2 hi1106_ftm_word2_stru;

struct hi1106_ftm_word3 {
    hi1106_ftm_agc_code_stru nd_agc_code_ant[HAL_HOST_MAX_RF_NUM];
} __OAL_DECLARE_PACKED;
typedef struct hi1106_ftm_word3 hi1106_ftm_word3_stru;

struct hi1106_ftm_word4 {
    uint64_t bit_phase_incr : 20;   /* [19..0]    */
    uint64_t bit_protocol_mode : 4; /* [23..20]  */
    uint64_t bit_nss_mcs_rate : 6;  /* [29..24]  */
    uint64_t bit_bf_flag : 1;       /* [30]  */
    uint64_t bit_reserved : 1;      /* [31]  */
    uint64_t bit_frm_ctrl : 16;     /* [47..32]  */
    uint64_t bit_vap_index : 8;     /* [55..48]  */
    uint64_t bit_dialog_token : 8;  /* [63..56]  */
} __OAL_DECLARE_PACKED;
typedef struct hi1106_ftm_word4 hi1106_ftm_word4_stru;

struct hi1106_ftm_word5 {
    uint64_t ra : 48;           /* [47..0]    */
    uint64_t bit_reserved : 16; /* [63..48]  */
} __OAL_DECLARE_PACKED;
typedef struct hi1106_ftm_word5 hi1106_ftm_word5_stru;

struct hi1106_ftm_word6 {
    uint64_t ta : 48;           /* [47..0]    */
    uint64_t bit_reserved : 16; /* [63..48]  */
} __OAL_DECLARE_PACKED;
typedef struct hi1106_ftm_word6 hi1106_ftm_word6_stru;

struct hi1106_ftm_word7 {
    uint64_t timestamp : 48;     /* [47..0]    */
    uint64_t bit_reserved : 16;  /* [63..48]  */
} __OAL_DECLARE_PACKED;
typedef struct hi1106_ftm_word7 hi1106_ftm_word7_stru;

struct hi1106_ftm_word8 {
    uint64_t tx_time : 48;      /* [47..0]    */
    uint64_t bit_reserved : 16; /* [63..48]  */
} __OAL_DECLARE_PACKED;
typedef struct hi1106_ftm_word8 hi1106_ftm_word8_stru;

struct hi1106_ftm_word9 {
    uint64_t rx_time : 48;      /* [47..0]    */
    uint64_t bit_reserved : 16; /* [63..48]  */
} __OAL_DECLARE_PACKED;
typedef struct hi1106_ftm_word9 hi1106_ftm_word9_stru;

struct hi1106_ftm_word10 {
    uint64_t bit_intp_time : 5;     /* [4..0]    */
    uint64_t bit_reserved_0 : 3;    /* [7..5]  */
    uint64_t bit_ftm_done_flag : 1; /* [8]  */
    uint64_t bit_init_resp_flag : 1; /* [9]  */
    uint64_t bit_11az_flag : 1;     /* [10]  */
    uint64_t bit_reserved_1 : 53;   /* [63..11]  */
} __OAL_DECLARE_PACKED;
typedef struct hi1106_ftm_word10 hi1106_ftm_word10_stru;

struct hi1106_ftm_dscr {
    hi1106_ftm_word0_stru word0;
    hi1106_ftm_word1_stru word1;
    hi1106_ftm_word2_stru word2;
    hi1106_ftm_word3_stru word3;
    hi1106_ftm_word4_stru word4;
    hi1106_ftm_word5_stru word5;
    hi1106_ftm_word6_stru word6;
    hi1106_ftm_word7_stru word7;
    hi1106_ftm_word8_stru word8;
    hi1106_ftm_word9_stru word9;
    hi1106_ftm_word10_stru word10;
} __OAL_DECLARE_PACKED;
typedef struct hi1106_ftm_dscr hi1106_ftm_dscr_stru;

struct hi1106_csi_ppu {
    hi1106_ftm_word0_stru word0;
    hi1106_ftm_word1_stru word1;
    hi1106_ftm_word2_stru word2;
    hi1106_ftm_word3_stru word3;
    hi1106_ftm_word4_stru word4;
    hi1106_ftm_word5_stru word5;
    hi1106_ftm_word6_stru word6;
    hi1106_ftm_word7_stru word7;
} __OAL_DECLARE_PACKED;
typedef struct hi1106_csi_ppu hi1106_csi_ppu_stru;

#pragma pack(pop)

uint32_t hi1106_rx_host_start_dscr_queue(uint8_t hal_dev_id);
int32_t hi1106_rx_host_stop_dscr_queue(uint8_t hal_dev_id);
void hi1106_tx_ba_info_dscr_get(uint8_t *ba_info_data, uint32_t size, hal_tx_ba_info_stru *tx_ba_info);
void hi1106_tx_msdu_dscr_info_get(oal_netbuf_stru *netbuf, hal_tx_msdu_dscr_info_stru *tx_msdu_info);
void hi1106_host_tx_intr_init(uint8_t hal_dev_id);
int32_t hi1106_rx_host_init_dscr_queue(uint8_t hal_dev_id);
void hi1106_host_rx_add_buff(hal_host_device_stru *hal_device, uint8_t en_queue_num);
uint32_t hi1106_rx_mpdu_que_len(hal_host_device_stru *pst_device);
int32_t hi1106_rx_get_node_idx(hal_rx_nodes *nodes, dma_addr_t dma_addr);
oal_netbuf_stru *hi1106_rx_get_node(hal_rx_nodes *nodes, uint32_t idx);
oal_netbuf_stru *hi1106_rx_get_next_sub_msdu(hal_host_device_stru *hal_device, oal_netbuf_stru *netbuf);
uint32_t hi1106_host_rx_reset_smac_handler(frw_event_mem_stru *event_mem);
void hi1106_host_rx_msdu_list_build(hal_host_device_stru *hal_device, oal_netbuf_stru *netbuf);

#endif

