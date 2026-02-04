#ifndef __SOC_SCTRL_INTERFACE_H__
#define __SOC_SCTRL_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_SCTRL_SCCTRL_ADDR(base) ((base) + (0x000))
#define SOC_SCTRL_SCSYSSTAT_ADDR(base) ((base) + (0x004))
#define SOC_SCTRL_SCXTALCTRL_ADDR(base) ((base) + (0x010))
#define SOC_SCTRL_SCXTALTIMEOUT0_ADDR(base) ((base) + (0x014))
#define SOC_SCTRL_SCXTALTIMEOUT1_ADDR(base) ((base) + (0x018))
#define SOC_SCTRL_SCXTALSTAT_ADDR(base) ((base) + (0x01C))
#define SOC_SCTRL_SCPPLLCTRL0_ADDR(base) ((base) + (0x020))
#define SOC_SCTRL_SCPPLLCTRL1_ADDR(base) ((base) + (0x024))
#define SOC_SCTRL_SCPPLLSSCCTRL_ADDR(base) ((base) + (0x028))
#define SOC_SCTRL_SCPPLLSTAT_ADDR(base) ((base) + (0x02C))
#define SOC_SCTRL_SCPEREN0_ADDR(base) ((base) + (0x030))
#define SOC_SCTRL_SCPERDIS0_ADDR(base) ((base) + (0x034))
#define SOC_SCTRL_SCPERCLKEN0_ADDR(base) ((base) + (0x038))
#define SOC_SCTRL_SCPERSTAT0_ADDR(base) ((base) + (0x03C))
#define SOC_SCTRL_SCPEREN1_ADDR(base) ((base) + (0x040))
#define SOC_SCTRL_SCPERDIS1_ADDR(base) ((base) + (0x044))
#define SOC_SCTRL_SCPERCLKEN1_ADDR(base) ((base) + (0x048))
#define SOC_SCTRL_SCPERSTAT1_ADDR(base) ((base) + (0x04C))
#define SOC_SCTRL_SCPEREN2_ADDR(base) ((base) + (0x050))
#define SOC_SCTRL_SCPERDIS2_ADDR(base) ((base) + (0x054))
#define SOC_SCTRL_SCPERCLKEN2_ADDR(base) ((base) + (0x058))
#define SOC_SCTRL_SCPERSTAT2_ADDR(base) ((base) + (0x05C))
#define SOC_SCTRL_SCTIMERCTRL_ADDR(base) ((base) + (0x060))
#define SOC_SCTRL_SCFPLLCTRL0_ADDR(base) ((base) + (0x064))
#define SOC_SCTRL_SCFPLLCTRL1_ADDR(base) ((base) + (0x068))
#define SOC_SCTRL_SCFPLLCTRL2_ADDR(base) ((base) + (0x06C))
#define SOC_SCTRL_SCFPLLCTRL3_ADDR(base) ((base) + (0x070))
#define SOC_SCTRL_SCFPLLSTAT_ADDR(base) ((base) + (0x074))
#define SOC_SCTRL_SCPERSTAT3_ADDR(base) ((base) + (0x07C))
#define SOC_SCTRL_SCPERRSTEN0_ADDR(base) ((base) + (0x080))
#define SOC_SCTRL_SCPERRSTDIS0_ADDR(base) ((base) + (0x084))
#define SOC_SCTRL_SCPERRSTSTAT0_ADDR(base) ((base) + (0x088))
#define SOC_SCTRL_SCPERRSTEN1_ADDR(base) ((base) + (0x08C))
#define SOC_SCTRL_SCPERRSTDIS1_ADDR(base) ((base) + (0x090))
#define SOC_SCTRL_SCPERRSTSTAT1_ADDR(base) ((base) + (0x094))
#define SOC_SCTRL_SCPERRSTEN2_ADDR(base) ((base) + (0x098))
#define SOC_SCTRL_SCPERRSTDIS2_ADDR(base) ((base) + (0x09C))
#define SOC_SCTRL_SCPERRSTSTAT2_ADDR(base) ((base) + (0x0A0))
#define SOC_SCTRL_SCIPCLKRSTBUS_ADDR(base) ((base) + (0x0A4))
#define SOC_SCTRL_SCISOEN_ADDR(base) ((base) + (0x0C0))
#define SOC_SCTRL_SCISODIS_ADDR(base) ((base) + (0x0C4))
#define SOC_SCTRL_SCISOSTAT_ADDR(base) ((base) + (0x0C8))
#define SOC_SCTRL_SCPWREN_ADDR(base) ((base) + (0x0D0))
#define SOC_SCTRL_SCPWRDIS_ADDR(base) ((base) + (0x0D4))
#define SOC_SCTRL_SCPWRSTAT_ADDR(base) ((base) + (0x0D8))
#define SOC_SCTRL_SCPWRACK_ADDR(base) ((base) + (0x0DC))
#define SOC_SCTRL_SCPERPWRDOWNTIME_ADDR(base) ((base) + (0x0E0))
#define SOC_SCTRL_SCPERPWRUPTIME_ADDR(base) ((base) + (0x0E4))
#define SOC_SCTRL_SCCLKDIV0_ADDR(base) ((base) + (0x100))
#define SOC_SCTRL_SCCLKDIV1_ADDR(base) ((base) + (0x104))
#define SOC_SCTRL_SCCLKDIV2_ADDR(base) ((base) + (0x108))
#define SOC_SCTRL_SCCLKDIV3_ADDR(base) ((base) + (0x10C))
#define SOC_SCTRL_SCCLKDIV4_ADDR(base) ((base) + (0x110))
#define SOC_SCTRL_SCCLKDIV5_ADDR(base) ((base) + (0x114))
#define SOC_SCTRL_SCPERCTRL0_ADDR(base) ((base) + (0x200))
#define SOC_SCTRL_SCPERCTRL1_ADDR(base) ((base) + (0x204))
#define SOC_SCTRL_SCPERCTRL2_ADDR(base) ((base) + (0x208))
#define SOC_SCTRL_SCPERCTRL3_ADDR(base) ((base) + (0x20C))
#define SOC_SCTRL_SCPERCTRL4_ADDR(base) ((base) + (0x210))
#define SOC_SCTRL_SCPERCTRL5_ADDR(base) ((base) + (0x214))
#define SOC_SCTRL_SCPERCTRL6_ADDR(base) ((base) + (0x218))
#define SOC_SCTRL_SCPERSTATUS0_ADDR(base) ((base) + (0x21C))
#define SOC_SCTRL_SCPERSTATUS1_ADDR(base) ((base) + (0x220))
#define SOC_SCTRL_SCPERSTATUS2_ADDR(base) ((base) + (0x224))
#define SOC_SCTRL_SCPERSTATUS3_ADDR(base) ((base) + (0x228))
#define SOC_SCTRL_SCPERSTATUS4_ADDR(base) ((base) + (0x22C))
#define SOC_SCTRL_SCPERSTATUS5_ADDR(base) ((base) + (0x230))
#define SOC_SCTRL_SCPERSTATUS6_ADDR(base) ((base) + (0x234))
#define SOC_SCTRL_SCPERSTATUS7_ADDR(base) ((base) + (0x238))
#define SOC_SCTRL_SCPERSTATUS8_ADDR(base) ((base) + (0x23C))
#define SOC_SCTRL_SCPERSTATUS9_ADDR(base) ((base) + (0x240))
#define SOC_SCTRL_SCPERSTATUS10_ADDR(base) ((base) + (0x244))
#define SOC_SCTRL_SCPERSTATUS11_ADDR(base) ((base) + (0x248))
#define SOC_SCTRL_SCPERSTATUS12_ADDR(base) ((base) + (0x24C))
#define SOC_SCTRL_SCPERSTATUS13_ADDR(base) ((base) + (0x250))
#define SOC_SCTRL_SCPERSTATUS14_ADDR(base) ((base) + (0x254))
#define SOC_SCTRL_SCPERCTRL7_ADDR(base) ((base) + (0x270))
#define SOC_SCTRL_SCINNERSTAT_ADDR(base) ((base) + (0x280))
#define SOC_SCTRL_SCDEEPSLEEPED_ADDR(base) ((base) + (0x300))
#define SOC_SCTRL_SCMRBBUSYSTAT_ADDR(base) ((base) + (0x304))
#define SOC_SCTRL_SCSWADDR_ADDR(base) ((base) + (0x308))
#define SOC_SCTRL_SCDDRADDR_ADDR(base) ((base) + (0x30C))
#define SOC_SCTRL_SCDDRDATA_ADDR(base) ((base) + (0x310))
#define SOC_SCTRL_SCBAKDATA0_ADDR(base) ((base) + (0x314))
#define SOC_SCTRL_SCBAKDATA1_ADDR(base) ((base) + (0x318))
#define SOC_SCTRL_SCBAKDATA2_ADDR(base) ((base) + (0x31C))
#define SOC_SCTRL_SCBAKDATA3_ADDR(base) ((base) + (0x320))
#define SOC_SCTRL_SCBAKDATA4_ADDR(base) ((base) + (0x324))
#define SOC_SCTRL_SCBAKDATA5_ADDR(base) ((base) + (0x328))
#define SOC_SCTRL_SCBAKDATA6_ADDR(base) ((base) + (0x32C))
#define SOC_SCTRL_SCBAKDATA7_ADDR(base) ((base) + (0x330))
#define SOC_SCTRL_SCBAKDATA8_ADDR(base) ((base) + (0x334))
#define SOC_SCTRL_SCBAKDATA9_ADDR(base) ((base) + (0x338))
#define SOC_SCTRL_SCBAKDATA10_ADDR(base) ((base) + (0x33C))
#define SOC_SCTRL_SCBAKDATA11_ADDR(base) ((base) + (0x340))
#define SOC_SCTRL_SCBAKDATA12_ADDR(base) ((base) + (0x344))
#define SOC_SCTRL_SCBAKDATA13_ADDR(base) ((base) + (0x348))
#define SOC_SCTRL_SCBAKDATA14_ADDR(base) ((base) + (0x34C))
#define SOC_SCTRL_SCBAKDATA15_ADDR(base) ((base) + (0x350))
#define SOC_SCTRL_SCINT_GATHER_STAT_ADDR(base) ((base) + (0x450))
#define SOC_SCTRL_SCINT_MASK_ADDR(base) ((base) + (0x454))
#define SOC_SCTRL_SCINT_STAT_ADDR(base) ((base) + (0x458))
#define SOC_SCTRL_SCDRX_INT_CFG_ADDR(base) ((base) + (0x45C))
#define SOC_SCTRL_SCLPMCUWFI_INT_ADDR(base) ((base) + (0x460))
#define SOC_SCTRL_SCINT_MASK1_ADDR(base) ((base) + (0x464))
#define SOC_SCTRL_SCINT_STAT1_ADDR(base) ((base) + (0x468))
#define SOC_SCTRL_SCINT_MASK2_ADDR(base) ((base) + (0x46C))
#define SOC_SCTRL_SCINT_STAT2_ADDR(base) ((base) + (0x470))
#define SOC_SCTRL_SCMALIBYPCFG_ADDR(base) ((base) + (0x48C))
#define SOC_SCTRL_SCLPMCUCLKEN_ADDR(base) ((base) + (0x500))
#define SOC_SCTRL_SCLPMCURSTEN_ADDR(base) ((base) + (0x504))
#define SOC_SCTRL_SCLPMCURSTDIS_ADDR(base) ((base) + (0x508))
#define SOC_SCTRL_SCLPMCURSTSTAT_ADDR(base) ((base) + (0x50C))
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
#define SOC_SCTRL_SCEVENT_STAT_ADDR(base) ((base) + (0x564))
#define SOC_SCTRL_SCISPA7_EVENT_MASK_ADDR(base) ((base) + (0x568))
#define SOC_SCTRL_SCIOMCUCLKCTRL_ADDR(base) ((base) + (0x580))
#define SOC_SCTRL_SCIOMCUCLKSTAT_ADDR(base) ((base) + (0x584))
#define SOC_SCTRL_SCIOMCUCLKEN_ADDR(base) ((base) + (0x588))
#define SOC_SCTRL_SCIOMCURSTEN_ADDR(base) ((base) + (0x58C))
#define SOC_SCTRL_SCIOMCURSTDIS_ADDR(base) ((base) + (0x590))
#define SOC_SCTRL_SCIOMCURSTSTAT_ADDR(base) ((base) + (0x594))
#define SOC_SCTRL_SCIOMCUCTRL_ADDR(base) ((base) + (0x598))
#define SOC_SCTRL_SCIOMCUSTAT_ADDR(base) ((base) + (0x59C))
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
#define SOC_SCTRL_SCSECURITYSTAT_ADDR(base) ((base) + (0x844))
#define SOC_SCTRL_SCCHIP_ID0_ADDR(base) ((base) + (0x848))
#define SOC_SCTRL_SCCHIP_ID1_ADDR(base) ((base) + (0x84C))
#define SOC_SCTRL_SCCPUSECCTRL_ADDR(base) ((base) + (0x850))
#define SOC_SCTRL_SCPEREN0_SEC_ADDR(base) ((base) + (0x900))
#define SOC_SCTRL_SCPERDIS0_SEC_ADDR(base) ((base) + (0x904))
#define SOC_SCTRL_SCPERCLKEN0_SEC_ADDR(base) ((base) + (0x908))
#define SOC_SCTRL_SCPERRSTEN0_SEC_ADDR(base) ((base) + (0x950))
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ADDR(base) ((base) + (0x954))
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ADDR(base) ((base) + (0x958))
#define SOC_SCTRL_SCTIMERCTRL_SEC_ADDR(base) ((base) + (0x9A0))
#define SOC_SCTRL_SCPERRSTEN1_SEC_ADDR(base) ((base) + (0xA50))
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ADDR(base) ((base) + (0xA54))
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ADDR(base) ((base) + (0xA58))
#define SOC_SCTRL_SCPERRSTEN2_SEC_ADDR(base) ((base) + (0xB50))
#define SOC_SCTRL_SCPERRSTDIS2_SEC_ADDR(base) ((base) + (0xB54))
#define SOC_SCTRL_SCPERRSTSTAT2_SEC_ADDR(base) ((base) + (0xB58))
#define SOC_SCTRL_SCSOCID0_ADDR(base) ((base) + (0xE00))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modectrl : 3;
        unsigned int modestatus : 4;
        unsigned int reserved_0 : 1;
        unsigned int timeforcehigh : 1;
        unsigned int timeren0sel : 1;
        unsigned int timeren0ov : 1;
        unsigned int timeren1sel : 1;
        unsigned int timeren1ov : 1;
        unsigned int mdmtimer_stop_dbg_bp : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int sc_aonoc_asp_mst_i_mainnopendingtrans_bp : 1;
        unsigned int timeren4sel : 1;
        unsigned int timeren4ov : 1;
        unsigned int timeren5sel : 1;
        unsigned int timeren5ov : 1;
        unsigned int timeren6sel : 1;
        unsigned int timeren6ov : 1;
        unsigned int timeren7sel : 1;
        unsigned int timeren7ov : 1;
        unsigned int deepsleepen : 1;
        unsigned int reserved_3 : 6;
    } reg;
} SOC_SCTRL_SCCTRL_UNION;
#endif
#define SOC_SCTRL_SCCTRL_modectrl_START (0)
#define SOC_SCTRL_SCCTRL_modectrl_END (2)
#define SOC_SCTRL_SCCTRL_modestatus_START (3)
#define SOC_SCTRL_SCCTRL_modestatus_END (6)
#define SOC_SCTRL_SCCTRL_timeforcehigh_START (8)
#define SOC_SCTRL_SCCTRL_timeforcehigh_END (8)
#define SOC_SCTRL_SCCTRL_timeren0sel_START (9)
#define SOC_SCTRL_SCCTRL_timeren0sel_END (9)
#define SOC_SCTRL_SCCTRL_timeren0ov_START (10)
#define SOC_SCTRL_SCCTRL_timeren0ov_END (10)
#define SOC_SCTRL_SCCTRL_timeren1sel_START (11)
#define SOC_SCTRL_SCCTRL_timeren1sel_END (11)
#define SOC_SCTRL_SCCTRL_timeren1ov_START (12)
#define SOC_SCTRL_SCCTRL_timeren1ov_END (12)
#define SOC_SCTRL_SCCTRL_mdmtimer_stop_dbg_bp_START (13)
#define SOC_SCTRL_SCCTRL_mdmtimer_stop_dbg_bp_END (13)
#define SOC_SCTRL_SCCTRL_sc_aonoc_asp_mst_i_mainnopendingtrans_bp_START (16)
#define SOC_SCTRL_SCCTRL_sc_aonoc_asp_mst_i_mainnopendingtrans_bp_END (16)
#define SOC_SCTRL_SCCTRL_timeren4sel_START (17)
#define SOC_SCTRL_SCCTRL_timeren4sel_END (17)
#define SOC_SCTRL_SCCTRL_timeren4ov_START (18)
#define SOC_SCTRL_SCCTRL_timeren4ov_END (18)
#define SOC_SCTRL_SCCTRL_timeren5sel_START (19)
#define SOC_SCTRL_SCCTRL_timeren5sel_END (19)
#define SOC_SCTRL_SCCTRL_timeren5ov_START (20)
#define SOC_SCTRL_SCCTRL_timeren5ov_END (20)
#define SOC_SCTRL_SCCTRL_timeren6sel_START (21)
#define SOC_SCTRL_SCCTRL_timeren6sel_END (21)
#define SOC_SCTRL_SCCTRL_timeren6ov_START (22)
#define SOC_SCTRL_SCCTRL_timeren6ov_END (22)
#define SOC_SCTRL_SCCTRL_timeren7sel_START (23)
#define SOC_SCTRL_SCCTRL_timeren7sel_END (23)
#define SOC_SCTRL_SCCTRL_timeren7ov_START (24)
#define SOC_SCTRL_SCCTRL_timeren7ov_END (24)
#define SOC_SCTRL_SCCTRL_deepsleepen_START (25)
#define SOC_SCTRL_SCCTRL_deepsleepen_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reset_source : 32;
    } reg;
} SOC_SCTRL_SCSYSSTAT_UNION;
#endif
#define SOC_SCTRL_SCSYSSTAT_reset_source_START (0)
#define SOC_SCTRL_SCSYSSTAT_reset_source_END (31)
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
        unsigned int tcxofast_ctrl0 : 1;
        unsigned int tcxofast_ctrl1 : 1;
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
#define SOC_SCTRL_SCXTALCTRL_tcxofast_ctrl0_START (5)
#define SOC_SCTRL_SCXTALCTRL_tcxofast_ctrl0_END (5)
#define SOC_SCTRL_SCXTALCTRL_tcxofast_ctrl1_START (6)
#define SOC_SCTRL_SCXTALCTRL_tcxofast_ctrl1_END (6)
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
        unsigned int tcxo_en0 : 1;
        unsigned int tcxo_en1 : 1;
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
#define SOC_SCTRL_SCXTALSTAT_tcxo_en0_START (0)
#define SOC_SCTRL_SCXTALSTAT_tcxo_en0_END (0)
#define SOC_SCTRL_SCXTALSTAT_tcxo_en1_START (1)
#define SOC_SCTRL_SCXTALSTAT_tcxo_en1_END (1)
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
        unsigned int reserved : 30;
    } reg;
} SOC_SCTRL_SCPPLLSTAT_UNION;
#endif
#define SOC_SCTRL_SCPPLLSTAT_pllstat_START (0)
#define SOC_SCTRL_SCPPLLSTAT_pllstat_END (0)
#define SOC_SCTRL_SCPPLLSTAT_ppll0_en_stat_START (1)
#define SOC_SCTRL_SCPPLLSTAT_ppll0_en_stat_END (1)
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
        unsigned int reserved : 2;
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
        unsigned int gt_clk_memrep : 1;
        unsigned int gt_clk_asp_subsys : 1;
        unsigned int gt_clk_asp_tcxo : 1;
        unsigned int gt_clk_bbpdrx : 1;
        unsigned int gt_clk_aobus : 1;
        unsigned int gt_pclk_efusec : 1;
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
#define SOC_SCTRL_SCPEREN0_gt_clk_memrep_START (25)
#define SOC_SCTRL_SCPEREN0_gt_clk_memrep_END (25)
#define SOC_SCTRL_SCPEREN0_gt_clk_asp_subsys_START (26)
#define SOC_SCTRL_SCPEREN0_gt_clk_asp_subsys_END (26)
#define SOC_SCTRL_SCPEREN0_gt_clk_asp_tcxo_START (27)
#define SOC_SCTRL_SCPEREN0_gt_clk_asp_tcxo_END (27)
#define SOC_SCTRL_SCPEREN0_gt_clk_bbpdrx_START (28)
#define SOC_SCTRL_SCPEREN0_gt_clk_bbpdrx_END (28)
#define SOC_SCTRL_SCPEREN0_gt_clk_aobus_START (29)
#define SOC_SCTRL_SCPEREN0_gt_clk_aobus_END (29)
#define SOC_SCTRL_SCPEREN0_gt_pclk_efusec_START (30)
#define SOC_SCTRL_SCPEREN0_gt_pclk_efusec_END (30)
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
        unsigned int reserved : 2;
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
        unsigned int gt_clk_memrep : 1;
        unsigned int gt_clk_asp_subsys : 1;
        unsigned int gt_clk_asp_tcxo : 1;
        unsigned int gt_clk_bbpdrx : 1;
        unsigned int gt_clk_aobus : 1;
        unsigned int gt_pclk_efusec : 1;
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
#define SOC_SCTRL_SCPERDIS0_gt_clk_memrep_START (25)
#define SOC_SCTRL_SCPERDIS0_gt_clk_memrep_END (25)
#define SOC_SCTRL_SCPERDIS0_gt_clk_asp_subsys_START (26)
#define SOC_SCTRL_SCPERDIS0_gt_clk_asp_subsys_END (26)
#define SOC_SCTRL_SCPERDIS0_gt_clk_asp_tcxo_START (27)
#define SOC_SCTRL_SCPERDIS0_gt_clk_asp_tcxo_END (27)
#define SOC_SCTRL_SCPERDIS0_gt_clk_bbpdrx_START (28)
#define SOC_SCTRL_SCPERDIS0_gt_clk_bbpdrx_END (28)
#define SOC_SCTRL_SCPERDIS0_gt_clk_aobus_START (29)
#define SOC_SCTRL_SCPERDIS0_gt_clk_aobus_END (29)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_efusec_START (30)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_efusec_END (30)
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
        unsigned int reserved : 2;
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
        unsigned int gt_clk_memrep : 1;
        unsigned int gt_clk_asp_subsys : 1;
        unsigned int gt_clk_asp_tcxo : 1;
        unsigned int gt_clk_bbpdrx : 1;
        unsigned int gt_clk_aobus : 1;
        unsigned int gt_pclk_efusec : 1;
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
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_memrep_START (25)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_memrep_END (25)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_asp_subsys_START (26)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_asp_subsys_END (26)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_asp_tcxo_START (27)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_asp_tcxo_END (27)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_bbpdrx_START (28)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_bbpdrx_END (28)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_aobus_START (29)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_aobus_END (29)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_efusec_START (30)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_efusec_END (30)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_ppll0_sscg_START (31)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_ppll0_sscg_END (31)
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
        unsigned int gt_pclk_timer1 : 1;
        unsigned int gt_clk_timer1 : 1;
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
        unsigned int gt_clk_memrep : 1;
        unsigned int gt_clk_asp_subsys : 1;
        unsigned int gt_clk_asp_tcxo : 1;
        unsigned int gt_clk_bbpdrx : 1;
        unsigned int gt_clk_aobus : 1;
        unsigned int gt_pclk_efusec : 1;
        unsigned int gt_clk_ppll0_sscg : 1;
    } reg;
} SOC_SCTRL_SCPERSTAT0_UNION;
#endif
#define SOC_SCTRL_SCPERSTAT0_gt_clk_ref_crc_START (0)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_ref_crc_END (0)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_rtc_START (1)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_rtc_END (1)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_rtc1_START (2)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_rtc1_END (2)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_timer0_START (3)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_timer0_END (3)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_timer0_START (4)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_timer0_END (4)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_timer1_START (5)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_timer1_END (5)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_timer1_START (6)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_timer1_END (6)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_timer2_START (7)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_timer2_END (7)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_timer2_START (8)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_timer2_END (8)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_timer3_START (9)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_timer3_END (9)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_timer3_START (10)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_timer3_END (10)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_ao_gpio0_START (11)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_ao_gpio0_END (11)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_ao_gpio1_START (12)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_ao_gpio1_END (12)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_ao_gpio2_START (13)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_ao_gpio2_END (13)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_ao_gpio3_START (14)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_ao_gpio3_END (14)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_ao_ioc_START (15)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_ao_ioc_END (15)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_out0_START (16)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_out0_END (16)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_out1_START (17)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_out1_END (17)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_jtag_auth_START (18)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_jtag_auth_END (18)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_syscnt_START (19)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_syscnt_END (19)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_syscnt_START (20)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_syscnt_END (20)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_ao_gpio4_START (21)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_ao_gpio4_END (21)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_ao_gpio5_START (22)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_ao_gpio5_END (22)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_sci0_START (23)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_sci0_END (23)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_sci1_START (24)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_sci1_END (24)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_memrep_START (25)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_memrep_END (25)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_asp_subsys_START (26)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_asp_subsys_END (26)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_asp_tcxo_START (27)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_asp_tcxo_END (27)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_bbpdrx_START (28)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_bbpdrx_END (28)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_aobus_START (29)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_aobus_END (29)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_efusec_START (30)
#define SOC_SCTRL_SCPERSTAT0_gt_pclk_efusec_END (30)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_ppll0_sscg_START (31)
#define SOC_SCTRL_SCPERSTAT0_gt_clk_ppll0_sscg_END (31)
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
        unsigned int gt_clk_bbpdrx_oth : 1;
        unsigned int gt_clk_aobus_noc : 1;
        unsigned int gt_clk_noc_aobus2mmbuf : 1;
        unsigned int ddr_loopback_en : 1;
        unsigned int gt_aclk_asc : 1;
        unsigned int reserved_0 : 1;
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
        unsigned int reserved_1 : 1;
        unsigned int gt_aclk_mmbuf : 1;
        unsigned int gt_pclk_mmbuf : 1;
        unsigned int gt_clk_mmbuf_dss : 1;
        unsigned int gt_pclk_mmbufcfg : 1;
        unsigned int gt_clk_autodiv_aobus_noc : 1;
        unsigned int reserved_2 : 3;
        unsigned int aobus2mmbuf_auto_gate_bypass : 1;
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
#define SOC_SCTRL_SCPEREN1_gt_clk_bbpdrx_oth_START (4)
#define SOC_SCTRL_SCPEREN1_gt_clk_bbpdrx_oth_END (4)
#define SOC_SCTRL_SCPEREN1_gt_clk_aobus_noc_START (5)
#define SOC_SCTRL_SCPEREN1_gt_clk_aobus_noc_END (5)
#define SOC_SCTRL_SCPEREN1_gt_clk_noc_aobus2mmbuf_START (6)
#define SOC_SCTRL_SCPEREN1_gt_clk_noc_aobus2mmbuf_END (6)
#define SOC_SCTRL_SCPEREN1_ddr_loopback_en_START (7)
#define SOC_SCTRL_SCPEREN1_ddr_loopback_en_END (7)
#define SOC_SCTRL_SCPEREN1_gt_aclk_asc_START (8)
#define SOC_SCTRL_SCPEREN1_gt_aclk_asc_END (8)
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
#define SOC_SCTRL_SCPEREN1_gt_aclk_mmbuf_START (22)
#define SOC_SCTRL_SCPEREN1_gt_aclk_mmbuf_END (22)
#define SOC_SCTRL_SCPEREN1_gt_pclk_mmbuf_START (23)
#define SOC_SCTRL_SCPEREN1_gt_pclk_mmbuf_END (23)
#define SOC_SCTRL_SCPEREN1_gt_clk_mmbuf_dss_START (24)
#define SOC_SCTRL_SCPEREN1_gt_clk_mmbuf_dss_END (24)
#define SOC_SCTRL_SCPEREN1_gt_pclk_mmbufcfg_START (25)
#define SOC_SCTRL_SCPEREN1_gt_pclk_mmbufcfg_END (25)
#define SOC_SCTRL_SCPEREN1_gt_clk_autodiv_aobus_noc_START (26)
#define SOC_SCTRL_SCPEREN1_gt_clk_autodiv_aobus_noc_END (26)
#define SOC_SCTRL_SCPEREN1_aobus2mmbuf_auto_gate_bypass_START (30)
#define SOC_SCTRL_SCPEREN1_aobus2mmbuf_auto_gate_bypass_END (30)
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
        unsigned int gt_clk_bbpdrx_oth : 1;
        unsigned int gt_clk_aobus_noc : 1;
        unsigned int gt_clk_noc_aobus2mmbuf : 1;
        unsigned int ddr_loopback_en : 1;
        unsigned int gt_aclk_asc : 1;
        unsigned int reserved_0 : 1;
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
        unsigned int reserved_1 : 1;
        unsigned int gt_aclk_mmbuf : 1;
        unsigned int gt_pclk_mmbuf : 1;
        unsigned int gt_clk_mmbuf_dss : 1;
        unsigned int gt_pclk_mmbufcfg : 1;
        unsigned int gt_clk_autodiv_aobus_noc : 1;
        unsigned int reserved_2 : 3;
        unsigned int aobus2mmbuf_auto_gate_bypass : 1;
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
#define SOC_SCTRL_SCPERDIS1_gt_clk_bbpdrx_oth_START (4)
#define SOC_SCTRL_SCPERDIS1_gt_clk_bbpdrx_oth_END (4)
#define SOC_SCTRL_SCPERDIS1_gt_clk_aobus_noc_START (5)
#define SOC_SCTRL_SCPERDIS1_gt_clk_aobus_noc_END (5)
#define SOC_SCTRL_SCPERDIS1_gt_clk_noc_aobus2mmbuf_START (6)
#define SOC_SCTRL_SCPERDIS1_gt_clk_noc_aobus2mmbuf_END (6)
#define SOC_SCTRL_SCPERDIS1_ddr_loopback_en_START (7)
#define SOC_SCTRL_SCPERDIS1_ddr_loopback_en_END (7)
#define SOC_SCTRL_SCPERDIS1_gt_aclk_asc_START (8)
#define SOC_SCTRL_SCPERDIS1_gt_aclk_asc_END (8)
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
#define SOC_SCTRL_SCPERDIS1_gt_aclk_mmbuf_START (22)
#define SOC_SCTRL_SCPERDIS1_gt_aclk_mmbuf_END (22)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_mmbuf_START (23)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_mmbuf_END (23)
#define SOC_SCTRL_SCPERDIS1_gt_clk_mmbuf_dss_START (24)
#define SOC_SCTRL_SCPERDIS1_gt_clk_mmbuf_dss_END (24)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_mmbufcfg_START (25)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_mmbufcfg_END (25)
#define SOC_SCTRL_SCPERDIS1_gt_clk_autodiv_aobus_noc_START (26)
#define SOC_SCTRL_SCPERDIS1_gt_clk_autodiv_aobus_noc_END (26)
#define SOC_SCTRL_SCPERDIS1_aobus2mmbuf_auto_gate_bypass_START (30)
#define SOC_SCTRL_SCPERDIS1_aobus2mmbuf_auto_gate_bypass_END (30)
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
        unsigned int gt_clk_bbpdrx_oth : 1;
        unsigned int gt_clk_aobus_noc : 1;
        unsigned int gt_clk_noc_aobus2mmbuf : 1;
        unsigned int ddr_loopback_en : 1;
        unsigned int gt_aclk_asc : 1;
        unsigned int reserved_0 : 1;
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
        unsigned int reserved_1 : 1;
        unsigned int gt_aclk_mmbuf : 1;
        unsigned int gt_pclk_mmbuf : 1;
        unsigned int gt_clk_mmbuf_dss : 1;
        unsigned int gt_pclk_mmbufcfg : 1;
        unsigned int gt_clk_autodiv_aobus_noc : 1;
        unsigned int reserved_2 : 3;
        unsigned int aobus2mmbuf_auto_gate_bypass : 1;
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
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_bbpdrx_oth_START (4)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_bbpdrx_oth_END (4)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_aobus_noc_START (5)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_aobus_noc_END (5)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_noc_aobus2mmbuf_START (6)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_noc_aobus2mmbuf_END (6)
#define SOC_SCTRL_SCPERCLKEN1_ddr_loopback_en_START (7)
#define SOC_SCTRL_SCPERCLKEN1_ddr_loopback_en_END (7)
#define SOC_SCTRL_SCPERCLKEN1_gt_aclk_asc_START (8)
#define SOC_SCTRL_SCPERCLKEN1_gt_aclk_asc_END (8)
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
#define SOC_SCTRL_SCPERCLKEN1_gt_aclk_mmbuf_START (22)
#define SOC_SCTRL_SCPERCLKEN1_gt_aclk_mmbuf_END (22)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_mmbuf_START (23)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_mmbuf_END (23)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_mmbuf_dss_START (24)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_mmbuf_dss_END (24)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_mmbufcfg_START (25)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_mmbufcfg_END (25)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_autodiv_aobus_noc_START (26)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_autodiv_aobus_noc_END (26)
#define SOC_SCTRL_SCPERCLKEN1_aobus2mmbuf_auto_gate_bypass_START (30)
#define SOC_SCTRL_SCPERCLKEN1_aobus2mmbuf_auto_gate_bypass_END (30)
#define SOC_SCTRL_SCPERCLKEN1_wait_ddr_selfreflash_done_bypass_START (31)
#define SOC_SCTRL_SCPERCLKEN1_wait_ddr_selfreflash_done_bypass_END (31)
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
        unsigned int gt_clk_104m_bbpdrx : 1;
        unsigned int st_clk_noc_aobus2sysbusddrc : 1;
        unsigned int st_clk_noc_aobus2mmbuf : 1;
        unsigned int reserved_0 : 1;
        unsigned int st_aclk_asc : 1;
        unsigned int reserved_1 : 1;
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
        unsigned int gt_clk_monitor : 1;
        unsigned int st_aclk_mmbuf : 1;
        unsigned int st_pclk_mmbuf : 1;
        unsigned int st_aclk_mmbuf_dss : 1;
        unsigned int gt_pclk_mmbufcfg : 1;
        unsigned int reserved_2 : 1;
        unsigned int swdone_clk_asp_subsys_div : 1;
        unsigned int swdone_clk_sci_div : 1;
        unsigned int swdone_clk_aobus_div : 1;
        unsigned int aobusclksw2sys : 1;
        unsigned int aobusclksw2pll : 1;
    } reg;
} SOC_SCTRL_SCPERSTAT1_UNION;
#endif
#define SOC_SCTRL_SCPERSTAT1_gt_clk_asp_dw_axi_START (0)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_asp_dw_axi_END (0)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_asp_x2h_START (1)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_asp_x2h_END (1)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_asp_h2p_START (2)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_asp_h2p_END (2)
#define SOC_SCTRL_SCPERSTAT1_gt_asp_cfg_START (3)
#define SOC_SCTRL_SCPERSTAT1_gt_asp_cfg_END (3)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_104m_bbpdrx_START (4)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_104m_bbpdrx_END (4)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2sysbusddrc_START (5)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2sysbusddrc_END (5)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2mmbuf_START (6)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2mmbuf_END (6)
#define SOC_SCTRL_SCPERSTAT1_st_aclk_asc_START (8)
#define SOC_SCTRL_SCPERSTAT1_st_aclk_asc_END (8)
#define SOC_SCTRL_SCPERSTAT1_gt_pclk_timer4_START (10)
#define SOC_SCTRL_SCPERSTAT1_gt_pclk_timer4_END (10)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_timer4_START (11)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_timer4_END (11)
#define SOC_SCTRL_SCPERSTAT1_gt_pclk_timer5_START (12)
#define SOC_SCTRL_SCPERSTAT1_gt_pclk_timer5_END (12)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_timer5_START (13)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_timer5_END (13)
#define SOC_SCTRL_SCPERSTAT1_gt_pclk_timer6_START (14)
#define SOC_SCTRL_SCPERSTAT1_gt_pclk_timer6_END (14)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_timer6_START (15)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_timer6_END (15)
#define SOC_SCTRL_SCPERSTAT1_gt_pclk_timer7_START (16)
#define SOC_SCTRL_SCPERSTAT1_gt_pclk_timer7_END (16)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_timer7_START (17)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_timer7_END (17)
#define SOC_SCTRL_SCPERSTAT1_gt_pclk_timer8_START (18)
#define SOC_SCTRL_SCPERSTAT1_gt_pclk_timer8_END (18)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_timer8_START (19)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_timer8_END (19)
#define SOC_SCTRL_SCPERSTAT1_gt_timerclk_refh_START (20)
#define SOC_SCTRL_SCPERSTAT1_gt_timerclk_refh_END (20)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_monitor_START (21)
#define SOC_SCTRL_SCPERSTAT1_gt_clk_monitor_END (21)
#define SOC_SCTRL_SCPERSTAT1_st_aclk_mmbuf_START (22)
#define SOC_SCTRL_SCPERSTAT1_st_aclk_mmbuf_END (22)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_mmbuf_START (23)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_mmbuf_END (23)
#define SOC_SCTRL_SCPERSTAT1_st_aclk_mmbuf_dss_START (24)
#define SOC_SCTRL_SCPERSTAT1_st_aclk_mmbuf_dss_END (24)
#define SOC_SCTRL_SCPERSTAT1_gt_pclk_mmbufcfg_START (25)
#define SOC_SCTRL_SCPERSTAT1_gt_pclk_mmbufcfg_END (25)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_asp_subsys_div_START (27)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_asp_subsys_div_END (27)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_sci_div_START (28)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_sci_div_END (28)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_aobus_div_START (29)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_aobus_div_END (29)
#define SOC_SCTRL_SCPERSTAT1_aobusclksw2sys_START (30)
#define SOC_SCTRL_SCPERSTAT1_aobusclksw2sys_END (30)
#define SOC_SCTRL_SCPERSTAT1_aobusclksw2pll_START (31)
#define SOC_SCTRL_SCPERSTAT1_aobusclksw2pll_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPEREN2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERDIS2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERCLKEN2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scperstat2 : 32;
    } reg;
} SOC_SCTRL_SCPERSTAT2_UNION;
#endif
#define SOC_SCTRL_SCPERSTAT2_scperstat2_START (0)
#define SOC_SCTRL_SCPERSTAT2_scperstat2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int timeren8ov : 1;
        unsigned int timeren8sel : 1;
        unsigned int timeren9ov : 1;
        unsigned int timeren9sel : 1;
        unsigned int timeren10ov : 1;
        unsigned int timeren10sel : 1;
        unsigned int timeren11ov : 1;
        unsigned int timeren11sel : 1;
        unsigned int timeren12ov : 1;
        unsigned int timeren12sel : 1;
        unsigned int timeren13ov : 1;
        unsigned int timeren13sel : 1;
        unsigned int timeren14ov : 1;
        unsigned int timeren14sel : 1;
        unsigned int timeren15ov : 1;
        unsigned int timeren15sel : 1;
        unsigned int timeren16ov : 1;
        unsigned int timeren16sel : 1;
        unsigned int timeren17ov : 1;
        unsigned int timeren17sel : 1;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCTIMERCTRL_UNION;
