#ifndef __SOC_SCTRL_INTERFACE_H__
#define __SOC_SCTRL_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_SCTRL_SCCTRL_ADDR(base) ((base) + (0x000))
#define SOC_SCTRL_SCSYSSTAT_ADDR(base) ((base) + (0x004))
#define SOC_SCTRL_SCDEEPSLEEPED_ADDR(base) ((base) + (0x008))
#define SOC_SCTRL_SCEFUSEDATA2_ADDR(base) ((base) + (0x00C))
#define SOC_SCTRL_SCEFUSEDATA3_ADDR(base) ((base) + (0x010))
#define SOC_SCTRL_SCXTALCTRL_ADDR(base) ((base) + (0x020))
#define SOC_SCTRL_SCXTALTIMEOUT0_ADDR(base) ((base) + (0x024))
#define SOC_SCTRL_SCXTALTIMEOUT1_ADDR(base) ((base) + (0x028))
#define SOC_SCTRL_SCXTALSTAT_ADDR(base) ((base) + (0x02C))
#define SOC_SCTRL_SCSW2FLLBYPASS_ADDR(base) ((base) + (0x030))
#define SOC_SCTRL_SCFLLLOCK_FILTER_ADDR(base) ((base) + (0x034))
#define SOC_SCTRL_SCLPSTATCFG_ADDR(base) ((base) + (0x038))
#define SOC_SCTRL_SCISOEN_ADDR(base) ((base) + (0x040))
#define SOC_SCTRL_SCISODIS_ADDR(base) ((base) + (0x044))
#define SOC_SCTRL_SCISOSTAT_ADDR(base) ((base) + (0x048))
#define SOC_SCTRL_SCPWREN_ADDR(base) ((base) + (0x060))
#define SOC_SCTRL_SCPWRDIS_ADDR(base) ((base) + (0x064))
#define SOC_SCTRL_SCPWRSTAT_ADDR(base) ((base) + (0x068))
#define SOC_SCTRL_SCPWRACK_ADDR(base) ((base) + (0x06C))
#define SOC_SCTRL_SCPERPWRDOWNTIME_ADDR(base) ((base) + (0x070))
#define SOC_SCTRL_SCPERPWRUPTIME_ADDR(base) ((base) + (0x074))
#define SOC_SCTRL_SCINT_GATHER_STAT_ADDR(base) ((base) + (0x0A0))
#define SOC_SCTRL_SCINT_MASK_ADDR(base) ((base) + (0x0A4))
#define SOC_SCTRL_SCINT_STAT_ADDR(base) ((base) + (0x0A8))
#define SOC_SCTRL_SCDRX_INT_CFG_ADDR(base) ((base) + (0x0AC))
#define SOC_SCTRL_SCLPMCUWFI_INT_ADDR(base) ((base) + (0x0B0))
#define SOC_SCTRL_SCINT_MASK1_ADDR(base) ((base) + (0x0B4))
#define SOC_SCTRL_SCINT_STAT1_ADDR(base) ((base) + (0x0B8))
#define SOC_SCTRL_SCINT_MASK2_ADDR(base) ((base) + (0x0BC))
#define SOC_SCTRL_SCINT_STAT2_ADDR(base) ((base) + (0x0C0))
#define SOC_SCTRL_SCLBINTPLLCTRL0_ADDR(base) ((base) + (0x0F0))
#define SOC_SCTRL_SCLBINTPLLCTRL1_ADDR(base) ((base) + (0x0F4))
#define SOC_SCTRL_SCLBINTPLLCTRL2_ADDR(base) ((base) + (0x0F8))
#define SOC_SCTRL_SCLBINTPLLSTAT_ADDR(base) ((base) + (0x0FC))
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
#define SOC_SCTRL_SCPERRSTEN0_ADDR(base) ((base) + (0x200))
#define SOC_SCTRL_SCPERRSTDIS0_ADDR(base) ((base) + (0x204))
#define SOC_SCTRL_SCPERRSTSTAT0_ADDR(base) ((base) + (0x208))
#define SOC_SCTRL_SCPERRSTEN1_ADDR(base) ((base) + (0x20C))
#define SOC_SCTRL_SCPERRSTDIS1_ADDR(base) ((base) + (0x210))
#define SOC_SCTRL_SCPERRSTSTAT1_ADDR(base) ((base) + (0x214))
#define SOC_SCTRL_SCPERRSTEN2_ADDR(base) ((base) + (0x218))
#define SOC_SCTRL_SCPERRSTDIS2_ADDR(base) ((base) + (0x21C))
#define SOC_SCTRL_SCPERRSTSTAT2_ADDR(base) ((base) + (0x220))
#define SOC_SCTRL_SCIPCLKRSTBUS_ADDR(base) ((base) + (0x240))
#define SOC_SCTRL_SCIPCLKRSTBUS1_ADDR(base) ((base) + (0x244))
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
#define SOC_SCTRL_SCPERCTRL0_ADDR(base) ((base) + (0x300))
#define SOC_SCTRL_SCPERCTRL1_ADDR(base) ((base) + (0x304))
#define SOC_SCTRL_SCPERCTRL2_ADDR(base) ((base) + (0x308))
#define SOC_SCTRL_SCPERCTRL3_ADDR(base) ((base) + (0x30C))
#define SOC_SCTRL_SCPERCTRL4_ADDR(base) ((base) + (0x310))
#define SOC_SCTRL_SCPERCTRL5_ADDR(base) ((base) + (0x314))
#define SOC_SCTRL_SCPERCTRL6_ADDR(base) ((base) + (0x318))
#define SOC_SCTRL_SCPERCTRL7_ADDR(base) ((base) + (0x31C))
#define SOC_SCTRL_SCPERCTRL8_ADDR(base) ((base) + (0x320))
#define SOC_SCTRL_SCPERCTRL9_ADDR(base) ((base) + (0x324))
#define SOC_SCTRL_SCPERCTRL10_ADDR(base) ((base) + (0x328))
#define SOC_SCTRL_SCPERCTRL11_ADDR(base) ((base) + (0x32C))
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
#define SOC_SCTRL_SCINNERCTRL_ADDR(base) ((base) + (0x3B0))
#define SOC_SCTRL_SCTIMERCTRL0_ADDR(base) ((base) + (0x3C0))
#define SOC_SCTRL_SCTIMERCTRL1_ADDR(base) ((base) + (0x3C4))
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
#define SOC_SCTRL_SCLPMCUCLKEN_ADDR(base) ((base) + (0x480))
#define SOC_SCTRL_SCLPMCUCLKDIS_ADDR(base) ((base) + (0x484))
#define SOC_SCTRL_SCLPMCUCLKSTAT_ADDR(base) ((base) + (0x488))
#define SOC_SCTRL_SCLPMCURSTEN_ADDR(base) ((base) + (0x500))
#define SOC_SCTRL_SCLPMCURSTDIS_ADDR(base) ((base) + (0x504))
#define SOC_SCTRL_SCLPMCURSTSTAT_ADDR(base) ((base) + (0x508))
#define SOC_SCTRL_SCLPMCUCTRL_ADDR(base) ((base) + (0x510))
#define SOC_SCTRL_SCLPMCUSTAT_ADDR(base) ((base) + (0x514))
#define SOC_SCTRL_SCLPMCURAMCTRL_ADDR(base) ((base) + (0x518))
#define SOC_SCTRL_SCOCBCSTAT_ADDR(base) ((base) + (0x51C))
#define SOC_SCTRL_SCOCBCCTRL_ADDR(base) ((base) + (0x520))
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
#define SOC_SCTRL_SCEVENT_STAT_ADDR(base) ((base) + (0x564))
#define SOC_SCTRL_SCISPA7_EVENT_MASK_ADDR(base) ((base) + (0x568))
#define SOC_SCTRL_SCOCBC_EVENT_MASK_ADDR(base) ((base) + (0x56C))
#define SOC_SCTRL_SCIOMCUCLKCTRL_ADDR(base) ((base) + (0x580))
#define SOC_SCTRL_SCIOMCUCLKSTAT_ADDR(base) ((base) + (0x584))
#define SOC_SCTRL_SCIOMCUCTRL_ADDR(base) ((base) + (0x588))
#define SOC_SCTRL_SCIOMCUSTAT_ADDR(base) ((base) + (0x58C))
#define SOC_SCTRL_SCJTAG_SEL_ADDR(base) ((base) + (0x800))
#define SOC_SCTRL_SCMODEM_KEY0_ADDR(base) ((base) + (0x804))
#define SOC_SCTRL_SCMODEM_KEY1_ADDR(base) ((base) + (0x808))
#define SOC_SCTRL_SCMODEM_KEY2_ADDR(base) ((base) + (0x80C))
#define SOC_SCTRL_SCMODEM_KEY3_ADDR(base) ((base) + (0x810))
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
#define SOC_SCTRL_SC_PERI_SEC_CTRL_ADDR(base) ((base) + (0x860))
#define SOC_SCTRL_SCOCBCFAMACTRL_ADDR(base) ((base) + (0x880))
#define SOC_SCTRL_SCALIGNMODE_ADDR(base) ((base) + (0x884))
#define SOC_SCTRL_SCSPMIADDR0_ADDR(base) ((base) + (0x888))
#define SOC_SCTRL_SCSPMIADDR1_ADDR(base) ((base) + (0x88C))
#define SOC_SCTRL_SCSPMIADDR2_ADDR(base) ((base) + (0x890))
#define SOC_SCTRL_SCHISEEGPIOLOCK_ADDR(base) ((base) + (0x894))
#define SOC_SCTRL_SCHISEESPILOCK_ADDR(base) ((base) + (0x898))
#define SOC_SCTRL_SCHISEEI2CLOCK_ADDR(base) ((base) + (0x89C))
#define SOC_SCTRL_SCHISEEIOSEL_ADDR(base) ((base) + (0x8A0))
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
#define SOC_SCTRL_SCOCBCRSTCTRL_ADDR(base) ((base) + (0xD0C))
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ADDR(base) ((base) + (0xD10))
#define SOC_SCTRL_OCBC_HWCLK_MODE_CTRL_ADDR(base) ((base) + (0xD14))
#define SOC_SCTRL_OCBC_AOCLK_CTRL_STAT_ADDR(base) ((base) + (0xD18))
#define SOC_SCTRL_OCBC_AOCLK_DEBUG_ADDR(base) ((base) + (0xD1C))
#define SOC_SCTRL_OCBC_HWM_AOCLK_CTRL_ADDR(base) ((base) + (0xD20))
#define SOC_SCTRL_SCSOCID0_ADDR(base) ((base) + (0xE00))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED0_ADDR(base) ((base) + (0xE10))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED1_ADDR(base) ((base) + (0xE14))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED2_ADDR(base) ((base) + (0xE18))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED3_ADDR(base) ((base) + (0xE1C))
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
        unsigned int reserved_1 : 6;
    } reg;
} SOC_SCTRL_SCCTRL_UNION;
#endif
#define SOC_SCTRL_SCCTRL_mode_ctrl_soft_START (0)
#define SOC_SCTRL_SCCTRL_mode_ctrl_soft_END (2)
#define SOC_SCTRL_SCCTRL_sys_mode_START (3)
#define SOC_SCTRL_SCCTRL_sys_mode_END (6)
#define SOC_SCTRL_SCCTRL_deepsleep_en_START (25)
#define SOC_SCTRL_SCCTRL_deepsleep_en_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int a53_tsensor0_rst_req : 1;
        unsigned int a53_tsensor1_rst_req : 1;
        unsigned int g3d_tsensor_rst_req : 1;
        unsigned int soft_rst_req : 1;
        unsigned int wd0_rst_req : 1;
        unsigned int wd1_rst_req : 1;
        unsigned int lpmcu_rst_req : 1;
        unsigned int modem_tsensor_rst_req : 1;
        unsigned int iomcu_rst_req : 1;
        unsigned int asp_subsys_wd_req : 1;
        unsigned int reserved_0 : 1;
        unsigned int modem_wd_rst_req : 1;
        unsigned int ivp32dsp_wd_rst_req : 1;
        unsigned int isp_a7_wd_rst_req : 1;
        unsigned int wait_ddr_selfreflash_timeout : 1;
        unsigned int ddr_fatal_intr : 2;
        unsigned int reserved_1 : 2;
        unsigned int ddr_uce_wd_rst_req : 2;
        unsigned int reserved_2 : 11;
    } reg;
} SOC_SCTRL_SCSYSSTAT_UNION;
#endif
#define SOC_SCTRL_SCSYSSTAT_a53_tsensor0_rst_req_START (0)
#define SOC_SCTRL_SCSYSSTAT_a53_tsensor0_rst_req_END (0)
#define SOC_SCTRL_SCSYSSTAT_a53_tsensor1_rst_req_START (1)
#define SOC_SCTRL_SCSYSSTAT_a53_tsensor1_rst_req_END (1)
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
#define SOC_SCTRL_SCSYSSTAT_modem_wd_rst_req_START (11)
#define SOC_SCTRL_SCSYSSTAT_modem_wd_rst_req_END (11)
#define SOC_SCTRL_SCSYSSTAT_ivp32dsp_wd_rst_req_START (12)
#define SOC_SCTRL_SCSYSSTAT_ivp32dsp_wd_rst_req_END (12)
#define SOC_SCTRL_SCSYSSTAT_isp_a7_wd_rst_req_START (13)
#define SOC_SCTRL_SCSYSSTAT_isp_a7_wd_rst_req_END (13)
#define SOC_SCTRL_SCSYSSTAT_wait_ddr_selfreflash_timeout_START (14)
#define SOC_SCTRL_SCSYSSTAT_wait_ddr_selfreflash_timeout_END (14)
#define SOC_SCTRL_SCSYSSTAT_ddr_fatal_intr_START (15)
#define SOC_SCTRL_SCSYSSTAT_ddr_fatal_intr_END (16)
#define SOC_SCTRL_SCSYSSTAT_ddr_uce_wd_rst_req_START (19)
#define SOC_SCTRL_SCSYSSTAT_ddr_uce_wd_rst_req_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int deepsleeped : 1;
        unsigned int reserved_0 : 3;
        unsigned int sleeped : 1;
        unsigned int reserved_1 : 15;
        unsigned int reserved_2 : 11;
        unsigned int reserved_3 : 1;
    } reg;
} SOC_SCTRL_SCDEEPSLEEPED_UNION;
#endif
#define SOC_SCTRL_SCDEEPSLEEPED_deepsleeped_START (0)
#define SOC_SCTRL_SCDEEPSLEEPED_deepsleeped_END (0)
#define SOC_SCTRL_SCDEEPSLEEPED_sleeped_START (4)
#define SOC_SCTRL_SCDEEPSLEEPED_sleeped_END (4)
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
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCSW2FLLBYPASS_UNION;
#endif
#define SOC_SCTRL_SCSW2FLLBYPASS_sclk_sw2fll_bypass_START (0)
#define SOC_SCTRL_SCSW2FLLBYPASS_sclk_sw2fll_bypass_END (0)
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
        unsigned int audioisoen : 1;
        unsigned int iomcuisoen : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int pciephyisoen : 1;
        unsigned int meidia_subsys_ao_iso_en : 1;
        unsigned int reserved_3 : 25;
    } reg;
} SOC_SCTRL_SCISOEN_UNION;
#endif
#define SOC_SCTRL_SCISOEN_audioisoen_START (0)
#define SOC_SCTRL_SCISOEN_audioisoen_END (0)
#define SOC_SCTRL_SCISOEN_iomcuisoen_START (1)
#define SOC_SCTRL_SCISOEN_iomcuisoen_END (1)
#define SOC_SCTRL_SCISOEN_pciephyisoen_START (5)
#define SOC_SCTRL_SCISOEN_pciephyisoen_END (5)
#define SOC_SCTRL_SCISOEN_meidia_subsys_ao_iso_en_START (6)
#define SOC_SCTRL_SCISOEN_meidia_subsys_ao_iso_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audioisoen : 1;
        unsigned int iomcuisoen : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int pciephyisoen : 1;
        unsigned int meidia_subsys_ao_iso_en : 1;
        unsigned int reserved_3 : 25;
    } reg;
} SOC_SCTRL_SCISODIS_UNION;
#endif
#define SOC_SCTRL_SCISODIS_audioisoen_START (0)
#define SOC_SCTRL_SCISODIS_audioisoen_END (0)
#define SOC_SCTRL_SCISODIS_iomcuisoen_START (1)
#define SOC_SCTRL_SCISODIS_iomcuisoen_END (1)
#define SOC_SCTRL_SCISODIS_pciephyisoen_START (5)
#define SOC_SCTRL_SCISODIS_pciephyisoen_END (5)
#define SOC_SCTRL_SCISODIS_meidia_subsys_ao_iso_en_START (6)
#define SOC_SCTRL_SCISODIS_meidia_subsys_ao_iso_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audioisoen : 1;
        unsigned int iomcuisoen : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int pciephyisoen : 1;
        unsigned int meidia_subsys_ao_iso_en : 1;
        unsigned int reserved_3 : 25;
    } reg;
} SOC_SCTRL_SCISOSTAT_UNION;
#endif
#define SOC_SCTRL_SCISOSTAT_audioisoen_START (0)
#define SOC_SCTRL_SCISOSTAT_audioisoen_END (0)
#define SOC_SCTRL_SCISOSTAT_iomcuisoen_START (1)
#define SOC_SCTRL_SCISOSTAT_iomcuisoen_END (1)
#define SOC_SCTRL_SCISOSTAT_pciephyisoen_START (5)
#define SOC_SCTRL_SCISOSTAT_pciephyisoen_END (5)
#define SOC_SCTRL_SCISOSTAT_meidia_subsys_ao_iso_en_START (6)
#define SOC_SCTRL_SCISOSTAT_meidia_subsys_ao_iso_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audiopwren : 1;
        unsigned int iomcupwren : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 27;
    } reg;
} SOC_SCTRL_SCPWREN_UNION;
#endif
#define SOC_SCTRL_SCPWREN_audiopwren_START (0)
#define SOC_SCTRL_SCPWREN_audiopwren_END (0)
#define SOC_SCTRL_SCPWREN_iomcupwren_START (1)
#define SOC_SCTRL_SCPWREN_iomcupwren_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audiopwren : 1;
        unsigned int iomcupwren : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 27;
    } reg;
} SOC_SCTRL_SCPWRDIS_UNION;
#endif
#define SOC_SCTRL_SCPWRDIS_audiopwren_START (0)
#define SOC_SCTRL_SCPWRDIS_audiopwren_END (0)
#define SOC_SCTRL_SCPWRDIS_iomcupwren_START (1)
#define SOC_SCTRL_SCPWRDIS_iomcupwren_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audiopwrstat : 1;
        unsigned int iomcupwrstat : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 27;
    } reg;
} SOC_SCTRL_SCPWRSTAT_UNION;
#endif
#define SOC_SCTRL_SCPWRSTAT_audiopwrstat_START (0)
#define SOC_SCTRL_SCPWRSTAT_audiopwrstat_END (0)
#define SOC_SCTRL_SCPWRSTAT_iomcupwrstat_START (1)
#define SOC_SCTRL_SCPWRSTAT_iomcupwrstat_END (1)
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
        unsigned int rtc_int : 1;
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
#define SOC_SCTRL_SCINT_MASK_rtc_int_START (6)
#define SOC_SCTRL_SCINT_MASK_rtc_int_END (6)
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
        unsigned int rtc_int : 1;
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
#define SOC_SCTRL_SCINT_STAT_rtc_int_START (6)
#define SOC_SCTRL_SCINT_STAT_rtc_int_END (6)
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
        unsigned int drx0_int : 1;
        unsigned int drx1_int : 1;
        unsigned int drx2_int : 1;
        unsigned int drx3_int : 1;
        unsigned int drx4_int : 1;
        unsigned int intr_iomcu_gpio_comb : 1;
        unsigned int drx5_int : 1;
        unsigned int drx6_int : 1;
        unsigned int drx7_int : 1;
        unsigned int drx8_int : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_SCTRL_SCDRX_INT_CFG_UNION;
