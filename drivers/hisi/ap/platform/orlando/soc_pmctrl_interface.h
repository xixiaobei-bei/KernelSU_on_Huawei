#ifndef __SOC_PMCTRL_INTERFACE_H__
#define __SOC_PMCTRL_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_PMCTRL_PMCINTEN_ADDR(base) ((base) + (0x000))
#define SOC_PMCTRL_PMCINTSTAT_ADDR(base) ((base) + (0x004))
#define SOC_PMCTRL_PMCINTCLR_ADDR(base) ((base) + (0x008))
#define SOC_PMCTRL_DVFS_AUTOGT_BP_ADDR(base) ((base) + (0x094))
#define SOC_PMCTRL_CLUSTER0_DVFS_STAT_ADDR(base) ((base) + (0x098))
#define SOC_PMCTRL_CLUSTER0_CPUCLKSEL_ADDR(base) ((base) + (0x09C))
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_ADDR(base) ((base) + (0x0A8))
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE0_ADDR(base) ((base) + (0x0B0))
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE0_1_ADDR(base) ((base) + (0x0B4))
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE1_ADDR(base) ((base) + (0x0B8))
#define SOC_PMCTRL_CLUSTER0_CPUVOLMOD_ADDR(base) ((base) + (0x0BC))
#define SOC_PMCTRL_CLUSTER0_CPUVOLPROFILE_ADDR(base) ((base) + (0x0C0))
#define SOC_PMCTRL_CLUSTER0_PMU_UP_TIME_ADDR(base) ((base) + (0x0D8))
#define SOC_PMCTRL_CLUSTER0_PMU_DN_TIME_ADDR(base) ((base) + (0x0E4))
#define SOC_PMCTRL_CLUSTER0_PMUSEL_ADDR(base) ((base) + (0x0E8))
#define SOC_PMCTRL_CLUSTER0_CPUVOLIDX_ADDR(base) ((base) + (0x0EC))
#define SOC_PMCTRL_CLUSTER0_GDVFS_EN_ADDR(base) ((base) + (0x0F0))
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE0_ADDR(base) ((base) + (0x0F4))
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE1_ADDR(base) ((base) + (0x0F8))
#define SOC_PMCTRL_CLUSTER0_MAX_VOL_DBG_STAT_ADDR(base) ((base) + (0x054))
#define SOC_PMCTRL_CLUSTER0_UNIFIED_VOL_DBG_STAT_ADDR(base) ((base) + (0x058))
#define SOC_PMCTRL_CLUSTER0_CUR_VOL_DBG_STAT_ADDR(base) ((base) + (0x05C))
#define SOC_PMCTRL_GPU_HPMCLKDIV_ADDR(base) ((base) + (0x120))
#define SOC_PMCTRL_GPU_HPMSEL_ADDR(base) ((base) + (0x124))
#define SOC_PMCTRL_GPU_HPMEN_ADDR(base) ((base) + (0x128))
#define SOC_PMCTRL_GPU_HPMOPC_ADDR(base) ((base) + (0x12C))
#define SOC_PMCTRL_PERIBAKDATA0_ADDR(base) ((base) + (0x130))
#define SOC_PMCTRL_PERIBAKDATA1_ADDR(base) ((base) + (0x134))
#define SOC_PMCTRL_PERIBAKDATA2_ADDR(base) ((base) + (0x138))
#define SOC_PMCTRL_PERIBAKDATA3_ADDR(base) ((base) + (0x13C))
#define SOC_PMCTRL_BIGFAST_PHASEIDX_ADDR(base) ((base) + (0x140))
#define SOC_PMCTRL_BIGSLOW_PHASEIDX_ADDR(base) ((base) + (0x144))
#define SOC_PMCTRL_LITTLE_PHASEIDX_ADDR(base) ((base) + (0x148))
#define SOC_PMCTRL_GPU_PHASEIDX_ADDR(base) ((base) + (0x14C))
#define SOC_PMCTRL_SPMIVOLCFG_ADDR(base) ((base) + (0x150))
#define SOC_PMCTRL_SPMIVOLCFG1_ADDR(base) ((base) + (0x154))
#define SOC_PMCTRL_G3D_PMU_VOLSTEPTIME_ADDR(base) ((base) + (0x160))
#define SOC_PMCTRL_G3D_PMU_UP_HOLDTIME_ADDR(base) ((base) + (0x164))
#define SOC_PMCTRL_G3D_PMU_DN_HOLDTIME_ADDR(base) ((base) + (0x168))
#define SOC_PMCTRL_G3D_MEM_PMU_VOLSTEPTIME_ADDR(base) ((base) + (0x16C))
#define SOC_PMCTRL_G3D_MEM_PMU_UP_HOLDTIME_ADDR(base) ((base) + (0x170))
#define SOC_PMCTRL_G3D_MEM_PMU_DN_HOLDTIME_ADDR(base) ((base) + (0x174))
#define SOC_PMCTRL_G3D_VOL_INDEX0_ADDR(base) ((base) + (0x178))
#define SOC_PMCTRL_G3D_VOL_INDEX1_ADDR(base) ((base) + (0x17C))
#define SOC_PMCTRL_G3D_VOL_INDEX2_ADDR(base) ((base) + (0x180))
#define SOC_PMCTRL_G3D_VOL_INDEX3_ADDR(base) ((base) + (0x184))
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX0_ADDR(base) ((base) + (0x188))
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX1_ADDR(base) ((base) + (0x18C))
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX2_ADDR(base) ((base) + (0x190))
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX3_ADDR(base) ((base) + (0x194))
#define SOC_PMCTRL_G3DPMUSEL_ADDR(base) ((base) + (0x198))
#define SOC_PMCTRL_G3D_VOL_BYPASS_ADDR(base) ((base) + (0x19C))
#define SOC_PMCTRL_G3D_MEM_PMUSEL_ADDR(base) ((base) + (0x1A0))
#define SOC_PMCTRL_G3D_MEM_VOL_BYPASS_ADDR(base) ((base) + (0x1A4))
#define SOC_PMCTRL_G3D_INITIALVOL_ADDR(base) ((base) + (0x1A8))
#define SOC_PMCTRL_G3D_MEM_INITIALVOL_ADDR(base) ((base) + (0x1AC))
#define SOC_PMCTRL_G3DCLKPROFILE_ADDR(base) ((base) + (0x1B0))
#define SOC_PMCTRL_G3DCLKSEL_ADDR(base) ((base) + (0x1B4))
#define SOC_PMCTRL_G3D_GDVFS_EN_ADDR(base) ((base) + (0x1B8))
#define SOC_PMCTRL_G3D_GDVFS_PROFILE0_ADDR(base) ((base) + (0x1BC))
#define SOC_PMCTRL_G3D_GDVFS_PROFILE1_ADDR(base) ((base) + (0x1C0))
#define SOC_PMCTRL_G3D_GDVFS_PROFILE2_ADDR(base) ((base) + (0x1C4))
#define SOC_PMCTRL_G3D_GDVFS_PROFILE3_ADDR(base) ((base) + (0x1C8))
#define SOC_PMCTRL_G3DVOLIDX_ADDR(base) ((base) + (0x1CC))
#define SOC_PMCTRL_G3DMEMVOLIDX_ADDR(base) ((base) + (0x1D0))
#define SOC_PMCTRL_G3DDBTSTAT_ADDR(base) ((base) + (0x1D4))
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
#define SOC_PMCTRL_VOTE_MODE_CFG_ADDR(base) ((base) + (0x2C0))
#define SOC_PMCTRL_VOTE_START_EN_ADDR(base) ((base) + (0x2C4))
#define SOC_PMCTRL_INT_VOTE_EN_ADDR(base) ((base) + (0x2C8))
#define SOC_PMCTRL_INT_VOTE_CLR_ADDR(base) ((base) + (0x2CC))
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_ADDR(base) ((base) + (0x2D0))
#define SOC_PMCTRL_INT_VOTE_STAT_ADDR(base) ((base) + (0x2D4))
#define SOC_PMCTRL_INT_FREQUENCY_VOTE_ADDR(base) ((base) + (0x2D8))
#define SOC_PMCTRL_VOTE_RESULT0_ADDR(base) ((base) + (0x2DC))
#define SOC_PMCTRL_VOTE_RESULT1_ADDR(base) ((base) + (0x2E0))
#define SOC_PMCTRL_VOTE_RESULT2_ADDR(base) ((base) + (0x2E4))
#define SOC_PMCTRL_VOTE_RESULT3_ADDR(base) ((base) + (0x2E8))
#define SOC_PMCTRL_VOTE_RESULT4_ADDR(base) ((base) + (0x2EC))
#define SOC_PMCTRL_DDRLPCTRL_ADDR(base) ((base) + (0x30C))
#define SOC_PMCTRL_PLLLOCKTIME_ADDR(base) ((base) + (0x320))
#define SOC_PMCTRL_PLLLOCKMOD_ADDR(base) ((base) + (0x324))
#define SOC_PMCTRL_PMUSSIAVSEN_ADDR(base) ((base) + (0x330))
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
#define SOC_PMCTRL_PERI_STAT5_ADDR(base) ((base) + (0x374))
#define SOC_PMCTRL_PERI_STAT6_ADDR(base) ((base) + (0x378))
#define SOC_PMCTRL_PERI_STAT7_ADDR(base) ((base) + (0x37C))
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_ADDR(base) ((base) + (0x380))
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_ADDR(base) ((base) + (0x384))
#define SOC_PMCTRL_NOC_POWER_IDLE_0_ADDR(base) ((base) + (0x388))
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_ADDR(base) ((base) + (0x38C))
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_ADDR(base) ((base) + (0x390))
#define SOC_PMCTRL_NOC_POWER_IDLE_1_ADDR(base) ((base) + (0x394))
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_2_ADDR(base) ((base) + (0x398))
#define SOC_PMCTRL_NOC_POWER_IDLEACK_2_ADDR(base) ((base) + (0x39C))
#define SOC_PMCTRL_NOC_POWER_IDLE_2_ADDR(base) ((base) + (0x2F0))
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
#define SOC_PMCTRL_VS_CTRL_EN_NPU_ADDR(base) ((base) + (0x020))
#define SOC_PMCTRL_VS_CTRL_BYPASS_NPU_ADDR(base) ((base) + (0x024))
#define SOC_PMCTRL_VS_CTRL_NPU_ADDR(base) ((base) + (0x028))
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_ADDR(base) ((base) + (0x02C))
#define SOC_PMCTRL_VS_SVFD_CTRL1_NPU_ADDR(base) ((base) + (0x030))
#define SOC_PMCTRL_VS_SVFD_CTRL2_NPU_ADDR(base) ((base) + (0x034))
#define SOC_PMCTRL_VS_CPM_DATA_STAT_NPU_ADDR(base) ((base) + (0x038))
#define SOC_PMCTRL_VS_TEST_STAT_NPU_ADDR(base) ((base) + (0x03C))
#define SOC_PMCTRL_BOOTROMFLAG_ADDR(base) ((base) + (0x460))
#define SOC_PMCTRL_PERIHPMEN_ADDR(base) ((base) + (0x464))
#define SOC_PMCTRL_PERIHPMCLKDIV_ADDR(base) ((base) + (0x468))
#define SOC_PMCTRL_PERIHPMOPC_ADDR(base) ((base) + (0x46C))
#define SOC_PMCTRL_CLUSTER0_SW_INITIAL0_ADDR(base) ((base) + (0x470))
#define SOC_PMCTRL_CLUSTER0_SW_INITIAL1_ADDR(base) ((base) + (0x474))
#define SOC_PMCTRL_G3D_SW_INITIAL0_ADDR(base) ((base) + (0x478))
#define SOC_PMCTRL_G3D_SW_INITIAL1_ADDR(base) ((base) + (0x47C))
#define SOC_PMCTRL_L3_SW_INITIAL0_ADDR(base) ((base) + (0x480))
#define SOC_PMCTRL_L3_SW_INITIAL1_ADDR(base) ((base) + (0x484))
#define SOC_PMCTRL_PERI_STAT8_ADDR(base) ((base) + (0x4D0))
#define SOC_PMCTRL_PERI_STAT9_ADDR(base) ((base) + (0x4D4))
#define SOC_PMCTRL_PERI_STAT10_ADDR(base) ((base) + (0x4D8))
#define SOC_PMCTRL_PERI_STAT11_ADDR(base) ((base) + (0x4DC))
#define SOC_PMCTRL_PERI_STAT12_ADDR(base) ((base) + (0x4E0))
#define SOC_PMCTRL_PERI_STAT13_ADDR(base) ((base) + (0x4E4))
#define SOC_PMCTRL_PERI_STAT14_ADDR(base) ((base) + (0x4E8))
#define SOC_PMCTRL_PERI_STAT15_ADDR(base) ((base) + (0x4EC))
#define SOC_PMCTRL_PPLL0CTRL0_ADDR(base) ((base) + (0x4F0))
#define SOC_PMCTRL_PPLL0CTRL1_ADDR(base) ((base) + (0x4F4))
#define SOC_PMCTRL_PPLL0SSCCTRL_ADDR(base) ((base) + (0x4F8))
#define SOC_PMCTRL_PPLL1CTRL0_ADDR(base) ((base) + (0x500))
#define SOC_PMCTRL_PPLL1CTRL1_ADDR(base) ((base) + (0x504))
#define SOC_PMCTRL_PPLL2CTRL0_ADDR(base) ((base) + (0x508))
#define SOC_PMCTRL_PPLL2CTRL1_ADDR(base) ((base) + (0x50C))
#define SOC_PMCTRL_PPLL3CTRL0_ADDR(base) ((base) + (0x510))
#define SOC_PMCTRL_PPLL3CTRL1_ADDR(base) ((base) + (0x514))
#define SOC_PMCTRL_PPLL4CTRL0_ADDR(base) ((base) + (0x518))
#define SOC_PMCTRL_PPLL4CTRL1_ADDR(base) ((base) + (0x51C))
#define SOC_PMCTRL_PPLL5CTRL0_ADDR(base) ((base) + (0x520))
#define SOC_PMCTRL_PPLL5CTRL1_ADDR(base) ((base) + (0x524))
#define SOC_PMCTRL_PPLL6CTRL0_ADDR(base) ((base) + (0x528))
#define SOC_PMCTRL_PPLL6CTRL1_ADDR(base) ((base) + (0x52C))
#define SOC_PMCTRL_PPLL7CTRL0_ADDR(base) ((base) + (0x530))
#define SOC_PMCTRL_PPLL7CTRL1_ADDR(base) ((base) + (0x534))
#define SOC_PMCTRL_APLL0CTRL0_ADDR(base) ((base) + (0x540))
#define SOC_PMCTRL_APLL0CTRL1_ADDR(base) ((base) + (0x544))
#define SOC_PMCTRL_APLL0CTRL0_STAT_ADDR(base) ((base) + (0x548))
#define SOC_PMCTRL_APLL0CTRL1_STAT_ADDR(base) ((base) + (0x54C))
#define SOC_PMCTRL_APLL1CTRL0_ADDR(base) ((base) + (0x550))
#define SOC_PMCTRL_APLL1CTRL1_ADDR(base) ((base) + (0x554))
#define SOC_PMCTRL_APLL1CTRL0_STAT_ADDR(base) ((base) + (0x558))
#define SOC_PMCTRL_APLL1CTRL1_STAT_ADDR(base) ((base) + (0x55C))
#define SOC_PMCTRL_APLL2CTRL0_ADDR(base) ((base) + (0x560))
#define SOC_PMCTRL_APLL2CTRL1_ADDR(base) ((base) + (0x564))
#define SOC_PMCTRL_APLL2CTRL0_STAT_ADDR(base) ((base) + (0x568))
#define SOC_PMCTRL_APLL2CTRL1_STAT_ADDR(base) ((base) + (0x56C))
#define SOC_PMCTRL_APLL3CTRL0_ADDR(base) ((base) + (0x570))
#define SOC_PMCTRL_APLL3CTRL1_ADDR(base) ((base) + (0x574))
#define SOC_PMCTRL_APLL3CTRL0_STAT_ADDR(base) ((base) + (0x578))
#define SOC_PMCTRL_APLL3CTRL1_STAT_ADDR(base) ((base) + (0x57C))
#define SOC_PMCTRL_APLL4CTRL0_ADDR(base) ((base) + (0x580))
#define SOC_PMCTRL_APLL4CTRL1_ADDR(base) ((base) + (0x584))
#define SOC_PMCTRL_APLL4CTRL0_STAT_ADDR(base) ((base) + (0x588))
#define SOC_PMCTRL_APLL4CTRL1_STAT_ADDR(base) ((base) + (0x58C))
#define SOC_PMCTRL_APLL5CTRL0_ADDR(base) ((base) + (0x590))
#define SOC_PMCTRL_APLL5CTRL1_ADDR(base) ((base) + (0x594))
#define SOC_PMCTRL_APLL5CTRL0_STAT_ADDR(base) ((base) + (0x598))
#define SOC_PMCTRL_APLL5CTRL1_STAT_ADDR(base) ((base) + (0x59C))
#define SOC_PMCTRL_APLL6CTRL0_ADDR(base) ((base) + (0x5A0))
#define SOC_PMCTRL_APLL6CTRL1_ADDR(base) ((base) + (0x5A4))
#define SOC_PMCTRL_APLL6CTRL0_STAT_ADDR(base) ((base) + (0x5A8))
#define SOC_PMCTRL_APLL6CTRL1_STAT_ADDR(base) ((base) + (0x5AC))
#define SOC_PMCTRL_PPLL1SSCCTRL_ADDR(base) ((base) + (0x5B0))
#define SOC_PMCTRL_PPLL2SSCCTRL_ADDR(base) ((base) + (0x5B4))
#define SOC_PMCTRL_PPLL3SSCCTRL_ADDR(base) ((base) + (0x5B8))
#define SOC_PMCTRL_PPLL4SSCCTRL_ADDR(base) ((base) + (0x5BC))
#define SOC_PMCTRL_PPLL5SSCCTRL_ADDR(base) ((base) + (0x5C0))
#define SOC_PMCTRL_PPLL6SSCCTRL_ADDR(base) ((base) + (0x5C4))
#define SOC_PMCTRL_PPLL7SSCCTRL_ADDR(base) ((base) + (0x5C8))
#define SOC_PMCTRL_APLL2SSCCTRL_ADDR(base) ((base) + (0x5D8))
#define SOC_PMCTRL_L3_CLKSEL_ADDR(base) ((base) + (0x620))
#define SOC_PMCTRL_L3_CLKDIV_ADDR(base) ((base) + (0x624))
#define SOC_PMCTRL_L3_CLKPROFILE0_ADDR(base) ((base) + (0x628))
#define SOC_PMCTRL_L3_CLKPROFILE0_1_ADDR(base) ((base) + (0x62C))
#define SOC_PMCTRL_L3_CLKPROFILE1_ADDR(base) ((base) + (0x630))
#define SOC_PMCTRL_L3_VOLMOD_ADDR(base) ((base) + (0x634))
#define SOC_PMCTRL_L3_VOLPROFILE_ADDR(base) ((base) + (0x638))
#define SOC_PMCTRL_L3_GDVFS_PROFILE0_ADDR(base) ((base) + (0x650))
#define SOC_PMCTRL_L3_GDVFS_PROFILE1_ADDR(base) ((base) + (0x654))
#define SOC_PMCTRL_L3_VOLIDX_ADDR(base) ((base) + (0x658))
#define SOC_PMCTRL_LITTLE_INITIALVOL_ADDR(base) ((base) + (0x660))
#define SOC_PMCTRL_BIGFAST_DVFS_STAT_ADDR(base) ((base) + (0x900))
#define SOC_PMCTRL_BIGFAST_INITIALVOL_ADDR(base) ((base) + (0x904))
#define SOC_PMCTRL_BIGFAST_PMUSEL_ADDR(base) ((base) + (0x908))
#define SOC_PMCTRL_BIGFAST_GDVFS_EN_ADDR(base) ((base) + (0x90C))
#define SOC_PMCTRL_BIGFAST0_CLKSEL_ADDR(base) ((base) + (0x910))
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE0_ADDR(base) ((base) + (0x918))
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE0_1_ADDR(base) ((base) + (0x91C))
#define SOC_PMCTRL_BIGFAST0_VOLMOD_ADDR(base) ((base) + (0x920))
#define SOC_PMCTRL_BIGFAST0_VOLPROFILE_ADDR(base) ((base) + (0x924))
#define SOC_PMCTRL_BIGFAST0_PMU_UP_TIME_ADDR(base) ((base) + (0x928))
#define SOC_PMCTRL_BIGFAST0_PMU_DN_TIME_ADDR(base) ((base) + (0x92C))
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE0_ADDR(base) ((base) + (0x940))
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE1_ADDR(base) ((base) + (0x944))
#define SOC_PMCTRL_BIGFAST0_VOLIDX_SFTCFG_ADDR(base) ((base) + (0x948))
#define SOC_PMCTRL_BIGFAST0_APLL_INITIAL0_ADDR(base) ((base) + (0x950))
#define SOC_PMCTRL_BIGFAST0_APLL_INITIAL1_ADDR(base) ((base) + (0x954))
#define SOC_PMCTRL_BIGFAST1_CLKSEL_ADDR(base) ((base) + (0x960))
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE0_ADDR(base) ((base) + (0x968))
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE0_1_ADDR(base) ((base) + (0x96C))
#define SOC_PMCTRL_BIGFAST1_VOLMOD_ADDR(base) ((base) + (0x974))
#define SOC_PMCTRL_BIGFAST1_VOLPROFILE_ADDR(base) ((base) + (0x978))
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE0_ADDR(base) ((base) + (0x990))
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE1_ADDR(base) ((base) + (0x994))
#define SOC_PMCTRL_BIGFAST1_VOLIDX_SFTCFG_ADDR(base) ((base) + (0x998))
#define SOC_PMCTRL_BIGFAST1_APLL_INITIAL0_ADDR(base) ((base) + (0xA00))
#define SOC_PMCTRL_BIGFAST1_APLL_INITIAL1_ADDR(base) ((base) + (0xA04))
#define SOC_PMCTRL_BIGFAST_VAU_DBG_STAT_ADDR(base) ((base) + (0xA20))
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT0_ADDR(base) ((base) + (0xA24))
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT1_ADDR(base) ((base) + (0xA28))
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT2_ADDR(base) ((base) + (0xA2C))
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_DBG_STAT0_ADDR(base) ((base) + (0xA30))
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_DBG_STAT1_ADDR(base) ((base) + (0xA34))
#define SOC_PMCTRL_BIGFAST_CUR_VOL_DBG_STAT0_ADDR(base) ((base) + (0xA38))
#define SOC_PMCTRL_BIGSLOW_DVFS_STAT_ADDR(base) ((base) + (0xB00))
#define SOC_PMCTRL_BIGSLOW_INITIALVOL_ADDR(base) ((base) + (0xB04))
#define SOC_PMCTRL_BIGSLOW_PMUSEL_ADDR(base) ((base) + (0xB08))
#define SOC_PMCTRL_BIGSLOW_GDVFS_EN_ADDR(base) ((base) + (0xB0C))
#define SOC_PMCTRL_BIGSLOW0_CLKSEL_ADDR(base) ((base) + (0xB10))
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE0_ADDR(base) ((base) + (0xB18))
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE0_1_ADDR(base) ((base) + (0xB1C))
#define SOC_PMCTRL_BIGSLOW0_VOLMOD_ADDR(base) ((base) + (0xB20))
#define SOC_PMCTRL_BIGSLOW0_VOLPROFILE_ADDR(base) ((base) + (0xB24))
#define SOC_PMCTRL_BIGSLOW0_PMU_UP_TIME_ADDR(base) ((base) + (0xB28))
#define SOC_PMCTRL_BIGSLOW0_PMU_DN_TIME_ADDR(base) ((base) + (0xB2C))
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE0_ADDR(base) ((base) + (0xB40))
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE1_ADDR(base) ((base) + (0xB44))
#define SOC_PMCTRL_BIGSLOW0_VOLIDX_SFTCFG_ADDR(base) ((base) + (0xB48))
#define SOC_PMCTRL_BIGSLOW0_APLL_INITIAL0_ADDR(base) ((base) + (0xB50))
#define SOC_PMCTRL_BIGSLOW0_APLL_INITIAL1_ADDR(base) ((base) + (0xB54))
#define SOC_PMCTRL_BIGSLOW1_CLKSEL_ADDR(base) ((base) + (0xB60))
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE0_ADDR(base) ((base) + (0xB68))
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE0_1_ADDR(base) ((base) + (0xB6C))
#define SOC_PMCTRL_BIGSLOW1_VOLMOD_ADDR(base) ((base) + (0xB74))
#define SOC_PMCTRL_BIGSLOW1_VOLPROFILE_ADDR(base) ((base) + (0xB78))
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE0_ADDR(base) ((base) + (0xB90))
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE1_ADDR(base) ((base) + (0xB94))
#define SOC_PMCTRL_BIGSLOW1_VOLIDX_SFTCFG_ADDR(base) ((base) + (0xB98))
#define SOC_PMCTRL_BIGSLOW1_APLL_INITIAL0_ADDR(base) ((base) + (0xC00))
#define SOC_PMCTRL_BIGSLOW1_APLL_INITIAL1_ADDR(base) ((base) + (0xC04))
#define SOC_PMCTRL_BIGSLOW_VAU_DBG_STAT_ADDR(base) ((base) + (0xC20))
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT0_ADDR(base) ((base) + (0xC24))
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT1_ADDR(base) ((base) + (0xC28))
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT2_ADDR(base) ((base) + (0xC2C))
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_DBG_STAT0_ADDR(base) ((base) + (0xC30))
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_DBG_STAT1_ADDR(base) ((base) + (0xC34))
#define SOC_PMCTRL_BIGSLOW_CUR_VOL_DBG_STAT0_ADDR(base) ((base) + (0xC38))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_little_cpu_dvfs_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int int_mid_cpu_dvfs_en : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int int_big_cpu_dvfs_en : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int int_g3d_dvfs_en : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 14;
    } reg;
} SOC_PMCTRL_PMCINTEN_UNION;
#endif
#define SOC_PMCTRL_PMCINTEN_int_little_cpu_dvfs_en_START (0)
#define SOC_PMCTRL_PMCINTEN_int_little_cpu_dvfs_en_END (0)
#define SOC_PMCTRL_PMCINTEN_int_mid_cpu_dvfs_en_START (4)
#define SOC_PMCTRL_PMCINTEN_int_mid_cpu_dvfs_en_END (4)
#define SOC_PMCTRL_PMCINTEN_int_big_cpu_dvfs_en_START (8)
#define SOC_PMCTRL_PMCINTEN_int_big_cpu_dvfs_en_END (8)
#define SOC_PMCTRL_PMCINTEN_int_g3d_dvfs_en_START (12)
#define SOC_PMCTRL_PMCINTEN_int_g3d_dvfs_en_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_little_cpu_dvfs_stat : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int int_mid_cpu_dvfs_stat : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int int_big_cpu_dvfs_stat : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int int_g3d_dvfs_stat : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int int_cpu_dvfs_stat : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 14;
    } reg;
} SOC_PMCTRL_PMCINTSTAT_UNION;
#endif
#define SOC_PMCTRL_PMCINTSTAT_int_little_cpu_dvfs_stat_START (0)
#define SOC_PMCTRL_PMCINTSTAT_int_little_cpu_dvfs_stat_END (0)
#define SOC_PMCTRL_PMCINTSTAT_int_mid_cpu_dvfs_stat_START (4)
#define SOC_PMCTRL_PMCINTSTAT_int_mid_cpu_dvfs_stat_END (4)
#define SOC_PMCTRL_PMCINTSTAT_int_big_cpu_dvfs_stat_START (8)
#define SOC_PMCTRL_PMCINTSTAT_int_big_cpu_dvfs_stat_END (8)
#define SOC_PMCTRL_PMCINTSTAT_int_g3d_dvfs_stat_START (12)
#define SOC_PMCTRL_PMCINTSTAT_int_g3d_dvfs_stat_END (12)
#define SOC_PMCTRL_PMCINTSTAT_int_cpu_dvfs_stat_START (16)
#define SOC_PMCTRL_PMCINTSTAT_int_cpu_dvfs_stat_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_litlle_cpu_dvfs_clr : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int int_mid_cpu_dvfs_clr : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int int_big_cpu_dvfs_clr : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int int_g3d_dvfs_clr : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 14;
    } reg;
} SOC_PMCTRL_PMCINTCLR_UNION;
#endif
#define SOC_PMCTRL_PMCINTCLR_int_litlle_cpu_dvfs_clr_START (0)
#define SOC_PMCTRL_PMCINTCLR_int_litlle_cpu_dvfs_clr_END (0)
#define SOC_PMCTRL_PMCINTCLR_int_mid_cpu_dvfs_clr_START (4)
#define SOC_PMCTRL_PMCINTCLR_int_mid_cpu_dvfs_clr_END (4)
#define SOC_PMCTRL_PMCINTCLR_int_big_cpu_dvfs_clr_START (8)
#define SOC_PMCTRL_PMCINTCLR_int_big_cpu_dvfs_clr_END (8)
#define SOC_PMCTRL_PMCINTCLR_int_g3d_dvfs_clr_START (12)
#define SOC_PMCTRL_PMCINTCLR_int_g3d_dvfs_clr_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int little_dvfs_auto_gt_bypass : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int mid_dvfs_auto_gt_bypass : 1;
        unsigned int reserved_3 : 3;
        unsigned int big_dvfs_auto_gt_bypass : 1;
        unsigned int reserved_4 : 3;
        unsigned int g3d_dvfs_auto_gt_bypass : 1;
        unsigned int reserved_5 : 3;
        unsigned int gt_ppll0_div_wait_time : 4;
        unsigned int reserved_6 : 12;
    } reg;
} SOC_PMCTRL_DVFS_AUTOGT_BP_UNION;
#endif
#define SOC_PMCTRL_DVFS_AUTOGT_BP_little_dvfs_auto_gt_bypass_START (0)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_little_dvfs_auto_gt_bypass_END (0)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_mid_dvfs_auto_gt_bypass_START (4)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_mid_dvfs_auto_gt_bypass_END (4)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_big_dvfs_auto_gt_bypass_START (8)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_big_dvfs_auto_gt_bypass_END (8)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_g3d_dvfs_auto_gt_bypass_START (12)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_g3d_dvfs_auto_gt_bypass_END (12)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_gt_ppll0_div_wait_time_START (16)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_gt_ppll0_div_wait_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int little_freq_fsm_cur_stat0 : 4;
        unsigned int little_freq_fsm_cur_stat1 : 4;
        unsigned int little_parallel_dvfs_fsm_cur_stat : 3;
        unsigned int reserved_0 : 1;
        unsigned int little_vau_fsm_cur_stat : 8;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_CLUSTER0_DVFS_STAT_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_DVFS_STAT_little_freq_fsm_cur_stat0_START (0)
