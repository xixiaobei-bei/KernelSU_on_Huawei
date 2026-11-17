/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_mipi_dsi.h"

/* global definition for the cmd queue which will be send after next vactive start */
static struct dsi_delayed_cmd_queue g_delayed_cmd_queue;
static bool g_delayed_cmd_queue_inited = false;
static bool g_mipi_trans_lock_inited = false;
static spinlock_t g_mipi_trans_lock;

static void mipi_dsi_sread_request(struct dsi_cmd_desc *cm, char __iomem *dsi_base);

static int mipi_dsi_delayed_cmd_queue_write(struct dsi_cmd_desc *pCmdset, int Cmdset_cnt, bool is_low_priority);

char __iomem *get_mipi_dsi_base(struct dpu_fb_data_type *dpufd)
{
	char __iomem *mipi_dsi_base = NULL;

	mipi_dsi_base = dpufd->mipi_dsi0_base;
	if (is_dsi1_pipe_switch_connector(dpufd))
		mipi_dsi_base = dpufd->mipi_dsi1_base;

	return mipi_dsi_base;
}

uint32_t get_mipi_dsi_index(struct dpu_fb_data_type *dpufd)
{
	uint32_t mipi_idx = DSI0_INDEX;

	if (is_dsi1_pipe_switch_connector(dpufd))
		mipi_idx = DSI1_INDEX;

	return mipi_idx;
}

static inline void mipi_dsi_cmd_send_lock(void)
{
	if (g_delayed_cmd_queue_inited)
		spin_lock(&g_delayed_cmd_queue.cmd_send_lock);
}

static inline void mipi_dsi_cmd_send_unlock(void)
{
	if (g_delayed_cmd_queue_inited)
		spin_unlock(&g_delayed_cmd_queue.cmd_send_lock);
}

void mipi_transfer_lock_init(void)
{
	if (!g_mipi_trans_lock_inited) {
		g_mipi_trans_lock_inited = true;
		spin_lock_init(&g_mipi_trans_lock);
	}
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
		DPU_FB_ERR("NO payload error!\n");
		return 0;
	}

	if (cm->dlen > 2) { /* mipi dsi short write with 0, 1 2 parameters, total 3 param */
		DPU_FB_ERR("cm->dlen is invalid");
		return -EINVAL;
	}
	len = cm->dlen;


	hdr |= DSI_HDR_DTYPE(cm->dtype);
	hdr |= DSI_HDR_VC(cm->vc);
	if (len == 1) {
		hdr |= DSI_HDR_DATA1((uint32_t)(cm->payload[0]));  /*lint !e571*/
		hdr |= DSI_HDR_DATA2(0);
	} else if (len == 2) {
		hdr |= DSI_HDR_DATA1((uint32_t)(cm->payload[0]));  /*lint !e571*/
		hdr |= DSI_HDR_DATA2((uint32_t)(cm->payload[1]));  /*lint !e571*/
	} else {
		hdr |= DSI_HDR_DATA1(0);
		hdr |= DSI_HDR_DATA2(0);
	}

	/* used for low power cmds trans under video mode */
	hdr |= cm->dtype & GEN_VID_LP_CMD;
	set_reg(dsi_base + MIPIDSI_GEN_HDR_OFFSET, hdr, 25, 0);

	DPU_FB_DEBUG("hdr=0x%x!\n", hdr);
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
		DPU_FB_ERR("NO payload error!\n");
		return 0;
	}

	/* fill up payload, 4bytes set reg, remain 1 byte(8 bits) set reg */
	for (i = 0;  i < cm->dlen; i += 4) {
		if ((i + 4) <= cm->dlen) {
			pld = *((uint32_t *)(cm->payload + i));
		} else {
			for (j = i; j < cm->dlen; j++)
				pld |= (((uint32_t)cm->payload[j] & 0x0ff) << ((j - i) * 8));  /*lint !e571*/

			DPU_FB_DEBUG("pld=0x%x!\n", pld);
		}

		set_reg(dsi_base + MIPIDSI_GEN_PLD_DATA_OFFSET, pld, 32, 0);
		pld = 0;
	}

	/* fill up header */
	hdr |= DSI_HDR_DTYPE(cm->dtype);
	hdr |= DSI_HDR_VC(cm->vc);
	hdr |= DSI_HDR_WC(cm->dlen);

	/* used for low power cmds trans under video mode */
	hdr |= cm->dtype & GEN_VID_LP_CMD;
	set_reg(dsi_base + MIPIDSI_GEN_HDR_OFFSET, hdr, 25, 0);

	DPU_FB_DEBUG("hdr=0x%x!\n", hdr);

	return cm->dlen;
}

void mipi_dsi_max_return_packet_size(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	uint32_t hdr = 0;

	/* fill up header */
	hdr |= DSI_HDR_DTYPE(cm->dtype);
	hdr |= DSI_HDR_VC(cm->vc);
	hdr |= DSI_HDR_WC(cm->dlen);
	set_reg(dsi_base + MIPIDSI_GEN_HDR_OFFSET, hdr, 24, 0);
}

uint32_t mipi_dsi_read(uint32_t *out, const char __iomem *dsi_base)
{
	uint32_t pkg_status;
	uint32_t try_times = 700;  /* 35ms(50*700) */

	do {
		pkg_status = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		if (!(pkg_status & 0x10))
			break;
		udelay(50);  /* 50us */
	} while (--try_times);

	*out = inp32(dsi_base + MIPIDSI_GEN_PLD_DATA_OFFSET);
	if (!try_times)
		DPU_FB_ERR("%s, CMD_PKT_STATUS[0x%x], PHY_STATUS[0x%x], INT_ST0[0x%x], INT_ST1[0x%x]\n",
			__func__, inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base + MIPIDSI_INT_ST0_OFFSET), inp32(dsi_base + MIPIDSI_INT_ST1_OFFSET));

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
		temp = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		if ((temp & 0x00000040) == 0x00000040)
			break;
	} while (time_after(dw_jiffies, jiffies));

	dw_jiffies = jiffies + HZ / 2;  /* HZ / 2 = 0.5s */
	do {
		temp = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		if ((temp & 0x00000040) != 0x00000040)
			break;
	} while (time_after(dw_jiffies, jiffies));

	*out = inp32(dsi_base + MIPIDSI_GEN_PLD_DATA_OFFSET);
}

void mipi_dsi_lread(uint32_t *out, char __iomem *dsi_base)
{
	/* do something here */
}

