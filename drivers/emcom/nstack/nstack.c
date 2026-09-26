/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
* Description: transfer messages between emcom and sub modules
* Author: songqiubin songqiubin@huawei.com
* Create: 2019-08-15
*/

#include "nstack.h"
#include <linux/rculist.h>
#include "../emcom_netlink.h"
#include "../emcom_utils.h"

#ifdef CONFIG_HW_NSTACK_FI
#include "fi/fi.h"
#endif

void nstack_event_process(int32_t event, uint8_t *pdata, uint16_t len)
{
	switch (event) {
#ifdef CONFIG_HW_NSTACK_FI
	case NETLINK_EMCOM_DK_NSTACK_FI_CFG:
		fi_cfg_process((struct fi_cfg_head *)pdata);
	break;
#endif

	default:
		emcom_loge(" : nstack received unsupported message");
		break;
	}
}
EXPORT_SYMBOL(nstack_event_process); /*lint !e580*/

void nstack_init(struct sock *nlfd)
{
#ifdef CONFIG_HW_NSTACK_FI
	fi_init(nlfd);
#endif
}
EXPORT_SYMBOL(nstack_init); /*lint !e580*/

void nstack_deinit(void)
{
#ifdef CONFIG_HW_NSTACK_FI
	fi_deinit();
#endif
}
EXPORT_SYMBOL(nstack_deinit); /*lint !e580*/