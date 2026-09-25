
#ifndef __DRV_VENC_H__
#define __DRV_VENC_H__

#include <linux/hisi-iommu.h>
#include <linux/iommu.h>
#include "drv_venc_efl.h"
#include "drv_venc_ioctl.h"

HI_S32 venc_drv_board_init(void);
HI_VOID venc_drv_board_deinit(void);

#endif //__DRV_VENC_H__

