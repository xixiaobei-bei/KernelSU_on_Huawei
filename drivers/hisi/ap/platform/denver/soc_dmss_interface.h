#ifndef __SOC_DMSS_INTERFACE_H__
#define __SOC_DMSS_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_DMSS_ASI_RTL_INF0_ADDR(base,asi_base) ((base) + (0x000+0x800*(asi_base)))
#define SOC_DMSS_ASI_RTL_INF1_ADDR(base,asi_base) ((base) + (0x004+0x800*(asi_base)))
#define SOC_DMSS_ASI_RTL_INF2_ADDR(base,asi_base) ((base) + (0x008+0x800*(asi_base)))
#define SOC_DMSS_ASI_STA_CKG_ADDR(base,asi_base) ((base) + (0x010+0x800*(asi_base)))
#define SOC_DMSS_ASI_DYN_CKG_ADDR(base,asi_base) ((base) + (0x014+0x800*(asi_base)))
#define SOC_DMSS_ASI_ASYNC_BRG_ADDR(base,asi_base) ((base) + (0x018+0x800*(asi_base)))
#define SOC_DMSS_ASI_ADDR_SHIFT_ADDR(base,asi_base) ((base) + (0x020+0x800*(asi_base)))
#define SOC_DMSS_ASI_RATE_ADPT_ADDR(base,asi_base) ((base) + (0x030+0x800*(asi_base)))
#define SOC_DMSS_ASI_ANTI_DEADLOCK_ADDR(base,asi_base) ((base) + (0x038+0x800*(asi_base)))
#define SOC_DMSS_ASI_RDR_CTRL_ADDR(base,rdr_filters,asi_base) ((base) + (0x040+0x4*(rdr_filters)+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_CTRL_ADDR(base,asi_base) ((base) + (0x100+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_WRPRI_ADDR(base,asi_base) ((base) + (0x104+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RDPRI_ADDR(base,asi_base) ((base) + (0x108+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_ADPT_ADDR(base,asi_base) ((base) + (0x10C+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LATENCY_ADDR(base,asi_base) ((base) + (0x110+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RGLR0_ADDR(base,asi_base) ((base) + (0x120+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RGLR0_PRI_ADDR(base,asi_base) ((base) + (0x124+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RGLR1_ADDR(base,asi_base) ((base) + (0x128+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RGLR1_PRI_ADDR(base,asi_base) ((base) + (0x12C+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LMTR0_ADDR(base,asi_base) ((base) + (0x140+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LMTR1_ADDR(base,asi_base) ((base) + (0x144+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LMTR2_ADDR(base,asi_base) ((base) + (0x148+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LMTR3_ADDR(base,asi_base) ((base) + (0x14C+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LMTR4_ADDR(base,asi_base) ((base) + (0x150+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RDOSTD_ADDR(base,asi_base) ((base) + (0x160+0x800*(asi_base)))
#define SOC_DMSS_ASI_PUSH_WRMID_ADDR(base,asi_base) ((base) + (0x180+0x800*(asi_base)))
#define SOC_DMSS_ASI_PUSH_RDMID_ADDR(base,asi_base) ((base) + (0x184+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_MASK_ADDR(base,asi_base) ((base) + (0x200+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_FLUX_FILTER0_ADDR(base,asi_base) ((base) + (0x210+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_FLUX_FILTER1_ADDR(base,asi_base) ((base) + (0x214+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_FLUX_WR_ADDR(base,asi_base) ((base) + (0x218+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_FLUX_RD_ADDR(base,asi_base) ((base) + (0x21C+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_LMTR0_ADDR(base,asi_base) ((base) + (0x220+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_LMTR1_ADDR(base,asi_base) ((base) + (0x224+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_LMTR2_ADDR(base,asi_base) ((base) + (0x228+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_LMTR3_ADDR(base,asi_base) ((base) + (0x22C+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_RDLAT_ADDR(base,asi_base) ((base) + (0x230+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_RDNUM_ADDR(base,asi_base) ((base) + (0x234+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_BLOCK_WR_ADDR(base,asi_base) ((base) + (0x238+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_BLOCK_RD_ADDR(base,asi_base) ((base) + (0x23C+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_PRI01_ADDR(base,asi_base) ((base) + (0x240+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_PRI23_ADDR(base,asi_base) ((base) + (0x244+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_PRI45_ADDR(base,asi_base) ((base) + (0x248+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_PRI67_ADDR(base,asi_base) ((base) + (0x24C+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_LMTR4_ADDR(base,asi_base) ((base) + (0x250+0x800*(asi_base)))
#define SOC_DMSS_ASI_DETECT_CFG0_ADDR(base,asi_base) ((base) + (0x280+0x800*(asi_base)))
#define SOC_DMSS_ASI_DETECT_CFG1_ADDR(base,asi_base) ((base) + (0x284+0x800*(asi_base)))
#define SOC_DMSS_ASI_DETECT_INF0_ADDR(base,asi_base) ((base) + (0x288+0x800*(asi_base)))
#define SOC_DMSS_ASI_WAKEUP_CFG_ADDR(base,asi_base) ((base) + (0x290+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_SUB_RGN0_ADDR(base,asi_sub_rgns,asi_base) ((base) + (0x300+0x10*(asi_sub_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_SUB_RGN1_ADDR(base,asi_sub_rgns,asi_base) ((base) + (0x304+0x10*(asi_sub_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_SUB_RGN2_ADDR(base,asi_sub_rgns,asi_base) ((base) + (0x308+0x10*(asi_sub_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_SUB_RGN3_ADDR(base,asi_sub_rgns,asi_base) ((base) + (0x30C+0x10*(asi_sub_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_LD_SEL_ADDR(base,asi_base) ((base) + (0x410+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_INT_EN_ADDR(base,asi_base) ((base) + (0x420+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_INT_STATUS_ADDR(base,asi_base) ((base) + (0x424+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF0_ADDR(base,asi_base) ((base) + (0x480+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_ADDR(base,asi_base) ((base) + (0x484+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF2_ADDR(base,asi_base) ((base) + (0x488+0x800*(asi_base)))
#define SOC_DMSS_ASI_AMI_PROT_INT_EN_ADDR(base,asi_base) ((base) + (0x4A0+0x800*(asi_base)))
#define SOC_DMSS_ASI_AMI_PROT_INT_STATUS_ADDR(base,asi_base) ((base) + (0x4A4+0x800*(asi_base)))
#define SOC_DMSS_ASI_AMI_PROT_CMD_INF0_ADDR(base,asi_base) ((base) + (0x4A8+0x800*(asi_base)))
#define SOC_DMSS_ASI_AMI_PROT_CMD_INF1_ADDR(base,asi_base) ((base) + (0x4AC+0x800*(asi_base)))
#define SOC_DMSS_ASI_AMI_ENHN_INT_EN_ADDR(base,asi_base) ((base) + (0x4B0+0x800*(asi_base)))
#define SOC_DMSS_ASI_AMI_ENHN_INT_STATUS_ADDR(base,asi_base) ((base) + (0x4B4+0x800*(asi_base)))
#define SOC_DMSS_ASI_AMI_ENHN_CMD_INF0_ADDR(base,asi_base) ((base) + (0x4B8+0x800*(asi_base)))
#define SOC_DMSS_ASI_AMI_ENHN_CMD_INF1_ADDR(base,asi_base) ((base) + (0x4BC+0x800*(asi_base)))
#define SOC_DMSS_ASI_MPU_TRUST_WR_NTRUST0_ADDR(base,asi_base) ((base) + (0x4C0+0x800*(asi_base)))
#define SOC_DMSS_ASI_MPU_TRUST_WR_NTRUST1_ADDR(base,asi_base) ((base) + (0x4C4+0x800*(asi_base)))
#define SOC_DMSS_ASI_MPU_TRUST_WR_PROT0_ADDR(base,asi_base) ((base) + (0x4C8+0x800*(asi_base)))
#define SOC_DMSS_ASI_MPU_TRUST_WR_PROT1_ADDR(base,asi_base) ((base) + (0x4CC+0x800*(asi_base)))
#define SOC_DMSS_ASI_MPU_PROT_WR_NTRUST0_ADDR(base,asi_base) ((base) + (0x4D0+0x800*(asi_base)))
#define SOC_DMSS_ASI_MPU_PROT_WR_NTRUST1_ADDR(base,asi_base) ((base) + (0x4D4+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_RGN_MAP0_ADDR(base,asi_rgns,asi_base) ((base) + (0x500+0x10*(asi_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_RGN_MAP1_ADDR(base,asi_rgns,asi_base) ((base) + (0x504+0x10*(asi_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_MID_WR_ADDR(base,asi_rgns,asi_base) ((base) + (0x508+0x10*(asi_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_MID_RD_ADDR(base,asi_rgns,asi_base) ((base) + (0x50C+0x10*(asi_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_MODULE_ADDR(base,asi_base) ((base) + (0x700+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_FIFO_ADDR(base,asi_base) ((base) + (0x704+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_FSM_ADDR(base,asi_base) ((base) + (0x708+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_OSTD_ADDR(base,asi_base) ((base) + (0x70C+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_RDR0_ADDR(base,asi_base) ((base) + (0x710+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_RDR1_ADDR(base,asi_base) ((base) + (0x714+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_WOOO_ADDR(base,asi_base) ((base) + (0x718+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_INT_EN_ADDR(base,asi_base) ((base) + (0x720+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_INT_STATUS_ADDR(base,asi_base) ((base) + (0x724+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_ABNM_INF0_ADDR(base,asi_base) ((base) + (0x730+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_ABNM_INF1_ADDR(base,asi_base) ((base) + (0x734+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_ABNM_INF2_ADDR(base,asi_base) ((base) + (0x738+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_SEC0_ADDR(base,asi_base) ((base) + (0x740+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_SEC1_ADDR(base,asi_base) ((base) + (0x744+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_SEC2_ADDR(base,asi_base) ((base) + (0x748+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_SEC3_ADDR(base,asi_base) ((base) + (0x74C+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_LOAD_ADDR(base,asi_base) ((base) + (0x760+0x800*(asi_base)))
#define SOC_DMSS_DMI_RTL_INF0_ADDR(base) ((base) + (0x10000UL))
#define SOC_DMSS_DMI_DYN_CKG_ADDR(base) ((base) + (0x10010UL))
#define SOC_DMSS_DMI_RD_ADPT_ADDR(base) ((base) + (0x10020UL))
#define SOC_DMSS_DMI_WR_ADPT_ADDR(base) ((base) + (0x10024UL))
#define SOC_DMSS_DMI_RD_TIMEOUT0_ADDR(base) ((base) + (0x10030UL))
#define SOC_DMSS_DMI_RD_TIMEOUT1_ADDR(base) ((base) + (0x10034UL))
#define SOC_DMSS_DMI_WR_TIMEOUT0_ADDR(base) ((base) + (0x10038UL))
#define SOC_DMSS_DMI_WR_TIMEOUT1_ADDR(base) ((base) + (0x1003CUL))
#define SOC_DMSS_DMI_GID_RGN_MAP0_ADDR(base,gid_rgns) ((base) + (0x10040+0x8*(gid_rgns)))
#define SOC_DMSS_DMI_GID_RGN_MAP1_ADDR(base,gid_rgns) ((base) + (0x10044+0x8*(gid_rgns)))
#define SOC_DMSS_DMI_SEC_LD_SEL_ADDR(base) ((base) + (0x10080UL))
#define SOC_DMSS_DMI_SEC_INT_EN_ADDR(base) ((base) + (0x10084UL))
#define SOC_DMSS_DMI_SEC_INT_STATUS_ADDR(base,dmi_base) ((base) + (0x10088+0x800*(dmi_base)))
#define SOC_DMSS_DMI_ENHN_INT_EN_ADDR(base) ((base) + (0x1008CUL))
#define SOC_DMSS_DMI_ENHN_INT_STATUS_ADDR(base,dmi_base) ((base) + (0x10090+0x800*(dmi_base)))
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF0_ADDR(base,dmi_base) ((base) + (0x10094+0x800*(dmi_base)))
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_ADDR(base,dmi_base) ((base) + (0x10098+0x800*(dmi_base)))
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF2_ADDR(base,dmi_base) ((base) + (0x1009C+0x800*(dmi_base)))
#define SOC_DMSS_DMI_MPU_INT_EN_ADDR(base) ((base) + (0x100A0UL))
#define SOC_DMSS_DMI_MPU_INT_STATUS_ADDR(base,dmi_base) ((base) + (0x100A4+0x800*(dmi_base)))
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF0_ADDR(base,dmi_base) ((base) + (0x100A8+0x800*(dmi_base)))
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF1_ADDR(base,dmi_base) ((base) + (0x100AC+0x800*(dmi_base)))
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_ADDR(base,dmi_base) ((base) + (0x100B0+0x800*(dmi_base)))
#define SOC_DMSS_DMI_ENHN_CFG_INT_EN_ADDR(base) ((base) + (0x100B4UL))
#define SOC_DMSS_DMI_ENHN_CFG_INT_STATUS_ADDR(base,dmi_base) ((base) + (0x100B8+0x800*(dmi_base)))
#define SOC_DMSS_DMI_SEC_RGN_MAP0_ADDR(base,dmi_rgns) ((base) + (0x10100+0x30*(dmi_rgns)))
#define SOC_DMSS_DMI_SEC_RGN_MAP1_ADDR(base,dmi_rgns) ((base) + (0x10104+0x30*(dmi_rgns)))
#define SOC_DMSS_DMI_SEC_RGN_ATTR_ADDR(base,dmi_enhn_rgns) ((base) + (0x10108+0x30*(dmi_enhn_rgns)))
#define SOC_DMSS_DMI_SEC_MID_WR0_ADDR(base,dmi_mid_rgns) ((base) + (0x10110+0x30*(dmi_mid_rgns)))
#define SOC_DMSS_DMI_SEC_MID_WR1_ADDR(base,dmi_mid_rgns) ((base) + (0x10114+0x30*(dmi_mid_rgns)))
#define SOC_DMSS_DMI_SEC_MID_WR2_ADDR(base,dmi_mid_rgns) ((base) + (0x10118+0x30*(dmi_mid_rgns)))
#define SOC_DMSS_DMI_SEC_MID_WR3_ADDR(base,dmi_mid_rgns) ((base) + (0x1011C+0x30*(dmi_mid_rgns)))
#define SOC_DMSS_DMI_SEC_MID_RD0_ADDR(base,dmi_mid_rgns) ((base) + (0x10120+0x30*(dmi_mid_rgns)))
#define SOC_DMSS_DMI_SEC_MID_RD1_ADDR(base,dmi_mid_rgns) ((base) + (0x10124+0x30*(dmi_mid_rgns)))
#define SOC_DMSS_DMI_SEC_MID_RD2_ADDR(base,dmi_mid_rgns) ((base) + (0x10128+0x30*(dmi_mid_rgns)))
#define SOC_DMSS_DMI_SEC_MID_RD3_ADDR(base,dmi_mid_rgns) ((base) + (0x1012C+0x30*(dmi_mid_rgns)))
#define SOC_DMSS_DMI_DFX_MODULE_ADDR(base,dmi_base) ((base) + (0x10700+0x800*(dmi_base)))
#define SOC_DMSS_DMI_DFX_FIFO_ADDR(base,dmi_base) ((base) + (0x10704+0x800*(dmi_base)))
#define SOC_DMSS_DMI_DFX_WD_FIFO_ADDR(base,dmi_base) ((base) + (0x10708+0x800*(dmi_base)))
#define SOC_DMSS_DMI_DFX_ST_ADDR(base,dmi_base) ((base) + (0x1070C+0x800*(dmi_base)))
#define SOC_DMSS_DMI_DFX_DCSC_ADDR(base,dmi_base) ((base) + (0x10710+0x800*(dmi_base)))
#define SOC_DMSS_AMI_RTL_INF0_ADDR(base,ami_base) ((base) + (0x12000+0x800*(ami_base)))
#define SOC_DMSS_AMI_DYN_CKG_ADDR(base,ami_base) ((base) + (0x12010+0x800*(ami_base)))
#define SOC_DMSS_AMI_GID_CTRL_ADDR(base,ami_base) ((base) + (0x12020+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_LD_SEL_ADDR(base,ami_base) ((base) + (0x12080+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_INT_EN_ADDR(base,ami_base) ((base) + (0x12084+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_INT_STATUS_ADDR(base,ami_base) ((base) + (0x12088+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF0_ADDR(base,ami_base) ((base) + (0x12094+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_ADDR(base,ami_base) ((base) + (0x12098+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF2_ADDR(base,ami_base) ((base) + (0x1209C+0x800*(ami_base)))
#define SOC_DMSS_AMI_MPU_INT_EN_ADDR(base,ami_base) ((base) + (0x120A0+0x800*(ami_base)))
#define SOC_DMSS_AMI_MPU_INT_STATUS_ADDR(base,ami_base) ((base) + (0x120A4+0x800*(ami_base)))
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF0_ADDR(base,ami_base) ((base) + (0x120A8+0x800*(ami_base)))
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF1_ADDR(base,ami_base) ((base) + (0x120AC+0x800*(ami_base)))
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF2_ADDR(base,ami_base) ((base) + (0x120B0+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_RGN_MAP0_ADDR(base,ami_rgns,ami_base) ((base) + (0x12100+0x30*(ami_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_RGN_MAP1_ADDR(base,ami_rgns,ami_base) ((base) + (0x12104+0x30*(ami_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_RGN_ATTR_ADDR(base,ami_enhn_rgns,ami_base) ((base) + (0x12108+0x30*(ami_enhn_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_MID_WR0_ADDR(base,ami_mid_rgns,ami_base) ((base) + (0x12110+0x30*(ami_mid_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_MID_WR1_ADDR(base,ami_mid_rgns,ami_base) ((base) + (0x12114+0x30*(ami_mid_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_MID_WR2_ADDR(base,ami_mid_rgns,ami_base) ((base) + (0x12118+0x30*(ami_mid_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_MID_WR3_ADDR(base,ami_mid_rgns,ami_base) ((base) + (0x1211C+0x30*(ami_mid_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_MID_RD0_ADDR(base,ami_mid_rgns,ami_base) ((base) + (0x12120+0x30*(ami_mid_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_MID_RD1_ADDR(base,ami_mid_rgns,ami_base) ((base) + (0x12124+0x30*(ami_mid_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_MID_RD2_ADDR(base,ami_mid_rgns,ami_base) ((base) + (0x12128+0x30*(ami_mid_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_MID_RD3_ADDR(base,ami_mid_rgns,ami_base) ((base) + (0x1212C+0x30*(ami_mid_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_DFX_MODULE_ADDR(base,ami_base) ((base) + (0x12700+0x800*(ami_base)))
#define SOC_DMSS_AMI_DFX_FIFO_ADDR(base,ami_base) ((base) + (0x12704+0x800*(ami_base)))
#define SOC_DMSS_AMI_DFX_ST_ADDR(base,ami_base) ((base) + (0x12708+0x800*(ami_base)))
#define SOC_DMSS_GLB_RTL_VER_ADDR(base) ((base) + (0x16000UL))
#define SOC_DMSS_GLB_RTL_INF0_ADDR(base) ((base) + (0x16004UL))
#define SOC_DMSS_GLB_RTL_INF1_ADDR(base) ((base) + (0x16008UL))
#define SOC_DMSS_GLB_RAM_TMOD0_ADDR(base) ((base) + (0x16010UL))
#define SOC_DMSS_GLB_RAM_TMOD1_ADDR(base) ((base) + (0x16014UL))
#define SOC_DMSS_GLB_RAM_SMOD_ADDR(base) ((base) + (0x16018UL))
#define SOC_DMSS_GLB_DYN_CKG_ADDR(base) ((base) + (0x16024UL))
#define SOC_DMSS_GLB_INT_STATUS0_ADDR(base) ((base) + (0x16030UL))
#define SOC_DMSS_GLB_INT_CLEAR0_ADDR(base) ((base) + (0x16034UL))
#define SOC_DMSS_GLB_INT_STATUS1_ADDR(base) ((base) + (0x16038UL))
#define SOC_DMSS_GLB_INT_CLEAR1_ADDR(base) ((base) + (0x1603CUL))
#define SOC_DMSS_GLB_EXCLU_ADDR(base) ((base) + (0x16040UL))
#define SOC_DMSS_GLB_ADDR_INTLV_ADDR(base) ((base) + (0x16100UL))
#define SOC_DMSS_GLB_ADDR_SCRMBL_ADDR(base) ((base) + (0x16110UL))
#define SOC_DMSS_GLB_RBUF_CTRL_ADDR(base) ((base) + (0x16148UL))
#define SOC_DMSS_GLB_RBUF_TRANS_CTRL_ADDR(base) ((base) + (0x1614CUL))
#define SOC_DMSS_GLB_RBUF_BP_ADDR(base,rbuf_bp_grps) ((base) + (0x16150+0x4*(rbuf_bp_grps)))
#define SOC_DMSS_GLB_RSP_ERR_INT_EN_ADDR(base) ((base) + (0x16180UL))
#define SOC_DMSS_GLB_RSP_ERR_INT_STATUS_ADDR(base) ((base) + (0x16184UL))
#define SOC_DMSS_GLB_AMI_CTRL_ADDR(base) ((base) + (0x161A0UL))
#define SOC_DMSS_GLB_DCQ_CTRL_ADDR(base) ((base) + (0x16200UL))
#define SOC_DMSS_GLB_DCQ_PRILVL0_ADDR(base,chans) ((base) + (0x16220+0x10*(chans)))
#define SOC_DMSS_GLB_DCQ_PRILVL1_ADDR(base,chans) ((base) + (0x16224+0x10*(chans)))
#define SOC_DMSS_GLB_DCQ_WR_PRILVL0_ADDR(base,chans) ((base) + (0x16228+0x10*(chans)))
#define SOC_DMSS_GLB_DCQ_WR_PRILVL1_ADDR(base,chans) ((base) + (0x1622C+0x10*(chans)))
#define SOC_DMSS_GLB_SC_CTRL_ADDR(base) ((base) + (0x16260UL))
#define SOC_DMSS_GLB_SC_DCQLVL_ADDR(base) ((base) + (0x16264UL))
#define SOC_DMSS_GLB_MST_FLUX_ADDR(base,dcq_msts) ((base) + (0x16280+0x4*(dcq_msts)))
#define SOC_DMSS_GLB_TRACE_CTRL0_ADDR(base) ((base) + (0x16300UL))
#define SOC_DMSS_GLB_TRACE_CTRL1_ADDR(base) ((base) + (0x16304UL))
#define SOC_DMSS_GLB_TRACE_CTRL2_ADDR(base) ((base) + (0x16308UL))
#define SOC_DMSS_GLB_TRACE_REC0_ADDR(base) ((base) + (0x16310UL))
#define SOC_DMSS_GLB_TRACE_REC1_ADDR(base) ((base) + (0x16314UL))
#define SOC_DMSS_GLB_TRACE_FILTER0_ADDR(base) ((base) + (0x16318UL))
#define SOC_DMSS_GLB_TRACE_FILTER1_ADDR(base) ((base) + (0x1631CUL))
#define SOC_DMSS_GLB_TRACE_FILTER2_ADDR(base,mid_grps) ((base) + (0x16320+0x4*(mid_grps)))
#define SOC_DMSS_GLB_TRACE_FREQ_ADDR(base) ((base) + (0x16340UL))
#define SOC_DMSS_GLB_TRACE_INT_EN_ADDR(base) ((base) + (0x16350UL))
#define SOC_DMSS_GLB_TRACE_LOCK_DOWN_ADDR(base) ((base) + (0x16358UL))
#define SOC_DMSS_GLB_TRACE_STAT0_ADDR(base,dmis) ((base) + (0x16360+0x10*(dmis)))
#define SOC_DMSS_GLB_TRACE_STAT1_ADDR(base,dmis) ((base) + (0x16364+0x10*(dmis)))
#define SOC_DMSS_GLB_TRACE_STAT2_ADDR(base,dmis) ((base) + (0x16368+0x10*(dmis)))
#define SOC_DMSS_GLB_MPU_CFG_ADDR(base) ((base) + (0x16380UL))
#define SOC_DMSS_GLB_MPU_ERROR_ADDR_ADDR(base) ((base) + (0x16384UL))
#define SOC_DMSS_GLB_MPU_ACPU_CFG_ADDR(base) ((base) + (0x16388UL))
#define SOC_DMSS_GLB_SEC_BYPASS_ADDR(base) ((base) + (0x1638CUL))
#define SOC_DMSS_GLB_STAT_CTRL_ADDR(base) ((base) + (0x16400UL))
#define SOC_DMSS_GLB_STAT_CYCLE_ADDR(base) ((base) + (0x16410UL))
#define SOC_DMSS_GLB_STAT_PERFSTAT_ADDR(base) ((base) + (0x16420UL))
#define SOC_DMSS_GLB_STAT_DCQ_WR_ADDR(base,chans) ((base) + (0x16440+0x4*(chans)))
#define SOC_DMSS_GLB_STAT_DCQ_ADDR(base,chans) ((base) + (0x16450+0x4*(chans)))
#define SOC_DMSS_GLB_STAT_WR_ALL_ALLOW_ADDR(base,chans) ((base) + (0x16500+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_WR_PRI1_ALLOW_ADDR(base,chans) ((base) + (0x16504+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_WR_PRI2_ALLOW_ADDR(base,chans) ((base) + (0x16508+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_WR_PRI3_ALLOW_ADDR(base,chans) ((base) + (0x1650C+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_WR_PRI4_ALLOW_ADDR(base,chans) ((base) + (0x16510+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_WR_PRI5_ALLOW_ADDR(base,chans) ((base) + (0x16514+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_WR_PRI6_ALLOW_ADDR(base,chans) ((base) + (0x16518+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_WR_PRI7_ALLOW_ADDR(base,chans) ((base) + (0x1651C+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RD_ALL_ALLOW_ADDR(base,chans) ((base) + (0x16520+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RD_PRI1_ALLOW_ADDR(base,chans) ((base) + (0x16524+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RD_PRI2_ALLOW_ADDR(base,chans) ((base) + (0x16528+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RD_PRI3_ALLOW_ADDR(base,chans) ((base) + (0x1652C+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RD_PRI4_ALLOW_ADDR(base,chans) ((base) + (0x16530+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RD_PRI5_ALLOW_ADDR(base,chans) ((base) + (0x16534+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RD_PRI6_ALLOW_ADDR(base,chans) ((base) + (0x16538+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RD_PRI7_ALLOW_ADDR(base,chans) ((base) + (0x1653C+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RBUF_CTRL_ADDR(base) ((base) + (0x16600UL))
#define SOC_DMSS_GLB_STAT_RBUF_OCCUP0_ADDR(base) ((base) + (0x16604UL))
#define SOC_DMSS_GLB_STAT_RBUF_OCCUP1_ADDR(base) ((base) + (0x16608UL))
#define SOC_DMSS_GLB_STAT_RBUF_OCCUP2_ADDR(base) ((base) + (0x1660CUL))
#define SOC_DMSS_GLB_STAT_AMI_WFIFO_ADDR(base) ((base) + (0x16640UL))
#define SOC_DMSS_GLB_DFX_MODULE_ADDR(base) ((base) + (0x16700UL))
#define SOC_DMSS_GLB_DFX_FIFO_ADDR(base) ((base) + (0x16704UL))
#define SOC_DMSS_GLB_DFX_RBUF_FIFO_ADDR(base) ((base) + (0x16708UL))
#define SOC_DMSS_GLB_DFX_DCQ_ST_ADDR(base,chans) ((base) + (0x16710+0x4*(chans)))
#define SOC_DMSS_GLB_DFX_RBUF0_ADDR(base,rbuf_srcs) ((base) + (0x16720+0x10*(rbuf_srcs)))
#define SOC_DMSS_GLB_DFX_RBUF1_ADDR(base,rbuf_srcs) ((base) + (0x16724+0x10*(rbuf_srcs)))
#define SOC_DMSS_GLB_DFX_DUMMY0_ADDR(base) ((base) + (0x167E0UL))
#define SOC_DMSS_GLB_DFX_DUMMY1_ADDR(base) ((base) + (0x167E4UL))
#define SOC_DMSS_GLB_DFX_DUMMY2_ADDR(base) ((base) + (0x167E8UL))
#define SOC_DMSS_GLB_DFX_DUMMY3_ADDR(base) ((base) + (0x167ECUL))
#define SOC_DMSS_GLB_DFX_LOAD_ADDR(base) ((base) + (0x167F0UL))
#define SOC_DMSS_MPU_ADDR_ATTR_ADDR(base,mpu_regs) ((base) + (0x20000+0x4*(mpu_regs)))
#else
#define SOC_DMSS_ASI_RTL_INF0_ADDR(base,asi_base) ((base) + (0x000+0x800*(asi_base)))
#define SOC_DMSS_ASI_RTL_INF1_ADDR(base,asi_base) ((base) + (0x004+0x800*(asi_base)))
#define SOC_DMSS_ASI_RTL_INF2_ADDR(base,asi_base) ((base) + (0x008+0x800*(asi_base)))
#define SOC_DMSS_ASI_STA_CKG_ADDR(base,asi_base) ((base) + (0x010+0x800*(asi_base)))
#define SOC_DMSS_ASI_DYN_CKG_ADDR(base,asi_base) ((base) + (0x014+0x800*(asi_base)))
#define SOC_DMSS_ASI_ASYNC_BRG_ADDR(base,asi_base) ((base) + (0x018+0x800*(asi_base)))
#define SOC_DMSS_ASI_ADDR_SHIFT_ADDR(base,asi_base) ((base) + (0x020+0x800*(asi_base)))
#define SOC_DMSS_ASI_RATE_ADPT_ADDR(base,asi_base) ((base) + (0x030+0x800*(asi_base)))
#define SOC_DMSS_ASI_ANTI_DEADLOCK_ADDR(base,asi_base) ((base) + (0x038+0x800*(asi_base)))
#define SOC_DMSS_ASI_RDR_CTRL_ADDR(base,rdr_filters,asi_base) ((base) + (0x040+0x4*(rdr_filters)+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_CTRL_ADDR(base,asi_base) ((base) + (0x100+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_WRPRI_ADDR(base,asi_base) ((base) + (0x104+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RDPRI_ADDR(base,asi_base) ((base) + (0x108+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_ADPT_ADDR(base,asi_base) ((base) + (0x10C+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LATENCY_ADDR(base,asi_base) ((base) + (0x110+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RGLR0_ADDR(base,asi_base) ((base) + (0x120+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RGLR0_PRI_ADDR(base,asi_base) ((base) + (0x124+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RGLR1_ADDR(base,asi_base) ((base) + (0x128+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RGLR1_PRI_ADDR(base,asi_base) ((base) + (0x12C+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LMTR0_ADDR(base,asi_base) ((base) + (0x140+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LMTR1_ADDR(base,asi_base) ((base) + (0x144+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LMTR2_ADDR(base,asi_base) ((base) + (0x148+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LMTR3_ADDR(base,asi_base) ((base) + (0x14C+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LMTR4_ADDR(base,asi_base) ((base) + (0x150+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RDOSTD_ADDR(base,asi_base) ((base) + (0x160+0x800*(asi_base)))
#define SOC_DMSS_ASI_PUSH_WRMID_ADDR(base,asi_base) ((base) + (0x180+0x800*(asi_base)))
#define SOC_DMSS_ASI_PUSH_RDMID_ADDR(base,asi_base) ((base) + (0x184+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_MASK_ADDR(base,asi_base) ((base) + (0x200+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_FLUX_FILTER0_ADDR(base,asi_base) ((base) + (0x210+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_FLUX_FILTER1_ADDR(base,asi_base) ((base) + (0x214+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_FLUX_WR_ADDR(base,asi_base) ((base) + (0x218+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_FLUX_RD_ADDR(base,asi_base) ((base) + (0x21C+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_LMTR0_ADDR(base,asi_base) ((base) + (0x220+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_LMTR1_ADDR(base,asi_base) ((base) + (0x224+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_LMTR2_ADDR(base,asi_base) ((base) + (0x228+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_LMTR3_ADDR(base,asi_base) ((base) + (0x22C+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_RDLAT_ADDR(base,asi_base) ((base) + (0x230+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_RDNUM_ADDR(base,asi_base) ((base) + (0x234+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_BLOCK_WR_ADDR(base,asi_base) ((base) + (0x238+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_BLOCK_RD_ADDR(base,asi_base) ((base) + (0x23C+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_PRI01_ADDR(base,asi_base) ((base) + (0x240+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_PRI23_ADDR(base,asi_base) ((base) + (0x244+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_PRI45_ADDR(base,asi_base) ((base) + (0x248+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_PRI67_ADDR(base,asi_base) ((base) + (0x24C+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_LMTR4_ADDR(base,asi_base) ((base) + (0x250+0x800*(asi_base)))
#define SOC_DMSS_ASI_DETECT_CFG0_ADDR(base,asi_base) ((base) + (0x280+0x800*(asi_base)))
#define SOC_DMSS_ASI_DETECT_CFG1_ADDR(base,asi_base) ((base) + (0x284+0x800*(asi_base)))
#define SOC_DMSS_ASI_DETECT_INF0_ADDR(base,asi_base) ((base) + (0x288+0x800*(asi_base)))
#define SOC_DMSS_ASI_WAKEUP_CFG_ADDR(base,asi_base) ((base) + (0x290+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_SUB_RGN0_ADDR(base,asi_sub_rgns,asi_base) ((base) + (0x300+0x10*(asi_sub_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_SUB_RGN1_ADDR(base,asi_sub_rgns,asi_base) ((base) + (0x304+0x10*(asi_sub_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_SUB_RGN2_ADDR(base,asi_sub_rgns,asi_base) ((base) + (0x308+0x10*(asi_sub_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_SUB_RGN3_ADDR(base,asi_sub_rgns,asi_base) ((base) + (0x30C+0x10*(asi_sub_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_LD_SEL_ADDR(base,asi_base) ((base) + (0x410+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_INT_EN_ADDR(base,asi_base) ((base) + (0x420+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_INT_STATUS_ADDR(base,asi_base) ((base) + (0x424+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF0_ADDR(base,asi_base) ((base) + (0x480+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_ADDR(base,asi_base) ((base) + (0x484+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF2_ADDR(base,asi_base) ((base) + (0x488+0x800*(asi_base)))
#define SOC_DMSS_ASI_AMI_PROT_INT_EN_ADDR(base,asi_base) ((base) + (0x4A0+0x800*(asi_base)))
#define SOC_DMSS_ASI_AMI_PROT_INT_STATUS_ADDR(base,asi_base) ((base) + (0x4A4+0x800*(asi_base)))
#define SOC_DMSS_ASI_AMI_PROT_CMD_INF0_ADDR(base,asi_base) ((base) + (0x4A8+0x800*(asi_base)))
#define SOC_DMSS_ASI_AMI_PROT_CMD_INF1_ADDR(base,asi_base) ((base) + (0x4AC+0x800*(asi_base)))
#define SOC_DMSS_ASI_AMI_ENHN_INT_EN_ADDR(base,asi_base) ((base) + (0x4B0+0x800*(asi_base)))
#define SOC_DMSS_ASI_AMI_ENHN_INT_STATUS_ADDR(base,asi_base) ((base) + (0x4B4+0x800*(asi_base)))
#define SOC_DMSS_ASI_AMI_ENHN_CMD_INF0_ADDR(base,asi_base) ((base) + (0x4B8+0x800*(asi_base)))
#define SOC_DMSS_ASI_AMI_ENHN_CMD_INF1_ADDR(base,asi_base) ((base) + (0x4BC+0x800*(asi_base)))
#define SOC_DMSS_ASI_MPU_TRUST_WR_NTRUST0_ADDR(base,asi_base) ((base) + (0x4C0+0x800*(asi_base)))
#define SOC_DMSS_ASI_MPU_TRUST_WR_NTRUST1_ADDR(base,asi_base) ((base) + (0x4C4+0x800*(asi_base)))
#define SOC_DMSS_ASI_MPU_TRUST_WR_PROT0_ADDR(base,asi_base) ((base) + (0x4C8+0x800*(asi_base)))
#define SOC_DMSS_ASI_MPU_TRUST_WR_PROT1_ADDR(base,asi_base) ((base) + (0x4CC+0x800*(asi_base)))
#define SOC_DMSS_ASI_MPU_PROT_WR_NTRUST0_ADDR(base,asi_base) ((base) + (0x4D0+0x800*(asi_base)))
#define SOC_DMSS_ASI_MPU_PROT_WR_NTRUST1_ADDR(base,asi_base) ((base) + (0x4D4+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_RGN_MAP0_ADDR(base,asi_rgns,asi_base) ((base) + (0x500+0x10*(asi_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_RGN_MAP1_ADDR(base,asi_rgns,asi_base) ((base) + (0x504+0x10*(asi_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_MID_WR_ADDR(base,asi_rgns,asi_base) ((base) + (0x508+0x10*(asi_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_MID_RD_ADDR(base,asi_rgns,asi_base) ((base) + (0x50C+0x10*(asi_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_MODULE_ADDR(base,asi_base) ((base) + (0x700+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_FIFO_ADDR(base,asi_base) ((base) + (0x704+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_FSM_ADDR(base,asi_base) ((base) + (0x708+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_OSTD_ADDR(base,asi_base) ((base) + (0x70C+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_RDR0_ADDR(base,asi_base) ((base) + (0x710+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_RDR1_ADDR(base,asi_base) ((base) + (0x714+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_WOOO_ADDR(base,asi_base) ((base) + (0x718+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_INT_EN_ADDR(base,asi_base) ((base) + (0x720+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_INT_STATUS_ADDR(base,asi_base) ((base) + (0x724+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_ABNM_INF0_ADDR(base,asi_base) ((base) + (0x730+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_ABNM_INF1_ADDR(base,asi_base) ((base) + (0x734+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_ABNM_INF2_ADDR(base,asi_base) ((base) + (0x738+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_SEC0_ADDR(base,asi_base) ((base) + (0x740+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_SEC1_ADDR(base,asi_base) ((base) + (0x744+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_SEC2_ADDR(base,asi_base) ((base) + (0x748+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_SEC3_ADDR(base,asi_base) ((base) + (0x74C+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_LOAD_ADDR(base,asi_base) ((base) + (0x760+0x800*(asi_base)))
#define SOC_DMSS_DMI_RTL_INF0_ADDR(base) ((base) + (0x10000))
#define SOC_DMSS_DMI_DYN_CKG_ADDR(base) ((base) + (0x10010))
#define SOC_DMSS_DMI_RD_ADPT_ADDR(base) ((base) + (0x10020))
#define SOC_DMSS_DMI_WR_ADPT_ADDR(base) ((base) + (0x10024))
#define SOC_DMSS_DMI_RD_TIMEOUT0_ADDR(base) ((base) + (0x10030))
#define SOC_DMSS_DMI_RD_TIMEOUT1_ADDR(base) ((base) + (0x10034))
#define SOC_DMSS_DMI_WR_TIMEOUT0_ADDR(base) ((base) + (0x10038))
#define SOC_DMSS_DMI_WR_TIMEOUT1_ADDR(base) ((base) + (0x1003C))
#define SOC_DMSS_DMI_GID_RGN_MAP0_ADDR(base,gid_rgns) ((base) + (0x10040+0x8*(gid_rgns)))
#define SOC_DMSS_DMI_GID_RGN_MAP1_ADDR(base,gid_rgns) ((base) + (0x10044+0x8*(gid_rgns)))
#define SOC_DMSS_DMI_SEC_LD_SEL_ADDR(base) ((base) + (0x10080))
#define SOC_DMSS_DMI_SEC_INT_EN_ADDR(base) ((base) + (0x10084))
#define SOC_DMSS_DMI_SEC_INT_STATUS_ADDR(base,dmi_base) ((base) + (0x10088+0x800*(dmi_base)))
#define SOC_DMSS_DMI_ENHN_INT_EN_ADDR(base) ((base) + (0x1008C))
#define SOC_DMSS_DMI_ENHN_INT_STATUS_ADDR(base,dmi_base) ((base) + (0x10090+0x800*(dmi_base)))
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF0_ADDR(base,dmi_base) ((base) + (0x10094+0x800*(dmi_base)))
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_ADDR(base,dmi_base) ((base) + (0x10098+0x800*(dmi_base)))
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF2_ADDR(base,dmi_base) ((base) + (0x1009C+0x800*(dmi_base)))
#define SOC_DMSS_DMI_MPU_INT_EN_ADDR(base) ((base) + (0x100A0))
#define SOC_DMSS_DMI_MPU_INT_STATUS_ADDR(base,dmi_base) ((base) + (0x100A4+0x800*(dmi_base)))
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF0_ADDR(base,dmi_base) ((base) + (0x100A8+0x800*(dmi_base)))
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF1_ADDR(base,dmi_base) ((base) + (0x100AC+0x800*(dmi_base)))
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_ADDR(base,dmi_base) ((base) + (0x100B0+0x800*(dmi_base)))
#define SOC_DMSS_DMI_ENHN_CFG_INT_EN_ADDR(base) ((base) + (0x100B4))
#define SOC_DMSS_DMI_ENHN_CFG_INT_STATUS_ADDR(base,dmi_base) ((base) + (0x100B8+0x800*(dmi_base)))
#define SOC_DMSS_DMI_SEC_RGN_MAP0_ADDR(base,dmi_rgns) ((base) + (0x10100+0x30*(dmi_rgns)))
#define SOC_DMSS_DMI_SEC_RGN_MAP1_ADDR(base,dmi_rgns) ((base) + (0x10104+0x30*(dmi_rgns)))
#define SOC_DMSS_DMI_SEC_RGN_ATTR_ADDR(base,dmi_enhn_rgns) ((base) + (0x10108+0x30*(dmi_enhn_rgns)))
#define SOC_DMSS_DMI_SEC_MID_WR0_ADDR(base,dmi_mid_rgns) ((base) + (0x10110+0x30*(dmi_mid_rgns)))
#define SOC_DMSS_DMI_SEC_MID_WR1_ADDR(base,dmi_mid_rgns) ((base) + (0x10114+0x30*(dmi_mid_rgns)))
#define SOC_DMSS_DMI_SEC_MID_WR2_ADDR(base,dmi_mid_rgns) ((base) + (0x10118+0x30*(dmi_mid_rgns)))
#define SOC_DMSS_DMI_SEC_MID_WR3_ADDR(base,dmi_mid_rgns) ((base) + (0x1011C+0x30*(dmi_mid_rgns)))
#define SOC_DMSS_DMI_SEC_MID_RD0_ADDR(base,dmi_mid_rgns) ((base) + (0x10120+0x30*(dmi_mid_rgns)))
#define SOC_DMSS_DMI_SEC_MID_RD1_ADDR(base,dmi_mid_rgns) ((base) + (0x10124+0x30*(dmi_mid_rgns)))
#define SOC_DMSS_DMI_SEC_MID_RD2_ADDR(base,dmi_mid_rgns) ((base) + (0x10128+0x30*(dmi_mid_rgns)))
#define SOC_DMSS_DMI_SEC_MID_RD3_ADDR(base,dmi_mid_rgns) ((base) + (0x1012C+0x30*(dmi_mid_rgns)))
#define SOC_DMSS_DMI_DFX_MODULE_ADDR(base,dmi_base) ((base) + (0x10700+0x800*(dmi_base)))
#define SOC_DMSS_DMI_DFX_FIFO_ADDR(base,dmi_base) ((base) + (0x10704+0x800*(dmi_base)))
#define SOC_DMSS_DMI_DFX_WD_FIFO_ADDR(base,dmi_base) ((base) + (0x10708+0x800*(dmi_base)))
#define SOC_DMSS_DMI_DFX_ST_ADDR(base,dmi_base) ((base) + (0x1070C+0x800*(dmi_base)))
#define SOC_DMSS_DMI_DFX_DCSC_ADDR(base,dmi_base) ((base) + (0x10710+0x800*(dmi_base)))
#define SOC_DMSS_AMI_RTL_INF0_ADDR(base,ami_base) ((base) + (0x12000+0x800*(ami_base)))
#define SOC_DMSS_AMI_DYN_CKG_ADDR(base,ami_base) ((base) + (0x12010+0x800*(ami_base)))
#define SOC_DMSS_AMI_GID_CTRL_ADDR(base,ami_base) ((base) + (0x12020+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_LD_SEL_ADDR(base,ami_base) ((base) + (0x12080+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_INT_EN_ADDR(base,ami_base) ((base) + (0x12084+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_INT_STATUS_ADDR(base,ami_base) ((base) + (0x12088+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF0_ADDR(base,ami_base) ((base) + (0x12094+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_ADDR(base,ami_base) ((base) + (0x12098+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF2_ADDR(base,ami_base) ((base) + (0x1209C+0x800*(ami_base)))
#define SOC_DMSS_AMI_MPU_INT_EN_ADDR(base,ami_base) ((base) + (0x120A0+0x800*(ami_base)))
#define SOC_DMSS_AMI_MPU_INT_STATUS_ADDR(base,ami_base) ((base) + (0x120A4+0x800*(ami_base)))
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF0_ADDR(base,ami_base) ((base) + (0x120A8+0x800*(ami_base)))
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF1_ADDR(base,ami_base) ((base) + (0x120AC+0x800*(ami_base)))
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF2_ADDR(base,ami_base) ((base) + (0x120B0+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_RGN_MAP0_ADDR(base,ami_rgns,ami_base) ((base) + (0x12100+0x30*(ami_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_RGN_MAP1_ADDR(base,ami_rgns,ami_base) ((base) + (0x12104+0x30*(ami_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_RGN_ATTR_ADDR(base,ami_enhn_rgns,ami_base) ((base) + (0x12108+0x30*(ami_enhn_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_MID_WR0_ADDR(base,ami_mid_rgns,ami_base) ((base) + (0x12110+0x30*(ami_mid_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_MID_WR1_ADDR(base,ami_mid_rgns,ami_base) ((base) + (0x12114+0x30*(ami_mid_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_MID_WR2_ADDR(base,ami_mid_rgns,ami_base) ((base) + (0x12118+0x30*(ami_mid_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_MID_WR3_ADDR(base,ami_mid_rgns,ami_base) ((base) + (0x1211C+0x30*(ami_mid_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_MID_RD0_ADDR(base,ami_mid_rgns,ami_base) ((base) + (0x12120+0x30*(ami_mid_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_MID_RD1_ADDR(base,ami_mid_rgns,ami_base) ((base) + (0x12124+0x30*(ami_mid_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_MID_RD2_ADDR(base,ami_mid_rgns,ami_base) ((base) + (0x12128+0x30*(ami_mid_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_SEC_MID_RD3_ADDR(base,ami_mid_rgns,ami_base) ((base) + (0x1212C+0x30*(ami_mid_rgns)+0x800*(ami_base)))
#define SOC_DMSS_AMI_DFX_MODULE_ADDR(base,ami_base) ((base) + (0x12700+0x800*(ami_base)))
#define SOC_DMSS_AMI_DFX_FIFO_ADDR(base,ami_base) ((base) + (0x12704+0x800*(ami_base)))
#define SOC_DMSS_AMI_DFX_ST_ADDR(base,ami_base) ((base) + (0x12708+0x800*(ami_base)))
#define SOC_DMSS_GLB_RTL_VER_ADDR(base) ((base) + (0x16000))
#define SOC_DMSS_GLB_RTL_INF0_ADDR(base) ((base) + (0x16004))
#define SOC_DMSS_GLB_RTL_INF1_ADDR(base) ((base) + (0x16008))
#define SOC_DMSS_GLB_RAM_TMOD0_ADDR(base) ((base) + (0x16010))
#define SOC_DMSS_GLB_RAM_TMOD1_ADDR(base) ((base) + (0x16014))
#define SOC_DMSS_GLB_RAM_SMOD_ADDR(base) ((base) + (0x16018))
#define SOC_DMSS_GLB_DYN_CKG_ADDR(base) ((base) + (0x16024))
#define SOC_DMSS_GLB_INT_STATUS0_ADDR(base) ((base) + (0x16030))
#define SOC_DMSS_GLB_INT_CLEAR0_ADDR(base) ((base) + (0x16034))
#define SOC_DMSS_GLB_INT_STATUS1_ADDR(base) ((base) + (0x16038))
#define SOC_DMSS_GLB_INT_CLEAR1_ADDR(base) ((base) + (0x1603C))
#define SOC_DMSS_GLB_EXCLU_ADDR(base) ((base) + (0x16040))
#define SOC_DMSS_GLB_ADDR_INTLV_ADDR(base) ((base) + (0x16100))
#define SOC_DMSS_GLB_ADDR_SCRMBL_ADDR(base) ((base) + (0x16110))
#define SOC_DMSS_GLB_RBUF_CTRL_ADDR(base) ((base) + (0x16148))
#define SOC_DMSS_GLB_RBUF_TRANS_CTRL_ADDR(base) ((base) + (0x1614C))
#define SOC_DMSS_GLB_RBUF_BP_ADDR(base,rbuf_bp_grps) ((base) + (0x16150+0x4*(rbuf_bp_grps)))
#define SOC_DMSS_GLB_RSP_ERR_INT_EN_ADDR(base) ((base) + (0x16180))
#define SOC_DMSS_GLB_RSP_ERR_INT_STATUS_ADDR(base) ((base) + (0x16184))
#define SOC_DMSS_GLB_AMI_CTRL_ADDR(base) ((base) + (0x161A0))
#define SOC_DMSS_GLB_DCQ_CTRL_ADDR(base) ((base) + (0x16200))
#define SOC_DMSS_GLB_DCQ_PRILVL0_ADDR(base,chans) ((base) + (0x16220+0x10*(chans)))
#define SOC_DMSS_GLB_DCQ_PRILVL1_ADDR(base,chans) ((base) + (0x16224+0x10*(chans)))
#define SOC_DMSS_GLB_DCQ_WR_PRILVL0_ADDR(base,chans) ((base) + (0x16228+0x10*(chans)))
#define SOC_DMSS_GLB_DCQ_WR_PRILVL1_ADDR(base,chans) ((base) + (0x1622C+0x10*(chans)))
#define SOC_DMSS_GLB_SC_CTRL_ADDR(base) ((base) + (0x16260))
#define SOC_DMSS_GLB_SC_DCQLVL_ADDR(base) ((base) + (0x16264))
#define SOC_DMSS_GLB_MST_FLUX_ADDR(base,dcq_msts) ((base) + (0x16280+0x4*(dcq_msts)))
#define SOC_DMSS_GLB_TRACE_CTRL0_ADDR(base) ((base) + (0x16300))
#define SOC_DMSS_GLB_TRACE_CTRL1_ADDR(base) ((base) + (0x16304))
#define SOC_DMSS_GLB_TRACE_CTRL2_ADDR(base) ((base) + (0x16308))
#define SOC_DMSS_GLB_TRACE_REC0_ADDR(base) ((base) + (0x16310))
#define SOC_DMSS_GLB_TRACE_REC1_ADDR(base) ((base) + (0x16314))
#define SOC_DMSS_GLB_TRACE_FILTER0_ADDR(base) ((base) + (0x16318))
#define SOC_DMSS_GLB_TRACE_FILTER1_ADDR(base) ((base) + (0x1631C))
#define SOC_DMSS_GLB_TRACE_FILTER2_ADDR(base,mid_grps) ((base) + (0x16320+0x4*(mid_grps)))
#define SOC_DMSS_GLB_TRACE_FREQ_ADDR(base) ((base) + (0x16340))
#define SOC_DMSS_GLB_TRACE_INT_EN_ADDR(base) ((base) + (0x16350))
#define SOC_DMSS_GLB_TRACE_LOCK_DOWN_ADDR(base) ((base) + (0x16358))
#define SOC_DMSS_GLB_TRACE_STAT0_ADDR(base,dmis) ((base) + (0x16360+0x10*(dmis)))
#define SOC_DMSS_GLB_TRACE_STAT1_ADDR(base,dmis) ((base) + (0x16364+0x10*(dmis)))
#define SOC_DMSS_GLB_TRACE_STAT2_ADDR(base,dmis) ((base) + (0x16368+0x10*(dmis)))
#define SOC_DMSS_GLB_MPU_CFG_ADDR(base) ((base) + (0x16380))
#define SOC_DMSS_GLB_MPU_ERROR_ADDR_ADDR(base) ((base) + (0x16384))
#define SOC_DMSS_GLB_MPU_ACPU_CFG_ADDR(base) ((base) + (0x16388))
#define SOC_DMSS_GLB_SEC_BYPASS_ADDR(base) ((base) + (0x1638C))
#define SOC_DMSS_GLB_STAT_CTRL_ADDR(base) ((base) + (0x16400))
#define SOC_DMSS_GLB_STAT_CYCLE_ADDR(base) ((base) + (0x16410))
#define SOC_DMSS_GLB_STAT_PERFSTAT_ADDR(base) ((base) + (0x16420))
#define SOC_DMSS_GLB_STAT_DCQ_WR_ADDR(base,chans) ((base) + (0x16440+0x4*(chans)))
#define SOC_DMSS_GLB_STAT_DCQ_ADDR(base,chans) ((base) + (0x16450+0x4*(chans)))
#define SOC_DMSS_GLB_STAT_WR_ALL_ALLOW_ADDR(base,chans) ((base) + (0x16500+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_WR_PRI1_ALLOW_ADDR(base,chans) ((base) + (0x16504+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_WR_PRI2_ALLOW_ADDR(base,chans) ((base) + (0x16508+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_WR_PRI3_ALLOW_ADDR(base,chans) ((base) + (0x1650C+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_WR_PRI4_ALLOW_ADDR(base,chans) ((base) + (0x16510+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_WR_PRI5_ALLOW_ADDR(base,chans) ((base) + (0x16514+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_WR_PRI6_ALLOW_ADDR(base,chans) ((base) + (0x16518+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_WR_PRI7_ALLOW_ADDR(base,chans) ((base) + (0x1651C+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RD_ALL_ALLOW_ADDR(base,chans) ((base) + (0x16520+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RD_PRI1_ALLOW_ADDR(base,chans) ((base) + (0x16524+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RD_PRI2_ALLOW_ADDR(base,chans) ((base) + (0x16528+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RD_PRI3_ALLOW_ADDR(base,chans) ((base) + (0x1652C+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RD_PRI4_ALLOW_ADDR(base,chans) ((base) + (0x16530+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RD_PRI5_ALLOW_ADDR(base,chans) ((base) + (0x16534+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RD_PRI6_ALLOW_ADDR(base,chans) ((base) + (0x16538+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RD_PRI7_ALLOW_ADDR(base,chans) ((base) + (0x1653C+0x40*(chans)))
#define SOC_DMSS_GLB_STAT_RBUF_CTRL_ADDR(base) ((base) + (0x16600))
#define SOC_DMSS_GLB_STAT_RBUF_OCCUP0_ADDR(base) ((base) + (0x16604))
#define SOC_DMSS_GLB_STAT_RBUF_OCCUP1_ADDR(base) ((base) + (0x16608))
#define SOC_DMSS_GLB_STAT_RBUF_OCCUP2_ADDR(base) ((base) + (0x1660C))
#define SOC_DMSS_GLB_STAT_AMI_WFIFO_ADDR(base) ((base) + (0x16640))
#define SOC_DMSS_GLB_DFX_MODULE_ADDR(base) ((base) + (0x16700))
#define SOC_DMSS_GLB_DFX_FIFO_ADDR(base) ((base) + (0x16704))
#define SOC_DMSS_GLB_DFX_RBUF_FIFO_ADDR(base) ((base) + (0x16708))
#define SOC_DMSS_GLB_DFX_DCQ_ST_ADDR(base,chans) ((base) + (0x16710+0x4*(chans)))
#define SOC_DMSS_GLB_DFX_RBUF0_ADDR(base,rbuf_srcs) ((base) + (0x16720+0x10*(rbuf_srcs)))
#define SOC_DMSS_GLB_DFX_RBUF1_ADDR(base,rbuf_srcs) ((base) + (0x16724+0x10*(rbuf_srcs)))
#define SOC_DMSS_GLB_DFX_DUMMY0_ADDR(base) ((base) + (0x167E0))
#define SOC_DMSS_GLB_DFX_DUMMY1_ADDR(base) ((base) + (0x167E4))
#define SOC_DMSS_GLB_DFX_DUMMY2_ADDR(base) ((base) + (0x167E8))
#define SOC_DMSS_GLB_DFX_DUMMY3_ADDR(base) ((base) + (0x167EC))
#define SOC_DMSS_GLB_DFX_LOAD_ADDR(base) ((base) + (0x167F0))
#define SOC_DMSS_MPU_ADDR_ATTR_ADDR(base,mpu_regs) ((base) + (0x20000+0x4*(mpu_regs)))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtl_data_width : 1;
        unsigned int reserved_0 : 3;
        unsigned int rtl_clk_mode : 2;
        unsigned int reserved_1 : 10;
        unsigned int rtl_rdr_que_depth : 4;
        unsigned int rtl_rdr_buf_num : 2;
        unsigned int reserved_2 : 2;
        unsigned int rtl_rdr_en : 1;
        unsigned int reserved_3 : 7;
    } reg;
} SOC_DMSS_ASI_RTL_INF0_UNION;
#endif
#define SOC_DMSS_ASI_RTL_INF0_rtl_data_width_START (0)
#define SOC_DMSS_ASI_RTL_INF0_rtl_data_width_END (0)
#define SOC_DMSS_ASI_RTL_INF0_rtl_clk_mode_START (4)
#define SOC_DMSS_ASI_RTL_INF0_rtl_clk_mode_END (5)
#define SOC_DMSS_ASI_RTL_INF0_rtl_rdr_que_depth_START (16)
#define SOC_DMSS_ASI_RTL_INF0_rtl_rdr_que_depth_END (19)
#define SOC_DMSS_ASI_RTL_INF0_rtl_rdr_buf_num_START (20)
#define SOC_DMSS_ASI_RTL_INF0_rtl_rdr_buf_num_END (21)
#define SOC_DMSS_ASI_RTL_INF0_rtl_rdr_en_START (24)
#define SOC_DMSS_ASI_RTL_INF0_rtl_rdr_en_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtl_wcmd_fifo_depth : 5;
        unsigned int reserved_0 : 3;
        unsigned int rtl_wd_fifo_depth : 5;
        unsigned int reserved_1 : 3;
        unsigned int rtl_b_fifo_depth : 5;
        unsigned int reserved_2 : 3;
        unsigned int rtl_ra_fifo_depth : 5;
        unsigned int reserved_3 : 3;
    } reg;
} SOC_DMSS_ASI_RTL_INF1_UNION;
#endif
#define SOC_DMSS_ASI_RTL_INF1_rtl_wcmd_fifo_depth_START (0)
#define SOC_DMSS_ASI_RTL_INF1_rtl_wcmd_fifo_depth_END (4)
#define SOC_DMSS_ASI_RTL_INF1_rtl_wd_fifo_depth_START (8)
#define SOC_DMSS_ASI_RTL_INF1_rtl_wd_fifo_depth_END (12)
#define SOC_DMSS_ASI_RTL_INF1_rtl_b_fifo_depth_START (16)
#define SOC_DMSS_ASI_RTL_INF1_rtl_b_fifo_depth_END (20)
#define SOC_DMSS_ASI_RTL_INF1_rtl_ra_fifo_depth_START (24)
#define SOC_DMSS_ASI_RTL_INF1_rtl_ra_fifo_depth_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtl_sec_rgn_num : 6;
        unsigned int reserved_0 : 2;
        unsigned int rtl_sec_chk_mid_width : 3;
        unsigned int reserved_1 : 1;
        unsigned int rtl_mpu_chk_mid_width : 3;
        unsigned int reserved_2 : 1;
        unsigned int rtl_rd_fifo_depth : 5;
        unsigned int reserved_3 : 3;
        unsigned int rtl_subsec_rgn_num : 4;
        unsigned int reserved_4 : 4;
    } reg;
} SOC_DMSS_ASI_RTL_INF2_UNION;
#endif
#define SOC_DMSS_ASI_RTL_INF2_rtl_sec_rgn_num_START (0)
#define SOC_DMSS_ASI_RTL_INF2_rtl_sec_rgn_num_END (5)
#define SOC_DMSS_ASI_RTL_INF2_rtl_sec_chk_mid_width_START (8)
#define SOC_DMSS_ASI_RTL_INF2_rtl_sec_chk_mid_width_END (10)
#define SOC_DMSS_ASI_RTL_INF2_rtl_mpu_chk_mid_width_START (12)
#define SOC_DMSS_ASI_RTL_INF2_rtl_mpu_chk_mid_width_END (14)
#define SOC_DMSS_ASI_RTL_INF2_rtl_rd_fifo_depth_START (16)
#define SOC_DMSS_ASI_RTL_INF2_rtl_rd_fifo_depth_END (20)
#define SOC_DMSS_ASI_RTL_INF2_rtl_subsec_rgn_num_START (24)
#define SOC_DMSS_ASI_RTL_INF2_rtl_subsec_rgn_num_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_en_asi : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_ASI_STA_CKG_UNION;
#endif
#define SOC_DMSS_ASI_STA_CKG_gt_en_asi_START (0)
#define SOC_DMSS_ASI_STA_CKG_gt_en_asi_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ckg_byp_asi : 1;
        unsigned int ckg_byp_ra : 1;
        unsigned int ckg_byp_arcs : 1;
        unsigned int ckg_byp_awcs : 1;
        unsigned int ckg_byp_wd : 1;
        unsigned int ckg_byp_b : 1;
        unsigned int ckg_byp_rd : 1;
        unsigned int ckg_byp_fkrd : 1;
        unsigned int ckg_byp_asi_sec : 1;
        unsigned int ckg_byp_rdr_est : 1;
        unsigned int ckg_byp_rdr_buf : 1;
        unsigned int ckg_byp_async : 1;
        unsigned int ckg_byp_ca : 1;
        unsigned int ckg_byp_qos_bwc : 1;
        unsigned int ckg_byp_qos_lat : 1;
        unsigned int ckg_byp_stat : 1;
        unsigned int ckg_byp_wooo : 1;
        unsigned int ckg_byp_detect : 1;
        unsigned int reserved : 14;
    } reg;
} SOC_DMSS_ASI_DYN_CKG_UNION;
#endif
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_asi_START (0)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_asi_END (0)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_ra_START (1)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_ra_END (1)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_arcs_START (2)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_arcs_END (2)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_awcs_START (3)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_awcs_END (3)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_wd_START (4)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_wd_END (4)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_b_START (5)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_b_END (5)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_rd_START (6)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_rd_END (6)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_fkrd_START (7)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_fkrd_END (7)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_asi_sec_START (8)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_asi_sec_END (8)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_rdr_est_START (9)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_rdr_est_END (9)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_rdr_buf_START (10)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_rdr_buf_END (10)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_async_START (11)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_async_END (11)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_ca_START (12)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_ca_END (12)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_qos_bwc_START (13)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_qos_bwc_END (13)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_qos_lat_START (14)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_qos_lat_END (14)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_stat_START (15)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_stat_END (15)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_wooo_START (16)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_wooo_END (16)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_detect_START (17)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_detect_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cactive_wl : 7;
        unsigned int reserved : 25;
    } reg;
} SOC_DMSS_ASI_ASYNC_BRG_UNION;
#endif
#define SOC_DMSS_ASI_ASYNC_BRG_cactive_wl_START (0)
#define SOC_DMSS_ASI_ASYNC_BRG_cactive_wl_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_shift_mode : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_DMSS_ASI_ADDR_SHIFT_UNION;
#endif
#define SOC_DMSS_ASI_ADDR_SHIFT_addr_shift_mode_START (0)
#define SOC_DMSS_ASI_ADDR_SHIFT_addr_shift_mode_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ra_en : 1;
        unsigned int reserved_0 : 3;
        unsigned int ra_trsfr_wl : 4;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_DMSS_ASI_RATE_ADPT_UNION;
#endif
#define SOC_DMSS_ASI_RATE_ADPT_ra_en_START (0)
#define SOC_DMSS_ASI_RATE_ADPT_ra_en_END (0)
#define SOC_DMSS_ASI_RATE_ADPT_ra_trsfr_wl_START (4)
#define SOC_DMSS_ASI_RATE_ADPT_ra_trsfr_wl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int anti_deadlock : 1;
        unsigned int ex_anti_deadlock : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_DMSS_ASI_ANTI_DEADLOCK_UNION;
#endif
#define SOC_DMSS_ASI_ANTI_DEADLOCK_anti_deadlock_START (0)
#define SOC_DMSS_ASI_ANTI_DEADLOCK_anti_deadlock_END (0)
#define SOC_DMSS_ASI_ANTI_DEADLOCK_ex_anti_deadlock_START (1)
#define SOC_DMSS_ASI_ANTI_DEADLOCK_ex_anti_deadlock_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rdr_id_match : 15;
        unsigned int reserved : 1;
        unsigned int rdr_id_mask : 15;
        unsigned int rdr_id_mode : 1;
    } reg;
} SOC_DMSS_ASI_RDR_CTRL_UNION;
#endif
#define SOC_DMSS_ASI_RDR_CTRL_rdr_id_match_START (0)
#define SOC_DMSS_ASI_RDR_CTRL_rdr_id_match_END (14)
#define SOC_DMSS_ASI_RDR_CTRL_rdr_id_mask_START (16)
#define SOC_DMSS_ASI_RDR_CTRL_rdr_id_mask_END (30)
#define SOC_DMSS_ASI_RDR_CTRL_rdr_id_mode_START (31)
#define SOC_DMSS_ASI_RDR_CTRL_rdr_id_mode_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pri_push_en : 1;
        unsigned int push_qos_en : 1;
        unsigned int hurry_qos_en : 1;
        unsigned int arb_pri_use : 1;
        unsigned int ami1_rcmd_dis : 1;
        unsigned int ami1_wcmd_dis : 1;
        unsigned int brg_push_en : 1;
        unsigned int rdr_head_byp_en : 1;
        unsigned int asi_dis : 1;
        unsigned int wooo_head_byp_en : 1;
        unsigned int ami0_rcmd_dis : 1;
        unsigned int ami0_wcmd_dis : 1;
        unsigned int reserved_0 : 2;
        unsigned int wooo_rlast_arb : 1;
        unsigned int wooo_free_disable : 1;
        unsigned int dcq_pri_allow_en : 1;
        unsigned int mst_pri_allow_en : 1;
        unsigned int reserved_1 : 2;
        unsigned int adpt_zoom : 2;
        unsigned int sc_cmo_len : 1;
        unsigned int reserved_2 : 9;
    } reg;
} SOC_DMSS_ASI_QOS_CTRL_UNION;
#endif
#define SOC_DMSS_ASI_QOS_CTRL_pri_push_en_START (0)
#define SOC_DMSS_ASI_QOS_CTRL_pri_push_en_END (0)
#define SOC_DMSS_ASI_QOS_CTRL_push_qos_en_START (1)
#define SOC_DMSS_ASI_QOS_CTRL_push_qos_en_END (1)
#define SOC_DMSS_ASI_QOS_CTRL_hurry_qos_en_START (2)
#define SOC_DMSS_ASI_QOS_CTRL_hurry_qos_en_END (2)
#define SOC_DMSS_ASI_QOS_CTRL_arb_pri_use_START (3)
#define SOC_DMSS_ASI_QOS_CTRL_arb_pri_use_END (3)
#define SOC_DMSS_ASI_QOS_CTRL_ami1_rcmd_dis_START (4)
#define SOC_DMSS_ASI_QOS_CTRL_ami1_rcmd_dis_END (4)
#define SOC_DMSS_ASI_QOS_CTRL_ami1_wcmd_dis_START (5)
#define SOC_DMSS_ASI_QOS_CTRL_ami1_wcmd_dis_END (5)
#define SOC_DMSS_ASI_QOS_CTRL_brg_push_en_START (6)
#define SOC_DMSS_ASI_QOS_CTRL_brg_push_en_END (6)
#define SOC_DMSS_ASI_QOS_CTRL_rdr_head_byp_en_START (7)
#define SOC_DMSS_ASI_QOS_CTRL_rdr_head_byp_en_END (7)
#define SOC_DMSS_ASI_QOS_CTRL_asi_dis_START (8)
#define SOC_DMSS_ASI_QOS_CTRL_asi_dis_END (8)
#define SOC_DMSS_ASI_QOS_CTRL_wooo_head_byp_en_START (9)
#define SOC_DMSS_ASI_QOS_CTRL_wooo_head_byp_en_END (9)
#define SOC_DMSS_ASI_QOS_CTRL_ami0_rcmd_dis_START (10)
#define SOC_DMSS_ASI_QOS_CTRL_ami0_rcmd_dis_END (10)
#define SOC_DMSS_ASI_QOS_CTRL_ami0_wcmd_dis_START (11)
#define SOC_DMSS_ASI_QOS_CTRL_ami0_wcmd_dis_END (11)
#define SOC_DMSS_ASI_QOS_CTRL_wooo_rlast_arb_START (14)
#define SOC_DMSS_ASI_QOS_CTRL_wooo_rlast_arb_END (14)
#define SOC_DMSS_ASI_QOS_CTRL_wooo_free_disable_START (15)
#define SOC_DMSS_ASI_QOS_CTRL_wooo_free_disable_END (15)
#define SOC_DMSS_ASI_QOS_CTRL_dcq_pri_allow_en_START (16)
#define SOC_DMSS_ASI_QOS_CTRL_dcq_pri_allow_en_END (16)
#define SOC_DMSS_ASI_QOS_CTRL_mst_pri_allow_en_START (17)
#define SOC_DMSS_ASI_QOS_CTRL_mst_pri_allow_en_END (17)
#define SOC_DMSS_ASI_QOS_CTRL_adpt_zoom_START (20)
#define SOC_DMSS_ASI_QOS_CTRL_adpt_zoom_END (21)
#define SOC_DMSS_ASI_QOS_CTRL_sc_cmo_len_START (22)
#define SOC_DMSS_ASI_QOS_CTRL_sc_cmo_len_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_pri0 : 3;
        unsigned int reserved_0: 1;
        unsigned int wr_pri1 : 3;
        unsigned int reserved_1: 1;
        unsigned int wr_pri2 : 3;
        unsigned int reserved_2: 1;
        unsigned int wr_pri3 : 3;
        unsigned int reserved_3: 1;
        unsigned int wr_pri4 : 3;
        unsigned int reserved_4: 1;
        unsigned int wr_pri5 : 3;
        unsigned int reserved_5: 1;
        unsigned int wr_pri6 : 3;
        unsigned int reserved_6: 1;
        unsigned int wr_pri7 : 3;
        unsigned int reserved_7: 1;
    } reg;
} SOC_DMSS_ASI_QOS_WRPRI_UNION;
#endif
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri0_START (0)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri0_END (2)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri1_START (4)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri1_END (6)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri2_START (8)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri2_END (10)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri3_START (12)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri3_END (14)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri4_START (16)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri4_END (18)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri5_START (20)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri5_END (22)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri6_START (24)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri6_END (26)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri7_START (28)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri7_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_pri0 : 3;
        unsigned int reserved_0: 1;
        unsigned int rd_pri1 : 3;
        unsigned int reserved_1: 1;
        unsigned int rd_pri2 : 3;
        unsigned int reserved_2: 1;
        unsigned int rd_pri3 : 3;
        unsigned int reserved_3: 1;
        unsigned int rd_pri4 : 3;
        unsigned int reserved_4: 1;
        unsigned int rd_pri5 : 3;
        unsigned int reserved_5: 1;
        unsigned int rd_pri6 : 3;
        unsigned int reserved_6: 1;
        unsigned int rd_pri7 : 3;
        unsigned int reserved_7: 1;
    } reg;
} SOC_DMSS_ASI_QOS_RDPRI_UNION;
#endif
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri0_START (0)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri0_END (2)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri1_START (4)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri1_END (6)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri2_START (8)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri2_END (10)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri3_START (12)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri3_END (14)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri4_START (16)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri4_END (18)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri5_START (20)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri5_END (22)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri6_START (24)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri6_END (26)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri7_START (28)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri7_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_adpt_low : 4;
        unsigned int rd_adpt_lvl : 3;
        unsigned int reserved_0 : 1;
        unsigned int rd_adpt_high : 4;
        unsigned int reserved_1 : 4;
        unsigned int wr_adpt_low : 4;
        unsigned int wr_adpt_lvl : 3;
        unsigned int reserved_2 : 1;
        unsigned int wr_adpt_high : 4;
        unsigned int reserved_3 : 4;
    } reg;
} SOC_DMSS_ASI_QOS_ADPT_UNION;
#endif
#define SOC_DMSS_ASI_QOS_ADPT_rd_adpt_low_START (0)
#define SOC_DMSS_ASI_QOS_ADPT_rd_adpt_low_END (3)
#define SOC_DMSS_ASI_QOS_ADPT_rd_adpt_lvl_START (4)
#define SOC_DMSS_ASI_QOS_ADPT_rd_adpt_lvl_END (6)
#define SOC_DMSS_ASI_QOS_ADPT_rd_adpt_high_START (8)
#define SOC_DMSS_ASI_QOS_ADPT_rd_adpt_high_END (11)
#define SOC_DMSS_ASI_QOS_ADPT_wr_adpt_low_START (16)
#define SOC_DMSS_ASI_QOS_ADPT_wr_adpt_low_END (19)
#define SOC_DMSS_ASI_QOS_ADPT_wr_adpt_lvl_START (20)
#define SOC_DMSS_ASI_QOS_ADPT_wr_adpt_lvl_END (22)
#define SOC_DMSS_ASI_QOS_ADPT_wr_adpt_high_START (24)
#define SOC_DMSS_ASI_QOS_ADPT_wr_adpt_high_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lat_target : 12;
        unsigned int lat_factor : 3;
        unsigned int reserved_0 : 1;
        unsigned int lat_pri_min : 3;
        unsigned int reserved_1 : 1;
        unsigned int lat_pri_max : 3;
        unsigned int reserved_2 : 1;
        unsigned int lat_flux_req_dly : 4;
        unsigned int lat_flux_req_en : 1;
        unsigned int lat_quiesce : 1;
        unsigned int lat_mode : 1;
        unsigned int lat_en : 1;
    } reg;
} SOC_DMSS_ASI_QOS_LATENCY_UNION;
#endif
#define SOC_DMSS_ASI_QOS_LATENCY_lat_target_START (0)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_target_END (11)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_factor_START (12)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_factor_END (14)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_pri_min_START (16)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_pri_min_END (18)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_pri_max_START (20)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_pri_max_END (22)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_flux_req_dly_START (24)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_flux_req_dly_END (27)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_flux_req_en_START (28)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_flux_req_en_END (28)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_quiesce_START (29)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_quiesce_END (29)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_mode_START (30)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_mode_END (30)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_en_START (31)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rglr_saturation : 14;
        unsigned int reserved_0 : 2;
        unsigned int rglr_bandwidth : 13;
        unsigned int reserved_1 : 1;
        unsigned int rglr_type : 1;
        unsigned int rglr_en : 1;
    } reg;
} SOC_DMSS_ASI_QOS_RGLR0_UNION;
#endif
#define SOC_DMSS_ASI_QOS_RGLR0_rglr_saturation_START (0)
#define SOC_DMSS_ASI_QOS_RGLR0_rglr_saturation_END (13)
#define SOC_DMSS_ASI_QOS_RGLR0_rglr_bandwidth_START (16)
#define SOC_DMSS_ASI_QOS_RGLR0_rglr_bandwidth_END (28)
#define SOC_DMSS_ASI_QOS_RGLR0_rglr_type_START (30)
#define SOC_DMSS_ASI_QOS_RGLR0_rglr_type_END (30)
#define SOC_DMSS_ASI_QOS_RGLR0_rglr_en_START (31)
#define SOC_DMSS_ASI_QOS_RGLR0_rglr_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rglr_pri0 : 3;
        unsigned int reserved_0: 5;
        unsigned int rglr_pri1 : 3;
        unsigned int reserved_1: 21;
    } reg;
} SOC_DMSS_ASI_QOS_RGLR0_PRI_UNION;
#endif
#define SOC_DMSS_ASI_QOS_RGLR0_PRI_rglr_pri0_START (0)
#define SOC_DMSS_ASI_QOS_RGLR0_PRI_rglr_pri0_END (2)
#define SOC_DMSS_ASI_QOS_RGLR0_PRI_rglr_pri1_START (8)
#define SOC_DMSS_ASI_QOS_RGLR0_PRI_rglr_pri1_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rglr_saturation : 14;
        unsigned int reserved_0 : 2;
        unsigned int rglr_bandwidth : 13;
        unsigned int reserved_1 : 2;
        unsigned int rglr_en : 1;
    } reg;
} SOC_DMSS_ASI_QOS_RGLR1_UNION;
#endif
#define SOC_DMSS_ASI_QOS_RGLR1_rglr_saturation_START (0)
#define SOC_DMSS_ASI_QOS_RGLR1_rglr_saturation_END (13)
#define SOC_DMSS_ASI_QOS_RGLR1_rglr_bandwidth_START (16)
#define SOC_DMSS_ASI_QOS_RGLR1_rglr_bandwidth_END (28)
#define SOC_DMSS_ASI_QOS_RGLR1_rglr_en_START (31)
#define SOC_DMSS_ASI_QOS_RGLR1_rglr_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rglr_pri0 : 3;
        unsigned int reserved_0: 5;
        unsigned int rglr_pri1 : 3;
        unsigned int reserved_1: 21;
    } reg;
} SOC_DMSS_ASI_QOS_RGLR1_PRI_UNION;
#endif
#define SOC_DMSS_ASI_QOS_RGLR1_PRI_rglr_pri0_START (0)
#define SOC_DMSS_ASI_QOS_RGLR1_PRI_rglr_pri0_END (2)
#define SOC_DMSS_ASI_QOS_RGLR1_PRI_rglr_pri1_START (8)
#define SOC_DMSS_ASI_QOS_RGLR1_PRI_rglr_pri1_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lmtr_saturation : 14;
        unsigned int reserved_0 : 2;
        unsigned int lmtr_bandwidth : 13;
        unsigned int reserved_1 : 1;
        unsigned int lmtr_type : 1;
        unsigned int lmtr_en : 1;
    } reg;
} SOC_DMSS_ASI_QOS_LMTR0_UNION;
#endif
#define SOC_DMSS_ASI_QOS_LMTR0_lmtr_saturation_START (0)
#define SOC_DMSS_ASI_QOS_LMTR0_lmtr_saturation_END (13)
#define SOC_DMSS_ASI_QOS_LMTR0_lmtr_bandwidth_START (16)
#define SOC_DMSS_ASI_QOS_LMTR0_lmtr_bandwidth_END (28)
#define SOC_DMSS_ASI_QOS_LMTR0_lmtr_type_START (30)
#define SOC_DMSS_ASI_QOS_LMTR0_lmtr_type_END (30)
#define SOC_DMSS_ASI_QOS_LMTR0_lmtr_en_START (31)
#define SOC_DMSS_ASI_QOS_LMTR0_lmtr_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lmtr_saturation : 14;
        unsigned int reserved_0 : 2;
        unsigned int lmtr_bandwidth : 13;
        unsigned int reserved_1 : 2;
        unsigned int lmtr_en : 1;
    } reg;
} SOC_DMSS_ASI_QOS_LMTR1_UNION;
#endif
#define SOC_DMSS_ASI_QOS_LMTR1_lmtr_saturation_START (0)
#define SOC_DMSS_ASI_QOS_LMTR1_lmtr_saturation_END (13)
#define SOC_DMSS_ASI_QOS_LMTR1_lmtr_bandwidth_START (16)
#define SOC_DMSS_ASI_QOS_LMTR1_lmtr_bandwidth_END (28)
#define SOC_DMSS_ASI_QOS_LMTR1_lmtr_en_START (31)
#define SOC_DMSS_ASI_QOS_LMTR1_lmtr_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lmtr_saturation : 14;
        unsigned int reserved_0 : 2;
        unsigned int lmtr_bandwidth : 13;
        unsigned int reserved_1 : 2;
        unsigned int lmtr_en : 1;
    } reg;
} SOC_DMSS_ASI_QOS_LMTR2_UNION;
#endif
#define SOC_DMSS_ASI_QOS_LMTR2_lmtr_saturation_START (0)
#define SOC_DMSS_ASI_QOS_LMTR2_lmtr_saturation_END (13)
#define SOC_DMSS_ASI_QOS_LMTR2_lmtr_bandwidth_START (16)
#define SOC_DMSS_ASI_QOS_LMTR2_lmtr_bandwidth_END (28)
#define SOC_DMSS_ASI_QOS_LMTR2_lmtr_en_START (31)
#define SOC_DMSS_ASI_QOS_LMTR2_lmtr_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lmtr_saturation : 14;
        unsigned int reserved_0 : 2;
        unsigned int lmtr_bandwidth : 13;
        unsigned int reserved_1 : 2;
        unsigned int lmtr_en : 1;
    } reg;
} SOC_DMSS_ASI_QOS_LMTR3_UNION;
#endif
#define SOC_DMSS_ASI_QOS_LMTR3_lmtr_saturation_START (0)
#define SOC_DMSS_ASI_QOS_LMTR3_lmtr_saturation_END (13)
#define SOC_DMSS_ASI_QOS_LMTR3_lmtr_bandwidth_START (16)
#define SOC_DMSS_ASI_QOS_LMTR3_lmtr_bandwidth_END (28)
#define SOC_DMSS_ASI_QOS_LMTR3_lmtr_en_START (31)
#define SOC_DMSS_ASI_QOS_LMTR3_lmtr_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lmtr_saturation : 14;
        unsigned int reserved_0 : 2;
        unsigned int lmtr_bandwidth : 13;
        unsigned int reserved_1 : 2;
        unsigned int lmtr_en : 1;
    } reg;
} SOC_DMSS_ASI_QOS_LMTR4_UNION;
#endif
#define SOC_DMSS_ASI_QOS_LMTR4_lmtr_saturation_START (0)
#define SOC_DMSS_ASI_QOS_LMTR4_lmtr_saturation_END (13)
#define SOC_DMSS_ASI_QOS_LMTR4_lmtr_bandwidth_START (16)
#define SOC_DMSS_ASI_QOS_LMTR4_lmtr_bandwidth_END (28)
#define SOC_DMSS_ASI_QOS_LMTR4_lmtr_en_START (31)
#define SOC_DMSS_ASI_QOS_LMTR4_lmtr_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_ostd_lvl : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_DMSS_ASI_QOS_RDOSTD_UNION;
#endif
#define SOC_DMSS_ASI_QOS_RDOSTD_rd_ostd_lvl_START (0)
#define SOC_DMSS_ASI_QOS_RDOSTD_rd_ostd_lvl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_mid_sel : 32;
    } reg;
} SOC_DMSS_ASI_PUSH_WRMID_UNION;
#endif
#define SOC_DMSS_ASI_PUSH_WRMID_wr_mid_sel_START (0)
#define SOC_DMSS_ASI_PUSH_WRMID_wr_mid_sel_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_mid_sel : 32;
    } reg;
} SOC_DMSS_ASI_PUSH_RDMID_UNION;
#endif
#define SOC_DMSS_ASI_PUSH_RDMID_rd_mid_sel_START (0)
#define SOC_DMSS_ASI_PUSH_RDMID_rd_mid_sel_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_mask_flux_rd : 1;
        unsigned int stat_mask_flux_wr : 1;
        unsigned int stat_mask_block_rd : 1;
        unsigned int stat_mask_block_wr : 1;
        unsigned int stat_mask_lmtr0 : 1;
        unsigned int stat_mask_lmtr1 : 1;
        unsigned int stat_mask_lmtr2 : 1;
        unsigned int stat_mask_lmtr3 : 1;
        unsigned int stat_mask_rdlat : 1;
        unsigned int stat_mask_pri : 1;
        unsigned int stat_mask_rdnum : 1;
        unsigned int stat_mask_dmi : 1;
        unsigned int stat_mask_lmtr4 : 1;
        unsigned int reserved : 19;
    } reg;
} SOC_DMSS_ASI_STAT_MASK_UNION;
#endif
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_flux_rd_START (0)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_flux_rd_END (0)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_flux_wr_START (1)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_flux_wr_END (1)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_block_rd_START (2)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_block_rd_END (2)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_block_wr_START (3)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_block_wr_END (3)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_lmtr0_START (4)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_lmtr0_END (4)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_lmtr1_START (5)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_lmtr1_END (5)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_lmtr2_START (6)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_lmtr2_END (6)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_lmtr3_START (7)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_lmtr3_END (7)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_rdlat_START (8)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_rdlat_END (8)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_pri_START (9)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_pri_END (9)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_rdnum_START (10)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_rdnum_END (10)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_dmi_START (11)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_dmi_END (11)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_lmtr4_START (12)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_lmtr4_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_flux_id_match : 20;
        unsigned int stat_flux_mid_match : 12;
    } reg;
} SOC_DMSS_ASI_STAT_FLUX_FILTER0_UNION;
#endif
#define SOC_DMSS_ASI_STAT_FLUX_FILTER0_stat_flux_id_match_START (0)
#define SOC_DMSS_ASI_STAT_FLUX_FILTER0_stat_flux_id_match_END (19)
#define SOC_DMSS_ASI_STAT_FLUX_FILTER0_stat_flux_mid_match_START (20)
#define SOC_DMSS_ASI_STAT_FLUX_FILTER0_stat_flux_mid_match_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_flux_id_mask : 20;
        unsigned int stat_flux_mid_mask : 12;
    } reg;
} SOC_DMSS_ASI_STAT_FLUX_FILTER1_UNION;
#endif
#define SOC_DMSS_ASI_STAT_FLUX_FILTER1_stat_flux_id_mask_START (0)
#define SOC_DMSS_ASI_STAT_FLUX_FILTER1_stat_flux_id_mask_END (19)
#define SOC_DMSS_ASI_STAT_FLUX_FILTER1_stat_flux_mid_mask_START (20)
#define SOC_DMSS_ASI_STAT_FLUX_FILTER1_stat_flux_mid_mask_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_flux_wr : 32;
    } reg;
} SOC_DMSS_ASI_STAT_FLUX_WR_UNION;
#endif
#define SOC_DMSS_ASI_STAT_FLUX_WR_stat_flux_wr_START (0)
#define SOC_DMSS_ASI_STAT_FLUX_WR_stat_flux_wr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_flux_rd : 32;
    } reg;
} SOC_DMSS_ASI_STAT_FLUX_RD_UNION;
#endif
#define SOC_DMSS_ASI_STAT_FLUX_RD_stat_flux_rd_START (0)
#define SOC_DMSS_ASI_STAT_FLUX_RD_stat_flux_rd_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_lmtr0 : 32;
    } reg;
} SOC_DMSS_ASI_STAT_LMTR0_UNION;
#endif
#define SOC_DMSS_ASI_STAT_LMTR0_stat_lmtr0_START (0)
#define SOC_DMSS_ASI_STAT_LMTR0_stat_lmtr0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_lmtr1 : 32;
    } reg;
} SOC_DMSS_ASI_STAT_LMTR1_UNION;
#endif
#define SOC_DMSS_ASI_STAT_LMTR1_stat_lmtr1_START (0)
#define SOC_DMSS_ASI_STAT_LMTR1_stat_lmtr1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_lmtr2 : 32;
    } reg;
} SOC_DMSS_ASI_STAT_LMTR2_UNION;
#endif
#define SOC_DMSS_ASI_STAT_LMTR2_stat_lmtr2_START (0)
#define SOC_DMSS_ASI_STAT_LMTR2_stat_lmtr2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_lmtr3 : 32;
    } reg;
} SOC_DMSS_ASI_STAT_LMTR3_UNION;
#endif
#define SOC_DMSS_ASI_STAT_LMTR3_stat_lmtr3_START (0)
#define SOC_DMSS_ASI_STAT_LMTR3_stat_lmtr3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_rdlat : 32;
    } reg;
} SOC_DMSS_ASI_STAT_RDLAT_UNION;
#endif
#define SOC_DMSS_ASI_STAT_RDLAT_stat_rdlat_START (0)
#define SOC_DMSS_ASI_STAT_RDLAT_stat_rdlat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_rdnum : 32;
    } reg;
} SOC_DMSS_ASI_STAT_RDNUM_UNION;
#endif
#define SOC_DMSS_ASI_STAT_RDNUM_stat_rdnum_START (0)
#define SOC_DMSS_ASI_STAT_RDNUM_stat_rdnum_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_block_wr : 32;
    } reg;
} SOC_DMSS_ASI_STAT_BLOCK_WR_UNION;
#endif
#define SOC_DMSS_ASI_STAT_BLOCK_WR_stat_block_wr_START (0)
#define SOC_DMSS_ASI_STAT_BLOCK_WR_stat_block_wr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_block_rd : 32;
    } reg;
} SOC_DMSS_ASI_STAT_BLOCK_RD_UNION;
#endif
#define SOC_DMSS_ASI_STAT_BLOCK_RD_stat_block_rd_START (0)
#define SOC_DMSS_ASI_STAT_BLOCK_RD_stat_block_rd_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_pri0 : 16;
        unsigned int stat_pri1 : 16;
    } reg;
} SOC_DMSS_ASI_STAT_PRI01_UNION;
#endif
#define SOC_DMSS_ASI_STAT_PRI01_stat_pri0_START (0)
#define SOC_DMSS_ASI_STAT_PRI01_stat_pri0_END (15)
#define SOC_DMSS_ASI_STAT_PRI01_stat_pri1_START (16)
#define SOC_DMSS_ASI_STAT_PRI01_stat_pri1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_pri2 : 16;
        unsigned int stat_pri3 : 16;
    } reg;
} SOC_DMSS_ASI_STAT_PRI23_UNION;
#endif
#define SOC_DMSS_ASI_STAT_PRI23_stat_pri2_START (0)
#define SOC_DMSS_ASI_STAT_PRI23_stat_pri2_END (15)
#define SOC_DMSS_ASI_STAT_PRI23_stat_pri3_START (16)
#define SOC_DMSS_ASI_STAT_PRI23_stat_pri3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_pri4 : 16;
        unsigned int stat_pri5 : 16;
    } reg;
} SOC_DMSS_ASI_STAT_PRI45_UNION;
#endif
#define SOC_DMSS_ASI_STAT_PRI45_stat_pri4_START (0)
#define SOC_DMSS_ASI_STAT_PRI45_stat_pri4_END (15)
#define SOC_DMSS_ASI_STAT_PRI45_stat_pri5_START (16)
#define SOC_DMSS_ASI_STAT_PRI45_stat_pri5_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_pri6 : 16;
        unsigned int stat_pri7 : 16;
    } reg;
} SOC_DMSS_ASI_STAT_PRI67_UNION;
#endif
#define SOC_DMSS_ASI_STAT_PRI67_stat_pri6_START (0)
#define SOC_DMSS_ASI_STAT_PRI67_stat_pri6_END (15)
#define SOC_DMSS_ASI_STAT_PRI67_stat_pri7_START (16)
#define SOC_DMSS_ASI_STAT_PRI67_stat_pri7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_lmtr4 : 32;
    } reg;
} SOC_DMSS_ASI_STAT_LMTR4_UNION;
#endif
#define SOC_DMSS_ASI_STAT_LMTR4_stat_lmtr4_START (0)
#define SOC_DMSS_ASI_STAT_LMTR4_stat_lmtr4_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int detect_timeout : 8;
        unsigned int reserved_0 : 20;
        unsigned int detect_int_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int detect_mode : 1;
        unsigned int detect_en : 1;
    } reg;
} SOC_DMSS_ASI_DETECT_CFG0_UNION;
#endif
#define SOC_DMSS_ASI_DETECT_CFG0_detect_timeout_START (0)
#define SOC_DMSS_ASI_DETECT_CFG0_detect_timeout_END (7)
#define SOC_DMSS_ASI_DETECT_CFG0_detect_int_en_START (28)
#define SOC_DMSS_ASI_DETECT_CFG0_detect_int_en_END (28)
#define SOC_DMSS_ASI_DETECT_CFG0_detect_mode_START (30)
#define SOC_DMSS_ASI_DETECT_CFG0_detect_mode_END (30)
#define SOC_DMSS_ASI_DETECT_CFG0_detect_en_START (31)
#define SOC_DMSS_ASI_DETECT_CFG0_detect_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int detect_cnt0_wl : 12;
        unsigned int detect_cnt1_wl : 12;
        unsigned int detect_cnt2_wl : 8;
    } reg;
} SOC_DMSS_ASI_DETECT_CFG1_UNION;
#endif
#define SOC_DMSS_ASI_DETECT_CFG1_detect_cnt0_wl_START (0)
#define SOC_DMSS_ASI_DETECT_CFG1_detect_cnt0_wl_END (11)
#define SOC_DMSS_ASI_DETECT_CFG1_detect_cnt1_wl_START (12)
#define SOC_DMSS_ASI_DETECT_CFG1_detect_cnt1_wl_END (23)
#define SOC_DMSS_ASI_DETECT_CFG1_detect_cnt2_wl_START (24)
#define SOC_DMSS_ASI_DETECT_CFG1_detect_cnt2_wl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int detect_cnt1 : 12;
        unsigned int reserved_0 : 4;
        unsigned int detect_time : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_DMSS_ASI_DETECT_INF0_UNION;