#endif
#define SOC_SCTRL_SCTIMERCTRL_timeren8ov_START (0)
#define SOC_SCTRL_SCTIMERCTRL_timeren8ov_END (0)
#define SOC_SCTRL_SCTIMERCTRL_timeren8sel_START (1)
#define SOC_SCTRL_SCTIMERCTRL_timeren8sel_END (1)
#define SOC_SCTRL_SCTIMERCTRL_timeren9ov_START (2)
#define SOC_SCTRL_SCTIMERCTRL_timeren9ov_END (2)
#define SOC_SCTRL_SCTIMERCTRL_timeren9sel_START (3)
#define SOC_SCTRL_SCTIMERCTRL_timeren9sel_END (3)
#define SOC_SCTRL_SCTIMERCTRL_timeren10ov_START (4)
#define SOC_SCTRL_SCTIMERCTRL_timeren10ov_END (4)
#define SOC_SCTRL_SCTIMERCTRL_timeren10sel_START (5)
#define SOC_SCTRL_SCTIMERCTRL_timeren10sel_END (5)
#define SOC_SCTRL_SCTIMERCTRL_timeren11ov_START (6)
#define SOC_SCTRL_SCTIMERCTRL_timeren11ov_END (6)
#define SOC_SCTRL_SCTIMERCTRL_timeren11sel_START (7)
#define SOC_SCTRL_SCTIMERCTRL_timeren11sel_END (7)
#define SOC_SCTRL_SCTIMERCTRL_timeren12ov_START (8)
#define SOC_SCTRL_SCTIMERCTRL_timeren12ov_END (8)
#define SOC_SCTRL_SCTIMERCTRL_timeren12sel_START (9)
#define SOC_SCTRL_SCTIMERCTRL_timeren12sel_END (9)
#define SOC_SCTRL_SCTIMERCTRL_timeren13ov_START (10)
#define SOC_SCTRL_SCTIMERCTRL_timeren13ov_END (10)
#define SOC_SCTRL_SCTIMERCTRL_timeren13sel_START (11)
#define SOC_SCTRL_SCTIMERCTRL_timeren13sel_END (11)
#define SOC_SCTRL_SCTIMERCTRL_timeren14ov_START (12)
#define SOC_SCTRL_SCTIMERCTRL_timeren14ov_END (12)
#define SOC_SCTRL_SCTIMERCTRL_timeren14sel_START (13)
#define SOC_SCTRL_SCTIMERCTRL_timeren14sel_END (13)
#define SOC_SCTRL_SCTIMERCTRL_timeren15ov_START (14)
#define SOC_SCTRL_SCTIMERCTRL_timeren15ov_END (14)
#define SOC_SCTRL_SCTIMERCTRL_timeren15sel_START (15)
#define SOC_SCTRL_SCTIMERCTRL_timeren15sel_END (15)
#define SOC_SCTRL_SCTIMERCTRL_timeren16ov_START (16)
#define SOC_SCTRL_SCTIMERCTRL_timeren16ov_END (16)
#define SOC_SCTRL_SCTIMERCTRL_timeren16sel_START (17)
#define SOC_SCTRL_SCTIMERCTRL_timeren16sel_END (17)
#define SOC_SCTRL_SCTIMERCTRL_timeren17ov_START (18)
#define SOC_SCTRL_SCTIMERCTRL_timeren17ov_END (18)
#define SOC_SCTRL_SCTIMERCTRL_timeren17sel_START (19)
#define SOC_SCTRL_SCTIMERCTRL_timeren17sel_END (19)
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
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTAT3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_rtc : 1;
        unsigned int ip_rst_rtc1 : 1;
        unsigned int ip_rst_timer0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_timer2 : 1;
        unsigned int ip_rst_timer3 : 1;
        unsigned int rst_asp_cfg : 1;
        unsigned int rst_asp_dw_axi : 1;
        unsigned int rst_asp_x2h : 1;
        unsigned int rst_asp_h2p : 1;
        unsigned int ip_rst_ao_gpio0 : 1;
        unsigned int ip_rst_ao_gpio1 : 1;
        unsigned int ip_rst_ao_gpio2 : 1;
        unsigned int ip_rst_ao_gpio3 : 1;
        unsigned int ip_rst_ao_gpio4 : 1;
        unsigned int ip_rst_ao_gpio5 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_ao_ioc : 1;
        unsigned int ip_prst_syscnt : 1;
        unsigned int ip_rst_syscnt : 1;
        unsigned int reserved_3 : 2;
        unsigned int ip_rst_sci0 : 1;
        unsigned int ip_rst_sci1 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int ip_rst_bbpdrx : 1;
        unsigned int reserved_6 : 1;
        unsigned int ip_rst_aobus : 1;
        unsigned int sc_rst_src_clr : 1;
        unsigned int sc_pmurst_ctrl : 1;
    } reg;
} SOC_SCTRL_SCPERRSTEN0_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_rtc_START (1)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_rtc_END (1)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_rtc1_START (2)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_rtc1_END (2)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_timer0_START (3)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_timer0_END (3)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_timer2_START (5)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_timer2_END (5)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_timer3_START (6)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_timer3_END (6)
#define SOC_SCTRL_SCPERRSTEN0_rst_asp_cfg_START (7)
#define SOC_SCTRL_SCPERRSTEN0_rst_asp_cfg_END (7)
#define SOC_SCTRL_SCPERRSTEN0_rst_asp_dw_axi_START (8)
#define SOC_SCTRL_SCPERRSTEN0_rst_asp_dw_axi_END (8)
#define SOC_SCTRL_SCPERRSTEN0_rst_asp_x2h_START (9)
#define SOC_SCTRL_SCPERRSTEN0_rst_asp_x2h_END (9)
#define SOC_SCTRL_SCPERRSTEN0_rst_asp_h2p_START (10)
#define SOC_SCTRL_SCPERRSTEN0_rst_asp_h2p_END (10)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_ao_gpio0_START (11)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_ao_gpio0_END (11)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_ao_gpio1_START (12)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_ao_gpio1_END (12)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_ao_gpio2_START (13)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_ao_gpio2_END (13)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_ao_gpio3_START (14)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_ao_gpio3_END (14)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_ao_gpio4_START (15)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_ao_gpio4_END (15)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_ao_gpio5_START (16)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_ao_gpio5_END (16)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_ao_ioc_START (18)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_ao_ioc_END (18)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_syscnt_START (19)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_syscnt_END (19)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_syscnt_START (20)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_syscnt_END (20)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_sci0_START (23)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_sci0_END (23)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_sci1_START (24)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_sci1_END (24)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_bbpdrx_START (27)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_bbpdrx_END (27)
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
        unsigned int ip_rst_rtc : 1;
        unsigned int ip_rst_rtc1 : 1;
        unsigned int ip_rst_timer0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_timer2 : 1;
        unsigned int ip_rst_timer3 : 1;
        unsigned int rst_asp_cfg : 1;
        unsigned int rst_asp_dw_axi : 1;
        unsigned int rst_asp_x2h : 1;
        unsigned int rst_asp_h2p : 1;
        unsigned int ip_rst_ao_gpio0 : 1;
        unsigned int ip_rst_ao_gpio1 : 1;
        unsigned int ip_rst_ao_gpio2 : 1;
        unsigned int ip_rst_ao_gpio3 : 1;
        unsigned int ip_rst_ao_gpio4 : 1;
        unsigned int ip_rst_ao_gpio5 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_ao_ioc : 1;
        unsigned int ip_prst_syscnt : 1;
        unsigned int ip_rst_syscnt : 1;
        unsigned int reserved_3 : 2;
        unsigned int ip_rst_sci0 : 1;
        unsigned int ip_rst_sci1 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int ip_rst_bbpdrx : 1;
        unsigned int reserved_6 : 1;
        unsigned int ip_rst_aobus : 1;
        unsigned int sc_rst_src_clr : 1;
        unsigned int sc_pmurst_ctrl : 1;
    } reg;
} SOC_SCTRL_SCPERRSTDIS0_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_rtc_START (1)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_rtc_END (1)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_rtc1_START (2)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_rtc1_END (2)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_timer0_START (3)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_timer0_END (3)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_timer2_START (5)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_timer2_END (5)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_timer3_START (6)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_timer3_END (6)
#define SOC_SCTRL_SCPERRSTDIS0_rst_asp_cfg_START (7)
#define SOC_SCTRL_SCPERRSTDIS0_rst_asp_cfg_END (7)
#define SOC_SCTRL_SCPERRSTDIS0_rst_asp_dw_axi_START (8)
#define SOC_SCTRL_SCPERRSTDIS0_rst_asp_dw_axi_END (8)
#define SOC_SCTRL_SCPERRSTDIS0_rst_asp_x2h_START (9)
#define SOC_SCTRL_SCPERRSTDIS0_rst_asp_x2h_END (9)
#define SOC_SCTRL_SCPERRSTDIS0_rst_asp_h2p_START (10)
#define SOC_SCTRL_SCPERRSTDIS0_rst_asp_h2p_END (10)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_ao_gpio0_START (11)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_ao_gpio0_END (11)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_ao_gpio1_START (12)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_ao_gpio1_END (12)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_ao_gpio2_START (13)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_ao_gpio2_END (13)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_ao_gpio3_START (14)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_ao_gpio3_END (14)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_ao_gpio4_START (15)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_ao_gpio4_END (15)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_ao_gpio5_START (16)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_ao_gpio5_END (16)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_ao_ioc_START (18)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_ao_ioc_END (18)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_syscnt_START (19)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_syscnt_END (19)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_syscnt_START (20)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_syscnt_END (20)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_sci0_START (23)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_sci0_END (23)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_sci1_START (24)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_sci1_END (24)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_bbpdrx_START (27)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_bbpdrx_END (27)
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
        unsigned int ip_rst_rtc : 1;
        unsigned int ip_rst_rtc1 : 1;
        unsigned int ip_rst_timer0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_timer2 : 1;
        unsigned int ip_rst_timer3 : 1;
        unsigned int rst_asp_cfg : 1;
        unsigned int rst_asp_dw_axi : 1;
        unsigned int rst_asp_x2h : 1;
        unsigned int rst_asp_h2p : 1;
        unsigned int ip_rst_ao_gpio0 : 1;
        unsigned int ip_rst_ao_gpio1 : 1;
        unsigned int ip_rst_ao_gpio2 : 1;
        unsigned int ip_rst_ao_gpio3 : 1;
        unsigned int ip_rst_ao_gpio4 : 1;
        unsigned int ip_rst_ao_gpio5 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_ao_ioc : 1;
        unsigned int ip_prst_syscnt : 1;
        unsigned int ip_rst_syscnt : 1;
        unsigned int reserved_3 : 2;
        unsigned int ip_rst_sci0 : 1;
        unsigned int ip_rst_sci1 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int ip_rst_bbpdrx : 1;
        unsigned int reserved_6 : 1;
        unsigned int ip_rst_aobus : 1;
        unsigned int sc_rst_src_clr : 1;
        unsigned int sc_pmurst_ctrl : 1;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT0_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_rtc_START (1)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_rtc_END (1)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_rtc1_START (2)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_rtc1_END (2)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_timer0_START (3)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_timer0_END (3)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_timer2_START (5)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_timer2_END (5)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_timer3_START (6)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_timer3_END (6)
