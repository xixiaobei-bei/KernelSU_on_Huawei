/*
 * npu_svm.h
 *
 * about npu svm
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#ifndef __NPU_SVM_H
#define __NPU_SVM_H
#include "npu_common.h"

int npu_svm_manager_init(uint32_t devid);
int npu_get_ssid_bypid(uint32_t devid, pid_t manager_pid, pid_t svm_pid,
	uint16_t *ssid, uint64_t *ttbr, uint64_t *tcr);
int npu_insert_item_bypid(uint32_t devid, pid_t manager_pid, pid_t svm_pid);
int npu_release_item_bypid(uint32_t devid, pid_t manager_pid, pid_t svm_pid);
int npu_clear_pid_ssid_table(uint32_t devid, int flag);
int npu_svm_manager_destroy(uint32_t devid);

#endif
