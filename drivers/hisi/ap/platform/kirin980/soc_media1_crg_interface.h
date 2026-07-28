#ifndef __SOC_MEDIA1_CRG_INTERFACE_H__
#define __SOC_MEDIA1_CRG_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_MEDIA1_CRG_PEREN0_ADDR(base) ((base) + (0x000))
#define SOC_MEDIA1_CRG_PERDIS0_ADDR(base) ((base) + (0x004))
#define SOC_MEDIA1_CRG_PERCLKEN0_ADDR(base) ((base) + (0x008))
#define SOC_MEDIA1_CRG_PERSTAT0_ADDR(base) ((base) + (0x00C))
#define SOC_MEDIA1_CRG_PEREN1_ADDR(base) ((base) + (0x010))
#define SOC_MEDIA1_CRG_PERDIS1_ADDR(base) ((base) + (0x014))
#define SOC_MEDIA1_CRG_PERCLKEN1_ADDR(base) ((base) + (0x018))
#define SOC_MEDIA1_CRG_PERSTAT1_ADDR(base) ((base) + (0x01C))
#define SOC_MEDIA1_CRG_PEREN2_ADDR(base) ((base) + (0x020))
#define SOC_MEDIA1_CRG_PERDIS2_ADDR(base) ((base) + (0x024))
#define SOC_MEDIA1_CRG_PERCLKEN2_ADDR(base) ((base) + (0x028))
#define SOC_MEDIA1_CRG_PERSTAT2_ADDR(base) ((base) + (0x02C))
#define SOC_MEDIA1_CRG_PERRSTEN0_ADDR(base) ((base) + (0x030))
#define SOC_MEDIA1_CRG_PERRSTDIS0_ADDR(base) ((base) + (0x034))
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ADDR(base) ((base) + (0x038))
#define SOC_MEDIA1_CRG_PERRSTEN1_ADDR(base) ((base) + (0x03C))
#define SOC_MEDIA1_CRG_PERRSTDIS1_ADDR(base) ((base) + (0x040))
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ADDR(base) ((base) + (0x044))
#define SOC_MEDIA1_CRG_CLKDIV0_ADDR(base) ((base) + (0x060))
#define SOC_MEDIA1_CRG_CLKDIV1_ADDR(base) ((base) + (0x064))
#define SOC_MEDIA1_CRG_CLKDIV2_ADDR(base) ((base) + (0x068))
#define SOC_MEDIA1_CRG_CLKDIV3_ADDR(base) ((base) + (0x06C))
#define SOC_MEDIA1_CRG_CLKDIV4_ADDR(base) ((base) + (0x070))
#define SOC_MEDIA1_CRG_CLKDIV5_ADDR(base) ((base) + (0x074))
#define SOC_MEDIA1_CRG_CLKDIV6_ADDR(base) ((base) + (0x078))
#define SOC_MEDIA1_CRG_CLKDIV7_ADDR(base) ((base) + (0x07C))
#define SOC_MEDIA1_CRG_CLKDIV8_ADDR(base) ((base) + (0x080))
#define SOC_MEDIA1_CRG_CLKDIV9_ADDR(base) ((base) + (0x084))
#define SOC_MEDIA1_CRG_CLKDIV10_ADDR(base) ((base) + (0x088))
#define SOC_MEDIA1_CRG_CLKDIV11_ADDR(base) ((base) + (0x08C))
#define SOC_MEDIA1_CRG_CLKDIV12_ADDR(base) ((base) + (0x090))
#define SOC_MEDIA1_CRG_CLKDIV13_ADDR(base) ((base) + (0x094))
#define SOC_MEDIA1_CRG_CLKDIV14_ADDR(base) ((base) + (0x098))
#define SOC_MEDIA1_CRG_CLKDIV15_ADDR(base) ((base) + (0x09c))
#define SOC_MEDIA1_CRG_PERI_STAT0_ADDR(base) ((base) + (0x0A0))
#define SOC_MEDIA1_CRG_PERI_STAT1_ADDR(base) ((base) + (0x0A4))
#define SOC_MEDIA1_CRG_PERI_STAT2_ADDR(base) ((base) + (0x0A8))
#define SOC_MEDIA1_CRG_PERI_STAT3_ADDR(base) ((base) + (0x0AC))
#define SOC_MEDIA1_CRG_ISOEN_ADDR(base) ((base) + (0x100))
#define SOC_MEDIA1_CRG_ISODIS_ADDR(base) ((base) + (0x104))
#define SOC_MEDIA1_CRG_ISOSTAT_ADDR(base) ((base) + (0x108))
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_ADDR(base) ((base) + (0x130))
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_ADDR(base) ((base) + (0x140))
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_ADDR(base) ((base) + (0x144))
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_ADDR(base) ((base) + (0x148))
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_ADDR(base) ((base) + (0x14C))
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_ADDR(base) ((base) + (0x150))
#define SOC_MEDIA1_CRG_ISP_CPU_STATE1_ADDR(base) ((base) + (0x154))
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ADDR(base) ((base) + (0x160))
#define SOC_MEDIA1_CRG_PERI_AUTODIV1_ADDR(base) ((base) + (0x164))
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_ADDR(base) ((base) + (0x168))
#define SOC_MEDIA1_CRG_PERI_AUTODIV3_ADDR(base) ((base) + (0x16C))
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_ADDR(base) ((base) + (0x170))
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_ADDR(base) ((base) + (0x174))
#define SOC_MEDIA1_CRG_PERI_AUTODIV6_ADDR(base) ((base) + (0x178))
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_ADDR(base) ((base) + (0x190))
#define SOC_MEDIA1_CRG_MMBUF_MEM_CTRL_ADDR(base) ((base) + (0x1C0))
#define SOC_MEDIA1_CRG_ISPTOMED1_RESERVE_OUT_ADDR(base) ((base) + (0x1C4))
#define SOC_MEDIA1_CRG_INTR_NOCBUS_NONIDLE_PEND_ADDR(base) ((base) + (0x1D0))
#define SOC_MEDIA1_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_ADDR(base) ((base) + (0x1D4))
#define SOC_MEDIA1_CRG_DMSS_ASYNC_BRG_ADDR(base) ((base) + (0x1D8))
#define SOC_MEDIA1_CRG_COMMON_CTRL0_ADDR(base) ((base) + (0x200))
#define SOC_MEDIA1_CRG_COMMON_CTRL1_ADDR(base) ((base) + (0x204))
#define SOC_MEDIA1_CRG_COMMON_CTRL2_ADDR(base) ((base) + (0x208))
#define SOC_MEDIA1_CRG_PERRSTEN_ISP_SEC_ADDR(base) ((base) + (0x800))
#define SOC_MEDIA1_CRG_PERRSTDIS_ISP_SEC_ADDR(base) ((base) + (0x804))
#define SOC_MEDIA1_CRG_PERRSTSTAT_ISP_SEC_ADDR(base) ((base) + (0x808))
#define SOC_MEDIA1_CRG_ISPCPU_CTRL0_SEC_ADDR(base) ((base) + (0x810))
#define SOC_MEDIA1_CRG_ISP_ADB_LP_OUT_ADDR(base) ((base) + (0x814))
#define SOC_MEDIA1_CRG_ISP_ADB_LP_IN_ADDR(base) ((base) + (0x818))
#define SOC_MEDIA1_CRG_PERRSTEN_SEC_IVP_ADDR(base) ((base) + (0x900))
#define SOC_MEDIA1_CRG_PERRSTDIS_SEC_IVP_ADDR(base) ((base) + (0x904))
#define SOC_MEDIA1_CRG_PERRSTSTAT_SEC_IVP_ADDR(base) ((base) + (0x908))
#define SOC_MEDIA1_CRG_PERRSTEN_GENERAL_SEC_ADDR(base) ((base) + (0xA00))
#define SOC_MEDIA1_CRG_PERRSTDIS_GENERAL_SEC_ADDR(base) ((base) + (0xA04))
#define SOC_MEDIA1_CRG_PERRSTSTAT_GENERAL_SEC_ADDR(base) ((base) + (0xA08))
#define SOC_MEDIA1_CRG_DECPROT0SET_ADDR(base) ((base) + (0xF00))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 1;
        unsigned int gt_pclk_ics2 : 1;
        unsigned int gt_aclk_ics2 : 1;
        unsigned int gt_clk_ics2 : 1;
        unsigned int gt_pclk_media1_lm_lpm3 : 1;
        unsigned int gt_clk_ispfunc4 : 1;
        unsigned int gt_clk_vivobus2ddrc : 1;
        unsigned int gt_pclk_media1_lm : 1;
        unsigned int gt_clk_ivp32dsp_tcxo : 1;
        unsigned int gt_clk_isp_i2c : 1;
        unsigned int gt_clk_ivp32dsp_core_crg : 1;
        unsigned int gt_clk_isp_sys : 1;
        unsigned int gt_clk_isp_cpu : 1;
        unsigned int gt_clk_ispfunc : 1;
        unsigned int gt_pclk_dss : 1;
        unsigned int gt_clk_edc0 : 1;
        unsigned int gt_clk_ldi0 : 1;
        unsigned int gt_clk_ldi1 : 1;
        unsigned int gt_clk_vivobus : 1;
        unsigned int gt_aclk_dss : 1;
        unsigned int gt_aclk_isp : 1;
        unsigned int gt_clk_ispfunc3 : 1;
        unsigned int gt_clk_ispfunc2 : 1;
        unsigned int gt_pclk_atgs_vivo : 1;
        unsigned int gt_pclk_atgm_dss : 1;
        unsigned int gt_pclk_atgm_isp : 1;
        unsigned int gt_clk_loadmonitor : 1;
        unsigned int gt_clk_dmss_vivo : 1;
        unsigned int gt_clk_dmss_ivp : 1;
        unsigned int gt_clk_brg : 1;
        unsigned int gt_clk_isp_cputocfg : 1;
        unsigned int gt_clk_ispfunc_to_ivp : 1;
    } reg;
} SOC_MEDIA1_CRG_PEREN0_UNION;
#endif
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_ics2_START (1)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_ics2_END (1)
#define SOC_MEDIA1_CRG_PEREN0_gt_aclk_ics2_START (2)
#define SOC_MEDIA1_CRG_PEREN0_gt_aclk_ics2_END (2)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ics2_START (3)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ics2_END (3)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_media1_lm_lpm3_START (4)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_media1_lm_lpm3_END (4)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ispfunc4_START (5)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ispfunc4_END (5)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_vivobus2ddrc_START (6)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_vivobus2ddrc_END (6)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_media1_lm_START (7)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_media1_lm_END (7)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ivp32dsp_tcxo_START (8)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ivp32dsp_tcxo_END (8)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_isp_i2c_START (9)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_isp_i2c_END (9)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ivp32dsp_core_crg_START (10)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ivp32dsp_core_crg_END (10)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_isp_sys_START (11)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_isp_sys_END (11)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_isp_cpu_START (12)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_isp_cpu_END (12)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ispfunc_START (13)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ispfunc_END (13)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_dss_START (14)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_dss_END (14)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_edc0_START (15)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_edc0_END (15)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ldi0_START (16)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ldi0_END (16)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ldi1_START (17)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ldi1_END (17)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_vivobus_START (18)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_vivobus_END (18)
#define SOC_MEDIA1_CRG_PEREN0_gt_aclk_dss_START (19)
#define SOC_MEDIA1_CRG_PEREN0_gt_aclk_dss_END (19)
#define SOC_MEDIA1_CRG_PEREN0_gt_aclk_isp_START (20)
#define SOC_MEDIA1_CRG_PEREN0_gt_aclk_isp_END (20)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ispfunc3_START (21)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ispfunc3_END (21)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ispfunc2_START (22)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ispfunc2_END (22)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_atgs_vivo_START (23)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_atgs_vivo_END (23)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_atgm_dss_START (24)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_atgm_dss_END (24)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_atgm_isp_START (25)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_atgm_isp_END (25)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_loadmonitor_START (26)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_loadmonitor_END (26)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_dmss_vivo_START (27)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_dmss_vivo_END (27)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_dmss_ivp_START (28)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_dmss_ivp_END (28)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_brg_START (29)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_brg_END (29)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_isp_cputocfg_START (30)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_isp_cputocfg_END (30)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ispfunc_to_ivp_START (31)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ispfunc_to_ivp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 1;
        unsigned int gt_pclk_ics2 : 1;
        unsigned int gt_aclk_ics2 : 1;
        unsigned int gt_clk_ics2 : 1;
        unsigned int gt_pclk_media1_lm_lpm3 : 1;
        unsigned int gt_clk_ispfunc4 : 1;
        unsigned int gt_clk_vivobus2ddrc : 1;
        unsigned int gt_pclk_media1_lm : 1;
        unsigned int gt_clk_ivp32dsp_tcxo : 1;
        unsigned int gt_clk_isp_i2c : 1;
        unsigned int gt_clk_ivp32dsp_core_crg : 1;
        unsigned int gt_clk_isp_sys : 1;
        unsigned int gt_clk_isp_cpu : 1;
        unsigned int gt_clk_ispfunc : 1;
        unsigned int gt_pclk_dss : 1;
        unsigned int gt_clk_edc0 : 1;
        unsigned int gt_clk_ldi0 : 1;
        unsigned int gt_clk_ldi1 : 1;
        unsigned int gt_clk_vivobus : 1;
        unsigned int gt_aclk_dss : 1;
        unsigned int gt_aclk_isp : 1;
        unsigned int gt_clk_ispfunc3 : 1;
        unsigned int gt_clk_ispfunc2 : 1;
        unsigned int gt_pclk_atgs_vivo : 1;
        unsigned int gt_pclk_atgm_dss : 1;
        unsigned int gt_pclk_atgm_isp : 1;
        unsigned int gt_clk_loadmonitor : 1;
        unsigned int gt_clk_dmss_vivo : 1;
        unsigned int gt_clk_dmss_ivp : 1;
        unsigned int gt_clk_brg : 1;
        unsigned int gt_clk_isp_cputocfg : 1;
        unsigned int gt_clk_ispfunc_to_ivp : 1;
    } reg;
} SOC_MEDIA1_CRG_PERDIS0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_ics2_START (1)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_ics2_END (1)
#define SOC_MEDIA1_CRG_PERDIS0_gt_aclk_ics2_START (2)
#define SOC_MEDIA1_CRG_PERDIS0_gt_aclk_ics2_END (2)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ics2_START (3)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ics2_END (3)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_media1_lm_lpm3_START (4)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_media1_lm_lpm3_END (4)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ispfunc4_START (5)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ispfunc4_END (5)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_vivobus2ddrc_START (6)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_vivobus2ddrc_END (6)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_media1_lm_START (7)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_media1_lm_END (7)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ivp32dsp_tcxo_START (8)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ivp32dsp_tcxo_END (8)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_isp_i2c_START (9)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_isp_i2c_END (9)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ivp32dsp_core_crg_START (10)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ivp32dsp_core_crg_END (10)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_isp_sys_START (11)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_isp_sys_END (11)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_isp_cpu_START (12)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_isp_cpu_END (12)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ispfunc_START (13)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ispfunc_END (13)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_dss_START (14)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_dss_END (14)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_edc0_START (15)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_edc0_END (15)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ldi0_START (16)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ldi0_END (16)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ldi1_START (17)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ldi1_END (17)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_vivobus_START (18)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_vivobus_END (18)
#define SOC_MEDIA1_CRG_PERDIS0_gt_aclk_dss_START (19)
#define SOC_MEDIA1_CRG_PERDIS0_gt_aclk_dss_END (19)
#define SOC_MEDIA1_CRG_PERDIS0_gt_aclk_isp_START (20)
#define SOC_MEDIA1_CRG_PERDIS0_gt_aclk_isp_END (20)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ispfunc3_START (21)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ispfunc3_END (21)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ispfunc2_START (22)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ispfunc2_END (22)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_atgs_vivo_START (23)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_atgs_vivo_END (23)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_atgm_dss_START (24)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_atgm_dss_END (24)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_atgm_isp_START (25)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_atgm_isp_END (25)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_loadmonitor_START (26)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_loadmonitor_END (26)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_dmss_vivo_START (27)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_dmss_vivo_END (27)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_dmss_ivp_START (28)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_dmss_ivp_END (28)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_brg_START (29)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_brg_END (29)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_isp_cputocfg_START (30)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_isp_cputocfg_END (30)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ispfunc_to_ivp_START (31)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ispfunc_to_ivp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 1;
        unsigned int gt_pclk_ics2 : 1;
        unsigned int gt_aclk_ics2 : 1;
        unsigned int gt_clk_ics2 : 1;
        unsigned int gt_pclk_media1_lm_lpm3 : 1;
        unsigned int gt_clk_ispfunc4 : 1;
        unsigned int gt_clk_vivobus2ddrc : 1;
        unsigned int gt_pclk_media1_lm : 1;
        unsigned int gt_clk_ivp32dsp_tcxo : 1;
        unsigned int gt_clk_isp_i2c : 1;
        unsigned int gt_clk_ivp32dsp_core_crg : 1;
        unsigned int gt_clk_isp_sys : 1;
        unsigned int gt_clk_isp_cpu : 1;
        unsigned int gt_clk_ispfunc : 1;
        unsigned int gt_pclk_dss : 1;
        unsigned int gt_clk_edc0 : 1;
        unsigned int gt_clk_ldi0 : 1;
        unsigned int gt_clk_ldi1 : 1;
        unsigned int gt_clk_vivobus : 1;
        unsigned int gt_aclk_dss : 1;
        unsigned int gt_aclk_isp : 1;
        unsigned int gt_clk_ispfunc3 : 1;
        unsigned int gt_clk_ispfunc2 : 1;
        unsigned int gt_pclk_atgs_vivo : 1;
        unsigned int gt_pclk_atgm_dss : 1;
        unsigned int gt_pclk_atgm_isp : 1;
        unsigned int gt_clk_loadmonitor : 1;
        unsigned int gt_clk_dmss_vivo : 1;
        unsigned int gt_clk_dmss_ivp : 1;
        unsigned int gt_clk_brg : 1;
        unsigned int gt_clk_isp_cputocfg : 1;
        unsigned int gt_clk_ispfunc_to_ivp : 1;
    } reg;
} SOC_MEDIA1_CRG_PERCLKEN0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_ics2_START (1)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_ics2_END (1)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_aclk_ics2_START (2)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_aclk_ics2_END (2)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ics2_START (3)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ics2_END (3)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_media1_lm_lpm3_START (4)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_media1_lm_lpm3_END (4)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ispfunc4_START (5)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ispfunc4_END (5)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_vivobus2ddrc_START (6)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_vivobus2ddrc_END (6)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_media1_lm_START (7)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_media1_lm_END (7)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ivp32dsp_tcxo_START (8)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ivp32dsp_tcxo_END (8)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_isp_i2c_START (9)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_isp_i2c_END (9)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ivp32dsp_core_crg_START (10)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ivp32dsp_core_crg_END (10)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_isp_sys_START (11)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_isp_sys_END (11)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_isp_cpu_START (12)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_isp_cpu_END (12)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ispfunc_START (13)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ispfunc_END (13)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_dss_START (14)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_dss_END (14)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_edc0_START (15)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_edc0_END (15)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ldi0_START (16)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ldi0_END (16)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ldi1_START (17)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ldi1_END (17)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_vivobus_START (18)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_vivobus_END (18)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_aclk_dss_START (19)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_aclk_dss_END (19)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_aclk_isp_START (20)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_aclk_isp_END (20)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ispfunc3_START (21)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ispfunc3_END (21)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ispfunc2_START (22)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ispfunc2_END (22)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_atgs_vivo_START (23)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_atgs_vivo_END (23)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_atgm_dss_START (24)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_atgm_dss_END (24)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_atgm_isp_START (25)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_atgm_isp_END (25)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_loadmonitor_START (26)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_loadmonitor_END (26)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_dmss_vivo_START (27)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_dmss_vivo_END (27)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_dmss_ivp_START (28)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_dmss_ivp_END (28)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_brg_START (29)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_brg_END (29)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_isp_cputocfg_START (30)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_isp_cputocfg_END (30)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ispfunc_to_ivp_START (31)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ispfunc_to_ivp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 1;
        unsigned int st_pclk_ics2 : 1;
        unsigned int st_aclk_ics2 : 1;
        unsigned int st_clk_ics2 : 1;
        unsigned int st_pclk_media1_lm_lpm3 : 1;
        unsigned int st_clk_ispfunc4 : 1;
        unsigned int st_clk_vivobus2ddrc : 1;
        unsigned int st_pclk_media1_lm : 1;
        unsigned int st_clk_ivp32dsp_tcxo : 1;
        unsigned int st_clk_isp_i2c : 1;
        unsigned int st_clk_ivp32dsp_core_crg : 1;
        unsigned int st_clk_isp_sys : 1;
        unsigned int st_clk_isp_cpu : 1;
        unsigned int st_clk_ispfunc : 1;
        unsigned int st_pclk_dss : 1;
        unsigned int st_clk_edc0 : 1;
        unsigned int st_clk_ldi0 : 1;
        unsigned int st_clk_ldi1 : 1;
        unsigned int st_clk_vivobus : 1;
        unsigned int st_aclk_dss : 1;
        unsigned int st_aclk_isp : 1;
        unsigned int st_clk_ispfunc3 : 1;
        unsigned int st_clk_ispfunc2 : 1;
        unsigned int st_pclk_atgs_vivo : 1;
        unsigned int st_pclk_atgm_dss : 1;
        unsigned int st_pclk_atgm_isp : 1;
        unsigned int st_clk_loadmonitor : 1;
        unsigned int st_clk_dmss_vivo : 1;
        unsigned int st_clk_dmss_ivp : 1;
        unsigned int st_clk_brg : 1;
        unsigned int st_clk_isp_cputocfg : 1;
        unsigned int st_clk_ispfunc_to_ivp : 1;
    } reg;
} SOC_MEDIA1_CRG_PERSTAT0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_ics2_START (1)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_ics2_END (1)
#define SOC_MEDIA1_CRG_PERSTAT0_st_aclk_ics2_START (2)
#define SOC_MEDIA1_CRG_PERSTAT0_st_aclk_ics2_END (2)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ics2_START (3)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ics2_END (3)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_media1_lm_lpm3_START (4)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_media1_lm_lpm3_END (4)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ispfunc4_START (5)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ispfunc4_END (5)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_vivobus2ddrc_START (6)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_vivobus2ddrc_END (6)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_media1_lm_START (7)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_media1_lm_END (7)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ivp32dsp_tcxo_START (8)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ivp32dsp_tcxo_END (8)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_isp_i2c_START (9)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_isp_i2c_END (9)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ivp32dsp_core_crg_START (10)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ivp32dsp_core_crg_END (10)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_isp_sys_START (11)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_isp_sys_END (11)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_isp_cpu_START (12)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_isp_cpu_END (12)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ispfunc_START (13)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ispfunc_END (13)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_dss_START (14)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_dss_END (14)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_edc0_START (15)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_edc0_END (15)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ldi0_START (16)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ldi0_END (16)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ldi1_START (17)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ldi1_END (17)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_vivobus_START (18)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_vivobus_END (18)
#define SOC_MEDIA1_CRG_PERSTAT0_st_aclk_dss_START (19)
#define SOC_MEDIA1_CRG_PERSTAT0_st_aclk_dss_END (19)
#define SOC_MEDIA1_CRG_PERSTAT0_st_aclk_isp_START (20)
#define SOC_MEDIA1_CRG_PERSTAT0_st_aclk_isp_END (20)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ispfunc3_START (21)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ispfunc3_END (21)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ispfunc2_START (22)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ispfunc2_END (22)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_atgs_vivo_START (23)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_atgs_vivo_END (23)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_atgm_dss_START (24)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_atgm_dss_END (24)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_atgm_isp_START (25)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_atgm_isp_END (25)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_loadmonitor_START (26)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_loadmonitor_END (26)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_dmss_vivo_START (27)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_dmss_vivo_END (27)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_dmss_ivp_START (28)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_dmss_ivp_END (28)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_brg_START (29)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_brg_END (29)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_isp_cputocfg_START (30)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_isp_cputocfg_END (30)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ispfunc_to_ivp_START (31)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ispfunc_to_ivp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_pclk_autodiv_ics2 : 1;
        unsigned int gt_pclk_autodiv_vivobus : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_autodiv_ispcpu : 1;
        unsigned int gt_pclk_autodiv_ivp : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_fdai_func : 1;
        unsigned int gt_clk_noc_ivp32_cfg : 1;
        unsigned int gt_pclk_atgm_ivp : 1;
        unsigned int gt_clk_media_common : 1;
        unsigned int gt_clk_media_common_core : 1;
        unsigned int gt_clk_media_common_p2p_rx : 1;
        unsigned int gt_clk_jpg_func : 1;
        unsigned int gt_clk_fd_func : 1;
        unsigned int gt_aclk_disp_noc_subsys : 1;
        unsigned int gt_pclk_disp_noc_subsys : 1;
        unsigned int gt_aclk_isp_noc_subsys : 1;
        unsigned int gt_pclk_isp_noc_subsys : 1;
        unsigned int gt_aclk_noc_dss : 1;
        unsigned int gt_pclk_noc_dss : 1;
        unsigned int gt_aclk_media_common : 1;
        unsigned int gt_pclk_media_common : 1;
        unsigned int gt_aclk_noc_isp : 1;
        unsigned int gt_pclk_noc_isp : 1;
        unsigned int gt_aclk_jpg : 1;
        unsigned int gt_pclk_jpg : 1;
        unsigned int reserved_5 : 3;
    } reg;
} SOC_MEDIA1_CRG_PEREN1_UNION;
#endif
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_autodiv_ics2_START (0)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_autodiv_ics2_END (0)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_autodiv_vivobus_START (1)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_autodiv_vivobus_END (1)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_autodiv_ispcpu_START (4)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_autodiv_ispcpu_END (4)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_autodiv_ivp_START (5)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_autodiv_ivp_END (5)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_fdai_func_START (9)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_fdai_func_END (9)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_noc_ivp32_cfg_START (10)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_noc_ivp32_cfg_END (10)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_atgm_ivp_START (11)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_atgm_ivp_END (11)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_media_common_START (12)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_media_common_END (12)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_media_common_core_START (13)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_media_common_core_END (13)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_media_common_p2p_rx_START (14)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_media_common_p2p_rx_END (14)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_jpg_func_START (15)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_jpg_func_END (15)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_fd_func_START (16)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_fd_func_END (16)
#define SOC_MEDIA1_CRG_PEREN1_gt_aclk_disp_noc_subsys_START (17)
#define SOC_MEDIA1_CRG_PEREN1_gt_aclk_disp_noc_subsys_END (17)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_disp_noc_subsys_START (18)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_disp_noc_subsys_END (18)
#define SOC_MEDIA1_CRG_PEREN1_gt_aclk_isp_noc_subsys_START (19)
#define SOC_MEDIA1_CRG_PEREN1_gt_aclk_isp_noc_subsys_END (19)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_isp_noc_subsys_START (20)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_isp_noc_subsys_END (20)
#define SOC_MEDIA1_CRG_PEREN1_gt_aclk_noc_dss_START (21)
#define SOC_MEDIA1_CRG_PEREN1_gt_aclk_noc_dss_END (21)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_noc_dss_START (22)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_noc_dss_END (22)
#define SOC_MEDIA1_CRG_PEREN1_gt_aclk_media_common_START (23)
#define SOC_MEDIA1_CRG_PEREN1_gt_aclk_media_common_END (23)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_media_common_START (24)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_media_common_END (24)
#define SOC_MEDIA1_CRG_PEREN1_gt_aclk_noc_isp_START (25)
#define SOC_MEDIA1_CRG_PEREN1_gt_aclk_noc_isp_END (25)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_noc_isp_START (26)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_noc_isp_END (26)
#define SOC_MEDIA1_CRG_PEREN1_gt_aclk_jpg_START (27)
#define SOC_MEDIA1_CRG_PEREN1_gt_aclk_jpg_END (27)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_jpg_START (28)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_jpg_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_pclk_autodiv_ics2 : 1;
        unsigned int gt_pclk_autodiv_vivobus : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_autodiv_ispcpu : 1;
        unsigned int gt_pclk_autodiv_ivp : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_fdai_func : 1;
        unsigned int gt_clk_noc_ivp32_cfg : 1;
        unsigned int gt_pclk_atgm_ivp : 1;
        unsigned int gt_clk_media_common : 1;
        unsigned int gt_clk_media_common_core : 1;
        unsigned int gt_clk_media_common_p2p_rx : 1;
        unsigned int gt_clk_jpg_func : 1;
        unsigned int gt_clk_fd_func : 1;
        unsigned int gt_aclk_disp_noc_subsys : 1;
        unsigned int gt_pclk_disp_noc_subsys : 1;
        unsigned int gt_aclk_isp_noc_subsys : 1;
        unsigned int gt_pclk_isp_noc_subsys : 1;
        unsigned int gt_aclk_noc_dss : 1;
        unsigned int gt_pclk_noc_dss : 1;
        unsigned int gt_aclk_media_common : 1;
        unsigned int gt_pclk_media_common : 1;
        unsigned int gt_aclk_noc_isp : 1;
        unsigned int gt_pclk_noc_isp : 1;
        unsigned int gt_aclk_jpg : 1;
        unsigned int gt_pclk_jpg : 1;
        unsigned int reserved_5 : 3;
    } reg;
} SOC_MEDIA1_CRG_PERDIS1_UNION;
#endif
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_autodiv_ics2_START (0)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_autodiv_ics2_END (0)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_autodiv_vivobus_START (1)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_autodiv_vivobus_END (1)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_autodiv_ispcpu_START (4)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_autodiv_ispcpu_END (4)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_autodiv_ivp_START (5)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_autodiv_ivp_END (5)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_fdai_func_START (9)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_fdai_func_END (9)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_noc_ivp32_cfg_START (10)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_noc_ivp32_cfg_END (10)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_atgm_ivp_START (11)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_atgm_ivp_END (11)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_media_common_START (12)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_media_common_END (12)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_media_common_core_START (13)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_media_common_core_END (13)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_media_common_p2p_rx_START (14)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_media_common_p2p_rx_END (14)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_jpg_func_START (15)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_jpg_func_END (15)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_fd_func_START (16)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_fd_func_END (16)
#define SOC_MEDIA1_CRG_PERDIS1_gt_aclk_disp_noc_subsys_START (17)
#define SOC_MEDIA1_CRG_PERDIS1_gt_aclk_disp_noc_subsys_END (17)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_disp_noc_subsys_START (18)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_disp_noc_subsys_END (18)
#define SOC_MEDIA1_CRG_PERDIS1_gt_aclk_isp_noc_subsys_START (19)
#define SOC_MEDIA1_CRG_PERDIS1_gt_aclk_isp_noc_subsys_END (19)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_isp_noc_subsys_START (20)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_isp_noc_subsys_END (20)
#define SOC_MEDIA1_CRG_PERDIS1_gt_aclk_noc_dss_START (21)
#define SOC_MEDIA1_CRG_PERDIS1_gt_aclk_noc_dss_END (21)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_noc_dss_START (22)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_noc_dss_END (22)
#define SOC_MEDIA1_CRG_PERDIS1_gt_aclk_media_common_START (23)
#define SOC_MEDIA1_CRG_PERDIS1_gt_aclk_media_common_END (23)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_media_common_START (24)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_media_common_END (24)
#define SOC_MEDIA1_CRG_PERDIS1_gt_aclk_noc_isp_START (25)
#define SOC_MEDIA1_CRG_PERDIS1_gt_aclk_noc_isp_END (25)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_noc_isp_START (26)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_noc_isp_END (26)
#define SOC_MEDIA1_CRG_PERDIS1_gt_aclk_jpg_START (27)
#define SOC_MEDIA1_CRG_PERDIS1_gt_aclk_jpg_END (27)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_jpg_START (28)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_jpg_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_pclk_autodiv_ics2 : 1;
        unsigned int gt_pclk_autodiv_vivobus : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_autodiv_ispcpu : 1;
        unsigned int gt_pclk_autodiv_ivp : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_fdai_func : 1;
        unsigned int gt_clk_noc_ivp32_cfg : 1;
        unsigned int gt_pclk_atgm_ivp : 1;
        unsigned int gt_clk_media_common : 1;
        unsigned int gt_clk_media_common_core : 1;
        unsigned int gt_clk_media_common_p2p_rx : 1;
        unsigned int gt_clk_jpg_func : 1;
        unsigned int gt_clk_fd_func : 1;
        unsigned int gt_aclk_disp_noc_subsys : 1;
        unsigned int gt_pclk_disp_noc_subsys : 1;
        unsigned int gt_aclk_isp_noc_subsys : 1;
        unsigned int gt_pclk_isp_noc_subsys : 1;
        unsigned int gt_aclk_noc_dss : 1;
        unsigned int gt_pclk_noc_dss : 1;
        unsigned int gt_aclk_media_common : 1;
        unsigned int gt_pclk_media_common : 1;
        unsigned int gt_aclk_noc_isp : 1;
        unsigned int gt_pclk_noc_isp : 1;
        unsigned int gt_aclk_jpg : 1;
        unsigned int gt_pclk_jpg : 1;
        unsigned int reserved_5 : 3;
    } reg;
} SOC_MEDIA1_CRG_PERCLKEN1_UNION;
#endif
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_autodiv_ics2_START (0)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_autodiv_ics2_END (0)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_autodiv_vivobus_START (1)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_autodiv_vivobus_END (1)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_autodiv_ispcpu_START (4)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_autodiv_ispcpu_END (4)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_autodiv_ivp_START (5)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_autodiv_ivp_END (5)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_fdai_func_START (9)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_fdai_func_END (9)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_noc_ivp32_cfg_START (10)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_noc_ivp32_cfg_END (10)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_atgm_ivp_START (11)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_atgm_ivp_END (11)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_media_common_START (12)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_media_common_END (12)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_media_common_core_START (13)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_media_common_core_END (13)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_media_common_p2p_rx_START (14)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_media_common_p2p_rx_END (14)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_jpg_func_START (15)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_jpg_func_END (15)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_fd_func_START (16)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_fd_func_END (16)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_aclk_disp_noc_subsys_START (17)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_aclk_disp_noc_subsys_END (17)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_disp_noc_subsys_START (18)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_disp_noc_subsys_END (18)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_aclk_isp_noc_subsys_START (19)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_aclk_isp_noc_subsys_END (19)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_isp_noc_subsys_START (20)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_isp_noc_subsys_END (20)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_aclk_noc_dss_START (21)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_aclk_noc_dss_END (21)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_noc_dss_START (22)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_noc_dss_END (22)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_aclk_media_common_START (23)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_aclk_media_common_END (23)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_media_common_START (24)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_media_common_END (24)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_aclk_noc_isp_START (25)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_aclk_noc_isp_END (25)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_noc_isp_START (26)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_noc_isp_END (26)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_aclk_jpg_START (27)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_aclk_jpg_END (27)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_jpg_START (28)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_jpg_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int st_pclk_autodiv_ics2 : 1;
        unsigned int st_pclk_autodiv_vivobus : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int st_pclk_autodiv_ispcpu : 1;
        unsigned int st_pclk_autodiv_ivp : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int st_clk_fdai_func : 1;
        unsigned int st_clk_noc_ivp32_cfg : 1;
        unsigned int st_pclk_atgm_ivp : 1;
        unsigned int st_clk_media_common : 1;
        unsigned int st_clk_media_common_core : 1;
        unsigned int st_clk_media_common_p2p_rx : 1;
        unsigned int st_clk_jpg_func : 1;
        unsigned int st_clk_fd_func : 1;
        unsigned int st_aclk_disp_noc_subsys : 1;
        unsigned int st_pclk_disp_noc_subsys : 1;
        unsigned int st_aclk_isp_noc_subsys : 1;
        unsigned int st_pclk_isp_noc_subsys : 1;
        unsigned int st_aclk_noc_dss : 1;
        unsigned int st_pclk_noc_dss : 1;
        unsigned int st_aclk_media_common : 1;
        unsigned int st_pclk_media_common : 1;
        unsigned int st_aclk_noc_isp : 1;
        unsigned int st_pclk_noc_isp : 1;
        unsigned int st_aclk_jpg : 1;
        unsigned int st_pclk_jpg : 1;
        unsigned int reserved_5 : 3;
    } reg;
} SOC_MEDIA1_CRG_PERSTAT1_UNION;
#endif
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_autodiv_ics2_START (0)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_autodiv_ics2_END (0)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_autodiv_vivobus_START (1)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_autodiv_vivobus_END (1)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_autodiv_ispcpu_START (4)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_autodiv_ispcpu_END (4)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_autodiv_ivp_START (5)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_autodiv_ivp_END (5)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_fdai_func_START (9)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_fdai_func_END (9)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_noc_ivp32_cfg_START (10)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_noc_ivp32_cfg_END (10)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_atgm_ivp_START (11)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_atgm_ivp_END (11)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_media_common_START (12)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_media_common_END (12)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_media_common_core_START (13)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_media_common_core_END (13)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_media_common_p2p_rx_START (14)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_media_common_p2p_rx_END (14)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_jpg_func_START (15)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_jpg_func_END (15)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_fd_func_START (16)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_fd_func_END (16)
#define SOC_MEDIA1_CRG_PERSTAT1_st_aclk_disp_noc_subsys_START (17)
#define SOC_MEDIA1_CRG_PERSTAT1_st_aclk_disp_noc_subsys_END (17)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_disp_noc_subsys_START (18)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_disp_noc_subsys_END (18)
#define SOC_MEDIA1_CRG_PERSTAT1_st_aclk_isp_noc_subsys_START (19)
#define SOC_MEDIA1_CRG_PERSTAT1_st_aclk_isp_noc_subsys_END (19)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_isp_noc_subsys_START (20)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_isp_noc_subsys_END (20)
#define SOC_MEDIA1_CRG_PERSTAT1_st_aclk_noc_dss_START (21)
#define SOC_MEDIA1_CRG_PERSTAT1_st_aclk_noc_dss_END (21)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_noc_dss_START (22)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_noc_dss_END (22)
#define SOC_MEDIA1_CRG_PERSTAT1_st_aclk_media_common_START (23)
#define SOC_MEDIA1_CRG_PERSTAT1_st_aclk_media_common_END (23)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_media_common_START (24)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_media_common_END (24)
#define SOC_MEDIA1_CRG_PERSTAT1_st_aclk_noc_isp_START (25)
#define SOC_MEDIA1_CRG_PERSTAT1_st_aclk_noc_isp_END (25)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_noc_isp_START (26)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_noc_isp_END (26)
#define SOC_MEDIA1_CRG_PERSTAT1_st_aclk_jpg_START (27)
#define SOC_MEDIA1_CRG_PERSTAT1_st_aclk_jpg_END (27)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_jpg_START (28)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_jpg_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_mmbuf : 1;
        unsigned int gt_pclk_mmbuf : 1;
        unsigned int gt_aclk_mmbuf : 1;
        unsigned int gt_aclk_asc : 1;
        unsigned int gt_clk_dss_axi_mm : 1;
        unsigned int gt_pclk_mmbuf_cfg : 1;
        unsigned int gt_clk_ivp32_to_x2x : 1;
        unsigned int gt_clk_ivp32dsp_core_ini : 1;
        unsigned int gt_clk_ivp32dsp_peri_ini : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_MEDIA1_CRG_PEREN2_UNION;
