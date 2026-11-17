/*
 * desktop_bridge.c
 *
 * operate function for desktop product
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
#include <securec.h>
#include <linux/init.h>
#include "edp_bridge.h"

#define valid_mode_key(C, H, V) (((int64_t)(C) << 40) + ((int64_t)(H) << 16) + (int64_t)V)

struct desktop_common_data {
	unsigned int hw_hpd_gpio;
	unsigned int hw_hpd_irq;
	unsigned int patch;

	int chip_reset;
	bool is_valid_irq;
};

struct desktop_v3_private_data {
	struct desktop_common_data* com_data;
};

struct desktop_v4_private_data {
	struct desktop_common_data* com_data;

	int dsi_irq_reset;
	int master_board_5v0_slot;
	int display_board_vga_hdmi_5v0;
	int master_board_3v3_disp;
	int display_board_3v3_slot;
	int master_board_1v2;
	int display_board_1v2;
	int master_board_1v8_dsi_vccio;
	int display_board_1v1;
};

static struct desktop_common_data desktop_com_data;
static struct desktop_v4_private_data desktop_v4_priv_data = {
	.com_data = &desktop_com_data
};

static struct desktop_v3_private_data desktop_v3_priv_data = {
	.com_data = &desktop_com_data
};

static struct gpio_desc desktop_v4_gpio_init_cmds[] = {
	/*init master_board_5v0_slot gpio, set high to enable bridge*/
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, "master_board_5v0_slot", &desktop_v4_priv_data.master_board_5v0_slot, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "master_board_5v0_slot", &desktop_v4_priv_data.master_board_5v0_slot, 1},
	/*init display_board_vga_hdmi_5v0 gpio, set high to enable bridge*/
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, "display_board_vga_hdmi_5v0", &desktop_v4_priv_data.display_board_vga_hdmi_5v0, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "display_board_vga_hdmi_5v0", &desktop_v4_priv_data.display_board_vga_hdmi_5v0, 1},
	/*init master_board_3v3_disp gpio, set high to enable bridge*/
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, "master_board_3v3_disp", &desktop_v4_priv_data.master_board_3v3_disp, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "master_board_3v3_disp", &desktop_v4_priv_data.master_board_3v3_disp, 1},
	/*init display_board_3v3_slot gpio, set high to enable bridge*/
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, "display_board_3v3_slot", &desktop_v4_priv_data.display_board_3v3_slot, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "display_board_3v3_slot", &desktop_v4_priv_data.display_board_3v3_slot, 1},
	/*init master_board_1v2 gpio, set high to enable bridge*/
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, "master_board_1v2", &desktop_v4_priv_data.master_board_1v2, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "master_board_1v2", &desktop_v4_priv_data.master_board_1v2, 1},
	/*init display_board_1v2 gpio, set high to enable bridge*/
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, "display_board_1v2", &desktop_v4_priv_data.display_board_1v2, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "display_board_1v2", &desktop_v4_priv_data.display_board_1v2, 1},
	/*init master_board_1v8_dsi_vccio gpio, set high to enable bridge*/
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, "master_board_1v8_dsi_vccio", &desktop_v4_priv_data.master_board_1v8_dsi_vccio, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "master_board_1v8_dsi_vccio", &desktop_v4_priv_data.master_board_1v8_dsi_vccio, 1},
	/*init display_board_1v1 gpio, set high to enable bridge*/
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, "display_board_1v1", &desktop_v4_priv_data.display_board_1v1, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "display_board_1v1", &desktop_v4_priv_data.display_board_1v1, 1},
	/*init enable gpio, set high to enable bridge*/
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, "chip_reset", &desktop_com_data.chip_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "chip_reset", &desktop_com_data.chip_reset, 1},
	/*init enable gpio, set high to enable bridge*/
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, "dsi_irq_reset", &desktop_v4_priv_data.dsi_irq_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "dsi_irq_reset", &desktop_v4_priv_data.dsi_irq_reset, 1},
};

static struct gpio_desc desktop_chip_reset_disable[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "chip_reset", &desktop_com_data.chip_reset, 0},
};

static struct gpio_desc desktop_chip_reset_enable[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "chip_reset", &desktop_com_data.chip_reset, 1},
};

static struct gpio_desc bridge_v4_dsi_irq_reset_enable[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "dsi_irq_reset", &desktop_v4_priv_data.dsi_irq_reset, 1}
};

