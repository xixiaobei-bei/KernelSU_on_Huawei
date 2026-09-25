#ifndef __SOC_CORECRG_B_M_INTERFACE_H__
#define __SOC_CORECRG_B_M_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_CORECRG_B_M_PEREN0_ADDR(base) ((base) + (0x000))
#define SOC_CORECRG_B_M_PERDIS0_ADDR(base) ((base) + (0x004))
#define SOC_CORECRG_B_M_PERCLKEN0_ADDR(base) ((base) + (0x008))
#define SOC_CORECRG_B_M_PERSTAT0_ADDR(base) ((base) + (0x00C))
#define SOC_CORECRG_B_M_CLKDIV0_ADDR(base) ((base) + (0x040))
#define SOC_CORECRG_B_M_PERI_STAT0_ADDR(base) ((base) + (0x060))
#define SOC_CORECRG_B_M_VS_CTRL_EN_BIG0_ADDR(base) ((base) + (0x150))
#define SOC_CORECRG_B_M_VS_CTRL_BYPASS_BIG0_ADDR(base) ((base) + (0x154))
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_ADDR(base) ((base) + (0x158))
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_ADDR(base) ((base) + (0x15C))
#define SOC_CORECRG_B_M_VS_SVFD_CTRL1_BIG0_ADDR(base) ((base) + (0x160))
#define SOC_CORECRG_B_M_VS_SVFD_CTRL2_BIG0_ADDR(base) ((base) + (0x164))
#define SOC_CORECRG_B_M_VS_CPM_DATA_STAT_BIG0_ADDR(base) ((base) + (0x168))
#define SOC_CORECRG_B_M_VS_TEST_STAT_BIG0_ADDR(base) ((base) + (0x16C))
#define SOC_CORECRG_B_M_APLL3SSCCTRL_ADDR(base) ((base) + (0x1E0))
#define SOC_CORECRG_B_M_FCM_CLKEN_ADDR(base) ((base) + (0x200))
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_ADDR(base) ((base) + (0x210))
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG1_ADDR(base) ((base) + (0x214))
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG2_ADDR(base) ((base) + (0x218))
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE0_ADDR(base) ((base) + (0x220))
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_ADDR(base) ((base) + (0x22C))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_apll3_sscg : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
        unsigned int reserved_24 : 7;
    } reg;
} SOC_CORECRG_B_M_PEREN0_UNION;
#endif
#define SOC_CORECRG_B_M_PEREN0_gt_clk_apll3_sscg_START (2)
#define SOC_CORECRG_B_M_PEREN0_gt_clk_apll3_sscg_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_apll3_sscg : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
        unsigned int reserved_24 : 7;
    } reg;
} SOC_CORECRG_B_M_PERDIS0_UNION;
#endif
#define SOC_CORECRG_B_M_PERDIS0_gt_clk_apll3_sscg_START (2)
#define SOC_CORECRG_B_M_PERDIS0_gt_clk_apll3_sscg_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_apll3_sscg : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
        unsigned int reserved_24 : 7;
    } reg;
} SOC_CORECRG_B_M_PERCLKEN0_UNION;
#endif
#define SOC_CORECRG_B_M_PERCLKEN0_gt_clk_apll3_sscg_START (2)
#define SOC_CORECRG_B_M_PERCLKEN0_gt_clk_apll3_sscg_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int st_clk_apll3_sscg : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
        unsigned int reserved_24 : 7;
    } reg;
} SOC_CORECRG_B_M_PERSTAT0_UNION;
#endif
#define SOC_CORECRG_B_M_PERSTAT0_st_clk_apll3_sscg_START (2)
#define SOC_CORECRG_B_M_PERSTAT0_st_clk_apll3_sscg_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 5;
        unsigned int reserved_3: 2;
        unsigned int reserved_4: 2;
        unsigned int reserved_5: 4;
        unsigned int reserved_6: 1;
        unsigned int bitmasken : 16;
    } reg;
} SOC_CORECRG_B_M_CLKDIV0_UNION;
#endif
#define SOC_CORECRG_B_M_CLKDIV0_bitmasken_START (16)
#define SOC_CORECRG_B_M_CLKDIV0_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int st_clk_cpu_big0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 2;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 26;
    } reg;
} SOC_CORECRG_B_M_PERI_STAT0_UNION;
#endif
#define SOC_CORECRG_B_M_PERI_STAT0_st_clk_cpu_big0_START (0)
#define SOC_CORECRG_B_M_PERI_STAT0_st_clk_cpu_big0_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_ctrl_en_big0 : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_CORECRG_B_M_VS_CTRL_EN_BIG0_UNION;
#endif
#define SOC_CORECRG_B_M_VS_CTRL_EN_BIG0_vs_ctrl_en_big0_START (0)
#define SOC_CORECRG_B_M_VS_CTRL_EN_BIG0_vs_ctrl_en_big0_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_ctrl_bypass_big0 : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_CORECRG_B_M_VS_CTRL_BYPASS_BIG0_UNION;
#endif
#define SOC_CORECRG_B_M_VS_CTRL_BYPASS_BIG0_vs_ctrl_bypass_big0_START (0)
#define SOC_CORECRG_B_M_VS_CTRL_BYPASS_BIG0_vs_ctrl_bypass_big0_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vol_drop_en_big0 : 1;
        unsigned int big0_cpu0_wfi_wfe_bypass : 1;
        unsigned int big0_cpu1_wfi_wfe_bypass : 1;
        unsigned int big0_cpu2_wfi_wfe_bypass : 1;
        unsigned int big0_cpu3_wfi_wfe_bypass : 1;
        unsigned int big0_l2_idle_div_mod : 2;
        unsigned int big0_cfg_cnt_cpu_wake_quit : 8;
        unsigned int big0_cfg_cnt_cpu_l2_idle_quit : 8;
        unsigned int big0_cpu_wake_up_mode : 2;
        unsigned int big0_cpu_l2_idle_switch_bypass : 1;
        unsigned int big0_cpu_l2_idle_gt_en : 1;
        unsigned int big0_dvfs_div_en : 1;
        unsigned int reserved : 1;
        unsigned int sel_ckmux_big0_icg : 1;
        unsigned int cpu_clk_div_cfg_big0 : 2;
    } reg;
} SOC_CORECRG_B_M_VS_CTRL_BIG0_UNION;
#endif
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_vol_drop_en_big0_START (0)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_vol_drop_en_big0_END (0)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cpu0_wfi_wfe_bypass_START (1)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cpu0_wfi_wfe_bypass_END (1)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cpu1_wfi_wfe_bypass_START (2)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cpu1_wfi_wfe_bypass_END (2)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cpu2_wfi_wfe_bypass_START (3)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cpu2_wfi_wfe_bypass_END (3)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cpu3_wfi_wfe_bypass_START (4)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cpu3_wfi_wfe_bypass_END (4)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_l2_idle_div_mod_START (5)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_l2_idle_div_mod_END (6)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cfg_cnt_cpu_wake_quit_START (7)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cfg_cnt_cpu_wake_quit_END (14)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cfg_cnt_cpu_l2_idle_quit_START (15)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cfg_cnt_cpu_l2_idle_quit_END (22)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cpu_wake_up_mode_START (23)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cpu_wake_up_mode_END (24)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cpu_l2_idle_switch_bypass_START (25)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cpu_l2_idle_switch_bypass_END (25)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cpu_l2_idle_gt_en_START (26)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_cpu_l2_idle_gt_en_END (26)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_dvfs_div_en_START (27)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_big0_dvfs_div_en_END (27)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_sel_ckmux_big0_icg_START (29)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_sel_ckmux_big0_icg_END (29)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_cpu_clk_div_cfg_big0_START (30)
#define SOC_CORECRG_B_M_VS_CTRL_BIG0_cpu_clk_div_cfg_big0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_data_limit_e_big0 : 1;
        unsigned int svfd_off_time_step_big0 : 1;
        unsigned int svfd_pulse_width_sel_big0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int svfd_ulvt_ll_big0 : 4;
        unsigned int svfd_ulvt_sl_big0 : 4;
        unsigned int svfd_lvt_ll_big0 : 4;
        unsigned int svfd_lvt_sl_big0 : 4;
        unsigned int svfd_trim_big0 : 2;
        unsigned int reserved_1 : 2;
        unsigned int svfd_d_rate_big0 : 2;
        unsigned int svfd_off_mode_big0 : 1;
        unsigned int svfd_div64_en_big0 : 1;
        unsigned int svfd_cpm_period_big0 : 1;
        unsigned int svfd_edge_sel_big0 : 1;
        unsigned int svfd_cmp_data_mode_big0 : 2;
    } reg;
} SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_UNION;
#endif
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_data_limit_e_big0_START (0)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_data_limit_e_big0_END (0)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_off_time_step_big0_START (1)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_off_time_step_big0_END (1)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_pulse_width_sel_big0_START (2)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_pulse_width_sel_big0_END (2)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_ulvt_ll_big0_START (4)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_ulvt_ll_big0_END (7)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_ulvt_sl_big0_START (8)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_ulvt_sl_big0_END (11)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_lvt_ll_big0_START (12)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_lvt_ll_big0_END (15)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_lvt_sl_big0_START (16)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_lvt_sl_big0_END (19)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_trim_big0_START (20)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_trim_big0_END (21)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_d_rate_big0_START (24)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_d_rate_big0_END (25)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_off_mode_big0_START (26)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_off_mode_big0_END (26)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_div64_en_big0_START (27)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_div64_en_big0_END (27)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_cpm_period_big0_START (28)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_cpm_period_big0_END (28)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_edge_sel_big0_START (29)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_edge_sel_big0_END (29)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_cmp_data_mode_big0_START (30)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL0_BIG0_svfd_cmp_data_mode_big0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_glitch_test_1 : 1;
        unsigned int svfd_test_ffs_1 : 2;
        unsigned int svfd_test_cmp_1 : 7;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 19;
    } reg;
} SOC_CORECRG_B_M_VS_SVFD_CTRL1_BIG0_UNION;
#endif
#define SOC_CORECRG_B_M_VS_SVFD_CTRL1_BIG0_svfd_glitch_test_1_START (0)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL1_BIG0_svfd_glitch_test_1_END (0)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL1_BIG0_svfd_test_ffs_1_START (1)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL1_BIG0_svfd_test_ffs_1_END (2)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL1_BIG0_svfd_test_cmp_1_START (3)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL1_BIG0_svfd_test_cmp_1_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_enalbe_mode_1 : 2;
        unsigned int svfs_cpm_threshold_r_1 : 6;
        unsigned int svfs_cpm_threshold_f_1 : 6;
        unsigned int reserved : 2;
        unsigned int bitmasken : 16;
    } reg;
} SOC_CORECRG_B_M_VS_SVFD_CTRL2_BIG0_UNION;
#endif
#define SOC_CORECRG_B_M_VS_SVFD_CTRL2_BIG0_svfd_enalbe_mode_1_START (0)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL2_BIG0_svfd_enalbe_mode_1_END (1)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL2_BIG0_svfs_cpm_threshold_r_1_START (2)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL2_BIG0_svfs_cpm_threshold_r_1_END (7)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL2_BIG0_svfs_cpm_threshold_f_1_START (8)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL2_BIG0_svfs_cpm_threshold_f_1_END (13)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL2_BIG0_bitmasken_START (16)
#define SOC_CORECRG_B_M_VS_SVFD_CTRL2_BIG0_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_cpm_data_r_big0 : 8;
        unsigned int svfd_cpm_data_f_big0 : 8;
        unsigned int svfd_dll_lock_big0 : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_CORECRG_B_M_VS_CPM_DATA_STAT_BIG0_UNION;