#endif
#define SOC_MEDIA1_CRG_PEREN2_gt_clk_mmbuf_START (0)
#define SOC_MEDIA1_CRG_PEREN2_gt_clk_mmbuf_END (0)
#define SOC_MEDIA1_CRG_PEREN2_gt_pclk_mmbuf_START (1)
#define SOC_MEDIA1_CRG_PEREN2_gt_pclk_mmbuf_END (1)
#define SOC_MEDIA1_CRG_PEREN2_gt_aclk_mmbuf_START (2)
#define SOC_MEDIA1_CRG_PEREN2_gt_aclk_mmbuf_END (2)
#define SOC_MEDIA1_CRG_PEREN2_gt_aclk_asc_START (3)
#define SOC_MEDIA1_CRG_PEREN2_gt_aclk_asc_END (3)
#define SOC_MEDIA1_CRG_PEREN2_gt_clk_dss_axi_mm_START (4)
#define SOC_MEDIA1_CRG_PEREN2_gt_clk_dss_axi_mm_END (4)
#define SOC_MEDIA1_CRG_PEREN2_gt_pclk_mmbuf_cfg_START (5)
#define SOC_MEDIA1_CRG_PEREN2_gt_pclk_mmbuf_cfg_END (5)
#define SOC_MEDIA1_CRG_PEREN2_gt_clk_ivp32_to_x2x_START (6)
#define SOC_MEDIA1_CRG_PEREN2_gt_clk_ivp32_to_x2x_END (6)
#define SOC_MEDIA1_CRG_PEREN2_gt_clk_ivp32dsp_core_ini_START (7)
#define SOC_MEDIA1_CRG_PEREN2_gt_clk_ivp32dsp_core_ini_END (7)
#define SOC_MEDIA1_CRG_PEREN2_gt_clk_ivp32dsp_peri_ini_START (8)
#define SOC_MEDIA1_CRG_PEREN2_gt_clk_ivp32dsp_peri_ini_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_mmbuf : 1;
        unsigned int gt_pclk_mmbuf : 1;
        unsigned int gt_aclk_mmbuf : 1;
        unsigned int gt_aclk_asc : 1;
        unsigned int gt_clk_dss_axi_mm : 1;
        unsigned int gt_pclk_mmbuf_cfg : 1;
        unsigned int gt_clk_ivp32_to_x2x : 1;
        unsigned int gt_clk_ivp32dsp_core_ini : 1;
        unsigned int gt_clk_ivp32dsp_peri_ini : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_MEDIA1_CRG_PERDIS2_UNION;
