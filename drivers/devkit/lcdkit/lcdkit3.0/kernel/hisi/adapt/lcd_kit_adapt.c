/*
 * lcd_kit_adapt.c
 *
 * lcdkit adapt function for lcd driver
 *
 * Copyright (c) 2018-2021 Huawei Technologies Co., Ltd.
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

#include "hisi_fb.h"
#include "lcd_kit_common.h"
#include "lcd_kit_dbg.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_power.h"

#define READ_MAX 100
#define BUF_MAX (4 * READ_MAX)

static const unsigned int fps_60hz = 60;
static const unsigned int frame_time_us_60hz = 16667;

static void lcd_kit_dump_cmd(struct dsi_cmd_desc *cmd)
{
	int i;

	LCD_KIT_DEBUG("cmd->dtype = 0x%x\n", cmd->dtype);
	LCD_KIT_DEBUG("cmd->vc = 0x%x\n", cmd->vc);
	LCD_KIT_DEBUG("cmd->wait = 0x%x\n", cmd->wait);
	LCD_KIT_DEBUG("cmd->waittype = 0x%x\n", cmd->waittype);
	LCD_KIT_DEBUG("cmd->dlen = 0x%x\n", cmd->dlen);
	LCD_KIT_DEBUG("cmd->payload:\n");
	if (lcd_kit_msg_level >= MSG_LEVEL_DEBUG) {
		for (i = 0; i < cmd->dlen; i++)
			LCD_KIT_DEBUG("0x%x\n", cmd->payload[i]);
	}
}

static void lcd_cmds_to_dsi_cmds_ex(struct lcd_kit_dsi_cmd_desc *lcd_kit_cmds,
	struct dsi_cmd_desc *cmd, int link_state)
{
	if (lcd_kit_cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is null point!\n");
		return;
	}
	if (cmd == NULL) {
		LCD_KIT_ERR("cmd is null point!\n");
		return;
	}
	cmd->dtype = lcd_kit_cmds->dtype;
	if (link_state == LCD_KIT_DSI_LP_MODE)
		cmd->dtype |= GEN_VID_LP_CMD;
	cmd->vc = lcd_kit_cmds->vc;
	cmd->wait = lcd_kit_cmds->wait;
	cmd->waittype = lcd_kit_cmds->waittype;
	cmd->dlen = lcd_kit_cmds->dlen;
	cmd->payload = lcd_kit_cmds->payload;
	lcd_kit_dump_cmd(cmd);
}

static int lcd_kit_cmds_to_dsi_cmds(struct lcd_kit_dsi_cmd_desc *lcd_kit_cmds,
	struct dsi_cmd_desc *cmd, int link_state)
{
	if (lcd_kit_cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is null point!\n");
		return LCD_KIT_FAIL;
	}
	if (cmd == NULL) {
		LCD_KIT_ERR("cmd is null!\n");
		return LCD_KIT_FAIL;
	}
	cmd->dtype = lcd_kit_cmds->dtype;
	if (link_state == LCD_KIT_DSI_LP_MODE)
		cmd->dtype |= GEN_VID_LP_CMD;
	cmd->vc =  lcd_kit_cmds->vc;
	cmd->waittype =  lcd_kit_cmds->waittype;
	cmd->dlen =  lcd_kit_cmds->dlen;
	cmd->payload = lcd_kit_cmds->payload;
	lcd_kit_dump_cmd(cmd);
	return LCD_KIT_OK;
}

static bool lcd_kit_cmd_is_write(struct dsi_cmd_desc *cmd)
{
	bool is_write = true;

	switch (DSI_HDR_DTYPE(cmd->dtype)) {
	case DTYPE_GEN_WRITE:
	case DTYPE_GEN_WRITE1:
	case DTYPE_GEN_WRITE2:
	case DTYPE_GEN_LWRITE:
	case DTYPE_DCS_WRITE:
	case DTYPE_DCS_WRITE1:
	case DTYPE_DCS_LWRITE:
	case DTYPE_DSC_LWRITE:
		is_write = true;
		break;
	case DTYPE_GEN_READ:
	case DTYPE_GEN_READ1:
	case DTYPE_GEN_READ2:
	case DTYPE_DCS_READ:
		is_write = false;
		break;
	default:
		is_write = false;
		break;
	}
	return is_write;
}
char __iomem *lcd_kit_get_dsi_base_addr(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;

	pinfo = &hisifd->panel_info;
	if (pinfo == NULL)
		return hisifd->mipi_dsi0_base;
	if (pinfo->mipi.dsi_te_type == DSI1_TE1_TYPE)
		return hisifd->mipi_dsi1_base;

	return hisifd->mipi_dsi0_base;
}
int lcd_kit_dsi_diff_cmds_tx(void *hld, struct lcd_kit_dsi_panel_cmds *dsi0_cmds,
	struct lcd_kit_dsi_panel_cmds *dsi1_cmds)
{
	int i;
	struct hisi_fb_data_type *hisifd = NULL;
	struct dsi_cmd_desc *dsi0_cmd = NULL;
	struct dsi_cmd_desc *dsi1_cmd = NULL;

	if ((dsi0_cmds == NULL) || (dsi1_cmds == NULL)) {
		LCD_KIT_ERR("cmd cnt is 0!\n");
		return LCD_KIT_FAIL;
	}
	if (((dsi0_cmds->cmds == NULL) || (dsi0_cmds->cmd_cnt <= 0)) ||
		((dsi1_cmds->cmds == NULL) || (dsi1_cmds->cmd_cnt <= 0))) {
		LCD_KIT_ERR("cmds is null, or cmds->cmd_cnt <= 0!\n");
		return LCD_KIT_FAIL;
	}
	hisifd = (struct hisi_fb_data_type *)hld;
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null!\n");
		return LCD_KIT_FAIL;
	}
	dsi0_cmd = kzalloc(sizeof(*dsi0_cmd) * dsi0_cmds->cmd_cnt, GFP_KERNEL);
	if (!dsi0_cmd) {
		LCD_KIT_ERR("dsi0_cmd is null!\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < dsi0_cmds->cmd_cnt; i++)
		lcd_cmds_to_dsi_cmds_ex(&dsi0_cmds->cmds[i],
				(dsi0_cmd + i), dsi0_cmds->link_state);
	dsi1_cmd = kzalloc(sizeof(*dsi1_cmd) * dsi1_cmds->cmd_cnt, GFP_KERNEL);
	if (!dsi1_cmd) {
		kfree(dsi0_cmd);
		LCD_KIT_ERR("dsi1_cmd is null!\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < dsi1_cmds->cmd_cnt; i++)
		lcd_cmds_to_dsi_cmds_ex(&dsi1_cmds->cmds[i],
				(dsi1_cmd + i), dsi1_cmds->link_state);
	down(&DISP_LOCK->lcd_kit_sem);
	(void)mipi_dual_dsi_cmds_tx(dsi0_cmd, dsi0_cmds->cmd_cnt,
		hisifd->mipi_dsi0_base, dsi1_cmd, dsi1_cmds->cmd_cnt,
		hisifd->mipi_dsi1_base, EN_DSI_TX_NORMAL_MODE);
	up(&DISP_LOCK->lcd_kit_sem);
	kfree(dsi0_cmd);
	kfree(dsi1_cmd);
	return LCD_KIT_OK;
}

static int lcd_kit_dual_dsi_tx_sync(void *hld, struct dsi_cmd_desc *cmds)
{
	struct hisi_fb_data_type *hisifd = (struct hisi_fb_data_type *)hld;

	if (!is_dual_mipi_panel(hld) || (disp_info->dual_dsi_send_sync == 0))
		return LCD_KIT_FAIL;
	(void)mipi_dual_dsi_cmds_tx(cmds, 1,
		hisifd->mipi_dsi0_base, cmds, 1,
		hisifd->mipi_dsi1_base, EN_DSI_TX_NORMAL_MODE);
	return LCD_KIT_OK;
}

int lcd_kit_dsi_cmds_tx(void *hld, struct lcd_kit_dsi_panel_cmds *cmds)
{
	int i;
	int link_state;
	char __iomem *dsi_base_addr = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct dsi_cmd_desc dsi_cmd;

	if (cmds == NULL) {
		LCD_KIT_ERR("cmd cnt is 0!\n");
		return LCD_KIT_FAIL;
	}
	memset(&dsi_cmd, 0, sizeof(dsi_cmd));
	if ((cmds->cmds == NULL) || (cmds->cmd_cnt <= 0)) {
		LCD_KIT_ERR("cmds is null, or cmds->cmd_cnt <= 0!\n");
		return LCD_KIT_FAIL;
	}
	hisifd = (struct hisi_fb_data_type *)hld;
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null!\n");
		return LCD_KIT_FAIL;
	}
	link_state = cmds->link_state;
	down(&DISP_LOCK->lcd_kit_sem);
	/* switch to LP mode */
	if (cmds->link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_LP_MODE);
	dsi_base_addr = lcd_kit_get_dsi_base_addr(hisifd);
	for (i = 0; i < cmds->cmd_cnt; i++) {
		lcd_kit_cmds_to_dsi_cmds(&cmds->cmds[i], &dsi_cmd, link_state);
		if (lcd_kit_dual_dsi_tx_sync(hld, &dsi_cmd) == LCD_KIT_OK) {
			lcd_kit_delay(cmds->cmds[i].wait, cmds->cmds[i].waittype, true);
			continue;
		}
		/* separate transmission */
		if ((disp_info != NULL) && (disp_info->transmission.separate_support)) {
			if (lcd_kit_dsi_fifo_is_empty(dsi_base_addr) == LCD_KIT_OK)
				mipi_dsi_cmds_tx(&dsi_cmd, 1, dsi_base_addr);
		} else {
			if (!lcd_kit_dsi_fifo_is_full(dsi_base_addr))
				mipi_dsi_cmds_tx(&dsi_cmd, 1, dsi_base_addr);
		}
		if (is_dual_mipi_panel(hisifd)) {
			if (!lcd_kit_dsi_fifo_is_full(hisifd->mipi_dsi1_base))
				mipi_dsi_cmds_tx(&dsi_cmd, 1,
					hisifd->mipi_dsi1_base);
		}
		lcd_kit_delay(cmds->cmds[i].wait, cmds->cmds[i].waittype, true);
	}
	/* switch to HS mode */
	if (cmds->link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_HS_MODE);
	up(&DISP_LOCK->lcd_kit_sem);
	return LCD_KIT_OK;
}

