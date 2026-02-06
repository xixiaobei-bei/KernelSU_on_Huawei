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
#define SOC_PMCTRL_DVFS_AUTOGT_BP_ADDR(base) ((base) + (0x098))
#define SOC_PMCTRL_CLUSTER0_CPUCLKSEL_ADDR(base) ((base) + (0x09C))
#define SOC_PMCTRL_CLUSTER0_CPUCLKDIV_ADDR(base) ((base) + (0x0A8))
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE0_ADDR(base) ((base) + (0x0B0))
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE0_1_ADDR(base) ((base) + (0x0B4))
#define SOC_PMCTRL_CLUSTER0_CPUCLKPROFILE1_ADDR(base) ((base) + (0x0B8))
#define SOC_PMCTRL_CLUSTER0_CPUVOLMOD_ADDR(base) ((base) + (0x0BC))
#define SOC_PMCTRL_CLUSTER0_CPUVOLPROFILE_ADDR(base) ((base) + (0x0C0))
#define SOC_PMCTRL_CLUSTER0_VOLUPSTEPTIME_ADDR(base) ((base) + (0x0D4))
#define SOC_PMCTRL_CLUSTER0_PMU_UP_HOLDTIME_ADDR(base) ((base) + (0x0D8))
#define SOC_PMCTRL_CLUSTER0_PMU_DN_HOLDTIME_ADDR(base) ((base) + (0x0E4))
#define SOC_PMCTRL_CLUSTER0_PMUSEL_ADDR(base) ((base) + (0x0E8))
#define SOC_PMCTRL_CLUSTER0_CPUVOLIDX_ADDR(base) ((base) + (0x0EC))
#define SOC_PMCTRL_CLUSTER0_GDVFS_EN_ADDR(base) ((base) + (0x4B0))
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE0_ADDR(base) ((base) + (0x4B4))
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE1_ADDR(base) ((base) + (0x4B8))
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
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_ADDR(base) ((base) + (0x248))
#define SOC_PMCTRL_L3_TOP_VOTE0_ADDR(base) ((base) + (0x250))
#define SOC_PMCTRL_L3_TOP_VOTE1_ADDR(base) ((base) + (0x254))
#define SOC_PMCTRL_L3_TOP_VOTE2_ADDR(base) ((base) + (0x258))
#define SOC_PMCTRL_L3_TOP_VOTE3_ADDR(base) ((base) + (0x25C))
#define SOC_PMCTRL_MIDDLE_TOP_VOTE0_ADDR(base) ((base) + (0x260))
#define SOC_PMCTRL_MIDDLE_TOP_VOTE1_ADDR(base) ((base) + (0x264))
#define SOC_PMCTRL_MIDDLE_TOP_VOTE2_ADDR(base) ((base) + (0x268))
#define SOC_PMCTRL_MIDDLE_TOP_VOTE3_ADDR(base) ((base) + (0x26C))
#define SOC_PMCTRL_BIG_TOP_VOTE0_ADDR(base) ((base) + (0x270))
#define SOC_PMCTRL_BIG_TOP_VOTE1_ADDR(base) ((base) + (0x274))
#define SOC_PMCTRL_BIG_TOP_VOTE2_ADDR(base) ((base) + (0x278))
#define SOC_PMCTRL_BIG_TOP_VOTE3_ADDR(base) ((base) + (0x27C))
#define SOC_PMCTRL_LITTLE_TOP_VOTE0_ADDR(base) ((base) + (0x280))
#define SOC_PMCTRL_LITTLE_TOP_VOTE1_ADDR(base) ((base) + (0x284))
#define SOC_PMCTRL_LITTLE_TOP_VOTE2_ADDR(base) ((base) + (0x288))
#define SOC_PMCTRL_LITTLE_TOP_VOTE3_ADDR(base) ((base) + (0x28C))
#define SOC_PMCTRL_GPU_TOP_VOTE0_ADDR(base) ((base) + (0x290))
#define SOC_PMCTRL_GPU_TOP_VOTE1_ADDR(base) ((base) + (0x294))
#define SOC_PMCTRL_GPU_TOP_VOTE2_ADDR(base) ((base) + (0x298))
#define SOC_PMCTRL_GPU_TOP_VOTE3_ADDR(base) ((base) + (0x29C))
#define SOC_PMCTRL_DDR_TOP_VOTE0_ADDR(base) ((base) + (0x2A0))
#define SOC_PMCTRL_DDR_TOP_VOTE1_ADDR(base) ((base) + (0x2A4))
#define SOC_PMCTRL_DDR_TOP_VOTE2_ADDR(base) ((base) + (0x2A8))
#define SOC_PMCTRL_DDR_TOP_VOTE3_ADDR(base) ((base) + (0x2AC))
#define SOC_PMCTRL_DDR_FLOOR_VOTE0_ADDR(base) ((base) + (0x2B0))
#define SOC_PMCTRL_DDR_FLOOR_VOTE1_ADDR(base) ((base) + (0x2B4))
#define SOC_PMCTRL_DDR_FLOOR_VOTE2_ADDR(base) ((base) + (0x2B8))
#define SOC_PMCTRL_DDR_FLOOR_VOTE3_ADDR(base) ((base) + (0x2BC))
#define SOC_PMCTRL_VOTE_MODE_CFG_ADDR(base) ((base) + (0x2C0))
#define SOC_PMCTRL_VOTE_START_EN_ADDR(base) ((base) + (0x2C4))
#define SOC_PMCTRL_INT_VOTE_EN_ADDR(base) ((base) + (0x2C8))
#define SOC_PMCTRL_INT_VOTE_CLR_ADDR(base) ((base) + (0x2CC))
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_ADDR(base) ((base) + (0x2D0))
#define SOC_PMCTRL_INT_VOTE_STAT_ADDR(base) ((base) + (0x2D4))
#define SOC_PMCTRL_INT_FREQUENCY_VOTE_ADDR(base) ((base) + (0x2D8))
#define SOC_PMCTRL_VOTE_RESULT0_ADDR(base) ((base) + (0x2DC))
#define SOC_PMCTRL_VOTE_RESULT1_ADDR(base) ((base) + (0x2E0))
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
#define SOC_PMCTRL_PERI_INT0_MASK_ADDR(base) ((base) + (0x3A0))
#define SOC_PMCTRL_PERI_INT0_STAT_ADDR(base) ((base) + (0x3A4))
#define SOC_PMCTRL_PERI_INT1_MASK_ADDR(base) ((base) + (0x3A8))
#define SOC_PMCTRL_PERI_INT1_STAT_ADDR(base) ((base) + (0x3AC))
#define SOC_PMCTRL_PERI_INT2_MASK_ADDR(base) ((base) + (0x3B0))
#define SOC_PMCTRL_PERI_INT2_STAT_ADDR(base) ((base) + (0x3B4))
#define SOC_PMCTRL_PERI_INT3_MASK_ADDR(base) ((base) + (0x3B8))
#define SOC_PMCTRL_PERI_INT3_STAT_ADDR(base) ((base) + (0x3BC))
#define SOC_PMCTRL_VS_CTRL_EN_BIG_ADDR(base) ((base) + (0x3C0))
#define SOC_PMCTRL_VS_CTRL_BYPASS_BIG_ADDR(base) ((base) + (0x3C4))
#define SOC_PMCTRL_VS_CTRL_BIG_ADDR(base) ((base) + (0x3C8))
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_ADDR(base) ((base) + (0x3CC))
#define SOC_PMCTRL_VS_SVFD_CTRL1_BIG_ADDR(base) ((base) + (0x3D0))
#define SOC_PMCTRL_VS_SVFD_CTRL2_BIG_ADDR(base) ((base) + (0x3D4))
#define SOC_PMCTRL_VS_CPM_DATA_STAT_BIG_ADDR(base) ((base) + (0x3D8))
#define SOC_PMCTRL_VS_TEST_STAT_BIG_ADDR(base) ((base) + (0x3DC))
#define SOC_PMCTRL_VS_CTRL_EN_MIDDLE_ADDR(base) ((base) + (0x3E0))
#define SOC_PMCTRL_VS_CTRL_BYPASS_MIDDLE_ADDR(base) ((base) + (0x3E4))
#define SOC_PMCTRL_VS_CTRL_MIDDLE_ADDR(base) ((base) + (0x3E8))
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_ADDR(base) ((base) + (0x3EC))
#define SOC_PMCTRL_VS_SVFD_CTRL1_MIDDLE_ADDR(base) ((base) + (0x3F0))
#define SOC_PMCTRL_VS_SVFD_CTRL2_MIDDLE_ADDR(base) ((base) + (0x3F4))
#define SOC_PMCTRL_VS_CPM_DATA_STAT_MIDDLE_ADDR(base) ((base) + (0x3F8))
#define SOC_PMCTRL_VS_TEST_STAT_MIDDLE_ADDR(base) ((base) + (0x3FC))
#define SOC_PMCTRL_VS_CTRL_EN_LITTLE_ADDR(base) ((base) + (0x400))
#define SOC_PMCTRL_VS_CTRL_BYPASS_LITTLE_ADDR(base) ((base) + (0x404))
#define SOC_PMCTRL_VS_CTRL_LITTLE_ADDR(base) ((base) + (0x408))
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_ADDR(base) ((base) + (0x40C))
#define SOC_PMCTRL_VS_SVFD_CTRL1_LITTLE_ADDR(base) ((base) + (0x410))
#define SOC_PMCTRL_VS_SVFD_CTRL2_LITTLE_ADDR(base) ((base) + (0x414))
#define SOC_PMCTRL_VS_CPM_DATA_STAT_LITTLE_ADDR(base) ((base) + (0x418))
#define SOC_PMCTRL_VS_TEST_STAT_LITTLE_ADDR(base) ((base) + (0x41C))
#define SOC_PMCTRL_VS_CTRL_EN_L3_ADDR(base) ((base) + (0x420))
#define SOC_PMCTRL_VS_CTRL_BYPASS_L3_ADDR(base) ((base) + (0x424))
#define SOC_PMCTRL_VS_CTRL_L3_ADDR(base) ((base) + (0x428))
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_ADDR(base) ((base) + (0x42C))
#define SOC_PMCTRL_VS_SVFD_CTRL1_L3_ADDR(base) ((base) + (0x430))
#define SOC_PMCTRL_VS_SVFD_CTRL2_L3_ADDR(base) ((base) + (0x434))
#define SOC_PMCTRL_VS_CPM_DATA_STAT_L3_ADDR(base) ((base) + (0x438))
#define SOC_PMCTRL_VS_TEST_STAT_L3_ADDR(base) ((base) + (0x43C))
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
#define SOC_PMCTRL_CLUSTER0_SW_INITIAL0_ADDR(base) ((base) + (0x470))
#define SOC_PMCTRL_CLUSTER0_SW_INITIAL1_ADDR(base) ((base) + (0x474))
#define SOC_PMCTRL_G3D_SW_INITIAL0_ADDR(base) ((base) + (0x478))
#define SOC_PMCTRL_G3D_SW_INITIAL1_ADDR(base) ((base) + (0x47C))
#define SOC_PMCTRL_L3_SW_INITIAL0_ADDR(base) ((base) + (0x480))
#define SOC_PMCTRL_L3_SW_INITIAL1_ADDR(base) ((base) + (0x484))
#define SOC_PMCTRL_PMCGDVFSSTATUS_ADDR(base) ((base) + (0x4DC))
#define SOC_PMCTRL_PERI_STAT8_ADDR(base) ((base) + (0x4E0))
#define SOC_PMCTRL_PERI_STAT9_ADDR(base) ((base) + (0x4E4))
#define SOC_PMCTRL_PERI_STAT10_ADDR(base) ((base) + (0x4E8))
#define SOC_PMCTRL_PERI_STAT11_ADDR(base) ((base) + (0x4EC))
#define SOC_PMCTRL_PERI_STAT12_ADDR(base) ((base) + (0x4F0))
#define SOC_PMCTRL_PERI_STAT13_ADDR(base) ((base) + (0x4F4))
#define SOC_PMCTRL_PERI_STAT14_ADDR(base) ((base) + (0x4F8))
#define SOC_PMCTRL_PERI_STAT15_ADDR(base) ((base) + (0x4FC))
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
#define SOC_PMCTRL_APLL0SSCCTRL_ADDR(base) ((base) + (0x5D0))
#define SOC_PMCTRL_APLL1SSCCTRL_ADDR(base) ((base) + (0x5D4))
#define SOC_PMCTRL_APLL2SSCCTRL_ADDR(base) ((base) + (0x5D8))
#define SOC_PMCTRL_APLL3SSCCTRL_ADDR(base) ((base) + (0x5DC))
#define SOC_PMCTRL_APLL4SSCCTRL_ADDR(base) ((base) + (0x5E0))
#define SOC_PMCTRL_APLL5SSCCTRL_ADDR(base) ((base) + (0x5E4))
#define SOC_PMCTRL_APLL6SSCCTRL_ADDR(base) ((base) + (0x5E8))
#define SOC_PMCTRL_L3_CLKSEL_ADDR(base) ((base) + (0x620))
#define SOC_PMCTRL_L3_CLKDIV_ADDR(base) ((base) + (0x624))
#define SOC_PMCTRL_L3_CLKPROFILE0_ADDR(base) ((base) + (0x628))
#define SOC_PMCTRL_L3_CLKPROFILE0_1_ADDR(base) ((base) + (0x62C))
#define SOC_PMCTRL_L3_CLKPROFILE1_ADDR(base) ((base) + (0x630))
#define SOC_PMCTRL_L3_VOLMOD_ADDR(base) ((base) + (0x634))
#define SOC_PMCTRL_L3_VOLPROFILE_ADDR(base) ((base) + (0x638))
#define SOC_PMCTRL_L3_VOLUPSTEPTIME_ADDR(base) ((base) + (0x63C))
#define SOC_PMCTRL_L3_PMU_UP_HOLDTIME_ADDR(base) ((base) + (0x640))
#define SOC_PMCTRL_L3_PMU_DN_HOLDTIME_ADDR(base) ((base) + (0x644))
#define SOC_PMCTRL_L3_PMUSEL_ADDR(base) ((base) + (0x648))
#define SOC_PMCTRL_L3_GDVFS_EN_ADDR(base) ((base) + (0x64C))
#define SOC_PMCTRL_L3_GDVFS_PROFILE0_ADDR(base) ((base) + (0x650))
#define SOC_PMCTRL_L3_GDVFS_PROFILE1_ADDR(base) ((base) + (0x654))
#define SOC_PMCTRL_L3_VOLIDX_ADDR(base) ((base) + (0x658))
#define SOC_PMCTRL_LITTLE_POWERSUPPLYMODE_ADDR(base) ((base) + (0x65C))
#define SOC_PMCTRL_LITTLE_INITIALVOL_ADDR(base) ((base) + (0x660))
#define SOC_PMCTRL_L3_INITIALVOL_ADDR(base) ((base) + (0x664))
#define SOC_PMCTRL_LILLTEL3_UNIFIED_VOL_STAT_ADDR(base) ((base) + (0x668))
#define SOC_PMCTRL_OCLDO_BYPASS_CTRL_ADDR(base) ((base) + (0x6B0))
#define SOC_PMCTRL_OCLDO_RETENTION_REQ_ADDR(base) ((base) + (0x6B4))
#define SOC_PMCTRL_OCLDO_DEBUG_STAT_ADDR(base) ((base) + (0x6B8))
#define SOC_PMCTRL_CORE0_DVFS_INNER_STAT_ADDR(base) ((base) + (0x6BC))
#define SOC_PMCTRL_CORE1_DVFS_INNER_STAT_ADDR(base) ((base) + (0x6C0))
#define SOC_PMCTRL_CORE2_DVFS_INNER_STAT_ADDR(base) ((base) + (0x6C4))
#define SOC_PMCTRL_CORE3_DVFS_INNER_STAT_ADDR(base) ((base) + (0x6C8))
#define SOC_PMCTRL_OCLDO0_DVFSVOLIDX_ADDR(base) ((base) + (0x6CC))
#define SOC_PMCTRL_OCLDO1_DVFSVOLIDX_ADDR(base) ((base) + (0x6D0))
#define SOC_PMCTRL_OCLDO2_DVFSVOLIDX_ADDR(base) ((base) + (0x6D4))
#define SOC_PMCTRL_OCLDO3_DVFSVOLIDX_ADDR(base) ((base) + (0x6D8))
#define SOC_PMCTRL_OCLDO_RH_CTRL0_ADDR(base) ((base) + (0x6DC))
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_ADDR(base) ((base) + (0x6E0))
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_ADDR(base) ((base) + (0x6E4))
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ADDR(base) ((base) + (0x700))
#define SOC_PMCTRL_OCLDO0_ANA_STAT_ADDR(base) ((base) + (0x710))
#define SOC_PMCTRL_OCLDO0_ERR_STAT_ADDR(base) ((base) + (0x714))
#define SOC_PMCTRL_OCLDO0_ACTIVE_CFG_ADDR(base) ((base) + (0x720))
#define SOC_PMCTRL_OCLDO0_RETENTION_CFG_ADDR(base) ((base) + (0x724))
#define SOC_PMCTRL_OCLDO0_OFFSET_VID_ADDR(base) ((base) + (0x728))
#define SOC_PMCTRL_OCLDO0_BYP_CTRL_ADDR(base) ((base) + (0x730))
#define SOC_PMCTRL_OCLDO0_SW_REQ_ADDR(base) ((base) + (0x734))
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_ADDR(base) ((base) + (0x738))
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_ADDR(base) ((base) + (0x73C))
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ADDR(base) ((base) + (0x740))
#define SOC_PMCTRL_OCLDO1_ANA_STAT_ADDR(base) ((base) + (0x750))
#define SOC_PMCTRL_OCLDO1_ERR_STAT_ADDR(base) ((base) + (0x754))
#define SOC_PMCTRL_OCLDO1_ACTIVE_CFG_ADDR(base) ((base) + (0x760))
#define SOC_PMCTRL_OCLDO1_RETENTION_CFG_ADDR(base) ((base) + (0x764))
#define SOC_PMCTRL_OCLDO1_OFFSET_VID_ADDR(base) ((base) + (0x768))
#define SOC_PMCTRL_OCLDO1_BYP_CTRL_ADDR(base) ((base) + (0x770))
#define SOC_PMCTRL_OCLDO1_SW_REQ_ADDR(base) ((base) + (0x774))
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_ADDR(base) ((base) + (0x778))
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_ADDR(base) ((base) + (0x77C))
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ADDR(base) ((base) + (0x780))
#define SOC_PMCTRL_OCLDO2_ANA_STAT_ADDR(base) ((base) + (0x790))
#define SOC_PMCTRL_OCLDO2_ERR_STAT_ADDR(base) ((base) + (0x794))
#define SOC_PMCTRL_OCLDO2_ACTIVE_CFG_ADDR(base) ((base) + (0x7A0))
#define SOC_PMCTRL_OCLDO2_RETENTION_CFG_ADDR(base) ((base) + (0x7A4))
#define SOC_PMCTRL_OCLDO2_OFFSET_VID_ADDR(base) ((base) + (0x7A8))
#define SOC_PMCTRL_OCLDO2_BYP_CTRL_ADDR(base) ((base) + (0x7B0))
#define SOC_PMCTRL_OCLDO2_SW_REQ_ADDR(base) ((base) + (0x7B4))
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_ADDR(base) ((base) + (0x7B8))
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_ADDR(base) ((base) + (0x7BC))
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ADDR(base) ((base) + (0x7C0))
#define SOC_PMCTRL_OCLDO3_ANA_STAT_ADDR(base) ((base) + (0x7D0))
#define SOC_PMCTRL_OCLDO3_ERR_STAT_ADDR(base) ((base) + (0x7D4))
#define SOC_PMCTRL_OCLDO3_ACTIVE_CFG_ADDR(base) ((base) + (0x7E0))
#define SOC_PMCTRL_OCLDO3_RETENTION_CFG_ADDR(base) ((base) + (0x7E4))
#define SOC_PMCTRL_OCLDO3_OFFSET_VID_ADDR(base) ((base) + (0x7E8))
#define SOC_PMCTRL_OCLDO3_BYP_CTRL_ADDR(base) ((base) + (0x7F0))
#define SOC_PMCTRL_OCLDO3_SW_REQ_ADDR(base) ((base) + (0x7F4))
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_ADDR(base) ((base) + (0x7F8))
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_ADDR(base) ((base) + (0x7FC))
#define SOC_PMCTRL_OCLDO_GLB_ACT_START_TIME_ADDR(base) ((base) + (0x800))
#define SOC_PMCTRL_OCLDO_GLB_RTN_START_TIME_ADDR(base) ((base) + (0x804))
#define SOC_PMCTRL_OCLDO_GLB_STEP_TIME_ADDR(base) ((base) + (0x808))
#define SOC_PMCTRL_PROFILE_RECOVERY_BYPASS_ADDR(base) ((base) + (0x8A0))
#define SOC_PMCTRL_PROFILE_RECOVERY_CLEAR_ADDR(base) ((base) + (0x8A4))
#define SOC_PMCTRL_BIGFAST_POWERSUPPLYMODE_ADDR(base) ((base) + (0x900))
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_STAT_ADDR(base) ((base) + (0x904))
#define SOC_PMCTRL_BIGFAST_DVFS_STAT_ADDR(base) ((base) + (0x908))
#define SOC_PMCTRL_BIGFAST0_CLKSEL_ADDR(base) ((base) + (0x910))
#define SOC_PMCTRL_BIGFAST0_CLKDIV_ADDR(base) ((base) + (0x914))
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE0_ADDR(base) ((base) + (0x918))
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE0_1_ADDR(base) ((base) + (0x91C))
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE1_ADDR(base) ((base) + (0x920))
#define SOC_PMCTRL_BIGFAST0_VOLMOD_ADDR(base) ((base) + (0x924))
#define SOC_PMCTRL_BIGFAST0_VOLPROFILE_ADDR(base) ((base) + (0x928))
#define SOC_PMCTRL_BIGFAST0_VOLUPSTEPTIME_ADDR(base) ((base) + (0x92C))
#define SOC_PMCTRL_BIGFAST0_PMU_UP_HOLDTIME_ADDR(base) ((base) + (0x930))
#define SOC_PMCTRL_BIGFAST0_PMU_DN_HOLDTIME_ADDR(base) ((base) + (0x934))
#define SOC_PMCTRL_BIGFAST0_PMUSEL_ADDR(base) ((base) + (0x938))
#define SOC_PMCTRL_BIGFAST0_GDVFS_EN_ADDR(base) ((base) + (0x93C))
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE0_ADDR(base) ((base) + (0x940))
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE1_ADDR(base) ((base) + (0x944))
#define SOC_PMCTRL_BIGFAST0_VOLIDX_ADDR(base) ((base) + (0x948))
#define SOC_PMCTRL_BIGFAST0_INITIALVOL_ADDR(base) ((base) + (0x94C))
#define SOC_PMCTRL_BIGFAST0_APLL_INITIAL0_ADDR(base) ((base) + (0x950))
#define SOC_PMCTRL_BIGFAST0_APLL_INITIAL1_ADDR(base) ((base) + (0x954))
#define SOC_PMCTRL_BIGFAST0_DVFS_SEL_ADDR(base) ((base) + (0x958))
#define SOC_PMCTRL_BIGFAST1_CLKSEL_ADDR(base) ((base) + (0x960))
#define SOC_PMCTRL_BIGFAST1_CLKDIV_ADDR(base) ((base) + (0x964))
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE0_ADDR(base) ((base) + (0x968))
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE0_1_ADDR(base) ((base) + (0x96C))
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE1_ADDR(base) ((base) + (0x970))
#define SOC_PMCTRL_BIGFAST1_VOLMOD_ADDR(base) ((base) + (0x974))
#define SOC_PMCTRL_BIGFAST1_VOLPROFILE_ADDR(base) ((base) + (0x978))
#define SOC_PMCTRL_BIGFAST1_VOLUPSTEPTIME_ADDR(base) ((base) + (0x97C))
#define SOC_PMCTRL_BIGFAST1_PMU_UP_HOLDTIME_ADDR(base) ((base) + (0x980))
#define SOC_PMCTRL_BIGFAST1_PMU_DN_HOLDTIME_ADDR(base) ((base) + (0x984))
#define SOC_PMCTRL_BIGFAST1_PMUSEL_ADDR(base) ((base) + (0x988))
#define SOC_PMCTRL_BIGFAST1_GDVFS_EN_ADDR(base) ((base) + (0x98C))
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE0_ADDR(base) ((base) + (0x990))
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE1_ADDR(base) ((base) + (0x994))
#define SOC_PMCTRL_BIGFAST1_VOLIDX_ADDR(base) ((base) + (0x998))
#define SOC_PMCTRL_BIGFAST1_INITIALVOL_ADDR(base) ((base) + (0x99C))
#define SOC_PMCTRL_BIGFAST1_APLL_INITIAL0_ADDR(base) ((base) + (0xA00))
#define SOC_PMCTRL_BIGFAST1_APLL_INITIAL1_ADDR(base) ((base) + (0xA04))
#define SOC_PMCTRL_BIGFAST1_DVFS_SEL_ADDR(base) ((base) + (0xA08))
#define SOC_PMCTRL_BIGSLOW_POWERSUPPLYMODE_ADDR(base) ((base) + (0xB00))
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_STAT_ADDR(base) ((base) + (0xB04))
#define SOC_PMCTRL_BIGSLOW_DVFS_STAT_ADDR(base) ((base) + (0xB08))
#define SOC_PMCTRL_BIGSLOW0_CLKSEL_ADDR(base) ((base) + (0xB10))
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_ADDR(base) ((base) + (0xB14))
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE0_ADDR(base) ((base) + (0xB18))
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE0_1_ADDR(base) ((base) + (0xB1C))
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE1_ADDR(base) ((base) + (0xB20))
#define SOC_PMCTRL_BIGSLOW0_VOLMOD_ADDR(base) ((base) + (0xB24))
#define SOC_PMCTRL_BIGSLOW0_VOLPROFILE_ADDR(base) ((base) + (0xB28))
#define SOC_PMCTRL_BIGSLOW0_VOLUPSTEPTIME_ADDR(base) ((base) + (0xB2C))
#define SOC_PMCTRL_BIGSLOW0_PMU_UP_HOLDTIME_ADDR(base) ((base) + (0xB30))
#define SOC_PMCTRL_BIGSLOW0_PMU_DN_HOLDTIME_ADDR(base) ((base) + (0xB34))
#define SOC_PMCTRL_BIGSLOW0_PMUSEL_ADDR(base) ((base) + (0xB38))
#define SOC_PMCTRL_BIGSLOW0_GDVFS_EN_ADDR(base) ((base) + (0xB3C))
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE0_ADDR(base) ((base) + (0xB40))
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE1_ADDR(base) ((base) + (0xB44))
#define SOC_PMCTRL_BIGSLOW0_VOLIDX_ADDR(base) ((base) + (0xB48))
#define SOC_PMCTRL_BIGSLOW0_INITIALVOL_ADDR(base) ((base) + (0xB4C))
#define SOC_PMCTRL_BIGSLOW0_APLL_INITIAL0_ADDR(base) ((base) + (0xB50))
#define SOC_PMCTRL_BIGSLOW0_APLL_INITIAL1_ADDR(base) ((base) + (0xB54))
#define SOC_PMCTRL_BIGSLOW0_DVFS_SEL_ADDR(base) ((base) + (0xB58))
#define SOC_PMCTRL_BIGSLOW1_CLKSEL_ADDR(base) ((base) + (0xB60))
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_ADDR(base) ((base) + (0xB64))
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE0_ADDR(base) ((base) + (0xB68))
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE0_1_ADDR(base) ((base) + (0xB6C))
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE1_ADDR(base) ((base) + (0xB70))
#define SOC_PMCTRL_BIGSLOW1_VOLMOD_ADDR(base) ((base) + (0xB74))
#define SOC_PMCTRL_BIGSLOW1_VOLPROFILE_ADDR(base) ((base) + (0xB78))
#define SOC_PMCTRL_BIGSLOW1_VOLUPSTEPTIME_ADDR(base) ((base) + (0xB7C))
#define SOC_PMCTRL_BIGSLOW1_PMU_UP_HOLDTIME_ADDR(base) ((base) + (0xB80))
#define SOC_PMCTRL_BIGSLOW1_PMU_DN_HOLDTIME_ADDR(base) ((base) + (0xB84))
#define SOC_PMCTRL_BIGSLOW1_PMUSEL_ADDR(base) ((base) + (0xB88))
#define SOC_PMCTRL_BIGSLOW1_GDVFS_EN_ADDR(base) ((base) + (0xB8C))
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE0_ADDR(base) ((base) + (0xB90))
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE1_ADDR(base) ((base) + (0xB94))
#define SOC_PMCTRL_BIGSLOW1_VOLIDX_ADDR(base) ((base) + (0xB98))
#define SOC_PMCTRL_BIGSLOW1_INITIALVOL_ADDR(base) ((base) + (0xB9C))
#define SOC_PMCTRL_BIGSLOW1_APLL_INITIAL0_ADDR(base) ((base) + (0xC00))
#define SOC_PMCTRL_BIGSLOW1_APLL_INITIAL1_ADDR(base) ((base) + (0xC04))
#define SOC_PMCTRL_BIGSLOW1_DVFS_SEL_ADDR(base) ((base) + (0xC08))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_cluster0_cpu_dvfs_en : 1;
        unsigned int int_l3_dvfs_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int int_bigfast0_dvfs_en : 1;
        unsigned int int_bigfast1_dvfs_en : 1;
        unsigned int int_bigslow0_dvfs_en : 1;
        unsigned int int_bigslow1_dvfs_en : 1;
        unsigned int int_g3d_dvfs_en : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 14;
    } reg;
} SOC_PMCTRL_PMCINTEN_UNION;
#endif
#define SOC_PMCTRL_PMCINTEN_int_cluster0_cpu_dvfs_en_START (0)
#define SOC_PMCTRL_PMCINTEN_int_cluster0_cpu_dvfs_en_END (0)
#define SOC_PMCTRL_PMCINTEN_int_l3_dvfs_en_START (1)
#define SOC_PMCTRL_PMCINTEN_int_l3_dvfs_en_END (1)
#define SOC_PMCTRL_PMCINTEN_int_bigfast0_dvfs_en_START (8)
#define SOC_PMCTRL_PMCINTEN_int_bigfast0_dvfs_en_END (8)
#define SOC_PMCTRL_PMCINTEN_int_bigfast1_dvfs_en_START (9)
#define SOC_PMCTRL_PMCINTEN_int_bigfast1_dvfs_en_END (9)
#define SOC_PMCTRL_PMCINTEN_int_bigslow0_dvfs_en_START (10)
#define SOC_PMCTRL_PMCINTEN_int_bigslow0_dvfs_en_END (10)
#define SOC_PMCTRL_PMCINTEN_int_bigslow1_dvfs_en_START (11)
#define SOC_PMCTRL_PMCINTEN_int_bigslow1_dvfs_en_END (11)
#define SOC_PMCTRL_PMCINTEN_int_g3d_dvfs_en_START (12)
#define SOC_PMCTRL_PMCINTEN_int_g3d_dvfs_en_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_cluster0_cpu_dvfs_stat : 1;
        unsigned int int_l3_dvfs_stat : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int int_bigfast0_dvfs_stat : 1;
        unsigned int int_bigfast1_dvfs_stat : 1;
        unsigned int int_bigslow0_dvfs_stat : 1;
        unsigned int int_bigslow1_dvfs_stat : 1;
        unsigned int int_g3d_dvfs_stat : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 14;
    } reg;
} SOC_PMCTRL_PMCINTSTAT_UNION;
#endif
#define SOC_PMCTRL_PMCINTSTAT_int_cluster0_cpu_dvfs_stat_START (0)
#define SOC_PMCTRL_PMCINTSTAT_int_cluster0_cpu_dvfs_stat_END (0)
#define SOC_PMCTRL_PMCINTSTAT_int_l3_dvfs_stat_START (1)
#define SOC_PMCTRL_PMCINTSTAT_int_l3_dvfs_stat_END (1)
#define SOC_PMCTRL_PMCINTSTAT_int_bigfast0_dvfs_stat_START (8)
#define SOC_PMCTRL_PMCINTSTAT_int_bigfast0_dvfs_stat_END (8)
#define SOC_PMCTRL_PMCINTSTAT_int_bigfast1_dvfs_stat_START (9)
#define SOC_PMCTRL_PMCINTSTAT_int_bigfast1_dvfs_stat_END (9)
#define SOC_PMCTRL_PMCINTSTAT_int_bigslow0_dvfs_stat_START (10)
#define SOC_PMCTRL_PMCINTSTAT_int_bigslow0_dvfs_stat_END (10)
#define SOC_PMCTRL_PMCINTSTAT_int_bigslow1_dvfs_stat_START (11)
#define SOC_PMCTRL_PMCINTSTAT_int_bigslow1_dvfs_stat_END (11)
#define SOC_PMCTRL_PMCINTSTAT_int_g3d_dvfs_stat_START (12)
#define SOC_PMCTRL_PMCINTSTAT_int_g3d_dvfs_stat_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_cluster0_cpu_dvfs_clr : 1;
        unsigned int int_l3_dvfs_clr : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int int_bigfast0_dvfs_clr : 1;
        unsigned int int_bigfast1_dvfs_clr : 1;
        unsigned int int_bigslow0_dvfs_clr : 1;
        unsigned int int_bigslow1_dvfs_clr : 1;
        unsigned int int_g3d_dvfs_clr : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 14;
    } reg;
} SOC_PMCTRL_PMCINTCLR_UNION;
#endif
#define SOC_PMCTRL_PMCINTCLR_int_cluster0_cpu_dvfs_clr_START (0)
#define SOC_PMCTRL_PMCINTCLR_int_cluster0_cpu_dvfs_clr_END (0)
#define SOC_PMCTRL_PMCINTCLR_int_l3_dvfs_clr_START (1)
#define SOC_PMCTRL_PMCINTCLR_int_l3_dvfs_clr_END (1)
#define SOC_PMCTRL_PMCINTCLR_int_bigfast0_dvfs_clr_START (8)
#define SOC_PMCTRL_PMCINTCLR_int_bigfast0_dvfs_clr_END (8)
#define SOC_PMCTRL_PMCINTCLR_int_bigfast1_dvfs_clr_START (9)
#define SOC_PMCTRL_PMCINTCLR_int_bigfast1_dvfs_clr_END (9)
#define SOC_PMCTRL_PMCINTCLR_int_bigslow0_dvfs_clr_START (10)
#define SOC_PMCTRL_PMCINTCLR_int_bigslow0_dvfs_clr_END (10)
#define SOC_PMCTRL_PMCINTCLR_int_bigslow1_dvfs_clr_START (11)
#define SOC_PMCTRL_PMCINTCLR_int_bigslow1_dvfs_clr_END (11)
#define SOC_PMCTRL_PMCINTCLR_int_g3d_dvfs_clr_START (12)
#define SOC_PMCTRL_PMCINTCLR_int_g3d_dvfs_clr_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_dvfs_auto_gt_bypass : 1;
        unsigned int bigslow0_dvfs_auto_gt_bypass : 1;
        unsigned int bigfast1_dvfs_auto_gt_bypass : 1;
        unsigned int bigfast0_dvfs_auto_gt_bypass : 1;
        unsigned int cluster0_dvfs_auto_gt_bypass : 1;
        unsigned int reserved_0 : 3;
        unsigned int g3d_dvfs_auto_gt_bypass : 1;
        unsigned int reserved_1 : 3;
        unsigned int l3_dvfs_auto_gt_bypass : 1;
        unsigned int reserved_2 : 19;
    } reg;
} SOC_PMCTRL_DVFS_AUTOGT_BP_UNION;
#endif
#define SOC_PMCTRL_DVFS_AUTOGT_BP_bigslow1_dvfs_auto_gt_bypass_START (0)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_bigslow1_dvfs_auto_gt_bypass_END (0)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_bigslow0_dvfs_auto_gt_bypass_START (1)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_bigslow0_dvfs_auto_gt_bypass_END (1)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_bigfast1_dvfs_auto_gt_bypass_START (2)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_bigfast1_dvfs_auto_gt_bypass_END (2)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_bigfast0_dvfs_auto_gt_bypass_START (3)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_bigfast0_dvfs_auto_gt_bypass_END (3)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_cluster0_dvfs_auto_gt_bypass_START (4)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_cluster0_dvfs_auto_gt_bypass_END (4)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_g3d_dvfs_auto_gt_bypass_START (8)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_g3d_dvfs_auto_gt_bypass_END (8)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_l3_dvfs_auto_gt_bypass_START (12)
#define SOC_PMCTRL_DVFS_AUTOGT_BP_l3_dvfs_auto_gt_bypass_END (12)
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
        unsigned int cluster0_cpu_vol_mod : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_CLUSTER0_CPUVOLMOD_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_CPUVOLMOD_cluster0_cpu_vol_mod_START (0)