#endif
#define SOC_MEDIA1_CRG_PERDIS2_gt_clk_mmbuf_START (0)
#define SOC_MEDIA1_CRG_PERDIS2_gt_clk_mmbuf_END (0)
#define SOC_MEDIA1_CRG_PERDIS2_gt_pclk_mmbuf_START (1)
#define SOC_MEDIA1_CRG_PERDIS2_gt_pclk_mmbuf_END (1)
#define SOC_MEDIA1_CRG_PERDIS2_gt_aclk_mmbuf_START (2)
#define SOC_MEDIA1_CRG_PERDIS2_gt_aclk_mmbuf_END (2)
#define SOC_MEDIA1_CRG_PERDIS2_gt_aclk_asc_START (3)
#define SOC_MEDIA1_CRG_PERDIS2_gt_aclk_asc_END (3)
#define SOC_MEDIA1_CRG_PERDIS2_gt_clk_dss_axi_mm_START (4)
#define SOC_MEDIA1_CRG_PERDIS2_gt_clk_dss_axi_mm_END (4)
#define SOC_MEDIA1_CRG_PERDIS2_gt_pclk_mmbuf_cfg_START (5)
#define SOC_MEDIA1_CRG_PERDIS2_gt_pclk_mmbuf_cfg_END (5)
#define SOC_MEDIA1_CRG_PERDIS2_gt_clk_ivp32_to_x2x_START (6)
#define SOC_MEDIA1_CRG_PERDIS2_gt_clk_ivp32_to_x2x_END (6)
#define SOC_MEDIA1_CRG_PERDIS2_gt_clk_ivp32dsp_core_ini_START (7)
#define SOC_MEDIA1_CRG_PERDIS2_gt_clk_ivp32dsp_core_ini_END (7)
#define SOC_MEDIA1_CRG_PERDIS2_gt_clk_ivp32dsp_peri_ini_START (8)
#define SOC_MEDIA1_CRG_PERDIS2_gt_clk_ivp32dsp_peri_ini_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_mmbuf : 1;
        unsigned int gt_pclk_mmbuf : 1;
        unsigned int gt_aclk_mmbuf : 1;
        unsigned int gt_aclk_asc : 1;
        unsigned int gt_clk_dss_axi_mm : 1;
        unsigned int gt_pclk_mmbuf_cfg : 1;
        unsigned int gt_clk_ivp32_to_x2x : 1;
        unsigned int gt_clk_ivp32dsp_core_ini : 1;
        unsigned int gt_clk_ivp32dsp_peri_ini : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_MEDIA1_CRG_PERCLKEN2_UNION;
