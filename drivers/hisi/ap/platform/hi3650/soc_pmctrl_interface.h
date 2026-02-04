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
#define SOC_PMCTRL_PMCSTATUS_ADDR(base) ((base) + (0x00C))
#define SOC_PMCTRL_APLL0CTRL0_ADDR(base) ((base) + (0x010))
#define SOC_PMCTRL_APLL0CTRL1_ADDR(base) ((base) + (0x014))
#define SOC_PMCTRL_APLL1CTRL0_ADDR(base) ((base) + (0x018))
#define SOC_PMCTRL_APLL1CTRL1_ADDR(base) ((base) + (0x01C))
#define SOC_PMCTRL_APLL2CTRL0_ADDR(base) ((base) + (0x020))
#define SOC_PMCTRL_APLL2CTRL1_ADDR(base) ((base) + (0x024))
#define SOC_PMCTRL_PPLL1CTRL0_ADDR(base) ((base) + (0x038))
#define SOC_PMCTRL_PPLL1CTRL1_ADDR(base) ((base) + (0x03C))
#define SOC_PMCTRL_PPLL2CTRL0_ADDR(base) ((base) + (0x040))
#define SOC_PMCTRL_PPLL2CTRL1_ADDR(base) ((base) + (0x044))
#define SOC_PMCTRL_PPLL3CTRL0_ADDR(base) ((base) + (0x048))
#define SOC_PMCTRL_PPLL3CTRL1_ADDR(base) ((base) + (0x04C))
#define SOC_PMCTRL_APLL0SSCCTRL_ADDR(base) ((base) + (0x070))
#define SOC_PMCTRL_APLL1SSCCTRL_ADDR(base) ((base) + (0x074))
#define SOC_PMCTRL_APLL2SSCCTRL_ADDR(base) ((base) + (0x078))
#define SOC_PMCTRL_PPLL1SSCCTRL_ADDR(base) ((base) + (0x084))
#define SOC_PMCTRL_PPLL2SSCCTRL_ADDR(base) ((base) + (0x088))
#define SOC_PMCTRL_PPLL3SSCCTRL_ADDR(base) ((base) + (0x08C))
#define SOC_PMCTRL_PMUMOD_ADDR(base) ((base) + (0x094))
#define SOC_PMCTRL_A53_CPUDVFSMOD_ADDR(base) ((base) + (0x098))
#define SOC_PMCTRL_A53_CPUCLKSEL_ADDR(base) ((base) + (0x09C))
#define SOC_PMCTRL_A53_CPUDVFSEN_ADDR(base) ((base) + (0x0A0))
#define SOC_PMCTRL_A53_CPUPLLSEL_ADDR(base) ((base) + (0x0A4))
#define SOC_PMCTRL_A53_CPUCLKDIV_ADDR(base) ((base) + (0x0A8))
#define SOC_PMCTRL_A53_CPUDVFSSTART_ADDR(base) ((base) + (0x0AC))
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_ADDR(base) ((base) + (0x0B0))
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_1_ADDR(base) ((base) + (0x0B4))
#define SOC_PMCTRL_A53_CPUCLKPROFILE1_ADDR(base) ((base) + (0x0B8))
#define SOC_PMCTRL_A53_CPUVOLMOD_ADDR(base) ((base) + (0x0BC))
#define SOC_PMCTRL_A53_CPUVOLPROFILE_ADDR(base) ((base) + (0x0C0))
#define SOC_PMCTRL_A53_CPUPHASEMOD_ADDR(base) ((base) + (0xC8))
#define SOC_PMCTRL_A53_CPUPHASEPROFILE_ADDR(base) ((base) + (0x0CC))
#define SOC_PMCTRL_A53_CPUVOLPHASESEL_ADDR(base) ((base) + (0x0D0))
#define SOC_PMCTRL_A53_VOLUPSTEPTIME_ADDR(base) ((base) + (0x0D4))
#define SOC_PMCTRL_A53_VOLDNSTEPTIME_ADDR(base) ((base) + (0x0D8))
#define SOC_PMCTRL_A53_PHASEUPTIME_ADDR(base) ((base) + (0x0DC))
#define SOC_PMCTRL_A53_PHASEDNTIME_ADDR(base) ((base) + (0x0E0))
#define SOC_PMCTRL_A53_PMUHOLDTIME_ADDR(base) ((base) + (0x0E4))
#define SOC_PMCTRL_A53_PMUSEL_ADDR(base) ((base) + (0x0E8))
#define SOC_PMCTRL_A53_CPUAVSEN_ADDR(base) ((base) + (0x0F0))
#define SOC_PMCTRL_A53_CPUAVSPARAMOD_ADDR(base) ((base) + (0x0F4))
#define SOC_PMCTRL_A53_CPUAVSPARA0_ADDR(base) ((base) + (0x0F8))
#define SOC_PMCTRL_A53_CPUAVSPARA1_ADDR(base) ((base) + (0x0FC))
#define SOC_PMCTRL_A53_CPUAVSPARA2_ADDR(base) ((base) + (0x100))
#define SOC_PMCTRL_A53_CPUAVSPARA3_ADDR(base) ((base) + (0x104))
#define SOC_PMCTRL_A53_CPUAVSPARA4_ADDR(base) ((base) + (0x108))
#define SOC_PMCTRL_A53_CPUAVSPARA5_ADDR(base) ((base) + (0x10C))
#define SOC_PMCTRL_A53_CPUAVSPARA6_ADDR(base) ((base) + (0x110))
#define SOC_PMCTRL_A53_CPUHPMTYP_ADDR(base) ((base) + (0x114))
#define SOC_PMCTRL_A53_CPU01HPMEN_ADDR(base) ((base) + (0x118))
#define SOC_PMCTRL_A53_CPU01HPMXEN_ADDR(base) ((base) + (0x11C))
#define SOC_PMCTRL_A53_CPU01HPMOPCVALID_ADDR(base) ((base) + (0x120))
#define SOC_PMCTRL_A53_CPU01HPMOPC_ADDR(base) ((base) + (0x124))
#define SOC_PMCTRL_A53_CPU01HPMXOPC_ADDR(base) ((base) + (0x128))
#define SOC_PMCTRL_A53_CPU01HPMPC_ADDR(base) ((base) + (0x12C))
#define SOC_PMCTRL_A53_CPU23HPMEN_ADDR(base) ((base) + (0x134))
#define SOC_PMCTRL_A53_CPU23HPMXEN_ADDR(base) ((base) + (0x138))
#define SOC_PMCTRL_A53_CPU23HPMOPCVALID_ADDR(base) ((base) + (0x13C))
#define SOC_PMCTRL_A53_CPU23HPMOPC_ADDR(base) ((base) + (0x140))
#define SOC_PMCTRL_A53_CPU23HPMXOPC_ADDR(base) ((base) + (0x144))
#define SOC_PMCTRL_A53_CPU23HPMPC_ADDR(base) ((base) + (0x148))
#define SOC_PMCTRL_A53_CPUHPMCLKDIV_ADDR(base) ((base) + (0x150))
#define SOC_PMCTRL_A53_CPUAVSVOLIDX_ADDR(base) ((base) + (0x154))
#define SOC_PMCTRL_A57_CPUDVFSMOD_ADDR(base) ((base) + (0x158))
#define SOC_PMCTRL_A57_CPUCLKSEL_ADDR(base) ((base) + (0x15C))
#define SOC_PMCTRL_A57_CPUDVFSEN_ADDR(base) ((base) + (0x160))
#define SOC_PMCTRL_A57_CPUPLLSEL_ADDR(base) ((base) + (0x164))
#define SOC_PMCTRL_A57_CPUCLKDIV_ADDR(base) ((base) + (0x168))
#define SOC_PMCTRL_A57_CPUDVFSSTART_ADDR(base) ((base) + (0x16C))
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_ADDR(base) ((base) + (0x170))
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_1_ADDR(base) ((base) + (0x174))
#define SOC_PMCTRL_A57_CPUCLKPROFILE1_ADDR(base) ((base) + (0x178))
#define SOC_PMCTRL_A57_CPUVOLMOD_ADDR(base) ((base) + (0x17C))
#define SOC_PMCTRL_A57_CPUVOLPROFILE_ADDR(base) ((base) + (0x180))
#define SOC_PMCTRL_A57_CPUPHASEMOD_ADDR(base) ((base) + (0x188))
#define SOC_PMCTRL_A57_CPUPHASEPROFILE_ADDR(base) ((base) + (0x18C))
#define SOC_PMCTRL_A57_CPUVOLPHASESEL_ADDR(base) ((base) + (0x190))
#define SOC_PMCTRL_A57_VOLUPSTEPTIME_ADDR(base) ((base) + (0x194))
#define SOC_PMCTRL_A57_VOLDNSTEPTIME_ADDR(base) ((base) + (0x198))
#define SOC_PMCTRL_A57_PHASEUPTIME_ADDR(base) ((base) + (0x19C))
#define SOC_PMCTRL_A57_PHASEDNTIME_ADDR(base) ((base) + (0x1A0))
#define SOC_PMCTRL_A57_PMUHOLDTIME_ADDR(base) ((base) + (0x1A4))
#define SOC_PMCTRL_A57_PMUSEL_ADDR(base) ((base) + (0x1A8))
#define SOC_PMCTRL_A57_CPUAVSEN_ADDR(base) ((base) + (0x1B0))
#define SOC_PMCTRL_A57_CPUAVSPARAMOD_ADDR(base) ((base) + (0x1B4))
#define SOC_PMCTRL_A57_CPUAVSPARA0_ADDR(base) ((base) + (0x1B8))
#define SOC_PMCTRL_A57_CPUAVSPARA1_ADDR(base) ((base) + (0x1BC))
#define SOC_PMCTRL_A57_CPUAVSPARA2_ADDR(base) ((base) + (0x1C0))
#define SOC_PMCTRL_A57_CPUAVSPARA3_ADDR(base) ((base) + (0x1C4))
#define SOC_PMCTRL_A57_CPUAVSPARA4_ADDR(base) ((base) + (0x1C8))
#define SOC_PMCTRL_A57_CPUAVSPARA5_ADDR(base) ((base) + (0x1CC))
#define SOC_PMCTRL_A57_CPUAVSPARA6_ADDR(base) ((base) + (0x1D0))
#define SOC_PMCTRL_A57_CPUHPMTYP_ADDR(base) ((base) + (0x1D4))
#define SOC_PMCTRL_A57_CPU01HPMEN_ADDR(base) ((base) + (0x1D8))
#define SOC_PMCTRL_A57_CPU01HPMXEN_ADDR(base) ((base) + (0x1DC))
#define SOC_PMCTRL_A57_CPU01HPMOPCVALID_ADDR(base) ((base) + (0x1E0))
#define SOC_PMCTRL_A57_CPU01HPMOPC_ADDR(base) ((base) + (0x1E4))
#define SOC_PMCTRL_A57_CPU01HPMXOPC_ADDR(base) ((base) + (0x1E8))
#define SOC_PMCTRL_A57_CPU01HPMPC_ADDR(base) ((base) + (0x1EC))
#define SOC_PMCTRL_A57_CPU23HPMEN_ADDR(base) ((base) + (0x1F4))
#define SOC_PMCTRL_A57_CPU23HPMXEN_ADDR(base) ((base) + (0x1F8))
#define SOC_PMCTRL_A57_CPU23HPMOPCVALID_ADDR(base) ((base) + (0x1FC))
#define SOC_PMCTRL_A57_CPU23HPMOPC_ADDR(base) ((base) + (0x200))
#define SOC_PMCTRL_A57_CPU23HPMXOPC_ADDR(base) ((base) + (0x204))
#define SOC_PMCTRL_A57_CPU23HPMPC_ADDR(base) ((base) + (0x208))
#define SOC_PMCTRL_A57_CPUHPMCLKDIV_ADDR(base) ((base) + (0x210))
#define SOC_PMCTRL_A57_CPUAVSVOLIDX_ADDR(base) ((base) + (0x214))
#define SOC_PMCTRL_G3DCLKSEL_ADDR(base) ((base) + (0x220))
#define SOC_PMCTRL_G3DCLKDIV_ADDR(base) ((base) + (0x224))
#define SOC_PMCTRL_G3DDVFSEN_ADDR(base) ((base) + (0x228))
#define SOC_PMCTRL_G3DDVFSSTART_ADDR(base) ((base) + (0x22C))
#define SOC_PMCTRL_G3DCLKPROFILE_ADDR(base) ((base) + (0x230))
#define SOC_PMCTRL_G3DVOLMOD_ADDR(base) ((base) + (0x234))
#define SOC_PMCTRL_G3DVOLPROFILE_ADDR(base) ((base) + (0x238))
#define SOC_PMCTRL_G3DPHASEMOD_ADDR(base) ((base) + (0x240))
#define SOC_PMCTRL_G3DPHASEPROFILE_ADDR(base) ((base) + (0x244))
#define SOC_PMCTRL_G3DVOLPHASESEL_ADDR(base) ((base) + (0x248))
#define SOC_PMCTRL_G3DPMUSEL_ADDR(base) ((base) + (0x24C))
#define SOC_PMCTRL_G3DVOLUPSTEPTIME_ADDR(base) ((base) + (0x250))
#define SOC_PMCTRL_G3DVOLDNSTEPTIME_ADDR(base) ((base) + (0x254))
#define SOC_PMCTRL_G3DPHASEUPTIME_ADDR(base) ((base) + (0x258))
#define SOC_PMCTRL_G3DPHASEDNTIME_ADDR(base) ((base) + (0x25C))
#define SOC_PMCTRL_G3DPMUHOLDTIME_ADDR(base) ((base) + (0x260))
#define SOC_PMCTRL_G3DHPMBYPASS_ADDR(base) ((base) + (0x264))
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_ADDR(base) ((base) + (0x268))
#define SOC_PMCTRL_G3DAVSEN_ADDR(base) ((base) + (0x270))
#define SOC_PMCTRL_G3DAVSPARAMOD_ADDR(base) ((base) + (0x274))
#define SOC_PMCTRL_G3DAVSPARA0_ADDR(base) ((base) + (0x278))
#define SOC_PMCTRL_G3DAVSPARA1_ADDR(base) ((base) + (0x27C))
#define SOC_PMCTRL_G3DAVSPARA2_ADDR(base) ((base) + (0x280))
#define SOC_PMCTRL_G3DAVSPARA3_ADDR(base) ((base) + (0x284))
#define SOC_PMCTRL_G3DAVSPARA4_ADDR(base) ((base) + (0x288))
#define SOC_PMCTRL_G3DAVSPARA5_ADDR(base) ((base) + (0x28C))
#define SOC_PMCTRL_G3DAVSPARA6_ADDR(base) ((base) + (0x290))
#define SOC_PMCTRL_G3DHPMTYP_ADDR(base) ((base) + (0x294))
#define SOC_PMCTRL_G3DHPMEN_ADDR(base) ((base) + (0x298))
#define SOC_PMCTRL_G3DHPMXEN_ADDR(base) ((base) + (0x29C))
#define SOC_PMCTRL_G3DHPMOPCVALID_ADDR(base) ((base) + (0x2A0))
#define SOC_PMCTRL_G3DHPMOPC_ADDR(base) ((base) + (0x2A4))
#define SOC_PMCTRL_G3DHPMXOPC_ADDR(base) ((base) + (0x2A8))
#define SOC_PMCTRL_G3DHPMPC_ADDR(base) ((base) + (0x2AC))
#define SOC_PMCTRL_G3DHPMEN1_ADDR(base) ((base) + (0x2B4))
#define SOC_PMCTRL_G3DHPMXEN1_ADDR(base) ((base) + (0x2B8))
#define SOC_PMCTRL_G3DHPMOPCVALID1_ADDR(base) ((base) + (0x2BC))
#define SOC_PMCTRL_G3DHPMOPC1_ADDR(base) ((base) + (0x2C0))
#define SOC_PMCTRL_G3DHPMXOPC1_ADDR(base) ((base) + (0x2C4))
#define SOC_PMCTRL_G3DHPMPC1_ADDR(base) ((base) + (0x2C8))
#define SOC_PMCTRL_G3DHPMMASKSTAT_ADDR(base) ((base) + (0x2E8))
#define SOC_PMCTRL_G3DHPMCLKDIV_ADDR(base) ((base) + (0x2EC))
#define SOC_PMCTRL_G3DAVSVOLIDX_ADDR(base) ((base) + (0x2F0))
#define SOC_PMCTRL_DDRLPCTRL_ADDR(base) ((base) + (0x30C))
#define SOC_PMCTRL_PLLLOCKTIME_ADDR(base) ((base) + (0x320))
#define SOC_PMCTRL_PLLLOCKMOD_ADDR(base) ((base) + (0x324))
#define SOC_PMCTRL_AVSRUNROUND_ADDR(base) ((base) + (0x32C))
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
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_ADDR(base) ((base) + (0x380))
#define SOC_PMCTRL_NOC_POWER_IDLEACK_ADDR(base) ((base) + (0x384))
#define SOC_PMCTRL_NOC_POWER_IDLE_ADDR(base) ((base) + (0x388))
#define SOC_PMCTRL_PERI_INT0_MASK_ADDR(base) ((base) + (0x3A0))
#define SOC_PMCTRL_PERI_INT0_STAT_ADDR(base) ((base) + (0x3A4))
#define SOC_PMCTRL_PERI_INT1_MASK_ADDR(base) ((base) + (0x3A8))
#define SOC_PMCTRL_PERI_INT1_STAT_ADDR(base) ((base) + (0x3AC))
#define SOC_PMCTRL_PERI_INT2_MASK_ADDR(base) ((base) + (0x3B0))
#define SOC_PMCTRL_PERI_INT2_STAT_ADDR(base) ((base) + (0x3B4))
#define SOC_PMCTRL_PERI_INT3_MASK_ADDR(base) ((base) + (0x3B8))
#define SOC_PMCTRL_PERI_INT3_STAT_ADDR(base) ((base) + (0x3BC))
#define SOC_PMCTRL_VSENSOR_CTRL_EN_1_ADDR(base) ((base) + (0x3D0))
#define SOC_PMCTRL_VSENSOR_CTRL_BYPASS_1_ADDR(base) ((base) + (0x3D4))
#define SOC_PMCTRL_VSENSOR_CTRL_1_ADDR(base) ((base) + (0x3D8))
#define SOC_PMCTRL_VSENSOR_CNT_1_ADDR(base) ((base) + (0x3DC))
#define SOC_PMCTRL_VSENSOR_REF_CODE_1_ADDR(base) ((base) + (0x3E0))
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_ADDR(base) ((base) + (0x3E4))
#define SOC_PMCTRL_VSENSOR_CODE_1_ADDR(base) ((base) + (0x3E8))
#define SOC_PMCTRL_VSENSOR_INTSTAT_1_ADDR(base) ((base) + (0x3EC))
#define SOC_PMCTRL_VSENSOR_D_RATE_1_ADDR(base) ((base) + (0x3F0))
#define SOC_PMCTRL_VSENSOR_CTRL_EN_0_ADDR(base) ((base) + (0x3F4))
#define SOC_PMCTRL_VSENSOR_CTRL_BYPASS_0_ADDR(base) ((base) + (0x3F8))
#define SOC_PMCTRL_VSENSOR_CTRL_0_ADDR(base) ((base) + (0x3FC))
#define SOC_PMCTRL_VSENSOR_CNT_0_ADDR(base) ((base) + (0x400))
#define SOC_PMCTRL_VSENSOR_REF_CODE_0_ADDR(base) ((base) + (0x404))
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_ADDR(base) ((base) + (0x408))
#define SOC_PMCTRL_VSENSOR_CODE_0_ADDR(base) ((base) + (0x40C))
#define SOC_PMCTRL_VSENSOR_INTSTAT_0_ADDR(base) ((base) + (0x410))
#define SOC_PMCTRL_VSENSOR_D_RATE_0_ADDR(base) ((base) + (0x414))
#define SOC_PMCTRL_PERIHPMEN_ADDR(base) ((base) + (0x430))
#define SOC_PMCTRL_PERIHPMXEN_ADDR(base) ((base) + (0x434))
#define SOC_PMCTRL_PERIHPMOPCVALID_ADDR(base) ((base) + (0x438))
#define SOC_PMCTRL_PERIHPMOPC_ADDR(base) ((base) + (0x43C))
#define SOC_PMCTRL_PERIHPMCLKDIV_ADDR(base) ((base) + (0x440))
#define SOC_PMCTRL_BOOTROMFLAG_ADDR(base) ((base) + (0x460))
#define SOC_PMCTRL_VSENSOR_CTRL_EN_2_ADDR(base) ((base) + (0x464))
#define SOC_PMCTRL_VSENSOR_CTRL_BYPASS_2_ADDR(base) ((base) + (0x468))
#define SOC_PMCTRL_VSENSOR_CTRL_2_ADDR(base) ((base) + (0x46C))
#define SOC_PMCTRL_VSENSOR_CNT_2_ADDR(base) ((base) + (0x470))
#define SOC_PMCTRL_VSENSOR_REF_CODE_2_ADDR(base) ((base) + (0x474))
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_ADDR(base) ((base) + (0x478))
#define SOC_PMCTRL_VSENSOR_CODE_2_ADDR(base) ((base) + (0x47C))
#define SOC_PMCTRL_VSENSOR_INTSTAT_2_ADDR(base) ((base) + (0x480))
#define SOC_PMCTRL_VSENSOR_D_RATE_2_ADDR(base) ((base) + (0x484))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_a53_cpu_dvfs_en : 1;
        unsigned int int_a53_cpu_avs_ok_en : 1;
        unsigned int int_a53_cpu_avs_up_err_en : 1;
        unsigned int int_a53_cpu_avs_dn_err_en : 1;
        unsigned int int_a53_cpu_avs_pc_vld_en : 1;
        unsigned int int_a53_cpu_avs_idle_en : 1;
        unsigned int int_a57_cpu_dvfs_en : 1;
        unsigned int int_a57_cpu_avs_ok_en : 1;
        unsigned int int_a57_cpu_avs_up_err_en : 1;
        unsigned int int_a57_cpu_avs_dn_err_en : 1;
        unsigned int int_a57_cpu_avs_pc_vld_en : 1;
        unsigned int int_a57_cpu_avs_idle_en : 1;
        unsigned int int_g3d_dvfs_en : 1;
        unsigned int int_g3d_avs_ok_en : 1;
        unsigned int int_g3d_avs_up_err_en : 1;
        unsigned int int_g3d_avs_dn_err_en : 1;
        unsigned int int_g3d_avs_pc_vld_en : 1;
        unsigned int int_g3d_avs_idle_en : 1;
        unsigned int reserved : 14;
    } reg;
} SOC_PMCTRL_PMCINTEN_UNION;
#endif
#define SOC_PMCTRL_PMCINTEN_int_a53_cpu_dvfs_en_START (0)
#define SOC_PMCTRL_PMCINTEN_int_a53_cpu_dvfs_en_END (0)
#define SOC_PMCTRL_PMCINTEN_int_a53_cpu_avs_ok_en_START (1)
#define SOC_PMCTRL_PMCINTEN_int_a53_cpu_avs_ok_en_END (1)
#define SOC_PMCTRL_PMCINTEN_int_a53_cpu_avs_up_err_en_START (2)
#define SOC_PMCTRL_PMCINTEN_int_a53_cpu_avs_up_err_en_END (2)
#define SOC_PMCTRL_PMCINTEN_int_a53_cpu_avs_dn_err_en_START (3)
#define SOC_PMCTRL_PMCINTEN_int_a53_cpu_avs_dn_err_en_END (3)
#define SOC_PMCTRL_PMCINTEN_int_a53_cpu_avs_pc_vld_en_START (4)
#define SOC_PMCTRL_PMCINTEN_int_a53_cpu_avs_pc_vld_en_END (4)
#define SOC_PMCTRL_PMCINTEN_int_a53_cpu_avs_idle_en_START (5)
#define SOC_PMCTRL_PMCINTEN_int_a53_cpu_avs_idle_en_END (5)
#define SOC_PMCTRL_PMCINTEN_int_a57_cpu_dvfs_en_START (6)
#define SOC_PMCTRL_PMCINTEN_int_a57_cpu_dvfs_en_END (6)
#define SOC_PMCTRL_PMCINTEN_int_a57_cpu_avs_ok_en_START (7)
#define SOC_PMCTRL_PMCINTEN_int_a57_cpu_avs_ok_en_END (7)
#define SOC_PMCTRL_PMCINTEN_int_a57_cpu_avs_up_err_en_START (8)
#define SOC_PMCTRL_PMCINTEN_int_a57_cpu_avs_up_err_en_END (8)
#define SOC_PMCTRL_PMCINTEN_int_a57_cpu_avs_dn_err_en_START (9)
#define SOC_PMCTRL_PMCINTEN_int_a57_cpu_avs_dn_err_en_END (9)
#define SOC_PMCTRL_PMCINTEN_int_a57_cpu_avs_pc_vld_en_START (10)
#define SOC_PMCTRL_PMCINTEN_int_a57_cpu_avs_pc_vld_en_END (10)
#define SOC_PMCTRL_PMCINTEN_int_a57_cpu_avs_idle_en_START (11)
#define SOC_PMCTRL_PMCINTEN_int_a57_cpu_avs_idle_en_END (11)
#define SOC_PMCTRL_PMCINTEN_int_g3d_dvfs_en_START (12)
#define SOC_PMCTRL_PMCINTEN_int_g3d_dvfs_en_END (12)
#define SOC_PMCTRL_PMCINTEN_int_g3d_avs_ok_en_START (13)
#define SOC_PMCTRL_PMCINTEN_int_g3d_avs_ok_en_END (13)
#define SOC_PMCTRL_PMCINTEN_int_g3d_avs_up_err_en_START (14)
#define SOC_PMCTRL_PMCINTEN_int_g3d_avs_up_err_en_END (14)
#define SOC_PMCTRL_PMCINTEN_int_g3d_avs_dn_err_en_START (15)
#define SOC_PMCTRL_PMCINTEN_int_g3d_avs_dn_err_en_END (15)
#define SOC_PMCTRL_PMCINTEN_int_g3d_avs_pc_vld_en_START (16)
#define SOC_PMCTRL_PMCINTEN_int_g3d_avs_pc_vld_en_END (16)
#define SOC_PMCTRL_PMCINTEN_int_g3d_avs_idle_en_START (17)
#define SOC_PMCTRL_PMCINTEN_int_g3d_avs_idle_en_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_a53_cpu_dvfs_stat : 1;
        unsigned int int_a53_cpu_avs_ok_stat : 1;
        unsigned int int_a53_cpu_avs_up_err_stat : 1;
        unsigned int int_a53_cpu_avs_dn_err_stat : 1;
        unsigned int int_a53_cpu_avs_pc_vld_stat : 1;
        unsigned int int_a53_cpu_avs_idle_stat : 1;
        unsigned int int_a57_cpu_dvfs_stat : 1;
        unsigned int int_a57_cpu_avs_ok_stat : 1;
        unsigned int int_a57_cpu_avs_up_err_stat : 1;
        unsigned int int_a57_cpu_avs_dn_err_stat : 1;
        unsigned int int_a57_cpu_avs_pc_vld_stat : 1;
        unsigned int int_a57_cpu_avs_idle_stat : 1;
        unsigned int int_g3d_dvfs_stat : 1;
        unsigned int int_g3d_avs_ok_stat : 1;
        unsigned int int_g3d_avs_up_err_stat : 1;
        unsigned int int_g3d_avs_dn_err_stat : 1;
        unsigned int int_g3d_avs_pc_vld_stat : 1;
        unsigned int int_g3d_avs_idle_stat : 1;
        unsigned int reserved : 14;
    } reg;
} SOC_PMCTRL_PMCINTSTAT_UNION;
#endif
#define SOC_PMCTRL_PMCINTSTAT_int_a53_cpu_dvfs_stat_START (0)
#define SOC_PMCTRL_PMCINTSTAT_int_a53_cpu_dvfs_stat_END (0)
#define SOC_PMCTRL_PMCINTSTAT_int_a53_cpu_avs_ok_stat_START (1)
#define SOC_PMCTRL_PMCINTSTAT_int_a53_cpu_avs_ok_stat_END (1)
#define SOC_PMCTRL_PMCINTSTAT_int_a53_cpu_avs_up_err_stat_START (2)
#define SOC_PMCTRL_PMCINTSTAT_int_a53_cpu_avs_up_err_stat_END (2)
#define SOC_PMCTRL_PMCINTSTAT_int_a53_cpu_avs_dn_err_stat_START (3)
#define SOC_PMCTRL_PMCINTSTAT_int_a53_cpu_avs_dn_err_stat_END (3)
#define SOC_PMCTRL_PMCINTSTAT_int_a53_cpu_avs_pc_vld_stat_START (4)
#define SOC_PMCTRL_PMCINTSTAT_int_a53_cpu_avs_pc_vld_stat_END (4)
#define SOC_PMCTRL_PMCINTSTAT_int_a53_cpu_avs_idle_stat_START (5)
#define SOC_PMCTRL_PMCINTSTAT_int_a53_cpu_avs_idle_stat_END (5)
#define SOC_PMCTRL_PMCINTSTAT_int_a57_cpu_dvfs_stat_START (6)
#define SOC_PMCTRL_PMCINTSTAT_int_a57_cpu_dvfs_stat_END (6)
#define SOC_PMCTRL_PMCINTSTAT_int_a57_cpu_avs_ok_stat_START (7)
#define SOC_PMCTRL_PMCINTSTAT_int_a57_cpu_avs_ok_stat_END (7)
#define SOC_PMCTRL_PMCINTSTAT_int_a57_cpu_avs_up_err_stat_START (8)
#define SOC_PMCTRL_PMCINTSTAT_int_a57_cpu_avs_up_err_stat_END (8)
#define SOC_PMCTRL_PMCINTSTAT_int_a57_cpu_avs_dn_err_stat_START (9)
#define SOC_PMCTRL_PMCINTSTAT_int_a57_cpu_avs_dn_err_stat_END (9)
#define SOC_PMCTRL_PMCINTSTAT_int_a57_cpu_avs_pc_vld_stat_START (10)
#define SOC_PMCTRL_PMCINTSTAT_int_a57_cpu_avs_pc_vld_stat_END (10)
#define SOC_PMCTRL_PMCINTSTAT_int_a57_cpu_avs_idle_stat_START (11)
#define SOC_PMCTRL_PMCINTSTAT_int_a57_cpu_avs_idle_stat_END (11)
#define SOC_PMCTRL_PMCINTSTAT_int_g3d_dvfs_stat_START (12)
#define SOC_PMCTRL_PMCINTSTAT_int_g3d_dvfs_stat_END (12)
#define SOC_PMCTRL_PMCINTSTAT_int_g3d_avs_ok_stat_START (13)
#define SOC_PMCTRL_PMCINTSTAT_int_g3d_avs_ok_stat_END (13)
#define SOC_PMCTRL_PMCINTSTAT_int_g3d_avs_up_err_stat_START (14)
#define SOC_PMCTRL_PMCINTSTAT_int_g3d_avs_up_err_stat_END (14)
#define SOC_PMCTRL_PMCINTSTAT_int_g3d_avs_dn_err_stat_START (15)
#define SOC_PMCTRL_PMCINTSTAT_int_g3d_avs_dn_err_stat_END (15)
#define SOC_PMCTRL_PMCINTSTAT_int_g3d_avs_pc_vld_stat_START (16)
#define SOC_PMCTRL_PMCINTSTAT_int_g3d_avs_pc_vld_stat_END (16)
#define SOC_PMCTRL_PMCINTSTAT_int_g3d_avs_idle_stat_START (17)
#define SOC_PMCTRL_PMCINTSTAT_int_g3d_avs_idle_stat_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_a53_cpu_dvfs_clr : 1;
        unsigned int int_a53_cpu_avs_ok_clr : 1;
        unsigned int int_a53_cpu_avs_up_err_clr : 1;
        unsigned int int_a53_cpu_avs_dn_err_clr : 1;
        unsigned int int_a53_cpu_avs_pc_vld_clr : 1;
        unsigned int int_a53_cpu_avs_idle_clr : 1;
        unsigned int int_a57_cpu_dvfs_clr : 1;
        unsigned int int_a57_cpu_avs_ok_clr : 1;
        unsigned int int_a57_cpu_avs_up_err_clr : 1;
        unsigned int int_a57_cpu_avs_dn_err_clr : 1;
        unsigned int int_a57_cpu_avs_pc_vld_clr : 1;
        unsigned int int_a57_cpu_avs_idle_clr : 1;
        unsigned int int_g3d_dvfs_clr : 1;
        unsigned int int_g3d_avs_ok_clr : 1;
        unsigned int int_g3d_avs_up_err_clr : 1;
        unsigned int int_g3d_avs_dn_err_clr : 1;
        unsigned int int_g3d_avs_pc_vld_clr : 1;
        unsigned int int_g3d_avs_idle_clr : 1;
        unsigned int reserved : 14;
    } reg;
} SOC_PMCTRL_PMCINTCLR_UNION;
#endif
#define SOC_PMCTRL_PMCINTCLR_int_a53_cpu_dvfs_clr_START (0)
#define SOC_PMCTRL_PMCINTCLR_int_a53_cpu_dvfs_clr_END (0)
#define SOC_PMCTRL_PMCINTCLR_int_a53_cpu_avs_ok_clr_START (1)
#define SOC_PMCTRL_PMCINTCLR_int_a53_cpu_avs_ok_clr_END (1)
#define SOC_PMCTRL_PMCINTCLR_int_a53_cpu_avs_up_err_clr_START (2)
#define SOC_PMCTRL_PMCINTCLR_int_a53_cpu_avs_up_err_clr_END (2)
#define SOC_PMCTRL_PMCINTCLR_int_a53_cpu_avs_dn_err_clr_START (3)
#define SOC_PMCTRL_PMCINTCLR_int_a53_cpu_avs_dn_err_clr_END (3)
#define SOC_PMCTRL_PMCINTCLR_int_a53_cpu_avs_pc_vld_clr_START (4)
#define SOC_PMCTRL_PMCINTCLR_int_a53_cpu_avs_pc_vld_clr_END (4)
#define SOC_PMCTRL_PMCINTCLR_int_a53_cpu_avs_idle_clr_START (5)
#define SOC_PMCTRL_PMCINTCLR_int_a53_cpu_avs_idle_clr_END (5)
#define SOC_PMCTRL_PMCINTCLR_int_a57_cpu_dvfs_clr_START (6)
#define SOC_PMCTRL_PMCINTCLR_int_a57_cpu_dvfs_clr_END (6)
#define SOC_PMCTRL_PMCINTCLR_int_a57_cpu_avs_ok_clr_START (7)
#define SOC_PMCTRL_PMCINTCLR_int_a57_cpu_avs_ok_clr_END (7)
#define SOC_PMCTRL_PMCINTCLR_int_a57_cpu_avs_up_err_clr_START (8)
#define SOC_PMCTRL_PMCINTCLR_int_a57_cpu_avs_up_err_clr_END (8)
#define SOC_PMCTRL_PMCINTCLR_int_a57_cpu_avs_dn_err_clr_START (9)
#define SOC_PMCTRL_PMCINTCLR_int_a57_cpu_avs_dn_err_clr_END (9)
#define SOC_PMCTRL_PMCINTCLR_int_a57_cpu_avs_pc_vld_clr_START (10)
#define SOC_PMCTRL_PMCINTCLR_int_a57_cpu_avs_pc_vld_clr_END (10)
#define SOC_PMCTRL_PMCINTCLR_int_a57_cpu_avs_idle_clr_START (11)
#define SOC_PMCTRL_PMCINTCLR_int_a57_cpu_avs_idle_clr_END (11)
#define SOC_PMCTRL_PMCINTCLR_int_g3d_dvfs_clr_START (12)
#define SOC_PMCTRL_PMCINTCLR_int_g3d_dvfs_clr_END (12)
#define SOC_PMCTRL_PMCINTCLR_int_g3d_avs_ok_clr_START (13)
#define SOC_PMCTRL_PMCINTCLR_int_g3d_avs_ok_clr_END (13)
#define SOC_PMCTRL_PMCINTCLR_int_g3d_avs_up_err_clr_START (14)
#define SOC_PMCTRL_PMCINTCLR_int_g3d_avs_up_err_clr_END (14)
#define SOC_PMCTRL_PMCINTCLR_int_g3d_avs_dn_err_clr_START (15)
#define SOC_PMCTRL_PMCINTCLR_int_g3d_avs_dn_err_clr_END (15)
#define SOC_PMCTRL_PMCINTCLR_int_g3d_avs_pc_vld_clr_START (16)
#define SOC_PMCTRL_PMCINTCLR_int_g3d_avs_pc_vld_clr_END (16)
#define SOC_PMCTRL_PMCINTCLR_int_g3d_avs_idle_clr_START (17)
#define SOC_PMCTRL_PMCINTCLR_int_g3d_avs_idle_clr_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_dvfs_stat : 4;
        unsigned int a53_cpu_avs_stat : 6;
        unsigned int a57_cpu_dvfs_stat : 4;
        unsigned int a57_cpu_avs_stat : 6;
        unsigned int g3d_dvfs_stat : 4;
        unsigned int g3d_avs_stat : 6;
        unsigned int reserved : 2;
    } reg;
} SOC_PMCTRL_PMCSTATUS_UNION;
#endif
#define SOC_PMCTRL_PMCSTATUS_a53_cpu_dvfs_stat_START (0)
#define SOC_PMCTRL_PMCSTATUS_a53_cpu_dvfs_stat_END (3)
#define SOC_PMCTRL_PMCSTATUS_a53_cpu_avs_stat_START (4)
#define SOC_PMCTRL_PMCSTATUS_a53_cpu_avs_stat_END (9)
#define SOC_PMCTRL_PMCSTATUS_a57_cpu_dvfs_stat_START (10)
#define SOC_PMCTRL_PMCSTATUS_a57_cpu_dvfs_stat_END (13)
#define SOC_PMCTRL_PMCSTATUS_a57_cpu_avs_stat_START (14)
#define SOC_PMCTRL_PMCSTATUS_a57_cpu_avs_stat_END (19)
#define SOC_PMCTRL_PMCSTATUS_g3d_dvfs_stat_START (20)
#define SOC_PMCTRL_PMCSTATUS_g3d_dvfs_stat_END (23)
#define SOC_PMCTRL_PMCSTATUS_g3d_avs_stat_START (24)
#define SOC_PMCTRL_PMCSTATUS_g3d_avs_stat_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll0_en : 1;
        unsigned int apll0_bp : 1;
        unsigned int apll0_refdiv : 6;
        unsigned int apll0_fbdiv : 12;
        unsigned int apll0_postdiv1 : 3;
        unsigned int apll0_postdiv2 : 3;
        unsigned int apll0_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL0CTRL0_UNION;
