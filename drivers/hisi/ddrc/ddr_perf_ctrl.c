/*
 * ddr_perf_ctrl.c
 *
 * perf ctrl features of ddr module
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
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/hisi/hisi_drmdriver.h>
#include <linux/of.h>
#include <asm/io.h>
#include <global_ddr_map.h>
#include <linux/hisi/ddr_perf_ctrl.h>

#define PERF_CTRL_DDR_FLUX_MAX_CNT UINT_MAX
#define PERF_CTRL_DDR_MAX_CH 4

enum DDR_PERFDATA_CLIENT {
	DDR_PERFDATA_PERFCTRL = 0,
	DDR_PERFDATA_KARMA,
	DDR_PERFDATA_CLIENT_MAX,
};

/* attention: there must be same structure definition with atf hisi_ddr.c */
struct ddr_perfdata {
	unsigned int channel_nr;
	unsigned int io_width;
	unsigned int rd_flux[PERF_CTRL_DDR_MAX_CH];
	unsigned int wr_flux[PERF_CTRL_DDR_MAX_CH];
};

static phys_addr_t g_ddr_perfdata_phy_addr;
static struct ddr_perfdata __iomem *g_ddr_perfdata_virt_addr;
static struct ddr_perfdata g_last_data[DDR_PERFDATA_CLIENT_MAX];

static unsigned int get_ddrflux_data(unsigned int curr, unsigned int *last)
{
	unsigned int delta;

	if (curr >= *last)
		delta = curr - *last;
	else
		delta = PERF_CTRL_DDR_FLUX_MAX_CNT - *last + curr;

	*last = curr;

	return delta;
}

static int get_ddrc_flux_all_ch(struct ddr_flux *ddr_flux_str,
				enum DDR_PERFDATA_CLIENT client)
{
	int ddr_max_ch, ch;

	if (ddr_flux_str == NULL) {
		pr_err("%s: invalid ddr_flux_str is NULL\n", __func__);
		return -EFAULT;
	}

	if (client >= DDR_PERFDATA_CLIENT_MAX) {
		pr_err("%s: invalid client %d\n", __func__, client);
		return -EFAULT;
	}

	if (g_ddr_perfdata_virt_addr == NULL)
		return -EFAULT;

	(void)atfd_hisi_service_access_register_smc(ACCESS_REGISTER_FN_MAIN_ID,
						    g_ddr_perfdata_phy_addr,
						    sizeof(struct ddr_perfdata),
						    ACCESS_REGISTER_FN_SUB_ID_DDR_PERF_CTRL);

	ddr_max_ch = g_ddr_perfdata_virt_addr->channel_nr;

	if (ddr_max_ch < 1 || ddr_max_ch > PERF_CTRL_DDR_MAX_CH) {
		pr_err("%s: invalid ddr_max_ch %d\n", __func__, ddr_max_ch);
		return -EFAULT;
	}

	for (ch = 0; ch < ddr_max_ch; ch++) {
		ddr_flux_str->rd_flux +=
			get_ddrflux_data(g_ddr_perfdata_virt_addr->rd_flux[ch],
					 &g_last_data[client].rd_flux[ch]);
		ddr_flux_str->wr_flux +=
			get_ddrflux_data(g_ddr_perfdata_virt_addr->wr_flux[ch],
					 &g_last_data[client].wr_flux[ch]);
	}

	return 0;
}

int perf_ctrl_get_ddr_flux(void __user *uarg)
{
	struct ddr_flux ddr_flux_val = {0};

	if (uarg == NULL)
		return -EINVAL;

	if (get_ddrc_flux_all_ch(&ddr_flux_val, DDR_PERFDATA_PERFCTRL) == -EFAULT)
		return -EFAULT;

	if (copy_to_user(uarg, &ddr_flux_val, sizeof(ddr_flux_val))) {
		pr_err("%s: copy_to_user fail\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static int ddr_perf_ctrl_init(void)
{
	int ret;
	struct device_node *np = NULL;
	uint32_t data[2] = {0};

	np = of_find_compatible_node(NULL, NULL, "hisi,hisi-ddr-perfdata");
	if (np == NULL) {
		pr_err("%s: no compatible node found!\n", __func__);
		return -ENODEV;
	}

	ret = of_property_read_u32_array(np, "hisi,ddr-perdata", &data[0], 2UL);
	if (ret != 0) {
		of_node_put(np);
		pr_err("%s: find ddr-perfdata node failed!\n", __func__);
		return ret;
	}

	of_node_put(np);
	g_ddr_perfdata_phy_addr =
		(phys_addr_t)(HISI_SUB_RESERVED_BL31_SHARE_MEM_PHYMEM_BASE + data[0]);
	g_ddr_perfdata_virt_addr =
		(struct ddr_perfdata *)ioremap(g_ddr_perfdata_phy_addr, (u64)data[1]); /*lint !e446*/
	if (g_ddr_perfdata_virt_addr == NULL) {
		pr_err("%s: allocate memory for ddr-perfdata failed!\n", __func__);
		return -ENOMEM;
	}
	pr_info("%s: perfdata phys:%llx virt:%llx data:%x %x\n",
		__func__, g_ddr_perfdata_phy_addr,
		(unsigned long long)(uintptr_t)g_ddr_perfdata_virt_addr,
		data[0], data[1]);

	return ret;
}

static void ddr_perf_ctrl_exit(void)
{
	if (g_ddr_perfdata_virt_addr != NULL) {
		iounmap(g_ddr_perfdata_virt_addr);
		g_ddr_perfdata_virt_addr = NULL;
	}
}

module_init(ddr_perf_ctrl_init);
module_exit(ddr_perf_ctrl_exit);
