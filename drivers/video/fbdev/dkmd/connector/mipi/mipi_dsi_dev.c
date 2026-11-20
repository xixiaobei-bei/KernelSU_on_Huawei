
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
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/genalloc.h>
#include <soc_crgperiph_interface.h>
#include <dpu/soc_dpu_define.h>

#include "dkmd_utils.h"
#include "mipi_dsi_drv.h"
#include "panel_drv.h"

/*******************************************************************************
 * MIPI DPHY GPIO for FPGA
 */
#define GPIO_MIPI_DPHY_PG_SEL_A_NAME "pg_sel_a"
#define GPIO_MIPI_DPHY_PG_SEL_B_NAME "pg_sel_b"
#define GPIO_MIPI_DPHY_TX_RX_A_NAME "tx_rx_a"
#define GPIO_MIPI_DPHY_TX_RX_B_NAME "tx_rx_b"

#define GPIO_PG_SEL_A (56)
#define GPIO_TX_RX_A (58)
#define GPIO_PG_SEL_B (37)
#define GPIO_TX_RX_B (39)

static uint32_t gpio_pg_sel_a = GPIO_PG_SEL_A;
static uint32_t gpio_tx_rx_a = GPIO_TX_RX_A;
static uint32_t gpio_pg_sel_b = GPIO_PG_SEL_B;
static uint32_t gpio_tx_rx_b = GPIO_TX_RX_B;

static struct gpio_desc mipi_dphy_gpio_request_cmds[] = {
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_A_NAME, &gpio_pg_sel_a, 0 },
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_B_NAME, &gpio_pg_sel_b, 0 },
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_A_NAME, &gpio_tx_rx_a, 0 },
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_B_NAME, &gpio_tx_rx_b, 0 },
};

static struct gpio_desc mipi_dphy_gpio_free_cmds[] = {
	{ DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_A_NAME, &gpio_pg_sel_a, 0 },
	{ DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_B_NAME, &gpio_pg_sel_b, 0 },
	{ DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_A_NAME, &gpio_tx_rx_a, 0 },
	{ DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_B_NAME, &gpio_tx_rx_b, 0 },
};

static struct gpio_desc mipi_dphy_gpio_normal_cmds[] = {
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_A_NAME, &gpio_pg_sel_a, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_A_NAME, &gpio_tx_rx_a, 1 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_B_NAME, &gpio_pg_sel_b, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_B_NAME, &gpio_tx_rx_b, 1 },
};

static struct gpio_desc mipi_dphy_gpio_lowpower_cmds[] = {
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_A_NAME, &gpio_pg_sel_a, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_A_NAME, &gpio_tx_rx_a, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_B_NAME, &gpio_pg_sel_b, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_B_NAME, &gpio_tx_rx_b, 0 },
};

static void mipi_dsi_dphy_fastboot_fpga(struct dkmd_connector_info *pinfo)
{
	if (pinfo->fpga_flag)
		gpio_cmds_tx(mipi_dphy_gpio_request_cmds, ARRAY_SIZE(mipi_dphy_gpio_request_cmds));
}

void mipi_dsi_dphy_on_fpga(struct dkmd_connector_info *pinfo)
{
	if (pinfo->fpga_flag) {
		gpio_cmds_tx(mipi_dphy_gpio_request_cmds, ARRAY_SIZE(mipi_dphy_gpio_request_cmds));
		gpio_cmds_tx(mipi_dphy_gpio_normal_cmds, ARRAY_SIZE(mipi_dphy_gpio_normal_cmds));
	}
}