#endif
#define SOC_DMSS_ASI_DETECT_INF0_detect_cnt1_START (0)
#define SOC_DMSS_ASI_DETECT_INF0_detect_cnt1_END (11)
#define SOC_DMSS_ASI_DETECT_INF0_detect_time_START (16)
#define SOC_DMSS_ASI_DETECT_INF0_detect_time_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wakeup_hold : 2;
        unsigned int reserved : 28;
        unsigned int detect_wakeup : 1;
        unsigned int wakeup_en : 1;
    } reg;
} SOC_DMSS_ASI_WAKEUP_CFG_UNION;
#endif
#define SOC_DMSS_ASI_WAKEUP_CFG_wakeup_hold_START (0)
#define SOC_DMSS_ASI_WAKEUP_CFG_wakeup_hold_END (1)
#define SOC_DMSS_ASI_WAKEUP_CFG_detect_wakeup_START (30)
#define SOC_DMSS_ASI_WAKEUP_CFG_detect_wakeup_END (30)
#define SOC_DMSS_ASI_WAKEUP_CFG_wakeup_en_START (31)
#define SOC_DMSS_ASI_WAKEUP_CFG_wakeup_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub_rgn_disable0 : 32;
    } reg;
} SOC_DMSS_ASI_SEC_SUB_RGN0_UNION;
#endif
#define SOC_DMSS_ASI_SEC_SUB_RGN0_sub_rgn_disable0_START (0)
#define SOC_DMSS_ASI_SEC_SUB_RGN0_sub_rgn_disable0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub_rgn_disable1 : 32;
    } reg;
} SOC_DMSS_ASI_SEC_SUB_RGN1_UNION;
#endif
#define SOC_DMSS_ASI_SEC_SUB_RGN1_sub_rgn_disable1_START (0)
#define SOC_DMSS_ASI_SEC_SUB_RGN1_sub_rgn_disable1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub_rgn_disable2 : 32;
    } reg;
} SOC_DMSS_ASI_SEC_SUB_RGN2_UNION;
#endif
#define SOC_DMSS_ASI_SEC_SUB_RGN2_sub_rgn_disable2_START (0)
#define SOC_DMSS_ASI_SEC_SUB_RGN2_sub_rgn_disable2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub_rgn_disable3 : 32;
    } reg;
} SOC_DMSS_ASI_SEC_SUB_RGN3_UNION;
#endif
#define SOC_DMSS_ASI_SEC_SUB_RGN3_sub_rgn_disable3_START (0)
#define SOC_DMSS_ASI_SEC_SUB_RGN3_sub_rgn_disable3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ld_rgns_sel : 32;
    } reg;
} SOC_DMSS_ASI_SEC_LD_SEL_UNION;
#endif
#define SOC_DMSS_ASI_SEC_LD_SEL_ld_rgns_sel_START (0)
#define SOC_DMSS_ASI_SEC_LD_SEL_ld_rgns_sel_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_ASI_SEC_INT_EN_UNION;
#endif
#define SOC_DMSS_ASI_SEC_INT_EN_int_en_START (0)
#define SOC_DMSS_ASI_SEC_INT_EN_int_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_status : 1;
        unsigned int int_overrun : 1;
        unsigned int reserved_0 : 2;
        unsigned int int_cnt : 4;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_DMSS_ASI_SEC_INT_STATUS_UNION;
