

#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "hal_common.h"
#include "wlan_chip.h"

#ifndef __HOST_HAL_DEVICE_H__
#define __HOST_HAL_DEVICE_H__
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_LINUX_TEST
typedef unsigned long long dma_addr_t;

enum dma_data_direction {
    DMA_BIDIRECTIONAL = 0,
    DMA_TO_DEVICE = 1,
    DMA_FROM_DEVICE = 2,
    DMA_NONE = 3,
};
#endif

typedef enum {
    HAL_COMMON_TIMER_MODE_ONE_SHOT = 0,
    HAL_COMMON_TIMER_MODE_CIRCULAR,

    HAL_COMMON_TIMER_MODE_BUTT,
} hal_common_timer_mode_enum;
typedef uint8_t hal_common_timer_mode_enum_uint8;

typedef enum {
    HAL_COMMON_TIMER_UNIT_Point1US = 0,
    HAL_COMMON_TIMER_UNIT_1US,
    HAL_COMMON_TIMER_UNIT_RESERVED,
    HAL_COMMON_TIMER_UNIT_1MS,

    HAL_COMMON_TIMER_UNIT_BUTT,
} hal_common_timer_unit_enum;
typedef uint8_t hal_common_timer_unit_enum_uint8;

typedef struct  {
    uint32_t base_lsb;
    uint32_t base_msb;
    uint32_t len;
    uint32_t size;
    uint32_t wptr_reg;
    uint32_t rptr_reg;
    uint32_t cfg_wptr_reg;
    uint32_t cfg_rptr_reg;
} hal_ring_mac_regs_info;


#define RX_NODES                     2048
#define HMAC_MSDU_RX_MAX_BUFF_NUM    1024

#define HMAC_MSDU_RX_MAX_PROCESS_NUM 512
/* 接收PING-PANG队列 */
#define HAL_MPDU_RXQ_NUM             2
#define HAL_HOST_RX_NORMAL_BUFF_NUM  512
#define HAL_HOST_RX_SMALL_BUFF_NUM   64
#define HAL_HOST_BAINFO_NUM          64
#define HAL_HOST_CMP_RING_NUM       (HAL_HOST_RX_NORMAL_BUFF_NUM + HAL_HOST_RX_SMALL_BUFF_NUM)
#define HAL_RX_MSDU_ADDR_BUCKET_NUM  HMAC_MSDU_RX_MAX_BUFF_NUM
#define HAL_RX_MSDU_ADDR_NODE_NUM   (HMAC_MSDU_RX_MAX_BUFF_NUM * 4)
#define RXQ_NUM                      2
#define HAL_MAX_VAP_NUM              4
#define HAL_MAX_LUT                  WLAN_ASSOC_USER_MAX_NUM
#define CSI_INFO_LEN 10

typedef struct {
    oal_netbuf_stru *netbuf;
    dma_addr_t       dma_addr;
    uint32_t         allocated;
    uint32_t         resv;
} hal_rx_node;

typedef struct  {
    hal_rx_node             *tbl;
    uint32_t                 tbl_size;
    uint32_t                 last_idx;
    enum dma_data_direction  dma_dir;
} hal_rx_nodes;

typedef struct {
    uint32_t nfree_ring_add;
    uint32_t sfree_ring_add;
    uint32_t comp_ring_get;
} hal_rx_buf_statics;


typedef struct {
    /* ring的类型: free ring或者complete ring */
    uint8_t ring_type;
    uint8_t ring_subtype;
    uint16_t resv;
    /* ring的深度 */
    uint16_t entries;
    /* ring中每个元素的大小,例如, BA info元素大小64字节 */
    uint16_t entry_size;
    /* 软件操作的 */
    union {
        uint16_t read_ptr; /* read index */
        struct {
            uint16_t bit_read_ptr  : 15,
                       bit_wrap_flag : 1;
        } st_read_ptr;
    } un_read_ptr;

    union {
        uint16_t write_ptr; /* write index */
        struct {
            uint16_t bit_write_ptr : 15,
                       bit_wrap_flag : 1;
        } st_write_ptr;
    } un_write_ptr;

    /* ring基地址 */
    uint64_t *p_entries;
    uint64_t devva;

    /* 寄存器地址 */
    volatile uint64_t write_ptr_reg;
    volatile uint64_t read_ptr_reg;
    volatile uint64_t ring_size_reg;
    volatile uint64_t ring_base_reg;
    volatile uint64_t cfg_write_ptr_reg;
    volatile uint64_t cfg_read_ptr_reg;
    volatile uint64_t base_lsb;
    volatile uint64_t base_msb;
    volatile uint64_t len_reg;
} hal_host_ring_ctl_stru;

