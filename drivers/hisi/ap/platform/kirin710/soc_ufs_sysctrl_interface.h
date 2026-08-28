#ifndef __SOC_UFS_SYSCTRL_INTERFACE_H__
#define __SOC_UFS_SYSCTRL_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_UFS_Sysctrl_MEMORY_CTRL_ADDR(base) ((base) + (0x000))
#define SOC_UFS_Sysctrl_PSW_POWER_CTRL_ADDR(base) ((base) + (0x004))
#define SOC_UFS_Sysctrl_PHY_ISO_EN_ADDR(base) ((base) + (0x0008))
#define SOC_UFS_Sysctrl_HC_LP_CTRL_ADDR(base) ((base) + (0x000C))
#define SOC_UFS_Sysctrl_PHY_CLK_CTRL_ADDR(base) ((base) + (0x0010))
#define SOC_UFS_Sysctrl_PSW_CLK_CTRL_ADDR(base) ((base) + (0x0014))
#define SOC_UFS_Sysctrl_CLOCK_GATE_BYPASS_ADDR(base) ((base) + (0x0018))
#define SOC_UFS_Sysctrl_RESET_CTRL_EN_ADDR(base) ((base) + (0x001C))
#define SOC_UFS_Sysctrl_PHY_RESET_STATUS_ADDR(base) ((base) + (0x0028))
#define SOC_UFS_Sysctrl_HC_DEBUG_ADDR(base) ((base) + (0x002C))
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_CTRL_ADDR(base) ((base) + (0x0030))
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_OBSV_ADDR(base) ((base) + (0x0034))
#define SOC_UFS_Sysctrl_PHY_DTB_OUT_ADDR(base) ((base) + (0x0038))
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_HH_ADDR(base) ((base) + (0x003C))
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_H_ADDR(base) ((base) + (0x0040))
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_L_ADDR(base) ((base) + (0x0044))
#define SOC_UFS_Sysctrl_DEBUG_MONITORUP_H_ADDR(base) ((base) + (0x0048))
#define SOC_UFS_Sysctrl_DEBUG_MONITORUP_L_ADDR(base) ((base) + (0x004C))
#define SOC_UFS_Sysctrl_MK2_CTRL_ADDR(base) ((base) + (0x0050))
#define SOC_UFS_Sysctrl_UFS_SYSCTRL_ADDR(base) ((base) + (0x005C))
#define SOC_UFS_Sysctrl_UFS_RESET_CTRL_ADDR(base) ((base) + (0x060))
#define SOC_UFS_Sysctrl_UFS_UMECTRL_ADDR(base) ((base) + (0x0064))
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_UME_HH_ADDR(base) ((base) + (0x0068))
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_UME_H_ADDR(base) ((base) + (0x006C))
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_UME_L_ADDR(base) ((base) + (0x0070))
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_ADDR(base) ((base) + (0x0074))
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(base) ((base) + (0x07C))
#define SOC_UFS_Sysctrl_CRG_UFS_CFG1_ADDR(base) ((base) + (0x080))
#define SOC_UFS_Sysctrl_UFSAXI_W_QOS_LMTR_ADDR(base) ((base) + (0x0084))
#define SOC_UFS_Sysctrl_UFSAXI_R_QOS_LMTR_ADDR(base) ((base) + (0x0088))
#define SOC_UFS_Sysctrl_CRG_UFS_STAT_ADDR(base) ((base) + (0x0A8))
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_ADDR(base) ((base) + (0x0AC))
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_ADDR(base) ((base) + (0x0B0))
#define SOC_UFS_Sysctrl_AUTO_H8_CONUTER_ADDR(base) ((base) + (0x0B4))
#define SOC_UFS_Sysctrl_PWM_CONUTER_ADDR(base) ((base) + (0x0B8))
#define SOC_UFS_Sysctrl_UFS_IDLE_CONUTER_ADDR(base) ((base) + (0x0BC))
#define SOC_UFS_Sysctrl_AUTO_H8_CONUTER_CRUT_ADDR(base) ((base) + (0x0C0))
#define SOC_UFS_Sysctrl_PWM_CONUTER_CRUT_ADDR(base) ((base) + (0x0C4))
#define SOC_UFS_Sysctrl_UFS_IDLE_CONUTER_CRUT_ADDR(base) ((base) + (0x0C8))
#define SOC_UFS_Sysctrl_AUTO_H8_CONUTER_CLR_ADDR(base) ((base) + (0x0CC))
#define SOC_UFS_Sysctrl_PWM_CONUTER_CLR_ADDR(base) ((base) + (0x0D0))
#define SOC_UFS_Sysctrl_UFS_IDLE_CONUTER_CLR_ADDR(base) ((base) + (0x0D4))
#define SOC_UFS_Sysctrl_PHY_FSM_STATE_ADDR(base) ((base) + (0x0D8))
#define SOC_UFS_Sysctrl_DEVICE_CONUTER_ADDR(base) ((base) + (0x0DC))
#define SOC_UFS_Sysctrl_DEVICE_CONUTER_CRUT_ADDR(base) ((base) + (0x0E0))
#define SOC_UFS_Sysctrl_DEVICE_CONUTER_CLR_ADDR(base) ((base) + (0x0E4))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ufs_mem_ctrl_d1w2r : 16;
        unsigned int ufs_mem_ctrl_uhd_d1w2r : 16;
    } reg;
} SOC_UFS_Sysctrl_MEMORY_CTRL_UNION;
#endif
#define SOC_UFS_Sysctrl_MEMORY_CTRL_ufs_mem_ctrl_d1w2r_START (0)
#define SOC_UFS_Sysctrl_MEMORY_CTRL_ufs_mem_ctrl_d1w2r_END (15)
#define SOC_UFS_Sysctrl_MEMORY_CTRL_ufs_mem_ctrl_uhd_d1w2r_START (16)
#define SOC_UFS_Sysctrl_MEMORY_CTRL_ufs_mem_ctrl_uhd_d1w2r_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ufs_psw_mtcmos_en : 1;
        unsigned int ufs_psw_mtcmos_ack : 1;
        unsigned int ufs_phy_mtcmos_en : 1;
        unsigned int ufs_phy_mtcmos_ack : 1;
        unsigned int reserved_0 : 12;
        unsigned int ufs_psw_iso_ctrl : 1;
        unsigned int reserved_1 : 15;
    } reg;
} SOC_UFS_Sysctrl_PSW_POWER_CTRL_UNION;
#endif
#define SOC_UFS_Sysctrl_PSW_POWER_CTRL_ufs_psw_mtcmos_en_START (0)
#define SOC_UFS_Sysctrl_PSW_POWER_CTRL_ufs_psw_mtcmos_en_END (0)
#define SOC_UFS_Sysctrl_PSW_POWER_CTRL_ufs_psw_mtcmos_ack_START (1)
#define SOC_UFS_Sysctrl_PSW_POWER_CTRL_ufs_psw_mtcmos_ack_END (1)
#define SOC_UFS_Sysctrl_PSW_POWER_CTRL_ufs_phy_mtcmos_en_START (2)
#define SOC_UFS_Sysctrl_PSW_POWER_CTRL_ufs_phy_mtcmos_en_END (2)
#define SOC_UFS_Sysctrl_PSW_POWER_CTRL_ufs_phy_mtcmos_ack_START (3)
#define SOC_UFS_Sysctrl_PSW_POWER_CTRL_ufs_phy_mtcmos_ack_END (3)
#define SOC_UFS_Sysctrl_PSW_POWER_CTRL_ufs_psw_iso_ctrl_START (16)
#define SOC_UFS_Sysctrl_PSW_POWER_CTRL_ufs_psw_iso_ctrl_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ufs_phy_iso_ctrl : 1;
        unsigned int reserved_0 : 15;
        unsigned int ufs_refclk_iso_en : 1;
        unsigned int reserved_1 : 15;
    } reg;
} SOC_UFS_Sysctrl_PHY_ISO_EN_UNION;
#endif
#define SOC_UFS_Sysctrl_PHY_ISO_EN_ufs_phy_iso_ctrl_START (0)
#define SOC_UFS_Sysctrl_PHY_ISO_EN_ufs_phy_iso_ctrl_END (0)
#define SOC_UFS_Sysctrl_PHY_ISO_EN_ufs_refclk_iso_en_START (16)
#define SOC_UFS_Sysctrl_PHY_ISO_EN_ufs_refclk_iso_en_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_lp_pwr_gate : 1;
        unsigned int reserved_0 : 7;
        unsigned int sysctrl_pwr_ready : 1;
        unsigned int reserved_1 : 7;
        unsigned int sysctrl_lp_isol_en : 1;
        unsigned int reserved_2 : 15;
    } reg;
} SOC_UFS_Sysctrl_HC_LP_CTRL_UNION;
#endif
#define SOC_UFS_Sysctrl_HC_LP_CTRL_sysctrl_lp_pwr_gate_START (0)
#define SOC_UFS_Sysctrl_HC_LP_CTRL_sysctrl_lp_pwr_gate_END (0)
#define SOC_UFS_Sysctrl_HC_LP_CTRL_sysctrl_pwr_ready_START (8)
#define SOC_UFS_Sysctrl_HC_LP_CTRL_sysctrl_pwr_ready_END (8)
#define SOC_UFS_Sysctrl_HC_LP_CTRL_sysctrl_lp_isol_en_START (16)
#define SOC_UFS_Sysctrl_HC_LP_CTRL_sysctrl_lp_isol_en_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_cfg_clock_freq : 8;
        unsigned int sysctrl_ref_clock_sel : 2;
        unsigned int reserved_0 : 6;
        unsigned int sysctrl_ref_use_pad : 1;
        unsigned int reserved_1 : 7;
        unsigned int sysctrl_ref_clock_en : 1;
        unsigned int reserved_2 : 7;
    } reg;
} SOC_UFS_Sysctrl_PHY_CLK_CTRL_UNION;
#endif
#define SOC_UFS_Sysctrl_PHY_CLK_CTRL_sysctrl_cfg_clock_freq_START (0)
#define SOC_UFS_Sysctrl_PHY_CLK_CTRL_sysctrl_cfg_clock_freq_END (7)
#define SOC_UFS_Sysctrl_PHY_CLK_CTRL_sysctrl_ref_clock_sel_START (8)
#define SOC_UFS_Sysctrl_PHY_CLK_CTRL_sysctrl_ref_clock_sel_END (9)
#define SOC_UFS_Sysctrl_PHY_CLK_CTRL_sysctrl_ref_use_pad_START (16)
#define SOC_UFS_Sysctrl_PHY_CLK_CTRL_sysctrl_ref_use_pad_END (16)
#define SOC_UFS_Sysctrl_PHY_CLK_CTRL_sysctrl_ref_clock_en_START (24)
#define SOC_UFS_Sysctrl_PHY_CLK_CTRL_sysctrl_ref_clock_en_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_cfg_clock_en : 1;
        unsigned int reserved_0 : 3;
        unsigned int sysctrl_psw_clk_en : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_UFS_Sysctrl_PSW_CLK_CTRL_UNION;