#define SOC_SCTRL_SCPERRSTSTAT0_rst_asp_cfg_START (7)
#define SOC_SCTRL_SCPERRSTSTAT0_rst_asp_cfg_END (7)
#define SOC_SCTRL_SCPERRSTSTAT0_rst_asp_dw_axi_START (8)
#define SOC_SCTRL_SCPERRSTSTAT0_rst_asp_dw_axi_END (8)
#define SOC_SCTRL_SCPERRSTSTAT0_rst_asp_x2h_START (9)
#define SOC_SCTRL_SCPERRSTSTAT0_rst_asp_x2h_END (9)
#define SOC_SCTRL_SCPERRSTSTAT0_rst_asp_h2p_START (10)
#define SOC_SCTRL_SCPERRSTSTAT0_rst_asp_h2p_END (10)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_ao_gpio0_START (11)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_ao_gpio0_END (11)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_ao_gpio1_START (12)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_ao_gpio1_END (12)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_ao_gpio2_START (13)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_ao_gpio2_END (13)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_ao_gpio3_START (14)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_ao_gpio3_END (14)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_ao_gpio4_START (15)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_ao_gpio4_END (15)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_ao_gpio5_START (16)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_ao_gpio5_END (16)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_ao_ioc_START (18)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_ao_ioc_END (18)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_syscnt_START (19)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_syscnt_END (19)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_syscnt_START (20)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_syscnt_END (20)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_sci0_START (23)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_sci0_END (23)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_sci1_START (24)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_sci1_END (24)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_bbpdrx_START (27)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_bbpdrx_END (27)
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
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_timer4 : 1;
        unsigned int ip_rst_timer5 : 1;
        unsigned int ip_rst_timer6 : 1;
        unsigned int ip_rst_timer7 : 1;
        unsigned int ip_rst_timer8 : 1;
        unsigned int ip_rst_aobus_noc : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int ip_rst_mmbuf : 1;
        unsigned int ip_rst_mmbufcfg : 1;
        unsigned int ip_rst_autodiv_aobusnoc : 1;
        unsigned int reserved_5 : 2;
        unsigned int ip_rst_dram_tmp_0 : 2;
        unsigned int reserved_6 : 14;
    } reg;
} SOC_SCTRL_SCPERRSTEN1_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_timer4_START (3)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_timer4_END (3)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_timer5_START (4)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_timer5_END (4)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_timer6_START (5)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_timer6_END (5)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_timer7_START (6)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_timer7_END (6)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_timer8_START (7)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_timer8_END (7)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_aobus_noc_START (8)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_aobus_noc_END (8)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_mmbuf_START (11)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_mmbuf_END (11)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_mmbufcfg_START (12)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_mmbufcfg_END (12)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_autodiv_aobusnoc_START (13)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_autodiv_aobusnoc_END (13)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_dram_tmp_0_START (16)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_dram_tmp_0_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_timer4 : 1;
        unsigned int ip_rst_timer5 : 1;
        unsigned int ip_rst_timer6 : 1;
        unsigned int ip_rst_timer7 : 1;
        unsigned int ip_rst_timer8 : 1;
        unsigned int ip_rst_aobus_noc : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int ip_rst_mmbuf : 1;
        unsigned int ip_rst_mmbufcfg : 1;
        unsigned int ip_rst_autodiv_aobusnoc : 1;
        unsigned int reserved_5 : 2;
        unsigned int ip_rst_dram_tmp_0 : 2;
        unsigned int reserved_6 : 14;
    } reg;
} SOC_SCTRL_SCPERRSTDIS1_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_timer4_START (3)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_timer4_END (3)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_timer5_START (4)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_timer5_END (4)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_timer6_START (5)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_timer6_END (5)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_timer7_START (6)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_timer7_END (6)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_timer8_START (7)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_timer8_END (7)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_aobus_noc_START (8)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_aobus_noc_END (8)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_mmbuf_START (11)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_mmbuf_END (11)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_mmbufcfg_START (12)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_mmbufcfg_END (12)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_autodiv_aobusnoc_START (13)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_autodiv_aobusnoc_END (13)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_dram_tmp_0_START (16)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_dram_tmp_0_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_timer4 : 1;
        unsigned int ip_rst_timer5 : 1;
        unsigned int ip_rst_timer6 : 1;
        unsigned int ip_rst_timer7 : 1;
        unsigned int ip_rst_timer8 : 1;
        unsigned int ip_rst_aobus_noc : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int ip_rst_mmbuf : 1;
        unsigned int ip_rst_mmbufcfg : 1;
        unsigned int ip_rst_autodiv_aobusnoc : 1;
        unsigned int reserved_5 : 2;
        unsigned int ip_rst_dram_tmp_0 : 2;
        unsigned int reserved_6 : 14;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT1_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_timer4_START (3)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_timer4_END (3)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_timer5_START (4)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_timer5_END (4)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_timer6_START (5)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_timer6_END (5)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_timer7_START (6)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_timer7_END (6)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_timer8_START (7)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_timer8_END (7)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_aobus_noc_START (8)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_aobus_noc_END (8)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_mmbuf_START (11)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_mmbuf_END (11)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_mmbufcfg_START (12)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_mmbufcfg_END (12)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_autodiv_aobusnoc_START (13)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_autodiv_aobusnoc_END (13)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_dram_tmp_0_START (16)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_dram_tmp_0_END (17)
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
        unsigned int softrst_dram_dis : 1;
        unsigned int ip_rst_dram_tmp_1 : 2;
        unsigned int reserved_4 : 25;
    } reg;
} SOC_SCTRL_SCPERRSTEN2_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN2_softrst_dram_dis_START (4)
#define SOC_SCTRL_SCPERRSTEN2_softrst_dram_dis_END (4)
#define SOC_SCTRL_SCPERRSTEN2_ip_rst_dram_tmp_1_START (5)
#define SOC_SCTRL_SCPERRSTEN2_ip_rst_dram_tmp_1_END (6)
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
        unsigned int softrst_dram_dis : 1;
        unsigned int ip_rst_dram_tmp_1 : 2;
        unsigned int reserved_4 : 25;
    } reg;
} SOC_SCTRL_SCPERRSTDIS2_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS2_softrst_dram_dis_START (4)
#define SOC_SCTRL_SCPERRSTDIS2_softrst_dram_dis_END (4)
#define SOC_SCTRL_SCPERRSTDIS2_ip_rst_dram_tmp_1_START (5)
#define SOC_SCTRL_SCPERRSTDIS2_ip_rst_dram_tmp_1_END (6)
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
        unsigned int softrst_dram_dis : 1;
        unsigned int ip_rst_dram_tmp_1 : 2;
        unsigned int reserved_4 : 25;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT2_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT2_softrst_dram_dis_START (4)