#define SOC_PMCTRL_CLUSTER0_DVFS_STAT_little_freq_fsm_cur_stat0_END (3)
#define SOC_PMCTRL_CLUSTER0_DVFS_STAT_little_freq_fsm_cur_stat1_START (4)
#define SOC_PMCTRL_CLUSTER0_DVFS_STAT_little_freq_fsm_cur_stat1_END (7)
#define SOC_PMCTRL_CLUSTER0_DVFS_STAT_little_parallel_dvfs_fsm_cur_stat_START (8)
#define SOC_PMCTRL_CLUSTER0_DVFS_STAT_little_parallel_dvfs_fsm_cur_stat_END (10)
#define SOC_PMCTRL_CLUSTER0_DVFS_STAT_little_vau_fsm_cur_stat_START (12)
#define SOC_PMCTRL_CLUSTER0_DVFS_STAT_little_vau_fsm_cur_stat_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_clk_sel : 1;
        unsigned int reserved_0 : 3;
        unsigned int cluster0_clk_sel_stat : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_PMCTRL_CLUSTER0_CPUCLKSEL_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_CPUCLKSEL_cluster0_clk_sel_START (0)
#define SOC_PMCTRL_CLUSTER0_CPUCLKSEL_cluster0_clk_sel_END (0)
#define SOC_PMCTRL_CLUSTER0_CPUCLKSEL_cluster0_clk_sel_stat_START (4)
#define SOC_PMCTRL_CLUSTER0_CPUCLKSEL_cluster0_clk_sel_stat_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_pclkendbg_sel_cfg : 2;
        unsigned int cluster0_atclken_sel_cfg : 2;
        unsigned int cluster0_aclkenm_sel_cfg : 3;
        unsigned int cluster0_atclken_l_sel_cfg : 1;
        unsigned int cluster0_pclkendbg_sel_stat : 2;
        unsigned int cluster0_atclken_sel_stat : 2;
        unsigned int cluster0_aclkenm_sel_stat : 3;
        unsigned int cluster0_atclken_l_sel_stat : 1;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_CLUSTER0_CPUCLKDIV_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_cluster0_pclkendbg_sel_cfg_START (0)
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_cluster0_pclkendbg_sel_cfg_END (1)
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_cluster0_atclken_sel_cfg_START (2)
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_cluster0_atclken_sel_cfg_END (3)
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_cluster0_aclkenm_sel_cfg_START (4)
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_cluster0_aclkenm_sel_cfg_END (6)
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_cluster0_atclken_l_sel_cfg_START (7)
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_cluster0_atclken_l_sel_cfg_END (7)
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_cluster0_pclkendbg_sel_stat_START (8)
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_cluster0_pclkendbg_sel_stat_END (9)
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_cluster0_atclken_sel_stat_START (10)
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_cluster0_atclken_sel_stat_END (11)
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_cluster0_aclkenm_sel_stat_START (12)
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_cluster0_aclkenm_sel_stat_END (14)
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_cluster0_atclken_l_sel_stat_START (15)
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_cluster0_atclken_l_sel_stat_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_apll_fbdiv : 12;
        unsigned int reserved : 20;
    } reg;
} SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE0_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE0_cluster0_apll_fbdiv_START (0)
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE0_cluster0_apll_fbdiv_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_apll_fracdiv : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE0_1_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE0_1_cluster0_apll_fracdiv_START (0)
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE0_1_cluster0_apll_fracdiv_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_pclkendbg_sel_prof : 2;
        unsigned int cluster0_atclken_sel_prof : 2;
        unsigned int cluster0_aclkenm_sel_prof : 3;
        unsigned int cluster0_atclken_l_sel_prof : 1;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE1_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE1_cluster0_pclkendbg_sel_prof_START (0)
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE1_cluster0_pclkendbg_sel_prof_END (1)
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE1_cluster0_atclken_sel_prof_START (2)
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE1_cluster0_atclken_sel_prof_END (3)
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE1_cluster0_aclkenm_sel_prof_START (4)
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE1_cluster0_aclkenm_sel_prof_END (6)
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE1_cluster0_atclken_l_sel_prof_START (7)
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE1_cluster0_atclken_l_sel_prof_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_dvfs_vol_bypass : 1;
        unsigned int reserved_0 : 3;
        unsigned int cluster0_dvfs_apll_freq_bypass : 1;
        unsigned int reserved_1 : 3;
        unsigned int cluster0_dvfs_vol_updn : 1;
        unsigned int reserved_2 : 3;
        unsigned int cluster0_dvfs_apll_freq_updn : 1;
        unsigned int reserved_3 : 19;
    } reg;
} SOC_PMCTRL_CLUSTER0_CPUVOLMOD_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_CPUVOLMOD_cluster0_dvfs_vol_bypass_START (0)
#define SOC_PMCTRL_CLUSTER0_CPUVOLMOD_cluster0_dvfs_vol_bypass_END (0)
#define SOC_PMCTRL_CLUSTER0_CPUVOLMOD_cluster0_dvfs_apll_freq_bypass_START (4)
#define SOC_PMCTRL_CLUSTER0_CPUVOLMOD_cluster0_dvfs_apll_freq_bypass_END (4)
#define SOC_PMCTRL_CLUSTER0_CPUVOLMOD_cluster0_dvfs_vol_updn_START (8)
#define SOC_PMCTRL_CLUSTER0_CPUVOLMOD_cluster0_dvfs_vol_updn_END (8)
#define SOC_PMCTRL_CLUSTER0_CPUVOLMOD_cluster0_dvfs_apll_freq_updn_START (12)
#define SOC_PMCTRL_CLUSTER0_CPUVOLMOD_cluster0_dvfs_apll_freq_updn_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_cpu_vol_idx : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_CLUSTER0_CPUVOLPROFILE_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_CPUVOLPROFILE_cluster0_cpu_vol_idx_START (0)
#define SOC_PMCTRL_CLUSTER0_CPUVOLPROFILE_cluster0_cpu_vol_idx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_pmu_up_vol_hold_time : 20;
        unsigned int cluster0_vol_up_step_time : 12;
    } reg;
} SOC_PMCTRL_CLUSTER0_PMU_UP_TIME_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_PMU_UP_TIME_cluster0_pmu_up_vol_hold_time_START (0)
#define SOC_PMCTRL_CLUSTER0_PMU_UP_TIME_cluster0_pmu_up_vol_hold_time_END (19)
#define SOC_PMCTRL_CLUSTER0_PMU_UP_TIME_cluster0_vol_up_step_time_START (20)
#define SOC_PMCTRL_CLUSTER0_PMU_UP_TIME_cluster0_vol_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_pmu_dn_vol_hold_time : 20;
        unsigned int cluster0_vol_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_CLUSTER0_PMU_DN_TIME_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_PMU_DN_TIME_cluster0_pmu_dn_vol_hold_time_START (0)
#define SOC_PMCTRL_CLUSTER0_PMU_DN_TIME_cluster0_pmu_dn_vol_hold_time_END (19)
#define SOC_PMCTRL_CLUSTER0_PMU_DN_TIME_cluster0_vol_dn_step_time_START (20)
#define SOC_PMCTRL_CLUSTER0_PMU_DN_TIME_cluster0_vol_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_pmu_sel : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_CLUSTER0_PMUSEL_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_PMUSEL_cluster0_pmu_sel_START (0)
#define SOC_PMCTRL_CLUSTER0_PMUSEL_cluster0_pmu_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_cpu_vol_idx_cfg : 8;
        unsigned int cluster0_cpu_vol_chg_sftreq_vote_en : 1;
        unsigned int cluster0_cpu_vol_chg_sftreq : 1;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 8;
        unsigned int reserved_2 : 12;
    } reg;
} SOC_PMCTRL_CLUSTER0_CPUVOLIDX_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_CPUVOLIDX_cluster0_cpu_vol_idx_cfg_START (0)
#define SOC_PMCTRL_CLUSTER0_CPUVOLIDX_cluster0_cpu_vol_idx_cfg_END (7)
#define SOC_PMCTRL_CLUSTER0_CPUVOLIDX_cluster0_cpu_vol_chg_sftreq_vote_en_START (8)
#define SOC_PMCTRL_CLUSTER0_CPUVOLIDX_cluster0_cpu_vol_chg_sftreq_vote_en_END (8)
#define SOC_PMCTRL_CLUSTER0_CPUVOLIDX_cluster0_cpu_vol_chg_sftreq_START (9)
#define SOC_PMCTRL_CLUSTER0_CPUVOLIDX_cluster0_cpu_vol_chg_sftreq_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_cpu_gdvfs_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_CLUSTER0_GDVFS_EN_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_GDVFS_EN_cluster0_cpu_gdvfs_en_START (0)
#define SOC_PMCTRL_CLUSTER0_GDVFS_EN_cluster0_cpu_gdvfs_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_gdvfs_profile_updn : 1;
        unsigned int reserved_0 : 3;
        unsigned int cluster0_dvfs_mode : 2;
        unsigned int reserved_1 : 2;
        unsigned int cluster0_step_number : 8;
        unsigned int cluster0_clksel_prof : 1;
        unsigned int reserved_2 : 3;
        unsigned int cluster0_gdvfs_ctrl_apll : 1;
        unsigned int reserved_3 : 11;
    } reg;
} SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE0_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE0_cluster0_gdvfs_profile_updn_START (0)
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE0_cluster0_gdvfs_profile_updn_END (0)
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE0_cluster0_dvfs_mode_START (4)
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE0_cluster0_dvfs_mode_END (5)
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE0_cluster0_step_number_START (8)
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE0_cluster0_step_number_END (15)
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE0_cluster0_clksel_prof_START (16)
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE0_cluster0_clksel_prof_END (16)
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE0_cluster0_gdvfs_ctrl_apll_START (20)
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE0_cluster0_gdvfs_ctrl_apll_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 2;
        unsigned int cluster0_apll_cfg_time : 12;
        unsigned int cluster0_clkdiv_time : 7;
        unsigned int reserved_2 : 9;
    } reg;
} SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE1_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE1_cluster0_apll_cfg_time_START (4)
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE1_cluster0_apll_cfg_time_END (15)
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE1_cluster0_clkdiv_time_START (16)
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE1_cluster0_clkdiv_time_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int little_nxt_max_buck_vol : 8;
        unsigned int little_cur_max_buck_vol : 8;
        unsigned int little_cur_max_buck_vol_tmp : 8;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_CLUSTER0_MAX_VOL_DBG_STAT_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_MAX_VOL_DBG_STAT_little_nxt_max_buck_vol_START (0)
#define SOC_PMCTRL_CLUSTER0_MAX_VOL_DBG_STAT_little_nxt_max_buck_vol_END (7)
#define SOC_PMCTRL_CLUSTER0_MAX_VOL_DBG_STAT_little_cur_max_buck_vol_START (8)
#define SOC_PMCTRL_CLUSTER0_MAX_VOL_DBG_STAT_little_cur_max_buck_vol_END (15)
#define SOC_PMCTRL_CLUSTER0_MAX_VOL_DBG_STAT_little_cur_max_buck_vol_tmp_START (16)
#define SOC_PMCTRL_CLUSTER0_MAX_VOL_DBG_STAT_little_cur_max_buck_vol_tmp_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_unified_vol_idx_stat0 : 8;
        unsigned int cluster0_unified_vol_idx_stat1 : 8;
        unsigned int cluster0_unified_pmu_vol_vol_idx_stat : 8;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_CLUSTER0_UNIFIED_VOL_DBG_STAT_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_UNIFIED_VOL_DBG_STAT_cluster0_unified_vol_idx_stat0_START (0)
#define SOC_PMCTRL_CLUSTER0_UNIFIED_VOL_DBG_STAT_cluster0_unified_vol_idx_stat0_END (7)
#define SOC_PMCTRL_CLUSTER0_UNIFIED_VOL_DBG_STAT_cluster0_unified_vol_idx_stat1_START (8)
#define SOC_PMCTRL_CLUSTER0_UNIFIED_VOL_DBG_STAT_cluster0_unified_vol_idx_stat1_END (15)
#define SOC_PMCTRL_CLUSTER0_UNIFIED_VOL_DBG_STAT_cluster0_unified_pmu_vol_vol_idx_stat_START (16)
#define SOC_PMCTRL_CLUSTER0_UNIFIED_VOL_DBG_STAT_cluster0_unified_pmu_vol_vol_idx_stat_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_cur_pmu_vol_idx_stat0 : 8;
        unsigned int cluster0_cur_pmu_vol_idx_stat1 : 8;
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_PMCTRL_CLUSTER0_CUR_VOL_DBG_STAT_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_CUR_VOL_DBG_STAT_cluster0_cur_pmu_vol_idx_stat0_START (0)
#define SOC_PMCTRL_CLUSTER0_CUR_VOL_DBG_STAT_cluster0_cur_pmu_vol_idx_stat0_END (7)
#define SOC_PMCTRL_CLUSTER0_CUR_VOL_DBG_STAT_cluster0_cur_pmu_vol_idx_stat1_START (8)
#define SOC_PMCTRL_CLUSTER0_CUR_VOL_DBG_STAT_cluster0_cur_pmu_vol_idx_stat1_END (15)
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
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_GPU_HPMEN_UNION;
#endif
#define SOC_PMCTRL_GPU_HPMEN_gpu_hpm_en_START (0)
#define SOC_PMCTRL_GPU_HPMEN_gpu_hpm_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpu_hpm_opc : 10;
        unsigned int reserved_0 : 2;
        unsigned int gpu_hpm_opc_vld : 1;
        unsigned int reserved_1 : 19;
    } reg;
} SOC_PMCTRL_GPU_HPMOPC_UNION;
#endif
#define SOC_PMCTRL_GPU_HPMOPC_gpu_hpm_opc_START (0)
#define SOC_PMCTRL_GPU_HPMOPC_gpu_hpm_opc_END (9)
#define SOC_PMCTRL_GPU_HPMOPC_gpu_hpm_opc_vld_START (12)
#define SOC_PMCTRL_GPU_HPMOPC_gpu_hpm_opc_vld_END (12)
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
        unsigned int bigfast_phase_idx_cfg : 8;
        unsigned int bigfast_phase_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 20;
    } reg;
} SOC_PMCTRL_BIGFAST_PHASEIDX_UNION;
#endif
#define SOC_PMCTRL_BIGFAST_PHASEIDX_bigfast_phase_idx_cfg_START (0)
#define SOC_PMCTRL_BIGFAST_PHASEIDX_bigfast_phase_idx_cfg_END (7)
#define SOC_PMCTRL_BIGFAST_PHASEIDX_bigfast_phase_sftreq_START (8)
#define SOC_PMCTRL_BIGFAST_PHASEIDX_bigfast_phase_sftreq_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow_phase_idx_cfg : 8;
        unsigned int bigslow_phase_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 20;
    } reg;
} SOC_PMCTRL_BIGSLOW_PHASEIDX_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW_PHASEIDX_bigslow_phase_idx_cfg_START (0)
#define SOC_PMCTRL_BIGSLOW_PHASEIDX_bigslow_phase_idx_cfg_END (7)
#define SOC_PMCTRL_BIGSLOW_PHASEIDX_bigslow_phase_sftreq_START (8)
#define SOC_PMCTRL_BIGSLOW_PHASEIDX_bigslow_phase_sftreq_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int little_phase_idx_cfg : 8;
        unsigned int little_phase_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 20;
    } reg;
} SOC_PMCTRL_LITTLE_PHASEIDX_UNION;
#endif
#define SOC_PMCTRL_LITTLE_PHASEIDX_little_phase_idx_cfg_START (0)
#define SOC_PMCTRL_LITTLE_PHASEIDX_little_phase_idx_cfg_END (7)
#define SOC_PMCTRL_LITTLE_PHASEIDX_little_phase_sftreq_START (8)
#define SOC_PMCTRL_LITTLE_PHASEIDX_little_phase_sftreq_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpu_phase_idx_cfg : 8;
        unsigned int gpu_phase_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 20;
    } reg;
} SOC_PMCTRL_GPU_PHASEIDX_UNION;
#endif
#define SOC_PMCTRL_GPU_PHASEIDX_gpu_phase_idx_cfg_START (0)
#define SOC_PMCTRL_GPU_PHASEIDX_gpu_phase_idx_cfg_END (7)
#define SOC_PMCTRL_GPU_PHASEIDX_gpu_phase_sftreq_START (8)
#define SOC_PMCTRL_GPU_PHASEIDX_gpu_phase_sftreq_END (8)
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
        unsigned int g3d_vol_dn_step_time : 12;
        unsigned int reserved_0 : 4;
        unsigned int g3d_vol_up_step_time : 12;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_G3D_PMU_VOLSTEPTIME_UNION;
#endif
#define SOC_PMCTRL_G3D_PMU_VOLSTEPTIME_g3d_vol_dn_step_time_START (0)
#define SOC_PMCTRL_G3D_PMU_VOLSTEPTIME_g3d_vol_dn_step_time_END (11)
#define SOC_PMCTRL_G3D_PMU_VOLSTEPTIME_g3d_vol_up_step_time_START (16)
#define SOC_PMCTRL_G3D_PMU_VOLSTEPTIME_g3d_vol_up_step_time_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_pmu_up_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_G3D_PMU_UP_HOLDTIME_UNION;
#endif
#define SOC_PMCTRL_G3D_PMU_UP_HOLDTIME_g3d_pmu_up_vol_hold_time_START (0)
#define SOC_PMCTRL_G3D_PMU_UP_HOLDTIME_g3d_pmu_up_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_pmu_dn_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_G3D_PMU_DN_HOLDTIME_UNION;
#endif
#define SOC_PMCTRL_G3D_PMU_DN_HOLDTIME_g3d_pmu_dn_vol_hold_time_START (0)
#define SOC_PMCTRL_G3D_PMU_DN_HOLDTIME_g3d_pmu_dn_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_mem_vol_dn_step_time : 12;
        unsigned int reserved_0 : 4;
        unsigned int g3d_mem_vol_up_step_time : 12;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_G3D_MEM_PMU_VOLSTEPTIME_UNION;
