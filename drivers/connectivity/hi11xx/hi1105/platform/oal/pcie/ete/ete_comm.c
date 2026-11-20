

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#include "oal_util.h"
#include "ete_comm.h"
#include "ete_addr.h"
#include "hi1106/host_ctrl_rb_regs.h"
#include "hi1106/pcie_ctrl_rb_regs.h"

/*lint -e651 */
static const host_ctrl_addr g_hi1106_hostctrl_addr = {
    .base_addr = HOST_CTRL_RB_BASE,
    .tx_chan_off[ETE_TX_CHANN_0] = {
        .tx_ch0_en = HOST_CTRL_RB_TX_CH0_EN_OFF,
        .tx_ch0_mode = HOST_CTRL_RB_TX_CH0_MODE_OFF,
        .tx_ch0_pri = HOST_CTRL_RB_TX_CH0_PRI_OFF,
        .tx_ch0_sr_base_addr_h = HOST_CTRL_RB_TX_CH0_SR_BASE_ADDR_H_OFF,
        .tx_ch0_sr_base_addr = HOST_CTRL_RB_TX_CH0_SR_BASE_ADDR_OFF,
        .tx_ch0_sr_entry_num = HOST_CTRL_RB_TX_CH0_SR_ENTRY_NUM_OFF,
        .tx_ch0_sr_head_ptr = HOST_CTRL_RB_TX_CH0_SR_HEAD_PTR_OFF,
        .tx_ch0_sr_tail_ptr = HOST_CTRL_RB_TX_CH0_SR_TAIL_PTR_OFF,
        .tx_ch0_sr_afull_th = HOST_CTRL_RB_TX_CH0_SR_AFULL_TH_OFF,
        .tx_ch0_sr_aempty_th = HOST_CTRL_RB_TX_CH0_SR_AEMPTY_TH_OFF,
        .tx_ch0_dr_buff_size = HOST_CTRL_RB_TX_CH0_DR_BUFF_SIZE_OFF,
        .tx_ch0_dr_base_addr = HOST_CTRL_RB_TX_CH0_DR_BASE_ADDR_OFF,
        .tx_ch0_dr_entry_num = HOST_CTRL_RB_TX_CH0_DR_ENTRY_NUM_OFF,
        .tx_ch0_dr_head_ptr = HOST_CTRL_RB_TX_CH0_DR_HEAD_PTR_OFF,
        .tx_ch0_dr_tail_ptr = HOST_CTRL_RB_TX_CH0_DR_TAIL_PTR_OFF,
        .tx_ch0_dr_afull_th = HOST_CTRL_RB_TX_CH0_DR_AFULL_TH_OFF,
        .tx_ch0_dr_aempty_th = HOST_CTRL_RB_TX_CH0_DR_AEMPTY_TH_OFF,
        .tx_ch0_status = HOST_CTRL_RB_TX_CH0_STATUS_OFF,
        .tx_ch0_done_total_cnt = HOST_CTRL_RB_TX_CH0_DONE_TOTAL_CNT_OFF
    },
    .tx_chan_off[ETE_TX_CHANN_1] = {
        .tx_ch0_en = HOST_CTRL_RB_TX_CH1_EN_OFF,
        .tx_ch0_mode = HOST_CTRL_RB_TX_CH1_MODE_OFF,
        .tx_ch0_pri = HOST_CTRL_RB_TX_CH1_PRI_OFF,
        .tx_ch0_sr_base_addr_h = HOST_CTRL_RB_TX_CH1_SR_BASE_ADDR_H_OFF,
        .tx_ch0_sr_base_addr = HOST_CTRL_RB_TX_CH1_SR_BASE_ADDR_OFF,
        .tx_ch0_sr_entry_num = HOST_CTRL_RB_TX_CH1_SR_ENTRY_NUM_OFF,
        .tx_ch0_sr_head_ptr = HOST_CTRL_RB_TX_CH1_SR_HEAD_PTR_OFF,
        .tx_ch0_sr_tail_ptr = HOST_CTRL_RB_TX_CH1_SR_TAIL_PTR_OFF,
        .tx_ch0_sr_afull_th = HOST_CTRL_RB_TX_CH1_SR_AFULL_TH_OFF,
        .tx_ch0_sr_aempty_th = HOST_CTRL_RB_TX_CH1_SR_AEMPTY_TH_OFF,
        .tx_ch0_dr_buff_size = HOST_CTRL_RB_TX_CH1_DR_BUFF_SIZE_OFF,
        .tx_ch0_dr_base_addr = HOST_CTRL_RB_TX_CH1_DR_BASE_ADDR_OFF,
        .tx_ch0_dr_entry_num = HOST_CTRL_RB_TX_CH1_DR_ENTRY_NUM_OFF,
        .tx_ch0_dr_head_ptr = HOST_CTRL_RB_TX_CH1_DR_HEAD_PTR_OFF,
        .tx_ch0_dr_tail_ptr = HOST_CTRL_RB_TX_CH1_DR_TAIL_PTR_OFF,
        .tx_ch0_dr_afull_th = HOST_CTRL_RB_TX_CH1_DR_AFULL_TH_OFF,
        .tx_ch0_dr_aempty_th = HOST_CTRL_RB_TX_CH1_DR_AEMPTY_TH_OFF,
        .tx_ch0_status = HOST_CTRL_RB_TX_CH1_STATUS_OFF,
        .tx_ch0_done_total_cnt = HOST_CTRL_RB_TX_CH1_DONE_TOTAL_CNT_OFF
    },
    .tx_chan_off[ETE_TX_CHANN_2] = {
        .tx_ch0_en = HOST_CTRL_RB_TX_CH2_EN_OFF,
        .tx_ch0_mode = HOST_CTRL_RB_TX_CH2_MODE_OFF,
        .tx_ch0_pri = HOST_CTRL_RB_TX_CH2_PRI_OFF,
        .tx_ch0_sr_base_addr_h = HOST_CTRL_RB_TX_CH2_SR_BASE_ADDR_H_OFF,
        .tx_ch0_sr_base_addr = HOST_CTRL_RB_TX_CH2_SR_BASE_ADDR_OFF,
        .tx_ch0_sr_entry_num = HOST_CTRL_RB_TX_CH2_SR_ENTRY_NUM_OFF,
        .tx_ch0_sr_head_ptr = HOST_CTRL_RB_TX_CH2_SR_HEAD_PTR_OFF,
        .tx_ch0_sr_tail_ptr = HOST_CTRL_RB_TX_CH2_SR_TAIL_PTR_OFF,
        .tx_ch0_sr_afull_th = HOST_CTRL_RB_TX_CH2_SR_AFULL_TH_OFF,
        .tx_ch0_sr_aempty_th = HOST_CTRL_RB_TX_CH2_SR_AEMPTY_TH_OFF,
        .tx_ch0_dr_buff_size = HOST_CTRL_RB_TX_CH2_DR_BUFF_SIZE_OFF,
        .tx_ch0_dr_base_addr = HOST_CTRL_RB_TX_CH2_DR_BASE_ADDR_OFF,
        .tx_ch0_dr_entry_num = HOST_CTRL_RB_TX_CH2_DR_ENTRY_NUM_OFF,
        .tx_ch0_dr_head_ptr = HOST_CTRL_RB_TX_CH2_DR_HEAD_PTR_OFF,
        .tx_ch0_dr_tail_ptr = HOST_CTRL_RB_TX_CH2_DR_TAIL_PTR_OFF,
        .tx_ch0_dr_afull_th = HOST_CTRL_RB_TX_CH2_DR_AFULL_TH_OFF,
        .tx_ch0_dr_aempty_th = HOST_CTRL_RB_TX_CH2_DR_AEMPTY_TH_OFF,
        .tx_ch0_status = HOST_CTRL_RB_TX_CH2_STATUS_OFF,
        .tx_ch0_done_total_cnt = HOST_CTRL_RB_TX_CH2_DONE_TOTAL_CNT_OFF
    },
    .tx_chan_off[ETE_TX_CHANN_3] = {
        .tx_ch0_en = HOST_CTRL_RB_TX_CH3_EN_OFF,
        .tx_ch0_mode = HOST_CTRL_RB_TX_CH3_MODE_OFF,
        .tx_ch0_pri = HOST_CTRL_RB_TX_CH3_PRI_OFF,
        .tx_ch0_sr_base_addr_h = HOST_CTRL_RB_TX_CH3_SR_BASE_ADDR_H_OFF,
        .tx_ch0_sr_base_addr = HOST_CTRL_RB_TX_CH3_SR_BASE_ADDR_OFF,
        .tx_ch0_sr_entry_num = HOST_CTRL_RB_TX_CH3_SR_ENTRY_NUM_OFF,
        .tx_ch0_sr_head_ptr = HOST_CTRL_RB_TX_CH3_SR_HEAD_PTR_OFF,
        .tx_ch0_sr_tail_ptr = HOST_CTRL_RB_TX_CH3_SR_TAIL_PTR_OFF,
        .tx_ch0_sr_afull_th = HOST_CTRL_RB_TX_CH3_SR_AFULL_TH_OFF,
        .tx_ch0_sr_aempty_th = HOST_CTRL_RB_TX_CH3_SR_AEMPTY_TH_OFF,
        .tx_ch0_dr_buff_size = HOST_CTRL_RB_TX_CH3_DR_BUFF_SIZE_OFF,
        .tx_ch0_dr_base_addr = HOST_CTRL_RB_TX_CH3_DR_BASE_ADDR_OFF,
        .tx_ch0_dr_entry_num = HOST_CTRL_RB_TX_CH3_DR_ENTRY_NUM_OFF,
        .tx_ch0_dr_head_ptr = HOST_CTRL_RB_TX_CH3_DR_HEAD_PTR_OFF,
        .tx_ch0_dr_tail_ptr = HOST_CTRL_RB_TX_CH3_DR_TAIL_PTR_OFF,
        .tx_ch0_dr_afull_th = HOST_CTRL_RB_TX_CH3_DR_AFULL_TH_OFF,
        .tx_ch0_dr_aempty_th = HOST_CTRL_RB_TX_CH3_DR_AEMPTY_TH_OFF,
        .tx_ch0_status = HOST_CTRL_RB_TX_CH3_STATUS_OFF,
        .tx_ch0_done_total_cnt = HOST_CTRL_RB_TX_CH3_DONE_TOTAL_CNT_OFF
    },
    .rx_chan_off[ETE_RX_CHANN_0] = {
        .rx_ch0_en = HOST_CTRL_RB_RX_CH0_EN_OFF,
        .rx_ch0_mode = HOST_CTRL_RB_RX_CH0_MODE_OFF,
        .rx_ch0_pri = HOST_CTRL_RB_RX_CH0_PRI_OFF,
        .rx_ch0_fc = HOST_CTRL_RB_RX_CH0_FC_OFF,
        .rx_ch0_sr_base_addr = HOST_CTRL_RB_RX_CH0_SR_BASE_ADDR_OFF,
        .rx_ch0_sr_entry_num = HOST_CTRL_RB_RX_CH0_SR_ENTRY_NUM_OFF,
        .rx_ch0_sr_head_ptr = HOST_CTRL_RB_RX_CH0_SR_HEAD_PTR_OFF,
        .rx_ch0_sr_tail_ptr = HOST_CTRL_RB_RX_CH0_SR_TAIL_PTR_OFF,
        .rx_ch0_sr_afull_th = HOST_CTRL_RB_RX_CH0_SR_AFULL_TH_OFF,
        .rx_ch0_sr_aempty_th = HOST_CTRL_RB_RX_CH0_SR_AEMPTY_TH_OFF,
        .rx_ch0_dr_base_addr_h = HOST_CTRL_RB_RX_CH0_DR_BASE_ADDR_H_OFF,
        .rx_ch0_dr_base_addr = HOST_CTRL_RB_RX_CH0_DR_BASE_ADDR_OFF,
        .rx_ch0_dr_entry_num = HOST_CTRL_RB_RX_CH0_DR_ENTRY_NUM_OFF,
        .rx_ch0_dr_head_ptr = HOST_CTRL_RB_RX_CH0_DR_HEAD_PTR_OFF,
        .rx_ch0_dr_tail_ptr = HOST_CTRL_RB_RX_CH0_DR_TAIL_PTR_OFF,
        .rx_ch0_dr_afull_th = HOST_CTRL_RB_RX_CH0_DR_AFULL_TH_OFF,
        .rx_ch0_dr_aempty_th = HOST_CTRL_RB_RX_CH0_DR_AEMPTY_TH_OFF,
        .rx_ch0_status = HOST_CTRL_RB_RX_CH0_STATUS_OFF,
        .rx_ch0_done_total_cnt = HOST_CTRL_RB_RX_CH0_DONE_TOTAL_CNT_OFF,
        .rx_ch0_pcie0_host_ddr_addr = HOST_CTRL_RB_RX_CH0_PCIE0_HOST_DDR_ADDR_OFF,
        .rx_ch0_pcie0_host_ddr_addr_h = HOST_CTRL_RB_RX_CH0_PCIE0_HOST_DDR_ADDR_H_OFF,
        .rx_ch0_pcie1_host_ddr_addr = HOST_CTRL_RB_RX_CH0_PCIE1_HOST_DDR_ADDR_OFF,
        .rx_ch0_pcie1_host_ddr_addr_h = HOST_CTRL_RB_RX_CH0_PCIE1_HOST_DDR_ADDR_H_OFF
    },
    .rx_chan_off[ETE_RX_CHANN_1] = {
        .rx_ch0_en = HOST_CTRL_RB_RX_CH1_EN_OFF,
        .rx_ch0_mode = HOST_CTRL_RB_RX_CH1_MODE_OFF,
        .rx_ch0_pri = HOST_CTRL_RB_RX_CH1_PRI_OFF,
        .rx_ch0_fc = HOST_CTRL_RB_RX_CH1_FC_OFF,
        .rx_ch0_sr_base_addr = HOST_CTRL_RB_RX_CH1_SR_BASE_ADDR_OFF,
        .rx_ch0_sr_entry_num = HOST_CTRL_RB_RX_CH1_SR_ENTRY_NUM_OFF,
        .rx_ch0_sr_head_ptr = HOST_CTRL_RB_RX_CH1_SR_HEAD_PTR_OFF,
        .rx_ch0_sr_tail_ptr = HOST_CTRL_RB_RX_CH1_SR_TAIL_PTR_OFF,
        .rx_ch0_sr_afull_th = HOST_CTRL_RB_RX_CH1_SR_AFULL_TH_OFF,
        .rx_ch0_sr_aempty_th = HOST_CTRL_RB_RX_CH1_SR_AEMPTY_TH_OFF,
        .rx_ch0_dr_base_addr_h = HOST_CTRL_RB_RX_CH1_DR_BASE_ADDR_H_OFF,
        .rx_ch0_dr_base_addr = HOST_CTRL_RB_RX_CH1_DR_BASE_ADDR_OFF,
        .rx_ch0_dr_entry_num = HOST_CTRL_RB_RX_CH1_DR_ENTRY_NUM_OFF,
        .rx_ch0_dr_head_ptr = HOST_CTRL_RB_RX_CH1_DR_HEAD_PTR_OFF,
        .rx_ch0_dr_tail_ptr = HOST_CTRL_RB_RX_CH1_DR_TAIL_PTR_OFF,
        .rx_ch0_dr_afull_th = HOST_CTRL_RB_RX_CH1_DR_AFULL_TH_OFF,
        .rx_ch0_dr_aempty_th = HOST_CTRL_RB_RX_CH1_DR_AEMPTY_TH_OFF,
        .rx_ch0_status = HOST_CTRL_RB_RX_CH1_STATUS_OFF,
        .rx_ch0_done_total_cnt = HOST_CTRL_RB_RX_CH1_DONE_TOTAL_CNT_OFF,
        .rx_ch0_pcie0_host_ddr_addr = HOST_CTRL_RB_RX_CH1_PCIE0_HOST_DDR_ADDR_OFF,
        .rx_ch0_pcie0_host_ddr_addr_h = HOST_CTRL_RB_RX_CH1_PCIE0_HOST_DDR_ADDR_H_OFF,
        .rx_ch0_pcie1_host_ddr_addr = HOST_CTRL_RB_RX_CH1_PCIE1_HOST_DDR_ADDR_OFF,
        .rx_ch0_pcie1_host_ddr_addr_h = HOST_CTRL_RB_RX_CH1_PCIE1_HOST_DDR_ADDR_H_OFF
    },
    .rx_chan_off[ETE_RX_CHANN_2] = {
        .rx_ch0_en = HOST_CTRL_RB_RX_CH2_EN_OFF,
        .rx_ch0_mode = HOST_CTRL_RB_RX_CH2_MODE_OFF,
        .rx_ch0_pri = HOST_CTRL_RB_RX_CH2_PRI_OFF,
        .rx_ch0_fc = HOST_CTRL_RB_RX_CH2_FC_OFF,
        .rx_ch0_sr_base_addr = HOST_CTRL_RB_RX_CH2_SR_BASE_ADDR_OFF,
        .rx_ch0_sr_entry_num = HOST_CTRL_RB_RX_CH2_SR_ENTRY_NUM_OFF,
        .rx_ch0_sr_head_ptr = HOST_CTRL_RB_RX_CH2_SR_HEAD_PTR_OFF,
        .rx_ch0_sr_tail_ptr = HOST_CTRL_RB_RX_CH2_SR_TAIL_PTR_OFF,
        .rx_ch0_sr_afull_th = HOST_CTRL_RB_RX_CH2_SR_AFULL_TH_OFF,
        .rx_ch0_sr_aempty_th = HOST_CTRL_RB_RX_CH2_SR_AEMPTY_TH_OFF,
        .rx_ch0_dr_base_addr_h = HOST_CTRL_RB_RX_CH2_DR_BASE_ADDR_H_OFF,
        .rx_ch0_dr_base_addr = HOST_CTRL_RB_RX_CH2_DR_BASE_ADDR_OFF,
        .rx_ch0_dr_entry_num = HOST_CTRL_RB_RX_CH2_DR_ENTRY_NUM_OFF,
        .rx_ch0_dr_head_ptr = HOST_CTRL_RB_RX_CH2_DR_HEAD_PTR_OFF,
        .rx_ch0_dr_tail_ptr = HOST_CTRL_RB_RX_CH2_DR_TAIL_PTR_OFF,
        .rx_ch0_dr_afull_th = HOST_CTRL_RB_RX_CH2_DR_AFULL_TH_OFF,
        .rx_ch0_dr_aempty_th = HOST_CTRL_RB_RX_CH2_DR_AEMPTY_TH_OFF,
        .rx_ch0_status = HOST_CTRL_RB_RX_CH2_STATUS_OFF,
        .rx_ch0_done_total_cnt = HOST_CTRL_RB_RX_CH2_DONE_TOTAL_CNT_OFF,
        .rx_ch0_pcie0_host_ddr_addr = HOST_CTRL_RB_RX_CH2_PCIE0_HOST_DDR_ADDR_OFF,
        .rx_ch0_pcie0_host_ddr_addr_h = HOST_CTRL_RB_RX_CH2_PCIE0_HOST_DDR_ADDR_H_OFF,
        .rx_ch0_pcie1_host_ddr_addr = HOST_CTRL_RB_RX_CH2_PCIE1_HOST_DDR_ADDR_OFF,
        .rx_ch0_pcie1_host_ddr_addr_h = HOST_CTRL_RB_RX_CH2_PCIE1_HOST_DDR_ADDR_H_OFF
    },
    .rx_chan_off[ETE_RX_CHANN_3] = {
        .rx_ch0_en = HOST_CTRL_RB_RX_CH3_EN_OFF,
        .rx_ch0_mode = HOST_CTRL_RB_RX_CH3_MODE_OFF,
        .rx_ch0_pri = HOST_CTRL_RB_RX_CH3_PRI_OFF,
        .rx_ch0_fc = HOST_CTRL_RB_RX_CH3_FC_OFF,
        .rx_ch0_sr_base_addr = HOST_CTRL_RB_RX_CH3_SR_BASE_ADDR_OFF,
        .rx_ch0_sr_entry_num = HOST_CTRL_RB_RX_CH3_SR_ENTRY_NUM_OFF,
        .rx_ch0_sr_head_ptr = HOST_CTRL_RB_RX_CH3_SR_HEAD_PTR_OFF,
        .rx_ch0_sr_tail_ptr = HOST_CTRL_RB_RX_CH3_SR_TAIL_PTR_OFF,
        .rx_ch0_sr_afull_th = HOST_CTRL_RB_RX_CH3_SR_AFULL_TH_OFF,
        .rx_ch0_sr_aempty_th = HOST_CTRL_RB_RX_CH3_SR_AEMPTY_TH_OFF,
        .rx_ch0_dr_base_addr_h = HOST_CTRL_RB_RX_CH3_DR_BASE_ADDR_H_OFF,
        .rx_ch0_dr_base_addr = HOST_CTRL_RB_RX_CH3_DR_BASE_ADDR_OFF,
        .rx_ch0_dr_entry_num = HOST_CTRL_RB_RX_CH3_DR_ENTRY_NUM_OFF,
        .rx_ch0_dr_head_ptr = HOST_CTRL_RB_RX_CH3_DR_HEAD_PTR_OFF,
        .rx_ch0_dr_tail_ptr = HOST_CTRL_RB_RX_CH3_DR_TAIL_PTR_OFF,
        .rx_ch0_dr_afull_th = HOST_CTRL_RB_RX_CH3_DR_AFULL_TH_OFF,
        .rx_ch0_dr_aempty_th = HOST_CTRL_RB_RX_CH3_DR_AEMPTY_TH_OFF,
        .rx_ch0_status = HOST_CTRL_RB_RX_CH3_STATUS_OFF,
        .rx_ch0_done_total_cnt = HOST_CTRL_RB_RX_CH3_DONE_TOTAL_CNT_OFF,
        .rx_ch0_pcie0_host_ddr_addr = HOST_CTRL_RB_RX_CH3_PCIE0_HOST_DDR_ADDR_OFF,
        .rx_ch0_pcie0_host_ddr_addr_h = HOST_CTRL_RB_RX_CH3_PCIE0_HOST_DDR_ADDR_H_OFF,
        .rx_ch0_pcie1_host_ddr_addr = HOST_CTRL_RB_RX_CH3_PCIE1_HOST_DDR_ADDR_OFF,
        .rx_ch0_pcie1_host_ddr_addr_h = HOST_CTRL_RB_RX_CH3_PCIE1_HOST_DDR_ADDR_H_OFF
    },
    .rx_chan_off[ETE_RX_CHANN_4] = {
        .rx_ch0_en = HOST_CTRL_RB_RX_CH4_EN_OFF,
        .rx_ch0_mode = HOST_CTRL_RB_RX_CH4_MODE_OFF,
        .rx_ch0_pri = HOST_CTRL_RB_RX_CH4_PRI_OFF,
        .rx_ch0_fc = HOST_CTRL_RB_RX_CH4_FC_OFF,
        .rx_ch0_sr_base_addr = HOST_CTRL_RB_RX_CH4_SR_BASE_ADDR_OFF,
        .rx_ch0_sr_entry_num = HOST_CTRL_RB_RX_CH4_SR_ENTRY_NUM_OFF,
        .rx_ch0_sr_head_ptr = HOST_CTRL_RB_RX_CH4_SR_HEAD_PTR_OFF,
        .rx_ch0_sr_tail_ptr = HOST_CTRL_RB_RX_CH4_SR_TAIL_PTR_OFF,
        .rx_ch0_sr_afull_th = HOST_CTRL_RB_RX_CH4_SR_AFULL_TH_OFF,
        .rx_ch0_sr_aempty_th = HOST_CTRL_RB_RX_CH4_SR_AEMPTY_TH_OFF,
        .rx_ch0_dr_base_addr_h = HOST_CTRL_RB_RX_CH4_DR_BASE_ADDR_H_OFF,
        .rx_ch0_dr_base_addr = HOST_CTRL_RB_RX_CH4_DR_BASE_ADDR_OFF,
        .rx_ch0_dr_entry_num = HOST_CTRL_RB_RX_CH4_DR_ENTRY_NUM_OFF,
        .rx_ch0_dr_head_ptr = HOST_CTRL_RB_RX_CH4_DR_HEAD_PTR_OFF,
        .rx_ch0_dr_tail_ptr = HOST_CTRL_RB_RX_CH4_DR_TAIL_PTR_OFF,
        .rx_ch0_dr_afull_th = HOST_CTRL_RB_RX_CH4_DR_AFULL_TH_OFF,
        .rx_ch0_dr_aempty_th = HOST_CTRL_RB_RX_CH4_DR_AEMPTY_TH_OFF,
        .rx_ch0_status = HOST_CTRL_RB_RX_CH4_STATUS_OFF,
        .rx_ch0_done_total_cnt = HOST_CTRL_RB_RX_CH4_DONE_TOTAL_CNT_OFF,
        .rx_ch0_pcie0_host_ddr_addr = HOST_CTRL_RB_RX_CH4_PCIE0_HOST_DDR_ADDR_OFF,
        .rx_ch0_pcie0_host_ddr_addr_h = HOST_CTRL_RB_RX_CH4_PCIE0_HOST_DDR_ADDR_H_OFF,
        .rx_ch0_pcie1_host_ddr_addr = HOST_CTRL_RB_RX_CH4_PCIE1_HOST_DDR_ADDR_OFF,
        .rx_ch0_pcie1_host_ddr_addr_h = HOST_CTRL_RB_RX_CH4_PCIE1_HOST_DDR_ADDR_H_OFF
    }
};
/*lint +e651 */

