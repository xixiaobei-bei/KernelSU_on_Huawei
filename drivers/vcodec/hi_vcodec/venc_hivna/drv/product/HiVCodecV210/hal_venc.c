#include "hal_venc.h"
#include "venc_regulator.h"
#include "drv_venc_efl.h"
#include "drv_venc_osal.h"
#include "hi_drv_mem.h"
#include <linux/hisi-iommu.h>
#include <linux/iommu.h>


/*lint -e438 -e838 -e826*/

#define DIST_PROTOCOL(protocol, value1, value2) ((VEDU_H265 == protocol) ? (value1) : (value2))

extern VENC_SMMU_ERR_ADDR g_smmu_err_mem;
extern VeduEfl_IpCtx_S VeduIpCtx;
extern struct iommu_domain* g_hisi_mmu_domain;
extern HI_BOOL g_vencSmmuSetFlag;

HI_VOID VENC_HAL_ClrAllInt(S_HEVC_AVC_REGS_TYPE *pVeduReg)
{
    pVeduReg->VEDU_VCPI_INTCLR.u32 = 0xFFFFFFFF;
}

HI_VOID VENC_HAL_DisableAllInt(S_HEVC_AVC_REGS_TYPE *pVeduReg)
{
    pVeduReg->VEDU_VCPI_INTMASK.u32 = 0;
}

HI_S32 VENC_HAL_ResetReg()
{
    HI_S32 s32Ret = HI_FAILURE;
    g_vencSmmuSetFlag = HI_FALSE;
    s32Ret = Venc_Regulator_Disable();
    s32Ret |= Venc_Regulator_Enable();
    return s32Ret;
}

HI_VOID VENC_HAL_SetSmmuAddr(S_HEVC_AVC_REGS_TYPE *pVeduReg)
{
	pVeduReg->SMMU_ERR_RDADDR                = g_smmu_err_mem.RdAddr & 0xFFFFFFFF;//config alloc phyaddr,in order system don't dump
	pVeduReg->SMMU_ADDR_MSB.bits.msb_errrd   = (g_smmu_err_mem.RdAddr >> 32)&0x7F;
	pVeduReg->SMMU_ERR_WRADDR[0]             = g_smmu_err_mem.WrAddr & 0xFFFFFFFF;
	pVeduReg->SMMU_ADDR_MSB.bits.msb_errwr   = (g_smmu_err_mem.WrAddr >> 32)&0x7F;
}

HI_VOID VENC_HAL_StartEncode(S_HEVC_AVC_REGS_TYPE *pVeduReg)
{
    if (pVeduReg) {
        pVeduReg->VEDU_VCPI_START.bits.vcpi_vstart = 0;
        pVeduReg->VEDU_VCPI_START.bits.vcpi_vstart = 1;
    }
}

HI_VOID VENC_HAL_Get_CfgRegSimple(VENC_REG_INFO_S *pVeduReg)
{
    S_HEVC_AVC_REGS_TYPE * pAllReg = (S_HEVC_AVC_REGS_TYPE*)VeduIpCtx.pRegBase;

    pVeduReg->all_reg.FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_eop = pAllReg->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_eop;
    pVeduReg->all_reg.FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_buffull = pAllReg->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_buffull;

    pVeduReg->all_reg.FUNC_VLCST_DSRPTR00.bits.slc_len0 = pAllReg->FUNC_VLCST_DSRPTR00.bits.slc_len0;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR10.bits.slc_len1 = pAllReg->FUNC_VLCST_DSRPTR10.bits.slc_len1;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR20.bits.slc_len2 = pAllReg->FUNC_VLCST_DSRPTR20.bits.slc_len2;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR30.bits.slc_len3 = pAllReg->FUNC_VLCST_DSRPTR30.bits.slc_len3;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR40.bits.slc_len4 = pAllReg->FUNC_VLCST_DSRPTR40.bits.slc_len4;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR50.bits.slc_len5 = pAllReg->FUNC_VLCST_DSRPTR50.bits.slc_len5;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR60.bits.slc_len6 = pAllReg->FUNC_VLCST_DSRPTR60.bits.slc_len6;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR70.bits.slc_len7 = pAllReg->FUNC_VLCST_DSRPTR70.bits.slc_len7;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR80.bits.slc_len8 = pAllReg->FUNC_VLCST_DSRPTR80.bits.slc_len8;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR90.bits.slc_len9 = pAllReg->FUNC_VLCST_DSRPTR90.bits.slc_len9;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR100.bits.slc_len10 = pAllReg->FUNC_VLCST_DSRPTR100.bits.slc_len10;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR110.bits.slc_len11 = pAllReg->FUNC_VLCST_DSRPTR110.bits.slc_len11;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR120.bits.slc_len12 = pAllReg->FUNC_VLCST_DSRPTR120.bits.slc_len12;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR130.bits.slc_len13 = pAllReg->FUNC_VLCST_DSRPTR130.bits.slc_len13;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR140.bits.slc_len14 = pAllReg->FUNC_VLCST_DSRPTR140.bits.slc_len14;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR150.bits.slc_len15 = pAllReg->FUNC_VLCST_DSRPTR150.bits.slc_len15;

    pVeduReg->all_reg.FUNC_VLCST_DSRPTR01.bits.invalidnum0 = pAllReg->FUNC_VLCST_DSRPTR01.bits.invalidnum0;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR11.bits.invalidnum1 = pAllReg->FUNC_VLCST_DSRPTR11.bits.invalidnum1;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR21.bits.invalidnum2 = pAllReg->FUNC_VLCST_DSRPTR21.bits.invalidnum2;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR31.bits.invalidnum3 = pAllReg->FUNC_VLCST_DSRPTR31.bits.invalidnum3;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR41.bits.invalidnum4 = pAllReg->FUNC_VLCST_DSRPTR41.bits.invalidnum4;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR51.bits.invalidnum5 = pAllReg->FUNC_VLCST_DSRPTR51.bits.invalidnum5;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR61.bits.invalidnum6 = pAllReg->FUNC_VLCST_DSRPTR61.bits.invalidnum6;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR71.bits.invalidnum7 = pAllReg->FUNC_VLCST_DSRPTR71.bits.invalidnum7;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR81.bits.invalidnum8 = pAllReg->FUNC_VLCST_DSRPTR81.bits.invalidnum8;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR91.bits.invalidnum9 = pAllReg->FUNC_VLCST_DSRPTR91.bits.invalidnum9;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR101.bits.invalidnum10 = pAllReg->FUNC_VLCST_DSRPTR101.bits.invalidnum10;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR111.bits.invalidnum11 = pAllReg->FUNC_VLCST_DSRPTR111.bits.invalidnum11;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR121.bits.invalidnum12 = pAllReg->FUNC_VLCST_DSRPTR121.bits.invalidnum12;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR131.bits.invalidnum13 = pAllReg->FUNC_VLCST_DSRPTR131.bits.invalidnum13 ;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR141.bits.invalidnum14 = pAllReg->FUNC_VLCST_DSRPTR141.bits.invalidnum14;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR151.bits.invalidnum15 = pAllReg->FUNC_VLCST_DSRPTR151.bits.invalidnum15;
}

HI_VOID VENC_HAL_Get_Reg_Venc(VENC_REG_INFO_S *pVeduReg)
{
    S_HEVC_AVC_REGS_TYPE * pAllReg = (S_HEVC_AVC_REGS_TYPE*)VeduIpCtx.pRegBase;/*lint !e826 */

    pVeduReg->all_reg.FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_eop = pAllReg->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_eop;
    pVeduReg->all_reg.FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_buffull = pAllReg->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_buffull;

    pVeduReg->all_reg.FUNC_CABAC_PIC_STRMSIZE = pAllReg->FUNC_CABAC_PIC_STRMSIZE;
    pVeduReg->all_reg.FUNC_SEL_OPT_4X4_CNT.bits.opt_4x4_cnt = pAllReg->FUNC_SEL_OPT_4X4_CNT.bits.opt_4x4_cnt;
    pVeduReg->all_reg.FUNC_SEL_INTRA_OPT_8X8_CNT.bits.intra_opt_8x8_cnt = pAllReg->FUNC_SEL_INTRA_OPT_8X8_CNT.bits.intra_opt_8x8_cnt;
    pVeduReg->all_reg.FUNC_SEL_INTRA_NORMAL_OPT_8X8_CNT.bits.intra_normal_opt_8x8_cnt = pAllReg->FUNC_SEL_INTRA_NORMAL_OPT_8X8_CNT.bits.intra_normal_opt_8x8_cnt;
    pVeduReg->all_reg.FUNC_SEL_INTRA_OPT_16X16_CNT.bits.intra_opt_16x16_cnt = pAllReg->FUNC_SEL_INTRA_OPT_16X16_CNT.bits.intra_opt_16x16_cnt;
    pVeduReg->all_reg.FUNC_SEL_INTRA_OPT_32X32_CNT.bits.intra_opt_32x32_cnt = pAllReg->FUNC_SEL_INTRA_OPT_32X32_CNT.bits.intra_opt_32x32_cnt;
    pVeduReg->all_reg.FUNC_VLC_PIC_BITS = pAllReg->FUNC_VLC_PIC_BITS;

    pVeduReg->all_reg.FUNC_SEL_INTRA_PCM_OPT_8X8_CNT.bits.pcm_opt_8x8_cnt = pAllReg->FUNC_SEL_INTRA_PCM_OPT_8X8_CNT.bits.pcm_opt_8x8_cnt;
    //pVeduReg->all_reg.FUNC_SEL_INTRA_OPT_16X16_CNT.bits.intra_opt_16x16_cnt = pAllReg->FUNC_SEL_INTRA_OPT_16X16_CNT.bits.intra_opt_16x16_cnt;//?ظ?
    pVeduReg->all_reg.FUNC_PME_MADI_SUM = pAllReg->FUNC_PME_MADI_SUM;

    pVeduReg->all_reg.FUNC_VLCST_DSRPTR00.bits.slc_len0 = pAllReg->FUNC_VLCST_DSRPTR00.bits.slc_len0;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR10.bits.slc_len1 = pAllReg->FUNC_VLCST_DSRPTR10.bits.slc_len1;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR20.bits.slc_len2 = pAllReg->FUNC_VLCST_DSRPTR20.bits.slc_len2;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR30.bits.slc_len3 = pAllReg->FUNC_VLCST_DSRPTR30.bits.slc_len3;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR40.bits.slc_len4 = pAllReg->FUNC_VLCST_DSRPTR40.bits.slc_len4;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR50.bits.slc_len5 = pAllReg->FUNC_VLCST_DSRPTR50.bits.slc_len5;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR60.bits.slc_len6 = pAllReg->FUNC_VLCST_DSRPTR60.bits.slc_len6;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR70.bits.slc_len7 = pAllReg->FUNC_VLCST_DSRPTR70.bits.slc_len7;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR80.bits.slc_len8 = pAllReg->FUNC_VLCST_DSRPTR80.bits.slc_len8;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR90.bits.slc_len9 = pAllReg->FUNC_VLCST_DSRPTR90.bits.slc_len9;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR100.bits.slc_len10 = pAllReg->FUNC_VLCST_DSRPTR100.bits.slc_len10;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR110.bits.slc_len11 = pAllReg->FUNC_VLCST_DSRPTR110.bits.slc_len11;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR120.bits.slc_len12 = pAllReg->FUNC_VLCST_DSRPTR120.bits.slc_len12;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR130.bits.slc_len13 = pAllReg->FUNC_VLCST_DSRPTR130.bits.slc_len13;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR140.bits.slc_len14 = pAllReg->FUNC_VLCST_DSRPTR140.bits.slc_len14;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR150.bits.slc_len15 = pAllReg->FUNC_VLCST_DSRPTR150.bits.slc_len15;

    pVeduReg->all_reg.FUNC_VLCST_DSRPTR01.bits.invalidnum0 = pAllReg->FUNC_VLCST_DSRPTR01.bits.invalidnum0;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR11.bits.invalidnum1 = pAllReg->FUNC_VLCST_DSRPTR11.bits.invalidnum1;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR21.bits.invalidnum2 = pAllReg->FUNC_VLCST_DSRPTR21.bits.invalidnum2;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR31.bits.invalidnum3 = pAllReg->FUNC_VLCST_DSRPTR31.bits.invalidnum3;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR41.bits.invalidnum4 = pAllReg->FUNC_VLCST_DSRPTR41.bits.invalidnum4;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR51.bits.invalidnum5 = pAllReg->FUNC_VLCST_DSRPTR51.bits.invalidnum5;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR61.bits.invalidnum6 = pAllReg->FUNC_VLCST_DSRPTR61.bits.invalidnum6;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR71.bits.invalidnum7 = pAllReg->FUNC_VLCST_DSRPTR71.bits.invalidnum7;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR81.bits.invalidnum8 = pAllReg->FUNC_VLCST_DSRPTR81.bits.invalidnum8;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR91.bits.invalidnum9 = pAllReg->FUNC_VLCST_DSRPTR91.bits.invalidnum9;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR101.bits.invalidnum10 = pAllReg->FUNC_VLCST_DSRPTR101.bits.invalidnum10;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR111.bits.invalidnum11 = pAllReg->FUNC_VLCST_DSRPTR111.bits.invalidnum11;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR121.bits.invalidnum12 = pAllReg->FUNC_VLCST_DSRPTR121.bits.invalidnum12;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR131.bits.invalidnum13 = pAllReg->FUNC_VLCST_DSRPTR131.bits.invalidnum13 ;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR141.bits.invalidnum14 = pAllReg->FUNC_VLCST_DSRPTR141.bits.invalidnum14;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR151.bits.invalidnum15 = pAllReg->FUNC_VLCST_DSRPTR151.bits.invalidnum15;

    pVeduReg->all_reg.FUNC_VLCST_DSRPTR01.bits.islastslc0 = pAllReg->FUNC_VLCST_DSRPTR01.bits.islastslc0;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR11.bits.islastslc1 = pAllReg->FUNC_VLCST_DSRPTR11.bits.islastslc1;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR21.bits.islastslc2 = pAllReg->FUNC_VLCST_DSRPTR21.bits.islastslc2;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR31.bits.islastslc3 = pAllReg->FUNC_VLCST_DSRPTR31.bits.islastslc3;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR41.bits.islastslc4 = pAllReg->FUNC_VLCST_DSRPTR41.bits.islastslc4;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR51.bits.islastslc5 = pAllReg->FUNC_VLCST_DSRPTR51.bits.islastslc5;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR61.bits.islastslc6 = pAllReg->FUNC_VLCST_DSRPTR61.bits.islastslc6;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR71.bits.islastslc7 = pAllReg->FUNC_VLCST_DSRPTR71.bits.islastslc7;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR81.bits.islastslc8 = pAllReg->FUNC_VLCST_DSRPTR81.bits.islastslc8;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR91.bits.islastslc9 = pAllReg->FUNC_VLCST_DSRPTR91.bits.islastslc9;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR101.bits.islastslc10 = pAllReg->FUNC_VLCST_DSRPTR101.bits.islastslc10;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR111.bits.islastslc11 = pAllReg->FUNC_VLCST_DSRPTR111.bits.islastslc11;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR121.bits.islastslc12 = pAllReg->FUNC_VLCST_DSRPTR121.bits.islastslc12;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR131.bits.islastslc13 = pAllReg->FUNC_VLCST_DSRPTR131.bits.islastslc13;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR141.bits.islastslc14 = pAllReg->FUNC_VLCST_DSRPTR141.bits.islastslc14;
    pVeduReg->all_reg.FUNC_VLCST_DSRPTR151.bits.islastslc15 = pAllReg->FUNC_VLCST_DSRPTR151.bits.islastslc15;

    pVeduReg->all_reg.FUNC_SAO_LCU_CNT.bits.sao_lcu_cnt = pAllReg->FUNC_SAO_LCU_CNT.bits.sao_lcu_cnt;
    pVeduReg->all_reg.FUNC_SAO_OFF_NUM.bits.saooff_chroma_num = pAllReg->FUNC_SAO_OFF_NUM.bits.saooff_chroma_num;
    pVeduReg->all_reg.FUNC_SAO_OFF_NUM.bits.saooff_luma_num = pAllReg->FUNC_SAO_OFF_NUM.bits.saooff_luma_num;

    pVeduReg->all_reg.FUNC_VCPI_VEDU_TIMER = pAllReg->FUNC_VCPI_VEDU_TIMER;
    pVeduReg->all_reg.FUNC_VCPI_IDLE_TIMER = pAllReg->FUNC_VCPI_IDLE_TIMER;

}
HI_VOID VeduHal_CfgReg_IntraSet(VENC_REG_INFO_S *channelcfg)
{
    S_HEVC_AVC_REGS_TYPE *pAllReg = NULL;

    pAllReg = (S_HEVC_AVC_REGS_TYPE*)VeduIpCtx.pRegBase;

    pAllReg->VEDU_VCTRL_INTRA_RDO_FACTOR_0.u32 = channelcfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_0.u32;

    pAllReg->VEDU_VCTRL_INTRA_RDO_FACTOR_1.u32 = channelcfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_1.u32;

    pAllReg->VEDU_VCTRL_INTRA_RDO_FACTOR_2.u32 = channelcfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_2.u32;

    pAllReg->VEDU_PME_NEW_COST.bits.pme_new_cost_en = channelcfg->all_reg.VEDU_PME_NEW_COST.bits.pme_new_cost_en;

}

