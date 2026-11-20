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
#include <pr_log.h>
#define PR_LOG_TAG NOC_TAG

#include "dfx_noc_info.h"
#include "dfx_noc.h"

/* !!NOTE: Used to caculate platform id defined in DTS!! */
#define PLATFORM_ID_BIT_MOUSE            (0U)
#define PLATFORM_ID_BIT_CAPRICORN        (1U)
#define PLATFORM_ID_BIT_COWS             (2U)
#define PLPLATFORM_ID_BIT_TRIGER_ES      (3U)
#define PLPLATFORM_ID_BIT_TRIGER         (4U)
#define PLATFORM_ID_BIT_AQUARIUS         (5U)
#define PLATFORM_ID_BIT_RABBIT_ES        (6U)
#define PLATFORM_ID_BIT_RABBIT           (7U)
#define PLATFORM_ID_BIT_PISCES           (8U)
#define PLATFORM_ID_BIT_DARGON_ES        (9U)
#define PLATFORM_ID_BIT_DARGON           (10U)
#define PLATFORM_ID_BIT_DARGON_CS2       (11U)
#define PLATFORM_ID_BIT_SNAKE_ES         (12U)
#define PLATFORM_ID_BIT_ARIES            (13U)
#define PLATFORM_ID_BIT_SNAKE_CS         (14U)
#define PLATFORM_ID_BIT_CANCER           (15U)
#define PLATFORM_ID_BIT_TAURUS           (16U)
#define PLATFORM_ID_BIT_GEMINI           (17U)