int lcd_kit_dsi0_cmds_tx(void *hld, struct lcd_kit_dsi_panel_cmds *cmds)
{
	int i;
	int link_state;
	char __iomem *dsi_base_addr = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct dsi_cmd_desc dsi_cmd;

	if (cmds == NULL) {
		LCD_KIT_ERR("cmd cnt is 0!\n");
		return LCD_KIT_FAIL;
	}
	memset(&dsi_cmd, 0, sizeof(dsi_cmd));
	if ((cmds->cmds == NULL) || (cmds->cmd_cnt <= 0)) {
		LCD_KIT_ERR("cmds is null, or cmds->cmd_cnt <= 0!\n");
		return LCD_KIT_FAIL;
	}
	hisifd = (struct hisi_fb_data_type *)hld;
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null!\n");
		return LCD_KIT_FAIL;
	}
	link_state = cmds->link_state;
	down(&DISP_LOCK->lcd_kit_sem);
	/* switch to LP mode */
	if (cmds->link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_LP_MODE);
	dsi_base_addr = hisifd->mipi_dsi0_base;
	for (i = 0; i < cmds->cmd_cnt; i++) {
		lcd_kit_cmds_to_dsi_cmds(&cmds->cmds[i], &dsi_cmd, link_state);
		if (!lcd_kit_dsi_fifo_is_full(dsi_base_addr))
			mipi_dsi_cmds_tx(&dsi_cmd, 1, dsi_base_addr);
		lcd_kit_delay(cmds->cmds[i].wait, cmds->cmds[i].waittype, true);
	}
	/* switch to HS mode */
	if (cmds->link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_HS_MODE);
	up(&DISP_LOCK->lcd_kit_sem);
	return LCD_KIT_OK;
}

