/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: transfer messages between emcom and sub modules
 * Author: liyouyong liyouyong@huawei.com
 * Create: 2018-09-10
 */
#include "smartcare.h"
#include <linux/rculist.h>
#include "../emcom_netlink.h"
#include "../emcom_utils.h"

#ifdef CONFIG_HW_SMARTCARE_FI
#include "fi/fi.h"
#endif

void smartcare_event_process(int32_t event, uint8_t *pdata, uint16_t len)
{
	switch (event) {
#ifdef CONFIG_HW_SMARTCARE_FI
	case NETLINK_EMCOM_DK_SMARTCARE_FI_APP_LAUNCH:
	case NETLINK_EMCOM_DK_SMARTCARE_FI_APP_STATUS:
		smfi_event_process(event, pdata, len);
		break;
#endif

	default:
		emcom_loge(" : smartcare received unsupported message");
		break;
	}
}
EXPORT_SYMBOL(smartcare_event_process); /*lint !e580*/

void smartcare_init(void)
{
#ifdef CONFIG_HW_SMARTCARE_FI
	smfi_init();
#endif
}
EXPORT_SYMBOL(smartcare_init); /*lint !e580*/

void smartcare_deinit(void)
{
#ifdef CONFIG_HW_SMARTCARE_FI
	smfi_deinit();
#endif
}
EXPORT_SYMBOL(smartcare_deinit); /*lint !e580*/
