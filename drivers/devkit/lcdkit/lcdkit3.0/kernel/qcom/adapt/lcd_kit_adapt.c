/*
 * lcd_kit_adapt.c
 *
 * lcdkit adapt function for lcd driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "lcd_kit_drm_panel.h"
#include <drm/drm_mipi_dsi.h>
#include "dsi_ctrl.h"

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

static int lcd_kit_cmds_to_qcom_cmds_tx(
	struct lcd_kit_dsi_cmd_desc *lcd_kit_cmds,
	struct dsi_cmd_desc *cmd, int link_state)
{
	if (lcd_kit_cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (cmd == NULL) {
		LCD_KIT_ERR("cmd is NULL\n");
		return LCD_KIT_FAIL;
	}
	memset(cmd, 0x0, sizeof(*cmd));
	cmd->msg.type = lcd_kit_cmds->dtype;
	cmd->last_command = (lcd_kit_cmds->last == 1);
	cmd->msg.channel = lcd_kit_cmds->vc;
	/* 2: mipi LP; 0: mipi HS */
	if (link_state == 0)
		cmd->msg.flags = MIPI_DSI_MSG_USE_LPM;
	if (cmd->last_command)
		cmd->msg.flags |= MIPI_DSI_MSG_LASTCOMMAND;
	cmd->msg.ctrl = 0;
	cmd->post_wait_ms = lcd_kit_cmds->wait;
	cmd->msg.wait_ms = lcd_kit_cmds->wait;
	cmd->msg.tx_len = lcd_kit_cmds->dlen;
	cmd->msg.tx_buf = lcd_kit_cmds->payload;

	if (cmd->msg.tx_len > MAX_CMD_PAYLOAD_SIZE) {
		LCD_KIT_ERR("Incorrect payload length tx_len %zu\n",
		       cmd->msg.tx_len);
		return -EINVAL;
	}

	return LCD_KIT_OK;
}