#define SOC_PMCTRL_CLUSTER0_CPUVOLMOD_cluster0_cpu_vol_mod_END (0)
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
        unsigned int cluster0_vol_up_step_time : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_PMCTRL_CLUSTER0_VOLUPSTEPTIME_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_VOLUPSTEPTIME_cluster0_vol_up_step_time_START (0)
#define SOC_PMCTRL_CLUSTER0_VOLUPSTEPTIME_cluster0_vol_up_step_time_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_pmu_up_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_CLUSTER0_PMU_UP_HOLDTIME_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_PMU_UP_HOLDTIME_cluster0_pmu_up_vol_hold_time_START (0)
#define SOC_PMCTRL_CLUSTER0_PMU_UP_HOLDTIME_cluster0_pmu_up_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cluster0_pmu_dn_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_CLUSTER0_PMU_DN_HOLDTIME_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_PMU_DN_HOLDTIME_cluster0_pmu_dn_vol_hold_time_START (0)
#define SOC_PMCTRL_CLUSTER0_PMU_DN_HOLDTIME_cluster0_pmu_dn_vol_hold_time_END (19)
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
        unsigned int cluster0_cpu_vol_chg_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int cluster0_cpu_vol_idx_stat : 8;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_CLUSTER0_CPUVOLIDX_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_CPUVOLIDX_cluster0_cpu_vol_idx_cfg_START (0)
#define SOC_PMCTRL_CLUSTER0_CPUVOLIDX_cluster0_cpu_vol_idx_cfg_END (7)
#define SOC_PMCTRL_CLUSTER0_CPUVOLIDX_cluster0_cpu_vol_chg_sftreq_START (8)
#define SOC_PMCTRL_CLUSTER0_CPUVOLIDX_cluster0_cpu_vol_chg_sftreq_END (8)
#define SOC_PMCTRL_CLUSTER0_CPUVOLIDX_cluster0_cpu_vol_idx_stat_START (12)
#define SOC_PMCTRL_CLUSTER0_CPUVOLIDX_cluster0_cpu_vol_idx_stat_END (19)
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
        unsigned int cluster0_fbdiv_step : 2;
        unsigned int reserved_0 : 2;
        unsigned int cluster0_apll_cfg_time : 12;
        unsigned int cluster0_clkdiv_time : 7;
        unsigned int reserved_1 : 9;
    } reg;
} SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE1_UNION;
#endif
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE1_cluster0_fbdiv_step_START (0)
#define SOC_PMCTRL_CLUSTER0_GDVFS_PROFILE1_cluster0_fbdiv_step_END (1)
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
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_G3DCLKPROFILE_UNION;
#endif
#define SOC_PMCTRL_G3DCLKPROFILE_g3da_clk_sel_START (0)
#define SOC_PMCTRL_G3DCLKPROFILE_g3da_clk_sel_END (0)
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
        unsigned int g3d_fbdiv_step : 2;
        unsigned int reserved_0 : 2;
        unsigned int g3d_apll_cfg_time : 12;
        unsigned int g3d_clkdiv_time : 7;
        unsigned int reserved_1 : 9;
    } reg;
} SOC_PMCTRL_G3D_GDVFS_PROFILE1_UNION;
#endif
#define SOC_PMCTRL_G3D_GDVFS_PROFILE1_g3d_fbdiv_step_START (0)
#define SOC_PMCTRL_G3D_GDVFS_PROFILE1_g3d_fbdiv_step_END (1)
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
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_G3DVOLIDX_UNION;
#endif
#define SOC_PMCTRL_G3DVOLIDX_g3d_vol_idx_cfg_START (0)
#define SOC_PMCTRL_G3DVOLIDX_g3d_vol_idx_cfg_END (7)
#define SOC_PMCTRL_G3DVOLIDX_g3d_vol_chg_sftreq_START (8)
#define SOC_PMCTRL_G3DVOLIDX_g3d_vol_chg_sftreq_END (8)
#define SOC_PMCTRL_G3DVOLIDX_g3d_vol_idx_stat_START (12)
#define SOC_PMCTRL_G3DVOLIDX_g3d_vol_idx_stat_END (19)
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
        unsigned int g3d_dbtfsm_curr_stat : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_PMCTRL_G3DDBTSTAT_UNION;
#endif
#define SOC_PMCTRL_G3DDBTSTAT_g3d_dbtfsm_curr_stat_START (0)
#define SOC_PMCTRL_G3DDBTSTAT_g3d_dbtfsm_curr_stat_END (2)
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
        unsigned int l3_f_vote_top0 : 8;
        unsigned int l3_f_vote_top1 : 8;
        unsigned int l3_top_vote0_msk : 16;
    } reg;
} SOC_PMCTRL_L3_TOP_VOTE0_UNION;
#endif
#define SOC_PMCTRL_L3_TOP_VOTE0_l3_f_vote_top0_START (0)
#define SOC_PMCTRL_L3_TOP_VOTE0_l3_f_vote_top0_END (7)
#define SOC_PMCTRL_L3_TOP_VOTE0_l3_f_vote_top1_START (8)
#define SOC_PMCTRL_L3_TOP_VOTE0_l3_f_vote_top1_END (15)
#define SOC_PMCTRL_L3_TOP_VOTE0_l3_top_vote0_msk_START (16)
#define SOC_PMCTRL_L3_TOP_VOTE0_l3_top_vote0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_f_vote_top2 : 8;
        unsigned int l3_f_vote_top3 : 8;
        unsigned int l3_top_vote1_msk : 16;
    } reg;
} SOC_PMCTRL_L3_TOP_VOTE1_UNION;
#endif
#define SOC_PMCTRL_L3_TOP_VOTE1_l3_f_vote_top2_START (0)
#define SOC_PMCTRL_L3_TOP_VOTE1_l3_f_vote_top2_END (7)
#define SOC_PMCTRL_L3_TOP_VOTE1_l3_f_vote_top3_START (8)
#define SOC_PMCTRL_L3_TOP_VOTE1_l3_f_vote_top3_END (15)
#define SOC_PMCTRL_L3_TOP_VOTE1_l3_top_vote1_msk_START (16)
#define SOC_PMCTRL_L3_TOP_VOTE1_l3_top_vote1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_f_vote_top4 : 8;
        unsigned int l3_f_vote_top5 : 8;
        unsigned int l3_top_vote2_msk : 16;
    } reg;
} SOC_PMCTRL_L3_TOP_VOTE2_UNION;
#endif
#define SOC_PMCTRL_L3_TOP_VOTE2_l3_f_vote_top4_START (0)
#define SOC_PMCTRL_L3_TOP_VOTE2_l3_f_vote_top4_END (7)
#define SOC_PMCTRL_L3_TOP_VOTE2_l3_f_vote_top5_START (8)
#define SOC_PMCTRL_L3_TOP_VOTE2_l3_f_vote_top5_END (15)
#define SOC_PMCTRL_L3_TOP_VOTE2_l3_top_vote2_msk_START (16)
#define SOC_PMCTRL_L3_TOP_VOTE2_l3_top_vote2_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_f_vote_top6 : 8;
        unsigned int l3_f_vote_top7 : 8;
        unsigned int l3_top_vote3_msk : 16;
    } reg;
} SOC_PMCTRL_L3_TOP_VOTE3_UNION;
#endif
#define SOC_PMCTRL_L3_TOP_VOTE3_l3_f_vote_top6_START (0)
#define SOC_PMCTRL_L3_TOP_VOTE3_l3_f_vote_top6_END (7)
#define SOC_PMCTRL_L3_TOP_VOTE3_l3_f_vote_top7_START (8)
#define SOC_PMCTRL_L3_TOP_VOTE3_l3_f_vote_top7_END (15)
#define SOC_PMCTRL_L3_TOP_VOTE3_l3_top_vote3_msk_START (16)
#define SOC_PMCTRL_L3_TOP_VOTE3_l3_top_vote3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int middle_f_vote_top0 : 8;
        unsigned int middle_f_vote_top1 : 8;
        unsigned int middle_top_vote0_msk : 16;
    } reg;
} SOC_PMCTRL_MIDDLE_TOP_VOTE0_UNION;
#endif
#define SOC_PMCTRL_MIDDLE_TOP_VOTE0_middle_f_vote_top0_START (0)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE0_middle_f_vote_top0_END (7)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE0_middle_f_vote_top1_START (8)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE0_middle_f_vote_top1_END (15)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE0_middle_top_vote0_msk_START (16)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE0_middle_top_vote0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int middle_f_vote_top2 : 8;
        unsigned int middle_f_vote_top3 : 8;
        unsigned int middle_top_vote1_msk : 16;
    } reg;
} SOC_PMCTRL_MIDDLE_TOP_VOTE1_UNION;
#endif
#define SOC_PMCTRL_MIDDLE_TOP_VOTE1_middle_f_vote_top2_START (0)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE1_middle_f_vote_top2_END (7)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE1_middle_f_vote_top3_START (8)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE1_middle_f_vote_top3_END (15)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE1_middle_top_vote1_msk_START (16)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE1_middle_top_vote1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int middle_f_vote_top4 : 8;
        unsigned int middle_f_vote_top5 : 8;
        unsigned int middle_top_vote2_msk : 16;
    } reg;
} SOC_PMCTRL_MIDDLE_TOP_VOTE2_UNION;
#endif
#define SOC_PMCTRL_MIDDLE_TOP_VOTE2_middle_f_vote_top4_START (0)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE2_middle_f_vote_top4_END (7)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE2_middle_f_vote_top5_START (8)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE2_middle_f_vote_top5_END (15)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE2_middle_top_vote2_msk_START (16)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE2_middle_top_vote2_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int middle_f_vote_top6 : 8;
        unsigned int middle_f_vote_top7 : 8;
        unsigned int middle_top_vote3_msk : 16;
    } reg;
} SOC_PMCTRL_MIDDLE_TOP_VOTE3_UNION;
#endif
#define SOC_PMCTRL_MIDDLE_TOP_VOTE3_middle_f_vote_top6_START (0)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE3_middle_f_vote_top6_END (7)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE3_middle_f_vote_top7_START (8)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE3_middle_f_vote_top7_END (15)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE3_middle_top_vote3_msk_START (16)
#define SOC_PMCTRL_MIDDLE_TOP_VOTE3_middle_top_vote3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int big_f_vote_top0 : 8;
        unsigned int big_f_vote_top1 : 8;
        unsigned int big_top_vote0_msk : 16;
    } reg;
} SOC_PMCTRL_BIG_TOP_VOTE0_UNION;
#endif
#define SOC_PMCTRL_BIG_TOP_VOTE0_big_f_vote_top0_START (0)
#define SOC_PMCTRL_BIG_TOP_VOTE0_big_f_vote_top0_END (7)
#define SOC_PMCTRL_BIG_TOP_VOTE0_big_f_vote_top1_START (8)
#define SOC_PMCTRL_BIG_TOP_VOTE0_big_f_vote_top1_END (15)
#define SOC_PMCTRL_BIG_TOP_VOTE0_big_top_vote0_msk_START (16)
#define SOC_PMCTRL_BIG_TOP_VOTE0_big_top_vote0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int big_f_vote_top2 : 8;
        unsigned int big_f_vote_top3 : 8;
        unsigned int big_top_vote1_msk : 16;
    } reg;
} SOC_PMCTRL_BIG_TOP_VOTE1_UNION;
#endif
#define SOC_PMCTRL_BIG_TOP_VOTE1_big_f_vote_top2_START (0)
#define SOC_PMCTRL_BIG_TOP_VOTE1_big_f_vote_top2_END (7)
#define SOC_PMCTRL_BIG_TOP_VOTE1_big_f_vote_top3_START (8)
#define SOC_PMCTRL_BIG_TOP_VOTE1_big_f_vote_top3_END (15)
#define SOC_PMCTRL_BIG_TOP_VOTE1_big_top_vote1_msk_START (16)
#define SOC_PMCTRL_BIG_TOP_VOTE1_big_top_vote1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int big_f_vote_top4 : 8;
        unsigned int big_f_vote_top5 : 8;
        unsigned int big_top_vote2_msk : 16;
    } reg;
} SOC_PMCTRL_BIG_TOP_VOTE2_UNION;
#endif
#define SOC_PMCTRL_BIG_TOP_VOTE2_big_f_vote_top4_START (0)
#define SOC_PMCTRL_BIG_TOP_VOTE2_big_f_vote_top4_END (7)
#define SOC_PMCTRL_BIG_TOP_VOTE2_big_f_vote_top5_START (8)
#define SOC_PMCTRL_BIG_TOP_VOTE2_big_f_vote_top5_END (15)
#define SOC_PMCTRL_BIG_TOP_VOTE2_big_top_vote2_msk_START (16)
#define SOC_PMCTRL_BIG_TOP_VOTE2_big_top_vote2_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int big_f_vote_top6 : 8;
        unsigned int big_f_vote_top7 : 8;
        unsigned int big_top_vote3_msk : 16;
    } reg;
} SOC_PMCTRL_BIG_TOP_VOTE3_UNION;
#endif
#define SOC_PMCTRL_BIG_TOP_VOTE3_big_f_vote_top6_START (0)
#define SOC_PMCTRL_BIG_TOP_VOTE3_big_f_vote_top6_END (7)
#define SOC_PMCTRL_BIG_TOP_VOTE3_big_f_vote_top7_START (8)
#define SOC_PMCTRL_BIG_TOP_VOTE3_big_f_vote_top7_END (15)
#define SOC_PMCTRL_BIG_TOP_VOTE3_big_top_vote3_msk_START (16)
#define SOC_PMCTRL_BIG_TOP_VOTE3_big_top_vote3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int little_f_vote_top0 : 8;
        unsigned int little_f_vote_top1 : 8;
        unsigned int little_top_vote0_msk : 16;
    } reg;
} SOC_PMCTRL_LITTLE_TOP_VOTE0_UNION;
#endif
#define SOC_PMCTRL_LITTLE_TOP_VOTE0_little_f_vote_top0_START (0)
#define SOC_PMCTRL_LITTLE_TOP_VOTE0_little_f_vote_top0_END (7)
#define SOC_PMCTRL_LITTLE_TOP_VOTE0_little_f_vote_top1_START (8)
#define SOC_PMCTRL_LITTLE_TOP_VOTE0_little_f_vote_top1_END (15)
#define SOC_PMCTRL_LITTLE_TOP_VOTE0_little_top_vote0_msk_START (16)
#define SOC_PMCTRL_LITTLE_TOP_VOTE0_little_top_vote0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int little_f_vote_top2 : 8;
        unsigned int little_f_vote_top3 : 8;
        unsigned int little_top_vote1_msk : 16;
    } reg;
} SOC_PMCTRL_LITTLE_TOP_VOTE1_UNION;
#endif
#define SOC_PMCTRL_LITTLE_TOP_VOTE1_little_f_vote_top2_START (0)
#define SOC_PMCTRL_LITTLE_TOP_VOTE1_little_f_vote_top2_END (7)
#define SOC_PMCTRL_LITTLE_TOP_VOTE1_little_f_vote_top3_START (8)
#define SOC_PMCTRL_LITTLE_TOP_VOTE1_little_f_vote_top3_END (15)
#define SOC_PMCTRL_LITTLE_TOP_VOTE1_little_top_vote1_msk_START (16)
#define SOC_PMCTRL_LITTLE_TOP_VOTE1_little_top_vote1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int little_f_vote_top4 : 8;
        unsigned int little_f_vote_top5 : 8;
        unsigned int little_top_vote2_msk : 16;
    } reg;
} SOC_PMCTRL_LITTLE_TOP_VOTE2_UNION;
#endif
#define SOC_PMCTRL_LITTLE_TOP_VOTE2_little_f_vote_top4_START (0)
#define SOC_PMCTRL_LITTLE_TOP_VOTE2_little_f_vote_top4_END (7)
#define SOC_PMCTRL_LITTLE_TOP_VOTE2_little_f_vote_top5_START (8)
#define SOC_PMCTRL_LITTLE_TOP_VOTE2_little_f_vote_top5_END (15)
#define SOC_PMCTRL_LITTLE_TOP_VOTE2_little_top_vote2_msk_START (16)
#define SOC_PMCTRL_LITTLE_TOP_VOTE2_little_top_vote2_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int little_f_vote_top6 : 8;
        unsigned int little_f_vote_top7 : 8;
        unsigned int little_top_vote3_msk : 16;
    } reg;
} SOC_PMCTRL_LITTLE_TOP_VOTE3_UNION;
#endif
#define SOC_PMCTRL_LITTLE_TOP_VOTE3_little_f_vote_top6_START (0)
#define SOC_PMCTRL_LITTLE_TOP_VOTE3_little_f_vote_top6_END (7)
#define SOC_PMCTRL_LITTLE_TOP_VOTE3_little_f_vote_top7_START (8)
#define SOC_PMCTRL_LITTLE_TOP_VOTE3_little_f_vote_top7_END (15)
#define SOC_PMCTRL_LITTLE_TOP_VOTE3_little_top_vote3_msk_START (16)
#define SOC_PMCTRL_LITTLE_TOP_VOTE3_little_top_vote3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpu_f_vote_top0 : 8;
        unsigned int gpu_f_vote_top1 : 8;
        unsigned int gpu_top_vote0_msk : 16;
    } reg;
} SOC_PMCTRL_GPU_TOP_VOTE0_UNION;
#endif
#define SOC_PMCTRL_GPU_TOP_VOTE0_gpu_f_vote_top0_START (0)
#define SOC_PMCTRL_GPU_TOP_VOTE0_gpu_f_vote_top0_END (7)
#define SOC_PMCTRL_GPU_TOP_VOTE0_gpu_f_vote_top1_START (8)
#define SOC_PMCTRL_GPU_TOP_VOTE0_gpu_f_vote_top1_END (15)
#define SOC_PMCTRL_GPU_TOP_VOTE0_gpu_top_vote0_msk_START (16)
#define SOC_PMCTRL_GPU_TOP_VOTE0_gpu_top_vote0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpu_f_vote_top2 : 8;
        unsigned int gpu_f_vote_top3 : 8;
        unsigned int gpu_top_vote1_msk : 16;
    } reg;
} SOC_PMCTRL_GPU_TOP_VOTE1_UNION;
#endif
#define SOC_PMCTRL_GPU_TOP_VOTE1_gpu_f_vote_top2_START (0)
#define SOC_PMCTRL_GPU_TOP_VOTE1_gpu_f_vote_top2_END (7)
#define SOC_PMCTRL_GPU_TOP_VOTE1_gpu_f_vote_top3_START (8)
#define SOC_PMCTRL_GPU_TOP_VOTE1_gpu_f_vote_top3_END (15)
#define SOC_PMCTRL_GPU_TOP_VOTE1_gpu_top_vote1_msk_START (16)
#define SOC_PMCTRL_GPU_TOP_VOTE1_gpu_top_vote1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpu_f_vote_top4 : 8;
        unsigned int gpu_f_vote_top5 : 8;
        unsigned int gpu_top_vote2_msk : 16;
    } reg;
} SOC_PMCTRL_GPU_TOP_VOTE2_UNION;
#endif
#define SOC_PMCTRL_GPU_TOP_VOTE2_gpu_f_vote_top4_START (0)
#define SOC_PMCTRL_GPU_TOP_VOTE2_gpu_f_vote_top4_END (7)
#define SOC_PMCTRL_GPU_TOP_VOTE2_gpu_f_vote_top5_START (8)
#define SOC_PMCTRL_GPU_TOP_VOTE2_gpu_f_vote_top5_END (15)
#define SOC_PMCTRL_GPU_TOP_VOTE2_gpu_top_vote2_msk_START (16)
#define SOC_PMCTRL_GPU_TOP_VOTE2_gpu_top_vote2_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpu_f_vote_top6 : 8;
        unsigned int gpu_f_vote_top7 : 8;
        unsigned int gpu_top_vote3_msk : 16;
    } reg;
} SOC_PMCTRL_GPU_TOP_VOTE3_UNION;
#endif
#define SOC_PMCTRL_GPU_TOP_VOTE3_gpu_f_vote_top6_START (0)
#define SOC_PMCTRL_GPU_TOP_VOTE3_gpu_f_vote_top6_END (7)
#define SOC_PMCTRL_GPU_TOP_VOTE3_gpu_f_vote_top7_START (8)
#define SOC_PMCTRL_GPU_TOP_VOTE3_gpu_f_vote_top7_END (15)
#define SOC_PMCTRL_GPU_TOP_VOTE3_gpu_top_vote3_msk_START (16)
#define SOC_PMCTRL_GPU_TOP_VOTE3_gpu_top_vote3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddr_f_vote_top0 : 8;
        unsigned int ddr_f_vote_top1 : 8;
        unsigned int ddr_top_vote0_msk : 16;
    } reg;
} SOC_PMCTRL_DDR_TOP_VOTE0_UNION;
#endif
#define SOC_PMCTRL_DDR_TOP_VOTE0_ddr_f_vote_top0_START (0)
#define SOC_PMCTRL_DDR_TOP_VOTE0_ddr_f_vote_top0_END (7)
#define SOC_PMCTRL_DDR_TOP_VOTE0_ddr_f_vote_top1_START (8)
#define SOC_PMCTRL_DDR_TOP_VOTE0_ddr_f_vote_top1_END (15)
#define SOC_PMCTRL_DDR_TOP_VOTE0_ddr_top_vote0_msk_START (16)
#define SOC_PMCTRL_DDR_TOP_VOTE0_ddr_top_vote0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddr_f_vote_top2 : 8;
        unsigned int ddr_f_vote_top3 : 8;
        unsigned int ddr_top_vote1_msk : 16;
    } reg;
} SOC_PMCTRL_DDR_TOP_VOTE1_UNION;
#endif
#define SOC_PMCTRL_DDR_TOP_VOTE1_ddr_f_vote_top2_START (0)
#define SOC_PMCTRL_DDR_TOP_VOTE1_ddr_f_vote_top2_END (7)
#define SOC_PMCTRL_DDR_TOP_VOTE1_ddr_f_vote_top3_START (8)
#define SOC_PMCTRL_DDR_TOP_VOTE1_ddr_f_vote_top3_END (15)
#define SOC_PMCTRL_DDR_TOP_VOTE1_ddr_top_vote1_msk_START (16)
#define SOC_PMCTRL_DDR_TOP_VOTE1_ddr_top_vote1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddr_f_vote_top4 : 8;
        unsigned int ddr_f_vote_top5 : 8;
        unsigned int ddr_top_vote2_msk : 16;
    } reg;
} SOC_PMCTRL_DDR_TOP_VOTE2_UNION;
#endif
#define SOC_PMCTRL_DDR_TOP_VOTE2_ddr_f_vote_top4_START (0)
#define SOC_PMCTRL_DDR_TOP_VOTE2_ddr_f_vote_top4_END (7)
#define SOC_PMCTRL_DDR_TOP_VOTE2_ddr_f_vote_top5_START (8)
#define SOC_PMCTRL_DDR_TOP_VOTE2_ddr_f_vote_top5_END (15)
#define SOC_PMCTRL_DDR_TOP_VOTE2_ddr_top_vote2_msk_START (16)
#define SOC_PMCTRL_DDR_TOP_VOTE2_ddr_top_vote2_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddr_f_vote_top6 : 8;
        unsigned int ddr_f_vote_top7 : 8;
        unsigned int ddr_top_vote3_msk : 16;
    } reg;
} SOC_PMCTRL_DDR_TOP_VOTE3_UNION;
#endif
#define SOC_PMCTRL_DDR_TOP_VOTE3_ddr_f_vote_top6_START (0)
#define SOC_PMCTRL_DDR_TOP_VOTE3_ddr_f_vote_top6_END (7)
#define SOC_PMCTRL_DDR_TOP_VOTE3_ddr_f_vote_top7_START (8)
#define SOC_PMCTRL_DDR_TOP_VOTE3_ddr_f_vote_top7_END (15)
#define SOC_PMCTRL_DDR_TOP_VOTE3_ddr_top_vote3_msk_START (16)
#define SOC_PMCTRL_DDR_TOP_VOTE3_ddr_top_vote3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddr_f_vote_floor0 : 8;
        unsigned int ddr_f_vote_floor1 : 8;
        unsigned int ddr_floor_vote0_msk : 16;
    } reg;
} SOC_PMCTRL_DDR_FLOOR_VOTE0_UNION;
#endif
#define SOC_PMCTRL_DDR_FLOOR_VOTE0_ddr_f_vote_floor0_START (0)
#define SOC_PMCTRL_DDR_FLOOR_VOTE0_ddr_f_vote_floor0_END (7)
#define SOC_PMCTRL_DDR_FLOOR_VOTE0_ddr_f_vote_floor1_START (8)
#define SOC_PMCTRL_DDR_FLOOR_VOTE0_ddr_f_vote_floor1_END (15)
#define SOC_PMCTRL_DDR_FLOOR_VOTE0_ddr_floor_vote0_msk_START (16)
#define SOC_PMCTRL_DDR_FLOOR_VOTE0_ddr_floor_vote0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddr_f_vote_floor2 : 8;
        unsigned int ddr_f_vote_floor3 : 8;
        unsigned int ddr_floor_vote1_msk : 16;
    } reg;
} SOC_PMCTRL_DDR_FLOOR_VOTE1_UNION;
#endif
#define SOC_PMCTRL_DDR_FLOOR_VOTE1_ddr_f_vote_floor2_START (0)
#define SOC_PMCTRL_DDR_FLOOR_VOTE1_ddr_f_vote_floor2_END (7)
#define SOC_PMCTRL_DDR_FLOOR_VOTE1_ddr_f_vote_floor3_START (8)
#define SOC_PMCTRL_DDR_FLOOR_VOTE1_ddr_f_vote_floor3_END (15)
#define SOC_PMCTRL_DDR_FLOOR_VOTE1_ddr_floor_vote1_msk_START (16)
#define SOC_PMCTRL_DDR_FLOOR_VOTE1_ddr_floor_vote1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddr_f_vote_floor4 : 8;
        unsigned int ddr_f_vote_floor5 : 8;
        unsigned int ddr_floor_vote2_msk : 16;
    } reg;
} SOC_PMCTRL_DDR_FLOOR_VOTE2_UNION;
#endif
#define SOC_PMCTRL_DDR_FLOOR_VOTE2_ddr_f_vote_floor4_START (0)
#define SOC_PMCTRL_DDR_FLOOR_VOTE2_ddr_f_vote_floor4_END (7)
#define SOC_PMCTRL_DDR_FLOOR_VOTE2_ddr_f_vote_floor5_START (8)
#define SOC_PMCTRL_DDR_FLOOR_VOTE2_ddr_f_vote_floor5_END (15)
#define SOC_PMCTRL_DDR_FLOOR_VOTE2_ddr_floor_vote2_msk_START (16)
#define SOC_PMCTRL_DDR_FLOOR_VOTE2_ddr_floor_vote2_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddr_f_vote_floor6 : 8;
        unsigned int ddr_f_vote_floor7 : 8;
        unsigned int ddr_floor_vote3_msk : 16;
    } reg;
} SOC_PMCTRL_DDR_FLOOR_VOTE3_UNION;
#endif
#define SOC_PMCTRL_DDR_FLOOR_VOTE3_ddr_f_vote_floor6_START (0)
#define SOC_PMCTRL_DDR_FLOOR_VOTE3_ddr_f_vote_floor6_END (7)
#define SOC_PMCTRL_DDR_FLOOR_VOTE3_ddr_f_vote_floor7_START (8)
#define SOC_PMCTRL_DDR_FLOOR_VOTE3_ddr_f_vote_floor7_END (15)
#define SOC_PMCTRL_DDR_FLOOR_VOTE3_ddr_floor_vote3_msk_START (16)
#define SOC_PMCTRL_DDR_FLOOR_VOTE3_ddr_floor_vote3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int big_top_vote_type : 1;
        unsigned int little_top_vote_type : 1;
        unsigned int gpu_top_vote_type : 1;
        unsigned int ddr_top_vote_type : 1;
        unsigned int ddr_floor_vote_type : 1;
        unsigned int middle_top_vote_type : 1;
        unsigned int l3_top_vote_type : 1;
        unsigned int reserved : 9;
        unsigned int vote_mode_cfg_msk : 16;
    } reg;
} SOC_PMCTRL_VOTE_MODE_CFG_UNION;
#endif
#define SOC_PMCTRL_VOTE_MODE_CFG_big_top_vote_type_START (0)
#define SOC_PMCTRL_VOTE_MODE_CFG_big_top_vote_type_END (0)
#define SOC_PMCTRL_VOTE_MODE_CFG_little_top_vote_type_START (1)
#define SOC_PMCTRL_VOTE_MODE_CFG_little_top_vote_type_END (1)
#define SOC_PMCTRL_VOTE_MODE_CFG_gpu_top_vote_type_START (2)
#define SOC_PMCTRL_VOTE_MODE_CFG_gpu_top_vote_type_END (2)
#define SOC_PMCTRL_VOTE_MODE_CFG_ddr_top_vote_type_START (3)
#define SOC_PMCTRL_VOTE_MODE_CFG_ddr_top_vote_type_END (3)
#define SOC_PMCTRL_VOTE_MODE_CFG_ddr_floor_vote_type_START (4)
#define SOC_PMCTRL_VOTE_MODE_CFG_ddr_floor_vote_type_END (4)
#define SOC_PMCTRL_VOTE_MODE_CFG_middle_top_vote_type_START (5)
#define SOC_PMCTRL_VOTE_MODE_CFG_middle_top_vote_type_END (5)
#define SOC_PMCTRL_VOTE_MODE_CFG_l3_top_vote_type_START (6)
#define SOC_PMCTRL_VOTE_MODE_CFG_l3_top_vote_type_END (6)
#define SOC_PMCTRL_VOTE_MODE_CFG_vote_mode_cfg_msk_START (16)
#define SOC_PMCTRL_VOTE_MODE_CFG_vote_mode_cfg_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int big_top_vote_start_en : 1;
        unsigned int little_top_vote_start_en : 1;
        unsigned int gpu_top_vote_start_en : 1;
        unsigned int ddr_top_vote_start_en : 1;
        unsigned int ddr_floor_vote_start_en : 1;
        unsigned int middle_top_vote_start_en : 1;
        unsigned int l3_top_vote_start_en : 1;
        unsigned int reserved : 9;
        unsigned int vote_start_en_msk : 16;
    } reg;
} SOC_PMCTRL_VOTE_START_EN_UNION;
#endif
#define SOC_PMCTRL_VOTE_START_EN_big_top_vote_start_en_START (0)
#define SOC_PMCTRL_VOTE_START_EN_big_top_vote_start_en_END (0)
#define SOC_PMCTRL_VOTE_START_EN_little_top_vote_start_en_START (1)
#define SOC_PMCTRL_VOTE_START_EN_little_top_vote_start_en_END (1)
#define SOC_PMCTRL_VOTE_START_EN_gpu_top_vote_start_en_START (2)
#define SOC_PMCTRL_VOTE_START_EN_gpu_top_vote_start_en_END (2)
#define SOC_PMCTRL_VOTE_START_EN_ddr_top_vote_start_en_START (3)
#define SOC_PMCTRL_VOTE_START_EN_ddr_top_vote_start_en_END (3)
#define SOC_PMCTRL_VOTE_START_EN_ddr_floor_vote_start_en_START (4)
#define SOC_PMCTRL_VOTE_START_EN_ddr_floor_vote_start_en_END (4)
#define SOC_PMCTRL_VOTE_START_EN_middle_top_vote_start_en_START (5)
#define SOC_PMCTRL_VOTE_START_EN_middle_top_vote_start_en_END (5)
#define SOC_PMCTRL_VOTE_START_EN_l3_top_vote_start_en_START (6)
#define SOC_PMCTRL_VOTE_START_EN_l3_top_vote_start_en_END (6)
#define SOC_PMCTRL_VOTE_START_EN_vote_start_en_msk_START (16)
#define SOC_PMCTRL_VOTE_START_EN_vote_start_en_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int big_top_vote_int_en : 1;
        unsigned int little_top_vote_int_en : 1;
        unsigned int gpu_top_vote_int_en : 1;
        unsigned int ddr_top_vote_int_en : 1;
        unsigned int ddr_floor_vote_int_en : 1;
        unsigned int middle_top_vote_int_en : 1;
        unsigned int l3_top_vote_int_en : 1;
        unsigned int reserved : 9;
        unsigned int int_vote_en_msk : 16;
    } reg;
} SOC_PMCTRL_INT_VOTE_EN_UNION;
#endif
#define SOC_PMCTRL_INT_VOTE_EN_big_top_vote_int_en_START (0)
#define SOC_PMCTRL_INT_VOTE_EN_big_top_vote_int_en_END (0)
#define SOC_PMCTRL_INT_VOTE_EN_little_top_vote_int_en_START (1)
#define SOC_PMCTRL_INT_VOTE_EN_little_top_vote_int_en_END (1)
#define SOC_PMCTRL_INT_VOTE_EN_gpu_top_vote_int_en_START (2)
#define SOC_PMCTRL_INT_VOTE_EN_gpu_top_vote_int_en_END (2)
#define SOC_PMCTRL_INT_VOTE_EN_ddr_top_vote_int_en_START (3)
#define SOC_PMCTRL_INT_VOTE_EN_ddr_top_vote_int_en_END (3)
#define SOC_PMCTRL_INT_VOTE_EN_ddr_floor_vote_int_en_START (4)
#define SOC_PMCTRL_INT_VOTE_EN_ddr_floor_vote_int_en_END (4)
#define SOC_PMCTRL_INT_VOTE_EN_middle_top_vote_int_en_START (5)
#define SOC_PMCTRL_INT_VOTE_EN_middle_top_vote_int_en_END (5)
#define SOC_PMCTRL_INT_VOTE_EN_l3_top_vote_int_en_START (6)
#define SOC_PMCTRL_INT_VOTE_EN_l3_top_vote_int_en_END (6)
#define SOC_PMCTRL_INT_VOTE_EN_int_vote_en_msk_START (16)
#define SOC_PMCTRL_INT_VOTE_EN_int_vote_en_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_vote_clr : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_INT_VOTE_CLR_UNION;
#endif
#define SOC_PMCTRL_INT_VOTE_CLR_int_vote_clr_START (0)
#define SOC_PMCTRL_INT_VOTE_CLR_int_vote_clr_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_vote_raw_stat : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_INT_VOTE_RAW_STAT_UNION;
#endif
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_vote_raw_stat_START (0)
#define SOC_PMCTRL_INT_VOTE_RAW_STAT_int_vote_raw_stat_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_vote_stat : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_INT_VOTE_STAT_UNION;
#endif
#define SOC_PMCTRL_INT_VOTE_STAT_int_vote_stat_START (0)
#define SOC_PMCTRL_INT_VOTE_STAT_int_vote_stat_END (15)
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
        unsigned int voted_f_big_top : 8;
        unsigned int voted_f_little_top : 8;
        unsigned int voted_f_gpu_top : 8;
        unsigned int voted_f_middle_top : 8;
    } reg;
} SOC_PMCTRL_VOTE_RESULT0_UNION;
#endif
#define SOC_PMCTRL_VOTE_RESULT0_voted_f_big_top_START (0)
#define SOC_PMCTRL_VOTE_RESULT0_voted_f_big_top_END (7)
#define SOC_PMCTRL_VOTE_RESULT0_voted_f_little_top_START (8)
#define SOC_PMCTRL_VOTE_RESULT0_voted_f_little_top_END (15)
#define SOC_PMCTRL_VOTE_RESULT0_voted_f_gpu_top_START (16)
#define SOC_PMCTRL_VOTE_RESULT0_voted_f_gpu_top_END (23)
#define SOC_PMCTRL_VOTE_RESULT0_voted_f_middle_top_START (24)
#define SOC_PMCTRL_VOTE_RESULT0_voted_f_middle_top_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int voted_f_ddr_top : 8;
        unsigned int voted_f_ddr_floor : 8;
        unsigned int voted_f_l3_top : 8;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_VOTE_RESULT1_UNION;