struct noc_dump_reg g_dump_reg_offset[MAX_DUMP_REG];
struct noc_busid_initflow g_busid_initflow[MAX_FILTER_INITFLOW];
struct noc_bus_info g_noc_bus_info[MAX_BUSID_VALE];
/* platform noc bus info gloabl variable. */
struct noc_platform_info g_noc_platform_info[] = {
	{
		.name = "mouse",
		.platform_id = 1 << PLATFORM_ID_BIT_MOUSE,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = noc_buses_info_mouse,
		.p_noc_info_dump = noc_dump_reg_list_mouse,
		.p_noc_info_filter_initflow = dfx_filter_initflow_mouse,
		.pfun_get_size = dfx_noc_get_array_size_mouse,
		.pfun_clock_enable = dfx_noc_clock_enable_mouse,
	},
	{
		.name = "capricorn",
		.platform_id = 1 << PLATFORM_ID_BIT_CAPRICORN,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = noc_buses_info_capricorn,
		.p_noc_info_dump = noc_dump_reg_list_capricorn,
		.p_noc_info_filter_initflow = dfx_filter_initflow_capricorn,
		.pfun_get_size = dfx_noc_get_array_size_capricorn,
		.pfun_clock_enable = dfx_noc_clock_enable_capricorn,
	},
	{
		.name = "cows",
		.platform_id = 1 << PLATFORM_ID_BIT_COWS,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = noc_buses_info_cows,
		.p_noc_info_dump = noc_dump_reg_list_cows,
		.p_noc_info_filter_initflow = dfx_filter_initflow_cows,
		.pfun_get_size = dfx_noc_get_array_size_cows,
		.pfun_clock_enable = dfx_noc_clock_enable_cows,
	},
	{
		.name = "tiger_es",
		.platform_id = 1 << PLPLATFORM_ID_BIT_TRIGER_ES,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = noc_buses_info_tiger_es,
		.p_noc_info_dump = noc_dump_reg_list_tiger_es,
		.p_noc_info_filter_initflow = dfx_filter_initflow_tiger_es,
		.pfun_get_size = dfx_noc_get_array_size_tiger_es,
		.pfun_clock_enable = dfx_noc_clock_enable_tiger_es,
	},
	{
		.name = "tiger",
		.platform_id = 1 << PLPLATFORM_ID_BIT_TRIGER,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = noc_buses_info_tiger,
		.p_noc_info_dump = noc_dump_reg_list_tiger,
		.p_noc_info_filter_initflow = dfx_filter_initflow_tiger,
		.pfun_get_size = dfx_noc_get_array_size_tiger,
		.pfun_clock_enable = dfx_noc_clock_enable_tiger,
	},
	{
		.name = "aquarius",
		.platform_id = 1 << PLATFORM_ID_BIT_AQUARIUS,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = noc_buses_info_aquarius,
		.p_noc_info_dump = noc_dump_reg_list_aquarius,
		.p_noc_info_filter_initflow = dfx_filter_initflow_aquarius,
		.pfun_get_size = dfx_noc_get_array_size_aquarius,
		.pfun_clock_enable = dfx_noc_clock_enable_aquarius,
	},
	{
		.name = "rabbit_es",
		.platform_id = 1 << PLATFORM_ID_BIT_RABBIT_ES,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = noc_buses_info_rabbit_es,
		.p_noc_info_dump = noc_dump_reg_list_rabbit_es,
		.p_noc_info_filter_initflow = dfx_filter_initflow_rabbit_es,
		.pfun_get_size = dfx_noc_get_array_size_rabbit_es,
		.pfun_clock_enable = dfx_noc_clock_enable_rabbit_es,
	},
	{
		.name = "rabbit",
		.platform_id = 1 << PLATFORM_ID_BIT_RABBIT,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = noc_buses_info_rabbit,
		.p_noc_info_dump = noc_dump_reg_list_rabbit,
		.p_noc_info_filter_initflow = dfx_filter_initflow_rabbit,
		.pfun_get_size = dfx_noc_get_array_size_rabbit,
		.pfun_clock_enable = dfx_noc_clock_enable_rabbit,
	},
	{
		.name = "pisces",
		.platform_id = 1 << PLATFORM_ID_BIT_PISCES,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = noc_buses_info_pisces,
		.p_noc_info_dump = noc_dump_reg_list_pisces,
		.p_noc_info_filter_initflow = dfx_filter_initflow_pisces,
		.pfun_get_size = dfx_noc_get_array_size_pisces,
		.pfun_clock_enable = dfx_noc_clock_enable_pisces,
	},
	{
		.name = "dragon_es",
		.platform_id = 1 << PLATFORM_ID_BIT_DARGON_ES,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = NULL,
		.p_noc_info_dump = NULL,
		.p_noc_info_filter_initflow = NULL,
		.pfun_get_size = NULL,
		.pfun_clock_enable = dfx_noc_clock_enable_check,
	},
	{
		.name = "dragon",
		.platform_id = 1 << PLATFORM_ID_BIT_DARGON,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = NULL,
		.p_noc_info_dump = NULL,
		.p_noc_info_filter_initflow = NULL,
		.pfun_get_size = NULL,
		.pfun_clock_enable = dfx_noc_clock_enable_check,
	},
	{
		.name = "dragon_cs2",
		.platform_id = 1 << PLATFORM_ID_BIT_DARGON_CS2,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = noc_buses_info_dragon_cs2,
		.p_noc_info_dump = noc_dump_reg_list_dragon_cs2,
		.p_noc_info_filter_initflow = dfx_filter_initflow_dragon_cs2,
		.pfun_get_size = dfx_noc_get_array_size_dragon_cs2,
		.pfun_clock_enable = dfx_noc_clock_enable_dragon_cs2,
	},
	{
		.name = "snake_es",
		.platform_id = 1 << PLATFORM_ID_BIT_SNAKE_ES,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = NULL,
		.p_noc_info_dump = NULL,
		.p_noc_info_filter_initflow = NULL,
		.pfun_get_size = NULL,
		.pfun_clock_enable = dfx_noc_clock_enable_check,
	},
	{
		.name = "aries",
		.platform_id = 1 << PLATFORM_ID_BIT_ARIES,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = NULL,
		.p_noc_info_dump = NULL,
		.p_noc_info_filter_initflow = NULL,
		.pfun_get_size = NULL,
		.pfun_clock_enable = dfx_noc_clock_enable_check,
		},
	{
		.name = "snake_cs",
		.platform_id = 1 << PLATFORM_ID_BIT_SNAKE_CS,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = NULL,
		.p_noc_info_dump = NULL,
		.p_noc_info_filter_initflow = NULL,
		.pfun_get_size = NULL,
		.pfun_clock_enable = dfx_noc_clock_enable_check,
	},
	{
		.name = "cancer",
		.platform_id = 1 << PLATFORM_ID_BIT_CANCER,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = NULL,
		.p_noc_info_dump = NULL,
		.p_noc_info_filter_initflow = NULL,
		.pfun_get_size = NULL,
		.pfun_clock_enable = dfx_noc_clock_enable_check,
	},
	{
		.name = "taurus",
		.platform_id = 1 << PLATFORM_ID_BIT_TAURUS,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = NULL,
		.p_noc_info_dump = NULL,
		.p_noc_info_filter_initflow = NULL,
		.pfun_get_size = NULL,
		.pfun_clock_enable = dfx_noc_clock_enable_check,
	},
	{
		.name = "gemini",
		.platform_id = 1 << PLATFORM_ID_BIT_GEMINI,	 /* must be same as the value defined in DTS. */
		.p_noc_info_bus = NULL,
		.p_noc_info_dump = NULL,
		.p_noc_info_filter_initflow = NULL,
		.pfun_get_size = NULL,
		.pfun_clock_enable = dfx_noc_clock_enable_check,
	},
};