static int lcd_kit_cmds_to_qcom_cmds_rx(
	struct lcd_kit_dsi_cmd_desc *lcd_kit_cmds,
	struct dsi_cmd_desc *cmd, int link_state,
	unsigned char *buf)
{
	if (lcd_kit_cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (cmd == NULL) {
		LCD_KIT_ERR("cmd is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (buf == NULL) {
		LCD_KIT_ERR("buf is NULL\n");
		return LCD_KIT_FAIL;
	}
	memset(cmd, 0x0, sizeof(*cmd));
	cmd->msg.type = lcd_kit_cmds->dtype;
	cmd->last_command = (lcd_kit_cmds->last == 1);
	cmd->msg.channel = lcd_kit_cmds->vc;
	/* 2: mipi LP; 0: mipi HS */
	if (link_state == 0)
		cmd->msg.flags = MIPI_DSI_MSG_USE_LPM;
	if (cmd->last_command)
		cmd->msg.flags |= MIPI_DSI_MSG_LASTCOMMAND;
	cmd->msg.flags |= DSI_CTRL_CMD_READ;
	cmd->msg.ctrl = 0;
	cmd->post_wait_ms = lcd_kit_cmds->wait;
	cmd->msg.wait_ms = lcd_kit_cmds->wait;
	cmd->msg.tx_len = lcd_kit_cmds->dlen;
	cmd->msg.tx_buf = lcd_kit_cmds->payload;
	cmd->msg.rx_buf = buf;
	cmd->msg.rx_len = lcd_kit_cmds->dlen;
	if (cmd->msg.type == DTYPE_GEN_READ2)
		/* long read, 2 parameter */
		cmd->msg.tx_len= 2;
	else
		cmd->msg.tx_len = 1;

	return LCD_KIT_OK;
}

int qcom_mipi_dsi_cmd_tx(struct dsi_ctrl *dsi_ctrl,
	struct lcd_kit_dsi_cmd_desc *cmds, int link_state )
{
	struct dsi_cmd_desc cmd;
	u32 flags = (DSI_CTRL_CMD_FETCH_MEMORY |
		DSI_CTRL_CMD_CUSTOM_DMA_SCHED);
	int ret;

	if (dsi_ctrl == NULL || cmds == NULL) {
		LCD_KIT_ERR("dsi_ctrl is NULL");
		return LCD_KIT_FAIL;
	}
	memset(&cmd, 0, sizeof(struct dsi_cmd_desc));
	ret = lcd_kit_cmds_to_qcom_cmds_tx(cmds, &cmd, link_state);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("exchange dsi mipi fail\n");
		return ret;
	}
	if (cmd.last_command)
		flags |= DSI_CTRL_CMD_LAST_COMMAND;

	ret = dsi_ctrl_cmd_transfer(dsi_ctrl, &cmd.msg, &flags);
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("tx cmd transfer fail ret = %d\n", ret);
	return ret;
}

int qcom_mipi_dsi_cmd_rx(struct dsi_ctrl *dsi_ctrl, unsigned char *buffer,
	unsigned int *len, struct lcd_kit_dsi_cmd_desc *cmds, int link_state )
{
	struct dsi_cmd_desc cmd;
	u32 flags = (DSI_CTRL_CMD_FETCH_MEMORY |
		DSI_CTRL_CMD_CUSTOM_DMA_SCHED | DSI_CTRL_CMD_READ);
	int ret;

	if ((dsi_ctrl == NULL)  || (cmds == NULL) || (buffer == NULL) || (len == NULL)) {
		LCD_KIT_ERR("buf or cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	memset(&cmd, 0, sizeof(struct dsi_cmd_desc));
	ret = lcd_kit_cmds_to_qcom_cmds_rx(cmds, &cmd, link_state, buffer);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("exchange dsi mipi fail\n");
		return ret;
	}
	if (cmd.msg.rx_len == 0) {
		LCD_KIT_ERR("cmd len is 0\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("cmd len is %d!\n", cmd.msg.rx_len);

	if (cmd.last_command)
		flags |= DSI_CTRL_CMD_LAST_COMMAND;

	ret = dsi_ctrl_cmd_transfer(dsi_ctrl, &cmd.msg, &flags);
	if (ret <= LCD_KIT_OK) {
		LCD_KIT_ERR("rx cmd transfer fail ret = %d\n", ret);
		return LCD_KIT_FAIL;
	}
	*len = cmd.msg.rx_len;
	return LCD_KIT_OK;
}

int qcom_mipi_dsi_cmds_transfer(struct dsi_ctrl *dsi_ctrl, unsigned char *out,
	unsigned int len, struct lcd_kit_dsi_panel_cmds *cmds)
{
	unsigned int i;
	unsigned int j;
	unsigned int k = 0;
	int ret;
	unsigned int rx_len = 0;
	unsigned char buffer[MAX_CMD_PAYLOAD_SIZE] = {0};

	if ((dsi_ctrl == NULL)  || (cmds == NULL) || (out == NULL) || (len == 0)) {
		LCD_KIT_ERR("out or cmds is NULL\n");
		return LCD_KIT_FAIL;
	}

	if (!dsi_ctrl_validate_host_state(dsi_ctrl)) {
		LCD_KIT_ERR("Invalid host state\n");
		return LCD_KIT_FAIL;
	}

	for (i = 0; i < cmds->cmd_cnt; i++) {
		if (lcd_kit_cmd_is_write(&cmds->cmds[i])) {
			ret = qcom_mipi_dsi_cmd_tx(dsi_ctrl, &cmds->cmds[i],
				cmds->link_state);
			if (ret != LCD_KIT_OK) {
				LCD_KIT_ERR("the %d mipi cmd tx fail, ret = %d\n",
					i, ret);
				return ret;
			}
		} else {
			ret = qcom_mipi_dsi_cmd_rx(dsi_ctrl, &buffer[0], &rx_len,
				&cmds->cmds[i], cmds->link_state);
			if (ret != LCD_KIT_OK) {
				LCD_KIT_ERR("the %d mipi cmd rx fail, ret = %d\n",
					i, ret);
				return ret;
			}
			for (j = 0; j < rx_len; j++) {
				out[k] = buffer[j];
				LCD_KIT_INFO("read %d cmd val is 0x%x\n", k, out[k]);
				k++;
				if (k == len) {
					LCD_KIT_INFO("buffer len %d is full\n", len);
					break;
				}
			}
			memset(buffer, 0x0, MAX_CMD_PAYLOAD_SIZE);
		}
	}
	return LCD_KIT_OK;
}

int lcd_kit_dsi_cmds_rx(void *hld, unsigned char *out,
	unsigned int len, struct lcd_kit_dsi_panel_cmds *cmds)
{
	int ret = LCD_KIT_OK;
	struct dsi_display *display = (struct dsi_display *)hld;
	struct dsi_panel *panel = NULL;
	struct dsi_display_ctrl *m_ctrl = &display->ctrl[display->cmd_master_idx];

	if ((display == NULL)  || (cmds == NULL) ||(out == NULL) || (len == 0)) {
		LCD_KIT_ERR("out or cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	panel = display->panel;
	dsi_panel_acquire_panel_lock(panel);
	if (!panel->panel_initialized) {
		LCD_KIT_ERR("Panel not initialized\n");
		ret = LCD_KIT_FAIL;
		goto release_panel_lock;
	}

	/* Prevent another ESD check,when ESD recovery is underway */
	if (atomic_read(&panel->esd_recovery_pending)) {
		LCD_KIT_ERR("esd_recovery_pending %d\n",
			panel->esd_recovery_pending);
		ret = LCD_KIT_FAIL;
		goto release_panel_lock;
	}

	ret = dsi_display_clk_ctrl(display->dsi_clk_handle, DSI_ALL_CLKS, DSI_CLK_ON);
	if (ret) {
		LCD_KIT_ERR("failed to enable DSI clocks, ret = %d\n", ret);
		goto release_panel_lock;
	}
	if (display->tx_cmd_buf == NULL) {
		ret = dsi_host_alloc_cmd_tx_buffer(display);
		if (ret) {
			LCD_KIT_ERR("failed to allocate cmd tx buffer memory\n");
			goto release_clk_ctrl;
		}
	}
	ret= dsi_display_cmd_engine_enable(display);
	if (ret) {
		LCD_KIT_ERR("cmd engine enable failed\n");
		goto release_clk_ctrl;
	}
	ret = qcom_mipi_dsi_cmds_transfer(m_ctrl->ctrl, out, len, cmds);
	if (ret  != LCD_KIT_OK)
		LCD_KIT_ERR("the mipi cmd transfer fail, ret = %d\n", ret);
	dsi_display_cmd_engine_disable(display);
release_clk_ctrl:
	dsi_display_clk_ctrl(display->dsi_clk_handle, DSI_ALL_CLKS, DSI_CLK_OFF);
release_panel_lock:
	dsi_panel_release_panel_lock(panel);
	return ret;
}

int lcd_kit_dsi_cmds_tx(void *hld, struct lcd_kit_dsi_panel_cmds *cmds)
{
	int i;
	int ret = LCD_KIT_OK;
	struct dsi_display *display = (struct dsi_display *)hld;
	struct dsi_panel *panel = NULL;
	struct dsi_display_ctrl *m_ctrl = &display->ctrl[display->cmd_master_idx];

	if ((display == NULL)  || (cmds == NULL)) {
		LCD_KIT_ERR("out or cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	panel = display->panel;
	dsi_panel_acquire_panel_lock(panel);
	if (!panel->panel_initialized) {
		LCD_KIT_ERR("Panel not initialized\n");
		ret = LCD_KIT_FAIL;
		goto release_panel_lock;
	}
	ret = dsi_display_clk_ctrl(display->dsi_clk_handle, DSI_ALL_CLKS, DSI_CLK_ON);
	if (ret) {
		LCD_KIT_ERR("failed to enable DSI clocks, ret = %d\n", ret);
		goto release_panel_lock;
	}
	if (display->tx_cmd_buf == NULL) {
		ret = dsi_host_alloc_cmd_tx_buffer(display);
		if (ret) {
			LCD_KIT_ERR("failed to allocate cmd tx buffer memory\n");
			goto release_clk_ctrl;
		}
	}
	ret= dsi_display_cmd_engine_enable(display);
	if (ret) {
		LCD_KIT_ERR("cmd engine enable failed\n");
		goto release_clk_ctrl;
	}
	for (i = 0; i < cmds->cmd_cnt; i++)
		ret = qcom_mipi_dsi_cmd_tx(m_ctrl->ctrl, &cmds->cmds[i],
			cmds->link_state);
	dsi_display_cmd_engine_disable(display);
release_clk_ctrl:
	dsi_display_clk_ctrl(display->dsi_clk_handle, DSI_ALL_CLKS, DSI_CLK_OFF);
release_panel_lock:
	dsi_panel_release_panel_lock(panel);
	return ret;
}

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

static int lcd_kit_gpio_request(u32 type)
{
	lcd_kit_gpio_tx(type, GPIO_REQ);
	return LCD_KIT_OK;
}

static int lcd_kit_regulate_ctrl(struct lcd_kit_array_data *pvreg_info,
	struct lcd_kit_regulate *pvreg, bool enable)
{
	int ret;
	int num_of_v = 0;

	if (pvreg_info == NULL || pvreg == NULL) {
		LCD_KIT_ERR("pvreg_info or pvreg is null\n");
		return LCD_KIT_FAIL;
	}
	if (enable) {
		ret = regulator_set_load(pvreg->vreg,
						pvreg_info->buf[POWER_ENABLE_LOAD]);
		if (ret < 0) {
			LCD_KIT_ERR("Setting optimum mode failed for %s\n",
				pvreg->name);
			goto error;
		}
		num_of_v = regulator_count_voltages(pvreg->vreg);
		if (num_of_v > 0) {
			ret = regulator_set_voltage(pvreg->vreg,
				pvreg_info->buf[POWER_MIN_VOL],
				pvreg_info->buf[POWER_MAX_VOL]);
			if (ret) {
				LCD_KIT_ERR("Set voltage %s fail, ret=%d\n",
					pvreg->name, ret);
				goto error_disable_opt_mode;
			}
		}
		ret = regulator_enable(pvreg->vreg);
		if (ret) {
			DSI_ERR("enable failed for %s, ret=%d\n",
				pvreg->name, ret);
			goto error_disable_voltage;
		}
	} else {
		(void)regulator_set_load(pvreg->vreg,
				pvreg_info->buf[POWER_DISABLE_LOAD]);
		(void)regulator_disable(pvreg->vreg);
	}

	return 0;
error_disable_opt_mode:
	(void)regulator_set_load(pvreg->vreg,
		pvreg_info->buf[POWER_DISABLE_LOAD]);

error_disable_voltage:
	if (num_of_v > 0)
		(void)regulator_set_voltage(pvreg->vreg,
			0, pvreg_info->buf[POWER_MAX_VOL]);
error:
	return ret;
}


static int lcd_kit_regulator_enable(u32 type)
{
	int ret = LCD_KIT_OK;

	switch (type) {
	case LCD_KIT_VCI:
		ret = lcd_kit_regulate_ctrl(&power_hdl->lcd_vci, &common_info->vci_vreg, true);
		break;
	case LCD_KIT_IOVCC:
		ret = lcd_kit_regulate_ctrl(&power_hdl->lcd_iovcc, &common_info->iovcc_vreg, true);
		break;
	case LCD_KIT_VSP:
		ret = lcd_kit_regulate_ctrl(&power_hdl->lcd_vsp, &common_info->vsp_vreg, true);
		break;
	case LCD_KIT_VSN:
		ret = lcd_kit_regulate_ctrl(&power_hdl->lcd_vsn, &common_info->vsn_vreg, true);
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
		ret = lcd_kit_regulate_ctrl(&power_hdl->lcd_vci, &common_info->vci_vreg, false);
		break;
	case LCD_KIT_IOVCC:
		ret = lcd_kit_regulate_ctrl(&power_hdl->lcd_iovcc, &common_info->iovcc_vreg, false);
		break;
	case LCD_KIT_VSP:
		ret = lcd_kit_regulate_ctrl(&power_hdl->lcd_vsp, &common_info->vsp_vreg, false);
		break;
	case LCD_KIT_VSN:
		ret = lcd_kit_regulate_ctrl(&power_hdl->lcd_vsn, &common_info->vsn_vreg, false);
		break;
	default:
		ret = LCD_KIT_FAIL;
		LCD_KIT_ERR("regulator type:%d not support\n", type);
		break;
	}
	return ret;
}

struct lcd_kit_adapt_ops adapt_ops = {
	.mipi_tx = lcd_kit_dsi_cmds_tx,
	.mipi_rx = lcd_kit_dsi_cmds_rx,
	.gpio_enable = lcd_kit_gpio_enable,
	.gpio_disable = lcd_kit_gpio_disable,
	.gpio_request = lcd_kit_gpio_request,
	.regulator_enable = lcd_kit_regulator_enable,
	.regulator_disable = lcd_kit_regulator_disable,
	.buf_trans = lcd_kit_buf_trans,
};

int lcd_kit_adapt_init(void)
{
	int ret;

	ret = lcd_kit_adapt_register(&adapt_ops);
	return ret;
}
