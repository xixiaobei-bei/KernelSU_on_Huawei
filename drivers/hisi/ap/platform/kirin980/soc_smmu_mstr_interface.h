#ifndef __SOC_SMMU_MSTR_INTERFACE_H__
#define __SOC_SMMU_MSTR_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_SMMU_MSTR_GLB_BYPASS_ADDR(base) ((base) + (0x0000))
#define SOC_SMMU_MSTR_DEBUG_MODE_ADDR(base) ((base) + (0x0004))
#define SOC_SMMU_MSTR_MEM_CTRL_ADDR(base) ((base) + (0x0008))
#define SOC_SMMU_MSTR_CLK_EN_ADDR(base) ((base) + (0x000C))
#define SOC_SMMU_MSTR_END_REQ_0_ADDR(base) ((base) + (0x0010))
#define SOC_SMMU_MSTR_END_REQ_1_ADDR(base) ((base) + (0x0014))
#define SOC_SMMU_MSTR_END_REQ_2_ADDR(base) ((base) + (0x0018))
#define SOC_SMMU_MSTR_END_ACK_0_ADDR(base) ((base) + (0x001C))
#define SOC_SMMU_MSTR_END_ACK_1_ADDR(base) ((base) + (0x0020))
#define SOC_SMMU_MSTR_END_ACK_2_ADDR(base) ((base) + (0x0024))
#define SOC_SMMU_MSTR_SMRX_START_0_ADDR(base) ((base) + (0x0028))
#define SOC_SMMU_MSTR_SMRX_START_1_ADDR(base) ((base) + (0x002C))
#define SOC_SMMU_MSTR_SMRX_START_2_ADDR(base) ((base) + (0x0030))
#define SOC_SMMU_MSTR_INPT_SEL_ADDR(base) ((base) + (0x0034))
#define SOC_SMMU_MSTR_RESERVED_0_ADDR(base) ((base) + (0x0038))
#define SOC_SMMU_MSTR_INTMASK_ADDR(base) ((base) + (0x0040))
#define SOC_SMMU_MSTR_INTRAW_ADDR(base) ((base) + (0x0044))
#define SOC_SMMU_MSTR_INTSTAT_ADDR(base) ((base) + (0x0048))
#define SOC_SMMU_MSTR_INTCLR_ADDR(base) ((base) + (0x004C))
#define SOC_SMMU_MSTR_DBG_0_ADDR(base) ((base) + (0x0050))
#define SOC_SMMU_MSTR_DBG_1_ADDR(base) ((base) + (0x0054))
#define SOC_SMMU_MSTR_DBG_2_ADDR(base) ((base) + (0x0058))
#define SOC_SMMU_MSTR_DBG_3_ADDR(base) ((base) + (0x005C))
#define SOC_SMMU_MSTR_DBG_4_ADDR(base) ((base) + (0x0060))
#define SOC_SMMU_MSTR_DBG_5_ADDR(base) ((base) + (0x0064))
#define SOC_SMMU_MSTR_RESERVED_1_ADDR(base) ((base) + (0x0068))
#define SOC_SMMU_MSTR_DBG_PORT_IN_0_ADDR(base) ((base) + (0x0070))
#define SOC_SMMU_MSTR_DBG_PORT_IN_1_ADDR(base) ((base) + (0x0074))
#define SOC_SMMU_MSTR_DBG_PORT_OUT_ADDR(base) ((base) + (0x0078))
#define SOC_SMMU_MSTR_RESERVED_2_ADDR(base) ((base) + (0x007C))
#define SOC_SMMU_MSTR_SMRX_0_ADDR(base,m) ((base) + ((m)*0x4+0x100))
#define SOC_SMMU_MSTR_SMRX_1_ADDR(base,m) ((base) + ((m)*0x4+0x260))
#define SOC_SMMU_MSTR_SMRX_2_ADDR(base,m) ((base) + ((m)*0x4+0x3C0))
#define SOC_SMMU_MSTR_RD_CMD_TOTAL_CNT_ADDR(base,q) ((base) + ((q)*0x4+0x0600))
#define SOC_SMMU_MSTR_RD_CMD_MISS_CNT_ADDR(base,q) ((base) + ((q)*0x4+0x760))
#define SOC_SMMU_MSTR_RD_DATA_TOTAL_CNT_ADDR(base,q) ((base) + ((q)*0x4+0x8C0))
#define SOC_SMMU_MSTR_RD_CMD_CASE_CNT_ADDR(base,s) ((base) + ((s)*0x4+0xA20))
#define SOC_SMMU_MSTR_RD_CMD_TRANS_LATENCY_ADDR(base) ((base) + (0x0A38))
#define SOC_SMMU_MSTR_WR_CMD_TOTAL_CNT_ADDR(base,r) ((base) + ((r)*0x4+0x0B00))
#define SOC_SMMU_MSTR_WR_CMD_MISS_CNT_ADDR(base,r) ((base) + ((r)*0x4+0xC60))
#define SOC_SMMU_MSTR_WR_DATA_TOTAL_CNT_ADDR(base,r) ((base) + ((r)*0x4+0xDC0))
#define SOC_SMMU_MSTR_WR_CMD_CASE_CNT_ADDR(base,s) ((base) + ((s)*0x4+0xF20))
#define SOC_SMMU_MSTR_WR_CMD_TRANS_LATENCY_ADDR(base) ((base) + (0x0F38))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int glb_bypass : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SMMU_MSTR_GLB_BYPASS_UNION;
#endif
#define SOC_SMMU_MSTR_GLB_BYPASS_glb_bypass_START (0)
#define SOC_SMMU_MSTR_GLB_BYPASS_glb_bypass_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int debug_mode : 1;
        unsigned int axilock_en : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_SMMU_MSTR_DEBUG_MODE_UNION;
