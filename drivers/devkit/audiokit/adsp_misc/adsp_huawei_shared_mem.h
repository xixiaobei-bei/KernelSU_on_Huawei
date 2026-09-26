/*
 * adsp_huawei_shared_mem interface
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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
#ifndef _ADSP_HUAWEI_SHARED_MEM_H_
#define _ADSP_HUAWEI_SHARED_MEM_H_

#include <linux/types.h>

/*
 * write to shared mem and then notify adsp of addr and size
 */
int update_adsp_shared_mem(const void __user *buf, size_t buf_size);

/*
 * notify adsp of addr and size only
 */
int notify_adsp_shared_mem(size_t buf_size);

#endif // _ADSP_HUAWEI_SHARED_MEM_H_
