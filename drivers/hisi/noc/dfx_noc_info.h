/*
 *
 * NoC. (NoC Mntn Module.)
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
#ifndef __DFX_NOC_INFO_H
#define __DFX_NOC_INFO_H

#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/syscore_ops.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/debugfs.h>
#include <linux/io.h>
#include <linux/string.h>
#include <linux/hisi/util.h>

#include "dfx_noc.h"

#define ARRAY_END_FLAG		0xffffffff
#define ARRAY_SIZE_NOC(arr) (sizeof(arr) / sizeof((arr)[0]))

struct noc_mid_info {
	unsigned int idx;	 /* Bus Id: 0 */
	int init_flow;
	unsigned int mask;
	unsigned int mid_val;
	char *mid_name;
};

struct noc_sec_info {
	unsigned int mask;
	unsigned int sec_val;
	char *sec_array;
	char *sec_mode;
};

/* keep target route id, initiator flow id etc */
struct noc_bus_info {
	char *name;
	unsigned int initflow_mask;
	unsigned int initflow_shift;

	unsigned int targetflow_mask;
	unsigned int targetflow_shift;

	unsigned int targ_subrange_mask;
	unsigned int seq_id_mask;

	char **initflow_array;
	unsigned int initflow_array_size;

	char **targetflow_array;
	unsigned int targetflow_array_size;

	const struct datapath_routid_addr *routeid_tbl;
	unsigned int routeid_tbl_size;

	unsigned int opc_mask;
	unsigned int opc_shift;
	char **opc_array;
	unsigned int opc_array_size;

	unsigned int err_code_mask;
	unsigned int err_code_shift;
	char **err_code_array;
	unsigned int err_code_array_size;

	struct noc_mid_info *p_noc_mid_info;
	unsigned int noc_mid_info_size;
	struct noc_sec_info *p_noc_sec_info;
	unsigned int noc_sec_info_size;
};

struct noc_arr_info {
	const struct noc_bus_info *ptr;
	unsigned int len;
};

/* for backup noc_errorprobe info */
struct noc_errorprobe_backup_info {
	unsigned int bus_id;
	int init_flow;
	char *nodename;
};

/* for modemnoc_initflow array */
struct noc_busid_initflow {
	unsigned int bus_id;
	int init_flow;
	int coreid;
};

struct noc_dump_reg {
	char *name;
	void __iomem *addr;
	unsigned int offset;
};

struct noc_platform_info {
	const char *name;
	unsigned int platform_id;
	const struct noc_bus_info *p_noc_info_bus;
	unsigned int noc_info_bus_len;
	struct noc_dump_reg *p_noc_info_dump;
	unsigned int noc_info_dump_len;
	const struct noc_busid_initflow *p_noc_info_filter_initflow;
	void (*pfun_get_size)(unsigned int *info_size, unsigned int *list_size);
	unsigned int (*pfun_clock_enable)(struct dfx_noc_device *device,
					   struct noc_node *node);
};

extern struct noc_dump_reg *noc_dump_reg_list;
extern struct noc_dump_reg g_dump_reg_offset[MAX_DUMP_REG];
extern struct noc_busid_initflow g_busid_initflow[MAX_FILTER_INITFLOW];
extern struct noc_bus_info g_noc_bus_info[MAX_BUSID_VALE];

extern const struct noc_bus_info noc_buses_info_mouse[];
extern struct noc_mid_info noc_mid_mouse[];
extern struct noc_dump_reg noc_dump_reg_list_mouse[];
extern const struct noc_busid_initflow dfx_filter_initflow_mouse[];

extern const struct noc_bus_info noc_buses_info_capricorn[];
extern struct noc_mid_info noc_mid_capricorn[];
extern struct noc_dump_reg noc_dump_reg_list_capricorn[];
extern const struct noc_busid_initflow dfx_filter_initflow_capricorn[];

extern const struct noc_bus_info noc_buses_info_cows[];
extern struct noc_mid_info noc_mid_cows[];
extern struct noc_dump_reg noc_dump_reg_list_cows[];
extern const struct noc_busid_initflow dfx_filter_initflow_cows[];

extern const struct noc_bus_info noc_buses_info_tiger_es[];
extern struct noc_mid_info noc_mid_tiger_es[];
extern struct noc_dump_reg noc_dump_reg_list_tiger_es[];
extern const struct noc_busid_initflow dfx_filter_initflow_tiger_es[];

extern const struct noc_bus_info noc_buses_info_rabbit_es[];
extern struct noc_mid_info noc_mid_rabbit_es[];
extern struct noc_dump_reg noc_dump_reg_list_rabbit_es[];
extern const struct noc_busid_initflow dfx_filter_initflow_rabbit_es[];

extern const struct noc_bus_info noc_buses_info_rabbit[];
extern struct noc_mid_info noc_mid_rabbit[];
extern struct noc_dump_reg noc_dump_reg_list_rabbit[];
extern const struct noc_busid_initflow dfx_filter_initflow_rabbit[];

extern const struct noc_bus_info noc_buses_info_pisces[];
extern struct noc_mid_info noc_mid_pisces[];
extern struct noc_dump_reg noc_dump_reg_list_pisces[];
extern const struct noc_busid_initflow dfx_filter_initflow_pisces[];