host_ctrl_addr* g_pst_ete_addr = (host_ctrl_addr*)&g_hi1106_hostctrl_addr;

int32_t pcie_ete_ringbuf_init(pcie_ete_ringbuf *pst_ringbuf, uint64_t cpu_addr, uint64_t io_addr,
                              uint32_t idx, uint32_t size, uint16_t item_len)
{
    if (size < item_len) {
        return -OAL_EINVAL;
    }

    pst_ringbuf->cpu_addr = (uint64_t)cpu_addr;
    pst_ringbuf->io_addr = (uint64_t)io_addr;
    pst_ringbuf->idx = idx;
    pst_ringbuf->size = size;
    pst_ringbuf->item_len = item_len;
    pst_ringbuf->items = size / item_len;
    pst_ringbuf->rflag = 0;
    pst_ringbuf->wflag = 0;
    return OAL_SUCC;
}

int32_t pcie_ete_ringbuf_reset(pcie_ete_ringbuf *pst_ringbuf)
{
    pst_ringbuf->rd = 0;
    pst_ringbuf->wr = 0;
    pst_ringbuf->done_cnt = 0;
    pst_ringbuf->rflag = 0;
    pst_ringbuf->wflag = 0;
    if (pst_ringbuf->cpu_addr != 0x0) {
        memset_s((void*)(uintptr_t)pst_ringbuf->cpu_addr, pst_ringbuf->size,
                 0xff, pst_ringbuf->size);
    }
    return OAL_SUCC;
}