HI_VOID VeduHal_CfgReg_LambdaSet(VENC_REG_INFO_S *channelcfg)
{
    S_HEVC_AVC_REGS_TYPE *pAllReg = NULL;

    pAllReg = (S_HEVC_AVC_REGS_TYPE*)VeduIpCtx.pRegBase;

    //lambd reg cfg set
    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG00.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG00.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG01.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG01.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG02.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG02.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG03.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG03.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG04.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG04.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG05.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG05.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG06.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG06.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG07.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG07.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG08.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG08.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG09.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG09.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG10.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG10.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG11.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG11.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG12.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG12.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG13.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG13.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG14.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG14.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG15.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG15.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG16.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG16.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG17.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG17.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG18.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG18.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG19.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG19.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG20.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG20.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG21.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG21.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG22.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG22.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG23.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG23.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG24.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG24.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG25.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG25.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG26.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG26.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG27.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG27.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG28.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG28.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG29.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG29.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG30.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG30.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG31.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG31.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG32.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG32.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG33.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG33.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG34.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG34.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG35.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG35.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG36.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG36.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG37.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG37.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG38.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG38.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG39.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG39.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG40.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG40.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG41.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG41.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG42.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG42.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG43.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG43.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG44.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG44.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG45.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG45.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG46.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG46.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG47.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG47.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG48.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG48.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG49.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG49.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG50.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG50.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG51.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG51.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG52.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG52.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG53.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG53.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG54.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG54.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG55.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG55.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG56.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG56.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG57.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG57.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG58.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG58.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG59.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG59.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG60.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG60.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG61.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG61.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG62.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG62.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG63.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG63.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG64.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG64.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG65.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG65.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG66.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG66.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG67.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG67.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG68.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG68.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG69.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG69.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG70.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG70.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG71.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG71.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG72.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG72.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG73.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG73.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG74.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG74.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG75.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG75.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG76.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG76.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG77.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG77.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG78.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG78.u32;
    }

    {
        pAllReg->VEDU_QPG_QP_LAMBDA_CTRL_REG79.u32 = channelcfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG79.u32;
    }

}
HI_VOID VeduHal_CfgReg_QpgmapSet(VENC_REG_INFO_S *channelcfg  )
{
    S_HEVC_AVC_REGS_TYPE *pAllReg = NULL;

    pAllReg = (S_HEVC_AVC_REGS_TYPE*)VeduIpCtx.pRegBase;

    {
        pAllReg->VEDU_VCTRL_ROI_CFG0.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_CFG0.u32;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_CFG1.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_CFG1.u32;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_CFG2.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_CFG2.u32;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_SIZE_0.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_SIZE_0.u32;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_SIZE_1.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_SIZE_1.u32;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_SIZE_2.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_SIZE_2.u32;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_SIZE_3.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_SIZE_3.u32;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_SIZE_4.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_SIZE_4.u32;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_SIZE_5.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_SIZE_5.u32;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_SIZE_6.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_SIZE_6.u32;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_SIZE_7.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_SIZE_7.u32;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_START_0.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_START_0.u32;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_START_1.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_START_1.u32;;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_START_2.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_START_2.u32;;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_START_3.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_START_3.u32;;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_START_4.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_START_4.u32;;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_START_5.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_START_5.u32;;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_START_6.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_START_6.u32;;
    }

    {
        pAllReg->VEDU_VCTRL_ROI_START_7.u32 = channelcfg->all_reg.VEDU_VCTRL_ROI_START_7.u32;;
    }

    {
        pAllReg->VEDU_VCPI_RC_ENABLE.u32 = channelcfg->all_reg.VEDU_VCPI_RC_ENABLE.u32;
    }
}
HI_VOID VeduHal_CfgReg_AddrSet(VENC_REG_INFO_S *channelcfg)
{
    S_HEVC_AVC_REGS_TYPE *pAllReg = NULL;

    HI_DBG_VENC("set Vedu Hal cfg reg addr\n");
    pAllReg = (S_HEVC_AVC_REGS_TYPE*)VeduIpCtx.pRegBase;
    //Col_frm_flag PMC_POC And So On;   I P B All need to Cfg
    {
        pAllReg->VEDU_VCPI_STRFMT.u32 = channelcfg->all_reg.VEDU_VCPI_STRFMT.u32;
    }

    {
        pAllReg->VEDU_VCPI_REF_FLAG.u32 = channelcfg->all_reg.VEDU_VCPI_REF_FLAG.u32;
    }

    {
        pAllReg->VEDU_PMV_POC_0 = channelcfg->all_reg.VEDU_PMV_POC_0;
    }
    {
        pAllReg->VEDU_PMV_POC_1 = channelcfg->all_reg.VEDU_PMV_POC_1;
    }
    {
        pAllReg->VEDU_PMV_POC_2 = channelcfg->all_reg.VEDU_PMV_POC_2;
    }
    {
        pAllReg->VEDU_PMV_POC_3 = channelcfg->all_reg.VEDU_PMV_POC_3;
    }
    {
        pAllReg->VEDU_PMV_POC_4 = channelcfg->all_reg.VEDU_PMV_POC_4;
    }
    {
        pAllReg->VEDU_PMV_POC_5 = channelcfg->all_reg.VEDU_PMV_POC_5;
    }

    pAllReg->VEDU_VCPI_TUNLCELL_ADDR = channelcfg->all_reg.VEDU_VCPI_TUNLCELL_ADDR;//pEncPara_channel->VEDU_SRC_YADDR;//pEncPara->vcpi_curld_tunlcell_addr;
    pAllReg->VEDU_VCPI_SRC_YADDR = channelcfg->all_reg.VEDU_VCPI_SRC_YADDR;//pEncPara_channel->VEDU_SRC_YADDR;//pEncPara->vcpi_curld_y_addr;
    pAllReg->VEDU_VCPI_SRC_CADDR = channelcfg->all_reg.VEDU_VCPI_SRC_CADDR;//pEncPara_channel->VEDU_SRC_CADDR;//pEncPara->vcpi_curld_c_addr;
    pAllReg->VEDU_VCPI_SRC_VADDR = channelcfg->all_reg.VEDU_VCPI_SRC_VADDR;//pEncPara_channel->VEDU_SRC_VADDR;//pEncPara->vcpi_curld_v_addr;

    {
        pAllReg->VEDU_VCPI_STRIDE.u32 = channelcfg->all_reg.VEDU_VCPI_STRIDE.u32;
    }
    HI_DBG_VENC("reg VEDU_VCPI_SRC_YADDR:%pK\n", (void *)(uintptr_t)(channelcfg->all_reg.VEDU_VCPI_SRC_YADDR));
    HI_DBG_VENC("reg VEDU_VCPI_SRC_CADDR:%pK\n", (void *)(uintptr_t)(channelcfg->all_reg.VEDU_VCPI_SRC_CADDR));
    HI_DBG_VENC("reg VEDU_VCPI_SRC_VADDR:%pK\n", (void *)(uintptr_t)(channelcfg->all_reg.VEDU_VCPI_SRC_VADDR));
    HI_DBG_VENC("reg vcpi_curld_c_stride:%x\n", channelcfg->all_reg.VEDU_VCPI_STRIDE.bits.vcpi_curld_c_stride);
    HI_DBG_VENC("reg vcpi_curld_y_stride:%x\n", channelcfg->all_reg.VEDU_VCPI_STRIDE.bits.vcpi_curld_y_stride);
    if(channelcfg->all_reg.VEDU_VCPI_STRFMT.bits.vcpi_str_fmt == 10)
    {
        pAllReg->VEDU_VCPI_YH_ADDR = channelcfg->all_reg.VEDU_VCPI_YH_ADDR;//pEncPara->vcpi_curld_yh_addr;
        pAllReg->VEDU_VCPI_CH_ADDR = channelcfg->all_reg.VEDU_VCPI_CH_ADDR;//pEncPara->vcpi_curld_ch_addr;
    }

    //-----------------------------------------------------------------------------------------
    //-------------------REC,REF0,REF1,PME0,PME1,NBI Relate ADDR Stide-----------------------
    //
    //REC-------------------REC-----------------REC--------------------------------------------
    pAllReg->VEDU_VCPI_REC_YADDR = channelcfg->all_reg.VEDU_VCPI_REC_YADDR;//pEncPara->vcpi_recst_yaddr;
    pAllReg->VEDU_VCPI_REC_CADDR = channelcfg->all_reg.VEDU_VCPI_REC_CADDR;//pEncPara->vcpi_recst_caddr;
    {
        pAllReg->VEDU_VCPI_REC_STRIDE.u32 = channelcfg->all_reg.VEDU_VCPI_REC_STRIDE.u32;
    }

    pAllReg->VEDU_VCPI_REC_YH_ADDR = channelcfg->all_reg.VEDU_VCPI_REC_YH_ADDR;//pEncPara->vcpi_recst_yh_addr;
    pAllReg->VEDU_VCPI_REC_CH_ADDR = channelcfg->all_reg.VEDU_VCPI_REC_CH_ADDR;//pEncPara->vcpi_recst_ch_addr;
    {
        pAllReg->VEDU_VCPI_REC_HEAD_STRIDE.u32 = channelcfg->all_reg.VEDU_VCPI_REC_HEAD_STRIDE.u32;
    }
    //
    //REF-------------------REF-----------------REF--------------------------------------------
    //REF0

    pAllReg->VEDU_VCPI_REFY_L0_ADDR = channelcfg->all_reg.VEDU_VCPI_REFY_L0_ADDR;
    pAllReg->VEDU_VCPI_REFC_L0_ADDR = channelcfg->all_reg.VEDU_VCPI_REFC_L0_ADDR;
    {
        pAllReg->VEDU_VCPI_REF_L0_STRIDE.u32 = channelcfg->all_reg.VEDU_VCPI_REF_L0_STRIDE.u32;
    }
    pAllReg->VEDU_VCPI_REFYH_L0_ADDR = channelcfg->all_reg.VEDU_VCPI_REFYH_L0_ADDR;
    pAllReg->VEDU_VCPI_REFCH_L0_ADDR = channelcfg->all_reg.VEDU_VCPI_REFCH_L0_ADDR;

    {
        pAllReg->VEDU_VCPI_REFH_L0_STRIDE.u32 = channelcfg->all_reg.VEDU_VCPI_REFH_L0_STRIDE.u32;
    }

    pAllReg->VEDU_VCPI_PMELD_L0_ADDR = channelcfg->all_reg.VEDU_VCPI_PMELD_L0_ADDR;
    //REF1

    pAllReg->VEDU_VCPI_REFY_L1_ADDR = channelcfg->all_reg.VEDU_VCPI_REFY_L1_ADDR;//pEncPara_channel->VEDU_REFY_L1_ADDR;
    pAllReg->VEDU_VCPI_REFC_L1_ADDR = channelcfg->all_reg.VEDU_VCPI_REFC_L1_ADDR;//pEncPara_channel->VEDU_REFC_L1_ADDR;
    {
        pAllReg->VEDU_VCPI_REF_L1_STRIDE.u32 = channelcfg->all_reg.VEDU_VCPI_REF_L1_STRIDE.u32;
    }

    pAllReg->VEDU_VCPI_REFYH_L1_ADDR = channelcfg->all_reg.VEDU_VCPI_REFYH_L1_ADDR;//pEncPara_channel->VEDU_REFYH_L1_ADDR;
    pAllReg->VEDU_VCPI_REFCH_L1_ADDR = channelcfg->all_reg.VEDU_VCPI_REFCH_L1_ADDR;//pEncPara_channel->VEDU_REFCH_L1_ADDR;

    {
        pAllReg->VEDU_VCPI_REFH_L1_STRIDE.u32 = channelcfg->all_reg.VEDU_VCPI_REFH_L1_STRIDE.u32;
    }
    pAllReg->VEDU_VCPI_PMELD_L1_ADDR = channelcfg->all_reg.VEDU_VCPI_PMELD_L1_ADDR;//pEncPara_channel->VEDU_PMELD_L1_ADDR;

    //PME NBI

    pAllReg->VEDU_VCPI_PMEST_ADDR = channelcfg->all_reg.VEDU_VCPI_PMEST_ADDR;//pEncPara->vcpi_pmest_addr;

    pAllReg->VEDU_VCPI_PMEST_STRIDE = channelcfg->all_reg.VEDU_VCPI_PMEST_STRIDE;

    pAllReg->VEDU_VCPI_PMELD_STRIDE = channelcfg->all_reg.VEDU_VCPI_PMELD_STRIDE;

    //NBI
    //{
    //   U_VEDU_VCPI_NBI_UPST_ADDR  D32;

    //   pAllReg->VEDU_VCPI_NBI_UPST_ADDR = channelcfg->all_reg..u32;
    //}

    pAllReg->VEDU_VCPI_NBI_MVST_ADDR = channelcfg->all_reg.VEDU_VCPI_NBI_MVST_ADDR;//pEncPara->vcpi_mvst_address;
    pAllReg->VEDU_VCPI_NBI_MVLD_ADDR = channelcfg->all_reg.VEDU_VCPI_NBI_MVLD_ADDR;//pEncPara->vcpi_mvld_address;
    //
    //PMEINFO
    pAllReg->VEDU_VCPI_PMEINFO_ST_ADDR = channelcfg->all_reg.VEDU_VCPI_PMEINFO_ST_ADDR;//pEncPara->vcpi_pmest_info_addr;
    pAllReg->VEDU_VCPI_PMEINFO_LD0_ADDR = channelcfg->all_reg.VEDU_VCPI_PMEINFO_LD0_ADDR;//pEncPara->vcpi_pmeinfo_ld_0_addr;
    pAllReg->VEDU_VCPI_PMEINFO_LD1_ADDR = channelcfg->all_reg.VEDU_VCPI_PMEINFO_LD1_ADDR;//pEncPara_channel->VEDU_PMEINFO_LD1_ADDR;//pEncPara->vcpi_pmeinfo_ld_1_addr;
}

