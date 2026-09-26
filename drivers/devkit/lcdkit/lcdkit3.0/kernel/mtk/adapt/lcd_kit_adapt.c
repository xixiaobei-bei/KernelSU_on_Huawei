/*
 * lcd_kit_adapt.c
 *
 * lcdkit adapt function for lcd driver
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#include "lcd_kit_common.h"
#include "lcd_kit_dbg.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_parse.h"
#include "lcd_kit_power.h"
#ifdef CONFIG_DRM_MEDIATEK
#include <drm/drm_mipi_dsi.h>
#include "mtk_panel_ext.h"
#else
#include "lcm_drv.h"
#endif

#ifdef CONFIG_DRM_MEDIATEK
#define MAX_TX_LEN_FOR_MIPI 16

struct dsi_cmd_desc {
	unsigned int dtype;
	unsigned int vc;
	unsigned int link_state;
	unsigned int tx_len[MAX_TX_LEN_FOR_MIPI];
	const void *tx_buf[MAX_TX_LEN_FOR_MIPI];
	unsigned int rx_len;
	char *rx_buf;
};
#endif

#ifdef CONFIG_DRM_MEDIATEK

int lcd_kit_dsi_cmds_extern_tx(struct lcd_kit_dsi_panel_cmds *cmds);

static int lcd_kit_cmd_is_write(struct lcd_kit_dsi_cmd_desc *cmd)
{
	int ret;

	if (cmd == NULL) {
		LCD_KIT_ERR("cmd is NULL!\n");
		return LCD_KIT_FAIL;
	}

	switch (cmd->dtype) {
	case DTYPE_GEN_WRITE:
	case DTYPE_GEN_WRITE1:
	case DTYPE_GEN_WRITE2:
	case DTYPE_GEN_LWRITE:
	case DTYPE_DCS_WRITE:
	case DTYPE_DCS_WRITE1:
	case DTYPE_DCS_LWRITE:
	case DTYPE_DSC_LWRITE:
		ret = LCD_KIT_FAIL;
		break;
	case DTYPE_GEN_READ:
	case DTYPE_GEN_READ1:
	case DTYPE_GEN_READ2:
	case DTYPE_DCS_READ:
		ret = LCD_KIT_OK;
		break;
	default:
		ret = LCD_KIT_FAIL;
		break;
	}
	return ret;
}

void mipi_dsi_cmds_tx(struct mipi_dsi_device *dsi, struct dsi_cmd_desc *cmds)
{
	ssize_t ret;

	if (cmds == NULL || dsi == NULL) {
		LCD_KIT_ERR("cmds or dsi is null");
		return;
	}
	switch (cmds->dtype) {
	case DTYPE_DCS_WRITE:
	case DTYPE_DCS_WRITE1:
	case DTYPE_DCS_LWRITE:
	case DTYPE_DSC_LWRITE:
		ret = mipi_dsi_dcs_write_buffer(dsi, cmds->tx_buf[0], cmds->tx_len[0]);
		break;
	case DTYPE_GEN_WRITE:
	case DTYPE_GEN_WRITE1:
	case DTYPE_GEN_WRITE2:
	case DTYPE_GEN_LWRITE:
		ret = mipi_dsi_generic_write(dsi, cmds->tx_buf[0], cmds->tx_len[0]);
		break;
	default:
		ret = mipi_dsi_dcs_write_buffer(dsi, cmds->tx_buf[0], cmds->tx_len[0]);
		break;
	}

	if (ret < 0)
		LCD_KIT_ERR("write error is %d cmd is 0x%x\n", ret, *((unsigned char *)cmds->tx_buf[0]));
}

void mipi_dsi_cmds_rx(struct mipi_dsi_device *dsi, struct dsi_cmd_desc *cmds)
{
	ssize_t ret = 0;

	if (cmds == NULL || dsi == NULL) {
		LCD_KIT_ERR("cmds or dsi is null");
		return;
	}
	switch (cmds->dtype) {
	case DTYPE_DCS_READ:
		ret = mipi_dsi_dcs_read(dsi, *(unsigned char *)cmds->tx_buf[0],
			cmds->rx_buf, cmds->rx_len);
		break;
	case DTYPE_GEN_READ:
	case DTYPE_GEN_READ1:
	case DTYPE_GEN_READ2:
		ret = mipi_dsi_generic_read(dsi, cmds->tx_buf[0],
			(size_t)cmds->tx_len, cmds->rx_buf, (size_t)cmds->rx_len);
		break;
	default:
		break;
	}

	if (ret < 0)
		LCD_KIT_ERR("read error is %d cmd is 0x%x\n", ret, *((unsigned char *)cmds->tx_buf[0]));
}

static int lcd_kit_cmds_to_mtk_dsi_cmds(struct lcd_kit_dsi_cmd_desc *lcd_kit_cmds,
	struct dsi_cmd_desc *cmd, int link_state)
{
	if (lcd_kit_cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is null point!\n");
		return LCD_KIT_FAIL;
	}
	if (cmd == NULL) {
		LCD_KIT_ERR("cmd is null point!\n");
		return LCD_KIT_FAIL;
	}
	cmd->dtype = lcd_kit_cmds->dtype;
	cmd->vc = lcd_kit_cmds->vc;
	cmd->link_state = link_state;
	cmd->tx_len[0] = lcd_kit_cmds->dlen;
	cmd->tx_buf[0] = lcd_kit_cmds->payload;

	return LCD_KIT_OK;
}

static int lcd_kit_cmds_to_mtk_dsi_read_cmds(struct lcd_kit_dsi_cmd_desc *lcd_kit_cmds,
	unsigned char *out, int out_len,
	struct dsi_cmd_desc *cmd, int link_state)
{
	if (lcd_kit_cmds == NULL || cmd == NULL || out == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds or cmd or out is null\n");
		return LCD_KIT_FAIL;
	}
	cmd->dtype = lcd_kit_cmds->dtype;
	cmd->vc = lcd_kit_cmds->vc;
	cmd->link_state = link_state;
	cmd->tx_len[0] = lcd_kit_cmds->dlen;
	cmd->tx_buf[0] = lcd_kit_cmds->payload;
	cmd->rx_len = out_len;
	cmd->rx_buf = out;

	return LCD_KIT_OK;
}

int mtk_mipi_dsi_cmds_tx(void *dsi, struct lcd_kit_dsi_cmd_desc *cmds,
	int cnt, int link_state)
{
	struct lcd_kit_dsi_cmd_desc *cm = NULL;
	struct dsi_cmd_desc dsi_cmd = {0};
	int i;

	if (cmds == NULL || dsi == NULL) {
		LCD_KIT_ERR("cmds or dsi is null");
		return -EINVAL;
	}
	cm = cmds;
	for (i = 0; i < cnt; i++) {
		lcd_kit_cmds_to_mtk_dsi_cmds(cm, &dsi_cmd, link_state);
		mipi_dsi_cmds_tx(dsi, &dsi_cmd);
		if (!(cm->wait)) {
			cm++;
			continue;
		}
		if (cm->waittype == WAIT_TYPE_US) {
			udelay(cm->wait);
		} else if (cm->waittype == WAIT_TYPE_MS) {
			if (cm->wait <= 10)
				mdelay(cm->wait);
			else
				msleep(cm->wait);
		} else {
			msleep(cm->wait * 1000); // change s to ms
		}
		cm++;
	}

	return cnt;
}

int lcd_kit_dsi_cmds_tx(void *dsi, struct lcd_kit_dsi_panel_cmds *cmds)
{
	int i;

	if (!cmds || !dsi) {
		LCD_KIT_ERR("lcd_kit_cmds or dsi is null\n");
		return LCD_KIT_FAIL;
	}

	if (lcm_get_panel_state()) {
		(void)lcd_kit_dsi_cmds_extern_tx(cmds);
		return LCD_KIT_OK;
	}

	down(&disp_info->lcd_kit_sem);
	for (i = 0; i < cmds->cmd_cnt; i++)
		mtk_mipi_dsi_cmds_tx(dsi, &cmds->cmds[i], 1, cmds->link_state);
	up(&disp_info->lcd_kit_sem);
	return LCD_KIT_OK;
}

int lcd_kit_dsi_cmds_rx(void *dsi, unsigned char *out, int out_len,
	struct lcd_kit_dsi_panel_cmds *cmds)
{
	int i;
	int ret = 0;
	struct dsi_cmd_desc dsi_cmd = {0};

	if ((cmds == NULL) || (out == NULL) || (dsi == NULL)) {
		LCD_KIT_ERR("out or cmds or dsi is null\n");
		return LCD_KIT_FAIL;
	}

	down(&disp_info->lcd_kit_sem);
	for (i = 0; i < cmds->cmd_cnt; i++) {
		if (lcd_kit_cmd_is_write(&cmds->cmds[i])) {
			lcd_kit_cmds_to_mtk_dsi_cmds(&cmds->cmds[i], &dsi_cmd,
				cmds->link_state);
			ret = mtk_mipi_dsi_cmds_tx(dsi, &cmds->cmds[i],
				1, cmds->link_state);
			if (ret < 0)
				LCD_KIT_ERR("mipi write error\n");
		} else {
			lcd_kit_cmds_to_mtk_dsi_read_cmds(&cmds->cmds[i], &out[0], out_len,
				&dsi_cmd, cmds->link_state);
			mipi_dsi_cmds_rx(dsi, &dsi_cmd);
		}
	}
	up(&disp_info->lcd_kit_sem);
	return ret;
}

static int lcd_kit_cmds_to_mtk_cmds_extern_tx(
	struct lcd_kit_dsi_cmd_desc *lcd_kit_cmds,
	struct mtk_ddic_dsi_msg *cmd_msg, int link_state)
{
	if (lcd_kit_cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (cmd_msg == NULL) {
		LCD_KIT_ERR("cmd_msg is NULL\n");
		return LCD_KIT_FAIL;
	}
	/* 2: mipi LP; 0: mipi HS */
	if (link_state == 0)
		link_state = MIPI_DSI_MSG_USE_LPM;
	else
		link_state = 0;

	cmd_msg->channel = 0;
	cmd_msg->flags = link_state;
	cmd_msg->tx_cmd_num = 1;
	cmd_msg->type[0] = lcd_kit_cmds->dtype;
	cmd_msg->tx_buf[0] = lcd_kit_cmds->payload;
	cmd_msg->tx_len[0] = lcd_kit_cmds->dlen;

	return LCD_KIT_OK;
}

