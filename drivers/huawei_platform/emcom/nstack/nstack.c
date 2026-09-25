/*
 * Copyright (c) 2019-2023 Huawei Technologies Co., Ltd.
 * Description: transfer messages between emcom and sub modules
 * Create: 2019-08-15
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "nstack.h"
#include "../emcom_netlink.h"
#include "../emcom_utils.h"
#include <linux/rculist.h>

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
		EMCOM_LOGE(" : nstack received unsupported message");
		break;
	}
}
EXPORT_SYMBOL(nstack_event_process);

void nstack_init(struct sock *nlfd)
{
#ifdef CONFIG_HW_NSTACK_FI
	fi_init(nlfd);
#endif
}
EXPORT_SYMBOL(nstack_init);

void nstack_deinit(void)
{
#ifdef CONFIG_HW_NSTACK_FI
	fi_deinit();
#endif
}
EXPORT_SYMBOL(nstack_deinit);
