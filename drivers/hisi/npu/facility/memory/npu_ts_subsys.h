/*
 * npu_ts_subsys.h
 *
 * about npu ts subsys
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#ifndef __NPU_TS_SUBSYS_H
#define __NPU_TS_SUBSYS_H

int devdrv_npu_ts_subsys_alloc(unsigned long long *virt_addr, unsigned long *iova, unsigned long size);

void devdrv_npu_ts_subsys_free(unsigned long long virt_addr, unsigned long iova);

#endif
