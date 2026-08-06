/*
 * zrhung_lastword.c
 *
 * kernel process of the hung lastword
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/sched.h>
#include <linux/module.h>
#include <linux/poll.h>
#ifdef CONFIG_HISI_DFX_CORE
#include "linux/hisi/rdr_dfx_core.h"
#endif

#include "zrhung_lastword.h"
#include "zrhung_transtation.h"
#include <chipset_common/hwzrhung/zrhung.h>
#include "zrhung_common.h"

int hlastword_write(void *buf, uint32_t len)
{
	if (!buf || (len > HTRANS_TOTAL_BUF_SIZE)) {
		HTRANS_ERROR("param not right\n");
		return -1;
	}
#ifdef CONFIG_HISI_DFX_CORE
	dfx_write(DFX_ZEROHUNG, buf, len);
#endif

	return 0;
}

int hlastword_read(void *buf, uint32_t len)
{
	if (!buf || (len > HTRANS_TOTAL_BUF_SIZE)) {
		HTRANS_ERROR("param not right\n");
		return -1;
	}
#ifdef CONFIG_HISI_DFX_CORE
	dfx_read(DFX_ZEROHUNG, buf, len);
#endif

	return 0;
}

int hlastword_init(void)
{
	return 0;
}
