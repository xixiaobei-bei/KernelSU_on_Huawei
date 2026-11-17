#ifndef __SOC_ICS_INTERFACE_H__
#define __SOC_ICS_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_ICS_START_ADDR(base) ((base) + (0x0018))
#define SOC_ICS_STATUS_ADDR(base) ((base) + (0x0020))
#define SOC_ICS_BASE_ADDR_ADDR(base) ((base) + (0x0028))
#define SOC_ICS_MEM_CTRL_ADDR(base) ((base) + (0x0030))
#define SOC_ICS_VERSION_ADDR(base) ((base) + (0x0040))
#define SOC_ICS_CLK_GT_ADDR(base) ((base) + (0x0068))
#define SOC_NPU_SSWQ_WORK_QUEUE_INFO_ADDR(base) ((base) + (0x0074))
#define SOC_NPU_SSWQ_WORK_QUEUE_FINISH_ADDR(base) ((base) + (0x0078))
#define SOC_NPU_SSWQ_SELF_START_CONFIG_ADDR(base) ((base) + (0x007C))
#define SOC_NPU_SSWQ_SELF_START_LO_ADDR(base) ((base) + (0x0080))
#define SOC_NPU_SSWQ_SELF_START_HI_ADDR(base) ((base) + (0x0084))
#define SOC_NPU_SSWQ_SELF_START_SIZE_ADDR(base) ((base) + (0x0088))
#define SOC_NPU_SSWQ_SELF_START_ID_ADDR(base) ((base) + (0x008C))
#define SOC_NPU_SSWQ_PUSH_WORK_QUEUE_ADDR(base) ((base) + (0x0090))
#define SOC_NPU_SSWQ_SOFT_RESET_ADDR(base) ((base) + (0x00A8))
#define SOC_NPU_SSWQ_DDR_BASE_ADDR(base) ((base) + (0x00C0))
#define SOC_NPU_IE_ADDR(base) ((base) + (0x0098))
#define SOC_NPU_IS_ADDR(base) ((base) + (0x00A0))
#define SOC_NPU_INTERRUPT_INST_ADDR(base) ((base) + (0x00AC))
#define SOC_NPU_INTERRUPT_PC_ADDR(base) ((base) + (0x00B0))
#define SOC_NPU_CONTROL_ID_ADDR(base) ((base) + (0x0200))
#define SOC_NPU_CONTROL_TIMER_ADDR(base) ((base) + (0x0210))
#define SOC_NPU_WATCH_DOG_TIMER_LOW_ADDR(base) ((base) + (0x0204))
#define SOC_NPU_WATCH_DOG_TIMER_HIGH_ADDR(base) ((base) + (0x0208))
#define SOC_NPU_WATCH_DOG_ADDR(base) ((base) + (0x020C))
#define SOC_NPU_ID_ADDR(base) ((base) + (0x40000))
#define SOC_NPU_TIMER_ADDR(base) ((base) + (0x40008))
#define SOC_ICS_IRQ_MASK_NS_ADDR(base) ((base) + (0x0000))
#define SOC_ICS_IRQ_RAW_NS_ADDR(base) ((base) + (0x0004))
#define SOC_ICS_IRQ_STAT_NS_ADDR(base) ((base) + (0x0008))
#define SOC_ICS_IRQ_CLR_NS_ADDR(base) ((base) + (0x000C))
#define SOC_ICS_IRQ_MASK_S_ADDR(base) ((base) + (0x0010))
#define SOC_ICS_IRQ_RAW_S_ADDR(base) ((base) + (0x0014))
#define SOC_ICS_IRQ_STAT_S_ADDR(base) ((base) + (0x0018))
#define SOC_ICS_IRQ_CLR_S_ADDR(base) ((base) + (0x001C))
#define SOC_ICS_IRQ_MASK_LPMCU_ADDR(base) ((base) + (0x0020))
#define SOC_ICS_IRQ_RAW_LPMCU_ADDR(base) ((base) + (0x0024))
#define SOC_ICS_IRQ_STAT_LPMCU_ADDR(base) ((base) + (0x0028))
#define SOC_ICS_IRQ_CLR_LPMCU_ADDR(base) ((base) + (0x002C))
#define SOC_ICS_IRQ_MASK_IOMCU_ADDR(base) ((base) + (0x0030))
#define SOC_ICS_IRQ_RAW_IOMCU_ADDR(base) ((base) + (0x0034))
#define SOC_ICS_IRQ_STAT_IOMCU_ADDR(base) ((base) + (0x0038))
#define SOC_ICS_IRQ_CLR_IOMCU_ADDR(base) ((base) + (0x003C))
#define SOC_ICS_IRQ_MASK_ISPCPU_ADDR(base) ((base) + (0x0040))
#define SOC_ICS_IRQ_RAW_ISPCPU_ADDR(base) ((base) + (0x0044))
#define SOC_ICS_IRQ_STAT_ISPCPU_ADDR(base) ((base) + (0x0048))
#define SOC_ICS_IRQ_CLR_ISPCPU_ADDR(base) ((base) + (0x004C))
#define SOC_ICS_IRQ_MASK_IVP_ADDR(base) ((base) + (0x0050))
#define SOC_ICS_IRQ_RAW_IVP_ADDR(base) ((base) + (0x0054))
#define SOC_ICS_IRQ_STAT_IVP_ADDR(base) ((base) + (0x0058))
#define SOC_ICS_IRQ_CLR_IVP_ADDR(base) ((base) + (0x005C))
#define SOC_ICS_IRQ_MASK_HIFI_ADDR(base) ((base) + (0x0060))
#define SOC_ICS_IRQ_RAW_HIFI_ADDR(base) ((base) + (0x0064))
#define SOC_ICS_IRQ_STAT_HIFI_ADDR(base) ((base) + (0x0068))
#define SOC_ICS_IRQ_CLR_HIFI_ADDR(base) ((base) + (0x006C))
#define SOC_ICS_MID_ADDR(base) ((base) + (0x0070))
#define SOC_ICS_SOFT_RST_REQ_ADDR(base) ((base) + (0x0074))
#define SOC_ICS_SOFT_RST_ACK_ADDR(base) ((base) + (0x0078))
#define SOC_ICS_SOFT_RST_STATE0_ADDR(base) ((base) + (0x007C))
#define SOC_ICS_SOFT_RST_STATE1_ADDR(base) ((base) + (0x0080))
#define SOC_ICS_PERF_CNT_CLEAR_ADDR(base) ((base) + (0x0084))
#define SOC_ICS_PERF_CNT_CLK_GT_ADDR(base) ((base) + (0x0088))
#define SOC_ICS_FRAME_CYC_CNT_ADDR(base) ((base) + (0x008C))
#define SOC_ICS_FU_IDLE_CNT_ADDR(base) ((base) + (0x0090))
#define SOC_ICS_IO_IDLE_CNT_ADDR(base) ((base) + (0x0094))
#define SOC_ICS_ALL_IDLE_CNT_ADDR(base) ((base) + (0x0098))
#define SOC_ICS_ALL_BUSY_CNT_ADDR(base) ((base) + (0x009C))
#define SOC_ICS_MAX_OSD_ADDR(base) ((base) + (0x00A0))
#define SOC_ICS_OSD_CNT_ADDR(base) ((base) + (0x00A4))
#define SOC_ICS_FRAME_CNT_ADDR(base) ((base) + (0x00A8))
#define SOC_ICS_FRAME_CNT_CLEAR_ADDR(base) ((base) + (0x00AC))
#define SOC_ICS_BC_READ_LMT_ADDR(base) ((base) + (0x00B0))
#define SOC_ICS_BC_WRITE_LMT_ADDR(base) ((base) + (0x00B4))
#define SOC_ICS_IRQ_MASK_P_ADDR(base) ((base) + (0x0))
#define SOC_ICS_IRQ_RAW_P_ADDR(base) ((base) + (0x4))
#define SOC_ICS_IRQ_STAT_P_ADDR(base) ((base) + (0x8))
#define SOC_ICS_IRQ_CLR_P_ADDR(base) ((base) + (0xC))
#define SOC_ICS_SMMU_MSTR_GLB_BYPASS_ADDR(base) ((base) + (0x0000))
#define SOC_ICS_SMMU_MSTR_DEBUG_MODE_ADDR(base) ((base) + (0x0004))
#define SOC_ICS_SMMU_MSTR_MEM_CTRL_ADDR(base) ((base) + (0x0008))
#define SOC_ICS_SMMU_MSTR_CLK_EN_ADDR(base) ((base) + (0x000C))
#define SOC_ICS_SMMU_MSTR_END_REQ_0_ADDR(base) ((base) + (0x0010))
#define SOC_ICS_SMMU_MSTR_END_REQ_1_ADDR(base) ((base) + (0x0014))
#define SOC_ICS_SMMU_MSTR_END_REQ_2_ADDR(base) ((base) + (0x0018))
#define SOC_ICS_SMMU_MSTR_END_ACK_0_ADDR(base) ((base) + (0x001C))
#define SOC_ICS_SMMU_MSTR_END_ACK_1_ADDR(base) ((base) + (0x0020))
#define SOC_ICS_SMMU_MSTR_END_ACK_2_ADDR(base) ((base) + (0x0024))
#define SOC_ICS_SMMU_MSTR_SMRX_START_0_ADDR(base) ((base) + (0x0028))
#define SOC_ICS_SMMU_MSTR_SMRX_START_1_ADDR(base) ((base) + (0x002C))
#define SOC_ICS_SMMU_MSTR_SMRX_START_2_ADDR(base) ((base) + (0x0030))
#define SOC_ICS_SMMU_MSTR_INPT_SEL_ADDR(base) ((base) + (0x0034))
#define SOC_ICS_SMMU_MSTR_RESERVED_0_ADDR(base) ((base) + (0x0038))
#define SOC_ICS_SMMU_MSTR_INTMASK_ADDR(base) ((base) + (0x0040))
#define SOC_ICS_SMMU_MSTR_INTRAW_ADDR(base) ((base) + (0x0044))
#define SOC_ICS_SMMU_MSTR_INTSTAT_ADDR(base) ((base) + (0x0048))
#define SOC_ICS_SMMU_MSTR_INTCLR_ADDR(base) ((base) + (0x004C))
#define SOC_ICS_SMMU_MSTR_DBG_0_ADDR(base) ((base) + (0x0050))
#define SOC_ICS_SMMU_MSTR_DBG_1_ADDR(base) ((base) + (0x0054))
#define SOC_ICS_SMMU_MSTR_DBG_2_ADDR(base) ((base) + (0x0058))
#define SOC_ICS_SMMU_MSTR_DBG_3_ADDR(base) ((base) + (0x005C))
#define SOC_ICS_SMMU_MSTR_DBG_4_ADDR(base) ((base) + (0x0060))
#define SOC_ICS_SMMU_MSTR_DBG_5_ADDR(base) ((base) + (0x0064))
#define SOC_ICS_SMMU_MSTR_RESERVED_1_ADDR(base) ((base) + (0x0068))
#define SOC_ICS_SMMU_MSTR_DBG_PORT_IN_0_ADDR(base) ((base) + (0x0070))
#define SOC_ICS_SMMU_MSTR_DBG_PORT_IN_1_ADDR(base) ((base) + (0x0074))
#define SOC_ICS_SMMU_MSTR_DBG_PORT_OUT_ADDR(base) ((base) + (0x0078))
#define SOC_ICS_SMMU_MSTR_RESERVED_2_ADDR(base) ((base) + (0x007C))
#define SOC_ICS_SMMU_MSTR_SMRX_0_ADDR(base,m) ((base) + ((m)*0x4+0x100))
#define SOC_ICS_SMMU_MSTR_SMRX_1_ADDR(base,m) ((base) + ((m)*0x4+0x260))
#define SOC_ICS_SMMU_MSTR_SMRX_2_ADDR(base,m) ((base) + ((m)*0x4+0x3C0))
#define SOC_ICS_RD_CMD_TOTAL_CNT_ADDR(base,q) ((base) + ((q)*0x4+0x0600))
#define SOC_ICS_RD_CMD_MISS_CNT_ADDR(base,q) ((base) + ((q)*0x4+0x760))
#define SOC_ICS_RD_DATA_TOTAL_CNT_ADDR(base,q) ((base) + ((q)*0x4+0x8C0))
#define SOC_ICS_RD_CMD_CASE_CNT_ADDR(base,s) ((base) + ((s)*0x4+0xA20))
#define SOC_ICS_RD_CMD_TRANS_LATENCY_ADDR(base) ((base) + (0x0A38))
#define SOC_ICS_WR_CMD_TOTAL_CNT_ADDR(base,r) ((base) + ((r)*0x4+0x0B00))
#define SOC_ICS_WR_CMD_MISS_CNT_ADDR(base,r) ((base) + ((r)*0x4+0xC60))
#define SOC_ICS_WR_DATA_TOTAL_CNT_ADDR(base,r) ((base) + ((r)*0x4+0xDC0))
#define SOC_ICS_WR_CMD_CASE_CNT_ADDR(base,s) ((base) + ((s)*0x4+0xF20))
#define SOC_ICS_WR_CMD_TRANS_LATENCY_ADDR(base) ((base) + (0x0F38))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ics_start : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_START_UNION;
#endif
#define SOC_ICS_START_ics_start_START (0)
#define SOC_ICS_START_ics_start_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ics_status : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_STATUS_UNION;
#endif
#define SOC_ICS_STATUS_ics_status_START (0)
#define SOC_ICS_STATUS_ics_status_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ics_base_addr : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_ICS_BASE_ADDR_UNION;
#endif
#define SOC_ICS_BASE_ADDR_ics_base_addr_START (0)
#define SOC_ICS_BASE_ADDR_ics_base_addr_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ics_mem_ctrl_s : 26;
        unsigned int reserved : 6;
    } reg;
} SOC_ICS_MEM_CTRL_UNION;
#endif
#define SOC_ICS_MEM_CTRL_ics_mem_ctrl_s_START (0)
#define SOC_ICS_MEM_CTRL_ics_mem_ctrl_s_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ics_version : 32;
    } reg;
} SOC_ICS_VERSION_UNION;
#endif
#define SOC_ICS_VERSION_ics_version_START (0)
#define SOC_ICS_VERSION_ics_version_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ics_clk_gt : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_CLK_GT_UNION;
#endif
#define SOC_ICS_CLK_GT_ics_clk_gt_START (0)
#define SOC_ICS_CLK_GT_ics_clk_gt_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_mask_ns : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_MASK_NS_UNION;
#endif
#define SOC_ICS_IRQ_MASK_NS_finish_irq_mask_ns_START (0)
#define SOC_ICS_IRQ_MASK_NS_finish_irq_mask_ns_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_raw_ns : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_RAW_NS_UNION;
#endif
#define SOC_ICS_IRQ_RAW_NS_finish_irq_raw_ns_START (0)
#define SOC_ICS_IRQ_RAW_NS_finish_irq_raw_ns_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_stat_ns : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_STAT_NS_UNION;
#endif
#define SOC_ICS_IRQ_STAT_NS_finish_irq_stat_ns_START (0)
#define SOC_ICS_IRQ_STAT_NS_finish_irq_stat_ns_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_clr_ns : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_CLR_NS_UNION;
#endif
#define SOC_ICS_IRQ_CLR_NS_finish_irq_clr_ns_START (0)
#define SOC_ICS_IRQ_CLR_NS_finish_irq_clr_ns_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_mask_s : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_MASK_S_UNION;
#endif
#define SOC_ICS_IRQ_MASK_S_finish_irq_mask_s_START (0)
#define SOC_ICS_IRQ_MASK_S_finish_irq_mask_s_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_raw_s : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_RAW_S_UNION;
#endif
#define SOC_ICS_IRQ_RAW_S_finish_irq_raw_s_START (0)
#define SOC_ICS_IRQ_RAW_S_finish_irq_raw_s_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_stat_s : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_STAT_S_UNION;
#endif
#define SOC_ICS_IRQ_STAT_S_finish_irq_stat_s_START (0)
#define SOC_ICS_IRQ_STAT_S_finish_irq_stat_s_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_clr_s : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_CLR_S_UNION;
#endif
#define SOC_ICS_IRQ_CLR_S_finish_irq_clr_s_START (0)
#define SOC_ICS_IRQ_CLR_S_finish_irq_clr_s_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_mask_lpmcu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_MASK_LPMCU_UNION;
#endif
#define SOC_ICS_IRQ_MASK_LPMCU_finish_irq_mask_lpmcu_START (0)
#define SOC_ICS_IRQ_MASK_LPMCU_finish_irq_mask_lpmcu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_raw_lpmcu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_RAW_LPMCU_UNION;
#endif
#define SOC_ICS_IRQ_RAW_LPMCU_finish_irq_raw_lpmcu_START (0)
#define SOC_ICS_IRQ_RAW_LPMCU_finish_irq_raw_lpmcu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_stat_lpmcu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_STAT_LPMCU_UNION;
#endif
#define SOC_ICS_IRQ_STAT_LPMCU_finish_irq_stat_lpmcu_START (0)
#define SOC_ICS_IRQ_STAT_LPMCU_finish_irq_stat_lpmcu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_clr_lpmcu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_CLR_LPMCU_UNION;
#endif
#define SOC_ICS_IRQ_CLR_LPMCU_finish_irq_clr_lpmcu_START (0)
#define SOC_ICS_IRQ_CLR_LPMCU_finish_irq_clr_lpmcu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_mask_iomcu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_MASK_IOMCU_UNION;
#endif
#define SOC_ICS_IRQ_MASK_IOMCU_finish_irq_mask_iomcu_START (0)
#define SOC_ICS_IRQ_MASK_IOMCU_finish_irq_mask_iomcu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_raw_iomcu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_RAW_IOMCU_UNION;
#endif
#define SOC_ICS_IRQ_RAW_IOMCU_finish_irq_raw_iomcu_START (0)
#define SOC_ICS_IRQ_RAW_IOMCU_finish_irq_raw_iomcu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_stat_iomcu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_STAT_IOMCU_UNION;
#endif
#define SOC_ICS_IRQ_STAT_IOMCU_finish_irq_stat_iomcu_START (0)
#define SOC_ICS_IRQ_STAT_IOMCU_finish_irq_stat_iomcu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_clr_iomcu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_CLR_IOMCU_UNION;
#endif
#define SOC_ICS_IRQ_CLR_IOMCU_finish_irq_clr_iomcu_START (0)
#define SOC_ICS_IRQ_CLR_IOMCU_finish_irq_clr_iomcu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_mask_ispcpu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_MASK_ISPCPU_UNION;
#endif
#define SOC_ICS_IRQ_MASK_ISPCPU_finish_irq_mask_ispcpu_START (0)
#define SOC_ICS_IRQ_MASK_ISPCPU_finish_irq_mask_ispcpu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_raw_ispcpu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_RAW_ISPCPU_UNION;
#endif
#define SOC_ICS_IRQ_RAW_ISPCPU_finish_irq_raw_ispcpu_START (0)
#define SOC_ICS_IRQ_RAW_ISPCPU_finish_irq_raw_ispcpu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_stat_ispcpu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_STAT_ISPCPU_UNION;
#endif
#define SOC_ICS_IRQ_STAT_ISPCPU_finish_irq_stat_ispcpu_START (0)
#define SOC_ICS_IRQ_STAT_ISPCPU_finish_irq_stat_ispcpu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_clr_ispcpu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_CLR_ISPCPU_UNION;
#endif
#define SOC_ICS_IRQ_CLR_ISPCPU_finish_irq_clr_ispcpu_START (0)
#define SOC_ICS_IRQ_CLR_ISPCPU_finish_irq_clr_ispcpu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_mask_ivp : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_MASK_IVP_UNION;
#endif
#define SOC_ICS_IRQ_MASK_IVP_finish_irq_mask_ivp_START (0)
#define SOC_ICS_IRQ_MASK_IVP_finish_irq_mask_ivp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_raw_ivp : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_RAW_IVP_UNION;
#endif
#define SOC_ICS_IRQ_RAW_IVP_finish_irq_raw_ivp_START (0)
#define SOC_ICS_IRQ_RAW_IVP_finish_irq_raw_ivp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_stat_ivp : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_STAT_IVP_UNION;
#endif
#define SOC_ICS_IRQ_STAT_IVP_finish_irq_stat_ivp_START (0)
#define SOC_ICS_IRQ_STAT_IVP_finish_irq_stat_ivp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_clr_ivp : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_CLR_IVP_UNION;
#endif
#define SOC_ICS_IRQ_CLR_IVP_finish_irq_clr_ivp_START (0)
#define SOC_ICS_IRQ_CLR_IVP_finish_irq_clr_ivp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_mask_hifi : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_MASK_HIFI_UNION;
#endif
#define SOC_ICS_IRQ_MASK_HIFI_finish_irq_mask_hifi_START (0)
#define SOC_ICS_IRQ_MASK_HIFI_finish_irq_mask_hifi_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_raw_hifi : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_RAW_HIFI_UNION;
#endif
#define SOC_ICS_IRQ_RAW_HIFI_finish_irq_raw_hifi_START (0)
#define SOC_ICS_IRQ_RAW_HIFI_finish_irq_raw_hifi_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_stat_hifi : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_STAT_HIFI_UNION;
#endif
#define SOC_ICS_IRQ_STAT_HIFI_finish_irq_stat_hifi_START (0)
#define SOC_ICS_IRQ_STAT_HIFI_finish_irq_stat_hifi_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_clr_hifi : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_CLR_HIFI_UNION;
#endif
#define SOC_ICS_IRQ_CLR_HIFI_finish_irq_clr_hifi_START (0)
#define SOC_ICS_IRQ_CLR_HIFI_finish_irq_clr_hifi_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ics_mid : 6;
        unsigned int reserved : 26;
    } reg;
} SOC_ICS_MID_UNION;
#endif
#define SOC_ICS_MID_ics_mid_START (0)
#define SOC_ICS_MID_ics_mid_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ics_soft_rst_req : 1;
        unsigned int ics_soft_rst_force_ack : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_ICS_SOFT_RST_REQ_UNION;
