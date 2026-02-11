/*
 * Copyright (c) 2018-2022 Huawei Technologies Co., Ltd.
 * Description: transfer messages between emcom and sub modules
 * Author: liyouyong liyouyong@huawei.com
 * Create: 2018-09-10
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
#include "smartcare.h"
#include "../emcom_netlink.h"
#include "../emcom_utils.h"
#include <linux/rculist.h>

#ifdef CONFIG_HW_SMARTCARE_FI
#include "fi/fi.h"
#endif

void smartcare_event_process(int32_t event, uint8_t *pdata, uint16_t len)
{
	switch (event) {
#ifdef CONFIG_HW_SMARTCARE_FI
	case NETLINK_EMCOM_DK_SMARTCARE_FI_APP_LAUNCH:
	case NETLINK_EMCOM_DK_SMARTCARE_FI_APP_STATUS:
		fi_event_process(event, pdata, len);
		break;
#endif

	default:
		EMCOM_LOGE(" : smartcare received unsupported message");
		break;
	}
}
EXPORT_SYMBOL(smartcare_event_process);

void smartcare_init(void)
{
#ifdef CONFIG_HW_SMARTCARE_FI
	fi_init();
#endif
}
EXPORT_SYMBOL(smartcare_init);

void smartcare_deinit(void)
{
#ifdef CONFIG_HW_SMARTCARE_FI
	fi_deinit();
#endif
}
EXPORT_SYMBOL(smartcare_deinit);