int lcd_kit_dsi1_cmds_tx(void *hld, struct lcd_kit_dsi_panel_cmds *cmds)
{
	int i;
	int link_state;
	char __iomem *dsi_base_addr = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct dsi_cmd_desc dsi_cmd;

	if (cmds == NULL) {
		LCD_KIT_ERR("cmd cnt is 0!\n");
		return LCD_KIT_FAIL;
	}
	memset(&dsi_cmd, 0, sizeof(dsi_cmd));
	if ((cmds->cmds == NULL) || (cmds->cmd_cnt <= 0)) {
		LCD_KIT_ERR("cmds is null, or cmds->cmd_cnt <= 0!\n");
		return LCD_KIT_FAIL;
	}
	hisifd = (struct hisi_fb_data_type *)hld;
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null!\n");
		return LCD_KIT_FAIL;
	}
	link_state = cmds->link_state;
	down(&DISP_LOCK->lcd_kit_sem);
	/* switch to LP mode */
	if (cmds->link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_LP_MODE);
	dsi_base_addr = hisifd->mipi_dsi1_base;
	for (i = 0; i < cmds->cmd_cnt; i++) {
		lcd_kit_cmds_to_dsi_cmds(&cmds->cmds[i], &dsi_cmd, link_state);
		if (!lcd_kit_dsi_fifo_is_full(dsi_base_addr))
			mipi_dsi_cmds_tx(&dsi_cmd, 1, dsi_base_addr);
		lcd_kit_delay(cmds->cmds[i].wait, cmds->cmds[i].waittype, true);
	}
	/* switch to HS mode */
	if (cmds->link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_HS_MODE);
	up(&DISP_LOCK->lcd_kit_sem);
	return LCD_KIT_OK;
}

static int lcd_get_read_value(struct dsi_cmd_desc *dsi_cmd,
	uint8_t *dest, uint32_t *src, uint32_t len)
{
	int dlen;
	int cnt = 0;
	int start_index = 0;
	int div = sizeof(uint32_t) / sizeof(uint8_t);

	if (dsi_cmd->dlen > 1)
		start_index = (int)dsi_cmd->payload[1];
	for (dlen = 0; dlen < dsi_cmd->dlen; dlen++) {
		if (dlen < (start_index - 1))
			continue;
		if (cnt >= len) {
			LCD_KIT_ERR("data len error\n");
			return LCD_KIT_FAIL;
		}
		switch (dlen % div) {
		case 0:
			dest[cnt] = (uint8_t)(src[dlen / div] & 0xFF);
			break;
		case 1:
			dest[cnt] = (uint8_t)((src[dlen / div] >> 8) & 0xFF);
			break;
		case 2:
			dest[cnt] = (uint8_t)((src[dlen / div] >> 16) & 0xFF);
			break;
		case 3:
			dest[cnt] = (uint8_t)((src[dlen / div] >> 24) & 0xFF);
			break;
		default:
			break;
		}
		cnt++;
	}
	return LCD_KIT_OK;
}

