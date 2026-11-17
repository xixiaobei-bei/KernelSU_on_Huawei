

#ifndef __HOST_IRQ_1106_H__
#define __HOST_IRQ_1106_H__
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "hal_common.h"

typedef union {
    /* Define the struct bits */
    struct {
        uint32_t    rpt_host_intr_tx_complete : 1;       /* [0]  */
        uint32_t    rpt_host_intr_ba_info : 1;           /* [1]  */
        uint32_t    rpt_host_intr_rx_complete : 1;       /* [2]  */
        uint32_t    rpt_host_intr_norm_ring_empty : 1;   /* [3]  */
        uint32_t    rpt_host_intr_small_ring_empty : 1;  /* [4]  */
        uint32_t    rpt_host_intr_data_ring_overrun : 1; /* [5]  */
        uint32_t    rpt_host_intr_location_complete : 1; /* [6]  */
        uint32_t    rpt_host_intr_msdu_table_rptr_overtake : 1;   /* [7]  */
        uint32_t    rpt_host_intr_ba_info_ring_overrun : 1;       /* [8]  */
        uint32_t    rpt_host_intr_ba_win_exceed : 1;              /* [9]  */
        uint32_t    rpt_host_intr_rx_ppdu_desc : 1;               /* [10]  */
        uint32_t    rpt_host_intr_location_intr_fifo_overrun : 1; /* [11]  */
        uint32_t    rpt_host_intr_msdu_table_rptr_mismatch : 1;   /* [12]  */
        uint32_t    rpt_host_intr_common_timer : 1;               /* [13]  */
        uint32_t    reserved_0            : 18;                   /* [31..14]  */
    } bits;
    /* Define an unsigned member */
    uint32_t        u32;
} host_intr_status_union;

typedef union {
    /* Define the struct bits */
    struct {
        uint32_t    rpt_intr_radar_det    : 1;    /* [0]  */
        uint32_t    rpt_intr_micro_wave_det : 1;  /* [1]  */
        uint32_t    rpt_psd_int : 1;              /* [2]  */
        uint32_t    rpt_txrdy_time_out_int : 1;   /* [3]  */
        uint32_t    rpt_lmi_mem_have_no_space_int : 1;     /* [4]  */
        uint32_t    rpt_rx_freq_pwr_ack_timeout_int  : 1;  /* [5]  */
        uint32_t    rpt_p2m_tx_oversifs_int : 1;        /* [6]  */

        uint32_t    rpt_hsi_master_abnormal_rap_int : 1;
        uint32_t    rpt_hsi_master_abnormal_3ch_int : 1;
        uint32_t    rpt_hsi_master_abnormal_2ch_int : 1;
        uint32_t    rpt_hsi_master_abnormal_1ch_int : 1;
        uint32_t    rpt_hsi_master_abnormal_0ch_int : 1;
        uint32_t    reserved_0            : 20; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    uint32_t    u32;
} phy_intr_rpt_union;

void hi1106_host_chip_irq_init(void);
void hi1106_irq_host_mac_isr(hal_host_device_stru *pst_hal_dev);
#endif