#endif
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_clk_mmbuf_START (0)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_clk_mmbuf_END (0)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_pclk_mmbuf_START (1)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_pclk_mmbuf_END (1)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_aclk_mmbuf_START (2)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_aclk_mmbuf_END (2)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_aclk_asc_START (3)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_aclk_asc_END (3)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_clk_dss_axi_mm_START (4)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_clk_dss_axi_mm_END (4)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_pclk_mmbuf_cfg_START (5)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_pclk_mmbuf_cfg_END (5)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_clk_ivp32_to_x2x_START (6)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_clk_ivp32_to_x2x_END (6)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_clk_ivp32dsp_core_ini_START (7)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_clk_ivp32dsp_core_ini_END (7)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_clk_ivp32dsp_peri_ini_START (8)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_clk_ivp32dsp_peri_ini_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int st_clk_mmbuf : 1;
        unsigned int st_pclk_mmbuf : 1;
        unsigned int st_aclk_mmbuf : 1;
        unsigned int st_aclk_asc : 1;
        unsigned int st_clk_dss_axi_mm : 1;
        unsigned int st_pclk_mmbuf_cfg : 1;
        unsigned int st_clk_ivp32_to_x2x : 1;
        unsigned int st_clk_ivp32dsp_core_ini : 1;
        unsigned int st_clk_ivp32dsp_peri_ini : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_MEDIA1_CRG_PERSTAT2_UNION;
#endif
#define SOC_MEDIA1_CRG_PERSTAT2_st_clk_mmbuf_START (0)
#define SOC_MEDIA1_CRG_PERSTAT2_st_clk_mmbuf_END (0)
#define SOC_MEDIA1_CRG_PERSTAT2_st_pclk_mmbuf_START (1)
#define SOC_MEDIA1_CRG_PERSTAT2_st_pclk_mmbuf_END (1)
#define SOC_MEDIA1_CRG_PERSTAT2_st_aclk_mmbuf_START (2)
#define SOC_MEDIA1_CRG_PERSTAT2_st_aclk_mmbuf_END (2)
#define SOC_MEDIA1_CRG_PERSTAT2_st_aclk_asc_START (3)
#define SOC_MEDIA1_CRG_PERSTAT2_st_aclk_asc_END (3)
#define SOC_MEDIA1_CRG_PERSTAT2_st_clk_dss_axi_mm_START (4)
#define SOC_MEDIA1_CRG_PERSTAT2_st_clk_dss_axi_mm_END (4)
#define SOC_MEDIA1_CRG_PERSTAT2_st_pclk_mmbuf_cfg_START (5)
#define SOC_MEDIA1_CRG_PERSTAT2_st_pclk_mmbuf_cfg_END (5)
#define SOC_MEDIA1_CRG_PERSTAT2_st_clk_ivp32_to_x2x_START (6)
#define SOC_MEDIA1_CRG_PERSTAT2_st_clk_ivp32_to_x2x_END (6)
#define SOC_MEDIA1_CRG_PERSTAT2_st_clk_ivp32dsp_core_ini_START (7)
#define SOC_MEDIA1_CRG_PERSTAT2_st_clk_ivp32dsp_core_ini_END (7)
#define SOC_MEDIA1_CRG_PERSTAT2_st_clk_ivp32dsp_peri_ini_START (8)
#define SOC_MEDIA1_CRG_PERSTAT2_st_clk_ivp32dsp_peri_ini_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_ics2_core : 1;
        unsigned int ip_prst_ics2 : 1;
        unsigned int ip_arst_ics2 : 1;
        unsigned int ip_rst_ics2 : 1;
        unsigned int ip_rst_dss : 1;
        unsigned int ip_prst_dss : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_prst_atgm_dss : 1;
        unsigned int ip_prst_atgm_isp : 1;
        unsigned int ip_rst_trace : 1;
        unsigned int reserved_3 : 1;
        unsigned int ip_prst_media1_lm : 1;
        unsigned int ip_rst_brg : 1;
        unsigned int ip_rst_fdai_func : 1;
        unsigned int ip_prst_atgs_vivo : 1;
        unsigned int ip_prst_atgm_ivp : 1;
        unsigned int ip_rst_media_common : 1;
        unsigned int ip_prst_media_common : 1;
        unsigned int ip_arst_media_common : 1;
        unsigned int ip_arst_jpg : 1;
        unsigned int ip_prst_jpg : 1;
        unsigned int ip_rst_jpg_func : 1;
        unsigned int ip_rst_fd_func : 1;
        unsigned int ip_rst_disp_subsys_cfg : 1;
        unsigned int ip_rst_isp_subsys_cfg : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_MEDIA1_CRG_PERRSTEN0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_ics2_core_START (2)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_ics2_core_END (2)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_ics2_START (3)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_ics2_END (3)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_arst_ics2_START (4)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_arst_ics2_END (4)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_ics2_START (5)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_ics2_END (5)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_dss_START (6)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_dss_END (6)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_dss_START (7)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_dss_END (7)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_atgm_dss_START (9)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_atgm_dss_END (9)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_atgm_isp_START (10)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_atgm_isp_END (10)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_trace_START (11)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_trace_END (11)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_media1_lm_START (13)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_media1_lm_END (13)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_brg_START (14)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_brg_END (14)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_fdai_func_START (15)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_fdai_func_END (15)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_atgs_vivo_START (16)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_atgs_vivo_END (16)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_atgm_ivp_START (17)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_atgm_ivp_END (17)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_media_common_START (18)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_media_common_END (18)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_media_common_START (19)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_media_common_END (19)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_arst_media_common_START (20)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_arst_media_common_END (20)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_arst_jpg_START (21)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_arst_jpg_END (21)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_jpg_START (22)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_jpg_END (22)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_jpg_func_START (23)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_jpg_func_END (23)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_fd_func_START (24)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_fd_func_END (24)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_disp_subsys_cfg_START (25)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_disp_subsys_cfg_END (25)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_isp_subsys_cfg_START (26)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_isp_subsys_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_ics2_core : 1;
        unsigned int ip_prst_ics2 : 1;
        unsigned int ip_arst_ics2 : 1;
        unsigned int ip_rst_ics2 : 1;
        unsigned int ip_rst_dss : 1;
        unsigned int ip_prst_dss : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_prst_atgm_dss : 1;
        unsigned int ip_prst_atgm_isp : 1;
        unsigned int ip_rst_trace : 1;
        unsigned int reserved_3 : 1;
        unsigned int ip_prst_media1_lm : 1;
        unsigned int ip_rst_brg : 1;
        unsigned int ip_rst_fdai_func : 1;
        unsigned int ip_prst_atgs_vivo : 1;
        unsigned int ip_prst_atgm_ivp : 1;
        unsigned int ip_rst_media_common : 1;
        unsigned int ip_prst_media_common : 1;
        unsigned int ip_arst_media_common : 1;
        unsigned int ip_arst_jpg : 1;
        unsigned int ip_prst_jpg : 1;
        unsigned int ip_rst_jpg_func : 1;
        unsigned int ip_rst_fd_func : 1;
        unsigned int ip_rst_disp_subsys_cfg : 1;
        unsigned int ip_rst_isp_subsys_cfg : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_MEDIA1_CRG_PERRSTDIS0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_ics2_core_START (2)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_ics2_core_END (2)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_ics2_START (3)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_ics2_END (3)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_arst_ics2_START (4)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_arst_ics2_END (4)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_ics2_START (5)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_ics2_END (5)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_dss_START (6)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_dss_END (6)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_dss_START (7)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_dss_END (7)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_atgm_dss_START (9)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_atgm_dss_END (9)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_atgm_isp_START (10)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_atgm_isp_END (10)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_trace_START (11)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_trace_END (11)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_media1_lm_START (13)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_media1_lm_END (13)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_brg_START (14)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_brg_END (14)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_fdai_func_START (15)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_fdai_func_END (15)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_atgs_vivo_START (16)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_atgs_vivo_END (16)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_atgm_ivp_START (17)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_atgm_ivp_END (17)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_media_common_START (18)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_media_common_END (18)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_media_common_START (19)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_media_common_END (19)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_arst_media_common_START (20)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_arst_media_common_END (20)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_arst_jpg_START (21)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_arst_jpg_END (21)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_jpg_START (22)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_jpg_END (22)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_jpg_func_START (23)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_jpg_func_END (23)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_fd_func_START (24)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_fd_func_END (24)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_disp_subsys_cfg_START (25)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_disp_subsys_cfg_END (25)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_isp_subsys_cfg_START (26)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_isp_subsys_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_ics2_core : 1;
        unsigned int ip_prst_ics2 : 1;
        unsigned int ip_arst_ics2 : 1;
        unsigned int ip_rst_ics2 : 1;
        unsigned int ip_rst_dss : 1;
        unsigned int ip_prst_dss : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_prst_atgm_dss : 1;
        unsigned int ip_prst_atgm_isp : 1;
        unsigned int ip_rst_trace : 1;
        unsigned int reserved_3 : 1;
        unsigned int ip_prst_media1_lm : 1;
        unsigned int ip_rst_brg : 1;
        unsigned int ip_rst_fdai_func : 1;
        unsigned int ip_prst_atgs_vivo : 1;
        unsigned int ip_prst_atgm_ivp : 1;
        unsigned int ip_rst_media_common : 1;
        unsigned int ip_prst_media_common : 1;
        unsigned int ip_arst_media_common : 1;
        unsigned int ip_arst_jpg : 1;
        unsigned int ip_prst_jpg : 1;
        unsigned int ip_rst_jpg_func : 1;
        unsigned int ip_rst_fd_func : 1;
        unsigned int ip_rst_disp_subsys_cfg : 1;
        unsigned int ip_rst_isp_subsys_cfg : 1;
        unsigned int reserved_4 : 5;
    } reg;
} SOC_MEDIA1_CRG_PERRSTSTAT0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_ics2_core_START (2)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_ics2_core_END (2)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_ics2_START (3)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_ics2_END (3)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_arst_ics2_START (4)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_arst_ics2_END (4)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_ics2_START (5)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_ics2_END (5)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_dss_START (6)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_dss_END (6)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_dss_START (7)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_dss_END (7)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_atgm_dss_START (9)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_atgm_dss_END (9)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_atgm_isp_START (10)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_atgm_isp_END (10)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_trace_START (11)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_trace_END (11)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_media1_lm_START (13)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_media1_lm_END (13)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_brg_START (14)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_brg_END (14)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_fdai_func_START (15)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_fdai_func_END (15)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_atgs_vivo_START (16)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_atgs_vivo_END (16)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_atgm_ivp_START (17)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_atgm_ivp_END (17)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_media_common_START (18)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_media_common_END (18)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_media_common_START (19)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_media_common_END (19)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_arst_media_common_START (20)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_arst_media_common_END (20)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_arst_jpg_START (21)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_arst_jpg_END (21)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_jpg_START (22)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_jpg_END (22)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_jpg_func_START (23)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_jpg_func_END (23)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_fd_func_START (24)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_fd_func_END (24)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_disp_subsys_cfg_START (25)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_disp_subsys_cfg_END (25)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_isp_subsys_cfg_START (26)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_isp_subsys_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_noc_isp : 1;
        unsigned int ip_rst_noc_isp_cfg : 1;
        unsigned int ip_rst_noc_dss : 1;
        unsigned int ip_rst_noc_dss_cfg : 1;
        unsigned int ip_prst_mmbuf : 1;
        unsigned int ip_prst_mmbuf_cfg : 1;
        unsigned int reserved_2 : 24;
    } reg;
} SOC_MEDIA1_CRG_PERRSTEN1_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_noc_isp_START (2)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_noc_isp_END (2)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_noc_isp_cfg_START (3)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_noc_isp_cfg_END (3)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_noc_dss_START (4)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_noc_dss_END (4)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_noc_dss_cfg_START (5)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_noc_dss_cfg_END (5)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_prst_mmbuf_START (6)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_prst_mmbuf_END (6)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_prst_mmbuf_cfg_START (7)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_prst_mmbuf_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_noc_isp : 1;
        unsigned int ip_rst_noc_isp_cfg : 1;
        unsigned int ip_rst_noc_dss : 1;
        unsigned int ip_rst_noc_dss_cfg : 1;
        unsigned int ip_prst_mmbuf : 1;
        unsigned int ip_prst_mmbuf_cfg : 1;
        unsigned int reserved_2 : 24;
    } reg;
} SOC_MEDIA1_CRG_PERRSTDIS1_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_noc_isp_START (2)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_noc_isp_END (2)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_noc_isp_cfg_START (3)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_noc_isp_cfg_END (3)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_noc_dss_START (4)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_noc_dss_END (4)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_noc_dss_cfg_START (5)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_noc_dss_cfg_END (5)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_prst_mmbuf_START (6)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_prst_mmbuf_END (6)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_prst_mmbuf_cfg_START (7)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_prst_mmbuf_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_noc_isp : 1;
        unsigned int ip_rst_noc_isp_cfg : 1;
        unsigned int ip_rst_noc_dss : 1;
        unsigned int ip_rst_noc_dss_cfg : 1;
        unsigned int ip_prst_mmbuf : 1;
        unsigned int ip_prst_mmbuf_cfg : 1;
        unsigned int reserved_2 : 24;
    } reg;
} SOC_MEDIA1_CRG_PERRSTSTAT1_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_noc_isp_START (2)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_noc_isp_END (2)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_noc_isp_cfg_START (3)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_noc_isp_cfg_END (3)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_noc_dss_START (4)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_noc_dss_END (4)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_noc_dss_cfg_START (5)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_noc_dss_cfg_END (5)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_prst_mmbuf_START (6)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_prst_mmbuf_END (6)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_prst_mmbuf_cfg_START (7)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_prst_mmbuf_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_ldi0 : 6;
        unsigned int sel_ldi0_pll : 4;
        unsigned int sel_isp_cpu_pll : 4;
        unsigned int sel_jpg_func_pll : 2;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV0_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV0_div_ldi0_START (0)
