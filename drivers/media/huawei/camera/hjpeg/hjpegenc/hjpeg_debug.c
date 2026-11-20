/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: provide interface for debug.
 * Author: lixiuhua
 * Create: 2017-02-28
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/io.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_data/remoteproc_hisp.h>

#include "hjpeg_debug.h"
#include "hjpeg160/hjpg_cfg_power_reg.h"
#include "hjpeg_common.h"
#include "cam_log.h"

#if (POWER_CTRL_INTERFACE == POWER_CTRL_CFG_REGS)
static void __iomem *g_viraddr_peri_crg = NULL;
static void __iomem *g_viraddr_sctr = NULL;
static void __iomem *g_viraddr_media_crg = NULL;
static void __iomem *g_viraddr_pmctrl = NULL;

static int cfg_reg_sets(struct reg_sets *regSets);
static int cfg_reg_sets_array(struct reg_sets *preg_sets, int count);

static void __iomem *get_viraddr(u32 phy_base);
static void reg_op_write_all(void __iomem *base, struct reg_sets *regSets);
static void reg_op_bit_equal(void __iomem *base,
	struct reg_sets *regSets, int bit);
static void reg_op_status_equal(void __iomem *base, struct reg_sets *regSets);

static void reg_op_write_all(void __iomem *base, struct reg_sets *regSets)
{
	set_reg_val((void __iomem*)((char*)base + regSets->offset),
		regSets->val);
	if (regSets->wait_us > 0)
		msleep(regSets->wait_us);
}

static void reg_op_bit_equal(void __iomem *base,
	struct reg_sets *regSets, int bit)
{
	u32 val;
	if (regSets->wait_us > 0)
		msleep(regSets->wait_us);
	val = get_reg_val((void __iomem*)((char*)base + regSets->offset)) &
		(1 << bit);
	val = val >> bit;
	if (val != regSets->val)
		cam_err("[%s] Failed: val()%d is not equal expected()%d",
			__FUNCTION__, val, regSets->val);
}

static void reg_op_status_equal(void __iomem *base, struct reg_sets *regSets)
{
	u32 val;
	val = get_reg_val((void __iomem*)((char*)base + regSets->offset));
	cam_info("[%s] STAT.%s, offset(0x%x), value(0x%x)",
		__FUNCTION__, regSets->reg_name, regSets->offset, val);
	if (regSets->wait_us == 0)
		val = ~val;
	val = val & regSets->val;
	if (val != regSets->val)
		cam_err("[%s] Failed: bits(0x%X) of value not expected()%d",
			__FUNCTION__, regSets->val, regSets->wait_us);
}

static void __iomem *get_viraddr(u32 phy_base)
{
	void __iomem *base = NULL;

	switch (phy_base) {
	case REG_BASE_PERI_CRG:
		base = g_viraddr_peri_crg;
		break;
	case REG_BASE_SCTR:
		base = g_viraddr_sctr;
		break;
	case REG_BASE_MEDIA_CRG:
		base = g_viraddr_media_crg;
		break;
	case REG_BASE_PMCTRL:
		base = g_viraddr_pmctrl;
		break;
	default:
		break;
	}
	return base;
}

static int cfg_reg_sets(struct reg_sets *reg_sets)
{
	int ret = 0;
	void __iomem *base = get_viraddr(reg_sets->base);

	if (!base) {
		cam_err("[%s] Failed: base viraddr of %s is NULL",
			__FUNCTION__, reg_sets->reg_name);
		return -EINVAL;
	}

	switch (reg_sets->op_flag) {
	case OP_WRITE_ALL: {
		reg_op_write_all(base, reg_sets);
		break;
	}
	case OP_READ_BIT15: {
		reg_op_bit_equal(base, reg_sets, 15); /* 15:the value of bit */
		break;
	}
	case OP_READ_BIT5: {
		reg_op_bit_equal(base, reg_sets, 5); /* 5:the value of bit */
		break;
	}
	case OP_READ_STATUS: {
		msleep(100); /* 100:100ms */
		reg_op_status_equal(base, reg_sets);
		/* fall through */
	}
	default:
		break;
	}

	cam_info("%s: set reg %s success", __FUNCTION__, reg_sets->reg_name);

	return ret;
}

static int cfg_reg_sets_array(struct reg_sets *preg_sets, int count)
{
	int ret = 0;
	int i;

	if (!preg_sets) {
		cam_err("[%s] Failed: reg sets is NULL", __FUNCTION__);
		return -EINVAL;
	}

	for (i = 0; i < count; i++) {
		ret = cfg_reg_sets(preg_sets + i);
		if (ret != 0) {
			cam_err("[%s] Failed: config reg sets ()%d, ret=%d",
				__FUNCTION__, i, ret);
			break;
		}
	}

	return ret;
}

