/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
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
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <dpu/soc_dpu_define.h>

#include "mipi_dsi_dev.h"
#include "dkmd_utils.h"

/* global definition for the cmd queue which will be send after next vactive start */
static bool g_mipi_trans_lock_inited = false;
static spinlock_t g_mipi_trans_lock;

void mipi_transfer_lock_init(void)
{
	if (!g_mipi_trans_lock_inited) {
		g_mipi_trans_lock_inited = true;
		spin_lock_init(&g_mipi_trans_lock);
	}
}

static void mipi_dsi_sread_request(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	uint32_t hdr = 0;

	/* fill up header */
	hdr |= dsi_hdr_dtype(cm->dtype);
	hdr |= dsi_hdr_vc(cm->vc);
	hdr |= dsi_hdr_data1((uint32_t)(cm->payload[0]));  /*lint !e571*/
	hdr |= dsi_hdr_data2(0);
#ifdef MIPI_DSI_HOST_VID_LP_MODE
	/* used for low power cmds trans under video mode */
	hdr |= cm->dtype & GEN_VID_LP_CMD;
	set_reg(DPU_DSI_APB_WR_LP_HDR_ADDR(dsi_base), hdr, 25, 0);
#else
	set_reg(DPU_DSI_APB_WR_LP_HDR_ADDR(dsi_base), hdr, 24, 0);
#endif
}

/*
 * mipi dsi short write with 0, 1 2 parameters
 * Write to GEN_HDR 24 bit register the value:
 * 1. 00h, MCS_command[15:8] ,VC[7:6],13h
 * 2. Data1[23:16], MCS_command[15:8] ,VC[7:6],23h
 */
static int mipi_dsi_swrite(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	uint32_t hdr = 0;
	int len;

	if (cm->dlen && cm->payload == 0) {
		pr_err("NO payload error!\n");
		return 0;
	}

	if (cm->dlen > 2) { /* mipi dsi short write with 0, 1 2 parameters, total 3 param */
		pr_err("cm->dlen is invalid");
		return -EINVAL;
	}
	len = cm->dlen;

	hdr |= dsi_hdr_dtype(cm->dtype);
	hdr |= dsi_hdr_vc(cm->vc);
	if (len == 1) {
		hdr |= dsi_hdr_data1((uint32_t)(cm->payload[0]));  /*lint !e571*/
		hdr |= dsi_hdr_data2(0);
	} else if (len == 2) {
		hdr |= dsi_hdr_data1((uint32_t)(cm->payload[0]));  /*lint !e571*/
		hdr |= dsi_hdr_data2((uint32_t)(cm->payload[1]));  /*lint !e571*/
	} else {
		hdr |= dsi_hdr_data1(0);
		hdr |= dsi_hdr_data2(0);
	}

	/* used for low power cmds trans under video mode */
	hdr |= cm->dtype & GEN_VID_LP_CMD;
	set_reg(DPU_DSI_APB_WR_LP_HDR_ADDR(dsi_base), hdr, 25, 0);

	pr_debug("hdr = 0x%x!\n", hdr);
	return len;  /* 4 bytes */
}

/*
 * mipi dsi long write
 * Write to GEN_PLD_DATA 32 bit register the value:
 * Data3[31:24], Data2[23:16], Data1[15:8], MCS_command[7:0]
 * If need write again to GEN_PLD_DATA 32 bit register the value:
 * Data7[31:24], Data6[23:16], Data5[15:8], Data4[7:0]
 *
 * Write to GEN_HDR 24 bit register the value: WC[23:8] ,VC[7:6],29h
 */