#endif
#define SOC_PMCTRL_APLL0CTRL0_apll0_en_START (0)
#define SOC_PMCTRL_APLL0CTRL0_apll0_en_END (0)
#define SOC_PMCTRL_APLL0CTRL0_apll0_bp_START (1)
#define SOC_PMCTRL_APLL0CTRL0_apll0_bp_END (1)
#define SOC_PMCTRL_APLL0CTRL0_apll0_refdiv_START (2)
#define SOC_PMCTRL_APLL0CTRL0_apll0_refdiv_END (7)
#define SOC_PMCTRL_APLL0CTRL0_apll0_fbdiv_START (8)
#define SOC_PMCTRL_APLL0CTRL0_apll0_fbdiv_END (19)
#define SOC_PMCTRL_APLL0CTRL0_apll0_postdiv1_START (20)
#define SOC_PMCTRL_APLL0CTRL0_apll0_postdiv1_END (22)
#define SOC_PMCTRL_APLL0CTRL0_apll0_postdiv2_START (23)
#define SOC_PMCTRL_APLL0CTRL0_apll0_postdiv2_END (25)
#define SOC_PMCTRL_APLL0CTRL0_apll0_lock_START (26)
#define SOC_PMCTRL_APLL0CTRL0_apll0_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll0_fracdiv : 24;
        unsigned int apll0_int_mod : 1;
        unsigned int apll0_cfg_vld : 1;
        unsigned int gt_clk_apll0 : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL0CTRL1_UNION;
#endif
#define SOC_PMCTRL_APLL0CTRL1_apll0_fracdiv_START (0)
#define SOC_PMCTRL_APLL0CTRL1_apll0_fracdiv_END (23)
#define SOC_PMCTRL_APLL0CTRL1_apll0_int_mod_START (24)
#define SOC_PMCTRL_APLL0CTRL1_apll0_int_mod_END (24)
#define SOC_PMCTRL_APLL0CTRL1_apll0_cfg_vld_START (25)
#define SOC_PMCTRL_APLL0CTRL1_apll0_cfg_vld_END (25)
#define SOC_PMCTRL_APLL0CTRL1_gt_clk_apll0_START (26)
#define SOC_PMCTRL_APLL0CTRL1_gt_clk_apll0_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll1_en : 1;
        unsigned int apll1_bp : 1;
        unsigned int apll1_refdiv : 6;
        unsigned int apll1_fbdiv : 12;
        unsigned int apll1_postdiv1 : 3;
        unsigned int apll1_postdiv2 : 3;
        unsigned int apll1_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL1CTRL0_UNION;
#endif
#define SOC_PMCTRL_APLL1CTRL0_apll1_en_START (0)
#define SOC_PMCTRL_APLL1CTRL0_apll1_en_END (0)
#define SOC_PMCTRL_APLL1CTRL0_apll1_bp_START (1)
#define SOC_PMCTRL_APLL1CTRL0_apll1_bp_END (1)
#define SOC_PMCTRL_APLL1CTRL0_apll1_refdiv_START (2)
#define SOC_PMCTRL_APLL1CTRL0_apll1_refdiv_END (7)
#define SOC_PMCTRL_APLL1CTRL0_apll1_fbdiv_START (8)
#define SOC_PMCTRL_APLL1CTRL0_apll1_fbdiv_END (19)
#define SOC_PMCTRL_APLL1CTRL0_apll1_postdiv1_START (20)
#define SOC_PMCTRL_APLL1CTRL0_apll1_postdiv1_END (22)
#define SOC_PMCTRL_APLL1CTRL0_apll1_postdiv2_START (23)
#define SOC_PMCTRL_APLL1CTRL0_apll1_postdiv2_END (25)
#define SOC_PMCTRL_APLL1CTRL0_apll1_lock_START (26)
#define SOC_PMCTRL_APLL1CTRL0_apll1_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll1_fracdiv : 24;
        unsigned int apll1_int_mod : 1;
        unsigned int apll1_cfg_vld : 1;
        unsigned int gt_clk_apll1 : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL1CTRL1_UNION;
#endif
#define SOC_PMCTRL_APLL1CTRL1_apll1_fracdiv_START (0)
#define SOC_PMCTRL_APLL1CTRL1_apll1_fracdiv_END (23)
#define SOC_PMCTRL_APLL1CTRL1_apll1_int_mod_START (24)
#define SOC_PMCTRL_APLL1CTRL1_apll1_int_mod_END (24)
#define SOC_PMCTRL_APLL1CTRL1_apll1_cfg_vld_START (25)
#define SOC_PMCTRL_APLL1CTRL1_apll1_cfg_vld_END (25)
#define SOC_PMCTRL_APLL1CTRL1_gt_clk_apll1_START (26)
#define SOC_PMCTRL_APLL1CTRL1_gt_clk_apll1_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll2_en : 1;
        unsigned int apll2_bp : 1;
        unsigned int apll2_refdiv : 6;
        unsigned int apll2_fbdiv : 12;
        unsigned int apll2_postdiv1 : 3;
        unsigned int apll2_postdiv2 : 3;
        unsigned int apll2_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL2CTRL0_UNION;
#endif
#define SOC_PMCTRL_APLL2CTRL0_apll2_en_START (0)
#define SOC_PMCTRL_APLL2CTRL0_apll2_en_END (0)
#define SOC_PMCTRL_APLL2CTRL0_apll2_bp_START (1)
#define SOC_PMCTRL_APLL2CTRL0_apll2_bp_END (1)
#define SOC_PMCTRL_APLL2CTRL0_apll2_refdiv_START (2)
#define SOC_PMCTRL_APLL2CTRL0_apll2_refdiv_END (7)
#define SOC_PMCTRL_APLL2CTRL0_apll2_fbdiv_START (8)
#define SOC_PMCTRL_APLL2CTRL0_apll2_fbdiv_END (19)
#define SOC_PMCTRL_APLL2CTRL0_apll2_postdiv1_START (20)
#define SOC_PMCTRL_APLL2CTRL0_apll2_postdiv1_END (22)
#define SOC_PMCTRL_APLL2CTRL0_apll2_postdiv2_START (23)
#define SOC_PMCTRL_APLL2CTRL0_apll2_postdiv2_END (25)
#define SOC_PMCTRL_APLL2CTRL0_apll2_lock_START (26)
#define SOC_PMCTRL_APLL2CTRL0_apll2_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apll2_fracdiv : 24;
        unsigned int apll2_int_mod : 1;
        unsigned int apll2_cfg_vld : 1;
        unsigned int gt_clk_apll2 : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_PMCTRL_APLL2CTRL1_UNION;
#endif
#define SOC_PMCTRL_APLL2CTRL1_apll2_fracdiv_START (0)
#define SOC_PMCTRL_APLL2CTRL1_apll2_fracdiv_END (23)
#define SOC_PMCTRL_APLL2CTRL1_apll2_int_mod_START (24)
#define SOC_PMCTRL_APLL2CTRL1_apll2_int_mod_END (24)
#define SOC_PMCTRL_APLL2CTRL1_apll2_cfg_vld_START (25)
#define SOC_PMCTRL_APLL2CTRL1_apll2_cfg_vld_END (25)
#define SOC_PMCTRL_APLL2CTRL1_gt_clk_apll2_START (26)
#define SOC_PMCTRL_APLL2CTRL1_gt_clk_apll2_END (26)
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
        unsigned int pmu_mod : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_PMUMOD_UNION;
#endif
#define SOC_PMCTRL_PMUMOD_pmu_mod_START (0)
#define SOC_PMCTRL_PMUMOD_pmu_mod_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_dvfs_mod : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_A53_CPUDVFSMOD_UNION;
#endif
#define SOC_PMCTRL_A53_CPUDVFSMOD_a53_cpu_dvfs_mod_START (0)
#define SOC_PMCTRL_A53_CPUDVFSMOD_a53_cpu_dvfs_mod_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_clk_sel : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_A53_CPUCLKSEL_UNION;
#endif
#define SOC_PMCTRL_A53_CPUCLKSEL_a53_clk_sel_START (0)
#define SOC_PMCTRL_A53_CPUCLKSEL_a53_clk_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_dvfs_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_A53_CPUDVFSEN_UNION;
#endif
#define SOC_PMCTRL_A53_CPUDVFSEN_a53_cpu_dvfs_en_START (0)
#define SOC_PMCTRL_A53_CPUDVFSEN_a53_cpu_dvfs_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_apll_sel : 2;
        unsigned int a53_pll_sw_ack : 5;
        unsigned int reserved : 25;
    } reg;
} SOC_PMCTRL_A53_CPUPLLSEL_UNION;
#endif
#define SOC_PMCTRL_A53_CPUPLLSEL_a53_apll_sel_START (0)
#define SOC_PMCTRL_A53_CPUPLLSEL_a53_apll_sel_END (1)
#define SOC_PMCTRL_A53_CPUPLLSEL_a53_pll_sw_ack_START (2)
#define SOC_PMCTRL_A53_CPUPLLSEL_a53_pll_sw_ack_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_pclkendbg_sel : 2;
        unsigned int a53_atclken_sel : 2;
        unsigned int a53_aclkenm_sel : 3;
        unsigned int a53_atclken_l_sel : 1;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_A53_CPUCLKDIV_UNION;
#endif
#define SOC_PMCTRL_A53_CPUCLKDIV_a53_pclkendbg_sel_START (0)
#define SOC_PMCTRL_A53_CPUCLKDIV_a53_pclkendbg_sel_END (1)
#define SOC_PMCTRL_A53_CPUCLKDIV_a53_atclken_sel_START (2)
#define SOC_PMCTRL_A53_CPUCLKDIV_a53_atclken_sel_END (3)
#define SOC_PMCTRL_A53_CPUCLKDIV_a53_aclkenm_sel_START (4)
#define SOC_PMCTRL_A53_CPUCLKDIV_a53_aclkenm_sel_END (6)
#define SOC_PMCTRL_A53_CPUCLKDIV_a53_atclken_l_sel_START (7)
#define SOC_PMCTRL_A53_CPUCLKDIV_a53_atclken_l_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_profile_updn : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_A53_CPUDVFSSTART_UNION;
#endif
#define SOC_PMCTRL_A53_CPUDVFSSTART_a53_cpu_profile_updn_START (0)
#define SOC_PMCTRL_A53_CPUDVFSSTART_a53_cpu_profile_updn_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_apll_refdiv : 6;
        unsigned int a53_apll_fbdiv : 12;
        unsigned int a53_apll_postdiv1 : 3;
        unsigned int a53_apll_postdiv2 : 3;
        unsigned int a53_apll_sw_target : 2;
        unsigned int reserved : 6;
    } reg;
} SOC_PMCTRL_A53_CPUCLKPROFILE0_UNION;
#endif
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_a53_apll_refdiv_START (0)
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_a53_apll_refdiv_END (5)
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_a53_apll_fbdiv_START (6)
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_a53_apll_fbdiv_END (17)
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_a53_apll_postdiv1_START (18)
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_a53_apll_postdiv1_END (20)
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_a53_apll_postdiv2_START (21)
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_a53_apll_postdiv2_END (23)
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_a53_apll_sw_target_START (24)
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_a53_apll_sw_target_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_apll_fracdiv : 24;
        unsigned int a53_apll_int_mod : 1;
        unsigned int a53_apll_cfg_vld : 1;
        unsigned int reserved : 6;
    } reg;
} SOC_PMCTRL_A53_CPUCLKPROFILE0_1_UNION;
#endif
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_1_a53_apll_fracdiv_START (0)
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_1_a53_apll_fracdiv_END (23)
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_1_a53_apll_int_mod_START (24)
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_1_a53_apll_int_mod_END (24)
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_1_a53_apll_cfg_vld_START (25)
#define SOC_PMCTRL_A53_CPUCLKPROFILE0_1_a53_apll_cfg_vld_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_pclkendbg_prof : 2;
        unsigned int a53_atclken_sel_prof : 2;
        unsigned int a53_aclkenm_sel_prof : 3;
        unsigned int a53_atclken_l_prof : 1;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_A53_CPUCLKPROFILE1_UNION;