int mipi_dsi_cmd_is_read(struct dsi_cmd_desc *cm)
{
	int ret;

	dpu_check_and_return(!cm, -1, ERR, "cmds is NULL!\n");

	switch (DSI_HDR_DTYPE(cm->dtype)) {
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

	dpu_check_and_return(!cm, -1, ERR, "cmds is NULL!\n");

	dpu_check_and_return(!dsi_base, -1, ERR, "dsi_base is NULL!\n");

	if (mipi_dsi_cmd_is_read(cm)) {
		packet_size_cmd_set.dtype = DTYPE_MAX_PKTSIZE;
		packet_size_cmd_set.vc = 0;
		packet_size_cmd_set.dlen = len;
		mipi_dsi_max_return_packet_size(&packet_size_cmd_set, dsi_base);
		mipi_dsi_sread_request(cm, dsi_base);
		for (i = 0; i < (len + 3) / 4; i++) {  /* read 4 bytes once */
			if (!mipi_dsi_read(out, dsi_base)) {
				ret = -1;
				DPU_FB_ERR("Read register 0x%X timeout\n", cm->payload[0]);
				break;
			}
			out++;
		}
	} else {
		ret = -1;
		DPU_FB_ERR("dtype=%x NOT supported!\n", cm->dtype);
	}

	return ret;
}

static void delay_for_next_cmd_by_sleep(uint32_t wait, uint32_t waittype)
{
	if (wait) {
		DPU_FB_DEBUG("waittype=%d, wait=%d\n", waittype, wait);
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
int mipi_dsi_cmd_add(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	int len = 0;
	unsigned long flags = 0;

	dpu_check_and_return(!cm, -EINVAL, ERR, "cmds is NULL!\n");

	dpu_check_and_return(!dsi_base, -EINVAL, ERR, "dsi_base is NULL!\n");

	mipi_dsi_cmd_send_lock();
	spin_lock_irqsave(&g_mipi_trans_lock, flags);

	switch (DSI_HDR_DTYPE(cm->dtype)) {
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
		DPU_FB_ERR("dtype=%x NOT supported!\n", cm->dtype);
		break;
	}

	spin_unlock_irqrestore(&g_mipi_trans_lock, flags);
	mipi_dsi_cmd_send_unlock();

	return len;
}

int mipi_dsi_cmds_tx(struct dsi_cmd_desc *cmds, int cnt, char __iomem *dsi_base)
{
	struct dsi_cmd_desc *cm = NULL;
	int i;

	dpu_check_and_return(!cmds, -EINVAL, ERR, "cmds is NULL!\n");

	dpu_check_and_return(!dsi_base, -EINVAL, ERR, "dsi_base is NULL!\n");

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		mipi_dsi_cmd_add(cm, dsi_base);
		delay_for_next_cmd_by_sleep(cm->wait, cm->waittype);
		cm++;
	}

	return cnt;
}

void mipi_dsi_check_0lane_is_ready(const char __iomem *dsi_base)
{
	unsigned long dw_jiffies;
	uint32_t temp = 0;

	dw_jiffies = jiffies + HZ / 10;  /* HZ / 10 = 0.1s */
	do {
		temp = inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		/* phy_stopstate0lane */
		if ((temp & 0x10) == 0x10) {
			DPU_FB_INFO("0 lane is stopping state\n");
			return;
		}
	} while (time_after(dw_jiffies, jiffies));

	DPU_FB_ERR("0 lane is not stopping state:tmp=0x%x\n", temp);
}

static void mipi_dsi_sread_request(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	uint32_t hdr = 0;

	/* fill up header */
	hdr |= DSI_HDR_DTYPE(cm->dtype);
	hdr |= DSI_HDR_VC(cm->vc);
	hdr |= DSI_HDR_DATA1((uint32_t)(cm->payload[0]));  /*lint !e571*/
	hdr |= DSI_HDR_DATA2(0);
#ifdef MIPI_DSI_HOST_VID_LP_MODE
	/* used for low power cmds trans under video mode */
	hdr |= cm->dtype & GEN_VID_LP_CMD;
	set_reg(dsi_base + MIPIDSI_GEN_HDR_OFFSET, hdr, 25, 0);
#else
	set_reg(dsi_base + MIPIDSI_GEN_HDR_OFFSET, hdr, 24, 0);
#endif
}

static int mipi_dsi_read_phy_status(const struct dsi_cmd_desc *cm, const char __iomem *dsi_base)
{
	unsigned long dw_jiffies;
	uint32_t pkg_status;
	uint32_t phy_status;
	int is_timeout = 1;

	/* read status register */
	dw_jiffies = jiffies + HZ;
	do {
		pkg_status = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		phy_status = inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		if ((pkg_status & 0x1) == 0x1 && !(phy_status & 0x2)) {
			is_timeout = 0;
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

	if (is_timeout) {
		DPU_FB_ERR("mipi_dsi_read timeout :0x%x\n"
			"IPIDSI_CMD_PKT_STATUS = 0x%x\n"
			"IPIDSI_PHY_STATUS = 0x%x\n"
			"IPIDSI_INT_ST1_OFFSET = 0x%x\n",
			cm->payload[0],
			inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base + MIPIDSI_INT_ST1_OFFSET));
		return -1;
	}

	return 0;
}

static int mipi_dsi_wait_data_status(const struct dsi_cmd_desc *cm, const char __iomem *dsi_base)
{
	unsigned long dw_jiffies;
	uint32_t pkg_status;
	int is_timeout = 1;

	/* wait dsi read data */
	dw_jiffies = jiffies + HZ;
	do {
		pkg_status = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		if (!(pkg_status & 0x10)) {
			is_timeout = 0;
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

	if (is_timeout) {
		DPU_FB_ERR("mipi_dsi_read timeout :0x%x\n"
			"IPIDSI_CMD_PKT_STATUS = 0x%x\n"
			"IPIDSI_PHY_STATUS = 0x%x\n",
			cm->payload[0],
			inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET));
		return -1;
	}

	return 0;
}

static int mipi_dsi_read_add(uint32_t *out, struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	int ret = 0;

	dpu_check_and_return(!cm, -EINVAL, ERR, "cmds is NULL!\n");

	dpu_check_and_return(!dsi_base, -EINVAL, ERR, "dsi_base is NULL!\n");

	if (DSI_HDR_DTYPE(cm->dtype) == DTYPE_DCS_READ) {
		mipi_dsi_cmd_send_lock();

		mipi_dsi_sread_request(cm, dsi_base);

		if (!mipi_dsi_read(out, dsi_base)) {
			DPU_FB_ERR("Read register 0x%X timeout\n", cm->payload[0]);
			ret = -1;
		}

		mipi_dsi_cmd_send_unlock();
	} else if (cm->dtype == DTYPE_GEN_READ1) {
		ret = mipi_dsi_read_phy_status(cm, dsi_base);
		if (ret)
			return ret;

		mipi_dsi_cmd_send_lock();
		/* send read cmd to fifo */
#ifdef MIPI_DSI_HOST_VID_LP_MODE
		/* used for low power cmds trans under video mode */
		/*lint -e647*/
		set_reg(dsi_base + MIPIDSI_GEN_HDR_OFFSET, ((((uint32_t)(uint8_t)cm->payload[0]) << 8)
			| (cm->dtype & GEN_VID_LP_CMD)), 25, 0);
		/*lint +e647*/
#else
		set_reg(dsi_base + MIPIDSI_GEN_HDR_OFFSET, ((((uint32_t)(uint8_t)cm->payload[0]) << 8)
			| cm->dtype), 24, 0);
#endif

		ret = mipi_dsi_wait_data_status(cm, dsi_base);
		if (ret) {
			mipi_dsi_cmd_send_unlock();
			return ret;
		}

		/* get read data */
		*out = inp32(dsi_base + MIPIDSI_GEN_PLD_DATA_OFFSET);

		mipi_dsi_cmd_send_unlock();
	} else {
		ret = -1;
		DPU_FB_ERR("dtype=%x NOT supported!\n", cm->dtype);
	}

	return ret;
}

int mipi_dsi_cmds_rx(uint32_t *out, struct dsi_cmd_desc *cmds, int cnt,
	char __iomem *dsi_base)
{
	struct dsi_cmd_desc *cm = NULL;
	int i;
	int err_num = 0;

	dpu_check_and_return(!cmds, -EINVAL, ERR, "cmds is NULL!\n");

	dpu_check_and_return(!dsi_base, -EINVAL, ERR, "dsi_base is NULL!\n");

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		if (mipi_dsi_read_add(&(out[i]), cm, dsi_base))
			err_num++;
		delay_for_next_cmd_by_sleep(cm->wait, cm->waittype);
		cm++;
	}

	return err_num;
}

int mipi_dsi_read_compare(struct mipi_dsi_read_compare_data *data, char __iomem *dsi_base)
{
	uint32_t *read_value = NULL;
	uint32_t *expected_value = NULL;
	uint32_t *read_mask = NULL;
	char **reg_name = NULL;
	int log_on;
	struct dsi_cmd_desc *cmds = NULL;

	int cnt;
	int cnt_not_match = 0;
	int ret;
	int i;

	dpu_check_and_return(!data, -EINVAL, ERR, "data is NULL!\n");

	dpu_check_and_return(!dsi_base, -EINVAL, ERR, "dsi_base is NULL!\n");

	read_value = data->read_value;
	expected_value = data->expected_value;
	read_mask = data->read_mask;
	reg_name = data->reg_name;
	log_on = data->log_on;

	cmds = data->cmds;
	cnt = data->cnt;

	ret = mipi_dsi_cmds_rx(read_value, cmds, cnt, dsi_base);
	if (ret) {
		DPU_FB_ERR("Read error number: %d\n", ret);
		return cnt;
	}

	for (i = 0; i < cnt; i++) {
		if (log_on)
			DPU_FB_INFO("Read reg %s: 0x%x, value = 0x%x\n",
				reg_name[i], cmds[i].payload[0], read_value[i]);

		if (expected_value[i] != (read_value[i] & read_mask[i]))
			cnt_not_match++;
	}

	return cnt_not_match;
}

static int mipi_dsi_fifo_is_full(const char __iomem *dsi_base)
{
	uint32_t pkg_status;
	uint32_t phy_status;
	int is_timeout = 100;  /* 10ms(100*100us) */

	dpu_check_and_return(!dsi_base, -EINVAL, ERR, "dsi_base is NULL!\n");

	/* read status register */
	do {
		pkg_status = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		phy_status = inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		if ((pkg_status & 0x2) != 0x2 && !(phy_status & 0x2))
			break;
		udelay(100);  /* 100us */
	} while (is_timeout-- > 0);

	if (is_timeout < 0) {
		DPU_FB_ERR("mipi check full fail:\n"
			"IPIDSI_CMD_PKT_STATUS = 0x%x\n"
			"IPIDSI_PHY_STATUS = 0x%x\n"
			"IPIDSI_INT_ST1_OFFSET = 0x%x\n",
			inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base + MIPIDSI_INT_ST1_OFFSET));
		return -1;
	}
	return 0;
}

int mipi_dsi_cmds_tx_with_check_fifo(struct dsi_cmd_desc *cmds, int cnt, char __iomem *dsi_base)
{
	struct dsi_cmd_desc *cm = NULL;
	int i;

	dpu_check_and_return(!cmds, -EINVAL, ERR, "cmds is NULL!\n");

	dpu_check_and_return(!dsi_base, -EINVAL, ERR, "dsi_base is NULL!\n");

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		if (!mipi_dsi_fifo_is_full(dsi_base)) {
			mipi_dsi_cmd_add(cm, dsi_base);
		} else {
			DPU_FB_ERR("dsi fifo full, write [%d] cmds, left [%d] cmds!!", i, cnt-i);
			break;
		}
		delay_for_next_cmd_by_sleep(cm->wait, cm->waittype);
		cm++;
	}

	return cnt;
}

int mipi_dsi_cmds_rx_with_check_fifo(uint32_t *out, struct dsi_cmd_desc *cmds, int cnt, char __iomem *dsi_base)
{
	struct dsi_cmd_desc *cm = NULL;
	int i;
	int err_num = 0;

	dpu_check_and_return(!cmds, -EINVAL, ERR, "cmds is NULL!\n");
	dpu_check_and_return(!dsi_base, -EINVAL, ERR, "dsi_base is NULL!\n");

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		if (!mipi_dsi_fifo_is_full(dsi_base)) {
			if (mipi_dsi_read_add(&(out[i]), cm, dsi_base))
				err_num++;
		} else {
			err_num += (cnt - i);
			DPU_FB_ERR("dsi fifo full, read [%d] cmds, left [%d] cmds!!", i, cnt - i);
			break;
		}
		delay_for_next_cmd_by_sleep(cm->wait, cm->waittype);
		cm++;
	}

	return err_num;
}