#endif
#define SOC_PMCTRL_VOTE_RESULT1_voted_f_ddr_top_START (0)
#define SOC_PMCTRL_VOTE_RESULT1_voted_f_ddr_top_END (7)
#define SOC_PMCTRL_VOTE_RESULT1_voted_f_ddr_floor_START (8)
#define SOC_PMCTRL_VOTE_RESULT1_voted_f_ddr_floor_END (15)
#define SOC_PMCTRL_VOTE_RESULT1_voted_f_l3_top_START (16)
#define SOC_PMCTRL_VOTE_RESULT1_voted_f_l3_top_END (23)
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
        unsigned int dbg_ctrl_ddrc : 1;
        unsigned int dbg_ctrl_cssys : 1;
        unsigned int reserved_0 : 1;
        unsigned int dbg_ctrl_i2c0 : 1;
        unsigned int dbg_ctrl_i2c1 : 1;
        unsigned int reserved_1 : 1;
        unsigned int dbg_ctrl_pmc : 1;
        unsigned int dbg_ctrl_crg : 1;
        unsigned int reserved_2 : 24;
    } reg;
} SOC_PMCTRL_PERI_CTRL0_UNION;
#endif
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_ddrc_START (0)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_ddrc_END (0)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_cssys_START (1)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_cssys_END (1)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_i2c0_START (3)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_i2c0_END (3)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_i2c1_START (4)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_i2c1_END (4)
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
        unsigned int noc_lpm3_mst_paddr : 2;
        unsigned int noc_lpm3_mst_chan_mode : 2;
        unsigned int noc_lpm3_mst_n_gran : 3;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_PMCTRL_PERI_CTRL6_UNION;
#endif
#define SOC_PMCTRL_PERI_CTRL6_noc_lpm3_mst_paddr_START (0)
#define SOC_PMCTRL_PERI_CTRL6_noc_lpm3_mst_paddr_END (1)
#define SOC_PMCTRL_PERI_CTRL6_noc_lpm3_mst_chan_mode_START (2)
#define SOC_PMCTRL_PERI_CTRL6_noc_lpm3_mst_chan_mode_END (3)
#define SOC_PMCTRL_PERI_CTRL6_noc_lpm3_mst_n_gran_START (4)
#define SOC_PMCTRL_PERI_CTRL6_noc_lpm3_mst_n_gran_END (6)
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
        unsigned int ddrphy_bypass_mode : 1;
        unsigned int noc_mmc0bus_power_idlereq : 1;
        unsigned int noc_mmc1bus_power_idlereq : 1;
        unsigned int noc_modem_power_idlereq : 1;
        unsigned int noc_vcodec_bus_power_idlereq : 1;
        unsigned int noc_isp_power_idlereq : 1;
        unsigned int noc_sysbus_power_idlereq : 1;
        unsigned int noc_cfgbus_power_idlereq : 1;
        unsigned int noc_dmabus_power_idlereq : 1;
        unsigned int noc_ics_power_idlereq : 1;
        unsigned int noc_vdec_power_idlereq : 1;
        unsigned int noc_venc_power_idlereq : 1;
        unsigned int noc_ics2_power_idlereq : 1;
        unsigned int noc_dss_power_idlereq : 1;
        unsigned int noc_ivp32_power_idlereq : 1;
        unsigned int noc_vivobus_power_idlereq : 1;
        unsigned int biten : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEREQ_0_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_ddrphy_bypass_mode_START (0)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_ddrphy_bypass_mode_END (0)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_mmc0bus_power_idlereq_START (1)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_mmc0bus_power_idlereq_END (1)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_mmc1bus_power_idlereq_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_mmc1bus_power_idlereq_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_modem_power_idlereq_START (3)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_modem_power_idlereq_END (3)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_vcodec_bus_power_idlereq_START (4)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_vcodec_bus_power_idlereq_END (4)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_isp_power_idlereq_START (5)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_isp_power_idlereq_END (5)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_sysbus_power_idlereq_START (6)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_sysbus_power_idlereq_END (6)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_cfgbus_power_idlereq_START (7)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_cfgbus_power_idlereq_END (7)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_dmabus_power_idlereq_START (8)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_dmabus_power_idlereq_END (8)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_ics_power_idlereq_START (9)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_ics_power_idlereq_END (9)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_vdec_power_idlereq_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_vdec_power_idlereq_END (10)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_venc_power_idlereq_START (11)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_venc_power_idlereq_END (11)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_ics2_power_idlereq_START (12)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_ics2_power_idlereq_END (12)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_dss_power_idlereq_START (13)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_dss_power_idlereq_END (13)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_ivp32_power_idlereq_START (14)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_ivp32_power_idlereq_END (14)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_vivobus_power_idlereq_START (15)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_noc_vivobus_power_idlereq_END (15)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_biten_START (16)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_0_biten_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int noc_mmc0bus_power_idleack : 1;
        unsigned int noc_mmc1bus_power_idleack : 1;
        unsigned int noc_modem_power_idleack : 1;
        unsigned int noc_vcodec_bus_power_idleack : 1;
        unsigned int noc_isp_power_idleack : 1;
        unsigned int noc_sysbus_power_idleack : 1;
        unsigned int noc_cfgbus_power_idleack : 1;
        unsigned int noc_dmabus_power_idleack : 1;
        unsigned int noc_ics_power_idleack : 1;
        unsigned int noc_vdec_power_idleack : 1;
        unsigned int noc_venc_power_idleack : 1;
        unsigned int noc_ics2_power_idleack : 1;
        unsigned int noc_dss_power_idleack : 1;
        unsigned int noc_ivp32_power_idleack : 1;
        unsigned int noc_vivobus_power_idle : 1;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEACK_0_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_mmc0bus_power_idleack_START (1)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_mmc0bus_power_idleack_END (1)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_mmc1bus_power_idleack_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_mmc1bus_power_idleack_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_modem_power_idleack_START (3)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_modem_power_idleack_END (3)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_vcodec_bus_power_idleack_START (4)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_vcodec_bus_power_idleack_END (4)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_isp_power_idleack_START (5)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_isp_power_idleack_END (5)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_sysbus_power_idleack_START (6)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_sysbus_power_idleack_END (6)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_cfgbus_power_idleack_START (7)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_cfgbus_power_idleack_END (7)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_dmabus_power_idleack_START (8)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_dmabus_power_idleack_END (8)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_ics_power_idleack_START (9)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_ics_power_idleack_END (9)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_vdec_power_idleack_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_vdec_power_idleack_END (10)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_venc_power_idleack_START (11)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_venc_power_idleack_END (11)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_ics2_power_idleack_START (12)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_ics2_power_idleack_END (12)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_dss_power_idleack_START (13)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_dss_power_idleack_END (13)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_ivp32_power_idleack_START (14)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_ivp32_power_idleack_END (14)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_vivobus_power_idle_START (15)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_0_noc_vivobus_power_idle_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int noc_mmc0bus_power_idle : 1;
        unsigned int noc_mmc1bus_power_idle : 1;
        unsigned int noc_modem_power_idle : 1;
        unsigned int noc_vcodec_bus_power_idle : 1;
        unsigned int noc_isp_power_idle : 1;
        unsigned int noc_sysbus_power_idle : 1;
        unsigned int noc_cfgbus_power_idle : 1;
        unsigned int noc_dmabus_power_idle : 1;
        unsigned int noc_ics_power_idle : 1;
        unsigned int noc_vdec_power_idle : 1;
        unsigned int noc_venc_power_idle : 1;
        unsigned int noc_ics2_power_idle : 1;
        unsigned int noc_dss_power_idle : 1;
        unsigned int noc_ivp32_power_idle : 1;
        unsigned int noc_vivobus_power_idle : 1;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLE_0_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_mmc0bus_power_idle_START (1)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_mmc0bus_power_idle_END (1)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_mmc1bus_power_idle_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_mmc1bus_power_idle_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_modem_power_idle_START (3)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_modem_power_idle_END (3)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_vcodec_bus_power_idle_START (4)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_vcodec_bus_power_idle_END (4)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_isp_power_idle_START (5)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_isp_power_idle_END (5)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_sysbus_power_idle_START (6)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_sysbus_power_idle_END (6)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_cfgbus_power_idle_START (7)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_cfgbus_power_idle_END (7)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_dmabus_power_idle_START (8)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_dmabus_power_idle_END (8)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_ics_power_idle_START (9)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_ics_power_idle_END (9)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_vdec_power_idle_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_vdec_power_idle_END (10)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_venc_power_idle_START (11)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_venc_power_idle_END (11)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_ics2_power_idle_START (12)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_ics2_power_idle_END (12)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_dss_power_idle_START (13)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_dss_power_idle_END (13)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_ivp32_power_idle_START (14)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_ivp32_power_idle_END (14)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_vivobus_power_idle_START (15)
#define SOC_PMCTRL_NOC_POWER_IDLE_0_noc_vivobus_power_idle_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_pcie0_power_idlereq : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_hiseebus_power_idlereq : 1;
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
        unsigned int biten : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEREQ_1_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_pcie0_power_idlereq_START (0)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_pcie0_power_idlereq_END (0)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_hiseebus_power_idlereq_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_noc_hiseebus_power_idlereq_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_biten_START (16)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_1_biten_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_pcie0_power_idleack : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_hiseebus_power_idleack : 1;
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
        unsigned int reserved_14 : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEACK_1_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_pcie0_power_idleack_START (0)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_pcie0_power_idleack_END (0)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_hiseebus_power_idleack_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_1_noc_hiseebus_power_idleack_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_pcie0_power_idle : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_hiseebus_power_idle : 1;
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
        unsigned int reserved_14 : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLE_1_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_pcie0_power_idle_START (0)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_pcie0_power_idle_END (0)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_hiseebus_power_idle_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLE_1_noc_hiseebus_power_idle_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int noc_hkadc_ssi_t_maintimeout : 1;
        unsigned int noc_gpu_cfg_t_maintimeout : 1;
        unsigned int noc_lpm3_slv_t_maintimeout : 1;
        unsigned int noc_sys_peri0_cfg_t_maintimeout : 1;
        unsigned int noc_sys_peri1_cfg_t_maintimeout : 1;
        unsigned int noc_sys_peri2_cfg_t_maintimeout : 1;
        unsigned int noc_sys_peri3_cfg_t_maintimeout : 1;
        unsigned int reserved_1 : 1;
        unsigned int noc_dmac_cfg_t_maintimeout : 1;
        unsigned int reserved_2 : 1;
        unsigned int noc_hisee_cfg_t_maintimeout : 1;
        unsigned int noc_socp_cfg_t_maintimeout : 1;
        unsigned int noc_top_cssys_slv_cfg_t_maintimeout : 1;
        unsigned int noc_cci_cfg_t_maintimeout : 1;
        unsigned int noc_modem_bus_cfg_t_maintimeout : 1;
        unsigned int noc_usb_cfg_t_maintimeout : 1;
        unsigned int noc_ics2_cfg_t_maintimeout : 1;
        unsigned int noc_emmc0bus_cfg_t_maintimeout : 1;
        unsigned int noc_vcodec_cfg_t_maintimeout : 1;
        unsigned int noc_sys2hkmem_t_maintimeout : 1;
        unsigned int noc_mmc1bus_apb_cfg_t_maintimeout : 1;
        unsigned int noc_pcie_cfg_t_maintimeout : 1;
        unsigned int noc_cfg2media_t_maintimeout : 1;
        unsigned int noc_ufs_cfg_t_maintimeout : 1;
        unsigned int noc_dmss_apb_slv_t_maintimeout : 1;
        unsigned int noc_aobus_mmbuf_t_maitimeout : 1;
        unsigned int noc_asp_cfg_t_maintimeout : 1;
        unsigned int noc_ocbc_cfg_t_maintimeout : 1;
        unsigned int iomcu_ahb_slv_maintimeout : 1;
        unsigned int iomcu_apb_slv_maintimeout : 1;
        unsigned int noc_aon_spb_slv_t_maintimeout : 1;
    } reg;
} SOC_PMCTRL_PERI_INT0_MASK_UNION;
#endif
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
#define SOC_PMCTRL_PERI_INT0_MASK_noc_dmac_cfg_t_maintimeout_START (9)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_dmac_cfg_t_maintimeout_END (9)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_hisee_cfg_t_maintimeout_START (11)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_hisee_cfg_t_maintimeout_END (11)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_socp_cfg_t_maintimeout_START (12)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_socp_cfg_t_maintimeout_END (12)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_top_cssys_slv_cfg_t_maintimeout_START (13)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_top_cssys_slv_cfg_t_maintimeout_END (13)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_cci_cfg_t_maintimeout_START (14)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_cci_cfg_t_maintimeout_END (14)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_modem_bus_cfg_t_maintimeout_START (15)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_modem_bus_cfg_t_maintimeout_END (15)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_usb_cfg_t_maintimeout_START (16)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_usb_cfg_t_maintimeout_END (16)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_ics2_cfg_t_maintimeout_START (17)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_ics2_cfg_t_maintimeout_END (17)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_emmc0bus_cfg_t_maintimeout_START (18)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_emmc0bus_cfg_t_maintimeout_END (18)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_vcodec_cfg_t_maintimeout_START (19)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_vcodec_cfg_t_maintimeout_END (19)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_sys2hkmem_t_maintimeout_START (20)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_sys2hkmem_t_maintimeout_END (20)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_mmc1bus_apb_cfg_t_maintimeout_START (21)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_mmc1bus_apb_cfg_t_maintimeout_END (21)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_pcie_cfg_t_maintimeout_START (22)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_pcie_cfg_t_maintimeout_END (22)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_cfg2media_t_maintimeout_START (23)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_cfg2media_t_maintimeout_END (23)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_ufs_cfg_t_maintimeout_START (24)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_ufs_cfg_t_maintimeout_END (24)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_dmss_apb_slv_t_maintimeout_START (25)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_dmss_apb_slv_t_maintimeout_END (25)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_aobus_mmbuf_t_maitimeout_START (26)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_aobus_mmbuf_t_maitimeout_END (26)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_asp_cfg_t_maintimeout_START (27)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_asp_cfg_t_maintimeout_END (27)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_ocbc_cfg_t_maintimeout_START (28)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_ocbc_cfg_t_maintimeout_END (28)
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
        unsigned int reserved_0 : 1;
        unsigned int noc_hkadc_ssi_t_maintimeout : 1;
        unsigned int noc_gpu_cfg_t_maintimeout : 1;
        unsigned int noc_lpm3_slv_t_maintimeout : 1;
        unsigned int noc_sys_peri0_cfg_t_maintimeout : 1;
        unsigned int noc_sys_peri1_cfg_t_maintimeout : 1;
        unsigned int noc_sys_peri2_cfg_t_maintimeout : 1;
        unsigned int noc_sys_peri3_cfg_t_maintimeout : 1;
        unsigned int reserved_1 : 1;
        unsigned int noc_dmac_cfg_t_maintimeout : 1;
        unsigned int reserved_2 : 1;
        unsigned int noc_hisee_cfg_t_maintimeout : 1;
        unsigned int noc_socp_cfg_t_maintimeout : 1;
        unsigned int noc_top_cssys_slv_cfg_t_maintimeout : 1;
        unsigned int noc_cci_cfg_t_maintimeout : 1;
        unsigned int noc_modem_bus_cfg_t_maintimeout : 1;
        unsigned int noc_usb_cfg_t_maintimeout : 1;
        unsigned int noc_ics2_cfg_t_maintimeout : 1;
        unsigned int noc_emmc0bus_cfg_t_maintimeout : 1;
        unsigned int noc_vcodec_cfg_t_maintimeout : 1;
        unsigned int noc_sys2hkmem_t_maintimeout : 1;
        unsigned int noc_mmc1bus_apb_cfg_t_maintimeout : 1;
        unsigned int noc_pcie_cfg_t_maintimeout : 1;
        unsigned int noc_cfg2media_t_maintimeout : 1;
        unsigned int noc_ufs_cfg_t_maintimeout : 1;
        unsigned int noc_dmss_apb_slv_t_maintimeout : 1;
        unsigned int noc_aobus_mmbuf_t_maitimeout : 1;
        unsigned int noc_asp_cfg_t_maintimeout : 1;
        unsigned int noc_ocbc_cfg_t_maintimeout : 1;
        unsigned int iomcu_ahb_slv_maintimeout : 1;
        unsigned int iomcu_apb_slv_maintimeout : 1;
        unsigned int noc_aon_spb_slv_t_maintimeout : 1;
    } reg;
} SOC_PMCTRL_PERI_INT0_STAT_UNION;
#endif
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
#define SOC_PMCTRL_PERI_INT0_STAT_noc_dmac_cfg_t_maintimeout_START (9)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_dmac_cfg_t_maintimeout_END (9)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_hisee_cfg_t_maintimeout_START (11)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_hisee_cfg_t_maintimeout_END (11)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_socp_cfg_t_maintimeout_START (12)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_socp_cfg_t_maintimeout_END (12)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_top_cssys_slv_cfg_t_maintimeout_START (13)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_top_cssys_slv_cfg_t_maintimeout_END (13)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_cci_cfg_t_maintimeout_START (14)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_cci_cfg_t_maintimeout_END (14)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_modem_bus_cfg_t_maintimeout_START (15)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_modem_bus_cfg_t_maintimeout_END (15)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_usb_cfg_t_maintimeout_START (16)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_usb_cfg_t_maintimeout_END (16)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_ics2_cfg_t_maintimeout_START (17)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_ics2_cfg_t_maintimeout_END (17)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_emmc0bus_cfg_t_maintimeout_START (18)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_emmc0bus_cfg_t_maintimeout_END (18)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_vcodec_cfg_t_maintimeout_START (19)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_vcodec_cfg_t_maintimeout_END (19)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_sys2hkmem_t_maintimeout_START (20)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_sys2hkmem_t_maintimeout_END (20)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_mmc1bus_apb_cfg_t_maintimeout_START (21)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_mmc1bus_apb_cfg_t_maintimeout_END (21)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_pcie_cfg_t_maintimeout_START (22)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_pcie_cfg_t_maintimeout_END (22)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_cfg2media_t_maintimeout_START (23)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_cfg2media_t_maintimeout_END (23)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_ufs_cfg_t_maintimeout_START (24)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_ufs_cfg_t_maintimeout_END (24)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_dmss_apb_slv_t_maintimeout_START (25)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_dmss_apb_slv_t_maintimeout_END (25)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_aobus_mmbuf_t_maitimeout_START (26)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_aobus_mmbuf_t_maitimeout_END (26)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_asp_cfg_t_maintimeout_START (27)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_asp_cfg_t_maintimeout_END (27)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_ocbc_cfg_t_maintimeout_START (28)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_ocbc_cfg_t_maintimeout_END (28)
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
        unsigned int noc_dmcc_t_maintimeout : 1;
        unsigned int noc_dmcd_t_maintimeout : 1;
        unsigned int reserved : 22;
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
#define SOC_PMCTRL_PERI_INT1_MASK_noc_dmcc_t_maintimeout_START (8)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_dmcc_t_maintimeout_END (8)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_dmcd_t_maintimeout_START (9)
#define SOC_PMCTRL_PERI_INT1_MASK_noc_dmcd_t_maintimeout_END (9)
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
        unsigned int noc_dmcc_t_maintimeout : 1;
        unsigned int noc_dmcd_t_maintimeout : 1;
        unsigned int reserved : 22;
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
#define SOC_PMCTRL_PERI_INT1_STAT_noc_dmcc_t_maintimeout_START (8)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_dmcc_t_maintimeout_END (8)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_dmcd_t_maintimeout_START (9)
#define SOC_PMCTRL_PERI_INT1_STAT_noc_dmcd_t_maintimeout_END (9)
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
        unsigned int vs_ctrl_en_big : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VS_CTRL_EN_BIG_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_EN_BIG_vs_ctrl_en_big_START (0)