static struct gpio_desc desktop_v4_power_on_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "master_board_5v0_slot", &desktop_v4_priv_data.master_board_5v0_slot, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "display_board_vga_hdmi_5v0", &desktop_v4_priv_data.display_board_vga_hdmi_5v0, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "master_board_3v3_disp", &desktop_v4_priv_data.master_board_3v3_disp, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "display_board_3v3_slot", &desktop_v4_priv_data.display_board_3v3_slot, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "master_board_1v2", &desktop_v4_priv_data.master_board_1v2, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "display_board_1v2", &desktop_v4_priv_data.display_board_1v2, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "master_board_1v8_dsi_vccio", &desktop_v4_priv_data.master_board_1v8_dsi_vccio, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "display_board_1v1", &desktop_v4_priv_data.display_board_1v1, 1}
};

static struct gpio_desc desktop_v4_power_off_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "display_board_1v1", &desktop_v4_priv_data.display_board_1v1, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "master_board_1v8_dsi_vccio", &desktop_v4_priv_data.master_board_1v8_dsi_vccio, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "master_board_1v2", &desktop_v4_priv_data.master_board_1v2, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "display_board_1v2", &desktop_v4_priv_data.display_board_1v2, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "display_board_3v3_slot", &desktop_v4_priv_data.display_board_3v3_slot, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "master_board_3v3_disp", &desktop_v4_priv_data.master_board_3v3_disp, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "display_board_vga_hdmi_5v0", &desktop_v4_priv_data.display_board_vga_hdmi_5v0, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "master_board_5v0_slot", &desktop_v4_priv_data.master_board_5v0_slot, 0}
};

static const struct mipi_info mipi2edp_panel_info_init_1920_1080 = {
	.dsi_bit_clk = 448,
	.hsa = 33,
	.hbp = 111,
	.hline_time = 1652,
	.vsa = 5,
	.vbp = 36,
	.vfp = 4,
	.dpi_hsize = 1442,
	.vactive_line = 1080
};

static const struct mipi_info mipi2edp_panel_info_init_1920_1200_60 = {
	.dsi_bit_clk = 477,
	.hsa = 24,
	.hbp = 60,
	.hline_time = 1562,
	.vsa = 6,
	.vbp = 26,
	.vfp = 3,
	.dpi_hsize = 1442,
	.vactive_line = 1200
};

static const struct mipi_info mipi2edp_panel_info_init_2560_1080_60 = {
	.dsi_bit_clk = 654,
	.hsa = 33,
	.hbp = 111,
	.hline_time = 2252,
	.vsa = 5,
	.vbp = 11,
	.vfp = 4,
	.dpi_hsize = 1922,
	.vactive_line = 1080
};

static const struct mipi_info mipi2edp_panel_info_init_1680_1050_60 = {
	.dsi_bit_clk = 443,
	.hsa = 132,
	.hbp = 210,
	.hline_time = 1682,
	.vsa = 6,
	.vbp = 30,
	.vfp = 3,
	.dpi_hsize = 1262,
	.vactive_line = 1050
};

static const struct mipi_info mipi2edp_panel_info_init_1600_900_60 = {
	.dsi_bit_clk = 328,
	.hsa = 60,
	.hbp = 72,
	.hline_time = 1352,
	.vsa = 3,
	.vbp = 96,
	.vfp = 1,
	.dpi_hsize = 1202,
	.vactive_line = 900
};

static const struct mipi_info mipi2edp_panel_info_init_1280_1024_75 = {
	.dsi_bit_clk = 407,
	.hsa = 108,
	.hbp = 186,
	.hline_time = 1268,
	.vsa = 3,
	.vbp = 38,
	.vfp = 1,
	.dpi_hsize = 962,
	.vactive_line = 1024
};

static const struct mipi_info mipi2edp_panel_info_init_1280_1024_60 = {
	.dsi_bit_clk = 328,
	.hsa = 84,
	.hbp = 186,
	.hline_time = 1268,
	.vsa = 3,
	.vbp = 38,
	.vfp = 1,
	.dpi_hsize = 962,
	.vactive_line = 1024
};

static const struct mipi_info mipi2edp_panel_info_init_1152_864_75 = {
	.dsi_bit_clk = 328,
	.hsa = 96,
	.hbp = 192,
	.hline_time = 1202,
	.vsa = 3,
	.vbp = 32,
	.vfp = 1,
	.dpi_hsize = 866,
	.vactive_line = 864
};

