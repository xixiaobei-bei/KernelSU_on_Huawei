#ifndef __SOC_MEDIA2_CRG_INTERFACE_H__
#define __SOC_MEDIA2_CRG_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_MEDIA2_CRG_PEREN0_ADDR(base) ((base) + (0x000))
#define SOC_MEDIA2_CRG_PERDIS0_ADDR(base) ((base) + (0x004))
#define SOC_MEDIA2_CRG_PERCLKEN0_ADDR(base) ((base) + (0x008))
#define SOC_MEDIA2_CRG_PERSTAT0_ADDR(base) ((base) + (0x00C))
#define SOC_MEDIA2_CRG_PERRSTEN0_ADDR(base) ((base) + (0x030))
#define SOC_MEDIA2_CRG_PERRSTDIS0_ADDR(base) ((base) + (0x034))
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ADDR(base) ((base) + (0x038))
#define SOC_MEDIA2_CRG_PERI_AUTODIV0_ADDR(base) ((base) + (0x160))
#define SOC_MEDIA2_CRG_PERI_AUTODIV1_ADDR(base) ((base) + (0x164))
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_ADDR(base) ((base) + (0x168))
#define SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_ADDR(base) ((base) + (0x1D0))
#define SOC_MEDIA2_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_ADDR(base) ((base) + (0x1D4))
#define SOC_MEDIA2_CRG_IPCLKRST_BYPASS_ADDR(base) ((base) + (0x1D8))
#define SOC_MEDIA2_CRG_DMSS_ASYNC_BRG_ADDR(base) ((base) + (0x1DC))
#define SOC_MEDIA2_CRG_DECPROT0SET_ADDR(base) ((base) + (0xF00))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_noc_ics_cfg : 1;
        unsigned int gt_clk_noc_ics : 1;
        unsigned int gt_clk_ics : 1;
        unsigned int gt_pclk_venc : 1;
        unsigned int gt_aclk_venc : 1;
        unsigned int gt_clk_venc : 1;
        unsigned int gt_pclk_vdec : 1;
        unsigned int gt_aclk_vdec : 1;
        unsigned int gt_clk_vdec : 1;
        unsigned int gt_clk_vcodecbus : 1;
        unsigned int gt_pclk_atgs_vcodec : 1;
        unsigned int gt_pclk_autodiv_vcodecbus : 1;
        unsigned int gt_pclk_autodiv_vdec : 1;
        unsigned int gt_pclk_autodiv_venc : 1;
        unsigned int gt_pclk_atgm_vdec : 1;
        unsigned int gt_clk_dmss_vcodec : 1;
        unsigned int gt_clk_loadmonitor : 1;
        unsigned int gt_pclk_media2_lm : 1;
        unsigned int gt_pclk_media2_lm_lpm3 : 1;
        unsigned int gt_clk_noc_ics2_cfg : 1;
        unsigned int gt_clk_noc_ics2 : 1;
        unsigned int reserved : 11;
    } reg;
} SOC_MEDIA2_CRG_PEREN0_UNION;
#endif
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_noc_ics_cfg_START (0)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_noc_ics_cfg_END (0)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_noc_ics_START (1)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_noc_ics_END (1)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_ics_START (2)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_ics_END (2)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_venc_START (3)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_venc_END (3)
#define SOC_MEDIA2_CRG_PEREN0_gt_aclk_venc_START (4)
#define SOC_MEDIA2_CRG_PEREN0_gt_aclk_venc_END (4)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_venc_START (5)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_venc_END (5)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_vdec_START (6)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_vdec_END (6)
#define SOC_MEDIA2_CRG_PEREN0_gt_aclk_vdec_START (7)
#define SOC_MEDIA2_CRG_PEREN0_gt_aclk_vdec_END (7)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_vdec_START (8)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_vdec_END (8)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_vcodecbus_START (9)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_vcodecbus_END (9)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_atgs_vcodec_START (10)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_atgs_vcodec_END (10)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_autodiv_vcodecbus_START (11)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_autodiv_vcodecbus_END (11)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_autodiv_vdec_START (12)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_autodiv_vdec_END (12)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_autodiv_venc_START (13)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_autodiv_venc_END (13)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_atgm_vdec_START (14)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_atgm_vdec_END (14)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_dmss_vcodec_START (15)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_dmss_vcodec_END (15)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_loadmonitor_START (16)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_loadmonitor_END (16)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_media2_lm_START (17)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_media2_lm_END (17)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_media2_lm_lpm3_START (18)
#define SOC_MEDIA2_CRG_PEREN0_gt_pclk_media2_lm_lpm3_END (18)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_noc_ics2_cfg_START (19)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_noc_ics2_cfg_END (19)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_noc_ics2_START (20)
#define SOC_MEDIA2_CRG_PEREN0_gt_clk_noc_ics2_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_noc_ics_cfg : 1;
        unsigned int gt_clk_noc_ics : 1;
        unsigned int gt_clk_ics : 1;
        unsigned int gt_pclk_venc : 1;
        unsigned int gt_aclk_venc : 1;
        unsigned int gt_clk_venc : 1;
        unsigned int gt_pclk_vdec : 1;
        unsigned int gt_aclk_vdec : 1;
        unsigned int gt_clk_vdec : 1;
        unsigned int gt_clk_vcodecbus : 1;
        unsigned int gt_pclk_atgs_vcodec : 1;
        unsigned int gt_pclk_autodiv_vcodecbus : 1;
        unsigned int gt_pclk_autodiv_vdec : 1;
        unsigned int gt_pclk_autodiv_venc : 1;
        unsigned int gt_pclk_atgm_vdec : 1;
        unsigned int gt_clk_dmss_vcodec : 1;
        unsigned int gt_clk_loadmonitor : 1;
        unsigned int gt_pclk_media2_lm : 1;
        unsigned int gt_pclk_media2_lm_lpm3 : 1;
        unsigned int gt_clk_noc_ics2_cfg : 1;
        unsigned int gt_clk_noc_ics2 : 1;
        unsigned int reserved : 11;
    } reg;
} SOC_MEDIA2_CRG_PERDIS0_UNION;
#endif
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_noc_ics_cfg_START (0)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_noc_ics_cfg_END (0)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_noc_ics_START (1)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_noc_ics_END (1)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_ics_START (2)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_ics_END (2)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_venc_START (3)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_venc_END (3)
#define SOC_MEDIA2_CRG_PERDIS0_gt_aclk_venc_START (4)
#define SOC_MEDIA2_CRG_PERDIS0_gt_aclk_venc_END (4)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_venc_START (5)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_venc_END (5)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_vdec_START (6)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_vdec_END (6)
#define SOC_MEDIA2_CRG_PERDIS0_gt_aclk_vdec_START (7)
#define SOC_MEDIA2_CRG_PERDIS0_gt_aclk_vdec_END (7)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_vdec_START (8)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_vdec_END (8)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_vcodecbus_START (9)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_vcodecbus_END (9)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_atgs_vcodec_START (10)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_atgs_vcodec_END (10)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_autodiv_vcodecbus_START (11)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_autodiv_vcodecbus_END (11)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_autodiv_vdec_START (12)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_autodiv_vdec_END (12)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_autodiv_venc_START (13)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_autodiv_venc_END (13)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_atgm_vdec_START (14)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_atgm_vdec_END (14)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_dmss_vcodec_START (15)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_dmss_vcodec_END (15)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_loadmonitor_START (16)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_loadmonitor_END (16)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_media2_lm_START (17)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_media2_lm_END (17)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_media2_lm_lpm3_START (18)
#define SOC_MEDIA2_CRG_PERDIS0_gt_pclk_media2_lm_lpm3_END (18)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_noc_ics2_cfg_START (19)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_noc_ics2_cfg_END (19)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_noc_ics2_START (20)
#define SOC_MEDIA2_CRG_PERDIS0_gt_clk_noc_ics2_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_clk_noc_ics_cfg : 1;
        unsigned int gt_clk_noc_ics : 1;
        unsigned int gt_clk_ics : 1;
        unsigned int gt_pclk_venc : 1;
        unsigned int gt_aclk_venc : 1;
        unsigned int gt_clk_venc : 1;
        unsigned int gt_pclk_vdec : 1;
        unsigned int gt_aclk_vdec : 1;
        unsigned int gt_clk_vdec : 1;
        unsigned int gt_clk_vcodecbus : 1;
        unsigned int gt_pclk_atgs_vcodec : 1;
        unsigned int gt_pclk_autodiv_vcodecbus : 1;
        unsigned int gt_pclk_autodiv_vdec : 1;
        unsigned int gt_pclk_autodiv_venc : 1;
        unsigned int gt_pclk_atgm_vdec : 1;
        unsigned int gt_clk_dmss_vcodec : 1;
        unsigned int gt_clk_loadmonitor : 1;
        unsigned int gt_pclk_media2_lm : 1;
        unsigned int gt_pclk_media2_lm_lpm3 : 1;
        unsigned int gt_clk_noc_ics2_cfg : 1;
        unsigned int gt_clk_noc_ics2 : 1;
        unsigned int reserved : 11;
    } reg;
} SOC_MEDIA2_CRG_PERCLKEN0_UNION;
#endif
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_noc_ics_cfg_START (0)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_noc_ics_cfg_END (0)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_noc_ics_START (1)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_noc_ics_END (1)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_ics_START (2)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_ics_END (2)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_venc_START (3)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_venc_END (3)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_aclk_venc_START (4)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_aclk_venc_END (4)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_venc_START (5)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_venc_END (5)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_vdec_START (6)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_vdec_END (6)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_aclk_vdec_START (7)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_aclk_vdec_END (7)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_vdec_START (8)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_vdec_END (8)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_vcodecbus_START (9)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_vcodecbus_END (9)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_atgs_vcodec_START (10)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_atgs_vcodec_END (10)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_autodiv_vcodecbus_START (11)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_autodiv_vcodecbus_END (11)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_autodiv_vdec_START (12)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_autodiv_vdec_END (12)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_autodiv_venc_START (13)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_autodiv_venc_END (13)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_atgm_vdec_START (14)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_atgm_vdec_END (14)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_dmss_vcodec_START (15)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_dmss_vcodec_END (15)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_loadmonitor_START (16)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_loadmonitor_END (16)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_media2_lm_START (17)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_media2_lm_END (17)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_media2_lm_lpm3_START (18)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_pclk_media2_lm_lpm3_END (18)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_noc_ics2_cfg_START (19)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_noc_ics2_cfg_END (19)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_noc_ics2_START (20)
#define SOC_MEDIA2_CRG_PERCLKEN0_gt_clk_noc_ics2_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int st_clk_noc_ics_cfg : 1;
        unsigned int st_clk_noc_ics : 1;
        unsigned int st_clk_ics : 1;
        unsigned int st_pclk_venc : 1;
        unsigned int st_aclk_venc : 1;
        unsigned int st_clk_venc : 1;
        unsigned int st_pclk_vdec : 1;
        unsigned int st_aclk_vdec : 1;
        unsigned int st_clk_vdec : 1;
        unsigned int st_clk_vcodecbus : 1;
        unsigned int st_pclk_atgs_vcodec : 1;
        unsigned int st_pclk_autodiv_vcodecbus : 1;
        unsigned int st_pclk_autodiv_vdec : 1;
        unsigned int st_pclk_autodiv_venc : 1;
        unsigned int st_pclk_atgm_vdec : 1;
        unsigned int st_clk_dmss_vcodec : 1;
        unsigned int st_clk_loadmonitor : 1;
        unsigned int st_pclk_media2_lm : 1;
        unsigned int st_pclk_media2_lm_lpm3 : 1;
        unsigned int st_clk_noc_ics2_cfg : 1;
        unsigned int st_clk_noc_ics2 : 1;
        unsigned int reserved : 11;
    } reg;
} SOC_MEDIA2_CRG_PERSTAT0_UNION;
#endif
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_noc_ics_cfg_START (0)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_noc_ics_cfg_END (0)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_noc_ics_START (1)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_noc_ics_END (1)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_ics_START (2)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_ics_END (2)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_venc_START (3)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_venc_END (3)
#define SOC_MEDIA2_CRG_PERSTAT0_st_aclk_venc_START (4)
#define SOC_MEDIA2_CRG_PERSTAT0_st_aclk_venc_END (4)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_venc_START (5)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_venc_END (5)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_vdec_START (6)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_vdec_END (6)
#define SOC_MEDIA2_CRG_PERSTAT0_st_aclk_vdec_START (7)
#define SOC_MEDIA2_CRG_PERSTAT0_st_aclk_vdec_END (7)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_vdec_START (8)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_vdec_END (8)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_vcodecbus_START (9)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_vcodecbus_END (9)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_atgs_vcodec_START (10)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_atgs_vcodec_END (10)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_autodiv_vcodecbus_START (11)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_autodiv_vcodecbus_END (11)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_autodiv_vdec_START (12)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_autodiv_vdec_END (12)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_autodiv_venc_START (13)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_autodiv_venc_END (13)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_atgm_vdec_START (14)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_atgm_vdec_END (14)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_dmss_vcodec_START (15)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_dmss_vcodec_END (15)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_loadmonitor_START (16)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_loadmonitor_END (16)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_media2_lm_START (17)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_media2_lm_END (17)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_media2_lm_lpm3_START (18)
#define SOC_MEDIA2_CRG_PERSTAT0_st_pclk_media2_lm_lpm3_END (18)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_noc_ics2_cfg_START (19)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_noc_ics2_cfg_END (19)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_noc_ics2_START (20)
#define SOC_MEDIA2_CRG_PERSTAT0_st_clk_noc_ics2_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_venc : 1;
        unsigned int ip_arst_venc : 1;
        unsigned int ip_prst_venc : 1;
        unsigned int ip_rst_ics : 1;
        unsigned int ip_rst_noc_ics : 1;
        unsigned int ip_rst_noc_ics_cfg : 1;
        unsigned int ip_rst_vdec : 1;
        unsigned int ip_prst_media2_lm : 1;
        unsigned int ip_prst_atgs_vcodec : 1;
        unsigned int ip_prst_atgm_vdec : 1;
        unsigned int ip_rst_noc_ics2_cfg : 1;
        unsigned int ip_rst_noc_ics2 : 1;
        unsigned int ip_rst_ics_core : 1;
        unsigned int reserved : 19;
    } reg;
} SOC_MEDIA2_CRG_PERRSTEN0_UNION;
#endif
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_rst_venc_START (0)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_rst_venc_END (0)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_arst_venc_START (1)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_arst_venc_END (1)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_prst_venc_START (2)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_prst_venc_END (2)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_rst_ics_START (3)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_rst_ics_END (3)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_rst_noc_ics_START (4)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_rst_noc_ics_END (4)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_rst_noc_ics_cfg_START (5)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_rst_noc_ics_cfg_END (5)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_rst_vdec_START (6)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_rst_vdec_END (6)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_prst_media2_lm_START (7)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_prst_media2_lm_END (7)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_prst_atgs_vcodec_START (8)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_prst_atgs_vcodec_END (8)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_prst_atgm_vdec_START (9)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_prst_atgm_vdec_END (9)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_rst_noc_ics2_cfg_START (10)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_rst_noc_ics2_cfg_END (10)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_rst_noc_ics2_START (11)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_rst_noc_ics2_END (11)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_rst_ics_core_START (12)
#define SOC_MEDIA2_CRG_PERRSTEN0_ip_rst_ics_core_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_venc : 1;
        unsigned int ip_arst_venc : 1;
        unsigned int ip_prst_venc : 1;
        unsigned int ip_rst_ics : 1;
        unsigned int ip_rst_noc_ics : 1;
        unsigned int ip_rst_noc_ics_cfg : 1;
        unsigned int ip_rst_vdec : 1;
        unsigned int ip_prst_media2_lm : 1;
        unsigned int ip_prst_atgs_vcodec : 1;
        unsigned int ip_prst_atgm_vdec : 1;
        unsigned int ip_rst_noc_ics2_cfg : 1;
        unsigned int ip_rst_noc_ics2 : 1;
        unsigned int ip_rst_ics_core : 1;
        unsigned int reserved : 19;
    } reg;
} SOC_MEDIA2_CRG_PERRSTDIS0_UNION;
#endif
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_rst_venc_START (0)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_rst_venc_END (0)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_arst_venc_START (1)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_arst_venc_END (1)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_prst_venc_START (2)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_prst_venc_END (2)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_rst_ics_START (3)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_rst_ics_END (3)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_rst_noc_ics_START (4)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_rst_noc_ics_END (4)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_rst_noc_ics_cfg_START (5)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_rst_noc_ics_cfg_END (5)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_rst_vdec_START (6)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_rst_vdec_END (6)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_prst_media2_lm_START (7)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_prst_media2_lm_END (7)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_prst_atgs_vcodec_START (8)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_prst_atgs_vcodec_END (8)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_prst_atgm_vdec_START (9)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_prst_atgm_vdec_END (9)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_rst_noc_ics2_cfg_START (10)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_rst_noc_ics2_cfg_END (10)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_rst_noc_ics2_START (11)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_rst_noc_ics2_END (11)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_rst_ics_core_START (12)
#define SOC_MEDIA2_CRG_PERRSTDIS0_ip_rst_ics_core_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ip_rst_venc : 1;
        unsigned int ip_arst_venc : 1;
        unsigned int ip_prst_venc : 1;
        unsigned int ip_rst_ics : 1;
        unsigned int ip_rst_noc_ics : 1;
        unsigned int ip_rst_noc_ics_cfg : 1;
        unsigned int ip_rst_vdec : 1;
        unsigned int ip_prst_media2_lm : 1;
        unsigned int ip_prst_atgs_vcodec : 1;
        unsigned int ip_prst_atgm_vdec : 1;
        unsigned int ip_rst_noc_ics2_cfg : 1;
        unsigned int ip_rst_noc_ics2 : 1;
        unsigned int ip_rst_ics_core : 1;
        unsigned int reserved : 19;
    } reg;
} SOC_MEDIA2_CRG_PERRSTSTAT0_UNION;
#endif
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_rst_venc_START (0)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_rst_venc_END (0)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_arst_venc_START (1)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_arst_venc_END (1)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_prst_venc_START (2)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_prst_venc_END (2)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_rst_ics_START (3)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_rst_ics_END (3)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_rst_noc_ics_START (4)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_rst_noc_ics_END (4)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_rst_noc_ics_cfg_START (5)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_rst_noc_ics_cfg_END (5)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_rst_vdec_START (6)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_rst_vdec_END (6)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_prst_media2_lm_START (7)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_prst_media2_lm_END (7)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_prst_atgs_vcodec_START (8)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_prst_atgs_vcodec_END (8)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_prst_atgm_vdec_START (9)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_prst_atgm_vdec_END (9)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_rst_noc_ics2_cfg_START (10)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_rst_noc_ics2_cfg_END (10)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_rst_noc_ics2_START (11)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_rst_noc_ics2_END (11)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_rst_ics_core_START (12)
#define SOC_MEDIA2_CRG_PERRSTSTAT0_ip_rst_ics_core_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int venc_auto_waitcfg_in : 10;
        unsigned int venc_auto_waitcfg_out : 10;
        unsigned int reserved_1 : 6;
        unsigned int venc_vcodecbus_cfg_bypass : 1;
        unsigned int reserved_2 : 4;
    } reg;
} SOC_MEDIA2_CRG_PERI_AUTODIV0_UNION;
#endif
#define SOC_MEDIA2_CRG_PERI_AUTODIV0_venc_auto_waitcfg_in_START (1)
#define SOC_MEDIA2_CRG_PERI_AUTODIV0_venc_auto_waitcfg_in_END (10)
#define SOC_MEDIA2_CRG_PERI_AUTODIV0_venc_auto_waitcfg_out_START (11)
#define SOC_MEDIA2_CRG_PERI_AUTODIV0_venc_auto_waitcfg_out_END (20)
#define SOC_MEDIA2_CRG_PERI_AUTODIV0_venc_vcodecbus_cfg_bypass_START (27)
#define SOC_MEDIA2_CRG_PERI_AUTODIV0_venc_vcodecbus_cfg_bypass_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int vdec_auto_waitcfg_in : 10;
        unsigned int vdec_auto_waitcfg_out : 10;
        unsigned int reserved_1 : 6;
        unsigned int vdec_vcodecbus_cfg_bypass : 1;
        unsigned int reserved_2 : 4;
    } reg;
} SOC_MEDIA2_CRG_PERI_AUTODIV1_UNION;
#endif
#define SOC_MEDIA2_CRG_PERI_AUTODIV1_vdec_auto_waitcfg_in_START (1)
#define SOC_MEDIA2_CRG_PERI_AUTODIV1_vdec_auto_waitcfg_in_END (10)
#define SOC_MEDIA2_CRG_PERI_AUTODIV1_vdec_auto_waitcfg_out_START (11)
#define SOC_MEDIA2_CRG_PERI_AUTODIV1_vdec_auto_waitcfg_out_END (20)
#define SOC_MEDIA2_CRG_PERI_AUTODIV1_vdec_vcodecbus_cfg_bypass_START (27)
#define SOC_MEDIA2_CRG_PERI_AUTODIV1_vdec_vcodecbus_cfg_bypass_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int vcodecbus_auto_waitcfg_in : 10;
        unsigned int vcodecbus_auto_waitcfg_out : 8;
        unsigned int vcodecbus_ics_idle_bypass : 1;
        unsigned int vcodecbus_ics_bypass : 1;
        unsigned int vcodecbus_ics2_idle_bypass : 1;
        unsigned int vcodecbus_ics2_bypass : 1;
        unsigned int reserved_1 : 4;
        unsigned int vcodecbus_vcodec_cfg_bypass : 1;
        unsigned int vcodecbus_venc_idle_bypass : 1;
        unsigned int vcodecbus_vdec_idle_bypass : 1;
        unsigned int vcodecbus_venc_bypass : 1;
        unsigned int vcodecbus_vdec_bypass : 1;
    } reg;
} SOC_MEDIA2_CRG_PERI_AUTODIV2_UNION;
#endif
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_auto_waitcfg_in_START (1)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_auto_waitcfg_in_END (10)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_auto_waitcfg_out_START (11)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_auto_waitcfg_out_END (18)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_ics_idle_bypass_START (19)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_ics_idle_bypass_END (19)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_ics_bypass_START (20)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_ics_bypass_END (20)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_ics2_idle_bypass_START (21)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_ics2_idle_bypass_END (21)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_ics2_bypass_START (22)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_ics2_bypass_END (22)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_vcodec_cfg_bypass_START (27)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_vcodec_cfg_bypass_END (27)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_venc_idle_bypass_START (28)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_venc_idle_bypass_END (28)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_vdec_idle_bypass_START (29)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_vdec_idle_bypass_END (29)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_venc_bypass_START (30)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_venc_bypass_END (30)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_vdec_bypass_START (31)
#define SOC_MEDIA2_CRG_PERI_AUTODIV2_vcodecbus_vdec_bypass_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_clr_media2bus_nonidle_pend : 1;
        unsigned int intr_mask_vcodecbus_nonidle_pend : 1;
        unsigned int intr_mask_icsbus_nonidle_pend : 1;
        unsigned int intr_mask_vdecbus_nonidle_pend : 1;
        unsigned int intr_mask_vencbus_nonidle_pend : 1;
        unsigned int intr_mask_ics2bus_nonidle_pend : 1;
        unsigned int reserved : 10;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_UNION;