#endif
#define SOC_ICS_SOFT_RST_REQ_ics_soft_rst_req_START (0)
#define SOC_ICS_SOFT_RST_REQ_ics_soft_rst_req_END (0)
#define SOC_ICS_SOFT_RST_REQ_ics_soft_rst_force_ack_START (1)
#define SOC_ICS_SOFT_RST_REQ_ics_soft_rst_force_ack_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ics_soft_rst_ack : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_SOFT_RST_ACK_UNION;
#endif
#define SOC_ICS_SOFT_RST_ACK_ics_soft_rst_ack_START (0)
#define SOC_ICS_SOFT_RST_ACK_ics_soft_rst_ack_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ics_soft_rst_stat0 : 32;
    } reg;
} SOC_ICS_SOFT_RST_STATE0_UNION;
#endif
#define SOC_ICS_SOFT_RST_STATE0_ics_soft_rst_stat0_START (0)
#define SOC_ICS_SOFT_RST_STATE0_ics_soft_rst_stat0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ics_soft_rst_stat1 : 32;
    } reg;
} SOC_ICS_SOFT_RST_STATE1_UNION;
#endif
#define SOC_ICS_SOFT_RST_STATE1_ics_soft_rst_stat1_START (0)
#define SOC_ICS_SOFT_RST_STATE1_ics_soft_rst_stat1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int perf_cnt_clear : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_PERF_CNT_CLEAR_UNION;
#endif
#define SOC_ICS_PERF_CNT_CLEAR_perf_cnt_clear_START (0)
#define SOC_ICS_PERF_CNT_CLEAR_perf_cnt_clear_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int perf_cnt_clk_gt : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_PERF_CNT_CLK_GT_UNION;
#endif
#define SOC_ICS_PERF_CNT_CLK_GT_perf_cnt_clk_gt_START (0)
#define SOC_ICS_PERF_CNT_CLK_GT_perf_cnt_clk_gt_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int frame_cyc_cnt : 32;
    } reg;
} SOC_ICS_FRAME_CYC_CNT_UNION;
#endif
#define SOC_ICS_FRAME_CYC_CNT_frame_cyc_cnt_START (0)
#define SOC_ICS_FRAME_CYC_CNT_frame_cyc_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fu_idle_cnt : 32;
    } reg;
} SOC_ICS_FU_IDLE_CNT_UNION;
#endif
#define SOC_ICS_FU_IDLE_CNT_fu_idle_cnt_START (0)
#define SOC_ICS_FU_IDLE_CNT_fu_idle_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int io_idle_cnt : 32;
    } reg;
} SOC_ICS_IO_IDLE_CNT_UNION;
#endif
#define SOC_ICS_IO_IDLE_CNT_io_idle_cnt_START (0)
#define SOC_ICS_IO_IDLE_CNT_io_idle_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int all_idle_cnt : 32;
    } reg;
} SOC_ICS_ALL_IDLE_CNT_UNION;
#endif
#define SOC_ICS_ALL_IDLE_CNT_all_idle_cnt_START (0)
#define SOC_ICS_ALL_IDLE_CNT_all_idle_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int all_busy_cnt : 32;
    } reg;
} SOC_ICS_ALL_BUSY_CNT_UNION;
#endif
#define SOC_ICS_ALL_BUSY_CNT_all_busy_cnt_START (0)
#define SOC_ICS_ALL_BUSY_CNT_all_busy_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_osd_max : 8;
        unsigned int rd_osd_max : 8;
        unsigned int reserved : 16;
    } reg;
} SOC_ICS_MAX_OSD_UNION;
#endif
#define SOC_ICS_MAX_OSD_wr_osd_max_START (0)
#define SOC_ICS_MAX_OSD_wr_osd_max_END (7)
#define SOC_ICS_MAX_OSD_rd_osd_max_START (8)
#define SOC_ICS_MAX_OSD_rd_osd_max_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_osd_cnt : 8;
        unsigned int rd_osd_cnt : 8;
        unsigned int reserved : 16;
    } reg;
} SOC_ICS_OSD_CNT_UNION;
#endif
#define SOC_ICS_OSD_CNT_wr_osd_cnt_START (0)
#define SOC_ICS_OSD_CNT_wr_osd_cnt_END (7)
#define SOC_ICS_OSD_CNT_rd_osd_cnt_START (8)
#define SOC_ICS_OSD_CNT_rd_osd_cnt_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ics_start_cnt : 16;
        unsigned int ics_finish_cnt : 16;
    } reg;
} SOC_ICS_FRAME_CNT_UNION;
#endif
#define SOC_ICS_FRAME_CNT_ics_start_cnt_START (0)
#define SOC_ICS_FRAME_CNT_ics_start_cnt_END (15)
#define SOC_ICS_FRAME_CNT_ics_finish_cnt_START (16)
#define SOC_ICS_FRAME_CNT_ics_finish_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int start_cnt_clear : 1;
        unsigned int reserved_0 : 3;
        unsigned int finish_cnt_clear : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_ICS_FRAME_CNT_CLEAR_UNION;