static const struct mipi_info mipi2edp_panel_info_init_1024_768_75 = {
	.dsi_bit_clk = 239,
	.hsa = 72,
	.hbp = 132,
	.hline_time = 986,
	.vsa = 3,
	.vbp = 28,
	.vfp = 1,
	.dpi_hsize = 770,
	.vactive_line = 768
};

static const struct mipi_info mipi2edp_panel_info_init_1024_768_60 = {
	.dsi_bit_clk = 197,
	.hsa = 102,
	.hbp = 120,
	.hline_time = 1010,
	.vsa = 6,
	.vbp = 29,
	.vfp = 3,
	.dpi_hsize = 770,
	.vactive_line = 768
};

static const struct config_table mipi2edp_config_table[] = {
	{screen_resolution_key(1920, 1080, 60), &mipi2edp_panel_info_init_1920_1080},
	{screen_resolution_key(2560, 1080, 60), &mipi2edp_panel_info_init_2560_1080_60},
	{screen_resolution_key(1920, 1200, 60), &mipi2edp_panel_info_init_1920_1200_60},
	{screen_resolution_key(1680, 1050, 60), &mipi2edp_panel_info_init_1680_1050_60},
	{screen_resolution_key(1600, 900, 60), &mipi2edp_panel_info_init_1600_900_60},
	{screen_resolution_key(1280, 1024, 75), &mipi2edp_panel_info_init_1280_1024_75},
	{screen_resolution_key(1280, 1024, 60), &mipi2edp_panel_info_init_1280_1024_60},
	{screen_resolution_key(1152, 864, 75), &mipi2edp_panel_info_init_1152_864_75},
	{screen_resolution_key(1024, 768, 75), &mipi2edp_panel_info_init_1024_768_75},
	{screen_resolution_key(1024, 768, 60), &mipi2edp_panel_info_init_1024_768_60}
};

static const struct drm_display_mode default_mode_vga = {
	.type = DRM_MODE_TYPE_DEFAULT,
	.clock = 148500,
	.hdisplay = 1920,
	.hsync_start = 1920 + 88,
	.hsync_end = 1920 + 88 + 148,
	.htotal = 1920 + 88 + 148 + 44,
	.vdisplay = 1080,
	.vsync_start = 1080 + 4,
	.vsync_end = 1080 + 4 + 36,
	.vtotal = 1080 + 4 + 36 + 5,
	.vrefresh = 60,
};

static const int64_t valid_mode_table[] = {
	valid_mode_key(148500, 1920, 1080),
	valid_mode_key(198000, 2560, 1080),
	valid_mode_key(138500, 1920, 1080),
	valid_mode_key(154000, 1920, 1200),
	valid_mode_key(146250, 1680, 1050),
	valid_mode_key(108000, 1280, 1024),
	valid_mode_key(108000, 1600, 900),
	valid_mode_key(135000, 1280, 1024),
	valid_mode_key(108000, 1152, 864),
	valid_mode_key(78750, 1024, 768),
	valid_mode_key(65000, 1024, 768)
};

static int desktop_bridge_init_common_data(struct device_node *dn, struct desktop_common_data *com_data)
{
	int ret = 0;

	ret = read_dts_value_u32(dn, "hw_hpd_gpio", &com_data->hw_hpd_gpio);
	if (ret < 0)
		return ret;

	ret = read_dts_value_u32(dn, "chip_reset", &com_data->chip_reset);
	if (ret < 0)
		return ret;

	com_data->patch = 1;

	return 0;
}

