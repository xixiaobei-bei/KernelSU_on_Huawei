/*
 * copyright: Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * file: ocp_hisi.h
 *
 * define platform-dependent bootfail interfaces
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

#ifndef OCP_HISI_H
#define OCP_HISI_H

/* ---- includes ---- */
#include <linux/semaphore.h>

/* ---- c++ support ---- */
#ifdef __cplusplus
extern "C" {
#endif

/* ---- export macroes ---- */
#define OCP_INFO_ARRAY_LEN 10
#define OCP_LDO_NUM_LEN 16
#define OCP_RESERVED_INFO_LEN 240

/* ---- export prototypes ---- */
enum hardware_fault_type {
	HW_FAULT_OCP = 0,
	HW_FAULT_MAX_COUNT,
};

enum hardware_fault_stage {
	HW_FAULT_STAGE_DURING_BOOTUP = 0,
	HW_FAULT_STAGE_AFTER_BOOT_SUCCESS,
	HW_FAULT_STAGE_MAX_COUNT,
};

struct ocp_basic_info {
	char ldo_num[OCP_LDO_NUM_LEN];
	char reserved[OCP_RESERVED_INFO_LEN];
};

struct boot_fail_ocp_info {
	struct ocp_basic_info binfo;
	enum hardware_fault_type type;
	struct semaphore sem;
};

struct ocp_proc_param {
	struct boot_fail_ocp_info ocp_info[OCP_INFO_ARRAY_LEN];
};

struct ocp_init_param {
	void *ctx;
};

/*---- export function prototypes ----*/
int ocp_init(struct ocp_init_param *pparam);

/* ---- c++ support ---- */
#ifdef __cplusplus
}
#endif
#endif
