/*
 * declare_static_ion.h
 *
 * set static ion mem info.
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
 */
#ifndef DECLARE_STATIC_ION_H
#define DECLARE_STATIC_ION_H
#include "static_ion_mem.h"

void set_ion_mem_info(struct register_ion_mem_tag *memtag);

#ifdef DEF_ENG
void get_secos_mem(uint64_t *addr, uint32_t *size);
#endif

#endif