#endif
#define SOC_SCTRL_SCDRX_INT_CFG_drx0_int_START (0)
#define SOC_SCTRL_SCDRX_INT_CFG_drx0_int_END (0)
#define SOC_SCTRL_SCDRX_INT_CFG_drx1_int_START (1)
#define SOC_SCTRL_SCDRX_INT_CFG_drx1_int_END (1)
#define SOC_SCTRL_SCDRX_INT_CFG_drx2_int_START (2)
#define SOC_SCTRL_SCDRX_INT_CFG_drx2_int_END (2)
#define SOC_SCTRL_SCDRX_INT_CFG_drx3_int_START (3)
#define SOC_SCTRL_SCDRX_INT_CFG_drx3_int_END (3)
#define SOC_SCTRL_SCDRX_INT_CFG_drx4_int_START (4)
#define SOC_SCTRL_SCDRX_INT_CFG_drx4_int_END (4)
#define SOC_SCTRL_SCDRX_INT_CFG_intr_iomcu_gpio_comb_START (5)
#define SOC_SCTRL_SCDRX_INT_CFG_intr_iomcu_gpio_comb_END (5)
#define SOC_SCTRL_SCDRX_INT_CFG_drx5_int_START (6)
#define SOC_SCTRL_SCDRX_INT_CFG_drx5_int_END (6)
#define SOC_SCTRL_SCDRX_INT_CFG_drx6_int_START (7)
#define SOC_SCTRL_SCDRX_INT_CFG_drx6_int_END (7)
#define SOC_SCTRL_SCDRX_INT_CFG_drx7_int_START (8)
#define SOC_SCTRL_SCDRX_INT_CFG_drx7_int_END (8)
#define SOC_SCTRL_SCDRX_INT_CFG_drx8_int_START (9)
#define SOC_SCTRL_SCDRX_INT_CFG_drx8_int_END (9)
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
        unsigned int drx0_int : 1;
        unsigned int drx1_int : 1;
        unsigned int drx2_int : 1;
        unsigned int drx3_int : 1;
        unsigned int drx4_int : 1;
        unsigned int drx5_int : 1;
        unsigned int drx6_int : 1;
        unsigned int drx7_int : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_asp_cfg_t_maintimeout_msksta : 1;
        unsigned int iomcu_ahb_slv_maintimeout_msksta : 1;
        unsigned int iomcu_apb_slv_maintimeout_msksta : 1;
        unsigned int reserved_1 : 1;
        unsigned int sc_noc_aon_apb_slv_t_maintimeout_msksta : 1;
        unsigned int reserved_2 : 1;
        unsigned int drx8_int : 1;
        unsigned int ao_ipc_int16 : 1;
        unsigned int ao_ipc_int17 : 1;
        unsigned int ao_ipc_int18 : 1;
        unsigned int ao_ipc_int19 : 1;
        unsigned int reserved_3 : 1;
        unsigned int ao_ipc_int0 : 1;
        unsigned int reserved_4 : 4;
        unsigned int gpio_18_int : 1;
        unsigned int gpio_19_int : 1;
        unsigned int gpio_28_int : 1;
        unsigned int se_gpio1 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
    } reg;
} SOC_SCTRL_SCINT_MASK1_UNION;
#endif
#define SOC_SCTRL_SCINT_MASK1_drx0_int_START (0)
#define SOC_SCTRL_SCINT_MASK1_drx0_int_END (0)
#define SOC_SCTRL_SCINT_MASK1_drx1_int_START (1)
#define SOC_SCTRL_SCINT_MASK1_drx1_int_END (1)
#define SOC_SCTRL_SCINT_MASK1_drx2_int_START (2)
#define SOC_SCTRL_SCINT_MASK1_drx2_int_END (2)
#define SOC_SCTRL_SCINT_MASK1_drx3_int_START (3)
#define SOC_SCTRL_SCINT_MASK1_drx3_int_END (3)
#define SOC_SCTRL_SCINT_MASK1_drx4_int_START (4)
#define SOC_SCTRL_SCINT_MASK1_drx4_int_END (4)
#define SOC_SCTRL_SCINT_MASK1_drx5_int_START (5)
#define SOC_SCTRL_SCINT_MASK1_drx5_int_END (5)
#define SOC_SCTRL_SCINT_MASK1_drx6_int_START (6)
#define SOC_SCTRL_SCINT_MASK1_drx6_int_END (6)
#define SOC_SCTRL_SCINT_MASK1_drx7_int_START (7)
#define SOC_SCTRL_SCINT_MASK1_drx7_int_END (7)
#define SOC_SCTRL_SCINT_MASK1_noc_asp_cfg_t_maintimeout_msksta_START (9)
#define SOC_SCTRL_SCINT_MASK1_noc_asp_cfg_t_maintimeout_msksta_END (9)
#define SOC_SCTRL_SCINT_MASK1_iomcu_ahb_slv_maintimeout_msksta_START (10)
#define SOC_SCTRL_SCINT_MASK1_iomcu_ahb_slv_maintimeout_msksta_END (10)
#define SOC_SCTRL_SCINT_MASK1_iomcu_apb_slv_maintimeout_msksta_START (11)
#define SOC_SCTRL_SCINT_MASK1_iomcu_apb_slv_maintimeout_msksta_END (11)
#define SOC_SCTRL_SCINT_MASK1_sc_noc_aon_apb_slv_t_maintimeout_msksta_START (13)
#define SOC_SCTRL_SCINT_MASK1_sc_noc_aon_apb_slv_t_maintimeout_msksta_END (13)
#define SOC_SCTRL_SCINT_MASK1_drx8_int_START (15)
#define SOC_SCTRL_SCINT_MASK1_drx8_int_END (15)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int16_START (16)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int16_END (16)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int17_START (17)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int17_END (17)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int18_START (18)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int18_END (18)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int19_START (19)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int19_END (19)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int0_START (21)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_int0_END (21)
#define SOC_SCTRL_SCINT_MASK1_gpio_18_int_START (26)
#define SOC_SCTRL_SCINT_MASK1_gpio_18_int_END (26)
#define SOC_SCTRL_SCINT_MASK1_gpio_19_int_START (27)
#define SOC_SCTRL_SCINT_MASK1_gpio_19_int_END (27)
#define SOC_SCTRL_SCINT_MASK1_gpio_28_int_START (28)
#define SOC_SCTRL_SCINT_MASK1_gpio_28_int_END (28)
#define SOC_SCTRL_SCINT_MASK1_se_gpio1_START (29)
#define SOC_SCTRL_SCINT_MASK1_se_gpio1_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int drx0_int : 1;
        unsigned int drx1_int : 1;
        unsigned int drx2_int : 1;
        unsigned int drx3_int : 1;
        unsigned int drx4_int : 1;
        unsigned int drx5_int : 1;
        unsigned int drx6_int : 1;
        unsigned int drx7_int : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_asp_cfg_t_maintimeout_msksta : 1;
        unsigned int iomcu_ahb_slv_maintimeout_msksta : 1;
        unsigned int iomcu_apb_slv_maintimeout_msksta : 1;
        unsigned int reserved_1 : 1;
        unsigned int sc_noc_aon_apb_slv_t_maintimeout_msksta : 1;
        unsigned int reserved_2 : 1;
        unsigned int drx8_int : 1;
        unsigned int ao_ipc_int16 : 1;
        unsigned int ao_ipc_int17 : 1;
        unsigned int ao_ipc_int18 : 1;
        unsigned int ao_ipc_int19 : 1;
        unsigned int reserved_3 : 1;
        unsigned int ao_ipc_int21 : 1;
        unsigned int reserved_4 : 4;
        unsigned int gpio_18_int : 1;
        unsigned int gpio_19_int : 1;
        unsigned int gpio_28_int : 1;
        unsigned int se_gpio1 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
    } reg;
} SOC_SCTRL_SCINT_STAT1_UNION;
#endif
#define SOC_SCTRL_SCINT_STAT1_drx0_int_START (0)
#define SOC_SCTRL_SCINT_STAT1_drx0_int_END (0)
#define SOC_SCTRL_SCINT_STAT1_drx1_int_START (1)
#define SOC_SCTRL_SCINT_STAT1_drx1_int_END (1)
#define SOC_SCTRL_SCINT_STAT1_drx2_int_START (2)
#define SOC_SCTRL_SCINT_STAT1_drx2_int_END (2)
#define SOC_SCTRL_SCINT_STAT1_drx3_int_START (3)
#define SOC_SCTRL_SCINT_STAT1_drx3_int_END (3)
#define SOC_SCTRL_SCINT_STAT1_drx4_int_START (4)
#define SOC_SCTRL_SCINT_STAT1_drx4_int_END (4)
#define SOC_SCTRL_SCINT_STAT1_drx5_int_START (5)
#define SOC_SCTRL_SCINT_STAT1_drx5_int_END (5)
#define SOC_SCTRL_SCINT_STAT1_drx6_int_START (6)
#define SOC_SCTRL_SCINT_STAT1_drx6_int_END (6)
#define SOC_SCTRL_SCINT_STAT1_drx7_int_START (7)
#define SOC_SCTRL_SCINT_STAT1_drx7_int_END (7)
#define SOC_SCTRL_SCINT_STAT1_noc_asp_cfg_t_maintimeout_msksta_START (9)
#define SOC_SCTRL_SCINT_STAT1_noc_asp_cfg_t_maintimeout_msksta_END (9)
#define SOC_SCTRL_SCINT_STAT1_iomcu_ahb_slv_maintimeout_msksta_START (10)
#define SOC_SCTRL_SCINT_STAT1_iomcu_ahb_slv_maintimeout_msksta_END (10)
#define SOC_SCTRL_SCINT_STAT1_iomcu_apb_slv_maintimeout_msksta_START (11)
#define SOC_SCTRL_SCINT_STAT1_iomcu_apb_slv_maintimeout_msksta_END (11)
#define SOC_SCTRL_SCINT_STAT1_sc_noc_aon_apb_slv_t_maintimeout_msksta_START (13)
#define SOC_SCTRL_SCINT_STAT1_sc_noc_aon_apb_slv_t_maintimeout_msksta_END (13)
#define SOC_SCTRL_SCINT_STAT1_drx8_int_START (15)
#define SOC_SCTRL_SCINT_STAT1_drx8_int_END (15)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int16_START (16)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int16_END (16)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int17_START (17)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int17_END (17)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int18_START (18)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int18_END (18)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int19_START (19)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int19_END (19)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int21_START (21)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_int21_END (21)
#define SOC_SCTRL_SCINT_STAT1_gpio_18_int_START (26)
#define SOC_SCTRL_SCINT_STAT1_gpio_18_int_END (26)
#define SOC_SCTRL_SCINT_STAT1_gpio_19_int_START (27)
#define SOC_SCTRL_SCINT_STAT1_gpio_19_int_END (27)
#define SOC_SCTRL_SCINT_STAT1_gpio_28_int_START (28)
#define SOC_SCTRL_SCINT_STAT1_gpio_28_int_END (28)
#define SOC_SCTRL_SCINT_STAT1_se_gpio1_START (29)
#define SOC_SCTRL_SCINT_STAT1_se_gpio1_END (29)
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
        unsigned int lbintpll_en : 1;
        unsigned int lbintpll_bypass : 1;
        unsigned int lbintpll_irefen : 1;
        unsigned int lbintpll_iref : 1;
        unsigned int lbintpll_vcodivsel : 1;
        unsigned int lbintpll_postdiv : 4;
        unsigned int lbintpll_lockcount : 2;
        unsigned int lbintpll_foutpostdiven : 1;
        unsigned int lbintpll_foutvcoven : 1;
        unsigned int lbintpll_cfgvld : 1;
        unsigned int reserved_0 : 1;
        unsigned int lbintpll_fbdiv : 14;
        unsigned int reserved_1 : 3;
    } reg;
} SOC_SCTRL_SCLBINTPLLCTRL0_UNION;
#endif
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_en_START (0)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_en_END (0)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_bypass_START (1)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_bypass_END (1)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_irefen_START (2)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_irefen_END (2)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_iref_START (3)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_iref_END (3)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_vcodivsel_START (4)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_vcodivsel_END (4)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_postdiv_START (5)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_postdiv_END (8)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_lockcount_START (9)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_lockcount_END (10)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_foutpostdiven_START (11)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_foutpostdiven_END (11)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_foutvcoven_START (12)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_foutvcoven_END (12)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_cfgvld_START (13)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_cfgvld_END (13)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_fbdiv_START (15)
#define SOC_SCTRL_SCLBINTPLLCTRL0_lbintpll_fbdiv_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbintpll_faclin : 8;
        unsigned int lbintpll_faclbypass : 1;
        unsigned int lbintpll_facltest : 1;
        unsigned int fpll_clk_src_sel : 2;
        unsigned int gt_lbintpll_test_vco : 1;
        unsigned int gt_lbintpll_test_postdiv : 1;
        unsigned int reserved : 18;
    } reg;
} SOC_SCTRL_SCLBINTPLLCTRL1_UNION;
#endif
#define SOC_SCTRL_SCLBINTPLLCTRL1_lbintpll_faclin_START (0)
#define SOC_SCTRL_SCLBINTPLLCTRL1_lbintpll_faclin_END (7)
#define SOC_SCTRL_SCLBINTPLLCTRL1_lbintpll_faclbypass_START (8)
#define SOC_SCTRL_SCLBINTPLLCTRL1_lbintpll_faclbypass_END (8)
#define SOC_SCTRL_SCLBINTPLLCTRL1_lbintpll_facltest_START (9)
#define SOC_SCTRL_SCLBINTPLLCTRL1_lbintpll_facltest_END (9)
#define SOC_SCTRL_SCLBINTPLLCTRL1_fpll_clk_src_sel_START (10)
#define SOC_SCTRL_SCLBINTPLLCTRL1_fpll_clk_src_sel_END (11)
#define SOC_SCTRL_SCLBINTPLLCTRL1_gt_lbintpll_test_vco_START (12)
#define SOC_SCTRL_SCLBINTPLLCTRL1_gt_lbintpll_test_vco_END (12)
#define SOC_SCTRL_SCLBINTPLLCTRL1_gt_lbintpll_test_postdiv_START (13)
#define SOC_SCTRL_SCLBINTPLLCTRL1_gt_lbintpll_test_postdiv_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbintpll_scanen : 1;
        unsigned int lbintpll_scanin : 1;
        unsigned int lbintpll_scanmode : 1;
        unsigned int lbintpll_scanrstb : 1;
        unsigned int reserved_0 : 6;
        unsigned int lbintpll_atesten : 1;
        unsigned int lbintpll_atestsel : 3;
        unsigned int reserved_1 : 18;
    } reg;
} SOC_SCTRL_SCLBINTPLLCTRL2_UNION;
#endif
#define SOC_SCTRL_SCLBINTPLLCTRL2_lbintpll_scanen_START (0)
#define SOC_SCTRL_SCLBINTPLLCTRL2_lbintpll_scanen_END (0)
#define SOC_SCTRL_SCLBINTPLLCTRL2_lbintpll_scanin_START (1)
#define SOC_SCTRL_SCLBINTPLLCTRL2_lbintpll_scanin_END (1)
#define SOC_SCTRL_SCLBINTPLLCTRL2_lbintpll_scanmode_START (2)
#define SOC_SCTRL_SCLBINTPLLCTRL2_lbintpll_scanmode_END (2)
#define SOC_SCTRL_SCLBINTPLLCTRL2_lbintpll_scanrstb_START (3)
#define SOC_SCTRL_SCLBINTPLLCTRL2_lbintpll_scanrstb_END (3)
#define SOC_SCTRL_SCLBINTPLLCTRL2_lbintpll_atesten_START (10)
#define SOC_SCTRL_SCLBINTPLLCTRL2_lbintpll_atesten_END (10)
#define SOC_SCTRL_SCLBINTPLLCTRL2_lbintpll_atestsel_START (11)
#define SOC_SCTRL_SCLBINTPLLCTRL2_lbintpll_atestsel_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbintpll_en : 1;
        unsigned int lbintpll_lock : 1;
        unsigned int clkref_lbintpll : 1;
        unsigned int clk_lbintpll_vco_test : 1;
        unsigned int clk_lbintpll_postdiv_test : 1;
        unsigned int lbintpll_atestout : 1;
        unsigned int lbintpll_scanout : 1;
        unsigned int libintpll_faclout : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 22;
    } reg;
} SOC_SCTRL_SCLBINTPLLSTAT_UNION;
#endif
#define SOC_SCTRL_SCLBINTPLLSTAT_lbintpll_en_START (0)
#define SOC_SCTRL_SCLBINTPLLSTAT_lbintpll_en_END (0)
#define SOC_SCTRL_SCLBINTPLLSTAT_lbintpll_lock_START (1)
#define SOC_SCTRL_SCLBINTPLLSTAT_lbintpll_lock_END (1)
#define SOC_SCTRL_SCLBINTPLLSTAT_clkref_lbintpll_START (2)
#define SOC_SCTRL_SCLBINTPLLSTAT_clkref_lbintpll_END (2)
#define SOC_SCTRL_SCLBINTPLLSTAT_clk_lbintpll_vco_test_START (3)
#define SOC_SCTRL_SCLBINTPLLSTAT_clk_lbintpll_vco_test_END (3)
#define SOC_SCTRL_SCLBINTPLLSTAT_clk_lbintpll_postdiv_test_START (4)
#define SOC_SCTRL_SCLBINTPLLSTAT_clk_lbintpll_postdiv_test_END (4)
#define SOC_SCTRL_SCLBINTPLLSTAT_lbintpll_atestout_START (5)
#define SOC_SCTRL_SCLBINTPLLSTAT_lbintpll_atestout_END (5)
#define SOC_SCTRL_SCLBINTPLLSTAT_lbintpll_scanout_START (6)
#define SOC_SCTRL_SCLBINTPLLSTAT_lbintpll_scanout_END (6)
#define SOC_SCTRL_SCLBINTPLLSTAT_libintpll_faclout_START (7)
#define SOC_SCTRL_SCLBINTPLLSTAT_libintpll_faclout_END (7)
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
        unsigned int reserved : 6;
    } reg;
} SOC_SCTRL_SCPPLLCTRL0_UNION;
#endif
#define SOC_SCTRL_SCPPLLCTRL0_ppll0_en_START (0)
#define SOC_SCTRL_SCPPLLCTRL0_ppll0_en_END (0)
#define SOC_SCTRL_SCPPLLCTRL0_ppll0_bp_START (1)
#define SOC_SCTRL_SCPPLLCTRL0_ppll0_bp_END (1)
#define SOC_SCTRL_SCPPLLCTRL0_ppll0_refdiv_START (2)
#define SOC_SCTRL_SCPPLLCTRL0_ppll0_refdiv_END (7)
#define SOC_SCTRL_SCPPLLCTRL0_ppll0_fbdiv_START (8)
#define SOC_SCTRL_SCPPLLCTRL0_ppll0_fbdiv_END (19)
#define SOC_SCTRL_SCPPLLCTRL0_ppll0_postdiv1_START (20)
#define SOC_SCTRL_SCPPLLCTRL0_ppll0_postdiv1_END (22)
#define SOC_SCTRL_SCPPLLCTRL0_ppll0_postdiv2_START (23)
#define SOC_SCTRL_SCPPLLCTRL0_ppll0_postdiv2_END (25)
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
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_SCTRL_SCPPLLCTRL1_UNION;
#endif
#define SOC_SCTRL_SCPPLLCTRL1_ppll0_fracdiv_START (0)
#define SOC_SCTRL_SCPPLLCTRL1_ppll0_fracdiv_END (23)
#define SOC_SCTRL_SCPPLLCTRL1_ppll0_int_mod_START (24)
#define SOC_SCTRL_SCPPLLCTRL1_ppll0_int_mod_END (24)
#define SOC_SCTRL_SCPPLLCTRL1_ppll0_cfg_vld_START (25)
#define SOC_SCTRL_SCPPLLCTRL1_ppll0_cfg_vld_END (25)
#define SOC_SCTRL_SCPPLLCTRL1_gt_clk_ppll0_START (26)
#define SOC_SCTRL_SCPPLLCTRL1_gt_clk_ppll0_END (26)
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
} SOC_SCTRL_SCPPLLSSCCTRL_UNION;
#endif
#define SOC_SCTRL_SCPPLLSSCCTRL_ppll0_ssc_reset_START (0)
#define SOC_SCTRL_SCPPLLSSCCTRL_ppll0_ssc_reset_END (0)
#define SOC_SCTRL_SCPPLLSSCCTRL_ppll0_ssc_disable_START (1)
#define SOC_SCTRL_SCPPLLSSCCTRL_ppll0_ssc_disable_END (1)
#define SOC_SCTRL_SCPPLLSSCCTRL_ppll0_ssc_downspread_START (2)
#define SOC_SCTRL_SCPPLLSSCCTRL_ppll0_ssc_downspread_END (2)
#define SOC_SCTRL_SCPPLLSSCCTRL_ppll0_ssc_spread_START (3)
#define SOC_SCTRL_SCPPLLSSCCTRL_ppll0_ssc_spread_END (5)
#define SOC_SCTRL_SCPPLLSSCCTRL_ppll0_ssc_divval_START (6)
#define SOC_SCTRL_SCPPLLSSCCTRL_ppll0_ssc_divval_END (9)
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
        unsigned int fpll0_en : 1;
        unsigned int fpll_bypass : 1;
        unsigned int scfpllctrl0 : 30;
    } reg;
} SOC_SCTRL_SCFPLLCTRL0_UNION;
#endif
#define SOC_SCTRL_SCFPLLCTRL0_fpll0_en_START (0)
#define SOC_SCTRL_SCFPLLCTRL0_fpll0_en_END (0)
#define SOC_SCTRL_SCFPLLCTRL0_fpll_bypass_START (1)
#define SOC_SCTRL_SCFPLLCTRL0_fpll_bypass_END (1)
#define SOC_SCTRL_SCFPLLCTRL0_scfpllctrl0_START (2)
#define SOC_SCTRL_SCFPLLCTRL0_scfpllctrl0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fpll_fcw : 13;
        unsigned int fpll_postdiv : 2;
        unsigned int scfpllctrl1 : 17;
    } reg;
} SOC_SCTRL_SCFPLLCTRL1_UNION;
#endif
#define SOC_SCTRL_SCFPLLCTRL1_fpll_fcw_START (0)
#define SOC_SCTRL_SCFPLLCTRL1_fpll_fcw_END (12)
#define SOC_SCTRL_SCFPLLCTRL1_fpll_postdiv_START (13)
#define SOC_SCTRL_SCFPLLCTRL1_fpll_postdiv_END (14)
#define SOC_SCTRL_SCFPLLCTRL1_scfpllctrl1_START (15)
#define SOC_SCTRL_SCFPLLCTRL1_scfpllctrl1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scfpllctrl2 : 32;
    } reg;
} SOC_SCTRL_SCFPLLCTRL2_UNION;
#endif
#define SOC_SCTRL_SCFPLLCTRL2_scfpllctrl2_START (0)
#define SOC_SCTRL_SCFPLLCTRL2_scfpllctrl2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scfpllctrl3 : 32;
    } reg;
} SOC_SCTRL_SCFPLLCTRL3_UNION;
#endif
#define SOC_SCTRL_SCFPLLCTRL3_scfpllctrl3_START (0)
#define SOC_SCTRL_SCFPLLCTRL3_scfpllctrl3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scfpllstat : 32;
    } reg;
} SOC_SCTRL_SCFPLLSTAT_UNION;
#endif
#define SOC_SCTRL_SCFPLLSTAT_scfpllstat_START (0)
#define SOC_SCTRL_SCFPLLSTAT_scfpllstat_END (31)
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
        unsigned int reserved : 1;
        unsigned int gt_clk_aonoc2ufsbus : 1;
        unsigned int gt_pclk_timer2 : 1;
        unsigned int gt_clk_timer2 : 1;
        unsigned int gt_pclk_timer3 : 1;
        unsigned int gt_clk_timer3 : 1;
        unsigned int gt_pclk_ao_gpio0 : 1;
        unsigned int gt_pclk_ao_gpio1 : 1;
        unsigned int gt_pclk_ao_gpio2 : 1;
        unsigned int gt_pclk_ao_gpio3 : 1;
        unsigned int gt_pclk_ao_ioc : 1;
        unsigned int gt_clk_out0 : 1;
        unsigned int gt_clk_out1 : 1;
        unsigned int gt_clk_jtag_auth : 1;
        unsigned int gt_pclk_syscnt : 1;
        unsigned int gt_clk_syscnt : 1;
        unsigned int gt_pclk_ao_gpio4 : 1;
        unsigned int gt_pclk_ao_gpio5 : 1;
        unsigned int gt_clk_sci0 : 1;
        unsigned int gt_clk_sci1 : 1;
        unsigned int gt_pclk_ao_gpio6 : 1;
        unsigned int gt_clk_asp_subsys_lpmcu : 1;
        unsigned int gt_clk_asp_tcxo : 1;
        unsigned int gt_pclk_bbpdrx : 1;
        unsigned int gt_clk_aobus : 1;
        unsigned int gt_clk_asp_codec : 1;
        unsigned int gt_clk_ppll0_sscg : 1;
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
#define SOC_SCTRL_SCPEREN0_gt_clk_aonoc2ufsbus_START (6)
#define SOC_SCTRL_SCPEREN0_gt_clk_aonoc2ufsbus_END (6)
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
#define SOC_SCTRL_SCPEREN0_gt_clk_out0_START (16)
#define SOC_SCTRL_SCPEREN0_gt_clk_out0_END (16)
#define SOC_SCTRL_SCPEREN0_gt_clk_out1_START (17)
#define SOC_SCTRL_SCPEREN0_gt_clk_out1_END (17)
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
#define SOC_SCTRL_SCPEREN0_gt_clk_asp_subsys_lpmcu_START (26)
#define SOC_SCTRL_SCPEREN0_gt_clk_asp_subsys_lpmcu_END (26)
#define SOC_SCTRL_SCPEREN0_gt_clk_asp_tcxo_START (27)
#define SOC_SCTRL_SCPEREN0_gt_clk_asp_tcxo_END (27)
#define SOC_SCTRL_SCPEREN0_gt_pclk_bbpdrx_START (28)
#define SOC_SCTRL_SCPEREN0_gt_pclk_bbpdrx_END (28)
#define SOC_SCTRL_SCPEREN0_gt_clk_aobus_START (29)
#define SOC_SCTRL_SCPEREN0_gt_clk_aobus_END (29)
#define SOC_SCTRL_SCPEREN0_gt_clk_asp_codec_START (30)
#define SOC_SCTRL_SCPEREN0_gt_clk_asp_codec_END (30)
#define SOC_SCTRL_SCPEREN0_gt_clk_ppll0_sscg_START (31)
#define SOC_SCTRL_SCPEREN0_gt_clk_ppll0_sscg_END (31)
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
        unsigned int reserved : 1;
        unsigned int gt_clk_aonoc2ufsbus : 1;
        unsigned int gt_pclk_timer2 : 1;
        unsigned int gt_clk_timer2 : 1;
        unsigned int gt_pclk_timer3 : 1;
        unsigned int gt_clk_timer3 : 1;
        unsigned int gt_pclk_ao_gpio0 : 1;
        unsigned int gt_pclk_ao_gpio1 : 1;
        unsigned int gt_pclk_ao_gpio2 : 1;
        unsigned int gt_pclk_ao_gpio3 : 1;
        unsigned int gt_pclk_ao_ioc : 1;
        unsigned int gt_clk_out0 : 1;
        unsigned int gt_clk_out1 : 1;
        unsigned int gt_clk_jtag_auth : 1;
        unsigned int gt_pclk_syscnt : 1;
        unsigned int gt_clk_syscnt : 1;
        unsigned int gt_pclk_ao_gpio4 : 1;
        unsigned int gt_pclk_ao_gpio5 : 1;
        unsigned int gt_clk_sci0 : 1;
        unsigned int gt_clk_sci1 : 1;
        unsigned int gt_pclk_ao_gpio6 : 1;
        unsigned int gt_clk_asp_subsys_lpmcu : 1;
        unsigned int gt_clk_asp_tcxo : 1;
        unsigned int gt_pclk_bbpdrx : 1;
        unsigned int gt_clk_aobus : 1;
        unsigned int gt_clk_asp_codec : 1;
        unsigned int gt_clk_ppll0_sscg : 1;
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
#define SOC_SCTRL_SCPERDIS0_gt_clk_aonoc2ufsbus_START (6)
#define SOC_SCTRL_SCPERDIS0_gt_clk_aonoc2ufsbus_END (6)
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
#define SOC_SCTRL_SCPERDIS0_gt_clk_out0_START (16)
#define SOC_SCTRL_SCPERDIS0_gt_clk_out0_END (16)
#define SOC_SCTRL_SCPERDIS0_gt_clk_out1_START (17)
#define SOC_SCTRL_SCPERDIS0_gt_clk_out1_END (17)
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
#define SOC_SCTRL_SCPERDIS0_gt_clk_asp_subsys_lpmcu_START (26)
#define SOC_SCTRL_SCPERDIS0_gt_clk_asp_subsys_lpmcu_END (26)
#define SOC_SCTRL_SCPERDIS0_gt_clk_asp_tcxo_START (27)
#define SOC_SCTRL_SCPERDIS0_gt_clk_asp_tcxo_END (27)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_bbpdrx_START (28)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_bbpdrx_END (28)
#define SOC_SCTRL_SCPERDIS0_gt_clk_aobus_START (29)
#define SOC_SCTRL_SCPERDIS0_gt_clk_aobus_END (29)
#define SOC_SCTRL_SCPERDIS0_gt_clk_asp_codec_START (30)
#define SOC_SCTRL_SCPERDIS0_gt_clk_asp_codec_END (30)
#define SOC_SCTRL_SCPERDIS0_gt_clk_ppll0_sscg_START (31)
#define SOC_SCTRL_SCPERDIS0_gt_clk_ppll0_sscg_END (31)
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
        unsigned int reserved : 1;
        unsigned int gt_clk_aonoc2ufsbus : 1;
        unsigned int gt_pclk_timer2 : 1;
        unsigned int gt_clk_timer2 : 1;
        unsigned int gt_pclk_timer3 : 1;
        unsigned int gt_clk_timer3 : 1;
        unsigned int gt_pclk_ao_gpio0 : 1;
        unsigned int gt_pclk_ao_gpio1 : 1;
        unsigned int gt_pclk_ao_gpio2 : 1;
        unsigned int gt_pclk_ao_gpio3 : 1;
        unsigned int gt_pclk_ao_ioc : 1;
        unsigned int gt_clk_out0 : 1;
        unsigned int gt_clk_out1 : 1;
        unsigned int gt_clk_jtag_auth : 1;
        unsigned int gt_pclk_syscnt : 1;
        unsigned int gt_clk_syscnt : 1;
        unsigned int gt_pclk_ao_gpio4 : 1;
        unsigned int gt_pclk_ao_gpio5 : 1;
        unsigned int gt_clk_sci0 : 1;
        unsigned int gt_clk_sci1 : 1;
        unsigned int gt_pclk_ao_gpio6 : 1;
        unsigned int gt_clk_asp_subsys_lpmcu : 1;
        unsigned int gt_clk_asp_tcxo : 1;
        unsigned int gt_pclk_bbpdrx : 1;
        unsigned int gt_clk_aobus : 1;
        unsigned int gt_clk_asp_codec : 1;
        unsigned int gt_clk_ppll0_sscg : 1;
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
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_aonoc2ufsbus_START (6)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_aonoc2ufsbus_END (6)
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
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_out0_START (16)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_out0_END (16)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_out1_START (17)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_out1_END (17)
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
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_asp_subsys_lpmcu_START (26)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_asp_subsys_lpmcu_END (26)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_asp_tcxo_START (27)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_asp_tcxo_END (27)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_bbpdrx_START (28)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_bbpdrx_END (28)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_aobus_START (29)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_aobus_END (29)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_asp_codec_START (30)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_asp_codec_END (30)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_ppll0_sscg_START (31)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_ppll0_sscg_END (31)
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
        unsigned int st_clk_out0 : 1;
        unsigned int st_clk_out1 : 1;
        unsigned int st_clk_jtag_auth : 1;
        unsigned int st_pclk_syscnt : 1;
        unsigned int st_clk_syscnt : 1;
        unsigned int st_pclk_ao_gpio4 : 1;
        unsigned int st_pclk_ao_gpio5 : 1;
        unsigned int st_clk_sci0 : 1;
        unsigned int st_clk_sci1 : 1;
        unsigned int reserved : 1;
        unsigned int st_clk_asp_subsys : 1;
        unsigned int st_clk_asp_tcxo : 1;
        unsigned int st_pclk_bbpdrx : 1;
        unsigned int st_clk_aobus : 1;
        unsigned int st_pclk_efusec : 1;
        unsigned int st_clk_ppll0_sscg : 1;
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
#define SOC_SCTRL_SCPERSTAT0_st_clk_out0_START (16)
#define SOC_SCTRL_SCPERSTAT0_st_clk_out0_END (16)
#define SOC_SCTRL_SCPERSTAT0_st_clk_out1_START (17)
#define SOC_SCTRL_SCPERSTAT0_st_clk_out1_END (17)
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
#define SOC_SCTRL_SCPERSTAT0_st_clk_ppll0_sscg_START (31)
#define SOC_SCTRL_SCPERSTAT0_st_clk_ppll0_sscg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_asp_dw_axi : 1;
        unsigned int gt_clk_asp_x2h : 1;
        unsigned int gt_clk_asp_h2p : 1;
        unsigned int gt_clk_asp_cfg : 1;
        unsigned int gt_clk_asp_subsys_acpu : 1;
        unsigned int gt_clk_noc_aobus2sysbusddrc : 1;
        unsigned int gt_clk_asp_subsys_peri_acpu : 1;
        unsigned int ddr_loopback_en : 1;
        unsigned int gt_clk_asp_subsys_peri_lpmcu : 1;
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
        unsigned int gt_clk_emmc : 1;
        unsigned int gt_clk_emmc_base : 1;
        unsigned int gt_clk_noc_aobus2asp : 1;
        unsigned int gt_clk_noc_aobus2iomcu : 1;
        unsigned int gt_clk_ppll0_peri : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_clk_autodiv_aonoc_pll : 1;
        unsigned int gt_clk_autodiv_aonoc_fll : 1;
        unsigned int reserved_1 : 1;
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
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_cfg_START (3)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_cfg_END (3)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_acpu_START (4)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_acpu_END (4)
#define SOC_SCTRL_SCPEREN1_gt_clk_noc_aobus2sysbusddrc_START (5)
#define SOC_SCTRL_SCPEREN1_gt_clk_noc_aobus2sysbusddrc_END (5)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_peri_acpu_START (6)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_peri_acpu_END (6)
#define SOC_SCTRL_SCPEREN1_ddr_loopback_en_START (7)
#define SOC_SCTRL_SCPEREN1_ddr_loopback_en_END (7)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_peri_lpmcu_START (8)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_peri_lpmcu_END (8)
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
#define SOC_SCTRL_SCPEREN1_gt_clk_emmc_START (22)
#define SOC_SCTRL_SCPEREN1_gt_clk_emmc_END (22)
#define SOC_SCTRL_SCPEREN1_gt_clk_emmc_base_START (23)
#define SOC_SCTRL_SCPEREN1_gt_clk_emmc_base_END (23)
#define SOC_SCTRL_SCPEREN1_gt_clk_noc_aobus2asp_START (24)
#define SOC_SCTRL_SCPEREN1_gt_clk_noc_aobus2asp_END (24)
#define SOC_SCTRL_SCPEREN1_gt_clk_noc_aobus2iomcu_START (25)
#define SOC_SCTRL_SCPEREN1_gt_clk_noc_aobus2iomcu_END (25)
#define SOC_SCTRL_SCPEREN1_gt_clk_ppll0_peri_START (26)
#define SOC_SCTRL_SCPEREN1_gt_clk_ppll0_peri_END (26)
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
        unsigned int gt_clk_asp_cfg : 1;
        unsigned int gt_clk_asp_subsys_acpu : 1;
        unsigned int gt_clk_noc_aobus2sysbusddrc : 1;
        unsigned int gt_clk_asp_subsys_peri_acpu : 1;
        unsigned int ddr_loopback_en : 1;
        unsigned int gt_clk_asp_subsys_peri_lpmcu : 1;
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
        unsigned int gt_clk_emmc : 1;
        unsigned int gt_clk_emmc_base : 1;
        unsigned int gt_clk_noc_aobus2asp : 1;
        unsigned int gt_clk_noc_aobus2iomcu : 1;
        unsigned int gt_clk_ppll0_peri : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_clk_autodiv_aonoc_pll : 1;
        unsigned int gt_clk_autodiv_aonoc_fll : 1;
        unsigned int reserved_1 : 1;
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
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_cfg_START (3)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_cfg_END (3)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_subsys_acpu_START (4)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_subsys_acpu_END (4)
#define SOC_SCTRL_SCPERDIS1_gt_clk_noc_aobus2sysbusddrc_START (5)
#define SOC_SCTRL_SCPERDIS1_gt_clk_noc_aobus2sysbusddrc_END (5)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_subsys_peri_acpu_START (6)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_subsys_peri_acpu_END (6)
#define SOC_SCTRL_SCPERDIS1_ddr_loopback_en_START (7)
#define SOC_SCTRL_SCPERDIS1_ddr_loopback_en_END (7)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_subsys_peri_lpmcu_START (8)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_subsys_peri_lpmcu_END (8)
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
#define SOC_SCTRL_SCPERDIS1_gt_clk_emmc_START (22)
#define SOC_SCTRL_SCPERDIS1_gt_clk_emmc_END (22)
#define SOC_SCTRL_SCPERDIS1_gt_clk_emmc_base_START (23)
#define SOC_SCTRL_SCPERDIS1_gt_clk_emmc_base_END (23)
#define SOC_SCTRL_SCPERDIS1_gt_clk_noc_aobus2asp_START (24)
#define SOC_SCTRL_SCPERDIS1_gt_clk_noc_aobus2asp_END (24)
#define SOC_SCTRL_SCPERDIS1_gt_clk_noc_aobus2iomcu_START (25)
#define SOC_SCTRL_SCPERDIS1_gt_clk_noc_aobus2iomcu_END (25)
#define SOC_SCTRL_SCPERDIS1_gt_clk_ppll0_peri_START (26)
#define SOC_SCTRL_SCPERDIS1_gt_clk_ppll0_peri_END (26)
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
        unsigned int gt_clk_asp_cfg : 1;
        unsigned int gt_clk_asp_subsys_acpu : 1;
        unsigned int gt_clk_noc_aobus2sysbusddrc : 1;
        unsigned int gt_clk_asp_subsys_peri_acpu : 1;
        unsigned int ddr_loopback_en : 1;
        unsigned int gt_clk_asp_subsys_peri_lpmcu : 1;
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
        unsigned int gt_clk_emmc : 1;
        unsigned int gt_clk_emmc_base : 1;
        unsigned int gt_clk_noc_aobus2asp : 1;
        unsigned int gt_clk_noc_aobus2iomcu : 1;
        unsigned int gt_clk_ppll0_peri : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_clk_autodiv_aonoc_pll : 1;
        unsigned int gt_clk_autodiv_aonoc_fll : 1;
        unsigned int reserved_1 : 1;
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
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_cfg_START (3)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_cfg_END (3)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_subsys_acpu_START (4)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_subsys_acpu_END (4)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_noc_aobus2sysbusddrc_START (5)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_noc_aobus2sysbusddrc_END (5)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_subsys_peri_acpu_START (6)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_subsys_peri_acpu_END (6)
#define SOC_SCTRL_SCPERCLKEN1_ddr_loopback_en_START (7)
#define SOC_SCTRL_SCPERCLKEN1_ddr_loopback_en_END (7)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_subsys_peri_lpmcu_START (8)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_subsys_peri_lpmcu_END (8)
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
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_emmc_START (22)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_emmc_END (22)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_emmc_base_START (23)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_emmc_base_END (23)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_noc_aobus2asp_START (24)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_noc_aobus2asp_END (24)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_noc_aobus2iomcu_START (25)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_noc_aobus2iomcu_END (25)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_ppll0_peri_START (26)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_ppll0_peri_END (26)
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
        unsigned int reserved_0 : 1;
        unsigned int st_clk_noc_aobus2sysbusddrc : 1;
        unsigned int st_clk_asp_subsys_peri : 1;
        unsigned int st_pclk_ao_gpio6 : 1;
        unsigned int reserved_1 : 1;
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
        unsigned int st_clk_emmc : 1;
        unsigned int st_clk_emmc_base : 1;
        unsigned int st_clk_noc_aobus2ufsbus : 1;
        unsigned int st_clk_asp_codec : 1;
        unsigned int st_clk_ppll0_peri : 1;
        unsigned int swdone_clk_asp_subsys_div : 1;
        unsigned int swdone_clk_sci_div : 1;
        unsigned int swdone_clk_aobus_div : 1;
        unsigned int aobus_sw2sys : 1;
        unsigned int aobus_sw2pll : 1;
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
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2sysbusddrc_START (5)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2sysbusddrc_END (5)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_subsys_peri_START (6)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_subsys_peri_END (6)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_ao_gpio6_START (7)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_ao_gpio6_END (7)
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
#define SOC_SCTRL_SCPERSTAT1_st_clk_emmc_START (22)
#define SOC_SCTRL_SCPERSTAT1_st_clk_emmc_END (22)
#define SOC_SCTRL_SCPERSTAT1_st_clk_emmc_base_START (23)
#define SOC_SCTRL_SCPERSTAT1_st_clk_emmc_base_END (23)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2ufsbus_START (24)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2ufsbus_END (24)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_codec_START (25)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_codec_END (25)
#define SOC_SCTRL_SCPERSTAT1_st_clk_ppll0_peri_START (26)
#define SOC_SCTRL_SCPERSTAT1_st_clk_ppll0_peri_END (26)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_asp_subsys_div_START (27)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_asp_subsys_div_END (27)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_sci_div_START (28)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_sci_div_END (28)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_aobus_div_START (29)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_aobus_div_END (29)
#define SOC_SCTRL_SCPERSTAT1_aobus_sw2sys_START (30)
#define SOC_SCTRL_SCPERSTAT1_aobus_sw2sys_END (30)
#define SOC_SCTRL_SCPERSTAT1_aobus_sw2pll_START (31)
#define SOC_SCTRL_SCPERSTAT1_aobus_sw2pll_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 9;
        unsigned int ppll0_en_cbbe16 : 1;
        unsigned int ppll0_en_bbe16 : 1;
        unsigned int ppll0_gt_cbbe16 : 1;
        unsigned int ppll0_gt_bbe16 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ppll0_gt_cpu : 1;
        unsigned int ppll0_gt_modem : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int gt_pclkdbg_to_iomcu : 1;
        unsigned int reserved_10 : 1;
        unsigned int ppll0_en_cpu : 1;
        unsigned int ppll0_en_mdm : 1;
        unsigned int fll_en_cpu : 1;
        unsigned int fll_en_mdm : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
    } reg;
} SOC_SCTRL_SCPEREN2_UNION;
#endif
#define SOC_SCTRL_SCPEREN2_ppll0_en_cbbe16_START (10)
#define SOC_SCTRL_SCPEREN2_ppll0_en_cbbe16_END (10)
#define SOC_SCTRL_SCPEREN2_ppll0_en_bbe16_START (11)
#define SOC_SCTRL_SCPEREN2_ppll0_en_bbe16_END (11)
#define SOC_SCTRL_SCPEREN2_ppll0_gt_cbbe16_START (12)
#define SOC_SCTRL_SCPEREN2_ppll0_gt_cbbe16_END (12)
#define SOC_SCTRL_SCPEREN2_ppll0_gt_bbe16_START (13)
#define SOC_SCTRL_SCPEREN2_ppll0_gt_bbe16_END (13)
#define SOC_SCTRL_SCPEREN2_ppll0_gt_cpu_START (15)
#define SOC_SCTRL_SCPEREN2_ppll0_gt_cpu_END (15)
#define SOC_SCTRL_SCPEREN2_ppll0_gt_modem_START (16)
#define SOC_SCTRL_SCPEREN2_ppll0_gt_modem_END (16)
#define SOC_SCTRL_SCPEREN2_gt_pclkdbg_to_iomcu_START (24)
#define SOC_SCTRL_SCPEREN2_gt_pclkdbg_to_iomcu_END (24)
#define SOC_SCTRL_SCPEREN2_ppll0_en_cpu_START (26)
#define SOC_SCTRL_SCPEREN2_ppll0_en_cpu_END (26)
#define SOC_SCTRL_SCPEREN2_ppll0_en_mdm_START (27)
#define SOC_SCTRL_SCPEREN2_ppll0_en_mdm_END (27)
#define SOC_SCTRL_SCPEREN2_fll_en_cpu_START (28)
#define SOC_SCTRL_SCPEREN2_fll_en_cpu_END (28)
#define SOC_SCTRL_SCPEREN2_fll_en_mdm_START (29)
#define SOC_SCTRL_SCPEREN2_fll_en_mdm_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 9;
        unsigned int ppll0_en_cbbe16 : 1;
        unsigned int ppll0_en_bbe16 : 1;
        unsigned int ppll0_gt_cbbe16 : 1;
        unsigned int ppll0_gt_bbe16 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ppll0_gt_cpu : 1;
        unsigned int ppll0_gt_modem : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int gt_pclkdbg_to_iomcu : 1;
        unsigned int reserved_10 : 1;
        unsigned int ppll0_en_cpu : 1;
        unsigned int ppll0_en_mdm : 1;
        unsigned int fll_en_cpu : 1;
        unsigned int fll_en_mdm : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
    } reg;
} SOC_SCTRL_SCPERDIS2_UNION;
#endif
#define SOC_SCTRL_SCPERDIS2_ppll0_en_cbbe16_START (10)
#define SOC_SCTRL_SCPERDIS2_ppll0_en_cbbe16_END (10)
#define SOC_SCTRL_SCPERDIS2_ppll0_en_bbe16_START (11)
#define SOC_SCTRL_SCPERDIS2_ppll0_en_bbe16_END (11)
#define SOC_SCTRL_SCPERDIS2_ppll0_gt_cbbe16_START (12)
#define SOC_SCTRL_SCPERDIS2_ppll0_gt_cbbe16_END (12)
#define SOC_SCTRL_SCPERDIS2_ppll0_gt_bbe16_START (13)
#define SOC_SCTRL_SCPERDIS2_ppll0_gt_bbe16_END (13)
#define SOC_SCTRL_SCPERDIS2_ppll0_gt_cpu_START (15)
#define SOC_SCTRL_SCPERDIS2_ppll0_gt_cpu_END (15)
#define SOC_SCTRL_SCPERDIS2_ppll0_gt_modem_START (16)
#define SOC_SCTRL_SCPERDIS2_ppll0_gt_modem_END (16)
#define SOC_SCTRL_SCPERDIS2_gt_pclkdbg_to_iomcu_START (24)
#define SOC_SCTRL_SCPERDIS2_gt_pclkdbg_to_iomcu_END (24)
#define SOC_SCTRL_SCPERDIS2_ppll0_en_cpu_START (26)
#define SOC_SCTRL_SCPERDIS2_ppll0_en_cpu_END (26)
#define SOC_SCTRL_SCPERDIS2_ppll0_en_mdm_START (27)
#define SOC_SCTRL_SCPERDIS2_ppll0_en_mdm_END (27)
#define SOC_SCTRL_SCPERDIS2_fll_en_cpu_START (28)
#define SOC_SCTRL_SCPERDIS2_fll_en_cpu_END (28)
#define SOC_SCTRL_SCPERDIS2_fll_en_mdm_START (29)
#define SOC_SCTRL_SCPERDIS2_fll_en_mdm_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 9;
        unsigned int ppll0_en_cbbe16 : 1;
        unsigned int ppll0_en_bbe16 : 1;
        unsigned int ppll0_gt_cbbe16 : 1;
        unsigned int ppll0_gt_bbe16 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ppll0_gt_cpu : 1;
        unsigned int ppll0_gt_modem : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int gt_pclkdbg_to_iomcu : 1;
        unsigned int reserved_10 : 1;
        unsigned int ppll0_en_cpu : 1;
        unsigned int ppll0_en_mdm : 1;
        unsigned int fll_en_cpu : 1;
        unsigned int fll_en_mdm : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN2_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN2_ppll0_en_cbbe16_START (10)