/* Returns  0:failed, non-zero:write bytes */
uint32_t pcie_ete_ringbuf_write(pcie_ete_ringbuf *pst_ringbuf, uint8_t *buf)
{
    uint32_t wr, real_wr;

    wr = pst_ringbuf->wr;

    if (pcie_ete_ringbuf_is_full(pst_ringbuf) == OAL_TRUE) {
        /* Ringbuf is full */
        return 0;
    }

    if (pst_ringbuf->debug) {
        oal_io_print("ete_ringbuf_write1 items:%u wr=%u rd=%u"NEWLINE,
                    pst_ringbuf->items, pst_ringbuf->wr, pst_ringbuf->rd);
        oal_io_print("ete_ringbuf_write2 wflag=%u rflag=%u"NEWLINE,
                    pst_ringbuf->wflag, pst_ringbuf->rflag);
    }

    real_wr = wr * pst_ringbuf->item_len;

    if (memcpy_s((void *)((uintptr_t)pst_ringbuf->cpu_addr + real_wr), pst_ringbuf->item_len,
        (void *)(buf), pst_ringbuf->item_len) != EOK) {
        return 0;
    }

#ifdef CONFIG_PRINTK
    if (pst_ringbuf->debug) {
        oal_print_hex_dump((uint8_t*)(uintptr_t)pst_ringbuf->cpu_addr + real_wr,
                           pst_ringbuf->item_len, 32, "ring data: "); // 32 B per
    }
#endif

    pst_ringbuf->wr++;

    if (pst_ringbuf->wr >= pst_ringbuf->items) {
        pst_ringbuf->wflag = !pst_ringbuf->wflag;
        pst_ringbuf->wr = 0; /* 翻转从0开始 */
    }

    return pst_ringbuf->item_len;
}

