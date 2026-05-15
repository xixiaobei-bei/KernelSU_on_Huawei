#include "vfmw_dts.h"
#include "sysconfig.h"
#include "public.h"

UINT32  gIsFPGA              = 0;
UINT32  gVdhRegBaseAddr      = 0;
UINT32  gScdRegBaseAddr      = 0;
UINT32  gBpdRegBaseAddr      = 0;
UINT32  gVdhRegRange         = 0;
UINT32  gSOFTRST_REQ_Addr    = 0;
UINT32  gSOFTRST_OK_ADDR     = 0;
UINT64  gSmmuPageBase        = 0;
UINT32  gPERICRG_RegBaseAddr = 0;
UINT32  gVdecQosMode         = 0;

/* irq num */
UINT32  gVdecIrqNumNorm           = 0;
UINT32  gVdecIrqNumProt           = 0;
UINT32  gVdecIrqNumSafe           = 0;

SINT32 VFMW_SetDtsConfig(VFMW_DTS_CONFIG_S *pDtsConfig)
{
	if (pDtsConfig == NULL) {
		dprint(PRN_ERROR, "%s : pDtsConfig is NULL\n", __func__);
		return VDEC_ERR;
	}

	if (pDtsConfig->VdecIrqNumNorm == 0 || pDtsConfig->VdecIrqNumProt == 0 || pDtsConfig->VdecIrqNumSafe == 0   ||
	    pDtsConfig->VdhRegBaseAddr == 0 || pDtsConfig->VdhRegRange == 0    || pDtsConfig->SmmuPageBaseAddr == 0 ||
	    pDtsConfig->PERICRG_RegBaseAddr == 0) {
		dprint(PRN_ERROR, "%s invalid param: IsFPGA : %d, VdecIrqNumNorm : %d\n", __func__, pDtsConfig->IsFPGA, pDtsConfig->VdecIrqNumNorm);
             dprint(PRN_ERROR,"VdecIrqNumProt : %d, VdecIrqNumSafe : %d, VdhRegBaseAddr : %pK\n", pDtsConfig->VdecIrqNumProt, pDtsConfig->VdecIrqNumSafe, (void *)(uintptr_t)(pDtsConfig->VdhRegBaseAddr));
             dprint(PRN_ERROR,"VdhRegSize : %d, SmmuPageBaseAddr : %pK, PERICRG_RegBaseAddr : %pK\n", pDtsConfig->VdhRegRange, (void *)(uintptr_t)(pDtsConfig->SmmuPageBaseAddr), (void *)(uintptr_t)(pDtsConfig->PERICRG_RegBaseAddr));
		return VDEC_ERR;
	}

	gIsFPGA              = pDtsConfig->IsFPGA;
	gVdecIrqNumNorm      = pDtsConfig->VdecIrqNumNorm;
	gVdecIrqNumProt      = pDtsConfig->VdecIrqNumProt;
	gVdecIrqNumSafe      = pDtsConfig->VdecIrqNumSafe;

	gVdhRegBaseAddr      = pDtsConfig->VdhRegBaseAddr;
	gVdhRegRange         = pDtsConfig->VdhRegRange;
	gSmmuPageBase        = pDtsConfig->SmmuPageBaseAddr;
	gPERICRG_RegBaseAddr = pDtsConfig->PERICRG_RegBaseAddr;
	gVdecQosMode         = pDtsConfig->VdecQosMode;

	gScdRegBaseAddr      = gVdhRegBaseAddr + SCD_REG_OFFSET;
	gBpdRegBaseAddr      = gVdhRegBaseAddr + BPD_REG_OFFSET;
	gSOFTRST_REQ_Addr    = gVdhRegBaseAddr + SOFTRST_REQ_OFFSET;
	gSOFTRST_OK_ADDR     = gVdhRegBaseAddr + SOFTRST_OK_OFFSET;

	return VDEC_OK;
}

SINT32 VFMW_GetDtsConfig(VFMW_DTS_CONFIG_S *pDtsConfig)
{
	if (pDtsConfig == NULL) {
		dprint(PRN_ERROR, "%s FATAL: pDtsConfig is NULL\n", __func__);
		return VDEC_ERR;
	}

	pDtsConfig->IsFPGA              = gIsFPGA;
	pDtsConfig->VdecIrqNumNorm      = gVdecIrqNumNorm;
	pDtsConfig->VdecIrqNumProt      = gVdecIrqNumProt;
	pDtsConfig->VdecIrqNumSafe      = gVdecIrqNumSafe;

	pDtsConfig->VdhRegBaseAddr      = gVdhRegBaseAddr;
	pDtsConfig->VdhRegRange         = gVdhRegRange;
	pDtsConfig->SmmuPageBaseAddr    = gSmmuPageBase;

	pDtsConfig->PERICRG_RegBaseAddr = gPERICRG_RegBaseAddr;

	return VDEC_OK;
}

#ifdef ENV_ARMLINUX_KERNEL
EXPORT_SYMBOL(VFMW_SetDtsConfig);
#endif