static int lcd_kit_cmd_rx(struct hisi_fb_data_type *hisifd, uint8_t *out,
	int out_len, struct dsi_cmd_desc *cmd, char __iomem *dsi_base)
{
	int ret;
	uint32_t tmp_value[READ_MAX] = {0};

	if (lcd_kit_dsi_fifo_is_full(dsi_base)) {
		LCD_KIT_ERR("mipi read error\n");
		ret = LCD_KIT_FAIL;
		return ret;
	}
	if (lcd_kit_cmd_is_write(cmd)) {
		(void)mipi_dsi_cmds_tx(cmd, 1, dsi_base);
	} else {
		ret = mipi_dsi_lread_reg(tmp_value, cmd, cmd->dlen, dsi_base);
		if (ret) {
			LCD_KIT_ERR("mipi read error\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_get_read_value(cmd, out, tmp_value, out_len);
		if (ret) {
			LCD_KIT_ERR("get read value error\n");
			return ret;
		}
	}
	return LCD_KIT_OK;
}

int lcd_kit_dsi_cmds_rx(void *hld, uint8_t *out, int out_len,
	struct lcd_kit_dsi_panel_cmds *cmds)
{
	int ret = LCD_KIT_OK;
	int i, j, link_state;
	int cnt = 0;
	char __iomem *dsi_base_addr = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct dsi_cmd_desc dsi_cmd;
	uint8_t tmp[BUF_MAX] = {0};

	hisifd = (struct hisi_fb_data_type *)hld;
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null!\n");
		return LCD_KIT_FAIL;
	}
	if (cmds == NULL || cmds->cmds == NULL || cmds->cmd_cnt <= 0) {
		LCD_KIT_ERR("cmds is null, or cmds->cmd_cnt <= 0!\n");
		return LCD_KIT_FAIL;
	}
	if (out == NULL) {
		LCD_KIT_ERR("out is null!\n");
		return LCD_KIT_FAIL;
	}
	memset(&dsi_cmd, 0, sizeof(dsi_cmd));
	link_state = cmds->link_state;
	down(&DISP_LOCK->lcd_kit_sem);
	if (link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_LP_MODE);
	dsi_base_addr = lcd_kit_get_dsi_base_addr(hisifd);
	for (i = 0; i < cmds->cmd_cnt; i++) {
		memset(tmp, 0, sizeof(tmp));
		lcd_kit_cmds_to_dsi_cmds(&cmds->cmds[i], &dsi_cmd, link_state);
		ret = lcd_kit_cmd_rx(hisifd, tmp, BUF_MAX, &dsi_cmd,
			dsi_base_addr);
		if (ret)
			LCD_KIT_ERR("mipi rx error\n");
		if (!lcd_kit_cmd_is_write(&dsi_cmd)) {
			for (j = 0; (j < dsi_cmd.dlen) && (cnt <= out_len); j++)
				out[cnt++] = tmp[j];
		}
		lcd_kit_delay(cmds->cmds[i].wait, cmds->cmds[i].waittype,
			true);
	}
	if (link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_HS_MODE);
	up(&DISP_LOCK->lcd_kit_sem);
	return ret;
}

int lcd_kit_dsi1_cmds_rx(void *hld, uint8_t *out, int out_len,
	struct lcd_kit_dsi_panel_cmds *cmds)
{
	int ret = LCD_KIT_OK;
	int i, j, link_state;
	int cnt = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	struct dsi_cmd_desc dsi_cmd;
	uint8_t tmp[BUF_MAX] = {0};

	hisifd = (struct hisi_fb_data_type *)hld;
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null!\n");
		return LCD_KIT_FAIL;
	}
	if (cmds == NULL || cmds->cmds == NULL || cmds->cmd_cnt <= 0) {
		LCD_KIT_ERR("cmds is null, or cmds->cmd_cnt <= 0!\n");
		return LCD_KIT_FAIL;
	}
	if (out == NULL) {
		LCD_KIT_ERR("out is null!\n");
		return LCD_KIT_FAIL;
	}
	memset(&dsi_cmd, 0, sizeof(dsi_cmd));
	link_state = cmds->link_state;
	down(&DISP_LOCK->lcd_kit_sem);
	if (link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_LP_MODE);
	for (i = 0; i < cmds->cmd_cnt; i++) {
		memset(tmp, 0, sizeof(tmp));
		lcd_kit_cmds_to_dsi_cmds(&cmds->cmds[i], &dsi_cmd, link_state);
		ret = lcd_kit_cmd_rx(hisifd, tmp, BUF_MAX, &dsi_cmd,
			hisifd->mipi_dsi1_base);
		if (ret)
			LCD_KIT_ERR("mipi rx error\n");
		if (!lcd_kit_cmd_is_write(&dsi_cmd)) {
			for (j = 0; (j < dsi_cmd.dlen) && (cnt <= out_len); j++)
				out[cnt++] = tmp[j];
		}
		lcd_kit_delay(cmds->cmds[i].wait, cmds->cmds[i].waittype,
			true);
	}
	if (link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_HS_MODE);
	up(&DISP_LOCK->lcd_kit_sem);
	return ret;
}

static int lcd_kit_cmd_rx_dual(struct hisi_fb_data_type *hisifd,
	uint8_t *dsi0_out, uint8_t *dsi1_out, int out_len,
	struct dsi_cmd_desc *cmd)
{
	int ret;
	struct mipi_dual_dsi_param dual_dsi0;
	struct mipi_dual_dsi_param dual_dsi1;
	uint32_t dsi0_tmp_val[READ_MAX] = { 0 };
	uint32_t dsi1_tmp_val[READ_MAX] = { 0 };

	memset(&dual_dsi0, 0, sizeof(dual_dsi0));
	memset(&dual_dsi1, 0, sizeof(dual_dsi1));
	dual_dsi0.dsi_base = hisifd->mipi_dsi0_base;
	dual_dsi0.value_out = dsi0_tmp_val;
	dual_dsi1.dsi_base = hisifd->mipi_dsi1_base;
	dual_dsi1.value_out = dsi1_tmp_val;

