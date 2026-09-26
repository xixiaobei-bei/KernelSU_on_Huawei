#ifndef __SOC_PMCTRL_INTERFACE_H__
#define __SOC_PMCTRL_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_PMCTRL_APLL_CTRL_SRC_ADDR(base) ((base) + (0x100UL))
#define SOC_PMCTRL_SUBSYS_VOL_FREQ_CTRL_SRC_ADDR(base) ((base) + (0x104UL))
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_ADDR(base) ((base) + (0x108UL))
#define SOC_PMCTRL_TIME_PARAMETER_CFG_0_ADDR(base) ((base) + (0x10CUL))
#define SOC_PMCTRL_TIME_PARAMETER_CFG_1_ADDR(base) ((base) + (0x110UL))
#define SOC_PMCTRL_CPUL_DVFS_CLK_STAT_ADDR(base) ((base) + (0x150UL))
#define SOC_PMCTRL_CPUM_DVFS_CLK_STAT_ADDR(base) ((base) + (0x154UL))
#define SOC_PMCTRL_CPUB_DVFS_CLK_STAT_ADDR(base) ((base) + (0x158UL))
#define SOC_PMCTRL_FCM_DVFS_CLK_STAT_ADDR(base) ((base) + (0x15CUL))
#define SOC_PMCTRL_GPU_DVFS_CLK_STAT_ADDR(base) ((base) + (0x160UL))
#define SOC_PMCTRL_GPU_TOP_DVFS_CLK_STAT_ADDR(base) ((base) + (0x164UL))
#define SOC_PMCTRL_CPUL_DVFS_VOL_STAT_ADDR(base) ((base) + (0x170UL))
#define SOC_PMCTRL_CPUM_DVFS_VOL_STAT_ADDR(base) ((base) + (0x174UL))
#define SOC_PMCTRL_CPUB_DVFS_VOL_STAT_ADDR(base) ((base) + (0x178UL))
#define SOC_PMCTRL_FCM_DVFS_VOL_STAT_ADDR(base) ((base) + (0x17CUL))
#define SOC_PMCTRL_GPU_CORE_DVFS_VOL_STAT_ADDR(base) ((base) + (0x180UL))
#define SOC_PMCTRL_GPU_TOP_DVFS_VOL_STAT_ADDR(base) ((base) + (0x184UL))
#define SOC_PMCTRL_GPU_HPMCLKDIV_ADDR(base) ((base) + (0x120UL))
#define SOC_PMCTRL_GPU_HPMSEL_ADDR(base) ((base) + (0x124UL))
#define SOC_PMCTRL_GPU_HPMEN_ADDR(base) ((base) + (0x128UL))
#define SOC_PMCTRL_GPU_HPMOPC_ADDR(base) ((base) + (0x12CUL))
#define SOC_PMCTRL_PERIBAKDATA0_ADDR(base) ((base) + (0x130UL))
#define SOC_PMCTRL_PERIBAKDATA1_ADDR(base) ((base) + (0x134UL))
#define SOC_PMCTRL_PERIBAKDATA2_ADDR(base) ((base) + (0x138UL))
#define SOC_PMCTRL_PERIBAKDATA3_ADDR(base) ((base) + (0x13CUL))
#define SOC_PMCTRL_SPMIVOLCFG_ADDR(base) ((base) + (0x114UL))
#define SOC_PMCTRL_SPMIVOLCFG1_ADDR(base) ((base) + (0x118UL))
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_ADDR(base) ((base) + (0x1D8UL))
#define SOC_PMCTRL_FREQ_VOTE_A0_ADDR(base) ((base) + (0x200UL))
#define SOC_PMCTRL_FREQ_VOTE_A1_ADDR(base) ((base) + (0x204UL))
#define SOC_PMCTRL_FREQ_VOTE_B0_ADDR(base) ((base) + (0x208UL))
#define SOC_PMCTRL_FREQ_VOTE_B1_ADDR(base) ((base) + (0x20CUL))
#define SOC_PMCTRL_FREQ_VOTE_C0_ADDR(base) ((base) + (0x210UL))
#define SOC_PMCTRL_FREQ_VOTE_C1_ADDR(base) ((base) + (0x214UL))
#define SOC_PMCTRL_FREQ_VOTE_D0_ADDR(base) ((base) + (0x218UL))
#define SOC_PMCTRL_FREQ_VOTE_D1_ADDR(base) ((base) + (0x21CUL))
#define SOC_PMCTRL_FREQ_VOTE_E0_ADDR(base) ((base) + (0x220UL))
#define SOC_PMCTRL_FREQ_VOTE_E1_ADDR(base) ((base) + (0x224UL))
#define SOC_PMCTRL_FREQ_VOTE_E2_ADDR(base) ((base) + (0x228UL))
#define SOC_PMCTRL_FREQ_VOTE_E3_ADDR(base) ((base) + (0x22CUL))
#define SOC_PMCTRL_FREQ_VOTE_F0_ADDR(base) ((base) + (0x230UL))
#define SOC_PMCTRL_FREQ_VOTE_F1_ADDR(base) ((base) + (0x234UL))
#define SOC_PMCTRL_FREQ_VOTE_F2_ADDR(base) ((base) + (0x238UL))
#define SOC_PMCTRL_FREQ_VOTE_F3_ADDR(base) ((base) + (0x23CUL))
#define SOC_PMCTRL_FREQ_VOTE_G0_ADDR(base) ((base) + (0x240UL))
#define SOC_PMCTRL_FREQ_VOTE_G1_ADDR(base) ((base) + (0x244UL))
#define SOC_PMCTRL_FREQ_VOTE_G2_ADDR(base) ((base) + (0x248UL))
#define SOC_PMCTRL_FREQ_VOTE_G3_ADDR(base) ((base) + (0x24CUL))
#define SOC_PMCTRL_FREQ_VOTE_H0_ADDR(base) ((base) + (0x250UL))
#define SOC_PMCTRL_FREQ_VOTE_H1_ADDR(base) ((base) + (0x254UL))
#define SOC_PMCTRL_FREQ_VOTE_H2_ADDR(base) ((base) + (0x258UL))
#define SOC_PMCTRL_FREQ_VOTE_H3_ADDR(base) ((base) + (0x25CUL))
#define SOC_PMCTRL_FREQ_VOTE_H4_ADDR(base) ((base) + (0x260UL))
#define SOC_PMCTRL_FREQ_VOTE_H5_ADDR(base) ((base) + (0x264UL))
#define SOC_PMCTRL_FREQ_VOTE_H6_ADDR(base) ((base) + (0x268UL))
#define SOC_PMCTRL_FREQ_VOTE_H7_ADDR(base) ((base) + (0x26CUL))
#define SOC_PMCTRL_FREQ_VOTE_I0_ADDR(base) ((base) + (0x270UL))
#define SOC_PMCTRL_FREQ_VOTE_I1_ADDR(base) ((base) + (0x274UL))
#define SOC_PMCTRL_FREQ_VOTE_I2_ADDR(base) ((base) + (0x278UL))
#define SOC_PMCTRL_FREQ_VOTE_I3_ADDR(base) ((base) + (0x27CUL))
#define SOC_PMCTRL_FREQ_VOTE_I4_ADDR(base) ((base) + (0x280UL))
#define SOC_PMCTRL_FREQ_VOTE_I5_ADDR(base) ((base) + (0x284UL))
#define SOC_PMCTRL_FREQ_VOTE_I6_ADDR(base) ((base) + (0x288UL))
#define SOC_PMCTRL_FREQ_VOTE_I7_ADDR(base) ((base) + (0x28CUL))
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_ADDR(base) ((base) + (0x2B0UL))
#define SOC_PMCTRL_VOTE_MODE_CFG_0_ADDR(base) ((base) + (0x2BCUL))
#define SOC_PMCTRL_VOTE_MODE_CFG_1_ADDR(base) ((base) + (0x2C0UL))
#define SOC_PMCTRL_VOTE_START_EN_ADDR(base) ((base) + (0x2C4UL))
#define SOC_PMCTRL_INT_VOTE_EN_ADDR(base) ((base) + (0x2C8UL))
#define SOC_PMCTRL_INT_VOTE_CLR_ADDR(base) ((base) + (0x2CCUL))
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_ADDR(base) ((base) + (0x2D0UL))
#define SOC_PMCTRL_INT_VOTE_STAT_ADDR(base) ((base) + (0x2D4UL))
#define SOC_PMCTRL_INT_FREQUENCY_VOTE_ADDR(base) ((base) + (0x2D8UL))
#define SOC_PMCTRL_VOTE_RESULT_A_ADDR(base) ((base) + (0x2DCUL))
#define SOC_PMCTRL_VOTE_RESULT_B_ADDR(base) ((base) + (0x2E0UL))
#define SOC_PMCTRL_VOTE_RESULT_C_ADDR(base) ((base) + (0x2E4UL))
#define SOC_PMCTRL_VOTE_RESULT_D_ADDR(base) ((base) + (0x2E8UL))
#define SOC_PMCTRL_VOTE_RESULT_E_ADDR(base) ((base) + (0x2ECUL))
#define SOC_PMCTRL_VOTE_RESULT_F_ADDR(base) ((base) + (0x2F0UL))
#define SOC_PMCTRL_VOTE_RESULT_G_ADDR(base) ((base) + (0x2F4UL))
#define SOC_PMCTRL_VOTE_RESULT_H_ADDR(base) ((base) + (0x2F8UL))
#define SOC_PMCTRL_VOTE_RESULT_I_ADDR(base) ((base) + (0x2FCUL))
#define SOC_PMCTRL_DDRLPCTRL_ADDR(base) ((base) + (0x30CUL))
#define SOC_PMCTRL_PERI_CTRL0_ADDR(base) ((base) + (0x340UL))
#define SOC_PMCTRL_PERI_CTRL1_ADDR(base) ((base) + (0x344UL))
#define SOC_PMCTRL_PERI_CTRL2_ADDR(base) ((base) + (0x348UL))
#define SOC_PMCTRL_PERI_CTRL3_ADDR(base) ((base) + (0x34CUL))
#define SOC_PMCTRL_PERI_CTRL4_ADDR(base) ((base) + (0x350UL))
#define SOC_PMCTRL_PERI_CTRL5_ADDR(base) ((base) + (0x354UL))
#define SOC_PMCTRL_PERI_CTRL6_ADDR(base) ((base) + (0x358UL))
#define SOC_PMCTRL_PERI_CTRL7_ADDR(base) ((base) + (0x35CUL))
#define SOC_PMCTRL_PERI_STAT0_ADDR(base) ((base) + (0x360UL))
#define SOC_PMCTRL_PERI_STAT1_ADDR(base) ((base) + (0x364UL))
#define SOC_PMCTRL_PERI_STAT2_ADDR(base) ((base) + (0x368UL))
#define SOC_PMCTRL_PERI_STAT3_ADDR(base) ((base) + (0x36CUL))
#define SOC_PMCTRL_PERI_STAT4_ADDR(base) ((base) + (0x370UL))
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_ADDR(base) ((base) + (0x380UL))
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_ADDR(base) ((base) + (0x384UL))
#define SOC_PMCTRL_NOC_POWER_IDLE_0_ADDR(base) ((base) + (0x388UL))
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_ADDR(base) ((base) + (0x38CUL))
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_ADDR(base) ((base) + (0x390UL))
#define SOC_PMCTRL_NOC_POWER_IDLE_1_ADDR(base) ((base) + (0x394UL))
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_2_ADDR(base) ((base) + (0x398UL))
#define SOC_PMCTRL_NOC_POWER_IDLEACK_2_ADDR(base) ((base) + (0x39CUL))
#define SOC_PMCTRL_NOC_POWER_IDLE_2_ADDR(base) ((base) + (0x374UL))
#define SOC_PMCTRL_PERI_INT0_MASK_ADDR(base) ((base) + (0x3A0UL))
#define SOC_PMCTRL_PERI_INT0_STAT_ADDR(base) ((base) + (0x3A4UL))
#define SOC_PMCTRL_PERI_INT1_MASK_ADDR(base) ((base) + (0x3A8UL))
#define SOC_PMCTRL_PERI_INT1_STAT_ADDR(base) ((base) + (0x3ACUL))
#define SOC_PMCTRL_PERI_INT2_MASK_ADDR(base) ((base) + (0x3B0UL))
#define SOC_PMCTRL_PERI_INT2_STAT_ADDR(base) ((base) + (0x3B4UL))
#define SOC_PMCTRL_PERI_INT3_MASK_ADDR(base) ((base) + (0x3B8UL))
#define SOC_PMCTRL_PERI_INT3_STAT_ADDR(base) ((base) + (0x3BCUL))
#define SOC_PMCTRL_VS_CTRL_EN_GPU_ADDR(base) ((base) + (0x440UL))
#define SOC_PMCTRL_VS_CTRL_BYPASS_GPU_ADDR(base) ((base) + (0x444UL))
#define SOC_PMCTRL_VS_CTRL_GPU_ADDR(base) ((base) + (0x448UL))
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_ADDR(base) ((base) + (0x44CUL))
#define SOC_PMCTRL_VS_SVFD_CTRL1_GPU_ADDR(base) ((base) + (0x450UL))
#define SOC_PMCTRL_VS_SVFD_CTRL2_GPU_ADDR(base) ((base) + (0x454UL))
#define SOC_PMCTRL_VS_CPM_DATA_STAT_GPU_ADDR(base) ((base) + (0x458UL))
#define SOC_PMCTRL_VS_TEST_STAT_GPU_ADDR(base) ((base) + (0x45CUL))
#define SOC_PMCTRL_BOOTROMFLAG_ADDR(base) ((base) + (0x460UL))
#define SOC_PMCTRL_PERIHPMEN_ADDR(base) ((base) + (0x464UL))
#define SOC_PMCTRL_PERIHPMCLKDIV_ADDR(base) ((base) + (0x468UL))
#define SOC_PMCTRL_PERIHPMOPC_ADDR(base) ((base) + (0x46CUL))
#define SOC_PMCTRL_PERI_STAT8_ADDR(base) ((base) + (0x4D0UL))
#define SOC_PMCTRL_PERI_STAT9_ADDR(base) ((base) + (0x4D4UL))
#define SOC_PMCTRL_PERI_STAT10_ADDR(base) ((base) + (0x4D8UL))
#define SOC_PMCTRL_PERI_STAT11_ADDR(base) ((base) + (0x4DCUL))
#define SOC_PMCTRL_PERI_STAT12_ADDR(base) ((base) + (0x4E0UL))
#define SOC_PMCTRL_PERI_STAT13_ADDR(base) ((base) + (0x4E4UL))
#define SOC_PMCTRL_PERI_STAT14_ADDR(base) ((base) + (0x4E8UL))
#define SOC_PMCTRL_PERI_STAT15_ADDR(base) ((base) + (0x4ECUL))
#define SOC_PMCTRL_APLL0CTRL0_ADDR(base) ((base) + (0x500UL))
#define SOC_PMCTRL_APLL0CTRL1_ADDR(base) ((base) + (0x504UL))
#define SOC_PMCTRL_APLL0CTRL0_STAT_ADDR(base) ((base) + (0x508UL))
#define SOC_PMCTRL_APLL0CTRL1_STAT_ADDR(base) ((base) + (0x50CUL))
#define SOC_PMCTRL_APLL1CTRL0_ADDR(base) ((base) + (0x510UL))
#define SOC_PMCTRL_APLL1CTRL1_ADDR(base) ((base) + (0x514UL))
#define SOC_PMCTRL_APLL1CTRL0_STAT_ADDR(base) ((base) + (0x518UL))
#define SOC_PMCTRL_APLL1CTRL1_STAT_ADDR(base) ((base) + (0x51CUL))
#define SOC_PMCTRL_APLL2CTRL0_ADDR(base) ((base) + (0x520UL))
#define SOC_PMCTRL_APLL2CTRL1_ADDR(base) ((base) + (0x524UL))
#define SOC_PMCTRL_APLL2CTRL0_STAT_ADDR(base) ((base) + (0x528UL))
#define SOC_PMCTRL_APLL2CTRL1_STAT_ADDR(base) ((base) + (0x52CUL))
#define SOC_PMCTRL_APLL3CTRL0_ADDR(base) ((base) + (0x530UL))
#define SOC_PMCTRL_APLL3CTRL1_ADDR(base) ((base) + (0x534UL))
#define SOC_PMCTRL_APLL3CTRL0_STAT_ADDR(base) ((base) + (0x538UL))
#define SOC_PMCTRL_APLL3CTRL1_STAT_ADDR(base) ((base) + (0x53CUL))
#define SOC_PMCTRL_APLL4CTRL0_ADDR(base) ((base) + (0x540UL))
#define SOC_PMCTRL_APLL4CTRL1_ADDR(base) ((base) + (0x544UL))
#define SOC_PMCTRL_APLL4CTRL0_STAT_ADDR(base) ((base) + (0x548UL))
#define SOC_PMCTRL_APLL4CTRL1_STAT_ADDR(base) ((base) + (0x54CUL))
#define SOC_PMCTRL_APLL5CTRL0_ADDR(base) ((base) + (0x550UL))
#define SOC_PMCTRL_APLL5CTRL1_ADDR(base) ((base) + (0x554UL))
#define SOC_PMCTRL_APLL5CTRL0_STAT_ADDR(base) ((base) + (0x558UL))
#define SOC_PMCTRL_APLL5CTRL1_STAT_ADDR(base) ((base) + (0x55CUL))
#define SOC_PMCTRL_APLL6CTRL0_ADDR(base) ((base) + (0x560UL))
#define SOC_PMCTRL_APLL6CTRL1_ADDR(base) ((base) + (0x564UL))
#define SOC_PMCTRL_APLL6CTRL0_STAT_ADDR(base) ((base) + (0x568UL))
#define SOC_PMCTRL_APLL6CTRL1_STAT_ADDR(base) ((base) + (0x56CUL))
#define SOC_PMCTRL_APLL7CTRL0_ADDR(base) ((base) + (0x570UL))
#define SOC_PMCTRL_APLL7CTRL1_ADDR(base) ((base) + (0x574UL))
#define SOC_PMCTRL_APLL7CTRL0_STAT_ADDR(base) ((base) + (0x578UL))
#define SOC_PMCTRL_APLL7CTRL1_STAT_ADDR(base) ((base) + (0x57CUL))
#define SOC_PMCTRL_APLL0_SW_INITIAL0_ADDR(base) ((base) + (0x5E0UL))
#define SOC_PMCTRL_APLL0_SW_INITIAL1_ADDR(base) ((base) + (0x5E4UL))
#define SOC_PMCTRL_APLL1_SW_INITIAL0_ADDR(base) ((base) + (0x5E8UL))
#define SOC_PMCTRL_APLL1_SW_INITIAL1_ADDR(base) ((base) + (0x5ECUL))
#define SOC_PMCTRL_APLL2_SW_INITIAL0_ADDR(base) ((base) + (0x5F0UL))
#define SOC_PMCTRL_APLL2_SW_INITIAL1_ADDR(base) ((base) + (0x5F4UL))
#define SOC_PMCTRL_APLL3_SW_INITIAL0_ADDR(base) ((base) + (0x5F8UL))
#define SOC_PMCTRL_APLL3_SW_INITIAL1_ADDR(base) ((base) + (0x5FCUL))
#define SOC_PMCTRL_APLL4_SW_INITIAL0_ADDR(base) ((base) + (0x600UL))
#define SOC_PMCTRL_APLL4_SW_INITIAL1_ADDR(base) ((base) + (0x604UL))
#define SOC_PMCTRL_APLL5_SW_INITIAL0_ADDR(base) ((base) + (0x608UL))
#define SOC_PMCTRL_APLL5_SW_INITIAL1_ADDR(base) ((base) + (0x60CUL))
#define SOC_PMCTRL_APLL6_SW_INITIAL0_ADDR(base) ((base) + (0x610UL))
#define SOC_PMCTRL_APLL6_SW_INITIAL1_ADDR(base) ((base) + (0x614UL))
#define SOC_PMCTRL_APLL7_SW_INITIAL0_ADDR(base) ((base) + (0x618UL))
#define SOC_PMCTRL_APLL7_SW_INITIAL1_ADDR(base) ((base) + (0x61CUL))
#define SOC_PMCTRL_PDVFS_INSTR_LIST_A_ADDR(base,ma) ((base) + (0xC00 + (ma)*4UL))
#define SOC_PMCTRL_PDVFS_INSTR_LIST_B_ADDR(base,mb) ((base) + (0xC80 + (mb)*4UL))
#define SOC_PMCTRL_PDVFS_INSTR_LIST_C_ADDR(base,mc) ((base) + (0xD00 + (mc)*4UL))
#define SOC_PMCTRL_PDVFS_INSTR_LIST_D_ADDR(base,md) ((base) + (0xD80 + (md)*4UL))
#define SOC_PMCTRL_PDVFS_INSTR_LIST_E_ADDR(base,me) ((base) + (0xE00 + (me)*4UL))
#define SOC_PMCTRL_PDVFS_INSTR_LIST_F_ADDR(base,mf) ((base) + (0xE80 + (mf)*4UL))
#define SOC_PMCTRL_INSTR_ADDR_CTRL_A_ADDR(base) ((base) + (0x700UL))
#define SOC_PMCTRL_INSTR_ADDR_CTRL_B_ADDR(base) ((base) + (0x704UL))
#define SOC_PMCTRL_INSTR_ADDR_CTRL_C_ADDR(base) ((base) + (0x708UL))
#define SOC_PMCTRL_INSTR_ADDR_CTRL_D_ADDR(base) ((base) + (0x70CUL))
#define SOC_PMCTRL_INSTR_ADDR_CTRL_E_ADDR(base) ((base) + (0x710UL))
#define SOC_PMCTRL_INSTR_ADDR_CTRL_F_ADDR(base) ((base) + (0x714UL))
#define SOC_PMCTRL_INSTR_ADDR_CTRL_G_ADDR(base) ((base) + (0x718UL))
#define SOC_PMCTRL_INSTR_ADDR_CTRL_H_ADDR(base) ((base) + (0x71CUL))
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_ADDR(base) ((base) + (0x720UL))
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_ADDR(base) ((base) + (0x724UL))
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_ADDR(base) ((base) + (0x730UL))
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_ADDR(base) ((base) + (0x734UL))
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_ADDR(base) ((base) + (0x738UL))
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_ADDR(base) ((base) + (0x73CUL))
#define SOC_PMCTRL_TASKCTRL_A_0_ADDR(base) ((base) + (0x740UL))
#define SOC_PMCTRL_TASKCTRL_A_1_ADDR(base) ((base) + (0x744UL))
#define SOC_PMCTRL_TASKCTRL_A_2_ADDR(base) ((base) + (0x748UL))
#define SOC_PMCTRL_TASKCTRL_A_3_ADDR(base) ((base) + (0x74CUL))
#define SOC_PMCTRL_TASKCTRL_B_0_ADDR(base) ((base) + (0x750UL))
#define SOC_PMCTRL_TASKCTRL_B_1_ADDR(base) ((base) + (0x754UL))
#define SOC_PMCTRL_TASKCTRL_B_2_ADDR(base) ((base) + (0x758UL))
#define SOC_PMCTRL_TASKCTRL_B_3_ADDR(base) ((base) + (0x75CUL))
#define SOC_PMCTRL_TASKCTRL_C_0_ADDR(base) ((base) + (0x760UL))
#define SOC_PMCTRL_TASKCTRL_C_1_ADDR(base) ((base) + (0x764UL))
#define SOC_PMCTRL_TASKCTRL_C_2_ADDR(base) ((base) + (0x768UL))
#define SOC_PMCTRL_TASKCTRL_C_3_ADDR(base) ((base) + (0x76CUL))
#define SOC_PMCTRL_TASKCTRL_D_0_ADDR(base) ((base) + (0x770UL))
#define SOC_PMCTRL_TASKCTRL_D_1_ADDR(base) ((base) + (0x774UL))
#define SOC_PMCTRL_TASKCTRL_D_2_ADDR(base) ((base) + (0x778UL))
#define SOC_PMCTRL_TASKCTRL_D_3_ADDR(base) ((base) + (0x77CUL))
#define SOC_PMCTRL_TASKCTRL_E_0_ADDR(base) ((base) + (0x780UL))
#define SOC_PMCTRL_TASKCTRL_E_1_ADDR(base) ((base) + (0x784UL))
#define SOC_PMCTRL_TASKCTRL_E_2_ADDR(base) ((base) + (0x788UL))
#define SOC_PMCTRL_TASKCTRL_E_3_ADDR(base) ((base) + (0x78CUL))
#define SOC_PMCTRL_TASKCTRL_F_0_ADDR(base) ((base) + (0x790UL))
#define SOC_PMCTRL_TASKCTRL_F_1_ADDR(base) ((base) + (0x794UL))
#define SOC_PMCTRL_TASKCTRL_F_2_ADDR(base) ((base) + (0x798UL))
#define SOC_PMCTRL_TASKCTRL_F_3_ADDR(base) ((base) + (0x79CUL))
#define SOC_PMCTRL_TASKCTRL_G_0_ADDR(base) ((base) + (0x7A0UL))
#define SOC_PMCTRL_TASKCTRL_G_1_ADDR(base) ((base) + (0x7A4UL))
#define SOC_PMCTRL_TASKCTRL_G_2_ADDR(base) ((base) + (0x7A8UL))
#define SOC_PMCTRL_TASKCTRL_G_3_ADDR(base) ((base) + (0x7ACUL))
#define SOC_PMCTRL_TASKCTRL_H_0_ADDR(base) ((base) + (0x7B0UL))
#define SOC_PMCTRL_TASKCTRL_H_1_ADDR(base) ((base) + (0x7B4UL))
#define SOC_PMCTRL_TASKCTRL_H_2_ADDR(base) ((base) + (0x7B8UL))
#define SOC_PMCTRL_TASKCTRL_H_3_ADDR(base) ((base) + (0x7BCUL))
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_ADDR(base) ((base) + (0x7C0UL))
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_ADDR(base) ((base) + (0x7C4UL))
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_ADDR(base) ((base) + (0x7C8UL))
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_ADDR(base) ((base) + (0x7CCUL))
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_ADDR(base) ((base) + (0x7D0UL))
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_ADDR(base) ((base) + (0x7D4UL))
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_ADDR(base) ((base) + (0x7D8UL))
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_ADDR(base) ((base) + (0x7DCUL))
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_ADDR(base) ((base) + (0x7E0UL))
#define SOC_PMCTRL_COFSM_VOL_SENDOUT_ADDR(base) ((base) + (0x7E4UL))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_ADDR(base) ((base) + (0x800UL))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_1_ADDR(base) ((base) + (0x804UL))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_ADDR(base) ((base) + (0x808UL))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_1_ADDR(base) ((base) + (0x80CUL))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_ADDR(base) ((base) + (0x810UL))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_1_ADDR(base) ((base) + (0x814UL))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_ADDR(base) ((base) + (0x818UL))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_1_ADDR(base) ((base) + (0x81CUL))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_ADDR(base) ((base) + (0x820UL))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_1_ADDR(base) ((base) + (0x824UL))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_ADDR(base) ((base) + (0x828UL))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_1_ADDR(base) ((base) + (0x82CUL))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_ADDR(base) ((base) + (0x830UL))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_1_ADDR(base) ((base) + (0x834UL))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_ADDR(base) ((base) + (0x838UL))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_1_ADDR(base) ((base) + (0x83CUL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_0_ADDR(base) ((base) + (0x840UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_ADDR(base) ((base) + (0x844UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_2_ADDR(base) ((base) + (0x848UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_3_ADDR(base) ((base) + (0x84CUL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_0_ADDR(base) ((base) + (0x850UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_ADDR(base) ((base) + (0x854UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_ADDR(base) ((base) + (0x858UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_3_ADDR(base) ((base) + (0x85CUL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_0_ADDR(base) ((base) + (0x860UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_ADDR(base) ((base) + (0x864UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_2_ADDR(base) ((base) + (0x868UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_3_ADDR(base) ((base) + (0x86CUL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_0_ADDR(base) ((base) + (0x870UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_ADDR(base) ((base) + (0x874UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_2_ADDR(base) ((base) + (0x878UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_3_ADDR(base) ((base) + (0x87CUL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_0_ADDR(base) ((base) + (0x880UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_ADDR(base) ((base) + (0x884UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_2_ADDR(base) ((base) + (0x888UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_3_ADDR(base) ((base) + (0x88CUL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_0_ADDR(base) ((base) + (0x890UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_ADDR(base) ((base) + (0x894UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_ADDR(base) ((base) + (0x898UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_3_ADDR(base) ((base) + (0x89CUL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_0_ADDR(base) ((base) + (0x8A0UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_ADDR(base) ((base) + (0x8A4UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_ADDR(base) ((base) + (0x8A8UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_3_ADDR(base) ((base) + (0x8ACUL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_0_ADDR(base) ((base) + (0x8B0UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_ADDR(base) ((base) + (0x8B4UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_ADDR(base) ((base) + (0x8B8UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_3_ADDR(base) ((base) + (0x8BCUL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_0_ADDR(base) ((base) + (0x8C0UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_ADDR(base) ((base) + (0x8C4UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_2_ADDR(base) ((base) + (0x8C8UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_3_ADDR(base) ((base) + (0x8CCUL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_0_ADDR(base) ((base) + (0x8D0UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_ADDR(base) ((base) + (0x8D4UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_2_ADDR(base) ((base) + (0x8D8UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_3_ADDR(base) ((base) + (0x8DCUL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_0_ADDR(base) ((base) + (0x8E0UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_ADDR(base) ((base) + (0x8E4UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_ADDR(base) ((base) + (0x8E8UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_3_ADDR(base) ((base) + (0x8ECUL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_0_ADDR(base) ((base) + (0x8F0UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_ADDR(base) ((base) + (0x8F4UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_ADDR(base) ((base) + (0x8F8UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_3_ADDR(base) ((base) + (0x8FCUL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_0_ADDR(base) ((base) + (0x9A0UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_ADDR(base) ((base) + (0x9A4UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_ADDR(base) ((base) + (0x9A8UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_3_ADDR(base) ((base) + (0x9ACUL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_0_ADDR(base) ((base) + (0x9B0UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_ADDR(base) ((base) + (0x9B4UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_ADDR(base) ((base) + (0x9B8UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_3_ADDR(base) ((base) + (0x9BCUL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_0_ADDR(base) ((base) + (0x9C0UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_ADDR(base) ((base) + (0x9C4UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_ADDR(base) ((base) + (0x9C8UL))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_3_ADDR(base) ((base) + (0x9CCUL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_0_ADDR(base) ((base) + (0x9D0UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_ADDR(base) ((base) + (0x9D4UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_ADDR(base) ((base) + (0x9D8UL))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_3_ADDR(base) ((base) + (0x9DCUL))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_A_0_ADDR(base) ((base) + (0x670UL))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_A_1_ADDR(base) ((base) + (0x674UL))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_A_0_ADDR(base) ((base) + (0x678UL))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_A_1_ADDR(base) ((base) + (0x67CUL))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_B_0_ADDR(base) ((base) + (0x680UL))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_B_1_ADDR(base) ((base) + (0x684UL))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_B_0_ADDR(base) ((base) + (0x688UL))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_B_1_ADDR(base) ((base) + (0x68CUL))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_C_0_ADDR(base) ((base) + (0x690UL))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_C_1_ADDR(base) ((base) + (0x694UL))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_C_0_ADDR(base) ((base) + (0x698UL))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_C_1_ADDR(base) ((base) + (0x69CUL))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_D_0_ADDR(base) ((base) + (0x6A0UL))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_D_1_ADDR(base) ((base) + (0x6A4UL))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_D_0_ADDR(base) ((base) + (0x6A8UL))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_D_1_ADDR(base) ((base) + (0x6ACUL))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_E_0_ADDR(base) ((base) + (0x6B0UL))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_E_1_ADDR(base) ((base) + (0x6B4UL))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_E_0_ADDR(base) ((base) + (0x6B8UL))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_E_1_ADDR(base) ((base) + (0x6BCUL))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_F_0_ADDR(base) ((base) + (0x6C0UL))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_F_1_ADDR(base) ((base) + (0x6C4UL))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_F_0_ADDR(base) ((base) + (0x6C8UL))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_F_1_ADDR(base) ((base) + (0x6CCUL))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_G_0_ADDR(base) ((base) + (0x6D0UL))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_G_1_ADDR(base) ((base) + (0x6D4UL))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_G_0_ADDR(base) ((base) + (0x6D8UL))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_G_1_ADDR(base) ((base) + (0x6DCUL))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_H_0_ADDR(base) ((base) + (0x6E0UL))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_H_1_ADDR(base) ((base) + (0x6E4UL))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_H_0_ADDR(base) ((base) + (0x6E8UL))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_H_1_ADDR(base) ((base) + (0x6ECUL))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_0_ADDR(base) ((base) + (0xA00UL))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_1_ADDR(base) ((base) + (0xA04UL))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_2_ADDR(base) ((base) + (0xA08UL))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_3_ADDR(base) ((base) + (0xA0CUL))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_4_ADDR(base) ((base) + (0xA10UL))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_5_ADDR(base) ((base) + (0xA14UL))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_6_ADDR(base) ((base) + (0xA18UL))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_7_ADDR(base) ((base) + (0xA1CUL))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_8_ADDR(base) ((base) + (0xA20UL))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_9_ADDR(base) ((base) + (0xA24UL))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_10_ADDR(base) ((base) + (0xA28UL))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_11_ADDR(base) ((base) + (0xA2CUL))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_12_ADDR(base) ((base) + (0xA30UL))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_13_ADDR(base) ((base) + (0xA34UL))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_14_ADDR(base) ((base) + (0xA38UL))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_15_ADDR(base) ((base) + (0xA3CUL))
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_ADDR(base) ((base) + (0x000UL))
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_ADDR(base) ((base) + (0x004UL))
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_ADDR(base) ((base) + (0x008UL))
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_ADDR(base) ((base) + (0x00CUL))
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_ADDR(base) ((base) + (0x010UL))
#define SOC_PMCTRL_PERI_PMU_TRIM_UP_TIME_ADDR(base) ((base) + (0xA54UL))
#define SOC_PMCTRL_PERI_PMU_TRIM_DN_TTME_ADDR(base) ((base) + (0xA58UL))
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_ADDR(base) ((base) + (0x014UL))
#define SOC_PMCTRL_PERI_VOL_STABLE_ADDR(base) ((base) + (0x018UL))
#define SOC_PMCTRL_PERI_INIT_VOL_CONFIG_ADDR(base) ((base) + (0xA64UL))
#define SOC_PMCTRL_PERI_FIX_VOL_CONFIG_ADDR(base) ((base) + (0xA68UL))
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_ADDR(base) ((base) + (0x01CUL))
#define SOC_PMCTRL_PERI_VOL_READ_ADDR(base) ((base) + (0xA70UL))
#define SOC_PMCTRL_PERI_DVS_OUTPUT_READ_ADDR(base) ((base) + (0x020UL))
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_0_ADDR(base) ((base) + (0xA78UL))
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_1_ADDR(base) ((base) + (0xA7CUL))
#define SOC_PMCTRL_PERI_VOTE_SFT_HRD_VOL_IDX_ADDR(base) ((base) + (0xA80UL))
#define SOC_PMCTRL_PERI_STAT_TIME_HOLD_ADDR(base) ((base) + (0xA84UL))
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_ADDR(base) ((base) + (0x024UL))
#define SOC_PMCTRL_PERI_INTR_BYPASS_HRD_ADDR(base) ((base) + (0x028UL))
#define SOC_PMCTRL_PERI_VOL_CHG_FLAG_ADDR(base) ((base) + (0xA90UL))
#define SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_ADDR(base) ((base) + (0xA94UL))
#else
#define SOC_PMCTRL_APLL_CTRL_SRC_ADDR(base) ((base) + (0x100))
#define SOC_PMCTRL_SUBSYS_VOL_FREQ_CTRL_SRC_ADDR(base) ((base) + (0x104))
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_ADDR(base) ((base) + (0x108))
#define SOC_PMCTRL_TIME_PARAMETER_CFG_0_ADDR(base) ((base) + (0x10C))
#define SOC_PMCTRL_TIME_PARAMETER_CFG_1_ADDR(base) ((base) + (0x110))
#define SOC_PMCTRL_CPUL_DVFS_CLK_STAT_ADDR(base) ((base) + (0x150))
#define SOC_PMCTRL_CPUM_DVFS_CLK_STAT_ADDR(base) ((base) + (0x154))
#define SOC_PMCTRL_CPUB_DVFS_CLK_STAT_ADDR(base) ((base) + (0x158))
#define SOC_PMCTRL_FCM_DVFS_CLK_STAT_ADDR(base) ((base) + (0x15C))
#define SOC_PMCTRL_GPU_DVFS_CLK_STAT_ADDR(base) ((base) + (0x160))
#define SOC_PMCTRL_GPU_TOP_DVFS_CLK_STAT_ADDR(base) ((base) + (0x164))
#define SOC_PMCTRL_CPUL_DVFS_VOL_STAT_ADDR(base) ((base) + (0x170))
#define SOC_PMCTRL_CPUM_DVFS_VOL_STAT_ADDR(base) ((base) + (0x174))
#define SOC_PMCTRL_CPUB_DVFS_VOL_STAT_ADDR(base) ((base) + (0x178))
#define SOC_PMCTRL_FCM_DVFS_VOL_STAT_ADDR(base) ((base) + (0x17C))
#define SOC_PMCTRL_GPU_CORE_DVFS_VOL_STAT_ADDR(base) ((base) + (0x180))
#define SOC_PMCTRL_GPU_TOP_DVFS_VOL_STAT_ADDR(base) ((base) + (0x184))
#define SOC_PMCTRL_GPU_HPMCLKDIV_ADDR(base) ((base) + (0x120))
#define SOC_PMCTRL_GPU_HPMSEL_ADDR(base) ((base) + (0x124))
#define SOC_PMCTRL_GPU_HPMEN_ADDR(base) ((base) + (0x128))
#define SOC_PMCTRL_GPU_HPMOPC_ADDR(base) ((base) + (0x12C))
#define SOC_PMCTRL_PERIBAKDATA0_ADDR(base) ((base) + (0x130))
#define SOC_PMCTRL_PERIBAKDATA1_ADDR(base) ((base) + (0x134))
#define SOC_PMCTRL_PERIBAKDATA2_ADDR(base) ((base) + (0x138))
#define SOC_PMCTRL_PERIBAKDATA3_ADDR(base) ((base) + (0x13C))
#define SOC_PMCTRL_SPMIVOLCFG_ADDR(base) ((base) + (0x114))
#define SOC_PMCTRL_SPMIVOLCFG1_ADDR(base) ((base) + (0x118))
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_ADDR(base) ((base) + (0x1D8))
#define SOC_PMCTRL_FREQ_VOTE_A0_ADDR(base) ((base) + (0x200))
#define SOC_PMCTRL_FREQ_VOTE_A1_ADDR(base) ((base) + (0x204))
#define SOC_PMCTRL_FREQ_VOTE_B0_ADDR(base) ((base) + (0x208))
#define SOC_PMCTRL_FREQ_VOTE_B1_ADDR(base) ((base) + (0x20C))
#define SOC_PMCTRL_FREQ_VOTE_C0_ADDR(base) ((base) + (0x210))
#define SOC_PMCTRL_FREQ_VOTE_C1_ADDR(base) ((base) + (0x214))
#define SOC_PMCTRL_FREQ_VOTE_D0_ADDR(base) ((base) + (0x218))
#define SOC_PMCTRL_FREQ_VOTE_D1_ADDR(base) ((base) + (0x21C))
#define SOC_PMCTRL_FREQ_VOTE_E0_ADDR(base) ((base) + (0x220))
#define SOC_PMCTRL_FREQ_VOTE_E1_ADDR(base) ((base) + (0x224))
#define SOC_PMCTRL_FREQ_VOTE_E2_ADDR(base) ((base) + (0x228))
#define SOC_PMCTRL_FREQ_VOTE_E3_ADDR(base) ((base) + (0x22C))
#define SOC_PMCTRL_FREQ_VOTE_F0_ADDR(base) ((base) + (0x230))
#define SOC_PMCTRL_FREQ_VOTE_F1_ADDR(base) ((base) + (0x234))
#define SOC_PMCTRL_FREQ_VOTE_F2_ADDR(base) ((base) + (0x238))
#define SOC_PMCTRL_FREQ_VOTE_F3_ADDR(base) ((base) + (0x23C))
#define SOC_PMCTRL_FREQ_VOTE_G0_ADDR(base) ((base) + (0x240))
#define SOC_PMCTRL_FREQ_VOTE_G1_ADDR(base) ((base) + (0x244))
#define SOC_PMCTRL_FREQ_VOTE_G2_ADDR(base) ((base) + (0x248))
#define SOC_PMCTRL_FREQ_VOTE_G3_ADDR(base) ((base) + (0x24C))
#define SOC_PMCTRL_FREQ_VOTE_H0_ADDR(base) ((base) + (0x250))
#define SOC_PMCTRL_FREQ_VOTE_H1_ADDR(base) ((base) + (0x254))
#define SOC_PMCTRL_FREQ_VOTE_H2_ADDR(base) ((base) + (0x258))
#define SOC_PMCTRL_FREQ_VOTE_H3_ADDR(base) ((base) + (0x25C))
#define SOC_PMCTRL_FREQ_VOTE_H4_ADDR(base) ((base) + (0x260))
#define SOC_PMCTRL_FREQ_VOTE_H5_ADDR(base) ((base) + (0x264))
#define SOC_PMCTRL_FREQ_VOTE_H6_ADDR(base) ((base) + (0x268))
#define SOC_PMCTRL_FREQ_VOTE_H7_ADDR(base) ((base) + (0x26C))
#define SOC_PMCTRL_FREQ_VOTE_I0_ADDR(base) ((base) + (0x270))
#define SOC_PMCTRL_FREQ_VOTE_I1_ADDR(base) ((base) + (0x274))
#define SOC_PMCTRL_FREQ_VOTE_I2_ADDR(base) ((base) + (0x278))
#define SOC_PMCTRL_FREQ_VOTE_I3_ADDR(base) ((base) + (0x27C))
#define SOC_PMCTRL_FREQ_VOTE_I4_ADDR(base) ((base) + (0x280))
#define SOC_PMCTRL_FREQ_VOTE_I5_ADDR(base) ((base) + (0x284))
#define SOC_PMCTRL_FREQ_VOTE_I6_ADDR(base) ((base) + (0x288))
#define SOC_PMCTRL_FREQ_VOTE_I7_ADDR(base) ((base) + (0x28C))
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_ADDR(base) ((base) + (0x2B0))
#define SOC_PMCTRL_VOTE_MODE_CFG_0_ADDR(base) ((base) + (0x2BC))
#define SOC_PMCTRL_VOTE_MODE_CFG_1_ADDR(base) ((base) + (0x2C0))
#define SOC_PMCTRL_VOTE_START_EN_ADDR(base) ((base) + (0x2C4))
#define SOC_PMCTRL_INT_VOTE_EN_ADDR(base) ((base) + (0x2C8))
#define SOC_PMCTRL_INT_VOTE_CLR_ADDR(base) ((base) + (0x2CC))
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_ADDR(base) ((base) + (0x2D0))
#define SOC_PMCTRL_INT_VOTE_STAT_ADDR(base) ((base) + (0x2D4))
#define SOC_PMCTRL_INT_FREQUENCY_VOTE_ADDR(base) ((base) + (0x2D8))
#define SOC_PMCTRL_VOTE_RESULT_A_ADDR(base) ((base) + (0x2DC))
#define SOC_PMCTRL_VOTE_RESULT_B_ADDR(base) ((base) + (0x2E0))
#define SOC_PMCTRL_VOTE_RESULT_C_ADDR(base) ((base) + (0x2E4))
#define SOC_PMCTRL_VOTE_RESULT_D_ADDR(base) ((base) + (0x2E8))
#define SOC_PMCTRL_VOTE_RESULT_E_ADDR(base) ((base) + (0x2EC))
#define SOC_PMCTRL_VOTE_RESULT_F_ADDR(base) ((base) + (0x2F0))
#define SOC_PMCTRL_VOTE_RESULT_G_ADDR(base) ((base) + (0x2F4))
#define SOC_PMCTRL_VOTE_RESULT_H_ADDR(base) ((base) + (0x2F8))
#define SOC_PMCTRL_VOTE_RESULT_I_ADDR(base) ((base) + (0x2FC))
#define SOC_PMCTRL_DDRLPCTRL_ADDR(base) ((base) + (0x30C))
#define SOC_PMCTRL_PERI_CTRL0_ADDR(base) ((base) + (0x340))
#define SOC_PMCTRL_PERI_CTRL1_ADDR(base) ((base) + (0x344))
#define SOC_PMCTRL_PERI_CTRL2_ADDR(base) ((base) + (0x348))
#define SOC_PMCTRL_PERI_CTRL3_ADDR(base) ((base) + (0x34C))
#define SOC_PMCTRL_PERI_CTRL4_ADDR(base) ((base) + (0x350))
#define SOC_PMCTRL_PERI_CTRL5_ADDR(base) ((base) + (0x354))
#define SOC_PMCTRL_PERI_CTRL6_ADDR(base) ((base) + (0x358))
#define SOC_PMCTRL_PERI_CTRL7_ADDR(base) ((base) + (0x35C))
#define SOC_PMCTRL_PERI_STAT0_ADDR(base) ((base) + (0x360))
#define SOC_PMCTRL_PERI_STAT1_ADDR(base) ((base) + (0x364))
#define SOC_PMCTRL_PERI_STAT2_ADDR(base) ((base) + (0x368))
#define SOC_PMCTRL_PERI_STAT3_ADDR(base) ((base) + (0x36C))
#define SOC_PMCTRL_PERI_STAT4_ADDR(base) ((base) + (0x370))
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_ADDR(base) ((base) + (0x380))
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_ADDR(base) ((base) + (0x384))
#define SOC_PMCTRL_NOC_POWER_IDLE_0_ADDR(base) ((base) + (0x388))
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_ADDR(base) ((base) + (0x38C))
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_ADDR(base) ((base) + (0x390))
#define SOC_PMCTRL_NOC_POWER_IDLE_1_ADDR(base) ((base) + (0x394))
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_2_ADDR(base) ((base) + (0x398))
#define SOC_PMCTRL_NOC_POWER_IDLEACK_2_ADDR(base) ((base) + (0x39C))
#define SOC_PMCTRL_NOC_POWER_IDLE_2_ADDR(base) ((base) + (0x374))
#define SOC_PMCTRL_PERI_INT0_MASK_ADDR(base) ((base) + (0x3A0))
#define SOC_PMCTRL_PERI_INT0_STAT_ADDR(base) ((base) + (0x3A4))
#define SOC_PMCTRL_PERI_INT1_MASK_ADDR(base) ((base) + (0x3A8))
#define SOC_PMCTRL_PERI_INT1_STAT_ADDR(base) ((base) + (0x3AC))
#define SOC_PMCTRL_PERI_INT2_MASK_ADDR(base) ((base) + (0x3B0))
#define SOC_PMCTRL_PERI_INT2_STAT_ADDR(base) ((base) + (0x3B4))
#define SOC_PMCTRL_PERI_INT3_MASK_ADDR(base) ((base) + (0x3B8))
#define SOC_PMCTRL_PERI_INT3_STAT_ADDR(base) ((base) + (0x3BC))
#define SOC_PMCTRL_VS_CTRL_EN_GPU_ADDR(base) ((base) + (0x440))
#define SOC_PMCTRL_VS_CTRL_BYPASS_GPU_ADDR(base) ((base) + (0x444))
#define SOC_PMCTRL_VS_CTRL_GPU_ADDR(base) ((base) + (0x448))
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_ADDR(base) ((base) + (0x44C))
#define SOC_PMCTRL_VS_SVFD_CTRL1_GPU_ADDR(base) ((base) + (0x450))
#define SOC_PMCTRL_VS_SVFD_CTRL2_GPU_ADDR(base) ((base) + (0x454))
#define SOC_PMCTRL_VS_CPM_DATA_STAT_GPU_ADDR(base) ((base) + (0x458))
#define SOC_PMCTRL_VS_TEST_STAT_GPU_ADDR(base) ((base) + (0x45C))
#define SOC_PMCTRL_BOOTROMFLAG_ADDR(base) ((base) + (0x460))
#define SOC_PMCTRL_PERIHPMEN_ADDR(base) ((base) + (0x464))
#define SOC_PMCTRL_PERIHPMCLKDIV_ADDR(base) ((base) + (0x468))
#define SOC_PMCTRL_PERIHPMOPC_ADDR(base) ((base) + (0x46C))
#define SOC_PMCTRL_PERI_STAT8_ADDR(base) ((base) + (0x4D0))
#define SOC_PMCTRL_PERI_STAT9_ADDR(base) ((base) + (0x4D4))
#define SOC_PMCTRL_PERI_STAT10_ADDR(base) ((base) + (0x4D8))
#define SOC_PMCTRL_PERI_STAT11_ADDR(base) ((base) + (0x4DC))
#define SOC_PMCTRL_PERI_STAT12_ADDR(base) ((base) + (0x4E0))
#define SOC_PMCTRL_PERI_STAT13_ADDR(base) ((base) + (0x4E4))
#define SOC_PMCTRL_PERI_STAT14_ADDR(base) ((base) + (0x4E8))
#define SOC_PMCTRL_PERI_STAT15_ADDR(base) ((base) + (0x4EC))
#define SOC_PMCTRL_APLL0CTRL0_ADDR(base) ((base) + (0x500))
#define SOC_PMCTRL_APLL0CTRL1_ADDR(base) ((base) + (0x504))
#define SOC_PMCTRL_APLL0CTRL0_STAT_ADDR(base) ((base) + (0x508))
#define SOC_PMCTRL_APLL0CTRL1_STAT_ADDR(base) ((base) + (0x50C))
#define SOC_PMCTRL_APLL1CTRL0_ADDR(base) ((base) + (0x510))
#define SOC_PMCTRL_APLL1CTRL1_ADDR(base) ((base) + (0x514))
#define SOC_PMCTRL_APLL1CTRL0_STAT_ADDR(base) ((base) + (0x518))
#define SOC_PMCTRL_APLL1CTRL1_STAT_ADDR(base) ((base) + (0x51C))
#define SOC_PMCTRL_APLL2CTRL0_ADDR(base) ((base) + (0x520))
#define SOC_PMCTRL_APLL2CTRL1_ADDR(base) ((base) + (0x524))
#define SOC_PMCTRL_APLL2CTRL0_STAT_ADDR(base) ((base) + (0x528))
#define SOC_PMCTRL_APLL2CTRL1_STAT_ADDR(base) ((base) + (0x52C))
#define SOC_PMCTRL_APLL3CTRL0_ADDR(base) ((base) + (0x530))
#define SOC_PMCTRL_APLL3CTRL1_ADDR(base) ((base) + (0x534))
#define SOC_PMCTRL_APLL3CTRL0_STAT_ADDR(base) ((base) + (0x538))
#define SOC_PMCTRL_APLL3CTRL1_STAT_ADDR(base) ((base) + (0x53C))
#define SOC_PMCTRL_APLL4CTRL0_ADDR(base) ((base) + (0x540))
#define SOC_PMCTRL_APLL4CTRL1_ADDR(base) ((base) + (0x544))
#define SOC_PMCTRL_APLL4CTRL0_STAT_ADDR(base) ((base) + (0x548))
#define SOC_PMCTRL_APLL4CTRL1_STAT_ADDR(base) ((base) + (0x54C))
#define SOC_PMCTRL_APLL5CTRL0_ADDR(base) ((base) + (0x550))
#define SOC_PMCTRL_APLL5CTRL1_ADDR(base) ((base) + (0x554))
#define SOC_PMCTRL_APLL5CTRL0_STAT_ADDR(base) ((base) + (0x558))
#define SOC_PMCTRL_APLL5CTRL1_STAT_ADDR(base) ((base) + (0x55C))
#define SOC_PMCTRL_APLL6CTRL0_ADDR(base) ((base) + (0x560))
#define SOC_PMCTRL_APLL6CTRL1_ADDR(base) ((base) + (0x564))
#define SOC_PMCTRL_APLL6CTRL0_STAT_ADDR(base) ((base) + (0x568))
#define SOC_PMCTRL_APLL6CTRL1_STAT_ADDR(base) ((base) + (0x56C))
#define SOC_PMCTRL_APLL7CTRL0_ADDR(base) ((base) + (0x570))
#define SOC_PMCTRL_APLL7CTRL1_ADDR(base) ((base) + (0x574))
#define SOC_PMCTRL_APLL7CTRL0_STAT_ADDR(base) ((base) + (0x578))
#define SOC_PMCTRL_APLL7CTRL1_STAT_ADDR(base) ((base) + (0x57C))
#define SOC_PMCTRL_APLL0_SW_INITIAL0_ADDR(base) ((base) + (0x5E0))
#define SOC_PMCTRL_APLL0_SW_INITIAL1_ADDR(base) ((base) + (0x5E4))
#define SOC_PMCTRL_APLL1_SW_INITIAL0_ADDR(base) ((base) + (0x5E8))
#define SOC_PMCTRL_APLL1_SW_INITIAL1_ADDR(base) ((base) + (0x5EC))
#define SOC_PMCTRL_APLL2_SW_INITIAL0_ADDR(base) ((base) + (0x5F0))
#define SOC_PMCTRL_APLL2_SW_INITIAL1_ADDR(base) ((base) + (0x5F4))
#define SOC_PMCTRL_APLL3_SW_INITIAL0_ADDR(base) ((base) + (0x5F8))
#define SOC_PMCTRL_APLL3_SW_INITIAL1_ADDR(base) ((base) + (0x5FC))
#define SOC_PMCTRL_APLL4_SW_INITIAL0_ADDR(base) ((base) + (0x600))
#define SOC_PMCTRL_APLL4_SW_INITIAL1_ADDR(base) ((base) + (0x604))
#define SOC_PMCTRL_APLL5_SW_INITIAL0_ADDR(base) ((base) + (0x608))
#define SOC_PMCTRL_APLL5_SW_INITIAL1_ADDR(base) ((base) + (0x60C))
#define SOC_PMCTRL_APLL6_SW_INITIAL0_ADDR(base) ((base) + (0x610))
#define SOC_PMCTRL_APLL6_SW_INITIAL1_ADDR(base) ((base) + (0x614))
#define SOC_PMCTRL_APLL7_SW_INITIAL0_ADDR(base) ((base) + (0x618))
#define SOC_PMCTRL_APLL7_SW_INITIAL1_ADDR(base) ((base) + (0x61C))
#define SOC_PMCTRL_PDVFS_INSTR_LIST_A_ADDR(base,ma) ((base) + (0xC00 + (ma)*4))
#define SOC_PMCTRL_PDVFS_INSTR_LIST_B_ADDR(base,mb) ((base) + (0xC80 + (mb)*4))
#define SOC_PMCTRL_PDVFS_INSTR_LIST_C_ADDR(base,mc) ((base) + (0xD00 + (mc)*4))
#define SOC_PMCTRL_PDVFS_INSTR_LIST_D_ADDR(base,md) ((base) + (0xD80 + (md)*4))
#define SOC_PMCTRL_PDVFS_INSTR_LIST_E_ADDR(base,me) ((base) + (0xE00 + (me)*4))
#define SOC_PMCTRL_PDVFS_INSTR_LIST_F_ADDR(base,mf) ((base) + (0xE80 + (mf)*4))
#define SOC_PMCTRL_INSTR_ADDR_CTRL_A_ADDR(base) ((base) + (0x700))
#define SOC_PMCTRL_INSTR_ADDR_CTRL_B_ADDR(base) ((base) + (0x704))
#define SOC_PMCTRL_INSTR_ADDR_CTRL_C_ADDR(base) ((base) + (0x708))
#define SOC_PMCTRL_INSTR_ADDR_CTRL_D_ADDR(base) ((base) + (0x70C))
#define SOC_PMCTRL_INSTR_ADDR_CTRL_E_ADDR(base) ((base) + (0x710))
#define SOC_PMCTRL_INSTR_ADDR_CTRL_F_ADDR(base) ((base) + (0x714))
#define SOC_PMCTRL_INSTR_ADDR_CTRL_G_ADDR(base) ((base) + (0x718))
#define SOC_PMCTRL_INSTR_ADDR_CTRL_H_ADDR(base) ((base) + (0x71C))
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_ADDR(base) ((base) + (0x720))
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_ADDR(base) ((base) + (0x724))
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_ADDR(base) ((base) + (0x730))
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_ADDR(base) ((base) + (0x734))
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_ADDR(base) ((base) + (0x738))
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_ADDR(base) ((base) + (0x73C))
#define SOC_PMCTRL_TASKCTRL_A_0_ADDR(base) ((base) + (0x740))
#define SOC_PMCTRL_TASKCTRL_A_1_ADDR(base) ((base) + (0x744))
#define SOC_PMCTRL_TASKCTRL_A_2_ADDR(base) ((base) + (0x748))
#define SOC_PMCTRL_TASKCTRL_A_3_ADDR(base) ((base) + (0x74C))
#define SOC_PMCTRL_TASKCTRL_B_0_ADDR(base) ((base) + (0x750))
#define SOC_PMCTRL_TASKCTRL_B_1_ADDR(base) ((base) + (0x754))
#define SOC_PMCTRL_TASKCTRL_B_2_ADDR(base) ((base) + (0x758))
#define SOC_PMCTRL_TASKCTRL_B_3_ADDR(base) ((base) + (0x75C))
#define SOC_PMCTRL_TASKCTRL_C_0_ADDR(base) ((base) + (0x760))
#define SOC_PMCTRL_TASKCTRL_C_1_ADDR(base) ((base) + (0x764))
#define SOC_PMCTRL_TASKCTRL_C_2_ADDR(base) ((base) + (0x768))
#define SOC_PMCTRL_TASKCTRL_C_3_ADDR(base) ((base) + (0x76C))
#define SOC_PMCTRL_TASKCTRL_D_0_ADDR(base) ((base) + (0x770))
#define SOC_PMCTRL_TASKCTRL_D_1_ADDR(base) ((base) + (0x774))
#define SOC_PMCTRL_TASKCTRL_D_2_ADDR(base) ((base) + (0x778))
#define SOC_PMCTRL_TASKCTRL_D_3_ADDR(base) ((base) + (0x77C))
#define SOC_PMCTRL_TASKCTRL_E_0_ADDR(base) ((base) + (0x780))
#define SOC_PMCTRL_TASKCTRL_E_1_ADDR(base) ((base) + (0x784))
#define SOC_PMCTRL_TASKCTRL_E_2_ADDR(base) ((base) + (0x788))
#define SOC_PMCTRL_TASKCTRL_E_3_ADDR(base) ((base) + (0x78C))
#define SOC_PMCTRL_TASKCTRL_F_0_ADDR(base) ((base) + (0x790))
#define SOC_PMCTRL_TASKCTRL_F_1_ADDR(base) ((base) + (0x794))
#define SOC_PMCTRL_TASKCTRL_F_2_ADDR(base) ((base) + (0x798))
#define SOC_PMCTRL_TASKCTRL_F_3_ADDR(base) ((base) + (0x79C))
#define SOC_PMCTRL_TASKCTRL_G_0_ADDR(base) ((base) + (0x7A0))
#define SOC_PMCTRL_TASKCTRL_G_1_ADDR(base) ((base) + (0x7A4))
#define SOC_PMCTRL_TASKCTRL_G_2_ADDR(base) ((base) + (0x7A8))
#define SOC_PMCTRL_TASKCTRL_G_3_ADDR(base) ((base) + (0x7AC))
#define SOC_PMCTRL_TASKCTRL_H_0_ADDR(base) ((base) + (0x7B0))
#define SOC_PMCTRL_TASKCTRL_H_1_ADDR(base) ((base) + (0x7B4))
#define SOC_PMCTRL_TASKCTRL_H_2_ADDR(base) ((base) + (0x7B8))
#define SOC_PMCTRL_TASKCTRL_H_3_ADDR(base) ((base) + (0x7BC))
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_ADDR(base) ((base) + (0x7C0))
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_ADDR(base) ((base) + (0x7C4))
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_ADDR(base) ((base) + (0x7C8))
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_ADDR(base) ((base) + (0x7CC))
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_ADDR(base) ((base) + (0x7D0))
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_ADDR(base) ((base) + (0x7D4))
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_ADDR(base) ((base) + (0x7D8))
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_ADDR(base) ((base) + (0x7DC))
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_ADDR(base) ((base) + (0x7E0))
#define SOC_PMCTRL_COFSM_VOL_SENDOUT_ADDR(base) ((base) + (0x7E4))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_ADDR(base) ((base) + (0x800))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_1_ADDR(base) ((base) + (0x804))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_ADDR(base) ((base) + (0x808))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_1_ADDR(base) ((base) + (0x80C))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_ADDR(base) ((base) + (0x810))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_1_ADDR(base) ((base) + (0x814))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_ADDR(base) ((base) + (0x818))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_1_ADDR(base) ((base) + (0x81C))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_ADDR(base) ((base) + (0x820))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_1_ADDR(base) ((base) + (0x824))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_ADDR(base) ((base) + (0x828))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_1_ADDR(base) ((base) + (0x82C))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_ADDR(base) ((base) + (0x830))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_1_ADDR(base) ((base) + (0x834))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_ADDR(base) ((base) + (0x838))
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_1_ADDR(base) ((base) + (0x83C))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_0_ADDR(base) ((base) + (0x840))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_ADDR(base) ((base) + (0x844))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_2_ADDR(base) ((base) + (0x848))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_3_ADDR(base) ((base) + (0x84C))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_0_ADDR(base) ((base) + (0x850))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_ADDR(base) ((base) + (0x854))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_ADDR(base) ((base) + (0x858))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_3_ADDR(base) ((base) + (0x85C))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_0_ADDR(base) ((base) + (0x860))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_ADDR(base) ((base) + (0x864))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_2_ADDR(base) ((base) + (0x868))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_3_ADDR(base) ((base) + (0x86C))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_0_ADDR(base) ((base) + (0x870))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_ADDR(base) ((base) + (0x874))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_2_ADDR(base) ((base) + (0x878))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_3_ADDR(base) ((base) + (0x87C))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_0_ADDR(base) ((base) + (0x880))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_ADDR(base) ((base) + (0x884))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_2_ADDR(base) ((base) + (0x888))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_3_ADDR(base) ((base) + (0x88C))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_0_ADDR(base) ((base) + (0x890))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_ADDR(base) ((base) + (0x894))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_ADDR(base) ((base) + (0x898))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_3_ADDR(base) ((base) + (0x89C))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_0_ADDR(base) ((base) + (0x8A0))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_ADDR(base) ((base) + (0x8A4))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_ADDR(base) ((base) + (0x8A8))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_3_ADDR(base) ((base) + (0x8AC))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_0_ADDR(base) ((base) + (0x8B0))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_ADDR(base) ((base) + (0x8B4))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_ADDR(base) ((base) + (0x8B8))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_3_ADDR(base) ((base) + (0x8BC))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_0_ADDR(base) ((base) + (0x8C0))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_ADDR(base) ((base) + (0x8C4))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_2_ADDR(base) ((base) + (0x8C8))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_3_ADDR(base) ((base) + (0x8CC))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_0_ADDR(base) ((base) + (0x8D0))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_ADDR(base) ((base) + (0x8D4))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_2_ADDR(base) ((base) + (0x8D8))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_3_ADDR(base) ((base) + (0x8DC))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_0_ADDR(base) ((base) + (0x8E0))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_ADDR(base) ((base) + (0x8E4))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_ADDR(base) ((base) + (0x8E8))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_3_ADDR(base) ((base) + (0x8EC))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_0_ADDR(base) ((base) + (0x8F0))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_ADDR(base) ((base) + (0x8F4))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_ADDR(base) ((base) + (0x8F8))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_3_ADDR(base) ((base) + (0x8FC))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_0_ADDR(base) ((base) + (0x9A0))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_ADDR(base) ((base) + (0x9A4))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_ADDR(base) ((base) + (0x9A8))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_3_ADDR(base) ((base) + (0x9AC))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_0_ADDR(base) ((base) + (0x9B0))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_ADDR(base) ((base) + (0x9B4))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_ADDR(base) ((base) + (0x9B8))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_3_ADDR(base) ((base) + (0x9BC))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_0_ADDR(base) ((base) + (0x9C0))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_ADDR(base) ((base) + (0x9C4))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_ADDR(base) ((base) + (0x9C8))
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_3_ADDR(base) ((base) + (0x9CC))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_0_ADDR(base) ((base) + (0x9D0))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_ADDR(base) ((base) + (0x9D4))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_ADDR(base) ((base) + (0x9D8))
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_3_ADDR(base) ((base) + (0x9DC))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_A_0_ADDR(base) ((base) + (0x670))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_A_1_ADDR(base) ((base) + (0x674))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_A_0_ADDR(base) ((base) + (0x678))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_A_1_ADDR(base) ((base) + (0x67C))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_B_0_ADDR(base) ((base) + (0x680))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_B_1_ADDR(base) ((base) + (0x684))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_B_0_ADDR(base) ((base) + (0x688))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_B_1_ADDR(base) ((base) + (0x68C))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_C_0_ADDR(base) ((base) + (0x690))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_C_1_ADDR(base) ((base) + (0x694))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_C_0_ADDR(base) ((base) + (0x698))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_C_1_ADDR(base) ((base) + (0x69C))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_D_0_ADDR(base) ((base) + (0x6A0))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_D_1_ADDR(base) ((base) + (0x6A4))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_D_0_ADDR(base) ((base) + (0x6A8))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_D_1_ADDR(base) ((base) + (0x6AC))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_E_0_ADDR(base) ((base) + (0x6B0))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_E_1_ADDR(base) ((base) + (0x6B4))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_E_0_ADDR(base) ((base) + (0x6B8))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_E_1_ADDR(base) ((base) + (0x6BC))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_F_0_ADDR(base) ((base) + (0x6C0))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_F_1_ADDR(base) ((base) + (0x6C4))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_F_0_ADDR(base) ((base) + (0x6C8))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_F_1_ADDR(base) ((base) + (0x6CC))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_G_0_ADDR(base) ((base) + (0x6D0))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_G_1_ADDR(base) ((base) + (0x6D4))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_G_0_ADDR(base) ((base) + (0x6D8))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_G_1_ADDR(base) ((base) + (0x6DC))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_H_0_ADDR(base) ((base) + (0x6E0))
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_H_1_ADDR(base) ((base) + (0x6E4))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_H_0_ADDR(base) ((base) + (0x6E8))
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_H_1_ADDR(base) ((base) + (0x6EC))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_0_ADDR(base) ((base) + (0xA00))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_1_ADDR(base) ((base) + (0xA04))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_2_ADDR(base) ((base) + (0xA08))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_3_ADDR(base) ((base) + (0xA0C))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_4_ADDR(base) ((base) + (0xA10))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_5_ADDR(base) ((base) + (0xA14))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_6_ADDR(base) ((base) + (0xA18))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_7_ADDR(base) ((base) + (0xA1C))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_8_ADDR(base) ((base) + (0xA20))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_9_ADDR(base) ((base) + (0xA24))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_10_ADDR(base) ((base) + (0xA28))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_11_ADDR(base) ((base) + (0xA2C))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_12_ADDR(base) ((base) + (0xA30))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_13_ADDR(base) ((base) + (0xA34))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_14_ADDR(base) ((base) + (0xA38))
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_15_ADDR(base) ((base) + (0xA3C))
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_ADDR(base) ((base) + (0x000))
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_ADDR(base) ((base) + (0x004))
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_ADDR(base) ((base) + (0x008))
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_ADDR(base) ((base) + (0x00C))
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_ADDR(base) ((base) + (0x010))
#define SOC_PMCTRL_PERI_PMU_TRIM_UP_TIME_ADDR(base) ((base) + (0xA54))
#define SOC_PMCTRL_PERI_PMU_TRIM_DN_TTME_ADDR(base) ((base) + (0xA58))
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_ADDR(base) ((base) + (0x014))
#define SOC_PMCTRL_PERI_VOL_STABLE_ADDR(base) ((base) + (0x018))
#define SOC_PMCTRL_PERI_INIT_VOL_CONFIG_ADDR(base) ((base) + (0xA64))
#define SOC_PMCTRL_PERI_FIX_VOL_CONFIG_ADDR(base) ((base) + (0xA68))
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_ADDR(base) ((base) + (0x01C))
#define SOC_PMCTRL_PERI_VOL_READ_ADDR(base) ((base) + (0xA70))
#define SOC_PMCTRL_PERI_DVS_OUTPUT_READ_ADDR(base) ((base) + (0x020))
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_0_ADDR(base) ((base) + (0xA78))
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_1_ADDR(base) ((base) + (0xA7C))
#define SOC_PMCTRL_PERI_VOTE_SFT_HRD_VOL_IDX_ADDR(base) ((base) + (0xA80))
#define SOC_PMCTRL_PERI_STAT_TIME_HOLD_ADDR(base) ((base) + (0xA84))
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_ADDR(base) ((base) + (0x024))
#define SOC_PMCTRL_PERI_INTR_BYPASS_HRD_ADDR(base) ((base) + (0x028))
#define SOC_PMCTRL_PERI_VOL_CHG_FLAG_ADDR(base) ((base) + (0xA90))
#define SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_ADDR(base) ((base) + (0xA94))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll0_ctrl_src : 4;
        unsigned int apll1_ctrl_src : 4;
        unsigned int apll2_ctrl_src : 4;
        unsigned int apll3_ctrl_src : 4;
        unsigned int apll4_ctrl_src : 4;
        unsigned int apll5_ctrl_src : 4;
        unsigned int apll6_ctrl_src : 4;
        unsigned int apll7_ctrl_src : 4;
    } reg;
} SOC_PMCTRL_APLL_CTRL_SRC_UNION;
#endif
#define SOC_PMCTRL_APLL_CTRL_SRC_apll0_ctrl_src_START (0)
#define SOC_PMCTRL_APLL_CTRL_SRC_apll0_ctrl_src_END (3)
#define SOC_PMCTRL_APLL_CTRL_SRC_apll1_ctrl_src_START (4)
#define SOC_PMCTRL_APLL_CTRL_SRC_apll1_ctrl_src_END (7)
#define SOC_PMCTRL_APLL_CTRL_SRC_apll2_ctrl_src_START (8)
#define SOC_PMCTRL_APLL_CTRL_SRC_apll2_ctrl_src_END (11)
#define SOC_PMCTRL_APLL_CTRL_SRC_apll3_ctrl_src_START (12)
#define SOC_PMCTRL_APLL_CTRL_SRC_apll3_ctrl_src_END (15)
#define SOC_PMCTRL_APLL_CTRL_SRC_apll4_ctrl_src_START (16)
#define SOC_PMCTRL_APLL_CTRL_SRC_apll4_ctrl_src_END (19)
#define SOC_PMCTRL_APLL_CTRL_SRC_apll5_ctrl_src_START (20)
#define SOC_PMCTRL_APLL_CTRL_SRC_apll5_ctrl_src_END (23)
#define SOC_PMCTRL_APLL_CTRL_SRC_apll6_ctrl_src_START (24)
#define SOC_PMCTRL_APLL_CTRL_SRC_apll6_ctrl_src_END (27)
#define SOC_PMCTRL_APLL_CTRL_SRC_apll7_ctrl_src_START (28)
#define SOC_PMCTRL_APLL_CTRL_SRC_apll7_ctrl_src_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpul_vol_freq_ctrl_src : 4;
        unsigned int cpum_vol_freq_ctrl_src : 4;
        unsigned int cpub_vol_freq_ctrl_src : 4;
        unsigned int fcm_vol_freq_ctrl_src : 4;
        unsigned int gpu_core_vol_freq_ctrl_src : 4;
        unsigned int gpu_top_vol_freq_ctrl_src : 4;
        unsigned int reserved_0 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_SUBSYS_VOL_FREQ_CTRL_SRC_UNION;
#endif
#define SOC_PMCTRL_SUBSYS_VOL_FREQ_CTRL_SRC_cpul_vol_freq_ctrl_src_START (0)
#define SOC_PMCTRL_SUBSYS_VOL_FREQ_CTRL_SRC_cpul_vol_freq_ctrl_src_END (3)
#define SOC_PMCTRL_SUBSYS_VOL_FREQ_CTRL_SRC_cpum_vol_freq_ctrl_src_START (4)
#define SOC_PMCTRL_SUBSYS_VOL_FREQ_CTRL_SRC_cpum_vol_freq_ctrl_src_END (7)
#define SOC_PMCTRL_SUBSYS_VOL_FREQ_CTRL_SRC_cpub_vol_freq_ctrl_src_START (8)
#define SOC_PMCTRL_SUBSYS_VOL_FREQ_CTRL_SRC_cpub_vol_freq_ctrl_src_END (11)
#define SOC_PMCTRL_SUBSYS_VOL_FREQ_CTRL_SRC_fcm_vol_freq_ctrl_src_START (12)
#define SOC_PMCTRL_SUBSYS_VOL_FREQ_CTRL_SRC_fcm_vol_freq_ctrl_src_END (15)
#define SOC_PMCTRL_SUBSYS_VOL_FREQ_CTRL_SRC_gpu_core_vol_freq_ctrl_src_START (16)
#define SOC_PMCTRL_SUBSYS_VOL_FREQ_CTRL_SRC_gpu_core_vol_freq_ctrl_src_END (19)
#define SOC_PMCTRL_SUBSYS_VOL_FREQ_CTRL_SRC_gpu_top_vol_freq_ctrl_src_START (20)
#define SOC_PMCTRL_SUBSYS_VOL_FREQ_CTRL_SRC_gpu_top_vol_freq_ctrl_src_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpul_mem_vol_channel_sel : 2;
        unsigned int cpul_logic_vol_channel_sel : 2;
        unsigned int cpum_mem_vol_channel_sel : 2;
        unsigned int cpum_logic_vol_channel_sel : 2;
        unsigned int cpub_mem_vol_channel_sel : 2;
        unsigned int cpub_logic_vol_channel_sel : 2;
        unsigned int fcm_mem_vol_channel_sel : 2;
        unsigned int fcm_logic_vol_channel_sel : 2;
        unsigned int gpu_core_mem_vol_channel_sel : 2;
        unsigned int gpu_core_logic_vol_channel_sel : 2;
        unsigned int gpu_top_mem_vol_channel_sel : 2;
        unsigned int gpu_top_logic_vol_channel_sel : 2;
        unsigned int reserved_0 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_UNION;
#endif
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_cpul_mem_vol_channel_sel_START (0)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_cpul_mem_vol_channel_sel_END (1)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_cpul_logic_vol_channel_sel_START (2)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_cpul_logic_vol_channel_sel_END (3)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_cpum_mem_vol_channel_sel_START (4)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_cpum_mem_vol_channel_sel_END (5)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_cpum_logic_vol_channel_sel_START (6)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_cpum_logic_vol_channel_sel_END (7)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_cpub_mem_vol_channel_sel_START (8)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_cpub_mem_vol_channel_sel_END (9)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_cpub_logic_vol_channel_sel_START (10)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_cpub_logic_vol_channel_sel_END (11)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_fcm_mem_vol_channel_sel_START (12)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_fcm_mem_vol_channel_sel_END (13)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_fcm_logic_vol_channel_sel_START (14)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_fcm_logic_vol_channel_sel_END (15)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_gpu_core_mem_vol_channel_sel_START (16)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_gpu_core_mem_vol_channel_sel_END (17)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_gpu_core_logic_vol_channel_sel_START (18)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_gpu_core_logic_vol_channel_sel_END (19)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_gpu_top_mem_vol_channel_sel_START (20)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_gpu_top_mem_vol_channel_sel_END (21)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_gpu_top_logic_vol_channel_sel_START (22)
#define SOC_PMCTRL_SUBSYS_PMU_CHANNEL_CTRL_gpu_top_logic_vol_channel_sel_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll_cfg_time : 8;
        unsigned int clk_cfg_wait_cycle : 8;
        unsigned int pclk_cycle_per_us : 10;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TIME_PARAMETER_CFG_0_UNION;
#endif
#define SOC_PMCTRL_TIME_PARAMETER_CFG_0_apll_cfg_time_START (0)
#define SOC_PMCTRL_TIME_PARAMETER_CFG_0_apll_cfg_time_END (7)
#define SOC_PMCTRL_TIME_PARAMETER_CFG_0_clk_cfg_wait_cycle_START (8)
#define SOC_PMCTRL_TIME_PARAMETER_CFG_0_clk_cfg_wait_cycle_END (15)
#define SOC_PMCTRL_TIME_PARAMETER_CFG_0_pclk_cycle_per_us_START (16)
#define SOC_PMCTRL_TIME_PARAMETER_CFG_0_pclk_cycle_per_us_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll_lock_time : 12;
        unsigned int reserved_0 : 4;
        unsigned int reserved_1 : 10;
        unsigned int reserved_2 : 2;
        unsigned int reserved_3 : 4;
    } reg;
} SOC_PMCTRL_TIME_PARAMETER_CFG_1_UNION;
#endif
#define SOC_PMCTRL_TIME_PARAMETER_CFG_1_apll_lock_time_START (0)
#define SOC_PMCTRL_TIME_PARAMETER_CFG_1_apll_lock_time_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpul_clk_div_cpu_stat : 16;
        unsigned int cpul_clk_div_vdm_stat : 2;
        unsigned int reserved_0 : 2;
        unsigned int cpul_clk_div_ppll0_stat : 4;
        unsigned int cpul_gt_clk_ppll0_gt_stat : 1;
        unsigned int reserved_1 : 3;
        unsigned int cpul_pll_sw_stat : 1;
        unsigned int cpul_pll_sw_ack_sync : 1;
        unsigned int reserved_2 : 2;
    } reg;
} SOC_PMCTRL_CPUL_DVFS_CLK_STAT_UNION;
#endif
#define SOC_PMCTRL_CPUL_DVFS_CLK_STAT_cpul_clk_div_cpu_stat_START (0)
#define SOC_PMCTRL_CPUL_DVFS_CLK_STAT_cpul_clk_div_cpu_stat_END (15)
#define SOC_PMCTRL_CPUL_DVFS_CLK_STAT_cpul_clk_div_vdm_stat_START (16)
#define SOC_PMCTRL_CPUL_DVFS_CLK_STAT_cpul_clk_div_vdm_stat_END (17)
#define SOC_PMCTRL_CPUL_DVFS_CLK_STAT_cpul_clk_div_ppll0_stat_START (20)
#define SOC_PMCTRL_CPUL_DVFS_CLK_STAT_cpul_clk_div_ppll0_stat_END (23)
#define SOC_PMCTRL_CPUL_DVFS_CLK_STAT_cpul_gt_clk_ppll0_gt_stat_START (24)
#define SOC_PMCTRL_CPUL_DVFS_CLK_STAT_cpul_gt_clk_ppll0_gt_stat_END (24)
#define SOC_PMCTRL_CPUL_DVFS_CLK_STAT_cpul_pll_sw_stat_START (28)
#define SOC_PMCTRL_CPUL_DVFS_CLK_STAT_cpul_pll_sw_stat_END (28)
#define SOC_PMCTRL_CPUL_DVFS_CLK_STAT_cpul_pll_sw_ack_sync_START (29)
#define SOC_PMCTRL_CPUL_DVFS_CLK_STAT_cpul_pll_sw_ack_sync_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpum_clk_div_cpu_stat : 16;
        unsigned int cpum_clk_div_vdm_stat : 2;
        unsigned int reserved_0 : 2;
        unsigned int cpum_clk_div_ppll0_stat : 4;
        unsigned int cpum_gt_clk_ppll0_gt_stat : 1;
        unsigned int reserved_1 : 3;
        unsigned int cpum_pll_sw_stat : 1;
        unsigned int cpum_pll_sw_ack_sync : 1;
        unsigned int reserved_2 : 2;
    } reg;
} SOC_PMCTRL_CPUM_DVFS_CLK_STAT_UNION;
#endif
#define SOC_PMCTRL_CPUM_DVFS_CLK_STAT_cpum_clk_div_cpu_stat_START (0)
#define SOC_PMCTRL_CPUM_DVFS_CLK_STAT_cpum_clk_div_cpu_stat_END (15)
#define SOC_PMCTRL_CPUM_DVFS_CLK_STAT_cpum_clk_div_vdm_stat_START (16)
#define SOC_PMCTRL_CPUM_DVFS_CLK_STAT_cpum_clk_div_vdm_stat_END (17)
#define SOC_PMCTRL_CPUM_DVFS_CLK_STAT_cpum_clk_div_ppll0_stat_START (20)
#define SOC_PMCTRL_CPUM_DVFS_CLK_STAT_cpum_clk_div_ppll0_stat_END (23)
#define SOC_PMCTRL_CPUM_DVFS_CLK_STAT_cpum_gt_clk_ppll0_gt_stat_START (24)
#define SOC_PMCTRL_CPUM_DVFS_CLK_STAT_cpum_gt_clk_ppll0_gt_stat_END (24)
#define SOC_PMCTRL_CPUM_DVFS_CLK_STAT_cpum_pll_sw_stat_START (28)
#define SOC_PMCTRL_CPUM_DVFS_CLK_STAT_cpum_pll_sw_stat_END (28)
#define SOC_PMCTRL_CPUM_DVFS_CLK_STAT_cpum_pll_sw_ack_sync_START (29)
#define SOC_PMCTRL_CPUM_DVFS_CLK_STAT_cpum_pll_sw_ack_sync_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpub_clk_div_cpu_stat : 16;
        unsigned int cpub_clk_div_vdm_stat : 2;
        unsigned int reserved_0 : 2;
        unsigned int cpub_clk_div_ppll0_stat : 4;
        unsigned int cpub_gt_clk_ppll0_gt_stat : 1;
        unsigned int reserved_1 : 3;
        unsigned int cpub_pll_sw_stat : 1;
        unsigned int cpub_pll_sw_ack_sync : 1;
        unsigned int reserved_2 : 2;
    } reg;
} SOC_PMCTRL_CPUB_DVFS_CLK_STAT_UNION;
#endif
#define SOC_PMCTRL_CPUB_DVFS_CLK_STAT_cpub_clk_div_cpu_stat_START (0)
#define SOC_PMCTRL_CPUB_DVFS_CLK_STAT_cpub_clk_div_cpu_stat_END (15)
#define SOC_PMCTRL_CPUB_DVFS_CLK_STAT_cpub_clk_div_vdm_stat_START (16)
#define SOC_PMCTRL_CPUB_DVFS_CLK_STAT_cpub_clk_div_vdm_stat_END (17)
#define SOC_PMCTRL_CPUB_DVFS_CLK_STAT_cpub_clk_div_ppll0_stat_START (20)
#define SOC_PMCTRL_CPUB_DVFS_CLK_STAT_cpub_clk_div_ppll0_stat_END (23)
#define SOC_PMCTRL_CPUB_DVFS_CLK_STAT_cpub_gt_clk_ppll0_gt_stat_START (24)
#define SOC_PMCTRL_CPUB_DVFS_CLK_STAT_cpub_gt_clk_ppll0_gt_stat_END (24)
#define SOC_PMCTRL_CPUB_DVFS_CLK_STAT_cpub_pll_sw_stat_START (28)
#define SOC_PMCTRL_CPUB_DVFS_CLK_STAT_cpub_pll_sw_stat_END (28)
#define SOC_PMCTRL_CPUB_DVFS_CLK_STAT_cpub_pll_sw_ack_sync_START (29)
#define SOC_PMCTRL_CPUB_DVFS_CLK_STAT_cpub_pll_sw_ack_sync_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fcm_clk_div_cpu_stat : 16;
        unsigned int fcm_clk_div_vdm_stat : 2;
        unsigned int reserved_0 : 2;
        unsigned int fcm_clk_div_ppll0_stat : 4;
        unsigned int fcm_gt_clk_ppll0_gt_stat : 1;
        unsigned int reserved_1 : 3;
        unsigned int fcm_pll_sw_stat : 1;
        unsigned int fcm_pll_sw_ack_sync : 1;
        unsigned int reserved_2 : 2;
    } reg;
} SOC_PMCTRL_FCM_DVFS_CLK_STAT_UNION;
#endif
#define SOC_PMCTRL_FCM_DVFS_CLK_STAT_fcm_clk_div_cpu_stat_START (0)
#define SOC_PMCTRL_FCM_DVFS_CLK_STAT_fcm_clk_div_cpu_stat_END (15)
#define SOC_PMCTRL_FCM_DVFS_CLK_STAT_fcm_clk_div_vdm_stat_START (16)
#define SOC_PMCTRL_FCM_DVFS_CLK_STAT_fcm_clk_div_vdm_stat_END (17)
#define SOC_PMCTRL_FCM_DVFS_CLK_STAT_fcm_clk_div_ppll0_stat_START (20)
#define SOC_PMCTRL_FCM_DVFS_CLK_STAT_fcm_clk_div_ppll0_stat_END (23)
#define SOC_PMCTRL_FCM_DVFS_CLK_STAT_fcm_gt_clk_ppll0_gt_stat_START (24)
#define SOC_PMCTRL_FCM_DVFS_CLK_STAT_fcm_gt_clk_ppll0_gt_stat_END (24)
#define SOC_PMCTRL_FCM_DVFS_CLK_STAT_fcm_pll_sw_stat_START (28)
#define SOC_PMCTRL_FCM_DVFS_CLK_STAT_fcm_pll_sw_stat_END (28)
#define SOC_PMCTRL_FCM_DVFS_CLK_STAT_fcm_pll_sw_ack_sync_START (29)
#define SOC_PMCTRL_FCM_DVFS_CLK_STAT_fcm_pll_sw_ack_sync_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpu_clk_div_cpu_stat : 16;
        unsigned int gpu_clk_div_vdm_stat : 2;
        unsigned int reserved_0 : 2;
        unsigned int gpu_clk_div_ppll0_stat : 4;
        unsigned int gpu_gt_clk_ppll0_gt_stat : 1;
        unsigned int reserved_1 : 3;
        unsigned int gpu_core_pll_sw_stat : 1;
        unsigned int gpu_core_pll_sw_ack_sync : 1;
        unsigned int reserved_2 : 2;
    } reg;
} SOC_PMCTRL_GPU_DVFS_CLK_STAT_UNION;
#endif
#define SOC_PMCTRL_GPU_DVFS_CLK_STAT_gpu_clk_div_cpu_stat_START (0)
#define SOC_PMCTRL_GPU_DVFS_CLK_STAT_gpu_clk_div_cpu_stat_END (15)
#define SOC_PMCTRL_GPU_DVFS_CLK_STAT_gpu_clk_div_vdm_stat_START (16)
#define SOC_PMCTRL_GPU_DVFS_CLK_STAT_gpu_clk_div_vdm_stat_END (17)
#define SOC_PMCTRL_GPU_DVFS_CLK_STAT_gpu_clk_div_ppll0_stat_START (20)
#define SOC_PMCTRL_GPU_DVFS_CLK_STAT_gpu_clk_div_ppll0_stat_END (23)
#define SOC_PMCTRL_GPU_DVFS_CLK_STAT_gpu_gt_clk_ppll0_gt_stat_START (24)
#define SOC_PMCTRL_GPU_DVFS_CLK_STAT_gpu_gt_clk_ppll0_gt_stat_END (24)
#define SOC_PMCTRL_GPU_DVFS_CLK_STAT_gpu_core_pll_sw_stat_START (28)
#define SOC_PMCTRL_GPU_DVFS_CLK_STAT_gpu_core_pll_sw_stat_END (28)
#define SOC_PMCTRL_GPU_DVFS_CLK_STAT_gpu_core_pll_sw_ack_sync_START (29)
#define SOC_PMCTRL_GPU_DVFS_CLK_STAT_gpu_core_pll_sw_ack_sync_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpu_top_clk_div_cpu_stat : 16;
        unsigned int gpu_top_clk_div_vdm_stat : 2;
        unsigned int reserved_0 : 2;
        unsigned int gpu_top_clk_div_ppll0_stat : 4;
        unsigned int gpu_top_gt_clk_ppll0_gt_stat : 1;
        unsigned int reserved_1 : 3;
        unsigned int gpu_top_pll_sw_stat : 1;
        unsigned int gpu_top_pll_sw_ack_sync : 1;
        unsigned int reserved_2 : 2;
    } reg;
} SOC_PMCTRL_GPU_TOP_DVFS_CLK_STAT_UNION;
#endif
#define SOC_PMCTRL_GPU_TOP_DVFS_CLK_STAT_gpu_top_clk_div_cpu_stat_START (0)
#define SOC_PMCTRL_GPU_TOP_DVFS_CLK_STAT_gpu_top_clk_div_cpu_stat_END (15)
#define SOC_PMCTRL_GPU_TOP_DVFS_CLK_STAT_gpu_top_clk_div_vdm_stat_START (16)
#define SOC_PMCTRL_GPU_TOP_DVFS_CLK_STAT_gpu_top_clk_div_vdm_stat_END (17)
#define SOC_PMCTRL_GPU_TOP_DVFS_CLK_STAT_gpu_top_clk_div_ppll0_stat_START (20)
#define SOC_PMCTRL_GPU_TOP_DVFS_CLK_STAT_gpu_top_clk_div_ppll0_stat_END (23)
#define SOC_PMCTRL_GPU_TOP_DVFS_CLK_STAT_gpu_top_gt_clk_ppll0_gt_stat_START (24)
#define SOC_PMCTRL_GPU_TOP_DVFS_CLK_STAT_gpu_top_gt_clk_ppll0_gt_stat_END (24)
#define SOC_PMCTRL_GPU_TOP_DVFS_CLK_STAT_gpu_top_pll_sw_stat_START (28)
#define SOC_PMCTRL_GPU_TOP_DVFS_CLK_STAT_gpu_top_pll_sw_stat_END (28)
#define SOC_PMCTRL_GPU_TOP_DVFS_CLK_STAT_gpu_top_pll_sw_ack_sync_START (29)
#define SOC_PMCTRL_GPU_TOP_DVFS_CLK_STAT_gpu_top_pll_sw_ack_sync_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpul_pmu_mem_vol_idx : 8;
        unsigned int cpul_pmu_mem_vol_chg : 1;
        unsigned int reserved_0 : 3;
        unsigned int cpul_pmu_logic_vol_idx : 8;
        unsigned int cpul_pmu_logic_vol_chg : 1;
        unsigned int reserved_1 : 3;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_CPUL_DVFS_VOL_STAT_UNION;
#endif
#define SOC_PMCTRL_CPUL_DVFS_VOL_STAT_cpul_pmu_mem_vol_idx_START (0)
#define SOC_PMCTRL_CPUL_DVFS_VOL_STAT_cpul_pmu_mem_vol_idx_END (7)
#define SOC_PMCTRL_CPUL_DVFS_VOL_STAT_cpul_pmu_mem_vol_chg_START (8)
#define SOC_PMCTRL_CPUL_DVFS_VOL_STAT_cpul_pmu_mem_vol_chg_END (8)
#define SOC_PMCTRL_CPUL_DVFS_VOL_STAT_cpul_pmu_logic_vol_idx_START (12)
#define SOC_PMCTRL_CPUL_DVFS_VOL_STAT_cpul_pmu_logic_vol_idx_END (19)
#define SOC_PMCTRL_CPUL_DVFS_VOL_STAT_cpul_pmu_logic_vol_chg_START (20)
#define SOC_PMCTRL_CPUL_DVFS_VOL_STAT_cpul_pmu_logic_vol_chg_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpum_pmu_mem_vol_idx : 8;
        unsigned int cpum_pmu_mem_vol_chg : 1;
        unsigned int reserved_0 : 3;
        unsigned int cpum_pmu_logic_vol_idx : 8;
        unsigned int cpum_pmu_logic_vol_chg : 1;
        unsigned int reserved_1 : 3;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_CPUM_DVFS_VOL_STAT_UNION;
#endif
#define SOC_PMCTRL_CPUM_DVFS_VOL_STAT_cpum_pmu_mem_vol_idx_START (0)
#define SOC_PMCTRL_CPUM_DVFS_VOL_STAT_cpum_pmu_mem_vol_idx_END (7)
#define SOC_PMCTRL_CPUM_DVFS_VOL_STAT_cpum_pmu_mem_vol_chg_START (8)
#define SOC_PMCTRL_CPUM_DVFS_VOL_STAT_cpum_pmu_mem_vol_chg_END (8)
#define SOC_PMCTRL_CPUM_DVFS_VOL_STAT_cpum_pmu_logic_vol_idx_START (12)
#define SOC_PMCTRL_CPUM_DVFS_VOL_STAT_cpum_pmu_logic_vol_idx_END (19)
#define SOC_PMCTRL_CPUM_DVFS_VOL_STAT_cpum_pmu_logic_vol_chg_START (20)
#define SOC_PMCTRL_CPUM_DVFS_VOL_STAT_cpum_pmu_logic_vol_chg_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpub_pmu_mem_vol_idx : 8;
        unsigned int cpub_pmu_mem_vol_chg : 1;
        unsigned int reserved_0 : 3;
        unsigned int cpub_pmu_logic_vol_idx : 8;
        unsigned int cpub_pmu_logic_vol_chg : 1;
        unsigned int reserved_1 : 3;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_CPUB_DVFS_VOL_STAT_UNION;
#endif
#define SOC_PMCTRL_CPUB_DVFS_VOL_STAT_cpub_pmu_mem_vol_idx_START (0)
#define SOC_PMCTRL_CPUB_DVFS_VOL_STAT_cpub_pmu_mem_vol_idx_END (7)
#define SOC_PMCTRL_CPUB_DVFS_VOL_STAT_cpub_pmu_mem_vol_chg_START (8)
#define SOC_PMCTRL_CPUB_DVFS_VOL_STAT_cpub_pmu_mem_vol_chg_END (8)
#define SOC_PMCTRL_CPUB_DVFS_VOL_STAT_cpub_pmu_logic_vol_idx_START (12)
#define SOC_PMCTRL_CPUB_DVFS_VOL_STAT_cpub_pmu_logic_vol_idx_END (19)
#define SOC_PMCTRL_CPUB_DVFS_VOL_STAT_cpub_pmu_logic_vol_chg_START (20)
#define SOC_PMCTRL_CPUB_DVFS_VOL_STAT_cpub_pmu_logic_vol_chg_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fcm_pmu_mem_vol_idx : 8;
        unsigned int fcm_pmu_mem_vol_chg : 1;
        unsigned int reserved_0 : 3;
        unsigned int fcm_pmu_logic_vol_idx : 8;
        unsigned int fcm_pmu_logic_vol_chg : 1;
        unsigned int reserved_1 : 3;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_FCM_DVFS_VOL_STAT_UNION;
#endif
#define SOC_PMCTRL_FCM_DVFS_VOL_STAT_fcm_pmu_mem_vol_idx_START (0)
#define SOC_PMCTRL_FCM_DVFS_VOL_STAT_fcm_pmu_mem_vol_idx_END (7)
#define SOC_PMCTRL_FCM_DVFS_VOL_STAT_fcm_pmu_mem_vol_chg_START (8)
#define SOC_PMCTRL_FCM_DVFS_VOL_STAT_fcm_pmu_mem_vol_chg_END (8)
#define SOC_PMCTRL_FCM_DVFS_VOL_STAT_fcm_pmu_logic_vol_idx_START (12)
#define SOC_PMCTRL_FCM_DVFS_VOL_STAT_fcm_pmu_logic_vol_idx_END (19)
#define SOC_PMCTRL_FCM_DVFS_VOL_STAT_fcm_pmu_logic_vol_chg_START (20)
#define SOC_PMCTRL_FCM_DVFS_VOL_STAT_fcm_pmu_logic_vol_chg_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpu_core_pmu_mem_vol_idx : 8;
        unsigned int gpu_core_pmu_mem_vol_chg : 1;
        unsigned int reserved_0 : 3;
        unsigned int gpu_core_pmu_logic_vol_idx : 8;
        unsigned int gpu_core_pmu_logic_vol_chg : 1;
        unsigned int reserved_1 : 3;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_GPU_CORE_DVFS_VOL_STAT_UNION;
#endif
#define SOC_PMCTRL_GPU_CORE_DVFS_VOL_STAT_gpu_core_pmu_mem_vol_idx_START (0)
#define SOC_PMCTRL_GPU_CORE_DVFS_VOL_STAT_gpu_core_pmu_mem_vol_idx_END (7)
#define SOC_PMCTRL_GPU_CORE_DVFS_VOL_STAT_gpu_core_pmu_mem_vol_chg_START (8)
#define SOC_PMCTRL_GPU_CORE_DVFS_VOL_STAT_gpu_core_pmu_mem_vol_chg_END (8)
#define SOC_PMCTRL_GPU_CORE_DVFS_VOL_STAT_gpu_core_pmu_logic_vol_idx_START (12)
#define SOC_PMCTRL_GPU_CORE_DVFS_VOL_STAT_gpu_core_pmu_logic_vol_idx_END (19)
#define SOC_PMCTRL_GPU_CORE_DVFS_VOL_STAT_gpu_core_pmu_logic_vol_chg_START (20)
#define SOC_PMCTRL_GPU_CORE_DVFS_VOL_STAT_gpu_core_pmu_logic_vol_chg_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpu_top_pmu_mem_vol_idx : 8;
        unsigned int gpu_top_pmu_mem_vol_chg : 1;
        unsigned int reserved_0 : 3;
        unsigned int gpu_top_pmu_logic_vol_idx : 8;
        unsigned int gpu_top_pmu_logic_vol_chg : 1;
        unsigned int reserved_1 : 3;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_GPU_TOP_DVFS_VOL_STAT_UNION;
#endif
#define SOC_PMCTRL_GPU_TOP_DVFS_VOL_STAT_gpu_top_pmu_mem_vol_idx_START (0)
#define SOC_PMCTRL_GPU_TOP_DVFS_VOL_STAT_gpu_top_pmu_mem_vol_idx_END (7)
#define SOC_PMCTRL_GPU_TOP_DVFS_VOL_STAT_gpu_top_pmu_mem_vol_chg_START (8)
#define SOC_PMCTRL_GPU_TOP_DVFS_VOL_STAT_gpu_top_pmu_mem_vol_chg_END (8)
#define SOC_PMCTRL_GPU_TOP_DVFS_VOL_STAT_gpu_top_pmu_logic_vol_idx_START (12)
#define SOC_PMCTRL_GPU_TOP_DVFS_VOL_STAT_gpu_top_pmu_logic_vol_idx_END (19)
#define SOC_PMCTRL_GPU_TOP_DVFS_VOL_STAT_gpu_top_pmu_logic_vol_chg_START (20)
#define SOC_PMCTRL_GPU_TOP_DVFS_VOL_STAT_gpu_top_pmu_logic_vol_chg_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpu_hpm_clk_div : 6;
        unsigned int reserved : 26;
    } reg;
} SOC_PMCTRL_GPU_HPMCLKDIV_UNION;
#endif
#define SOC_PMCTRL_GPU_HPMCLKDIV_gpu_hpm_clk_div_START (0)
#define SOC_PMCTRL_GPU_HPMCLKDIV_gpu_hpm_clk_div_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpu_hpm_type_sel : 1;
        unsigned int reserved_0 : 3;
        unsigned int gpu_hpm_addr_sel : 5;
        unsigned int reserved_1 : 23;
    } reg;
} SOC_PMCTRL_GPU_HPMSEL_UNION;
#endif
#define SOC_PMCTRL_GPU_HPMSEL_gpu_hpm_type_sel_START (0)
#define SOC_PMCTRL_GPU_HPMSEL_gpu_hpm_type_sel_END (0)
#define SOC_PMCTRL_GPU_HPMSEL_gpu_hpm_addr_sel_START (4)
#define SOC_PMCTRL_GPU_HPMSEL_gpu_hpm_addr_sel_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpu_hpm_en : 1;
        unsigned int reserved_0 : 3;
        unsigned int gpu_hpm_opc_min_en : 1;
        unsigned int reserved_1 : 3;
        unsigned int gpu_hpmx_opc_min_en : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 22;
    } reg;
} SOC_PMCTRL_GPU_HPMEN_UNION;
#endif
#define SOC_PMCTRL_GPU_HPMEN_gpu_hpm_en_START (0)
#define SOC_PMCTRL_GPU_HPMEN_gpu_hpm_en_END (0)
#define SOC_PMCTRL_GPU_HPMEN_gpu_hpm_opc_min_en_START (4)
#define SOC_PMCTRL_GPU_HPMEN_gpu_hpm_opc_min_en_END (4)
#define SOC_PMCTRL_GPU_HPMEN_gpu_hpmx_opc_min_en_START (8)
#define SOC_PMCTRL_GPU_HPMEN_gpu_hpmx_opc_min_en_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpu_hpm_opc : 10;
        unsigned int reserved_0 : 2;
        unsigned int gpu_hpm_opc_vld : 1;
        unsigned int reserved_1 : 3;
        unsigned int gpu_hpm_opc_min : 10;
        unsigned int reserved_2 : 6;
    } reg;
} SOC_PMCTRL_GPU_HPMOPC_UNION;
#endif
#define SOC_PMCTRL_GPU_HPMOPC_gpu_hpm_opc_START (0)
#define SOC_PMCTRL_GPU_HPMOPC_gpu_hpm_opc_END (9)
#define SOC_PMCTRL_GPU_HPMOPC_gpu_hpm_opc_vld_START (12)
#define SOC_PMCTRL_GPU_HPMOPC_gpu_hpm_opc_vld_END (12)
#define SOC_PMCTRL_GPU_HPMOPC_gpu_hpm_opc_min_START (16)
#define SOC_PMCTRL_GPU_HPMOPC_gpu_hpm_opc_min_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERIBAKDATA0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERIBAKDATA1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERIBAKDATA2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERIBAKDATA3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spmi_vol_idx_sftcfg0 : 8;
        unsigned int spmi_vol_chg_sftreq0 : 1;
        unsigned int reserved_0 : 3;
        unsigned int spmi_vol_idx_sftcfg1 : 8;
        unsigned int spmi_vol_chg_sftreq1 : 1;
        unsigned int reserved_1 : 11;
    } reg;
} SOC_PMCTRL_SPMIVOLCFG_UNION;
#endif
#define SOC_PMCTRL_SPMIVOLCFG_spmi_vol_idx_sftcfg0_START (0)
#define SOC_PMCTRL_SPMIVOLCFG_spmi_vol_idx_sftcfg0_END (7)
#define SOC_PMCTRL_SPMIVOLCFG_spmi_vol_chg_sftreq0_START (8)
#define SOC_PMCTRL_SPMIVOLCFG_spmi_vol_chg_sftreq0_END (8)
#define SOC_PMCTRL_SPMIVOLCFG_spmi_vol_idx_sftcfg1_START (12)
#define SOC_PMCTRL_SPMIVOLCFG_spmi_vol_idx_sftcfg1_END (19)
#define SOC_PMCTRL_SPMIVOLCFG_spmi_vol_chg_sftreq1_START (20)
#define SOC_PMCTRL_SPMIVOLCFG_spmi_vol_chg_sftreq1_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spmi_vol_idx_sftcfg2 : 8;
        unsigned int spmi_vol_chg_sftreq2 : 1;
        unsigned int reserved_0 : 3;
        unsigned int spmi_vol_idx_sftcfg3 : 8;
        unsigned int spmi_vol_chg_sftreq3 : 1;
        unsigned int reserved_1 : 11;
    } reg;
} SOC_PMCTRL_SPMIVOLCFG1_UNION;
#endif
#define SOC_PMCTRL_SPMIVOLCFG1_spmi_vol_idx_sftcfg2_START (0)
#define SOC_PMCTRL_SPMIVOLCFG1_spmi_vol_idx_sftcfg2_END (7)
#define SOC_PMCTRL_SPMIVOLCFG1_spmi_vol_chg_sftreq2_START (8)
#define SOC_PMCTRL_SPMIVOLCFG1_spmi_vol_chg_sftreq2_END (8)
#define SOC_PMCTRL_SPMIVOLCFG1_spmi_vol_idx_sftcfg3_START (12)
#define SOC_PMCTRL_SPMIVOLCFG1_spmi_vol_idx_sftcfg3_END (19)
#define SOC_PMCTRL_SPMIVOLCFG1_spmi_vol_chg_sftreq3_START (20)
#define SOC_PMCTRL_SPMIVOLCFG1_spmi_vol_chg_sftreq3_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_auto_clkdiv_bypass : 1;
        unsigned int g3d_auto_clkdiv_time : 7;
        unsigned int g3d_pwr_dly_cnt : 8;
        unsigned int reserved_0 : 2;
        unsigned int g3d_div_debounce : 6;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_UNION;
#endif
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_auto_clkdiv_bypass_START (0)
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_auto_clkdiv_bypass_END (0)
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_auto_clkdiv_time_START (1)
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_auto_clkdiv_time_END (7)
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_pwr_dly_cnt_START (8)
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_pwr_dly_cnt_END (15)
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_div_debounce_START (18)
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_div_debounce_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_a_channel0 : 15;
        unsigned int freq_vote_a0_msk0 : 1;
        unsigned int freq_vote_a_channel1 : 15;
        unsigned int freq_vote_a0_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_A0_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_A0_freq_vote_a_channel0_START (0)
#define SOC_PMCTRL_FREQ_VOTE_A0_freq_vote_a_channel0_END (14)
#define SOC_PMCTRL_FREQ_VOTE_A0_freq_vote_a0_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_A0_freq_vote_a0_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_A0_freq_vote_a_channel1_START (16)
#define SOC_PMCTRL_FREQ_VOTE_A0_freq_vote_a_channel1_END (30)
#define SOC_PMCTRL_FREQ_VOTE_A0_freq_vote_a0_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_A0_freq_vote_a0_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_a_channel2 : 15;
        unsigned int freq_vote_a1_msk0 : 1;
        unsigned int freq_vote_a_channel3 : 15;
        unsigned int freq_vote_a1_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_A1_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_A1_freq_vote_a_channel2_START (0)
#define SOC_PMCTRL_FREQ_VOTE_A1_freq_vote_a_channel2_END (14)
#define SOC_PMCTRL_FREQ_VOTE_A1_freq_vote_a1_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_A1_freq_vote_a1_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_A1_freq_vote_a_channel3_START (16)
#define SOC_PMCTRL_FREQ_VOTE_A1_freq_vote_a_channel3_END (30)
#define SOC_PMCTRL_FREQ_VOTE_A1_freq_vote_a1_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_A1_freq_vote_a1_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_b_channel0 : 15;
        unsigned int freq_vote_b0_msk0 : 1;
        unsigned int freq_vote_b_channel1 : 15;
        unsigned int freq_vote_b0_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_B0_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_B0_freq_vote_b_channel0_START (0)
#define SOC_PMCTRL_FREQ_VOTE_B0_freq_vote_b_channel0_END (14)
#define SOC_PMCTRL_FREQ_VOTE_B0_freq_vote_b0_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_B0_freq_vote_b0_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_B0_freq_vote_b_channel1_START (16)
#define SOC_PMCTRL_FREQ_VOTE_B0_freq_vote_b_channel1_END (30)
#define SOC_PMCTRL_FREQ_VOTE_B0_freq_vote_b0_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_B0_freq_vote_b0_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_b_channel2 : 15;
        unsigned int freq_vote_b1_msk0 : 1;
        unsigned int freq_vote_b_channel3 : 15;
        unsigned int freq_vote_b1_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_B1_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_B1_freq_vote_b_channel2_START (0)
#define SOC_PMCTRL_FREQ_VOTE_B1_freq_vote_b_channel2_END (14)
#define SOC_PMCTRL_FREQ_VOTE_B1_freq_vote_b1_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_B1_freq_vote_b1_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_B1_freq_vote_b_channel3_START (16)
#define SOC_PMCTRL_FREQ_VOTE_B1_freq_vote_b_channel3_END (30)
#define SOC_PMCTRL_FREQ_VOTE_B1_freq_vote_b1_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_B1_freq_vote_b1_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_c_channel0 : 15;
        unsigned int freq_vote_c0_msk0 : 1;
        unsigned int freq_vote_c_channel1 : 15;
        unsigned int freq_vote_c0_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_C0_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_C0_freq_vote_c_channel0_START (0)
#define SOC_PMCTRL_FREQ_VOTE_C0_freq_vote_c_channel0_END (14)
#define SOC_PMCTRL_FREQ_VOTE_C0_freq_vote_c0_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_C0_freq_vote_c0_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_C0_freq_vote_c_channel1_START (16)
#define SOC_PMCTRL_FREQ_VOTE_C0_freq_vote_c_channel1_END (30)
#define SOC_PMCTRL_FREQ_VOTE_C0_freq_vote_c0_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_C0_freq_vote_c0_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_c_channel2 : 15;
        unsigned int freq_vote_c1_msk0 : 1;
        unsigned int freq_vote_c_channel3 : 15;
        unsigned int freq_vote_c1_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_C1_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_C1_freq_vote_c_channel2_START (0)
#define SOC_PMCTRL_FREQ_VOTE_C1_freq_vote_c_channel2_END (14)
#define SOC_PMCTRL_FREQ_VOTE_C1_freq_vote_c1_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_C1_freq_vote_c1_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_C1_freq_vote_c_channel3_START (16)
#define SOC_PMCTRL_FREQ_VOTE_C1_freq_vote_c_channel3_END (30)
#define SOC_PMCTRL_FREQ_VOTE_C1_freq_vote_c1_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_C1_freq_vote_c1_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_d_channel0 : 15;
        unsigned int freq_vote_d0_msk0 : 1;
        unsigned int freq_vote_d_channel1 : 15;
        unsigned int freq_vote_d0_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_D0_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_D0_freq_vote_d_channel0_START (0)
#define SOC_PMCTRL_FREQ_VOTE_D0_freq_vote_d_channel0_END (14)
#define SOC_PMCTRL_FREQ_VOTE_D0_freq_vote_d0_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_D0_freq_vote_d0_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_D0_freq_vote_d_channel1_START (16)
#define SOC_PMCTRL_FREQ_VOTE_D0_freq_vote_d_channel1_END (30)
#define SOC_PMCTRL_FREQ_VOTE_D0_freq_vote_d0_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_D0_freq_vote_d0_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_d_channel2 : 15;
        unsigned int freq_vote_d1_msk0 : 1;
        unsigned int freq_vote_d_channel3 : 15;
        unsigned int freq_vote_d1_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_D1_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_D1_freq_vote_d_channel2_START (0)
#define SOC_PMCTRL_FREQ_VOTE_D1_freq_vote_d_channel2_END (14)
#define SOC_PMCTRL_FREQ_VOTE_D1_freq_vote_d1_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_D1_freq_vote_d1_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_D1_freq_vote_d_channel3_START (16)
#define SOC_PMCTRL_FREQ_VOTE_D1_freq_vote_d_channel3_END (30)
#define SOC_PMCTRL_FREQ_VOTE_D1_freq_vote_d1_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_D1_freq_vote_d1_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_e_channel0 : 15;
        unsigned int freq_vote_e0_msk0 : 1;
        unsigned int freq_vote_e_channel1 : 15;
        unsigned int freq_vote_e0_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_E0_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_E0_freq_vote_e_channel0_START (0)
#define SOC_PMCTRL_FREQ_VOTE_E0_freq_vote_e_channel0_END (14)
#define SOC_PMCTRL_FREQ_VOTE_E0_freq_vote_e0_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_E0_freq_vote_e0_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_E0_freq_vote_e_channel1_START (16)
#define SOC_PMCTRL_FREQ_VOTE_E0_freq_vote_e_channel1_END (30)
#define SOC_PMCTRL_FREQ_VOTE_E0_freq_vote_e0_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_E0_freq_vote_e0_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_e_channel2 : 15;
        unsigned int freq_vote_e1_msk0 : 1;
        unsigned int freq_vote_e_channel3 : 15;
        unsigned int freq_vote_e1_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_E1_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_E1_freq_vote_e_channel2_START (0)
#define SOC_PMCTRL_FREQ_VOTE_E1_freq_vote_e_channel2_END (14)
#define SOC_PMCTRL_FREQ_VOTE_E1_freq_vote_e1_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_E1_freq_vote_e1_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_E1_freq_vote_e_channel3_START (16)
#define SOC_PMCTRL_FREQ_VOTE_E1_freq_vote_e_channel3_END (30)
#define SOC_PMCTRL_FREQ_VOTE_E1_freq_vote_e1_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_E1_freq_vote_e1_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_e_channel4 : 15;
        unsigned int freq_vote_e2_msk0 : 1;
        unsigned int freq_vote_e_channel5 : 15;
        unsigned int freq_vote_e2_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_E2_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_E2_freq_vote_e_channel4_START (0)
#define SOC_PMCTRL_FREQ_VOTE_E2_freq_vote_e_channel4_END (14)
#define SOC_PMCTRL_FREQ_VOTE_E2_freq_vote_e2_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_E2_freq_vote_e2_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_E2_freq_vote_e_channel5_START (16)
#define SOC_PMCTRL_FREQ_VOTE_E2_freq_vote_e_channel5_END (30)
#define SOC_PMCTRL_FREQ_VOTE_E2_freq_vote_e2_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_E2_freq_vote_e2_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_e_channel6 : 15;
        unsigned int freq_vote_e3_msk0 : 1;
        unsigned int freq_vote_e_channel7 : 15;
        unsigned int freq_vote_e3_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_E3_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_E3_freq_vote_e_channel6_START (0)
#define SOC_PMCTRL_FREQ_VOTE_E3_freq_vote_e_channel6_END (14)
#define SOC_PMCTRL_FREQ_VOTE_E3_freq_vote_e3_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_E3_freq_vote_e3_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_E3_freq_vote_e_channel7_START (16)
#define SOC_PMCTRL_FREQ_VOTE_E3_freq_vote_e_channel7_END (30)
#define SOC_PMCTRL_FREQ_VOTE_E3_freq_vote_e3_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_E3_freq_vote_e3_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_f_channel0 : 15;
        unsigned int freq_vote_f0_msk0 : 1;
        unsigned int freq_vote_f_channel1 : 15;
        unsigned int freq_vote_f0_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_F0_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_F0_freq_vote_f_channel0_START (0)
#define SOC_PMCTRL_FREQ_VOTE_F0_freq_vote_f_channel0_END (14)
#define SOC_PMCTRL_FREQ_VOTE_F0_freq_vote_f0_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_F0_freq_vote_f0_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_F0_freq_vote_f_channel1_START (16)
#define SOC_PMCTRL_FREQ_VOTE_F0_freq_vote_f_channel1_END (30)
#define SOC_PMCTRL_FREQ_VOTE_F0_freq_vote_f0_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_F0_freq_vote_f0_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_f_channel2 : 15;
        unsigned int freq_vote_f1_msk0 : 1;
        unsigned int freq_vote_f_channel3 : 15;
        unsigned int freq_vote_f1_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_F1_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_F1_freq_vote_f_channel2_START (0)
#define SOC_PMCTRL_FREQ_VOTE_F1_freq_vote_f_channel2_END (14)
#define SOC_PMCTRL_FREQ_VOTE_F1_freq_vote_f1_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_F1_freq_vote_f1_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_F1_freq_vote_f_channel3_START (16)
#define SOC_PMCTRL_FREQ_VOTE_F1_freq_vote_f_channel3_END (30)
#define SOC_PMCTRL_FREQ_VOTE_F1_freq_vote_f1_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_F1_freq_vote_f1_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_f_channel4 : 15;
        unsigned int freq_vote_f2_msk0 : 1;
        unsigned int freq_vote_f_channel5 : 15;
        unsigned int freq_vote_f2_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_F2_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_F2_freq_vote_f_channel4_START (0)
#define SOC_PMCTRL_FREQ_VOTE_F2_freq_vote_f_channel4_END (14)
#define SOC_PMCTRL_FREQ_VOTE_F2_freq_vote_f2_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_F2_freq_vote_f2_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_F2_freq_vote_f_channel5_START (16)
#define SOC_PMCTRL_FREQ_VOTE_F2_freq_vote_f_channel5_END (30)
#define SOC_PMCTRL_FREQ_VOTE_F2_freq_vote_f2_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_F2_freq_vote_f2_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_f_channel6 : 15;
        unsigned int freq_vote_f3_msk0 : 1;
        unsigned int freq_vote_f_channel7 : 15;
        unsigned int freq_vote_f3_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_F3_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_F3_freq_vote_f_channel6_START (0)
#define SOC_PMCTRL_FREQ_VOTE_F3_freq_vote_f_channel6_END (14)
#define SOC_PMCTRL_FREQ_VOTE_F3_freq_vote_f3_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_F3_freq_vote_f3_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_F3_freq_vote_f_channel7_START (16)
#define SOC_PMCTRL_FREQ_VOTE_F3_freq_vote_f_channel7_END (30)
#define SOC_PMCTRL_FREQ_VOTE_F3_freq_vote_f3_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_F3_freq_vote_f3_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_g_channel0 : 15;
        unsigned int freq_vote_g0_msk0 : 1;
        unsigned int freq_vote_g_channel1 : 15;
        unsigned int freq_vote_g0_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_G0_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_G0_freq_vote_g_channel0_START (0)
#define SOC_PMCTRL_FREQ_VOTE_G0_freq_vote_g_channel0_END (14)
#define SOC_PMCTRL_FREQ_VOTE_G0_freq_vote_g0_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_G0_freq_vote_g0_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_G0_freq_vote_g_channel1_START (16)
#define SOC_PMCTRL_FREQ_VOTE_G0_freq_vote_g_channel1_END (30)
#define SOC_PMCTRL_FREQ_VOTE_G0_freq_vote_g0_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_G0_freq_vote_g0_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_g_channel2 : 15;
        unsigned int freq_vote_g1_msk0 : 1;
        unsigned int freq_vote_g_channel3 : 15;
        unsigned int freq_vote_g1_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_G1_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_G1_freq_vote_g_channel2_START (0)
#define SOC_PMCTRL_FREQ_VOTE_G1_freq_vote_g_channel2_END (14)
#define SOC_PMCTRL_FREQ_VOTE_G1_freq_vote_g1_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_G1_freq_vote_g1_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_G1_freq_vote_g_channel3_START (16)
#define SOC_PMCTRL_FREQ_VOTE_G1_freq_vote_g_channel3_END (30)
#define SOC_PMCTRL_FREQ_VOTE_G1_freq_vote_g1_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_G1_freq_vote_g1_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_g_channel4 : 15;
        unsigned int freq_vote_g2_msk0 : 1;
        unsigned int freq_vote_g_channel5 : 15;
        unsigned int freq_vote_g2_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_G2_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_G2_freq_vote_g_channel4_START (0)
#define SOC_PMCTRL_FREQ_VOTE_G2_freq_vote_g_channel4_END (14)
#define SOC_PMCTRL_FREQ_VOTE_G2_freq_vote_g2_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_G2_freq_vote_g2_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_G2_freq_vote_g_channel5_START (16)
#define SOC_PMCTRL_FREQ_VOTE_G2_freq_vote_g_channel5_END (30)
#define SOC_PMCTRL_FREQ_VOTE_G2_freq_vote_g2_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_G2_freq_vote_g2_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_g_channel6 : 15;
        unsigned int freq_vote_g3_msk0 : 1;
        unsigned int freq_vote_g_channel7 : 15;
        unsigned int freq_vote_g3_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_G3_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_G3_freq_vote_g_channel6_START (0)
#define SOC_PMCTRL_FREQ_VOTE_G3_freq_vote_g_channel6_END (14)
#define SOC_PMCTRL_FREQ_VOTE_G3_freq_vote_g3_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_G3_freq_vote_g3_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_G3_freq_vote_g_channel7_START (16)
#define SOC_PMCTRL_FREQ_VOTE_G3_freq_vote_g_channel7_END (30)
#define SOC_PMCTRL_FREQ_VOTE_G3_freq_vote_g3_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_G3_freq_vote_g3_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_h_channel0 : 15;
        unsigned int freq_vote_h0_msk0 : 1;
        unsigned int freq_vote_h_channel1 : 15;
        unsigned int freq_vote_h0_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_H0_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_H0_freq_vote_h_channel0_START (0)
#define SOC_PMCTRL_FREQ_VOTE_H0_freq_vote_h_channel0_END (14)
#define SOC_PMCTRL_FREQ_VOTE_H0_freq_vote_h0_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_H0_freq_vote_h0_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_H0_freq_vote_h_channel1_START (16)
#define SOC_PMCTRL_FREQ_VOTE_H0_freq_vote_h_channel1_END (30)
#define SOC_PMCTRL_FREQ_VOTE_H0_freq_vote_h0_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_H0_freq_vote_h0_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_h_channel2 : 15;
        unsigned int freq_vote_h1_msk0 : 1;
        unsigned int freq_vote_h_channel3 : 15;
        unsigned int freq_vote_h1_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_H1_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_H1_freq_vote_h_channel2_START (0)
#define SOC_PMCTRL_FREQ_VOTE_H1_freq_vote_h_channel2_END (14)
#define SOC_PMCTRL_FREQ_VOTE_H1_freq_vote_h1_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_H1_freq_vote_h1_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_H1_freq_vote_h_channel3_START (16)
#define SOC_PMCTRL_FREQ_VOTE_H1_freq_vote_h_channel3_END (30)
#define SOC_PMCTRL_FREQ_VOTE_H1_freq_vote_h1_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_H1_freq_vote_h1_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_h_channel4 : 15;
        unsigned int freq_vote_h2_msk0 : 1;
        unsigned int freq_vote_h_channel5 : 15;
        unsigned int freq_vote_h2_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_H2_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_H2_freq_vote_h_channel4_START (0)
#define SOC_PMCTRL_FREQ_VOTE_H2_freq_vote_h_channel4_END (14)
#define SOC_PMCTRL_FREQ_VOTE_H2_freq_vote_h2_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_H2_freq_vote_h2_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_H2_freq_vote_h_channel5_START (16)
#define SOC_PMCTRL_FREQ_VOTE_H2_freq_vote_h_channel5_END (30)
#define SOC_PMCTRL_FREQ_VOTE_H2_freq_vote_h2_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_H2_freq_vote_h2_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_h_channel6 : 15;
        unsigned int freq_vote_h3_msk0 : 1;
        unsigned int freq_vote_h_channel7 : 15;
        unsigned int freq_vote_h3_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_H3_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_H3_freq_vote_h_channel6_START (0)
#define SOC_PMCTRL_FREQ_VOTE_H3_freq_vote_h_channel6_END (14)
#define SOC_PMCTRL_FREQ_VOTE_H3_freq_vote_h3_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_H3_freq_vote_h3_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_H3_freq_vote_h_channel7_START (16)
#define SOC_PMCTRL_FREQ_VOTE_H3_freq_vote_h_channel7_END (30)
#define SOC_PMCTRL_FREQ_VOTE_H3_freq_vote_h3_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_H3_freq_vote_h3_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_h_channel8 : 15;
        unsigned int freq_vote_h4_msk0 : 1;
        unsigned int freq_vote_h_channel9 : 15;
        unsigned int freq_vote_h4_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_H4_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_H4_freq_vote_h_channel8_START (0)
#define SOC_PMCTRL_FREQ_VOTE_H4_freq_vote_h_channel8_END (14)
#define SOC_PMCTRL_FREQ_VOTE_H4_freq_vote_h4_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_H4_freq_vote_h4_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_H4_freq_vote_h_channel9_START (16)
#define SOC_PMCTRL_FREQ_VOTE_H4_freq_vote_h_channel9_END (30)
#define SOC_PMCTRL_FREQ_VOTE_H4_freq_vote_h4_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_H4_freq_vote_h4_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_h_channel10 : 15;
        unsigned int freq_vote_h5_msk0 : 1;
        unsigned int freq_vote_h_channel11 : 15;
        unsigned int freq_vote_h5_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_H5_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_H5_freq_vote_h_channel10_START (0)
#define SOC_PMCTRL_FREQ_VOTE_H5_freq_vote_h_channel10_END (14)
#define SOC_PMCTRL_FREQ_VOTE_H5_freq_vote_h5_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_H5_freq_vote_h5_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_H5_freq_vote_h_channel11_START (16)
#define SOC_PMCTRL_FREQ_VOTE_H5_freq_vote_h_channel11_END (30)
#define SOC_PMCTRL_FREQ_VOTE_H5_freq_vote_h5_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_H5_freq_vote_h5_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_h_channel12 : 15;
        unsigned int freq_vote_h6_msk0 : 1;
        unsigned int freq_vote_h_channel13 : 15;
        unsigned int freq_vote_h6_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_H6_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_H6_freq_vote_h_channel12_START (0)
#define SOC_PMCTRL_FREQ_VOTE_H6_freq_vote_h_channel12_END (14)
#define SOC_PMCTRL_FREQ_VOTE_H6_freq_vote_h6_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_H6_freq_vote_h6_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_H6_freq_vote_h_channel13_START (16)
#define SOC_PMCTRL_FREQ_VOTE_H6_freq_vote_h_channel13_END (30)
#define SOC_PMCTRL_FREQ_VOTE_H6_freq_vote_h6_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_H6_freq_vote_h6_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_h_channel14 : 15;
        unsigned int freq_vote_h7_msk0 : 1;
        unsigned int freq_vote_h_channel15 : 15;
        unsigned int freq_vote_h7_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_H7_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_H7_freq_vote_h_channel14_START (0)
#define SOC_PMCTRL_FREQ_VOTE_H7_freq_vote_h_channel14_END (14)
#define SOC_PMCTRL_FREQ_VOTE_H7_freq_vote_h7_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_H7_freq_vote_h7_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_H7_freq_vote_h_channel15_START (16)
#define SOC_PMCTRL_FREQ_VOTE_H7_freq_vote_h_channel15_END (30)
#define SOC_PMCTRL_FREQ_VOTE_H7_freq_vote_h7_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_H7_freq_vote_h7_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_i_channel0 : 15;
        unsigned int freq_vote_i0_msk0 : 1;
        unsigned int freq_vote_i_channel1 : 15;
        unsigned int freq_vote_i0_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_I0_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_I0_freq_vote_i_channel0_START (0)
#define SOC_PMCTRL_FREQ_VOTE_I0_freq_vote_i_channel0_END (14)
#define SOC_PMCTRL_FREQ_VOTE_I0_freq_vote_i0_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_I0_freq_vote_i0_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_I0_freq_vote_i_channel1_START (16)
#define SOC_PMCTRL_FREQ_VOTE_I0_freq_vote_i_channel1_END (30)
#define SOC_PMCTRL_FREQ_VOTE_I0_freq_vote_i0_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_I0_freq_vote_i0_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_i_channel2 : 15;
        unsigned int freq_vote_i1_msk0 : 1;
        unsigned int freq_vote_i_channel3 : 15;
        unsigned int freq_vote_i1_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_I1_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_I1_freq_vote_i_channel2_START (0)
#define SOC_PMCTRL_FREQ_VOTE_I1_freq_vote_i_channel2_END (14)
#define SOC_PMCTRL_FREQ_VOTE_I1_freq_vote_i1_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_I1_freq_vote_i1_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_I1_freq_vote_i_channel3_START (16)
#define SOC_PMCTRL_FREQ_VOTE_I1_freq_vote_i_channel3_END (30)
#define SOC_PMCTRL_FREQ_VOTE_I1_freq_vote_i1_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_I1_freq_vote_i1_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_i_channel4 : 15;
        unsigned int freq_vote_i2_msk0 : 1;
        unsigned int freq_vote_i_channel5 : 15;
        unsigned int freq_vote_i2_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_I2_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_I2_freq_vote_i_channel4_START (0)
#define SOC_PMCTRL_FREQ_VOTE_I2_freq_vote_i_channel4_END (14)
#define SOC_PMCTRL_FREQ_VOTE_I2_freq_vote_i2_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_I2_freq_vote_i2_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_I2_freq_vote_i_channel5_START (16)
#define SOC_PMCTRL_FREQ_VOTE_I2_freq_vote_i_channel5_END (30)
#define SOC_PMCTRL_FREQ_VOTE_I2_freq_vote_i2_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_I2_freq_vote_i2_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_i_channel6 : 15;
        unsigned int freq_vote_i3_msk0 : 1;
        unsigned int freq_vote_i_channel7 : 15;
        unsigned int freq_vote_i3_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_I3_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_I3_freq_vote_i_channel6_START (0)
#define SOC_PMCTRL_FREQ_VOTE_I3_freq_vote_i_channel6_END (14)
#define SOC_PMCTRL_FREQ_VOTE_I3_freq_vote_i3_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_I3_freq_vote_i3_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_I3_freq_vote_i_channel7_START (16)
#define SOC_PMCTRL_FREQ_VOTE_I3_freq_vote_i_channel7_END (30)
#define SOC_PMCTRL_FREQ_VOTE_I3_freq_vote_i3_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_I3_freq_vote_i3_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_i_channel8 : 15;
        unsigned int freq_vote_i4_msk0 : 1;
        unsigned int freq_vote_i_channel9 : 15;
        unsigned int freq_vote_i4_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_I4_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_I4_freq_vote_i_channel8_START (0)
#define SOC_PMCTRL_FREQ_VOTE_I4_freq_vote_i_channel8_END (14)
#define SOC_PMCTRL_FREQ_VOTE_I4_freq_vote_i4_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_I4_freq_vote_i4_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_I4_freq_vote_i_channel9_START (16)
#define SOC_PMCTRL_FREQ_VOTE_I4_freq_vote_i_channel9_END (30)
#define SOC_PMCTRL_FREQ_VOTE_I4_freq_vote_i4_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_I4_freq_vote_i4_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_i_channel10 : 15;
        unsigned int freq_vote_i5_msk0 : 1;
        unsigned int freq_vote_i_channel11 : 15;
        unsigned int freq_vote_i5_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_I5_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_I5_freq_vote_i_channel10_START (0)
#define SOC_PMCTRL_FREQ_VOTE_I5_freq_vote_i_channel10_END (14)
#define SOC_PMCTRL_FREQ_VOTE_I5_freq_vote_i5_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_I5_freq_vote_i5_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_I5_freq_vote_i_channel11_START (16)
#define SOC_PMCTRL_FREQ_VOTE_I5_freq_vote_i_channel11_END (30)
#define SOC_PMCTRL_FREQ_VOTE_I5_freq_vote_i5_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_I5_freq_vote_i5_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_i_channel12 : 15;
        unsigned int freq_vote_i6_msk0 : 1;
        unsigned int freq_vote_i_channel13 : 15;
        unsigned int freq_vote_i6_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_I6_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_I6_freq_vote_i_channel12_START (0)
#define SOC_PMCTRL_FREQ_VOTE_I6_freq_vote_i_channel12_END (14)
#define SOC_PMCTRL_FREQ_VOTE_I6_freq_vote_i6_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_I6_freq_vote_i6_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_I6_freq_vote_i_channel13_START (16)
#define SOC_PMCTRL_FREQ_VOTE_I6_freq_vote_i_channel13_END (30)
#define SOC_PMCTRL_FREQ_VOTE_I6_freq_vote_i6_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_I6_freq_vote_i6_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_i_channel14 : 15;
        unsigned int freq_vote_i7_msk0 : 1;
        unsigned int freq_vote_i_channel15 : 15;
        unsigned int freq_vote_i7_msk1 : 1;
    } reg;
} SOC_PMCTRL_FREQ_VOTE_I7_UNION;
#endif
#define SOC_PMCTRL_FREQ_VOTE_I7_freq_vote_i_channel14_START (0)
#define SOC_PMCTRL_FREQ_VOTE_I7_freq_vote_i_channel14_END (14)
#define SOC_PMCTRL_FREQ_VOTE_I7_freq_vote_i7_msk0_START (15)
#define SOC_PMCTRL_FREQ_VOTE_I7_freq_vote_i7_msk0_END (15)
#define SOC_PMCTRL_FREQ_VOTE_I7_freq_vote_i_channel15_START (16)
#define SOC_PMCTRL_FREQ_VOTE_I7_freq_vote_i_channel15_END (30)
#define SOC_PMCTRL_FREQ_VOTE_I7_freq_vote_i7_msk1_START (31)
#define SOC_PMCTRL_FREQ_VOTE_I7_freq_vote_i7_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_channel_num_a : 2;
        unsigned int freq_vote_channel_num_b : 2;
        unsigned int freq_vote_channel_num_c : 2;
        unsigned int freq_vote_channel_num_d : 2;
        unsigned int freq_vote_channel_num_e : 2;
        unsigned int freq_vote_channel_num_f : 2;
        unsigned int freq_vote_channel_num_g : 2;
        unsigned int freq_vote_channel_num_h : 2;
        unsigned int freq_vote_channel_num_i : 2;
        unsigned int reserved : 14;
    } reg;
} SOC_PMCTRL_VOTE_CHANNEL_CFG_UNION;
#endif
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_a_START (0)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_a_END (1)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_b_START (2)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_b_END (3)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_c_START (4)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_c_END (5)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_d_START (6)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_d_END (7)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_e_START (8)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_e_END (9)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_f_START (10)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_f_END (11)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_g_START (12)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_g_END (13)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_h_START (14)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_h_END (15)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_i_START (16)
#define SOC_PMCTRL_VOTE_CHANNEL_CFG_freq_vote_channel_num_i_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_a_type : 2;
        unsigned int freq_vote_b_type : 2;
        unsigned int freq_vote_c_type : 2;
        unsigned int freq_vote_d_type : 2;
        unsigned int freq_vote_e_type : 2;
        unsigned int freq_vote_f_type : 2;
        unsigned int freq_vote_g_type : 2;
        unsigned int freq_vote_h_type : 2;
        unsigned int vote_mode_cfg_0_msk : 16;
    } reg;
} SOC_PMCTRL_VOTE_MODE_CFG_0_UNION;
#endif
#define SOC_PMCTRL_VOTE_MODE_CFG_0_freq_vote_a_type_START (0)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_freq_vote_a_type_END (1)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_freq_vote_b_type_START (2)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_freq_vote_b_type_END (3)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_freq_vote_c_type_START (4)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_freq_vote_c_type_END (5)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_freq_vote_d_type_START (6)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_freq_vote_d_type_END (7)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_freq_vote_e_type_START (8)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_freq_vote_e_type_END (9)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_freq_vote_f_type_START (10)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_freq_vote_f_type_END (11)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_freq_vote_g_type_START (12)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_freq_vote_g_type_END (13)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_freq_vote_h_type_START (14)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_freq_vote_h_type_END (15)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_vote_mode_cfg_0_msk_START (16)
#define SOC_PMCTRL_VOTE_MODE_CFG_0_vote_mode_cfg_0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_i_type : 2;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 2;
        unsigned int reserved_2 : 2;
        unsigned int reserved_3 : 2;
        unsigned int reserved_4 : 2;
        unsigned int reserved_5 : 2;
        unsigned int reserved_6 : 2;
        unsigned int vote_mode_cfg_1_msk : 16;
    } reg;
} SOC_PMCTRL_VOTE_MODE_CFG_1_UNION;
#endif
#define SOC_PMCTRL_VOTE_MODE_CFG_1_freq_vote_i_type_START (0)
#define SOC_PMCTRL_VOTE_MODE_CFG_1_freq_vote_i_type_END (1)
#define SOC_PMCTRL_VOTE_MODE_CFG_1_vote_mode_cfg_1_msk_START (16)
#define SOC_PMCTRL_VOTE_MODE_CFG_1_vote_mode_cfg_1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_a_start_en : 1;
        unsigned int freq_vote_b_start_en : 1;
        unsigned int freq_vote_c_start_en : 1;
        unsigned int freq_vote_d_start_en : 1;
        unsigned int freq_vote_e_start_en : 1;
        unsigned int freq_vote_f_start_en : 1;
        unsigned int freq_vote_g_start_en : 1;
        unsigned int freq_vote_h_start_en : 1;
        unsigned int freq_vote_i_start_en : 1;
        unsigned int reserved : 7;
        unsigned int vote_start_en_msk : 16;
    } reg;
} SOC_PMCTRL_VOTE_START_EN_UNION;
#endif
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_a_start_en_START (0)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_a_start_en_END (0)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_b_start_en_START (1)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_b_start_en_END (1)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_c_start_en_START (2)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_c_start_en_END (2)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_d_start_en_START (3)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_d_start_en_END (3)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_e_start_en_START (4)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_e_start_en_END (4)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_f_start_en_START (5)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_f_start_en_END (5)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_g_start_en_START (6)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_g_start_en_END (6)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_h_start_en_START (7)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_h_start_en_END (7)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_i_start_en_START (8)
#define SOC_PMCTRL_VOTE_START_EN_freq_vote_i_start_en_END (8)
#define SOC_PMCTRL_VOTE_START_EN_vote_start_en_msk_START (16)
#define SOC_PMCTRL_VOTE_START_EN_vote_start_en_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_freq_vote_en_a : 1;
        unsigned int int_freq_vote_en_b : 1;
        unsigned int int_freq_vote_en_c : 1;
        unsigned int int_freq_vote_en_d : 1;
        unsigned int int_freq_vote_en_e : 1;
        unsigned int int_freq_vote_en_f : 1;
        unsigned int int_freq_vote_en_g : 1;
        unsigned int int_freq_vote_en_h : 1;
        unsigned int int_freq_vote_en_i : 1;
        unsigned int reserved : 7;
        unsigned int int_vote_en_msk : 16;
    } reg;
} SOC_PMCTRL_INT_VOTE_EN_UNION;
#endif
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_a_START (0)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_a_END (0)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_b_START (1)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_b_END (1)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_c_START (2)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_c_END (2)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_d_START (3)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_d_END (3)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_e_START (4)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_e_END (4)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_f_START (5)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_f_END (5)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_g_START (6)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_g_END (6)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_h_START (7)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_h_END (7)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_i_START (8)
#define SOC_PMCTRL_INT_VOTE_EN_int_freq_vote_en_i_END (8)
#define SOC_PMCTRL_INT_VOTE_EN_int_vote_en_msk_START (16)
#define SOC_PMCTRL_INT_VOTE_EN_int_vote_en_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_freq_vote_a_clr : 1;
        unsigned int int_freq_vote_b_clr : 1;
        unsigned int int_freq_vote_c_clr : 1;
        unsigned int int_freq_vote_d_clr : 1;
        unsigned int int_freq_vote_e_clr : 1;
        unsigned int int_freq_vote_f_clr : 1;
        unsigned int int_freq_vote_g_clr : 1;
        unsigned int int_freq_vote_h_clr : 1;
        unsigned int int_freq_vote_i_clr : 1;
        unsigned int reserved_0 : 7;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_PMCTRL_INT_VOTE_CLR_UNION;
#endif
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_a_clr_START (0)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_a_clr_END (0)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_b_clr_START (1)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_b_clr_END (1)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_c_clr_START (2)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_c_clr_END (2)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_d_clr_START (3)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_d_clr_END (3)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_e_clr_START (4)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_e_clr_END (4)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_f_clr_START (5)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_f_clr_END (5)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_g_clr_START (6)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_g_clr_END (6)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_h_clr_START (7)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_h_clr_END (7)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_i_clr_START (8)
#define SOC_PMCTRL_INT_VOTE_CLR_int_freq_vote_i_clr_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_freq_vote_raw_stat_a : 1;
        unsigned int int_freq_vote_raw_stat_b : 1;
        unsigned int int_freq_vote_raw_stat_c : 1;
        unsigned int int_freq_vote_raw_stat_d : 1;
        unsigned int int_freq_vote_raw_stat_e : 1;
        unsigned int int_freq_vote_raw_stat_f : 1;
        unsigned int int_freq_vote_raw_stat_g : 1;
        unsigned int int_freq_vote_raw_stat_h : 1;
        unsigned int int_freq_vote_raw_stat_i : 1;
        unsigned int reserved_0 : 7;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_PMCTRL_INT_VOTE_RAW_STAT_UNION;
#endif
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_a_START (0)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_a_END (0)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_b_START (1)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_b_END (1)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_c_START (2)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_c_END (2)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_d_START (3)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_d_END (3)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_e_START (4)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_e_END (4)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_f_START (5)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_f_END (5)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_g_START (6)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_g_END (6)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_h_START (7)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_h_END (7)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_i_START (8)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_freq_vote_raw_stat_i_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_freq_vote_stat_a : 1;
        unsigned int int_freq_vote_stat_b : 1;
        unsigned int int_freq_vote_stat_c : 1;
        unsigned int int_freq_vote_stat_d : 1;
        unsigned int int_freq_vote_stat_e : 1;
        unsigned int int_freq_vote_stat_f : 1;
        unsigned int int_freq_vote_stat_g : 1;
        unsigned int int_freq_vote_stat_h : 1;
        unsigned int int_freq_vote_stat_i : 1;
        unsigned int reserved_0 : 7;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_PMCTRL_INT_VOTE_STAT_UNION;
#endif
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_a_START (0)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_a_END (0)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_b_START (1)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_b_END (1)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_c_START (2)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_c_END (2)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_d_START (3)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_d_END (3)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_e_START (4)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_e_END (4)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_f_START (5)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_f_END (5)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_g_START (6)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_g_END (6)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_h_START (7)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_h_END (7)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_i_START (8)
#define SOC_PMCTRL_INT_VOTE_STAT_int_freq_vote_stat_i_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_frequency_vote : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_INT_FREQUENCY_VOTE_UNION;
#endif
#define SOC_PMCTRL_INT_FREQUENCY_VOTE_int_frequency_vote_START (0)
#define SOC_PMCTRL_INT_FREQUENCY_VOTE_int_frequency_vote_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_result_a : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_VOTE_RESULT_A_UNION;
#endif
#define SOC_PMCTRL_VOTE_RESULT_A_freq_vote_result_a_START (0)
#define SOC_PMCTRL_VOTE_RESULT_A_freq_vote_result_a_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_result_b : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_VOTE_RESULT_B_UNION;
#endif
#define SOC_PMCTRL_VOTE_RESULT_B_freq_vote_result_b_START (0)
#define SOC_PMCTRL_VOTE_RESULT_B_freq_vote_result_b_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_result_c : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_VOTE_RESULT_C_UNION;
#endif
#define SOC_PMCTRL_VOTE_RESULT_C_freq_vote_result_c_START (0)
#define SOC_PMCTRL_VOTE_RESULT_C_freq_vote_result_c_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_result_d : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_VOTE_RESULT_D_UNION;
#endif
#define SOC_PMCTRL_VOTE_RESULT_D_freq_vote_result_d_START (0)
#define SOC_PMCTRL_VOTE_RESULT_D_freq_vote_result_d_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_result_e : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_VOTE_RESULT_E_UNION;
#endif
#define SOC_PMCTRL_VOTE_RESULT_E_freq_vote_result_e_START (0)
#define SOC_PMCTRL_VOTE_RESULT_E_freq_vote_result_e_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_result_f : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_VOTE_RESULT_F_UNION;
#endif
#define SOC_PMCTRL_VOTE_RESULT_F_freq_vote_result_f_START (0)
#define SOC_PMCTRL_VOTE_RESULT_F_freq_vote_result_f_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_result_g : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_VOTE_RESULT_G_UNION;
#endif
#define SOC_PMCTRL_VOTE_RESULT_G_freq_vote_result_g_START (0)
#define SOC_PMCTRL_VOTE_RESULT_G_freq_vote_result_g_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_result_h : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_VOTE_RESULT_H_UNION;
#endif
#define SOC_PMCTRL_VOTE_RESULT_H_freq_vote_result_h_START (0)
#define SOC_PMCTRL_VOTE_RESULT_H_freq_vote_result_h_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_result_i : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_VOTE_RESULT_I_UNION;
#endif
#define SOC_PMCTRL_VOTE_RESULT_I_freq_vote_result_i_START (0)
#define SOC_PMCTRL_VOTE_RESULT_I_freq_vote_result_i_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddrc_csysreq_cfg : 4;
        unsigned int ddrc_csysack : 4;
        unsigned int ddrc_csysreq_stat : 4;
        unsigned int reserved : 20;
    } reg;
} SOC_PMCTRL_DDRLPCTRL_UNION;
#endif
#define SOC_PMCTRL_DDRLPCTRL_ddrc_csysreq_cfg_START (0)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_csysreq_cfg_END (3)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_csysack_START (4)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_csysack_END (7)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_csysreq_stat_START (8)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_csysreq_stat_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_ctrl_core_crg : 1;
        unsigned int dbg_ctrl_cssys : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int dbg_ctrl_pmc : 1;
        unsigned int dbg_ctrl_crg : 1;
        unsigned int reserved_4 : 24;
    } reg;
} SOC_PMCTRL_PERI_CTRL0_UNION;
#endif
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_core_crg_START (0)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_core_crg_END (0)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_cssys_START (1)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_cssys_END (1)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_pmc_START (6)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_pmc_END (6)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_crg_START (7)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_crg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 10;
        unsigned int reserved_1: 6;
        unsigned int reserved_2: 10;
        unsigned int reserved_3: 6;
    } reg;
} SOC_PMCTRL_PERI_CTRL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_vol_addr : 10;
        unsigned int reserved_0 : 6;
        unsigned int reserved_1 : 10;
        unsigned int reserved_2 : 6;
    } reg;
} SOC_PMCTRL_PERI_CTRL2_UNION;
#endif
#define SOC_PMCTRL_PERI_CTRL2_cluster0_vol_addr_START (0)
#define SOC_PMCTRL_PERI_CTRL2_cluster0_vol_addr_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_mem_vol_addr : 10;
        unsigned int reserved_0 : 6;
        unsigned int peri_vol_addr : 10;
        unsigned int reserved_1 : 6;
    } reg;
} SOC_PMCTRL_PERI_CTRL3_UNION;
#endif
#define SOC_PMCTRL_PERI_CTRL3_g3d_mem_vol_addr_START (0)
#define SOC_PMCTRL_PERI_CTRL3_g3d_mem_vol_addr_END (9)
#define SOC_PMCTRL_PERI_CTRL3_peri_vol_addr_START (16)
#define SOC_PMCTRL_PERI_CTRL3_peri_vol_addr_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 16;
        unsigned int reserved_1: 16;
    } reg;
} SOC_PMCTRL_PERI_CTRL4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_CTRL5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 2;
        unsigned int reserved_1: 2;
        unsigned int reserved_2: 3;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 24;
    } reg;
} SOC_PMCTRL_PERI_CTRL6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ispdss2ddr_dfs_ongoing : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_PERI_CTRL7_UNION;
#endif
#define SOC_PMCTRL_PERI_CTRL7_ispdss2ddr_dfs_ongoing_START (0)
#define SOC_PMCTRL_PERI_CTRL7_ispdss2ddr_dfs_ongoing_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_info_cssys : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT1_UNION;
#endif
#define SOC_PMCTRL_PERI_STAT1_dbg_info_cssys_START (0)
#define SOC_PMCTRL_PERI_STAT1_dbg_info_cssys_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddrphy_bypass_mode : 1;
        unsigned int noc_ivp_power_idlereq : 1;
        unsigned int noc_vcodec_power_idlereq : 1;
        unsigned int noc_cfgbus_power_idlereq : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_hsdt_power_idlereq : 1;
        unsigned int noc_usb_power_idlereq : 1;
        unsigned int noc_dmadebugbus_power_idlereq : 1;
        unsigned int noc_modem_power_idlereq : 1;
        unsigned int reserved_1 : 1;
        unsigned int noc_pcie0_power_idlereq : 1;
        unsigned int noc_tcu_power_idlereq : 1;
        unsigned int noc_isp_power_idlereq : 1;
        unsigned int noc_dss_power_idlereq : 1;
        unsigned int noc_vivo_power_idlereq : 1;
        unsigned int noc_vdec_power_idlereq : 1;
        unsigned int biten : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEREQ_0_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_ddrphy_bypass_mode_START (0)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_ddrphy_bypass_mode_END (0)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_ivp_power_idlereq_START (1)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_ivp_power_idlereq_END (1)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_vcodec_power_idlereq_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_vcodec_power_idlereq_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_cfgbus_power_idlereq_START (3)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_cfgbus_power_idlereq_END (3)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_hsdt_power_idlereq_START (5)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_hsdt_power_idlereq_END (5)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_usb_power_idlereq_START (6)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_usb_power_idlereq_END (6)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_dmadebugbus_power_idlereq_START (7)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_dmadebugbus_power_idlereq_END (7)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_modem_power_idlereq_START (8)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_modem_power_idlereq_END (8)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_pcie0_power_idlereq_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_pcie0_power_idlereq_END (10)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_tcu_power_idlereq_START (11)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_tcu_power_idlereq_END (11)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_isp_power_idlereq_START (12)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_isp_power_idlereq_END (12)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_dss_power_idlereq_START (13)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_dss_power_idlereq_END (13)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_vivo_power_idlereq_START (14)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_vivo_power_idlereq_END (14)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_vdec_power_idlereq_START (15)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_vdec_power_idlereq_END (15)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_biten_START (16)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_biten_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int noc_ivp_power_idleack : 1;
        unsigned int noc_vcodec_power_idleack : 1;
        unsigned int noc_cfgbus_power_idleack : 1;
        unsigned int reserved_1 : 1;
        unsigned int noc_hsdt_power_idleack : 1;
        unsigned int noc_usb_power_idleack : 1;
        unsigned int noc_dmadebugbus_power_idleack : 1;
        unsigned int noc_modem_power_idleack : 1;
        unsigned int reserved_2 : 1;
        unsigned int noc_pcie0_power_idleack : 1;
        unsigned int noc_tcu_power_idleack : 1;
        unsigned int noc_isp_power_idleack : 1;
        unsigned int noc_dss_power_idleack : 1;
        unsigned int noc_vivo_power_idleack : 1;
        unsigned int noc_vdec_power_idleack : 1;
        unsigned int reserved_3 : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEACK_0_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_ivp_power_idleack_START (1)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_ivp_power_idleack_END (1)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_vcodec_power_idleack_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_vcodec_power_idleack_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_cfgbus_power_idleack_START (3)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_cfgbus_power_idleack_END (3)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_hsdt_power_idleack_START (5)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_hsdt_power_idleack_END (5)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_usb_power_idleack_START (6)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_usb_power_idleack_END (6)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_dmadebugbus_power_idleack_START (7)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_dmadebugbus_power_idleack_END (7)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_modem_power_idleack_START (8)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_modem_power_idleack_END (8)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_pcie0_power_idleack_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_pcie0_power_idleack_END (10)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_tcu_power_idleack_START (11)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_tcu_power_idleack_END (11)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_isp_power_idleack_START (12)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_isp_power_idleack_END (12)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_dss_power_idleack_START (13)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_dss_power_idleack_END (13)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_vivo_power_idleack_START (14)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_vivo_power_idleack_END (14)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_vdec_power_idleack_START (15)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_vdec_power_idleack_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int noc_ivp_power_idle : 1;
        unsigned int noc_vcodec_power_idle : 1;
        unsigned int noc_cfgbus_power_idle : 1;
        unsigned int reserved_1 : 1;
        unsigned int noc_hsdt_power_idle : 1;
        unsigned int noc_usb_power_idle : 1;
        unsigned int noc_dmadebugbus_power_idle : 1;
        unsigned int noc_modem_power_idle : 1;
        unsigned int reserved_2 : 1;
        unsigned int noc_pcie0_power_idle : 1;
        unsigned int noc_tcu_power_idle : 1;
        unsigned int noc_isp_power_idle : 1;
        unsigned int noc_dss_power_idle : 1;
        unsigned int noc_vivo_power_idle : 1;
        unsigned int noc_vdec_power_idle : 1;
        unsigned int reserved_3 : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLE_0_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_ivp_power_idle_START (1)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_ivp_power_idle_END (1)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_vcodec_power_idle_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_vcodec_power_idle_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_cfgbus_power_idle_START (3)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_cfgbus_power_idle_END (3)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_hsdt_power_idle_START (5)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_hsdt_power_idle_END (5)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_usb_power_idle_START (6)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_usb_power_idle_END (6)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_dmadebugbus_power_idle_START (7)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_dmadebugbus_power_idle_END (7)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_modem_power_idle_START (8)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_modem_power_idle_END (8)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_pcie0_power_idle_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_pcie0_power_idle_END (10)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_tcu_power_idle_START (11)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_tcu_power_idle_END (11)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_isp_power_idle_START (12)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_isp_power_idle_END (12)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_dss_power_idle_START (13)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_dss_power_idle_END (13)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_vivo_power_idle_START (14)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_vivo_power_idle_END (14)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_vdec_power_idle_START (15)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_vdec_power_idle_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_venc_power_idlereq : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_npubus_power_idlereq : 1;
        unsigned int noc_aicore_power_idlereq : 1;
        unsigned int noc_mdm5gbus_power_idlereq : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int noc_npucpu_power_idlereq : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int noc_sysdma_power_idlereq : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int biten : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEREQ_1_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_venc_power_idlereq_START (0)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_venc_power_idlereq_END (0)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_npubus_power_idlereq_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_npubus_power_idlereq_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_aicore_power_idlereq_START (3)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_aicore_power_idlereq_END (3)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_mdm5gbus_power_idlereq_START (4)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_mdm5gbus_power_idlereq_END (4)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_npucpu_power_idlereq_START (7)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_npucpu_power_idlereq_END (7)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_sysdma_power_idlereq_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_sysdma_power_idlereq_END (10)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_biten_START (16)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_biten_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_venc_power_idleack : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_npubus_power_idleack : 1;
        unsigned int noc_aicore_power_idleack : 1;
        unsigned int noc_mdm5gbus_power_idleack : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int noc_npucpu_power_idleack : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int noc_sysdma_power_idleack : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEACK_1_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_venc_power_idleack_START (0)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_venc_power_idleack_END (0)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_npubus_power_idleack_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_npubus_power_idleack_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_aicore_power_idleack_START (3)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_aicore_power_idleack_END (3)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_mdm5gbus_power_idleack_START (4)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_mdm5gbus_power_idleack_END (4)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_npucpu_power_idleack_START (7)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_npucpu_power_idleack_END (7)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_sysdma_power_idleack_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_sysdma_power_idleack_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_venc_power_idle : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_npubus_power_idle : 1;
        unsigned int noc_aicore_power_idle : 1;
        unsigned int noc_mdm5gbus_power_idle : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int noc_npucpu_power_idle : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int noc_sysdma_power_idle : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLE_1_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_venc_power_idle_START (0)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_venc_power_idle_END (0)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_npubus_power_idle_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_npubus_power_idle_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_aicore_power_idle_START (3)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_aicore_power_idle_END (3)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_mdm5gbus_power_idle_START (4)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_mdm5gbus_power_idle_END (4)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_npucpu_power_idle_START (7)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_npucpu_power_idle_END (7)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_sysdma_power_idle_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_sysdma_power_idle_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int noc_mmc0noc_power_idlereq : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int biten : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEREQ_2_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_2_noc_mmc0noc_power_idlereq_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_2_noc_mmc0noc_power_idlereq_END (10)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_2_biten_START (16)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_2_biten_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int noc_mmc0noc_power_idleack : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEACK_2_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEACK_2_noc_mmc0noc_power_idleack_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_2_noc_mmc0noc_power_idleack_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int noc_mmc0noc_power_idle : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLE_2_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLE_2_noc_mmc0noc_power_idle_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLE_2_noc_mmc0noc_power_idle_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_npugic_cfg_t_maintimeout : 1;
        unsigned int noc_hkadc_ssi_t_maintimeout : 1;
        unsigned int noc_gpu_cfg_t_maintimeout : 1;
        unsigned int noc_lpm3_slv_t_maintimeout : 1;
        unsigned int noc_sys_peri0_cfg_t_maintimeout : 1;
        unsigned int noc_sys_peri1_cfg_t_maintimeout : 1;
        unsigned int noc_sys_peri2_cfg_t_maintimeout : 1;
        unsigned int noc_sys_peri3_cfg_t_maintimeout : 1;
        unsigned int noc_fd_cfg_t_maintimeout : 1;
        unsigned int noc_dmac_cfg_t_maintimeout : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_hisee_cfg_t_maintimeout : 1;
        unsigned int reserved_1 : 1;
        unsigned int noc_top_cssys_slv_cfg_t_maintimeout : 1;
        unsigned int noc_ao_tcp_cfg_t_maintimeout : 1;
        unsigned int noc_modem_cfg_t_maintimeout : 1;
        unsigned int noc_usb3otg_cfg_t_maintimeout : 1;
        unsigned int noc_npucpu_cfg_t_maintimeout : 1;
        unsigned int reserved_2 : 1;
        unsigned int noc_vcodec_crg_cfg_t_maintimeout : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int noc_pcie0_cfg_t_maintimeout : 1;
        unsigned int noc_cfg2vcodec_t_maintimeout : 1;
        unsigned int noc_cfg2vivo_t_maintimeout : 1;
        unsigned int noc_dmss_apb_slv_t_maintimeout : 1;
        unsigned int noc_npu2cfgbus_cfg_t_maitimeout : 1;
        unsigned int noc_asp_cfg_t_maintimeout : 1;
        unsigned int noc_npubus_cfg_t_maintimeout : 1;
        unsigned int noc_iomcu_ahb_slv_maintimeout : 1;
        unsigned int noc_iomcu_apb_slv_maintimeout : 1;
        unsigned int noc_aon_apb_slv_t_maintimeout : 1;
    } reg;
} SOC_PMCTRL_PERI_INT0_MASK_UNION;
#endif
#define SOC_PMCTRL_PERI_INT0_MASK_noc_npugic_cfg_t_maintimeout_START (0)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_npugic_cfg_t_maintimeout_END (0)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_hkadc_ssi_t_maintimeout_START (1)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_hkadc_ssi_t_maintimeout_END (1)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_gpu_cfg_t_maintimeout_START (2)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_gpu_cfg_t_maintimeout_END (2)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_lpm3_slv_t_maintimeout_START (3)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_lpm3_slv_t_maintimeout_END (3)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_sys_peri0_cfg_t_maintimeout_START (4)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_sys_peri0_cfg_t_maintimeout_END (4)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_sys_peri1_cfg_t_maintimeout_START (5)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_sys_peri1_cfg_t_maintimeout_END (5)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_sys_peri2_cfg_t_maintimeout_START (6)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_sys_peri2_cfg_t_maintimeout_END (6)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_sys_peri3_cfg_t_maintimeout_START (7)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_sys_peri3_cfg_t_maintimeout_END (7)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_fd_cfg_t_maintimeout_START (8)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_fd_cfg_t_maintimeout_END (8)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_dmac_cfg_t_maintimeout_START (9)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_dmac_cfg_t_maintimeout_END (9)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_hisee_cfg_t_maintimeout_START (11)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_hisee_cfg_t_maintimeout_END (11)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_top_cssys_slv_cfg_t_maintimeout_START (13)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_top_cssys_slv_cfg_t_maintimeout_END (13)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_ao_tcp_cfg_t_maintimeout_START (14)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_ao_tcp_cfg_t_maintimeout_END (14)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_modem_cfg_t_maintimeout_START (15)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_modem_cfg_t_maintimeout_END (15)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_usb3otg_cfg_t_maintimeout_START (16)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_usb3otg_cfg_t_maintimeout_END (16)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_npucpu_cfg_t_maintimeout_START (17)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_npucpu_cfg_t_maintimeout_END (17)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_vcodec_crg_cfg_t_maintimeout_START (19)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_vcodec_crg_cfg_t_maintimeout_END (19)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_pcie0_cfg_t_maintimeout_START (22)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_pcie0_cfg_t_maintimeout_END (22)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_cfg2vcodec_t_maintimeout_START (23)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_cfg2vcodec_t_maintimeout_END (23)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_cfg2vivo_t_maintimeout_START (24)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_cfg2vivo_t_maintimeout_END (24)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_dmss_apb_slv_t_maintimeout_START (25)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_dmss_apb_slv_t_maintimeout_END (25)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_npu2cfgbus_cfg_t_maitimeout_START (26)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_npu2cfgbus_cfg_t_maitimeout_END (26)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_asp_cfg_t_maintimeout_START (27)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_asp_cfg_t_maintimeout_END (27)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_npubus_cfg_t_maintimeout_START (28)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_npubus_cfg_t_maintimeout_END (28)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_iomcu_ahb_slv_maintimeout_START (29)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_iomcu_ahb_slv_maintimeout_END (29)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_iomcu_apb_slv_maintimeout_START (30)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_iomcu_apb_slv_maintimeout_END (30)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_aon_apb_slv_t_maintimeout_START (31)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_aon_apb_slv_t_maintimeout_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_npugic_cfg_t_maintimeout : 1;
        unsigned int noc_hkadc_ssi_t_maintimeout : 1;
        unsigned int noc_gpu_cfg_t_maintimeout : 1;
        unsigned int noc_lpm3_slv_t_maintimeout : 1;
        unsigned int noc_sys_peri0_cfg_t_maintimeout : 1;
        unsigned int noc_sys_peri1_cfg_t_maintimeout : 1;
        unsigned int noc_sys_peri2_cfg_t_maintimeout : 1;
        unsigned int noc_sys_peri3_cfg_t_maintimeout : 1;
        unsigned int noc_fd_cfg_t_maintimeout : 1;
        unsigned int noc_dmac_cfg_t_maintimeout : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_hisee_cfg_t_maintimeout : 1;
        unsigned int reserved_1 : 1;
        unsigned int noc_top_cssys_slv_cfg_t_maintimeout : 1;
        unsigned int noc_ao_tcp_cfg_t_maintimeout : 1;
        unsigned int noc_modem_cfg_t_maintimeout : 1;
        unsigned int noc_usb3otg_cfg_t_maintimeout : 1;
        unsigned int noc_npucpu_cfg_t_maintimeout : 1;
        unsigned int reserved_2 : 1;
        unsigned int noc_vcodec_crg_cfg_t_maintimeout : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int noc_pcie0_cfg_t_maintimeout : 1;
        unsigned int noc_cfg2vcodec_t_maintimeout : 1;
        unsigned int noc_cfg2vivo_t_maintimeout : 1;
        unsigned int noc_dmss_apb_slv_t_maintimeout : 1;
        unsigned int noc_npu2cfgbus_cfg_t_maitimeout : 1;
        unsigned int noc_asp_cfg_t_maintimeout : 1;
        unsigned int noc_npubus_cfg_t_maintimeout : 1;
        unsigned int noc_iomcu_ahb_slv_maintimeout : 1;
        unsigned int noc_iomcu_apb_slv_maintimeout : 1;
        unsigned int noc_aon_apb_slv_t_maintimeout : 1;
    } reg;
} SOC_PMCTRL_PERI_INT0_STAT_UNION;
#endif
#define SOC_PMCTRL_PERI_INT0_STAT_noc_npugic_cfg_t_maintimeout_START (0)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_npugic_cfg_t_maintimeout_END (0)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_hkadc_ssi_t_maintimeout_START (1)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_hkadc_ssi_t_maintimeout_END (1)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_gpu_cfg_t_maintimeout_START (2)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_gpu_cfg_t_maintimeout_END (2)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_lpm3_slv_t_maintimeout_START (3)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_lpm3_slv_t_maintimeout_END (3)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_sys_peri0_cfg_t_maintimeout_START (4)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_sys_peri0_cfg_t_maintimeout_END (4)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_sys_peri1_cfg_t_maintimeout_START (5)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_sys_peri1_cfg_t_maintimeout_END (5)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_sys_peri2_cfg_t_maintimeout_START (6)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_sys_peri2_cfg_t_maintimeout_END (6)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_sys_peri3_cfg_t_maintimeout_START (7)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_sys_peri3_cfg_t_maintimeout_END (7)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_fd_cfg_t_maintimeout_START (8)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_fd_cfg_t_maintimeout_END (8)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_dmac_cfg_t_maintimeout_START (9)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_dmac_cfg_t_maintimeout_END (9)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_hisee_cfg_t_maintimeout_START (11)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_hisee_cfg_t_maintimeout_END (11)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_top_cssys_slv_cfg_t_maintimeout_START (13)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_top_cssys_slv_cfg_t_maintimeout_END (13)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_ao_tcp_cfg_t_maintimeout_START (14)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_ao_tcp_cfg_t_maintimeout_END (14)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_modem_cfg_t_maintimeout_START (15)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_modem_cfg_t_maintimeout_END (15)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_usb3otg_cfg_t_maintimeout_START (16)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_usb3otg_cfg_t_maintimeout_END (16)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_npucpu_cfg_t_maintimeout_START (17)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_npucpu_cfg_t_maintimeout_END (17)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_vcodec_crg_cfg_t_maintimeout_START (19)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_vcodec_crg_cfg_t_maintimeout_END (19)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_pcie0_cfg_t_maintimeout_START (22)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_pcie0_cfg_t_maintimeout_END (22)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_cfg2vcodec_t_maintimeout_START (23)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_cfg2vcodec_t_maintimeout_END (23)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_cfg2vivo_t_maintimeout_START (24)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_cfg2vivo_t_maintimeout_END (24)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_dmss_apb_slv_t_maintimeout_START (25)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_dmss_apb_slv_t_maintimeout_END (25)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_npu2cfgbus_cfg_t_maitimeout_START (26)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_npu2cfgbus_cfg_t_maitimeout_END (26)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_asp_cfg_t_maintimeout_START (27)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_asp_cfg_t_maintimeout_END (27)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_npubus_cfg_t_maintimeout_START (28)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_npubus_cfg_t_maintimeout_END (28)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_iomcu_ahb_slv_maintimeout_START (29)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_iomcu_ahb_slv_maintimeout_END (29)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_iomcu_apb_slv_maintimeout_START (30)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_iomcu_apb_slv_maintimeout_END (30)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_aon_apb_slv_t_maintimeout_START (31)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_aon_apb_slv_t_maintimeout_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_dss_cfg_t_maintimeout : 1;
        unsigned int noc_isp_cfg_t_maintimeout : 1;
        unsigned int noc_ipp_cfg_t_maintimeout : 1;
        unsigned int noc_vcodec2cfg_cfg_t_maintimeout : 1;
        unsigned int noc_vivo_crg_cfg_t_maintimeout : 1;
        unsigned int noc_ivp_cfg_t_maintimeout : 1;
        unsigned int noc_dmca_ahb_slv_t_maintimeout : 1;
        unsigned int noc_dmcb_ahb_slv_t_maintimeout : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int noc_hsdtbus_apb_cfg_t_maintimeout : 1;
        unsigned int noc_mad_cfg_t_maintimeout : 1;
        unsigned int noc_cfg2ipu_t_maintimeout : 1;
        unsigned int noc_venc_cfg_t_maintimeout : 1;
        unsigned int reserved_3 : 1;
        unsigned int noc_vdec_cfg_t_maintimeout : 1;
        unsigned int noc_vivo2cfg_cfg_t_maintimeout : 1;
        unsigned int reserved_4 : 1;
        unsigned int noc_aicore_cfg_t_maintimeout : 1;
        unsigned int reserved_5 : 1;
        unsigned int noc_mmc0bus_apb_cfg_t_maintimeout : 1;
        unsigned int noc_l2buffer_slv0_rd_t_maintimeout : 1;
        unsigned int noc_l2buffer_slv0_wr_t_maintimeout : 1;
        unsigned int noc_l2buffer_slv1_rd_t_maintimeout : 1;
        unsigned int noc_l2buffer_slv1_wr_t_maintimeout : 1;
        unsigned int reserved_6 : 1;
        unsigned int noc_bba_cfg_t_maintimeout : 1;
        unsigned int noc_modem5g_cfg_t_maintimeout : 1;
        unsigned int noc_maa_cfg_t_maintimeout : 1;
        unsigned int noc_aximem_slv_cfg_t_maintimeout : 1;
        unsigned int reserved_7 : 1;
    } reg;
} SOC_PMCTRL_PERI_INT1_MASK_UNION;
#endif
#define SOC_PMCTRL_PERI_INT1_MASK_noc_dss_cfg_t_maintimeout_START (0)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_dss_cfg_t_maintimeout_END (0)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_isp_cfg_t_maintimeout_START (1)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_isp_cfg_t_maintimeout_END (1)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_ipp_cfg_t_maintimeout_START (2)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_ipp_cfg_t_maintimeout_END (2)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_vcodec2cfg_cfg_t_maintimeout_START (3)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_vcodec2cfg_cfg_t_maintimeout_END (3)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_vivo_crg_cfg_t_maintimeout_START (4)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_vivo_crg_cfg_t_maintimeout_END (4)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_ivp_cfg_t_maintimeout_START (5)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_ivp_cfg_t_maintimeout_END (5)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_dmca_ahb_slv_t_maintimeout_START (6)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_dmca_ahb_slv_t_maintimeout_END (6)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_dmcb_ahb_slv_t_maintimeout_START (7)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_dmcb_ahb_slv_t_maintimeout_END (7)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_hsdtbus_apb_cfg_t_maintimeout_START (11)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_hsdtbus_apb_cfg_t_maintimeout_END (11)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_mad_cfg_t_maintimeout_START (12)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_mad_cfg_t_maintimeout_END (12)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_cfg2ipu_t_maintimeout_START (13)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_cfg2ipu_t_maintimeout_END (13)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_venc_cfg_t_maintimeout_START (14)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_venc_cfg_t_maintimeout_END (14)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_vdec_cfg_t_maintimeout_START (16)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_vdec_cfg_t_maintimeout_END (16)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_vivo2cfg_cfg_t_maintimeout_START (17)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_vivo2cfg_cfg_t_maintimeout_END (17)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_aicore_cfg_t_maintimeout_START (19)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_aicore_cfg_t_maintimeout_END (19)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_mmc0bus_apb_cfg_t_maintimeout_START (21)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_mmc0bus_apb_cfg_t_maintimeout_END (21)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_l2buffer_slv0_rd_t_maintimeout_START (22)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_l2buffer_slv0_rd_t_maintimeout_END (22)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_l2buffer_slv0_wr_t_maintimeout_START (23)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_l2buffer_slv0_wr_t_maintimeout_END (23)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_l2buffer_slv1_rd_t_maintimeout_START (24)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_l2buffer_slv1_rd_t_maintimeout_END (24)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_l2buffer_slv1_wr_t_maintimeout_START (25)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_l2buffer_slv1_wr_t_maintimeout_END (25)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_bba_cfg_t_maintimeout_START (27)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_bba_cfg_t_maintimeout_END (27)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_modem5g_cfg_t_maintimeout_START (28)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_modem5g_cfg_t_maintimeout_END (28)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_maa_cfg_t_maintimeout_START (29)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_maa_cfg_t_maintimeout_END (29)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_aximem_slv_cfg_t_maintimeout_START (30)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_aximem_slv_cfg_t_maintimeout_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_dss_cfg_t_maintimeout : 1;
        unsigned int noc_isp_cfg_t_maintimeout : 1;
        unsigned int noc_ipp_cfg_t_maintimeout : 1;
        unsigned int noc_vcodec2cfg_cfg_t_maintimeout : 1;
        unsigned int noc_vivo_crg_cfg_t_maintimeout : 1;
        unsigned int noc_ivp_cfg_t_maintimeout : 1;
        unsigned int noc_dmca_ahb_slv_t_maintimeout : 1;
        unsigned int noc_dmcb_ahb_slv_t_maintimeout : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int noc_hsdtbus_apb_cfg_t_maintimeout : 1;
        unsigned int noc_mad_cfg_t_maintimeout : 1;
        unsigned int noc_cfg2ipu_t_maintimeout : 1;
        unsigned int noc_venc_cfg_t_maintimeout : 1;
        unsigned int reserved_3 : 1;
        unsigned int noc_vdec_cfg_t_maintimeout : 1;
        unsigned int noc_vivo2cfg_cfg_t_maintimeout : 1;
        unsigned int reserved_4 : 1;
        unsigned int noc_aicore_cfg_t_maintimeout : 1;
        unsigned int reserved_5 : 1;
        unsigned int noc_mmc0bus_apb_cfg_t_maintimeout : 1;
        unsigned int noc_l2buffer_slv0_rd_t_maintimeout : 1;
        unsigned int noc_l2buffer_slv0_wr_t_maintimeout : 1;
        unsigned int noc_l2buffer_slv1_rd_t_maintimeout : 1;
        unsigned int noc_l2buffer_slv1_wr_t_maintimeout : 1;
        unsigned int reserved_6 : 1;
        unsigned int noc_bba_cfg_t_maintimeout : 1;
        unsigned int noc_modem5g_cfg_t_maintimeout : 1;
        unsigned int noc_maa_cfg_t_maintimeout : 1;
        unsigned int noc_aximem_slv_cfg_t_maintimeout : 1;
        unsigned int reserved_7 : 1;
    } reg;
} SOC_PMCTRL_PERI_INT1_STAT_UNION;
#endif
#define SOC_PMCTRL_PERI_INT1_STAT_noc_dss_cfg_t_maintimeout_START (0)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_dss_cfg_t_maintimeout_END (0)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_isp_cfg_t_maintimeout_START (1)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_isp_cfg_t_maintimeout_END (1)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_ipp_cfg_t_maintimeout_START (2)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_ipp_cfg_t_maintimeout_END (2)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_vcodec2cfg_cfg_t_maintimeout_START (3)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_vcodec2cfg_cfg_t_maintimeout_END (3)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_vivo_crg_cfg_t_maintimeout_START (4)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_vivo_crg_cfg_t_maintimeout_END (4)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_ivp_cfg_t_maintimeout_START (5)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_ivp_cfg_t_maintimeout_END (5)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_dmca_ahb_slv_t_maintimeout_START (6)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_dmca_ahb_slv_t_maintimeout_END (6)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_dmcb_ahb_slv_t_maintimeout_START (7)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_dmcb_ahb_slv_t_maintimeout_END (7)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_hsdtbus_apb_cfg_t_maintimeout_START (11)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_hsdtbus_apb_cfg_t_maintimeout_END (11)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_mad_cfg_t_maintimeout_START (12)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_mad_cfg_t_maintimeout_END (12)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_cfg2ipu_t_maintimeout_START (13)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_cfg2ipu_t_maintimeout_END (13)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_venc_cfg_t_maintimeout_START (14)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_venc_cfg_t_maintimeout_END (14)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_vdec_cfg_t_maintimeout_START (16)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_vdec_cfg_t_maintimeout_END (16)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_vivo2cfg_cfg_t_maintimeout_START (17)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_vivo2cfg_cfg_t_maintimeout_END (17)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_aicore_cfg_t_maintimeout_START (19)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_aicore_cfg_t_maintimeout_END (19)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_mmc0bus_apb_cfg_t_maintimeout_START (21)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_mmc0bus_apb_cfg_t_maintimeout_END (21)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_l2buffer_slv0_rd_t_maintimeout_START (22)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_l2buffer_slv0_rd_t_maintimeout_END (22)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_l2buffer_slv0_wr_t_maintimeout_START (23)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_l2buffer_slv0_wr_t_maintimeout_END (23)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_l2buffer_slv1_rd_t_maintimeout_START (24)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_l2buffer_slv1_rd_t_maintimeout_END (24)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_l2buffer_slv1_wr_t_maintimeout_START (25)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_l2buffer_slv1_wr_t_maintimeout_END (25)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_bba_cfg_t_maintimeout_START (27)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_bba_cfg_t_maintimeout_END (27)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_modem5g_cfg_t_maintimeout_START (28)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_modem5g_cfg_t_maintimeout_END (28)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_maa_cfg_t_maintimeout_START (29)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_maa_cfg_t_maintimeout_END (29)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_aximem_slv_cfg_t_maintimeout_START (30)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_aximem_slv_cfg_t_maintimeout_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_ufs_cfg_t_maintimeout : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int noc_npu2cfgbus_t_maintimeout : 1;
        unsigned int noc_fcm2sys_t_maintimeout : 1;
        unsigned int noc_gic_cfg_t_maintimeout : 1;
        unsigned int noc_fcm_cfg_t_maintimeout : 1;
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
        unsigned int reserved_24 : 1;
        unsigned int reserved_25 : 1;
        unsigned int reserved_26 : 1;
    } reg;
} SOC_PMCTRL_PERI_INT2_MASK_UNION;
#endif
#define SOC_PMCTRL_PERI_INT2_MASK_noc_ufs_cfg_t_maintimeout_START (0)
#define SOC_PMCTRL_PERI_INT2_MASK_noc_ufs_cfg_t_maintimeout_END (0)
#define SOC_PMCTRL_PERI_INT2_MASK_noc_npu2cfgbus_t_maintimeout_START (4)
#define SOC_PMCTRL_PERI_INT2_MASK_noc_npu2cfgbus_t_maintimeout_END (4)
#define SOC_PMCTRL_PERI_INT2_MASK_noc_fcm2sys_t_maintimeout_START (5)
#define SOC_PMCTRL_PERI_INT2_MASK_noc_fcm2sys_t_maintimeout_END (5)
#define SOC_PMCTRL_PERI_INT2_MASK_noc_gic_cfg_t_maintimeout_START (6)
#define SOC_PMCTRL_PERI_INT2_MASK_noc_gic_cfg_t_maintimeout_END (6)
#define SOC_PMCTRL_PERI_INT2_MASK_noc_fcm_cfg_t_maintimeout_START (7)
#define SOC_PMCTRL_PERI_INT2_MASK_noc_fcm_cfg_t_maintimeout_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_ufs_cfg_t_maintimeout : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int noc_npu2cfgbus_t_maintimeout : 1;
        unsigned int noc_fcm2sys_t_maintimeout : 1;
        unsigned int noc_gic_cfg_t_maintimeout : 1;
        unsigned int noc_fcm_cfg_t_maintimeout : 1;
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
        unsigned int reserved_24 : 1;
        unsigned int reserved_25 : 1;
        unsigned int reserved_26 : 1;
    } reg;
} SOC_PMCTRL_PERI_INT2_STAT_UNION;
#endif
#define SOC_PMCTRL_PERI_INT2_STAT_noc_ufs_cfg_t_maintimeout_START (0)
#define SOC_PMCTRL_PERI_INT2_STAT_noc_ufs_cfg_t_maintimeout_END (0)
#define SOC_PMCTRL_PERI_INT2_STAT_noc_npu2cfgbus_t_maintimeout_START (4)
#define SOC_PMCTRL_PERI_INT2_STAT_noc_npu2cfgbus_t_maintimeout_END (4)
#define SOC_PMCTRL_PERI_INT2_STAT_noc_fcm2sys_t_maintimeout_START (5)
#define SOC_PMCTRL_PERI_INT2_STAT_noc_fcm2sys_t_maintimeout_END (5)
#define SOC_PMCTRL_PERI_INT2_STAT_noc_gic_cfg_t_maintimeout_START (6)
#define SOC_PMCTRL_PERI_INT2_STAT_noc_gic_cfg_t_maintimeout_END (6)
#define SOC_PMCTRL_PERI_INT2_STAT_noc_fcm_cfg_t_maintimeout_START (7)
#define SOC_PMCTRL_PERI_INT2_STAT_noc_fcm_cfg_t_maintimeout_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_INT3_MASK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_INT3_STAT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_ctrl_en_gpu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VS_CTRL_EN_GPU_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_EN_GPU_vs_ctrl_en_gpu_START (0)
#define SOC_PMCTRL_VS_CTRL_EN_GPU_vs_ctrl_en_gpu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_ctrl_bypass_gpu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VS_CTRL_BYPASS_GPU_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_BYPASS_GPU_vs_ctrl_bypass_gpu_START (0)
#define SOC_PMCTRL_VS_CTRL_BYPASS_GPU_vs_ctrl_bypass_gpu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int gpu_cpu0_wfi_wfe_bypass : 1;
        unsigned int gpu_cpu1_wfi_wfe_bypass : 1;
        unsigned int gpu_cpu2_wfi_wfe_bypass : 1;
        unsigned int gpu_cpu3_wfi_wfe_bypass : 1;
        unsigned int gpu_l2_idle_div_mod : 2;
        unsigned int gpu_cfg_cnt_cpu_l2_idle_quit : 16;
        unsigned int gpu_cpu_wake_up_mode : 2;
        unsigned int gpu_cpu_l2_idle_switch_bypass : 1;
        unsigned int gpu_cpu_l2_idle_gt_en : 1;
        unsigned int gpu_dvfs_div_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int sel_ckmux_gpu_icg : 1;
        unsigned int gpu_clk_div_cfg : 2;
    } reg;
} SOC_PMCTRL_VS_CTRL_GPU_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cpu0_wfi_wfe_bypass_START (1)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cpu0_wfi_wfe_bypass_END (1)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cpu1_wfi_wfe_bypass_START (2)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cpu1_wfi_wfe_bypass_END (2)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cpu2_wfi_wfe_bypass_START (3)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cpu2_wfi_wfe_bypass_END (3)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cpu3_wfi_wfe_bypass_START (4)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cpu3_wfi_wfe_bypass_END (4)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_l2_idle_div_mod_START (5)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_l2_idle_div_mod_END (6)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cfg_cnt_cpu_l2_idle_quit_START (7)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cfg_cnt_cpu_l2_idle_quit_END (22)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cpu_wake_up_mode_START (23)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cpu_wake_up_mode_END (24)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cpu_l2_idle_switch_bypass_START (25)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cpu_l2_idle_switch_bypass_END (25)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cpu_l2_idle_gt_en_START (26)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cpu_l2_idle_gt_en_END (26)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_dvfs_div_en_START (27)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_dvfs_div_en_END (27)
#define SOC_PMCTRL_VS_CTRL_GPU_sel_ckmux_gpu_icg_START (29)
#define SOC_PMCTRL_VS_CTRL_GPU_sel_ckmux_gpu_icg_END (29)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_clk_div_cfg_START (30)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_clk_div_cfg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_data_limit_e_gpu : 1;
        unsigned int svfd_off_time_step_gpu : 1;
        unsigned int svfd_pulse_width_sel_gpu : 1;
        unsigned int reserved_0 : 1;
        unsigned int svfd_ulvt_ll_gpu : 4;
        unsigned int svfd_ulvt_sl_gpu : 4;
        unsigned int svfd_lvt_ll_gpu : 4;
        unsigned int svfd_lvt_sl_gpu : 4;
        unsigned int svfd_trim_gpu : 2;
        unsigned int reserved_1 : 2;
        unsigned int svfd_d_rate_gpu : 2;
        unsigned int svfd_off_mode_gpu : 1;
        unsigned int svfd_div64_en_gpu : 1;
        unsigned int svfd_cpm_period_gpu : 1;
        unsigned int svfd_edge_sel_gpu : 1;
        unsigned int svfd_cmp_data_mode_gpu : 2;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL0_GPU_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_data_limit_e_gpu_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_data_limit_e_gpu_END (0)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_off_time_step_gpu_START (1)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_off_time_step_gpu_END (1)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_pulse_width_sel_gpu_START (2)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_pulse_width_sel_gpu_END (2)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_ulvt_ll_gpu_START (4)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_ulvt_ll_gpu_END (7)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_ulvt_sl_gpu_START (8)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_ulvt_sl_gpu_END (11)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_lvt_ll_gpu_START (12)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_lvt_ll_gpu_END (15)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_lvt_sl_gpu_START (16)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_lvt_sl_gpu_END (19)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_trim_gpu_START (20)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_trim_gpu_END (21)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_d_rate_gpu_START (24)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_d_rate_gpu_END (25)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_off_mode_gpu_START (26)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_off_mode_gpu_END (26)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_div64_en_gpu_START (27)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_div64_en_gpu_END (27)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_cpm_period_gpu_START (28)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_cpm_period_gpu_END (28)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_edge_sel_gpu_START (29)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_edge_sel_gpu_END (29)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_cmp_data_mode_gpu_START (30)
#define SOC_PMCTRL_VS_SVFD_CTRL0_GPU_svfd_cmp_data_mode_gpu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_glitch_test_gpu : 1;
        unsigned int reserved_0 : 3;
        unsigned int svfd_test_ffs_gpu : 8;
        unsigned int svfd_test_cpm_gpu : 8;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL1_GPU_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL1_GPU_svfd_glitch_test_gpu_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL1_GPU_svfd_glitch_test_gpu_END (0)
#define SOC_PMCTRL_VS_SVFD_CTRL1_GPU_svfd_test_ffs_gpu_START (4)
#define SOC_PMCTRL_VS_SVFD_CTRL1_GPU_svfd_test_ffs_gpu_END (11)
#define SOC_PMCTRL_VS_SVFD_CTRL1_GPU_svfd_test_cpm_gpu_START (12)
#define SOC_PMCTRL_VS_SVFD_CTRL1_GPU_svfd_test_cpm_gpu_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_enalbe_mode_gpu : 2;
        unsigned int svfs_cpm_threshold_r_gpu : 6;
        unsigned int svfs_cpm_threshold_f_gpu : 6;
        unsigned int vol_drop_en_gpu : 1;
        unsigned int reserved : 1;
        unsigned int vs_svfd_ctrl2_gpu_msk : 16;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL2_GPU_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL2_GPU_svfd_enalbe_mode_gpu_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL2_GPU_svfd_enalbe_mode_gpu_END (1)
#define SOC_PMCTRL_VS_SVFD_CTRL2_GPU_svfs_cpm_threshold_r_gpu_START (2)
#define SOC_PMCTRL_VS_SVFD_CTRL2_GPU_svfs_cpm_threshold_r_gpu_END (7)
#define SOC_PMCTRL_VS_SVFD_CTRL2_GPU_svfs_cpm_threshold_f_gpu_START (8)
#define SOC_PMCTRL_VS_SVFD_CTRL2_GPU_svfs_cpm_threshold_f_gpu_END (13)
#define SOC_PMCTRL_VS_SVFD_CTRL2_GPU_vol_drop_en_gpu_START (14)
#define SOC_PMCTRL_VS_SVFD_CTRL2_GPU_vol_drop_en_gpu_END (14)
#define SOC_PMCTRL_VS_SVFD_CTRL2_GPU_vs_svfd_ctrl2_gpu_msk_START (16)
#define SOC_PMCTRL_VS_SVFD_CTRL2_GPU_vs_svfd_ctrl2_gpu_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_cpm_data_r_gpu : 8;
        unsigned int svfd_cpm_data_f_gpu : 8;
        unsigned int svfd_dll_lock_gpu : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_PMCTRL_VS_CPM_DATA_STAT_GPU_UNION;
#endif
#define SOC_PMCTRL_VS_CPM_DATA_STAT_GPU_svfd_cpm_data_r_gpu_START (0)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_GPU_svfd_cpm_data_r_gpu_END (7)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_GPU_svfd_cpm_data_f_gpu_START (8)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_GPU_svfd_cpm_data_f_gpu_END (15)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_GPU_svfd_dll_lock_gpu_START (16)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_GPU_svfd_dll_lock_gpu_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_test_out_ffs_gpu : 4;
        unsigned int svfd_test_out_cpm_gpu : 4;
        unsigned int svfd_glitch_result_gpu : 1;
        unsigned int idle_low_freq_en_gpu : 1;
        unsigned int vbat_drop_protect_ind_gpu : 1;
        unsigned int reserved : 21;
    } reg;
} SOC_PMCTRL_VS_TEST_STAT_GPU_UNION;
#endif
#define SOC_PMCTRL_VS_TEST_STAT_GPU_svfd_test_out_ffs_gpu_START (0)
#define SOC_PMCTRL_VS_TEST_STAT_GPU_svfd_test_out_ffs_gpu_END (3)
#define SOC_PMCTRL_VS_TEST_STAT_GPU_svfd_test_out_cpm_gpu_START (4)
#define SOC_PMCTRL_VS_TEST_STAT_GPU_svfd_test_out_cpm_gpu_END (7)
#define SOC_PMCTRL_VS_TEST_STAT_GPU_svfd_glitch_result_gpu_START (8)
#define SOC_PMCTRL_VS_TEST_STAT_GPU_svfd_glitch_result_gpu_END (8)
#define SOC_PMCTRL_VS_TEST_STAT_GPU_idle_low_freq_en_gpu_START (9)
#define SOC_PMCTRL_VS_TEST_STAT_GPU_idle_low_freq_en_gpu_END (9)
#define SOC_PMCTRL_VS_TEST_STAT_GPU_vbat_drop_protect_ind_gpu_START (10)
#define SOC_PMCTRL_VS_TEST_STAT_GPU_vbat_drop_protect_ind_gpu_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bootrom_flag : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BOOTROMFLAG_UNION;
#endif
#define SOC_PMCTRL_BOOTROMFLAG_bootrom_flag_START (0)
#define SOC_PMCTRL_BOOTROMFLAG_bootrom_flag_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_hpm_en : 1;
        unsigned int reserved_0 : 3;
        unsigned int peri_hpmx_en : 1;
        unsigned int reserved_1 : 3;
        unsigned int peri_hpm_opc_min_en : 1;
        unsigned int reserved_2 : 3;
        unsigned int peri_hpmx_opc_min_en : 1;
        unsigned int reserved_3 : 19;
    } reg;
} SOC_PMCTRL_PERIHPMEN_UNION;
#endif
#define SOC_PMCTRL_PERIHPMEN_peri_hpm_en_START (0)
#define SOC_PMCTRL_PERIHPMEN_peri_hpm_en_END (0)
#define SOC_PMCTRL_PERIHPMEN_peri_hpmx_en_START (4)
#define SOC_PMCTRL_PERIHPMEN_peri_hpmx_en_END (4)
#define SOC_PMCTRL_PERIHPMEN_peri_hpm_opc_min_en_START (8)
#define SOC_PMCTRL_PERIHPMEN_peri_hpm_opc_min_en_END (8)
#define SOC_PMCTRL_PERIHPMEN_peri_hpmx_opc_min_en_START (12)
#define SOC_PMCTRL_PERIHPMEN_peri_hpmx_opc_min_en_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_hpm_clk_div : 6;
        unsigned int reserved_0 : 2;
        unsigned int peri_hpm_opc_min : 10;
        unsigned int reserved_1 : 2;
        unsigned int peri_hpmx_opc_min : 10;
        unsigned int reserved_2 : 2;
    } reg;
} SOC_PMCTRL_PERIHPMCLKDIV_UNION;
#endif
#define SOC_PMCTRL_PERIHPMCLKDIV_peri_hpm_clk_div_START (0)
#define SOC_PMCTRL_PERIHPMCLKDIV_peri_hpm_clk_div_END (5)
#define SOC_PMCTRL_PERIHPMCLKDIV_peri_hpm_opc_min_START (8)
#define SOC_PMCTRL_PERIHPMCLKDIV_peri_hpm_opc_min_END (17)
#define SOC_PMCTRL_PERIHPMCLKDIV_peri_hpmx_opc_min_START (20)
#define SOC_PMCTRL_PERIHPMCLKDIV_peri_hpmx_opc_min_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_hpm_opc : 10;
        unsigned int reserved_0 : 2;
        unsigned int peri_hpm_opc_vld : 1;
        unsigned int reserved_1 : 3;
        unsigned int peri_hpmx_opc : 10;
        unsigned int reserved_2 : 2;
        unsigned int peri_hpmx_opc_vld : 1;
        unsigned int reserved_3 : 3;
    } reg;
} SOC_PMCTRL_PERIHPMOPC_UNION;
#endif
#define SOC_PMCTRL_PERIHPMOPC_peri_hpm_opc_START (0)
#define SOC_PMCTRL_PERIHPMOPC_peri_hpm_opc_END (9)
#define SOC_PMCTRL_PERIHPMOPC_peri_hpm_opc_vld_START (12)
#define SOC_PMCTRL_PERIHPMOPC_peri_hpm_opc_vld_END (12)
#define SOC_PMCTRL_PERIHPMOPC_peri_hpmx_opc_START (16)
#define SOC_PMCTRL_PERIHPMOPC_peri_hpmx_opc_END (25)
#define SOC_PMCTRL_PERIHPMOPC_peri_hpmx_opc_vld_START (28)
#define SOC_PMCTRL_PERIHPMOPC_peri_hpmx_opc_vld_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uce_prog_stat_d : 8;
        unsigned int uce_prog_stat_c : 8;
        unsigned int uce_prog_stat_b : 8;
        unsigned int uce_prog_stat_a : 8;
    } reg;
} SOC_PMCTRL_PERI_STAT8_UNION;
#endif
#define SOC_PMCTRL_PERI_STAT8_uce_prog_stat_d_START (0)
#define SOC_PMCTRL_PERI_STAT8_uce_prog_stat_d_END (7)
#define SOC_PMCTRL_PERI_STAT8_uce_prog_stat_c_START (8)
#define SOC_PMCTRL_PERI_STAT8_uce_prog_stat_c_END (15)
#define SOC_PMCTRL_PERI_STAT8_uce_prog_stat_b_START (16)
#define SOC_PMCTRL_PERI_STAT8_uce_prog_stat_b_END (23)
#define SOC_PMCTRL_PERI_STAT8_uce_prog_stat_a_START (24)
#define SOC_PMCTRL_PERI_STAT8_uce_prog_stat_a_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT9_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_info_pmc : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT12_UNION;
#endif
#define SOC_PMCTRL_PERI_STAT12_dbg_info_pmc_START (0)
#define SOC_PMCTRL_PERI_STAT12_dbg_info_pmc_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_info_crg : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT13_UNION;
#endif
#define SOC_PMCTRL_PERI_STAT13_dbg_info_crg_START (0)
#define SOC_PMCTRL_PERI_STAT13_dbg_info_crg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT14_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT15_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll0_en_cfg : 1;
        unsigned int apll0_bp_cfg : 1;
        unsigned int apll0_refdiv_cfg : 6;
        unsigned int apll0_fbdiv_cfg : 12;
        unsigned int apll0_postdiv1_cfg : 3;
        unsigned int apll0_postdiv2_cfg : 3;
        unsigned int apll0_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL0CTRL0_UNION;
#endif
#define SOC_PMCTRL_APLL0CTRL0_apll0_en_cfg_START (0)
#define SOC_PMCTRL_APLL0CTRL0_apll0_en_cfg_END (0)
#define SOC_PMCTRL_APLL0CTRL0_apll0_bp_cfg_START (1)
#define SOC_PMCTRL_APLL0CTRL0_apll0_bp_cfg_END (1)
#define SOC_PMCTRL_APLL0CTRL0_apll0_refdiv_cfg_START (2)
#define SOC_PMCTRL_APLL0CTRL0_apll0_refdiv_cfg_END (7)
#define SOC_PMCTRL_APLL0CTRL0_apll0_fbdiv_cfg_START (8)
#define SOC_PMCTRL_APLL0CTRL0_apll0_fbdiv_cfg_END (19)
#define SOC_PMCTRL_APLL0CTRL0_apll0_postdiv1_cfg_START (20)
#define SOC_PMCTRL_APLL0CTRL0_apll0_postdiv1_cfg_END (22)
#define SOC_PMCTRL_APLL0CTRL0_apll0_postdiv2_cfg_START (23)
#define SOC_PMCTRL_APLL0CTRL0_apll0_postdiv2_cfg_END (25)
#define SOC_PMCTRL_APLL0CTRL0_apll0_lock_START (26)
#define SOC_PMCTRL_APLL0CTRL0_apll0_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll0_fracdiv_cfg : 24;
        unsigned int apll0_int_mod_cfg : 1;
        unsigned int apll0_cfg_vld_cfg : 1;
        unsigned int gt_clk_apll0_cfg : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL0CTRL1_UNION;
#endif
#define SOC_PMCTRL_APLL0CTRL1_apll0_fracdiv_cfg_START (0)
#define SOC_PMCTRL_APLL0CTRL1_apll0_fracdiv_cfg_END (23)
#define SOC_PMCTRL_APLL0CTRL1_apll0_int_mod_cfg_START (24)
#define SOC_PMCTRL_APLL0CTRL1_apll0_int_mod_cfg_END (24)
#define SOC_PMCTRL_APLL0CTRL1_apll0_cfg_vld_cfg_START (25)
#define SOC_PMCTRL_APLL0CTRL1_apll0_cfg_vld_cfg_END (25)
#define SOC_PMCTRL_APLL0CTRL1_gt_clk_apll0_cfg_START (26)
#define SOC_PMCTRL_APLL0CTRL1_gt_clk_apll0_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll0_en_stat : 1;
        unsigned int apll0_bp_stat : 1;
        unsigned int apll0_refdiv_stat : 6;
        unsigned int apll0_fbdiv_stat : 12;
        unsigned int apll0_postdiv1_stat : 3;
        unsigned int apll0_postdiv2_stat : 3;
        unsigned int reserved : 6;
    } reg;
} SOC_PMCTRL_APLL0CTRL0_STAT_UNION;
#endif
#define SOC_PMCTRL_APLL0CTRL0_STAT_apll0_en_stat_START (0)
#define SOC_PMCTRL_APLL0CTRL0_STAT_apll0_en_stat_END (0)
#define SOC_PMCTRL_APLL0CTRL0_STAT_apll0_bp_stat_START (1)
#define SOC_PMCTRL_APLL0CTRL0_STAT_apll0_bp_stat_END (1)
#define SOC_PMCTRL_APLL0CTRL0_STAT_apll0_refdiv_stat_START (2)
#define SOC_PMCTRL_APLL0CTRL0_STAT_apll0_refdiv_stat_END (7)
#define SOC_PMCTRL_APLL0CTRL0_STAT_apll0_fbdiv_stat_START (8)
#define SOC_PMCTRL_APLL0CTRL0_STAT_apll0_fbdiv_stat_END (19)
#define SOC_PMCTRL_APLL0CTRL0_STAT_apll0_postdiv1_stat_START (20)
#define SOC_PMCTRL_APLL0CTRL0_STAT_apll0_postdiv1_stat_END (22)
#define SOC_PMCTRL_APLL0CTRL0_STAT_apll0_postdiv2_stat_START (23)
#define SOC_PMCTRL_APLL0CTRL0_STAT_apll0_postdiv2_stat_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll0_fracdiv_stat : 24;
        unsigned int apll0_int_mod_stat : 1;
        unsigned int apll0_cfg_vld_stat : 1;
        unsigned int gt_clk_apll0_stat : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL0CTRL1_STAT_UNION;
#endif
#define SOC_PMCTRL_APLL0CTRL1_STAT_apll0_fracdiv_stat_START (0)
#define SOC_PMCTRL_APLL0CTRL1_STAT_apll0_fracdiv_stat_END (23)
#define SOC_PMCTRL_APLL0CTRL1_STAT_apll0_int_mod_stat_START (24)
#define SOC_PMCTRL_APLL0CTRL1_STAT_apll0_int_mod_stat_END (24)
#define SOC_PMCTRL_APLL0CTRL1_STAT_apll0_cfg_vld_stat_START (25)
#define SOC_PMCTRL_APLL0CTRL1_STAT_apll0_cfg_vld_stat_END (25)
#define SOC_PMCTRL_APLL0CTRL1_STAT_gt_clk_apll0_stat_START (26)
#define SOC_PMCTRL_APLL0CTRL1_STAT_gt_clk_apll0_stat_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll1_en_cfg : 1;
        unsigned int apll1_bp_cfg : 1;
        unsigned int apll1_refdiv_cfg : 6;
        unsigned int apll1_fbdiv_cfg : 12;
        unsigned int apll1_postdiv1_cfg : 3;
        unsigned int apll1_postdiv2_cfg : 3;
        unsigned int apll1_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL1CTRL0_UNION;
#endif
#define SOC_PMCTRL_APLL1CTRL0_apll1_en_cfg_START (0)
#define SOC_PMCTRL_APLL1CTRL0_apll1_en_cfg_END (0)
#define SOC_PMCTRL_APLL1CTRL0_apll1_bp_cfg_START (1)
#define SOC_PMCTRL_APLL1CTRL0_apll1_bp_cfg_END (1)
#define SOC_PMCTRL_APLL1CTRL0_apll1_refdiv_cfg_START (2)
#define SOC_PMCTRL_APLL1CTRL0_apll1_refdiv_cfg_END (7)
#define SOC_PMCTRL_APLL1CTRL0_apll1_fbdiv_cfg_START (8)
#define SOC_PMCTRL_APLL1CTRL0_apll1_fbdiv_cfg_END (19)
#define SOC_PMCTRL_APLL1CTRL0_apll1_postdiv1_cfg_START (20)
#define SOC_PMCTRL_APLL1CTRL0_apll1_postdiv1_cfg_END (22)
#define SOC_PMCTRL_APLL1CTRL0_apll1_postdiv2_cfg_START (23)
#define SOC_PMCTRL_APLL1CTRL0_apll1_postdiv2_cfg_END (25)
#define SOC_PMCTRL_APLL1CTRL0_apll1_lock_START (26)
#define SOC_PMCTRL_APLL1CTRL0_apll1_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll1_fracdiv_cfg : 24;
        unsigned int apll1_int_mod_cfg : 1;
        unsigned int apll1_cfg_vld_cfg : 1;
        unsigned int gt_clk_apll1_cfg : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL1CTRL1_UNION;
#endif
#define SOC_PMCTRL_APLL1CTRL1_apll1_fracdiv_cfg_START (0)
#define SOC_PMCTRL_APLL1CTRL1_apll1_fracdiv_cfg_END (23)
#define SOC_PMCTRL_APLL1CTRL1_apll1_int_mod_cfg_START (24)
#define SOC_PMCTRL_APLL1CTRL1_apll1_int_mod_cfg_END (24)
#define SOC_PMCTRL_APLL1CTRL1_apll1_cfg_vld_cfg_START (25)
#define SOC_PMCTRL_APLL1CTRL1_apll1_cfg_vld_cfg_END (25)
#define SOC_PMCTRL_APLL1CTRL1_gt_clk_apll1_cfg_START (26)
#define SOC_PMCTRL_APLL1CTRL1_gt_clk_apll1_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll1_en_stat : 1;
        unsigned int apll1_bp_stat : 1;
        unsigned int apll1_refdiv_stat : 6;
        unsigned int apll1_fbdiv_stat : 12;
        unsigned int apll1_postdiv1_stat : 3;
        unsigned int apll1_postdiv2_stat : 3;
        unsigned int reserved : 6;
    } reg;
} SOC_PMCTRL_APLL1CTRL0_STAT_UNION;
#endif
#define SOC_PMCTRL_APLL1CTRL0_STAT_apll1_en_stat_START (0)
#define SOC_PMCTRL_APLL1CTRL0_STAT_apll1_en_stat_END (0)
#define SOC_PMCTRL_APLL1CTRL0_STAT_apll1_bp_stat_START (1)
#define SOC_PMCTRL_APLL1CTRL0_STAT_apll1_bp_stat_END (1)
#define SOC_PMCTRL_APLL1CTRL0_STAT_apll1_refdiv_stat_START (2)
#define SOC_PMCTRL_APLL1CTRL0_STAT_apll1_refdiv_stat_END (7)
#define SOC_PMCTRL_APLL1CTRL0_STAT_apll1_fbdiv_stat_START (8)
#define SOC_PMCTRL_APLL1CTRL0_STAT_apll1_fbdiv_stat_END (19)
#define SOC_PMCTRL_APLL1CTRL0_STAT_apll1_postdiv1_stat_START (20)
#define SOC_PMCTRL_APLL1CTRL0_STAT_apll1_postdiv1_stat_END (22)
#define SOC_PMCTRL_APLL1CTRL0_STAT_apll1_postdiv2_stat_START (23)
#define SOC_PMCTRL_APLL1CTRL0_STAT_apll1_postdiv2_stat_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll1_fracdiv_stat : 24;
        unsigned int apll1_int_mod_stat : 1;
        unsigned int apll1_cfg_vld_stat : 1;
        unsigned int gt_clk_apll1_stat : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL1CTRL1_STAT_UNION;
#endif
#define SOC_PMCTRL_APLL1CTRL1_STAT_apll1_fracdiv_stat_START (0)
#define SOC_PMCTRL_APLL1CTRL1_STAT_apll1_fracdiv_stat_END (23)
#define SOC_PMCTRL_APLL1CTRL1_STAT_apll1_int_mod_stat_START (24)
#define SOC_PMCTRL_APLL1CTRL1_STAT_apll1_int_mod_stat_END (24)
#define SOC_PMCTRL_APLL1CTRL1_STAT_apll1_cfg_vld_stat_START (25)
#define SOC_PMCTRL_APLL1CTRL1_STAT_apll1_cfg_vld_stat_END (25)
#define SOC_PMCTRL_APLL1CTRL1_STAT_gt_clk_apll1_stat_START (26)
#define SOC_PMCTRL_APLL1CTRL1_STAT_gt_clk_apll1_stat_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll2_en_cfg : 1;
        unsigned int apll2_bp_cfg : 1;
        unsigned int apll2_refdiv_cfg : 6;
        unsigned int apll2_fbdiv_cfg : 12;
        unsigned int apll2_postdiv1_cfg : 3;
        unsigned int apll2_postdiv2_cfg : 3;
        unsigned int apll2_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL2CTRL0_UNION;
#endif
#define SOC_PMCTRL_APLL2CTRL0_apll2_en_cfg_START (0)
#define SOC_PMCTRL_APLL2CTRL0_apll2_en_cfg_END (0)
#define SOC_PMCTRL_APLL2CTRL0_apll2_bp_cfg_START (1)
#define SOC_PMCTRL_APLL2CTRL0_apll2_bp_cfg_END (1)
#define SOC_PMCTRL_APLL2CTRL0_apll2_refdiv_cfg_START (2)
#define SOC_PMCTRL_APLL2CTRL0_apll2_refdiv_cfg_END (7)
#define SOC_PMCTRL_APLL2CTRL0_apll2_fbdiv_cfg_START (8)
#define SOC_PMCTRL_APLL2CTRL0_apll2_fbdiv_cfg_END (19)
#define SOC_PMCTRL_APLL2CTRL0_apll2_postdiv1_cfg_START (20)
#define SOC_PMCTRL_APLL2CTRL0_apll2_postdiv1_cfg_END (22)
#define SOC_PMCTRL_APLL2CTRL0_apll2_postdiv2_cfg_START (23)
#define SOC_PMCTRL_APLL2CTRL0_apll2_postdiv2_cfg_END (25)
#define SOC_PMCTRL_APLL2CTRL0_apll2_lock_START (26)
#define SOC_PMCTRL_APLL2CTRL0_apll2_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll2_fracdiv_cfg : 24;
        unsigned int apll2_int_mod_cfg : 1;
        unsigned int apll2_cfg_vld_cfg : 1;
        unsigned int gt_clk_apll2_cfg : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL2CTRL1_UNION;
#endif
#define SOC_PMCTRL_APLL2CTRL1_apll2_fracdiv_cfg_START (0)
#define SOC_PMCTRL_APLL2CTRL1_apll2_fracdiv_cfg_END (23)
#define SOC_PMCTRL_APLL2CTRL1_apll2_int_mod_cfg_START (24)
#define SOC_PMCTRL_APLL2CTRL1_apll2_int_mod_cfg_END (24)
#define SOC_PMCTRL_APLL2CTRL1_apll2_cfg_vld_cfg_START (25)
#define SOC_PMCTRL_APLL2CTRL1_apll2_cfg_vld_cfg_END (25)
#define SOC_PMCTRL_APLL2CTRL1_gt_clk_apll2_cfg_START (26)
#define SOC_PMCTRL_APLL2CTRL1_gt_clk_apll2_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll2_en_stat : 1;
        unsigned int apll2_bp_stat : 1;
        unsigned int apll2_refdiv_stat : 6;
        unsigned int apll2_fbdiv_stat : 12;
        unsigned int apll2_postdiv1_stat : 3;
        unsigned int apll2_postdiv2_stat : 3;
        unsigned int reserved : 6;
    } reg;
} SOC_PMCTRL_APLL2CTRL0_STAT_UNION;
#endif
#define SOC_PMCTRL_APLL2CTRL0_STAT_apll2_en_stat_START (0)
#define SOC_PMCTRL_APLL2CTRL0_STAT_apll2_en_stat_END (0)
#define SOC_PMCTRL_APLL2CTRL0_STAT_apll2_bp_stat_START (1)
#define SOC_PMCTRL_APLL2CTRL0_STAT_apll2_bp_stat_END (1)
#define SOC_PMCTRL_APLL2CTRL0_STAT_apll2_refdiv_stat_START (2)
#define SOC_PMCTRL_APLL2CTRL0_STAT_apll2_refdiv_stat_END (7)
#define SOC_PMCTRL_APLL2CTRL0_STAT_apll2_fbdiv_stat_START (8)
#define SOC_PMCTRL_APLL2CTRL0_STAT_apll2_fbdiv_stat_END (19)
#define SOC_PMCTRL_APLL2CTRL0_STAT_apll2_postdiv1_stat_START (20)
#define SOC_PMCTRL_APLL2CTRL0_STAT_apll2_postdiv1_stat_END (22)
#define SOC_PMCTRL_APLL2CTRL0_STAT_apll2_postdiv2_stat_START (23)
#define SOC_PMCTRL_APLL2CTRL0_STAT_apll2_postdiv2_stat_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll2_fracdiv_stat : 24;
        unsigned int apll2_int_mod_stat : 1;
        unsigned int apll2_cfg_vld_stat : 1;
        unsigned int gt_clk_apll2_stat : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL2CTRL1_STAT_UNION;
#endif
#define SOC_PMCTRL_APLL2CTRL1_STAT_apll2_fracdiv_stat_START (0)
#define SOC_PMCTRL_APLL2CTRL1_STAT_apll2_fracdiv_stat_END (23)
#define SOC_PMCTRL_APLL2CTRL1_STAT_apll2_int_mod_stat_START (24)
#define SOC_PMCTRL_APLL2CTRL1_STAT_apll2_int_mod_stat_END (24)
#define SOC_PMCTRL_APLL2CTRL1_STAT_apll2_cfg_vld_stat_START (25)
#define SOC_PMCTRL_APLL2CTRL1_STAT_apll2_cfg_vld_stat_END (25)
#define SOC_PMCTRL_APLL2CTRL1_STAT_gt_clk_apll2_stat_START (26)
#define SOC_PMCTRL_APLL2CTRL1_STAT_gt_clk_apll2_stat_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll3_en_cfg : 1;
        unsigned int apll3_bp_cfg : 1;
        unsigned int apll3_refdiv_cfg : 6;
        unsigned int apll3_fbdiv_cfg : 12;
        unsigned int apll3_postdiv1_cfg : 3;
        unsigned int apll3_postdiv2_cfg : 3;
        unsigned int apll3_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL3CTRL0_UNION;
#endif
#define SOC_PMCTRL_APLL3CTRL0_apll3_en_cfg_START (0)
#define SOC_PMCTRL_APLL3CTRL0_apll3_en_cfg_END (0)
#define SOC_PMCTRL_APLL3CTRL0_apll3_bp_cfg_START (1)
#define SOC_PMCTRL_APLL3CTRL0_apll3_bp_cfg_END (1)
#define SOC_PMCTRL_APLL3CTRL0_apll3_refdiv_cfg_START (2)
#define SOC_PMCTRL_APLL3CTRL0_apll3_refdiv_cfg_END (7)
#define SOC_PMCTRL_APLL3CTRL0_apll3_fbdiv_cfg_START (8)
#define SOC_PMCTRL_APLL3CTRL0_apll3_fbdiv_cfg_END (19)
#define SOC_PMCTRL_APLL3CTRL0_apll3_postdiv1_cfg_START (20)
#define SOC_PMCTRL_APLL3CTRL0_apll3_postdiv1_cfg_END (22)
#define SOC_PMCTRL_APLL3CTRL0_apll3_postdiv2_cfg_START (23)
#define SOC_PMCTRL_APLL3CTRL0_apll3_postdiv2_cfg_END (25)
#define SOC_PMCTRL_APLL3CTRL0_apll3_lock_START (26)
#define SOC_PMCTRL_APLL3CTRL0_apll3_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll3_fracdiv_cfg : 24;
        unsigned int apll3_int_mod_cfg : 1;
        unsigned int apll3_cfg_vld_cfg : 1;
        unsigned int gt_clk_apll3_cfg : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL3CTRL1_UNION;
#endif
#define SOC_PMCTRL_APLL3CTRL1_apll3_fracdiv_cfg_START (0)
#define SOC_PMCTRL_APLL3CTRL1_apll3_fracdiv_cfg_END (23)
#define SOC_PMCTRL_APLL3CTRL1_apll3_int_mod_cfg_START (24)
#define SOC_PMCTRL_APLL3CTRL1_apll3_int_mod_cfg_END (24)
#define SOC_PMCTRL_APLL3CTRL1_apll3_cfg_vld_cfg_START (25)
#define SOC_PMCTRL_APLL3CTRL1_apll3_cfg_vld_cfg_END (25)
#define SOC_PMCTRL_APLL3CTRL1_gt_clk_apll3_cfg_START (26)
#define SOC_PMCTRL_APLL3CTRL1_gt_clk_apll3_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll3_en_stat : 1;
        unsigned int apll3_bp_stat : 1;
        unsigned int apll3_refdiv_stat : 6;
        unsigned int apll3_fbdiv_stat : 12;
        unsigned int apll3_postdiv1_stat : 3;
        unsigned int apll3_postdiv2_stat : 3;
        unsigned int reserved : 6;
    } reg;
} SOC_PMCTRL_APLL3CTRL0_STAT_UNION;
#endif
#define SOC_PMCTRL_APLL3CTRL0_STAT_apll3_en_stat_START (0)
#define SOC_PMCTRL_APLL3CTRL0_STAT_apll3_en_stat_END (0)
#define SOC_PMCTRL_APLL3CTRL0_STAT_apll3_bp_stat_START (1)
#define SOC_PMCTRL_APLL3CTRL0_STAT_apll3_bp_stat_END (1)
#define SOC_PMCTRL_APLL3CTRL0_STAT_apll3_refdiv_stat_START (2)
#define SOC_PMCTRL_APLL3CTRL0_STAT_apll3_refdiv_stat_END (7)
#define SOC_PMCTRL_APLL3CTRL0_STAT_apll3_fbdiv_stat_START (8)
#define SOC_PMCTRL_APLL3CTRL0_STAT_apll3_fbdiv_stat_END (19)
#define SOC_PMCTRL_APLL3CTRL0_STAT_apll3_postdiv1_stat_START (20)
#define SOC_PMCTRL_APLL3CTRL0_STAT_apll3_postdiv1_stat_END (22)
#define SOC_PMCTRL_APLL3CTRL0_STAT_apll3_postdiv2_stat_START (23)
#define SOC_PMCTRL_APLL3CTRL0_STAT_apll3_postdiv2_stat_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll3_fracdiv_stat : 24;
        unsigned int apll3_int_mod_stat : 1;
        unsigned int apll3_cfg_vld_stat : 1;
        unsigned int gt_clk_apll3_stat : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL3CTRL1_STAT_UNION;
#endif
#define SOC_PMCTRL_APLL3CTRL1_STAT_apll3_fracdiv_stat_START (0)
#define SOC_PMCTRL_APLL3CTRL1_STAT_apll3_fracdiv_stat_END (23)
#define SOC_PMCTRL_APLL3CTRL1_STAT_apll3_int_mod_stat_START (24)
#define SOC_PMCTRL_APLL3CTRL1_STAT_apll3_int_mod_stat_END (24)
#define SOC_PMCTRL_APLL3CTRL1_STAT_apll3_cfg_vld_stat_START (25)
#define SOC_PMCTRL_APLL3CTRL1_STAT_apll3_cfg_vld_stat_END (25)
#define SOC_PMCTRL_APLL3CTRL1_STAT_gt_clk_apll3_stat_START (26)
#define SOC_PMCTRL_APLL3CTRL1_STAT_gt_clk_apll3_stat_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll4_en_cfg : 1;
        unsigned int apll4_bp_cfg : 1;
        unsigned int apll4_refdiv_cfg : 6;
        unsigned int apll4_fbdiv_cfg : 12;
        unsigned int apll4_postdiv1_cfg : 3;
        unsigned int apll4_postdiv2_cfg : 3;
        unsigned int apll4_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL4CTRL0_UNION;
#endif
#define SOC_PMCTRL_APLL4CTRL0_apll4_en_cfg_START (0)
#define SOC_PMCTRL_APLL4CTRL0_apll4_en_cfg_END (0)
#define SOC_PMCTRL_APLL4CTRL0_apll4_bp_cfg_START (1)
#define SOC_PMCTRL_APLL4CTRL0_apll4_bp_cfg_END (1)
#define SOC_PMCTRL_APLL4CTRL0_apll4_refdiv_cfg_START (2)
#define SOC_PMCTRL_APLL4CTRL0_apll4_refdiv_cfg_END (7)
#define SOC_PMCTRL_APLL4CTRL0_apll4_fbdiv_cfg_START (8)
#define SOC_PMCTRL_APLL4CTRL0_apll4_fbdiv_cfg_END (19)
#define SOC_PMCTRL_APLL4CTRL0_apll4_postdiv1_cfg_START (20)
#define SOC_PMCTRL_APLL4CTRL0_apll4_postdiv1_cfg_END (22)
#define SOC_PMCTRL_APLL4CTRL0_apll4_postdiv2_cfg_START (23)
#define SOC_PMCTRL_APLL4CTRL0_apll4_postdiv2_cfg_END (25)
#define SOC_PMCTRL_APLL4CTRL0_apll4_lock_START (26)
#define SOC_PMCTRL_APLL4CTRL0_apll4_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll4_fracdiv_cfg : 24;
        unsigned int apll4_int_mod_cfg : 1;
        unsigned int apll4_cfg_vld_cfg : 1;
        unsigned int gt_clk_apll4_cfg : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL4CTRL1_UNION;
#endif
#define SOC_PMCTRL_APLL4CTRL1_apll4_fracdiv_cfg_START (0)
#define SOC_PMCTRL_APLL4CTRL1_apll4_fracdiv_cfg_END (23)
#define SOC_PMCTRL_APLL4CTRL1_apll4_int_mod_cfg_START (24)
#define SOC_PMCTRL_APLL4CTRL1_apll4_int_mod_cfg_END (24)
#define SOC_PMCTRL_APLL4CTRL1_apll4_cfg_vld_cfg_START (25)
#define SOC_PMCTRL_APLL4CTRL1_apll4_cfg_vld_cfg_END (25)
#define SOC_PMCTRL_APLL4CTRL1_gt_clk_apll4_cfg_START (26)
#define SOC_PMCTRL_APLL4CTRL1_gt_clk_apll4_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll4_en_stat : 1;
        unsigned int apll4_bp_stat : 1;
        unsigned int apll4_refdiv_stat : 6;
        unsigned int apll4_fbdiv_stat : 12;
        unsigned int apll4_postdiv1_stat : 3;
        unsigned int apll4_postdiv2_stat : 3;
        unsigned int reserved : 6;
    } reg;
} SOC_PMCTRL_APLL4CTRL0_STAT_UNION;
#endif
#define SOC_PMCTRL_APLL4CTRL0_STAT_apll4_en_stat_START (0)
#define SOC_PMCTRL_APLL4CTRL0_STAT_apll4_en_stat_END (0)
#define SOC_PMCTRL_APLL4CTRL0_STAT_apll4_bp_stat_START (1)
#define SOC_PMCTRL_APLL4CTRL0_STAT_apll4_bp_stat_END (1)
#define SOC_PMCTRL_APLL4CTRL0_STAT_apll4_refdiv_stat_START (2)
#define SOC_PMCTRL_APLL4CTRL0_STAT_apll4_refdiv_stat_END (7)
#define SOC_PMCTRL_APLL4CTRL0_STAT_apll4_fbdiv_stat_START (8)
#define SOC_PMCTRL_APLL4CTRL0_STAT_apll4_fbdiv_stat_END (19)
#define SOC_PMCTRL_APLL4CTRL0_STAT_apll4_postdiv1_stat_START (20)
#define SOC_PMCTRL_APLL4CTRL0_STAT_apll4_postdiv1_stat_END (22)
#define SOC_PMCTRL_APLL4CTRL0_STAT_apll4_postdiv2_stat_START (23)
#define SOC_PMCTRL_APLL4CTRL0_STAT_apll4_postdiv2_stat_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll4_fracdiv_stat : 24;
        unsigned int apll4_int_mod_stat : 1;
        unsigned int apll4_cfg_vld_stat : 1;
        unsigned int gt_clk_apll4_stat : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL4CTRL1_STAT_UNION;
#endif
#define SOC_PMCTRL_APLL4CTRL1_STAT_apll4_fracdiv_stat_START (0)
#define SOC_PMCTRL_APLL4CTRL1_STAT_apll4_fracdiv_stat_END (23)
#define SOC_PMCTRL_APLL4CTRL1_STAT_apll4_int_mod_stat_START (24)
#define SOC_PMCTRL_APLL4CTRL1_STAT_apll4_int_mod_stat_END (24)
#define SOC_PMCTRL_APLL4CTRL1_STAT_apll4_cfg_vld_stat_START (25)
#define SOC_PMCTRL_APLL4CTRL1_STAT_apll4_cfg_vld_stat_END (25)
#define SOC_PMCTRL_APLL4CTRL1_STAT_gt_clk_apll4_stat_START (26)
#define SOC_PMCTRL_APLL4CTRL1_STAT_gt_clk_apll4_stat_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll5_en_cfg : 1;
        unsigned int apll5_bp_cfg : 1;
        unsigned int apll5_refdiv_cfg : 6;
        unsigned int apll5_fbdiv_cfg : 12;
        unsigned int apll5_postdiv1_cfg : 3;
        unsigned int apll5_postdiv2_cfg : 3;
        unsigned int apll5_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL5CTRL0_UNION;
#endif
#define SOC_PMCTRL_APLL5CTRL0_apll5_en_cfg_START (0)
#define SOC_PMCTRL_APLL5CTRL0_apll5_en_cfg_END (0)
#define SOC_PMCTRL_APLL5CTRL0_apll5_bp_cfg_START (1)
#define SOC_PMCTRL_APLL5CTRL0_apll5_bp_cfg_END (1)
#define SOC_PMCTRL_APLL5CTRL0_apll5_refdiv_cfg_START (2)
#define SOC_PMCTRL_APLL5CTRL0_apll5_refdiv_cfg_END (7)
#define SOC_PMCTRL_APLL5CTRL0_apll5_fbdiv_cfg_START (8)
#define SOC_PMCTRL_APLL5CTRL0_apll5_fbdiv_cfg_END (19)
#define SOC_PMCTRL_APLL5CTRL0_apll5_postdiv1_cfg_START (20)
#define SOC_PMCTRL_APLL5CTRL0_apll5_postdiv1_cfg_END (22)
#define SOC_PMCTRL_APLL5CTRL0_apll5_postdiv2_cfg_START (23)
#define SOC_PMCTRL_APLL5CTRL0_apll5_postdiv2_cfg_END (25)
#define SOC_PMCTRL_APLL5CTRL0_apll5_lock_START (26)
#define SOC_PMCTRL_APLL5CTRL0_apll5_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll5_fracdiv_cfg : 24;
        unsigned int apll5_int_mod_cfg : 1;
        unsigned int apll5_cfg_vld_cfg : 1;
        unsigned int gt_clk_apll5_cfg : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL5CTRL1_UNION;
#endif
#define SOC_PMCTRL_APLL5CTRL1_apll5_fracdiv_cfg_START (0)
#define SOC_PMCTRL_APLL5CTRL1_apll5_fracdiv_cfg_END (23)
#define SOC_PMCTRL_APLL5CTRL1_apll5_int_mod_cfg_START (24)
#define SOC_PMCTRL_APLL5CTRL1_apll5_int_mod_cfg_END (24)
#define SOC_PMCTRL_APLL5CTRL1_apll5_cfg_vld_cfg_START (25)
#define SOC_PMCTRL_APLL5CTRL1_apll5_cfg_vld_cfg_END (25)
#define SOC_PMCTRL_APLL5CTRL1_gt_clk_apll5_cfg_START (26)
#define SOC_PMCTRL_APLL5CTRL1_gt_clk_apll5_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll5_en_stat : 1;
        unsigned int apll5_bp_stat : 1;
        unsigned int apll5_refdiv_stat : 6;
        unsigned int apll5_fbdiv_stat : 12;
        unsigned int apll5_postdiv1_stat : 3;
        unsigned int apll5_postdiv2_stat : 3;
        unsigned int reserved : 6;
    } reg;
} SOC_PMCTRL_APLL5CTRL0_STAT_UNION;
#endif
#define SOC_PMCTRL_APLL5CTRL0_STAT_apll5_en_stat_START (0)
#define SOC_PMCTRL_APLL5CTRL0_STAT_apll5_en_stat_END (0)
#define SOC_PMCTRL_APLL5CTRL0_STAT_apll5_bp_stat_START (1)
#define SOC_PMCTRL_APLL5CTRL0_STAT_apll5_bp_stat_END (1)
#define SOC_PMCTRL_APLL5CTRL0_STAT_apll5_refdiv_stat_START (2)
#define SOC_PMCTRL_APLL5CTRL0_STAT_apll5_refdiv_stat_END (7)
#define SOC_PMCTRL_APLL5CTRL0_STAT_apll5_fbdiv_stat_START (8)
#define SOC_PMCTRL_APLL5CTRL0_STAT_apll5_fbdiv_stat_END (19)
#define SOC_PMCTRL_APLL5CTRL0_STAT_apll5_postdiv1_stat_START (20)
#define SOC_PMCTRL_APLL5CTRL0_STAT_apll5_postdiv1_stat_END (22)
#define SOC_PMCTRL_APLL5CTRL0_STAT_apll5_postdiv2_stat_START (23)
#define SOC_PMCTRL_APLL5CTRL0_STAT_apll5_postdiv2_stat_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll5_fracdiv_stat : 24;
        unsigned int apll5_int_mod_stat : 1;
        unsigned int apll5_cfg_vld_stat : 1;
        unsigned int gt_clk_apll5_stat : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL5CTRL1_STAT_UNION;
#endif
#define SOC_PMCTRL_APLL5CTRL1_STAT_apll5_fracdiv_stat_START (0)
#define SOC_PMCTRL_APLL5CTRL1_STAT_apll5_fracdiv_stat_END (23)
#define SOC_PMCTRL_APLL5CTRL1_STAT_apll5_int_mod_stat_START (24)
#define SOC_PMCTRL_APLL5CTRL1_STAT_apll5_int_mod_stat_END (24)
#define SOC_PMCTRL_APLL5CTRL1_STAT_apll5_cfg_vld_stat_START (25)
#define SOC_PMCTRL_APLL5CTRL1_STAT_apll5_cfg_vld_stat_END (25)
#define SOC_PMCTRL_APLL5CTRL1_STAT_gt_clk_apll5_stat_START (26)
#define SOC_PMCTRL_APLL5CTRL1_STAT_gt_clk_apll5_stat_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll6_en_cfg : 1;
        unsigned int apll6_bp_cfg : 1;
        unsigned int apll6_refdiv_cfg : 6;
        unsigned int apll6_fbdiv_cfg : 12;
        unsigned int apll6_postdiv1_cfg : 3;
        unsigned int apll6_postdiv2_cfg : 3;
        unsigned int apll6_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL6CTRL0_UNION;
#endif
#define SOC_PMCTRL_APLL6CTRL0_apll6_en_cfg_START (0)
#define SOC_PMCTRL_APLL6CTRL0_apll6_en_cfg_END (0)
#define SOC_PMCTRL_APLL6CTRL0_apll6_bp_cfg_START (1)
#define SOC_PMCTRL_APLL6CTRL0_apll6_bp_cfg_END (1)
#define SOC_PMCTRL_APLL6CTRL0_apll6_refdiv_cfg_START (2)
#define SOC_PMCTRL_APLL6CTRL0_apll6_refdiv_cfg_END (7)
#define SOC_PMCTRL_APLL6CTRL0_apll6_fbdiv_cfg_START (8)
#define SOC_PMCTRL_APLL6CTRL0_apll6_fbdiv_cfg_END (19)
#define SOC_PMCTRL_APLL6CTRL0_apll6_postdiv1_cfg_START (20)
#define SOC_PMCTRL_APLL6CTRL0_apll6_postdiv1_cfg_END (22)
#define SOC_PMCTRL_APLL6CTRL0_apll6_postdiv2_cfg_START (23)
#define SOC_PMCTRL_APLL6CTRL0_apll6_postdiv2_cfg_END (25)
#define SOC_PMCTRL_APLL6CTRL0_apll6_lock_START (26)
#define SOC_PMCTRL_APLL6CTRL0_apll6_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll6_fracdiv_cfg : 24;
        unsigned int apll6_int_mod_cfg : 1;
        unsigned int apll6_cfg_vld_cfg : 1;
        unsigned int gt_clk_apll6_cfg : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL6CTRL1_UNION;
#endif
#define SOC_PMCTRL_APLL6CTRL1_apll6_fracdiv_cfg_START (0)
#define SOC_PMCTRL_APLL6CTRL1_apll6_fracdiv_cfg_END (23)
#define SOC_PMCTRL_APLL6CTRL1_apll6_int_mod_cfg_START (24)
#define SOC_PMCTRL_APLL6CTRL1_apll6_int_mod_cfg_END (24)
#define SOC_PMCTRL_APLL6CTRL1_apll6_cfg_vld_cfg_START (25)
#define SOC_PMCTRL_APLL6CTRL1_apll6_cfg_vld_cfg_END (25)
#define SOC_PMCTRL_APLL6CTRL1_gt_clk_apll6_cfg_START (26)
#define SOC_PMCTRL_APLL6CTRL1_gt_clk_apll6_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll6_en_stat : 1;
        unsigned int apll6_bp_stat : 1;
        unsigned int apll6_refdiv_stat : 6;
        unsigned int apll6_fbdiv_stat : 12;
        unsigned int apll6_postdiv1_stat : 3;
        unsigned int apll6_postdiv2_stat : 3;
        unsigned int reserved : 6;
    } reg;
} SOC_PMCTRL_APLL6CTRL0_STAT_UNION;
#endif
#define SOC_PMCTRL_APLL6CTRL0_STAT_apll6_en_stat_START (0)
#define SOC_PMCTRL_APLL6CTRL0_STAT_apll6_en_stat_END (0)
#define SOC_PMCTRL_APLL6CTRL0_STAT_apll6_bp_stat_START (1)
#define SOC_PMCTRL_APLL6CTRL0_STAT_apll6_bp_stat_END (1)
#define SOC_PMCTRL_APLL6CTRL0_STAT_apll6_refdiv_stat_START (2)
#define SOC_PMCTRL_APLL6CTRL0_STAT_apll6_refdiv_stat_END (7)
#define SOC_PMCTRL_APLL6CTRL0_STAT_apll6_fbdiv_stat_START (8)
#define SOC_PMCTRL_APLL6CTRL0_STAT_apll6_fbdiv_stat_END (19)
#define SOC_PMCTRL_APLL6CTRL0_STAT_apll6_postdiv1_stat_START (20)
#define SOC_PMCTRL_APLL6CTRL0_STAT_apll6_postdiv1_stat_END (22)
#define SOC_PMCTRL_APLL6CTRL0_STAT_apll6_postdiv2_stat_START (23)
#define SOC_PMCTRL_APLL6CTRL0_STAT_apll6_postdiv2_stat_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll6_fracdiv_stat : 24;
        unsigned int apll6_int_mod_stat : 1;
        unsigned int apll6_cfg_vld_stat : 1;
        unsigned int gt_clk_apll6_stat : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL6CTRL1_STAT_UNION;
#endif
#define SOC_PMCTRL_APLL6CTRL1_STAT_apll6_fracdiv_stat_START (0)
#define SOC_PMCTRL_APLL6CTRL1_STAT_apll6_fracdiv_stat_END (23)
#define SOC_PMCTRL_APLL6CTRL1_STAT_apll6_int_mod_stat_START (24)
#define SOC_PMCTRL_APLL6CTRL1_STAT_apll6_int_mod_stat_END (24)
#define SOC_PMCTRL_APLL6CTRL1_STAT_apll6_cfg_vld_stat_START (25)
#define SOC_PMCTRL_APLL6CTRL1_STAT_apll6_cfg_vld_stat_END (25)
#define SOC_PMCTRL_APLL6CTRL1_STAT_gt_clk_apll6_stat_START (26)
#define SOC_PMCTRL_APLL6CTRL1_STAT_gt_clk_apll6_stat_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll7_en_cfg : 1;
        unsigned int apll7_bp_cfg : 1;
        unsigned int apll7_refdiv_cfg : 6;
        unsigned int apll7_fbdiv_cfg : 12;
        unsigned int apll7_postdiv1_cfg : 3;
        unsigned int apll7_postdiv2_cfg : 3;
        unsigned int apll7_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL7CTRL0_UNION;
#endif
#define SOC_PMCTRL_APLL7CTRL0_apll7_en_cfg_START (0)
#define SOC_PMCTRL_APLL7CTRL0_apll7_en_cfg_END (0)
#define SOC_PMCTRL_APLL7CTRL0_apll7_bp_cfg_START (1)
#define SOC_PMCTRL_APLL7CTRL0_apll7_bp_cfg_END (1)
#define SOC_PMCTRL_APLL7CTRL0_apll7_refdiv_cfg_START (2)
#define SOC_PMCTRL_APLL7CTRL0_apll7_refdiv_cfg_END (7)
#define SOC_PMCTRL_APLL7CTRL0_apll7_fbdiv_cfg_START (8)
#define SOC_PMCTRL_APLL7CTRL0_apll7_fbdiv_cfg_END (19)
#define SOC_PMCTRL_APLL7CTRL0_apll7_postdiv1_cfg_START (20)
#define SOC_PMCTRL_APLL7CTRL0_apll7_postdiv1_cfg_END (22)
#define SOC_PMCTRL_APLL7CTRL0_apll7_postdiv2_cfg_START (23)
#define SOC_PMCTRL_APLL7CTRL0_apll7_postdiv2_cfg_END (25)
#define SOC_PMCTRL_APLL7CTRL0_apll7_lock_START (26)
#define SOC_PMCTRL_APLL7CTRL0_apll7_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll7_fracdiv_cfg : 24;
        unsigned int apll7_int_mod_cfg : 1;
        unsigned int apll7_cfg_vld_cfg : 1;
        unsigned int gt_clk_apll7_cfg : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL7CTRL1_UNION;
#endif
#define SOC_PMCTRL_APLL7CTRL1_apll7_fracdiv_cfg_START (0)
#define SOC_PMCTRL_APLL7CTRL1_apll7_fracdiv_cfg_END (23)
#define SOC_PMCTRL_APLL7CTRL1_apll7_int_mod_cfg_START (24)
#define SOC_PMCTRL_APLL7CTRL1_apll7_int_mod_cfg_END (24)
#define SOC_PMCTRL_APLL7CTRL1_apll7_cfg_vld_cfg_START (25)
#define SOC_PMCTRL_APLL7CTRL1_apll7_cfg_vld_cfg_END (25)
#define SOC_PMCTRL_APLL7CTRL1_gt_clk_apll7_cfg_START (26)
#define SOC_PMCTRL_APLL7CTRL1_gt_clk_apll7_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll7_en_stat : 1;
        unsigned int apll7_bp_stat : 1;
        unsigned int apll7_refdiv_stat : 6;
        unsigned int apll7_fbdiv_stat : 12;
        unsigned int apll7_postdiv1_stat : 3;
        unsigned int apll7_postdiv2_stat : 3;
        unsigned int reserved : 6;
    } reg;
} SOC_PMCTRL_APLL7CTRL0_STAT_UNION;
#endif
#define SOC_PMCTRL_APLL7CTRL0_STAT_apll7_en_stat_START (0)
#define SOC_PMCTRL_APLL7CTRL0_STAT_apll7_en_stat_END (0)
#define SOC_PMCTRL_APLL7CTRL0_STAT_apll7_bp_stat_START (1)
#define SOC_PMCTRL_APLL7CTRL0_STAT_apll7_bp_stat_END (1)
#define SOC_PMCTRL_APLL7CTRL0_STAT_apll7_refdiv_stat_START (2)
#define SOC_PMCTRL_APLL7CTRL0_STAT_apll7_refdiv_stat_END (7)
#define SOC_PMCTRL_APLL7CTRL0_STAT_apll7_fbdiv_stat_START (8)
#define SOC_PMCTRL_APLL7CTRL0_STAT_apll7_fbdiv_stat_END (19)
#define SOC_PMCTRL_APLL7CTRL0_STAT_apll7_postdiv1_stat_START (20)
#define SOC_PMCTRL_APLL7CTRL0_STAT_apll7_postdiv1_stat_END (22)
#define SOC_PMCTRL_APLL7CTRL0_STAT_apll7_postdiv2_stat_START (23)
#define SOC_PMCTRL_APLL7CTRL0_STAT_apll7_postdiv2_stat_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll7_fracdiv_stat : 24;
        unsigned int apll7_int_mod_stat : 1;
        unsigned int apll7_cfg_vld_stat : 1;
        unsigned int gt_clk_apll7_stat : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL7CTRL1_STAT_UNION;
#endif
#define SOC_PMCTRL_APLL7CTRL1_STAT_apll7_fracdiv_stat_START (0)
#define SOC_PMCTRL_APLL7CTRL1_STAT_apll7_fracdiv_stat_END (23)
#define SOC_PMCTRL_APLL7CTRL1_STAT_apll7_int_mod_stat_START (24)
#define SOC_PMCTRL_APLL7CTRL1_STAT_apll7_int_mod_stat_END (24)
#define SOC_PMCTRL_APLL7CTRL1_STAT_apll7_cfg_vld_stat_START (25)
#define SOC_PMCTRL_APLL7CTRL1_STAT_apll7_cfg_vld_stat_END (25)
#define SOC_PMCTRL_APLL7CTRL1_STAT_gt_clk_apll7_stat_START (26)
#define SOC_PMCTRL_APLL7CTRL1_STAT_gt_clk_apll7_stat_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int apll0_fbdiv_sw_initial : 12;
        unsigned int apll0_postdiv1_sw_initial : 3;
        unsigned int apll0_postdiv2_sw_initial : 3;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_PMCTRL_APLL0_SW_INITIAL0_UNION;
#endif
#define SOC_PMCTRL_APLL0_SW_INITIAL0_apll0_fbdiv_sw_initial_START (8)
#define SOC_PMCTRL_APLL0_SW_INITIAL0_apll0_fbdiv_sw_initial_END (19)
#define SOC_PMCTRL_APLL0_SW_INITIAL0_apll0_postdiv1_sw_initial_START (20)
#define SOC_PMCTRL_APLL0_SW_INITIAL0_apll0_postdiv1_sw_initial_END (22)
#define SOC_PMCTRL_APLL0_SW_INITIAL0_apll0_postdiv2_sw_initial_START (23)
#define SOC_PMCTRL_APLL0_SW_INITIAL0_apll0_postdiv2_sw_initial_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll0_fracdiv_sw_initial : 24;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 5;
    } reg;
} SOC_PMCTRL_APLL0_SW_INITIAL1_UNION;
#endif
#define SOC_PMCTRL_APLL0_SW_INITIAL1_apll0_fracdiv_sw_initial_START (0)
#define SOC_PMCTRL_APLL0_SW_INITIAL1_apll0_fracdiv_sw_initial_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int apll1_fbdiv_sw_initial : 12;
        unsigned int apll1_postdiv1_sw_initial : 3;
        unsigned int apll1_postdiv2_sw_initial : 3;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_PMCTRL_APLL1_SW_INITIAL0_UNION;
#endif
#define SOC_PMCTRL_APLL1_SW_INITIAL0_apll1_fbdiv_sw_initial_START (8)
#define SOC_PMCTRL_APLL1_SW_INITIAL0_apll1_fbdiv_sw_initial_END (19)
#define SOC_PMCTRL_APLL1_SW_INITIAL0_apll1_postdiv1_sw_initial_START (20)
#define SOC_PMCTRL_APLL1_SW_INITIAL0_apll1_postdiv1_sw_initial_END (22)
#define SOC_PMCTRL_APLL1_SW_INITIAL0_apll1_postdiv2_sw_initial_START (23)
#define SOC_PMCTRL_APLL1_SW_INITIAL0_apll1_postdiv2_sw_initial_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll1_fracdiv_sw_initial : 24;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 5;
    } reg;
} SOC_PMCTRL_APLL1_SW_INITIAL1_UNION;
#endif
#define SOC_PMCTRL_APLL1_SW_INITIAL1_apll1_fracdiv_sw_initial_START (0)
#define SOC_PMCTRL_APLL1_SW_INITIAL1_apll1_fracdiv_sw_initial_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int apll2_fbdiv_sw_initial : 12;
        unsigned int apll2_postdiv1_sw_initial : 3;
        unsigned int apll2_postdiv2_sw_initial : 3;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_PMCTRL_APLL2_SW_INITIAL0_UNION;
#endif
#define SOC_PMCTRL_APLL2_SW_INITIAL0_apll2_fbdiv_sw_initial_START (8)
#define SOC_PMCTRL_APLL2_SW_INITIAL0_apll2_fbdiv_sw_initial_END (19)
#define SOC_PMCTRL_APLL2_SW_INITIAL0_apll2_postdiv1_sw_initial_START (20)
#define SOC_PMCTRL_APLL2_SW_INITIAL0_apll2_postdiv1_sw_initial_END (22)
#define SOC_PMCTRL_APLL2_SW_INITIAL0_apll2_postdiv2_sw_initial_START (23)
#define SOC_PMCTRL_APLL2_SW_INITIAL0_apll2_postdiv2_sw_initial_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll2_fracdiv_sw_initial : 24;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 5;
    } reg;
} SOC_PMCTRL_APLL2_SW_INITIAL1_UNION;
#endif
#define SOC_PMCTRL_APLL2_SW_INITIAL1_apll2_fracdiv_sw_initial_START (0)
#define SOC_PMCTRL_APLL2_SW_INITIAL1_apll2_fracdiv_sw_initial_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int apll3_fbdiv_sw_initial : 12;
        unsigned int apll3_postdiv1_sw_initial : 3;
        unsigned int apll3_postdiv2_sw_initial : 3;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_PMCTRL_APLL3_SW_INITIAL0_UNION;
#endif
#define SOC_PMCTRL_APLL3_SW_INITIAL0_apll3_fbdiv_sw_initial_START (8)
#define SOC_PMCTRL_APLL3_SW_INITIAL0_apll3_fbdiv_sw_initial_END (19)
#define SOC_PMCTRL_APLL3_SW_INITIAL0_apll3_postdiv1_sw_initial_START (20)
#define SOC_PMCTRL_APLL3_SW_INITIAL0_apll3_postdiv1_sw_initial_END (22)
#define SOC_PMCTRL_APLL3_SW_INITIAL0_apll3_postdiv2_sw_initial_START (23)
#define SOC_PMCTRL_APLL3_SW_INITIAL0_apll3_postdiv2_sw_initial_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll3_fracdiv_sw_initial : 24;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 5;
    } reg;
} SOC_PMCTRL_APLL3_SW_INITIAL1_UNION;
#endif
#define SOC_PMCTRL_APLL3_SW_INITIAL1_apll3_fracdiv_sw_initial_START (0)
#define SOC_PMCTRL_APLL3_SW_INITIAL1_apll3_fracdiv_sw_initial_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int apll4_fbdiv_sw_initial : 12;
        unsigned int apll4_postdiv1_sw_initial : 3;
        unsigned int apll4_postdiv2_sw_initial : 3;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_PMCTRL_APLL4_SW_INITIAL0_UNION;
#endif
#define SOC_PMCTRL_APLL4_SW_INITIAL0_apll4_fbdiv_sw_initial_START (8)
#define SOC_PMCTRL_APLL4_SW_INITIAL0_apll4_fbdiv_sw_initial_END (19)
#define SOC_PMCTRL_APLL4_SW_INITIAL0_apll4_postdiv1_sw_initial_START (20)
#define SOC_PMCTRL_APLL4_SW_INITIAL0_apll4_postdiv1_sw_initial_END (22)
#define SOC_PMCTRL_APLL4_SW_INITIAL0_apll4_postdiv2_sw_initial_START (23)
#define SOC_PMCTRL_APLL4_SW_INITIAL0_apll4_postdiv2_sw_initial_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll4_fracdiv_sw_initial : 24;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 5;
    } reg;
} SOC_PMCTRL_APLL4_SW_INITIAL1_UNION;
#endif
#define SOC_PMCTRL_APLL4_SW_INITIAL1_apll4_fracdiv_sw_initial_START (0)
#define SOC_PMCTRL_APLL4_SW_INITIAL1_apll4_fracdiv_sw_initial_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int apll5_fbdiv_sw_initial : 12;
        unsigned int apll5_postdiv1_sw_initial : 3;
        unsigned int apll5_postdiv2_sw_initial : 3;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_PMCTRL_APLL5_SW_INITIAL0_UNION;
#endif
#define SOC_PMCTRL_APLL5_SW_INITIAL0_apll5_fbdiv_sw_initial_START (8)
#define SOC_PMCTRL_APLL5_SW_INITIAL0_apll5_fbdiv_sw_initial_END (19)
#define SOC_PMCTRL_APLL5_SW_INITIAL0_apll5_postdiv1_sw_initial_START (20)
#define SOC_PMCTRL_APLL5_SW_INITIAL0_apll5_postdiv1_sw_initial_END (22)
#define SOC_PMCTRL_APLL5_SW_INITIAL0_apll5_postdiv2_sw_initial_START (23)
#define SOC_PMCTRL_APLL5_SW_INITIAL0_apll5_postdiv2_sw_initial_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll5_fracdiv_sw_initial : 24;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 5;
    } reg;
} SOC_PMCTRL_APLL5_SW_INITIAL1_UNION;
#endif
#define SOC_PMCTRL_APLL5_SW_INITIAL1_apll5_fracdiv_sw_initial_START (0)
#define SOC_PMCTRL_APLL5_SW_INITIAL1_apll5_fracdiv_sw_initial_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int apll6_fbdiv_sw_initial : 12;
        unsigned int apll6_postdiv1_sw_initial : 3;
        unsigned int apll6_postdiv2_sw_initial : 3;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_PMCTRL_APLL6_SW_INITIAL0_UNION;
#endif
#define SOC_PMCTRL_APLL6_SW_INITIAL0_apll6_fbdiv_sw_initial_START (8)
#define SOC_PMCTRL_APLL6_SW_INITIAL0_apll6_fbdiv_sw_initial_END (19)
#define SOC_PMCTRL_APLL6_SW_INITIAL0_apll6_postdiv1_sw_initial_START (20)
#define SOC_PMCTRL_APLL6_SW_INITIAL0_apll6_postdiv1_sw_initial_END (22)
#define SOC_PMCTRL_APLL6_SW_INITIAL0_apll6_postdiv2_sw_initial_START (23)
#define SOC_PMCTRL_APLL6_SW_INITIAL0_apll6_postdiv2_sw_initial_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll6_fracdiv_sw_initial : 24;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 5;
    } reg;
} SOC_PMCTRL_APLL6_SW_INITIAL1_UNION;
#endif
#define SOC_PMCTRL_APLL6_SW_INITIAL1_apll6_fracdiv_sw_initial_START (0)
#define SOC_PMCTRL_APLL6_SW_INITIAL1_apll6_fracdiv_sw_initial_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int apll7_fbdiv_sw_initial : 12;
        unsigned int apll7_postdiv1_sw_initial : 3;
        unsigned int apll7_postdiv2_sw_initial : 3;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_PMCTRL_APLL7_SW_INITIAL0_UNION;
#endif
#define SOC_PMCTRL_APLL7_SW_INITIAL0_apll7_fbdiv_sw_initial_START (8)
#define SOC_PMCTRL_APLL7_SW_INITIAL0_apll7_fbdiv_sw_initial_END (19)
#define SOC_PMCTRL_APLL7_SW_INITIAL0_apll7_postdiv1_sw_initial_START (20)
#define SOC_PMCTRL_APLL7_SW_INITIAL0_apll7_postdiv1_sw_initial_END (22)
#define SOC_PMCTRL_APLL7_SW_INITIAL0_apll7_postdiv2_sw_initial_START (23)
#define SOC_PMCTRL_APLL7_SW_INITIAL0_apll7_postdiv2_sw_initial_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll7_fracdiv_sw_initial : 24;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 5;
    } reg;
} SOC_PMCTRL_APLL7_SW_INITIAL1_UNION;
#endif
#define SOC_PMCTRL_APLL7_SW_INITIAL1_apll7_fracdiv_sw_initial_START (0)
#define SOC_PMCTRL_APLL7_SW_INITIAL1_apll7_fracdiv_sw_initial_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PDVFS_INSTR_LIST_A_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PDVFS_INSTR_LIST_B_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PDVFS_INSTR_LIST_C_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PDVFS_INSTR_LIST_D_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PDVFS_INSTR_LIST_E_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PDVFS_INSTR_LIST_F_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrla_instr_start_addr : 12;
        unsigned int taskctrla_instr_stop_addr : 12;
        unsigned int taskctrla_instr_addr_err : 1;
        unsigned int reserved : 7;
    } reg;
} SOC_PMCTRL_INSTR_ADDR_CTRL_A_UNION;
#endif
#define SOC_PMCTRL_INSTR_ADDR_CTRL_A_taskctrla_instr_start_addr_START (0)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_A_taskctrla_instr_start_addr_END (11)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_A_taskctrla_instr_stop_addr_START (12)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_A_taskctrla_instr_stop_addr_END (23)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_A_taskctrla_instr_addr_err_START (24)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_A_taskctrla_instr_addr_err_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrlb_instr_start_addr : 12;
        unsigned int taskctrlb_instr_stop_addr : 12;
        unsigned int taskctrlb_instr_addr_err : 1;
        unsigned int reserved : 7;
    } reg;
} SOC_PMCTRL_INSTR_ADDR_CTRL_B_UNION;
#endif
#define SOC_PMCTRL_INSTR_ADDR_CTRL_B_taskctrlb_instr_start_addr_START (0)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_B_taskctrlb_instr_start_addr_END (11)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_B_taskctrlb_instr_stop_addr_START (12)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_B_taskctrlb_instr_stop_addr_END (23)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_B_taskctrlb_instr_addr_err_START (24)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_B_taskctrlb_instr_addr_err_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrlc_instr_start_addr : 12;
        unsigned int taskctrlc_instr_stop_addr : 12;
        unsigned int taskctrlc_instr_addr_err : 1;
        unsigned int reserved : 7;
    } reg;
} SOC_PMCTRL_INSTR_ADDR_CTRL_C_UNION;
#endif
#define SOC_PMCTRL_INSTR_ADDR_CTRL_C_taskctrlc_instr_start_addr_START (0)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_C_taskctrlc_instr_start_addr_END (11)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_C_taskctrlc_instr_stop_addr_START (12)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_C_taskctrlc_instr_stop_addr_END (23)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_C_taskctrlc_instr_addr_err_START (24)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_C_taskctrlc_instr_addr_err_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrld_instr_start_addr : 12;
        unsigned int taskctrld_instr_stop_addr : 12;
        unsigned int taskctrld_instr_addr_err : 1;
        unsigned int reserved : 7;
    } reg;
} SOC_PMCTRL_INSTR_ADDR_CTRL_D_UNION;
#endif
#define SOC_PMCTRL_INSTR_ADDR_CTRL_D_taskctrld_instr_start_addr_START (0)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_D_taskctrld_instr_start_addr_END (11)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_D_taskctrld_instr_stop_addr_START (12)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_D_taskctrld_instr_stop_addr_END (23)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_D_taskctrld_instr_addr_err_START (24)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_D_taskctrld_instr_addr_err_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrle_instr_start_addr : 12;
        unsigned int taskctrle_instr_stop_addr : 12;
        unsigned int taskctrle_instr_addr_err : 1;
        unsigned int reserved : 7;
    } reg;
} SOC_PMCTRL_INSTR_ADDR_CTRL_E_UNION;
#endif
#define SOC_PMCTRL_INSTR_ADDR_CTRL_E_taskctrle_instr_start_addr_START (0)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_E_taskctrle_instr_start_addr_END (11)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_E_taskctrle_instr_stop_addr_START (12)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_E_taskctrle_instr_stop_addr_END (23)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_E_taskctrle_instr_addr_err_START (24)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_E_taskctrle_instr_addr_err_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrlf_instr_start_addr : 12;
        unsigned int taskctrlf_instr_stop_addr : 12;
        unsigned int taskctrlf_instr_addr_err : 1;
        unsigned int reserved : 7;
    } reg;
} SOC_PMCTRL_INSTR_ADDR_CTRL_F_UNION;
#endif
#define SOC_PMCTRL_INSTR_ADDR_CTRL_F_taskctrlf_instr_start_addr_START (0)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_F_taskctrlf_instr_start_addr_END (11)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_F_taskctrlf_instr_stop_addr_START (12)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_F_taskctrlf_instr_stop_addr_END (23)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_F_taskctrlf_instr_addr_err_START (24)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_F_taskctrlf_instr_addr_err_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrlg_instr_start_addr : 12;
        unsigned int taskctrlg_instr_stop_addr : 12;
        unsigned int taskctrlg_instr_addr_err : 1;
        unsigned int reserved : 7;
    } reg;
} SOC_PMCTRL_INSTR_ADDR_CTRL_G_UNION;
#endif
#define SOC_PMCTRL_INSTR_ADDR_CTRL_G_taskctrlg_instr_start_addr_START (0)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_G_taskctrlg_instr_start_addr_END (11)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_G_taskctrlg_instr_stop_addr_START (12)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_G_taskctrlg_instr_stop_addr_END (23)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_G_taskctrlg_instr_addr_err_START (24)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_G_taskctrlg_instr_addr_err_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrlh_instr_start_addr : 12;
        unsigned int taskctrlh_instr_stop_addr : 12;
        unsigned int taskctrlh_instr_addr_err : 1;
        unsigned int reserved : 7;
    } reg;
} SOC_PMCTRL_INSTR_ADDR_CTRL_H_UNION;
#endif
#define SOC_PMCTRL_INSTR_ADDR_CTRL_H_taskctrlh_instr_start_addr_START (0)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_H_taskctrlh_instr_start_addr_END (11)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_H_taskctrlh_instr_stop_addr_START (12)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_H_taskctrlh_instr_stop_addr_END (23)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_H_taskctrlh_instr_addr_err_START (24)
#define SOC_PMCTRL_INSTR_ADDR_CTRL_H_taskctrlh_instr_addr_err_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrla_instr_clr : 1;
        unsigned int taskctrlb_instr_clr : 1;
        unsigned int taskctrlc_instr_clr : 1;
        unsigned int taskctrld_instr_clr : 1;
        unsigned int taskctrle_instr_clr : 1;
        unsigned int taskctrlf_instr_clr : 1;
        unsigned int taskctrlg_instr_clr : 1;
        unsigned int taskctrlh_instr_clr : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
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
    } reg;
} SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_taskctrla_instr_clr_START (0)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_taskctrla_instr_clr_END (0)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_taskctrlb_instr_clr_START (1)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_taskctrlb_instr_clr_END (1)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_taskctrlc_instr_clr_START (2)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_taskctrlc_instr_clr_END (2)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_taskctrld_instr_clr_START (3)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_taskctrld_instr_clr_END (3)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_taskctrle_instr_clr_START (4)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_taskctrle_instr_clr_END (4)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_taskctrlf_instr_clr_START (5)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_taskctrlf_instr_clr_END (5)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_taskctrlg_instr_clr_START (6)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_taskctrlg_instr_clr_END (6)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_taskctrlh_instr_clr_START (7)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_0_taskctrlh_instr_clr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrla_instr_lock : 1;
        unsigned int taskctrlb_instr_lock : 1;
        unsigned int taskctrlc_instr_lock : 1;
        unsigned int taskctrld_instr_lock : 1;
        unsigned int taskctrle_instr_lock : 1;
        unsigned int taskctrlf_instr_lock : 1;
        unsigned int taskctrlg_instr_lock : 1;
        unsigned int taskctrlh_instr_lock : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
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
    } reg;
} SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_taskctrla_instr_lock_START (0)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_taskctrla_instr_lock_END (0)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_taskctrlb_instr_lock_START (1)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_taskctrlb_instr_lock_END (1)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_taskctrlc_instr_lock_START (2)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_taskctrlc_instr_lock_END (2)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_taskctrld_instr_lock_START (3)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_taskctrld_instr_lock_END (3)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_taskctrle_instr_lock_START (4)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_taskctrle_instr_lock_END (4)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_taskctrlf_instr_lock_START (5)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_taskctrlf_instr_lock_END (5)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_taskctrlg_instr_lock_START (6)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_taskctrlg_instr_lock_END (6)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_taskctrlh_instr_lock_START (7)
#define SOC_PMCTRL_TASKCTRL_INSTR_CTRL_1_taskctrlh_instr_lock_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrla_intr_dvfs_en : 1;
        unsigned int taskctrlb_intr_dvfs_en : 1;
        unsigned int taskctrlc_intr_dvfs_en : 1;
        unsigned int taskctrld_intr_dvfs_en : 1;
        unsigned int taskctrle_intr_dvfs_en : 1;
        unsigned int taskctrlf_intr_dvfs_en : 1;
        unsigned int taskctrlg_intr_dvfs_en : 1;
        unsigned int taskctrlh_intr_dvfs_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
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
    } reg;
} SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_taskctrla_intr_dvfs_en_START (0)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_taskctrla_intr_dvfs_en_END (0)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_taskctrlb_intr_dvfs_en_START (1)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_taskctrlb_intr_dvfs_en_END (1)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_taskctrlc_intr_dvfs_en_START (2)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_taskctrlc_intr_dvfs_en_END (2)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_taskctrld_intr_dvfs_en_START (3)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_taskctrld_intr_dvfs_en_END (3)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_taskctrle_intr_dvfs_en_START (4)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_taskctrle_intr_dvfs_en_END (4)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_taskctrlf_intr_dvfs_en_START (5)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_taskctrlf_intr_dvfs_en_END (5)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_taskctrlg_intr_dvfs_en_START (6)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_taskctrlg_intr_dvfs_en_END (6)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_taskctrlh_intr_dvfs_en_START (7)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_0_taskctrlh_intr_dvfs_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrla_intr_dvfs_clr : 1;
        unsigned int taskctrlb_intr_dvfs_clr : 1;
        unsigned int taskctrlc_intr_dvfs_clr : 1;
        unsigned int taskctrld_intr_dvfs_clr : 1;
        unsigned int taskctrle_intr_dvfs_clr : 1;
        unsigned int taskctrlf_intr_dvfs_clr : 1;
        unsigned int taskctrlg_intr_dvfs_clr : 1;
        unsigned int taskctrlh_intr_dvfs_clr : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int taskctrla_intr_dvfs_raw_stat : 1;
        unsigned int taskctrlb_intr_dvfs_raw_stat : 1;
        unsigned int taskctrlc_intr_dvfs_raw_stat : 1;
        unsigned int taskctrld_intr_dvfs_raw_stat : 1;
        unsigned int taskctrle_intr_dvfs_raw_stat : 1;
        unsigned int taskctrlf_intr_dvfs_raw_stat : 1;
        unsigned int taskctrlg_intr_dvfs_raw_stat : 1;
        unsigned int taskctrlh_intr_dvfs_raw_stat : 1;
        unsigned int taskctrla_intr_dvfs_stat : 1;
        unsigned int taskctrlb_intr_dvfs_stat : 1;
        unsigned int taskctrlc_intr_dvfs_stat : 1;
        unsigned int taskctrld_intr_dvfs_stat : 1;
        unsigned int taskctrle_intr_dvfs_stat : 1;
        unsigned int taskctrlf_intr_dvfs_stat : 1;
        unsigned int taskctrlg_intr_dvfs_stat : 1;
        unsigned int taskctrlh_intr_dvfs_stat : 1;
    } reg;
} SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrla_intr_dvfs_clr_START (0)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrla_intr_dvfs_clr_END (0)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlb_intr_dvfs_clr_START (1)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlb_intr_dvfs_clr_END (1)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlc_intr_dvfs_clr_START (2)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlc_intr_dvfs_clr_END (2)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrld_intr_dvfs_clr_START (3)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrld_intr_dvfs_clr_END (3)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrle_intr_dvfs_clr_START (4)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrle_intr_dvfs_clr_END (4)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlf_intr_dvfs_clr_START (5)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlf_intr_dvfs_clr_END (5)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlg_intr_dvfs_clr_START (6)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlg_intr_dvfs_clr_END (6)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlh_intr_dvfs_clr_START (7)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlh_intr_dvfs_clr_END (7)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrla_intr_dvfs_raw_stat_START (16)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrla_intr_dvfs_raw_stat_END (16)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlb_intr_dvfs_raw_stat_START (17)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlb_intr_dvfs_raw_stat_END (17)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlc_intr_dvfs_raw_stat_START (18)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlc_intr_dvfs_raw_stat_END (18)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrld_intr_dvfs_raw_stat_START (19)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrld_intr_dvfs_raw_stat_END (19)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrle_intr_dvfs_raw_stat_START (20)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrle_intr_dvfs_raw_stat_END (20)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlf_intr_dvfs_raw_stat_START (21)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlf_intr_dvfs_raw_stat_END (21)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlg_intr_dvfs_raw_stat_START (22)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlg_intr_dvfs_raw_stat_END (22)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlh_intr_dvfs_raw_stat_START (23)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlh_intr_dvfs_raw_stat_END (23)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrla_intr_dvfs_stat_START (24)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrla_intr_dvfs_stat_END (24)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlb_intr_dvfs_stat_START (25)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlb_intr_dvfs_stat_END (25)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlc_intr_dvfs_stat_START (26)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlc_intr_dvfs_stat_END (26)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrld_intr_dvfs_stat_START (27)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrld_intr_dvfs_stat_END (27)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrle_intr_dvfs_stat_START (28)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrle_intr_dvfs_stat_END (28)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlf_intr_dvfs_stat_START (29)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlf_intr_dvfs_stat_END (29)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlg_intr_dvfs_stat_START (30)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlg_intr_dvfs_stat_END (30)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlh_intr_dvfs_stat_START (31)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_1_taskctrlh_intr_dvfs_stat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrla_intr_dvfs_stat_converge_ctrl : 2;
        unsigned int taskctrlb_intr_dvfs_stat_converge_ctrl : 2;
        unsigned int taskctrlc_intr_dvfs_stat_converge_ctrl : 2;
        unsigned int taskctrld_intr_dvfs_stat_converge_ctrl : 2;
        unsigned int taskctrle_intr_dvfs_stat_converge_ctrl : 2;
        unsigned int taskctrlf_intr_dvfs_stat_converge_ctrl : 2;
        unsigned int taskctrlg_intr_dvfs_stat_converge_ctrl : 2;
        unsigned int taskctrlh_intr_dvfs_stat_converge_ctrl : 2;
        unsigned int intr_dvfs_stat_pmc2lpm3_0 : 1;
        unsigned int reserved_0 : 3;
        unsigned int intr_dvfs_stat_pmc2lpm3_1 : 1;
        unsigned int reserved_1 : 3;
        unsigned int intr_dvfs_stat_pmc2lpm3_2 : 1;
        unsigned int reserved_2 : 3;
        unsigned int reserved_3 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_taskctrla_intr_dvfs_stat_converge_ctrl_START (0)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_taskctrla_intr_dvfs_stat_converge_ctrl_END (1)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_taskctrlb_intr_dvfs_stat_converge_ctrl_START (2)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_taskctrlb_intr_dvfs_stat_converge_ctrl_END (3)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_taskctrlc_intr_dvfs_stat_converge_ctrl_START (4)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_taskctrlc_intr_dvfs_stat_converge_ctrl_END (5)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_taskctrld_intr_dvfs_stat_converge_ctrl_START (6)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_taskctrld_intr_dvfs_stat_converge_ctrl_END (7)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_taskctrle_intr_dvfs_stat_converge_ctrl_START (8)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_taskctrle_intr_dvfs_stat_converge_ctrl_END (9)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_taskctrlf_intr_dvfs_stat_converge_ctrl_START (10)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_taskctrlf_intr_dvfs_stat_converge_ctrl_END (11)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_taskctrlg_intr_dvfs_stat_converge_ctrl_START (12)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_taskctrlg_intr_dvfs_stat_converge_ctrl_END (13)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_taskctrlh_intr_dvfs_stat_converge_ctrl_START (14)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_taskctrlh_intr_dvfs_stat_converge_ctrl_END (15)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_intr_dvfs_stat_pmc2lpm3_0_START (16)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_intr_dvfs_stat_pmc2lpm3_0_END (16)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_intr_dvfs_stat_pmc2lpm3_1_START (20)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_intr_dvfs_stat_pmc2lpm3_1_END (20)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_intr_dvfs_stat_pmc2lpm3_2_START (24)
#define SOC_PMCTRL_TASKCTRL_INTR_CTRL_2_intr_dvfs_stat_pmc2lpm3_2_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrla_enable : 1;
        unsigned int taskctrlb_enable : 1;
        unsigned int taskctrlc_enable : 1;
        unsigned int taskctrld_enable : 1;
        unsigned int taskctrle_enable : 1;
        unsigned int taskctrlf_enable : 1;
        unsigned int taskctrlg_enable : 1;
        unsigned int taskctrlh_enable : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
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
    } reg;
} SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_taskctrla_enable_START (0)
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_taskctrla_enable_END (0)
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_taskctrlb_enable_START (1)
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_taskctrlb_enable_END (1)
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_taskctrlc_enable_START (2)
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_taskctrlc_enable_END (2)
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_taskctrld_enable_START (3)
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_taskctrld_enable_END (3)
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_taskctrle_enable_START (4)
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_taskctrle_enable_END (4)
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_taskctrlf_enable_START (5)
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_taskctrlf_enable_END (5)
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_taskctrlg_enable_START (6)
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_taskctrlg_enable_END (6)
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_taskctrlh_enable_START (7)
#define SOC_PMCTRL_TASKCTRL_ENABLE_CTRL_taskctrlh_enable_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrla_dvfs_en : 1;
        unsigned int taskctrla_next_step_cfg : 1;
        unsigned int taskctrla_escape_cfg : 1;
        unsigned int taskctrla_invalid_instr_flag_clr : 1;
        unsigned int taskctrla_invalid_instr_flag : 1;
        unsigned int taskctrla_halt_flag : 1;
        unsigned int reserved_0 : 2;
        unsigned int taskctrla_cur_decoding_instr_addr : 12;
        unsigned int taskctrla_cur_fetch_num : 8;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_A_0_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_A_0_taskctrla_dvfs_en_START (0)
#define SOC_PMCTRL_TASKCTRL_A_0_taskctrla_dvfs_en_END (0)
#define SOC_PMCTRL_TASKCTRL_A_0_taskctrla_next_step_cfg_START (1)
#define SOC_PMCTRL_TASKCTRL_A_0_taskctrla_next_step_cfg_END (1)
#define SOC_PMCTRL_TASKCTRL_A_0_taskctrla_escape_cfg_START (2)
#define SOC_PMCTRL_TASKCTRL_A_0_taskctrla_escape_cfg_END (2)
#define SOC_PMCTRL_TASKCTRL_A_0_taskctrla_invalid_instr_flag_clr_START (3)
#define SOC_PMCTRL_TASKCTRL_A_0_taskctrla_invalid_instr_flag_clr_END (3)
#define SOC_PMCTRL_TASKCTRL_A_0_taskctrla_invalid_instr_flag_START (4)
#define SOC_PMCTRL_TASKCTRL_A_0_taskctrla_invalid_instr_flag_END (4)
#define SOC_PMCTRL_TASKCTRL_A_0_taskctrla_halt_flag_START (5)
#define SOC_PMCTRL_TASKCTRL_A_0_taskctrla_halt_flag_END (5)
#define SOC_PMCTRL_TASKCTRL_A_0_taskctrla_cur_decoding_instr_addr_START (8)
#define SOC_PMCTRL_TASKCTRL_A_0_taskctrla_cur_decoding_instr_addr_END (19)
#define SOC_PMCTRL_TASKCTRL_A_0_taskctrla_cur_fetch_num_START (20)
#define SOC_PMCTRL_TASKCTRL_A_0_taskctrla_cur_fetch_num_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrla_cur_task_stat : 8;
        unsigned int taskctrla_num_of_end_instr : 8;
        unsigned int reserved_0 : 12;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_A_1_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_A_1_taskctrla_cur_task_stat_START (0)
#define SOC_PMCTRL_TASKCTRL_A_1_taskctrla_cur_task_stat_END (7)
#define SOC_PMCTRL_TASKCTRL_A_1_taskctrla_num_of_end_instr_START (8)
#define SOC_PMCTRL_TASKCTRL_A_1_taskctrla_num_of_end_instr_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 8;
        unsigned int reserved_1: 8;
        unsigned int reserved_2: 12;
        unsigned int reserved_3: 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_A_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 8;
        unsigned int reserved_1: 8;
        unsigned int reserved_2: 12;
        unsigned int reserved_3: 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_A_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrlb_dvfs_en : 1;
        unsigned int taskctrlb_next_step_cfg : 1;
        unsigned int taskctrlb_escape_cfg : 1;
        unsigned int taskctrlb_invalid_instr_flag_clr : 1;
        unsigned int taskctrlb_invalid_instr_flag : 1;
        unsigned int taskctrlb_halt_flag : 1;
        unsigned int reserved_0 : 2;
        unsigned int taskctrlb_cur_decoding_instr_addr : 12;
        unsigned int taskctrlb_cur_fetch_num : 8;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_B_0_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_B_0_taskctrlb_dvfs_en_START (0)
#define SOC_PMCTRL_TASKCTRL_B_0_taskctrlb_dvfs_en_END (0)
#define SOC_PMCTRL_TASKCTRL_B_0_taskctrlb_next_step_cfg_START (1)
#define SOC_PMCTRL_TASKCTRL_B_0_taskctrlb_next_step_cfg_END (1)
#define SOC_PMCTRL_TASKCTRL_B_0_taskctrlb_escape_cfg_START (2)
#define SOC_PMCTRL_TASKCTRL_B_0_taskctrlb_escape_cfg_END (2)
#define SOC_PMCTRL_TASKCTRL_B_0_taskctrlb_invalid_instr_flag_clr_START (3)
#define SOC_PMCTRL_TASKCTRL_B_0_taskctrlb_invalid_instr_flag_clr_END (3)
#define SOC_PMCTRL_TASKCTRL_B_0_taskctrlb_invalid_instr_flag_START (4)
#define SOC_PMCTRL_TASKCTRL_B_0_taskctrlb_invalid_instr_flag_END (4)
#define SOC_PMCTRL_TASKCTRL_B_0_taskctrlb_halt_flag_START (5)
#define SOC_PMCTRL_TASKCTRL_B_0_taskctrlb_halt_flag_END (5)
#define SOC_PMCTRL_TASKCTRL_B_0_taskctrlb_cur_decoding_instr_addr_START (8)
#define SOC_PMCTRL_TASKCTRL_B_0_taskctrlb_cur_decoding_instr_addr_END (19)
#define SOC_PMCTRL_TASKCTRL_B_0_taskctrlb_cur_fetch_num_START (20)
#define SOC_PMCTRL_TASKCTRL_B_0_taskctrlb_cur_fetch_num_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrlb_cur_task_stat : 8;
        unsigned int taskctrlb_num_of_end_instr : 8;
        unsigned int reserved_0 : 12;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_B_1_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_B_1_taskctrlb_cur_task_stat_START (0)
#define SOC_PMCTRL_TASKCTRL_B_1_taskctrlb_cur_task_stat_END (7)
#define SOC_PMCTRL_TASKCTRL_B_1_taskctrlb_num_of_end_instr_START (8)
#define SOC_PMCTRL_TASKCTRL_B_1_taskctrlb_num_of_end_instr_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 8;
        unsigned int reserved_1: 8;
        unsigned int reserved_2: 12;
        unsigned int reserved_3: 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_B_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 8;
        unsigned int reserved_1: 8;
        unsigned int reserved_2: 12;
        unsigned int reserved_3: 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_B_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrlc_dvfs_en : 1;
        unsigned int taskctrlc_next_step_cfg : 1;
        unsigned int taskctrlc_escape_cfg : 1;
        unsigned int taskctrlc_invalid_instr_flag_clr : 1;
        unsigned int taskctrlc_invalid_instr_flag : 1;
        unsigned int taskctrlc_halt_flag : 1;
        unsigned int reserved_0 : 2;
        unsigned int taskctrlc_cur_decoding_instr_addr : 12;
        unsigned int taskctrlc_cur_fetch_num : 8;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_C_0_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_C_0_taskctrlc_dvfs_en_START (0)
#define SOC_PMCTRL_TASKCTRL_C_0_taskctrlc_dvfs_en_END (0)
#define SOC_PMCTRL_TASKCTRL_C_0_taskctrlc_next_step_cfg_START (1)
#define SOC_PMCTRL_TASKCTRL_C_0_taskctrlc_next_step_cfg_END (1)
#define SOC_PMCTRL_TASKCTRL_C_0_taskctrlc_escape_cfg_START (2)
#define SOC_PMCTRL_TASKCTRL_C_0_taskctrlc_escape_cfg_END (2)
#define SOC_PMCTRL_TASKCTRL_C_0_taskctrlc_invalid_instr_flag_clr_START (3)
#define SOC_PMCTRL_TASKCTRL_C_0_taskctrlc_invalid_instr_flag_clr_END (3)
#define SOC_PMCTRL_TASKCTRL_C_0_taskctrlc_invalid_instr_flag_START (4)
#define SOC_PMCTRL_TASKCTRL_C_0_taskctrlc_invalid_instr_flag_END (4)
#define SOC_PMCTRL_TASKCTRL_C_0_taskctrlc_halt_flag_START (5)
#define SOC_PMCTRL_TASKCTRL_C_0_taskctrlc_halt_flag_END (5)
#define SOC_PMCTRL_TASKCTRL_C_0_taskctrlc_cur_decoding_instr_addr_START (8)
#define SOC_PMCTRL_TASKCTRL_C_0_taskctrlc_cur_decoding_instr_addr_END (19)
#define SOC_PMCTRL_TASKCTRL_C_0_taskctrlc_cur_fetch_num_START (20)
#define SOC_PMCTRL_TASKCTRL_C_0_taskctrlc_cur_fetch_num_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrlc_cur_task_stat : 8;
        unsigned int taskctrlc_num_of_end_instr : 8;
        unsigned int reserved_0 : 12;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_C_1_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_C_1_taskctrlc_cur_task_stat_START (0)
#define SOC_PMCTRL_TASKCTRL_C_1_taskctrlc_cur_task_stat_END (7)
#define SOC_PMCTRL_TASKCTRL_C_1_taskctrlc_num_of_end_instr_START (8)
#define SOC_PMCTRL_TASKCTRL_C_1_taskctrlc_num_of_end_instr_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 8;
        unsigned int reserved_1: 8;
        unsigned int reserved_2: 12;
        unsigned int reserved_3: 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_C_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 8;
        unsigned int reserved_1: 8;
        unsigned int reserved_2: 12;
        unsigned int reserved_3: 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_C_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrld_dvfs_en : 1;
        unsigned int taskctrld_next_step_cfg : 1;
        unsigned int taskctrld_escape_cfg : 1;
        unsigned int taskctrld_invalid_instr_flag_clr : 1;
        unsigned int taskctrld_invalid_instr_flag : 1;
        unsigned int taskctrld_halt_flag : 1;
        unsigned int reserved_0 : 2;
        unsigned int taskctrld_cur_decoding_instr_addr : 12;
        unsigned int taskctrld_cur_fetch_num : 8;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_D_0_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_D_0_taskctrld_dvfs_en_START (0)
#define SOC_PMCTRL_TASKCTRL_D_0_taskctrld_dvfs_en_END (0)
#define SOC_PMCTRL_TASKCTRL_D_0_taskctrld_next_step_cfg_START (1)
#define SOC_PMCTRL_TASKCTRL_D_0_taskctrld_next_step_cfg_END (1)
#define SOC_PMCTRL_TASKCTRL_D_0_taskctrld_escape_cfg_START (2)
#define SOC_PMCTRL_TASKCTRL_D_0_taskctrld_escape_cfg_END (2)
#define SOC_PMCTRL_TASKCTRL_D_0_taskctrld_invalid_instr_flag_clr_START (3)
#define SOC_PMCTRL_TASKCTRL_D_0_taskctrld_invalid_instr_flag_clr_END (3)
#define SOC_PMCTRL_TASKCTRL_D_0_taskctrld_invalid_instr_flag_START (4)
#define SOC_PMCTRL_TASKCTRL_D_0_taskctrld_invalid_instr_flag_END (4)
#define SOC_PMCTRL_TASKCTRL_D_0_taskctrld_halt_flag_START (5)
#define SOC_PMCTRL_TASKCTRL_D_0_taskctrld_halt_flag_END (5)
#define SOC_PMCTRL_TASKCTRL_D_0_taskctrld_cur_decoding_instr_addr_START (8)
#define SOC_PMCTRL_TASKCTRL_D_0_taskctrld_cur_decoding_instr_addr_END (19)
#define SOC_PMCTRL_TASKCTRL_D_0_taskctrld_cur_fetch_num_START (20)
#define SOC_PMCTRL_TASKCTRL_D_0_taskctrld_cur_fetch_num_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrld_cur_task_stat : 8;
        unsigned int taskctrld_num_of_end_instr : 8;
        unsigned int reserved_0 : 12;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_D_1_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_D_1_taskctrld_cur_task_stat_START (0)
#define SOC_PMCTRL_TASKCTRL_D_1_taskctrld_cur_task_stat_END (7)
#define SOC_PMCTRL_TASKCTRL_D_1_taskctrld_num_of_end_instr_START (8)
#define SOC_PMCTRL_TASKCTRL_D_1_taskctrld_num_of_end_instr_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 8;
        unsigned int reserved_1: 8;
        unsigned int reserved_2: 12;
        unsigned int reserved_3: 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_D_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 8;
        unsigned int reserved_1: 8;
        unsigned int reserved_2: 12;
        unsigned int reserved_3: 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_D_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrle_dvfs_en : 1;
        unsigned int taskctrle_next_step_cfg : 1;
        unsigned int taskctrle_escape_cfg : 1;
        unsigned int taskctrle_invalid_instr_flag_clr : 1;
        unsigned int taskctrle_invalid_instr_flag : 1;
        unsigned int taskctrle_halt_flag : 1;
        unsigned int reserved_0 : 2;
        unsigned int taskctrle_cur_decoding_instr_addr : 12;
        unsigned int taskctrle_cur_fetch_num : 8;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_E_0_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_E_0_taskctrle_dvfs_en_START (0)
#define SOC_PMCTRL_TASKCTRL_E_0_taskctrle_dvfs_en_END (0)
#define SOC_PMCTRL_TASKCTRL_E_0_taskctrle_next_step_cfg_START (1)
#define SOC_PMCTRL_TASKCTRL_E_0_taskctrle_next_step_cfg_END (1)
#define SOC_PMCTRL_TASKCTRL_E_0_taskctrle_escape_cfg_START (2)
#define SOC_PMCTRL_TASKCTRL_E_0_taskctrle_escape_cfg_END (2)
#define SOC_PMCTRL_TASKCTRL_E_0_taskctrle_invalid_instr_flag_clr_START (3)
#define SOC_PMCTRL_TASKCTRL_E_0_taskctrle_invalid_instr_flag_clr_END (3)
#define SOC_PMCTRL_TASKCTRL_E_0_taskctrle_invalid_instr_flag_START (4)
#define SOC_PMCTRL_TASKCTRL_E_0_taskctrle_invalid_instr_flag_END (4)
#define SOC_PMCTRL_TASKCTRL_E_0_taskctrle_halt_flag_START (5)
#define SOC_PMCTRL_TASKCTRL_E_0_taskctrle_halt_flag_END (5)
#define SOC_PMCTRL_TASKCTRL_E_0_taskctrle_cur_decoding_instr_addr_START (8)
#define SOC_PMCTRL_TASKCTRL_E_0_taskctrle_cur_decoding_instr_addr_END (19)
#define SOC_PMCTRL_TASKCTRL_E_0_taskctrle_cur_fetch_num_START (20)
#define SOC_PMCTRL_TASKCTRL_E_0_taskctrle_cur_fetch_num_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrle_cur_task_stat : 8;
        unsigned int taskctrle_num_of_end_instr : 8;
        unsigned int reserved_0 : 12;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_E_1_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_E_1_taskctrle_cur_task_stat_START (0)
#define SOC_PMCTRL_TASKCTRL_E_1_taskctrle_cur_task_stat_END (7)
#define SOC_PMCTRL_TASKCTRL_E_1_taskctrle_num_of_end_instr_START (8)
#define SOC_PMCTRL_TASKCTRL_E_1_taskctrle_num_of_end_instr_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 8;
        unsigned int reserved_1: 8;
        unsigned int reserved_2: 12;
        unsigned int reserved_3: 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_E_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 8;
        unsigned int reserved_1: 8;
        unsigned int reserved_2: 12;
        unsigned int reserved_3: 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_E_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrlf_dvfs_en : 1;
        unsigned int taskctrlf_next_step_cfg : 1;
        unsigned int taskctrlf_escape_cfg : 1;
        unsigned int taskctrlf_invalid_instr_flag_clr : 1;
        unsigned int taskctrlf_invalid_instr_flag : 1;
        unsigned int taskctrlf_halt_flag : 1;
        unsigned int reserved_0 : 2;
        unsigned int taskctrlf_cur_decoding_instr_addr : 12;
        unsigned int taskctrlf_cur_fetch_num : 8;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_F_0_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_F_0_taskctrlf_dvfs_en_START (0)
#define SOC_PMCTRL_TASKCTRL_F_0_taskctrlf_dvfs_en_END (0)
#define SOC_PMCTRL_TASKCTRL_F_0_taskctrlf_next_step_cfg_START (1)
#define SOC_PMCTRL_TASKCTRL_F_0_taskctrlf_next_step_cfg_END (1)
#define SOC_PMCTRL_TASKCTRL_F_0_taskctrlf_escape_cfg_START (2)
#define SOC_PMCTRL_TASKCTRL_F_0_taskctrlf_escape_cfg_END (2)
#define SOC_PMCTRL_TASKCTRL_F_0_taskctrlf_invalid_instr_flag_clr_START (3)
#define SOC_PMCTRL_TASKCTRL_F_0_taskctrlf_invalid_instr_flag_clr_END (3)
#define SOC_PMCTRL_TASKCTRL_F_0_taskctrlf_invalid_instr_flag_START (4)
#define SOC_PMCTRL_TASKCTRL_F_0_taskctrlf_invalid_instr_flag_END (4)
#define SOC_PMCTRL_TASKCTRL_F_0_taskctrlf_halt_flag_START (5)
#define SOC_PMCTRL_TASKCTRL_F_0_taskctrlf_halt_flag_END (5)
#define SOC_PMCTRL_TASKCTRL_F_0_taskctrlf_cur_decoding_instr_addr_START (8)
#define SOC_PMCTRL_TASKCTRL_F_0_taskctrlf_cur_decoding_instr_addr_END (19)
#define SOC_PMCTRL_TASKCTRL_F_0_taskctrlf_cur_fetch_num_START (20)
#define SOC_PMCTRL_TASKCTRL_F_0_taskctrlf_cur_fetch_num_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrlf_cur_task_stat : 8;
        unsigned int taskctrlf_num_of_end_instr : 8;
        unsigned int reserved_0 : 12;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_F_1_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_F_1_taskctrlf_cur_task_stat_START (0)
#define SOC_PMCTRL_TASKCTRL_F_1_taskctrlf_cur_task_stat_END (7)
#define SOC_PMCTRL_TASKCTRL_F_1_taskctrlf_num_of_end_instr_START (8)
#define SOC_PMCTRL_TASKCTRL_F_1_taskctrlf_num_of_end_instr_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 8;
        unsigned int reserved_1: 8;
        unsigned int reserved_2: 12;
        unsigned int reserved_3: 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_F_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 8;
        unsigned int reserved_1: 8;
        unsigned int reserved_2: 12;
        unsigned int reserved_3: 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_F_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrlg_dvfs_en : 1;
        unsigned int taskctrlg_next_step_cfg : 1;
        unsigned int taskctrlg_escape_cfg : 1;
        unsigned int taskctrlg_invalid_instr_flag_clr : 1;
        unsigned int taskctrlg_invalid_instr_flag : 1;
        unsigned int taskctrlg_halt_flag : 1;
        unsigned int reserved_0 : 2;
        unsigned int taskctrlg_cur_decoding_instr_addr : 12;
        unsigned int taskctrlg_cur_fetch_num : 8;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_G_0_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_G_0_taskctrlg_dvfs_en_START (0)
#define SOC_PMCTRL_TASKCTRL_G_0_taskctrlg_dvfs_en_END (0)
#define SOC_PMCTRL_TASKCTRL_G_0_taskctrlg_next_step_cfg_START (1)
#define SOC_PMCTRL_TASKCTRL_G_0_taskctrlg_next_step_cfg_END (1)
#define SOC_PMCTRL_TASKCTRL_G_0_taskctrlg_escape_cfg_START (2)
#define SOC_PMCTRL_TASKCTRL_G_0_taskctrlg_escape_cfg_END (2)
#define SOC_PMCTRL_TASKCTRL_G_0_taskctrlg_invalid_instr_flag_clr_START (3)
#define SOC_PMCTRL_TASKCTRL_G_0_taskctrlg_invalid_instr_flag_clr_END (3)
#define SOC_PMCTRL_TASKCTRL_G_0_taskctrlg_invalid_instr_flag_START (4)
#define SOC_PMCTRL_TASKCTRL_G_0_taskctrlg_invalid_instr_flag_END (4)
#define SOC_PMCTRL_TASKCTRL_G_0_taskctrlg_halt_flag_START (5)
#define SOC_PMCTRL_TASKCTRL_G_0_taskctrlg_halt_flag_END (5)
#define SOC_PMCTRL_TASKCTRL_G_0_taskctrlg_cur_decoding_instr_addr_START (8)
#define SOC_PMCTRL_TASKCTRL_G_0_taskctrlg_cur_decoding_instr_addr_END (19)
#define SOC_PMCTRL_TASKCTRL_G_0_taskctrlg_cur_fetch_num_START (20)
#define SOC_PMCTRL_TASKCTRL_G_0_taskctrlg_cur_fetch_num_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrlg_cur_task_stat : 8;
        unsigned int taskctrlg_num_of_end_instr : 8;
        unsigned int reserved_0 : 12;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_G_1_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_G_1_taskctrlg_cur_task_stat_START (0)
#define SOC_PMCTRL_TASKCTRL_G_1_taskctrlg_cur_task_stat_END (7)
#define SOC_PMCTRL_TASKCTRL_G_1_taskctrlg_num_of_end_instr_START (8)
#define SOC_PMCTRL_TASKCTRL_G_1_taskctrlg_num_of_end_instr_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 8;
        unsigned int reserved_1: 8;
        unsigned int reserved_2: 12;
        unsigned int reserved_3: 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_G_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 8;
        unsigned int reserved_1: 8;
        unsigned int reserved_2: 12;
        unsigned int reserved_3: 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_G_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrlh_dvfs_en : 1;
        unsigned int taskctrlh_next_step_cfg : 1;
        unsigned int taskctrlh_escape_cfg : 1;
        unsigned int taskctrlh_invalid_instr_flag_clr : 1;
        unsigned int taskctrlh_invalid_instr_flag : 1;
        unsigned int taskctrlh_halt_flag : 1;
        unsigned int reserved_0 : 2;
        unsigned int taskctrlh_cur_decoding_instr_addr : 12;
        unsigned int taskctrlh_cur_fetch_num : 8;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_H_0_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_H_0_taskctrlh_dvfs_en_START (0)
#define SOC_PMCTRL_TASKCTRL_H_0_taskctrlh_dvfs_en_END (0)
#define SOC_PMCTRL_TASKCTRL_H_0_taskctrlh_next_step_cfg_START (1)
#define SOC_PMCTRL_TASKCTRL_H_0_taskctrlh_next_step_cfg_END (1)
#define SOC_PMCTRL_TASKCTRL_H_0_taskctrlh_escape_cfg_START (2)
#define SOC_PMCTRL_TASKCTRL_H_0_taskctrlh_escape_cfg_END (2)
#define SOC_PMCTRL_TASKCTRL_H_0_taskctrlh_invalid_instr_flag_clr_START (3)
#define SOC_PMCTRL_TASKCTRL_H_0_taskctrlh_invalid_instr_flag_clr_END (3)
#define SOC_PMCTRL_TASKCTRL_H_0_taskctrlh_invalid_instr_flag_START (4)
#define SOC_PMCTRL_TASKCTRL_H_0_taskctrlh_invalid_instr_flag_END (4)
#define SOC_PMCTRL_TASKCTRL_H_0_taskctrlh_halt_flag_START (5)
#define SOC_PMCTRL_TASKCTRL_H_0_taskctrlh_halt_flag_END (5)
#define SOC_PMCTRL_TASKCTRL_H_0_taskctrlh_cur_decoding_instr_addr_START (8)
#define SOC_PMCTRL_TASKCTRL_H_0_taskctrlh_cur_decoding_instr_addr_END (19)
#define SOC_PMCTRL_TASKCTRL_H_0_taskctrlh_cur_fetch_num_START (20)
#define SOC_PMCTRL_TASKCTRL_H_0_taskctrlh_cur_fetch_num_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int taskctrlh_cur_task_stat : 8;
        unsigned int taskctrlh_num_of_end_instr : 8;
        unsigned int reserved_0 : 12;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_H_1_UNION;
#endif
#define SOC_PMCTRL_TASKCTRL_H_1_taskctrlh_cur_task_stat_START (0)
#define SOC_PMCTRL_TASKCTRL_H_1_taskctrlh_cur_task_stat_END (7)
#define SOC_PMCTRL_TASKCTRL_H_1_taskctrlh_num_of_end_instr_START (8)
#define SOC_PMCTRL_TASKCTRL_H_1_taskctrlh_num_of_end_instr_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 8;
        unsigned int reserved_1: 8;
        unsigned int reserved_2: 12;
        unsigned int reserved_3: 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_H_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 8;
        unsigned int reserved_1: 8;
        unsigned int reserved_2: 12;
        unsigned int reserved_3: 4;
    } reg;
} SOC_PMCTRL_TASKCTRL_H_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsma_logic_vol_ctrl_src_0 : 4;
        unsigned int cofsma_logic_vol_ctrl_src_1 : 4;
        unsigned int cofsma_logic_vol_ctrl_src_2 : 4;
        unsigned int cofsma_logic_vol_ctrl_src_3 : 4;
        unsigned int cofsma_mem_vol_ctrl_src_0 : 4;
        unsigned int cofsma_mem_vol_ctrl_src_1 : 4;
        unsigned int cofsma_mem_vol_ctrl_src_2 : 4;
        unsigned int cofsma_mem_vol_ctrl_src_3 : 4;
    } reg;
} SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_UNION;
#endif
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_cofsma_logic_vol_ctrl_src_0_START (0)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_cofsma_logic_vol_ctrl_src_0_END (3)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_cofsma_logic_vol_ctrl_src_1_START (4)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_cofsma_logic_vol_ctrl_src_1_END (7)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_cofsma_logic_vol_ctrl_src_2_START (8)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_cofsma_logic_vol_ctrl_src_2_END (11)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_cofsma_logic_vol_ctrl_src_3_START (12)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_cofsma_logic_vol_ctrl_src_3_END (15)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_cofsma_mem_vol_ctrl_src_0_START (16)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_cofsma_mem_vol_ctrl_src_0_END (19)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_cofsma_mem_vol_ctrl_src_1_START (20)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_cofsma_mem_vol_ctrl_src_1_END (23)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_cofsma_mem_vol_ctrl_src_2_START (24)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_cofsma_mem_vol_ctrl_src_2_END (27)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_cofsma_mem_vol_ctrl_src_3_START (28)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_A_cofsma_mem_vol_ctrl_src_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmb_logic_vol_ctrl_src_0 : 4;
        unsigned int cofsmb_logic_vol_ctrl_src_1 : 4;
        unsigned int cofsmb_logic_vol_ctrl_src_2 : 4;
        unsigned int cofsmb_logic_vol_ctrl_src_3 : 4;
        unsigned int cofsmb_mem_vol_ctrl_src_0 : 4;
        unsigned int cofsmb_mem_vol_ctrl_src_1 : 4;
        unsigned int cofsmb_mem_vol_ctrl_src_2 : 4;
        unsigned int cofsmb_mem_vol_ctrl_src_3 : 4;
    } reg;
} SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_UNION;
#endif
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_cofsmb_logic_vol_ctrl_src_0_START (0)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_cofsmb_logic_vol_ctrl_src_0_END (3)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_cofsmb_logic_vol_ctrl_src_1_START (4)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_cofsmb_logic_vol_ctrl_src_1_END (7)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_cofsmb_logic_vol_ctrl_src_2_START (8)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_cofsmb_logic_vol_ctrl_src_2_END (11)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_cofsmb_logic_vol_ctrl_src_3_START (12)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_cofsmb_logic_vol_ctrl_src_3_END (15)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_cofsmb_mem_vol_ctrl_src_0_START (16)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_cofsmb_mem_vol_ctrl_src_0_END (19)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_cofsmb_mem_vol_ctrl_src_1_START (20)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_cofsmb_mem_vol_ctrl_src_1_END (23)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_cofsmb_mem_vol_ctrl_src_2_START (24)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_cofsmb_mem_vol_ctrl_src_2_END (27)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_cofsmb_mem_vol_ctrl_src_3_START (28)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_B_cofsmb_mem_vol_ctrl_src_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmc_logic_vol_ctrl_src_0 : 4;
        unsigned int cofsmc_logic_vol_ctrl_src_1 : 4;
        unsigned int cofsmc_logic_vol_ctrl_src_2 : 4;
        unsigned int cofsmc_logic_vol_ctrl_src_3 : 4;
        unsigned int cofsmc_mem_vol_ctrl_src_0 : 4;
        unsigned int cofsmc_mem_vol_ctrl_src_1 : 4;
        unsigned int cofsmc_mem_vol_ctrl_src_2 : 4;
        unsigned int cofsmc_mem_vol_ctrl_src_3 : 4;
    } reg;
} SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_UNION;
#endif
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_cofsmc_logic_vol_ctrl_src_0_START (0)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_cofsmc_logic_vol_ctrl_src_0_END (3)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_cofsmc_logic_vol_ctrl_src_1_START (4)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_cofsmc_logic_vol_ctrl_src_1_END (7)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_cofsmc_logic_vol_ctrl_src_2_START (8)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_cofsmc_logic_vol_ctrl_src_2_END (11)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_cofsmc_logic_vol_ctrl_src_3_START (12)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_cofsmc_logic_vol_ctrl_src_3_END (15)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_cofsmc_mem_vol_ctrl_src_0_START (16)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_cofsmc_mem_vol_ctrl_src_0_END (19)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_cofsmc_mem_vol_ctrl_src_1_START (20)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_cofsmc_mem_vol_ctrl_src_1_END (23)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_cofsmc_mem_vol_ctrl_src_2_START (24)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_cofsmc_mem_vol_ctrl_src_2_END (27)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_cofsmc_mem_vol_ctrl_src_3_START (28)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_C_cofsmc_mem_vol_ctrl_src_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmd_logic_vol_ctrl_src_0 : 4;
        unsigned int cofsmd_logic_vol_ctrl_src_1 : 4;
        unsigned int cofsmd_logic_vol_ctrl_src_2 : 4;
        unsigned int cofsmd_logic_vol_ctrl_src_3 : 4;
        unsigned int cofsmd_mem_vol_ctrl_src_0 : 4;
        unsigned int cofsmd_mem_vol_ctrl_src_1 : 4;
        unsigned int cofsmd_mem_vol_ctrl_src_2 : 4;
        unsigned int cofsmd_mem_vol_ctrl_src_3 : 4;
    } reg;
} SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_UNION;
#endif
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_cofsmd_logic_vol_ctrl_src_0_START (0)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_cofsmd_logic_vol_ctrl_src_0_END (3)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_cofsmd_logic_vol_ctrl_src_1_START (4)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_cofsmd_logic_vol_ctrl_src_1_END (7)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_cofsmd_logic_vol_ctrl_src_2_START (8)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_cofsmd_logic_vol_ctrl_src_2_END (11)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_cofsmd_logic_vol_ctrl_src_3_START (12)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_cofsmd_logic_vol_ctrl_src_3_END (15)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_cofsmd_mem_vol_ctrl_src_0_START (16)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_cofsmd_mem_vol_ctrl_src_0_END (19)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_cofsmd_mem_vol_ctrl_src_1_START (20)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_cofsmd_mem_vol_ctrl_src_1_END (23)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_cofsmd_mem_vol_ctrl_src_2_START (24)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_cofsmd_mem_vol_ctrl_src_2_END (27)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_cofsmd_mem_vol_ctrl_src_3_START (28)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_D_cofsmd_mem_vol_ctrl_src_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsme_logic_vol_ctrl_src_0 : 4;
        unsigned int cofsme_logic_vol_ctrl_src_1 : 4;
        unsigned int cofsme_logic_vol_ctrl_src_2 : 4;
        unsigned int cofsme_logic_vol_ctrl_src_3 : 4;
        unsigned int cofsme_mem_vol_ctrl_src_0 : 4;
        unsigned int cofsme_mem_vol_ctrl_src_1 : 4;
        unsigned int cofsme_mem_vol_ctrl_src_2 : 4;
        unsigned int cofsme_mem_vol_ctrl_src_3 : 4;
    } reg;
} SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_UNION;
#endif
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_cofsme_logic_vol_ctrl_src_0_START (0)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_cofsme_logic_vol_ctrl_src_0_END (3)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_cofsme_logic_vol_ctrl_src_1_START (4)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_cofsme_logic_vol_ctrl_src_1_END (7)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_cofsme_logic_vol_ctrl_src_2_START (8)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_cofsme_logic_vol_ctrl_src_2_END (11)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_cofsme_logic_vol_ctrl_src_3_START (12)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_cofsme_logic_vol_ctrl_src_3_END (15)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_cofsme_mem_vol_ctrl_src_0_START (16)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_cofsme_mem_vol_ctrl_src_0_END (19)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_cofsme_mem_vol_ctrl_src_1_START (20)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_cofsme_mem_vol_ctrl_src_1_END (23)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_cofsme_mem_vol_ctrl_src_2_START (24)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_cofsme_mem_vol_ctrl_src_2_END (27)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_cofsme_mem_vol_ctrl_src_3_START (28)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_E_cofsme_mem_vol_ctrl_src_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmf_logic_vol_ctrl_src_0 : 4;
        unsigned int cofsmf_logic_vol_ctrl_src_1 : 4;
        unsigned int cofsmf_logic_vol_ctrl_src_2 : 4;
        unsigned int cofsmf_logic_vol_ctrl_src_3 : 4;
        unsigned int cofsmf_mem_vol_ctrl_src_0 : 4;
        unsigned int cofsmf_mem_vol_ctrl_src_1 : 4;
        unsigned int cofsmf_mem_vol_ctrl_src_2 : 4;
        unsigned int cofsmf_mem_vol_ctrl_src_3 : 4;
    } reg;
} SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_UNION;
#endif
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_cofsmf_logic_vol_ctrl_src_0_START (0)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_cofsmf_logic_vol_ctrl_src_0_END (3)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_cofsmf_logic_vol_ctrl_src_1_START (4)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_cofsmf_logic_vol_ctrl_src_1_END (7)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_cofsmf_logic_vol_ctrl_src_2_START (8)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_cofsmf_logic_vol_ctrl_src_2_END (11)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_cofsmf_logic_vol_ctrl_src_3_START (12)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_cofsmf_logic_vol_ctrl_src_3_END (15)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_cofsmf_mem_vol_ctrl_src_0_START (16)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_cofsmf_mem_vol_ctrl_src_0_END (19)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_cofsmf_mem_vol_ctrl_src_1_START (20)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_cofsmf_mem_vol_ctrl_src_1_END (23)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_cofsmf_mem_vol_ctrl_src_2_START (24)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_cofsmf_mem_vol_ctrl_src_2_END (27)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_cofsmf_mem_vol_ctrl_src_3_START (28)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_F_cofsmf_mem_vol_ctrl_src_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmg_logic_vol_ctrl_src_0 : 4;
        unsigned int cofsmg_logic_vol_ctrl_src_1 : 4;
        unsigned int cofsmg_logic_vol_ctrl_src_2 : 4;
        unsigned int cofsmg_logic_vol_ctrl_src_3 : 4;
        unsigned int cofsmg_mem_vol_ctrl_src_0 : 4;
        unsigned int cofsmg_mem_vol_ctrl_src_1 : 4;
        unsigned int cofsmg_mem_vol_ctrl_src_2 : 4;
        unsigned int cofsmg_mem_vol_ctrl_src_3 : 4;
    } reg;
} SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_UNION;
#endif
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_cofsmg_logic_vol_ctrl_src_0_START (0)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_cofsmg_logic_vol_ctrl_src_0_END (3)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_cofsmg_logic_vol_ctrl_src_1_START (4)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_cofsmg_logic_vol_ctrl_src_1_END (7)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_cofsmg_logic_vol_ctrl_src_2_START (8)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_cofsmg_logic_vol_ctrl_src_2_END (11)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_cofsmg_logic_vol_ctrl_src_3_START (12)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_cofsmg_logic_vol_ctrl_src_3_END (15)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_cofsmg_mem_vol_ctrl_src_0_START (16)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_cofsmg_mem_vol_ctrl_src_0_END (19)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_cofsmg_mem_vol_ctrl_src_1_START (20)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_cofsmg_mem_vol_ctrl_src_1_END (23)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_cofsmg_mem_vol_ctrl_src_2_START (24)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_cofsmg_mem_vol_ctrl_src_2_END (27)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_cofsmg_mem_vol_ctrl_src_3_START (28)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_G_cofsmg_mem_vol_ctrl_src_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmh_logic_vol_ctrl_src_0 : 4;
        unsigned int cofsmh_logic_vol_ctrl_src_1 : 4;
        unsigned int cofsmh_logic_vol_ctrl_src_2 : 4;
        unsigned int cofsmh_logic_vol_ctrl_src_3 : 4;
        unsigned int cofsmh_mem_vol_ctrl_src_0 : 4;
        unsigned int cofsmh_mem_vol_ctrl_src_1 : 4;
        unsigned int cofsmh_mem_vol_ctrl_src_2 : 4;
        unsigned int cofsmh_mem_vol_ctrl_src_3 : 4;
    } reg;
} SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_UNION;
#endif
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_cofsmh_logic_vol_ctrl_src_0_START (0)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_cofsmh_logic_vol_ctrl_src_0_END (3)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_cofsmh_logic_vol_ctrl_src_1_START (4)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_cofsmh_logic_vol_ctrl_src_1_END (7)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_cofsmh_logic_vol_ctrl_src_2_START (8)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_cofsmh_logic_vol_ctrl_src_2_END (11)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_cofsmh_logic_vol_ctrl_src_3_START (12)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_cofsmh_logic_vol_ctrl_src_3_END (15)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_cofsmh_mem_vol_ctrl_src_0_START (16)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_cofsmh_mem_vol_ctrl_src_0_END (19)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_cofsmh_mem_vol_ctrl_src_1_START (20)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_cofsmh_mem_vol_ctrl_src_1_END (23)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_cofsmh_mem_vol_ctrl_src_2_START (24)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_cofsmh_mem_vol_ctrl_src_2_END (27)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_cofsmh_mem_vol_ctrl_src_3_START (28)
#define SOC_PMCTRL_COFSM_VOL_CTRL_SRC_H_cofsmh_mem_vol_ctrl_src_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsma_freq_ctrl_src : 4;
        unsigned int cofsmb_freq_ctrl_src : 4;
        unsigned int cofsmc_freq_ctrl_src : 4;
        unsigned int cofsmd_freq_ctrl_src : 4;
        unsigned int cofsme_freq_ctrl_src : 4;
        unsigned int cofsmf_freq_ctrl_src : 4;
        unsigned int cofsmg_freq_ctrl_src : 4;
        unsigned int cofsmh_freq_ctrl_src : 4;
    } reg;
} SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_UNION;
#endif
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_cofsma_freq_ctrl_src_START (0)
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_cofsma_freq_ctrl_src_END (3)
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_cofsmb_freq_ctrl_src_START (4)
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_cofsmb_freq_ctrl_src_END (7)
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_cofsmc_freq_ctrl_src_START (8)
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_cofsmc_freq_ctrl_src_END (11)
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_cofsmd_freq_ctrl_src_START (12)
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_cofsmd_freq_ctrl_src_END (15)
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_cofsme_freq_ctrl_src_START (16)
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_cofsme_freq_ctrl_src_END (19)
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_cofsmf_freq_ctrl_src_START (20)
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_cofsmf_freq_ctrl_src_END (23)
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_cofsmg_freq_ctrl_src_START (24)
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_cofsmg_freq_ctrl_src_END (27)
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_cofsmh_freq_ctrl_src_START (28)
#define SOC_PMCTRL_COFSM_FREQ_CTRL_SRC_cofsmh_freq_ctrl_src_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int cofsme_logic_vol_sendout : 1;
        unsigned int reserved_8 : 1;
        unsigned int cofsmf_logic_vol_sendout : 1;
        unsigned int reserved_9 : 1;
        unsigned int cofsmg_logic_vol_sendout : 1;
        unsigned int reserved_10 : 1;
        unsigned int cofsmh_logic_vol_sendout : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 16;
    } reg;
} SOC_PMCTRL_COFSM_VOL_SENDOUT_UNION;
#endif
#define SOC_PMCTRL_COFSM_VOL_SENDOUT_cofsme_logic_vol_sendout_START (8)
#define SOC_PMCTRL_COFSM_VOL_SENDOUT_cofsme_logic_vol_sendout_END (8)
#define SOC_PMCTRL_COFSM_VOL_SENDOUT_cofsmf_logic_vol_sendout_START (10)
#define SOC_PMCTRL_COFSM_VOL_SENDOUT_cofsmf_logic_vol_sendout_END (10)
#define SOC_PMCTRL_COFSM_VOL_SENDOUT_cofsmg_logic_vol_sendout_START (12)
#define SOC_PMCTRL_COFSM_VOL_SENDOUT_cofsmg_logic_vol_sendout_END (12)
#define SOC_PMCTRL_COFSM_VOL_SENDOUT_cofsmh_logic_vol_sendout_START (14)
#define SOC_PMCTRL_COFSM_VOL_SENDOUT_cofsmh_logic_vol_sendout_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsma_cpu_div_sft_idx : 16;
        unsigned int cofsma_cpu_div_sft_cfg : 1;
        unsigned int cofsma_sw2ppll0_req : 1;
        unsigned int cofsma_sw2apll_req : 1;
        unsigned int cofsma_pll_sw_stat : 1;
        unsigned int cofsma_ppll0_div_stat : 4;
        unsigned int cofsma_ppll0_div_sft_idx : 4;
        unsigned int cofsma_ppll0_div_sft_cfg : 1;
        unsigned int cofsma_gt_clk_ppll0_gt_sft_set : 1;
        unsigned int cofsma_gt_clk_ppll0_gt_sft_clr : 1;
        unsigned int cofsma_gt_clk_ppll0_gt_stat : 1;
    } reg;
} SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_cpu_div_sft_idx_START (0)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_cpu_div_sft_idx_END (15)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_cpu_div_sft_cfg_START (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_cpu_div_sft_cfg_END (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_sw2ppll0_req_START (17)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_sw2ppll0_req_END (17)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_sw2apll_req_START (18)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_sw2apll_req_END (18)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_pll_sw_stat_START (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_pll_sw_stat_END (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_ppll0_div_stat_START (20)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_ppll0_div_stat_END (23)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_ppll0_div_sft_idx_START (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_ppll0_div_sft_idx_END (27)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_ppll0_div_sft_cfg_START (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_ppll0_div_sft_cfg_END (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_gt_clk_ppll0_gt_sft_set_START (29)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_gt_clk_ppll0_gt_sft_set_END (29)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_gt_clk_ppll0_gt_sft_clr_START (30)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_gt_clk_ppll0_gt_sft_clr_END (30)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_gt_clk_ppll0_gt_stat_START (31)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_0_cofsma_gt_clk_ppll0_gt_stat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsma_cpu_div_stat : 16;
        unsigned int cofsma_vdm_div_stat : 4;
        unsigned int cofsma_vdm_div_sft_idx : 4;
        unsigned int cofsma_vdm_div_sft_cfg : 1;
        unsigned int cofsma_cur_clk_src_stat : 3;
        unsigned int cofsma_cur_apll_freq_stat : 3;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_1_cofsma_cpu_div_stat_START (0)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_1_cofsma_cpu_div_stat_END (15)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_1_cofsma_vdm_div_stat_START (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_1_cofsma_vdm_div_stat_END (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_1_cofsma_vdm_div_sft_idx_START (20)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_1_cofsma_vdm_div_sft_idx_END (23)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_1_cofsma_vdm_div_sft_cfg_START (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_1_cofsma_vdm_div_sft_cfg_END (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_1_cofsma_cur_clk_src_stat_START (25)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_1_cofsma_cur_clk_src_stat_END (27)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_1_cofsma_cur_apll_freq_stat_START (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_A_1_cofsma_cur_apll_freq_stat_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmb_cpu_div_sft_idx : 16;
        unsigned int cofsmb_cpu_div_sft_cfg : 1;
        unsigned int cofsmb_sw2ppll0_req : 1;
        unsigned int cofsmb_sw2apll_req : 1;
        unsigned int cofsmb_pll_sw_stat : 1;
        unsigned int cofsmb_ppll0_div_stat : 4;
        unsigned int cofsmb_ppll0_div_sft_idx : 4;
        unsigned int cofsmb_ppll0_div_sft_cfg : 1;
        unsigned int cofsmb_gt_clk_ppll0_gt_sft_set : 1;
        unsigned int cofsmb_gt_clk_ppll0_gt_sft_clr : 1;
        unsigned int cofsmb_gt_clk_ppll0_gt_stat : 1;
    } reg;
} SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_cpu_div_sft_idx_START (0)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_cpu_div_sft_idx_END (15)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_cpu_div_sft_cfg_START (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_cpu_div_sft_cfg_END (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_sw2ppll0_req_START (17)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_sw2ppll0_req_END (17)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_sw2apll_req_START (18)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_sw2apll_req_END (18)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_pll_sw_stat_START (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_pll_sw_stat_END (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_ppll0_div_stat_START (20)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_ppll0_div_stat_END (23)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_ppll0_div_sft_idx_START (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_ppll0_div_sft_idx_END (27)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_ppll0_div_sft_cfg_START (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_ppll0_div_sft_cfg_END (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_gt_clk_ppll0_gt_sft_set_START (29)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_gt_clk_ppll0_gt_sft_set_END (29)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_gt_clk_ppll0_gt_sft_clr_START (30)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_gt_clk_ppll0_gt_sft_clr_END (30)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_gt_clk_ppll0_gt_stat_START (31)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_0_cofsmb_gt_clk_ppll0_gt_stat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmb_cpu_div_stat : 16;
        unsigned int cofsmb_vdm_div_stat : 4;
        unsigned int cofsmb_vdm_div_sft_idx : 4;
        unsigned int cofsmb_vdm_div_sft_cfg : 1;
        unsigned int cofsmb_cur_clk_src_stat : 3;
        unsigned int cofsmb_cur_apll_freq_stat : 3;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_1_cofsmb_cpu_div_stat_START (0)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_1_cofsmb_cpu_div_stat_END (15)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_1_cofsmb_vdm_div_stat_START (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_1_cofsmb_vdm_div_stat_END (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_1_cofsmb_vdm_div_sft_idx_START (20)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_1_cofsmb_vdm_div_sft_idx_END (23)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_1_cofsmb_vdm_div_sft_cfg_START (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_1_cofsmb_vdm_div_sft_cfg_END (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_1_cofsmb_cur_clk_src_stat_START (25)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_1_cofsmb_cur_clk_src_stat_END (27)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_1_cofsmb_cur_apll_freq_stat_START (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_B_1_cofsmb_cur_apll_freq_stat_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmc_cpu_div_sft_idx : 16;
        unsigned int cofsmc_cpu_div_sft_cfg : 1;
        unsigned int cofsmc_sw2ppll0_req : 1;
        unsigned int cofsmc_sw2apll_req : 1;
        unsigned int cofsmc_pll_sw_stat : 1;
        unsigned int cofsmc_ppll0_div_stat : 4;
        unsigned int cofsmc_ppll0_div_sft_idx : 4;
        unsigned int cofsmc_ppll0_div_sft_cfg : 1;
        unsigned int cofsmc_gt_clk_ppll0_gt_sft_set : 1;
        unsigned int cofsmc_gt_clk_ppll0_gt_sft_clr : 1;
        unsigned int cofsmc_gt_clk_ppll0_gt_stat : 1;
    } reg;
} SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_cpu_div_sft_idx_START (0)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_cpu_div_sft_idx_END (15)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_cpu_div_sft_cfg_START (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_cpu_div_sft_cfg_END (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_sw2ppll0_req_START (17)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_sw2ppll0_req_END (17)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_sw2apll_req_START (18)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_sw2apll_req_END (18)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_pll_sw_stat_START (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_pll_sw_stat_END (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_ppll0_div_stat_START (20)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_ppll0_div_stat_END (23)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_ppll0_div_sft_idx_START (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_ppll0_div_sft_idx_END (27)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_ppll0_div_sft_cfg_START (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_ppll0_div_sft_cfg_END (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_gt_clk_ppll0_gt_sft_set_START (29)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_gt_clk_ppll0_gt_sft_set_END (29)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_gt_clk_ppll0_gt_sft_clr_START (30)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_gt_clk_ppll0_gt_sft_clr_END (30)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_gt_clk_ppll0_gt_stat_START (31)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_0_cofsmc_gt_clk_ppll0_gt_stat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmc_cpu_div_stat : 16;
        unsigned int cofsmc_vdm_div_stat : 4;
        unsigned int cofsmc_vdm_div_sft_idx : 4;
        unsigned int cofsmc_vdm_div_sft_cfg : 1;
        unsigned int cofsmc_cur_clk_src_stat : 3;
        unsigned int cofsmc_cur_apll_freq_stat : 3;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_1_cofsmc_cpu_div_stat_START (0)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_1_cofsmc_cpu_div_stat_END (15)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_1_cofsmc_vdm_div_stat_START (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_1_cofsmc_vdm_div_stat_END (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_1_cofsmc_vdm_div_sft_idx_START (20)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_1_cofsmc_vdm_div_sft_idx_END (23)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_1_cofsmc_vdm_div_sft_cfg_START (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_1_cofsmc_vdm_div_sft_cfg_END (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_1_cofsmc_cur_clk_src_stat_START (25)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_1_cofsmc_cur_clk_src_stat_END (27)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_1_cofsmc_cur_apll_freq_stat_START (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_C_1_cofsmc_cur_apll_freq_stat_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmd_cpu_div_sft_idx : 16;
        unsigned int cofsmd_cpu_div_sft_cfg : 1;
        unsigned int cofsmd_sw2ppll0_req : 1;
        unsigned int cofsmd_sw2apll_req : 1;
        unsigned int cofsmd_pll_sw_stat : 1;
        unsigned int cofsmd_ppll0_div_stat : 4;
        unsigned int cofsmd_ppll0_div_sft_idx : 4;
        unsigned int cofsmd_ppll0_div_sft_cfg : 1;
        unsigned int cofsmd_gt_clk_ppll0_gt_sft_set : 1;
        unsigned int cofsmd_gt_clk_ppll0_gt_sft_clr : 1;
        unsigned int cofsmd_gt_clk_ppll0_gt_stat : 1;
    } reg;
} SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_cpu_div_sft_idx_START (0)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_cpu_div_sft_idx_END (15)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_cpu_div_sft_cfg_START (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_cpu_div_sft_cfg_END (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_sw2ppll0_req_START (17)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_sw2ppll0_req_END (17)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_sw2apll_req_START (18)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_sw2apll_req_END (18)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_pll_sw_stat_START (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_pll_sw_stat_END (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_ppll0_div_stat_START (20)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_ppll0_div_stat_END (23)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_ppll0_div_sft_idx_START (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_ppll0_div_sft_idx_END (27)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_ppll0_div_sft_cfg_START (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_ppll0_div_sft_cfg_END (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_gt_clk_ppll0_gt_sft_set_START (29)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_gt_clk_ppll0_gt_sft_set_END (29)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_gt_clk_ppll0_gt_sft_clr_START (30)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_gt_clk_ppll0_gt_sft_clr_END (30)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_gt_clk_ppll0_gt_stat_START (31)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_0_cofsmd_gt_clk_ppll0_gt_stat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmd_cpu_div_stat : 16;
        unsigned int cofsmd_vdm_div_stat : 4;
        unsigned int cofsmd_vdm_div_sft_idx : 4;
        unsigned int cofsmd_vdm_div_sft_cfg : 1;
        unsigned int cofsmd_cur_clk_src_stat : 3;
        unsigned int cofsmd_cur_apll_freq_stat : 3;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_1_cofsmd_cpu_div_stat_START (0)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_1_cofsmd_cpu_div_stat_END (15)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_1_cofsmd_vdm_div_stat_START (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_1_cofsmd_vdm_div_stat_END (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_1_cofsmd_vdm_div_sft_idx_START (20)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_1_cofsmd_vdm_div_sft_idx_END (23)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_1_cofsmd_vdm_div_sft_cfg_START (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_1_cofsmd_vdm_div_sft_cfg_END (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_1_cofsmd_cur_clk_src_stat_START (25)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_1_cofsmd_cur_clk_src_stat_END (27)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_1_cofsmd_cur_apll_freq_stat_START (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_D_1_cofsmd_cur_apll_freq_stat_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsme_cpu_div_sft_idx : 16;
        unsigned int cofsme_cpu_div_sft_cfg : 1;
        unsigned int cofsme_sw2ppll0_req : 1;
        unsigned int cofsme_sw2apll_req : 1;
        unsigned int cofsme_pll_sw_stat : 1;
        unsigned int cofsme_ppll0_div_stat : 4;
        unsigned int cofsme_ppll0_div_sft_idx : 4;
        unsigned int cofsme_ppll0_div_sft_cfg : 1;
        unsigned int cofsme_gt_clk_ppll0_gt_sft_set : 1;
        unsigned int cofsme_gt_clk_ppll0_gt_sft_clr : 1;
        unsigned int cofsme_gt_clk_ppll0_gt_stat : 1;
    } reg;
} SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_cpu_div_sft_idx_START (0)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_cpu_div_sft_idx_END (15)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_cpu_div_sft_cfg_START (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_cpu_div_sft_cfg_END (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_sw2ppll0_req_START (17)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_sw2ppll0_req_END (17)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_sw2apll_req_START (18)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_sw2apll_req_END (18)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_pll_sw_stat_START (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_pll_sw_stat_END (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_ppll0_div_stat_START (20)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_ppll0_div_stat_END (23)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_ppll0_div_sft_idx_START (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_ppll0_div_sft_idx_END (27)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_ppll0_div_sft_cfg_START (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_ppll0_div_sft_cfg_END (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_gt_clk_ppll0_gt_sft_set_START (29)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_gt_clk_ppll0_gt_sft_set_END (29)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_gt_clk_ppll0_gt_sft_clr_START (30)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_gt_clk_ppll0_gt_sft_clr_END (30)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_gt_clk_ppll0_gt_stat_START (31)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_0_cofsme_gt_clk_ppll0_gt_stat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsme_cpu_div_stat : 16;
        unsigned int cofsme_vdm_div_stat : 4;
        unsigned int cofsme_vdm_div_sft_idx : 4;
        unsigned int cofsme_vdm_div_sft_cfg : 1;
        unsigned int cofsme_cur_clk_src_stat : 3;
        unsigned int cofsme_cur_apll_freq_stat : 3;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_1_cofsme_cpu_div_stat_START (0)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_1_cofsme_cpu_div_stat_END (15)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_1_cofsme_vdm_div_stat_START (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_1_cofsme_vdm_div_stat_END (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_1_cofsme_vdm_div_sft_idx_START (20)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_1_cofsme_vdm_div_sft_idx_END (23)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_1_cofsme_vdm_div_sft_cfg_START (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_1_cofsme_vdm_div_sft_cfg_END (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_1_cofsme_cur_clk_src_stat_START (25)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_1_cofsme_cur_clk_src_stat_END (27)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_1_cofsme_cur_apll_freq_stat_START (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_E_1_cofsme_cur_apll_freq_stat_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmf_cpu_div_sft_idx : 16;
        unsigned int cofsmf_cpu_div_sft_cfg : 1;
        unsigned int cofsmf_sw2ppll0_req : 1;
        unsigned int cofsmf_sw2apll_req : 1;
        unsigned int cofsmf_pll_sw_stat : 1;
        unsigned int cofsmf_ppll0_div_stat : 4;
        unsigned int cofsmf_ppll0_div_sft_idx : 4;
        unsigned int cofsmf_ppll0_div_sft_cfg : 1;
        unsigned int cofsmf_gt_clk_ppll0_gt_sft_set : 1;
        unsigned int cofsmf_gt_clk_ppll0_gt_sft_clr : 1;
        unsigned int cofsmf_gt_clk_ppll0_gt_stat : 1;
    } reg;
} SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_cpu_div_sft_idx_START (0)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_cpu_div_sft_idx_END (15)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_cpu_div_sft_cfg_START (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_cpu_div_sft_cfg_END (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_sw2ppll0_req_START (17)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_sw2ppll0_req_END (17)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_sw2apll_req_START (18)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_sw2apll_req_END (18)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_pll_sw_stat_START (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_pll_sw_stat_END (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_ppll0_div_stat_START (20)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_ppll0_div_stat_END (23)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_ppll0_div_sft_idx_START (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_ppll0_div_sft_idx_END (27)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_ppll0_div_sft_cfg_START (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_ppll0_div_sft_cfg_END (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_gt_clk_ppll0_gt_sft_set_START (29)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_gt_clk_ppll0_gt_sft_set_END (29)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_gt_clk_ppll0_gt_sft_clr_START (30)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_gt_clk_ppll0_gt_sft_clr_END (30)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_gt_clk_ppll0_gt_stat_START (31)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_0_cofsmf_gt_clk_ppll0_gt_stat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmf_cpu_div_stat : 16;
        unsigned int cofsmf_vdm_div_stat : 4;
        unsigned int cofsmf_vdm_div_sft_idx : 4;
        unsigned int cofsmf_vdm_div_sft_cfg : 1;
        unsigned int cofsmf_cur_clk_src_stat : 3;
        unsigned int cofsmf_cur_apll_freq_stat : 3;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_1_cofsmf_cpu_div_stat_START (0)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_1_cofsmf_cpu_div_stat_END (15)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_1_cofsmf_vdm_div_stat_START (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_1_cofsmf_vdm_div_stat_END (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_1_cofsmf_vdm_div_sft_idx_START (20)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_1_cofsmf_vdm_div_sft_idx_END (23)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_1_cofsmf_vdm_div_sft_cfg_START (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_1_cofsmf_vdm_div_sft_cfg_END (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_1_cofsmf_cur_clk_src_stat_START (25)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_1_cofsmf_cur_clk_src_stat_END (27)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_1_cofsmf_cur_apll_freq_stat_START (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_F_1_cofsmf_cur_apll_freq_stat_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmg_cpu_div_sft_idx : 16;
        unsigned int cofsmg_cpu_div_sft_cfg : 1;
        unsigned int cofsmg_sw2ppll0_req : 1;
        unsigned int cofsmg_sw2apll_req : 1;
        unsigned int cofsmg_pll_sw_stat : 1;
        unsigned int cofsmg_ppll0_div_stat : 4;
        unsigned int cofsmg_ppll0_div_sft_idx : 4;
        unsigned int cofsmg_ppll0_div_sft_cfg : 1;
        unsigned int cofsmg_gt_clk_ppll0_gt_sft_set : 1;
        unsigned int cofsmg_gt_clk_ppll0_gt_sft_clr : 1;
        unsigned int cofsmg_gt_clk_ppll0_gt_stat : 1;
    } reg;
} SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_cpu_div_sft_idx_START (0)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_cpu_div_sft_idx_END (15)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_cpu_div_sft_cfg_START (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_cpu_div_sft_cfg_END (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_sw2ppll0_req_START (17)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_sw2ppll0_req_END (17)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_sw2apll_req_START (18)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_sw2apll_req_END (18)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_pll_sw_stat_START (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_pll_sw_stat_END (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_ppll0_div_stat_START (20)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_ppll0_div_stat_END (23)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_ppll0_div_sft_idx_START (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_ppll0_div_sft_idx_END (27)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_ppll0_div_sft_cfg_START (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_ppll0_div_sft_cfg_END (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_gt_clk_ppll0_gt_sft_set_START (29)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_gt_clk_ppll0_gt_sft_set_END (29)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_gt_clk_ppll0_gt_sft_clr_START (30)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_gt_clk_ppll0_gt_sft_clr_END (30)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_gt_clk_ppll0_gt_stat_START (31)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_0_cofsmg_gt_clk_ppll0_gt_stat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmg_cpu_div_stat : 16;
        unsigned int cofsmg_vdm_div_stat : 4;
        unsigned int cofsmg_vdm_div_sft_idx : 4;
        unsigned int cofsmg_vdm_div_sft_cfg : 1;
        unsigned int cofsmg_cur_clk_src_stat : 3;
        unsigned int cofsmg_cur_apll_freq_stat : 3;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_1_cofsmg_cpu_div_stat_START (0)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_1_cofsmg_cpu_div_stat_END (15)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_1_cofsmg_vdm_div_stat_START (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_1_cofsmg_vdm_div_stat_END (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_1_cofsmg_vdm_div_sft_idx_START (20)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_1_cofsmg_vdm_div_sft_idx_END (23)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_1_cofsmg_vdm_div_sft_cfg_START (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_1_cofsmg_vdm_div_sft_cfg_END (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_1_cofsmg_cur_clk_src_stat_START (25)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_1_cofsmg_cur_clk_src_stat_END (27)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_1_cofsmg_cur_apll_freq_stat_START (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_G_1_cofsmg_cur_apll_freq_stat_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmh_cpu_div_sft_idx : 16;
        unsigned int cofsmh_cpu_div_sft_cfg : 1;
        unsigned int cofsmh_sw2ppll0_req : 1;
        unsigned int cofsmh_sw2apll_req : 1;
        unsigned int cofsmh_pll_sw_stat : 1;
        unsigned int cofsmh_ppll0_div_stat : 4;
        unsigned int cofsmh_ppll0_div_sft_idx : 4;
        unsigned int cofsmh_ppll0_div_sft_cfg : 1;
        unsigned int cofsmh_gt_clk_ppll0_gt_sft_set : 1;
        unsigned int cofsmh_gt_clk_ppll0_gt_sft_clr : 1;
        unsigned int cofsmh_gt_clk_ppll0_gt_stat : 1;
    } reg;
} SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_cpu_div_sft_idx_START (0)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_cpu_div_sft_idx_END (15)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_cpu_div_sft_cfg_START (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_cpu_div_sft_cfg_END (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_sw2ppll0_req_START (17)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_sw2ppll0_req_END (17)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_sw2apll_req_START (18)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_sw2apll_req_END (18)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_pll_sw_stat_START (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_pll_sw_stat_END (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_ppll0_div_stat_START (20)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_ppll0_div_stat_END (23)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_ppll0_div_sft_idx_START (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_ppll0_div_sft_idx_END (27)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_ppll0_div_sft_cfg_START (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_ppll0_div_sft_cfg_END (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_gt_clk_ppll0_gt_sft_set_START (29)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_gt_clk_ppll0_gt_sft_set_END (29)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_gt_clk_ppll0_gt_sft_clr_START (30)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_gt_clk_ppll0_gt_sft_clr_END (30)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_gt_clk_ppll0_gt_stat_START (31)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_0_cofsmh_gt_clk_ppll0_gt_stat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmh_cpu_div_stat : 16;
        unsigned int cofsmh_vdm_div_stat : 4;
        unsigned int cofsmh_vdm_div_sft_idx : 4;
        unsigned int cofsmh_vdm_div_sft_cfg : 1;
        unsigned int cofsmh_cur_clk_src_stat : 3;
        unsigned int cofsmh_cur_apll_freq_stat : 3;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_1_cofsmh_cpu_div_stat_START (0)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_1_cofsmh_cpu_div_stat_END (15)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_1_cofsmh_vdm_div_stat_START (16)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_1_cofsmh_vdm_div_stat_END (19)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_1_cofsmh_vdm_div_sft_idx_START (20)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_1_cofsmh_vdm_div_sft_idx_END (23)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_1_cofsmh_vdm_div_sft_cfg_START (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_1_cofsmh_vdm_div_sft_cfg_END (24)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_1_cofsmh_cur_clk_src_stat_START (25)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_1_cofsmh_cur_clk_src_stat_END (27)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_1_cofsmh_cur_apll_freq_stat_START (28)
#define SOC_PMCTRL_COFSM_CLK_DIV_CTRL_H_1_cofsmh_cur_apll_freq_stat_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsma_initial_pmu_logic_vol_idx : 8;
        unsigned int cofsma_initial_pmu_logic_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int cofsma_cur_pmu_logic_vol_stat : 8;
        unsigned int cofsma_logic_cur_vau_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_0_cofsma_initial_pmu_logic_vol_idx_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_0_cofsma_initial_pmu_logic_vol_idx_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_0_cofsma_initial_pmu_logic_vol_req_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_0_cofsma_initial_pmu_logic_vol_req_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_0_cofsma_cur_pmu_logic_vol_stat_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_0_cofsma_cur_pmu_logic_vol_stat_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_0_cofsma_logic_cur_vau_stat_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_0_cofsma_logic_cur_vau_stat_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsma_soft_logic_vol_chg_idx_0 : 8;
        unsigned int cofsma_soft_logic_vol_vote_en_0 : 1;
        unsigned int cofsma_soft_logic_vol_chg_pls_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsma_soft_logic_vol_chg_idx_1 : 8;
        unsigned int cofsma_soft_logic_vol_vote_en_1 : 1;
        unsigned int cofsma_soft_logic_vol_chg_pls_1 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsma_logic_nxt_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_cofsma_soft_logic_vol_chg_idx_0_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_cofsma_soft_logic_vol_chg_idx_0_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_cofsma_soft_logic_vol_vote_en_0_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_cofsma_soft_logic_vol_vote_en_0_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_cofsma_soft_logic_vol_chg_pls_0_START (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_cofsma_soft_logic_vol_chg_pls_0_END (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_cofsma_soft_logic_vol_chg_idx_1_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_cofsma_soft_logic_vol_chg_idx_1_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_cofsma_soft_logic_vol_vote_en_1_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_cofsma_soft_logic_vol_vote_en_1_END (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_cofsma_soft_logic_vol_chg_pls_1_START (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_cofsma_soft_logic_vol_chg_pls_1_END (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_cofsma_logic_nxt_max_vol_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_1_cofsma_logic_nxt_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 2;
        unsigned int reserved_4 : 8;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 2;
        unsigned int cofsma_logic_cur_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_2_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_2_cofsma_logic_cur_max_vol_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_2_cofsma_logic_cur_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsma_cur_pmu_logic_vol_req_stat_0 : 8;
        unsigned int cofsma_cur_pmu_logic_vol_req_stat_1 : 8;
        unsigned int cofsma_cur_pmu_logic_vol_req_stat_2 : 8;
        unsigned int cofsma_cur_pmu_logic_vol_req_stat_3 : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_3_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_3_cofsma_cur_pmu_logic_vol_req_stat_0_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_3_cofsma_cur_pmu_logic_vol_req_stat_0_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_3_cofsma_cur_pmu_logic_vol_req_stat_1_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_3_cofsma_cur_pmu_logic_vol_req_stat_1_END (15)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_3_cofsma_cur_pmu_logic_vol_req_stat_2_START (16)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_3_cofsma_cur_pmu_logic_vol_req_stat_2_END (23)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_3_cofsma_cur_pmu_logic_vol_req_stat_3_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_A_3_cofsma_cur_pmu_logic_vol_req_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsma_initial_pmu_mem_vol_idx : 8;
        unsigned int cofsma_initial_pmu_mem_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int cofsma_cur_pmu_mem_vol_stat : 8;
        unsigned int cofsma_mem_cur_vau_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_0_cofsma_initial_pmu_mem_vol_idx_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_0_cofsma_initial_pmu_mem_vol_idx_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_0_cofsma_initial_pmu_mem_vol_req_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_0_cofsma_initial_pmu_mem_vol_req_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_0_cofsma_cur_pmu_mem_vol_stat_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_0_cofsma_cur_pmu_mem_vol_stat_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_0_cofsma_mem_cur_vau_stat_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_0_cofsma_mem_cur_vau_stat_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsma_soft_mem_vol_chg_idx_0 : 8;
        unsigned int cofsma_soft_mem_vol_vote_en_0 : 1;
        unsigned int cofsma_soft_mem_vol_chg_pls_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsma_soft_mem_vol_chg_idx_1 : 8;
        unsigned int cofsma_soft_mem_vol_vote_en_1 : 1;
        unsigned int cofsma_soft_mem_vol_chg_pls_1 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsma_mem_nxt_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_cofsma_soft_mem_vol_chg_idx_0_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_cofsma_soft_mem_vol_chg_idx_0_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_cofsma_soft_mem_vol_vote_en_0_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_cofsma_soft_mem_vol_vote_en_0_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_cofsma_soft_mem_vol_chg_pls_0_START (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_cofsma_soft_mem_vol_chg_pls_0_END (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_cofsma_soft_mem_vol_chg_idx_1_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_cofsma_soft_mem_vol_chg_idx_1_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_cofsma_soft_mem_vol_vote_en_1_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_cofsma_soft_mem_vol_vote_en_1_END (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_cofsma_soft_mem_vol_chg_pls_1_START (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_cofsma_soft_mem_vol_chg_pls_1_END (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_cofsma_mem_nxt_max_vol_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_1_cofsma_mem_nxt_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsma_soft_mem_vol_chg_idx_2 : 8;
        unsigned int cofsma_soft_mem_vol_vote_en_2 : 1;
        unsigned int cofsma_soft_mem_vol_chg_pls_2 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsma_soft_mem_vol_chg_idx_3 : 8;
        unsigned int cofsma_soft_mem_vol_vote_en_3 : 1;
        unsigned int cofsma_soft_mem_vol_chg_pls_3 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsma_mem_cur_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_cofsma_soft_mem_vol_chg_idx_2_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_cofsma_soft_mem_vol_chg_idx_2_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_cofsma_soft_mem_vol_vote_en_2_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_cofsma_soft_mem_vol_vote_en_2_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_cofsma_soft_mem_vol_chg_pls_2_START (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_cofsma_soft_mem_vol_chg_pls_2_END (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_cofsma_soft_mem_vol_chg_idx_3_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_cofsma_soft_mem_vol_chg_idx_3_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_cofsma_soft_mem_vol_vote_en_3_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_cofsma_soft_mem_vol_vote_en_3_END (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_cofsma_soft_mem_vol_chg_pls_3_START (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_cofsma_soft_mem_vol_chg_pls_3_END (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_cofsma_mem_cur_max_vol_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_2_cofsma_mem_cur_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsma_cur_pmu_mem_vol_req_stat_0 : 8;
        unsigned int cofsma_cur_pmu_mem_vol_req_stat_1 : 8;
        unsigned int cofsma_cur_pmu_mem_vol_req_stat_2 : 8;
        unsigned int cofsma_cur_pmu_mem_vol_req_stat_3 : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_3_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_3_cofsma_cur_pmu_mem_vol_req_stat_0_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_3_cofsma_cur_pmu_mem_vol_req_stat_0_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_3_cofsma_cur_pmu_mem_vol_req_stat_1_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_3_cofsma_cur_pmu_mem_vol_req_stat_1_END (15)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_3_cofsma_cur_pmu_mem_vol_req_stat_2_START (16)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_3_cofsma_cur_pmu_mem_vol_req_stat_2_END (23)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_3_cofsma_cur_pmu_mem_vol_req_stat_3_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_A_3_cofsma_cur_pmu_mem_vol_req_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmb_initial_pmu_logic_vol_idx : 8;
        unsigned int cofsmb_initial_pmu_logic_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int cofsmb_cur_pmu_logic_vol_stat : 8;
        unsigned int cofsmb_logic_cur_vau_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_0_cofsmb_initial_pmu_logic_vol_idx_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_0_cofsmb_initial_pmu_logic_vol_idx_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_0_cofsmb_initial_pmu_logic_vol_req_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_0_cofsmb_initial_pmu_logic_vol_req_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_0_cofsmb_cur_pmu_logic_vol_stat_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_0_cofsmb_cur_pmu_logic_vol_stat_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_0_cofsmb_logic_cur_vau_stat_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_0_cofsmb_logic_cur_vau_stat_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmb_soft_logic_vol_chg_idx_0 : 8;
        unsigned int cofsmb_soft_logic_vol_vote_en_0 : 1;
        unsigned int cofsmb_soft_logic_vol_chg_pls_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmb_soft_logic_vol_chg_idx_1 : 8;
        unsigned int cofsmb_soft_logic_vol_vote_en_1 : 1;
        unsigned int cofsmb_soft_logic_vol_chg_pls_1 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmb_logic_nxt_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_cofsmb_soft_logic_vol_chg_idx_0_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_cofsmb_soft_logic_vol_chg_idx_0_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_cofsmb_soft_logic_vol_vote_en_0_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_cofsmb_soft_logic_vol_vote_en_0_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_cofsmb_soft_logic_vol_chg_pls_0_START (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_cofsmb_soft_logic_vol_chg_pls_0_END (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_cofsmb_soft_logic_vol_chg_idx_1_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_cofsmb_soft_logic_vol_chg_idx_1_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_cofsmb_soft_logic_vol_vote_en_1_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_cofsmb_soft_logic_vol_vote_en_1_END (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_cofsmb_soft_logic_vol_chg_pls_1_START (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_cofsmb_soft_logic_vol_chg_pls_1_END (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_cofsmb_logic_nxt_max_vol_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_1_cofsmb_logic_nxt_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 2;
        unsigned int reserved_4 : 8;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 2;
        unsigned int cofsmb_logic_cur_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_2_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_2_cofsmb_logic_cur_max_vol_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_2_cofsmb_logic_cur_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmb_cur_pmu_logic_vol_req_stat_0 : 8;
        unsigned int cofsmb_cur_pmu_logic_vol_req_stat_1 : 8;
        unsigned int cofsmb_cur_pmu_logic_vol_req_stat_2 : 8;
        unsigned int cofsmb_cur_pmu_logic_vol_req_stat_3 : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_3_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_3_cofsmb_cur_pmu_logic_vol_req_stat_0_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_3_cofsmb_cur_pmu_logic_vol_req_stat_0_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_3_cofsmb_cur_pmu_logic_vol_req_stat_1_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_3_cofsmb_cur_pmu_logic_vol_req_stat_1_END (15)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_3_cofsmb_cur_pmu_logic_vol_req_stat_2_START (16)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_3_cofsmb_cur_pmu_logic_vol_req_stat_2_END (23)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_3_cofsmb_cur_pmu_logic_vol_req_stat_3_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_B_3_cofsmb_cur_pmu_logic_vol_req_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmb_initial_pmu_mem_vol_idx : 8;
        unsigned int cofsmb_initial_pmu_mem_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int cofsmb_cur_pmu_mem_vol_stat : 8;
        unsigned int cofsmb_mem_cur_vau_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_0_cofsmb_initial_pmu_mem_vol_idx_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_0_cofsmb_initial_pmu_mem_vol_idx_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_0_cofsmb_initial_pmu_mem_vol_req_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_0_cofsmb_initial_pmu_mem_vol_req_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_0_cofsmb_cur_pmu_mem_vol_stat_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_0_cofsmb_cur_pmu_mem_vol_stat_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_0_cofsmb_mem_cur_vau_stat_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_0_cofsmb_mem_cur_vau_stat_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmb_soft_mem_vol_chg_idx_0 : 8;
        unsigned int cofsmb_soft_mem_vol_vote_en_0 : 1;
        unsigned int cofsmb_soft_mem_vol_chg_pls_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmb_soft_mem_vol_chg_idx_1 : 8;
        unsigned int cofsmb_soft_mem_vol_vote_en_1 : 1;
        unsigned int cofsmb_soft_mem_vol_chg_pls_1 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmb_mem_nxt_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_cofsmb_soft_mem_vol_chg_idx_0_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_cofsmb_soft_mem_vol_chg_idx_0_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_cofsmb_soft_mem_vol_vote_en_0_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_cofsmb_soft_mem_vol_vote_en_0_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_cofsmb_soft_mem_vol_chg_pls_0_START (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_cofsmb_soft_mem_vol_chg_pls_0_END (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_cofsmb_soft_mem_vol_chg_idx_1_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_cofsmb_soft_mem_vol_chg_idx_1_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_cofsmb_soft_mem_vol_vote_en_1_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_cofsmb_soft_mem_vol_vote_en_1_END (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_cofsmb_soft_mem_vol_chg_pls_1_START (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_cofsmb_soft_mem_vol_chg_pls_1_END (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_cofsmb_mem_nxt_max_vol_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_1_cofsmb_mem_nxt_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 2;
        unsigned int reserved_4 : 8;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 2;
        unsigned int cofsmb_mem_cur_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_2_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_2_cofsmb_mem_cur_max_vol_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_2_cofsmb_mem_cur_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmb_cur_pmu_mem_vol_req_stat_0 : 8;
        unsigned int cofsmb_cur_pmu_mem_vol_req_stat_1 : 8;
        unsigned int cofsmb_cur_pmu_mem_vol_req_stat_2 : 8;
        unsigned int cofsmb_cur_pmu_mem_vol_req_stat_3 : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_3_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_3_cofsmb_cur_pmu_mem_vol_req_stat_0_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_3_cofsmb_cur_pmu_mem_vol_req_stat_0_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_3_cofsmb_cur_pmu_mem_vol_req_stat_1_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_3_cofsmb_cur_pmu_mem_vol_req_stat_1_END (15)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_3_cofsmb_cur_pmu_mem_vol_req_stat_2_START (16)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_3_cofsmb_cur_pmu_mem_vol_req_stat_2_END (23)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_3_cofsmb_cur_pmu_mem_vol_req_stat_3_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_B_3_cofsmb_cur_pmu_mem_vol_req_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmc_initial_pmu_logic_vol_idx : 8;
        unsigned int cofsmc_initial_pmu_logic_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int cofsmc_cur_pmu_logic_vol_stat : 8;
        unsigned int cofsmc_logic_cur_vau_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_0_cofsmc_initial_pmu_logic_vol_idx_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_0_cofsmc_initial_pmu_logic_vol_idx_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_0_cofsmc_initial_pmu_logic_vol_req_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_0_cofsmc_initial_pmu_logic_vol_req_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_0_cofsmc_cur_pmu_logic_vol_stat_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_0_cofsmc_cur_pmu_logic_vol_stat_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_0_cofsmc_logic_cur_vau_stat_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_0_cofsmc_logic_cur_vau_stat_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmc_soft_logic_vol_chg_idx_0 : 8;
        unsigned int cofsmc_soft_logic_vol_vote_en_0 : 1;
        unsigned int cofsmc_soft_logic_vol_chg_pls_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmc_soft_logic_vol_chg_idx_1 : 8;
        unsigned int cofsmc_soft_logic_vol_vote_en_1 : 1;
        unsigned int cofsmc_soft_logic_vol_chg_pls_1 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmc_logic_nxt_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_cofsmc_soft_logic_vol_chg_idx_0_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_cofsmc_soft_logic_vol_chg_idx_0_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_cofsmc_soft_logic_vol_vote_en_0_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_cofsmc_soft_logic_vol_vote_en_0_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_cofsmc_soft_logic_vol_chg_pls_0_START (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_cofsmc_soft_logic_vol_chg_pls_0_END (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_cofsmc_soft_logic_vol_chg_idx_1_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_cofsmc_soft_logic_vol_chg_idx_1_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_cofsmc_soft_logic_vol_vote_en_1_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_cofsmc_soft_logic_vol_vote_en_1_END (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_cofsmc_soft_logic_vol_chg_pls_1_START (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_cofsmc_soft_logic_vol_chg_pls_1_END (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_cofsmc_logic_nxt_max_vol_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_1_cofsmc_logic_nxt_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 2;
        unsigned int reserved_4 : 8;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 2;
        unsigned int cofsmc_logic_cur_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_2_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_2_cofsmc_logic_cur_max_vol_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_2_cofsmc_logic_cur_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmc_cur_pmu_logic_vol_req_stat_0 : 8;
        unsigned int cofsmc_cur_pmu_logic_vol_req_stat_1 : 8;
        unsigned int cofsmc_cur_pmu_logic_vol_req_stat_2 : 8;
        unsigned int cofsmc_cur_pmu_logic_vol_req_stat_3 : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_3_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_3_cofsmc_cur_pmu_logic_vol_req_stat_0_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_3_cofsmc_cur_pmu_logic_vol_req_stat_0_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_3_cofsmc_cur_pmu_logic_vol_req_stat_1_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_3_cofsmc_cur_pmu_logic_vol_req_stat_1_END (15)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_3_cofsmc_cur_pmu_logic_vol_req_stat_2_START (16)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_3_cofsmc_cur_pmu_logic_vol_req_stat_2_END (23)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_3_cofsmc_cur_pmu_logic_vol_req_stat_3_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_C_3_cofsmc_cur_pmu_logic_vol_req_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmc_initial_pmu_mem_vol_idx : 8;
        unsigned int cofsmc_initial_pmu_mem_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int cofsmc_cur_pmu_mem_vol_stat : 8;
        unsigned int cofsmc_mem_cur_vau_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_0_cofsmc_initial_pmu_mem_vol_idx_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_0_cofsmc_initial_pmu_mem_vol_idx_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_0_cofsmc_initial_pmu_mem_vol_req_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_0_cofsmc_initial_pmu_mem_vol_req_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_0_cofsmc_cur_pmu_mem_vol_stat_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_0_cofsmc_cur_pmu_mem_vol_stat_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_0_cofsmc_mem_cur_vau_stat_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_0_cofsmc_mem_cur_vau_stat_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmc_soft_mem_vol_chg_idx_0 : 8;
        unsigned int cofsmc_soft_mem_vol_vote_en_0 : 1;
        unsigned int cofsmc_soft_mem_vol_chg_pls_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmc_soft_mem_vol_chg_idx_1 : 8;
        unsigned int cofsmc_soft_mem_vol_vote_en_1 : 1;
        unsigned int cofsmc_soft_mem_vol_chg_pls_1 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmc_mem_nxt_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_cofsmc_soft_mem_vol_chg_idx_0_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_cofsmc_soft_mem_vol_chg_idx_0_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_cofsmc_soft_mem_vol_vote_en_0_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_cofsmc_soft_mem_vol_vote_en_0_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_cofsmc_soft_mem_vol_chg_pls_0_START (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_cofsmc_soft_mem_vol_chg_pls_0_END (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_cofsmc_soft_mem_vol_chg_idx_1_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_cofsmc_soft_mem_vol_chg_idx_1_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_cofsmc_soft_mem_vol_vote_en_1_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_cofsmc_soft_mem_vol_vote_en_1_END (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_cofsmc_soft_mem_vol_chg_pls_1_START (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_cofsmc_soft_mem_vol_chg_pls_1_END (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_cofsmc_mem_nxt_max_vol_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_1_cofsmc_mem_nxt_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmc_soft_mem_vol_chg_idx_2 : 8;
        unsigned int cofsmc_soft_mem_vol_vote_en_2 : 1;
        unsigned int cofsmc_soft_mem_vol_chg_pls_2 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmc_soft_mem_vol_chg_idx_3 : 8;
        unsigned int cofsmc_soft_mem_vol_vote_en_3 : 1;
        unsigned int cofsmc_soft_mem_vol_chg_pls_3 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmc_mem_cur_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_cofsmc_soft_mem_vol_chg_idx_2_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_cofsmc_soft_mem_vol_chg_idx_2_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_cofsmc_soft_mem_vol_vote_en_2_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_cofsmc_soft_mem_vol_vote_en_2_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_cofsmc_soft_mem_vol_chg_pls_2_START (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_cofsmc_soft_mem_vol_chg_pls_2_END (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_cofsmc_soft_mem_vol_chg_idx_3_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_cofsmc_soft_mem_vol_chg_idx_3_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_cofsmc_soft_mem_vol_vote_en_3_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_cofsmc_soft_mem_vol_vote_en_3_END (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_cofsmc_soft_mem_vol_chg_pls_3_START (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_cofsmc_soft_mem_vol_chg_pls_3_END (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_cofsmc_mem_cur_max_vol_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_2_cofsmc_mem_cur_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmc_cur_pmu_mem_vol_req_stat_0 : 8;
        unsigned int cofsmc_cur_pmu_mem_vol_req_stat_1 : 8;
        unsigned int cofsmc_cur_pmu_mem_vol_req_stat_2 : 8;
        unsigned int cofsmc_cur_pmu_mem_vol_req_stat_3 : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_3_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_3_cofsmc_cur_pmu_mem_vol_req_stat_0_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_3_cofsmc_cur_pmu_mem_vol_req_stat_0_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_3_cofsmc_cur_pmu_mem_vol_req_stat_1_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_3_cofsmc_cur_pmu_mem_vol_req_stat_1_END (15)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_3_cofsmc_cur_pmu_mem_vol_req_stat_2_START (16)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_3_cofsmc_cur_pmu_mem_vol_req_stat_2_END (23)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_3_cofsmc_cur_pmu_mem_vol_req_stat_3_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_C_3_cofsmc_cur_pmu_mem_vol_req_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmd_initial_pmu_logic_vol_idx : 8;
        unsigned int cofsmd_initial_pmu_logic_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int cofsmd_cur_pmu_logic_vol_stat : 8;
        unsigned int cofsmd_logic_cur_vau_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_0_cofsmd_initial_pmu_logic_vol_idx_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_0_cofsmd_initial_pmu_logic_vol_idx_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_0_cofsmd_initial_pmu_logic_vol_req_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_0_cofsmd_initial_pmu_logic_vol_req_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_0_cofsmd_cur_pmu_logic_vol_stat_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_0_cofsmd_cur_pmu_logic_vol_stat_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_0_cofsmd_logic_cur_vau_stat_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_0_cofsmd_logic_cur_vau_stat_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmd_soft_logic_vol_chg_idx_0 : 8;
        unsigned int cofsmd_soft_logic_vol_vote_en_0 : 1;
        unsigned int cofsmd_soft_logic_vol_chg_pls_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmd_soft_logic_vol_chg_idx_1 : 8;
        unsigned int cofsmd_soft_logic_vol_vote_en_1 : 1;
        unsigned int cofsmd_soft_logic_vol_chg_pls_1 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmd_logic_nxt_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_cofsmd_soft_logic_vol_chg_idx_0_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_cofsmd_soft_logic_vol_chg_idx_0_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_cofsmd_soft_logic_vol_vote_en_0_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_cofsmd_soft_logic_vol_vote_en_0_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_cofsmd_soft_logic_vol_chg_pls_0_START (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_cofsmd_soft_logic_vol_chg_pls_0_END (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_cofsmd_soft_logic_vol_chg_idx_1_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_cofsmd_soft_logic_vol_chg_idx_1_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_cofsmd_soft_logic_vol_vote_en_1_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_cofsmd_soft_logic_vol_vote_en_1_END (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_cofsmd_soft_logic_vol_chg_pls_1_START (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_cofsmd_soft_logic_vol_chg_pls_1_END (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_cofsmd_logic_nxt_max_vol_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_1_cofsmd_logic_nxt_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmd_soft_logic_vol_chg_idx_2 : 8;
        unsigned int cofsmd_soft_logic_vol_vote_en_2 : 1;
        unsigned int cofsmd_soft_logic_vol_chg_pls_2 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmd_soft_logic_vol_chg_idx_3 : 8;
        unsigned int cofsmd_soft_logic_vol_vote_en_3 : 1;
        unsigned int cofsmd_soft_logic_vol_chg_pls_3 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmd_logic_cur_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_cofsmd_soft_logic_vol_chg_idx_2_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_cofsmd_soft_logic_vol_chg_idx_2_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_cofsmd_soft_logic_vol_vote_en_2_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_cofsmd_soft_logic_vol_vote_en_2_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_cofsmd_soft_logic_vol_chg_pls_2_START (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_cofsmd_soft_logic_vol_chg_pls_2_END (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_cofsmd_soft_logic_vol_chg_idx_3_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_cofsmd_soft_logic_vol_chg_idx_3_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_cofsmd_soft_logic_vol_vote_en_3_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_cofsmd_soft_logic_vol_vote_en_3_END (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_cofsmd_soft_logic_vol_chg_pls_3_START (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_cofsmd_soft_logic_vol_chg_pls_3_END (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_cofsmd_logic_cur_max_vol_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_2_cofsmd_logic_cur_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmd_cur_pmu_logic_vol_req_stat_0 : 8;
        unsigned int cofsmd_cur_pmu_logic_vol_req_stat_1 : 8;
        unsigned int cofsmd_cur_pmu_logic_vol_req_stat_2 : 8;
        unsigned int cofsmd_cur_pmu_logic_vol_req_stat_3 : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_3_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_3_cofsmd_cur_pmu_logic_vol_req_stat_0_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_3_cofsmd_cur_pmu_logic_vol_req_stat_0_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_3_cofsmd_cur_pmu_logic_vol_req_stat_1_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_3_cofsmd_cur_pmu_logic_vol_req_stat_1_END (15)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_3_cofsmd_cur_pmu_logic_vol_req_stat_2_START (16)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_3_cofsmd_cur_pmu_logic_vol_req_stat_2_END (23)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_3_cofsmd_cur_pmu_logic_vol_req_stat_3_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_D_3_cofsmd_cur_pmu_logic_vol_req_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmd_initial_pmu_mem_vol_idx : 8;
        unsigned int cofsmd_initial_pmu_mem_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int cofsmd_cur_pmu_mem_vol_stat : 8;
        unsigned int cofsmd_mem_cur_vau_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_0_cofsmd_initial_pmu_mem_vol_idx_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_0_cofsmd_initial_pmu_mem_vol_idx_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_0_cofsmd_initial_pmu_mem_vol_req_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_0_cofsmd_initial_pmu_mem_vol_req_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_0_cofsmd_cur_pmu_mem_vol_stat_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_0_cofsmd_cur_pmu_mem_vol_stat_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_0_cofsmd_mem_cur_vau_stat_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_0_cofsmd_mem_cur_vau_stat_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmd_soft_mem_vol_chg_idx_0 : 8;
        unsigned int cofsmd_soft_mem_vol_vote_en_0 : 1;
        unsigned int cofsmd_soft_mem_vol_chg_pls_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmd_soft_mem_vol_chg_idx_1 : 8;
        unsigned int cofsmd_soft_mem_vol_vote_en_1 : 1;
        unsigned int cofsmd_soft_mem_vol_chg_pls_1 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmd_mem_nxt_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_cofsmd_soft_mem_vol_chg_idx_0_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_cofsmd_soft_mem_vol_chg_idx_0_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_cofsmd_soft_mem_vol_vote_en_0_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_cofsmd_soft_mem_vol_vote_en_0_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_cofsmd_soft_mem_vol_chg_pls_0_START (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_cofsmd_soft_mem_vol_chg_pls_0_END (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_cofsmd_soft_mem_vol_chg_idx_1_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_cofsmd_soft_mem_vol_chg_idx_1_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_cofsmd_soft_mem_vol_vote_en_1_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_cofsmd_soft_mem_vol_vote_en_1_END (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_cofsmd_soft_mem_vol_chg_pls_1_START (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_cofsmd_soft_mem_vol_chg_pls_1_END (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_cofsmd_mem_nxt_max_vol_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_1_cofsmd_mem_nxt_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmd_soft_mem_vol_chg_idx_2 : 8;
        unsigned int cofsmd_soft_mem_vol_vote_en_2 : 1;
        unsigned int cofsmd_soft_mem_vol_chg_pls_2 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmd_soft_mem_vol_chg_idx_3 : 8;
        unsigned int cofsmd_soft_mem_vol_vote_en_3 : 1;
        unsigned int cofsmd_soft_mem_vol_chg_pls_3 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmd_mem_cur_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_cofsmd_soft_mem_vol_chg_idx_2_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_cofsmd_soft_mem_vol_chg_idx_2_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_cofsmd_soft_mem_vol_vote_en_2_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_cofsmd_soft_mem_vol_vote_en_2_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_cofsmd_soft_mem_vol_chg_pls_2_START (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_cofsmd_soft_mem_vol_chg_pls_2_END (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_cofsmd_soft_mem_vol_chg_idx_3_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_cofsmd_soft_mem_vol_chg_idx_3_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_cofsmd_soft_mem_vol_vote_en_3_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_cofsmd_soft_mem_vol_vote_en_3_END (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_cofsmd_soft_mem_vol_chg_pls_3_START (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_cofsmd_soft_mem_vol_chg_pls_3_END (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_cofsmd_mem_cur_max_vol_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_2_cofsmd_mem_cur_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmd_cur_pmu_mem_vol_req_stat_0 : 8;
        unsigned int cofsmd_cur_pmu_mem_vol_req_stat_1 : 8;
        unsigned int cofsmd_cur_pmu_mem_vol_req_stat_2 : 8;
        unsigned int cofsmd_cur_pmu_mem_vol_req_stat_3 : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_3_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_3_cofsmd_cur_pmu_mem_vol_req_stat_0_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_3_cofsmd_cur_pmu_mem_vol_req_stat_0_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_3_cofsmd_cur_pmu_mem_vol_req_stat_1_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_3_cofsmd_cur_pmu_mem_vol_req_stat_1_END (15)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_3_cofsmd_cur_pmu_mem_vol_req_stat_2_START (16)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_3_cofsmd_cur_pmu_mem_vol_req_stat_2_END (23)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_3_cofsmd_cur_pmu_mem_vol_req_stat_3_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_D_3_cofsmd_cur_pmu_mem_vol_req_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsme_initial_pmu_logic_vol_idx : 8;
        unsigned int cofsme_initial_pmu_logic_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int cofsme_cur_pmu_logic_vol_stat : 8;
        unsigned int cofsme_logic_cur_vau_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_0_cofsme_initial_pmu_logic_vol_idx_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_0_cofsme_initial_pmu_logic_vol_idx_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_0_cofsme_initial_pmu_logic_vol_req_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_0_cofsme_initial_pmu_logic_vol_req_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_0_cofsme_cur_pmu_logic_vol_stat_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_0_cofsme_cur_pmu_logic_vol_stat_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_0_cofsme_logic_cur_vau_stat_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_0_cofsme_logic_cur_vau_stat_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsme_soft_logic_vol_chg_idx_0 : 8;
        unsigned int cofsme_soft_logic_vol_vote_en_0 : 1;
        unsigned int cofsme_soft_logic_vol_chg_pls_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsme_soft_logic_vol_chg_idx_1 : 8;
        unsigned int cofsme_soft_logic_vol_vote_en_1 : 1;
        unsigned int cofsme_soft_logic_vol_chg_pls_1 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsme_logic_nxt_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_cofsme_soft_logic_vol_chg_idx_0_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_cofsme_soft_logic_vol_chg_idx_0_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_cofsme_soft_logic_vol_vote_en_0_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_cofsme_soft_logic_vol_vote_en_0_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_cofsme_soft_logic_vol_chg_pls_0_START (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_cofsme_soft_logic_vol_chg_pls_0_END (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_cofsme_soft_logic_vol_chg_idx_1_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_cofsme_soft_logic_vol_chg_idx_1_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_cofsme_soft_logic_vol_vote_en_1_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_cofsme_soft_logic_vol_vote_en_1_END (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_cofsme_soft_logic_vol_chg_pls_1_START (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_cofsme_soft_logic_vol_chg_pls_1_END (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_cofsme_logic_nxt_max_vol_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_1_cofsme_logic_nxt_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 2;
        unsigned int reserved_4 : 8;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 2;
        unsigned int cofsme_logic_cur_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_2_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_2_cofsme_logic_cur_max_vol_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_2_cofsme_logic_cur_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsme_cur_pmu_logic_vol_req_stat_0 : 8;
        unsigned int cofsme_cur_pmu_logic_vol_req_stat_1 : 8;
        unsigned int cofsme_cur_pmu_logic_vol_req_stat_2 : 8;
        unsigned int cofsme_cur_pmu_logic_vol_req_stat_3 : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_3_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_3_cofsme_cur_pmu_logic_vol_req_stat_0_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_3_cofsme_cur_pmu_logic_vol_req_stat_0_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_3_cofsme_cur_pmu_logic_vol_req_stat_1_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_3_cofsme_cur_pmu_logic_vol_req_stat_1_END (15)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_3_cofsme_cur_pmu_logic_vol_req_stat_2_START (16)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_3_cofsme_cur_pmu_logic_vol_req_stat_2_END (23)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_3_cofsme_cur_pmu_logic_vol_req_stat_3_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_E_3_cofsme_cur_pmu_logic_vol_req_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsme_initial_pmu_mem_vol_idx : 8;
        unsigned int cofsme_initial_pmu_mem_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int cofsme_cur_pmu_mem_vol_stat : 8;
        unsigned int cofsme_mem_cur_vau_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_0_cofsme_initial_pmu_mem_vol_idx_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_0_cofsme_initial_pmu_mem_vol_idx_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_0_cofsme_initial_pmu_mem_vol_req_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_0_cofsme_initial_pmu_mem_vol_req_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_0_cofsme_cur_pmu_mem_vol_stat_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_0_cofsme_cur_pmu_mem_vol_stat_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_0_cofsme_mem_cur_vau_stat_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_0_cofsme_mem_cur_vau_stat_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsme_soft_mem_vol_chg_idx_0 : 8;
        unsigned int cofsme_soft_mem_vol_vote_en_0 : 1;
        unsigned int cofsme_soft_mem_vol_chg_pls_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsme_soft_mem_vol_chg_idx_1 : 8;
        unsigned int cofsme_soft_mem_vol_vote_en_1 : 1;
        unsigned int cofsme_soft_mem_vol_chg_pls_1 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsme_mem_nxt_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_cofsme_soft_mem_vol_chg_idx_0_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_cofsme_soft_mem_vol_chg_idx_0_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_cofsme_soft_mem_vol_vote_en_0_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_cofsme_soft_mem_vol_vote_en_0_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_cofsme_soft_mem_vol_chg_pls_0_START (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_cofsme_soft_mem_vol_chg_pls_0_END (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_cofsme_soft_mem_vol_chg_idx_1_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_cofsme_soft_mem_vol_chg_idx_1_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_cofsme_soft_mem_vol_vote_en_1_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_cofsme_soft_mem_vol_vote_en_1_END (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_cofsme_soft_mem_vol_chg_pls_1_START (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_cofsme_soft_mem_vol_chg_pls_1_END (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_cofsme_mem_nxt_max_vol_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_1_cofsme_mem_nxt_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 2;
        unsigned int reserved_4 : 8;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 2;
        unsigned int cofsme_mem_cur_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_2_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_2_cofsme_mem_cur_max_vol_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_2_cofsme_mem_cur_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsme_cur_pmu_mem_vol_req_stat_0 : 8;
        unsigned int cofsme_cur_pmu_mem_vol_req_stat_1 : 8;
        unsigned int cofsme_cur_pmu_mem_vol_req_stat_2 : 8;
        unsigned int cofsme_cur_pmu_mem_vol_req_stat_3 : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_3_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_3_cofsme_cur_pmu_mem_vol_req_stat_0_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_3_cofsme_cur_pmu_mem_vol_req_stat_0_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_3_cofsme_cur_pmu_mem_vol_req_stat_1_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_3_cofsme_cur_pmu_mem_vol_req_stat_1_END (15)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_3_cofsme_cur_pmu_mem_vol_req_stat_2_START (16)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_3_cofsme_cur_pmu_mem_vol_req_stat_2_END (23)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_3_cofsme_cur_pmu_mem_vol_req_stat_3_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_E_3_cofsme_cur_pmu_mem_vol_req_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmf_initial_pmu_logic_vol_idx : 8;
        unsigned int cofsmf_initial_pmu_logic_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int cofsmf_cur_pmu_logic_vol_stat : 8;
        unsigned int cofsmf_logic_cur_vau_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_0_cofsmf_initial_pmu_logic_vol_idx_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_0_cofsmf_initial_pmu_logic_vol_idx_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_0_cofsmf_initial_pmu_logic_vol_req_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_0_cofsmf_initial_pmu_logic_vol_req_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_0_cofsmf_cur_pmu_logic_vol_stat_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_0_cofsmf_cur_pmu_logic_vol_stat_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_0_cofsmf_logic_cur_vau_stat_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_0_cofsmf_logic_cur_vau_stat_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmf_soft_logic_vol_chg_idx_0 : 8;
        unsigned int cofsmf_soft_logic_vol_vote_en_0 : 1;
        unsigned int cofsmf_soft_logic_vol_chg_pls_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmf_soft_logic_vol_chg_idx_1 : 8;
        unsigned int cofsmf_soft_logic_vol_vote_en_1 : 1;
        unsigned int cofsmf_soft_logic_vol_chg_pls_1 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmf_logic_nxt_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_cofsmf_soft_logic_vol_chg_idx_0_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_cofsmf_soft_logic_vol_chg_idx_0_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_cofsmf_soft_logic_vol_vote_en_0_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_cofsmf_soft_logic_vol_vote_en_0_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_cofsmf_soft_logic_vol_chg_pls_0_START (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_cofsmf_soft_logic_vol_chg_pls_0_END (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_cofsmf_soft_logic_vol_chg_idx_1_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_cofsmf_soft_logic_vol_chg_idx_1_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_cofsmf_soft_logic_vol_vote_en_1_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_cofsmf_soft_logic_vol_vote_en_1_END (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_cofsmf_soft_logic_vol_chg_pls_1_START (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_cofsmf_soft_logic_vol_chg_pls_1_END (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_cofsmf_logic_nxt_max_vol_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_1_cofsmf_logic_nxt_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmf_soft_logic_vol_chg_idx_2 : 8;
        unsigned int cofsmf_soft_logic_vol_vote_en_2 : 1;
        unsigned int cofsmf_soft_logic_vol_chg_pls_2 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmf_soft_logic_vol_chg_idx_3 : 8;
        unsigned int cofsmf_soft_logic_vol_vote_en_3 : 1;
        unsigned int cofsmf_soft_logic_vol_chg_pls_3 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmf_logic_cur_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_cofsmf_soft_logic_vol_chg_idx_2_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_cofsmf_soft_logic_vol_chg_idx_2_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_cofsmf_soft_logic_vol_vote_en_2_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_cofsmf_soft_logic_vol_vote_en_2_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_cofsmf_soft_logic_vol_chg_pls_2_START (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_cofsmf_soft_logic_vol_chg_pls_2_END (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_cofsmf_soft_logic_vol_chg_idx_3_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_cofsmf_soft_logic_vol_chg_idx_3_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_cofsmf_soft_logic_vol_vote_en_3_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_cofsmf_soft_logic_vol_vote_en_3_END (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_cofsmf_soft_logic_vol_chg_pls_3_START (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_cofsmf_soft_logic_vol_chg_pls_3_END (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_cofsmf_logic_cur_max_vol_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_2_cofsmf_logic_cur_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmf_cur_pmu_logic_vol_req_stat_0 : 8;
        unsigned int cofsmf_cur_pmu_logic_vol_req_stat_1 : 8;
        unsigned int cofsmf_cur_pmu_logic_vol_req_stat_2 : 8;
        unsigned int cofsmf_cur_pmu_logic_vol_req_stat_3 : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_3_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_3_cofsmf_cur_pmu_logic_vol_req_stat_0_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_3_cofsmf_cur_pmu_logic_vol_req_stat_0_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_3_cofsmf_cur_pmu_logic_vol_req_stat_1_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_3_cofsmf_cur_pmu_logic_vol_req_stat_1_END (15)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_3_cofsmf_cur_pmu_logic_vol_req_stat_2_START (16)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_3_cofsmf_cur_pmu_logic_vol_req_stat_2_END (23)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_3_cofsmf_cur_pmu_logic_vol_req_stat_3_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_F_3_cofsmf_cur_pmu_logic_vol_req_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmf_initial_pmu_mem_vol_idx : 8;
        unsigned int cofsmf_initial_pmu_mem_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int cofsmf_cur_pmu_mem_vol_stat : 8;
        unsigned int cofsmf_mem_cur_vau_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_0_cofsmf_initial_pmu_mem_vol_idx_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_0_cofsmf_initial_pmu_mem_vol_idx_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_0_cofsmf_initial_pmu_mem_vol_req_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_0_cofsmf_initial_pmu_mem_vol_req_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_0_cofsmf_cur_pmu_mem_vol_stat_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_0_cofsmf_cur_pmu_mem_vol_stat_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_0_cofsmf_mem_cur_vau_stat_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_0_cofsmf_mem_cur_vau_stat_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmf_soft_mem_vol_chg_idx_0 : 8;
        unsigned int cofsmf_soft_mem_vol_vote_en_0 : 1;
        unsigned int cofsmf_soft_mem_vol_chg_pls_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmf_soft_mem_vol_chg_idx_1 : 8;
        unsigned int cofsmf_soft_mem_vol_vote_en_1 : 1;
        unsigned int cofsmf_soft_mem_vol_chg_pls_1 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmf_mem_nxt_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_cofsmf_soft_mem_vol_chg_idx_0_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_cofsmf_soft_mem_vol_chg_idx_0_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_cofsmf_soft_mem_vol_vote_en_0_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_cofsmf_soft_mem_vol_vote_en_0_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_cofsmf_soft_mem_vol_chg_pls_0_START (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_cofsmf_soft_mem_vol_chg_pls_0_END (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_cofsmf_soft_mem_vol_chg_idx_1_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_cofsmf_soft_mem_vol_chg_idx_1_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_cofsmf_soft_mem_vol_vote_en_1_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_cofsmf_soft_mem_vol_vote_en_1_END (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_cofsmf_soft_mem_vol_chg_pls_1_START (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_cofsmf_soft_mem_vol_chg_pls_1_END (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_cofsmf_mem_nxt_max_vol_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_1_cofsmf_mem_nxt_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmf_soft_mem_vol_chg_idx_2 : 8;
        unsigned int cofsmf_soft_mem_vol_vote_en_2 : 1;
        unsigned int cofsmf_soft_mem_vol_chg_pls_2 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmf_soft_mem_vol_chg_idx_3 : 8;
        unsigned int cofsmf_soft_mem_vol_vote_en_3 : 1;
        unsigned int cofsmf_soft_mem_vol_chg_pls_3 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmf_mem_cur_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_cofsmf_soft_mem_vol_chg_idx_2_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_cofsmf_soft_mem_vol_chg_idx_2_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_cofsmf_soft_mem_vol_vote_en_2_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_cofsmf_soft_mem_vol_vote_en_2_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_cofsmf_soft_mem_vol_chg_pls_2_START (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_cofsmf_soft_mem_vol_chg_pls_2_END (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_cofsmf_soft_mem_vol_chg_idx_3_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_cofsmf_soft_mem_vol_chg_idx_3_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_cofsmf_soft_mem_vol_vote_en_3_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_cofsmf_soft_mem_vol_vote_en_3_END (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_cofsmf_soft_mem_vol_chg_pls_3_START (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_cofsmf_soft_mem_vol_chg_pls_3_END (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_cofsmf_mem_cur_max_vol_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_2_cofsmf_mem_cur_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmf_cur_pmu_mem_vol_req_stat_0 : 8;
        unsigned int cofsmf_cur_pmu_mem_vol_req_stat_1 : 8;
        unsigned int cofsmf_cur_pmu_mem_vol_req_stat_2 : 8;
        unsigned int cofsmf_cur_pmu_mem_vol_req_stat_3 : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_3_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_3_cofsmf_cur_pmu_mem_vol_req_stat_0_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_3_cofsmf_cur_pmu_mem_vol_req_stat_0_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_3_cofsmf_cur_pmu_mem_vol_req_stat_1_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_3_cofsmf_cur_pmu_mem_vol_req_stat_1_END (15)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_3_cofsmf_cur_pmu_mem_vol_req_stat_2_START (16)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_3_cofsmf_cur_pmu_mem_vol_req_stat_2_END (23)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_3_cofsmf_cur_pmu_mem_vol_req_stat_3_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_F_3_cofsmf_cur_pmu_mem_vol_req_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmg_initial_pmu_logic_vol_idx : 8;
        unsigned int cofsmg_initial_pmu_logic_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int cofsmg_cur_pmu_logic_vol_stat : 8;
        unsigned int cofsmg_logic_cur_vau_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_0_cofsmg_initial_pmu_logic_vol_idx_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_0_cofsmg_initial_pmu_logic_vol_idx_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_0_cofsmg_initial_pmu_logic_vol_req_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_0_cofsmg_initial_pmu_logic_vol_req_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_0_cofsmg_cur_pmu_logic_vol_stat_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_0_cofsmg_cur_pmu_logic_vol_stat_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_0_cofsmg_logic_cur_vau_stat_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_0_cofsmg_logic_cur_vau_stat_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmg_soft_logic_vol_chg_idx_0 : 8;
        unsigned int cofsmg_soft_logic_vol_vote_en_0 : 1;
        unsigned int cofsmg_soft_logic_vol_chg_pls_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmg_soft_logic_vol_chg_idx_1 : 8;
        unsigned int cofsmg_soft_logic_vol_vote_en_1 : 1;
        unsigned int cofsmg_soft_logic_vol_chg_pls_1 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmg_logic_nxt_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_cofsmg_soft_logic_vol_chg_idx_0_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_cofsmg_soft_logic_vol_chg_idx_0_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_cofsmg_soft_logic_vol_vote_en_0_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_cofsmg_soft_logic_vol_vote_en_0_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_cofsmg_soft_logic_vol_chg_pls_0_START (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_cofsmg_soft_logic_vol_chg_pls_0_END (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_cofsmg_soft_logic_vol_chg_idx_1_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_cofsmg_soft_logic_vol_chg_idx_1_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_cofsmg_soft_logic_vol_vote_en_1_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_cofsmg_soft_logic_vol_vote_en_1_END (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_cofsmg_soft_logic_vol_chg_pls_1_START (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_cofsmg_soft_logic_vol_chg_pls_1_END (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_cofsmg_logic_nxt_max_vol_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_1_cofsmg_logic_nxt_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmg_soft_logic_vol_chg_idx_2 : 8;
        unsigned int cofsmg_soft_logic_vol_vote_en_2 : 1;
        unsigned int cofsmg_soft_logic_vol_chg_pls_2 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmg_soft_logic_vol_chg_idx_3 : 8;
        unsigned int cofsmg_soft_logic_vol_vote_en_3 : 1;
        unsigned int cofsmg_soft_logic_vol_chg_pls_3 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmg_logic_cur_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_cofsmg_soft_logic_vol_chg_idx_2_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_cofsmg_soft_logic_vol_chg_idx_2_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_cofsmg_soft_logic_vol_vote_en_2_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_cofsmg_soft_logic_vol_vote_en_2_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_cofsmg_soft_logic_vol_chg_pls_2_START (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_cofsmg_soft_logic_vol_chg_pls_2_END (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_cofsmg_soft_logic_vol_chg_idx_3_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_cofsmg_soft_logic_vol_chg_idx_3_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_cofsmg_soft_logic_vol_vote_en_3_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_cofsmg_soft_logic_vol_vote_en_3_END (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_cofsmg_soft_logic_vol_chg_pls_3_START (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_cofsmg_soft_logic_vol_chg_pls_3_END (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_cofsmg_logic_cur_max_vol_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_2_cofsmg_logic_cur_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmg_cur_pmu_logic_vol_req_stat_0 : 8;
        unsigned int cofsmg_cur_pmu_logic_vol_req_stat_1 : 8;
        unsigned int cofsmg_cur_pmu_logic_vol_req_stat_2 : 8;
        unsigned int cofsmg_cur_pmu_logic_vol_req_stat_3 : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_3_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_3_cofsmg_cur_pmu_logic_vol_req_stat_0_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_3_cofsmg_cur_pmu_logic_vol_req_stat_0_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_3_cofsmg_cur_pmu_logic_vol_req_stat_1_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_3_cofsmg_cur_pmu_logic_vol_req_stat_1_END (15)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_3_cofsmg_cur_pmu_logic_vol_req_stat_2_START (16)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_3_cofsmg_cur_pmu_logic_vol_req_stat_2_END (23)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_3_cofsmg_cur_pmu_logic_vol_req_stat_3_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_G_3_cofsmg_cur_pmu_logic_vol_req_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmg_initial_pmu_mem_vol_idx : 8;
        unsigned int cofsmg_initial_pmu_mem_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int cofsmg_cur_pmu_mem_vol_stat : 8;
        unsigned int cofsmg_mem_cur_vau_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_0_cofsmg_initial_pmu_mem_vol_idx_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_0_cofsmg_initial_pmu_mem_vol_idx_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_0_cofsmg_initial_pmu_mem_vol_req_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_0_cofsmg_initial_pmu_mem_vol_req_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_0_cofsmg_cur_pmu_mem_vol_stat_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_0_cofsmg_cur_pmu_mem_vol_stat_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_0_cofsmg_mem_cur_vau_stat_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_0_cofsmg_mem_cur_vau_stat_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmg_soft_mem_vol_chg_idx_0 : 8;
        unsigned int cofsmg_soft_mem_vol_vote_en_0 : 1;
        unsigned int cofsmg_soft_mem_vol_chg_pls_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmg_soft_mem_vol_chg_idx_1 : 8;
        unsigned int cofsmg_soft_mem_vol_vote_en_1 : 1;
        unsigned int cofsmg_soft_mem_vol_chg_pls_1 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmg_mem_nxt_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_cofsmg_soft_mem_vol_chg_idx_0_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_cofsmg_soft_mem_vol_chg_idx_0_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_cofsmg_soft_mem_vol_vote_en_0_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_cofsmg_soft_mem_vol_vote_en_0_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_cofsmg_soft_mem_vol_chg_pls_0_START (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_cofsmg_soft_mem_vol_chg_pls_0_END (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_cofsmg_soft_mem_vol_chg_idx_1_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_cofsmg_soft_mem_vol_chg_idx_1_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_cofsmg_soft_mem_vol_vote_en_1_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_cofsmg_soft_mem_vol_vote_en_1_END (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_cofsmg_soft_mem_vol_chg_pls_1_START (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_cofsmg_soft_mem_vol_chg_pls_1_END (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_cofsmg_mem_nxt_max_vol_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_1_cofsmg_mem_nxt_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmg_soft_mem_vol_chg_idx_2 : 8;
        unsigned int cofsmg_soft_mem_vol_vote_en_2 : 1;
        unsigned int cofsmg_soft_mem_vol_chg_pls_2 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmg_soft_mem_vol_chg_idx_3 : 8;
        unsigned int cofsmg_soft_mem_vol_vote_en_3 : 1;
        unsigned int cofsmg_soft_mem_vol_chg_pls_3 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmg_mem_cur_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_cofsmg_soft_mem_vol_chg_idx_2_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_cofsmg_soft_mem_vol_chg_idx_2_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_cofsmg_soft_mem_vol_vote_en_2_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_cofsmg_soft_mem_vol_vote_en_2_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_cofsmg_soft_mem_vol_chg_pls_2_START (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_cofsmg_soft_mem_vol_chg_pls_2_END (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_cofsmg_soft_mem_vol_chg_idx_3_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_cofsmg_soft_mem_vol_chg_idx_3_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_cofsmg_soft_mem_vol_vote_en_3_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_cofsmg_soft_mem_vol_vote_en_3_END (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_cofsmg_soft_mem_vol_chg_pls_3_START (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_cofsmg_soft_mem_vol_chg_pls_3_END (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_cofsmg_mem_cur_max_vol_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_2_cofsmg_mem_cur_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmg_cur_pmu_mem_vol_req_stat_0 : 8;
        unsigned int cofsmg_cur_pmu_mem_vol_req_stat_1 : 8;
        unsigned int cofsmg_cur_pmu_mem_vol_req_stat_2 : 8;
        unsigned int cofsmg_cur_pmu_mem_vol_req_stat_3 : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_3_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_3_cofsmg_cur_pmu_mem_vol_req_stat_0_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_3_cofsmg_cur_pmu_mem_vol_req_stat_0_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_3_cofsmg_cur_pmu_mem_vol_req_stat_1_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_3_cofsmg_cur_pmu_mem_vol_req_stat_1_END (15)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_3_cofsmg_cur_pmu_mem_vol_req_stat_2_START (16)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_3_cofsmg_cur_pmu_mem_vol_req_stat_2_END (23)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_3_cofsmg_cur_pmu_mem_vol_req_stat_3_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_G_3_cofsmg_cur_pmu_mem_vol_req_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmh_initial_pmu_logic_vol_idx : 8;
        unsigned int cofsmh_initial_pmu_logic_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int cofsmh_cur_pmu_logic_vol_stat : 8;
        unsigned int cofsmh_logic_cur_vau_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_0_cofsmh_initial_pmu_logic_vol_idx_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_0_cofsmh_initial_pmu_logic_vol_idx_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_0_cofsmh_initial_pmu_logic_vol_req_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_0_cofsmh_initial_pmu_logic_vol_req_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_0_cofsmh_cur_pmu_logic_vol_stat_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_0_cofsmh_cur_pmu_logic_vol_stat_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_0_cofsmh_logic_cur_vau_stat_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_0_cofsmh_logic_cur_vau_stat_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmh_soft_logic_vol_chg_idx_0 : 8;
        unsigned int cofsmh_soft_logic_vol_vote_en_0 : 1;
        unsigned int cofsmh_soft_logic_vol_chg_pls_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmh_soft_logic_vol_chg_idx_1 : 8;
        unsigned int cofsmh_soft_logic_vol_vote_en_1 : 1;
        unsigned int cofsmh_soft_logic_vol_chg_pls_1 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmh_logic_nxt_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_cofsmh_soft_logic_vol_chg_idx_0_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_cofsmh_soft_logic_vol_chg_idx_0_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_cofsmh_soft_logic_vol_vote_en_0_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_cofsmh_soft_logic_vol_vote_en_0_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_cofsmh_soft_logic_vol_chg_pls_0_START (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_cofsmh_soft_logic_vol_chg_pls_0_END (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_cofsmh_soft_logic_vol_chg_idx_1_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_cofsmh_soft_logic_vol_chg_idx_1_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_cofsmh_soft_logic_vol_vote_en_1_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_cofsmh_soft_logic_vol_vote_en_1_END (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_cofsmh_soft_logic_vol_chg_pls_1_START (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_cofsmh_soft_logic_vol_chg_pls_1_END (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_cofsmh_logic_nxt_max_vol_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_1_cofsmh_logic_nxt_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmh_soft_logic_vol_chg_idx_2 : 8;
        unsigned int cofsmh_soft_logic_vol_vote_en_2 : 1;
        unsigned int cofsmh_soft_logic_vol_chg_pls_2 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmh_soft_logic_vol_chg_idx_3 : 8;
        unsigned int cofsmh_soft_logic_vol_vote_en_3 : 1;
        unsigned int cofsmh_soft_logic_vol_chg_pls_3 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmh_logic_cur_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_cofsmh_soft_logic_vol_chg_idx_2_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_cofsmh_soft_logic_vol_chg_idx_2_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_cofsmh_soft_logic_vol_vote_en_2_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_cofsmh_soft_logic_vol_vote_en_2_END (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_cofsmh_soft_logic_vol_chg_pls_2_START (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_cofsmh_soft_logic_vol_chg_pls_2_END (9)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_cofsmh_soft_logic_vol_chg_idx_3_START (12)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_cofsmh_soft_logic_vol_chg_idx_3_END (19)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_cofsmh_soft_logic_vol_vote_en_3_START (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_cofsmh_soft_logic_vol_vote_en_3_END (20)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_cofsmh_soft_logic_vol_chg_pls_3_START (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_cofsmh_soft_logic_vol_chg_pls_3_END (21)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_cofsmh_logic_cur_max_vol_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_2_cofsmh_logic_cur_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmh_cur_pmu_logic_vol_req_stat_0 : 8;
        unsigned int cofsmh_cur_pmu_logic_vol_req_stat_1 : 8;
        unsigned int cofsmh_cur_pmu_logic_vol_req_stat_2 : 8;
        unsigned int cofsmh_cur_pmu_logic_vol_req_stat_3 : 8;
    } reg;
} SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_3_UNION;
#endif
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_3_cofsmh_cur_pmu_logic_vol_req_stat_0_START (0)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_3_cofsmh_cur_pmu_logic_vol_req_stat_0_END (7)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_3_cofsmh_cur_pmu_logic_vol_req_stat_1_START (8)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_3_cofsmh_cur_pmu_logic_vol_req_stat_1_END (15)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_3_cofsmh_cur_pmu_logic_vol_req_stat_2_START (16)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_3_cofsmh_cur_pmu_logic_vol_req_stat_2_END (23)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_3_cofsmh_cur_pmu_logic_vol_req_stat_3_START (24)
#define SOC_PMCTRL_COFSM_LOGIC_VOL_CTRL_H_3_cofsmh_cur_pmu_logic_vol_req_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmh_initial_pmu_mem_vol_idx : 8;
        unsigned int cofsmh_initial_pmu_mem_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int cofsmh_cur_pmu_mem_vol_stat : 8;
        unsigned int cofsmh_mem_cur_vau_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_0_cofsmh_initial_pmu_mem_vol_idx_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_0_cofsmh_initial_pmu_mem_vol_idx_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_0_cofsmh_initial_pmu_mem_vol_req_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_0_cofsmh_initial_pmu_mem_vol_req_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_0_cofsmh_cur_pmu_mem_vol_stat_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_0_cofsmh_cur_pmu_mem_vol_stat_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_0_cofsmh_mem_cur_vau_stat_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_0_cofsmh_mem_cur_vau_stat_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmh_soft_mem_vol_chg_idx_0 : 8;
        unsigned int cofsmh_soft_mem_vol_vote_en_0 : 1;
        unsigned int cofsmh_soft_mem_vol_chg_pls_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmh_soft_mem_vol_chg_idx_1 : 8;
        unsigned int cofsmh_soft_mem_vol_vote_en_1 : 1;
        unsigned int cofsmh_soft_mem_vol_chg_pls_1 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmh_mem_nxt_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_cofsmh_soft_mem_vol_chg_idx_0_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_cofsmh_soft_mem_vol_chg_idx_0_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_cofsmh_soft_mem_vol_vote_en_0_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_cofsmh_soft_mem_vol_vote_en_0_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_cofsmh_soft_mem_vol_chg_pls_0_START (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_cofsmh_soft_mem_vol_chg_pls_0_END (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_cofsmh_soft_mem_vol_chg_idx_1_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_cofsmh_soft_mem_vol_chg_idx_1_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_cofsmh_soft_mem_vol_vote_en_1_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_cofsmh_soft_mem_vol_vote_en_1_END (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_cofsmh_soft_mem_vol_chg_pls_1_START (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_cofsmh_soft_mem_vol_chg_pls_1_END (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_cofsmh_mem_nxt_max_vol_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_1_cofsmh_mem_nxt_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmh_soft_mem_vol_chg_idx_2 : 8;
        unsigned int cofsmh_soft_mem_vol_vote_en_2 : 1;
        unsigned int cofsmh_soft_mem_vol_chg_pls_2 : 1;
        unsigned int reserved_0 : 2;
        unsigned int cofsmh_soft_mem_vol_chg_idx_3 : 8;
        unsigned int cofsmh_soft_mem_vol_vote_en_3 : 1;
        unsigned int cofsmh_soft_mem_vol_chg_pls_3 : 1;
        unsigned int reserved_1 : 2;
        unsigned int cofsmh_mem_cur_max_vol : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_cofsmh_soft_mem_vol_chg_idx_2_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_cofsmh_soft_mem_vol_chg_idx_2_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_cofsmh_soft_mem_vol_vote_en_2_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_cofsmh_soft_mem_vol_vote_en_2_END (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_cofsmh_soft_mem_vol_chg_pls_2_START (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_cofsmh_soft_mem_vol_chg_pls_2_END (9)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_cofsmh_soft_mem_vol_chg_idx_3_START (12)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_cofsmh_soft_mem_vol_chg_idx_3_END (19)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_cofsmh_soft_mem_vol_vote_en_3_START (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_cofsmh_soft_mem_vol_vote_en_3_END (20)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_cofsmh_soft_mem_vol_chg_pls_3_START (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_cofsmh_soft_mem_vol_chg_pls_3_END (21)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_cofsmh_mem_cur_max_vol_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_2_cofsmh_mem_cur_max_vol_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmh_cur_pmu_mem_vol_req_stat_0 : 8;
        unsigned int cofsmh_cur_pmu_mem_vol_req_stat_1 : 8;
        unsigned int cofsmh_cur_pmu_mem_vol_req_stat_2 : 8;
        unsigned int cofsmh_cur_pmu_mem_vol_req_stat_3 : 8;
    } reg;
} SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_3_UNION;
#endif
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_3_cofsmh_cur_pmu_mem_vol_req_stat_0_START (0)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_3_cofsmh_cur_pmu_mem_vol_req_stat_0_END (7)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_3_cofsmh_cur_pmu_mem_vol_req_stat_1_START (8)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_3_cofsmh_cur_pmu_mem_vol_req_stat_1_END (15)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_3_cofsmh_cur_pmu_mem_vol_req_stat_2_START (16)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_3_cofsmh_cur_pmu_mem_vol_req_stat_2_END (23)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_3_cofsmh_cur_pmu_mem_vol_req_stat_3_START (24)
#define SOC_PMCTRL_COFSM_MEM_VOL_CTRL_H_3_cofsmh_cur_pmu_mem_vol_req_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsma_pmu_logic_up_hold_time : 20;
        unsigned int cofsma_pmu_logic_up_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_A_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_A_0_cofsma_pmu_logic_up_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_A_0_cofsma_pmu_logic_up_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_A_0_cofsma_pmu_logic_up_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_A_0_cofsma_pmu_logic_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsma_pmu_logic_dn_hold_time : 20;
        unsigned int cofsma_pmu_logic_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_A_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_A_1_cofsma_pmu_logic_dn_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_A_1_cofsma_pmu_logic_dn_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_A_1_cofsma_pmu_logic_dn_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_A_1_cofsma_pmu_logic_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsma_pmu_mem_up_hold_time : 20;
        unsigned int cofsma_pmu_mem_up_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_MEM_TIME_A_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_A_0_cofsma_pmu_mem_up_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_A_0_cofsma_pmu_mem_up_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_A_0_cofsma_pmu_mem_up_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_A_0_cofsma_pmu_mem_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsma_pmu_mem_dn_hold_time : 20;
        unsigned int cofsma_pmu_mem_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_MEM_TIME_A_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_A_1_cofsma_pmu_mem_dn_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_A_1_cofsma_pmu_mem_dn_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_A_1_cofsma_pmu_mem_dn_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_A_1_cofsma_pmu_mem_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmb_pmu_logic_up_hold_time : 20;
        unsigned int cofsmb_pmu_logic_up_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_B_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_B_0_cofsmb_pmu_logic_up_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_B_0_cofsmb_pmu_logic_up_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_B_0_cofsmb_pmu_logic_up_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_B_0_cofsmb_pmu_logic_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmb_pmu_logic_dn_hold_time : 20;
        unsigned int cofsmb_pmu_logic_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_B_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_B_1_cofsmb_pmu_logic_dn_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_B_1_cofsmb_pmu_logic_dn_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_B_1_cofsmb_pmu_logic_dn_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_B_1_cofsmb_pmu_logic_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmb_pmu_mem_up_hold_time : 20;
        unsigned int cofsmb_pmu_mem_up_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_MEM_TIME_B_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_B_0_cofsmb_pmu_mem_up_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_B_0_cofsmb_pmu_mem_up_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_B_0_cofsmb_pmu_mem_up_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_B_0_cofsmb_pmu_mem_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmb_pmu_mem_dn_hold_time : 20;
        unsigned int cofsmb_pmu_mem_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_MEM_TIME_B_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_B_1_cofsmb_pmu_mem_dn_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_B_1_cofsmb_pmu_mem_dn_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_B_1_cofsmb_pmu_mem_dn_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_B_1_cofsmb_pmu_mem_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmc_pmu_logic_up_hold_time : 20;
        unsigned int cofsmc_pmu_logic_up_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_C_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_C_0_cofsmc_pmu_logic_up_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_C_0_cofsmc_pmu_logic_up_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_C_0_cofsmc_pmu_logic_up_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_C_0_cofsmc_pmu_logic_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmc_pmu_logic_dn_hold_time : 20;
        unsigned int cofsmc_pmu_logic_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_C_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_C_1_cofsmc_pmu_logic_dn_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_C_1_cofsmc_pmu_logic_dn_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_C_1_cofsmc_pmu_logic_dn_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_C_1_cofsmc_pmu_logic_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmc_pmu_mem_up_hold_time : 20;
        unsigned int cofsmc_pmu_mem_up_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_MEM_TIME_C_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_C_0_cofsmc_pmu_mem_up_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_C_0_cofsmc_pmu_mem_up_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_C_0_cofsmc_pmu_mem_up_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_C_0_cofsmc_pmu_mem_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmc_pmu_mem_dn_hold_time : 20;
        unsigned int cofsmc_pmu_mem_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_MEM_TIME_C_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_C_1_cofsmc_pmu_mem_dn_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_C_1_cofsmc_pmu_mem_dn_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_C_1_cofsmc_pmu_mem_dn_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_C_1_cofsmc_pmu_mem_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmd_pmu_logic_up_hold_time : 20;
        unsigned int cofsmd_pmu_logic_up_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_D_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_D_0_cofsmd_pmu_logic_up_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_D_0_cofsmd_pmu_logic_up_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_D_0_cofsmd_pmu_logic_up_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_D_0_cofsmd_pmu_logic_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmd_pmu_logic_dn_hold_time : 20;
        unsigned int cofsmd_pmu_logic_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_D_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_D_1_cofsmd_pmu_logic_dn_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_D_1_cofsmd_pmu_logic_dn_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_D_1_cofsmd_pmu_logic_dn_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_D_1_cofsmd_pmu_logic_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmd_pmu_mem_up_hold_time : 20;
        unsigned int cofsmd_pmu_mem_up_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_MEM_TIME_D_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_D_0_cofsmd_pmu_mem_up_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_D_0_cofsmd_pmu_mem_up_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_D_0_cofsmd_pmu_mem_up_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_D_0_cofsmd_pmu_mem_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmd_pmu_mem_dn_hold_time : 20;
        unsigned int cofsmd_pmu_mem_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_MEM_TIME_D_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_D_1_cofsmd_pmu_mem_dn_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_D_1_cofsmd_pmu_mem_dn_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_D_1_cofsmd_pmu_mem_dn_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_D_1_cofsmd_pmu_mem_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsme_pmu_logic_up_hold_time : 20;
        unsigned int cofsme_pmu_logic_up_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_E_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_E_0_cofsme_pmu_logic_up_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_E_0_cofsme_pmu_logic_up_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_E_0_cofsme_pmu_logic_up_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_E_0_cofsme_pmu_logic_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsme_pmu_logic_dn_hold_time : 20;
        unsigned int cofsme_pmu_logic_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_E_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_E_1_cofsme_pmu_logic_dn_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_E_1_cofsme_pmu_logic_dn_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_E_1_cofsme_pmu_logic_dn_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_E_1_cofsme_pmu_logic_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsme_pmu_mem_up_hold_time : 20;
        unsigned int cofsme_pmu_mem_up_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_MEM_TIME_E_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_E_0_cofsme_pmu_mem_up_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_E_0_cofsme_pmu_mem_up_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_E_0_cofsme_pmu_mem_up_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_E_0_cofsme_pmu_mem_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsme_pmu_mem_dn_hold_time : 20;
        unsigned int cofsme_pmu_mem_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_MEM_TIME_E_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_E_1_cofsme_pmu_mem_dn_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_E_1_cofsme_pmu_mem_dn_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_E_1_cofsme_pmu_mem_dn_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_E_1_cofsme_pmu_mem_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmf_pmu_logic_up_hold_time : 20;
        unsigned int cofsmf_pmu_logic_up_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_F_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_F_0_cofsmf_pmu_logic_up_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_F_0_cofsmf_pmu_logic_up_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_F_0_cofsmf_pmu_logic_up_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_F_0_cofsmf_pmu_logic_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmf_pmu_logic_dn_hold_time : 20;
        unsigned int cofsmf_pmu_logic_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_F_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_F_1_cofsmf_pmu_logic_dn_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_F_1_cofsmf_pmu_logic_dn_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_F_1_cofsmf_pmu_logic_dn_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_F_1_cofsmf_pmu_logic_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmf_pmu_mem_up_hold_time : 20;
        unsigned int cofsmf_pmu_mem_up_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_MEM_TIME_F_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_F_0_cofsmf_pmu_mem_up_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_F_0_cofsmf_pmu_mem_up_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_F_0_cofsmf_pmu_mem_up_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_F_0_cofsmf_pmu_mem_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmf_pmu_mem_dn_hold_time : 20;
        unsigned int cofsmf_pmu_mem_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_MEM_TIME_F_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_F_1_cofsmf_pmu_mem_dn_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_F_1_cofsmf_pmu_mem_dn_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_F_1_cofsmf_pmu_mem_dn_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_F_1_cofsmf_pmu_mem_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmg_pmu_logic_up_hold_time : 20;
        unsigned int cofsmg_pmu_logic_up_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_G_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_G_0_cofsmg_pmu_logic_up_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_G_0_cofsmg_pmu_logic_up_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_G_0_cofsmg_pmu_logic_up_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_G_0_cofsmg_pmu_logic_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmg_pmu_logic_dn_hold_time : 20;
        unsigned int cofsmg_pmu_logic_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_G_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_G_1_cofsmg_pmu_logic_dn_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_G_1_cofsmg_pmu_logic_dn_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_G_1_cofsmg_pmu_logic_dn_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_G_1_cofsmg_pmu_logic_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmg_pmu_mem_up_hold_time : 20;
        unsigned int cofsmg_pmu_mem_up_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_MEM_TIME_G_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_G_0_cofsmg_pmu_mem_up_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_G_0_cofsmg_pmu_mem_up_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_G_0_cofsmg_pmu_mem_up_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_G_0_cofsmg_pmu_mem_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmg_pmu_mem_dn_hold_time : 20;
        unsigned int cofsmg_pmu_mem_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_MEM_TIME_G_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_G_1_cofsmg_pmu_mem_dn_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_G_1_cofsmg_pmu_mem_dn_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_G_1_cofsmg_pmu_mem_dn_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_G_1_cofsmg_pmu_mem_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmh_pmu_logic_up_hold_time : 20;
        unsigned int cofsmh_pmu_logic_up_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_H_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_H_0_cofsmh_pmu_logic_up_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_H_0_cofsmh_pmu_logic_up_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_H_0_cofsmh_pmu_logic_up_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_H_0_cofsmh_pmu_logic_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmh_pmu_logic_dn_hold_time : 20;
        unsigned int cofsmh_pmu_logic_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_H_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_H_1_cofsmh_pmu_logic_dn_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_H_1_cofsmh_pmu_logic_dn_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_H_1_cofsmh_pmu_logic_dn_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_LOGIC_TIME_H_1_cofsmh_pmu_logic_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmh_pmu_mem_up_hold_time : 20;
        unsigned int cofsmh_pmu_mem_up_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_MEM_TIME_H_0_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_H_0_cofsmh_pmu_mem_up_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_H_0_cofsmh_pmu_mem_up_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_H_0_cofsmh_pmu_mem_up_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_H_0_cofsmh_pmu_mem_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cofsmh_pmu_mem_dn_hold_time : 20;
        unsigned int cofsmh_pmu_mem_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_COFSM_PMU_MEM_TIME_H_1_UNION;
#endif
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_H_1_cofsmh_pmu_mem_dn_hold_time_START (0)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_H_1_cofsmh_pmu_mem_dn_hold_time_END (19)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_H_1_cofsmh_pmu_mem_dn_step_time_START (20)
#define SOC_PMCTRL_COFSM_PMU_MEM_TIME_H_1_cofsmh_pmu_mem_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_idx_0 : 32;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_IDX_0_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_0_peri_vote_vol_idx_0_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_0_peri_vote_vol_idx_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_idx_1 : 32;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_IDX_1_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_1_peri_vote_vol_idx_1_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_1_peri_vote_vol_idx_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_idx_2 : 32;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_IDX_2_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_2_peri_vote_vol_idx_2_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_2_peri_vote_vol_idx_2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_idx_3 : 32;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_IDX_3_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_3_peri_vote_vol_idx_3_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_3_peri_vote_vol_idx_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_idx_4 : 32;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_IDX_4_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_4_peri_vote_vol_idx_4_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_4_peri_vote_vol_idx_4_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_idx_5 : 32;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_IDX_5_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_5_peri_vote_vol_idx_5_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_5_peri_vote_vol_idx_5_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_idx_6 : 32;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_IDX_6_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_6_peri_vote_vol_idx_6_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_6_peri_vote_vol_idx_6_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_idx_7 : 32;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_IDX_7_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_7_peri_vote_vol_idx_7_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_7_peri_vote_vol_idx_7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_idx_8 : 32;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_IDX_8_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_8_peri_vote_vol_idx_8_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_8_peri_vote_vol_idx_8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_idx_9 : 32;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_IDX_9_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_9_peri_vote_vol_idx_9_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_9_peri_vote_vol_idx_9_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_idx_10 : 32;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_IDX_10_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_10_peri_vote_vol_idx_10_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_10_peri_vote_vol_idx_10_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_idx_11 : 32;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_IDX_11_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_11_peri_vote_vol_idx_11_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_11_peri_vote_vol_idx_11_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_idx_12 : 32;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_IDX_12_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_12_peri_vote_vol_idx_12_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_12_peri_vote_vol_idx_12_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_idx_13 : 32;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_IDX_13_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_13_peri_vote_vol_idx_13_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_13_peri_vote_vol_idx_13_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_idx_14 : 32;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_IDX_14_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_14_peri_vote_vol_idx_14_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_14_peri_vote_vol_idx_14_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_idx_15 : 32;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_IDX_15_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_15_peri_vote_vol_idx_15_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_IDX_15_peri_vote_vol_idx_15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_rank_0 : 2;
        unsigned int peri_vote_vol_rank_1 : 2;
        unsigned int peri_vote_vol_rank_2 : 2;
        unsigned int peri_vote_vol_rank_3 : 2;
        unsigned int peri_vote_vol_rank_4 : 2;
        unsigned int peri_vote_vol_rank_5 : 2;
        unsigned int peri_vote_vol_rank_6 : 2;
        unsigned int peri_vote_vol_rank_7 : 2;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_peri_vote_vol_rank_0_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_peri_vote_vol_rank_0_END (1)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_peri_vote_vol_rank_1_START (2)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_peri_vote_vol_rank_1_END (3)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_peri_vote_vol_rank_2_START (4)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_peri_vote_vol_rank_2_END (5)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_peri_vote_vol_rank_3_START (6)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_peri_vote_vol_rank_3_END (7)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_peri_vote_vol_rank_4_START (8)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_peri_vote_vol_rank_4_END (9)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_peri_vote_vol_rank_5_START (10)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_peri_vote_vol_rank_5_END (11)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_peri_vote_vol_rank_6_START (12)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_peri_vote_vol_rank_6_END (13)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_peri_vote_vol_rank_7_START (14)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_peri_vote_vol_rank_7_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_rank_8 : 2;
        unsigned int peri_vote_vol_rank_9 : 2;
        unsigned int peri_vote_vol_rank_10 : 2;
        unsigned int peri_vote_vol_rank_11 : 2;
        unsigned int peri_vote_vol_rank_12 : 2;
        unsigned int peri_vote_vol_rank_13 : 2;
        unsigned int peri_vote_vol_rank_14 : 2;
        unsigned int peri_vote_vol_rank_15 : 2;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_peri_vote_vol_rank_8_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_peri_vote_vol_rank_8_END (1)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_peri_vote_vol_rank_9_START (2)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_peri_vote_vol_rank_9_END (3)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_peri_vote_vol_rank_10_START (4)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_peri_vote_vol_rank_10_END (5)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_peri_vote_vol_rank_11_START (6)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_peri_vote_vol_rank_11_END (7)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_peri_vote_vol_rank_12_START (8)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_peri_vote_vol_rank_12_END (9)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_peri_vote_vol_rank_13_START (10)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_peri_vote_vol_rank_13_END (11)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_peri_vote_vol_rank_14_START (12)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_peri_vote_vol_rank_14_END (13)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_peri_vote_vol_rank_15_START (14)
#define SOC_PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_peri_vote_vol_rank_15_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_vol_valid_0 : 1;
        unsigned int peri_vote_vol_valid_1 : 1;
        unsigned int peri_vote_vol_valid_2 : 1;
        unsigned int peri_vote_vol_valid_3 : 1;
        unsigned int peri_vote_vol_valid_4 : 1;
        unsigned int peri_vote_vol_valid_5 : 1;
        unsigned int peri_vote_vol_valid_6 : 1;
        unsigned int peri_vote_vol_valid_7 : 1;
        unsigned int peri_vote_vol_valid_8 : 1;
        unsigned int peri_vote_vol_valid_9 : 1;
        unsigned int peri_vote_vol_valid_10 : 1;
        unsigned int peri_vote_vol_valid_11 : 1;
        unsigned int peri_vote_vol_valid_12 : 1;
        unsigned int peri_vote_vol_valid_13 : 1;
        unsigned int peri_vote_vol_valid_14 : 1;
        unsigned int peri_vote_vol_valid_15 : 1;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_PERI_VOTE_VOL_VALID_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_0_START (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_0_END (0)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_1_START (1)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_1_END (1)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_2_START (2)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_2_END (2)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_3_START (3)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_3_END (3)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_4_START (4)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_4_END (4)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_5_START (5)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_5_END (5)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_6_START (6)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_6_END (6)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_7_START (7)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_7_END (7)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_8_START (8)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_8_END (8)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_9_START (9)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_9_END (9)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_10_START (10)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_10_END (10)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_11_START (11)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_11_END (11)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_12_START (12)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_12_END (12)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_13_START (13)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_13_END (13)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_14_START (14)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_14_END (14)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_15_START (15)
#define SOC_PMCTRL_PERI_VOTE_VOL_VALID_peri_vote_vol_valid_15_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_sft_chg_0 : 1;
        unsigned int peri_vote_sft_chg_1 : 1;
        unsigned int peri_vote_sft_chg_2 : 1;
        unsigned int peri_vote_sft_chg_3 : 1;
        unsigned int peri_vote_sft_chg_4 : 1;
        unsigned int peri_vote_sft_chg_5 : 1;
        unsigned int peri_vote_sft_chg_6 : 1;
        unsigned int peri_vote_sft_chg_7 : 1;
        unsigned int peri_vote_sft_chg_8 : 1;
        unsigned int peri_vote_sft_chg_9 : 1;
        unsigned int peri_vote_sft_chg_10 : 1;
        unsigned int peri_vote_sft_chg_11 : 1;
        unsigned int peri_vote_sft_chg_12 : 1;
        unsigned int peri_vote_sft_chg_13 : 1;
        unsigned int peri_vote_sft_chg_14 : 1;
        unsigned int peri_vote_sft_chg_15 : 1;
        unsigned int peri_vote_sft_hrd_chg_0 : 1;
        unsigned int peri_vote_sft_hrd_chg_1 : 1;
        unsigned int peri_vote_sft_hrd_chg_2 : 1;
        unsigned int peri_vote_sft_hrd_chg_3 : 1;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_PERI_VOTE_SFT_CHG_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_0_START (0)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_0_END (0)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_1_START (1)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_1_END (1)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_2_START (2)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_2_END (2)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_3_START (3)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_3_END (3)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_4_START (4)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_4_END (4)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_5_START (5)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_5_END (5)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_6_START (6)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_6_END (6)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_7_START (7)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_7_END (7)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_8_START (8)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_8_END (8)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_9_START (9)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_9_END (9)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_10_START (10)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_10_END (10)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_11_START (11)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_11_END (11)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_12_START (12)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_12_END (12)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_13_START (13)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_13_END (13)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_14_START (14)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_14_END (14)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_15_START (15)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_chg_15_END (15)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_hrd_chg_0_START (16)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_hrd_chg_0_END (16)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_hrd_chg_1_START (17)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_hrd_chg_1_END (17)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_hrd_chg_2_START (18)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_hrd_chg_2_END (18)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_hrd_chg_3_START (19)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_peri_vote_sft_hrd_chg_3_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_sft_chg_bypass_0 : 1;
        unsigned int peri_vote_sft_chg_bypass_1 : 1;
        unsigned int peri_vote_sft_chg_bypass_2 : 1;
        unsigned int peri_vote_sft_chg_bypass_3 : 1;
        unsigned int peri_vote_sft_chg_bypass_4 : 1;
        unsigned int peri_vote_sft_chg_bypass_5 : 1;
        unsigned int peri_vote_sft_chg_bypass_6 : 1;
        unsigned int peri_vote_sft_chg_bypass_7 : 1;
        unsigned int peri_vote_sft_chg_bypass_8 : 1;
        unsigned int peri_vote_sft_chg_bypass_9 : 1;
        unsigned int peri_vote_sft_chg_bypass_10 : 1;
        unsigned int peri_vote_sft_chg_bypass_11 : 1;
        unsigned int peri_vote_sft_chg_bypass_12 : 1;
        unsigned int peri_vote_sft_chg_bypass_13 : 1;
        unsigned int peri_vote_sft_chg_bypass_14 : 1;
        unsigned int peri_vote_sft_chg_bypass_15 : 1;
        unsigned int peri_vote_hrd_chg_bypass_0 : 1;
        unsigned int peri_vote_hrd_chg_bypass_1 : 1;
        unsigned int peri_vote_hrd_chg_bypass_2 : 1;
        unsigned int peri_vote_hrd_chg_bypass_3 : 1;
        unsigned int peri_vote_sft_hrd_chg_bypass_0 : 1;
        unsigned int peri_vote_sft_hrd_chg_bypass_1 : 1;
        unsigned int peri_vote_sft_hrd_chg_bypass_2 : 1;
        unsigned int peri_vote_sft_hrd_chg_bypass_3 : 1;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_0_START (0)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_0_END (0)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_1_START (1)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_1_END (1)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_2_START (2)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_2_END (2)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_3_START (3)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_3_END (3)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_4_START (4)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_4_END (4)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_5_START (5)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_5_END (5)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_6_START (6)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_6_END (6)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_7_START (7)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_7_END (7)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_8_START (8)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_8_END (8)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_9_START (9)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_9_END (9)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_10_START (10)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_10_END (10)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_11_START (11)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_11_END (11)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_12_START (12)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_12_END (12)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_13_START (13)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_13_END (13)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_14_START (14)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_14_END (14)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_15_START (15)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_chg_bypass_15_END (15)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_hrd_chg_bypass_0_START (16)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_hrd_chg_bypass_0_END (16)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_hrd_chg_bypass_1_START (17)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_hrd_chg_bypass_1_END (17)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_hrd_chg_bypass_2_START (18)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_hrd_chg_bypass_2_END (18)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_hrd_chg_bypass_3_START (19)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_hrd_chg_bypass_3_END (19)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_hrd_chg_bypass_0_START (20)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_hrd_chg_bypass_0_END (20)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_hrd_chg_bypass_1_START (21)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_hrd_chg_bypass_1_END (21)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_hrd_chg_bypass_2_START (22)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_hrd_chg_bypass_2_END (22)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_hrd_chg_bypass_3_START (23)
#define SOC_PMCTRL_PERI_VOTE_SFT_CHG_BYPASS_peri_vote_sft_hrd_chg_bypass_3_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_pmu_up_hold_time : 20;
        unsigned int peri_pmu_up_step_time : 12;
    } reg;
} SOC_PMCTRL_PERI_PMU_TRIM_UP_TIME_UNION;
#endif
#define SOC_PMCTRL_PERI_PMU_TRIM_UP_TIME_peri_pmu_up_hold_time_START (0)
#define SOC_PMCTRL_PERI_PMU_TRIM_UP_TIME_peri_pmu_up_hold_time_END (19)
#define SOC_PMCTRL_PERI_PMU_TRIM_UP_TIME_peri_pmu_up_step_time_START (20)
#define SOC_PMCTRL_PERI_PMU_TRIM_UP_TIME_peri_pmu_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_pmu_dn_hold_time : 20;
        unsigned int peri_pmu_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_PERI_PMU_TRIM_DN_TTME_UNION;
#endif
#define SOC_PMCTRL_PERI_PMU_TRIM_DN_TTME_peri_pmu_dn_hold_time_START (0)
#define SOC_PMCTRL_PERI_PMU_TRIM_DN_TTME_peri_pmu_dn_hold_time_END (19)
#define SOC_PMCTRL_PERI_PMU_TRIM_DN_TTME_peri_pmu_dn_step_time_START (20)
#define SOC_PMCTRL_PERI_PMU_TRIM_DN_TTME_peri_pmu_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vol_sft_clr_0 : 1;
        unsigned int peri_vol_sft_clr_1 : 1;
        unsigned int peri_vol_sft_clr_2 : 1;
        unsigned int peri_vol_sft_clr_3 : 1;
        unsigned int peri_vol_sft_clr_4 : 1;
        unsigned int peri_vol_sft_clr_5 : 1;
        unsigned int peri_vol_sft_clr_6 : 1;
        unsigned int peri_vol_sft_clr_7 : 1;
        unsigned int peri_vol_sft_clr_8 : 1;
        unsigned int peri_vol_sft_clr_9 : 1;
        unsigned int peri_vol_sft_clr_10 : 1;
        unsigned int peri_vol_sft_clr_11 : 1;
        unsigned int peri_vol_sft_clr_12 : 1;
        unsigned int peri_vol_sft_clr_13 : 1;
        unsigned int peri_vol_sft_clr_14 : 1;
        unsigned int peri_vol_sft_clr_15 : 1;
        unsigned int peri_vol_sft_clr_hrd_0 : 1;
        unsigned int peri_vol_sft_clr_hrd_1 : 1;
        unsigned int peri_vol_sft_clr_hrd_2 : 1;
        unsigned int peri_vol_sft_clr_hrd_3 : 1;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_PERI_VOL_SFT_CLR_UNION;
#endif
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_0_START (0)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_0_END (0)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_1_START (1)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_1_END (1)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_2_START (2)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_2_END (2)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_3_START (3)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_3_END (3)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_4_START (4)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_4_END (4)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_5_START (5)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_5_END (5)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_6_START (6)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_6_END (6)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_7_START (7)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_7_END (7)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_8_START (8)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_8_END (8)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_9_START (9)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_9_END (9)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_10_START (10)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_10_END (10)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_11_START (11)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_11_END (11)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_12_START (12)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_12_END (12)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_13_START (13)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_13_END (13)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_14_START (14)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_14_END (14)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_15_START (15)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_15_END (15)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_hrd_0_START (16)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_hrd_0_END (16)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_hrd_1_START (17)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_hrd_1_END (17)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_hrd_2_START (18)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_hrd_2_END (18)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_hrd_3_START (19)
#define SOC_PMCTRL_PERI_VOL_SFT_CLR_peri_vol_sft_clr_hrd_3_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vol_stable_0 : 1;
        unsigned int peri_vol_stable_1 : 1;
        unsigned int peri_vol_stable_2 : 1;
        unsigned int peri_vol_stable_3 : 1;
        unsigned int peri_vol_stable_4 : 1;
        unsigned int peri_vol_stable_5 : 1;
        unsigned int peri_vol_stable_6 : 1;
        unsigned int peri_vol_stable_7 : 1;
        unsigned int peri_vol_stable_8 : 1;
        unsigned int peri_vol_stable_9 : 1;
        unsigned int peri_vol_stable_10 : 1;
        unsigned int peri_vol_stable_11 : 1;
        unsigned int peri_vol_stable_12 : 1;
        unsigned int peri_vol_stable_13 : 1;
        unsigned int peri_vol_stable_14 : 1;
        unsigned int peri_vol_stable_15 : 1;
        unsigned int peri_vol_hrd_stable_0 : 1;
        unsigned int peri_vol_hrd_stable_1 : 1;
        unsigned int peri_vol_hrd_stable_2 : 1;
        unsigned int peri_vol_hrd_stable_3 : 1;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_PERI_VOL_STABLE_UNION;
#endif
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_0_START (0)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_0_END (0)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_1_START (1)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_1_END (1)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_2_START (2)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_2_END (2)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_3_START (3)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_3_END (3)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_4_START (4)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_4_END (4)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_5_START (5)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_5_END (5)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_6_START (6)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_6_END (6)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_7_START (7)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_7_END (7)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_8_START (8)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_8_END (8)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_9_START (9)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_9_END (9)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_10_START (10)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_10_END (10)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_11_START (11)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_11_END (11)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_12_START (12)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_12_END (12)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_13_START (13)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_13_END (13)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_14_START (14)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_14_END (14)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_15_START (15)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_stable_15_END (15)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_hrd_stable_0_START (16)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_hrd_stable_0_END (16)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_hrd_stable_1_START (17)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_hrd_stable_1_END (17)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_hrd_stable_2_START (18)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_hrd_stable_2_END (18)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_hrd_stable_3_START (19)
#define SOC_PMCTRL_PERI_VOL_STABLE_peri_vol_hrd_stable_3_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_init_vol_idx : 8;
        unsigned int peri_init_vol_req : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_PMCTRL_PERI_INIT_VOL_CONFIG_UNION;
#endif
#define SOC_PMCTRL_PERI_INIT_VOL_CONFIG_peri_init_vol_idx_START (0)
#define SOC_PMCTRL_PERI_INIT_VOL_CONFIG_peri_init_vol_idx_END (7)
#define SOC_PMCTRL_PERI_INIT_VOL_CONFIG_peri_init_vol_req_START (8)
#define SOC_PMCTRL_PERI_INIT_VOL_CONFIG_peri_init_vol_req_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_sft_fix_vol_idx : 8;
        unsigned int peri_sft_fix_vol : 1;
        unsigned int peri_sft_fix_vol_req : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_PMCTRL_PERI_FIX_VOL_CONFIG_UNION;
#endif
#define SOC_PMCTRL_PERI_FIX_VOL_CONFIG_peri_sft_fix_vol_idx_START (0)
#define SOC_PMCTRL_PERI_FIX_VOL_CONFIG_peri_sft_fix_vol_idx_END (7)
#define SOC_PMCTRL_PERI_FIX_VOL_CONFIG_peri_sft_fix_vol_START (8)
#define SOC_PMCTRL_PERI_FIX_VOL_CONFIG_peri_sft_fix_vol_END (8)
#define SOC_PMCTRL_PERI_FIX_VOL_CONFIG_peri_sft_fix_vol_req_START (9)
#define SOC_PMCTRL_PERI_FIX_VOL_CONFIG_peri_sft_fix_vol_req_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_intr_m3orcpu_msk_0 : 1;
        unsigned int peri_intr_m3orcpu_msk_1 : 1;
        unsigned int peri_intr_m3orcpu_msk_2 : 1;
        unsigned int peri_intr_m3orcpu_msk_3 : 1;
        unsigned int peri_intr_m3orcpu_msk_4 : 1;
        unsigned int peri_intr_m3orcpu_msk_5 : 1;
        unsigned int peri_intr_m3orcpu_msk_6 : 1;
        unsigned int peri_intr_m3orcpu_msk_7 : 1;
        unsigned int peri_intr_m3orcpu_msk_8 : 1;
        unsigned int peri_intr_m3orcpu_msk_9 : 1;
        unsigned int peri_intr_m3orcpu_msk_10 : 1;
        unsigned int peri_intr_m3orcpu_msk_11 : 1;
        unsigned int peri_intr_m3orcpu_msk_12 : 1;
        unsigned int peri_intr_m3orcpu_msk_13 : 1;
        unsigned int peri_intr_m3orcpu_msk_14 : 1;
        unsigned int peri_intr_m3orcpu_msk_15 : 1;
        unsigned int peri_intr_m3orcpu_msk_16 : 1;
        unsigned int peri_intr_m3orcpu_msk_17 : 1;
        unsigned int peri_intr_m3orcpu_msk_18 : 1;
        unsigned int peri_intr_m3orcpu_msk_19 : 1;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_UNION;
#endif
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_0_START (0)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_0_END (0)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_1_START (1)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_1_END (1)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_2_START (2)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_2_END (2)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_3_START (3)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_3_END (3)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_4_START (4)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_4_END (4)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_5_START (5)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_5_END (5)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_6_START (6)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_6_END (6)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_7_START (7)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_7_END (7)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_8_START (8)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_8_END (8)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_9_START (9)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_9_END (9)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_10_START (10)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_10_END (10)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_11_START (11)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_11_END (11)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_12_START (12)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_12_END (12)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_13_START (13)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_13_END (13)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_14_START (14)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_14_END (14)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_15_START (15)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_15_END (15)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_16_START (16)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_16_END (16)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_17_START (17)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_17_END (17)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_18_START (18)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_18_END (18)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_19_START (19)
#define SOC_PMCTRL_PERI_INTR_M3ORCPU_MSK_peri_intr_m3orcpu_msk_19_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_cur_max_vol : 8;
        unsigned int peri_nxt_max_vol : 8;
        unsigned int peri_cur_max_vol_hst_0 : 8;
        unsigned int peri_cur_max_vol_hst_1 : 8;
    } reg;
} SOC_PMCTRL_PERI_VOL_READ_UNION;
#endif
#define SOC_PMCTRL_PERI_VOL_READ_peri_cur_max_vol_START (0)
#define SOC_PMCTRL_PERI_VOL_READ_peri_cur_max_vol_END (7)
#define SOC_PMCTRL_PERI_VOL_READ_peri_nxt_max_vol_START (8)
#define SOC_PMCTRL_PERI_VOL_READ_peri_nxt_max_vol_END (15)
#define SOC_PMCTRL_PERI_VOL_READ_peri_cur_max_vol_hst_0_START (16)
#define SOC_PMCTRL_PERI_VOL_READ_peri_cur_max_vol_hst_0_END (23)
#define SOC_PMCTRL_PERI_VOL_READ_peri_cur_max_vol_hst_1_START (24)
#define SOC_PMCTRL_PERI_VOL_READ_peri_cur_max_vol_hst_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_pmu_vol_idx_spmi : 8;
        unsigned int peri_pmu_vol_chg_spmi : 1;
        unsigned int intr_peri_dvs_stat_pmc2lpm3 : 1;
        unsigned int intr_peri_dvs_stat_pmc2acpu : 1;
        unsigned int peri_vol_rank_pmc2dpm : 3;
        unsigned int peri_cur_vau_stat : 3;
        unsigned int reserved : 15;
    } reg;
} SOC_PMCTRL_PERI_DVS_OUTPUT_READ_UNION;
#endif
#define SOC_PMCTRL_PERI_DVS_OUTPUT_READ_peri_pmu_vol_idx_spmi_START (0)
#define SOC_PMCTRL_PERI_DVS_OUTPUT_READ_peri_pmu_vol_idx_spmi_END (7)
#define SOC_PMCTRL_PERI_DVS_OUTPUT_READ_peri_pmu_vol_chg_spmi_START (8)
#define SOC_PMCTRL_PERI_DVS_OUTPUT_READ_peri_pmu_vol_chg_spmi_END (8)
#define SOC_PMCTRL_PERI_DVS_OUTPUT_READ_intr_peri_dvs_stat_pmc2lpm3_START (9)
#define SOC_PMCTRL_PERI_DVS_OUTPUT_READ_intr_peri_dvs_stat_pmc2lpm3_END (9)
#define SOC_PMCTRL_PERI_DVS_OUTPUT_READ_intr_peri_dvs_stat_pmc2acpu_START (10)
#define SOC_PMCTRL_PERI_DVS_OUTPUT_READ_intr_peri_dvs_stat_pmc2acpu_END (10)
#define SOC_PMCTRL_PERI_DVS_OUTPUT_READ_peri_vol_rank_pmc2dpm_START (11)
#define SOC_PMCTRL_PERI_DVS_OUTPUT_READ_peri_vol_rank_pmc2dpm_END (13)
#define SOC_PMCTRL_PERI_DVS_OUTPUT_READ_peri_cur_vau_stat_START (14)
#define SOC_PMCTRL_PERI_DVS_OUTPUT_READ_peri_cur_vau_stat_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vol_rank_idx_0 : 8;
        unsigned int peri_vol_rank_idx_1 : 8;
        unsigned int peri_vol_rank_idx_2 : 8;
        unsigned int peri_vol_rank_idx_3 : 8;
    } reg;
} SOC_PMCTRL_PERI_VOL_RANK_IDX_0_UNION;
#endif
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_0_peri_vol_rank_idx_0_START (0)
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_0_peri_vol_rank_idx_0_END (7)
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_0_peri_vol_rank_idx_1_START (8)
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_0_peri_vol_rank_idx_1_END (15)
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_0_peri_vol_rank_idx_2_START (16)
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_0_peri_vol_rank_idx_2_END (23)
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_0_peri_vol_rank_idx_3_START (24)
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_0_peri_vol_rank_idx_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vol_rank_idx_4 : 8;
        unsigned int peri_vol_rank_idx_5 : 8;
        unsigned int peri_vol_rank_idx_6 : 8;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_PERI_VOL_RANK_IDX_1_UNION;
#endif
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_1_peri_vol_rank_idx_4_START (0)
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_1_peri_vol_rank_idx_4_END (7)
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_1_peri_vol_rank_idx_5_START (8)
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_1_peri_vol_rank_idx_5_END (15)
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_1_peri_vol_rank_idx_6_START (16)
#define SOC_PMCTRL_PERI_VOL_RANK_IDX_1_peri_vol_rank_idx_6_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vote_sft_hrd_vol_idx_0 : 8;
        unsigned int peri_vote_sft_hrd_vol_idx_1 : 8;
        unsigned int peri_vote_sft_hrd_vol_idx_2 : 8;
        unsigned int peri_vote_sft_hrd_vol_idx_3 : 8;
    } reg;
} SOC_PMCTRL_PERI_VOTE_SFT_HRD_VOL_IDX_UNION;
#endif
#define SOC_PMCTRL_PERI_VOTE_SFT_HRD_VOL_IDX_peri_vote_sft_hrd_vol_idx_0_START (0)
#define SOC_PMCTRL_PERI_VOTE_SFT_HRD_VOL_IDX_peri_vote_sft_hrd_vol_idx_0_END (7)
#define SOC_PMCTRL_PERI_VOTE_SFT_HRD_VOL_IDX_peri_vote_sft_hrd_vol_idx_1_START (8)
#define SOC_PMCTRL_PERI_VOTE_SFT_HRD_VOL_IDX_peri_vote_sft_hrd_vol_idx_1_END (15)
#define SOC_PMCTRL_PERI_VOTE_SFT_HRD_VOL_IDX_peri_vote_sft_hrd_vol_idx_2_START (16)
#define SOC_PMCTRL_PERI_VOTE_SFT_HRD_VOL_IDX_peri_vote_sft_hrd_vol_idx_2_END (23)
#define SOC_PMCTRL_PERI_VOTE_SFT_HRD_VOL_IDX_peri_vote_sft_hrd_vol_idx_3_START (24)
#define SOC_PMCTRL_PERI_VOTE_SFT_HRD_VOL_IDX_peri_vote_sft_hrd_vol_idx_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_stat_time_hold : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_PERI_STAT_TIME_HOLD_UNION;
#endif
#define SOC_PMCTRL_PERI_STAT_TIME_HOLD_peri_stat_time_hold_START (0)
#define SOC_PMCTRL_PERI_STAT_TIME_HOLD_peri_stat_time_hold_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_intr_bypass_0 : 1;
        unsigned int peri_intr_bypass_1 : 1;
        unsigned int peri_intr_bypass_2 : 1;
        unsigned int peri_intr_bypass_3 : 1;
        unsigned int peri_intr_bypass_4 : 1;
        unsigned int peri_intr_bypass_5 : 1;
        unsigned int peri_intr_bypass_6 : 1;
        unsigned int peri_intr_bypass_7 : 1;
        unsigned int peri_intr_bypass_8 : 1;
        unsigned int peri_intr_bypass_9 : 1;
        unsigned int peri_intr_bypass_10 : 1;
        unsigned int peri_intr_bypass_11 : 1;
        unsigned int peri_intr_bypass_12 : 1;
        unsigned int peri_intr_bypass_13 : 1;
        unsigned int peri_intr_bypass_14 : 1;
        unsigned int peri_intr_bypass_15 : 1;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_PERI_INTR_BYPASS_SFT_UNION;
#endif
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_0_START (0)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_0_END (0)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_1_START (1)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_1_END (1)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_2_START (2)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_2_END (2)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_3_START (3)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_3_END (3)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_4_START (4)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_4_END (4)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_5_START (5)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_5_END (5)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_6_START (6)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_6_END (6)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_7_START (7)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_7_END (7)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_8_START (8)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_8_END (8)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_9_START (9)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_9_END (9)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_10_START (10)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_10_END (10)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_11_START (11)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_11_END (11)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_12_START (12)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_12_END (12)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_13_START (13)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_13_END (13)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_14_START (14)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_14_END (14)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_15_START (15)
#define SOC_PMCTRL_PERI_INTR_BYPASS_SFT_peri_intr_bypass_15_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_intr_bypass_16 : 1;
        unsigned int peri_intr_bypass_17 : 1;
        unsigned int peri_intr_bypass_18 : 1;
        unsigned int peri_intr_bypass_19 : 1;
        unsigned int peri_vol_rank_sftorhrd : 1;
        unsigned int peri_vol_rank_pmc2dpm_sft : 3;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 16;
    } reg;
} SOC_PMCTRL_PERI_INTR_BYPASS_HRD_UNION;
#endif
#define SOC_PMCTRL_PERI_INTR_BYPASS_HRD_peri_intr_bypass_16_START (0)
#define SOC_PMCTRL_PERI_INTR_BYPASS_HRD_peri_intr_bypass_16_END (0)
#define SOC_PMCTRL_PERI_INTR_BYPASS_HRD_peri_intr_bypass_17_START (1)
#define SOC_PMCTRL_PERI_INTR_BYPASS_HRD_peri_intr_bypass_17_END (1)
#define SOC_PMCTRL_PERI_INTR_BYPASS_HRD_peri_intr_bypass_18_START (2)
#define SOC_PMCTRL_PERI_INTR_BYPASS_HRD_peri_intr_bypass_18_END (2)
#define SOC_PMCTRL_PERI_INTR_BYPASS_HRD_peri_intr_bypass_19_START (3)
#define SOC_PMCTRL_PERI_INTR_BYPASS_HRD_peri_intr_bypass_19_END (3)
#define SOC_PMCTRL_PERI_INTR_BYPASS_HRD_peri_vol_rank_sftorhrd_START (4)
#define SOC_PMCTRL_PERI_INTR_BYPASS_HRD_peri_vol_rank_sftorhrd_END (4)
#define SOC_PMCTRL_PERI_INTR_BYPASS_HRD_peri_vol_rank_pmc2dpm_sft_START (5)
#define SOC_PMCTRL_PERI_INTR_BYPASS_HRD_peri_vol_rank_pmc2dpm_sft_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_vol_chg_flag : 21;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_PMCTRL_PERI_VOL_CHG_FLAG_UNION;
#endif
#define SOC_PMCTRL_PERI_VOL_CHG_FLAG_peri_vol_chg_flag_START (0)
#define SOC_PMCTRL_PERI_VOL_CHG_FLAG_peri_vol_chg_flag_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_valid_hrd_vol_idx_0 : 8;
        unsigned int peri_valid_hrd_vol_idx_1 : 8;
        unsigned int peri_valid_hrd_vol_idx_2 : 8;
        unsigned int peri_valid_hrd_vol_idx_3 : 8;
    } reg;
} SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_UNION;
#endif
#define SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_peri_valid_hrd_vol_idx_0_START (0)
#define SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_peri_valid_hrd_vol_idx_0_END (7)
#define SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_peri_valid_hrd_vol_idx_1_START (8)
#define SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_peri_valid_hrd_vol_idx_1_END (15)
#define SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_peri_valid_hrd_vol_idx_2_START (16)
#define SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_peri_valid_hrd_vol_idx_2_END (23)
#define SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_peri_valid_hrd_vol_idx_3_START (24)
#define SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_peri_valid_hrd_vol_idx_3_END (31)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
