
#include "drv_venc.h"
#include "venc_regulator.h"
#include "drv_venc_osal.h"

HI_S32 venc_drv_board_init(HI_VOID)
{
	HI_S32 ret;

	HI_DBG_VENC("enter %s()\n", __func__);

	ret = venc_regulator_enable();
	if (ret != 0) {
		HI_INFO_VENC("enable regulator failed\n", __func__);
		return HI_FAILURE;
	}

	HI_DBG_VENC("exit %s ()\n", __func__);
	return HI_SUCCESS;
}

HI_VOID venc_drv_board_deinit(HI_VOID)
{
	HI_DBG_VENC("enter %s ()\n", __func__);
	venc_regulator_disable();
	HI_DBG_VENC("exit %s ()\n", __func__);
}