#define SOC_PMCTRL_VS_CTRL_EN_BIG_vs_ctrl_en_big_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_ctrl_bypass_big : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VS_CTRL_BYPASS_BIG_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_BYPASS_BIG_vs_ctrl_bypass_big_START (0)
#define SOC_PMCTRL_VS_CTRL_BYPASS_BIG_vs_ctrl_bypass_big_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vol_drop_en_big : 1;
        unsigned int big_cpu0_wfi_wfe_bypass : 1;
        unsigned int big_cpu1_wfi_wfe_bypass : 1;
        unsigned int big_cpu2_wfi_wfe_bypass : 1;
        unsigned int big_cpu3_wfi_wfe_bypass : 1;
        unsigned int big_l2_idle_div_mod : 2;
        unsigned int big_cfg_cnt_cpu_wake_quit : 8;
        unsigned int big_cfg_cnt_cpu_l2_idle_quit : 8;
        unsigned int big_cpu_wake_up_mode : 2;
        unsigned int big_cpu_l2_idle_switch_bypass : 1;
        unsigned int big_cpu_l2_idle_gt_en : 1;
        unsigned int big_dvfs_div_en : 1;
        unsigned int reserved : 1;
        unsigned int sel_ckmux_big_icg : 1;
        unsigned int cpu_clk_div_cfg_big : 2;
    } reg;
} SOC_PMCTRL_VS_CTRL_BIG_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_BIG_vol_drop_en_big_START (0)
#define SOC_PMCTRL_VS_CTRL_BIG_vol_drop_en_big_END (0)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cpu0_wfi_wfe_bypass_START (1)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cpu0_wfi_wfe_bypass_END (1)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cpu1_wfi_wfe_bypass_START (2)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cpu1_wfi_wfe_bypass_END (2)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cpu2_wfi_wfe_bypass_START (3)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cpu2_wfi_wfe_bypass_END (3)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cpu3_wfi_wfe_bypass_START (4)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cpu3_wfi_wfe_bypass_END (4)
#define SOC_PMCTRL_VS_CTRL_BIG_big_l2_idle_div_mod_START (5)
#define SOC_PMCTRL_VS_CTRL_BIG_big_l2_idle_div_mod_END (6)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cfg_cnt_cpu_wake_quit_START (7)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cfg_cnt_cpu_wake_quit_END (14)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cfg_cnt_cpu_l2_idle_quit_START (15)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cfg_cnt_cpu_l2_idle_quit_END (22)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cpu_wake_up_mode_START (23)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cpu_wake_up_mode_END (24)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cpu_l2_idle_switch_bypass_START (25)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cpu_l2_idle_switch_bypass_END (25)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cpu_l2_idle_gt_en_START (26)
#define SOC_PMCTRL_VS_CTRL_BIG_big_cpu_l2_idle_gt_en_END (26)
#define SOC_PMCTRL_VS_CTRL_BIG_big_dvfs_div_en_START (27)
#define SOC_PMCTRL_VS_CTRL_BIG_big_dvfs_div_en_END (27)
#define SOC_PMCTRL_VS_CTRL_BIG_sel_ckmux_big_icg_START (29)
#define SOC_PMCTRL_VS_CTRL_BIG_sel_ckmux_big_icg_END (29)
#define SOC_PMCTRL_VS_CTRL_BIG_cpu_clk_div_cfg_big_START (30)
#define SOC_PMCTRL_VS_CTRL_BIG_cpu_clk_div_cfg_big_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_data_limit_e_1 : 1;
        unsigned int svfd_off_time_step_1 : 1;
        unsigned int svfd_pulse_width_sel_1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int svfd_ulvt_ll_1 : 4;
        unsigned int svfd_ulvt_sl_1 : 4;
        unsigned int svfd_lvt_ll_1 : 4;
        unsigned int svfd_lvt_sl_1 : 4;
        unsigned int svfd_trim_1 : 2;
        unsigned int reserved_1 : 2;
        unsigned int svfd_d_rate_1 : 2;
        unsigned int svfd_off_mode_1 : 1;
        unsigned int svfd_div64_en_1 : 1;
        unsigned int svfd_cpm_period_1 : 1;
        unsigned int svfd_edge_sel_1 : 1;
        unsigned int svfd_cmp_data_mode_1 : 2;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL0_BIG_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_data_limit_e_1_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_data_limit_e_1_END (0)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_off_time_step_1_START (1)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_off_time_step_1_END (1)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_pulse_width_sel_1_START (2)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_pulse_width_sel_1_END (2)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_ulvt_ll_1_START (4)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_ulvt_ll_1_END (7)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_ulvt_sl_1_START (8)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_ulvt_sl_1_END (11)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_lvt_ll_1_START (12)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_lvt_ll_1_END (15)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_lvt_sl_1_START (16)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_lvt_sl_1_END (19)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_trim_1_START (20)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_trim_1_END (21)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_d_rate_1_START (24)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_d_rate_1_END (25)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_off_mode_1_START (26)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_off_mode_1_END (26)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_div64_en_1_START (27)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_div64_en_1_END (27)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_cpm_period_1_START (28)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_cpm_period_1_END (28)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_edge_sel_1_START (29)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_edge_sel_1_END (29)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_cmp_data_mode_1_START (30)
#define SOC_PMCTRL_VS_SVFD_CTRL0_BIG_svfd_cmp_data_mode_1_END (31)
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
} SOC_PMCTRL_VS_SVFD_CTRL1_BIG_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL1_BIG_svfd_glitch_test_1_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL1_BIG_svfd_glitch_test_1_END (0)
#define SOC_PMCTRL_VS_SVFD_CTRL1_BIG_svfd_test_ffs_1_START (1)
#define SOC_PMCTRL_VS_SVFD_CTRL1_BIG_svfd_test_ffs_1_END (2)
#define SOC_PMCTRL_VS_SVFD_CTRL1_BIG_svfd_test_cmp_1_START (3)
#define SOC_PMCTRL_VS_SVFD_CTRL1_BIG_svfd_test_cmp_1_END (9)
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
        unsigned int vs_svfd_ctrl2_big_msk : 16;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL2_BIG_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL2_BIG_svfd_enalbe_mode_1_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL2_BIG_svfd_enalbe_mode_1_END (1)
#define SOC_PMCTRL_VS_SVFD_CTRL2_BIG_svfs_cpm_threshold_r_1_START (2)
#define SOC_PMCTRL_VS_SVFD_CTRL2_BIG_svfs_cpm_threshold_r_1_END (7)
#define SOC_PMCTRL_VS_SVFD_CTRL2_BIG_svfs_cpm_threshold_f_1_START (8)
#define SOC_PMCTRL_VS_SVFD_CTRL2_BIG_svfs_cpm_threshold_f_1_END (13)
#define SOC_PMCTRL_VS_SVFD_CTRL2_BIG_vs_svfd_ctrl2_big_msk_START (16)
#define SOC_PMCTRL_VS_SVFD_CTRL2_BIG_vs_svfd_ctrl2_big_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_cpm_data_f_big : 8;
        unsigned int svfd_cpm_data_r_big : 8;
        unsigned int svfd_dll_lock_big : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_PMCTRL_VS_CPM_DATA_STAT_BIG_UNION;
#endif
#define SOC_PMCTRL_VS_CPM_DATA_STAT_BIG_svfd_cpm_data_f_big_START (0)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_BIG_svfd_cpm_data_f_big_END (7)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_BIG_svfd_cpm_data_r_big_START (8)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_BIG_svfd_cpm_data_r_big_END (15)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_BIG_svfd_dll_lock_big_START (16)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_BIG_svfd_dll_lock_big_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_test_out_ffs_big : 4;
        unsigned int svfd_test_out_cpm_big : 4;
        unsigned int svfd_glitch_result_big : 1;
        unsigned int idle_low_freq_en_big : 1;
        unsigned int vbat_drop_protect_ind_big : 1;
        unsigned int reserved : 21;
    } reg;
} SOC_PMCTRL_VS_TEST_STAT_BIG_UNION;
#endif
#define SOC_PMCTRL_VS_TEST_STAT_BIG_svfd_test_out_ffs_big_START (0)
#define SOC_PMCTRL_VS_TEST_STAT_BIG_svfd_test_out_ffs_big_END (3)
#define SOC_PMCTRL_VS_TEST_STAT_BIG_svfd_test_out_cpm_big_START (4)
#define SOC_PMCTRL_VS_TEST_STAT_BIG_svfd_test_out_cpm_big_END (7)
#define SOC_PMCTRL_VS_TEST_STAT_BIG_svfd_glitch_result_big_START (8)
#define SOC_PMCTRL_VS_TEST_STAT_BIG_svfd_glitch_result_big_END (8)
#define SOC_PMCTRL_VS_TEST_STAT_BIG_idle_low_freq_en_big_START (9)
#define SOC_PMCTRL_VS_TEST_STAT_BIG_idle_low_freq_en_big_END (9)
#define SOC_PMCTRL_VS_TEST_STAT_BIG_vbat_drop_protect_ind_big_START (10)
#define SOC_PMCTRL_VS_TEST_STAT_BIG_vbat_drop_protect_ind_big_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_ctrl_en_middle : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VS_CTRL_EN_MIDDLE_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_EN_MIDDLE_vs_ctrl_en_middle_START (0)
#define SOC_PMCTRL_VS_CTRL_EN_MIDDLE_vs_ctrl_en_middle_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_ctrl_bypass_middle : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VS_CTRL_BYPASS_MIDDLE_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_BYPASS_MIDDLE_vs_ctrl_bypass_middle_START (0)
#define SOC_PMCTRL_VS_CTRL_BYPASS_MIDDLE_vs_ctrl_bypass_middle_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vol_drop_en_middle : 1;
        unsigned int middle_cpu0_wfi_wfe_bypass : 1;
        unsigned int middle_cpu1_wfi_wfe_bypass : 1;
        unsigned int middle_cpu2_wfi_wfe_bypass : 1;
        unsigned int middle_cpu3_wfi_wfe_bypass : 1;
        unsigned int middle_l2_idle_div_mod : 2;
        unsigned int middle_cfg_cnt_cpu_wake_quit : 8;
        unsigned int middle_cfg_cnt_cpu_l2_idle_quit : 8;
        unsigned int middle_cpu_wake_up_mode : 2;
        unsigned int middle_cpu_l2_idle_switch_bypass : 1;
        unsigned int middle_cpu_l2_idle_gt_en : 1;
        unsigned int middle_dvfs_div_en : 1;
        unsigned int reserved : 1;
        unsigned int sel_ckmux_middle_icg : 1;
        unsigned int cpu_clk_div_cfg_middle : 2;
    } reg;
} SOC_PMCTRL_VS_CTRL_MIDDLE_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_MIDDLE_vol_drop_en_middle_START (0)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_vol_drop_en_middle_END (0)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cpu0_wfi_wfe_bypass_START (1)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cpu0_wfi_wfe_bypass_END (1)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cpu1_wfi_wfe_bypass_START (2)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cpu1_wfi_wfe_bypass_END (2)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cpu2_wfi_wfe_bypass_START (3)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cpu2_wfi_wfe_bypass_END (3)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cpu3_wfi_wfe_bypass_START (4)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cpu3_wfi_wfe_bypass_END (4)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_l2_idle_div_mod_START (5)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_l2_idle_div_mod_END (6)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cfg_cnt_cpu_wake_quit_START (7)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cfg_cnt_cpu_wake_quit_END (14)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cfg_cnt_cpu_l2_idle_quit_START (15)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cfg_cnt_cpu_l2_idle_quit_END (22)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cpu_wake_up_mode_START (23)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cpu_wake_up_mode_END (24)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cpu_l2_idle_switch_bypass_START (25)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cpu_l2_idle_switch_bypass_END (25)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cpu_l2_idle_gt_en_START (26)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_cpu_l2_idle_gt_en_END (26)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_dvfs_div_en_START (27)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_middle_dvfs_div_en_END (27)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_sel_ckmux_middle_icg_START (29)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_sel_ckmux_middle_icg_END (29)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_cpu_clk_div_cfg_middle_START (30)
#define SOC_PMCTRL_VS_CTRL_MIDDLE_cpu_clk_div_cfg_middle_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_data_limit_e_middle : 1;
        unsigned int svfd_off_time_step_middle : 1;
        unsigned int svfd_pulse_width_sel_middle : 1;
        unsigned int reserved_0 : 1;
        unsigned int svfd_ulvt_ll_middle : 4;
        unsigned int svfd_ulvt_sl_middle : 4;
        unsigned int svfd_lvt_ll_middle : 4;
        unsigned int svfd_lvt_sl_middle : 4;
        unsigned int svfd_trim_middle : 2;
        unsigned int reserved_1 : 2;
        unsigned int svfd_d_rate_middle : 2;
        unsigned int svfd_off_mode_middle : 1;
        unsigned int svfd_div64_en_middle : 1;
        unsigned int svfd_cpm_period_middle : 1;
        unsigned int svfd_edge_sel_middle : 1;
        unsigned int svfd_cmp_data_mode_middle : 2;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_data_limit_e_middle_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_data_limit_e_middle_END (0)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_off_time_step_middle_START (1)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_off_time_step_middle_END (1)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_pulse_width_sel_middle_START (2)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_pulse_width_sel_middle_END (2)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_ulvt_ll_middle_START (4)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_ulvt_ll_middle_END (7)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_ulvt_sl_middle_START (8)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_ulvt_sl_middle_END (11)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_lvt_ll_middle_START (12)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_lvt_ll_middle_END (15)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_lvt_sl_middle_START (16)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_lvt_sl_middle_END (19)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_trim_middle_START (20)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_trim_middle_END (21)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_d_rate_middle_START (24)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_d_rate_middle_END (25)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_off_mode_middle_START (26)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_off_mode_middle_END (26)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_div64_en_middle_START (27)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_div64_en_middle_END (27)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_cpm_period_middle_START (28)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_cpm_period_middle_END (28)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_edge_sel_middle_START (29)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_edge_sel_middle_END (29)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_cmp_data_mode_middle_START (30)
#define SOC_PMCTRL_VS_SVFD_CTRL0_MIDDLE_svfd_cmp_data_mode_middle_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_glitch_test_middle : 1;
        unsigned int svfd_test_ffs_middle : 2;
        unsigned int svfd_test_cmp_middle : 7;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 19;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL1_MIDDLE_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL1_MIDDLE_svfd_glitch_test_middle_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL1_MIDDLE_svfd_glitch_test_middle_END (0)
#define SOC_PMCTRL_VS_SVFD_CTRL1_MIDDLE_svfd_test_ffs_middle_START (1)
#define SOC_PMCTRL_VS_SVFD_CTRL1_MIDDLE_svfd_test_ffs_middle_END (2)
#define SOC_PMCTRL_VS_SVFD_CTRL1_MIDDLE_svfd_test_cmp_middle_START (3)
#define SOC_PMCTRL_VS_SVFD_CTRL1_MIDDLE_svfd_test_cmp_middle_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_enalbe_mode_middle : 2;
        unsigned int svfs_cpm_threshold_r_middle : 6;
        unsigned int svfs_cpm_threshold_f_middle : 6;
        unsigned int reserved : 2;
        unsigned int vs_svfd_ctrl2_middle_msk : 16;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL2_MIDDLE_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL2_MIDDLE_svfd_enalbe_mode_middle_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL2_MIDDLE_svfd_enalbe_mode_middle_END (1)
#define SOC_PMCTRL_VS_SVFD_CTRL2_MIDDLE_svfs_cpm_threshold_r_middle_START (2)
#define SOC_PMCTRL_VS_SVFD_CTRL2_MIDDLE_svfs_cpm_threshold_r_middle_END (7)
#define SOC_PMCTRL_VS_SVFD_CTRL2_MIDDLE_svfs_cpm_threshold_f_middle_START (8)
#define SOC_PMCTRL_VS_SVFD_CTRL2_MIDDLE_svfs_cpm_threshold_f_middle_END (13)
#define SOC_PMCTRL_VS_SVFD_CTRL2_MIDDLE_vs_svfd_ctrl2_middle_msk_START (16)
#define SOC_PMCTRL_VS_SVFD_CTRL2_MIDDLE_vs_svfd_ctrl2_middle_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_cpm_data_f_middle : 8;
        unsigned int svfd_cpm_data_r_middle : 8;
        unsigned int svfd_dll_lock_middle : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_PMCTRL_VS_CPM_DATA_STAT_MIDDLE_UNION;
#endif
#define SOC_PMCTRL_VS_CPM_DATA_STAT_MIDDLE_svfd_cpm_data_f_middle_START (0)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_MIDDLE_svfd_cpm_data_f_middle_END (7)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_MIDDLE_svfd_cpm_data_r_middle_START (8)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_MIDDLE_svfd_cpm_data_r_middle_END (15)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_MIDDLE_svfd_dll_lock_middle_START (16)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_MIDDLE_svfd_dll_lock_middle_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_test_out_ffs_middle : 4;
        unsigned int svfd_test_out_cpm_middle : 4;
        unsigned int svfd_glitch_result_middle : 1;
        unsigned int idle_low_freq_en_middle : 1;
        unsigned int vbat_drop_protect_ind_middle : 1;
        unsigned int reserved : 21;
    } reg;
} SOC_PMCTRL_VS_TEST_STAT_MIDDLE_UNION;
#endif
#define SOC_PMCTRL_VS_TEST_STAT_MIDDLE_svfd_test_out_ffs_middle_START (0)
#define SOC_PMCTRL_VS_TEST_STAT_MIDDLE_svfd_test_out_ffs_middle_END (3)
#define SOC_PMCTRL_VS_TEST_STAT_MIDDLE_svfd_test_out_cpm_middle_START (4)
#define SOC_PMCTRL_VS_TEST_STAT_MIDDLE_svfd_test_out_cpm_middle_END (7)
#define SOC_PMCTRL_VS_TEST_STAT_MIDDLE_svfd_glitch_result_middle_START (8)
#define SOC_PMCTRL_VS_TEST_STAT_MIDDLE_svfd_glitch_result_middle_END (8)
#define SOC_PMCTRL_VS_TEST_STAT_MIDDLE_idle_low_freq_en_middle_START (9)
#define SOC_PMCTRL_VS_TEST_STAT_MIDDLE_idle_low_freq_en_middle_END (9)
#define SOC_PMCTRL_VS_TEST_STAT_MIDDLE_vbat_drop_protect_ind_middle_START (10)
#define SOC_PMCTRL_VS_TEST_STAT_MIDDLE_vbat_drop_protect_ind_middle_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_ctrl_en_little : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VS_CTRL_EN_LITTLE_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_EN_LITTLE_vs_ctrl_en_little_START (0)
#define SOC_PMCTRL_VS_CTRL_EN_LITTLE_vs_ctrl_en_little_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_ctrl_bypass_little : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VS_CTRL_BYPASS_LITTLE_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_BYPASS_LITTLE_vs_ctrl_bypass_little_START (0)
#define SOC_PMCTRL_VS_CTRL_BYPASS_LITTLE_vs_ctrl_bypass_little_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vol_drop_en_little : 1;
        unsigned int little_cpu0_wfi_wfe_bypass : 1;
        unsigned int little_cpu1_wfi_wfe_bypass : 1;
        unsigned int little_cpu2_wfi_wfe_bypass : 1;
        unsigned int little_cpu3_wfi_wfe_bypass : 1;
        unsigned int little_l2_idle_div_mod : 2;
        unsigned int little_cfg_cnt_cpu_wake_quit : 8;
        unsigned int little_cfg_cnt_cpu_l2_idle_quit : 8;
        unsigned int little_cpu_wake_up_mode : 2;
        unsigned int little_cpu_l2_idle_switch_bypass : 1;
        unsigned int little_cpu_l2_idle_gt_en : 1;
        unsigned int little_dvfs_div_en : 1;
        unsigned int reserved : 1;
        unsigned int sel_ckmux_little_icg : 1;
        unsigned int cpu_clk_div_cfg_little : 2;
    } reg;
} SOC_PMCTRL_VS_CTRL_LITTLE_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_LITTLE_vol_drop_en_little_START (0)
#define SOC_PMCTRL_VS_CTRL_LITTLE_vol_drop_en_little_END (0)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cpu0_wfi_wfe_bypass_START (1)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cpu0_wfi_wfe_bypass_END (1)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cpu1_wfi_wfe_bypass_START (2)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cpu1_wfi_wfe_bypass_END (2)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cpu2_wfi_wfe_bypass_START (3)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cpu2_wfi_wfe_bypass_END (3)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cpu3_wfi_wfe_bypass_START (4)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cpu3_wfi_wfe_bypass_END (4)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_l2_idle_div_mod_START (5)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_l2_idle_div_mod_END (6)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cfg_cnt_cpu_wake_quit_START (7)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cfg_cnt_cpu_wake_quit_END (14)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cfg_cnt_cpu_l2_idle_quit_START (15)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cfg_cnt_cpu_l2_idle_quit_END (22)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cpu_wake_up_mode_START (23)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cpu_wake_up_mode_END (24)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cpu_l2_idle_switch_bypass_START (25)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cpu_l2_idle_switch_bypass_END (25)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cpu_l2_idle_gt_en_START (26)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_cpu_l2_idle_gt_en_END (26)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_dvfs_div_en_START (27)
#define SOC_PMCTRL_VS_CTRL_LITTLE_little_dvfs_div_en_END (27)
#define SOC_PMCTRL_VS_CTRL_LITTLE_sel_ckmux_little_icg_START (29)
#define SOC_PMCTRL_VS_CTRL_LITTLE_sel_ckmux_little_icg_END (29)
#define SOC_PMCTRL_VS_CTRL_LITTLE_cpu_clk_div_cfg_little_START (30)
#define SOC_PMCTRL_VS_CTRL_LITTLE_cpu_clk_div_cfg_little_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_data_limit_e_little : 1;
        unsigned int svfd_off_time_step_little : 1;
        unsigned int svfd_pulse_width_sel_little : 1;
        unsigned int reserved_0 : 1;
        unsigned int svfd_ulvt_ll_little : 4;
        unsigned int svfd_ulvt_sl_little : 4;
        unsigned int svfd_lvt_ll_little : 4;
        unsigned int svfd_lvt_sl_little : 4;
        unsigned int svfd_trim_little : 2;
        unsigned int reserved_1 : 2;
        unsigned int svfd_d_rate_little : 2;
        unsigned int svfd_off_mode_little : 1;
        unsigned int svfd_div64_en_little : 1;
        unsigned int svfd_cpm_period_little : 1;
        unsigned int svfd_edge_sel_little : 1;
        unsigned int svfd_cmp_data_mode_little : 2;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_data_limit_e_little_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_data_limit_e_little_END (0)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_off_time_step_little_START (1)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_off_time_step_little_END (1)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_pulse_width_sel_little_START (2)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_pulse_width_sel_little_END (2)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_ulvt_ll_little_START (4)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_ulvt_ll_little_END (7)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_ulvt_sl_little_START (8)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_ulvt_sl_little_END (11)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_lvt_ll_little_START (12)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_lvt_ll_little_END (15)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_lvt_sl_little_START (16)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_lvt_sl_little_END (19)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_trim_little_START (20)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_trim_little_END (21)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_d_rate_little_START (24)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_d_rate_little_END (25)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_off_mode_little_START (26)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_off_mode_little_END (26)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_div64_en_little_START (27)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_div64_en_little_END (27)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_cpm_period_little_START (28)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_cpm_period_little_END (28)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_edge_sel_little_START (29)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_edge_sel_little_END (29)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_cmp_data_mode_little_START (30)
#define SOC_PMCTRL_VS_SVFD_CTRL0_LITTLE_svfd_cmp_data_mode_little_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_glitch_test_little : 1;
        unsigned int svfd_test_ffs_little : 2;
        unsigned int svfd_test_cmp_little : 7;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 19;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL1_LITTLE_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL1_LITTLE_svfd_glitch_test_little_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL1_LITTLE_svfd_glitch_test_little_END (0)
#define SOC_PMCTRL_VS_SVFD_CTRL1_LITTLE_svfd_test_ffs_little_START (1)
#define SOC_PMCTRL_VS_SVFD_CTRL1_LITTLE_svfd_test_ffs_little_END (2)
#define SOC_PMCTRL_VS_SVFD_CTRL1_LITTLE_svfd_test_cmp_little_START (3)
#define SOC_PMCTRL_VS_SVFD_CTRL1_LITTLE_svfd_test_cmp_little_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_enalbe_mode_little : 2;
        unsigned int svfs_cpm_threshold_r_little : 6;
        unsigned int svfs_cpm_threshold_f_little : 6;
        unsigned int reserved : 2;
        unsigned int vs_svfd_ctrl2_little_msk : 16;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL2_LITTLE_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL2_LITTLE_svfd_enalbe_mode_little_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL2_LITTLE_svfd_enalbe_mode_little_END (1)
#define SOC_PMCTRL_VS_SVFD_CTRL2_LITTLE_svfs_cpm_threshold_r_little_START (2)
#define SOC_PMCTRL_VS_SVFD_CTRL2_LITTLE_svfs_cpm_threshold_r_little_END (7)
#define SOC_PMCTRL_VS_SVFD_CTRL2_LITTLE_svfs_cpm_threshold_f_little_START (8)
#define SOC_PMCTRL_VS_SVFD_CTRL2_LITTLE_svfs_cpm_threshold_f_little_END (13)
#define SOC_PMCTRL_VS_SVFD_CTRL2_LITTLE_vs_svfd_ctrl2_little_msk_START (16)
#define SOC_PMCTRL_VS_SVFD_CTRL2_LITTLE_vs_svfd_ctrl2_little_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_cpm_data_f_little : 8;
        unsigned int svfd_cpm_data_r_little : 8;
        unsigned int svfd_dll_lock_little : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_PMCTRL_VS_CPM_DATA_STAT_LITTLE_UNION;
#endif
#define SOC_PMCTRL_VS_CPM_DATA_STAT_LITTLE_svfd_cpm_data_f_little_START (0)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_LITTLE_svfd_cpm_data_f_little_END (7)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_LITTLE_svfd_cpm_data_r_little_START (8)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_LITTLE_svfd_cpm_data_r_little_END (15)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_LITTLE_svfd_dll_lock_little_START (16)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_LITTLE_svfd_dll_lock_little_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_test_out_ffs_little : 4;
        unsigned int svfd_test_out_cpm_little : 4;
        unsigned int svfd_glitch_result_little : 1;
        unsigned int idle_low_freq_en_little : 1;
        unsigned int vbat_drop_protect_ind_little : 1;
        unsigned int reserved : 21;
    } reg;
} SOC_PMCTRL_VS_TEST_STAT_LITTLE_UNION;
#endif
#define SOC_PMCTRL_VS_TEST_STAT_LITTLE_svfd_test_out_ffs_little_START (0)
#define SOC_PMCTRL_VS_TEST_STAT_LITTLE_svfd_test_out_ffs_little_END (3)
#define SOC_PMCTRL_VS_TEST_STAT_LITTLE_svfd_test_out_cpm_little_START (4)
#define SOC_PMCTRL_VS_TEST_STAT_LITTLE_svfd_test_out_cpm_little_END (7)
#define SOC_PMCTRL_VS_TEST_STAT_LITTLE_svfd_glitch_result_little_START (8)
#define SOC_PMCTRL_VS_TEST_STAT_LITTLE_svfd_glitch_result_little_END (8)
#define SOC_PMCTRL_VS_TEST_STAT_LITTLE_idle_low_freq_en_little_START (9)
#define SOC_PMCTRL_VS_TEST_STAT_LITTLE_idle_low_freq_en_little_END (9)
#define SOC_PMCTRL_VS_TEST_STAT_LITTLE_vbat_drop_protect_ind_little_START (10)
#define SOC_PMCTRL_VS_TEST_STAT_LITTLE_vbat_drop_protect_ind_little_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_ctrl_en_l3 : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VS_CTRL_EN_L3_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_EN_L3_vs_ctrl_en_l3_START (0)
#define SOC_PMCTRL_VS_CTRL_EN_L3_vs_ctrl_en_l3_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_ctrl_bypass_l3 : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VS_CTRL_BYPASS_L3_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_BYPASS_L3_vs_ctrl_bypass_l3_START (0)
#define SOC_PMCTRL_VS_CTRL_BYPASS_L3_vs_ctrl_bypass_l3_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vol_drop_en_l3 : 1;
        unsigned int l3_cpu0_wfi_wfe_bypass : 1;
        unsigned int l3_cpu1_wfi_wfe_bypass : 1;
        unsigned int l3_cpu2_wfi_wfe_bypass : 1;
        unsigned int l3_cpu3_wfi_wfe_bypass : 1;
        unsigned int l3_l2_idle_div_mod : 2;
        unsigned int l3_cfg_cnt_cpu_wake_quit : 8;
        unsigned int l3_cfg_cnt_cpu_l2_idle_quit : 8;
        unsigned int l3_cpu_wake_up_mode : 2;
        unsigned int l3_cpu_l2_idle_switch_bypass : 1;
        unsigned int l3_cpu_l2_idle_gt_en : 1;
        unsigned int l3_dvfs_div_en : 1;
        unsigned int reserved : 1;
        unsigned int sel_ckmux_l3_icg : 1;
        unsigned int cpu_clk_div_cfg_l3 : 2;
    } reg;
} SOC_PMCTRL_VS_CTRL_L3_UNION;
#endif
#define SOC_PMCTRL_VS_CTRL_L3_vol_drop_en_l3_START (0)
#define SOC_PMCTRL_VS_CTRL_L3_vol_drop_en_l3_END (0)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cpu0_wfi_wfe_bypass_START (1)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cpu0_wfi_wfe_bypass_END (1)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cpu1_wfi_wfe_bypass_START (2)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cpu1_wfi_wfe_bypass_END (2)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cpu2_wfi_wfe_bypass_START (3)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cpu2_wfi_wfe_bypass_END (3)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cpu3_wfi_wfe_bypass_START (4)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cpu3_wfi_wfe_bypass_END (4)
#define SOC_PMCTRL_VS_CTRL_L3_l3_l2_idle_div_mod_START (5)
#define SOC_PMCTRL_VS_CTRL_L3_l3_l2_idle_div_mod_END (6)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cfg_cnt_cpu_wake_quit_START (7)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cfg_cnt_cpu_wake_quit_END (14)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cfg_cnt_cpu_l2_idle_quit_START (15)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cfg_cnt_cpu_l2_idle_quit_END (22)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cpu_wake_up_mode_START (23)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cpu_wake_up_mode_END (24)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cpu_l2_idle_switch_bypass_START (25)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cpu_l2_idle_switch_bypass_END (25)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cpu_l2_idle_gt_en_START (26)
#define SOC_PMCTRL_VS_CTRL_L3_l3_cpu_l2_idle_gt_en_END (26)
#define SOC_PMCTRL_VS_CTRL_L3_l3_dvfs_div_en_START (27)
#define SOC_PMCTRL_VS_CTRL_L3_l3_dvfs_div_en_END (27)
#define SOC_PMCTRL_VS_CTRL_L3_sel_ckmux_l3_icg_START (29)
#define SOC_PMCTRL_VS_CTRL_L3_sel_ckmux_l3_icg_END (29)
#define SOC_PMCTRL_VS_CTRL_L3_cpu_clk_div_cfg_l3_START (30)
#define SOC_PMCTRL_VS_CTRL_L3_cpu_clk_div_cfg_l3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_data_limit_e_l3 : 1;
        unsigned int svfd_off_time_step_l3 : 1;
        unsigned int svfd_pulse_width_sel_l3 : 1;
        unsigned int reserved_0 : 1;
        unsigned int svfd_ulvt_ll_l3 : 4;
        unsigned int svfd_ulvt_sl_l3 : 4;
        unsigned int svfd_lvt_ll_l3 : 4;
        unsigned int svfd_lvt_sl_l3 : 4;
        unsigned int svfd_trim_l3 : 2;
        unsigned int reserved_1 : 2;
        unsigned int svfd_d_rate_l3 : 2;
        unsigned int svfd_off_mode_l3 : 1;
        unsigned int svfd_div64_en_l3 : 1;
        unsigned int svfd_cpm_period_l3 : 1;
        unsigned int svfd_edge_sel_l3 : 1;
        unsigned int svfd_cmp_data_mode_l3 : 2;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL0_L3_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_data_limit_e_l3_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_data_limit_e_l3_END (0)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_off_time_step_l3_START (1)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_off_time_step_l3_END (1)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_pulse_width_sel_l3_START (2)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_pulse_width_sel_l3_END (2)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_ulvt_ll_l3_START (4)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_ulvt_ll_l3_END (7)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_ulvt_sl_l3_START (8)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_ulvt_sl_l3_END (11)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_lvt_ll_l3_START (12)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_lvt_ll_l3_END (15)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_lvt_sl_l3_START (16)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_lvt_sl_l3_END (19)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_trim_l3_START (20)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_trim_l3_END (21)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_d_rate_l3_START (24)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_d_rate_l3_END (25)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_off_mode_l3_START (26)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_off_mode_l3_END (26)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_div64_en_l3_START (27)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_div64_en_l3_END (27)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_cpm_period_l3_START (28)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_cpm_period_l3_END (28)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_edge_sel_l3_START (29)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_edge_sel_l3_END (29)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_cmp_data_mode_l3_START (30)