/* Returns  done rd count, update rd value */
uint32_t pcie_ete_ringbuf_update_rd(pcie_ete_ringbuf *pst_ringbuf, uint32_t rd_update)
{
    /* totalcount 最高bit表示翻转位 */
    uint32_t trans_cnt, rd;
    uint32_t flag = pcie_ete_trans_overflow_bitmask(rd_update);
    uint32_t value = pcie_ete_trans_value(rd_update);
    uint32_t overflow;

    overflow = (flag ^ pst_ringbuf->rflag) & 0x1;

    rd = pst_ringbuf->rd;

    if (overflow == PCIE_ETE_TRANS_OVERFLOW_TAG) {
        trans_cnt = value + pst_ringbuf->items - rd;
    } else {
        trans_cnt = value - rd;
    }

    if (trans_cnt > pst_ringbuf->items) {
        /* invalid rd */
        return PCIE_ETE_INVALID_RD_VALUE;
    }

    if (pst_ringbuf->debug) {
        oal_io_print("ete_ringbuf_update_rd1 items:%u wr=%u rd=%u"NEWLINE,
                    pst_ringbuf->items, pst_ringbuf->wr, pst_ringbuf->rd);
        oal_io_print("ete_ringbuf_update_rd2 wflag=%u rflag=%u flag=%u value=%u"NEWLINE,
                    pst_ringbuf->wflag, pst_ringbuf->rflag, flag, value);
    }

    if (overflow == PCIE_ETE_TRANS_OVERFLOW_TAG) {
        pst_ringbuf->rflag = flag; /* SOC维护 */
    }

    pst_ringbuf->rd = value;

    return trans_cnt;
}