static void mipi_dsi_disreset_ctrl(struct platform_device *pdev, bool disreset)
{
	uint32_t value = 0;
	struct mipi_dsi_drv_private *priv = to_mipi_dsi_private(pdev);

	if (is_mipi_dsi0_panel(&(priv->base)))
		value |= 0x10000000;

	if (is_mipi_dsi1_panel(&(priv->base)))
		value |= 0x20000000;

	pr_err("set CRGPERIPH value = %#x!\n", value);
	/* dis-reset ip_reset_dis_dsi0, ip_reset_dis_dsi1 */
	if (disreset)
		outp32(SOC_CRGPERIPH_PERRSTDIS3_ADDR(priv->peri_crg_base), value);
	else /* reset dsi */
		outp32(SOC_CRGPERIPH_PERRSTEN3_ADDR(priv->peri_crg_base), value);
}

static int mipi_dsi_clk_enable(struct platform_device *pdev)
{
	int ret = 0;
	struct mipi_dsi_drv_private *priv = to_mipi_dsi_private(pdev);

	if (!IS_ERR(priv->clk_gate_txdphy_ref)) {
		pr_err("set clk_gate_txdphy_ref enable!\n");
		ret = clk_prepare_enable(priv->clk_gate_txdphy_ref);
		if (ret)
			pr_err("id: %d, clk_gate_txdphy_ref enable failed, error=%d!\n", priv->id, ret);
	}

	if (!IS_ERR(priv->clk_gate_txdphy_cfg)) {
		pr_err("set clk_gate_txdphy_cfg enable!\n");
		ret = clk_prepare_enable(priv->clk_gate_txdphy_cfg);
		if (ret)
			pr_err("id: %d, clk_gate_txdphy_cfg enable failed, error=%d!\n", priv->id, ret);
	}

	if (!IS_ERR(priv->pclk_gate_dsi)) {
		pr_err("set pclk_gate_dsi enable!\n");
		ret = clk_prepare_enable(priv->pclk_gate_dsi);
		if (ret)
			pr_err("id: %d, pclk_gate_dsi enable failed, error=%d!\n", priv->id, ret);
	}

	if (priv->next_dsi_dev) {
		pr_err("set next dsi dev clk enable!\n");
		ret = mipi_dsi_clk_enable(priv->next_dsi_dev);
	}

	return ret;
}

static void mipi_dsi_on_sub1(struct platform_device *pdev)
{
	char __iomem *mipi_dsi_base = NULL;
	struct mipi_dsi_drv_private *priv = NULL;

	priv = to_mipi_dsi_private(pdev);
	mipi_dsi_base = priv->dsi_base;

	/* mipi init */
	mipi_init(pdev, mipi_dsi_base);

	/* switch to cmd mode */
	set_reg(DPU_DSI_MODE_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 0);
	/* cmd mode: low power mode */
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0x7f, 7, 8);
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0xf, 4, 16);
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 24);
	/* disable generate High Speed clock */
	set_reg(DPU_DSI_LP_CLK_CTRL_ADDR(mipi_dsi_base), 0x0, 1, 0);

	if (priv->next_dsi_dev)
		mipi_dsi_on_sub1(priv->next_dsi_dev);
}

static void mipi_dsi_on_sub2(struct platform_device *pdev)
{
	char __iomem *mipi_dsi_base = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	struct mipi_dsi_drv_private *priv = NULL;

	priv = to_mipi_dsi_private(pdev);
	pinfo = &priv->base;
	mipi_dsi_base = priv->dsi_base;

	/* switch to video mode */
	if (is_mipi_video_panel(pinfo))
		set_reg(DPU_DSI_MODE_CTRL_ADDR(mipi_dsi_base), 0x0, 1, 0);

	/* cmd mode: high speed mode */
	if (is_mipi_cmd_panel(pinfo)) {
		set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0x0, 7, 8);
		set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0x0, 4, 16);
		set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0x0, 1, 24);
	}

	/* enable EOTP TX */
	if (priv->mipi.phy_mode == DPHY_MODE)
		set_reg(DPU_DSI_PERIP_CHAR_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 0);

	/* enable generate High Speed clock, non continue */
	if (priv->mipi.non_continue_en)
		set_reg(DPU_DSI_LP_CLK_CTRL_ADDR(mipi_dsi_base), 0x3, 2, 0);
	else
		set_reg(DPU_DSI_LP_CLK_CTRL_ADDR(mipi_dsi_base), 0x1, 2, 0);

	if (priv->next_dsi_dev)
		mipi_dsi_on_sub2(priv->next_dsi_dev);
}

