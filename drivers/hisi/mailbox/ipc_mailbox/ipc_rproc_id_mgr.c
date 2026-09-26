/*
 * ipc_rproc_id_mgr.c
 *
 * mailbox driver: business channel id and business channel name map.
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
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
#include <linux/hisi/hisi_rproc.h>
#include "ipc_rproc_id_mgr.h"

#define PR_LOG_TAG AP_MAILBOX_TAG

/*lint -e773 */
#define def_rproc_id_name(_rproc_enum_id) \
	[_rproc_enum_id] = #_rproc_enum_id
/*lint +e773 */

static char *g_rproc_id_names[HISI_RPROC_MAX_MBX_ID] = {
	def_rproc_id_name(HISI_LPM3_ACPU_MBX_1),
	def_rproc_id_name(HISI_LPM3_ACPU_MBX_2),
	def_rproc_id_name(HISI_LPM3_ACPU_MBX_3),
	def_rproc_id_name(HISI_HIFI_ACPU_MBX_1),
	def_rproc_id_name(HISI_IOM3_ACPU_MBX_1),
	def_rproc_id_name(HISI_IVP_ACPU_MBX_1),
	def_rproc_id_name(HISI_IVP_ACPU_MBX_2),
	def_rproc_id_name(HISI_ISP_ACPU_MBX_1),
	def_rproc_id_name(HISI_ISP_ACPU_MBX_2),
	def_rproc_id_name(HISI_MODEM_ACPU_MBX_1),
	def_rproc_id_name(HISI_MODEM_ACPU_MBX_2),
	def_rproc_id_name(HISI_MODEM_ACPU_MBX_3),
	def_rproc_id_name(HISI_MODEM_ACPU_MBX_4),
	def_rproc_id_name(HISI_ACPU_IOM3_MBX_1),
	def_rproc_id_name(HISI_ACPU_IOM3_MBX_2),
	def_rproc_id_name(HISI_ACPU_IOM3_MBX_3),
	def_rproc_id_name(HISI_ACPU_LPM3_MBX_1),
	def_rproc_id_name(HISI_ACPU_LPM3_MBX_2),
	def_rproc_id_name(HISI_ACPU_LPM3_MBX_3),
	def_rproc_id_name(HISI_ACPU_LPM3_MBX_4),
	def_rproc_id_name(HISI_ACPU_LPM3_MBX_5),
	def_rproc_id_name(HISI_ACPU_LPM3_MBX_6),
	def_rproc_id_name(HISI_ACPU_LPM3_MBX_7),
	def_rproc_id_name(HISI_ACPU_LPM3_MBX_8),
	def_rproc_id_name(HISI_ACPU_LPM3_MBX_9),
	def_rproc_id_name(HISI_ACPU_HIFI_MBX_1),
	def_rproc_id_name(HISI_ACPU_HIFI_MBX_2),
	def_rproc_id_name(HISI_ACPU_MODEM_MBX_1),
	def_rproc_id_name(HISI_ACPU_MODEM_MBX_2),
	def_rproc_id_name(HISI_ACPU_MODEM_MBX_3),
	def_rproc_id_name(HISI_ACPU_MODEM_MBX_4),
	def_rproc_id_name(HISI_ACPU_MODEM_MBX_5),
	def_rproc_id_name(HISI_ACPU_IVP_MBX_1),
	def_rproc_id_name(HISI_ACPU_IVP_MBX_2),
	def_rproc_id_name(HISI_ACPU_IVP_MBX_3),
	def_rproc_id_name(HISI_ACPU_IVP_MBX_4),
	def_rproc_id_name(HISI_ACPU_ISP_MBX_1),
	def_rproc_id_name(HISI_ACPU_ISP_MBX_2),
	def_rproc_id_name(HISI_AO_ACPU_IOM3_MBX1),
	def_rproc_id_name(HISI_AO_IOM3_ACPU_MBX1),
	def_rproc_id_name(HISI_NPU_ACPU_NPU_MBX1),
	def_rproc_id_name(HISI_NPU_ACPU_NPU_MBX2),
	def_rproc_id_name(HISI_NPU_ACPU_NPU_MBX3),
	def_rproc_id_name(HISI_NPU_ACPU_NPU_MBX4),
	def_rproc_id_name(HISI_NPU_NPU_ACPU_MBX1),
	def_rproc_id_name(HISI_NPU_NPU_ACPU_MBX2)
};

char *ipc_get_rproc_name(int rproc_id)
{
	if (rproc_id < 0 || rproc_id >= HISI_RPROC_MAX_MBX_ID)
		return "Invalid rproc_id";

	return g_rproc_id_names[rproc_id];
}

int ipc_find_rproc_id(const char *rproc_name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_rproc_id_names); i++) {
		if (g_rproc_id_names[i] &&
			!strcmp(rproc_name, g_rproc_id_names[i])) {
			pr_debug("rproc_map{%s : %u}\n",
				g_rproc_id_names[i], i);
			return i;
		}
	}

	return -1;
}

int ipc_check_rproc_id_cfg(void)
{
	int rproc_id;
	char *rproc_name = NULL;
	int ret = 0;

	for (rproc_id = 0; rproc_id < HISI_RPROC_MAX_MBX_ID; rproc_id++) {
		rproc_name = g_rproc_id_names[rproc_id];
		if (!rproc_name) {
			pr_err("rproc_id:%d is not config rproc_name!\n",
				rproc_id);
			ret++;
		}
	}

	return ret;
}