#define SOC_SCTRL_SCPERRSTSTAT2_softrst_dram_dis_END (4)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_rst_dram_tmp_1_START (5)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_rst_dram_tmp_1_END (6)
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
        unsigned int rst_gt_mmbuf_bypass : 1;
        unsigned int rst_gt_asc_bypass : 1;
        unsigned int reserved : 7;
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
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_mmbuf_bypass_START (23)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_mmbuf_bypass_END (23)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_asc_bypass_START (24)
#define SOC_SCTRL_SCIPCLKRSTBUS_rst_gt_asc_bypass_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audioisoen : 1;
        unsigned int iomcuisoen : 1;
        unsigned int debugsubisoen : 1;
        unsigned int mmbufisoen : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_SCTRL_SCISOEN_UNION;
#endif
#define SOC_SCTRL_SCISOEN_audioisoen_START (0)
#define SOC_SCTRL_SCISOEN_audioisoen_END (0)
#define SOC_SCTRL_SCISOEN_iomcuisoen_START (1)
#define SOC_SCTRL_SCISOEN_iomcuisoen_END (1)
#define SOC_SCTRL_SCISOEN_debugsubisoen_START (2)
#define SOC_SCTRL_SCISOEN_debugsubisoen_END (2)
#define SOC_SCTRL_SCISOEN_mmbufisoen_START (3)
#define SOC_SCTRL_SCISOEN_mmbufisoen_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audioisoen : 1;
        unsigned int iomcuisoen : 1;
        unsigned int debugsubisoen : 1;
        unsigned int mmbufisoen : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_SCTRL_SCISODIS_UNION;