#define SOC_MEDIA1_CRG_CLKDIV0_div_ldi0_END (5)
#define SOC_MEDIA1_CRG_CLKDIV0_sel_ldi0_pll_START (6)
#define SOC_MEDIA1_CRG_CLKDIV0_sel_ldi0_pll_END (9)
#define SOC_MEDIA1_CRG_CLKDIV0_sel_isp_cpu_pll_START (10)
#define SOC_MEDIA1_CRG_CLKDIV0_sel_isp_cpu_pll_END (13)
#define SOC_MEDIA1_CRG_CLKDIV0_sel_jpg_func_pll_START (14)
#define SOC_MEDIA1_CRG_CLKDIV0_sel_jpg_func_pll_END (15)
#define SOC_MEDIA1_CRG_CLKDIV0_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV0_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_ldi1 : 6;
        unsigned int sel_ldi1_pll : 4;
        unsigned int reserved_0 : 4;
        unsigned int reserved_1 : 2;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV1_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV1_div_ldi1_START (0)
#define SOC_MEDIA1_CRG_CLKDIV1_div_ldi1_END (5)
#define SOC_MEDIA1_CRG_CLKDIV1_sel_ldi1_pll_START (6)
#define SOC_MEDIA1_CRG_CLKDIV1_sel_ldi1_pll_END (9)
#define SOC_MEDIA1_CRG_CLKDIV1_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV1_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_edc0 : 6;
        unsigned int sel_edc0_pll : 4;
        unsigned int div_ivp32dsp_core : 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV2_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV2_div_edc0_START (0)
#define SOC_MEDIA1_CRG_CLKDIV2_div_edc0_END (5)
#define SOC_MEDIA1_CRG_CLKDIV2_sel_edc0_pll_START (6)
#define SOC_MEDIA1_CRG_CLKDIV2_sel_edc0_pll_END (9)
#define SOC_MEDIA1_CRG_CLKDIV2_div_ivp32dsp_core_START (10)
#define SOC_MEDIA1_CRG_CLKDIV2_div_ivp32dsp_core_END (15)
#define SOC_MEDIA1_CRG_CLKDIV2_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV2_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_ispfunc : 6;
        unsigned int sel_ispfunc_pll : 4;
        unsigned int sel_ispfunc2_pll : 4;
        unsigned int reserved : 2;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV3_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV3_div_ispfunc_START (0)
#define SOC_MEDIA1_CRG_CLKDIV3_div_ispfunc_END (5)
#define SOC_MEDIA1_CRG_CLKDIV3_sel_ispfunc_pll_START (6)
#define SOC_MEDIA1_CRG_CLKDIV3_sel_ispfunc_pll_END (9)
#define SOC_MEDIA1_CRG_CLKDIV3_sel_ispfunc2_pll_START (10)
#define SOC_MEDIA1_CRG_CLKDIV3_sel_ispfunc2_pll_END (13)
#define SOC_MEDIA1_CRG_CLKDIV3_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV3_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_isp_cpu : 6;
        unsigned int reserved_0 : 2;
        unsigned int sel_fd_func_pll : 2;
        unsigned int sel_ispfunc3_pll : 4;
        unsigned int reserved_1 : 2;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV4_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV4_div_isp_cpu_START (0)
#define SOC_MEDIA1_CRG_CLKDIV4_div_isp_cpu_END (5)
#define SOC_MEDIA1_CRG_CLKDIV4_sel_fd_func_pll_START (8)
#define SOC_MEDIA1_CRG_CLKDIV4_sel_fd_func_pll_END (9)
#define SOC_MEDIA1_CRG_CLKDIV4_sel_ispfunc3_pll_START (10)
#define SOC_MEDIA1_CRG_CLKDIV4_sel_ispfunc3_pll_END (13)
#define SOC_MEDIA1_CRG_CLKDIV4_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV4_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_vivobus : 6;
        unsigned int sel_vivobus_pll : 4;
        unsigned int div_ispfunc4 : 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV5_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START (0)
#define SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END (5)
#define SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START (6)
#define SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_END (9)
#define SOC_MEDIA1_CRG_CLKDIV5_div_ispfunc4_START (10)
#define SOC_MEDIA1_CRG_CLKDIV5_div_ispfunc4_END (15)
#define SOC_MEDIA1_CRG_CLKDIV5_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV5_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_pclk_mmbuf : 2;
        unsigned int sel_isp_i2c : 1;
        unsigned int sel_ispfunc4_pll : 4;
        unsigned int reserved_0 : 2;
        unsigned int sel_ivp32dsp_core_pll : 4;
        unsigned int reserved_1 : 3;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV6_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV6_div_pclk_mmbuf_START (0)
#define SOC_MEDIA1_CRG_CLKDIV6_div_pclk_mmbuf_END (1)
#define SOC_MEDIA1_CRG_CLKDIV6_sel_isp_i2c_START (2)
#define SOC_MEDIA1_CRG_CLKDIV6_sel_isp_i2c_END (2)
#define SOC_MEDIA1_CRG_CLKDIV6_sel_ispfunc4_pll_START (3)
#define SOC_MEDIA1_CRG_CLKDIV6_sel_ispfunc4_pll_END (6)
#define SOC_MEDIA1_CRG_CLKDIV6_sel_ivp32dsp_core_pll_START (9)
#define SOC_MEDIA1_CRG_CLKDIV6_sel_ivp32dsp_core_pll_END (12)
#define SOC_MEDIA1_CRG_CLKDIV6_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV6_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 4;
        unsigned int div_fd_func : 6;
        unsigned int div_aclk_mmbuf : 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV7_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV7_div_fd_func_START (4)
#define SOC_MEDIA1_CRG_CLKDIV7_div_fd_func_END (9)
#define SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_START (10)
#define SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_END (15)
#define SOC_MEDIA1_CRG_CLKDIV7_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV7_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_jpg_func : 6;
        unsigned int div_media_common : 6;
        unsigned int sel_media_common_pll : 4;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV8_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV8_div_jpg_func_START (0)
#define SOC_MEDIA1_CRG_CLKDIV8_div_jpg_func_END (5)
#define SOC_MEDIA1_CRG_CLKDIV8_div_media_common_START (6)
#define SOC_MEDIA1_CRG_CLKDIV8_div_media_common_END (11)
#define SOC_MEDIA1_CRG_CLKDIV8_sel_media_common_pll_START (12)
#define SOC_MEDIA1_CRG_CLKDIV8_sel_media_common_pll_END (15)
#define SOC_MEDIA1_CRG_CLKDIV8_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV8_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_gt_clk_ispfunc4 : 1;
        unsigned int sc_gt_clk_ispfunc3 : 1;
        unsigned int sc_gt_clk_ispfunc2 : 1;
        unsigned int sc_gt_clk_vivobus : 1;
        unsigned int sc_gt_clk_ivp32dsp_core : 1;
        unsigned int sc_gt_clk_isp_cpu : 1;
        unsigned int sc_gt_clk_ispfunc : 1;
        unsigned int sc_gt_clk_edc0 : 1;
        unsigned int sc_gt_clk_ldi1 : 1;
        unsigned int sc_gt_clk_ldi0 : 1;
        unsigned int sc_gt_clk_fdai_func : 1;
        unsigned int sc_gt_clk_media_common : 1;
        unsigned int sc_gt_clk_jpg_func : 1;
        unsigned int sc_gt_clk_fd_func : 1;
        unsigned int sc_gt_aclk_mmbuf : 1;
        unsigned int sc_gt_pclk_mmbuf : 1;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV9_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ispfunc4_START (0)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ispfunc4_END (0)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ispfunc3_START (1)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ispfunc3_END (1)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ispfunc2_START (2)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ispfunc2_END (2)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_vivobus_START (3)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_vivobus_END (3)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ivp32dsp_core_START (4)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ivp32dsp_core_END (4)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_isp_cpu_START (5)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_isp_cpu_END (5)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ispfunc_START (6)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ispfunc_END (6)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_edc0_START (7)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_edc0_END (7)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ldi1_START (8)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ldi1_END (8)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ldi0_START (9)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ldi0_END (9)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_fdai_func_START (10)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_fdai_func_END (10)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_media_common_START (11)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_media_common_END (11)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_jpg_func_START (12)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_jpg_func_END (12)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_fd_func_START (13)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_fd_func_END (13)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_aclk_mmbuf_START (14)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_aclk_mmbuf_END (14)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_pclk_mmbuf_START (15)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_pclk_mmbuf_END (15)
#define SOC_MEDIA1_CRG_CLKDIV9_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV9_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sel_mmbuf_pll : 4;
        unsigned int div_ispfunc2 : 6;
        unsigned int div_ispfunc3 : 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV10_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV10_sel_mmbuf_pll_START (0)
#define SOC_MEDIA1_CRG_CLKDIV10_sel_mmbuf_pll_END (3)
#define SOC_MEDIA1_CRG_CLKDIV10_div_ispfunc2_START (4)
#define SOC_MEDIA1_CRG_CLKDIV10_div_ispfunc2_END (9)
#define SOC_MEDIA1_CRG_CLKDIV10_div_ispfunc3_START (10)
#define SOC_MEDIA1_CRG_CLKDIV10_div_ispfunc3_END (15)
#define SOC_MEDIA1_CRG_CLKDIV10_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV10_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 16;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV11_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV11_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV11_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 16;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV12_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV12_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV12_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 16;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV13_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV13_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV13_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_fdai_func : 6;
        unsigned int sel_fdai_func_pll : 2;
        unsigned int reserved : 8;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV14_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV14_div_fdai_func_START (0)
#define SOC_MEDIA1_CRG_CLKDIV14_div_fdai_func_END (5)
#define SOC_MEDIA1_CRG_CLKDIV14_sel_fdai_func_pll_START (6)
#define SOC_MEDIA1_CRG_CLKDIV14_sel_fdai_func_pll_END (7)
#define SOC_MEDIA1_CRG_CLKDIV14_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV14_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_ics2 : 6;
        unsigned int sel_ics2_pll : 4;
        unsigned int sc_gt_clk_ics2 : 1;
        unsigned int reserved : 5;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV15_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV15_div_ics2_START (0)