#define SOC_SCTRL_SCPERCLKEN2_ppll0_en_cbbe16_END (10)
#define SOC_SCTRL_SCPERCLKEN2_ppll0_en_bbe16_START (11)
#define SOC_SCTRL_SCPERCLKEN2_ppll0_en_bbe16_END (11)
#define SOC_SCTRL_SCPERCLKEN2_ppll0_gt_cbbe16_START (12)
#define SOC_SCTRL_SCPERCLKEN2_ppll0_gt_cbbe16_END (12)
#define SOC_SCTRL_SCPERCLKEN2_ppll0_gt_bbe16_START (13)
#define SOC_SCTRL_SCPERCLKEN2_ppll0_gt_bbe16_END (13)
#define SOC_SCTRL_SCPERCLKEN2_ppll0_gt_cpu_START (15)
#define SOC_SCTRL_SCPERCLKEN2_ppll0_gt_cpu_END (15)
#define SOC_SCTRL_SCPERCLKEN2_ppll0_gt_modem_START (16)
#define SOC_SCTRL_SCPERCLKEN2_ppll0_gt_modem_END (16)
#define SOC_SCTRL_SCPERCLKEN2_gt_pclkdbg_to_iomcu_START (24)
#define SOC_SCTRL_SCPERCLKEN2_gt_pclkdbg_to_iomcu_END (24)
#define SOC_SCTRL_SCPERCLKEN2_ppll0_en_cpu_START (26)
#define SOC_SCTRL_SCPERCLKEN2_ppll0_en_cpu_END (26)
#define SOC_SCTRL_SCPERCLKEN2_ppll0_en_mdm_START (27)
#define SOC_SCTRL_SCPERCLKEN2_ppll0_en_mdm_END (27)
#define SOC_SCTRL_SCPERCLKEN2_fll_en_cpu_START (28)
#define SOC_SCTRL_SCPERCLKEN2_fll_en_cpu_END (28)
#define SOC_SCTRL_SCPERCLKEN2_fll_en_mdm_START (29)
#define SOC_SCTRL_SCPERCLKEN2_fll_en_mdm_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int swdone_clk_aonoc_pll_div : 1;
        unsigned int swdone_clk_iomcu_pll_div : 1;
        unsigned int aonoc_sw2sys : 1;
        unsigned int aonoc_sw2fll : 1;
        unsigned int aonoc_sw2ddr : 1;
        unsigned int aonoc_sw2pll : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int asp_subsys_sw2sys : 1;
        unsigned int asp_subsys_sw2pll0 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reset_dram_n : 1;
        unsigned int asp_subsys_sw2fll : 1;
        unsigned int asp_subsys_sw2peripll : 1;
        unsigned int reserved_6 : 1;
        unsigned int aobus_sw2sys : 1;
        unsigned int aobus_sw2pll : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int st_pclkdbg_to_iomcu : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int swdone_clk_aonoc_fll_div : 1;
        unsigned int reserved_16 : 1;
    } reg;
} SOC_SCTRL_SCPERSTAT2_UNION;
#endif
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_aonoc_pll_div_START (2)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_aonoc_pll_div_END (2)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_iomcu_pll_div_START (3)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_iomcu_pll_div_END (3)
#define SOC_SCTRL_SCPERSTAT2_aonoc_sw2sys_START (4)
#define SOC_SCTRL_SCPERSTAT2_aonoc_sw2sys_END (4)
#define SOC_SCTRL_SCPERSTAT2_aonoc_sw2fll_START (5)
#define SOC_SCTRL_SCPERSTAT2_aonoc_sw2fll_END (5)
#define SOC_SCTRL_SCPERSTAT2_aonoc_sw2ddr_START (6)
#define SOC_SCTRL_SCPERSTAT2_aonoc_sw2ddr_END (6)
#define SOC_SCTRL_SCPERSTAT2_aonoc_sw2pll_START (7)
#define SOC_SCTRL_SCPERSTAT2_aonoc_sw2pll_END (7)
#define SOC_SCTRL_SCPERSTAT2_asp_subsys_sw2sys_START (10)
#define SOC_SCTRL_SCPERSTAT2_asp_subsys_sw2sys_END (10)
#define SOC_SCTRL_SCPERSTAT2_asp_subsys_sw2pll0_START (11)
#define SOC_SCTRL_SCPERSTAT2_asp_subsys_sw2pll0_END (11)
#define SOC_SCTRL_SCPERSTAT2_reset_dram_n_START (14)
#define SOC_SCTRL_SCPERSTAT2_reset_dram_n_END (14)
#define SOC_SCTRL_SCPERSTAT2_asp_subsys_sw2fll_START (15)
#define SOC_SCTRL_SCPERSTAT2_asp_subsys_sw2fll_END (15)
#define SOC_SCTRL_SCPERSTAT2_asp_subsys_sw2peripll_START (16)
#define SOC_SCTRL_SCPERSTAT2_asp_subsys_sw2peripll_END (16)
#define SOC_SCTRL_SCPERSTAT2_aobus_sw2sys_START (18)
#define SOC_SCTRL_SCPERSTAT2_aobus_sw2sys_END (18)
#define SOC_SCTRL_SCPERSTAT2_aobus_sw2pll_START (19)
#define SOC_SCTRL_SCPERSTAT2_aobus_sw2pll_END (19)
#define SOC_SCTRL_SCPERSTAT2_st_pclkdbg_to_iomcu_START (26)
#define SOC_SCTRL_SCPERSTAT2_st_pclkdbg_to_iomcu_END (26)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_aonoc_fll_div_START (30)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_aonoc_fll_div_END (30)
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
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 4;
        unsigned int st_pclk_ao_ipc : 1;
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
#define SOC_SCTRL_SCPERSTAT3_st_pclk_ao_ipc_START (15)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_ao_ipc_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_aonoc_ufs_i_mainnopendingtrans_bp : 1;
        unsigned int sc_aonoc_asp_mst_i_mainnopendingtrans_bp : 1;
        unsigned int sc_aonoc_ufs_emmc_i_mainnopendingtrans_bp : 1;
        unsigned int sc_aonoc_ufshc_hibernate_ind_bp : 1;
        unsigned int reserved_0 : 5;
        unsigned int reserved_1 : 2;
        unsigned int reserved_2 : 8;
        unsigned int reserved_3 : 8;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN3_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN3_sc_aonoc_ufs_i_mainnopendingtrans_bp_START (0)