#endif
#define SOC_DMSS_ASI_SEC_INT_STATUS_int_status_START (0)
#define SOC_DMSS_ASI_SEC_INT_STATUS_int_status_END (0)
#define SOC_DMSS_ASI_SEC_INT_STATUS_int_overrun_START (1)
#define SOC_DMSS_ASI_SEC_INT_STATUS_int_overrun_END (1)
#define SOC_DMSS_ASI_SEC_INT_STATUS_int_cnt_START (4)
#define SOC_DMSS_ASI_SEC_INT_STATUS_int_cnt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int address_low : 32;
    } reg;
} SOC_DMSS_ASI_SEC_FAIL_CMD_INF0_UNION;
#endif
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF0_address_low_START (0)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF0_address_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int address_high : 8;
        unsigned int reserved : 8;
        unsigned int privileged : 1;
        unsigned int nonsecure : 1;
        unsigned int access_burst : 2;
        unsigned int access_type : 1;
        unsigned int access_size : 3;
        unsigned int access_len : 4;
        unsigned int access_cache : 4;
    } reg;
} SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_UNION;
#endif
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_address_high_START (0)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_address_high_END (7)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_privileged_START (16)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_privileged_END (16)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_nonsecure_START (17)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_nonsecure_END (17)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_access_burst_START (18)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_access_burst_END (19)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_access_type_START (20)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_access_type_END (20)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_access_size_START (21)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_access_size_END (23)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_access_len_START (24)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_access_len_END (27)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_access_cache_START (28)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_access_cache_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int id : 24;
        unsigned int mid : 8;
    } reg;
} SOC_DMSS_ASI_SEC_FAIL_CMD_INF2_UNION;
#endif
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF2_id_START (0)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF2_id_END (23)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF2_mid_START (24)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF2_mid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_ASI_AMI_PROT_INT_EN_UNION;
#endif
#define SOC_DMSS_ASI_AMI_PROT_INT_EN_int_en_START (0)
#define SOC_DMSS_ASI_AMI_PROT_INT_EN_int_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_status : 1;
        unsigned int int_overrun : 1;
        unsigned int reserved_0 : 2;
        unsigned int int_cnt : 4;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_DMSS_ASI_AMI_PROT_INT_STATUS_UNION;