#endif
#define SOC_UFS_Sysctrl_PSW_CLK_CTRL_sysctrl_cfg_clock_en_START (0)
#define SOC_UFS_Sysctrl_PSW_CLK_CTRL_sysctrl_cfg_clock_en_END (0)
#define SOC_UFS_Sysctrl_PSW_CLK_CTRL_sysctrl_psw_clk_en_START (4)
#define SOC_UFS_Sysctrl_PSW_CLK_CTRL_sysctrl_psw_clk_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_cgsysclk_bypass : 1;
        unsigned int sysctrl_hclk_bypass : 1;
        unsigned int sysctrl_lp_clkt_bypass : 1;
        unsigned int sysctrl_clkt_bypass : 1;
        unsigned int sysctrl_pclk_bypass : 1;
        unsigned int sysctrl_txsymbol_bypass : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_UFS_Sysctrl_CLOCK_GATE_BYPASS_UNION;
#endif
#define SOC_UFS_Sysctrl_CLOCK_GATE_BYPASS_sysctrl_cgsysclk_bypass_START (0)
#define SOC_UFS_Sysctrl_CLOCK_GATE_BYPASS_sysctrl_cgsysclk_bypass_END (0)
#define SOC_UFS_Sysctrl_CLOCK_GATE_BYPASS_sysctrl_hclk_bypass_START (1)
#define SOC_UFS_Sysctrl_CLOCK_GATE_BYPASS_sysctrl_hclk_bypass_END (1)
#define SOC_UFS_Sysctrl_CLOCK_GATE_BYPASS_sysctrl_lp_clkt_bypass_START (2)
#define SOC_UFS_Sysctrl_CLOCK_GATE_BYPASS_sysctrl_lp_clkt_bypass_END (2)
#define SOC_UFS_Sysctrl_CLOCK_GATE_BYPASS_sysctrl_clkt_bypass_START (3)
#define SOC_UFS_Sysctrl_CLOCK_GATE_BYPASS_sysctrl_clkt_bypass_END (3)
#define SOC_UFS_Sysctrl_CLOCK_GATE_BYPASS_sysctrl_pclk_bypass_START (4)
#define SOC_UFS_Sysctrl_CLOCK_GATE_BYPASS_sysctrl_pclk_bypass_END (4)
#define SOC_UFS_Sysctrl_CLOCK_GATE_BYPASS_sysctrl_txsymbol_bypass_START (5)
#define SOC_UFS_Sysctrl_CLOCK_GATE_BYPASS_sysctrl_txsymbol_bypass_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_lp_reset_n : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_UFS_Sysctrl_RESET_CTRL_EN_UNION;
#endif
#define SOC_UFS_Sysctrl_RESET_CTRL_EN_sysctrl_lp_reset_n_START (0)
#define SOC_UFS_Sysctrl_RESET_CTRL_EN_sysctrl_lp_reset_n_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_lp_resetcomplete : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_UFS_Sysctrl_PHY_RESET_STATUS_UNION;
#endif
#define SOC_UFS_Sysctrl_PHY_RESET_STATUS_sysctrl_lp_resetcomplete_START (0)
#define SOC_UFS_Sysctrl_PHY_RESET_STATUS_sysctrl_lp_resetcomplete_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_dbg_ufshc_fe_interrupt : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_UFS_Sysctrl_HC_DEBUG_UNION;
#endif
#define SOC_UFS_Sysctrl_HC_DEBUG_sysctrl_dbg_ufshc_fe_interrupt_START (0)
#define SOC_UFS_Sysctrl_HC_DEBUG_sysctrl_dbg_ufshc_fe_interrupt_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_dbg_rx_tst_rtcontrol_0 : 8;
        unsigned int sysctrl_dbg_rx_tst_rtcontrol_1 : 8;
        unsigned int sysctrl_dbg_tx_tst_rtcontrol_0 : 8;
        unsigned int sysctrl_dbg_tx_tst_rtcontrol_1 : 8;
    } reg;
} SOC_UFS_Sysctrl_PHY_MPX_TEST_CTRL_UNION;
#endif
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_CTRL_sysctrl_dbg_rx_tst_rtcontrol_0_START (0)
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_CTRL_sysctrl_dbg_rx_tst_rtcontrol_0_END (7)
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_CTRL_sysctrl_dbg_rx_tst_rtcontrol_1_START (8)
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_CTRL_sysctrl_dbg_rx_tst_rtcontrol_1_END (15)
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_CTRL_sysctrl_dbg_tx_tst_rtcontrol_0_START (16)
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_CTRL_sysctrl_dbg_tx_tst_rtcontrol_0_END (23)
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_CTRL_sysctrl_dbg_tx_tst_rtcontrol_1_START (24)
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_CTRL_sysctrl_dbg_tx_tst_rtcontrol_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_dbg_rx_tst_rtobserve_0 : 8;
        unsigned int sysctrl_dbg_rx_tst_rtobserve_1 : 8;
        unsigned int sysctrl_dbg_tx_tst_rtobserve_0 : 8;
        unsigned int sysctrl_dbg_tx_tst_rtobserve_1 : 8;
    } reg;
} SOC_UFS_Sysctrl_PHY_MPX_TEST_OBSV_UNION;
#endif
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_OBSV_sysctrl_dbg_rx_tst_rtobserve_0_START (0)
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_OBSV_sysctrl_dbg_rx_tst_rtobserve_0_END (7)
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_OBSV_sysctrl_dbg_rx_tst_rtobserve_1_START (8)
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_OBSV_sysctrl_dbg_rx_tst_rtobserve_1_END (15)
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_OBSV_sysctrl_dbg_tx_tst_rtobserve_0_START (16)
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_OBSV_sysctrl_dbg_tx_tst_rtobserve_0_END (23)
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_OBSV_sysctrl_dbg_tx_tst_rtobserve_1_START (24)
#define SOC_UFS_Sysctrl_PHY_MPX_TEST_OBSV_sysctrl_dbg_tx_tst_rtobserve_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_dbg_phy_dtb_out : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_UFS_Sysctrl_PHY_DTB_OUT_UNION;
#endif
#define SOC_UFS_Sysctrl_PHY_DTB_OUT_sysctrl_dbg_phy_dtb_out_START (0)
#define SOC_UFS_Sysctrl_PHY_DTB_OUT_sysctrl_dbg_phy_dtb_out_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_dbg_monitor_hh : 10;
        unsigned int reserved : 22;
    } reg;
} SOC_UFS_Sysctrl_DEBUG_MONITOR_HH_UNION;
#endif
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_HH_sysctrl_dbg_monitor_hh_START (0)
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_HH_sysctrl_dbg_monitor_hh_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_dbg_monitor_h : 32;
    } reg;
} SOC_UFS_Sysctrl_DEBUG_MONITOR_H_UNION;
#endif
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_H_sysctrl_dbg_monitor_h_START (0)
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_H_sysctrl_dbg_monitor_h_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_dbg_monitor_l : 32;
    } reg;
} SOC_UFS_Sysctrl_DEBUG_MONITOR_L_UNION;
#endif
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_L_sysctrl_dbg_monitor_l_START (0)
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_L_sysctrl_dbg_monitor_l_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_dbg_monitorup_h : 32;
    } reg;
} SOC_UFS_Sysctrl_DEBUG_MONITORUP_H_UNION;
#endif
#define SOC_UFS_Sysctrl_DEBUG_MONITORUP_H_sysctrl_dbg_monitorup_h_START (0)
#define SOC_UFS_Sysctrl_DEBUG_MONITORUP_H_sysctrl_dbg_monitorup_h_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_dbg_monitorup_l : 32;
    } reg;
} SOC_UFS_Sysctrl_DEBUG_MONITORUP_L_UNION;
#endif
#define SOC_UFS_Sysctrl_DEBUG_MONITORUP_L_sysctrl_dbg_monitorup_l_START (0)
#define SOC_UFS_Sysctrl_DEBUG_MONITORUP_L_sysctrl_dbg_monitorup_l_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_txX_mk2extensionen : 1;
        unsigned int sysctrl_rx_mk2deepexit : 1;
        unsigned int reserved_0 : 6;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_UFS_Sysctrl_MK2_CTRL_UNION;