/* ete channel registers */
/* tx chan register ops */
OAL_STATIC OAL_INLINE void pcie_ete_tx_chan_clken(uintptr_t base_address, uint32_t chan_idx, uint32_t enable)
{
    hreg_tx_ch0_en value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_en;

    hreg_get_val(value, offset);
    value.bits.tx_ch0_en = !!enable;
    hreg_set_val(value, offset);
}

/* default descr mode */
OAL_STATIC OAL_INLINE void pcie_ete_set_tx_chan_mode(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_tx_ch0_mode value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_mode;

    value.as_dword = 0;
    value.bits.tx_ch0_mode = 0; /* 0 is descr mode, 1 is memory copy mode */
    hreg_set_val(value, offset);
}

OAL_STATIC OAL_INLINE void pcie_ete_set_tx_chan_pri(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_tx_ch0_pri value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_pri;

    /* default 0, no fc, default pri, high */
    value.as_dword = 0;
    hreg_set_val(value, offset);
}

/* set host iova address */
OAL_STATIC OAL_INLINE void pcie_ete_set_tx_sr_base_addr(uintptr_t base_address, uint32_t chan_idx, uint64_t iova)
{
    hreg_tx_ch0_sr_base_addr_h hvalue;
    hreg_tx_ch0_sr_base_addr   lvalue;
    uintptr_t hoffset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_sr_base_addr_h;
    uintptr_t loffset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_sr_base_addr;

    hvalue.as_dword = 0;
    lvalue.as_dword = 0;

    hvalue.bits.tx_ch0_sr_base_addr_h = iova >> 32; /* high 32 bits */
    lvalue.bits.tx_ch0_sr_base_addr   = iova; /* low 32 bits */

    hreg_set_val(hvalue, hoffset);
    hreg_set_val(lvalue, loffset);
}

uint64_t pcie_ete_get_tx_sr_base_addr(uintptr_t base_address, uint32_t chan_idx)
{
    uint64_t ret;

    hreg_tx_ch0_sr_base_addr_h hvalue;
    hreg_tx_ch0_sr_base_addr   lvalue;
    uintptr_t hoffset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_sr_base_addr_h;
    uintptr_t loffset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_sr_base_addr;

    hreg_get_val(hvalue, hoffset);
    hreg_get_val(lvalue, loffset);

    ret = hvalue.bits.tx_ch0_sr_base_addr_h; /*lint !e838 */
    ret <<= 32; // 32 B per
    ret |= lvalue.bits.tx_ch0_sr_base_addr;

    return ret;
}

/* set host iova address */
OAL_STATIC OAL_INLINE void pcie_ete_set_tx_dr_base_addr(uintptr_t base_address, uint32_t chan_idx, uint32_t cpuaddr)
{
    hreg_tx_ch0_dr_base_addr   value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_dr_base_addr;

    value.as_dword = 0;
    value.bits.tx_ch0_dr_base_addr   = cpuaddr; /* low 32 bits */

    hreg_set_val(value, offset);
}

uint32_t pcie_ete_get_tx_dr_base_addr(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_tx_ch0_dr_base_addr  value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_dr_base_addr;

    hreg_get_val(value, offset);

    return value.bits.tx_ch0_dr_base_addr;
}

uint32_t pcie_ete_get_tx_sr_entry_num(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_tx_ch0_sr_entry_num value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_sr_entry_num;
    hreg_get_val(value, offset);
    return value.bits.tx_ch0_sr_entry_num;
}

uint32_t pcie_ete_get_tx_dr_entry_num(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_tx_ch0_dr_entry_num value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_dr_entry_num;
    hreg_get_val(value, offset);
    return value.bits.tx_ch0_dr_entry_num;
}

OAL_STATIC OAL_INLINE void pcie_ete_set_tx_sr_entry_num(uintptr_t base_address, uint32_t chan_idx, uint32_t entry_num)
{
    hreg_tx_ch0_sr_entry_num value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_sr_entry_num;

    value.bits.tx_ch0_sr_entry_num = entry_num;
    hreg_set_val(value, offset);
}

OAL_STATIC OAL_INLINE void pcie_ete_set_tx_dr_entry_num(uintptr_t base_address, uint32_t chan_idx, uint32_t entry_num)
{
    hreg_tx_ch0_dr_entry_num value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_dr_entry_num;

    value.bits.tx_ch0_dr_entry_num = entry_num;
    hreg_set_val(value, offset);
}

/* head ptr update after chan en & clk en */
OAL_STATIC OAL_INLINE void pcie_ete_set_tx_sr_head_ptr(uintptr_t base_address, uint32_t chan_idx, uint32_t head_ptr)
{
    hreg_tx_ch0_sr_head_ptr value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_sr_head_ptr;

    /* default 0, no fc, default pri, high */
    value.bits.tx_ch0_sr_head_ptr = head_ptr;
    hreg_set_val(value, offset);
}

uint32_t pcie_ete_get_tx_sr_head_ptr(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_tx_ch0_sr_head_ptr value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_sr_head_ptr;

    hreg_get_val(value, offset);
    return value.bits.tx_ch0_sr_head_ptr;
}

/* head ptr update after chan en & clk en */
OAL_STATIC OAL_INLINE void pcie_ete_set_tx_dr_head_ptr(uintptr_t base_address, uint32_t chan_idx, uint32_t head_ptr)
{
    hreg_tx_ch0_dr_head_ptr value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_dr_head_ptr;

    /* default 0, no fc, default pri, high */
    value.bits.tx_ch0_dr_head_ptr = head_ptr;
    hreg_set_val(value, offset);
}

uint32_t pcie_ete_get_tx_dr_head_ptr(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_tx_ch0_dr_head_ptr value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_dr_head_ptr;

    hreg_get_val(value, offset);
    return value.bits.tx_ch0_dr_head_ptr;
}

unsigned int pcie_ete_get_tx_done_total_cnt(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_tx_ch0_done_total_cnt value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_done_total_cnt;

    hreg_get_val(value, offset);
    return value.bits.tx_ch0_total_cnt;
}

uint32_t pcie_ete_get_tx_sr_tail_ptr(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_tx_ch0_sr_tail_ptr value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_sr_tail_ptr;

    hreg_get_val(value, offset);
    return value.bits.tx_ch0_sr_tail_ptr;
}

uint32_t pcie_ete_get_tx_dr_tail_ptr(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_tx_ch0_dr_tail_ptr value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_dr_tail_ptr;

    hreg_get_val(value, offset);
    return value.bits.tx_ch0_dr_tail_ptr;
}

uint32_t pcie_ete_get_tx_status(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_tx_ch0_status value;
    uintptr_t offset = base_address + g_pst_ete_addr->tx_chan_off[chan_idx].tx_ch0_status;

    hreg_get_val(value, offset);
    return value.bits.tx_ch0_status;
}