#endif
#define SOC_DMSS_ASI_AMI_PROT_INT_STATUS_int_status_START (0)
#define SOC_DMSS_ASI_AMI_PROT_INT_STATUS_int_status_END (0)
#define SOC_DMSS_ASI_AMI_PROT_INT_STATUS_int_overrun_START (1)
#define SOC_DMSS_ASI_AMI_PROT_INT_STATUS_int_overrun_END (1)
#define SOC_DMSS_ASI_AMI_PROT_INT_STATUS_int_cnt_START (4)
#define SOC_DMSS_ASI_AMI_PROT_INT_STATUS_int_cnt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ami_prot_low_address : 32;
    } reg;
} SOC_DMSS_ASI_AMI_PROT_CMD_INF0_UNION;
#endif
#define SOC_DMSS_ASI_AMI_PROT_CMD_INF0_ami_prot_low_address_START (0)
#define SOC_DMSS_ASI_AMI_PROT_CMD_INF0_ami_prot_low_address_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ami_prot_high_address : 8;
        unsigned int reserved_0 : 8;
        unsigned int ami_prot_mid : 8;
        unsigned int ami_prot_cache : 4;
        unsigned int ami_prot_type : 1;
        unsigned int reserved_1 : 3;
    } reg;
} SOC_DMSS_ASI_AMI_PROT_CMD_INF1_UNION;
#endif
#define SOC_DMSS_ASI_AMI_PROT_CMD_INF1_ami_prot_high_address_START (0)
#define SOC_DMSS_ASI_AMI_PROT_CMD_INF1_ami_prot_high_address_END (7)
#define SOC_DMSS_ASI_AMI_PROT_CMD_INF1_ami_prot_mid_START (16)
#define SOC_DMSS_ASI_AMI_PROT_CMD_INF1_ami_prot_mid_END (23)
#define SOC_DMSS_ASI_AMI_PROT_CMD_INF1_ami_prot_cache_START (24)
#define SOC_DMSS_ASI_AMI_PROT_CMD_INF1_ami_prot_cache_END (27)
#define SOC_DMSS_ASI_AMI_PROT_CMD_INF1_ami_prot_type_START (28)
#define SOC_DMSS_ASI_AMI_PROT_CMD_INF1_ami_prot_type_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_ASI_AMI_ENHN_INT_EN_UNION;
#endif
#define SOC_DMSS_ASI_AMI_ENHN_INT_EN_int_en_START (0)
#define SOC_DMSS_ASI_AMI_ENHN_INT_EN_int_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_status : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_ASI_AMI_ENHN_INT_STATUS_UNION;
#endif
#define SOC_DMSS_ASI_AMI_ENHN_INT_STATUS_int_status_START (0)
#define SOC_DMSS_ASI_AMI_ENHN_INT_STATUS_int_status_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ami_enhn_low_address : 32;
    } reg;
} SOC_DMSS_ASI_AMI_ENHN_CMD_INF0_UNION;
#endif
#define SOC_DMSS_ASI_AMI_ENHN_CMD_INF0_ami_enhn_low_address_START (0)
#define SOC_DMSS_ASI_AMI_ENHN_CMD_INF0_ami_enhn_low_address_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ami_enhn_high_address : 8;
        unsigned int reserved_0 : 8;
        unsigned int ami_enhn_mid : 8;
        unsigned int ami_enhn_cache : 4;
        unsigned int ami_enhn_type : 1;
        unsigned int reserved_1 : 3;
    } reg;
} SOC_DMSS_ASI_AMI_ENHN_CMD_INF1_UNION;
#endif
#define SOC_DMSS_ASI_AMI_ENHN_CMD_INF1_ami_enhn_high_address_START (0)
#define SOC_DMSS_ASI_AMI_ENHN_CMD_INF1_ami_enhn_high_address_END (7)
#define SOC_DMSS_ASI_AMI_ENHN_CMD_INF1_ami_enhn_mid_START (16)
#define SOC_DMSS_ASI_AMI_ENHN_CMD_INF1_ami_enhn_mid_END (23)
#define SOC_DMSS_ASI_AMI_ENHN_CMD_INF1_ami_enhn_cache_START (24)
#define SOC_DMSS_ASI_AMI_ENHN_CMD_INF1_ami_enhn_cache_END (27)
#define SOC_DMSS_ASI_AMI_ENHN_CMD_INF1_ami_enhn_type_START (28)
#define SOC_DMSS_ASI_AMI_ENHN_CMD_INF1_ami_enhn_type_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_trust_wr_ntrust0 : 32;
    } reg;
} SOC_DMSS_ASI_MPU_TRUST_WR_NTRUST0_UNION;
#endif
#define SOC_DMSS_ASI_MPU_TRUST_WR_NTRUST0_mid_trust_wr_ntrust0_START (0)
#define SOC_DMSS_ASI_MPU_TRUST_WR_NTRUST0_mid_trust_wr_ntrust0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_trust_wr_ntrust1 : 32;
    } reg;
} SOC_DMSS_ASI_MPU_TRUST_WR_NTRUST1_UNION;
#endif
#define SOC_DMSS_ASI_MPU_TRUST_WR_NTRUST1_mid_trust_wr_ntrust1_START (0)
#define SOC_DMSS_ASI_MPU_TRUST_WR_NTRUST1_mid_trust_wr_ntrust1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_trust_wr_prot0 : 32;
    } reg;
} SOC_DMSS_ASI_MPU_TRUST_WR_PROT0_UNION;
#endif
#define SOC_DMSS_ASI_MPU_TRUST_WR_PROT0_mid_trust_wr_prot0_START (0)
#define SOC_DMSS_ASI_MPU_TRUST_WR_PROT0_mid_trust_wr_prot0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_trust_wr_prot1 : 32;
    } reg;
} SOC_DMSS_ASI_MPU_TRUST_WR_PROT1_UNION;
#endif
#define SOC_DMSS_ASI_MPU_TRUST_WR_PROT1_mid_trust_wr_prot1_START (0)
#define SOC_DMSS_ASI_MPU_TRUST_WR_PROT1_mid_trust_wr_prot1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_prot_wr_ntrust0 : 32;
    } reg;
} SOC_DMSS_ASI_MPU_PROT_WR_NTRUST0_UNION;
#endif
#define SOC_DMSS_ASI_MPU_PROT_WR_NTRUST0_mid_prot_wr_ntrust0_START (0)
#define SOC_DMSS_ASI_MPU_PROT_WR_NTRUST0_mid_prot_wr_ntrust0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_prot_wr_ntrust1 : 32;
    } reg;
} SOC_DMSS_ASI_MPU_PROT_WR_NTRUST1_UNION;
#endif
#define SOC_DMSS_ASI_MPU_PROT_WR_NTRUST1_mid_prot_wr_ntrust1_START (0)
#define SOC_DMSS_ASI_MPU_PROT_WR_NTRUST1_mid_prot_wr_ntrust1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rgn_base_addr : 20;
        unsigned int reserved : 10;
        unsigned int sub_rgn_zone : 1;
        unsigned int rgn_en : 1;
    } reg;
} SOC_DMSS_ASI_SEC_RGN_MAP0_UNION;
#endif
#define SOC_DMSS_ASI_SEC_RGN_MAP0_rgn_base_addr_START (0)
#define SOC_DMSS_ASI_SEC_RGN_MAP0_rgn_base_addr_END (19)
#define SOC_DMSS_ASI_SEC_RGN_MAP0_sub_rgn_zone_START (30)
#define SOC_DMSS_ASI_SEC_RGN_MAP0_sub_rgn_zone_END (30)
#define SOC_DMSS_ASI_SEC_RGN_MAP0_rgn_en_START (31)
#define SOC_DMSS_ASI_SEC_RGN_MAP0_rgn_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rgn_top_addr : 20;
        unsigned int reserved : 8;
        unsigned int sp : 4;
    } reg;
} SOC_DMSS_ASI_SEC_RGN_MAP1_UNION;
#endif
#define SOC_DMSS_ASI_SEC_RGN_MAP1_rgn_top_addr_START (0)
#define SOC_DMSS_ASI_SEC_RGN_MAP1_rgn_top_addr_END (19)
#define SOC_DMSS_ASI_SEC_RGN_MAP1_sp_START (28)
#define SOC_DMSS_ASI_SEC_RGN_MAP1_sp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_wr : 32;
    } reg;
} SOC_DMSS_ASI_SEC_MID_WR_UNION;
#endif
#define SOC_DMSS_ASI_SEC_MID_WR_mid_sel_wr_START (0)
#define SOC_DMSS_ASI_SEC_MID_WR_mid_sel_wr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_rd : 32;
    } reg;
} SOC_DMSS_ASI_SEC_MID_RD_UNION;
#endif
#define SOC_DMSS_ASI_SEC_MID_RD_mid_sel_rd_START (0)
#define SOC_DMSS_ASI_SEC_MID_RD_mid_sel_rd_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int asi_busy : 1;
        unsigned int asi_temp_busy : 1;
        unsigned int reserved_0 : 2;
        unsigned int ra_busy : 1;
        unsigned int aw_busy : 1;
        unsigned int ar_busy : 1;
        unsigned int awcs_busy : 1;
        unsigned int arcs_busy : 1;
        unsigned int wd_busy : 1;
        unsigned int b_busy : 1;
        unsigned int rd_busy : 1;
        unsigned int fkrd_busy : 1;
        unsigned int awsec_busy : 1;
        unsigned int arsec_busy : 1;
        unsigned int rdr_est_busy : 1;
        unsigned int rdr_buf_busy : 1;
        unsigned int ca_busy : 1;
        unsigned int wooo_busy : 1;
        unsigned int reserved_1 : 1;
        unsigned int w_waiting : 1;
        unsigned int reserved_2 : 3;
        unsigned int aw_bp : 1;
        unsigned int w_bp : 1;
        unsigned int b_bp : 1;
        unsigned int ar_bp : 1;
        unsigned int r_bp : 1;
        unsigned int reserved_3 : 3;
    } reg;
} SOC_DMSS_ASI_DFX_MODULE_UNION;
#endif
#define SOC_DMSS_ASI_DFX_MODULE_asi_busy_START (0)
#define SOC_DMSS_ASI_DFX_MODULE_asi_busy_END (0)
#define SOC_DMSS_ASI_DFX_MODULE_asi_temp_busy_START (1)
#define SOC_DMSS_ASI_DFX_MODULE_asi_temp_busy_END (1)
#define SOC_DMSS_ASI_DFX_MODULE_ra_busy_START (4)
#define SOC_DMSS_ASI_DFX_MODULE_ra_busy_END (4)
#define SOC_DMSS_ASI_DFX_MODULE_aw_busy_START (5)
#define SOC_DMSS_ASI_DFX_MODULE_aw_busy_END (5)
#define SOC_DMSS_ASI_DFX_MODULE_ar_busy_START (6)
#define SOC_DMSS_ASI_DFX_MODULE_ar_busy_END (6)
#define SOC_DMSS_ASI_DFX_MODULE_awcs_busy_START (7)
#define SOC_DMSS_ASI_DFX_MODULE_awcs_busy_END (7)
#define SOC_DMSS_ASI_DFX_MODULE_arcs_busy_START (8)
#define SOC_DMSS_ASI_DFX_MODULE_arcs_busy_END (8)
#define SOC_DMSS_ASI_DFX_MODULE_wd_busy_START (9)
#define SOC_DMSS_ASI_DFX_MODULE_wd_busy_END (9)
#define SOC_DMSS_ASI_DFX_MODULE_b_busy_START (10)
#define SOC_DMSS_ASI_DFX_MODULE_b_busy_END (10)
#define SOC_DMSS_ASI_DFX_MODULE_rd_busy_START (11)
#define SOC_DMSS_ASI_DFX_MODULE_rd_busy_END (11)
#define SOC_DMSS_ASI_DFX_MODULE_fkrd_busy_START (12)
#define SOC_DMSS_ASI_DFX_MODULE_fkrd_busy_END (12)
#define SOC_DMSS_ASI_DFX_MODULE_awsec_busy_START (13)
#define SOC_DMSS_ASI_DFX_MODULE_awsec_busy_END (13)
#define SOC_DMSS_ASI_DFX_MODULE_arsec_busy_START (14)
#define SOC_DMSS_ASI_DFX_MODULE_arsec_busy_END (14)
#define SOC_DMSS_ASI_DFX_MODULE_rdr_est_busy_START (15)
#define SOC_DMSS_ASI_DFX_MODULE_rdr_est_busy_END (15)
#define SOC_DMSS_ASI_DFX_MODULE_rdr_buf_busy_START (16)
#define SOC_DMSS_ASI_DFX_MODULE_rdr_buf_busy_END (16)
#define SOC_DMSS_ASI_DFX_MODULE_ca_busy_START (17)
#define SOC_DMSS_ASI_DFX_MODULE_ca_busy_END (17)
#define SOC_DMSS_ASI_DFX_MODULE_wooo_busy_START (18)
#define SOC_DMSS_ASI_DFX_MODULE_wooo_busy_END (18)
#define SOC_DMSS_ASI_DFX_MODULE_w_waiting_START (20)
#define SOC_DMSS_ASI_DFX_MODULE_w_waiting_END (20)
#define SOC_DMSS_ASI_DFX_MODULE_aw_bp_START (24)
#define SOC_DMSS_ASI_DFX_MODULE_aw_bp_END (24)
#define SOC_DMSS_ASI_DFX_MODULE_w_bp_START (25)
#define SOC_DMSS_ASI_DFX_MODULE_w_bp_END (25)
#define SOC_DMSS_ASI_DFX_MODULE_b_bp_START (26)
#define SOC_DMSS_ASI_DFX_MODULE_b_bp_END (26)
#define SOC_DMSS_ASI_DFX_MODULE_ar_bp_START (27)
#define SOC_DMSS_ASI_DFX_MODULE_ar_bp_END (27)
#define SOC_DMSS_ASI_DFX_MODULE_r_bp_START (28)
#define SOC_DMSS_ASI_DFX_MODULE_r_bp_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ra_wfifo_st : 2;
        unsigned int wd_cmdfifo_st : 2;
        unsigned int b_respfifo_st : 2;
        unsigned int rd_rfifo_st : 2;
        unsigned int b_mififo_st : 2;
        unsigned int fkrd_fkfifo_st : 2;
        unsigned int reserved_0 : 6;
        unsigned int ami_wfifo_st : 2;
        unsigned int dmi0_wfifo_st : 2;
        unsigned int dmi1_wfifo_st : 2;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_DMSS_ASI_DFX_FIFO_UNION;
#endif
#define SOC_DMSS_ASI_DFX_FIFO_ra_wfifo_st_START (0)
#define SOC_DMSS_ASI_DFX_FIFO_ra_wfifo_st_END (1)
#define SOC_DMSS_ASI_DFX_FIFO_wd_cmdfifo_st_START (2)
#define SOC_DMSS_ASI_DFX_FIFO_wd_cmdfifo_st_END (3)
#define SOC_DMSS_ASI_DFX_FIFO_b_respfifo_st_START (4)
#define SOC_DMSS_ASI_DFX_FIFO_b_respfifo_st_END (5)
#define SOC_DMSS_ASI_DFX_FIFO_rd_rfifo_st_START (6)
#define SOC_DMSS_ASI_DFX_FIFO_rd_rfifo_st_END (7)
#define SOC_DMSS_ASI_DFX_FIFO_b_mififo_st_START (8)
#define SOC_DMSS_ASI_DFX_FIFO_b_mififo_st_END (9)
#define SOC_DMSS_ASI_DFX_FIFO_fkrd_fkfifo_st_START (10)
#define SOC_DMSS_ASI_DFX_FIFO_fkrd_fkfifo_st_END (11)
#define SOC_DMSS_ASI_DFX_FIFO_ami_wfifo_st_START (18)
#define SOC_DMSS_ASI_DFX_FIFO_ami_wfifo_st_END (19)
#define SOC_DMSS_ASI_DFX_FIFO_dmi0_wfifo_st_START (20)
#define SOC_DMSS_ASI_DFX_FIFO_dmi0_wfifo_st_END (21)
#define SOC_DMSS_ASI_DFX_FIFO_dmi1_wfifo_st_START (22)
#define SOC_DMSS_ASI_DFX_FIFO_dmi1_wfifo_st_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcs_fsm_st : 8;
        unsigned int awcs_fsm_st : 8;
        unsigned int rd_fsm_st : 5;
        unsigned int reserved_0 : 3;
        unsigned int wd_fsm_st : 6;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_DMSS_ASI_DFX_FSM_UNION;