	if (lcd_kit_cmd_is_write(cmd)) {
		(void)mipi_dual_dsi_cmds_tx(cmd, 1,
			hisifd->mipi_dsi0_base, cmd, 1,
			hisifd->mipi_dsi1_base, EN_DSI_TX_NORMAL_MODE);
	} else {
		ret = mipi_dual_dsi_lread_reg(&dual_dsi0, cmd,
			cmd->dlen, &dual_dsi1);
		if (ret) {
			LCD_KIT_ERR("mipi read error\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_get_read_value(cmd, dsi0_out, dsi0_tmp_val, out_len);
		if (ret) {
			LCD_KIT_ERR("get read value error\n");
			return ret;
		}
		ret = lcd_get_read_value(cmd, dsi1_out, dsi1_tmp_val, out_len);
		if (ret) {
			LCD_KIT_ERR("get read value error\n");
			return ret;
		}
	}
	return LCD_KIT_OK;
}

int lcd_kit_dual_dsi_cmds_rx(void *hld, uint8_t *dsi0_out,
	uint8_t *dsi1_out, int out_len, struct lcd_kit_dsi_panel_cmds *cmds)
{
	int ret = LCD_KIT_OK;
	int i, link_state;
	int cnt = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	struct dsi_cmd_desc dsi_cmd;
	uint8_t dsi0_val[BUF_MAX] = { 0 };
	uint8_t dsi1_val[BUF_MAX] = { 0 };
	uint32_t j;

	hisifd = (struct hisi_fb_data_type *)hld;
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null!\n");
		return LCD_KIT_FAIL;
	}
	if (cmds == NULL || cmds->cmds == NULL || cmds->cmd_cnt <= 0) {
		LCD_KIT_ERR("cmds is null, or cmds->cmd_cnt <= 0!\n");
		return LCD_KIT_FAIL;
	}
	if ((out_len >= READ_MAX) || (dsi0_out == NULL) || (dsi1_out == NULL)) {
		LCD_KIT_ERR("out_len %d, out is null!\n", out_len);
		return LCD_KIT_FAIL;
	}

	memset(&dsi_cmd, 0, sizeof(dsi_cmd));
	link_state = cmds->link_state;
	down(&DISP_LOCK->lcd_kit_sem);
	if (link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_LP_MODE);
	for (i = 0; i < cmds->cmd_cnt; i++) {
		memset(dsi0_val, 0, sizeof(dsi0_val));
		memset(dsi1_val, 0, sizeof(dsi1_val));
		(void)lcd_kit_cmds_to_dsi_cmds(&cmds->cmds[i], &dsi_cmd, link_state);
		ret = lcd_kit_cmd_rx_dual(hisifd, dsi0_val, dsi1_val, BUF_MAX,
			&dsi_cmd);
		if (ret != LCD_KIT_OK)
			LCD_KIT_ERR("mipi rx error\n");
		if (!lcd_kit_cmd_is_write(&dsi_cmd)) {
			for (j = 0; (j < dsi_cmd.dlen) && (cnt <= out_len); j++) {
				dsi0_out[cnt] = dsi0_val[j];
				dsi1_out[cnt] = dsi1_val[j];
				cnt++;
			}
		}
		lcd_kit_delay(cmds->cmds[i].wait, cmds->cmds[i].waittype,
			true);
	}
	if (link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_HS_MODE);
	up(&DISP_LOCK->lcd_kit_sem);
	return ret;
}

int lcd_kit_dsi_cmds_tx_no_lock(void *hld,
	struct lcd_kit_dsi_panel_cmds *cmds)
{
	int ret = LCD_KIT_OK;
	int i = 0;
	int link_state;
	char __iomem *dsi_base_addr = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct dsi_cmd_desc dsi_cmd;

	if (cmds == NULL) {
		LCD_KIT_ERR("cmd cnt is 0!\n");
		return LCD_KIT_FAIL;
	}
	if (cmds->cmds == NULL || cmds->cmd_cnt <= 0) {
		LCD_KIT_ERR("cmds is null, or cmds->cmd_cnt <= 0!\n");
		return LCD_KIT_FAIL;
	}
	hisifd = (struct hisi_fb_data_type *)hld;
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null!\n");
		return LCD_KIT_FAIL;
	}
	memset(&dsi_cmd, 0, sizeof(struct dsi_cmd_desc));
	link_state = cmds->link_state;
	if (cmds->link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_LP_MODE);
	dsi_base_addr = lcd_kit_get_dsi_base_addr(hisifd);
	for (i = 0; i < cmds->cmd_cnt; i++) {
		lcd_kit_cmds_to_dsi_cmds(&cmds->cmds[i], &dsi_cmd, link_state);
		if (lcd_kit_dual_dsi_tx_sync(hld, &dsi_cmd) == LCD_KIT_OK) {
			lcd_kit_delay(cmds->cmds[i].wait, cmds->cmds[i].waittype, false);
			continue;
		}
		if (!lcd_kit_dsi_fifo_is_full(dsi_base_addr))
			mipi_dsi_cmds_tx(&dsi_cmd, 1, dsi_base_addr);
		if (is_dual_mipi_panel(hisifd)) {
			if (!lcd_kit_dsi_fifo_is_full(hisifd->mipi_dsi1_base))
				mipi_dsi_cmds_tx(&dsi_cmd, 1,
					hisifd->mipi_dsi1_base);
		}
		lcd_kit_delay(cmds->cmds[i].wait, cmds->cmds[i].waittype,
			false);
	}
	if (cmds->link_state == LCD_KIT_DSI_LP_MODE)
		lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_HS_MODE);
	return ret;

}