#define SOC_MEDIA1_CRG_CLKDIV15_div_ics2_END (5)
#define SOC_MEDIA1_CRG_CLKDIV15_sel_ics2_pll_START (6)
#define SOC_MEDIA1_CRG_CLKDIV15_sel_ics2_pll_END (9)
#define SOC_MEDIA1_CRG_CLKDIV15_sc_gt_clk_ics2_START (10)
#define SOC_MEDIA1_CRG_CLKDIV15_sc_gt_clk_ics2_END (10)
#define SOC_MEDIA1_CRG_CLKDIV15_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV15_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int swdone_clk_ispfunc4_div : 1;
        unsigned int swdone_clk_ispfunc3_div : 1;
        unsigned int swdone_clk_ispfunc2_div : 1;
        unsigned int swdone_clk_vivobus_div : 1;
        unsigned int swdone_clk_ivp32dsp_core_div : 1;
        unsigned int swdone_clk_isp_cpu_div : 1;
        unsigned int swdone_clk_ispfunc_div : 1;
        unsigned int swdone_clk_edc0_div : 1;
        unsigned int swdone_clk_ldi1_div : 1;
        unsigned int swdone_clk_ldi0_div : 1;
        unsigned int swdone_clk_fdai_func_div : 1;
        unsigned int swdone_clk_media_common_div : 1;
        unsigned int swdone_clk_jpg_func_div : 1;
        unsigned int swdone_clk_fd_func_div : 1;
        unsigned int swdone_aclk_mmbuf_div : 1;
        unsigned int swdone_pclk_mmbuf_div : 1;
        unsigned int reserved : 16;
    } reg;
} SOC_MEDIA1_CRG_PERI_STAT0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_ispfunc4_div_START (0)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_ispfunc4_div_END (0)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_ispfunc3_div_START (1)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_ispfunc3_div_END (1)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_ispfunc2_div_START (2)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_ispfunc2_div_END (2)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_vivobus_div_START (3)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_vivobus_div_END (3)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_ivp32dsp_core_div_START (4)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_ivp32dsp_core_div_END (4)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_isp_cpu_div_START (5)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_isp_cpu_div_END (5)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_ispfunc_div_START (6)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_ispfunc_div_END (6)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_edc0_div_START (7)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_edc0_div_END (7)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_ldi1_div_START (8)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_ldi1_div_END (8)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_ldi0_div_START (9)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_ldi0_div_END (9)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_fdai_func_div_START (10)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_fdai_func_div_END (10)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_media_common_div_START (11)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_media_common_div_END (11)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_jpg_func_div_START (12)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_jpg_func_div_END (12)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_fd_func_div_START (13)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_fd_func_div_END (13)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_aclk_mmbuf_div_START (14)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_aclk_mmbuf_div_END (14)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_pclk_mmbuf_div_START (15)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_pclk_mmbuf_div_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_ack_clk_ldi0_sw : 4;
        unsigned int sw_ack_clk_ldi1_sw : 4;
        unsigned int sw_ack_clk_edc0_sw : 4;
        unsigned int sw_ack_clk_ispfunc_sw : 4;
        unsigned int sw_ack_clk_vivobus_sw : 4;
        unsigned int sw_ack_clk_ispfunc3_sw : 4;
        unsigned int sw_ack_clk_ispfunc2_sw : 4;
        unsigned int sw_ack_clk_ispfunc4_sw : 4;
    } reg;
} SOC_MEDIA1_CRG_PERI_STAT1_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_ldi0_sw_START (0)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_ldi0_sw_END (3)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_ldi1_sw_START (4)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_ldi1_sw_END (7)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_edc0_sw_START (8)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_edc0_sw_END (11)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_ispfunc_sw_START (12)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_ispfunc_sw_END (15)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_vivobus_sw_START (16)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_vivobus_sw_END (19)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_ispfunc3_sw_START (20)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_ispfunc3_sw_END (23)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_ispfunc2_sw_START (24)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_ispfunc2_sw_END (27)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_ispfunc4_sw_START (28)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_ispfunc4_sw_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_ack_clk_isp_cpu_sw : 4;
        unsigned int sw_ack_clk_media_common_sw : 4;
        unsigned int sw_ack_aclk_mmbuf_sw : 4;
        unsigned int sw_ack_clk_ivp32dsp_core_sw : 4;
        unsigned int sw_ack_clk_ics2_sw : 4;
        unsigned int reserved : 12;
    } reg;
} SOC_MEDIA1_CRG_PERI_STAT2_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_STAT2_sw_ack_clk_isp_cpu_sw_START (0)
#define SOC_MEDIA1_CRG_PERI_STAT2_sw_ack_clk_isp_cpu_sw_END (3)
#define SOC_MEDIA1_CRG_PERI_STAT2_sw_ack_clk_media_common_sw_START (4)
#define SOC_MEDIA1_CRG_PERI_STAT2_sw_ack_clk_media_common_sw_END (7)
#define SOC_MEDIA1_CRG_PERI_STAT2_sw_ack_aclk_mmbuf_sw_START (8)
#define SOC_MEDIA1_CRG_PERI_STAT2_sw_ack_aclk_mmbuf_sw_END (11)
#define SOC_MEDIA1_CRG_PERI_STAT2_sw_ack_clk_ivp32dsp_core_sw_START (12)
#define SOC_MEDIA1_CRG_PERI_STAT2_sw_ack_clk_ivp32dsp_core_sw_END (15)
#define SOC_MEDIA1_CRG_PERI_STAT2_sw_ack_clk_ics2_sw_START (16)
#define SOC_MEDIA1_CRG_PERI_STAT2_sw_ack_clk_ics2_sw_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int swdone_clk_ics2_div : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_MEDIA1_CRG_PERI_STAT3_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_STAT3_swdone_clk_ics2_div_START (0)
#define SOC_MEDIA1_CRG_PERI_STAT3_swdone_clk_ics2_div_END (0)
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
        unsigned int reserved_7: 25;
    } reg;
} SOC_MEDIA1_CRG_ISOEN_UNION;
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
        unsigned int reserved_7: 25;
    } reg;
} SOC_MEDIA1_CRG_ISODIS_UNION;
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
        unsigned int reserved_7: 25;
    } reg;
} SOC_MEDIA1_CRG_ISOSTAT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_clkrst_bypass : 1;
        unsigned int media_common_clkrst_bypass : 1;
        unsigned int media1_lm_clkrst_bypass : 1;
        unsigned int media1_ics2_smmu_clkrst_bypass : 1;
        unsigned int media1_ics2_bcu_clkrst_bypass : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_MEDIA1_CRG_IPCLKRST_BYPASS_UNION;
#endif
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_dss_clkrst_bypass_START (0)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_dss_clkrst_bypass_END (0)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_media_common_clkrst_bypass_START (1)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_media_common_clkrst_bypass_END (1)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_media1_lm_clkrst_bypass_START (2)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_media1_lm_clkrst_bypass_END (2)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_media1_ics2_smmu_clkrst_bypass_START (3)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_media1_ics2_smmu_clkrst_bypass_END (3)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_media1_ics2_bcu_clkrst_bypass_START (4)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_media1_ics2_bcu_clkrst_bypass_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int clkinen : 1;
        unsigned int atclkoff_n : 1;
        unsigned int pclkdbg_to_asyn_bri_clkoff_n : 1;
        unsigned int atclk_to_asyn_bri_clkoff_n : 1;
        unsigned int pclkdbg_to_cs_clkoff_n : 1;
        unsigned int aclkenm_sel : 1;
        unsigned int sys_cnt_en_dly : 4;
        unsigned int periphclk_div : 3;
        unsigned int axi_clkoff_n : 1;
        unsigned int periphclkoff_n : 1;
        unsigned int reserved : 1;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_ISPCPU_CLKEN_UNION;
#endif
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_clkinen_START (0)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_clkinen_END (0)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_atclkoff_n_START (1)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_atclkoff_n_END (1)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_pclkdbg_to_asyn_bri_clkoff_n_START (2)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_pclkdbg_to_asyn_bri_clkoff_n_END (2)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_atclk_to_asyn_bri_clkoff_n_START (3)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_atclk_to_asyn_bri_clkoff_n_END (3)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_pclkdbg_to_cs_clkoff_n_START (4)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_pclkdbg_to_cs_clkoff_n_END (4)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_aclkenm_sel_START (5)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_aclkenm_sel_END (5)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_sys_cnt_en_dly_START (6)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_sys_cnt_en_dly_END (9)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_periphclk_div_START (10)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_periphclk_div_END (12)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_axi_clkoff_n_START (13)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_axi_clkoff_n_END (13)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_periphclkoff_n_START (14)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_periphclkoff_n_END (14)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_bitmasken_START (16)
#define SOC_MEDIA1_CRG_ISPCPU_CLKEN_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_por_rst_req : 1;
        unsigned int rst_software_req : 1;
        unsigned int core_rst_software_req : 1;
        unsigned int debug_all_rst_req : 1;
        unsigned int core_debug_rst_req : 1;
        unsigned int coresight_soft_rst_req : 1;
        unsigned int dbgrstreq : 1;
        unsigned int reserved_0 : 1;
        unsigned int wdreset_req : 1;
        unsigned int reserved_1 : 23;
    } reg;
} SOC_MEDIA1_CRG_ISPCPU_RSTEN_UNION;
#endif
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_core_por_rst_req_START (0)
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_core_por_rst_req_END (0)
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_rst_software_req_START (1)
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_rst_software_req_END (1)
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_core_rst_software_req_START (2)
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_core_rst_software_req_END (2)
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_debug_all_rst_req_START (3)
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_debug_all_rst_req_END (3)
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_core_debug_rst_req_START (4)
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_core_debug_rst_req_END (4)
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_coresight_soft_rst_req_START (5)
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_coresight_soft_rst_req_END (5)
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_dbgrstreq_START (6)
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_dbgrstreq_END (6)
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_wdreset_req_START (8)
#define SOC_MEDIA1_CRG_ISPCPU_RSTEN_wdreset_req_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_por_rst_req : 1;
        unsigned int rst_software_req : 1;
        unsigned int core_rst_software_req : 1;
        unsigned int debug_all_rst_req : 1;
        unsigned int core_debug_rst_req : 1;
        unsigned int coresight_soft_rst_req : 1;
        unsigned int dbgrstreq : 1;
        unsigned int reserved_0 : 1;
        unsigned int wdreset_req : 1;
        unsigned int reserved_1 : 23;
    } reg;
} SOC_MEDIA1_CRG_ISPCPU_RSTDIS_UNION;
#endif
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_core_por_rst_req_START (0)
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_core_por_rst_req_END (0)
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_rst_software_req_START (1)
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_rst_software_req_END (1)
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_core_rst_software_req_START (2)
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_core_rst_software_req_END (2)
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_debug_all_rst_req_START (3)
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_debug_all_rst_req_END (3)
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_core_debug_rst_req_START (4)
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_core_debug_rst_req_END (4)
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_coresight_soft_rst_req_START (5)
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_coresight_soft_rst_req_END (5)
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_dbgrstreq_START (6)
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_dbgrstreq_END (6)
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_wdreset_req_START (8)
#define SOC_MEDIA1_CRG_ISPCPU_RSTDIS_wdreset_req_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_por_rst_req : 1;
        unsigned int rst_software_req : 1;
        unsigned int core_rst_software_req : 1;
        unsigned int debug_all_rst_req : 1;
        unsigned int core_debug_rst_req : 1;
        unsigned int coresight_soft_rst_req : 1;
        unsigned int dbgrstreq : 1;
        unsigned int reserved_0 : 1;
        unsigned int wdreset_req : 1;
        unsigned int reserved_1 : 23;
    } reg;
} SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_UNION;
#endif
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_core_por_rst_req_START (0)
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_core_por_rst_req_END (0)
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_rst_software_req_START (1)
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_rst_software_req_END (1)
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_core_rst_software_req_START (2)
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_core_rst_software_req_END (2)
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_debug_all_rst_req_START (3)
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_debug_all_rst_req_END (3)
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_core_debug_rst_req_START (4)
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_core_debug_rst_req_END (4)
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_coresight_soft_rst_req_START (5)
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_coresight_soft_rst_req_END (5)
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_dbgrstreq_START (6)
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_dbgrstreq_END (6)
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_wdreset_req_START (8)
#define SOC_MEDIA1_CRG_ISPCPU_RSTSTAT_wdreset_req_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int standbywfi : 1;
        unsigned int standbywfe : 1;
        unsigned int smpnamp : 1;
        unsigned int scuidle : 1;
        unsigned int dbgack : 1;
        unsigned int dbgcpudone : 1;
        unsigned int pmupirv : 1;
        unsigned int fpuflags0 : 6;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_MEDIA1_CRG_ISP_CPU_STATE0_UNION;