#endif
#define SOC_SMMU_MSTR_DEBUG_MODE_debug_mode_START (0)
#define SOC_SMMU_MSTR_DEBUG_MODE_debug_mode_END (0)
#define SOC_SMMU_MSTR_DEBUG_MODE_axilock_en_START (1)
#define SOC_SMMU_MSTR_DEBUG_MODE_axilock_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mem_ctrl_s : 16;
        unsigned int mem_ctrl_d1w2r : 16;
    } reg;
} SOC_SMMU_MSTR_MEM_CTRL_UNION;
#endif
#define SOC_SMMU_MSTR_MEM_CTRL_mem_ctrl_s_START (0)
#define SOC_SMMU_MSTR_MEM_CTRL_mem_ctrl_s_END (15)
#define SOC_SMMU_MSTR_MEM_CTRL_mem_ctrl_d1w2r_START (16)
#define SOC_SMMU_MSTR_MEM_CTRL_mem_ctrl_d1w2r_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apb_clk_en : 2;
        unsigned int core_clk_en : 2;
        unsigned int reserved : 28;
    } reg;
} SOC_SMMU_MSTR_CLK_EN_UNION;
#endif
#define SOC_SMMU_MSTR_CLK_EN_apb_clk_en_START (0)
#define SOC_SMMU_MSTR_CLK_EN_apb_clk_en_END (1)
#define SOC_SMMU_MSTR_CLK_EN_core_clk_en_START (2)
#define SOC_SMMU_MSTR_CLK_EN_core_clk_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int end_req_0 : 32;
    } reg;
} SOC_SMMU_MSTR_END_REQ_0_UNION;
#endif
#define SOC_SMMU_MSTR_END_REQ_0_end_req_0_START (0)
#define SOC_SMMU_MSTR_END_REQ_0_end_req_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int end_req_1 : 32;
    } reg;
} SOC_SMMU_MSTR_END_REQ_1_UNION;
#endif
#define SOC_SMMU_MSTR_END_REQ_1_end_req_1_START (0)
#define SOC_SMMU_MSTR_END_REQ_1_end_req_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int end_req_2 : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_SMMU_MSTR_END_REQ_2_UNION;
#endif
#define SOC_SMMU_MSTR_END_REQ_2_end_req_2_START (0)
#define SOC_SMMU_MSTR_END_REQ_2_end_req_2_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int end_ack_0 : 32;
    } reg;
} SOC_SMMU_MSTR_END_ACK_0_UNION;
#endif
#define SOC_SMMU_MSTR_END_ACK_0_end_ack_0_START (0)
#define SOC_SMMU_MSTR_END_ACK_0_end_ack_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int end_ack_1 : 32;
    } reg;
} SOC_SMMU_MSTR_END_ACK_1_UNION;
#endif
#define SOC_SMMU_MSTR_END_ACK_1_end_ack_1_START (0)
#define SOC_SMMU_MSTR_END_ACK_1_end_ack_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int end_ack_2 : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_SMMU_MSTR_END_ACK_2_UNION;
#endif
#define SOC_SMMU_MSTR_END_ACK_2_end_ack_2_START (0)
#define SOC_SMMU_MSTR_END_ACK_2_end_ack_2_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stream_start_0 : 32;
    } reg;
} SOC_SMMU_MSTR_SMRX_START_0_UNION;
#endif
#define SOC_SMMU_MSTR_SMRX_START_0_stream_start_0_START (0)
#define SOC_SMMU_MSTR_SMRX_START_0_stream_start_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stream_start_1 : 32;
    } reg;
} SOC_SMMU_MSTR_SMRX_START_1_UNION;
#endif
#define SOC_SMMU_MSTR_SMRX_START_1_stream_start_1_START (0)
#define SOC_SMMU_MSTR_SMRX_START_1_stream_start_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stream_start_2 : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_SMMU_MSTR_SMRX_START_2_UNION;
#endif
#define SOC_SMMU_MSTR_SMRX_START_2_stream_start_2_START (0)
#define SOC_SMMU_MSTR_SMRX_START_2_stream_start_2_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smr_start_sel : 1;
        unsigned int end_req_sel : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_SMMU_MSTR_INPT_SEL_UNION;