#define SOC_SCTRL_SCPERCLKEN3_sc_aonoc_ufs_i_mainnopendingtrans_bp_END (0)
#define SOC_SCTRL_SCPERCLKEN3_sc_aonoc_asp_mst_i_mainnopendingtrans_bp_START (1)
#define SOC_SCTRL_SCPERCLKEN3_sc_aonoc_asp_mst_i_mainnopendingtrans_bp_END (1)
#define SOC_SCTRL_SCPERCLKEN3_sc_aonoc_ufs_emmc_i_mainnopendingtrans_bp_START (2)
#define SOC_SCTRL_SCPERCLKEN3_sc_aonoc_ufs_emmc_i_mainnopendingtrans_bp_END (2)
#define SOC_SCTRL_SCPERCLKEN3_sc_aonoc_ufshc_hibernate_ind_bp_START (3)
#define SOC_SCTRL_SCPERCLKEN3_sc_aonoc_ufshc_hibernate_ind_bp_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_ao_tzpc : 1;
        unsigned int gt_pclk_ao_ipc_ns : 1;
        unsigned int gt_pclk_ao_loadmonitor : 1;
        unsigned int gt_clk_ao_loadmonitor : 1;
        unsigned int gt_clk_ppll0_media : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_pclk_gpio19 : 1;
        unsigned int gt_pclk_gpio18 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_ufsio_ref : 1;
        unsigned int gt_clk_autodiv_ufs_subsys : 1;
        unsigned int gt_clk_ufs_subsys : 1;
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
} SOC_SCTRL_SCPEREN4_UNION;
#endif
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_tzpc_START (2)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_tzpc_END (2)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_ipc_ns_START (3)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_ipc_ns_END (3)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_loadmonitor_START (4)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_loadmonitor_END (4)
#define SOC_SCTRL_SCPEREN4_gt_clk_ao_loadmonitor_START (5)
#define SOC_SCTRL_SCPEREN4_gt_clk_ao_loadmonitor_END (5)
#define SOC_SCTRL_SCPEREN4_gt_clk_ppll0_media_START (6)
#define SOC_SCTRL_SCPEREN4_gt_clk_ppll0_media_END (6)
#define SOC_SCTRL_SCPEREN4_gt_pclk_gpio19_START (8)
#define SOC_SCTRL_SCPEREN4_gt_pclk_gpio19_END (8)
#define SOC_SCTRL_SCPEREN4_gt_pclk_gpio18_START (9)
#define SOC_SCTRL_SCPEREN4_gt_pclk_gpio18_END (9)
#define SOC_SCTRL_SCPEREN4_gt_clk_ufsio_ref_START (12)
#define SOC_SCTRL_SCPEREN4_gt_clk_ufsio_ref_END (12)
#define SOC_SCTRL_SCPEREN4_gt_clk_autodiv_ufs_subsys_START (13)
#define SOC_SCTRL_SCPEREN4_gt_clk_autodiv_ufs_subsys_END (13)
#define SOC_SCTRL_SCPEREN4_gt_clk_ufs_subsys_START (14)
#define SOC_SCTRL_SCPEREN4_gt_clk_ufs_subsys_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_ao_tzpc : 1;
        unsigned int gt_pclk_ao_ipc_ns : 1;
        unsigned int gt_pclk_ao_loadmonitor : 1;
        unsigned int gt_clk_ao_loadmonitor : 1;
        unsigned int gt_clk_ppll0_media : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_pclk_gpio19 : 1;
        unsigned int gt_pclk_gpio18 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_ufsio_ref : 1;
        unsigned int gt_clk_autodiv_ufs_subsys : 1;
        unsigned int gt_clk_ufs_subsys : 1;
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
} SOC_SCTRL_SCPERDIS4_UNION;
#endif
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_tzpc_START (2)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_tzpc_END (2)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_ipc_ns_START (3)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_ipc_ns_END (3)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_loadmonitor_START (4)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_loadmonitor_END (4)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ao_loadmonitor_START (5)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ao_loadmonitor_END (5)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ppll0_media_START (6)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ppll0_media_END (6)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_gpio19_START (8)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_gpio19_END (8)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_gpio18_START (9)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_gpio18_END (9)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ufsio_ref_START (12)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ufsio_ref_END (12)
#define SOC_SCTRL_SCPERDIS4_gt_clk_autodiv_ufs_subsys_START (13)
#define SOC_SCTRL_SCPERDIS4_gt_clk_autodiv_ufs_subsys_END (13)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ufs_subsys_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_ao_tzpc : 1;
        unsigned int gt_pclk_ao_ipc_ns : 1;
        unsigned int gt_pclk_ao_loadmonitor : 1;
        unsigned int gt_clk_ao_loadmonitor : 1;
        unsigned int gt_clk_ppll0_media : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_pclk_gpio19 : 1;
        unsigned int gt_pclk_gpio18 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_ufsio_ref : 1;
        unsigned int gt_clk_autodiv_ufs_subsys : 1;
        unsigned int gt_clk_ufs_subsys : 1;
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
} SOC_SCTRL_SCPERCLKEN4_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_tzpc_START (2)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_tzpc_END (2)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_ipc_ns_START (3)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_ipc_ns_END (3)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_loadmonitor_START (4)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_loadmonitor_END (4)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ao_loadmonitor_START (5)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ao_loadmonitor_END (5)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ppll0_media_START (6)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ppll0_media_END (6)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_gpio19_START (8)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_gpio19_END (8)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_gpio18_START (9)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_gpio18_END (9)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ufsio_ref_START (12)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ufsio_ref_END (12)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_autodiv_ufs_subsys_START (13)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_autodiv_ufs_subsys_END (13)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ufs_subsys_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int st_clk_spmi_mst : 1;
        unsigned int st_pclk_ao_tzpc : 1;
        unsigned int st_pclk_ao_ipc_ns : 1;
        unsigned int st_pclk_ao_loadmonitor : 1;
        unsigned int st_clk_ao_loadmonitor : 1;
        unsigned int st_clk_ppll0_media : 1;
        unsigned int reserved_1 : 1;
        unsigned int st_pclk_gpio19 : 1;
        unsigned int st_pclk_gpio18 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int st_clk_ufsio_ref : 1;
        unsigned int st_clk_autodiv_ufs_subsys : 1;
        unsigned int st_clk_ufs_subsys : 1;
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
} SOC_SCTRL_SCPERSTAT4_UNION;
#endif
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
#define SOC_SCTRL_SCPERSTAT4_st_clk_ppll0_media_START (6)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ppll0_media_END (6)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_gpio19_START (8)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_gpio19_END (8)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_gpio18_START (9)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_gpio18_END (9)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ufsio_ref_START (12)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ufsio_ref_END (12)
#define SOC_SCTRL_SCPERSTAT4_st_clk_autodiv_ufs_subsys_START (13)
#define SOC_SCTRL_SCPERSTAT4_st_clk_autodiv_ufs_subsys_END (13)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ufs_subsys_END (14)
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
        unsigned int ip_rst_asp_cfg : 1;
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
        unsigned int ip_rst_emmc : 1;
        unsigned int ip_rst_emmc_base : 1;
        unsigned int ip_rst_sci0 : 1;
        unsigned int ip_rst_sci1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_asp_codec : 1;
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
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_asp_cfg_START (7)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_asp_cfg_END (7)
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
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_emmc_START (21)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_emmc_END (21)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_emmc_base_START (22)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_emmc_base_END (22)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_sci0_START (23)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_sci0_END (23)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_sci1_START (24)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_sci1_END (24)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_asp_codec_START (26)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_asp_codec_END (26)
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
        unsigned int ip_rst_asp_cfg : 1;
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
        unsigned int ip_rst_emmc : 1;
        unsigned int ip_rst_emmc_base : 1;
        unsigned int ip_rst_sci0 : 1;
        unsigned int ip_rst_sci1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_asp_codec : 1;
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
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_asp_cfg_START (7)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_asp_cfg_END (7)
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
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_emmc_START (21)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_emmc_END (21)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_emmc_base_START (22)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_emmc_base_END (22)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_sci0_START (23)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_sci0_END (23)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_sci1_START (24)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_sci1_END (24)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_asp_codec_START (26)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_asp_codec_END (26)
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
        unsigned int ip_rst_asp_cfg : 1;
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
        unsigned int ip_rst_emmc : 1;
        unsigned int ip_rst_emmc_base : 1;
        unsigned int ip_rst_sci0 : 1;
        unsigned int ip_rst_sci1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_asp_codec : 1;
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
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_asp_cfg_START (7)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_asp_cfg_END (7)
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
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_emmc_START (21)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_emmc_END (21)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_emmc_base_START (22)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_emmc_base_END (22)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_sci0_START (23)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_sci0_END (23)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_sci1_START (24)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_sci1_END (24)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_asp_codec_START (26)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_asp_codec_END (26)
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
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_prst_ao_loadmonitor : 1;
        unsigned int ip_rst_ufs_subsys : 1;
        unsigned int reserved_3 : 1;
        unsigned int ip_rst_dram_tmp_0 : 2;
        unsigned int reserved_4 : 14;
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
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_ao_loadmonitor_START (13)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_ao_loadmonitor_END (13)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_ufs_subsys_END (14)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_dram_tmp_0_START (16)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_dram_tmp_0_END (17)
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
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_prst_ao_loadmonitor : 1;
        unsigned int ip_rst_ufs_subsys : 1;
        unsigned int reserved_3 : 1;
        unsigned int ip_rst_dram_tmp_0 : 2;
        unsigned int reserved_4 : 14;
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
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_ao_loadmonitor_START (13)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_ao_loadmonitor_END (13)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_ufs_subsys_END (14)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_dram_tmp_0_START (16)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_dram_tmp_0_END (17)
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
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_prst_ao_loadmonitor : 1;
        unsigned int ip_rst_ufs_subsys : 1;
        unsigned int reserved_3 : 1;
        unsigned int ip_rst_dram_tmp_0 : 2;
        unsigned int reserved_4 : 14;
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
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_ao_loadmonitor_START (13)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_ao_loadmonitor_END (13)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_ufs_subsys_END (14)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_dram_tmp_0_START (16)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_dram_tmp_0_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 3;
        unsigned int softrst_dram_dis : 1;
        unsigned int ip_rst_dram_tmp_1 : 2;
        unsigned int ip_prst_gpio19 : 1;
        unsigned int ip_prst_gpio18 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 21;
    } reg;
} SOC_SCTRL_SCPERRSTEN2_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN2_softrst_dram_dis_START (4)
#define SOC_SCTRL_SCPERRSTEN2_softrst_dram_dis_END (4)
#define SOC_SCTRL_SCPERRSTEN2_ip_rst_dram_tmp_1_START (5)
#define SOC_SCTRL_SCPERRSTEN2_ip_rst_dram_tmp_1_END (6)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_gpio19_START (7)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_gpio19_END (7)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_gpio18_START (8)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_gpio18_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 3;
        unsigned int softrst_dram_dis : 1;
        unsigned int ip_rst_dram_tmp_1 : 2;
        unsigned int ip_prst_gpio19 : 1;
        unsigned int ip_prst_gpio18 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 21;
    } reg;
} SOC_SCTRL_SCPERRSTDIS2_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS2_softrst_dram_dis_START (4)
#define SOC_SCTRL_SCPERRSTDIS2_softrst_dram_dis_END (4)
#define SOC_SCTRL_SCPERRSTDIS2_ip_rst_dram_tmp_1_START (5)
#define SOC_SCTRL_SCPERRSTDIS2_ip_rst_dram_tmp_1_END (6)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_gpio19_START (7)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_gpio19_END (7)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_gpio18_START (8)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_gpio18_END (8)
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
        unsigned int ip_prst_gpio19 : 1;
        unsigned int ip_prst_gpio18 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 21;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT2_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT2_soft_rst_dram_dis_START (4)
#define SOC_SCTRL_SCPERRSTSTAT2_soft_rst_dram_dis_END (4)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_rst_dram_tmp_1_START (5)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_rst_dram_tmp_1_END (6)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_gpio19_START (7)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_gpio19_END (7)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_gpio18_START (8)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_gpio18_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rst_gt_rtc_bypass : 1;
        unsigned int rst_gt_rtc1_bypass : 1;
        unsigned int rst_gt_timer0_bypass : 1;
        unsigned int rst_gt_timer1_bypass : 1;
        unsigned int rst_gt_timer2_bypass : 1;
        unsigned int rst_gt_timer3_bypass : 1;
        unsigned int rst_gt_gpio0_bypass : 1;
        unsigned int rst_gt_gpio1_bypass : 1;
        unsigned int rst_gt_gpio2_bypass : 1;
        unsigned int rst_gt_gpio3_bypass : 1;
        unsigned int rst_gt_gpio4_bypass : 1;
        unsigned int rst_gt_gpio5_bypass : 1;
        unsigned int rst_gt_ioc_bypass : 1;
        unsigned int rst_gt_syscnt_bypass : 1;
        unsigned int rst_gt_sci0_bypass : 1;
        unsigned int rst_gt_sci1_bypass : 1;
        unsigned int rst_gt_bbpdrx_bypass : 1;
        unsigned int rst_gt_efusec_bypass : 1;
        unsigned int rst_gt_timer4_bypass : 1;
        unsigned int rst_gt_timer5_bypass : 1;
        unsigned int rst_gt_timer6_bypass : 1;
        unsigned int rst_gt_timer7_bypass : 1;
        unsigned int rst_gt_timer8_bypass : 1;
        unsigned int reserved_0 : 1;
        unsigned int rst_gt_asc_bypass : 1;
        unsigned int rst_gt_gpio6_bypass : 1;
        unsigned int rst_gt_gpio1_se_bypass : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int rst_gt_ao_loadmonitor_bypass : 1;
        unsigned int rst_gt_spmi_bypass : 1;
        unsigned int rts_gt_ao_tzpc_bypass : 1;
    } reg;
} SOC_SCTRL_SCIPCLKRSTBUS_UNION;
#endif
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_rtc_bypass_START (0)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_rtc_bypass_END (0)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_rtc1_bypass_START (1)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_rtc1_bypass_END (1)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer0_bypass_START (2)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer0_bypass_END (2)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer1_bypass_START (3)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer1_bypass_END (3)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer2_bypass_START (4)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer2_bypass_END (4)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer3_bypass_START (5)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer3_bypass_END (5)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_gpio0_bypass_START (6)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_gpio0_bypass_END (6)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_gpio1_bypass_START (7)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_gpio1_bypass_END (7)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_gpio2_bypass_START (8)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_gpio2_bypass_END (8)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_gpio3_bypass_START (9)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_gpio3_bypass_END (9)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_gpio4_bypass_START (10)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_gpio4_bypass_END (10)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_gpio5_bypass_START (11)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_gpio5_bypass_END (11)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_ioc_bypass_START (12)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_ioc_bypass_END (12)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_syscnt_bypass_START (13)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_syscnt_bypass_END (13)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_sci0_bypass_START (14)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_sci0_bypass_END (14)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_sci1_bypass_START (15)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_sci1_bypass_END (15)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_bbpdrx_bypass_START (16)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_bbpdrx_bypass_END (16)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_efusec_bypass_START (17)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_efusec_bypass_END (17)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer4_bypass_START (18)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer4_bypass_END (18)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer5_bypass_START (19)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer5_bypass_END (19)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer6_bypass_START (20)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer6_bypass_END (20)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer7_bypass_START (21)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer7_bypass_END (21)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer8_bypass_START (22)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_timer8_bypass_END (22)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_asc_bypass_START (24)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_asc_bypass_END (24)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_gpio6_bypass_START (25)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_gpio6_bypass_END (25)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_gpio1_se_bypass_START (26)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_gpio1_se_bypass_END (26)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_ao_loadmonitor_bypass_START (29)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_ao_loadmonitor_bypass_END (29)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_spmi_bypass_START (30)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_spmi_bypass_END (30)
#define SOC_SCTRL_SCIPCLKRSTBUS_rts_gt_ao_tzpc_bypass_START (31)
#define SOC_SCTRL_SCIPCLKRSTBUS_rts_gt_ao_tzpc_bypass_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rst_gt_ao_ipc_bypass : 1;
        unsigned int rst_gt_ao_ipc_ns_bypass : 1;
        unsigned int rst_gt_gpio19_bypass : 1;
        unsigned int rst_gt_gpio18_bypass : 1;
        unsigned int rst_gt_spi3_bypass : 1;
        unsigned int reserved_0 : 1;
        unsigned int rst_gt_ufs_sys_ctrl_bypass : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 24;
    } reg;
} SOC_SCTRL_SCIPCLKRSTBUS1_UNION;
#endif
#define SOC_SCTRL_SCIPCLKRSTBUS1_rst_gt_ao_ipc_bypass_START (0)
#define SOC_SCTRL_SCIPCLKRSTBUS1_rst_gt_ao_ipc_bypass_END (0)
#define SOC_SCTRL_SCIPCLKRSTBUS1_rst_gt_ao_ipc_ns_bypass_START (1)
#define SOC_SCTRL_SCIPCLKRSTBUS1_rst_gt_ao_ipc_ns_bypass_END (1)
#define SOC_SCTRL_SCIPCLKRSTBUS1_rst_gt_gpio19_bypass_START (2)
#define SOC_SCTRL_SCIPCLKRSTBUS1_rst_gt_gpio19_bypass_END (2)
#define SOC_SCTRL_SCIPCLKRSTBUS1_rst_gt_gpio18_bypass_START (3)
#define SOC_SCTRL_SCIPCLKRSTBUS1_rst_gt_gpio18_bypass_END (3)
#define SOC_SCTRL_SCIPCLKRSTBUS1_rst_gt_spi3_bypass_START (4)
#define SOC_SCTRL_SCIPCLKRSTBUS1_rst_gt_spi3_bypass_END (4)
#define SOC_SCTRL_SCIPCLKRSTBUS1_rst_gt_ufs_sys_ctrl_bypass_START (6)
#define SOC_SCTRL_SCIPCLKRSTBUS1_rst_gt_ufs_sys_ctrl_bypass_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_asp_subsys : 3;
        unsigned int sc_div_hifidsp : 6;
        unsigned int div_sci : 2;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int div_fll_src_tp : 2;
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
#define SOC_SCTRL_SCCLKDIV0_div_fll_src_tp_START (13)
#define SOC_SCTRL_SCCLKDIV0_div_fll_src_tp_END (14)
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
        unsigned int div_clkout0_tcxo : 3;
        unsigned int div_clkout1_tcxo : 3;
        unsigned int sel_clk_out0 : 2;
        unsigned int sel_clk_out1 : 2;
        unsigned int scclkdiv1_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV1_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV1_div_aobus_START (0)