HI_VOID VeduHal_CfgReg_SlcHeadSet(VENC_REG_INFO_S *channelcfg)
{
    S_HEVC_AVC_REGS_TYPE *pAllReg = NULL;

    HI_DBG_VENC("set Vedu Hal SlcHead\n");
    pAllReg = (S_HEVC_AVC_REGS_TYPE*)VeduIpCtx.pRegBase;
    //----------------------------Stream head info------------------------------------------
    pAllReg->VEDU_VLCST_STRMBUFLEN0 = channelcfg->all_reg.VEDU_VLCST_STRMBUFLEN0;
    pAllReg->VEDU_VLCST_STRMBUFLEN1 = channelcfg->all_reg.VEDU_VLCST_STRMBUFLEN1;
    pAllReg->VEDU_VLCST_STRMBUFLEN2 = channelcfg->all_reg.VEDU_VLCST_STRMBUFLEN2;
    pAllReg->VEDU_VLCST_STRMBUFLEN3 = channelcfg->all_reg.VEDU_VLCST_STRMBUFLEN3;
    pAllReg->VEDU_VLCST_STRMBUFLEN4 = channelcfg->all_reg.VEDU_VLCST_STRMBUFLEN4;
    pAllReg->VEDU_VLCST_STRMBUFLEN5 = channelcfg->all_reg.VEDU_VLCST_STRMBUFLEN5;
    pAllReg->VEDU_VLCST_STRMBUFLEN6 = channelcfg->all_reg.VEDU_VLCST_STRMBUFLEN6;
    pAllReg->VEDU_VLCST_STRMBUFLEN7 = channelcfg->all_reg.VEDU_VLCST_STRMBUFLEN7;
    pAllReg->VEDU_VLCST_STRMBUFLEN8 = channelcfg->all_reg.VEDU_VLCST_STRMBUFLEN8;
    pAllReg->VEDU_VLCST_STRMBUFLEN9 = channelcfg->all_reg.VEDU_VLCST_STRMBUFLEN9;
    pAllReg->VEDU_VLCST_STRMBUFLEN10 = channelcfg->all_reg.VEDU_VLCST_STRMBUFLEN10;
    pAllReg->VEDU_VLCST_STRMBUFLEN11 = channelcfg->all_reg.VEDU_VLCST_STRMBUFLEN11;
    pAllReg->VEDU_VLCST_STRMBUFLEN12 = channelcfg->all_reg.VEDU_VLCST_STRMBUFLEN12;
    pAllReg->VEDU_VLCST_STRMBUFLEN13 = channelcfg->all_reg.VEDU_VLCST_STRMBUFLEN13;
    pAllReg->VEDU_VLCST_STRMBUFLEN14 = channelcfg->all_reg.VEDU_VLCST_STRMBUFLEN14;
    pAllReg->VEDU_VLCST_STRMBUFLEN15 = channelcfg->all_reg.VEDU_VLCST_STRMBUFLEN15;
    pAllReg->VEDU_VLCST_STRMADDR0 = channelcfg->all_reg.VEDU_VLCST_STRMADDR0;
    pAllReg->VEDU_VLCST_STRMADDR1 = channelcfg->all_reg.VEDU_VLCST_STRMADDR1;
    pAllReg->VEDU_VLCST_STRMADDR2 = channelcfg->all_reg.VEDU_VLCST_STRMADDR2;
    pAllReg->VEDU_VLCST_STRMADDR3 = channelcfg->all_reg.VEDU_VLCST_STRMADDR3;
    pAllReg->VEDU_VLCST_STRMADDR4 = channelcfg->all_reg.VEDU_VLCST_STRMADDR4;
    pAllReg->VEDU_VLCST_STRMADDR5 = channelcfg->all_reg.VEDU_VLCST_STRMADDR5;
    pAllReg->VEDU_VLCST_STRMADDR6 = channelcfg->all_reg.VEDU_VLCST_STRMADDR6;
    pAllReg->VEDU_VLCST_STRMADDR7 = channelcfg->all_reg.VEDU_VLCST_STRMADDR7;
    pAllReg->VEDU_VLCST_STRMADDR8 = channelcfg->all_reg.VEDU_VLCST_STRMADDR8;
    pAllReg->VEDU_VLCST_STRMADDR9 = channelcfg->all_reg.VEDU_VLCST_STRMADDR9;
    pAllReg->VEDU_VLCST_STRMADDR10 = channelcfg->all_reg.VEDU_VLCST_STRMADDR10;
    pAllReg->VEDU_VLCST_STRMADDR11 = channelcfg->all_reg.VEDU_VLCST_STRMADDR11;
    pAllReg->VEDU_VLCST_STRMADDR12 = channelcfg->all_reg.VEDU_VLCST_STRMADDR12;
    pAllReg->VEDU_VLCST_STRMADDR13 = channelcfg->all_reg.VEDU_VLCST_STRMADDR13;
    pAllReg->VEDU_VLCST_STRMADDR14 = channelcfg->all_reg.VEDU_VLCST_STRMADDR14;
    pAllReg->VEDU_VLCST_STRMADDR15 = channelcfg->all_reg.VEDU_VLCST_STRMADDR15;

    {
        pAllReg->VEDU_CABAC_GLB_CFG.u32 = channelcfg->all_reg.VEDU_CABAC_GLB_CFG.u32;
    }

    {
        pAllReg->VEDU_CABAC_SLCHDR_SIZE.u32 = channelcfg->all_reg.VEDU_CABAC_SLCHDR_SIZE.u32;
    }

    pAllReg->VEDU_CABAC_SLCHDR_PART1.bits.cabac_slchdr_part1 = channelcfg->all_reg.VEDU_CABAC_SLCHDR_PART1.bits.cabac_slchdr_part1;

    {
        pAllReg->VEDU_VCPI_VLC_CONFIG.u32 = channelcfg->all_reg.VEDU_VCPI_VLC_CONFIG.u32;
    }


    if (channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_protocol == VEDU_H265)
    {
        pAllReg->VEDU_CABAC_SLCHDR_PART2_SEG1 = channelcfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG1;
        pAllReg->VEDU_CABAC_SLCHDR_PART2_SEG2 = channelcfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG2;
        pAllReg->VEDU_CABAC_SLCHDR_PART2_SEG3 = channelcfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG3;
        pAllReg->VEDU_CABAC_SLCHDR_PART2_SEG4 = channelcfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG4;
        pAllReg->VEDU_CABAC_SLCHDR_PART2_SEG5 = channelcfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG5;
        pAllReg->VEDU_CABAC_SLCHDR_PART2_SEG6 = channelcfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG6;
        pAllReg->VEDU_CABAC_SLCHDR_PART2_SEG7 = channelcfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG7;
        pAllReg->VEDU_CABAC_SLCHDR_PART2_SEG8 = channelcfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG8;
    }
    else
    {
        pAllReg->VEDU_VLC_SLCHDRSTRM0 = channelcfg->all_reg.VEDU_VLC_SLCHDRSTRM0;
        pAllReg->VEDU_VLC_SLCHDRSTRM1 = channelcfg->all_reg.VEDU_VLC_SLCHDRSTRM1;
        pAllReg->VEDU_VLC_SLCHDRSTRM2 = channelcfg->all_reg.VEDU_VLC_SLCHDRSTRM2;
        pAllReg->VEDU_VLC_SLCHDRSTRM3 = channelcfg->all_reg.VEDU_VLC_SLCHDRSTRM3;
        pAllReg->VEDU_VLC_REORDERSTRM0 = channelcfg->all_reg.VEDU_VLC_REORDERSTRM0;
        pAllReg->VEDU_VLC_REORDERSTRM1 = channelcfg->all_reg.VEDU_VLC_REORDERSTRM1;
        pAllReg->VEDU_VLC_MARKINGSTRM0 = channelcfg->all_reg.VEDU_VLC_MARKINGSTRM0;
        pAllReg->VEDU_VLC_MARKINGSTRM1 = channelcfg->all_reg.VEDU_VLC_MARKINGSTRM1;

        {
            pAllReg->VEDU_VLC_SLCHDRPARA.u32 = channelcfg->all_reg.VEDU_VLC_SLCHDRPARA.u32;
        }

        {
            pAllReg->VEDU_VLC_SVC.u32 = channelcfg->all_reg.VEDU_VLC_SVC.u32;
        }
    }
    //----------------------------Cabac/Vlc Stream Head info end-------------------------
    {
#ifdef HARDWARE_SPLIT_SPS_PPS_EN
        if (channelcfg->bFirstNal2Send)
        {
            pAllReg->VEDU_VLCST_PARA_ADDR = channelcfg->all_reg.VEDU_VLCST_PARA_ADDR;
        }
#endif

        pAllReg->VEDU_VLCST_PARAMETER.u32 = channelcfg->all_reg.VEDU_VLCST_PARAMETER.u32;
    }
    if (channelcfg->all_reg.VEDU_VLCST_PARAMETER.bits.vlcst_para_set_en == 1)
    {

        pAllReg->VEDU_VLCST_PARA_DATA0 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA0;
        pAllReg->VEDU_VLCST_PARA_DATA1 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA1;
        pAllReg->VEDU_VLCST_PARA_DATA2 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA2;
        pAllReg->VEDU_VLCST_PARA_DATA3 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA3;
        pAllReg->VEDU_VLCST_PARA_DATA4 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA4;
        pAllReg->VEDU_VLCST_PARA_DATA5 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA5;
        pAllReg->VEDU_VLCST_PARA_DATA6 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA6;
        pAllReg->VEDU_VLCST_PARA_DATA7 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA7;
        pAllReg->VEDU_VLCST_PARA_DATA8 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA8;
        HI_DBG_VENC("reg VLCST_PARA_DATA0:0x%x\n",pAllReg->VEDU_VLCST_PARA_DATA0);
        HI_DBG_VENC("reg VLCST_PARA_DATA1:0x%x\n",pAllReg->VEDU_VLCST_PARA_DATA1);
        HI_DBG_VENC("reg VLCST_PARA_DATA2:0x%x\n",pAllReg->VEDU_VLCST_PARA_DATA2);
        HI_DBG_VENC("reg VLCST_PARA_DATA3:0x%x\n",pAllReg->VEDU_VLCST_PARA_DATA3);
        HI_DBG_VENC("reg VLCST_PARA_DATA4:0x%x\n",pAllReg->VEDU_VLCST_PARA_DATA4);
        HI_DBG_VENC("reg VLCST_PARA_DATA5:0x%x\n",pAllReg->VEDU_VLCST_PARA_DATA5);
        HI_DBG_VENC("reg VLCST_PARA_DATA6:0x%x\n",pAllReg->VEDU_VLCST_PARA_DATA6);
        HI_DBG_VENC("reg VLCST_PARA_DATA7:0x%x\n",pAllReg->VEDU_VLCST_PARA_DATA7);
        HI_DBG_VENC("reg VLCST_PARA_DATA8:0x%x\n",pAllReg->VEDU_VLCST_PARA_DATA8);
        pAllReg->VEDU_VLCST_PARA_DATA9 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA9;
        pAllReg->VEDU_VLCST_PARA_DATA10 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA10;
        pAllReg->VEDU_VLCST_PARA_DATA11 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA11;
        pAllReg->VEDU_VLCST_PARA_DATA12 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA12;
        pAllReg->VEDU_VLCST_PARA_DATA13 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA13;
        pAllReg->VEDU_VLCST_PARA_DATA14 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA14;
        pAllReg->VEDU_VLCST_PARA_DATA15 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA15;
        pAllReg->VEDU_VLCST_PARA_DATA16 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA16;
        pAllReg->VEDU_VLCST_PARA_DATA17 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA17;
        pAllReg->VEDU_VLCST_PARA_DATA18 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA18;
        pAllReg->VEDU_VLCST_PARA_DATA19 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA19;
        pAllReg->VEDU_VLCST_PARA_DATA20 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA20;
        pAllReg->VEDU_VLCST_PARA_DATA21 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA21;
        pAllReg->VEDU_VLCST_PARA_DATA22 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA22;
        pAllReg->VEDU_VLCST_PARA_DATA23 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA23;
        pAllReg->VEDU_VLCST_PARA_DATA24 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA24;
        pAllReg->VEDU_VLCST_PARA_DATA25 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA25;
        pAllReg->VEDU_VLCST_PARA_DATA26 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA26;
        pAllReg->VEDU_VLCST_PARA_DATA27 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA27;
        pAllReg->VEDU_VLCST_PARA_DATA28 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA28;
        pAllReg->VEDU_VLCST_PARA_DATA29 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA29;
        pAllReg->VEDU_VLCST_PARA_DATA30 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA30;
        pAllReg->VEDU_VLCST_PARA_DATA31 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA31;
        pAllReg->VEDU_VLCST_PARA_DATA32 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA32;
        pAllReg->VEDU_VLCST_PARA_DATA33 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA33;
        pAllReg->VEDU_VLCST_PARA_DATA34 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA34;
        pAllReg->VEDU_VLCST_PARA_DATA35 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA35;
        pAllReg->VEDU_VLCST_PARA_DATA36 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA36;
        pAllReg->VEDU_VLCST_PARA_DATA37 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA37;
        pAllReg->VEDU_VLCST_PARA_DATA38 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA38;
        pAllReg->VEDU_VLCST_PARA_DATA39 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA39;
        pAllReg->VEDU_VLCST_PARA_DATA40 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA40;
        pAllReg->VEDU_VLCST_PARA_DATA41 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA41;
        pAllReg->VEDU_VLCST_PARA_DATA42 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA42;
        pAllReg->VEDU_VLCST_PARA_DATA43 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA43;
        pAllReg->VEDU_VLCST_PARA_DATA44 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA44;
        pAllReg->VEDU_VLCST_PARA_DATA45 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA45;
        pAllReg->VEDU_VLCST_PARA_DATA46 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA46;
        pAllReg->VEDU_VLCST_PARA_DATA47 = channelcfg->all_reg.VEDU_VLCST_PARA_DATA47;
    }
}

HI_VOID VeduHal_CfgReg_SMMUSet(HI_VOID)
{
	S_HEVC_AVC_REGS_TYPE *pAllReg = NULL;
	HI_U64 phy_pgd_base;
	struct iommu_domain_data* domain_data;
	pAllReg = (S_HEVC_AVC_REGS_TYPE*)VeduIpCtx.pRegBase;

	domain_data = (struct iommu_domain_data *)(g_hisi_mmu_domain->priv);
	phy_pgd_base = (HI_U64)(domain_data->phy_pgd_base);
	HI_DBG_VENC("phy_pgd_base:%pK\n", (void *)(uintptr_t)phy_pgd_base);
	HI_DBG_VENC("(phy_pgd_base >> 32):%pK\n", (void *)(uintptr_t)(phy_pgd_base >> 32));
	//SMRX_0

	pAllReg->MMU_PRE_GLB_SCR.bits.glb_scr = 1;
	pAllReg->SMMU_MSTR_GLB_BYPASS.bits.glb_bypass = 0;
	pAllReg->SMMU_SCR.bits.glb_bypass = 0;

	//QOS
	pAllReg->SMMU_SCR.bits.rqos_en = 1;
	pAllReg->SMMU_SCR.bits.wqos_en = 1;
	pAllReg->SMMU_SCR.bits.rqos = 0;
	pAllReg->SMMU_SCR.bits.wqos = 0;

	pAllReg->SMMU_CB_SCTRL.bits.cb_bypass = 0;
	pAllReg->SMMU_CB_TTBCR.bits.cb_ttbcr_des = 1;//channelcfg->all_reg.SMMU_CB_TTBCR.bits.cb_ttbcr_des;
	pAllReg->SMMU_CB_TTBR0 = phy_pgd_base;/*lint !e712 */
	pAllReg->SMMU_CB_TTBR1 = phy_pgd_base;/*lint !e712 */
	pAllReg->SMMU_FAMA_CTRL1_NS[0].bits.fama_ptw_msb_ns = (phy_pgd_base >> 32) & 0x7F;
	VENC_HAL_SetSmmuAddr(pAllReg);

	{//nosec
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			pAllReg->SMMU_MSTR_SMRX_0[11].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			pAllReg->SMMU_MSTR_SMRX_0[12].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			pAllReg->SMMU_MSTR_SMRX_0[13].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			pAllReg->SMMU_MSTR_SMRX_0[15].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			pAllReg->SMMU_MSTR_SMRX_0[16].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 36;
			D32.bits.len = 4;
			pAllReg->SMMU_MSTR_SMRX_0[17].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 18;
			D32.bits.len = 2;
			pAllReg->SMMU_MSTR_SMRX_0[18].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 12;
			D32.bits.len = 4;
			pAllReg->SMMU_MSTR_SMRX_0[19].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 2;
			D32.bits.len = 1;
			pAllReg->SMMU_MSTR_SMRX_0[20].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 2;
			D32.bits.len = 1;
			pAllReg->SMMU_MSTR_SMRX_0[21].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			pAllReg->SMMU_MSTR_SMRX_0[29].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 1;
			D32.bits.len = 1;
			pAllReg->SMMU_MSTR_SMRX_0[30].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 1;
			D32.bits.len = 1;
			pAllReg->SMMU_MSTR_SMRX_0[31].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 8;
			D32.bits.len = 2;
			pAllReg->SMMU_MSTR_SMRX_0[32].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 4;
			D32.bits.len = 1;
			pAllReg->SMMU_MSTR_SMRX_0[33].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			pAllReg->SMMU_MSTR_SMRX_0[34].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0	D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			pAllReg->SMMU_MSTR_SMRX_0[35].u32 = D32.u32;
		}
	}
}