static int mipi_dsi_on(struct platform_device *pdev)
{
	struct mipi_dsi_drv_private *priv = NULL;
	struct panel_drv_private *panel_priv = NULL;
	struct dkmd_connector_data *pdata = NULL;

	if (!pdev) {
		pr_err("pdev is NULL!\n");
		return -EINVAL;
	}

	priv = to_mipi_dsi_private(pdev);
	if (!priv) {
		pr_err("priv is NULL!\n");
		return -EINVAL;
	}

	pdata = dev_get_platdata(&pdev->dev);
	if (!pdata) {
		pr_err("pdata is NULL!\n");
		return -EINVAL;
	}

	panel_priv = to_panel_drv_private(pdata->next);
	if (!panel_priv) {
		pr_err("panel_priv is NULL!\n");
		return -EINVAL;
	}

	mipi_dsi_dphy_on_fpga(&priv->base);

	panel_priv->panel_initial_step = LCD_INIT_POWER_ON;
	pipeline_next_on(pdev);

	mipi_dsi_disreset_ctrl(pdev, true);

	mipi_dsi_clk_enable(pdev);

	mipi_dsi_on_sub1(pdev);

	pipeline_next_on(pdev);

	mipi_dsi_on_sub2(pdev);

	/* mipi hs video/command mode */
	pipeline_next_on(pdev);

	pr_err("primary dsi-%d -\n", priv->id);

	return 0;
}

void mipi_dsi_dphy_off_fpga(struct dkmd_connector_info *pinfo)
{
	if (pinfo->fpga_flag) {
		gpio_cmds_tx(mipi_dphy_gpio_lowpower_cmds, ARRAY_SIZE(mipi_dphy_gpio_lowpower_cmds));
		gpio_cmds_tx(mipi_dphy_gpio_free_cmds, ARRAY_SIZE(mipi_dphy_gpio_free_cmds));
	}
}

static void mipi_dsi_clk_disable(struct platform_device *pdev)
{
	struct mipi_dsi_drv_private *priv = to_mipi_dsi_private(pdev);

	if (!IS_ERR(priv->pclk_gate_dsi))
		clk_disable_unprepare(priv->pclk_gate_dsi);

	if (!IS_ERR(priv->clk_gate_txdphy_cfg))
		clk_disable_unprepare(priv->clk_gate_txdphy_cfg);

	if (!IS_ERR(priv->clk_gate_txdphy_ref))
		clk_disable_unprepare(priv->clk_gate_txdphy_ref);

	if (priv->next_dsi_dev)
		mipi_dsi_clk_disable(priv->next_dsi_dev);
}

void mipi_dsi_off_sub(struct platform_device *pdev)
{
	struct mipi_dsi_drv_private *priv = to_mipi_dsi_private(pdev);
	char __iomem *mipi_dsi_base = priv->dsi_base;

	/* switch to cmd mode */
	set_reg(DPU_DSI_MODE_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 0);
	/* cmd mode: low power mode */
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0x7f, 7, 8);
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0xf, 4, 16);
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 24);

	/* disable generate High Speed clock */
	set_reg(DPU_DSI_LP_CLK_CTRL_ADDR(mipi_dsi_base), 0x0, 1, 0);
	udelay(10);  /* 10us */

	/* shutdown d_phy */
	set_reg(DPU_DSI_CDPHY_RST_CTRL_ADDR(mipi_dsi_base), 0x0, 3, 0);

	if (priv->next_dsi_dev)
		mipi_dsi_off_sub(priv->next_dsi_dev);
}