static void delay_by_msleep(uint32_t usec)
{
	uint32_t msec;

	if (usec > 1000) {  /* 1ms */
		msec = usec / 1000;  /* calculate integer */
		usec = usec - msec * 1000;  /* calculate decimal */

		if (msec <= 10)  /* less then 10ms, use mdelay() */
			mdelay((unsigned long)msec);
		else
			msleep(msec);
		if (usec)
			udelay(usec);
	} else if (usec) {
		udelay(usec);
	}
}

static uint32_t calc_next_wait_time(uint32_t wait, uint32_t waittype)
{
	if (wait) {
		if (waittype == WAIT_TYPE_US)
			return wait;
		else if (waittype == WAIT_TYPE_MS)
			return wait * 1000;  /* ms to us, 1ms = 1000us */
		else
			return wait * 1000 * 1000;  /* other to us */
	}

	return 0;
}

static int mipi_dual_dsi_read(uint32_t *ValueOut_0, uint32_t *ValueOut_1,
	const char __iomem *dsi_base_0, const char __iomem *dsi_base_1)
{
	uint32_t pkg_status_0 = 0;
	uint32_t pkg_status_1 = 0;
	uint32_t try_times = 700;  /* 35ms(50*700) */
	bool read_done_0 = false;
	bool read_done_1 = false;
	int ret = 2;

	dpu_check_and_return((!dsi_base_0 || !dsi_base_1), 0, ERR, "dsi_base is NULL!\n");

	dpu_check_and_return((!ValueOut_0 || !ValueOut_1), 0, ERR, "out buffer is NULL!\n");

	do {
		if (!read_done_0) {
			pkg_status_0 = inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
			if (!(pkg_status_0 & 0x10)) {
				read_done_0 = true;
				*ValueOut_0 = inp32(dsi_base_0 + MIPIDSI_GEN_PLD_DATA_OFFSET);
			}
		}
		if (!read_done_1) {
			pkg_status_1 = inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
			if (!(pkg_status_1 & 0x10)) {
				read_done_1 = true;
				*ValueOut_1 = inp32(dsi_base_1 + MIPIDSI_GEN_PLD_DATA_OFFSET);
			}
		}

		if (read_done_0 && read_done_1)
			break;
		udelay(50);  /* 50us */
	} while (--try_times);

	if (!read_done_0) {
		ret -= 1;
		DPU_FB_ERR("%s, DSI0 CMD_PKT_STATUS[0x%x], PHY_STATUS[0x%x], INT_ST0[0x%x], INT_ST1[0x%x]\n",
			__func__,
			inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base_0 + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base_0 + MIPIDSI_INT_ST0_OFFSET), inp32(dsi_base_0 + MIPIDSI_INT_ST1_OFFSET));
	}
	if (!read_done_1) {
		ret -= 1;
		DPU_FB_ERR("%s, DSI1 CMD_PKT_STATUS[0x%x], PHY_STATUS[0x%x], INT_ST0[0x%x], INT_ST1[0x%x]\n",
			__func__, inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base_1 + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base_1 + MIPIDSI_INT_ST0_OFFSET), inp32(dsi_base_1 + MIPIDSI_INT_ST1_OFFSET));
	}

	return ret;
}

static void mipi_single_dsi_read_send(struct dsi_cmd_desc *pCmd, char __iomem *dsi_base)
{
	/* send read cmd to fifo */
#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501) || \
	defined(CONFIG_HISI_FB_V330) || defined(CONFIG_HISI_FB_V510) || \
	defined(CONFIG_HISI_FB_V600)
	/* used for low power cmds trans under video mode */
	set_reg(dsi_base + MIPIDSI_GEN_HDR_OFFSET, ((((uint32_t)(uint8_t)pCmd->payload[0]) << 8)
		| (pCmd->dtype & GEN_VID_LP_CMD)), 25, 0);
#else
	set_reg(dsi_base + MIPIDSI_GEN_HDR_OFFSET, ((((uint32_t)(uint8_t)pCmd->payload[0]) << 8)
		| pCmd->dtype), 24, 0);
#endif
}