/* rx chan register ops */
OAL_STATIC OAL_INLINE void pcie_ete_rx_chan_clken(uintptr_t base_address, uint32_t chan_idx, uint32_t enable)
{
    hreg_rx_ch0_en value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_en;

    hreg_get_val(value, offset);
    value.bits.rx_ch0_en = !!enable;
    hreg_set_val(value, offset);
}

/* default descr mode */
OAL_STATIC OAL_INLINE void pcie_ete_set_rx_chan_mode(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_rx_ch0_mode value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_mode;

    value.as_dword = 0;
    value.bits.rx_ch0_mode = 0; /* 0 is descr mode, 1 is memory copy mode */
    hreg_set_val(value, offset);
}

OAL_STATIC OAL_INLINE void pcie_ete_set_rx_chan_pri(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_rx_ch0_pri value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_pri;

    /* default 0, no fc, default pri, high */
    value.as_dword = 0;
    hreg_set_val(value, offset);
}

OAL_STATIC OAL_INLINE void pcie_ete_set_rx_sr_base_addr(uintptr_t base_address, uint32_t chan_idx, uint32_t cpuaddr)
{
    hreg_rx_ch0_sr_base_addr   value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_sr_base_addr;

    value.as_dword = 0;
    value.bits.rx_ch0_sr_base_addr   = cpuaddr; /* low 32 bits */

    hreg_set_val(value, offset);
}

uint32_t pcie_ete_get_rx_sr_base_addr(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_rx_ch0_sr_base_addr  value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_sr_base_addr;

    hreg_get_val(value, offset);

    return value.bits.rx_ch0_sr_base_addr;
}

/* set host iova address */
OAL_STATIC OAL_INLINE void pcie_ete_set_rx_dr_base_addr(uintptr_t base_address, uint32_t chan_idx, uint64_t iova)
{
    hreg_rx_ch0_dr_base_addr_h hvalue;
    hreg_rx_ch0_dr_base_addr   lvalue;
    uintptr_t hoffset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_dr_base_addr_h;
    uintptr_t loffset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_dr_base_addr;

    hvalue.as_dword = 0;
    lvalue.as_dword = 0;

    hvalue.bits.rx_ch0_dr_base_addr_h = iova >> 32; /* high 32 bits */
    lvalue.bits.rx_ch0_dr_base_addr   = iova; /* low 32 bits */

    hreg_set_val(hvalue, hoffset);
    hreg_set_val(lvalue, loffset);
}

uint64_t pcie_ete_get_rx_dr_base_addr(uintptr_t base_address, uint32_t chan_idx)
{
    uint64_t ret;

    hreg_rx_ch0_dr_base_addr_h hvalue;
    hreg_rx_ch0_dr_base_addr   lvalue;
    uintptr_t hoffset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_dr_base_addr_h;
    uintptr_t loffset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_dr_base_addr;

    hreg_get_val(hvalue, hoffset);
    hreg_get_val(lvalue, loffset);

    ret = hvalue.bits.rx_ch0_dr_base_addr_h; /*lint !e838 */
    ret <<= 32; // 32 B per
    ret |= lvalue.bits.rx_ch0_dr_base_addr;

    return ret;
}

uint32_t pcie_ete_get_rx_sr_entry_num(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_rx_ch0_sr_entry_num value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_sr_entry_num;
    hreg_get_val(value, offset);
    return value.bits.rx_ch0_sr_entry_num;
}

uint32_t pcie_ete_get_rx_dr_entry_num(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_rx_ch0_dr_entry_num value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_dr_entry_num;
    hreg_get_val(value, offset);
    return value.bits.rx_ch0_dr_entry_num;
}

OAL_STATIC OAL_INLINE void pcie_ete_set_rx_sr_entry_num(uintptr_t base_address, uint32_t chan_idx, uint32_t entry_num)
{
    hreg_rx_ch0_sr_entry_num value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_sr_entry_num;
    value.bits.rx_ch0_sr_entry_num = entry_num;
    hreg_set_val(value, offset);
}

OAL_STATIC OAL_INLINE void pcie_ete_set_rx_dr_entry_num(uintptr_t base_address, uint32_t chan_idx, uint32_t entry_num)
{
    hreg_rx_ch0_dr_entry_num value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_dr_entry_num;

    value.bits.rx_ch0_dr_entry_num = entry_num;
    hreg_set_val(value, offset);
}

OAL_STATIC OAL_INLINE void pcie_ete_set_rx_sr_head_ptr(uintptr_t base_address, uint32_t chan_idx, uint32_t head_ptr)
{
    hreg_rx_ch0_sr_head_ptr value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_sr_head_ptr;

    /* default 0, no fc, default pri, high */
    value.bits.rx_ch0_sr_head_ptr = head_ptr;
    hreg_set_val(value, offset);
}

uint32_t pcie_ete_get_rx_sr_head_ptr(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_rx_ch0_sr_head_ptr value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_sr_head_ptr;

    hreg_get_val(value, offset);
    return value.bits.rx_ch0_sr_head_ptr;
}

OAL_STATIC OAL_INLINE void pcie_ete_set_rx_dr_head_ptr(uintptr_t base_address, uint32_t chan_idx, uint32_t head_ptr)
{
    hreg_rx_ch0_dr_head_ptr value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_dr_head_ptr;

    /* default 0, no fc, default pri, high */
    value.bits.rx_ch0_dr_head_ptr = head_ptr;
    hreg_set_val(value, offset);
}

uint32_t pcie_ete_get_rx_dr_head_ptr(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_rx_ch0_dr_head_ptr value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_dr_head_ptr;

    hreg_get_val(value, offset);
    return value.bits.rx_ch0_dr_head_ptr;
}

unsigned int pcie_ete_get_rx_done_total_cnt(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_rx_ch0_done_total_cnt value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_done_total_cnt;

    hreg_get_val(value, offset);
    return value.bits.rx_ch0_total_cnt;
}

uint32_t pcie_ete_get_rx_sr_tail_ptr(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_rx_ch0_sr_tail_ptr value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_sr_tail_ptr;

    hreg_get_val(value, offset);
    return value.bits.rx_ch0_sr_tail_ptr;
}

uint32_t pcie_ete_get_rx_dr_tail_ptr(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_rx_ch0_dr_tail_ptr value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_dr_tail_ptr;

    hreg_get_val(value, offset);
    return value.bits.rx_ch0_dr_tail_ptr;
}

uint32_t pcie_ete_get_rx_status(uintptr_t base_address, uint32_t chan_idx)
{
    hreg_rx_ch0_status value;
    uintptr_t offset = base_address + g_pst_ete_addr->rx_chan_off[chan_idx].rx_ch0_status;

    hreg_get_val(value, offset);
    return value.bits.rx_ch0_status;
}

/* common registers */
#define PCIE_ETE_TEST_VALUE (0x5a5a)
#define PCIE_ETE_TEST_RESULT (0xa5a5)
OAL_STATIC OAL_INLINE int32_t pcie_ete_test_reg(uintptr_t base_address)
{
    hreg_host_ctrl_test_value value;
    uintptr_t offset = base_address + HOST_CTRL_RB_HOST_CTRL_TEST_VALUE_OFF;
    value.bits.host_ctrl_test_value = PCIE_ETE_TEST_VALUE; /* 5a5a is test value */
    hreg_set_val(value, offset);
    hreg_get_val(value, offset);
    if (value.bits.host_ctrl_test_value == PCIE_ETE_TEST_RESULT) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}

/* interrupt status */
uint32_t pcie_ete_get_rx_err_addr(uintptr_t base_address)
{
    hreg_rx_err_addr value;
    uintptr_t offset = base_address + HOST_CTRL_RB_RX_ERR_ADDR_OFF;

    hreg_get_val(value, offset);
    return value.bits.rx_err_addr;
}

