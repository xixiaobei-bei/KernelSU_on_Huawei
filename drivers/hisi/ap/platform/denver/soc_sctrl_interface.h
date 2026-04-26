#ifndef __SOC_SCTRL_INTERFACE_H__
#define __SOC_SCTRL_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_SCTRL_SCCTRL_ADDR(base) ((base) + (0x000UL))
#define SOC_SCTRL_SCSYSSTAT_ADDR(base) ((base) + (0x004UL))
#define SOC_SCTRL_SCDEEPSLEEPED_ADDR(base) ((base) + (0x008UL))
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_ADDR(base) ((base) + (0x014UL))
#define SOC_SCTRL_SCEFUSEDATA2_ADDR(base) ((base) + (0x00CUL))
#define SOC_SCTRL_SCEFUSEDATA3_ADDR(base) ((base) + (0x010UL))
#define SOC_SCTRL_SCEFUSEDATA4_ADDR(base) ((base) + (0x050UL))
#define SOC_SCTRL_SCEFUSEDATA5_ADDR(base) ((base) + (0x054UL))
#define SOC_SCTRL_SCEFUSEDATA6_ADDR(base) ((base) + (0x058UL))
#define SOC_SCTRL_SCEFUSEDATA7_ADDR(base) ((base) + (0x05CUL))
#define SOC_SCTRL_SCEFUSEDATA8_ADDR(base) ((base) + (0x150UL))
#define SOC_SCTRL_SCEFUSEDATA9_ADDR(base) ((base) + (0x154UL))
#define SOC_SCTRL_SCXTALCTRL_ADDR(base) ((base) + (0x020UL))
#define SOC_SCTRL_SCXTALTIMEOUT0_ADDR(base) ((base) + (0x024UL))
#define SOC_SCTRL_SCXTALTIMEOUT1_ADDR(base) ((base) + (0x028UL))
#define SOC_SCTRL_SCXTALSTAT_ADDR(base) ((base) + (0x02CUL))
#define SOC_SCTRL_SCSW2FLLBYPASS_ADDR(base) ((base) + (0x030UL))
#define SOC_SCTRL_SCFLLLOCK_FILTER_ADDR(base) ((base) + (0x034UL))
#define SOC_SCTRL_SCLPSTATCFG_ADDR(base) ((base) + (0x038UL))
#define SOC_SCTRL_SCPERIISOBYPASS_ADDR(base) ((base) + (0x03cUL))
#define SOC_SCTRL_SCISOEN_ADDR(base) ((base) + (0x040UL))
#define SOC_SCTRL_SCISODIS_ADDR(base) ((base) + (0x044UL))
#define SOC_SCTRL_SCISOSTAT_ADDR(base) ((base) + (0x048UL))
#define SOC_SCTRL_SCPWREN_ADDR(base) ((base) + (0x060UL))
#define SOC_SCTRL_SCPWRDIS_ADDR(base) ((base) + (0x064UL))
#define SOC_SCTRL_SCPWRSTAT_ADDR(base) ((base) + (0x068UL))
#define SOC_SCTRL_SCPWRACK_ADDR(base) ((base) + (0x06CUL))
#define SOC_SCTRL_SCPERPWRDOWNTIME_ADDR(base) ((base) + (0x070UL))
#define SOC_SCTRL_SCPERPWRUPTIME_ADDR(base) ((base) + (0x074UL))
#define SOC_SCTRL_SC_ASP_PWRCFG0_ADDR(base) ((base) + (0x080UL))
#define SOC_SCTRL_SC_ASP_PWRCFG1_ADDR(base) ((base) + (0x084UL))
#define SOC_SCTRL_SCINT_GATHER_STAT_ADDR(base) ((base) + (0x0A0UL))
#define SOC_SCTRL_SCINT_MASK_ADDR(base) ((base) + (0x0A4UL))
#define SOC_SCTRL_SCINT_STAT_ADDR(base) ((base) + (0x0A8UL))
#define SOC_SCTRL_SCDRX_INT_CFG_ADDR(base) ((base) + (0x0ACUL))
#define SOC_SCTRL_SCLPMCUWFI_INT_ADDR(base) ((base) + (0x0B0UL))
#define SOC_SCTRL_SCINT_MASK1_ADDR(base) ((base) + (0x0B4UL))
#define SOC_SCTRL_SCINT_STAT1_ADDR(base) ((base) + (0x0B8UL))
#define SOC_SCTRL_SCINT_MASK2_ADDR(base) ((base) + (0x0BCUL))
#define SOC_SCTRL_SCINT_STAT2_ADDR(base) ((base) + (0x0C0UL))
#define SOC_SCTRL_SCINT_MASK3_ADDR(base) ((base) + (0x0C4UL))
#define SOC_SCTRL_SCINT_STAT3_ADDR(base) ((base) + (0x0C8UL))
#define SOC_SCTRL_SCINT_MASK4_ADDR(base) ((base) + (0x090UL))
#define SOC_SCTRL_SCINT_STAT4_ADDR(base) ((base) + (0x094UL))
#define SOC_SCTRL_SCINT_MASK5_ADDR(base) ((base) + (0x098UL))
#define SOC_SCTRL_SCINT_STAT5_ADDR(base) ((base) + (0x09CUL))
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_ADDR(base) ((base) + (0x0E0UL))
#define SOC_SCTRL_SCLBINTPLL1_CTRL1_ADDR(base) ((base) + (0x0E4UL))
#define SOC_SCTRL_SCLBINTPLL1_CTRL2_ADDR(base) ((base) + (0x0E8UL))
#define SOC_SCTRL_SCLBINTPLL1_STAT_ADDR(base) ((base) + (0x0ECUL))
#define SOC_SCTRL_SCPPLLCTRL0_ADDR(base) ((base) + (0x100UL))
#define SOC_SCTRL_SCPPLLCTRL1_ADDR(base) ((base) + (0x104UL))
#define SOC_SCTRL_SCPPLLSSCCTRL_ADDR(base) ((base) + (0x108UL))
#define SOC_SCTRL_SCPPLLSTAT_ADDR(base) ((base) + (0x10CUL))
#define SOC_SCTRL_SCFPLLCTRL0_ADDR(base) ((base) + (0x120UL))
#define SOC_SCTRL_SCFPLLCTRL1_ADDR(base) ((base) + (0x124UL))
#define SOC_SCTRL_SCFPLLCTRL2_ADDR(base) ((base) + (0x128UL))
#define SOC_SCTRL_SCFPLLCTRL3_ADDR(base) ((base) + (0x12CUL))
#define SOC_SCTRL_SCFPLLSTAT_ADDR(base) ((base) + (0x130UL))
#define SOC_SCTRL_SCPEREN0_ADDR(base) ((base) + (0x160UL))
#define SOC_SCTRL_SCPERDIS0_ADDR(base) ((base) + (0x164UL))
#define SOC_SCTRL_SCPERCLKEN0_ADDR(base) ((base) + (0x168UL))
#define SOC_SCTRL_SCPERSTAT0_ADDR(base) ((base) + (0x16CUL))
#define SOC_SCTRL_SCPEREN1_ADDR(base) ((base) + (0x170UL))
#define SOC_SCTRL_SCPERDIS1_ADDR(base) ((base) + (0x174UL))
#define SOC_SCTRL_SCPERCLKEN1_ADDR(base) ((base) + (0x178UL))
#define SOC_SCTRL_SCPERSTAT1_ADDR(base) ((base) + (0x17CUL))
#define SOC_SCTRL_SCPEREN2_ADDR(base) ((base) + (0x190UL))
#define SOC_SCTRL_SCPERDIS2_ADDR(base) ((base) + (0x194UL))
#define SOC_SCTRL_SCPERCLKEN2_ADDR(base) ((base) + (0x198UL))
#define SOC_SCTRL_SCPERSTAT2_ADDR(base) ((base) + (0x19CUL))
#define SOC_SCTRL_SCPERSTAT3_ADDR(base) ((base) + (0x1A0UL))
#define SOC_SCTRL_SCPERCLKEN3_ADDR(base) ((base) + (0x1A4UL))
#define SOC_SCTRL_SCPEREN4_ADDR(base) ((base) + (0x1B0UL))
#define SOC_SCTRL_SCPERDIS4_ADDR(base) ((base) + (0x1B4UL))
#define SOC_SCTRL_SCPERCLKEN4_ADDR(base) ((base) + (0x1B8UL))
#define SOC_SCTRL_SCPERSTAT4_ADDR(base) ((base) + (0x1BCUL))
#define SOC_SCTRL_SCPEREN5_ADDR(base) ((base) + (0x1C0UL))
#define SOC_SCTRL_SCPERDIS5_ADDR(base) ((base) + (0x1C4UL))
#define SOC_SCTRL_SCPERCLKEN5_ADDR(base) ((base) + (0x1C8UL))
#define SOC_SCTRL_SCPERSTAT5_ADDR(base) ((base) + (0x1CCUL))
#define SOC_SCTRL_SCPERRSTEN0_ADDR(base) ((base) + (0x200UL))
#define SOC_SCTRL_SCPERRSTDIS0_ADDR(base) ((base) + (0x204UL))
#define SOC_SCTRL_SCPERRSTSTAT0_ADDR(base) ((base) + (0x208UL))
#define SOC_SCTRL_SCPERRSTEN1_ADDR(base) ((base) + (0x20CUL))
#define SOC_SCTRL_SCPERRSTDIS1_ADDR(base) ((base) + (0x210UL))
#define SOC_SCTRL_SCPERRSTSTAT1_ADDR(base) ((base) + (0x214UL))
#define SOC_SCTRL_SCPERRSTEN2_ADDR(base) ((base) + (0x218UL))
#define SOC_SCTRL_SCPERRSTDIS2_ADDR(base) ((base) + (0x21CUL))
#define SOC_SCTRL_SCPERRSTSTAT2_ADDR(base) ((base) + (0x220UL))
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_ADDR(base) ((base) + (0x240UL))
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_ADDR(base) ((base) + (0x244UL))
#define SOC_SCTRL_SCCLKDIV0_ADDR(base) ((base) + (0x250UL))
#define SOC_SCTRL_SCCLKDIV1_ADDR(base) ((base) + (0x254UL))
#define SOC_SCTRL_SCCLKDIV2_ADDR(base) ((base) + (0x258UL))
#define SOC_SCTRL_SCCLKDIV3_ADDR(base) ((base) + (0x25CUL))
#define SOC_SCTRL_SCCLKDIV4_ADDR(base) ((base) + (0x260UL))
#define SOC_SCTRL_SCCLKDIV5_ADDR(base) ((base) + (0x264UL))
#define SOC_SCTRL_SCCLKDIV6_ADDR(base) ((base) + (0x268UL))
#define SOC_SCTRL_SCCLKDIV7_ADDR(base) ((base) + (0x26CUL))
#define SOC_SCTRL_SCCLKDIV8_ADDR(base) ((base) + (0x270UL))
#define SOC_SCTRL_SCCLKDIV9_ADDR(base) ((base) + (0x274UL))
#define SOC_SCTRL_SCUFS_AUTODIV_ADDR(base) ((base) + (0x278UL))
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_ADDR(base) ((base) + (0x27CUL))
#define SOC_SCTRL_SCCLKDIV10_ADDR(base) ((base) + (0x280UL))
#define SOC_SCTRL_SCCLKDIV11_ADDR(base) ((base) + (0x284UL))
#define SOC_SCTRL_SCCLKDIV12_ADDR(base) ((base) + (0x288UL))
#define SOC_SCTRL_SCCLKDIV13_ADDR(base) ((base) + (0x28CUL))
#define SOC_SCTRL_SCCLKDIV14_ADDR(base) ((base) + (0x290UL))
#define SOC_SCTRL_SCCLKDIV15_ADDR(base) ((base) + (0x294UL))
#define SOC_SCTRL_SCCLKDIV16_ADDR(base) ((base) + (0x298UL))
#define SOC_SCTRL_SCPERCTRL0_ADDR(base) ((base) + (0x300UL))
#define SOC_SCTRL_SCPERCTRL1_ADDR(base) ((base) + (0x304UL))
#define SOC_SCTRL_SCPERCTRL2_ADDR(base) ((base) + (0x308UL))
#define SOC_SCTRL_SCPERCTRL3_ADDR(base) ((base) + (0x30CUL))
#define SOC_SCTRL_SCPERCTRL4_ADDR(base) ((base) + (0x310UL))
#define SOC_SCTRL_SCPERCTRL5_ADDR(base) ((base) + (0x314UL))
#define SOC_SCTRL_SCPERCTRL6_ADDR(base) ((base) + (0x318UL))
#define SOC_SCTRL_SCPERCTRL7_ADDR(base) ((base) + (0x31CUL))
#define SOC_SCTRL_SCPERCTRL8_ADDR(base) ((base) + (0x8A8UL))
#define SOC_SCTRL_SCPERCTRL8_ADD_ADDR(base) ((base) + (0x8ACUL))
#define SOC_SCTRL_SCPERCTRL9_ADDR(base) ((base) + (0x324UL))
#define SOC_SCTRL_SCPERCTRL10_ADDR(base) ((base) + (0x328UL))
#define SOC_SCTRL_SCPERCTRL11_ADDR(base) ((base) + (0x32CUL))
#define SOC_SCTRL_SCPERCTRL12_ADDR(base) ((base) + (0x330UL))
#define SOC_SCTRL_SCPERCTRL13_ADDR(base) ((base) + (0x334UL))
#define SOC_SCTRL_SCPERCTRL14_ADDR(base) ((base) + (0x338UL))
#define SOC_SCTRL_SCPERCTRL15_ADDR(base) ((base) + (0x33CUL))
#define SOC_SCTRL_SCPERCTRL16_ADDR(base) ((base) + (0x340UL))
#define SOC_SCTRL_SCPERCTRL17_ADDR(base) ((base) + (0x344UL))
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_ADDR(base) ((base) + (0x354UL))
#define SOC_SCTRL_SCPERSTATUS17_ADDR(base) ((base) + (0x358UL))
#define SOC_SCTRL_SCPERSTATUS16_ADDR(base) ((base) + (0x35CUL))
#define SOC_SCTRL_SCPERSTATUS0_ADDR(base) ((base) + (0x360UL))
#define SOC_SCTRL_SCPERSTATUS1_ADDR(base) ((base) + (0x364UL))
#define SOC_SCTRL_SCPERSTATUS2_ADDR(base) ((base) + (0x368UL))
#define SOC_SCTRL_SCPERSTATUS3_ADDR(base) ((base) + (0x36CUL))
#define SOC_SCTRL_SCPERSTATUS4_ADDR(base) ((base) + (0x370UL))
#define SOC_SCTRL_SCPERSTATUS5_ADDR(base) ((base) + (0x374UL))
#define SOC_SCTRL_SCPERSTATUS6_ADDR(base) ((base) + (0x378UL))
#define SOC_SCTRL_SCPERSTATUS7_ADDR(base) ((base) + (0x37CUL))
#define SOC_SCTRL_SCPERSTATUS8_ADDR(base) ((base) + (0x380UL))
#define SOC_SCTRL_SCPERSTATUS9_ADDR(base) ((base) + (0x384UL))
#define SOC_SCTRL_SCPERSTATUS10_ADDR(base) ((base) + (0x388UL))
#define SOC_SCTRL_SCPERSTATUS11_ADDR(base) ((base) + (0x38CUL))
#define SOC_SCTRL_SCPERSTATUS12_ADDR(base) ((base) + (0x390UL))
#define SOC_SCTRL_SCPERSTATUS13_ADDR(base) ((base) + (0x394UL))
#define SOC_SCTRL_SCPERSTATUS14_ADDR(base) ((base) + (0x398UL))
#define SOC_SCTRL_SCPERSTATUS15_ADDR(base) ((base) + (0x39CUL))
#define SOC_SCTRL_SCINNERSTAT_ADDR(base) ((base) + (0x3A0UL))
#define SOC_SCTRL_SCINNERCTRL_ADDR(base) ((base) + (0x3A4UL))
#define SOC_SCTRL_SC_MEM_CTRL_SD_ADDR(base) ((base) + (0x3A8UL))
#define SOC_SCTRL_SC_MEM_STAT_SD_ADDR(base) ((base) + (0x3ACUL))
#define SOC_SCTRL_SCTIMERCTRL0_ADDR(base) ((base) + (0x3C0UL))
#define SOC_SCTRL_SCTIMERCTRL1_ADDR(base) ((base) + (0x3C4UL))
#define SOC_SCTRL_SCTIMERCTRL2_ADDR(base) ((base) + (0x3C8UL))
#define SOC_SCTRL_SC_SECOND_INT_MASK_ADDR(base) ((base) + (0x3D0UL))
#define SOC_SCTRL_SC_SECOND_INT_ORG_ADDR(base) ((base) + (0x3D4UL))
#define SOC_SCTRL_SC_SECOND_INT_OUT_ADDR(base) ((base) + (0x3D8UL))
#define SOC_SCTRL_SCMRBBUSYSTAT_ADDR(base) ((base) + (0x3FCUL))
#define SOC_SCTRL_SCSWADDR_ADDR(base) ((base) + (0x400UL))
#define SOC_SCTRL_SCDDRADDR_ADDR(base) ((base) + (0x404UL))
#define SOC_SCTRL_SCDDRDATA_ADDR(base) ((base) + (0x408UL))
#define SOC_SCTRL_SCBAKDATA0_ADDR(base) ((base) + (0x40CUL))
#define SOC_SCTRL_SCBAKDATA1_ADDR(base) ((base) + (0x410UL))
#define SOC_SCTRL_SCBAKDATA2_ADDR(base) ((base) + (0x414UL))
#define SOC_SCTRL_SCBAKDATA3_ADDR(base) ((base) + (0x418UL))
#define SOC_SCTRL_SCBAKDATA4_ADDR(base) ((base) + (0x41CUL))
#define SOC_SCTRL_SCBAKDATA5_ADDR(base) ((base) + (0x420UL))
#define SOC_SCTRL_SCBAKDATA6_ADDR(base) ((base) + (0x424UL))
#define SOC_SCTRL_SCBAKDATA7_ADDR(base) ((base) + (0x428UL))
#define SOC_SCTRL_SCBAKDATA8_ADDR(base) ((base) + (0x42CUL))
#define SOC_SCTRL_SCBAKDATA9_ADDR(base) ((base) + (0x430UL))
#define SOC_SCTRL_SCBAKDATA10_ADDR(base) ((base) + (0x434UL))
#define SOC_SCTRL_SCBAKDATA11_ADDR(base) ((base) + (0x438UL))
#define SOC_SCTRL_SCBAKDATA12_ADDR(base) ((base) + (0x43CUL))
#define SOC_SCTRL_SCBAKDATA13_ADDR(base) ((base) + (0x440UL))
#define SOC_SCTRL_SCBAKDATA14_ADDR(base) ((base) + (0x444UL))
#define SOC_SCTRL_SCBAKDATA15_ADDR(base) ((base) + (0x448UL))
#define SOC_SCTRL_SCBAKDATA16_ADDR(base) ((base) + (0x44CUL))
#define SOC_SCTRL_SCBAKDATA17_ADDR(base) ((base) + (0x450UL))
#define SOC_SCTRL_SCBAKDATA18_ADDR(base) ((base) + (0x454UL))
#define SOC_SCTRL_SCBAKDATA19_ADDR(base) ((base) + (0x458UL))
#define SOC_SCTRL_SCBAKDATA20_ADDR(base) ((base) + (0x45CUL))
#define SOC_SCTRL_SCBAKDATA21_ADDR(base) ((base) + (0x460UL))
#define SOC_SCTRL_SCBAKDATA22_ADDR(base) ((base) + (0x464UL))
#define SOC_SCTRL_SCBAKDATA23_ADDR(base) ((base) + (0x468UL))
#define SOC_SCTRL_SCBAKDATA24_ADDR(base) ((base) + (0x46CUL))
#define SOC_SCTRL_SCBAKDATA25_ADDR(base) ((base) + (0x470UL))
#define SOC_SCTRL_SCBAKDATA26_ADDR(base) ((base) + (0x474UL))
#define SOC_SCTRL_SCBAKDATA27_ADDR(base) ((base) + (0x478UL))
#define SOC_SCTRL_SCBAKDATA28_ADDR(base) ((base) + (0x47CUL))
#define SOC_SCTRL_SCLPMCUCLKEN_ADDR(base) ((base) + (0x480UL))
#define SOC_SCTRL_SCLPMCUCLKDIS_ADDR(base) ((base) + (0x484UL))
#define SOC_SCTRL_SCLPMCUCLKSTAT_ADDR(base) ((base) + (0x488UL))
#define SOC_SCTRL_SCLPMCURSTEN_ADDR(base) ((base) + (0x500UL))
#define SOC_SCTRL_SCLPMCURSTDIS_ADDR(base) ((base) + (0x504UL))
#define SOC_SCTRL_SCLPMCURSTSTAT_ADDR(base) ((base) + (0x508UL))
#define SOC_SCTRL_SCLPMCUCTRL_ADDR(base) ((base) + (0x510UL))
#define SOC_SCTRL_SCLPMCUSTAT_ADDR(base) ((base) + (0x514UL))
#define SOC_SCTRL_SCLPMCURAMCTRL_ADDR(base) ((base) + (0x518UL))
#define SOC_SCTRL_SCBBPDRXSTAT0_ADDR(base) ((base) + (0x530UL))
#define SOC_SCTRL_SCBBPDRXSTAT1_ADDR(base) ((base) + (0x534UL))
#define SOC_SCTRL_SCBBPDRXSTAT2_ADDR(base) ((base) + (0x538UL))
#define SOC_SCTRL_SCBBPDRXSTAT3_ADDR(base) ((base) + (0x53CUL))
#define SOC_SCTRL_SCBBPDRXSTAT4_ADDR(base) ((base) + (0x540UL))
#define SOC_SCTRL_SCA53_EVENT_MASK_ADDR(base) ((base) + (0x550UL))
#define SOC_SCTRL_SCA57_EVENT_MASK_ADDR(base) ((base) + (0x554UL))
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_ADDR(base) ((base) + (0x558UL))
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_ADDR(base) ((base) + (0x55CUL))
#define SOC_SCTRL_SCMCPU_EVENT_MASK_ADDR(base) ((base) + (0x560UL))
#define SOC_SCTRL_SCISPA7_EVENT_MASK_ADDR(base) ((base) + (0x568UL))
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_ADDR(base) ((base) + (0x56CUL))
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_ADDR(base) ((base) + (0x570UL))
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_ADDR(base) ((base) + (0x574UL))
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_ADDR(base) ((base) + (0x578UL))
#define SOC_SCTRL_SCEPS_EVENT_MASK_ADDR(base) ((base) + (0x57CUL))
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_ADDR(base) ((base) + (0x544UL))
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_ADDR(base) ((base) + (0x548UL))
#define SOC_SCTRL_SCEVENT_STAT_ADDR(base) ((base) + (0x564UL))
#define SOC_SCTRL_SCIOMCUCLKCTRL_ADDR(base) ((base) + (0x580UL))
#define SOC_SCTRL_SCIOMCUCLKSTAT_ADDR(base) ((base) + (0x584UL))
#define SOC_SCTRL_SCIOMCUCTRL_ADDR(base) ((base) + (0x588UL))
#define SOC_SCTRL_SCIOMCUSTAT_ADDR(base) ((base) + (0x58CUL))
#define SOC_SCTRL_SCPERSTATUS18_ADDR(base) ((base) + (0x590UL))
#define SOC_SCTRL_SCPERSTATUS19_ADDR(base) ((base) + (0x594UL))
#define SOC_SCTRL_SCPERSTATUS20_ADDR(base) ((base) + (0x598UL))
#define SOC_SCTRL_SCPERSTATUS21_ADDR(base) ((base) + (0x59CUL))
#define SOC_SCTRL_SCPERSTATUS22_ADDR(base) ((base) + (0x5A0UL))
#define SOC_SCTRL_SCPERSTATUS23_ADDR(base) ((base) + (0x5A4UL))
#define SOC_SCTRL_SCPERSTATUS24_ADDR(base) ((base) + (0x5A8UL))
#define SOC_SCTRL_SCPERSTATUS25_ADDR(base) ((base) + (0x5ACUL))
#define SOC_SCTRL_SCPERSTATUS26_ADDR(base) ((base) + (0x5B0UL))
#define SOC_SCTRL_SCPERSTATUS27_ADDR(base) ((base) + (0x5B4UL))
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_ADDR(base) ((base) + (0x600UL))
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_ADDR(base) ((base) + (0x604UL))
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_ADDR(base) ((base) + (0x608UL))
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_ADDR(base) ((base) + (0x60CUL))
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_ADDR(base) ((base) + (0x610UL))
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_ADDR(base) ((base) + (0x614UL))
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_ADDR(base) ((base) + (0x9E8UL))
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_ADDR(base) ((base) + (0x9ECUL))
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_ADDR(base) ((base) + (0x9F0UL))
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_ADDR(base) ((base) + (0x9F4UL))
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_ADDR(base) ((base) + (0x9F8UL))
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_ADDR(base) ((base) + (0x9FCUL))
#define SOC_SCTRL_SCJTAG_SEL_ADDR(base) ((base) + (0x800UL))
#define SOC_SCTRL_SCCFG_DJTAG_ADDR(base) ((base) + (0x814UL))
#define SOC_SCTRL_SCCP15_DISABLE_ADDR(base) ((base) + (0x818UL))
#define SOC_SCTRL_SCCLKCNTSTAT_ADDR(base) ((base) + (0x81CUL))
#define SOC_SCTRL_SCCLKCNTCFG_ADDR(base) ((base) + (0x820UL))
#define SOC_SCTRL_SCCLKMONCTRL_ADDR(base) ((base) + (0x824UL))
#define SOC_SCTRL_SCCLKMONINT_ADDR(base) ((base) + (0x828UL))
#define SOC_SCTRL_SCCFG_ARM_DBGEN_ADDR(base) ((base) + (0x82CUL))
#define SOC_SCTRL_SCARM_DBG_KEY0_ADDR(base) ((base) + (0x830UL))
#define SOC_SCTRL_SCARM_DBG_KEY1_ADDR(base) ((base) + (0x834UL))
#define SOC_SCTRL_SCARM_DBGEN_STAT_ADDR(base) ((base) + (0x838UL))
#define SOC_SCTRL_SCEFUSECTRL_ADDR(base) ((base) + (0x83CUL))
#define SOC_SCTRL_SCEFUSESEL_ADDR(base) ((base) + (0x840UL))
#define SOC_SCTRL_SCCHIP_ID0_ADDR(base) ((base) + (0x848UL))
#define SOC_SCTRL_SCCHIP_ID1_ADDR(base) ((base) + (0x84CUL))
#define SOC_SCTRL_SCCPUSECCTRL_ADDR(base) ((base) + (0x850UL))
#define SOC_SCTRL_SCJTAGSD_SW_SEL_ADDR(base) ((base) + (0x854UL))
#define SOC_SCTRL_SCJTAGSYS_SW_SEL_ADDR(base) ((base) + (0x858UL))
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_ADDR(base) ((base) + (0x85CUL))
#define SOC_SCTRL_SCSPMIADDR0_ADDR(base) ((base) + (0x860UL))
#define SOC_SCTRL_SCSPMIADDR1_ADDR(base) ((base) + (0x864UL))
#define SOC_SCTRL_SCSPMIADDR2_ADDR(base) ((base) + (0x868UL))
#define SOC_SCTRL_SCSPMIADDR3_ADDR(base) ((base) + (0x86CUL))
#define SOC_SCTRL_SCSPMIADDR4_ADDR(base) ((base) + (0x870UL))
#define SOC_SCTRL_SCSPMIADDR5_ADDR(base) ((base) + (0x874UL))
#define SOC_SCTRL_IOMCU_MEM_SD_IN_ADDR(base) ((base) + (0x880UL))
#define SOC_SCTRL_IOMCU_MEM_DS_IN_ADDR(base) ((base) + (0x884UL))
#define SOC_SCTRL_SCHISEEGPIOLOCK_ADDR(base) ((base) + (0x894UL))
#define SOC_SCTRL_SCHISEESPILOCK_ADDR(base) ((base) + (0x898UL))
#define SOC_SCTRL_SCHISEEI2CLOCK_ADDR(base) ((base) + (0x89CUL))
#define SOC_SCTRL_SCHISEEIOSEL_ADDR(base) ((base) + (0x8A0UL))
#define SOC_SCTRL_SCPERCTRL0_SEC_ADDR(base) ((base) + (0x8A4UL))
#define SOC_SCTRL_SCPERCTRL1_SEC_ADDR(base) ((base) + (0x8B0UL))
#define SOC_SCTRL_SCPERCTRL2_SEC_ADDR(base) ((base) + (0x8B4UL))
#define SOC_SCTRL_SCPERCTRL3_SEC_ADDR(base) ((base) + (0x8B8UL))
#define SOC_SCTRL_SCPERCTRL4_SEC_ADDR(base) ((base) + (0x8BCUL))
#define SOC_SCTRL_SCPERCTRL5_SEC_ADDR(base) ((base) + (0x8E0UL))
#define SOC_SCTRL_SCPERCTRL6_SEC_ADDR(base) ((base) + (0x8E4UL))
#define SOC_SCTRL_SCPERCTRL7_SEC_ADDR(base) ((base) + (0x8E8UL))
#define SOC_SCTRL_SCPERCTRL8_SEC_ADDR(base) ((base) + (0x8ECUL))
#define SOC_SCTRL_SCPERCTRL9_SEC_ADDR(base) ((base) + (0xE20UL))
#define SOC_SCTRL_SCPERCTRL10_SEC_ADDR(base) ((base) + (0xE24UL))
#define SOC_SCTRL_SCPERCTRL11_SEC_ADDR(base) ((base) + (0xE28UL))
#define SOC_SCTRL_SCPERSTATUS0_SEC_ADDR(base) ((base) + (0x8C0UL))
#define SOC_SCTRL_SCPERSTATUS1_SEC_ADDR(base) ((base) + (0x8C4UL))
#define SOC_SCTRL_SCPERSTATUS2_SEC_ADDR(base) ((base) + (0x8C8UL))
#define SOC_SCTRL_SCPERSTATUS3_SEC_ADDR(base) ((base) + (0x8CCUL))
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_ADDR(base) ((base) + (0x8D0UL))
#define SOC_SCTRL_SC_TCP_VOTE_STAT_ADDR(base) ((base) + (0x8D4UL))
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_ADDR(base) ((base) + (0x8D8UL))
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_ADDR(base) ((base) + (0x8DCUL))
#define SOC_SCTRL_SCPEREN0_SEC_ADDR(base) ((base) + (0x900UL))
#define SOC_SCTRL_SCPERDIS0_SEC_ADDR(base) ((base) + (0x904UL))
#define SOC_SCTRL_SCPERCLKEN0_SEC_ADDR(base) ((base) + (0x908UL))
#define SOC_SCTRL_SCTIMERCTRL_SEC_ADDR(base) ((base) + (0x940UL))
#define SOC_SCTRL_SCPERRSTEN0_SEC_ADDR(base) ((base) + (0x950UL))
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ADDR(base) ((base) + (0x954UL))
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ADDR(base) ((base) + (0x958UL))
#define SOC_SCTRL_SCPERRSTEN1_SEC_ADDR(base) ((base) + (0xA50UL))
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ADDR(base) ((base) + (0xA54UL))
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ADDR(base) ((base) + (0xA58UL))
#define SOC_SCTRL_SCPERRSTEN2_SEC_ADDR(base) ((base) + (0xB50UL))
#define SOC_SCTRL_SCPERRSTDIS2_SEC_ADDR(base) ((base) + (0xB54UL))
#define SOC_SCTRL_SCPERRSTSTAT2_SEC_ADDR(base) ((base) + (0xB58UL))
#define SOC_SCTRL_SCPERRSTEN3_SEC_ADDR(base) ((base) + (0xC00UL))
#define SOC_SCTRL_SCPERRSTDIS3_SEC_ADDR(base) ((base) + (0xC04UL))
#define SOC_SCTRL_SCPERRSTSTAT3_SEC_ADDR(base) ((base) + (0xC08UL))
#define SOC_SCTRL_SCPEREN3_SEC_ADDR(base) ((base) + (0xC10UL))
#define SOC_SCTRL_SCPERDIS3_SEC_ADDR(base) ((base) + (0xC14UL))
#define SOC_SCTRL_SCPERCLKEN3_SEC_ADDR(base) ((base) + (0xC18UL))
#define SOC_SCTRL_SCPERRSTEN4_SEC_ADDR(base) ((base) + (0xD00UL))
#define SOC_SCTRL_SCPERRSTDIS4_SEC_ADDR(base) ((base) + (0xD04UL))
#define SOC_SCTRL_SCPERRSTSTAT4_SEC_ADDR(base) ((base) + (0xD08UL))
#define SOC_SCTRL_SCSOCID0_ADDR(base) ((base) + (0xE00UL))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED0_ADDR(base) ((base) + (0xE10UL))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED1_ADDR(base) ((base) + (0xE14UL))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED2_ADDR(base) ((base) + (0xE18UL))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED3_ADDR(base) ((base) + (0xE1CUL))
#else
#define SOC_SCTRL_SCCTRL_ADDR(base) ((base) + (0x000))
#define SOC_SCTRL_SCSYSSTAT_ADDR(base) ((base) + (0x004))
#define SOC_SCTRL_SCDEEPSLEEPED_ADDR(base) ((base) + (0x008))
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_ADDR(base) ((base) + (0x014))
#define SOC_SCTRL_SCEFUSEDATA2_ADDR(base) ((base) + (0x00C))
#define SOC_SCTRL_SCEFUSEDATA3_ADDR(base) ((base) + (0x010))
#define SOC_SCTRL_SCEFUSEDATA4_ADDR(base) ((base) + (0x050))
#define SOC_SCTRL_SCEFUSEDATA5_ADDR(base) ((base) + (0x054))
#define SOC_SCTRL_SCEFUSEDATA6_ADDR(base) ((base) + (0x058))
#define SOC_SCTRL_SCEFUSEDATA7_ADDR(base) ((base) + (0x05C))
#define SOC_SCTRL_SCEFUSEDATA8_ADDR(base) ((base) + (0x150))
#define SOC_SCTRL_SCEFUSEDATA9_ADDR(base) ((base) + (0x154))
#define SOC_SCTRL_SCXTALCTRL_ADDR(base) ((base) + (0x020))
#define SOC_SCTRL_SCXTALTIMEOUT0_ADDR(base) ((base) + (0x024))
#define SOC_SCTRL_SCXTALTIMEOUT1_ADDR(base) ((base) + (0x028))
#define SOC_SCTRL_SCXTALSTAT_ADDR(base) ((base) + (0x02C))
#define SOC_SCTRL_SCSW2FLLBYPASS_ADDR(base) ((base) + (0x030))
#define SOC_SCTRL_SCFLLLOCK_FILTER_ADDR(base) ((base) + (0x034))
#define SOC_SCTRL_SCLPSTATCFG_ADDR(base) ((base) + (0x038))
#define SOC_SCTRL_SCPERIISOBYPASS_ADDR(base) ((base) + (0x03c))
#define SOC_SCTRL_SCISOEN_ADDR(base) ((base) + (0x040))
#define SOC_SCTRL_SCISODIS_ADDR(base) ((base) + (0x044))
#define SOC_SCTRL_SCISOSTAT_ADDR(base) ((base) + (0x048))
#define SOC_SCTRL_SCPWREN_ADDR(base) ((base) + (0x060))
#define SOC_SCTRL_SCPWRDIS_ADDR(base) ((base) + (0x064))
#define SOC_SCTRL_SCPWRSTAT_ADDR(base) ((base) + (0x068))
#define SOC_SCTRL_SCPWRACK_ADDR(base) ((base) + (0x06C))
#define SOC_SCTRL_SCPERPWRDOWNTIME_ADDR(base) ((base) + (0x070))
#define SOC_SCTRL_SCPERPWRUPTIME_ADDR(base) ((base) + (0x074))
#define SOC_SCTRL_SC_ASP_PWRCFG0_ADDR(base) ((base) + (0x080))
#define SOC_SCTRL_SC_ASP_PWRCFG1_ADDR(base) ((base) + (0x084))
#define SOC_SCTRL_SCINT_GATHER_STAT_ADDR(base) ((base) + (0x0A0))
#define SOC_SCTRL_SCINT_MASK_ADDR(base) ((base) + (0x0A4))
#define SOC_SCTRL_SCINT_STAT_ADDR(base) ((base) + (0x0A8))
#define SOC_SCTRL_SCDRX_INT_CFG_ADDR(base) ((base) + (0x0AC))
#define SOC_SCTRL_SCLPMCUWFI_INT_ADDR(base) ((base) + (0x0B0))
#define SOC_SCTRL_SCINT_MASK1_ADDR(base) ((base) + (0x0B4))
#define SOC_SCTRL_SCINT_STAT1_ADDR(base) ((base) + (0x0B8))
#define SOC_SCTRL_SCINT_MASK2_ADDR(base) ((base) + (0x0BC))
#define SOC_SCTRL_SCINT_STAT2_ADDR(base) ((base) + (0x0C0))
#define SOC_SCTRL_SCINT_MASK3_ADDR(base) ((base) + (0x0C4))
#define SOC_SCTRL_SCINT_STAT3_ADDR(base) ((base) + (0x0C8))
#define SOC_SCTRL_SCINT_MASK4_ADDR(base) ((base) + (0x090))
#define SOC_SCTRL_SCINT_STAT4_ADDR(base) ((base) + (0x094))
#define SOC_SCTRL_SCINT_MASK5_ADDR(base) ((base) + (0x098))
#define SOC_SCTRL_SCINT_STAT5_ADDR(base) ((base) + (0x09C))
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_ADDR(base) ((base) + (0x0E0))
#define SOC_SCTRL_SCLBINTPLL1_CTRL1_ADDR(base) ((base) + (0x0E4))
#define SOC_SCTRL_SCLBINTPLL1_CTRL2_ADDR(base) ((base) + (0x0E8))
#define SOC_SCTRL_SCLBINTPLL1_STAT_ADDR(base) ((base) + (0x0EC))
#define SOC_SCTRL_SCPPLLCTRL0_ADDR(base) ((base) + (0x100))
#define SOC_SCTRL_SCPPLLCTRL1_ADDR(base) ((base) + (0x104))
#define SOC_SCTRL_SCPPLLSSCCTRL_ADDR(base) ((base) + (0x108))
#define SOC_SCTRL_SCPPLLSTAT_ADDR(base) ((base) + (0x10C))
#define SOC_SCTRL_SCFPLLCTRL0_ADDR(base) ((base) + (0x120))
#define SOC_SCTRL_SCFPLLCTRL1_ADDR(base) ((base) + (0x124))
#define SOC_SCTRL_SCFPLLCTRL2_ADDR(base) ((base) + (0x128))
#define SOC_SCTRL_SCFPLLCTRL3_ADDR(base) ((base) + (0x12C))
#define SOC_SCTRL_SCFPLLSTAT_ADDR(base) ((base) + (0x130))
#define SOC_SCTRL_SCPEREN0_ADDR(base) ((base) + (0x160))
#define SOC_SCTRL_SCPERDIS0_ADDR(base) ((base) + (0x164))
#define SOC_SCTRL_SCPERCLKEN0_ADDR(base) ((base) + (0x168))
#define SOC_SCTRL_SCPERSTAT0_ADDR(base) ((base) + (0x16C))
#define SOC_SCTRL_SCPEREN1_ADDR(base) ((base) + (0x170))
#define SOC_SCTRL_SCPERDIS1_ADDR(base) ((base) + (0x174))
#define SOC_SCTRL_SCPERCLKEN1_ADDR(base) ((base) + (0x178))
#define SOC_SCTRL_SCPERSTAT1_ADDR(base) ((base) + (0x17C))
#define SOC_SCTRL_SCPEREN2_ADDR(base) ((base) + (0x190))
#define SOC_SCTRL_SCPERDIS2_ADDR(base) ((base) + (0x194))
#define SOC_SCTRL_SCPERCLKEN2_ADDR(base) ((base) + (0x198))
#define SOC_SCTRL_SCPERSTAT2_ADDR(base) ((base) + (0x19C))
#define SOC_SCTRL_SCPERSTAT3_ADDR(base) ((base) + (0x1A0))
#define SOC_SCTRL_SCPERCLKEN3_ADDR(base) ((base) + (0x1A4))
#define SOC_SCTRL_SCPEREN4_ADDR(base) ((base) + (0x1B0))
#define SOC_SCTRL_SCPERDIS4_ADDR(base) ((base) + (0x1B4))
#define SOC_SCTRL_SCPERCLKEN4_ADDR(base) ((base) + (0x1B8))
#define SOC_SCTRL_SCPERSTAT4_ADDR(base) ((base) + (0x1BC))
#define SOC_SCTRL_SCPEREN5_ADDR(base) ((base) + (0x1C0))
#define SOC_SCTRL_SCPERDIS5_ADDR(base) ((base) + (0x1C4))
#define SOC_SCTRL_SCPERCLKEN5_ADDR(base) ((base) + (0x1C8))
#define SOC_SCTRL_SCPERSTAT5_ADDR(base) ((base) + (0x1CC))
#define SOC_SCTRL_SCPERRSTEN0_ADDR(base) ((base) + (0x200))
#define SOC_SCTRL_SCPERRSTDIS0_ADDR(base) ((base) + (0x204))
#define SOC_SCTRL_SCPERRSTSTAT0_ADDR(base) ((base) + (0x208))
#define SOC_SCTRL_SCPERRSTEN1_ADDR(base) ((base) + (0x20C))
#define SOC_SCTRL_SCPERRSTDIS1_ADDR(base) ((base) + (0x210))
#define SOC_SCTRL_SCPERRSTSTAT1_ADDR(base) ((base) + (0x214))
#define SOC_SCTRL_SCPERRSTEN2_ADDR(base) ((base) + (0x218))
#define SOC_SCTRL_SCPERRSTDIS2_ADDR(base) ((base) + (0x21C))
#define SOC_SCTRL_SCPERRSTSTAT2_ADDR(base) ((base) + (0x220))
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_ADDR(base) ((base) + (0x240))
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_ADDR(base) ((base) + (0x244))
#define SOC_SCTRL_SCCLKDIV0_ADDR(base) ((base) + (0x250))
#define SOC_SCTRL_SCCLKDIV1_ADDR(base) ((base) + (0x254))
#define SOC_SCTRL_SCCLKDIV2_ADDR(base) ((base) + (0x258))
#define SOC_SCTRL_SCCLKDIV3_ADDR(base) ((base) + (0x25C))
#define SOC_SCTRL_SCCLKDIV4_ADDR(base) ((base) + (0x260))
#define SOC_SCTRL_SCCLKDIV5_ADDR(base) ((base) + (0x264))
#define SOC_SCTRL_SCCLKDIV6_ADDR(base) ((base) + (0x268))
#define SOC_SCTRL_SCCLKDIV7_ADDR(base) ((base) + (0x26C))
#define SOC_SCTRL_SCCLKDIV8_ADDR(base) ((base) + (0x270))
#define SOC_SCTRL_SCCLKDIV9_ADDR(base) ((base) + (0x274))
#define SOC_SCTRL_SCUFS_AUTODIV_ADDR(base) ((base) + (0x278))
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_ADDR(base) ((base) + (0x27C))
#define SOC_SCTRL_SCCLKDIV10_ADDR(base) ((base) + (0x280))
#define SOC_SCTRL_SCCLKDIV11_ADDR(base) ((base) + (0x284))
#define SOC_SCTRL_SCCLKDIV12_ADDR(base) ((base) + (0x288))
#define SOC_SCTRL_SCCLKDIV13_ADDR(base) ((base) + (0x28C))
#define SOC_SCTRL_SCCLKDIV14_ADDR(base) ((base) + (0x290))
#define SOC_SCTRL_SCCLKDIV15_ADDR(base) ((base) + (0x294))
#define SOC_SCTRL_SCCLKDIV16_ADDR(base) ((base) + (0x298))
#define SOC_SCTRL_SCPERCTRL0_ADDR(base) ((base) + (0x300))
#define SOC_SCTRL_SCPERCTRL1_ADDR(base) ((base) + (0x304))
#define SOC_SCTRL_SCPERCTRL2_ADDR(base) ((base) + (0x308))
#define SOC_SCTRL_SCPERCTRL3_ADDR(base) ((base) + (0x30C))
#define SOC_SCTRL_SCPERCTRL4_ADDR(base) ((base) + (0x310))
#define SOC_SCTRL_SCPERCTRL5_ADDR(base) ((base) + (0x314))
#define SOC_SCTRL_SCPERCTRL6_ADDR(base) ((base) + (0x318))
#define SOC_SCTRL_SCPERCTRL7_ADDR(base) ((base) + (0x31C))
#define SOC_SCTRL_SCPERCTRL8_ADDR(base) ((base) + (0x8A8))
#define SOC_SCTRL_SCPERCTRL8_ADD_ADDR(base) ((base) + (0x8AC))
#define SOC_SCTRL_SCPERCTRL9_ADDR(base) ((base) + (0x324))
#define SOC_SCTRL_SCPERCTRL10_ADDR(base) ((base) + (0x328))
#define SOC_SCTRL_SCPERCTRL11_ADDR(base) ((base) + (0x32C))
#define SOC_SCTRL_SCPERCTRL12_ADDR(base) ((base) + (0x330))
#define SOC_SCTRL_SCPERCTRL13_ADDR(base) ((base) + (0x334))
#define SOC_SCTRL_SCPERCTRL14_ADDR(base) ((base) + (0x338))
#define SOC_SCTRL_SCPERCTRL15_ADDR(base) ((base) + (0x33C))
#define SOC_SCTRL_SCPERCTRL16_ADDR(base) ((base) + (0x340))
#define SOC_SCTRL_SCPERCTRL17_ADDR(base) ((base) + (0x344))
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_ADDR(base) ((base) + (0x354))
#define SOC_SCTRL_SCPERSTATUS17_ADDR(base) ((base) + (0x358))
#define SOC_SCTRL_SCPERSTATUS16_ADDR(base) ((base) + (0x35C))
#define SOC_SCTRL_SCPERSTATUS0_ADDR(base) ((base) + (0x360))
#define SOC_SCTRL_SCPERSTATUS1_ADDR(base) ((base) + (0x364))
#define SOC_SCTRL_SCPERSTATUS2_ADDR(base) ((base) + (0x368))
#define SOC_SCTRL_SCPERSTATUS3_ADDR(base) ((base) + (0x36C))
#define SOC_SCTRL_SCPERSTATUS4_ADDR(base) ((base) + (0x370))
#define SOC_SCTRL_SCPERSTATUS5_ADDR(base) ((base) + (0x374))
#define SOC_SCTRL_SCPERSTATUS6_ADDR(base) ((base) + (0x378))
#define SOC_SCTRL_SCPERSTATUS7_ADDR(base) ((base) + (0x37C))
#define SOC_SCTRL_SCPERSTATUS8_ADDR(base) ((base) + (0x380))
#define SOC_SCTRL_SCPERSTATUS9_ADDR(base) ((base) + (0x384))
#define SOC_SCTRL_SCPERSTATUS10_ADDR(base) ((base) + (0x388))
#define SOC_SCTRL_SCPERSTATUS11_ADDR(base) ((base) + (0x38C))
#define SOC_SCTRL_SCPERSTATUS12_ADDR(base) ((base) + (0x390))
#define SOC_SCTRL_SCPERSTATUS13_ADDR(base) ((base) + (0x394))
#define SOC_SCTRL_SCPERSTATUS14_ADDR(base) ((base) + (0x398))
#define SOC_SCTRL_SCPERSTATUS15_ADDR(base) ((base) + (0x39C))
#define SOC_SCTRL_SCINNERSTAT_ADDR(base) ((base) + (0x3A0))
#define SOC_SCTRL_SCINNERCTRL_ADDR(base) ((base) + (0x3A4))
#define SOC_SCTRL_SC_MEM_CTRL_SD_ADDR(base) ((base) + (0x3A8))
#define SOC_SCTRL_SC_MEM_STAT_SD_ADDR(base) ((base) + (0x3AC))
#define SOC_SCTRL_SCTIMERCTRL0_ADDR(base) ((base) + (0x3C0))
#define SOC_SCTRL_SCTIMERCTRL1_ADDR(base) ((base) + (0x3C4))
#define SOC_SCTRL_SCTIMERCTRL2_ADDR(base) ((base) + (0x3C8))
#define SOC_SCTRL_SC_SECOND_INT_MASK_ADDR(base) ((base) + (0x3D0))
#define SOC_SCTRL_SC_SECOND_INT_ORG_ADDR(base) ((base) + (0x3D4))
#define SOC_SCTRL_SC_SECOND_INT_OUT_ADDR(base) ((base) + (0x3D8))
#define SOC_SCTRL_SCMRBBUSYSTAT_ADDR(base) ((base) + (0x3FC))
#define SOC_SCTRL_SCSWADDR_ADDR(base) ((base) + (0x400))
#define SOC_SCTRL_SCDDRADDR_ADDR(base) ((base) + (0x404))
#define SOC_SCTRL_SCDDRDATA_ADDR(base) ((base) + (0x408))
#define SOC_SCTRL_SCBAKDATA0_ADDR(base) ((base) + (0x40C))
#define SOC_SCTRL_SCBAKDATA1_ADDR(base) ((base) + (0x410))
#define SOC_SCTRL_SCBAKDATA2_ADDR(base) ((base) + (0x414))
#define SOC_SCTRL_SCBAKDATA3_ADDR(base) ((base) + (0x418))
#define SOC_SCTRL_SCBAKDATA4_ADDR(base) ((base) + (0x41C))
#define SOC_SCTRL_SCBAKDATA5_ADDR(base) ((base) + (0x420))
#define SOC_SCTRL_SCBAKDATA6_ADDR(base) ((base) + (0x424))
#define SOC_SCTRL_SCBAKDATA7_ADDR(base) ((base) + (0x428))
#define SOC_SCTRL_SCBAKDATA8_ADDR(base) ((base) + (0x42C))
#define SOC_SCTRL_SCBAKDATA9_ADDR(base) ((base) + (0x430))
#define SOC_SCTRL_SCBAKDATA10_ADDR(base) ((base) + (0x434))
#define SOC_SCTRL_SCBAKDATA11_ADDR(base) ((base) + (0x438))
#define SOC_SCTRL_SCBAKDATA12_ADDR(base) ((base) + (0x43C))
#define SOC_SCTRL_SCBAKDATA13_ADDR(base) ((base) + (0x440))
#define SOC_SCTRL_SCBAKDATA14_ADDR(base) ((base) + (0x444))
#define SOC_SCTRL_SCBAKDATA15_ADDR(base) ((base) + (0x448))
#define SOC_SCTRL_SCBAKDATA16_ADDR(base) ((base) + (0x44C))
#define SOC_SCTRL_SCBAKDATA17_ADDR(base) ((base) + (0x450))
#define SOC_SCTRL_SCBAKDATA18_ADDR(base) ((base) + (0x454))
#define SOC_SCTRL_SCBAKDATA19_ADDR(base) ((base) + (0x458))
#define SOC_SCTRL_SCBAKDATA20_ADDR(base) ((base) + (0x45C))
#define SOC_SCTRL_SCBAKDATA21_ADDR(base) ((base) + (0x460))
#define SOC_SCTRL_SCBAKDATA22_ADDR(base) ((base) + (0x464))
#define SOC_SCTRL_SCBAKDATA23_ADDR(base) ((base) + (0x468))
#define SOC_SCTRL_SCBAKDATA24_ADDR(base) ((base) + (0x46C))
#define SOC_SCTRL_SCBAKDATA25_ADDR(base) ((base) + (0x470))
#define SOC_SCTRL_SCBAKDATA26_ADDR(base) ((base) + (0x474))
#define SOC_SCTRL_SCBAKDATA27_ADDR(base) ((base) + (0x478))
#define SOC_SCTRL_SCBAKDATA28_ADDR(base) ((base) + (0x47C))
#define SOC_SCTRL_SCLPMCUCLKEN_ADDR(base) ((base) + (0x480))
#define SOC_SCTRL_SCLPMCUCLKDIS_ADDR(base) ((base) + (0x484))
#define SOC_SCTRL_SCLPMCUCLKSTAT_ADDR(base) ((base) + (0x488))
#define SOC_SCTRL_SCLPMCURSTEN_ADDR(base) ((base) + (0x500))
#define SOC_SCTRL_SCLPMCURSTDIS_ADDR(base) ((base) + (0x504))
#define SOC_SCTRL_SCLPMCURSTSTAT_ADDR(base) ((base) + (0x508))
#define SOC_SCTRL_SCLPMCUCTRL_ADDR(base) ((base) + (0x510))
#define SOC_SCTRL_SCLPMCUSTAT_ADDR(base) ((base) + (0x514))
#define SOC_SCTRL_SCLPMCURAMCTRL_ADDR(base) ((base) + (0x518))
#define SOC_SCTRL_SCBBPDRXSTAT0_ADDR(base) ((base) + (0x530))
#define SOC_SCTRL_SCBBPDRXSTAT1_ADDR(base) ((base) + (0x534))
#define SOC_SCTRL_SCBBPDRXSTAT2_ADDR(base) ((base) + (0x538))
#define SOC_SCTRL_SCBBPDRXSTAT3_ADDR(base) ((base) + (0x53C))
#define SOC_SCTRL_SCBBPDRXSTAT4_ADDR(base) ((base) + (0x540))
#define SOC_SCTRL_SCA53_EVENT_MASK_ADDR(base) ((base) + (0x550))
#define SOC_SCTRL_SCA57_EVENT_MASK_ADDR(base) ((base) + (0x554))
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_ADDR(base) ((base) + (0x558))
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_ADDR(base) ((base) + (0x55C))
#define SOC_SCTRL_SCMCPU_EVENT_MASK_ADDR(base) ((base) + (0x560))
#define SOC_SCTRL_SCISPA7_EVENT_MASK_ADDR(base) ((base) + (0x568))
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_ADDR(base) ((base) + (0x56C))
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_ADDR(base) ((base) + (0x570))
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_ADDR(base) ((base) + (0x574))
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_ADDR(base) ((base) + (0x578))
#define SOC_SCTRL_SCEPS_EVENT_MASK_ADDR(base) ((base) + (0x57C))
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_ADDR(base) ((base) + (0x544))
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_ADDR(base) ((base) + (0x548))
#define SOC_SCTRL_SCEVENT_STAT_ADDR(base) ((base) + (0x564))
#define SOC_SCTRL_SCIOMCUCLKCTRL_ADDR(base) ((base) + (0x580))
#define SOC_SCTRL_SCIOMCUCLKSTAT_ADDR(base) ((base) + (0x584))
#define SOC_SCTRL_SCIOMCUCTRL_ADDR(base) ((base) + (0x588))
#define SOC_SCTRL_SCIOMCUSTAT_ADDR(base) ((base) + (0x58C))
#define SOC_SCTRL_SCPERSTATUS18_ADDR(base) ((base) + (0x590))
#define SOC_SCTRL_SCPERSTATUS19_ADDR(base) ((base) + (0x594))
#define SOC_SCTRL_SCPERSTATUS20_ADDR(base) ((base) + (0x598))
#define SOC_SCTRL_SCPERSTATUS21_ADDR(base) ((base) + (0x59C))
#define SOC_SCTRL_SCPERSTATUS22_ADDR(base) ((base) + (0x5A0))
#define SOC_SCTRL_SCPERSTATUS23_ADDR(base) ((base) + (0x5A4))
#define SOC_SCTRL_SCPERSTATUS24_ADDR(base) ((base) + (0x5A8))
#define SOC_SCTRL_SCPERSTATUS25_ADDR(base) ((base) + (0x5AC))
#define SOC_SCTRL_SCPERSTATUS26_ADDR(base) ((base) + (0x5B0))
#define SOC_SCTRL_SCPERSTATUS27_ADDR(base) ((base) + (0x5B4))
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_ADDR(base) ((base) + (0x600))
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_ADDR(base) ((base) + (0x604))
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_ADDR(base) ((base) + (0x608))
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_ADDR(base) ((base) + (0x60C))
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_ADDR(base) ((base) + (0x610))
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_ADDR(base) ((base) + (0x614))
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_ADDR(base) ((base) + (0x9E8))
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_ADDR(base) ((base) + (0x9EC))
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_ADDR(base) ((base) + (0x9F0))
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_ADDR(base) ((base) + (0x9F4))
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_ADDR(base) ((base) + (0x9F8))
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_ADDR(base) ((base) + (0x9FC))
#define SOC_SCTRL_SCJTAG_SEL_ADDR(base) ((base) + (0x800))
#define SOC_SCTRL_SCCFG_DJTAG_ADDR(base) ((base) + (0x814))
#define SOC_SCTRL_SCCP15_DISABLE_ADDR(base) ((base) + (0x818))
#define SOC_SCTRL_SCCLKCNTSTAT_ADDR(base) ((base) + (0x81C))
#define SOC_SCTRL_SCCLKCNTCFG_ADDR(base) ((base) + (0x820))
#define SOC_SCTRL_SCCLKMONCTRL_ADDR(base) ((base) + (0x824))
#define SOC_SCTRL_SCCLKMONINT_ADDR(base) ((base) + (0x828))
#define SOC_SCTRL_SCCFG_ARM_DBGEN_ADDR(base) ((base) + (0x82C))
#define SOC_SCTRL_SCARM_DBG_KEY0_ADDR(base) ((base) + (0x830))
#define SOC_SCTRL_SCARM_DBG_KEY1_ADDR(base) ((base) + (0x834))
#define SOC_SCTRL_SCARM_DBGEN_STAT_ADDR(base) ((base) + (0x838))
#define SOC_SCTRL_SCEFUSECTRL_ADDR(base) ((base) + (0x83C))
#define SOC_SCTRL_SCEFUSESEL_ADDR(base) ((base) + (0x840))
#define SOC_SCTRL_SCCHIP_ID0_ADDR(base) ((base) + (0x848))
#define SOC_SCTRL_SCCHIP_ID1_ADDR(base) ((base) + (0x84C))
#define SOC_SCTRL_SCCPUSECCTRL_ADDR(base) ((base) + (0x850))
#define SOC_SCTRL_SCJTAGSD_SW_SEL_ADDR(base) ((base) + (0x854))
#define SOC_SCTRL_SCJTAGSYS_SW_SEL_ADDR(base) ((base) + (0x858))
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_ADDR(base) ((base) + (0x85C))
#define SOC_SCTRL_SCSPMIADDR0_ADDR(base) ((base) + (0x860))
#define SOC_SCTRL_SCSPMIADDR1_ADDR(base) ((base) + (0x864))
#define SOC_SCTRL_SCSPMIADDR2_ADDR(base) ((base) + (0x868))
#define SOC_SCTRL_SCSPMIADDR3_ADDR(base) ((base) + (0x86C))
#define SOC_SCTRL_SCSPMIADDR4_ADDR(base) ((base) + (0x870))
#define SOC_SCTRL_SCSPMIADDR5_ADDR(base) ((base) + (0x874))
#define SOC_SCTRL_IOMCU_MEM_SD_IN_ADDR(base) ((base) + (0x880))
#define SOC_SCTRL_IOMCU_MEM_DS_IN_ADDR(base) ((base) + (0x884))
#define SOC_SCTRL_SCHISEEGPIOLOCK_ADDR(base) ((base) + (0x894))
#define SOC_SCTRL_SCHISEESPILOCK_ADDR(base) ((base) + (0x898))
#define SOC_SCTRL_SCHISEEI2CLOCK_ADDR(base) ((base) + (0x89C))
#define SOC_SCTRL_SCHISEEIOSEL_ADDR(base) ((base) + (0x8A0))
#define SOC_SCTRL_SCPERCTRL0_SEC_ADDR(base) ((base) + (0x8A4))
#define SOC_SCTRL_SCPERCTRL1_SEC_ADDR(base) ((base) + (0x8B0))
#define SOC_SCTRL_SCPERCTRL2_SEC_ADDR(base) ((base) + (0x8B4))
#define SOC_SCTRL_SCPERCTRL3_SEC_ADDR(base) ((base) + (0x8B8))
#define SOC_SCTRL_SCPERCTRL4_SEC_ADDR(base) ((base) + (0x8BC))
#define SOC_SCTRL_SCPERCTRL5_SEC_ADDR(base) ((base) + (0x8E0))
#define SOC_SCTRL_SCPERCTRL6_SEC_ADDR(base) ((base) + (0x8E4))
#define SOC_SCTRL_SCPERCTRL7_SEC_ADDR(base) ((base) + (0x8E8))
#define SOC_SCTRL_SCPERCTRL8_SEC_ADDR(base) ((base) + (0x8EC))
#define SOC_SCTRL_SCPERCTRL9_SEC_ADDR(base) ((base) + (0xE20))
#define SOC_SCTRL_SCPERCTRL10_SEC_ADDR(base) ((base) + (0xE24))
#define SOC_SCTRL_SCPERCTRL11_SEC_ADDR(base) ((base) + (0xE28))
#define SOC_SCTRL_SCPERSTATUS0_SEC_ADDR(base) ((base) + (0x8C0))
#define SOC_SCTRL_SCPERSTATUS1_SEC_ADDR(base) ((base) + (0x8C4))
#define SOC_SCTRL_SCPERSTATUS2_SEC_ADDR(base) ((base) + (0x8C8))
#define SOC_SCTRL_SCPERSTATUS3_SEC_ADDR(base) ((base) + (0x8CC))
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_ADDR(base) ((base) + (0x8D0))
#define SOC_SCTRL_SC_TCP_VOTE_STAT_ADDR(base) ((base) + (0x8D4))
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_ADDR(base) ((base) + (0x8D8))
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_ADDR(base) ((base) + (0x8DC))
#define SOC_SCTRL_SCPEREN0_SEC_ADDR(base) ((base) + (0x900))
#define SOC_SCTRL_SCPERDIS0_SEC_ADDR(base) ((base) + (0x904))
#define SOC_SCTRL_SCPERCLKEN0_SEC_ADDR(base) ((base) + (0x908))
#define SOC_SCTRL_SCTIMERCTRL_SEC_ADDR(base) ((base) + (0x940))
#define SOC_SCTRL_SCPERRSTEN0_SEC_ADDR(base) ((base) + (0x950))
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ADDR(base) ((base) + (0x954))
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ADDR(base) ((base) + (0x958))
#define SOC_SCTRL_SCPERRSTEN1_SEC_ADDR(base) ((base) + (0xA50))
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ADDR(base) ((base) + (0xA54))
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ADDR(base) ((base) + (0xA58))
#define SOC_SCTRL_SCPERRSTEN2_SEC_ADDR(base) ((base) + (0xB50))
#define SOC_SCTRL_SCPERRSTDIS2_SEC_ADDR(base) ((base) + (0xB54))
#define SOC_SCTRL_SCPERRSTSTAT2_SEC_ADDR(base) ((base) + (0xB58))
#define SOC_SCTRL_SCPERRSTEN3_SEC_ADDR(base) ((base) + (0xC00))
#define SOC_SCTRL_SCPERRSTDIS3_SEC_ADDR(base) ((base) + (0xC04))
#define SOC_SCTRL_SCPERRSTSTAT3_SEC_ADDR(base) ((base) + (0xC08))
#define SOC_SCTRL_SCPEREN3_SEC_ADDR(base) ((base) + (0xC10))
#define SOC_SCTRL_SCPERDIS3_SEC_ADDR(base) ((base) + (0xC14))
#define SOC_SCTRL_SCPERCLKEN3_SEC_ADDR(base) ((base) + (0xC18))
#define SOC_SCTRL_SCPERRSTEN4_SEC_ADDR(base) ((base) + (0xD00))
#define SOC_SCTRL_SCPERRSTDIS4_SEC_ADDR(base) ((base) + (0xD04))
#define SOC_SCTRL_SCPERRSTSTAT4_SEC_ADDR(base) ((base) + (0xD08))
#define SOC_SCTRL_SCSOCID0_ADDR(base) ((base) + (0xE00))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED0_ADDR(base) ((base) + (0xE10))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED1_ADDR(base) ((base) + (0xE14))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED2_ADDR(base) ((base) + (0xE18))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED3_ADDR(base) ((base) + (0xE1C))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mode_ctrl_soft : 3;
        unsigned int sys_mode : 4;
        unsigned int reserved_0 : 18;
        unsigned int deepsleep_en : 1;
        unsigned int reserved_1 : 2;
        unsigned int sc_pmu_type_sel : 1;
        unsigned int reserved_2 : 3;
    } reg;
} SOC_SCTRL_SCCTRL_UNION;
#endif
#define SOC_SCTRL_SCCTRL_mode_ctrl_soft_START (0)
#define SOC_SCTRL_SCCTRL_mode_ctrl_soft_END (2)
#define SOC_SCTRL_SCCTRL_sys_mode_START (3)
#define SOC_SCTRL_SCCTRL_sys_mode_END (6)
#define SOC_SCTRL_SCCTRL_deepsleep_en_START (25)
#define SOC_SCTRL_SCCTRL_deepsleep_en_END (25)
#define SOC_SCTRL_SCCTRL_sc_pmu_type_sel_START (28)
#define SOC_SCTRL_SCCTRL_sc_pmu_type_sel_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fcm_tsensor0_rst_req : 1;
        unsigned int fcm_tsensor1_rst_req : 1;
        unsigned int g3d_tsensor_rst_req : 1;
        unsigned int soft_rst_req : 1;
        unsigned int wd0_rst_req : 1;
        unsigned int wd1_rst_req : 1;
        unsigned int lpmcu_rst_req : 1;
        unsigned int modem_tsensor_rst_req : 1;
        unsigned int iomcu_rst_req : 1;
        unsigned int asp_subsys_wd_req : 1;
        unsigned int modem5g_wd_rst_req : 1;
        unsigned int modem_wd_rst_req : 1;
        unsigned int ivp32dsp_wd_rst_req : 1;
        unsigned int isp_a7_wd_rst_req : 1;
        unsigned int wait_ddr_selfreflash_timeout : 1;
        unsigned int ddr_fatal_intr : 4;
        unsigned int ddr_uce_wd_rst_req : 4;
        unsigned int ao_wd_rst_req : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 7;
    } reg;
} SOC_SCTRL_SCSYSSTAT_UNION;
#endif
#define SOC_SCTRL_SCSYSSTAT_fcm_tsensor0_rst_req_START (0)
#define SOC_SCTRL_SCSYSSTAT_fcm_tsensor0_rst_req_END (0)
#define SOC_SCTRL_SCSYSSTAT_fcm_tsensor1_rst_req_START (1)
#define SOC_SCTRL_SCSYSSTAT_fcm_tsensor1_rst_req_END (1)
#define SOC_SCTRL_SCSYSSTAT_g3d_tsensor_rst_req_START (2)
#define SOC_SCTRL_SCSYSSTAT_g3d_tsensor_rst_req_END (2)
#define SOC_SCTRL_SCSYSSTAT_soft_rst_req_START (3)
#define SOC_SCTRL_SCSYSSTAT_soft_rst_req_END (3)
#define SOC_SCTRL_SCSYSSTAT_wd0_rst_req_START (4)
#define SOC_SCTRL_SCSYSSTAT_wd0_rst_req_END (4)
#define SOC_SCTRL_SCSYSSTAT_wd1_rst_req_START (5)
#define SOC_SCTRL_SCSYSSTAT_wd1_rst_req_END (5)
#define SOC_SCTRL_SCSYSSTAT_lpmcu_rst_req_START (6)
#define SOC_SCTRL_SCSYSSTAT_lpmcu_rst_req_END (6)
#define SOC_SCTRL_SCSYSSTAT_modem_tsensor_rst_req_START (7)
#define SOC_SCTRL_SCSYSSTAT_modem_tsensor_rst_req_END (7)
#define SOC_SCTRL_SCSYSSTAT_iomcu_rst_req_START (8)
#define SOC_SCTRL_SCSYSSTAT_iomcu_rst_req_END (8)
#define SOC_SCTRL_SCSYSSTAT_asp_subsys_wd_req_START (9)
#define SOC_SCTRL_SCSYSSTAT_asp_subsys_wd_req_END (9)
#define SOC_SCTRL_SCSYSSTAT_modem5g_wd_rst_req_START (10)
#define SOC_SCTRL_SCSYSSTAT_modem5g_wd_rst_req_END (10)
#define SOC_SCTRL_SCSYSSTAT_modem_wd_rst_req_START (11)
#define SOC_SCTRL_SCSYSSTAT_modem_wd_rst_req_END (11)
#define SOC_SCTRL_SCSYSSTAT_ivp32dsp_wd_rst_req_START (12)
#define SOC_SCTRL_SCSYSSTAT_ivp32dsp_wd_rst_req_END (12)
#define SOC_SCTRL_SCSYSSTAT_isp_a7_wd_rst_req_START (13)
#define SOC_SCTRL_SCSYSSTAT_isp_a7_wd_rst_req_END (13)
#define SOC_SCTRL_SCSYSSTAT_wait_ddr_selfreflash_timeout_START (14)
#define SOC_SCTRL_SCSYSSTAT_wait_ddr_selfreflash_timeout_END (14)
#define SOC_SCTRL_SCSYSSTAT_ddr_fatal_intr_START (15)
#define SOC_SCTRL_SCSYSSTAT_ddr_fatal_intr_END (18)
#define SOC_SCTRL_SCSYSSTAT_ddr_uce_wd_rst_req_START (19)
#define SOC_SCTRL_SCSYSSTAT_ddr_uce_wd_rst_req_END (22)
#define SOC_SCTRL_SCSYSSTAT_ao_wd_rst_req_START (23)
#define SOC_SCTRL_SCSYSSTAT_ao_wd_rst_req_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int deepsleeped : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int sleeped : 1;
        unsigned int reserved_3 : 15;
        unsigned int reserved_4 : 11;
        unsigned int kce_auth_pass : 1;
    } reg;
} SOC_SCTRL_SCDEEPSLEEPED_UNION;
#endif
#define SOC_SCTRL_SCDEEPSLEEPED_deepsleeped_START (0)
#define SOC_SCTRL_SCDEEPSLEEPED_deepsleeped_END (0)
#define SOC_SCTRL_SCDEEPSLEEPED_sleeped_START (4)
#define SOC_SCTRL_SCDEEPSLEEPED_sleeped_END (4)
#define SOC_SCTRL_SCDEEPSLEEPED_kce_auth_pass_START (31)
#define SOC_SCTRL_SCDEEPSLEEPED_kce_auth_pass_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_wakeup_sctrl2m3_stat : 1;
        unsigned int intr_wakeup_sys_ff2 : 1;
        unsigned int intr_wakeup_sctrl2m3_en_at_suspend : 1;
        unsigned int reserved_0 : 1;
        unsigned int intr_wakeup_sctrl2m3_clr_at_suspend : 1;
        unsigned int reserved_1 : 3;
        unsigned int intr_wakeup_sctrl2m3_en : 1;
        unsigned int reserved_2 : 22;
        unsigned int reserved_3 : 1;
    } reg;
} SOC_SCTRL_SC_INTR_WAKEUP_CTRL_UNION;
#endif
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sctrl2m3_stat_START (0)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sctrl2m3_stat_END (0)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sys_ff2_START (1)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sys_ff2_END (1)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sctrl2m3_en_at_suspend_START (2)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sctrl2m3_en_at_suspend_END (2)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sctrl2m3_clr_at_suspend_START (4)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sctrl2m3_clr_at_suspend_END (4)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sctrl2m3_en_START (8)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sctrl2m3_en_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCEFUSEDATA2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCEFUSEDATA3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCEFUSEDATA4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCEFUSEDATA5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCEFUSEDATA6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCEFUSEDATA7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCEFUSEDATA8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCEFUSEDATA9_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tcxopresel_apb : 1;
        unsigned int tcxohardcon_bypass : 1;
        unsigned int tcxosel_apb : 1;
        unsigned int tcxosoft_apb : 1;
        unsigned int defau_tcxo : 1;
        unsigned int tcxofast0_ctrl : 1;
        unsigned int tcxofast1_ctrl : 1;
        unsigned int ctrlen1_apb : 1;
        unsigned int ctrlsel1_apb : 1;
        unsigned int ctrlen0_apb : 1;
        unsigned int ctrlsel0_apb : 1;
        unsigned int timeout_bypass0 : 1;
        unsigned int timeout_bypass1 : 1;
        unsigned int tcxoseq_bypass : 1;
        unsigned int tcxoseq0_time : 5;
        unsigned int tcxoseq1_time : 5;
        unsigned int tcxodown_bypass0 : 1;
        unsigned int tcxodown_bypass1 : 1;
        unsigned int reserved : 6;
    } reg;
} SOC_SCTRL_SCXTALCTRL_UNION;
#endif
#define SOC_SCTRL_SCXTALCTRL_tcxopresel_apb_START (0)
#define SOC_SCTRL_SCXTALCTRL_tcxopresel_apb_END (0)
#define SOC_SCTRL_SCXTALCTRL_tcxohardcon_bypass_START (1)
#define SOC_SCTRL_SCXTALCTRL_tcxohardcon_bypass_END (1)
#define SOC_SCTRL_SCXTALCTRL_tcxosel_apb_START (2)
#define SOC_SCTRL_SCXTALCTRL_tcxosel_apb_END (2)
#define SOC_SCTRL_SCXTALCTRL_tcxosoft_apb_START (3)
#define SOC_SCTRL_SCXTALCTRL_tcxosoft_apb_END (3)
#define SOC_SCTRL_SCXTALCTRL_defau_tcxo_START (4)
#define SOC_SCTRL_SCXTALCTRL_defau_tcxo_END (4)
#define SOC_SCTRL_SCXTALCTRL_tcxofast0_ctrl_START (5)
#define SOC_SCTRL_SCXTALCTRL_tcxofast0_ctrl_END (5)
#define SOC_SCTRL_SCXTALCTRL_tcxofast1_ctrl_START (6)
#define SOC_SCTRL_SCXTALCTRL_tcxofast1_ctrl_END (6)
#define SOC_SCTRL_SCXTALCTRL_ctrlen1_apb_START (7)
#define SOC_SCTRL_SCXTALCTRL_ctrlen1_apb_END (7)
#define SOC_SCTRL_SCXTALCTRL_ctrlsel1_apb_START (8)
#define SOC_SCTRL_SCXTALCTRL_ctrlsel1_apb_END (8)
#define SOC_SCTRL_SCXTALCTRL_ctrlen0_apb_START (9)
#define SOC_SCTRL_SCXTALCTRL_ctrlen0_apb_END (9)
#define SOC_SCTRL_SCXTALCTRL_ctrlsel0_apb_START (10)
#define SOC_SCTRL_SCXTALCTRL_ctrlsel0_apb_END (10)
#define SOC_SCTRL_SCXTALCTRL_timeout_bypass0_START (11)
#define SOC_SCTRL_SCXTALCTRL_timeout_bypass0_END (11)
#define SOC_SCTRL_SCXTALCTRL_timeout_bypass1_START (12)
#define SOC_SCTRL_SCXTALCTRL_timeout_bypass1_END (12)
#define SOC_SCTRL_SCXTALCTRL_tcxoseq_bypass_START (13)
#define SOC_SCTRL_SCXTALCTRL_tcxoseq_bypass_END (13)
#define SOC_SCTRL_SCXTALCTRL_tcxoseq0_time_START (14)
#define SOC_SCTRL_SCXTALCTRL_tcxoseq0_time_END (18)
#define SOC_SCTRL_SCXTALCTRL_tcxoseq1_time_START (19)
#define SOC_SCTRL_SCXTALCTRL_tcxoseq1_time_END (23)
#define SOC_SCTRL_SCXTALCTRL_tcxodown_bypass0_START (24)
#define SOC_SCTRL_SCXTALCTRL_tcxodown_bypass0_END (24)
#define SOC_SCTRL_SCXTALCTRL_tcxodown_bypass1_START (25)
#define SOC_SCTRL_SCXTALCTRL_tcxodown_bypass1_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int timeoutcnt0_apb : 32;
    } reg;
} SOC_SCTRL_SCXTALTIMEOUT0_UNION;
#endif
#define SOC_SCTRL_SCXTALTIMEOUT0_timeoutcnt0_apb_START (0)
#define SOC_SCTRL_SCXTALTIMEOUT0_timeoutcnt0_apb_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int timeoutcnt1_apb : 32;
    } reg;
} SOC_SCTRL_SCXTALTIMEOUT1_UNION;
#endif
#define SOC_SCTRL_SCXTALTIMEOUT1_timeoutcnt1_apb_START (0)
#define SOC_SCTRL_SCXTALTIMEOUT1_timeoutcnt1_apb_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tcxo0_en : 1;
        unsigned int tcxo1_en : 1;
        unsigned int tcxo_timeout1 : 1;
        unsigned int tcxo_timeout0 : 1;
        unsigned int sysclk_en1 : 1;
        unsigned int sysclk_en0 : 1;
        unsigned int sysclk_sel : 1;
        unsigned int clkgt_ctrl0 : 1;
        unsigned int clkgt_ctrl1 : 1;
        unsigned int clkgt_ctrl : 1;
        unsigned int abbbuf_en0 : 1;
        unsigned int abbbuf_en1 : 1;
        unsigned int tcxoseq_finish0 : 1;
        unsigned int tcxoseq_finish1 : 1;
        unsigned int reserved : 18;
    } reg;
} SOC_SCTRL_SCXTALSTAT_UNION;
#endif
#define SOC_SCTRL_SCXTALSTAT_tcxo0_en_START (0)
#define SOC_SCTRL_SCXTALSTAT_tcxo0_en_END (0)
#define SOC_SCTRL_SCXTALSTAT_tcxo1_en_START (1)
#define SOC_SCTRL_SCXTALSTAT_tcxo1_en_END (1)
#define SOC_SCTRL_SCXTALSTAT_tcxo_timeout1_START (2)
#define SOC_SCTRL_SCXTALSTAT_tcxo_timeout1_END (2)
#define SOC_SCTRL_SCXTALSTAT_tcxo_timeout0_START (3)
#define SOC_SCTRL_SCXTALSTAT_tcxo_timeout0_END (3)
#define SOC_SCTRL_SCXTALSTAT_sysclk_en1_START (4)
#define SOC_SCTRL_SCXTALSTAT_sysclk_en1_END (4)
#define SOC_SCTRL_SCXTALSTAT_sysclk_en0_START (5)
#define SOC_SCTRL_SCXTALSTAT_sysclk_en0_END (5)
#define SOC_SCTRL_SCXTALSTAT_sysclk_sel_START (6)
#define SOC_SCTRL_SCXTALSTAT_sysclk_sel_END (6)
#define SOC_SCTRL_SCXTALSTAT_clkgt_ctrl0_START (7)
#define SOC_SCTRL_SCXTALSTAT_clkgt_ctrl0_END (7)
#define SOC_SCTRL_SCXTALSTAT_clkgt_ctrl1_START (8)
#define SOC_SCTRL_SCXTALSTAT_clkgt_ctrl1_END (8)
#define SOC_SCTRL_SCXTALSTAT_clkgt_ctrl_START (9)
#define SOC_SCTRL_SCXTALSTAT_clkgt_ctrl_END (9)
#define SOC_SCTRL_SCXTALSTAT_abbbuf_en0_START (10)
#define SOC_SCTRL_SCXTALSTAT_abbbuf_en0_END (10)
#define SOC_SCTRL_SCXTALSTAT_abbbuf_en1_START (11)
#define SOC_SCTRL_SCXTALSTAT_abbbuf_en1_END (11)
#define SOC_SCTRL_SCXTALSTAT_tcxoseq_finish0_START (12)
#define SOC_SCTRL_SCXTALSTAT_tcxoseq_finish0_END (12)
#define SOC_SCTRL_SCXTALSTAT_tcxoseq_finish1_START (13)
#define SOC_SCTRL_SCXTALSTAT_tcxoseq_finish1_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sclk_sw2fll_bypass : 1;
        unsigned int reserved_0 : 3;
        unsigned int sclk_sw2fll_ctrl : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_SCTRL_SCSW2FLLBYPASS_UNION;
#endif
#define SOC_SCTRL_SCSW2FLLBYPASS_sclk_sw2fll_bypass_START (0)
#define SOC_SCTRL_SCSW2FLLBYPASS_sclk_sw2fll_bypass_END (0)
#define SOC_SCTRL_SCSW2FLLBYPASS_sclk_sw2fll_ctrl_START (4)
#define SOC_SCTRL_SCSW2FLLBYPASS_sclk_sw2fll_ctrl_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fll_lock_neg_filter_num : 10;
        unsigned int fll_lock_pos_filter_num : 10;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCFLLLOCK_FILTER_UNION;
#endif
#define SOC_SCTRL_SCFLLLOCK_FILTER_fll_lock_neg_filter_num_START (0)
#define SOC_SCTRL_SCFLLLOCK_FILTER_fll_lock_neg_filter_num_END (9)
#define SOC_SCTRL_SCFLLLOCK_FILTER_fll_lock_pos_filter_num_START (10)
#define SOC_SCTRL_SCFLLLOCK_FILTER_fll_lock_pos_filter_num_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lp_stat_cfg_s0 : 1;
        unsigned int lp_stat_cfg_s1 : 1;
        unsigned int lp_stat_cfg_s2 : 1;
        unsigned int lp_stat_cfg_s3 : 1;
        unsigned int lp_stat_cfg_s4 : 1;
        unsigned int reserved_0 : 26;
        unsigned int reserved_1 : 1;
    } reg;
} SOC_SCTRL_SCLPSTATCFG_UNION;
#endif
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s0_START (0)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s0_END (0)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s1_START (1)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s1_END (1)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s2_START (2)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s2_END (2)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s3_START (3)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s3_END (3)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s4_START (4)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s4_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_iso_dymanic_ctrl_bypass : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCPERIISOBYPASS_UNION;
#endif
#define SOC_SCTRL_SCPERIISOBYPASS_peri_iso_dymanic_ctrl_bypass_START (0)
#define SOC_SCTRL_SCPERIISOBYPASS_peri_iso_dymanic_ctrl_bypass_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audio_iso_en : 1;
        unsigned int iomcu_iso_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int pcie0phy_iso_en : 1;
        unsigned int hifd_ul_iso_en : 1;
        unsigned int hifd_ul_core_iso_en : 1;
        unsigned int davinci_tiny_iso_en : 1;
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
        unsigned int sc_mdoem4_mem_iso_en : 1;
        unsigned int sc_mdoem5_mem_iso_en : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
    } reg;
} SOC_SCTRL_SCISOEN_UNION;
#endif
#define SOC_SCTRL_SCISOEN_audio_iso_en_START (0)
#define SOC_SCTRL_SCISOEN_audio_iso_en_END (0)
#define SOC_SCTRL_SCISOEN_iomcu_iso_en_START (1)
#define SOC_SCTRL_SCISOEN_iomcu_iso_en_END (1)
#define SOC_SCTRL_SCISOEN_pcie0phy_iso_en_START (5)
#define SOC_SCTRL_SCISOEN_pcie0phy_iso_en_END (5)
#define SOC_SCTRL_SCISOEN_hifd_ul_iso_en_START (6)
#define SOC_SCTRL_SCISOEN_hifd_ul_iso_en_END (6)
#define SOC_SCTRL_SCISOEN_hifd_ul_core_iso_en_START (7)
#define SOC_SCTRL_SCISOEN_hifd_ul_core_iso_en_END (7)
#define SOC_SCTRL_SCISOEN_davinci_tiny_iso_en_START (8)
#define SOC_SCTRL_SCISOEN_davinci_tiny_iso_en_END (8)
#define SOC_SCTRL_SCISOEN_sc_mdoem4_mem_iso_en_START (24)
#define SOC_SCTRL_SCISOEN_sc_mdoem4_mem_iso_en_END (24)
#define SOC_SCTRL_SCISOEN_sc_mdoem5_mem_iso_en_START (25)
#define SOC_SCTRL_SCISOEN_sc_mdoem5_mem_iso_en_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audio_iso_en : 1;
        unsigned int iomcu_iso_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int pcie0phy_iso_en : 1;
        unsigned int hifd_ul_iso_en : 1;
        unsigned int hifd_ul_core_iso_en : 1;
        unsigned int davinci_tiny_iso_en : 1;
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
        unsigned int sc_mdoem4_mem_iso_en : 1;
        unsigned int sc_mdoem5_mem_iso_en : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
    } reg;
} SOC_SCTRL_SCISODIS_UNION;
#endif
#define SOC_SCTRL_SCISODIS_audio_iso_en_START (0)
#define SOC_SCTRL_SCISODIS_audio_iso_en_END (0)
#define SOC_SCTRL_SCISODIS_iomcu_iso_en_START (1)
#define SOC_SCTRL_SCISODIS_iomcu_iso_en_END (1)
#define SOC_SCTRL_SCISODIS_pcie0phy_iso_en_START (5)
#define SOC_SCTRL_SCISODIS_pcie0phy_iso_en_END (5)
#define SOC_SCTRL_SCISODIS_hifd_ul_iso_en_START (6)
#define SOC_SCTRL_SCISODIS_hifd_ul_iso_en_END (6)
#define SOC_SCTRL_SCISODIS_hifd_ul_core_iso_en_START (7)
#define SOC_SCTRL_SCISODIS_hifd_ul_core_iso_en_END (7)
#define SOC_SCTRL_SCISODIS_davinci_tiny_iso_en_START (8)
#define SOC_SCTRL_SCISODIS_davinci_tiny_iso_en_END (8)
#define SOC_SCTRL_SCISODIS_sc_mdoem4_mem_iso_en_START (24)
#define SOC_SCTRL_SCISODIS_sc_mdoem4_mem_iso_en_END (24)
#define SOC_SCTRL_SCISODIS_sc_mdoem5_mem_iso_en_START (25)
#define SOC_SCTRL_SCISODIS_sc_mdoem5_mem_iso_en_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audio_iso_en : 1;
        unsigned int iomcu_iso_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int pcie0phy_iso_en : 1;
        unsigned int hifd_ul_iso_en : 1;
        unsigned int hifd_ul_core_iso_en : 1;
        unsigned int davinci_tiny_iso_en : 1;
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
        unsigned int sc_mdoem4_mem_iso_en : 1;
        unsigned int sc_mdoem5_mem_iso_en : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
    } reg;
} SOC_SCTRL_SCISOSTAT_UNION;
#endif
#define SOC_SCTRL_SCISOSTAT_audio_iso_en_START (0)
#define SOC_SCTRL_SCISOSTAT_audio_iso_en_END (0)
#define SOC_SCTRL_SCISOSTAT_iomcu_iso_en_START (1)
#define SOC_SCTRL_SCISOSTAT_iomcu_iso_en_END (1)
#define SOC_SCTRL_SCISOSTAT_pcie0phy_iso_en_START (5)
#define SOC_SCTRL_SCISOSTAT_pcie0phy_iso_en_END (5)
#define SOC_SCTRL_SCISOSTAT_hifd_ul_iso_en_START (6)
#define SOC_SCTRL_SCISOSTAT_hifd_ul_iso_en_END (6)
#define SOC_SCTRL_SCISOSTAT_hifd_ul_core_iso_en_START (7)
#define SOC_SCTRL_SCISOSTAT_hifd_ul_core_iso_en_END (7)
#define SOC_SCTRL_SCISOSTAT_davinci_tiny_iso_en_START (8)
#define SOC_SCTRL_SCISOSTAT_davinci_tiny_iso_en_END (8)
#define SOC_SCTRL_SCISOSTAT_sc_mdoem4_mem_iso_en_START (24)
#define SOC_SCTRL_SCISOSTAT_sc_mdoem4_mem_iso_en_END (24)
#define SOC_SCTRL_SCISOSTAT_sc_mdoem5_mem_iso_en_START (25)
#define SOC_SCTRL_SCISOSTAT_sc_mdoem5_mem_iso_en_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audiopwren : 1;
        unsigned int iomcupwren : 1;
        unsigned int hifd_ul_pwren : 1;
        unsigned int hifd_ul_core_pwren : 1;
        unsigned int davinci_tiny_pwren : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 23;
    } reg;
} SOC_SCTRL_SCPWREN_UNION;
#endif
#define SOC_SCTRL_SCPWREN_audiopwren_START (0)
#define SOC_SCTRL_SCPWREN_audiopwren_END (0)
#define SOC_SCTRL_SCPWREN_iomcupwren_START (1)
#define SOC_SCTRL_SCPWREN_iomcupwren_END (1)
#define SOC_SCTRL_SCPWREN_hifd_ul_pwren_START (2)
#define SOC_SCTRL_SCPWREN_hifd_ul_pwren_END (2)
#define SOC_SCTRL_SCPWREN_hifd_ul_core_pwren_START (3)
#define SOC_SCTRL_SCPWREN_hifd_ul_core_pwren_END (3)
#define SOC_SCTRL_SCPWREN_davinci_tiny_pwren_START (4)
#define SOC_SCTRL_SCPWREN_davinci_tiny_pwren_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audiopwren : 1;
        unsigned int iomcupwren : 1;
        unsigned int hifd_ul_pwren : 1;
        unsigned int hifd_ul_core_pwren : 1;
        unsigned int davinci_tiny_pwren : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 23;
    } reg;
} SOC_SCTRL_SCPWRDIS_UNION;
#endif
#define SOC_SCTRL_SCPWRDIS_audiopwren_START (0)
#define SOC_SCTRL_SCPWRDIS_audiopwren_END (0)
#define SOC_SCTRL_SCPWRDIS_iomcupwren_START (1)
#define SOC_SCTRL_SCPWRDIS_iomcupwren_END (1)
#define SOC_SCTRL_SCPWRDIS_hifd_ul_pwren_START (2)
#define SOC_SCTRL_SCPWRDIS_hifd_ul_pwren_END (2)
#define SOC_SCTRL_SCPWRDIS_hifd_ul_core_pwren_START (3)
#define SOC_SCTRL_SCPWRDIS_hifd_ul_core_pwren_END (3)
#define SOC_SCTRL_SCPWRDIS_davinci_tiny_pwren_START (4)
#define SOC_SCTRL_SCPWRDIS_davinci_tiny_pwren_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audiopwrstat : 1;
        unsigned int iomcupwrstat : 1;
        unsigned int hifdulpwstat : 1;
        unsigned int hifdulcorepwstat : 1;
        unsigned int davinci_tiny_pwrstat : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 23;
    } reg;
} SOC_SCTRL_SCPWRSTAT_UNION;
#endif
#define SOC_SCTRL_SCPWRSTAT_audiopwrstat_START (0)
#define SOC_SCTRL_SCPWRSTAT_audiopwrstat_END (0)
#define SOC_SCTRL_SCPWRSTAT_iomcupwrstat_START (1)
#define SOC_SCTRL_SCPWRSTAT_iomcupwrstat_END (1)
#define SOC_SCTRL_SCPWRSTAT_hifdulpwstat_START (2)
#define SOC_SCTRL_SCPWRSTAT_hifdulpwstat_END (2)
#define SOC_SCTRL_SCPWRSTAT_hifdulcorepwstat_START (3)
#define SOC_SCTRL_SCPWRSTAT_hifdulcorepwstat_END (3)
#define SOC_SCTRL_SCPWRSTAT_davinci_tiny_pwrstat_START (4)
#define SOC_SCTRL_SCPWRSTAT_davinci_tiny_pwrstat_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audio_mtcmos_ack : 1;
        unsigned int iomcu_mtcmos_ack : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 27;
    } reg;
} SOC_SCTRL_SCPWRACK_UNION;
#endif
#define SOC_SCTRL_SCPWRACK_audio_mtcmos_ack_START (0)
#define SOC_SCTRL_SCPWRACK_audio_mtcmos_ack_END (0)
#define SOC_SCTRL_SCPWRACK_iomcu_mtcmos_ack_START (1)
#define SOC_SCTRL_SCPWRACK_iomcu_mtcmos_ack_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_pwr_down_time : 25;
        unsigned int timeout_op_rston : 1;
        unsigned int timeout_op_clkoff : 1;
        unsigned int timeout_op_eniso : 1;
        unsigned int timeout_op_downperi : 1;
        unsigned int timeout_op_rston2 : 1;
        unsigned int timeout_op_disiso : 1;
        unsigned int reserved : 1;
    } reg;
} SOC_SCTRL_SCPERPWRDOWNTIME_UNION;
#endif
#define SOC_SCTRL_SCPERPWRDOWNTIME_peri_pwr_down_time_START (0)
#define SOC_SCTRL_SCPERPWRDOWNTIME_peri_pwr_down_time_END (24)
#define SOC_SCTRL_SCPERPWRDOWNTIME_timeout_op_rston_START (25)
#define SOC_SCTRL_SCPERPWRDOWNTIME_timeout_op_rston_END (25)
#define SOC_SCTRL_SCPERPWRDOWNTIME_timeout_op_clkoff_START (26)
#define SOC_SCTRL_SCPERPWRDOWNTIME_timeout_op_clkoff_END (26)
#define SOC_SCTRL_SCPERPWRDOWNTIME_timeout_op_eniso_START (27)
#define SOC_SCTRL_SCPERPWRDOWNTIME_timeout_op_eniso_END (27)
#define SOC_SCTRL_SCPERPWRDOWNTIME_timeout_op_downperi_START (28)
#define SOC_SCTRL_SCPERPWRDOWNTIME_timeout_op_downperi_END (28)
#define SOC_SCTRL_SCPERPWRDOWNTIME_timeout_op_rston2_START (29)
#define SOC_SCTRL_SCPERPWRDOWNTIME_timeout_op_rston2_END (29)
#define SOC_SCTRL_SCPERPWRDOWNTIME_timeout_op_disiso_START (30)
#define SOC_SCTRL_SCPERPWRDOWNTIME_timeout_op_disiso_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_pwr_up_time : 25;
        unsigned int reserved : 7;
    } reg;
} SOC_SCTRL_SCPERPWRUPTIME_UNION;
#endif
#define SOC_SCTRL_SCPERPWRUPTIME_peri_pwr_up_time_START (0)
#define SOC_SCTRL_SCPERPWRUPTIME_peri_pwr_up_time_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_asp_powerdn_msk : 1;
        unsigned int intr_asp_powerup_msk : 1;
        unsigned int vote_asp_powerupdn_bypass : 1;
        unsigned int reserved_0 : 1;
        unsigned int asp_power_req_lpmcu : 2;
        unsigned int reserved_1 : 2;
        unsigned int asp_power_req_sft0 : 1;
        unsigned int asp_power_req_sft1 : 1;
        unsigned int asp_power_req_sft2 : 1;
        unsigned int asp_power_req_sft3 : 1;
        unsigned int reserved_2 : 4;
        unsigned int sc_asp_pwrcfg0_msk : 16;
    } reg;
} SOC_SCTRL_SC_ASP_PWRCFG0_UNION;
#endif
#define SOC_SCTRL_SC_ASP_PWRCFG0_intr_asp_powerdn_msk_START (0)
#define SOC_SCTRL_SC_ASP_PWRCFG0_intr_asp_powerdn_msk_END (0)
#define SOC_SCTRL_SC_ASP_PWRCFG0_intr_asp_powerup_msk_START (1)
#define SOC_SCTRL_SC_ASP_PWRCFG0_intr_asp_powerup_msk_END (1)
#define SOC_SCTRL_SC_ASP_PWRCFG0_vote_asp_powerupdn_bypass_START (2)
#define SOC_SCTRL_SC_ASP_PWRCFG0_vote_asp_powerupdn_bypass_END (2)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_lpmcu_START (4)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_lpmcu_END (5)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_sft0_START (8)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_sft0_END (8)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_sft1_START (9)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_sft1_END (9)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_sft2_START (10)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_sft2_END (10)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_sft3_START (11)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_sft3_END (11)
#define SOC_SCTRL_SC_ASP_PWRCFG0_sc_asp_pwrcfg0_msk_START (16)
#define SOC_SCTRL_SC_ASP_PWRCFG0_sc_asp_pwrcfg0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_asp_powerdn_clr : 1;
        unsigned int intr_asp_powerup_clr : 1;
        unsigned int reserved_0 : 2;
        unsigned int intr_asp_powerdn_raw : 1;
        unsigned int intr_asp_powerup_raw : 1;
        unsigned int reserved_1 : 2;
        unsigned int intr_asp_powerdn : 1;
        unsigned int intr_asp_powerup : 1;
        unsigned int reserved_2 : 2;
        unsigned int asp_power_req_hifi_sync : 2;
        unsigned int reserved_3 : 2;
        unsigned int asp_power_req_hifi_stat : 2;
        unsigned int reserved_4 : 2;
        unsigned int asp_power_req_cpu_sync : 2;
        unsigned int reserved_5 : 2;
        unsigned int asp_power_req : 1;
        unsigned int reserved_6 : 3;
        unsigned int curr_asp_power_stat : 2;
        unsigned int reserved_7 : 2;
    } reg;
} SOC_SCTRL_SC_ASP_PWRCFG1_UNION;
#endif
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerdn_clr_START (0)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerdn_clr_END (0)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerup_clr_START (1)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerup_clr_END (1)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerdn_raw_START (4)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerdn_raw_END (4)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerup_raw_START (5)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerup_raw_END (5)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerdn_START (8)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerdn_END (8)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerup_START (9)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerup_END (9)
#define SOC_SCTRL_SC_ASP_PWRCFG1_asp_power_req_hifi_sync_START (12)
#define SOC_SCTRL_SC_ASP_PWRCFG1_asp_power_req_hifi_sync_END (13)
#define SOC_SCTRL_SC_ASP_PWRCFG1_asp_power_req_hifi_stat_START (16)
#define SOC_SCTRL_SC_ASP_PWRCFG1_asp_power_req_hifi_stat_END (17)
#define SOC_SCTRL_SC_ASP_PWRCFG1_asp_power_req_cpu_sync_START (20)
#define SOC_SCTRL_SC_ASP_PWRCFG1_asp_power_req_cpu_sync_END (21)
#define SOC_SCTRL_SC_ASP_PWRCFG1_asp_power_req_START (24)
#define SOC_SCTRL_SC_ASP_PWRCFG1_asp_power_req_END (24)
#define SOC_SCTRL_SC_ASP_PWRCFG1_curr_asp_power_stat_START (28)
#define SOC_SCTRL_SC_ASP_PWRCFG1_curr_asp_power_stat_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_wakeup_sys : 1;
        unsigned int drx0_int : 1;
        unsigned int drx1_int : 1;
        unsigned int notdrx_int : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_SCTRL_SCINT_GATHER_STAT_UNION;
#endif
#define SOC_SCTRL_SCINT_GATHER_STAT_int_wakeup_sys_START (0)
#define SOC_SCTRL_SCINT_GATHER_STAT_int_wakeup_sys_END (0)
#define SOC_SCTRL_SCINT_GATHER_STAT_drx0_int_START (1)
#define SOC_SCTRL_SCINT_GATHER_STAT_drx0_int_END (1)
#define SOC_SCTRL_SCINT_GATHER_STAT_drx1_int_START (2)
#define SOC_SCTRL_SCINT_GATHER_STAT_drx1_int_END (2)
#define SOC_SCTRL_SCINT_GATHER_STAT_notdrx_int_START (3)
#define SOC_SCTRL_SCINT_GATHER_STAT_notdrx_int_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpio_22_int : 1;
        unsigned int gpio_23_int : 1;
        unsigned int gpio_24_int : 1;
        unsigned int gpio_25_int : 1;
        unsigned int gpio_26_int : 1;
        unsigned int gpio_27_int : 1;
        unsigned int rtc0_int : 1;
        unsigned int rtc1_int : 1;
        unsigned int timer00_int : 1;
        unsigned int timer01_int : 1;
        unsigned int timer10_int : 1;
        unsigned int timer11_int : 1;
        unsigned int timer20_int : 1;
        unsigned int timer21_int : 1;
        unsigned int timer30_int : 1;
        unsigned int timer31_int : 1;
        unsigned int timer40_int : 1;
        unsigned int timer41_int : 1;
        unsigned int timer50_int : 1;
        unsigned int timer51_int : 1;
        unsigned int timer60_int : 1;
        unsigned int timer61_int : 1;
        unsigned int timer70_int : 1;
        unsigned int timer71_int : 1;
        unsigned int timer80_int : 1;
        unsigned int timer81_int : 1;
        unsigned int intr_asp_ipc_arm : 1;
        unsigned int intr_asp_watchdog : 1;
        unsigned int intr_iomcu_wdog : 1;
        unsigned int intr_iomcu_gpio_comb : 1;
        unsigned int intr_iomcu_gpio_comb_sc : 1;
        unsigned int intr_wakeup_iomcu : 1;
    } reg;
} SOC_SCTRL_SCINT_MASK_UNION;
#endif
#define SOC_SCTRL_SCINT_MASK_gpio_22_int_START (0)
#define SOC_SCTRL_SCINT_MASK_gpio_22_int_END (0)
#define SOC_SCTRL_SCINT_MASK_gpio_23_int_START (1)
#define SOC_SCTRL_SCINT_MASK_gpio_23_int_END (1)
#define SOC_SCTRL_SCINT_MASK_gpio_24_int_START (2)
#define SOC_SCTRL_SCINT_MASK_gpio_24_int_END (2)
#define SOC_SCTRL_SCINT_MASK_gpio_25_int_START (3)
#define SOC_SCTRL_SCINT_MASK_gpio_25_int_END (3)
#define SOC_SCTRL_SCINT_MASK_gpio_26_int_START (4)
#define SOC_SCTRL_SCINT_MASK_gpio_26_int_END (4)
#define SOC_SCTRL_SCINT_MASK_gpio_27_int_START (5)
#define SOC_SCTRL_SCINT_MASK_gpio_27_int_END (5)
#define SOC_SCTRL_SCINT_MASK_rtc0_int_START (6)
#define SOC_SCTRL_SCINT_MASK_rtc0_int_END (6)
#define SOC_SCTRL_SCINT_MASK_rtc1_int_START (7)
#define SOC_SCTRL_SCINT_MASK_rtc1_int_END (7)
#define SOC_SCTRL_SCINT_MASK_timer00_int_START (8)
#define SOC_SCTRL_SCINT_MASK_timer00_int_END (8)
#define SOC_SCTRL_SCINT_MASK_timer01_int_START (9)
#define SOC_SCTRL_SCINT_MASK_timer01_int_END (9)
#define SOC_SCTRL_SCINT_MASK_timer10_int_START (10)
#define SOC_SCTRL_SCINT_MASK_timer10_int_END (10)
#define SOC_SCTRL_SCINT_MASK_timer11_int_START (11)
#define SOC_SCTRL_SCINT_MASK_timer11_int_END (11)
#define SOC_SCTRL_SCINT_MASK_timer20_int_START (12)
#define SOC_SCTRL_SCINT_MASK_timer20_int_END (12)
#define SOC_SCTRL_SCINT_MASK_timer21_int_START (13)
#define SOC_SCTRL_SCINT_MASK_timer21_int_END (13)
#define SOC_SCTRL_SCINT_MASK_timer30_int_START (14)
#define SOC_SCTRL_SCINT_MASK_timer30_int_END (14)
#define SOC_SCTRL_SCINT_MASK_timer31_int_START (15)
#define SOC_SCTRL_SCINT_MASK_timer31_int_END (15)
#define SOC_SCTRL_SCINT_MASK_timer40_int_START (16)
#define SOC_SCTRL_SCINT_MASK_timer40_int_END (16)
#define SOC_SCTRL_SCINT_MASK_timer41_int_START (17)
#define SOC_SCTRL_SCINT_MASK_timer41_int_END (17)
#define SOC_SCTRL_SCINT_MASK_timer50_int_START (18)
#define SOC_SCTRL_SCINT_MASK_timer50_int_END (18)
#define SOC_SCTRL_SCINT_MASK_timer51_int_START (19)
#define SOC_SCTRL_SCINT_MASK_timer51_int_END (19)
#define SOC_SCTRL_SCINT_MASK_timer60_int_START (20)
#define SOC_SCTRL_SCINT_MASK_timer60_int_END (20)
#define SOC_SCTRL_SCINT_MASK_timer61_int_START (21)
#define SOC_SCTRL_SCINT_MASK_timer61_int_END (21)
#define SOC_SCTRL_SCINT_MASK_timer70_int_START (22)
#define SOC_SCTRL_SCINT_MASK_timer70_int_END (22)
#define SOC_SCTRL_SCINT_MASK_timer71_int_START (23)
#define SOC_SCTRL_SCINT_MASK_timer71_int_END (23)
#define SOC_SCTRL_SCINT_MASK_timer80_int_START (24)
#define SOC_SCTRL_SCINT_MASK_timer80_int_END (24)
#define SOC_SCTRL_SCINT_MASK_timer81_int_START (25)
#define SOC_SCTRL_SCINT_MASK_timer81_int_END (25)
#define SOC_SCTRL_SCINT_MASK_intr_asp_ipc_arm_START (26)
#define SOC_SCTRL_SCINT_MASK_intr_asp_ipc_arm_END (26)
#define SOC_SCTRL_SCINT_MASK_intr_asp_watchdog_START (27)
#define SOC_SCTRL_SCINT_MASK_intr_asp_watchdog_END (27)
#define SOC_SCTRL_SCINT_MASK_intr_iomcu_wdog_START (28)
#define SOC_SCTRL_SCINT_MASK_intr_iomcu_wdog_END (28)
#define SOC_SCTRL_SCINT_MASK_intr_iomcu_gpio_comb_START (29)
#define SOC_SCTRL_SCINT_MASK_intr_iomcu_gpio_comb_END (29)
#define SOC_SCTRL_SCINT_MASK_intr_iomcu_gpio_comb_sc_START (30)
#define SOC_SCTRL_SCINT_MASK_intr_iomcu_gpio_comb_sc_END (30)
#define SOC_SCTRL_SCINT_MASK_intr_wakeup_iomcu_START (31)
#define SOC_SCTRL_SCINT_MASK_intr_wakeup_iomcu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpio_22_int : 1;
        unsigned int gpio_23_int : 1;
        unsigned int gpio_24_int : 1;
        unsigned int gpio_25_int : 1;
        unsigned int gpio_26_int : 1;
        unsigned int gpio_27_int : 1;
        unsigned int rtc0_int : 1;
        unsigned int rtc1_int : 1;
        unsigned int timer00_int : 1;
        unsigned int timer01_int : 1;
        unsigned int timer10_int : 1;
        unsigned int timer11_int : 1;
        unsigned int timer20_int : 1;
        unsigned int timer21_int : 1;
        unsigned int timer30_int : 1;
        unsigned int timer31_int : 1;
        unsigned int timer40_int : 1;
        unsigned int timer41_int : 1;
        unsigned int timer50_int : 1;
        unsigned int timer51_int : 1;
        unsigned int timer60_int : 1;
        unsigned int timer61_int : 1;
        unsigned int timer70_int : 1;
        unsigned int timer71_int : 1;
        unsigned int timer80_int : 1;
        unsigned int timer81_int : 1;
        unsigned int intr_asp_ipc_arm : 1;
        unsigned int intr_asp_watchdog : 1;
        unsigned int intr_iomcu_wdog : 1;
        unsigned int intr_iomcu_gpio_comb : 1;
        unsigned int intr_wakeup_iomcu : 1;
        unsigned int reserved : 1;
    } reg;
} SOC_SCTRL_SCINT_STAT_UNION;
#endif
#define SOC_SCTRL_SCINT_STAT_gpio_22_int_START (0)
#define SOC_SCTRL_SCINT_STAT_gpio_22_int_END (0)
#define SOC_SCTRL_SCINT_STAT_gpio_23_int_START (1)
#define SOC_SCTRL_SCINT_STAT_gpio_23_int_END (1)
#define SOC_SCTRL_SCINT_STAT_gpio_24_int_START (2)
#define SOC_SCTRL_SCINT_STAT_gpio_24_int_END (2)
#define SOC_SCTRL_SCINT_STAT_gpio_25_int_START (3)
#define SOC_SCTRL_SCINT_STAT_gpio_25_int_END (3)
#define SOC_SCTRL_SCINT_STAT_gpio_26_int_START (4)
#define SOC_SCTRL_SCINT_STAT_gpio_26_int_END (4)
#define SOC_SCTRL_SCINT_STAT_gpio_27_int_START (5)
#define SOC_SCTRL_SCINT_STAT_gpio_27_int_END (5)
#define SOC_SCTRL_SCINT_STAT_rtc0_int_START (6)
#define SOC_SCTRL_SCINT_STAT_rtc0_int_END (6)
#define SOC_SCTRL_SCINT_STAT_rtc1_int_START (7)
#define SOC_SCTRL_SCINT_STAT_rtc1_int_END (7)
#define SOC_SCTRL_SCINT_STAT_timer00_int_START (8)
#define SOC_SCTRL_SCINT_STAT_timer00_int_END (8)
#define SOC_SCTRL_SCINT_STAT_timer01_int_START (9)
#define SOC_SCTRL_SCINT_STAT_timer01_int_END (9)
#define SOC_SCTRL_SCINT_STAT_timer10_int_START (10)
#define SOC_SCTRL_SCINT_STAT_timer10_int_END (10)
#define SOC_SCTRL_SCINT_STAT_timer11_int_START (11)
#define SOC_SCTRL_SCINT_STAT_timer11_int_END (11)
#define SOC_SCTRL_SCINT_STAT_timer20_int_START (12)
#define SOC_SCTRL_SCINT_STAT_timer20_int_END (12)
#define SOC_SCTRL_SCINT_STAT_timer21_int_START (13)
#define SOC_SCTRL_SCINT_STAT_timer21_int_END (13)
#define SOC_SCTRL_SCINT_STAT_timer30_int_START (14)
#define SOC_SCTRL_SCINT_STAT_timer30_int_END (14)
#define SOC_SCTRL_SCINT_STAT_timer31_int_START (15)
#define SOC_SCTRL_SCINT_STAT_timer31_int_END (15)
#define SOC_SCTRL_SCINT_STAT_timer40_int_START (16)
#define SOC_SCTRL_SCINT_STAT_timer40_int_END (16)
#define SOC_SCTRL_SCINT_STAT_timer41_int_START (17)
#define SOC_SCTRL_SCINT_STAT_timer41_int_END (17)
#define SOC_SCTRL_SCINT_STAT_timer50_int_START (18)
#define SOC_SCTRL_SCINT_STAT_timer50_int_END (18)
#define SOC_SCTRL_SCINT_STAT_timer51_int_START (19)
#define SOC_SCTRL_SCINT_STAT_timer51_int_END (19)
#define SOC_SCTRL_SCINT_STAT_timer60_int_START (20)
#define SOC_SCTRL_SCINT_STAT_timer60_int_END (20)
#define SOC_SCTRL_SCINT_STAT_timer61_int_START (21)
#define SOC_SCTRL_SCINT_STAT_timer61_int_END (21)
#define SOC_SCTRL_SCINT_STAT_timer70_int_START (22)
#define SOC_SCTRL_SCINT_STAT_timer70_int_END (22)
#define SOC_SCTRL_SCINT_STAT_timer71_int_START (23)
#define SOC_SCTRL_SCINT_STAT_timer71_int_END (23)
#define SOC_SCTRL_SCINT_STAT_timer80_int_START (24)
#define SOC_SCTRL_SCINT_STAT_timer80_int_END (24)
#define SOC_SCTRL_SCINT_STAT_timer81_int_START (25)
#define SOC_SCTRL_SCINT_STAT_timer81_int_END (25)
#define SOC_SCTRL_SCINT_STAT_intr_asp_ipc_arm_START (26)
#define SOC_SCTRL_SCINT_STAT_intr_asp_ipc_arm_END (26)
#define SOC_SCTRL_SCINT_STAT_intr_asp_watchdog_START (27)
#define SOC_SCTRL_SCINT_STAT_intr_asp_watchdog_END (27)
#define SOC_SCTRL_SCINT_STAT_intr_iomcu_wdog_START (28)
#define SOC_SCTRL_SCINT_STAT_intr_iomcu_wdog_END (28)
#define SOC_SCTRL_SCINT_STAT_intr_iomcu_gpio_comb_START (29)
#define SOC_SCTRL_SCINT_STAT_intr_iomcu_gpio_comb_END (29)
#define SOC_SCTRL_SCINT_STAT_intr_wakeup_iomcu_START (30)
#define SOC_SCTRL_SCINT_STAT_intr_wakeup_iomcu_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lte_drx_arm_wakeup_int : 1;
        unsigned int tds_drx_arm_wakeup_int : 1;
        unsigned int g1_int_gbbp_to_cpu_on : 1;
        unsigned int g2_int_gbbp_to_cpu_on : 1;
        unsigned int w_arm_int02 : 1;
        unsigned int cbbp_int01_2 : 1;
        unsigned int g3_int_gbbp_to_cpu_on : 1;
        unsigned int w_arm_int02_2 : 1;
        unsigned int lte2_drx_arm_wakeup_int : 1;
        unsigned int ltev_drx_arm_wakeup_int : 1;
        unsigned int nr_drx_arm_wakeup_int : 1;
        unsigned int reserved : 20;
        unsigned int intr_iomcu_gpio_comb : 1;
    } reg;
} SOC_SCTRL_SCDRX_INT_CFG_UNION;
#endif
#define SOC_SCTRL_SCDRX_INT_CFG_lte_drx_arm_wakeup_int_START (0)
#define SOC_SCTRL_SCDRX_INT_CFG_lte_drx_arm_wakeup_int_END (0)
#define SOC_SCTRL_SCDRX_INT_CFG_tds_drx_arm_wakeup_int_START (1)
#define SOC_SCTRL_SCDRX_INT_CFG_tds_drx_arm_wakeup_int_END (1)
#define SOC_SCTRL_SCDRX_INT_CFG_g1_int_gbbp_to_cpu_on_START (2)
#define SOC_SCTRL_SCDRX_INT_CFG_g1_int_gbbp_to_cpu_on_END (2)
#define SOC_SCTRL_SCDRX_INT_CFG_g2_int_gbbp_to_cpu_on_START (3)
#define SOC_SCTRL_SCDRX_INT_CFG_g2_int_gbbp_to_cpu_on_END (3)
#define SOC_SCTRL_SCDRX_INT_CFG_w_arm_int02_START (4)
#define SOC_SCTRL_SCDRX_INT_CFG_w_arm_int02_END (4)
#define SOC_SCTRL_SCDRX_INT_CFG_cbbp_int01_2_START (5)
#define SOC_SCTRL_SCDRX_INT_CFG_cbbp_int01_2_END (5)
#define SOC_SCTRL_SCDRX_INT_CFG_g3_int_gbbp_to_cpu_on_START (6)
#define SOC_SCTRL_SCDRX_INT_CFG_g3_int_gbbp_to_cpu_on_END (6)
#define SOC_SCTRL_SCDRX_INT_CFG_w_arm_int02_2_START (7)
#define SOC_SCTRL_SCDRX_INT_CFG_w_arm_int02_2_END (7)
#define SOC_SCTRL_SCDRX_INT_CFG_lte2_drx_arm_wakeup_int_START (8)
#define SOC_SCTRL_SCDRX_INT_CFG_lte2_drx_arm_wakeup_int_END (8)
#define SOC_SCTRL_SCDRX_INT_CFG_ltev_drx_arm_wakeup_int_START (9)
#define SOC_SCTRL_SCDRX_INT_CFG_ltev_drx_arm_wakeup_int_END (9)
#define SOC_SCTRL_SCDRX_INT_CFG_nr_drx_arm_wakeup_int_START (10)
#define SOC_SCTRL_SCDRX_INT_CFG_nr_drx_arm_wakeup_int_END (10)
#define SOC_SCTRL_SCDRX_INT_CFG_intr_iomcu_gpio_comb_START (31)
#define SOC_SCTRL_SCDRX_INT_CFG_intr_iomcu_gpio_comb_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lpmcu_wfi_int : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCLPMCUWFI_INT_UNION;
#endif
#define SOC_SCTRL_SCLPMCUWFI_INT_lpmcu_wfi_int_START (0)
#define SOC_SCTRL_SCLPMCUWFI_INT_lpmcu_wfi_int_END (0)
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
        unsigned int noc_ufs_cfg_t_maintimeout_msksta : 1;
        unsigned int noc_mad_cfg_maintimeout_msk : 1;
        unsigned int noc_asp_cfg_t_maintimeout_msk : 1;
        unsigned int iomcu_ahb_slv_maintimeout_msk : 1;
        unsigned int iomcu_apb_slv_maintimeout_msk : 1;
        unsigned int noc_fd_cfg_t_maintimeout_msk : 1;
        unsigned int sc_noc_aon_apb_slv_t_maintimeout_msk : 1;
        unsigned int noc_ao_tcp_cfg_t_maintimeout_msk : 1;
        unsigned int ao_ipc_int13 : 1;
        unsigned int ao_ipc_int1 : 1;
        unsigned int ao_ipc_int5 : 1;
        unsigned int ao_ipc_int8 : 1;
        unsigned int ao_ipc_int7 : 1;
        unsigned int int_gpio31 : 1;
        unsigned int ao_ipc_int0 : 1;
        unsigned int int_gpio32 : 1;
        unsigned int int_gpio33 : 1;
        unsigned int int_gpio29 : 1;
        unsigned int int_gpio30 : 1;
        unsigned int gpio_20_int : 1;
        unsigned int gpio_21_int : 1;
        unsigned int gpio_28_int : 1;
        unsigned int se_gpio1 : 1;
        unsigned int mad_int : 1;
        unsigned int intr_ao_wd : 1;
    } reg;
} SOC_SCTRL_SCINT_MASK1_UNION;
#endif
#define SOC_SCTRL_SCINT_MASK1_noc_ufs_cfg_t_maintimeout_msksta_START (7)
#define SOC_SCTRL_SCINT_MASK1_noc_ufs_cfg_t_maintimeout_msksta_END (7)
#define SOC_SCTRL_SCINT_MASK1_noc_mad_cfg_maintimeout_msk_START (8)
#define SOC_SCTRL_SCINT_MASK1_noc_mad_cfg_maintimeout_msk_END (8)
#define SOC_SCTRL_SCINT_MASK1_noc_asp_cfg_t_maintimeout_msk_START (9)
#define SOC_SCTRL_SCINT_MASK1_noc_asp_cfg_t_maintimeout_msk_END (9)
#define SOC_SCTRL_SCINT_MASK1_iomcu_ahb_slv_maintimeout_msk_START (10)
#define SOC_SCTRL_SCINT_MASK1_iomcu_ahb_slv_maintimeout_msk_END (10)
#define SOC_SCTRL_SCINT_MASK1_iomcu_apb_slv_maintimeout_msk_START (11)
#define SOC_SCTRL_SCINT_MASK1_iomcu_apb_slv_maintimeout_msk_END (11)
#define SOC_SCTRL_SCINT_MASK1_noc_fd_cfg_t_maintimeout_msk_START (12)
#define SOC_SCTRL_SCINT_MASK1_noc_fd_cfg_t_maintimeout_msk_END (12)
#define SOC_SCTRL_SCINT_MASK1_sc_noc_aon_apb_slv_t_maintimeout_msk_START (13)
#define SOC_SCTRL_SCINT_MASK1_sc_noc_aon_apb_slv_t_maintimeout_msk_END (13)
#define SOC_SCTRL_SCINT_MASK1_noc_ao_tcp_cfg_t_maintimeout_msk_START (14)
#define SOC_SCTRL_SCINT_MASK1_noc_ao_tcp_cfg_t_maintimeout_msk_END (14)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int13_START (15)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int13_END (15)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int1_START (16)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int1_END (16)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int5_START (17)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int5_END (17)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int8_START (18)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int8_END (18)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int7_START (19)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int7_END (19)
#define SOC_SCTRL_SCINT_MASK1_int_gpio31_START (20)
#define SOC_SCTRL_SCINT_MASK1_int_gpio31_END (20)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int0_START (21)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int0_END (21)
#define SOC_SCTRL_SCINT_MASK1_int_gpio32_START (22)
#define SOC_SCTRL_SCINT_MASK1_int_gpio32_END (22)
#define SOC_SCTRL_SCINT_MASK1_int_gpio33_START (23)
#define SOC_SCTRL_SCINT_MASK1_int_gpio33_END (23)
#define SOC_SCTRL_SCINT_MASK1_int_gpio29_START (24)
#define SOC_SCTRL_SCINT_MASK1_int_gpio29_END (24)
#define SOC_SCTRL_SCINT_MASK1_int_gpio30_START (25)
#define SOC_SCTRL_SCINT_MASK1_int_gpio30_END (25)
#define SOC_SCTRL_SCINT_MASK1_gpio_20_int_START (26)
#define SOC_SCTRL_SCINT_MASK1_gpio_20_int_END (26)
#define SOC_SCTRL_SCINT_MASK1_gpio_21_int_START (27)
#define SOC_SCTRL_SCINT_MASK1_gpio_21_int_END (27)
#define SOC_SCTRL_SCINT_MASK1_gpio_28_int_START (28)
#define SOC_SCTRL_SCINT_MASK1_gpio_28_int_END (28)
#define SOC_SCTRL_SCINT_MASK1_se_gpio1_START (29)
#define SOC_SCTRL_SCINT_MASK1_se_gpio1_END (29)
#define SOC_SCTRL_SCINT_MASK1_mad_int_START (30)
#define SOC_SCTRL_SCINT_MASK1_mad_int_END (30)
#define SOC_SCTRL_SCINT_MASK1_intr_ao_wd_START (31)
#define SOC_SCTRL_SCINT_MASK1_intr_ao_wd_END (31)
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
        unsigned int noc_ufs_cfg_t_maintimeout_msksta : 1;
        unsigned int noc_mad_cfg_maintimeout_msksta : 1;
        unsigned int noc_asp_cfg_t_maintimeout_msksta : 1;
        unsigned int iomcu_ahb_slv_maintimeout_msksta : 1;
        unsigned int iomcu_apb_slv_maintimeout_msksta : 1;
        unsigned int noc_fd_cfg_t_maintimeout_msksta : 1;
        unsigned int sc_noc_aon_apb_slv_t_maintimeout_msksta : 1;
        unsigned int noc_ao_tcp_cfg_t_maintimeout_mskstat : 1;
        unsigned int ao_ipc_int13 : 1;
        unsigned int ao_ipc_int1 : 1;
        unsigned int ao_ipc_int5 : 1;
        unsigned int ao_ipc_int8 : 1;
        unsigned int ao_ipc_int7 : 1;
        unsigned int int_gpio31 : 1;
        unsigned int ao_ipc_int0 : 1;
        unsigned int int_gpio32 : 1;
        unsigned int int_gpio33 : 1;
        unsigned int int_gpio29 : 1;
        unsigned int int_gpio30 : 1;
        unsigned int gpio_20_int : 1;
        unsigned int gpio_21_int : 1;
        unsigned int gpio_28_int : 1;
        unsigned int se_gpio1 : 1;
        unsigned int mad_int : 1;
        unsigned int intr_ao_wd : 1;
    } reg;
} SOC_SCTRL_SCINT_STAT1_UNION;
#endif
#define SOC_SCTRL_SCINT_STAT1_noc_ufs_cfg_t_maintimeout_msksta_START (7)
#define SOC_SCTRL_SCINT_STAT1_noc_ufs_cfg_t_maintimeout_msksta_END (7)
#define SOC_SCTRL_SCINT_STAT1_noc_mad_cfg_maintimeout_msksta_START (8)
#define SOC_SCTRL_SCINT_STAT1_noc_mad_cfg_maintimeout_msksta_END (8)
#define SOC_SCTRL_SCINT_STAT1_noc_asp_cfg_t_maintimeout_msksta_START (9)
#define SOC_SCTRL_SCINT_STAT1_noc_asp_cfg_t_maintimeout_msksta_END (9)
#define SOC_SCTRL_SCINT_STAT1_iomcu_ahb_slv_maintimeout_msksta_START (10)
#define SOC_SCTRL_SCINT_STAT1_iomcu_ahb_slv_maintimeout_msksta_END (10)
#define SOC_SCTRL_SCINT_STAT1_iomcu_apb_slv_maintimeout_msksta_START (11)
#define SOC_SCTRL_SCINT_STAT1_iomcu_apb_slv_maintimeout_msksta_END (11)
#define SOC_SCTRL_SCINT_STAT1_noc_fd_cfg_t_maintimeout_msksta_START (12)
#define SOC_SCTRL_SCINT_STAT1_noc_fd_cfg_t_maintimeout_msksta_END (12)
#define SOC_SCTRL_SCINT_STAT1_sc_noc_aon_apb_slv_t_maintimeout_msksta_START (13)
#define SOC_SCTRL_SCINT_STAT1_sc_noc_aon_apb_slv_t_maintimeout_msksta_END (13)
#define SOC_SCTRL_SCINT_STAT1_noc_ao_tcp_cfg_t_maintimeout_mskstat_START (14)
#define SOC_SCTRL_SCINT_STAT1_noc_ao_tcp_cfg_t_maintimeout_mskstat_END (14)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int13_START (15)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int13_END (15)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int1_START (16)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int1_END (16)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int5_START (17)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int5_END (17)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int8_START (18)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int8_END (18)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int7_START (19)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int7_END (19)
#define SOC_SCTRL_SCINT_STAT1_int_gpio31_START (20)
#define SOC_SCTRL_SCINT_STAT1_int_gpio31_END (20)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int0_START (21)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int0_END (21)
#define SOC_SCTRL_SCINT_STAT1_int_gpio32_START (22)
#define SOC_SCTRL_SCINT_STAT1_int_gpio32_END (22)
#define SOC_SCTRL_SCINT_STAT1_int_gpio33_START (23)
#define SOC_SCTRL_SCINT_STAT1_int_gpio33_END (23)
#define SOC_SCTRL_SCINT_STAT1_int_gpio29_START (24)
#define SOC_SCTRL_SCINT_STAT1_int_gpio29_END (24)
#define SOC_SCTRL_SCINT_STAT1_int_gpio30_START (25)
#define SOC_SCTRL_SCINT_STAT1_int_gpio30_END (25)
#define SOC_SCTRL_SCINT_STAT1_gpio_20_int_START (26)
#define SOC_SCTRL_SCINT_STAT1_gpio_20_int_END (26)
#define SOC_SCTRL_SCINT_STAT1_gpio_21_int_START (27)
#define SOC_SCTRL_SCINT_STAT1_gpio_21_int_END (27)
#define SOC_SCTRL_SCINT_STAT1_gpio_28_int_START (28)
#define SOC_SCTRL_SCINT_STAT1_gpio_28_int_END (28)
#define SOC_SCTRL_SCINT_STAT1_se_gpio1_START (29)
#define SOC_SCTRL_SCINT_STAT1_se_gpio1_END (29)
#define SOC_SCTRL_SCINT_STAT1_mad_int_START (30)
#define SOC_SCTRL_SCINT_STAT1_mad_int_END (30)
#define SOC_SCTRL_SCINT_STAT1_intr_ao_wd_START (31)
#define SOC_SCTRL_SCINT_STAT1_intr_ao_wd_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gic_irq_int0 : 1;
        unsigned int gic_irq_int1 : 1;
        unsigned int gic_fiq_int2 : 1;
        unsigned int gic_fiq_int3 : 1;
        unsigned int intr_lpmcu_wdog : 1;
        unsigned int lpmcu_wfi_int : 1;
        unsigned int clk_monitor_intmsk : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_SCTRL_SCINT_MASK2_UNION;
#endif
#define SOC_SCTRL_SCINT_MASK2_gic_irq_int0_START (0)
#define SOC_SCTRL_SCINT_MASK2_gic_irq_int0_END (0)
#define SOC_SCTRL_SCINT_MASK2_gic_irq_int1_START (1)
#define SOC_SCTRL_SCINT_MASK2_gic_irq_int1_END (1)
#define SOC_SCTRL_SCINT_MASK2_gic_fiq_int2_START (2)
#define SOC_SCTRL_SCINT_MASK2_gic_fiq_int2_END (2)
#define SOC_SCTRL_SCINT_MASK2_gic_fiq_int3_START (3)
#define SOC_SCTRL_SCINT_MASK2_gic_fiq_int3_END (3)
#define SOC_SCTRL_SCINT_MASK2_intr_lpmcu_wdog_START (4)
#define SOC_SCTRL_SCINT_MASK2_intr_lpmcu_wdog_END (4)
#define SOC_SCTRL_SCINT_MASK2_lpmcu_wfi_int_START (5)
#define SOC_SCTRL_SCINT_MASK2_lpmcu_wfi_int_END (5)
#define SOC_SCTRL_SCINT_MASK2_clk_monitor_intmsk_START (6)
#define SOC_SCTRL_SCINT_MASK2_clk_monitor_intmsk_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gic_irq_int0 : 1;
        unsigned int gic_irq_int1 : 1;
        unsigned int gic_fiq_int2 : 1;
        unsigned int gic_fiq_int3 : 1;
        unsigned int intr_lpmcu_wdog : 1;
        unsigned int clk_monitor_intr : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SCTRL_SCINT_STAT2_UNION;
#endif
#define SOC_SCTRL_SCINT_STAT2_gic_irq_int0_START (0)
#define SOC_SCTRL_SCINT_STAT2_gic_irq_int0_END (0)
#define SOC_SCTRL_SCINT_STAT2_gic_irq_int1_START (1)
#define SOC_SCTRL_SCINT_STAT2_gic_irq_int1_END (1)
#define SOC_SCTRL_SCINT_STAT2_gic_fiq_int2_START (2)
#define SOC_SCTRL_SCINT_STAT2_gic_fiq_int2_END (2)
#define SOC_SCTRL_SCINT_STAT2_gic_fiq_int3_START (3)
#define SOC_SCTRL_SCINT_STAT2_gic_fiq_int3_END (3)
#define SOC_SCTRL_SCINT_STAT2_intr_lpmcu_wdog_START (4)
#define SOC_SCTRL_SCINT_STAT2_intr_lpmcu_wdog_END (4)
#define SOC_SCTRL_SCINT_STAT2_clk_monitor_intr_START (5)
#define SOC_SCTRL_SCINT_STAT2_clk_monitor_intr_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_peri_gpio_wakeup : 21;
        unsigned int intr_gpio_34 : 1;
        unsigned int intr_gpio_35 : 1;
        unsigned int intr_gpio_36 : 1;
        unsigned int vad_int : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 5;
    } reg;
} SOC_SCTRL_SCINT_MASK3_UNION;
#endif
#define SOC_SCTRL_SCINT_MASK3_intr_peri_gpio_wakeup_START (0)
#define SOC_SCTRL_SCINT_MASK3_intr_peri_gpio_wakeup_END (20)
#define SOC_SCTRL_SCINT_MASK3_intr_gpio_34_START (21)
#define SOC_SCTRL_SCINT_MASK3_intr_gpio_34_END (21)
#define SOC_SCTRL_SCINT_MASK3_intr_gpio_35_START (22)
#define SOC_SCTRL_SCINT_MASK3_intr_gpio_35_END (22)
#define SOC_SCTRL_SCINT_MASK3_intr_gpio_36_START (23)
#define SOC_SCTRL_SCINT_MASK3_intr_gpio_36_END (23)
#define SOC_SCTRL_SCINT_MASK3_vad_int_START (24)
#define SOC_SCTRL_SCINT_MASK3_vad_int_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_peri_gpio_wakeup : 21;
        unsigned int intr_gpio_34 : 1;
        unsigned int intr_gpio_35 : 1;
        unsigned int intr_gpio_36 : 1;
        unsigned int vad_int : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 5;
    } reg;
} SOC_SCTRL_SCINT_STAT3_UNION;
#endif
#define SOC_SCTRL_SCINT_STAT3_intr_peri_gpio_wakeup_START (0)
#define SOC_SCTRL_SCINT_STAT3_intr_peri_gpio_wakeup_END (20)
#define SOC_SCTRL_SCINT_STAT3_intr_gpio_34_START (21)
#define SOC_SCTRL_SCINT_STAT3_intr_gpio_34_END (21)
#define SOC_SCTRL_SCINT_STAT3_intr_gpio_35_START (22)
#define SOC_SCTRL_SCINT_STAT3_intr_gpio_35_END (22)
#define SOC_SCTRL_SCINT_STAT3_intr_gpio_36_START (23)
#define SOC_SCTRL_SCINT_STAT3_intr_gpio_36_END (23)
#define SOC_SCTRL_SCINT_STAT3_vad_int_START (24)
#define SOC_SCTRL_SCINT_STAT3_vad_int_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lte_drx_arm_wakeup_int : 1;
        unsigned int tds_drx_arm_wakeup_int : 1;
        unsigned int g1_int_gbbp_to_cpu_on : 1;
        unsigned int g2_int_gbbp_to_cpu_on : 1;
        unsigned int w_arm_int02 : 1;
        unsigned int cbbp_int01_2 : 1;
        unsigned int g3_int_gbbp_to_cpu_on : 1;
        unsigned int w_arm_int02_2 : 1;
        unsigned int lte2_drx_arm_wakeup_int : 1;
        unsigned int ltev_drx_arm_wakeup_int : 1;
        unsigned int nr_drx_arm_wakeup_int : 1;
        unsigned int nr_drx_arm_4g_wakeup_int : 1;
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
    } reg;
} SOC_SCTRL_SCINT_MASK4_UNION;
#endif
#define SOC_SCTRL_SCINT_MASK4_lte_drx_arm_wakeup_int_START (0)
#define SOC_SCTRL_SCINT_MASK4_lte_drx_arm_wakeup_int_END (0)
#define SOC_SCTRL_SCINT_MASK4_tds_drx_arm_wakeup_int_START (1)
#define SOC_SCTRL_SCINT_MASK4_tds_drx_arm_wakeup_int_END (1)
#define SOC_SCTRL_SCINT_MASK4_g1_int_gbbp_to_cpu_on_START (2)
#define SOC_SCTRL_SCINT_MASK4_g1_int_gbbp_to_cpu_on_END (2)
#define SOC_SCTRL_SCINT_MASK4_g2_int_gbbp_to_cpu_on_START (3)
#define SOC_SCTRL_SCINT_MASK4_g2_int_gbbp_to_cpu_on_END (3)
#define SOC_SCTRL_SCINT_MASK4_w_arm_int02_START (4)
#define SOC_SCTRL_SCINT_MASK4_w_arm_int02_END (4)
#define SOC_SCTRL_SCINT_MASK4_cbbp_int01_2_START (5)
#define SOC_SCTRL_SCINT_MASK4_cbbp_int01_2_END (5)
#define SOC_SCTRL_SCINT_MASK4_g3_int_gbbp_to_cpu_on_START (6)
#define SOC_SCTRL_SCINT_MASK4_g3_int_gbbp_to_cpu_on_END (6)
#define SOC_SCTRL_SCINT_MASK4_w_arm_int02_2_START (7)
#define SOC_SCTRL_SCINT_MASK4_w_arm_int02_2_END (7)
#define SOC_SCTRL_SCINT_MASK4_lte2_drx_arm_wakeup_int_START (8)
#define SOC_SCTRL_SCINT_MASK4_lte2_drx_arm_wakeup_int_END (8)
#define SOC_SCTRL_SCINT_MASK4_ltev_drx_arm_wakeup_int_START (9)
#define SOC_SCTRL_SCINT_MASK4_ltev_drx_arm_wakeup_int_END (9)
#define SOC_SCTRL_SCINT_MASK4_nr_drx_arm_wakeup_int_START (10)
#define SOC_SCTRL_SCINT_MASK4_nr_drx_arm_wakeup_int_END (10)
#define SOC_SCTRL_SCINT_MASK4_nr_drx_arm_4g_wakeup_int_START (11)
#define SOC_SCTRL_SCINT_MASK4_nr_drx_arm_4g_wakeup_int_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lte_drx_arm_wakeup_int : 1;
        unsigned int tds_drx_arm_wakeup_int : 1;
        unsigned int g1_int_gbbp_to_cpu_on : 1;
        unsigned int g2_int_gbbp_to_cpu_on : 1;
        unsigned int w_arm_int02 : 1;
        unsigned int cbbp_int01_2 : 1;
        unsigned int g3_int_gbbp_to_cpu_on : 1;
        unsigned int w_arm_int02_2 : 1;
        unsigned int lte2_drx_arm_wakeup_int : 1;
        unsigned int ltev_drx_arm_wakeup_int : 1;
        unsigned int nr_drx_arm_wakeup_int : 1;
        unsigned int nr_drx_arm_4g_wakeup_int : 1;
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
    } reg;
} SOC_SCTRL_SCINT_STAT4_UNION;
#endif
#define SOC_SCTRL_SCINT_STAT4_lte_drx_arm_wakeup_int_START (0)
#define SOC_SCTRL_SCINT_STAT4_lte_drx_arm_wakeup_int_END (0)
#define SOC_SCTRL_SCINT_STAT4_tds_drx_arm_wakeup_int_START (1)
#define SOC_SCTRL_SCINT_STAT4_tds_drx_arm_wakeup_int_END (1)
#define SOC_SCTRL_SCINT_STAT4_g1_int_gbbp_to_cpu_on_START (2)
#define SOC_SCTRL_SCINT_STAT4_g1_int_gbbp_to_cpu_on_END (2)
#define SOC_SCTRL_SCINT_STAT4_g2_int_gbbp_to_cpu_on_START (3)
#define SOC_SCTRL_SCINT_STAT4_g2_int_gbbp_to_cpu_on_END (3)
#define SOC_SCTRL_SCINT_STAT4_w_arm_int02_START (4)
#define SOC_SCTRL_SCINT_STAT4_w_arm_int02_END (4)
#define SOC_SCTRL_SCINT_STAT4_cbbp_int01_2_START (5)
#define SOC_SCTRL_SCINT_STAT4_cbbp_int01_2_END (5)
#define SOC_SCTRL_SCINT_STAT4_g3_int_gbbp_to_cpu_on_START (6)
#define SOC_SCTRL_SCINT_STAT4_g3_int_gbbp_to_cpu_on_END (6)
#define SOC_SCTRL_SCINT_STAT4_w_arm_int02_2_START (7)
#define SOC_SCTRL_SCINT_STAT4_w_arm_int02_2_END (7)
#define SOC_SCTRL_SCINT_STAT4_lte2_drx_arm_wakeup_int_START (8)
#define SOC_SCTRL_SCINT_STAT4_lte2_drx_arm_wakeup_int_END (8)
#define SOC_SCTRL_SCINT_STAT4_ltev_drx_arm_wakeup_int_START (9)
#define SOC_SCTRL_SCINT_STAT4_ltev_drx_arm_wakeup_int_END (9)
#define SOC_SCTRL_SCINT_STAT4_nr_drx_arm_wakeup_int_START (10)
#define SOC_SCTRL_SCINT_STAT4_nr_drx_arm_wakeup_int_END (10)
#define SOC_SCTRL_SCINT_STAT4_nr_drx_arm_4g_wakeup_int_START (11)
#define SOC_SCTRL_SCINT_STAT4_nr_drx_arm_4g_wakeup_int_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_timer130 : 1;
        unsigned int intr_timer131 : 1;
        unsigned int intr_timer140 : 1;
        unsigned int intr_timer141 : 1;
        unsigned int intr_timer150 : 1;
        unsigned int intr_timer151 : 1;
        unsigned int intr_timer160 : 1;
        unsigned int intr_timer161 : 1;
        unsigned int intr_timer170 : 1;
        unsigned int intr_timer171 : 1;
        unsigned int pcie0_l12wake_int : 1;
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
    } reg;
} SOC_SCTRL_SCINT_MASK5_UNION;
#endif
#define SOC_SCTRL_SCINT_MASK5_intr_timer130_START (0)
#define SOC_SCTRL_SCINT_MASK5_intr_timer130_END (0)
#define SOC_SCTRL_SCINT_MASK5_intr_timer131_START (1)
#define SOC_SCTRL_SCINT_MASK5_intr_timer131_END (1)
#define SOC_SCTRL_SCINT_MASK5_intr_timer140_START (2)
#define SOC_SCTRL_SCINT_MASK5_intr_timer140_END (2)
#define SOC_SCTRL_SCINT_MASK5_intr_timer141_START (3)
#define SOC_SCTRL_SCINT_MASK5_intr_timer141_END (3)
#define SOC_SCTRL_SCINT_MASK5_intr_timer150_START (4)
#define SOC_SCTRL_SCINT_MASK5_intr_timer150_END (4)
#define SOC_SCTRL_SCINT_MASK5_intr_timer151_START (5)
#define SOC_SCTRL_SCINT_MASK5_intr_timer151_END (5)
#define SOC_SCTRL_SCINT_MASK5_intr_timer160_START (6)
#define SOC_SCTRL_SCINT_MASK5_intr_timer160_END (6)
#define SOC_SCTRL_SCINT_MASK5_intr_timer161_START (7)
#define SOC_SCTRL_SCINT_MASK5_intr_timer161_END (7)
#define SOC_SCTRL_SCINT_MASK5_intr_timer170_START (8)
#define SOC_SCTRL_SCINT_MASK5_intr_timer170_END (8)
#define SOC_SCTRL_SCINT_MASK5_intr_timer171_START (9)
#define SOC_SCTRL_SCINT_MASK5_intr_timer171_END (9)
#define SOC_SCTRL_SCINT_MASK5_pcie0_l12wake_int_START (10)
#define SOC_SCTRL_SCINT_MASK5_pcie0_l12wake_int_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_timer130 : 1;
        unsigned int intr_timer131 : 1;
        unsigned int intr_timer140 : 1;
        unsigned int intr_timer141 : 1;
        unsigned int intr_timer150 : 1;
        unsigned int intr_timer151 : 1;
        unsigned int intr_timer160 : 1;
        unsigned int intr_timer161 : 1;
        unsigned int intr_timer170 : 1;
        unsigned int intr_timer171 : 1;
        unsigned int pcie0_l12wake_int : 1;
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
    } reg;
} SOC_SCTRL_SCINT_STAT5_UNION;
#endif
#define SOC_SCTRL_SCINT_STAT5_intr_timer130_START (0)
#define SOC_SCTRL_SCINT_STAT5_intr_timer130_END (0)
#define SOC_SCTRL_SCINT_STAT5_intr_timer131_START (1)
#define SOC_SCTRL_SCINT_STAT5_intr_timer131_END (1)
#define SOC_SCTRL_SCINT_STAT5_intr_timer140_START (2)
#define SOC_SCTRL_SCINT_STAT5_intr_timer140_END (2)
#define SOC_SCTRL_SCINT_STAT5_intr_timer141_START (3)
#define SOC_SCTRL_SCINT_STAT5_intr_timer141_END (3)
#define SOC_SCTRL_SCINT_STAT5_intr_timer150_START (4)
#define SOC_SCTRL_SCINT_STAT5_intr_timer150_END (4)
#define SOC_SCTRL_SCINT_STAT5_intr_timer151_START (5)
#define SOC_SCTRL_SCINT_STAT5_intr_timer151_END (5)
#define SOC_SCTRL_SCINT_STAT5_intr_timer160_START (6)
#define SOC_SCTRL_SCINT_STAT5_intr_timer160_END (6)
#define SOC_SCTRL_SCINT_STAT5_intr_timer161_START (7)
#define SOC_SCTRL_SCINT_STAT5_intr_timer161_END (7)
#define SOC_SCTRL_SCINT_STAT5_intr_timer170_START (8)
#define SOC_SCTRL_SCINT_STAT5_intr_timer170_END (8)
#define SOC_SCTRL_SCINT_STAT5_intr_timer171_START (9)
#define SOC_SCTRL_SCINT_STAT5_intr_timer171_END (9)
#define SOC_SCTRL_SCINT_STAT5_pcie0_l12wake_int_START (10)
#define SOC_SCTRL_SCINT_STAT5_pcie0_l12wake_int_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbintpll_en : 1;
        unsigned int lbintpll_bypass : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int lbintpll_postdiv : 4;
        unsigned int lbintpll_lockcount : 2;
        unsigned int lbintpll_foutpostdiven : 1;
        unsigned int lbintpll_foutvcoven : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int lbintpll_fbdiv : 14;
        unsigned int lbintpll_gt : 1;
        unsigned int sel_lbintpll_out : 1;
        unsigned int reserved_5 : 1;
    } reg;
} SOC_SCTRL_SCLBINTPLL1_CTRL0_UNION;
#endif
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_lbintpll_en_START (0)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_lbintpll_en_END (0)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_lbintpll_bypass_START (1)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_lbintpll_bypass_END (1)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_lbintpll_postdiv_START (5)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_lbintpll_postdiv_END (8)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_lbintpll_lockcount_START (9)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_lbintpll_lockcount_END (10)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_lbintpll_foutpostdiven_START (11)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_lbintpll_foutpostdiven_END (11)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_lbintpll_foutvcoven_START (12)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_lbintpll_foutvcoven_END (12)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_lbintpll_fbdiv_START (15)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_lbintpll_fbdiv_END (28)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_lbintpll_gt_START (29)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_lbintpll_gt_END (29)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_sel_lbintpll_out_START (30)
#define SOC_SCTRL_SCLBINTPLL1_CTRL0_sel_lbintpll_out_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbintpll_faclin : 8;
        unsigned int lbintpll_faclbypass : 1;
        unsigned int lbintpll_facltest : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 18;
    } reg;
} SOC_SCTRL_SCLBINTPLL1_CTRL1_UNION;
#endif
#define SOC_SCTRL_SCLBINTPLL1_CTRL1_lbintpll_faclin_START (0)
#define SOC_SCTRL_SCLBINTPLL1_CTRL1_lbintpll_faclin_END (7)
#define SOC_SCTRL_SCLBINTPLL1_CTRL1_lbintpll_faclbypass_START (8)
#define SOC_SCTRL_SCLBINTPLL1_CTRL1_lbintpll_faclbypass_END (8)
#define SOC_SCTRL_SCLBINTPLL1_CTRL1_lbintpll_facltest_START (9)
#define SOC_SCTRL_SCLBINTPLL1_CTRL1_lbintpll_facltest_END (9)
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
        unsigned int reserved_4: 6;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 3;
        unsigned int reserved_7: 18;
    } reg;
} SOC_SCTRL_SCLBINTPLL1_CTRL2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbintpll_en : 1;
        unsigned int lbintpll_lock : 1;
        unsigned int libintpll_bypass_stat : 1;
        unsigned int st_clk_lbintpll_out : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int libintpll_faclout : 8;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 15;
    } reg;
} SOC_SCTRL_SCLBINTPLL1_STAT_UNION;
#endif
#define SOC_SCTRL_SCLBINTPLL1_STAT_lbintpll_en_START (0)
#define SOC_SCTRL_SCLBINTPLL1_STAT_lbintpll_en_END (0)
#define SOC_SCTRL_SCLBINTPLL1_STAT_lbintpll_lock_START (1)
#define SOC_SCTRL_SCLBINTPLL1_STAT_lbintpll_lock_END (1)
#define SOC_SCTRL_SCLBINTPLL1_STAT_libintpll_bypass_stat_START (2)
#define SOC_SCTRL_SCLBINTPLL1_STAT_libintpll_bypass_stat_END (2)
#define SOC_SCTRL_SCLBINTPLL1_STAT_st_clk_lbintpll_out_START (3)
#define SOC_SCTRL_SCLBINTPLL1_STAT_st_clk_lbintpll_out_END (3)
#define SOC_SCTRL_SCLBINTPLL1_STAT_libintpll_faclout_START (7)
#define SOC_SCTRL_SCLBINTPLL1_STAT_libintpll_faclout_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spll_en : 1;
        unsigned int spll_bp : 1;
        unsigned int spll_refdiv : 6;
        unsigned int spll_fbdiv : 12;
        unsigned int spll_postdiv1 : 3;
        unsigned int spll_postdiv2 : 3;
        unsigned int reserved : 6;
    } reg;
} SOC_SCTRL_SCPPLLCTRL0_UNION;
#endif
#define SOC_SCTRL_SCPPLLCTRL0_spll_en_START (0)
#define SOC_SCTRL_SCPPLLCTRL0_spll_en_END (0)
#define SOC_SCTRL_SCPPLLCTRL0_spll_bp_START (1)
#define SOC_SCTRL_SCPPLLCTRL0_spll_bp_END (1)
#define SOC_SCTRL_SCPPLLCTRL0_spll_refdiv_START (2)
#define SOC_SCTRL_SCPPLLCTRL0_spll_refdiv_END (7)
#define SOC_SCTRL_SCPPLLCTRL0_spll_fbdiv_START (8)
#define SOC_SCTRL_SCPPLLCTRL0_spll_fbdiv_END (19)
#define SOC_SCTRL_SCPPLLCTRL0_spll_postdiv1_START (20)
#define SOC_SCTRL_SCPPLLCTRL0_spll_postdiv1_END (22)
#define SOC_SCTRL_SCPPLLCTRL0_spll_postdiv2_START (23)
#define SOC_SCTRL_SCPPLLCTRL0_spll_postdiv2_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spll_fracdiv : 24;
        unsigned int spll_int_mod : 1;
        unsigned int spll_cfg_vld : 1;
        unsigned int gt_clk_spll : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_SCTRL_SCPPLLCTRL1_UNION;
#endif
#define SOC_SCTRL_SCPPLLCTRL1_spll_fracdiv_START (0)
#define SOC_SCTRL_SCPPLLCTRL1_spll_fracdiv_END (23)
#define SOC_SCTRL_SCPPLLCTRL1_spll_int_mod_START (24)
#define SOC_SCTRL_SCPPLLCTRL1_spll_int_mod_END (24)
#define SOC_SCTRL_SCPPLLCTRL1_spll_cfg_vld_START (25)
#define SOC_SCTRL_SCPPLLCTRL1_spll_cfg_vld_END (25)
#define SOC_SCTRL_SCPPLLCTRL1_gt_clk_spll_START (26)
#define SOC_SCTRL_SCPPLLCTRL1_gt_clk_spll_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spll_ssc_reset : 1;
        unsigned int spll_ssc_disable : 1;
        unsigned int spll_ssc_downspread : 1;
        unsigned int spll_ssc_spread : 3;
        unsigned int spll_ssc_divval : 4;
        unsigned int reserved : 22;
    } reg;
} SOC_SCTRL_SCPPLLSSCCTRL_UNION;
#endif
#define SOC_SCTRL_SCPPLLSSCCTRL_spll_ssc_reset_START (0)
#define SOC_SCTRL_SCPPLLSSCCTRL_spll_ssc_reset_END (0)
#define SOC_SCTRL_SCPPLLSSCCTRL_spll_ssc_disable_START (1)
#define SOC_SCTRL_SCPPLLSSCCTRL_spll_ssc_disable_END (1)
#define SOC_SCTRL_SCPPLLSSCCTRL_spll_ssc_downspread_START (2)
#define SOC_SCTRL_SCPPLLSSCCTRL_spll_ssc_downspread_END (2)
#define SOC_SCTRL_SCPPLLSSCCTRL_spll_ssc_spread_START (3)
#define SOC_SCTRL_SCPPLLSSCCTRL_spll_ssc_spread_END (5)
#define SOC_SCTRL_SCPPLLSSCCTRL_spll_ssc_divval_START (6)
#define SOC_SCTRL_SCPPLLSSCCTRL_spll_ssc_divval_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pllstat : 1;
        unsigned int ppll0_en_stat : 1;
        unsigned int ppll0_gt_stat : 1;
        unsigned int ppll0_bypass_stat : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_SCTRL_SCPPLLSTAT_UNION;
#endif
#define SOC_SCTRL_SCPPLLSTAT_pllstat_START (0)
#define SOC_SCTRL_SCPPLLSTAT_pllstat_END (0)
#define SOC_SCTRL_SCPPLLSTAT_ppll0_en_stat_START (1)
#define SOC_SCTRL_SCPPLLSTAT_ppll0_en_stat_END (1)
#define SOC_SCTRL_SCPPLLSTAT_ppll0_gt_stat_START (2)
#define SOC_SCTRL_SCPPLLSTAT_ppll0_gt_stat_END (2)
#define SOC_SCTRL_SCPPLLSTAT_ppll0_bypass_stat_START (3)
#define SOC_SCTRL_SCPPLLSTAT_ppll0_bypass_stat_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fll_en_1v2 : 1;
        unsigned int sleep_en_fll_1v2 : 1;
        unsigned int fout_en_fll_1v2 : 1;
        unsigned int clkssc_en_fll_1v2 : 1;
        unsigned int postdiv_fll_1v2 : 3;
        unsigned int ac_buf_pd_cfg_fll_1v2 : 1;
        unsigned int lock_thresh_fll_1v2 : 6;
        unsigned int ac_buf_sel : 1;
        unsigned int calib_mode_sel : 1;
        unsigned int fcw_fll_1v2 : 15;
        unsigned int reserved : 1;
    } reg;
} SOC_SCTRL_SCFPLLCTRL0_UNION;
#endif
#define SOC_SCTRL_SCFPLLCTRL0_fll_en_1v2_START (0)
#define SOC_SCTRL_SCFPLLCTRL0_fll_en_1v2_END (0)
#define SOC_SCTRL_SCFPLLCTRL0_sleep_en_fll_1v2_START (1)
#define SOC_SCTRL_SCFPLLCTRL0_sleep_en_fll_1v2_END (1)
#define SOC_SCTRL_SCFPLLCTRL0_fout_en_fll_1v2_START (2)
#define SOC_SCTRL_SCFPLLCTRL0_fout_en_fll_1v2_END (2)
#define SOC_SCTRL_SCFPLLCTRL0_clkssc_en_fll_1v2_START (3)
#define SOC_SCTRL_SCFPLLCTRL0_clkssc_en_fll_1v2_END (3)
#define SOC_SCTRL_SCFPLLCTRL0_postdiv_fll_1v2_START (4)
#define SOC_SCTRL_SCFPLLCTRL0_postdiv_fll_1v2_END (6)
#define SOC_SCTRL_SCFPLLCTRL0_ac_buf_pd_cfg_fll_1v2_START (7)
#define SOC_SCTRL_SCFPLLCTRL0_ac_buf_pd_cfg_fll_1v2_END (7)
#define SOC_SCTRL_SCFPLLCTRL0_lock_thresh_fll_1v2_START (8)
#define SOC_SCTRL_SCFPLLCTRL0_lock_thresh_fll_1v2_END (13)
#define SOC_SCTRL_SCFPLLCTRL0_ac_buf_sel_START (14)
#define SOC_SCTRL_SCFPLLCTRL0_ac_buf_sel_END (14)
#define SOC_SCTRL_SCFPLLCTRL0_calib_mode_sel_START (15)
#define SOC_SCTRL_SCFPLLCTRL0_calib_mode_sel_END (15)
#define SOC_SCTRL_SCFPLLCTRL0_fcw_fll_1v2_START (16)
#define SOC_SCTRL_SCFPLLCTRL0_fcw_fll_1v2_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bypass_fll_1v2 : 1;
        unsigned int overlap_en_fll_1v2 : 1;
        unsigned int dsm_en_fll_1v2 : 1;
        unsigned int ref_det_en_fll_1v2 : 1;
        unsigned int ref_det_thresh_fll_1v2 : 4;
        unsigned int calib_byp_fll_1v2 : 1;
        unsigned int calib_thresh_fll_1v2 : 5;
        unsigned int reserved_0 : 2;
        unsigned int test_data_en_fll_1v2 : 1;
        unsigned int test_dco_en_fll_1v2 : 1;
        unsigned int read_en_fll_1v2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int test_data_sel_fll_1v2 : 2;
        unsigned int reserved_2 : 2;
        unsigned int test_dco_i_fll_1v2 : 7;
        unsigned int reserved_3 : 1;
    } reg;
} SOC_SCTRL_SCFPLLCTRL1_UNION;
#endif
#define SOC_SCTRL_SCFPLLCTRL1_bypass_fll_1v2_START (0)
#define SOC_SCTRL_SCFPLLCTRL1_bypass_fll_1v2_END (0)
#define SOC_SCTRL_SCFPLLCTRL1_overlap_en_fll_1v2_START (1)
#define SOC_SCTRL_SCFPLLCTRL1_overlap_en_fll_1v2_END (1)
#define SOC_SCTRL_SCFPLLCTRL1_dsm_en_fll_1v2_START (2)
#define SOC_SCTRL_SCFPLLCTRL1_dsm_en_fll_1v2_END (2)
#define SOC_SCTRL_SCFPLLCTRL1_ref_det_en_fll_1v2_START (3)
#define SOC_SCTRL_SCFPLLCTRL1_ref_det_en_fll_1v2_END (3)
#define SOC_SCTRL_SCFPLLCTRL1_ref_det_thresh_fll_1v2_START (4)
#define SOC_SCTRL_SCFPLLCTRL1_ref_det_thresh_fll_1v2_END (7)
#define SOC_SCTRL_SCFPLLCTRL1_calib_byp_fll_1v2_START (8)
#define SOC_SCTRL_SCFPLLCTRL1_calib_byp_fll_1v2_END (8)
#define SOC_SCTRL_SCFPLLCTRL1_calib_thresh_fll_1v2_START (9)
#define SOC_SCTRL_SCFPLLCTRL1_calib_thresh_fll_1v2_END (13)
#define SOC_SCTRL_SCFPLLCTRL1_test_data_en_fll_1v2_START (16)
#define SOC_SCTRL_SCFPLLCTRL1_test_data_en_fll_1v2_END (16)
#define SOC_SCTRL_SCFPLLCTRL1_test_dco_en_fll_1v2_START (17)
#define SOC_SCTRL_SCFPLLCTRL1_test_dco_en_fll_1v2_END (17)
#define SOC_SCTRL_SCFPLLCTRL1_read_en_fll_1v2_START (18)
#define SOC_SCTRL_SCFPLLCTRL1_read_en_fll_1v2_END (18)
#define SOC_SCTRL_SCFPLLCTRL1_test_data_sel_fll_1v2_START (20)
#define SOC_SCTRL_SCFPLLCTRL1_test_data_sel_fll_1v2_END (21)
#define SOC_SCTRL_SCFPLLCTRL1_test_dco_i_fll_1v2_START (24)
#define SOC_SCTRL_SCFPLLCTRL1_test_dco_i_fll_1v2_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int init_calib_code_fll_1v2 : 7;
        unsigned int reserved_0 : 1;
        unsigned int bw_cfg_fll_1v2 : 5;
        unsigned int ck_filter_cfg : 3;
        unsigned int decap_sw_fll_1v2 : 1;
        unsigned int filter_mos_sw_fll_1v2 : 1;
        unsigned int filter_rhm_sw_fll_1v2 : 1;
        unsigned int bias_sel_fll_1v2 : 1;
        unsigned int bias_test_en_fll_1v2 : 1;
        unsigned int fix_cc_fll_1v2 : 4;
        unsigned int reserved_1 : 1;
        unsigned int kdco_ctrl_fll_1v2 : 6;
    } reg;
} SOC_SCTRL_SCFPLLCTRL2_UNION;
#endif
#define SOC_SCTRL_SCFPLLCTRL2_init_calib_code_fll_1v2_START (0)
#define SOC_SCTRL_SCFPLLCTRL2_init_calib_code_fll_1v2_END (6)
#define SOC_SCTRL_SCFPLLCTRL2_bw_cfg_fll_1v2_START (8)
#define SOC_SCTRL_SCFPLLCTRL2_bw_cfg_fll_1v2_END (12)
#define SOC_SCTRL_SCFPLLCTRL2_ck_filter_cfg_START (13)
#define SOC_SCTRL_SCFPLLCTRL2_ck_filter_cfg_END (15)
#define SOC_SCTRL_SCFPLLCTRL2_decap_sw_fll_1v2_START (16)
#define SOC_SCTRL_SCFPLLCTRL2_decap_sw_fll_1v2_END (16)
#define SOC_SCTRL_SCFPLLCTRL2_filter_mos_sw_fll_1v2_START (17)
#define SOC_SCTRL_SCFPLLCTRL2_filter_mos_sw_fll_1v2_END (17)
#define SOC_SCTRL_SCFPLLCTRL2_filter_rhm_sw_fll_1v2_START (18)
#define SOC_SCTRL_SCFPLLCTRL2_filter_rhm_sw_fll_1v2_END (18)
#define SOC_SCTRL_SCFPLLCTRL2_bias_sel_fll_1v2_START (19)
#define SOC_SCTRL_SCFPLLCTRL2_bias_sel_fll_1v2_END (19)
#define SOC_SCTRL_SCFPLLCTRL2_bias_test_en_fll_1v2_START (20)
#define SOC_SCTRL_SCFPLLCTRL2_bias_test_en_fll_1v2_END (20)
#define SOC_SCTRL_SCFPLLCTRL2_fix_cc_fll_1v2_START (21)
#define SOC_SCTRL_SCFPLLCTRL2_fix_cc_fll_1v2_END (24)
#define SOC_SCTRL_SCFPLLCTRL2_kdco_ctrl_fll_1v2_START (26)
#define SOC_SCTRL_SCFPLLCTRL2_kdco_ctrl_fll_1v2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int voutb_test_en_fll_1v2 : 1;
        unsigned int reserved_0 : 3;
        unsigned int vouta_test_en_fll_1v2 : 1;
        unsigned int reserved_1 : 7;
        unsigned int i_test_en_fll_1v2 : 1;
        unsigned int reserved_2 : 19;
    } reg;
} SOC_SCTRL_SCFPLLCTRL3_UNION;
#endif
#define SOC_SCTRL_SCFPLLCTRL3_voutb_test_en_fll_1v2_START (0)
#define SOC_SCTRL_SCFPLLCTRL3_voutb_test_en_fll_1v2_END (0)
#define SOC_SCTRL_SCFPLLCTRL3_vouta_test_en_fll_1v2_START (4)
#define SOC_SCTRL_SCFPLLCTRL3_vouta_test_en_fll_1v2_END (4)
#define SOC_SCTRL_SCFPLLCTRL3_i_test_en_fll_1v2_START (12)
#define SOC_SCTRL_SCFPLLCTRL3_i_test_en_fll_1v2_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lock_fll_1v2 : 1;
        unsigned int calib_done_fll_1v2 : 1;
        unsigned int ref_lost_fll_1v2 : 1;
        unsigned int pll_unlock : 1;
        unsigned int ocw_dsm_fll_1v2 : 2;
        unsigned int abnormal_calib : 1;
        unsigned int overflow_fll_1v2 : 1;
        unsigned int test_data_fll_1v2 : 8;
        unsigned int reserved : 16;
    } reg;
} SOC_SCTRL_SCFPLLSTAT_UNION;
#endif
#define SOC_SCTRL_SCFPLLSTAT_lock_fll_1v2_START (0)
#define SOC_SCTRL_SCFPLLSTAT_lock_fll_1v2_END (0)
#define SOC_SCTRL_SCFPLLSTAT_calib_done_fll_1v2_START (1)
#define SOC_SCTRL_SCFPLLSTAT_calib_done_fll_1v2_END (1)
#define SOC_SCTRL_SCFPLLSTAT_ref_lost_fll_1v2_START (2)
#define SOC_SCTRL_SCFPLLSTAT_ref_lost_fll_1v2_END (2)
#define SOC_SCTRL_SCFPLLSTAT_pll_unlock_START (3)
#define SOC_SCTRL_SCFPLLSTAT_pll_unlock_END (3)
#define SOC_SCTRL_SCFPLLSTAT_ocw_dsm_fll_1v2_START (4)
#define SOC_SCTRL_SCFPLLSTAT_ocw_dsm_fll_1v2_END (5)
#define SOC_SCTRL_SCFPLLSTAT_abnormal_calib_START (6)
#define SOC_SCTRL_SCFPLLSTAT_abnormal_calib_END (6)
#define SOC_SCTRL_SCFPLLSTAT_overflow_fll_1v2_START (7)
#define SOC_SCTRL_SCFPLLSTAT_overflow_fll_1v2_END (7)
#define SOC_SCTRL_SCFPLLSTAT_test_data_fll_1v2_START (8)
#define SOC_SCTRL_SCFPLLSTAT_test_data_fll_1v2_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_ref_crc : 1;
        unsigned int gt_pclk_rtc : 1;
        unsigned int gt_pclk_rtc1 : 1;
        unsigned int gt_pclk_timer0 : 1;
        unsigned int gt_clk_timer0 : 1;
        unsigned int gt_clk_mad_32k : 1;
        unsigned int gt_clk_fll_test_src : 1;
        unsigned int gt_pclk_timer2 : 1;
        unsigned int gt_clk_timer2 : 1;
        unsigned int gt_pclk_timer3 : 1;
        unsigned int gt_clk_timer3 : 1;
        unsigned int gt_pclk_ao_gpio0 : 1;
        unsigned int gt_pclk_ao_gpio1 : 1;
        unsigned int gt_pclk_ao_gpio2 : 1;
        unsigned int gt_pclk_ao_gpio3 : 1;
        unsigned int gt_pclk_ao_ioc : 1;
        unsigned int gt_clk_asp_codec_backup : 1;
        unsigned int gt_clk_mad_acpu : 1;
        unsigned int gt_clk_jtag_auth : 1;
        unsigned int gt_pclk_syscnt : 1;
        unsigned int gt_clk_syscnt : 1;
        unsigned int gt_pclk_ao_gpio4 : 1;
        unsigned int gt_pclk_ao_gpio5 : 1;
        unsigned int gt_clk_sci0 : 1;
        unsigned int gt_clk_sci1 : 1;
        unsigned int gt_pclk_ao_gpio6 : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_clk_asp_tcxo : 1;
        unsigned int gt_pclk_bbpdrx : 1;
        unsigned int gt_clk_aobus : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_spll_sscg : 1;
    } reg;
} SOC_SCTRL_SCPEREN0_UNION;
#endif
#define SOC_SCTRL_SCPEREN0_gt_clk_ref_crc_START (0)
#define SOC_SCTRL_SCPEREN0_gt_clk_ref_crc_END (0)
#define SOC_SCTRL_SCPEREN0_gt_pclk_rtc_START (1)
#define SOC_SCTRL_SCPEREN0_gt_pclk_rtc_END (1)
#define SOC_SCTRL_SCPEREN0_gt_pclk_rtc1_START (2)
#define SOC_SCTRL_SCPEREN0_gt_pclk_rtc1_END (2)
#define SOC_SCTRL_SCPEREN0_gt_pclk_timer0_START (3)
#define SOC_SCTRL_SCPEREN0_gt_pclk_timer0_END (3)
#define SOC_SCTRL_SCPEREN0_gt_clk_timer0_START (4)
#define SOC_SCTRL_SCPEREN0_gt_clk_timer0_END (4)
#define SOC_SCTRL_SCPEREN0_gt_clk_mad_32k_START (5)
#define SOC_SCTRL_SCPEREN0_gt_clk_mad_32k_END (5)
#define SOC_SCTRL_SCPEREN0_gt_clk_fll_test_src_START (6)
#define SOC_SCTRL_SCPEREN0_gt_clk_fll_test_src_END (6)
#define SOC_SCTRL_SCPEREN0_gt_pclk_timer2_START (7)
#define SOC_SCTRL_SCPEREN0_gt_pclk_timer2_END (7)
#define SOC_SCTRL_SCPEREN0_gt_clk_timer2_START (8)
#define SOC_SCTRL_SCPEREN0_gt_clk_timer2_END (8)
#define SOC_SCTRL_SCPEREN0_gt_pclk_timer3_START (9)
#define SOC_SCTRL_SCPEREN0_gt_pclk_timer3_END (9)
#define SOC_SCTRL_SCPEREN0_gt_clk_timer3_START (10)
#define SOC_SCTRL_SCPEREN0_gt_clk_timer3_END (10)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio0_START (11)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio0_END (11)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio1_START (12)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio1_END (12)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio2_START (13)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio2_END (13)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio3_START (14)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio3_END (14)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_ioc_START (15)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_ioc_END (15)
#define SOC_SCTRL_SCPEREN0_gt_clk_asp_codec_backup_START (16)
#define SOC_SCTRL_SCPEREN0_gt_clk_asp_codec_backup_END (16)
#define SOC_SCTRL_SCPEREN0_gt_clk_mad_acpu_START (17)
#define SOC_SCTRL_SCPEREN0_gt_clk_mad_acpu_END (17)
#define SOC_SCTRL_SCPEREN0_gt_clk_jtag_auth_START (18)
#define SOC_SCTRL_SCPEREN0_gt_clk_jtag_auth_END (18)
#define SOC_SCTRL_SCPEREN0_gt_pclk_syscnt_START (19)
#define SOC_SCTRL_SCPEREN0_gt_pclk_syscnt_END (19)
#define SOC_SCTRL_SCPEREN0_gt_clk_syscnt_START (20)
#define SOC_SCTRL_SCPEREN0_gt_clk_syscnt_END (20)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio4_START (21)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio4_END (21)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio5_START (22)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio5_END (22)
#define SOC_SCTRL_SCPEREN0_gt_clk_sci0_START (23)
#define SOC_SCTRL_SCPEREN0_gt_clk_sci0_END (23)
#define SOC_SCTRL_SCPEREN0_gt_clk_sci1_START (24)
#define SOC_SCTRL_SCPEREN0_gt_clk_sci1_END (24)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio6_START (25)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio6_END (25)
#define SOC_SCTRL_SCPEREN0_gt_clk_asp_tcxo_START (27)
#define SOC_SCTRL_SCPEREN0_gt_clk_asp_tcxo_END (27)
#define SOC_SCTRL_SCPEREN0_gt_pclk_bbpdrx_START (28)
#define SOC_SCTRL_SCPEREN0_gt_pclk_bbpdrx_END (28)
#define SOC_SCTRL_SCPEREN0_gt_clk_aobus_START (29)
#define SOC_SCTRL_SCPEREN0_gt_clk_aobus_END (29)
#define SOC_SCTRL_SCPEREN0_gt_clk_spll_sscg_START (31)
#define SOC_SCTRL_SCPEREN0_gt_clk_spll_sscg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_ref_crc : 1;
        unsigned int gt_pclk_rtc : 1;
        unsigned int gt_pclk_rtc1 : 1;
        unsigned int gt_pclk_timer0 : 1;
        unsigned int gt_clk_timer0 : 1;
        unsigned int gt_clk_mad_32k : 1;
        unsigned int gt_clk_fll_test_src : 1;
        unsigned int gt_pclk_timer2 : 1;
        unsigned int gt_clk_timer2 : 1;
        unsigned int gt_pclk_timer3 : 1;
        unsigned int gt_clk_timer3 : 1;
        unsigned int gt_pclk_ao_gpio0 : 1;
        unsigned int gt_pclk_ao_gpio1 : 1;
        unsigned int gt_pclk_ao_gpio2 : 1;
        unsigned int gt_pclk_ao_gpio3 : 1;
        unsigned int gt_pclk_ao_ioc : 1;
        unsigned int gt_clk_asp_codec_backup : 1;
        unsigned int gt_clk_mad_acpu : 1;
        unsigned int gt_clk_jtag_auth : 1;
        unsigned int gt_pclk_syscnt : 1;
        unsigned int gt_clk_syscnt : 1;
        unsigned int gt_pclk_ao_gpio4 : 1;
        unsigned int gt_pclk_ao_gpio5 : 1;
        unsigned int gt_clk_sci0 : 1;
        unsigned int gt_clk_sci1 : 1;
        unsigned int gt_pclk_ao_gpio6 : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_clk_asp_tcxo : 1;
        unsigned int gt_pclk_bbpdrx : 1;
        unsigned int gt_clk_aobus : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_spll_sscg : 1;
    } reg;
} SOC_SCTRL_SCPERDIS0_UNION;
#endif
#define SOC_SCTRL_SCPERDIS0_gt_clk_ref_crc_START (0)
#define SOC_SCTRL_SCPERDIS0_gt_clk_ref_crc_END (0)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_rtc_START (1)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_rtc_END (1)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_rtc1_START (2)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_rtc1_END (2)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_timer0_START (3)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_timer0_END (3)
#define SOC_SCTRL_SCPERDIS0_gt_clk_timer0_START (4)
#define SOC_SCTRL_SCPERDIS0_gt_clk_timer0_END (4)
#define SOC_SCTRL_SCPERDIS0_gt_clk_mad_32k_START (5)
#define SOC_SCTRL_SCPERDIS0_gt_clk_mad_32k_END (5)
#define SOC_SCTRL_SCPERDIS0_gt_clk_fll_test_src_START (6)
#define SOC_SCTRL_SCPERDIS0_gt_clk_fll_test_src_END (6)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_timer2_START (7)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_timer2_END (7)
#define SOC_SCTRL_SCPERDIS0_gt_clk_timer2_START (8)
#define SOC_SCTRL_SCPERDIS0_gt_clk_timer2_END (8)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_timer3_START (9)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_timer3_END (9)
#define SOC_SCTRL_SCPERDIS0_gt_clk_timer3_START (10)
#define SOC_SCTRL_SCPERDIS0_gt_clk_timer3_END (10)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_gpio0_START (11)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_gpio0_END (11)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_gpio1_START (12)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_gpio1_END (12)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_gpio2_START (13)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_gpio2_END (13)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_gpio3_START (14)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_gpio3_END (14)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_ioc_START (15)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_ioc_END (15)
#define SOC_SCTRL_SCPERDIS0_gt_clk_asp_codec_backup_START (16)
#define SOC_SCTRL_SCPERDIS0_gt_clk_asp_codec_backup_END (16)
#define SOC_SCTRL_SCPERDIS0_gt_clk_mad_acpu_START (17)
#define SOC_SCTRL_SCPERDIS0_gt_clk_mad_acpu_END (17)
#define SOC_SCTRL_SCPERDIS0_gt_clk_jtag_auth_START (18)
#define SOC_SCTRL_SCPERDIS0_gt_clk_jtag_auth_END (18)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_syscnt_START (19)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_syscnt_END (19)
#define SOC_SCTRL_SCPERDIS0_gt_clk_syscnt_START (20)
#define SOC_SCTRL_SCPERDIS0_gt_clk_syscnt_END (20)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_gpio4_START (21)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_gpio4_END (21)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_gpio5_START (22)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_gpio5_END (22)
#define SOC_SCTRL_SCPERDIS0_gt_clk_sci0_START (23)
#define SOC_SCTRL_SCPERDIS0_gt_clk_sci0_END (23)
#define SOC_SCTRL_SCPERDIS0_gt_clk_sci1_START (24)
#define SOC_SCTRL_SCPERDIS0_gt_clk_sci1_END (24)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_gpio6_START (25)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_gpio6_END (25)
#define SOC_SCTRL_SCPERDIS0_gt_clk_asp_tcxo_START (27)
#define SOC_SCTRL_SCPERDIS0_gt_clk_asp_tcxo_END (27)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_bbpdrx_START (28)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_bbpdrx_END (28)
#define SOC_SCTRL_SCPERDIS0_gt_clk_aobus_START (29)
#define SOC_SCTRL_SCPERDIS0_gt_clk_aobus_END (29)
#define SOC_SCTRL_SCPERDIS0_gt_clk_spll_sscg_START (31)
#define SOC_SCTRL_SCPERDIS0_gt_clk_spll_sscg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_ref_crc : 1;
        unsigned int gt_pclk_rtc : 1;
        unsigned int gt_pclk_rtc1 : 1;
        unsigned int gt_pclk_timer0 : 1;
        unsigned int gt_clk_timer0 : 1;
        unsigned int gt_clk_mad_32k : 1;
        unsigned int gt_clk_fll_test_src : 1;
        unsigned int gt_pclk_timer2 : 1;
        unsigned int gt_clk_timer2 : 1;
        unsigned int gt_pclk_timer3 : 1;
        unsigned int gt_clk_timer3 : 1;
        unsigned int gt_pclk_ao_gpio0 : 1;
        unsigned int gt_pclk_ao_gpio1 : 1;
        unsigned int gt_pclk_ao_gpio2 : 1;
        unsigned int gt_pclk_ao_gpio3 : 1;
        unsigned int gt_pclk_ao_ioc : 1;
        unsigned int gt_clk_asp_codec_backup : 1;
        unsigned int gt_clk_mad_acpu : 1;
        unsigned int gt_clk_jtag_auth : 1;
        unsigned int gt_pclk_syscnt : 1;
        unsigned int gt_clk_syscnt : 1;
        unsigned int gt_pclk_ao_gpio4 : 1;
        unsigned int gt_pclk_ao_gpio5 : 1;
        unsigned int gt_clk_sci0 : 1;
        unsigned int gt_clk_sci1 : 1;
        unsigned int gt_pclk_ao_gpio6 : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_clk_asp_tcxo : 1;
        unsigned int gt_pclk_bbpdrx : 1;
        unsigned int gt_clk_aobus : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_spll_sscg : 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN0_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_ref_crc_START (0)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_ref_crc_END (0)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_rtc_START (1)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_rtc_END (1)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_rtc1_START (2)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_rtc1_END (2)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_timer0_START (3)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_timer0_END (3)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_timer0_START (4)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_timer0_END (4)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_mad_32k_START (5)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_mad_32k_END (5)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_fll_test_src_START (6)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_fll_test_src_END (6)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_timer2_START (7)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_timer2_END (7)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_timer2_START (8)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_timer2_END (8)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_timer3_START (9)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_timer3_END (9)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_timer3_START (10)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_timer3_END (10)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_gpio0_START (11)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_gpio0_END (11)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_gpio1_START (12)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_gpio1_END (12)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_gpio2_START (13)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_gpio2_END (13)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_gpio3_START (14)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_gpio3_END (14)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_ioc_START (15)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_ioc_END (15)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_asp_codec_backup_START (16)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_asp_codec_backup_END (16)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_mad_acpu_START (17)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_mad_acpu_END (17)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_jtag_auth_START (18)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_jtag_auth_END (18)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_syscnt_START (19)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_syscnt_END (19)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_syscnt_START (20)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_syscnt_END (20)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_gpio4_START (21)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_gpio4_END (21)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_gpio5_START (22)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_gpio5_END (22)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_sci0_START (23)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_sci0_END (23)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_sci1_START (24)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_sci1_END (24)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_gpio6_START (25)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_gpio6_END (25)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_asp_tcxo_START (27)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_asp_tcxo_END (27)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_bbpdrx_START (28)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_bbpdrx_END (28)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_aobus_START (29)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_aobus_END (29)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_spll_sscg_START (31)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_spll_sscg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int st_clk_ref_crc : 1;
        unsigned int st_pclk_rtc : 1;
        unsigned int st_pclk_rtc1 : 1;
        unsigned int st_pclk_timer0 : 1;
        unsigned int st_clk_timer0 : 1;
        unsigned int st_pclk_timer1 : 1;
        unsigned int st_clk_timer1 : 1;
        unsigned int st_pclk_timer2 : 1;
        unsigned int st_clk_timer2 : 1;
        unsigned int st_pclk_timer3 : 1;
        unsigned int st_clk_timer3 : 1;
        unsigned int st_pclk_ao_gpio0 : 1;
        unsigned int st_pclk_ao_gpio1 : 1;
        unsigned int st_pclk_ao_gpio2 : 1;
        unsigned int st_pclk_ao_gpio3 : 1;
        unsigned int st_pclk_ao_ioc : 1;
        unsigned int st_clk_asp_codec_backup : 1;
        unsigned int st_clk_out0 : 1;
        unsigned int st_clk_jtag_auth : 1;
        unsigned int st_pclk_syscnt : 1;
        unsigned int st_clk_syscnt : 1;
        unsigned int st_pclk_ao_gpio4 : 1;
        unsigned int st_pclk_ao_gpio5 : 1;
        unsigned int st_clk_sci0 : 1;
        unsigned int st_clk_sci1 : 1;
        unsigned int st_clk_mad : 1;
        unsigned int st_clk_asp_subsys : 1;
        unsigned int st_clk_asp_tcxo : 1;
        unsigned int st_pclk_bbpdrx : 1;
        unsigned int st_clk_aobus : 1;
        unsigned int st_pclk_efusec : 1;
        unsigned int st_clk_spll_sscg : 1;
    } reg;
} SOC_SCTRL_SCPERSTAT0_UNION;
#endif
#define SOC_SCTRL_SCPERSTAT0_st_clk_ref_crc_START (0)
#define SOC_SCTRL_SCPERSTAT0_st_clk_ref_crc_END (0)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_rtc_START (1)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_rtc_END (1)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_rtc1_START (2)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_rtc1_END (2)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_timer0_START (3)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_timer0_END (3)
#define SOC_SCTRL_SCPERSTAT0_st_clk_timer0_START (4)
#define SOC_SCTRL_SCPERSTAT0_st_clk_timer0_END (4)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_timer1_START (5)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_timer1_END (5)
#define SOC_SCTRL_SCPERSTAT0_st_clk_timer1_START (6)
#define SOC_SCTRL_SCPERSTAT0_st_clk_timer1_END (6)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_timer2_START (7)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_timer2_END (7)
#define SOC_SCTRL_SCPERSTAT0_st_clk_timer2_START (8)
#define SOC_SCTRL_SCPERSTAT0_st_clk_timer2_END (8)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_timer3_START (9)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_timer3_END (9)
#define SOC_SCTRL_SCPERSTAT0_st_clk_timer3_START (10)
#define SOC_SCTRL_SCPERSTAT0_st_clk_timer3_END (10)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_ao_gpio0_START (11)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_ao_gpio0_END (11)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_ao_gpio1_START (12)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_ao_gpio1_END (12)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_ao_gpio2_START (13)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_ao_gpio2_END (13)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_ao_gpio3_START (14)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_ao_gpio3_END (14)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_ao_ioc_START (15)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_ao_ioc_END (15)
#define SOC_SCTRL_SCPERSTAT0_st_clk_asp_codec_backup_START (16)
#define SOC_SCTRL_SCPERSTAT0_st_clk_asp_codec_backup_END (16)
#define SOC_SCTRL_SCPERSTAT0_st_clk_out0_START (17)
#define SOC_SCTRL_SCPERSTAT0_st_clk_out0_END (17)
#define SOC_SCTRL_SCPERSTAT0_st_clk_jtag_auth_START (18)
#define SOC_SCTRL_SCPERSTAT0_st_clk_jtag_auth_END (18)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_syscnt_START (19)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_syscnt_END (19)
#define SOC_SCTRL_SCPERSTAT0_st_clk_syscnt_START (20)
#define SOC_SCTRL_SCPERSTAT0_st_clk_syscnt_END (20)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_ao_gpio4_START (21)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_ao_gpio4_END (21)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_ao_gpio5_START (22)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_ao_gpio5_END (22)
#define SOC_SCTRL_SCPERSTAT0_st_clk_sci0_START (23)
#define SOC_SCTRL_SCPERSTAT0_st_clk_sci0_END (23)
#define SOC_SCTRL_SCPERSTAT0_st_clk_sci1_START (24)
#define SOC_SCTRL_SCPERSTAT0_st_clk_sci1_END (24)
#define SOC_SCTRL_SCPERSTAT0_st_clk_mad_START (25)
#define SOC_SCTRL_SCPERSTAT0_st_clk_mad_END (25)
#define SOC_SCTRL_SCPERSTAT0_st_clk_asp_subsys_START (26)
#define SOC_SCTRL_SCPERSTAT0_st_clk_asp_subsys_END (26)
#define SOC_SCTRL_SCPERSTAT0_st_clk_asp_tcxo_START (27)
#define SOC_SCTRL_SCPERSTAT0_st_clk_asp_tcxo_END (27)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_bbpdrx_START (28)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_bbpdrx_END (28)
#define SOC_SCTRL_SCPERSTAT0_st_clk_aobus_START (29)
#define SOC_SCTRL_SCPERSTAT0_st_clk_aobus_END (29)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_efusec_START (30)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_efusec_END (30)
#define SOC_SCTRL_SCPERSTAT0_st_clk_spll_sscg_START (31)
#define SOC_SCTRL_SCPERSTAT0_st_clk_spll_sscg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_asp_dw_axi : 1;
        unsigned int gt_clk_asp_x2h : 1;
        unsigned int gt_clk_asp_h2p : 1;
        unsigned int gt_asp_cfg : 1;
        unsigned int gt_clk_asp_subsys_acpu : 1;
        unsigned int gt_clk_noc_aobus2sysbusddrc : 1;
        unsigned int gt_clk_ao_camera : 1;
        unsigned int ddr_loopback_en : 1;
        unsigned int gt_clk_out1 : 1;
        unsigned int gt_clk_fll_src_tp : 1;
        unsigned int gt_pclk_timer4 : 1;
        unsigned int gt_clk_timer4 : 1;
        unsigned int gt_pclk_timer5 : 1;
        unsigned int gt_clk_timer5 : 1;
        unsigned int gt_pclk_timer6 : 1;
        unsigned int gt_clk_timer6 : 1;
        unsigned int gt_pclk_timer7 : 1;
        unsigned int gt_clk_timer7 : 1;
        unsigned int gt_pclk_timer8 : 1;
        unsigned int gt_clk_timer8 : 1;
        unsigned int gt_timerclk_refh : 1;
        unsigned int ddr_reset_en : 1;
        unsigned int gt_clk_hifd_fll : 1;
        unsigned int gt_clk_hifd_pll : 1;
        unsigned int gt_clk_noc_aobus2aspbus : 1;
        unsigned int gt_clk_noc_aobus2iomcubus : 1;
        unsigned int gt_clk_hsdt_subsys : 1;
        unsigned int gt_clk_asp_codec_acpu : 1;
        unsigned int gt_clk_autodiv_aonoc_pll : 1;
        unsigned int gt_clk_autodiv_aonoc_fll : 1;
        unsigned int reserved : 1;
        unsigned int wait_ddr_selfreflash_done_bypass : 1;
    } reg;
} SOC_SCTRL_SCPEREN1_UNION;
#endif
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_dw_axi_START (0)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_dw_axi_END (0)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_x2h_START (1)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_x2h_END (1)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_h2p_START (2)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_h2p_END (2)
#define SOC_SCTRL_SCPEREN1_gt_asp_cfg_START (3)
#define SOC_SCTRL_SCPEREN1_gt_asp_cfg_END (3)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_acpu_START (4)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_acpu_END (4)
#define SOC_SCTRL_SCPEREN1_gt_clk_noc_aobus2sysbusddrc_START (5)
#define SOC_SCTRL_SCPEREN1_gt_clk_noc_aobus2sysbusddrc_END (5)
#define SOC_SCTRL_SCPEREN1_gt_clk_ao_camera_START (6)
#define SOC_SCTRL_SCPEREN1_gt_clk_ao_camera_END (6)
#define SOC_SCTRL_SCPEREN1_ddr_loopback_en_START (7)
#define SOC_SCTRL_SCPEREN1_ddr_loopback_en_END (7)
#define SOC_SCTRL_SCPEREN1_gt_clk_out1_START (8)
#define SOC_SCTRL_SCPEREN1_gt_clk_out1_END (8)
#define SOC_SCTRL_SCPEREN1_gt_clk_fll_src_tp_START (9)
#define SOC_SCTRL_SCPEREN1_gt_clk_fll_src_tp_END (9)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer4_START (10)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer4_END (10)
#define SOC_SCTRL_SCPEREN1_gt_clk_timer4_START (11)
#define SOC_SCTRL_SCPEREN1_gt_clk_timer4_END (11)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer5_START (12)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer5_END (12)
#define SOC_SCTRL_SCPEREN1_gt_clk_timer5_START (13)
#define SOC_SCTRL_SCPEREN1_gt_clk_timer5_END (13)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer6_START (14)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer6_END (14)
#define SOC_SCTRL_SCPEREN1_gt_clk_timer6_START (15)
#define SOC_SCTRL_SCPEREN1_gt_clk_timer6_END (15)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer7_START (16)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer7_END (16)
#define SOC_SCTRL_SCPEREN1_gt_clk_timer7_START (17)
#define SOC_SCTRL_SCPEREN1_gt_clk_timer7_END (17)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer8_START (18)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer8_END (18)
#define SOC_SCTRL_SCPEREN1_gt_clk_timer8_START (19)
#define SOC_SCTRL_SCPEREN1_gt_clk_timer8_END (19)
#define SOC_SCTRL_SCPEREN1_gt_timerclk_refh_START (20)
#define SOC_SCTRL_SCPEREN1_gt_timerclk_refh_END (20)
#define SOC_SCTRL_SCPEREN1_ddr_reset_en_START (21)
#define SOC_SCTRL_SCPEREN1_ddr_reset_en_END (21)
#define SOC_SCTRL_SCPEREN1_gt_clk_hifd_fll_START (22)
#define SOC_SCTRL_SCPEREN1_gt_clk_hifd_fll_END (22)
#define SOC_SCTRL_SCPEREN1_gt_clk_hifd_pll_START (23)
#define SOC_SCTRL_SCPEREN1_gt_clk_hifd_pll_END (23)
#define SOC_SCTRL_SCPEREN1_gt_clk_noc_aobus2aspbus_START (24)
#define SOC_SCTRL_SCPEREN1_gt_clk_noc_aobus2aspbus_END (24)
#define SOC_SCTRL_SCPEREN1_gt_clk_noc_aobus2iomcubus_START (25)
#define SOC_SCTRL_SCPEREN1_gt_clk_noc_aobus2iomcubus_END (25)
#define SOC_SCTRL_SCPEREN1_gt_clk_hsdt_subsys_START (26)
#define SOC_SCTRL_SCPEREN1_gt_clk_hsdt_subsys_END (26)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_codec_acpu_START (27)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_codec_acpu_END (27)
#define SOC_SCTRL_SCPEREN1_gt_clk_autodiv_aonoc_pll_START (28)
#define SOC_SCTRL_SCPEREN1_gt_clk_autodiv_aonoc_pll_END (28)
#define SOC_SCTRL_SCPEREN1_gt_clk_autodiv_aonoc_fll_START (29)
#define SOC_SCTRL_SCPEREN1_gt_clk_autodiv_aonoc_fll_END (29)
#define SOC_SCTRL_SCPEREN1_wait_ddr_selfreflash_done_bypass_START (31)
#define SOC_SCTRL_SCPEREN1_wait_ddr_selfreflash_done_bypass_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_asp_dw_axi : 1;
        unsigned int gt_clk_asp_x2h : 1;
        unsigned int gt_clk_asp_h2p : 1;
        unsigned int gt_asp_cfg : 1;
        unsigned int gt_clk_asp_subsys_acpu : 1;
        unsigned int gt_clk_noc_aobus2sysbusddrc : 1;
        unsigned int gt_clk_ao_camera : 1;
        unsigned int ddr_loopback_en : 1;
        unsigned int gt_clk_out1 : 1;
        unsigned int gt_clk_fll_src_tp : 1;
        unsigned int gt_pclk_timer4 : 1;
        unsigned int gt_clk_timer4 : 1;
        unsigned int gt_pclk_timer5 : 1;
        unsigned int gt_clk_timer5 : 1;
        unsigned int gt_pclk_timer6 : 1;
        unsigned int gt_clk_timer6 : 1;
        unsigned int gt_pclk_timer7 : 1;
        unsigned int gt_clk_timer7 : 1;
        unsigned int gt_pclk_timer8 : 1;
        unsigned int gt_clk_timer8 : 1;
        unsigned int gt_timerclk_refh : 1;
        unsigned int ddr_reset_en : 1;
        unsigned int gt_clk_hifd_fll : 1;
        unsigned int gt_clk_hifd_pll : 1;
        unsigned int gt_clk_noc_aobus2aspbus : 1;
        unsigned int gt_clk_noc_aobus2iomcubus : 1;
        unsigned int gt_clk_hsdt_subsys : 1;
        unsigned int gt_clk_asp_codec_acpu : 1;
        unsigned int gt_clk_autodiv_aonoc_pll : 1;
        unsigned int gt_clk_autodiv_aonoc_fll : 1;
        unsigned int reserved : 1;
        unsigned int wait_ddr_selfreflash_done_bypass : 1;
    } reg;
} SOC_SCTRL_SCPERDIS1_UNION;
#endif
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_dw_axi_START (0)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_dw_axi_END (0)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_x2h_START (1)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_x2h_END (1)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_h2p_START (2)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_h2p_END (2)
#define SOC_SCTRL_SCPERDIS1_gt_asp_cfg_START (3)
#define SOC_SCTRL_SCPERDIS1_gt_asp_cfg_END (3)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_subsys_acpu_START (4)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_subsys_acpu_END (4)
#define SOC_SCTRL_SCPERDIS1_gt_clk_noc_aobus2sysbusddrc_START (5)
#define SOC_SCTRL_SCPERDIS1_gt_clk_noc_aobus2sysbusddrc_END (5)
#define SOC_SCTRL_SCPERDIS1_gt_clk_ao_camera_START (6)
#define SOC_SCTRL_SCPERDIS1_gt_clk_ao_camera_END (6)
#define SOC_SCTRL_SCPERDIS1_ddr_loopback_en_START (7)
#define SOC_SCTRL_SCPERDIS1_ddr_loopback_en_END (7)
#define SOC_SCTRL_SCPERDIS1_gt_clk_out1_START (8)
#define SOC_SCTRL_SCPERDIS1_gt_clk_out1_END (8)
#define SOC_SCTRL_SCPERDIS1_gt_clk_fll_src_tp_START (9)
#define SOC_SCTRL_SCPERDIS1_gt_clk_fll_src_tp_END (9)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer4_START (10)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer4_END (10)
#define SOC_SCTRL_SCPERDIS1_gt_clk_timer4_START (11)
#define SOC_SCTRL_SCPERDIS1_gt_clk_timer4_END (11)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer5_START (12)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer5_END (12)
#define SOC_SCTRL_SCPERDIS1_gt_clk_timer5_START (13)
#define SOC_SCTRL_SCPERDIS1_gt_clk_timer5_END (13)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer6_START (14)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer6_END (14)
#define SOC_SCTRL_SCPERDIS1_gt_clk_timer6_START (15)
#define SOC_SCTRL_SCPERDIS1_gt_clk_timer6_END (15)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer7_START (16)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer7_END (16)
#define SOC_SCTRL_SCPERDIS1_gt_clk_timer7_START (17)
#define SOC_SCTRL_SCPERDIS1_gt_clk_timer7_END (17)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer8_START (18)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer8_END (18)
#define SOC_SCTRL_SCPERDIS1_gt_clk_timer8_START (19)
#define SOC_SCTRL_SCPERDIS1_gt_clk_timer8_END (19)
#define SOC_SCTRL_SCPERDIS1_gt_timerclk_refh_START (20)
#define SOC_SCTRL_SCPERDIS1_gt_timerclk_refh_END (20)
#define SOC_SCTRL_SCPERDIS1_ddr_reset_en_START (21)
#define SOC_SCTRL_SCPERDIS1_ddr_reset_en_END (21)
#define SOC_SCTRL_SCPERDIS1_gt_clk_hifd_fll_START (22)
#define SOC_SCTRL_SCPERDIS1_gt_clk_hifd_fll_END (22)
#define SOC_SCTRL_SCPERDIS1_gt_clk_hifd_pll_START (23)
#define SOC_SCTRL_SCPERDIS1_gt_clk_hifd_pll_END (23)
#define SOC_SCTRL_SCPERDIS1_gt_clk_noc_aobus2aspbus_START (24)
#define SOC_SCTRL_SCPERDIS1_gt_clk_noc_aobus2aspbus_END (24)
#define SOC_SCTRL_SCPERDIS1_gt_clk_noc_aobus2iomcubus_START (25)
#define SOC_SCTRL_SCPERDIS1_gt_clk_noc_aobus2iomcubus_END (25)
#define SOC_SCTRL_SCPERDIS1_gt_clk_hsdt_subsys_START (26)
#define SOC_SCTRL_SCPERDIS1_gt_clk_hsdt_subsys_END (26)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_codec_acpu_START (27)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_codec_acpu_END (27)
#define SOC_SCTRL_SCPERDIS1_gt_clk_autodiv_aonoc_pll_START (28)
#define SOC_SCTRL_SCPERDIS1_gt_clk_autodiv_aonoc_pll_END (28)
#define SOC_SCTRL_SCPERDIS1_gt_clk_autodiv_aonoc_fll_START (29)
#define SOC_SCTRL_SCPERDIS1_gt_clk_autodiv_aonoc_fll_END (29)
#define SOC_SCTRL_SCPERDIS1_wait_ddr_selfreflash_done_bypass_START (31)
#define SOC_SCTRL_SCPERDIS1_wait_ddr_selfreflash_done_bypass_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_asp_dw_axi : 1;
        unsigned int gt_clk_asp_x2h : 1;
        unsigned int gt_clk_asp_h2p : 1;
        unsigned int gt_asp_cfg : 1;
        unsigned int gt_clk_asp_subsys_acpu : 1;
        unsigned int gt_clk_noc_aobus2sysbusddrc : 1;
        unsigned int gt_clk_ao_camera : 1;
        unsigned int ddr_loopback_en : 1;
        unsigned int gt_clk_out1 : 1;
        unsigned int gt_clk_fll_src_tp : 1;
        unsigned int gt_pclk_timer4 : 1;
        unsigned int gt_clk_timer4 : 1;
        unsigned int gt_pclk_timer5 : 1;
        unsigned int gt_clk_timer5 : 1;
        unsigned int gt_pclk_timer6 : 1;
        unsigned int gt_clk_timer6 : 1;
        unsigned int gt_pclk_timer7 : 1;
        unsigned int gt_clk_timer7 : 1;
        unsigned int gt_pclk_timer8 : 1;
        unsigned int gt_clk_timer8 : 1;
        unsigned int gt_timerclk_refh : 1;
        unsigned int ddr_reset_en : 1;
        unsigned int gt_clk_hifd_fll : 1;
        unsigned int gt_clk_hifd_pll : 1;
        unsigned int gt_clk_noc_aobus2aspbus : 1;
        unsigned int gt_clk_noc_aobus2iomcubus : 1;
        unsigned int gt_clk_hsdt_subsys : 1;
        unsigned int gt_clk_asp_codec_acpu : 1;
        unsigned int gt_clk_autodiv_aonoc_pll : 1;
        unsigned int gt_clk_autodiv_aonoc_fll : 1;
        unsigned int reserved : 1;
        unsigned int wait_ddr_selfreflash_done_bypass : 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN1_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_dw_axi_START (0)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_dw_axi_END (0)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_x2h_START (1)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_x2h_END (1)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_h2p_START (2)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_h2p_END (2)
#define SOC_SCTRL_SCPERCLKEN1_gt_asp_cfg_START (3)
#define SOC_SCTRL_SCPERCLKEN1_gt_asp_cfg_END (3)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_subsys_acpu_START (4)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_subsys_acpu_END (4)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_noc_aobus2sysbusddrc_START (5)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_noc_aobus2sysbusddrc_END (5)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_ao_camera_START (6)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_ao_camera_END (6)
#define SOC_SCTRL_SCPERCLKEN1_ddr_loopback_en_START (7)
#define SOC_SCTRL_SCPERCLKEN1_ddr_loopback_en_END (7)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_out1_START (8)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_out1_END (8)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_fll_src_tp_START (9)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_fll_src_tp_END (9)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer4_START (10)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer4_END (10)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_timer4_START (11)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_timer4_END (11)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer5_START (12)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer5_END (12)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_timer5_START (13)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_timer5_END (13)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer6_START (14)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer6_END (14)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_timer6_START (15)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_timer6_END (15)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer7_START (16)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer7_END (16)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_timer7_START (17)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_timer7_END (17)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer8_START (18)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer8_END (18)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_timer8_START (19)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_timer8_END (19)
#define SOC_SCTRL_SCPERCLKEN1_gt_timerclk_refh_START (20)
#define SOC_SCTRL_SCPERCLKEN1_gt_timerclk_refh_END (20)
#define SOC_SCTRL_SCPERCLKEN1_ddr_reset_en_START (21)
#define SOC_SCTRL_SCPERCLKEN1_ddr_reset_en_END (21)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_hifd_fll_START (22)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_hifd_fll_END (22)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_hifd_pll_START (23)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_hifd_pll_END (23)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_noc_aobus2aspbus_START (24)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_noc_aobus2aspbus_END (24)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_noc_aobus2iomcubus_START (25)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_noc_aobus2iomcubus_END (25)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_hsdt_subsys_START (26)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_hsdt_subsys_END (26)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_codec_acpu_START (27)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_codec_acpu_END (27)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_autodiv_aonoc_pll_START (28)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_autodiv_aonoc_pll_END (28)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_autodiv_aonoc_fll_START (29)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_autodiv_aonoc_fll_END (29)
#define SOC_SCTRL_SCPERCLKEN1_wait_ddr_selfreflash_done_bypass_START (31)
#define SOC_SCTRL_SCPERCLKEN1_wait_ddr_selfreflash_done_bypass_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int st_clk_asp_dw_axi : 1;
        unsigned int st_clk_asp_x2h : 1;
        unsigned int st_clk_asp_h2p : 1;
        unsigned int st_clk_asp_cfg : 1;
        unsigned int st_clk_mad_axi : 1;
        unsigned int st_clk_noc_aobus2sysbusddrc : 1;
        unsigned int st_clk_ao_camera : 1;
        unsigned int st_pclk_ao_gpio6 : 1;
        unsigned int st_clk_mad_32k : 1;
        unsigned int st_clk_fll_src_tp : 1;
        unsigned int st_pclk_timer4 : 1;
        unsigned int st_clk_timer4 : 1;
        unsigned int st_pclk_timer5 : 1;
        unsigned int st_clk_timer5 : 1;
        unsigned int st_pclk_timer6 : 1;
        unsigned int st_clk_timer6 : 1;
        unsigned int st_pclk_timer7 : 1;
        unsigned int st_clk_timer7 : 1;
        unsigned int st_pclk_timer8 : 1;
        unsigned int st_clk_timer8 : 1;
        unsigned int st_timerclk_refh : 1;
        unsigned int st_clk_monitor_tcxo : 1;
        unsigned int swdone_clk_iomcu_fll_src_div : 1;
        unsigned int swdone_clk_aobus_div : 1;
        unsigned int st_clk_noc_aobus2aspbus : 1;
        unsigned int st_clk_noc_aobus2iomcubus : 1;
        unsigned int st_clk_hsdt_subsys : 1;
        unsigned int swdone_clk_asp_subsys_div : 1;
        unsigned int swdone_clk_sci_div : 1;
        unsigned int sw_ack_clk_aobus_sw : 3;
    } reg;
} SOC_SCTRL_SCPERSTAT1_UNION;
#endif
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_dw_axi_START (0)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_dw_axi_END (0)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_x2h_START (1)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_x2h_END (1)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_h2p_START (2)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_h2p_END (2)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_cfg_START (3)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_cfg_END (3)
#define SOC_SCTRL_SCPERSTAT1_st_clk_mad_axi_START (4)
#define SOC_SCTRL_SCPERSTAT1_st_clk_mad_axi_END (4)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2sysbusddrc_START (5)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2sysbusddrc_END (5)
#define SOC_SCTRL_SCPERSTAT1_st_clk_ao_camera_START (6)
#define SOC_SCTRL_SCPERSTAT1_st_clk_ao_camera_END (6)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_ao_gpio6_START (7)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_ao_gpio6_END (7)
#define SOC_SCTRL_SCPERSTAT1_st_clk_mad_32k_START (8)
#define SOC_SCTRL_SCPERSTAT1_st_clk_mad_32k_END (8)
#define SOC_SCTRL_SCPERSTAT1_st_clk_fll_src_tp_START (9)
#define SOC_SCTRL_SCPERSTAT1_st_clk_fll_src_tp_END (9)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer4_START (10)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer4_END (10)
#define SOC_SCTRL_SCPERSTAT1_st_clk_timer4_START (11)
#define SOC_SCTRL_SCPERSTAT1_st_clk_timer4_END (11)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer5_START (12)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer5_END (12)
#define SOC_SCTRL_SCPERSTAT1_st_clk_timer5_START (13)
#define SOC_SCTRL_SCPERSTAT1_st_clk_timer5_END (13)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer6_START (14)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer6_END (14)
#define SOC_SCTRL_SCPERSTAT1_st_clk_timer6_START (15)
#define SOC_SCTRL_SCPERSTAT1_st_clk_timer6_END (15)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer7_START (16)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer7_END (16)
#define SOC_SCTRL_SCPERSTAT1_st_clk_timer7_START (17)
#define SOC_SCTRL_SCPERSTAT1_st_clk_timer7_END (17)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer8_START (18)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer8_END (18)
#define SOC_SCTRL_SCPERSTAT1_st_clk_timer8_START (19)
#define SOC_SCTRL_SCPERSTAT1_st_clk_timer8_END (19)
#define SOC_SCTRL_SCPERSTAT1_st_timerclk_refh_START (20)
#define SOC_SCTRL_SCPERSTAT1_st_timerclk_refh_END (20)
#define SOC_SCTRL_SCPERSTAT1_st_clk_monitor_tcxo_START (21)
#define SOC_SCTRL_SCPERSTAT1_st_clk_monitor_tcxo_END (21)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_iomcu_fll_src_div_START (22)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_iomcu_fll_src_div_END (22)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_aobus_div_START (23)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_aobus_div_END (23)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2aspbus_START (24)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2aspbus_END (24)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2iomcubus_START (25)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2iomcubus_END (25)
#define SOC_SCTRL_SCPERSTAT1_st_clk_hsdt_subsys_START (26)
#define SOC_SCTRL_SCPERSTAT1_st_clk_hsdt_subsys_END (26)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_asp_subsys_div_START (27)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_asp_subsys_div_END (27)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_sci_div_START (28)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_sci_div_END (28)
#define SOC_SCTRL_SCPERSTAT1_sw_ack_clk_aobus_sw_START (29)
#define SOC_SCTRL_SCPERSTAT1_sw_ack_clk_aobus_sw_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_aonoc2fdbus : 1;
        unsigned int gt_clk_hifd_32k : 1;
        unsigned int gt_clk_hifd_tcxo : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int spll_en_cbbe16 : 1;
        unsigned int spll_en_bbe16 : 1;
        unsigned int spll_gt_cbbe16 : 1;
        unsigned int spll_gt_bbe16 : 1;
        unsigned int fll_gt_soft : 1;
        unsigned int spll_gt_cpu : 1;
        unsigned int spll_gt_modem : 1;
        unsigned int fll_bp_cpu : 1;
        unsigned int fll_bp_mdm : 1;
        unsigned int fll_gt_cpu : 1;
        unsigned int fll_gt_mdm : 1;
        unsigned int gt_pclk_ao_wd : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int gt_pclkdbg_to_iomcu : 1;
        unsigned int reserved_9 : 1;
        unsigned int spll_en_cpu : 1;
        unsigned int spll_en_mdm : 1;
        unsigned int fll_en_cpu : 1;
        unsigned int fll_en_mdm : 1;
        unsigned int reserved_10 : 1;
        unsigned int gt_clk_mad_axi : 1;
    } reg;
} SOC_SCTRL_SCPEREN2_UNION;
#endif
#define SOC_SCTRL_SCPEREN2_gt_clk_aonoc2fdbus_START (0)
#define SOC_SCTRL_SCPEREN2_gt_clk_aonoc2fdbus_END (0)
#define SOC_SCTRL_SCPEREN2_gt_clk_hifd_32k_START (1)
#define SOC_SCTRL_SCPEREN2_gt_clk_hifd_32k_END (1)
#define SOC_SCTRL_SCPEREN2_gt_clk_hifd_tcxo_START (2)
#define SOC_SCTRL_SCPEREN2_gt_clk_hifd_tcxo_END (2)
#define SOC_SCTRL_SCPEREN2_spll_en_cbbe16_START (10)
#define SOC_SCTRL_SCPEREN2_spll_en_cbbe16_END (10)
#define SOC_SCTRL_SCPEREN2_spll_en_bbe16_START (11)
#define SOC_SCTRL_SCPEREN2_spll_en_bbe16_END (11)
#define SOC_SCTRL_SCPEREN2_spll_gt_cbbe16_START (12)
#define SOC_SCTRL_SCPEREN2_spll_gt_cbbe16_END (12)
#define SOC_SCTRL_SCPEREN2_spll_gt_bbe16_START (13)
#define SOC_SCTRL_SCPEREN2_spll_gt_bbe16_END (13)
#define SOC_SCTRL_SCPEREN2_fll_gt_soft_START (14)
#define SOC_SCTRL_SCPEREN2_fll_gt_soft_END (14)
#define SOC_SCTRL_SCPEREN2_spll_gt_cpu_START (15)
#define SOC_SCTRL_SCPEREN2_spll_gt_cpu_END (15)
#define SOC_SCTRL_SCPEREN2_spll_gt_modem_START (16)
#define SOC_SCTRL_SCPEREN2_spll_gt_modem_END (16)
#define SOC_SCTRL_SCPEREN2_fll_bp_cpu_START (17)
#define SOC_SCTRL_SCPEREN2_fll_bp_cpu_END (17)
#define SOC_SCTRL_SCPEREN2_fll_bp_mdm_START (18)
#define SOC_SCTRL_SCPEREN2_fll_bp_mdm_END (18)
#define SOC_SCTRL_SCPEREN2_fll_gt_cpu_START (19)
#define SOC_SCTRL_SCPEREN2_fll_gt_cpu_END (19)
#define SOC_SCTRL_SCPEREN2_fll_gt_mdm_START (20)
#define SOC_SCTRL_SCPEREN2_fll_gt_mdm_END (20)
#define SOC_SCTRL_SCPEREN2_gt_pclk_ao_wd_START (21)
#define SOC_SCTRL_SCPEREN2_gt_pclk_ao_wd_END (21)
#define SOC_SCTRL_SCPEREN2_gt_pclkdbg_to_iomcu_START (24)
#define SOC_SCTRL_SCPEREN2_gt_pclkdbg_to_iomcu_END (24)
#define SOC_SCTRL_SCPEREN2_spll_en_cpu_START (26)
#define SOC_SCTRL_SCPEREN2_spll_en_cpu_END (26)
#define SOC_SCTRL_SCPEREN2_spll_en_mdm_START (27)
#define SOC_SCTRL_SCPEREN2_spll_en_mdm_END (27)
#define SOC_SCTRL_SCPEREN2_fll_en_cpu_START (28)
#define SOC_SCTRL_SCPEREN2_fll_en_cpu_END (28)
#define SOC_SCTRL_SCPEREN2_fll_en_mdm_START (29)
#define SOC_SCTRL_SCPEREN2_fll_en_mdm_END (29)
#define SOC_SCTRL_SCPEREN2_gt_clk_mad_axi_START (31)
#define SOC_SCTRL_SCPEREN2_gt_clk_mad_axi_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_aonoc2fdbus : 1;
        unsigned int gt_clk_hifd_32k : 1;
        unsigned int gt_clk_hifd_tcxo : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int spll_en_cbbe16 : 1;
        unsigned int spll_en_bbe16 : 1;
        unsigned int spll_gt_cbbe16 : 1;
        unsigned int spll_gt_bbe16 : 1;
        unsigned int fll_gt_soft : 1;
        unsigned int spll_gt_cpu : 1;
        unsigned int spll_gt_modem : 1;
        unsigned int fll_bp_cpu : 1;
        unsigned int fll_bp_mdm : 1;
        unsigned int fll_gt_cpu : 1;
        unsigned int fll_gt_mdm : 1;
        unsigned int gt_pclk_ao_wd : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int gt_pclkdbg_to_iomcu : 1;
        unsigned int reserved_9 : 1;
        unsigned int spll_en_cpu : 1;
        unsigned int spll_en_mdm : 1;
        unsigned int fll_en_cpu : 1;
        unsigned int fll_en_mdm : 1;
        unsigned int reserved_10 : 1;
        unsigned int gt_clk_mad_axi : 1;
    } reg;
} SOC_SCTRL_SCPERDIS2_UNION;
#endif
#define SOC_SCTRL_SCPERDIS2_gt_clk_aonoc2fdbus_START (0)
#define SOC_SCTRL_SCPERDIS2_gt_clk_aonoc2fdbus_END (0)
#define SOC_SCTRL_SCPERDIS2_gt_clk_hifd_32k_START (1)
#define SOC_SCTRL_SCPERDIS2_gt_clk_hifd_32k_END (1)
#define SOC_SCTRL_SCPERDIS2_gt_clk_hifd_tcxo_START (2)
#define SOC_SCTRL_SCPERDIS2_gt_clk_hifd_tcxo_END (2)
#define SOC_SCTRL_SCPERDIS2_spll_en_cbbe16_START (10)
#define SOC_SCTRL_SCPERDIS2_spll_en_cbbe16_END (10)
#define SOC_SCTRL_SCPERDIS2_spll_en_bbe16_START (11)
#define SOC_SCTRL_SCPERDIS2_spll_en_bbe16_END (11)
#define SOC_SCTRL_SCPERDIS2_spll_gt_cbbe16_START (12)
#define SOC_SCTRL_SCPERDIS2_spll_gt_cbbe16_END (12)
#define SOC_SCTRL_SCPERDIS2_spll_gt_bbe16_START (13)
#define SOC_SCTRL_SCPERDIS2_spll_gt_bbe16_END (13)
#define SOC_SCTRL_SCPERDIS2_fll_gt_soft_START (14)
#define SOC_SCTRL_SCPERDIS2_fll_gt_soft_END (14)
#define SOC_SCTRL_SCPERDIS2_spll_gt_cpu_START (15)
#define SOC_SCTRL_SCPERDIS2_spll_gt_cpu_END (15)
#define SOC_SCTRL_SCPERDIS2_spll_gt_modem_START (16)
#define SOC_SCTRL_SCPERDIS2_spll_gt_modem_END (16)
#define SOC_SCTRL_SCPERDIS2_fll_bp_cpu_START (17)
#define SOC_SCTRL_SCPERDIS2_fll_bp_cpu_END (17)
#define SOC_SCTRL_SCPERDIS2_fll_bp_mdm_START (18)
#define SOC_SCTRL_SCPERDIS2_fll_bp_mdm_END (18)
#define SOC_SCTRL_SCPERDIS2_fll_gt_cpu_START (19)
#define SOC_SCTRL_SCPERDIS2_fll_gt_cpu_END (19)
#define SOC_SCTRL_SCPERDIS2_fll_gt_mdm_START (20)
#define SOC_SCTRL_SCPERDIS2_fll_gt_mdm_END (20)
#define SOC_SCTRL_SCPERDIS2_gt_pclk_ao_wd_START (21)
#define SOC_SCTRL_SCPERDIS2_gt_pclk_ao_wd_END (21)
#define SOC_SCTRL_SCPERDIS2_gt_pclkdbg_to_iomcu_START (24)
#define SOC_SCTRL_SCPERDIS2_gt_pclkdbg_to_iomcu_END (24)
#define SOC_SCTRL_SCPERDIS2_spll_en_cpu_START (26)
#define SOC_SCTRL_SCPERDIS2_spll_en_cpu_END (26)
#define SOC_SCTRL_SCPERDIS2_spll_en_mdm_START (27)
#define SOC_SCTRL_SCPERDIS2_spll_en_mdm_END (27)
#define SOC_SCTRL_SCPERDIS2_fll_en_cpu_START (28)
#define SOC_SCTRL_SCPERDIS2_fll_en_cpu_END (28)
#define SOC_SCTRL_SCPERDIS2_fll_en_mdm_START (29)
#define SOC_SCTRL_SCPERDIS2_fll_en_mdm_END (29)
#define SOC_SCTRL_SCPERDIS2_gt_clk_mad_axi_START (31)
#define SOC_SCTRL_SCPERDIS2_gt_clk_mad_axi_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_aonoc2fdbus : 1;
        unsigned int gt_clk_hifd_32k : 1;
        unsigned int gt_clk_hifd_tcxo : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int spll_en_cbbe16 : 1;
        unsigned int spll_en_bbe16 : 1;
        unsigned int spll_gt_cbbe16 : 1;
        unsigned int spll_gt_bbe16 : 1;
        unsigned int fll_gt_soft : 1;
        unsigned int spll_gt_cpu : 1;
        unsigned int spll_gt_modem : 1;
        unsigned int fll_bp_cpu : 1;
        unsigned int fll_bp_mdm : 1;
        unsigned int fll_gt_cpu : 1;
        unsigned int fll_gt_mdm : 1;
        unsigned int gt_pclk_ao_wd : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int gt_pclkdbg_to_iomcu : 1;
        unsigned int reserved_9 : 1;
        unsigned int spll_en_cpu : 1;
        unsigned int spll_en_mdm : 1;
        unsigned int fll_en_cpu : 1;
        unsigned int fll_en_mdm : 1;
        unsigned int reserved_10 : 1;
        unsigned int gt_clk_mad_axi : 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN2_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN2_gt_clk_aonoc2fdbus_START (0)
#define SOC_SCTRL_SCPERCLKEN2_gt_clk_aonoc2fdbus_END (0)
#define SOC_SCTRL_SCPERCLKEN2_gt_clk_hifd_32k_START (1)
#define SOC_SCTRL_SCPERCLKEN2_gt_clk_hifd_32k_END (1)
#define SOC_SCTRL_SCPERCLKEN2_gt_clk_hifd_tcxo_START (2)
#define SOC_SCTRL_SCPERCLKEN2_gt_clk_hifd_tcxo_END (2)
#define SOC_SCTRL_SCPERCLKEN2_spll_en_cbbe16_START (10)
#define SOC_SCTRL_SCPERCLKEN2_spll_en_cbbe16_END (10)
#define SOC_SCTRL_SCPERCLKEN2_spll_en_bbe16_START (11)
#define SOC_SCTRL_SCPERCLKEN2_spll_en_bbe16_END (11)
#define SOC_SCTRL_SCPERCLKEN2_spll_gt_cbbe16_START (12)
#define SOC_SCTRL_SCPERCLKEN2_spll_gt_cbbe16_END (12)
#define SOC_SCTRL_SCPERCLKEN2_spll_gt_bbe16_START (13)
#define SOC_SCTRL_SCPERCLKEN2_spll_gt_bbe16_END (13)
#define SOC_SCTRL_SCPERCLKEN2_fll_gt_soft_START (14)
#define SOC_SCTRL_SCPERCLKEN2_fll_gt_soft_END (14)
#define SOC_SCTRL_SCPERCLKEN2_spll_gt_cpu_START (15)
#define SOC_SCTRL_SCPERCLKEN2_spll_gt_cpu_END (15)
#define SOC_SCTRL_SCPERCLKEN2_spll_gt_modem_START (16)
#define SOC_SCTRL_SCPERCLKEN2_spll_gt_modem_END (16)
#define SOC_SCTRL_SCPERCLKEN2_fll_bp_cpu_START (17)
#define SOC_SCTRL_SCPERCLKEN2_fll_bp_cpu_END (17)
#define SOC_SCTRL_SCPERCLKEN2_fll_bp_mdm_START (18)
#define SOC_SCTRL_SCPERCLKEN2_fll_bp_mdm_END (18)
#define SOC_SCTRL_SCPERCLKEN2_fll_gt_cpu_START (19)
#define SOC_SCTRL_SCPERCLKEN2_fll_gt_cpu_END (19)
#define SOC_SCTRL_SCPERCLKEN2_fll_gt_mdm_START (20)
#define SOC_SCTRL_SCPERCLKEN2_fll_gt_mdm_END (20)
#define SOC_SCTRL_SCPERCLKEN2_gt_pclk_ao_wd_START (21)
#define SOC_SCTRL_SCPERCLKEN2_gt_pclk_ao_wd_END (21)
#define SOC_SCTRL_SCPERCLKEN2_gt_pclkdbg_to_iomcu_START (24)
#define SOC_SCTRL_SCPERCLKEN2_gt_pclkdbg_to_iomcu_END (24)
#define SOC_SCTRL_SCPERCLKEN2_spll_en_cpu_START (26)
#define SOC_SCTRL_SCPERCLKEN2_spll_en_cpu_END (26)
#define SOC_SCTRL_SCPERCLKEN2_spll_en_mdm_START (27)
#define SOC_SCTRL_SCPERCLKEN2_spll_en_mdm_END (27)
#define SOC_SCTRL_SCPERCLKEN2_fll_en_cpu_START (28)
#define SOC_SCTRL_SCPERCLKEN2_fll_en_cpu_END (28)
#define SOC_SCTRL_SCPERCLKEN2_fll_en_mdm_START (29)
#define SOC_SCTRL_SCPERCLKEN2_fll_en_mdm_END (29)
#define SOC_SCTRL_SCPERCLKEN2_gt_clk_mad_axi_START (31)
#define SOC_SCTRL_SCPERCLKEN2_gt_clk_mad_axi_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int st_clk_hifd_32k : 1;
        unsigned int st_clk_hifd_tcxo : 1;
        unsigned int swdone_clk_aonoc_pll_div : 1;
        unsigned int swdone_clk_iomcu_pll_div : 1;
        unsigned int sw_ack_clk_aonoc_sw : 4;
        unsigned int swdone_clk_hifd_fll_div : 1;
        unsigned int swdone_clk_hifd_pll_div : 1;
        unsigned int sw_ack_clk_asp_subsys_ini : 4;
        unsigned int reset_dram_n : 1;
        unsigned int st_clk_hifd_fll : 1;
        unsigned int st_clk_hifd_pll : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int st_clk_iomcu_fll_src : 1;
        unsigned int st_pclk_ao_wd : 1;
        unsigned int st_clk_iomcu_pll_div : 1;
        unsigned int swdone_clk_csi_cfg_div : 1;
        unsigned int swdone_clk_mad_fll_div : 1;
        unsigned int st_clk_fll_test_src : 1;
        unsigned int st_pclkdbg_to_iomcu : 1;
        unsigned int st_clk_asp_codec : 1;
        unsigned int swdone_clk_out0_div : 1;
        unsigned int swdone_clk_asp_subsys_fll_div : 1;
        unsigned int swdone_clk_aonoc_fll_div : 1;
        unsigned int swdone_clk_noc_timeout_extref_div : 1;
    } reg;
} SOC_SCTRL_SCPERSTAT2_UNION;
#endif
#define SOC_SCTRL_SCPERSTAT2_st_clk_hifd_32k_START (0)
#define SOC_SCTRL_SCPERSTAT2_st_clk_hifd_32k_END (0)
#define SOC_SCTRL_SCPERSTAT2_st_clk_hifd_tcxo_START (1)
#define SOC_SCTRL_SCPERSTAT2_st_clk_hifd_tcxo_END (1)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_aonoc_pll_div_START (2)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_aonoc_pll_div_END (2)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_iomcu_pll_div_START (3)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_iomcu_pll_div_END (3)
#define SOC_SCTRL_SCPERSTAT2_sw_ack_clk_aonoc_sw_START (4)
#define SOC_SCTRL_SCPERSTAT2_sw_ack_clk_aonoc_sw_END (7)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_hifd_fll_div_START (8)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_hifd_fll_div_END (8)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_hifd_pll_div_START (9)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_hifd_pll_div_END (9)
#define SOC_SCTRL_SCPERSTAT2_sw_ack_clk_asp_subsys_ini_START (10)
#define SOC_SCTRL_SCPERSTAT2_sw_ack_clk_asp_subsys_ini_END (13)
#define SOC_SCTRL_SCPERSTAT2_reset_dram_n_START (14)
#define SOC_SCTRL_SCPERSTAT2_reset_dram_n_END (14)
#define SOC_SCTRL_SCPERSTAT2_st_clk_hifd_fll_START (15)
#define SOC_SCTRL_SCPERSTAT2_st_clk_hifd_fll_END (15)
#define SOC_SCTRL_SCPERSTAT2_st_clk_hifd_pll_START (16)
#define SOC_SCTRL_SCPERSTAT2_st_clk_hifd_pll_END (16)
#define SOC_SCTRL_SCPERSTAT2_st_clk_iomcu_fll_src_START (20)
#define SOC_SCTRL_SCPERSTAT2_st_clk_iomcu_fll_src_END (20)
#define SOC_SCTRL_SCPERSTAT2_st_pclk_ao_wd_START (21)
#define SOC_SCTRL_SCPERSTAT2_st_pclk_ao_wd_END (21)
#define SOC_SCTRL_SCPERSTAT2_st_clk_iomcu_pll_div_START (22)
#define SOC_SCTRL_SCPERSTAT2_st_clk_iomcu_pll_div_END (22)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_csi_cfg_div_START (23)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_csi_cfg_div_END (23)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_mad_fll_div_START (24)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_mad_fll_div_END (24)
#define SOC_SCTRL_SCPERSTAT2_st_clk_fll_test_src_START (25)
#define SOC_SCTRL_SCPERSTAT2_st_clk_fll_test_src_END (25)
#define SOC_SCTRL_SCPERSTAT2_st_pclkdbg_to_iomcu_START (26)
#define SOC_SCTRL_SCPERSTAT2_st_pclkdbg_to_iomcu_END (26)
#define SOC_SCTRL_SCPERSTAT2_st_clk_asp_codec_START (27)
#define SOC_SCTRL_SCPERSTAT2_st_clk_asp_codec_END (27)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_out0_div_START (28)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_out0_div_END (28)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_asp_subsys_fll_div_START (29)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_asp_subsys_fll_div_END (29)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_aonoc_fll_div_START (30)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_aonoc_fll_div_END (30)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_noc_timeout_extref_div_START (31)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_noc_timeout_extref_div_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_aobus_idle_flag : 1;
        unsigned int st_pclk_ao_gpio1_se : 1;
        unsigned int gt_aobus_noc_ini : 1;
        unsigned int autodiv_aonoc_pll_stat : 1;
        unsigned int autodiv_aonoc_fll_stat : 1;
        unsigned int st_pclk_timer17 : 1;
        unsigned int st_clk_timer17 : 1;
        unsigned int st_pclk_timer16 : 1;
        unsigned int st_clk_timer16 : 1;
        unsigned int st_pclk_timer15 : 1;
        unsigned int st_clk_timer15 : 1;
        unsigned int st_pclk_timer14 : 1;
        unsigned int st_clk_timer14 : 1;
        unsigned int st_pclk_timer13 : 1;
        unsigned int st_clk_timer13 : 1;
        unsigned int st_pclk_ao_ipc : 1;
        unsigned int swdone_clk_out1_pll_div : 1;
        unsigned int reserved : 1;
        unsigned int st_clk_spll_peri : 1;
        unsigned int st_clk_ao_tcp : 1;
        unsigned int st_clk_ao_tcp_32k : 1;
        unsigned int st_clk_ref_gpio_peri : 1;
        unsigned int st_clk_noc_timeout_extref_peri : 1;
        unsigned int st_timerclk_refh_peri : 1;
        unsigned int st_clkin_sys_peri : 1;
        unsigned int st_clkin_ref_peri : 1;
        unsigned int st_clk_sys_ini_peri : 1;
        unsigned int st_clk_out1 : 1;
        unsigned int st_clk_ref_peri : 1;
        unsigned int st_clk_sys_peri : 1;
        unsigned int st_clk_aobus_peri : 1;
        unsigned int st_clk_fll_src_peri : 1;
    } reg;
} SOC_SCTRL_SCPERSTAT3_UNION;
#endif
#define SOC_SCTRL_SCPERSTAT3_noc_aobus_idle_flag_START (0)
#define SOC_SCTRL_SCPERSTAT3_noc_aobus_idle_flag_END (0)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_ao_gpio1_se_START (1)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_ao_gpio1_se_END (1)
#define SOC_SCTRL_SCPERSTAT3_gt_aobus_noc_ini_START (2)
#define SOC_SCTRL_SCPERSTAT3_gt_aobus_noc_ini_END (2)
#define SOC_SCTRL_SCPERSTAT3_autodiv_aonoc_pll_stat_START (3)
#define SOC_SCTRL_SCPERSTAT3_autodiv_aonoc_pll_stat_END (3)
#define SOC_SCTRL_SCPERSTAT3_autodiv_aonoc_fll_stat_START (4)
#define SOC_SCTRL_SCPERSTAT3_autodiv_aonoc_fll_stat_END (4)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer17_START (5)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer17_END (5)
#define SOC_SCTRL_SCPERSTAT3_st_clk_timer17_START (6)
#define SOC_SCTRL_SCPERSTAT3_st_clk_timer17_END (6)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer16_START (7)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer16_END (7)
#define SOC_SCTRL_SCPERSTAT3_st_clk_timer16_START (8)
#define SOC_SCTRL_SCPERSTAT3_st_clk_timer16_END (8)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer15_START (9)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer15_END (9)
#define SOC_SCTRL_SCPERSTAT3_st_clk_timer15_START (10)
#define SOC_SCTRL_SCPERSTAT3_st_clk_timer15_END (10)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer14_START (11)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer14_END (11)
#define SOC_SCTRL_SCPERSTAT3_st_clk_timer14_START (12)
#define SOC_SCTRL_SCPERSTAT3_st_clk_timer14_END (12)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer13_START (13)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer13_END (13)
#define SOC_SCTRL_SCPERSTAT3_st_clk_timer13_START (14)
#define SOC_SCTRL_SCPERSTAT3_st_clk_timer13_END (14)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_ao_ipc_START (15)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_ao_ipc_END (15)
#define SOC_SCTRL_SCPERSTAT3_swdone_clk_out1_pll_div_START (16)
#define SOC_SCTRL_SCPERSTAT3_swdone_clk_out1_pll_div_END (16)
#define SOC_SCTRL_SCPERSTAT3_st_clk_spll_peri_START (18)
#define SOC_SCTRL_SCPERSTAT3_st_clk_spll_peri_END (18)
#define SOC_SCTRL_SCPERSTAT3_st_clk_ao_tcp_START (19)
#define SOC_SCTRL_SCPERSTAT3_st_clk_ao_tcp_END (19)
#define SOC_SCTRL_SCPERSTAT3_st_clk_ao_tcp_32k_START (20)
#define SOC_SCTRL_SCPERSTAT3_st_clk_ao_tcp_32k_END (20)
#define SOC_SCTRL_SCPERSTAT3_st_clk_ref_gpio_peri_START (21)
#define SOC_SCTRL_SCPERSTAT3_st_clk_ref_gpio_peri_END (21)
#define SOC_SCTRL_SCPERSTAT3_st_clk_noc_timeout_extref_peri_START (22)
#define SOC_SCTRL_SCPERSTAT3_st_clk_noc_timeout_extref_peri_END (22)
#define SOC_SCTRL_SCPERSTAT3_st_timerclk_refh_peri_START (23)
#define SOC_SCTRL_SCPERSTAT3_st_timerclk_refh_peri_END (23)
#define SOC_SCTRL_SCPERSTAT3_st_clkin_sys_peri_START (24)
#define SOC_SCTRL_SCPERSTAT3_st_clkin_sys_peri_END (24)
#define SOC_SCTRL_SCPERSTAT3_st_clkin_ref_peri_START (25)
#define SOC_SCTRL_SCPERSTAT3_st_clkin_ref_peri_END (25)
#define SOC_SCTRL_SCPERSTAT3_st_clk_sys_ini_peri_START (26)
#define SOC_SCTRL_SCPERSTAT3_st_clk_sys_ini_peri_END (26)
#define SOC_SCTRL_SCPERSTAT3_st_clk_out1_START (27)
#define SOC_SCTRL_SCPERSTAT3_st_clk_out1_END (27)
#define SOC_SCTRL_SCPERSTAT3_st_clk_ref_peri_START (28)
#define SOC_SCTRL_SCPERSTAT3_st_clk_ref_peri_END (28)
#define SOC_SCTRL_SCPERSTAT3_st_clk_sys_peri_START (29)
#define SOC_SCTRL_SCPERSTAT3_st_clk_sys_peri_END (29)
#define SOC_SCTRL_SCPERSTAT3_st_clk_aobus_peri_START (30)
#define SOC_SCTRL_SCPERSTAT3_st_clk_aobus_peri_END (30)
#define SOC_SCTRL_SCPERSTAT3_st_clk_fll_src_peri_START (31)
#define SOC_SCTRL_SCPERSTAT3_st_clk_fll_src_peri_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_aonoc_asp_mst_i_mainnowrpendingtrans_bp : 1;
        unsigned int sc_aonoc_asp_mst_i_mainnordpendingtrans_bp : 1;
        unsigned int noc_fd_mst_i_mainnowrpendingtrans_bp : 1;
        unsigned int noc_fd_mst_i_mainnordpendingtrans_bp : 1;
        unsigned int noc_ao_tcp_mst_i_mainnowrpendingtrans_bp : 1;
        unsigned int noc_ao_tcp_mst_i_mainnordpendingtrans_bp : 1;
        unsigned int tcpc_busy_bp : 1;
        unsigned int noc_bbpdrx_mst_i_mainnowrpendingtrans_bp : 1;
        unsigned int noc_bbpdrx_mst_i_mainnordpendingtrans_bp : 1;
        unsigned int sc_noc_ufs_mst_i_mainnowrpendingtrans_bp : 1;
        unsigned int sc_noc_ufs_mst_i_mainnordpendingtrans_bp : 1;
        unsigned int aonoc_ufshc_hibernate_ind_bp : 1;
        unsigned int reserved_0 : 7;
        unsigned int reserved_1 : 8;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN3_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN3_sc_aonoc_asp_mst_i_mainnowrpendingtrans_bp_START (0)
#define SOC_SCTRL_SCPERCLKEN3_sc_aonoc_asp_mst_i_mainnowrpendingtrans_bp_END (0)
#define SOC_SCTRL_SCPERCLKEN3_sc_aonoc_asp_mst_i_mainnordpendingtrans_bp_START (1)
#define SOC_SCTRL_SCPERCLKEN3_sc_aonoc_asp_mst_i_mainnordpendingtrans_bp_END (1)
#define SOC_SCTRL_SCPERCLKEN3_noc_fd_mst_i_mainnowrpendingtrans_bp_START (2)
#define SOC_SCTRL_SCPERCLKEN3_noc_fd_mst_i_mainnowrpendingtrans_bp_END (2)
#define SOC_SCTRL_SCPERCLKEN3_noc_fd_mst_i_mainnordpendingtrans_bp_START (3)
#define SOC_SCTRL_SCPERCLKEN3_noc_fd_mst_i_mainnordpendingtrans_bp_END (3)
#define SOC_SCTRL_SCPERCLKEN3_noc_ao_tcp_mst_i_mainnowrpendingtrans_bp_START (4)
#define SOC_SCTRL_SCPERCLKEN3_noc_ao_tcp_mst_i_mainnowrpendingtrans_bp_END (4)
#define SOC_SCTRL_SCPERCLKEN3_noc_ao_tcp_mst_i_mainnordpendingtrans_bp_START (5)
#define SOC_SCTRL_SCPERCLKEN3_noc_ao_tcp_mst_i_mainnordpendingtrans_bp_END (5)
#define SOC_SCTRL_SCPERCLKEN3_tcpc_busy_bp_START (6)
#define SOC_SCTRL_SCPERCLKEN3_tcpc_busy_bp_END (6)
#define SOC_SCTRL_SCPERCLKEN3_noc_bbpdrx_mst_i_mainnowrpendingtrans_bp_START (7)
#define SOC_SCTRL_SCPERCLKEN3_noc_bbpdrx_mst_i_mainnowrpendingtrans_bp_END (7)
#define SOC_SCTRL_SCPERCLKEN3_noc_bbpdrx_mst_i_mainnordpendingtrans_bp_START (8)
#define SOC_SCTRL_SCPERCLKEN3_noc_bbpdrx_mst_i_mainnordpendingtrans_bp_END (8)
#define SOC_SCTRL_SCPERCLKEN3_sc_noc_ufs_mst_i_mainnowrpendingtrans_bp_START (9)
#define SOC_SCTRL_SCPERCLKEN3_sc_noc_ufs_mst_i_mainnowrpendingtrans_bp_END (9)
#define SOC_SCTRL_SCPERCLKEN3_sc_noc_ufs_mst_i_mainnordpendingtrans_bp_START (10)
#define SOC_SCTRL_SCPERCLKEN3_sc_noc_ufs_mst_i_mainnordpendingtrans_bp_END (10)
#define SOC_SCTRL_SCPERCLKEN3_aonoc_ufshc_hibernate_ind_bp_START (11)
#define SOC_SCTRL_SCPERCLKEN3_aonoc_ufshc_hibernate_ind_bp_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_memrepair : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_ao_tzpc : 1;
        unsigned int gt_pclk_ao_ipc_ns : 1;
        unsigned int gt_pclk_ao_loadmonitor : 1;
        unsigned int gt_clk_ao_loadmonitor : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_pclk_gpio21 : 1;
        unsigned int gt_pclk_gpio20 : 1;
        unsigned int gt_clk_spi3 : 1;
        unsigned int gt_clk_ao_tcp : 1;
        unsigned int gt_clk_ao_tcp_32k : 1;
        unsigned int gt_clk_autodiv_ufs_subsys : 1;
        unsigned int gt_clk_ufs_subsys : 1;
        unsigned int gt_pclk_ao_gpio30 : 1;
        unsigned int gt_pclk_ao_gpio29 : 1;
        unsigned int gt_clk_ao_atb_brg : 1;
        unsigned int gt_clk_pcie_aux : 1;
        unsigned int gt_pclk_ao_gpio31 : 1;
        unsigned int gt_pclk_ao_gpio32 : 1;
        unsigned int gt_pclk_ao_gpio33 : 1;
        unsigned int gt_clk_csi_cfg_isp : 1;
        unsigned int gt_clk_csi_cfg_hifd : 1;
        unsigned int gt_clk_csi_cfg : 1;
        unsigned int gt_clk_ufs_ref_test : 1;
        unsigned int gt_pclk_ao_loadmonitor_m3 : 1;
        unsigned int gt_pclk_ao_gpio34 : 1;
        unsigned int gt_pclk_ao_gpio35 : 1;
        unsigned int gt_pclk_ao_gpio36 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
    } reg;
} SOC_SCTRL_SCPEREN4_UNION;
#endif
#define SOC_SCTRL_SCPEREN4_gt_clk_memrepair_START (0)
#define SOC_SCTRL_SCPEREN4_gt_clk_memrepair_END (0)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_tzpc_START (2)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_tzpc_END (2)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_ipc_ns_START (3)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_ipc_ns_END (3)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_loadmonitor_START (4)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_loadmonitor_END (4)
#define SOC_SCTRL_SCPEREN4_gt_clk_ao_loadmonitor_START (5)
#define SOC_SCTRL_SCPEREN4_gt_clk_ao_loadmonitor_END (5)
#define SOC_SCTRL_SCPEREN4_gt_pclk_gpio21_START (8)
#define SOC_SCTRL_SCPEREN4_gt_pclk_gpio21_END (8)
#define SOC_SCTRL_SCPEREN4_gt_pclk_gpio20_START (9)
#define SOC_SCTRL_SCPEREN4_gt_pclk_gpio20_END (9)
#define SOC_SCTRL_SCPEREN4_gt_clk_spi3_START (10)
#define SOC_SCTRL_SCPEREN4_gt_clk_spi3_END (10)
#define SOC_SCTRL_SCPEREN4_gt_clk_ao_tcp_START (11)
#define SOC_SCTRL_SCPEREN4_gt_clk_ao_tcp_END (11)
#define SOC_SCTRL_SCPEREN4_gt_clk_ao_tcp_32k_START (12)
#define SOC_SCTRL_SCPEREN4_gt_clk_ao_tcp_32k_END (12)
#define SOC_SCTRL_SCPEREN4_gt_clk_autodiv_ufs_subsys_START (13)
#define SOC_SCTRL_SCPEREN4_gt_clk_autodiv_ufs_subsys_END (13)
#define SOC_SCTRL_SCPEREN4_gt_clk_ufs_subsys_START (14)
#define SOC_SCTRL_SCPEREN4_gt_clk_ufs_subsys_END (14)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_gpio30_START (15)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_gpio30_END (15)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_gpio29_START (16)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_gpio29_END (16)
#define SOC_SCTRL_SCPEREN4_gt_clk_ao_atb_brg_START (17)
#define SOC_SCTRL_SCPEREN4_gt_clk_ao_atb_brg_END (17)
#define SOC_SCTRL_SCPEREN4_gt_clk_pcie_aux_START (18)
#define SOC_SCTRL_SCPEREN4_gt_clk_pcie_aux_END (18)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_gpio31_START (19)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_gpio31_END (19)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_gpio32_START (20)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_gpio32_END (20)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_gpio33_START (21)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_gpio33_END (21)
#define SOC_SCTRL_SCPEREN4_gt_clk_csi_cfg_isp_START (22)
#define SOC_SCTRL_SCPEREN4_gt_clk_csi_cfg_isp_END (22)
#define SOC_SCTRL_SCPEREN4_gt_clk_csi_cfg_hifd_START (23)
#define SOC_SCTRL_SCPEREN4_gt_clk_csi_cfg_hifd_END (23)
#define SOC_SCTRL_SCPEREN4_gt_clk_csi_cfg_START (24)
#define SOC_SCTRL_SCPEREN4_gt_clk_csi_cfg_END (24)
#define SOC_SCTRL_SCPEREN4_gt_clk_ufs_ref_test_START (25)
#define SOC_SCTRL_SCPEREN4_gt_clk_ufs_ref_test_END (25)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_loadmonitor_m3_START (26)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_loadmonitor_m3_END (26)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_gpio34_START (27)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_gpio34_END (27)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_gpio35_START (28)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_gpio35_END (28)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_gpio36_START (29)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_gpio36_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_memrepair : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_ao_tzpc : 1;
        unsigned int gt_pclk_ao_ipc_ns : 1;
        unsigned int gt_pclk_ao_loadmonitor : 1;
        unsigned int gt_clk_ao_loadmonitor : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_pclk_gpio21 : 1;
        unsigned int gt_pclk_gpio20 : 1;
        unsigned int gt_clk_spi3 : 1;
        unsigned int gt_clk_ao_tcp : 1;
        unsigned int gt_clk_ao_tcp_32k : 1;
        unsigned int gt_clk_autodiv_ufs_subsys : 1;
        unsigned int gt_clk_ufs_subsys : 1;
        unsigned int gt_pclk_ao_gpio30 : 1;
        unsigned int gt_pclk_ao_gpio29 : 1;
        unsigned int gt_clk_ao_atb_brg : 1;
        unsigned int gt_clk_pcie_aux : 1;
        unsigned int gt_pclk_ao_gpio31 : 1;
        unsigned int gt_pclk_ao_gpio32 : 1;
        unsigned int gt_pclk_ao_gpio33 : 1;
        unsigned int gt_clk_csi_cfg_isp : 1;
        unsigned int gt_clk_csi_cfg_hifd : 1;
        unsigned int gt_clk_csi_cfg : 1;
        unsigned int gt_clk_ufs_ref_test : 1;
        unsigned int gt_pclk_ao_loadmonitor_m3 : 1;
        unsigned int gt_pclk_ao_gpio34 : 1;
        unsigned int gt_pclk_ao_gpio35 : 1;
        unsigned int gt_pclk_ao_gpio36 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
    } reg;
} SOC_SCTRL_SCPERDIS4_UNION;
#endif
#define SOC_SCTRL_SCPERDIS4_gt_clk_memrepair_START (0)
#define SOC_SCTRL_SCPERDIS4_gt_clk_memrepair_END (0)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_tzpc_START (2)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_tzpc_END (2)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_ipc_ns_START (3)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_ipc_ns_END (3)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_loadmonitor_START (4)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_loadmonitor_END (4)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ao_loadmonitor_START (5)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ao_loadmonitor_END (5)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_gpio21_START (8)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_gpio21_END (8)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_gpio20_START (9)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_gpio20_END (9)
#define SOC_SCTRL_SCPERDIS4_gt_clk_spi3_START (10)
#define SOC_SCTRL_SCPERDIS4_gt_clk_spi3_END (10)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ao_tcp_START (11)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ao_tcp_END (11)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ao_tcp_32k_START (12)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ao_tcp_32k_END (12)
#define SOC_SCTRL_SCPERDIS4_gt_clk_autodiv_ufs_subsys_START (13)
#define SOC_SCTRL_SCPERDIS4_gt_clk_autodiv_ufs_subsys_END (13)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ufs_subsys_END (14)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_gpio30_START (15)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_gpio30_END (15)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_gpio29_START (16)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_gpio29_END (16)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ao_atb_brg_START (17)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ao_atb_brg_END (17)
#define SOC_SCTRL_SCPERDIS4_gt_clk_pcie_aux_START (18)
#define SOC_SCTRL_SCPERDIS4_gt_clk_pcie_aux_END (18)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_gpio31_START (19)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_gpio31_END (19)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_gpio32_START (20)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_gpio32_END (20)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_gpio33_START (21)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_gpio33_END (21)
#define SOC_SCTRL_SCPERDIS4_gt_clk_csi_cfg_isp_START (22)
#define SOC_SCTRL_SCPERDIS4_gt_clk_csi_cfg_isp_END (22)
#define SOC_SCTRL_SCPERDIS4_gt_clk_csi_cfg_hifd_START (23)
#define SOC_SCTRL_SCPERDIS4_gt_clk_csi_cfg_hifd_END (23)
#define SOC_SCTRL_SCPERDIS4_gt_clk_csi_cfg_START (24)
#define SOC_SCTRL_SCPERDIS4_gt_clk_csi_cfg_END (24)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ufs_ref_test_START (25)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ufs_ref_test_END (25)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_loadmonitor_m3_START (26)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_loadmonitor_m3_END (26)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_gpio34_START (27)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_gpio34_END (27)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_gpio35_START (28)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_gpio35_END (28)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_gpio36_START (29)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_gpio36_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_memrepair : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_ao_tzpc : 1;
        unsigned int gt_pclk_ao_ipc_ns : 1;
        unsigned int gt_pclk_ao_loadmonitor : 1;
        unsigned int gt_clk_ao_loadmonitor : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_pclk_gpio21 : 1;
        unsigned int gt_pclk_gpio20 : 1;
        unsigned int gt_clk_spi3 : 1;
        unsigned int gt_clk_ao_tcp : 1;
        unsigned int gt_clk_ao_tcp_32k : 1;
        unsigned int gt_clk_autodiv_ufs_subsys : 1;
        unsigned int gt_clk_ufs_subsys : 1;
        unsigned int gt_pclk_ao_gpio30 : 1;
        unsigned int gt_pclk_ao_gpio29 : 1;
        unsigned int gt_clk_ao_atb_brg : 1;
        unsigned int gt_clk_pcie_aux : 1;
        unsigned int gt_pclk_ao_gpio31 : 1;
        unsigned int gt_pclk_ao_gpio32 : 1;
        unsigned int gt_pclk_ao_gpio33 : 1;
        unsigned int gt_clk_csi_cfg_isp : 1;
        unsigned int gt_clk_csi_cfg_hifd : 1;
        unsigned int gt_clk_csi_cfg : 1;
        unsigned int gt_clk_ufs_ref_test : 1;
        unsigned int gt_pclk_ao_loadmonitor_m3 : 1;
        unsigned int gt_pclk_ao_gpio34 : 1;
        unsigned int gt_pclk_ao_gpio35 : 1;
        unsigned int gt_pclk_ao_gpio36 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN4_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_memrepair_START (0)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_memrepair_END (0)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_tzpc_START (2)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_tzpc_END (2)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_ipc_ns_START (3)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_ipc_ns_END (3)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_loadmonitor_START (4)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_loadmonitor_END (4)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ao_loadmonitor_START (5)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ao_loadmonitor_END (5)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_gpio21_START (8)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_gpio21_END (8)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_gpio20_START (9)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_gpio20_END (9)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_spi3_START (10)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_spi3_END (10)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ao_tcp_START (11)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ao_tcp_END (11)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ao_tcp_32k_START (12)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ao_tcp_32k_END (12)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_autodiv_ufs_subsys_START (13)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_autodiv_ufs_subsys_END (13)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ufs_subsys_END (14)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_gpio30_START (15)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_gpio30_END (15)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_gpio29_START (16)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_gpio29_END (16)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ao_atb_brg_START (17)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ao_atb_brg_END (17)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_pcie_aux_START (18)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_pcie_aux_END (18)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_gpio31_START (19)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_gpio31_END (19)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_gpio32_START (20)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_gpio32_END (20)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_gpio33_START (21)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_gpio33_END (21)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_csi_cfg_isp_START (22)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_csi_cfg_isp_END (22)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_csi_cfg_hifd_START (23)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_csi_cfg_hifd_END (23)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_csi_cfg_START (24)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_csi_cfg_END (24)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ufs_ref_test_START (25)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ufs_ref_test_END (25)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_loadmonitor_m3_START (26)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_loadmonitor_m3_END (26)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_gpio34_START (27)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_gpio34_END (27)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_gpio35_START (28)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_gpio35_END (28)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_gpio36_START (29)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_gpio36_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int st_clk_memrepair : 1;
        unsigned int st_clk_spmi_mst : 1;
        unsigned int st_pclk_ao_tzpc : 1;
        unsigned int st_pclk_ao_ipc_ns : 1;
        unsigned int st_pclk_ao_loadmonitor : 1;
        unsigned int st_clk_ao_loadmonitor : 1;
        unsigned int sw_ack_clk_mad_mux_pre : 2;
        unsigned int st_pclk_gpio21 : 1;
        unsigned int st_pclk_gpio20 : 1;
        unsigned int st_clk_spi3 : 1;
        unsigned int sw_ack_clk_asp_codec_mux : 2;
        unsigned int st_clk_autodiv_ufs_subsys : 1;
        unsigned int st_clk_ufs_subsys : 1;
        unsigned int st_pclk_ao_gpio30 : 1;
        unsigned int st_pclk_ao_gpio29 : 1;
        unsigned int st_clk_ao_atb_brg : 1;
        unsigned int st_clk_pcie_aux : 1;
        unsigned int st_pclk_ao_gpio31 : 1;
        unsigned int st_pclk_ao_gpio32 : 1;
        unsigned int st_pclk_ao_gpio33 : 1;
        unsigned int st_clk_csi_cfg_hifd : 1;
        unsigned int st_clk_csi_cfg : 1;
        unsigned int reserved_0 : 1;
        unsigned int st_clk_ufs_ref_test : 1;
        unsigned int st_pclk_ao_gpio36 : 1;
        unsigned int st_pclk_ao_gpio34 : 1;
        unsigned int st_pclk_ao_gpio35 : 1;
        unsigned int st_clk_aonoc2fdbus : 1;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_SCTRL_SCPERSTAT4_UNION;
#endif
#define SOC_SCTRL_SCPERSTAT4_st_clk_memrepair_START (0)
#define SOC_SCTRL_SCPERSTAT4_st_clk_memrepair_END (0)
#define SOC_SCTRL_SCPERSTAT4_st_clk_spmi_mst_START (1)
#define SOC_SCTRL_SCPERSTAT4_st_clk_spmi_mst_END (1)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_tzpc_START (2)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_tzpc_END (2)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_ipc_ns_START (3)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_ipc_ns_END (3)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_loadmonitor_START (4)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_loadmonitor_END (4)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ao_loadmonitor_START (5)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ao_loadmonitor_END (5)
#define SOC_SCTRL_SCPERSTAT4_sw_ack_clk_mad_mux_pre_START (6)
#define SOC_SCTRL_SCPERSTAT4_sw_ack_clk_mad_mux_pre_END (7)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_gpio21_START (8)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_gpio21_END (8)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_gpio20_START (9)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_gpio20_END (9)
#define SOC_SCTRL_SCPERSTAT4_st_clk_spi3_START (10)
#define SOC_SCTRL_SCPERSTAT4_st_clk_spi3_END (10)
#define SOC_SCTRL_SCPERSTAT4_sw_ack_clk_asp_codec_mux_START (11)
#define SOC_SCTRL_SCPERSTAT4_sw_ack_clk_asp_codec_mux_END (12)
#define SOC_SCTRL_SCPERSTAT4_st_clk_autodiv_ufs_subsys_START (13)
#define SOC_SCTRL_SCPERSTAT4_st_clk_autodiv_ufs_subsys_END (13)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ufs_subsys_END (14)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_gpio30_START (15)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_gpio30_END (15)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_gpio29_START (16)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_gpio29_END (16)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ao_atb_brg_START (17)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ao_atb_brg_END (17)
#define SOC_SCTRL_SCPERSTAT4_st_clk_pcie_aux_START (18)
#define SOC_SCTRL_SCPERSTAT4_st_clk_pcie_aux_END (18)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_gpio31_START (19)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_gpio31_END (19)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_gpio32_START (20)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_gpio32_END (20)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_gpio33_START (21)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_gpio33_END (21)
#define SOC_SCTRL_SCPERSTAT4_st_clk_csi_cfg_hifd_START (22)
#define SOC_SCTRL_SCPERSTAT4_st_clk_csi_cfg_hifd_END (22)
#define SOC_SCTRL_SCPERSTAT4_st_clk_csi_cfg_START (23)
#define SOC_SCTRL_SCPERSTAT4_st_clk_csi_cfg_END (23)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ufs_ref_test_START (25)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ufs_ref_test_END (25)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_gpio36_START (26)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_gpio36_END (26)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_gpio34_START (27)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_gpio34_END (27)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_gpio35_START (28)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_gpio35_END (28)
#define SOC_SCTRL_SCPERSTAT4_st_clk_aonoc2fdbus_START (29)
#define SOC_SCTRL_SCPERSTAT4_st_clk_aonoc2fdbus_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_pclk_timer17 : 1;
        unsigned int gt_clk_timer17 : 1;
        unsigned int gt_pclk_timer16 : 1;
        unsigned int gt_clk_timer16 : 1;
        unsigned int gt_pclk_timer15 : 1;
        unsigned int gt_clk_timer15 : 1;
        unsigned int gt_pclk_timer14 : 1;
        unsigned int gt_clk_timer14 : 1;
        unsigned int gt_pclk_timer13 : 1;
        unsigned int gt_clk_timer13 : 1;
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
    } reg;
} SOC_SCTRL_SCPEREN5_UNION;
#endif
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer17_START (0)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer17_END (0)
#define SOC_SCTRL_SCPEREN5_gt_clk_timer17_START (1)
#define SOC_SCTRL_SCPEREN5_gt_clk_timer17_END (1)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer16_START (2)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer16_END (2)
#define SOC_SCTRL_SCPEREN5_gt_clk_timer16_START (3)
#define SOC_SCTRL_SCPEREN5_gt_clk_timer16_END (3)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer15_START (4)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer15_END (4)
#define SOC_SCTRL_SCPEREN5_gt_clk_timer15_START (5)
#define SOC_SCTRL_SCPEREN5_gt_clk_timer15_END (5)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer14_START (6)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer14_END (6)
#define SOC_SCTRL_SCPEREN5_gt_clk_timer14_START (7)
#define SOC_SCTRL_SCPEREN5_gt_clk_timer14_END (7)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer13_START (8)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer13_END (8)
#define SOC_SCTRL_SCPEREN5_gt_clk_timer13_START (9)
#define SOC_SCTRL_SCPEREN5_gt_clk_timer13_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_pclk_timer17 : 1;
        unsigned int gt_clk_timer17 : 1;
        unsigned int gt_pclk_timer16 : 1;
        unsigned int gt_clk_timer16 : 1;
        unsigned int gt_pclk_timer15 : 1;
        unsigned int gt_clk_timer15 : 1;
        unsigned int gt_pclk_timer14 : 1;
        unsigned int gt_clk_timer14 : 1;
        unsigned int gt_pclk_timer13 : 1;
        unsigned int gt_clk_timer13 : 1;
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
    } reg;
} SOC_SCTRL_SCPERDIS5_UNION;
#endif
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer17_START (0)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer17_END (0)
#define SOC_SCTRL_SCPERDIS5_gt_clk_timer17_START (1)
#define SOC_SCTRL_SCPERDIS5_gt_clk_timer17_END (1)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer16_START (2)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer16_END (2)
#define SOC_SCTRL_SCPERDIS5_gt_clk_timer16_START (3)
#define SOC_SCTRL_SCPERDIS5_gt_clk_timer16_END (3)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer15_START (4)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer15_END (4)
#define SOC_SCTRL_SCPERDIS5_gt_clk_timer15_START (5)
#define SOC_SCTRL_SCPERDIS5_gt_clk_timer15_END (5)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer14_START (6)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer14_END (6)
#define SOC_SCTRL_SCPERDIS5_gt_clk_timer14_START (7)
#define SOC_SCTRL_SCPERDIS5_gt_clk_timer14_END (7)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer13_START (8)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer13_END (8)
#define SOC_SCTRL_SCPERDIS5_gt_clk_timer13_START (9)
#define SOC_SCTRL_SCPERDIS5_gt_clk_timer13_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_pclk_timer17 : 1;
        unsigned int gt_clk_timer17 : 1;
        unsigned int gt_pclk_timer16 : 1;
        unsigned int gt_clk_timer16 : 1;
        unsigned int gt_pclk_timer15 : 1;
        unsigned int gt_clk_timer15 : 1;
        unsigned int gt_pclk_timer14 : 1;
        unsigned int gt_clk_timer14 : 1;
        unsigned int gt_pclk_timer13 : 1;
        unsigned int gt_clk_timer13 : 1;
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
    } reg;
} SOC_SCTRL_SCPERCLKEN5_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer17_START (0)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer17_END (0)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_timer17_START (1)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_timer17_END (1)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer16_START (2)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer16_END (2)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_timer16_START (3)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_timer16_END (3)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer15_START (4)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer15_END (4)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_timer15_START (5)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_timer15_END (5)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer14_START (6)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer14_END (6)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_timer14_START (7)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_timer14_END (7)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer13_START (8)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer13_END (8)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_timer13_START (9)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_timer13_END (9)
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
        unsigned int reserved_16: 1;
        unsigned int reserved_17: 1;
        unsigned int reserved_18: 1;
        unsigned int reserved_19: 1;
        unsigned int reserved_20: 1;
        unsigned int reserved_21: 1;
        unsigned int reserved_22: 1;
        unsigned int reserved_23: 1;
        unsigned int reserved_24: 1;
        unsigned int reserved_25: 1;
        unsigned int reserved_26: 1;
        unsigned int reserved_27: 1;
        unsigned int reserved_28: 1;
        unsigned int reserved_29: 1;
        unsigned int reserved_30: 2;
    } reg;
} SOC_SCTRL_SCPERSTAT5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_rtc : 1;
        unsigned int ip_prst_rtc1 : 1;
        unsigned int ip_prst_timer0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_prst_timer2 : 1;
        unsigned int ip_prst_timer3 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_asp_dw_axi : 1;
        unsigned int ip_rst_asp_x2h : 1;
        unsigned int ip_rst_asp_h2p : 1;
        unsigned int ip_prst_ao_gpio0 : 1;
        unsigned int ip_prst_ao_gpio1 : 1;
        unsigned int ip_prst_ao_gpio2 : 1;
        unsigned int ip_prst_ao_gpio3 : 1;
        unsigned int ip_prst_ao_gpio4 : 1;
        unsigned int ip_prst_ao_gpio5 : 1;
        unsigned int ip_prst_ao_gpio6 : 1;
        unsigned int ip_prst_ao_ioc : 1;
        unsigned int ip_prst_syscnt : 1;
        unsigned int ip_rst_syscnt : 1;
        unsigned int ip_prst_ao_gpio30 : 1;
        unsigned int ip_prst_ao_gpio29 : 1;
        unsigned int ip_rst_sci0 : 1;
        unsigned int ip_rst_sci1 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int ip_prst_bbpdrx : 1;
        unsigned int ip_prst_ao_ipc_ns : 1;
        unsigned int ip_rst_aobus : 1;
        unsigned int sc_rst_src_clr : 1;
        unsigned int sc_pmurst_ctrl : 1;
    } reg;
} SOC_SCTRL_SCPERRSTEN0_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_rtc_START (1)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_rtc_END (1)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_rtc1_START (2)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_rtc1_END (2)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_timer0_START (3)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_timer0_END (3)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_timer2_START (5)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_timer2_END (5)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_timer3_START (6)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_timer3_END (6)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_asp_dw_axi_START (8)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_asp_dw_axi_END (8)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_asp_x2h_START (9)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_asp_x2h_END (9)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_asp_h2p_START (10)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_asp_h2p_END (10)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio0_START (11)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio0_END (11)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio1_START (12)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio1_END (12)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio2_START (13)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio2_END (13)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio3_START (14)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio3_END (14)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio4_START (15)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio4_END (15)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio5_START (16)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio5_END (16)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio6_START (17)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio6_END (17)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_ioc_START (18)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_ioc_END (18)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_syscnt_START (19)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_syscnt_END (19)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_syscnt_START (20)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_syscnt_END (20)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio30_START (21)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio30_END (21)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio29_START (22)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_gpio29_END (22)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_sci0_START (23)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_sci0_END (23)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_sci1_START (24)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_sci1_END (24)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_bbpdrx_START (27)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_bbpdrx_END (27)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_ipc_ns_START (28)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_ipc_ns_END (28)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_aobus_START (29)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_aobus_END (29)
#define SOC_SCTRL_SCPERRSTEN0_sc_rst_src_clr_START (30)
#define SOC_SCTRL_SCPERRSTEN0_sc_rst_src_clr_END (30)
#define SOC_SCTRL_SCPERRSTEN0_sc_pmurst_ctrl_START (31)
#define SOC_SCTRL_SCPERRSTEN0_sc_pmurst_ctrl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_rtc : 1;
        unsigned int ip_prst_rtc1 : 1;
        unsigned int ip_prst_timer0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_prst_timer2 : 1;
        unsigned int ip_prst_timer3 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_asp_dw_axi : 1;
        unsigned int ip_rst_asp_x2h : 1;
        unsigned int ip_rst_asp_h2p : 1;
        unsigned int ip_prst_ao_gpio0 : 1;
        unsigned int ip_prst_ao_gpio1 : 1;
        unsigned int ip_prst_ao_gpio2 : 1;
        unsigned int ip_prst_ao_gpio3 : 1;
        unsigned int ip_prst_ao_gpio4 : 1;
        unsigned int ip_prst_ao_gpio5 : 1;
        unsigned int ip_prst_ao_gpio6 : 1;
        unsigned int ip_prst_ao_ioc : 1;
        unsigned int ip_prst_syscnt : 1;
        unsigned int ip_rst_syscnt : 1;
        unsigned int ip_prst_ao_gpio30 : 1;
        unsigned int ip_prst_ao_gpio29 : 1;
        unsigned int ip_rst_sci0 : 1;
        unsigned int ip_rst_sci1 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int ip_prst_bbpdrx : 1;
        unsigned int ip_prst_ao_ipc_ns : 1;
        unsigned int ip_rst_aobus : 1;
        unsigned int sc_rst_src_clr : 1;
        unsigned int sc_pmurst_ctrl : 1;
    } reg;
} SOC_SCTRL_SCPERRSTDIS0_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_rtc_START (1)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_rtc_END (1)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_rtc1_START (2)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_rtc1_END (2)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_timer0_START (3)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_timer0_END (3)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_timer2_START (5)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_timer2_END (5)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_timer3_START (6)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_timer3_END (6)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_asp_dw_axi_START (8)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_asp_dw_axi_END (8)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_asp_x2h_START (9)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_asp_x2h_END (9)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_asp_h2p_START (10)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_asp_h2p_END (10)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio0_START (11)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio0_END (11)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio1_START (12)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio1_END (12)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio2_START (13)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio2_END (13)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio3_START (14)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio3_END (14)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio4_START (15)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio4_END (15)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio5_START (16)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio5_END (16)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio6_START (17)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio6_END (17)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_ioc_START (18)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_ioc_END (18)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_syscnt_START (19)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_syscnt_END (19)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_syscnt_START (20)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_syscnt_END (20)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio30_START (21)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio30_END (21)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio29_START (22)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_gpio29_END (22)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_sci0_START (23)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_sci0_END (23)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_sci1_START (24)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_sci1_END (24)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_bbpdrx_START (27)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_bbpdrx_END (27)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_ipc_ns_START (28)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_ipc_ns_END (28)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_aobus_START (29)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_aobus_END (29)
#define SOC_SCTRL_SCPERRSTDIS0_sc_rst_src_clr_START (30)
#define SOC_SCTRL_SCPERRSTDIS0_sc_rst_src_clr_END (30)
#define SOC_SCTRL_SCPERRSTDIS0_sc_pmurst_ctrl_START (31)
#define SOC_SCTRL_SCPERRSTDIS0_sc_pmurst_ctrl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_rtc : 1;
        unsigned int ip_prst_rtc1 : 1;
        unsigned int ip_prst_timer0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_prst_timer2 : 1;
        unsigned int ip_prst_timer3 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_asp_dw_axi : 1;
        unsigned int ip_rst_asp_x2h : 1;
        unsigned int ip_rst_asp_h2p : 1;
        unsigned int ip_prst_ao_gpio0 : 1;
        unsigned int ip_prst_ao_gpio1 : 1;
        unsigned int ip_prst_ao_gpio2 : 1;
        unsigned int ip_prst_ao_gpio3 : 1;
        unsigned int ip_prst_ao_gpio4 : 1;
        unsigned int ip_prst_ao_gpio5 : 1;
        unsigned int ip_prst_ao_gpio6 : 1;
        unsigned int ip_prst_ao_ioc : 1;
        unsigned int ip_prst_syscnt : 1;
        unsigned int ip_rst_syscnt : 1;
        unsigned int ip_prst_ao_gpio30 : 1;
        unsigned int ip_prst_ao_gpio29 : 1;
        unsigned int ip_rst_sci0 : 1;
        unsigned int ip_rst_sci1 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int ip_prst_bbpdrx : 1;
        unsigned int ip_prst_ao_ipc_ns : 1;
        unsigned int ip_rst_aobus : 1;
        unsigned int sc_rst_src_clr : 1;
        unsigned int sc_pmurst_ctrl : 1;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT0_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_rtc_START (1)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_rtc_END (1)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_rtc1_START (2)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_rtc1_END (2)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_timer0_START (3)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_timer0_END (3)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_timer2_START (5)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_timer2_END (5)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_timer3_START (6)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_timer3_END (6)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_asp_dw_axi_START (8)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_asp_dw_axi_END (8)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_asp_x2h_START (9)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_asp_x2h_END (9)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_asp_h2p_START (10)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_asp_h2p_END (10)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio0_START (11)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio0_END (11)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio1_START (12)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio1_END (12)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio2_START (13)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio2_END (13)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio3_START (14)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio3_END (14)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio4_START (15)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio4_END (15)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio5_START (16)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio5_END (16)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio6_START (17)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio6_END (17)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_ioc_START (18)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_ioc_END (18)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_syscnt_START (19)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_syscnt_END (19)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_syscnt_START (20)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_syscnt_END (20)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio30_START (21)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio30_END (21)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio29_START (22)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_gpio29_END (22)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_sci0_START (23)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_sci0_END (23)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_sci1_START (24)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_sci1_END (24)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_bbpdrx_START (27)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_bbpdrx_END (27)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_ipc_ns_START (28)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_ipc_ns_END (28)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_aobus_START (29)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_aobus_END (29)
#define SOC_SCTRL_SCPERRSTSTAT0_sc_rst_src_clr_START (30)
#define SOC_SCTRL_SCPERRSTSTAT0_sc_rst_src_clr_END (30)
#define SOC_SCTRL_SCPERRSTSTAT0_sc_pmurst_ctrl_START (31)
#define SOC_SCTRL_SCPERRSTSTAT0_sc_pmurst_ctrl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_spmi : 1;
        unsigned int ip_prst_ao_tzpc : 1;
        unsigned int ip_prst_dbg_to_ao : 1;
        unsigned int ip_prst_timer4 : 1;
        unsigned int ip_prst_timer5 : 1;
        unsigned int ip_prst_timer6 : 1;
        unsigned int ip_prst_timer7 : 1;
        unsigned int ip_prst_timer8 : 1;
        unsigned int ip_rst_aonoc : 1;
        unsigned int ip_rst_trace_ao : 1;
        unsigned int ip_rst_csi_cfg_hifd : 1;
        unsigned int ip_rst_csi_cfg : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_ao_loadmonitor : 1;
        unsigned int ip_rst_ufs_subsys : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_dram_tmp_0 : 2;
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
        unsigned int ip_rst_ao_tcp : 1;
        unsigned int ip_rst_ao_tcp_32k : 1;
    } reg;
} SOC_SCTRL_SCPERRSTEN1_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_spmi_START (0)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_spmi_END (0)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_ao_tzpc_START (1)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_ao_tzpc_END (1)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_dbg_to_ao_START (2)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_dbg_to_ao_END (2)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer4_START (3)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer4_END (3)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer5_START (4)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer5_END (4)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer6_START (5)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer6_END (5)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer7_START (6)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer7_END (6)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer8_START (7)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer8_END (7)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_aonoc_START (8)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_aonoc_END (8)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_trace_ao_START (9)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_trace_ao_END (9)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_csi_cfg_hifd_START (10)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_csi_cfg_hifd_END (10)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_csi_cfg_START (11)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_csi_cfg_END (11)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_ao_loadmonitor_START (13)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_ao_loadmonitor_END (13)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_ufs_subsys_END (14)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_dram_tmp_0_START (16)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_dram_tmp_0_END (17)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_ao_tcp_START (30)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_ao_tcp_END (30)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_ao_tcp_32k_START (31)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_ao_tcp_32k_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_spmi : 1;
        unsigned int ip_prst_ao_tzpc : 1;
        unsigned int ip_prst_dbg_to_ao : 1;
        unsigned int ip_prst_timer4 : 1;
        unsigned int ip_prst_timer5 : 1;
        unsigned int ip_prst_timer6 : 1;
        unsigned int ip_prst_timer7 : 1;
        unsigned int ip_prst_timer8 : 1;
        unsigned int ip_rst_aonoc : 1;
        unsigned int ip_rst_trace_ao : 1;
        unsigned int ip_rst_csi_cfg_hifd : 1;
        unsigned int ip_rst_csi_cfg : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_ao_loadmonitor : 1;
        unsigned int ip_rst_ufs_subsys : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_dram_tmp_0 : 2;
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
        unsigned int ip_rst_ao_tcp : 1;
        unsigned int ip_rst_ao_tcp_32k : 1;
    } reg;
} SOC_SCTRL_SCPERRSTDIS1_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_spmi_START (0)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_spmi_END (0)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_ao_tzpc_START (1)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_ao_tzpc_END (1)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_dbg_to_ao_START (2)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_dbg_to_ao_END (2)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer4_START (3)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer4_END (3)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer5_START (4)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer5_END (4)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer6_START (5)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer6_END (5)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer7_START (6)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer7_END (6)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer8_START (7)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer8_END (7)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_aonoc_START (8)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_aonoc_END (8)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_trace_ao_START (9)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_trace_ao_END (9)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_csi_cfg_hifd_START (10)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_csi_cfg_hifd_END (10)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_csi_cfg_START (11)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_csi_cfg_END (11)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_ao_loadmonitor_START (13)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_ao_loadmonitor_END (13)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_ufs_subsys_END (14)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_dram_tmp_0_START (16)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_dram_tmp_0_END (17)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_ao_tcp_START (30)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_ao_tcp_END (30)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_ao_tcp_32k_START (31)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_ao_tcp_32k_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_spmi : 1;
        unsigned int ip_prst_ao_tzpc : 1;
        unsigned int ip_prst_dbg_to_ao : 1;
        unsigned int ip_prst_timer4 : 1;
        unsigned int ip_prst_timer5 : 1;
        unsigned int ip_prst_timer6 : 1;
        unsigned int ip_prst_timer7 : 1;
        unsigned int ip_prst_timer8 : 1;
        unsigned int ip_rst_aonoc : 1;
        unsigned int ip_rst_trace_ao : 1;
        unsigned int ip_rst_csi_cfg_hifd : 1;
        unsigned int ip_rst_csi_cfg : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_ao_loadmonitor : 1;
        unsigned int ip_rst_ufs_subsys : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_dram_tmp_0 : 2;
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
        unsigned int ip_rst_ao_tcp : 1;
        unsigned int ip_rst_ao_tcp_32k : 1;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT1_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_spmi_START (0)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_spmi_END (0)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_ao_tzpc_START (1)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_ao_tzpc_END (1)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_dbg_to_ao_START (2)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_dbg_to_ao_END (2)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer4_START (3)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer4_END (3)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer5_START (4)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer5_END (4)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer6_START (5)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer6_END (5)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer7_START (6)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer7_END (6)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer8_START (7)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer8_END (7)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_aonoc_START (8)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_aonoc_END (8)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_trace_ao_START (9)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_trace_ao_END (9)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_csi_cfg_hifd_START (10)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_csi_cfg_hifd_END (10)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_csi_cfg_START (11)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_csi_cfg_END (11)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_ao_loadmonitor_START (13)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_ao_loadmonitor_END (13)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_ufs_subsys_END (14)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_dram_tmp_0_START (16)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_dram_tmp_0_END (17)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_ao_tcp_START (30)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_ao_tcp_END (30)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_ao_tcp_32k_START (31)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_ao_tcp_32k_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 3;
        unsigned int soft_rst_dram_dis : 1;
        unsigned int ip_rst_dram_tmp_1 : 2;
        unsigned int ip_prst_gpio21 : 1;
        unsigned int ip_prst_gpio20 : 1;
        unsigned int ip_rst_spi3 : 1;
        unsigned int ip_prst_ao_gpio31 : 1;
        unsigned int ip_prst_ao_gpio32 : 1;
        unsigned int ip_prst_ao_gpio33 : 1;
        unsigned int ip_prst_ao_gpio34 : 1;
        unsigned int ip_prst_ao_gpio35 : 1;
        unsigned int ip_prst_ao_gpio36 : 1;
        unsigned int ip_prst_timer17 : 1;
        unsigned int ip_prst_timer16 : 1;
        unsigned int ip_prst_timer15 : 1;
        unsigned int ip_prst_timer14 : 1;
        unsigned int ip_prst_timer13 : 1;
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
    } reg;
} SOC_SCTRL_SCPERRSTEN2_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN2_soft_rst_dram_dis_START (4)
#define SOC_SCTRL_SCPERRSTEN2_soft_rst_dram_dis_END (4)
#define SOC_SCTRL_SCPERRSTEN2_ip_rst_dram_tmp_1_START (5)
#define SOC_SCTRL_SCPERRSTEN2_ip_rst_dram_tmp_1_END (6)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_gpio21_START (7)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_gpio21_END (7)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_gpio20_START (8)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_gpio20_END (8)
#define SOC_SCTRL_SCPERRSTEN2_ip_rst_spi3_START (9)
#define SOC_SCTRL_SCPERRSTEN2_ip_rst_spi3_END (9)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_ao_gpio31_START (10)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_ao_gpio31_END (10)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_ao_gpio32_START (11)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_ao_gpio32_END (11)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_ao_gpio33_START (12)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_ao_gpio33_END (12)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_ao_gpio34_START (13)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_ao_gpio34_END (13)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_ao_gpio35_START (14)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_ao_gpio35_END (14)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_ao_gpio36_START (15)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_ao_gpio36_END (15)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer17_START (16)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer17_END (16)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer16_START (17)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer16_END (17)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer15_START (18)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer15_END (18)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer14_START (19)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer14_END (19)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer13_START (20)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer13_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 3;
        unsigned int soft_rst_dram_dis : 1;
        unsigned int ip_rst_dram_tmp_1 : 2;
        unsigned int ip_prst_gpio21 : 1;
        unsigned int ip_prst_gpio20 : 1;
        unsigned int ip_rst_spi3 : 1;
        unsigned int ip_prst_ao_gpio31 : 1;
        unsigned int ip_prst_ao_gpio32 : 1;
        unsigned int ip_prst_ao_gpio33 : 1;
        unsigned int ip_prst_ao_gpio34 : 1;
        unsigned int ip_prst_ao_gpio35 : 1;
        unsigned int ip_prst_ao_gpio36 : 1;
        unsigned int ip_prst_timer17 : 1;
        unsigned int ip_prst_timer16 : 1;
        unsigned int ip_prst_timer15 : 1;
        unsigned int ip_prst_timer14 : 1;
        unsigned int ip_prst_timer13 : 1;
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
    } reg;
} SOC_SCTRL_SCPERRSTDIS2_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS2_soft_rst_dram_dis_START (4)
#define SOC_SCTRL_SCPERRSTDIS2_soft_rst_dram_dis_END (4)
#define SOC_SCTRL_SCPERRSTDIS2_ip_rst_dram_tmp_1_START (5)
#define SOC_SCTRL_SCPERRSTDIS2_ip_rst_dram_tmp_1_END (6)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_gpio21_START (7)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_gpio21_END (7)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_gpio20_START (8)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_gpio20_END (8)
#define SOC_SCTRL_SCPERRSTDIS2_ip_rst_spi3_START (9)
#define SOC_SCTRL_SCPERRSTDIS2_ip_rst_spi3_END (9)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_ao_gpio31_START (10)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_ao_gpio31_END (10)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_ao_gpio32_START (11)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_ao_gpio32_END (11)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_ao_gpio33_START (12)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_ao_gpio33_END (12)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_ao_gpio34_START (13)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_ao_gpio34_END (13)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_ao_gpio35_START (14)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_ao_gpio35_END (14)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_ao_gpio36_START (15)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_ao_gpio36_END (15)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer17_START (16)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer17_END (16)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer16_START (17)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer16_END (17)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer15_START (18)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer15_END (18)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer14_START (19)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer14_END (19)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer13_START (20)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer13_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 3;
        unsigned int soft_rst_dram_dis : 1;
        unsigned int ip_rst_dram_tmp_1 : 2;
        unsigned int ip_prst_gpio21 : 1;
        unsigned int ip_prst_gpio20 : 1;
        unsigned int ip_rst_spi3 : 1;
        unsigned int ip_prst_ao_gpio31 : 1;
        unsigned int ip_prst_ao_gpio32 : 1;
        unsigned int ip_prst_ao_gpio33 : 1;
        unsigned int ip_prst_ao_gpio34 : 1;
        unsigned int ip_prst_ao_gpio35 : 1;
        unsigned int ip_prst_ao_gpio36 : 1;
        unsigned int ip_prst_timer17 : 1;
        unsigned int ip_prst_timer16 : 1;
        unsigned int ip_prst_timer15 : 1;
        unsigned int ip_prst_timer14 : 1;
        unsigned int ip_prst_timer13 : 1;
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
    } reg;
} SOC_SCTRL_SCPERRSTSTAT2_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT2_soft_rst_dram_dis_START (4)
#define SOC_SCTRL_SCPERRSTSTAT2_soft_rst_dram_dis_END (4)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_rst_dram_tmp_1_START (5)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_rst_dram_tmp_1_END (6)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_gpio21_START (7)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_gpio21_END (7)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_gpio20_START (8)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_gpio20_END (8)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_rst_spi3_START (9)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_rst_spi3_END (9)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_ao_gpio31_START (10)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_ao_gpio31_END (10)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_ao_gpio32_START (11)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_ao_gpio32_END (11)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_ao_gpio33_START (12)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_ao_gpio33_END (12)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_ao_gpio34_START (13)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_ao_gpio34_END (13)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_ao_gpio35_START (14)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_ao_gpio35_END (14)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_ao_gpio36_START (15)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_ao_gpio36_END (15)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer17_START (16)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer17_END (16)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer16_START (17)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer16_END (17)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer15_START (18)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer15_END (18)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer14_START (19)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer14_END (19)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer13_START (20)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer13_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtc_clkrst_bypass : 1;
        unsigned int rtc1_clkrst_bypass : 1;
        unsigned int timer0_clkrst_bypass : 1;
        unsigned int timer1_clkrst_bypass : 1;
        unsigned int timer2_clkrst_bypass : 1;
        unsigned int timer3_clkrst_bypass : 1;
        unsigned int gpio0_clkrst_bypass : 1;
        unsigned int gpio1_clkrst_bypass : 1;
        unsigned int gpio2_clkrst_bypass : 1;
        unsigned int gpio3_clkrst_bypass : 1;
        unsigned int gpio4_clkrst_bypass : 1;
        unsigned int gpio5_clkrst_bypass : 1;
        unsigned int ioc_clkrst_bypass : 1;
        unsigned int syscnt_clkrst_bypass : 1;
        unsigned int sci0_clkrst_bypass : 1;
        unsigned int sci1_clkrst_bypass : 1;
        unsigned int bbpdrx_clkrst_bypass : 1;
        unsigned int efusec_clkrst_bypass : 1;
        unsigned int timer4_clkrst_bypass : 1;
        unsigned int timer5_clkrst_bypass : 1;
        unsigned int timer6_clkrst_bypass : 1;
        unsigned int timer7_clkrst_bypass : 1;
        unsigned int timer8_clkrst_bypass : 1;
        unsigned int gpio30_clkrst_bypass : 1;
        unsigned int gpio29_clkrst_bypass : 1;
        unsigned int gpio6_clkrst_bypass : 1;
        unsigned int gpio1_se_clkrst_bypass : 1;
        unsigned int gpio34_clkrst_bypass : 1;
        unsigned int gpio35_clkrst_bypass : 1;
        unsigned int ao_loadmonitor_clkrst_bypass : 1;
        unsigned int spmi_clkrst_bypass : 1;
        unsigned int ao_tzpc_clkrst_bypass : 1;
    } reg;
} SOC_SCTRL_SC_IPCLKRST_BYPASS0_UNION;
#endif
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_rtc_clkrst_bypass_START (0)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_rtc_clkrst_bypass_END (0)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_rtc1_clkrst_bypass_START (1)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_rtc1_clkrst_bypass_END (1)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer0_clkrst_bypass_START (2)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer0_clkrst_bypass_END (2)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer1_clkrst_bypass_START (3)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer1_clkrst_bypass_END (3)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer2_clkrst_bypass_START (4)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer2_clkrst_bypass_END (4)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer3_clkrst_bypass_START (5)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer3_clkrst_bypass_END (5)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio0_clkrst_bypass_START (6)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio0_clkrst_bypass_END (6)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio1_clkrst_bypass_START (7)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio1_clkrst_bypass_END (7)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio2_clkrst_bypass_START (8)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio2_clkrst_bypass_END (8)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio3_clkrst_bypass_START (9)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio3_clkrst_bypass_END (9)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio4_clkrst_bypass_START (10)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio4_clkrst_bypass_END (10)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio5_clkrst_bypass_START (11)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio5_clkrst_bypass_END (11)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_ioc_clkrst_bypass_START (12)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_ioc_clkrst_bypass_END (12)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_syscnt_clkrst_bypass_START (13)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_syscnt_clkrst_bypass_END (13)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_sci0_clkrst_bypass_START (14)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_sci0_clkrst_bypass_END (14)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_sci1_clkrst_bypass_START (15)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_sci1_clkrst_bypass_END (15)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_bbpdrx_clkrst_bypass_START (16)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_bbpdrx_clkrst_bypass_END (16)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_efusec_clkrst_bypass_START (17)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_efusec_clkrst_bypass_END (17)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer4_clkrst_bypass_START (18)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer4_clkrst_bypass_END (18)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer5_clkrst_bypass_START (19)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer5_clkrst_bypass_END (19)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer6_clkrst_bypass_START (20)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer6_clkrst_bypass_END (20)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer7_clkrst_bypass_START (21)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer7_clkrst_bypass_END (21)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer8_clkrst_bypass_START (22)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_timer8_clkrst_bypass_END (22)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio30_clkrst_bypass_START (23)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio30_clkrst_bypass_END (23)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio29_clkrst_bypass_START (24)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio29_clkrst_bypass_END (24)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio6_clkrst_bypass_START (25)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio6_clkrst_bypass_END (25)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio1_se_clkrst_bypass_START (26)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio1_se_clkrst_bypass_END (26)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio34_clkrst_bypass_START (27)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio34_clkrst_bypass_END (27)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio35_clkrst_bypass_START (28)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_gpio35_clkrst_bypass_END (28)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_ao_loadmonitor_clkrst_bypass_START (29)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_ao_loadmonitor_clkrst_bypass_END (29)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_spmi_clkrst_bypass_START (30)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_spmi_clkrst_bypass_END (30)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_ao_tzpc_clkrst_bypass_START (31)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_ao_tzpc_clkrst_bypass_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ao_ipc_clkrst_bypass : 1;
        unsigned int ao_ipc_ns_clkrst_bypass : 1;
        unsigned int gpio21_clkrst_bypass : 1;
        unsigned int gpio20_clkrst_bypass : 1;
        unsigned int spi3_clkrst_bypass : 1;
        unsigned int tcp_clkrst_bypass : 1;
        unsigned int ufs_sys_ctrl_clkrst_bypass : 1;
        unsigned int ao_wd_clkrst_bypass : 1;
        unsigned int gpio31_clkrst_bypass : 1;
        unsigned int gpio32_clkrst_bypass : 1;
        unsigned int gpio33_clkrst_bypass : 1;
        unsigned int gpio36_clkrst_bypass : 1;
        unsigned int timer17_clkrst_bypass : 1;
        unsigned int timer16_clkrst_bypass : 1;
        unsigned int timer15_clkrst_bypass : 1;
        unsigned int timer14_clkrst_bypass : 1;
        unsigned int timer13_clkrst_bypass : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_SCTRL_SC_IPCLKRST_BYPASS1_UNION;
#endif
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_ao_ipc_clkrst_bypass_START (0)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_ao_ipc_clkrst_bypass_END (0)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_ao_ipc_ns_clkrst_bypass_START (1)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_ao_ipc_ns_clkrst_bypass_END (1)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_gpio21_clkrst_bypass_START (2)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_gpio21_clkrst_bypass_END (2)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_gpio20_clkrst_bypass_START (3)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_gpio20_clkrst_bypass_END (3)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_spi3_clkrst_bypass_START (4)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_spi3_clkrst_bypass_END (4)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_tcp_clkrst_bypass_START (5)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_tcp_clkrst_bypass_END (5)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_ufs_sys_ctrl_clkrst_bypass_START (6)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_ufs_sys_ctrl_clkrst_bypass_END (6)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_ao_wd_clkrst_bypass_START (7)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_ao_wd_clkrst_bypass_END (7)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_gpio31_clkrst_bypass_START (8)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_gpio31_clkrst_bypass_END (8)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_gpio32_clkrst_bypass_START (9)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_gpio32_clkrst_bypass_END (9)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_gpio33_clkrst_bypass_START (10)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_gpio33_clkrst_bypass_END (10)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_gpio36_clkrst_bypass_START (11)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_gpio36_clkrst_bypass_END (11)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_timer17_clkrst_bypass_START (12)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_timer17_clkrst_bypass_END (12)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_timer16_clkrst_bypass_START (13)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_timer16_clkrst_bypass_END (13)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_timer15_clkrst_bypass_START (14)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_timer15_clkrst_bypass_END (14)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_timer14_clkrst_bypass_START (15)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_timer14_clkrst_bypass_END (15)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_timer13_clkrst_bypass_START (16)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_timer13_clkrst_bypass_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_asp_subsys : 3;
        unsigned int sc_div_hifidsp : 6;
        unsigned int div_sci : 2;
        unsigned int reserved : 1;
        unsigned int sel_ao_asp : 1;
        unsigned int sel_pll_ctrl_tp_ao : 2;
        unsigned int extclk_en : 1;
        unsigned int scclkdiv0_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV0_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV0_div_asp_subsys_START (0)
#define SOC_SCTRL_SCCLKDIV0_div_asp_subsys_END (2)
#define SOC_SCTRL_SCCLKDIV0_sc_div_hifidsp_START (3)
#define SOC_SCTRL_SCCLKDIV0_sc_div_hifidsp_END (8)
#define SOC_SCTRL_SCCLKDIV0_div_sci_START (9)
#define SOC_SCTRL_SCCLKDIV0_div_sci_END (10)
#define SOC_SCTRL_SCCLKDIV0_sel_ao_asp_START (12)
#define SOC_SCTRL_SCCLKDIV0_sel_ao_asp_END (12)
#define SOC_SCTRL_SCCLKDIV0_sel_pll_ctrl_tp_ao_START (13)
#define SOC_SCTRL_SCCLKDIV0_sel_pll_ctrl_tp_ao_END (14)
#define SOC_SCTRL_SCCLKDIV0_extclk_en_START (15)
#define SOC_SCTRL_SCCLKDIV0_extclk_en_END (15)
#define SOC_SCTRL_SCCLKDIV0_scclkdiv0_msk_START (16)
#define SOC_SCTRL_SCCLKDIV0_scclkdiv0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_aobus : 6;
        unsigned int sc_gt_clk_mad_spll : 1;
        unsigned int sc_gt_clk_pcie_aux : 1;
        unsigned int div_pcie_aux : 4;
        unsigned int sel_clk_fll_test_src : 3;
        unsigned int sc_gt_clk_hifd_fll : 1;
        unsigned int scclkdiv1_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV1_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV1_div_aobus_START (0)
#define SOC_SCTRL_SCCLKDIV1_div_aobus_END (5)
#define SOC_SCTRL_SCCLKDIV1_sc_gt_clk_mad_spll_START (6)
#define SOC_SCTRL_SCCLKDIV1_sc_gt_clk_mad_spll_END (6)
#define SOC_SCTRL_SCCLKDIV1_sc_gt_clk_pcie_aux_START (7)
#define SOC_SCTRL_SCCLKDIV1_sc_gt_clk_pcie_aux_END (7)
#define SOC_SCTRL_SCCLKDIV1_div_pcie_aux_START (8)
#define SOC_SCTRL_SCCLKDIV1_div_pcie_aux_END (11)
#define SOC_SCTRL_SCCLKDIV1_sel_clk_fll_test_src_START (12)
#define SOC_SCTRL_SCCLKDIV1_sel_clk_fll_test_src_END (14)
#define SOC_SCTRL_SCCLKDIV1_sc_gt_clk_hifd_fll_START (15)
#define SOC_SCTRL_SCCLKDIV1_sc_gt_clk_hifd_fll_END (15)
#define SOC_SCTRL_SCCLKDIV1_scclkdiv1_msk_START (16)
#define SOC_SCTRL_SCCLKDIV1_scclkdiv1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_gt_clk_asp_subsys : 1;
        unsigned int gt_hifidsp_clk_div : 1;
        unsigned int sc_gt_clk_sci : 1;
        unsigned int sc_gt_clk_aobus : 1;
        unsigned int gt_asp_hclk_div : 1;
        unsigned int sc_gt_clk_hifd_pll : 1;
        unsigned int gt_clk_spmi_mst_vote : 10;
        unsigned int scclkdiv2_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV2_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV2_sc_gt_clk_asp_subsys_START (0)
#define SOC_SCTRL_SCCLKDIV2_sc_gt_clk_asp_subsys_END (0)
#define SOC_SCTRL_SCCLKDIV2_gt_hifidsp_clk_div_START (1)
#define SOC_SCTRL_SCCLKDIV2_gt_hifidsp_clk_div_END (1)
#define SOC_SCTRL_SCCLKDIV2_sc_gt_clk_sci_START (2)
#define SOC_SCTRL_SCCLKDIV2_sc_gt_clk_sci_END (2)
#define SOC_SCTRL_SCCLKDIV2_sc_gt_clk_aobus_START (3)
#define SOC_SCTRL_SCCLKDIV2_sc_gt_clk_aobus_END (3)
#define SOC_SCTRL_SCCLKDIV2_gt_asp_hclk_div_START (4)
#define SOC_SCTRL_SCCLKDIV2_gt_asp_hclk_div_END (4)
#define SOC_SCTRL_SCCLKDIV2_sc_gt_clk_hifd_pll_START (5)
#define SOC_SCTRL_SCCLKDIV2_sc_gt_clk_hifd_pll_END (5)
#define SOC_SCTRL_SCCLKDIV2_gt_clk_spmi_mst_vote_START (6)
#define SOC_SCTRL_SCCLKDIV2_gt_clk_spmi_mst_vote_END (15)
#define SOC_SCTRL_SCCLKDIV2_scclkdiv2_msk_START (16)
#define SOC_SCTRL_SCCLKDIV2_scclkdiv2_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aobusnoc_auto_waitcfg_out : 10;
        unsigned int aobusnoc_div_auto_cfg : 6;
        unsigned int scclkdiv3_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV3_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV3_aobusnoc_auto_waitcfg_out_START (0)
#define SOC_SCTRL_SCCLKDIV3_aobusnoc_auto_waitcfg_out_END (9)
#define SOC_SCTRL_SCCLKDIV3_aobusnoc_div_auto_cfg_START (10)
#define SOC_SCTRL_SCCLKDIV3_aobusnoc_div_auto_cfg_END (15)
#define SOC_SCTRL_SCCLKDIV3_scclkdiv3_msk_START (16)
#define SOC_SCTRL_SCCLKDIV3_scclkdiv3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_aobus_noc : 4;
        unsigned int sel_clk_aonoc : 2;
        unsigned int sc_gt_aonoc_pll : 1;
        unsigned int sc_gt_aonoc_fll : 1;
        unsigned int sc_div_asp_hclk : 2;
        unsigned int sc_gt_aonoc_sys : 1;
        unsigned int reserved : 4;
        unsigned int sel_asp_codec : 1;
        unsigned int scclkdiv4_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV4_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV4_div_aobus_noc_START (0)
#define SOC_SCTRL_SCCLKDIV4_div_aobus_noc_END (3)
#define SOC_SCTRL_SCCLKDIV4_sel_clk_aonoc_START (4)
#define SOC_SCTRL_SCCLKDIV4_sel_clk_aonoc_END (5)
#define SOC_SCTRL_SCCLKDIV4_sc_gt_aonoc_pll_START (6)
#define SOC_SCTRL_SCCLKDIV4_sc_gt_aonoc_pll_END (6)
#define SOC_SCTRL_SCCLKDIV4_sc_gt_aonoc_fll_START (7)
#define SOC_SCTRL_SCCLKDIV4_sc_gt_aonoc_fll_END (7)
#define SOC_SCTRL_SCCLKDIV4_sc_div_asp_hclk_START (8)
#define SOC_SCTRL_SCCLKDIV4_sc_div_asp_hclk_END (9)
#define SOC_SCTRL_SCCLKDIV4_sc_gt_aonoc_sys_START (10)
#define SOC_SCTRL_SCCLKDIV4_sc_gt_aonoc_sys_END (10)
#define SOC_SCTRL_SCCLKDIV4_sel_asp_codec_START (15)
#define SOC_SCTRL_SCCLKDIV4_sel_asp_codec_END (15)
#define SOC_SCTRL_SCCLKDIV4_scclkdiv4_msk_START (16)
#define SOC_SCTRL_SCCLKDIV4_scclkdiv4_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aobusnoc_div_auto_reduce_bypass : 1;
        unsigned int aobusnoc_auto_waitcfg_in : 10;
        unsigned int aobus_clk_sw_req_cfg : 2;
        unsigned int sel_syscnt : 1;
        unsigned int sel_clk_pcie_aux : 1;
        unsigned int sc_gt_clk_mad_fll : 1;
        unsigned int scclkdiv5_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV5_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV5_aobusnoc_div_auto_reduce_bypass_START (0)
#define SOC_SCTRL_SCCLKDIV5_aobusnoc_div_auto_reduce_bypass_END (0)
#define SOC_SCTRL_SCCLKDIV5_aobusnoc_auto_waitcfg_in_START (1)
#define SOC_SCTRL_SCCLKDIV5_aobusnoc_auto_waitcfg_in_END (10)
#define SOC_SCTRL_SCCLKDIV5_aobus_clk_sw_req_cfg_START (11)
#define SOC_SCTRL_SCCLKDIV5_aobus_clk_sw_req_cfg_END (12)
#define SOC_SCTRL_SCCLKDIV5_sel_syscnt_START (13)
#define SOC_SCTRL_SCCLKDIV5_sel_syscnt_END (13)
#define SOC_SCTRL_SCCLKDIV5_sel_clk_pcie_aux_START (14)
#define SOC_SCTRL_SCCLKDIV5_sel_clk_pcie_aux_END (14)
#define SOC_SCTRL_SCCLKDIV5_sc_gt_clk_mad_fll_START (15)
#define SOC_SCTRL_SCCLKDIV5_sc_gt_clk_mad_fll_END (15)
#define SOC_SCTRL_SCCLKDIV5_scclkdiv5_msk_START (16)
#define SOC_SCTRL_SCCLKDIV5_scclkdiv5_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_mad_spll : 4;
        unsigned int div_syscnt : 4;
        unsigned int sel_mad_mux_pre : 1;
        unsigned int div_iomcu_fll : 2;
        unsigned int div_aobus_fll : 5;
        unsigned int scclkdiv6_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV6_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV6_div_mad_spll_START (0)
#define SOC_SCTRL_SCCLKDIV6_div_mad_spll_END (3)
#define SOC_SCTRL_SCCLKDIV6_div_syscnt_START (4)
#define SOC_SCTRL_SCCLKDIV6_div_syscnt_END (7)
#define SOC_SCTRL_SCCLKDIV6_sel_mad_mux_pre_START (8)
#define SOC_SCTRL_SCCLKDIV6_sel_mad_mux_pre_END (8)
#define SOC_SCTRL_SCCLKDIV6_div_iomcu_fll_START (9)
#define SOC_SCTRL_SCCLKDIV6_div_iomcu_fll_END (10)
#define SOC_SCTRL_SCCLKDIV6_div_aobus_fll_START (11)
#define SOC_SCTRL_SCCLKDIV6_div_aobus_fll_END (15)
#define SOC_SCTRL_SCCLKDIV6_scclkdiv6_msk_START (16)
#define SOC_SCTRL_SCCLKDIV6_scclkdiv6_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_gt_clk_memrepair : 1;
        unsigned int sel_clk_csi_cfg : 1;
        unsigned int div_memrepair : 6;
        unsigned int sel_ao_loadmonitor : 1;
        unsigned int sc_gt_clk_ao_loadmonitor : 1;
        unsigned int div_ao_loadmonitor : 4;
        unsigned int div_aonoc_fll : 2;
        unsigned int scclkdiv7_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV7_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV7_sc_gt_clk_memrepair_START (0)
#define SOC_SCTRL_SCCLKDIV7_sc_gt_clk_memrepair_END (0)
#define SOC_SCTRL_SCCLKDIV7_sel_clk_csi_cfg_START (1)
#define SOC_SCTRL_SCCLKDIV7_sel_clk_csi_cfg_END (1)
#define SOC_SCTRL_SCCLKDIV7_div_memrepair_START (2)
#define SOC_SCTRL_SCCLKDIV7_div_memrepair_END (7)
#define SOC_SCTRL_SCCLKDIV7_sel_ao_loadmonitor_START (8)
#define SOC_SCTRL_SCCLKDIV7_sel_ao_loadmonitor_END (8)
#define SOC_SCTRL_SCCLKDIV7_sc_gt_clk_ao_loadmonitor_START (9)
#define SOC_SCTRL_SCCLKDIV7_sc_gt_clk_ao_loadmonitor_END (9)
#define SOC_SCTRL_SCCLKDIV7_div_ao_loadmonitor_START (10)
#define SOC_SCTRL_SCCLKDIV7_div_ao_loadmonitor_END (13)
#define SOC_SCTRL_SCCLKDIV7_div_aonoc_fll_START (14)
#define SOC_SCTRL_SCCLKDIV7_div_aonoc_fll_END (15)
#define SOC_SCTRL_SCCLKDIV7_scclkdiv7_msk_START (16)
#define SOC_SCTRL_SCCLKDIV7_scclkdiv7_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_ioperi : 6;
        unsigned int sc_gt_clk_ioperi : 1;
        unsigned int sel_mad_mux : 1;
        unsigned int sc_gt_clk_spmi_mst : 1;
        unsigned int div_spmi_mst : 6;
        unsigned int sel_spmi_mst : 1;
        unsigned int scclkdiv8_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV8_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV8_div_ioperi_START (0)
#define SOC_SCTRL_SCCLKDIV8_div_ioperi_END (5)
#define SOC_SCTRL_SCCLKDIV8_sc_gt_clk_ioperi_START (6)
#define SOC_SCTRL_SCCLKDIV8_sc_gt_clk_ioperi_END (6)
#define SOC_SCTRL_SCCLKDIV8_sel_mad_mux_START (7)
#define SOC_SCTRL_SCCLKDIV8_sel_mad_mux_END (7)
#define SOC_SCTRL_SCCLKDIV8_sc_gt_clk_spmi_mst_START (8)
#define SOC_SCTRL_SCCLKDIV8_sc_gt_clk_spmi_mst_END (8)
#define SOC_SCTRL_SCCLKDIV8_div_spmi_mst_START (9)
#define SOC_SCTRL_SCCLKDIV8_div_spmi_mst_END (14)
#define SOC_SCTRL_SCCLKDIV8_sel_spmi_mst_START (15)
#define SOC_SCTRL_SCCLKDIV8_sel_spmi_mst_END (15)
#define SOC_SCTRL_SCCLKDIV8_scclkdiv8_msk_START (16)
#define SOC_SCTRL_SCCLKDIV8_scclkdiv8_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_ufs_subsys_pll : 6;
        unsigned int div_sys_ufs_subsys : 2;
        unsigned int sc_gt_clk_ufs_subsys_pll : 1;
        unsigned int ufsbus_div_auto_reduce_bypass_acpu : 1;
        unsigned int ufsbus_div_auto_reduce_bypass_mcpu : 1;
        unsigned int ao_wd_soft_en : 1;
        unsigned int ao_wd_en_ov : 1;
        unsigned int reserved : 1;
        unsigned int sc_gt_clk_asp_codec_pll : 1;
        unsigned int sc_gt_clk_asp_subsys_fll : 1;
        unsigned int scclkdiv9_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV9_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV9_div_ufs_subsys_pll_START (0)
#define SOC_SCTRL_SCCLKDIV9_div_ufs_subsys_pll_END (5)
#define SOC_SCTRL_SCCLKDIV9_div_sys_ufs_subsys_START (6)
#define SOC_SCTRL_SCCLKDIV9_div_sys_ufs_subsys_END (7)
#define SOC_SCTRL_SCCLKDIV9_sc_gt_clk_ufs_subsys_pll_START (8)
#define SOC_SCTRL_SCCLKDIV9_sc_gt_clk_ufs_subsys_pll_END (8)
#define SOC_SCTRL_SCCLKDIV9_ufsbus_div_auto_reduce_bypass_acpu_START (9)
#define SOC_SCTRL_SCCLKDIV9_ufsbus_div_auto_reduce_bypass_acpu_END (9)
#define SOC_SCTRL_SCCLKDIV9_ufsbus_div_auto_reduce_bypass_mcpu_START (10)
#define SOC_SCTRL_SCCLKDIV9_ufsbus_div_auto_reduce_bypass_mcpu_END (10)
#define SOC_SCTRL_SCCLKDIV9_ao_wd_soft_en_START (11)
#define SOC_SCTRL_SCCLKDIV9_ao_wd_soft_en_END (11)
#define SOC_SCTRL_SCCLKDIV9_ao_wd_en_ov_START (12)
#define SOC_SCTRL_SCCLKDIV9_ao_wd_en_ov_END (12)
#define SOC_SCTRL_SCCLKDIV9_sc_gt_clk_asp_codec_pll_START (14)
#define SOC_SCTRL_SCCLKDIV9_sc_gt_clk_asp_codec_pll_END (14)
#define SOC_SCTRL_SCCLKDIV9_sc_gt_clk_asp_subsys_fll_START (15)
#define SOC_SCTRL_SCCLKDIV9_sc_gt_clk_asp_subsys_fll_END (15)
#define SOC_SCTRL_SCCLKDIV9_scclkdiv9_msk_START (16)
#define SOC_SCTRL_SCCLKDIV9_scclkdiv9_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ufsbus_div_auto_reduce_bypass_lpm3 : 1;
        unsigned int ufsbus_auto_waitcfg_in : 10;
        unsigned int ufsbus_auto_waitcfg_out : 10;
        unsigned int ufsbus_div_auto_cfg : 6;
        unsigned int ufs_bus_rd_bypass : 1;
        unsigned int ufs_bus_wr_bypass : 1;
        unsigned int ufs_hibernate_bypass : 1;
        unsigned int noc_ufs_cfg_t_mainnopendingtrans_bp : 1;
        unsigned int reserved : 1;
    } reg;
} SOC_SCTRL_SCUFS_AUTODIV_UNION;
#endif
#define SOC_SCTRL_SCUFS_AUTODIV_ufsbus_div_auto_reduce_bypass_lpm3_START (0)
#define SOC_SCTRL_SCUFS_AUTODIV_ufsbus_div_auto_reduce_bypass_lpm3_END (0)
#define SOC_SCTRL_SCUFS_AUTODIV_ufsbus_auto_waitcfg_in_START (1)
#define SOC_SCTRL_SCUFS_AUTODIV_ufsbus_auto_waitcfg_in_END (10)
#define SOC_SCTRL_SCUFS_AUTODIV_ufsbus_auto_waitcfg_out_START (11)
#define SOC_SCTRL_SCUFS_AUTODIV_ufsbus_auto_waitcfg_out_END (20)
#define SOC_SCTRL_SCUFS_AUTODIV_ufsbus_div_auto_cfg_START (21)
#define SOC_SCTRL_SCUFS_AUTODIV_ufsbus_div_auto_cfg_END (26)
#define SOC_SCTRL_SCUFS_AUTODIV_ufs_bus_rd_bypass_START (27)
#define SOC_SCTRL_SCUFS_AUTODIV_ufs_bus_rd_bypass_END (27)
#define SOC_SCTRL_SCUFS_AUTODIV_ufs_bus_wr_bypass_START (28)
#define SOC_SCTRL_SCUFS_AUTODIV_ufs_bus_wr_bypass_END (28)
#define SOC_SCTRL_SCUFS_AUTODIV_ufs_hibernate_bypass_START (29)
#define SOC_SCTRL_SCUFS_AUTODIV_ufs_hibernate_bypass_END (29)
#define SOC_SCTRL_SCUFS_AUTODIV_noc_ufs_cfg_t_mainnopendingtrans_bp_START (30)
#define SOC_SCTRL_SCUFS_AUTODIV_noc_ufs_cfg_t_mainnopendingtrans_bp_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_mask_asp_nonidle_pend : 1;
        unsigned int intr_mask_iomcu_nonidle_pend : 1;
        unsigned int intr_mask_fd_nonidle_pend : 1;
        unsigned int reserved_0 : 5;
        unsigned int intr_clr_asp_nonidle_pend : 1;
        unsigned int intr_clr_iomcu_nonidle_pend : 1;
        unsigned int intr_clr_fd_nonidle_pend : 1;
        unsigned int reserved_1 : 5;
        unsigned int sc_intr_nocbus_ctrl_msk : 16;
    } reg;
} SOC_SCTRL_SC_INTR_NOCBUS_CTRL_UNION;
#endif
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_mask_asp_nonidle_pend_START (0)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_mask_asp_nonidle_pend_END (0)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_mask_iomcu_nonidle_pend_START (1)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_mask_iomcu_nonidle_pend_END (1)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_mask_fd_nonidle_pend_START (2)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_mask_fd_nonidle_pend_END (2)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_clr_asp_nonidle_pend_START (8)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_clr_asp_nonidle_pend_END (8)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_clr_iomcu_nonidle_pend_START (9)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_clr_iomcu_nonidle_pend_END (9)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_clr_fd_nonidle_pend_START (10)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_clr_fd_nonidle_pend_END (10)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_sc_intr_nocbus_ctrl_msk_START (16)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_sc_intr_nocbus_ctrl_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sel_asp_subsys : 4;
        unsigned int div_asp_subsys_fll : 2;
        unsigned int div_noc_timeout_extref : 5;
        unsigned int div_fll_src_tp : 5;
        unsigned int scclkdiv10_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV10_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV10_sel_asp_subsys_START (0)
#define SOC_SCTRL_SCCLKDIV10_sel_asp_subsys_END (3)
#define SOC_SCTRL_SCCLKDIV10_div_asp_subsys_fll_START (4)
#define SOC_SCTRL_SCCLKDIV10_div_asp_subsys_fll_END (5)
#define SOC_SCTRL_SCCLKDIV10_div_noc_timeout_extref_START (6)
#define SOC_SCTRL_SCCLKDIV10_div_noc_timeout_extref_END (10)
#define SOC_SCTRL_SCCLKDIV10_div_fll_src_tp_START (11)
#define SOC_SCTRL_SCCLKDIV10_div_fll_src_tp_END (15)
#define SOC_SCTRL_SCCLKDIV10_scclkdiv10_msk_START (16)
#define SOC_SCTRL_SCCLKDIV10_scclkdiv10_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_hifd_pll : 4;
        unsigned int div_hifd_fll : 2;
        unsigned int sel_clk_out0 : 2;
        unsigned int sc_gt_clk_out0_fll : 1;
        unsigned int sc_gt_clk_out0_pll : 1;
        unsigned int sel_fll_src : 1;
        unsigned int sel_clk_hifd_pll : 1;
        unsigned int div_clk_out0 : 2;
        unsigned int sc_gt_clk_iomcu_fll : 1;
        unsigned int sc_gt_clk_iomcu_pll : 1;
        unsigned int scclkdiv11_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV11_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV11_div_hifd_pll_START (0)
#define SOC_SCTRL_SCCLKDIV11_div_hifd_pll_END (3)
#define SOC_SCTRL_SCCLKDIV11_div_hifd_fll_START (4)
#define SOC_SCTRL_SCCLKDIV11_div_hifd_fll_END (5)
#define SOC_SCTRL_SCCLKDIV11_sel_clk_out0_START (6)
#define SOC_SCTRL_SCCLKDIV11_sel_clk_out0_END (7)
#define SOC_SCTRL_SCCLKDIV11_sc_gt_clk_out0_fll_START (8)
#define SOC_SCTRL_SCCLKDIV11_sc_gt_clk_out0_fll_END (8)
#define SOC_SCTRL_SCCLKDIV11_sc_gt_clk_out0_pll_START (9)
#define SOC_SCTRL_SCCLKDIV11_sc_gt_clk_out0_pll_END (9)
#define SOC_SCTRL_SCCLKDIV11_sel_fll_src_START (10)
#define SOC_SCTRL_SCCLKDIV11_sel_fll_src_END (10)
#define SOC_SCTRL_SCCLKDIV11_sel_clk_hifd_pll_START (11)
#define SOC_SCTRL_SCCLKDIV11_sel_clk_hifd_pll_END (11)
#define SOC_SCTRL_SCCLKDIV11_div_clk_out0_START (12)
#define SOC_SCTRL_SCCLKDIV11_div_clk_out0_END (13)
#define SOC_SCTRL_SCCLKDIV11_sc_gt_clk_iomcu_fll_START (14)
#define SOC_SCTRL_SCCLKDIV11_sc_gt_clk_iomcu_fll_END (14)
#define SOC_SCTRL_SCCLKDIV11_sc_gt_clk_iomcu_pll_START (15)
#define SOC_SCTRL_SCCLKDIV11_sc_gt_clk_iomcu_pll_END (15)
#define SOC_SCTRL_SCCLKDIV11_scclkdiv11_msk_START (16)
#define SOC_SCTRL_SCCLKDIV11_scclkdiv11_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_noc_timeout_extref_peri_bypass : 1;
        unsigned int gt_timerclk_refh_peri_bypass : 1;
        unsigned int gt_clkin_sys_peri_bypass : 1;
        unsigned int gt_clkin_ref_peri_bypass : 1;
        unsigned int gt_clk_sys_ini_peri_bypass : 1;
        unsigned int gt_clk_spll_peri_bypass : 1;
        unsigned int gt_clk_ref_peri_bypass : 1;
        unsigned int gt_clk_sys_peri_bypass : 1;
        unsigned int gt_clk_aobus_peri_bypass : 1;
        unsigned int gt_clk_fll_src_peri_bypass : 1;
        unsigned int gt_clk_ref_gpio_peri_bypass : 1;
        unsigned int hsdt_subsys_clk_sw_req_cfg : 2;
        unsigned int div_clkout1_tcxo : 3;
        unsigned int scclkdiv12_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV12_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV12_gt_clk_noc_timeout_extref_peri_bypass_START (0)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_noc_timeout_extref_peri_bypass_END (0)
#define SOC_SCTRL_SCCLKDIV12_gt_timerclk_refh_peri_bypass_START (1)
#define SOC_SCTRL_SCCLKDIV12_gt_timerclk_refh_peri_bypass_END (1)
#define SOC_SCTRL_SCCLKDIV12_gt_clkin_sys_peri_bypass_START (2)
#define SOC_SCTRL_SCCLKDIV12_gt_clkin_sys_peri_bypass_END (2)
#define SOC_SCTRL_SCCLKDIV12_gt_clkin_ref_peri_bypass_START (3)
#define SOC_SCTRL_SCCLKDIV12_gt_clkin_ref_peri_bypass_END (3)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_sys_ini_peri_bypass_START (4)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_sys_ini_peri_bypass_END (4)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_spll_peri_bypass_START (5)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_spll_peri_bypass_END (5)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_ref_peri_bypass_START (6)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_ref_peri_bypass_END (6)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_sys_peri_bypass_START (7)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_sys_peri_bypass_END (7)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_aobus_peri_bypass_START (8)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_aobus_peri_bypass_END (8)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_fll_src_peri_bypass_START (9)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_fll_src_peri_bypass_END (9)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_ref_gpio_peri_bypass_START (10)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_ref_gpio_peri_bypass_END (10)
#define SOC_SCTRL_SCCLKDIV12_hsdt_subsys_clk_sw_req_cfg_START (11)
#define SOC_SCTRL_SCCLKDIV12_hsdt_subsys_clk_sw_req_cfg_END (12)
#define SOC_SCTRL_SCCLKDIV12_div_clkout1_tcxo_START (13)
#define SOC_SCTRL_SCCLKDIV12_div_clkout1_tcxo_END (15)
#define SOC_SCTRL_SCCLKDIV12_scclkdiv12_msk_START (16)
#define SOC_SCTRL_SCCLKDIV12_scclkdiv12_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_noc_timeout_extref_peri_bypass_iomcu : 1;
        unsigned int gt_timerclk_refh_peri_bypass_iomcu : 1;
        unsigned int gt_clkin_sys_peri_bypass_iomcu : 1;
        unsigned int gt_clkin_ref_peri_bypass_iomcu : 1;
        unsigned int gt_clk_sys_ini_peri_bypass_iomcu : 1;
        unsigned int gt_clk_spll_peri_bypass_iomcu : 1;
        unsigned int gt_clk_ref_peri_bypass_iomcu : 1;
        unsigned int gt_clk_sys_peri_bypass_iomcu : 1;
        unsigned int gt_clk_aobus_peri_bypass_iomcu : 1;
        unsigned int gt_clk_fll_src_peri_bypass_iomcu : 1;
        unsigned int gt_clk_ref_gpio_peri_bypass_iomcu : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int sleep_mode_spi3_bypass : 1;
        unsigned int sleep_mode_ufs_bypass : 1;
        unsigned int sleep_mode_cssys_bypass : 1;
        unsigned int scclkdiv13_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV13_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV13_gt_clk_noc_timeout_extref_peri_bypass_iomcu_START (0)
#define SOC_SCTRL_SCCLKDIV13_gt_clk_noc_timeout_extref_peri_bypass_iomcu_END (0)
#define SOC_SCTRL_SCCLKDIV13_gt_timerclk_refh_peri_bypass_iomcu_START (1)
#define SOC_SCTRL_SCCLKDIV13_gt_timerclk_refh_peri_bypass_iomcu_END (1)
#define SOC_SCTRL_SCCLKDIV13_gt_clkin_sys_peri_bypass_iomcu_START (2)
#define SOC_SCTRL_SCCLKDIV13_gt_clkin_sys_peri_bypass_iomcu_END (2)
#define SOC_SCTRL_SCCLKDIV13_gt_clkin_ref_peri_bypass_iomcu_START (3)
#define SOC_SCTRL_SCCLKDIV13_gt_clkin_ref_peri_bypass_iomcu_END (3)
#define SOC_SCTRL_SCCLKDIV13_gt_clk_sys_ini_peri_bypass_iomcu_START (4)
#define SOC_SCTRL_SCCLKDIV13_gt_clk_sys_ini_peri_bypass_iomcu_END (4)
#define SOC_SCTRL_SCCLKDIV13_gt_clk_spll_peri_bypass_iomcu_START (5)
#define SOC_SCTRL_SCCLKDIV13_gt_clk_spll_peri_bypass_iomcu_END (5)
#define SOC_SCTRL_SCCLKDIV13_gt_clk_ref_peri_bypass_iomcu_START (6)
#define SOC_SCTRL_SCCLKDIV13_gt_clk_ref_peri_bypass_iomcu_END (6)
#define SOC_SCTRL_SCCLKDIV13_gt_clk_sys_peri_bypass_iomcu_START (7)
#define SOC_SCTRL_SCCLKDIV13_gt_clk_sys_peri_bypass_iomcu_END (7)
#define SOC_SCTRL_SCCLKDIV13_gt_clk_aobus_peri_bypass_iomcu_START (8)
#define SOC_SCTRL_SCCLKDIV13_gt_clk_aobus_peri_bypass_iomcu_END (8)
#define SOC_SCTRL_SCCLKDIV13_gt_clk_fll_src_peri_bypass_iomcu_START (9)
#define SOC_SCTRL_SCCLKDIV13_gt_clk_fll_src_peri_bypass_iomcu_END (9)
#define SOC_SCTRL_SCCLKDIV13_gt_clk_ref_gpio_peri_bypass_iomcu_START (10)
#define SOC_SCTRL_SCCLKDIV13_gt_clk_ref_gpio_peri_bypass_iomcu_END (10)
#define SOC_SCTRL_SCCLKDIV13_sleep_mode_spi3_bypass_START (13)
#define SOC_SCTRL_SCCLKDIV13_sleep_mode_spi3_bypass_END (13)
#define SOC_SCTRL_SCCLKDIV13_sleep_mode_ufs_bypass_START (14)
#define SOC_SCTRL_SCCLKDIV13_sleep_mode_ufs_bypass_END (14)
#define SOC_SCTRL_SCCLKDIV13_sleep_mode_cssys_bypass_START (15)
#define SOC_SCTRL_SCCLKDIV13_sleep_mode_cssys_bypass_END (15)
#define SOC_SCTRL_SCCLKDIV13_scclkdiv13_msk_START (16)
#define SOC_SCTRL_SCCLKDIV13_scclkdiv13_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_memrepair_fll : 4;
        unsigned int sel_clk_ao_camera : 2;
        unsigned int reserved : 2;
        unsigned int div_ao_camera : 4;
        unsigned int sel_memrepair : 2;
        unsigned int sc_gt_clk_ao_camera : 1;
        unsigned int sc_gt_clk_memrepair_fll_div : 1;
        unsigned int scclkdiv14_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV14_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV14_div_memrepair_fll_START (0)
#define SOC_SCTRL_SCCLKDIV14_div_memrepair_fll_END (3)
#define SOC_SCTRL_SCCLKDIV14_sel_clk_ao_camera_START (4)
#define SOC_SCTRL_SCCLKDIV14_sel_clk_ao_camera_END (5)
#define SOC_SCTRL_SCCLKDIV14_div_ao_camera_START (8)
#define SOC_SCTRL_SCCLKDIV14_div_ao_camera_END (11)
#define SOC_SCTRL_SCCLKDIV14_sel_memrepair_START (12)
#define SOC_SCTRL_SCCLKDIV14_sel_memrepair_END (13)
#define SOC_SCTRL_SCCLKDIV14_sc_gt_clk_ao_camera_START (14)
#define SOC_SCTRL_SCCLKDIV14_sc_gt_clk_ao_camera_END (14)
#define SOC_SCTRL_SCCLKDIV14_sc_gt_clk_memrepair_fll_div_START (15)
#define SOC_SCTRL_SCCLKDIV14_sc_gt_clk_memrepair_fll_div_END (15)
#define SOC_SCTRL_SCCLKDIV14_scclkdiv14_msk_START (16)
#define SOC_SCTRL_SCCLKDIV14_scclkdiv14_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_mad_fll : 5;
        unsigned int sc_gt_clk_out1_div1 : 1;
        unsigned int sel_clk_out1 : 2;
        unsigned int div_csi_cfg : 6;
        unsigned int sel_ioperi : 1;
        unsigned int sc_gt_clk_csi_cfg : 1;
        unsigned int scclkdiv15_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV15_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV15_div_mad_fll_START (0)
#define SOC_SCTRL_SCCLKDIV15_div_mad_fll_END (4)
#define SOC_SCTRL_SCCLKDIV15_sc_gt_clk_out1_div1_START (5)
#define SOC_SCTRL_SCCLKDIV15_sc_gt_clk_out1_div1_END (5)
#define SOC_SCTRL_SCCLKDIV15_sel_clk_out1_START (6)
#define SOC_SCTRL_SCCLKDIV15_sel_clk_out1_END (7)
#define SOC_SCTRL_SCCLKDIV15_div_csi_cfg_START (8)
#define SOC_SCTRL_SCCLKDIV15_div_csi_cfg_END (13)
#define SOC_SCTRL_SCCLKDIV15_sel_ioperi_START (14)
#define SOC_SCTRL_SCCLKDIV15_sel_ioperi_END (14)
#define SOC_SCTRL_SCCLKDIV15_sc_gt_clk_csi_cfg_START (15)
#define SOC_SCTRL_SCCLKDIV15_sc_gt_clk_csi_cfg_END (15)
#define SOC_SCTRL_SCCLKDIV15_scclkdiv15_msk_START (16)
#define SOC_SCTRL_SCCLKDIV15_scclkdiv15_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_clkout1_pll : 6;
        unsigned int div_asp_codec : 6;
        unsigned int reserved : 4;
        unsigned int scclkdiv16_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV16_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV16_div_clkout1_pll_START (0)
#define SOC_SCTRL_SCCLKDIV16_div_clkout1_pll_END (5)
#define SOC_SCTRL_SCCLKDIV16_div_asp_codec_START (6)
#define SOC_SCTRL_SCCLKDIV16_div_asp_codec_END (11)
#define SOC_SCTRL_SCCLKDIV16_scclkdiv16_msk_START (16)
#define SOC_SCTRL_SCCLKDIV16_scclkdiv16_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_efusec_mem_ctrl : 26;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 5;
    } reg;
} SOC_SCTRL_SCPERCTRL0_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL0_sc_efusec_mem_ctrl_START (0)
#define SOC_SCTRL_SCPERCTRL0_sc_efusec_mem_ctrl_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int repair_disable_31to0 : 32;
    } reg;
} SOC_SCTRL_SCPERCTRL1_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL1_repair_disable_31to0_START (0)
#define SOC_SCTRL_SCPERCTRL1_repair_disable_31to0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_spi_mem_ctrl : 16;
        unsigned int cmp_cc712_resume_kcst : 1;
        unsigned int cmp_cc712_resume_kfde0 : 1;
        unsigned int cmp_cc712_resume_kfde1 : 1;
        unsigned int cmp_cc712_resume_kfde2 : 1;
        unsigned int cmp_cc712_resume_kfde3 : 1;
        unsigned int reserved_0 : 4;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
    } reg;
} SOC_SCTRL_SCPERCTRL2_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL2_sc_spi_mem_ctrl_START (0)
#define SOC_SCTRL_SCPERCTRL2_sc_spi_mem_ctrl_END (15)
#define SOC_SCTRL_SCPERCTRL2_cmp_cc712_resume_kcst_START (16)
#define SOC_SCTRL_SCPERCTRL2_cmp_cc712_resume_kcst_END (16)
#define SOC_SCTRL_SCPERCTRL2_cmp_cc712_resume_kfde0_START (17)
#define SOC_SCTRL_SCPERCTRL2_cmp_cc712_resume_kfde0_END (17)
#define SOC_SCTRL_SCPERCTRL2_cmp_cc712_resume_kfde1_START (18)
#define SOC_SCTRL_SCPERCTRL2_cmp_cc712_resume_kfde1_END (18)
#define SOC_SCTRL_SCPERCTRL2_cmp_cc712_resume_kfde2_START (19)
#define SOC_SCTRL_SCPERCTRL2_cmp_cc712_resume_kfde2_END (19)
#define SOC_SCTRL_SCPERCTRL2_cmp_cc712_resume_kfde3_START (20)
#define SOC_SCTRL_SCPERCTRL2_cmp_cc712_resume_kfde3_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int repair_hs_sw : 1;
        unsigned int repair_iddq : 1;
        unsigned int repair_hs_sw_1 : 1;
        unsigned int repair_iddq_1 : 1;
        unsigned int reserved_1 : 3;
        unsigned int efusec_timeout_bypass : 1;
        unsigned int bbdrx_timeout_bypass : 1;
        unsigned int sleep_mode_cfgbus_bypass : 1;
        unsigned int sleep_mode_lpmcu_bypass : 1;
        unsigned int first_power_on_flag : 1;
        unsigned int sleep_mode_aobus_bypass : 1;
        unsigned int bbp_clk_en : 1;
        unsigned int reserved_2 : 1;
        unsigned int scperctrl3_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL3_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL3_repair_hs_sw_START (1)
#define SOC_SCTRL_SCPERCTRL3_repair_hs_sw_END (1)
#define SOC_SCTRL_SCPERCTRL3_repair_iddq_START (2)
#define SOC_SCTRL_SCPERCTRL3_repair_iddq_END (2)
#define SOC_SCTRL_SCPERCTRL3_repair_hs_sw_1_START (3)
#define SOC_SCTRL_SCPERCTRL3_repair_hs_sw_1_END (3)
#define SOC_SCTRL_SCPERCTRL3_repair_iddq_1_START (4)
#define SOC_SCTRL_SCPERCTRL3_repair_iddq_1_END (4)
#define SOC_SCTRL_SCPERCTRL3_efusec_timeout_bypass_START (8)
#define SOC_SCTRL_SCPERCTRL3_efusec_timeout_bypass_END (8)
#define SOC_SCTRL_SCPERCTRL3_bbdrx_timeout_bypass_START (9)
#define SOC_SCTRL_SCPERCTRL3_bbdrx_timeout_bypass_END (9)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_cfgbus_bypass_START (10)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_cfgbus_bypass_END (10)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_lpmcu_bypass_START (11)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_lpmcu_bypass_END (11)
#define SOC_SCTRL_SCPERCTRL3_first_power_on_flag_START (12)
#define SOC_SCTRL_SCPERCTRL3_first_power_on_flag_END (12)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_aobus_bypass_START (13)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_aobus_bypass_END (13)
#define SOC_SCTRL_SCPERCTRL3_bbp_clk_en_START (14)
#define SOC_SCTRL_SCPERCTRL3_bbp_clk_en_END (14)
#define SOC_SCTRL_SCPERCTRL3_scperctrl3_msk_START (16)
#define SOC_SCTRL_SCPERCTRL3_scperctrl3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddr_cha_test_ctrl : 1;
        unsigned int ddr_chb_test_ctrl : 1;
        unsigned int ddr_chc_test_ctrl : 1;
        unsigned int ddr_chd_test_ctrl : 1;
        unsigned int xo_mode_a2d : 1;
        unsigned int reserved_0 : 3;
        unsigned int crc_value : 21;
        unsigned int reserved_1 : 3;
    } reg;
} SOC_SCTRL_SCPERCTRL4_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL4_ddr_cha_test_ctrl_START (0)
#define SOC_SCTRL_SCPERCTRL4_ddr_cha_test_ctrl_END (0)
#define SOC_SCTRL_SCPERCTRL4_ddr_chb_test_ctrl_START (1)
#define SOC_SCTRL_SCPERCTRL4_ddr_chb_test_ctrl_END (1)
#define SOC_SCTRL_SCPERCTRL4_ddr_chc_test_ctrl_START (2)
#define SOC_SCTRL_SCPERCTRL4_ddr_chc_test_ctrl_END (2)
#define SOC_SCTRL_SCPERCTRL4_ddr_chd_test_ctrl_START (3)
#define SOC_SCTRL_SCPERCTRL4_ddr_chd_test_ctrl_END (3)
#define SOC_SCTRL_SCPERCTRL4_xo_mode_a2d_START (4)
#define SOC_SCTRL_SCPERCTRL4_xo_mode_a2d_END (4)
#define SOC_SCTRL_SCPERCTRL4_crc_value_START (8)
#define SOC_SCTRL_SCPERCTRL4_crc_value_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 3;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int tp_sel : 4;
        unsigned int ao_hpm_clk_div : 6;
        unsigned int ao_hpm_en : 1;
        unsigned int ao_hpmx_en : 1;
        unsigned int sc_noc_timeout_en : 1;
        unsigned int reserved_6 : 11;
    } reg;
} SOC_SCTRL_SCPERCTRL5_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL5_tp_sel_START (8)
#define SOC_SCTRL_SCPERCTRL5_tp_sel_END (11)
#define SOC_SCTRL_SCPERCTRL5_ao_hpm_clk_div_START (12)
#define SOC_SCTRL_SCPERCTRL5_ao_hpm_clk_div_END (17)
#define SOC_SCTRL_SCPERCTRL5_ao_hpm_en_START (18)
#define SOC_SCTRL_SCPERCTRL5_ao_hpm_en_END (18)
#define SOC_SCTRL_SCPERCTRL5_ao_hpmx_en_START (19)
#define SOC_SCTRL_SCPERCTRL5_ao_hpmx_en_END (19)
#define SOC_SCTRL_SCPERCTRL5_sc_noc_timeout_en_START (20)
#define SOC_SCTRL_SCPERCTRL5_sc_noc_timeout_en_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_io_ret : 1;
        unsigned int sc_noc_aobus_idle_flag : 1;
        unsigned int efuse_arm_dbg_ctrl : 2;
        unsigned int peri_bootio_ret : 1;
        unsigned int peri_bootio_ret_mode : 1;
        unsigned int sel_clk_aobus_noc_comb : 2;
        unsigned int sel_clk_mmbuf_comb : 2;
        unsigned int reserved_0 : 2;
        unsigned int peri_bootio_ret_hw_enter_bypass : 1;
        unsigned int reserved_1 : 3;
        unsigned int scperctrl6_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL6_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL6_peri_io_ret_START (0)
#define SOC_SCTRL_SCPERCTRL6_peri_io_ret_END (0)
#define SOC_SCTRL_SCPERCTRL6_sc_noc_aobus_idle_flag_START (1)
#define SOC_SCTRL_SCPERCTRL6_sc_noc_aobus_idle_flag_END (1)
#define SOC_SCTRL_SCPERCTRL6_efuse_arm_dbg_ctrl_START (2)
#define SOC_SCTRL_SCPERCTRL6_efuse_arm_dbg_ctrl_END (3)
#define SOC_SCTRL_SCPERCTRL6_peri_bootio_ret_START (4)
#define SOC_SCTRL_SCPERCTRL6_peri_bootio_ret_END (4)
#define SOC_SCTRL_SCPERCTRL6_peri_bootio_ret_mode_START (5)
#define SOC_SCTRL_SCPERCTRL6_peri_bootio_ret_mode_END (5)
#define SOC_SCTRL_SCPERCTRL6_sel_clk_aobus_noc_comb_START (6)
#define SOC_SCTRL_SCPERCTRL6_sel_clk_aobus_noc_comb_END (7)
#define SOC_SCTRL_SCPERCTRL6_sel_clk_mmbuf_comb_START (8)
#define SOC_SCTRL_SCPERCTRL6_sel_clk_mmbuf_comb_END (9)
#define SOC_SCTRL_SCPERCTRL6_peri_bootio_ret_hw_enter_bypass_START (12)
#define SOC_SCTRL_SCPERCTRL6_peri_bootio_ret_hw_enter_bypass_END (12)
#define SOC_SCTRL_SCPERCTRL6_scperctrl6_msk_START (16)
#define SOC_SCTRL_SCPERCTRL6_scperctrl6_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sctrl2lm_hard_volt : 1;
        unsigned int noc_fd_power_idlereq : 1;
        unsigned int noc_asp_power_idlereq : 1;
        unsigned int iomcu_power_idlereq : 1;
        unsigned int usim0_io_sel18 : 1;
        unsigned int usim1_io_sel18 : 1;
        unsigned int sdcard_io_sel18 : 1;
        unsigned int usim_od_en : 1;
        unsigned int sel_suspend_ao2syscache : 1;
        unsigned int tcp_hresp_ctrl : 1;
        unsigned int sleep_mode_to_peri_bypass : 1;
        unsigned int sleep_mode_to_hsdt_bypass : 1;
        unsigned int sleep_mode_to_core_crg_bypass : 1;
        unsigned int noc_sysbus_power_idlereq : 1;
        unsigned int sc_peri_io_ret_from_sctrl : 1;
        unsigned int pmu_powerhold_protect : 1;
        unsigned int scperctrl7_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL7_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL7_sctrl2lm_hard_volt_START (0)
#define SOC_SCTRL_SCPERCTRL7_sctrl2lm_hard_volt_END (0)
#define SOC_SCTRL_SCPERCTRL7_noc_fd_power_idlereq_START (1)
#define SOC_SCTRL_SCPERCTRL7_noc_fd_power_idlereq_END (1)
#define SOC_SCTRL_SCPERCTRL7_noc_asp_power_idlereq_START (2)
#define SOC_SCTRL_SCPERCTRL7_noc_asp_power_idlereq_END (2)
#define SOC_SCTRL_SCPERCTRL7_iomcu_power_idlereq_START (3)
#define SOC_SCTRL_SCPERCTRL7_iomcu_power_idlereq_END (3)
#define SOC_SCTRL_SCPERCTRL7_usim0_io_sel18_START (4)
#define SOC_SCTRL_SCPERCTRL7_usim0_io_sel18_END (4)
#define SOC_SCTRL_SCPERCTRL7_usim1_io_sel18_START (5)
#define SOC_SCTRL_SCPERCTRL7_usim1_io_sel18_END (5)
#define SOC_SCTRL_SCPERCTRL7_sdcard_io_sel18_START (6)
#define SOC_SCTRL_SCPERCTRL7_sdcard_io_sel18_END (6)
#define SOC_SCTRL_SCPERCTRL7_usim_od_en_START (7)
#define SOC_SCTRL_SCPERCTRL7_usim_od_en_END (7)
#define SOC_SCTRL_SCPERCTRL7_sel_suspend_ao2syscache_START (8)
#define SOC_SCTRL_SCPERCTRL7_sel_suspend_ao2syscache_END (8)
#define SOC_SCTRL_SCPERCTRL7_tcp_hresp_ctrl_START (9)
#define SOC_SCTRL_SCPERCTRL7_tcp_hresp_ctrl_END (9)
#define SOC_SCTRL_SCPERCTRL7_sleep_mode_to_peri_bypass_START (10)
#define SOC_SCTRL_SCPERCTRL7_sleep_mode_to_peri_bypass_END (10)
#define SOC_SCTRL_SCPERCTRL7_sleep_mode_to_hsdt_bypass_START (11)
#define SOC_SCTRL_SCPERCTRL7_sleep_mode_to_hsdt_bypass_END (11)
#define SOC_SCTRL_SCPERCTRL7_sleep_mode_to_core_crg_bypass_START (12)
#define SOC_SCTRL_SCPERCTRL7_sleep_mode_to_core_crg_bypass_END (12)
#define SOC_SCTRL_SCPERCTRL7_noc_sysbus_power_idlereq_START (13)
#define SOC_SCTRL_SCPERCTRL7_noc_sysbus_power_idlereq_END (13)
#define SOC_SCTRL_SCPERCTRL7_sc_peri_io_ret_from_sctrl_START (14)
#define SOC_SCTRL_SCPERCTRL7_sc_peri_io_ret_from_sctrl_END (14)
#define SOC_SCTRL_SCPERCTRL7_pmu_powerhold_protect_START (15)
#define SOC_SCTRL_SCPERCTRL7_pmu_powerhold_protect_END (15)
#define SOC_SCTRL_SCPERCTRL7_scperctrl7_msk_START (16)
#define SOC_SCTRL_SCPERCTRL7_scperctrl7_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mdm_ret_ctrl_0 : 1;
        unsigned int mdm_ret_ctrl_1 : 1;
        unsigned int mdm_ret_ctrl_2 : 1;
        unsigned int mdm_ret_ctrl_3 : 1;
        unsigned int mdm_ret_ctrl_4 : 1;
        unsigned int mdm_ret_ctrl_5 : 1;
        unsigned int mdm_ret_ctrl_6 : 1;
        unsigned int mdm_ret_ctrl_7 : 1;
        unsigned int mdm_ret_ctrl_8 : 1;
        unsigned int mdm_ret_ctrl_9 : 1;
        unsigned int mdm_ret_ctrl_10 : 1;
        unsigned int mdm_ret_ctrl_11 : 1;
        unsigned int mdm_ret_ctrl_12 : 1;
        unsigned int mdm_ret_ctrl_13 : 1;
        unsigned int mdm_ret_ctrl_14 : 1;
        unsigned int mdm_ret_ctrl_15 : 1;
        unsigned int scperctrl8_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL8_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_0_START (0)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_0_END (0)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_1_START (1)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_1_END (1)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_2_START (2)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_2_END (2)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_3_START (3)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_3_END (3)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_4_START (4)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_4_END (4)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_5_START (5)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_5_END (5)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_6_START (6)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_6_END (6)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_7_START (7)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_7_END (7)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_8_START (8)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_8_END (8)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_9_START (9)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_9_END (9)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_10_START (10)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_10_END (10)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_11_START (11)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_11_END (11)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_12_START (12)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_12_END (12)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_13_START (13)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_13_END (13)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_14_START (14)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_14_END (14)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_15_START (15)
#define SOC_SCTRL_SCPERCTRL8_mdm_ret_ctrl_15_END (15)
#define SOC_SCTRL_SCPERCTRL8_scperctrl8_msk_START (16)
#define SOC_SCTRL_SCPERCTRL8_scperctrl8_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mdm_ret_ctrl_16 : 1;
        unsigned int mdm_ret_ctrl_17 : 1;
        unsigned int mdm_ret_ctrl_18 : 1;
        unsigned int mdm_ret_ctrl_19 : 1;
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
        unsigned int scperctrl8_add_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL8_ADD_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL8_ADD_mdm_ret_ctrl_16_START (0)
#define SOC_SCTRL_SCPERCTRL8_ADD_mdm_ret_ctrl_16_END (0)
#define SOC_SCTRL_SCPERCTRL8_ADD_mdm_ret_ctrl_17_START (1)
#define SOC_SCTRL_SCPERCTRL8_ADD_mdm_ret_ctrl_17_END (1)
#define SOC_SCTRL_SCPERCTRL8_ADD_mdm_ret_ctrl_18_START (2)
#define SOC_SCTRL_SCPERCTRL8_ADD_mdm_ret_ctrl_18_END (2)
#define SOC_SCTRL_SCPERCTRL8_ADD_mdm_ret_ctrl_19_START (3)
#define SOC_SCTRL_SCPERCTRL8_ADD_mdm_ret_ctrl_19_END (3)
#define SOC_SCTRL_SCPERCTRL8_ADD_scperctrl8_add_msk_START (16)
#define SOC_SCTRL_SCPERCTRL8_ADD_scperctrl8_add_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rst_apb_bbp_on : 1;
        unsigned int rst_19m_bbp_on : 1;
        unsigned int rst_32k_bbp_on : 1;
        unsigned int rst_104m_gbbp1_on : 1;
        unsigned int rst_104m_gbbp2_on : 1;
        unsigned int rst_32k_timer_on : 1;
        unsigned int rst_ahb_bbp_on_n : 1;
        unsigned int bbp_on_ctrl : 25;
    } reg;
} SOC_SCTRL_SCPERCTRL9_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL9_rst_apb_bbp_on_START (0)
#define SOC_SCTRL_SCPERCTRL9_rst_apb_bbp_on_END (0)
#define SOC_SCTRL_SCPERCTRL9_rst_19m_bbp_on_START (1)
#define SOC_SCTRL_SCPERCTRL9_rst_19m_bbp_on_END (1)
#define SOC_SCTRL_SCPERCTRL9_rst_32k_bbp_on_START (2)
#define SOC_SCTRL_SCPERCTRL9_rst_32k_bbp_on_END (2)
#define SOC_SCTRL_SCPERCTRL9_rst_104m_gbbp1_on_START (3)
#define SOC_SCTRL_SCPERCTRL9_rst_104m_gbbp1_on_END (3)
#define SOC_SCTRL_SCPERCTRL9_rst_104m_gbbp2_on_START (4)
#define SOC_SCTRL_SCPERCTRL9_rst_104m_gbbp2_on_END (4)
#define SOC_SCTRL_SCPERCTRL9_rst_32k_timer_on_START (5)
#define SOC_SCTRL_SCPERCTRL9_rst_32k_timer_on_END (5)
#define SOC_SCTRL_SCPERCTRL9_rst_ahb_bbp_on_n_START (6)
#define SOC_SCTRL_SCPERCTRL9_rst_ahb_bbp_on_n_END (6)
#define SOC_SCTRL_SCPERCTRL9_bbp_on_ctrl_START (7)
#define SOC_SCTRL_SCPERCTRL9_bbp_on_ctrl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddra_acctl_ioctrl_henl_in : 2;
        unsigned int ddra_pll_en_lat : 1;
        unsigned int ddrb_pll_en_lat : 1;
        unsigned int reserved_0 : 4;
        unsigned int ddrb_acctl_ioctrl_henl_in : 2;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 4;
        unsigned int reserved_4 : 2;
        unsigned int reserved_5 : 2;
        unsigned int reserved_6 : 4;
        unsigned int reserved_7 : 2;
        unsigned int reserved_8 : 2;
        unsigned int reserved_9 : 4;
    } reg;
} SOC_SCTRL_SCPERCTRL10_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL10_ddra_acctl_ioctrl_henl_in_START (0)
#define SOC_SCTRL_SCPERCTRL10_ddra_acctl_ioctrl_henl_in_END (1)
#define SOC_SCTRL_SCPERCTRL10_ddra_pll_en_lat_START (2)
#define SOC_SCTRL_SCPERCTRL10_ddra_pll_en_lat_END (2)
#define SOC_SCTRL_SCPERCTRL10_ddrb_pll_en_lat_START (3)
#define SOC_SCTRL_SCPERCTRL10_ddrb_pll_en_lat_END (3)
#define SOC_SCTRL_SCPERCTRL10_ddrb_acctl_ioctrl_henl_in_START (8)
#define SOC_SCTRL_SCPERCTRL10_ddrb_acctl_ioctrl_henl_in_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bbpdrx_hresp_ctrl : 1;
        unsigned int bbpon_ahb_buffable_en : 1;
        unsigned int bisr_rom_mem_ctrl : 8;
        unsigned int bisr_dbg_sel_1 : 4;
        unsigned int bisr_dbg_sel_0 : 4;
        unsigned int intr_repair_err_msk_1 : 1;
        unsigned int intr_repair_err_msk_0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 11;
    } reg;
} SOC_SCTRL_SCPERCTRL11_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL11_bbpdrx_hresp_ctrl_START (0)
#define SOC_SCTRL_SCPERCTRL11_bbpdrx_hresp_ctrl_END (0)
#define SOC_SCTRL_SCPERCTRL11_bbpon_ahb_buffable_en_START (1)
#define SOC_SCTRL_SCPERCTRL11_bbpon_ahb_buffable_en_END (1)
#define SOC_SCTRL_SCPERCTRL11_bisr_rom_mem_ctrl_START (2)
#define SOC_SCTRL_SCPERCTRL11_bisr_rom_mem_ctrl_END (9)
#define SOC_SCTRL_SCPERCTRL11_bisr_dbg_sel_1_START (10)
#define SOC_SCTRL_SCPERCTRL11_bisr_dbg_sel_1_END (13)
#define SOC_SCTRL_SCPERCTRL11_bisr_dbg_sel_0_START (14)
#define SOC_SCTRL_SCPERCTRL11_bisr_dbg_sel_0_END (17)
#define SOC_SCTRL_SCPERCTRL11_intr_repair_err_msk_1_START (18)
#define SOC_SCTRL_SCPERCTRL11_intr_repair_err_msk_1_END (18)
#define SOC_SCTRL_SCPERCTRL11_intr_repair_err_msk_0_START (19)
#define SOC_SCTRL_SCPERCTRL11_intr_repair_err_msk_0_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bisr_spram_mem_ctrl : 26;
        unsigned int reserved : 6;
    } reg;
} SOC_SCTRL_SCPERCTRL12_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL12_bisr_spram_mem_ctrl_START (0)
#define SOC_SCTRL_SCPERCTRL12_bisr_spram_mem_ctrl_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int repair_disable_53to32 : 22;
        unsigned int reserved : 10;
    } reg;
} SOC_SCTRL_SCPERCTRL13_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL13_repair_disable_53to32_START (0)
#define SOC_SCTRL_SCPERCTRL13_repair_disable_53to32_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aolpram_spram_mem_ctrl : 26;
        unsigned int reserved : 6;
    } reg;
} SOC_SCTRL_SCPERCTRL14_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL14_aolpram_spram_mem_ctrl_START (0)
#define SOC_SCTRL_SCPERCTRL14_aolpram_spram_mem_ctrl_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERCTRL15_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int avs_en0 : 1;
        unsigned int avs_en1 : 1;
        unsigned int avs_en2 : 1;
        unsigned int avs_en3 : 1;
        unsigned int avs_en4 : 1;
        unsigned int avs_en5 : 1;
        unsigned int avs_en6 : 1;
        unsigned int avs_en7 : 1;
        unsigned int avs_en8 : 1;
        unsigned int avs_en9 : 1;
        unsigned int avs_en10 : 1;
        unsigned int avs_en11 : 1;
        unsigned int reserved : 4;
        unsigned int scperctrl16_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL16_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL16_avs_en0_START (0)
#define SOC_SCTRL_SCPERCTRL16_avs_en0_END (0)
#define SOC_SCTRL_SCPERCTRL16_avs_en1_START (1)
#define SOC_SCTRL_SCPERCTRL16_avs_en1_END (1)
#define SOC_SCTRL_SCPERCTRL16_avs_en2_START (2)
#define SOC_SCTRL_SCPERCTRL16_avs_en2_END (2)
#define SOC_SCTRL_SCPERCTRL16_avs_en3_START (3)
#define SOC_SCTRL_SCPERCTRL16_avs_en3_END (3)
#define SOC_SCTRL_SCPERCTRL16_avs_en4_START (4)
#define SOC_SCTRL_SCPERCTRL16_avs_en4_END (4)
#define SOC_SCTRL_SCPERCTRL16_avs_en5_START (5)
#define SOC_SCTRL_SCPERCTRL16_avs_en5_END (5)
#define SOC_SCTRL_SCPERCTRL16_avs_en6_START (6)
#define SOC_SCTRL_SCPERCTRL16_avs_en6_END (6)
#define SOC_SCTRL_SCPERCTRL16_avs_en7_START (7)
#define SOC_SCTRL_SCPERCTRL16_avs_en7_END (7)
#define SOC_SCTRL_SCPERCTRL16_avs_en8_START (8)
#define SOC_SCTRL_SCPERCTRL16_avs_en8_END (8)
#define SOC_SCTRL_SCPERCTRL16_avs_en9_START (9)
#define SOC_SCTRL_SCPERCTRL16_avs_en9_END (9)
#define SOC_SCTRL_SCPERCTRL16_avs_en10_START (10)
#define SOC_SCTRL_SCPERCTRL16_avs_en10_END (10)
#define SOC_SCTRL_SCPERCTRL16_avs_en11_START (11)
#define SOC_SCTRL_SCPERCTRL16_avs_en11_END (11)
#define SOC_SCTRL_SCPERCTRL16_scperctrl16_msk_START (16)
#define SOC_SCTRL_SCPERCTRL16_scperctrl16_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bisr_reserve_in : 4;
        unsigned int dbg_repair_frm_efu : 1;
        unsigned int dbg_repair_frm_efu_1 : 1;
        unsigned int sc_spi3_cs : 4;
        unsigned int sc_onewire1_en : 1;
        unsigned int reserved : 5;
        unsigned int scperctrl17_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL17_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL17_bisr_reserve_in_START (0)
#define SOC_SCTRL_SCPERCTRL17_bisr_reserve_in_END (3)
#define SOC_SCTRL_SCPERCTRL17_dbg_repair_frm_efu_START (4)
#define SOC_SCTRL_SCPERCTRL17_dbg_repair_frm_efu_END (4)
#define SOC_SCTRL_SCPERCTRL17_dbg_repair_frm_efu_1_START (5)
#define SOC_SCTRL_SCPERCTRL17_dbg_repair_frm_efu_1_END (5)
#define SOC_SCTRL_SCPERCTRL17_sc_spi3_cs_START (6)
#define SOC_SCTRL_SCPERCTRL17_sc_spi3_cs_END (9)
#define SOC_SCTRL_SCPERCTRL17_sc_onewire1_en_START (10)
#define SOC_SCTRL_SCPERCTRL17_sc_onewire1_en_END (10)
#define SOC_SCTRL_SCPERCTRL17_scperctrl17_msk_START (16)
#define SOC_SCTRL_SCPERCTRL17_scperctrl17_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_stat_asp_nonidle_pend : 1;
        unsigned int intr_stat_iomcu_nonidle_pend : 1;
        unsigned int intr_stat_fd_nonidle_pend : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 2;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 3;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 2;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 15;
    } reg;
} SOC_SCTRL_SC_INTR_NOCBUS_STAT_UNION;
#endif
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_intr_stat_asp_nonidle_pend_START (0)
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_intr_stat_asp_nonidle_pend_END (0)
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_intr_stat_iomcu_nonidle_pend_START (1)
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_intr_stat_iomcu_nonidle_pend_END (1)
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_intr_stat_fd_nonidle_pend_START (2)
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_intr_stat_fd_nonidle_pend_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int autodiv_ufs_subsys_stat : 1;
        unsigned int reserved_1 : 2;
        unsigned int reserved_2 : 1;
        unsigned int sw_ack_clk_fll_src : 2;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 3;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 2;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 15;
    } reg;
} SOC_SCTRL_SCPERSTATUS17_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS17_autodiv_ufs_subsys_stat_START (1)
#define SOC_SCTRL_SCPERSTATUS17_autodiv_ufs_subsys_stat_END (1)
#define SOC_SCTRL_SCPERSTATUS17_sw_ack_clk_fll_src_START (5)
#define SOC_SCTRL_SCPERSTATUS17_sw_ack_clk_fll_src_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_ack_clk_memrepair_sw : 3;
        unsigned int sw_ack_clk_aobus_ini : 2;
        unsigned int swdone_clk_asp_codec_pll_div : 1;
        unsigned int swdone_clk_io_peri : 1;
        unsigned int swdone_clk_ufs_subsys_sys_div : 1;
        unsigned int swdone_clk_ufs_subsys_pll_div : 1;
        unsigned int sw_ack_clk_ufs_subsys_sw : 3;
        unsigned int swdone_clk_spmi_mst : 1;
        unsigned int sw_ack_clk_hifd_pll_sw : 2;
        unsigned int swdone_clk_ao_camera_div : 1;
        unsigned int reserved_0 : 1;
        unsigned int st_pclk_spmi : 1;
        unsigned int sw_ack_clk_hsdt_subsys_sw : 3;
        unsigned int reserved_1 : 1;
        unsigned int st_clk_lbintpll1_out : 1;
        unsigned int sw_ack_clk_csi_cfg_sw : 2;
        unsigned int reserved_2 : 7;
    } reg;
} SOC_SCTRL_SCPERSTATUS16_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_memrepair_sw_START (0)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_memrepair_sw_END (2)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_aobus_ini_START (3)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_aobus_ini_END (4)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_asp_codec_pll_div_START (5)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_asp_codec_pll_div_END (5)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_io_peri_START (6)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_io_peri_END (6)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_ufs_subsys_sys_div_START (7)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_ufs_subsys_sys_div_END (7)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_ufs_subsys_pll_div_START (8)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_ufs_subsys_pll_div_END (8)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_ufs_subsys_sw_START (9)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_ufs_subsys_sw_END (11)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_spmi_mst_START (12)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_spmi_mst_END (12)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_hifd_pll_sw_START (13)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_hifd_pll_sw_END (14)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_ao_camera_div_START (15)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_ao_camera_div_END (15)
#define SOC_SCTRL_SCPERSTATUS16_st_pclk_spmi_START (17)
#define SOC_SCTRL_SCPERSTATUS16_st_pclk_spmi_END (17)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_hsdt_subsys_sw_START (18)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_hsdt_subsys_sw_END (20)
#define SOC_SCTRL_SCPERSTATUS16_st_clk_lbintpll1_out_START (22)
#define SOC_SCTRL_SCPERSTATUS16_st_clk_lbintpll1_out_END (22)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_csi_cfg_sw_START (23)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_csi_cfg_sw_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int asp_dwaxi_dlock_wr : 1;
        unsigned int asp_dwaxi_dlock_id : 4;
        unsigned int asp_dwaxi_dlock_slv : 3;
        unsigned int asp_dwaxi_dlock_mst : 2;
        unsigned int asp_hifi_st : 1;
        unsigned int tp_a0_grp : 10;
        unsigned int tp_a1_grp : 10;
        unsigned int bbp_apb_bus_slv_stat : 1;
    } reg;
} SOC_SCTRL_SCPERSTATUS0_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS0_asp_dwaxi_dlock_wr_START (0)
#define SOC_SCTRL_SCPERSTATUS0_asp_dwaxi_dlock_wr_END (0)
#define SOC_SCTRL_SCPERSTATUS0_asp_dwaxi_dlock_id_START (1)
#define SOC_SCTRL_SCPERSTATUS0_asp_dwaxi_dlock_id_END (4)
#define SOC_SCTRL_SCPERSTATUS0_asp_dwaxi_dlock_slv_START (5)
#define SOC_SCTRL_SCPERSTATUS0_asp_dwaxi_dlock_slv_END (7)
#define SOC_SCTRL_SCPERSTATUS0_asp_dwaxi_dlock_mst_START (8)
#define SOC_SCTRL_SCPERSTATUS0_asp_dwaxi_dlock_mst_END (9)
#define SOC_SCTRL_SCPERSTATUS0_asp_hifi_st_START (10)
#define SOC_SCTRL_SCPERSTATUS0_asp_hifi_st_END (10)
#define SOC_SCTRL_SCPERSTATUS0_tp_a0_grp_START (11)
#define SOC_SCTRL_SCPERSTATUS0_tp_a0_grp_END (20)
#define SOC_SCTRL_SCPERSTATUS0_tp_a1_grp_START (21)
#define SOC_SCTRL_SCPERSTATUS0_tp_a1_grp_END (30)
#define SOC_SCTRL_SCPERSTATUS0_bbp_apb_bus_slv_stat_START (31)
#define SOC_SCTRL_SCPERSTATUS0_bbp_apb_bus_slv_stat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bisr_reserve_out : 8;
        unsigned int repair_start_point_err_1 : 1;
        unsigned int repair_start_point_err_0 : 1;
        unsigned int intr_repair_err_1 : 1;
        unsigned int intr_repair_err_0 : 1;
        unsigned int pack_is0_err_1 : 1;
        unsigned int pack_is0_err_0 : 1;
        unsigned int dft_bisr_efuse_full : 1;
        unsigned int dft_bisr_efuse_full_1 : 1;
        unsigned int reserved : 16;
    } reg;
} SOC_SCTRL_SCPERSTATUS1_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS1_bisr_reserve_out_START (0)
#define SOC_SCTRL_SCPERSTATUS1_bisr_reserve_out_END (7)
#define SOC_SCTRL_SCPERSTATUS1_repair_start_point_err_1_START (8)
#define SOC_SCTRL_SCPERSTATUS1_repair_start_point_err_1_END (8)
#define SOC_SCTRL_SCPERSTATUS1_repair_start_point_err_0_START (9)
#define SOC_SCTRL_SCPERSTATUS1_repair_start_point_err_0_END (9)
#define SOC_SCTRL_SCPERSTATUS1_intr_repair_err_1_START (10)
#define SOC_SCTRL_SCPERSTATUS1_intr_repair_err_1_END (10)
#define SOC_SCTRL_SCPERSTATUS1_intr_repair_err_0_START (11)
#define SOC_SCTRL_SCPERSTATUS1_intr_repair_err_0_END (11)
#define SOC_SCTRL_SCPERSTATUS1_pack_is0_err_1_START (12)
#define SOC_SCTRL_SCPERSTATUS1_pack_is0_err_1_END (12)
#define SOC_SCTRL_SCPERSTATUS1_pack_is0_err_0_START (13)
#define SOC_SCTRL_SCPERSTATUS1_pack_is0_err_0_END (13)
#define SOC_SCTRL_SCPERSTATUS1_dft_bisr_efuse_full_START (14)
#define SOC_SCTRL_SCPERSTATUS1_dft_bisr_efuse_full_END (14)
#define SOC_SCTRL_SCPERSTATUS1_dft_bisr_efuse_full_1_START (15)
#define SOC_SCTRL_SCPERSTATUS1_dft_bisr_efuse_full_1_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tp_a2_grp : 10;
        unsigned int tp_a3_grp : 10;
        unsigned int tp_a4_grp : 10;
        unsigned int reserved : 2;
    } reg;
} SOC_SCTRL_SCPERSTATUS2_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS2_tp_a2_grp_START (0)
#define SOC_SCTRL_SCPERSTATUS2_tp_a2_grp_END (9)
#define SOC_SCTRL_SCPERSTATUS2_tp_a3_grp_START (10)
#define SOC_SCTRL_SCPERSTATUS2_tp_a3_grp_END (19)
#define SOC_SCTRL_SCPERSTATUS2_tp_a4_grp_START (20)
#define SOC_SCTRL_SCPERSTATUS2_tp_a4_grp_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bisr_repair_ack_0 : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS3_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS3_bisr_repair_ack_0_START (0)
#define SOC_SCTRL_SCPERSTATUS3_bisr_repair_ack_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bbdrx_timeout_dbg_info : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS4_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS4_bbdrx_timeout_dbg_info_START (0)
#define SOC_SCTRL_SCPERSTATUS4_bbdrx_timeout_dbg_info_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tp_a9_grp : 10;
        unsigned int tp_a10_grp : 10;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int ccpu5g_dbg_ack : 1;
        unsigned int ccpu_l2hac_dbg_ack : 1;
        unsigned int repair_finish : 1;
        unsigned int mdmcpu_dbgack : 1;
        unsigned int cfg_ispa7_dbgen_dx : 4;
    } reg;
} SOC_SCTRL_SCPERSTATUS5_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS5_tp_a9_grp_START (0)
#define SOC_SCTRL_SCPERSTATUS5_tp_a9_grp_END (9)
#define SOC_SCTRL_SCPERSTATUS5_tp_a10_grp_START (10)
#define SOC_SCTRL_SCPERSTATUS5_tp_a10_grp_END (19)
#define SOC_SCTRL_SCPERSTATUS5_ccpu5g_dbg_ack_START (24)
#define SOC_SCTRL_SCPERSTATUS5_ccpu5g_dbg_ack_END (24)
#define SOC_SCTRL_SCPERSTATUS5_ccpu_l2hac_dbg_ack_START (25)
#define SOC_SCTRL_SCPERSTATUS5_ccpu_l2hac_dbg_ack_END (25)
#define SOC_SCTRL_SCPERSTATUS5_repair_finish_START (26)
#define SOC_SCTRL_SCPERSTATUS5_repair_finish_END (26)
#define SOC_SCTRL_SCPERSTATUS5_mdmcpu_dbgack_START (27)
#define SOC_SCTRL_SCPERSTATUS5_mdmcpu_dbgack_END (27)
#define SOC_SCTRL_SCPERSTATUS5_cfg_ispa7_dbgen_dx_START (28)
#define SOC_SCTRL_SCPERSTATUS5_cfg_ispa7_dbgen_dx_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int sc_noc_asp_cfg_t_maintimeout : 1;
        unsigned int intr_aobus_aon_trace_packet_probe_tracealar : 1;
        unsigned int sc_noc_aon_apb_slv_t_maintimeout : 1;
        unsigned int noc_asp_power_idle : 1;
        unsigned int intr_asp_transaction_probe_mainstatalarm : 1;
        unsigned int intr_aobus_error_probe_observer_mainfault : 1;
        unsigned int noc_asp_power_idleack : 1;
        unsigned int iomcu_ahb_slv_maintimeout : 1;
        unsigned int iomcu_apb_slv_maintimeout : 1;
        unsigned int iomcu_power_idle : 1;
        unsigned int iomcu_power_idleack : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int sc_cfg_arm_dbgen_dx : 4;
        unsigned int sc_cfg_mcu_dbgen_dx : 4;
        unsigned int audio_mmbuf_ctrl_to_sctrl : 8;
    } reg;
} SOC_SCTRL_SCPERSTATUS6_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS6_sc_noc_asp_cfg_t_maintimeout_START (2)
#define SOC_SCTRL_SCPERSTATUS6_sc_noc_asp_cfg_t_maintimeout_END (2)
#define SOC_SCTRL_SCPERSTATUS6_intr_aobus_aon_trace_packet_probe_tracealar_START (3)
#define SOC_SCTRL_SCPERSTATUS6_intr_aobus_aon_trace_packet_probe_tracealar_END (3)
#define SOC_SCTRL_SCPERSTATUS6_sc_noc_aon_apb_slv_t_maintimeout_START (4)
#define SOC_SCTRL_SCPERSTATUS6_sc_noc_aon_apb_slv_t_maintimeout_END (4)
#define SOC_SCTRL_SCPERSTATUS6_noc_asp_power_idle_START (5)
#define SOC_SCTRL_SCPERSTATUS6_noc_asp_power_idle_END (5)
#define SOC_SCTRL_SCPERSTATUS6_intr_asp_transaction_probe_mainstatalarm_START (6)
#define SOC_SCTRL_SCPERSTATUS6_intr_asp_transaction_probe_mainstatalarm_END (6)
#define SOC_SCTRL_SCPERSTATUS6_intr_aobus_error_probe_observer_mainfault_START (7)
#define SOC_SCTRL_SCPERSTATUS6_intr_aobus_error_probe_observer_mainfault_END (7)
#define SOC_SCTRL_SCPERSTATUS6_noc_asp_power_idleack_START (8)
#define SOC_SCTRL_SCPERSTATUS6_noc_asp_power_idleack_END (8)
#define SOC_SCTRL_SCPERSTATUS6_iomcu_ahb_slv_maintimeout_START (9)
#define SOC_SCTRL_SCPERSTATUS6_iomcu_ahb_slv_maintimeout_END (9)
#define SOC_SCTRL_SCPERSTATUS6_iomcu_apb_slv_maintimeout_START (10)
#define SOC_SCTRL_SCPERSTATUS6_iomcu_apb_slv_maintimeout_END (10)
#define SOC_SCTRL_SCPERSTATUS6_iomcu_power_idle_START (11)
#define SOC_SCTRL_SCPERSTATUS6_iomcu_power_idle_END (11)
#define SOC_SCTRL_SCPERSTATUS6_iomcu_power_idleack_START (12)
#define SOC_SCTRL_SCPERSTATUS6_iomcu_power_idleack_END (12)
#define SOC_SCTRL_SCPERSTATUS6_sc_cfg_arm_dbgen_dx_START (16)
#define SOC_SCTRL_SCPERSTATUS6_sc_cfg_arm_dbgen_dx_END (19)
#define SOC_SCTRL_SCPERSTATUS6_sc_cfg_mcu_dbgen_dx_START (20)
#define SOC_SCTRL_SCPERSTATUS6_sc_cfg_mcu_dbgen_dx_END (23)
#define SOC_SCTRL_SCPERSTATUS6_audio_mmbuf_ctrl_to_sctrl_START (24)
#define SOC_SCTRL_SCPERSTATUS6_audio_mmbuf_ctrl_to_sctrl_END (31)
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
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int iomcu_power_idle : 1;
        unsigned int iomcu_power_idleack : 1;
        unsigned int sc_noc_asp_power_idle : 1;
        unsigned int sc_noc_asp_power_idleack : 1;
        unsigned int noc_fd_power_idle : 1;
        unsigned int noc_fd_power_idleack : 1;
        unsigned int noc_sysbus_power_idle : 1;
        unsigned int noc_sysbus_power_idleack : 1;
        unsigned int reserved_16 : 8;
    } reg;
} SOC_SCTRL_SCPERSTATUS7_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS7_iomcu_power_idle_START (16)
#define SOC_SCTRL_SCPERSTATUS7_iomcu_power_idle_END (16)
#define SOC_SCTRL_SCPERSTATUS7_iomcu_power_idleack_START (17)
#define SOC_SCTRL_SCPERSTATUS7_iomcu_power_idleack_END (17)
#define SOC_SCTRL_SCPERSTATUS7_sc_noc_asp_power_idle_START (18)
#define SOC_SCTRL_SCPERSTATUS7_sc_noc_asp_power_idle_END (18)
#define SOC_SCTRL_SCPERSTATUS7_sc_noc_asp_power_idleack_START (19)
#define SOC_SCTRL_SCPERSTATUS7_sc_noc_asp_power_idleack_END (19)
#define SOC_SCTRL_SCPERSTATUS7_noc_fd_power_idle_START (20)
#define SOC_SCTRL_SCPERSTATUS7_noc_fd_power_idle_END (20)
#define SOC_SCTRL_SCPERSTATUS7_noc_fd_power_idleack_START (21)
#define SOC_SCTRL_SCPERSTATUS7_noc_fd_power_idleack_END (21)
#define SOC_SCTRL_SCPERSTATUS7_noc_sysbus_power_idle_START (22)
#define SOC_SCTRL_SCPERSTATUS7_noc_sysbus_power_idle_END (22)
#define SOC_SCTRL_SCPERSTATUS7_noc_sysbus_power_idleack_START (23)
#define SOC_SCTRL_SCPERSTATUS7_noc_sysbus_power_idleack_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int iomcu_apb_slv_maintimeout : 1;
        unsigned int iomcu_ahb_slv_maintimeout : 1;
        unsigned int sc_noc_aon_apb_slv_t_maintimeout : 1;
        unsigned int sc_noc_asp_cfg_t_maintimeout : 1;
        unsigned int noc_fd_cfg_t_maintimeout : 1;
        unsigned int sc_noc_mad_cfg_t_maintimeout : 1;
        unsigned int noc_ao_tcp_cfg_t_maintimeout : 1;
        unsigned int noc_ufs_cfg_t_maintimeout : 1;
        unsigned int intr_aobus_error_probe_observer_mainfault : 1;
        unsigned int reserved_0 : 7;
        unsigned int intr_asp_transaction_probe_mainstatalarm : 1;
        unsigned int intr_aobus_aon_trace_packet_probe_tracealar : 1;
        unsigned int reserved_1 : 14;
    } reg;
} SOC_SCTRL_SCPERSTATUS8_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS8_iomcu_apb_slv_maintimeout_START (0)
#define SOC_SCTRL_SCPERSTATUS8_iomcu_apb_slv_maintimeout_END (0)
#define SOC_SCTRL_SCPERSTATUS8_iomcu_ahb_slv_maintimeout_START (1)
#define SOC_SCTRL_SCPERSTATUS8_iomcu_ahb_slv_maintimeout_END (1)
#define SOC_SCTRL_SCPERSTATUS8_sc_noc_aon_apb_slv_t_maintimeout_START (2)
#define SOC_SCTRL_SCPERSTATUS8_sc_noc_aon_apb_slv_t_maintimeout_END (2)
#define SOC_SCTRL_SCPERSTATUS8_sc_noc_asp_cfg_t_maintimeout_START (3)
#define SOC_SCTRL_SCPERSTATUS8_sc_noc_asp_cfg_t_maintimeout_END (3)
#define SOC_SCTRL_SCPERSTATUS8_noc_fd_cfg_t_maintimeout_START (4)
#define SOC_SCTRL_SCPERSTATUS8_noc_fd_cfg_t_maintimeout_END (4)
#define SOC_SCTRL_SCPERSTATUS8_sc_noc_mad_cfg_t_maintimeout_START (5)
#define SOC_SCTRL_SCPERSTATUS8_sc_noc_mad_cfg_t_maintimeout_END (5)
#define SOC_SCTRL_SCPERSTATUS8_noc_ao_tcp_cfg_t_maintimeout_START (6)
#define SOC_SCTRL_SCPERSTATUS8_noc_ao_tcp_cfg_t_maintimeout_END (6)
#define SOC_SCTRL_SCPERSTATUS8_noc_ufs_cfg_t_maintimeout_START (7)
#define SOC_SCTRL_SCPERSTATUS8_noc_ufs_cfg_t_maintimeout_END (7)
#define SOC_SCTRL_SCPERSTATUS8_intr_aobus_error_probe_observer_mainfault_START (8)
#define SOC_SCTRL_SCPERSTATUS8_intr_aobus_error_probe_observer_mainfault_END (8)
#define SOC_SCTRL_SCPERSTATUS8_intr_asp_transaction_probe_mainstatalarm_START (16)
#define SOC_SCTRL_SCPERSTATUS8_intr_asp_transaction_probe_mainstatalarm_END (16)
#define SOC_SCTRL_SCPERSTATUS8_intr_aobus_aon_trace_packet_probe_tracealar_START (17)
#define SOC_SCTRL_SCPERSTATUS8_intr_aobus_aon_trace_packet_probe_tracealar_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_aobus_service_target_mainnopendingtrans : 1;
        unsigned int sc_noc_asp_mst_i_mainnowrpendingtrans : 1;
        unsigned int sc_aspbus_service_target_mainnopendingtrans : 1;
        unsigned int iomcu_dma_mst_mainnowrpendingtrans : 1;
        unsigned int reserved_0 : 1;
        unsigned int iomcu_ahb_mst_i_mainnowrpendingtrans : 1;
        unsigned int iomcu_ahb_mst_tcp_i_mainnowrpendingtrans : 1;
        unsigned int noc_asp_cfg_t_mainnopendingtrans : 1;
        unsigned int sc_noc_asp_mst_i_mainnordpendingtrans : 1;
        unsigned int noc_aon_apb_slv_t_mainnopendingtrans : 1;
        unsigned int noc_mad_cfg_t_mainnopendingtrans : 1;
        unsigned int iomcu_apb_slv_t_mainnopendingtrans : 1;
        unsigned int iomcu_ahb_slv_t_mainnopendingtrans : 1;
        unsigned int iomcu_dma_mst_i_mainnordpendingtrans : 1;
        unsigned int iomcu_ahb_mst_i_mainnordpendingtrans : 1;
        unsigned int iomcu_ahb_mst_tcp_i_mainnordpendingtrans : 1;
        unsigned int noc_fd_mst_i_mainnowrpendingtrans : 1;
        unsigned int noc_fd_mst_i_mainnordpendingtrans : 1;
        unsigned int noc_fd_cfg_t_mainnopendingtrans : 1;
        unsigned int fdbus_service_target_mainnopendingtrans : 1;
        unsigned int noc_ao_tcp_mst_i_mainnowrpendingtrans : 1;
        unsigned int noc_ao_tcp_mst_i_mainnordpendingtrans : 1;
        unsigned int noc_ao_tcp_cfg_t_mainnordpendingtrans : 1;
        unsigned int noc_bbpdrx_mst_i_mainnowrpdendingtrans : 1;
        unsigned int noc_bbpdrx_mst_i_mainnordpdendingtrans : 1;
        unsigned int noc_ufs_cfg_t_mainnopdendingtrans : 1;
        unsigned int sc_noc_ufs_mst_i_mainnowrpdendingtrans : 1;
        unsigned int sc_noc_ufs_mst_i_mainnordpdendingtrans : 1;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_SCTRL_SCPERSTATUS9_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS9_sc_aobus_service_target_mainnopendingtrans_START (0)
#define SOC_SCTRL_SCPERSTATUS9_sc_aobus_service_target_mainnopendingtrans_END (0)
#define SOC_SCTRL_SCPERSTATUS9_sc_noc_asp_mst_i_mainnowrpendingtrans_START (1)
#define SOC_SCTRL_SCPERSTATUS9_sc_noc_asp_mst_i_mainnowrpendingtrans_END (1)
#define SOC_SCTRL_SCPERSTATUS9_sc_aspbus_service_target_mainnopendingtrans_START (2)
#define SOC_SCTRL_SCPERSTATUS9_sc_aspbus_service_target_mainnopendingtrans_END (2)
#define SOC_SCTRL_SCPERSTATUS9_iomcu_dma_mst_mainnowrpendingtrans_START (3)
#define SOC_SCTRL_SCPERSTATUS9_iomcu_dma_mst_mainnowrpendingtrans_END (3)
#define SOC_SCTRL_SCPERSTATUS9_iomcu_ahb_mst_i_mainnowrpendingtrans_START (5)
#define SOC_SCTRL_SCPERSTATUS9_iomcu_ahb_mst_i_mainnowrpendingtrans_END (5)
#define SOC_SCTRL_SCPERSTATUS9_iomcu_ahb_mst_tcp_i_mainnowrpendingtrans_START (6)
#define SOC_SCTRL_SCPERSTATUS9_iomcu_ahb_mst_tcp_i_mainnowrpendingtrans_END (6)
#define SOC_SCTRL_SCPERSTATUS9_noc_asp_cfg_t_mainnopendingtrans_START (7)
#define SOC_SCTRL_SCPERSTATUS9_noc_asp_cfg_t_mainnopendingtrans_END (7)
#define SOC_SCTRL_SCPERSTATUS9_sc_noc_asp_mst_i_mainnordpendingtrans_START (8)
#define SOC_SCTRL_SCPERSTATUS9_sc_noc_asp_mst_i_mainnordpendingtrans_END (8)
#define SOC_SCTRL_SCPERSTATUS9_noc_aon_apb_slv_t_mainnopendingtrans_START (9)
#define SOC_SCTRL_SCPERSTATUS9_noc_aon_apb_slv_t_mainnopendingtrans_END (9)
#define SOC_SCTRL_SCPERSTATUS9_noc_mad_cfg_t_mainnopendingtrans_START (10)
#define SOC_SCTRL_SCPERSTATUS9_noc_mad_cfg_t_mainnopendingtrans_END (10)
#define SOC_SCTRL_SCPERSTATUS9_iomcu_apb_slv_t_mainnopendingtrans_START (11)
#define SOC_SCTRL_SCPERSTATUS9_iomcu_apb_slv_t_mainnopendingtrans_END (11)
#define SOC_SCTRL_SCPERSTATUS9_iomcu_ahb_slv_t_mainnopendingtrans_START (12)
#define SOC_SCTRL_SCPERSTATUS9_iomcu_ahb_slv_t_mainnopendingtrans_END (12)
#define SOC_SCTRL_SCPERSTATUS9_iomcu_dma_mst_i_mainnordpendingtrans_START (13)
#define SOC_SCTRL_SCPERSTATUS9_iomcu_dma_mst_i_mainnordpendingtrans_END (13)
#define SOC_SCTRL_SCPERSTATUS9_iomcu_ahb_mst_i_mainnordpendingtrans_START (14)
#define SOC_SCTRL_SCPERSTATUS9_iomcu_ahb_mst_i_mainnordpendingtrans_END (14)
#define SOC_SCTRL_SCPERSTATUS9_iomcu_ahb_mst_tcp_i_mainnordpendingtrans_START (15)
#define SOC_SCTRL_SCPERSTATUS9_iomcu_ahb_mst_tcp_i_mainnordpendingtrans_END (15)
#define SOC_SCTRL_SCPERSTATUS9_noc_fd_mst_i_mainnowrpendingtrans_START (16)
#define SOC_SCTRL_SCPERSTATUS9_noc_fd_mst_i_mainnowrpendingtrans_END (16)
#define SOC_SCTRL_SCPERSTATUS9_noc_fd_mst_i_mainnordpendingtrans_START (17)
#define SOC_SCTRL_SCPERSTATUS9_noc_fd_mst_i_mainnordpendingtrans_END (17)
#define SOC_SCTRL_SCPERSTATUS9_noc_fd_cfg_t_mainnopendingtrans_START (18)
#define SOC_SCTRL_SCPERSTATUS9_noc_fd_cfg_t_mainnopendingtrans_END (18)
#define SOC_SCTRL_SCPERSTATUS9_fdbus_service_target_mainnopendingtrans_START (19)
#define SOC_SCTRL_SCPERSTATUS9_fdbus_service_target_mainnopendingtrans_END (19)
#define SOC_SCTRL_SCPERSTATUS9_noc_ao_tcp_mst_i_mainnowrpendingtrans_START (20)
#define SOC_SCTRL_SCPERSTATUS9_noc_ao_tcp_mst_i_mainnowrpendingtrans_END (20)
#define SOC_SCTRL_SCPERSTATUS9_noc_ao_tcp_mst_i_mainnordpendingtrans_START (21)
#define SOC_SCTRL_SCPERSTATUS9_noc_ao_tcp_mst_i_mainnordpendingtrans_END (21)
#define SOC_SCTRL_SCPERSTATUS9_noc_ao_tcp_cfg_t_mainnordpendingtrans_START (22)
#define SOC_SCTRL_SCPERSTATUS9_noc_ao_tcp_cfg_t_mainnordpendingtrans_END (22)
#define SOC_SCTRL_SCPERSTATUS9_noc_bbpdrx_mst_i_mainnowrpdendingtrans_START (23)
#define SOC_SCTRL_SCPERSTATUS9_noc_bbpdrx_mst_i_mainnowrpdendingtrans_END (23)
#define SOC_SCTRL_SCPERSTATUS9_noc_bbpdrx_mst_i_mainnordpdendingtrans_START (24)
#define SOC_SCTRL_SCPERSTATUS9_noc_bbpdrx_mst_i_mainnordpdendingtrans_END (24)
#define SOC_SCTRL_SCPERSTATUS9_noc_ufs_cfg_t_mainnopdendingtrans_START (25)
#define SOC_SCTRL_SCPERSTATUS9_noc_ufs_cfg_t_mainnopdendingtrans_END (25)
#define SOC_SCTRL_SCPERSTATUS9_sc_noc_ufs_mst_i_mainnowrpdendingtrans_START (26)
#define SOC_SCTRL_SCPERSTATUS9_sc_noc_ufs_mst_i_mainnowrpdendingtrans_END (26)
#define SOC_SCTRL_SCPERSTATUS9_sc_noc_ufs_mst_i_mainnordpdendingtrans_START (27)
#define SOC_SCTRL_SCPERSTATUS9_sc_noc_ufs_mst_i_mainnordpdendingtrans_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int iomcu_arc_watchdog_reset : 1;
        unsigned int hifd_ul_mtcmos_ack : 1;
        unsigned int hifd_ul_core_mtcmos_ack : 1;
        unsigned int pcie0_100ms_idle_timeout_intr : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int davinci_tiny_mtcmos_ack : 1;
        unsigned int iomcu_core_stalled : 1;
        unsigned int iomcu_sys_tf_half_r : 1;
        unsigned int reserved_2 : 13;
        unsigned int tp_a14_grp : 10;
    } reg;
} SOC_SCTRL_SCPERSTATUS10_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS10_iomcu_arc_watchdog_reset_START (0)
#define SOC_SCTRL_SCPERSTATUS10_iomcu_arc_watchdog_reset_END (0)
#define SOC_SCTRL_SCPERSTATUS10_hifd_ul_mtcmos_ack_START (1)
#define SOC_SCTRL_SCPERSTATUS10_hifd_ul_mtcmos_ack_END (1)
#define SOC_SCTRL_SCPERSTATUS10_hifd_ul_core_mtcmos_ack_START (2)
#define SOC_SCTRL_SCPERSTATUS10_hifd_ul_core_mtcmos_ack_END (2)
#define SOC_SCTRL_SCPERSTATUS10_pcie0_100ms_idle_timeout_intr_START (3)
#define SOC_SCTRL_SCPERSTATUS10_pcie0_100ms_idle_timeout_intr_END (3)
#define SOC_SCTRL_SCPERSTATUS10_davinci_tiny_mtcmos_ack_START (6)
#define SOC_SCTRL_SCPERSTATUS10_davinci_tiny_mtcmos_ack_END (6)
#define SOC_SCTRL_SCPERSTATUS10_iomcu_core_stalled_START (7)
#define SOC_SCTRL_SCPERSTATUS10_iomcu_core_stalled_END (7)
#define SOC_SCTRL_SCPERSTATUS10_iomcu_sys_tf_half_r_START (8)
#define SOC_SCTRL_SCPERSTATUS10_iomcu_sys_tf_half_r_END (8)
#define SOC_SCTRL_SCPERSTATUS10_tp_a14_grp_START (22)
#define SOC_SCTRL_SCPERSTATUS10_tp_a14_grp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tp_a11_grp : 10;
        unsigned int tp_a12_grp : 10;
        unsigned int tp_a13_grp : 10;
        unsigned int reserved : 2;
    } reg;
} SOC_SCTRL_SCPERSTATUS11_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS11_tp_a11_grp_START (0)
#define SOC_SCTRL_SCPERSTATUS11_tp_a11_grp_END (9)
#define SOC_SCTRL_SCPERSTATUS11_tp_a12_grp_START (10)
#define SOC_SCTRL_SCPERSTATUS11_tp_a12_grp_END (19)
#define SOC_SCTRL_SCPERSTATUS11_tp_a13_grp_START (20)
#define SOC_SCTRL_SCPERSTATUS11_tp_a13_grp_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bisr_repair_ack_1 : 22;
        unsigned int reserved : 10;
    } reg;
} SOC_SCTRL_SCPERSTATUS12_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS12_bisr_repair_ack_1_START (0)
#define SOC_SCTRL_SCPERSTATUS12_bisr_repair_ack_1_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ao_hpm_opc : 10;
        unsigned int ao_hpm_opc_vld : 1;
        unsigned int reserved_0 : 1;
        unsigned int ao_hpmx_opc : 10;
        unsigned int ao_hpmx_opc_vld : 1;
        unsigned int reserved_1 : 9;
    } reg;
} SOC_SCTRL_SCPERSTATUS13_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS13_ao_hpm_opc_START (0)
#define SOC_SCTRL_SCPERSTATUS13_ao_hpm_opc_END (9)
#define SOC_SCTRL_SCPERSTATUS13_ao_hpm_opc_vld_START (10)
#define SOC_SCTRL_SCPERSTATUS13_ao_hpm_opc_vld_END (10)
#define SOC_SCTRL_SCPERSTATUS13_ao_hpmx_opc_START (12)
#define SOC_SCTRL_SCPERSTATUS13_ao_hpmx_opc_END (21)
#define SOC_SCTRL_SCPERSTATUS13_ao_hpmx_opc_vld_START (22)
#define SOC_SCTRL_SCPERSTATUS13_ao_hpmx_opc_vld_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tp_a8_grp : 10;
        unsigned int reserved : 22;
    } reg;
} SOC_SCTRL_SCPERSTATUS14_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS14_tp_a8_grp_START (0)
#define SOC_SCTRL_SCPERSTATUS14_tp_a8_grp_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddra_reset_state : 1;
        unsigned int ddrb_reset_state : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 12;
        unsigned int cmp_cc712_key_state : 16;
    } reg;
} SOC_SCTRL_SCPERSTATUS15_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS15_ddra_reset_state_START (0)
#define SOC_SCTRL_SCPERSTATUS15_ddra_reset_state_END (0)
#define SOC_SCTRL_SCPERSTATUS15_ddrb_reset_state_START (1)
#define SOC_SCTRL_SCPERSTATUS15_ddrb_reset_state_END (1)
#define SOC_SCTRL_SCPERSTATUS15_cmp_cc712_key_state_START (16)
#define SOC_SCTRL_SCPERSTATUS15_cmp_cc712_key_state_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int memory_repair_bypass : 1;
        unsigned int hardw_ctrl_sel0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int fpll0_bp : 1;
        unsigned int fpll0_gt : 1;
        unsigned int fpll0_en : 1;
        unsigned int reserved_1 : 7;
        unsigned int isp_face_detect_fuse : 1;
        unsigned int emmc_ufs_sel : 1;
        unsigned int ate_mode : 5;
        unsigned int vote_clk_gt_sel_flag : 7;
        unsigned int reserved_2 : 4;
    } reg;
} SOC_SCTRL_SCINNERSTAT_UNION;
#endif
#define SOC_SCTRL_SCINNERSTAT_memory_repair_bypass_START (0)
#define SOC_SCTRL_SCINNERSTAT_memory_repair_bypass_END (0)
#define SOC_SCTRL_SCINNERSTAT_hardw_ctrl_sel0_START (1)
#define SOC_SCTRL_SCINNERSTAT_hardw_ctrl_sel0_END (1)
#define SOC_SCTRL_SCINNERSTAT_fpll0_bp_START (4)
#define SOC_SCTRL_SCINNERSTAT_fpll0_bp_END (4)
#define SOC_SCTRL_SCINNERSTAT_fpll0_gt_START (5)
#define SOC_SCTRL_SCINNERSTAT_fpll0_gt_END (5)
#define SOC_SCTRL_SCINNERSTAT_fpll0_en_START (6)
#define SOC_SCTRL_SCINNERSTAT_fpll0_en_END (6)
#define SOC_SCTRL_SCINNERSTAT_isp_face_detect_fuse_START (14)
#define SOC_SCTRL_SCINNERSTAT_isp_face_detect_fuse_END (14)
#define SOC_SCTRL_SCINNERSTAT_emmc_ufs_sel_START (15)
#define SOC_SCTRL_SCINNERSTAT_emmc_ufs_sel_END (15)
#define SOC_SCTRL_SCINNERSTAT_ate_mode_START (16)
#define SOC_SCTRL_SCINNERSTAT_ate_mode_END (20)
#define SOC_SCTRL_SCINNERSTAT_vote_clk_gt_sel_flag_START (21)
#define SOC_SCTRL_SCINNERSTAT_vote_clk_gt_sel_flag_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mali_dbgack_byp : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCINNERCTRL_UNION;
#endif
#define SOC_SCTRL_SCINNERCTRL_mali_dbgack_byp_START (0)
#define SOC_SCTRL_SCINNERCTRL_mali_dbgack_byp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mddrc_syscache_retention_flag : 1;
        unsigned int sc_mem_ctrl_sd_asp : 1;
        unsigned int sc_mem_ctrl_sd_mad : 1;
        unsigned int sc_spi3_sd_in : 1;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 8;
        unsigned int hifi_load_image_flag : 1;
        unsigned int asp_imgld_flag : 1;
        unsigned int sc_mem_ctrl_sd_msk : 16;
    } reg;
} SOC_SCTRL_SC_MEM_CTRL_SD_UNION;
#endif
#define SOC_SCTRL_SC_MEM_CTRL_SD_mddrc_syscache_retention_flag_START (0)
#define SOC_SCTRL_SC_MEM_CTRL_SD_mddrc_syscache_retention_flag_END (0)
#define SOC_SCTRL_SC_MEM_CTRL_SD_sc_mem_ctrl_sd_asp_START (1)
#define SOC_SCTRL_SC_MEM_CTRL_SD_sc_mem_ctrl_sd_asp_END (1)
#define SOC_SCTRL_SC_MEM_CTRL_SD_sc_mem_ctrl_sd_mad_START (2)
#define SOC_SCTRL_SC_MEM_CTRL_SD_sc_mem_ctrl_sd_mad_END (2)
#define SOC_SCTRL_SC_MEM_CTRL_SD_sc_spi3_sd_in_START (3)
#define SOC_SCTRL_SC_MEM_CTRL_SD_sc_spi3_sd_in_END (3)
#define SOC_SCTRL_SC_MEM_CTRL_SD_hifi_load_image_flag_START (14)
#define SOC_SCTRL_SC_MEM_CTRL_SD_hifi_load_image_flag_END (14)
#define SOC_SCTRL_SC_MEM_CTRL_SD_asp_imgld_flag_START (15)
#define SOC_SCTRL_SC_MEM_CTRL_SD_asp_imgld_flag_END (15)
#define SOC_SCTRL_SC_MEM_CTRL_SD_sc_mem_ctrl_sd_msk_START (16)
#define SOC_SCTRL_SC_MEM_CTRL_SD_sc_mem_ctrl_sd_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_mem_stat_sd_iomcu : 1;
        unsigned int sc_mem_stat_sd_asp : 1;
        unsigned int sc_mem_stat_sd_ao : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 26;
    } reg;
} SOC_SCTRL_SC_MEM_STAT_SD_UNION;
#endif
#define SOC_SCTRL_SC_MEM_STAT_SD_sc_mem_stat_sd_iomcu_START (0)
#define SOC_SCTRL_SC_MEM_STAT_SD_sc_mem_stat_sd_iomcu_END (0)
#define SOC_SCTRL_SC_MEM_STAT_SD_sc_mem_stat_sd_asp_START (1)
#define SOC_SCTRL_SC_MEM_STAT_SD_sc_mem_stat_sd_asp_END (1)
#define SOC_SCTRL_SC_MEM_STAT_SD_sc_mem_stat_sd_ao_START (2)
#define SOC_SCTRL_SC_MEM_STAT_SD_sc_mem_stat_sd_ao_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int timer_en_force_high : 1;
        unsigned int mdmtimer_stop_dbg_bp : 1;
        unsigned int timer0_A_en_ov : 1;
        unsigned int timer0_A_en_sel : 1;
        unsigned int timer0_B_en_ov : 1;
        unsigned int timer0_B_en_sel : 1;
        unsigned int timer2_A_en_ov : 1;
        unsigned int timer2_A_en_sel : 1;
        unsigned int timer2_B_en_ov : 1;
        unsigned int timer2_B_en_sel : 1;
        unsigned int timer3_A_en_ov : 1;
        unsigned int timer3_A_en_sel : 1;
        unsigned int timer3_B_en_ov : 1;
        unsigned int timer3_B_en_sel : 1;
        unsigned int reserved : 18;
    } reg;
} SOC_SCTRL_SCTIMERCTRL0_UNION;
#endif
#define SOC_SCTRL_SCTIMERCTRL0_timer_en_force_high_START (0)
#define SOC_SCTRL_SCTIMERCTRL0_timer_en_force_high_END (0)
#define SOC_SCTRL_SCTIMERCTRL0_mdmtimer_stop_dbg_bp_START (1)
#define SOC_SCTRL_SCTIMERCTRL0_mdmtimer_stop_dbg_bp_END (1)
#define SOC_SCTRL_SCTIMERCTRL0_timer0_A_en_ov_START (2)
#define SOC_SCTRL_SCTIMERCTRL0_timer0_A_en_ov_END (2)
#define SOC_SCTRL_SCTIMERCTRL0_timer0_A_en_sel_START (3)
#define SOC_SCTRL_SCTIMERCTRL0_timer0_A_en_sel_END (3)
#define SOC_SCTRL_SCTIMERCTRL0_timer0_B_en_ov_START (4)
#define SOC_SCTRL_SCTIMERCTRL0_timer0_B_en_ov_END (4)
#define SOC_SCTRL_SCTIMERCTRL0_timer0_B_en_sel_START (5)
#define SOC_SCTRL_SCTIMERCTRL0_timer0_B_en_sel_END (5)
#define SOC_SCTRL_SCTIMERCTRL0_timer2_A_en_ov_START (6)
#define SOC_SCTRL_SCTIMERCTRL0_timer2_A_en_ov_END (6)
#define SOC_SCTRL_SCTIMERCTRL0_timer2_A_en_sel_START (7)
#define SOC_SCTRL_SCTIMERCTRL0_timer2_A_en_sel_END (7)
#define SOC_SCTRL_SCTIMERCTRL0_timer2_B_en_ov_START (8)
#define SOC_SCTRL_SCTIMERCTRL0_timer2_B_en_ov_END (8)
#define SOC_SCTRL_SCTIMERCTRL0_timer2_B_en_sel_START (9)
#define SOC_SCTRL_SCTIMERCTRL0_timer2_B_en_sel_END (9)
#define SOC_SCTRL_SCTIMERCTRL0_timer3_A_en_ov_START (10)
#define SOC_SCTRL_SCTIMERCTRL0_timer3_A_en_ov_END (10)
#define SOC_SCTRL_SCTIMERCTRL0_timer3_A_en_sel_START (11)
#define SOC_SCTRL_SCTIMERCTRL0_timer3_A_en_sel_END (11)
#define SOC_SCTRL_SCTIMERCTRL0_timer3_B_en_ov_START (12)
#define SOC_SCTRL_SCTIMERCTRL0_timer3_B_en_ov_END (12)
#define SOC_SCTRL_SCTIMERCTRL0_timer3_B_en_sel_START (13)
#define SOC_SCTRL_SCTIMERCTRL0_timer3_B_en_sel_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int timer4_A_en_ov : 1;
        unsigned int timer4_A_en_sel : 1;
        unsigned int timer4_B_en_ov : 1;
        unsigned int timer4_B_en_sel : 1;
        unsigned int timer5_A_en_ov : 1;
        unsigned int timer5_A_en_sel : 1;
        unsigned int timer5_B_en_ov : 1;
        unsigned int timer5_B_en_sel : 1;
        unsigned int timer6_A_en_ov : 1;
        unsigned int timer6_A_en_sel : 1;
        unsigned int timer6_B_en_ov : 1;
        unsigned int timer6_B_en_sel : 1;
        unsigned int timer7_A_en_ov : 1;
        unsigned int timer7_A_en_sel : 1;
        unsigned int timer7_B_en_ov : 1;
        unsigned int timer7_B_en_sel : 1;
        unsigned int timer8_A_en_ov : 1;
        unsigned int timer8_A_en_sel : 1;
        unsigned int timer8_B_en_ov : 1;
        unsigned int timer8_B_en_sel : 1;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCTIMERCTRL1_UNION;
#endif
#define SOC_SCTRL_SCTIMERCTRL1_timer4_A_en_ov_START (0)
#define SOC_SCTRL_SCTIMERCTRL1_timer4_A_en_ov_END (0)
#define SOC_SCTRL_SCTIMERCTRL1_timer4_A_en_sel_START (1)
#define SOC_SCTRL_SCTIMERCTRL1_timer4_A_en_sel_END (1)
#define SOC_SCTRL_SCTIMERCTRL1_timer4_B_en_ov_START (2)
#define SOC_SCTRL_SCTIMERCTRL1_timer4_B_en_ov_END (2)
#define SOC_SCTRL_SCTIMERCTRL1_timer4_B_en_sel_START (3)
#define SOC_SCTRL_SCTIMERCTRL1_timer4_B_en_sel_END (3)
#define SOC_SCTRL_SCTIMERCTRL1_timer5_A_en_ov_START (4)
#define SOC_SCTRL_SCTIMERCTRL1_timer5_A_en_ov_END (4)
#define SOC_SCTRL_SCTIMERCTRL1_timer5_A_en_sel_START (5)
#define SOC_SCTRL_SCTIMERCTRL1_timer5_A_en_sel_END (5)
#define SOC_SCTRL_SCTIMERCTRL1_timer5_B_en_ov_START (6)
#define SOC_SCTRL_SCTIMERCTRL1_timer5_B_en_ov_END (6)
#define SOC_SCTRL_SCTIMERCTRL1_timer5_B_en_sel_START (7)
#define SOC_SCTRL_SCTIMERCTRL1_timer5_B_en_sel_END (7)
#define SOC_SCTRL_SCTIMERCTRL1_timer6_A_en_ov_START (8)
#define SOC_SCTRL_SCTIMERCTRL1_timer6_A_en_ov_END (8)
#define SOC_SCTRL_SCTIMERCTRL1_timer6_A_en_sel_START (9)
#define SOC_SCTRL_SCTIMERCTRL1_timer6_A_en_sel_END (9)
#define SOC_SCTRL_SCTIMERCTRL1_timer6_B_en_ov_START (10)
#define SOC_SCTRL_SCTIMERCTRL1_timer6_B_en_ov_END (10)
#define SOC_SCTRL_SCTIMERCTRL1_timer6_B_en_sel_START (11)
#define SOC_SCTRL_SCTIMERCTRL1_timer6_B_en_sel_END (11)
#define SOC_SCTRL_SCTIMERCTRL1_timer7_A_en_ov_START (12)
#define SOC_SCTRL_SCTIMERCTRL1_timer7_A_en_ov_END (12)
#define SOC_SCTRL_SCTIMERCTRL1_timer7_A_en_sel_START (13)
#define SOC_SCTRL_SCTIMERCTRL1_timer7_A_en_sel_END (13)
#define SOC_SCTRL_SCTIMERCTRL1_timer7_B_en_ov_START (14)
#define SOC_SCTRL_SCTIMERCTRL1_timer7_B_en_ov_END (14)
#define SOC_SCTRL_SCTIMERCTRL1_timer7_B_en_sel_START (15)
#define SOC_SCTRL_SCTIMERCTRL1_timer7_B_en_sel_END (15)
#define SOC_SCTRL_SCTIMERCTRL1_timer8_A_en_ov_START (16)
#define SOC_SCTRL_SCTIMERCTRL1_timer8_A_en_ov_END (16)
#define SOC_SCTRL_SCTIMERCTRL1_timer8_A_en_sel_START (17)
#define SOC_SCTRL_SCTIMERCTRL1_timer8_A_en_sel_END (17)
#define SOC_SCTRL_SCTIMERCTRL1_timer8_B_en_ov_START (18)
#define SOC_SCTRL_SCTIMERCTRL1_timer8_B_en_ov_END (18)
#define SOC_SCTRL_SCTIMERCTRL1_timer8_B_en_sel_START (19)
#define SOC_SCTRL_SCTIMERCTRL1_timer8_B_en_sel_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int timer_en_ov18 : 1;
        unsigned int timer_en_sel18 : 1;
        unsigned int timer_en_ov19 : 1;
        unsigned int timer_en_sel19 : 1;
        unsigned int timer_en_ov20 : 1;
        unsigned int timer_en_sel20 : 1;
        unsigned int timer_en_ov21 : 1;
        unsigned int timer_en_sel21 : 1;
        unsigned int timer_en_ov22 : 1;
        unsigned int timer_en_sel22 : 1;
        unsigned int timer_en_ov23 : 1;
        unsigned int timer_en_sel23 : 1;
        unsigned int timer_en_ov24 : 1;
        unsigned int timer_en_sel24 : 1;
        unsigned int timer_en_ov25 : 1;
        unsigned int timer_en_sel25 : 1;
        unsigned int timer_en_ov26 : 1;
        unsigned int timer_en_sel26 : 1;
        unsigned int timer_en_ov27 : 1;
        unsigned int timer_en_sel27 : 1;
        unsigned int mdm4g_timer_stop_dbg_bp_1 : 1;
        unsigned int mdm5g_timer_stop_dbg_bp_0 : 1;
        unsigned int mdm5g_timer_stop_dbg_bp_1 : 1;
        unsigned int mdm5g_timer_stop_dbg_bp_2 : 1;
        unsigned int mdm5g_timer_stop_dbg_bp_3 : 1;
        unsigned int reserved : 7;
    } reg;
} SOC_SCTRL_SCTIMERCTRL2_UNION;
#endif
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov18_START (0)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov18_END (0)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel18_START (1)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel18_END (1)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov19_START (2)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov19_END (2)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel19_START (3)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel19_END (3)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov20_START (4)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov20_END (4)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel20_START (5)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel20_END (5)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov21_START (6)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov21_END (6)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel21_START (7)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel21_END (7)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov22_START (8)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov22_END (8)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel22_START (9)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel22_END (9)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov23_START (10)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov23_END (10)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel23_START (11)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel23_END (11)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov24_START (12)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov24_END (12)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel24_START (13)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel24_END (13)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov25_START (14)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov25_END (14)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel25_START (15)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel25_END (15)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov26_START (16)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov26_END (16)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel26_START (17)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel26_END (17)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov27_START (18)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_ov27_END (18)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel27_START (19)
#define SOC_SCTRL_SCTIMERCTRL2_timer_en_sel27_END (19)
#define SOC_SCTRL_SCTIMERCTRL2_mdm4g_timer_stop_dbg_bp_1_START (20)
#define SOC_SCTRL_SCTIMERCTRL2_mdm4g_timer_stop_dbg_bp_1_END (20)
#define SOC_SCTRL_SCTIMERCTRL2_mdm5g_timer_stop_dbg_bp_0_START (21)
#define SOC_SCTRL_SCTIMERCTRL2_mdm5g_timer_stop_dbg_bp_0_END (21)
#define SOC_SCTRL_SCTIMERCTRL2_mdm5g_timer_stop_dbg_bp_1_START (22)
#define SOC_SCTRL_SCTIMERCTRL2_mdm5g_timer_stop_dbg_bp_1_END (22)
#define SOC_SCTRL_SCTIMERCTRL2_mdm5g_timer_stop_dbg_bp_2_START (23)
#define SOC_SCTRL_SCTIMERCTRL2_mdm5g_timer_stop_dbg_bp_2_END (23)
#define SOC_SCTRL_SCTIMERCTRL2_mdm5g_timer_stop_dbg_bp_3_START (24)
#define SOC_SCTRL_SCTIMERCTRL2_mdm5g_timer_stop_dbg_bp_3_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int asp_transaction_probe_mainstatalarm_intmsk : 1;
        unsigned int reserved_2 : 1;
        unsigned int intr_aobus_aon_trace_packet_probe_tracealarm_intmsk : 1;
        unsigned int msk_noc_asp_cfg_t_maintimeout_intmsk : 1;
        unsigned int msk_iomcu_ahb_slv_maintimeout_intmsk : 1;
        unsigned int msk_iomcu_apb_slv_maintimeout_intmsk : 1;
        unsigned int reserved_3 : 1;
        unsigned int sc_noc_aon_apb_slv_t_maintimeout_intmsk : 1;
        unsigned int aobus_error_probe_observer_mainfault_0_intmsk : 1;
        unsigned int sc_noc_mad_cfg_t_maintineout_intmsk : 1;
        unsigned int noc_fd_cfg_t_maintimeout_intmsk : 1;
        unsigned int noc_ao_tcp_cfg_t_maintimeout_intmsk : 1;
        unsigned int noc_ufs_cfg_t_maintimeout_intmsk : 1;
        unsigned int reserved_4 : 17;
    } reg;
} SOC_SCTRL_SC_SECOND_INT_MASK_UNION;
#endif
#define SOC_SCTRL_SC_SECOND_INT_MASK_asp_transaction_probe_mainstatalarm_intmsk_START (2)
#define SOC_SCTRL_SC_SECOND_INT_MASK_asp_transaction_probe_mainstatalarm_intmsk_END (2)
#define SOC_SCTRL_SC_SECOND_INT_MASK_intr_aobus_aon_trace_packet_probe_tracealarm_intmsk_START (4)
#define SOC_SCTRL_SC_SECOND_INT_MASK_intr_aobus_aon_trace_packet_probe_tracealarm_intmsk_END (4)
#define SOC_SCTRL_SC_SECOND_INT_MASK_msk_noc_asp_cfg_t_maintimeout_intmsk_START (5)
#define SOC_SCTRL_SC_SECOND_INT_MASK_msk_noc_asp_cfg_t_maintimeout_intmsk_END (5)
#define SOC_SCTRL_SC_SECOND_INT_MASK_msk_iomcu_ahb_slv_maintimeout_intmsk_START (6)
#define SOC_SCTRL_SC_SECOND_INT_MASK_msk_iomcu_ahb_slv_maintimeout_intmsk_END (6)
#define SOC_SCTRL_SC_SECOND_INT_MASK_msk_iomcu_apb_slv_maintimeout_intmsk_START (7)
#define SOC_SCTRL_SC_SECOND_INT_MASK_msk_iomcu_apb_slv_maintimeout_intmsk_END (7)
#define SOC_SCTRL_SC_SECOND_INT_MASK_sc_noc_aon_apb_slv_t_maintimeout_intmsk_START (9)
#define SOC_SCTRL_SC_SECOND_INT_MASK_sc_noc_aon_apb_slv_t_maintimeout_intmsk_END (9)
#define SOC_SCTRL_SC_SECOND_INT_MASK_aobus_error_probe_observer_mainfault_0_intmsk_START (10)
#define SOC_SCTRL_SC_SECOND_INT_MASK_aobus_error_probe_observer_mainfault_0_intmsk_END (10)
#define SOC_SCTRL_SC_SECOND_INT_MASK_sc_noc_mad_cfg_t_maintineout_intmsk_START (11)
#define SOC_SCTRL_SC_SECOND_INT_MASK_sc_noc_mad_cfg_t_maintineout_intmsk_END (11)
#define SOC_SCTRL_SC_SECOND_INT_MASK_noc_fd_cfg_t_maintimeout_intmsk_START (12)
#define SOC_SCTRL_SC_SECOND_INT_MASK_noc_fd_cfg_t_maintimeout_intmsk_END (12)
#define SOC_SCTRL_SC_SECOND_INT_MASK_noc_ao_tcp_cfg_t_maintimeout_intmsk_START (13)
#define SOC_SCTRL_SC_SECOND_INT_MASK_noc_ao_tcp_cfg_t_maintimeout_intmsk_END (13)
#define SOC_SCTRL_SC_SECOND_INT_MASK_noc_ufs_cfg_t_maintimeout_intmsk_START (14)
#define SOC_SCTRL_SC_SECOND_INT_MASK_noc_ufs_cfg_t_maintimeout_intmsk_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int asp_transaction_probe_mainstatalarm_int : 1;
        unsigned int reserved_2 : 1;
        unsigned int intr_aobus_aon_trace_packet_probe_tracealarm_int : 1;
        unsigned int noc_asp_cfg_t_maintimeout : 1;
        unsigned int iomcu_ahb_slv_maintimeout : 1;
        unsigned int iomcu_apb_slv_maintimeout : 1;
        unsigned int reserved_3 : 1;
        unsigned int sc_noc_aon_apb_slv_t_maintimeout : 1;
        unsigned int aobus_error_probe_observer_mainfault_0 : 1;
        unsigned int sc_noc_mad_cfg_t_maintineout_intmsk : 1;
        unsigned int noc_fd_cfg_t_maintimeout_int : 1;
        unsigned int noc_ao_tcp_cfg_t_maintimeout_int : 1;
        unsigned int noc_ufs_cfg_t_maintimeout_int : 1;
        unsigned int reserved_4 : 17;
    } reg;
} SOC_SCTRL_SC_SECOND_INT_ORG_UNION;
#endif
#define SOC_SCTRL_SC_SECOND_INT_ORG_asp_transaction_probe_mainstatalarm_int_START (2)
#define SOC_SCTRL_SC_SECOND_INT_ORG_asp_transaction_probe_mainstatalarm_int_END (2)
#define SOC_SCTRL_SC_SECOND_INT_ORG_intr_aobus_aon_trace_packet_probe_tracealarm_int_START (4)
#define SOC_SCTRL_SC_SECOND_INT_ORG_intr_aobus_aon_trace_packet_probe_tracealarm_int_END (4)
#define SOC_SCTRL_SC_SECOND_INT_ORG_noc_asp_cfg_t_maintimeout_START (5)
#define SOC_SCTRL_SC_SECOND_INT_ORG_noc_asp_cfg_t_maintimeout_END (5)
#define SOC_SCTRL_SC_SECOND_INT_ORG_iomcu_ahb_slv_maintimeout_START (6)
#define SOC_SCTRL_SC_SECOND_INT_ORG_iomcu_ahb_slv_maintimeout_END (6)
#define SOC_SCTRL_SC_SECOND_INT_ORG_iomcu_apb_slv_maintimeout_START (7)
#define SOC_SCTRL_SC_SECOND_INT_ORG_iomcu_apb_slv_maintimeout_END (7)
#define SOC_SCTRL_SC_SECOND_INT_ORG_sc_noc_aon_apb_slv_t_maintimeout_START (9)
#define SOC_SCTRL_SC_SECOND_INT_ORG_sc_noc_aon_apb_slv_t_maintimeout_END (9)
#define SOC_SCTRL_SC_SECOND_INT_ORG_aobus_error_probe_observer_mainfault_0_START (10)
#define SOC_SCTRL_SC_SECOND_INT_ORG_aobus_error_probe_observer_mainfault_0_END (10)
#define SOC_SCTRL_SC_SECOND_INT_ORG_sc_noc_mad_cfg_t_maintineout_intmsk_START (11)
#define SOC_SCTRL_SC_SECOND_INT_ORG_sc_noc_mad_cfg_t_maintineout_intmsk_END (11)
#define SOC_SCTRL_SC_SECOND_INT_ORG_noc_fd_cfg_t_maintimeout_int_START (12)
#define SOC_SCTRL_SC_SECOND_INT_ORG_noc_fd_cfg_t_maintimeout_int_END (12)
#define SOC_SCTRL_SC_SECOND_INT_ORG_noc_ao_tcp_cfg_t_maintimeout_int_START (13)
#define SOC_SCTRL_SC_SECOND_INT_ORG_noc_ao_tcp_cfg_t_maintimeout_int_END (13)
#define SOC_SCTRL_SC_SECOND_INT_ORG_noc_ufs_cfg_t_maintimeout_int_START (14)
#define SOC_SCTRL_SC_SECOND_INT_ORG_noc_ufs_cfg_t_maintimeout_int_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int asp_transaction_probe_mainstatalarm_int_msksta : 1;
        unsigned int reserved_2 : 1;
        unsigned int intr_aobus_aon_trace_packet_probe_tracealarm_int_msksta : 1;
        unsigned int noc_asp_cfg_t_maintimeout_msksta : 1;
        unsigned int iomcu_ahb_slv_maintimeout_msksta : 1;
        unsigned int iomcu_apb_slv_maintimeout_msksta : 1;
        unsigned int reserved_3 : 1;
        unsigned int sc_noc_aon_apb_slv_t_maintimeout_msksta : 1;
        unsigned int aobus_error_probe_observer_mainfault_0_msksta : 1;
        unsigned int sc_noc_mad_cfg_t_maintineout_msksta : 1;
        unsigned int noc_fd_cfg_t_maintimeout_msksta : 1;
        unsigned int noc_ao_tcp_cfg_t_maintimeout_mskstat : 1;
        unsigned int noc_ufs_cfg_t_maintimeout_mskstat : 1;
        unsigned int reserved_4 : 17;
    } reg;
} SOC_SCTRL_SC_SECOND_INT_OUT_UNION;
#endif
#define SOC_SCTRL_SC_SECOND_INT_OUT_asp_transaction_probe_mainstatalarm_int_msksta_START (2)
#define SOC_SCTRL_SC_SECOND_INT_OUT_asp_transaction_probe_mainstatalarm_int_msksta_END (2)
#define SOC_SCTRL_SC_SECOND_INT_OUT_intr_aobus_aon_trace_packet_probe_tracealarm_int_msksta_START (4)
#define SOC_SCTRL_SC_SECOND_INT_OUT_intr_aobus_aon_trace_packet_probe_tracealarm_int_msksta_END (4)
#define SOC_SCTRL_SC_SECOND_INT_OUT_noc_asp_cfg_t_maintimeout_msksta_START (5)
#define SOC_SCTRL_SC_SECOND_INT_OUT_noc_asp_cfg_t_maintimeout_msksta_END (5)
#define SOC_SCTRL_SC_SECOND_INT_OUT_iomcu_ahb_slv_maintimeout_msksta_START (6)
#define SOC_SCTRL_SC_SECOND_INT_OUT_iomcu_ahb_slv_maintimeout_msksta_END (6)
#define SOC_SCTRL_SC_SECOND_INT_OUT_iomcu_apb_slv_maintimeout_msksta_START (7)
#define SOC_SCTRL_SC_SECOND_INT_OUT_iomcu_apb_slv_maintimeout_msksta_END (7)
#define SOC_SCTRL_SC_SECOND_INT_OUT_sc_noc_aon_apb_slv_t_maintimeout_msksta_START (9)
#define SOC_SCTRL_SC_SECOND_INT_OUT_sc_noc_aon_apb_slv_t_maintimeout_msksta_END (9)
#define SOC_SCTRL_SC_SECOND_INT_OUT_aobus_error_probe_observer_mainfault_0_msksta_START (10)
#define SOC_SCTRL_SC_SECOND_INT_OUT_aobus_error_probe_observer_mainfault_0_msksta_END (10)
#define SOC_SCTRL_SC_SECOND_INT_OUT_sc_noc_mad_cfg_t_maintineout_msksta_START (11)
#define SOC_SCTRL_SC_SECOND_INT_OUT_sc_noc_mad_cfg_t_maintineout_msksta_END (11)
#define SOC_SCTRL_SC_SECOND_INT_OUT_noc_fd_cfg_t_maintimeout_msksta_START (12)
#define SOC_SCTRL_SC_SECOND_INT_OUT_noc_fd_cfg_t_maintimeout_msksta_END (12)
#define SOC_SCTRL_SC_SECOND_INT_OUT_noc_ao_tcp_cfg_t_maintimeout_mskstat_START (13)
#define SOC_SCTRL_SC_SECOND_INT_OUT_noc_ao_tcp_cfg_t_maintimeout_mskstat_END (13)
#define SOC_SCTRL_SC_SECOND_INT_OUT_noc_ufs_cfg_t_maintimeout_mskstat_START (14)
#define SOC_SCTRL_SC_SECOND_INT_OUT_noc_ufs_cfg_t_maintimeout_mskstat_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mrb_busy_flag : 32;
    } reg;
} SOC_SCTRL_SCMRBBUSYSTAT_UNION;
#endif
#define SOC_SCTRL_SCMRBBUSYSTAT_mrb_busy_flag_START (0)
#define SOC_SCTRL_SCMRBBUSYSTAT_mrb_busy_flag_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int swaddr : 32;
    } reg;
} SOC_SCTRL_SCSWADDR_UNION;
#endif
#define SOC_SCTRL_SCSWADDR_swaddr_START (0)
#define SOC_SCTRL_SCSWADDR_swaddr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddrtrainaddr : 32;
    } reg;
} SOC_SCTRL_SCDDRADDR_UNION;
#endif
#define SOC_SCTRL_SCDDRADDR_ddrtrainaddr_START (0)
#define SOC_SCTRL_SCDDRADDR_ddrtrainaddr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddrtraindata : 32;
    } reg;
} SOC_SCTRL_SCDDRDATA_UNION;
#endif
#define SOC_SCTRL_SCDDRDATA_ddrtraindata_START (0)
#define SOC_SCTRL_SCDDRDATA_ddrtraindata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA0_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA0_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA0_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA1_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA1_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA1_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA2_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA2_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA2_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA3_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA3_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA3_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA4_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA4_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA4_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA5_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA5_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA5_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA6_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA6_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA6_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA7_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA7_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA7_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA8_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA8_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA8_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA9_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA9_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA9_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA10_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA10_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA10_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA11_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA11_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA11_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA12_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA12_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA12_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA13_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA13_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA13_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA14_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA14_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA14_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA15_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA15_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA15_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA16_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA16_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA16_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA17_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA17_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA17_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA18_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA18_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA18_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA19_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA19_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA19_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA20_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA20_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA20_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA21_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA21_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA21_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA22_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA22_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA22_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA23_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA23_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA23_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA24_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA24_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA24_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA25_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA25_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA25_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA26_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA26_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA26_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA27_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA27_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA27_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA28_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA28_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA28_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dapclkoff_sys_n : 1;
        unsigned int i2c1_clk_clkoff_sys_n : 1;
        unsigned int rom_clk_clkoff_sys_n : 1;
        unsigned int ram_clk_clkoff_sys_n : 1;
        unsigned int aon_clk_clkoff_sys_n : 1;
        unsigned int ddrc_clk_clkoff_sys_n : 1;
        unsigned int tsen_clk_clkoff_sys_n : 1;
        unsigned int pmc_clk_clkoff_sys_n : 1;
        unsigned int uart_clk_clkoff_sys_n : 1;
        unsigned int i2c0_clk_clkoff_sys_n : 1;
        unsigned int reserved_0 : 1;
        unsigned int pcrg_clk_clkoff_sys_n : 1;
        unsigned int reserved_1 : 1;
        unsigned int wdog_clk_clkoff_sys_n : 1;
        unsigned int tim_clk_clkoff_sys_n : 1;
        unsigned int cssys_clk_clkoff_sys_n : 1;
        unsigned int spi_clk_clkoff_sys_n : 1;
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
    } reg;
} SOC_SCTRL_SCLPMCUCLKEN_UNION;
#endif
#define SOC_SCTRL_SCLPMCUCLKEN_dapclkoff_sys_n_START (0)
#define SOC_SCTRL_SCLPMCUCLKEN_dapclkoff_sys_n_END (0)
#define SOC_SCTRL_SCLPMCUCLKEN_i2c1_clk_clkoff_sys_n_START (1)
#define SOC_SCTRL_SCLPMCUCLKEN_i2c1_clk_clkoff_sys_n_END (1)
#define SOC_SCTRL_SCLPMCUCLKEN_rom_clk_clkoff_sys_n_START (2)
#define SOC_SCTRL_SCLPMCUCLKEN_rom_clk_clkoff_sys_n_END (2)
#define SOC_SCTRL_SCLPMCUCLKEN_ram_clk_clkoff_sys_n_START (3)
#define SOC_SCTRL_SCLPMCUCLKEN_ram_clk_clkoff_sys_n_END (3)
#define SOC_SCTRL_SCLPMCUCLKEN_aon_clk_clkoff_sys_n_START (4)
#define SOC_SCTRL_SCLPMCUCLKEN_aon_clk_clkoff_sys_n_END (4)
#define SOC_SCTRL_SCLPMCUCLKEN_ddrc_clk_clkoff_sys_n_START (5)
#define SOC_SCTRL_SCLPMCUCLKEN_ddrc_clk_clkoff_sys_n_END (5)
#define SOC_SCTRL_SCLPMCUCLKEN_tsen_clk_clkoff_sys_n_START (6)
#define SOC_SCTRL_SCLPMCUCLKEN_tsen_clk_clkoff_sys_n_END (6)
#define SOC_SCTRL_SCLPMCUCLKEN_pmc_clk_clkoff_sys_n_START (7)
#define SOC_SCTRL_SCLPMCUCLKEN_pmc_clk_clkoff_sys_n_END (7)
#define SOC_SCTRL_SCLPMCUCLKEN_uart_clk_clkoff_sys_n_START (8)
#define SOC_SCTRL_SCLPMCUCLKEN_uart_clk_clkoff_sys_n_END (8)
#define SOC_SCTRL_SCLPMCUCLKEN_i2c0_clk_clkoff_sys_n_START (9)
#define SOC_SCTRL_SCLPMCUCLKEN_i2c0_clk_clkoff_sys_n_END (9)
#define SOC_SCTRL_SCLPMCUCLKEN_pcrg_clk_clkoff_sys_n_START (11)
#define SOC_SCTRL_SCLPMCUCLKEN_pcrg_clk_clkoff_sys_n_END (11)
#define SOC_SCTRL_SCLPMCUCLKEN_wdog_clk_clkoff_sys_n_START (13)
#define SOC_SCTRL_SCLPMCUCLKEN_wdog_clk_clkoff_sys_n_END (13)
#define SOC_SCTRL_SCLPMCUCLKEN_tim_clk_clkoff_sys_n_START (14)
#define SOC_SCTRL_SCLPMCUCLKEN_tim_clk_clkoff_sys_n_END (14)
#define SOC_SCTRL_SCLPMCUCLKEN_cssys_clk_clkoff_sys_n_START (15)
#define SOC_SCTRL_SCLPMCUCLKEN_cssys_clk_clkoff_sys_n_END (15)
#define SOC_SCTRL_SCLPMCUCLKEN_spi_clk_clkoff_sys_n_START (16)
#define SOC_SCTRL_SCLPMCUCLKEN_spi_clk_clkoff_sys_n_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dapclkoff_sys_n : 1;
        unsigned int i2c1_clk_clkoff_sys_n : 1;
        unsigned int rom_clk_clkoff_sys_n : 1;
        unsigned int ram_clk_clkoff_sys_n : 1;
        unsigned int aon_clk_clkoff_sys_n : 1;
        unsigned int ddrc_clk_clkoff_sys_n : 1;
        unsigned int tsen_clk_clkoff_sys_n : 1;
        unsigned int pmc_clk_clkoff_sys_n : 1;
        unsigned int uart_clk_clkoff_sys_n : 1;
        unsigned int i2c0_clk_clkoff_sys_n : 1;
        unsigned int reserved_0 : 1;
        unsigned int pcrg_clk_clkoff_sys_n : 1;
        unsigned int reserved_1 : 1;
        unsigned int wdog_clk_clkoff_sys_n : 1;
        unsigned int tim_clk_clkoff_sys_n : 1;
        unsigned int cssys_clk_clkoff_sys_n : 1;
        unsigned int spi_clk_clkoff_sys_n : 1;
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
    } reg;
} SOC_SCTRL_SCLPMCUCLKDIS_UNION;
#endif
#define SOC_SCTRL_SCLPMCUCLKDIS_dapclkoff_sys_n_START (0)
#define SOC_SCTRL_SCLPMCUCLKDIS_dapclkoff_sys_n_END (0)
#define SOC_SCTRL_SCLPMCUCLKDIS_i2c1_clk_clkoff_sys_n_START (1)
#define SOC_SCTRL_SCLPMCUCLKDIS_i2c1_clk_clkoff_sys_n_END (1)
#define SOC_SCTRL_SCLPMCUCLKDIS_rom_clk_clkoff_sys_n_START (2)
#define SOC_SCTRL_SCLPMCUCLKDIS_rom_clk_clkoff_sys_n_END (2)
#define SOC_SCTRL_SCLPMCUCLKDIS_ram_clk_clkoff_sys_n_START (3)
#define SOC_SCTRL_SCLPMCUCLKDIS_ram_clk_clkoff_sys_n_END (3)
#define SOC_SCTRL_SCLPMCUCLKDIS_aon_clk_clkoff_sys_n_START (4)
#define SOC_SCTRL_SCLPMCUCLKDIS_aon_clk_clkoff_sys_n_END (4)
#define SOC_SCTRL_SCLPMCUCLKDIS_ddrc_clk_clkoff_sys_n_START (5)
#define SOC_SCTRL_SCLPMCUCLKDIS_ddrc_clk_clkoff_sys_n_END (5)
#define SOC_SCTRL_SCLPMCUCLKDIS_tsen_clk_clkoff_sys_n_START (6)
#define SOC_SCTRL_SCLPMCUCLKDIS_tsen_clk_clkoff_sys_n_END (6)
#define SOC_SCTRL_SCLPMCUCLKDIS_pmc_clk_clkoff_sys_n_START (7)
#define SOC_SCTRL_SCLPMCUCLKDIS_pmc_clk_clkoff_sys_n_END (7)
#define SOC_SCTRL_SCLPMCUCLKDIS_uart_clk_clkoff_sys_n_START (8)
#define SOC_SCTRL_SCLPMCUCLKDIS_uart_clk_clkoff_sys_n_END (8)
#define SOC_SCTRL_SCLPMCUCLKDIS_i2c0_clk_clkoff_sys_n_START (9)
#define SOC_SCTRL_SCLPMCUCLKDIS_i2c0_clk_clkoff_sys_n_END (9)
#define SOC_SCTRL_SCLPMCUCLKDIS_pcrg_clk_clkoff_sys_n_START (11)
#define SOC_SCTRL_SCLPMCUCLKDIS_pcrg_clk_clkoff_sys_n_END (11)
#define SOC_SCTRL_SCLPMCUCLKDIS_wdog_clk_clkoff_sys_n_START (13)
#define SOC_SCTRL_SCLPMCUCLKDIS_wdog_clk_clkoff_sys_n_END (13)
#define SOC_SCTRL_SCLPMCUCLKDIS_tim_clk_clkoff_sys_n_START (14)
#define SOC_SCTRL_SCLPMCUCLKDIS_tim_clk_clkoff_sys_n_END (14)
#define SOC_SCTRL_SCLPMCUCLKDIS_cssys_clk_clkoff_sys_n_START (15)
#define SOC_SCTRL_SCLPMCUCLKDIS_cssys_clk_clkoff_sys_n_END (15)
#define SOC_SCTRL_SCLPMCUCLKDIS_spi_clk_clkoff_sys_n_START (16)
#define SOC_SCTRL_SCLPMCUCLKDIS_spi_clk_clkoff_sys_n_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dapclkoff_sys_n : 1;
        unsigned int i2c1_clk_clkoff_sys_n : 1;
        unsigned int rom_clk_clkoff_sys_n : 1;
        unsigned int ram_clk_clkoff_sys_n : 1;
        unsigned int aon_clk_clkoff_sys_n : 1;
        unsigned int ddrc_clk_clkoff_sys_n : 1;
        unsigned int tsen_clk_clkoff_sys_n : 1;
        unsigned int pmc_clk_clkoff_sys_n : 1;
        unsigned int uart_clk_clkoff_sys_n : 1;
        unsigned int i2c0_clk_clkoff_sys_n : 1;
        unsigned int reserved_0 : 1;
        unsigned int pcrg_clk_clkoff_sys_n : 1;
        unsigned int reserved_1 : 1;
        unsigned int wdog_clk_clkoff_sys_n : 1;
        unsigned int tim_clk_clkoff_sys_n : 1;
        unsigned int cssys_clk_clkoff_sys_n : 1;
        unsigned int spi_clk_clkoff_sys_n : 1;
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
    } reg;
} SOC_SCTRL_SCLPMCUCLKSTAT_UNION;
#endif
#define SOC_SCTRL_SCLPMCUCLKSTAT_dapclkoff_sys_n_START (0)
#define SOC_SCTRL_SCLPMCUCLKSTAT_dapclkoff_sys_n_END (0)
#define SOC_SCTRL_SCLPMCUCLKSTAT_i2c1_clk_clkoff_sys_n_START (1)
#define SOC_SCTRL_SCLPMCUCLKSTAT_i2c1_clk_clkoff_sys_n_END (1)
#define SOC_SCTRL_SCLPMCUCLKSTAT_rom_clk_clkoff_sys_n_START (2)
#define SOC_SCTRL_SCLPMCUCLKSTAT_rom_clk_clkoff_sys_n_END (2)
#define SOC_SCTRL_SCLPMCUCLKSTAT_ram_clk_clkoff_sys_n_START (3)
#define SOC_SCTRL_SCLPMCUCLKSTAT_ram_clk_clkoff_sys_n_END (3)
#define SOC_SCTRL_SCLPMCUCLKSTAT_aon_clk_clkoff_sys_n_START (4)
#define SOC_SCTRL_SCLPMCUCLKSTAT_aon_clk_clkoff_sys_n_END (4)
#define SOC_SCTRL_SCLPMCUCLKSTAT_ddrc_clk_clkoff_sys_n_START (5)
#define SOC_SCTRL_SCLPMCUCLKSTAT_ddrc_clk_clkoff_sys_n_END (5)
#define SOC_SCTRL_SCLPMCUCLKSTAT_tsen_clk_clkoff_sys_n_START (6)
#define SOC_SCTRL_SCLPMCUCLKSTAT_tsen_clk_clkoff_sys_n_END (6)
#define SOC_SCTRL_SCLPMCUCLKSTAT_pmc_clk_clkoff_sys_n_START (7)
#define SOC_SCTRL_SCLPMCUCLKSTAT_pmc_clk_clkoff_sys_n_END (7)
#define SOC_SCTRL_SCLPMCUCLKSTAT_uart_clk_clkoff_sys_n_START (8)
#define SOC_SCTRL_SCLPMCUCLKSTAT_uart_clk_clkoff_sys_n_END (8)
#define SOC_SCTRL_SCLPMCUCLKSTAT_i2c0_clk_clkoff_sys_n_START (9)
#define SOC_SCTRL_SCLPMCUCLKSTAT_i2c0_clk_clkoff_sys_n_END (9)
#define SOC_SCTRL_SCLPMCUCLKSTAT_pcrg_clk_clkoff_sys_n_START (11)
#define SOC_SCTRL_SCLPMCUCLKSTAT_pcrg_clk_clkoff_sys_n_END (11)
#define SOC_SCTRL_SCLPMCUCLKSTAT_wdog_clk_clkoff_sys_n_START (13)
#define SOC_SCTRL_SCLPMCUCLKSTAT_wdog_clk_clkoff_sys_n_END (13)
#define SOC_SCTRL_SCLPMCUCLKSTAT_tim_clk_clkoff_sys_n_START (14)
#define SOC_SCTRL_SCLPMCUCLKSTAT_tim_clk_clkoff_sys_n_END (14)
#define SOC_SCTRL_SCLPMCUCLKSTAT_cssys_clk_clkoff_sys_n_START (15)
#define SOC_SCTRL_SCLPMCUCLKSTAT_cssys_clk_clkoff_sys_n_END (15)
#define SOC_SCTRL_SCLPMCUCLKSTAT_spi_clk_clkoff_sys_n_START (16)
#define SOC_SCTRL_SCLPMCUCLKSTAT_spi_clk_clkoff_sys_n_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rst_software_req : 1;
        unsigned int coresight_soft_rst_req : 1;
        unsigned int i2c1_soft_rst_req : 1;
        unsigned int rom_soft_rst_req : 1;
        unsigned int ram_soft_rst_req : 1;
        unsigned int aon_soft_rst_req : 1;
        unsigned int ccrg_soft_rst_req : 1;
        unsigned int tsen_soft_rst_req : 1;
        unsigned int pmc_soft_rst_req : 1;
        unsigned int uart_soft_rst_req : 1;
        unsigned int i2c0_soft_rst_req : 1;
        unsigned int reserved_0 : 1;
        unsigned int pcrg_soft_rst_req : 1;
        unsigned int reserved_1 : 1;
        unsigned int wdog_soft_rst_req : 1;
        unsigned int tim_soft_rst_req : 1;
        unsigned int cssys_soft_rst_req : 1;
        unsigned int spi_soft_rst_req : 1;
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
    } reg;
} SOC_SCTRL_SCLPMCURSTEN_UNION;
#endif
#define SOC_SCTRL_SCLPMCURSTEN_rst_software_req_START (0)
#define SOC_SCTRL_SCLPMCURSTEN_rst_software_req_END (0)
#define SOC_SCTRL_SCLPMCURSTEN_coresight_soft_rst_req_START (1)
#define SOC_SCTRL_SCLPMCURSTEN_coresight_soft_rst_req_END (1)
#define SOC_SCTRL_SCLPMCURSTEN_i2c1_soft_rst_req_START (2)
#define SOC_SCTRL_SCLPMCURSTEN_i2c1_soft_rst_req_END (2)
#define SOC_SCTRL_SCLPMCURSTEN_rom_soft_rst_req_START (3)
#define SOC_SCTRL_SCLPMCURSTEN_rom_soft_rst_req_END (3)
#define SOC_SCTRL_SCLPMCURSTEN_ram_soft_rst_req_START (4)
#define SOC_SCTRL_SCLPMCURSTEN_ram_soft_rst_req_END (4)
#define SOC_SCTRL_SCLPMCURSTEN_aon_soft_rst_req_START (5)
#define SOC_SCTRL_SCLPMCURSTEN_aon_soft_rst_req_END (5)
#define SOC_SCTRL_SCLPMCURSTEN_ccrg_soft_rst_req_START (6)
#define SOC_SCTRL_SCLPMCURSTEN_ccrg_soft_rst_req_END (6)
#define SOC_SCTRL_SCLPMCURSTEN_tsen_soft_rst_req_START (7)
#define SOC_SCTRL_SCLPMCURSTEN_tsen_soft_rst_req_END (7)
#define SOC_SCTRL_SCLPMCURSTEN_pmc_soft_rst_req_START (8)
#define SOC_SCTRL_SCLPMCURSTEN_pmc_soft_rst_req_END (8)
#define SOC_SCTRL_SCLPMCURSTEN_uart_soft_rst_req_START (9)
#define SOC_SCTRL_SCLPMCURSTEN_uart_soft_rst_req_END (9)
#define SOC_SCTRL_SCLPMCURSTEN_i2c0_soft_rst_req_START (10)
#define SOC_SCTRL_SCLPMCURSTEN_i2c0_soft_rst_req_END (10)
#define SOC_SCTRL_SCLPMCURSTEN_pcrg_soft_rst_req_START (12)
#define SOC_SCTRL_SCLPMCURSTEN_pcrg_soft_rst_req_END (12)
#define SOC_SCTRL_SCLPMCURSTEN_wdog_soft_rst_req_START (14)
#define SOC_SCTRL_SCLPMCURSTEN_wdog_soft_rst_req_END (14)
#define SOC_SCTRL_SCLPMCURSTEN_tim_soft_rst_req_START (15)
#define SOC_SCTRL_SCLPMCURSTEN_tim_soft_rst_req_END (15)
#define SOC_SCTRL_SCLPMCURSTEN_cssys_soft_rst_req_START (16)
#define SOC_SCTRL_SCLPMCURSTEN_cssys_soft_rst_req_END (16)
#define SOC_SCTRL_SCLPMCURSTEN_spi_soft_rst_req_START (17)
#define SOC_SCTRL_SCLPMCURSTEN_spi_soft_rst_req_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rst_software_req : 1;
        unsigned int coresight_soft_rst_req : 1;
        unsigned int i2c1_soft_rst_req : 1;
        unsigned int rom_soft_rst_req : 1;
        unsigned int ram_soft_rst_req : 1;
        unsigned int aon_soft_rst_req : 1;
        unsigned int ccrg_soft_rst_req : 1;
        unsigned int tsen_soft_rst_req : 1;
        unsigned int pmc_soft_rst_req : 1;
        unsigned int uart_soft_rst_req : 1;
        unsigned int i2c0_soft_rst_req : 1;
        unsigned int reserved_0 : 1;
        unsigned int pcrg_soft_rst_req : 1;
        unsigned int reserved_1 : 1;
        unsigned int wdog_soft_rst_req : 1;
        unsigned int tim_soft_rst_req : 1;
        unsigned int cssys_soft_rst_req : 1;
        unsigned int spi_soft_rst_req : 1;
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
    } reg;
} SOC_SCTRL_SCLPMCURSTDIS_UNION;
#endif
#define SOC_SCTRL_SCLPMCURSTDIS_rst_software_req_START (0)
#define SOC_SCTRL_SCLPMCURSTDIS_rst_software_req_END (0)
#define SOC_SCTRL_SCLPMCURSTDIS_coresight_soft_rst_req_START (1)
#define SOC_SCTRL_SCLPMCURSTDIS_coresight_soft_rst_req_END (1)
#define SOC_SCTRL_SCLPMCURSTDIS_i2c1_soft_rst_req_START (2)
#define SOC_SCTRL_SCLPMCURSTDIS_i2c1_soft_rst_req_END (2)
#define SOC_SCTRL_SCLPMCURSTDIS_rom_soft_rst_req_START (3)
#define SOC_SCTRL_SCLPMCURSTDIS_rom_soft_rst_req_END (3)
#define SOC_SCTRL_SCLPMCURSTDIS_ram_soft_rst_req_START (4)
#define SOC_SCTRL_SCLPMCURSTDIS_ram_soft_rst_req_END (4)
#define SOC_SCTRL_SCLPMCURSTDIS_aon_soft_rst_req_START (5)
#define SOC_SCTRL_SCLPMCURSTDIS_aon_soft_rst_req_END (5)
#define SOC_SCTRL_SCLPMCURSTDIS_ccrg_soft_rst_req_START (6)
#define SOC_SCTRL_SCLPMCURSTDIS_ccrg_soft_rst_req_END (6)
#define SOC_SCTRL_SCLPMCURSTDIS_tsen_soft_rst_req_START (7)
#define SOC_SCTRL_SCLPMCURSTDIS_tsen_soft_rst_req_END (7)
#define SOC_SCTRL_SCLPMCURSTDIS_pmc_soft_rst_req_START (8)
#define SOC_SCTRL_SCLPMCURSTDIS_pmc_soft_rst_req_END (8)
#define SOC_SCTRL_SCLPMCURSTDIS_uart_soft_rst_req_START (9)
#define SOC_SCTRL_SCLPMCURSTDIS_uart_soft_rst_req_END (9)
#define SOC_SCTRL_SCLPMCURSTDIS_i2c0_soft_rst_req_START (10)
#define SOC_SCTRL_SCLPMCURSTDIS_i2c0_soft_rst_req_END (10)
#define SOC_SCTRL_SCLPMCURSTDIS_pcrg_soft_rst_req_START (12)
#define SOC_SCTRL_SCLPMCURSTDIS_pcrg_soft_rst_req_END (12)
#define SOC_SCTRL_SCLPMCURSTDIS_wdog_soft_rst_req_START (14)
#define SOC_SCTRL_SCLPMCURSTDIS_wdog_soft_rst_req_END (14)
#define SOC_SCTRL_SCLPMCURSTDIS_tim_soft_rst_req_START (15)
#define SOC_SCTRL_SCLPMCURSTDIS_tim_soft_rst_req_END (15)
#define SOC_SCTRL_SCLPMCURSTDIS_cssys_soft_rst_req_START (16)
#define SOC_SCTRL_SCLPMCURSTDIS_cssys_soft_rst_req_END (16)
#define SOC_SCTRL_SCLPMCURSTDIS_spi_soft_rst_req_START (17)
#define SOC_SCTRL_SCLPMCURSTDIS_spi_soft_rst_req_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rst_software_req : 1;
        unsigned int coresight_soft_rst_req : 1;
        unsigned int i2c1_soft_rst_req : 1;
        unsigned int rom_soft_rst_req : 1;
        unsigned int ram_soft_rst_req : 1;
        unsigned int aon_soft_rst_req : 1;
        unsigned int ccrg_soft_rst_req : 1;
        unsigned int tsen_soft_rst_req : 1;
        unsigned int pmc_soft_rst_req : 1;
        unsigned int uart_soft_rst_req : 1;
        unsigned int i2c0_soft_rst_req : 1;
        unsigned int reserved_0 : 1;
        unsigned int pcrg_soft_rst_req : 1;
        unsigned int reserved_1 : 1;
        unsigned int wdog_soft_rst_req : 1;
        unsigned int tim_soft_rst_req : 1;
        unsigned int cssys_soft_rst_req : 1;
        unsigned int spi_soft_rst_req : 1;
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
    } reg;
} SOC_SCTRL_SCLPMCURSTSTAT_UNION;
#endif
#define SOC_SCTRL_SCLPMCURSTSTAT_rst_software_req_START (0)
#define SOC_SCTRL_SCLPMCURSTSTAT_rst_software_req_END (0)
#define SOC_SCTRL_SCLPMCURSTSTAT_coresight_soft_rst_req_START (1)
#define SOC_SCTRL_SCLPMCURSTSTAT_coresight_soft_rst_req_END (1)
#define SOC_SCTRL_SCLPMCURSTSTAT_i2c1_soft_rst_req_START (2)
#define SOC_SCTRL_SCLPMCURSTSTAT_i2c1_soft_rst_req_END (2)
#define SOC_SCTRL_SCLPMCURSTSTAT_rom_soft_rst_req_START (3)
#define SOC_SCTRL_SCLPMCURSTSTAT_rom_soft_rst_req_END (3)
#define SOC_SCTRL_SCLPMCURSTSTAT_ram_soft_rst_req_START (4)
#define SOC_SCTRL_SCLPMCURSTSTAT_ram_soft_rst_req_END (4)
#define SOC_SCTRL_SCLPMCURSTSTAT_aon_soft_rst_req_START (5)
#define SOC_SCTRL_SCLPMCURSTSTAT_aon_soft_rst_req_END (5)
#define SOC_SCTRL_SCLPMCURSTSTAT_ccrg_soft_rst_req_START (6)
#define SOC_SCTRL_SCLPMCURSTSTAT_ccrg_soft_rst_req_END (6)
#define SOC_SCTRL_SCLPMCURSTSTAT_tsen_soft_rst_req_START (7)
#define SOC_SCTRL_SCLPMCURSTSTAT_tsen_soft_rst_req_END (7)
#define SOC_SCTRL_SCLPMCURSTSTAT_pmc_soft_rst_req_START (8)
#define SOC_SCTRL_SCLPMCURSTSTAT_pmc_soft_rst_req_END (8)
#define SOC_SCTRL_SCLPMCURSTSTAT_uart_soft_rst_req_START (9)
#define SOC_SCTRL_SCLPMCURSTSTAT_uart_soft_rst_req_END (9)
#define SOC_SCTRL_SCLPMCURSTSTAT_i2c0_soft_rst_req_START (10)
#define SOC_SCTRL_SCLPMCURSTSTAT_i2c0_soft_rst_req_END (10)
#define SOC_SCTRL_SCLPMCURSTSTAT_pcrg_soft_rst_req_START (12)
#define SOC_SCTRL_SCLPMCURSTSTAT_pcrg_soft_rst_req_END (12)
#define SOC_SCTRL_SCLPMCURSTSTAT_wdog_soft_rst_req_START (14)
#define SOC_SCTRL_SCLPMCURSTSTAT_wdog_soft_rst_req_END (14)
#define SOC_SCTRL_SCLPMCURSTSTAT_tim_soft_rst_req_START (15)
#define SOC_SCTRL_SCLPMCURSTSTAT_tim_soft_rst_req_END (15)
#define SOC_SCTRL_SCLPMCURSTSTAT_cssys_soft_rst_req_START (16)
#define SOC_SCTRL_SCLPMCURSTSTAT_cssys_soft_rst_req_END (16)
#define SOC_SCTRL_SCLPMCURSTSTAT_spi_soft_rst_req_START (17)
#define SOC_SCTRL_SCLPMCURSTSTAT_spi_soft_rst_req_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int mpudisable : 1;
        unsigned int nmi_in : 1;
        unsigned int bus_div_clk_sel : 2;
        unsigned int sysresetreq_en : 1;
        unsigned int reserved_1 : 5;
        unsigned int lpmcuidle_bypass : 1;
        unsigned int reserved_2 : 12;
        unsigned int reserved_3 : 8;
    } reg;
} SOC_SCTRL_SCLPMCUCTRL_UNION;
#endif
#define SOC_SCTRL_SCLPMCUCTRL_mpudisable_START (1)
#define SOC_SCTRL_SCLPMCUCTRL_mpudisable_END (1)
#define SOC_SCTRL_SCLPMCUCTRL_nmi_in_START (2)
#define SOC_SCTRL_SCLPMCUCTRL_nmi_in_END (2)
#define SOC_SCTRL_SCLPMCUCTRL_bus_div_clk_sel_START (3)
#define SOC_SCTRL_SCLPMCUCTRL_bus_div_clk_sel_END (4)
#define SOC_SCTRL_SCLPMCUCTRL_sysresetreq_en_START (5)
#define SOC_SCTRL_SCLPMCUCTRL_sysresetreq_en_END (5)
#define SOC_SCTRL_SCLPMCUCTRL_lpmcuidle_bypass_START (11)
#define SOC_SCTRL_SCLPMCUCTRL_lpmcuidle_bypass_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sysresetreq_sign_out_stat : 1;
        unsigned int sleeping : 1;
        unsigned int lockup : 1;
        unsigned int halted : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_SCTRL_SCLPMCUSTAT_UNION;
#endif
#define SOC_SCTRL_SCLPMCUSTAT_sysresetreq_sign_out_stat_START (0)
#define SOC_SCTRL_SCLPMCUSTAT_sysresetreq_sign_out_stat_END (0)
#define SOC_SCTRL_SCLPMCUSTAT_sleeping_START (1)
#define SOC_SCTRL_SCLPMCUSTAT_sleeping_END (1)
#define SOC_SCTRL_SCLPMCUSTAT_lockup_START (2)
#define SOC_SCTRL_SCLPMCUSTAT_lockup_END (2)
#define SOC_SCTRL_SCLPMCUSTAT_halted_START (3)
#define SOC_SCTRL_SCLPMCUSTAT_halted_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lpramctrl_sel : 1;
        unsigned int lpram0_mod : 2;
        unsigned int lpram1_mod : 2;
        unsigned int lpram2_mod : 2;
        unsigned int lpram3_mod : 2;
        unsigned int lpram0ctrl_soft : 3;
        unsigned int lpram1ctrl_soft : 3;
        unsigned int lpram2ctrl_soft : 3;
        unsigned int lpram3ctrl_soft : 3;
        unsigned int memrep_ram_mod : 2;
        unsigned int memrep_ramctrl_soft : 3;
        unsigned int reserved : 6;
    } reg;
} SOC_SCTRL_SCLPMCURAMCTRL_UNION;
#endif
#define SOC_SCTRL_SCLPMCURAMCTRL_lpramctrl_sel_START (0)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpramctrl_sel_END (0)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram0_mod_START (1)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram0_mod_END (2)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram1_mod_START (3)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram1_mod_END (4)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram2_mod_START (5)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram2_mod_END (6)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram3_mod_START (7)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram3_mod_END (8)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram0ctrl_soft_START (9)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram0ctrl_soft_END (11)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram1ctrl_soft_START (12)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram1ctrl_soft_END (14)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram2ctrl_soft_START (15)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram2ctrl_soft_END (17)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram3ctrl_soft_START (18)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram3ctrl_soft_END (20)
#define SOC_SCTRL_SCLPMCURAMCTRL_memrep_ram_mod_START (21)
#define SOC_SCTRL_SCLPMCURAMCTRL_memrep_ram_mod_END (22)
#define SOC_SCTRL_SCLPMCURAMCTRL_memrep_ramctrl_soft_START (23)
#define SOC_SCTRL_SCLPMCURAMCTRL_memrep_ramctrl_soft_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g2_gdrx_depsleep_flag : 1;
        unsigned int g1_gdrx_depsleep_flag : 1;
        unsigned int wdrx_deepsleep_flag : 1;
        unsigned int cdrx_deepsleep_flag_1x : 1;
        unsigned int cdrx_deepsleep_flag_evdo : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_SCTRL_SCBBPDRXSTAT0_UNION;
#endif
#define SOC_SCTRL_SCBBPDRXSTAT0_g2_gdrx_depsleep_flag_START (0)
#define SOC_SCTRL_SCBBPDRXSTAT0_g2_gdrx_depsleep_flag_END (0)
#define SOC_SCTRL_SCBBPDRXSTAT0_g1_gdrx_depsleep_flag_START (1)
#define SOC_SCTRL_SCBBPDRXSTAT0_g1_gdrx_depsleep_flag_END (1)
#define SOC_SCTRL_SCBBPDRXSTAT0_wdrx_deepsleep_flag_START (2)
#define SOC_SCTRL_SCBBPDRXSTAT0_wdrx_deepsleep_flag_END (2)
#define SOC_SCTRL_SCBBPDRXSTAT0_cdrx_deepsleep_flag_1x_START (3)
#define SOC_SCTRL_SCBBPDRXSTAT0_cdrx_deepsleep_flag_1x_END (3)
#define SOC_SCTRL_SCBBPDRXSTAT0_cdrx_deepsleep_flag_evdo_START (4)
#define SOC_SCTRL_SCBBPDRXSTAT0_cdrx_deepsleep_flag_evdo_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ldrx2dbg_abs_timer1 : 32;
    } reg;
} SOC_SCTRL_SCBBPDRXSTAT1_UNION;
#endif
#define SOC_SCTRL_SCBBPDRXSTAT1_ldrx2dbg_abs_timer1_START (0)
#define SOC_SCTRL_SCBBPDRXSTAT1_ldrx2dbg_abs_timer1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ldrx2dbg_abs_timer2 : 32;
    } reg;
} SOC_SCTRL_SCBBPDRXSTAT2_UNION;
#endif
#define SOC_SCTRL_SCBBPDRXSTAT2_ldrx2dbg_abs_timer2_START (0)
#define SOC_SCTRL_SCBBPDRXSTAT2_ldrx2dbg_abs_timer2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ldrx2dbg_abs_timer_sleep1 : 32;
    } reg;
} SOC_SCTRL_SCBBPDRXSTAT3_UNION;
#endif
#define SOC_SCTRL_SCBBPDRXSTAT3_ldrx2dbg_abs_timer_sleep1_START (0)
#define SOC_SCTRL_SCBBPDRXSTAT3_ldrx2dbg_abs_timer_sleep1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ldrx2dbg_abs_timer_sleep2 : 32;
    } reg;
} SOC_SCTRL_SCBBPDRXSTAT4_UNION;
#endif
#define SOC_SCTRL_SCBBPDRXSTAT4_ldrx2dbg_abs_timer_sleep2_START (0)
#define SOC_SCTRL_SCBBPDRXSTAT4_ldrx2dbg_abs_timer_sleep2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
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
    } reg;
} SOC_SCTRL_SCA53_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCA53_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCA53_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCA53_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCA53_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCA53_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCA53_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCA53_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCA53_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCA53_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCA53_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCA53_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCA53_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCA53_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCA53_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCA53_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCA53_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCA53_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCA53_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCA53_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCA53_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCA53_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCA53_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCA53_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCA53_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCA53_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCA53_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
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
    } reg;
} SOC_SCTRL_SCA57_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCA57_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCA57_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCA57_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCA57_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCA57_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCA57_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCA57_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCA57_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCA57_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCA57_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCA57_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCA57_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCA57_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCA57_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCA57_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCA57_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCA57_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCA57_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCA57_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCA57_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCA57_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCA57_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCA57_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCA57_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCA57_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCA57_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
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
    } reg;
} SOC_SCTRL_SCIOMCU_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
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
    } reg;
} SOC_SCTRL_SCLPMCU_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
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
    } reg;
} SOC_SCTRL_SCMCPU_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCMCPU_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
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
    } reg;
} SOC_SCTRL_SCISPA7_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCISPA7_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
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
    } reg;
} SOC_SCTRL_SCHIFD_UL_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
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
    } reg;
} SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
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
    } reg;
} SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
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
    } reg;
} SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
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
    } reg;
} SOC_SCTRL_SCEPS_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCEPS_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCEPS_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCEPS_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCEPS_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCEPS_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCEPS_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCEPS_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCEPS_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCEPS_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCEPS_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCEPS_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCEPS_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCEPS_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCEPS_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCEPS_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCEPS_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCEPS_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCEPS_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCEPS_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCEPS_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCEPS_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCEPS_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCEPS_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCEPS_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCEPS_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCEPS_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
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
    } reg;
} SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
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
    } reg;
} SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_event : 1;
        unsigned int a57_event : 1;
        unsigned int iomcu_event : 1;
        unsigned int lpmcu_event : 1;
        unsigned int mcpu_event : 1;
        unsigned int ispa7_event : 1;
        unsigned int hifd_ul_event : 1;
        unsigned int eps_event : 1;
        unsigned int npu_archs_event : 1;
        unsigned int npu_tscpu_event : 1;
        unsigned int npu_aicpu_event : 1;
        unsigned int mdm5g_l2hac_event : 1;
        unsigned int mdm5g_ccpu5g_event : 1;
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
    } reg;
} SOC_SCTRL_SCEVENT_STAT_UNION;
#endif
#define SOC_SCTRL_SCEVENT_STAT_a53_event_START (0)
#define SOC_SCTRL_SCEVENT_STAT_a53_event_END (0)
#define SOC_SCTRL_SCEVENT_STAT_a57_event_START (1)
#define SOC_SCTRL_SCEVENT_STAT_a57_event_END (1)
#define SOC_SCTRL_SCEVENT_STAT_iomcu_event_START (2)
#define SOC_SCTRL_SCEVENT_STAT_iomcu_event_END (2)
#define SOC_SCTRL_SCEVENT_STAT_lpmcu_event_START (3)
#define SOC_SCTRL_SCEVENT_STAT_lpmcu_event_END (3)
#define SOC_SCTRL_SCEVENT_STAT_mcpu_event_START (4)
#define SOC_SCTRL_SCEVENT_STAT_mcpu_event_END (4)
#define SOC_SCTRL_SCEVENT_STAT_ispa7_event_START (5)
#define SOC_SCTRL_SCEVENT_STAT_ispa7_event_END (5)
#define SOC_SCTRL_SCEVENT_STAT_hifd_ul_event_START (6)
#define SOC_SCTRL_SCEVENT_STAT_hifd_ul_event_END (6)
#define SOC_SCTRL_SCEVENT_STAT_eps_event_START (7)
#define SOC_SCTRL_SCEVENT_STAT_eps_event_END (7)
#define SOC_SCTRL_SCEVENT_STAT_npu_archs_event_START (8)
#define SOC_SCTRL_SCEVENT_STAT_npu_archs_event_END (8)
#define SOC_SCTRL_SCEVENT_STAT_npu_tscpu_event_START (9)
#define SOC_SCTRL_SCEVENT_STAT_npu_tscpu_event_END (9)
#define SOC_SCTRL_SCEVENT_STAT_npu_aicpu_event_START (10)
#define SOC_SCTRL_SCEVENT_STAT_npu_aicpu_event_END (10)
#define SOC_SCTRL_SCEVENT_STAT_mdm5g_l2hac_event_START (11)
#define SOC_SCTRL_SCEVENT_STAT_mdm5g_l2hac_event_END (11)
#define SOC_SCTRL_SCEVENT_STAT_mdm5g_ccpu5g_event_START (12)
#define SOC_SCTRL_SCEVENT_STAT_mdm5g_ccpu5g_event_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_iomcu_pll_div : 1;
        unsigned int gt_iomcu_sys_div : 1;
        unsigned int reserved : 14;
        unsigned int sciomcuclkctrl_msk : 16;
    } reg;
} SOC_SCTRL_SCIOMCUCLKCTRL_UNION;
#endif
#define SOC_SCTRL_SCIOMCUCLKCTRL_gt_iomcu_pll_div_START (0)
#define SOC_SCTRL_SCIOMCUCLKCTRL_gt_iomcu_pll_div_END (0)
#define SOC_SCTRL_SCIOMCUCLKCTRL_gt_iomcu_sys_div_START (1)
#define SOC_SCTRL_SCIOMCUCLKCTRL_gt_iomcu_sys_div_END (1)
#define SOC_SCTRL_SCIOMCUCLKCTRL_sciomcuclkctrl_msk_START (16)
#define SOC_SCTRL_SCIOMCUCLKCTRL_sciomcuclkctrl_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int iomcu_pll_ctrl : 1;
        unsigned int iomcu_pll_div_ctrl : 1;
        unsigned int iomcu_sys_div_ctrl : 1;
        unsigned int gt_iomcu_pll_div_stat : 1;
        unsigned int gt_iomcu_sys_div_stat : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_SCTRL_SCIOMCUCLKSTAT_UNION;
#endif
#define SOC_SCTRL_SCIOMCUCLKSTAT_iomcu_pll_ctrl_START (0)
#define SOC_SCTRL_SCIOMCUCLKSTAT_iomcu_pll_ctrl_END (0)
#define SOC_SCTRL_SCIOMCUCLKSTAT_iomcu_pll_div_ctrl_START (1)
#define SOC_SCTRL_SCIOMCUCLKSTAT_iomcu_pll_div_ctrl_END (1)
#define SOC_SCTRL_SCIOMCUCLKSTAT_iomcu_sys_div_ctrl_START (2)
#define SOC_SCTRL_SCIOMCUCLKSTAT_iomcu_sys_div_ctrl_END (2)
#define SOC_SCTRL_SCIOMCUCLKSTAT_gt_iomcu_pll_div_stat_START (3)
#define SOC_SCTRL_SCIOMCUCLKSTAT_gt_iomcu_pll_div_stat_END (3)
#define SOC_SCTRL_SCIOMCUCLKSTAT_gt_iomcu_sys_div_stat_START (4)
#define SOC_SCTRL_SCIOMCUCLKSTAT_gt_iomcu_sys_div_stat_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int nmi_in : 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 29;
    } reg;
} SOC_SCTRL_SCIOMCUCTRL_UNION;
#endif
#define SOC_SCTRL_SCIOMCUCTRL_nmi_in_START (1)
#define SOC_SCTRL_SCIOMCUCTRL_nmi_in_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int iomcu_sysresetreq : 1;
        unsigned int iomcu_m7_state : 3;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int m7_sc_stat : 9;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int iomcu_stat : 10;
    } reg;
} SOC_SCTRL_SCIOMCUSTAT_UNION;
#endif
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_sysresetreq_START (0)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_sysresetreq_END (0)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_m7_state_START (1)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_m7_state_END (3)
#define SOC_SCTRL_SCIOMCUSTAT_m7_sc_stat_START (7)
#define SOC_SCTRL_SCIOMCUSTAT_m7_sc_stat_END (15)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_stat_START (22)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_stat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bisr_repair_done_0 : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS18_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS18_bisr_repair_done_0_START (0)
#define SOC_SCTRL_SCPERSTATUS18_bisr_repair_done_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bisr_repair_done_1 : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS19_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS19_bisr_repair_done_1_START (0)
#define SOC_SCTRL_SCPERSTATUS19_bisr_repair_done_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bisr_mrb_busy_flag_0 : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS20_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS20_bisr_mrb_busy_flag_0_START (0)
#define SOC_SCTRL_SCPERSTATUS20_bisr_mrb_busy_flag_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bisr_mrb_busy_flag_1 : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS21_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS21_bisr_mrb_busy_flag_1_START (0)
#define SOC_SCTRL_SCPERSTATUS21_bisr_mrb_busy_flag_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bisr_dbg_info_0_31_0 : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS22_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS22_bisr_dbg_info_0_31_0_START (0)
#define SOC_SCTRL_SCPERSTATUS22_bisr_dbg_info_0_31_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bisr_dbg_info_0_63_32 : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS23_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS23_bisr_dbg_info_0_63_32_START (0)
#define SOC_SCTRL_SCPERSTATUS23_bisr_dbg_info_0_63_32_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bisr_dbg_info_1_31_0 : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS24_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS24_bisr_dbg_info_1_31_0_START (0)
#define SOC_SCTRL_SCPERSTATUS24_bisr_dbg_info_1_31_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bisr_dbg_info_1_63_32 : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS25_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS25_bisr_dbg_info_1_63_32_START (0)
#define SOC_SCTRL_SCPERSTATUS25_bisr_dbg_info_1_63_32_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS26_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS27_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0lock_cmd0 : 1;
        unsigned int resource0lock_id0 : 3;
        unsigned int resource0lock_cmd1 : 1;
        unsigned int resource0lock_id1 : 3;
        unsigned int resource0lock_cmd2 : 1;
        unsigned int resource0lock_id2 : 3;
        unsigned int resource0lock_cmd3 : 1;
        unsigned int resource0lock_id3 : 3;
        unsigned int resource0lock_cmd4 : 1;
        unsigned int resource0lock_id4 : 3;
        unsigned int resource0lock_cmd5 : 1;
        unsigned int resource0lock_id5 : 3;
        unsigned int resource0lock_cmd6 : 1;
        unsigned int resource0lock_id6 : 3;
        unsigned int resource0lock_cmd7 : 1;
        unsigned int resource0lock_id7 : 3;
    } reg;
} SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_UNION;
#endif
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd0_START (0)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd0_END (0)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id0_START (1)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id0_END (3)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd1_START (4)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd1_END (4)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id1_START (5)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id1_END (7)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd2_START (8)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd2_END (8)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id2_START (9)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id2_END (11)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd3_START (12)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd3_END (12)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id3_START (13)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id3_END (15)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd4_START (16)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd4_END (16)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id4_START (17)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id4_END (19)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd5_START (20)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd5_END (20)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id5_START (21)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id5_END (23)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd6_START (24)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd6_END (24)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id6_START (25)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id6_END (27)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd7_START (28)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd7_END (28)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id7_START (29)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0unlock_cmd0 : 1;
        unsigned int resource0unlock_id0 : 3;
        unsigned int resource0unlock_cmd1 : 1;
        unsigned int resource0unlock_id1 : 3;
        unsigned int resource0unlock_cmd2 : 1;
        unsigned int resource0unlock_id2 : 3;
        unsigned int resource0unlock_cmd3 : 1;
        unsigned int resource0unlock_id3 : 3;
        unsigned int resource0unlock_cmd4 : 1;
        unsigned int resource0unlock_id4 : 3;
        unsigned int resource0unlock_cmd5 : 1;
        unsigned int resource0unlock_id5 : 3;
        unsigned int resource0unlock_cmd6 : 1;
        unsigned int resource0unlock_id6 : 3;
        unsigned int resource0unlock_cmd7 : 1;
        unsigned int resource0unlock_id7 : 3;
    } reg;
} SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_UNION;
#endif
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd0_START (0)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd0_END (0)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id0_START (1)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id0_END (3)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd1_START (4)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd1_END (4)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id1_START (5)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id1_END (7)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd2_START (8)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd2_END (8)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id2_START (9)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id2_END (11)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd3_START (12)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd3_END (12)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id3_START (13)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id3_END (15)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd4_START (16)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd4_END (16)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id4_START (17)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id4_END (19)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd5_START (20)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd5_END (20)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id5_START (21)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id5_END (23)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd6_START (24)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd6_END (24)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id6_START (25)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id6_END (27)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd7_START (28)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd7_END (28)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id7_START (29)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0lock_st0 : 1;
        unsigned int resource0lock_st_id0 : 3;
        unsigned int resource0lock_st1 : 1;
        unsigned int resource0lock_st_id1 : 3;
        unsigned int resource0lock_st2 : 1;
        unsigned int resource0lock_st_id2 : 3;
        unsigned int resource0lock_st3 : 1;
        unsigned int resource0lock_st_id3 : 3;
        unsigned int resource0lock_st4 : 1;
        unsigned int resource0lock_st_id4 : 3;
        unsigned int resource0lock_st5 : 1;
        unsigned int resource0lock_st_id5 : 3;
        unsigned int resource0lock_st6 : 1;
        unsigned int resource0lock_st_id6 : 3;
        unsigned int resource0lock_st7 : 1;
        unsigned int resource0lock_st_id7 : 3;
    } reg;
} SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_UNION;
#endif
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st0_START (0)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st0_END (0)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id0_START (1)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id0_END (3)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st1_START (4)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st1_END (4)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id1_START (5)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id1_END (7)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st2_START (8)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st2_END (8)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id2_START (9)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id2_END (11)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st3_START (12)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st3_END (12)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id3_START (13)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id3_END (15)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st4_START (16)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st4_END (16)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id4_START (17)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id4_END (19)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st5_START (20)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st5_END (20)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id5_START (21)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id5_END (23)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st6_START (24)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st6_END (24)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id6_START (25)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id6_END (27)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st7_START (28)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st7_END (28)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id7_START (29)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0lock_cmd0 : 1;
        unsigned int resource0lock_id0 : 3;
        unsigned int resource0lock_cmd1 : 1;
        unsigned int resource0lock_id1 : 3;
        unsigned int resource0lock_cmd2 : 1;
        unsigned int resource0lock_id2 : 3;
        unsigned int resource0lock_cmd3 : 1;
        unsigned int resource0lock_id3 : 3;
        unsigned int resource0lock_cmd4 : 1;
        unsigned int resource0lock_id4 : 3;
        unsigned int resource0lock_cmd5 : 1;
        unsigned int resource0lock_id5 : 3;
        unsigned int resource0lock_cmd6 : 1;
        unsigned int resource0lock_id6 : 3;
        unsigned int resource0lock_cmd7 : 1;
        unsigned int resource0lock_id7 : 3;
    } reg;
} SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_UNION;
#endif
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd0_START (0)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd0_END (0)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id0_START (1)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id0_END (3)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd1_START (4)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd1_END (4)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id1_START (5)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id1_END (7)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd2_START (8)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd2_END (8)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id2_START (9)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id2_END (11)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd3_START (12)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd3_END (12)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id3_START (13)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id3_END (15)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd4_START (16)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd4_END (16)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id4_START (17)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id4_END (19)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd5_START (20)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd5_END (20)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id5_START (21)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id5_END (23)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd6_START (24)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd6_END (24)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id6_START (25)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id6_END (27)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd7_START (28)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd7_END (28)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id7_START (29)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0unlock_cmd0 : 1;
        unsigned int resource0unlock_id0 : 3;
        unsigned int resource0unlock_cmd1 : 1;
        unsigned int resource0unlock_id1 : 3;
        unsigned int resource0unlock_cmd2 : 1;
        unsigned int resource0unlock_id2 : 3;
        unsigned int resource0unlock_cmd3 : 1;
        unsigned int resource0unlock_id3 : 3;
        unsigned int resource0unlock_cmd4 : 1;
        unsigned int resource0unlock_id4 : 3;
        unsigned int resource0unlock_cmd5 : 1;
        unsigned int resource0unlock_id5 : 3;
        unsigned int resource0unlock_cmd6 : 1;
        unsigned int resource0unlock_id6 : 3;
        unsigned int resource0unlock_cmd7 : 1;
        unsigned int resource0unlock_id7 : 3;
    } reg;
} SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_UNION;
#endif
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd0_START (0)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd0_END (0)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id0_START (1)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id0_END (3)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd1_START (4)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd1_END (4)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id1_START (5)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id1_END (7)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd2_START (8)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd2_END (8)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id2_START (9)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id2_END (11)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd3_START (12)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd3_END (12)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id3_START (13)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id3_END (15)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd4_START (16)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd4_END (16)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id4_START (17)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id4_END (19)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd5_START (20)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd5_END (20)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id5_START (21)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id5_END (23)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd6_START (24)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd6_END (24)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id6_START (25)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id6_END (27)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd7_START (28)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd7_END (28)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id7_START (29)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0lock_st0 : 1;
        unsigned int resource0lock_st_id0 : 3;
        unsigned int resource0lock_st1 : 1;
        unsigned int resource0lock_st_id1 : 3;
        unsigned int resource0lock_st2 : 1;
        unsigned int resource0lock_st_id2 : 3;
        unsigned int resource0lock_st3 : 1;
        unsigned int resource0lock_st_id3 : 3;
        unsigned int resource0lock_st4 : 1;
        unsigned int resource0lock_st_id4 : 3;
        unsigned int resource0lock_st5 : 1;
        unsigned int resource0lock_st_id5 : 3;
        unsigned int resource0lock_st6 : 1;
        unsigned int resource0lock_st_id6 : 3;
        unsigned int resource0lock_st7 : 1;
        unsigned int resource0lock_st_id7 : 3;
    } reg;
} SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_UNION;
#endif
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st0_START (0)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st0_END (0)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id0_START (1)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id0_END (3)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st1_START (4)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st1_END (4)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id1_START (5)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id1_END (7)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st2_START (8)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st2_END (8)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id2_START (9)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id2_END (11)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st3_START (12)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st3_END (12)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id3_START (13)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id3_END (15)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st4_START (16)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st4_END (16)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id4_START (17)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id4_END (19)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st5_START (20)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st5_END (20)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id5_START (21)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id5_END (23)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st6_START (24)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st6_END (24)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id6_START (25)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id6_END (27)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st7_START (28)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st7_END (28)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id7_START (29)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0lock_cmd0 : 1;
        unsigned int resource0lock_id0 : 3;
        unsigned int resource0lock_cmd1 : 1;
        unsigned int resource0lock_id1 : 3;
        unsigned int resource0lock_cmd2 : 1;
        unsigned int resource0lock_id2 : 3;
        unsigned int resource0lock_cmd3 : 1;
        unsigned int resource0lock_id3 : 3;
        unsigned int resource0lock_cmd4 : 1;
        unsigned int resource0lock_id4 : 3;
        unsigned int resource0lock_cmd5 : 1;
        unsigned int resource0lock_id5 : 3;
        unsigned int resource0lock_cmd6 : 1;
        unsigned int resource0lock_id6 : 3;
        unsigned int resource0lock_cmd7 : 1;
        unsigned int resource0lock_id7 : 3;
    } reg;
} SOC_SCTRL_SC_LOCK_CFG_SEC_0_UNION;
#endif
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd0_START (0)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd0_END (0)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id0_START (1)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id0_END (3)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd1_START (4)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd1_END (4)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id1_START (5)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id1_END (7)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd2_START (8)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd2_END (8)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id2_START (9)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id2_END (11)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd3_START (12)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd3_END (12)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id3_START (13)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id3_END (15)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd4_START (16)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd4_END (16)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id4_START (17)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id4_END (19)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd5_START (20)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd5_END (20)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id5_START (21)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id5_END (23)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd6_START (24)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd6_END (24)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id6_START (25)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id6_END (27)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd7_START (28)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd7_END (28)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id7_START (29)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0unlock_cmd0 : 1;
        unsigned int resource0unlock_id0 : 3;
        unsigned int resource0unlock_cmd1 : 1;
        unsigned int resource0unlock_id1 : 3;
        unsigned int resource0unlock_cmd2 : 1;
        unsigned int resource0unlock_id2 : 3;
        unsigned int resource0unlock_cmd3 : 1;
        unsigned int resource0unlock_id3 : 3;
        unsigned int resource0unlock_cmd4 : 1;
        unsigned int resource0unlock_id4 : 3;
        unsigned int resource0unlock_cmd5 : 1;
        unsigned int resource0unlock_id5 : 3;
        unsigned int resource0unlock_cmd6 : 1;
        unsigned int resource0unlock_id6 : 3;
        unsigned int resource0unlock_cmd7 : 1;
        unsigned int resource0unlock_id7 : 3;
    } reg;
} SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_UNION;
#endif
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd0_START (0)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd0_END (0)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id0_START (1)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id0_END (3)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd1_START (4)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd1_END (4)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id1_START (5)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id1_END (7)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd2_START (8)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd2_END (8)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id2_START (9)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id2_END (11)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd3_START (12)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd3_END (12)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id3_START (13)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id3_END (15)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd4_START (16)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd4_END (16)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id4_START (17)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id4_END (19)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd5_START (20)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd5_END (20)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id5_START (21)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id5_END (23)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd6_START (24)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd6_END (24)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id6_START (25)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id6_END (27)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd7_START (28)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd7_END (28)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id7_START (29)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0lock_st0 : 1;
        unsigned int resource0lock_st_id0 : 3;
        unsigned int resource0lock_st1 : 1;
        unsigned int resource0lock_st_id1 : 3;
        unsigned int resource0lock_st2 : 1;
        unsigned int resource0lock_st_id2 : 3;
        unsigned int resource0lock_st3 : 1;
        unsigned int resource0lock_st_id3 : 3;
        unsigned int resource0lock_st4 : 1;
        unsigned int resource0lock_st_id4 : 3;
        unsigned int resource0lock_st5 : 1;
        unsigned int resource0lock_st_id5 : 3;
        unsigned int resource0lock_st6 : 1;
        unsigned int resource0lock_st_id6 : 3;
        unsigned int resource0lock_st7 : 1;
        unsigned int resource0lock_st_id7 : 3;
    } reg;
} SOC_SCTRL_SC_LOCK_STAT_SEC_0_UNION;
#endif
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st0_START (0)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st0_END (0)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id0_START (1)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id0_END (3)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st1_START (4)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st1_END (4)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id1_START (5)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id1_END (7)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st2_START (8)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st2_END (8)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id2_START (9)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id2_END (11)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st3_START (12)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st3_END (12)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id3_START (13)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id3_END (15)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st4_START (16)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st4_END (16)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id4_START (17)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id4_END (19)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st5_START (20)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st5_END (20)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id5_START (21)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id5_END (23)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st6_START (24)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st6_END (24)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id6_START (25)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id6_END (27)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st7_START (28)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st7_END (28)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id7_START (29)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0lock_cmd0 : 1;
        unsigned int resource0lock_id0 : 3;
        unsigned int resource0lock_cmd1 : 1;
        unsigned int resource0lock_id1 : 3;
        unsigned int resource0lock_cmd2 : 1;
        unsigned int resource0lock_id2 : 3;
        unsigned int resource0lock_cmd3 : 1;
        unsigned int resource0lock_id3 : 3;
        unsigned int resource0lock_cmd4 : 1;
        unsigned int resource0lock_id4 : 3;
        unsigned int resource0lock_cmd5 : 1;
        unsigned int resource0lock_id5 : 3;
        unsigned int resource0lock_cmd6 : 1;
        unsigned int resource0lock_id6 : 3;
        unsigned int resource0lock_cmd7 : 1;
        unsigned int resource0lock_id7 : 3;
    } reg;
} SOC_SCTRL_SC_LOCK_CFG_SEC_1_UNION;
#endif
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd0_START (0)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd0_END (0)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id0_START (1)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id0_END (3)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd1_START (4)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd1_END (4)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id1_START (5)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id1_END (7)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd2_START (8)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd2_END (8)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id2_START (9)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id2_END (11)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd3_START (12)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd3_END (12)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id3_START (13)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id3_END (15)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd4_START (16)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd4_END (16)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id4_START (17)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id4_END (19)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd5_START (20)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd5_END (20)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id5_START (21)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id5_END (23)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd6_START (24)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd6_END (24)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id6_START (25)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id6_END (27)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd7_START (28)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd7_END (28)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id7_START (29)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0unlock_cmd0 : 1;
        unsigned int resource0unlock_id0 : 3;
        unsigned int resource0unlock_cmd1 : 1;
        unsigned int resource0unlock_id1 : 3;
        unsigned int resource0unlock_cmd2 : 1;
        unsigned int resource0unlock_id2 : 3;
        unsigned int resource0unlock_cmd3 : 1;
        unsigned int resource0unlock_id3 : 3;
        unsigned int resource0unlock_cmd4 : 1;
        unsigned int resource0unlock_id4 : 3;
        unsigned int resource0unlock_cmd5 : 1;
        unsigned int resource0unlock_id5 : 3;
        unsigned int resource0unlock_cmd6 : 1;
        unsigned int resource0unlock_id6 : 3;
        unsigned int resource0unlock_cmd7 : 1;
        unsigned int resource0unlock_id7 : 3;
    } reg;
} SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_UNION;
#endif
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd0_START (0)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd0_END (0)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id0_START (1)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id0_END (3)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd1_START (4)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd1_END (4)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id1_START (5)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id1_END (7)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd2_START (8)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd2_END (8)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id2_START (9)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id2_END (11)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd3_START (12)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd3_END (12)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id3_START (13)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id3_END (15)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd4_START (16)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd4_END (16)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id4_START (17)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id4_END (19)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd5_START (20)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd5_END (20)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id5_START (21)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id5_END (23)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd6_START (24)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd6_END (24)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id6_START (25)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id6_END (27)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd7_START (28)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd7_END (28)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id7_START (29)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0lock_st0 : 1;
        unsigned int resource0lock_st_id0 : 3;
        unsigned int resource0lock_st1 : 1;
        unsigned int resource0lock_st_id1 : 3;
        unsigned int resource0lock_st2 : 1;
        unsigned int resource0lock_st_id2 : 3;
        unsigned int resource0lock_st3 : 1;
        unsigned int resource0lock_st_id3 : 3;
        unsigned int resource0lock_st4 : 1;
        unsigned int resource0lock_st_id4 : 3;
        unsigned int resource0lock_st5 : 1;
        unsigned int resource0lock_st_id5 : 3;
        unsigned int resource0lock_st6 : 1;
        unsigned int resource0lock_st_id6 : 3;
        unsigned int resource0lock_st7 : 1;
        unsigned int resource0lock_st_id7 : 3;
    } reg;
} SOC_SCTRL_SC_LOCK_STAT_SEC_1_UNION;
#endif
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st0_START (0)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st0_END (0)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id0_START (1)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id0_END (3)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st1_START (4)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st1_END (4)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id1_START (5)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id1_END (7)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st2_START (8)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st2_END (8)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id2_START (9)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id2_END (11)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st3_START (12)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st3_END (12)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id3_START (13)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id3_END (15)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st4_START (16)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st4_END (16)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id4_START (17)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id4_END (19)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st5_START (20)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st5_END (20)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id5_START (21)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id5_END (23)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st6_START (24)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st6_END (24)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id6_START (25)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id6_END (27)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st7_START (28)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st7_END (28)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id7_START (29)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 2;
        unsigned int reserved_2: 29;
    } reg;
} SOC_SCTRL_SCJTAG_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cfg_djtag : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCCFG_DJTAG_UNION;
#endif
#define SOC_SCTRL_SCCFG_DJTAG_cfg_djtag_START (0)
#define SOC_SCTRL_SCCFG_DJTAG_cfg_djtag_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cp15disable0 : 1;
        unsigned int a53_cp15disable1 : 1;
        unsigned int a53_cp15disable2 : 1;
        unsigned int a53_cp15disable3 : 1;
        unsigned int a57_cp15disable0 : 1;
        unsigned int a57_cp15disable1 : 1;
        unsigned int a57_cp15disable2 : 1;
        unsigned int a57_cp15disable3 : 1;
        unsigned int ispa7cp15disable : 1;
        unsigned int gicdisable : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_SCTRL_SCCP15_DISABLE_UNION;
#endif
#define SOC_SCTRL_SCCP15_DISABLE_a53_cp15disable0_START (0)
#define SOC_SCTRL_SCCP15_DISABLE_a53_cp15disable0_END (0)
#define SOC_SCTRL_SCCP15_DISABLE_a53_cp15disable1_START (1)
#define SOC_SCTRL_SCCP15_DISABLE_a53_cp15disable1_END (1)
#define SOC_SCTRL_SCCP15_DISABLE_a53_cp15disable2_START (2)
#define SOC_SCTRL_SCCP15_DISABLE_a53_cp15disable2_END (2)
#define SOC_SCTRL_SCCP15_DISABLE_a53_cp15disable3_START (3)
#define SOC_SCTRL_SCCP15_DISABLE_a53_cp15disable3_END (3)
#define SOC_SCTRL_SCCP15_DISABLE_a57_cp15disable0_START (4)
#define SOC_SCTRL_SCCP15_DISABLE_a57_cp15disable0_END (4)
#define SOC_SCTRL_SCCP15_DISABLE_a57_cp15disable1_START (5)
#define SOC_SCTRL_SCCP15_DISABLE_a57_cp15disable1_END (5)
#define SOC_SCTRL_SCCP15_DISABLE_a57_cp15disable2_START (6)
#define SOC_SCTRL_SCCP15_DISABLE_a57_cp15disable2_END (6)
#define SOC_SCTRL_SCCP15_DISABLE_a57_cp15disable3_START (7)
#define SOC_SCTRL_SCCP15_DISABLE_a57_cp15disable3_END (7)
#define SOC_SCTRL_SCCP15_DISABLE_ispa7cp15disable_START (8)
#define SOC_SCTRL_SCCP15_DISABLE_ispa7cp15disable_END (8)
#define SOC_SCTRL_SCCP15_DISABLE_gicdisable_START (9)
#define SOC_SCTRL_SCCP15_DISABLE_gicdisable_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int clk_cnt : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_SCTRL_SCCLKCNTSTAT_UNION;
#endif
#define SOC_SCTRL_SCCLKCNTSTAT_clk_cnt_START (0)
#define SOC_SCTRL_SCCLKCNTSTAT_clk_cnt_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cnt_cfg : 16;
        unsigned int reserved : 14;
        unsigned int clk_cnt_en : 1;
        unsigned int clk_monitor_en : 1;
    } reg;
} SOC_SCTRL_SCCLKCNTCFG_UNION;
#endif
#define SOC_SCTRL_SCCLKCNTCFG_cnt_cfg_START (0)
#define SOC_SCTRL_SCCLKCNTCFG_cnt_cfg_END (15)
#define SOC_SCTRL_SCCLKCNTCFG_clk_cnt_en_START (30)
#define SOC_SCTRL_SCCLKCNTCFG_clk_cnt_en_END (30)
#define SOC_SCTRL_SCCLKCNTCFG_clk_monitor_en_START (31)
#define SOC_SCTRL_SCCLKCNTCFG_clk_monitor_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_clr : 1;
        unsigned int intr_mask : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_SCTRL_SCCLKMONCTRL_UNION;
#endif
#define SOC_SCTRL_SCCLKMONCTRL_intr_clr_START (0)
#define SOC_SCTRL_SCCLKMONCTRL_intr_clr_END (0)
#define SOC_SCTRL_SCCLKMONCTRL_intr_mask_START (1)
#define SOC_SCTRL_SCCLKMONCTRL_intr_mask_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int raw_intr_stat : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCCLKMONINT_UNION;
#endif
#define SOC_SCTRL_SCCLKMONINT_raw_intr_stat_START (0)
#define SOC_SCTRL_SCCLKMONINT_raw_intr_stat_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cfg_arm_dbgen : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_SCTRL_SCCFG_ARM_DBGEN_UNION;
#endif
#define SOC_SCTRL_SCCFG_ARM_DBGEN_cfg_arm_dbgen_START (0)
#define SOC_SCTRL_SCCFG_ARM_DBGEN_cfg_arm_dbgen_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arm_dbg_key0 : 32;
    } reg;
} SOC_SCTRL_SCARM_DBG_KEY0_UNION;
#endif
#define SOC_SCTRL_SCARM_DBG_KEY0_arm_dbg_key0_START (0)
#define SOC_SCTRL_SCARM_DBG_KEY0_arm_dbg_key0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arm_dbg_key1 : 32;
    } reg;
} SOC_SCTRL_SCARM_DBG_KEY1_UNION;
#endif
#define SOC_SCTRL_SCARM_DBG_KEY1_arm_dbg_key1_START (0)
#define SOC_SCTRL_SCARM_DBG_KEY1_arm_dbg_key1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arm_dbgen_wr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCARM_DBGEN_STAT_UNION;
#endif
#define SOC_SCTRL_SCARM_DBGEN_STAT_arm_dbgen_wr_START (0)
#define SOC_SCTRL_SCARM_DBGEN_STAT_arm_dbgen_wr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sys_efuse_pad_sel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCEFUSECTRL_UNION;
#endif
#define SOC_SCTRL_SCEFUSECTRL_sys_efuse_pad_sel_START (0)
#define SOC_SCTRL_SCEFUSECTRL_sys_efuse_pad_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 31;
    } reg;
} SOC_SCTRL_SCEFUSESEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int CHIP_ID0 : 32;
    } reg;
} SOC_SCTRL_SCCHIP_ID0_UNION;
#endif
#define SOC_SCTRL_SCCHIP_ID0_CHIP_ID0_START (0)
#define SOC_SCTRL_SCCHIP_ID0_CHIP_ID0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int CHIP_ID1 : 32;
    } reg;
} SOC_SCTRL_SCCHIP_ID1_UNION;
#endif
#define SOC_SCTRL_SCCHIP_ID1_CHIP_ID1_START (0)
#define SOC_SCTRL_SCCHIP_ID1_CHIP_ID1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_cryptodisable : 3;
        unsigned int a53_sc_cfg : 5;
        unsigned int reserved : 24;
    } reg;
} SOC_SCTRL_SCCPUSECCTRL_UNION;
#endif
#define SOC_SCTRL_SCCPUSECCTRL_a53_cryptodisable_START (0)
#define SOC_SCTRL_SCCPUSECCTRL_a53_cryptodisable_END (2)
#define SOC_SCTRL_SCCPUSECCTRL_a53_sc_cfg_START (3)
#define SOC_SCTRL_SCCPUSECCTRL_a53_sc_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int jtagsd_sw_sel : 8;
        unsigned int auth_pass : 1;
        unsigned int arm_dbgen_wr1 : 1;
        unsigned int reserved : 15;
        unsigned int jtag_sim_hwselmd : 2;
        unsigned int jtag_sd_hwselmd : 2;
        unsigned int gpio_sd_sim_sel : 1;
        unsigned int jtag_sim_mode : 1;
        unsigned int jtag_sd_mode : 1;
    } reg;
} SOC_SCTRL_SCJTAGSD_SW_SEL_UNION;
#endif
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtagsd_sw_sel_START (0)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtagsd_sw_sel_END (7)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_auth_pass_START (8)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_auth_pass_END (8)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_arm_dbgen_wr1_START (9)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_arm_dbgen_wr1_END (9)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtag_sim_hwselmd_START (25)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtag_sim_hwselmd_END (26)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtag_sd_hwselmd_START (27)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtag_sd_hwselmd_END (28)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_gpio_sd_sim_sel_START (29)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_gpio_sd_sim_sel_END (29)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtag_sim_mode_START (30)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtag_sim_mode_END (30)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtag_sd_mode_START (31)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtag_sd_mode_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int jtagsys_sw_sel : 8;
        unsigned int reserved : 8;
        unsigned int io_fac_test_sel : 16;
    } reg;
} SOC_SCTRL_SCJTAGSYS_SW_SEL_UNION;
#endif
#define SOC_SCTRL_SCJTAGSYS_SW_SEL_jtagsys_sw_sel_START (0)
#define SOC_SCTRL_SCJTAGSYS_SW_SEL_jtagsys_sw_sel_END (7)
#define SOC_SCTRL_SCJTAGSYS_SW_SEL_io_fac_test_sel_START (16)
#define SOC_SCTRL_SCJTAGSYS_SW_SEL_io_fac_test_sel_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_iomcu_enable_syscache_gen : 1;
        unsigned int intr_iomcu_enable_syscache_clr : 1;
        unsigned int intr_iomcu_enable_syscache_msk : 1;
        unsigned int reserved_0 : 1;
        unsigned int intr_iomcu_enable_syscache_raw : 1;
        unsigned int intr_iomcu_enable_syscache_stat : 1;
        unsigned int reserved_1 : 2;
        unsigned int intr_iomcu_disable_syscache_gen : 1;
        unsigned int intr_iomcu_disable_syscache_clr : 1;
        unsigned int intr_iomcu_disable_syscache_msk : 1;
        unsigned int reserved_2 : 1;
        unsigned int intr_iomcu_disable_syscache_raw : 1;
        unsigned int intr_iomcu_disable_syscache_stat : 1;
        unsigned int reserved_3 : 2;
        unsigned int reserved_4 : 16;
    } reg;
} SOC_SCTRL_SC_INTR_SYSCACHE_SEC_UNION;
#endif
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_gen_START (0)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_gen_END (0)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_clr_START (1)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_clr_END (1)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_msk_START (2)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_msk_END (2)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_raw_START (4)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_raw_END (4)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_stat_START (5)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_stat_END (5)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_gen_START (8)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_gen_END (8)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_clr_START (9)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_clr_END (9)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_msk_START (10)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_msk_END (10)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_raw_START (12)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_raw_END (12)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_stat_START (13)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_stat_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spmi_avs_addr0 : 16;
        unsigned int spmi_avs_addr1 : 16;
    } reg;
} SOC_SCTRL_SCSPMIADDR0_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR0_spmi_avs_addr0_START (0)
#define SOC_SCTRL_SCSPMIADDR0_spmi_avs_addr0_END (15)
#define SOC_SCTRL_SCSPMIADDR0_spmi_avs_addr1_START (16)
#define SOC_SCTRL_SCSPMIADDR0_spmi_avs_addr1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spmi_avs_addr2 : 16;
        unsigned int spmi_avs_addr3 : 16;
    } reg;
} SOC_SCTRL_SCSPMIADDR1_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR1_spmi_avs_addr2_START (0)
#define SOC_SCTRL_SCSPMIADDR1_spmi_avs_addr2_END (15)
#define SOC_SCTRL_SCSPMIADDR1_spmi_avs_addr3_START (16)
#define SOC_SCTRL_SCSPMIADDR1_spmi_avs_addr3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spmi_avs_addr4 : 16;
        unsigned int spmi_avs_addr5 : 16;
    } reg;
} SOC_SCTRL_SCSPMIADDR2_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR2_spmi_avs_addr4_START (0)
#define SOC_SCTRL_SCSPMIADDR2_spmi_avs_addr4_END (15)
#define SOC_SCTRL_SCSPMIADDR2_spmi_avs_addr5_START (16)
#define SOC_SCTRL_SCSPMIADDR2_spmi_avs_addr5_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spmi_avs_addr6 : 16;
        unsigned int spmi_avs_addr7 : 16;
    } reg;
} SOC_SCTRL_SCSPMIADDR3_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR3_spmi_avs_addr6_START (0)
#define SOC_SCTRL_SCSPMIADDR3_spmi_avs_addr6_END (15)
#define SOC_SCTRL_SCSPMIADDR3_spmi_avs_addr7_START (16)
#define SOC_SCTRL_SCSPMIADDR3_spmi_avs_addr7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spmi_avs_addr8 : 16;
        unsigned int spmi_avs_addr9 : 16;
    } reg;
} SOC_SCTRL_SCSPMIADDR4_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR4_spmi_avs_addr8_START (0)
#define SOC_SCTRL_SCSPMIADDR4_spmi_avs_addr8_END (15)
#define SOC_SCTRL_SCSPMIADDR4_spmi_avs_addr9_START (16)
#define SOC_SCTRL_SCSPMIADDR4_spmi_avs_addr9_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spmi_avs_addr10 : 16;
        unsigned int spmi_avs_addr11 : 16;
    } reg;
} SOC_SCTRL_SCSPMIADDR5_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR5_spmi_avs_addr10_START (0)
#define SOC_SCTRL_SCSPMIADDR5_spmi_avs_addr10_END (15)
#define SOC_SCTRL_SCSPMIADDR5_spmi_avs_addr11_START (16)
#define SOC_SCTRL_SCSPMIADDR5_spmi_avs_addr11_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int iomcu_mem_bank0_sd_in : 1;
        unsigned int iomcu_mem_bank1_sd_in : 1;
        unsigned int iomcu_mem_bank2_sd_in : 1;
        unsigned int iomcu_mem_bank3_sd_in : 1;
        unsigned int iomcu_mem_bank4_sd_in : 1;
        unsigned int iomcu_mem_bank5_sd_in : 1;
        unsigned int iomcu_mem_bank6_sd_in : 1;
        unsigned int iomcu_mem_bank7_sd_in : 1;
        unsigned int iomcu_mem_bank8_sd_in : 1;
        unsigned int iomcu_mem_bank9_sd_in : 1;
        unsigned int iomcu_mem_bank10_sd_in : 1;
        unsigned int iomcu_mem_bank11_sd_in : 1;
        unsigned int iomcu_mem_bank12_sd_in : 1;
        unsigned int iomcu_mem_bank13_sd_in : 1;
        unsigned int iomcu_mem_bank14_sd_in : 1;
        unsigned int iomcu_mem_bank15_sd_in : 1;
        unsigned int iomcu_mem_sd_in_msk : 16;
    } reg;
} SOC_SCTRL_IOMCU_MEM_SD_IN_UNION;
#endif
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank0_sd_in_START (0)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank0_sd_in_END (0)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank1_sd_in_START (1)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank1_sd_in_END (1)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank2_sd_in_START (2)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank2_sd_in_END (2)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank3_sd_in_START (3)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank3_sd_in_END (3)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank4_sd_in_START (4)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank4_sd_in_END (4)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank5_sd_in_START (5)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank5_sd_in_END (5)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank6_sd_in_START (6)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank6_sd_in_END (6)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank7_sd_in_START (7)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank7_sd_in_END (7)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank8_sd_in_START (8)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank8_sd_in_END (8)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank9_sd_in_START (9)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank9_sd_in_END (9)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank10_sd_in_START (10)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank10_sd_in_END (10)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank11_sd_in_START (11)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank11_sd_in_END (11)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank12_sd_in_START (12)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank12_sd_in_END (12)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank13_sd_in_START (13)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank13_sd_in_END (13)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank14_sd_in_START (14)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank14_sd_in_END (14)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank15_sd_in_START (15)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_bank15_sd_in_END (15)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_sd_in_msk_START (16)
#define SOC_SCTRL_IOMCU_MEM_SD_IN_iomcu_mem_sd_in_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int iomcu_mem_bank0_ds_in : 1;
        unsigned int iomcu_mem_bank1_ds_in : 1;
        unsigned int iomcu_mem_bank2_ds_in : 1;
        unsigned int iomcu_mem_bank3_ds_in : 1;
        unsigned int iomcu_mem_bank4_ds_in : 1;
        unsigned int iomcu_mem_bank5_ds_in : 1;
        unsigned int iomcu_mem_bank6_ds_in : 1;
        unsigned int iomcu_mem_bank7_ds_in : 1;
        unsigned int iomcu_mem_bank8_ds_in : 1;
        unsigned int iomcu_mem_bank9_ds_in : 1;
        unsigned int iomcu_mem_bank10_ds_in : 1;
        unsigned int iomcu_mem_bank11_ds_in : 1;
        unsigned int iomcu_mem_bank12_ds_in : 1;
        unsigned int iomcu_mem_bank13_ds_in : 1;
        unsigned int iomcu_mem_bank14_ds_in : 1;
        unsigned int iomcu_mem_bank15_ds_in : 1;
        unsigned int iomcu_mem_ds_in_msk : 16;
    } reg;
} SOC_SCTRL_IOMCU_MEM_DS_IN_UNION;
#endif
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank0_ds_in_START (0)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank0_ds_in_END (0)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank1_ds_in_START (1)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank1_ds_in_END (1)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank2_ds_in_START (2)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank2_ds_in_END (2)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank3_ds_in_START (3)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank3_ds_in_END (3)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank4_ds_in_START (4)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank4_ds_in_END (4)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank5_ds_in_START (5)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank5_ds_in_END (5)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank6_ds_in_START (6)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank6_ds_in_END (6)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank7_ds_in_START (7)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank7_ds_in_END (7)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank8_ds_in_START (8)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank8_ds_in_END (8)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank9_ds_in_START (9)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank9_ds_in_END (9)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank10_ds_in_START (10)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank10_ds_in_END (10)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank11_ds_in_START (11)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank11_ds_in_END (11)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank12_ds_in_START (12)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank12_ds_in_END (12)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank13_ds_in_START (13)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank13_ds_in_END (13)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank14_ds_in_START (14)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank14_ds_in_END (14)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank15_ds_in_START (15)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_bank15_ds_in_END (15)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_ds_in_msk_START (16)
#define SOC_SCTRL_IOMCU_MEM_DS_IN_iomcu_mem_ds_in_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCHISEEGPIOLOCK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCHISEESPILOCK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCHISEEI2CLOCK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hisee_gpio1_io_sel : 1;
        unsigned int hisee_gpio2_io_sel : 1;
        unsigned int hisee_gpio3_io_sel : 1;
        unsigned int hisee_spi_io_sel : 1;
        unsigned int hisee_i2c_io_sel : 1;
        unsigned int reserved_0 : 26;
        unsigned int reserved_1 : 1;
    } reg;
} SOC_SCTRL_SCHISEEIOSEL_UNION;
#endif
#define SOC_SCTRL_SCHISEEIOSEL_hisee_gpio1_io_sel_START (0)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_gpio1_io_sel_END (0)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_gpio2_io_sel_START (1)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_gpio2_io_sel_END (1)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_gpio3_io_sel_START (2)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_gpio3_io_sel_END (2)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_spi_io_sel_START (3)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_spi_io_sel_END (3)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_i2c_io_sel_START (4)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_i2c_io_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arc_halt_req_a : 1;
        unsigned int arc_run_req_a : 1;
        unsigned int noc_intrleave_gran : 2;
        unsigned int iomcu_tcp_mem_ctrl : 2;
        unsigned int reserved_0 : 4;
        unsigned int reserved_1 : 6;
        unsigned int scperctrl0_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL0_SEC_arc_halt_req_a_START (0)
#define SOC_SCTRL_SCPERCTRL0_SEC_arc_halt_req_a_END (0)
#define SOC_SCTRL_SCPERCTRL0_SEC_arc_run_req_a_START (1)
#define SOC_SCTRL_SCPERCTRL0_SEC_arc_run_req_a_END (1)
#define SOC_SCTRL_SCPERCTRL0_SEC_noc_intrleave_gran_START (2)
#define SOC_SCTRL_SCPERCTRL0_SEC_noc_intrleave_gran_END (3)
#define SOC_SCTRL_SCPERCTRL0_SEC_iomcu_tcp_mem_ctrl_START (4)
#define SOC_SCTRL_SCPERCTRL0_SEC_iomcu_tcp_mem_ctrl_END (5)
#define SOC_SCTRL_SCPERCTRL0_SEC_scperctrl0_sec_msk_START (16)
#define SOC_SCTRL_SCPERCTRL0_SEC_scperctrl0_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ao_tcp_mid : 6;
        unsigned int ufs_mid : 6;
        unsigned int reserved : 4;
        unsigned int scperctrl1_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL1_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL1_SEC_ao_tcp_mid_START (0)
#define SOC_SCTRL_SCPERCTRL1_SEC_ao_tcp_mid_END (5)
#define SOC_SCTRL_SCPERCTRL1_SEC_ufs_mid_START (6)
#define SOC_SCTRL_SCPERCTRL1_SEC_ufs_mid_END (11)
#define SOC_SCTRL_SCPERCTRL1_SEC_scperctrl1_sec_msk_START (16)
#define SOC_SCTRL_SCPERCTRL1_SEC_scperctrl1_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int asp_mem_lp_sc : 6;
        unsigned int reserved : 2;
        unsigned int fdul_lb_sd_in : 8;
        unsigned int scperctrl2_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL2_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL2_SEC_asp_mem_lp_sc_START (0)
#define SOC_SCTRL_SCPERCTRL2_SEC_asp_mem_lp_sc_END (5)
#define SOC_SCTRL_SCPERCTRL2_SEC_fdul_lb_sd_in_START (8)
#define SOC_SCTRL_SCPERCTRL2_SEC_fdul_lb_sd_in_END (15)
#define SOC_SCTRL_SCPERCTRL2_SEC_scperctrl2_sec_msk_START (16)
#define SOC_SCTRL_SCPERCTRL2_SEC_scperctrl2_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifd_ul_sd_in : 3;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 28;
    } reg;
} SOC_SCTRL_SCPERCTRL3_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL3_SEC_hifd_ul_sd_in_START (0)
#define SOC_SCTRL_SCPERCTRL3_SEC_hifd_ul_sd_in_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hisee_gpio1_ack : 4;
        unsigned int hisee_gpio2_ack : 4;
        unsigned int hisee_gpio3_ack : 4;
        unsigned int reserved : 20;
    } reg;
} SOC_SCTRL_SCPERCTRL4_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL4_SEC_hisee_gpio1_ack_START (0)
#define SOC_SCTRL_SCPERCTRL4_SEC_hisee_gpio1_ack_END (3)
#define SOC_SCTRL_SCPERCTRL4_SEC_hisee_gpio2_ack_START (4)
#define SOC_SCTRL_SCPERCTRL4_SEC_hisee_gpio2_ack_END (7)
#define SOC_SCTRL_SCPERCTRL4_SEC_hisee_gpio3_ack_START (8)
#define SOC_SCTRL_SCPERCTRL4_SEC_hisee_gpio3_ack_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ao_tcp_ext_irp_in : 8;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 6;
        unsigned int iomcu_debug_sel : 1;
        unsigned int scperctrl5_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL5_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL5_SEC_ao_tcp_ext_irp_in_START (0)
#define SOC_SCTRL_SCPERCTRL5_SEC_ao_tcp_ext_irp_in_END (7)
#define SOC_SCTRL_SCPERCTRL5_SEC_iomcu_debug_sel_START (15)
#define SOC_SCTRL_SCPERCTRL5_SEC_iomcu_debug_sel_END (15)
#define SOC_SCTRL_SCPERCTRL5_SEC_scperctrl5_sec_msk_START (16)
#define SOC_SCTRL_SCPERCTRL5_SEC_scperctrl5_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mdm5g_ret_ctrl_15to0 : 16;
        unsigned int scperctrl6_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL6_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL6_SEC_mdm5g_ret_ctrl_15to0_START (0)
#define SOC_SCTRL_SCPERCTRL6_SEC_mdm5g_ret_ctrl_15to0_END (15)
#define SOC_SCTRL_SCPERCTRL6_SEC_scperctrl6_sec_msk_START (16)
#define SOC_SCTRL_SCPERCTRL6_SEC_scperctrl6_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mdm5g_ret_ctrl_31to16 : 16;
        unsigned int scperctrl7_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL7_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL7_SEC_mdm5g_ret_ctrl_31to16_START (0)
#define SOC_SCTRL_SCPERCTRL7_SEC_mdm5g_ret_ctrl_31to16_END (15)
#define SOC_SCTRL_SCPERCTRL7_SEC_scperctrl7_sec_msk_START (16)
#define SOC_SCTRL_SCPERCTRL7_SEC_scperctrl7_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mdm5g_ret_ctrl_47to32 : 16;
        unsigned int scperctrl8_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL8_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL8_SEC_mdm5g_ret_ctrl_47to32_START (0)
#define SOC_SCTRL_SCPERCTRL8_SEC_mdm5g_ret_ctrl_47to32_END (15)
#define SOC_SCTRL_SCPERCTRL8_SEC_scperctrl8_sec_msk_START (16)
#define SOC_SCTRL_SCPERCTRL8_SEC_scperctrl8_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 16;
        unsigned int scperctrl9_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL9_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL9_SEC_scperctrl9_sec_msk_START (16)
#define SOC_SCTRL_SCPERCTRL9_SEC_scperctrl9_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bbp_drx_mid : 6;
        unsigned int iomcu_mem_ds_in_2 : 10;
        unsigned int scperctrl10_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL10_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL10_SEC_bbp_drx_mid_START (0)
#define SOC_SCTRL_SCPERCTRL10_SEC_bbp_drx_mid_END (5)
#define SOC_SCTRL_SCPERCTRL10_SEC_iomcu_mem_ds_in_2_START (6)
#define SOC_SCTRL_SCPERCTRL10_SEC_iomcu_mem_ds_in_2_END (15)
#define SOC_SCTRL_SCPERCTRL10_SEC_scperctrl10_sec_msk_START (16)
#define SOC_SCTRL_SCPERCTRL10_SEC_scperctrl10_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int iomcu_tcp_mem_ctrl_num : 5;
        unsigned int reserved : 1;
        unsigned int iomcu_mem_sd_in_2 : 10;
        unsigned int scperctrl11_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL11_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL11_SEC_iomcu_tcp_mem_ctrl_num_START (0)
#define SOC_SCTRL_SCPERCTRL11_SEC_iomcu_tcp_mem_ctrl_num_END (4)
#define SOC_SCTRL_SCPERCTRL11_SEC_iomcu_mem_sd_in_2_START (6)
#define SOC_SCTRL_SCPERCTRL11_SEC_iomcu_mem_sd_in_2_END (15)
#define SOC_SCTRL_SCPERCTRL11_SEC_scperctrl11_sec_msk_START (16)
#define SOC_SCTRL_SCPERCTRL11_SEC_scperctrl11_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arc_run_ack : 1;
        unsigned int arc_halt_ack : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int hifd_ul_sd_out : 3;
        unsigned int fdul_lb_sd_out : 8;
        unsigned int asp_dram_dso : 1;
        unsigned int asp_dram_sdo : 1;
        unsigned int asp_iram_dso : 1;
        unsigned int asp_iram_sdo : 1;
        unsigned int asp_ocram_dso : 1;
        unsigned int asp_ocram_sdo : 1;
        unsigned int aolpram_sdo : 1;
        unsigned int aolpram_dso : 1;
        unsigned int reserved_3 : 7;
        unsigned int reserved_4 : 1;
    } reg;
} SOC_SCTRL_SCPERSTATUS0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS0_SEC_arc_run_ack_START (0)
#define SOC_SCTRL_SCPERSTATUS0_SEC_arc_run_ack_END (0)
#define SOC_SCTRL_SCPERSTATUS0_SEC_arc_halt_ack_START (1)
#define SOC_SCTRL_SCPERSTATUS0_SEC_arc_halt_ack_END (1)
#define SOC_SCTRL_SCPERSTATUS0_SEC_hifd_ul_sd_out_START (5)
#define SOC_SCTRL_SCPERSTATUS0_SEC_hifd_ul_sd_out_END (7)
#define SOC_SCTRL_SCPERSTATUS0_SEC_fdul_lb_sd_out_START (8)
#define SOC_SCTRL_SCPERSTATUS0_SEC_fdul_lb_sd_out_END (15)
#define SOC_SCTRL_SCPERSTATUS0_SEC_asp_dram_dso_START (16)
#define SOC_SCTRL_SCPERSTATUS0_SEC_asp_dram_dso_END (16)
#define SOC_SCTRL_SCPERSTATUS0_SEC_asp_dram_sdo_START (17)
#define SOC_SCTRL_SCPERSTATUS0_SEC_asp_dram_sdo_END (17)
#define SOC_SCTRL_SCPERSTATUS0_SEC_asp_iram_dso_START (18)
#define SOC_SCTRL_SCPERSTATUS0_SEC_asp_iram_dso_END (18)
#define SOC_SCTRL_SCPERSTATUS0_SEC_asp_iram_sdo_START (19)
#define SOC_SCTRL_SCPERSTATUS0_SEC_asp_iram_sdo_END (19)
#define SOC_SCTRL_SCPERSTATUS0_SEC_asp_ocram_dso_START (20)
#define SOC_SCTRL_SCPERSTATUS0_SEC_asp_ocram_dso_END (20)
#define SOC_SCTRL_SCPERSTATUS0_SEC_asp_ocram_sdo_START (21)
#define SOC_SCTRL_SCPERSTATUS0_SEC_asp_ocram_sdo_END (21)
#define SOC_SCTRL_SCPERSTATUS0_SEC_aolpram_sdo_START (22)
#define SOC_SCTRL_SCPERSTATUS0_SEC_aolpram_sdo_END (22)
#define SOC_SCTRL_SCPERSTATUS0_SEC_aolpram_dso_START (23)
#define SOC_SCTRL_SCPERSTATUS0_SEC_aolpram_dso_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS1_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS2_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS3_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vote_req_0 : 7;
        unsigned int vote_en_0 : 1;
        unsigned int vote_req_1 : 7;
        unsigned int vote_en_1 : 1;
        unsigned int sc_tcp_vote_req_0_msk : 16;
    } reg;
} SOC_SCTRL_SC_TCP_VOTE_REQ_0_UNION;
#endif
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_vote_req_0_START (0)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_vote_req_0_END (6)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_vote_en_0_START (7)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_vote_en_0_END (7)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_vote_req_1_START (8)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_vote_req_1_END (14)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_vote_en_1_START (15)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_vote_en_1_END (15)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_sc_tcp_vote_req_0_msk_START (16)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_sc_tcp_vote_req_0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vote_close_req_0 : 1;
        unsigned int vote_open_req_0 : 1;
        unsigned int vote_close_ack_sync_0 : 1;
        unsigned int vote_open_ack_sync_0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int cur_path_stat_0 : 3;
        unsigned int vote_close_req_1 : 1;
        unsigned int vote_open_req_1 : 1;
        unsigned int vote_close_ack_sync_1 : 1;
        unsigned int vote_open_ack_sync_1 : 1;
        unsigned int reserved_1 : 1;
        unsigned int cur_path_stat_1 : 3;
        unsigned int vote_close_req_2 : 1;
        unsigned int vote_open_req_2 : 1;
        unsigned int vote_close_ack_sync_2 : 1;
        unsigned int vote_open_ack_sync_2 : 1;
        unsigned int reserved_2 : 1;
        unsigned int cur_path_stat_2 : 3;
        unsigned int vote_close_req_3 : 1;
        unsigned int vote_open_req_3 : 1;
        unsigned int vote_close_ack_sync_3 : 1;
        unsigned int vote_open_ack_sync_3 : 1;
        unsigned int reserved_3 : 1;
        unsigned int cur_path_stat_3 : 3;
    } reg;
} SOC_SCTRL_SC_TCP_VOTE_STAT_UNION;
#endif
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_req_0_START (0)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_req_0_END (0)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_req_0_START (1)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_req_0_END (1)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_ack_sync_0_START (2)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_ack_sync_0_END (2)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_ack_sync_0_START (3)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_ack_sync_0_END (3)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_cur_path_stat_0_START (5)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_cur_path_stat_0_END (7)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_req_1_START (8)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_req_1_END (8)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_req_1_START (9)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_req_1_END (9)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_ack_sync_1_START (10)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_ack_sync_1_END (10)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_ack_sync_1_START (11)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_ack_sync_1_END (11)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_cur_path_stat_1_START (13)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_cur_path_stat_1_END (15)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_req_2_START (16)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_req_2_END (16)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_req_2_START (17)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_req_2_END (17)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_ack_sync_2_START (18)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_ack_sync_2_END (18)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_ack_sync_2_START (19)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_ack_sync_2_END (19)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_cur_path_stat_2_START (21)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_cur_path_stat_2_END (23)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_req_3_START (24)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_req_3_END (24)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_req_3_START (25)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_req_3_END (25)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_ack_sync_3_START (26)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_ack_sync_3_END (26)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_ack_sync_3_START (27)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_ack_sync_3_END (27)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_cur_path_stat_3_START (29)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_cur_path_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int tcp_busy_stat_sw2fll_en : 1;
        unsigned int tcp_sclk_sw2fll_req : 1;
        unsigned int tcp_busy_stat_sync : 1;
        unsigned int asp_sclk_sw2fll_req_ff2 : 1;
        unsigned int iomcu_sclk_sw2fll_req_ff2 : 1;
        unsigned int int_wakeup_sys_ff2 : 1;
        unsigned int sclk_sw2fll_req_syn : 1;
        unsigned int mad_int_sync : 1;
        unsigned int reserved_2 : 21;
        unsigned int reserved_3 : 1;
    } reg;
} SOC_SCTRL_SC_TCP_VOTE_CTRL0_UNION;
#endif
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_tcp_busy_stat_sw2fll_en_START (2)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_tcp_busy_stat_sw2fll_en_END (2)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_tcp_sclk_sw2fll_req_START (3)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_tcp_sclk_sw2fll_req_END (3)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_tcp_busy_stat_sync_START (4)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_tcp_busy_stat_sync_END (4)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_asp_sclk_sw2fll_req_ff2_START (5)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_asp_sclk_sw2fll_req_ff2_END (5)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_iomcu_sclk_sw2fll_req_ff2_START (6)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_iomcu_sclk_sw2fll_req_ff2_END (6)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_int_wakeup_sys_ff2_START (7)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_int_wakeup_sys_ff2_END (7)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_sclk_sw2fll_req_syn_START (8)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_sclk_sw2fll_req_syn_END (8)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_mad_int_sync_START (9)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_mad_int_sync_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vote_req_2 : 7;
        unsigned int vote_en_2 : 1;
        unsigned int vote_req_3 : 7;
        unsigned int vote_en_3 : 1;
        unsigned int sc_tcp_vote_req_1_msk : 16;
    } reg;
} SOC_SCTRL_SC_TCP_VOTE_REQ_1_UNION;
#endif
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_vote_req_2_START (0)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_vote_req_2_END (6)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_vote_en_2_START (7)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_vote_en_2_END (7)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_vote_req_3_START (8)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_vote_req_3_END (14)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_vote_en_3_START (15)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_vote_en_3_END (15)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_sc_tcp_vote_req_1_msk_START (16)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_sc_tcp_vote_req_1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_pclk_timer1 : 1;
        unsigned int gt_clk_timer1 : 1;
        unsigned int gt_clk_monitor_tcxo : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_efusec : 1;
        unsigned int gt_clk_out0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_ao_gpio1_se : 1;
        unsigned int timer0_clk_change_en : 1;
        unsigned int timer1_clk_change_en : 1;
        unsigned int timer2_clk_change_en : 1;
        unsigned int timer3_clk_change_en : 1;
        unsigned int timer4_clk_change_en : 1;
        unsigned int timer5_clk_change_en : 1;
        unsigned int timer6_clk_change_en : 1;
        unsigned int timer7_clk_change_en : 1;
        unsigned int reserved_2 : 1;
        unsigned int timer8_clk_change_en : 1;
        unsigned int timer_clk_change_en_sw : 1;
        unsigned int gt_pclk_ao_ipc : 1;
        unsigned int gt_clk_ao_cc_32k : 1;
        unsigned int timer17_clk_change_en : 1;
        unsigned int timer16_clk_change_en : 1;
        unsigned int timer15_clk_change_en : 1;
        unsigned int timer14_clk_change_en : 1;
        unsigned int timer13_clk_change_en : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
    } reg;
} SOC_SCTRL_SCPEREN0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_timer1_START (0)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_timer1_END (0)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_timer1_START (1)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_timer1_END (1)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_monitor_tcxo_START (2)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_monitor_tcxo_END (2)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_efusec_START (4)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_efusec_END (4)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_out0_START (5)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_out0_END (5)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_ao_gpio1_se_START (7)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_ao_gpio1_se_END (7)
#define SOC_SCTRL_SCPEREN0_SEC_timer0_clk_change_en_START (8)
#define SOC_SCTRL_SCPEREN0_SEC_timer0_clk_change_en_END (8)
#define SOC_SCTRL_SCPEREN0_SEC_timer1_clk_change_en_START (9)
#define SOC_SCTRL_SCPEREN0_SEC_timer1_clk_change_en_END (9)
#define SOC_SCTRL_SCPEREN0_SEC_timer2_clk_change_en_START (10)
#define SOC_SCTRL_SCPEREN0_SEC_timer2_clk_change_en_END (10)
#define SOC_SCTRL_SCPEREN0_SEC_timer3_clk_change_en_START (11)
#define SOC_SCTRL_SCPEREN0_SEC_timer3_clk_change_en_END (11)
#define SOC_SCTRL_SCPEREN0_SEC_timer4_clk_change_en_START (12)
#define SOC_SCTRL_SCPEREN0_SEC_timer4_clk_change_en_END (12)
#define SOC_SCTRL_SCPEREN0_SEC_timer5_clk_change_en_START (13)
#define SOC_SCTRL_SCPEREN0_SEC_timer5_clk_change_en_END (13)
#define SOC_SCTRL_SCPEREN0_SEC_timer6_clk_change_en_START (14)
#define SOC_SCTRL_SCPEREN0_SEC_timer6_clk_change_en_END (14)
#define SOC_SCTRL_SCPEREN0_SEC_timer7_clk_change_en_START (15)
#define SOC_SCTRL_SCPEREN0_SEC_timer7_clk_change_en_END (15)
#define SOC_SCTRL_SCPEREN0_SEC_timer8_clk_change_en_START (17)
#define SOC_SCTRL_SCPEREN0_SEC_timer8_clk_change_en_END (17)
#define SOC_SCTRL_SCPEREN0_SEC_timer_clk_change_en_sw_START (18)
#define SOC_SCTRL_SCPEREN0_SEC_timer_clk_change_en_sw_END (18)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_ao_ipc_START (19)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_ao_ipc_END (19)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_ao_cc_32k_START (20)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_ao_cc_32k_END (20)
#define SOC_SCTRL_SCPEREN0_SEC_timer17_clk_change_en_START (21)
#define SOC_SCTRL_SCPEREN0_SEC_timer17_clk_change_en_END (21)
#define SOC_SCTRL_SCPEREN0_SEC_timer16_clk_change_en_START (22)
#define SOC_SCTRL_SCPEREN0_SEC_timer16_clk_change_en_END (22)
#define SOC_SCTRL_SCPEREN0_SEC_timer15_clk_change_en_START (23)
#define SOC_SCTRL_SCPEREN0_SEC_timer15_clk_change_en_END (23)
#define SOC_SCTRL_SCPEREN0_SEC_timer14_clk_change_en_START (24)
#define SOC_SCTRL_SCPEREN0_SEC_timer14_clk_change_en_END (24)
#define SOC_SCTRL_SCPEREN0_SEC_timer13_clk_change_en_START (25)
#define SOC_SCTRL_SCPEREN0_SEC_timer13_clk_change_en_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_pclk_timer1 : 1;
        unsigned int gt_clk_timer1 : 1;
        unsigned int gt_clk_monitor_tcxo : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_efusec : 1;
        unsigned int gt_clk_out0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_ao_gpio1_se : 1;
        unsigned int timer0_clk_change_en : 1;
        unsigned int timer1_clk_change_en : 1;
        unsigned int timer2_clk_change_en : 1;
        unsigned int timer3_clk_change_en : 1;
        unsigned int timer4_clk_change_en : 1;
        unsigned int timer5_clk_change_en : 1;
        unsigned int timer6_clk_change_en : 1;
        unsigned int timer7_clk_change_en : 1;
        unsigned int reserved_2 : 1;
        unsigned int timer8_clk_change_en : 1;
        unsigned int timer_clk_change_en_sw : 1;
        unsigned int gt_pclk_ao_ipc : 1;
        unsigned int gt_clk_ao_cc_32k : 1;
        unsigned int timer17_clk_change_en : 1;
        unsigned int timer16_clk_change_en : 1;
        unsigned int timer15_clk_change_en : 1;
        unsigned int timer14_clk_change_en : 1;
        unsigned int timer13_clk_change_en : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
    } reg;
} SOC_SCTRL_SCPERDIS0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_timer1_START (0)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_timer1_END (0)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_timer1_START (1)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_timer1_END (1)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_monitor_tcxo_START (2)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_monitor_tcxo_END (2)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_efusec_START (4)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_efusec_END (4)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_out0_START (5)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_out0_END (5)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_ao_gpio1_se_START (7)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_ao_gpio1_se_END (7)
#define SOC_SCTRL_SCPERDIS0_SEC_timer0_clk_change_en_START (8)
#define SOC_SCTRL_SCPERDIS0_SEC_timer0_clk_change_en_END (8)
#define SOC_SCTRL_SCPERDIS0_SEC_timer1_clk_change_en_START (9)
#define SOC_SCTRL_SCPERDIS0_SEC_timer1_clk_change_en_END (9)
#define SOC_SCTRL_SCPERDIS0_SEC_timer2_clk_change_en_START (10)
#define SOC_SCTRL_SCPERDIS0_SEC_timer2_clk_change_en_END (10)
#define SOC_SCTRL_SCPERDIS0_SEC_timer3_clk_change_en_START (11)
#define SOC_SCTRL_SCPERDIS0_SEC_timer3_clk_change_en_END (11)
#define SOC_SCTRL_SCPERDIS0_SEC_timer4_clk_change_en_START (12)
#define SOC_SCTRL_SCPERDIS0_SEC_timer4_clk_change_en_END (12)
#define SOC_SCTRL_SCPERDIS0_SEC_timer5_clk_change_en_START (13)
#define SOC_SCTRL_SCPERDIS0_SEC_timer5_clk_change_en_END (13)
#define SOC_SCTRL_SCPERDIS0_SEC_timer6_clk_change_en_START (14)
#define SOC_SCTRL_SCPERDIS0_SEC_timer6_clk_change_en_END (14)
#define SOC_SCTRL_SCPERDIS0_SEC_timer7_clk_change_en_START (15)
#define SOC_SCTRL_SCPERDIS0_SEC_timer7_clk_change_en_END (15)
#define SOC_SCTRL_SCPERDIS0_SEC_timer8_clk_change_en_START (17)
#define SOC_SCTRL_SCPERDIS0_SEC_timer8_clk_change_en_END (17)
#define SOC_SCTRL_SCPERDIS0_SEC_timer_clk_change_en_sw_START (18)
#define SOC_SCTRL_SCPERDIS0_SEC_timer_clk_change_en_sw_END (18)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_ao_ipc_START (19)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_ao_ipc_END (19)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_ao_cc_32k_START (20)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_ao_cc_32k_END (20)
#define SOC_SCTRL_SCPERDIS0_SEC_timer17_clk_change_en_START (21)
#define SOC_SCTRL_SCPERDIS0_SEC_timer17_clk_change_en_END (21)
#define SOC_SCTRL_SCPERDIS0_SEC_timer16_clk_change_en_START (22)
#define SOC_SCTRL_SCPERDIS0_SEC_timer16_clk_change_en_END (22)
#define SOC_SCTRL_SCPERDIS0_SEC_timer15_clk_change_en_START (23)
#define SOC_SCTRL_SCPERDIS0_SEC_timer15_clk_change_en_END (23)
#define SOC_SCTRL_SCPERDIS0_SEC_timer14_clk_change_en_START (24)
#define SOC_SCTRL_SCPERDIS0_SEC_timer14_clk_change_en_END (24)
#define SOC_SCTRL_SCPERDIS0_SEC_timer13_clk_change_en_START (25)
#define SOC_SCTRL_SCPERDIS0_SEC_timer13_clk_change_en_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_pclk_timer1 : 1;
        unsigned int gt_clk_timer1 : 1;
        unsigned int gt_clk_monitor_tcxo : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_efusec : 1;
        unsigned int gt_clk_out0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_ao_gpio1_se : 1;
        unsigned int timer0_clk_change_en : 1;
        unsigned int timer1_clk_change_en : 1;
        unsigned int timer2_clk_change_en : 1;
        unsigned int timer3_clk_change_en : 1;
        unsigned int timer4_clk_change_en : 1;
        unsigned int timer5_clk_change_en : 1;
        unsigned int timer6_clk_change_en : 1;
        unsigned int timer7_clk_change_en : 1;
        unsigned int reserved_2 : 1;
        unsigned int timer8_clk_change_en : 1;
        unsigned int timer_clk_change_en_sw : 1;
        unsigned int gt_pclk_ao_ipc : 1;
        unsigned int gt_clk_ao_cc_32k : 1;
        unsigned int timer17_clk_change_en : 1;
        unsigned int timer16_clk_change_en : 1;
        unsigned int timer15_clk_change_en : 1;
        unsigned int timer14_clk_change_en : 1;
        unsigned int timer13_clk_change_en : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_timer1_START (0)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_timer1_END (0)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_timer1_START (1)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_timer1_END (1)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_monitor_tcxo_START (2)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_monitor_tcxo_END (2)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_efusec_START (4)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_efusec_END (4)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_out0_START (5)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_out0_END (5)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_ao_gpio1_se_START (7)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_ao_gpio1_se_END (7)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer0_clk_change_en_START (8)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer0_clk_change_en_END (8)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer1_clk_change_en_START (9)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer1_clk_change_en_END (9)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer2_clk_change_en_START (10)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer2_clk_change_en_END (10)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer3_clk_change_en_START (11)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer3_clk_change_en_END (11)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer4_clk_change_en_START (12)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer4_clk_change_en_END (12)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer5_clk_change_en_START (13)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer5_clk_change_en_END (13)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer6_clk_change_en_START (14)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer6_clk_change_en_END (14)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer7_clk_change_en_START (15)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer7_clk_change_en_END (15)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer8_clk_change_en_START (17)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer8_clk_change_en_END (17)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer_clk_change_en_sw_START (18)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer_clk_change_en_sw_END (18)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_ao_ipc_START (19)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_ao_ipc_END (19)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_ao_cc_32k_START (20)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_ao_cc_32k_END (20)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer17_clk_change_en_START (21)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer17_clk_change_en_END (21)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer16_clk_change_en_START (22)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer16_clk_change_en_END (22)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer15_clk_change_en_START (23)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer15_clk_change_en_END (23)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer14_clk_change_en_START (24)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer14_clk_change_en_END (24)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer13_clk_change_en_START (25)
#define SOC_SCTRL_SCPERCLKEN0_SEC_timer13_clk_change_en_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int timer1_A_en_sel : 1;
        unsigned int timer1_A_en_ov : 1;
        unsigned int timer1_B_en_sel : 1;
        unsigned int timer1_B_en_ov : 1;
        unsigned int reserved : 26;
        unsigned int timer_secu_en : 1;
        unsigned int timerforcehigh : 1;
    } reg;
} SOC_SCTRL_SCTIMERCTRL_SEC_UNION;
#endif
#define SOC_SCTRL_SCTIMERCTRL_SEC_timer1_A_en_sel_START (0)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timer1_A_en_sel_END (0)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timer1_A_en_ov_START (1)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timer1_A_en_ov_END (1)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timer1_B_en_sel_START (2)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timer1_B_en_sel_END (2)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timer1_B_en_ov_START (3)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timer1_B_en_ov_END (3)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timer_secu_en_START (30)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timer_secu_en_END (30)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timerforcehigh_START (31)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timerforcehigh_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_prst_timer1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_ao_ipc : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_prst_ao_gpio1_se : 1;
        unsigned int reserved_2 : 27;
    } reg;
} SOC_SCTRL_SCPERRSTEN0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_timer1_START (0)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_timer1_END (0)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_ao_ipc_START (2)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_ao_ipc_END (2)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_ao_gpio1_se_START (4)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_ao_gpio1_se_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_prst_timer1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_ao_ipc : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_prst_ao_gpio1_se : 1;
        unsigned int reserved_2 : 27;
    } reg;
} SOC_SCTRL_SCPERRSTDIS0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_timer1_START (0)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_timer1_END (0)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_ao_ipc_START (2)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_ao_ipc_END (2)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_ao_gpio1_se_START (4)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_ao_gpio1_se_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_prst_timer1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_ao_ipc : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_prst_ao_gpio1_se : 1;
        unsigned int reserved_2 : 27;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_timer1_START (0)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_timer1_END (0)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_ao_ipc_START (2)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_ao_ipc_END (2)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_ao_gpio1_se_START (4)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_ao_gpio1_se_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_asp_subsys : 1;
        unsigned int ip_rst_asp_subsys_crg : 1;
        unsigned int ip_rst_mad : 1;
        unsigned int ip_rst_asp_cfg : 1;
        unsigned int ip_rst_hifd : 1;
        unsigned int ip_rst_hifd_crg : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SCTRL_SCPERRSTEN1_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_subsys_START (0)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_subsys_END (0)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_subsys_crg_START (1)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_subsys_crg_END (1)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_mad_START (2)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_mad_END (2)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_cfg_START (3)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_cfg_END (3)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_hifd_START (4)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_hifd_END (4)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_hifd_crg_START (5)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_hifd_crg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_asp_subsys : 1;
        unsigned int ip_rst_asp_subsys_crg : 1;
        unsigned int ip_rst_mad : 1;
        unsigned int ip_rst_asp_cfg : 1;
        unsigned int ip_rst_hifd : 1;
        unsigned int ip_rst_hifd_crg : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SCTRL_SCPERRSTDIS1_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_subsys_START (0)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_subsys_END (0)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_subsys_crg_START (1)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_subsys_crg_END (1)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_mad_START (2)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_mad_END (2)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_cfg_START (3)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_cfg_END (3)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_hifd_START (4)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_hifd_END (4)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_hifd_crg_START (5)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_hifd_crg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_asp_subsys : 1;
        unsigned int ip_rst_asp_subsys_crg : 1;
        unsigned int ip_rst_mad : 1;
        unsigned int ip_rst_asp_cfg : 1;
        unsigned int ip_rst_hifd : 1;
        unsigned int ip_rst_hifd_crg : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT1_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_subsys_START (0)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_subsys_END (0)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_subsys_crg_START (1)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_subsys_crg_END (1)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_mad_START (2)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_mad_END (2)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_cfg_START (3)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_cfg_END (3)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_hifd_START (4)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_hifd_END (4)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_hifd_crg_START (5)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_hifd_crg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_iomcu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCPERRSTEN2_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN2_SEC_ip_rst_iomcu_START (0)
#define SOC_SCTRL_SCPERRSTEN2_SEC_ip_rst_iomcu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_iomcu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCPERRSTDIS2_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS2_SEC_ip_rst_iomcu_START (0)
#define SOC_SCTRL_SCPERRSTDIS2_SEC_ip_rst_iomcu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_iomcu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT2_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT2_SEC_ip_rst_iomcu_START (0)
#define SOC_SCTRL_SCPERRSTSTAT2_SEC_ip_rst_iomcu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 30;
    } reg;
} SOC_SCTRL_SCPERRSTEN3_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 30;
    } reg;
} SOC_SCTRL_SCPERRSTDIS3_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 30;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT3_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_asp_subsys_lpmcu : 1;
        unsigned int gt_clk_asp_codec_lpm3 : 1;
        unsigned int gt_clk_mad_lpm3 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 28;
    } reg;
} SOC_SCTRL_SCPEREN3_SEC_UNION;
#endif
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_asp_subsys_lpmcu_START (0)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_asp_subsys_lpmcu_END (0)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_asp_codec_lpm3_START (1)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_asp_codec_lpm3_END (1)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_mad_lpm3_START (2)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_mad_lpm3_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_asp_subsys_lpmcu : 1;
        unsigned int gt_clk_asp_codec_lpm3 : 1;
        unsigned int gt_clk_mad_lpm3 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 28;
    } reg;
} SOC_SCTRL_SCPERDIS3_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_asp_subsys_lpmcu_START (0)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_asp_subsys_lpmcu_END (0)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_asp_codec_lpm3_START (1)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_asp_codec_lpm3_END (1)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_mad_lpm3_START (2)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_mad_lpm3_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_asp_subsys_lpmcu : 1;
        unsigned int gt_clk_asp_codec_lpm3 : 1;
        unsigned int gt_clk_mad_lpm3 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 28;
    } reg;
} SOC_SCTRL_SCPERCLKEN3_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_asp_subsys_lpmcu_START (0)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_asp_subsys_lpmcu_END (0)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_asp_codec_lpm3_START (1)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_asp_codec_lpm3_END (1)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_mad_lpm3_START (2)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_mad_lpm3_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 30;
    } reg;
} SOC_SCTRL_SCPERRSTEN4_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 30;
    } reg;
} SOC_SCTRL_SCPERRSTDIS4_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 30;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT4_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int version_code : 16;
        unsigned int chip_code : 16;
    } reg;
} SOC_SCTRL_SCSOCID0_UNION;
#endif
#define SOC_SCTRL_SCSOCID0_version_code_START (0)
#define SOC_SCTRL_SCSOCID0_version_code_END (15)
#define SOC_SCTRL_SCSOCID0_chip_code_START (16)
#define SOC_SCTRL_SCSOCID0_chip_code_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTAT_POR_RESERVED0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTAT_POR_RESERVED1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTAT_POR_RESERVED2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTAT_POR_RESERVED3_UNION;
#endif
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