static int desktop_panel_diff_info_init(struct hisi_panel_info *pinfo, const struct mipi_info *pinfo_init_data) {
	pinfo->mipi.dsi_bit_clk = pinfo_init_data->dsi_bit_clk;
	pinfo->mipi.dsi_bit_clk_upt = pinfo_init_data->dsi_bit_clk;
	pinfo->mipi.hsa = pinfo_init_data->hsa;
	pinfo->mipi.hbp = pinfo_init_data->hbp;
	pinfo->mipi.hline_time = pinfo_init_data->hline_time;
	pinfo->mipi.vsa = pinfo_init_data->vsa;
	pinfo->mipi.vbp = pinfo_init_data->vbp;
	pinfo->mipi.vfp = pinfo_init_data->vfp;
	pinfo->mipi.dpi_hsize = pinfo_init_data->dpi_hsize;
	pinfo->mipi.vactive_line = pinfo_init_data->vactive_line;
	HISI_DRM_INFO("dsi_bit_clk=%d, hsa=%d, hbp=%d, hline_time=%d, vsa=%d, vbp=%d, vfp=%d, dpi_hsize=%d, vactive_line=%d",
		pinfo->mipi.dsi_bit_clk, pinfo->mipi.hsa, pinfo->mipi.hbp, pinfo->mipi.hline_time, pinfo->mipi.vsa,
		pinfo->mipi.vbp, pinfo->mipi.vfp, pinfo->mipi.dpi_hsize, pinfo->mipi.vactive_line);
	HISI_DRM_INFO("type=%d, dsi_bit_clk_upt=%d, non_continue_en=%d, lane_nums=%d, color_mode=%d, vc=%d, max_tx_esc_clk=%d,"
		" burst_mode=%d, phy_mode=%d, dsi_version=%d, dsi_timing_support=%d, ifbc_type=%d, pxl_clk_rate_div=%d",
		pinfo->type, pinfo->mipi.dsi_bit_clk_upt, pinfo->mipi.non_continue_en, pinfo->mipi.lane_nums,
		pinfo->mipi.color_mode,	pinfo->mipi.vc, pinfo->mipi.max_tx_esc_clk, pinfo->mipi.burst_mode, pinfo->mipi.phy_mode,
		pinfo->mipi.dsi_version, pinfo->mipi.dsi_timing_support, pinfo->ifbc_type, pinfo->pxl_clk_rate_div);
	return 0;
}

static int desktop_panel_info_config(struct mipi2edp *pdata)
{
	int64_t key;
	int count;
	int len;
	HISI_DRM_INFO("+");
	len = ARRAY_SIZE(mipi2edp_config_table);
	key = screen_resolution_key(pdata->output_hdisplay, pdata->output_vdisplay, pdata->output_vrefresh);
	for (count = 0; count < len; count++) {
		if (key == mipi2edp_config_table[count].key) {
			desktop_panel_diff_info_init(&pdata->panel_info, (mipi2edp_config_table[count].mipi_config_info));
			return 0;
		}
	}
	HISI_DRM_INFO("output_hdisplay=%d, output_vdisplay=%d, output_vrefresh=%d", pdata->output_hdisplay,
		pdata->output_vdisplay, pdata->output_vrefresh);
	HISI_DRM_INFO("-");
	return -EINVAL;
}

static void desktop_panel_info_init(struct hisi_panel_info *pinfo, int dsi_bit_clk)
{
	mipi2edp_panel_common_info_init(pinfo);

	HISI_DRM_INFO("set dsi_bit_clk:%d", dsi_bit_clk);

	pinfo->mipi.dsi_bit_clk = dsi_bit_clk;
	pinfo->mipi.dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk;
	pinfo->mipi.burst_mode = DSI_NON_BURST_SYNC_PULSES;

	pinfo->mipi.hsa = 33;
	pinfo->mipi.hbp = 111;
	pinfo->mipi.hline_time = 1652;
	pinfo->mipi.vsa = 5;
	pinfo->mipi.vbp = 36;
	pinfo->mipi.vfp = 4;
	pinfo->mipi.dpi_hsize = 1442;
	pinfo->mipi.vactive_line = 1080;
}

static irqreturn_t desktop_hpd_gpio_irq_thread_impl(int irq, struct desktop_common_data *com_data, struct mipi2edp *pdata)
{
	struct drm_device *drm_dev = pdata->connector.dev;
	int i;

	if (!com_data->is_valid_irq) {
		HISI_DRM_INFO("[mipi2edp]irq is not valid = %d", com_data->is_valid_irq);
		return 0;
	}
	com_data->is_valid_irq = false;

	if (irq == com_data->hw_hpd_irq) {
		if (gpio_get_value(com_data->hw_hpd_gpio)) {
			HISI_DRM_INFO("enable mipi2edp bridge! +\n");
			if (pdata->bridge_chip_info.set_clk)
				pdata->bridge_chip_info.set_clk(pdata->mipi2edp_clk, 1);

			mdelay(15);
			gpio_cmds_tx(desktop_chip_reset_enable, ARRAY_SIZE(desktop_chip_reset_enable));

			if (pdata->bridge_chip_info.pre_enable)
				pdata->bridge_chip_info.pre_enable(pdata);
			if (pdata->bridge_chip_info.enable)
				pdata->bridge_chip_info.enable(pdata);
			HISI_DRM_INFO("enable mipi2edp bridge! -\n");

		} else {
			HISI_DRM_INFO("hw_hpd_gpio is low! \n");
			HISI_DRM_INFO("cable out ! clear edid array.\n");
			for (i = 0; i < EDID_LEN; i++)
				pdata->edid_array[i] = 0;
		}
		drm_kms_helper_hotplug_event(drm_dev);
		HISI_DRM_INFO("handle irq %d!\n", irq);
	} else {
		HISI_DRM_ERR("invalid irq %d!\n", irq);
	}

	return IRQ_HANDLED;
}