typedef enum {
    HAL_RING_TYPE_L_F,
    /* Free ring: 软件操作write ptr, HW操作read ptr */
    HAL_RING_TYPE_S_F,
    /* Complete ring: 软件操作read ptr, HW操作write ptr */
    HAL_RING_TYPE_COMP,
    /* Tx complete ba ring: 软件操作read ptr, HW操作write ptr */
    TX_BA_INFO_RING,

    HAL_RING_ID_BUTT
} hal_host_ring_id_enum;

typedef struct {
    volatile uint64_t irq_status_reg;
    volatile uint64_t irq_status_mask_reg;
    volatile uint64_t irq_clr_reg;
} hal_host_mac_regs;


typedef struct {
    volatile uint64_t irq_status_reg;
    volatile uint64_t irq_status_mask_reg;
    volatile uint64_t irq_clr_reg;
} hal_host_phy_regs;

typedef struct {
    volatile uint8_t inited;
    /* 基地址 */
    void *rbase_vaddr;
    dma_addr_t rbase_dma_addr;
    uint64_t devva;
    /* 寄存器地址 */
    volatile uint64_t base_lsb;
    volatile uint64_t base_msb;
    volatile uint64_t size;
    volatile uint64_t csi_pro_reg;
    volatile uint64_t chn_set;
    volatile uint64_t rx_ctrl_reg;
    volatile uint64_t white_addr_msb;
    volatile uint64_t white_addr_lsb;
    volatile uint64_t location_info;
    volatile uint64_t csi_info_lsb;
    volatile uint64_t csi_info_msb;
} hal_host_csi_regs;

typedef struct {
    /* MAC通用定时器 */
    uint8_t timer_id;                            /* 定时器id */
    volatile uint64_t mask_reg_addr;             /* mask寄存器 */
    volatile uint64_t timer_ctrl_addr;           /* timer ctrl寄存器 */
    volatile uint64_t timer_val_addr;            /* tmer val超时值 */
    oal_bool_enum_uint8 timer_en;                /* 是否使能定时器 */
    oal_bool_enum_uint8 timer_mask_en;           /* 是否屏蔽中断 */
    hal_common_timer_mode_enum_uint8 timer_mode; /* 定时器触发方式 */
    hal_common_timer_unit_enum_uint8 timer_unit; /* 定时器精度 */
    uint32_t common_timer_val;                   /* 通用定时器定时值配置 */
} hal_mac_common_timer;

typedef struct {
    volatile uint8_t inited;
    /* 基地址 */
    void *rbase_vaddr;
    dma_addr_t rbase_dma_addr;
    uint64_t devva;
    /* 寄存器地址 */
    volatile uint64_t base_lsb;
    volatile uint64_t base_msb;
    volatile uint64_t ftm_pro_reg;
    volatile uint64_t ftm_cfg_reg;
    volatile uint64_t ftm_cali_reg;
    volatile uint64_t fifo_force_reg;
    volatile uint64_t rx_ctrl_reg;
    volatile uint64_t white_addr_msb;
    volatile uint64_t white_addr_lsb;
    volatile uint64_t ftm_info_lsb;
    volatile uint64_t ftm_info_msb;
    volatile uint64_t ftm_phy_bw_mode;
} hal_host_ftm_regs;

/* device 申请hal vap 同步给hmac */
typedef struct {
    uint8_t hal_vap_valid;
    uint8_t mac_vap_id;
} hal_vap_status_info;

typedef struct {
    uint8_t user_valid;
    uint16_t user_id;
} hal_user_status_info;

typedef struct {
    uint32_t                          rx_ppdu_succ_num;
    uint32_t                          rx_normal_mdpu_succ_num;
    uint32_t                          rx_ampdu_succ_num;
    uint32_t                          rx_ppdu_fail_num;
    uint32_t                          rx_delimiter_pass_mdpu_num;
    uint32_t                          rx_fcs_pass_mdpu_num;
    uint32_t                          rx_first_delimiter_err;
    uint32_t                          rx_first_fcs_err;
    int16_t                           s_always_rx_rssi;

    hal_device_always_rx_state_enum   en_al_rx_flag;
    int8_t                            c_rpt_rssi_lltf_ant0;
    int8_t                            c_rpt_rssi_lltf_ant1;
    uint8_t                           auc_resv[3];  /* 3 对齐 */
} mac_al_rx_stru;