#endif
#define SOC_SCTRL_SCISODIS_audioisoen_START (0)
#define SOC_SCTRL_SCISODIS_audioisoen_END (0)
#define SOC_SCTRL_SCISODIS_iomcuisoen_START (1)
#define SOC_SCTRL_SCISODIS_iomcuisoen_END (1)
#define SOC_SCTRL_SCISODIS_debugsubisoen_START (2)
#define SOC_SCTRL_SCISODIS_debugsubisoen_END (2)
#define SOC_SCTRL_SCISODIS_mmbufisoen_START (3)
#define SOC_SCTRL_SCISODIS_mmbufisoen_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audioisoen : 1;
        unsigned int iomcuisoen : 1;
        unsigned int debugsubisoen : 1;
        unsigned int mmbufisoen : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_SCTRL_SCISOSTAT_UNION;
#endif
#define SOC_SCTRL_SCISOSTAT_audioisoen_START (0)
#define SOC_SCTRL_SCISOSTAT_audioisoen_END (0)
#define SOC_SCTRL_SCISOSTAT_iomcuisoen_START (1)
#define SOC_SCTRL_SCISOSTAT_iomcuisoen_END (1)
#define SOC_SCTRL_SCISOSTAT_debugsubisoen_START (2)
#define SOC_SCTRL_SCISOSTAT_debugsubisoen_END (2)
#define SOC_SCTRL_SCISOSTAT_mmbufisoen_START (3)
#define SOC_SCTRL_SCISOSTAT_mmbufisoen_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audiopwren : 1;
        unsigned int iomcupwren : 1;
        unsigned int debugsubpwren : 1;
        unsigned int mmbufpwren : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_SCTRL_SCPWREN_UNION;
#endif
#define SOC_SCTRL_SCPWREN_audiopwren_START (0)
#define SOC_SCTRL_SCPWREN_audiopwren_END (0)
#define SOC_SCTRL_SCPWREN_iomcupwren_START (1)
#define SOC_SCTRL_SCPWREN_iomcupwren_END (1)
#define SOC_SCTRL_SCPWREN_debugsubpwren_START (2)
#define SOC_SCTRL_SCPWREN_debugsubpwren_END (2)
#define SOC_SCTRL_SCPWREN_mmbufpwren_START (3)
#define SOC_SCTRL_SCPWREN_mmbufpwren_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audiopwren : 1;
        unsigned int iomcupwren : 1;
        unsigned int debugsubpwren : 1;
        unsigned int mmbufpwren : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_SCTRL_SCPWRDIS_UNION;
#endif
#define SOC_SCTRL_SCPWRDIS_audiopwren_START (0)
#define SOC_SCTRL_SCPWRDIS_audiopwren_END (0)
#define SOC_SCTRL_SCPWRDIS_iomcupwren_START (1)
#define SOC_SCTRL_SCPWRDIS_iomcupwren_END (1)
#define SOC_SCTRL_SCPWRDIS_debugsubpwren_START (2)
#define SOC_SCTRL_SCPWRDIS_debugsubpwren_END (2)
#define SOC_SCTRL_SCPWRDIS_mmbufpwren_START (3)
#define SOC_SCTRL_SCPWRDIS_mmbufpwren_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audiopwrstat : 1;
        unsigned int iomcupwrstat : 1;
        unsigned int debugsubpwrstat : 1;
        unsigned int mmbufpwrstat : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_SCTRL_SCPWRSTAT_UNION;
#endif
#define SOC_SCTRL_SCPWRSTAT_audiopwrstat_START (0)
#define SOC_SCTRL_SCPWRSTAT_audiopwrstat_END (0)
#define SOC_SCTRL_SCPWRSTAT_iomcupwrstat_START (1)
#define SOC_SCTRL_SCPWRSTAT_iomcupwrstat_END (1)
#define SOC_SCTRL_SCPWRSTAT_debugsubpwrstat_START (2)
#define SOC_SCTRL_SCPWRSTAT_debugsubpwrstat_END (2)
#define SOC_SCTRL_SCPWRSTAT_mmbufpwrstat_START (3)
#define SOC_SCTRL_SCPWRSTAT_mmbufpwrstat_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int audiopwrack : 1;
        unsigned int reserved_0 : 1;
        unsigned int debugsubpwrack : 1;
        unsigned int mmbufpwrack : 1;
        unsigned int reserved_1 : 28;
    } reg;
} SOC_SCTRL_SCPWRACK_UNION;
#endif
#define SOC_SCTRL_SCPWRACK_audiopwrack_START (0)
#define SOC_SCTRL_SCPWRACK_audiopwrack_END (0)
#define SOC_SCTRL_SCPWRACK_debugsubpwrack_START (2)
#define SOC_SCTRL_SCPWRACK_debugsubpwrack_END (2)
#define SOC_SCTRL_SCPWRACK_mmbufpwrack_START (3)
#define SOC_SCTRL_SCPWRACK_mmbufpwrack_END (3)
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
        unsigned int sc_div_asp_subsys : 3;
        unsigned int sc_div_hifidsp : 6;
        unsigned int sc_div_sci : 2;
        unsigned int sc_clk_asp_pll_sel : 1;
        unsigned int sc_sel_mmbuf_backup : 1;
        unsigned int sc_sel_memrep : 1;
        unsigned int reserved : 1;
        unsigned int extclk_en : 1;
        unsigned int scclkdiv0_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV0_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV0_sc_div_asp_subsys_START (0)
#define SOC_SCTRL_SCCLKDIV0_sc_div_asp_subsys_END (2)
#define SOC_SCTRL_SCCLKDIV0_sc_div_hifidsp_START (3)
#define SOC_SCTRL_SCCLKDIV0_sc_div_hifidsp_END (8)
#define SOC_SCTRL_SCCLKDIV0_sc_div_sci_START (9)
#define SOC_SCTRL_SCCLKDIV0_sc_div_sci_END (10)
#define SOC_SCTRL_SCCLKDIV0_sc_clk_asp_pll_sel_START (11)
#define SOC_SCTRL_SCCLKDIV0_sc_clk_asp_pll_sel_END (11)
#define SOC_SCTRL_SCCLKDIV0_sc_sel_mmbuf_backup_START (12)
#define SOC_SCTRL_SCCLKDIV0_sc_sel_mmbuf_backup_END (12)
#define SOC_SCTRL_SCCLKDIV0_sc_sel_memrep_START (13)
#define SOC_SCTRL_SCCLKDIV0_sc_sel_memrep_END (13)
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
        unsigned int sc_div_aobus : 6;
        unsigned int div_clkout0_tcxo : 3;
        unsigned int div_clkout1_tcxo : 3;
        unsigned int sel_clkout0 : 2;
        unsigned int sel_clkout1 : 2;
        unsigned int scclkdiv1_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV1_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV1_sc_div_aobus_START (0)
#define SOC_SCTRL_SCCLKDIV1_sc_div_aobus_END (5)
#define SOC_SCTRL_SCCLKDIV1_div_clkout0_tcxo_START (6)
#define SOC_SCTRL_SCCLKDIV1_div_clkout0_tcxo_END (8)
#define SOC_SCTRL_SCCLKDIV1_div_clkout1_tcxo_START (9)
#define SOC_SCTRL_SCCLKDIV1_div_clkout1_tcxo_END (11)
#define SOC_SCTRL_SCCLKDIV1_sel_clkout0_START (12)
#define SOC_SCTRL_SCCLKDIV1_sel_clkout0_END (13)
#define SOC_SCTRL_SCCLKDIV1_sel_clkout1_START (14)
#define SOC_SCTRL_SCCLKDIV1_sel_clkout1_END (15)
#define SOC_SCTRL_SCCLKDIV1_scclkdiv1_msk_START (16)
#define SOC_SCTRL_SCCLKDIV1_scclkdiv1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_asp_subsys : 1;
        unsigned int gt_hifidsp_clk_div : 1;
        unsigned int gt_clk_sci : 1;
        unsigned int gt_clk_aobus : 1;
        unsigned int gt_asp_hclk_div : 1;
        unsigned int gt_sclk_tp : 1;
        unsigned int gt_pll_clk_mmbuf : 1;
        unsigned int gt_pclk_mmbuf : 1;
        unsigned int sel_clk_mmbuf : 2;
        unsigned int sc_div2_mmbuf : 2;
        unsigned int sc_div4_mmbuf : 4;
        unsigned int scclkdiv2_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV2_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV2_gt_clk_asp_subsys_START (0)
#define SOC_SCTRL_SCCLKDIV2_gt_clk_asp_subsys_END (0)
#define SOC_SCTRL_SCCLKDIV2_gt_hifidsp_clk_div_START (1)
#define SOC_SCTRL_SCCLKDIV2_gt_hifidsp_clk_div_END (1)
#define SOC_SCTRL_SCCLKDIV2_gt_clk_sci_START (2)
#define SOC_SCTRL_SCCLKDIV2_gt_clk_sci_END (2)
#define SOC_SCTRL_SCCLKDIV2_gt_clk_aobus_START (3)
#define SOC_SCTRL_SCCLKDIV2_gt_clk_aobus_END (3)
#define SOC_SCTRL_SCCLKDIV2_gt_asp_hclk_div_START (4)
#define SOC_SCTRL_SCCLKDIV2_gt_asp_hclk_div_END (4)
#define SOC_SCTRL_SCCLKDIV2_gt_sclk_tp_START (5)
#define SOC_SCTRL_SCCLKDIV2_gt_sclk_tp_END (5)
#define SOC_SCTRL_SCCLKDIV2_gt_pll_clk_mmbuf_START (6)
#define SOC_SCTRL_SCCLKDIV2_gt_pll_clk_mmbuf_END (6)
#define SOC_SCTRL_SCCLKDIV2_gt_pclk_mmbuf_START (7)
#define SOC_SCTRL_SCCLKDIV2_gt_pclk_mmbuf_END (7)
#define SOC_SCTRL_SCCLKDIV2_sel_clk_mmbuf_START (8)
#define SOC_SCTRL_SCCLKDIV2_sel_clk_mmbuf_END (9)
#define SOC_SCTRL_SCCLKDIV2_sc_div2_mmbuf_START (10)
#define SOC_SCTRL_SCCLKDIV2_sc_div2_mmbuf_END (11)
#define SOC_SCTRL_SCCLKDIV2_sc_div4_mmbuf_START (12)
#define SOC_SCTRL_SCCLKDIV2_sc_div4_mmbuf_END (15)
#define SOC_SCTRL_SCCLKDIV2_scclkdiv2_msk_START (16)
#define SOC_SCTRL_SCCLKDIV2_scclkdiv2_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aobus_auto_waitcfg_out : 10;
        unsigned int aobus_div_auto_cfg : 6;
        unsigned int scclkdiv3_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV3_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV3_aobus_auto_waitcfg_out_START (0)
#define SOC_SCTRL_SCCLKDIV3_aobus_auto_waitcfg_out_END (9)
#define SOC_SCTRL_SCCLKDIV3_aobus_div_auto_cfg_START (10)
#define SOC_SCTRL_SCCLKDIV3_aobus_div_auto_cfg_END (15)
#define SOC_SCTRL_SCCLKDIV3_scclkdiv3_msk_START (16)
#define SOC_SCTRL_SCCLKDIV3_scclkdiv3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_aobus_noc : 4;
        unsigned int sel_clk_aobus_noc : 2;
        unsigned int gt_aobus_noc_ini : 1;
        unsigned int reserved : 1;
        unsigned int sc_div_asp_hclk : 2;
        unsigned int gt_clk_memrep : 1;
        unsigned int sc_div_memrep : 5;
        unsigned int scclkdiv4_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV4_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV4_div_aobus_noc_START (0)
#define SOC_SCTRL_SCCLKDIV4_div_aobus_noc_END (3)
#define SOC_SCTRL_SCCLKDIV4_sel_clk_aobus_noc_START (4)
#define SOC_SCTRL_SCCLKDIV4_sel_clk_aobus_noc_END (5)
#define SOC_SCTRL_SCCLKDIV4_gt_aobus_noc_ini_START (6)
#define SOC_SCTRL_SCCLKDIV4_gt_aobus_noc_ini_END (6)
#define SOC_SCTRL_SCCLKDIV4_sc_div_asp_hclk_START (8)
#define SOC_SCTRL_SCCLKDIV4_sc_div_asp_hclk_END (9)
#define SOC_SCTRL_SCCLKDIV4_gt_clk_memrep_START (10)
#define SOC_SCTRL_SCCLKDIV4_gt_clk_memrep_END (10)
#define SOC_SCTRL_SCCLKDIV4_sc_div_memrep_START (11)
#define SOC_SCTRL_SCCLKDIV4_sc_div_memrep_END (15)
#define SOC_SCTRL_SCCLKDIV4_scclkdiv4_msk_START (16)
#define SOC_SCTRL_SCCLKDIV4_scclkdiv4_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aobus_div_auto_reduce_bypass : 1;
        unsigned int aobus_auto_waitcfg_in : 10;
        unsigned int aobus_clk_sw_req_cfg : 2;
        unsigned int reserved : 3;
        unsigned int scclkdiv5_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV5_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV5_aobus_div_auto_reduce_bypass_START (0)