uint32_t pcie_ete_get_tx_err_addr(uintptr_t base_address)
{
    hreg_tx_err_addr value;
    uintptr_t offset = base_address + HOST_CTRL_RB_TX_ERR_ADDR_OFF;

    hreg_get_val(value, offset);
    return value.bits.tx_err_addr;
}

uint32_t pcie_ete_get_ete_intr_mask(uintptr_t base_address)
{
    hreg_ete_intr_mask intr_mask;
    uintptr_t intr_addr = base_address + HOST_CTRL_RB_ETE_INTR_MASK_OFF;
    hreg_get_val(intr_mask, intr_addr);
    return intr_mask.as_dword;
}

OAL_STATIC OAL_INLINE void pcie_ete_set_ete_intr_mask(uintptr_t base_address)
{
    hreg_ete_intr_mask intr_mask;
    uintptr_t intr_addr = base_address + HOST_CTRL_RB_ETE_INTR_MASK_OFF;
    intr_mask.as_dword = 0x0; /* unmask all done int */
    hreg_set_val(intr_mask, intr_addr);
}

uint32_t pcie_ete_get_ete_intr_sts(uintptr_t base_address)
{
    hreg_ete_intr_sts value;
    uintptr_t offset = base_address + HOST_CTRL_RB_ETE_INTR_STS_OFF;

    hreg_get_val(value, offset);

    return value.as_dword;
}

uint32_t pcie_ete_get_ete_intr_raw_sts(uintptr_t base_address)
{
    hreg_ete_intr_sts value;
    uintptr_t offset = base_address + HOST_CTRL_RB_ETE_INTR_RAW_STS_OFF;

    hreg_get_val(value, offset);

    return value.as_dword;
}

OAL_STATIC OAL_INLINE void pcie_ete_set_ete_intr_clr(uintptr_t base_address, uint32_t clr)
{
    hreg_ete_intr_clr value;

    uintptr_t offset = base_address + HOST_CTRL_RB_ETE_INTR_CLR_OFF;

    value.as_dword = clr; /* unmask all done int */
    hreg_set_val(value, offset);
}

uint32_t pcie_ete_get_ete_ch_dr_empty_intr_mask(uintptr_t base_address)
{
    hreg_ete_ch_dr_empty_intr_mask value;

    uintptr_t offset = base_address + HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_MASK_OFF;

    hreg_get_val(value, offset);

    return value.as_dword;
}

OAL_STATIC OAL_INLINE void pcie_ete_set_ete_ch_dr_empty_intr_mask(uintptr_t base_address)
{
#ifdef _PRE_COMMENT_CODE_
    hreg_ete_ch_dr_empty_intr_mask value;

    uintptr_t offset = base_address + HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_MASK_OFF;
    hreg_get_val(value, offset);
    value.bits.tx_ch_dr_empty_intr_mask = 0x0; /* unmask tx dr empty intr */
    hreg_set_val(value, offset);
#endif
}

uint32_t pcie_ete_get_ete_ch_dr_empty_intr_sts(uintptr_t base_address)
{
    hreg_ete_ch_dr_empty_intr_sts value;
    uintptr_t offset = base_address + HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_STS_OFF;

    hreg_get_val(value, offset);

    return value.as_dword;
}

uint32_t pcie_ete_get_ete_ch_dr_empty_intr_raw_sts(uintptr_t base_address)
{
    hreg_ete_intr_raw_sts value;
    uintptr_t offset = base_address + HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_RAW_STS_OFF;

    hreg_get_val(value, offset);

    return value.as_dword;
}

OAL_STATIC OAL_INLINE void pcie_ete_set_ete_ch_dr_empty_intr_clr(uintptr_t base_address, uint32_t clr)
{
    hreg_ete_intr_clr value;

    uintptr_t offset = base_address + HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_CLR_OFF;

    value.as_dword = clr; /* unmask all done int */
    hreg_set_val(value, offset);
}

/* Export Functions */
/* Set by Host */
int pcie_ete_tx_sr_hardinit(uintptr_t base_address, uint32_t chan_idx, pcie_ete_ringbuf* pst_ring)
{
    pcie_ete_set_tx_sr_base_addr(base_address, chan_idx, pst_ring->io_addr);
    pcie_ete_set_tx_sr_entry_num(base_address, chan_idx, pst_ring->items - 1);
    return 0;
}

int pcie_ete_rx_dr_hardinit(uintptr_t base_address, uint32_t chan_idx, pcie_ete_ringbuf* pst_ring)
{
    pcie_ete_set_rx_dr_base_addr(base_address, chan_idx, pst_ring->io_addr);
    pcie_ete_set_rx_dr_entry_num(base_address, chan_idx, pst_ring->items - 1);
    return 0;
}

/* Set by Device */
int pcie_ete_tx_dr_hardinit(uintptr_t base_address, uint32_t chan_idx, pcie_ete_ringbuf* pst_ring)
{
    pcie_ete_set_tx_dr_base_addr(base_address, chan_idx, (uint32_t)pst_ring->io_addr);
    pcie_ete_set_tx_dr_entry_num(base_address, chan_idx, pst_ring->items - 1);
    return 0;
}

int pcie_ete_rx_sr_hardinit(uintptr_t base_address, uint32_t chan_idx, pcie_ete_ringbuf* pst_ring)
{
    pcie_ete_set_rx_sr_base_addr(base_address, chan_idx, (uint32_t)pst_ring->io_addr);
    pcie_ete_set_rx_sr_entry_num(base_address, chan_idx, pst_ring->items - 1);
    return 0;
}

void pcie_ete_ring_update_rx_dr_hptr(pcie_ete_ringbuf *pst_ringbuf, uintptr_t base_address, uint32_t chan_idx)
{
    uint32_t head_ptr = pcie_ete_trans_get_ptr(pst_ringbuf->wr, pst_ringbuf->wflag);
    (void)oal_readl((uintptr_t)pst_ringbuf->cpu_addr);
    pcie_ete_set_rx_dr_head_ptr(base_address, chan_idx, head_ptr);
#ifdef ETE_COMM_DEBUG
    oal_io_print("update rx_dr chan_idx:%u head_ptr:0x%x wr%u wflag:%u"NEWLINE,
                chan_idx, head_ptr, pst_ringbuf->wr, pst_ringbuf->wflag);
    pcie_ete_print_trans_rx_chan_regs(base_address, chan_idx);
#endif
}

void pcie_ete_ring_update_tx_sr_hptr(pcie_ete_ringbuf *pst_ringbuf, uintptr_t base_address, uint32_t chan_idx)
{
    uint32_t head_ptr = pcie_ete_trans_get_ptr(pst_ringbuf->wr, pst_ringbuf->wflag);
    (void)oal_readl((uintptr_t)pst_ringbuf->cpu_addr);
    pcie_ete_set_tx_sr_head_ptr(base_address, chan_idx, head_ptr);
#ifdef ETE_COMM_DEBUG
    oal_io_print("update tx_sr chan_idx:%u head_ptr:0x%x wr%u wflag:%u"NEWLINE,
                chan_idx, head_ptr, pst_ringbuf->wr, pst_ringbuf->wflag);
    pcie_ete_print_trans_tx_chan_regs(base_address, chan_idx);
#endif
}

