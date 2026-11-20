/*
 * tvp_adapter.h
 *
 * This is vcodec utils.
 *
 * Copyright (c) 2019-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef TVP_ADAPTER_H
#define TVP_ADAPTER_H

#include "hi_type.h"
#include "drv_mem.h"

enum {
	HIVCODEC_CMD_ID_INIT = 1,
	HIVCODEC_CMD_ID_EXIT,
	HIVCODEC_CMD_ID_SUSPEND,
	HIVCODEC_CMD_ID_RESUME,
	HIVCODEC_CMD_ID_CONTROL,
	HIVCODEC_CMD_ID_RUN_PROCESS,
	HIVCODEC_CMD_ID_GET_IMAGE,
	HIVCODEC_CMD_ID_RELEASE_IMAGE,
	HIVCODEC_CMD_ID_CONFIG_INPUT_BUFFER,
	HIVCODEC_CMD_ID_READ_PROC,
	HIVCODEC_CMD_ID_WRITE_PROC,
	HIVCODEC_CMD_ID_MEM_CPY = 20,
	HIVCODEC_CMD_ID_CFG_MASTER,
};

enum sec_venc_state {
	SEC_VENC_OFF,
	SEC_VENC_ON,
};

int32_t init_kernel_ca(void);
void deinit_kernel_ca(void);
void config_master(enum sec_venc_state state, uint32_t core_id);

#endif