#define SOC_SCTRL_SCCLKDIV5_aobus_div_auto_reduce_bypass_END (0)
#define SOC_SCTRL_SCCLKDIV5_aobus_auto_waitcfg_in_START (1)
#define SOC_SCTRL_SCCLKDIV5_aobus_auto_waitcfg_in_END (10)
#define SOC_SCTRL_SCCLKDIV5_aobus_clk_sw_req_cfg_START (11)
#define SOC_SCTRL_SCCLKDIV5_aobus_clk_sw_req_cfg_END (12)
#define SOC_SCTRL_SCCLKDIV5_scclkdiv5_msk_START (16)
#define SOC_SCTRL_SCCLKDIV5_scclkdiv5_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int efuse_remap : 1;
        unsigned int reserved_1 : 3;
        unsigned int reserved_2 : 12;
    } reg;
} SOC_SCTRL_SCPERCTRL0_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL0_efuse_remap_START (16)
#define SOC_SCTRL_SCPERCTRL0_efuse_remap_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int repair_disable : 32;
    } reg;
} SOC_SCTRL_SCPERCTRL1_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL1_repair_disable_START (0)
#define SOC_SCTRL_SCPERCTRL1_repair_disable_END (31)
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
        unsigned int jtaq_to_sdcard : 1;
        unsigned int sc_djtaq_sel : 1;
        unsigned int reserved_1 : 6;
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
#define SOC_SCTRL_SCPERCTRL2_jtaq_to_sdcard_START (24)
#define SOC_SCTRL_SCPERCTRL2_jtaq_to_sdcard_END (24)
#define SOC_SCTRL_SCPERCTRL2_sc_djtaq_sel_START (25)
#define SOC_SCTRL_SCPERCTRL2_sc_djtaq_sel_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int repair_debug_sel : 1;
        unsigned int reset_memrep_msk : 1;
        unsigned int repair_iddq : 1;
        unsigned int repair_frm_sram : 1;
        unsigned int repair_frm_efu : 1;
        unsigned int dram_pwr_ctl : 3;
        unsigned int efusec_timeout_bypass : 1;
        unsigned int bbdrx_timeout_bypass : 1;
        unsigned int sleep_mode_cfgbus_bypass : 1;
        unsigned int sleep_mode_lpmcu_bypass : 1;
        unsigned int ao_mrb_rst_msk : 1;
        unsigned int sleep_mode_aobus_bypass : 1;
        unsigned int bbp_clk_en : 1;
        unsigned int reserved : 1;
        unsigned int bbp_on_ctrl : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL3_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL3_repair_debug_sel_START (0)
#define SOC_SCTRL_SCPERCTRL3_repair_debug_sel_END (0)
#define SOC_SCTRL_SCPERCTRL3_reset_memrep_msk_START (1)
#define SOC_SCTRL_SCPERCTRL3_reset_memrep_msk_END (1)
#define SOC_SCTRL_SCPERCTRL3_repair_iddq_START (2)
#define SOC_SCTRL_SCPERCTRL3_repair_iddq_END (2)
#define SOC_SCTRL_SCPERCTRL3_repair_frm_sram_START (3)
#define SOC_SCTRL_SCPERCTRL3_repair_frm_sram_END (3)
#define SOC_SCTRL_SCPERCTRL3_repair_frm_efu_START (4)
#define SOC_SCTRL_SCPERCTRL3_repair_frm_efu_END (4)
#define SOC_SCTRL_SCPERCTRL3_dram_pwr_ctl_START (5)
#define SOC_SCTRL_SCPERCTRL3_dram_pwr_ctl_END (7)
#define SOC_SCTRL_SCPERCTRL3_efusec_timeout_bypass_START (8)
#define SOC_SCTRL_SCPERCTRL3_efusec_timeout_bypass_END (8)
#define SOC_SCTRL_SCPERCTRL3_bbdrx_timeout_bypass_START (9)
#define SOC_SCTRL_SCPERCTRL3_bbdrx_timeout_bypass_END (9)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_cfgbus_bypass_START (10)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_cfgbus_bypass_END (10)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_lpmcu_bypass_START (11)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_lpmcu_bypass_END (11)
#define SOC_SCTRL_SCPERCTRL3_ao_mrb_rst_msk_START (12)
#define SOC_SCTRL_SCPERCTRL3_ao_mrb_rst_msk_END (12)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_aobus_bypass_START (13)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_aobus_bypass_END (13)
#define SOC_SCTRL_SCPERCTRL3_bbp_clk_en_START (14)
#define SOC_SCTRL_SCPERCTRL3_bbp_clk_en_END (14)
#define SOC_SCTRL_SCPERCTRL3_bbp_on_ctrl_START (16)
#define SOC_SCTRL_SCPERCTRL3_bbp_on_ctrl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddr_ret_en : 4;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 3;
        unsigned int crc_value : 21;
        unsigned int reserved_2 : 3;
    } reg;
} SOC_SCTRL_SCPERCTRL4_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL4_ddr_ret_en_START (0)
#define SOC_SCTRL_SCPERCTRL4_ddr_ret_en_END (3)
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
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 2;
        unsigned int reserved_2 : 9;
        unsigned int gt_rst_asc : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 8;
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
#define SOC_SCTRL_SCPERCTRL6_gt_rst_asc_START (19)
#define SOC_SCTRL_SCPERCTRL6_gt_rst_asc_END (19)
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
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
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
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int err_flag : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS1_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS1_err_flag_START (0)
#define SOC_SCTRL_SCPERSTATUS1_err_flag_END (31)
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
        unsigned int repair_done : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS3_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS3_repair_done_START (0)
#define SOC_SCTRL_SCPERSTATUS3_repair_done_END (31)
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
        unsigned int aomcu_ao_noc_mainstatalarm_int : 1;
        unsigned int aomcu_ao_noc_mainfault_int : 1;
        unsigned int asp_transaction_probe_mainstatalarm_int : 1;
        unsigned int asp_error_probe_observer_mainfault_int : 1;
        unsigned int repair_start_point_err : 1;
        unsigned int pack_is0_err : 1;
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
#define SOC_SCTRL_SCPERSTATUS5_aomcu_ao_noc_mainstatalarm_int_START (20)
#define SOC_SCTRL_SCPERSTATUS5_aomcu_ao_noc_mainstatalarm_int_END (20)
#define SOC_SCTRL_SCPERSTATUS5_aomcu_ao_noc_mainfault_int_START (21)
#define SOC_SCTRL_SCPERSTATUS5_aomcu_ao_noc_mainfault_int_END (21)
#define SOC_SCTRL_SCPERSTATUS5_asp_transaction_probe_mainstatalarm_int_START (22)
#define SOC_SCTRL_SCPERSTATUS5_asp_transaction_probe_mainstatalarm_int_END (22)
#define SOC_SCTRL_SCPERSTATUS5_asp_error_probe_observer_mainfault_int_START (23)
#define SOC_SCTRL_SCPERSTATUS5_asp_error_probe_observer_mainfault_int_END (23)
#define SOC_SCTRL_SCPERSTATUS5_repair_start_point_err_START (24)
#define SOC_SCTRL_SCPERSTATUS5_repair_start_point_err_END (24)
#define SOC_SCTRL_SCPERSTATUS5_pack_is0_err_START (25)
#define SOC_SCTRL_SCPERSTATUS5_pack_is0_err_END (25)
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
        unsigned int peri_stat6 : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS6_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS6_peri_stat6_START (0)
#define SOC_SCTRL_SCPERSTATUS6_peri_stat6_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int repair_finish_flag : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS7_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS7_repair_finish_flag_START (0)
#define SOC_SCTRL_SCPERSTATUS7_repair_finish_flag_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int repair_data_err0 : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS8_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS8_repair_data_err0_START (0)
#define SOC_SCTRL_SCPERSTATUS8_repair_data_err0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int repair_data_err1 : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS9_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS9_repair_data_err1_START (0)
#define SOC_SCTRL_SCPERSTATUS9_repair_data_err1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int repair_data_err2 : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS10_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS10_repair_data_err2_START (0)
#define SOC_SCTRL_SCPERSTATUS10_repair_data_err2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mrb_need_repair : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS11_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS11_mrb_need_repair_START (0)
#define SOC_SCTRL_SCPERSTATUS11_mrb_need_repair_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int repair_disable_flag : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS12_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS12_repair_disable_flag_START (0)
#define SOC_SCTRL_SCPERSTATUS12_repair_disable_flag_END (31)
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
        unsigned int scperstatus14 : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS14_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS14_scperstatus14_START (0)
#define SOC_SCTRL_SCPERSTATUS14_scperstatus14_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_mmbuf_pending_en : 1;
        unsigned int reserved : 1;
        unsigned int noc_asp_power_idlereq : 1;
        unsigned int iomcu_power_idlereq : 1;
        unsigned int aomcu_ao_noc_mainstatalarm_intmsk : 1;
        unsigned int aomcu_ao_noc_mainfault_intmsk : 1;
        unsigned int asp_transaction_probe_mainstatalarm_intmsk : 1;
        unsigned int asp_error_probe_observer_mainfault_intmsk : 1;
        unsigned int msk_noc_aobus_mmbuf_t_maintimeout : 1;
        unsigned int msk_noc_asp_cfg_t_maintimeout : 1;
        unsigned int msk_iomcu_ahb_slv_maintimeout : 1;
        unsigned int msk_iomcu_apb_slv_maintimeout : 1;
        unsigned int scperctrl7 : 4;
        unsigned int scperctrl7_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL7_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL7_gt_clk_mmbuf_pending_en_START (0)
#define SOC_SCTRL_SCPERCTRL7_gt_clk_mmbuf_pending_en_END (0)
#define SOC_SCTRL_SCPERCTRL7_noc_asp_power_idlereq_START (2)
#define SOC_SCTRL_SCPERCTRL7_noc_asp_power_idlereq_END (2)
#define SOC_SCTRL_SCPERCTRL7_iomcu_power_idlereq_START (3)
#define SOC_SCTRL_SCPERCTRL7_iomcu_power_idlereq_END (3)
#define SOC_SCTRL_SCPERCTRL7_aomcu_ao_noc_mainstatalarm_intmsk_START (4)
#define SOC_SCTRL_SCPERCTRL7_aomcu_ao_noc_mainstatalarm_intmsk_END (4)
#define SOC_SCTRL_SCPERCTRL7_aomcu_ao_noc_mainfault_intmsk_START (5)
#define SOC_SCTRL_SCPERCTRL7_aomcu_ao_noc_mainfault_intmsk_END (5)
#define SOC_SCTRL_SCPERCTRL7_asp_transaction_probe_mainstatalarm_intmsk_START (6)
#define SOC_SCTRL_SCPERCTRL7_asp_transaction_probe_mainstatalarm_intmsk_END (6)
#define SOC_SCTRL_SCPERCTRL7_asp_error_probe_observer_mainfault_intmsk_START (7)
#define SOC_SCTRL_SCPERCTRL7_asp_error_probe_observer_mainfault_intmsk_END (7)
#define SOC_SCTRL_SCPERCTRL7_msk_noc_aobus_mmbuf_t_maintimeout_START (8)
#define SOC_SCTRL_SCPERCTRL7_msk_noc_aobus_mmbuf_t_maintimeout_END (8)
#define SOC_SCTRL_SCPERCTRL7_msk_noc_asp_cfg_t_maintimeout_START (9)
#define SOC_SCTRL_SCPERCTRL7_msk_noc_asp_cfg_t_maintimeout_END (9)
#define SOC_SCTRL_SCPERCTRL7_msk_iomcu_ahb_slv_maintimeout_START (10)
#define SOC_SCTRL_SCPERCTRL7_msk_iomcu_ahb_slv_maintimeout_END (10)
#define SOC_SCTRL_SCPERCTRL7_msk_iomcu_apb_slv_maintimeout_START (11)
#define SOC_SCTRL_SCPERCTRL7_msk_iomcu_apb_slv_maintimeout_END (11)
#define SOC_SCTRL_SCPERCTRL7_scperctrl7_START (12)
#define SOC_SCTRL_SCPERCTRL7_scperctrl7_END (15)
#define SOC_SCTRL_SCPERCTRL7_scperctrl7_msk_START (16)
#define SOC_SCTRL_SCPERCTRL7_scperctrl7_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int memory_repair_bypass : 1;
        unsigned int hardw_ctrl_sel0 : 1;
        unsigned int factest_mode : 2;
        unsigned int memory_repair_bypass_hw : 10;
        unsigned int isp_face_detect_fuse : 1;
        unsigned int reserved_0 : 1;
        unsigned int ate_mode : 3;
        unsigned int vote_clk_gt_sel_flag : 7;
        unsigned int reserved_1 : 6;
    } reg;
} SOC_SCTRL_SCINNERSTAT_UNION;
#endif
#define SOC_SCTRL_SCINNERSTAT_memory_repair_bypass_START (0)
#define SOC_SCTRL_SCINNERSTAT_memory_repair_bypass_END (0)
#define SOC_SCTRL_SCINNERSTAT_hardw_ctrl_sel0_START (1)
#define SOC_SCTRL_SCINNERSTAT_hardw_ctrl_sel0_END (1)
#define SOC_SCTRL_SCINNERSTAT_factest_mode_START (2)
#define SOC_SCTRL_SCINNERSTAT_factest_mode_END (3)
#define SOC_SCTRL_SCINNERSTAT_memory_repair_bypass_hw_START (4)
#define SOC_SCTRL_SCINNERSTAT_memory_repair_bypass_hw_END (13)
#define SOC_SCTRL_SCINNERSTAT_isp_face_detect_fuse_START (14)
#define SOC_SCTRL_SCINNERSTAT_isp_face_detect_fuse_END (14)
#define SOC_SCTRL_SCINNERSTAT_ate_mode_START (16)
#define SOC_SCTRL_SCINNERSTAT_ate_mode_END (18)
#define SOC_SCTRL_SCINNERSTAT_vote_clk_gt_sel_flag_START (19)
#define SOC_SCTRL_SCINNERSTAT_vote_clk_gt_sel_flag_END (25)
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
        unsigned int mrb_efuse_busy : 32;
    } reg;
} SOC_SCTRL_SCMRBBUSYSTAT_UNION;
#endif
#define SOC_SCTRL_SCMRBBUSYSTAT_mrb_efuse_busy_START (0)
#define SOC_SCTRL_SCMRBBUSYSTAT_mrb_efuse_busy_END (31)
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
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 24;
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
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 25;
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
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 25;
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
        unsigned int gic_fiq_int0 : 1;
        unsigned int gic_fiq_int1 : 1;
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
#define SOC_SCTRL_SCINT_STAT2_gic_fiq_int0_START (2)
#define SOC_SCTRL_SCINT_STAT2_gic_fiq_int0_END (2)
#define SOC_SCTRL_SCINT_STAT2_gic_fiq_int1_START (3)
#define SOC_SCTRL_SCINT_STAT2_gic_fiq_int1_END (3)
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
        unsigned int mali_dbgack_byp : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCMALIBYPCFG_UNION;