/* just for Cyclomatic Complexity, no need to check input param */
static int mipi_dual_dsi_read_add_send(struct dsi_cmd_desc *pCmd, char __iomem *dsi_base_0,
	char __iomem *dsi_base_1, int *is_timeout_0, int *is_timeout_1)
{
	unsigned long dw_jiffies;
	uint32_t pkg_status_0 = 0;
	uint32_t pkg_status_1 = 0;
	uint32_t phy_status_0 = 0;
	uint32_t phy_status_1 = 0;
	int ret = 2;

	/* read status register */
	dw_jiffies = jiffies + HZ;
	do {
		if (*is_timeout_0) {
			pkg_status_0 = inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
			phy_status_0 = inp32(dsi_base_0 + MIPIDSI_PHY_STATUS_OFFSET);
			if ((pkg_status_0 & 0x1) == 0x1 && !(phy_status_0 & 0x2)) {
				*is_timeout_0 = 0;

				mipi_single_dsi_read_send(pCmd, dsi_base_0);
			}
		}

		if (*is_timeout_1) {
			pkg_status_1 = inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
			phy_status_1 = inp32(dsi_base_1 + MIPIDSI_PHY_STATUS_OFFSET);
			if ((pkg_status_1 & 0x1) == 0x1 && !(phy_status_1 & 0x2)) {
				*is_timeout_1 = 0;

				mipi_single_dsi_read_send(pCmd, dsi_base_1);
			}
		}

		if ((!(*is_timeout_0)) && (!(*is_timeout_1)))   /* if both done, break */
			break;
	} while (time_after(dw_jiffies, jiffies));

	if (*is_timeout_0) {
		DPU_FB_ERR("mipi_dual_dsi_read_add dsi0 tx timeout :0x%x\n"
			"IPIDSI_CMD_PKT_STATUS = 0x%x\n"
			"IPIDSI_PHY_STATUS = 0x%x\n"
			"IPIDSI_INT_ST1_OFFSET = 0x%x\n",
			pCmd->payload[0], inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base_0 + MIPIDSI_PHY_STATUS_OFFSET), inp32(dsi_base_0 + MIPIDSI_INT_ST1_OFFSET));
		ret -= 1;
	}
	if (*is_timeout_1) {
		DPU_FB_ERR("mipi_dual_dsi_read_add dsi1 tx timeout :0x%x\n"
			"MIPIDSI_CMD_PKT_STATUS = 0x%x\n"
			"MIPIDSI_PHY_STATUS = 0x%x\n"
			"MIPIDSI_INT_ST1_OFFSET = 0x%x\n",
			pCmd->payload[0], inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base_1 + MIPIDSI_PHY_STATUS_OFFSET), inp32(dsi_base_1 + MIPIDSI_INT_ST1_OFFSET));

		ret -= 1;
	}

	return ret;
}

/* just for Cyclomatic Complexity, no need to check input param */
static inline int mipi_dsi_read_add_receive(uint32_t *ValueOut, struct dsi_cmd_desc *pCmd,
	const char __iomem *dsi_base)
{
	unsigned long dw_jiffies;
	uint32_t pkg_status;
	bool read_done = false;

	/* wait dsi read data */
	dw_jiffies = jiffies + HZ;
	do {
		pkg_status = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		if (!(pkg_status & 0x10)) {
			read_done = true;
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

	if (!read_done) {
		DPU_FB_ERR("mipi_dsi_read timeout :0x%x\n"
			"MIPIDSI_CMD_PKT_STATUS = 0x%x\n"
			"MIPIDSI_PHY_STATUS = 0x%x\n",
			pCmd->payload[0],
			inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET));
		return 0;
	}
	/* get read data */
	*ValueOut = inp32(dsi_base + MIPIDSI_GEN_PLD_DATA_OFFSET);

	return 1;
}

static int mipi_dual_dsi_pkg_status_check(const char __iomem *dsi_base_0,
	const char __iomem *dsi_base_1, struct dsi_cmd_desc *pCmd,
	bool read_done_0, bool read_done_1)
{
	int ret = 2;

	if (!read_done_0) {
		DPU_FB_ERR("mipi_dual_dsi_read_add dsi0 rx timeout :0x%x\n"
			"MIPIDSI_CMD_PKT_STATUS = 0x%x\n"
			"MIPIDSI_PHY_STATUS = 0x%x\n",
			pCmd->payload[0],
			inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base_0 + MIPIDSI_PHY_STATUS_OFFSET));
		ret -= 1;
	}
	if (!read_done_1) {
		DPU_FB_ERR("mipi_dual_dsi_read_add dsi1 rx timeout :0x%x\n"
			"MIPIDSI_CMD_PKT_STATUS = 0x%x\n"
			"MIPIDSI_PHY_STATUS = 0x%x\n",
			pCmd->payload[0],
			inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base_1 + MIPIDSI_PHY_STATUS_OFFSET));
		ret -= 1;
	}

	return ret;
}

static int mipi_dual_dsi_read_add_receive(struct dsi_cmd_desc *p_cmd,
	struct mipi_dual_dsi_param *dual_dsi0, struct mipi_dual_dsi_param *dual_dsi1)
{
	unsigned long dw_jiffies = 0;
	uint32_t pkg_status_0 = 0;
	uint32_t pkg_status_1 = 0;
	bool read_done_0 = false;
	bool read_done_1 = false;
	int ret = 0;