#endif
#define SOC_UFS_Sysctrl_MK2_CTRL_sysctrl_txX_mk2extensionen_START (0)
#define SOC_UFS_Sysctrl_MK2_CTRL_sysctrl_txX_mk2extensionen_END (0)
#define SOC_UFS_Sysctrl_MK2_CTRL_sysctrl_rx_mk2deepexit_START (1)
#define SOC_UFS_Sysctrl_MK2_CTRL_sysctrl_rx_mk2deepexit_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 1;
        unsigned int ufs_sysctrl_bakup : 31;
    } reg;
} SOC_UFS_Sysctrl_UFS_SYSCTRL_UNION;
#endif
#define SOC_UFS_Sysctrl_UFS_SYSCTRL_ufs_sysctrl_bakup_START (1)
#define SOC_UFS_Sysctrl_UFS_SYSCTRL_ufs_sysctrl_bakup_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ufs_device_rst_n : 1;
        unsigned int ufs_mask_ctrl_bak : 15;
        unsigned int clkdivmasken : 16;
    } reg;
} SOC_UFS_Sysctrl_UFS_RESET_CTRL_UNION;
#endif
#define SOC_UFS_Sysctrl_UFS_RESET_CTRL_ufs_device_rst_n_START (0)
#define SOC_UFS_Sysctrl_UFS_RESET_CTRL_ufs_device_rst_n_END (0)
#define SOC_UFS_Sysctrl_UFS_RESET_CTRL_ufs_mask_ctrl_bak_START (1)
#define SOC_UFS_Sysctrl_UFS_RESET_CTRL_ufs_mask_ctrl_bak_END (15)
#define SOC_UFS_Sysctrl_UFS_RESET_CTRL_clkdivmasken_START (16)
#define SOC_UFS_Sysctrl_UFS_RESET_CTRL_clkdivmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ufs_ies_en_mask : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_UFS_Sysctrl_UFS_UMECTRL_UNION;
#endif
#define SOC_UFS_Sysctrl_UFS_UMECTRL_ufs_ies_en_mask_START (0)
#define SOC_UFS_Sysctrl_UFS_UMECTRL_ufs_ies_en_mask_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_dbg_monitor_ume_hh : 7;
        unsigned int reserved : 25;
    } reg;
} SOC_UFS_Sysctrl_DEBUG_MONITOR_UME_HH_UNION;
#endif
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_UME_HH_sysctrl_dbg_monitor_ume_hh_START (0)
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_UME_HH_sysctrl_dbg_monitor_ume_hh_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_dbg_monitor_ume_h : 32;
    } reg;
} SOC_UFS_Sysctrl_DEBUG_MONITOR_UME_H_UNION;
#endif
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_UME_H_sysctrl_dbg_monitor_ume_h_START (0)
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_UME_H_sysctrl_dbg_monitor_ume_h_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_dbg_monitor_ume_l : 32;
    } reg;
} SOC_UFS_Sysctrl_DEBUG_MONITOR_UME_L_UNION;
#endif
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_UME_L_sysctrl_dbg_monitor_ume_l_START (0)
#define SOC_UFS_Sysctrl_DEBUG_MONITOR_UME_L_sysctrl_dbg_monitor_ume_l_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysctrl_cmu_clk_wr_en : 1;
        unsigned int sysctrl_cmu_clk_rd_en : 1;
        unsigned int sysctrl_rdp_clk_wr_en : 1;
        unsigned int sysctrl_rdp_clk_rd_en : 1;
        unsigned int sysctrl_wdp_clk_wr_en : 1;
        unsigned int sysctrl_wdp_clk_rd_en : 1;
        unsigned int sysctrl_rx_tc0_clk_wr_en : 1;
        unsigned int sysctrl_rx_tc0_clk_rd_en : 1;
        unsigned int sysctrl_tx_l2_clk_wr_en : 1;
        unsigned int sysctrl_tx_l2_clk_rd_en : 1;
        unsigned int sysctrl_rx_tc1_clk_wr_en : 1;
        unsigned int sysctrl_rx_tc1_clk_rd_en : 1;
        unsigned int sysctrl_umcmu_clk_wr_en : 1;
        unsigned int sysctrl_umcmu_clk_rd_en : 1;
        unsigned int sysctrl_umrdp_clk_wr_en : 1;
        unsigned int sysctrl_umrdp_clk_rd_en : 1;
        unsigned int sysctrl_umwdp_clk_wr_en : 1;
        unsigned int sysctrl_umwdp_clk_rd_en : 1;
        unsigned int ufs_mem_clk_gate_bypass : 14;
    } reg;
} SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_UNION;
#endif
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_cmu_clk_wr_en_START (0)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_cmu_clk_wr_en_END (0)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_cmu_clk_rd_en_START (1)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_cmu_clk_rd_en_END (1)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_rdp_clk_wr_en_START (2)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_rdp_clk_wr_en_END (2)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_rdp_clk_rd_en_START (3)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_rdp_clk_rd_en_END (3)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_wdp_clk_wr_en_START (4)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_wdp_clk_wr_en_END (4)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_wdp_clk_rd_en_START (5)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_wdp_clk_rd_en_END (5)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_rx_tc0_clk_wr_en_START (6)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_rx_tc0_clk_wr_en_END (6)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_rx_tc0_clk_rd_en_START (7)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_rx_tc0_clk_rd_en_END (7)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_tx_l2_clk_wr_en_START (8)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_tx_l2_clk_wr_en_END (8)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_tx_l2_clk_rd_en_START (9)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_tx_l2_clk_rd_en_END (9)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_rx_tc1_clk_wr_en_START (10)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_rx_tc1_clk_wr_en_END (10)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_rx_tc1_clk_rd_en_START (11)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_rx_tc1_clk_rd_en_END (11)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_umcmu_clk_wr_en_START (12)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_umcmu_clk_wr_en_END (12)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_umcmu_clk_rd_en_START (13)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_umcmu_clk_rd_en_END (13)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_umrdp_clk_wr_en_START (14)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_umrdp_clk_wr_en_END (14)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_umrdp_clk_rd_en_START (15)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_umrdp_clk_rd_en_END (15)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_umwdp_clk_wr_en_START (16)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_umwdp_clk_wr_en_END (16)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_umwdp_clk_rd_en_START (17)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_sysctrl_umwdp_clk_rd_en_END (17)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_ufs_mem_clk_gate_bypass_START (18)
#define SOC_UFS_Sysctrl_UFS_MEM_CLK_GATE_BYPASS_ufs_mem_clk_gate_bypass_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_ufs : 1;
        unsigned int ip_arst_ufs : 1;
        unsigned int div_ufsphy_cfg : 3;
        unsigned int gt_clk_ufs : 1;
        unsigned int gt_clk_ufsphy_cfg : 1;
        unsigned int gt_clk_ufs_tick : 1;
        unsigned int div_ufs_tick : 6;
        unsigned int sc_gt_clk_ufs_tick_div : 1;
        unsigned int ufs_clkrst_bypass : 1;
        unsigned int clkdivmasken : 16;
    } reg;
} SOC_UFS_Sysctrl_CRG_UFS_CFG_UNION;
#endif
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START (0)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_END (0)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_arst_ufs_START (1)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_arst_ufs_END (1)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_div_ufsphy_cfg_START (2)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_div_ufsphy_cfg_END (4)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_gt_clk_ufs_START (5)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_gt_clk_ufs_END (5)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_gt_clk_ufsphy_cfg_START (6)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_gt_clk_ufsphy_cfg_END (6)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_gt_clk_ufs_tick_START (7)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_gt_clk_ufs_tick_END (7)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_div_ufs_tick_START (8)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_div_ufs_tick_END (13)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_sc_gt_clk_ufs_tick_div_START (14)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_sc_gt_clk_ufs_tick_div_END (14)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_ufs_clkrst_bypass_START (15)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_ufs_clkrst_bypass_END (15)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_clkdivmasken_START (16)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG_clkdivmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_gt_clk_ufsphy_cfg : 1;
        unsigned int reserved : 15;
        unsigned int clkdivmasken : 16;
    } reg;
} SOC_UFS_Sysctrl_CRG_UFS_CFG1_UNION;
#endif
#define SOC_UFS_Sysctrl_CRG_UFS_CFG1_sc_gt_clk_ufsphy_cfg_START (0)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG1_sc_gt_clk_ufsphy_cfg_END (0)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG1_clkdivmasken_START (16)
#define SOC_UFS_Sysctrl_CRG_UFS_CFG1_clkdivmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ufsaxi_w_lmtr_saturation : 14;
        unsigned int reserved_0 : 2;
        unsigned int ufsaxi_w_lmtr_bandwidth : 13;
        unsigned int reserved_1 : 2;
        unsigned int ufsaxi_w_lmtr_en : 1;
    } reg;
} SOC_UFS_Sysctrl_UFSAXI_W_QOS_LMTR_UNION;
#endif
#define SOC_UFS_Sysctrl_UFSAXI_W_QOS_LMTR_ufsaxi_w_lmtr_saturation_START (0)
#define SOC_UFS_Sysctrl_UFSAXI_W_QOS_LMTR_ufsaxi_w_lmtr_saturation_END (13)
#define SOC_UFS_Sysctrl_UFSAXI_W_QOS_LMTR_ufsaxi_w_lmtr_bandwidth_START (16)
#define SOC_UFS_Sysctrl_UFSAXI_W_QOS_LMTR_ufsaxi_w_lmtr_bandwidth_END (28)
#define SOC_UFS_Sysctrl_UFSAXI_W_QOS_LMTR_ufsaxi_w_lmtr_en_START (31)
#define SOC_UFS_Sysctrl_UFSAXI_W_QOS_LMTR_ufsaxi_w_lmtr_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ufsaxi_r_lmtr_saturation : 14;
        unsigned int reserved_0 : 2;
        unsigned int ufsaxi_r_lmtr_bandwidth : 13;
        unsigned int reserved_1 : 2;
        unsigned int ufsaxi_r_lmtr_en : 1;
    } reg;
} SOC_UFS_Sysctrl_UFSAXI_R_QOS_LMTR_UNION;
#endif
#define SOC_UFS_Sysctrl_UFSAXI_R_QOS_LMTR_ufsaxi_r_lmtr_saturation_START (0)
#define SOC_UFS_Sysctrl_UFSAXI_R_QOS_LMTR_ufsaxi_r_lmtr_saturation_END (13)
#define SOC_UFS_Sysctrl_UFSAXI_R_QOS_LMTR_ufsaxi_r_lmtr_bandwidth_START (16)
#define SOC_UFS_Sysctrl_UFSAXI_R_QOS_LMTR_ufsaxi_r_lmtr_bandwidth_END (28)
#define SOC_UFS_Sysctrl_UFSAXI_R_QOS_LMTR_ufsaxi_r_lmtr_en_START (31)
#define SOC_UFS_Sysctrl_UFSAXI_R_QOS_LMTR_ufsaxi_r_lmtr_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int st_clk_ufs : 1;
        unsigned int reserved_0 : 1;
        unsigned int st_clk_ufs_tick : 1;
        unsigned int st_clk_ufsphy_cfg : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 27;
    } reg;
} SOC_UFS_Sysctrl_CRG_UFS_STAT_UNION;
#endif
#define SOC_UFS_Sysctrl_CRG_UFS_STAT_st_clk_ufs_START (0)
#define SOC_UFS_Sysctrl_CRG_UFS_STAT_st_clk_ufs_END (0)
#define SOC_UFS_Sysctrl_CRG_UFS_STAT_st_clk_ufs_tick_START (2)
#define SOC_UFS_Sysctrl_CRG_UFS_STAT_st_clk_ufs_tick_END (2)
#define SOC_UFS_Sysctrl_CRG_UFS_STAT_st_clk_ufsphy_cfg_START (3)
#define SOC_UFS_Sysctrl_CRG_UFS_STAT_st_clk_ufsphy_cfg_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int h8_cnt_en : 1;
        unsigned int h8_cnt_timeout_mask : 1;
        unsigned int pwm_cnt_en : 1;
        unsigned int pwm_cnt_timeout_mask : 1;
        unsigned int ufs_idle_cnt_en : 1;
        unsigned int ufs_idle_cnt_timeout_mask : 1;
        unsigned int ufs_la_int_mask : 1;
        unsigned int device_cnt_en : 1;
        unsigned int device_cnt_timeout_mask : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_UNION;