#endif
#define SOC_SMMU_MSTR_INPT_SEL_smr_start_sel_START (0)
#define SOC_SMMU_MSTR_INPT_SEL_smr_start_sel_END (0)
#define SOC_SMMU_MSTR_INPT_SEL_end_req_sel_START (1)
#define SOC_SMMU_MSTR_INPT_SEL_end_req_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SMMU_MSTR_RESERVED_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_va_err0_msk : 1;
        unsigned int rd_va_err1_msk : 1;
        unsigned int wr_va_err0_msk : 1;
        unsigned int wr_va_err1_msk : 1;
        unsigned int wdata_burst_msk : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_SMMU_MSTR_INTMASK_UNION;
#endif
#define SOC_SMMU_MSTR_INTMASK_rd_va_err0_msk_START (0)
#define SOC_SMMU_MSTR_INTMASK_rd_va_err0_msk_END (0)
#define SOC_SMMU_MSTR_INTMASK_rd_va_err1_msk_START (1)
#define SOC_SMMU_MSTR_INTMASK_rd_va_err1_msk_END (1)
#define SOC_SMMU_MSTR_INTMASK_wr_va_err0_msk_START (2)
#define SOC_SMMU_MSTR_INTMASK_wr_va_err0_msk_END (2)
#define SOC_SMMU_MSTR_INTMASK_wr_va_err1_msk_START (3)
#define SOC_SMMU_MSTR_INTMASK_wr_va_err1_msk_END (3)
#define SOC_SMMU_MSTR_INTMASK_wdata_burst_msk_START (4)
#define SOC_SMMU_MSTR_INTMASK_wdata_burst_msk_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_va_err0_raw : 1;
        unsigned int rd_va_err1_raw : 1;
        unsigned int wr_va_err0_raw : 1;
        unsigned int wr_va_err1_raw : 1;
        unsigned int wdata_burst_raw : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_SMMU_MSTR_INTRAW_UNION;
