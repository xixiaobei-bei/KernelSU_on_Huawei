#ifndef __VDEC_REGULATOR_H__
#define __VDEC_REGULATOR_H__

#include "hi_type.h"
#include "vfmw_dts.h"

typedef enum {
#ifdef LOWER_FREQUENCY_SUPPORT
	VDEC_CLK_RATE_LOWER = 0,
#endif
	VDEC_CLK_RATE_LOW,
	VDEC_CLK_RATE_NORMAL,
	VDEC_CLK_RATE_HIGH,
	VDEC_CLK_RATE_BUTT,
}CLK_RATE_E;

#ifdef PLATFORM_KIRIN970
static const struct of_device_id Hisi_Vdec_Match_Table[] = {
	{.compatible = "hisi,kirin970-vdec",},
	{ }
};
#endif

#ifdef PLATFORM_HIVCODECV210
static const struct of_device_id Hisi_Vdec_Match_Table[] = {
	{.compatible = "hisi,HiVCodecV210-vdec",},
	{ }
};
#endif

#ifdef PLATFORM_HIVCODECV300
static const struct of_device_id Hisi_Vdec_Match_Table[] = {
	{.compatible = "hisi,HiVCodecV300-vdec",},
	{ }
};
#endif

#ifdef PLATFORM_HIVCODECV310
static const struct of_device_id Hisi_Vdec_Match_Table[] = {
	{.compatible = "hisi,HiVCodecV310-vdec",},
	{}
};
#endif

#ifdef PLATFORM_HIVCODECV500
static const struct of_device_id Hisi_Vdec_Match_Table[] = {
	{.compatible = "hisi,HiVCodecV500-vdec",},
	{ }
};
#endif

HI_S32  VDEC_Regulator_Probe(struct device *dev);
HI_S32  VDEC_Regulator_Remove(struct device *dev);
HI_S32  VDEC_Regulator_Enable(HI_VOID);
HI_S32  VDEC_Regulator_Disable(HI_VOID);
HI_VOID VDEC_Regulator_GetClkRate(CLK_RATE_E *pClkRate);
HI_S32  VDEC_Regulator_SetClkRate(CLK_RATE_E eClkRate);

#endif