	if ((dual_dsi0->is_timeout == 0) && (dual_dsi1->is_timeout == 0)) {
		/* wait dsi read data */
		dw_jiffies = jiffies + HZ;
		do {
			if (!read_done_0) {
				pkg_status_0 = inp32(dual_dsi0->dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
				if (!(pkg_status_0 & 0x10)) {
					read_done_0 = true;
					*dual_dsi0->value_out = inp32(dual_dsi0->dsi_base +
						MIPIDSI_GEN_PLD_DATA_OFFSET);
				}
			}
			if (!read_done_1) {
				pkg_status_1 = inp32(dual_dsi1->dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
				if (!(pkg_status_1 & 0x10)) {
					read_done_1 = true;
					*dual_dsi1->value_out = inp32(dual_dsi1->dsi_base +
						MIPIDSI_GEN_PLD_DATA_OFFSET);
				}
			}

			if (read_done_0 && read_done_1)
				break;
		} while (time_after(dw_jiffies, jiffies));

		ret = mipi_dual_dsi_pkg_status_check(dual_dsi0->dsi_base, dual_dsi1->dsi_base,
			p_cmd, read_done_0, read_done_1);
	} else if (dual_dsi0->is_timeout == 0) {
		ret = mipi_dsi_read_add_receive(dual_dsi0->value_out, p_cmd, dual_dsi0->dsi_base);
	} else if (dual_dsi1->is_timeout == 0) {
		ret = mipi_dsi_read_add_receive(dual_dsi1->value_out, p_cmd, dual_dsi1->dsi_base);
	} else {
		ret = 0;
	}

	return ret;
}

/* only support same cmd for two dsi */
static int mipi_dual_dsi_read_add(uint32_t *value_out_0, uint32_t *value_out_1,
	struct dsi_cmd_desc *cmd, char __iomem *dsi_base_0, char __iomem *dsi_base_1)
{
	int ret = 0;
	struct mipi_dual_dsi_param dual_dsi0 = { value_out_0, dsi_base_0, NULL, NULL, 1, 0 };
	struct mipi_dual_dsi_param dual_dsi1 = { value_out_1, dsi_base_1, NULL, NULL, 1, 0 };

	dpu_check_and_return(!cmd, -EINVAL, ERR, "Cmd is NULL!\n");

	dpu_check_and_return((!dsi_base_0 || !dsi_base_1), -EINVAL, ERR, "dsi_base is NULL!\n");

	dpu_check_and_return((!value_out_0 || !value_out_1), -EINVAL, ERR, "out buffer is NULL!\n");

	if (DSI_HDR_DTYPE(cmd->dtype) == DTYPE_DCS_READ) {
		mipi_dsi_cmd_send_lock();

		mipi_dsi_sread_request(cmd, dual_dsi0.dsi_base);
		mipi_dsi_sread_request(cmd, dual_dsi1.dsi_base);
		ret = mipi_dual_dsi_read(dual_dsi0.value_out, dual_dsi1.value_out,
			dual_dsi0.dsi_base, dual_dsi1.dsi_base);

		if (ret <= 0) {
			DPU_FB_ERR("Read register 0x%X error\n", cmd->payload[0]);
			ret = -1;
		}

		mipi_dsi_cmd_send_unlock();
	} else if (cmd->dtype == DTYPE_GEN_READ1) {
		mipi_dsi_cmd_send_lock();

		ret = mipi_dual_dsi_read_add_send(cmd, dual_dsi0.dsi_base, dual_dsi1.dsi_base,
			&dual_dsi0.is_timeout, &dual_dsi1.is_timeout);
		if (ret > 0)
			ret = mipi_dual_dsi_read_add_receive(cmd, &dual_dsi0, &dual_dsi1);

		mipi_dsi_cmd_send_unlock();
	} else {
		ret = -1;
		DPU_FB_ERR("dtype=%x NOT supported!\n", cmd->dtype);
	}

	return ret;
}

static void mipi_dsi_add_tx_cmd(struct mipi_dual_dsi_param *dual_dsi, uint32_t *nextwait, int *cmd_cnt)
{
	if (*nextwait == 0) {
		mipi_dsi_cmd_add(dual_dsi->cmd, dual_dsi->dsi_base);

		*nextwait = calc_next_wait_time(dual_dsi->cmd->wait, dual_dsi->cmd->waittype);
		dual_dsi->cmd++;
		(*cmd_cnt)++;
	}
}

static bool mipi_dsi_add_tx_remain_cmd(struct mipi_dual_dsi_param *dual_dsi, uint32_t nextwait,
	int cmd_cnt)
{
	int cnt = cmd_cnt;

	if (cnt < dual_dsi->cmdset_cnt) {
		/* delay the left time */
		delay_by_msleep(nextwait);
		for (; cnt < dual_dsi->cmdset_cnt; cnt++) {
			mipi_dsi_cmd_add(dual_dsi->cmd, dual_dsi->dsi_base);

			delay_for_next_cmd_by_sleep(dual_dsi->cmd->wait, dual_dsi->cmd->waittype);
			dual_dsi->cmd++;
		}
		return true;
	}

	return false;
}

/* just for Cyclomatic Complexity, no need to check input param */
static int mipi_dual_dsi_cmds_tx_normal(struct mipi_dual_dsi_param *dual_dsi0,
	struct mipi_dual_dsi_param *dual_dsi1)
{
	int i = 0;
	int j = 0;
	uint32_t nextwait = 0;
	uint32_t nextwait_0 = 0;
	uint32_t nextwait_1 = 0;

	while ((i < dual_dsi0->cmdset_cnt) && (j < dual_dsi1->cmdset_cnt)) {
		mipi_dsi_add_tx_cmd(dual_dsi0, &nextwait_0, &i);
		mipi_dsi_add_tx_cmd(dual_dsi1, &nextwait_1, &j);

		nextwait = (nextwait_0 > nextwait_1) ? nextwait_1 : nextwait_0;
		delay_by_msleep(nextwait);
		nextwait_0 -= nextwait;
		nextwait_1 -= nextwait;
	}

	/* send the left Cmds */
	if (mipi_dsi_add_tx_remain_cmd(dual_dsi0, nextwait_0, i) == false)
		mipi_dsi_add_tx_remain_cmd(dual_dsi1, nextwait_0, j);

	return dual_dsi0->cmdset_cnt + dual_dsi1->cmdset_cnt;
}

static void mipi_dsi_add_rx_cmd(struct mipi_dual_dsi_param *dual_dsi, uint32_t *nextwait,
	int *cmd_cnt, int *err_num)
{
	if (*nextwait == 0) {
		if (mipi_dsi_read_add(&(dual_dsi->value_out[*cmd_cnt]), dual_dsi->cmd, dual_dsi->dsi_base))
			(*err_num)++;

		*nextwait = calc_next_wait_time(dual_dsi->cmd->wait, dual_dsi->cmd->waittype);
		dual_dsi->cmd++;
		(*cmd_cnt)++;
	}
}

static bool mipi_dsi_add_rx_remain_cmd(struct mipi_dual_dsi_param *dual_dsi,
	uint32_t nextwait, int cmd_cnt, int *err_num)
{
	int cnt = cmd_cnt;

	if (cnt < dual_dsi->cmdset_cnt) {
		/* delay the left time */
		delay_by_msleep(nextwait);

		for (; cnt < dual_dsi->cmdset_cnt; cnt++) {
			if (mipi_dsi_read_add(&(dual_dsi->value_out[cnt]), dual_dsi->cmd, dual_dsi->dsi_base))
				(*err_num)++;

			delay_for_next_cmd_by_sleep(dual_dsi->cmd->wait, dual_dsi->cmd->waittype);
			dual_dsi->cmd++;
		}
		return true;
	}

	return false;
}

/* just for Cyclomatic Complexity, no need to check input param */
static int mipi_dual_dsi_cmds_rx_normal(struct mipi_dual_dsi_param *dual_dsi0,
	struct mipi_dual_dsi_param *dual_dsi1)
{
	int i = 0;
	int j = 0;
	uint32_t nextwait = 0;
	uint32_t nextwait_0 = 0;
	uint32_t nextwait_1 = 0;
	int err_num = 0;

	while ((i < dual_dsi0->cmdset_cnt) && (j < dual_dsi1->cmdset_cnt)) {
		mipi_dsi_add_rx_cmd(dual_dsi0, &nextwait_0, &i, &err_num);
		mipi_dsi_add_rx_cmd(dual_dsi1, &nextwait_1, &j, &err_num);

		nextwait = (nextwait_0 > nextwait_1) ? nextwait_1 : nextwait_0;
		delay_by_msleep(nextwait);
		nextwait_0 -= nextwait;
		nextwait_1 -= nextwait;
	}

	/* send the left Cmds */
	if (mipi_dsi_add_rx_remain_cmd(dual_dsi0, nextwait_0, i, &err_num) == false)
		mipi_dsi_add_rx_remain_cmd(dual_dsi1, nextwait_0, j, &err_num);

	return err_num;
}

static int mipi_dual_dsi_fifo_is_full(const char __iomem *dsi_base_0, const char __iomem *dsi_base_1)
{
	uint32_t pkg_status_0;
	uint32_t pkg_status_1;
	uint32_t phy_status_0;
	uint32_t phy_status_1;
	int is_timeout = 100;  /* 10ms */

	dpu_check_and_return((!dsi_base_0 || !dsi_base_1), -EINVAL, ERR, "dsi_base is NULL!\n");

	/* read status register */
	do {
		pkg_status_0 = inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		phy_status_0 = inp32(dsi_base_0 + MIPIDSI_PHY_STATUS_OFFSET);
		pkg_status_1 = inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		phy_status_1 = inp32(dsi_base_1 + MIPIDSI_PHY_STATUS_OFFSET);
		if ((pkg_status_0 & 0x2) != 0x2 && !(phy_status_0 & 0x2) &&
			(pkg_status_1 & 0x2) != 0x2 && !(phy_status_1 & 0x2))
			break;
		udelay(100); /* 100us */
	} while (is_timeout-- > 0);

	if (is_timeout < 0) {
		DPU_FB_ERR("mipi check full fail: dsi0-\n"
			"MIPIDSI_CMD_PKT_STATUS = 0x%x\n"
			"MIPIDSI_PHY_STATUS = 0x%x\n"
			"MIPIDSI_INT_ST1_OFFSET = 0x%x\n",
			inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base_0 + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base_0 + MIPIDSI_INT_ST1_OFFSET));
		DPU_FB_ERR("mipi check full fail: dsi1-\n"
			"MIPIDSI_CMD_PKT_STATUS = 0x%x\n"
			"MIPIDSI_PHY_STATUS = 0x%x\n"
			"MIPIDSI_INT_ST1_OFFSET = 0x%x\n",
			inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base_1 + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base_1 + MIPIDSI_INT_ST1_OFFSET));
		return -1;
	}

	return 0;
}

/* just for Cyclomatic Complexity, no need to check input param */
static inline int mipi_dual_dsi_tx_normal_same_delay(struct dsi_cmd_desc *Cmd0,
	struct dsi_cmd_desc *Cmd1, int Cmdset_cnt, char __iomem *dsi_base_0, char __iomem *dsi_base_1)
{
	int i;
	int send_cnt = 0;

	for (i = 0; i < Cmdset_cnt; i++) {
		if (!mipi_dual_dsi_fifo_is_full(dsi_base_0, dsi_base_1)) {
			mipi_dsi_cmd_add(Cmd0, dsi_base_0);
			mipi_dsi_cmd_add(Cmd1, dsi_base_1);
		} else {
			DPU_FB_ERR("dsi fifo full, send [%d] cmds, left [%d] cmds!!",
				send_cnt, Cmdset_cnt * 2 - send_cnt);
			break;
		}

		delay_for_next_cmd_by_sleep(Cmd0->wait, Cmd0->waittype);
		send_cnt += 2;

		Cmd0++;
		Cmd1++;
	}