static int lcd_kit_cmds_to_mtk_cmds_extern_rx(
	struct lcd_kit_dsi_cmd_desc *lcd_kit_cmds,
	struct mtk_ddic_dsi_msg *cmd_msg, int link_state,
	unsigned char *buf)
{
	if (lcd_kit_cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (cmd_msg == NULL) {
		LCD_KIT_ERR("cmd_msg is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (buf == NULL) {
		LCD_KIT_ERR("buf is NULL\n");
		return LCD_KIT_FAIL;
	}
	/* 1: mipi LP; 0: mipi HS */
	if (link_state == 0)
		link_state = 1;
	else
		link_state = 0;

	cmd_msg->channel = 0;
	cmd_msg->flags = link_state;
	cmd_msg->tx_cmd_num = 1;
	cmd_msg->type[0] = lcd_kit_cmds->dtype;
	cmd_msg->tx_buf[0] = lcd_kit_cmds->payload;
	if (cmd_msg->type[0] == DTYPE_GEN_READ2)
		/* long read, 2 parameter */
		cmd_msg->tx_len[0] = 2;
	else
		cmd_msg->tx_len[0] = 1;

	cmd_msg->rx_cmd_num = 1;
	cmd_msg->rx_buf[0] = buf;
	cmd_msg->rx_len[0] = lcd_kit_cmds->dlen;

	return LCD_KIT_OK;
}

int mtk_mipi_dsi_cmds_extern_tx(struct lcd_kit_dsi_cmd_desc *cmds,
	int cnt, int link_state, bool lock_state)
{
	struct lcd_kit_dsi_cmd_desc *cm = NULL;
	struct mtk_ddic_dsi_msg cmd_msg = {0};
	int i;
	int ret;

	if (cmds == NULL) {
		LCD_KIT_ERR("cmds is NULL");
		return -EINVAL;
	}
	cm = cmds;
	for (i = 0; i < cnt; i++) {
		ret = lcd_kit_cmds_to_mtk_cmds_extern_tx(cm,
			&cmd_msg, link_state);
		if (ret != LCD_KIT_OK) {
			LCD_KIT_ERR("exchange dsi mipi fail\n");
			return -EINVAL;
		}
		if (lock_state)
			ret = mtk_ddic_dsi_send_cmd(&cmd_msg, true);
		else
			ret = mtk_ddic_dsi_send_cmd_nolock(&cmd_msg, true);
		if (ret != 0)
			LCD_KIT_ERR("send cmd fail dtype:0x%x, len:%d\n",
				cmd_msg.type[0], cmd_msg.tx_len[0]);
		else
			LCD_KIT_ERR("dtype is 0x%x, len is %d\n",
				cmd_msg.type[0], cmd_msg.tx_len[0]);

		if (cm->wait == 0) {
			cm++;
			continue;
		}
		if (cm->waittype == WAIT_TYPE_US) {
			udelay(cm->wait);
		} else if (cm->waittype == WAIT_TYPE_MS) {
			if (cm->wait <= 10)
				mdelay(cm->wait);
			else
				msleep(cm->wait);
		} else {
			/* change s to ms */
			msleep(cm->wait * 1000);
		}
		cm++;
	}
	return cnt;
}

int lcd_kit_dsi_cmds_extern_rx(unsigned char *out,
	struct lcd_kit_dsi_panel_cmds *cmds, unsigned int len)
{
	unsigned int i;
	unsigned int j;
	unsigned int k = 0;
	struct mtk_ddic_dsi_msg cmd_msg = {0};
	int ret;
	unsigned char buffer[MTK_READ_MAX_LEN] = {0};

	if ((cmds == NULL)  || (out == NULL) || (len == 0)) {
		LCD_KIT_ERR("out or cmds is NULL\n");
		return LCD_KIT_FAIL;
	}

	for (i = 0; i < cmds->cmd_cnt; i++) {
		if (lcd_kit_cmd_is_write(&cmds->cmds[i])) {
			ret = mtk_mipi_dsi_cmds_extern_tx(&cmds->cmds[i], 1,
			cmds->link_state, true);
			if (ret < 0) {
				LCD_KIT_ERR("the %d mipi cmd send fail\n", i);
				return LCD_KIT_FAIL;
			}
		} else {
			ret = lcd_kit_cmds_to_mtk_cmds_extern_rx(&cmds->cmds[i],
				&cmd_msg, cmds->link_state, &buffer[0]);
			if (ret != LCD_KIT_OK) {
				LCD_KIT_ERR("exchange dsi mipi fail\n");
				return LCD_KIT_FAIL;
			}
			if (cmd_msg.rx_len[0] == 0) {
				LCD_KIT_ERR("cmd len is 0\n");
				return LCD_KIT_FAIL;
			}
			LCD_KIT_INFO("cmd len is %d!\n", cmd_msg.rx_len[0]);
			ret = mtk_ddic_dsi_read_cmd(&cmd_msg);
			if (ret != 0) {
				LCD_KIT_ERR("dsi mipi read fail\n");
				return LCD_KIT_FAIL;
			}
			for (j = 0; j < cmd_msg.rx_len[0]; j++) {
				out[k] = buffer[j];
				LCD_KIT_INFO("read the %d value is 0x%x\n", k, out[k]);
				k++;
				if (k == len) {
					LCD_KIT_INFO("buffer is full, len is %d\n", len);
					break;
				}
			}
		}
	}
	return LCD_KIT_OK;
}

int lcd_kit_dsi_cmds_extern_tx(struct lcd_kit_dsi_panel_cmds *cmds)
{
	int ret = LCD_KIT_OK;
	int i;

	if (cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < cmds->cmd_cnt; i++)
		ret = mtk_mipi_dsi_cmds_extern_tx(&cmds->cmds[i], 1,
			cmds->link_state, true);

	return ret;
}

int lcd_kit_dsi_cmds_extern_tx_nolock(struct lcd_kit_dsi_panel_cmds *cmds)
{
	int ret = LCD_KIT_OK;
	int i;

	if (cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < cmds->cmd_cnt; i++)
		ret = mtk_mipi_dsi_cmds_extern_tx(&cmds->cmds[i], 1,
			cmds->link_state, false);

	return ret;
}

int cb_tx(void *dsi_drv, void *cb, void *handle,
	struct lcd_kit_dsi_panel_cmds *pcmds)
{
	dcs_write_gce extern_cb = (dcs_write_gce)cb;
	int i;
	int j;

	if (!dsi_drv || !cb || !pcmds) {
		LCD_KIT_ERR("parameter error\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_DEBUG("cmd_cnt:%d\n", pcmds->cmd_cnt);

	for (i = 0; i < pcmds->cmd_cnt; i++) {
		extern_cb(dsi_drv, handle, pcmds->cmds[i].payload,
			pcmds->cmds[i].dlen);
		lcd_kit_delay(pcmds->cmds[i].wait,
			pcmds->cmds[i].waittype, true);
		LCD_KIT_DEBUG("dlen:%d, payload:", pcmds->cmds[i].dlen);
		for (j = 0; j < pcmds->cmds[i].dlen; j++)
			LCD_KIT_DEBUG("0x%x ", pcmds->cmds[i].payload[j]);
		LCD_KIT_DEBUG(" \n");
	}
	return LCD_KIT_OK;
}

#else

extern struct LCM_UTIL_FUNCS lcm_util_mtk;
extern int do_lcm_vdo_lp_write(struct dsi_cmd_desc *write_table,
	unsigned int count);
extern int do_lcm_vdo_lp_read(struct dsi_cmd_desc *cmd_tab, unsigned int count);
extern int do_lcm_vdo_lp_read_v1(struct dsi_cmd_desc *cmd_tab);
#define mipi_dsi_cmds_tx(cmdq, cmds) lcm_util_mtk.mipi_dsi_cmds_tx(cmdq, cmds)
#define mipi_dsi_cmds_rx(out, cmds, len) lcm_util_mtk.mipi_dsi_cmds_rx(out, cmds, len)

#ifdef CONFIG_MACH_MT6768
#define dsi_set_cmdq_v5(ppara, size, hs) \
	lcm_util_mtk.dsi_set_cmdq_V5(ppara, size, hs)

#define dsi_set_cmdq_v22(cmdq, cmd, count, ppara, force_update) \
	lcm_util_mtk.dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update)
int lcd_kit_fp_highlight_cmds_tx(void *handle,
	const struct lcd_kit_dsi_panel_cmds *pcmds)
{
	int i;
	int j;
	unsigned char payload[128] = {0};

	if (handle == NULL || pcmds == NULL) {
		LCD_KIT_ERR("handle or cmds is null!\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("cmd_cnt=%d\n", pcmds->cmd_cnt);
	for (i = 0; i < pcmds->cmd_cnt; i++) {
		if (pcmds->cmds[i].dlen - 1 > sizeof(payload))
			return LCD_KIT_FAIL;
		memcpy(&payload, pcmds->cmds[i].payload + 1,
			pcmds->cmds[i].dlen - 1);
		LCD_KIT_DEBUG("cmd=0x%X, count=%d\n",
			pcmds->cmds[i].payload[0], pcmds->cmds[i].dlen - 1);
		LCD_KIT_DEBUG("payload=");
		for (j = 0; j < pcmds->cmds[i].dlen - 1; j++)
			LCD_KIT_DEBUG("0x%X, ", payload[j]);
		LCD_KIT_DEBUG("\n");
		dsi_set_cmdq_v22(handle, pcmds->cmds[i].payload[0],
			pcmds->cmds[i].dlen - 1, payload, 1);
	}
	return LCD_KIT_OK;
}

static int get_setting_table_len(const struct lcd_kit_dsi_panel_cmds *cmds)
{
	int cmd_count;
	int i;

	LCD_KIT_INFO("get_setting_table_len +\n");
	if (cmds == NULL)
		return 0;
	cmd_count = cmds->cmd_cnt;
	for (i = 0; i < cmds->cmd_cnt; i++) {
		if (cmds->cmds[i].waittype == WAIT_TYPE_MS)
			cmd_count++;
	}
	LCD_KIT_INFO("get_setting_table_len -,cmd_count=%d\n", cmd_count);
	return cmd_count;
}

static int get_setting_table(const struct lcd_kit_dsi_panel_cmds *cmds,
	struct LCM_setting_table_V3 *setting_table, int len)
{
	struct LCM_setting_table_V3 table;
	int i;
	int index = 0;

	LCD_KIT_INFO("get_setting_table +\n");
	if (cmds == NULL || setting_table == NULL) {
		LCD_KIT_ERR("cmds or setting_table is NULL\n");
		return LCD_KIT_FAIL;
	}

	for (i = 0; i < cmds->cmd_cnt; i++) {
		if (len <= index) {
			LCD_KIT_ERR("setting_table size is not enough\n");
			return LCD_KIT_FAIL;
		}
		memset(&table.para_list, 0, sizeof(table.para_list));
		table.id = REGFLAG_ESCAPE_ID;
		table.cmd = cmds->cmds[i].payload[0];
		table.count = cmds->cmds[i].dlen - 1;
		if (table.count >= sizeof(table.para_list)) {
			LCD_KIT_ERR("table.count is longer than para_list size\n");
			return LCD_KIT_FAIL;
		}
		memcpy(&table.para_list, cmds->cmds[i].payload + 1, table.count);
		memcpy(setting_table + index, &table, sizeof(table));
		index++;
		if (cmds->cmds[i].waittype == WAIT_TYPE_MS) {
			if (len <= index) {
				LCD_KIT_ERR("setting_table size is not enough\n");
				return LCD_KIT_FAIL;
			}
			table.cmd = REGFLAG_DELAY_MS_V3;
			table.count = cmds->cmds[i].wait;
			memset(&table.para_list[0], 0, sizeof(table.para_list));
			memcpy(setting_table + index, &table, sizeof(table));
			index++;
		}
	}
	LCD_KIT_INFO("get_setting_table -\n");
	return LCD_KIT_OK;
}

void lcd_kit_aod_cmd_tx(const struct lcd_kit_dsi_panel_cmds *cmds)
{
	int table_len;
	struct LCM_setting_table_V3 *setting_table = NULL;
	int setting_table_size;
	int ret;
	int i;
	int j;

	LCD_KIT_INFO("lcd_kit_aod_cmd_tx +\n");
	if (cmds == NULL) {
		LCD_KIT_ERR("cmds is null\n");
		return;
	}
	table_len = get_setting_table_len(cmds);
	if (table_len <= 0) {
		LCD_KIT_ERR("table_len is invalid\n");
		return;
	}
	setting_table_size = table_len * sizeof(*setting_table);
	setting_table =
		(struct LCM_setting_table_V3 *)vmalloc(setting_table_size);
	if (setting_table == NULL)
		return;
	memset(setting_table, 0, setting_table_size);
	ret = get_setting_table(cmds, setting_table, table_len);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("get_setting_table failed\n");
		if (setting_table != NULL)
			vfree(setting_table);
		return;
	}
	for (i = 0; i < table_len; i++) {
		LCD_KIT_DEBUG("id=%d, cmd=0x%X, count=%d\n",
			setting_table[i].id, setting_table[i].cmd,
			setting_table[i].count);
		LCD_KIT_DEBUG("para_list=");
		for (j = 0; j < setting_table[i].count; j++) {
			if (setting_table[i].cmd != REGFLAG_DELAY_MS_V3)
				LCD_KIT_DEBUG("0x%X, ",
					setting_table[i].para_list[j]);
		}
		LCD_KIT_DEBUG("\n");
	}
	dsi_set_cmdq_v5(setting_table, table_len, 0);
	if (setting_table != NULL)
		vfree(setting_table);
	LCD_KIT_INFO("lcd_kit_aod_cmd_tx -\n");
}
#endif

static int lcd_kit_cmd_is_write(struct lcd_kit_dsi_panel_cmds *cmd)
{
	int ret;

	if (cmd == NULL) {
		LCD_KIT_ERR("lcd_kit_cmd is null point!\n");
		return LCD_KIT_FAIL;
	}

	switch (cmd->cmds->dtype) {
	case DTYPE_GEN_WRITE:
	case DTYPE_GEN_WRITE1:
	case DTYPE_GEN_WRITE2:
	case DTYPE_GEN_LWRITE:
	case DTYPE_DCS_WRITE:
	case DTYPE_DCS_WRITE1:
	case DTYPE_DCS_LWRITE:
	case DTYPE_DSC_LWRITE:
		ret = LCD_KIT_FAIL;
		break;
	case DTYPE_GEN_READ:
	case DTYPE_GEN_READ1:
	case DTYPE_GEN_READ2:
	case DTYPE_DCS_READ:
		ret = LCD_KIT_OK;
		break;
	default:
		ret = LCD_KIT_FAIL;
		break;
	}
	return ret;
}

static int lcd_kit_cmd_is_write_v1(struct lcd_kit_dsi_cmd_desc *cmd)
{
	int ret = LCD_KIT_FAIL;

	if (!cmd) {
		LCD_KIT_ERR("cmd is NULL!\n");
		return LCD_KIT_FAIL;
	}

	switch (cmd->dtype) {
	case DTYPE_GEN_WRITE:
	case DTYPE_GEN_WRITE1:
	case DTYPE_GEN_WRITE2:
	case DTYPE_GEN_LWRITE:
	case DTYPE_DCS_WRITE:
	case DTYPE_DCS_WRITE1:
	case DTYPE_DCS_LWRITE:
	case DTYPE_DSC_LWRITE:
		ret = 1;
		break;
	case DTYPE_GEN_READ:
	case DTYPE_GEN_READ1:
	case DTYPE_GEN_READ2:
	case DTYPE_DCS_READ:
		ret = 0;
		break;
	default:
		ret = LCD_KIT_FAIL;
		break;
	}
	return ret;
}

static void lcd_kit_dump_cmd(struct dsi_cmd_desc *cmd)
{
	int i;

	if (cmd == NULL) {
		LCD_KIT_ERR("lcd_kit_cmd is null point!\n");
		return;
	}
	LCD_KIT_DEBUG("cmd->dtype = 0x%x\n", cmd->dtype);
	LCD_KIT_DEBUG("cmd->vc = 0x%x\n", cmd->vc);
	LCD_KIT_DEBUG("cmd->dlen = 0x%x\n", cmd->dlen);
	LCD_KIT_DEBUG("cmd->payload:\n");
	if (lcd_kit_msg_level >= MSG_LEVEL_DEBUG) {
		for (i = 0; i < cmd->dlen; i++)
			LCD_KIT_DEBUG("0x%x\n", cmd->payload[i]);
	}
}

static int lcd_kit_cmds_to_mtk_dsi_cmds(struct lcd_kit_dsi_cmd_desc *lcd_kit_cmds,
	struct dsi_cmd_desc *cmd)
{
	if (lcd_kit_cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is null point!\n");
		return LCD_KIT_FAIL;
	}
	if (cmd == NULL) {
		LCD_KIT_ERR("cmd is null point!\n");
		return LCD_KIT_FAIL;
	}
	cmd->dtype = lcd_kit_cmds->payload[0];
	cmd->vc = lcd_kit_cmds->vc;
	cmd->dlen = lcd_kit_cmds->dlen - 1;
	cmd->payload = &lcd_kit_cmds->payload[1];
	cmd->link_state = 1;
	lcd_kit_dump_cmd(cmd);
	return LCD_KIT_OK;
}

static int lcd_kit_cmds_to_mtk_dsi_read_cmds(struct lcd_kit_dsi_cmd_desc *lcd_kit_cmds,
	struct dsi_cmd_desc *cmd)
{
	if (lcd_kit_cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is null point!\n");
		return LCD_KIT_FAIL;
	}
	if (cmd == NULL) {
		LCD_KIT_ERR("cmd is null point!\n");
		return LCD_KIT_FAIL;
	}
	cmd->dtype = lcd_kit_cmds->payload[0];
	cmd->vc =  lcd_kit_cmds->vc;
	cmd->dlen =  lcd_kit_cmds->dlen;
	cmd->link_state = 1;
	cmd->cmd = lcd_kit_cmds->dtype;
	return LCD_KIT_OK;
}

int mtk_mipi_dsi_cmds_tx(struct lcd_kit_dsi_cmd_desc *cmds, int cnt)
{
	struct lcd_kit_dsi_cmd_desc *cm = NULL;
	struct dsi_cmd_desc dsi_cmd;
	int i;

	if (!cmds) {
		LCD_KIT_ERR("cmds is NULL");
		return -EINVAL;
	}
	cm = cmds;
	for (i = 0; i < cnt; i++) {
		lcd_kit_cmds_to_mtk_dsi_cmds(cm, &dsi_cmd);
		(void)mipi_dsi_cmds_tx(NULL, &dsi_cmd);
		if (!(cm->wait)) {
			cm++;
			continue;
		}
		if (cm->waittype == WAIT_TYPE_US) {
			udelay(cm->wait);
		} else if (cm->waittype == WAIT_TYPE_MS) {
			if (cm->wait <= 10)
				mdelay(cm->wait);
			else
				msleep(cm->wait);
		} else {
			msleep(cm->wait * 1000); // change s to ms
		}
		cm++;
	}
	return cnt;
}

#ifdef CONFIG_MACH_MT6768
extern do_lcm_vdo_lp_write_nolock(struct dsi_cmd_desc *write_table,
	unsigned int count);
#endif

int mtk_mipi_dsi_cmds_extern_tx(struct lcd_kit_dsi_cmd_desc *cmds, int cnt,
	bool lock_state)
{
	struct lcd_kit_dsi_cmd_desc *cm = NULL;
	struct dsi_cmd_desc dsi_cmd;
	int i;

	if (!cmds) {
		LCD_KIT_ERR("cmds is NULL");
		return -EINVAL;
	}
	cm = cmds;
	for (i = 0; i < cnt; i++) {
		lcd_kit_cmds_to_mtk_dsi_cmds(cm, &dsi_cmd);
		if (lock_state) {
			(void)do_lcm_vdo_lp_write(&dsi_cmd, 1);
                } else {
#ifdef CONFIG_MACH_MT6768
			(void)do_lcm_vdo_lp_write_nolock(&dsi_cmd, 1);
#else
			(void)do_lcm_vdo_lp_write(&dsi_cmd, 1);
#endif
		}
		LCD_KIT_ERR("dttype is 0x%x, len is %d, payload is 0x%x\n",
			dsi_cmd.dtype, dsi_cmd.dlen, *(dsi_cmd.payload));
		if (!(cm->wait)) {
			cm++;
			continue;
		}
		if (cm->waittype == WAIT_TYPE_US) {
			udelay(cm->wait);
		} else if (cm->waittype == WAIT_TYPE_MS) {
			if (cm->wait <= 10)
				mdelay(cm->wait);
			else
				msleep(cm->wait);
		} else {
			msleep(cm->wait * 1000); // change s to ms
		}
		cm++;
	}
	return cnt;
}

int lcd_kit_dsi_cmds_tx(void *hld, struct lcd_kit_dsi_panel_cmds *cmds)
{
	int i;

	if (cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is null point!\n");
		return LCD_KIT_FAIL;
	}
	down(&disp_info->lcd_kit_sem);
	for (i = 0; i < cmds->cmd_cnt; i++)
		mtk_mipi_dsi_cmds_tx(&cmds->cmds[i], 1);
	up(&disp_info->lcd_kit_sem);
	return 0;
}

int lcd_kit_parse_read_data(uint8_t *out, uint32_t *in, int start_index, int dlen)
{
	return 0;
}

int lcd_kit_dsi_cmds_rx(void *hld, uint8_t *out, int out_len,
	struct lcd_kit_dsi_panel_cmds *cmds)
{
	int i;
	int ret = 0;
	struct dsi_cmd_desc dsi_cmd;

	if ((cmds == NULL)  || (out == NULL)) {
		LCD_KIT_ERR("out or cmds is null point!\n");
		return LCD_KIT_FAIL;
	}
	down(&disp_info->lcd_kit_sem);
	for (i = 0; i < cmds->cmd_cnt; i++) {
		lcd_kit_cmds_to_mtk_dsi_cmds(&cmds->cmds[i], &dsi_cmd);
		if (lcd_kit_cmd_is_write(cmds)) {
			ret = mtk_mipi_dsi_cmds_tx(&cmds->cmds[i], 1);
		} else {
			lcd_kit_cmds_to_mtk_dsi_cmds(&cmds->cmds[i], &dsi_cmd);
			ret = mipi_dsi_cmds_rx(out, &dsi_cmd, dsi_cmd.dlen);
		}
	}
	up(&disp_info->lcd_kit_sem);
	return ret;
}

int lcd_kit_dsi_cmds_extern_rx(uint8_t *out,
	struct lcd_kit_dsi_panel_cmds *cmds, unsigned int len)
{
	unsigned int i;
	unsigned int j;
	unsigned int k = 0;
	struct dsi_cmd_desc dsi_cmd;
	unsigned char *buffer = NULL;

	if ((cmds == NULL)  || (out == NULL)) {
		LCD_KIT_ERR("out or cmds is null point!\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < cmds->cmd_cnt; i++) {
		if (lcd_kit_cmd_is_write(cmds))
			mtk_mipi_dsi_cmds_extern_tx(&cmds->cmds[i], 1, true);
		lcd_kit_cmds_to_mtk_dsi_read_cmds(&cmds->cmds[i], &dsi_cmd);
		if (dsi_cmd.dlen == 0) {
			LCD_KIT_ERR("cmd len is 0!\n");
			return LCD_KIT_FAIL;
		}
		LCD_KIT_INFO("cmd len is %d!\n", dsi_cmd.dlen);
		buffer = kzalloc(dsi_cmd.dlen, GFP_KERNEL);
		if (buffer == NULL) {
			LCD_KIT_ERR("buffer is NULL!\n");
			return LCD_KIT_FAIL;
		}
		dsi_cmd.payload = buffer;
		do_lcm_vdo_lp_read(&dsi_cmd, 1);
		if (dsi_cmd.dlen == 0) {
			LCD_KIT_ERR("read data len is 0!\n");
			kfree(buffer);
			buffer = NULL;
			return LCD_KIT_FAIL;
		}
		for (j = 0; j < dsi_cmd.dlen; j++) {
			if (k >= len) {
				LCD_KIT_ERR("out buffer is full!\n");
				break;
			}
			out[k] = buffer[j];
			k++;
			LCD_KIT_INFO("j is %d k is %d data1 is 0x%x\n",
				j, k, buffer[j]);
		}
		kfree(buffer);
		buffer = NULL;
	}
	return LCD_KIT_OK;
}

int lcd_kit_dsi_cmds_extern_rx_v1(char *out,
	struct lcd_kit_dsi_panel_cmds *cmds, unsigned int len)
{
	int i;
	int j = 0;
	int dlen;
	int ret;
	char buffer[MTK_READ_MAX_LEN] = {0};
	struct dsi_cmd_desc dsi_cmd;

	if ((cmds == NULL)  || (out == NULL)) {
		LCD_KIT_ERR("out or cmds is null point!\n");
		return LCD_KIT_FAIL;
	}

	for (i = 0; i < cmds->cmd_cnt; i++) {
		if (lcd_kit_cmd_is_write_v1(&cmds->cmds[i])) {
			mtk_mipi_dsi_cmds_extern_tx(&cmds->cmds[i], 1, true);
		} else {
			lcd_kit_cmds_to_mtk_dsi_read_cmds(&cmds->cmds[i], &dsi_cmd);
			dsi_cmd.payload = &buffer[0];

			ret = do_lcm_vdo_lp_read_v1(&dsi_cmd);
			if (ret == LCD_KIT_FAIL) {
				LCD_KIT_ERR("lcd vdo lp read error");
				return LCD_KIT_FAIL;
			}

			for (dlen = 0; dlen < dsi_cmd.dlen; dlen++) {
				if (j >= len) {
					LCD_KIT_ERR("out buffer is full!\n");
					break;
				}
				out[j] = buffer[dlen];
				LCD_KIT_INFO("read value is 0x%x", out[j]);
				j++;
			}
		}
	}

	return LCD_KIT_OK;
}

int lcd_kit_dsi_cmds_extern_tx(struct lcd_kit_dsi_panel_cmds *cmds)
{
	int i;

	if (cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is null point!\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < cmds->cmd_cnt; i++)
		mtk_mipi_dsi_cmds_extern_tx(&cmds->cmds[i], 1, true);
	return 0;

}

int lcd_kit_dsi_cmds_extern_tx_nolock(struct lcd_kit_dsi_panel_cmds *cmds)
{
	int i;

	if (cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is null point!\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < cmds->cmd_cnt; i++)
		mtk_mipi_dsi_cmds_extern_tx(&cmds->cmds[i], 1, false);
	return 0;

}
#endif

static int lcd_kit_buf_trans(const char *inbuf, int inlen, char **outbuf,
	int *outlen)
{
	char *buf = NULL;
	int i;
	int bufsize = inlen;

	if (!inbuf || !outbuf || !outlen) {
		LCD_KIT_ERR("inbuf is null point!\n");
		return LCD_KIT_FAIL;
	}
	/* The property is 4 bytes long per element in cells: <> */
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
	/* For use cells property: <> */
	for (i = 0; i < bufsize; i++)
		buf[i] = inbuf[i * 4 + 3];
	*outbuf = buf;
	*outlen = bufsize;
	return LCD_KIT_OK;
}

static int lcd_kit_gpio_enable(u32 type)
{
	lcd_kit_gpio_tx(type, GPIO_HIGH);
	return LCD_KIT_OK;
}

static int lcd_kit_gpio_disable(u32 type)
{
	lcd_kit_gpio_tx(type, GPIO_LOW);
	return LCD_KIT_OK;
}

static int lcd_kit_regulator_enable(u32 type)
{
	int ret;

	switch (type) {
	case LCD_KIT_VSP:
	case LCD_KIT_VSN:
	case LCD_KIT_BL:
		ret = lcd_kit_charger_ctrl(type, LDO_ENABLE);
		break;
	case LCD_KIT_VCI:
	case LCD_KIT_IOVCC:
		ret = lcd_kit_pmu_ctrl(type, LDO_ENABLE);
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
	case LCD_KIT_VSP:
	case LCD_KIT_VSN:
	case LCD_KIT_BL:
		ret = lcd_kit_charger_ctrl(type, LDO_DISABLE);
		break;
	case LCD_KIT_VCI:
	case LCD_KIT_IOVCC:
		ret = lcd_kit_pmu_ctrl(type, LDO_DISABLE);
		break;
	default:
		LCD_KIT_ERR("regulator type:%d not support\n", type);
		break;
	}
	return ret;
}

static int lcd_kit_lock(void *hld)
{
	return LCD_KIT_FAIL;
}

static void lcd_kit_release(void *hld)
{
}

void *lcd_kit_get_pdata_hld(void)
{
	return 0;
}

struct lcd_kit_adapt_ops adapt_ops = {
	.mipi_tx = lcd_kit_dsi_cmds_tx,
	.mipi_rx = lcd_kit_dsi_cmds_rx,
	.gpio_enable = lcd_kit_gpio_enable,
	.gpio_disable = lcd_kit_gpio_disable,
	.regulator_enable = lcd_kit_regulator_enable,
	.regulator_disable = lcd_kit_regulator_disable,
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
