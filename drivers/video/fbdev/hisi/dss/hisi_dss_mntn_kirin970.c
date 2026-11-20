/* Copyright (c) 2018-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/errno.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/fs.h>
#include <mntn_subtype_exception.h>
#include "hisi_dss_mntn.h"
#include "hisi_dss_mntn_dss.h"
#include <soc_mid.h>
#include "soc_media1_crg_interface.h"
#include "soc_pmctrl_interface.h"
#include "soc_crgperiph_interface.h"


// peri crg PERPWRACK bit 5 indicates media subsys power state
#define MEDIA_SUBSYS_POWER_STATUS_BIT (0x20)

// pmctrl NOC_POWER_IDLE bit 15 indicates vivobus power state
#define VIVOBUS_POWER_STATUS_BIT (0x8000)

// media crg bit 6 indicates dss power state
#define DSS_POWER_STATUS_BIT (0x40)

#ifdef CONFIG_HISI_NOC_MODID_REGISTER

// if you don't care targetflow, you can set it to a default value 0xFF
#define MNTN_DEFAULT_TARGETFLOW    (0xFF)

static struct noc_err_para_s dss_noc_para[] = {
	{
		.masterid = (u32)SOC_DSS_CMDLIST_MID,
		.targetflow = MNTN_DEFAULT_TARGETFLOW,
		.bus = NOC_ERRBUS_VIVO,
	},
	{
		.masterid = (u32)SOC_DSS_WR_1_MID,
		.targetflow = MNTN_DEFAULT_TARGETFLOW,
		.bus = NOC_ERRBUS_VIVO,
	},
	{
		.masterid = (u32)SOC_DSS_WR_0_MID,
		.targetflow = MNTN_DEFAULT_TARGETFLOW,
		.bus = NOC_ERRBUS_VIVO,
	},
	{
		.masterid = (u32)SOC_DSS_RD_8_MID,
		.targetflow = MNTN_DEFAULT_TARGETFLOW,
		.bus = NOC_ERRBUS_VIVO,
	},
	{
		.masterid = (u32)SOC_DSS_RD_7_MID,
		.targetflow = MNTN_DEFAULT_TARGETFLOW,
		.bus = NOC_ERRBUS_VIVO,
	},
	{
		.masterid = (u32)SOC_DSS_RD_6_MID,
		.targetflow = MNTN_DEFAULT_TARGETFLOW,
		.bus = NOC_ERRBUS_VIVO,
	},
	{
		.masterid = (u32)SOC_DSS_RD_5_MID,
		.targetflow = MNTN_DEFAULT_TARGETFLOW,
		.bus = NOC_ERRBUS_VIVO,
	},
	{
		.masterid = (u32)SOC_DSS_RD_4_MID,
		.targetflow = MNTN_DEFAULT_TARGETFLOW,
		.bus = NOC_ERRBUS_VIVO,
	},
	{
		.masterid = (u32)SOC_DSS_RD_3_MID,
		.targetflow = MNTN_DEFAULT_TARGETFLOW,
		.bus = NOC_ERRBUS_VIVO,
	},
	{
		.masterid = (u32)SOC_DSS_RD_2_MID,
		.targetflow = MNTN_DEFAULT_TARGETFLOW,
		.bus = NOC_ERRBUS_VIVO,
	},
	{
		.masterid = (u32)SOC_DSS_RD_1_MID,
		.targetflow = MNTN_DEFAULT_TARGETFLOW,
		.bus = NOC_ERRBUS_VIVO,
	},
	{
		.masterid = (u32)SOC_DSS_RD_0_MID,
		.targetflow = MNTN_DEFAULT_TARGETFLOW,
		.bus = NOC_ERRBUS_VIVO,
	},
};

struct noc_err_para_s *dss_get_noc_err_para(uint32_t *p_noc_para_num)
{
	if (p_noc_para_num == NULL) {
		DSS_MNTN_ERR("p_noc_para_num is NULL!\n");
		return NULL;
	}

	*p_noc_para_num = ARRAY_SIZE(dss_noc_para);

	return dss_noc_para;
}
#endif


int dss_check_media_subsys_status(struct dpu_fb_data_type *dpufd)
{
	uint32_t ret;

	if (dpufd == NULL) {
		DSS_MNTN_ERR("dpufd is NULL!\n");
		return -1;
	}

	/* MEDIA SUBSYS status:
	 * 0: MEDIA SUBSYS power off;
	 * 1:MEDIA SUBSYS power on.
	 */
	ret = inp32(dpufd->peri_crg_base + PERPWRSTAT);
	DSS_MNTN_INFO("reg [0x%x] = 0x%x\n",
		dpufd->peri_crg_base + PERPWRSTAT, ret);
	if ((ret & MEDIA_SUBSYS_POWER_STATUS_BIT) ==
		MEDIA_SUBSYS_POWER_STATUS_BIT) {
		// bit 5 is 1 indicates media subsys is power on
		return 0;
	} else {
		return -1;
	}

}