#endif
#define SOC_CORECRG_B_M_VS_CPM_DATA_STAT_BIG0_svfd_cpm_data_r_big0_START (0)
#define SOC_CORECRG_B_M_VS_CPM_DATA_STAT_BIG0_svfd_cpm_data_r_big0_END (7)
#define SOC_CORECRG_B_M_VS_CPM_DATA_STAT_BIG0_svfd_cpm_data_f_big0_START (8)
#define SOC_CORECRG_B_M_VS_CPM_DATA_STAT_BIG0_svfd_cpm_data_f_big0_END (15)
#define SOC_CORECRG_B_M_VS_CPM_DATA_STAT_BIG0_svfd_dll_lock_big0_START (16)
#define SOC_CORECRG_B_M_VS_CPM_DATA_STAT_BIG0_svfd_dll_lock_big0_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_test_out_ffs_big0 : 4;
        unsigned int svfd_test_out_cpm_big0 : 4;
        unsigned int svfd_glitch_result_big0 : 1;
        unsigned int idle_low_freq_en_big0 : 1;
        unsigned int vbat_drop_protect_ind_big0 : 1;
        unsigned int reserved : 21;
    } reg;
} SOC_CORECRG_B_M_VS_TEST_STAT_BIG0_UNION;
#endif
#define SOC_CORECRG_B_M_VS_TEST_STAT_BIG0_svfd_test_out_ffs_big0_START (0)
#define SOC_CORECRG_B_M_VS_TEST_STAT_BIG0_svfd_test_out_ffs_big0_END (3)
#define SOC_CORECRG_B_M_VS_TEST_STAT_BIG0_svfd_test_out_cpm_big0_START (4)
#define SOC_CORECRG_B_M_VS_TEST_STAT_BIG0_svfd_test_out_cpm_big0_END (7)
#define SOC_CORECRG_B_M_VS_TEST_STAT_BIG0_svfd_glitch_result_big0_START (8)
#define SOC_CORECRG_B_M_VS_TEST_STAT_BIG0_svfd_glitch_result_big0_END (8)
#define SOC_CORECRG_B_M_VS_TEST_STAT_BIG0_idle_low_freq_en_big0_START (9)
#define SOC_CORECRG_B_M_VS_TEST_STAT_BIG0_idle_low_freq_en_big0_END (9)
#define SOC_CORECRG_B_M_VS_TEST_STAT_BIG0_vbat_drop_protect_ind_big0_START (10)
#define SOC_CORECRG_B_M_VS_TEST_STAT_BIG0_vbat_drop_protect_ind_big0_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll3_ssc_reset : 1;
        unsigned int apll3_ssc_disable : 1;
        unsigned int apll3_ssc_downspread : 1;
        unsigned int apll3_ssc_spread : 3;
        unsigned int apll3_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_CORECRG_B_M_APLL3SSCCTRL_UNION;