static int lcd_kit_buf_trans(const char *inbuf, int inlen, char **outbuf,
	int *outlen)
{
	char *buf = NULL;
	int i;
	int bufsize = inlen;

	if (!inbuf) {
		LCD_KIT_ERR("inbuf is null point!\n");
		return LCD_KIT_FAIL;
	}
	/* The property is 4bytes long per element in cells: <> */
	bufsize = bufsize / 4;
	if (bufsize <= 0) {
		LCD_KIT_ERR("bufsize is less 0!\n");
		return LCD_KIT_FAIL;
	}
	/* If use bype property: [], this division should be removed */
	buf = kzalloc(sizeof(char) * bufsize, GFP_KERNEL);
	if (!buf) {
		LCD_KIT_ERR("buf is null point!\n");
		return LCD_KIT_FAIL;
	}
	// For use cells property: <>
	for (i = 0; i < bufsize; i++)
		buf[i] = inbuf[i * 4 + 3];
	*outbuf = buf;
	*outlen = bufsize;
	return LCD_KIT_OK;
}

static int lcd_kit_gpio_enable(u32 type)
{
	lcd_kit_gpio_tx(type, GPIO_REQ);
	lcd_kit_gpio_tx(type, GPIO_HIGH);
	return LCD_KIT_OK;
}

static int lcd_kit_gpio_disable(u32 type)
{
	lcd_kit_gpio_tx(type, GPIO_LOW);
	lcd_kit_gpio_tx(type, GPIO_FREE);
	return LCD_KIT_OK;
}

static int lcd_kit_gpio_disable_plugin(u32 type)
{
	lcd_kit_gpio_tx_plugin(type, GPIO_LOW);
	lcd_kit_gpio_tx_plugin(type, GPIO_FREE);
	return LCD_KIT_OK;
}

static int lcd_kit_gpio_enable_nolock(u32 type)
{
	lcd_kit_gpio_tx(type, GPIO_REQ);
	lcd_kit_gpio_tx(type, GPIO_HIGH);
	lcd_kit_gpio_tx(type, GPIO_FREE);
	return LCD_KIT_OK;
}

static int lcd_kit_gpio_disable_nolock(u32 type)
{
	lcd_kit_gpio_tx(type, GPIO_REQ);
	lcd_kit_gpio_tx(type, GPIO_LOW);
	lcd_kit_gpio_tx(type, GPIO_FREE);
	return LCD_KIT_OK;
}
static int lcd_kit_regulator_enable(u32 type)
{
	int ret = LCD_KIT_OK;

	switch (type) {
	case LCD_KIT_VCI:
	case LCD_KIT_IOVCC:
	case LCD_KIT_VDD:
		ret = lcd_kit_pmu_ctrl(type, 1);
		break;
	case LCD_KIT_VSP:
	case LCD_KIT_VSN:
	case LCD_KIT_BL:
		ret = lcd_kit_charger_ctrl(type, 1);
		break;
	default:
		ret = LCD_KIT_FAIL;
		LCD_KIT_ERR("regulator type:%d not support\n", type);
		break;
	}
	return ret;
}

static int lcd_kit_regulator_disable(u32 type)
{
	int ret = LCD_KIT_OK;

	switch (type) {
	case LCD_KIT_VCI:
	case LCD_KIT_IOVCC:
	case LCD_KIT_VDD:
		ret = lcd_kit_pmu_ctrl(type, 0);
		break;
	case LCD_KIT_VSP:
	case LCD_KIT_VSN:
	case LCD_KIT_BL:
		ret = lcd_kit_charger_ctrl(type, 0);
		break;
	default:
		LCD_KIT_ERR("regulator type:%d not support\n", type);
		break;
	}
	return ret;
}

static int lcd_kit_regulator_disable_plugin(u32 type)
{
	int ret = LCD_KIT_OK;

	switch (type) {
	case LCD_KIT_VCI:
	case LCD_KIT_IOVCC:
	case LCD_KIT_VDD:
		ret = lcd_kit_pmu_ctrl_plugin(type, 0);
		break;
	default:
		LCD_KIT_ERR("regulator type:%d not support\n", type);
		break;
	}
	return ret;
}