#endif
#define SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_clr_media2bus_nonidle_pend_START (0)
#define SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_clr_media2bus_nonidle_pend_END (0)
#define SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_vcodecbus_nonidle_pend_START (1)
#define SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_vcodecbus_nonidle_pend_END (1)
#define SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_icsbus_nonidle_pend_START (2)
#define SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_icsbus_nonidle_pend_END (2)
#define SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_vdecbus_nonidle_pend_START (3)
#define SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_vdecbus_nonidle_pend_END (3)
#define SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_vencbus_nonidle_pend_START (4)
#define SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_vencbus_nonidle_pend_END (4)
#define SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_ics2bus_nonidle_pend_START (5)
#define SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_intr_mask_ics2bus_nonidle_pend_END (5)
#define SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_bitmasken_START (16)
#define SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_stat_vcodecbus_nonidle_pend : 1;
        unsigned int intr_stat_icsbus_nonidle_pend : 1;
        unsigned int intr_stat_vdecbus_nonidle_pend : 1;
        unsigned int intr_stat_vencbus_nonidle_pend : 1;
        unsigned int intr_stat_ics2bus_nonidle_pend : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_MEDIA2_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_UNION;
#endif
#define SOC_MEDIA2_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_vcodecbus_nonidle_pend_START (0)
#define SOC_MEDIA2_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_vcodecbus_nonidle_pend_END (0)
#define SOC_MEDIA2_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_icsbus_nonidle_pend_START (1)
#define SOC_MEDIA2_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_icsbus_nonidle_pend_END (1)
#define SOC_MEDIA2_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_vdecbus_nonidle_pend_START (2)
#define SOC_MEDIA2_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_vdecbus_nonidle_pend_END (2)
#define SOC_MEDIA2_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_vencbus_nonidle_pend_START (3)
#define SOC_MEDIA2_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_vencbus_nonidle_pend_END (3)
#define SOC_MEDIA2_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_ics2bus_nonidle_pend_START (4)
#define SOC_MEDIA2_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_ics2bus_nonidle_pend_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int media2_lm_clkrst_bypass : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_MEDIA2_CRG_IPCLKRST_BYPASS_UNION;
#endif
#define SOC_MEDIA2_CRG_IPCLKRST_BYPASS_media2_lm_clkrst_bypass_START (0)
#define SOC_MEDIA2_CRG_IPCLKRST_BYPASS_media2_lm_clkrst_bypass_END (0)
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
} SOC_MEDIA2_CRG_DMSS_ASYNC_BRG_UNION;
#endif
#define SOC_MEDIA2_CRG_DMSS_ASYNC_BRG_dmss_ckg_byp_START (0)
#define SOC_MEDIA2_CRG_DMSS_ASYNC_BRG_dmss_ckg_byp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tz_secure_n_med2_lm : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_MEDIA2_CRG_DECPROT0SET_UNION;
#endif
#define SOC_MEDIA2_CRG_DECPROT0SET_tz_secure_n_med2_lm_START (0)
#define SOC_MEDIA2_CRG_DECPROT0SET_tz_secure_n_med2_lm_END (0)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