#endif
#define SOC_CORECRG_B_M_APLL3SSCCTRL_apll3_ssc_reset_START (0)
#define SOC_CORECRG_B_M_APLL3SSCCTRL_apll3_ssc_reset_END (0)
#define SOC_CORECRG_B_M_APLL3SSCCTRL_apll3_ssc_disable_START (1)
#define SOC_CORECRG_B_M_APLL3SSCCTRL_apll3_ssc_disable_END (1)
#define SOC_CORECRG_B_M_APLL3SSCCTRL_apll3_ssc_downspread_START (2)
#define SOC_CORECRG_B_M_APLL3SSCCTRL_apll3_ssc_downspread_END (2)
#define SOC_CORECRG_B_M_APLL3SSCCTRL_apll3_ssc_spread_START (3)
#define SOC_CORECRG_B_M_APLL3SSCCTRL_apll3_ssc_spread_END (5)
#define SOC_CORECRG_B_M_APLL3SSCCTRL_apll3_ssc_divval_START (6)
#define SOC_CORECRG_B_M_APLL3SSCCTRL_apll3_ssc_divval_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_cpu_big0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 13;
        unsigned int bitmasken : 16;
    } reg;
} SOC_CORECRG_B_M_FCM_CLKEN_UNION;
#endif
#define SOC_CORECRG_B_M_FCM_CLKEN_gt_clk_cpu_big0_START (0)
#define SOC_CORECRG_B_M_FCM_CLKEN_gt_clk_cpu_big0_END (0)
#define SOC_CORECRG_B_M_FCM_CLKEN_bitmasken_START (16)
#define SOC_CORECRG_B_M_FCM_CLKEN_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_jm_clk_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int sc_jm_rst_n : 1;
        unsigned int sc_one2two_cyc : 1;
        unsigned int sc_timing_ctrl_en : 1;
        unsigned int sc_dtc_start : 1;
        unsigned int sc_fdtc_rev : 1;
        unsigned int sc_cdtc_rev : 1;
        unsigned int sc_max2min_cyc : 5;
        unsigned int reserved_1 : 3;
        unsigned int sc_dtc_margin : 10;
        unsigned int reserved_2 : 1;
        unsigned int sc_cdtc_gap : 5;
    } reg;
} SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_UNION;
#endif
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_jm_clk_en_START (0)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_jm_clk_en_END (0)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_jm_rst_n_START (2)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_jm_rst_n_END (2)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_one2two_cyc_START (3)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_one2two_cyc_END (3)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_timing_ctrl_en_START (4)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_timing_ctrl_en_END (4)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_dtc_start_START (5)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_dtc_start_END (5)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_fdtc_rev_START (6)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_fdtc_rev_END (6)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_cdtc_rev_START (7)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_cdtc_rev_END (7)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_max2min_cyc_START (8)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_max2min_cyc_END (12)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_dtc_margin_START (16)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_dtc_margin_END (25)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_cdtc_gap_START (27)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG0_sc_cdtc_gap_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_fdtc_code_init : 8;
        unsigned int sc_cdtc_code_init : 8;
        unsigned int sc_fdtc_cyc : 5;
        unsigned int reserved_0 : 3;
        unsigned int sc_cdtc_cyc : 5;
        unsigned int reserved_1 : 3;
    } reg;
} SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG1_UNION;
#endif
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG1_sc_fdtc_code_init_START (0)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG1_sc_fdtc_code_init_END (7)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG1_sc_cdtc_code_init_START (8)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG1_sc_cdtc_code_init_END (15)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG1_sc_fdtc_cyc_START (16)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG1_sc_fdtc_cyc_END (20)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG1_sc_cdtc_cyc_START (24)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG1_sc_cdtc_cyc_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_fdtc_step : 7;
        unsigned int reserved_0 : 1;
        unsigned int sc_cdtc_step : 6;
        unsigned int reserved_1 : 18;
    } reg;
} SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG2_UNION;
#endif
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG2_sc_fdtc_step_START (0)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG2_sc_fdtc_step_END (6)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG2_sc_cdtc_step_START (8)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_CFG2_sc_cdtc_step_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int jm_code_b : 8;
        unsigned int jm_code_a : 8;
        unsigned int jm_code_d : 8;
        unsigned int jm_code_c : 8;
    } reg;
} SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE0_UNION;
#endif
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE0_jm_code_b_START (0)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE0_jm_code_b_END (7)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE0_jm_code_a_START (8)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE0_jm_code_a_END (15)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE0_jm_code_d_START (16)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE0_jm_code_d_END (23)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE0_jm_code_c_START (24)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE0_jm_code_c_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int jm_dtc_mea_done : 1;
        unsigned int test_comp : 1;
        unsigned int jm_underflow : 1;
        unsigned int jm_overflow : 1;
        unsigned int test_dtc_curr_st : 3;
        unsigned int reserved : 1;
        unsigned int jm_code_e : 8;
        unsigned int jm_code_min : 8;
        unsigned int jm_code_max : 8;
    } reg;
} SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_UNION;
#endif
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_jm_dtc_mea_done_START (0)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_jm_dtc_mea_done_END (0)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_test_comp_START (1)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_test_comp_END (1)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_jm_underflow_START (2)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_jm_underflow_END (2)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_jm_overflow_START (3)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_jm_overflow_END (3)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_test_dtc_curr_st_START (4)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_test_dtc_curr_st_END (6)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_jm_code_e_START (8)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_jm_code_e_END (15)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_jm_code_min_START (16)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_jm_code_min_END (23)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_jm_code_max_START (24)
#define SOC_CORECRG_B_M_JITTER_MONITOR_APLL3_STATE1_jm_code_max_END (31)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