#endif
#define SOC_DMSS_ASI_DFX_FSM_arcs_fsm_st_START (0)
#define SOC_DMSS_ASI_DFX_FSM_arcs_fsm_st_END (7)
#define SOC_DMSS_ASI_DFX_FSM_awcs_fsm_st_START (8)
#define SOC_DMSS_ASI_DFX_FSM_awcs_fsm_st_END (15)
#define SOC_DMSS_ASI_DFX_FSM_rd_fsm_st_START (16)
#define SOC_DMSS_ASI_DFX_FSM_rd_fsm_st_END (20)
#define SOC_DMSS_ASI_DFX_FSM_wd_fsm_st_START (24)
#define SOC_DMSS_ASI_DFX_FSM_wd_fsm_st_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_ostd_st : 6;
        unsigned int reserved : 2;
        unsigned int rd_ostd_st : 8;
        unsigned int wr_dmi_ostd_st : 8;
        unsigned int rd_dmi_ostd_st : 8;
    } reg;
} SOC_DMSS_ASI_DFX_OSTD_UNION;
#endif
#define SOC_DMSS_ASI_DFX_OSTD_wr_ostd_st_START (0)
#define SOC_DMSS_ASI_DFX_OSTD_wr_ostd_st_END (5)
#define SOC_DMSS_ASI_DFX_OSTD_rd_ostd_st_START (8)
#define SOC_DMSS_ASI_DFX_OSTD_rd_ostd_st_END (15)
#define SOC_DMSS_ASI_DFX_OSTD_wr_dmi_ostd_st_START (16)
#define SOC_DMSS_ASI_DFX_OSTD_wr_dmi_ostd_st_END (23)
#define SOC_DMSS_ASI_DFX_OSTD_rd_dmi_ostd_st_START (24)
#define SOC_DMSS_ASI_DFX_OSTD_rd_dmi_ostd_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rdr_cmd_que_cnt_st : 8;
        unsigned int rdr_buf_cnt_st : 8;
        unsigned int rdr_bid_use_cnt_st : 8;
        unsigned int reserved : 8;
    } reg;
} SOC_DMSS_ASI_DFX_RDR0_UNION;
#endif
#define SOC_DMSS_ASI_DFX_RDR0_rdr_cmd_que_cnt_st_START (0)
#define SOC_DMSS_ASI_DFX_RDR0_rdr_cmd_que_cnt_st_END (7)
#define SOC_DMSS_ASI_DFX_RDR0_rdr_buf_cnt_st_START (8)
#define SOC_DMSS_ASI_DFX_RDR0_rdr_buf_cnt_st_END (15)
#define SOC_DMSS_ASI_DFX_RDR0_rdr_bid_use_cnt_st_START (16)
#define SOC_DMSS_ASI_DFX_RDR0_rdr_bid_use_cnt_st_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rdr_buf_rcv_done_st : 32;
    } reg;
} SOC_DMSS_ASI_DFX_RDR1_UNION;
#endif
#define SOC_DMSS_ASI_DFX_RDR1_rdr_buf_rcv_done_st_START (0)
#define SOC_DMSS_ASI_DFX_RDR1_rdr_buf_rcv_done_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wooo_que_cnt : 8;
        unsigned int wooo_buf_cnt : 8;
        unsigned int reserved : 16;
    } reg;
} SOC_DMSS_ASI_DFX_WOOO_UNION;
#endif
#define SOC_DMSS_ASI_DFX_WOOO_wooo_que_cnt_START (0)
#define SOC_DMSS_ASI_DFX_WOOO_wooo_que_cnt_END (7)
#define SOC_DMSS_ASI_DFX_WOOO_wooo_buf_cnt_START (8)
#define SOC_DMSS_ASI_DFX_WOOO_wooo_buf_cnt_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfx_int_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_ASI_DFX_INT_EN_UNION;
#endif
#define SOC_DMSS_ASI_DFX_INT_EN_dfx_int_en_START (0)
#define SOC_DMSS_ASI_DFX_INT_EN_dfx_int_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfx_int_status : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_ASI_DFX_INT_STATUS_UNION;
#endif
#define SOC_DMSS_ASI_DFX_INT_STATUS_dfx_int_status_START (0)
#define SOC_DMSS_ASI_DFX_INT_STATUS_dfx_int_status_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int abnm_link_err : 1;
        unsigned int abnm_rdr_split : 1;
        unsigned int abnm_ex_len : 1;
        unsigned int abnm_nar_wrap : 1;
        unsigned int abnm_fix : 1;
        unsigned int abnm_len16 : 1;
        unsigned int reserved_0 : 2;
        unsigned int abnm_cmd_len : 4;
        unsigned int abnm_cmd_size : 3;
        unsigned int reserved_1 : 1;
        unsigned int abnm_cmd_mid : 15;
        unsigned int abnm_cmd_type : 1;
    } reg;
} SOC_DMSS_ASI_DFX_ABNM_INF0_UNION;
#endif
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_link_err_START (0)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_link_err_END (0)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_rdr_split_START (1)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_rdr_split_END (1)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_ex_len_START (2)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_ex_len_END (2)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_nar_wrap_START (3)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_nar_wrap_END (3)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_fix_START (4)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_fix_END (4)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_len16_START (5)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_len16_END (5)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_cmd_len_START (8)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_cmd_len_END (11)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_cmd_size_START (12)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_cmd_size_END (14)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_cmd_mid_START (16)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_cmd_mid_END (30)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_cmd_type_START (31)
#define SOC_DMSS_ASI_DFX_ABNM_INF0_abnm_cmd_type_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int abnm_wd_mismatch : 1;
        unsigned int reserved_0 : 3;
        unsigned int target_awlen : 4;
        unsigned int cur_wd_cnt : 4;
        unsigned int reserved_1 : 4;
        unsigned int abnm_cmd_addr_high : 16;
    } reg;
} SOC_DMSS_ASI_DFX_ABNM_INF1_UNION;
#endif
#define SOC_DMSS_ASI_DFX_ABNM_INF1_abnm_wd_mismatch_START (0)
#define SOC_DMSS_ASI_DFX_ABNM_INF1_abnm_wd_mismatch_END (0)
#define SOC_DMSS_ASI_DFX_ABNM_INF1_target_awlen_START (4)
#define SOC_DMSS_ASI_DFX_ABNM_INF1_target_awlen_END (7)
#define SOC_DMSS_ASI_DFX_ABNM_INF1_cur_wd_cnt_START (8)
#define SOC_DMSS_ASI_DFX_ABNM_INF1_cur_wd_cnt_END (11)
#define SOC_DMSS_ASI_DFX_ABNM_INF1_abnm_cmd_addr_high_START (16)
#define SOC_DMSS_ASI_DFX_ABNM_INF1_abnm_cmd_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int abnm_cmd_addr_low : 32;
    } reg;
} SOC_DMSS_ASI_DFX_ABNM_INF2_UNION;
#endif
#define SOC_DMSS_ASI_DFX_ABNM_INF2_abnm_cmd_addr_low_START (0)
#define SOC_DMSS_ASI_DFX_ABNM_INF2_abnm_cmd_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_status : 1;
        unsigned int int_overrun : 1;
        unsigned int reserved_0 : 2;
        unsigned int int_cnt : 4;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_DMSS_ASI_DFX_SEC0_UNION;
#endif
#define SOC_DMSS_ASI_DFX_SEC0_int_status_START (0)
#define SOC_DMSS_ASI_DFX_SEC0_int_status_END (0)
#define SOC_DMSS_ASI_DFX_SEC0_int_overrun_START (1)
#define SOC_DMSS_ASI_DFX_SEC0_int_overrun_END (1)
#define SOC_DMSS_ASI_DFX_SEC0_int_cnt_START (4)
#define SOC_DMSS_ASI_DFX_SEC0_int_cnt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int address_low : 32;
    } reg;
} SOC_DMSS_ASI_DFX_SEC1_UNION;
#endif
#define SOC_DMSS_ASI_DFX_SEC1_address_low_START (0)
#define SOC_DMSS_ASI_DFX_SEC1_address_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int address_high : 8;
        unsigned int reserved : 8;
        unsigned int privileged : 1;
        unsigned int nonsecure : 1;
        unsigned int access_burst : 2;
        unsigned int access_type : 1;
        unsigned int access_size : 3;
        unsigned int access_len : 4;
        unsigned int access_cache : 4;
    } reg;
} SOC_DMSS_ASI_DFX_SEC2_UNION;
#endif
#define SOC_DMSS_ASI_DFX_SEC2_address_high_START (0)
#define SOC_DMSS_ASI_DFX_SEC2_address_high_END (7)
#define SOC_DMSS_ASI_DFX_SEC2_privileged_START (16)
#define SOC_DMSS_ASI_DFX_SEC2_privileged_END (16)
#define SOC_DMSS_ASI_DFX_SEC2_nonsecure_START (17)
#define SOC_DMSS_ASI_DFX_SEC2_nonsecure_END (17)
#define SOC_DMSS_ASI_DFX_SEC2_access_burst_START (18)
#define SOC_DMSS_ASI_DFX_SEC2_access_burst_END (19)
#define SOC_DMSS_ASI_DFX_SEC2_access_type_START (20)
#define SOC_DMSS_ASI_DFX_SEC2_access_type_END (20)
#define SOC_DMSS_ASI_DFX_SEC2_access_size_START (21)
#define SOC_DMSS_ASI_DFX_SEC2_access_size_END (23)
#define SOC_DMSS_ASI_DFX_SEC2_access_len_START (24)
#define SOC_DMSS_ASI_DFX_SEC2_access_len_END (27)
#define SOC_DMSS_ASI_DFX_SEC2_access_cache_START (28)
#define SOC_DMSS_ASI_DFX_SEC2_access_cache_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int id : 24;
        unsigned int mid : 8;
    } reg;
} SOC_DMSS_ASI_DFX_SEC3_UNION;
#endif
#define SOC_DMSS_ASI_DFX_SEC3_id_START (0)
#define SOC_DMSS_ASI_DFX_SEC3_id_END (23)
#define SOC_DMSS_ASI_DFX_SEC3_mid_START (24)
#define SOC_DMSS_ASI_DFX_SEC3_mid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ra_wfifo_load_st : 1;
        unsigned int wd_cmdfifo_load_st : 1;
        unsigned int b_respfifo_load_st : 1;
        unsigned int rd_rfifo_load_st : 1;
        unsigned int reserved_0 : 1;
        unsigned int fkrd_fkfifo_load_st : 1;
        unsigned int rdr_cmd_que_load_st : 1;
        unsigned int rdr_bid_use_load_st : 1;
        unsigned int dmi_wfifo0_load_st : 1;
        unsigned int dmi_wfifo1_load_st : 1;
        unsigned int dmi_wfifo2_load_st : 1;
        unsigned int dmi_wfifo3_load_st : 1;
        unsigned int reserved_1 : 20;
    } reg;
} SOC_DMSS_ASI_DFX_LOAD_UNION;
#endif
#define SOC_DMSS_ASI_DFX_LOAD_ra_wfifo_load_st_START (0)
#define SOC_DMSS_ASI_DFX_LOAD_ra_wfifo_load_st_END (0)
#define SOC_DMSS_ASI_DFX_LOAD_wd_cmdfifo_load_st_START (1)
#define SOC_DMSS_ASI_DFX_LOAD_wd_cmdfifo_load_st_END (1)
#define SOC_DMSS_ASI_DFX_LOAD_b_respfifo_load_st_START (2)
#define SOC_DMSS_ASI_DFX_LOAD_b_respfifo_load_st_END (2)
#define SOC_DMSS_ASI_DFX_LOAD_rd_rfifo_load_st_START (3)
#define SOC_DMSS_ASI_DFX_LOAD_rd_rfifo_load_st_END (3)
#define SOC_DMSS_ASI_DFX_LOAD_fkrd_fkfifo_load_st_START (5)
#define SOC_DMSS_ASI_DFX_LOAD_fkrd_fkfifo_load_st_END (5)
#define SOC_DMSS_ASI_DFX_LOAD_rdr_cmd_que_load_st_START (6)
#define SOC_DMSS_ASI_DFX_LOAD_rdr_cmd_que_load_st_END (6)
#define SOC_DMSS_ASI_DFX_LOAD_rdr_bid_use_load_st_START (7)
#define SOC_DMSS_ASI_DFX_LOAD_rdr_bid_use_load_st_END (7)
#define SOC_DMSS_ASI_DFX_LOAD_dmi_wfifo0_load_st_START (8)
#define SOC_DMSS_ASI_DFX_LOAD_dmi_wfifo0_load_st_END (8)
#define SOC_DMSS_ASI_DFX_LOAD_dmi_wfifo1_load_st_START (9)
#define SOC_DMSS_ASI_DFX_LOAD_dmi_wfifo1_load_st_END (9)
#define SOC_DMSS_ASI_DFX_LOAD_dmi_wfifo2_load_st_START (10)
#define SOC_DMSS_ASI_DFX_LOAD_dmi_wfifo2_load_st_END (10)
#define SOC_DMSS_ASI_DFX_LOAD_dmi_wfifo3_load_st_START (11)
#define SOC_DMSS_ASI_DFX_LOAD_dmi_wfifo3_load_st_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtl_sec_rgn_num : 8;
        unsigned int rtl_mid_sec_rgn_num : 8;
        unsigned int rtl_enhn_sec_rgn_num : 4;
        unsigned int reserved_0 : 4;
        unsigned int rtl_mpu_en : 1;
        unsigned int reserved_1 : 7;
    } reg;
} SOC_DMSS_DMI_RTL_INF0_UNION;
#endif
#define SOC_DMSS_DMI_RTL_INF0_rtl_sec_rgn_num_START (0)
#define SOC_DMSS_DMI_RTL_INF0_rtl_sec_rgn_num_END (7)
#define SOC_DMSS_DMI_RTL_INF0_rtl_mid_sec_rgn_num_START (8)
#define SOC_DMSS_DMI_RTL_INF0_rtl_mid_sec_rgn_num_END (15)
#define SOC_DMSS_DMI_RTL_INF0_rtl_enhn_sec_rgn_num_START (16)
#define SOC_DMSS_DMI_RTL_INF0_rtl_enhn_sec_rgn_num_END (19)
#define SOC_DMSS_DMI_RTL_INF0_rtl_mpu_en_START (24)
#define SOC_DMSS_DMI_RTL_INF0_rtl_mpu_en_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ckg_byp_dmi : 1;
        unsigned int ckg_byp_dmi_wdfifo : 1;
        unsigned int ckg_byp_dmi_sec : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_DMSS_DMI_DYN_CKG_UNION;
#endif
#define SOC_DMSS_DMI_DYN_CKG_ckg_byp_dmi_START (0)
#define SOC_DMSS_DMI_DYN_CKG_ckg_byp_dmi_END (0)
#define SOC_DMSS_DMI_DYN_CKG_ckg_byp_dmi_wdfifo_START (1)
#define SOC_DMSS_DMI_DYN_CKG_ckg_byp_dmi_wdfifo_END (1)
#define SOC_DMSS_DMI_DYN_CKG_ckg_byp_dmi_sec_START (2)
#define SOC_DMSS_DMI_DYN_CKG_ckg_byp_dmi_sec_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_adpt_pri0 : 4;
        unsigned int rd_adpt_pri1 : 4;
        unsigned int rd_adpt_pri2 : 4;
        unsigned int rd_adpt_pri3 : 4;
        unsigned int rd_adpt_pri4 : 4;
        unsigned int rd_adpt_pri5 : 4;
        unsigned int reserved : 8;
    } reg;
} SOC_DMSS_DMI_RD_ADPT_UNION;
#endif
#define SOC_DMSS_DMI_RD_ADPT_rd_adpt_pri0_START (0)
#define SOC_DMSS_DMI_RD_ADPT_rd_adpt_pri0_END (3)
#define SOC_DMSS_DMI_RD_ADPT_rd_adpt_pri1_START (4)
#define SOC_DMSS_DMI_RD_ADPT_rd_adpt_pri1_END (7)
#define SOC_DMSS_DMI_RD_ADPT_rd_adpt_pri2_START (8)
#define SOC_DMSS_DMI_RD_ADPT_rd_adpt_pri2_END (11)
#define SOC_DMSS_DMI_RD_ADPT_rd_adpt_pri3_START (12)
#define SOC_DMSS_DMI_RD_ADPT_rd_adpt_pri3_END (15)
#define SOC_DMSS_DMI_RD_ADPT_rd_adpt_pri4_START (16)
#define SOC_DMSS_DMI_RD_ADPT_rd_adpt_pri4_END (19)
#define SOC_DMSS_DMI_RD_ADPT_rd_adpt_pri5_START (20)
#define SOC_DMSS_DMI_RD_ADPT_rd_adpt_pri5_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_adpt_pri0 : 4;
        unsigned int wr_adpt_pri1 : 4;
        unsigned int wr_adpt_pri2 : 4;
        unsigned int wr_adpt_pri3 : 4;
        unsigned int wr_adpt_pri4 : 4;
        unsigned int wr_adpt_pri5 : 4;
        unsigned int reserved : 8;
    } reg;
} SOC_DMSS_DMI_WR_ADPT_UNION;
#endif
#define SOC_DMSS_DMI_WR_ADPT_wr_adpt_pri0_START (0)
#define SOC_DMSS_DMI_WR_ADPT_wr_adpt_pri0_END (3)
#define SOC_DMSS_DMI_WR_ADPT_wr_adpt_pri1_START (4)
#define SOC_DMSS_DMI_WR_ADPT_wr_adpt_pri1_END (7)
#define SOC_DMSS_DMI_WR_ADPT_wr_adpt_pri2_START (8)
#define SOC_DMSS_DMI_WR_ADPT_wr_adpt_pri2_END (11)
#define SOC_DMSS_DMI_WR_ADPT_wr_adpt_pri3_START (12)
#define SOC_DMSS_DMI_WR_ADPT_wr_adpt_pri3_END (15)
#define SOC_DMSS_DMI_WR_ADPT_wr_adpt_pri4_START (16)
#define SOC_DMSS_DMI_WR_ADPT_wr_adpt_pri4_END (19)
#define SOC_DMSS_DMI_WR_ADPT_wr_adpt_pri5_START (20)
#define SOC_DMSS_DMI_WR_ADPT_wr_adpt_pri5_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_timeout_pri0 : 6;
        unsigned int reserved_0 : 2;
        unsigned int rd_timeout_pri1 : 6;
        unsigned int reserved_1 : 2;
        unsigned int rd_timeout_pri2 : 6;
        unsigned int reserved_2 : 2;
        unsigned int rd_timeout_pri3 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} SOC_DMSS_DMI_RD_TIMEOUT0_UNION;
#endif
#define SOC_DMSS_DMI_RD_TIMEOUT0_rd_timeout_pri0_START (0)
#define SOC_DMSS_DMI_RD_TIMEOUT0_rd_timeout_pri0_END (5)
#define SOC_DMSS_DMI_RD_TIMEOUT0_rd_timeout_pri1_START (8)
#define SOC_DMSS_DMI_RD_TIMEOUT0_rd_timeout_pri1_END (13)
#define SOC_DMSS_DMI_RD_TIMEOUT0_rd_timeout_pri2_START (16)
#define SOC_DMSS_DMI_RD_TIMEOUT0_rd_timeout_pri2_END (21)
#define SOC_DMSS_DMI_RD_TIMEOUT0_rd_timeout_pri3_START (24)
#define SOC_DMSS_DMI_RD_TIMEOUT0_rd_timeout_pri3_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_timeout_pri4 : 6;
        unsigned int reserved_0 : 2;
        unsigned int rd_timeout_pri5 : 6;
        unsigned int reserved_1 : 2;
        unsigned int rd_timeout_pri6 : 6;
        unsigned int reserved_2 : 10;
    } reg;
} SOC_DMSS_DMI_RD_TIMEOUT1_UNION;
#endif
#define SOC_DMSS_DMI_RD_TIMEOUT1_rd_timeout_pri4_START (0)
#define SOC_DMSS_DMI_RD_TIMEOUT1_rd_timeout_pri4_END (5)
#define SOC_DMSS_DMI_RD_TIMEOUT1_rd_timeout_pri5_START (8)
#define SOC_DMSS_DMI_RD_TIMEOUT1_rd_timeout_pri5_END (13)
#define SOC_DMSS_DMI_RD_TIMEOUT1_rd_timeout_pri6_START (16)
#define SOC_DMSS_DMI_RD_TIMEOUT1_rd_timeout_pri6_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_timeout_pri0 : 6;
        unsigned int reserved_0 : 2;
        unsigned int wr_timeout_pri1 : 6;
        unsigned int reserved_1 : 2;
        unsigned int wr_timeout_pri2 : 6;
        unsigned int reserved_2 : 2;
        unsigned int wr_timeout_pri3 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} SOC_DMSS_DMI_WR_TIMEOUT0_UNION;
#endif
#define SOC_DMSS_DMI_WR_TIMEOUT0_wr_timeout_pri0_START (0)
#define SOC_DMSS_DMI_WR_TIMEOUT0_wr_timeout_pri0_END (5)
#define SOC_DMSS_DMI_WR_TIMEOUT0_wr_timeout_pri1_START (8)
#define SOC_DMSS_DMI_WR_TIMEOUT0_wr_timeout_pri1_END (13)
#define SOC_DMSS_DMI_WR_TIMEOUT0_wr_timeout_pri2_START (16)
#define SOC_DMSS_DMI_WR_TIMEOUT0_wr_timeout_pri2_END (21)
#define SOC_DMSS_DMI_WR_TIMEOUT0_wr_timeout_pri3_START (24)
#define SOC_DMSS_DMI_WR_TIMEOUT0_wr_timeout_pri3_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_timeout_pri4 : 6;
        unsigned int reserved_0 : 2;
        unsigned int wr_timeout_pri5 : 6;
        unsigned int reserved_1 : 2;
        unsigned int wr_timeout_pri6 : 6;
        unsigned int reserved_2 : 10;
    } reg;
} SOC_DMSS_DMI_WR_TIMEOUT1_UNION;
#endif
#define SOC_DMSS_DMI_WR_TIMEOUT1_wr_timeout_pri4_START (0)
#define SOC_DMSS_DMI_WR_TIMEOUT1_wr_timeout_pri4_END (5)
#define SOC_DMSS_DMI_WR_TIMEOUT1_wr_timeout_pri5_START (8)
#define SOC_DMSS_DMI_WR_TIMEOUT1_wr_timeout_pri5_END (13)
#define SOC_DMSS_DMI_WR_TIMEOUT1_wr_timeout_pri6_START (16)
#define SOC_DMSS_DMI_WR_TIMEOUT1_wr_timeout_pri6_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rgn_base_addr : 24;
        unsigned int reserved : 7;
        unsigned int rgn_en : 1;
    } reg;
} SOC_DMSS_DMI_GID_RGN_MAP0_UNION;
#endif
#define SOC_DMSS_DMI_GID_RGN_MAP0_rgn_base_addr_START (0)
#define SOC_DMSS_DMI_GID_RGN_MAP0_rgn_base_addr_END (23)
#define SOC_DMSS_DMI_GID_RGN_MAP0_rgn_en_START (31)
#define SOC_DMSS_DMI_GID_RGN_MAP0_rgn_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rgn_top_addr : 24;
        unsigned int reserved : 4;
        unsigned int remap_gid : 4;
    } reg;
} SOC_DMSS_DMI_GID_RGN_MAP1_UNION;
#endif
#define SOC_DMSS_DMI_GID_RGN_MAP1_rgn_top_addr_START (0)
#define SOC_DMSS_DMI_GID_RGN_MAP1_rgn_top_addr_END (23)
#define SOC_DMSS_DMI_GID_RGN_MAP1_remap_gid_START (28)
#define SOC_DMSS_DMI_GID_RGN_MAP1_remap_gid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ld_rgns_sel : 32;
    } reg;
} SOC_DMSS_DMI_SEC_LD_SEL_UNION;
#endif
#define SOC_DMSS_DMI_SEC_LD_SEL_ld_rgns_sel_START (0)
#define SOC_DMSS_DMI_SEC_LD_SEL_ld_rgns_sel_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_DMI_SEC_INT_EN_UNION;
#endif
#define SOC_DMSS_DMI_SEC_INT_EN_int_en_START (0)
#define SOC_DMSS_DMI_SEC_INT_EN_int_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_status : 1;
        unsigned int int_overrun : 1;
        unsigned int reserved_0 : 2;
        unsigned int int_cnt : 4;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_DMSS_DMI_SEC_INT_STATUS_UNION;
#endif
#define SOC_DMSS_DMI_SEC_INT_STATUS_int_status_START (0)
#define SOC_DMSS_DMI_SEC_INT_STATUS_int_status_END (0)
#define SOC_DMSS_DMI_SEC_INT_STATUS_int_overrun_START (1)
#define SOC_DMSS_DMI_SEC_INT_STATUS_int_overrun_END (1)
#define SOC_DMSS_DMI_SEC_INT_STATUS_int_cnt_START (4)
#define SOC_DMSS_DMI_SEC_INT_STATUS_int_cnt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_DMI_ENHN_INT_EN_UNION;
#endif
#define SOC_DMSS_DMI_ENHN_INT_EN_int_en_START (0)
#define SOC_DMSS_DMI_ENHN_INT_EN_int_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_status : 1;
        unsigned int int_overrun : 1;
        unsigned int reserved_0 : 2;
        unsigned int int_cnt : 4;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_DMSS_DMI_ENHN_INT_STATUS_UNION;
#endif
#define SOC_DMSS_DMI_ENHN_INT_STATUS_int_status_START (0)
#define SOC_DMSS_DMI_ENHN_INT_STATUS_int_status_END (0)
#define SOC_DMSS_DMI_ENHN_INT_STATUS_int_overrun_START (1)
#define SOC_DMSS_DMI_ENHN_INT_STATUS_int_overrun_END (1)
#define SOC_DMSS_DMI_ENHN_INT_STATUS_int_cnt_START (4)
#define SOC_DMSS_DMI_ENHN_INT_STATUS_int_cnt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int address_low : 32;
    } reg;
} SOC_DMSS_DMI_SEC_FAIL_CMD_INF0_UNION;
#endif
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF0_address_low_START (0)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF0_address_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int address_high : 8;
        unsigned int reserved : 8;
        unsigned int privileged : 1;
        unsigned int nonsecure : 1;
        unsigned int access_burst : 2;
        unsigned int access_type : 1;
        unsigned int access_nsaid : 3;
        unsigned int access_len : 4;
        unsigned int access_portid : 4;
    } reg;
} SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_UNION;
#endif
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_address_high_START (0)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_address_high_END (7)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_privileged_START (16)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_privileged_END (16)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_nonsecure_START (17)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_nonsecure_END (17)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_access_burst_START (18)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_access_burst_END (19)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_access_type_START (20)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_access_type_END (20)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_access_nsaid_START (21)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_access_nsaid_END (23)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_access_len_START (24)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_access_len_END (27)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_access_portid_START (28)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF1_access_portid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int id : 24;
        unsigned int mid : 8;
    } reg;
} SOC_DMSS_DMI_SEC_FAIL_CMD_INF2_UNION;
#endif
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF2_id_START (0)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF2_id_END (23)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF2_mid_START (24)
#define SOC_DMSS_DMI_SEC_FAIL_CMD_INF2_mid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_DMI_MPU_INT_EN_UNION;
#endif
#define SOC_DMSS_DMI_MPU_INT_EN_int_en_START (0)
#define SOC_DMSS_DMI_MPU_INT_EN_int_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_status : 1;
        unsigned int int_overrun : 1;
        unsigned int reserved_0 : 2;
        unsigned int int_cnt : 4;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_DMSS_DMI_MPU_INT_STATUS_UNION;