#endif
#define SOC_SMMU_MSTR_INTRAW_rd_va_err0_raw_START (0)
#define SOC_SMMU_MSTR_INTRAW_rd_va_err0_raw_END (0)
#define SOC_SMMU_MSTR_INTRAW_rd_va_err1_raw_START (1)
#define SOC_SMMU_MSTR_INTRAW_rd_va_err1_raw_END (1)
#define SOC_SMMU_MSTR_INTRAW_wr_va_err0_raw_START (2)
#define SOC_SMMU_MSTR_INTRAW_wr_va_err0_raw_END (2)
#define SOC_SMMU_MSTR_INTRAW_wr_va_err1_raw_START (3)
#define SOC_SMMU_MSTR_INTRAW_wr_va_err1_raw_END (3)
#define SOC_SMMU_MSTR_INTRAW_wdata_burst_raw_START (4)
#define SOC_SMMU_MSTR_INTRAW_wdata_burst_raw_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_va_err0_stat : 1;
        unsigned int rd_va_err1_stat : 1;
        unsigned int wr_va_err0_stat : 1;
        unsigned int wr_va_err1_stat : 1;
        unsigned int wdata_burst_stat : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_SMMU_MSTR_INTSTAT_UNION;
#endif
#define SOC_SMMU_MSTR_INTSTAT_rd_va_err0_stat_START (0)
#define SOC_SMMU_MSTR_INTSTAT_rd_va_err0_stat_END (0)
#define SOC_SMMU_MSTR_INTSTAT_rd_va_err1_stat_START (1)
#define SOC_SMMU_MSTR_INTSTAT_rd_va_err1_stat_END (1)
#define SOC_SMMU_MSTR_INTSTAT_wr_va_err0_stat_START (2)
#define SOC_SMMU_MSTR_INTSTAT_wr_va_err0_stat_END (2)
#define SOC_SMMU_MSTR_INTSTAT_wr_va_err1_stat_START (3)
#define SOC_SMMU_MSTR_INTSTAT_wr_va_err1_stat_END (3)
#define SOC_SMMU_MSTR_INTSTAT_wdata_burst_stat_START (4)
#define SOC_SMMU_MSTR_INTSTAT_wdata_burst_stat_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_va_err0_clr : 1;
        unsigned int rd_va_err1_clr : 1;
        unsigned int wr_va_err0_clr : 1;
        unsigned int wr_va_err1_clr : 1;
        unsigned int wdata_burst_clr : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_SMMU_MSTR_INTCLR_UNION;