#endif
#define SOC_PMCTRL_A53_CPUCLKPROFILE1_a53_pclkendbg_prof_START (0)
#define SOC_PMCTRL_A53_CPUCLKPROFILE1_a53_pclkendbg_prof_END (1)
#define SOC_PMCTRL_A53_CPUCLKPROFILE1_a53_atclken_sel_prof_START (2)
#define SOC_PMCTRL_A53_CPUCLKPROFILE1_a53_atclken_sel_prof_END (3)
#define SOC_PMCTRL_A53_CPUCLKPROFILE1_a53_aclkenm_sel_prof_START (4)
#define SOC_PMCTRL_A53_CPUCLKPROFILE1_a53_aclkenm_sel_prof_END (6)
#define SOC_PMCTRL_A53_CPUCLKPROFILE1_a53_atclken_l_prof_START (7)
#define SOC_PMCTRL_A53_CPUCLKPROFILE1_a53_atclken_l_prof_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_vol_mod : 1;
        unsigned int a53_cpu_clk_off_vol_mod : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_A53_CPUVOLMOD_UNION;
#endif
#define SOC_PMCTRL_A53_CPUVOLMOD_a53_cpu_vol_mod_START (0)
#define SOC_PMCTRL_A53_CPUVOLMOD_a53_cpu_vol_mod_END (0)
#define SOC_PMCTRL_A53_CPUVOLMOD_a53_cpu_clk_off_vol_mod_START (1)
#define SOC_PMCTRL_A53_CPUVOLMOD_a53_cpu_clk_off_vol_mod_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_vol_idx : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_A53_CPUVOLPROFILE_UNION;
#endif
#define SOC_PMCTRL_A53_CPUVOLPROFILE_a53_cpu_vol_idx_START (0)
#define SOC_PMCTRL_A53_CPUVOLPROFILE_a53_cpu_vol_idx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_phase_mod : 1;
        unsigned int a53_cpu_clk_off_phase_mod : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_A53_CPUPHASEMOD_UNION;
#endif
#define SOC_PMCTRL_A53_CPUPHASEMOD_a53_cpu_phase_mod_START (0)
#define SOC_PMCTRL_A53_CPUPHASEMOD_a53_cpu_phase_mod_END (0)
#define SOC_PMCTRL_A53_CPUPHASEMOD_a53_cpu_clk_off_phase_mod_START (1)
#define SOC_PMCTRL_A53_CPUPHASEMOD_a53_cpu_clk_off_phase_mod_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_phase_idx : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_A53_CPUPHASEPROFILE_UNION;
#endif
#define SOC_PMCTRL_A53_CPUPHASEPROFILE_a53_cpu_phase_idx_START (0)
#define SOC_PMCTRL_A53_CPUPHASEPROFILE_a53_cpu_phase_idx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_vol_phase_sel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_A53_CPUVOLPHASESEL_UNION;
#endif
#define SOC_PMCTRL_A53_CPUVOLPHASESEL_a53_cpu_vol_phase_sel_START (0)
#define SOC_PMCTRL_A53_CPUVOLPHASESEL_a53_cpu_vol_phase_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_vol_up_step_time : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_PMCTRL_A53_VOLUPSTEPTIME_UNION;
#endif
#define SOC_PMCTRL_A53_VOLUPSTEPTIME_a53_vol_up_step_time_START (0)
#define SOC_PMCTRL_A53_VOLUPSTEPTIME_a53_vol_up_step_time_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_vol_dn_step_time : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_PMCTRL_A53_VOLDNSTEPTIME_UNION;
#endif
#define SOC_PMCTRL_A53_VOLDNSTEPTIME_a53_vol_dn_step_time_START (0)
#define SOC_PMCTRL_A53_VOLDNSTEPTIME_a53_vol_dn_step_time_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_dvfs_phase_up_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_A53_PHASEUPTIME_UNION;
#endif
#define SOC_PMCTRL_A53_PHASEUPTIME_a53_dvfs_phase_up_time_START (0)
#define SOC_PMCTRL_A53_PHASEUPTIME_a53_dvfs_phase_up_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_dvfs_phase_dn_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_A53_PHASEDNTIME_UNION;
#endif
#define SOC_PMCTRL_A53_PHASEDNTIME_a53_dvfs_phase_dn_time_START (0)
#define SOC_PMCTRL_A53_PHASEDNTIME_a53_dvfs_phase_dn_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_pmu_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_A53_PMUHOLDTIME_UNION;
#endif
#define SOC_PMCTRL_A53_PMUHOLDTIME_a53_pmu_vol_hold_time_START (0)
#define SOC_PMCTRL_A53_PMUHOLDTIME_a53_pmu_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_pmu_sel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_A53_PMUSEL_UNION;
#endif
#define SOC_PMCTRL_A53_PMUSEL_a53_pmu_sel_START (0)
#define SOC_PMCTRL_A53_PMUSEL_a53_pmu_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_avs_en : 1;
        unsigned int a53_cpu_avs_pwctrl_en : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_A53_CPUAVSEN_UNION;
#endif
#define SOC_PMCTRL_A53_CPUAVSEN_a53_cpu_avs_en_START (0)
#define SOC_PMCTRL_A53_CPUAVSEN_a53_cpu_avs_en_END (0)
#define SOC_PMCTRL_A53_CPUAVSEN_a53_cpu_avs_pwctrl_en_START (1)
#define SOC_PMCTRL_A53_CPUAVSEN_a53_cpu_avs_pwctrl_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_avspara_mod : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_A53_CPUAVSPARAMOD_UNION;
#endif
#define SOC_PMCTRL_A53_CPUAVSPARAMOD_a53_cpu_avspara_mod_START (0)
#define SOC_PMCTRL_A53_CPUAVSPARAMOD_a53_cpu_avspara_mod_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 2;
        unsigned int a53_cpu_avs_opc_offset : 10;
        unsigned int a53_cpu_avs_rcc : 5;
        unsigned int a53_cpu_avs_irgap : 5;
        unsigned int a53_cpu_avs_opc_mod : 2;
        unsigned int a53_cpu_avs_hpm_sel : 4;
        unsigned int a53_cpu_avs_opc_shift : 3;
        unsigned int reserved_1 : 1;
    } reg;
} SOC_PMCTRL_A53_CPUAVSPARA0_UNION;
#endif
#define SOC_PMCTRL_A53_CPUAVSPARA0_a53_cpu_avs_opc_offset_START (2)
#define SOC_PMCTRL_A53_CPUAVSPARA0_a53_cpu_avs_opc_offset_END (11)
#define SOC_PMCTRL_A53_CPUAVSPARA0_a53_cpu_avs_rcc_START (12)
#define SOC_PMCTRL_A53_CPUAVSPARA0_a53_cpu_avs_rcc_END (16)
#define SOC_PMCTRL_A53_CPUAVSPARA0_a53_cpu_avs_irgap_START (17)
#define SOC_PMCTRL_A53_CPUAVSPARA0_a53_cpu_avs_irgap_END (21)
#define SOC_PMCTRL_A53_CPUAVSPARA0_a53_cpu_avs_opc_mod_START (22)
#define SOC_PMCTRL_A53_CPUAVSPARA0_a53_cpu_avs_opc_mod_END (23)
#define SOC_PMCTRL_A53_CPUAVSPARA0_a53_cpu_avs_hpm_sel_START (24)
#define SOC_PMCTRL_A53_CPUAVSPARA0_a53_cpu_avs_hpm_sel_END (27)
#define SOC_PMCTRL_A53_CPUAVSPARA0_a53_cpu_avs_opc_shift_START (28)
#define SOC_PMCTRL_A53_CPUAVSPARA0_a53_cpu_avs_opc_shift_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_avs_vol_up_gain1 : 4;
        unsigned int a53_cpu_avs_vol_up_gain2 : 4;
        unsigned int a53_cpu_avs_vol_up_gain3 : 4;
        unsigned int a53_cpu_avs_vol_up_gain4 : 4;
        unsigned int a53_cpu_avs_vol_up_gain5 : 4;
        unsigned int a53_cpu_avs_vol_up_gain6 : 4;
        unsigned int a53_cpu_avs_vol_up_gain7 : 4;
        unsigned int a53_cpu_avs_vol_up_gain8 : 4;
    } reg;
} SOC_PMCTRL_A53_CPUAVSPARA1_UNION;
#endif
#define SOC_PMCTRL_A53_CPUAVSPARA1_a53_cpu_avs_vol_up_gain1_START (0)
#define SOC_PMCTRL_A53_CPUAVSPARA1_a53_cpu_avs_vol_up_gain1_END (3)
#define SOC_PMCTRL_A53_CPUAVSPARA1_a53_cpu_avs_vol_up_gain2_START (4)
#define SOC_PMCTRL_A53_CPUAVSPARA1_a53_cpu_avs_vol_up_gain2_END (7)
#define SOC_PMCTRL_A53_CPUAVSPARA1_a53_cpu_avs_vol_up_gain3_START (8)
#define SOC_PMCTRL_A53_CPUAVSPARA1_a53_cpu_avs_vol_up_gain3_END (11)
#define SOC_PMCTRL_A53_CPUAVSPARA1_a53_cpu_avs_vol_up_gain4_START (12)
#define SOC_PMCTRL_A53_CPUAVSPARA1_a53_cpu_avs_vol_up_gain4_END (15)
#define SOC_PMCTRL_A53_CPUAVSPARA1_a53_cpu_avs_vol_up_gain5_START (16)
#define SOC_PMCTRL_A53_CPUAVSPARA1_a53_cpu_avs_vol_up_gain5_END (19)
#define SOC_PMCTRL_A53_CPUAVSPARA1_a53_cpu_avs_vol_up_gain6_START (20)
#define SOC_PMCTRL_A53_CPUAVSPARA1_a53_cpu_avs_vol_up_gain6_END (23)
#define SOC_PMCTRL_A53_CPUAVSPARA1_a53_cpu_avs_vol_up_gain7_START (24)
#define SOC_PMCTRL_A53_CPUAVSPARA1_a53_cpu_avs_vol_up_gain7_END (27)
#define SOC_PMCTRL_A53_CPUAVSPARA1_a53_cpu_avs_vol_up_gain8_START (28)
#define SOC_PMCTRL_A53_CPUAVSPARA1_a53_cpu_avs_vol_up_gain8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_avs_vol_up_gain9 : 4;
        unsigned int a53_cpu_avs_vol_up_gain10 : 4;
        unsigned int a53_cpu_avs_vol_up_gain11 : 4;
        unsigned int a53_cpu_avs_vol_up_gain12 : 4;
        unsigned int a53_cpu_avs_vol_up_gain13 : 4;
        unsigned int a53_cpu_avs_vol_up_gain14 : 4;
        unsigned int a53_cpu_avs_vol_up_gain15 : 7;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_A53_CPUAVSPARA2_UNION;
#endif
#define SOC_PMCTRL_A53_CPUAVSPARA2_a53_cpu_avs_vol_up_gain9_START (0)
#define SOC_PMCTRL_A53_CPUAVSPARA2_a53_cpu_avs_vol_up_gain9_END (3)
#define SOC_PMCTRL_A53_CPUAVSPARA2_a53_cpu_avs_vol_up_gain10_START (4)
#define SOC_PMCTRL_A53_CPUAVSPARA2_a53_cpu_avs_vol_up_gain10_END (7)
#define SOC_PMCTRL_A53_CPUAVSPARA2_a53_cpu_avs_vol_up_gain11_START (8)
#define SOC_PMCTRL_A53_CPUAVSPARA2_a53_cpu_avs_vol_up_gain11_END (11)
#define SOC_PMCTRL_A53_CPUAVSPARA2_a53_cpu_avs_vol_up_gain12_START (12)
#define SOC_PMCTRL_A53_CPUAVSPARA2_a53_cpu_avs_vol_up_gain12_END (15)
#define SOC_PMCTRL_A53_CPUAVSPARA2_a53_cpu_avs_vol_up_gain13_START (16)
#define SOC_PMCTRL_A53_CPUAVSPARA2_a53_cpu_avs_vol_up_gain13_END (19)
#define SOC_PMCTRL_A53_CPUAVSPARA2_a53_cpu_avs_vol_up_gain14_START (20)
#define SOC_PMCTRL_A53_CPUAVSPARA2_a53_cpu_avs_vol_up_gain14_END (23)
#define SOC_PMCTRL_A53_CPUAVSPARA2_a53_cpu_avs_vol_up_gain15_START (24)
#define SOC_PMCTRL_A53_CPUAVSPARA2_a53_cpu_avs_vol_up_gain15_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_avs_vol_dn_gain1 : 4;
        unsigned int a53_cpu_avs_vol_dn_gain2 : 4;
        unsigned int a53_cpu_avs_vol_dn_gain3 : 4;
        unsigned int a53_cpu_avs_vol_dn_gain4 : 4;
        unsigned int a53_cpu_avs_vol_dn_gain5 : 4;
        unsigned int a53_cpu_avs_vol_dn_gain6 : 4;
        unsigned int a53_cpu_avs_vol_dn_gain7 : 4;
        unsigned int a53_cpu_avs_vol_dn_gain8 : 4;
    } reg;
} SOC_PMCTRL_A53_CPUAVSPARA3_UNION;
#endif
#define SOC_PMCTRL_A53_CPUAVSPARA3_a53_cpu_avs_vol_dn_gain1_START (0)
#define SOC_PMCTRL_A53_CPUAVSPARA3_a53_cpu_avs_vol_dn_gain1_END (3)
#define SOC_PMCTRL_A53_CPUAVSPARA3_a53_cpu_avs_vol_dn_gain2_START (4)
#define SOC_PMCTRL_A53_CPUAVSPARA3_a53_cpu_avs_vol_dn_gain2_END (7)
#define SOC_PMCTRL_A53_CPUAVSPARA3_a53_cpu_avs_vol_dn_gain3_START (8)
#define SOC_PMCTRL_A53_CPUAVSPARA3_a53_cpu_avs_vol_dn_gain3_END (11)
#define SOC_PMCTRL_A53_CPUAVSPARA3_a53_cpu_avs_vol_dn_gain4_START (12)
#define SOC_PMCTRL_A53_CPUAVSPARA3_a53_cpu_avs_vol_dn_gain4_END (15)
#define SOC_PMCTRL_A53_CPUAVSPARA3_a53_cpu_avs_vol_dn_gain5_START (16)
#define SOC_PMCTRL_A53_CPUAVSPARA3_a53_cpu_avs_vol_dn_gain5_END (19)
#define SOC_PMCTRL_A53_CPUAVSPARA3_a53_cpu_avs_vol_dn_gain6_START (20)
#define SOC_PMCTRL_A53_CPUAVSPARA3_a53_cpu_avs_vol_dn_gain6_END (23)
#define SOC_PMCTRL_A53_CPUAVSPARA3_a53_cpu_avs_vol_dn_gain7_START (24)
#define SOC_PMCTRL_A53_CPUAVSPARA3_a53_cpu_avs_vol_dn_gain7_END (27)
#define SOC_PMCTRL_A53_CPUAVSPARA3_a53_cpu_avs_vol_dn_gain8_START (28)
#define SOC_PMCTRL_A53_CPUAVSPARA3_a53_cpu_avs_vol_dn_gain8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_avs_vol_dn_gain9 : 4;
        unsigned int a53_cpu_avs_vol_dn_gain10 : 4;
        unsigned int a53_cpu_avs_vol_dn_gain11 : 4;
        unsigned int a53_cpu_avs_vol_dn_gain12 : 4;
        unsigned int a53_cpu_avs_vol_dn_gain13 : 4;
        unsigned int a53_cpu_avs_vol_dn_gain14 : 4;
        unsigned int a53_cpu_avs_vol_dn_gain15 : 7;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_A53_CPUAVSPARA4_UNION;
#endif
#define SOC_PMCTRL_A53_CPUAVSPARA4_a53_cpu_avs_vol_dn_gain9_START (0)
#define SOC_PMCTRL_A53_CPUAVSPARA4_a53_cpu_avs_vol_dn_gain9_END (3)
#define SOC_PMCTRL_A53_CPUAVSPARA4_a53_cpu_avs_vol_dn_gain10_START (4)
#define SOC_PMCTRL_A53_CPUAVSPARA4_a53_cpu_avs_vol_dn_gain10_END (7)
#define SOC_PMCTRL_A53_CPUAVSPARA4_a53_cpu_avs_vol_dn_gain11_START (8)
#define SOC_PMCTRL_A53_CPUAVSPARA4_a53_cpu_avs_vol_dn_gain11_END (11)
#define SOC_PMCTRL_A53_CPUAVSPARA4_a53_cpu_avs_vol_dn_gain12_START (12)
#define SOC_PMCTRL_A53_CPUAVSPARA4_a53_cpu_avs_vol_dn_gain12_END (15)
#define SOC_PMCTRL_A53_CPUAVSPARA4_a53_cpu_avs_vol_dn_gain13_START (16)
#define SOC_PMCTRL_A53_CPUAVSPARA4_a53_cpu_avs_vol_dn_gain13_END (19)
#define SOC_PMCTRL_A53_CPUAVSPARA4_a53_cpu_avs_vol_dn_gain14_START (20)
#define SOC_PMCTRL_A53_CPUAVSPARA4_a53_cpu_avs_vol_dn_gain14_END (23)
#define SOC_PMCTRL_A53_CPUAVSPARA4_a53_cpu_avs_vol_dn_gain15_START (24)
#define SOC_PMCTRL_A53_CPUAVSPARA4_a53_cpu_avs_vol_dn_gain15_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_avs_max_vol : 7;
        unsigned int a53_cpu_avs_min_vol : 7;
        unsigned int reserved : 18;
    } reg;
} SOC_PMCTRL_A53_CPUAVSPARA5_UNION;
#endif
#define SOC_PMCTRL_A53_CPUAVSPARA5_a53_cpu_avs_max_vol_START (0)
#define SOC_PMCTRL_A53_CPUAVSPARA5_a53_cpu_avs_max_vol_END (6)
#define SOC_PMCTRL_A53_CPUAVSPARA5_a53_cpu_avs_min_vol_START (7)
#define SOC_PMCTRL_A53_CPUAVSPARA5_a53_cpu_avs_min_vol_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_avs_sample_num : 22;
        unsigned int reserved : 10;
    } reg;
} SOC_PMCTRL_A53_CPUAVSPARA6_UNION;
#endif
#define SOC_PMCTRL_A53_CPUAVSPARA6_a53_cpu_avs_sample_num_START (0)
#define SOC_PMCTRL_A53_CPUAVSPARA6_a53_cpu_avs_sample_num_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_hpm_type : 1;
        unsigned int a53_cpu0_hpm_mask : 1;
        unsigned int a53_cpu1_hpm_mask : 1;
        unsigned int a53_cpu2_hpm_mask : 1;
        unsigned int a53_cpu3_hpm_mask : 1;
        unsigned int a53_scu0_hpm_mask : 1;
        unsigned int a53_cpu4_hpm_mask : 1;
        unsigned int a53_cpu5_hpm_mask : 1;
        unsigned int a53_cpu6_hpm_mask : 1;
        unsigned int a53_cpu7_hpm_mask : 1;
        unsigned int a53_scu1_hpm_mask : 1;
        unsigned int reserved : 5;
        unsigned int biten : 16;
    } reg;
} SOC_PMCTRL_A53_CPUHPMTYP_UNION;
#endif
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu_hpm_type_START (0)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu_hpm_type_END (0)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu0_hpm_mask_START (1)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu0_hpm_mask_END (1)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu1_hpm_mask_START (2)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu1_hpm_mask_END (2)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu2_hpm_mask_START (3)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu2_hpm_mask_END (3)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu3_hpm_mask_START (4)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu3_hpm_mask_END (4)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_scu0_hpm_mask_START (5)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_scu0_hpm_mask_END (5)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu4_hpm_mask_START (6)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu4_hpm_mask_END (6)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu5_hpm_mask_START (7)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu5_hpm_mask_END (7)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu6_hpm_mask_START (8)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu6_hpm_mask_END (8)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu7_hpm_mask_START (9)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_cpu7_hpm_mask_END (9)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_scu1_hpm_mask_START (10)
#define SOC_PMCTRL_A53_CPUHPMTYP_a53_scu1_hpm_mask_END (10)
#define SOC_PMCTRL_A53_CPUHPMTYP_biten_START (16)
#define SOC_PMCTRL_A53_CPUHPMTYP_biten_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu0_hpm_en : 1;
        unsigned int a53_cpu1_hpm_en : 1;
        unsigned int a53_scu_hpm_en : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_PMCTRL_A53_CPU01HPMEN_UNION;
#endif
#define SOC_PMCTRL_A53_CPU01HPMEN_a53_cpu0_hpm_en_START (0)
#define SOC_PMCTRL_A53_CPU01HPMEN_a53_cpu0_hpm_en_END (0)
#define SOC_PMCTRL_A53_CPU01HPMEN_a53_cpu1_hpm_en_START (1)
#define SOC_PMCTRL_A53_CPU01HPMEN_a53_cpu1_hpm_en_END (1)
#define SOC_PMCTRL_A53_CPU01HPMEN_a53_scu_hpm_en_START (2)
#define SOC_PMCTRL_A53_CPU01HPMEN_a53_scu_hpm_en_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu0_hpmx_en : 1;
        unsigned int a53_cpu1_hpmx_en : 1;
        unsigned int a53_scu_hpmx_en : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_PMCTRL_A53_CPU01HPMXEN_UNION;
#endif
#define SOC_PMCTRL_A53_CPU01HPMXEN_a53_cpu0_hpmx_en_START (0)
#define SOC_PMCTRL_A53_CPU01HPMXEN_a53_cpu0_hpmx_en_END (0)
#define SOC_PMCTRL_A53_CPU01HPMXEN_a53_cpu1_hpmx_en_START (1)
#define SOC_PMCTRL_A53_CPU01HPMXEN_a53_cpu1_hpmx_en_END (1)
#define SOC_PMCTRL_A53_CPU01HPMXEN_a53_scu_hpmx_en_START (2)
#define SOC_PMCTRL_A53_CPU01HPMXEN_a53_scu_hpmx_en_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu0_hpm_opc_vld : 1;
        unsigned int a53_cpu1_hpm_opc_vld : 1;
        unsigned int a53_scu_hpm_opc_vld : 1;
        unsigned int a53_cpu0_hpmx_opc_vld : 1;
        unsigned int a53_cpu1_hpmx_opc_vld : 1;
        unsigned int a53_scu_hpmx_opc_vld : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_PMCTRL_A53_CPU01HPMOPCVALID_UNION;
#endif
#define SOC_PMCTRL_A53_CPU01HPMOPCVALID_a53_cpu0_hpm_opc_vld_START (0)
#define SOC_PMCTRL_A53_CPU01HPMOPCVALID_a53_cpu0_hpm_opc_vld_END (0)
#define SOC_PMCTRL_A53_CPU01HPMOPCVALID_a53_cpu1_hpm_opc_vld_START (1)
#define SOC_PMCTRL_A53_CPU01HPMOPCVALID_a53_cpu1_hpm_opc_vld_END (1)
#define SOC_PMCTRL_A53_CPU01HPMOPCVALID_a53_scu_hpm_opc_vld_START (2)
#define SOC_PMCTRL_A53_CPU01HPMOPCVALID_a53_scu_hpm_opc_vld_END (2)
#define SOC_PMCTRL_A53_CPU01HPMOPCVALID_a53_cpu0_hpmx_opc_vld_START (3)
#define SOC_PMCTRL_A53_CPU01HPMOPCVALID_a53_cpu0_hpmx_opc_vld_END (3)
#define SOC_PMCTRL_A53_CPU01HPMOPCVALID_a53_cpu1_hpmx_opc_vld_START (4)
#define SOC_PMCTRL_A53_CPU01HPMOPCVALID_a53_cpu1_hpmx_opc_vld_END (4)
#define SOC_PMCTRL_A53_CPU01HPMOPCVALID_a53_scu_hpmx_opc_vld_START (5)
#define SOC_PMCTRL_A53_CPU01HPMOPCVALID_a53_scu_hpmx_opc_vld_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu0_hpm_opc : 10;
        unsigned int a53_cpu1_hpm_opc : 10;
        unsigned int a53_scu_hpm_opc : 10;
        unsigned int reserved : 2;
    } reg;
} SOC_PMCTRL_A53_CPU01HPMOPC_UNION;
#endif
#define SOC_PMCTRL_A53_CPU01HPMOPC_a53_cpu0_hpm_opc_START (0)
#define SOC_PMCTRL_A53_CPU01HPMOPC_a53_cpu0_hpm_opc_END (9)
#define SOC_PMCTRL_A53_CPU01HPMOPC_a53_cpu1_hpm_opc_START (10)
#define SOC_PMCTRL_A53_CPU01HPMOPC_a53_cpu1_hpm_opc_END (19)
#define SOC_PMCTRL_A53_CPU01HPMOPC_a53_scu_hpm_opc_START (20)
#define SOC_PMCTRL_A53_CPU01HPMOPC_a53_scu_hpm_opc_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu0_hpmx_opc : 10;
        unsigned int a53_cpu1_hpmx_opc : 10;
        unsigned int a53_scu_hpmx_opc : 10;
        unsigned int reserved : 2;
    } reg;
} SOC_PMCTRL_A53_CPU01HPMXOPC_UNION;
#endif
#define SOC_PMCTRL_A53_CPU01HPMXOPC_a53_cpu0_hpmx_opc_START (0)
#define SOC_PMCTRL_A53_CPU01HPMXOPC_a53_cpu0_hpmx_opc_END (9)
#define SOC_PMCTRL_A53_CPU01HPMXOPC_a53_cpu1_hpmx_opc_START (10)
#define SOC_PMCTRL_A53_CPU01HPMXOPC_a53_cpu1_hpmx_opc_END (19)
#define SOC_PMCTRL_A53_CPU01HPMXOPC_a53_scu_hpmx_opc_START (20)
#define SOC_PMCTRL_A53_CPU01HPMXOPC_a53_scu_hpmx_opc_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu0_hpm_pc_avs : 10;
        unsigned int a53_cpu1_hpm_pc_avs : 10;
        unsigned int a53_scu_hpm_pc_avs : 10;
        unsigned int reserved : 2;
    } reg;
} SOC_PMCTRL_A53_CPU01HPMPC_UNION;
#endif
#define SOC_PMCTRL_A53_CPU01HPMPC_a53_cpu0_hpm_pc_avs_START (0)
#define SOC_PMCTRL_A53_CPU01HPMPC_a53_cpu0_hpm_pc_avs_END (9)
#define SOC_PMCTRL_A53_CPU01HPMPC_a53_cpu1_hpm_pc_avs_START (10)
#define SOC_PMCTRL_A53_CPU01HPMPC_a53_cpu1_hpm_pc_avs_END (19)
#define SOC_PMCTRL_A53_CPU01HPMPC_a53_scu_hpm_pc_avs_START (20)
#define SOC_PMCTRL_A53_CPU01HPMPC_a53_scu_hpm_pc_avs_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu2_hpm_en : 1;
        unsigned int a53_cpu3_hpm_en : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_A53_CPU23HPMEN_UNION;