static void disable_mipi_ldi(struct dkmd_connector_info *pinfo, char __iomem *mipi_dsi_base)
{
	if (is_dual_mipi_panel(pinfo))
		set_reg(DPU_DSI_LDI_CTRL_ADDR(mipi_dsi_base), 0x0, 1, 5);
	else
		set_reg(DPU_DSI_LDI_CTRL_ADDR(mipi_dsi_base), 0x0, 1, 0);
}

static int mipi_dsi_off(struct platform_device *pdev)
{
	struct mipi_dsi_drv_private *priv = NULL;
	struct panel_drv_private *panel_priv = NULL;
	struct dkmd_connector_data *pdata = NULL;

	if (!pdev) {
		pr_err("pdev is NULL!\n");
		return -EINVAL;
	}

	priv = to_mipi_dsi_private(pdev);
	if (!priv) {
		pr_err("priv is NULL!\n");
		return -EINVAL;
	}

	pdata = dev_get_platdata(&pdev->dev);
	if (!pdata) {
		pr_err("pdata is NULL!\n");
		return -EINVAL;
	}

	panel_priv = to_panel_drv_private(pdata->next);
	if (!panel_priv) {
		pr_err("panel_priv is NULL!\n");
		return -EINVAL;
	}

	panel_priv->panel_uninitial_step = LCD_UNINIT_MIPI_HS_SEND_SEQUENCE;
	pipeline_next_off(pdev);

	/* add MIPI LP mode here if necessary MIPI LP mode end */
	pipeline_next_off(pdev);

	// TODO: whether need ctrl pipe dsi1 ?
	disable_mipi_ldi(&priv->base, priv->dsi_base);

	/* Here need to enter ulps when panel off bypass ddic power down */
	mipi_dsi_off_sub(pdev);

	mipi_dsi_clk_disable(pdev);

	mipi_dsi_dphy_off_fpga(&priv->base);

	mipi_dsi_disreset_ctrl(pdev, false);

	pipeline_next_off(pdev);

	pr_info("dsi-%d, -\n", priv->id);

	return 0;
}

static int mipi_dsi_set_bit_clk_update(struct platform_device *pdev, void *value)
{
	struct mipi_dsi_drv_private *priv = NULL;

	if (!pdev) {
		pr_err("pdev is NULL!\n");
		return -EINVAL;
	}

	priv = to_mipi_dsi_private(pdev);
	if (!priv) {
		pr_err("priv is NULL!\n");
		return -EINVAL;
	}

	mipi_dsi_dphy_fastboot_fpga(&priv->base);

	return 0;
}

struct dsi_ops_handle_data dsi_ops_table[] = {
	{ "set_bitclk_update", mipi_dsi_set_bit_clk_update },
};

static int mipi_dsi_ops_handle(struct platform_device *pdev, char *ops_cmd, void *value)
{
	int i = 0;
	struct dsi_ops_handle_data *ops_handle = NULL;
	struct dsi_dev_private_data *pdata = NULL;

	pdata = to_dsi_dev_private_data(pdev);
	for (i = 0; i < ARRAY_SIZE(dsi_ops_table); i++) {
		ops_handle = &(dsi_ops_table[i]);
		if (!strcmp(ops_cmd, ops_handle->ops_cmd) && ops_handle->handle_func) {
			return ops_handle->handle_func(pdev, value);
		}
	}
	return pipeline_next_ops_handle(pdev, ops_cmd, value);
}

/**
 * @brief Register link mipi dsi device
 *  1. multiple call for multiple device (such as scene0, scene1 etc.)
 *  2. single device dual mipi need link ext dsi dev
 *
 * @param pdev panel device
 * @return int return 0: success
 */