#define SOC_SCTRL_SCCLKDIV1_div_aobus_END (5)
#define SOC_SCTRL_SCCLKDIV1_div_clkout0_tcxo_START (6)
#define SOC_SCTRL_SCCLKDIV1_div_clkout0_tcxo_END (8)
#define SOC_SCTRL_SCCLKDIV1_div_clkout1_tcxo_START (9)
#define SOC_SCTRL_SCCLKDIV1_div_clkout1_tcxo_END (11)
#define SOC_SCTRL_SCCLKDIV1_sel_clk_out0_START (12)
#define SOC_SCTRL_SCCLKDIV1_sel_clk_out0_END (13)
#define SOC_SCTRL_SCCLKDIV1_sel_clk_out1_START (14)
#define SOC_SCTRL_SCCLKDIV1_sel_clk_out1_END (15)
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
        unsigned int sc_gt_sclk_tp : 1;
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
#define SOC_SCTRL_SCCLKDIV2_sc_gt_sclk_tp_START (5)
#define SOC_SCTRL_SCCLKDIV2_sc_gt_sclk_tp_END (5)
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
        unsigned int sc_gt_clk_emmc_pll : 1;
        unsigned int div_emmc_pll : 4;
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
#define SOC_SCTRL_SCCLKDIV4_sc_gt_clk_emmc_pll_START (11)
#define SOC_SCTRL_SCCLKDIV4_sc_gt_clk_emmc_pll_END (11)
#define SOC_SCTRL_SCCLKDIV4_div_emmc_pll_START (12)
#define SOC_SCTRL_SCCLKDIV4_div_emmc_pll_END (15)
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
        unsigned int sel_clk_emmc : 2;
        unsigned int reserved : 1;
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
#define SOC_SCTRL_SCCLKDIV5_sel_clk_emmc_START (13)
#define SOC_SCTRL_SCCLKDIV5_sel_clk_emmc_END (14)
#define SOC_SCTRL_SCCLKDIV5_scclkdiv5_msk_START (16)
#define SOC_SCTRL_SCCLKDIV5_scclkdiv5_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_asp_subsys_peri : 3;
        unsigned int sc_gt_clk_asp_subsys_peri : 1;
        unsigned int div_syscnt : 4;
        unsigned int sel_asp_subsys_peri : 1;
        unsigned int sel_asp_subsys : 2;
        unsigned int div_aobus_fll : 5;
        unsigned int scclkdiv6_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV6_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV6_div_asp_subsys_peri_START (0)
#define SOC_SCTRL_SCCLKDIV6_div_asp_subsys_peri_END (2)
#define SOC_SCTRL_SCCLKDIV6_sc_gt_clk_asp_subsys_peri_START (3)
#define SOC_SCTRL_SCCLKDIV6_sc_gt_clk_asp_subsys_peri_END (3)
#define SOC_SCTRL_SCCLKDIV6_div_syscnt_START (4)
#define SOC_SCTRL_SCCLKDIV6_div_syscnt_END (7)
#define SOC_SCTRL_SCCLKDIV6_sel_asp_subsys_peri_START (8)
#define SOC_SCTRL_SCCLKDIV6_sel_asp_subsys_peri_END (8)
#define SOC_SCTRL_SCCLKDIV6_sel_asp_subsys_START (9)
#define SOC_SCTRL_SCCLKDIV6_sel_asp_subsys_END (10)
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
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int sel_ao_loadmonitor : 1;
        unsigned int sc_gt_clk_ao_loadmonitor : 1;
        unsigned int div_ao_loadmonitor : 4;
        unsigned int reserved_3 : 2;
        unsigned int scclkdiv7_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV7_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV7_sel_ao_loadmonitor_START (8)
#define SOC_SCTRL_SCCLKDIV7_sel_ao_loadmonitor_END (8)
#define SOC_SCTRL_SCCLKDIV7_sc_gt_clk_ao_loadmonitor_START (9)
#define SOC_SCTRL_SCCLKDIV7_sc_gt_clk_ao_loadmonitor_END (9)
#define SOC_SCTRL_SCCLKDIV7_div_ao_loadmonitor_START (10)
#define SOC_SCTRL_SCCLKDIV7_div_ao_loadmonitor_END (13)
#define SOC_SCTRL_SCCLKDIV7_scclkdiv7_msk_START (16)
#define SOC_SCTRL_SCCLKDIV7_scclkdiv7_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_asp_codec : 6;
        unsigned int sc_gt_clk_asp_codec_pll : 1;
        unsigned int reserved : 1;
        unsigned int sc_gt_clk_spmi_mst : 1;
        unsigned int div_spmi_mst : 6;
        unsigned int sel_spmi_mst : 1;
        unsigned int scclkdiv8_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV8_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV8_div_asp_codec_START (0)
#define SOC_SCTRL_SCCLKDIV8_div_asp_codec_END (5)
#define SOC_SCTRL_SCCLKDIV8_sc_gt_clk_asp_codec_pll_START (6)
#define SOC_SCTRL_SCCLKDIV8_sc_gt_clk_asp_codec_pll_END (6)
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
        unsigned int sel_ufs_subsys_pll : 1;
        unsigned int ufsbus_div_auto_reduce_bypass_acpu : 1;
        unsigned int ufsbus_div_auto_reduce_bypass_mcpu : 1;
        unsigned int sc_gt_clk_emmc_base_pll : 1;
        unsigned int div_emmc_base_pll : 4;
        unsigned int scclkdiv9_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV9_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV9_div_ufs_subsys_pll_START (0)
#define SOC_SCTRL_SCCLKDIV9_div_ufs_subsys_pll_END (5)
#define SOC_SCTRL_SCCLKDIV9_div_sys_ufs_subsys_START (6)
#define SOC_SCTRL_SCCLKDIV9_div_sys_ufs_subsys_END (7)
#define SOC_SCTRL_SCCLKDIV9_sel_ufs_subsys_pll_START (8)
#define SOC_SCTRL_SCCLKDIV9_sel_ufs_subsys_pll_END (8)
#define SOC_SCTRL_SCCLKDIV9_ufsbus_div_auto_reduce_bypass_acpu_START (9)
#define SOC_SCTRL_SCCLKDIV9_ufsbus_div_auto_reduce_bypass_acpu_END (9)
#define SOC_SCTRL_SCCLKDIV9_ufsbus_div_auto_reduce_bypass_mcpu_START (10)
#define SOC_SCTRL_SCCLKDIV9_ufsbus_div_auto_reduce_bypass_mcpu_END (10)
#define SOC_SCTRL_SCCLKDIV9_sc_gt_clk_emmc_base_pll_START (11)
#define SOC_SCTRL_SCCLKDIV9_sc_gt_clk_emmc_base_pll_END (11)
#define SOC_SCTRL_SCCLKDIV9_div_emmc_base_pll_START (12)
#define SOC_SCTRL_SCCLKDIV9_div_emmc_base_pll_END (15)
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
        unsigned int ufs_emmc_bypass : 1;
        unsigned int ufs_bus_bypass : 1;
        unsigned int ufs_hibernate_bypass : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
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
#define SOC_SCTRL_SCUFS_AUTODIV_ufs_emmc_bypass_START (27)
#define SOC_SCTRL_SCUFS_AUTODIV_ufs_emmc_bypass_END (27)
#define SOC_SCTRL_SCUFS_AUTODIV_ufs_bus_bypass_START (28)
#define SOC_SCTRL_SCUFS_AUTODIV_ufs_bus_bypass_END (28)
#define SOC_SCTRL_SCUFS_AUTODIV_ufs_hibernate_bypass_START (29)
#define SOC_SCTRL_SCUFS_AUTODIV_ufs_hibernate_bypass_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_efusec_mem_ctrl : 16;
        unsigned int efuse_remap : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_SCTRL_SCPERCTRL0_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL0_sc_efusec_mem_ctrl_START (0)
#define SOC_SCTRL_SCPERCTRL0_sc_efusec_mem_ctrl_END (15)
#define SOC_SCTRL_SCPERCTRL0_efuse_remap_START (16)
#define SOC_SCTRL_SCPERCTRL0_efuse_remap_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int repaie_disable_ao_subsys : 1;
        unsigned int repaie_disable_peri_subsys : 1;
        unsigned int repaie_disable_modem_subsys : 1;
        unsigned int repaie_disable_audio_subsys : 1;
        unsigned int repaie_disable_iomcu_subsys : 1;
        unsigned int repaie_disable_bbp_com : 1;
        unsigned int repaie_disable_cdsp : 1;
        unsigned int repaie_disable_bbp_lte_top : 1;
        unsigned int repaie_disable_bbp_3gp_top : 1;
        unsigned int repaie_disable_ccpu_wrap : 1;
        unsigned int repaie_disable_tldsp0 : 1;
        unsigned int repaie_disable_dss : 1;
        unsigned int repaie_disable_isp_noc_subsys_wrap : 1;
        unsigned int repaie_disable_isp_top_srtbpe_wrap : 1;
        unsigned int repaie_disable_ivp_subsys : 1;
        unsigned int repaie_disable_vdec_subsys : 1;
        unsigned int repaie_disable_venc_subsys : 1;
        unsigned int repaie_disable_cnn : 1;
        unsigned int repaie_disable_cci_subsys : 1;
        unsigned int repaie_disable_ufshc_psw : 1;
        unsigned int repaie_disable_a57_adb_cluster_wrap : 1;
        unsigned int repaie_disable_a53_adb_cluster_wrap : 1;
        unsigned int repaie_disable_hisee : 1;
        unsigned int repaie_disable_gpu_wrap : 1;
        unsigned int repaie_disable_mali_shader0to2 : 1;
        unsigned int repaie_disable_mali_shader3to5 : 1;
        unsigned int repaie_disable_mali_shader6to8 : 1;
        unsigned int repaie_disable_mali_shader9to11 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 3;
    } reg;
} SOC_SCTRL_SCPERCTRL1_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_ao_subsys_START (0)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_ao_subsys_END (0)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_peri_subsys_START (1)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_peri_subsys_END (1)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_modem_subsys_START (2)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_modem_subsys_END (2)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_audio_subsys_START (3)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_audio_subsys_END (3)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_iomcu_subsys_START (4)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_iomcu_subsys_END (4)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_bbp_com_START (5)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_bbp_com_END (5)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_cdsp_START (6)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_cdsp_END (6)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_bbp_lte_top_START (7)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_bbp_lte_top_END (7)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_bbp_3gp_top_START (8)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_bbp_3gp_top_END (8)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_ccpu_wrap_START (9)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_ccpu_wrap_END (9)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_tldsp0_START (10)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_tldsp0_END (10)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_dss_START (11)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_dss_END (11)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_isp_noc_subsys_wrap_START (12)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_isp_noc_subsys_wrap_END (12)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_isp_top_srtbpe_wrap_START (13)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_isp_top_srtbpe_wrap_END (13)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_ivp_subsys_START (14)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_ivp_subsys_END (14)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_vdec_subsys_START (15)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_vdec_subsys_END (15)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_venc_subsys_START (16)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_venc_subsys_END (16)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_cnn_START (17)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_cnn_END (17)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_cci_subsys_START (18)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_cci_subsys_END (18)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_ufshc_psw_START (19)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_ufshc_psw_END (19)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_a57_adb_cluster_wrap_START (20)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_a57_adb_cluster_wrap_END (20)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_a53_adb_cluster_wrap_START (21)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_a53_adb_cluster_wrap_END (21)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_hisee_START (22)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_hisee_END (22)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_gpu_wrap_START (23)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_gpu_wrap_END (23)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_mali_shader0to2_START (24)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_mali_shader0to2_END (24)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_mali_shader3to5_START (25)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_mali_shader3to5_END (25)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_mali_shader6to8_START (26)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_mali_shader6to8_END (26)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_mali_shader9to11_START (27)
#define SOC_SCTRL_SCPERCTRL1_repaie_disable_mali_shader9to11_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ao_spram_rtsel : 2;
        unsigned int ao_spram_wtsel : 2;
        unsigned int ao_spmbsram_rtsel : 2;
        unsigned int ao_spmbsram_wtsel : 2;
        unsigned int ao_rom_rtsel : 2;
        unsigned int ao_rom_ptsel : 2;
        unsigned int ao_rom_trb : 2;
        unsigned int ao_rom_tm : 1;
        unsigned int mmbuf_sram_sd : 1;
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 6;
    } reg;
} SOC_SCTRL_SCPERCTRL2_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL2_ao_spram_rtsel_START (0)
#define SOC_SCTRL_SCPERCTRL2_ao_spram_rtsel_END (1)
#define SOC_SCTRL_SCPERCTRL2_ao_spram_wtsel_START (2)
#define SOC_SCTRL_SCPERCTRL2_ao_spram_wtsel_END (3)
#define SOC_SCTRL_SCPERCTRL2_ao_spmbsram_rtsel_START (4)
#define SOC_SCTRL_SCPERCTRL2_ao_spmbsram_rtsel_END (5)
#define SOC_SCTRL_SCPERCTRL2_ao_spmbsram_wtsel_START (6)
#define SOC_SCTRL_SCPERCTRL2_ao_spmbsram_wtsel_END (7)
#define SOC_SCTRL_SCPERCTRL2_ao_rom_rtsel_START (8)
#define SOC_SCTRL_SCPERCTRL2_ao_rom_rtsel_END (9)
#define SOC_SCTRL_SCPERCTRL2_ao_rom_ptsel_START (10)
#define SOC_SCTRL_SCPERCTRL2_ao_rom_ptsel_END (11)
#define SOC_SCTRL_SCPERCTRL2_ao_rom_trb_START (12)
#define SOC_SCTRL_SCPERCTRL2_ao_rom_trb_END (13)
#define SOC_SCTRL_SCPERCTRL2_ao_rom_tm_START (14)
#define SOC_SCTRL_SCPERCTRL2_ao_rom_tm_END (14)
#define SOC_SCTRL_SCPERCTRL2_mmbuf_sram_sd_START (15)
#define SOC_SCTRL_SCPERCTRL2_mmbuf_sram_sd_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int repair_debug_sel : 1;
        unsigned int repair_hs_sw : 1;
        unsigned int repair_iddq : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 3;
        unsigned int efusec_timeout_bypass : 1;
        unsigned int bbdrx_timeout_bypass : 1;
        unsigned int sleep_mode_cfgbus_bypass : 1;
        unsigned int sleep_mode_lpmcu_bypass : 1;
        unsigned int first_power_on_flag : 1;
        unsigned int sleep_mode_aobus_bypass : 1;
        unsigned int bbp_clk_en : 1;
        unsigned int mrb_rst_msk : 1;
        unsigned int bisr_reserve_in : 4;
        unsigned int reserved_3 : 12;
    } reg;
} SOC_SCTRL_SCPERCTRL3_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL3_repair_debug_sel_START (0)
#define SOC_SCTRL_SCPERCTRL3_repair_debug_sel_END (0)
#define SOC_SCTRL_SCPERCTRL3_repair_hs_sw_START (1)
#define SOC_SCTRL_SCPERCTRL3_repair_hs_sw_END (1)
#define SOC_SCTRL_SCPERCTRL3_repair_iddq_START (2)
#define SOC_SCTRL_SCPERCTRL3_repair_iddq_END (2)
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
#define SOC_SCTRL_SCPERCTRL3_mrb_rst_msk_START (15)
#define SOC_SCTRL_SCPERCTRL3_mrb_rst_msk_END (15)
#define SOC_SCTRL_SCPERCTRL3_bisr_reserve_in_START (16)
#define SOC_SCTRL_SCPERCTRL3_bisr_reserve_in_END (19)
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
        unsigned int usim0_io_sel18 : 1;
        unsigned int usim1_io_sel18 : 1;
        unsigned int sdcard_io_sel18 : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 1;
        unsigned int usim_od_en : 1;
        unsigned int tp_sel : 4;
        unsigned int ao_hpm_clk_div : 6;
        unsigned int ao_hpm_en : 1;
        unsigned int ao_hpmx_en : 1;
        unsigned int sc_noc_timeout_en : 1;
        unsigned int peri_ctrl5_other : 11;
    } reg;
} SOC_SCTRL_SCPERCTRL5_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL5_usim0_io_sel18_START (0)
#define SOC_SCTRL_SCPERCTRL5_usim0_io_sel18_END (0)
#define SOC_SCTRL_SCPERCTRL5_usim1_io_sel18_START (1)
#define SOC_SCTRL_SCPERCTRL5_usim1_io_sel18_END (1)
#define SOC_SCTRL_SCPERCTRL5_sdcard_io_sel18_START (2)
#define SOC_SCTRL_SCPERCTRL5_sdcard_io_sel18_END (2)
#define SOC_SCTRL_SCPERCTRL5_usim_od_en_START (7)
#define SOC_SCTRL_SCPERCTRL5_usim_od_en_END (7)
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
#define SOC_SCTRL_SCPERCTRL5_peri_ctrl5_other_START (21)
#define SOC_SCTRL_SCPERCTRL5_peri_ctrl5_other_END (31)
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
        unsigned int peri_io_in_ao_ret : 1;
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 8;
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
#define SOC_SCTRL_SCPERCTRL6_peri_io_in_ao_ret_START (10)
#define SOC_SCTRL_SCPERCTRL6_peri_io_in_ao_ret_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int noc_asp_power_idlereq : 1;
        unsigned int iomcu_power_idlereq : 1;
        unsigned int ufs_emmc_power_idlereq : 1;
        unsigned int scperctrl7 : 11;
        unsigned int scperctrl7_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL7_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL7_noc_asp_power_idlereq_START (2)