#endif
#define SOC_PMCTRL_A53_CPU23HPMEN_a53_cpu2_hpm_en_START (0)
#define SOC_PMCTRL_A53_CPU23HPMEN_a53_cpu2_hpm_en_END (0)
#define SOC_PMCTRL_A53_CPU23HPMEN_a53_cpu3_hpm_en_START (1)
#define SOC_PMCTRL_A53_CPU23HPMEN_a53_cpu3_hpm_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu2_hpmx_en : 1;
        unsigned int a53_cpu3_hpmx_en : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_A53_CPU23HPMXEN_UNION;
#endif
#define SOC_PMCTRL_A53_CPU23HPMXEN_a53_cpu2_hpmx_en_START (0)
#define SOC_PMCTRL_A53_CPU23HPMXEN_a53_cpu2_hpmx_en_END (0)
#define SOC_PMCTRL_A53_CPU23HPMXEN_a53_cpu3_hpmx_en_START (1)
#define SOC_PMCTRL_A53_CPU23HPMXEN_a53_cpu3_hpmx_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu2_hpm_opc_vld : 1;
        unsigned int a53_cpu3_hpm_opc_vld : 1;
        unsigned int a53_cpu2_hpmx_opc_vld : 1;
        unsigned int a53_cpu3_hpmx_opc_vld : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_PMCTRL_A53_CPU23HPMOPCVALID_UNION;
#endif
#define SOC_PMCTRL_A53_CPU23HPMOPCVALID_a53_cpu2_hpm_opc_vld_START (0)
#define SOC_PMCTRL_A53_CPU23HPMOPCVALID_a53_cpu2_hpm_opc_vld_END (0)
#define SOC_PMCTRL_A53_CPU23HPMOPCVALID_a53_cpu3_hpm_opc_vld_START (1)
#define SOC_PMCTRL_A53_CPU23HPMOPCVALID_a53_cpu3_hpm_opc_vld_END (1)
#define SOC_PMCTRL_A53_CPU23HPMOPCVALID_a53_cpu2_hpmx_opc_vld_START (2)
#define SOC_PMCTRL_A53_CPU23HPMOPCVALID_a53_cpu2_hpmx_opc_vld_END (2)
#define SOC_PMCTRL_A53_CPU23HPMOPCVALID_a53_cpu3_hpmx_opc_vld_START (3)
#define SOC_PMCTRL_A53_CPU23HPMOPCVALID_a53_cpu3_hpmx_opc_vld_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu2_hpm_opc : 10;
        unsigned int a53_cpu3_hpm_opc : 10;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_A53_CPU23HPMOPC_UNION;
#endif
#define SOC_PMCTRL_A53_CPU23HPMOPC_a53_cpu2_hpm_opc_START (0)
#define SOC_PMCTRL_A53_CPU23HPMOPC_a53_cpu2_hpm_opc_END (9)
#define SOC_PMCTRL_A53_CPU23HPMOPC_a53_cpu3_hpm_opc_START (10)
#define SOC_PMCTRL_A53_CPU23HPMOPC_a53_cpu3_hpm_opc_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu2_hpmx_opc : 10;
        unsigned int a53_cpu3_hpmx_opc : 10;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_A53_CPU23HPMXOPC_UNION;
#endif
#define SOC_PMCTRL_A53_CPU23HPMXOPC_a53_cpu2_hpmx_opc_START (0)
#define SOC_PMCTRL_A53_CPU23HPMXOPC_a53_cpu2_hpmx_opc_END (9)
#define SOC_PMCTRL_A53_CPU23HPMXOPC_a53_cpu3_hpmx_opc_START (10)
#define SOC_PMCTRL_A53_CPU23HPMXOPC_a53_cpu3_hpmx_opc_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu2_hpm_pc_avs : 10;
        unsigned int a53_cpu3_hpm_pc_avs : 10;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_A53_CPU23HPMPC_UNION;
#endif
#define SOC_PMCTRL_A53_CPU23HPMPC_a53_cpu2_hpm_pc_avs_START (0)
#define SOC_PMCTRL_A53_CPU23HPMPC_a53_cpu2_hpm_pc_avs_END (9)
#define SOC_PMCTRL_A53_CPU23HPMPC_a53_cpu3_hpm_pc_avs_START (10)
#define SOC_PMCTRL_A53_CPU23HPMPC_a53_cpu3_hpm_pc_avs_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_avs_hpm_div : 6;
        unsigned int reserved : 26;
    } reg;
} SOC_PMCTRL_A53_CPUHPMCLKDIV_UNION;
#endif
#define SOC_PMCTRL_A53_CPUHPMCLKDIV_a53_cpu_avs_hpm_div_START (0)
#define SOC_PMCTRL_A53_CPUHPMCLKDIV_a53_cpu_avs_hpm_div_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpu_avs_vol_idx : 8;
        unsigned int a53_cpu_vol_chg_sftreq : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_PMCTRL_A53_CPUAVSVOLIDX_UNION;
#endif
#define SOC_PMCTRL_A53_CPUAVSVOLIDX_a53_cpu_avs_vol_idx_START (0)
#define SOC_PMCTRL_A53_CPUAVSVOLIDX_a53_cpu_avs_vol_idx_END (7)
#define SOC_PMCTRL_A53_CPUAVSVOLIDX_a53_cpu_vol_chg_sftreq_START (8)
#define SOC_PMCTRL_A53_CPUAVSVOLIDX_a53_cpu_vol_chg_sftreq_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_dvfs_mod : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_A57_CPUDVFSMOD_UNION;
#endif
#define SOC_PMCTRL_A57_CPUDVFSMOD_a57_cpu_dvfs_mod_START (0)
#define SOC_PMCTRL_A57_CPUDVFSMOD_a57_cpu_dvfs_mod_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_clk_sel : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_A57_CPUCLKSEL_UNION;
#endif
#define SOC_PMCTRL_A57_CPUCLKSEL_a57_clk_sel_START (0)
#define SOC_PMCTRL_A57_CPUCLKSEL_a57_clk_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_dvfs_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_A57_CPUDVFSEN_UNION;
#endif
#define SOC_PMCTRL_A57_CPUDVFSEN_a57_cpu_dvfs_en_START (0)
#define SOC_PMCTRL_A57_CPUDVFSEN_a57_cpu_dvfs_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_apll_sel : 2;
        unsigned int a57_pll_sw_ack : 5;
        unsigned int reserved : 25;
    } reg;
} SOC_PMCTRL_A57_CPUPLLSEL_UNION;
#endif
#define SOC_PMCTRL_A57_CPUPLLSEL_a57_apll_sel_START (0)
#define SOC_PMCTRL_A57_CPUPLLSEL_a57_apll_sel_END (1)
#define SOC_PMCTRL_A57_CPUPLLSEL_a57_pll_sw_ack_START (2)
#define SOC_PMCTRL_A57_CPUPLLSEL_a57_pll_sw_ack_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 2;
        unsigned int a57_atclken_sel : 2;
        unsigned int a57_aclkenm_sel : 3;
        unsigned int a57_atclken_l_sel : 1;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_PMCTRL_A57_CPUCLKDIV_UNION;
#endif
#define SOC_PMCTRL_A57_CPUCLKDIV_a57_atclken_sel_START (2)
#define SOC_PMCTRL_A57_CPUCLKDIV_a57_atclken_sel_END (3)
#define SOC_PMCTRL_A57_CPUCLKDIV_a57_aclkenm_sel_START (4)
#define SOC_PMCTRL_A57_CPUCLKDIV_a57_aclkenm_sel_END (6)
#define SOC_PMCTRL_A57_CPUCLKDIV_a57_atclken_l_sel_START (7)
#define SOC_PMCTRL_A57_CPUCLKDIV_a57_atclken_l_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_profile_updn : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_A57_CPUDVFSSTART_UNION;
#endif
#define SOC_PMCTRL_A57_CPUDVFSSTART_a57_cpu_profile_updn_START (0)
#define SOC_PMCTRL_A57_CPUDVFSSTART_a57_cpu_profile_updn_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_apll_refdiv : 6;
        unsigned int a57_apll_fbdiv : 12;
        unsigned int a57_apll_postdiv1 : 3;
        unsigned int a57_apll_postdiv2 : 3;
        unsigned int a57_apll_sw_target : 2;
        unsigned int reserved : 6;
    } reg;
} SOC_PMCTRL_A57_CPUCLKPROFILE0_UNION;
#endif
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_a57_apll_refdiv_START (0)
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_a57_apll_refdiv_END (5)
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_a57_apll_fbdiv_START (6)
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_a57_apll_fbdiv_END (17)
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_a57_apll_postdiv1_START (18)
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_a57_apll_postdiv1_END (20)
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_a57_apll_postdiv2_START (21)
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_a57_apll_postdiv2_END (23)
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_a57_apll_sw_target_START (24)
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_a57_apll_sw_target_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_apll_fracdiv : 24;
        unsigned int a57_apll_int_mod : 1;
        unsigned int a57_apll_cfg_vld : 1;
        unsigned int reserved : 6;
    } reg;
} SOC_PMCTRL_A57_CPUCLKPROFILE0_1_UNION;
#endif
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_1_a57_apll_fracdiv_START (0)
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_1_a57_apll_fracdiv_END (23)
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_1_a57_apll_int_mod_START (24)
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_1_a57_apll_int_mod_END (24)
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_1_a57_apll_cfg_vld_START (25)
#define SOC_PMCTRL_A57_CPUCLKPROFILE0_1_a57_apll_cfg_vld_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 2;
        unsigned int a57_atclken_sel_prof : 2;
        unsigned int a57_aclkenm_sel_prof : 3;
        unsigned int a57_atclken_l_prof : 1;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_PMCTRL_A57_CPUCLKPROFILE1_UNION;
#endif
#define SOC_PMCTRL_A57_CPUCLKPROFILE1_a57_atclken_sel_prof_START (2)
#define SOC_PMCTRL_A57_CPUCLKPROFILE1_a57_atclken_sel_prof_END (3)
#define SOC_PMCTRL_A57_CPUCLKPROFILE1_a57_aclkenm_sel_prof_START (4)
#define SOC_PMCTRL_A57_CPUCLKPROFILE1_a57_aclkenm_sel_prof_END (6)
#define SOC_PMCTRL_A57_CPUCLKPROFILE1_a57_atclken_l_prof_START (7)
#define SOC_PMCTRL_A57_CPUCLKPROFILE1_a57_atclken_l_prof_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_vol_mod : 1;
        unsigned int a57_cpu_clk_off_vol_mod : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_A57_CPUVOLMOD_UNION;
#endif
#define SOC_PMCTRL_A57_CPUVOLMOD_a57_cpu_vol_mod_START (0)
#define SOC_PMCTRL_A57_CPUVOLMOD_a57_cpu_vol_mod_END (0)
#define SOC_PMCTRL_A57_CPUVOLMOD_a57_cpu_clk_off_vol_mod_START (1)
#define SOC_PMCTRL_A57_CPUVOLMOD_a57_cpu_clk_off_vol_mod_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_vol_idx : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_A57_CPUVOLPROFILE_UNION;
#endif
#define SOC_PMCTRL_A57_CPUVOLPROFILE_a57_cpu_vol_idx_START (0)
#define SOC_PMCTRL_A57_CPUVOLPROFILE_a57_cpu_vol_idx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_phase_mod : 1;
        unsigned int a57_cpu_clk_off_phase_mod : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_A57_CPUPHASEMOD_UNION;
#endif
#define SOC_PMCTRL_A57_CPUPHASEMOD_a57_cpu_phase_mod_START (0)
#define SOC_PMCTRL_A57_CPUPHASEMOD_a57_cpu_phase_mod_END (0)
#define SOC_PMCTRL_A57_CPUPHASEMOD_a57_cpu_clk_off_phase_mod_START (1)
#define SOC_PMCTRL_A57_CPUPHASEMOD_a57_cpu_clk_off_phase_mod_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_phase_idx : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_A57_CPUPHASEPROFILE_UNION;
#endif
#define SOC_PMCTRL_A57_CPUPHASEPROFILE_a57_cpu_phase_idx_START (0)
#define SOC_PMCTRL_A57_CPUPHASEPROFILE_a57_cpu_phase_idx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_vol_phase_sel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_A57_CPUVOLPHASESEL_UNION;
#endif
#define SOC_PMCTRL_A57_CPUVOLPHASESEL_a57_cpu_vol_phase_sel_START (0)
#define SOC_PMCTRL_A57_CPUVOLPHASESEL_a57_cpu_vol_phase_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_vol_up_step_time : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_PMCTRL_A57_VOLUPSTEPTIME_UNION;
#endif
#define SOC_PMCTRL_A57_VOLUPSTEPTIME_a57_vol_up_step_time_START (0)
#define SOC_PMCTRL_A57_VOLUPSTEPTIME_a57_vol_up_step_time_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_vol_dn_step_time : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_PMCTRL_A57_VOLDNSTEPTIME_UNION;
#endif
#define SOC_PMCTRL_A57_VOLDNSTEPTIME_a57_vol_dn_step_time_START (0)
#define SOC_PMCTRL_A57_VOLDNSTEPTIME_a57_vol_dn_step_time_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_dvfs_phase_up_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_A57_PHASEUPTIME_UNION;
#endif
#define SOC_PMCTRL_A57_PHASEUPTIME_a57_dvfs_phase_up_time_START (0)
#define SOC_PMCTRL_A57_PHASEUPTIME_a57_dvfs_phase_up_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_dvfs_phase_dn_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_A57_PHASEDNTIME_UNION;
#endif
#define SOC_PMCTRL_A57_PHASEDNTIME_a57_dvfs_phase_dn_time_START (0)
#define SOC_PMCTRL_A57_PHASEDNTIME_a57_dvfs_phase_dn_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_pmu_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_A57_PMUHOLDTIME_UNION;
#endif
#define SOC_PMCTRL_A57_PMUHOLDTIME_a57_pmu_vol_hold_time_START (0)
#define SOC_PMCTRL_A57_PMUHOLDTIME_a57_pmu_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_pmu_sel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_A57_PMUSEL_UNION;
#endif
#define SOC_PMCTRL_A57_PMUSEL_a57_pmu_sel_START (0)
#define SOC_PMCTRL_A57_PMUSEL_a57_pmu_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_avs_en : 1;
        unsigned int a57_cpu_avs_pwctrl_en : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_A57_CPUAVSEN_UNION;
#endif
#define SOC_PMCTRL_A57_CPUAVSEN_a57_cpu_avs_en_START (0)
#define SOC_PMCTRL_A57_CPUAVSEN_a57_cpu_avs_en_END (0)
#define SOC_PMCTRL_A57_CPUAVSEN_a57_cpu_avs_pwctrl_en_START (1)
#define SOC_PMCTRL_A57_CPUAVSEN_a57_cpu_avs_pwctrl_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_avspara_mod : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_A57_CPUAVSPARAMOD_UNION;
#endif
#define SOC_PMCTRL_A57_CPUAVSPARAMOD_a57_cpu_avspara_mod_START (0)
#define SOC_PMCTRL_A57_CPUAVSPARAMOD_a57_cpu_avspara_mod_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 2;
        unsigned int a57_cpu_avs_opc_offset : 10;
        unsigned int a57_cpu_avs_rcc : 5;
        unsigned int a57_cpu_avs_irgap : 5;
        unsigned int a57_cpu_avs_opc_mod : 2;
        unsigned int a57_cpu_avs_hpm_sel : 4;
        unsigned int a57_cpu_avs_opc_shift : 3;
        unsigned int reserved_1 : 1;
    } reg;
} SOC_PMCTRL_A57_CPUAVSPARA0_UNION;
#endif
#define SOC_PMCTRL_A57_CPUAVSPARA0_a57_cpu_avs_opc_offset_START (2)
#define SOC_PMCTRL_A57_CPUAVSPARA0_a57_cpu_avs_opc_offset_END (11)
#define SOC_PMCTRL_A57_CPUAVSPARA0_a57_cpu_avs_rcc_START (12)
#define SOC_PMCTRL_A57_CPUAVSPARA0_a57_cpu_avs_rcc_END (16)
#define SOC_PMCTRL_A57_CPUAVSPARA0_a57_cpu_avs_irgap_START (17)
#define SOC_PMCTRL_A57_CPUAVSPARA0_a57_cpu_avs_irgap_END (21)
#define SOC_PMCTRL_A57_CPUAVSPARA0_a57_cpu_avs_opc_mod_START (22)
#define SOC_PMCTRL_A57_CPUAVSPARA0_a57_cpu_avs_opc_mod_END (23)
#define SOC_PMCTRL_A57_CPUAVSPARA0_a57_cpu_avs_hpm_sel_START (24)
#define SOC_PMCTRL_A57_CPUAVSPARA0_a57_cpu_avs_hpm_sel_END (27)
#define SOC_PMCTRL_A57_CPUAVSPARA0_a57_cpu_avs_opc_shift_START (28)
#define SOC_PMCTRL_A57_CPUAVSPARA0_a57_cpu_avs_opc_shift_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_avs_vol_up_gain1 : 4;
        unsigned int a57_cpu_avs_vol_up_gain2 : 4;
        unsigned int a57_cpu_avs_vol_up_gain3 : 4;
        unsigned int a57_cpu_avs_vol_up_gain4 : 4;
        unsigned int a57_cpu_avs_vol_up_gain5 : 4;
        unsigned int a57_cpu_avs_vol_up_gain6 : 4;
        unsigned int a57_cpu_avs_vol_up_gain7 : 4;
        unsigned int a57_cpu_avs_vol_up_gain8 : 4;
    } reg;
} SOC_PMCTRL_A57_CPUAVSPARA1_UNION;
#endif
#define SOC_PMCTRL_A57_CPUAVSPARA1_a57_cpu_avs_vol_up_gain1_START (0)
#define SOC_PMCTRL_A57_CPUAVSPARA1_a57_cpu_avs_vol_up_gain1_END (3)
#define SOC_PMCTRL_A57_CPUAVSPARA1_a57_cpu_avs_vol_up_gain2_START (4)
#define SOC_PMCTRL_A57_CPUAVSPARA1_a57_cpu_avs_vol_up_gain2_END (7)
#define SOC_PMCTRL_A57_CPUAVSPARA1_a57_cpu_avs_vol_up_gain3_START (8)
#define SOC_PMCTRL_A57_CPUAVSPARA1_a57_cpu_avs_vol_up_gain3_END (11)
#define SOC_PMCTRL_A57_CPUAVSPARA1_a57_cpu_avs_vol_up_gain4_START (12)
#define SOC_PMCTRL_A57_CPUAVSPARA1_a57_cpu_avs_vol_up_gain4_END (15)
#define SOC_PMCTRL_A57_CPUAVSPARA1_a57_cpu_avs_vol_up_gain5_START (16)
#define SOC_PMCTRL_A57_CPUAVSPARA1_a57_cpu_avs_vol_up_gain5_END (19)
#define SOC_PMCTRL_A57_CPUAVSPARA1_a57_cpu_avs_vol_up_gain6_START (20)
#define SOC_PMCTRL_A57_CPUAVSPARA1_a57_cpu_avs_vol_up_gain6_END (23)
#define SOC_PMCTRL_A57_CPUAVSPARA1_a57_cpu_avs_vol_up_gain7_START (24)
#define SOC_PMCTRL_A57_CPUAVSPARA1_a57_cpu_avs_vol_up_gain7_END (27)
#define SOC_PMCTRL_A57_CPUAVSPARA1_a57_cpu_avs_vol_up_gain8_START (28)
#define SOC_PMCTRL_A57_CPUAVSPARA1_a57_cpu_avs_vol_up_gain8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_avs_vol_up_gain9 : 4;
        unsigned int a57_cpu_avs_vol_up_gain10 : 4;
        unsigned int a57_cpu_avs_vol_up_gain11 : 4;
        unsigned int a57_cpu_avs_vol_up_gain12 : 4;
        unsigned int a57_cpu_avs_vol_up_gain13 : 4;
        unsigned int a57_cpu_avs_vol_up_gain14 : 4;
        unsigned int a57_cpu_avs_vol_up_gain15 : 7;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_A57_CPUAVSPARA2_UNION;
#endif
#define SOC_PMCTRL_A57_CPUAVSPARA2_a57_cpu_avs_vol_up_gain9_START (0)
#define SOC_PMCTRL_A57_CPUAVSPARA2_a57_cpu_avs_vol_up_gain9_END (3)
#define SOC_PMCTRL_A57_CPUAVSPARA2_a57_cpu_avs_vol_up_gain10_START (4)
#define SOC_PMCTRL_A57_CPUAVSPARA2_a57_cpu_avs_vol_up_gain10_END (7)
#define SOC_PMCTRL_A57_CPUAVSPARA2_a57_cpu_avs_vol_up_gain11_START (8)
#define SOC_PMCTRL_A57_CPUAVSPARA2_a57_cpu_avs_vol_up_gain11_END (11)
#define SOC_PMCTRL_A57_CPUAVSPARA2_a57_cpu_avs_vol_up_gain12_START (12)
#define SOC_PMCTRL_A57_CPUAVSPARA2_a57_cpu_avs_vol_up_gain12_END (15)
#define SOC_PMCTRL_A57_CPUAVSPARA2_a57_cpu_avs_vol_up_gain13_START (16)
#define SOC_PMCTRL_A57_CPUAVSPARA2_a57_cpu_avs_vol_up_gain13_END (19)
#define SOC_PMCTRL_A57_CPUAVSPARA2_a57_cpu_avs_vol_up_gain14_START (20)
#define SOC_PMCTRL_A57_CPUAVSPARA2_a57_cpu_avs_vol_up_gain14_END (23)
#define SOC_PMCTRL_A57_CPUAVSPARA2_a57_cpu_avs_vol_up_gain15_START (24)
#define SOC_PMCTRL_A57_CPUAVSPARA2_a57_cpu_avs_vol_up_gain15_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_avs_vol_dn_gain1 : 4;
        unsigned int a57_cpu_avs_vol_dn_gain2 : 4;
        unsigned int a57_cpu_avs_vol_dn_gain3 : 4;
        unsigned int a57_cpu_avs_vol_dn_gain4 : 4;
        unsigned int a57_cpu_avs_vol_dn_gain5 : 4;
        unsigned int a57_cpu_avs_vol_dn_gain6 : 4;
        unsigned int a57_cpu_avs_vol_dn_gain7 : 4;
        unsigned int a57_cpu_avs_vol_dn_gain8 : 4;
    } reg;
} SOC_PMCTRL_A57_CPUAVSPARA3_UNION;
#endif
#define SOC_PMCTRL_A57_CPUAVSPARA3_a57_cpu_avs_vol_dn_gain1_START (0)
#define SOC_PMCTRL_A57_CPUAVSPARA3_a57_cpu_avs_vol_dn_gain1_END (3)
#define SOC_PMCTRL_A57_CPUAVSPARA3_a57_cpu_avs_vol_dn_gain2_START (4)
#define SOC_PMCTRL_A57_CPUAVSPARA3_a57_cpu_avs_vol_dn_gain2_END (7)
#define SOC_PMCTRL_A57_CPUAVSPARA3_a57_cpu_avs_vol_dn_gain3_START (8)
#define SOC_PMCTRL_A57_CPUAVSPARA3_a57_cpu_avs_vol_dn_gain3_END (11)
#define SOC_PMCTRL_A57_CPUAVSPARA3_a57_cpu_avs_vol_dn_gain4_START (12)
#define SOC_PMCTRL_A57_CPUAVSPARA3_a57_cpu_avs_vol_dn_gain4_END (15)
#define SOC_PMCTRL_A57_CPUAVSPARA3_a57_cpu_avs_vol_dn_gain5_START (16)
#define SOC_PMCTRL_A57_CPUAVSPARA3_a57_cpu_avs_vol_dn_gain5_END (19)
#define SOC_PMCTRL_A57_CPUAVSPARA3_a57_cpu_avs_vol_dn_gain6_START (20)
#define SOC_PMCTRL_A57_CPUAVSPARA3_a57_cpu_avs_vol_dn_gain6_END (23)
#define SOC_PMCTRL_A57_CPUAVSPARA3_a57_cpu_avs_vol_dn_gain7_START (24)
#define SOC_PMCTRL_A57_CPUAVSPARA3_a57_cpu_avs_vol_dn_gain7_END (27)
#define SOC_PMCTRL_A57_CPUAVSPARA3_a57_cpu_avs_vol_dn_gain8_START (28)
#define SOC_PMCTRL_A57_CPUAVSPARA3_a57_cpu_avs_vol_dn_gain8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_avs_vol_dn_gain9 : 4;
        unsigned int a57_cpu_avs_vol_dn_gain10 : 4;
        unsigned int a57_cpu_avs_vol_dn_gain11 : 4;
        unsigned int a57_cpu_avs_vol_dn_gain12 : 4;
        unsigned int a57_cpu_avs_vol_dn_gain13 : 4;
        unsigned int a57_cpu_avs_vol_dn_gain14 : 4;
        unsigned int a57_cpu_avs_vol_dn_gain15 : 7;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_A57_CPUAVSPARA4_UNION;