#endif
#define SOC_DMSS_DMI_MPU_INT_STATUS_int_status_START (0)
#define SOC_DMSS_DMI_MPU_INT_STATUS_int_status_END (0)
#define SOC_DMSS_DMI_MPU_INT_STATUS_int_overrun_START (1)
#define SOC_DMSS_DMI_MPU_INT_STATUS_int_overrun_END (1)
#define SOC_DMSS_DMI_MPU_INT_STATUS_int_cnt_START (4)
#define SOC_DMSS_DMI_MPU_INT_STATUS_int_cnt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mpu_fail_low_address : 32;
    } reg;
} SOC_DMSS_DMI_MPU_FAIL_CMD_INF0_UNION;
#endif
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF0_mpu_fail_low_address_START (0)
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF0_mpu_fail_low_address_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mpu_fail_high_address : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_DMSS_DMI_MPU_FAIL_CMD_INF1_UNION;
#endif
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF1_mpu_fail_high_address_START (0)
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF1_mpu_fail_high_address_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mpu_fail_mid : 8;
        unsigned int mpu_fail_chmod : 2;
        unsigned int reserved_0 : 2;
        unsigned int mpu_fail_nsaid : 2;
        unsigned int reserved_1 : 2;
        unsigned int mpu_fail_attr : 1;
        unsigned int reserved_2 : 5;
        unsigned int mpu_fail_ch : 1;
        unsigned int reserved_3 : 1;
        unsigned int mpu_fail_type : 1;
        unsigned int reserved_4 : 3;
        unsigned int mpu_fail_portid : 4;
    } reg;
} SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_UNION;
#endif
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_mpu_fail_mid_START (0)
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_mpu_fail_mid_END (7)
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_mpu_fail_chmod_START (8)
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_mpu_fail_chmod_END (9)
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_mpu_fail_nsaid_START (12)
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_mpu_fail_nsaid_END (13)
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_mpu_fail_attr_START (16)
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_mpu_fail_attr_END (16)
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_mpu_fail_ch_START (22)
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_mpu_fail_ch_END (22)
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_mpu_fail_type_START (24)
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_mpu_fail_type_END (24)
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_mpu_fail_portid_START (28)
#define SOC_DMSS_DMI_MPU_FAIL_CMD_INF2_mpu_fail_portid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_DMI_ENHN_CFG_INT_EN_UNION;
#endif
#define SOC_DMSS_DMI_ENHN_CFG_INT_EN_int_en_START (0)
#define SOC_DMSS_DMI_ENHN_CFG_INT_EN_int_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_status : 1;
        unsigned int reserved_0 : 3;
        unsigned int cfg_type : 1;
        unsigned int reserved_1 : 7;
        unsigned int cfg_address : 20;
    } reg;
} SOC_DMSS_DMI_ENHN_CFG_INT_STATUS_UNION;
#endif
#define SOC_DMSS_DMI_ENHN_CFG_INT_STATUS_int_status_START (0)
#define SOC_DMSS_DMI_ENHN_CFG_INT_STATUS_int_status_END (0)
#define SOC_DMSS_DMI_ENHN_CFG_INT_STATUS_cfg_type_START (4)
#define SOC_DMSS_DMI_ENHN_CFG_INT_STATUS_cfg_type_END (4)
#define SOC_DMSS_DMI_ENHN_CFG_INT_STATUS_cfg_address_START (12)
#define SOC_DMSS_DMI_ENHN_CFG_INT_STATUS_cfg_address_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rgn_base_addr : 20;
        unsigned int reserved : 11;
        unsigned int rgn_en : 1;
    } reg;
} SOC_DMSS_DMI_SEC_RGN_MAP0_UNION;
#endif
#define SOC_DMSS_DMI_SEC_RGN_MAP0_rgn_base_addr_START (0)
#define SOC_DMSS_DMI_SEC_RGN_MAP0_rgn_base_addr_END (19)
#define SOC_DMSS_DMI_SEC_RGN_MAP0_rgn_en_START (31)
#define SOC_DMSS_DMI_SEC_RGN_MAP0_rgn_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rgn_top_addr : 20;
        unsigned int reserved : 8;
        unsigned int sp : 4;
    } reg;
} SOC_DMSS_DMI_SEC_RGN_MAP1_UNION;
#endif
#define SOC_DMSS_DMI_SEC_RGN_MAP1_rgn_top_addr_START (0)
#define SOC_DMSS_DMI_SEC_RGN_MAP1_rgn_top_addr_END (19)
#define SOC_DMSS_DMI_SEC_RGN_MAP1_sp_START (28)
#define SOC_DMSS_DMI_SEC_RGN_MAP1_sp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int etrust : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_DMI_SEC_RGN_ATTR_UNION;
#endif
#define SOC_DMSS_DMI_SEC_RGN_ATTR_etrust_START (0)
#define SOC_DMSS_DMI_SEC_RGN_ATTR_etrust_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_wr0 : 32;
    } reg;
} SOC_DMSS_DMI_SEC_MID_WR0_UNION;
#endif
#define SOC_DMSS_DMI_SEC_MID_WR0_mid_sel_wr0_START (0)
#define SOC_DMSS_DMI_SEC_MID_WR0_mid_sel_wr0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_wr1 : 32;
    } reg;
} SOC_DMSS_DMI_SEC_MID_WR1_UNION;
#endif
#define SOC_DMSS_DMI_SEC_MID_WR1_mid_sel_wr1_START (0)
#define SOC_DMSS_DMI_SEC_MID_WR1_mid_sel_wr1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_wr2 : 32;
    } reg;
} SOC_DMSS_DMI_SEC_MID_WR2_UNION;
#endif
#define SOC_DMSS_DMI_SEC_MID_WR2_mid_sel_wr2_START (0)
#define SOC_DMSS_DMI_SEC_MID_WR2_mid_sel_wr2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_wr3 : 32;
    } reg;
} SOC_DMSS_DMI_SEC_MID_WR3_UNION;
#endif
#define SOC_DMSS_DMI_SEC_MID_WR3_mid_sel_wr3_START (0)
#define SOC_DMSS_DMI_SEC_MID_WR3_mid_sel_wr3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_rd0 : 32;
    } reg;
} SOC_DMSS_DMI_SEC_MID_RD0_UNION;
#endif
#define SOC_DMSS_DMI_SEC_MID_RD0_mid_sel_rd0_START (0)
#define SOC_DMSS_DMI_SEC_MID_RD0_mid_sel_rd0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_rd1 : 32;
    } reg;
} SOC_DMSS_DMI_SEC_MID_RD1_UNION;
#endif
#define SOC_DMSS_DMI_SEC_MID_RD1_mid_sel_rd1_START (0)
#define SOC_DMSS_DMI_SEC_MID_RD1_mid_sel_rd1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_rd2 : 32;
    } reg;
} SOC_DMSS_DMI_SEC_MID_RD2_UNION;
#endif
#define SOC_DMSS_DMI_SEC_MID_RD2_mid_sel_rd2_START (0)
#define SOC_DMSS_DMI_SEC_MID_RD2_mid_sel_rd2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_rd3 : 32;
    } reg;
} SOC_DMSS_DMI_SEC_MID_RD3_UNION;
#endif
#define SOC_DMSS_DMI_SEC_MID_RD3_mid_sel_rd3_START (0)
#define SOC_DMSS_DMI_SEC_MID_RD3_mid_sel_rd3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dmi_busy : 1;
        unsigned int dmi_sec_busy : 1;
        unsigned int dcsc_busy : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_DMSS_DMI_DFX_MODULE_UNION;
#endif
#define SOC_DMSS_DMI_DFX_MODULE_dmi_busy_START (0)
#define SOC_DMSS_DMI_DFX_MODULE_dmi_busy_END (0)
#define SOC_DMSS_DMI_DFX_MODULE_dmi_sec_busy_START (1)
#define SOC_DMSS_DMI_DFX_MODULE_dmi_sec_busy_END (1)
#define SOC_DMSS_DMI_DFX_MODULE_dcsc_busy_START (2)
#define SOC_DMSS_DMI_DFX_MODULE_dcsc_busy_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wd_mux0_fifo_st : 2;
        unsigned int wd_mux1_fifo_st : 2;
        unsigned int wd_arb0_fifo_st : 2;
        unsigned int wd_arb1_fifo_st : 2;
        unsigned int wrsp_fifo_st : 2;
        unsigned int reserved : 22;
    } reg;
} SOC_DMSS_DMI_DFX_FIFO_UNION;
#endif
#define SOC_DMSS_DMI_DFX_FIFO_wd_mux0_fifo_st_START (0)
#define SOC_DMSS_DMI_DFX_FIFO_wd_mux0_fifo_st_END (1)
#define SOC_DMSS_DMI_DFX_FIFO_wd_mux1_fifo_st_START (2)
#define SOC_DMSS_DMI_DFX_FIFO_wd_mux1_fifo_st_END (3)
#define SOC_DMSS_DMI_DFX_FIFO_wd_arb0_fifo_st_START (4)
#define SOC_DMSS_DMI_DFX_FIFO_wd_arb0_fifo_st_END (5)
#define SOC_DMSS_DMI_DFX_FIFO_wd_arb1_fifo_st_START (6)
#define SOC_DMSS_DMI_DFX_FIFO_wd_arb1_fifo_st_END (7)
#define SOC_DMSS_DMI_DFX_FIFO_wrsp_fifo_st_START (8)
#define SOC_DMSS_DMI_DFX_FIFO_wrsp_fifo_st_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wfifo_empty : 14;
        unsigned int reserved_0 : 2;
        unsigned int wfifo_full : 14;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_DMSS_DMI_DFX_WD_FIFO_UNION;
#endif
#define SOC_DMSS_DMI_DFX_WD_FIFO_wfifo_empty_START (0)
#define SOC_DMSS_DMI_DFX_WD_FIFO_wfifo_empty_END (13)
#define SOC_DMSS_DMI_DFX_WD_FIFO_wfifo_full_START (16)
#define SOC_DMSS_DMI_DFX_WD_FIFO_wfifo_full_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dmi_grant : 16;
        unsigned int reserved_0: 8;
        unsigned int cmd_bp : 1;
        unsigned int w_bp : 1;
        unsigned int r_bp : 1;
        unsigned int b_bp : 1;
        unsigned int reserved_1: 4;
    } reg;
} SOC_DMSS_DMI_DFX_ST_UNION;
#endif
#define SOC_DMSS_DMI_DFX_ST_dmi_grant_START (0)
#define SOC_DMSS_DMI_DFX_ST_dmi_grant_END (15)
#define SOC_DMSS_DMI_DFX_ST_cmd_bp_START (24)
#define SOC_DMSS_DMI_DFX_ST_cmd_bp_END (24)
#define SOC_DMSS_DMI_DFX_ST_w_bp_START (25)
#define SOC_DMSS_DMI_DFX_ST_w_bp_END (25)
#define SOC_DMSS_DMI_DFX_ST_r_bp_START (26)
#define SOC_DMSS_DMI_DFX_ST_r_bp_END (26)
#define SOC_DMSS_DMI_DFX_ST_b_bp_START (27)
#define SOC_DMSS_DMI_DFX_ST_b_bp_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dcsc_m_r_bp : 1;
        unsigned int dcsc_m_w_bp : 1;
        unsigned int dcsc_m_cmd_bp : 1;
        unsigned int sc_m_r_bp : 1;
        unsigned int sc_m_b_bp : 1;
        unsigned int sc_m_w_bp : 1;
        unsigned int sc_m_cmd_bp : 1;
        unsigned int sc_s_r_bp : 1;
        unsigned int sc_s_b_bp : 1;
        unsigned int sc_s_w_bp : 1;
        unsigned int sc_s_cmd_bp : 1;
        unsigned int reserved_0 : 5;
        unsigned int dcsc_brsp_fifo_st : 2;
        unsigned int dcsc_wd_mux1_fifo_st : 2;
        unsigned int dcsc_wd_mux0_fifo_st : 2;
        unsigned int dcsc_wd_dmux1_fifo_st : 2;
        unsigned int dcsc_wd_dmux0_fifo_st : 2;
        unsigned int dcsc_trace_wd_mux1_fifo_st : 2;
        unsigned int dcsc_trace_wd_mux0_fifo_st : 2;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_DMSS_DMI_DFX_DCSC_UNION;
#endif
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_m_r_bp_START (0)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_m_r_bp_END (0)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_m_w_bp_START (1)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_m_w_bp_END (1)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_m_cmd_bp_START (2)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_m_cmd_bp_END (2)
#define SOC_DMSS_DMI_DFX_DCSC_sc_m_r_bp_START (3)
#define SOC_DMSS_DMI_DFX_DCSC_sc_m_r_bp_END (3)
#define SOC_DMSS_DMI_DFX_DCSC_sc_m_b_bp_START (4)
#define SOC_DMSS_DMI_DFX_DCSC_sc_m_b_bp_END (4)
#define SOC_DMSS_DMI_DFX_DCSC_sc_m_w_bp_START (5)
#define SOC_DMSS_DMI_DFX_DCSC_sc_m_w_bp_END (5)
#define SOC_DMSS_DMI_DFX_DCSC_sc_m_cmd_bp_START (6)
#define SOC_DMSS_DMI_DFX_DCSC_sc_m_cmd_bp_END (6)
#define SOC_DMSS_DMI_DFX_DCSC_sc_s_r_bp_START (7)
#define SOC_DMSS_DMI_DFX_DCSC_sc_s_r_bp_END (7)
#define SOC_DMSS_DMI_DFX_DCSC_sc_s_b_bp_START (8)
#define SOC_DMSS_DMI_DFX_DCSC_sc_s_b_bp_END (8)
#define SOC_DMSS_DMI_DFX_DCSC_sc_s_w_bp_START (9)
#define SOC_DMSS_DMI_DFX_DCSC_sc_s_w_bp_END (9)
#define SOC_DMSS_DMI_DFX_DCSC_sc_s_cmd_bp_START (10)
#define SOC_DMSS_DMI_DFX_DCSC_sc_s_cmd_bp_END (10)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_brsp_fifo_st_START (16)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_brsp_fifo_st_END (17)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_wd_mux1_fifo_st_START (18)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_wd_mux1_fifo_st_END (19)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_wd_mux0_fifo_st_START (20)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_wd_mux0_fifo_st_END (21)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_wd_dmux1_fifo_st_START (22)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_wd_dmux1_fifo_st_END (23)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_wd_dmux0_fifo_st_START (24)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_wd_dmux0_fifo_st_END (25)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_trace_wd_mux1_fifo_st_START (26)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_trace_wd_mux1_fifo_st_END (27)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_trace_wd_mux0_fifo_st_START (28)
#define SOC_DMSS_DMI_DFX_DCSC_dcsc_trace_wd_mux0_fifo_st_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtl_sec_rgn_num : 8;
        unsigned int rtl_mid_sec_rgn_num : 8;
        unsigned int rtl_enhn_sec_rgn_num : 4;
        unsigned int reserved_0 : 4;
        unsigned int rtl_mpu_en : 1;
        unsigned int reserved_1 : 7;
    } reg;
} SOC_DMSS_AMI_RTL_INF0_UNION;
#endif
#define SOC_DMSS_AMI_RTL_INF0_rtl_sec_rgn_num_START (0)
#define SOC_DMSS_AMI_RTL_INF0_rtl_sec_rgn_num_END (7)
#define SOC_DMSS_AMI_RTL_INF0_rtl_mid_sec_rgn_num_START (8)
#define SOC_DMSS_AMI_RTL_INF0_rtl_mid_sec_rgn_num_END (15)
#define SOC_DMSS_AMI_RTL_INF0_rtl_enhn_sec_rgn_num_START (16)
#define SOC_DMSS_AMI_RTL_INF0_rtl_enhn_sec_rgn_num_END (19)
#define SOC_DMSS_AMI_RTL_INF0_rtl_mpu_en_START (24)
#define SOC_DMSS_AMI_RTL_INF0_rtl_mpu_en_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ckg_byp_ami : 1;
        unsigned int ckg_byp_ami_wdfifo : 1;
        unsigned int ckg_byp_ami_sec : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_DMSS_AMI_DYN_CKG_UNION;
#endif
#define SOC_DMSS_AMI_DYN_CKG_ckg_byp_ami_START (0)
#define SOC_DMSS_AMI_DYN_CKG_ckg_byp_ami_END (0)
#define SOC_DMSS_AMI_DYN_CKG_ckg_byp_ami_wdfifo_START (1)
#define SOC_DMSS_AMI_DYN_CKG_ckg_byp_ami_wdfifo_END (1)
#define SOC_DMSS_AMI_DYN_CKG_ckg_byp_ami_sec_START (2)
#define SOC_DMSS_AMI_DYN_CKG_ckg_byp_ami_sec_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gid_disable : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_DMSS_AMI_GID_CTRL_UNION;
#endif
#define SOC_DMSS_AMI_GID_CTRL_gid_disable_START (0)
#define SOC_DMSS_AMI_GID_CTRL_gid_disable_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ld_rgns_sel : 32;
    } reg;
} SOC_DMSS_AMI_SEC_LD_SEL_UNION;
#endif
#define SOC_DMSS_AMI_SEC_LD_SEL_ld_rgns_sel_START (0)
#define SOC_DMSS_AMI_SEC_LD_SEL_ld_rgns_sel_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_AMI_SEC_INT_EN_UNION;
#endif
#define SOC_DMSS_AMI_SEC_INT_EN_int_en_START (0)
#define SOC_DMSS_AMI_SEC_INT_EN_int_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_status : 1;
        unsigned int int_overrun : 1;
        unsigned int reserved_0 : 2;
        unsigned int int_cnt : 4;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_DMSS_AMI_SEC_INT_STATUS_UNION;
#endif
#define SOC_DMSS_AMI_SEC_INT_STATUS_int_status_START (0)
#define SOC_DMSS_AMI_SEC_INT_STATUS_int_status_END (0)
#define SOC_DMSS_AMI_SEC_INT_STATUS_int_overrun_START (1)
#define SOC_DMSS_AMI_SEC_INT_STATUS_int_overrun_END (1)
#define SOC_DMSS_AMI_SEC_INT_STATUS_int_cnt_START (4)
#define SOC_DMSS_AMI_SEC_INT_STATUS_int_cnt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int address_low : 32;
    } reg;
} SOC_DMSS_AMI_SEC_FAIL_CMD_INF0_UNION;
#endif
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF0_address_low_START (0)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF0_address_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int address_high : 8;
        unsigned int reserved : 8;
        unsigned int privileged : 1;
        unsigned int nonsecure : 1;
        unsigned int access_burst : 2;
        unsigned int access_type : 1;
        unsigned int access_size : 3;
        unsigned int access_len : 4;
        unsigned int access_portid : 4;
    } reg;
} SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_UNION;
#endif
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_address_high_START (0)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_address_high_END (7)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_privileged_START (16)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_privileged_END (16)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_nonsecure_START (17)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_nonsecure_END (17)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_access_burst_START (18)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_access_burst_END (19)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_access_type_START (20)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_access_type_END (20)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_access_size_START (21)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_access_size_END (23)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_access_len_START (24)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_access_len_END (27)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_access_portid_START (28)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF1_access_portid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int id : 24;
        unsigned int mid : 8;
    } reg;
} SOC_DMSS_AMI_SEC_FAIL_CMD_INF2_UNION;
#endif
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF2_id_START (0)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF2_id_END (23)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF2_mid_START (24)
#define SOC_DMSS_AMI_SEC_FAIL_CMD_INF2_mid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_AMI_MPU_INT_EN_UNION;
#endif
#define SOC_DMSS_AMI_MPU_INT_EN_int_en_START (0)
#define SOC_DMSS_AMI_MPU_INT_EN_int_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_status : 1;
        unsigned int int_overrun : 1;
        unsigned int reserved_0 : 2;
        unsigned int int_cnt : 4;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_DMSS_AMI_MPU_INT_STATUS_UNION;
#endif
#define SOC_DMSS_AMI_MPU_INT_STATUS_int_status_START (0)
#define SOC_DMSS_AMI_MPU_INT_STATUS_int_status_END (0)
#define SOC_DMSS_AMI_MPU_INT_STATUS_int_overrun_START (1)
#define SOC_DMSS_AMI_MPU_INT_STATUS_int_overrun_END (1)
#define SOC_DMSS_AMI_MPU_INT_STATUS_int_cnt_START (4)
#define SOC_DMSS_AMI_MPU_INT_STATUS_int_cnt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mpu_fail_low_address : 32;
    } reg;
} SOC_DMSS_AMI_MPU_FAIL_CMD_INF0_UNION;
#endif
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF0_mpu_fail_low_address_START (0)
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF0_mpu_fail_low_address_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mpu_fail_high_address : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_DMSS_AMI_MPU_FAIL_CMD_INF1_UNION;
#endif
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF1_mpu_fail_high_address_START (0)
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF1_mpu_fail_high_address_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mpu_fail_mid : 8;
        unsigned int mpu_fail_chmod : 2;
        unsigned int reserved_0 : 2;
        unsigned int mpu_fail_nsaid : 2;
        unsigned int reserved_1 : 2;
        unsigned int mpu_fail_attr : 1;
        unsigned int reserved_2 : 7;
        unsigned int mpu_fail_type : 1;
        unsigned int reserved_3 : 3;
        unsigned int mpu_fail_portid : 4;
    } reg;
} SOC_DMSS_AMI_MPU_FAIL_CMD_INF2_UNION;
#endif
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF2_mpu_fail_mid_START (0)
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF2_mpu_fail_mid_END (7)
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF2_mpu_fail_chmod_START (8)
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF2_mpu_fail_chmod_END (9)
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF2_mpu_fail_nsaid_START (12)
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF2_mpu_fail_nsaid_END (13)
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF2_mpu_fail_attr_START (16)
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF2_mpu_fail_attr_END (16)
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF2_mpu_fail_type_START (24)
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF2_mpu_fail_type_END (24)
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF2_mpu_fail_portid_START (28)
#define SOC_DMSS_AMI_MPU_FAIL_CMD_INF2_mpu_fail_portid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rgn_base_addr : 20;
        unsigned int reserved : 11;
        unsigned int rgn_en : 1;
    } reg;
} SOC_DMSS_AMI_SEC_RGN_MAP0_UNION;
#endif
#define SOC_DMSS_AMI_SEC_RGN_MAP0_rgn_base_addr_START (0)
#define SOC_DMSS_AMI_SEC_RGN_MAP0_rgn_base_addr_END (19)
#define SOC_DMSS_AMI_SEC_RGN_MAP0_rgn_en_START (31)
#define SOC_DMSS_AMI_SEC_RGN_MAP0_rgn_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rgn_top_addr : 20;
        unsigned int reserved : 8;
        unsigned int sp : 4;
    } reg;
} SOC_DMSS_AMI_SEC_RGN_MAP1_UNION;
#endif
#define SOC_DMSS_AMI_SEC_RGN_MAP1_rgn_top_addr_START (0)
#define SOC_DMSS_AMI_SEC_RGN_MAP1_rgn_top_addr_END (19)
#define SOC_DMSS_AMI_SEC_RGN_MAP1_sp_START (28)
#define SOC_DMSS_AMI_SEC_RGN_MAP1_sp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int etrust : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_AMI_SEC_RGN_ATTR_UNION;
#endif
#define SOC_DMSS_AMI_SEC_RGN_ATTR_etrust_START (0)
#define SOC_DMSS_AMI_SEC_RGN_ATTR_etrust_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_wr0 : 32;
    } reg;
} SOC_DMSS_AMI_SEC_MID_WR0_UNION;
#endif
#define SOC_DMSS_AMI_SEC_MID_WR0_mid_sel_wr0_START (0)
#define SOC_DMSS_AMI_SEC_MID_WR0_mid_sel_wr0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_wr1 : 32;
    } reg;
} SOC_DMSS_AMI_SEC_MID_WR1_UNION;
#endif
#define SOC_DMSS_AMI_SEC_MID_WR1_mid_sel_wr1_START (0)
#define SOC_DMSS_AMI_SEC_MID_WR1_mid_sel_wr1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_wr2 : 32;
    } reg;
} SOC_DMSS_AMI_SEC_MID_WR2_UNION;
#endif
#define SOC_DMSS_AMI_SEC_MID_WR2_mid_sel_wr2_START (0)
#define SOC_DMSS_AMI_SEC_MID_WR2_mid_sel_wr2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_wr3 : 32;
    } reg;
} SOC_DMSS_AMI_SEC_MID_WR3_UNION;
#endif
#define SOC_DMSS_AMI_SEC_MID_WR3_mid_sel_wr3_START (0)
#define SOC_DMSS_AMI_SEC_MID_WR3_mid_sel_wr3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_rd0 : 32;
    } reg;
} SOC_DMSS_AMI_SEC_MID_RD0_UNION;
#endif
#define SOC_DMSS_AMI_SEC_MID_RD0_mid_sel_rd0_START (0)
#define SOC_DMSS_AMI_SEC_MID_RD0_mid_sel_rd0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_rd1 : 32;
    } reg;
} SOC_DMSS_AMI_SEC_MID_RD1_UNION;
#endif
#define SOC_DMSS_AMI_SEC_MID_RD1_mid_sel_rd1_START (0)
#define SOC_DMSS_AMI_SEC_MID_RD1_mid_sel_rd1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_rd2 : 32;
    } reg;
} SOC_DMSS_AMI_SEC_MID_RD2_UNION;
#endif
#define SOC_DMSS_AMI_SEC_MID_RD2_mid_sel_rd2_START (0)
#define SOC_DMSS_AMI_SEC_MID_RD2_mid_sel_rd2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_rd3 : 32;
    } reg;
} SOC_DMSS_AMI_SEC_MID_RD3_UNION;
#endif
#define SOC_DMSS_AMI_SEC_MID_RD3_mid_sel_rd3_START (0)
#define SOC_DMSS_AMI_SEC_MID_RD3_mid_sel_rd3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ami_busy : 1;
        unsigned int ami_sec_busy : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_DMSS_AMI_DFX_MODULE_UNION;