#endif
#define SOC_PMCTRL_G3D_MEM_PMU_VOLSTEPTIME_g3d_mem_vol_dn_step_time_START (0)
#define SOC_PMCTRL_G3D_MEM_PMU_VOLSTEPTIME_g3d_mem_vol_dn_step_time_END (11)
#define SOC_PMCTRL_G3D_MEM_PMU_VOLSTEPTIME_g3d_mem_vol_up_step_time_START (16)
#define SOC_PMCTRL_G3D_MEM_PMU_VOLSTEPTIME_g3d_mem_vol_up_step_time_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_mem_up_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_G3D_MEM_PMU_UP_HOLDTIME_UNION;
#endif
#define SOC_PMCTRL_G3D_MEM_PMU_UP_HOLDTIME_g3d_mem_up_vol_hold_time_START (0)
#define SOC_PMCTRL_G3D_MEM_PMU_UP_HOLDTIME_g3d_mem_up_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_mem_dn_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_G3D_MEM_PMU_DN_HOLDTIME_UNION;
#endif
#define SOC_PMCTRL_G3D_MEM_PMU_DN_HOLDTIME_g3d_mem_dn_vol_hold_time_START (0)
#define SOC_PMCTRL_G3D_MEM_PMU_DN_HOLDTIME_g3d_mem_dn_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_vol_index_0 : 8;
        unsigned int g3d_vol_index_1 : 8;
        unsigned int g3d_vol_index_2 : 8;
        unsigned int g3d_vol_index_3 : 8;
    } reg;
} SOC_PMCTRL_G3D_VOL_INDEX0_UNION;
#endif
#define SOC_PMCTRL_G3D_VOL_INDEX0_g3d_vol_index_0_START (0)
#define SOC_PMCTRL_G3D_VOL_INDEX0_g3d_vol_index_0_END (7)
#define SOC_PMCTRL_G3D_VOL_INDEX0_g3d_vol_index_1_START (8)
#define SOC_PMCTRL_G3D_VOL_INDEX0_g3d_vol_index_1_END (15)
#define SOC_PMCTRL_G3D_VOL_INDEX0_g3d_vol_index_2_START (16)
#define SOC_PMCTRL_G3D_VOL_INDEX0_g3d_vol_index_2_END (23)
#define SOC_PMCTRL_G3D_VOL_INDEX0_g3d_vol_index_3_START (24)
#define SOC_PMCTRL_G3D_VOL_INDEX0_g3d_vol_index_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_vol_index_4 : 8;
        unsigned int g3d_vol_index_5 : 8;
        unsigned int g3d_vol_index_6 : 8;
        unsigned int g3d_vol_index_7 : 8;
    } reg;
} SOC_PMCTRL_G3D_VOL_INDEX1_UNION;
#endif
#define SOC_PMCTRL_G3D_VOL_INDEX1_g3d_vol_index_4_START (0)
#define SOC_PMCTRL_G3D_VOL_INDEX1_g3d_vol_index_4_END (7)
#define SOC_PMCTRL_G3D_VOL_INDEX1_g3d_vol_index_5_START (8)
#define SOC_PMCTRL_G3D_VOL_INDEX1_g3d_vol_index_5_END (15)
#define SOC_PMCTRL_G3D_VOL_INDEX1_g3d_vol_index_6_START (16)
#define SOC_PMCTRL_G3D_VOL_INDEX1_g3d_vol_index_6_END (23)
#define SOC_PMCTRL_G3D_VOL_INDEX1_g3d_vol_index_7_START (24)
#define SOC_PMCTRL_G3D_VOL_INDEX1_g3d_vol_index_7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_vol_index_8 : 8;
        unsigned int g3d_vol_index_9 : 8;
        unsigned int g3d_vol_index_10 : 8;
        unsigned int g3d_vol_index_11 : 8;
    } reg;
} SOC_PMCTRL_G3D_VOL_INDEX2_UNION;
#endif
#define SOC_PMCTRL_G3D_VOL_INDEX2_g3d_vol_index_8_START (0)
#define SOC_PMCTRL_G3D_VOL_INDEX2_g3d_vol_index_8_END (7)
#define SOC_PMCTRL_G3D_VOL_INDEX2_g3d_vol_index_9_START (8)
#define SOC_PMCTRL_G3D_VOL_INDEX2_g3d_vol_index_9_END (15)
#define SOC_PMCTRL_G3D_VOL_INDEX2_g3d_vol_index_10_START (16)
#define SOC_PMCTRL_G3D_VOL_INDEX2_g3d_vol_index_10_END (23)
#define SOC_PMCTRL_G3D_VOL_INDEX2_g3d_vol_index_11_START (24)
#define SOC_PMCTRL_G3D_VOL_INDEX2_g3d_vol_index_11_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_vol_index_12 : 8;
        unsigned int g3d_vol_index_13 : 8;
        unsigned int g3d_vol_index_14 : 8;
        unsigned int g3d_vol_index_15 : 8;
    } reg;
} SOC_PMCTRL_G3D_VOL_INDEX3_UNION;
#endif
#define SOC_PMCTRL_G3D_VOL_INDEX3_g3d_vol_index_12_START (0)
#define SOC_PMCTRL_G3D_VOL_INDEX3_g3d_vol_index_12_END (7)
#define SOC_PMCTRL_G3D_VOL_INDEX3_g3d_vol_index_13_START (8)
#define SOC_PMCTRL_G3D_VOL_INDEX3_g3d_vol_index_13_END (15)
#define SOC_PMCTRL_G3D_VOL_INDEX3_g3d_vol_index_14_START (16)
#define SOC_PMCTRL_G3D_VOL_INDEX3_g3d_vol_index_14_END (23)
#define SOC_PMCTRL_G3D_VOL_INDEX3_g3d_vol_index_15_START (24)
#define SOC_PMCTRL_G3D_VOL_INDEX3_g3d_vol_index_15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_mem_vol_index_0 : 8;
        unsigned int g3d_mem_vol_index_1 : 8;
        unsigned int g3d_mem_vol_index_2 : 8;
        unsigned int g3d_mem_vol_index_3 : 8;
    } reg;
} SOC_PMCTRL_G3D_MEM_VOL_INDEX0_UNION;
#endif
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX0_g3d_mem_vol_index_0_START (0)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX0_g3d_mem_vol_index_0_END (7)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX0_g3d_mem_vol_index_1_START (8)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX0_g3d_mem_vol_index_1_END (15)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX0_g3d_mem_vol_index_2_START (16)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX0_g3d_mem_vol_index_2_END (23)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX0_g3d_mem_vol_index_3_START (24)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX0_g3d_mem_vol_index_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_mem_vol_index_4 : 8;
        unsigned int g3d_mem_vol_index_5 : 8;
        unsigned int g3d_mem_vol_index_6 : 8;
        unsigned int g3d_mem_vol_index_7 : 8;
    } reg;
} SOC_PMCTRL_G3D_MEM_VOL_INDEX1_UNION;
#endif
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX1_g3d_mem_vol_index_4_START (0)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX1_g3d_mem_vol_index_4_END (7)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX1_g3d_mem_vol_index_5_START (8)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX1_g3d_mem_vol_index_5_END (15)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX1_g3d_mem_vol_index_6_START (16)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX1_g3d_mem_vol_index_6_END (23)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX1_g3d_mem_vol_index_7_START (24)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX1_g3d_mem_vol_index_7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_mem_vol_index_8 : 8;
        unsigned int g3d_mem_vol_index_9 : 8;
        unsigned int g3d_mem_vol_index_10 : 8;
        unsigned int g3d_mem_vol_index_11 : 8;
    } reg;
} SOC_PMCTRL_G3D_MEM_VOL_INDEX2_UNION;
#endif
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX2_g3d_mem_vol_index_8_START (0)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX2_g3d_mem_vol_index_8_END (7)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX2_g3d_mem_vol_index_9_START (8)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX2_g3d_mem_vol_index_9_END (15)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX2_g3d_mem_vol_index_10_START (16)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX2_g3d_mem_vol_index_10_END (23)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX2_g3d_mem_vol_index_11_START (24)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX2_g3d_mem_vol_index_11_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_mem_vol_index_12 : 8;
        unsigned int g3d_mem_vol_index_13 : 8;
        unsigned int g3d_mem_vol_index_14 : 8;
        unsigned int g3d_mem_vol_index_15 : 8;
    } reg;
} SOC_PMCTRL_G3D_MEM_VOL_INDEX3_UNION;
#endif
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX3_g3d_mem_vol_index_12_START (0)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX3_g3d_mem_vol_index_12_END (7)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX3_g3d_mem_vol_index_13_START (8)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX3_g3d_mem_vol_index_13_END (15)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX3_g3d_mem_vol_index_14_START (16)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX3_g3d_mem_vol_index_14_END (23)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX3_g3d_mem_vol_index_15_START (24)
#define SOC_PMCTRL_G3D_MEM_VOL_INDEX3_g3d_mem_vol_index_15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_pmu_sel : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_G3DPMUSEL_UNION;
#endif
#define SOC_PMCTRL_G3DPMUSEL_g3d_pmu_sel_START (0)
#define SOC_PMCTRL_G3DPMUSEL_g3d_pmu_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_vol_bypass : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_G3D_VOL_BYPASS_UNION;
#endif
#define SOC_PMCTRL_G3D_VOL_BYPASS_g3d_vol_bypass_START (0)
#define SOC_PMCTRL_G3D_VOL_BYPASS_g3d_vol_bypass_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_mem_pmu_sel : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_G3D_MEM_PMUSEL_UNION;
#endif
#define SOC_PMCTRL_G3D_MEM_PMUSEL_g3d_mem_pmu_sel_START (0)
#define SOC_PMCTRL_G3D_MEM_PMUSEL_g3d_mem_pmu_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_mem_vol_bypass : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_G3D_MEM_VOL_BYPASS_UNION;
#endif
#define SOC_PMCTRL_G3D_MEM_VOL_BYPASS_g3d_mem_vol_bypass_START (0)
#define SOC_PMCTRL_G3D_MEM_VOL_BYPASS_g3d_mem_vol_bypass_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_initial_vol_idx : 8;
        unsigned int g3d_initial_vol_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 20;
    } reg;
} SOC_PMCTRL_G3D_INITIALVOL_UNION;
#endif
#define SOC_PMCTRL_G3D_INITIALVOL_g3d_initial_vol_idx_START (0)
#define SOC_PMCTRL_G3D_INITIALVOL_g3d_initial_vol_idx_END (7)
#define SOC_PMCTRL_G3D_INITIALVOL_g3d_initial_vol_sftreq_START (8)
#define SOC_PMCTRL_G3D_INITIALVOL_g3d_initial_vol_sftreq_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_mem_initial_vol_idx : 8;
        unsigned int g3d_mem_initial_vol_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 20;
    } reg;
} SOC_PMCTRL_G3D_MEM_INITIALVOL_UNION;
#endif
#define SOC_PMCTRL_G3D_MEM_INITIALVOL_g3d_mem_initial_vol_idx_START (0)
#define SOC_PMCTRL_G3D_MEM_INITIALVOL_g3d_mem_initial_vol_idx_END (7)
#define SOC_PMCTRL_G3D_MEM_INITIALVOL_g3d_mem_initial_vol_sftreq_START (8)
#define SOC_PMCTRL_G3D_MEM_INITIALVOL_g3d_mem_initial_vol_sftreq_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3da_clk_sel : 1;
        unsigned int reserved_0 : 3;
        unsigned int g3d_gdvfs_apll_freq_updn : 1;
        unsigned int reserved_1 : 3;
        unsigned int g3d_dvfs_vol_updn : 1;
        unsigned int reserved_2 : 3;
        unsigned int g3d_gdvfs_apll_freq_bypass : 1;
        unsigned int reserved_3 : 19;
    } reg;
} SOC_PMCTRL_G3DCLKPROFILE_UNION;
#endif
#define SOC_PMCTRL_G3DCLKPROFILE_g3da_clk_sel_START (0)
#define SOC_PMCTRL_G3DCLKPROFILE_g3da_clk_sel_END (0)
#define SOC_PMCTRL_G3DCLKPROFILE_g3d_gdvfs_apll_freq_updn_START (4)
#define SOC_PMCTRL_G3DCLKPROFILE_g3d_gdvfs_apll_freq_updn_END (4)
#define SOC_PMCTRL_G3DCLKPROFILE_g3d_dvfs_vol_updn_START (8)
#define SOC_PMCTRL_G3DCLKPROFILE_g3d_dvfs_vol_updn_END (8)
#define SOC_PMCTRL_G3DCLKPROFILE_g3d_gdvfs_apll_freq_bypass_START (12)
#define SOC_PMCTRL_G3DCLKPROFILE_g3d_gdvfs_apll_freq_bypass_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3da_sw_cfg : 1;
        unsigned int g3da_sw_ack : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int g3da_sw_stat : 1;
        unsigned int reserved_2 : 27;
    } reg;
} SOC_PMCTRL_G3DCLKSEL_UNION;
#endif
#define SOC_PMCTRL_G3DCLKSEL_g3da_sw_cfg_START (0)
#define SOC_PMCTRL_G3DCLKSEL_g3da_sw_cfg_END (0)
#define SOC_PMCTRL_G3DCLKSEL_g3da_sw_ack_START (1)
#define SOC_PMCTRL_G3DCLKSEL_g3da_sw_ack_END (1)
#define SOC_PMCTRL_G3DCLKSEL_g3da_sw_stat_START (4)
#define SOC_PMCTRL_G3DCLKSEL_g3da_sw_stat_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_cpu_gdvfs_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_G3D_GDVFS_EN_UNION;
#endif
#define SOC_PMCTRL_G3D_GDVFS_EN_g3d_cpu_gdvfs_en_START (0)
#define SOC_PMCTRL_G3D_GDVFS_EN_g3d_cpu_gdvfs_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_gdvfs_profile_updn : 1;
        unsigned int reserved_0 : 3;
        unsigned int g3d_dvfs_mode : 2;
        unsigned int reserved_1 : 2;
        unsigned int g3d_step_number : 8;
        unsigned int g3d_gdvfs_ctrl_apll : 1;
        unsigned int reserved_2 : 3;
        unsigned int g3d_vol_step_number : 5;
        unsigned int reserved_3 : 7;
    } reg;
} SOC_PMCTRL_G3D_GDVFS_PROFILE0_UNION;
#endif
#define SOC_PMCTRL_G3D_GDVFS_PROFILE0_g3d_gdvfs_profile_updn_START (0)
#define SOC_PMCTRL_G3D_GDVFS_PROFILE0_g3d_gdvfs_profile_updn_END (0)
#define SOC_PMCTRL_G3D_GDVFS_PROFILE0_g3d_dvfs_mode_START (4)
#define SOC_PMCTRL_G3D_GDVFS_PROFILE0_g3d_dvfs_mode_END (5)
#define SOC_PMCTRL_G3D_GDVFS_PROFILE0_g3d_step_number_START (8)
#define SOC_PMCTRL_G3D_GDVFS_PROFILE0_g3d_step_number_END (15)
#define SOC_PMCTRL_G3D_GDVFS_PROFILE0_g3d_gdvfs_ctrl_apll_START (16)
#define SOC_PMCTRL_G3D_GDVFS_PROFILE0_g3d_gdvfs_ctrl_apll_END (16)
#define SOC_PMCTRL_G3D_GDVFS_PROFILE0_g3d_vol_step_number_START (20)
#define SOC_PMCTRL_G3D_GDVFS_PROFILE0_g3d_vol_step_number_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 2;
        unsigned int g3d_apll_cfg_time : 12;
        unsigned int g3d_clkdiv_time : 7;
        unsigned int reserved_2 : 9;
    } reg;
} SOC_PMCTRL_G3D_GDVFS_PROFILE1_UNION;
#endif
#define SOC_PMCTRL_G3D_GDVFS_PROFILE1_g3d_apll_cfg_time_START (4)
#define SOC_PMCTRL_G3D_GDVFS_PROFILE1_g3d_apll_cfg_time_END (15)
#define SOC_PMCTRL_G3D_GDVFS_PROFILE1_g3d_clkdiv_time_START (16)
#define SOC_PMCTRL_G3D_GDVFS_PROFILE1_g3d_clkdiv_time_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_apll_fbdiv_prof : 12;
        unsigned int reserved : 20;
    } reg;
} SOC_PMCTRL_G3D_GDVFS_PROFILE2_UNION;
#endif
#define SOC_PMCTRL_G3D_GDVFS_PROFILE2_g3d_apll_fbdiv_prof_START (0)
#define SOC_PMCTRL_G3D_GDVFS_PROFILE2_g3d_apll_fbdiv_prof_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_apll_frac_prof : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_G3D_GDVFS_PROFILE3_UNION;
#endif
#define SOC_PMCTRL_G3D_GDVFS_PROFILE3_g3d_apll_frac_prof_START (0)
#define SOC_PMCTRL_G3D_GDVFS_PROFILE3_g3d_apll_frac_prof_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_vol_idx_cfg : 8;
        unsigned int g3d_vol_chg_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int g3d_vol_idx_stat : 8;
        unsigned int g3d_dvfs_vol_idx_stat : 8;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_G3DVOLIDX_UNION;
#endif
#define SOC_PMCTRL_G3DVOLIDX_g3d_vol_idx_cfg_START (0)
#define SOC_PMCTRL_G3DVOLIDX_g3d_vol_idx_cfg_END (7)
#define SOC_PMCTRL_G3DVOLIDX_g3d_vol_chg_sftreq_START (8)
#define SOC_PMCTRL_G3DVOLIDX_g3d_vol_chg_sftreq_END (8)
#define SOC_PMCTRL_G3DVOLIDX_g3d_vol_idx_stat_START (12)
#define SOC_PMCTRL_G3DVOLIDX_g3d_vol_idx_stat_END (19)
#define SOC_PMCTRL_G3DVOLIDX_g3d_dvfs_vol_idx_stat_START (20)
#define SOC_PMCTRL_G3DVOLIDX_g3d_dvfs_vol_idx_stat_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_mem_vol_idx_cfg : 8;
        unsigned int g3d_mem_vol_chg_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int g3d_mem_vol_idx_stat : 8;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_G3DMEMVOLIDX_UNION;
#endif
#define SOC_PMCTRL_G3DMEMVOLIDX_g3d_mem_vol_idx_cfg_START (0)
#define SOC_PMCTRL_G3DMEMVOLIDX_g3d_mem_vol_idx_cfg_END (7)
#define SOC_PMCTRL_G3DMEMVOLIDX_g3d_mem_vol_chg_sftreq_START (8)
#define SOC_PMCTRL_G3DMEMVOLIDX_g3d_mem_vol_chg_sftreq_END (8)
#define SOC_PMCTRL_G3DMEMVOLIDX_g3d_mem_vol_idx_stat_START (12)
#define SOC_PMCTRL_G3DMEMVOLIDX_g3d_mem_vol_idx_stat_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_dvfs_master_fsm_cur_stat : 3;
        unsigned int reserved_0 : 1;
        unsigned int g3d_dvfs_freq_fsm_cur_stat : 4;
        unsigned int reserved_1 : 1;
        unsigned int g3d_dbtfsm_curr_stat : 3;
        unsigned int reserved_2 : 20;
    } reg;
} SOC_PMCTRL_G3DDBTSTAT_UNION;
#endif
#define SOC_PMCTRL_G3DDBTSTAT_g3d_dvfs_master_fsm_cur_stat_START (0)
#define SOC_PMCTRL_G3DDBTSTAT_g3d_dvfs_master_fsm_cur_stat_END (2)
#define SOC_PMCTRL_G3DDBTSTAT_g3d_dvfs_freq_fsm_cur_stat_START (4)
#define SOC_PMCTRL_G3DDBTSTAT_g3d_dvfs_freq_fsm_cur_stat_END (7)
#define SOC_PMCTRL_G3DDBTSTAT_g3d_dbtfsm_curr_stat_START (9)
#define SOC_PMCTRL_G3DDBTSTAT_g3d_dbtfsm_curr_stat_END (11)
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
        unsigned int freq_vote_a_type : 1;
        unsigned int freq_vote_b_type : 1;
        unsigned int freq_vote_c_type : 1;
        unsigned int freq_vote_d_type : 1;
        unsigned int freq_vote_e_type : 1;
        unsigned int freq_vote_f_type : 1;
        unsigned int freq_vote_g_type : 1;
        unsigned int freq_vote_h_type : 1;
        unsigned int freq_vote_i_type : 1;
        unsigned int reserved_0 : 7;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_PMCTRL_VOTE_MODE_CFG_UNION;
#endif
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_a_type_START (0)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_a_type_END (0)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_b_type_START (1)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_b_type_END (1)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_c_type_START (2)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_c_type_END (2)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_d_type_START (3)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_d_type_END (3)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_e_type_START (4)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_e_type_END (4)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_f_type_START (5)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_f_type_END (5)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_g_type_START (6)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_g_type_END (6)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_h_type_START (7)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_h_type_END (7)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_i_type_START (8)
#define SOC_PMCTRL_VOTE_MODE_CFG_freq_vote_i_type_END (8)
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
        unsigned int freq_vote_result_a : 16;
        unsigned int freq_vote_result_b : 16;
    } reg;
} SOC_PMCTRL_VOTE_RESULT0_UNION;
#endif
#define SOC_PMCTRL_VOTE_RESULT0_freq_vote_result_a_START (0)
#define SOC_PMCTRL_VOTE_RESULT0_freq_vote_result_a_END (15)
#define SOC_PMCTRL_VOTE_RESULT0_freq_vote_result_b_START (16)
#define SOC_PMCTRL_VOTE_RESULT0_freq_vote_result_b_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_result_c : 16;
        unsigned int freq_vote_result_d : 16;
    } reg;
} SOC_PMCTRL_VOTE_RESULT1_UNION;
#endif
#define SOC_PMCTRL_VOTE_RESULT1_freq_vote_result_c_START (0)
#define SOC_PMCTRL_VOTE_RESULT1_freq_vote_result_c_END (15)
#define SOC_PMCTRL_VOTE_RESULT1_freq_vote_result_d_START (16)
#define SOC_PMCTRL_VOTE_RESULT1_freq_vote_result_d_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_result_e : 16;
        unsigned int freq_vote_result_f : 16;
    } reg;
} SOC_PMCTRL_VOTE_RESULT2_UNION;
#endif
#define SOC_PMCTRL_VOTE_RESULT2_freq_vote_result_e_START (0)
#define SOC_PMCTRL_VOTE_RESULT2_freq_vote_result_e_END (15)
#define SOC_PMCTRL_VOTE_RESULT2_freq_vote_result_f_START (16)
#define SOC_PMCTRL_VOTE_RESULT2_freq_vote_result_f_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_result_g : 16;
        unsigned int freq_vote_result_h : 16;
    } reg;
} SOC_PMCTRL_VOTE_RESULT3_UNION;
#endif
#define SOC_PMCTRL_VOTE_RESULT3_freq_vote_result_g_START (0)
#define SOC_PMCTRL_VOTE_RESULT3_freq_vote_result_g_END (15)
#define SOC_PMCTRL_VOTE_RESULT3_freq_vote_result_h_START (16)
#define SOC_PMCTRL_VOTE_RESULT3_freq_vote_result_h_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_vote_result_i : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_VOTE_RESULT4_UNION;
#endif
#define SOC_PMCTRL_VOTE_RESULT4_freq_vote_result_i_START (0)
#define SOC_PMCTRL_VOTE_RESULT4_freq_vote_result_i_END (15)
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
        unsigned int pll_lock_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_PLLLOCKTIME_UNION;
#endif
#define SOC_PMCTRL_PLLLOCKTIME_pll_lock_time_START (0)
#define SOC_PMCTRL_PLLLOCKTIME_pll_lock_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pll_lock_mod : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_PLLLOCKMOD_UNION;
#endif
#define SOC_PMCTRL_PLLLOCKMOD_pll_lock_mod_START (0)
#define SOC_PMCTRL_PLLLOCKMOD_pll_lock_mod_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int avs_en_ssi : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_PMUSSIAVSEN_UNION;
#endif
#define SOC_PMCTRL_PMUSSIAVSEN_avs_en_ssi_START (0)
#define SOC_PMCTRL_PMUSSIAVSEN_avs_en_ssi_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_ctrl_core_crg : 1;
        unsigned int dbg_ctrl_cssys : 1;
        unsigned int dbg_ctrl_ssi : 1;
        unsigned int dbg_ctrl_i2c0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int dbg_ctrl_pmc : 1;
        unsigned int dbg_ctrl_crg : 1;
        unsigned int reserved_2 : 24;
    } reg;
} SOC_PMCTRL_PERI_CTRL0_UNION;
#endif
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_core_crg_START (0)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_core_crg_END (0)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_cssys_START (1)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_cssys_END (1)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_ssi_START (2)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_ssi_END (2)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_i2c0_START (3)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_i2c0_END (3)
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
        unsigned int bigfast0_vol_addr : 10;
        unsigned int reserved_1 : 6;
    } reg;
} SOC_PMCTRL_PERI_CTRL3_UNION;
#endif
#define SOC_PMCTRL_PERI_CTRL3_g3d_mem_vol_addr_START (0)
#define SOC_PMCTRL_PERI_CTRL3_g3d_mem_vol_addr_END (9)
#define SOC_PMCTRL_PERI_CTRL3_bigfast0_vol_addr_START (16)
#define SOC_PMCTRL_PERI_CTRL3_bigfast0_vol_addr_END (25)
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
        unsigned int noc_lpm3_mst_paddr : 2;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 3;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 24;
    } reg;
} SOC_PMCTRL_PERI_CTRL6_UNION;
#endif
#define SOC_PMCTRL_PERI_CTRL6_noc_lpm3_mst_paddr_START (0)
#define SOC_PMCTRL_PERI_CTRL6_noc_lpm3_mst_paddr_END (1)
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
        unsigned int dbg_info_ddrc : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT0_UNION;
#endif
#define SOC_PMCTRL_PERI_STAT0_dbg_info_ddrc_START (0)
#define SOC_PMCTRL_PERI_STAT0_dbg_info_ddrc_END (31)
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
        unsigned int dbg_info_ssi : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT2_UNION;