#endif
#define SOC_PMCTRL_A57_CPUAVSPARA4_a57_cpu_avs_vol_dn_gain9_START (0)
#define SOC_PMCTRL_A57_CPUAVSPARA4_a57_cpu_avs_vol_dn_gain9_END (3)
#define SOC_PMCTRL_A57_CPUAVSPARA4_a57_cpu_avs_vol_dn_gain10_START (4)
#define SOC_PMCTRL_A57_CPUAVSPARA4_a57_cpu_avs_vol_dn_gain10_END (7)
#define SOC_PMCTRL_A57_CPUAVSPARA4_a57_cpu_avs_vol_dn_gain11_START (8)
#define SOC_PMCTRL_A57_CPUAVSPARA4_a57_cpu_avs_vol_dn_gain11_END (11)
#define SOC_PMCTRL_A57_CPUAVSPARA4_a57_cpu_avs_vol_dn_gain12_START (12)
#define SOC_PMCTRL_A57_CPUAVSPARA4_a57_cpu_avs_vol_dn_gain12_END (15)
#define SOC_PMCTRL_A57_CPUAVSPARA4_a57_cpu_avs_vol_dn_gain13_START (16)
#define SOC_PMCTRL_A57_CPUAVSPARA4_a57_cpu_avs_vol_dn_gain13_END (19)
#define SOC_PMCTRL_A57_CPUAVSPARA4_a57_cpu_avs_vol_dn_gain14_START (20)
#define SOC_PMCTRL_A57_CPUAVSPARA4_a57_cpu_avs_vol_dn_gain14_END (23)
#define SOC_PMCTRL_A57_CPUAVSPARA4_a57_cpu_avs_vol_dn_gain15_START (24)
#define SOC_PMCTRL_A57_CPUAVSPARA4_a57_cpu_avs_vol_dn_gain15_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_avs_max_vol : 7;
        unsigned int a57_cpu_avs_min_vol : 7;
        unsigned int reserved : 18;
    } reg;
} SOC_PMCTRL_A57_CPUAVSPARA5_UNION;
#endif
#define SOC_PMCTRL_A57_CPUAVSPARA5_a57_cpu_avs_max_vol_START (0)
#define SOC_PMCTRL_A57_CPUAVSPARA5_a57_cpu_avs_max_vol_END (6)
#define SOC_PMCTRL_A57_CPUAVSPARA5_a57_cpu_avs_min_vol_START (7)
#define SOC_PMCTRL_A57_CPUAVSPARA5_a57_cpu_avs_min_vol_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_avs_sample_num : 22;
        unsigned int reserved : 10;
    } reg;
} SOC_PMCTRL_A57_CPUAVSPARA6_UNION;
#endif
#define SOC_PMCTRL_A57_CPUAVSPARA6_a57_cpu_avs_sample_num_START (0)
#define SOC_PMCTRL_A57_CPUAVSPARA6_a57_cpu_avs_sample_num_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_hpm_type : 1;
        unsigned int a57_cpu0_hpm_mask : 1;
        unsigned int a57_cpu1_hpm_mask : 1;
        unsigned int a57_cpu2_hpm_mask : 1;
        unsigned int a57_cpu3_hpm_mask : 1;
        unsigned int a57_scu_hpm_mask : 1;
        unsigned int reserved : 10;
        unsigned int biten : 16;
    } reg;
} SOC_PMCTRL_A57_CPUHPMTYP_UNION;
#endif
#define SOC_PMCTRL_A57_CPUHPMTYP_a57_cpu_hpm_type_START (0)
#define SOC_PMCTRL_A57_CPUHPMTYP_a57_cpu_hpm_type_END (0)
#define SOC_PMCTRL_A57_CPUHPMTYP_a57_cpu0_hpm_mask_START (1)
#define SOC_PMCTRL_A57_CPUHPMTYP_a57_cpu0_hpm_mask_END (1)
#define SOC_PMCTRL_A57_CPUHPMTYP_a57_cpu1_hpm_mask_START (2)
#define SOC_PMCTRL_A57_CPUHPMTYP_a57_cpu1_hpm_mask_END (2)
#define SOC_PMCTRL_A57_CPUHPMTYP_a57_cpu2_hpm_mask_START (3)
#define SOC_PMCTRL_A57_CPUHPMTYP_a57_cpu2_hpm_mask_END (3)
#define SOC_PMCTRL_A57_CPUHPMTYP_a57_cpu3_hpm_mask_START (4)
#define SOC_PMCTRL_A57_CPUHPMTYP_a57_cpu3_hpm_mask_END (4)
#define SOC_PMCTRL_A57_CPUHPMTYP_a57_scu_hpm_mask_START (5)
#define SOC_PMCTRL_A57_CPUHPMTYP_a57_scu_hpm_mask_END (5)
#define SOC_PMCTRL_A57_CPUHPMTYP_biten_START (16)
#define SOC_PMCTRL_A57_CPUHPMTYP_biten_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu0_hpm_en : 1;
        unsigned int a57_cpu1_hpm_en : 1;
        unsigned int a57_scu_hpm_en : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_PMCTRL_A57_CPU01HPMEN_UNION;
#endif
#define SOC_PMCTRL_A57_CPU01HPMEN_a57_cpu0_hpm_en_START (0)
#define SOC_PMCTRL_A57_CPU01HPMEN_a57_cpu0_hpm_en_END (0)
#define SOC_PMCTRL_A57_CPU01HPMEN_a57_cpu1_hpm_en_START (1)
#define SOC_PMCTRL_A57_CPU01HPMEN_a57_cpu1_hpm_en_END (1)
#define SOC_PMCTRL_A57_CPU01HPMEN_a57_scu_hpm_en_START (2)
#define SOC_PMCTRL_A57_CPU01HPMEN_a57_scu_hpm_en_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu0_hpmx_en : 1;
        unsigned int a57_cpu1_hpmx_en : 1;
        unsigned int a57_scu_hpmx_en : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_PMCTRL_A57_CPU01HPMXEN_UNION;
#endif
#define SOC_PMCTRL_A57_CPU01HPMXEN_a57_cpu0_hpmx_en_START (0)
#define SOC_PMCTRL_A57_CPU01HPMXEN_a57_cpu0_hpmx_en_END (0)
#define SOC_PMCTRL_A57_CPU01HPMXEN_a57_cpu1_hpmx_en_START (1)
#define SOC_PMCTRL_A57_CPU01HPMXEN_a57_cpu1_hpmx_en_END (1)
#define SOC_PMCTRL_A57_CPU01HPMXEN_a57_scu_hpmx_en_START (2)
#define SOC_PMCTRL_A57_CPU01HPMXEN_a57_scu_hpmx_en_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu0_hpm_opc_vld : 1;
        unsigned int a57_cpu1_hpm_opc_vld : 1;
        unsigned int a57_scu_hpm_opc_vld : 1;
        unsigned int a57_cpu0_hpmx_opc_vld : 1;
        unsigned int a57_cpu1_hpmx_opc_vld : 1;
        unsigned int a57_scu_hpmx_opc_vld : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_PMCTRL_A57_CPU01HPMOPCVALID_UNION;
#endif
#define SOC_PMCTRL_A57_CPU01HPMOPCVALID_a57_cpu0_hpm_opc_vld_START (0)
#define SOC_PMCTRL_A57_CPU01HPMOPCVALID_a57_cpu0_hpm_opc_vld_END (0)
#define SOC_PMCTRL_A57_CPU01HPMOPCVALID_a57_cpu1_hpm_opc_vld_START (1)
#define SOC_PMCTRL_A57_CPU01HPMOPCVALID_a57_cpu1_hpm_opc_vld_END (1)
#define SOC_PMCTRL_A57_CPU01HPMOPCVALID_a57_scu_hpm_opc_vld_START (2)
#define SOC_PMCTRL_A57_CPU01HPMOPCVALID_a57_scu_hpm_opc_vld_END (2)
#define SOC_PMCTRL_A57_CPU01HPMOPCVALID_a57_cpu0_hpmx_opc_vld_START (3)
#define SOC_PMCTRL_A57_CPU01HPMOPCVALID_a57_cpu0_hpmx_opc_vld_END (3)
#define SOC_PMCTRL_A57_CPU01HPMOPCVALID_a57_cpu1_hpmx_opc_vld_START (4)
#define SOC_PMCTRL_A57_CPU01HPMOPCVALID_a57_cpu1_hpmx_opc_vld_END (4)
#define SOC_PMCTRL_A57_CPU01HPMOPCVALID_a57_scu_hpmx_opc_vld_START (5)
#define SOC_PMCTRL_A57_CPU01HPMOPCVALID_a57_scu_hpmx_opc_vld_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu0_hpm_opc : 10;
        unsigned int a57_cpu1_hpm_opc : 10;
        unsigned int a57_scu_hpm_opc : 10;
        unsigned int reserved : 2;
    } reg;
} SOC_PMCTRL_A57_CPU01HPMOPC_UNION;
#endif
#define SOC_PMCTRL_A57_CPU01HPMOPC_a57_cpu0_hpm_opc_START (0)
#define SOC_PMCTRL_A57_CPU01HPMOPC_a57_cpu0_hpm_opc_END (9)
#define SOC_PMCTRL_A57_CPU01HPMOPC_a57_cpu1_hpm_opc_START (10)
#define SOC_PMCTRL_A57_CPU01HPMOPC_a57_cpu1_hpm_opc_END (19)
#define SOC_PMCTRL_A57_CPU01HPMOPC_a57_scu_hpm_opc_START (20)
#define SOC_PMCTRL_A57_CPU01HPMOPC_a57_scu_hpm_opc_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu0_hpmx_opc : 10;
        unsigned int a57_cpu1_hpmx_opc : 10;
        unsigned int a57_scu_hpmx_opc : 10;
        unsigned int reserved : 2;
    } reg;
} SOC_PMCTRL_A57_CPU01HPMXOPC_UNION;
#endif
#define SOC_PMCTRL_A57_CPU01HPMXOPC_a57_cpu0_hpmx_opc_START (0)
#define SOC_PMCTRL_A57_CPU01HPMXOPC_a57_cpu0_hpmx_opc_END (9)
#define SOC_PMCTRL_A57_CPU01HPMXOPC_a57_cpu1_hpmx_opc_START (10)
#define SOC_PMCTRL_A57_CPU01HPMXOPC_a57_cpu1_hpmx_opc_END (19)
#define SOC_PMCTRL_A57_CPU01HPMXOPC_a57_scu_hpmx_opc_START (20)
#define SOC_PMCTRL_A57_CPU01HPMXOPC_a57_scu_hpmx_opc_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu0_hpm_pc_avs : 10;
        unsigned int a57_cpu1_hpm_pc_avs : 10;
        unsigned int a57_scu_hpm_pc_avs : 10;
        unsigned int reserved : 2;
    } reg;
} SOC_PMCTRL_A57_CPU01HPMPC_UNION;
#endif
#define SOC_PMCTRL_A57_CPU01HPMPC_a57_cpu0_hpm_pc_avs_START (0)
#define SOC_PMCTRL_A57_CPU01HPMPC_a57_cpu0_hpm_pc_avs_END (9)
#define SOC_PMCTRL_A57_CPU01HPMPC_a57_cpu1_hpm_pc_avs_START (10)
#define SOC_PMCTRL_A57_CPU01HPMPC_a57_cpu1_hpm_pc_avs_END (19)
#define SOC_PMCTRL_A57_CPU01HPMPC_a57_scu_hpm_pc_avs_START (20)
#define SOC_PMCTRL_A57_CPU01HPMPC_a57_scu_hpm_pc_avs_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu2_hpm_en : 1;
        unsigned int a57_cpu3_hpm_en : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_A57_CPU23HPMEN_UNION;
#endif
#define SOC_PMCTRL_A57_CPU23HPMEN_a57_cpu2_hpm_en_START (0)
#define SOC_PMCTRL_A57_CPU23HPMEN_a57_cpu2_hpm_en_END (0)
#define SOC_PMCTRL_A57_CPU23HPMEN_a57_cpu3_hpm_en_START (1)
#define SOC_PMCTRL_A57_CPU23HPMEN_a57_cpu3_hpm_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu2_hpmx_en : 1;
        unsigned int a57_cpu3_hpmx_en : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_A57_CPU23HPMXEN_UNION;
#endif
#define SOC_PMCTRL_A57_CPU23HPMXEN_a57_cpu2_hpmx_en_START (0)
#define SOC_PMCTRL_A57_CPU23HPMXEN_a57_cpu2_hpmx_en_END (0)
#define SOC_PMCTRL_A57_CPU23HPMXEN_a57_cpu3_hpmx_en_START (1)
#define SOC_PMCTRL_A57_CPU23HPMXEN_a57_cpu3_hpmx_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu2_hpm_opc_vld : 1;
        unsigned int a57_cpu3_hpm_opc_vld : 1;
        unsigned int a57_cpu2_hpmx_opc_vld : 1;
        unsigned int a57_cpu3_hpmx_opc_vld : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_PMCTRL_A57_CPU23HPMOPCVALID_UNION;
#endif
#define SOC_PMCTRL_A57_CPU23HPMOPCVALID_a57_cpu2_hpm_opc_vld_START (0)
#define SOC_PMCTRL_A57_CPU23HPMOPCVALID_a57_cpu2_hpm_opc_vld_END (0)
#define SOC_PMCTRL_A57_CPU23HPMOPCVALID_a57_cpu3_hpm_opc_vld_START (1)
#define SOC_PMCTRL_A57_CPU23HPMOPCVALID_a57_cpu3_hpm_opc_vld_END (1)
#define SOC_PMCTRL_A57_CPU23HPMOPCVALID_a57_cpu2_hpmx_opc_vld_START (2)
#define SOC_PMCTRL_A57_CPU23HPMOPCVALID_a57_cpu2_hpmx_opc_vld_END (2)
#define SOC_PMCTRL_A57_CPU23HPMOPCVALID_a57_cpu3_hpmx_opc_vld_START (3)
#define SOC_PMCTRL_A57_CPU23HPMOPCVALID_a57_cpu3_hpmx_opc_vld_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu2_hpm_opc : 10;
        unsigned int a57_cpu3_hpm_opc : 10;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_A57_CPU23HPMOPC_UNION;
#endif
#define SOC_PMCTRL_A57_CPU23HPMOPC_a57_cpu2_hpm_opc_START (0)
#define SOC_PMCTRL_A57_CPU23HPMOPC_a57_cpu2_hpm_opc_END (9)
#define SOC_PMCTRL_A57_CPU23HPMOPC_a57_cpu3_hpm_opc_START (10)
#define SOC_PMCTRL_A57_CPU23HPMOPC_a57_cpu3_hpm_opc_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu2_hpmx_opc : 10;
        unsigned int a57_cpu3_hpmx_opc : 10;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_A57_CPU23HPMXOPC_UNION;
#endif
#define SOC_PMCTRL_A57_CPU23HPMXOPC_a57_cpu2_hpmx_opc_START (0)
#define SOC_PMCTRL_A57_CPU23HPMXOPC_a57_cpu2_hpmx_opc_END (9)
#define SOC_PMCTRL_A57_CPU23HPMXOPC_a57_cpu3_hpmx_opc_START (10)
#define SOC_PMCTRL_A57_CPU23HPMXOPC_a57_cpu3_hpmx_opc_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu2_hpm_pc_avs : 10;
        unsigned int a57_cpu3_hpm_pc_avs : 10;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_A57_CPU23HPMPC_UNION;
#endif
#define SOC_PMCTRL_A57_CPU23HPMPC_a57_cpu2_hpm_pc_avs_START (0)
#define SOC_PMCTRL_A57_CPU23HPMPC_a57_cpu2_hpm_pc_avs_END (9)
#define SOC_PMCTRL_A57_CPU23HPMPC_a57_cpu3_hpm_pc_avs_START (10)
#define SOC_PMCTRL_A57_CPU23HPMPC_a57_cpu3_hpm_pc_avs_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_avs_hpm_div : 6;
        unsigned int reserved : 26;
    } reg;
} SOC_PMCTRL_A57_CPUHPMCLKDIV_UNION;
#endif
#define SOC_PMCTRL_A57_CPUHPMCLKDIV_a57_cpu_avs_hpm_div_START (0)
#define SOC_PMCTRL_A57_CPUHPMCLKDIV_a57_cpu_avs_hpm_div_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a57_cpu_avs_vol_idx : 8;
        unsigned int a57_cpu_vol_chg_sftreq : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_PMCTRL_A57_CPUAVSVOLIDX_UNION;
#endif
#define SOC_PMCTRL_A57_CPUAVSVOLIDX_a57_cpu_avs_vol_idx_START (0)
#define SOC_PMCTRL_A57_CPUAVSVOLIDX_a57_cpu_avs_vol_idx_END (7)
#define SOC_PMCTRL_A57_CPUAVSVOLIDX_a57_cpu_vol_chg_sftreq_START (8)
#define SOC_PMCTRL_A57_CPUAVSVOLIDX_a57_cpu_vol_chg_sftreq_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3da_sw : 1;
        unsigned int g3da_sw_ack : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_G3DCLKSEL_UNION;
#endif
#define SOC_PMCTRL_G3DCLKSEL_g3da_sw_START (0)
#define SOC_PMCTRL_G3DCLKSEL_g3da_sw_END (0)
#define SOC_PMCTRL_G3DCLKSEL_g3da_sw_ack_START (1)
#define SOC_PMCTRL_G3DCLKSEL_g3da_sw_ack_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3da_clk_div0 : 8;
        unsigned int g3da_clk_div1 : 8;
        unsigned int g3da_clk_div0_ack : 1;
        unsigned int g3da_clk_div1_ack : 1;
        unsigned int reserved : 2;
        unsigned int g3da_clk_div0_mux : 6;
        unsigned int g3da_clk_div1_mux : 6;
    } reg;
} SOC_PMCTRL_G3DCLKDIV_UNION;
#endif
#define SOC_PMCTRL_G3DCLKDIV_g3da_clk_div0_START (0)
#define SOC_PMCTRL_G3DCLKDIV_g3da_clk_div0_END (7)
#define SOC_PMCTRL_G3DCLKDIV_g3da_clk_div1_START (8)
#define SOC_PMCTRL_G3DCLKDIV_g3da_clk_div1_END (15)
#define SOC_PMCTRL_G3DCLKDIV_g3da_clk_div0_ack_START (16)
#define SOC_PMCTRL_G3DCLKDIV_g3da_clk_div0_ack_END (16)
#define SOC_PMCTRL_G3DCLKDIV_g3da_clk_div1_ack_START (17)
#define SOC_PMCTRL_G3DCLKDIV_g3da_clk_div1_ack_END (17)
#define SOC_PMCTRL_G3DCLKDIV_g3da_clk_div0_mux_START (20)
#define SOC_PMCTRL_G3DCLKDIV_g3da_clk_div0_mux_END (25)
#define SOC_PMCTRL_G3DCLKDIV_g3da_clk_div1_mux_START (26)
#define SOC_PMCTRL_G3DCLKDIV_g3da_clk_div1_mux_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_dvfs_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_G3DDVFSEN_UNION;
#endif
#define SOC_PMCTRL_G3DDVFSEN_g3d_dvfs_en_START (0)
#define SOC_PMCTRL_G3DDVFSEN_g3d_dvfs_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_profile_updn : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_G3DDVFSSTART_UNION;
#endif
#define SOC_PMCTRL_G3DDVFSSTART_g3d_profile_updn_START (0)
#define SOC_PMCTRL_G3DDVFSSTART_g3d_profile_updn_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3da_clk_div : 8;
        unsigned int g3da_clk_sel : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_PMCTRL_G3DCLKPROFILE_UNION;
#endif
#define SOC_PMCTRL_G3DCLKPROFILE_g3da_clk_div_START (0)
#define SOC_PMCTRL_G3DCLKPROFILE_g3da_clk_div_END (7)
#define SOC_PMCTRL_G3DCLKPROFILE_g3da_clk_sel_START (8)
#define SOC_PMCTRL_G3DCLKPROFILE_g3da_clk_sel_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_vol_mod : 1;
        unsigned int g3d_clk_off_vol_mod : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_G3DVOLMOD_UNION;
#endif
#define SOC_PMCTRL_G3DVOLMOD_g3d_vol_mod_START (0)
#define SOC_PMCTRL_G3DVOLMOD_g3d_vol_mod_END (0)
#define SOC_PMCTRL_G3DVOLMOD_g3d_clk_off_vol_mod_START (1)
#define SOC_PMCTRL_G3DVOLMOD_g3d_clk_off_vol_mod_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_vol_idx : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_G3DVOLPROFILE_UNION;
#endif
#define SOC_PMCTRL_G3DVOLPROFILE_g3d_vol_idx_START (0)
#define SOC_PMCTRL_G3DVOLPROFILE_g3d_vol_idx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_phase_mod : 1;
        unsigned int g3d_clk_off_phase_mod : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_G3DPHASEMOD_UNION;
#endif
#define SOC_PMCTRL_G3DPHASEMOD_g3d_phase_mod_START (0)
#define SOC_PMCTRL_G3DPHASEMOD_g3d_phase_mod_END (0)
#define SOC_PMCTRL_G3DPHASEMOD_g3d_clk_off_phase_mod_START (1)
#define SOC_PMCTRL_G3DPHASEMOD_g3d_clk_off_phase_mod_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_phase_idx : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_G3DPHASEPROFILE_UNION;
#endif
#define SOC_PMCTRL_G3DPHASEPROFILE_g3d_phase_idx_START (0)
#define SOC_PMCTRL_G3DPHASEPROFILE_g3d_phase_idx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_vol_phase_sel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_G3DVOLPHASESEL_UNION;
#endif
#define SOC_PMCTRL_G3DVOLPHASESEL_g3d_vol_phase_sel_START (0)
#define SOC_PMCTRL_G3DVOLPHASESEL_g3d_vol_phase_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_pmu_sel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_G3DPMUSEL_UNION;
#endif
#define SOC_PMCTRL_G3DPMUSEL_g3d_pmu_sel_START (0)
#define SOC_PMCTRL_G3DPMUSEL_g3d_pmu_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_vol_up_step_time : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_PMCTRL_G3DVOLUPSTEPTIME_UNION;
#endif
#define SOC_PMCTRL_G3DVOLUPSTEPTIME_g3d_vol_up_step_time_START (0)
#define SOC_PMCTRL_G3DVOLUPSTEPTIME_g3d_vol_up_step_time_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_vol_dn_step_time : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_PMCTRL_G3DVOLDNSTEPTIME_UNION;
#endif
#define SOC_PMCTRL_G3DVOLDNSTEPTIME_g3d_vol_dn_step_time_START (0)
#define SOC_PMCTRL_G3DVOLDNSTEPTIME_g3d_vol_dn_step_time_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_dvfs_phase_up_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_G3DPHASEUPTIME_UNION;
#endif
#define SOC_PMCTRL_G3DPHASEUPTIME_g3d_dvfs_phase_up_time_START (0)
#define SOC_PMCTRL_G3DPHASEUPTIME_g3d_dvfs_phase_up_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_dvfs_phase_dn_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_G3DPHASEDNTIME_UNION;
#endif
#define SOC_PMCTRL_G3DPHASEDNTIME_g3d_dvfs_phase_dn_time_START (0)
#define SOC_PMCTRL_G3DPHASEDNTIME_g3d_dvfs_phase_dn_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_pmu_vol_hold_time : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_G3DPMUHOLDTIME_UNION;
#endif
#define SOC_PMCTRL_G3DPMUHOLDTIME_g3d_pmu_vol_hold_time_START (0)
#define SOC_PMCTRL_G3DPMUHOLDTIME_g3d_pmu_vol_hold_time_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_hpm0_bypass : 1;
        unsigned int g3d_hpm1_bypass : 1;
        unsigned int g3d_hpm2_bypass : 1;
        unsigned int g3d_hpm3_bypass : 1;
        unsigned int g3d_hpm_dly_cnt : 12;
        unsigned int g3d_idle_clkdiv_bypass : 1;
        unsigned int reserved_0 : 3;
        unsigned int g3d_idle_clk_div : 6;
        unsigned int reserved_1 : 6;
    } reg;
} SOC_PMCTRL_G3DHPMBYPASS_UNION;
#endif
#define SOC_PMCTRL_G3DHPMBYPASS_g3d_hpm0_bypass_START (0)
#define SOC_PMCTRL_G3DHPMBYPASS_g3d_hpm0_bypass_END (0)
#define SOC_PMCTRL_G3DHPMBYPASS_g3d_hpm1_bypass_START (1)
#define SOC_PMCTRL_G3DHPMBYPASS_g3d_hpm1_bypass_END (1)
#define SOC_PMCTRL_G3DHPMBYPASS_g3d_hpm2_bypass_START (2)
#define SOC_PMCTRL_G3DHPMBYPASS_g3d_hpm2_bypass_END (2)
#define SOC_PMCTRL_G3DHPMBYPASS_g3d_hpm3_bypass_START (3)
#define SOC_PMCTRL_G3DHPMBYPASS_g3d_hpm3_bypass_END (3)
#define SOC_PMCTRL_G3DHPMBYPASS_g3d_hpm_dly_cnt_START (4)
#define SOC_PMCTRL_G3DHPMBYPASS_g3d_hpm_dly_cnt_END (15)
#define SOC_PMCTRL_G3DHPMBYPASS_g3d_idle_clkdiv_bypass_START (16)
#define SOC_PMCTRL_G3DHPMBYPASS_g3d_idle_clkdiv_bypass_END (16)
#define SOC_PMCTRL_G3DHPMBYPASS_g3d_idle_clk_div_START (20)
#define SOC_PMCTRL_G3DHPMBYPASS_g3d_idle_clk_div_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_auto_clkdiv_bypass : 1;
        unsigned int g3d_auto_clkdiv_time : 7;
        unsigned int g3d_pwr_dly_cnt : 8;
        unsigned int g3d_auto_clk_div : 2;
        unsigned int g3d_div_debounce : 6;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_UNION;