#endif
#define SOC_DMSS_AMI_DFX_MODULE_ami_busy_START (0)
#define SOC_DMSS_AMI_DFX_MODULE_ami_busy_END (0)
#define SOC_DMSS_AMI_DFX_MODULE_ami_sec_busy_START (1)
#define SOC_DMSS_AMI_DFX_MODULE_ami_sec_busy_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ami_cmd_fifo_st : 2;
        unsigned int ami_rcmd_fifo_st : 2;
        unsigned int ami_wcmd_fifo_st : 2;
        unsigned int ami_warb_fifo_st : 2;
        unsigned int ami_wd_fifo_st : 2;
        unsigned int mpu_rcmd_fifo_st : 2;
        unsigned int mpu_wcmd_fifo_st : 2;
        unsigned int mpu_wd_fifo_st : 2;
        unsigned int mpu_rfail_fifo_st : 2;
        unsigned int mpu_wfail_fifo_st : 2;
        unsigned int reserved : 12;
    } reg;
} SOC_DMSS_AMI_DFX_FIFO_UNION;
#endif
#define SOC_DMSS_AMI_DFX_FIFO_ami_cmd_fifo_st_START (0)
#define SOC_DMSS_AMI_DFX_FIFO_ami_cmd_fifo_st_END (1)
#define SOC_DMSS_AMI_DFX_FIFO_ami_rcmd_fifo_st_START (2)
#define SOC_DMSS_AMI_DFX_FIFO_ami_rcmd_fifo_st_END (3)
#define SOC_DMSS_AMI_DFX_FIFO_ami_wcmd_fifo_st_START (4)
#define SOC_DMSS_AMI_DFX_FIFO_ami_wcmd_fifo_st_END (5)
#define SOC_DMSS_AMI_DFX_FIFO_ami_warb_fifo_st_START (6)
#define SOC_DMSS_AMI_DFX_FIFO_ami_warb_fifo_st_END (7)
#define SOC_DMSS_AMI_DFX_FIFO_ami_wd_fifo_st_START (8)
#define SOC_DMSS_AMI_DFX_FIFO_ami_wd_fifo_st_END (9)
#define SOC_DMSS_AMI_DFX_FIFO_mpu_rcmd_fifo_st_START (10)
#define SOC_DMSS_AMI_DFX_FIFO_mpu_rcmd_fifo_st_END (11)
#define SOC_DMSS_AMI_DFX_FIFO_mpu_wcmd_fifo_st_START (12)
#define SOC_DMSS_AMI_DFX_FIFO_mpu_wcmd_fifo_st_END (13)
#define SOC_DMSS_AMI_DFX_FIFO_mpu_wd_fifo_st_START (14)
#define SOC_DMSS_AMI_DFX_FIFO_mpu_wd_fifo_st_END (15)
#define SOC_DMSS_AMI_DFX_FIFO_mpu_rfail_fifo_st_START (16)
#define SOC_DMSS_AMI_DFX_FIFO_mpu_rfail_fifo_st_END (17)
#define SOC_DMSS_AMI_DFX_FIFO_mpu_wfail_fifo_st_START (18)
#define SOC_DMSS_AMI_DFX_FIFO_mpu_wfail_fifo_st_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ami_grant : 16;
        unsigned int ami_table_cnt : 8;
        unsigned int ar_bp : 1;
        unsigned int aw_bp : 1;
        unsigned int w_bp : 1;
        unsigned int r_bp : 1;
        unsigned int b_bp : 1;
        unsigned int reserved : 3;
    } reg;
} SOC_DMSS_AMI_DFX_ST_UNION;
#endif
#define SOC_DMSS_AMI_DFX_ST_ami_grant_START (0)
#define SOC_DMSS_AMI_DFX_ST_ami_grant_END (15)
#define SOC_DMSS_AMI_DFX_ST_ami_table_cnt_START (16)
#define SOC_DMSS_AMI_DFX_ST_ami_table_cnt_END (23)
#define SOC_DMSS_AMI_DFX_ST_ar_bp_START (24)
#define SOC_DMSS_AMI_DFX_ST_ar_bp_END (24)
#define SOC_DMSS_AMI_DFX_ST_aw_bp_START (25)
#define SOC_DMSS_AMI_DFX_ST_aw_bp_END (25)
#define SOC_DMSS_AMI_DFX_ST_w_bp_START (26)
#define SOC_DMSS_AMI_DFX_ST_w_bp_END (26)
#define SOC_DMSS_AMI_DFX_ST_r_bp_START (27)
#define SOC_DMSS_AMI_DFX_ST_r_bp_END (27)
#define SOC_DMSS_AMI_DFX_ST_b_bp_START (28)
#define SOC_DMSS_AMI_DFX_ST_b_bp_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtl_tag : 8;
        unsigned int reserved_0: 8;
        unsigned int version : 12;
        unsigned int reserved_1: 4;
    } reg;
} SOC_DMSS_GLB_RTL_VER_UNION;
#endif
#define SOC_DMSS_GLB_RTL_VER_rtl_tag_START (0)
#define SOC_DMSS_GLB_RTL_VER_rtl_tag_END (7)
#define SOC_DMSS_GLB_RTL_VER_version_START (16)
#define SOC_DMSS_GLB_RTL_VER_version_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtl_addr_width : 4;
        unsigned int rtl_id_width : 5;
        unsigned int reserved_0 : 3;
        unsigned int rtl_mid_width : 4;
        unsigned int rtl_dmi_num : 1;
        unsigned int reserved_1 : 1;
        unsigned int rtl_ch_num_pre_dmi : 1;
        unsigned int reserved_2 : 1;
        unsigned int rtl_asi_num : 4;
        unsigned int rtl_exclusive_num : 6;
        unsigned int reserved_3 : 2;
    } reg;
} SOC_DMSS_GLB_RTL_INF0_UNION;
#endif
#define SOC_DMSS_GLB_RTL_INF0_rtl_addr_width_START (0)
#define SOC_DMSS_GLB_RTL_INF0_rtl_addr_width_END (3)
#define SOC_DMSS_GLB_RTL_INF0_rtl_id_width_START (4)
#define SOC_DMSS_GLB_RTL_INF0_rtl_id_width_END (8)
#define SOC_DMSS_GLB_RTL_INF0_rtl_mid_width_START (12)
#define SOC_DMSS_GLB_RTL_INF0_rtl_mid_width_END (15)
#define SOC_DMSS_GLB_RTL_INF0_rtl_dmi_num_START (16)
#define SOC_DMSS_GLB_RTL_INF0_rtl_dmi_num_END (16)
#define SOC_DMSS_GLB_RTL_INF0_rtl_ch_num_pre_dmi_START (18)
#define SOC_DMSS_GLB_RTL_INF0_rtl_ch_num_pre_dmi_END (18)
#define SOC_DMSS_GLB_RTL_INF0_rtl_asi_num_START (20)
#define SOC_DMSS_GLB_RTL_INF0_rtl_asi_num_END (23)
#define SOC_DMSS_GLB_RTL_INF0_rtl_exclusive_num_START (24)
#define SOC_DMSS_GLB_RTL_INF0_rtl_exclusive_num_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtl_rbuf_buf_num : 6;
        unsigned int reserved_0 : 2;
        unsigned int rtl_mst_flux_num : 4;
        unsigned int rtl_perf_stat : 1;
        unsigned int reserved_1 : 19;
    } reg;
} SOC_DMSS_GLB_RTL_INF1_UNION;
#endif
#define SOC_DMSS_GLB_RTL_INF1_rtl_rbuf_buf_num_START (0)
#define SOC_DMSS_GLB_RTL_INF1_rtl_rbuf_buf_num_END (5)
#define SOC_DMSS_GLB_RTL_INF1_rtl_mst_flux_num_START (8)
#define SOC_DMSS_GLB_RTL_INF1_rtl_mst_flux_num_END (11)
#define SOC_DMSS_GLB_RTL_INF1_rtl_perf_stat_START (12)
#define SOC_DMSS_GLB_RTL_INF1_rtl_perf_stat_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ram_tmod0 : 16;
        unsigned int ram_tmod1 : 16;
    } reg;
} SOC_DMSS_GLB_RAM_TMOD0_UNION;
#endif
#define SOC_DMSS_GLB_RAM_TMOD0_ram_tmod0_START (0)
#define SOC_DMSS_GLB_RAM_TMOD0_ram_tmod0_END (15)
#define SOC_DMSS_GLB_RAM_TMOD0_ram_tmod1_START (16)
#define SOC_DMSS_GLB_RAM_TMOD0_ram_tmod1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ram_tmod2 : 16;
        unsigned int ram_tmod3 : 16;
    } reg;
} SOC_DMSS_GLB_RAM_TMOD1_UNION;
#endif
#define SOC_DMSS_GLB_RAM_TMOD1_ram_tmod2_START (0)
#define SOC_DMSS_GLB_RAM_TMOD1_ram_tmod2_END (15)
#define SOC_DMSS_GLB_RAM_TMOD1_ram_tmod3_START (16)
#define SOC_DMSS_GLB_RAM_TMOD1_ram_tmod3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ram_smod : 26;
        unsigned int reserved : 6;
    } reg;
} SOC_DMSS_GLB_RAM_SMOD_UNION;
#endif
#define SOC_DMSS_GLB_RAM_SMOD_ram_smod_START (0)
#define SOC_DMSS_GLB_RAM_SMOD_ram_smod_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ckg_byp_rbuf_src : 1;
        unsigned int ckg_byp_rbuf_dst : 1;
        unsigned int ckg_byp_ra_rs : 1;
        unsigned int ckg_byp_rbuf_rs : 1;
        unsigned int ckg_byp_dcq : 1;
        unsigned int ckg_byp_mst_flux : 1;
        unsigned int ckg_byp_dcq_stat : 1;
        unsigned int ckg_byp_ram : 1;
        unsigned int ckg_byp_trace : 1;
        unsigned int ckg_byp_cfg : 1;
        unsigned int ckg_byp_ptrn_rt : 1;
        unsigned int reserved : 21;
    } reg;
} SOC_DMSS_GLB_DYN_CKG_UNION;
#endif
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_rbuf_src_START (0)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_rbuf_src_END (0)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_rbuf_dst_START (1)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_rbuf_dst_END (1)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_ra_rs_START (2)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_ra_rs_END (2)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_rbuf_rs_START (3)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_rbuf_rs_END (3)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_dcq_START (4)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_dcq_END (4)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_mst_flux_START (5)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_mst_flux_END (5)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_dcq_stat_START (6)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_dcq_stat_END (6)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_ram_START (7)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_ram_END (7)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_trace_START (8)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_trace_END (8)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_cfg_START (9)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_cfg_END (9)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_ptrn_rt_START (10)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_ptrn_rt_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int asi_dfx_int : 16;
        unsigned int ami_prot_int : 1;
        unsigned int ami_enhn_int : 1;
        unsigned int rsp_err_int : 1;
        unsigned int reserved : 13;
    } reg;
} SOC_DMSS_GLB_INT_STATUS0_UNION;
#endif
#define SOC_DMSS_GLB_INT_STATUS0_asi_dfx_int_START (0)
#define SOC_DMSS_GLB_INT_STATUS0_asi_dfx_int_END (15)
#define SOC_DMSS_GLB_INT_STATUS0_ami_prot_int_START (16)
#define SOC_DMSS_GLB_INT_STATUS0_ami_prot_int_END (16)
#define SOC_DMSS_GLB_INT_STATUS0_ami_enhn_int_START (17)
#define SOC_DMSS_GLB_INT_STATUS0_ami_enhn_int_END (17)
#define SOC_DMSS_GLB_INT_STATUS0_rsp_err_int_START (18)
#define SOC_DMSS_GLB_INT_STATUS0_rsp_err_int_END (18)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int asi_dfx_int_clear : 16;
        unsigned int ami_prot_int_clear : 1;
        unsigned int ami_enhn_int_clear : 1;
        unsigned int rsp_err_int_clear : 1;
        unsigned int reserved : 13;
    } reg;
} SOC_DMSS_GLB_INT_CLEAR0_UNION;
#endif
#define SOC_DMSS_GLB_INT_CLEAR0_asi_dfx_int_clear_START (0)
#define SOC_DMSS_GLB_INT_CLEAR0_asi_dfx_int_clear_END (15)
#define SOC_DMSS_GLB_INT_CLEAR0_ami_prot_int_clear_START (16)
#define SOC_DMSS_GLB_INT_CLEAR0_ami_prot_int_clear_END (16)
#define SOC_DMSS_GLB_INT_CLEAR0_ami_enhn_int_clear_START (17)
#define SOC_DMSS_GLB_INT_CLEAR0_ami_enhn_int_clear_END (17)
#define SOC_DMSS_GLB_INT_CLEAR0_rsp_err_int_clear_START (18)
#define SOC_DMSS_GLB_INT_CLEAR0_rsp_err_int_clear_END (18)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int asi_sec_int : 16;
        unsigned int dmi_sec_int : 2;
        unsigned int dmi_enhn_int : 2;
        unsigned int dmi_enhn_cfg_int : 1;
        unsigned int ami_sec_int : 1;
        unsigned int mpu_int : 1;
        unsigned int detect_int : 1;
        unsigned int trace_int : 1;
        unsigned int rbuf_stat_int : 1;
        unsigned int reserved : 6;
    } reg;
} SOC_DMSS_GLB_INT_STATUS1_UNION;
#endif
#define SOC_DMSS_GLB_INT_STATUS1_asi_sec_int_START (0)
#define SOC_DMSS_GLB_INT_STATUS1_asi_sec_int_END (15)
#define SOC_DMSS_GLB_INT_STATUS1_dmi_sec_int_START (16)
#define SOC_DMSS_GLB_INT_STATUS1_dmi_sec_int_END (17)
#define SOC_DMSS_GLB_INT_STATUS1_dmi_enhn_int_START (18)
#define SOC_DMSS_GLB_INT_STATUS1_dmi_enhn_int_END (19)
#define SOC_DMSS_GLB_INT_STATUS1_dmi_enhn_cfg_int_START (20)
#define SOC_DMSS_GLB_INT_STATUS1_dmi_enhn_cfg_int_END (20)
#define SOC_DMSS_GLB_INT_STATUS1_ami_sec_int_START (21)
#define SOC_DMSS_GLB_INT_STATUS1_ami_sec_int_END (21)
#define SOC_DMSS_GLB_INT_STATUS1_mpu_int_START (22)
#define SOC_DMSS_GLB_INT_STATUS1_mpu_int_END (22)
#define SOC_DMSS_GLB_INT_STATUS1_detect_int_START (23)
#define SOC_DMSS_GLB_INT_STATUS1_detect_int_END (23)
#define SOC_DMSS_GLB_INT_STATUS1_trace_int_START (24)
#define SOC_DMSS_GLB_INT_STATUS1_trace_int_END (24)
#define SOC_DMSS_GLB_INT_STATUS1_rbuf_stat_int_START (25)
#define SOC_DMSS_GLB_INT_STATUS1_rbuf_stat_int_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int asi_sec_int_clear : 16;
        unsigned int dmi_sec_int_clear : 2;
        unsigned int dmi_enhn_int_clear : 2;
        unsigned int dmi_enhn_cfg_int_clear : 1;
        unsigned int ami_sec_int_clear : 1;
        unsigned int mpu_int_clear : 1;
        unsigned int detect_int_clear : 1;
        unsigned int trace_int_clear : 1;
        unsigned int rbuf_stat_int_clear : 1;
        unsigned int reserved : 6;
    } reg;
} SOC_DMSS_GLB_INT_CLEAR1_UNION;
#endif
#define SOC_DMSS_GLB_INT_CLEAR1_asi_sec_int_clear_START (0)
#define SOC_DMSS_GLB_INT_CLEAR1_asi_sec_int_clear_END (15)
#define SOC_DMSS_GLB_INT_CLEAR1_dmi_sec_int_clear_START (16)
#define SOC_DMSS_GLB_INT_CLEAR1_dmi_sec_int_clear_END (17)
#define SOC_DMSS_GLB_INT_CLEAR1_dmi_enhn_int_clear_START (18)
#define SOC_DMSS_GLB_INT_CLEAR1_dmi_enhn_int_clear_END (19)
#define SOC_DMSS_GLB_INT_CLEAR1_dmi_enhn_cfg_int_clear_START (20)
#define SOC_DMSS_GLB_INT_CLEAR1_dmi_enhn_cfg_int_clear_END (20)
#define SOC_DMSS_GLB_INT_CLEAR1_ami_sec_int_clear_START (21)
#define SOC_DMSS_GLB_INT_CLEAR1_ami_sec_int_clear_END (21)
#define SOC_DMSS_GLB_INT_CLEAR1_mpu_int_clear_START (22)
#define SOC_DMSS_GLB_INT_CLEAR1_mpu_int_clear_END (22)
#define SOC_DMSS_GLB_INT_CLEAR1_detect_int_clear_START (23)
#define SOC_DMSS_GLB_INT_CLEAR1_detect_int_clear_END (23)
#define SOC_DMSS_GLB_INT_CLEAR1_trace_int_clear_START (24)
#define SOC_DMSS_GLB_INT_CLEAR1_trace_int_clear_END (24)
#define SOC_DMSS_GLB_INT_CLEAR1_rbuf_stat_int_clear_START (25)
#define SOC_DMSS_GLB_INT_CLEAR1_rbuf_stat_int_clear_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int exclu_en : 1;
        unsigned int reserved_0 : 3;
        unsigned int exclu_id_sel : 3;
        unsigned int reserved_1 : 1;
        unsigned int exclu_clrmonhk_en : 1;
        unsigned int reserved_2 : 23;
    } reg;
} SOC_DMSS_GLB_EXCLU_UNION;
#endif
#define SOC_DMSS_GLB_EXCLU_exclu_en_START (0)
#define SOC_DMSS_GLB_EXCLU_exclu_en_END (0)
#define SOC_DMSS_GLB_EXCLU_exclu_id_sel_START (4)
#define SOC_DMSS_GLB_EXCLU_exclu_id_sel_END (6)
#define SOC_DMSS_GLB_EXCLU_exclu_clrmonhk_en_START (8)
#define SOC_DMSS_GLB_EXCLU_exclu_clrmonhk_en_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intlv_mode : 4;
        unsigned int intlv_gran : 2;
        unsigned int reserved_0 : 6;
        unsigned int ch4_order : 1;
        unsigned int reserved_1 : 19;
    } reg;
} SOC_DMSS_GLB_ADDR_INTLV_UNION;
#endif
#define SOC_DMSS_GLB_ADDR_INTLV_intlv_mode_START (0)
#define SOC_DMSS_GLB_ADDR_INTLV_intlv_mode_END (3)
#define SOC_DMSS_GLB_ADDR_INTLV_intlv_gran_START (4)
#define SOC_DMSS_GLB_ADDR_INTLV_intlv_gran_END (5)
#define SOC_DMSS_GLB_ADDR_INTLV_ch4_order_START (12)
#define SOC_DMSS_GLB_ADDR_INTLV_ch4_order_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bnk_scrmbl : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_DMSS_GLB_ADDR_SCRMBL_UNION;
#endif
#define SOC_DMSS_GLB_ADDR_SCRMBL_bnk_scrmbl_START (0)
#define SOC_DMSS_GLB_ADDR_SCRMBL_bnk_scrmbl_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rbuf_byp_path_en : 1;
        unsigned int reserved : 15;
        unsigned int rbuf_cnt_wl : 8;
        unsigned int rbuf_urg_wl : 8;
    } reg;
} SOC_DMSS_GLB_RBUF_CTRL_UNION;
#endif
#define SOC_DMSS_GLB_RBUF_CTRL_rbuf_byp_path_en_START (0)
#define SOC_DMSS_GLB_RBUF_CTRL_rbuf_byp_path_en_END (0)
#define SOC_DMSS_GLB_RBUF_CTRL_rbuf_cnt_wl_START (16)
#define SOC_DMSS_GLB_RBUF_CTRL_rbuf_cnt_wl_END (23)
#define SOC_DMSS_GLB_RBUF_CTRL_rbuf_urg_wl_START (24)
#define SOC_DMSS_GLB_RBUF_CTRL_rbuf_urg_wl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rbuf_trans_req : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_DMSS_GLB_RBUF_TRANS_CTRL_UNION;
#endif
#define SOC_DMSS_GLB_RBUF_TRANS_CTRL_rbuf_trans_req_START (0)
#define SOC_DMSS_GLB_RBUF_TRANS_CTRL_rbuf_trans_req_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rbuf_bp_grp : 16;
        unsigned int rbuf_bp_wl : 16;
    } reg;
} SOC_DMSS_GLB_RBUF_BP_UNION;
#endif
#define SOC_DMSS_GLB_RBUF_BP_rbuf_bp_grp_START (0)
#define SOC_DMSS_GLB_RBUF_BP_rbuf_bp_grp_END (15)
#define SOC_DMSS_GLB_RBUF_BP_rbuf_bp_wl_START (16)
#define SOC_DMSS_GLB_RBUF_BP_rbuf_bp_wl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_GLB_RSP_ERR_INT_EN_UNION;
#endif
#define SOC_DMSS_GLB_RSP_ERR_INT_EN_int_en_START (0)
#define SOC_DMSS_GLB_RSP_ERR_INT_EN_int_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_status : 1;
        unsigned int reserved_0 : 3;
        unsigned int rsp_err_type : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_DMSS_GLB_RSP_ERR_INT_STATUS_UNION;
#endif
#define SOC_DMSS_GLB_RSP_ERR_INT_STATUS_int_status_START (0)
#define SOC_DMSS_GLB_RSP_ERR_INT_STATUS_int_status_END (0)
#define SOC_DMSS_GLB_RSP_ERR_INT_STATUS_rsp_err_type_START (4)
#define SOC_DMSS_GLB_RSP_ERR_INT_STATUS_rsp_err_type_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wd_fifo_wl : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_DMSS_GLB_AMI_CTRL_UNION;
#endif
#define SOC_DMSS_GLB_AMI_CTRL_wd_fifo_wl_START (0)
#define SOC_DMSS_GLB_AMI_CTRL_wd_fifo_wl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dcq_en : 1;
        unsigned int dcq_mode : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_DMSS_GLB_DCQ_CTRL_UNION;
#endif
#define SOC_DMSS_GLB_DCQ_CTRL_dcq_en_START (0)
#define SOC_DMSS_GLB_DCQ_CTRL_dcq_en_END (0)
#define SOC_DMSS_GLB_DCQ_CTRL_dcq_mode_START (1)
#define SOC_DMSS_GLB_DCQ_CTRL_dcq_mode_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 8;
        unsigned int dcq_pri1_lvl : 8;
        unsigned int dcq_pri2_lvl : 8;
        unsigned int dcq_pri3_lvl : 8;
    } reg;
} SOC_DMSS_GLB_DCQ_PRILVL0_UNION;
#endif
#define SOC_DMSS_GLB_DCQ_PRILVL0_dcq_pri1_lvl_START (8)
#define SOC_DMSS_GLB_DCQ_PRILVL0_dcq_pri1_lvl_END (15)
#define SOC_DMSS_GLB_DCQ_PRILVL0_dcq_pri2_lvl_START (16)
#define SOC_DMSS_GLB_DCQ_PRILVL0_dcq_pri2_lvl_END (23)
#define SOC_DMSS_GLB_DCQ_PRILVL0_dcq_pri3_lvl_START (24)
#define SOC_DMSS_GLB_DCQ_PRILVL0_dcq_pri3_lvl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dcq_pri4_lvl : 8;
        unsigned int dcq_pri5_lvl : 8;
        unsigned int dcq_pri6_lvl : 8;
        unsigned int dcq_pri7_lvl : 8;
    } reg;
} SOC_DMSS_GLB_DCQ_PRILVL1_UNION;
#endif
#define SOC_DMSS_GLB_DCQ_PRILVL1_dcq_pri4_lvl_START (0)
#define SOC_DMSS_GLB_DCQ_PRILVL1_dcq_pri4_lvl_END (7)
#define SOC_DMSS_GLB_DCQ_PRILVL1_dcq_pri5_lvl_START (8)
#define SOC_DMSS_GLB_DCQ_PRILVL1_dcq_pri5_lvl_END (15)
#define SOC_DMSS_GLB_DCQ_PRILVL1_dcq_pri6_lvl_START (16)
#define SOC_DMSS_GLB_DCQ_PRILVL1_dcq_pri6_lvl_END (23)
#define SOC_DMSS_GLB_DCQ_PRILVL1_dcq_pri7_lvl_START (24)
#define SOC_DMSS_GLB_DCQ_PRILVL1_dcq_pri7_lvl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 8;
        unsigned int dcq_wr_pri1_lvl : 8;
        unsigned int dcq_wr_pri2_lvl : 8;
        unsigned int dcq_wr_pri3_lvl : 8;
    } reg;
} SOC_DMSS_GLB_DCQ_WR_PRILVL0_UNION;
#endif
#define SOC_DMSS_GLB_DCQ_WR_PRILVL0_dcq_wr_pri1_lvl_START (8)
#define SOC_DMSS_GLB_DCQ_WR_PRILVL0_dcq_wr_pri1_lvl_END (15)
#define SOC_DMSS_GLB_DCQ_WR_PRILVL0_dcq_wr_pri2_lvl_START (16)
#define SOC_DMSS_GLB_DCQ_WR_PRILVL0_dcq_wr_pri2_lvl_END (23)
#define SOC_DMSS_GLB_DCQ_WR_PRILVL0_dcq_wr_pri3_lvl_START (24)
#define SOC_DMSS_GLB_DCQ_WR_PRILVL0_dcq_wr_pri3_lvl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dcq_wr_pri4_lvl : 8;
        unsigned int dcq_wr_pri5_lvl : 8;
        unsigned int dcq_wr_pri6_lvl : 8;
        unsigned int dcq_wr_pri7_lvl : 8;
    } reg;
} SOC_DMSS_GLB_DCQ_WR_PRILVL1_UNION;
#endif
#define SOC_DMSS_GLB_DCQ_WR_PRILVL1_dcq_wr_pri4_lvl_START (0)
#define SOC_DMSS_GLB_DCQ_WR_PRILVL1_dcq_wr_pri4_lvl_END (7)
#define SOC_DMSS_GLB_DCQ_WR_PRILVL1_dcq_wr_pri5_lvl_START (8)
#define SOC_DMSS_GLB_DCQ_WR_PRILVL1_dcq_wr_pri5_lvl_END (15)
#define SOC_DMSS_GLB_DCQ_WR_PRILVL1_dcq_wr_pri6_lvl_START (16)
#define SOC_DMSS_GLB_DCQ_WR_PRILVL1_dcq_wr_pri6_lvl_END (23)
#define SOC_DMSS_GLB_DCQ_WR_PRILVL1_dcq_wr_pri7_lvl_START (24)
#define SOC_DMSS_GLB_DCQ_WR_PRILVL1_dcq_wr_pri7_lvl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_en : 1;
        unsigned int sc_prot_alloc_en : 1;
        unsigned int sc_gid_byp : 2;
        unsigned int reserved : 28;
    } reg;
} SOC_DMSS_GLB_SC_CTRL_UNION;
#endif
#define SOC_DMSS_GLB_SC_CTRL_sc_en_START (0)
#define SOC_DMSS_GLB_SC_CTRL_sc_en_END (0)
#define SOC_DMSS_GLB_SC_CTRL_sc_prot_alloc_en_START (1)
#define SOC_DMSS_GLB_SC_CTRL_sc_prot_alloc_en_END (1)
#define SOC_DMSS_GLB_SC_CTRL_sc_gid_byp_START (2)
#define SOC_DMSS_GLB_SC_CTRL_sc_gid_byp_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_dcq_lvl0 : 8;
        unsigned int sc_dcq_lvl1 : 8;
        unsigned int reserved : 16;
    } reg;
} SOC_DMSS_GLB_SC_DCQLVL_UNION;
#endif
#define SOC_DMSS_GLB_SC_DCQLVL_sc_dcq_lvl0_START (0)
#define SOC_DMSS_GLB_SC_DCQLVL_sc_dcq_lvl0_END (7)
#define SOC_DMSS_GLB_SC_DCQLVL_sc_dcq_lvl1_START (8)
#define SOC_DMSS_GLB_SC_DCQLVL_sc_dcq_lvl1_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mst_flux_req_rd_pri : 3;
        unsigned int reserved_0 : 1;
        unsigned int mst_flux_req_wr_pri : 3;
        unsigned int reserved_1 : 1;
        unsigned int mst_flux_req_sel : 4;
        unsigned int mst_flux_req_en : 1;
        unsigned int reserved_2 : 3;
        unsigned int mst_flux_ctrl_rd_pri : 3;
        unsigned int reserved_3 : 1;
        unsigned int mst_flux_ctrl_wr_pri : 3;
        unsigned int reserved_4 : 1;
        unsigned int mst_flux_ctrl_sel : 4;
        unsigned int mst_flux_ctrl_en : 1;
        unsigned int reserved_5 : 3;
    } reg;
} SOC_DMSS_GLB_MST_FLUX_UNION;
#endif
#define SOC_DMSS_GLB_MST_FLUX_mst_flux_req_rd_pri_START (0)
#define SOC_DMSS_GLB_MST_FLUX_mst_flux_req_rd_pri_END (2)
#define SOC_DMSS_GLB_MST_FLUX_mst_flux_req_wr_pri_START (4)
#define SOC_DMSS_GLB_MST_FLUX_mst_flux_req_wr_pri_END (6)
#define SOC_DMSS_GLB_MST_FLUX_mst_flux_req_sel_START (8)
#define SOC_DMSS_GLB_MST_FLUX_mst_flux_req_sel_END (11)
#define SOC_DMSS_GLB_MST_FLUX_mst_flux_req_en_START (12)
#define SOC_DMSS_GLB_MST_FLUX_mst_flux_req_en_END (12)
#define SOC_DMSS_GLB_MST_FLUX_mst_flux_ctrl_rd_pri_START (16)
#define SOC_DMSS_GLB_MST_FLUX_mst_flux_ctrl_rd_pri_END (18)
#define SOC_DMSS_GLB_MST_FLUX_mst_flux_ctrl_wr_pri_START (20)
#define SOC_DMSS_GLB_MST_FLUX_mst_flux_ctrl_wr_pri_END (22)
#define SOC_DMSS_GLB_MST_FLUX_mst_flux_ctrl_sel_START (24)
#define SOC_DMSS_GLB_MST_FLUX_mst_flux_ctrl_sel_END (27)
#define SOC_DMSS_GLB_MST_FLUX_mst_flux_ctrl_en_START (28)
#define SOC_DMSS_GLB_MST_FLUX_mst_flux_ctrl_en_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int trace_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_GLB_TRACE_CTRL0_UNION;
#endif
#define SOC_DMSS_GLB_TRACE_CTRL0_trace_en_START (0)
#define SOC_DMSS_GLB_TRACE_CTRL0_trace_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int max_pattern_num : 28;
        unsigned int unaligned_mode : 1;
        unsigned int reserved : 2;
        unsigned int roll_en : 1;
    } reg;
} SOC_DMSS_GLB_TRACE_CTRL1_UNION;
#endif
#define SOC_DMSS_GLB_TRACE_CTRL1_max_pattern_num_START (0)
#define SOC_DMSS_GLB_TRACE_CTRL1_max_pattern_num_END (27)
#define SOC_DMSS_GLB_TRACE_CTRL1_unaligned_mode_START (28)
#define SOC_DMSS_GLB_TRACE_CTRL1_unaligned_mode_END (28)
#define SOC_DMSS_GLB_TRACE_CTRL1_roll_en_START (31)
#define SOC_DMSS_GLB_TRACE_CTRL1_roll_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int trace_prd : 32;
    } reg;
} SOC_DMSS_GLB_TRACE_CTRL2_UNION;
#endif
#define SOC_DMSS_GLB_TRACE_CTRL2_trace_prd_START (0)
#define SOC_DMSS_GLB_TRACE_CTRL2_trace_prd_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rec_base_addr : 16;
        unsigned int rec_top_addr : 16;
    } reg;
} SOC_DMSS_GLB_TRACE_REC0_UNION;
#endif
#define SOC_DMSS_GLB_TRACE_REC0_rec_base_addr_START (0)
#define SOC_DMSS_GLB_TRACE_REC0_rec_base_addr_END (15)
#define SOC_DMSS_GLB_TRACE_REC0_rec_top_addr_START (16)
#define SOC_DMSS_GLB_TRACE_REC0_rec_top_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rec_mid : 8;
        unsigned int rec_pri : 3;
        unsigned int reserved_0 : 5;
        unsigned int rec_ts_max_intrvl : 4;
        unsigned int rec_intlv_gran : 3;
        unsigned int reserved_1 : 9;
    } reg;
} SOC_DMSS_GLB_TRACE_REC1_UNION;
#endif
#define SOC_DMSS_GLB_TRACE_REC1_rec_mid_START (0)
#define SOC_DMSS_GLB_TRACE_REC1_rec_mid_END (7)
#define SOC_DMSS_GLB_TRACE_REC1_rec_pri_START (8)
#define SOC_DMSS_GLB_TRACE_REC1_rec_pri_END (10)
#define SOC_DMSS_GLB_TRACE_REC1_rec_ts_max_intrvl_START (16)
#define SOC_DMSS_GLB_TRACE_REC1_rec_ts_max_intrvl_END (19)
#define SOC_DMSS_GLB_TRACE_REC1_rec_intlv_gran_START (20)
#define SOC_DMSS_GLB_TRACE_REC1_rec_intlv_gran_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int filter_base_addr : 16;
        unsigned int filter_top_addr : 16;
    } reg;
} SOC_DMSS_GLB_TRACE_FILTER0_UNION;
#endif
#define SOC_DMSS_GLB_TRACE_FILTER0_filter_base_addr_START (0)
#define SOC_DMSS_GLB_TRACE_FILTER0_filter_base_addr_END (15)
#define SOC_DMSS_GLB_TRACE_FILTER0_filter_top_addr_START (16)
#define SOC_DMSS_GLB_TRACE_FILTER0_filter_top_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int filter_asi : 14;
        unsigned int filter_type_rd : 1;
        unsigned int filter_type_wr : 1;
        unsigned int filter_ch : 4;
        unsigned int filter_src : 1;
        unsigned int reserved : 11;
    } reg;
} SOC_DMSS_GLB_TRACE_FILTER1_UNION;
#endif
#define SOC_DMSS_GLB_TRACE_FILTER1_filter_asi_START (0)
#define SOC_DMSS_GLB_TRACE_FILTER1_filter_asi_END (13)
#define SOC_DMSS_GLB_TRACE_FILTER1_filter_type_rd_START (14)
#define SOC_DMSS_GLB_TRACE_FILTER1_filter_type_rd_END (14)
#define SOC_DMSS_GLB_TRACE_FILTER1_filter_type_wr_START (15)
#define SOC_DMSS_GLB_TRACE_FILTER1_filter_type_wr_END (15)
#define SOC_DMSS_GLB_TRACE_FILTER1_filter_ch_START (16)
#define SOC_DMSS_GLB_TRACE_FILTER1_filter_ch_END (19)
#define SOC_DMSS_GLB_TRACE_FILTER1_filter_src_START (20)
#define SOC_DMSS_GLB_TRACE_FILTER1_filter_src_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int filter_mid0 : 32;
    } reg;
} SOC_DMSS_GLB_TRACE_FILTER2_UNION;
#endif
#define SOC_DMSS_GLB_TRACE_FILTER2_filter_mid0_START (0)
#define SOC_DMSS_GLB_TRACE_FILTER2_filter_mid0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cur_freq : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_DMSS_GLB_TRACE_FREQ_UNION;
#endif
#define SOC_DMSS_GLB_TRACE_FREQ_cur_freq_START (0)
#define SOC_DMSS_GLB_TRACE_FREQ_cur_freq_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int trace_int_en : 1;
        unsigned int reserved : 15;
        unsigned int int_trigger : 16;
    } reg;
} SOC_DMSS_GLB_TRACE_INT_EN_UNION;
#endif
#define SOC_DMSS_GLB_TRACE_INT_EN_trace_int_en_START (0)
#define SOC_DMSS_GLB_TRACE_INT_EN_trace_int_en_END (0)
#define SOC_DMSS_GLB_TRACE_INT_EN_int_trigger_START (16)
#define SOC_DMSS_GLB_TRACE_INT_EN_int_trigger_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int trace_lock_down : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_GLB_TRACE_LOCK_DOWN_UNION;
#endif
#define SOC_DMSS_GLB_TRACE_LOCK_DOWN_trace_lock_down_START (0)
#define SOC_DMSS_GLB_TRACE_LOCK_DOWN_trace_lock_down_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_cur_addr : 31;
        unsigned int stat_cur_addr_ch : 1;
    } reg;
} SOC_DMSS_GLB_TRACE_STAT0_UNION;
#endif
#define SOC_DMSS_GLB_TRACE_STAT0_stat_cur_addr_START (0)
#define SOC_DMSS_GLB_TRACE_STAT0_stat_cur_addr_END (30)
#define SOC_DMSS_GLB_TRACE_STAT0_stat_cur_addr_ch_START (31)
#define SOC_DMSS_GLB_TRACE_STAT0_stat_cur_addr_ch_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_pattern_cnt : 32;
    } reg;
} SOC_DMSS_GLB_TRACE_STAT1_UNION;
#endif
#define SOC_DMSS_GLB_TRACE_STAT1_stat_pattern_cnt_START (0)
#define SOC_DMSS_GLB_TRACE_STAT1_stat_pattern_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_roll_cnt : 12;
        unsigned int reserved : 20;
    } reg;
} SOC_DMSS_GLB_TRACE_STAT2_UNION;
#endif
#define SOC_DMSS_GLB_TRACE_STAT2_stat_roll_cnt_START (0)
#define SOC_DMSS_GLB_TRACE_STAT2_stat_roll_cnt_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mpu_bypass : 1;
        unsigned int reserved_0 : 3;
        unsigned int mpu_addr_zone : 2;
        unsigned int reserved_1 : 2;
        unsigned int mpu_apb_sel : 2;
        unsigned int reserved_2 : 2;
        unsigned int mpu_apb_be : 1;
        unsigned int reserved_3 : 3;
        unsigned int mpu_fix_allow : 1;
        unsigned int reserved_4 : 15;
    } reg;
} SOC_DMSS_GLB_MPU_CFG_UNION;
#endif
#define SOC_DMSS_GLB_MPU_CFG_mpu_bypass_START (0)
#define SOC_DMSS_GLB_MPU_CFG_mpu_bypass_END (0)
#define SOC_DMSS_GLB_MPU_CFG_mpu_addr_zone_START (4)
#define SOC_DMSS_GLB_MPU_CFG_mpu_addr_zone_END (5)
#define SOC_DMSS_GLB_MPU_CFG_mpu_apb_sel_START (8)
#define SOC_DMSS_GLB_MPU_CFG_mpu_apb_sel_END (9)
#define SOC_DMSS_GLB_MPU_CFG_mpu_apb_be_START (12)
#define SOC_DMSS_GLB_MPU_CFG_mpu_apb_be_END (12)
#define SOC_DMSS_GLB_MPU_CFG_mpu_fix_allow_START (16)
#define SOC_DMSS_GLB_MPU_CFG_mpu_fix_allow_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mpu_err_addr : 32;
    } reg;
} SOC_DMSS_GLB_MPU_ERROR_ADDR_UNION;
#endif
#define SOC_DMSS_GLB_MPU_ERROR_ADDR_mpu_err_addr_START (0)
#define SOC_DMSS_GLB_MPU_ERROR_ADDR_mpu_err_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pmm_el2 : 1;
        unsigned int reserved_0 : 3;
        unsigned int mpu_acpu_wr_ntrst : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_DMSS_GLB_MPU_ACPU_CFG_UNION;