	return send_cnt;
}

/* just for Cyclomatic Complexity, no need to check input param */
static inline int mipi_dual_dsi_tx_auto_mode(struct dsi_cmd_desc *Cmd0, struct dsi_cmd_desc *Cmd1,
	int Cmdset_cnt, char __iomem *dsi_base_0, char __iomem *dsi_base_1)
{
	int send_cnt;
	struct timespec64 ts;
	s64 timestamp;
	s64 delta_time;
	s64 oneframe_time;

	DPU_FB_DEBUG("+\n");

	if (Cmd0 != Cmd1) {
		DPU_FB_DEBUG("different cmd for two dsi, using normal mode\n");
		goto NormalMode;
	}

	if (!g_delayed_cmd_queue_inited) {
		DPU_FB_ERR("delayed cmd queue is not inited yet!\n");
		goto NormalMode;
	}

	ktime_get_ts64(&ts);
	timestamp = ts.tv_sec;
	timestamp *= NSEC_PER_SEC;
	timestamp += ts.tv_nsec;
	delta_time = timestamp - g_delayed_cmd_queue.timestamp_frame_start;
	oneframe_time = g_delayed_cmd_queue.oneframe_time;

	/* the CMD_AUTO_MODE_THRESHOLD is not accurate value, can be ajusted later */
	if ((delta_time < oneframe_time * CMD_AUTO_MODE_THRESHOLD) || (delta_time > oneframe_time)) {
		DPU_FB_DEBUG("enough time[%lld] to send in this frame, using normal mode\n", delta_time);
		goto NormalMode;
	}

	DPU_FB_DEBUG("NOT enough time[%lld] to send in this frame, using low priority delayed mode\n", delta_time);
	send_cnt = mipi_dsi_delayed_cmd_queue_write(Cmd0, Cmdset_cnt, true) * 2;

	DPU_FB_DEBUG("-\n");
	return send_cnt;

NormalMode:
	send_cnt = mipi_dual_dsi_tx_normal_same_delay(Cmd0, Cmd1, Cmdset_cnt, dsi_base_0, dsi_base_1);
	DPU_FB_DEBUG("--\n");

	return send_cnt;
}

int  mipi_dual_dsi_cmds_tx(struct dsi_cmd_desc *pCmdset_0, int Cmdset_cnt_0, char __iomem *dsi_base_0,
	struct dsi_cmd_desc *pCmdset_1, int Cmdset_cnt_1, char __iomem *dsi_base_1, uint8_t tx_mode)
{
	struct mipi_dual_dsi_param dual_dsi0 = { NULL, dsi_base_0, pCmdset_0, pCmdset_0, 0, Cmdset_cnt_0 };
	struct mipi_dual_dsi_param dual_dsi1 = { NULL, dsi_base_1, pCmdset_1, pCmdset_1, 0, Cmdset_cnt_1 };
	struct dsi_cmd_desc *Cmd0 = dual_dsi0.p_cmdset;
	struct dsi_cmd_desc *Cmd1 = dual_dsi1.p_cmdset;
	int send_cnt = 0;
	bool sameCmdDelay = false;

	dpu_check_and_return((!dual_dsi0.p_cmdset || (dual_dsi0.cmdset_cnt == 0)), 0, ERR, "Cmdset is NULL!\n");

	dpu_check_and_return(!dual_dsi0.dsi_base, 0, ERR, "dsi_base is NULL!\n");

	/* if only one dsi_base exist, fallback to mipi_dsi_cmds_tx() interface */
	if (!dual_dsi1.dsi_base)
		return mipi_dsi_cmds_tx_with_check_fifo(dual_dsi0.p_cmdset,
			dual_dsi0.cmdset_cnt, dual_dsi0.dsi_base);

	/* only one valid Cmdset, use same Cmd_set for two dsi */
	if (!dual_dsi1.p_cmdset || (dual_dsi1.cmdset_cnt == 0)) {
		Cmd1 = dual_dsi0.p_cmdset;
		sameCmdDelay = true;
	} else if (dual_dsi0.cmdset_cnt == dual_dsi1.cmdset_cnt) { /* support different cmd value by same step */
		sameCmdDelay = true;
	}

	if (sameCmdDelay) {
		if (tx_mode == EN_DSI_TX_NORMAL_MODE)
			/* only support same cmdset length and wait, just cmd value different */
			send_cnt = mipi_dual_dsi_tx_normal_same_delay(Cmd0, Cmd1, dual_dsi0.cmdset_cnt,
				dual_dsi0.dsi_base, dual_dsi1.dsi_base);
		else if (tx_mode == EN_DSI_TX_LOW_PRIORITY_DELAY_MODE)
			/* only support same cmd set yet */
			send_cnt = mipi_dsi_delayed_cmd_queue_write(Cmd0, dual_dsi0.cmdset_cnt, true) * 2;
		else if (tx_mode == EN_DSI_TX_HIGH_PRIORITY_DELAY_MODE)
			/* only support same cmd set yet */
			send_cnt = mipi_dsi_delayed_cmd_queue_write(Cmd0, dual_dsi0.cmdset_cnt, false) * 2;
		else if (tx_mode == EN_DSI_TX_AUTO_MODE)
			/* only support same cmdset length and wait, just cmd value different */
			send_cnt = mipi_dual_dsi_tx_auto_mode(Cmd0, Cmd1, dual_dsi0.cmdset_cnt,
				dual_dsi0.dsi_base, dual_dsi1.dsi_base);
	} else { /* two different Cmdsets case */
		/* just use normal mode, expect no need use */
		send_cnt = mipi_dual_dsi_cmds_tx_normal(&dual_dsi0, &dual_dsi1);
	}

	return send_cnt;
}

int mipi_dual_dsi_cmds_rx(struct mipi_dual_dsi_param *dual_dsi0, struct mipi_dual_dsi_param *dual_dsi1)
{
	struct dsi_cmd_desc *cmd = NULL;
	int i = 0;
	int err_num = 0;
	int cnt = 0;

	dpu_check_and_return((!dual_dsi0->p_cmdset || (dual_dsi0->cmdset_cnt == 0)), -EINVAL, ERR,
		"Cmdset is NULL!\n");

	dpu_check_and_return(!dual_dsi0->dsi_base, -EINVAL, ERR, "dsi_base is NULL!\n");

	dpu_check_and_return(!dual_dsi0->value_out, -EINVAL, ERR, "out buffer 0 is NULL!\n");

	/* if only one dsi_base exist, fallback to mipi_dsi_cmds_rx() interface */
	if (!dual_dsi1->dsi_base)
		return mipi_dsi_cmds_rx_with_check_fifo(dual_dsi0->value_out, dual_dsi0->p_cmdset,
			dual_dsi0->cmdset_cnt, dual_dsi0->dsi_base);

	dpu_check_and_return(!dual_dsi1->value_out, 0, ERR, "out buffer 1 is NULL!\n");

	cmd = dual_dsi0->p_cmdset;

	/* only one valid Cmdset, use same Cmd_set for two dsi */
	if (!dual_dsi1->p_cmdset || (dual_dsi1->cmdset_cnt == 0)) {
		for (i = 0; i < dual_dsi0->cmdset_cnt; i++) {
			if (!mipi_dual_dsi_fifo_is_full(dual_dsi0->dsi_base, dual_dsi1->dsi_base)) {
				cnt = mipi_dual_dsi_read_add(&(dual_dsi0->value_out[i]), &(dual_dsi1->value_out[i]),
					cmd, dual_dsi0->dsi_base, dual_dsi1->dsi_base);
				if (cnt <= 0)
					err_num += 2;  /* dsi0 and dsi1 read failed */
				else if (cnt == 1)
					err_num += 1;  /* dsi0 or dsi1 read failed */
			} else {
				err_num += (dual_dsi0->cmdset_cnt - i) * 2;  /* 2 is for dsi0,dsi1 */
				DPU_FB_ERR("dsi fifo full, read [%d] cmds, left [%d] cmds!!\n", i,
					dual_dsi0->cmdset_cnt - i);
				break;
			}

			delay_for_next_cmd_by_sleep(cmd->wait, cmd->waittype);
			cmd++;
		}
	} else {  /* two different Cmdsets case, expect no need use */
		err_num = mipi_dual_dsi_cmds_rx_normal(dual_dsi0, dual_dsi1);
	}

	return err_num;
}