typedef struct {
    oal_netbuf_head_stru    list;
    oal_spin_lock_stru      lock;
}hal_host_rx_alloc_list_stru;

typedef struct {
    uint8_t                            chip_id;
    uint8_t                            device_id;      /* 此id用来获取hal device */
    uint8_t                            mac_device_id;  /* 此id用来获取mac dmac hmac device */
    uint8_t                            inited;

    uint32_t                           tx_inited;
    hal_host_mac_regs                  mac_regs;
    hal_host_phy_regs                  phy_regs;
    hal_host_csi_regs                  csi_regs;
    hal_host_ftm_regs                  ftm_regs;
    hal_host_ring_ctl_stru             st_host_rx_normal_free_ring;
    hal_host_rx_alloc_list_stru        host_rx_normal_alloc_list;
    hal_host_ring_ctl_stru             st_host_rx_small_free_ring;
    hal_host_rx_alloc_list_stru        host_rx_small_alloc_list;
    hal_host_ring_ctl_stru             st_host_rx_complete_ring;
    hal_host_ring_ctl_stru             st_host_tx_ba_info_ring;
    hal_rx_nodes                      *rx_nodes;

    hal_rx_buf_statics                 rx_statics;
    oal_spin_lock_stru                 st_free_ring_lock;
    hal_rx_mpdu_que                    st_rx_mpdu;
    hal_vap_status_info                hal_vap_sts_info[HAL_MAX_VAP_NUM];
    hal_user_status_info               user_sts_info[HAL_MAX_LUT];
    mac_al_rx_stru                     st_alrx;
    uint8_t                            rx_q_inited;
    uint8_t                            rx_mode;
} hal_host_device_stru;

typedef struct {
    uint8_t     vap_index;
    uint8_t     csi_data[CSI_INFO_LEN];
    uint16_t    rpt_info_len;
} hmac_csi_info_stru;

#define FTM_MAX_POWER_NUM (32) /* HWMAC上报32个word用于FTM功率相关峰计算 */

typedef union {
    uint32_t reg_value;
    struct {
        uint32_t bit_lsb : 16;
        uint32_t bit_msb : 16;
    } reg;
} hal_ftm_correlation_peak_union;
typedef struct {
    uint8_t   ftm_init_resp_flag : 1;
    uint8_t   ftm_intp_time : 5;
    uint8_t   vap_index;
    uint8_t   ftm_dialog_token;
    uint8_t   ftm_ra[WLAN_MAC_ADDR_LEN];
    uint8_t   ftm_ta[WLAN_MAC_ADDR_LEN];
    uint8_t   ftm_snr_ant[HAL_HOST_MAX_RF_NUM];
    int8_t    ftm_rssi_ant[HAL_HOST_MAX_RF_NUM];
    uint32_t  ftm_phase_incr_ant;
    uint64_t  ftm_rx_time;
    uint64_t  ftm_tx_time;
    uint64_t  ftm_timestamp;
    hal_ftm_correlation_peak_union ftm_power[FTM_MAX_POWER_NUM];
} hal_ftm_info_stru;

typedef struct tag_hal_hal_chip_stru {
    uint8_t                   chip_id;            /* 芯片ID */
    uint8_t                   device_nums;        /* chip下device的数目 */
    uint8_t                   device_id_bitmap;   /* 用于标识device分配情况 (需要提供内部的get和clear，add使用) */
    uint8_t                   resv;
    uint32_t                  chip_version;       /* 芯片版本 */
    hal_host_device_stru      hal_device[WLAN_DEVICE_MAX_NUM_PER_CHIP];
} hal_host_chip_stru;

void hal_host_rx_mpdu_que_pop(hal_host_device_stru *hal_device, oal_netbuf_head_stru *netbuf_head);
uint8_t hal_rx_mpdu_que_is_full(hal_rx_mpdu_que *rx_mpdu);
hal_host_device_stru *hal_get_host_device(uint8_t dev_id);
void hal_host_board_init(void);
void hal_device_reset_rx_res(void);
void hal_set_alrx_status(uint8_t status);
uint8_t hal_device_is_in_ddr_rx(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