int cfg_map_reg_base(void)
{
	cam_info("enter %s", __FUNCTION__);
	int ret = 0;
	/* 4096:memory size */
	g_viraddr_peri_crg = ioremap_nocache(REG_BASE_PERI_CRG, 4096);
	if (IS_ERR_OR_NULL(g_viraddr_peri_crg)) {
		cam_err("%s() %d remap peri_crg viraddr failed",
			__FUNCTION__, __LINE__);
		ret = -ENXIO;
		goto fail;
	}

	/* 4096:memory size */
	g_viraddr_media_crg = ioremap_nocache(REG_BASE_MEDIA_CRG, 4096);
	if (IS_ERR_OR_NULL(g_viraddr_media_crg)) {
		cam_err("%s() %dremap media_crg viraddr failed",
			__FUNCTION__, __LINE__);
		ret = -ENXIO;
		goto fail;
	}

	/* 4096:memory size */
	g_viraddr_sctr = ioremap_nocache(REG_BASE_SCTR, 4096);
	if (IS_ERR_OR_NULL(g_viraddr_sctr)) {
		cam_err("%s() %d remap sctr viraddr failed",
			__FUNCTION__, __LINE__);
		ret = -ENXIO;
		goto fail;
	}

	/* 4096:memory size */
	g_viraddr_pmctrl = ioremap_nocache(REG_BASE_PMCTRL, 4096);
	if (IS_ERR_OR_NULL(g_viraddr_pmctrl)) {
		cam_err("%s() %dremap pmctrl viraddr failed",
			__FUNCTION__, __LINE__);
		ret = -ENXIO;
		goto fail;
	}

	return ret;
fail:
	cfg_unmap_reg_base();
	return ret;
}

int cfg_unmap_reg_base(void)
{
	cam_info("enter %s", __FUNCTION__);
	if (g_viraddr_media_crg)
		iounmap((void*)g_viraddr_media_crg);

	if (g_viraddr_peri_crg)
		iounmap((void*)g_viraddr_peri_crg);

	if (g_viraddr_sctr)
		iounmap((void*)g_viraddr_sctr);

	if (g_viraddr_pmctrl)
		iounmap((void*)g_viraddr_pmctrl);

	g_viraddr_peri_crg    = NULL;
	g_viraddr_sctr        = NULL;
	g_viraddr_media_crg   = NULL;
	g_viraddr_pmctrl      = NULL;

	return 0;
}

int cfg_powerup_regs(void)
{
	int ret = 0;
	cam_info("%s: enter", __FUNCTION__);

	ret |= cfg_reg_sets_array(media_subsys_powerup,
		sizeof(media_subsys_powerup) / sizeof(struct reg_sets));
	if (ret != 0)
		cam_err("[%s] Failed: media subsys power up, ret=%d",
			__FUNCTION__, ret);
	ret |= cfg_reg_sets_array(vivobus_reg_sets_powerup,
		sizeof(vivobus_reg_sets_powerup) / sizeof(struct reg_sets));
	if (ret != 0)
		cam_err("[%s] Failed: vivobus power up, ret=%d",
			__FUNCTION__, ret);
	ret |= cfg_reg_sets_array(isp_reg_sets_powerup,
		sizeof(isp_reg_sets_powerup) / sizeof(struct reg_sets));
	if (ret != 0)
		cam_err("[%s] Failed: isp power up, ret=%d",
			__FUNCTION__, ret);

	return ret;
}

int cfg_powerdn_regs(void)
{
	int ret = 0;
	ret |= cfg_reg_sets_array(isp_reg_sets_powerdown,
		sizeof(isp_reg_sets_powerdown) / sizeof(struct reg_sets));
	if (ret != 0)
		cam_err("[%s] Failed: isp power down, ret=%d",
			__FUNCTION__, ret);

	return 0;
}
#endif /* (POWER_CTRL_INTERFACE==POWER_CTRL_CFG_REGS) */

#if defined(HISP120_CAMERA)
static void __iomem *g_jpeg_120_map_reg[4];
unsigned int jpeg_120_reg[4] = { /* 4: array size */
	0xE87FF000 + 0X00C, /* MEDIA1_CRG1 */
	0xE8420000 + 0X374, /* ISP_ISPSS_CTRL1 */
	0xE8420000 + 0X0010, /* ISP_ISPSS_CTRL2 */
	0xE8583000 + 0X700 /* ISP_SUBCTRL */
};

int hjpeg_120_map_reg(void)
{
	unsigned int i;
	cam_info("%s: enter", __FUNCTION__);
	for (i = 0; i < sizeof(jpeg_120_reg) / sizeof(int); i++) {
		/* 4:memory size */
		g_jpeg_120_map_reg[i] = ioremap_nocache(jpeg_120_reg[i], 4);
		if (IS_ERR_OR_NULL(g_jpeg_120_map_reg[i])) {
			cam_err("ioremap failed 0x%x", jpeg_120_reg[i]);
			hjpeg_120_unmap_reg();
			return -ENXIO;
		}
	}
	return 0;
}

void hjpeg_120_unmap_reg(void)
{
	unsigned int i;
	cam_info("%s: enter", __FUNCTION__);
	for (i = 0; i < sizeof(jpeg_120_reg) / sizeof(int); i++) {
		if (g_jpeg_120_map_reg[i]) {
			iounmap((void*)g_jpeg_120_map_reg[i]);
			g_jpeg_120_map_reg[i] = NULL;
		}
	}
}

void hjpeg_120_dump_reg(void)
{
	unsigned int i;
	int ret;
	cam_info("%s: enter", __FUNCTION__);
	ret = get_ispcpu_cfg_info();
	if (ret < 0)
		cam_err("get_ispcpu_cfg_info:%d", ret);

	for (i = 0; i < sizeof(jpeg_120_reg) / sizeof(int); i++) {
		if (g_jpeg_120_map_reg[i])
			cam_err("hjpeg read reg:0x%x = 0x%x",
				jpeg_120_reg[i],
				get_reg_val(g_jpeg_120_map_reg[i]));
	}
}
#endif
