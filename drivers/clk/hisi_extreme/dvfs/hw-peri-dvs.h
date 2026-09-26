/*
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef __HW_PERI_DVS_H__
#define __HW_PERI_DVS_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/hwspinlock.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include "peri_volt_poll.h"
#include "securec.h"

#define MAX_PERI_DVS_SFT_CHANNEL       16
#define MAX_PERI_DVS_DUMP_REG_NUM      8
#define MAX_PERI_DVS_VOTE_WIDTH        2
#define MAX_PERI_DVS_IRQ_USEC          1000
#define DEFAULT_PERI_DVS_IRQ_USEC      100
#define PERI_DVS_CHANNEL_MASK          0x0000FFFF
#define HWLOCK_TIMEOUT                 1000
#define PERI_DVS_HWLOCK                19
#define DVS_VOTE_REG_NUM               2

enum mediabus_type {
	MEDIABUS_VIVOBUS  = 0,
	MEDIABUS_VCODECBUS,
	MEDIABUS_MAX,
};

enum {
	NORMAL_TEMPRATURE = 0,
	LOW_TEMPRATURE,
};

enum {
	PERI_FIXED_VOLT_NONE = 0,
	PERI_FIXED_VOLT_0,
	PERI_FIXED_VOLT_1,
	PERI_FIXED_VOLT_2,
	PERI_FIXED_VOLT_3,
	PERI_FIXED_VOLT_MAX,
};

enum {
	DVS_VOTE_REG = 0,    /* peri dvs vote reg addr */
	DVS_VLD_REG,         /* peri dvs vote valid reg addr */
	DVS_REQ_REG,         /* peri dvs vote request reg addr */
	DVS_IRQ_CPT_REG,     /* peri dvs vote irq complete reg addr */
	DVS_IRQ_CLR_REG,     /* peri dvs vote irq clear reg addr */
	DVS_REQ_BP_REG,      /* peri dvs vote request bypass reg addr */
	DVS_IRQ_CPT_BP_REG,  /* peri dvs vote irq complete bypass reg addr */
	DVS_MAX_REG,
};

struct mediabus_profile {
	unsigned int  volt_level;
	unsigned long source_freq;
	unsigned int  div;
};

struct peri_dvs_mediabus {
	enum mediabus_type  bus_type;
	unsigned int        volt_level;
	unsigned int        valid;
	unsigned int        low_temp_profile;
	struct clk         *pll_clks[PERI_VOLT_MAX];
	struct list_head    mediabus_list;
};

struct peri_dvs_vote {
	const char       *name;
	unsigned int      dev_id;
	unsigned int      channel;
	unsigned int      vote_valid;
	unsigned int      mediabus_id;
	unsigned int      volt_level;
	unsigned int      record_reg_type;
	unsigned int      record_reg_offset;
	unsigned int      record_mask;
	unsigned int      record_shift;
	unsigned int      valid_vote_sync;
	unsigned int      sysbus_sync_flag;
	struct list_head  mediabus_node;
	struct list_head  chan_node;
};

struct peri_dvs_channel {
	unsigned int      channel;
	unsigned int      volt_level;
	unsigned int      chan_valid;
	unsigned int      reg[DVS_MAX_REG];
	unsigned int      offset[DVS_MAX_REG];
	struct list_head  chan_list;
};

enum {
	VOTE_RECORD_PMC_REG_BASE,
	VOTE_RECORD_SCTRL_REG_BASE,
	VOTE_RECORD_MAX_REG_BASE,
};

struct peri_dvs_config {
	void __iomem *pmc_reg_base;
	void __iomem *sctrl_reg_base;
	unsigned int  lpmcu_offset;
	unsigned int  acpu_offset;
	unsigned int  max_volt_level;
	unsigned int  max_vote_channel;
	unsigned int  vote_width;
	unsigned int  lowtemp_mask;
	unsigned int  lowtemp_shift;
	unsigned int  fixed_volt_mask;
	unsigned int  fixed_volt_shift;
	unsigned int  volt_level_fixed;
	unsigned int  max_irq_usec;
	unsigned int  peri_dvs_enabled;
#ifdef CONFIG_HISI_CLK_DEBUG
	int dump_reg_num;
	unsigned int dump_reg[MAX_PERI_DVS_DUMP_REG_NUM];
#endif
};

#endif /* __HW_PERI_DVS_H__ */