static int lcd_kit_lock(void *hld)
{
	struct hisi_fb_data_type *hisifd = NULL;

	if (!hld) {
		LCD_KIT_ERR("hld is void\n");
		return LCD_KIT_FAIL;
	}
	hisifd = (struct hisi_fb_data_type *)hld;

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		LCD_KIT_ERR("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}
	return LCD_KIT_OK;
err_out:
	up(&hisifd->blank_sem);
	return LCD_KIT_FAIL;
}

static void lcd_kit_release(void *hld)
{
	struct hisi_fb_data_type *hisifd = NULL;

	if (!hld) {
		LCD_KIT_ERR("hld is void\n");
		return;
	}
	hisifd = (struct hisi_fb_data_type *)hld;

	up(&hisifd->blank_sem);
}

void *lcd_kit_get_pdata_hld(void)
{
	return hisifd_list[PRIMARY_PANEL_IDX];
}

struct lcd_kit_adapt_ops adapt_ops = {
	.mipi_tx = lcd_kit_dsi_cmds_tx,
	.mipi_rx = lcd_kit_dsi_cmds_rx,
	.gpio_enable = lcd_kit_gpio_enable,
	.gpio_disable = lcd_kit_gpio_disable,
	.gpio_disable_plugin = lcd_kit_gpio_disable_plugin,
	.gpio_enable_nolock = lcd_kit_gpio_enable_nolock,
	.gpio_disable_nolock = lcd_kit_gpio_disable_nolock,
	.regulator_enable = lcd_kit_regulator_enable,
	.regulator_disable = lcd_kit_regulator_disable,
	.regulator_disable_plugin = lcd_kit_regulator_disable_plugin,
	.buf_trans = lcd_kit_buf_trans,
	.lock = lcd_kit_lock,
	.release = lcd_kit_release,
	.get_pdata_hld = lcd_kit_get_pdata_hld,
};
int lcd_kit_adapt_init(void)
{
	int ret;

	ret = lcd_kit_adapt_register(&adapt_ops);
	return ret;
}

bool lcd_kit_is_current_frame_ok_to_set_backlight(
	struct hisi_fb_data_type *hisifd)
{
	// should only be invoked in online_play
	uint8_t value = 0;
	struct hisi_panel_info *pinfo = &(hisifd->panel_info);
	uint64_t timestamp_start = ktime_to_us(ktime_get());
	if (disp_info->frame_odd_even.support == 0)
		return true;

	LCD_KIT_INFO("enter");
	if (pinfo == NULL) {
		LCD_KIT_ERR("pinfo is NULL");
		return true;
	}
	if (pinfo->fps != fps_60hz) {
		LCD_KIT_DEBUG("pinfo->fps != FPS_60HZ, returned");
		return true;
	}
	if (disp_info->frame_odd_even.read.cmds == NULL) {
		LCD_KIT_ERR("disp_info->frame_odd_even.read.cmds == NULL!");
		return true;
	}
	// sleep one frame before single_frame-update to make sure mipi is idle
	usleep_range(frame_time_us_60hz, frame_time_us_60hz);
	if (lcd_kit_dsi_cmds_rx(hisifd, &value, 0,
		&disp_info->frame_odd_even.read)) { // read 1 byte, out_len: 0
		LCD_KIT_ERR("read mipi_rx failed!");
		return true;
	}
	LCD_KIT_INFO("read returns %d, cost %ld ms", value,
		(ktime_to_us(ktime_get()) - timestamp_start) / 1000); // to ms
	return (value == 0x03); // 0x03: odd frame, 0x02 even frame
}

bool lcd_kit_is_current_frame_ok_to_set_fp_backlight(
	struct hisi_fb_data_type *hisifd, int backlight_type)
{
	// should only be invoked in online_play
	uint8_t value = 0;
	uint64_t timestamp_start;
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is NULL");
		return true;
	}
	pinfo = &(hisifd->panel_info);
	timestamp_start = ktime_to_us(ktime_get());
	if (disp_info->frame_odd_even.support == 0)
		return true;
	LCD_KIT_INFO("enter curret level:%d, pwm_thre_level:%d",
		hisifd->de_info.actual_bl_level,
		hisifd->de_info.amoled_param.Lowac_Fixed_DBVThres);
	if (backlight_type == BACKLIGHT_HIGH_LEVEL &&
		hisifd->de_info.actual_bl_level >=
		hisifd->de_info.amoled_param.Lowac_Fixed_DBVThres)
		return true;
	if (pinfo == NULL) {
		LCD_KIT_ERR("pinfo is NULL");
		return true;
	}
	if (pinfo->fps != fps_60hz) {
		LCD_KIT_DEBUG("pinfo->fps != FPS_60HZ, returned");
		return true;
	}
	if (disp_info->frame_odd_even.read.cmds == NULL) {
		LCD_KIT_ERR("disp_info->frame_odd_even.read.cmds == NULL!");
		return true;
	}

	// sleep one frame before odd_frame-update to make sure mipi is idle
	usleep_range(frame_time_us_60hz, frame_time_us_60hz);
	if (lcd_kit_dsi_cmds_rx(hisifd, &value, 0,
		&disp_info->frame_odd_even.read)) { // read one byte, out_len:0
		LCD_KIT_ERR("read mipi_rx failed!");
		return true;
	}
	LCD_KIT_INFO("read returns %d, cost %ld ms", value,
		(ktime_to_us(ktime_get()) - timestamp_start) / 1000); // to ms
	return (value == 0x02); // 0x03: odd frame, 0x02 even frame
}

static bool lcd_kit_is_need_swap_fps_gamma(struct hisi_fb_data_type *hisifd)
{
	char* name = NULL;
	if (disp_info->fps_gamma.support == 0 ||
		disp_info->fps_gamma.check_flag == 0) {
		LCD_KIT_DEBUG("no need to swap due to support or check_flag");
		return false;
	}
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is NULL!");
		return false;
	}
	name = common_info->panel_model != NULL ?
		common_info->panel_model : disp_info->compatible;
	if (name == NULL) {
		LCD_KIT_ERR("name is NULL!");
		return false;
	}
	if (disp_info->fps_gamma.swap_support == 0) {
		LCD_KIT_INFO("no need to swap due to configure");
		return false;
	}
	if (disp_info->panel_version.support == 0) {
		LCD_KIT_ERR("panel_version.support == 0");
		return false;
	}
	if ((strcmp(hisifd->panel_info.lcd_panel_version, " VER:V4") != 0) &&
		(strcmp(hisifd->panel_info.lcd_panel_version, " VER:V3") != 0)) {
		LCD_KIT_INFO("no need to swap due to panel_version");
		return false;
	}
	return true;
}

static void lcd_kit_swap_fps_gamma(struct lcd_kit_dsi_panel_cmds *gamma_cmd_1,
	struct lcd_kit_dsi_panel_cmds *gamma_cmd_2)
{
	struct lcd_kit_dsi_panel_cmds temp;
	if (gamma_cmd_1 == NULL || gamma_cmd_2 == NULL) {
		LCD_KIT_ERR("gamma_cmd is NULL!");
		return;
	}
	temp = *gamma_cmd_1;
	*gamma_cmd_1 = *gamma_cmd_2;
	*gamma_cmd_2 = temp;
}

