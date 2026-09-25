/*
 * tee_rdr_register.h
 *
 * for rdr memory register
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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
#ifndef __TEE_RDR_REGISTER_H__
#define __TEE_RDR_REGISTER_H__

int tc_ns_register_rdr_mem(void);
int teeos_register_exception(void);
int tee_rdr_init(void);
unsigned long tc_ns_get_rdr_mem_addr(void);
unsigned int tc_ns_get_rdr_mem_len(void);

#endif