#define SOC_PMCTRL_VS_SVFD_CTRL0_L3_svfd_cmp_data_mode_l3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_glitch_test_l3 : 1;
        unsigned int svfd_test_ffs_l3 : 2;
        unsigned int svfd_test_cmp_l3 : 7;
        unsigned int reserved_0 : 1;
        unsigned int lpm_little_ctrl : 5;
        unsigned int lpm_middle_ctrl : 5;
        unsigned int lpm_big_ctrl : 5;
        unsigned int reserved_1 : 6;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL1_L3_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL1_L3_svfd_glitch_test_l3_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL1_L3_svfd_glitch_test_l3_END (0)
#define SOC_PMCTRL_VS_SVFD_CTRL1_L3_svfd_test_ffs_l3_START (1)
#define SOC_PMCTRL_VS_SVFD_CTRL1_L3_svfd_test_ffs_l3_END (2)
#define SOC_PMCTRL_VS_SVFD_CTRL1_L3_svfd_test_cmp_l3_START (3)
#define SOC_PMCTRL_VS_SVFD_CTRL1_L3_svfd_test_cmp_l3_END (9)
#define SOC_PMCTRL_VS_SVFD_CTRL1_L3_lpm_little_ctrl_START (11)
#define SOC_PMCTRL_VS_SVFD_CTRL1_L3_lpm_little_ctrl_END (15)
#define SOC_PMCTRL_VS_SVFD_CTRL1_L3_lpm_middle_ctrl_START (16)
#define SOC_PMCTRL_VS_SVFD_CTRL1_L3_lpm_middle_ctrl_END (20)
#define SOC_PMCTRL_VS_SVFD_CTRL1_L3_lpm_big_ctrl_START (21)
#define SOC_PMCTRL_VS_SVFD_CTRL1_L3_lpm_big_ctrl_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_enalbe_mode_l3 : 2;
        unsigned int svfs_cpm_threshold_r_l3 : 6;
        unsigned int svfs_cpm_threshold_f_l3 : 6;
        unsigned int reserved : 2;
        unsigned int vs_svfd_ctrl2_l3_msk : 16;
    } reg;
} SOC_PMCTRL_VS_SVFD_CTRL2_L3_UNION;
#endif
#define SOC_PMCTRL_VS_SVFD_CTRL2_L3_svfd_enalbe_mode_l3_START (0)
#define SOC_PMCTRL_VS_SVFD_CTRL2_L3_svfd_enalbe_mode_l3_END (1)
#define SOC_PMCTRL_VS_SVFD_CTRL2_L3_svfs_cpm_threshold_r_l3_START (2)
#define SOC_PMCTRL_VS_SVFD_CTRL2_L3_svfs_cpm_threshold_r_l3_END (7)
#define SOC_PMCTRL_VS_SVFD_CTRL2_L3_svfs_cpm_threshold_f_l3_START (8)
#define SOC_PMCTRL_VS_SVFD_CTRL2_L3_svfs_cpm_threshold_f_l3_END (13)
#define SOC_PMCTRL_VS_SVFD_CTRL2_L3_vs_svfd_ctrl2_l3_msk_START (16)
#define SOC_PMCTRL_VS_SVFD_CTRL2_L3_vs_svfd_ctrl2_l3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_cpm_data_f_l3 : 8;
        unsigned int svfd_cpm_data_r_l3 : 8;
        unsigned int svfd_dll_lock_l3 : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_PMCTRL_VS_CPM_DATA_STAT_L3_UNION;
#endif
#define SOC_PMCTRL_VS_CPM_DATA_STAT_L3_svfd_cpm_data_f_l3_START (0)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_L3_svfd_cpm_data_f_l3_END (7)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_L3_svfd_cpm_data_r_l3_START (8)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_L3_svfd_cpm_data_r_l3_END (15)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_L3_svfd_dll_lock_l3_START (16)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_L3_svfd_dll_lock_l3_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_test_out_ffs_l3 : 4;
        unsigned int svfd_test_out_cpm_l3 : 4;
        unsigned int svfd_glitch_result_l3 : 1;
        unsigned int idle_low_freq_en_l3 : 1;
        unsigned int vbat_drop_protect_ind_l3 : 1;
        unsigned int lpm_little_data_valid_out : 1;
        unsigned int lpm_little_div_status : 1;
        unsigned int lpm_little_data_out : 5;
        unsigned int lpm_middle_data_valid_out : 1;
        unsigned int lpm_middle_div_status : 1;
        unsigned int lpm_middle_data_out : 5;
        unsigned int lpm_big_data_valid_out : 1;
        unsigned int lpm_big_div_status : 1;
        unsigned int lpm_big_data_out : 5;
    } reg;
} SOC_PMCTRL_VS_TEST_STAT_L3_UNION;
#endif
#define SOC_PMCTRL_VS_TEST_STAT_L3_svfd_test_out_ffs_l3_START (0)
#define SOC_PMCTRL_VS_TEST_STAT_L3_svfd_test_out_ffs_l3_END (3)
#define SOC_PMCTRL_VS_TEST_STAT_L3_svfd_test_out_cpm_l3_START (4)
#define SOC_PMCTRL_VS_TEST_STAT_L3_svfd_test_out_cpm_l3_END (7)
#define SOC_PMCTRL_VS_TEST_STAT_L3_svfd_glitch_result_l3_START (8)
#define SOC_PMCTRL_VS_TEST_STAT_L3_svfd_glitch_result_l3_END (8)
#define SOC_PMCTRL_VS_TEST_STAT_L3_idle_low_freq_en_l3_START (9)
#define SOC_PMCTRL_VS_TEST_STAT_L3_idle_low_freq_en_l3_END (9)
#define SOC_PMCTRL_VS_TEST_STAT_L3_vbat_drop_protect_ind_l3_START (10)
#define SOC_PMCTRL_VS_TEST_STAT_L3_vbat_drop_protect_ind_l3_END (10)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_little_data_valid_out_START (11)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_little_data_valid_out_END (11)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_little_div_status_START (12)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_little_div_status_END (12)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_little_data_out_START (13)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_little_data_out_END (17)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_middle_data_valid_out_START (18)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_middle_data_valid_out_END (18)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_middle_div_status_START (19)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_middle_div_status_END (19)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_middle_data_out_START (20)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_middle_data_out_END (24)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_big_data_valid_out_START (25)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_big_data_valid_out_END (25)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_big_div_status_START (26)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_big_div_status_END (26)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_big_data_out_START (27)
#define SOC_PMCTRL_VS_TEST_STAT_L3_lpm_big_data_out_END (31)
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
        unsigned int svfd_cpm_data_f_gpu : 8;
        unsigned int svfd_cpm_data_r_gpu : 8;
        unsigned int svfd_dll_lock_gpu : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_PMCTRL_VS_CPM_DATA_STAT_GPU_UNION;
#endif
#define SOC_PMCTRL_VS_CPM_DATA_STAT_GPU_svfd_cpm_data_f_gpu_START (0)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_GPU_svfd_cpm_data_f_gpu_END (7)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_GPU_svfd_cpm_data_r_gpu_START (8)
#define SOC_PMCTRL_VS_CPM_DATA_STAT_GPU_svfd_cpm_data_r_gpu_END (15)
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
        unsigned int cluster0_gdvfs_curr_stat : 5;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 5;
        unsigned int reserved_2 : 3;
        unsigned int g3d_gdvfs_curr_stat : 5;
        unsigned int l3_gdvfs_curr_stat : 5;
        unsigned int reserved_3 : 6;
    } reg;
} SOC_PMCTRL_PMCGDVFSSTATUS_UNION;
#endif
#define SOC_PMCTRL_PMCGDVFSSTATUS_cluster0_gdvfs_curr_stat_START (0)
#define SOC_PMCTRL_PMCGDVFSSTATUS_cluster0_gdvfs_curr_stat_END (4)
#define SOC_PMCTRL_PMCGDVFSSTATUS_g3d_gdvfs_curr_stat_START (16)
#define SOC_PMCTRL_PMCGDVFSSTATUS_g3d_gdvfs_curr_stat_END (20)
#define SOC_PMCTRL_PMCGDVFSSTATUS_l3_gdvfs_curr_stat_START (21)
#define SOC_PMCTRL_PMCGDVFSSTATUS_l3_gdvfs_curr_stat_END (25)
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
        unsigned int apll0_ssc_reset : 1;
        unsigned int apll0_ssc_disable : 1;
        unsigned int apll0_ssc_downspread : 1;
        unsigned int apll0_ssc_spread : 3;
        unsigned int apll0_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_PMCTRL_APLL0SSCCTRL_UNION;
#endif
#define SOC_PMCTRL_APLL0SSCCTRL_apll0_ssc_reset_START (0)
#define SOC_PMCTRL_APLL0SSCCTRL_apll0_ssc_reset_END (0)
#define SOC_PMCTRL_APLL0SSCCTRL_apll0_ssc_disable_START (1)
#define SOC_PMCTRL_APLL0SSCCTRL_apll0_ssc_disable_END (1)
#define SOC_PMCTRL_APLL0SSCCTRL_apll0_ssc_downspread_START (2)
#define SOC_PMCTRL_APLL0SSCCTRL_apll0_ssc_downspread_END (2)
#define SOC_PMCTRL_APLL0SSCCTRL_apll0_ssc_spread_START (3)
#define SOC_PMCTRL_APLL0SSCCTRL_apll0_ssc_spread_END (5)
#define SOC_PMCTRL_APLL0SSCCTRL_apll0_ssc_divval_START (6)
#define SOC_PMCTRL_APLL0SSCCTRL_apll0_ssc_divval_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll1_ssc_reset : 1;
        unsigned int apll1_ssc_disable : 1;
        unsigned int apll1_ssc_downspread : 1;
        unsigned int apll1_ssc_spread : 3;
        unsigned int apll1_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_PMCTRL_APLL1SSCCTRL_UNION;
#endif
#define SOC_PMCTRL_APLL1SSCCTRL_apll1_ssc_reset_START (0)
#define SOC_PMCTRL_APLL1SSCCTRL_apll1_ssc_reset_END (0)
#define SOC_PMCTRL_APLL1SSCCTRL_apll1_ssc_disable_START (1)
#define SOC_PMCTRL_APLL1SSCCTRL_apll1_ssc_disable_END (1)
#define SOC_PMCTRL_APLL1SSCCTRL_apll1_ssc_downspread_START (2)
#define SOC_PMCTRL_APLL1SSCCTRL_apll1_ssc_downspread_END (2)
#define SOC_PMCTRL_APLL1SSCCTRL_apll1_ssc_spread_START (3)
#define SOC_PMCTRL_APLL1SSCCTRL_apll1_ssc_spread_END (5)
#define SOC_PMCTRL_APLL1SSCCTRL_apll1_ssc_divval_START (6)
#define SOC_PMCTRL_APLL1SSCCTRL_apll1_ssc_divval_END (9)
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
        unsigned int apll3_ssc_reset : 1;
        unsigned int apll3_ssc_disable : 1;
        unsigned int apll3_ssc_downspread : 1;
        unsigned int apll3_ssc_spread : 3;
        unsigned int apll3_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_PMCTRL_APLL3SSCCTRL_UNION;
#endif
#define SOC_PMCTRL_APLL3SSCCTRL_apll3_ssc_reset_START (0)
#define SOC_PMCTRL_APLL3SSCCTRL_apll3_ssc_reset_END (0)
#define SOC_PMCTRL_APLL3SSCCTRL_apll3_ssc_disable_START (1)
#define SOC_PMCTRL_APLL3SSCCTRL_apll3_ssc_disable_END (1)
#define SOC_PMCTRL_APLL3SSCCTRL_apll3_ssc_downspread_START (2)
#define SOC_PMCTRL_APLL3SSCCTRL_apll3_ssc_downspread_END (2)
#define SOC_PMCTRL_APLL3SSCCTRL_apll3_ssc_spread_START (3)
#define SOC_PMCTRL_APLL3SSCCTRL_apll3_ssc_spread_END (5)
#define SOC_PMCTRL_APLL3SSCCTRL_apll3_ssc_divval_START (6)
#define SOC_PMCTRL_APLL3SSCCTRL_apll3_ssc_divval_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll4_ssc_reset : 1;
        unsigned int apll4_ssc_disable : 1;
        unsigned int apll4_ssc_downspread : 1;
        unsigned int apll4_ssc_spread : 3;
        unsigned int apll4_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_PMCTRL_APLL4SSCCTRL_UNION;
#endif
#define SOC_PMCTRL_APLL4SSCCTRL_apll4_ssc_reset_START (0)
#define SOC_PMCTRL_APLL4SSCCTRL_apll4_ssc_reset_END (0)
#define SOC_PMCTRL_APLL4SSCCTRL_apll4_ssc_disable_START (1)
#define SOC_PMCTRL_APLL4SSCCTRL_apll4_ssc_disable_END (1)
#define SOC_PMCTRL_APLL4SSCCTRL_apll4_ssc_downspread_START (2)
#define SOC_PMCTRL_APLL4SSCCTRL_apll4_ssc_downspread_END (2)
#define SOC_PMCTRL_APLL4SSCCTRL_apll4_ssc_spread_START (3)
#define SOC_PMCTRL_APLL4SSCCTRL_apll4_ssc_spread_END (5)
#define SOC_PMCTRL_APLL4SSCCTRL_apll4_ssc_divval_START (6)
#define SOC_PMCTRL_APLL4SSCCTRL_apll4_ssc_divval_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll5_ssc_reset : 1;
        unsigned int apll5_ssc_disable : 1;
        unsigned int apll5_ssc_downspread : 1;
        unsigned int apll5_ssc_spread : 3;
        unsigned int apll5_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_PMCTRL_APLL5SSCCTRL_UNION;
#endif
#define SOC_PMCTRL_APLL5SSCCTRL_apll5_ssc_reset_START (0)
#define SOC_PMCTRL_APLL5SSCCTRL_apll5_ssc_reset_END (0)
#define SOC_PMCTRL_APLL5SSCCTRL_apll5_ssc_disable_START (1)
#define SOC_PMCTRL_APLL5SSCCTRL_apll5_ssc_disable_END (1)
#define SOC_PMCTRL_APLL5SSCCTRL_apll5_ssc_downspread_START (2)
#define SOC_PMCTRL_APLL5SSCCTRL_apll5_ssc_downspread_END (2)
#define SOC_PMCTRL_APLL5SSCCTRL_apll5_ssc_spread_START (3)
#define SOC_PMCTRL_APLL5SSCCTRL_apll5_ssc_spread_END (5)
#define SOC_PMCTRL_APLL5SSCCTRL_apll5_ssc_divval_START (6)
#define SOC_PMCTRL_APLL5SSCCTRL_apll5_ssc_divval_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll6_ssc_reset : 1;
        unsigned int apll6_ssc_disable : 1;
        unsigned int apll6_ssc_downspread : 1;
        unsigned int apll6_ssc_spread : 3;
        unsigned int apll6_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_PMCTRL_APLL6SSCCTRL_UNION;
#endif
#define SOC_PMCTRL_APLL6SSCCTRL_apll6_ssc_reset_START (0)
#define SOC_PMCTRL_APLL6SSCCTRL_apll6_ssc_reset_END (0)
#define SOC_PMCTRL_APLL6SSCCTRL_apll6_ssc_disable_START (1)
#define SOC_PMCTRL_APLL6SSCCTRL_apll6_ssc_disable_END (1)
#define SOC_PMCTRL_APLL6SSCCTRL_apll6_ssc_downspread_START (2)
#define SOC_PMCTRL_APLL6SSCCTRL_apll6_ssc_downspread_END (2)
#define SOC_PMCTRL_APLL6SSCCTRL_apll6_ssc_spread_START (3)
#define SOC_PMCTRL_APLL6SSCCTRL_apll6_ssc_spread_END (5)
#define SOC_PMCTRL_APLL6SSCCTRL_apll6_ssc_divval_START (6)
#define SOC_PMCTRL_APLL6SSCCTRL_apll6_ssc_divval_END (9)
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
        unsigned int l3_vol_mod : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_L3_VOLMOD_UNION;
#endif
#define SOC_PMCTRL_L3_VOLMOD_l3_vol_mod_START (0)
#define SOC_PMCTRL_L3_VOLMOD_l3_vol_mod_END (0)
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
        unsigned int l3_vol_up_step_time : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_PMCTRL_L3_VOLUPSTEPTIME_UNION;
#endif
#define SOC_PMCTRL_L3_VOLUPSTEPTIME_l3_vol_up_step_time_START (0)
#define SOC_PMCTRL_L3_VOLUPSTEPTIME_l3_vol_up_step_time_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_pmu_up_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_L3_PMU_UP_HOLDTIME_UNION;
#endif
#define SOC_PMCTRL_L3_PMU_UP_HOLDTIME_l3_pmu_up_vol_hold_time_START (0)
#define SOC_PMCTRL_L3_PMU_UP_HOLDTIME_l3_pmu_up_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_pmu_dn_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_L3_PMU_DN_HOLDTIME_UNION;
#endif
#define SOC_PMCTRL_L3_PMU_DN_HOLDTIME_l3_pmu_dn_vol_hold_time_START (0)
#define SOC_PMCTRL_L3_PMU_DN_HOLDTIME_l3_pmu_dn_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_pmu_sel : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_L3_PMUSEL_UNION;
#endif
#define SOC_PMCTRL_L3_PMUSEL_l3_pmu_sel_START (0)
#define SOC_PMCTRL_L3_PMUSEL_l3_pmu_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int l3_gdvfs_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_L3_GDVFS_EN_UNION;
#endif
#define SOC_PMCTRL_L3_GDVFS_EN_l3_gdvfs_en_START (0)
#define SOC_PMCTRL_L3_GDVFS_EN_l3_gdvfs_en_END (0)
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
        unsigned int l3_fbdiv_step : 2;
        unsigned int reserved_0 : 2;
        unsigned int l3_apll_cfg_time : 12;
        unsigned int l3_clkdiv_time : 7;
        unsigned int reserved_1 : 9;
    } reg;
} SOC_PMCTRL_L3_GDVFS_PROFILE1_UNION;
#endif
#define SOC_PMCTRL_L3_GDVFS_PROFILE1_l3_fbdiv_step_START (0)
#define SOC_PMCTRL_L3_GDVFS_PROFILE1_l3_fbdiv_step_END (1)
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
        unsigned int l3_vol_chg_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int l3_vol_idx_stat : 8;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_L3_VOLIDX_UNION;
#endif
#define SOC_PMCTRL_L3_VOLIDX_l3_vol_idx_cfg_START (0)
#define SOC_PMCTRL_L3_VOLIDX_l3_vol_idx_cfg_END (7)
#define SOC_PMCTRL_L3_VOLIDX_l3_vol_chg_sftreq_START (8)
#define SOC_PMCTRL_L3_VOLIDX_l3_vol_chg_sftreq_END (8)
#define SOC_PMCTRL_L3_VOLIDX_l3_vol_idx_stat_START (12)
#define SOC_PMCTRL_L3_VOLIDX_l3_vol_idx_stat_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int little_l3_unified_power_mode : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_LITTLE_POWERSUPPLYMODE_UNION;
#endif
#define SOC_PMCTRL_LITTLE_POWERSUPPLYMODE_little_l3_unified_power_mode_START (0)
#define SOC_PMCTRL_LITTLE_POWERSUPPLYMODE_little_l3_unified_power_mode_END (0)
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
        unsigned int l3_initial_vol_idx : 8;
        unsigned int l3_initialvol_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 20;
    } reg;
} SOC_PMCTRL_L3_INITIALVOL_UNION;
#endif
#define SOC_PMCTRL_L3_INITIALVOL_l3_initial_vol_idx_START (0)
#define SOC_PMCTRL_L3_INITIALVOL_l3_initial_vol_idx_END (7)
#define SOC_PMCTRL_L3_INITIALVOL_l3_initialvol_sftreq_START (8)
#define SOC_PMCTRL_L3_INITIALVOL_l3_initialvol_sftreq_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int little_unified_vol_idx_stat : 8;
        unsigned int l3_unified_vol_idx_stat : 8;
        unsigned int little_l3_unified_pmu_vol_idx_stat : 8;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_LILLTEL3_UNIFIED_VOL_STAT_UNION;
#endif
#define SOC_PMCTRL_LILLTEL3_UNIFIED_VOL_STAT_little_unified_vol_idx_stat_START (0)
#define SOC_PMCTRL_LILLTEL3_UNIFIED_VOL_STAT_little_unified_vol_idx_stat_END (7)
#define SOC_PMCTRL_LILLTEL3_UNIFIED_VOL_STAT_l3_unified_vol_idx_stat_START (8)
#define SOC_PMCTRL_LILLTEL3_UNIFIED_VOL_STAT_l3_unified_vol_idx_stat_END (15)
#define SOC_PMCTRL_LILLTEL3_UNIFIED_VOL_STAT_little_l3_unified_pmu_vol_idx_stat_START (16)
#define SOC_PMCTRL_LILLTEL3_UNIFIED_VOL_STAT_little_l3_unified_pmu_vol_idx_stat_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dynamic_retention_bypass : 4;
        unsigned int static_retention_bypass : 4;
        unsigned int ocldo_toggle_bypass : 1;
        unsigned int artemis_ocldo_sel : 3;
        unsigned int ocldo_ana_iomux : 4;
        unsigned int ocldo_bypass_ctrl_msk : 16;
    } reg;
} SOC_PMCTRL_OCLDO_BYPASS_CTRL_UNION;
#endif
#define SOC_PMCTRL_OCLDO_BYPASS_CTRL_dynamic_retention_bypass_START (0)
#define SOC_PMCTRL_OCLDO_BYPASS_CTRL_dynamic_retention_bypass_END (3)
#define SOC_PMCTRL_OCLDO_BYPASS_CTRL_static_retention_bypass_START (4)
#define SOC_PMCTRL_OCLDO_BYPASS_CTRL_static_retention_bypass_END (7)
#define SOC_PMCTRL_OCLDO_BYPASS_CTRL_ocldo_toggle_bypass_START (8)
#define SOC_PMCTRL_OCLDO_BYPASS_CTRL_ocldo_toggle_bypass_END (8)
#define SOC_PMCTRL_OCLDO_BYPASS_CTRL_artemis_ocldo_sel_START (9)
#define SOC_PMCTRL_OCLDO_BYPASS_CTRL_artemis_ocldo_sel_END (11)
#define SOC_PMCTRL_OCLDO_BYPASS_CTRL_ocldo_ana_iomux_START (12)
#define SOC_PMCTRL_OCLDO_BYPASS_CTRL_ocldo_ana_iomux_END (15)
#define SOC_PMCTRL_OCLDO_BYPASS_CTRL_ocldo_bypass_ctrl_msk_START (16)
#define SOC_PMCTRL_OCLDO_BYPASS_CTRL_ocldo_bypass_ctrl_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core0_ocldo_req_syn : 1;
        unsigned int core1_ocldo_req_syn : 1;
        unsigned int core2_ocldo_req_syn : 1;
        unsigned int core3_ocldo_req_syn : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_PMCTRL_OCLDO_RETENTION_REQ_UNION;
#endif
#define SOC_PMCTRL_OCLDO_RETENTION_REQ_core0_ocldo_req_syn_START (0)
#define SOC_PMCTRL_OCLDO_RETENTION_REQ_core0_ocldo_req_syn_END (0)
#define SOC_PMCTRL_OCLDO_RETENTION_REQ_core1_ocldo_req_syn_START (1)
#define SOC_PMCTRL_OCLDO_RETENTION_REQ_core1_ocldo_req_syn_END (1)
#define SOC_PMCTRL_OCLDO_RETENTION_REQ_core2_ocldo_req_syn_START (2)
#define SOC_PMCTRL_OCLDO_RETENTION_REQ_core2_ocldo_req_syn_END (2)
#define SOC_PMCTRL_OCLDO_RETENTION_REQ_core3_ocldo_req_syn_START (3)
#define SOC_PMCTRL_OCLDO_RETENTION_REQ_core3_ocldo_req_syn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 20;
        unsigned int reserved_1: 12;
    } reg;
} SOC_PMCTRL_OCLDO_DEBUG_STAT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core0_pswfsm_curr_stat : 3;
        unsigned int reserved_0 : 1;
        unsigned int core0_rhfsm_curr_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int core0_ocldo_retention_stat : 1;
        unsigned int reserved_2 : 3;
        unsigned int core0_ps_stat : 1;
        unsigned int reserved_3 : 3;
        unsigned int core0_ocldo_fsm_idle : 1;
        unsigned int reserved_4 : 3;
        unsigned int reserved_5 : 12;
    } reg;
} SOC_PMCTRL_CORE0_DVFS_INNER_STAT_UNION;
#endif
#define SOC_PMCTRL_CORE0_DVFS_INNER_STAT_core0_pswfsm_curr_stat_START (0)
#define SOC_PMCTRL_CORE0_DVFS_INNER_STAT_core0_pswfsm_curr_stat_END (2)
#define SOC_PMCTRL_CORE0_DVFS_INNER_STAT_core0_rhfsm_curr_stat_START (4)
#define SOC_PMCTRL_CORE0_DVFS_INNER_STAT_core0_rhfsm_curr_stat_END (6)
#define SOC_PMCTRL_CORE0_DVFS_INNER_STAT_core0_ocldo_retention_stat_START (8)
#define SOC_PMCTRL_CORE0_DVFS_INNER_STAT_core0_ocldo_retention_stat_END (8)
#define SOC_PMCTRL_CORE0_DVFS_INNER_STAT_core0_ps_stat_START (12)
#define SOC_PMCTRL_CORE0_DVFS_INNER_STAT_core0_ps_stat_END (12)
#define SOC_PMCTRL_CORE0_DVFS_INNER_STAT_core0_ocldo_fsm_idle_START (16)
#define SOC_PMCTRL_CORE0_DVFS_INNER_STAT_core0_ocldo_fsm_idle_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core1_pswfsm_curr_stat : 3;
        unsigned int reserved_0 : 1;
        unsigned int core1_rhfsm_curr_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int core1_ocldo_retention_stat : 1;
        unsigned int reserved_2 : 3;
        unsigned int core1_ps_stat : 1;
        unsigned int reserved_3 : 3;
        unsigned int core1_ocldo_fsm_idle : 1;
        unsigned int reserved_4 : 3;
        unsigned int reserved_5 : 12;
    } reg;
} SOC_PMCTRL_CORE1_DVFS_INNER_STAT_UNION;
#endif
#define SOC_PMCTRL_CORE1_DVFS_INNER_STAT_core1_pswfsm_curr_stat_START (0)
#define SOC_PMCTRL_CORE1_DVFS_INNER_STAT_core1_pswfsm_curr_stat_END (2)
#define SOC_PMCTRL_CORE1_DVFS_INNER_STAT_core1_rhfsm_curr_stat_START (4)
#define SOC_PMCTRL_CORE1_DVFS_INNER_STAT_core1_rhfsm_curr_stat_END (6)
#define SOC_PMCTRL_CORE1_DVFS_INNER_STAT_core1_ocldo_retention_stat_START (8)
#define SOC_PMCTRL_CORE1_DVFS_INNER_STAT_core1_ocldo_retention_stat_END (8)
#define SOC_PMCTRL_CORE1_DVFS_INNER_STAT_core1_ps_stat_START (12)
#define SOC_PMCTRL_CORE1_DVFS_INNER_STAT_core1_ps_stat_END (12)
#define SOC_PMCTRL_CORE1_DVFS_INNER_STAT_core1_ocldo_fsm_idle_START (16)
#define SOC_PMCTRL_CORE1_DVFS_INNER_STAT_core1_ocldo_fsm_idle_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core2_pswfsm_curr_stat : 3;
        unsigned int reserved_0 : 1;
        unsigned int core2_rhfsm_curr_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int core2_ocldo_retention_stat : 1;
        unsigned int reserved_2 : 3;
        unsigned int core2_ps_stat : 1;
        unsigned int reserved_3 : 3;
        unsigned int core2_ocldo_fsm_idle : 1;
        unsigned int reserved_4 : 3;
        unsigned int reserved_5 : 12;
    } reg;
} SOC_PMCTRL_CORE2_DVFS_INNER_STAT_UNION;
#endif
#define SOC_PMCTRL_CORE2_DVFS_INNER_STAT_core2_pswfsm_curr_stat_START (0)
#define SOC_PMCTRL_CORE2_DVFS_INNER_STAT_core2_pswfsm_curr_stat_END (2)
#define SOC_PMCTRL_CORE2_DVFS_INNER_STAT_core2_rhfsm_curr_stat_START (4)
#define SOC_PMCTRL_CORE2_DVFS_INNER_STAT_core2_rhfsm_curr_stat_END (6)
#define SOC_PMCTRL_CORE2_DVFS_INNER_STAT_core2_ocldo_retention_stat_START (8)
#define SOC_PMCTRL_CORE2_DVFS_INNER_STAT_core2_ocldo_retention_stat_END (8)
#define SOC_PMCTRL_CORE2_DVFS_INNER_STAT_core2_ps_stat_START (12)
#define SOC_PMCTRL_CORE2_DVFS_INNER_STAT_core2_ps_stat_END (12)
#define SOC_PMCTRL_CORE2_DVFS_INNER_STAT_core2_ocldo_fsm_idle_START (16)
#define SOC_PMCTRL_CORE2_DVFS_INNER_STAT_core2_ocldo_fsm_idle_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core3_pswfsm_curr_stat : 3;
        unsigned int reserved_0 : 1;
        unsigned int core3_rhfsm_curr_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int core3_ocldo_retention_stat : 1;
        unsigned int reserved_2 : 3;
        unsigned int core3_ps_stat : 1;
        unsigned int reserved_3 : 3;
        unsigned int core3_ocldo_fsm_idle : 1;
        unsigned int reserved_4 : 3;
        unsigned int reserved_5 : 12;
    } reg;
} SOC_PMCTRL_CORE3_DVFS_INNER_STAT_UNION;
#endif
#define SOC_PMCTRL_CORE3_DVFS_INNER_STAT_core3_pswfsm_curr_stat_START (0)
#define SOC_PMCTRL_CORE3_DVFS_INNER_STAT_core3_pswfsm_curr_stat_END (2)
#define SOC_PMCTRL_CORE3_DVFS_INNER_STAT_core3_rhfsm_curr_stat_START (4)
#define SOC_PMCTRL_CORE3_DVFS_INNER_STAT_core3_rhfsm_curr_stat_END (6)
#define SOC_PMCTRL_CORE3_DVFS_INNER_STAT_core3_ocldo_retention_stat_START (8)
#define SOC_PMCTRL_CORE3_DVFS_INNER_STAT_core3_ocldo_retention_stat_END (8)
#define SOC_PMCTRL_CORE3_DVFS_INNER_STAT_core3_ps_stat_START (12)
#define SOC_PMCTRL_CORE3_DVFS_INNER_STAT_core3_ps_stat_END (12)
#define SOC_PMCTRL_CORE3_DVFS_INNER_STAT_core3_ocldo_fsm_idle_START (16)
#define SOC_PMCTRL_CORE3_DVFS_INNER_STAT_core3_ocldo_fsm_idle_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocldo0_dvfs_vol_idx : 6;
        unsigned int reserved_0 : 2;
        unsigned int ocldo0_dvfs_pcell_en : 12;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_OCLDO0_DVFSVOLIDX_UNION;