#endif
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_standbywfi_START (0)
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_standbywfi_END (0)
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_standbywfe_START (1)
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_standbywfe_END (1)
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_smpnamp_START (2)
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_smpnamp_END (2)
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_scuidle_START (3)
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_scuidle_END (3)
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_dbgack_START (4)
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_dbgack_END (4)
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_dbgcpudone_START (5)
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_dbgcpudone_END (5)
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_pmupirv_START (6)
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_pmupirv_END (6)
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_fpuflags0_START (7)
#define SOC_MEDIA1_CRG_ISP_CPU_STATE0_fpuflags0_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_MEDIA1_CRG_ISP_CPU_STATE1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ivp_div_auto_reduce_bypass : 1;
        unsigned int ivp_auto_waitcfg_in : 10;
        unsigned int ivp_auto_waitcfg_out : 9;
        unsigned int ivp_dw_axi_m5_st_bypass : 1;
        unsigned int ivp_div_auto_cfg : 6;
        unsigned int ivp_pwaitmode_bypass : 1;
        unsigned int ivp_dw_axi_m1_st_bypass : 1;
        unsigned int ivp_dw_axi_m2_st_bypass : 1;
        unsigned int ivp_dw_axi_m3_st_bypass : 1;
        unsigned int ivp_dw_axi_m4_st_bypass : 1;
    } reg;
} SOC_MEDIA1_CRG_PERI_AUTODIV0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_div_auto_reduce_bypass_START (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_div_auto_reduce_bypass_END (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_auto_waitcfg_in_START (1)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_auto_waitcfg_in_END (10)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_auto_waitcfg_out_START (11)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_auto_waitcfg_out_END (19)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_dw_axi_m5_st_bypass_START (20)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_dw_axi_m5_st_bypass_END (20)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_div_auto_cfg_START (21)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_div_auto_cfg_END (26)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_pwaitmode_bypass_START (27)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_pwaitmode_bypass_END (27)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_dw_axi_m1_st_bypass_START (28)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_dw_axi_m1_st_bypass_END (28)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_dw_axi_m2_st_bypass_START (29)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_dw_axi_m2_st_bypass_END (29)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_dw_axi_m3_st_bypass_START (30)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_dw_axi_m3_st_bypass_END (30)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_dw_axi_m4_st_bypass_START (31)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ivp_dw_axi_m4_st_bypass_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int isapcpu_div_auto_reduce_bypass : 1;
        unsigned int isapcpu_auto_waitcfg_in : 10;
        unsigned int isapcpu_auto_waitcfg_out : 10;
        unsigned int isapcpu_div_auto_cfg : 6;
        unsigned int reserved : 5;
    } reg;
} SOC_MEDIA1_CRG_PERI_AUTODIV1_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_AUTODIV1_isapcpu_div_auto_reduce_bypass_START (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV1_isapcpu_div_auto_reduce_bypass_END (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV1_isapcpu_auto_waitcfg_in_START (1)
#define SOC_MEDIA1_CRG_PERI_AUTODIV1_isapcpu_auto_waitcfg_in_END (10)
#define SOC_MEDIA1_CRG_PERI_AUTODIV1_isapcpu_auto_waitcfg_out_START (11)
#define SOC_MEDIA1_CRG_PERI_AUTODIV1_isapcpu_auto_waitcfg_out_END (20)
#define SOC_MEDIA1_CRG_PERI_AUTODIV1_isapcpu_div_auto_cfg_START (21)
#define SOC_MEDIA1_CRG_PERI_AUTODIV1_isapcpu_div_auto_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 10;
        unsigned int reserved_2: 10;
        unsigned int reserved_3: 6;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 4;
    } reg;
} SOC_MEDIA1_CRG_PERI_AUTODIV2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 10;
        unsigned int reserved_2: 10;
        unsigned int reserved_3: 6;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 4;
    } reg;
} SOC_MEDIA1_CRG_PERI_AUTODIV3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vivobus_div_auto_reduce_bypass : 1;
        unsigned int vivobus_auto_waitcfg_in : 10;
        unsigned int vivobus_auto_waitcfg_out : 8;
        unsigned int vivobus_isp1_bypass : 1;
        unsigned int vivobus_ispcpu_wfi_bypass : 1;
        unsigned int vivobus_div_auto_cfg : 6;
        unsigned int vivobus_media_com_bypass : 1;
        unsigned int vivobus_dss0_bypass : 1;
        unsigned int vivobus_dss1_bypass : 1;
        unsigned int vivobus_isp0_bypass : 1;
        unsigned int vivobus_ispcpu_bypass : 1;
    } reg;
} SOC_MEDIA1_CRG_PERI_AUTODIV4_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_div_auto_reduce_bypass_START (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_div_auto_reduce_bypass_END (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_auto_waitcfg_in_START (1)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_auto_waitcfg_in_END (10)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_auto_waitcfg_out_START (11)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_auto_waitcfg_out_END (18)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_isp1_bypass_START (19)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_isp1_bypass_END (19)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_ispcpu_wfi_bypass_START (20)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_ispcpu_wfi_bypass_END (20)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_div_auto_cfg_START (21)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_div_auto_cfg_END (26)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_media_com_bypass_START (27)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_media_com_bypass_END (27)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_dss0_bypass_START (28)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_dss0_bypass_END (28)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_dss1_bypass_START (29)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_dss1_bypass_END (29)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_isp0_bypass_START (30)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_isp0_bypass_END (30)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_ispcpu_bypass_START (31)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_ispcpu_bypass_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ics2_div_auto_reduce_bypass : 1;
        unsigned int ics2_auto_waitcfg_in : 10;
        unsigned int ics2_auto_waitcfg_out : 10;
        unsigned int ics2_div_auto_cfg : 6;
        unsigned int reserved : 5;
    } reg;
} SOC_MEDIA1_CRG_PERI_AUTODIV5_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_ics2_div_auto_reduce_bypass_START (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_ics2_div_auto_reduce_bypass_END (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_ics2_auto_waitcfg_in_START (1)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_ics2_auto_waitcfg_in_END (10)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_ics2_auto_waitcfg_out_START (11)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_ics2_auto_waitcfg_out_END (20)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_ics2_div_auto_cfg_START (21)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_ics2_div_auto_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vivobus_jpeg_fd_bypass : 1;
        unsigned int vivobus_isp_cpu_per_bypass : 1;
        unsigned int vivobus_cfg2media_bypass : 1;
        unsigned int vivobus_ivp32_mst_bypass : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_MEDIA1_CRG_PERI_AUTODIV6_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_AUTODIV6_vivobus_jpeg_fd_bypass_START (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV6_vivobus_jpeg_fd_bypass_END (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV6_vivobus_isp_cpu_per_bypass_START (1)
#define SOC_MEDIA1_CRG_PERI_AUTODIV6_vivobus_isp_cpu_per_bypass_END (1)
#define SOC_MEDIA1_CRG_PERI_AUTODIV6_vivobus_cfg2media_bypass_START (2)
#define SOC_MEDIA1_CRG_PERI_AUTODIV6_vivobus_cfg2media_bypass_END (2)
#define SOC_MEDIA1_CRG_PERI_AUTODIV6_vivobus_ivp32_mst_bypass_START (3)
#define SOC_MEDIA1_CRG_PERI_AUTODIV6_vivobus_ivp32_mst_bypass_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int autodiv_ispcpu_stat : 1;
        unsigned int vbat_drop_freq_down_isp_cpu_stat : 1;
        unsigned int vbat_drop_freq_down_ispfunc_stat : 1;
        unsigned int autodiv_vivobus_stat : 1;
        unsigned int vbat_drop_freq_down_ispfunc2_stat : 1;
        unsigned int autodiv_ivp32dsp_stat : 1;
        unsigned int vbat_drop_freq_down_ispfunc3_stat : 1;
        unsigned int vbat_drop_freq_down_ispfunc4_stat : 1;
        unsigned int vbat_drop_freq_down_ics2_stat : 1;
        unsigned int autodiv_ics2_stat : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_UNION;
#endif
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_autodiv_ispcpu_stat_START (0)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_autodiv_ispcpu_stat_END (0)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_vbat_drop_freq_down_isp_cpu_stat_START (1)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_vbat_drop_freq_down_isp_cpu_stat_END (1)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_vbat_drop_freq_down_ispfunc_stat_START (2)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_vbat_drop_freq_down_ispfunc_stat_END (2)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_autodiv_vivobus_stat_START (3)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_autodiv_vivobus_stat_END (3)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_vbat_drop_freq_down_ispfunc2_stat_START (4)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_vbat_drop_freq_down_ispfunc2_stat_END (4)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_autodiv_ivp32dsp_stat_START (5)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_autodiv_ivp32dsp_stat_END (5)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_vbat_drop_freq_down_ispfunc3_stat_START (6)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_vbat_drop_freq_down_ispfunc3_stat_END (6)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_vbat_drop_freq_down_ispfunc4_stat_START (7)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_vbat_drop_freq_down_ispfunc4_stat_END (7)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_vbat_drop_freq_down_ics2_stat_START (8)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_vbat_drop_freq_down_ics2_stat_END (8)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_autodiv_ics2_stat_START (9)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_autodiv_ics2_stat_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mmbuf_mem_ctrl_s : 26;
        unsigned int reserved : 6;
    } reg;
} SOC_MEDIA1_CRG_MMBUF_MEM_CTRL_UNION;
#endif
#define SOC_MEDIA1_CRG_MMBUF_MEM_CTRL_mmbuf_mem_ctrl_s_START (0)
#define SOC_MEDIA1_CRG_MMBUF_MEM_CTRL_mmbuf_mem_ctrl_s_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int isptomed1_reserve_stat : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_MEDIA1_CRG_ISPTOMED1_RESERVE_OUT_UNION;
#endif
#define SOC_MEDIA1_CRG_ISPTOMED1_RESERVE_OUT_isptomed1_reserve_stat_START (0)
#define SOC_MEDIA1_CRG_ISPTOMED1_RESERVE_OUT_isptomed1_reserve_stat_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_clr_mediabus_nonidle_pend : 1;
        unsigned int intr_mask_vivobus_nonidle_pend : 1;
        unsigned int intr_mask_dispbus_nonidle_pend : 1;
        unsigned int intr_mask_ispbus_nonidle_pend : 1;
        unsigned int intr_mask_ivpbus_nonidle_pend : 1;
        unsigned int reserved : 11;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_INTR_NOCBUS_NONIDLE_PEND_UNION;
#endif
#define SOC_MEDIA1_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_clr_mediabus_nonidle_pend_START (0)
#define SOC_MEDIA1_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_clr_mediabus_nonidle_pend_END (0)
#define SOC_MEDIA1_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_vivobus_nonidle_pend_START (1)
#define SOC_MEDIA1_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_vivobus_nonidle_pend_END (1)
#define SOC_MEDIA1_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_dispbus_nonidle_pend_START (2)
#define SOC_MEDIA1_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_dispbus_nonidle_pend_END (2)
#define SOC_MEDIA1_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_ispbus_nonidle_pend_START (3)
#define SOC_MEDIA1_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_ispbus_nonidle_pend_END (3)
#define SOC_MEDIA1_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_ivpbus_nonidle_pend_START (4)
#define SOC_MEDIA1_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_ivpbus_nonidle_pend_END (4)
#define SOC_MEDIA1_CRG_INTR_NOCBUS_NONIDLE_PEND_bitmasken_START (16)
#define SOC_MEDIA1_CRG_INTR_NOCBUS_NONIDLE_PEND_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_stat_vivobus_nonidle_pend : 1;
        unsigned int intr_stat_dispbus_nonidle_pend : 1;
        unsigned int intr_stat_ispbus_nonidle_pend : 1;
        unsigned int intr_stat_ivpbus_nonidle_pend : 1;
        unsigned int reserved_0 : 12;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_MEDIA1_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_UNION;
#endif
#define SOC_MEDIA1_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_vivobus_nonidle_pend_START (0)
#define SOC_MEDIA1_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_vivobus_nonidle_pend_END (0)
#define SOC_MEDIA1_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_dispbus_nonidle_pend_START (1)
#define SOC_MEDIA1_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_dispbus_nonidle_pend_END (1)
#define SOC_MEDIA1_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_ispbus_nonidle_pend_START (2)
#define SOC_MEDIA1_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_ispbus_nonidle_pend_END (2)
#define SOC_MEDIA1_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_ivpbus_nonidle_pend_START (3)
#define SOC_MEDIA1_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_ivpbus_nonidle_pend_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dmss_ckg_byp : 1;
        unsigned int reserved_0 : 7;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_MEDIA1_CRG_DMSS_ASYNC_BRG_UNION;
#endif
#define SOC_MEDIA1_CRG_DMSS_ASYNC_BRG_dmss_ckg_byp_START (0)
#define SOC_MEDIA1_CRG_DMSS_ASYNC_BRG_dmss_ckg_byp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vdbat_drop_freq_down_isp_cpu_bypass : 1;
        unsigned int div_isp_cpu_vbat_drop_freq_down : 5;
        unsigned int vdbat_drop_freq_down_ispfunc_bypass : 1;
        unsigned int div_ispfunc_vbat_drop_freq_down : 6;
        unsigned int reserved : 3;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_COMMON_CTRL0_UNION;
#endif
#define SOC_MEDIA1_CRG_COMMON_CTRL0_vdbat_drop_freq_down_isp_cpu_bypass_START (0)
#define SOC_MEDIA1_CRG_COMMON_CTRL0_vdbat_drop_freq_down_isp_cpu_bypass_END (0)
#define SOC_MEDIA1_CRG_COMMON_CTRL0_div_isp_cpu_vbat_drop_freq_down_START (1)
#define SOC_MEDIA1_CRG_COMMON_CTRL0_div_isp_cpu_vbat_drop_freq_down_END (5)
#define SOC_MEDIA1_CRG_COMMON_CTRL0_vdbat_drop_freq_down_ispfunc_bypass_START (6)
#define SOC_MEDIA1_CRG_COMMON_CTRL0_vdbat_drop_freq_down_ispfunc_bypass_END (6)
#define SOC_MEDIA1_CRG_COMMON_CTRL0_div_ispfunc_vbat_drop_freq_down_START (7)
#define SOC_MEDIA1_CRG_COMMON_CTRL0_div_ispfunc_vbat_drop_freq_down_END (12)
#define SOC_MEDIA1_CRG_COMMON_CTRL0_bitmasken_START (16)
#define SOC_MEDIA1_CRG_COMMON_CTRL0_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vdbat_drop_freq_down_ispfunc2_bypass : 1;
        unsigned int div_ispfunc2_vbat_drop_freq_down : 5;
        unsigned int vdbat_drop_freq_down_ispfunc3_bypass : 1;
        unsigned int div_ispfunc3_vbat_drop_freq_down : 5;
        unsigned int reserved : 4;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_COMMON_CTRL1_UNION;
#endif
#define SOC_MEDIA1_CRG_COMMON_CTRL1_vdbat_drop_freq_down_ispfunc2_bypass_START (0)
#define SOC_MEDIA1_CRG_COMMON_CTRL1_vdbat_drop_freq_down_ispfunc2_bypass_END (0)
#define SOC_MEDIA1_CRG_COMMON_CTRL1_div_ispfunc2_vbat_drop_freq_down_START (1)
#define SOC_MEDIA1_CRG_COMMON_CTRL1_div_ispfunc2_vbat_drop_freq_down_END (5)
#define SOC_MEDIA1_CRG_COMMON_CTRL1_vdbat_drop_freq_down_ispfunc3_bypass_START (6)
#define SOC_MEDIA1_CRG_COMMON_CTRL1_vdbat_drop_freq_down_ispfunc3_bypass_END (6)
#define SOC_MEDIA1_CRG_COMMON_CTRL1_div_ispfunc3_vbat_drop_freq_down_START (7)
#define SOC_MEDIA1_CRG_COMMON_CTRL1_div_ispfunc3_vbat_drop_freq_down_END (11)
#define SOC_MEDIA1_CRG_COMMON_CTRL1_bitmasken_START (16)
#define SOC_MEDIA1_CRG_COMMON_CTRL1_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vdbat_drop_freq_down_ispfunc4_bypass : 1;
        unsigned int div_ispfunc4_vbat_drop_freq_down : 5;
        unsigned int vdbat_drop_freq_down_ics2_bypass : 1;
        unsigned int div_ics2_vbat_drop_freq_down : 5;
        unsigned int reserved : 4;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_COMMON_CTRL2_UNION;
#endif
#define SOC_MEDIA1_CRG_COMMON_CTRL2_vdbat_drop_freq_down_ispfunc4_bypass_START (0)
#define SOC_MEDIA1_CRG_COMMON_CTRL2_vdbat_drop_freq_down_ispfunc4_bypass_END (0)
#define SOC_MEDIA1_CRG_COMMON_CTRL2_div_ispfunc4_vbat_drop_freq_down_START (1)
#define SOC_MEDIA1_CRG_COMMON_CTRL2_div_ispfunc4_vbat_drop_freq_down_END (5)
#define SOC_MEDIA1_CRG_COMMON_CTRL2_vdbat_drop_freq_down_ics2_bypass_START (6)
#define SOC_MEDIA1_CRG_COMMON_CTRL2_vdbat_drop_freq_down_ics2_bypass_END (6)
#define SOC_MEDIA1_CRG_COMMON_CTRL2_div_ics2_vbat_drop_freq_down_START (7)
#define SOC_MEDIA1_CRG_COMMON_CTRL2_div_ics2_vbat_drop_freq_down_END (11)
#define SOC_MEDIA1_CRG_COMMON_CTRL2_bitmasken_START (16)
#define SOC_MEDIA1_CRG_COMMON_CTRL2_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_isp : 1;
        unsigned int ip_arst_isp : 1;
        unsigned int ip_rst_isp_in_adb_n : 1;
        unsigned int ip_rst_isp_cputocfg : 1;
        unsigned int ip_rst_isp_cpu : 1;
        unsigned int ip_rst_isp_sys : 1;
        unsigned int ip_rst_isp_i2c : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_MEDIA1_CRG_PERRSTEN_ISP_SEC_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTEN_ISP_SEC_ip_rst_isp_START (0)
#define SOC_MEDIA1_CRG_PERRSTEN_ISP_SEC_ip_rst_isp_END (0)
#define SOC_MEDIA1_CRG_PERRSTEN_ISP_SEC_ip_arst_isp_START (1)
#define SOC_MEDIA1_CRG_PERRSTEN_ISP_SEC_ip_arst_isp_END (1)
#define SOC_MEDIA1_CRG_PERRSTEN_ISP_SEC_ip_rst_isp_in_adb_n_START (2)
#define SOC_MEDIA1_CRG_PERRSTEN_ISP_SEC_ip_rst_isp_in_adb_n_END (2)
#define SOC_MEDIA1_CRG_PERRSTEN_ISP_SEC_ip_rst_isp_cputocfg_START (3)
#define SOC_MEDIA1_CRG_PERRSTEN_ISP_SEC_ip_rst_isp_cputocfg_END (3)
#define SOC_MEDIA1_CRG_PERRSTEN_ISP_SEC_ip_rst_isp_cpu_START (4)
#define SOC_MEDIA1_CRG_PERRSTEN_ISP_SEC_ip_rst_isp_cpu_END (4)
#define SOC_MEDIA1_CRG_PERRSTEN_ISP_SEC_ip_rst_isp_sys_START (5)
#define SOC_MEDIA1_CRG_PERRSTEN_ISP_SEC_ip_rst_isp_sys_END (5)
#define SOC_MEDIA1_CRG_PERRSTEN_ISP_SEC_ip_rst_isp_i2c_START (6)
#define SOC_MEDIA1_CRG_PERRSTEN_ISP_SEC_ip_rst_isp_i2c_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_isp : 1;
        unsigned int ip_arst_isp : 1;
        unsigned int ip_rst_isp_in_adb_n : 1;
        unsigned int ip_rst_isp_cputocfg : 1;
        unsigned int ip_rst_isp_cpu : 1;
        unsigned int ip_rst_isp_sys : 1;
        unsigned int ip_rst_isp_i2c : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_MEDIA1_CRG_PERRSTDIS_ISP_SEC_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTDIS_ISP_SEC_ip_rst_isp_START (0)
#define SOC_MEDIA1_CRG_PERRSTDIS_ISP_SEC_ip_rst_isp_END (0)
#define SOC_MEDIA1_CRG_PERRSTDIS_ISP_SEC_ip_arst_isp_START (1)
#define SOC_MEDIA1_CRG_PERRSTDIS_ISP_SEC_ip_arst_isp_END (1)
#define SOC_MEDIA1_CRG_PERRSTDIS_ISP_SEC_ip_rst_isp_in_adb_n_START (2)
#define SOC_MEDIA1_CRG_PERRSTDIS_ISP_SEC_ip_rst_isp_in_adb_n_END (2)
#define SOC_MEDIA1_CRG_PERRSTDIS_ISP_SEC_ip_rst_isp_cputocfg_START (3)
#define SOC_MEDIA1_CRG_PERRSTDIS_ISP_SEC_ip_rst_isp_cputocfg_END (3)
#define SOC_MEDIA1_CRG_PERRSTDIS_ISP_SEC_ip_rst_isp_cpu_START (4)
#define SOC_MEDIA1_CRG_PERRSTDIS_ISP_SEC_ip_rst_isp_cpu_END (4)
#define SOC_MEDIA1_CRG_PERRSTDIS_ISP_SEC_ip_rst_isp_sys_START (5)
#define SOC_MEDIA1_CRG_PERRSTDIS_ISP_SEC_ip_rst_isp_sys_END (5)
#define SOC_MEDIA1_CRG_PERRSTDIS_ISP_SEC_ip_rst_isp_i2c_START (6)
#define SOC_MEDIA1_CRG_PERRSTDIS_ISP_SEC_ip_rst_isp_i2c_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_isp : 1;
        unsigned int ip_arst_isp : 1;
        unsigned int ip_rst_isp_in_adb_n : 1;
        unsigned int ip_rst_isp_cputocfg : 1;
        unsigned int ip_rst_isp_cpu : 1;
        unsigned int ip_rst_isp_sys : 1;
        unsigned int ip_rst_isp_i2c : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_MEDIA1_CRG_PERRSTSTAT_ISP_SEC_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTSTAT_ISP_SEC_ip_rst_isp_START (0)
#define SOC_MEDIA1_CRG_PERRSTSTAT_ISP_SEC_ip_rst_isp_END (0)
#define SOC_MEDIA1_CRG_PERRSTSTAT_ISP_SEC_ip_arst_isp_START (1)
#define SOC_MEDIA1_CRG_PERRSTSTAT_ISP_SEC_ip_arst_isp_END (1)
#define SOC_MEDIA1_CRG_PERRSTSTAT_ISP_SEC_ip_rst_isp_in_adb_n_START (2)
#define SOC_MEDIA1_CRG_PERRSTSTAT_ISP_SEC_ip_rst_isp_in_adb_n_END (2)
#define SOC_MEDIA1_CRG_PERRSTSTAT_ISP_SEC_ip_rst_isp_cputocfg_START (3)
#define SOC_MEDIA1_CRG_PERRSTSTAT_ISP_SEC_ip_rst_isp_cputocfg_END (3)
#define SOC_MEDIA1_CRG_PERRSTSTAT_ISP_SEC_ip_rst_isp_cpu_START (4)
#define SOC_MEDIA1_CRG_PERRSTSTAT_ISP_SEC_ip_rst_isp_cpu_END (4)
#define SOC_MEDIA1_CRG_PERRSTSTAT_ISP_SEC_ip_rst_isp_sys_START (5)
#define SOC_MEDIA1_CRG_PERRSTSTAT_ISP_SEC_ip_rst_isp_sys_END (5)
#define SOC_MEDIA1_CRG_PERRSTSTAT_ISP_SEC_ip_rst_isp_i2c_START (6)
#define SOC_MEDIA1_CRG_PERRSTSTAT_ISP_SEC_ip_rst_isp_i2c_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cfgend : 1;
        unsigned int vinithi : 1;
        unsigned int initram0 : 1;
        unsigned int cfgnmfi : 1;
        unsigned int cpuhalt : 1;
        unsigned int reserved : 1;
        unsigned int isp_r8_mem_ctrl_s : 26;
    } reg;
} SOC_MEDIA1_CRG_ISPCPU_CTRL0_SEC_UNION;
#endif
#define SOC_MEDIA1_CRG_ISPCPU_CTRL0_SEC_cfgend_START (0)
#define SOC_MEDIA1_CRG_ISPCPU_CTRL0_SEC_cfgend_END (0)
#define SOC_MEDIA1_CRG_ISPCPU_CTRL0_SEC_vinithi_START (1)
#define SOC_MEDIA1_CRG_ISPCPU_CTRL0_SEC_vinithi_END (1)
#define SOC_MEDIA1_CRG_ISPCPU_CTRL0_SEC_initram0_START (2)
#define SOC_MEDIA1_CRG_ISPCPU_CTRL0_SEC_initram0_END (2)
#define SOC_MEDIA1_CRG_ISPCPU_CTRL0_SEC_cfgnmfi_START (3)
#define SOC_MEDIA1_CRG_ISPCPU_CTRL0_SEC_cfgnmfi_END (3)
#define SOC_MEDIA1_CRG_ISPCPU_CTRL0_SEC_cpuhalt_START (4)
#define SOC_MEDIA1_CRG_ISPCPU_CTRL0_SEC_cpuhalt_END (4)
#define SOC_MEDIA1_CRG_ISPCPU_CTRL0_SEC_isp_r8_mem_ctrl_s_START (6)
#define SOC_MEDIA1_CRG_ISPCPU_CTRL0_SEC_isp_r8_mem_ctrl_s_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int isp_pwrqactives : 3;
        unsigned int isp_pwrqdenys : 3;
        unsigned int isp_pwrqacceptns : 3;
        unsigned int reserved : 23;
    } reg;
} SOC_MEDIA1_CRG_ISP_ADB_LP_OUT_UNION;
#endif
#define SOC_MEDIA1_CRG_ISP_ADB_LP_OUT_isp_pwrqactives_START (0)
#define SOC_MEDIA1_CRG_ISP_ADB_LP_OUT_isp_pwrqactives_END (2)
#define SOC_MEDIA1_CRG_ISP_ADB_LP_OUT_isp_pwrqdenys_START (3)
#define SOC_MEDIA1_CRG_ISP_ADB_LP_OUT_isp_pwrqdenys_END (5)
#define SOC_MEDIA1_CRG_ISP_ADB_LP_OUT_isp_pwrqacceptns_START (6)
#define SOC_MEDIA1_CRG_ISP_ADB_LP_OUT_isp_pwrqacceptns_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int isp_pwrqreqns : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_MEDIA1_CRG_ISP_ADB_LP_IN_UNION;
#endif
#define SOC_MEDIA1_CRG_ISP_ADB_LP_IN_isp_pwrqreqns_START (0)
#define SOC_MEDIA1_CRG_ISP_ADB_LP_IN_isp_pwrqreqns_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_ivp32dsp_subsys_crg : 1;
        unsigned int ip_rst_ivp32dsp_subsys : 1;
        unsigned int ip_rst_ispfunc_to_ivp : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_MEDIA1_CRG_PERRSTEN_SEC_IVP_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTEN_SEC_IVP_ip_rst_ivp32dsp_subsys_crg_START (0)