#endif
#define SOC_SCTRL_SCMALIBYPCFG_mali_dbgack_byp_START (0)
#define SOC_SCTRL_SCMALIBYPCFG_mali_dbgack_byp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dapclkoff_sys_n : 1;
        unsigned int ssi1_clk_clkoff_sys_n : 1;
        unsigned int rom_clk_clkoff_sys_n : 1;
        unsigned int ram_clk_clkoff_sys_n : 1;
        unsigned int aon_clk_clkoff_sys_n : 1;
        unsigned int ddrc_clk_clkoff_sys_n : 1;
        unsigned int tsen_clk_clkoff_sys_n : 1;
        unsigned int pmc_clk_clkoff_sys_n : 1;
        unsigned int uart_clk_clkoff_sys_n : 1;
        unsigned int i2c_clk_clkoff_sys_n : 1;
        unsigned int ssi_clk_clkoff_sys_n : 1;
        unsigned int pcrg_clk_clkoff_sys_n : 1;
        unsigned int nandc_clk_clkoff_sys_n : 1;
        unsigned int wdog_clk_clkoff_sys_n : 1;
        unsigned int tim_clk_clkoff_sys_n : 1;
        unsigned int cssys_clk_clkoff_sys_n : 1;
        unsigned int sclpmcuclken_msk : 16;
    } reg;
} SOC_SCTRL_SCLPMCUCLKEN_UNION;
#endif
#define SOC_SCTRL_SCLPMCUCLKEN_dapclkoff_sys_n_START (0)
#define SOC_SCTRL_SCLPMCUCLKEN_dapclkoff_sys_n_END (0)
#define SOC_SCTRL_SCLPMCUCLKEN_ssi1_clk_clkoff_sys_n_START (1)
#define SOC_SCTRL_SCLPMCUCLKEN_ssi1_clk_clkoff_sys_n_END (1)
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
#define SOC_SCTRL_SCLPMCUCLKEN_i2c_clk_clkoff_sys_n_START (9)
#define SOC_SCTRL_SCLPMCUCLKEN_i2c_clk_clkoff_sys_n_END (9)
#define SOC_SCTRL_SCLPMCUCLKEN_ssi_clk_clkoff_sys_n_START (10)
#define SOC_SCTRL_SCLPMCUCLKEN_ssi_clk_clkoff_sys_n_END (10)
#define SOC_SCTRL_SCLPMCUCLKEN_pcrg_clk_clkoff_sys_n_START (11)
#define SOC_SCTRL_SCLPMCUCLKEN_pcrg_clk_clkoff_sys_n_END (11)
#define SOC_SCTRL_SCLPMCUCLKEN_nandc_clk_clkoff_sys_n_START (12)
#define SOC_SCTRL_SCLPMCUCLKEN_nandc_clk_clkoff_sys_n_END (12)
#define SOC_SCTRL_SCLPMCUCLKEN_wdog_clk_clkoff_sys_n_START (13)
#define SOC_SCTRL_SCLPMCUCLKEN_wdog_clk_clkoff_sys_n_END (13)
#define SOC_SCTRL_SCLPMCUCLKEN_tim_clk_clkoff_sys_n_START (14)
#define SOC_SCTRL_SCLPMCUCLKEN_tim_clk_clkoff_sys_n_END (14)
#define SOC_SCTRL_SCLPMCUCLKEN_cssys_clk_clkoff_sys_n_START (15)
#define SOC_SCTRL_SCLPMCUCLKEN_cssys_clk_clkoff_sys_n_END (15)
#define SOC_SCTRL_SCLPMCUCLKEN_sclpmcuclken_msk_START (16)
#define SOC_SCTRL_SCLPMCUCLKEN_sclpmcuclken_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rst_software_req : 1;
        unsigned int coresight_soft_rst_req : 1;
        unsigned int ssi1_soft_rst_req : 1;
        unsigned int rom_soft_rst_req : 1;
        unsigned int ram_soft_rst_req : 1;
        unsigned int aon_soft_rst_req : 1;
        unsigned int ddrc_soft_rst_req : 1;
        unsigned int tsen_soft_rst_req : 1;
        unsigned int pmc_soft_rst_req : 1;
        unsigned int uart_soft_rst_req : 1;
        unsigned int i2c_soft_rst_req : 1;
        unsigned int ssi_soft_rst_req : 1;
        unsigned int pcrg_soft_rst_req : 1;
        unsigned int nandc_soft_rst_req : 1;
        unsigned int wdog_soft_rst_req : 1;
        unsigned int tim_soft_rst_req : 1;
        unsigned int cssys_soft_rst_req : 1;
        unsigned int m3_lp_rst_req : 1;
        unsigned int reserved : 14;
    } reg;
} SOC_SCTRL_SCLPMCURSTEN_UNION;
#endif
#define SOC_SCTRL_SCLPMCURSTEN_rst_software_req_START (0)
#define SOC_SCTRL_SCLPMCURSTEN_rst_software_req_END (0)
#define SOC_SCTRL_SCLPMCURSTEN_coresight_soft_rst_req_START (1)
#define SOC_SCTRL_SCLPMCURSTEN_coresight_soft_rst_req_END (1)
#define SOC_SCTRL_SCLPMCURSTEN_ssi1_soft_rst_req_START (2)
#define SOC_SCTRL_SCLPMCURSTEN_ssi1_soft_rst_req_END (2)
#define SOC_SCTRL_SCLPMCURSTEN_rom_soft_rst_req_START (3)
#define SOC_SCTRL_SCLPMCURSTEN_rom_soft_rst_req_END (3)
#define SOC_SCTRL_SCLPMCURSTEN_ram_soft_rst_req_START (4)
#define SOC_SCTRL_SCLPMCURSTEN_ram_soft_rst_req_END (4)
#define SOC_SCTRL_SCLPMCURSTEN_aon_soft_rst_req_START (5)
#define SOC_SCTRL_SCLPMCURSTEN_aon_soft_rst_req_END (5)
#define SOC_SCTRL_SCLPMCURSTEN_ddrc_soft_rst_req_START (6)
#define SOC_SCTRL_SCLPMCURSTEN_ddrc_soft_rst_req_END (6)
#define SOC_SCTRL_SCLPMCURSTEN_tsen_soft_rst_req_START (7)
#define SOC_SCTRL_SCLPMCURSTEN_tsen_soft_rst_req_END (7)
#define SOC_SCTRL_SCLPMCURSTEN_pmc_soft_rst_req_START (8)
#define SOC_SCTRL_SCLPMCURSTEN_pmc_soft_rst_req_END (8)
#define SOC_SCTRL_SCLPMCURSTEN_uart_soft_rst_req_START (9)
#define SOC_SCTRL_SCLPMCURSTEN_uart_soft_rst_req_END (9)
#define SOC_SCTRL_SCLPMCURSTEN_i2c_soft_rst_req_START (10)
#define SOC_SCTRL_SCLPMCURSTEN_i2c_soft_rst_req_END (10)
#define SOC_SCTRL_SCLPMCURSTEN_ssi_soft_rst_req_START (11)
#define SOC_SCTRL_SCLPMCURSTEN_ssi_soft_rst_req_END (11)
#define SOC_SCTRL_SCLPMCURSTEN_pcrg_soft_rst_req_START (12)
#define SOC_SCTRL_SCLPMCURSTEN_pcrg_soft_rst_req_END (12)
#define SOC_SCTRL_SCLPMCURSTEN_nandc_soft_rst_req_START (13)
#define SOC_SCTRL_SCLPMCURSTEN_nandc_soft_rst_req_END (13)
#define SOC_SCTRL_SCLPMCURSTEN_wdog_soft_rst_req_START (14)
#define SOC_SCTRL_SCLPMCURSTEN_wdog_soft_rst_req_END (14)
#define SOC_SCTRL_SCLPMCURSTEN_tim_soft_rst_req_START (15)
#define SOC_SCTRL_SCLPMCURSTEN_tim_soft_rst_req_END (15)
#define SOC_SCTRL_SCLPMCURSTEN_cssys_soft_rst_req_START (16)
#define SOC_SCTRL_SCLPMCURSTEN_cssys_soft_rst_req_END (16)
#define SOC_SCTRL_SCLPMCURSTEN_m3_lp_rst_req_START (17)
#define SOC_SCTRL_SCLPMCURSTEN_m3_lp_rst_req_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rst_software_req : 1;
        unsigned int coresight_soft_rst_req : 1;
        unsigned int ssi1_soft_rst_req : 1;
        unsigned int rom_soft_rst_req : 1;
        unsigned int ram_soft_rst_req : 1;
        unsigned int aon_soft_rst_req : 1;
        unsigned int ddrc_soft_rst_req : 1;
        unsigned int tsen_soft_rst_req : 1;
        unsigned int pmc_soft_rst_req : 1;
        unsigned int uart_soft_rst_req : 1;
        unsigned int i2c_soft_rst_req : 1;
        unsigned int ssi_soft_rst_req : 1;
        unsigned int pcrg_soft_rst_req : 1;
        unsigned int nandc_soft_rst_req : 1;
        unsigned int wdog_soft_rst_req : 1;
        unsigned int tim_soft_rst_req : 1;
        unsigned int cssys_soft_rst_req : 1;
        unsigned int m3_lp_rst_req : 1;
        unsigned int reserved : 14;
    } reg;
} SOC_SCTRL_SCLPMCURSTDIS_UNION;
#endif
#define SOC_SCTRL_SCLPMCURSTDIS_rst_software_req_START (0)
#define SOC_SCTRL_SCLPMCURSTDIS_rst_software_req_END (0)
#define SOC_SCTRL_SCLPMCURSTDIS_coresight_soft_rst_req_START (1)
#define SOC_SCTRL_SCLPMCURSTDIS_coresight_soft_rst_req_END (1)
#define SOC_SCTRL_SCLPMCURSTDIS_ssi1_soft_rst_req_START (2)
#define SOC_SCTRL_SCLPMCURSTDIS_ssi1_soft_rst_req_END (2)
#define SOC_SCTRL_SCLPMCURSTDIS_rom_soft_rst_req_START (3)
#define SOC_SCTRL_SCLPMCURSTDIS_rom_soft_rst_req_END (3)
#define SOC_SCTRL_SCLPMCURSTDIS_ram_soft_rst_req_START (4)
#define SOC_SCTRL_SCLPMCURSTDIS_ram_soft_rst_req_END (4)
#define SOC_SCTRL_SCLPMCURSTDIS_aon_soft_rst_req_START (5)
#define SOC_SCTRL_SCLPMCURSTDIS_aon_soft_rst_req_END (5)
#define SOC_SCTRL_SCLPMCURSTDIS_ddrc_soft_rst_req_START (6)
#define SOC_SCTRL_SCLPMCURSTDIS_ddrc_soft_rst_req_END (6)
#define SOC_SCTRL_SCLPMCURSTDIS_tsen_soft_rst_req_START (7)
#define SOC_SCTRL_SCLPMCURSTDIS_tsen_soft_rst_req_END (7)
#define SOC_SCTRL_SCLPMCURSTDIS_pmc_soft_rst_req_START (8)
#define SOC_SCTRL_SCLPMCURSTDIS_pmc_soft_rst_req_END (8)
#define SOC_SCTRL_SCLPMCURSTDIS_uart_soft_rst_req_START (9)
#define SOC_SCTRL_SCLPMCURSTDIS_uart_soft_rst_req_END (9)
#define SOC_SCTRL_SCLPMCURSTDIS_i2c_soft_rst_req_START (10)
#define SOC_SCTRL_SCLPMCURSTDIS_i2c_soft_rst_req_END (10)
#define SOC_SCTRL_SCLPMCURSTDIS_ssi_soft_rst_req_START (11)
#define SOC_SCTRL_SCLPMCURSTDIS_ssi_soft_rst_req_END (11)
#define SOC_SCTRL_SCLPMCURSTDIS_pcrg_soft_rst_req_START (12)
#define SOC_SCTRL_SCLPMCURSTDIS_pcrg_soft_rst_req_END (12)
#define SOC_SCTRL_SCLPMCURSTDIS_nandc_soft_rst_req_START (13)
#define SOC_SCTRL_SCLPMCURSTDIS_nandc_soft_rst_req_END (13)
#define SOC_SCTRL_SCLPMCURSTDIS_wdog_soft_rst_req_START (14)
#define SOC_SCTRL_SCLPMCURSTDIS_wdog_soft_rst_req_END (14)
#define SOC_SCTRL_SCLPMCURSTDIS_tim_soft_rst_req_START (15)
#define SOC_SCTRL_SCLPMCURSTDIS_tim_soft_rst_req_END (15)
#define SOC_SCTRL_SCLPMCURSTDIS_cssys_soft_rst_req_START (16)
#define SOC_SCTRL_SCLPMCURSTDIS_cssys_soft_rst_req_END (16)
#define SOC_SCTRL_SCLPMCURSTDIS_m3_lp_rst_req_START (17)
#define SOC_SCTRL_SCLPMCURSTDIS_m3_lp_rst_req_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rst_software_req : 1;
        unsigned int coresight_soft_rst_req : 1;
        unsigned int ssi1_soft_rst_req : 1;
        unsigned int rom_soft_rst_req : 1;
        unsigned int ram_soft_rst_req : 1;
        unsigned int aon_soft_rst_req : 1;
        unsigned int ddrc_soft_rst_req : 1;
        unsigned int tsen_soft_rst_req : 1;
        unsigned int pmc_soft_rst_req : 1;
        unsigned int uart_soft_rst_req : 1;
        unsigned int i2c_soft_rst_req : 1;
        unsigned int ssi_soft_rst_req : 1;
        unsigned int pcrg_soft_rst_req : 1;
        unsigned int nandc_soft_rst_req : 1;
        unsigned int wdog_soft_rst_req : 1;
        unsigned int tim_soft_rst_req : 1;
        unsigned int cssys_soft_rst_req : 1;
        unsigned int m3_lp_rst_req : 1;
        unsigned int reserved : 14;
    } reg;
} SOC_SCTRL_SCLPMCURSTSTAT_UNION;
#endif
#define SOC_SCTRL_SCLPMCURSTSTAT_rst_software_req_START (0)
#define SOC_SCTRL_SCLPMCURSTSTAT_rst_software_req_END (0)
#define SOC_SCTRL_SCLPMCURSTSTAT_coresight_soft_rst_req_START (1)
#define SOC_SCTRL_SCLPMCURSTSTAT_coresight_soft_rst_req_END (1)
#define SOC_SCTRL_SCLPMCURSTSTAT_ssi1_soft_rst_req_START (2)
#define SOC_SCTRL_SCLPMCURSTSTAT_ssi1_soft_rst_req_END (2)
#define SOC_SCTRL_SCLPMCURSTSTAT_rom_soft_rst_req_START (3)
#define SOC_SCTRL_SCLPMCURSTSTAT_rom_soft_rst_req_END (3)
#define SOC_SCTRL_SCLPMCURSTSTAT_ram_soft_rst_req_START (4)
#define SOC_SCTRL_SCLPMCURSTSTAT_ram_soft_rst_req_END (4)
#define SOC_SCTRL_SCLPMCURSTSTAT_aon_soft_rst_req_START (5)
#define SOC_SCTRL_SCLPMCURSTSTAT_aon_soft_rst_req_END (5)
#define SOC_SCTRL_SCLPMCURSTSTAT_ddrc_soft_rst_req_START (6)
#define SOC_SCTRL_SCLPMCURSTSTAT_ddrc_soft_rst_req_END (6)
#define SOC_SCTRL_SCLPMCURSTSTAT_tsen_soft_rst_req_START (7)
#define SOC_SCTRL_SCLPMCURSTSTAT_tsen_soft_rst_req_END (7)
#define SOC_SCTRL_SCLPMCURSTSTAT_pmc_soft_rst_req_START (8)
#define SOC_SCTRL_SCLPMCURSTSTAT_pmc_soft_rst_req_END (8)
#define SOC_SCTRL_SCLPMCURSTSTAT_uart_soft_rst_req_START (9)
#define SOC_SCTRL_SCLPMCURSTSTAT_uart_soft_rst_req_END (9)
#define SOC_SCTRL_SCLPMCURSTSTAT_i2c_soft_rst_req_START (10)
#define SOC_SCTRL_SCLPMCURSTSTAT_i2c_soft_rst_req_END (10)
#define SOC_SCTRL_SCLPMCURSTSTAT_ssi_soft_rst_req_START (11)
#define SOC_SCTRL_SCLPMCURSTSTAT_ssi_soft_rst_req_END (11)
#define SOC_SCTRL_SCLPMCURSTSTAT_pcrg_soft_rst_req_START (12)
#define SOC_SCTRL_SCLPMCURSTSTAT_pcrg_soft_rst_req_END (12)
#define SOC_SCTRL_SCLPMCURSTSTAT_nandc_soft_rst_req_START (13)
#define SOC_SCTRL_SCLPMCURSTSTAT_nandc_soft_rst_req_END (13)
#define SOC_SCTRL_SCLPMCURSTSTAT_wdog_soft_rst_req_START (14)
#define SOC_SCTRL_SCLPMCURSTSTAT_wdog_soft_rst_req_END (14)
#define SOC_SCTRL_SCLPMCURSTSTAT_tim_soft_rst_req_START (15)
#define SOC_SCTRL_SCLPMCURSTSTAT_tim_soft_rst_req_END (15)
#define SOC_SCTRL_SCLPMCURSTSTAT_cssys_soft_rst_req_START (16)
#define SOC_SCTRL_SCLPMCURSTSTAT_cssys_soft_rst_req_END (16)
#define SOC_SCTRL_SCLPMCURSTSTAT_m3_lp_rst_req_START (17)
#define SOC_SCTRL_SCLPMCURSTSTAT_m3_lp_rst_req_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int m3_lp_gnspwm_clkoff_n : 1;
        unsigned int mpudisable : 1;
        unsigned int nmi_in : 1;
        unsigned int bus_div_clk_sel : 2;
        unsigned int sysresetreq_en : 1;
        unsigned int reserved_0 : 5;
        unsigned int lpmcuidle_bypass : 1;
        unsigned int reserved_1 : 12;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_SCTRL_SCLPMCUCTRL_UNION;