#endif
#define SOC_PMCTRL_OCLDO0_DVFSVOLIDX_ocldo0_dvfs_vol_idx_START (0)
#define SOC_PMCTRL_OCLDO0_DVFSVOLIDX_ocldo0_dvfs_vol_idx_END (5)
#define SOC_PMCTRL_OCLDO0_DVFSVOLIDX_ocldo0_dvfs_pcell_en_START (8)
#define SOC_PMCTRL_OCLDO0_DVFSVOLIDX_ocldo0_dvfs_pcell_en_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocldo1_dvfs_vol_idx : 6;
        unsigned int reserved_0 : 2;
        unsigned int ocldo1_dvfs_pcell_en : 12;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_OCLDO1_DVFSVOLIDX_UNION;
#endif
#define SOC_PMCTRL_OCLDO1_DVFSVOLIDX_ocldo1_dvfs_vol_idx_START (0)
#define SOC_PMCTRL_OCLDO1_DVFSVOLIDX_ocldo1_dvfs_vol_idx_END (5)
#define SOC_PMCTRL_OCLDO1_DVFSVOLIDX_ocldo1_dvfs_pcell_en_START (8)
#define SOC_PMCTRL_OCLDO1_DVFSVOLIDX_ocldo1_dvfs_pcell_en_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocldo2_dvfs_vol_idx : 6;
        unsigned int reserved_0 : 2;
        unsigned int ocldo2_dvfs_pcell_en : 12;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_OCLDO2_DVFSVOLIDX_UNION;
#endif
#define SOC_PMCTRL_OCLDO2_DVFSVOLIDX_ocldo2_dvfs_vol_idx_START (0)
#define SOC_PMCTRL_OCLDO2_DVFSVOLIDX_ocldo2_dvfs_vol_idx_END (5)
#define SOC_PMCTRL_OCLDO2_DVFSVOLIDX_ocldo2_dvfs_pcell_en_START (8)
#define SOC_PMCTRL_OCLDO2_DVFSVOLIDX_ocldo2_dvfs_pcell_en_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocldo3_dvfs_vol_idx : 6;
        unsigned int reserved_0 : 2;
        unsigned int ocldo3_dvfs_pcell_en : 12;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_OCLDO3_DVFSVOLIDX_UNION;
#endif
#define SOC_PMCTRL_OCLDO3_DVFSVOLIDX_ocldo3_dvfs_vol_idx_START (0)
#define SOC_PMCTRL_OCLDO3_DVFSVOLIDX_ocldo3_dvfs_vol_idx_END (5)
#define SOC_PMCTRL_OCLDO3_DVFSVOLIDX_ocldo3_dvfs_pcell_en_START (8)
#define SOC_PMCTRL_OCLDO3_DVFSVOLIDX_ocldo3_dvfs_pcell_en_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core0_ocldo_iso_bypass : 1;
        unsigned int core1_ocldo_iso_bypass : 1;
        unsigned int core2_ocldo_iso_bypass : 1;
        unsigned int core3_ocldo_iso_bypass : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_PMCTRL_OCLDO_RH_CTRL0_UNION;
#endif
#define SOC_PMCTRL_OCLDO_RH_CTRL0_core0_ocldo_iso_bypass_START (0)
#define SOC_PMCTRL_OCLDO_RH_CTRL0_core0_ocldo_iso_bypass_END (0)
#define SOC_PMCTRL_OCLDO_RH_CTRL0_core1_ocldo_iso_bypass_START (1)
#define SOC_PMCTRL_OCLDO_RH_CTRL0_core1_ocldo_iso_bypass_END (1)
#define SOC_PMCTRL_OCLDO_RH_CTRL0_core2_ocldo_iso_bypass_START (2)
#define SOC_PMCTRL_OCLDO_RH_CTRL0_core2_ocldo_iso_bypass_END (2)
#define SOC_PMCTRL_OCLDO_RH_CTRL0_core3_ocldo_iso_bypass_START (3)
#define SOC_PMCTRL_OCLDO_RH_CTRL0_core3_ocldo_iso_bypass_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core0_aldo_enter_sft_req : 1;
        unsigned int core0_aldo_exit_sft_req : 1;
        unsigned int core1_aldo_enter_sft_req : 1;
        unsigned int core1_aldo_exit_sft_req : 1;
        unsigned int core2_aldo_enter_sft_req : 1;
        unsigned int core2_aldo_exit_sft_req : 1;
        unsigned int core3_aldo_enter_sft_req : 1;
        unsigned int core3_aldo_exit_sft_req : 1;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_UNION;
#endif
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_core0_aldo_enter_sft_req_START (0)
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_core0_aldo_enter_sft_req_END (0)
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_core0_aldo_exit_sft_req_START (1)
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_core0_aldo_exit_sft_req_END (1)
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_core1_aldo_enter_sft_req_START (2)
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_core1_aldo_enter_sft_req_END (2)
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_core1_aldo_exit_sft_req_START (3)
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_core1_aldo_exit_sft_req_END (3)
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_core2_aldo_enter_sft_req_START (4)
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_core2_aldo_enter_sft_req_END (4)
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_core2_aldo_exit_sft_req_START (5)
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_core2_aldo_exit_sft_req_END (5)
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_core3_aldo_enter_sft_req_START (6)
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_core3_aldo_enter_sft_req_END (6)
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_core3_aldo_exit_sft_req_START (7)
#define SOC_PMCTRL_OCLDO_ACTIVE_CTRL0_core3_aldo_exit_sft_req_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core0_ocldo_retention_sft_req : 1;
        unsigned int core1_ocldo_retention_sft_req : 1;
        unsigned int core2_ocldo_retention_sft_req : 1;
        unsigned int core3_ocldo_retention_sft_req : 1;
        unsigned int core0_ocldo_retention_debug_mode : 1;
        unsigned int core1_ocldo_retention_debug_mode : 1;
        unsigned int core2_ocldo_retention_debug_mode : 1;
        unsigned int core3_ocldo_retention_debug_mode : 1;
        unsigned int reserved : 8;
        unsigned int ocldo_retention_debug_msk : 16;
    } reg;
} SOC_PMCTRL_OCLDO_RETENTION_DEBUG_UNION;
#endif
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_core0_ocldo_retention_sft_req_START (0)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_core0_ocldo_retention_sft_req_END (0)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_core1_ocldo_retention_sft_req_START (1)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_core1_ocldo_retention_sft_req_END (1)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_core2_ocldo_retention_sft_req_START (2)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_core2_ocldo_retention_sft_req_END (2)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_core3_ocldo_retention_sft_req_START (3)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_core3_ocldo_retention_sft_req_END (3)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_core0_ocldo_retention_debug_mode_START (4)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_core0_ocldo_retention_debug_mode_END (4)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_core1_ocldo_retention_debug_mode_START (5)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_core1_ocldo_retention_debug_mode_END (5)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_core2_ocldo_retention_debug_mode_START (6)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_core2_ocldo_retention_debug_mode_END (6)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_core3_ocldo_retention_debug_mode_START (7)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_core3_ocldo_retention_debug_mode_END (7)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_ocldo_retention_debug_msk_START (16)
#define SOC_PMCTRL_OCLDO_RETENTION_DEBUG_ocldo_retention_debug_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocldo0_sw_en_m : 4;
        unsigned int ocldo0_sw_vout_set : 6;
        unsigned int ocldo0_sw_m1_pcell_en : 3;
        unsigned int ocldo0_sw_m2_pcell_en : 3;
        unsigned int ocldo0_sw_m3_pcell_en : 3;
        unsigned int ocldo0_sw_m4_pcell_en : 3;
        unsigned int ocldo0_sw_pset_in : 4;
        unsigned int ocldo0_sw_vin_det : 4;
        unsigned int ocldo0_sw_en_v : 1;
        unsigned int ocldo0_sw_ctrl1_chg : 1;
    } reg;
} SOC_PMCTRL_OCLDO0_SW_CTRL1_UNION;
#endif
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_en_m_START (0)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_en_m_END (3)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_vout_set_START (4)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_vout_set_END (9)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_m1_pcell_en_START (10)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_m1_pcell_en_END (12)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_m2_pcell_en_START (13)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_m2_pcell_en_END (15)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_m3_pcell_en_START (16)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_m3_pcell_en_END (18)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_m4_pcell_en_START (19)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_m4_pcell_en_END (21)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_pset_in_START (22)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_pset_in_END (25)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_vin_det_START (26)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_vin_det_END (29)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_en_v_START (30)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_en_v_END (30)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_ctrl1_chg_START (31)
#define SOC_PMCTRL_OCLDO0_SW_CTRL1_ocldo0_sw_ctrl1_chg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocldo_vin_out : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_PMCTRL_OCLDO0_ANA_STAT_UNION;
#endif
#define SOC_PMCTRL_OCLDO0_ANA_STAT_ocldo_vin_out_START (0)
#define SOC_PMCTRL_OCLDO0_ANA_STAT_ocldo_vin_out_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int act_on_err : 1;
        unsigned int rtn_on_err : 1;
        unsigned int act_ramp_up_err : 1;
        unsigned int act_ramp_dn_err : 1;
        unsigned int act2rtn_err : 1;
        unsigned int rtn2act_err : 1;
        unsigned int off_err : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_PMCTRL_OCLDO0_ERR_STAT_UNION;
#endif
#define SOC_PMCTRL_OCLDO0_ERR_STAT_act_on_err_START (0)
#define SOC_PMCTRL_OCLDO0_ERR_STAT_act_on_err_END (0)
#define SOC_PMCTRL_OCLDO0_ERR_STAT_rtn_on_err_START (1)
#define SOC_PMCTRL_OCLDO0_ERR_STAT_rtn_on_err_END (1)
#define SOC_PMCTRL_OCLDO0_ERR_STAT_act_ramp_up_err_START (2)
#define SOC_PMCTRL_OCLDO0_ERR_STAT_act_ramp_up_err_END (2)
#define SOC_PMCTRL_OCLDO0_ERR_STAT_act_ramp_dn_err_START (3)
#define SOC_PMCTRL_OCLDO0_ERR_STAT_act_ramp_dn_err_END (3)
#define SOC_PMCTRL_OCLDO0_ERR_STAT_act2rtn_err_START (4)
#define SOC_PMCTRL_OCLDO0_ERR_STAT_act2rtn_err_END (4)
#define SOC_PMCTRL_OCLDO0_ERR_STAT_rtn2act_err_START (5)
#define SOC_PMCTRL_OCLDO0_ERR_STAT_rtn2act_err_END (5)
#define SOC_PMCTRL_OCLDO0_ERR_STAT_off_err_START (6)
#define SOC_PMCTRL_OCLDO0_ERR_STAT_off_err_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int active_en_m : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_PMCTRL_OCLDO0_ACTIVE_CFG_UNION;
#endif
#define SOC_PMCTRL_OCLDO0_ACTIVE_CFG_active_en_m_START (0)
#define SOC_PMCTRL_OCLDO0_ACTIVE_CFG_active_en_m_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int retention_en_m : 4;
        unsigned int retention_vid : 6;
        unsigned int retention_pcell_en : 12;
        unsigned int reserved : 10;
    } reg;
} SOC_PMCTRL_OCLDO0_RETENTION_CFG_UNION;
#endif
#define SOC_PMCTRL_OCLDO0_RETENTION_CFG_retention_en_m_START (0)
#define SOC_PMCTRL_OCLDO0_RETENTION_CFG_retention_en_m_END (3)
#define SOC_PMCTRL_OCLDO0_RETENTION_CFG_retention_vid_START (4)
#define SOC_PMCTRL_OCLDO0_RETENTION_CFG_retention_vid_END (9)
#define SOC_PMCTRL_OCLDO0_RETENTION_CFG_retention_pcell_en_START (10)
#define SOC_PMCTRL_OCLDO0_RETENTION_CFG_retention_pcell_en_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int act_on_offset_vid : 6;
        unsigned int reserved_0 : 2;
        unsigned int rtn_on_offset_vid : 6;
        unsigned int reserved_1 : 2;
        unsigned int rtn2act_offset_vid : 6;
        unsigned int reserved_2 : 10;
    } reg;
} SOC_PMCTRL_OCLDO0_OFFSET_VID_UNION;
#endif
#define SOC_PMCTRL_OCLDO0_OFFSET_VID_act_on_offset_vid_START (0)
#define SOC_PMCTRL_OCLDO0_OFFSET_VID_act_on_offset_vid_END (5)
#define SOC_PMCTRL_OCLDO0_OFFSET_VID_rtn_on_offset_vid_START (8)
#define SOC_PMCTRL_OCLDO0_OFFSET_VID_rtn_on_offset_vid_END (13)
#define SOC_PMCTRL_OCLDO0_OFFSET_VID_rtn2act_offset_vid_START (16)
#define SOC_PMCTRL_OCLDO0_OFFSET_VID_rtn2act_offset_vid_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int byp_hw_act_on_req : 1;
        unsigned int byp_hw_rtn_on_req : 1;
        unsigned int byp_hw_act_ramp_req : 1;
        unsigned int byp_hw_rtn2act_req : 1;
        unsigned int byp_hw_act2rtn_req : 1;
        unsigned int byp_hw_off_req : 1;
        unsigned int byp_pset_hw_ctrl : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_PMCTRL_OCLDO0_BYP_CTRL_UNION;
#endif
#define SOC_PMCTRL_OCLDO0_BYP_CTRL_byp_hw_act_on_req_START (0)
#define SOC_PMCTRL_OCLDO0_BYP_CTRL_byp_hw_act_on_req_END (0)
#define SOC_PMCTRL_OCLDO0_BYP_CTRL_byp_hw_rtn_on_req_START (1)
#define SOC_PMCTRL_OCLDO0_BYP_CTRL_byp_hw_rtn_on_req_END (1)
#define SOC_PMCTRL_OCLDO0_BYP_CTRL_byp_hw_act_ramp_req_START (2)
#define SOC_PMCTRL_OCLDO0_BYP_CTRL_byp_hw_act_ramp_req_END (2)
#define SOC_PMCTRL_OCLDO0_BYP_CTRL_byp_hw_rtn2act_req_START (3)
#define SOC_PMCTRL_OCLDO0_BYP_CTRL_byp_hw_rtn2act_req_END (3)
#define SOC_PMCTRL_OCLDO0_BYP_CTRL_byp_hw_act2rtn_req_START (4)
#define SOC_PMCTRL_OCLDO0_BYP_CTRL_byp_hw_act2rtn_req_END (4)
#define SOC_PMCTRL_OCLDO0_BYP_CTRL_byp_hw_off_req_START (5)
#define SOC_PMCTRL_OCLDO0_BYP_CTRL_byp_hw_off_req_END (5)
#define SOC_PMCTRL_OCLDO0_BYP_CTRL_byp_pset_hw_ctrl_START (6)
#define SOC_PMCTRL_OCLDO0_BYP_CTRL_byp_pset_hw_ctrl_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_act_on_req : 1;
        unsigned int sw_act_ramp_req : 1;
        unsigned int sw_off_req : 1;
        unsigned int fsm_rst : 1;
        unsigned int err_clr : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_PMCTRL_OCLDO0_SW_REQ_UNION;
#endif
#define SOC_PMCTRL_OCLDO0_SW_REQ_sw_act_on_req_START (0)
#define SOC_PMCTRL_OCLDO0_SW_REQ_sw_act_on_req_END (0)
#define SOC_PMCTRL_OCLDO0_SW_REQ_sw_act_ramp_req_START (1)
#define SOC_PMCTRL_OCLDO0_SW_REQ_sw_act_ramp_req_END (1)
#define SOC_PMCTRL_OCLDO0_SW_REQ_sw_off_req_START (2)
#define SOC_PMCTRL_OCLDO0_SW_REQ_sw_off_req_END (2)
#define SOC_PMCTRL_OCLDO0_SW_REQ_fsm_rst_START (3)
#define SOC_PMCTRL_OCLDO0_SW_REQ_fsm_rst_END (3)
#define SOC_PMCTRL_OCLDO0_SW_REQ_err_clr_START (4)
#define SOC_PMCTRL_OCLDO0_SW_REQ_err_clr_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fsm_stat : 4;
        unsigned int ramp_done : 1;
        unsigned int retention_mode : 1;
        unsigned int act_on_ramp : 1;
        unsigned int rtn_on_ramp : 1;
        unsigned int act_up_ramp : 1;
        unsigned int act_dn_ramp : 1;
        unsigned int act2rtn_ramp : 1;
        unsigned int rtn2act_ramp : 1;
        unsigned int off_ramp : 1;
        unsigned int reserved : 3;
        unsigned int ramp_cnt : 16;
    } reg;
} SOC_PMCTRL_OCLDO0_CTRL_DEBUG_UNION;
#endif
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_fsm_stat_START (0)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_fsm_stat_END (3)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_ramp_done_START (4)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_ramp_done_END (4)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_retention_mode_START (5)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_retention_mode_END (5)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_act_on_ramp_START (6)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_act_on_ramp_END (6)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_rtn_on_ramp_START (7)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_rtn_on_ramp_END (7)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_act_up_ramp_START (8)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_act_up_ramp_END (8)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_act_dn_ramp_START (9)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_act_dn_ramp_END (9)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_act2rtn_ramp_START (10)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_act2rtn_ramp_END (10)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_rtn2act_ramp_START (11)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_rtn2act_ramp_END (11)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_off_ramp_START (12)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_off_ramp_END (12)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_ramp_cnt_START (16)
#define SOC_PMCTRL_OCLDO0_CTRL_DEBUG_ramp_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int en_m_stat : 4;
        unsigned int vout_stat : 6;
        unsigned int m1_pcell_en_stat : 3;
        unsigned int m2_pcell_en_stat : 3;
        unsigned int m3_pcell_en_stat : 3;
        unsigned int m4_pcell_en_stat : 3;
        unsigned int pset_in_stat : 4;
        unsigned int vin_det_stat : 4;
        unsigned int en_v_stat : 1;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_OCLDO0_CTRL_STAT_UNION;
#endif
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_en_m_stat_START (0)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_en_m_stat_END (3)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_vout_stat_START (4)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_vout_stat_END (9)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_m1_pcell_en_stat_START (10)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_m1_pcell_en_stat_END (12)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_m2_pcell_en_stat_START (13)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_m2_pcell_en_stat_END (15)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_m3_pcell_en_stat_START (16)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_m3_pcell_en_stat_END (18)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_m4_pcell_en_stat_START (19)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_m4_pcell_en_stat_END (21)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_pset_in_stat_START (22)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_pset_in_stat_END (25)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_vin_det_stat_START (26)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_vin_det_stat_END (29)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_en_v_stat_START (30)
#define SOC_PMCTRL_OCLDO0_CTRL_STAT_en_v_stat_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocldo1_sw_en_m : 4;
        unsigned int ocldo1_sw_vout_set : 6;
        unsigned int ocldo1_sw_m1_pcell_en : 3;
        unsigned int ocldo1_sw_m2_pcell_en : 3;
        unsigned int ocldo1_sw_m3_pcell_en : 3;
        unsigned int ocldo1_sw_m4_pcell_en : 3;
        unsigned int ocldo1_sw_pset_in : 4;
        unsigned int ocldo1_sw_vin_det : 4;
        unsigned int ocldo1_sw_en_v : 1;
        unsigned int ocldo1_sw_ctrl1_chg : 1;
    } reg;
} SOC_PMCTRL_OCLDO1_SW_CTRL1_UNION;
#endif
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_en_m_START (0)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_en_m_END (3)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_vout_set_START (4)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_vout_set_END (9)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_m1_pcell_en_START (10)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_m1_pcell_en_END (12)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_m2_pcell_en_START (13)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_m2_pcell_en_END (15)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_m3_pcell_en_START (16)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_m3_pcell_en_END (18)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_m4_pcell_en_START (19)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_m4_pcell_en_END (21)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_pset_in_START (22)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_pset_in_END (25)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_vin_det_START (26)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_vin_det_END (29)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_en_v_START (30)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_en_v_END (30)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_ctrl1_chg_START (31)
#define SOC_PMCTRL_OCLDO1_SW_CTRL1_ocldo1_sw_ctrl1_chg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocldo_vin_out : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_PMCTRL_OCLDO1_ANA_STAT_UNION;
#endif
#define SOC_PMCTRL_OCLDO1_ANA_STAT_ocldo_vin_out_START (0)
#define SOC_PMCTRL_OCLDO1_ANA_STAT_ocldo_vin_out_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int act_on_err : 1;
        unsigned int rtn_on_err : 1;
        unsigned int act_ramp_up_err : 1;
        unsigned int act_ramp_dn_err : 1;
        unsigned int act2rtn_err : 1;
        unsigned int rtn2act_err : 1;
        unsigned int off_err : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_PMCTRL_OCLDO1_ERR_STAT_UNION;
#endif
#define SOC_PMCTRL_OCLDO1_ERR_STAT_act_on_err_START (0)
#define SOC_PMCTRL_OCLDO1_ERR_STAT_act_on_err_END (0)
#define SOC_PMCTRL_OCLDO1_ERR_STAT_rtn_on_err_START (1)
#define SOC_PMCTRL_OCLDO1_ERR_STAT_rtn_on_err_END (1)
#define SOC_PMCTRL_OCLDO1_ERR_STAT_act_ramp_up_err_START (2)
#define SOC_PMCTRL_OCLDO1_ERR_STAT_act_ramp_up_err_END (2)
#define SOC_PMCTRL_OCLDO1_ERR_STAT_act_ramp_dn_err_START (3)
#define SOC_PMCTRL_OCLDO1_ERR_STAT_act_ramp_dn_err_END (3)
#define SOC_PMCTRL_OCLDO1_ERR_STAT_act2rtn_err_START (4)
#define SOC_PMCTRL_OCLDO1_ERR_STAT_act2rtn_err_END (4)
#define SOC_PMCTRL_OCLDO1_ERR_STAT_rtn2act_err_START (5)
#define SOC_PMCTRL_OCLDO1_ERR_STAT_rtn2act_err_END (5)
#define SOC_PMCTRL_OCLDO1_ERR_STAT_off_err_START (6)
#define SOC_PMCTRL_OCLDO1_ERR_STAT_off_err_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int active_en_m : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_PMCTRL_OCLDO1_ACTIVE_CFG_UNION;
#endif
#define SOC_PMCTRL_OCLDO1_ACTIVE_CFG_active_en_m_START (0)
#define SOC_PMCTRL_OCLDO1_ACTIVE_CFG_active_en_m_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int retention_en_m : 4;
        unsigned int retention_vid : 6;
        unsigned int retention_pcell_en : 12;
        unsigned int reserved : 10;
    } reg;
} SOC_PMCTRL_OCLDO1_RETENTION_CFG_UNION;
#endif
#define SOC_PMCTRL_OCLDO1_RETENTION_CFG_retention_en_m_START (0)
#define SOC_PMCTRL_OCLDO1_RETENTION_CFG_retention_en_m_END (3)
#define SOC_PMCTRL_OCLDO1_RETENTION_CFG_retention_vid_START (4)
#define SOC_PMCTRL_OCLDO1_RETENTION_CFG_retention_vid_END (9)
#define SOC_PMCTRL_OCLDO1_RETENTION_CFG_retention_pcell_en_START (10)
#define SOC_PMCTRL_OCLDO1_RETENTION_CFG_retention_pcell_en_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int act_on_offset_vid : 6;
        unsigned int reserved_0 : 2;
        unsigned int rtn_on_offset_vid : 6;
        unsigned int reserved_1 : 2;
        unsigned int rtn2act_offset_vid : 6;
        unsigned int reserved_2 : 10;
    } reg;
} SOC_PMCTRL_OCLDO1_OFFSET_VID_UNION;
#endif
#define SOC_PMCTRL_OCLDO1_OFFSET_VID_act_on_offset_vid_START (0)
#define SOC_PMCTRL_OCLDO1_OFFSET_VID_act_on_offset_vid_END (5)
#define SOC_PMCTRL_OCLDO1_OFFSET_VID_rtn_on_offset_vid_START (8)
#define SOC_PMCTRL_OCLDO1_OFFSET_VID_rtn_on_offset_vid_END (13)
#define SOC_PMCTRL_OCLDO1_OFFSET_VID_rtn2act_offset_vid_START (16)
#define SOC_PMCTRL_OCLDO1_OFFSET_VID_rtn2act_offset_vid_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int byp_hw_act_on_req : 1;
        unsigned int byp_hw_rtn_on_req : 1;
        unsigned int byp_hw_act_ramp_req : 1;
        unsigned int byp_hw_rtn2act_req : 1;
        unsigned int byp_hw_act2rtn_req : 1;
        unsigned int byp_hw_off_req : 1;
        unsigned int byp_pset_hw_ctrl : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_PMCTRL_OCLDO1_BYP_CTRL_UNION;
#endif
#define SOC_PMCTRL_OCLDO1_BYP_CTRL_byp_hw_act_on_req_START (0)
#define SOC_PMCTRL_OCLDO1_BYP_CTRL_byp_hw_act_on_req_END (0)
#define SOC_PMCTRL_OCLDO1_BYP_CTRL_byp_hw_rtn_on_req_START (1)
#define SOC_PMCTRL_OCLDO1_BYP_CTRL_byp_hw_rtn_on_req_END (1)
#define SOC_PMCTRL_OCLDO1_BYP_CTRL_byp_hw_act_ramp_req_START (2)
#define SOC_PMCTRL_OCLDO1_BYP_CTRL_byp_hw_act_ramp_req_END (2)
#define SOC_PMCTRL_OCLDO1_BYP_CTRL_byp_hw_rtn2act_req_START (3)
#define SOC_PMCTRL_OCLDO1_BYP_CTRL_byp_hw_rtn2act_req_END (3)
#define SOC_PMCTRL_OCLDO1_BYP_CTRL_byp_hw_act2rtn_req_START (4)
#define SOC_PMCTRL_OCLDO1_BYP_CTRL_byp_hw_act2rtn_req_END (4)
#define SOC_PMCTRL_OCLDO1_BYP_CTRL_byp_hw_off_req_START (5)
#define SOC_PMCTRL_OCLDO1_BYP_CTRL_byp_hw_off_req_END (5)
#define SOC_PMCTRL_OCLDO1_BYP_CTRL_byp_pset_hw_ctrl_START (6)
#define SOC_PMCTRL_OCLDO1_BYP_CTRL_byp_pset_hw_ctrl_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_act_on_req : 1;
        unsigned int sw_act_ramp_req : 1;
        unsigned int sw_off_req : 1;
        unsigned int fsm_rst : 1;
        unsigned int err_clr : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_PMCTRL_OCLDO1_SW_REQ_UNION;
#endif
#define SOC_PMCTRL_OCLDO1_SW_REQ_sw_act_on_req_START (0)
#define SOC_PMCTRL_OCLDO1_SW_REQ_sw_act_on_req_END (0)
#define SOC_PMCTRL_OCLDO1_SW_REQ_sw_act_ramp_req_START (1)
#define SOC_PMCTRL_OCLDO1_SW_REQ_sw_act_ramp_req_END (1)
#define SOC_PMCTRL_OCLDO1_SW_REQ_sw_off_req_START (2)
#define SOC_PMCTRL_OCLDO1_SW_REQ_sw_off_req_END (2)
#define SOC_PMCTRL_OCLDO1_SW_REQ_fsm_rst_START (3)
#define SOC_PMCTRL_OCLDO1_SW_REQ_fsm_rst_END (3)
#define SOC_PMCTRL_OCLDO1_SW_REQ_err_clr_START (4)
#define SOC_PMCTRL_OCLDO1_SW_REQ_err_clr_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fsm_stat : 4;
        unsigned int ramp_done : 1;
        unsigned int retention_mode : 1;
        unsigned int act_on_ramp : 1;
        unsigned int rtn_on_ramp : 1;
        unsigned int act_up_ramp : 1;
        unsigned int act_dn_ramp : 1;
        unsigned int act2rtn_ramp : 1;
        unsigned int rtn2act_ramp : 1;
        unsigned int off_ramp : 1;
        unsigned int reserved : 3;
        unsigned int ramp_cnt : 16;
    } reg;
} SOC_PMCTRL_OCLDO1_CTRL_DEBUG_UNION;
#endif
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_fsm_stat_START (0)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_fsm_stat_END (3)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_ramp_done_START (4)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_ramp_done_END (4)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_retention_mode_START (5)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_retention_mode_END (5)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_act_on_ramp_START (6)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_act_on_ramp_END (6)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_rtn_on_ramp_START (7)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_rtn_on_ramp_END (7)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_act_up_ramp_START (8)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_act_up_ramp_END (8)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_act_dn_ramp_START (9)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_act_dn_ramp_END (9)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_act2rtn_ramp_START (10)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_act2rtn_ramp_END (10)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_rtn2act_ramp_START (11)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_rtn2act_ramp_END (11)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_off_ramp_START (12)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_off_ramp_END (12)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_ramp_cnt_START (16)
#define SOC_PMCTRL_OCLDO1_CTRL_DEBUG_ramp_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int en_m_stat : 4;
        unsigned int vout_stat : 6;
        unsigned int m1_pcell_en_stat : 3;
        unsigned int m2_pcell_en_stat : 3;
        unsigned int m3_pcell_en_stat : 3;
        unsigned int m4_pcell_en_stat : 3;
        unsigned int pset_in_stat : 4;
        unsigned int vin_det_stat : 4;
        unsigned int en_v_stat : 1;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_OCLDO1_CTRL_STAT_UNION;