#define SOC_SCTRL_SCPERCTRL7_noc_asp_power_idlereq_END (2)
#define SOC_SCTRL_SCPERCTRL7_iomcu_power_idlereq_START (3)
#define SOC_SCTRL_SCPERCTRL7_iomcu_power_idlereq_END (3)
#define SOC_SCTRL_SCPERCTRL7_ufs_emmc_power_idlereq_START (4)
#define SOC_SCTRL_SCPERCTRL7_ufs_emmc_power_idlereq_END (4)
#define SOC_SCTRL_SCPERCTRL7_scperctrl7_START (5)
#define SOC_SCTRL_SCPERCTRL7_scperctrl7_END (15)
#define SOC_SCTRL_SCPERCTRL7_scperctrl7_msk_START (16)
#define SOC_SCTRL_SCPERCTRL7_scperctrl7_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 29;
    } reg;
} SOC_SCTRL_SCPERCTRL8_UNION;
#endif
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
        unsigned int bbp_on_ctrl : 26;
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
#define SOC_SCTRL_SCPERCTRL9_bbp_on_ctrl_START (6)
#define SOC_SCTRL_SCPERCTRL9_bbp_on_ctrl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddra_acctl_ioctrl_henl_in : 2;
        unsigned int reserved_0 : 2;
        unsigned int ddra_dxctl_ioctrl_henl_in : 4;
        unsigned int ddrb_acctl_ioctrl_henl_in : 2;
        unsigned int reserved_1 : 2;
        unsigned int ddrb_dxctl_ioctrl_henl_in : 4;
        unsigned int ddrc_acctl_ioctrl_henl_in : 2;
        unsigned int reserved_2 : 2;
        unsigned int ddrc_dxctl_ioctrl_henl_in : 4;
        unsigned int ddrd_acctl_ioctrl_henl_in : 2;
        unsigned int reserved_3 : 2;
        unsigned int ddrd_dxctl_ioctrl_henl_in : 4;
    } reg;
} SOC_SCTRL_SCPERCTRL10_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL10_ddra_acctl_ioctrl_henl_in_START (0)
#define SOC_SCTRL_SCPERCTRL10_ddra_acctl_ioctrl_henl_in_END (1)
#define SOC_SCTRL_SCPERCTRL10_ddra_dxctl_ioctrl_henl_in_START (4)
#define SOC_SCTRL_SCPERCTRL10_ddra_dxctl_ioctrl_henl_in_END (7)
#define SOC_SCTRL_SCPERCTRL10_ddrb_acctl_ioctrl_henl_in_START (8)
#define SOC_SCTRL_SCPERCTRL10_ddrb_acctl_ioctrl_henl_in_END (9)
#define SOC_SCTRL_SCPERCTRL10_ddrb_dxctl_ioctrl_henl_in_START (12)
#define SOC_SCTRL_SCPERCTRL10_ddrb_dxctl_ioctrl_henl_in_END (15)
#define SOC_SCTRL_SCPERCTRL10_ddrc_acctl_ioctrl_henl_in_START (16)
#define SOC_SCTRL_SCPERCTRL10_ddrc_acctl_ioctrl_henl_in_END (17)
#define SOC_SCTRL_SCPERCTRL10_ddrc_dxctl_ioctrl_henl_in_START (20)
#define SOC_SCTRL_SCPERCTRL10_ddrc_dxctl_ioctrl_henl_in_END (23)
#define SOC_SCTRL_SCPERCTRL10_ddrd_acctl_ioctrl_henl_in_START (24)
#define SOC_SCTRL_SCPERCTRL10_ddrd_acctl_ioctrl_henl_in_END (25)
#define SOC_SCTRL_SCPERCTRL10_ddrd_dxctl_ioctrl_henl_in_START (28)
#define SOC_SCTRL_SCPERCTRL10_ddrd_dxctl_ioctrl_henl_in_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ap2mdm_ctrl : 32;
    } reg;
} SOC_SCTRL_SCPERCTRL11_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL11_ap2mdm_ctrl_START (0)
#define SOC_SCTRL_SCPERCTRL11_ap2mdm_ctrl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noc_ao_bus_idle_flag : 1;
        unsigned int autodiv_ufs_subsys_stat : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 2;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 3;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 2;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 15;
    } reg;
} SOC_SCTRL_SCPERSTATUS17_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS17_noc_ao_bus_idle_flag_START (0)
#define SOC_SCTRL_SCPERSTATUS17_noc_ao_bus_idle_flag_END (0)
#define SOC_SCTRL_SCPERSTATUS17_autodiv_ufs_subsys_stat_START (1)
#define SOC_SCTRL_SCPERSTATUS17_autodiv_ufs_subsys_stat_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int st_sclk_tp : 1;
        unsigned int sw_ack_clk_aobus_ini : 2;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int swdone_clk_ufs_subsys_sys_div : 1;
        unsigned int swdone_clk_ufs_subsys_pll_div : 1;
        unsigned int sw_ack_clk_ufs_subsys_sw : 3;
        unsigned int swdone_clk_spmi_mst : 1;
        unsigned int reserved_4 : 2;
        unsigned int st_clk_noc_aobus2aspbus : 1;
        unsigned int st_clk_noc_aobus2iomcubus : 1;
        unsigned int st_pclk_spmi : 1;
        unsigned int swdone_clk_asp_subsys_peri_div : 1;
        unsigned int sw_ack_clk_asp_subsys_peri_ini : 2;
        unsigned int swdone_clk_emmc_pll_div : 1;
        unsigned int swdone_clk_emmc_base_pll_div : 1;
        unsigned int reserved_5 : 9;
    } reg;
} SOC_SCTRL_SCPERSTATUS16_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS16_st_sclk_tp_START (2)
#define SOC_SCTRL_SCPERSTATUS16_st_sclk_tp_END (2)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_aobus_ini_START (3)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_aobus_ini_END (4)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_ufs_subsys_sys_div_START (7)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_ufs_subsys_sys_div_END (7)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_ufs_subsys_pll_div_START (8)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_ufs_subsys_pll_div_END (8)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_ufs_subsys_sw_START (9)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_ufs_subsys_sw_END (11)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_spmi_mst_START (12)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_spmi_mst_END (12)
#define SOC_SCTRL_SCPERSTATUS16_st_clk_noc_aobus2aspbus_START (15)
#define SOC_SCTRL_SCPERSTATUS16_st_clk_noc_aobus2aspbus_END (15)
#define SOC_SCTRL_SCPERSTATUS16_st_clk_noc_aobus2iomcubus_START (16)
#define SOC_SCTRL_SCPERSTATUS16_st_clk_noc_aobus2iomcubus_END (16)
#define SOC_SCTRL_SCPERSTATUS16_st_pclk_spmi_START (17)
#define SOC_SCTRL_SCPERSTATUS16_st_pclk_spmi_END (17)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_asp_subsys_peri_div_START (18)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_asp_subsys_peri_div_END (18)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_asp_subsys_peri_ini_START (19)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_asp_subsys_peri_ini_END (20)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_emmc_pll_div_START (21)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_emmc_pll_div_END (21)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_emmc_base_pll_div_START (22)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_emmc_base_pll_div_END (22)
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
        unsigned int tp_a0_grp : 10;
        unsigned int tp_a1_grp : 10;
        unsigned int bbp_apb_bus_slv_stat : 1;
        unsigned int reserved : 1;
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
#define SOC_SCTRL_SCPERSTATUS0_tp_a0_grp_START (10)
#define SOC_SCTRL_SCPERSTATUS0_tp_a0_grp_END (19)
#define SOC_SCTRL_SCPERSTATUS0_tp_a1_grp_START (20)
#define SOC_SCTRL_SCPERSTATUS0_tp_a1_grp_END (29)
#define SOC_SCTRL_SCPERSTATUS0_bbp_apb_bus_slv_stat_START (30)
#define SOC_SCTRL_SCPERSTATUS0_bbp_apb_bus_slv_stat_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bisr_reserve_out : 4;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_SCTRL_SCPERSTATUS1_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS1_bisr_reserve_out_START (0)
#define SOC_SCTRL_SCPERSTATUS1_bisr_reserve_out_END (3)
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
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS3_UNION;
#endif
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
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
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
        unsigned int noc_ufs_emmc_power_idle : 1;
        unsigned int noc_ufs_emmc_power_idleack : 1;
        unsigned int sc_noc_asp_cfg_t_maintimeout : 1;
        unsigned int reserved : 1;
        unsigned int sc_noc_aon_apb_slv_t_maintimeout : 1;
        unsigned int noc_asp_power_idle : 1;
        unsigned int intr_asp_transaction_probe_mainstatalarm : 1;
        unsigned int intr_aobus_error_probe_observer_mainfault : 1;
        unsigned int noc_asp_power_idleack : 1;
        unsigned int iomcu_ahb_slv_maintimeout : 1;
        unsigned int iomcu_apb_slv_maintimeout : 1;
        unsigned int iomcu_power_idle : 1;
        unsigned int iomcu_power_idleack : 1;
        unsigned int sc_aspbus_service_target_mainnopendingtrans : 1;
        unsigned int sc_aonoc_asp_mst_i_mainnopendingtrans : 1;
        unsigned int sc_aobus_service_target_mainnopendingtrans : 1;
        unsigned int sc_cfg_arm_dbgen_dx : 4;
        unsigned int sc_cfg_mcu_dbgen_dx : 4;
        unsigned int audio_mmbuf_ctrl_to_sctrl : 8;
    } reg;
} SOC_SCTRL_SCPERSTATUS6_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS6_noc_ufs_emmc_power_idle_START (0)
#define SOC_SCTRL_SCPERSTATUS6_noc_ufs_emmc_power_idle_END (0)
#define SOC_SCTRL_SCPERSTATUS6_noc_ufs_emmc_power_idleack_START (1)
#define SOC_SCTRL_SCPERSTATUS6_noc_ufs_emmc_power_idleack_END (1)
#define SOC_SCTRL_SCPERSTATUS6_sc_noc_asp_cfg_t_maintimeout_START (2)
#define SOC_SCTRL_SCPERSTATUS6_sc_noc_asp_cfg_t_maintimeout_END (2)
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
#define SOC_SCTRL_SCPERSTATUS6_sc_aspbus_service_target_mainnopendingtrans_START (13)
#define SOC_SCTRL_SCPERSTATUS6_sc_aspbus_service_target_mainnopendingtrans_END (13)
#define SOC_SCTRL_SCPERSTATUS6_sc_aonoc_asp_mst_i_mainnopendingtrans_START (14)
#define SOC_SCTRL_SCPERSTATUS6_sc_aonoc_asp_mst_i_mainnopendingtrans_END (14)
#define SOC_SCTRL_SCPERSTATUS6_sc_aobus_service_target_mainnopendingtrans_START (15)
#define SOC_SCTRL_SCPERSTATUS6_sc_aobus_service_target_mainnopendingtrans_END (15)
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
        unsigned int sc_aobus_service_target_mainnopendingtrans : 1;
        unsigned int sc_noc_asp_mst_i_mainnopendingtrans : 1;
        unsigned int sc_aspbus_service_target_mainnopendingtrans : 1;
        unsigned int iomcu_axi_mst_mainnopendingtrans : 1;
        unsigned int iomcu_service_mainnopendingtrans : 1;
        unsigned int iomcu_axi_ahb_mainnopendingtrans : 1;
        unsigned int iomcu_ahb_mst_tcp_mainnopendingtrans : 1;
        unsigned int ufs_axi_nopendingtrans : 1;
        unsigned int emmc_axi_nopendingtrans : 1;
        unsigned int ufs_emmc_service_nopendingtrans : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_SCTRL_SCPERSTATUS7_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS7_sc_aobus_service_target_mainnopendingtrans_START (0)
#define SOC_SCTRL_SCPERSTATUS7_sc_aobus_service_target_mainnopendingtrans_END (0)
#define SOC_SCTRL_SCPERSTATUS7_sc_noc_asp_mst_i_mainnopendingtrans_START (1)
#define SOC_SCTRL_SCPERSTATUS7_sc_noc_asp_mst_i_mainnopendingtrans_END (1)
#define SOC_SCTRL_SCPERSTATUS7_sc_aspbus_service_target_mainnopendingtrans_START (2)
#define SOC_SCTRL_SCPERSTATUS7_sc_aspbus_service_target_mainnopendingtrans_END (2)
#define SOC_SCTRL_SCPERSTATUS7_iomcu_axi_mst_mainnopendingtrans_START (3)
#define SOC_SCTRL_SCPERSTATUS7_iomcu_axi_mst_mainnopendingtrans_END (3)
#define SOC_SCTRL_SCPERSTATUS7_iomcu_service_mainnopendingtrans_START (4)
#define SOC_SCTRL_SCPERSTATUS7_iomcu_service_mainnopendingtrans_END (4)
#define SOC_SCTRL_SCPERSTATUS7_iomcu_axi_ahb_mainnopendingtrans_START (5)
#define SOC_SCTRL_SCPERSTATUS7_iomcu_axi_ahb_mainnopendingtrans_END (5)
#define SOC_SCTRL_SCPERSTATUS7_iomcu_ahb_mst_tcp_mainnopendingtrans_START (6)
#define SOC_SCTRL_SCPERSTATUS7_iomcu_ahb_mst_tcp_mainnopendingtrans_END (6)
#define SOC_SCTRL_SCPERSTATUS7_ufs_axi_nopendingtrans_START (7)
#define SOC_SCTRL_SCPERSTATUS7_ufs_axi_nopendingtrans_END (7)
#define SOC_SCTRL_SCPERSTATUS7_emmc_axi_nopendingtrans_START (8)
#define SOC_SCTRL_SCPERSTATUS7_emmc_axi_nopendingtrans_END (8)
#define SOC_SCTRL_SCPERSTATUS7_ufs_emmc_service_nopendingtrans_START (9)
#define SOC_SCTRL_SCPERSTATUS7_ufs_emmc_service_nopendingtrans_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS9_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ao_hpm_opc : 10;
        unsigned int ao_hpm_opc_vld : 1;
        unsigned int reserved : 1;
        unsigned int ao_hpmx_opc : 10;
        unsigned int ao_hpmx_opc_vld : 1;
        unsigned int scperstatus13 : 9;
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
#define SOC_SCTRL_SCPERSTATUS13_scperstatus13_START (23)
#define SOC_SCTRL_SCPERSTATUS13_scperstatus13_END (31)
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
        unsigned int ddrc_reset_state : 1;
        unsigned int ddrd_reset_state : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_SCTRL_SCPERSTATUS15_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS15_ddra_reset_state_START (0)
#define SOC_SCTRL_SCPERSTATUS15_ddra_reset_state_END (0)
#define SOC_SCTRL_SCPERSTATUS15_ddrb_reset_state_START (1)
#define SOC_SCTRL_SCPERSTATUS15_ddrb_reset_state_END (1)
#define SOC_SCTRL_SCPERSTATUS15_ddrc_reset_state_START (2)
#define SOC_SCTRL_SCPERSTATUS15_ddrc_reset_state_END (2)
#define SOC_SCTRL_SCPERSTATUS15_ddrd_reset_state_START (3)
#define SOC_SCTRL_SCPERSTATUS15_ddrd_reset_state_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int memory_repair_bypass : 1;
        unsigned int hardw_ctrl_sel0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int memory_repair_bypass_hw : 10;
        unsigned int isp_face_detect_fuse : 1;
        unsigned int emmc_ufs_sel : 1;
        unsigned int ate_mode : 5;
        unsigned int vote_clk_gt_sel_flag : 7;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_SCTRL_SCINNERSTAT_UNION;