HI_VOID VeduHal_CfgReg_PREMMUSet(VENC_REG_INFO_S *channelcfg)
{
    S_HEVC_AVC_REGS_TYPE *pAllReg = NULL;

    pAllReg = (S_HEVC_AVC_REGS_TYPE*)VeduIpCtx.pRegBase;
    HI_DBG_VENC("MMU RRE ADDR enter %s\n", __func__);
    HI_DBG_VENC("MMU_PRE_NBI_MVST_ADDR_STR:%pK, SMMU_MSTR_SMRX_1[0]:%pK, MMU_PRE_STRMADDR0_STR:%pK\n", (void *)(&(pAllReg->MMU_PRE_NBI_MVST_ADDR_STR)),
                    (&(pAllReg->SMMU_MSTR_SMRX_1[0])), (void *)(uintptr_t)(&(pAllReg->MMU_PRE_STRMADDR0_STR)));
    HI_DBG_VENC("MMU_PRE_NBI_MVST_ADDR_STR:%pK, SMMU_MSTR_SMRX_1[0]:%pK, MMU_PRE_STRMADDR0_STR:%pK\n", (void *)(&(pAllReg->MMU_PRE_NBI_MVST_ADDR_STR)),
                    (&(pAllReg->SMMU_MSTR_SMRX_1[0])), (void *)(uintptr_t)(&(pAllReg->MMU_PRE_STRMADDR0_STR)));
    HI_DBG_VENC("MMU_PRE_NBI_MVST_ADDR_STR:%pK, SMMU_MSTR_SMRX_2[0]:%pK, MMU_PRE_STRMADDR0_STR:%pK\n", (void *)(&(pAllReg->MMU_PRE_NBI_MVST_ADDR_STR)),
                    (&(pAllReg->SMMU_MSTR_SMRX_2[0])), (void *)(uintptr_t)(&(pAllReg->MMU_PRE_STRMADDR0_STR)));
    //////PRE_ADDR

    pAllReg->MMU_PRE_NBI_MVST_ADDR_STR = channelcfg->all_reg.MMU_PRE_NBI_MVST_ADDR_STR;
    pAllReg->MMU_PRE_NBI_MVST_ADDR_END = channelcfg->all_reg.MMU_PRE_NBI_MVST_ADDR_END;

    pAllReg->MMU_PRE_NBI_MVLD_ADDR_STR = channelcfg->all_reg.MMU_PRE_NBI_MVLD_ADDR_STR;
    pAllReg->MMU_PRE_NBI_MVLD_ADDR_END = channelcfg->all_reg.MMU_PRE_NBI_MVLD_ADDR_END;

    pAllReg->MMU_PRE_PMEST_ADDR_STR = channelcfg->all_reg.MMU_PRE_PMEST_ADDR_STR;
    pAllReg->MMU_PRE_PMEST_ADDR_END = channelcfg->all_reg.MMU_PRE_PMEST_ADDR_END;

    pAllReg->MMU_PRE_PMELD_ADDR_STR = channelcfg->all_reg.MMU_PRE_PMELD_ADDR_STR;
    pAllReg->MMU_PRE_PMELD_ADDR_END = channelcfg->all_reg.MMU_PRE_PMELD_ADDR_END;

    pAllReg->MMU_PRE_PMEINFOST_ADDR_STR = channelcfg->all_reg.MMU_PRE_PMEINFOST_ADDR_STR;
    pAllReg->MMU_PRE_PMEINFOST_ADDR_END = channelcfg->all_reg.MMU_PRE_PMEINFOST_ADDR_END;

    pAllReg->MMU_PRE_PMEINFOLD0_ADDR_STR = channelcfg->all_reg.MMU_PRE_PMEINFOLD0_ADDR_STR;
    pAllReg->MMU_PRE_PMEINFOLD0_ADDR_END = channelcfg->all_reg.MMU_PRE_PMEINFOLD0_ADDR_END;

    pAllReg->MMU_PRE_PMEINFOLD1_ADDR_STR = channelcfg->all_reg.MMU_PRE_PMEINFOLD1_ADDR_STR;
    pAllReg->MMU_PRE_PMEINFOLD1_ADDR_END = channelcfg->all_reg.MMU_PRE_PMEINFOLD1_ADDR_END;

    ////REC
    pAllReg->MMU_PRE_REC_YADDR_STR = channelcfg->all_reg.MMU_PRE_REC_YADDR_STR;
    pAllReg->MMU_PRE_REC_YADDR_END = channelcfg->all_reg.MMU_PRE_REC_YADDR_END;

    pAllReg->MMU_PRE_REC_CADDR_STR = channelcfg->all_reg.MMU_PRE_REC_CADDR_STR;
    pAllReg->MMU_PRE_REC_CADDR_END = channelcfg->all_reg.MMU_PRE_REC_CADDR_END;

    ////REF
    pAllReg->MMU_PRE_REF_YADDR_STR = channelcfg->all_reg.MMU_PRE_REF_YADDR_STR;
    pAllReg->MMU_PRE_REF_YADDR_END = channelcfg->all_reg.MMU_PRE_REF_YADDR_END;

    pAllReg->MMU_PRE_REF_CADDR_STR = channelcfg->all_reg.MMU_PRE_REF_CADDR_STR;
    pAllReg->MMU_PRE_REF_CADDR_END = channelcfg->all_reg.MMU_PRE_REF_CADDR_END;

    ////SRC
    pAllReg->MMU_PRE_SRC_YADDR_STR = channelcfg->all_reg.MMU_PRE_SRC_YADDR_STR;
    pAllReg->MMU_PRE_SRC_YADDR_END = channelcfg->all_reg.MMU_PRE_SRC_YADDR_END;

    pAllReg->MMU_PRE_SRC_CADDR_STR = channelcfg->all_reg.MMU_PRE_SRC_CADDR_STR;
    pAllReg->MMU_PRE_SRC_CADDR_END = channelcfg->all_reg.MMU_PRE_SRC_CADDR_END;

    pAllReg->MMU_PRE_SRC_VADDR_STR = channelcfg->all_reg.MMU_PRE_SRC_VADDR_STR;
    pAllReg->MMU_PRE_SRC_VADDR_END = channelcfg->all_reg.MMU_PRE_SRC_VADDR_END;

    ////LOWDLY
    pAllReg->MMU_PRE_LOWDLY_ADDR_STR = channelcfg->all_reg.MMU_PRE_LOWDLY_ADDR_STR;
    pAllReg->MMU_PRE_LOWDLY_ADDR_END = channelcfg->all_reg.MMU_PRE_LOWDLY_ADDR_END;
    ////PPS
#ifdef HARDWARE_SPLIT_SPS_PPS_EN
    if (channelcfg->bFirstNal2Send)
    {
        pAllReg->MMU_PRE_PPS_ADDR_STR = channelcfg->all_reg.MMU_PRE_PPS_ADDR_STR;
        pAllReg->MMU_PRE_PPS_ADDR_END = channelcfg->all_reg.MMU_PRE_PPS_ADDR_END;
        HI_DBG_VENC("pAllReg->MMU_PRE_PPS_ADDR_STR:%pK, pAllReg->MMU_PRE_PPS_ADDR_END:%pK\n", (void *)(uintptr_t)(pAllReg->MMU_PRE_PPS_ADDR_STR), (void *)(uintptr_t)(pAllReg->MMU_PRE_PPS_ADDR_END));
    }
#endif

    ////STREAM
    pAllReg->MMU_PRE_STRMADDR0_STR = channelcfg->all_reg.MMU_PRE_STRMADDR0_STR;
    pAllReg->MMU_PRE_STRMADDR0_END = channelcfg->all_reg.MMU_PRE_STRMADDR0_END;

    pAllReg->MMU_PRE_STRMADDR1_STR = channelcfg->all_reg.MMU_PRE_STRMADDR1_STR;
    pAllReg->MMU_PRE_STRMADDR1_END = channelcfg->all_reg.MMU_PRE_STRMADDR1_END;

    pAllReg->MMU_PRE_STRMADDR2_STR = channelcfg->all_reg.MMU_PRE_STRMADDR2_STR;
    pAllReg->MMU_PRE_STRMADDR2_END = channelcfg->all_reg.MMU_PRE_STRMADDR2_END;

    pAllReg->MMU_PRE_STRMADDR3_STR = channelcfg->all_reg.MMU_PRE_STRMADDR3_STR;
    pAllReg->MMU_PRE_STRMADDR3_END = channelcfg->all_reg.MMU_PRE_STRMADDR3_END;

    pAllReg->MMU_PRE_STRMADDR4_STR = channelcfg->all_reg.MMU_PRE_STRMADDR4_STR;
    pAllReg->MMU_PRE_STRMADDR4_END = channelcfg->all_reg.MMU_PRE_STRMADDR4_END;

    pAllReg->MMU_PRE_STRMADDR5_STR = channelcfg->all_reg.MMU_PRE_STRMADDR5_STR;
    pAllReg->MMU_PRE_STRMADDR5_END = channelcfg->all_reg.MMU_PRE_STRMADDR5_END;

    pAllReg->MMU_PRE_STRMADDR6_STR = channelcfg->all_reg.MMU_PRE_STRMADDR6_STR;
    pAllReg->MMU_PRE_STRMADDR6_END = channelcfg->all_reg.MMU_PRE_STRMADDR6_END;

    pAllReg->MMU_PRE_STRMADDR7_STR = channelcfg->all_reg.MMU_PRE_STRMADDR7_STR;
    pAllReg->MMU_PRE_STRMADDR7_END = channelcfg->all_reg.MMU_PRE_STRMADDR7_END;

    pAllReg->MMU_PRE_STRMADDR8_STR = channelcfg->all_reg.MMU_PRE_STRMADDR8_STR;
    pAllReg->MMU_PRE_STRMADDR8_END = channelcfg->all_reg.MMU_PRE_STRMADDR8_END;

    pAllReg->MMU_PRE_STRMADDR9_STR = channelcfg->all_reg.MMU_PRE_STRMADDR9_STR;
    pAllReg->MMU_PRE_STRMADDR9_END = channelcfg->all_reg.MMU_PRE_STRMADDR9_END;

    pAllReg->MMU_PRE_STRMADDR10_STR = channelcfg->all_reg.MMU_PRE_STRMADDR10_STR;
    pAllReg->MMU_PRE_STRMADDR10_END = channelcfg->all_reg.MMU_PRE_STRMADDR10_END;

    pAllReg->MMU_PRE_STRMADDR11_STR = channelcfg->all_reg.MMU_PRE_STRMADDR11_STR;
    pAllReg->MMU_PRE_STRMADDR11_END = channelcfg->all_reg.MMU_PRE_STRMADDR11_END;

    pAllReg->MMU_PRE_STRMADDR12_STR = channelcfg->all_reg.MMU_PRE_STRMADDR12_STR;
    pAllReg->MMU_PRE_STRMADDR12_END = channelcfg->all_reg.MMU_PRE_STRMADDR12_END;

    pAllReg->MMU_PRE_STRMADDR13_STR = channelcfg->all_reg.MMU_PRE_STRMADDR13_STR;
    pAllReg->MMU_PRE_STRMADDR13_END = channelcfg->all_reg.MMU_PRE_STRMADDR13_END;

    pAllReg->MMU_PRE_STRMADDR14_STR = channelcfg->all_reg.MMU_PRE_STRMADDR14_STR;
    pAllReg->MMU_PRE_STRMADDR14_END = channelcfg->all_reg.MMU_PRE_STRMADDR14_END;

    pAllReg->MMU_PRE_STRMADDR15_STR = channelcfg->all_reg.MMU_PRE_STRMADDR15_STR;
    pAllReg->MMU_PRE_STRMADDR15_END = channelcfg->all_reg.MMU_PRE_STRMADDR15_END;

    ////smmu_mstr
    ////mstr1 mstr2

#if 1
    if(channelcfg->bSecureFlag == 1){ //sec

        pAllReg->SMMU_MSTR_SMRX_1[0].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[0].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[0].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[0].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[1].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[1].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[1].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[1].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[2].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[2].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[2].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[2].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[4].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[4].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[4].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[4].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[5].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[5].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[5].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[5].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[6].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[6].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[6].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[6].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[7].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[7].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[7].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[7].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[8].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[8].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[8].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[8].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[9].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[9].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[9].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[9].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[10].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[10].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[10].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[10].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[22].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[22].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[22].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[22].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[23].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[23].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[23].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[23].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[24].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[24].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[24].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[24].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[25].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[25].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[25].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[25].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[26].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[26].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[26].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[26].bits.va_end;

#ifdef HARDWARE_SPLIT_SPS_PPS_EN
        if (channelcfg->bFirstNal2Send) {
            pAllReg->SMMU_MSTR_SMRX_1[27].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[27].bits.va_str;
            pAllReg->SMMU_MSTR_SMRX_2[27].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[27].bits.va_end;
            HI_DBG_VENC("pAllReg->MMU_PRE_PPS_ADDR_STR:%pK, pAllReg->MMU_PRE_PPS_ADDR_END:%pK\n", (void *)(uintptr_t)(pAllReg->MMU_PRE_PPS_ADDR_STR), (void *)(uintptr_t)(pAllReg->MMU_PRE_PPS_ADDR_END));
        }
#endif
    }
    else {
        pAllReg->SMMU_MSTR_SMRX_1[11].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[11].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[11].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[11].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[12].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[12].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[12].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[12].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[13].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[13].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[13].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[13].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[15].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[15].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[15].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[15].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[16].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[16].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[16].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[16].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[17].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[17].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[17].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[17].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[18].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[18].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[18].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[18].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[19].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[19].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[19].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[19].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[20].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[20].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[20].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[20].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[21].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[21].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[21].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[21].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[29].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[29].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[29].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[29].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[30].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[30].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[30].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[30].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[31].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[31].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[31].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[31].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[32].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[32].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[32].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[32].bits.va_end;

        pAllReg->SMMU_MSTR_SMRX_1[33].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[33].bits.va_str;
        pAllReg->SMMU_MSTR_SMRX_2[33].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[33].bits.va_end;

#ifdef HARDWARE_SPLIT_SPS_PPS_EN
        if (channelcfg->bFirstNal2Send) {
            pAllReg->SMMU_MSTR_SMRX_1[34].bits.va_str = channelcfg->all_reg.SMMU_MSTR_SMRX_1[34].bits.va_str;
            pAllReg->SMMU_MSTR_SMRX_2[34].bits.va_end = channelcfg->all_reg.SMMU_MSTR_SMRX_2[34].bits.va_end;
            HI_DBG_VENC("pAllReg->MMU_PRE_PPS_ADDR_STR:%pK, pAllReg->MMU_PRE_PPS_ADDR_END:%pK\n",(void *)(uintptr_t)(pAllReg->MMU_PRE_PPS_ADDR_STR), (void *)(uintptr_t)(pAllReg->MMU_PRE_PPS_ADDR_END));
        }
#endif
    }
#endif
}

//----------------------------------------
HI_VOID VeduHal_CfgReg_RCSet(VENC_REG_INFO_S *channelcfg)
{
    S_HEVC_AVC_REGS_TYPE *pAllReg = NULL;

    pAllReg = (S_HEVC_AVC_REGS_TYPE*)VeduIpCtx.pRegBase;

    pAllReg->VEDU_QPG_CU_QP_DELTA_THRESH_REG0.u32 = channelcfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG0.u32;
    pAllReg->VEDU_QPG_CU_QP_DELTA_THRESH_REG1.u32 = channelcfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG1.u32;
    pAllReg->VEDU_QPG_CU_QP_DELTA_THRESH_REG2.u32 = channelcfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG2.u32;
    pAllReg->VEDU_QPG_CU_QP_DELTA_THRESH_REG3.u32 = channelcfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG3.u32;

    {
        U_VEDU_QPG_MADI_SWITCH_THR  D32;
        D32.bits.qpg_qp_madi_switch_thr = channelcfg->all_reg.VEDU_QPG_MADI_SWITCH_THR.bits.qpg_qp_madi_switch_thr;
        pAllReg->VEDU_QPG_MADI_SWITCH_THR.u32 = D32.u32;
    }
    HI_DBG_VENC("----qpg_qp_madi_switch_thr:%d----\n", channelcfg->all_reg.VEDU_QPG_MADI_SWITCH_THR.bits.qpg_qp_madi_switch_thr);
    HI_DBG_VENC("VeduHal_CfgReg_RCSet\n");
}