#endif
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_en_m_stat_START (0)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_en_m_stat_END (3)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_vout_stat_START (4)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_vout_stat_END (9)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_m1_pcell_en_stat_START (10)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_m1_pcell_en_stat_END (12)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_m2_pcell_en_stat_START (13)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_m2_pcell_en_stat_END (15)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_m3_pcell_en_stat_START (16)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_m3_pcell_en_stat_END (18)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_m4_pcell_en_stat_START (19)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_m4_pcell_en_stat_END (21)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_pset_in_stat_START (22)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_pset_in_stat_END (25)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_vin_det_stat_START (26)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_vin_det_stat_END (29)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_en_v_stat_START (30)
#define SOC_PMCTRL_OCLDO1_CTRL_STAT_en_v_stat_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocldo2_sw_en_m : 4;
        unsigned int ocldo2_sw_vout_set : 6;
        unsigned int ocldo2_sw_m1_pcell_en : 3;
        unsigned int ocldo2_sw_m2_pcell_en : 3;
        unsigned int ocldo2_sw_m3_pcell_en : 3;
        unsigned int ocldo2_sw_m4_pcell_en : 3;
        unsigned int ocldo2_sw_pset_in : 4;
        unsigned int ocldo2_sw_vin_det : 4;
        unsigned int ocldo2_sw_en_v : 1;
        unsigned int ocldo2_sw_ctrl1_chg : 1;
    } reg;
} SOC_PMCTRL_OCLDO2_SW_CTRL1_UNION;
#endif
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_en_m_START (0)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_en_m_END (3)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_vout_set_START (4)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_vout_set_END (9)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_m1_pcell_en_START (10)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_m1_pcell_en_END (12)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_m2_pcell_en_START (13)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_m2_pcell_en_END (15)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_m3_pcell_en_START (16)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_m3_pcell_en_END (18)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_m4_pcell_en_START (19)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_m4_pcell_en_END (21)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_pset_in_START (22)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_pset_in_END (25)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_vin_det_START (26)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_vin_det_END (29)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_en_v_START (30)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_en_v_END (30)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_ctrl1_chg_START (31)
#define SOC_PMCTRL_OCLDO2_SW_CTRL1_ocldo2_sw_ctrl1_chg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocldo_vin_out : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_PMCTRL_OCLDO2_ANA_STAT_UNION;
#endif
#define SOC_PMCTRL_OCLDO2_ANA_STAT_ocldo_vin_out_START (0)
#define SOC_PMCTRL_OCLDO2_ANA_STAT_ocldo_vin_out_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int act_on_err : 1;
        unsigned int rtn_on_err : 1;
        unsigned int act_ramp_up_err : 1;
        unsigned int act_ramp_dn_err : 1;
        unsigned int act2rtn_err : 1;
        unsigned int rtn2act_err : 1;
        unsigned int off_err : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_PMCTRL_OCLDO2_ERR_STAT_UNION;
#endif
#define SOC_PMCTRL_OCLDO2_ERR_STAT_act_on_err_START (0)
#define SOC_PMCTRL_OCLDO2_ERR_STAT_act_on_err_END (0)
#define SOC_PMCTRL_OCLDO2_ERR_STAT_rtn_on_err_START (1)
#define SOC_PMCTRL_OCLDO2_ERR_STAT_rtn_on_err_END (1)
#define SOC_PMCTRL_OCLDO2_ERR_STAT_act_ramp_up_err_START (2)
#define SOC_PMCTRL_OCLDO2_ERR_STAT_act_ramp_up_err_END (2)
#define SOC_PMCTRL_OCLDO2_ERR_STAT_act_ramp_dn_err_START (3)
#define SOC_PMCTRL_OCLDO2_ERR_STAT_act_ramp_dn_err_END (3)
#define SOC_PMCTRL_OCLDO2_ERR_STAT_act2rtn_err_START (4)
#define SOC_PMCTRL_OCLDO2_ERR_STAT_act2rtn_err_END (4)
#define SOC_PMCTRL_OCLDO2_ERR_STAT_rtn2act_err_START (5)
#define SOC_PMCTRL_OCLDO2_ERR_STAT_rtn2act_err_END (5)
#define SOC_PMCTRL_OCLDO2_ERR_STAT_off_err_START (6)
#define SOC_PMCTRL_OCLDO2_ERR_STAT_off_err_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int active_en_m : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_PMCTRL_OCLDO2_ACTIVE_CFG_UNION;
#endif
#define SOC_PMCTRL_OCLDO2_ACTIVE_CFG_active_en_m_START (0)
#define SOC_PMCTRL_OCLDO2_ACTIVE_CFG_active_en_m_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int retention_en_m : 4;
        unsigned int retention_vid : 6;
        unsigned int retention_pcell_en : 12;
        unsigned int reserved : 10;
    } reg;
} SOC_PMCTRL_OCLDO2_RETENTION_CFG_UNION;
#endif
#define SOC_PMCTRL_OCLDO2_RETENTION_CFG_retention_en_m_START (0)
#define SOC_PMCTRL_OCLDO2_RETENTION_CFG_retention_en_m_END (3)
#define SOC_PMCTRL_OCLDO2_RETENTION_CFG_retention_vid_START (4)
#define SOC_PMCTRL_OCLDO2_RETENTION_CFG_retention_vid_END (9)
#define SOC_PMCTRL_OCLDO2_RETENTION_CFG_retention_pcell_en_START (10)
#define SOC_PMCTRL_OCLDO2_RETENTION_CFG_retention_pcell_en_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int act_on_offset_vid : 6;
        unsigned int reserved_0 : 2;
        unsigned int rtn_on_offset_vid : 6;
        unsigned int reserved_1 : 2;
        unsigned int rtn2act_offset_vid : 6;
        unsigned int reserved_2 : 10;
    } reg;
} SOC_PMCTRL_OCLDO2_OFFSET_VID_UNION;
#endif
#define SOC_PMCTRL_OCLDO2_OFFSET_VID_act_on_offset_vid_START (0)
#define SOC_PMCTRL_OCLDO2_OFFSET_VID_act_on_offset_vid_END (5)
#define SOC_PMCTRL_OCLDO2_OFFSET_VID_rtn_on_offset_vid_START (8)
#define SOC_PMCTRL_OCLDO2_OFFSET_VID_rtn_on_offset_vid_END (13)
#define SOC_PMCTRL_OCLDO2_OFFSET_VID_rtn2act_offset_vid_START (16)
#define SOC_PMCTRL_OCLDO2_OFFSET_VID_rtn2act_offset_vid_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int byp_hw_act_on_req : 1;
        unsigned int byp_hw_rtn_on_req : 1;
        unsigned int byp_hw_act_ramp_req : 1;
        unsigned int byp_hw_rtn2act_req : 1;
        unsigned int byp_hw_act2rtn_req : 1;
        unsigned int byp_hw_off_req : 1;
        unsigned int byp_pset_hw_ctrl : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_PMCTRL_OCLDO2_BYP_CTRL_UNION;
#endif
#define SOC_PMCTRL_OCLDO2_BYP_CTRL_byp_hw_act_on_req_START (0)
#define SOC_PMCTRL_OCLDO2_BYP_CTRL_byp_hw_act_on_req_END (0)
#define SOC_PMCTRL_OCLDO2_BYP_CTRL_byp_hw_rtn_on_req_START (1)
#define SOC_PMCTRL_OCLDO2_BYP_CTRL_byp_hw_rtn_on_req_END (1)
#define SOC_PMCTRL_OCLDO2_BYP_CTRL_byp_hw_act_ramp_req_START (2)
#define SOC_PMCTRL_OCLDO2_BYP_CTRL_byp_hw_act_ramp_req_END (2)
#define SOC_PMCTRL_OCLDO2_BYP_CTRL_byp_hw_rtn2act_req_START (3)
#define SOC_PMCTRL_OCLDO2_BYP_CTRL_byp_hw_rtn2act_req_END (3)
#define SOC_PMCTRL_OCLDO2_BYP_CTRL_byp_hw_act2rtn_req_START (4)
#define SOC_PMCTRL_OCLDO2_BYP_CTRL_byp_hw_act2rtn_req_END (4)
#define SOC_PMCTRL_OCLDO2_BYP_CTRL_byp_hw_off_req_START (5)
#define SOC_PMCTRL_OCLDO2_BYP_CTRL_byp_hw_off_req_END (5)
#define SOC_PMCTRL_OCLDO2_BYP_CTRL_byp_pset_hw_ctrl_START (6)
#define SOC_PMCTRL_OCLDO2_BYP_CTRL_byp_pset_hw_ctrl_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_act_on_req : 1;
        unsigned int sw_act_ramp_req : 1;
        unsigned int sw_off_req : 1;
        unsigned int fsm_rst : 1;
        unsigned int err_clr : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_PMCTRL_OCLDO2_SW_REQ_UNION;
#endif
#define SOC_PMCTRL_OCLDO2_SW_REQ_sw_act_on_req_START (0)
#define SOC_PMCTRL_OCLDO2_SW_REQ_sw_act_on_req_END (0)
#define SOC_PMCTRL_OCLDO2_SW_REQ_sw_act_ramp_req_START (1)
#define SOC_PMCTRL_OCLDO2_SW_REQ_sw_act_ramp_req_END (1)
#define SOC_PMCTRL_OCLDO2_SW_REQ_sw_off_req_START (2)
#define SOC_PMCTRL_OCLDO2_SW_REQ_sw_off_req_END (2)
#define SOC_PMCTRL_OCLDO2_SW_REQ_fsm_rst_START (3)
#define SOC_PMCTRL_OCLDO2_SW_REQ_fsm_rst_END (3)
#define SOC_PMCTRL_OCLDO2_SW_REQ_err_clr_START (4)
#define SOC_PMCTRL_OCLDO2_SW_REQ_err_clr_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fsm_stat : 4;
        unsigned int ramp_done : 1;
        unsigned int retention_mode : 1;
        unsigned int act_on_ramp : 1;
        unsigned int rtn_on_ramp : 1;
        unsigned int act_up_ramp : 1;
        unsigned int act_dn_ramp : 1;
        unsigned int act2rtn_ramp : 1;
        unsigned int rtn2act_ramp : 1;
        unsigned int off_ramp : 1;
        unsigned int reserved : 3;
        unsigned int ramp_cnt : 16;
    } reg;
} SOC_PMCTRL_OCLDO2_CTRL_DEBUG_UNION;
#endif
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_fsm_stat_START (0)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_fsm_stat_END (3)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_ramp_done_START (4)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_ramp_done_END (4)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_retention_mode_START (5)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_retention_mode_END (5)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_act_on_ramp_START (6)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_act_on_ramp_END (6)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_rtn_on_ramp_START (7)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_rtn_on_ramp_END (7)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_act_up_ramp_START (8)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_act_up_ramp_END (8)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_act_dn_ramp_START (9)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_act_dn_ramp_END (9)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_act2rtn_ramp_START (10)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_act2rtn_ramp_END (10)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_rtn2act_ramp_START (11)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_rtn2act_ramp_END (11)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_off_ramp_START (12)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_off_ramp_END (12)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_ramp_cnt_START (16)
#define SOC_PMCTRL_OCLDO2_CTRL_DEBUG_ramp_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int en_m_stat : 4;
        unsigned int vout_stat : 6;
        unsigned int m1_pcell_en_stat : 3;
        unsigned int m2_pcell_en_stat : 3;
        unsigned int m3_pcell_en_stat : 3;
        unsigned int m4_pcell_en_stat : 3;
        unsigned int pset_in_stat : 4;
        unsigned int vin_det_stat : 4;
        unsigned int en_v_stat : 1;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_OCLDO2_CTRL_STAT_UNION;
#endif
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_en_m_stat_START (0)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_en_m_stat_END (3)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_vout_stat_START (4)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_vout_stat_END (9)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_m1_pcell_en_stat_START (10)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_m1_pcell_en_stat_END (12)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_m2_pcell_en_stat_START (13)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_m2_pcell_en_stat_END (15)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_m3_pcell_en_stat_START (16)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_m3_pcell_en_stat_END (18)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_m4_pcell_en_stat_START (19)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_m4_pcell_en_stat_END (21)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_pset_in_stat_START (22)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_pset_in_stat_END (25)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_vin_det_stat_START (26)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_vin_det_stat_END (29)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_en_v_stat_START (30)
#define SOC_PMCTRL_OCLDO2_CTRL_STAT_en_v_stat_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocldo3_sw_en_m : 4;
        unsigned int ocldo3_sw_vout_set : 6;
        unsigned int ocldo3_sw_m1_pcell_en : 3;
        unsigned int ocldo3_sw_m2_pcell_en : 3;
        unsigned int ocldo3_sw_m3_pcell_en : 3;
        unsigned int ocldo3_sw_m4_pcell_en : 3;
        unsigned int ocldo3_sw_pset_in : 4;
        unsigned int ocldo3_sw_vin_det : 4;
        unsigned int ocldo3_sw_en_v : 1;
        unsigned int ocldo3_sw_ctrl1_chg : 1;
    } reg;
} SOC_PMCTRL_OCLDO3_SW_CTRL1_UNION;
#endif
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_en_m_START (0)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_en_m_END (3)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_vout_set_START (4)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_vout_set_END (9)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_m1_pcell_en_START (10)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_m1_pcell_en_END (12)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_m2_pcell_en_START (13)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_m2_pcell_en_END (15)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_m3_pcell_en_START (16)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_m3_pcell_en_END (18)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_m4_pcell_en_START (19)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_m4_pcell_en_END (21)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_pset_in_START (22)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_pset_in_END (25)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_vin_det_START (26)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_vin_det_END (29)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_en_v_START (30)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_en_v_END (30)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_ctrl1_chg_START (31)
#define SOC_PMCTRL_OCLDO3_SW_CTRL1_ocldo3_sw_ctrl1_chg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocldo_vin_out : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_PMCTRL_OCLDO3_ANA_STAT_UNION;
#endif
#define SOC_PMCTRL_OCLDO3_ANA_STAT_ocldo_vin_out_START (0)
#define SOC_PMCTRL_OCLDO3_ANA_STAT_ocldo_vin_out_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int act_on_err : 1;
        unsigned int rtn_on_err : 1;
        unsigned int act_ramp_up_err : 1;
        unsigned int act_ramp_dn_err : 1;
        unsigned int act2rtn_err : 1;
        unsigned int rtn2act_err : 1;
        unsigned int off_err : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_PMCTRL_OCLDO3_ERR_STAT_UNION;
#endif
#define SOC_PMCTRL_OCLDO3_ERR_STAT_act_on_err_START (0)
#define SOC_PMCTRL_OCLDO3_ERR_STAT_act_on_err_END (0)
#define SOC_PMCTRL_OCLDO3_ERR_STAT_rtn_on_err_START (1)
#define SOC_PMCTRL_OCLDO3_ERR_STAT_rtn_on_err_END (1)
#define SOC_PMCTRL_OCLDO3_ERR_STAT_act_ramp_up_err_START (2)
#define SOC_PMCTRL_OCLDO3_ERR_STAT_act_ramp_up_err_END (2)
#define SOC_PMCTRL_OCLDO3_ERR_STAT_act_ramp_dn_err_START (3)
#define SOC_PMCTRL_OCLDO3_ERR_STAT_act_ramp_dn_err_END (3)
#define SOC_PMCTRL_OCLDO3_ERR_STAT_act2rtn_err_START (4)
#define SOC_PMCTRL_OCLDO3_ERR_STAT_act2rtn_err_END (4)
#define SOC_PMCTRL_OCLDO3_ERR_STAT_rtn2act_err_START (5)
#define SOC_PMCTRL_OCLDO3_ERR_STAT_rtn2act_err_END (5)
#define SOC_PMCTRL_OCLDO3_ERR_STAT_off_err_START (6)
#define SOC_PMCTRL_OCLDO3_ERR_STAT_off_err_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int active_en_m : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_PMCTRL_OCLDO3_ACTIVE_CFG_UNION;
#endif
#define SOC_PMCTRL_OCLDO3_ACTIVE_CFG_active_en_m_START (0)
#define SOC_PMCTRL_OCLDO3_ACTIVE_CFG_active_en_m_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int retention_en_m : 4;
        unsigned int retention_vid : 6;
        unsigned int retention_pcell_en : 12;
        unsigned int reserved : 10;
    } reg;
} SOC_PMCTRL_OCLDO3_RETENTION_CFG_UNION;
#endif
#define SOC_PMCTRL_OCLDO3_RETENTION_CFG_retention_en_m_START (0)
#define SOC_PMCTRL_OCLDO3_RETENTION_CFG_retention_en_m_END (3)
#define SOC_PMCTRL_OCLDO3_RETENTION_CFG_retention_vid_START (4)
#define SOC_PMCTRL_OCLDO3_RETENTION_CFG_retention_vid_END (9)
#define SOC_PMCTRL_OCLDO3_RETENTION_CFG_retention_pcell_en_START (10)
#define SOC_PMCTRL_OCLDO3_RETENTION_CFG_retention_pcell_en_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int act_on_offset_vid : 6;
        unsigned int reserved_0 : 2;
        unsigned int rtn_on_offset_vid : 6;
        unsigned int reserved_1 : 2;
        unsigned int rtn2act_offset_vid : 6;
        unsigned int reserved_2 : 10;
    } reg;
} SOC_PMCTRL_OCLDO3_OFFSET_VID_UNION;
#endif
#define SOC_PMCTRL_OCLDO3_OFFSET_VID_act_on_offset_vid_START (0)
#define SOC_PMCTRL_OCLDO3_OFFSET_VID_act_on_offset_vid_END (5)
#define SOC_PMCTRL_OCLDO3_OFFSET_VID_rtn_on_offset_vid_START (8)
#define SOC_PMCTRL_OCLDO3_OFFSET_VID_rtn_on_offset_vid_END (13)
#define SOC_PMCTRL_OCLDO3_OFFSET_VID_rtn2act_offset_vid_START (16)
#define SOC_PMCTRL_OCLDO3_OFFSET_VID_rtn2act_offset_vid_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int byp_hw_act_on_req : 1;
        unsigned int byp_hw_rtn_on_req : 1;
        unsigned int byp_hw_act_ramp_req : 1;
        unsigned int byp_hw_rtn2act_req : 1;
        unsigned int byp_hw_act2rtn_req : 1;
        unsigned int byp_hw_off_req : 1;
        unsigned int byp_pset_hw_ctrl : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_PMCTRL_OCLDO3_BYP_CTRL_UNION;
#endif
#define SOC_PMCTRL_OCLDO3_BYP_CTRL_byp_hw_act_on_req_START (0)
#define SOC_PMCTRL_OCLDO3_BYP_CTRL_byp_hw_act_on_req_END (0)
#define SOC_PMCTRL_OCLDO3_BYP_CTRL_byp_hw_rtn_on_req_START (1)
#define SOC_PMCTRL_OCLDO3_BYP_CTRL_byp_hw_rtn_on_req_END (1)
#define SOC_PMCTRL_OCLDO3_BYP_CTRL_byp_hw_act_ramp_req_START (2)
#define SOC_PMCTRL_OCLDO3_BYP_CTRL_byp_hw_act_ramp_req_END (2)
#define SOC_PMCTRL_OCLDO3_BYP_CTRL_byp_hw_rtn2act_req_START (3)
#define SOC_PMCTRL_OCLDO3_BYP_CTRL_byp_hw_rtn2act_req_END (3)
#define SOC_PMCTRL_OCLDO3_BYP_CTRL_byp_hw_act2rtn_req_START (4)
#define SOC_PMCTRL_OCLDO3_BYP_CTRL_byp_hw_act2rtn_req_END (4)
#define SOC_PMCTRL_OCLDO3_BYP_CTRL_byp_hw_off_req_START (5)
#define SOC_PMCTRL_OCLDO3_BYP_CTRL_byp_hw_off_req_END (5)
#define SOC_PMCTRL_OCLDO3_BYP_CTRL_byp_pset_hw_ctrl_START (6)
#define SOC_PMCTRL_OCLDO3_BYP_CTRL_byp_pset_hw_ctrl_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_act_on_req : 1;
        unsigned int sw_act_ramp_req : 1;
        unsigned int sw_off_req : 1;
        unsigned int fsm_rst : 1;
        unsigned int err_clr : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_PMCTRL_OCLDO3_SW_REQ_UNION;
#endif
#define SOC_PMCTRL_OCLDO3_SW_REQ_sw_act_on_req_START (0)
#define SOC_PMCTRL_OCLDO3_SW_REQ_sw_act_on_req_END (0)
#define SOC_PMCTRL_OCLDO3_SW_REQ_sw_act_ramp_req_START (1)
#define SOC_PMCTRL_OCLDO3_SW_REQ_sw_act_ramp_req_END (1)
#define SOC_PMCTRL_OCLDO3_SW_REQ_sw_off_req_START (2)
#define SOC_PMCTRL_OCLDO3_SW_REQ_sw_off_req_END (2)
#define SOC_PMCTRL_OCLDO3_SW_REQ_fsm_rst_START (3)
#define SOC_PMCTRL_OCLDO3_SW_REQ_fsm_rst_END (3)
#define SOC_PMCTRL_OCLDO3_SW_REQ_err_clr_START (4)
#define SOC_PMCTRL_OCLDO3_SW_REQ_err_clr_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fsm_stat : 4;
        unsigned int ramp_done : 1;
        unsigned int retention_mode : 1;
        unsigned int act_on_ramp : 1;
        unsigned int rtn_on_ramp : 1;
        unsigned int act_up_ramp : 1;
        unsigned int act_dn_ramp : 1;
        unsigned int act2rtn_ramp : 1;
        unsigned int rtn2act_ramp : 1;
        unsigned int off_ramp : 1;
        unsigned int reserved : 3;
        unsigned int ramp_cnt : 16;
    } reg;
} SOC_PMCTRL_OCLDO3_CTRL_DEBUG_UNION;
#endif
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_fsm_stat_START (0)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_fsm_stat_END (3)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_ramp_done_START (4)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_ramp_done_END (4)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_retention_mode_START (5)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_retention_mode_END (5)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_act_on_ramp_START (6)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_act_on_ramp_END (6)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_rtn_on_ramp_START (7)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_rtn_on_ramp_END (7)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_act_up_ramp_START (8)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_act_up_ramp_END (8)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_act_dn_ramp_START (9)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_act_dn_ramp_END (9)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_act2rtn_ramp_START (10)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_act2rtn_ramp_END (10)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_rtn2act_ramp_START (11)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_rtn2act_ramp_END (11)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_off_ramp_START (12)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_off_ramp_END (12)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_ramp_cnt_START (16)
#define SOC_PMCTRL_OCLDO3_CTRL_DEBUG_ramp_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int en_m_stat : 4;
        unsigned int vout_stat : 6;
        unsigned int m1_pcell_en_stat : 3;
        unsigned int m2_pcell_en_stat : 3;
        unsigned int m3_pcell_en_stat : 3;
        unsigned int m4_pcell_en_stat : 3;
        unsigned int pset_in_stat : 4;
        unsigned int vin_det_stat : 4;
        unsigned int en_v_stat : 1;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_OCLDO3_CTRL_STAT_UNION;
#endif
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_en_m_stat_START (0)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_en_m_stat_END (3)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_vout_stat_START (4)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_vout_stat_END (9)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_m1_pcell_en_stat_START (10)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_m1_pcell_en_stat_END (12)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_m2_pcell_en_stat_START (13)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_m2_pcell_en_stat_END (15)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_m3_pcell_en_stat_START (16)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_m3_pcell_en_stat_END (18)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_m4_pcell_en_stat_START (19)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_m4_pcell_en_stat_END (21)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_pset_in_stat_START (22)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_pset_in_stat_END (25)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_vin_det_stat_START (26)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_vin_det_stat_END (29)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_en_v_stat_START (30)
#define SOC_PMCTRL_OCLDO3_CTRL_STAT_en_v_stat_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int act_on_cnt : 16;
        unsigned int act_on_wait_cnt : 16;
    } reg;
} SOC_PMCTRL_OCLDO_GLB_ACT_START_TIME_UNION;
#endif
#define SOC_PMCTRL_OCLDO_GLB_ACT_START_TIME_act_on_cnt_START (0)
#define SOC_PMCTRL_OCLDO_GLB_ACT_START_TIME_act_on_cnt_END (15)
#define SOC_PMCTRL_OCLDO_GLB_ACT_START_TIME_act_on_wait_cnt_START (16)
#define SOC_PMCTRL_OCLDO_GLB_ACT_START_TIME_act_on_wait_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtn_on_cnt : 16;
        unsigned int rtn_on_wait_cnt : 16;
    } reg;
} SOC_PMCTRL_OCLDO_GLB_RTN_START_TIME_UNION;
#endif
#define SOC_PMCTRL_OCLDO_GLB_RTN_START_TIME_rtn_on_cnt_START (0)
#define SOC_PMCTRL_OCLDO_GLB_RTN_START_TIME_rtn_on_cnt_END (15)
#define SOC_PMCTRL_OCLDO_GLB_RTN_START_TIME_rtn_on_wait_cnt_START (16)
#define SOC_PMCTRL_OCLDO_GLB_RTN_START_TIME_rtn_on_wait_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int step_ramp_on : 8;
        unsigned int step_ramp_off : 8;
        unsigned int step_ramp_up : 8;
        unsigned int step_ramp_dn : 8;
    } reg;
} SOC_PMCTRL_OCLDO_GLB_STEP_TIME_UNION;
#endif
#define SOC_PMCTRL_OCLDO_GLB_STEP_TIME_step_ramp_on_START (0)
#define SOC_PMCTRL_OCLDO_GLB_STEP_TIME_step_ramp_on_END (7)
#define SOC_PMCTRL_OCLDO_GLB_STEP_TIME_step_ramp_off_START (8)
#define SOC_PMCTRL_OCLDO_GLB_STEP_TIME_step_ramp_off_END (15)
#define SOC_PMCTRL_OCLDO_GLB_STEP_TIME_step_ramp_up_START (16)
#define SOC_PMCTRL_OCLDO_GLB_STEP_TIME_step_ramp_up_END (23)
#define SOC_PMCTRL_OCLDO_GLB_STEP_TIME_step_ramp_dn_START (24)
#define SOC_PMCTRL_OCLDO_GLB_STEP_TIME_step_ramp_dn_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_profile_recovery_bypass : 1;
        unsigned int bigfast1_profile_recovery_bypass : 1;
        unsigned int bigslow0_profile_recovery_bypass : 1;
        unsigned int bigslow1_profile_recovery_bypass : 1;
        unsigned int reserved : 12;
        unsigned int profile_recovery_bypass_msk : 16;
    } reg;
} SOC_PMCTRL_PROFILE_RECOVERY_BYPASS_UNION;
#endif
#define SOC_PMCTRL_PROFILE_RECOVERY_BYPASS_bigfast0_profile_recovery_bypass_START (0)
#define SOC_PMCTRL_PROFILE_RECOVERY_BYPASS_bigfast0_profile_recovery_bypass_END (0)
#define SOC_PMCTRL_PROFILE_RECOVERY_BYPASS_bigfast1_profile_recovery_bypass_START (1)
#define SOC_PMCTRL_PROFILE_RECOVERY_BYPASS_bigfast1_profile_recovery_bypass_END (1)
#define SOC_PMCTRL_PROFILE_RECOVERY_BYPASS_bigslow0_profile_recovery_bypass_START (2)
#define SOC_PMCTRL_PROFILE_RECOVERY_BYPASS_bigslow0_profile_recovery_bypass_END (2)
#define SOC_PMCTRL_PROFILE_RECOVERY_BYPASS_bigslow1_profile_recovery_bypass_START (3)
#define SOC_PMCTRL_PROFILE_RECOVERY_BYPASS_bigslow1_profile_recovery_bypass_END (3)
#define SOC_PMCTRL_PROFILE_RECOVERY_BYPASS_profile_recovery_bypass_msk_START (16)
#define SOC_PMCTRL_PROFILE_RECOVERY_BYPASS_profile_recovery_bypass_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_profile_recovery_clear : 1;
        unsigned int bigfast1_profile_recovery_clear : 1;
        unsigned int bigslow0_profile_recovery_clear : 1;
        unsigned int bigslow1_profile_recovery_clear : 1;
        unsigned int reserved : 12;
        unsigned int profile_recovery_clear_msk : 16;
    } reg;
} SOC_PMCTRL_PROFILE_RECOVERY_CLEAR_UNION;
#endif
#define SOC_PMCTRL_PROFILE_RECOVERY_CLEAR_bigfast0_profile_recovery_clear_START (0)
#define SOC_PMCTRL_PROFILE_RECOVERY_CLEAR_bigfast0_profile_recovery_clear_END (0)
#define SOC_PMCTRL_PROFILE_RECOVERY_CLEAR_bigfast1_profile_recovery_clear_START (1)
#define SOC_PMCTRL_PROFILE_RECOVERY_CLEAR_bigfast1_profile_recovery_clear_END (1)
#define SOC_PMCTRL_PROFILE_RECOVERY_CLEAR_bigslow0_profile_recovery_clear_START (2)
#define SOC_PMCTRL_PROFILE_RECOVERY_CLEAR_bigslow0_profile_recovery_clear_END (2)
#define SOC_PMCTRL_PROFILE_RECOVERY_CLEAR_bigslow1_profile_recovery_clear_START (3)
#define SOC_PMCTRL_PROFILE_RECOVERY_CLEAR_bigslow1_profile_recovery_clear_END (3)
#define SOC_PMCTRL_PROFILE_RECOVERY_CLEAR_profile_recovery_clear_msk_START (16)
#define SOC_PMCTRL_PROFILE_RECOVERY_CLEAR_profile_recovery_clear_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast_unified_power_mode : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BIGFAST_POWERSUPPLYMODE_UNION;
#endif
#define SOC_PMCTRL_BIGFAST_POWERSUPPLYMODE_bigfast_unified_power_mode_START (0)
#define SOC_PMCTRL_BIGFAST_POWERSUPPLYMODE_bigfast_unified_power_mode_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_unified_vol_idx_stat : 8;
        unsigned int bigfast1_unified_vol_idx_stat : 8;
        unsigned int bigfast_unified_pmu_vol_idx_stat : 8;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_BIGFAST_UNIFIED_VOL_STAT_UNION;