#endif
#define SOC_SCTRL_SCINNERSTAT_memory_repair_bypass_START (0)
#define SOC_SCTRL_SCINNERSTAT_memory_repair_bypass_END (0)
#define SOC_SCTRL_SCINNERSTAT_hardw_ctrl_sel0_START (1)
#define SOC_SCTRL_SCINNERSTAT_hardw_ctrl_sel0_END (1)
#define SOC_SCTRL_SCINNERSTAT_memory_repair_bypass_hw_START (4)
#define SOC_SCTRL_SCINNERSTAT_memory_repair_bypass_hw_END (13)
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
        unsigned int iomcu_ao_noc_mainstatalarm_intmsk : 1;
        unsigned int iomcu_ao_noc_mainfault_intmsk : 1;
        unsigned int asp_transaction_probe_mainstatalarm_intmsk : 1;
        unsigned int asp_error_probe_observer_mainfault_intmsk : 1;
        unsigned int reserved_0 : 1;
        unsigned int msk_noc_asp_cfg_t_maintimeout_intmsk : 1;
        unsigned int msk_iomcu_ahb_slv_maintimeout_intmsk : 1;
        unsigned int msk_iomcu_apb_slv_maintimeout_intmsk : 1;
        unsigned int reserved_1 : 1;
        unsigned int sc_noc_aon_apb_slv_t_maintimeout_intmsk : 1;
        unsigned int aobus_error_probe_observer_mainfault_0_intmsk : 1;
        unsigned int reserved_2 : 1;
        unsigned int ufs_emmc_apb_slv_maintimeout_intmsk : 1;
        unsigned int emmc_ahb_slv_maintimeout_intmsk : 1;
        unsigned int reserved_3 : 18;
    } reg;
} SOC_SCTRL_SC_SECOND_INT_MASK_UNION;
#endif
#define SOC_SCTRL_SC_SECOND_INT_MASK_iomcu_ao_noc_mainstatalarm_intmsk_START (0)
#define SOC_SCTRL_SC_SECOND_INT_MASK_iomcu_ao_noc_mainstatalarm_intmsk_END (0)
#define SOC_SCTRL_SC_SECOND_INT_MASK_iomcu_ao_noc_mainfault_intmsk_START (1)
#define SOC_SCTRL_SC_SECOND_INT_MASK_iomcu_ao_noc_mainfault_intmsk_END (1)
#define SOC_SCTRL_SC_SECOND_INT_MASK_asp_transaction_probe_mainstatalarm_intmsk_START (2)
#define SOC_SCTRL_SC_SECOND_INT_MASK_asp_transaction_probe_mainstatalarm_intmsk_END (2)
#define SOC_SCTRL_SC_SECOND_INT_MASK_asp_error_probe_observer_mainfault_intmsk_START (3)
#define SOC_SCTRL_SC_SECOND_INT_MASK_asp_error_probe_observer_mainfault_intmsk_END (3)
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
#define SOC_SCTRL_SC_SECOND_INT_MASK_ufs_emmc_apb_slv_maintimeout_intmsk_START (12)
#define SOC_SCTRL_SC_SECOND_INT_MASK_ufs_emmc_apb_slv_maintimeout_intmsk_END (12)
#define SOC_SCTRL_SC_SECOND_INT_MASK_emmc_ahb_slv_maintimeout_intmsk_START (13)
#define SOC_SCTRL_SC_SECOND_INT_MASK_emmc_ahb_slv_maintimeout_intmsk_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int iomcu_ao_noc_mainstatalarm_int : 1;
        unsigned int iomcu_ao_noc_mainfault_int : 1;
        unsigned int asp_transaction_probe_mainstatalarm_int : 1;
        unsigned int asp_error_probe_observer_mainfault_int : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_asp_cfg_t_maintimeout : 1;
        unsigned int iomcu_ahb_slv_maintimeout : 1;
        unsigned int iomcu_apb_slv_maintimeout : 1;
        unsigned int reserved_1 : 1;
        unsigned int sc_noc_aon_apb_slv_t_maintimeout : 1;
        unsigned int aobus_error_probe_observer_mainfault_0 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ufs_emmc_apb_slv_maintimeout : 1;
        unsigned int emmc_ahb_slv_maintimeout : 1;
        unsigned int reserved_3 : 18;
    } reg;
} SOC_SCTRL_SC_SECOND_INT_ORG_UNION;
#endif
#define SOC_SCTRL_SC_SECOND_INT_ORG_iomcu_ao_noc_mainstatalarm_int_START (0)
#define SOC_SCTRL_SC_SECOND_INT_ORG_iomcu_ao_noc_mainstatalarm_int_END (0)
#define SOC_SCTRL_SC_SECOND_INT_ORG_iomcu_ao_noc_mainfault_int_START (1)
#define SOC_SCTRL_SC_SECOND_INT_ORG_iomcu_ao_noc_mainfault_int_END (1)
#define SOC_SCTRL_SC_SECOND_INT_ORG_asp_transaction_probe_mainstatalarm_int_START (2)
#define SOC_SCTRL_SC_SECOND_INT_ORG_asp_transaction_probe_mainstatalarm_int_END (2)
#define SOC_SCTRL_SC_SECOND_INT_ORG_asp_error_probe_observer_mainfault_int_START (3)
#define SOC_SCTRL_SC_SECOND_INT_ORG_asp_error_probe_observer_mainfault_int_END (3)
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
#define SOC_SCTRL_SC_SECOND_INT_ORG_ufs_emmc_apb_slv_maintimeout_START (12)
#define SOC_SCTRL_SC_SECOND_INT_ORG_ufs_emmc_apb_slv_maintimeout_END (12)
#define SOC_SCTRL_SC_SECOND_INT_ORG_emmc_ahb_slv_maintimeout_START (13)
#define SOC_SCTRL_SC_SECOND_INT_ORG_emmc_ahb_slv_maintimeout_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int iomcu_ao_noc_mainstatalarm_int_msksta : 1;
        unsigned int iomcu_ao_noc_mainfault_int_msksta : 1;
        unsigned int asp_transaction_probe_mainstatalarm_int_msksta : 1;
        unsigned int asp_error_probe_observer_mainfault_int_msksta : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_asp_cfg_t_maintimeout_msksta : 1;
        unsigned int iomcu_ahb_slv_maintimeout_msksta : 1;
        unsigned int iomcu_apb_slv_maintimeout_msksta : 1;
        unsigned int reserved_1 : 1;
        unsigned int sc_noc_aon_apb_slv_t_maintimeout_msksta : 1;
        unsigned int aobus_error_probe_observer_mainfault_0_msksta : 1;
        unsigned int intr_ufs_emmc_error_probe_observer_mainfault_msksta : 1;
        unsigned int ufs_emmc_apb_slv_maintimeout_msksta : 1;
        unsigned int emmc_ahb_slv_maintimeout_msksta : 1;
        unsigned int reserved_2 : 18;
    } reg;
} SOC_SCTRL_SC_SECOND_INT_OUT_UNION;
#endif
#define SOC_SCTRL_SC_SECOND_INT_OUT_iomcu_ao_noc_mainstatalarm_int_msksta_START (0)
#define SOC_SCTRL_SC_SECOND_INT_OUT_iomcu_ao_noc_mainstatalarm_int_msksta_END (0)
#define SOC_SCTRL_SC_SECOND_INT_OUT_iomcu_ao_noc_mainfault_int_msksta_START (1)
#define SOC_SCTRL_SC_SECOND_INT_OUT_iomcu_ao_noc_mainfault_int_msksta_END (1)
#define SOC_SCTRL_SC_SECOND_INT_OUT_asp_transaction_probe_mainstatalarm_int_msksta_START (2)
#define SOC_SCTRL_SC_SECOND_INT_OUT_asp_transaction_probe_mainstatalarm_int_msksta_END (2)
#define SOC_SCTRL_SC_SECOND_INT_OUT_asp_error_probe_observer_mainfault_int_msksta_START (3)
#define SOC_SCTRL_SC_SECOND_INT_OUT_asp_error_probe_observer_mainfault_int_msksta_END (3)
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
#define SOC_SCTRL_SC_SECOND_INT_OUT_intr_ufs_emmc_error_probe_observer_mainfault_msksta_START (11)
#define SOC_SCTRL_SC_SECOND_INT_OUT_intr_ufs_emmc_error_probe_observer_mainfault_msksta_END (11)
#define SOC_SCTRL_SC_SECOND_INT_OUT_ufs_emmc_apb_slv_maintimeout_msksta_START (12)
#define SOC_SCTRL_SC_SECOND_INT_OUT_ufs_emmc_apb_slv_maintimeout_msksta_END (12)
#define SOC_SCTRL_SC_SECOND_INT_OUT_emmc_ahb_slv_maintimeout_msksta_START (13)
#define SOC_SCTRL_SC_SECOND_INT_OUT_emmc_ahb_slv_maintimeout_msksta_END (13)
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
        unsigned int dapclkoff_sys_n : 1;
        unsigned int pmussi1_clk_clkoff_sys_n : 1;
        unsigned int rom_clk_clkoff_sys_n : 1;
        unsigned int ram_clk_clkoff_sys_n : 1;
        unsigned int reserved_0 : 1;
        unsigned int ddrc_clk_clkoff_sys_n : 1;
        unsigned int tsen_clk_clkoff_sys_n : 1;
        unsigned int pmc_clk_clkoff_sys_n : 1;
        unsigned int uart_clk_clkoff_sys_n : 1;
        unsigned int reserved_1 : 1;
        unsigned int ssi_clk_clkoff_sys_n : 1;
        unsigned int pcrg_clk_clkoff_sys_n : 1;
        unsigned int reserved_2 : 1;
        unsigned int wdog_clk_clkoff_sys_n : 1;
        unsigned int tim_clk_clkoff_sys_n : 1;
        unsigned int cssys_clk_clkoff_sys_n : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 14;
    } reg;
} SOC_SCTRL_SCLPMCUCLKEN_UNION;
#endif
#define SOC_SCTRL_SCLPMCUCLKEN_dapclkoff_sys_n_START (0)
#define SOC_SCTRL_SCLPMCUCLKEN_dapclkoff_sys_n_END (0)
#define SOC_SCTRL_SCLPMCUCLKEN_pmussi1_clk_clkoff_sys_n_START (1)
#define SOC_SCTRL_SCLPMCUCLKEN_pmussi1_clk_clkoff_sys_n_END (1)
#define SOC_SCTRL_SCLPMCUCLKEN_rom_clk_clkoff_sys_n_START (2)
#define SOC_SCTRL_SCLPMCUCLKEN_rom_clk_clkoff_sys_n_END (2)
#define SOC_SCTRL_SCLPMCUCLKEN_ram_clk_clkoff_sys_n_START (3)
#define SOC_SCTRL_SCLPMCUCLKEN_ram_clk_clkoff_sys_n_END (3)
#define SOC_SCTRL_SCLPMCUCLKEN_ddrc_clk_clkoff_sys_n_START (5)
#define SOC_SCTRL_SCLPMCUCLKEN_ddrc_clk_clkoff_sys_n_END (5)
#define SOC_SCTRL_SCLPMCUCLKEN_tsen_clk_clkoff_sys_n_START (6)
#define SOC_SCTRL_SCLPMCUCLKEN_tsen_clk_clkoff_sys_n_END (6)
#define SOC_SCTRL_SCLPMCUCLKEN_pmc_clk_clkoff_sys_n_START (7)
#define SOC_SCTRL_SCLPMCUCLKEN_pmc_clk_clkoff_sys_n_END (7)
#define SOC_SCTRL_SCLPMCUCLKEN_uart_clk_clkoff_sys_n_START (8)
#define SOC_SCTRL_SCLPMCUCLKEN_uart_clk_clkoff_sys_n_END (8)
#define SOC_SCTRL_SCLPMCUCLKEN_ssi_clk_clkoff_sys_n_START (10)
#define SOC_SCTRL_SCLPMCUCLKEN_ssi_clk_clkoff_sys_n_END (10)
#define SOC_SCTRL_SCLPMCUCLKEN_pcrg_clk_clkoff_sys_n_START (11)
#define SOC_SCTRL_SCLPMCUCLKEN_pcrg_clk_clkoff_sys_n_END (11)
#define SOC_SCTRL_SCLPMCUCLKEN_wdog_clk_clkoff_sys_n_START (13)
#define SOC_SCTRL_SCLPMCUCLKEN_wdog_clk_clkoff_sys_n_END (13)
#define SOC_SCTRL_SCLPMCUCLKEN_tim_clk_clkoff_sys_n_START (14)
#define SOC_SCTRL_SCLPMCUCLKEN_tim_clk_clkoff_sys_n_END (14)
#define SOC_SCTRL_SCLPMCUCLKEN_cssys_clk_clkoff_sys_n_START (15)
#define SOC_SCTRL_SCLPMCUCLKEN_cssys_clk_clkoff_sys_n_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dapclkoff_sys_n : 1;
        unsigned int pmussi1_clk_clkoff_sys_n : 1;
        unsigned int rom_clk_clkoff_sys_n : 1;
        unsigned int ram_clk_clkoff_sys_n : 1;
        unsigned int reserved_0 : 1;
        unsigned int ddrc_clk_clkoff_sys_n : 1;
        unsigned int tsen_clk_clkoff_sys_n : 1;
        unsigned int pmc_clk_clkoff_sys_n : 1;
        unsigned int uart_clk_clkoff_sys_n : 1;
        unsigned int reserved_1 : 1;
        unsigned int ssi_clk_clkoff_sys_n : 1;
        unsigned int pcrg_clk_clkoff_sys_n : 1;
        unsigned int reserved_2 : 1;
        unsigned int wdog_clk_clkoff_sys_n : 1;
        unsigned int tim_clk_clkoff_sys_n : 1;
        unsigned int cssys_clk_clkoff_sys_n : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 14;
    } reg;
} SOC_SCTRL_SCLPMCUCLKDIS_UNION;
#endif
#define SOC_SCTRL_SCLPMCUCLKDIS_dapclkoff_sys_n_START (0)
#define SOC_SCTRL_SCLPMCUCLKDIS_dapclkoff_sys_n_END (0)
#define SOC_SCTRL_SCLPMCUCLKDIS_pmussi1_clk_clkoff_sys_n_START (1)
#define SOC_SCTRL_SCLPMCUCLKDIS_pmussi1_clk_clkoff_sys_n_END (1)
#define SOC_SCTRL_SCLPMCUCLKDIS_rom_clk_clkoff_sys_n_START (2)
#define SOC_SCTRL_SCLPMCUCLKDIS_rom_clk_clkoff_sys_n_END (2)
#define SOC_SCTRL_SCLPMCUCLKDIS_ram_clk_clkoff_sys_n_START (3)
#define SOC_SCTRL_SCLPMCUCLKDIS_ram_clk_clkoff_sys_n_END (3)
#define SOC_SCTRL_SCLPMCUCLKDIS_ddrc_clk_clkoff_sys_n_START (5)
#define SOC_SCTRL_SCLPMCUCLKDIS_ddrc_clk_clkoff_sys_n_END (5)
#define SOC_SCTRL_SCLPMCUCLKDIS_tsen_clk_clkoff_sys_n_START (6)
#define SOC_SCTRL_SCLPMCUCLKDIS_tsen_clk_clkoff_sys_n_END (6)
#define SOC_SCTRL_SCLPMCUCLKDIS_pmc_clk_clkoff_sys_n_START (7)
#define SOC_SCTRL_SCLPMCUCLKDIS_pmc_clk_clkoff_sys_n_END (7)
#define SOC_SCTRL_SCLPMCUCLKDIS_uart_clk_clkoff_sys_n_START (8)
#define SOC_SCTRL_SCLPMCUCLKDIS_uart_clk_clkoff_sys_n_END (8)
#define SOC_SCTRL_SCLPMCUCLKDIS_ssi_clk_clkoff_sys_n_START (10)
#define SOC_SCTRL_SCLPMCUCLKDIS_ssi_clk_clkoff_sys_n_END (10)
#define SOC_SCTRL_SCLPMCUCLKDIS_pcrg_clk_clkoff_sys_n_START (11)
#define SOC_SCTRL_SCLPMCUCLKDIS_pcrg_clk_clkoff_sys_n_END (11)
#define SOC_SCTRL_SCLPMCUCLKDIS_wdog_clk_clkoff_sys_n_START (13)
#define SOC_SCTRL_SCLPMCUCLKDIS_wdog_clk_clkoff_sys_n_END (13)
#define SOC_SCTRL_SCLPMCUCLKDIS_tim_clk_clkoff_sys_n_START (14)
#define SOC_SCTRL_SCLPMCUCLKDIS_tim_clk_clkoff_sys_n_END (14)
#define SOC_SCTRL_SCLPMCUCLKDIS_cssys_clk_clkoff_sys_n_START (15)
#define SOC_SCTRL_SCLPMCUCLKDIS_cssys_clk_clkoff_sys_n_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dapclkoff_sys_n : 1;
        unsigned int pmussi1_clk_clkoff_sys_n : 1;
        unsigned int rom_clk_clkoff_sys_n : 1;
        unsigned int ram_clk_clkoff_sys_n : 1;
        unsigned int reserved_0 : 1;
        unsigned int ddrc_clk_clkoff_sys_n : 1;
        unsigned int tsen_clk_clkoff_sys_n : 1;
        unsigned int pmc_clk_clkoff_sys_n : 1;
        unsigned int uart_clk_clkoff_sys_n : 1;
        unsigned int reserved_1 : 1;
        unsigned int ssi_clk_clkoff_sys_n : 1;
        unsigned int pcrg_clk_clkoff_sys_n : 1;
        unsigned int reserved_2 : 1;
        unsigned int wdog_clk_clkoff_sys_n : 1;
        unsigned int tim_clk_clkoff_sys_n : 1;
        unsigned int cssys_clk_clkoff_sys_n : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 14;
    } reg;
} SOC_SCTRL_SCLPMCUCLKSTAT_UNION;
#endif
#define SOC_SCTRL_SCLPMCUCLKSTAT_dapclkoff_sys_n_START (0)
#define SOC_SCTRL_SCLPMCUCLKSTAT_dapclkoff_sys_n_END (0)
#define SOC_SCTRL_SCLPMCUCLKSTAT_pmussi1_clk_clkoff_sys_n_START (1)
#define SOC_SCTRL_SCLPMCUCLKSTAT_pmussi1_clk_clkoff_sys_n_END (1)
#define SOC_SCTRL_SCLPMCUCLKSTAT_rom_clk_clkoff_sys_n_START (2)
#define SOC_SCTRL_SCLPMCUCLKSTAT_rom_clk_clkoff_sys_n_END (2)
#define SOC_SCTRL_SCLPMCUCLKSTAT_ram_clk_clkoff_sys_n_START (3)
#define SOC_SCTRL_SCLPMCUCLKSTAT_ram_clk_clkoff_sys_n_END (3)
#define SOC_SCTRL_SCLPMCUCLKSTAT_ddrc_clk_clkoff_sys_n_START (5)
#define SOC_SCTRL_SCLPMCUCLKSTAT_ddrc_clk_clkoff_sys_n_END (5)
#define SOC_SCTRL_SCLPMCUCLKSTAT_tsen_clk_clkoff_sys_n_START (6)
#define SOC_SCTRL_SCLPMCUCLKSTAT_tsen_clk_clkoff_sys_n_END (6)
#define SOC_SCTRL_SCLPMCUCLKSTAT_pmc_clk_clkoff_sys_n_START (7)
#define SOC_SCTRL_SCLPMCUCLKSTAT_pmc_clk_clkoff_sys_n_END (7)
#define SOC_SCTRL_SCLPMCUCLKSTAT_uart_clk_clkoff_sys_n_START (8)
#define SOC_SCTRL_SCLPMCUCLKSTAT_uart_clk_clkoff_sys_n_END (8)
#define SOC_SCTRL_SCLPMCUCLKSTAT_ssi_clk_clkoff_sys_n_START (10)
#define SOC_SCTRL_SCLPMCUCLKSTAT_ssi_clk_clkoff_sys_n_END (10)
#define SOC_SCTRL_SCLPMCUCLKSTAT_pcrg_clk_clkoff_sys_n_START (11)
#define SOC_SCTRL_SCLPMCUCLKSTAT_pcrg_clk_clkoff_sys_n_END (11)
#define SOC_SCTRL_SCLPMCUCLKSTAT_wdog_clk_clkoff_sys_n_START (13)
#define SOC_SCTRL_SCLPMCUCLKSTAT_wdog_clk_clkoff_sys_n_END (13)
#define SOC_SCTRL_SCLPMCUCLKSTAT_tim_clk_clkoff_sys_n_START (14)
#define SOC_SCTRL_SCLPMCUCLKSTAT_tim_clk_clkoff_sys_n_END (14)
#define SOC_SCTRL_SCLPMCUCLKSTAT_cssys_clk_clkoff_sys_n_START (15)
#define SOC_SCTRL_SCLPMCUCLKSTAT_cssys_clk_clkoff_sys_n_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rst_software_req : 1;
        unsigned int coresight_soft_rst_req : 1;
        unsigned int pmussi1_soft_rst_req : 1;
        unsigned int rom_soft_rst_req : 1;
        unsigned int ram_soft_rst_req : 1;
        unsigned int reserved_0 : 1;
        unsigned int ddrc_soft_rst_req : 1;
        unsigned int tsen_soft_rst_req : 1;
        unsigned int pmc_soft_rst_req : 1;
        unsigned int uart_soft_rst_req : 1;
        unsigned int reserved_1 : 1;
        unsigned int ssi_soft_rst_req : 1;
        unsigned int pcrg_soft_rst_req : 1;
        unsigned int reserved_2 : 1;
        unsigned int wdog_soft_rst_req : 1;
        unsigned int tim_soft_rst_req : 1;
        unsigned int cssys_soft_rst_req : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 13;
    } reg;
} SOC_SCTRL_SCLPMCURSTEN_UNION;
#endif
#define SOC_SCTRL_SCLPMCURSTEN_rst_software_req_START (0)
#define SOC_SCTRL_SCLPMCURSTEN_rst_software_req_END (0)
#define SOC_SCTRL_SCLPMCURSTEN_coresight_soft_rst_req_START (1)
#define SOC_SCTRL_SCLPMCURSTEN_coresight_soft_rst_req_END (1)
#define SOC_SCTRL_SCLPMCURSTEN_pmussi1_soft_rst_req_START (2)
#define SOC_SCTRL_SCLPMCURSTEN_pmussi1_soft_rst_req_END (2)
#define SOC_SCTRL_SCLPMCURSTEN_rom_soft_rst_req_START (3)
#define SOC_SCTRL_SCLPMCURSTEN_rom_soft_rst_req_END (3)
#define SOC_SCTRL_SCLPMCURSTEN_ram_soft_rst_req_START (4)
#define SOC_SCTRL_SCLPMCURSTEN_ram_soft_rst_req_END (4)
#define SOC_SCTRL_SCLPMCURSTEN_ddrc_soft_rst_req_START (6)
#define SOC_SCTRL_SCLPMCURSTEN_ddrc_soft_rst_req_END (6)
#define SOC_SCTRL_SCLPMCURSTEN_tsen_soft_rst_req_START (7)
#define SOC_SCTRL_SCLPMCURSTEN_tsen_soft_rst_req_END (7)
#define SOC_SCTRL_SCLPMCURSTEN_pmc_soft_rst_req_START (8)
#define SOC_SCTRL_SCLPMCURSTEN_pmc_soft_rst_req_END (8)
#define SOC_SCTRL_SCLPMCURSTEN_uart_soft_rst_req_START (9)
#define SOC_SCTRL_SCLPMCURSTEN_uart_soft_rst_req_END (9)
#define SOC_SCTRL_SCLPMCURSTEN_ssi_soft_rst_req_START (11)
#define SOC_SCTRL_SCLPMCURSTEN_ssi_soft_rst_req_END (11)
#define SOC_SCTRL_SCLPMCURSTEN_pcrg_soft_rst_req_START (12)
#define SOC_SCTRL_SCLPMCURSTEN_pcrg_soft_rst_req_END (12)
#define SOC_SCTRL_SCLPMCURSTEN_wdog_soft_rst_req_START (14)
#define SOC_SCTRL_SCLPMCURSTEN_wdog_soft_rst_req_END (14)
#define SOC_SCTRL_SCLPMCURSTEN_tim_soft_rst_req_START (15)
#define SOC_SCTRL_SCLPMCURSTEN_tim_soft_rst_req_END (15)
#define SOC_SCTRL_SCLPMCURSTEN_cssys_soft_rst_req_START (16)
#define SOC_SCTRL_SCLPMCURSTEN_cssys_soft_rst_req_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rst_software_req : 1;
        unsigned int coresight_soft_rst_req : 1;
        unsigned int pmussi1_soft_rst_req : 1;
        unsigned int rom_soft_rst_req : 1;
        unsigned int ram_soft_rst_req : 1;
        unsigned int reserved_0 : 1;
        unsigned int ddrc_soft_rst_req : 1;
        unsigned int tsen_soft_rst_req : 1;
        unsigned int pmc_soft_rst_req : 1;
        unsigned int uart_soft_rst_req : 1;
        unsigned int reserved_1 : 1;
        unsigned int ssi_soft_rst_req : 1;
        unsigned int pcrg_soft_rst_req : 1;
        unsigned int reserved_2 : 1;
        unsigned int wdog_soft_rst_req : 1;
        unsigned int tim_soft_rst_req : 1;
        unsigned int cssys_soft_rst_req : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 13;
    } reg;
} SOC_SCTRL_SCLPMCURSTDIS_UNION;
#endif
#define SOC_SCTRL_SCLPMCURSTDIS_rst_software_req_START (0)
#define SOC_SCTRL_SCLPMCURSTDIS_rst_software_req_END (0)
#define SOC_SCTRL_SCLPMCURSTDIS_coresight_soft_rst_req_START (1)
#define SOC_SCTRL_SCLPMCURSTDIS_coresight_soft_rst_req_END (1)
#define SOC_SCTRL_SCLPMCURSTDIS_pmussi1_soft_rst_req_START (2)
#define SOC_SCTRL_SCLPMCURSTDIS_pmussi1_soft_rst_req_END (2)
#define SOC_SCTRL_SCLPMCURSTDIS_rom_soft_rst_req_START (3)
#define SOC_SCTRL_SCLPMCURSTDIS_rom_soft_rst_req_END (3)
#define SOC_SCTRL_SCLPMCURSTDIS_ram_soft_rst_req_START (4)
#define SOC_SCTRL_SCLPMCURSTDIS_ram_soft_rst_req_END (4)
#define SOC_SCTRL_SCLPMCURSTDIS_ddrc_soft_rst_req_START (6)
#define SOC_SCTRL_SCLPMCURSTDIS_ddrc_soft_rst_req_END (6)
#define SOC_SCTRL_SCLPMCURSTDIS_tsen_soft_rst_req_START (7)
#define SOC_SCTRL_SCLPMCURSTDIS_tsen_soft_rst_req_END (7)
#define SOC_SCTRL_SCLPMCURSTDIS_pmc_soft_rst_req_START (8)
#define SOC_SCTRL_SCLPMCURSTDIS_pmc_soft_rst_req_END (8)
#define SOC_SCTRL_SCLPMCURSTDIS_uart_soft_rst_req_START (9)
#define SOC_SCTRL_SCLPMCURSTDIS_uart_soft_rst_req_END (9)
#define SOC_SCTRL_SCLPMCURSTDIS_ssi_soft_rst_req_START (11)
#define SOC_SCTRL_SCLPMCURSTDIS_ssi_soft_rst_req_END (11)
#define SOC_SCTRL_SCLPMCURSTDIS_pcrg_soft_rst_req_START (12)
#define SOC_SCTRL_SCLPMCURSTDIS_pcrg_soft_rst_req_END (12)
#define SOC_SCTRL_SCLPMCURSTDIS_wdog_soft_rst_req_START (14)
#define SOC_SCTRL_SCLPMCURSTDIS_wdog_soft_rst_req_END (14)
#define SOC_SCTRL_SCLPMCURSTDIS_tim_soft_rst_req_START (15)
#define SOC_SCTRL_SCLPMCURSTDIS_tim_soft_rst_req_END (15)
#define SOC_SCTRL_SCLPMCURSTDIS_cssys_soft_rst_req_START (16)
#define SOC_SCTRL_SCLPMCURSTDIS_cssys_soft_rst_req_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rst_software_req : 1;
        unsigned int coresight_soft_rst_req : 1;
        unsigned int pmussi1_soft_rst_req : 1;
        unsigned int rom_soft_rst_req : 1;
        unsigned int ram_soft_rst_req : 1;
        unsigned int reserved_0 : 1;
        unsigned int ddrc_soft_rst_req : 1;
        unsigned int tsen_soft_rst_req : 1;
        unsigned int pmc_soft_rst_req : 1;
        unsigned int uart_soft_rst_req : 1;
        unsigned int reserved_1 : 1;
        unsigned int ssi_soft_rst_req : 1;
        unsigned int pcrg_soft_rst_req : 1;
        unsigned int reserved_2 : 1;
        unsigned int wdog_soft_rst_req : 1;
        unsigned int tim_soft_rst_req : 1;
        unsigned int cssys_soft_rst_req : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 13;
    } reg;
} SOC_SCTRL_SCLPMCURSTSTAT_UNION;
#endif
#define SOC_SCTRL_SCLPMCURSTSTAT_rst_software_req_START (0)
#define SOC_SCTRL_SCLPMCURSTSTAT_rst_software_req_END (0)
#define SOC_SCTRL_SCLPMCURSTSTAT_coresight_soft_rst_req_START (1)
#define SOC_SCTRL_SCLPMCURSTSTAT_coresight_soft_rst_req_END (1)
#define SOC_SCTRL_SCLPMCURSTSTAT_pmussi1_soft_rst_req_START (2)
#define SOC_SCTRL_SCLPMCURSTSTAT_pmussi1_soft_rst_req_END (2)
#define SOC_SCTRL_SCLPMCURSTSTAT_rom_soft_rst_req_START (3)
#define SOC_SCTRL_SCLPMCURSTSTAT_rom_soft_rst_req_END (3)
#define SOC_SCTRL_SCLPMCURSTSTAT_ram_soft_rst_req_START (4)
#define SOC_SCTRL_SCLPMCURSTSTAT_ram_soft_rst_req_END (4)
#define SOC_SCTRL_SCLPMCURSTSTAT_ddrc_soft_rst_req_START (6)
#define SOC_SCTRL_SCLPMCURSTSTAT_ddrc_soft_rst_req_END (6)
#define SOC_SCTRL_SCLPMCURSTSTAT_tsen_soft_rst_req_START (7)
#define SOC_SCTRL_SCLPMCURSTSTAT_tsen_soft_rst_req_END (7)
#define SOC_SCTRL_SCLPMCURSTSTAT_pmc_soft_rst_req_START (8)
#define SOC_SCTRL_SCLPMCURSTSTAT_pmc_soft_rst_req_END (8)
#define SOC_SCTRL_SCLPMCURSTSTAT_uart_soft_rst_req_START (9)
#define SOC_SCTRL_SCLPMCURSTSTAT_uart_soft_rst_req_END (9)
#define SOC_SCTRL_SCLPMCURSTSTAT_ssi_soft_rst_req_START (11)
#define SOC_SCTRL_SCLPMCURSTSTAT_ssi_soft_rst_req_END (11)
#define SOC_SCTRL_SCLPMCURSTSTAT_pcrg_soft_rst_req_START (12)
#define SOC_SCTRL_SCLPMCURSTSTAT_pcrg_soft_rst_req_END (12)
#define SOC_SCTRL_SCLPMCURSTSTAT_wdog_soft_rst_req_START (14)
#define SOC_SCTRL_SCLPMCURSTSTAT_wdog_soft_rst_req_END (14)
#define SOC_SCTRL_SCLPMCURSTSTAT_tim_soft_rst_req_START (15)
#define SOC_SCTRL_SCLPMCURSTSTAT_tim_soft_rst_req_END (15)
#define SOC_SCTRL_SCLPMCURSTSTAT_cssys_soft_rst_req_START (16)
#define SOC_SCTRL_SCLPMCURSTSTAT_cssys_soft_rst_req_END (16)
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
        unsigned int reserved_0 : 1;
        unsigned int sleeping : 1;
        unsigned int lockup : 1;
        unsigned int halted : 1;
        unsigned int ocbc_fll_en_req : 1;
        unsigned int ocbc_ppll0_gt_req : 1;
        unsigned int ocbc_ppll0_bypass_req : 1;
        unsigned int ocbc_ppll0_en_req : 1;
        unsigned int ocbc_sw_req : 4;
        unsigned int ocbc_tcxo_gt : 1;
        unsigned int ocbc_fll_gt : 1;
        unsigned int ocbc_ppll0_gt : 1;
        unsigned int reserved_1 : 17;
    } reg;
} SOC_SCTRL_SCOCBCSTAT_UNION;
#endif
#define SOC_SCTRL_SCOCBCSTAT_sleeping_START (1)
#define SOC_SCTRL_SCOCBCSTAT_sleeping_END (1)
#define SOC_SCTRL_SCOCBCSTAT_lockup_START (2)
#define SOC_SCTRL_SCOCBCSTAT_lockup_END (2)
#define SOC_SCTRL_SCOCBCSTAT_halted_START (3)
#define SOC_SCTRL_SCOCBCSTAT_halted_END (3)
#define SOC_SCTRL_SCOCBCSTAT_ocbc_fll_en_req_START (4)
#define SOC_SCTRL_SCOCBCSTAT_ocbc_fll_en_req_END (4)
#define SOC_SCTRL_SCOCBCSTAT_ocbc_ppll0_gt_req_START (5)
#define SOC_SCTRL_SCOCBCSTAT_ocbc_ppll0_gt_req_END (5)
#define SOC_SCTRL_SCOCBCSTAT_ocbc_ppll0_bypass_req_START (6)
#define SOC_SCTRL_SCOCBCSTAT_ocbc_ppll0_bypass_req_END (6)
#define SOC_SCTRL_SCOCBCSTAT_ocbc_ppll0_en_req_START (7)
#define SOC_SCTRL_SCOCBCSTAT_ocbc_ppll0_en_req_END (7)
#define SOC_SCTRL_SCOCBCSTAT_ocbc_sw_req_START (8)
#define SOC_SCTRL_SCOCBCSTAT_ocbc_sw_req_END (11)
#define SOC_SCTRL_SCOCBCSTAT_ocbc_tcxo_gt_START (12)
#define SOC_SCTRL_SCOCBCSTAT_ocbc_tcxo_gt_END (12)
#define SOC_SCTRL_SCOCBCSTAT_ocbc_fll_gt_START (13)
#define SOC_SCTRL_SCOCBCSTAT_ocbc_fll_gt_END (13)
#define SOC_SCTRL_SCOCBCSTAT_ocbc_ppll0_gt_START (14)
#define SOC_SCTRL_SCOCBCSTAT_ocbc_ppll0_gt_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mpudisable : 1;
        unsigned int reserved : 15;
        unsigned int scocbcctrl_msk : 16;
    } reg;
} SOC_SCTRL_SCOCBCCTRL_UNION;
#endif
#define SOC_SCTRL_SCOCBCCTRL_mpudisable_START (0)
#define SOC_SCTRL_SCOCBCCTRL_mpudisable_END (0)
#define SOC_SCTRL_SCOCBCCTRL_scocbcctrl_msk_START (16)
#define SOC_SCTRL_SCOCBCCTRL_scocbcctrl_msk_END (31)
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
        unsigned int ocbc_event_mask : 1;
        unsigned int reserved : 25;
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
#define SOC_SCTRL_SCA53_EVENT_MASK_ocbc_event_mask_START (6)
#define SOC_SCTRL_SCA53_EVENT_MASK_ocbc_event_mask_END (6)
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
        unsigned int ocbc_event_mask : 1;
        unsigned int reserved : 25;
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
#define SOC_SCTRL_SCA57_EVENT_MASK_ocbc_event_mask_START (6)
#define SOC_SCTRL_SCA57_EVENT_MASK_ocbc_event_mask_END (6)
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
        unsigned int ocbc_event_mask : 1;
        unsigned int reserved : 25;
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
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_ocbc_event_mask_START (6)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_ocbc_event_mask_END (6)
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
        unsigned int ocbc_event_mask : 1;
        unsigned int reserved : 25;
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
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_ocbc_event_mask_START (6)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_ocbc_event_mask_END (6)
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
        unsigned int ocbc_event_mask : 1;
        unsigned int reserved : 25;
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
#define SOC_SCTRL_SCMCPU_EVENT_MASK_ocbc_event_mask_START (6)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_ocbc_event_mask_END (6)
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
        unsigned int ocbc_event : 1;
        unsigned int reserved : 25;
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
#define SOC_SCTRL_SCEVENT_STAT_ocbc_event_START (6)
#define SOC_SCTRL_SCEVENT_STAT_ocbc_event_END (6)
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
        unsigned int ocbc_event_mask : 1;
        unsigned int reserved : 25;
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
#define SOC_SCTRL_SCISPA7_EVENT_MASK_ocbc_event_mask_START (6)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_ocbc_event_mask_END (6)
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
        unsigned int ocbc_event_mask : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_SCTRL_SCOCBC_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCOCBC_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCOCBC_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCOCBC_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCOCBC_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCOCBC_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCOCBC_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCOCBC_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCOCBC_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCOCBC_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCOCBC_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCOCBC_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCOCBC_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCOCBC_EVENT_MASK_ocbc_event_mask_START (6)