static unsigned int g_info_index;
static struct noc_arr_info noc_buses_i;
struct noc_dump_reg *noc_dump_reg_list;
/* if noc_happend's nodename is in the dfx_modemnoc_nodemame,
 * firstly save log, then system reset.
 */
static const char * const dfx_modemnoc_nodemame[] = {
	"modem_bus",
	"mmd_bus",
	NULL,			 /* end */
};

/*******************************************************************************
 * Function:       dfx_get_modemnoc_nodename
 * Description:    get the filter conditions of modemnoc.
 * Input:          NA
 * Output:         modemnoc_nodename:the filter condition of nodename
 * Return:         NA
 *******************************************************************************/
void dfx_get_modemnoc_nodename(char ***modemnoc_nodename)
{
	if (modemnoc_nodename == NULL)
		return;

	*modemnoc_nodename = (char **)dfx_modemnoc_nodemame;
}

/*
 * dfx_noc_get_bus_info_num -- return noc parse table pointer.
 */
unsigned int dfx_noc_get_bus_info_num(void)
{
	return noc_buses_i.len;
}

/*
 * dfx_noc_get_dump_reg_list_num -- return noc dump register list lenght.
 */
unsigned int dfx_noc_get_dump_reg_list_num(void)
{
	return g_noc_platform_info[g_info_index].noc_info_dump_len;
}

/*******************************************************************************
 * Function:       dfx_get_noc_initflow
 * Description:    get the filter conditions of modem,hifi etc noc.
 * Input:          NA
 * Output:         noc_initflow:the filter condition of initflow
 * Return:         NA
 *******************************************************************************/
void dfx_get_noc_initflow(const struct noc_busid_initflow **filter_initflow)
{
	if (filter_initflow == NULL)
		return;

	*filter_initflow = g_noc_platform_info[g_info_index].p_noc_info_filter_initflow;
}

/*
 * dfx_noc_clock_enable -- check noc device node clock state.
 * @noc_dev -- noc device node pointer;
 * reture clock state of related node.
 */
unsigned int dfx_noc_clock_enable(struct dfx_noc_device *noc_dev, struct noc_node *node)
{
	return g_noc_platform_info[g_info_index].pfun_clock_enable(noc_dev, node);
}

/*
 * noc_set_buses_info -- Get NoC Error Address By Route-id Table.
 * @platform_id -- param from noc probe flow;
 * return 0 if success, otherwise, -1
 */
int noc_get_platform_info_index(unsigned int platform_id)
{
	unsigned int index;

	for (index = 0; index < ARRAY_SIZE_NOC(g_noc_platform_info); index++) {
		if (g_noc_platform_info[index].platform_id == platform_id)
			return index;
	}

	return -1;
}

/*
 * noc_set_buses_info -- get platform info define by platform id value.
 * @platform_id -- param from noc probe flow;
 * return 0 if success, otherwise, -1
 */
int noc_set_buses_info(unsigned int platform_id)
{
	int ret;

	/* get platform info index by platform id value defined in DTS. */
	ret = noc_get_platform_info_index(platform_id);
	if (ret < 0) {
		pr_err("[%s]: Error!! platform_id[%d], No platform id matched!!\n", __func__, platform_id);
		return -1;
	}

	/* save platform info index. */
	g_info_index = ret;
	pr_crit("[%s]: info index is [%d], platform is:[%s].\n", __func__,
		g_info_index, g_noc_platform_info[g_info_index].name);

	/* get platform info array size. */
	g_noc_platform_info[g_info_index].pfun_get_size(&g_noc_platform_info[g_info_index].noc_info_bus_len,
							&g_noc_platform_info[g_info_index].noc_info_dump_len);
	if ((g_noc_platform_info[g_info_index].noc_info_dump_len == 0)
	    || (g_noc_platform_info[g_info_index].noc_info_bus_len == 0)) {
		pr_err("[%s]: Get noc info length Error!!\n", __func__);
		return -1;
	}

	/* save platform info. */
	noc_buses_i.ptr = g_noc_platform_info[g_info_index].p_noc_info_bus;
	noc_buses_i.len = g_noc_platform_info[g_info_index].noc_info_bus_len;
	noc_dump_reg_list = g_noc_platform_info[g_info_index].p_noc_info_dump;

	return 0;
}

/*
 * Function: noc_get_mid_info
 * Description: noc get mid_info and mid_info size
 * input: bus_id
 * output: noc_mid_info pointer and mid_info size
 * return: none
 */