#endif
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_auto_clkdiv_bypass_START (0)
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_auto_clkdiv_bypass_END (0)
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_auto_clkdiv_time_START (1)
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_auto_clkdiv_time_END (7)
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_pwr_dly_cnt_START (8)
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_pwr_dly_cnt_END (15)
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_auto_clk_div_START (16)
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_auto_clk_div_END (17)
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_div_debounce_START (18)
#define SOC_PMCTRL_G3DAUTOCLKDIVBYPASS_g3d_div_debounce_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_avs_en : 1;
        unsigned int g3d_avs_pwctrl_en : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_G3DAVSEN_UNION;
#endif
#define SOC_PMCTRL_G3DAVSEN_g3d_avs_en_START (0)
#define SOC_PMCTRL_G3DAVSEN_g3d_avs_en_END (0)
#define SOC_PMCTRL_G3DAVSEN_g3d_avs_pwctrl_en_START (1)
#define SOC_PMCTRL_G3DAVSEN_g3d_avs_pwctrl_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_avspara_mod : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_G3DAVSPARAMOD_UNION;
#endif
#define SOC_PMCTRL_G3DAVSPARAMOD_g3d_avspara_mod_START (0)
#define SOC_PMCTRL_G3DAVSPARAMOD_g3d_avspara_mod_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 2;
        unsigned int g3d_avs_opc_offset : 10;
        unsigned int g3d_avs_rcc : 5;
        unsigned int g3d_avs_irgap : 5;
        unsigned int g3d_avs_opc_mod : 2;
        unsigned int g3d_avs_hpm_sel : 3;
        unsigned int g3d_avs_opc_shift : 3;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_PMCTRL_G3DAVSPARA0_UNION;
#endif
#define SOC_PMCTRL_G3DAVSPARA0_g3d_avs_opc_offset_START (2)
#define SOC_PMCTRL_G3DAVSPARA0_g3d_avs_opc_offset_END (11)
#define SOC_PMCTRL_G3DAVSPARA0_g3d_avs_rcc_START (12)
#define SOC_PMCTRL_G3DAVSPARA0_g3d_avs_rcc_END (16)
#define SOC_PMCTRL_G3DAVSPARA0_g3d_avs_irgap_START (17)
#define SOC_PMCTRL_G3DAVSPARA0_g3d_avs_irgap_END (21)
#define SOC_PMCTRL_G3DAVSPARA0_g3d_avs_opc_mod_START (22)
#define SOC_PMCTRL_G3DAVSPARA0_g3d_avs_opc_mod_END (23)
#define SOC_PMCTRL_G3DAVSPARA0_g3d_avs_hpm_sel_START (24)
#define SOC_PMCTRL_G3DAVSPARA0_g3d_avs_hpm_sel_END (26)
#define SOC_PMCTRL_G3DAVSPARA0_g3d_avs_opc_shift_START (27)
#define SOC_PMCTRL_G3DAVSPARA0_g3d_avs_opc_shift_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_avs_vol_up_gain1 : 4;
        unsigned int g3d_avs_vol_up_gain2 : 4;
        unsigned int g3d_avs_vol_up_gain3 : 4;
        unsigned int g3d_avs_vol_up_gain4 : 4;
        unsigned int g3d_avs_vol_up_gain5 : 4;
        unsigned int g3d_avs_vol_up_gain6 : 4;
        unsigned int g3d_avs_vol_up_gain7 : 4;
        unsigned int g3d_avs_vol_up_gain8 : 4;
    } reg;
} SOC_PMCTRL_G3DAVSPARA1_UNION;
#endif
#define SOC_PMCTRL_G3DAVSPARA1_g3d_avs_vol_up_gain1_START (0)
#define SOC_PMCTRL_G3DAVSPARA1_g3d_avs_vol_up_gain1_END (3)
#define SOC_PMCTRL_G3DAVSPARA1_g3d_avs_vol_up_gain2_START (4)
#define SOC_PMCTRL_G3DAVSPARA1_g3d_avs_vol_up_gain2_END (7)
#define SOC_PMCTRL_G3DAVSPARA1_g3d_avs_vol_up_gain3_START (8)
#define SOC_PMCTRL_G3DAVSPARA1_g3d_avs_vol_up_gain3_END (11)
#define SOC_PMCTRL_G3DAVSPARA1_g3d_avs_vol_up_gain4_START (12)
#define SOC_PMCTRL_G3DAVSPARA1_g3d_avs_vol_up_gain4_END (15)
#define SOC_PMCTRL_G3DAVSPARA1_g3d_avs_vol_up_gain5_START (16)
#define SOC_PMCTRL_G3DAVSPARA1_g3d_avs_vol_up_gain5_END (19)
#define SOC_PMCTRL_G3DAVSPARA1_g3d_avs_vol_up_gain6_START (20)
#define SOC_PMCTRL_G3DAVSPARA1_g3d_avs_vol_up_gain6_END (23)
#define SOC_PMCTRL_G3DAVSPARA1_g3d_avs_vol_up_gain7_START (24)
#define SOC_PMCTRL_G3DAVSPARA1_g3d_avs_vol_up_gain7_END (27)
#define SOC_PMCTRL_G3DAVSPARA1_g3d_avs_vol_up_gain8_START (28)
#define SOC_PMCTRL_G3DAVSPARA1_g3d_avs_vol_up_gain8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_avs_vol_up_gain9 : 4;
        unsigned int g3d_avs_vol_up_gain10 : 4;
        unsigned int g3d_avs_vol_up_gain11 : 4;
        unsigned int g3d_avs_vol_up_gain12 : 4;
        unsigned int g3d_avs_vol_up_gain13 : 4;
        unsigned int g3d_avs_vol_up_gain14 : 4;
        unsigned int g3d_avs_vol_up_gain15 : 7;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_G3DAVSPARA2_UNION;
#endif
#define SOC_PMCTRL_G3DAVSPARA2_g3d_avs_vol_up_gain9_START (0)
#define SOC_PMCTRL_G3DAVSPARA2_g3d_avs_vol_up_gain9_END (3)
#define SOC_PMCTRL_G3DAVSPARA2_g3d_avs_vol_up_gain10_START (4)
#define SOC_PMCTRL_G3DAVSPARA2_g3d_avs_vol_up_gain10_END (7)
#define SOC_PMCTRL_G3DAVSPARA2_g3d_avs_vol_up_gain11_START (8)
#define SOC_PMCTRL_G3DAVSPARA2_g3d_avs_vol_up_gain11_END (11)
#define SOC_PMCTRL_G3DAVSPARA2_g3d_avs_vol_up_gain12_START (12)
#define SOC_PMCTRL_G3DAVSPARA2_g3d_avs_vol_up_gain12_END (15)
#define SOC_PMCTRL_G3DAVSPARA2_g3d_avs_vol_up_gain13_START (16)
#define SOC_PMCTRL_G3DAVSPARA2_g3d_avs_vol_up_gain13_END (19)
#define SOC_PMCTRL_G3DAVSPARA2_g3d_avs_vol_up_gain14_START (20)
#define SOC_PMCTRL_G3DAVSPARA2_g3d_avs_vol_up_gain14_END (23)
#define SOC_PMCTRL_G3DAVSPARA2_g3d_avs_vol_up_gain15_START (24)
#define SOC_PMCTRL_G3DAVSPARA2_g3d_avs_vol_up_gain15_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_avs_vol_dn_gain1 : 4;
        unsigned int g3d_avs_vol_dn_gain2 : 4;
        unsigned int g3d_avs_vol_dn_gain3 : 4;
        unsigned int g3d_avs_vol_dn_gain4 : 4;
        unsigned int g3d_avs_vol_dn_gain5 : 4;
        unsigned int g3d_avs_vol_dn_gain6 : 4;
        unsigned int g3d_avs_vol_dn_gain7 : 4;
        unsigned int g3dc_avs_vol_dn_gain8 : 4;
    } reg;
} SOC_PMCTRL_G3DAVSPARA3_UNION;
#endif
#define SOC_PMCTRL_G3DAVSPARA3_g3d_avs_vol_dn_gain1_START (0)
#define SOC_PMCTRL_G3DAVSPARA3_g3d_avs_vol_dn_gain1_END (3)
#define SOC_PMCTRL_G3DAVSPARA3_g3d_avs_vol_dn_gain2_START (4)
#define SOC_PMCTRL_G3DAVSPARA3_g3d_avs_vol_dn_gain2_END (7)
#define SOC_PMCTRL_G3DAVSPARA3_g3d_avs_vol_dn_gain3_START (8)
#define SOC_PMCTRL_G3DAVSPARA3_g3d_avs_vol_dn_gain3_END (11)
#define SOC_PMCTRL_G3DAVSPARA3_g3d_avs_vol_dn_gain4_START (12)
#define SOC_PMCTRL_G3DAVSPARA3_g3d_avs_vol_dn_gain4_END (15)
#define SOC_PMCTRL_G3DAVSPARA3_g3d_avs_vol_dn_gain5_START (16)
#define SOC_PMCTRL_G3DAVSPARA3_g3d_avs_vol_dn_gain5_END (19)
#define SOC_PMCTRL_G3DAVSPARA3_g3d_avs_vol_dn_gain6_START (20)
#define SOC_PMCTRL_G3DAVSPARA3_g3d_avs_vol_dn_gain6_END (23)
#define SOC_PMCTRL_G3DAVSPARA3_g3d_avs_vol_dn_gain7_START (24)
#define SOC_PMCTRL_G3DAVSPARA3_g3d_avs_vol_dn_gain7_END (27)
#define SOC_PMCTRL_G3DAVSPARA3_g3dc_avs_vol_dn_gain8_START (28)
#define SOC_PMCTRL_G3DAVSPARA3_g3dc_avs_vol_dn_gain8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_avs_vol_dn_gain9 : 4;
        unsigned int g3d_avs_vol_dn_gain10 : 4;
        unsigned int g3d_avs_vol_dn_gain11 : 4;
        unsigned int g3d_avs_vol_dn_gain12 : 4;
        unsigned int g3d_avs_vol_dn_gain13 : 4;
        unsigned int g3d_avs_vol_dn_gain14 : 4;
        unsigned int g3d_avs_vol_dn_gain15 : 7;
        unsigned int reserved : 1;
    } reg;
} SOC_PMCTRL_G3DAVSPARA4_UNION;
#endif
#define SOC_PMCTRL_G3DAVSPARA4_g3d_avs_vol_dn_gain9_START (0)
#define SOC_PMCTRL_G3DAVSPARA4_g3d_avs_vol_dn_gain9_END (3)
#define SOC_PMCTRL_G3DAVSPARA4_g3d_avs_vol_dn_gain10_START (4)
#define SOC_PMCTRL_G3DAVSPARA4_g3d_avs_vol_dn_gain10_END (7)
#define SOC_PMCTRL_G3DAVSPARA4_g3d_avs_vol_dn_gain11_START (8)
#define SOC_PMCTRL_G3DAVSPARA4_g3d_avs_vol_dn_gain11_END (11)
#define SOC_PMCTRL_G3DAVSPARA4_g3d_avs_vol_dn_gain12_START (12)
#define SOC_PMCTRL_G3DAVSPARA4_g3d_avs_vol_dn_gain12_END (15)
#define SOC_PMCTRL_G3DAVSPARA4_g3d_avs_vol_dn_gain13_START (16)
#define SOC_PMCTRL_G3DAVSPARA4_g3d_avs_vol_dn_gain13_END (19)
#define SOC_PMCTRL_G3DAVSPARA4_g3d_avs_vol_dn_gain14_START (20)
#define SOC_PMCTRL_G3DAVSPARA4_g3d_avs_vol_dn_gain14_END (23)
#define SOC_PMCTRL_G3DAVSPARA4_g3d_avs_vol_dn_gain15_START (24)
#define SOC_PMCTRL_G3DAVSPARA4_g3d_avs_vol_dn_gain15_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_avs_max_vol : 7;
        unsigned int g3d_avs_min_vol : 7;
        unsigned int reserved : 18;
    } reg;
} SOC_PMCTRL_G3DAVSPARA5_UNION;
#endif
#define SOC_PMCTRL_G3DAVSPARA5_g3d_avs_max_vol_START (0)
#define SOC_PMCTRL_G3DAVSPARA5_g3d_avs_max_vol_END (6)
#define SOC_PMCTRL_G3DAVSPARA5_g3d_avs_min_vol_START (7)
#define SOC_PMCTRL_G3DAVSPARA5_g3d_avs_min_vol_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_avs_sample_num : 22;
        unsigned int reserved : 10;
    } reg;
} SOC_PMCTRL_G3DAVSPARA6_UNION;
#endif
#define SOC_PMCTRL_G3DAVSPARA6_g3d_avs_sample_num_START (0)
#define SOC_PMCTRL_G3DAVSPARA6_g3d_avs_sample_num_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_hpm_type : 1;
        unsigned int g3ds_hpm0_mask : 1;
        unsigned int g3ds_hpm1_mask : 1;
        unsigned int g3ds_hpm2_mask : 1;
        unsigned int g3ds_hpm3_mask : 1;
        unsigned int g3dg_hpm_mask : 1;
        unsigned int reserved : 10;
        unsigned int biten : 16;
    } reg;
} SOC_PMCTRL_G3DHPMTYP_UNION;
#endif
#define SOC_PMCTRL_G3DHPMTYP_g3d_hpm_type_START (0)
#define SOC_PMCTRL_G3DHPMTYP_g3d_hpm_type_END (0)
#define SOC_PMCTRL_G3DHPMTYP_g3ds_hpm0_mask_START (1)
#define SOC_PMCTRL_G3DHPMTYP_g3ds_hpm0_mask_END (1)
#define SOC_PMCTRL_G3DHPMTYP_g3ds_hpm1_mask_START (2)
#define SOC_PMCTRL_G3DHPMTYP_g3ds_hpm1_mask_END (2)
#define SOC_PMCTRL_G3DHPMTYP_g3ds_hpm2_mask_START (3)
#define SOC_PMCTRL_G3DHPMTYP_g3ds_hpm2_mask_END (3)
#define SOC_PMCTRL_G3DHPMTYP_g3ds_hpm3_mask_START (4)
#define SOC_PMCTRL_G3DHPMTYP_g3ds_hpm3_mask_END (4)
#define SOC_PMCTRL_G3DHPMTYP_g3dg_hpm_mask_START (5)
#define SOC_PMCTRL_G3DHPMTYP_g3dg_hpm_mask_END (5)
#define SOC_PMCTRL_G3DHPMTYP_biten_START (16)
#define SOC_PMCTRL_G3DHPMTYP_biten_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3ds_hpm0_en : 1;
        unsigned int g3ds_hpm1_en : 1;
        unsigned int g3dg_hpm_en : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_PMCTRL_G3DHPMEN_UNION;
#endif
#define SOC_PMCTRL_G3DHPMEN_g3ds_hpm0_en_START (0)
#define SOC_PMCTRL_G3DHPMEN_g3ds_hpm0_en_END (0)
#define SOC_PMCTRL_G3DHPMEN_g3ds_hpm1_en_START (1)
#define SOC_PMCTRL_G3DHPMEN_g3ds_hpm1_en_END (1)
#define SOC_PMCTRL_G3DHPMEN_g3dg_hpm_en_START (2)
#define SOC_PMCTRL_G3DHPMEN_g3dg_hpm_en_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3ds_hpmx0_en : 1;
        unsigned int g3ds_hpmx1_en : 1;
        unsigned int g3dg_hpmx_en : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_PMCTRL_G3DHPMXEN_UNION;
#endif
#define SOC_PMCTRL_G3DHPMXEN_g3ds_hpmx0_en_START (0)
#define SOC_PMCTRL_G3DHPMXEN_g3ds_hpmx0_en_END (0)
#define SOC_PMCTRL_G3DHPMXEN_g3ds_hpmx1_en_START (1)
#define SOC_PMCTRL_G3DHPMXEN_g3ds_hpmx1_en_END (1)
#define SOC_PMCTRL_G3DHPMXEN_g3dg_hpmx_en_START (2)
#define SOC_PMCTRL_G3DHPMXEN_g3dg_hpmx_en_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3ds_hpm0_opc_vld : 1;
        unsigned int g3ds_hpm1_opc_vld : 1;
        unsigned int g3dg_hpm_opc_vld : 1;
        unsigned int g3ds_hpmx0_opc_vld : 1;
        unsigned int g3ds_hpmx1_opc_vld : 1;
        unsigned int g3dg_hpmx_opc_vld : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_PMCTRL_G3DHPMOPCVALID_UNION;
#endif
#define SOC_PMCTRL_G3DHPMOPCVALID_g3ds_hpm0_opc_vld_START (0)
#define SOC_PMCTRL_G3DHPMOPCVALID_g3ds_hpm0_opc_vld_END (0)
#define SOC_PMCTRL_G3DHPMOPCVALID_g3ds_hpm1_opc_vld_START (1)
#define SOC_PMCTRL_G3DHPMOPCVALID_g3ds_hpm1_opc_vld_END (1)
#define SOC_PMCTRL_G3DHPMOPCVALID_g3dg_hpm_opc_vld_START (2)
#define SOC_PMCTRL_G3DHPMOPCVALID_g3dg_hpm_opc_vld_END (2)
#define SOC_PMCTRL_G3DHPMOPCVALID_g3ds_hpmx0_opc_vld_START (3)
#define SOC_PMCTRL_G3DHPMOPCVALID_g3ds_hpmx0_opc_vld_END (3)
#define SOC_PMCTRL_G3DHPMOPCVALID_g3ds_hpmx1_opc_vld_START (4)
#define SOC_PMCTRL_G3DHPMOPCVALID_g3ds_hpmx1_opc_vld_END (4)
#define SOC_PMCTRL_G3DHPMOPCVALID_g3dg_hpmx_opc_vld_START (5)
#define SOC_PMCTRL_G3DHPMOPCVALID_g3dg_hpmx_opc_vld_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3ds_hpm0_opc : 10;
        unsigned int g3ds_hpm1_opc : 10;
        unsigned int g3dg_hpm_opc : 10;
        unsigned int reserved : 2;
    } reg;
} SOC_PMCTRL_G3DHPMOPC_UNION;
#endif
#define SOC_PMCTRL_G3DHPMOPC_g3ds_hpm0_opc_START (0)
#define SOC_PMCTRL_G3DHPMOPC_g3ds_hpm0_opc_END (9)
#define SOC_PMCTRL_G3DHPMOPC_g3ds_hpm1_opc_START (10)
#define SOC_PMCTRL_G3DHPMOPC_g3ds_hpm1_opc_END (19)
#define SOC_PMCTRL_G3DHPMOPC_g3dg_hpm_opc_START (20)
#define SOC_PMCTRL_G3DHPMOPC_g3dg_hpm_opc_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3ds_hpmx0_opc : 10;
        unsigned int g3ds_hpmx1_opc : 10;
        unsigned int g3dg_hpmx_opc : 10;
        unsigned int reserved : 2;
    } reg;
} SOC_PMCTRL_G3DHPMXOPC_UNION;
#endif
#define SOC_PMCTRL_G3DHPMXOPC_g3ds_hpmx0_opc_START (0)
#define SOC_PMCTRL_G3DHPMXOPC_g3ds_hpmx0_opc_END (9)
#define SOC_PMCTRL_G3DHPMXOPC_g3ds_hpmx1_opc_START (10)
#define SOC_PMCTRL_G3DHPMXOPC_g3ds_hpmx1_opc_END (19)
#define SOC_PMCTRL_G3DHPMXOPC_g3dg_hpmx_opc_START (20)
#define SOC_PMCTRL_G3DHPMXOPC_g3dg_hpmx_opc_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3ds_hpm0_pc_avs : 10;
        unsigned int g3ds_hpm1_pc_avs : 10;
        unsigned int g3dg_hpm_pc_avs : 10;
        unsigned int reserved : 2;
    } reg;
} SOC_PMCTRL_G3DHPMPC_UNION;
#endif
#define SOC_PMCTRL_G3DHPMPC_g3ds_hpm0_pc_avs_START (0)
#define SOC_PMCTRL_G3DHPMPC_g3ds_hpm0_pc_avs_END (9)
#define SOC_PMCTRL_G3DHPMPC_g3ds_hpm1_pc_avs_START (10)
#define SOC_PMCTRL_G3DHPMPC_g3ds_hpm1_pc_avs_END (19)
#define SOC_PMCTRL_G3DHPMPC_g3dg_hpm_pc_avs_START (20)
#define SOC_PMCTRL_G3DHPMPC_g3dg_hpm_pc_avs_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3ds_hpm2_en : 1;
        unsigned int g3ds_hpm3_en : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_G3DHPMEN1_UNION;
#endif
#define SOC_PMCTRL_G3DHPMEN1_g3ds_hpm2_en_START (0)
#define SOC_PMCTRL_G3DHPMEN1_g3ds_hpm2_en_END (0)
#define SOC_PMCTRL_G3DHPMEN1_g3ds_hpm3_en_START (1)
#define SOC_PMCTRL_G3DHPMEN1_g3ds_hpm3_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3ds_hpmx2_en : 1;
        unsigned int g3ds_hpmx3_en : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_G3DHPMXEN1_UNION;
#endif
#define SOC_PMCTRL_G3DHPMXEN1_g3ds_hpmx2_en_START (0)
#define SOC_PMCTRL_G3DHPMXEN1_g3ds_hpmx2_en_END (0)
#define SOC_PMCTRL_G3DHPMXEN1_g3ds_hpmx3_en_START (1)
#define SOC_PMCTRL_G3DHPMXEN1_g3ds_hpmx3_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3ds_hpm2_opc_vld : 1;
        unsigned int g3ds_hpm3_opc_vld : 1;
        unsigned int g3ds_hpmx2_opc_vld : 1;
        unsigned int g3ds_hpmx3_opc_vld : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_PMCTRL_G3DHPMOPCVALID1_UNION;
#endif
#define SOC_PMCTRL_G3DHPMOPCVALID1_g3ds_hpm2_opc_vld_START (0)
#define SOC_PMCTRL_G3DHPMOPCVALID1_g3ds_hpm2_opc_vld_END (0)
#define SOC_PMCTRL_G3DHPMOPCVALID1_g3ds_hpm3_opc_vld_START (1)
#define SOC_PMCTRL_G3DHPMOPCVALID1_g3ds_hpm3_opc_vld_END (1)
#define SOC_PMCTRL_G3DHPMOPCVALID1_g3ds_hpmx2_opc_vld_START (2)
#define SOC_PMCTRL_G3DHPMOPCVALID1_g3ds_hpmx2_opc_vld_END (2)
#define SOC_PMCTRL_G3DHPMOPCVALID1_g3ds_hpmx3_opc_vld_START (3)
#define SOC_PMCTRL_G3DHPMOPCVALID1_g3ds_hpmx3_opc_vld_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3ds_hpm2_opc : 10;
        unsigned int g3ds_hpm3_opc : 10;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_G3DHPMOPC1_UNION;
#endif
#define SOC_PMCTRL_G3DHPMOPC1_g3ds_hpm2_opc_START (0)
#define SOC_PMCTRL_G3DHPMOPC1_g3ds_hpm2_opc_END (9)
#define SOC_PMCTRL_G3DHPMOPC1_g3ds_hpm3_opc_START (10)
#define SOC_PMCTRL_G3DHPMOPC1_g3ds_hpm3_opc_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3ds_hpmx2_opc : 10;
        unsigned int g3ds_hpmx3_opc : 10;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_G3DHPMXOPC1_UNION;