#endif
#define SOC_SCTRL_SCLPMCUCTRL_m3_lp_gnspwm_clkoff_n_START (0)
#define SOC_SCTRL_SCLPMCUCTRL_m3_lp_gnspwm_clkoff_n_END (0)
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
        unsigned int ldrx2dbg_abs_timer : 32;
    } reg;
} SOC_SCTRL_SCBBPDRXSTAT1_UNION;
#endif
#define SOC_SCTRL_SCBBPDRXSTAT1_ldrx2dbg_abs_timer_START (0)
#define SOC_SCTRL_SCBBPDRXSTAT1_ldrx2dbg_abs_timer_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ldrx2dbg_abs_timer : 32;
    } reg;
} SOC_SCTRL_SCBBPDRXSTAT2_UNION;
#endif
#define SOC_SCTRL_SCBBPDRXSTAT2_ldrx2dbg_abs_timer_START (0)
#define SOC_SCTRL_SCBBPDRXSTAT2_ldrx2dbg_abs_timer_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ldrx2dbg_abs_timer_sleep : 32;
    } reg;
} SOC_SCTRL_SCBBPDRXSTAT3_UNION;
#endif
#define SOC_SCTRL_SCBBPDRXSTAT3_ldrx2dbg_abs_timer_sleep_START (0)
#define SOC_SCTRL_SCBBPDRXSTAT3_ldrx2dbg_abs_timer_sleep_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ldrx2dbg_abs_timer_sleep : 32;
    } reg;
} SOC_SCTRL_SCBBPDRXSTAT4_UNION;
#endif
#define SOC_SCTRL_SCBBPDRXSTAT4_ldrx2dbg_abs_timer_sleep_START (0)
#define SOC_SCTRL_SCBBPDRXSTAT4_ldrx2dbg_abs_timer_sleep_END (31)
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
        unsigned int reserved : 26;
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
        unsigned int reserved : 26;
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
        unsigned int reserved : 26;
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
        unsigned int reserved : 26;
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
        unsigned int reserved : 26;
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
        unsigned int reserved : 26;
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
        unsigned int reserved : 26;
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
        unsigned int reserved : 16;
        unsigned int sciomcuclken_msk : 16;
    } reg;
} SOC_SCTRL_SCIOMCUCLKEN_UNION;
#endif
#define SOC_SCTRL_SCIOMCUCLKEN_sciomcuclken_msk_START (16)
#define SOC_SCTRL_SCIOMCUCLKEN_sciomcuclken_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCIOMCURSTEN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCIOMCURSTDIS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCIOMCURSTSTAT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int nmi_int : 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 29;
    } reg;
} SOC_SCTRL_SCIOMCUCTRL_UNION;
#endif
#define SOC_SCTRL_SCIOMCUCTRL_nmi_int_START (1)
#define SOC_SCTRL_SCIOMCUCTRL_nmi_int_END (1)
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
        unsigned int iomcu_dma_mst_nopendingtrans : 1;
        unsigned int iomcu_axi_mst_nopendingtrans : 1;
        unsigned int reserved : 1;
        unsigned int iomcu_axi_ahb_nopendingtrans : 1;
        unsigned int iomcu_stat : 12;
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
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_dma_mst_nopendingtrans_START (16)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_dma_mst_nopendingtrans_END (16)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_axi_mst_nopendingtrans_START (17)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_axi_mst_nopendingtrans_END (17)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_axi_ahb_nopendingtrans_START (19)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_axi_ahb_nopendingtrans_END (19)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_stat_START (20)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_stat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int processor_jtag_sel : 1;
        unsigned int dsp_jtag_sel : 2;
        unsigned int reserved : 29;
    } reg;
} SOC_SCTRL_SCJTAG_SEL_UNION;
#endif
#define SOC_SCTRL_SCJTAG_SEL_processor_jtag_sel_START (0)
#define SOC_SCTRL_SCJTAG_SEL_processor_jtag_sel_END (0)
#define SOC_SCTRL_SCJTAG_SEL_dsp_jtag_sel_START (1)
#define SOC_SCTRL_SCJTAG_SEL_dsp_jtag_sel_END (2)
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
        unsigned int cnt_stat : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_SCTRL_SCCLKCNTSTAT_UNION;
#endif
#define SOC_SCTRL_SCCLKCNTSTAT_cnt_stat_START (0)
#define SOC_SCTRL_SCCLKCNTSTAT_cnt_stat_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cnt_cfg : 16;
        unsigned int timer_secu_en : 1;
        unsigned int reserved : 13;
        unsigned int clk_cnt_en : 1;
        unsigned int clk_monitor_en : 1;
    } reg;
} SOC_SCTRL_SCCLKCNTCFG_UNION;
#endif
#define SOC_SCTRL_SCCLKCNTCFG_cnt_cfg_START (0)
#define SOC_SCTRL_SCCLKCNTCFG_cnt_cfg_END (15)
#define SOC_SCTRL_SCCLKCNTCFG_timer_secu_en_START (16)
#define SOC_SCTRL_SCCLKCNTCFG_timer_secu_en_END (16)
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
        unsigned int raw_intr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCCLKMONINT_UNION;
#endif
#define SOC_SCTRL_SCCLKMONINT_raw_intr_START (0)
#define SOC_SCTRL_SCCLKMONINT_raw_intr_END (0)
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
        unsigned int emmc_sel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCSECURITYSTAT_UNION;
#endif
#define SOC_SCTRL_SCSECURITYSTAT_emmc_sel_START (0)
#define SOC_SCTRL_SCSECURITYSTAT_emmc_sel_END (0)
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
        unsigned int gt_pclk_timer1 : 1;
        unsigned int gt_clk_timer1 : 1;
        unsigned int gt_clk_clkmonitor : 1;
        unsigned int gt_clk_ao_cc : 1;
        unsigned int reserved : 16;
        unsigned int mmbuf_sec_ctrl : 12;
    } reg;
} SOC_SCTRL_SCPEREN0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_timer1_START (0)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_timer1_END (0)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_timer1_START (1)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_timer1_END (1)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_clkmonitor_START (2)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_clkmonitor_END (2)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_ao_cc_START (3)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_ao_cc_END (3)
#define SOC_SCTRL_SCPEREN0_SEC_mmbuf_sec_ctrl_START (20)
#define SOC_SCTRL_SCPEREN0_SEC_mmbuf_sec_ctrl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_pclk_timer1 : 1;
        unsigned int gt_clk_timer1 : 1;
        unsigned int gt_clk_clkmonitor : 1;
        unsigned int gt_clk_ao_cc : 1;
        unsigned int reserved : 16;
        unsigned int mmbuf_sec_ctrl : 12;
    } reg;
} SOC_SCTRL_SCPERDIS0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_timer1_START (0)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_timer1_END (0)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_timer1_START (1)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_timer1_END (1)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_clkmonitor_START (2)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_clkmonitor_END (2)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_ao_cc_START (3)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_ao_cc_END (3)
#define SOC_SCTRL_SCPERDIS0_SEC_mmbuf_sec_ctrl_START (20)
#define SOC_SCTRL_SCPERDIS0_SEC_mmbuf_sec_ctrl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_pclk_timer1 : 1;
        unsigned int gt_clk_timer1 : 1;
        unsigned int gt_clk_clkmonitor : 1;
        unsigned int gt_clk_ao_cc : 1;
        unsigned int reserved : 16;
        unsigned int mmbuf_sec_ctrl : 12;
    } reg;
} SOC_SCTRL_SCPERCLKEN0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_timer1_START (0)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_timer1_END (0)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_timer1_START (1)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_timer1_END (1)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_clkmonitor_START (2)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_clkmonitor_END (2)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_ao_cc_START (3)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_ao_cc_END (3)
#define SOC_SCTRL_SCPERCLKEN0_SEC_mmbuf_sec_ctrl_START (20)
#define SOC_SCTRL_SCPERCLKEN0_SEC_mmbuf_sec_ctrl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_timer1 : 1;
        unsigned int ip_rst_asc : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_ao_cc : 1;
        unsigned int reserved_1 : 28;
    } reg;
} SOC_SCTRL_SCPERRSTEN0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_rst_timer1_START (0)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_rst_timer1_END (0)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_rst_asc_START (1)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_rst_asc_END (1)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_rst_ao_cc_START (3)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_rst_ao_cc_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_timer1 : 1;
        unsigned int ip_rst_asc : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_ao_cc : 1;
        unsigned int reserved_1 : 28;
    } reg;
} SOC_SCTRL_SCPERRSTDIS0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_rst_timer1_START (0)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_rst_timer1_END (0)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_rst_asc_START (1)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_rst_asc_END (1)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_rst_ao_cc_START (3)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_rst_ao_cc_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_timer1 : 1;
        unsigned int ip_rst_asc : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_ao_cc : 1;
        unsigned int reserved_1 : 28;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_rst_timer1_START (0)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_rst_timer1_END (0)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_rst_asc_START (1)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_rst_asc_END (1)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_rst_ao_cc_START (3)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_rst_ao_cc_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int timeren2sel : 1;
        unsigned int timeren2ov : 1;
        unsigned int timeren3sel : 1;
        unsigned int timeren3ov : 1;
        unsigned int reserved : 27;
        unsigned int timerforcehigh : 1;
    } reg;
} SOC_SCTRL_SCTIMERCTRL_SEC_UNION;
#endif
#define SOC_SCTRL_SCTIMERCTRL_SEC_timeren2sel_START (0)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timeren2sel_END (0)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timeren2ov_START (1)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timeren2ov_END (1)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timeren3sel_START (2)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timeren3sel_END (2)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timeren3ov_START (3)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timeren3ov_END (3)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timerforcehigh_START (31)
#define SOC_SCTRL_SCTIMERCTRL_SEC_timerforcehigh_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_asp_subsys : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCPERRSTEN1_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_subsys_START (0)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_subsys_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_asp_subsys : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCPERRSTDIS1_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_subsys_START (0)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_subsys_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_asp_subsys : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT1_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_subsys_START (0)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_subsys_END (0)
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
        unsigned int version_code : 16;
        unsigned int chip_code : 16;
    } reg;
} SOC_SCTRL_SCSOCID0_UNION;
#endif
#define SOC_SCTRL_SCSOCID0_version_code_START (0)
#define SOC_SCTRL_SCSOCID0_version_code_END (15)
#define SOC_SCTRL_SCSOCID0_chip_code_START (16)
#define SOC_SCTRL_SCSOCID0_chip_code_END (31)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