static int dss_check_vivobus_power_status(struct dpu_fb_data_type *dpufd)
{
	uint32_t ret;

	if (dpufd == NULL) {
		DSS_MNTN_ERR("dpufd is NULL!\n");
		return -1;
	}

	/* 0: non idle; 1: idle,power off */
	ret = inp32(dpufd->pmctrl_base + NOC_POWER_IDLE);
	DSS_MNTN_INFO("reg [0x%x] = 0x%x\n",
		dpufd->pmctrl_base + NOC_POWER_IDLE, ret);
	if ((ret & VIVOBUS_POWER_STATUS_BIT) ==
		VIVOBUS_POWER_STATUS_BIT) {
		// bit 15 is 1 indicates media subsys is power off
		return -1;
	}
	return 0;
}

static int dss_check_dss_power_status(struct dpu_fb_data_type *dpufd)
{
	uint32_t ret;

	if (dpufd == NULL) {
		DSS_MNTN_ERR("dpufd is NULL!\n");
		return -1;
	}

	/* 0: power on; 1: power off */
	ret = inp32(dpufd->media_crg_base + MEDIA_PERRSTSTAT0);
	DSS_MNTN_INFO("reg [0x%x] = 0x%x\n",
		dpufd->media_crg_base + MEDIA_PERRSTSTAT0, ret);
	if ((ret & DSS_POWER_STATUS_BIT) == DSS_POWER_STATUS_BIT) {
		// bit 6 is 1 indicates dss is power off
		return -1;
	}
	return 0;
}

static void dss_media_subsys_power_up(struct dpu_fb_data_type *dpufd)
{
	uint32_t ret;

	// step 1: module mtcmos on
	outp32(dpufd->peri_crg_base + PERPWREN, 0x00000020);
	udelay(100);
	// step 1.1: module unrst
	outp32(dpufd->peri_crg_base + PERRSTDIS5, 0x00040000);
	// step 2: module clk enable
	outp32(dpufd->peri_crg_base + PEREN6, 0x7C002028);
	outp32(dpufd->sctrl_base + SCPEREN4, 0x00000040);
	outp32(dpufd->peri_crg_base + PEREN4, 0x00000040);
	udelay(1);
	// step 3: module clk disable
	outp32(dpufd->peri_crg_base + PERDIS6, 0x7C002028);
	outp32(dpufd->sctrl_base + SCPERDIS4, 0x00000040);
	outp32(dpufd->peri_crg_base + PERDIS4, 0x00000040);
	udelay(1);
	// step 4: module iso disable
	outp32(dpufd->peri_crg_base + ISODIS, 0x00000040);
	// step 5: memory rempair
	ret = inp32(dpufd->pctrl_base + PERI_STAT64);
	udelay(400);
	DSS_MNTN_INFO("read pctrl_base PERI_STAT64 = 0x%x\n", ret);
	// step 6: module unrst
	outp32(dpufd->peri_crg_base + PERRSTDIS5, 0x00020000);
	// step 7: module clk enable
	outp32(dpufd->peri_crg_base + PEREN6, 0x7C002028);
	outp32(dpufd->sctrl_base + SCPEREN4, 0x00000040);
	outp32(dpufd->peri_crg_base + PEREN4, 0x00000040);
}