static irqreturn_t desktop_hpd_gpio_irq_thread(int irq, void *data)
{
	struct mipi2edp *pdata = (struct mipi2edp *)data;

	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return 0;
	}

	mdelay(5);
	return desktop_hpd_gpio_irq_thread_impl(irq, &desktop_com_data, pdata);
}

static irqreturn_t desktop_hpd_irq_handler(int irq, void *data)
{
	HISI_DRM_INFO("[mipi] hpd gpio irq\n");
	desktop_com_data.is_valid_irq = !desktop_com_data.is_valid_irq;

	return IRQ_WAKE_THREAD;
}

static int desktop_hpd_init_impl(struct desktop_common_data *com_data, struct mipi2edp *pdata)
{
	int ret = 0;

	if (!gpio_is_valid(com_data->hw_hpd_gpio)) {
		HISI_DRM_ERR("gpio of hpd is not valid, check DTS!");
		return -EINVAL;
	}

	ret = devm_gpio_request(pdata->dev, com_data->hw_hpd_gpio, "hw_hpd_gpio");
	if (ret < 0) {
		HISI_DRM_ERR("Fail request hw_hpd_gpio:%d", com_data->hw_hpd_gpio);
		return ret;
	}
	HISI_DRM_INFO("success request hw_hpd_gpio:%d", com_data->hw_hpd_gpio);

	ret = gpio_direction_input(com_data->hw_hpd_gpio);
	if (ret < 0) {
		HISI_DRM_ERR("Failed to set hw_hpd_gpio directoin!");
		return ret;
	}
	HISI_DRM_INFO("success to set hw_hpd_gpio directoin!");

	com_data->hw_hpd_irq = gpio_to_irq(com_data->hw_hpd_gpio);
	if (com_data->hw_hpd_irq < 0) {
		HISI_DRM_ERR("Failed to get hw_hpd_gpio irq!");
		return ret;
	}
	com_data->is_valid_irq = false;
	HISI_DRM_INFO("success to get hw_hpd_gpio irq!");

	ret = devm_request_threaded_irq(pdata->dev,
	com_data->hw_hpd_irq, desktop_hpd_irq_handler, desktop_hpd_gpio_irq_thread,
	IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	"edp_bridge", pdata);
	if (ret) {
		HISI_DRM_ERR("Failed to request hw_hpd_gpio handler!");
		return ret;
	}
	HISI_DRM_INFO("success to request hw_hpd_gpio handler!");

	return ret;
}

static int desktop_hpd_init(struct mipi2edp *pdata)
{
	return desktop_hpd_init_impl(&desktop_com_data, pdata);
}

static int desktop_bridge_remove(struct i2c_client *client)
{
	return 0;
}

static int desktop_bridge_attach(struct mipi2edp *pdata)
{
	return desktop_hpd_init(pdata);
}

static void desktop_bridge_pre_enable(struct mipi2edp *pdata)
{
	if (pdata->bridge_chip_info.set_clk)
		pdata->bridge_chip_info.set_clk(pdata->mipi2edp_clk, 1);

	mdelay(15);

	gpio_cmds_tx(desktop_chip_reset_enable, ARRAY_SIZE(desktop_chip_reset_enable));
}

static void desktop_bridge_enable(struct mipi2edp *pdata)
{
	UNUSED(pdata);
	return;
}

static void desktop_bridge_disable(struct mipi2edp *pdata)
{
	gpio_cmds_tx(desktop_chip_reset_disable, ARRAY_SIZE(desktop_chip_reset_disable));

	if (pdata->bridge_chip_info.set_clk)
		pdata->bridge_chip_info.set_clk(pdata->mipi2edp_clk, 0);
}

static void desktop_bridge_post_disable(struct mipi2edp *pdata)
{
	gpio_cmds_tx(desktop_chip_reset_disable, ARRAY_SIZE(desktop_chip_reset_disable));
}