static int mipi_dsi_lwrite(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	uint32_t hdr = 0;
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t pld = 0;

	if (cm->dlen && cm->payload == 0) {
		pr_err("NO payload error!\n");
		return 0;
	}

	/* fill up payload, 4bytes set reg, remain 1 byte(8 bits) set reg */
	for (i = 0;  i < cm->dlen; i += 4) {
		if ((i + 4) <= cm->dlen) {
			pld = *((uint32_t *)(cm->payload + i));
		} else {
			for (j = i; j < cm->dlen; j++)
				pld |= (((uint32_t)cm->payload[j] & 0x0ff) << ((j - i) * 8));  /*lint !e571*/

			pr_debug("pld = 0x%x!\n", pld);
		}

		set_reg(DPU_DSI_APB_WR_LP_PLD_DATA_ADDR(dsi_base), pld, 32, 0);
		pld = 0;
	}

	/* fill up header */
	hdr |= dsi_hdr_dtype(cm->dtype);
	hdr |= dsi_hdr_vc(cm->vc);
	hdr |= dsi_hdr_wc(cm->dlen);

	/* used for low power cmds trans under video mode */
	hdr |= cm->dtype & GEN_VID_LP_CMD;
	set_reg(DPU_DSI_APB_WR_LP_HDR_ADDR(dsi_base), hdr, 25, 0);

	pr_debug("hdr = 0x%x!\n", hdr);

	return cm->dlen;
}

void mipi_dsi_max_return_packet_size(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	uint32_t hdr = 0;

	/* fill up header */
	hdr |= dsi_hdr_dtype(cm->dtype);
	hdr |= dsi_hdr_vc(cm->vc);
	hdr |= dsi_hdr_wc(cm->dlen);
	set_reg(DPU_DSI_APB_WR_LP_HDR_ADDR(dsi_base), hdr, 24, 0);
}

uint32_t mipi_dsi_read(uint32_t *out, const char __iomem *dsi_base)
{
	uint32_t pkg_status;
	uint32_t try_times = 700;  /* 35ms(50*700) */

	do {
		pkg_status = inp32(DPU_DSI_CMD_PLD_BUF_STATUS_ADDR(dsi_base));
		if (!(pkg_status & 0x10))
			break;
		udelay(50);  /* 50us */
	} while (--try_times);

	*out = inp32(DPU_DSI_APB_WR_LP_PLD_DATA_ADDR(dsi_base));
	if (!try_times)
		pr_err("CMD_PKT_STATUS[0x%x], PHY_STATUS[0x%x], INT_ST0[0x%x], INT_ST1[0x%x]\n",
			inp32(DPU_DSI_CMD_PLD_BUF_STATUS_ADDR(dsi_base)),
			inp32(DPU_DSI_CDPHY_STATUS_ADDR(dsi_base)),
			inp32(DPU_DSI_INT_ERROR_FORCE0_ADDR(dsi_base)),
			inp32(DPU_DSI_INT_ERROR_FORCE1_ADDR(dsi_base)));

	return try_times;
}

void mipi_dsi_sread(uint32_t *out, const char __iomem *dsi_base)
{
	unsigned long dw_jiffies;
	uint32_t temp = 0;

	/*
	 * jiffies:Current total system clock ticks
	 * dw_jiffies:timeout
	 */
	dw_jiffies = jiffies + HZ / 2;  /* HZ / 2 = 0.5s */
	do {
		temp = inp32(DPU_DSI_CMD_PLD_BUF_STATUS_ADDR(dsi_base));
		if ((temp & 0x00000040) == 0x00000040)
			break;
	} while (time_after(dw_jiffies, jiffies));

	dw_jiffies = jiffies + HZ / 2;  /* HZ / 2 = 0.5s */
	do {
		temp = inp32(DPU_DSI_CMD_PLD_BUF_STATUS_ADDR(dsi_base));
		if ((temp & 0x00000040) != 0x00000040)
			break;
	} while (time_after(dw_jiffies, jiffies));

	*out = inp32(DPU_DSI_APB_WR_LP_PLD_DATA_ADDR(dsi_base));
}

void mipi_dsi_lread(uint32_t *out, char __iomem *dsi_base)
{
	/* do something here */
}