void pcie_ete_ring_update_rx_sr_hptr(pcie_ete_ringbuf *pst_ringbuf, uintptr_t base_address, uint32_t chan_idx)
{
    uint32_t head_ptr = pcie_ete_trans_get_ptr(pst_ringbuf->wr, pst_ringbuf->wflag);
    (void)oal_readl((uintptr_t)pst_ringbuf->cpu_addr);
    pcie_ete_set_rx_sr_head_ptr(base_address, chan_idx, head_ptr);
#ifdef ETE_COMM_DEBUG
    oal_io_print("update rx_sr chan_idx:%u head_ptr:0x%x wr%u wflag:%u"NEWLINE,
                chan_idx, head_ptr, pst_ringbuf->wr, pst_ringbuf->wflag);
    pcie_ete_print_trans_rx_chan_regs(base_address, chan_idx);
#endif
}


void pcie_ete_ring_update_tx_dr_hptr(pcie_ete_ringbuf *pst_ringbuf, uintptr_t base_address, uint32_t chan_idx)
{
    uint32_t head_ptr = pcie_ete_trans_get_ptr(pst_ringbuf->wr, pst_ringbuf->wflag);
    (void)oal_readl((uintptr_t)pst_ringbuf->cpu_addr);
    pcie_ete_set_tx_dr_head_ptr(base_address, chan_idx, head_ptr);
#ifdef ETE_COMM_DEBUG
    oal_io_print("update tx_dr chan_idx:%u head_ptr:0x%x wr%u wflag:%u"NEWLINE,
                chan_idx, head_ptr, pst_ringbuf->wr, pst_ringbuf->wflag);
    pcie_ete_print_trans_tx_chan_regs(base_address, chan_idx);
#endif
}

uint32_t pcie_ete_update_ring_rx_donecnt(pcie_ete_ringbuf *pst_ringbuf, uintptr_t base_address, uint32_t chan_idx)
{
    uint32_t rd_update = pcie_ete_get_rx_done_total_cnt(base_address, chan_idx);
    return pcie_ete_ringbuf_update_rd(pst_ringbuf, rd_update);
}

uint32_t pcie_ete_update_ring_tx_donecnt(pcie_ete_ringbuf *pst_ringbuf, uintptr_t base_address, uint32_t chan_idx)
{
    uint32_t rd_update = pcie_ete_get_tx_done_total_cnt(base_address, chan_idx);
    return pcie_ete_ringbuf_update_rd(pst_ringbuf, rd_update);
}

void pcie_ete_rx_chan_clken_enable(uintptr_t base_address, uint32_t chan_idx)
{
    pcie_ete_rx_chan_clken(base_address, chan_idx, OAL_TRUE);
}

void pcie_ete_tx_chan_clken_enable(uintptr_t base_address, uint32_t chan_idx)
{
    pcie_ete_tx_chan_clken(base_address, chan_idx, OAL_TRUE);
}

/* base functions */
int32_t pcie_ete_clk_en(uintptr_t base_address, uint32_t enable)
{
    hreg_ete_clk_en value;
    hreg_ete_clk_en read;
    uintptr_t offset = base_address + HOST_CTRL_RB_ETE_CLK_EN_OFF;
    hreg_get_val(value, offset);
    value.bits.ete_clk_en = !!enable;
    hreg_set_val(value, offset);
    hreg_get_val(read, offset);
    if (read.bits.ete_clk_en != value.bits.ete_clk_en) {
        return -OAL_EFAIL;
    } else {
        return OAL_SUCC;
    }
}

void pcie_dual_enable(uintptr_t base_address, uint32_t enable)
{
    hreg_cfg_host_mode value;
    uintptr_t offset = base_address + HOST_CTRL_RB_CFG_HOST_MODE_OFF;
    value.as_dword = 0x0;
    value.bits.cfg_host_mode = !!enable;
    hreg_set_val(value, offset);
}

void pcie_ete_intr_winit(uintptr_t base_address)
{
    pcie_ete_set_ete_intr_mask(base_address);
#ifdef _PRE_COMMENT_CODE_
    pcie_ete_set_ete_ch_dr_empty_intr_mask(base_address);
#endif
}

/* LOG PRINT */
void pcie_ete_print_trans_tx_chan_regs(uintptr_t base, uint32_t i)
{
    oal_io_print("[%d] tx_status:0x%x tx_sr_base 0x%llx tx_dr_base 0x%x"NEWLINE, i,
                pcie_ete_get_tx_status(base, i),
                pcie_ete_get_tx_sr_base_addr(base, i),
                pcie_ete_get_tx_dr_base_addr(base, i));
    oal_io_print("[%d] tx_dr_head:0x%x tx_totalcnt 0x%x dr_tail:0x%x"NEWLINE, i,
                pcie_ete_get_tx_dr_head_ptr(base, i),
                pcie_ete_get_tx_done_total_cnt(base, i),
                pcie_ete_get_tx_dr_tail_ptr(base, i));
    oal_io_print("[%d] tx_sr_head:0x%x tx_totalcnt 0x%x sr_tail:0x%x"NEWLINE, i,
                pcie_ete_get_tx_sr_head_ptr(base, i),
                pcie_ete_get_tx_done_total_cnt(base, i),
                pcie_ete_get_tx_sr_tail_ptr(base, i));
    oal_io_print("[%d] tx_sr_entry_num:%u tx_dr_entry_num %u"NEWLINE, i,
                pcie_ete_get_tx_sr_entry_num(base, i),
                pcie_ete_get_tx_dr_entry_num(base, i));
    oal_io_print(""NEWLINE);
}

void pcie_ete_print_trans_rx_chan_regs(uintptr_t base, uint32_t i)
{
    oal_io_print("[%d] rx_status:0x%x rx_dr_base 0x%llx rx_sr_base 0x%x"NEWLINE, i,
                pcie_ete_get_rx_status(base, i),
                pcie_ete_get_rx_dr_base_addr(base, i),
                pcie_ete_get_rx_sr_base_addr(base, i));
    oal_io_print("[%d] rx_dr_head:0x%x rx_totalcnt 0x%x dr_tail:0x%x"NEWLINE, i,
                pcie_ete_get_rx_dr_head_ptr(base, i),
                pcie_ete_get_rx_done_total_cnt(base, i),
                pcie_ete_get_rx_dr_tail_ptr(base, i));
    oal_io_print("[%d] rx_sr_head:0x%x rx_totalcnt 0x%x sr_tail:0x%x"NEWLINE, i,
                pcie_ete_get_rx_sr_head_ptr(base, i),
                pcie_ete_get_rx_done_total_cnt(base, i),
                pcie_ete_get_rx_sr_tail_ptr(base, i));
    oal_io_print("[%d] rx_sr_entry_num:0x%x rx_dr_entry_num 0x%x"NEWLINE, i,
                pcie_ete_get_rx_sr_entry_num(base, i),
                pcie_ete_get_rx_dr_entry_num(base, i));
    oal_io_print(""NEWLINE);
}

void oal_ete_print_trans_comm_regs(uintptr_t base)
{
    oal_io_print("intr_sts 0x%x intr_rawsts 0x%x dre_sts 0x%x dre_rawsts 0x%x\n",
                 pcie_ete_get_ete_intr_sts(base), pcie_ete_get_ete_intr_raw_sts(base),
                 pcie_ete_get_ete_ch_dr_empty_intr_sts(base),
                 pcie_ete_get_ete_ch_dr_empty_intr_raw_sts(base));
    oal_io_print("rx_err_addr 0x%x tx_err_addr 0x%x ete_intr_mask 0x%x dre_intmask 0x%x\n",
                 pcie_ete_get_rx_err_addr(base), pcie_ete_get_tx_err_addr(base),
                 pcie_ete_get_ete_intr_mask(base),
                 pcie_ete_get_ete_ch_dr_empty_intr_mask(base));
    oal_io_print(""NEWLINE);
}
#endif