#endif
#define SOC_PMCTRL_PERI_STAT2_dbg_info_ssi_START (0)
#define SOC_PMCTRL_PERI_STAT2_dbg_info_ssi_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_info_i2c0 : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT3_UNION;
#endif
#define SOC_PMCTRL_PERI_STAT3_dbg_info_i2c0_START (0)
#define SOC_PMCTRL_PERI_STAT3_dbg_info_i2c0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_info_i2c1 : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT4_UNION;
#endif
#define SOC_PMCTRL_PERI_STAT4_dbg_info_i2c1_START (0)
#define SOC_PMCTRL_PERI_STAT4_dbg_info_i2c1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 4;
        unsigned int reserved_1: 4;
        unsigned int reserved_2: 4;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 17;
    } reg;
} SOC_PMCTRL_PERI_STAT5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 16;
        unsigned int reserved_1: 16;
    } reg;
} SOC_PMCTRL_PERI_STAT6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 9;
        unsigned int reserved_8: 16;
    } reg;
} SOC_PMCTRL_PERI_STAT7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_ddrphy_bypass_mode : 1;
        unsigned int sc_noc_mmc0bus_power_idlereq : 1;
        unsigned int sc_noc_mmc1bus_power_idlereq : 1;
        unsigned int sc_noc_modem_power_idlereq : 1;
        unsigned int sc_noc_vcodec_bus_power_idlereq : 1;
        unsigned int sc_noc_isp_power_idlereq : 1;
        unsigned int sc_noc_sysbus_power_idlereq : 1;
        unsigned int sc_noc_cfgbus_power_idlereq : 1;
        unsigned int sc_noc_dmabus_power_idlereq : 1;
        unsigned int sc_noc_fcm_power_idlereq : 1;
        unsigned int sc_noc_vdec_power_idlereq : 1;
        unsigned int sc_noc_venc_power_idlereq : 1;
        unsigned int noc_npubus_power_IdleReq : 1;
        unsigned int sc_noc_dss_power_idlereq : 1;
        unsigned int sc_noc_ivp32_peri_bus_power_idlereq : 1;
        unsigned int sc_noc_vivobus_power_idlereq : 1;
        unsigned int biten : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEREQ_0_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_ddrphy_bypass_mode_START (0)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_ddrphy_bypass_mode_END (0)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_mmc0bus_power_idlereq_START (1)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_mmc0bus_power_idlereq_END (1)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_mmc1bus_power_idlereq_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_mmc1bus_power_idlereq_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_modem_power_idlereq_START (3)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_modem_power_idlereq_END (3)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_vcodec_bus_power_idlereq_START (4)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_vcodec_bus_power_idlereq_END (4)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_isp_power_idlereq_START (5)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_isp_power_idlereq_END (5)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_sysbus_power_idlereq_START (6)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_sysbus_power_idlereq_END (6)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_cfgbus_power_idlereq_START (7)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_cfgbus_power_idlereq_END (7)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_dmabus_power_idlereq_START (8)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_dmabus_power_idlereq_END (8)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_fcm_power_idlereq_START (9)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_fcm_power_idlereq_END (9)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_vdec_power_idlereq_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_vdec_power_idlereq_END (10)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_venc_power_idlereq_START (11)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_venc_power_idlereq_END (11)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_npubus_power_IdleReq_START (12)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_npubus_power_IdleReq_END (12)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_dss_power_idlereq_START (13)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_dss_power_idlereq_END (13)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_ivp32_peri_bus_power_idlereq_START (14)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_ivp32_peri_bus_power_idlereq_END (14)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_vivobus_power_idlereq_START (15)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_sc_noc_vivobus_power_idlereq_END (15)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_biten_START (16)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_biten_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int sc_noc_mmc0bus_power_idleack : 1;
        unsigned int sc_noc_mmc1bus_power_idleack : 1;
        unsigned int sc_noc_modem_power_idleack : 1;
        unsigned int sc_noc_vcodec_bus_power_idleack : 1;
        unsigned int sc_noc_isp_power_idleack : 1;
        unsigned int sc_noc_sysbus_power_idleack : 1;
        unsigned int sc_noc_cfgbus_power_idleack : 1;
        unsigned int sc_noc_dmabus_power_idleack : 1;
        unsigned int sc_noc_fcm_power_idleack : 1;
        unsigned int sc_noc_vdec_power_idleack : 1;
        unsigned int sc_noc_venc_power_idleack : 1;
        unsigned int noc_npubus_power_IdleAck : 1;
        unsigned int sc_noc_dss_power_idleack : 1;
        unsigned int sc_noc_ivp32_power_idleack : 1;
        unsigned int sc_noc_vivobus_power_idleack : 1;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEACK_0_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_mmc0bus_power_idleack_START (1)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_mmc0bus_power_idleack_END (1)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_mmc1bus_power_idleack_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_mmc1bus_power_idleack_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_modem_power_idleack_START (3)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_modem_power_idleack_END (3)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_vcodec_bus_power_idleack_START (4)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_vcodec_bus_power_idleack_END (4)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_isp_power_idleack_START (5)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_isp_power_idleack_END (5)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_sysbus_power_idleack_START (6)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_sysbus_power_idleack_END (6)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_cfgbus_power_idleack_START (7)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_cfgbus_power_idleack_END (7)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_dmabus_power_idleack_START (8)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_dmabus_power_idleack_END (8)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_fcm_power_idleack_START (9)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_fcm_power_idleack_END (9)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_vdec_power_idleack_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_vdec_power_idleack_END (10)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_venc_power_idleack_START (11)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_venc_power_idleack_END (11)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_npubus_power_IdleAck_START (12)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_npubus_power_IdleAck_END (12)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_dss_power_idleack_START (13)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_dss_power_idleack_END (13)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_ivp32_power_idleack_START (14)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_ivp32_power_idleack_END (14)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_vivobus_power_idleack_START (15)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_sc_noc_vivobus_power_idleack_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int sc_noc_mmc0bus_power_idle : 1;
        unsigned int sc_noc_mmc1bus_power_idle : 1;
        unsigned int sc_noc_modem_power_idle : 1;
        unsigned int sc_noc_vcodec_bus_power_idle : 1;
        unsigned int sc_noc_isp_power_idle : 1;
        unsigned int sc_noc_sysbus_power_idle : 1;
        unsigned int sc_noc_cfgbus_power_idle : 1;
        unsigned int sc_noc_dmabus_power_idle : 1;
        unsigned int sc_noc_fcm_power_idle : 1;
        unsigned int sc_noc_vdec_power_idle : 1;
        unsigned int sc_noc_venc_power_idle : 1;
        unsigned int noc_npubus_power_Idle : 1;
        unsigned int sc_noc_dss_power_idle : 1;
        unsigned int sc_noc_ivp32_power_idle : 1;
        unsigned int sc_noc_vivobus_power_idle : 1;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLE_0_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_mmc0bus_power_idle_START (1)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_mmc0bus_power_idle_END (1)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_mmc1bus_power_idle_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_mmc1bus_power_idle_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_modem_power_idle_START (3)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_modem_power_idle_END (3)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_vcodec_bus_power_idle_START (4)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_vcodec_bus_power_idle_END (4)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_isp_power_idle_START (5)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_isp_power_idle_END (5)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_sysbus_power_idle_START (6)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_sysbus_power_idle_END (6)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_cfgbus_power_idle_START (7)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_cfgbus_power_idle_END (7)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_dmabus_power_idle_START (8)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_dmabus_power_idle_END (8)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_fcm_power_idle_START (9)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_fcm_power_idle_END (9)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_vdec_power_idle_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_vdec_power_idle_END (10)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_venc_power_idle_START (11)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_venc_power_idle_END (11)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_npubus_power_Idle_START (12)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_npubus_power_Idle_END (12)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_dss_power_idle_START (13)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_dss_power_idle_END (13)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_ivp32_power_idle_START (14)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_ivp32_power_idle_END (14)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_vivobus_power_idle_START (15)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_sc_noc_vivobus_power_idle_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int sc_noc_hiseebus_power_idlereq : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int noc_npu_cpu_power_IdleReq : 1;
        unsigned int noc_smmu_tcu_power_IdleReq : 1;
        unsigned int noc_sysdma_power_IdleReq : 1;
        unsigned int noc_aicore_power_IdleReq : 1;
        unsigned int biten : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEREQ_1_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_sc_noc_hiseebus_power_idlereq_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_sc_noc_hiseebus_power_idlereq_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_npu_cpu_power_IdleReq_START (12)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_npu_cpu_power_IdleReq_END (12)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_smmu_tcu_power_IdleReq_START (13)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_smmu_tcu_power_IdleReq_END (13)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_sysdma_power_IdleReq_START (14)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_sysdma_power_IdleReq_END (14)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_aicore_power_IdleReq_START (15)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_aicore_power_IdleReq_END (15)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_biten_START (16)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_biten_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int sc_noc_hiseebus_power_idleack : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int noc_npu_cpu_power_IdleAck : 1;
        unsigned int noc_smmu_power_IdleAck : 1;
        unsigned int noc_sysdma_power_IdleAck : 1;
        unsigned int noc_aicore_power_IdleAck : 1;
        unsigned int reserved_11 : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEACK_1_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_sc_noc_hiseebus_power_idleack_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_sc_noc_hiseebus_power_idleack_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_npu_cpu_power_IdleAck_START (12)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_npu_cpu_power_IdleAck_END (12)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_smmu_power_IdleAck_START (13)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_smmu_power_IdleAck_END (13)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_sysdma_power_IdleAck_START (14)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_sysdma_power_IdleAck_END (14)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_aicore_power_IdleAck_START (15)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_aicore_power_IdleAck_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int sc_noc_hiseebus_power_idle : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int noc_npu_cpu_power_Idle : 1;
        unsigned int noc_smmu_tcu_power_Idle : 1;
        unsigned int noc_sysdma_power_Idle : 1;
        unsigned int noc_aicore_power_Idle : 1;
        unsigned int reserved_11 : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLE_1_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLE_1_sc_noc_hiseebus_power_idle_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_sc_noc_hiseebus_power_idle_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_npu_cpu_power_Idle_START (12)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_npu_cpu_power_Idle_END (12)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_smmu_tcu_power_Idle_START (13)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_smmu_tcu_power_Idle_END (13)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_sysdma_power_Idle_START (14)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_sysdma_power_Idle_END (14)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_aicore_power_Idle_START (15)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_aicore_power_Idle_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 1;
        unsigned int reserved_9: 1;
        unsigned int reserved_10: 1;
        unsigned int reserved_11: 1;
        unsigned int reserved_12: 1;
        unsigned int reserved_13: 1;
        unsigned int reserved_14: 1;
        unsigned int reserved_15: 1;
        unsigned int biten : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEREQ_2_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_2_biten_START (16)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_2_biten_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 1;
        unsigned int reserved_9: 1;
        unsigned int reserved_10: 1;
        unsigned int reserved_11: 1;
        unsigned int reserved_12: 1;
        unsigned int reserved_13: 1;
        unsigned int reserved_14: 1;
        unsigned int reserved_15: 1;
        unsigned int reserved_16: 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEACK_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 1;
        unsigned int reserved_9: 1;
        unsigned int reserved_10: 1;
        unsigned int reserved_11: 1;
        unsigned int reserved_12: 1;
        unsigned int reserved_13: 1;
        unsigned int reserved_14: 1;
        unsigned int reserved_15: 1;
        unsigned int reserved_16: 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLE_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_noc_cfg2npu_t_maintimeout : 1;
        unsigned int sc_noc_hkadc_ssi_t_maintimeout : 1;
        unsigned int sc_noc_gpu_cfg_t_maintimeout : 1;
        unsigned int sc_noc_lpm3_slv_t_maintimeout : 1;
        unsigned int sc_noc_sys_peri0_cfg_t_maintimeout : 1;
        unsigned int sc_noc_sys_peri1_cfg_t_maintimeout : 1;
        unsigned int sc_noc_sys_peri2_cfg_t_maintimeout : 1;
        unsigned int sc_noc_sys_peri3_cfg_t_maintimeout : 1;
        unsigned int sc_noc_emmc_cfg_t_maintimeout : 1;
        unsigned int sc_noc_dmac_cfg_t_maintimeout : 1;
        unsigned int sc_noc_vcodec2_cfg_t_maintimeout : 1;
        unsigned int sc_noc_hisee_cfg_t_maintimeout : 1;
        unsigned int sc_noc_socp_cfg_t_maintimeout : 1;
        unsigned int sc_noc_top_cssys_slv_t_maintimeout : 1;
        unsigned int reserved_0 : 1;
        unsigned int sc_modem_bus_cfg_t_maintimeout : 1;
        unsigned int sc_noc_usb2_cfg_t_maintimeout : 1;
        unsigned int reserved_1 : 1;
        unsigned int sc_noc_emmc0bus_cfg_t_maintimeout : 1;
        unsigned int sc_noc_vcodec_cfg_t_maintimeout : 1;
        unsigned int sc_noc_sys2hkmem_t_maintimeout : 1;
        unsigned int sc_noc_mmc1bus_apb_cfg_t_maintimeout : 1;
        unsigned int reserved_2 : 1;
        unsigned int sc_noc_cfg2media_t_maintimeout : 1;
        unsigned int sc_noc_ufs_cfg_t_maintimeout : 1;
        unsigned int noc_dmss_apb_slv_t_maintimeout : 1;
        unsigned int reserved_3 : 1;
        unsigned int sc_noc_asp_cfg_t_maintimeout : 1;
        unsigned int reserved_4 : 1;
        unsigned int iomcu_ahb_slv_maintimeout : 1;
        unsigned int iomcu_apb_slv_maintimeout : 1;
        unsigned int noc_aon_spb_slv_t_maintimeout : 1;
    } reg;
} SOC_PMCTRL_PERI_INT0_MASK_UNION;
#endif
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_cfg2npu_t_maintimeout_START (0)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_cfg2npu_t_maintimeout_END (0)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_hkadc_ssi_t_maintimeout_START (1)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_hkadc_ssi_t_maintimeout_END (1)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_gpu_cfg_t_maintimeout_START (2)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_gpu_cfg_t_maintimeout_END (2)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_lpm3_slv_t_maintimeout_START (3)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_lpm3_slv_t_maintimeout_END (3)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_sys_peri0_cfg_t_maintimeout_START (4)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_sys_peri0_cfg_t_maintimeout_END (4)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_sys_peri1_cfg_t_maintimeout_START (5)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_sys_peri1_cfg_t_maintimeout_END (5)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_sys_peri2_cfg_t_maintimeout_START (6)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_sys_peri2_cfg_t_maintimeout_END (6)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_sys_peri3_cfg_t_maintimeout_START (7)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_sys_peri3_cfg_t_maintimeout_END (7)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_emmc_cfg_t_maintimeout_START (8)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_emmc_cfg_t_maintimeout_END (8)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_dmac_cfg_t_maintimeout_START (9)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_dmac_cfg_t_maintimeout_END (9)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_vcodec2_cfg_t_maintimeout_START (10)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_vcodec2_cfg_t_maintimeout_END (10)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_hisee_cfg_t_maintimeout_START (11)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_hisee_cfg_t_maintimeout_END (11)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_socp_cfg_t_maintimeout_START (12)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_socp_cfg_t_maintimeout_END (12)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_top_cssys_slv_t_maintimeout_START (13)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_top_cssys_slv_t_maintimeout_END (13)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_modem_bus_cfg_t_maintimeout_START (15)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_modem_bus_cfg_t_maintimeout_END (15)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_usb2_cfg_t_maintimeout_START (16)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_usb2_cfg_t_maintimeout_END (16)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_emmc0bus_cfg_t_maintimeout_START (18)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_emmc0bus_cfg_t_maintimeout_END (18)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_vcodec_cfg_t_maintimeout_START (19)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_vcodec_cfg_t_maintimeout_END (19)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_sys2hkmem_t_maintimeout_START (20)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_sys2hkmem_t_maintimeout_END (20)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_mmc1bus_apb_cfg_t_maintimeout_START (21)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_mmc1bus_apb_cfg_t_maintimeout_END (21)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_cfg2media_t_maintimeout_START (23)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_cfg2media_t_maintimeout_END (23)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_ufs_cfg_t_maintimeout_START (24)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_ufs_cfg_t_maintimeout_END (24)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_dmss_apb_slv_t_maintimeout_START (25)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_dmss_apb_slv_t_maintimeout_END (25)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_asp_cfg_t_maintimeout_START (27)
#define SOC_PMCTRL_PERI_INT0_MASK_sc_noc_asp_cfg_t_maintimeout_END (27)
#define SOC_PMCTRL_PERI_INT0_MASK_iomcu_ahb_slv_maintimeout_START (29)
#define SOC_PMCTRL_PERI_INT0_MASK_iomcu_ahb_slv_maintimeout_END (29)
#define SOC_PMCTRL_PERI_INT0_MASK_iomcu_apb_slv_maintimeout_START (30)
#define SOC_PMCTRL_PERI_INT0_MASK_iomcu_apb_slv_maintimeout_END (30)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_aon_spb_slv_t_maintimeout_START (31)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_aon_spb_slv_t_maintimeout_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_noc_cfg2npu_t_maintimeout : 1;
        unsigned int sc_noc_hkadc_ssi_t_maintimeout : 1;
        unsigned int sc_noc_gpu_cfg_t_maintimeout : 1;
        unsigned int sc_noc_lpm3_slv_t_maintimeout : 1;
        unsigned int sc_noc_sys_peri0_cfg_t_maintimeout : 1;
        unsigned int sc_noc_sys_peri1_cfg_t_maintimeout : 1;
        unsigned int sc_noc_sys_peri2_cfg_t_maintimeout : 1;
        unsigned int sc_noc_sys_peri3_cfg_t_maintimeout : 1;
        unsigned int sc_noc_emmc_cfg_t_maintimeout : 1;
        unsigned int sc_noc_dmac_cfg_t_maintimeout : 1;
        unsigned int sc_noc_vcodec2_cfg_t_maintimeout : 1;
        unsigned int sc_noc_hisee_cfg_t_maintimeout : 1;
        unsigned int sc_noc_socp_cfg_t_maintimeout : 1;
        unsigned int sc_noc_top_cssys_slv_t_maintimeout : 1;
        unsigned int reserved_0 : 1;
        unsigned int sc_modem_bus_cfg_t_maintimeout : 1;
        unsigned int sc_noc_usb2_cfg_t_maintimeout : 1;
        unsigned int reserved_1 : 1;
        unsigned int sc_noc_emmc0bus_cfg_t_maintimeout : 1;
        unsigned int sc_noc_vcodec_cfg_t_maintimeout : 1;
        unsigned int sc_noc_sys2hkmem_t_maintimeout : 1;
        unsigned int sc_noc_mmc1bus_apb_cfg_t_maintimeout : 1;
        unsigned int reserved_2 : 1;
        unsigned int sc_noc_cfg2media_t_maintimeout : 1;
        unsigned int sc_noc_ufs_cfg_t_maintimeout : 1;
        unsigned int noc_dmss_apb_slv_t_maintimeout : 1;
        unsigned int reserved_3 : 1;
        unsigned int sc_noc_asp_cfg_t_maintimeout : 1;
        unsigned int reserved_4 : 1;
        unsigned int iomcu_ahb_slv_maintimeout : 1;
        unsigned int iomcu_apb_slv_maintimeout : 1;
        unsigned int noc_aon_spb_slv_t_maintimeout : 1;
    } reg;
} SOC_PMCTRL_PERI_INT0_STAT_UNION;
#endif
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_cfg2npu_t_maintimeout_START (0)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_cfg2npu_t_maintimeout_END (0)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_hkadc_ssi_t_maintimeout_START (1)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_hkadc_ssi_t_maintimeout_END (1)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_gpu_cfg_t_maintimeout_START (2)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_gpu_cfg_t_maintimeout_END (2)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_lpm3_slv_t_maintimeout_START (3)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_lpm3_slv_t_maintimeout_END (3)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_sys_peri0_cfg_t_maintimeout_START (4)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_sys_peri0_cfg_t_maintimeout_END (4)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_sys_peri1_cfg_t_maintimeout_START (5)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_sys_peri1_cfg_t_maintimeout_END (5)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_sys_peri2_cfg_t_maintimeout_START (6)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_sys_peri2_cfg_t_maintimeout_END (6)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_sys_peri3_cfg_t_maintimeout_START (7)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_sys_peri3_cfg_t_maintimeout_END (7)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_emmc_cfg_t_maintimeout_START (8)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_emmc_cfg_t_maintimeout_END (8)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_dmac_cfg_t_maintimeout_START (9)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_dmac_cfg_t_maintimeout_END (9)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_vcodec2_cfg_t_maintimeout_START (10)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_vcodec2_cfg_t_maintimeout_END (10)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_hisee_cfg_t_maintimeout_START (11)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_hisee_cfg_t_maintimeout_END (11)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_socp_cfg_t_maintimeout_START (12)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_socp_cfg_t_maintimeout_END (12)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_top_cssys_slv_t_maintimeout_START (13)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_top_cssys_slv_t_maintimeout_END (13)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_modem_bus_cfg_t_maintimeout_START (15)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_modem_bus_cfg_t_maintimeout_END (15)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_usb2_cfg_t_maintimeout_START (16)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_usb2_cfg_t_maintimeout_END (16)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_emmc0bus_cfg_t_maintimeout_START (18)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_emmc0bus_cfg_t_maintimeout_END (18)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_vcodec_cfg_t_maintimeout_START (19)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_vcodec_cfg_t_maintimeout_END (19)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_sys2hkmem_t_maintimeout_START (20)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_sys2hkmem_t_maintimeout_END (20)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_mmc1bus_apb_cfg_t_maintimeout_START (21)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_mmc1bus_apb_cfg_t_maintimeout_END (21)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_cfg2media_t_maintimeout_START (23)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_cfg2media_t_maintimeout_END (23)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_ufs_cfg_t_maintimeout_START (24)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_ufs_cfg_t_maintimeout_END (24)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_dmss_apb_slv_t_maintimeout_START (25)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_dmss_apb_slv_t_maintimeout_END (25)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_asp_cfg_t_maintimeout_START (27)
#define SOC_PMCTRL_PERI_INT0_STAT_sc_noc_asp_cfg_t_maintimeout_END (27)
#define SOC_PMCTRL_PERI_INT0_STAT_iomcu_ahb_slv_maintimeout_START (29)
#define SOC_PMCTRL_PERI_INT0_STAT_iomcu_ahb_slv_maintimeout_END (29)
#define SOC_PMCTRL_PERI_INT0_STAT_iomcu_apb_slv_maintimeout_START (30)
#define SOC_PMCTRL_PERI_INT0_STAT_iomcu_apb_slv_maintimeout_END (30)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_aon_spb_slv_t_maintimeout_START (31)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_aon_spb_slv_t_maintimeout_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_dss_cfg_t_maintimeout : 1;
        unsigned int noc_isp_cfg_t_maintimeout : 1;
        unsigned int noc_jpeg_fd_t_maintimeout : 1;
        unsigned int noc_media2cfg_cfg_t_maintimeout : 1;
        unsigned int noc_media_crg_cfg_t_maintimeout : 1;
        unsigned int noc_ivp32_cfg_t_maintimeout : 1;
        unsigned int noc_dmca_t_maintimeout : 1;
        unsigned int noc_dmcb_t_maintimeout : 1;
        unsigned int noc_fcm2gic_cfg_t_maintimeout : 1;
        unsigned int noc_fcm2sys_service_cfg_t_maintimeout : 1;
        unsigned int noc_aicore_cfg_T_mainTimeOut : 1;
        unsigned int noc_npu_cpu_cfg_T_mainTimeOut : 1;
        unsigned int noc_npu2cfgbus_T_mainTimeOut : 1;
        unsigned int noc_npubus_cfg_T_mainTimeOut : 1;
        unsigned int noc_npugic_cfg_T_mainTimeOut : 1;
        unsigned int noc_l2buffer_slv0_rd_T_mainTimeOut : 1;
        unsigned int noc_l2buffer_slv0_wr_T_mainTimeOut : 1;
        unsigned int noc_l2buffer_slv1_rd_T_mainTimeOut : 1;
        unsigned int noc_l2buffer_slv1_wr_T_mainTimeOut : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_fcm2sys_cfg_T_mainTimeOut : 1;
        unsigned int noc_vcodec_crg_cfg_T_mainTimeout : 1;
        unsigned int noc_vdec_cfg_T_mainTimeout : 1;
        unsigned int noc_venc_cfg_T_mainTimeout : 1;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_PMCTRL_PERI_INT1_MASK_UNION;
#endif
#define SOC_PMCTRL_PERI_INT1_MASK_noc_dss_cfg_t_maintimeout_START (0)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_dss_cfg_t_maintimeout_END (0)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_isp_cfg_t_maintimeout_START (1)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_isp_cfg_t_maintimeout_END (1)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_jpeg_fd_t_maintimeout_START (2)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_jpeg_fd_t_maintimeout_END (2)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_media2cfg_cfg_t_maintimeout_START (3)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_media2cfg_cfg_t_maintimeout_END (3)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_media_crg_cfg_t_maintimeout_START (4)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_media_crg_cfg_t_maintimeout_END (4)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_ivp32_cfg_t_maintimeout_START (5)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_ivp32_cfg_t_maintimeout_END (5)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_dmca_t_maintimeout_START (6)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_dmca_t_maintimeout_END (6)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_dmcb_t_maintimeout_START (7)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_dmcb_t_maintimeout_END (7)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_fcm2gic_cfg_t_maintimeout_START (8)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_fcm2gic_cfg_t_maintimeout_END (8)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_fcm2sys_service_cfg_t_maintimeout_START (9)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_fcm2sys_service_cfg_t_maintimeout_END (9)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_aicore_cfg_T_mainTimeOut_START (10)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_aicore_cfg_T_mainTimeOut_END (10)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_npu_cpu_cfg_T_mainTimeOut_START (11)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_npu_cpu_cfg_T_mainTimeOut_END (11)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_npu2cfgbus_T_mainTimeOut_START (12)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_npu2cfgbus_T_mainTimeOut_END (12)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_npubus_cfg_T_mainTimeOut_START (13)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_npubus_cfg_T_mainTimeOut_END (13)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_npugic_cfg_T_mainTimeOut_START (14)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_npugic_cfg_T_mainTimeOut_END (14)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_l2buffer_slv0_rd_T_mainTimeOut_START (15)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_l2buffer_slv0_rd_T_mainTimeOut_END (15)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_l2buffer_slv0_wr_T_mainTimeOut_START (16)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_l2buffer_slv0_wr_T_mainTimeOut_END (16)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_l2buffer_slv1_rd_T_mainTimeOut_START (17)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_l2buffer_slv1_rd_T_mainTimeOut_END (17)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_l2buffer_slv1_wr_T_mainTimeOut_START (18)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_l2buffer_slv1_wr_T_mainTimeOut_END (18)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_fcm2sys_cfg_T_mainTimeOut_START (20)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_fcm2sys_cfg_T_mainTimeOut_END (20)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_vcodec_crg_cfg_T_mainTimeout_START (21)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_vcodec_crg_cfg_T_mainTimeout_END (21)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_vdec_cfg_T_mainTimeout_START (22)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_vdec_cfg_T_mainTimeout_END (22)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_venc_cfg_T_mainTimeout_START (23)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_venc_cfg_T_mainTimeout_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_dss_cfg_t_maintimeout : 1;
        unsigned int noc_isp_cfg_t_maintimeout : 1;
        unsigned int noc_jpeg_fd_t_maintimeout : 1;
        unsigned int noc_media2cfg_cfg_t_maintimeout : 1;
        unsigned int noc_media_crg_cfg_t_maintimeout : 1;
        unsigned int noc_ivp32_cfg_t_maintimeout : 1;
        unsigned int noc_dmca_t_maintimeout : 1;
        unsigned int noc_dmcb_t_maintimeout : 1;
        unsigned int noc_fcm2gic_cfg_t_maintimeout : 1;
        unsigned int noc_fcm2sys_service_cfg_t_maintimeout : 1;
        unsigned int noc_aicore_cfg_T_mainTimeOut : 1;
        unsigned int noc_npu_cpu_cfg_T_mainTimeOut : 1;
        unsigned int noc_npu2cfgbus_T_mainTimeOut : 1;
        unsigned int noc_npubus_cfg_T_mainTimeOut : 1;
        unsigned int noc_npugic_cfg_T_mainTimeOut : 1;
        unsigned int noc_l2buffer_slv0_rd_T_mainTimeOut : 1;
        unsigned int noc_l2buffer_slv0_wr_T_mainTimeOut : 1;
        unsigned int noc_l2buffer_slv1_rd_T_mainTimeOut : 1;
        unsigned int noc_l2buffer_slv1_wr_T_mainTimeOut : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_fcm2sys_cfg_T_mainTimeOut : 1;
        unsigned int noc_vcodec_crg_cfg_T_mainTimeout : 1;
        unsigned int noc_vdec_cfg_T_mainTimeout : 1;
        unsigned int noc_venc_cfg_T_mainTimeout : 1;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_PMCTRL_PERI_INT1_STAT_UNION;
#endif
#define SOC_PMCTRL_PERI_INT1_STAT_noc_dss_cfg_t_maintimeout_START (0)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_dss_cfg_t_maintimeout_END (0)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_isp_cfg_t_maintimeout_START (1)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_isp_cfg_t_maintimeout_END (1)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_jpeg_fd_t_maintimeout_START (2)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_jpeg_fd_t_maintimeout_END (2)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_media2cfg_cfg_t_maintimeout_START (3)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_media2cfg_cfg_t_maintimeout_END (3)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_media_crg_cfg_t_maintimeout_START (4)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_media_crg_cfg_t_maintimeout_END (4)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_ivp32_cfg_t_maintimeout_START (5)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_ivp32_cfg_t_maintimeout_END (5)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_dmca_t_maintimeout_START (6)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_dmca_t_maintimeout_END (6)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_dmcb_t_maintimeout_START (7)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_dmcb_t_maintimeout_END (7)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_fcm2gic_cfg_t_maintimeout_START (8)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_fcm2gic_cfg_t_maintimeout_END (8)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_fcm2sys_service_cfg_t_maintimeout_START (9)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_fcm2sys_service_cfg_t_maintimeout_END (9)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_aicore_cfg_T_mainTimeOut_START (10)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_aicore_cfg_T_mainTimeOut_END (10)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_npu_cpu_cfg_T_mainTimeOut_START (11)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_npu_cpu_cfg_T_mainTimeOut_END (11)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_npu2cfgbus_T_mainTimeOut_START (12)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_npu2cfgbus_T_mainTimeOut_END (12)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_npubus_cfg_T_mainTimeOut_START (13)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_npubus_cfg_T_mainTimeOut_END (13)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_npugic_cfg_T_mainTimeOut_START (14)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_npugic_cfg_T_mainTimeOut_END (14)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_l2buffer_slv0_rd_T_mainTimeOut_START (15)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_l2buffer_slv0_rd_T_mainTimeOut_END (15)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_l2buffer_slv0_wr_T_mainTimeOut_START (16)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_l2buffer_slv0_wr_T_mainTimeOut_END (16)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_l2buffer_slv1_rd_T_mainTimeOut_START (17)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_l2buffer_slv1_rd_T_mainTimeOut_END (17)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_l2buffer_slv1_wr_T_mainTimeOut_START (18)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_l2buffer_slv1_wr_T_mainTimeOut_END (18)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_fcm2sys_cfg_T_mainTimeOut_START (20)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_fcm2sys_cfg_T_mainTimeOut_END (20)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_vcodec_crg_cfg_T_mainTimeout_START (21)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_vcodec_crg_cfg_T_mainTimeout_END (21)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_vdec_cfg_T_mainTimeout_START (22)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_vdec_cfg_T_mainTimeout_END (22)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_venc_cfg_T_mainTimeout_START (23)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_venc_cfg_T_mainTimeout_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_INT2_MASK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_INT2_STAT_UNION;
#endif
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
        unsigned int vol_drop_en_gpu : 1;
        unsigned int gpu_cpu0_wfi_wfe_bypass : 1;
        unsigned int gpu_cpu1_wfi_wfe_bypass : 1;
        unsigned int gpu_cpu2_wfi_wfe_bypass : 1;
        unsigned int gpu_cpu3_wfi_wfe_bypass : 1;
        unsigned int gpu_l2_idle_div_mod : 2;
        unsigned int gpu_cfg_cnt_cpu_wake_quit : 8;
        unsigned int gpu_cfg_cnt_cpu_l2_idle_quit : 8;
        unsigned int gpu_cpu_wake_up_mode : 2;
        unsigned int gpu_cpu_l2_idle_switch_bypass : 1;
        unsigned int gpu_cpu_l2_idle_gt_en : 1;
        unsigned int gpu_dvfs_div_en : 1;
        unsigned int reserved : 1;
        unsigned int sel_ckmux_gpu_icg : 1;
        unsigned int gpu_clk_div_cfg : 2;
    } reg;
} SOC_PMCTRL_VS_CTRL_GPU_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_GPU_vol_drop_en_gpu_START (0)
#define SOC_PMCTRL_VS_CTRL_GPU_vol_drop_en_gpu_END (0)
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
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cfg_cnt_cpu_wake_quit_START (7)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cfg_cnt_cpu_wake_quit_END (14)
#define SOC_PMCTRL_VS_CTRL_GPU_gpu_cfg_cnt_cpu_l2_idle_quit_START (15)
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
        unsigned int svfd_test_ffs_gpu : 2;
        unsigned int svfd_test_cmp_gpu : 7;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 19;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL1_GPU_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL1_GPU_svfd_glitch_test_gpu_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL1_GPU_svfd_glitch_test_gpu_END (0)
#define SOC_PMCTRL_VS_SVFD_CTRL1_GPU_svfd_test_ffs_gpu_START (1)
#define SOC_PMCTRL_VS_SVFD_CTRL1_GPU_svfd_test_ffs_gpu_END (2)
#define SOC_PMCTRL_VS_SVFD_CTRL1_GPU_svfd_test_cmp_gpu_START (3)
#define SOC_PMCTRL_VS_SVFD_CTRL1_GPU_svfd_test_cmp_gpu_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_enalbe_mode_gpu : 2;
        unsigned int svfs_cpm_threshold_r_gpu : 6;
        unsigned int svfs_cpm_threshold_f_gpu : 6;
        unsigned int reserved : 2;
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
        unsigned int vs_ctrl_en_npu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VS_CTRL_EN_NPU_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_EN_NPU_vs_ctrl_en_npu_START (0)