#endif
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_STAT_bigfast0_unified_vol_idx_stat_START (0)
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_STAT_bigfast0_unified_vol_idx_stat_END (7)
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_STAT_bigfast1_unified_vol_idx_stat_START (8)
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_STAT_bigfast1_unified_vol_idx_stat_END (15)
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_STAT_bigfast_unified_pmu_vol_idx_stat_START (16)
#define SOC_PMCTRL_BIGFAST_UNIFIED_VOL_STAT_bigfast_unified_pmu_vol_idx_stat_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_dvfs_stat : 5;
        unsigned int reserved_0 : 3;
        unsigned int bigfast1_dvfs_stat : 5;
        unsigned int reserved_1 : 19;
    } reg;
} SOC_PMCTRL_BIGFAST_DVFS_STAT_UNION;
#endif
#define SOC_PMCTRL_BIGFAST_DVFS_STAT_bigfast0_dvfs_stat_START (0)
#define SOC_PMCTRL_BIGFAST_DVFS_STAT_bigfast0_dvfs_stat_END (4)
#define SOC_PMCTRL_BIGFAST_DVFS_STAT_bigfast1_dvfs_stat_START (8)
#define SOC_PMCTRL_BIGFAST_DVFS_STAT_bigfast1_dvfs_stat_END (12)
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
        unsigned int bigfast0_pclkendbg_sel_cfg : 2;
        unsigned int bigfast0_atclken_sel_cfg : 2;
        unsigned int bigfast0_aclkenm_sel_cfg : 3;
        unsigned int bigfast0_atclken_l_sel_cfg : 1;
        unsigned int bigfast0_pclkendbg_sel_stat : 2;
        unsigned int bigfast0_atclken_sel_stat : 2;
        unsigned int bigfast0_aclkenm_sel_stat : 3;
        unsigned int bigfast0_atclken_l_sel_stat : 1;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_BIGFAST0_CLKDIV_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_CLKDIV_bigfast0_pclkendbg_sel_cfg_START (0)
#define SOC_PMCTRL_BIGFAST0_CLKDIV_bigfast0_pclkendbg_sel_cfg_END (1)
#define SOC_PMCTRL_BIGFAST0_CLKDIV_bigfast0_atclken_sel_cfg_START (2)
#define SOC_PMCTRL_BIGFAST0_CLKDIV_bigfast0_atclken_sel_cfg_END (3)
#define SOC_PMCTRL_BIGFAST0_CLKDIV_bigfast0_aclkenm_sel_cfg_START (4)
#define SOC_PMCTRL_BIGFAST0_CLKDIV_bigfast0_aclkenm_sel_cfg_END (6)
#define SOC_PMCTRL_BIGFAST0_CLKDIV_bigfast0_atclken_l_sel_cfg_START (7)
#define SOC_PMCTRL_BIGFAST0_CLKDIV_bigfast0_atclken_l_sel_cfg_END (7)
#define SOC_PMCTRL_BIGFAST0_CLKDIV_bigfast0_pclkendbg_sel_stat_START (8)
#define SOC_PMCTRL_BIGFAST0_CLKDIV_bigfast0_pclkendbg_sel_stat_END (9)
#define SOC_PMCTRL_BIGFAST0_CLKDIV_bigfast0_atclken_sel_stat_START (10)
#define SOC_PMCTRL_BIGFAST0_CLKDIV_bigfast0_atclken_sel_stat_END (11)
#define SOC_PMCTRL_BIGFAST0_CLKDIV_bigfast0_aclkenm_sel_stat_START (12)
#define SOC_PMCTRL_BIGFAST0_CLKDIV_bigfast0_aclkenm_sel_stat_END (14)
#define SOC_PMCTRL_BIGFAST0_CLKDIV_bigfast0_atclken_l_sel_stat_START (15)
#define SOC_PMCTRL_BIGFAST0_CLKDIV_bigfast0_atclken_l_sel_stat_END (15)
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
        unsigned int bigfast0_pclkendbg_sel_prof : 2;
        unsigned int bigfast0_atclken_sel_prof : 2;
        unsigned int bigfast0_aclkenm_sel_prof : 3;
        unsigned int bigfast0_atclken_l_sel_prof : 1;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_BIGFAST0_CLKPROFILE1_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE1_bigfast0_pclkendbg_sel_prof_START (0)
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE1_bigfast0_pclkendbg_sel_prof_END (1)
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE1_bigfast0_atclken_sel_prof_START (2)
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE1_bigfast0_atclken_sel_prof_END (3)
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE1_bigfast0_aclkenm_sel_prof_START (4)
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE1_bigfast0_aclkenm_sel_prof_END (6)
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE1_bigfast0_atclken_l_sel_prof_START (7)
#define SOC_PMCTRL_BIGFAST0_CLKPROFILE1_bigfast0_atclken_l_sel_prof_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_vol_mod : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BIGFAST0_VOLMOD_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_VOLMOD_bigfast0_vol_mod_START (0)
#define SOC_PMCTRL_BIGFAST0_VOLMOD_bigfast0_vol_mod_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_vol_idx : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_BIGFAST0_VOLPROFILE_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_VOLPROFILE_bigfast0_vol_idx_START (0)
#define SOC_PMCTRL_BIGFAST0_VOLPROFILE_bigfast0_vol_idx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_vol_up_step_time : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_PMCTRL_BIGFAST0_VOLUPSTEPTIME_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_VOLUPSTEPTIME_bigfast0_vol_up_step_time_START (0)
#define SOC_PMCTRL_BIGFAST0_VOLUPSTEPTIME_bigfast0_vol_up_step_time_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_pmu_up_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_BIGFAST0_PMU_UP_HOLDTIME_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_PMU_UP_HOLDTIME_bigfast0_pmu_up_vol_hold_time_START (0)
#define SOC_PMCTRL_BIGFAST0_PMU_UP_HOLDTIME_bigfast0_pmu_up_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_pmu_dn_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_BIGFAST0_PMU_DN_HOLDTIME_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_PMU_DN_HOLDTIME_bigfast0_pmu_dn_vol_hold_time_START (0)
#define SOC_PMCTRL_BIGFAST0_PMU_DN_HOLDTIME_bigfast0_pmu_dn_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_pmu_sel : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_BIGFAST0_PMUSEL_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_PMUSEL_bigfast0_pmu_sel_START (0)
#define SOC_PMCTRL_BIGFAST0_PMUSEL_bigfast0_pmu_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_gdvfs_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BIGFAST0_GDVFS_EN_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_GDVFS_EN_bigfast0_gdvfs_en_START (0)
#define SOC_PMCTRL_BIGFAST0_GDVFS_EN_bigfast0_gdvfs_en_END (0)
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
        unsigned int bigfast0_fbdiv_step : 2;
        unsigned int reserved_0 : 2;
        unsigned int bigfast0_apll_cfg_time : 12;
        unsigned int bigfast0_clkdiv_time : 7;
        unsigned int reserved_1 : 9;
    } reg;
} SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE1_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE1_bigfast0_fbdiv_step_START (0)
#define SOC_PMCTRL_BIGFAST0_GDVFS_PROFILE1_bigfast0_fbdiv_step_END (1)
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
        unsigned int bigfast0_vol_idx_cfg : 8;
        unsigned int bigfast0_vol_chg_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int bigfast0_vol_idx_stat : 8;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_BIGFAST0_VOLIDX_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_VOLIDX_bigfast0_vol_idx_cfg_START (0)
#define SOC_PMCTRL_BIGFAST0_VOLIDX_bigfast0_vol_idx_cfg_END (7)
#define SOC_PMCTRL_BIGFAST0_VOLIDX_bigfast0_vol_chg_sftreq_START (8)
#define SOC_PMCTRL_BIGFAST0_VOLIDX_bigfast0_vol_chg_sftreq_END (8)
#define SOC_PMCTRL_BIGFAST0_VOLIDX_bigfast0_vol_idx_stat_START (12)
#define SOC_PMCTRL_BIGFAST0_VOLIDX_bigfast0_vol_idx_stat_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast0_initial_vol_idx : 8;
        unsigned int bigfast0_initialvol_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 20;
    } reg;
} SOC_PMCTRL_BIGFAST0_INITIALVOL_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_INITIALVOL_bigfast0_initial_vol_idx_START (0)
#define SOC_PMCTRL_BIGFAST0_INITIALVOL_bigfast0_initial_vol_idx_END (7)
#define SOC_PMCTRL_BIGFAST0_INITIALVOL_bigfast0_initialvol_sftreq_START (8)
#define SOC_PMCTRL_BIGFAST0_INITIALVOL_bigfast0_initialvol_sftreq_END (8)
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
        unsigned int bigfast0_dvfs_sel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BIGFAST0_DVFS_SEL_UNION;
#endif
#define SOC_PMCTRL_BIGFAST0_DVFS_SEL_bigfast0_dvfs_sel_START (0)
#define SOC_PMCTRL_BIGFAST0_DVFS_SEL_bigfast0_dvfs_sel_END (0)
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
        unsigned int bigfast1_pclkendbg_sel_cfg : 2;
        unsigned int bigfast1_atclken_sel_cfg : 2;
        unsigned int bigfast1_aclkenm_sel_cfg : 3;
        unsigned int bigfast1_atclken_l_sel_cfg : 1;
        unsigned int bigfast1_pclkendbg_sel_stat : 2;
        unsigned int bigfast1_atclken_sel_stat : 2;
        unsigned int bigfast1_aclkenm_sel_stat : 3;
        unsigned int bigfast1_atclken_l_sel_stat : 1;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_BIGFAST1_CLKDIV_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_CLKDIV_bigfast1_pclkendbg_sel_cfg_START (0)
#define SOC_PMCTRL_BIGFAST1_CLKDIV_bigfast1_pclkendbg_sel_cfg_END (1)
#define SOC_PMCTRL_BIGFAST1_CLKDIV_bigfast1_atclken_sel_cfg_START (2)
#define SOC_PMCTRL_BIGFAST1_CLKDIV_bigfast1_atclken_sel_cfg_END (3)
#define SOC_PMCTRL_BIGFAST1_CLKDIV_bigfast1_aclkenm_sel_cfg_START (4)
#define SOC_PMCTRL_BIGFAST1_CLKDIV_bigfast1_aclkenm_sel_cfg_END (6)
#define SOC_PMCTRL_BIGFAST1_CLKDIV_bigfast1_atclken_l_sel_cfg_START (7)
#define SOC_PMCTRL_BIGFAST1_CLKDIV_bigfast1_atclken_l_sel_cfg_END (7)
#define SOC_PMCTRL_BIGFAST1_CLKDIV_bigfast1_pclkendbg_sel_stat_START (8)
#define SOC_PMCTRL_BIGFAST1_CLKDIV_bigfast1_pclkendbg_sel_stat_END (9)
#define SOC_PMCTRL_BIGFAST1_CLKDIV_bigfast1_atclken_sel_stat_START (10)
#define SOC_PMCTRL_BIGFAST1_CLKDIV_bigfast1_atclken_sel_stat_END (11)
#define SOC_PMCTRL_BIGFAST1_CLKDIV_bigfast1_aclkenm_sel_stat_START (12)
#define SOC_PMCTRL_BIGFAST1_CLKDIV_bigfast1_aclkenm_sel_stat_END (14)
#define SOC_PMCTRL_BIGFAST1_CLKDIV_bigfast1_atclken_l_sel_stat_START (15)
#define SOC_PMCTRL_BIGFAST1_CLKDIV_bigfast1_atclken_l_sel_stat_END (15)
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
        unsigned int bigfast1_pclkendbg_sel_prof : 2;
        unsigned int bigfast1_atclken_sel_prof : 2;
        unsigned int bigfast1_aclkenm_sel_prof : 3;
        unsigned int bigfast1_atclken_l_sel_prof : 1;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_BIGFAST1_CLKPROFILE1_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE1_bigfast1_pclkendbg_sel_prof_START (0)
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE1_bigfast1_pclkendbg_sel_prof_END (1)
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE1_bigfast1_atclken_sel_prof_START (2)
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE1_bigfast1_atclken_sel_prof_END (3)
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE1_bigfast1_aclkenm_sel_prof_START (4)
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE1_bigfast1_aclkenm_sel_prof_END (6)
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE1_bigfast1_atclken_l_sel_prof_START (7)
#define SOC_PMCTRL_BIGFAST1_CLKPROFILE1_bigfast1_atclken_l_sel_prof_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast1_vol_mod : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BIGFAST1_VOLMOD_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_VOLMOD_bigfast1_vol_mod_START (0)
#define SOC_PMCTRL_BIGFAST1_VOLMOD_bigfast1_vol_mod_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast1_vol_idx : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_BIGFAST1_VOLPROFILE_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_VOLPROFILE_bigfast1_vol_idx_START (0)
#define SOC_PMCTRL_BIGFAST1_VOLPROFILE_bigfast1_vol_idx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast1_vol_up_step_time : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_PMCTRL_BIGFAST1_VOLUPSTEPTIME_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_VOLUPSTEPTIME_bigfast1_vol_up_step_time_START (0)
#define SOC_PMCTRL_BIGFAST1_VOLUPSTEPTIME_bigfast1_vol_up_step_time_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast1_pmu_up_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_BIGFAST1_PMU_UP_HOLDTIME_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_PMU_UP_HOLDTIME_bigfast1_pmu_up_vol_hold_time_START (0)
#define SOC_PMCTRL_BIGFAST1_PMU_UP_HOLDTIME_bigfast1_pmu_up_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast1_pmu_dn_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_BIGFAST1_PMU_DN_HOLDTIME_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_PMU_DN_HOLDTIME_bigfast1_pmu_dn_vol_hold_time_START (0)
#define SOC_PMCTRL_BIGFAST1_PMU_DN_HOLDTIME_bigfast1_pmu_dn_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast1_pmu_sel : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_BIGFAST1_PMUSEL_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_PMUSEL_bigfast1_pmu_sel_START (0)
#define SOC_PMCTRL_BIGFAST1_PMUSEL_bigfast1_pmu_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast1_gdvfs_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BIGFAST1_GDVFS_EN_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_GDVFS_EN_bigfast1_gdvfs_en_START (0)
#define SOC_PMCTRL_BIGFAST1_GDVFS_EN_bigfast1_gdvfs_en_END (0)
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
        unsigned int bigfast1_fbdiv_step : 2;
        unsigned int reserved_0 : 2;
        unsigned int bigfast1_apll_cfg_time : 12;
        unsigned int bigfast1_clkdiv_time : 7;
        unsigned int reserved_1 : 9;
    } reg;
} SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE1_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE1_bigfast1_fbdiv_step_START (0)
#define SOC_PMCTRL_BIGFAST1_GDVFS_PROFILE1_bigfast1_fbdiv_step_END (1)
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
        unsigned int bigfast1_vol_idx_cfg : 8;
        unsigned int bigfast1_vol_chg_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int bigfast1_vol_idx_stat : 8;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_BIGFAST1_VOLIDX_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_VOLIDX_bigfast1_vol_idx_cfg_START (0)
#define SOC_PMCTRL_BIGFAST1_VOLIDX_bigfast1_vol_idx_cfg_END (7)
#define SOC_PMCTRL_BIGFAST1_VOLIDX_bigfast1_vol_chg_sftreq_START (8)
#define SOC_PMCTRL_BIGFAST1_VOLIDX_bigfast1_vol_chg_sftreq_END (8)
#define SOC_PMCTRL_BIGFAST1_VOLIDX_bigfast1_vol_idx_stat_START (12)
#define SOC_PMCTRL_BIGFAST1_VOLIDX_bigfast1_vol_idx_stat_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigfast1_initial_vol_idx : 8;
        unsigned int bigfast1_initialvol_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 20;
    } reg;
} SOC_PMCTRL_BIGFAST1_INITIALVOL_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_INITIALVOL_bigfast1_initial_vol_idx_START (0)
#define SOC_PMCTRL_BIGFAST1_INITIALVOL_bigfast1_initial_vol_idx_END (7)
#define SOC_PMCTRL_BIGFAST1_INITIALVOL_bigfast1_initialvol_sftreq_START (8)
#define SOC_PMCTRL_BIGFAST1_INITIALVOL_bigfast1_initialvol_sftreq_END (8)
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
        unsigned int bigfast1_dvfs_sel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BIGFAST1_DVFS_SEL_UNION;
#endif
#define SOC_PMCTRL_BIGFAST1_DVFS_SEL_bigfast1_dvfs_sel_START (0)
#define SOC_PMCTRL_BIGFAST1_DVFS_SEL_bigfast1_dvfs_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow_unified_power_mode : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BIGSLOW_POWERSUPPLYMODE_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW_POWERSUPPLYMODE_bigslow_unified_power_mode_START (0)
#define SOC_PMCTRL_BIGSLOW_POWERSUPPLYMODE_bigslow_unified_power_mode_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_unified_vol_idx_stat : 8;
        unsigned int bigslow1_unified_vol_idx_stat : 8;
        unsigned int bigslow_unified_pmu_vol_idx_stat : 8;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_STAT_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_STAT_bigslow0_unified_vol_idx_stat_START (0)
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_STAT_bigslow0_unified_vol_idx_stat_END (7)
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_STAT_bigslow1_unified_vol_idx_stat_START (8)
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_STAT_bigslow1_unified_vol_idx_stat_END (15)
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_STAT_bigslow_unified_pmu_vol_idx_stat_START (16)
#define SOC_PMCTRL_BIGSLOW_UNIFIED_VOL_STAT_bigslow_unified_pmu_vol_idx_stat_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_dvfs_stat : 5;
        unsigned int reserved_0 : 3;
        unsigned int bigslow1_dvfs_stat : 5;
        unsigned int reserved_1 : 19;
    } reg;
} SOC_PMCTRL_BIGSLOW_DVFS_STAT_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW_DVFS_STAT_bigslow0_dvfs_stat_START (0)
#define SOC_PMCTRL_BIGSLOW_DVFS_STAT_bigslow0_dvfs_stat_END (4)
#define SOC_PMCTRL_BIGSLOW_DVFS_STAT_bigslow1_dvfs_stat_START (8)
#define SOC_PMCTRL_BIGSLOW_DVFS_STAT_bigslow1_dvfs_stat_END (12)
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
        unsigned int bigslow0_pclkendbg_sel_cfg : 2;
        unsigned int bigslow0_atclken_sel_cfg : 2;
        unsigned int bigslow0_aclkenm_sel_cfg : 3;
        unsigned int bigslow0_atclken_l_sel_cfg : 1;
        unsigned int bigslow0_pclkendbg_sel_stat : 2;
        unsigned int bigslow0_atclken_sel_stat : 2;
        unsigned int bigslow0_aclkenm_sel_stat : 3;
        unsigned int bigslow0_atclken_l_sel_stat : 1;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_BIGSLOW0_CLKDIV_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_bigslow0_pclkendbg_sel_cfg_START (0)
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_bigslow0_pclkendbg_sel_cfg_END (1)
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_bigslow0_atclken_sel_cfg_START (2)
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_bigslow0_atclken_sel_cfg_END (3)
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_bigslow0_aclkenm_sel_cfg_START (4)
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_bigslow0_aclkenm_sel_cfg_END (6)
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_bigslow0_atclken_l_sel_cfg_START (7)
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_bigslow0_atclken_l_sel_cfg_END (7)
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_bigslow0_pclkendbg_sel_stat_START (8)
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_bigslow0_pclkendbg_sel_stat_END (9)
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_bigslow0_atclken_sel_stat_START (10)
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_bigslow0_atclken_sel_stat_END (11)
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_bigslow0_aclkenm_sel_stat_START (12)
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_bigslow0_aclkenm_sel_stat_END (14)
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_bigslow0_atclken_l_sel_stat_START (15)
#define SOC_PMCTRL_BIGSLOW0_CLKDIV_bigslow0_atclken_l_sel_stat_END (15)
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
        unsigned int bigslow0_pclkendbg_sel_prof : 2;
        unsigned int bigslow0_atclken_sel_prof : 2;
        unsigned int bigslow0_aclkenm_sel_prof : 3;
        unsigned int bigslow0_atclken_l_sel_prof : 1;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_BIGSLOW0_CLKPROFILE1_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE1_bigslow0_pclkendbg_sel_prof_START (0)
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE1_bigslow0_pclkendbg_sel_prof_END (1)
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE1_bigslow0_atclken_sel_prof_START (2)
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE1_bigslow0_atclken_sel_prof_END (3)
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE1_bigslow0_aclkenm_sel_prof_START (4)
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE1_bigslow0_aclkenm_sel_prof_END (6)
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE1_bigslow0_atclken_l_sel_prof_START (7)
#define SOC_PMCTRL_BIGSLOW0_CLKPROFILE1_bigslow0_atclken_l_sel_prof_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_vol_mod : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BIGSLOW0_VOLMOD_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_VOLMOD_bigslow0_vol_mod_START (0)
#define SOC_PMCTRL_BIGSLOW0_VOLMOD_bigslow0_vol_mod_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_vol_idx : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_BIGSLOW0_VOLPROFILE_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_VOLPROFILE_bigslow0_vol_idx_START (0)
#define SOC_PMCTRL_BIGSLOW0_VOLPROFILE_bigslow0_vol_idx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_vol_up_step_time : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_PMCTRL_BIGSLOW0_VOLUPSTEPTIME_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_VOLUPSTEPTIME_bigslow0_vol_up_step_time_START (0)
#define SOC_PMCTRL_BIGSLOW0_VOLUPSTEPTIME_bigslow0_vol_up_step_time_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_pmu_up_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_BIGSLOW0_PMU_UP_HOLDTIME_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_PMU_UP_HOLDTIME_bigslow0_pmu_up_vol_hold_time_START (0)
#define SOC_PMCTRL_BIGSLOW0_PMU_UP_HOLDTIME_bigslow0_pmu_up_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_pmu_dn_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_BIGSLOW0_PMU_DN_HOLDTIME_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_PMU_DN_HOLDTIME_bigslow0_pmu_dn_vol_hold_time_START (0)
#define SOC_PMCTRL_BIGSLOW0_PMU_DN_HOLDTIME_bigslow0_pmu_dn_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_pmu_sel : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_BIGSLOW0_PMUSEL_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_PMUSEL_bigslow0_pmu_sel_START (0)
#define SOC_PMCTRL_BIGSLOW0_PMUSEL_bigslow0_pmu_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_gdvfs_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BIGSLOW0_GDVFS_EN_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_GDVFS_EN_bigslow0_gdvfs_en_START (0)
#define SOC_PMCTRL_BIGSLOW0_GDVFS_EN_bigslow0_gdvfs_en_END (0)
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
        unsigned int bigslow0_fbdiv_step : 2;
        unsigned int reserved_0 : 2;
        unsigned int bigslow0_apll_cfg_time : 12;
        unsigned int bigslow0_clkdiv_time : 7;
        unsigned int reserved_1 : 9;
    } reg;
} SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE1_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE1_bigslow0_fbdiv_step_START (0)
#define SOC_PMCTRL_BIGSLOW0_GDVFS_PROFILE1_bigslow0_fbdiv_step_END (1)
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
        unsigned int bigslow0_vol_idx_cfg : 8;
        unsigned int bigslow0_vol_chg_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int bigslow0_vol_idx_stat : 8;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_BIGSLOW0_VOLIDX_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_VOLIDX_bigslow0_vol_idx_cfg_START (0)
#define SOC_PMCTRL_BIGSLOW0_VOLIDX_bigslow0_vol_idx_cfg_END (7)
#define SOC_PMCTRL_BIGSLOW0_VOLIDX_bigslow0_vol_chg_sftreq_START (8)
#define SOC_PMCTRL_BIGSLOW0_VOLIDX_bigslow0_vol_chg_sftreq_END (8)
#define SOC_PMCTRL_BIGSLOW0_VOLIDX_bigslow0_vol_idx_stat_START (12)
#define SOC_PMCTRL_BIGSLOW0_VOLIDX_bigslow0_vol_idx_stat_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow0_initial_vol_idx : 8;
        unsigned int bigslow0_initialvol_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 20;
    } reg;
} SOC_PMCTRL_BIGSLOW0_INITIALVOL_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_INITIALVOL_bigslow0_initial_vol_idx_START (0)
#define SOC_PMCTRL_BIGSLOW0_INITIALVOL_bigslow0_initial_vol_idx_END (7)
#define SOC_PMCTRL_BIGSLOW0_INITIALVOL_bigslow0_initialvol_sftreq_START (8)
#define SOC_PMCTRL_BIGSLOW0_INITIALVOL_bigslow0_initialvol_sftreq_END (8)
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
        unsigned int bigslow0_dvfs_sel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BIGSLOW0_DVFS_SEL_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW0_DVFS_SEL_bigslow0_dvfs_sel_START (0)
#define SOC_PMCTRL_BIGSLOW0_DVFS_SEL_bigslow0_dvfs_sel_END (0)
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
        unsigned int bigslow1_pclkendbg_sel_cfg : 2;
        unsigned int bigslow1_atclken_sel_cfg : 2;
        unsigned int bigslow1_aclkenm_sel_cfg : 3;
        unsigned int bigslow1_atclken_l_sel_cfg : 1;
        unsigned int bigslow1_pclkendbg_sel_stat : 2;
        unsigned int bigslow1_atclken_sel_stat : 2;
        unsigned int bigslow1_aclkenm_sel_stat : 3;
        unsigned int bigslow1_atclken_l_sel_stat : 1;
        unsigned int reserved : 16;
    } reg;
} SOC_PMCTRL_BIGSLOW1_CLKDIV_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_bigslow1_pclkendbg_sel_cfg_START (0)
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_bigslow1_pclkendbg_sel_cfg_END (1)
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_bigslow1_atclken_sel_cfg_START (2)
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_bigslow1_atclken_sel_cfg_END (3)
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_bigslow1_aclkenm_sel_cfg_START (4)
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_bigslow1_aclkenm_sel_cfg_END (6)
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_bigslow1_atclken_l_sel_cfg_START (7)
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_bigslow1_atclken_l_sel_cfg_END (7)
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_bigslow1_pclkendbg_sel_stat_START (8)
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_bigslow1_pclkendbg_sel_stat_END (9)
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_bigslow1_atclken_sel_stat_START (10)
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_bigslow1_atclken_sel_stat_END (11)
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_bigslow1_aclkenm_sel_stat_START (12)
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_bigslow1_aclkenm_sel_stat_END (14)
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_bigslow1_atclken_l_sel_stat_START (15)
#define SOC_PMCTRL_BIGSLOW1_CLKDIV_bigslow1_atclken_l_sel_stat_END (15)
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
        unsigned int bigslow1_pclkendbg_sel_prof : 2;
        unsigned int bigslow1_atclken_sel_prof : 2;
        unsigned int bigslow1_aclkenm_sel_prof : 3;
        unsigned int bigslow1_atclken_l_sel_prof : 1;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_BIGSLOW1_CLKPROFILE1_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE1_bigslow1_pclkendbg_sel_prof_START (0)
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE1_bigslow1_pclkendbg_sel_prof_END (1)
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE1_bigslow1_atclken_sel_prof_START (2)
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE1_bigslow1_atclken_sel_prof_END (3)
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE1_bigslow1_aclkenm_sel_prof_START (4)
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE1_bigslow1_aclkenm_sel_prof_END (6)
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE1_bigslow1_atclken_l_sel_prof_START (7)
#define SOC_PMCTRL_BIGSLOW1_CLKPROFILE1_bigslow1_atclken_l_sel_prof_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_vol_mod : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BIGSLOW1_VOLMOD_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_VOLMOD_bigslow1_vol_mod_START (0)
#define SOC_PMCTRL_BIGSLOW1_VOLMOD_bigslow1_vol_mod_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_vol_idx : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_BIGSLOW1_VOLPROFILE_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_VOLPROFILE_bigslow1_vol_idx_START (0)
#define SOC_PMCTRL_BIGSLOW1_VOLPROFILE_bigslow1_vol_idx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_vol_up_step_time : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_PMCTRL_BIGSLOW1_VOLUPSTEPTIME_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_VOLUPSTEPTIME_bigslow1_vol_up_step_time_START (0)
#define SOC_PMCTRL_BIGSLOW1_VOLUPSTEPTIME_bigslow1_vol_up_step_time_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_pmu_up_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_BIGSLOW1_PMU_UP_HOLDTIME_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_PMU_UP_HOLDTIME_bigslow1_pmu_up_vol_hold_time_START (0)
#define SOC_PMCTRL_BIGSLOW1_PMU_UP_HOLDTIME_bigslow1_pmu_up_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_pmu_dn_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_BIGSLOW1_PMU_DN_HOLDTIME_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_PMU_DN_HOLDTIME_bigslow1_pmu_dn_vol_hold_time_START (0)
#define SOC_PMCTRL_BIGSLOW1_PMU_DN_HOLDTIME_bigslow1_pmu_dn_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_pmu_sel : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_BIGSLOW1_PMUSEL_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_PMUSEL_bigslow1_pmu_sel_START (0)
#define SOC_PMCTRL_BIGSLOW1_PMUSEL_bigslow1_pmu_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_gdvfs_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BIGSLOW1_GDVFS_EN_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_GDVFS_EN_bigslow1_gdvfs_en_START (0)
#define SOC_PMCTRL_BIGSLOW1_GDVFS_EN_bigslow1_gdvfs_en_END (0)
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
        unsigned int bigslow1_fbdiv_step : 2;
        unsigned int reserved_0 : 2;
        unsigned int bigslow1_apll_cfg_time : 12;
        unsigned int bigslow1_clkdiv_time : 7;
        unsigned int reserved_1 : 9;
    } reg;
} SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE1_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE1_bigslow1_fbdiv_step_START (0)
#define SOC_PMCTRL_BIGSLOW1_GDVFS_PROFILE1_bigslow1_fbdiv_step_END (1)
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
        unsigned int bigslow1_vol_idx_cfg : 8;
        unsigned int bigslow1_vol_chg_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int bigslow1_vol_idx_stat : 8;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_PMCTRL_BIGSLOW1_VOLIDX_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_VOLIDX_bigslow1_vol_idx_cfg_START (0)
#define SOC_PMCTRL_BIGSLOW1_VOLIDX_bigslow1_vol_idx_cfg_END (7)
#define SOC_PMCTRL_BIGSLOW1_VOLIDX_bigslow1_vol_chg_sftreq_START (8)
#define SOC_PMCTRL_BIGSLOW1_VOLIDX_bigslow1_vol_chg_sftreq_END (8)
#define SOC_PMCTRL_BIGSLOW1_VOLIDX_bigslow1_vol_idx_stat_START (12)
#define SOC_PMCTRL_BIGSLOW1_VOLIDX_bigslow1_vol_idx_stat_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bigslow1_initial_vol_idx : 8;
        unsigned int bigslow1_initialvol_sftreq : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 20;
    } reg;
} SOC_PMCTRL_BIGSLOW1_INITIALVOL_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_INITIALVOL_bigslow1_initial_vol_idx_START (0)
#define SOC_PMCTRL_BIGSLOW1_INITIALVOL_bigslow1_initial_vol_idx_END (7)
#define SOC_PMCTRL_BIGSLOW1_INITIALVOL_bigslow1_initialvol_sftreq_START (8)
#define SOC_PMCTRL_BIGSLOW1_INITIALVOL_bigslow1_initialvol_sftreq_END (8)
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
        unsigned int bigslow1_dvfs_sel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_BIGSLOW1_DVFS_SEL_UNION;
#endif
#define SOC_PMCTRL_BIGSLOW1_DVFS_SEL_bigslow1_dvfs_sel_START (0)
#define SOC_PMCTRL_BIGSLOW1_DVFS_SEL_bigslow1_dvfs_sel_END (0)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