HI_VOID VeduHal_CfgRegSimple(VENC_REG_INFO_S *channelcfg)
{
    S_HEVC_AVC_REGS_TYPE *pAllReg = NULL;

    pAllReg = (S_HEVC_AVC_REGS_TYPE*)VeduIpCtx.pRegBase;

    {
        U_VEDU_QPG_MAX_MIN_QP  D32;
        D32.bits.qpg_min_qp = channelcfg->all_reg.VEDU_QPG_MAX_MIN_QP.bits.qpg_min_qp;
        D32.bits.qpg_max_qp = channelcfg->all_reg.VEDU_QPG_MAX_MIN_QP.bits.qpg_max_qp;
        D32.bits.qpg_cu_qp_delta_enable_flag = 1;
        pAllReg->VEDU_QPG_MAX_MIN_QP.u32 = D32.u32;
    }
    pAllReg->VEDU_VCPI_MODE.bits.vcpi_idr_pic = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_idr_pic ;

    pAllReg->VEDU_VCPI_MODE.bits.vcpi_frame_type = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_frame_type;
    pAllReg->VEDU_VCPI_MODE.bits.vcpi_ref_num = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_ref_num;

    pAllReg->VEDU_VCPI_QPCFG.bits.vcpi_frm_qp = channelcfg->all_reg.VEDU_VCPI_QPCFG.bits.vcpi_frm_qp;
    pAllReg->VEDU_QPG_AVERAGE_LCU_BITS.bits.qpg_ave_lcu_bits = channelcfg->all_reg.VEDU_QPG_AVERAGE_LCU_BITS.bits.qpg_ave_lcu_bits;
    pAllReg->VEDU_QPG_ROW_TARGET_BITS.bits.qpg_row_target_bits = channelcfg->all_reg.VEDU_QPG_ROW_TARGET_BITS.bits.qpg_row_target_bits;
    pAllReg->VEDU_VCPI_MODE.bits.vcpi_sao_chroma = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_sao_chroma;
    pAllReg->VEDU_VCPI_MODE.bits.vcpi_sao_luma = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_sao_luma;

    VeduHal_CfgReg_RCSet(channelcfg);

    VeduHal_CfgReg_IntraSet(channelcfg);

    VeduHal_CfgReg_LambdaSet(channelcfg);

    VeduHal_CfgReg_QpgmapSet(channelcfg);

    VeduHal_CfgReg_AddrSet(channelcfg);

    VeduHal_CfgReg_SlcHeadSet(channelcfg);

    if (HI_TRUE != g_vencSmmuSetFlag)
    {
        VeduHal_CfgReg_SMMUSet();
        g_vencSmmuSetFlag = HI_TRUE;
    }

    VeduHal_CfgReg_PREMMUSet(channelcfg);

}