#endif
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_h8_cnt_en_START (0)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_h8_cnt_en_END (0)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_h8_cnt_timeout_mask_START (1)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_h8_cnt_timeout_mask_END (1)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_pwm_cnt_en_START (2)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_pwm_cnt_en_END (2)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_pwm_cnt_timeout_mask_START (3)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_pwm_cnt_timeout_mask_END (3)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_ufs_idle_cnt_en_START (4)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_ufs_idle_cnt_en_END (4)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_ufs_idle_cnt_timeout_mask_START (5)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_ufs_idle_cnt_timeout_mask_END (5)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_ufs_la_int_mask_START (6)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_ufs_la_int_mask_END (6)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_device_cnt_en_START (7)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_device_cnt_en_END (7)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_device_cnt_timeout_mask_START (8)
#define SOC_UFS_Sysctrl_UFS_DEBUG_CTRL_device_cnt_timeout_mask_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int h8_cnt_timeout : 1;
        unsigned int h8_cnt_timeout_int_mask : 1;
        unsigned int pwm_cnt_timeout : 1;
        unsigned int pwm_cnt_timeout_int_mask : 1;
        unsigned int ufs_idle_timeout : 1;
        unsigned int ufs_idle_timeout_int_mask : 1;
        unsigned int ufs_idle_ind_i : 1;
        unsigned int ufs_la_int_ori : 1;
        unsigned int ufs_la_int : 1;
        unsigned int device_timeout : 1;
        unsigned int device_timeout_int_mask : 1;
        unsigned int reserved : 21;
    } reg;
} SOC_UFS_Sysctrl_UFS_DEBUG_STAT_UNION;
#endif
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_h8_cnt_timeout_START (0)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_h8_cnt_timeout_END (0)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_h8_cnt_timeout_int_mask_START (1)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_h8_cnt_timeout_int_mask_END (1)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_pwm_cnt_timeout_START (2)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_pwm_cnt_timeout_END (2)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_pwm_cnt_timeout_int_mask_START (3)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_pwm_cnt_timeout_int_mask_END (3)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_ufs_idle_timeout_START (4)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_ufs_idle_timeout_END (4)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_ufs_idle_timeout_int_mask_START (5)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_ufs_idle_timeout_int_mask_END (5)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_ufs_idle_ind_i_START (6)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_ufs_idle_ind_i_END (6)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_ufs_la_int_ori_START (7)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_ufs_la_int_ori_END (7)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_ufs_la_int_START (8)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_ufs_la_int_END (8)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_device_timeout_START (9)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_device_timeout_END (9)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_device_timeout_int_mask_START (10)
#define SOC_UFS_Sysctrl_UFS_DEBUG_STAT_device_timeout_int_mask_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int h8_cnt_base : 32;
    } reg;
} SOC_UFS_Sysctrl_AUTO_H8_CONUTER_UNION;
#endif
#define SOC_UFS_Sysctrl_AUTO_H8_CONUTER_h8_cnt_base_START (0)
#define SOC_UFS_Sysctrl_AUTO_H8_CONUTER_h8_cnt_base_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pwm_cnt_base : 32;
    } reg;
} SOC_UFS_Sysctrl_PWM_CONUTER_UNION;
#endif
#define SOC_UFS_Sysctrl_PWM_CONUTER_pwm_cnt_base_START (0)
#define SOC_UFS_Sysctrl_PWM_CONUTER_pwm_cnt_base_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ufs_idle_cnt_base : 32;
    } reg;
} SOC_UFS_Sysctrl_UFS_IDLE_CONUTER_UNION;
#endif
#define SOC_UFS_Sysctrl_UFS_IDLE_CONUTER_ufs_idle_cnt_base_START (0)
#define SOC_UFS_Sysctrl_UFS_IDLE_CONUTER_ufs_idle_cnt_base_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int h8_cnt_crut : 32;
    } reg;
} SOC_UFS_Sysctrl_AUTO_H8_CONUTER_CRUT_UNION;
#endif
#define SOC_UFS_Sysctrl_AUTO_H8_CONUTER_CRUT_h8_cnt_crut_START (0)
#define SOC_UFS_Sysctrl_AUTO_H8_CONUTER_CRUT_h8_cnt_crut_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pwm_cnt_crut : 32;
    } reg;
} SOC_UFS_Sysctrl_PWM_CONUTER_CRUT_UNION;
#endif
#define SOC_UFS_Sysctrl_PWM_CONUTER_CRUT_pwm_cnt_crut_START (0)
#define SOC_UFS_Sysctrl_PWM_CONUTER_CRUT_pwm_cnt_crut_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ufs_idle_cnt_crut : 32;
    } reg;
} SOC_UFS_Sysctrl_UFS_IDLE_CONUTER_CRUT_UNION;
#endif
#define SOC_UFS_Sysctrl_UFS_IDLE_CONUTER_CRUT_ufs_idle_cnt_crut_START (0)
#define SOC_UFS_Sysctrl_UFS_IDLE_CONUTER_CRUT_ufs_idle_cnt_crut_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int h8_cnt_timeout_clr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_UFS_Sysctrl_AUTO_H8_CONUTER_CLR_UNION;
#endif
#define SOC_UFS_Sysctrl_AUTO_H8_CONUTER_CLR_h8_cnt_timeout_clr_START (0)
#define SOC_UFS_Sysctrl_AUTO_H8_CONUTER_CLR_h8_cnt_timeout_clr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pwm_cnt_timeout_clr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_UFS_Sysctrl_PWM_CONUTER_CLR_UNION;
#endif
#define SOC_UFS_Sysctrl_PWM_CONUTER_CLR_pwm_cnt_timeout_clr_START (0)
#define SOC_UFS_Sysctrl_PWM_CONUTER_CLR_pwm_cnt_timeout_clr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ufs_idle_cnt_timeout_clr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_UFS_Sysctrl_UFS_IDLE_CONUTER_CLR_UNION;
#endif
#define SOC_UFS_Sysctrl_UFS_IDLE_CONUTER_CLR_ufs_idle_cnt_timeout_clr_START (0)
#define SOC_UFS_Sysctrl_UFS_IDLE_CONUTER_CLR_ufs_idle_cnt_timeout_clr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tx0_fsm_state_cfg : 4;
        unsigned int tx1_fsm_state_cfg : 4;
        unsigned int rx0_fsm_state_attr : 4;
        unsigned int rx1_fsm_state_attr : 4;
        unsigned int reserved : 16;
    } reg;
} SOC_UFS_Sysctrl_PHY_FSM_STATE_UNION;
#endif
#define SOC_UFS_Sysctrl_PHY_FSM_STATE_tx0_fsm_state_cfg_START (0)
#define SOC_UFS_Sysctrl_PHY_FSM_STATE_tx0_fsm_state_cfg_END (3)
#define SOC_UFS_Sysctrl_PHY_FSM_STATE_tx1_fsm_state_cfg_START (4)
#define SOC_UFS_Sysctrl_PHY_FSM_STATE_tx1_fsm_state_cfg_END (7)
#define SOC_UFS_Sysctrl_PHY_FSM_STATE_rx0_fsm_state_attr_START (8)
#define SOC_UFS_Sysctrl_PHY_FSM_STATE_rx0_fsm_state_attr_END (11)
#define SOC_UFS_Sysctrl_PHY_FSM_STATE_rx1_fsm_state_attr_START (12)
#define SOC_UFS_Sysctrl_PHY_FSM_STATE_rx1_fsm_state_attr_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int device_cnt_base : 32;
    } reg;
} SOC_UFS_Sysctrl_DEVICE_CONUTER_UNION;
#endif
#define SOC_UFS_Sysctrl_DEVICE_CONUTER_device_cnt_base_START (0)
#define SOC_UFS_Sysctrl_DEVICE_CONUTER_device_cnt_base_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int device_cnt_crut : 32;
    } reg;
} SOC_UFS_Sysctrl_DEVICE_CONUTER_CRUT_UNION;
#endif
#define SOC_UFS_Sysctrl_DEVICE_CONUTER_CRUT_device_cnt_crut_START (0)
#define SOC_UFS_Sysctrl_DEVICE_CONUTER_CRUT_device_cnt_crut_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int device_cnt_timeout_clr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_UFS_Sysctrl_DEVICE_CONUTER_CLR_UNION;
#endif
#define SOC_UFS_Sysctrl_DEVICE_CONUTER_CLR_device_cnt_timeout_clr_START (0)
#define SOC_UFS_Sysctrl_DEVICE_CONUTER_CLR_device_cnt_timeout_clr_END (0)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