static void dss_vivobus_power_up(struct dpu_fb_data_type *dpufd)
{
	uint32_t ret;

	// step 1: module mtcmos on
	// step 2: module clk enable
	outp32(dpufd->media_crg_base + MEDIA_CLKDIV9, 0x00080008);
	outp32(dpufd->media_crg_base + MEDIA_PEREN0, 0x08040040);
	udelay(1);
	// step 3: module clk disable
	outp32(dpufd->media_crg_base + MEDIA_PERDIS0, 0x00040040);
	udelay(1);
	// step 4: module iso disable
	// step 5: memory rempair
	// step 6: module unrst
	// step 7: module clk enable
	outp32(dpufd->media_crg_base + MEDIA_PEREN0, 0x00040040);
	// step 8: bus idle clear
	outp32(dpufd->pmctrl_base + NOC_POWER_IDLEREQ, 0x80000000);
	ret = inp32(dpufd->pmctrl_base + NOC_POWER_IDLEACK);
	udelay(1);
	DSS_MNTN_INFO("read pmctrl NOC_POWER_IDLEACK = 0x%x\n", ret);
	ret = inp32(dpufd->pmctrl_base + NOC_POWER_IDLE);
	udelay(1);
	DPU_FB_DEBUG("read pmctrl NOC_POWER_IDLE = 0x%x\n", ret);

}

static void dss_dss_power_up(struct dpu_fb_data_type *dpufd)
{
	uint32_t ret;
	// step 1: module mtcmos on
	// step 1.1: module unrst
	outp32(dpufd->media_crg_base + MEDIA_PERRSTDIS0, 0x02000000);
	// step 2: module clk enable
	outp32(dpufd->media_crg_base + MEDIA_CLKDIV9, 0xCA80CA80);
	outp32(dpufd->media_crg_base + MEDIA_PEREN0, 0x0009C000);
	outp32(dpufd->media_crg_base + MEDIA_PEREN1, 0x00660000);
	outp32(dpufd->media_crg_base + MEDIA_PEREN2, 0x0000003F);
	udelay(1);
	// step 3: module clk disable
	outp32(dpufd->media_crg_base + MEDIA_PERDIS0, 0x0009C000);
	outp32(dpufd->media_crg_base + MEDIA_PERDIS1, 0x00600000);
	outp32(dpufd->media_crg_base + MEDIA_PERDIS2, 0x0000003F);
	udelay(1);
	// step 4: module iso disable
	// step 5: memory rempair
	// step 6: module unrst
	outp32(dpufd->media_crg_base + MEDIA_PERRSTDIS0, 0x000000C0);
	outp32(dpufd->media_crg_base + MEDIA_PERRSTDIS1, 0x000000F0);
	// step 7: module clk enable
	outp32(dpufd->media_crg_base + MEDIA_PEREN0, 0x0009C000);
	outp32(dpufd->media_crg_base + MEDIA_PEREN1, 0x00600000);
	outp32(dpufd->media_crg_base + MEDIA_PEREN2, 0x0000003F);
	// step 8: bus idle clear
	outp32(dpufd->pmctrl_base + NOC_POWER_IDLEREQ, 0x20000000);
	ret = inp32(dpufd->pmctrl_base + NOC_POWER_IDLEACK);
	udelay(1);
	DSS_MNTN_INFO("read pmctrl NOC_POWER_IDLEACK = 0x%x\n", ret);
	ret = inp32(dpufd->pmctrl_base + NOC_POWER_IDLE);
	udelay(1);
	DSS_MNTN_INFO("read pmctrl NOC_POWER_IDLE = 0x%x\n", ret);
}

// dss power up
void dss_power_up(struct dpu_fb_data_type *dpufd)
{
	if (dpufd == NULL) {
		DSS_MNTN_ERR("dpufd is NULL!\n");
		return;
	}

	DSS_MNTN_INFO("+\n");

	/*** pu_media1_subsys ***/
	/* MEDIA SUBSYS status:
	 * 0:MEDIA SUBSYS power off;
	 * 1:MEDIA SUBSYS power on.
	 */
	if (dss_check_media_subsys_status(dpufd) != 0) {
		DSS_MNTN_INFO("media subsys to power up!\n");
		dss_media_subsys_power_up(dpufd);
	}

	/*** pu_vivobus ***/
	/* 0: non idle;1: idle,power off */
	if (dss_check_vivobus_power_status(dpufd) != 0) {
		DSS_MNTN_INFO("vivobus to power up!\n");
		dss_vivobus_power_up(dpufd);
	}

	/*** pu_dss ***/
	/* 0: power on; 1: power off */
	if (dss_check_dss_power_status(dpufd) != 0) {
		DSS_MNTN_INFO("dss to power up!\n");
		dss_dss_power_up((dpufd));
	}

	DSS_MNTN_INFO("-\n");
}