HI_VOID VeduHal_CfgReg(VENC_REG_INFO_S *channelcfg)
{
    S_HEVC_AVC_REGS_TYPE *pAllReg = NULL;
    HI_U32 vcpi_protocol = 0;

    pAllReg = (S_HEVC_AVC_REGS_TYPE*)VeduIpCtx.pRegBase;
    vcpi_protocol = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_protocol;

    //nosec reg
    {
        U_VEDU_VCPI_INTMASK  D32;
        D32.bits.vcpi_enable_ve_eop = 1;
        D32.bits.vcpi_enable_vedu_slice_end = 1;
        D32.bits.vcpi_enable_ve_buffull = 1;
        D32.bits.vcpi_enable_ve_pbitsover = 0;
        D32.bits.vcpi_enable_vedu_brkpt = 0;
        D32.bits.vcpi_enable_vedu_step = 0;
        D32.bits.vcpi_enable_vedu_timeout = 1;
        D32.bits.vcpi_enable_cfg_err = 0;
        pAllReg->VEDU_VCPI_INTMASK.u32 = D32.u32;
    }

    {
        U_VEDU_VCPI_INTCLR  D32;
        D32.bits.vcpi_clr_ve_eop = 0;
        D32.bits.vcpi_clr_vedu_slice_end = 0;
        D32.bits.vcpi_clr_ve_buffull = 0;
        D32.bits.vcpi_clr_ve_pbitsover = 0;
        D32.bits.vcpi_clr_vedu_brkpt = 0;
        D32.bits.vcpi_clr_vedu_step = 0;
        D32.bits.vcpi_clr_vedu_timeout = 0;
        D32.bits.vcpi_clr_cfg_err = 0;
        pAllReg->VEDU_VCPI_INTCLR.u32 = D32.u32;
    }

    {
        pAllReg->VEDU_VCPI_CNTCLR.u32 = 0;
    }

    {
        pAllReg->VEDU_VCPI_FRAMENO = 0;
    }

    {
        U_VEDU_VCPI_BP_POS  D32;
        D32.bits.vcpi_bp_lcu_x = 0;
        D32.bits.vcpi_bp_lcu_y = 0;
        D32.bits.vcpi_bkp_en = 0;
        D32.bits.vcpi_dbgmod = 0;
        pAllReg->VEDU_VCPI_BP_POS.u32 = D32.u32;
    }

    {
        pAllReg->VEDU_VCPI_TIMEOUT = 0;
    }
    {
        U_VEDU_VCPI_MODE  D32;
        D32.bits.vcpi_vedsel = 0;
        D32.bits.vcpi_lcu_time_sel = 1;
        D32.bits.vcpi_protocol = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_protocol;
        D32.bits.vcpi_cfg_mode = 0;
        D32.bits.vcpi_slice_int_en = 1;
        D32.bits.vcpi_sao_luma = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_sao_luma;
        D32.bits.vcpi_sao_chroma = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_sao_chroma;
        D32.bits.vcpi_rec_cmp_en = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_rec_cmp_en;
        D32.bits.vcpi_img_improve_en = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_img_improve_en;
        D32.bits.vcpi_frame_type = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_frame_type;
        D32.bits.vcpi_entropy_mode = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_entropy_mode;
        D32.bits.vcpi_ref_num = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_ref_num;
        D32.bits.vcpi_idr_pic = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_idr_pic;
        D32.bits.vcpi_pskip_en = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_pskip_en;
        D32.bits.vcpi_trans_mode = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_trans_mode;
        D32.bits.vcpi_sobel_weight_en = 0;
        D32.bits.vcpi_lcu_size = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_lcu_size;
        D32.bits.vcpi_time_en = 0;
        D32.bits.vcpi_ref_cmp_en = channelcfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_ref_cmp_en;
        D32.bits.vcpi_refc_nload = 0;
        pAllReg->VEDU_VCPI_MODE.u32 = D32.u32;
    }

    {
        pAllReg->VEDU_VCPI_TILE_SIZE.u32 = 0;
    }
    {
        pAllReg->VEDU_VCPI_PICSIZE_PIX.u32 = channelcfg->all_reg.VEDU_VCPI_PICSIZE_PIX.u32;
    }

    {
        U_VEDU_VCPI_MULTISLC  D32;
        D32.bits.vcpi_slice_size = channelcfg->all_reg.VEDU_VCPI_MULTISLC.bits.vcpi_slice_size;
        D32.bits.vcpi_slcspilt_mod = 1;
        D32.bits.vcpi_multislc_en = channelcfg->all_reg.VEDU_VCPI_MULTISLC.bits.vcpi_multislc_en;
        pAllReg->VEDU_VCPI_MULTISLC.u32 = D32.u32;
    }

    {
        pAllReg->VEDU_VCPI_QPCFG.u32 = channelcfg->all_reg.VEDU_VCPI_QPCFG.u32;
    }

    {
        pAllReg->VEDU_VCPI_DBLKCFG.u32 = channelcfg->all_reg.VEDU_VCPI_DBLKCFG.u32;
    }

    {

        pAllReg->VEDU_VCPI_LOW_POWER.u32 = channelcfg->all_reg.VEDU_VCPI_LOW_POWER.u32;
    }

    {
        U_VEDU_VCPI_OUTSTD  D32;
        D32.bits.vcpi_r_outstanding = DIST_PROTOCOL(vcpi_protocol, 0x0F, 0x1F);
        D32.bits.vcpi_w_outstanding = DIST_PROTOCOL(vcpi_protocol, 0x3, 0x7);
        pAllReg->VEDU_VCPI_OUTSTD.u32 = D32.u32;
    }

    {
        pAllReg->VEDU_VCPI_TMV_LOAD.bits.vcpi_tmv_wr_rd_avail = 3;
    }
    {
        pAllReg->VEDU_VCPI_CROSS_TILE_SLC.u32 = channelcfg->all_reg.VEDU_VCPI_CROSS_TILE_SLC.u32;
    }

    {
        U_VEDU_VCPI_MEM_CTRL  D32;
        D32.bits.vcpi_ema = 3;
        D32.bits.vcpi_emaw = 1;
        D32.bits.vcpi_emaa = 3;
        D32.bits.vcpi_emab = 3;
        pAllReg->VEDU_VCPI_MEM_CTRL.u32 = D32.u32;
    }

    {
        U_VEDU_VCPI_INTRA_INTER_CU_EN  D32;
        D32.bits.vcpi_intra_cu_en = channelcfg->all_reg.VEDU_VCPI_INTRA_INTER_CU_EN.bits.vcpi_intra_cu_en;
        D32.bits.vcpi_ipcm_en = 1;
        D32.bits.vcpi_intra_h264_cutdiag = DIST_PROTOCOL(vcpi_protocol, 0, 1);
        D32.bits.vcpi_fme_cu_en = channelcfg->all_reg.VEDU_VCPI_INTRA_INTER_CU_EN.bits.vcpi_fme_cu_en;
        D32.bits.vcpi_mrg_cu_en = channelcfg->all_reg.VEDU_VCPI_INTRA_INTER_CU_EN.bits.vcpi_mrg_cu_en;
        pAllReg->VEDU_VCPI_INTRA_INTER_CU_EN.u32 = D32.u32;
    }

    {
        U_VEDU_VCPI_PRE_JUDGE_EXT_EN  D32;
        D32.bits.vcpi_iblk_pre_en = channelcfg->all_reg.VEDU_VCPI_PRE_JUDGE_EXT_EN.bits.vcpi_iblk_pre_en;
        D32.bits.vcpi_pblk_pre_en = channelcfg->all_reg.VEDU_VCPI_PRE_JUDGE_EXT_EN.bits.vcpi_pblk_pre_en;
        D32.bits.vcpi_force_inter = 0;
        D32.bits.vcpi_pintra_inter_flag_disable = 0;
        D32.bits.vcpi_ext_edge_en = DIST_PROTOCOL(vcpi_protocol, 0, 1);
        pAllReg->VEDU_VCPI_PRE_JUDGE_EXT_EN.u32 = D32.u32;
    }

    {
        U_VEDU_VCPI_PRE_JUDGE_COST_THR  D32;
        D32.bits.vcpi_iblk_pre_cost_thr = DIST_PROTOCOL(vcpi_protocol, 0x1F4, 0x100);
        D32.bits.vcpi_pblk_pre_cost_thr = 0x64;
        pAllReg->VEDU_VCPI_PRE_JUDGE_COST_THR.u32 = D32.u32;
    }

    {
        U_VEDU_VCPI_IBLK_PRE_MV_THR  D32;
        D32.bits.vcpi_iblk_pre_mv_dif_thr0 = DIST_PROTOCOL(vcpi_protocol, 0xF, 0);
        D32.bits.vcpi_iblk_pre_mv_dif_thr1 = DIST_PROTOCOL(vcpi_protocol, 0xF, 0);
        D32.bits.vcpi_iblk_pre_mvx_thr = DIST_PROTOCOL(vcpi_protocol, 5, 0xC0);
        D32.bits.vcpi_iblk_pre_mvy_thr = DIST_PROTOCOL(vcpi_protocol, 5, 0xA0);
        pAllReg->VEDU_VCPI_IBLK_PRE_MV_THR.u32 = D32.u32;
    }

    {
        U_VEDU_VCPI_PME_PARAM  D32;
        D32.bits.vcpi_move_sad_en = 0;
        D32.bits.vcpi_pblk_pre_mvx_thr = DIST_PROTOCOL(vcpi_protocol, 2, 0x10);
        D32.bits.vcpi_pblk_pre_mvy_thr = DIST_PROTOCOL(vcpi_protocol, 2, 0x10);
        pAllReg->VEDU_VCPI_PME_PARAM.u32 = D32.u32;
    }

    {
        U_VEDU_VCPI_PIC_STRONG_EN  D32;
        D32.bits.vcpi_skin_en = channelcfg->all_reg.VEDU_VCPI_PIC_STRONG_EN.bits.vcpi_skin_en;
        D32.bits.vcpi_strong_edge_en = channelcfg->all_reg.VEDU_VCPI_PIC_STRONG_EN.bits.vcpi_strong_edge_en;
        D32.bits.vcpi_still_en = 0;
        D32.bits.vcpi_skin_close_angle = 0;
        D32.bits.vcpi_rounding_sobel_en = 0;
        pAllReg->VEDU_VCPI_PIC_STRONG_EN.u32 = D32.u32;
    }
    {
        U_VEDU_VCPI_CLKDIV_ENABLE  D32;
        D32.bits.vcpi_clkdiv_en = 0;
        D32.bits.vcpi_down_freq_en = 0;
        pAllReg->VEDU_VCPI_CLKDIV_ENABLE.u32 = D32.u32;//channelcfg->all_reg.VEDU_VCPI_CLKDIV_ENABLE.u32;
    }

    {
        pAllReg->VEDU_VCPI_SW_L0_SIZE.u32 = channelcfg->all_reg.VEDU_VCPI_SW_L0_SIZE.u32;
    }

    {
        pAllReg->VEDU_VCPI_SW_L1_SIZE.u32 = channelcfg->all_reg.VEDU_VCPI_SW_L1_SIZE.u32;
    }
    {
        pAllReg->VEDU_VCPI_ORI_PICSIZE.u32 = channelcfg->all_reg.VEDU_VCPI_ORI_PICSIZE.u32;
    }

    {
        pAllReg->VEDU_VCTRL_LCU_TARGET_BIT.bits.vctrl_lcu_target_bit = 0x64;
    }

    {
        pAllReg->VEDU_VCTRL_NARROW_THRESHOLD.bits.vctrl_narrow_tile_width = 3;
    }

    {
        U_VEDU_VCTRL_LCU_BASELINE  D32;
        D32.bits.vctrl_lcu_performance_baseline = DIST_PROTOCOL(vcpi_protocol, 0x186A, 0x1806);
        pAllReg->VEDU_VCTRL_LCU_BASELINE.u32 = D32.u32;
    }
    {
        U_VEDU_VCTRL_NORM_TR16X16_COEFF_DENOISE  D32;
        D32.bits.vctrl_norm16_coeff_protect_num = DIST_PROTOCOL(vcpi_protocol, 0x40, 0);
        D32.bits.vctrl_norm16_tr1_denois_max_num = DIST_PROTOCOL(vcpi_protocol, 0x3, 0);
        D32.bits.vctrl_roundingoffset16x16 = DIST_PROTOCOL(vcpi_protocol, 0x7, 0);
        pAllReg->VEDU_VCTRL_NORM_TR16X16_COEFF_DENOISE.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_NORM_COEFF_DENOISE  D32;
        D32.bits.vctrl_roundingmechanism = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.vctrl_roundingdegreethresh = DIST_PROTOCOL(vcpi_protocol, 0x21, 0);
        D32.bits.vctrl_roundingforcezeroresidthresh = DIST_PROTOCOL(vcpi_protocol, 0x4, 0);
        D32.bits.vctrl_roundingac16sum = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_roundinglowfreqacblk16 = DIST_PROTOCOL(vcpi_protocol, 0x4, 0);
        pAllReg->VEDU_VCTRL_NORM_COEFF_DENOISE.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_NORM_ENG_DENOISE  D32;
        D32.bits.vctrl_norm_isolate_ac_enable = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.vctrl_norm_force_zero_cnt = DIST_PROTOCOL(vcpi_protocol, 0x2, 0);
        D32.bits.vctrl_norm_engsum_16 = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_norm_engcnt_16 = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        pAllReg->VEDU_VCTRL_NORM_ENG_DENOISE.u32 = D32.u32;
    }
    {
        U_VEDU_VCTRL_SKIN_TR16X16_COEFF_DENOISE  D32;
        D32.bits.vctrl_skin16_coeff_protect_num = DIST_PROTOCOL(vcpi_protocol, 0x10, 0);
        D32.bits.vctrl_skin16_tr1_denois_max_num = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_roundingskinoffset16x16 = DIST_PROTOCOL(vcpi_protocol, 0x7, 0);
        pAllReg->VEDU_VCTRL_SKIN_TR16X16_COEFF_DENOISE.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_SKIN_COEFF_DENOISE  D32;
        D32.bits.vctrl_roundingskinmechanism = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.vctrl_roundingskindegreethresh = DIST_PROTOCOL(vcpi_protocol, 0x5A, 0);
        D32.bits.vctrl_roundingskinforcezeroresidthresh = DIST_PROTOCOL(vcpi_protocol, 0x4, 0);
        D32.bits.vctrl_roundingskinac16sum = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_roundingskinlowfreqacblk16 = DIST_PROTOCOL(vcpi_protocol, 0x4, 0);
        pAllReg->VEDU_VCTRL_SKIN_COEFF_DENOISE.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_SKIN_ENG_DENOISE  D32;
        D32.bits.vctrl_skin_isolate_ac_enable = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.vctrl_skin_force_zero_cnt = DIST_PROTOCOL(vcpi_protocol, 0x2, 0);
        D32.bits.vctrl_skin_engsum_16 = 0;
        D32.bits.vctrl_skin_engcnt_16 = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        pAllReg->VEDU_VCTRL_SKIN_ENG_DENOISE.u32 = D32.u32;
    }
    {
        U_VEDU_VCTRL_HEDGE_TR16X16_COEFF_DENOISE  D32;
        D32.bits.vctrl_hedge16_coeff_protect_num = DIST_PROTOCOL(vcpi_protocol, 0x10, 0);
        D32.bits.vctrl_hedge16_tr1_denois_max_num = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_roundingedgeoffset16x16 = DIST_PROTOCOL(vcpi_protocol, 0x7, 0);
        pAllReg->VEDU_VCTRL_HEDGE_TR16X16_COEFF_DENOISE.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_HEDGE_COEFF_DENOISE  D32;
        D32.bits.vctrl_roundingedgemechanism = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.vctrl_roundingedgedegreethresh = DIST_PROTOCOL(vcpi_protocol, 0x21, 0);
        D32.bits.vctrl_roundingedgeforcezeroresidthresh = DIST_PROTOCOL(vcpi_protocol, 0x4, 0);
        D32.bits.vctrl_roundingedgeac16sum = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_roundingedgelowfreqacblk16 = DIST_PROTOCOL(vcpi_protocol, 0x4, 0);
        pAllReg->VEDU_VCTRL_HEDGE_COEFF_DENOISE.u32 = D32.u32;
    }

    {
      U_VEDU_VCTRL_HEDGE_ENG_DENOISE  D32;
      D32.bits.vctrl_stredge_isolate_ac_enable = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
      D32.bits.vctrl_stredge_force_zero_cnt = DIST_PROTOCOL(vcpi_protocol, 0x2, 0);
      D32.bits.vctrl_stredge_engsum_16 = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
      D32.bits.vctrl_stredge_engcnt_16 = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        pAllReg->VEDU_VCTRL_HEDGE_ENG_DENOISE.u32 = D32.u32;
    }
    {
        U_VEDU_VCTRL_HEDGEMOV_TR16X16_COEFF_DENOISE  D32;
        D32.bits.vctrl_edgemov16_coeff_protect_num = DIST_PROTOCOL(vcpi_protocol, 0x10, 0);
        D32.bits.vctrl_edgemov16_tr1_denois_max_num = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_roundingedgemovoffset16x16 = 0;
        pAllReg->VEDU_VCTRL_HEDGEMOV_TR16X16_COEFF_DENOISE.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_HEDGEMOV_COEFF_DENOISE  D32;
        D32.bits.vctrl_roundingedgemovmechanism = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.vctrl_roundingedgemovdegreethresh = DIST_PROTOCOL(vcpi_protocol, 0x21, 0);
        D32.bits.vctrl_roundingedgemovforcezeroresidthresh = DIST_PROTOCOL(vcpi_protocol, 0x4, 0);
        D32.bits.vctrl_roundingedgemovac16sum = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_roundingedgemovlowfreqacblk16 = DIST_PROTOCOL(vcpi_protocol, 0x4, 0);
        pAllReg->VEDU_VCTRL_HEDGEMOV_COEFF_DENOISE.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_HEDGEMOV_ENG_DENOISE  D32;
        D32.bits.vctrl_edgemov_isolate_ac_enable = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.vctrl_edgemov_force_zero_cnt = DIST_PROTOCOL(vcpi_protocol, 0x2, 0);
        D32.bits.vctrl_edgemov_engsum_16 = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_edgemov_engcnt_16 = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        pAllReg->VEDU_VCTRL_HEDGEMOV_ENG_DENOISE.u32 = D32.u32;
    }
    {
        U_VEDU_VCTRL_STATIC_TR16X16_COEFF_DENOISE  D32;
        D32.bits.vctrl_static16_coeff_protect_num = DIST_PROTOCOL(vcpi_protocol, 0x8, 0);
        D32.bits.vctrl_static16_tr1_denois_max_num = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_roundingstilloffset16x16 = DIST_PROTOCOL(vcpi_protocol, 0x7, 0);
        pAllReg->VEDU_VCTRL_STATIC_TR16X16_COEFF_DENOISE.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_STATIC_COEFF_DENOISE  D32;
        D32.bits.vctrl_roundingstillmechanism = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.vctrl_roundingstilldegreethresh = DIST_PROTOCOL(vcpi_protocol, 0x21, 0);
        D32.bits.vctrl_roundingstillforcezeroresidthresh = DIST_PROTOCOL(vcpi_protocol, 0x4, 0);
        D32.bits.vctrl_roundingstillac16sum = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_roundingstilllowfreqacblk16 = DIST_PROTOCOL(vcpi_protocol, 0x4, 0);
        pAllReg->VEDU_VCTRL_STATIC_COEFF_DENOISE.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_STATIC_ENG_DENOISE  D32;
        D32.bits.vctrl_still_isolate_ac_enable = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.vctrl_still_force_zero_cnt = DIST_PROTOCOL(vcpi_protocol, 0x2, 0);
        D32.bits.vctrl_still_engsum_16 = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_still_engcnt_16 = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        pAllReg->VEDU_VCTRL_STATIC_ENG_DENOISE.u32 = D32.u32;
    }
    {
        U_VEDU_VCTRL_SOBELSTR_TR16X16_COEFF_DENOISE  D32;
        D32.bits.vctrl_sobelstr16_coeff_protect_num = DIST_PROTOCOL(vcpi_protocol, 0x8, 0);
        D32.bits.vctrl_sobelstr16_tr1_denois_max_num = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_roundingsobelstroffset16x16 = 0;
        pAllReg->VEDU_VCTRL_SOBELSTR_TR16X16_COEFF_DENOISE.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_SOBELSTR_COEFF_DENOISE  D32;
        D32.bits.vctrl_roundingsobelstrmechanism = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.vctrl_roundingsobelstrdegreethresh = DIST_PROTOCOL(vcpi_protocol, 0x21, 0);
        D32.bits.vctrl_roundingsobelstrforcezeroresidthresh = DIST_PROTOCOL(vcpi_protocol, 0x4, 0);
        D32.bits.vctrl_roundingsobelstrlowfreqacblk16 = DIST_PROTOCOL(vcpi_protocol, 0x4, 0);
        D32.bits.vctrl_roundingsobelstrac16sum = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        pAllReg->VEDU_VCTRL_SOBELSTR_COEFF_DENOISE.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_SOBELSTR_ENG_DENOISE  D32;
        D32.bits.vctrl_sobelstr_isolate_ac_enable = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.vctrl_sobelstr_force_zero_cnt = DIST_PROTOCOL(vcpi_protocol, 0x2, 0);
        D32.bits.vctrl_sobelstr_engsum_16 = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_sobelstr_engcnt_16 = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        pAllReg->VEDU_VCTRL_SOBELSTR_ENG_DENOISE.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_SOBELWEAK_TR16X16_COEFF_DENOISE  D32;
        D32.bits.vctrl_sobelweak16_coeff_protect_num = DIST_PROTOCOL(vcpi_protocol, 0x8, 0);
        D32.bits.vctrl_sobelweak16_tr1_denois_max_num = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_roundingsobelweakoffset16x16 = 0;
        pAllReg->VEDU_VCTRL_SOBELWEAK_TR16X16_COEFF_DENOISE.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_SOBELWEAK_COEFF_DENOISE  D32;
        D32.bits.vctrl_roundingsobelweakmechanism = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.vctrl_roundingsobelweakdegreethresh = DIST_PROTOCOL(vcpi_protocol, 0x21, 0);
        D32.bits.vctrl_roundingsobelweakforcezeroresidthresh = DIST_PROTOCOL(vcpi_protocol, 0x4, 0);
        D32.bits.vctrl_roundingsobelweakac16sum = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_roundingsobelweaklowfreqacblk16 = DIST_PROTOCOL(vcpi_protocol, 0x4, 0);
        pAllReg->VEDU_VCTRL_SOBELWEAK_COEFF_DENOISE.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_SOBELWEAK_ENG_DENOISE  D32;
        D32.bits.vctrl_sobelwk_isolate_ac_enable = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.vctrl_sobelwk_force_zero_cnt = DIST_PROTOCOL(vcpi_protocol, 0x2, 0);
        D32.bits.vctrl_sobelwk_engsum_16 = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        D32.bits.vctrl_sobelwk_engcnt_16 = DIST_PROTOCOL(vcpi_protocol, 0x6, 0);
        pAllReg->VEDU_VCTRL_SOBELWEAK_ENG_DENOISE.u32 = D32.u32;
    }

    {
      U_VEDU_VCTRL_INTRA_RDO_FACTOR_0  D32;
      D32.bits.vctrl_strmov_intra_cu4_rdcost_offset = 0;
      D32.bits.vctrl_strmov_intra_cu8_rdcost_offset = 0;
      D32.bits.vctrl_strmov_intra_cu16_rdcost_offset = 0;
      D32.bits.vctrl_norm_intra_cu4_rdcost_offset = 0;
      D32.bits.vctrl_norm_intra_cu8_rdcost_offset = 0;
      D32.bits.vctrl_norm_intra_cu16_rdcost_offset = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
      pAllReg->VEDU_VCTRL_INTRA_RDO_FACTOR_0.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_INTRA_RDO_FACTOR_1  D32;
        D32.bits.vctrl_sobel_str_intra_cu4_rdcost_offset = 0;
        D32.bits.vctrl_sobel_str_intra_cu8_rdcost_offset = 0;
        D32.bits.vctrl_sobel_str_intra_cu16_rdcost_offset = 0;
        D32.bits.vctrl_skin_intra_cu4_rdcost_offset = channelcfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_1.bits.vctrl_skin_intra_cu4_rdcost_offset;
        D32.bits.vctrl_skin_intra_cu8_rdcost_offset = channelcfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_1.bits.vctrl_skin_intra_cu8_rdcost_offset;
        D32.bits.vctrl_skin_intra_cu16_rdcost_offset = channelcfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_1.bits.vctrl_skin_intra_cu16_rdcost_offset;
        pAllReg->VEDU_VCTRL_INTRA_RDO_FACTOR_1.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_INTRA_RDO_FACTOR_2  D32;
        D32.bits.vctrl_sobel_tex_intra_cu4_rdcost_offset = 0;
        D32.bits.vctrl_sobel_tex_intra_cu8_rdcost_offset = 0;
        D32.bits.vctrl_sobel_tex_intra_cu16_rdcost_offset = 0;
        D32.bits.vctrl_hedge_intra_cu4_rdcost_offset = channelcfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_2.bits.vctrl_hedge_intra_cu4_rdcost_offset;
        D32.bits.vctrl_hedge_intra_cu8_rdcost_offset = channelcfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_2.bits.vctrl_hedge_intra_cu8_rdcost_offset;
        D32.bits.vctrl_hedge_intra_cu16_rdcost_offset = channelcfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_2.bits.vctrl_hedge_intra_cu16_rdcost_offset;
        pAllReg->VEDU_VCTRL_INTRA_RDO_FACTOR_2.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_MRG_RDO_FACTOR_0  D32;
        D32.bits.vctrl_norm_mrg_cu8_rdcost_offset = 0;
        D32.bits.vctrl_norm_mrg_cu16_rdcost_offset = 0;
        D32.bits.vctrl_norm_mrg_cu32_rdcost_offset = 0;
        D32.bits.vctrl_strmov_mrg_cu8_rdcost_offset = 0;
        D32.bits.vctrl_strmov_mrg_cu16_rdcost_offset = 0;
        D32.bits.vctrl_strmov_mrg_cu32_rdcost_offset = 0;
        pAllReg->VEDU_VCTRL_MRG_RDO_FACTOR_0.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_MRG_RDO_FACTOR_1  D32;
        D32.bits.vctrl_skin_mrg_cu8_rdcost_offset = 0;
        D32.bits.vctrl_skin_mrg_cu16_rdcost_offset = 0;
        D32.bits.vctrl_skin_mrg_cu32_rdcost_offset = 0;
        D32.bits.vctrl_sobel_str_mrg_cu8_rdcost_offset = 0;
        D32.bits.vctrl_sobel_str_mrg_cu16_rdcost_offset = 0;
        D32.bits.vctrl_sobel_str_mrg_cu32_rdcost_offset = 0;
        pAllReg->VEDU_VCTRL_MRG_RDO_FACTOR_1.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_MRG_RDO_FACTOR_2  D32;
        D32.bits.vctrl_hedge_mrg_cu8_rdcost_offset = 0;
        D32.bits.vctrl_hedge_mrg_cu16_rdcost_offset = 0;
        D32.bits.vctrl_hedge_mrg_cu32_rdcost_offset = 0;
        D32.bits.vctrl_sobel_tex_mrg_cu8_rdcost_offset = 0;
        D32.bits.vctrl_sobel_tex_mrg_cu16_rdcost_offset = 0;
        D32.bits.vctrl_sobel_tex_mrg_cu32_rdcost_offset = 0;
        pAllReg->VEDU_VCTRL_MRG_RDO_FACTOR_2.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_FME_RDO_FACTOR_0  D32;
        D32.bits.vctrl_norm_fme_cu8_rdcost_offset = 0;
        D32.bits.vctrl_norm_fme_cu16_rdcost_offset = 0;
        D32.bits.vctrl_norm_fme_cu32_rdcost_offset = 0;
        D32.bits.vctrl_strmov_fme_cu8_rdcost_offset = 0;
        D32.bits.vctrl_strmov_fme_cu16_rdcost_offset = 0;
        D32.bits.vctrl_strmov_fme_cu32_rdcost_offset = 0;
        pAllReg->VEDU_VCTRL_FME_RDO_FACTOR_0.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_FME_RDO_FACTOR_1  D32;
        D32.bits.vctrl_skin_fme_cu8_rdcost_offset = 0;
        D32.bits.vctrl_skin_fme_cu16_rdcost_offset = 0;
        D32.bits.vctrl_skin_fme_cu32_rdcost_offset = 0;
        D32.bits.vctrl_sobel_str_fme_cu8_rdcost_offset = 0;
        D32.bits.vctrl_sobel_str_fme_cu16_rdcost_offset = 0;
        D32.bits.vctrl_sobel_str_fme_cu32_rdcost_offset = 0;
        pAllReg->VEDU_VCTRL_FME_RDO_FACTOR_1.u32 = D32.u32;
    }

    {
        U_VEDU_VCTRL_FME_RDO_FACTOR_2  D32;
        D32.bits.vctrl_hedge_fme_cu8_rdcost_offset = 0;
        D32.bits.vctrl_hedge_fme_cu16_rdcost_offset = 0;
        D32.bits.vctrl_hedge_fme_cu32_rdcost_offset = 0;
        D32.bits.vctrl_sobel_tex_fme_cu8_rdcost_offset = 0;
        D32.bits.vctrl_sobel_tex_fme_cu16_rdcost_offset = 0;
        D32.bits.vctrl_sobel_tex_fme_cu32_rdcost_offset = 0;
        pAllReg->VEDU_VCTRL_FME_RDO_FACTOR_2.u32 = D32.u32;
    }

    {
        pAllReg->VEDU_CURLD_GCFG.u32 = channelcfg->all_reg.VEDU_CURLD_GCFG.u32;
    }
    {
        pAllReg->VEDU_VCTRL_SEL_INTRA_RDO_FACTOR.u32 = channelcfg->all_reg.VEDU_VCTRL_SEL_INTRA_RDO_FACTOR.u32;
    }
    {
        pAllReg->VEDU_CURLD_NARROW_THR.u32 = channelcfg->all_reg.VEDU_CURLD_NARROW_THR.u32;
    }
    {
        pAllReg->VEDU_444TO422_PARAMETER.u32 = channelcfg->all_reg.VEDU_444TO422_PARAMETER.u32;
    }
    {
        pAllReg->VEDU_422TO420_PARAMETER.u32 = channelcfg->all_reg.VEDU_422TO420_PARAMETER.u32;
    }
    {
        pAllReg->VEDU_ARGBTO444_PARAMETER0.u32 = channelcfg->all_reg.VEDU_ARGBTO444_PARAMETER0.u32;
    }
    {
        pAllReg->VEDU_ARGBTO444_PARAMETER1.u32 = channelcfg->all_reg.VEDU_ARGBTO444_PARAMETER1.u32;
    }
    {
        pAllReg->VEDU_ARGBTO444_PARAMETER2.u32 = channelcfg->all_reg.VEDU_ARGBTO444_PARAMETER2.u32;
    }
    {
        pAllReg->VEDU_ARGBTO444_PARAMETER3.u32 = channelcfg->all_reg.VEDU_ARGBTO444_PARAMETER3.u32;
    }
    {
        pAllReg->VEDU_ARGBTO444_PARAMETER4.u32 = channelcfg->all_reg.VEDU_ARGBTO444_PARAMETER4.u32;
    }
    {
        pAllReg->VEDU_ARGBTO444_PARAMETER5.u32 = channelcfg->all_reg.VEDU_ARGBTO444_PARAMETER5.u32;
    }
    {
        pAllReg->VEDU_ARGBTO444_PARAMETER6.u32 = channelcfg->all_reg.VEDU_ARGBTO444_PARAMETER6.u32;
    }

    {
        U_VEDU_CURLD_ARGB_CLIP  D32;
        D32.bits.vcpi_rgb_clpmin = 16;
        D32.bits.vcpi_rgb_clpmax = 235;
        D32.bits.vcpi_rgb_clip_en = 0;
        pAllReg->VEDU_CURLD_ARGB_CLIP.u32 = D32.u32;
    }

    {
        U_VEDU_PME_SW_ADAPT_EN  D32;
        D32.bits.pme_l0_psw_adapt_en = 0;
        pAllReg->VEDU_PME_SW_ADAPT_EN.u32 = D32.u32;
    }

    {
        U_VEDU_PME_SW_THR0  D32;
        D32.bits.pme_l0_psw_thr0 = 0x14;
        pAllReg->VEDU_PME_SW_THR0.u32 = D32.u32;
    }

    {
        U_VEDU_PME_SW_THR1  D32;
        D32.bits.pme_l0_psw_thr1 = 0x32;
        pAllReg->VEDU_PME_SW_THR1.u32 = D32.u32;
    }

    {
        U_VEDU_PME_SW_THR2  D32;
        D32.bits.pme_l0_psw_thr2 = 0x96;
        pAllReg->VEDU_PME_SW_THR2.u32 = D32.u32;
    }

    {
        U_VEDU_PME_SKIP_PRE  D32;
        D32.bits.pme_skipblk_pre_cost_thr = DIST_PROTOCOL(vcpi_protocol, 0x1E, 0);
        D32.bits.pme_skipblk_pre_en = 0;
        pAllReg->VEDU_PME_SKIP_PRE.u32 = D32.u32;
    }

    {
        U_VEDU_PME_TR_WEIGHTX  D32;
        D32.bits.pme_tr_weightx_0 = DIST_PROTOCOL(vcpi_protocol, 0x4, 0x10);
        D32.bits.pme_tr_weightx_1 = DIST_PROTOCOL(vcpi_protocol, 0x8, 0x20);
        D32.bits.pme_tr_weightx_2 = DIST_PROTOCOL(vcpi_protocol, 0xC, 0x30);
        pAllReg->VEDU_PME_TR_WEIGHTX.u32 = D32.u32;
    }

    {
        U_VEDU_PME_TR_WEIGHTY  D32;
        D32.bits.pme_tr_weighty_0 = DIST_PROTOCOL(vcpi_protocol, 0x4, 0x10);
        D32.bits.pme_tr_weighty_1 = DIST_PROTOCOL(vcpi_protocol, 0x8, 0x20);
        D32.bits.pme_tr_weighty_2 = DIST_PROTOCOL(vcpi_protocol, 0xC, 0x30);
        pAllReg->VEDU_PME_TR_WEIGHTY.u32 = D32.u32;
    }

    {
        U_VEDU_PME_SR_WEIGHT  D32;
        D32.bits.pme_sr_weight_0 = 6;
        D32.bits.pme_sr_weight_1 = 4;
        D32.bits.pme_sr_weight_2 = 2;
        D32.bits.pme_pskip_strongedge_madi_thr = 0x14;
        D32.bits.pme_pskip_strongedge_madi_times = 0x03;
        pAllReg->VEDU_PME_SR_WEIGHT.u32 = D32.u32;
    }

    {
        U_VEDU_PME_INTRABLK_DET  D32;
        D32.bits.pme_intrablk_det_cost_thr0 = DIST_PROTOCOL(vcpi_protocol, 0x174, 0x64);
        D32.bits.pme_pskip_mvy_consistency_thr = 0;
        D32.bits.pme_pskip_mvx_consistency_thr = 0;
        pAllReg->VEDU_PME_INTRABLK_DET.u32 = D32.u32;
    }

    {
        U_VEDU_PME_INTRABLK_DET_THR  D32;
        D32.bits.pme_intrablk_det_mv_dif_thr1 = DIST_PROTOCOL(vcpi_protocol, 0x2, 0x0F);
        D32.bits.pme_intrablk_det_mv_dif_thr0 = DIST_PROTOCOL(vcpi_protocol, 0x2, 0x0F);
        D32.bits.pme_intrablk_det_mvy_thr = DIST_PROTOCOL(vcpi_protocol, 4, 0x10);
        D32.bits.pme_intrablk_det_mvx_thr = DIST_PROTOCOL(vcpi_protocol, 4, 0x10);
        pAllReg->VEDU_PME_INTRABLK_DET_THR.u32 = D32.u32;
    }

    {
        U_VEDU_PME_SKIN_THR  D32;
        D32.bits.pme_skin_u_max_thr = 0x7F;
        D32.bits.pme_skin_u_min_thr = 0x64;
        D32.bits.pme_skin_v_max_thr = 0xA0;
        D32.bits.pme_skin_v_min_thr = 0x87;
        pAllReg->VEDU_PME_SKIN_THR.u32 = D32.u32;
    }

    {
        U_VEDU_PME_INTRA_LOWPOW  D32;
        D32.bits.pme_intra16_madi_thr = DIST_PROTOCOL(vcpi_protocol, 0xA, 0);
        D32.bits.pme_inter_first = 0;
        pAllReg->VEDU_PME_INTRA_LOWPOW.u32 = D32.u32;
    }

    {
        U_VEDU_PME_IBLK_COST_THR  D32;
        D32.bits.pme_iblk_pre_cost_thr_h264 = DIST_PROTOCOL(vcpi_protocol, 0x02FF, 0x400);
        D32.bits.pme_intrablk_det_cost_thr1 = DIST_PROTOCOL(vcpi_protocol, 0x0200, 0xC8);
        pAllReg->VEDU_PME_IBLK_COST_THR.u32 = D32.u32;
    }

    {
        U_VEDU_PME_STRONG_EDGE  D32;
        D32.bits.pme_skin_num = DIST_PROTOCOL(vcpi_protocol, 0x80, 0x30);
        D32.bits.pme_strong_edge_thr = DIST_PROTOCOL(vcpi_protocol, 0x1E, 0);
        D32.bits.pme_strong_edge_cnt = DIST_PROTOCOL(vcpi_protocol, 0x3, 0);
        D32.bits.pme_still_scene_thr = 0;
        pAllReg->VEDU_PME_STRONG_EDGE.u32 = D32.u32;
    }

    {
        U_VEDU_PME_LARGE_MOVE_THR  D32;
        D32.bits.pme_move_scene_thr = DIST_PROTOCOL(vcpi_protocol, 0x3, 0);
        D32.bits.pme_move_sad_thr = DIST_PROTOCOL(vcpi_protocol, 0x200, 0);
        pAllReg->VEDU_PME_LARGE_MOVE_THR.u32 = D32.u32;
    }

    {
        U_VEDU_PME_INTER_STRONG_EDGE  D32;
        D32.bits.pme_interdiff_max_min_madi_abs = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.pme_interdiff_max_min_madi_times = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.pme_interstrongedge_madi_thr = DIST_PROTOCOL(vcpi_protocol, 0x3C, 0);
        pAllReg->VEDU_PME_INTER_STRONG_EDGE.u32 = D32.u32;
    }

    {
        pAllReg->VEDU_PME_NEW_COST.bits.pme_cost_lamda0 = channelcfg->all_reg.VEDU_PME_NEW_COST.bits.pme_cost_lamda0;
        pAllReg->VEDU_PME_NEW_COST.bits.pme_cost_lamda1 = 0x2;
        pAllReg->VEDU_PME_NEW_COST.bits.pme_mvp3median_en = 0;
    }

    {
        pAllReg->VEDU_PME_WINDOW_SIZE0_L0.u32 = channelcfg->all_reg.VEDU_PME_WINDOW_SIZE0_L0.u32;
    }

    {
        pAllReg->VEDU_PME_WINDOW_SIZE1_L0.u32 = channelcfg->all_reg.VEDU_PME_WINDOW_SIZE1_L0.u32;
    }

    {
        pAllReg->VEDU_PME_WINDOW_SIZE2_L0.u32 = channelcfg->all_reg.VEDU_PME_WINDOW_SIZE2_L0.u32;
    }

    {
        pAllReg->VEDU_PME_WINDOW_SIZE3_L0.u32 = channelcfg->all_reg.VEDU_PME_WINDOW_SIZE3_L0.u32;
    }

    {
        pAllReg->VEDU_PME_COST_OFFSET.u32 = 0;
    }

    {
        U_VEDU_PME_SAFE_CFG  D32;
        D32.bits.pme_safe_line = channelcfg->all_reg.VEDU_PME_SAFE_CFG.bits.pme_safe_line;
        D32.bits.pme_safe_line_val = 0;
        pAllReg->VEDU_PME_SAFE_CFG.u32 = D32.u32;
    }

    {
        pAllReg->VEDU_PME_IBLK_REFRESH.u32 = 0;
    }

    {
        pAllReg->VEDU_PME_IBLK_REFRESH_NUM.u32 = 0;
    }

    {
        U_VEDU_PME_QPG_RC_THR0  D32;
        D32.bits.pme_madi_dif_thr = DIST_PROTOCOL(vcpi_protocol, 0, 0x5);
        D32.bits.pme_cur_madi_dif_thr = DIST_PROTOCOL(vcpi_protocol, 0, 0x5);
        pAllReg->VEDU_PME_QPG_RC_THR0.u32 = D32.u32;
    }

    {
        U_VEDU_PME_QPG_RC_THR1  D32;
        D32.bits.pme_min_sad_thr_offset = DIST_PROTOCOL(vcpi_protocol, 0x8, 0);
        D32.bits.pme_min_sad_thr_gain = DIST_PROTOCOL(vcpi_protocol, 0x8, 0);
        D32.bits.pme_smooth_madi_thr = DIST_PROTOCOL(vcpi_protocol, 0, 0x2);
        D32.bits.pme_min_sad_thr_offset_cur = DIST_PROTOCOL(vcpi_protocol, 0x8, 0);
        D32.bits.pme_min_sad_thr_gain_cur = DIST_PROTOCOL(vcpi_protocol, 0x8, 0);
        pAllReg->VEDU_PME_QPG_RC_THR1.u32 = D32.u32;
    }

    {
        U_VEDU_PME_LOW_LUMA_THR  D32;
        D32.bits.pme_low_luma_thr = 0;
        D32.bits.pme_low_luma_madi_thr = 0x4;
        D32.bits.pme_high_luma_thr = 0x3C;
        pAllReg->VEDU_PME_LOW_LUMA_THR.u32 = D32.u32;
    }

    {
        U_VEDU_PME_PBLK_PRE1  D32;
        D32.bits.pme_pblk_pre_mv_dif_thr1 = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.pme_pblk_pre_mv_dif_thr0 = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.pme_pblk_pre_mv_dif_cost_thr = DIST_PROTOCOL(vcpi_protocol, 0xC8, 0);
        pAllReg->VEDU_PME_PBLK_PRE1.u32 = D32.u32;
    }

    {
        U_VEDU_PME_CHROMA_FLAT  D32;
        D32.bits.pme_flat_v_thr_high = DIST_PROTOCOL(vcpi_protocol, 0x85, 0);
        D32.bits.pme_flat_v_thr_low = DIST_PROTOCOL(vcpi_protocol, 0x7D, 0);
        D32.bits.pme_flat_u_thr_high = DIST_PROTOCOL(vcpi_protocol, 0x85, 0);
        D32.bits.pme_flat_u_thr_low = DIST_PROTOCOL(vcpi_protocol, 0x7D, 0);
        pAllReg->VEDU_PME_CHROMA_FLAT.u32 = D32.u32;
    }

    {
        U_VEDU_PME_LUMA_FLAT  D32;
        D32.bits.pme_flat_pmemv_thr = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.pme_flat_luma_madi_thr = DIST_PROTOCOL(vcpi_protocol, 0x2, 0);
        D32.bits.pme_flat_low_luma_thr = DIST_PROTOCOL(vcpi_protocol, 0x28, 0);
        D32.bits.pme_flat_high_luma_thr = DIST_PROTOCOL(vcpi_protocol, 0x64, 0);
        pAllReg->VEDU_PME_LUMA_FLAT.u32 = D32.u32;
    }

    {
        U_VEDU_PME_MADI_FLAT  D32;
        D32.bits.pme_flat_pmesad_thr = DIST_PROTOCOL(vcpi_protocol, 0x40, 0);
        D32.bits.pme_flat_icount_thr = DIST_PROTOCOL(vcpi_protocol, 0xC8, 0);
        D32.bits.pme_flat_region_cnt = DIST_PROTOCOL(vcpi_protocol, 0xD, 0);
        D32.bits.pme_flat_madi_times = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        pAllReg->VEDU_PME_MADI_FLAT.u32 = D32.u32;
    }

    {
        U_VEDU_QPG_MAX_MIN_QP  D32;
        D32.bits.qpg_min_qp = channelcfg->all_reg.VEDU_QPG_MAX_MIN_QP.bits.qpg_min_qp;
        D32.bits.qpg_max_qp = channelcfg->all_reg.VEDU_QPG_MAX_MIN_QP.bits.qpg_max_qp;
        D32.bits.qpg_cu_qp_delta_enable_flag = 1;
        pAllReg->VEDU_QPG_MAX_MIN_QP.u32 = D32.u32;
    }

    {
        U_VEDU_QPG_ROW_TARGET_BITS  D32;
        D32.bits.qpg_qp_delta = channelcfg->all_reg.VEDU_QPG_ROW_TARGET_BITS.bits.qpg_qp_delta;
        D32.bits.qpg_row_target_bits = channelcfg->all_reg.VEDU_QPG_ROW_TARGET_BITS.bits.qpg_row_target_bits;//DIST_PROTOCOL(vcpi_protocol, 0xB956, 0x0);
        pAllReg->VEDU_QPG_ROW_TARGET_BITS.u32 = D32.u32;
    }

    {

        pAllReg->VEDU_QPG_AVERAGE_LCU_BITS.u32 = channelcfg->all_reg.VEDU_QPG_AVERAGE_LCU_BITS.u32;
    }

    {
        U_VEDU_QPG_LOWLUMA  D32;

        D32.bits.qpg_lowluma_min_qp = 0xA;
        D32.bits.qpg_lowluma_max_qp = 0x33;
        D32.bits.qpg_lowluma_qp_delta = 0x3;
        pAllReg->VEDU_QPG_LOWLUMA .u32 = D32.u32;
    }

    {
        U_VEDU_QPG_HEDGE  D32;
        D32.bits.qpg_hedge_min_qp = 0xA;
        D32.bits.qpg_hedge_max_qp = 0x33;
        D32.bits.qpg_hedge_qp_delta = DIST_PROTOCOL(vcpi_protocol, 0x0, 0x4);
        pAllReg->VEDU_QPG_HEDGE.u32 = D32.u32;
    }

    {
        U_VEDU_QPG_HEDGE_MOVE  D32;
        D32.bits.qpg_hedge_move_min_qp = 0xA;
        D32.bits.qpg_hedge_move_max_qp = 0x33;
        D32.bits.qpg_hedge_move_qp_delta = DIST_PROTOCOL(vcpi_protocol, 0x0, 0x5);
        pAllReg->VEDU_QPG_HEDGE_MOVE.u32 = D32.u32;
    }

    {
        U_VEDU_QPG_LARGE_MOVE  D32;
        D32.bits.qpg_large_move_min_qp = 0xA;
        D32.bits.qpg_large_move_max_qp = 0x33;
        D32.bits.qpg_large_move_qp_delta = DIST_PROTOCOL(vcpi_protocol, 0x0, 0x0);
        pAllReg->VEDU_QPG_LARGE_MOVE.u32 = D32.u32;
    }

    {
        U_VEDU_QPG_SKIN  D32;
        D32.bits.qpg_skin_min_qp = 0xA;
        D32.bits.qpg_skin_max_qp = channelcfg->all_reg.VEDU_QPG_SKIN.bits.qpg_skin_max_qp;//DIST_PROTOCOL(vcpi_protocol, 0x1E, 0x33);
        D32.bits.qpg_skin_qp_delta = channelcfg->all_reg.VEDU_QPG_SKIN.bits.qpg_skin_qp_delta;//DIST_PROTOCOL(vcpi_protocol, 0x2, 0x3);
        pAllReg->VEDU_QPG_SKIN.u32 = D32.u32;
    }

    {
        U_VEDU_QPG_INTRA_DET  D32;
        D32.bits.qpg_intra_det_min_qp = 0xA;
        D32.bits.qpg_intra_det_max_qp = 0x33;
        D32.bits.qpg_intra_det_qp_delta = DIST_PROTOCOL(vcpi_protocol, 0x4, 0x3);
        pAllReg->VEDU_QPG_INTRA_DET.u32 = D32.u32;
    }

    {
        U_VEDU_QPG_H264_SMOOTH  D32;
        D32.bits.qpg_h264_smooth_min_qp = DIST_PROTOCOL(vcpi_protocol, 0x0, 0xA);
        D32.bits.qpg_h264_smooth_max_qp = DIST_PROTOCOL(vcpi_protocol, 0, 0x2D);
        D32.bits.qpg_h264_smooth_qp_delta = DIST_PROTOCOL(vcpi_protocol, 0, 0x5);
        D32.bits.qpg_h264_smooth_qp_delta1 = DIST_PROTOCOL(vcpi_protocol, 0, 0x3);
        pAllReg->VEDU_QPG_H264_SMOOTH.u32 = D32.u32;
    }

    {
        U_VEDU_QPG_DELTA_LEVEL  D32;
        D32.bits.qpg_qp_delta_level_0 = 1;
        D32.bits.qpg_qp_delta_level_1 = 1;
        D32.bits.qpg_qp_delta_level_2 = 1;
        D32.bits.qpg_qp_delta_level_3 = 1;
        D32.bits.qpg_qp_delta_level_4 = 1;
        D32.bits.qpg_qp_delta_level_5 = 1;
        D32.bits.qpg_qp_delta_level_6 = 1;
        D32.bits.qpg_qp_delta_level_7 = 1;
        D32.bits.qpg_qp_delta_level_8 = 1;
        D32.bits.qpg_qp_delta_level_9 = 1;
        D32.bits.qpg_qp_delta_level_10 = 1;
        D32.bits.qpg_qp_delta_level_11 = 1;
        D32.bits.qpg_qp_delta_level_12 = 1;
        D32.bits.qpg_qp_delta_level_13 = 1;
        D32.bits.qpg_qp_delta_level_14 = 1;
        D32.bits.qpg_qp_delta_level_15 = 1;
        pAllReg->VEDU_QPG_DELTA_LEVEL.u32 = D32.u32;
    }

    {
        U_VEDU_QPG_CU32_DELTA  D32;
        D32.bits.qpg_cu32_delta_low = 0xC;
        D32.bits.qpg_cu32_delta_high = 0xC;
        pAllReg->VEDU_QPG_CU32_DELTA.u32 = D32.u32;
    }

    {
        U_VEDU_QPG_LAMBDA_MODE  D32;
        D32.bits.qpg_lambda_qp_offset = DIST_PROTOCOL(vcpi_protocol, 0x0, 0x1A);
        D32.bits.qpg_rdo_lambda_choose_mode = 0;
        D32.bits.qpg_lambda_inter_stredge_en = DIST_PROTOCOL(vcpi_protocol, 1, 0);
        pAllReg->VEDU_QPG_LAMBDA_MODE.u32 = D32.u32;
    }

    {
        U_VEDU_QPG_QP_RESTRAIN  D32;
        D32.bits.qpg_qp_restrain_madi_thr = DIST_PROTOCOL(vcpi_protocol, 0xA, 0x8);
        D32.bits.qpg_qp_restrain_en = DIST_PROTOCOL(vcpi_protocol, 0x1, 0);
        D32.bits.qpg_qp_restrain_mode = 0;
        D32.bits.qpg_qp_restrain_delta_blk16 = DIST_PROTOCOL(vcpi_protocol, 0x4, 1);
        D32.bits.qpg_qp_restrain_delta_blk32 = DIST_PROTOCOL(vcpi_protocol, 0x5, 3);
        pAllReg->VEDU_QPG_QP_RESTRAIN.u32 = D32.u32;
    }

    {
        pAllReg->VEDU_QPG_CU_MIN_SAD_THRESH_0 = 0x0C080401;//channelcfg->all_reg.VEDU_QPG_CU_MIN_SAD_THRESH_0;
    }

    {
        pAllReg->VEDU_QPG_CU_MIN_SAD_THRESH_1 = 0xB4825A3C;//channelcfg->all_reg.VEDU_QPG_CU_MIN_SAD_THRESH_1;
    }

    {
        U_VEDU_QPG_CU_MIN_SAD_REG  D32;
        D32.bits.qpg_min_sad_level = DIST_PROTOCOL(vcpi_protocol, 0x5555, 0x5595);
        D32.bits.qpg_low_min_sad_mode = 0;
        D32.bits.qpg_high_min_sad_mode = 0;
        D32.bits.qpg_min_sad_madi_en = DIST_PROTOCOL(vcpi_protocol, 0x1, 0x0);
        D32.bits.qpg_min_sad_qp_restrain_en = 0;
        pAllReg->VEDU_QPG_CU_MIN_SAD_REG.u32 = D32.u32;
    }

    {
        U_VEDU_QPG_FLAT_REGION  D32;
        D32.bits.qpg_flat_region_qp_delta = DIST_PROTOCOL(vcpi_protocol, 5, 0); // DIST_PROTOCOL(vcpi_protocol, 5, 5);
        D32.bits.qpg_flat_region_max_qp = 0x33;
        D32.bits.qpg_flat_region_min_qp = DIST_PROTOCOL(vcpi_protocol, 0xF, 0);
        pAllReg->VEDU_QPG_FLAT_REGION.u32 = D32.u32;
    }

    {
        U_VEDU_IME_INTER_MODE  D32;
        D32.bits.ime_inter8x8_en = DIST_PROTOCOL(vcpi_protocol, 0, 1);
        D32.bits.ime_flat_region_force_low3layer = 0;
        D32.bits.ime_high3pre_en = 0;
        pAllReg->VEDU_IME_INTER_MODE.u32 = D32.u32;
    }

    {
        pAllReg->VEDU_IME_RDOCFG.u32 = 0;
    }

    {
        U_VEDU_IME_FME_LPOW_THR  D32;
        D32.bits.ime_lowpow_fme_thr0 = 0xA;
        D32.bits.ime_lowpow_fme_thr1 = DIST_PROTOCOL(vcpi_protocol, 0x10, 0x14);
        pAllReg->VEDU_IME_FME_LPOW_THR.u32 = D32.u32;
    }

    {
        U_VEDU_MRG_FORCE_ZERO_EN  D32;
        D32.bits.mrg_force_zero_en = channelcfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.mrg_force_zero_en;
        D32.bits.mrg_force_y_zero_en = channelcfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.mrg_force_y_zero_en;
        D32.bits.mrg_force_u_zero_en = channelcfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.mrg_force_u_zero_en;
        D32.bits.mrg_force_v_zero_en = channelcfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.mrg_force_v_zero_en;
        pAllReg->VEDU_MRG_FORCE_ZERO_EN.u32 = D32.u32;
    }

    {
        pAllReg->VEDU_MRG_FORCE_SKIP_EN.u32 = 0;
    }

    {
        U_VEDU_INTRA_BIT_WEIGHT  D32;
        D32.bits.intra_bit_weight = DIST_PROTOCOL(vcpi_protocol, 0xD, 0);
        pAllReg->VEDU_INTRA_BIT_WEIGHT.u32 = D32.u32;
    }

    {
        pAllReg->VEDU_INTRA_RDO_COST_OFFSET.u32 = channelcfg->all_reg.VEDU_INTRA_RDO_COST_OFFSET.u32;
    }

    {
        pAllReg->VEDU_INTRA_CHNL4_ANG_0EN.u32 = DIST_PROTOCOL(vcpi_protocol, (0xffffffff | 0x2), (0xffffffff | 0x4));
    }

    {
        pAllReg->VEDU_INTRA_CHNL8_ANG_0EN.u32 = DIST_PROTOCOL(vcpi_protocol, (0xffffffff | 0x2), (0xffffffff | 0x4));
        pAllReg->VEDU_INTRA_CHNL8_ANG_1EN.u32 = (7 & 0x7);
    }

    {
        pAllReg->VEDU_INTRA_CHNL16_ANG_0EN.u32 = DIST_PROTOCOL(vcpi_protocol, (0xffffffff | 0x2), (0xffffffff | 0x4));
        pAllReg->VEDU_INTRA_CHNL16_ANG_1EN.u32 = (7 & 0x7);
    }

    if (vcpi_protocol == VEDU_H265)
    {
        pAllReg->VEDU_IME_NO_INTRA_ANG_EN.u32 = channelcfg->all_reg.VEDU_IME_NO_INTRA_ANG_EN.u32;
        pAllReg->VEDU_IME_NO_INTRA_ANG_MV_THR.u32 = channelcfg->all_reg.VEDU_IME_NO_INTRA_ANG_MV_THR.u32;
    }
    else if (vcpi_protocol == VEDU_H264)
    {
        pAllReg->VEDU_INTRA_DC_SHIFT.u32 = channelcfg->all_reg.VEDU_INTRA_DC_SHIFT.u32;
        pAllReg->VEDU_INTRA_DC_MADI.u32 = channelcfg->all_reg.VEDU_INTRA_DC_MADI.u32;
        pAllReg->VEDU_INTRA_CHROMA_ANG_EN.u32 = channelcfg->all_reg.VEDU_INTRA_CHROMA_ANG_EN.u32;
    }

    {
        pAllReg->VEDU_INTRA_INTER_AVAIL.u32 = channelcfg->all_reg.VEDU_INTRA_INTER_AVAIL.u32;
    }

    {
        U_VEDU_PMV_TMV_EN  D32;
        D32.bits.pmv_tmv_en = DIST_PROTOCOL(vcpi_protocol, 1, 0);
        pAllReg->VEDU_PMV_TMV_EN.u32 = D32.u32;
    }

    {
        U_VEDU_TQITQ_DEADZONE  D32;
        D32.bits.tqitq_deadzone_intra_slice = 0xAB;
        D32.bits.tqitq_deadzone_inter_slice = 0x55;
        pAllReg->VEDU_TQITQ_DEADZONE.u32 = D32.u32;
    }

    {
        U_VEDU_SEL_OFFSET_STRENGTH  D32;
        D32.bits.sel_offset_strength = DIST_PROTOCOL(vcpi_protocol, 0, 2);
        pAllReg->VEDU_SEL_OFFSET_STRENGTH.u32 = D32.u32;
    }

    {
        U_VEDU_SEL_CU32_DC_AC_TH_OFFSET  D32;
        D32.bits.sel_cu32_dc_ac_th_offset = DIST_PROTOCOL(vcpi_protocol, 0, 1);
        pAllReg->VEDU_SEL_CU32_DC_AC_TH_OFFSET.u32 = D32.u32;
    }

    {
        U_VEDU_SEL_CU32_QP_TH  D32;
        D32.bits.sel_cu32_qp0_th = DIST_PROTOCOL(vcpi_protocol, 0x0, 0x26);
        D32.bits.sel_cu32_qp1_th = DIST_PROTOCOL(vcpi_protocol, 0x0, 0x21);
        pAllReg->VEDU_SEL_CU32_QP_TH.u32 = D32.u32;
    }

    {
        U_VEDU_SEL_RES_DC_AC_TH  D32;
        D32.bits.sel_res16_luma_dc_th = DIST_PROTOCOL(vcpi_protocol, 0, 3);
        D32.bits.sel_res16_chroma_dc_th = DIST_PROTOCOL(vcpi_protocol, 0, 2);
        D32.bits.sel_res16_luma_ac_th = DIST_PROTOCOL(vcpi_protocol, 0, 3);
        D32.bits.sel_res16_chroma_ac_th = DIST_PROTOCOL(vcpi_protocol, 0, 2);
        D32.bits.sel_res32_luma_dc_th = DIST_PROTOCOL(vcpi_protocol, 0, 4);
        D32.bits.sel_res32_chroma_dc_th = DIST_PROTOCOL(vcpi_protocol, 0, 3);
        D32.bits.sel_res32_luma_ac_th = DIST_PROTOCOL(vcpi_protocol, 0, 4);
        D32.bits.sel_res32_chroma_ac_th = DIST_PROTOCOL(vcpi_protocol, 0, 3);
        pAllReg->VEDU_SEL_RES_DC_AC_TH.u32 = D32.u32;
    }

    {
        U_VEDU_EMAR_WAIT_TIM_OUT  D32;
        D32.bits.vcpi_wtmax = 0xFF;
        D32.bits.vcpi_rtmax = 0xFF;
        pAllReg->VEDU_EMAR_WAIT_TIM_OUT.u32 = D32.u32;
    }

    {
        U_VEDU_EMAR_RCH_RPT_TH0  D32;
        D32.bits.vcpi_ch00_rrmax = 0xF;
        D32.bits.vcpi_ch01_rrmax = 0xF;
        D32.bits.vcpi_ch02_rrmax = 0xF;
        D32.bits.vcpi_ch03_rrmax = 0xF;
        D32.bits.vcpi_ch04_rrmax = 0xF;
        D32.bits.vcpi_ch05_rrmax = 0xF;
        pAllReg->VEDU_EMAR_RCH_RPT_TH0.u32 = D32.u32;
    }

    {
        U_VEDU_EMAR_RCH_RPT_TH1  D32;
        D32.bits.vcpi_ch06_rrmax = 0xF;
        D32.bits.vcpi_ch07_rrmax = 0xF;
        D32.bits.vcpi_ch08_rrmax = 0xF;
        D32.bits.vcpi_ch09_rrmax = 0xF;
        D32.bits.vcpi_ch10_rrmax = 0xF;
        D32.bits.vcpi_ch11_rrmax = 0xF;
        pAllReg->VEDU_EMAR_RCH_RPT_TH1.u32 = D32.u32;
    }

    {
        U_VEDU_EMAR_RCH_RPT_TH2  D32;
        D32.bits.vcpi_ch12_rrmax = 0xF;
        pAllReg->VEDU_EMAR_RCH_RPT_TH2.u32 = D32.u32;
    }

    {
        U_VEDU_EMAR_WCH_RPT_TH0  D32;
        D32.bits.vcpi_ch00_wrmax = 0xF;
        D32.bits.vcpi_ch01_wrmax = 0xF;
        D32.bits.vcpi_ch02_wrmax = 0xF;
        D32.bits.vcpi_ch03_wrmax = 0xF;
        D32.bits.vcpi_ch04_wrmax = 0xF;
        D32.bits.vcpi_ch05_wrmax = 0xF;
        pAllReg->VEDU_EMAR_WCH_RPT_TH0.u32 = D32.u32;
    }

    {
        U_VEDU_EMAR_WCH_RPT_TH1  D32;
        D32.bits.vcpi_ch06_wrmax = 0xF;
        D32.bits.vcpi_ch07_wrmax = 0xF;
        D32.bits.vcpi_ch08_wrmax = 0xF;
        D32.bits.vcpi_ch09_wrmax = 0xF;
        D32.bits.vcpi_ch10_wrmax = 0xF;
        D32.bits.vcpi_ch11_wrmax = 0xF;
        pAllReg->VEDU_EMAR_WCH_RPT_TH1.u32 = D32.u32;
    }

    {
        U_VEDU_EMAR_WCH_RPT_TH2  D32;
        D32.bits.vcpi_ch12_wrmax = 0xF;
        D32.bits.vcpi_ch13_wrmax = 0xF;
        D32.bits.vcpi_ch14_wrmax = 0xF;
        pAllReg->VEDU_EMAR_WCH_RPT_TH2.u32 = D32.u32;
    }

    {
        U_VEDU_VLCST_PTBITS_EN  D32;
        D32.bits.vlcst_ptbits_en = 0;
        pAllReg->VEDU_VLCST_PTBITS_EN.u32 = D32.u32;
    }

    {
        pAllReg->VEDU_VLCST_PTBITS = 0;
    }

    VeduHal_CfgReg_RCSet(channelcfg);

    VeduHal_CfgReg_IntraSet(channelcfg);

    VeduHal_CfgReg_LambdaSet(channelcfg);

    VeduHal_CfgReg_QpgmapSet(channelcfg);

    VeduHal_CfgReg_AddrSet(channelcfg);

    VeduHal_CfgReg_SlcHeadSet(channelcfg);

    //------------------------------MMU MAIN REG CFG---------------------------------------------
    //MMU ABOUT SETTING
    ////MST INT_NS

    if (HI_TRUE != g_vencSmmuSetFlag)
    {
        VeduHal_CfgReg_SMMUSet();
        g_vencSmmuSetFlag = HI_TRUE;
    }

    VeduHal_CfgReg_PREMMUSet(channelcfg);

}