#endif
#define SOC_DMSS_GLB_MPU_ACPU_CFG_pmm_el2_START (0)
#define SOC_DMSS_GLB_MPU_ACPU_CFG_pmm_el2_END (0)
#define SOC_DMSS_GLB_MPU_ACPU_CFG_mpu_acpu_wr_ntrst_START (4)
#define SOC_DMSS_GLB_MPU_ACPU_CFG_mpu_acpu_wr_ntrst_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sec_rgn_byp : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_GLB_SEC_BYPASS_UNION;
#endif
#define SOC_DMSS_GLB_SEC_BYPASS_sec_rgn_byp_START (0)
#define SOC_DMSS_GLB_SEC_BYPASS_sec_rgn_byp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int asi_stat_en : 16;
        unsigned int pri_allow_stat_en : 1;
        unsigned int reserved_0 : 3;
        unsigned int rbuf_stat_en : 1;
        unsigned int reserved_1 : 3;
        unsigned int ami_wfifo_stat_en : 1;
        unsigned int reserved_2 : 7;
    } reg;
} SOC_DMSS_GLB_STAT_CTRL_UNION;
#endif
#define SOC_DMSS_GLB_STAT_CTRL_asi_stat_en_START (0)
#define SOC_DMSS_GLB_STAT_CTRL_asi_stat_en_END (15)
#define SOC_DMSS_GLB_STAT_CTRL_pri_allow_stat_en_START (16)
#define SOC_DMSS_GLB_STAT_CTRL_pri_allow_stat_en_END (16)
#define SOC_DMSS_GLB_STAT_CTRL_rbuf_stat_en_START (20)
#define SOC_DMSS_GLB_STAT_CTRL_rbuf_stat_en_END (20)
#define SOC_DMSS_GLB_STAT_CTRL_ami_wfifo_stat_en_START (24)
#define SOC_DMSS_GLB_STAT_CTRL_ami_wfifo_stat_en_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_cycle : 32;
    } reg;
} SOC_DMSS_GLB_STAT_CYCLE_UNION;
#endif
#define SOC_DMSS_GLB_STAT_CYCLE_stat_cycle_START (0)
#define SOC_DMSS_GLB_STAT_CYCLE_stat_cycle_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch0_wr_pri_allow_set : 3;
        unsigned int reserved_0 : 1;
        unsigned int ch1_wr_pri_allow_set : 3;
        unsigned int reserved_1 : 1;
        unsigned int ch2_wr_pri_allow_set : 3;
        unsigned int reserved_2 : 1;
        unsigned int ch3_wr_pri_allow_set : 3;
        unsigned int reserved_3 : 1;
        unsigned int ch0_rd_pri_allow_set : 3;
        unsigned int reserved_4 : 1;
        unsigned int ch1_rd_pri_allow_set : 3;
        unsigned int reserved_5 : 1;
        unsigned int ch2_rd_pri_allow_set : 3;
        unsigned int reserved_6 : 1;
        unsigned int ch3_rd_pri_allow_set : 3;
        unsigned int reserved_7 : 1;
    } reg;
} SOC_DMSS_GLB_STAT_PERFSTAT_UNION;
#endif
#define SOC_DMSS_GLB_STAT_PERFSTAT_ch0_wr_pri_allow_set_START (0)
#define SOC_DMSS_GLB_STAT_PERFSTAT_ch0_wr_pri_allow_set_END (2)
#define SOC_DMSS_GLB_STAT_PERFSTAT_ch1_wr_pri_allow_set_START (4)
#define SOC_DMSS_GLB_STAT_PERFSTAT_ch1_wr_pri_allow_set_END (6)
#define SOC_DMSS_GLB_STAT_PERFSTAT_ch2_wr_pri_allow_set_START (8)
#define SOC_DMSS_GLB_STAT_PERFSTAT_ch2_wr_pri_allow_set_END (10)
#define SOC_DMSS_GLB_STAT_PERFSTAT_ch3_wr_pri_allow_set_START (12)
#define SOC_DMSS_GLB_STAT_PERFSTAT_ch3_wr_pri_allow_set_END (14)
#define SOC_DMSS_GLB_STAT_PERFSTAT_ch0_rd_pri_allow_set_START (16)
#define SOC_DMSS_GLB_STAT_PERFSTAT_ch0_rd_pri_allow_set_END (18)
#define SOC_DMSS_GLB_STAT_PERFSTAT_ch1_rd_pri_allow_set_START (20)
#define SOC_DMSS_GLB_STAT_PERFSTAT_ch1_rd_pri_allow_set_END (22)
#define SOC_DMSS_GLB_STAT_PERFSTAT_ch2_rd_pri_allow_set_START (24)
#define SOC_DMSS_GLB_STAT_PERFSTAT_ch2_rd_pri_allow_set_END (26)
#define SOC_DMSS_GLB_STAT_PERFSTAT_ch3_rd_pri_allow_set_START (28)
#define SOC_DMSS_GLB_STAT_PERFSTAT_ch3_rd_pri_allow_set_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_dcq_wr_que_acc : 32;
    } reg;
} SOC_DMSS_GLB_STAT_DCQ_WR_UNION;
#endif
#define SOC_DMSS_GLB_STAT_DCQ_WR_stat_dcq_wr_que_acc_START (0)
#define SOC_DMSS_GLB_STAT_DCQ_WR_stat_dcq_wr_que_acc_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_dcq_que_acc : 32;
    } reg;
} SOC_DMSS_GLB_STAT_DCQ_UNION;
#endif
#define SOC_DMSS_GLB_STAT_DCQ_stat_dcq_que_acc_START (0)
#define SOC_DMSS_GLB_STAT_DCQ_stat_dcq_que_acc_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_wr_all_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_WR_ALL_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_WR_ALL_ALLOW_stat_wr_all_allow_START (0)
#define SOC_DMSS_GLB_STAT_WR_ALL_ALLOW_stat_wr_all_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_wr_pri1_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_WR_PRI1_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_WR_PRI1_ALLOW_stat_wr_pri1_allow_START (0)
#define SOC_DMSS_GLB_STAT_WR_PRI1_ALLOW_stat_wr_pri1_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_wr_pri2_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_WR_PRI2_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_WR_PRI2_ALLOW_stat_wr_pri2_allow_START (0)
#define SOC_DMSS_GLB_STAT_WR_PRI2_ALLOW_stat_wr_pri2_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_wr_pri3_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_WR_PRI3_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_WR_PRI3_ALLOW_stat_wr_pri3_allow_START (0)
#define SOC_DMSS_GLB_STAT_WR_PRI3_ALLOW_stat_wr_pri3_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_wr_pri4_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_WR_PRI4_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_WR_PRI4_ALLOW_stat_wr_pri4_allow_START (0)
#define SOC_DMSS_GLB_STAT_WR_PRI4_ALLOW_stat_wr_pri4_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_wr_pri5_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_WR_PRI5_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_WR_PRI5_ALLOW_stat_wr_pri5_allow_START (0)
#define SOC_DMSS_GLB_STAT_WR_PRI5_ALLOW_stat_wr_pri5_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_wr_pri6_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_WR_PRI6_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_WR_PRI6_ALLOW_stat_wr_pri6_allow_START (0)
#define SOC_DMSS_GLB_STAT_WR_PRI6_ALLOW_stat_wr_pri6_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_wr_pri7_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_WR_PRI7_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_WR_PRI7_ALLOW_stat_wr_pri7_allow_START (0)
#define SOC_DMSS_GLB_STAT_WR_PRI7_ALLOW_stat_wr_pri7_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_rd_all_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_RD_ALL_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_RD_ALL_ALLOW_stat_rd_all_allow_START (0)
#define SOC_DMSS_GLB_STAT_RD_ALL_ALLOW_stat_rd_all_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_rd_pri1_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_RD_PRI1_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_RD_PRI1_ALLOW_stat_rd_pri1_allow_START (0)
#define SOC_DMSS_GLB_STAT_RD_PRI1_ALLOW_stat_rd_pri1_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_rd_pri2_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_RD_PRI2_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_RD_PRI2_ALLOW_stat_rd_pri2_allow_START (0)
#define SOC_DMSS_GLB_STAT_RD_PRI2_ALLOW_stat_rd_pri2_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_rd_pri3_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_RD_PRI3_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_RD_PRI3_ALLOW_stat_rd_pri3_allow_START (0)
#define SOC_DMSS_GLB_STAT_RD_PRI3_ALLOW_stat_rd_pri3_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_rd_pri4_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_RD_PRI4_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_RD_PRI4_ALLOW_stat_rd_pri4_allow_START (0)
#define SOC_DMSS_GLB_STAT_RD_PRI4_ALLOW_stat_rd_pri4_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_rd_pri5_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_RD_PRI5_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_RD_PRI5_ALLOW_stat_rd_pri5_allow_START (0)
#define SOC_DMSS_GLB_STAT_RD_PRI5_ALLOW_stat_rd_pri5_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_rd_pri6_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_RD_PRI6_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_RD_PRI6_ALLOW_stat_rd_pri6_allow_START (0)
#define SOC_DMSS_GLB_STAT_RD_PRI6_ALLOW_stat_rd_pri6_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_rd_pri7_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_RD_PRI7_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_RD_PRI7_ALLOW_stat_rd_pri7_allow_START (0)
#define SOC_DMSS_GLB_STAT_RD_PRI7_ALLOW_stat_rd_pri7_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_buf_sel : 4;
        unsigned int stat_int_en : 1;
        unsigned int reserved_0 : 3;
        unsigned int stat_splitter0 : 8;
        unsigned int stat_splitter1 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_DMSS_GLB_STAT_RBUF_CTRL_UNION;
#endif
#define SOC_DMSS_GLB_STAT_RBUF_CTRL_stat_buf_sel_START (0)
#define SOC_DMSS_GLB_STAT_RBUF_CTRL_stat_buf_sel_END (3)
#define SOC_DMSS_GLB_STAT_RBUF_CTRL_stat_int_en_START (4)
#define SOC_DMSS_GLB_STAT_RBUF_CTRL_stat_int_en_END (4)
#define SOC_DMSS_GLB_STAT_RBUF_CTRL_stat_splitter0_START (8)
#define SOC_DMSS_GLB_STAT_RBUF_CTRL_stat_splitter0_END (15)
#define SOC_DMSS_GLB_STAT_RBUF_CTRL_stat_splitter1_START (16)
#define SOC_DMSS_GLB_STAT_RBUF_CTRL_stat_splitter1_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_rbuf_occup0 : 32;
    } reg;
} SOC_DMSS_GLB_STAT_RBUF_OCCUP0_UNION;
#endif
#define SOC_DMSS_GLB_STAT_RBUF_OCCUP0_stat_rbuf_occup0_START (0)
#define SOC_DMSS_GLB_STAT_RBUF_OCCUP0_stat_rbuf_occup0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_rbuf_occup1 : 32;
    } reg;
} SOC_DMSS_GLB_STAT_RBUF_OCCUP1_UNION;
#endif
#define SOC_DMSS_GLB_STAT_RBUF_OCCUP1_stat_rbuf_occup1_START (0)
#define SOC_DMSS_GLB_STAT_RBUF_OCCUP1_stat_rbuf_occup1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_rbuf_occup2 : 32;
    } reg;
} SOC_DMSS_GLB_STAT_RBUF_OCCUP2_UNION;
#endif
#define SOC_DMSS_GLB_STAT_RBUF_OCCUP2_stat_rbuf_occup2_START (0)
#define SOC_DMSS_GLB_STAT_RBUF_OCCUP2_stat_rbuf_occup2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_ami_wfifo_busy : 20;
        unsigned int stat_ami_wfifo_full : 12;
    } reg;
} SOC_DMSS_GLB_STAT_AMI_WFIFO_UNION;
#endif
#define SOC_DMSS_GLB_STAT_AMI_WFIFO_stat_ami_wfifo_busy_START (0)
#define SOC_DMSS_GLB_STAT_AMI_WFIFO_stat_ami_wfifo_busy_END (19)
#define SOC_DMSS_GLB_STAT_AMI_WFIFO_stat_ami_wfifo_full_START (20)
#define SOC_DMSS_GLB_STAT_AMI_WFIFO_stat_ami_wfifo_full_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int asi_busy : 14;
        unsigned int reserved_0 : 2;
        unsigned int dmi_busy : 2;
        unsigned int reserved_1 : 2;
        unsigned int ami_busy : 2;
        unsigned int reserved_2 : 2;
        unsigned int trace_busy : 2;
        unsigned int dmss_ddr_busy : 1;
        unsigned int ddr_wake_up : 1;
        unsigned int dmi_cmd_bp : 2;
        unsigned int reserved_3 : 2;
    } reg;
} SOC_DMSS_GLB_DFX_MODULE_UNION;
#endif
#define SOC_DMSS_GLB_DFX_MODULE_asi_busy_START (0)
#define SOC_DMSS_GLB_DFX_MODULE_asi_busy_END (13)
#define SOC_DMSS_GLB_DFX_MODULE_dmi_busy_START (16)
#define SOC_DMSS_GLB_DFX_MODULE_dmi_busy_END (17)
#define SOC_DMSS_GLB_DFX_MODULE_ami_busy_START (20)
#define SOC_DMSS_GLB_DFX_MODULE_ami_busy_END (21)
#define SOC_DMSS_GLB_DFX_MODULE_trace_busy_START (24)
#define SOC_DMSS_GLB_DFX_MODULE_trace_busy_END (25)
#define SOC_DMSS_GLB_DFX_MODULE_dmss_ddr_busy_START (26)
#define SOC_DMSS_GLB_DFX_MODULE_dmss_ddr_busy_END (26)
#define SOC_DMSS_GLB_DFX_MODULE_ddr_wake_up_START (27)
#define SOC_DMSS_GLB_DFX_MODULE_ddr_wake_up_END (27)
#define SOC_DMSS_GLB_DFX_MODULE_dmi_cmd_bp_START (28)
#define SOC_DMSS_GLB_DFX_MODULE_dmi_cmd_bp_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int trace0_wfifo_st : 2;
        unsigned int trace1_wfifo_st : 2;
        unsigned int reserved : 28;
    } reg;
} SOC_DMSS_GLB_DFX_FIFO_UNION;
#endif
#define SOC_DMSS_GLB_DFX_FIFO_trace0_wfifo_st_START (0)
#define SOC_DMSS_GLB_DFX_FIFO_trace0_wfifo_st_END (1)
#define SOC_DMSS_GLB_DFX_FIFO_trace1_wfifo_st_START (2)
#define SOC_DMSS_GLB_DFX_FIFO_trace1_wfifo_st_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int burstfifo_empty : 14;
        unsigned int reserved_0 : 2;
        unsigned int burstfifo_full : 14;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_DMSS_GLB_DFX_RBUF_FIFO_UNION;
#endif
#define SOC_DMSS_GLB_DFX_RBUF_FIFO_burstfifo_empty_START (0)
#define SOC_DMSS_GLB_DFX_RBUF_FIFO_burstfifo_empty_END (13)
#define SOC_DMSS_GLB_DFX_RBUF_FIFO_burstfifo_full_START (16)
#define SOC_DMSS_GLB_DFX_RBUF_FIFO_burstfifo_full_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dcq_wr_que_st : 8;
        unsigned int dcq_que_st : 8;
        unsigned int dcq_rd_pri_allow : 3;
        unsigned int reserved_0 : 1;
        unsigned int dcq_wr_pri_allow : 3;
        unsigned int reserved_1 : 1;
        unsigned int mst_rd_pri_allow : 3;
        unsigned int reserved_2 : 1;
        unsigned int mst_wr_pri_allow : 3;
        unsigned int reserved_3 : 1;
    } reg;
} SOC_DMSS_GLB_DFX_DCQ_ST_UNION;
#endif
#define SOC_DMSS_GLB_DFX_DCQ_ST_dcq_wr_que_st_START (0)
#define SOC_DMSS_GLB_DFX_DCQ_ST_dcq_wr_que_st_END (7)
#define SOC_DMSS_GLB_DFX_DCQ_ST_dcq_que_st_START (8)
#define SOC_DMSS_GLB_DFX_DCQ_ST_dcq_que_st_END (15)
#define SOC_DMSS_GLB_DFX_DCQ_ST_dcq_rd_pri_allow_START (16)
#define SOC_DMSS_GLB_DFX_DCQ_ST_dcq_rd_pri_allow_END (18)
#define SOC_DMSS_GLB_DFX_DCQ_ST_dcq_wr_pri_allow_START (20)
#define SOC_DMSS_GLB_DFX_DCQ_ST_dcq_wr_pri_allow_END (22)
#define SOC_DMSS_GLB_DFX_DCQ_ST_mst_rd_pri_allow_START (24)
#define SOC_DMSS_GLB_DFX_DCQ_ST_mst_rd_pri_allow_END (26)
#define SOC_DMSS_GLB_DFX_DCQ_ST_mst_wr_pri_allow_START (28)
#define SOC_DMSS_GLB_DFX_DCQ_ST_mst_wr_pri_allow_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vque_st : 32;
    } reg;
} SOC_DMSS_GLB_DFX_RBUF0_UNION;
#endif
#define SOC_DMSS_GLB_DFX_RBUF0_vque_st_START (0)
#define SOC_DMSS_GLB_DFX_RBUF0_vque_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vque_grp0_cnt_st : 8;
        unsigned int vque_grp1_cnt_st : 8;
        unsigned int vque_grp2_cnt_st : 8;
        unsigned int vque_grp3_cnt_st : 8;
    } reg;
} SOC_DMSS_GLB_DFX_RBUF1_UNION;
#endif
#define SOC_DMSS_GLB_DFX_RBUF1_vque_grp0_cnt_st_START (0)
#define SOC_DMSS_GLB_DFX_RBUF1_vque_grp0_cnt_st_END (7)
#define SOC_DMSS_GLB_DFX_RBUF1_vque_grp1_cnt_st_START (8)
#define SOC_DMSS_GLB_DFX_RBUF1_vque_grp1_cnt_st_END (15)
#define SOC_DMSS_GLB_DFX_RBUF1_vque_grp2_cnt_st_START (16)
#define SOC_DMSS_GLB_DFX_RBUF1_vque_grp2_cnt_st_END (23)
#define SOC_DMSS_GLB_DFX_RBUF1_vque_grp3_cnt_st_START (24)
#define SOC_DMSS_GLB_DFX_RBUF1_vque_grp3_cnt_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfx_dummy0 : 32;
    } reg;
} SOC_DMSS_GLB_DFX_DUMMY0_UNION;
#endif
#define SOC_DMSS_GLB_DFX_DUMMY0_dfx_dummy0_START (0)
#define SOC_DMSS_GLB_DFX_DUMMY0_dfx_dummy0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfx_dummy1 : 32;
    } reg;
} SOC_DMSS_GLB_DFX_DUMMY1_UNION;
#endif
#define SOC_DMSS_GLB_DFX_DUMMY1_dfx_dummy1_START (0)
#define SOC_DMSS_GLB_DFX_DUMMY1_dfx_dummy1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfx_dummy2 : 32;
    } reg;
} SOC_DMSS_GLB_DFX_DUMMY2_UNION;
#endif
#define SOC_DMSS_GLB_DFX_DUMMY2_dfx_dummy2_START (0)
#define SOC_DMSS_GLB_DFX_DUMMY2_dfx_dummy2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfx_dummy3 : 32;
    } reg;
} SOC_DMSS_GLB_DFX_DUMMY3_UNION;
#endif
#define SOC_DMSS_GLB_DFX_DUMMY3_dfx_dummy3_START (0)
#define SOC_DMSS_GLB_DFX_DUMMY3_dfx_dummy3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rbuf0_load_cnt_st : 8;
        unsigned int rbuf1_load_cnt_st : 8;
        unsigned int rbuf2_load_cnt_st : 8;
        unsigned int rbuf3_load_cnt_st : 8;
    } reg;
} SOC_DMSS_GLB_DFX_LOAD_UNION;
#endif
#define SOC_DMSS_GLB_DFX_LOAD_rbuf0_load_cnt_st_START (0)
#define SOC_DMSS_GLB_DFX_LOAD_rbuf0_load_cnt_st_END (7)
#define SOC_DMSS_GLB_DFX_LOAD_rbuf1_load_cnt_st_START (8)
#define SOC_DMSS_GLB_DFX_LOAD_rbuf1_load_cnt_st_END (15)
#define SOC_DMSS_GLB_DFX_LOAD_rbuf2_load_cnt_st_START (16)
#define SOC_DMSS_GLB_DFX_LOAD_rbuf2_load_cnt_st_END (23)
#define SOC_DMSS_GLB_DFX_LOAD_rbuf3_load_cnt_st_START (24)
#define SOC_DMSS_GLB_DFX_LOAD_rbuf3_load_cnt_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mpu_addr_attr : 32;
    } reg;
} SOC_DMSS_MPU_ADDR_ATTR_UNION;
#endif
#define SOC_DMSS_MPU_ADDR_ATTR_mpu_addr_attr_START (0)
#define SOC_DMSS_MPU_ADDR_ATTR_mpu_addr_attr_END (31)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