#define SOC_PMCTRL_VS_CTRL_EN_NPU_vs_ctrl_en_npu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_ctrl_bypass_npu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VS_CTRL_BYPASS_NPU_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_BYPASS_NPU_vs_ctrl_bypass_npu_START (0)
#define SOC_PMCTRL_VS_CTRL_BYPASS_NPU_vs_ctrl_bypass_npu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vol_drop_en_npu : 1;
        unsigned int npu_cpu0_wfi_wfe_bypass : 1;
        unsigned int npu_cpu1_wfi_wfe_bypass : 1;
        unsigned int npu_cpu2_wfi_wfe_bypass : 1;
        unsigned int npu_cpu3_wfi_wfe_bypass : 1;
        unsigned int npu_l2_idle_div_mod : 2;
        unsigned int npu_cfg_cnt_cpu_wake_quit : 8;
        unsigned int npu_cfg_cnt_cpu_l2_idle_quit : 8;
        unsigned int npu_cpu_wake_up_mode : 2;
        unsigned int npu_cpu_l2_idle_switch_bypass : 1;
        unsigned int npu_cpu_l2_idle_gt_en : 1;
        unsigned int npu_dvfs_div_en : 1;
        unsigned int reserved : 1;
        unsigned int sel_ckmux_npu_icg : 1;
        unsigned int cpu_clk_div_cfg_npu : 2;
    } reg;
} SOC_PMCTRL_VS_CTRL_NPU_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_NPU_vol_drop_en_npu_START (0)
#define SOC_PMCTRL_VS_CTRL_NPU_vol_drop_en_npu_END (0)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cpu0_wfi_wfe_bypass_START (1)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cpu0_wfi_wfe_bypass_END (1)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cpu1_wfi_wfe_bypass_START (2)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cpu1_wfi_wfe_bypass_END (2)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cpu2_wfi_wfe_bypass_START (3)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cpu2_wfi_wfe_bypass_END (3)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cpu3_wfi_wfe_bypass_START (4)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cpu3_wfi_wfe_bypass_END (4)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_l2_idle_div_mod_START (5)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_l2_idle_div_mod_END (6)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cfg_cnt_cpu_wake_quit_START (7)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cfg_cnt_cpu_wake_quit_END (14)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cfg_cnt_cpu_l2_idle_quit_START (15)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cfg_cnt_cpu_l2_idle_quit_END (22)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cpu_wake_up_mode_START (23)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cpu_wake_up_mode_END (24)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cpu_l2_idle_switch_bypass_START (25)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cpu_l2_idle_switch_bypass_END (25)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cpu_l2_idle_gt_en_START (26)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_cpu_l2_idle_gt_en_END (26)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_dvfs_div_en_START (27)
#define SOC_PMCTRL_VS_CTRL_NPU_npu_dvfs_div_en_END (27)
#define SOC_PMCTRL_VS_CTRL_NPU_sel_ckmux_npu_icg_START (29)
#define SOC_PMCTRL_VS_CTRL_NPU_sel_ckmux_npu_icg_END (29)
#define SOC_PMCTRL_VS_CTRL_NPU_cpu_clk_div_cfg_npu_START (30)
#define SOC_PMCTRL_VS_CTRL_NPU_cpu_clk_div_cfg_npu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_data_limit_e_npu : 1;
        unsigned int svfd_off_time_step_npu : 1;
        unsigned int svfd_pulse_width_sel_npu : 1;
        unsigned int reserved_0 : 1;
        unsigned int svfd_ulvt_ll_npu : 4;
        unsigned int svfd_ulvt_sl_npu : 4;
        unsigned int svfd_lvt_ll_npu : 4;
        unsigned int svfd_lvt_sl_npu : 4;
        unsigned int svfd_trim_npu : 2;
        unsigned int reserved_1 : 2;
        unsigned int svfd_d_rate_npu : 2;
        unsigned int svfd_off_mode_npu : 1;
        unsigned int svfd_div64_en_npu : 1;
        unsigned int svfd_cpm_period_npu : 1;
        unsigned int svfd_edge_sel_npu : 1;
        unsigned int svfd_cmp_data_mode_npu : 2;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL0_NPU_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_data_limit_e_npu_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_data_limit_e_npu_END (0)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_off_time_step_npu_START (1)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_off_time_step_npu_END (1)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_pulse_width_sel_npu_START (2)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_pulse_width_sel_npu_END (2)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_ulvt_ll_npu_START (4)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_ulvt_ll_npu_END (7)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_ulvt_sl_npu_START (8)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_ulvt_sl_npu_END (11)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_lvt_ll_npu_START (12)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_lvt_ll_npu_END (15)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_lvt_sl_npu_START (16)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_lvt_sl_npu_END (19)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_trim_npu_START (20)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_trim_npu_END (21)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_d_rate_npu_START (24)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_d_rate_npu_END (25)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_off_mode_npu_START (26)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_off_mode_npu_END (26)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_div64_en_npu_START (27)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_div64_en_npu_END (27)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_cpm_period_npu_START (28)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_cpm_period_npu_END (28)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_edge_sel_npu_START (29)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_edge_sel_npu_END (29)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_cmp_data_mode_npu_START (30)
#define SOC_PMCTRL_VS_SVFD_CTRL0_NPU_svfd_cmp_data_mode_npu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_glitch_test_npu : 1;
        unsigned int svfd_test_ffs_npu : 2;
        unsigned int svfd_test_cmp_npu : 7;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 19;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL1_NPU_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL1_NPU_svfd_glitch_test_npu_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL1_NPU_svfd_glitch_test_npu_END (0)
#define SOC_PMCTRL_VS_SVFD_CTRL1_NPU_svfd_test_ffs_npu_START (1)
#define SOC_PMCTRL_VS_SVFD_CTRL1_NPU_svfd_test_ffs_npu_END (2)
#define SOC_PMCTRL_VS_SVFD_CTRL1_NPU_svfd_test_cmp_npu_START (3)
#define SOC_PMCTRL_VS_SVFD_CTRL1_NPU_svfd_test_cmp_npu_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_enalbe_mode_npu : 2;
        unsigned int svfs_cpm_threshold_r_npu : 6;
        unsigned int svfs_cpm_threshold_f_npu : 6;
        unsigned int reserved : 2;
        unsigned int vs_svfd_ctrl2_npu_msk : 16;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL2_NPU_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL2_NPU_svfd_enalbe_mode_npu_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL2_NPU_svfd_enalbe_mode_npu_END (1)
#define SOC_PMCTRL_VS_SVFD_CTRL2_NPU_svfs_cpm_threshold_r_npu_START (2)
#define SOC_PMCTRL_VS_SVFD_CTRL2_NPU_svfs_cpm_threshold_r_npu_END (7)
#define SOC_PMCTRL_VS_SVFD_CTRL2_NPU_svfs_cpm_threshold_f_npu_START (8)
#define SOC_PMCTRL_VS_SVFD_CTRL2_NPU_svfs_cpm_threshold_f_npu_END (13)
#define SOC_PMCTRL_VS_SVFD_CTRL2_NPU_vs_svfd_ctrl2_npu_msk_START (16)
#define SOC_PMCTRL_VS_SVFD_CTRL2_NPU_vs_svfd_ctrl2_npu_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_cpm_data_r_npu : 8;
        unsigned int svfd_cpm_data_f_npu : 8;
        unsigned int svfd_dll_lock_npu : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_PMCTRL_VS_CPM_DATA_STAT_NPU_UNION;
#endif
#define SOC_PMCTRL_VS_CPM_DATA_STAT_NPU_svfd_cpm_data_r_npu_START (0)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_NPU_svfd_cpm_data_r_npu_END (7)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_NPU_svfd_cpm_data_f_npu_START (8)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_NPU_svfd_cpm_data_f_npu_END (15)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_NPU_svfd_dll_lock_npu_START (16)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_NPU_svfd_dll_lock_npu_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_test_out_ffs_npu : 4;
        unsigned int svfd_test_out_cpm_npu : 4;
        unsigned int svfd_glitch_result_npu : 1;
        unsigned int idle_low_freq_en_npu : 1;
        unsigned int vbat_drop_protect_ind_npu : 1;
        unsigned int reserved : 21;
    } reg;
} SOC_PMCTRL_VS_TEST_STAT_NPU_UNION;
#endif
#define SOC_PMCTRL_VS_TEST_STAT_NPU_svfd_test_out_ffs_npu_START (0)
#define SOC_PMCTRL_VS_TEST_STAT_NPU_svfd_test_out_ffs_npu_END (3)
#define SOC_PMCTRL_VS_TEST_STAT_NPU_svfd_test_out_cpm_npu_START (4)
#define SOC_PMCTRL_VS_TEST_STAT_NPU_svfd_test_out_cpm_npu_END (7)
#define SOC_PMCTRL_VS_TEST_STAT_NPU_svfd_glitch_result_npu_START (8)
#define SOC_PMCTRL_VS_TEST_STAT_NPU_svfd_glitch_result_npu_END (8)
#define SOC_PMCTRL_VS_TEST_STAT_NPU_idle_low_freq_en_npu_START (9)
#define SOC_PMCTRL_VS_TEST_STAT_NPU_idle_low_freq_en_npu_END (9)
#define SOC_PMCTRL_VS_TEST_STAT_NPU_vbat_drop_protect_ind_npu_START (10)
#define SOC_PMCTRL_VS_TEST_STAT_NPU_vbat_drop_protect_ind_npu_END (10)
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
        unsigned int reserved_1 : 27;
    } reg;
} SOC_PMCTRL_PERIHPMEN_UNION;
#endif
#define SOC_PMCTRL_PERIHPMEN_peri_hpm_en_START (0)
#define SOC_PMCTRL_PERIHPMEN_peri_hpm_en_END (0)
#define SOC_PMCTRL_PERIHPMEN_peri_hpmx_en_START (4)
#define SOC_PMCTRL_PERIHPMEN_peri_hpmx_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_hpm_clk_div : 6;
        unsigned int reserved : 26;
    } reg;
} SOC_PMCTRL_PERIHPMCLKDIV_UNION;
#endif
#define SOC_PMCTRL_PERIHPMCLKDIV_peri_hpm_clk_div_START (0)
#define SOC_PMCTRL_PERIHPMCLKDIV_peri_hpm_clk_div_END (5)
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
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int cluster0_fbdiv_sw_initial : 12;
        unsigned int cluster0_postdiv1_sw_initial : 3;
        unsigned int cluster0_postdiv2_sw_initial : 3;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_PMCTRL_CLUSTER0_SW_INITIAL0_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_SW_INITIAL0_cluster0_fbdiv_sw_initial_START (8)
#define SOC_PMCTRL_CLUSTER0_SW_INITIAL0_cluster0_fbdiv_sw_initial_END (19)
#define SOC_PMCTRL_CLUSTER0_SW_INITIAL0_cluster0_postdiv1_sw_initial_START (20)
#define SOC_PMCTRL_CLUSTER0_SW_INITIAL0_cluster0_postdiv1_sw_initial_END (22)
#define SOC_PMCTRL_CLUSTER0_SW_INITIAL0_cluster0_postdiv2_sw_initial_START (23)
#define SOC_PMCTRL_CLUSTER0_SW_INITIAL0_cluster0_postdiv2_sw_initial_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_fracdiv_sw_initial : 24;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 5;
    } reg;
} SOC_PMCTRL_CLUSTER0_SW_INITIAL1_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_SW_INITIAL1_cluster0_fracdiv_sw_initial_START (0)
#define SOC_PMCTRL_CLUSTER0_SW_INITIAL1_cluster0_fracdiv_sw_initial_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int g3d_fbdiv_sw_initial : 12;
        unsigned int g3d_postdiv1_sw_initial : 3;
        unsigned int g3d_postdiv2_sw_initial : 3;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_PMCTRL_G3D_SW_INITIAL0_UNION;
#endif
#define SOC_PMCTRL_G3D_SW_INITIAL0_g3d_fbdiv_sw_initial_START (8)
#define SOC_PMCTRL_G3D_SW_INITIAL0_g3d_fbdiv_sw_initial_END (19)
#define SOC_PMCTRL_G3D_SW_INITIAL0_g3d_postdiv1_sw_initial_START (20)
#define SOC_PMCTRL_G3D_SW_INITIAL0_g3d_postdiv1_sw_initial_END (22)
#define SOC_PMCTRL_G3D_SW_INITIAL0_g3d_postdiv2_sw_initial_START (23)
#define SOC_PMCTRL_G3D_SW_INITIAL0_g3d_postdiv2_sw_initial_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_fracdiv_sw_initial : 24;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 5;
    } reg;
} SOC_PMCTRL_G3D_SW_INITIAL1_UNION;
#endif
#define SOC_PMCTRL_G3D_SW_INITIAL1_g3d_fracdiv_sw_initial_START (0)
#define SOC_PMCTRL_G3D_SW_INITIAL1_g3d_fracdiv_sw_initial_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int l3_fbdiv_sw_initial : 12;
        unsigned int l3_postdiv1_sw_initial : 3;
        unsigned int l3_postdiv2_sw_initial : 3;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_PMCTRL_L3_SW_INITIAL0_UNION;
#endif
#define SOC_PMCTRL_L3_SW_INITIAL0_l3_fbdiv_sw_initial_START (8)
#define SOC_PMCTRL_L3_SW_INITIAL0_l3_fbdiv_sw_initial_END (19)
#define SOC_PMCTRL_L3_SW_INITIAL0_l3_postdiv1_sw_initial_START (20)
#define SOC_PMCTRL_L3_SW_INITIAL0_l3_postdiv1_sw_initial_END (22)
#define SOC_PMCTRL_L3_SW_INITIAL0_l3_postdiv2_sw_initial_START (23)
#define SOC_PMCTRL_L3_SW_INITIAL0_l3_postdiv2_sw_initial_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_fracdiv_sw_initial : 24;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 5;
    } reg;
} SOC_PMCTRL_L3_SW_INITIAL1_UNION;
#endif
#define SOC_PMCTRL_L3_SW_INITIAL1_l3_fracdiv_sw_initial_START (0)
#define SOC_PMCTRL_L3_SW_INITIAL1_l3_fracdiv_sw_initial_END (23)
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
        unsigned int ppll0_en : 1;
        unsigned int ppll0_bp : 1;
        unsigned int ppll0_refdiv : 6;
        unsigned int ppll0_fbdiv : 12;
        unsigned int ppll0_postdiv1 : 3;
        unsigned int ppll0_postdiv2 : 3;
        unsigned int ppll0_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_PPLL0CTRL0_UNION;
#endif
#define SOC_PMCTRL_PPLL0CTRL0_ppll0_en_START (0)
#define SOC_PMCTRL_PPLL0CTRL0_ppll0_en_END (0)
#define SOC_PMCTRL_PPLL0CTRL0_ppll0_bp_START (1)
#define SOC_PMCTRL_PPLL0CTRL0_ppll0_bp_END (1)
#define SOC_PMCTRL_PPLL0CTRL0_ppll0_refdiv_START (2)
#define SOC_PMCTRL_PPLL0CTRL0_ppll0_refdiv_END (7)
#define SOC_PMCTRL_PPLL0CTRL0_ppll0_fbdiv_START (8)
#define SOC_PMCTRL_PPLL0CTRL0_ppll0_fbdiv_END (19)
#define SOC_PMCTRL_PPLL0CTRL0_ppll0_postdiv1_START (20)
#define SOC_PMCTRL_PPLL0CTRL0_ppll0_postdiv1_END (22)
#define SOC_PMCTRL_PPLL0CTRL0_ppll0_postdiv2_START (23)
#define SOC_PMCTRL_PPLL0CTRL0_ppll0_postdiv2_END (25)
#define SOC_PMCTRL_PPLL0CTRL0_ppll0_lock_START (26)
#define SOC_PMCTRL_PPLL0CTRL0_ppll0_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll0_fracdiv : 24;
        unsigned int ppll0_int_mod : 1;
        unsigned int ppll0_cfg_vld : 1;
        unsigned int gt_clk_ppll0 : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_PPLL0CTRL1_UNION;
#endif
#define SOC_PMCTRL_PPLL0CTRL1_ppll0_fracdiv_START (0)
#define SOC_PMCTRL_PPLL0CTRL1_ppll0_fracdiv_END (23)
#define SOC_PMCTRL_PPLL0CTRL1_ppll0_int_mod_START (24)
#define SOC_PMCTRL_PPLL0CTRL1_ppll0_int_mod_END (24)
#define SOC_PMCTRL_PPLL0CTRL1_ppll0_cfg_vld_START (25)
#define SOC_PMCTRL_PPLL0CTRL1_ppll0_cfg_vld_END (25)
#define SOC_PMCTRL_PPLL0CTRL1_gt_clk_ppll0_START (26)
#define SOC_PMCTRL_PPLL0CTRL1_gt_clk_ppll0_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll0_ssc_reset : 1;
        unsigned int ppll0_ssc_disable : 1;
        unsigned int ppll0_ssc_downspread : 1;
        unsigned int ppll0_ssc_spread : 3;
        unsigned int ppll0_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_PMCTRL_PPLL0SSCCTRL_UNION;
#endif
#define SOC_PMCTRL_PPLL0SSCCTRL_ppll0_ssc_reset_START (0)
#define SOC_PMCTRL_PPLL0SSCCTRL_ppll0_ssc_reset_END (0)
#define SOC_PMCTRL_PPLL0SSCCTRL_ppll0_ssc_disable_START (1)
#define SOC_PMCTRL_PPLL0SSCCTRL_ppll0_ssc_disable_END (1)
#define SOC_PMCTRL_PPLL0SSCCTRL_ppll0_ssc_downspread_START (2)
#define SOC_PMCTRL_PPLL0SSCCTRL_ppll0_ssc_downspread_END (2)
#define SOC_PMCTRL_PPLL0SSCCTRL_ppll0_ssc_spread_START (3)
#define SOC_PMCTRL_PPLL0SSCCTRL_ppll0_ssc_spread_END (5)
#define SOC_PMCTRL_PPLL0SSCCTRL_ppll0_ssc_divval_START (6)
#define SOC_PMCTRL_PPLL0SSCCTRL_ppll0_ssc_divval_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll1_en : 1;
        unsigned int ppll1_bp : 1;
        unsigned int ppll1_refdiv : 6;
        unsigned int ppll1_fbdiv : 12;
        unsigned int ppll1_postdiv1 : 3;
        unsigned int ppll1_postdiv2 : 3;
        unsigned int ppll1_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_PPLL1CTRL0_UNION;
#endif
#define SOC_PMCTRL_PPLL1CTRL0_ppll1_en_START (0)
#define SOC_PMCTRL_PPLL1CTRL0_ppll1_en_END (0)
#define SOC_PMCTRL_PPLL1CTRL0_ppll1_bp_START (1)
#define SOC_PMCTRL_PPLL1CTRL0_ppll1_bp_END (1)
#define SOC_PMCTRL_PPLL1CTRL0_ppll1_refdiv_START (2)
#define SOC_PMCTRL_PPLL1CTRL0_ppll1_refdiv_END (7)
#define SOC_PMCTRL_PPLL1CTRL0_ppll1_fbdiv_START (8)
#define SOC_PMCTRL_PPLL1CTRL0_ppll1_fbdiv_END (19)
#define SOC_PMCTRL_PPLL1CTRL0_ppll1_postdiv1_START (20)
#define SOC_PMCTRL_PPLL1CTRL0_ppll1_postdiv1_END (22)
#define SOC_PMCTRL_PPLL1CTRL0_ppll1_postdiv2_START (23)
#define SOC_PMCTRL_PPLL1CTRL0_ppll1_postdiv2_END (25)
#define SOC_PMCTRL_PPLL1CTRL0_ppll1_lock_START (26)
#define SOC_PMCTRL_PPLL1CTRL0_ppll1_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll1_fracdiv : 24;
        unsigned int ppll1_int_mod : 1;
        unsigned int ppll1_cfg_vld : 1;
        unsigned int gt_clk_ppll1 : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_PPLL1CTRL1_UNION;
#endif
#define SOC_PMCTRL_PPLL1CTRL1_ppll1_fracdiv_START (0)
#define SOC_PMCTRL_PPLL1CTRL1_ppll1_fracdiv_END (23)
#define SOC_PMCTRL_PPLL1CTRL1_ppll1_int_mod_START (24)
#define SOC_PMCTRL_PPLL1CTRL1_ppll1_int_mod_END (24)
#define SOC_PMCTRL_PPLL1CTRL1_ppll1_cfg_vld_START (25)
#define SOC_PMCTRL_PPLL1CTRL1_ppll1_cfg_vld_END (25)
#define SOC_PMCTRL_PPLL1CTRL1_gt_clk_ppll1_START (26)
#define SOC_PMCTRL_PPLL1CTRL1_gt_clk_ppll1_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll2_en : 1;
        unsigned int ppll2_bp : 1;
        unsigned int ppll2_refdiv : 6;
        unsigned int ppll2_fbdiv : 12;
        unsigned int ppll2_postdiv1 : 3;
        unsigned int ppll2_postdiv2 : 3;
        unsigned int ppll2_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_PPLL2CTRL0_UNION;
#endif
#define SOC_PMCTRL_PPLL2CTRL0_ppll2_en_START (0)
#define SOC_PMCTRL_PPLL2CTRL0_ppll2_en_END (0)
#define SOC_PMCTRL_PPLL2CTRL0_ppll2_bp_START (1)
#define SOC_PMCTRL_PPLL2CTRL0_ppll2_bp_END (1)
#define SOC_PMCTRL_PPLL2CTRL0_ppll2_refdiv_START (2)
#define SOC_PMCTRL_PPLL2CTRL0_ppll2_refdiv_END (7)
#define SOC_PMCTRL_PPLL2CTRL0_ppll2_fbdiv_START (8)
#define SOC_PMCTRL_PPLL2CTRL0_ppll2_fbdiv_END (19)
#define SOC_PMCTRL_PPLL2CTRL0_ppll2_postdiv1_START (20)
#define SOC_PMCTRL_PPLL2CTRL0_ppll2_postdiv1_END (22)
#define SOC_PMCTRL_PPLL2CTRL0_ppll2_postdiv2_START (23)
#define SOC_PMCTRL_PPLL2CTRL0_ppll2_postdiv2_END (25)
#define SOC_PMCTRL_PPLL2CTRL0_ppll2_lock_START (26)
#define SOC_PMCTRL_PPLL2CTRL0_ppll2_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll2_fracdiv : 24;
        unsigned int ppll2_int_mod : 1;
        unsigned int ppll2_cfg_vld : 1;
        unsigned int gt_clk_ppll2 : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_PPLL2CTRL1_UNION;
#endif
#define SOC_PMCTRL_PPLL2CTRL1_ppll2_fracdiv_START (0)
#define SOC_PMCTRL_PPLL2CTRL1_ppll2_fracdiv_END (23)
#define SOC_PMCTRL_PPLL2CTRL1_ppll2_int_mod_START (24)
#define SOC_PMCTRL_PPLL2CTRL1_ppll2_int_mod_END (24)
#define SOC_PMCTRL_PPLL2CTRL1_ppll2_cfg_vld_START (25)
#define SOC_PMCTRL_PPLL2CTRL1_ppll2_cfg_vld_END (25)
#define SOC_PMCTRL_PPLL2CTRL1_gt_clk_ppll2_START (26)
#define SOC_PMCTRL_PPLL2CTRL1_gt_clk_ppll2_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll3_en : 1;
        unsigned int ppll3_bp : 1;
        unsigned int ppll3_refdiv : 6;
        unsigned int ppll3_fbdiv : 12;
        unsigned int ppll3_postdiv1 : 3;
        unsigned int ppll3_postdiv2 : 3;
        unsigned int ppll3_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_PPLL3CTRL0_UNION;
#endif
#define SOC_PMCTRL_PPLL3CTRL0_ppll3_en_START (0)
#define SOC_PMCTRL_PPLL3CTRL0_ppll3_en_END (0)
#define SOC_PMCTRL_PPLL3CTRL0_ppll3_bp_START (1)
#define SOC_PMCTRL_PPLL3CTRL0_ppll3_bp_END (1)
#define SOC_PMCTRL_PPLL3CTRL0_ppll3_refdiv_START (2)
#define SOC_PMCTRL_PPLL3CTRL0_ppll3_refdiv_END (7)
#define SOC_PMCTRL_PPLL3CTRL0_ppll3_fbdiv_START (8)
#define SOC_PMCTRL_PPLL3CTRL0_ppll3_fbdiv_END (19)
#define SOC_PMCTRL_PPLL3CTRL0_ppll3_postdiv1_START (20)
#define SOC_PMCTRL_PPLL3CTRL0_ppll3_postdiv1_END (22)
#define SOC_PMCTRL_PPLL3CTRL0_ppll3_postdiv2_START (23)
#define SOC_PMCTRL_PPLL3CTRL0_ppll3_postdiv2_END (25)
#define SOC_PMCTRL_PPLL3CTRL0_ppll3_lock_START (26)
#define SOC_PMCTRL_PPLL3CTRL0_ppll3_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll3_fracdiv : 24;
        unsigned int ppll3_int_mod : 1;
        unsigned int ppll3_cfg_vld : 1;
        unsigned int gt_clk_ppll3 : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_PPLL3CTRL1_UNION;
#endif
#define SOC_PMCTRL_PPLL3CTRL1_ppll3_fracdiv_START (0)
#define SOC_PMCTRL_PPLL3CTRL1_ppll3_fracdiv_END (23)
#define SOC_PMCTRL_PPLL3CTRL1_ppll3_int_mod_START (24)
#define SOC_PMCTRL_PPLL3CTRL1_ppll3_int_mod_END (24)
#define SOC_PMCTRL_PPLL3CTRL1_ppll3_cfg_vld_START (25)
#define SOC_PMCTRL_PPLL3CTRL1_ppll3_cfg_vld_END (25)
#define SOC_PMCTRL_PPLL3CTRL1_gt_clk_ppll3_START (26)
#define SOC_PMCTRL_PPLL3CTRL1_gt_clk_ppll3_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll4_en : 1;
        unsigned int ppll4_bp : 1;
        unsigned int ppll4_refdiv : 6;
        unsigned int ppll4_fbdiv : 12;
        unsigned int ppll4_postdiv1 : 3;
        unsigned int ppll4_postdiv2 : 3;
        unsigned int ppll4_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_PPLL4CTRL0_UNION;
#endif
#define SOC_PMCTRL_PPLL4CTRL0_ppll4_en_START (0)
#define SOC_PMCTRL_PPLL4CTRL0_ppll4_en_END (0)
#define SOC_PMCTRL_PPLL4CTRL0_ppll4_bp_START (1)
#define SOC_PMCTRL_PPLL4CTRL0_ppll4_bp_END (1)
#define SOC_PMCTRL_PPLL4CTRL0_ppll4_refdiv_START (2)
#define SOC_PMCTRL_PPLL4CTRL0_ppll4_refdiv_END (7)
#define SOC_PMCTRL_PPLL4CTRL0_ppll4_fbdiv_START (8)
#define SOC_PMCTRL_PPLL4CTRL0_ppll4_fbdiv_END (19)
#define SOC_PMCTRL_PPLL4CTRL0_ppll4_postdiv1_START (20)
#define SOC_PMCTRL_PPLL4CTRL0_ppll4_postdiv1_END (22)
#define SOC_PMCTRL_PPLL4CTRL0_ppll4_postdiv2_START (23)
#define SOC_PMCTRL_PPLL4CTRL0_ppll4_postdiv2_END (25)
#define SOC_PMCTRL_PPLL4CTRL0_ppll4_lock_START (26)
#define SOC_PMCTRL_PPLL4CTRL0_ppll4_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll4_fracdiv : 24;
        unsigned int ppll4_int_mod : 1;
        unsigned int ppll4_cfg_vld : 1;
        unsigned int gt_clk_ppll4 : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_PPLL4CTRL1_UNION;