static void desktop_bridge_mode_set(struct mipi2edp *pdata, struct drm_display_mode *mode, struct drm_display_mode *adjusted_mode)
{
	pdata->output_hdisplay = adjusted_mode->hdisplay;
	pdata->output_vdisplay = adjusted_mode->vdisplay;
	pdata->output_vrefresh = adjusted_mode->vrefresh;

	HISI_DRM_INFO("cur_mode hdisplay = %d", adjusted_mode->hdisplay);
	HISI_DRM_INFO("cur_mode vdisplay = %d", adjusted_mode->vdisplay);
	HISI_DRM_INFO("cur_mode vrefresh = %d", adjusted_mode->vrefresh);
	desktop_panel_info_config(pdata);
}

static enum drm_connector_status desktop_drm_connector_detect(struct mipi2edp *pdata, bool force)
{
	enum drm_connector_status status = connector_status_disconnected;

	if (desktop_com_data.patch) {
		desktop_com_data.patch = 0;
		status = connector_status_connected;
	} else {
		status = gpio_get_value(desktop_com_data.hw_hpd_gpio) ? connector_status_connected : connector_status_disconnected;
	}

	return status;
}

static int desktop_drm_connector_get_mode(struct mipi2edp *pdata)
{
	int num_modes = 0;
	struct drm_display_mode *mode = NULL;
	int retries = 3;

	while ((check_edid(pdata->edid_array, EDID_LEN) != 0) && --retries) {
		if (pdata->bridge_product_info.bridge_ops->get_edid)
			pdata->bridge_product_info.bridge_ops->get_edid(pdata);
	}

	if (check_edid(pdata->edid_array, EDID_LEN) == 0) {
		num_modes += drm_add_edid_modes(&pdata->connector, (struct edid *)pdata->edid_array);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
		drm_mode_connector_update_edid_property(&pdata->connector, (struct edid *)pdata->edid_array);
		drm_edid_to_eld(&pdata->connector, (struct edid *)pdata->edid_array);
#else
		drm_connector_update_edid_property(&pdata->connector, (struct edid *)pdata->edid_array);
#endif
		HISI_DRM_INFO("mipi2edp_connector_get_modes-use-edid:num_modes = %d", num_modes);
		return num_modes;
	}
	mode = drm_mode_duplicate(pdata->connector.dev, &default_mode_vga);
	if (mode == NULL) {
		HISI_DRM_ERR("failed to add mode %ux%ux@%u!",
			default_mode_vga.hdisplay, default_mode_vga.vdisplay,
			default_mode_vga.vrefresh);
		return -ENOMEM;
	}

	mode->type = DRM_MODE_TYPE_PREFERRED;
	drm_mode_set_name(mode);

	drm_mode_probed_add(&pdata->connector, mode);

	return 1;
}

static enum drm_mode_status desktop_drm_connector_mode_valid(struct drm_connector *connector, struct drm_display_mode *mode)
{
	int count;
	int64_t key;
	int len;

	key = valid_mode_key(mode->clock, mode->hdisplay, mode->vdisplay);
	len = ARRAY_SIZE(valid_mode_table);
	for (count = 0; count < len; count++) {
		if (key == valid_mode_table[count]) {
			HISI_DRM_INFO("hdisplay=%d, vdisplay=%d, vrefresh=%d", mode->hdisplay, mode->vdisplay, mode->clock);
			return MODE_OK;
		}
	}
	HISI_DRM_INFO("hdisplay_notpass=%d, vdisplay_notpass=%d, vrefresh_notpass=%d", mode->hdisplay, mode->vdisplay, mode->clock);
	return MODE_ERROR;
}

/************************************desktop v4 private impl******************************************/
static int desktop_v4_init_private_data_impl(struct mipi2edp *pdata, struct i2c_client *client,
	struct desktop_v4_private_data* priv_data)
{
	int ret = 0;
	struct device_node *dn = client->dev.of_node;

	ret = desktop_bridge_init_common_data(dn, priv_data->com_data);
	if (ret)
		return ret;

	ret = read_dts_value_u32(dn, "dsi_irq_reset", &priv_data->dsi_irq_reset);
	if (ret < 0)
		return ret;

	ret = read_dts_value_u32(dn, "master_board_5v0_slot", &priv_data->master_board_5v0_slot);
	if (ret < 0)
		return ret;

	ret = read_dts_value_u32(dn, "display_board_vga_hdmi_5v0", &priv_data->display_board_vga_hdmi_5v0);
	if (ret < 0)
		return ret;