#endif
#define SOC_PMCTRL_G3DHPMXOPC1_g3ds_hpmx2_opc_START (0)
#define SOC_PMCTRL_G3DHPMXOPC1_g3ds_hpmx2_opc_END (9)
#define SOC_PMCTRL_G3DHPMXOPC1_g3ds_hpmx3_opc_START (10)
#define SOC_PMCTRL_G3DHPMXOPC1_g3ds_hpmx3_opc_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3ds_hpm2_pc_avs : 10;
        unsigned int g3ds_hpm3_pc_avs : 10;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_G3DHPMPC1_UNION;
#endif
#define SOC_PMCTRL_G3DHPMPC1_g3ds_hpm2_pc_avs_START (0)
#define SOC_PMCTRL_G3DHPMPC1_g3ds_hpm2_pc_avs_END (9)
#define SOC_PMCTRL_G3DHPMPC1_g3ds_hpm3_pc_avs_START (10)
#define SOC_PMCTRL_G3DHPMPC1_g3ds_hpm3_pc_avs_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3ds_hpm0_mask_stat : 1;
        unsigned int g3ds_hpm1_mask_stat : 1;
        unsigned int g3ds_hpm2_mask_stat : 1;
        unsigned int g3ds_hpm3_mask_stat : 1;
        unsigned int g3dg_hpm_mask_stat : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_PMCTRL_G3DHPMMASKSTAT_UNION;
#endif
#define SOC_PMCTRL_G3DHPMMASKSTAT_g3ds_hpm0_mask_stat_START (0)
#define SOC_PMCTRL_G3DHPMMASKSTAT_g3ds_hpm0_mask_stat_END (0)
#define SOC_PMCTRL_G3DHPMMASKSTAT_g3ds_hpm1_mask_stat_START (1)
#define SOC_PMCTRL_G3DHPMMASKSTAT_g3ds_hpm1_mask_stat_END (1)
#define SOC_PMCTRL_G3DHPMMASKSTAT_g3ds_hpm2_mask_stat_START (2)
#define SOC_PMCTRL_G3DHPMMASKSTAT_g3ds_hpm2_mask_stat_END (2)
#define SOC_PMCTRL_G3DHPMMASKSTAT_g3ds_hpm3_mask_stat_START (3)
#define SOC_PMCTRL_G3DHPMMASKSTAT_g3ds_hpm3_mask_stat_END (3)
#define SOC_PMCTRL_G3DHPMMASKSTAT_g3dg_hpm_mask_stat_START (4)
#define SOC_PMCTRL_G3DHPMMASKSTAT_g3dg_hpm_mask_stat_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_avs_hpm_div : 6;
        unsigned int reserved : 26;
    } reg;
} SOC_PMCTRL_G3DHPMCLKDIV_UNION;
#endif
#define SOC_PMCTRL_G3DHPMCLKDIV_g3d_avs_hpm_div_START (0)
#define SOC_PMCTRL_G3DHPMCLKDIV_g3d_avs_hpm_div_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_avs_vol_idx : 8;
        unsigned int g3d_vol_chg_sftreq : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_PMCTRL_G3DAVSVOLIDX_UNION;
#endif
#define SOC_PMCTRL_G3DAVSVOLIDX_g3d_avs_vol_idx_START (0)
#define SOC_PMCTRL_G3DAVSVOLIDX_g3d_avs_vol_idx_END (7)
#define SOC_PMCTRL_G3DAVSVOLIDX_g3d_vol_chg_sftreq_START (8)
#define SOC_PMCTRL_G3DAVSVOLIDX_g3d_vol_chg_sftreq_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddrc_a_csysreq : 1;
        unsigned int ddrc_b_csysreq : 1;
        unsigned int ddrc_c_csysreq : 1;
        unsigned int ddrc_d_csysreq : 1;
        unsigned int ddrc_a_csysack : 1;
        unsigned int ddrc_b_csysack : 1;
        unsigned int ddrc_c_csysack : 1;
        unsigned int ddrc_d_csysack : 1;
        unsigned int reserved : 24;
    } reg;
} SOC_PMCTRL_DDRLPCTRL_UNION;
#endif
#define SOC_PMCTRL_DDRLPCTRL_ddrc_a_csysreq_START (0)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_a_csysreq_END (0)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_b_csysreq_START (1)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_b_csysreq_END (1)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_c_csysreq_START (2)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_c_csysreq_END (2)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_d_csysreq_START (3)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_d_csysreq_END (3)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_a_csysack_START (4)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_a_csysack_END (4)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_b_csysack_START (5)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_b_csysack_END (5)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_c_csysack_START (6)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_c_csysack_END (6)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_d_csysack_START (7)
#define SOC_PMCTRL_DDRLPCTRL_ddrc_d_csysack_END (7)
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
        unsigned int a53_cpu_avs_round : 8;
        unsigned int a57_cpu_avs_round : 8;
        unsigned int g3d_avs_round : 8;
        unsigned int reserved : 8;
    } reg;
} SOC_PMCTRL_AVSRUNROUND_UNION;
#endif
#define SOC_PMCTRL_AVSRUNROUND_a53_cpu_avs_round_START (0)
#define SOC_PMCTRL_AVSRUNROUND_a53_cpu_avs_round_END (7)
#define SOC_PMCTRL_AVSRUNROUND_a57_cpu_avs_round_START (8)
#define SOC_PMCTRL_AVSRUNROUND_a57_cpu_avs_round_END (15)
#define SOC_PMCTRL_AVSRUNROUND_g3d_avs_round_START (16)
#define SOC_PMCTRL_AVSRUNROUND_g3d_avs_round_END (23)
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
        unsigned int dbg_ctrl_ssi : 1;
        unsigned int dbg_ctrl_i2c : 1;
        unsigned int dbg_ctrl_ssi1 : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_PMCTRL_PERI_CTRL0_UNION;
#endif
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_ddrc_START (0)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_ddrc_END (0)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_cssys_START (1)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_cssys_END (1)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_ssi_START (2)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_ssi_END (2)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_i2c_START (3)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_i2c_END (3)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_ssi1_START (4)
#define SOC_PMCTRL_PERI_CTRL0_dbg_ctrl_ssi1_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_CTRL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pmu_vol_addr0 : 10;
        unsigned int reserved_0 : 6;
        unsigned int pmu_vol_addr1 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} SOC_PMCTRL_PERI_CTRL2_UNION;
#endif
#define SOC_PMCTRL_PERI_CTRL2_pmu_vol_addr0_START (0)
#define SOC_PMCTRL_PERI_CTRL2_pmu_vol_addr0_END (9)
#define SOC_PMCTRL_PERI_CTRL2_pmu_vol_addr1_START (16)
#define SOC_PMCTRL_PERI_CTRL2_pmu_vol_addr1_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pmu_vol_addr2 : 10;
        unsigned int reserved_0 : 6;
        unsigned int pmu_vol_addr3 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} SOC_PMCTRL_PERI_CTRL3_UNION;
#endif
#define SOC_PMCTRL_PERI_CTRL3_pmu_vol_addr2_START (0)
#define SOC_PMCTRL_PERI_CTRL3_pmu_vol_addr2_END (9)
#define SOC_PMCTRL_PERI_CTRL3_pmu_vol_addr3_START (16)
#define SOC_PMCTRL_PERI_CTRL3_pmu_vol_addr3_END (25)
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
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_CTRL6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_CTRL7_UNION;
#endif
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
        unsigned int dbg_info_i2c : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT3_UNION;
#endif
#define SOC_PMCTRL_PERI_STAT3_dbg_info_i2c_START (0)
#define SOC_PMCTRL_PERI_STAT3_dbg_info_i2c_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_info_ssi1 : 32;
    } reg;
} SOC_PMCTRL_PERI_STAT4_UNION;
#endif
#define SOC_PMCTRL_PERI_STAT4_dbg_info_ssi1_START (0)
#define SOC_PMCTRL_PERI_STAT4_dbg_info_ssi1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_test_out_0 : 4;
        unsigned int svfd_test_out_1 : 4;
        unsigned int svfd_test_out_2 : 4;
        unsigned int svfd_match_result_0 : 1;
        unsigned int svfd_match_result_1 : 1;
        unsigned int svfd_match_result_2 : 1;
        unsigned int reserved : 17;
    } reg;
} SOC_PMCTRL_PERI_STAT5_UNION;
#endif
#define SOC_PMCTRL_PERI_STAT5_svfd_test_out_0_START (0)
#define SOC_PMCTRL_PERI_STAT5_svfd_test_out_0_END (3)
#define SOC_PMCTRL_PERI_STAT5_svfd_test_out_1_START (4)
#define SOC_PMCTRL_PERI_STAT5_svfd_test_out_1_END (7)
#define SOC_PMCTRL_PERI_STAT5_svfd_test_out_2_START (8)
#define SOC_PMCTRL_PERI_STAT5_svfd_test_out_2_END (11)
#define SOC_PMCTRL_PERI_STAT5_svfd_match_result_0_START (12)
#define SOC_PMCTRL_PERI_STAT5_svfd_match_result_0_END (12)
#define SOC_PMCTRL_PERI_STAT5_svfd_match_result_1_START (13)
#define SOC_PMCTRL_PERI_STAT5_svfd_match_result_1_END (13)
#define SOC_PMCTRL_PERI_STAT5_svfd_match_result_2_START (14)
#define SOC_PMCTRL_PERI_STAT5_svfd_match_result_2_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cpm_data : 16;
        unsigned int maia_cpm_data : 16;
    } reg;
} SOC_PMCTRL_PERI_STAT6_UNION;
#endif
#define SOC_PMCTRL_PERI_STAT6_a53_cpm_data_START (0)
#define SOC_PMCTRL_PERI_STAT6_a53_cpm_data_END (15)
#define SOC_PMCTRL_PERI_STAT6_maia_cpm_data_START (16)
#define SOC_PMCTRL_PERI_STAT6_maia_cpm_data_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int maia_cpm_data_vld : 1;
        unsigned int a53_cpm_data_vld : 1;
        unsigned int g3d_cpm_data_vld : 1;
        unsigned int reserved_0 : 1;
        unsigned int maia_svfd_glitch_result : 1;
        unsigned int a53_svfd_glitch_result : 1;
        unsigned int g3d_svfd_glitch_result : 1;
        unsigned int reserved_1 : 9;
        unsigned int g3d_cpm_data : 16;
    } reg;
} SOC_PMCTRL_PERI_STAT7_UNION;
#endif
#define SOC_PMCTRL_PERI_STAT7_maia_cpm_data_vld_START (0)
#define SOC_PMCTRL_PERI_STAT7_maia_cpm_data_vld_END (0)
#define SOC_PMCTRL_PERI_STAT7_a53_cpm_data_vld_START (1)
#define SOC_PMCTRL_PERI_STAT7_a53_cpm_data_vld_END (1)
#define SOC_PMCTRL_PERI_STAT7_g3d_cpm_data_vld_START (2)
#define SOC_PMCTRL_PERI_STAT7_g3d_cpm_data_vld_END (2)
#define SOC_PMCTRL_PERI_STAT7_maia_svfd_glitch_result_START (4)
#define SOC_PMCTRL_PERI_STAT7_maia_svfd_glitch_result_END (4)
#define SOC_PMCTRL_PERI_STAT7_a53_svfd_glitch_result_START (5)
#define SOC_PMCTRL_PERI_STAT7_a53_svfd_glitch_result_END (5)
#define SOC_PMCTRL_PERI_STAT7_g3d_svfd_glitch_result_START (6)
#define SOC_PMCTRL_PERI_STAT7_g3d_svfd_glitch_result_END (6)
#define SOC_PMCTRL_PERI_STAT7_g3d_cpm_data_START (16)
#define SOC_PMCTRL_PERI_STAT7_g3d_cpm_data_END (31)
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
        unsigned int noc_vcodec_power_idlereq : 1;
        unsigned int noc_isp_power_idlereq : 1;
        unsigned int noc_sysbus_power_idlereq : 1;
        unsigned int noc_cfgbus_power_idlereq : 1;
        unsigned int noc_dmabus_power_idlereq : 1;
        unsigned int noc_dbgbus_power_idlereq : 1;
        unsigned int noc_vdec_power_idlereq : 1;
        unsigned int noc_venc_power_idlereq : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_dss_power_idlereq : 1;
        unsigned int noc_ivp32_peri_bus_power_idlereq : 1;
        unsigned int reserved_1 : 1;
        unsigned int biten : 16;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEREQ_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_ddrphy_bypass_mode_START (0)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_ddrphy_bypass_mode_END (0)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_mmc0bus_power_idlereq_START (1)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_mmc0bus_power_idlereq_END (1)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_mmc1bus_power_idlereq_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_mmc1bus_power_idlereq_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_modem_power_idlereq_START (3)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_modem_power_idlereq_END (3)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_vcodec_power_idlereq_START (4)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_vcodec_power_idlereq_END (4)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_isp_power_idlereq_START (5)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_isp_power_idlereq_END (5)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_sysbus_power_idlereq_START (6)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_sysbus_power_idlereq_END (6)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_cfgbus_power_idlereq_START (7)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_cfgbus_power_idlereq_END (7)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_dmabus_power_idlereq_START (8)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_dmabus_power_idlereq_END (8)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_dbgbus_power_idlereq_START (9)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_dbgbus_power_idlereq_END (9)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_vdec_power_idlereq_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_vdec_power_idlereq_END (10)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_venc_power_idlereq_START (11)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_venc_power_idlereq_END (11)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_dss_power_idlereq_START (13)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_dss_power_idlereq_END (13)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_ivp32_peri_bus_power_idlereq_START (14)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_noc_ivp32_peri_bus_power_idlereq_END (14)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_biten_START (16)
#define SOC_PMCTRL_NOC_POWER_IDLEREQ_biten_END (31)
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
        unsigned int noc_vcodec_power_idleack : 1;
        unsigned int noc_isp_power_idleack : 1;
        unsigned int noc_sysbus_power_idleack : 1;
        unsigned int noc_cfgbus_power_idleack : 1;
        unsigned int noc_dmabus_power_idleack : 1;
        unsigned int noc_dbgbus_power_idleack : 1;
        unsigned int noc_vdec_power_idleack : 1;
        unsigned int noc_venc_power_idleack : 1;
        unsigned int reserved_1 : 1;
        unsigned int noc_dss_power_idleack : 1;
        unsigned int noc_ivp32_peri_bus_power_idleack : 1;
        unsigned int reserved_2 : 17;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLEACK_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_mmc0bus_power_idleack_START (1)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_mmc0bus_power_idleack_END (1)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_mmc1bus_power_idleack_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_mmc1bus_power_idleack_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_modem_power_idleack_START (3)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_modem_power_idleack_END (3)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_vcodec_power_idleack_START (4)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_vcodec_power_idleack_END (4)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_isp_power_idleack_START (5)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_isp_power_idleack_END (5)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_sysbus_power_idleack_START (6)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_sysbus_power_idleack_END (6)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_cfgbus_power_idleack_START (7)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_cfgbus_power_idleack_END (7)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_dmabus_power_idleack_START (8)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_dmabus_power_idleack_END (8)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_dbgbus_power_idleack_START (9)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_dbgbus_power_idleack_END (9)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_vdec_power_idleack_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_vdec_power_idleack_END (10)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_venc_power_idleack_START (11)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_venc_power_idleack_END (11)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_dss_power_idleack_START (13)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_dss_power_idleack_END (13)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_ivp32_peri_bus_power_idleack_START (14)
#define SOC_PMCTRL_NOC_POWER_IDLEACK_noc_ivp32_peri_bus_power_idleack_END (14)
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
        unsigned int noc_vcodec_power_idle : 1;
        unsigned int noc_isp_power_idle : 1;
        unsigned int noc_sysbus_power_idle : 1;
        unsigned int noc_cfgbus_power_idle : 1;
        unsigned int noc_dmabus_power_idle : 1;
        unsigned int noc_dbgbus_power_idle : 1;
        unsigned int noc_vdec_power_idle : 1;
        unsigned int noc_venc_power_idle : 1;
        unsigned int reserved_1 : 1;
        unsigned int noc_dss_power_idle : 1;
        unsigned int noc_ivp32_peri_bus_power_idle : 1;
        unsigned int reserved_2 : 17;
    } reg;
} SOC_PMCTRL_NOC_POWER_IDLE_UNION;
#endif
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_mmc0bus_power_idle_START (1)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_mmc0bus_power_idle_END (1)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_mmc1bus_power_idle_START (2)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_mmc1bus_power_idle_END (2)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_modem_power_idle_START (3)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_modem_power_idle_END (3)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_vcodec_power_idle_START (4)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_vcodec_power_idle_END (4)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_isp_power_idle_START (5)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_isp_power_idle_END (5)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_sysbus_power_idle_START (6)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_sysbus_power_idle_END (6)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_cfgbus_power_idle_START (7)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_cfgbus_power_idle_END (7)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_dmabus_power_idle_START (8)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_dmabus_power_idle_END (8)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_dbgbus_power_idle_START (9)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_dbgbus_power_idle_END (9)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_vdec_power_idle_START (10)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_vdec_power_idle_END (10)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_venc_power_idle_START (11)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_venc_power_idle_END (11)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_dss_power_idle_START (13)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_dss_power_idle_END (13)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_ivp32_peri_bus_power_idle_START (14)
#define SOC_PMCTRL_NOC_POWER_IDLE_noc_ivp32_peri_bus_power_idle_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_maintimeout_mask : 25;
        unsigned int reserved : 7;
    } reg;
} SOC_PMCTRL_PERI_INT0_MASK_UNION;
#endif
#define SOC_PMCTRL_PERI_INT0_MASK_noc_maintimeout_mask_START (0)
#define SOC_PMCTRL_PERI_INT0_MASK_noc_maintimeout_mask_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_gic_t_maintimeout : 1;
        unsigned int noc_hkadc_ssi_t_maintimeout : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_lpm3_slv_t_maintimeout : 1;
        unsigned int noc_sys_peri0_cfg_t_maintimeout : 1;
        unsigned int noc_sys_peri1_cfg_t_maintimeout : 1;
        unsigned int noc_sys_peri2_cfg_t_maintimeout : 1;
        unsigned int noc_sys_peri3_cfg_t_maintimeout : 1;
        unsigned int reserved_1 : 1;
        unsigned int noc_dmac_cfg_t_maintimeout : 1;
        unsigned int reserved_2 : 2;
        unsigned int noc_socp_cfg_t_maintimeout : 1;
        unsigned int noc_top_cssys_slv_cfg_t_maintimeout : 1;
        unsigned int reserved_3 : 1;
        unsigned int noc_modem_bus_cfg_t_maintimeout : 1;
        unsigned int noc_usb3_cfg_t_maintimeout : 1;
        unsigned int noc_emmc0_cfg_t_maintimeout : 1;
        unsigned int noc_emmc0bus_apb_cfg_t_maintimeout : 1;
        unsigned int noc_ivp32_cfg_t_maintimeout : 1;
        unsigned int noc_sys2hkmem_t_maintimeout : 1;
        unsigned int noc_emmc1bus_apb_cfg_t_maintimeout : 1;
        unsigned int reserved_4 : 1;
        unsigned int noc_cfg2vivo_t_maintimeout : 1;
        unsigned int noc_debug_apb_cfg_t_maintimeout : 1;
        unsigned int reserved_5 : 7;
    } reg;
} SOC_PMCTRL_PERI_INT0_STAT_UNION;
#endif
#define SOC_PMCTRL_PERI_INT0_STAT_noc_gic_t_maintimeout_START (0)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_gic_t_maintimeout_END (0)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_hkadc_ssi_t_maintimeout_START (1)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_hkadc_ssi_t_maintimeout_END (1)
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
#define SOC_PMCTRL_PERI_INT0_STAT_noc_socp_cfg_t_maintimeout_START (12)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_socp_cfg_t_maintimeout_END (12)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_top_cssys_slv_cfg_t_maintimeout_START (13)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_top_cssys_slv_cfg_t_maintimeout_END (13)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_modem_bus_cfg_t_maintimeout_START (15)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_modem_bus_cfg_t_maintimeout_END (15)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_usb3_cfg_t_maintimeout_START (16)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_usb3_cfg_t_maintimeout_END (16)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_emmc0_cfg_t_maintimeout_START (17)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_emmc0_cfg_t_maintimeout_END (17)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_emmc0bus_apb_cfg_t_maintimeout_START (18)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_emmc0bus_apb_cfg_t_maintimeout_END (18)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_ivp32_cfg_t_maintimeout_START (19)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_ivp32_cfg_t_maintimeout_END (19)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_sys2hkmem_t_maintimeout_START (20)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_sys2hkmem_t_maintimeout_END (20)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_emmc1bus_apb_cfg_t_maintimeout_START (21)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_emmc1bus_apb_cfg_t_maintimeout_END (21)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_cfg2vivo_t_maintimeout_START (23)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_cfg2vivo_t_maintimeout_END (23)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_debug_apb_cfg_t_maintimeout_START (24)
#define SOC_PMCTRL_PERI_INT0_STAT_noc_debug_apb_cfg_t_maintimeout_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_INT1_MASK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PMCTRL_PERI_INT1_STAT_UNION;
#endif
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
        unsigned int vs_ctrl_en_1 : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VSENSOR_CTRL_EN_1_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CTRL_EN_1_vs_ctrl_en_1_START (0)