#endif
#define SOC_PMCTRL_PPLL4CTRL1_ppll4_fracdiv_START (0)
#define SOC_PMCTRL_PPLL4CTRL1_ppll4_fracdiv_END (23)
#define SOC_PMCTRL_PPLL4CTRL1_ppll4_int_mod_START (24)
#define SOC_PMCTRL_PPLL4CTRL1_ppll4_int_mod_END (24)
#define SOC_PMCTRL_PPLL4CTRL1_ppll4_cfg_vld_START (25)
#define SOC_PMCTRL_PPLL4CTRL1_ppll4_cfg_vld_END (25)
#define SOC_PMCTRL_PPLL4CTRL1_gt_clk_ppll4_START (26)
#define SOC_PMCTRL_PPLL4CTRL1_gt_clk_ppll4_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll5_en : 1;
        unsigned int ppll5_bp : 1;
        unsigned int ppll5_refdiv : 6;
        unsigned int ppll5_fbdiv : 12;
        unsigned int ppll5_postdiv1 : 3;
        unsigned int ppll5_postdiv2 : 3;
        unsigned int ppll5_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_PPLL5CTRL0_UNION;
#endif
#define SOC_PMCTRL_PPLL5CTRL0_ppll5_en_START (0)
#define SOC_PMCTRL_PPLL5CTRL0_ppll5_en_END (0)
#define SOC_PMCTRL_PPLL5CTRL0_ppll5_bp_START (1)
#define SOC_PMCTRL_PPLL5CTRL0_ppll5_bp_END (1)
#define SOC_PMCTRL_PPLL5CTRL0_ppll5_refdiv_START (2)
#define SOC_PMCTRL_PPLL5CTRL0_ppll5_refdiv_END (7)
#define SOC_PMCTRL_PPLL5CTRL0_ppll5_fbdiv_START (8)
#define SOC_PMCTRL_PPLL5CTRL0_ppll5_fbdiv_END (19)
#define SOC_PMCTRL_PPLL5CTRL0_ppll5_postdiv1_START (20)
#define SOC_PMCTRL_PPLL5CTRL0_ppll5_postdiv1_END (22)
#define SOC_PMCTRL_PPLL5CTRL0_ppll5_postdiv2_START (23)
#define SOC_PMCTRL_PPLL5CTRL0_ppll5_postdiv2_END (25)
#define SOC_PMCTRL_PPLL5CTRL0_ppll5_lock_START (26)
#define SOC_PMCTRL_PPLL5CTRL0_ppll5_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll5_fracdiv : 24;
        unsigned int ppll5_int_mod : 1;
        unsigned int ppll5_cfg_vld : 1;
        unsigned int gt_clk_ppll5 : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_PPLL5CTRL1_UNION;
#endif
#define SOC_PMCTRL_PPLL5CTRL1_ppll5_fracdiv_START (0)
#define SOC_PMCTRL_PPLL5CTRL1_ppll5_fracdiv_END (23)
#define SOC_PMCTRL_PPLL5CTRL1_ppll5_int_mod_START (24)
#define SOC_PMCTRL_PPLL5CTRL1_ppll5_int_mod_END (24)
#define SOC_PMCTRL_PPLL5CTRL1_ppll5_cfg_vld_START (25)
#define SOC_PMCTRL_PPLL5CTRL1_ppll5_cfg_vld_END (25)
#define SOC_PMCTRL_PPLL5CTRL1_gt_clk_ppll5_START (26)
#define SOC_PMCTRL_PPLL5CTRL1_gt_clk_ppll5_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll6_en : 1;
        unsigned int ppll6_bp : 1;
        unsigned int ppll6_refdiv : 6;
        unsigned int ppll6_fbdiv : 12;
        unsigned int ppll6_postdiv1 : 3;
        unsigned int ppll6_postdiv2 : 3;
        unsigned int ppll6_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_PPLL6CTRL0_UNION;
#endif
#define SOC_PMCTRL_PPLL6CTRL0_ppll6_en_START (0)
#define SOC_PMCTRL_PPLL6CTRL0_ppll6_en_END (0)
#define SOC_PMCTRL_PPLL6CTRL0_ppll6_bp_START (1)
#define SOC_PMCTRL_PPLL6CTRL0_ppll6_bp_END (1)
#define SOC_PMCTRL_PPLL6CTRL0_ppll6_refdiv_START (2)
#define SOC_PMCTRL_PPLL6CTRL0_ppll6_refdiv_END (7)
#define SOC_PMCTRL_PPLL6CTRL0_ppll6_fbdiv_START (8)
#define SOC_PMCTRL_PPLL6CTRL0_ppll6_fbdiv_END (19)
#define SOC_PMCTRL_PPLL6CTRL0_ppll6_postdiv1_START (20)
#define SOC_PMCTRL_PPLL6CTRL0_ppll6_postdiv1_END (22)
#define SOC_PMCTRL_PPLL6CTRL0_ppll6_postdiv2_START (23)
#define SOC_PMCTRL_PPLL6CTRL0_ppll6_postdiv2_END (25)
#define SOC_PMCTRL_PPLL6CTRL0_ppll6_lock_START (26)
#define SOC_PMCTRL_PPLL6CTRL0_ppll6_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll6_fracdiv : 24;
        unsigned int ppll6_int_mod : 1;
        unsigned int ppll6_cfg_vld : 1;
        unsigned int gt_clk_ppll6 : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_PPLL6CTRL1_UNION;
#endif
#define SOC_PMCTRL_PPLL6CTRL1_ppll6_fracdiv_START (0)
#define SOC_PMCTRL_PPLL6CTRL1_ppll6_fracdiv_END (23)
#define SOC_PMCTRL_PPLL6CTRL1_ppll6_int_mod_START (24)
#define SOC_PMCTRL_PPLL6CTRL1_ppll6_int_mod_END (24)
#define SOC_PMCTRL_PPLL6CTRL1_ppll6_cfg_vld_START (25)
#define SOC_PMCTRL_PPLL6CTRL1_ppll6_cfg_vld_END (25)
#define SOC_PMCTRL_PPLL6CTRL1_gt_clk_ppll6_START (26)
#define SOC_PMCTRL_PPLL6CTRL1_gt_clk_ppll6_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll7_en : 1;
        unsigned int ppll7_bp : 1;
        unsigned int ppll7_refdiv : 6;
        unsigned int ppll7_fbdiv : 12;
        unsigned int ppll7_postdiv1 : 3;
        unsigned int ppll7_postdiv2 : 3;
        unsigned int ppll7_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_PPLL7CTRL0_UNION;
#endif
#define SOC_PMCTRL_PPLL7CTRL0_ppll7_en_START (0)
#define SOC_PMCTRL_PPLL7CTRL0_ppll7_en_END (0)
#define SOC_PMCTRL_PPLL7CTRL0_ppll7_bp_START (1)
#define SOC_PMCTRL_PPLL7CTRL0_ppll7_bp_END (1)
#define SOC_PMCTRL_PPLL7CTRL0_ppll7_refdiv_START (2)
#define SOC_PMCTRL_PPLL7CTRL0_ppll7_refdiv_END (7)
#define SOC_PMCTRL_PPLL7CTRL0_ppll7_fbdiv_START (8)
#define SOC_PMCTRL_PPLL7CTRL0_ppll7_fbdiv_END (19)
#define SOC_PMCTRL_PPLL7CTRL0_ppll7_postdiv1_START (20)
#define SOC_PMCTRL_PPLL7CTRL0_ppll7_postdiv1_END (22)
#define SOC_PMCTRL_PPLL7CTRL0_ppll7_postdiv2_START (23)
#define SOC_PMCTRL_PPLL7CTRL0_ppll7_postdiv2_END (25)
#define SOC_PMCTRL_PPLL7CTRL0_ppll7_lock_START (26)
#define SOC_PMCTRL_PPLL7CTRL0_ppll7_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll7_fracdiv : 24;
        unsigned int ppll7_int_mod : 1;
        unsigned int ppll7_cfg_vld : 1;
        unsigned int gt_clk_ppll7 : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_PPLL7CTRL1_UNION;
#endif
#define SOC_PMCTRL_PPLL7CTRL1_ppll7_fracdiv_START (0)
#define SOC_PMCTRL_PPLL7CTRL1_ppll7_fracdiv_END (23)
#define SOC_PMCTRL_PPLL7CTRL1_ppll7_int_mod_START (24)
#define SOC_PMCTRL_PPLL7CTRL1_ppll7_int_mod_END (24)
#define SOC_PMCTRL_PPLL7CTRL1_ppll7_cfg_vld_START (25)
#define SOC_PMCTRL_PPLL7CTRL1_ppll7_cfg_vld_END (25)
#define SOC_PMCTRL_PPLL7CTRL1_gt_clk_ppll7_START (26)
#define SOC_PMCTRL_PPLL7CTRL1_gt_clk_ppll7_END (26)
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
        unsigned int ppll1_ssc_reset : 1;
        unsigned int ppll1_ssc_disable : 1;
        unsigned int ppll1_ssc_downspread : 1;
        unsigned int ppll1_ssc_spread : 3;
        unsigned int ppll1_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_PMCTRL_PPLL1SSCCTRL_UNION;
#endif
#define SOC_PMCTRL_PPLL1SSCCTRL_ppll1_ssc_reset_START (0)
#define SOC_PMCTRL_PPLL1SSCCTRL_ppll1_ssc_reset_END (0)
#define SOC_PMCTRL_PPLL1SSCCTRL_ppll1_ssc_disable_START (1)
#define SOC_PMCTRL_PPLL1SSCCTRL_ppll1_ssc_disable_END (1)
#define SOC_PMCTRL_PPLL1SSCCTRL_ppll1_ssc_downspread_START (2)
#define SOC_PMCTRL_PPLL1SSCCTRL_ppll1_ssc_downspread_END (2)
#define SOC_PMCTRL_PPLL1SSCCTRL_ppll1_ssc_spread_START (3)
#define SOC_PMCTRL_PPLL1SSCCTRL_ppll1_ssc_spread_END (5)
#define SOC_PMCTRL_PPLL1SSCCTRL_ppll1_ssc_divval_START (6)
#define SOC_PMCTRL_PPLL1SSCCTRL_ppll1_ssc_divval_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll2_ssc_reset : 1;
        unsigned int ppll2_ssc_disable : 1;
        unsigned int ppll2_ssc_downspread : 1;
        unsigned int ppll2_ssc_spread : 3;
        unsigned int ppll2_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_PMCTRL_PPLL2SSCCTRL_UNION;
#endif
#define SOC_PMCTRL_PPLL2SSCCTRL_ppll2_ssc_reset_START (0)
#define SOC_PMCTRL_PPLL2SSCCTRL_ppll2_ssc_reset_END (0)
#define SOC_PMCTRL_PPLL2SSCCTRL_ppll2_ssc_disable_START (1)
#define SOC_PMCTRL_PPLL2SSCCTRL_ppll2_ssc_disable_END (1)
#define SOC_PMCTRL_PPLL2SSCCTRL_ppll2_ssc_downspread_START (2)
#define SOC_PMCTRL_PPLL2SSCCTRL_ppll2_ssc_downspread_END (2)
#define SOC_PMCTRL_PPLL2SSCCTRL_ppll2_ssc_spread_START (3)
#define SOC_PMCTRL_PPLL2SSCCTRL_ppll2_ssc_spread_END (5)
#define SOC_PMCTRL_PPLL2SSCCTRL_ppll2_ssc_divval_START (6)
#define SOC_PMCTRL_PPLL2SSCCTRL_ppll2_ssc_divval_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll3_ssc_reset : 1;
        unsigned int ppll3_ssc_disable : 1;
        unsigned int ppll3_ssc_downspread : 1;
        unsigned int ppll3_ssc_spread : 3;
        unsigned int ppll3_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_PMCTRL_PPLL3SSCCTRL_UNION;
#endif
#define SOC_PMCTRL_PPLL3SSCCTRL_ppll3_ssc_reset_START (0)
#define SOC_PMCTRL_PPLL3SSCCTRL_ppll3_ssc_reset_END (0)
#define SOC_PMCTRL_PPLL3SSCCTRL_ppll3_ssc_disable_START (1)
#define SOC_PMCTRL_PPLL3SSCCTRL_ppll3_ssc_disable_END (1)
#define SOC_PMCTRL_PPLL3SSCCTRL_ppll3_ssc_downspread_START (2)
#define SOC_PMCTRL_PPLL3SSCCTRL_ppll3_ssc_downspread_END (2)
#define SOC_PMCTRL_PPLL3SSCCTRL_ppll3_ssc_spread_START (3)
#define SOC_PMCTRL_PPLL3SSCCTRL_ppll3_ssc_spread_END (5)
#define SOC_PMCTRL_PPLL3SSCCTRL_ppll3_ssc_divval_START (6)
#define SOC_PMCTRL_PPLL3SSCCTRL_ppll3_ssc_divval_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll4_ssc_reset : 1;
        unsigned int ppll4_ssc_disable : 1;
        unsigned int ppll4_ssc_downspread : 1;
        unsigned int ppll4_ssc_spread : 3;
        unsigned int ppll4_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_PMCTRL_PPLL4SSCCTRL_UNION;
#endif
#define SOC_PMCTRL_PPLL4SSCCTRL_ppll4_ssc_reset_START (0)
#define SOC_PMCTRL_PPLL4SSCCTRL_ppll4_ssc_reset_END (0)
#define SOC_PMCTRL_PPLL4SSCCTRL_ppll4_ssc_disable_START (1)
#define SOC_PMCTRL_PPLL4SSCCTRL_ppll4_ssc_disable_END (1)
#define SOC_PMCTRL_PPLL4SSCCTRL_ppll4_ssc_downspread_START (2)
#define SOC_PMCTRL_PPLL4SSCCTRL_ppll4_ssc_downspread_END (2)
#define SOC_PMCTRL_PPLL4SSCCTRL_ppll4_ssc_spread_START (3)
#define SOC_PMCTRL_PPLL4SSCCTRL_ppll4_ssc_spread_END (5)
#define SOC_PMCTRL_PPLL4SSCCTRL_ppll4_ssc_divval_START (6)
#define SOC_PMCTRL_PPLL4SSCCTRL_ppll4_ssc_divval_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll5_ssc_reset : 1;
        unsigned int ppll5_ssc_disable : 1;
        unsigned int ppll5_ssc_downspread : 1;
        unsigned int ppll5_ssc_spread : 3;
        unsigned int ppll5_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_PMCTRL_PPLL5SSCCTRL_UNION;
#endif
#define SOC_PMCTRL_PPLL5SSCCTRL_ppll5_ssc_reset_START (0)
#define SOC_PMCTRL_PPLL5SSCCTRL_ppll5_ssc_reset_END (0)
#define SOC_PMCTRL_PPLL5SSCCTRL_ppll5_ssc_disable_START (1)
#define SOC_PMCTRL_PPLL5SSCCTRL_ppll5_ssc_disable_END (1)
#define SOC_PMCTRL_PPLL5SSCCTRL_ppll5_ssc_downspread_START (2)
#define SOC_PMCTRL_PPLL5SSCCTRL_ppll5_ssc_downspread_END (2)
#define SOC_PMCTRL_PPLL5SSCCTRL_ppll5_ssc_spread_START (3)
#define SOC_PMCTRL_PPLL5SSCCTRL_ppll5_ssc_spread_END (5)
#define SOC_PMCTRL_PPLL5SSCCTRL_ppll5_ssc_divval_START (6)
#define SOC_PMCTRL_PPLL5SSCCTRL_ppll5_ssc_divval_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll6_ssc_reset : 1;
        unsigned int ppll6_ssc_disable : 1;
        unsigned int ppll6_ssc_downspread : 1;
        unsigned int ppll6_ssc_spread : 3;
        unsigned int ppll6_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_PMCTRL_PPLL6SSCCTRL_UNION;
#endif
#define SOC_PMCTRL_PPLL6SSCCTRL_ppll6_ssc_reset_START (0)
#define SOC_PMCTRL_PPLL6SSCCTRL_ppll6_ssc_reset_END (0)
#define SOC_PMCTRL_PPLL6SSCCTRL_ppll6_ssc_disable_START (1)
#define SOC_PMCTRL_PPLL6SSCCTRL_ppll6_ssc_disable_END (1)
#define SOC_PMCTRL_PPLL6SSCCTRL_ppll6_ssc_downspread_START (2)
#define SOC_PMCTRL_PPLL6SSCCTRL_ppll6_ssc_downspread_END (2)
#define SOC_PMCTRL_PPLL6SSCCTRL_ppll6_ssc_spread_START (3)
#define SOC_PMCTRL_PPLL6SSCCTRL_ppll6_ssc_spread_END (5)
#define SOC_PMCTRL_PPLL6SSCCTRL_ppll6_ssc_divval_START (6)
#define SOC_PMCTRL_PPLL6SSCCTRL_ppll6_ssc_divval_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll7_ssc_reset : 1;
        unsigned int ppll7_ssc_disable : 1;
        unsigned int ppll7_ssc_downspread : 1;
        unsigned int ppll7_ssc_spread : 3;
        unsigned int ppll7_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_PMCTRL_PPLL7SSCCTRL_UNION;
#endif
#define SOC_PMCTRL_PPLL7SSCCTRL_ppll7_ssc_reset_START (0)
#define SOC_PMCTRL_PPLL7SSCCTRL_ppll7_ssc_reset_END (0)
#define SOC_PMCTRL_PPLL7SSCCTRL_ppll7_ssc_disable_START (1)
#define SOC_PMCTRL_PPLL7SSCCTRL_ppll7_ssc_disable_END (1)
#define SOC_PMCTRL_PPLL7SSCCTRL_ppll7_ssc_downspread_START (2)
#define SOC_PMCTRL_PPLL7SSCCTRL_ppll7_ssc_downspread_END (2)
#define SOC_PMCTRL_PPLL7SSCCTRL_ppll7_ssc_spread_START (3)
#define SOC_PMCTRL_PPLL7SSCCTRL_ppll7_ssc_spread_END (5)
#define SOC_PMCTRL_PPLL7SSCCTRL_ppll7_ssc_divval_START (6)
#define SOC_PMCTRL_PPLL7SSCCTRL_ppll7_ssc_divval_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll2_ssc_reset : 1;
        unsigned int apll2_ssc_disable : 1;
        unsigned int apll2_ssc_downspread : 1;
        unsigned int apll2_ssc_spread : 3;
        unsigned int apll2_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_PMCTRL_APLL2SSCCTRL_UNION;
#endif
#define SOC_PMCTRL_APLL2SSCCTRL_apll2_ssc_reset_START (0)
#define SOC_PMCTRL_APLL2SSCCTRL_apll2_ssc_reset_END (0)
#define SOC_PMCTRL_APLL2SSCCTRL_apll2_ssc_disable_START (1)
#define SOC_PMCTRL_APLL2SSCCTRL_apll2_ssc_disable_END (1)
#define SOC_PMCTRL_APLL2SSCCTRL_apll2_ssc_downspread_START (2)
#define SOC_PMCTRL_APLL2SSCCTRL_apll2_ssc_downspread_END (2)
#define SOC_PMCTRL_APLL2SSCCTRL_apll2_ssc_spread_START (3)
#define SOC_PMCTRL_APLL2SSCCTRL_apll2_ssc_spread_END (5)
#define SOC_PMCTRL_APLL2SSCCTRL_apll2_ssc_divval_START (6)
#define SOC_PMCTRL_APLL2SSCCTRL_apll2_ssc_divval_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_clk_sel : 1;
        unsigned int reserved_0 : 3;
        unsigned int l3_clk_sel_stat : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_PMCTRL_L3_CLKSEL_UNION;
#endif
#define SOC_PMCTRL_L3_CLKSEL_l3_clk_sel_START (0)
#define SOC_PMCTRL_L3_CLKSEL_l3_clk_sel_END (0)
#define SOC_PMCTRL_L3_CLKSEL_l3_clk_sel_stat_START (4)
#define SOC_PMCTRL_L3_CLKSEL_l3_clk_sel_stat_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_pclkendbg_sel_cfg : 2;
        unsigned int l3_atclken_sel_cfg : 2;
        unsigned int l3_aclkenm_sel_cfg : 3;
        unsigned int l3_atclken_l_sel_cfg : 1;
        unsigned int l3_peri_clk_sel_cfg : 2;
        unsigned int l3_acp_clk_sel_cfg : 2;
        unsigned int reserved_0 : 4;
        unsigned int l3_pclkendbg_sel_stat : 2;
        unsigned int l3_atclken_sel_stat : 2;
        unsigned int l3_aclkenm_sel_stat : 3;
        unsigned int l3_atclken_l_sel_stat : 1;
        unsigned int l3_peri_clk_sel_stat : 2;
        unsigned int l3_acp_clk_sel_stat : 2;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_PMCTRL_L3_CLKDIV_UNION;
#endif
#define SOC_PMCTRL_L3_CLKDIV_l3_pclkendbg_sel_cfg_START (0)
#define SOC_PMCTRL_L3_CLKDIV_l3_pclkendbg_sel_cfg_END (1)
#define SOC_PMCTRL_L3_CLKDIV_l3_atclken_sel_cfg_START (2)
#define SOC_PMCTRL_L3_CLKDIV_l3_atclken_sel_cfg_END (3)
#define SOC_PMCTRL_L3_CLKDIV_l3_aclkenm_sel_cfg_START (4)
#define SOC_PMCTRL_L3_CLKDIV_l3_aclkenm_sel_cfg_END (6)
#define SOC_PMCTRL_L3_CLKDIV_l3_atclken_l_sel_cfg_START (7)
#define SOC_PMCTRL_L3_CLKDIV_l3_atclken_l_sel_cfg_END (7)
#define SOC_PMCTRL_L3_CLKDIV_l3_peri_clk_sel_cfg_START (8)
#define SOC_PMCTRL_L3_CLKDIV_l3_peri_clk_sel_cfg_END (9)
#define SOC_PMCTRL_L3_CLKDIV_l3_acp_clk_sel_cfg_START (10)
#define SOC_PMCTRL_L3_CLKDIV_l3_acp_clk_sel_cfg_END (11)
#define SOC_PMCTRL_L3_CLKDIV_l3_pclkendbg_sel_stat_START (16)
#define SOC_PMCTRL_L3_CLKDIV_l3_pclkendbg_sel_stat_END (17)
#define SOC_PMCTRL_L3_CLKDIV_l3_atclken_sel_stat_START (18)
#define SOC_PMCTRL_L3_CLKDIV_l3_atclken_sel_stat_END (19)
#define SOC_PMCTRL_L3_CLKDIV_l3_aclkenm_sel_stat_START (20)
#define SOC_PMCTRL_L3_CLKDIV_l3_aclkenm_sel_stat_END (22)
#define SOC_PMCTRL_L3_CLKDIV_l3_atclken_l_sel_stat_START (23)
#define SOC_PMCTRL_L3_CLKDIV_l3_atclken_l_sel_stat_END (23)
#define SOC_PMCTRL_L3_CLKDIV_l3_peri_clk_sel_stat_START (24)
#define SOC_PMCTRL_L3_CLKDIV_l3_peri_clk_sel_stat_END (25)
#define SOC_PMCTRL_L3_CLKDIV_l3_acp_clk_sel_stat_START (26)
#define SOC_PMCTRL_L3_CLKDIV_l3_acp_clk_sel_stat_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_apll_fbdiv : 12;
        unsigned int reserved : 20;
    } reg;
} SOC_PMCTRL_L3_CLKPROFILE0_UNION;
#endif
#define SOC_PMCTRL_L3_CLKPROFILE0_l3_apll_fbdiv_START (0)
#define SOC_PMCTRL_L3_CLKPROFILE0_l3_apll_fbdiv_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_apll_fracdiv : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_L3_CLKPROFILE0_1_UNION;
#endif
#define SOC_PMCTRL_L3_CLKPROFILE0_1_l3_apll_fracdiv_START (0)
#define SOC_PMCTRL_L3_CLKPROFILE0_1_l3_apll_fracdiv_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_pclkendbg_sel_prof : 2;
        unsigned int l3_atclken_sel_prof : 2;
        unsigned int l3_aclkenm_sel_prof : 3;
        unsigned int l3_atclken_l_sel_prof : 1;
        unsigned int l3_peri_clk_sel_prof : 2;
        unsigned int l3_acp_clk_sel_prof : 2;
        unsigned int reserved : 20;
    } reg;
} SOC_PMCTRL_L3_CLKPROFILE1_UNION;
#endif
#define SOC_PMCTRL_L3_CLKPROFILE1_l3_pclkendbg_sel_prof_START (0)
#define SOC_PMCTRL_L3_CLKPROFILE1_l3_pclkendbg_sel_prof_END (1)
#define SOC_PMCTRL_L3_CLKPROFILE1_l3_atclken_sel_prof_START (2)
#define SOC_PMCTRL_L3_CLKPROFILE1_l3_atclken_sel_prof_END (3)
#define SOC_PMCTRL_L3_CLKPROFILE1_l3_aclkenm_sel_prof_START (4)
#define SOC_PMCTRL_L3_CLKPROFILE1_l3_aclkenm_sel_prof_END (6)
#define SOC_PMCTRL_L3_CLKPROFILE1_l3_atclken_l_sel_prof_START (7)
#define SOC_PMCTRL_L3_CLKPROFILE1_l3_atclken_l_sel_prof_END (7)
#define SOC_PMCTRL_L3_CLKPROFILE1_l3_peri_clk_sel_prof_START (8)
#define SOC_PMCTRL_L3_CLKPROFILE1_l3_peri_clk_sel_prof_END (9)
#define SOC_PMCTRL_L3_CLKPROFILE1_l3_acp_clk_sel_prof_START (10)
#define SOC_PMCTRL_L3_CLKPROFILE1_l3_acp_clk_sel_prof_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_dvfs_vol_bypass : 1;
        unsigned int reserved_0 : 3;
        unsigned int l3_dvfs_apll_freq_bypass : 1;
        unsigned int reserved_1 : 3;
        unsigned int l3_dvfs_vol_updn : 1;
        unsigned int reserved_2 : 3;
        unsigned int l3_dvfs_apll_freq_updn : 1;
        unsigned int reserved_3 : 19;
    } reg;
} SOC_PMCTRL_L3_VOLMOD_UNION;
#endif
#define SOC_PMCTRL_L3_VOLMOD_l3_dvfs_vol_bypass_START (0)
#define SOC_PMCTRL_L3_VOLMOD_l3_dvfs_vol_bypass_END (0)
#define SOC_PMCTRL_L3_VOLMOD_l3_dvfs_apll_freq_bypass_START (4)
#define SOC_PMCTRL_L3_VOLMOD_l3_dvfs_apll_freq_bypass_END (4)
#define SOC_PMCTRL_L3_VOLMOD_l3_dvfs_vol_updn_START (8)
#define SOC_PMCTRL_L3_VOLMOD_l3_dvfs_vol_updn_END (8)
#define SOC_PMCTRL_L3_VOLMOD_l3_dvfs_apll_freq_updn_START (12)
#define SOC_PMCTRL_L3_VOLMOD_l3_dvfs_apll_freq_updn_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_vol_idx : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_L3_VOLPROFILE_UNION;
#endif
#define SOC_PMCTRL_L3_VOLPROFILE_l3_vol_idx_START (0)
#define SOC_PMCTRL_L3_VOLPROFILE_l3_vol_idx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_gdvfs_profile_updn : 1;
        unsigned int reserved_0 : 3;
        unsigned int l3_dvfs_mode : 2;
        unsigned int reserved_1 : 2;
        unsigned int l3_step_number : 8;
        unsigned int l3_clksel_prof : 1;
        unsigned int reserved_2 : 3;
        unsigned int l3_gdvfs_ctrl_apll : 1;
        unsigned int reserved_3 : 11;
    } reg;
} SOC_PMCTRL_L3_GDVFS_PROFILE0_UNION;
#endif
#define SOC_PMCTRL_L3_GDVFS_PROFILE0_l3_gdvfs_profile_updn_START (0)
#define SOC_PMCTRL_L3_GDVFS_PROFILE0_l3_gdvfs_profile_updn_END (0)
#define SOC_PMCTRL_L3_GDVFS_PROFILE0_l3_dvfs_mode_START (4)
#define SOC_PMCTRL_L3_GDVFS_PROFILE0_l3_dvfs_mode_END (5)
#define SOC_PMCTRL_L3_GDVFS_PROFILE0_l3_step_number_START (8)
#define SOC_PMCTRL_L3_GDVFS_PROFILE0_l3_step_number_END (15)
#define SOC_PMCTRL_L3_GDVFS_PROFILE0_l3_clksel_prof_START (16)
#define SOC_PMCTRL_L3_GDVFS_PROFILE0_l3_clksel_prof_END (16)
#define SOC_PMCTRL_L3_GDVFS_PROFILE0_l3_gdvfs_ctrl_apll_START (20)
#define SOC_PMCTRL_L3_GDVFS_PROFILE0_l3_gdvfs_ctrl_apll_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 2;
        unsigned int l3_apll_cfg_time : 12;
        unsigned int l3_clkdiv_time : 7;
        unsigned int reserved_2 : 9;
    } reg;
} SOC_PMCTRL_L3_GDVFS_PROFILE1_UNION;
#endif
#define SOC_PMCTRL_L3_GDVFS_PROFILE1_l3_apll_cfg_time_START (4)
#define SOC_PMCTRL_L3_GDVFS_PROFILE1_l3_apll_cfg_time_END (15)
#define SOC_PMCTRL_L3_GDVFS_PROFILE1_l3_clkdiv_time_START (16)
#define SOC_PMCTRL_L3_GDVFS_PROFILE1_l3_clkdiv_time_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_vol_idx_cfg : 8;
        unsigned int l3_vol_chg_sftreq_vote_en : 1;
        unsigned int l3_vol_chg_sftreq : 1;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 8;
        unsigned int reserved_2 : 12;
    } reg;
} SOC_PMCTRL_L3_VOLIDX_UNION;
#endif
#define SOC_PMCTRL_L3_VOLIDX_l3_vol_idx_cfg_START (0)
#define SOC_PMCTRL_L3_VOLIDX_l3_vol_idx_cfg_END (7)
#define SOC_PMCTRL_L3_VOLIDX_l3_vol_chg_sftreq_vote_en_START (8)
#define SOC_PMCTRL_L3_VOLIDX_l3_vol_chg_sftreq_vote_en_END (8)
#define SOC_PMCTRL_L3_VOLIDX_l3_vol_chg_sftreq_START (9)
#define SOC_PMCTRL_L3_VOLIDX_l3_vol_chg_sftreq_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int little_initial_vol_idx : 8;
        unsigned int little_initialvol_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 20;
    } reg;
} SOC_PMCTRL_LITTLE_INITIALVOL_UNION;
#endif
#define SOC_PMCTRL_LITTLE_INITIALVOL_little_initial_vol_idx_START (0)
#define SOC_PMCTRL_LITTLE_INITIALVOL_little_initial_vol_idx_END (7)
#define SOC_PMCTRL_LITTLE_INITIALVOL_little_initialvol_sftreq_START (8)
#define SOC_PMCTRL_LITTLE_INITIALVOL_little_initialvol_sftreq_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast_freq_fsm_cur_stat0 : 4;
        unsigned int bigfast_freq_fsm_cur_stat1 : 4;
        unsigned int bigfast_parallel_dvfs_fsm_cur_stat : 3;
        unsigned int reserved_0 : 1;
        unsigned int bigfast_vau_fsm_cur_stat : 8;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_BIGFAST_DVFS_STAT_UNION;