#endif
#define SOC_ICS_FRAME_CNT_CLEAR_start_cnt_clear_START (0)
#define SOC_ICS_FRAME_CNT_CLEAR_start_cnt_clear_END (0)
#define SOC_ICS_FRAME_CNT_CLEAR_finish_cnt_clear_START (4)
#define SOC_ICS_FRAME_CNT_CLEAR_finish_cnt_clear_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_mask_p : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_MASK_P_UNION;
#endif
#define SOC_ICS_IRQ_MASK_P_finish_irq_mask_p_START (0)
#define SOC_ICS_IRQ_MASK_P_finish_irq_mask_p_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_raw_p : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_RAW_P_UNION;
#endif
#define SOC_ICS_IRQ_RAW_P_finish_irq_raw_p_START (0)
#define SOC_ICS_IRQ_RAW_P_finish_irq_raw_p_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_stat_p : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_STAT_P_UNION;
#endif
#define SOC_ICS_IRQ_STAT_P_finish_irq_stat_p_START (0)
#define SOC_ICS_IRQ_STAT_P_finish_irq_stat_p_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int finish_irq_clr_p : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_IRQ_CLR_P_UNION;
#endif
#define SOC_ICS_IRQ_CLR_P_finish_irq_clr_p_START (0)
#define SOC_ICS_IRQ_CLR_P_finish_irq_clr_p_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int glb_bypass : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ICS_SMMU_MSTR_GLB_BYPASS_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_GLB_BYPASS_glb_bypass_START (0)
#define SOC_ICS_SMMU_MSTR_GLB_BYPASS_glb_bypass_END (0)
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
} SOC_ICS_SMMU_MSTR_DEBUG_MODE_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_DEBUG_MODE_debug_mode_START (0)
#define SOC_ICS_SMMU_MSTR_DEBUG_MODE_debug_mode_END (0)
#define SOC_ICS_SMMU_MSTR_DEBUG_MODE_axilock_en_START (1)
#define SOC_ICS_SMMU_MSTR_DEBUG_MODE_axilock_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mem_ctrl_s : 16;
        unsigned int mem_ctrl_d1w2r : 16;
    } reg;
} SOC_ICS_SMMU_MSTR_MEM_CTRL_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_MEM_CTRL_mem_ctrl_s_START (0)
#define SOC_ICS_SMMU_MSTR_MEM_CTRL_mem_ctrl_s_END (15)
#define SOC_ICS_SMMU_MSTR_MEM_CTRL_mem_ctrl_d1w2r_START (16)
#define SOC_ICS_SMMU_MSTR_MEM_CTRL_mem_ctrl_d1w2r_END (31)
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
} SOC_ICS_SMMU_MSTR_CLK_EN_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_CLK_EN_apb_clk_en_START (0)
#define SOC_ICS_SMMU_MSTR_CLK_EN_apb_clk_en_END (1)
#define SOC_ICS_SMMU_MSTR_CLK_EN_core_clk_en_START (2)
#define SOC_ICS_SMMU_MSTR_CLK_EN_core_clk_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int end_req_0 : 32;
    } reg;
} SOC_ICS_SMMU_MSTR_END_REQ_0_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_END_REQ_0_end_req_0_START (0)
#define SOC_ICS_SMMU_MSTR_END_REQ_0_end_req_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int end_req_1 : 32;
    } reg;
} SOC_ICS_SMMU_MSTR_END_REQ_1_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_END_REQ_1_end_req_1_START (0)
#define SOC_ICS_SMMU_MSTR_END_REQ_1_end_req_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int end_req_2 : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_ICS_SMMU_MSTR_END_REQ_2_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_END_REQ_2_end_req_2_START (0)
#define SOC_ICS_SMMU_MSTR_END_REQ_2_end_req_2_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int end_ack_0 : 32;
    } reg;
} SOC_ICS_SMMU_MSTR_END_ACK_0_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_END_ACK_0_end_ack_0_START (0)
#define SOC_ICS_SMMU_MSTR_END_ACK_0_end_ack_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int end_ack_1 : 32;
    } reg;
} SOC_ICS_SMMU_MSTR_END_ACK_1_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_END_ACK_1_end_ack_1_START (0)
#define SOC_ICS_SMMU_MSTR_END_ACK_1_end_ack_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int end_ack_2 : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_ICS_SMMU_MSTR_END_ACK_2_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_END_ACK_2_end_ack_2_START (0)
#define SOC_ICS_SMMU_MSTR_END_ACK_2_end_ack_2_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stream_start_0 : 32;
    } reg;
} SOC_ICS_SMMU_MSTR_SMRX_START_0_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_SMRX_START_0_stream_start_0_START (0)
#define SOC_ICS_SMMU_MSTR_SMRX_START_0_stream_start_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stream_start_1 : 32;
    } reg;
} SOC_ICS_SMMU_MSTR_SMRX_START_1_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_SMRX_START_1_stream_start_1_START (0)
#define SOC_ICS_SMMU_MSTR_SMRX_START_1_stream_start_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stream_start_2 : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_ICS_SMMU_MSTR_SMRX_START_2_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_SMRX_START_2_stream_start_2_START (0)
#define SOC_ICS_SMMU_MSTR_SMRX_START_2_stream_start_2_END (23)
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
} SOC_ICS_SMMU_MSTR_INPT_SEL_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_INPT_SEL_smr_start_sel_START (0)
#define SOC_ICS_SMMU_MSTR_INPT_SEL_smr_start_sel_END (0)
#define SOC_ICS_SMMU_MSTR_INPT_SEL_end_req_sel_START (1)
#define SOC_ICS_SMMU_MSTR_INPT_SEL_end_req_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_ICS_SMMU_MSTR_RESERVED_0_UNION;
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
} SOC_ICS_SMMU_MSTR_INTMASK_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_INTMASK_rd_va_err0_msk_START (0)
#define SOC_ICS_SMMU_MSTR_INTMASK_rd_va_err0_msk_END (0)
#define SOC_ICS_SMMU_MSTR_INTMASK_rd_va_err1_msk_START (1)
#define SOC_ICS_SMMU_MSTR_INTMASK_rd_va_err1_msk_END (1)
#define SOC_ICS_SMMU_MSTR_INTMASK_wr_va_err0_msk_START (2)
#define SOC_ICS_SMMU_MSTR_INTMASK_wr_va_err0_msk_END (2)
#define SOC_ICS_SMMU_MSTR_INTMASK_wr_va_err1_msk_START (3)
#define SOC_ICS_SMMU_MSTR_INTMASK_wr_va_err1_msk_END (3)
#define SOC_ICS_SMMU_MSTR_INTMASK_wdata_burst_msk_START (4)
#define SOC_ICS_SMMU_MSTR_INTMASK_wdata_burst_msk_END (4)
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
} SOC_ICS_SMMU_MSTR_INTRAW_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_INTRAW_rd_va_err0_raw_START (0)
#define SOC_ICS_SMMU_MSTR_INTRAW_rd_va_err0_raw_END (0)
#define SOC_ICS_SMMU_MSTR_INTRAW_rd_va_err1_raw_START (1)
#define SOC_ICS_SMMU_MSTR_INTRAW_rd_va_err1_raw_END (1)
#define SOC_ICS_SMMU_MSTR_INTRAW_wr_va_err0_raw_START (2)
#define SOC_ICS_SMMU_MSTR_INTRAW_wr_va_err0_raw_END (2)
#define SOC_ICS_SMMU_MSTR_INTRAW_wr_va_err1_raw_START (3)
#define SOC_ICS_SMMU_MSTR_INTRAW_wr_va_err1_raw_END (3)
#define SOC_ICS_SMMU_MSTR_INTRAW_wdata_burst_raw_START (4)
#define SOC_ICS_SMMU_MSTR_INTRAW_wdata_burst_raw_END (4)
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
} SOC_ICS_SMMU_MSTR_INTSTAT_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_INTSTAT_rd_va_err0_stat_START (0)
#define SOC_ICS_SMMU_MSTR_INTSTAT_rd_va_err0_stat_END (0)
#define SOC_ICS_SMMU_MSTR_INTSTAT_rd_va_err1_stat_START (1)
#define SOC_ICS_SMMU_MSTR_INTSTAT_rd_va_err1_stat_END (1)
#define SOC_ICS_SMMU_MSTR_INTSTAT_wr_va_err0_stat_START (2)
#define SOC_ICS_SMMU_MSTR_INTSTAT_wr_va_err0_stat_END (2)
#define SOC_ICS_SMMU_MSTR_INTSTAT_wr_va_err1_stat_START (3)
#define SOC_ICS_SMMU_MSTR_INTSTAT_wr_va_err1_stat_END (3)
#define SOC_ICS_SMMU_MSTR_INTSTAT_wdata_burst_stat_START (4)
#define SOC_ICS_SMMU_MSTR_INTSTAT_wdata_burst_stat_END (4)
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
} SOC_ICS_SMMU_MSTR_INTCLR_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_INTCLR_rd_va_err0_clr_START (0)
#define SOC_ICS_SMMU_MSTR_INTCLR_rd_va_err0_clr_END (0)
#define SOC_ICS_SMMU_MSTR_INTCLR_rd_va_err1_clr_START (1)
#define SOC_ICS_SMMU_MSTR_INTCLR_rd_va_err1_clr_END (1)
#define SOC_ICS_SMMU_MSTR_INTCLR_wr_va_err0_clr_START (2)
#define SOC_ICS_SMMU_MSTR_INTCLR_wr_va_err0_clr_END (2)
#define SOC_ICS_SMMU_MSTR_INTCLR_wr_va_err1_clr_START (3)
#define SOC_ICS_SMMU_MSTR_INTCLR_wr_va_err1_clr_END (3)
#define SOC_ICS_SMMU_MSTR_INTCLR_wdata_burst_clr_START (4)
#define SOC_ICS_SMMU_MSTR_INTCLR_wdata_burst_clr_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_sid : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_ICS_SMMU_MSTR_DBG_0_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_DBG_0_rd_sid_START (0)
#define SOC_ICS_SMMU_MSTR_DBG_0_rd_sid_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_addr : 32;
    } reg;
} SOC_ICS_SMMU_MSTR_DBG_1_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_DBG_1_rd_addr_START (0)
#define SOC_ICS_SMMU_MSTR_DBG_1_rd_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_sid : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_ICS_SMMU_MSTR_DBG_2_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_DBG_2_wr_sid_START (0)
#define SOC_ICS_SMMU_MSTR_DBG_2_wr_sid_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_addr : 32;
    } reg;
} SOC_ICS_SMMU_MSTR_DBG_3_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_DBG_3_wr_addr_START (0)
#define SOC_ICS_SMMU_MSTR_DBG_3_wr_addr_END (31)
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
} SOC_ICS_SMMU_MSTR_DBG_4_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_DBG_4_awid_START (0)
#define SOC_ICS_SMMU_MSTR_DBG_4_awid_END (7)
#define SOC_ICS_SMMU_MSTR_DBG_4_awlen_START (8)
#define SOC_ICS_SMMU_MSTR_DBG_4_awlen_END (15)
#define SOC_ICS_SMMU_MSTR_DBG_4_wid_START (16)
#define SOC_ICS_SMMU_MSTR_DBG_4_wid_END (23)
#define SOC_ICS_SMMU_MSTR_DBG_4_wdata_len_START (24)
#define SOC_ICS_SMMU_MSTR_DBG_4_wdata_len_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int awaddr : 32;
    } reg;
} SOC_ICS_SMMU_MSTR_DBG_5_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_DBG_5_awaddr_START (0)
#define SOC_ICS_SMMU_MSTR_DBG_5_awaddr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_ICS_SMMU_MSTR_RESERVED_1_UNION;
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
} SOC_ICS_SMMU_MSTR_DBG_PORT_IN_0_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_DBG_PORT_IN_0_dbg_cfg_addr_START (0)
#define SOC_ICS_SMMU_MSTR_DBG_PORT_IN_0_dbg_cfg_addr_END (15)
#define SOC_ICS_SMMU_MSTR_DBG_PORT_IN_0_dbg_cfg_cs_START (16)
#define SOC_ICS_SMMU_MSTR_DBG_PORT_IN_0_dbg_cfg_cs_END (16)
#define SOC_ICS_SMMU_MSTR_DBG_PORT_IN_0_dbg_cfg_wr_START (17)
#define SOC_ICS_SMMU_MSTR_DBG_PORT_IN_0_dbg_cfg_wr_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_cfg_wdata : 32;
    } reg;
} SOC_ICS_SMMU_MSTR_DBG_PORT_IN_1_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_DBG_PORT_IN_1_dbg_cfg_wdata_START (0)
#define SOC_ICS_SMMU_MSTR_DBG_PORT_IN_1_dbg_cfg_wdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_cfg_rdata : 32;
    } reg;
} SOC_ICS_SMMU_MSTR_DBG_PORT_OUT_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_DBG_PORT_OUT_dbg_cfg_rdata_START (0)
#define SOC_ICS_SMMU_MSTR_DBG_PORT_OUT_dbg_cfg_rdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_ICS_SMMU_MSTR_RESERVED_2_UNION;
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
} SOC_ICS_SMMU_MSTR_SMRX_0_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_SMRX_0_bypass_START (0)
#define SOC_ICS_SMMU_MSTR_SMRX_0_bypass_END (0)
#define SOC_ICS_SMMU_MSTR_SMRX_0_upwin_START (4)
#define SOC_ICS_SMMU_MSTR_SMRX_0_upwin_END (11)
#define SOC_ICS_SMMU_MSTR_SMRX_0_len_START (12)
#define SOC_ICS_SMMU_MSTR_SMRX_0_len_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 15;
        unsigned int va_str : 17;
    } reg;
} SOC_ICS_SMMU_MSTR_SMRX_1_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_SMRX_1_va_str_START (15)
#define SOC_ICS_SMMU_MSTR_SMRX_1_va_str_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 15;
        unsigned int va_end : 17;
    } reg;
} SOC_ICS_SMMU_MSTR_SMRX_2_UNION;
#endif
#define SOC_ICS_SMMU_MSTR_SMRX_2_va_end_START (15)
#define SOC_ICS_SMMU_MSTR_SMRX_2_va_end_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_cmd_total_cnt : 32;
    } reg;
} SOC_ICS_RD_CMD_TOTAL_CNT_UNION;
#endif
#define SOC_ICS_RD_CMD_TOTAL_CNT_rd_cmd_total_cnt_START (0)
#define SOC_ICS_RD_CMD_TOTAL_CNT_rd_cmd_total_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_cmd_miss_cnt : 32;
    } reg;
} SOC_ICS_RD_CMD_MISS_CNT_UNION;
#endif
#define SOC_ICS_RD_CMD_MISS_CNT_rd_cmd_miss_cnt_START (0)
#define SOC_ICS_RD_CMD_MISS_CNT_rd_cmd_miss_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_data_total_cnt : 32;
    } reg;
} SOC_ICS_RD_DATA_TOTAL_CNT_UNION;
#endif
#define SOC_ICS_RD_DATA_TOTAL_CNT_rd_data_total_cnt_START (0)
#define SOC_ICS_RD_DATA_TOTAL_CNT_rd_data_total_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_cmd_case_cnt : 32;
    } reg;
} SOC_ICS_RD_CMD_CASE_CNT_UNION;
#endif
#define SOC_ICS_RD_CMD_CASE_CNT_rd_cmd_case_cnt_START (0)
#define SOC_ICS_RD_CMD_CASE_CNT_rd_cmd_case_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_cmd_max_latency : 16;
        unsigned int rd_cmd_avg_latency : 16;
    } reg;
} SOC_ICS_RD_CMD_TRANS_LATENCY_UNION;
#endif
#define SOC_ICS_RD_CMD_TRANS_LATENCY_rd_cmd_max_latency_START (0)
#define SOC_ICS_RD_CMD_TRANS_LATENCY_rd_cmd_max_latency_END (15)
#define SOC_ICS_RD_CMD_TRANS_LATENCY_rd_cmd_avg_latency_START (16)
#define SOC_ICS_RD_CMD_TRANS_LATENCY_rd_cmd_avg_latency_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_cmd_total_cnt : 32;
    } reg;
} SOC_ICS_WR_CMD_TOTAL_CNT_UNION;
#endif
#define SOC_ICS_WR_CMD_TOTAL_CNT_wr_cmd_total_cnt_START (0)
#define SOC_ICS_WR_CMD_TOTAL_CNT_wr_cmd_total_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_cmd_miss_cnt : 32;
    } reg;
} SOC_ICS_WR_CMD_MISS_CNT_UNION;
#endif
#define SOC_ICS_WR_CMD_MISS_CNT_wr_cmd_miss_cnt_START (0)
#define SOC_ICS_WR_CMD_MISS_CNT_wr_cmd_miss_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_data_total_cnt : 32;
    } reg;
} SOC_ICS_WR_DATA_TOTAL_CNT_UNION;
#endif
#define SOC_ICS_WR_DATA_TOTAL_CNT_wr_data_total_cnt_START (0)
#define SOC_ICS_WR_DATA_TOTAL_CNT_wr_data_total_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_cmd_case_cnt : 32;
    } reg;
} SOC_ICS_WR_CMD_CASE_CNT_UNION;
#endif
#define SOC_ICS_WR_CMD_CASE_CNT_wr_cmd_case_cnt_START (0)
#define SOC_ICS_WR_CMD_CASE_CNT_wr_cmd_case_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_cmd_max_latency : 16;
        unsigned int wr_cmd_avg_latency : 16;
    } reg;
} SOC_ICS_WR_CMD_TRANS_LATENCY_UNION;
#endif
#define SOC_ICS_WR_CMD_TRANS_LATENCY_wr_cmd_max_latency_START (0)
#define SOC_ICS_WR_CMD_TRANS_LATENCY_wr_cmd_max_latency_END (15)
#define SOC_ICS_WR_CMD_TRANS_LATENCY_wr_cmd_avg_latency_START (16)
#define SOC_ICS_WR_CMD_TRANS_LATENCY_wr_cmd_avg_latency_END (31)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