/* only support same cmd for two dsi */
int mipi_dual_dsi_lread_reg(struct mipi_dual_dsi_param *dual_dsi0,
	struct dsi_cmd_desc *p_cmd, uint32_t dlen, struct mipi_dual_dsi_param *dual_dsi1)
{
	int ret = 0;
	uint32_t i = 0;
	struct dsi_cmd_desc packet_size_cmd_set;

	dpu_check_and_return((!p_cmd || (dlen == 0)), -1, ERR, "Cmd is NULL!\n");

	dpu_check_and_return((!dual_dsi0->dsi_base || !dual_dsi1->dsi_base), -1, ERR, "dsi_base is NULL!\n");

	dpu_check_and_return((!dual_dsi0->value_out || !dual_dsi1->value_out), -1, ERR, "out buffer is NULL!\n");

	if (mipi_dsi_cmd_is_read(p_cmd)) {
		if (!mipi_dual_dsi_fifo_is_full(dual_dsi0->dsi_base, dual_dsi1->dsi_base)) {
			packet_size_cmd_set.dtype = DTYPE_MAX_PKTSIZE;
			packet_size_cmd_set.vc = 0;
			packet_size_cmd_set.dlen = dlen;

			mipi_dsi_cmd_send_lock();

			mipi_dsi_max_return_packet_size(&packet_size_cmd_set, dual_dsi0->dsi_base);
			mipi_dsi_max_return_packet_size(&packet_size_cmd_set, dual_dsi1->dsi_base);
			mipi_dsi_sread_request(p_cmd, dual_dsi0->dsi_base);
			mipi_dsi_sread_request(p_cmd, dual_dsi1->dsi_base);
			for (i = 0; i < (dlen + 3) / 4; i++) { /* 4byte Align */
				if (mipi_dual_dsi_read(dual_dsi0->value_out, dual_dsi1->value_out,
					dual_dsi0->dsi_base, dual_dsi1->dsi_base) < 2) {
					ret = -1;
					DPU_FB_ERR("Read register 0x%X timeout\n", p_cmd->payload[0]);
					break;
				}
				dual_dsi0->value_out++;
				dual_dsi1->value_out++;
			}

			mipi_dsi_cmd_send_unlock();
		} else {
			ret = -1;
			DPU_FB_ERR("dsi fifo full, read fail!!\n");
		}
	} else {
		ret = -1;
		DPU_FB_ERR("dtype=%x NOT supported!\n", p_cmd->dtype);
	}

	return ret;
}

static void mipi_dsi_get_cmd_queue_resource(struct dsi_cmd_desc **cmd_queue,
	spinlock_t **spinlock, uint32_t *queue_len, bool is_low_priority)
{
	if (is_low_priority) {
		*cmd_queue = g_delayed_cmd_queue.cmd_queue_low_priority;
		*spinlock = &g_delayed_cmd_queue.cmd_queue_low_priority_lock;
		*queue_len = MAX_CMD_QUEUE_LOW_PRIORITY_SIZE;
	} else {
		*cmd_queue = g_delayed_cmd_queue.cmd_queue_high_priority;
		*spinlock = &g_delayed_cmd_queue.cmd_queue_high_priority_lock;
		*queue_len = MAX_CMD_QUEUE_HIGH_PRIORITY_SIZE;
	}
}

static void mipi_dsi_get_cmd_queue_prio_status(uint32_t **w_ptr, uint32_t **r_ptr,
	bool **is_queue_full, bool **is_queue_working, bool is_low_priority)
{
	if (is_low_priority) {
		*w_ptr = &g_delayed_cmd_queue.cmd_queue_low_priority_wr;
		*r_ptr = &g_delayed_cmd_queue.cmd_queue_low_priority_rd;
		*is_queue_full = &g_delayed_cmd_queue.is_cmd_queue_low_priority_full;
		*is_queue_working = &g_delayed_cmd_queue.is_cmd_queue_low_priority_working;
	} else {
		*w_ptr = &g_delayed_cmd_queue.cmd_queue_high_priority_wr;
		*r_ptr = &g_delayed_cmd_queue.cmd_queue_high_priority_rd;
		*is_queue_full = &g_delayed_cmd_queue.is_cmd_queue_high_priority_full;
		*is_queue_working = &g_delayed_cmd_queue.is_cmd_queue_high_priority_working;
	}
}

static int mipi_dsi_cmd_queue_init(struct dsi_cmd_desc *cmd_queue_elem, struct dsi_cmd_desc *cmd)
{
	u32 j;

	cmd_queue_elem->dtype = cmd->dtype;
	cmd_queue_elem->vc = cmd->vc;
	cmd_queue_elem->wait = cmd->wait;
	cmd_queue_elem->waittype = cmd->waittype;
	cmd_queue_elem->dlen = cmd->dlen;
	if (cmd->dlen > 0) {
		cmd_queue_elem->payload = (char *)kmalloc(cmd->dlen * sizeof(char), GFP_ATOMIC);
		if (!cmd_queue_elem->payload) {
			DPU_FB_ERR("cmd[%d/%d] payload malloc [%d] fail!\n", cmd->dtype, cmd->vc, cmd->dlen);
			return -1;  /* skip this cmd */
		}
		memset(cmd_queue_elem->payload, 0, cmd->dlen * sizeof(char));
		for (j = 0; j < cmd->dlen; j++)
			cmd_queue_elem->payload[j] = cmd->payload[j];
	}

	return 0;
}

static int mipi_dsi_delayed_cmd_queue_write(struct dsi_cmd_desc *cmd_set,
	int cmd_set_cnt, bool is_low_priority)
{
	spinlock_t *spinlock = NULL;
	uint32_t *w_ptr = NULL;
	uint32_t *r_ptr = NULL;
	struct dsi_cmd_desc *cmd_queue = NULL;
	bool *is_queue_full = NULL;
	bool *is_queue_working = NULL;
	uint32_t queue_len;
	int i;
	struct dsi_cmd_desc *cmd = cmd_set;

	dpu_check_and_return(!cmd_set, 0, ERR, "cmd is NULL!\n");

	if (!g_delayed_cmd_queue_inited) {
		DPU_FB_ERR("delayed cmd queue is not inited yet!\n");
		return 0;
	}

	mipi_dsi_get_cmd_queue_resource(&cmd_queue, &spinlock, &queue_len, is_low_priority);

	spin_lock(spinlock);

	mipi_dsi_get_cmd_queue_prio_status(&w_ptr, &r_ptr, &is_queue_full, &is_queue_working, is_low_priority);

	if (*is_queue_full) {
		DPU_FB_ERR("Fail, delayed cmd queue [%d] is full!\n", is_low_priority);
		spin_unlock(spinlock);
		return 0;
	}

	for (i = 0; i < cmd_set_cnt; i++) {
		if (mipi_dsi_cmd_queue_init(&cmd_queue[(*w_ptr)], cmd) < 0)
			continue;

		(*w_ptr) = (*w_ptr) + 1;
		if ((*w_ptr) >= queue_len)
			(*w_ptr) = 0;

		(*is_queue_working) = true;

		if ((*w_ptr) == (*r_ptr)) {
			(*is_queue_full) = true;
			DPU_FB_ERR("Fail, delayed cmd queue [%d] become full, %d cmds are not added in queue!\n",
				is_low_priority, (cmd_set_cnt - i));
			break;
		}

		cmd++;
	}

	spin_unlock(spinlock);

	DPU_FB_DEBUG("%d cmds are added to delayed cmd queue [%d]\n", i, is_low_priority);

	return i;
}

