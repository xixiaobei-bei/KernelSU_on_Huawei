#ifndef __VENC_REGULATOR_H__
#define __VENC_REGULATOR_H__

#include "hi_type.h"
#include "drv_venc_ioctl.h"

HI_S32  Venc_Regulator_Init(struct platform_device *pdev);
HI_VOID Venc_Regulator_Deinit(struct platform_device *pdev);
HI_S32  Venc_Regulator_Enable(HI_VOID);
HI_S32  Venc_Regulator_Disable(HI_VOID);
HI_S32  Venc_SetClkRate(VENC_CLK_TYPE clk_type);

#endif