#define SOC_SCTRL_SCOCBC_EVENT_MASK_ocbc_event_mask_END (6)
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
        unsigned int iomcu_dma_mst_nopendingtrans_bp : 1;
        unsigned int iomcu_axi_mst_nopendingtrans_bp : 1;
        unsigned int iomcu_axi_ahb_nopendingtrans_bp : 1;
        unsigned int m7_sc_stat : 9;
        unsigned int reserved_0 : 1;
        unsigned int iomcu_axi_mst_nopendingtrans : 1;
        unsigned int iomcu_service_nopendingtrans : 1;
        unsigned int iomcu_axi_ahb_nopendingtrans : 1;
        unsigned int sc_noc_ocb_mst_mainnopendingtrans : 1;
        unsigned int reserved_1 : 1;
        unsigned int iomcu_stat : 10;
    } reg;
} SOC_SCTRL_SCIOMCUSTAT_UNION;
#endif
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_sysresetreq_START (0)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_sysresetreq_END (0)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_m7_state_START (1)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_m7_state_END (3)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_dma_mst_nopendingtrans_bp_START (4)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_dma_mst_nopendingtrans_bp_END (4)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_axi_mst_nopendingtrans_bp_START (5)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_axi_mst_nopendingtrans_bp_END (5)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_axi_ahb_nopendingtrans_bp_START (6)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_axi_ahb_nopendingtrans_bp_END (6)
#define SOC_SCTRL_SCIOMCUSTAT_m7_sc_stat_START (7)
#define SOC_SCTRL_SCIOMCUSTAT_m7_sc_stat_END (15)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_axi_mst_nopendingtrans_START (17)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_axi_mst_nopendingtrans_END (17)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_service_nopendingtrans_START (18)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_service_nopendingtrans_END (18)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_axi_ahb_nopendingtrans_START (19)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_axi_ahb_nopendingtrans_END (19)
#define SOC_SCTRL_SCIOMCUSTAT_sc_noc_ocb_mst_mainnopendingtrans_START (20)
#define SOC_SCTRL_SCIOMCUSTAT_sc_noc_ocb_mst_mainnopendingtrans_END (20)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_stat_START (22)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_stat_END (31)
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
        unsigned int modem_key0 : 32;
    } reg;
} SOC_SCTRL_SCMODEM_KEY0_UNION;
#endif
#define SOC_SCTRL_SCMODEM_KEY0_modem_key0_START (0)
#define SOC_SCTRL_SCMODEM_KEY0_modem_key0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_key1 : 32;
    } reg;
} SOC_SCTRL_SCMODEM_KEY1_UNION;
#endif
#define SOC_SCTRL_SCMODEM_KEY1_modem_key1_START (0)
#define SOC_SCTRL_SCMODEM_KEY1_modem_key1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_key2 : 32;
    } reg;
} SOC_SCTRL_SCMODEM_KEY2_UNION;
#endif
#define SOC_SCTRL_SCMODEM_KEY2_modem_key2_START (0)
#define SOC_SCTRL_SCMODEM_KEY2_modem_key2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_key3 : 32;
    } reg;
} SOC_SCTRL_SCMODEM_KEY3_UNION;
#endif
#define SOC_SCTRL_SCMODEM_KEY3_modem_key3_START (0)
#define SOC_SCTRL_SCMODEM_KEY3_modem_key3_END (31)
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
        unsigned int gt_clk_secs : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCEFUSESEL_UNION;
#endif
#define SOC_SCTRL_SCEFUSESEL_gt_clk_secs_START (0)
#define SOC_SCTRL_SCEFUSESEL_gt_clk_secs_END (0)
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
        unsigned int a53_cryptodisable : 4;
        unsigned int a57_cryptodisable : 4;
        unsigned int reserved : 24;
    } reg;
} SOC_SCTRL_SCCPUSECCTRL_UNION;
#endif
#define SOC_SCTRL_SCCPUSECCTRL_a53_cryptodisable_START (0)
#define SOC_SCTRL_SCCPUSECCTRL_a53_cryptodisable_END (3)
#define SOC_SCTRL_SCCPUSECCTRL_a57_cryptodisable_START (4)
#define SOC_SCTRL_SCCPUSECCTRL_a57_cryptodisable_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int jtagsd_sw_sel : 8;
        unsigned int auth_pass : 1;
        unsigned int arm_dbgen_wr1 : 1;
        unsigned int reserved : 20;
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
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 3;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 3;
        unsigned int reserved_4: 2;
        unsigned int reserved_5: 2;
        unsigned int reserved_6: 2;
        unsigned int reserved_7: 18;
    } reg;
} SOC_SCTRL_SC_PERI_SEC_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocbc_pa_fama_msb : 7;
        unsigned int reserved : 9;
        unsigned int scocbcfamactrl_msk : 16;
    } reg;
} SOC_SCTRL_SCOCBCFAMACTRL_UNION;
#endif
#define SOC_SCTRL_SCOCBCFAMACTRL_ocbc_pa_fama_msb_START (0)
#define SOC_SCTRL_SCOCBCFAMACTRL_ocbc_pa_fama_msb_END (6)
#define SOC_SCTRL_SCOCBCFAMACTRL_scocbcfamactrl_msk_START (16)
#define SOC_SCTRL_SCOCBCFAMACTRL_scocbcfamactrl_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_fama_align_mode : 1;
        unsigned int media_fama_deft_gran : 3;
        unsigned int reserved_0 : 1;
        unsigned int media_fama_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int media_fama_perf_pwr_sel : 1;
        unsigned int reserved_2 : 24;
    } reg;
} SOC_SCTRL_SCALIGNMODE_UNION;
#endif
#define SOC_SCTRL_SCALIGNMODE_sc_fama_align_mode_START (0)
#define SOC_SCTRL_SCALIGNMODE_sc_fama_align_mode_END (0)
#define SOC_SCTRL_SCALIGNMODE_media_fama_deft_gran_START (1)
#define SOC_SCTRL_SCALIGNMODE_media_fama_deft_gran_END (3)
#define SOC_SCTRL_SCALIGNMODE_media_fama_en_START (5)
#define SOC_SCTRL_SCALIGNMODE_media_fama_en_END (5)
#define SOC_SCTRL_SCALIGNMODE_media_fama_perf_pwr_sel_START (7)
#define SOC_SCTRL_SCALIGNMODE_media_fama_perf_pwr_sel_END (7)
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
        unsigned int gt_pclk_timer1 : 1;
        unsigned int gt_clk_timer1 : 1;
        unsigned int gt_clk_monitor_tcxo : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_efusec : 1;
        unsigned int reserved_1 : 2;
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
        unsigned int reserved_3 : 11;
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
        unsigned int reserved_1 : 2;
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
        unsigned int reserved_3 : 11;
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
        unsigned int reserved_1 : 2;
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
        unsigned int reserved_3 : 11;
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
        unsigned int ip_rst_ao_cc : 1;
        unsigned int ip_prst_ao_gpio1_se : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_SCTRL_SCPERRSTEN0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_timer1_START (0)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_timer1_END (0)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_ao_ipc_START (2)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_ao_ipc_END (2)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_rst_ao_cc_START (3)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_rst_ao_cc_END (3)
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
        unsigned int ip_rst_ao_cc : 1;
        unsigned int ip_prst_ao_gpio1_se : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_SCTRL_SCPERRSTDIS0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_timer1_START (0)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_timer1_END (0)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_ao_ipc_START (2)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_ao_ipc_END (2)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_rst_ao_cc_START (3)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_rst_ao_cc_END (3)
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
        unsigned int ip_rst_ao_cc : 1;
        unsigned int ip_prst_ao_gpio1_se : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_timer1_START (0)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_timer1_END (0)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_ao_ipc_START (2)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_ao_ipc_END (2)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_rst_ao_cc_START (3)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_rst_ao_cc_END (3)
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
        unsigned int reserved : 30;
    } reg;
} SOC_SCTRL_SCPERRSTEN1_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_subsys_START (0)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_subsys_END (0)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_subsys_crg_START (1)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_subsys_crg_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_asp_subsys : 1;
        unsigned int ip_rst_asp_subsys_crg : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_SCTRL_SCPERRSTDIS1_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_subsys_START (0)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_subsys_END (0)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_subsys_crg_START (1)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_subsys_crg_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_asp_subsys : 1;
        unsigned int ip_rst_asp_subsys_crg : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT1_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_subsys_START (0)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_subsys_END (0)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_subsys_crg_START (1)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_subsys_crg_END (1)
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
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 28;
    } reg;
} SOC_SCTRL_SCPEREN3_SEC_UNION;
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
        unsigned int reserved_4: 28;
    } reg;
} SOC_SCTRL_SCPERDIS3_SEC_UNION;
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
        unsigned int reserved_4: 28;
    } reg;
} SOC_SCTRL_SCPERCLKEN3_SEC_UNION;
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
        unsigned int ocbc_bus_soft_rst_req : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCOCBCRSTCTRL_UNION;
#endif
#define SOC_SCTRL_SCOCBCRSTCTRL_ocbc_bus_soft_rst_req_START (0)
#define SOC_SCTRL_SCOCBCRSTCTRL_ocbc_bus_soft_rst_req_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocbc_swm_fll_en_req : 1;
        unsigned int ocbc_swm_pll0_gt_req : 1;
        unsigned int ocbc_swm_pll0_bypass : 1;
        unsigned int ocbc_swm_pll0_en_req : 1;
        unsigned int ocbc_swm_clk_sw : 4;
        unsigned int ocbc_swm_tcxo_gt : 1;
        unsigned int ocbc_swm_fll_gt : 1;
        unsigned int ocbc_swm_pll0_gt : 1;
        unsigned int reserved : 5;
        unsigned int aoclk_ctrl_mask : 16;
    } reg;
} SOC_SCTRL_OCBC_SW_AOCLK_CTRL_UNION;
#endif
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ocbc_swm_fll_en_req_START (0)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ocbc_swm_fll_en_req_END (0)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ocbc_swm_pll0_gt_req_START (1)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ocbc_swm_pll0_gt_req_END (1)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ocbc_swm_pll0_bypass_START (2)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ocbc_swm_pll0_bypass_END (2)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ocbc_swm_pll0_en_req_START (3)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ocbc_swm_pll0_en_req_END (3)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ocbc_swm_clk_sw_START (4)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ocbc_swm_clk_sw_END (7)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ocbc_swm_tcxo_gt_START (8)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ocbc_swm_tcxo_gt_END (8)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ocbc_swm_fll_gt_START (9)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ocbc_swm_fll_gt_END (9)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ocbc_swm_pll0_gt_START (10)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_ocbc_swm_pll0_gt_END (10)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_aoclk_ctrl_mask_START (16)
#define SOC_SCTRL_OCBC_SW_AOCLK_CTRL_aoclk_ctrl_mask_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int clk_sw_sel : 1;
        unsigned int refclk_req_ocbc : 1;
        unsigned int reserved_0 : 1;
        unsigned int aoclk_hw_mode_en : 1;
        unsigned int switch_cycle : 4;
        unsigned int fpll_stable_cycle : 6;
        unsigned int reserved_1 : 2;
        unsigned int hwclk_mode_mask : 16;
    } reg;
} SOC_SCTRL_OCBC_HWCLK_MODE_CTRL_UNION;
#endif
#define SOC_SCTRL_OCBC_HWCLK_MODE_CTRL_clk_sw_sel_START (0)
#define SOC_SCTRL_OCBC_HWCLK_MODE_CTRL_clk_sw_sel_END (0)
#define SOC_SCTRL_OCBC_HWCLK_MODE_CTRL_refclk_req_ocbc_START (1)
#define SOC_SCTRL_OCBC_HWCLK_MODE_CTRL_refclk_req_ocbc_END (1)
#define SOC_SCTRL_OCBC_HWCLK_MODE_CTRL_aoclk_hw_mode_en_START (3)
#define SOC_SCTRL_OCBC_HWCLK_MODE_CTRL_aoclk_hw_mode_en_END (3)
#define SOC_SCTRL_OCBC_HWCLK_MODE_CTRL_switch_cycle_START (4)
#define SOC_SCTRL_OCBC_HWCLK_MODE_CTRL_switch_cycle_END (7)
#define SOC_SCTRL_OCBC_HWCLK_MODE_CTRL_fpll_stable_cycle_START (8)
#define SOC_SCTRL_OCBC_HWCLK_MODE_CTRL_fpll_stable_cycle_END (13)
#define SOC_SCTRL_OCBC_HWCLK_MODE_CTRL_hwclk_mode_mask_START (16)
#define SOC_SCTRL_OCBC_HWCLK_MODE_CTRL_hwclk_mode_mask_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocbc_aoclk_ctrl : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_SCTRL_OCBC_AOCLK_CTRL_STAT_UNION;
#endif
#define SOC_SCTRL_OCBC_AOCLK_CTRL_STAT_ocbc_aoclk_ctrl_START (0)
#define SOC_SCTRL_OCBC_AOCLK_CTRL_STAT_ocbc_aoclk_ctrl_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cur_aoclk_st : 6;
        unsigned int reserved_0 : 2;
        unsigned int ocbc_idle : 1;
        unsigned int lp_cond_sync : 1;
        unsigned int reserved_1 : 6;
        unsigned int cyc_cnt : 6;
        unsigned int reserved_2 : 10;
    } reg;
} SOC_SCTRL_OCBC_AOCLK_DEBUG_UNION;
#endif
#define SOC_SCTRL_OCBC_AOCLK_DEBUG_cur_aoclk_st_START (0)
#define SOC_SCTRL_OCBC_AOCLK_DEBUG_cur_aoclk_st_END (5)
#define SOC_SCTRL_OCBC_AOCLK_DEBUG_ocbc_idle_START (8)
#define SOC_SCTRL_OCBC_AOCLK_DEBUG_ocbc_idle_END (8)
#define SOC_SCTRL_OCBC_AOCLK_DEBUG_lp_cond_sync_START (9)
#define SOC_SCTRL_OCBC_AOCLK_DEBUG_lp_cond_sync_END (9)
#define SOC_SCTRL_OCBC_AOCLK_DEBUG_cyc_cnt_START (16)
#define SOC_SCTRL_OCBC_AOCLK_DEBUG_cyc_cnt_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocbc_hwm_fll_en_req : 1;
        unsigned int ocbc_hwm_pll0_gt : 1;
        unsigned int ocbc_hwm_pll0_bypass : 1;
        unsigned int ocbc_hwm_pll0_en_req : 1;
        unsigned int reserved_0 : 4;
        unsigned int ocbc_hwm_ao_tcxo_gt : 1;
        unsigned int ocbc_hwm_ao_fll_gt : 1;
        unsigned int ocbc_hwm_ao_pll0_gt : 1;
        unsigned int reserved_1 : 5;
        unsigned int reserved_2 : 16;
    } reg;
} SOC_SCTRL_OCBC_HWM_AOCLK_CTRL_UNION;
#endif
#define SOC_SCTRL_OCBC_HWM_AOCLK_CTRL_ocbc_hwm_fll_en_req_START (0)
#define SOC_SCTRL_OCBC_HWM_AOCLK_CTRL_ocbc_hwm_fll_en_req_END (0)
#define SOC_SCTRL_OCBC_HWM_AOCLK_CTRL_ocbc_hwm_pll0_gt_START (1)
#define SOC_SCTRL_OCBC_HWM_AOCLK_CTRL_ocbc_hwm_pll0_gt_END (1)
#define SOC_SCTRL_OCBC_HWM_AOCLK_CTRL_ocbc_hwm_pll0_bypass_START (2)
#define SOC_SCTRL_OCBC_HWM_AOCLK_CTRL_ocbc_hwm_pll0_bypass_END (2)
#define SOC_SCTRL_OCBC_HWM_AOCLK_CTRL_ocbc_hwm_pll0_en_req_START (3)
#define SOC_SCTRL_OCBC_HWM_AOCLK_CTRL_ocbc_hwm_pll0_en_req_END (3)
#define SOC_SCTRL_OCBC_HWM_AOCLK_CTRL_ocbc_hwm_ao_tcxo_gt_START (8)
#define SOC_SCTRL_OCBC_HWM_AOCLK_CTRL_ocbc_hwm_ao_tcxo_gt_END (8)
#define SOC_SCTRL_OCBC_HWM_AOCLK_CTRL_ocbc_hwm_ao_fll_gt_START (9)
#define SOC_SCTRL_OCBC_HWM_AOCLK_CTRL_ocbc_hwm_ao_fll_gt_END (9)
#define SOC_SCTRL_OCBC_HWM_AOCLK_CTRL_ocbc_hwm_ao_pll0_gt_START (10)
#define SOC_SCTRL_OCBC_HWM_AOCLK_CTRL_ocbc_hwm_ao_pll0_gt_END (10)
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