	ret = read_dts_value_u32(dn, "master_board_3v3_disp", &priv_data->master_board_3v3_disp);
	if (ret < 0)
		return ret;

	ret = read_dts_value_u32(dn, "display_board_3v3_slot", &priv_data->display_board_3v3_slot);
	if (ret < 0)
		return ret;

	ret = read_dts_value_u32(dn, "master_board_1v2", &priv_data->master_board_1v2);
	if (ret < 0)
		return ret;

	ret = read_dts_value_u32(dn, "display_board_1v2", &priv_data->display_board_1v2);
	if (ret < 0)
		return ret;

	ret = read_dts_value_u32(dn, "master_board_1v8_dsi_vccio", &priv_data->master_board_1v8_dsi_vccio);
	if (ret < 0)
		return ret;

	ret = read_dts_value_u32(dn, "display_board_1v1", &priv_data->display_board_1v1);
	if (ret < 0)
		return ret;

	return ret;
}

static int desktop_v4_init_private_data(struct mipi2edp *pdata, struct i2c_client *client)
{
	int ret = 0;

	ret = desktop_v4_init_private_data_impl(pdata, client, &desktop_v4_priv_data);
	if (ret)
		return ret;

	pdata->private_data = &desktop_v4_priv_data;

	return ret;
}

static int desktop_v4_bridge_probe(struct mipi2edp *pdata, struct i2c_client *client)
{
	int ret = 0;
	struct hisi_panel_info *pinfo = &(pdata->panel_info);

	gpio_cmds_tx(desktop_v4_gpio_init_cmds, ARRAY_SIZE(desktop_v4_gpio_init_cmds));

	ret = set_bl_props(pdata, pdata->dev);
	if (ret)
		return ret;

	aux_register(pdata);

	desktop_panel_info_init(pinfo, pdata->dsi_bit_clk);

	return 0;
}

static int desktop_v4_bridge_shutdown(struct mipi2edp *pdata)
{
	gpio_cmds_tx(desktop_chip_reset_disable, ARRAY_SIZE(desktop_chip_reset_disable));

	if (pdata->bridge_chip_info.set_clk)
		pdata->bridge_chip_info.set_clk(pdata->mipi2edp_clk, 0);

	gpio_cmds_tx(desktop_v4_power_off_cmds, ARRAY_SIZE(desktop_v4_power_off_cmds));

	return 0;
}

static void desktop_v4_bridge_pre_enable(struct mipi2edp *pdata)
{
	desktop_bridge_pre_enable(pdata);

	gpio_cmds_tx(bridge_v4_dsi_irq_reset_enable, ARRAY_SIZE(bridge_v4_dsi_irq_reset_enable));
}

static int desktop_v4_bridge_system_suspend(struct mipi2edp *pdata)
{
	gpio_cmds_tx(desktop_chip_reset_disable, ARRAY_SIZE(desktop_chip_reset_disable));

	if (pdata->bridge_chip_info.set_clk)
		pdata->bridge_chip_info.set_clk(pdata->mipi2edp_clk, 0);

	gpio_cmds_tx(desktop_v4_power_off_cmds, ARRAY_SIZE(desktop_v4_power_off_cmds));

	return 0;
}

static int desktop_v4_bridge_system_resume(struct mipi2edp *pdata)
{
	gpio_cmds_tx(desktop_v4_power_on_cmds, ARRAY_SIZE(desktop_v4_power_on_cmds));

	if (pdata->bridge_chip_info.set_clk)
		pdata->bridge_chip_info.set_clk(pdata->mipi2edp_clk, 1);

	mdelay(15);

	gpio_cmds_tx(desktop_chip_reset_enable, ARRAY_SIZE(desktop_chip_reset_enable));

	gpio_cmds_tx(bridge_v4_dsi_irq_reset_enable, ARRAY_SIZE(bridge_v4_dsi_irq_reset_enable));

	mdelay(100);

	return 0;
}

static struct bridge_product_funcs desktop_v4_bridge_funcs = {
	.bridge_init_private_data = desktop_v4_init_private_data,
	.bridge_probe = desktop_v4_bridge_probe,
	.bridge_remove = desktop_bridge_remove,
	.bridge_shutdown = desktop_v4_bridge_shutdown,
	.bridge_attach = desktop_bridge_attach,
	.bridge_pre_enable = desktop_v4_bridge_pre_enable,
	.bridge_enable = desktop_bridge_enable,
	.bridge_disable = desktop_bridge_disable,
	.bridge_post_disable = desktop_bridge_post_disable,
	.bridge_mode_set = desktop_bridge_mode_set,
	.get_edid = mipi2edp_get_edid
};