#define SOC_MEDIA1_CRG_PERRSTEN_SEC_IVP_ip_rst_ivp32dsp_subsys_crg_END (0)
#define SOC_MEDIA1_CRG_PERRSTEN_SEC_IVP_ip_rst_ivp32dsp_subsys_START (1)
#define SOC_MEDIA1_CRG_PERRSTEN_SEC_IVP_ip_rst_ivp32dsp_subsys_END (1)
#define SOC_MEDIA1_CRG_PERRSTEN_SEC_IVP_ip_rst_ispfunc_to_ivp_START (2)
#define SOC_MEDIA1_CRG_PERRSTEN_SEC_IVP_ip_rst_ispfunc_to_ivp_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_ivp32dsp_subsys_crg : 1;
        unsigned int ip_rst_ivp32dsp_subsys : 1;
        unsigned int ip_rst_ispfunc_to_ivp : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_MEDIA1_CRG_PERRSTDIS_SEC_IVP_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTDIS_SEC_IVP_ip_rst_ivp32dsp_subsys_crg_START (0)
#define SOC_MEDIA1_CRG_PERRSTDIS_SEC_IVP_ip_rst_ivp32dsp_subsys_crg_END (0)
#define SOC_MEDIA1_CRG_PERRSTDIS_SEC_IVP_ip_rst_ivp32dsp_subsys_START (1)
#define SOC_MEDIA1_CRG_PERRSTDIS_SEC_IVP_ip_rst_ivp32dsp_subsys_END (1)
#define SOC_MEDIA1_CRG_PERRSTDIS_SEC_IVP_ip_rst_ispfunc_to_ivp_START (2)
#define SOC_MEDIA1_CRG_PERRSTDIS_SEC_IVP_ip_rst_ispfunc_to_ivp_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_ivp32dsp_subsys_crg : 1;
        unsigned int ip_rst_ivp32dsp_subsys : 1;
        unsigned int ip_rst_ispfunc_to_ivp : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_MEDIA1_CRG_PERRSTSTAT_SEC_IVP_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTSTAT_SEC_IVP_ip_rst_ivp32dsp_subsys_crg_START (0)
#define SOC_MEDIA1_CRG_PERRSTSTAT_SEC_IVP_ip_rst_ivp32dsp_subsys_crg_END (0)
#define SOC_MEDIA1_CRG_PERRSTSTAT_SEC_IVP_ip_rst_ivp32dsp_subsys_START (1)
#define SOC_MEDIA1_CRG_PERRSTSTAT_SEC_IVP_ip_rst_ivp32dsp_subsys_END (1)
#define SOC_MEDIA1_CRG_PERRSTSTAT_SEC_IVP_ip_rst_ispfunc_to_ivp_START (2)
#define SOC_MEDIA1_CRG_PERRSTSTAT_SEC_IVP_ip_rst_ispfunc_to_ivp_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_arst_asc : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_MEDIA1_CRG_PERRSTEN_GENERAL_SEC_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTEN_GENERAL_SEC_ip_arst_asc_START (0)
#define SOC_MEDIA1_CRG_PERRSTEN_GENERAL_SEC_ip_arst_asc_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_arst_asc : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_MEDIA1_CRG_PERRSTDIS_GENERAL_SEC_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTDIS_GENERAL_SEC_ip_arst_asc_START (0)
#define SOC_MEDIA1_CRG_PERRSTDIS_GENERAL_SEC_ip_arst_asc_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_arst_asc : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_MEDIA1_CRG_PERRSTSTAT_GENERAL_SEC_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTSTAT_GENERAL_SEC_ip_arst_asc_START (0)
#define SOC_MEDIA1_CRG_PERRSTSTAT_GENERAL_SEC_ip_arst_asc_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tz_secure_n_med1_lm : 1;
        unsigned int tz_secure_med1_ics2_bcu : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_MEDIA1_CRG_DECPROT0SET_UNION;
#endif
#define SOC_MEDIA1_CRG_DECPROT0SET_tz_secure_n_med1_lm_START (0)
#define SOC_MEDIA1_CRG_DECPROT0SET_tz_secure_n_med1_lm_END (0)
#define SOC_MEDIA1_CRG_DECPROT0SET_tz_secure_med1_ics2_bcu_START (1)
#define SOC_MEDIA1_CRG_DECPROT0SET_tz_secure_med1_ics2_bcu_END (1)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