static int mipi_dsi_delayed_cmd_queue_read(struct dsi_cmd_desc *cmd, bool is_low_priority)
{
	spinlock_t *spinlock = NULL;
	uint32_t *w_ptr = NULL;
	uint32_t *r_ptr = NULL;
	struct dsi_cmd_desc *cmd_queue = NULL;
	bool *is_queue_full = NULL;
	bool *is_queue_working = NULL;
	uint32_t queue_len;

	dpu_check_and_return(!cmd, -1, ERR, "cmd is NULL!\n");

	if (!g_delayed_cmd_queue_inited) {
		DPU_FB_ERR("delayed cmd queue is not inited yet!\n");
		return -1;
	}

	mipi_dsi_get_cmd_queue_resource(&cmd_queue, &spinlock, &queue_len, is_low_priority);

	spin_lock(spinlock);

	mipi_dsi_get_cmd_queue_prio_status(&w_ptr, &r_ptr, &is_queue_full, &is_queue_working, is_low_priority);

	if ((!(*is_queue_full)) && ((*w_ptr) == (*r_ptr))) {
		spin_unlock(spinlock);
		return -1;
	}

	cmd->dtype = cmd_queue[(*r_ptr)].dtype;
	cmd->vc = cmd_queue[(*r_ptr)].vc;
	cmd->wait = cmd_queue[(*r_ptr)].wait;
	cmd->waittype = cmd_queue[(*r_ptr)].waittype;
	cmd->dlen = cmd_queue[(*r_ptr)].dlen;
	/* Note: just copy the pointer. The malloc mem will be free by the caller after useless */
	cmd->payload = cmd_queue[(*r_ptr)].payload;
	memset(&(cmd_queue[(*r_ptr)]), 0, sizeof(struct dsi_cmd_desc));

	(*r_ptr) = (*r_ptr) + 1;
	if ((*r_ptr) >= queue_len)
		(*r_ptr) = 0;

	(*is_queue_full) = false;

	if ((*r_ptr) == (*w_ptr)) {
		(*is_queue_working) = false;
		DPU_FB_DEBUG("The last cmd in delayed queue [%d] is sent!\n", is_low_priority);
	}

	spin_unlock(spinlock);

	DPU_FB_DEBUG("Acquire one cmd from delayed cmd queue [%d] successl!\n", is_low_priority);

	return 0;
}

static uint32_t mipi_dsi_get_delayed_cmd_queue_send_count(bool is_low_priority)
{
	spinlock_t *spinlock = NULL;
	uint32_t w_ptr = 0;
	uint32_t r_ptr = 0;
	bool is_queue_full = false;
	uint32_t queue_len;
	uint32_t send_count = 0;


	if (!g_delayed_cmd_queue_inited) {
		DPU_FB_ERR("delayed cmd queue is not inited yet!\n");
		return 0;
	}

	if (is_low_priority) {
		spinlock = &g_delayed_cmd_queue.cmd_queue_low_priority_lock;
		queue_len = MAX_CMD_QUEUE_LOW_PRIORITY_SIZE;
	} else {
		spinlock = &g_delayed_cmd_queue.cmd_queue_high_priority_lock;
		queue_len = MAX_CMD_QUEUE_HIGH_PRIORITY_SIZE;
	}

	spin_lock(spinlock);

	if (is_low_priority) {
		w_ptr = g_delayed_cmd_queue.cmd_queue_low_priority_wr;
		r_ptr = g_delayed_cmd_queue.cmd_queue_low_priority_rd;
		is_queue_full = g_delayed_cmd_queue.is_cmd_queue_low_priority_full;
	} else {
		w_ptr = g_delayed_cmd_queue.cmd_queue_high_priority_wr;
		r_ptr = g_delayed_cmd_queue.cmd_queue_high_priority_rd;
		is_queue_full = g_delayed_cmd_queue.is_cmd_queue_high_priority_full;
	}

	spin_unlock(spinlock); /* no need lock any more */

	if (w_ptr > r_ptr)
		send_count = w_ptr - r_ptr;
	else if (w_ptr < r_ptr)
		send_count = w_ptr + queue_len - r_ptr;
	else if (is_queue_full)
		send_count = queue_len;
	/* else: means queue empty, send_count is 0 */

	if (send_count > 0)
		DPU_FB_DEBUG("delay cmd queue [%d]: %d cmds need to be sent\n", is_low_priority, send_count);

	return send_count;
}

void mipi_dsi_init_delayed_cmd_queue(void)
{
	DPU_FB_INFO("+\n");

	memset(&g_delayed_cmd_queue, 0, sizeof(struct dsi_delayed_cmd_queue));
	sema_init(&g_delayed_cmd_queue.work_queue_sem, 1);
	spin_lock_init(&g_delayed_cmd_queue.cmd_send_lock);
	spin_lock_init(&g_delayed_cmd_queue.cmd_queue_low_priority_lock);
	spin_lock_init(&g_delayed_cmd_queue.cmd_queue_high_priority_lock);
	g_delayed_cmd_queue_inited = true;
}

void mipi_dsi_delayed_cmd_queue_handle_func(struct work_struct *work)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dsi_cmd_desc cmd = {0};
	uint32_t send_count_high_priority;
	uint32_t send_count_low_priority;
	s64 oneframe_time;

	dpu_check_and_no_retval(!work, ERR, "work is NULL\n");

	if (!g_delayed_cmd_queue_inited) {
		DPU_FB_ERR("delayed cmd queue is not inited yet!\n");
		return;
	}

	dpufd = container_of(work, struct dpu_fb_data_type, delayed_cmd_queue_work);
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");

	DPU_FB_INFO("+\n");

	down(&g_delayed_cmd_queue.work_queue_sem);

	send_count_low_priority = mipi_dsi_get_delayed_cmd_queue_send_count(true);
	send_count_high_priority = mipi_dsi_get_delayed_cmd_queue_send_count(false);

	while (send_count_high_priority > 0) {
		if (mipi_dsi_delayed_cmd_queue_read(&cmd, false)) { /* get the next cmd in high priority queue */
			/* send the cmd with normal mode */
			mipi_dual_dsi_tx_normal_same_delay(&cmd, &cmd, 1,
				dpufd->mipi_dsi0_base, dpufd->mipi_dsi1_base);
			if (cmd.payload) {
				kfree(cmd.payload);
				cmd.payload = NULL;
			}
		}
		send_count_high_priority--;
	}

	while (send_count_low_priority > 0) {
		if (mipi_dsi_delayed_cmd_queue_read(&cmd, true)) {  /* get the next cmd in low priority queue */
			/* send the cmd with normal mode */
			mipi_dual_dsi_tx_normal_same_delay(&cmd, &cmd, 1,
				dpufd->mipi_dsi0_base, dpufd->mipi_dsi1_base);
			if (cmd.payload) {
				kfree(cmd.payload);
				cmd.payload = NULL;
			}
		}
		send_count_low_priority--;
	}

	up(&g_delayed_cmd_queue.work_queue_sem);

	/* re-fresh one frame time unit:ms */
	oneframe_time = (s64)(dpufd->panel_info.xres + dpufd->panel_info.ldi.h_back_porch +
		dpufd->panel_info.ldi.h_front_porch + dpufd->panel_info.ldi.h_pulse_width) *
		(dpufd->panel_info.yres + dpufd->panel_info.ldi.v_back_porch + dpufd->panel_info.ldi.v_front_porch +
		dpufd->panel_info.ldi.v_pulse_width) * 1000000000UL / dpufd->panel_info.pxl_clk_rate;
	if (oneframe_time != g_delayed_cmd_queue.oneframe_time) {
		g_delayed_cmd_queue.oneframe_time = oneframe_time;
		DPU_FB_INFO("update one frame time to %lld\n", oneframe_time);
	}

	DPU_FB_INFO("-\n");
}

void mipi_dsi_set_timestamp(void)
{
	struct timespec64 ts;
	s64 timestamp = 0;

	if (g_delayed_cmd_queue_inited) {
		ktime_get_ts64(&ts);
		timestamp = ts.tv_sec;
		timestamp *= NSEC_PER_SEC;
		timestamp += ts.tv_nsec;
		g_delayed_cmd_queue.timestamp_frame_start = timestamp;
	}
}

bool mipi_dsi_check_delayed_cmd_queue_working(void)
{
	bool ret = false;

	if (g_delayed_cmd_queue.is_cmd_queue_high_priority_working ||
		g_delayed_cmd_queue.is_cmd_queue_low_priority_working)
		ret = true;

	return ret;
}