extern const struct noc_bus_info noc_buses_info_tiger[];
extern struct noc_mid_info noc_mid_tiger[];
extern struct noc_dump_reg noc_dump_reg_list_tiger[];
extern const struct noc_busid_initflow dfx_filter_initflow_tiger[];

extern const struct noc_bus_info noc_buses_info_aquarius[];
extern struct noc_mid_info noc_mid_aquarius[];
extern struct noc_dump_reg noc_dump_reg_list_aquarius[];
extern const struct noc_busid_initflow dfx_filter_initflow_aquarius[];

extern const struct noc_bus_info noc_buses_info_dragon_es[];
extern struct noc_mid_info noc_mid_dragon_es[];
extern struct noc_dump_reg noc_dump_reg_list_dragon_es[];
extern const struct noc_busid_initflow dfx_filter_initflow_dragon_es[];

extern const struct noc_bus_info noc_buses_info_dragon[];
extern struct noc_mid_info noc_mid_dragon[];
extern struct noc_dump_reg noc_dump_reg_list_dragon[];
extern const struct noc_busid_initflow dfx_filter_initflow_dragon[];

extern const struct noc_bus_info noc_buses_info_dragon_cs2[];
extern struct noc_mid_info noc_mid_dragon_cs2[];
extern struct noc_dump_reg noc_dump_reg_list_dragon_cs2[];
extern const struct noc_busid_initflow dfx_filter_initflow_dragon_cs2[];

int noc_set_buses_info(unsigned int info_index);
void noc_get_mid_info(unsigned int bus_id, struct noc_mid_info **pt_info,
		      unsigned int *pt_size);
void noc_get_sec_info(unsigned int bus_id, struct noc_sec_info **pt_info,
		      unsigned int *pt_size);
struct noc_arr_info *noc_get_buses_info(void);
unsigned int dfx_noc_clock_enable_check(struct dfx_noc_device *noc_dev, struct noc_node *node);
extern int noc_get_platform_info_index(unsigned int platform_id);
extern const struct noc_bus_info *noc_get_bus_info(unsigned int bus_id);
extern struct noc_platform_info g_noc_platform_info[];
extern unsigned int dfx_noc_get_bus_info_num(void);
extern unsigned int dfx_noc_get_dump_reg_list_num(void);
extern void dfx_get_noc_initflow(const struct noc_busid_initflow
				  **filter_initflow);
extern void dfx_get_modemnoc_nodename(char ***modemnoc_nodename);
extern void dfx_noc_get_array_size_mouse(unsigned int *bus_info_size,
					     unsigned int *dump_list_size);
extern void dfx_noc_get_array_size_capricorn(unsigned int *bus_info_size,
					     unsigned int *dump_list_size);
extern void dfx_noc_get_array_size_cows(unsigned int *bus_info_size,
					     unsigned int *dump_list_size);
extern void dfx_noc_get_array_size_tiger_es(unsigned int *bus_info_size,
						 unsigned int *dump_list_size);
extern void dfx_noc_get_array_size_tiger(unsigned int *bus_info_size,
					     unsigned int *dump_list_size);
extern void dfx_noc_get_array_size_rabbit_es(unsigned int *bus_info_size,
						unsigned int *dump_list_size);
extern void dfx_noc_get_array_size_rabbit(unsigned int *bus_info_size,
						unsigned int *dump_list_size);
extern void dfx_noc_get_array_size_aquarius(unsigned int *bus_info_size,
					     unsigned int *dump_list_size);
extern void dfx_noc_get_array_size_pisces(unsigned int *bus_info_size,
						unsigned int *dump_list_size);
extern void dfx_noc_get_array_size_dragon_cs2(unsigned int *bus_info_size,
					     unsigned int *dump_list_size);

extern unsigned int dfx_noc_clock_enable(struct dfx_noc_device *noc_dev,
					  struct noc_node *node);
extern unsigned int dfx_noc_clock_enable_mouse(struct dfx_noc_device *noc_dev,
						 struct noc_node *node);
extern unsigned int dfx_noc_clock_enable_capricorn(struct dfx_noc_device *noc_dev,
						 struct noc_node *node);
extern unsigned int dfx_noc_clock_enable_cows(struct dfx_noc_device *noc_dev,
						 struct noc_node *node);
extern unsigned int dfx_noc_clock_enable_tiger_es(struct dfx_noc_device *noc_dev,
						      struct noc_node *node);
extern unsigned int dfx_noc_clock_enable_rabbit_es(struct dfx_noc_device *noc_dev,
								struct noc_node *node);
extern unsigned int dfx_noc_clock_enable_rabbit(struct dfx_noc_device *noc_dev,
								struct noc_node *node);
extern unsigned int dfx_noc_clock_enable_tiger(struct dfx_noc_device *noc_dev,
						   struct noc_node *node);
extern unsigned int dfx_noc_clock_enable_aquarius(struct dfx_noc_device *noc_dev,
						   struct noc_node *node);
extern unsigned int dfx_noc_clock_enable_pisces(struct dfx_noc_device *noc_dev,
								struct noc_node *node);
extern unsigned int dfx_noc_clock_enable_dragon_cs2(struct dfx_noc_device *noc_dev,
						   struct noc_node *node);

#endif