void noc_get_mid_info(unsigned int bus_id, struct noc_mid_info **pt_info, unsigned int *pt_size)
{
	const struct noc_bus_info *pt_noc_bus = noc_get_bus_info(bus_id);

	if (!pt_noc_bus) {
		pr_err("[%s]: pt_noc_bus is NULL\n", __func__);
		return;
	}

	if (pt_info == NULL || pt_size == NULL)
		return;

	*pt_info = (struct noc_mid_info *)(pt_noc_bus->p_noc_mid_info);
	*pt_size = pt_noc_bus->noc_mid_info_size;
}

/*
 * Function: noc_get_sec_info
 * Description: noc get sec_info and sec_info size
 * input: bus_id
 * output: noc_sec_info pointer and sec_info size
 * return: none
 */
void noc_get_sec_info(unsigned int bus_id, struct noc_sec_info **pt_info, unsigned int *pt_size)
{
	const struct noc_bus_info *pt_noc_bus = noc_get_bus_info(bus_id);

	if (!pt_noc_bus) {
		pr_err("[%s]: pt_noc_bus is NULL\n", __func__);
		return;
	}

	if (pt_info == NULL || pt_size == NULL)
		return;

	*pt_info = (struct noc_sec_info *)(pt_noc_bus->p_noc_sec_info);
	*pt_size = pt_noc_bus->noc_sec_info_size;
}

/*
 * Function: noc_get_buses_info
 * Description: get noc_bus_info
 * input: none
 * output: none
 * return: noc_arr_info pointer
 */
struct noc_arr_info *noc_get_buses_info(void)
{
	return &noc_buses_i;
}

/*
 * Function: noc_get_bus_info
 * Description: get noc_bus_info from bus_id
 * input: int bus_id -> bus id input
 * output: none
 * return: noc_bus_info
 */
const struct noc_bus_info *noc_get_bus_info(unsigned int bus_id)
{
	const struct noc_bus_info *noc_bus = NULL;

	if (bus_id < noc_buses_i.len) {
		noc_bus = noc_buses_i.ptr;
		noc_bus += bus_id;
	}

	return noc_bus;
}

/*
 * Function: noc_find_addr_from_routeid
 * Description:  Get NoC Error Address By Route-id Table.
 * @unsigned int idx -- Bus Id;
 * @int initflow -- Access Init Flow
 * @int targetflow -- Access Target Flow
 * @int targetsubrange -- Target Subrange
 * Output: NA
 * Return: u64 -- NoC Error Local Address
 */
u64 noc_find_addr_from_routeid(unsigned int idx, int initflow, int targetflow, int targetsubrange)
{
	unsigned int i;
	unsigned int count;

	const struct datapath_routid_addr *past_tbl = NULL;

	if (idx >= dfx_noc_get_bus_info_num())
		return 0;

	past_tbl = noc_buses_i.ptr[idx].routeid_tbl;
	count = noc_buses_i.ptr[idx].routeid_tbl_size;

	for (i = 0; i < count; i++) {
		if ((past_tbl[i].targ_flow == targetflow) && (past_tbl[i].targ_subrange == targetsubrange))
			return past_tbl[i].init_localaddr;
	}

	return 0;
}

/*
 * dfx_noc_clock_enable - check noc clock state : on or off
 * @noc_dev : noc device poiter
 * @node:  noc node poiter
 *
 * If clock enable, return 1, else return 0;
 */
unsigned int dfx_noc_clock_enable_check(struct dfx_noc_device *noc_dev, struct noc_node *node)
{
	void __iomem *reg_base = NULL;
	unsigned int reg_value;
	unsigned int i;
	unsigned int ret = 1;

	if ((noc_dev == NULL) || (node == NULL))
		return 0;

	if (noc_dev->pcrgctrl_base != NULL) {
		reg_base = noc_dev->pcrgctrl_base;
	} else {
		pr_err("%s: bus id and clock domain error!\n", __func__);
		return 0;
	}

	for (i = 0; i < DFX_NOC_CLOCK_MAX; i++) {
		if (node->crg_clk[i].offset == DFX_NOC_CLOCK_REG_DEFAULT)
			continue;

		reg_value = readl_relaxed((u8 __iomem *) reg_base + node->crg_clk[i].offset);
		/* Clock is enabled */
		if (reg_value & (1U << node->crg_clk[i].mask_bit)) {
			continue;
		} else {
			ret = 0;
			break;
		}
	}

	if (noc_dev->noc_property->noc_debug)
		pr_err("%s: clock_reg = 0x%pK\n", __func__, reg_base);

	return ret;
}