#define SOC_PMCTRL_VSENSOR_CTRL_EN_1_vs_ctrl_en_1_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_ctrl_bypass_1 : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VSENSOR_CTRL_BYPASS_1_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CTRL_BYPASS_1_vs_ctrl_bypass_1_START (0)
#define SOC_PMCTRL_VSENSOR_CTRL_BYPASS_1_vs_ctrl_bypass_1_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_mod_a53 : 1;
        unsigned int a53_cpu0_wfi_wfe_bypass : 1;
        unsigned int a53_cpu1_wfi_wfe_bypass : 1;
        unsigned int a53_cpu2_wfi_wfe_bypass : 1;
        unsigned int a53_cpu3_wfi_wfe_bypass : 1;
        unsigned int a53_l2_idle_div_mod : 2;
        unsigned int reserved : 1;
        unsigned int svfd_test_in_1 : 4;
        unsigned int svfd_ulvt_ll_1 : 4;
        unsigned int svfd_ulvt_sl_1 : 4;
        unsigned int svfd_lvt_ll_1 : 4;
        unsigned int svfd_lvt_sl_1 : 4;
        unsigned int svfd_vdm_mode_1 : 2;
        unsigned int svfd_match_detect_1 : 1;
        unsigned int svfd_trim_1 : 1;
    } reg;
} SOC_PMCTRL_VSENSOR_CTRL_1_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CTRL_1_vs_mod_a53_START (0)
#define SOC_PMCTRL_VSENSOR_CTRL_1_vs_mod_a53_END (0)
#define SOC_PMCTRL_VSENSOR_CTRL_1_a53_cpu0_wfi_wfe_bypass_START (1)
#define SOC_PMCTRL_VSENSOR_CTRL_1_a53_cpu0_wfi_wfe_bypass_END (1)
#define SOC_PMCTRL_VSENSOR_CTRL_1_a53_cpu1_wfi_wfe_bypass_START (2)
#define SOC_PMCTRL_VSENSOR_CTRL_1_a53_cpu1_wfi_wfe_bypass_END (2)
#define SOC_PMCTRL_VSENSOR_CTRL_1_a53_cpu2_wfi_wfe_bypass_START (3)
#define SOC_PMCTRL_VSENSOR_CTRL_1_a53_cpu2_wfi_wfe_bypass_END (3)
#define SOC_PMCTRL_VSENSOR_CTRL_1_a53_cpu3_wfi_wfe_bypass_START (4)
#define SOC_PMCTRL_VSENSOR_CTRL_1_a53_cpu3_wfi_wfe_bypass_END (4)
#define SOC_PMCTRL_VSENSOR_CTRL_1_a53_l2_idle_div_mod_START (5)
#define SOC_PMCTRL_VSENSOR_CTRL_1_a53_l2_idle_div_mod_END (6)
#define SOC_PMCTRL_VSENSOR_CTRL_1_svfd_test_in_1_START (8)
#define SOC_PMCTRL_VSENSOR_CTRL_1_svfd_test_in_1_END (11)
#define SOC_PMCTRL_VSENSOR_CTRL_1_svfd_ulvt_ll_1_START (12)
#define SOC_PMCTRL_VSENSOR_CTRL_1_svfd_ulvt_ll_1_END (15)
#define SOC_PMCTRL_VSENSOR_CTRL_1_svfd_ulvt_sl_1_START (16)
#define SOC_PMCTRL_VSENSOR_CTRL_1_svfd_ulvt_sl_1_END (19)
#define SOC_PMCTRL_VSENSOR_CTRL_1_svfd_lvt_ll_1_START (20)
#define SOC_PMCTRL_VSENSOR_CTRL_1_svfd_lvt_ll_1_END (23)
#define SOC_PMCTRL_VSENSOR_CTRL_1_svfd_lvt_sl_1_START (24)
#define SOC_PMCTRL_VSENSOR_CTRL_1_svfd_lvt_sl_1_END (27)
#define SOC_PMCTRL_VSENSOR_CTRL_1_svfd_vdm_mode_1_START (28)
#define SOC_PMCTRL_VSENSOR_CTRL_1_svfd_vdm_mode_1_END (29)
#define SOC_PMCTRL_VSENSOR_CTRL_1_svfd_match_detect_1_START (30)
#define SOC_PMCTRL_VSENSOR_CTRL_1_svfd_match_detect_1_END (30)
#define SOC_PMCTRL_VSENSOR_CTRL_1_svfd_trim_1_START (31)
#define SOC_PMCTRL_VSENSOR_CTRL_1_svfd_trim_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_cnt_quit_1 : 16;
        unsigned int vs_cnt_enter_1 : 16;
    } reg;
} SOC_PMCTRL_VSENSOR_CNT_1_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CNT_1_vs_cnt_quit_1_START (0)
#define SOC_PMCTRL_VSENSOR_CNT_1_vs_cnt_quit_1_END (15)
#define SOC_PMCTRL_VSENSOR_CNT_1_vs_cnt_enter_1_START (16)
#define SOC_PMCTRL_VSENSOR_CNT_1_vs_cnt_enter_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_vs_ref_code_l : 6;
        unsigned int a53_vs_ref_code_h : 6;
        unsigned int a53_cfg_cnt_cpu_wake_quit : 8;
        unsigned int a53_cfg_cnt_cpu_l2_idle_quit : 8;
        unsigned int a53_cpu_wake_up_mode : 2;
        unsigned int a53_cpu_l2_idle_switch_bypass : 1;
        unsigned int a53_cpu_l2_idle_gt_en : 1;
    } reg;
} SOC_PMCTRL_VSENSOR_REF_CODE_1_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_REF_CODE_1_a53_vs_ref_code_l_START (0)
#define SOC_PMCTRL_VSENSOR_REF_CODE_1_a53_vs_ref_code_l_END (5)
#define SOC_PMCTRL_VSENSOR_REF_CODE_1_a53_vs_ref_code_h_START (6)
#define SOC_PMCTRL_VSENSOR_REF_CODE_1_a53_vs_ref_code_h_END (11)
#define SOC_PMCTRL_VSENSOR_REF_CODE_1_a53_cfg_cnt_cpu_wake_quit_START (12)
#define SOC_PMCTRL_VSENSOR_REF_CODE_1_a53_cfg_cnt_cpu_wake_quit_END (19)
#define SOC_PMCTRL_VSENSOR_REF_CODE_1_a53_cfg_cnt_cpu_l2_idle_quit_START (20)
#define SOC_PMCTRL_VSENSOR_REF_CODE_1_a53_cfg_cnt_cpu_l2_idle_quit_END (27)
#define SOC_PMCTRL_VSENSOR_REF_CODE_1_a53_cpu_wake_up_mode_START (28)
#define SOC_PMCTRL_VSENSOR_REF_CODE_1_a53_cpu_wake_up_mode_END (29)
#define SOC_PMCTRL_VSENSOR_REF_CODE_1_a53_cpu_l2_idle_switch_bypass_START (30)
#define SOC_PMCTRL_VSENSOR_REF_CODE_1_a53_cpu_l2_idle_switch_bypass_END (30)
#define SOC_PMCTRL_VSENSOR_REF_CODE_1_a53_cpu_l2_idle_gt_en_START (31)
#define SOC_PMCTRL_VSENSOR_REF_CODE_1_a53_cpu_l2_idle_gt_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_vs_cali_code : 7;
        unsigned int reserved_0 : 1;
        unsigned int a53_svfd_off_mode : 1;
        unsigned int a53_svfd_div64_en : 1;
        unsigned int a53_svfd_vdm_period : 1;
        unsigned int a53_svfd_edge_sel : 1;
        unsigned int a53_svfd_cmp_data_mode : 2;
        unsigned int a53_svfd_data_limit_e : 1;
        unsigned int a53_svfd_glitch_test : 1;
        unsigned int a53_svfd_out_div_sel : 4;
        unsigned int reserved_1 : 1;
        unsigned int sel_ckmux_a53_icg : 1;
        unsigned int reserved_2 : 1;
        unsigned int a53_ext_ref_code : 7;
        unsigned int a53_compare_mod : 2;
    } reg;
} SOC_PMCTRL_VSENSOR_CALI_CODE_1_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_vs_cali_code_START (0)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_vs_cali_code_END (6)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_svfd_off_mode_START (8)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_svfd_off_mode_END (8)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_svfd_div64_en_START (9)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_svfd_div64_en_END (9)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_svfd_vdm_period_START (10)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_svfd_vdm_period_END (10)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_svfd_edge_sel_START (11)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_svfd_edge_sel_END (11)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_svfd_cmp_data_mode_START (12)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_svfd_cmp_data_mode_END (13)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_svfd_data_limit_e_START (14)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_svfd_data_limit_e_END (14)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_svfd_glitch_test_START (15)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_svfd_glitch_test_END (15)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_svfd_out_div_sel_START (16)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_svfd_out_div_sel_END (19)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_sel_ckmux_a53_icg_START (21)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_sel_ckmux_a53_icg_END (21)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_ext_ref_code_START (23)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_ext_ref_code_END (29)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_compare_mod_START (30)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_1_a53_compare_mod_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_vs_code : 6;
        unsigned int reserved : 26;
    } reg;
} SOC_PMCTRL_VSENSOR_CODE_1_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CODE_1_a53_vs_code_START (0)
#define SOC_PMCTRL_VSENSOR_CODE_1_a53_vs_code_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_vdm_stat_1 : 1;
        unsigned int dll_lock_1 : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_VSENSOR_INTSTAT_1_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_INTSTAT_1_intr_vdm_stat_1_START (0)
#define SOC_PMCTRL_VSENSOR_INTSTAT_1_intr_vdm_stat_1_END (0)
#define SOC_PMCTRL_VSENSOR_INTSTAT_1_dll_lock_1_START (1)
#define SOC_PMCTRL_VSENSOR_INTSTAT_1_dll_lock_1_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_d_rate_1 : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_VSENSOR_D_RATE_1_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_D_RATE_1_svfd_d_rate_1_START (0)
#define SOC_PMCTRL_VSENSOR_D_RATE_1_svfd_d_rate_1_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_ctrl_en_0 : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VSENSOR_CTRL_EN_0_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CTRL_EN_0_vs_ctrl_en_0_START (0)
#define SOC_PMCTRL_VSENSOR_CTRL_EN_0_vs_ctrl_en_0_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_ctrl_bypass_0 : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VSENSOR_CTRL_BYPASS_0_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CTRL_BYPASS_0_vs_ctrl_bypass_0_START (0)
#define SOC_PMCTRL_VSENSOR_CTRL_BYPASS_0_vs_ctrl_bypass_0_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_mod_maia : 1;
        unsigned int maia_cpu0_wfi_wfe_bypass : 1;
        unsigned int maia_cpu1_wfi_wfe_bypass : 1;
        unsigned int maia_cpu2_wfi_wfe_bypass : 1;
        unsigned int maia_cpu3_wfi_wfe_bypass : 1;
        unsigned int maia_l2_idle_div_mod : 2;
        unsigned int reserved : 1;
        unsigned int svfd_test_in_0 : 4;
        unsigned int svfd_ulvt_ll_0 : 4;
        unsigned int svfd_ulvt_sl_0 : 4;
        unsigned int svfd_lvt_ll_0 : 4;
        unsigned int svfd_lvt_sl_0 : 4;
        unsigned int svfd_vdm_mode_0 : 2;
        unsigned int svfd_match_detect_0 : 1;
        unsigned int svfd_trim_0 : 1;
    } reg;
} SOC_PMCTRL_VSENSOR_CTRL_0_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CTRL_0_vs_mod_maia_START (0)
#define SOC_PMCTRL_VSENSOR_CTRL_0_vs_mod_maia_END (0)
#define SOC_PMCTRL_VSENSOR_CTRL_0_maia_cpu0_wfi_wfe_bypass_START (1)
#define SOC_PMCTRL_VSENSOR_CTRL_0_maia_cpu0_wfi_wfe_bypass_END (1)
#define SOC_PMCTRL_VSENSOR_CTRL_0_maia_cpu1_wfi_wfe_bypass_START (2)
#define SOC_PMCTRL_VSENSOR_CTRL_0_maia_cpu1_wfi_wfe_bypass_END (2)
#define SOC_PMCTRL_VSENSOR_CTRL_0_maia_cpu2_wfi_wfe_bypass_START (3)
#define SOC_PMCTRL_VSENSOR_CTRL_0_maia_cpu2_wfi_wfe_bypass_END (3)
#define SOC_PMCTRL_VSENSOR_CTRL_0_maia_cpu3_wfi_wfe_bypass_START (4)
#define SOC_PMCTRL_VSENSOR_CTRL_0_maia_cpu3_wfi_wfe_bypass_END (4)
#define SOC_PMCTRL_VSENSOR_CTRL_0_maia_l2_idle_div_mod_START (5)
#define SOC_PMCTRL_VSENSOR_CTRL_0_maia_l2_idle_div_mod_END (6)
#define SOC_PMCTRL_VSENSOR_CTRL_0_svfd_test_in_0_START (8)
#define SOC_PMCTRL_VSENSOR_CTRL_0_svfd_test_in_0_END (11)
#define SOC_PMCTRL_VSENSOR_CTRL_0_svfd_ulvt_ll_0_START (12)
#define SOC_PMCTRL_VSENSOR_CTRL_0_svfd_ulvt_ll_0_END (15)
#define SOC_PMCTRL_VSENSOR_CTRL_0_svfd_ulvt_sl_0_START (16)
#define SOC_PMCTRL_VSENSOR_CTRL_0_svfd_ulvt_sl_0_END (19)
#define SOC_PMCTRL_VSENSOR_CTRL_0_svfd_lvt_ll_0_START (20)
#define SOC_PMCTRL_VSENSOR_CTRL_0_svfd_lvt_ll_0_END (23)
#define SOC_PMCTRL_VSENSOR_CTRL_0_svfd_lvt_sl_0_START (24)
#define SOC_PMCTRL_VSENSOR_CTRL_0_svfd_lvt_sl_0_END (27)
#define SOC_PMCTRL_VSENSOR_CTRL_0_svfd_vdm_mode_0_START (28)
#define SOC_PMCTRL_VSENSOR_CTRL_0_svfd_vdm_mode_0_END (29)
#define SOC_PMCTRL_VSENSOR_CTRL_0_svfd_match_detect_0_START (30)
#define SOC_PMCTRL_VSENSOR_CTRL_0_svfd_match_detect_0_END (30)
#define SOC_PMCTRL_VSENSOR_CTRL_0_svfd_trim_0_START (31)
#define SOC_PMCTRL_VSENSOR_CTRL_0_svfd_trim_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_cnt_quit_0 : 16;
        unsigned int vs_cnt_enter_0 : 16;
    } reg;
} SOC_PMCTRL_VSENSOR_CNT_0_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CNT_0_vs_cnt_quit_0_START (0)
#define SOC_PMCTRL_VSENSOR_CNT_0_vs_cnt_quit_0_END (15)
#define SOC_PMCTRL_VSENSOR_CNT_0_vs_cnt_enter_0_START (16)
#define SOC_PMCTRL_VSENSOR_CNT_0_vs_cnt_enter_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int maia_vs_ref_code_l : 6;
        unsigned int maia_vs_ref_code_h : 6;
        unsigned int maia_cfg_cnt_cpu_wake_quit : 8;
        unsigned int maia_cfg_cnt_cpu_l2_idle_quit : 8;
        unsigned int maia_cpu_wake_up_mode : 2;
        unsigned int maia_cpu_l2_idle_switch_bypass : 1;
        unsigned int maia_cpu_l2_idle_gt_en : 1;
    } reg;
} SOC_PMCTRL_VSENSOR_REF_CODE_0_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_REF_CODE_0_maia_vs_ref_code_l_START (0)
#define SOC_PMCTRL_VSENSOR_REF_CODE_0_maia_vs_ref_code_l_END (5)
#define SOC_PMCTRL_VSENSOR_REF_CODE_0_maia_vs_ref_code_h_START (6)
#define SOC_PMCTRL_VSENSOR_REF_CODE_0_maia_vs_ref_code_h_END (11)
#define SOC_PMCTRL_VSENSOR_REF_CODE_0_maia_cfg_cnt_cpu_wake_quit_START (12)
#define SOC_PMCTRL_VSENSOR_REF_CODE_0_maia_cfg_cnt_cpu_wake_quit_END (19)
#define SOC_PMCTRL_VSENSOR_REF_CODE_0_maia_cfg_cnt_cpu_l2_idle_quit_START (20)
#define SOC_PMCTRL_VSENSOR_REF_CODE_0_maia_cfg_cnt_cpu_l2_idle_quit_END (27)
#define SOC_PMCTRL_VSENSOR_REF_CODE_0_maia_cpu_wake_up_mode_START (28)
#define SOC_PMCTRL_VSENSOR_REF_CODE_0_maia_cpu_wake_up_mode_END (29)
#define SOC_PMCTRL_VSENSOR_REF_CODE_0_maia_cpu_l2_idle_switch_bypass_START (30)
#define SOC_PMCTRL_VSENSOR_REF_CODE_0_maia_cpu_l2_idle_switch_bypass_END (30)
#define SOC_PMCTRL_VSENSOR_REF_CODE_0_maia_cpu_l2_idle_gt_en_START (31)
#define SOC_PMCTRL_VSENSOR_REF_CODE_0_maia_cpu_l2_idle_gt_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int maia_vs_cali_code : 7;
        unsigned int reserved_0 : 1;
        unsigned int maia_svfd_off_mode : 1;
        unsigned int maia_svfd_div64_en : 1;
        unsigned int maia_svfd_vdm_period : 1;
        unsigned int maia_svfd_edge_sel : 1;
        unsigned int maia_svfd_cmp_data_mode : 2;
        unsigned int maia_svfd_data_limit_e : 1;
        unsigned int maia_svfd_glitch_test : 1;
        unsigned int maia_svfd_out_div_sel : 4;
        unsigned int reserved_1 : 1;
        unsigned int sel_ckmux_maia_icg : 1;
        unsigned int reserved_2 : 1;
        unsigned int maia_ext_ref_code : 7;
        unsigned int maia_compare_mod : 2;
    } reg;
} SOC_PMCTRL_VSENSOR_CALI_CODE_0_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_vs_cali_code_START (0)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_vs_cali_code_END (6)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_svfd_off_mode_START (8)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_svfd_off_mode_END (8)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_svfd_div64_en_START (9)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_svfd_div64_en_END (9)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_svfd_vdm_period_START (10)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_svfd_vdm_period_END (10)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_svfd_edge_sel_START (11)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_svfd_edge_sel_END (11)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_svfd_cmp_data_mode_START (12)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_svfd_cmp_data_mode_END (13)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_svfd_data_limit_e_START (14)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_svfd_data_limit_e_END (14)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_svfd_glitch_test_START (15)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_svfd_glitch_test_END (15)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_svfd_out_div_sel_START (16)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_svfd_out_div_sel_END (19)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_sel_ckmux_maia_icg_START (21)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_sel_ckmux_maia_icg_END (21)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_ext_ref_code_START (23)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_ext_ref_code_END (29)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_compare_mod_START (30)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_0_maia_compare_mod_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int maia_vs_code : 6;
        unsigned int reserved : 26;
    } reg;
} SOC_PMCTRL_VSENSOR_CODE_0_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CODE_0_maia_vs_code_START (0)
#define SOC_PMCTRL_VSENSOR_CODE_0_maia_vs_code_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_vdm_stat_0 : 1;
        unsigned int dll_lock_0 : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_VSENSOR_INTSTAT_0_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_INTSTAT_0_intr_vdm_stat_0_START (0)
#define SOC_PMCTRL_VSENSOR_INTSTAT_0_intr_vdm_stat_0_END (0)
#define SOC_PMCTRL_VSENSOR_INTSTAT_0_dll_lock_0_START (1)
#define SOC_PMCTRL_VSENSOR_INTSTAT_0_dll_lock_0_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_d_rate_0 : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_VSENSOR_D_RATE_0_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_D_RATE_0_svfd_d_rate_0_START (0)
#define SOC_PMCTRL_VSENSOR_D_RATE_0_svfd_d_rate_0_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_hpm_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_PERIHPMEN_UNION;
#endif
#define SOC_PMCTRL_PERIHPMEN_peri_hpm_en_START (0)
#define SOC_PMCTRL_PERIHPMEN_peri_hpm_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_hpmx_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_PERIHPMXEN_UNION;
#endif
#define SOC_PMCTRL_PERIHPMXEN_peri_hpmx_en_START (0)
#define SOC_PMCTRL_PERIHPMXEN_peri_hpmx_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_hpm_opc_vld : 1;
        unsigned int peri_hpmx_opc_vld : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_PERIHPMOPCVALID_UNION;
#endif
#define SOC_PMCTRL_PERIHPMOPCVALID_peri_hpm_opc_vld_START (0)
#define SOC_PMCTRL_PERIHPMOPCVALID_peri_hpm_opc_vld_END (0)
#define SOC_PMCTRL_PERIHPMOPCVALID_peri_hpmx_opc_vld_START (1)
#define SOC_PMCTRL_PERIHPMOPCVALID_peri_hpmx_opc_vld_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_hpm_opc : 10;
        unsigned int peri_hpmx_opc : 10;
        unsigned int reserved : 12;
    } reg;
} SOC_PMCTRL_PERIHPMOPC_UNION;
#endif
#define SOC_PMCTRL_PERIHPMOPC_peri_hpm_opc_START (0)
#define SOC_PMCTRL_PERIHPMOPC_peri_hpm_opc_END (9)
#define SOC_PMCTRL_PERIHPMOPC_peri_hpmx_opc_START (10)
#define SOC_PMCTRL_PERIHPMOPC_peri_hpmx_opc_END (19)
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
        unsigned int vs_ctrl_en_2 : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VSENSOR_CTRL_EN_2_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CTRL_EN_2_vs_ctrl_en_2_START (0)
#define SOC_PMCTRL_VSENSOR_CTRL_EN_2_vs_ctrl_en_2_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_ctrl_bypass_2 : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PMCTRL_VSENSOR_CTRL_BYPASS_2_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CTRL_BYPASS_2_vs_ctrl_bypass_2_START (0)
#define SOC_PMCTRL_VSENSOR_CTRL_BYPASS_2_vs_ctrl_bypass_2_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_mod_g3d : 1;
        unsigned int g3d_pwrdiv_bypass : 1;
        unsigned int reserved : 6;
        unsigned int svfd_test_in_2 : 4;
        unsigned int svfd_ulvt_ll_2 : 4;
        unsigned int svfd_ulvt_sl_2 : 4;
        unsigned int svfd_lvt_ll_2 : 4;
        unsigned int svfd_lvt_sl_2 : 4;
        unsigned int svfd_vdm_mode_2 : 2;
        unsigned int svfd_match_detect_2 : 1;
        unsigned int svfd_trim_2 : 1;
    } reg;
} SOC_PMCTRL_VSENSOR_CTRL_2_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CTRL_2_vs_mod_g3d_START (0)
#define SOC_PMCTRL_VSENSOR_CTRL_2_vs_mod_g3d_END (0)
#define SOC_PMCTRL_VSENSOR_CTRL_2_g3d_pwrdiv_bypass_START (1)
#define SOC_PMCTRL_VSENSOR_CTRL_2_g3d_pwrdiv_bypass_END (1)
#define SOC_PMCTRL_VSENSOR_CTRL_2_svfd_test_in_2_START (8)
#define SOC_PMCTRL_VSENSOR_CTRL_2_svfd_test_in_2_END (11)
#define SOC_PMCTRL_VSENSOR_CTRL_2_svfd_ulvt_ll_2_START (12)
#define SOC_PMCTRL_VSENSOR_CTRL_2_svfd_ulvt_ll_2_END (15)
#define SOC_PMCTRL_VSENSOR_CTRL_2_svfd_ulvt_sl_2_START (16)
#define SOC_PMCTRL_VSENSOR_CTRL_2_svfd_ulvt_sl_2_END (19)
#define SOC_PMCTRL_VSENSOR_CTRL_2_svfd_lvt_ll_2_START (20)
#define SOC_PMCTRL_VSENSOR_CTRL_2_svfd_lvt_ll_2_END (23)
#define SOC_PMCTRL_VSENSOR_CTRL_2_svfd_lvt_sl_2_START (24)
#define SOC_PMCTRL_VSENSOR_CTRL_2_svfd_lvt_sl_2_END (27)
#define SOC_PMCTRL_VSENSOR_CTRL_2_svfd_vdm_mode_2_START (28)
#define SOC_PMCTRL_VSENSOR_CTRL_2_svfd_vdm_mode_2_END (29)
#define SOC_PMCTRL_VSENSOR_CTRL_2_svfd_match_detect_2_START (30)
#define SOC_PMCTRL_VSENSOR_CTRL_2_svfd_match_detect_2_END (30)
#define SOC_PMCTRL_VSENSOR_CTRL_2_svfd_trim_2_START (31)
#define SOC_PMCTRL_VSENSOR_CTRL_2_svfd_trim_2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vs_cnt_quit_2 : 16;
        unsigned int vs_cnt_enter_2 : 16;
    } reg;
} SOC_PMCTRL_VSENSOR_CNT_2_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CNT_2_vs_cnt_quit_2_START (0)
#define SOC_PMCTRL_VSENSOR_CNT_2_vs_cnt_quit_2_END (15)
#define SOC_PMCTRL_VSENSOR_CNT_2_vs_cnt_enter_2_START (16)
#define SOC_PMCTRL_VSENSOR_CNT_2_vs_cnt_enter_2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_vs_ref_code_l : 6;
        unsigned int g3d_vs_ref_code_h : 6;
        unsigned int g3d_cfg_cnt_cpu_wake_quit : 8;
        unsigned int reserved_0 : 8;
        unsigned int g3d_wake_up_mode : 2;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_PMCTRL_VSENSOR_REF_CODE_2_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_REF_CODE_2_g3d_vs_ref_code_l_START (0)
#define SOC_PMCTRL_VSENSOR_REF_CODE_2_g3d_vs_ref_code_l_END (5)
#define SOC_PMCTRL_VSENSOR_REF_CODE_2_g3d_vs_ref_code_h_START (6)
#define SOC_PMCTRL_VSENSOR_REF_CODE_2_g3d_vs_ref_code_h_END (11)
#define SOC_PMCTRL_VSENSOR_REF_CODE_2_g3d_cfg_cnt_cpu_wake_quit_START (12)
#define SOC_PMCTRL_VSENSOR_REF_CODE_2_g3d_cfg_cnt_cpu_wake_quit_END (19)
#define SOC_PMCTRL_VSENSOR_REF_CODE_2_g3d_wake_up_mode_START (28)
#define SOC_PMCTRL_VSENSOR_REF_CODE_2_g3d_wake_up_mode_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_vs_cali_code : 7;
        unsigned int reserved_0 : 1;
        unsigned int g3d_svfd_off_mode : 1;
        unsigned int g3d_svfd_div64_en : 1;
        unsigned int g3d_svfd_vdm_period : 1;
        unsigned int g3d_svfd_edge_sel : 1;
        unsigned int g3d_svfd_cmp_data_mode : 2;
        unsigned int g3d_svfd_data_limit_e : 1;
        unsigned int g3d_svfd_glitch_test : 1;
        unsigned int g3d_svfd_out_div_sel : 4;
        unsigned int reserved_1 : 1;
        unsigned int sel_ckmux_gpu_icg : 1;
        unsigned int reserved_2 : 10;
    } reg;
} SOC_PMCTRL_VSENSOR_CALI_CODE_2_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_vs_cali_code_START (0)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_vs_cali_code_END (6)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_svfd_off_mode_START (8)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_svfd_off_mode_END (8)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_svfd_div64_en_START (9)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_svfd_div64_en_END (9)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_svfd_vdm_period_START (10)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_svfd_vdm_period_END (10)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_svfd_edge_sel_START (11)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_svfd_edge_sel_END (11)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_svfd_cmp_data_mode_START (12)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_svfd_cmp_data_mode_END (13)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_svfd_data_limit_e_START (14)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_svfd_data_limit_e_END (14)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_svfd_glitch_test_START (15)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_svfd_glitch_test_END (15)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_svfd_out_div_sel_START (16)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_g3d_svfd_out_div_sel_END (19)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_sel_ckmux_gpu_icg_START (21)
#define SOC_PMCTRL_VSENSOR_CALI_CODE_2_sel_ckmux_gpu_icg_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_vs_code : 6;
        unsigned int reserved : 26;
    } reg;
} SOC_PMCTRL_VSENSOR_CODE_2_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_CODE_2_g3d_vs_code_START (0)
#define SOC_PMCTRL_VSENSOR_CODE_2_g3d_vs_code_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_vdm_stat_2 : 1;
        unsigned int dll_lock_2 : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_VSENSOR_INTSTAT_2_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_INTSTAT_2_intr_vdm_stat_2_START (0)
#define SOC_PMCTRL_VSENSOR_INTSTAT_2_intr_vdm_stat_2_END (0)
#define SOC_PMCTRL_VSENSOR_INTSTAT_2_dll_lock_2_START (1)
#define SOC_PMCTRL_VSENSOR_INTSTAT_2_dll_lock_2_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int svfd_d_rate_2 : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_PMCTRL_VSENSOR_D_RATE_2_UNION;
#endif
#define SOC_PMCTRL_VSENSOR_D_RATE_2_svfd_d_rate_2_START (0)
#define SOC_PMCTRL_VSENSOR_D_RATE_2_svfd_d_rate_2_END (1)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