#endif
#define SOC_PMCTRL_BIGFAST_DVFS_STAT_bigfast_freq_fsm_cur_stat0_START (0)
#define SOC_PMCTRL_BIGFAST_DVFS_STAT_bigfast_freq_fsm_cur_stat0_END (3)
#define SOC_PMCTRL_BIGFAST_DVFS_STAT_bigfast_freq_fsm_cur_stat1_START (4)
#define SOC_PMCTRL_BIGFAST_DVFS_STAT_bigfast_freq_fsm_cur_stat1_END (7)
#define SOC_PMCTRL_BIGFAST_DVFS_STAT_bigfast_parallel_dvfs_fsm_cur_stat_START (8)
#define SOC_PMCTRL_BIGFAST_DVFS_STAT_bigfast_parallel_dvfs_fsm_cur_stat_END (10)
#define SOC_PMCTRL_BIGFAST_DVFS_STAT_bigfast_vau_fsm_cur_stat_START (12)
#define SOC_PMCTRL_BIGFAST_DVFS_STAT_bigfast_vau_fsm_cur_stat_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast_initial_pmu_vol_idx_ocldo : 8;
        unsigned int bigfast_initial_pmu_vol_idx_pmu : 8;
        unsigned int bigfast_initial_pmu_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_BIGFAST_INITIALVOL_UNION;
#endif
#define SOC_PMCTRL_BIGFAST_INITIALVOL_bigfast_initial_pmu_vol_idx_ocldo_START (0)
#define SOC_PMCTRL_BIGFAST_INITIALVOL_bigfast_initial_pmu_vol_idx_ocldo_END (7)
#define SOC_PMCTRL_BIGFAST_INITIALVOL_bigfast_initial_pmu_vol_idx_pmu_START (8)
#define SOC_PMCTRL_BIGFAST_INITIALVOL_bigfast_initial_pmu_vol_idx_pmu_END (15)
#define SOC_PMCTRL_BIGFAST_INITIALVOL_bigfast_initial_pmu_vol_req_START (16)
#define SOC_PMCTRL_BIGFAST_INITIALVOL_bigfast_initial_pmu_vol_req_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast_pmu_sel : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_BIGFAST_PMUSEL_UNION;
#endif
#define SOC_PMCTRL_BIGFAST_PMUSEL_bigfast_pmu_sel_START (0)
#define SOC_PMCTRL_BIGFAST_PMUSEL_bigfast_pmu_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast_gdvfs_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BIGFAST_GDVFS_EN_UNION;
#endif
#define SOC_PMCTRL_BIGFAST_GDVFS_EN_bigfast_gdvfs_en_START (0)
#define SOC_PMCTRL_BIGFAST_GDVFS_EN_bigfast_gdvfs_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_clk_sel : 2;
        unsigned int reserved_0 : 2;
        unsigned int bigfast0_clk_sel_stat : 2;
        unsigned int reserved_1 : 26;
    } reg;
} SOC_PMCTRL_BIGFAST0_CLKSEL_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_CLKSEL_bigfast0_clk_sel_START (0)
#define SOC_PMCTRL_BIGFAST0_CLKSEL_bigfast0_clk_sel_END (1)
#define SOC_PMCTRL_BIGFAST0_CLKSEL_bigfast0_clk_sel_stat_START (4)
#define SOC_PMCTRL_BIGFAST0_CLKSEL_bigfast0_clk_sel_stat_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_apll_fbdiv : 12;
        unsigned int reserved : 20;
    } reg;
} SOC_PMCTRL_BIGFAST0_CLKPROFILE0_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE0_bigfast0_apll_fbdiv_START (0)
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE0_bigfast0_apll_fbdiv_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_apll_fracdiv : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_BIGFAST0_CLKPROFILE0_1_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE0_1_bigfast0_apll_fracdiv_START (0)
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE0_1_bigfast0_apll_fracdiv_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_dvfs_vol_bypass : 1;
        unsigned int reserved_0 : 3;
        unsigned int bigfast0_dvfs_apll_freq_bypass : 1;
        unsigned int reserved_1 : 3;
        unsigned int bigfast0_dvfs_vol_updn : 1;
        unsigned int reserved_2 : 3;
        unsigned int bigfast0_dvfs_apll_freq_updn : 1;
        unsigned int reserved_3 : 19;
    } reg;
} SOC_PMCTRL_BIGFAST0_VOLMOD_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_VOLMOD_bigfast0_dvfs_vol_bypass_START (0)
#define SOC_PMCTRL_BIGFAST0_VOLMOD_bigfast0_dvfs_vol_bypass_END (0)
#define SOC_PMCTRL_BIGFAST0_VOLMOD_bigfast0_dvfs_apll_freq_bypass_START (4)
#define SOC_PMCTRL_BIGFAST0_VOLMOD_bigfast0_dvfs_apll_freq_bypass_END (4)
#define SOC_PMCTRL_BIGFAST0_VOLMOD_bigfast0_dvfs_vol_updn_START (8)
#define SOC_PMCTRL_BIGFAST0_VOLMOD_bigfast0_dvfs_vol_updn_END (8)
#define SOC_PMCTRL_BIGFAST0_VOLMOD_bigfast0_dvfs_apll_freq_updn_START (12)
#define SOC_PMCTRL_BIGFAST0_VOLMOD_bigfast0_dvfs_apll_freq_updn_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_vol_idx_ocldo : 8;
        unsigned int bigfast0_vol_idx_pmu : 8;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_BIGFAST0_VOLPROFILE_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_VOLPROFILE_bigfast0_vol_idx_ocldo_START (0)
#define SOC_PMCTRL_BIGFAST0_VOLPROFILE_bigfast0_vol_idx_ocldo_END (7)
#define SOC_PMCTRL_BIGFAST0_VOLPROFILE_bigfast0_vol_idx_pmu_START (8)
#define SOC_PMCTRL_BIGFAST0_VOLPROFILE_bigfast0_vol_idx_pmu_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_pmu_vol_up_vol_hold_time : 20;
        unsigned int bigfast0_pmu_vol_up_step_time : 12;
    } reg;
} SOC_PMCTRL_BIGFAST0_PMU_UP_TIME_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_PMU_UP_TIME_bigfast0_pmu_vol_up_vol_hold_time_START (0)
#define SOC_PMCTRL_BIGFAST0_PMU_UP_TIME_bigfast0_pmu_vol_up_vol_hold_time_END (19)
#define SOC_PMCTRL_BIGFAST0_PMU_UP_TIME_bigfast0_pmu_vol_up_step_time_START (20)
#define SOC_PMCTRL_BIGFAST0_PMU_UP_TIME_bigfast0_pmu_vol_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_pmu_vol_dn_vol_hold_time : 20;
        unsigned int bigfast0_pmu_vol_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_BIGFAST0_PMU_DN_TIME_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_PMU_DN_TIME_bigfast0_pmu_vol_dn_vol_hold_time_START (0)
#define SOC_PMCTRL_BIGFAST0_PMU_DN_TIME_bigfast0_pmu_vol_dn_vol_hold_time_END (19)
#define SOC_PMCTRL_BIGFAST0_PMU_DN_TIME_bigfast0_pmu_vol_dn_step_time_START (20)
#define SOC_PMCTRL_BIGFAST0_PMU_DN_TIME_bigfast0_pmu_vol_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_gdvfs_profile_updn : 1;
        unsigned int reserved_0 : 3;
        unsigned int bigfast0_dvfs_mode : 2;
        unsigned int reserved_1 : 2;
        unsigned int bigfast0_step_number : 8;
        unsigned int bigfast0_clksel_prof : 2;
        unsigned int reserved_2 : 2;
        unsigned int bigfast0_gdvfs_ctrl_apll : 1;
        unsigned int reserved_3 : 11;
    } reg;
} SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE0_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE0_bigfast0_gdvfs_profile_updn_START (0)
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE0_bigfast0_gdvfs_profile_updn_END (0)
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE0_bigfast0_dvfs_mode_START (4)
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE0_bigfast0_dvfs_mode_END (5)
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE0_bigfast0_step_number_START (8)
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE0_bigfast0_step_number_END (15)
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE0_bigfast0_clksel_prof_START (16)
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE0_bigfast0_clksel_prof_END (17)
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE0_bigfast0_gdvfs_ctrl_apll_START (20)
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE0_bigfast0_gdvfs_ctrl_apll_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 2;
        unsigned int bigfast0_apll_cfg_time : 12;
        unsigned int bigfast0_clkdiv_time : 7;
        unsigned int reserved_2 : 9;
    } reg;
} SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE1_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE1_bigfast0_apll_cfg_time_START (4)
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE1_bigfast0_apll_cfg_time_END (15)
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE1_bigfast0_clkdiv_time_START (16)
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE1_bigfast0_clkdiv_time_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_vol_idx_cfg_ocldo : 8;
        unsigned int bigfast0_vol_idx_cfg_pmu : 8;
        unsigned int bigfast0_vol_chg_sftreq_vote_en : 1;
        unsigned int bigfast0_vol_chg_sftreq : 1;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_BIGFAST0_VOLIDX_SFTCFG_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_VOLIDX_SFTCFG_bigfast0_vol_idx_cfg_ocldo_START (0)
#define SOC_PMCTRL_BIGFAST0_VOLIDX_SFTCFG_bigfast0_vol_idx_cfg_ocldo_END (7)
#define SOC_PMCTRL_BIGFAST0_VOLIDX_SFTCFG_bigfast0_vol_idx_cfg_pmu_START (8)
#define SOC_PMCTRL_BIGFAST0_VOLIDX_SFTCFG_bigfast0_vol_idx_cfg_pmu_END (15)
#define SOC_PMCTRL_BIGFAST0_VOLIDX_SFTCFG_bigfast0_vol_chg_sftreq_vote_en_START (16)
#define SOC_PMCTRL_BIGFAST0_VOLIDX_SFTCFG_bigfast0_vol_chg_sftreq_vote_en_END (16)
#define SOC_PMCTRL_BIGFAST0_VOLIDX_SFTCFG_bigfast0_vol_chg_sftreq_START (17)
#define SOC_PMCTRL_BIGFAST0_VOLIDX_SFTCFG_bigfast0_vol_chg_sftreq_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int bigfast0_fbdiv_sw_initial : 12;
        unsigned int bigfast0_postdiv1_sw_initial : 3;
        unsigned int bigfast0_postdiv2_sw_initial : 3;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_PMCTRL_BIGFAST0_APLL_INITIAL0_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_APLL_INITIAL0_bigfast0_fbdiv_sw_initial_START (8)
#define SOC_PMCTRL_BIGFAST0_APLL_INITIAL0_bigfast0_fbdiv_sw_initial_END (19)
#define SOC_PMCTRL_BIGFAST0_APLL_INITIAL0_bigfast0_postdiv1_sw_initial_START (20)
#define SOC_PMCTRL_BIGFAST0_APLL_INITIAL0_bigfast0_postdiv1_sw_initial_END (22)
#define SOC_PMCTRL_BIGFAST0_APLL_INITIAL0_bigfast0_postdiv2_sw_initial_START (23)
#define SOC_PMCTRL_BIGFAST0_APLL_INITIAL0_bigfast0_postdiv2_sw_initial_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_fracdiv_sw_initial : 24;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 5;
    } reg;
} SOC_PMCTRL_BIGFAST0_APLL_INITIAL1_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_APLL_INITIAL1_bigfast0_fracdiv_sw_initial_START (0)
#define SOC_PMCTRL_BIGFAST0_APLL_INITIAL1_bigfast0_fracdiv_sw_initial_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast1_clk_sel : 2;
        unsigned int reserved_0 : 2;
        unsigned int bigfast1_clk_sel_stat : 2;
        unsigned int reserved_1 : 26;
    } reg;
} SOC_PMCTRL_BIGFAST1_CLKSEL_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_CLKSEL_bigfast1_clk_sel_START (0)
#define SOC_PMCTRL_BIGFAST1_CLKSEL_bigfast1_clk_sel_END (1)
#define SOC_PMCTRL_BIGFAST1_CLKSEL_bigfast1_clk_sel_stat_START (4)
#define SOC_PMCTRL_BIGFAST1_CLKSEL_bigfast1_clk_sel_stat_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast1_apll_fbdiv : 12;
        unsigned int reserved : 20;
    } reg;
} SOC_PMCTRL_BIGFAST1_CLKPROFILE0_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE0_bigfast1_apll_fbdiv_START (0)
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE0_bigfast1_apll_fbdiv_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast1_apll_fracdiv : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_BIGFAST1_CLKPROFILE0_1_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE0_1_bigfast1_apll_fracdiv_START (0)
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE0_1_bigfast1_apll_fracdiv_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast1_dvfs_vol_bypass : 1;
        unsigned int reserved_0 : 3;
        unsigned int bigfast1_dvfs_apll_freq_bypass : 1;
        unsigned int reserved_1 : 3;
        unsigned int bigfast1_dvfs_vol_updn : 1;
        unsigned int reserved_2 : 3;
        unsigned int bigfast1_dvfs_apll_freq_updn : 1;
        unsigned int reserved_3 : 19;
    } reg;
} SOC_PMCTRL_BIGFAST1_VOLMOD_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_VOLMOD_bigfast1_dvfs_vol_bypass_START (0)
#define SOC_PMCTRL_BIGFAST1_VOLMOD_bigfast1_dvfs_vol_bypass_END (0)
#define SOC_PMCTRL_BIGFAST1_VOLMOD_bigfast1_dvfs_apll_freq_bypass_START (4)
#define SOC_PMCTRL_BIGFAST1_VOLMOD_bigfast1_dvfs_apll_freq_bypass_END (4)
#define SOC_PMCTRL_BIGFAST1_VOLMOD_bigfast1_dvfs_vol_updn_START (8)
#define SOC_PMCTRL_BIGFAST1_VOLMOD_bigfast1_dvfs_vol_updn_END (8)
#define SOC_PMCTRL_BIGFAST1_VOLMOD_bigfast1_dvfs_apll_freq_updn_START (12)
#define SOC_PMCTRL_BIGFAST1_VOLMOD_bigfast1_dvfs_apll_freq_updn_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast1_vol_idx_ocldo : 8;
        unsigned int bigfast1_vol_idx_pmu : 8;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_BIGFAST1_VOLPROFILE_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_VOLPROFILE_bigfast1_vol_idx_ocldo_START (0)
#define SOC_PMCTRL_BIGFAST1_VOLPROFILE_bigfast1_vol_idx_ocldo_END (7)
#define SOC_PMCTRL_BIGFAST1_VOLPROFILE_bigfast1_vol_idx_pmu_START (8)
#define SOC_PMCTRL_BIGFAST1_VOLPROFILE_bigfast1_vol_idx_pmu_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast1_gdvfs_profile_updn : 1;
        unsigned int reserved_0 : 3;
        unsigned int bigfast1_dvfs_mode : 2;
        unsigned int reserved_1 : 2;
        unsigned int bigfast1_step_number : 8;
        unsigned int bigfast1_clksel_prof : 2;
        unsigned int reserved_2 : 2;
        unsigned int bigfast1_gdvfs_ctrl_apll : 1;
        unsigned int reserved_3 : 11;
    } reg;
} SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE0_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE0_bigfast1_gdvfs_profile_updn_START (0)
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE0_bigfast1_gdvfs_profile_updn_END (0)
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE0_bigfast1_dvfs_mode_START (4)
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE0_bigfast1_dvfs_mode_END (5)
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE0_bigfast1_step_number_START (8)
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE0_bigfast1_step_number_END (15)
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE0_bigfast1_clksel_prof_START (16)
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE0_bigfast1_clksel_prof_END (17)
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE0_bigfast1_gdvfs_ctrl_apll_START (20)
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE0_bigfast1_gdvfs_ctrl_apll_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 2;
        unsigned int bigfast1_apll_cfg_time : 12;
        unsigned int bigfast1_clkdiv_time : 7;
        unsigned int reserved_2 : 9;
    } reg;
} SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE1_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE1_bigfast1_apll_cfg_time_START (4)
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE1_bigfast1_apll_cfg_time_END (15)
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE1_bigfast1_clkdiv_time_START (16)
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE1_bigfast1_clkdiv_time_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast1_vol_idx_cfg_ocldo : 8;
        unsigned int bigfast1_vol_idx_cfg_pmu : 8;
        unsigned int bigfast1_vol_chg_sftreq_vote_en : 1;
        unsigned int bigfast1_vol_chg_sftreq : 1;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_BIGFAST1_VOLIDX_SFTCFG_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_VOLIDX_SFTCFG_bigfast1_vol_idx_cfg_ocldo_START (0)
#define SOC_PMCTRL_BIGFAST1_VOLIDX_SFTCFG_bigfast1_vol_idx_cfg_ocldo_END (7)
#define SOC_PMCTRL_BIGFAST1_VOLIDX_SFTCFG_bigfast1_vol_idx_cfg_pmu_START (8)
#define SOC_PMCTRL_BIGFAST1_VOLIDX_SFTCFG_bigfast1_vol_idx_cfg_pmu_END (15)
#define SOC_PMCTRL_BIGFAST1_VOLIDX_SFTCFG_bigfast1_vol_chg_sftreq_vote_en_START (16)
#define SOC_PMCTRL_BIGFAST1_VOLIDX_SFTCFG_bigfast1_vol_chg_sftreq_vote_en_END (16)
#define SOC_PMCTRL_BIGFAST1_VOLIDX_SFTCFG_bigfast1_vol_chg_sftreq_START (17)
#define SOC_PMCTRL_BIGFAST1_VOLIDX_SFTCFG_bigfast1_vol_chg_sftreq_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int bigfast1_fbdiv_sw_initial : 12;
        unsigned int bigfast1_postdiv1_sw_initial : 3;
        unsigned int bigfast1_postdiv2_sw_initial : 3;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_PMCTRL_BIGFAST1_APLL_INITIAL0_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_APLL_INITIAL0_bigfast1_fbdiv_sw_initial_START (8)
#define SOC_PMCTRL_BIGFAST1_APLL_INITIAL0_bigfast1_fbdiv_sw_initial_END (19)
#define SOC_PMCTRL_BIGFAST1_APLL_INITIAL0_bigfast1_postdiv1_sw_initial_START (20)
#define SOC_PMCTRL_BIGFAST1_APLL_INITIAL0_bigfast1_postdiv1_sw_initial_END (22)
#define SOC_PMCTRL_BIGFAST1_APLL_INITIAL0_bigfast1_postdiv2_sw_initial_START (23)
#define SOC_PMCTRL_BIGFAST1_APLL_INITIAL0_bigfast1_postdiv2_sw_initial_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast1_fracdiv_sw_initial : 24;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 5;
    } reg;
} SOC_PMCTRL_BIGFAST1_APLL_INITIAL1_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_APLL_INITIAL1_bigfast1_fracdiv_sw_initial_START (0)
#define SOC_PMCTRL_BIGFAST1_APLL_INITIAL1_bigfast1_fracdiv_sw_initial_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 8;
        unsigned int bigfast_phase_protect_vol_ocldo : 8;
        unsigned int bigfast_phase_protect_vol_pmu : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_PMCTRL_BIGFAST_VAU_DBG_STAT_UNION;
#endif
#define SOC_PMCTRL_BIGFAST_VAU_DBG_STAT_bigfast_phase_protect_vol_ocldo_START (8)
#define SOC_PMCTRL_BIGFAST_VAU_DBG_STAT_bigfast_phase_protect_vol_ocldo_END (15)
#define SOC_PMCTRL_BIGFAST_VAU_DBG_STAT_bigfast_phase_protect_vol_pmu_START (16)
#define SOC_PMCTRL_BIGFAST_VAU_DBG_STAT_bigfast_phase_protect_vol_pmu_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast_cur_max_buck_vol_ocldo : 8;
        unsigned int bigfast_cur_max_buck_vol_pmu : 8;
        unsigned int bigfast_cur_max_buck_vol_tmp_ocldo : 8;
        unsigned int bigfast_cur_max_buck_vol_tmp_pmu : 8;
    } reg;
} SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT0_UNION;
#endif
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT0_bigfast_cur_max_buck_vol_ocldo_START (0)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT0_bigfast_cur_max_buck_vol_ocldo_END (7)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT0_bigfast_cur_max_buck_vol_pmu_START (8)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT0_bigfast_cur_max_buck_vol_pmu_END (15)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT0_bigfast_cur_max_buck_vol_tmp_ocldo_START (16)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT0_bigfast_cur_max_buck_vol_tmp_ocldo_END (23)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT0_bigfast_cur_max_buck_vol_tmp_pmu_START (24)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT0_bigfast_cur_max_buck_vol_tmp_pmu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast_nxt_max_buck_vol_ocldo : 8;
        unsigned int bigfast_nxt_max_buck_vol_pmu : 8;
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT1_UNION;
#endif
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT1_bigfast_nxt_max_buck_vol_ocldo_START (0)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT1_bigfast_nxt_max_buck_vol_ocldo_END (7)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT1_bigfast_nxt_max_buck_vol_pmu_START (8)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT1_bigfast_nxt_max_buck_vol_pmu_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast_cur_max_ocldo_vol_ocldo : 8;
        unsigned int bigfast_cur_max_ocldo_vol_pmu : 8;
        unsigned int bigfast_nxt_max_ocldo_vol_ocldo : 8;
        unsigned int bigfast_nxt_max_ocldo_vol_pmu : 8;
    } reg;
} SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT2_UNION;
#endif
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT2_bigfast_cur_max_ocldo_vol_ocldo_START (0)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT2_bigfast_cur_max_ocldo_vol_ocldo_END (7)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT2_bigfast_cur_max_ocldo_vol_pmu_START (8)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT2_bigfast_cur_max_ocldo_vol_pmu_END (15)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT2_bigfast_nxt_max_ocldo_vol_ocldo_START (16)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT2_bigfast_nxt_max_ocldo_vol_ocldo_END (23)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT2_bigfast_nxt_max_ocldo_vol_pmu_START (24)
#define SOC_PMCTRL_BIGFAST_MAX_VOL_DBG_STAT2_bigfast_nxt_max_ocldo_vol_pmu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast_unified_vol_idx_stat0_ocldo : 8;
        unsigned int bigfast_unified_vol_idx_stat0_pmu : 8;
        unsigned int bigfast_unified_vol_idx_stat1_ocldo : 8;
        unsigned int bigfast_unified_vol_idx_stat1_pmu : 8;
    } reg;
} SOC_PMCTRL_BIGFAST_UNIFIED_VOL_DBG_STAT0_UNION;
#endif
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_DBG_STAT0_bigfast_unified_vol_idx_stat0_ocldo_START (0)
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_DBG_STAT0_bigfast_unified_vol_idx_stat0_ocldo_END (7)
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_DBG_STAT0_bigfast_unified_vol_idx_stat0_pmu_START (8)
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_DBG_STAT0_bigfast_unified_vol_idx_stat0_pmu_END (15)
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_DBG_STAT0_bigfast_unified_vol_idx_stat1_ocldo_START (16)
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_DBG_STAT0_bigfast_unified_vol_idx_stat1_ocldo_END (23)
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_DBG_STAT0_bigfast_unified_vol_idx_stat1_pmu_START (24)
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_DBG_STAT0_bigfast_unified_vol_idx_stat1_pmu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast_unified_pmu_vol_vol_idx_stat_ocldo : 8;
        unsigned int bigfast_unified_pmu_vol_vol_idx_stat_pmu : 8;
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_PMCTRL_BIGFAST_UNIFIED_VOL_DBG_STAT1_UNION;
#endif
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_DBG_STAT1_bigfast_unified_pmu_vol_vol_idx_stat_ocldo_START (0)
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_DBG_STAT1_bigfast_unified_pmu_vol_vol_idx_stat_ocldo_END (7)
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_DBG_STAT1_bigfast_unified_pmu_vol_vol_idx_stat_pmu_START (8)
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_DBG_STAT1_bigfast_unified_pmu_vol_vol_idx_stat_pmu_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast_cur_pmu_vol_idx_stat0_ocldo : 8;
        unsigned int bigfast_cur_pmu_vol_idx_stat0_pmu : 8;
        unsigned int bigfast_cur_pmu_vol_idx_stat1_ocldo : 8;
        unsigned int bigfast_cur_pmu_vol_idx_stat1_pmu : 8;
    } reg;
} SOC_PMCTRL_BIGFAST_CUR_VOL_DBG_STAT0_UNION;
#endif
#define SOC_PMCTRL_BIGFAST_CUR_VOL_DBG_STAT0_bigfast_cur_pmu_vol_idx_stat0_ocldo_START (0)
#define SOC_PMCTRL_BIGFAST_CUR_VOL_DBG_STAT0_bigfast_cur_pmu_vol_idx_stat0_ocldo_END (7)
#define SOC_PMCTRL_BIGFAST_CUR_VOL_DBG_STAT0_bigfast_cur_pmu_vol_idx_stat0_pmu_START (8)
#define SOC_PMCTRL_BIGFAST_CUR_VOL_DBG_STAT0_bigfast_cur_pmu_vol_idx_stat0_pmu_END (15)
#define SOC_PMCTRL_BIGFAST_CUR_VOL_DBG_STAT0_bigfast_cur_pmu_vol_idx_stat1_ocldo_START (16)
#define SOC_PMCTRL_BIGFAST_CUR_VOL_DBG_STAT0_bigfast_cur_pmu_vol_idx_stat1_ocldo_END (23)
#define SOC_PMCTRL_BIGFAST_CUR_VOL_DBG_STAT0_bigfast_cur_pmu_vol_idx_stat1_pmu_START (24)
#define SOC_PMCTRL_BIGFAST_CUR_VOL_DBG_STAT0_bigfast_cur_pmu_vol_idx_stat1_pmu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow_freq_fsm_cur_stat0 : 4;
        unsigned int bigslow_freq_fsm_cur_stat1 : 4;
        unsigned int bigslow_parallel_dvfs_fsm_cur_stat : 3;
        unsigned int reserved_0 : 1;
        unsigned int bigslow_vau_fsm_cur_stat : 8;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_BIGSLOW_DVFS_STAT_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW_DVFS_STAT_bigslow_freq_fsm_cur_stat0_START (0)