#endif
#define SOC_SMMU_MSTR_INTCLR_rd_va_err0_clr_START (0)
#define SOC_SMMU_MSTR_INTCLR_rd_va_err0_clr_END (0)
#define SOC_SMMU_MSTR_INTCLR_rd_va_err1_clr_START (1)
#define SOC_SMMU_MSTR_INTCLR_rd_va_err1_clr_END (1)
#define SOC_SMMU_MSTR_INTCLR_wr_va_err0_clr_START (2)
#define SOC_SMMU_MSTR_INTCLR_wr_va_err0_clr_END (2)
#define SOC_SMMU_MSTR_INTCLR_wr_va_err1_clr_START (3)
#define SOC_SMMU_MSTR_INTCLR_wr_va_err1_clr_END (3)
#define SOC_SMMU_MSTR_INTCLR_wdata_burst_clr_START (4)
#define SOC_SMMU_MSTR_INTCLR_wdata_burst_clr_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_sid : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SMMU_MSTR_DBG_0_UNION;
#endif
#define SOC_SMMU_MSTR_DBG_0_rd_sid_START (0)
#define SOC_SMMU_MSTR_DBG_0_rd_sid_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_addr : 32;
    } reg;
} SOC_SMMU_MSTR_DBG_1_UNION;
#endif
#define SOC_SMMU_MSTR_DBG_1_rd_addr_START (0)
#define SOC_SMMU_MSTR_DBG_1_rd_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_sid : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SMMU_MSTR_DBG_2_UNION;
#endif
#define SOC_SMMU_MSTR_DBG_2_wr_sid_START (0)
#define SOC_SMMU_MSTR_DBG_2_wr_sid_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_addr : 32;
    } reg;
} SOC_SMMU_MSTR_DBG_3_UNION;
#endif
#define SOC_SMMU_MSTR_DBG_3_wr_addr_START (0)
#define SOC_SMMU_MSTR_DBG_3_wr_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int awid : 8;
        unsigned int awlen : 8;
        unsigned int wid : 8;
        unsigned int wdata_len : 8;
    } reg;
} SOC_SMMU_MSTR_DBG_4_UNION;
#endif
#define SOC_SMMU_MSTR_DBG_4_awid_START (0)
#define SOC_SMMU_MSTR_DBG_4_awid_END (7)
#define SOC_SMMU_MSTR_DBG_4_awlen_START (8)
#define SOC_SMMU_MSTR_DBG_4_awlen_END (15)
#define SOC_SMMU_MSTR_DBG_4_wid_START (16)
#define SOC_SMMU_MSTR_DBG_4_wid_END (23)
#define SOC_SMMU_MSTR_DBG_4_wdata_len_START (24)
#define SOC_SMMU_MSTR_DBG_4_wdata_len_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int awaddr : 32;
    } reg;
} SOC_SMMU_MSTR_DBG_5_UNION;
#endif
#define SOC_SMMU_MSTR_DBG_5_awaddr_START (0)
#define SOC_SMMU_MSTR_DBG_5_awaddr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SMMU_MSTR_RESERVED_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_cfg_addr : 16;
        unsigned int dbg_cfg_cs : 1;
        unsigned int dbg_cfg_wr : 1;
        unsigned int reserved : 14;
    } reg;
} SOC_SMMU_MSTR_DBG_PORT_IN_0_UNION;
#endif
#define SOC_SMMU_MSTR_DBG_PORT_IN_0_dbg_cfg_addr_START (0)
#define SOC_SMMU_MSTR_DBG_PORT_IN_0_dbg_cfg_addr_END (15)
#define SOC_SMMU_MSTR_DBG_PORT_IN_0_dbg_cfg_cs_START (16)
#define SOC_SMMU_MSTR_DBG_PORT_IN_0_dbg_cfg_cs_END (16)
#define SOC_SMMU_MSTR_DBG_PORT_IN_0_dbg_cfg_wr_START (17)
#define SOC_SMMU_MSTR_DBG_PORT_IN_0_dbg_cfg_wr_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_cfg_wdata : 32;
    } reg;
} SOC_SMMU_MSTR_DBG_PORT_IN_1_UNION;
#endif
#define SOC_SMMU_MSTR_DBG_PORT_IN_1_dbg_cfg_wdata_START (0)
#define SOC_SMMU_MSTR_DBG_PORT_IN_1_dbg_cfg_wdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_cfg_rdata : 32;
    } reg;
} SOC_SMMU_MSTR_DBG_PORT_OUT_UNION;
#endif
#define SOC_SMMU_MSTR_DBG_PORT_OUT_dbg_cfg_rdata_START (0)
#define SOC_SMMU_MSTR_DBG_PORT_OUT_dbg_cfg_rdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SMMU_MSTR_RESERVED_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bypass : 1;
        unsigned int reserved_0: 3;
        unsigned int upwin : 8;
        unsigned int len : 8;
        unsigned int reserved_1: 12;
    } reg;
} SOC_SMMU_MSTR_SMRX_0_UNION;
#endif
#define SOC_SMMU_MSTR_SMRX_0_bypass_START (0)
#define SOC_SMMU_MSTR_SMRX_0_bypass_END (0)
#define SOC_SMMU_MSTR_SMRX_0_upwin_START (4)
#define SOC_SMMU_MSTR_SMRX_0_upwin_END (11)
#define SOC_SMMU_MSTR_SMRX_0_len_START (12)
#define SOC_SMMU_MSTR_SMRX_0_len_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 15;
        unsigned int va_str : 17;
    } reg;
} SOC_SMMU_MSTR_SMRX_1_UNION;
#endif
#define SOC_SMMU_MSTR_SMRX_1_va_str_START (15)
#define SOC_SMMU_MSTR_SMRX_1_va_str_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 15;
        unsigned int va_end : 17;
    } reg;
} SOC_SMMU_MSTR_SMRX_2_UNION;
#endif
#define SOC_SMMU_MSTR_SMRX_2_va_end_START (15)
#define SOC_SMMU_MSTR_SMRX_2_va_end_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_cmd_total_cnt : 32;
    } reg;
} SOC_SMMU_MSTR_RD_CMD_TOTAL_CNT_UNION;
#endif
#define SOC_SMMU_MSTR_RD_CMD_TOTAL_CNT_rd_cmd_total_cnt_START (0)
#define SOC_SMMU_MSTR_RD_CMD_TOTAL_CNT_rd_cmd_total_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_cmd_miss_cnt : 32;
    } reg;
} SOC_SMMU_MSTR_RD_CMD_MISS_CNT_UNION;
#endif
#define SOC_SMMU_MSTR_RD_CMD_MISS_CNT_rd_cmd_miss_cnt_START (0)
#define SOC_SMMU_MSTR_RD_CMD_MISS_CNT_rd_cmd_miss_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_data_total_cnt : 32;
    } reg;
} SOC_SMMU_MSTR_RD_DATA_TOTAL_CNT_UNION;
#endif
#define SOC_SMMU_MSTR_RD_DATA_TOTAL_CNT_rd_data_total_cnt_START (0)
#define SOC_SMMU_MSTR_RD_DATA_TOTAL_CNT_rd_data_total_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_cmd_case_cnt : 32;
    } reg;
} SOC_SMMU_MSTR_RD_CMD_CASE_CNT_UNION;
#endif
#define SOC_SMMU_MSTR_RD_CMD_CASE_CNT_rd_cmd_case_cnt_START (0)
#define SOC_SMMU_MSTR_RD_CMD_CASE_CNT_rd_cmd_case_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_cmd_max_latency : 16;
        unsigned int rd_cmd_avg_latency : 16;
    } reg;
} SOC_SMMU_MSTR_RD_CMD_TRANS_LATENCY_UNION;
#endif
#define SOC_SMMU_MSTR_RD_CMD_TRANS_LATENCY_rd_cmd_max_latency_START (0)
#define SOC_SMMU_MSTR_RD_CMD_TRANS_LATENCY_rd_cmd_max_latency_END (15)
#define SOC_SMMU_MSTR_RD_CMD_TRANS_LATENCY_rd_cmd_avg_latency_START (16)
#define SOC_SMMU_MSTR_RD_CMD_TRANS_LATENCY_rd_cmd_avg_latency_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_cmd_total_cnt : 32;
    } reg;
} SOC_SMMU_MSTR_WR_CMD_TOTAL_CNT_UNION;
#endif
#define SOC_SMMU_MSTR_WR_CMD_TOTAL_CNT_wr_cmd_total_cnt_START (0)
#define SOC_SMMU_MSTR_WR_CMD_TOTAL_CNT_wr_cmd_total_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_cmd_miss_cnt : 32;
    } reg;
} SOC_SMMU_MSTR_WR_CMD_MISS_CNT_UNION;
#endif
#define SOC_SMMU_MSTR_WR_CMD_MISS_CNT_wr_cmd_miss_cnt_START (0)
#define SOC_SMMU_MSTR_WR_CMD_MISS_CNT_wr_cmd_miss_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_data_total_cnt : 32;
    } reg;
} SOC_SMMU_MSTR_WR_DATA_TOTAL_CNT_UNION;
#endif
#define SOC_SMMU_MSTR_WR_DATA_TOTAL_CNT_wr_data_total_cnt_START (0)
#define SOC_SMMU_MSTR_WR_DATA_TOTAL_CNT_wr_data_total_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_cmd_case_cnt : 32;
    } reg;
} SOC_SMMU_MSTR_WR_CMD_CASE_CNT_UNION;
#endif
#define SOC_SMMU_MSTR_WR_CMD_CASE_CNT_wr_cmd_case_cnt_START (0)
#define SOC_SMMU_MSTR_WR_CMD_CASE_CNT_wr_cmd_case_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_cmd_max_latency : 16;
        unsigned int wr_cmd_avg_latency : 16;
    } reg;
} SOC_SMMU_MSTR_WR_CMD_TRANS_LATENCY_UNION;
#endif
#define SOC_SMMU_MSTR_WR_CMD_TRANS_LATENCY_wr_cmd_max_latency_START (0)
#define SOC_SMMU_MSTR_WR_CMD_TRANS_LATENCY_wr_cmd_max_latency_END (15)
#define SOC_SMMU_MSTR_WR_CMD_TRANS_LATENCY_wr_cmd_avg_latency_START (16)
#define SOC_SMMU_MSTR_WR_CMD_TRANS_LATENCY_wr_cmd_avg_latency_END (31)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