static struct bridge_connector_help_funcs desktop_bridge_connector_help_funcs = {
	.drm_connector_detect = desktop_drm_connector_detect,
	.drm_connector_get_mode = desktop_drm_connector_get_mode,
	.drm_connector_mode_valid = desktop_drm_connector_mode_valid
};

static struct bridge_pm_func desktop_v4_bridge_pm_func = {
	.pm_runtime_suspend = NULL,
	.pm_runtime_resume = NULL,
	.pm_system_suspend = desktop_v4_bridge_system_suspend,
	.pm_system_resume = desktop_v4_bridge_system_resume
};

static struct mipi2edp_bridge_product_info desktop_v4_product_info = {
	.product_type = PRODUCT_TYPE_DESKTOP_V4,
	.product_series = PRODUCT_SERIES_DESKTOP,
	.desc = "desktop product v4",
	.connector_type = DRM_MODE_CONNECTOR_VGA,
	.bridge_ops = &desktop_v4_bridge_funcs,
	.bridge_connector_ops = &desktop_bridge_connector_help_funcs,
	.bridge_pm_ops = &desktop_v4_bridge_pm_func
};

/************************************desktop v3 private impl******************************************/
static int desktop_v3_init_private_data_impl(struct mipi2edp *pdata, struct i2c_client *client,
	struct desktop_v3_private_data* priv_data)
{
	return desktop_bridge_init_common_data(client->dev.of_node, priv_data->com_data);
}

static int desktop_v3_init_private_data(struct mipi2edp *pdata, struct i2c_client *client)
{
	int ret = 0;

	ret = desktop_v3_init_private_data_impl(pdata, client, &desktop_v3_priv_data);
	if (ret)
		return ret;

	pdata->private_data = &desktop_v3_priv_data;

	return ret;
}

static int desktop_v3_bridge_probe(struct mipi2edp *pdata, struct i2c_client *client)
{
	int ret = 0;
	struct hisi_panel_info *pinfo = &(pdata->panel_info);

	gpio_cmds_tx(desktop_chip_reset_enable, ARRAY_SIZE(desktop_chip_reset_enable));

	ret = set_bl_props(pdata, pdata->dev);
	if (ret)
		return ret;

	aux_register(pdata);

	desktop_panel_info_init(pinfo, pdata->dsi_bit_clk);

	return 0;
}

static struct bridge_product_funcs desktop_v3_bridge_funcs = {
	.bridge_init_private_data = desktop_v3_init_private_data,
	.bridge_probe = desktop_v3_bridge_probe,
	.bridge_remove = desktop_bridge_remove,
	.bridge_shutdown = NULL,
	.bridge_attach = desktop_bridge_attach,
	.bridge_pre_enable = desktop_bridge_pre_enable,
	.bridge_enable = desktop_bridge_enable,
	.bridge_disable = desktop_bridge_disable,
	.bridge_post_disable = desktop_bridge_post_disable,
	.bridge_mode_set = desktop_bridge_mode_set,
	.get_edid = mipi2edp_get_edid
};

static struct bridge_pm_func desktop_v3_bridge_pm_func = {
	.pm_runtime_suspend = NULL,
	.pm_runtime_resume = NULL,
	.pm_system_suspend = NULL,
	.pm_system_resume = NULL
};

static struct mipi2edp_bridge_product_info desktop_v3_product_info = {
	.product_type = PRODUCT_TYPE_DESKTOP_V3,
	.product_series = PRODUCT_SERIES_DESKTOP,
	.desc = "desktop product v3",
	.connector_type = DRM_MODE_CONNECTOR_VGA,
	.bridge_ops = &desktop_v3_bridge_funcs,
	.bridge_connector_ops = &desktop_bridge_connector_help_funcs,
	.bridge_pm_ops = &desktop_v3_bridge_pm_func
};

static int __init desktop_bridge_register(void)
{
	int ret = 0;

	printk(KERN_DEBUG "desktop_bridge_register +");

	ret = mipi2edp_bridge_product_info_register(&desktop_v4_product_info);
	if (ret)
		return ret;

	ret = mipi2edp_bridge_product_info_register(&desktop_v3_product_info);
	if (ret)
		return ret;

	printk(KERN_DEBUG "desktop_bridge_register -");

	return ret;
}

fs_initcall(desktop_bridge_register);