int mipi_dsi_cmd_is_read(struct dsi_cmd_desc *cm)
{
	int ret;

	if (!cm)
		return -1;

	switch (dsi_hdr_dtype(cm->dtype)) {
	case DTYPE_GEN_READ:
	case DTYPE_GEN_READ1:
	case DTYPE_GEN_READ2:
	case DTYPE_DCS_READ:
		ret = 1;
		break;
	default:
		ret = 0;
		break;
	}

	return ret;
}

int mipi_dsi_lread_reg(uint32_t *out, struct dsi_cmd_desc *cm, uint32_t len, char *dsi_base)
{
	int ret = 0;
	uint32_t i = 0;
	struct dsi_cmd_desc packet_size_cmd_set;

	if (!cm)
		return -1;

	if (!dsi_base)
		return -1;

	if (mipi_dsi_cmd_is_read(cm)) {
		packet_size_cmd_set.dtype = DTYPE_MAX_PKTSIZE;
		packet_size_cmd_set.vc = 0;
		packet_size_cmd_set.dlen = len;
		mipi_dsi_max_return_packet_size(&packet_size_cmd_set, dsi_base);
		mipi_dsi_sread_request(cm, dsi_base);
		for (i = 0; i < (len + 3) / 4; i++) {  /* read 4 bytes once */
			if (!mipi_dsi_read(out, dsi_base)) {
				ret = -1;
				pr_err("Read register 0x%x timeout\n", cm->payload[0]);
				break;
			}
			out++;
		}
	} else {
		ret = -1;
		pr_err("dtype=%x NOT supported!\n", cm->dtype);
	}

	return ret;
}

static void delay_for_next_cmd_by_sleep(uint32_t wait, uint32_t waittype)
{
	if (wait) {
		if (waittype == WAIT_TYPE_US) {
			udelay(wait);
		} else if (waittype == WAIT_TYPE_MS) {
			if (wait <= 10)  /* less then 10ms, use mdelay() */
				mdelay((unsigned long)wait);
			else
				msleep(wait);
		} else {
			msleep(wait * 1000);  /* ms */
		}
	}
}

/*
 * prepare cmd buffer to be txed
 */
static int mipi_dsi_cmd_add(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	int len = 0;
	unsigned long flags = 0;

	spin_lock_irqsave(&g_mipi_trans_lock, flags);

	switch (dsi_hdr_dtype(cm->dtype)) {
	case DTYPE_GEN_WRITE:
	case DTYPE_GEN_WRITE1:
	case DTYPE_GEN_WRITE2:

	case DTYPE_DCS_WRITE:
	case DTYPE_DCS_WRITE1:
	case DTYPE_DCS_WRITE2:
		len = mipi_dsi_swrite(cm, dsi_base);
		break;
	case DTYPE_GEN_LWRITE:
	case DTYPE_DCS_LWRITE:
	case DTYPE_DSC_LWRITE:

		len = mipi_dsi_lwrite(cm, dsi_base);
		break;
	default:
		pr_err("dtype=%x NOT supported!\n", cm->dtype);
		break;
	}

	spin_unlock_irqrestore(&g_mipi_trans_lock, flags);

	return len;
}

int mipi_dsi_cmds_tx(struct dsi_cmd_desc *cmds, int cnt, char __iomem *dsi_base)
{
	struct dsi_cmd_desc *cm = NULL;
	int i;

	if (!cmds)
		return -1;

	if (!dsi_base)
		return -1;

	mipi_transfer_lock_init();

	cm = cmds;
	for (i = 0; i < cnt; i++) {
		mipi_dsi_cmd_add(cm, dsi_base);
		delay_for_next_cmd_by_sleep(cm->wait, cm->waittype);
		cm++;
	}

	return cnt;
}
EXPORT_SYMBOL(mipi_dsi_cmds_tx);

MODULE_LICENSE("GPL");