/* should be invoked after lcd_kit_read_fps_gamma(void *hld) */
void lcd_kit_swap_fps_gamma_if_needed(struct hisi_fb_data_type *hisifd)
{
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is NULL!");
		return;
	}
	if (lcd_kit_is_need_swap_fps_gamma(hisifd)) {
		lcd_kit_swap_fps_gamma(&disp_info->fps_gamma.write_60hz,
			&disp_info->fps_gamma.write_90hz);
		lcd_kit_write_fps_gamma_by_scene(hisifd, LCD_FPS_SCENCE_60P);
		LCD_KIT_INFO("fps gamma swapped");
	}
}

void lcd_kit_optimize_grayscale(struct hisi_fb_data_type *hisifd)
{
	int ret = LCD_KIT_OK;

	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is NULL");
		return;
	}
	if (disp_info->panel_version.support == 0) {
		LCD_KIT_ERR("panel_version.support == 0");
		return;
	}
	LCD_KIT_DEBUG("grayscale_optimize.support: %d",
		common_info->grayscale_optimize.support);

	/* only special panel version need write grayscale optimize cmd */
	if (strcmp(hisifd->panel_info.lcd_panel_version, " VER:VN2") != 0) {
		LCD_KIT_DEBUG("panel ver: %s\n", hisifd->panel_info.lcd_panel_version);
		return;
	}

	/* grayscale optimize */
	if (common_info->grayscale_optimize.support) {
		LCD_KIT_INFO("write grayscale optimize cmds");
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&common_info->grayscale_optimize.cmds);
		if (ret != 0)
			LCD_KIT_ERR("send grayscale optimize cmds error");
		else
			LCD_KIT_INFO("grayscale optimize success");
	}
}

/* take care of effects after panel on in kernel */
void lcd_kit_display_effect_screen_on_default(struct hisi_fb_data_type *hisifd)
{
	if (!common_info->screen_on_effect.support) {
		LCD_KIT_DEBUG("screen_on_effect is not supported");
		return;
	}

	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is NULL");
		return;
	}

	if (common_info->screen_on_effect.prepare_cmds.cmds) {
		LCD_KIT_INFO("write panel effect prepare_cmds");
		if (lcd_kit_dsi_cmds_tx(hisifd,
			&common_info->screen_on_effect.prepare_cmds)) {
			LCD_KIT_ERR("prepare_cmds failed!");
			return;
		}
	}
	if (common_info->screen_on_effect.cmds.cmds) {
		LCD_KIT_INFO("write panel effect cmds");
		if (lcd_kit_dsi_cmds_tx(hisifd,
			&common_info->screen_on_effect.cmds)) {
			LCD_KIT_ERR("cmds failed!");
			return;
		}
	}
	if (common_info->screen_on_effect.exit_cmds.cmds) {
		LCD_KIT_INFO("write panel effect exit_cmds");
		if (lcd_kit_dsi_cmds_tx(hisifd,
			&common_info->screen_on_effect.exit_cmds)) {
			LCD_KIT_ERR("exit_cmds failed!");
			return;
		}
	}
}

/* handle ddic display effects */
/* will be invoked on LCD_INIT_MIPI_HS_SEND_SEQUENCE */
void lcd_kit_display_effect_screen_on(struct hisi_fb_data_type *hisifd)
{
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is NULL");
		return;
	}

	/* screen on default effects */
	lcd_kit_display_effect_screen_on_default(hisifd);

	/* fps update */
	if (common_info->dfr_info.fps_lock_command_support) {
		disp_info->fps.last_update_fps = LCD_FPS_60;
		LCD_KIT_INFO("screen on,hbm&fps lock is support");
	}

	if (common_info->hbm.hbm_fp_support) {
		mutex_lock(&COMMON_LOCK->hbm_lock);
		common_info->hbm.hbm_if_fp_is_using = 0;
		mutex_unlock(&COMMON_LOCK->hbm_lock);
	}

	/* ddic irc */
	if (disp_info->panel_irc.support == 1) {
		if (lcd_kit_set_panel_irc(hisifd,
			hisifd->de_param.ddic_irc.irc_enable) != LCD_KIT_OK)
			LCD_KIT_ERR("lcd_kit_set_panel_irc failed!");
	}

	/* swap fps gamma */
	if (lcd_kit_is_need_swap_fps_gamma(hisifd))
		lcd_kit_write_fps_gamma_by_scene(hisifd,
			LCD_FPS_SCENCE_60P);
		LCD_KIT_INFO("fps gamma swapped");

	/* grayscale optimize */
	lcd_kit_optimize_grayscale(hisifd);
}

int lcd_kit_set_panel_irc(struct hisi_fb_data_type *hisifd, bool enable)
{
	if (disp_info->panel_irc.support != 1) {
		LCD_KIT_INFO("not support irc!");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("enter, enable = %d", enable);
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is NULL");
		return LCD_KIT_FAIL;
	}
	if (enable) {
		if (lcd_kit_dsi_cmds_tx(hisifd, &disp_info->panel_irc.on)) {
			LCD_KIT_ERR("mipi_tx failed!");
			return LCD_KIT_FAIL;
		}
	} else {
		if (lcd_kit_dsi_cmds_tx(hisifd, &disp_info->panel_irc.off)) {
			LCD_KIT_ERR("mipi_tx failed!");
			return LCD_KIT_FAIL;
		}
	}
	LCD_KIT_INFO("exit, enable = %d", enable);
	return LCD_KIT_OK;
}