int dsi_device_register(struct platform_device *pdev)
{
	struct panel_drv_private *panel_priv = NULL;
	struct mipi_dsi_drv_private *dsi_priv = NULL;
	struct mipi_dsi_drv_private *ext_dsi_priv = NULL;
	struct dsi_dev_private_data *pdata = NULL;

	if (!pdev) {
		pr_err("pdev is NULL!\n");
		return -EINVAL;
	}

	panel_priv = to_panel_drv_private(pdev);
	if (!panel_priv) {
		pr_err("panel_priv is NULL!\n");
		return -EINVAL;
	}

	pr_info("panel register dsi-%d device!\n", panel_priv->dsi_idx);
	/* get dsi device header */
	dsi_priv = get_dsi_private_data_by_idx(panel_priv->dsi_idx);
	if (!dsi_priv) {
		pr_err("dsi_priv is NULL!\n");
		return -EINVAL;
	}

	if (!dsi_priv->pdev) {
		pr_err("dsi_priv pdev is NULL!\n");
		return -EINVAL;
	}

	pdata = (struct dsi_dev_private_data *)devm_kzalloc(&dsi_priv->pdev->dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		pr_err("pdata is NULL!\n");
		return -EINVAL;
	}
	/* Link next device */
	pdata->base.on_func = mipi_dsi_on;
	pdata->base.off_func = mipi_dsi_off;
	pdata->base.ops_handle_func = mipi_dsi_ops_handle;
	pdata->base.next = pdev;
	pdata->panel_priv = panel_priv;

	if (platform_device_add_data(dsi_priv->pdev,
		&(pdata->base), sizeof(pdata->base)) != 0) {
		pr_err("add dsi device data failed!\n");
		return -EINVAL;
	}

	dsi_priv->fpga_flag = panel_priv->base.fpga_flag;
	dsi_priv->base = panel_priv->base;
	dsi_priv->mipi = panel_priv->mipi;
	panel_priv->connector_dsi_base = dsi_priv->dsi_base;

	pr_info("panel_priv->connector_dsi_base:%#x!\n", panel_priv->connector_dsi_base);

	if (is_dual_mipi_panel(&panel_priv->base)) {
		pr_info("this dual mipi, but not multiple device!\n");
		ext_dsi_priv = get_dsi_private_data_by_idx(panel_priv->ext_dsi_idx);
		if (!ext_dsi_priv) {
			pr_err("ext_dsi_priv is NULL!\n");
			return -EINVAL;
		}
		dsi_priv->next_dsi_dev = ext_dsi_priv->pdev;
		ext_dsi_priv->base = panel_priv->base;
		panel_priv->ext_connector_dsi_base = ext_dsi_priv->dsi_base;
	}

	comp_mgr_register_device(dsi_priv->pdev);
	pr_info("dsi register driver pdev %p!\n", pdev);

	return 0;
}
EXPORT_SYMBOL(dsi_device_register);

int dsi_device_unregister(struct platform_device *pdev)
{
	struct panel_drv_private *panel_priv = NULL;
	struct mipi_dsi_drv_private *dsi_priv = NULL;

	if (!pdev) {
		pr_err("pdev is NULL!\n");
		return -EINVAL;
	}

	panel_priv = to_panel_drv_private(pdev);
	if (!panel_priv) {
		pr_err("panel_priv is NULL!\n");
		return -EINVAL;
	}

	pr_info("panel unregister dsi-%d device!\n", panel_priv->dsi_idx);
	/* get dsi device header */
	dsi_priv = get_dsi_private_data_by_idx(panel_priv->dsi_idx);
	if (!dsi_priv) {
		pr_err("dsi_priv is NULL!\n");
		return -EINVAL;
	}

	if (!dsi_priv->pdev) {
		pr_err("dsi_priv pdev is NULL!\n");
		return -EINVAL;
	}
	comp_mgr_unregister_device(dsi_priv->pdev);

	return 0;
}
EXPORT_SYMBOL(dsi_device_unregister);

MODULE_LICENSE("GPL");