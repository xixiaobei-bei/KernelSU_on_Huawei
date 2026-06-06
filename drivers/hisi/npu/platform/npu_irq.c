#include "npu_irq.h"
#include "drv_log.h"

int devdrv_plat_parse_irq(struct platform_device *pdev,
		struct devdrv_platform_info *plat_info)
{
	DEVDRV_PLAT_GET_CQ_UPDATE_IRQ(plat_info) =
		platform_get_irq(pdev, DEVDRV_IRQ_CALC_CQ_UPDATE0);
	DEVDRV_PLAT_GET_DFX_CQ_IRQ(plat_info) =
		platform_get_irq(pdev, DEVDRV_IRQ_DFX_CQ_UPDATE);
	DEVDRV_PLAT_GET_MAILBOX_ACK_IRQ(plat_info) =
		platform_get_irq(pdev, DEVDRV_IRQ_MAILBOX_ACK);
	devdrv_drv_debug("calc_cq_update0=%d\n", DEVDRV_PLAT_GET_CQ_UPDATE_IRQ(plat_info));
	devdrv_drv_debug("dfx_cq_update=%d\n", DEVDRV_PLAT_GET_DFX_CQ_IRQ(plat_info));
	devdrv_drv_debug("mailbox_ack=%d\n", DEVDRV_PLAT_GET_MAILBOX_ACK_IRQ(plat_info));
	return 0;
}
