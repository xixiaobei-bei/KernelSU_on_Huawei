

#ifndef __OAL_ETE_ADDR_H__
#define __OAL_ETE_ADDR_H__

#include "oal_types.h"
#include "soc_reg.h"

/* TX(Host2Device) 4 chans */
typedef enum _ete_tx_chann_type_ {
    ETE_TX_CHANN_0 = 0,
    ETE_TX_CHANN_1 = 1,
    ETE_TX_CHANN_2 = 2,
    ETE_TX_CHANN_3 = 3,
    ETE_TX_CHANN_MAX_NUMS
} ete_tx_chann_type;

/* RX(Device2Host) 5 chans */
typedef enum _ete_rx_chann_type_ {
    ETE_RX_CHANN_0 = 0,
    ETE_RX_CHANN_1 = 1,
    ETE_RX_CHANN_2 = 2,
    ETE_RX_CHANN_3 = 3,
    ETE_RX_CHANN_4 = 4,
    ETE_RX_CHANN_MAX_NUMS
} ete_rx_chann_type;

/* offset not base address */
typedef struct _host_ctrl_tx_chan_addr_ {
    unsigned short tx_ch0_en;   /* 0x400 */
    unsigned short tx_ch0_mode; /* 0x404 */
    unsigned short tx_ch0_pri;
    unsigned short tx_ch0_sr_base_addr_h;
    unsigned short tx_ch0_sr_base_addr;
    unsigned short tx_ch0_sr_entry_num;
    unsigned short tx_ch0_sr_head_ptr;
    unsigned short tx_ch0_sr_tail_ptr;
    unsigned short tx_ch0_sr_afull_th;
    unsigned short tx_ch0_sr_aempty_th;
    unsigned short tx_ch0_dr_buff_size;
    unsigned short tx_ch0_dr_base_addr;
    unsigned short tx_ch0_dr_entry_num;
    unsigned short tx_ch0_dr_head_ptr;
    unsigned short tx_ch0_dr_tail_ptr;
    unsigned short tx_ch0_dr_afull_th;
    unsigned short tx_ch0_dr_aempty_th;
    unsigned short tx_ch0_status;
    unsigned short tx_ch0_done_total_cnt;
} host_ctrl_tx_chan_addr;

typedef struct _host_ctrl_rx_chan_addr_ {
    unsigned short rx_ch0_en;
    unsigned short rx_ch0_mode;
    unsigned short rx_ch0_pri;
    unsigned short rx_ch0_fc;
    unsigned short rx_ch0_sr_base_addr;
    unsigned short rx_ch0_sr_entry_num;
    unsigned short rx_ch0_sr_head_ptr;
    unsigned short rx_ch0_sr_tail_ptr;
    unsigned short rx_ch0_sr_afull_th;
    unsigned short rx_ch0_sr_aempty_th;
    unsigned short rx_ch0_dr_base_addr_h;
    unsigned short rx_ch0_dr_base_addr;
    unsigned short rx_ch0_dr_entry_num;
    unsigned short rx_ch0_dr_head_ptr;
    unsigned short rx_ch0_dr_tail_ptr;
    unsigned short rx_ch0_dr_afull_th;
    unsigned short rx_ch0_dr_aempty_th;
    unsigned short rx_ch0_status;
    unsigned short rx_ch0_done_total_cnt;
    unsigned short rx_ch0_pcie0_host_ddr_addr;
    unsigned short rx_ch0_pcie0_host_ddr_addr_h;
    unsigned short rx_ch0_pcie1_host_ddr_addr;
    unsigned short rx_ch0_pcie1_host_ddr_addr_h;
} host_ctrl_rx_chan_addr;

typedef struct _host_ctrl_addr_ {
    unsigned long base_addr;

    /* comm regs use host_ctrl_rb_regs */
    host_ctrl_tx_chan_addr tx_chan_off[ETE_TX_CHANN_MAX_NUMS];
    host_ctrl_rx_chan_addr rx_chan_off[ETE_RX_CHANN_MAX_NUMS];
} host_ctrl_addr;

extern host_ctrl_addr *g_pst_ete_addr;

#endif