#define SOC_PMCTRL_BIGSLOW_DVFS_STAT_bigslow_freq_fsm_cur_stat0_END (3)
#define SOC_PMCTRL_BIGSLOW_DVFS_STAT_bigslow_freq_fsm_cur_stat1_START (4)
#define SOC_PMCTRL_BIGSLOW_DVFS_STAT_bigslow_freq_fsm_cur_stat1_END (7)
#define SOC_PMCTRL_BIGSLOW_DVFS_STAT_bigslow_parallel_dvfs_fsm_cur_stat_START (8)
#define SOC_PMCTRL_BIGSLOW_DVFS_STAT_bigslow_parallel_dvfs_fsm_cur_stat_END (10)
#define SOC_PMCTRL_BIGSLOW_DVFS_STAT_bigslow_vau_fsm_cur_stat_START (12)
#define SOC_PMCTRL_BIGSLOW_DVFS_STAT_bigslow_vau_fsm_cur_stat_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow_initial_pmu_vol_idx_ocldo : 8;
        unsigned int bigslow_initial_pmu_vol_idx_pmu : 8;
        unsigned int bigslow_initial_pmu_vol_req : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_BIGSLOW_INITIALVOL_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW_INITIALVOL_bigslow_initial_pmu_vol_idx_ocldo_START (0)
#define SOC_PMCTRL_BIGSLOW_INITIALVOL_bigslow_initial_pmu_vol_idx_ocldo_END (7)
#define SOC_PMCTRL_BIGSLOW_INITIALVOL_bigslow_initial_pmu_vol_idx_pmu_START (8)
#define SOC_PMCTRL_BIGSLOW_INITIALVOL_bigslow_initial_pmu_vol_idx_pmu_END (15)
#define SOC_PMCTRL_BIGSLOW_INITIALVOL_bigslow_initial_pmu_vol_req_START (16)
#define SOC_PMCTRL_BIGSLOW_INITIALVOL_bigslow_initial_pmu_vol_req_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow_pmu_sel : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_BIGSLOW_PMUSEL_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW_PMUSEL_bigslow_pmu_sel_START (0)
#define SOC_PMCTRL_BIGSLOW_PMUSEL_bigslow_pmu_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow_gdvfs_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BIGSLOW_GDVFS_EN_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW_GDVFS_EN_bigslow_gdvfs_en_START (0)
#define SOC_PMCTRL_BIGSLOW_GDVFS_EN_bigslow_gdvfs_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_clk_sel : 2;
        unsigned int reserved_0 : 2;
        unsigned int bigslow0_clk_sel_stat : 2;
        unsigned int reserved_1 : 26;
    } reg;
} SOC_PMCTRL_BIGSLOW0_CLKSEL_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_CLKSEL_bigslow0_clk_sel_START (0)
#define SOC_PMCTRL_BIGSLOW0_CLKSEL_bigslow0_clk_sel_END (1)
#define SOC_PMCTRL_BIGSLOW0_CLKSEL_bigslow0_clk_sel_stat_START (4)
#define SOC_PMCTRL_BIGSLOW0_CLKSEL_bigslow0_clk_sel_stat_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_apll_fbdiv : 12;
        unsigned int reserved : 20;
    } reg;
} SOC_PMCTRL_BIGSLOW0_CLKPROFILE0_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE0_bigslow0_apll_fbdiv_START (0)
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE0_bigslow0_apll_fbdiv_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_apll_fracdiv : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_BIGSLOW0_CLKPROFILE0_1_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE0_1_bigslow0_apll_fracdiv_START (0)
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE0_1_bigslow0_apll_fracdiv_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_dvfs_vol_bypass : 1;
        unsigned int reserved_0 : 3;
        unsigned int bigslow0_dvfs_apll_freq_bypass : 1;
        unsigned int reserved_1 : 3;
        unsigned int bigslow0_dvfs_vol_updn : 1;
        unsigned int reserved_2 : 3;
        unsigned int bigslow0_dvfs_apll_freq_updn : 1;
        unsigned int reserved_3 : 19;
    } reg;
} SOC_PMCTRL_BIGSLOW0_VOLMOD_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_VOLMOD_bigslow0_dvfs_vol_bypass_START (0)
#define SOC_PMCTRL_BIGSLOW0_VOLMOD_bigslow0_dvfs_vol_bypass_END (0)
#define SOC_PMCTRL_BIGSLOW0_VOLMOD_bigslow0_dvfs_apll_freq_bypass_START (4)
#define SOC_PMCTRL_BIGSLOW0_VOLMOD_bigslow0_dvfs_apll_freq_bypass_END (4)
#define SOC_PMCTRL_BIGSLOW0_VOLMOD_bigslow0_dvfs_vol_updn_START (8)
#define SOC_PMCTRL_BIGSLOW0_VOLMOD_bigslow0_dvfs_vol_updn_END (8)
#define SOC_PMCTRL_BIGSLOW0_VOLMOD_bigslow0_dvfs_apll_freq_updn_START (12)
#define SOC_PMCTRL_BIGSLOW0_VOLMOD_bigslow0_dvfs_apll_freq_updn_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_vol_idx_ocldo : 8;
        unsigned int bigslow0_vol_idx_pmu : 8;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_BIGSLOW0_VOLPROFILE_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_VOLPROFILE_bigslow0_vol_idx_ocldo_START (0)
#define SOC_PMCTRL_BIGSLOW0_VOLPROFILE_bigslow0_vol_idx_ocldo_END (7)
#define SOC_PMCTRL_BIGSLOW0_VOLPROFILE_bigslow0_vol_idx_pmu_START (8)
#define SOC_PMCTRL_BIGSLOW0_VOLPROFILE_bigslow0_vol_idx_pmu_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_pmu_vol_up_vol_hold_time : 20;
        unsigned int bigslow0_pmu_vol_up_step_time : 12;
    } reg;
} SOC_PMCTRL_BIGSLOW0_PMU_UP_TIME_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_PMU_UP_TIME_bigslow0_pmu_vol_up_vol_hold_time_START (0)
#define SOC_PMCTRL_BIGSLOW0_PMU_UP_TIME_bigslow0_pmu_vol_up_vol_hold_time_END (19)
#define SOC_PMCTRL_BIGSLOW0_PMU_UP_TIME_bigslow0_pmu_vol_up_step_time_START (20)
#define SOC_PMCTRL_BIGSLOW0_PMU_UP_TIME_bigslow0_pmu_vol_up_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_pmu_vol_dn_vol_hold_time : 20;
        unsigned int bigslow0_pmu_vol_dn_step_time : 12;
    } reg;
} SOC_PMCTRL_BIGSLOW0_PMU_DN_TIME_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_PMU_DN_TIME_bigslow0_pmu_vol_dn_vol_hold_time_START (0)
#define SOC_PMCTRL_BIGSLOW0_PMU_DN_TIME_bigslow0_pmu_vol_dn_vol_hold_time_END (19)
#define SOC_PMCTRL_BIGSLOW0_PMU_DN_TIME_bigslow0_pmu_vol_dn_step_time_START (20)
#define SOC_PMCTRL_BIGSLOW0_PMU_DN_TIME_bigslow0_pmu_vol_dn_step_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_gdvfs_profile_updn : 1;
        unsigned int reserved_0 : 3;
        unsigned int bigslow0_dvfs_mode : 2;
        unsigned int reserved_1 : 2;
        unsigned int bigslow0_step_number : 8;
        unsigned int bigslow0_clksel_prof : 2;
        unsigned int reserved_2 : 2;
        unsigned int bigslow0_gdvfs_ctrl_apll : 1;
        unsigned int reserved_3 : 11;
    } reg;
} SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE0_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE0_bigslow0_gdvfs_profile_updn_START (0)
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE0_bigslow0_gdvfs_profile_updn_END (0)
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE0_bigslow0_dvfs_mode_START (4)
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE0_bigslow0_dvfs_mode_END (5)
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE0_bigslow0_step_number_START (8)
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE0_bigslow0_step_number_END (15)
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE0_bigslow0_clksel_prof_START (16)
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE0_bigslow0_clksel_prof_END (17)
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE0_bigslow0_gdvfs_ctrl_apll_START (20)
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE0_bigslow0_gdvfs_ctrl_apll_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 2;
        unsigned int bigslow0_apll_cfg_time : 12;
        unsigned int bigslow0_clkdiv_time : 7;
        unsigned int reserved_2 : 9;
    } reg;
} SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE1_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE1_bigslow0_apll_cfg_time_START (4)
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE1_bigslow0_apll_cfg_time_END (15)
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE1_bigslow0_clkdiv_time_START (16)
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE1_bigslow0_clkdiv_time_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_vol_idx_cfg_ocldo : 8;
        unsigned int bigslow0_vol_idx_cfg_pmu : 8;
        unsigned int bigslow0_vol_chg_sftreq_vote_en : 1;
        unsigned int bigslow0_vol_chg_sftreq : 1;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_BIGSLOW0_VOLIDX_SFTCFG_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_VOLIDX_SFTCFG_bigslow0_vol_idx_cfg_ocldo_START (0)
#define SOC_PMCTRL_BIGSLOW0_VOLIDX_SFTCFG_bigslow0_vol_idx_cfg_ocldo_END (7)
#define SOC_PMCTRL_BIGSLOW0_VOLIDX_SFTCFG_bigslow0_vol_idx_cfg_pmu_START (8)
#define SOC_PMCTRL_BIGSLOW0_VOLIDX_SFTCFG_bigslow0_vol_idx_cfg_pmu_END (15)
#define SOC_PMCTRL_BIGSLOW0_VOLIDX_SFTCFG_bigslow0_vol_chg_sftreq_vote_en_START (16)
#define SOC_PMCTRL_BIGSLOW0_VOLIDX_SFTCFG_bigslow0_vol_chg_sftreq_vote_en_END (16)
#define SOC_PMCTRL_BIGSLOW0_VOLIDX_SFTCFG_bigslow0_vol_chg_sftreq_START (17)
#define SOC_PMCTRL_BIGSLOW0_VOLIDX_SFTCFG_bigslow0_vol_chg_sftreq_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int bigslow0_fbdiv_sw_initial : 12;
        unsigned int bigslow0_postdiv1_sw_initial : 3;
        unsigned int bigslow0_postdiv2_sw_initial : 3;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_PMCTRL_BIGSLOW0_APLL_INITIAL0_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_APLL_INITIAL0_bigslow0_fbdiv_sw_initial_START (8)
#define SOC_PMCTRL_BIGSLOW0_APLL_INITIAL0_bigslow0_fbdiv_sw_initial_END (19)
#define SOC_PMCTRL_BIGSLOW0_APLL_INITIAL0_bigslow0_postdiv1_sw_initial_START (20)
#define SOC_PMCTRL_BIGSLOW0_APLL_INITIAL0_bigslow0_postdiv1_sw_initial_END (22)
#define SOC_PMCTRL_BIGSLOW0_APLL_INITIAL0_bigslow0_postdiv2_sw_initial_START (23)
#define SOC_PMCTRL_BIGSLOW0_APLL_INITIAL0_bigslow0_postdiv2_sw_initial_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_fracdiv_sw_initial : 24;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 5;
    } reg;
} SOC_PMCTRL_BIGSLOW0_APLL_INITIAL1_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_APLL_INITIAL1_bigslow0_fracdiv_sw_initial_START (0)
#define SOC_PMCTRL_BIGSLOW0_APLL_INITIAL1_bigslow0_fracdiv_sw_initial_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_clk_sel : 2;
        unsigned int reserved_0 : 2;
        unsigned int bigslow1_clk_sel_stat : 2;
        unsigned int reserved_1 : 26;
    } reg;
} SOC_PMCTRL_BIGSLOW1_CLKSEL_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_CLKSEL_bigslow1_clk_sel_START (0)
#define SOC_PMCTRL_BIGSLOW1_CLKSEL_bigslow1_clk_sel_END (1)
#define SOC_PMCTRL_BIGSLOW1_CLKSEL_bigslow1_clk_sel_stat_START (4)
#define SOC_PMCTRL_BIGSLOW1_CLKSEL_bigslow1_clk_sel_stat_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_apll_fbdiv : 12;
        unsigned int reserved : 20;
    } reg;
} SOC_PMCTRL_BIGSLOW1_CLKPROFILE0_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE0_bigslow1_apll_fbdiv_START (0)
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE0_bigslow1_apll_fbdiv_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_apll_fracdiv : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_BIGSLOW1_CLKPROFILE0_1_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE0_1_bigslow1_apll_fracdiv_START (0)
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE0_1_bigslow1_apll_fracdiv_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_dvfs_vol_bypass : 1;
        unsigned int reserved_0 : 3;
        unsigned int bigslow1_dvfs_apll_freq_bypass : 1;
        unsigned int reserved_1 : 3;
        unsigned int bigslow1_dvfs_vol_updn : 1;
        unsigned int reserved_2 : 3;
        unsigned int bigslow1_dvfs_apll_freq_updn : 1;
        unsigned int reserved_3 : 19;
    } reg;
} SOC_PMCTRL_BIGSLOW1_VOLMOD_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_VOLMOD_bigslow1_dvfs_vol_bypass_START (0)
#define SOC_PMCTRL_BIGSLOW1_VOLMOD_bigslow1_dvfs_vol_bypass_END (0)
#define SOC_PMCTRL_BIGSLOW1_VOLMOD_bigslow1_dvfs_apll_freq_bypass_START (4)
#define SOC_PMCTRL_BIGSLOW1_VOLMOD_bigslow1_dvfs_apll_freq_bypass_END (4)
#define SOC_PMCTRL_BIGSLOW1_VOLMOD_bigslow1_dvfs_vol_updn_START (8)
#define SOC_PMCTRL_BIGSLOW1_VOLMOD_bigslow1_dvfs_vol_updn_END (8)
#define SOC_PMCTRL_BIGSLOW1_VOLMOD_bigslow1_dvfs_apll_freq_updn_START (12)
#define SOC_PMCTRL_BIGSLOW1_VOLMOD_bigslow1_dvfs_apll_freq_updn_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_vol_idx_ocldo : 8;
        unsigned int bigslow1_vol_idx_pmu : 8;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_BIGSLOW1_VOLPROFILE_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_VOLPROFILE_bigslow1_vol_idx_ocldo_START (0)
#define SOC_PMCTRL_BIGSLOW1_VOLPROFILE_bigslow1_vol_idx_ocldo_END (7)
#define SOC_PMCTRL_BIGSLOW1_VOLPROFILE_bigslow1_vol_idx_pmu_START (8)
#define SOC_PMCTRL_BIGSLOW1_VOLPROFILE_bigslow1_vol_idx_pmu_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_gdvfs_profile_updn : 1;
        unsigned int reserved_0 : 3;
        unsigned int bigslow1_dvfs_mode : 2;
        unsigned int reserved_1 : 2;
        unsigned int bigslow1_step_number : 8;
        unsigned int bigslow1_clksel_prof : 2;
        unsigned int reserved_2 : 2;
        unsigned int bigslow1_gdvfs_ctrl_apll : 1;
        unsigned int reserved_3 : 11;
    } reg;
} SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE0_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE0_bigslow1_gdvfs_profile_updn_START (0)
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE0_bigslow1_gdvfs_profile_updn_END (0)
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE0_bigslow1_dvfs_mode_START (4)
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE0_bigslow1_dvfs_mode_END (5)
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE0_bigslow1_step_number_START (8)
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE0_bigslow1_step_number_END (15)
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE0_bigslow1_clksel_prof_START (16)
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE0_bigslow1_clksel_prof_END (17)
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE0_bigslow1_gdvfs_ctrl_apll_START (20)
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE0_bigslow1_gdvfs_ctrl_apll_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 2;
        unsigned int bigslow1_apll_cfg_time : 12;
        unsigned int bigslow1_clkdiv_time : 7;
        unsigned int reserved_2 : 9;
    } reg;
} SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE1_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE1_bigslow1_apll_cfg_time_START (4)
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE1_bigslow1_apll_cfg_time_END (15)
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE1_bigslow1_clkdiv_time_START (16)
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE1_bigslow1_clkdiv_time_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_vol_idx_cfg_ocldo : 8;
        unsigned int bigslow1_vol_idx_cfg_pmu : 8;
        unsigned int bigslow1_vol_chg_sftreq_vote_en : 1;
        unsigned int bigslow1_vol_chg_sftreq : 1;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_BIGSLOW1_VOLIDX_SFTCFG_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_VOLIDX_SFTCFG_bigslow1_vol_idx_cfg_ocldo_START (0)
#define SOC_PMCTRL_BIGSLOW1_VOLIDX_SFTCFG_bigslow1_vol_idx_cfg_ocldo_END (7)
#define SOC_PMCTRL_BIGSLOW1_VOLIDX_SFTCFG_bigslow1_vol_idx_cfg_pmu_START (8)
#define SOC_PMCTRL_BIGSLOW1_VOLIDX_SFTCFG_bigslow1_vol_idx_cfg_pmu_END (15)
#define SOC_PMCTRL_BIGSLOW1_VOLIDX_SFTCFG_bigslow1_vol_chg_sftreq_vote_en_START (16)
#define SOC_PMCTRL_BIGSLOW1_VOLIDX_SFTCFG_bigslow1_vol_chg_sftreq_vote_en_END (16)
#define SOC_PMCTRL_BIGSLOW1_VOLIDX_SFTCFG_bigslow1_vol_chg_sftreq_START (17)
#define SOC_PMCTRL_BIGSLOW1_VOLIDX_SFTCFG_bigslow1_vol_chg_sftreq_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int bigslow1_fbdiv_sw_initial : 12;
        unsigned int bigslow1_postdiv1_sw_initial : 3;
        unsigned int bigslow1_postdiv2_sw_initial : 3;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_PMCTRL_BIGSLOW1_APLL_INITIAL0_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_APLL_INITIAL0_bigslow1_fbdiv_sw_initial_START (8)
#define SOC_PMCTRL_BIGSLOW1_APLL_INITIAL0_bigslow1_fbdiv_sw_initial_END (19)
#define SOC_PMCTRL_BIGSLOW1_APLL_INITIAL0_bigslow1_postdiv1_sw_initial_START (20)
#define SOC_PMCTRL_BIGSLOW1_APLL_INITIAL0_bigslow1_postdiv1_sw_initial_END (22)
#define SOC_PMCTRL_BIGSLOW1_APLL_INITIAL0_bigslow1_postdiv2_sw_initial_START (23)
#define SOC_PMCTRL_BIGSLOW1_APLL_INITIAL0_bigslow1_postdiv2_sw_initial_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_fracdiv_sw_initial : 24;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 5;
    } reg;
} SOC_PMCTRL_BIGSLOW1_APLL_INITIAL1_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_APLL_INITIAL1_bigslow1_fracdiv_sw_initial_START (0)
#define SOC_PMCTRL_BIGSLOW1_APLL_INITIAL1_bigslow1_fracdiv_sw_initial_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 8;
        unsigned int bigslow_phase_protect_vol_ocldo : 8;
        unsigned int bigslow_phase_protect_vol_pmu : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_PMCTRL_BIGSLOW_VAU_DBG_STAT_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW_VAU_DBG_STAT_bigslow_phase_protect_vol_ocldo_START (8)
#define SOC_PMCTRL_BIGSLOW_VAU_DBG_STAT_bigslow_phase_protect_vol_ocldo_END (15)
#define SOC_PMCTRL_BIGSLOW_VAU_DBG_STAT_bigslow_phase_protect_vol_pmu_START (16)
#define SOC_PMCTRL_BIGSLOW_VAU_DBG_STAT_bigslow_phase_protect_vol_pmu_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow_cur_max_buck_vol_ocldo : 8;
        unsigned int bigslow_cur_max_buck_vol_pmu : 8;
        unsigned int bigslow_cur_max_buck_vol_tmp_ocldo : 8;
        unsigned int bigslow_cur_max_buck_vol_tmp_pmu : 8;
    } reg;
} SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT0_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT0_bigslow_cur_max_buck_vol_ocldo_START (0)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT0_bigslow_cur_max_buck_vol_ocldo_END (7)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT0_bigslow_cur_max_buck_vol_pmu_START (8)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT0_bigslow_cur_max_buck_vol_pmu_END (15)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT0_bigslow_cur_max_buck_vol_tmp_ocldo_START (16)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT0_bigslow_cur_max_buck_vol_tmp_ocldo_END (23)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT0_bigslow_cur_max_buck_vol_tmp_pmu_START (24)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT0_bigslow_cur_max_buck_vol_tmp_pmu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow_nxt_max_buck_vol_ocldo : 8;
        unsigned int bigslow_nxt_max_buck_vol_pmu : 8;
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT1_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT1_bigslow_nxt_max_buck_vol_ocldo_START (0)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT1_bigslow_nxt_max_buck_vol_ocldo_END (7)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT1_bigslow_nxt_max_buck_vol_pmu_START (8)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT1_bigslow_nxt_max_buck_vol_pmu_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow_cur_max_ocldo_vol_ocldo : 8;
        unsigned int bigslow_cur_max_ocldo_vol_pmu : 8;
        unsigned int bigslow_nxt_max_ocldo_vol_ocldo : 8;
        unsigned int bigslow_nxt_max_ocldo_vol_pmu : 8;
    } reg;
} SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT2_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT2_bigslow_cur_max_ocldo_vol_ocldo_START (0)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT2_bigslow_cur_max_ocldo_vol_ocldo_END (7)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT2_bigslow_cur_max_ocldo_vol_pmu_START (8)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT2_bigslow_cur_max_ocldo_vol_pmu_END (15)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT2_bigslow_nxt_max_ocldo_vol_ocldo_START (16)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT2_bigslow_nxt_max_ocldo_vol_ocldo_END (23)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT2_bigslow_nxt_max_ocldo_vol_pmu_START (24)
#define SOC_PMCTRL_BIGSLOW_MAX_VOL_DBG_STAT2_bigslow_nxt_max_ocldo_vol_pmu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow_unified_vol_idx_stat0_ocldo : 8;
        unsigned int bigslow_unified_vol_idx_stat0_pmu : 8;
        unsigned int bigslow_unified_vol_idx_stat1_ocldo : 8;
        unsigned int bigslow_unified_vol_idx_stat1_pmu : 8;
    } reg;
} SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_DBG_STAT0_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_DBG_STAT0_bigslow_unified_vol_idx_stat0_ocldo_START (0)
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_DBG_STAT0_bigslow_unified_vol_idx_stat0_ocldo_END (7)
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_DBG_STAT0_bigslow_unified_vol_idx_stat0_pmu_START (8)
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_DBG_STAT0_bigslow_unified_vol_idx_stat0_pmu_END (15)
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_DBG_STAT0_bigslow_unified_vol_idx_stat1_ocldo_START (16)
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_DBG_STAT0_bigslow_unified_vol_idx_stat1_ocldo_END (23)
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_DBG_STAT0_bigslow_unified_vol_idx_stat1_pmu_START (24)
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_DBG_STAT0_bigslow_unified_vol_idx_stat1_pmu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow_unified_pmu_vol_vol_idx_stat_ocldo : 8;
        unsigned int bigslow_unified_pmu_vol_vol_idx_stat_pmu : 8;
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_DBG_STAT1_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_DBG_STAT1_bigslow_unified_pmu_vol_vol_idx_stat_ocldo_START (0)
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_DBG_STAT1_bigslow_unified_pmu_vol_vol_idx_stat_ocldo_END (7)
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_DBG_STAT1_bigslow_unified_pmu_vol_vol_idx_stat_pmu_START (8)
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_DBG_STAT1_bigslow_unified_pmu_vol_vol_idx_stat_pmu_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow_cur_pmu_vol_idx_stat0_ocldo : 8;
        unsigned int bigslow_cur_pmu_vol_idx_stat0_pmu : 8;
        unsigned int bigslow_cur_pmu_vol_idx_stat1_ocldo : 8;
        unsigned int bigslow_cur_pmu_vol_idx_stat1_pmu : 8;
    } reg;
} SOC_PMCTRL_BIGSLOW_CUR_VOL_DBG_STAT0_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW_CUR_VOL_DBG_STAT0_bigslow_cur_pmu_vol_idx_stat0_ocldo_START (0)
#define SOC_PMCTRL_BIGSLOW_CUR_VOL_DBG_STAT0_bigslow_cur_pmu_vol_idx_stat0_ocldo_END (7)
#define SOC_PMCTRL_BIGSLOW_CUR_VOL_DBG_STAT0_bigslow_cur_pmu_vol_idx_stat0_pmu_START (8)
#define SOC_PMCTRL_BIGSLOW_CUR_VOL_DBG_STAT0_bigslow_cur_pmu_vol_idx_stat0_pmu_END (15)
#define SOC_PMCTRL_BIGSLOW_CUR_VOL_DBG_STAT0_bigslow_cur_pmu_vol_idx_stat1_ocldo_START (16)
#define SOC_PMCTRL_BIGSLOW_CUR_VOL_DBG_STAT0_bigslow_cur_pmu_vol_idx_stat1_ocldo_END (23)
#define SOC_PMCTRL_BIGSLOW_CUR_VOL_DBG_STAT0_bigslow_cur_pmu_vol_idx_stat1_pmu_START (24)
#define SOC_PMCTRL_BIGSLOW_CUR_VOL_DBG_STAT0_bigslow_cur_pmu_vol_idx_stat1_pmu_END (31)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
