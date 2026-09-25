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

static struct rdr_register_module_result g_current_info;
static uint32_t g_num_reg_regions;

static uint32_t g_num_peri_reg;
static uint32_t g_num_pctrl_reg;
static uint32_t g_num_media1_reg;

static struct dss_regs_info g_regs[DSS_MAX_REG_DUMP_REGIONS];

static uint32_t media1_reg[DSS_MAX_REG_DUMP_NUM] = {0};
static uint32_t peri_reg[DSS_MAX_REG_DUMP_NUM] = {0};
static uint32_t pctrl_reg[DSS_MAX_REG_DUMP_NUM] = {0};

// delay 100ms after lcd panel off
#define MNTN_INTERVAL_TIME    100

// dump 128 bytes in one line
#define MNTN_DUMP_BYTES    128

struct rdr_exception_info_s dss_excetption_info[] = {
	{
	.e_modid            = (u32)MODID_DSS_NOC_EXCEPTION,
	.e_modid_end        = (u32)MODID_DSS_NOC_EXCEPTION,
	.e_process_priority = RDR_ERR,
	.e_reboot_priority  = RDR_REBOOT_NO,
	.e_notify_core_mask = RDR_DSS,
	.e_reset_core_mask  = RDR_DSS,
	.e_from_core        = RDR_DSS,
	.e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
	.e_exce_type        = DSS_S_EXCEPTION,
	.e_exce_subtype     = DSS_NOC_EXCEPTION,
	.e_upload_flag      = (u32)RDR_UPLOAD_YES,
	.e_from_module      = "DSS",
	.e_desc             = "DSS_NOC_EXCEPTION",
	},
	{
	.e_modid            = (u32)MODID_DSS_DDRC_EXCEPTION,
	.e_modid_end        = (u32)MODID_DSS_DDRC_EXCEPTION,
	.e_process_priority = RDR_ERR,
	.e_reboot_priority  = RDR_REBOOT_NO,
	.e_notify_core_mask = RDR_DSS,
	.e_reset_core_mask  = RDR_DSS,
	.e_from_core        = RDR_DSS,
	.e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
	.e_exce_type        = DSS_S_EXCEPTION,
	.e_exce_subtype     = DSS_DDRC_EXCEPTION,
	.e_upload_flag      = (u32)RDR_UPLOAD_YES,
	.e_from_module      = "DSS",
	.e_desc             = "DSS_DDRC_EXCEPTION",
	},
};

static int dss_mntn_dts_check_para(void)
{
	if (g_num_reg_regions == 0) {
		DSS_MNTN_ERR("no reg resource to init!\n");
		return -1;
	}

	if (g_num_reg_regions > DSS_MAX_REG_DUMP_REGIONS) {
		DSS_MNTN_ERR("dump region oversize!\n");
		return -1;
	}
	return 0;
}

static int dss_mntn_dts_res_load(struct device_node *np)
{
	uint32_t i;
	struct resource res;

	memset(g_regs, 0, sizeof(struct dss_regs_info) * DSS_MAX_REG_DUMP_REGIONS);

	for (i = 0; i < g_num_reg_regions; i++) {
		if (of_address_to_resource(np, i, &res)) {
			DSS_MNTN_ERR("of_address_to_resource [%d] fail!\n",  i);
			return -1;
		}

		strncpy(g_regs[i].reg_name, res.name, DSS_REG_NAME_LEN - 1);
		g_regs[i].reg_name[DSS_REG_NAME_LEN - 1] = '\0';
		g_regs[i].reg_base = res.start;
		g_regs[i].reg_size = resource_size(&res);

		if (g_regs[i].reg_size == 0) {
			DSS_MNTN_ERR("[%s] reg size is 0, skip map!\n",
				(g_regs[i].reg_name));
			return -1;
		}

		g_regs[i].reg_map_addr = of_iomap(np, i);
		DSS_MNTN_INFO("reg_name [%s], reg_map_addr [0x%x]",
			g_regs[i].reg_name, g_regs[i].reg_map_addr);
		if (g_regs[i].reg_map_addr == NULL) {
			DSS_MNTN_ERR("cannot map [%s] phyaddr to viraddr!\n",
				(g_regs[i].reg_name));
			return -1;
		}
		DSS_MNTN_INFO("map [%s] reg ok!\n", (g_regs[i].reg_name));
	}

	return 0;

}

static int dss_mntn_dts_reg_load(struct device_node *np)
{
	int ret;

	// get peri dump reg number to g_num_peri_reg
	ret = of_property_read_u32(np, "peri-reg-dump-size", &g_num_peri_reg);
	if (ret) {
		DSS_MNTN_ERR("cannot find peri-reg-dump-size in dts!\n");
		return -1;
	}
	ret = of_property_read_u32(np, "pctrl-reg-dump-size", &g_num_pctrl_reg);
	if (ret) {
		DSS_MNTN_ERR("cannot find pctrl-reg-dump-size in dts!\n");
		return -1;
	}
	ret = of_property_read_u32(np, "media1-reg-dump-size",
		&g_num_media1_reg);
	if (ret) {
		DSS_MNTN_ERR("cannot find media1-reg-dump-size in dts!\n");
		return -1;
	}
	// get peri-reg-offset-list from dts to array
	ret = of_property_read_u32_array(np, "peri-reg-offset-list",
		peri_reg, g_num_peri_reg);
	if (ret) {
		pr_err("get peri-reg-offset-list from dts error!\n");
		return -1;
	}
	ret = of_property_read_u32_array(np, "pctrl-reg-offset-list",
		pctrl_reg, g_num_pctrl_reg);
	if (ret) {
		pr_err("get pctrl-reg-offset-list from dts error!\n");
		return -1;
	}
	ret = of_property_read_u32_array(np, "media1-reg-offset-list",
		media1_reg, g_num_media1_reg);
	if (ret) {
		pr_err("get media1-reg-offset-list from dts error!\n");
		return -1;
	}
	return 0;
}

static int dss_mntn_dts_np_load(void)
{
	struct device_node *np = NULL;
	int ret;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,dss_mntn");

	ret = of_property_read_u32(np, "reg-dump-regions", &g_num_reg_regions);
	if (ret) {
		DSS_MNTN_ERR("cannot find reg-dump-regions in dts!\n");
		return -1;
	}

	if (dss_mntn_dts_check_para() == -1)
		return -1;

	if (dss_mntn_dts_res_load(np) == -1)
		return -1;

	if (dss_mntn_dts_reg_load(np) == -1)
		return -1;

	return 0;

}

// reboot dss when NOC happened
static void dss_reboot(struct hisi_fb_data_type *hisifd)
{
	int ret;
	uint32_t bl_level_cur;

	if (hisifd == NULL) {
		DSS_MNTN_ERR("hisifd is null!\n");
		return;
	}

	// power up dss by set reg directly
	dss_power_up(hisifd);

	// record current bl level
	bl_level_cur = hisifd->bl_level;

	// set backlight to 0
	down(&hisifd->brightness_esd_sem);
	hisifb_set_backlight(hisifd, 0, false);
	up(&hisifd->brightness_esd_sem);
	DSS_MNTN_INFO("hisifb set backlight 0!\n");

	// lcd panel off
	ret = hisi_fb_blank_sub(FB_BLANK_POWERDOWN, hisifd->fbi);
	msleep(MNTN_INTERVAL_TIME);
	if (ret != 0)
		DSS_MNTN_ERR("fb%d, blank_mode [%d] failed!\n",
		hisifd->index, FB_BLANK_POWERDOWN);

	DSS_MNTN_INFO("hisifb blank!\n");

	// lcd panel on
	ret = hisi_fb_blank_sub(FB_BLANK_UNBLANK, hisifd->fbi);
	if (ret != 0)
		DSS_MNTN_ERR("fb%d, blank_mode [%d] failed!\n",
			hisifd->index, FB_BLANK_UNBLANK);

	DSS_MNTN_INFO("hisifb unblank!\n");

	// indicated that dss reboot successful
	hisi_fb_frame_refresh(hisifd, "mntn");
	DSS_MNTN_INFO("DSS_RESET_HAPPENDED!\n");
	msleep(MNTN_INTERVAL_TIME);

	// set backlight to bl_level_cur
	down(&hisifd->brightness_esd_sem);
	hisifb_set_backlight(hisifd,
		bl_level_cur ? bl_level_cur : hisifd->bl_level, false);
	up(&hisifd->brightness_esd_sem);
	DSS_MNTN_INFO("set backlight, level_cur = %d, level = %d!\n",
		bl_level_cur, hisifd->bl_level);
}

static int dss_mntn_savebuf2fs(char *logpath, const void *buf, u32 len, u32 is_append)
{
	int ret;
	int flags;
	struct file *fp = NULL;

	if ((logpath == NULL) || (buf == NULL) || (len <= 0)) {
		DSS_MNTN_ERR("invalid  parameter!\n");
		return -1;
	}
	flags = O_CREAT | O_RDWR | (is_append ? O_APPEND : O_TRUNC);
	fp = filp_open(logpath, flags, DSS_FILESYS_DEFAULT_MODE);
	if (IS_ERR(fp)) {
		DSS_MNTN_ERR("create file %s err! fp = %d\n", logpath, fp);
		return -1;
	}
	vfs_llseek(fp, 0L, SEEK_END);
	ret = vfs_write(fp, buf, len, &(fp->f_pos));
	if (ret != len) {
		DSS_MNTN_ERR("write file %s exception, ret = %d\n",
			logpath, ret);
		filp_close(fp, NULL);
		return -1;
	}
	vfs_fsync(fp, 0);
	return 0;
}

static int dss_dump_reg(const char *pathname)
{
	char path[DSS_LOG_PATH_MAXLEN] = {0};
	int ret;
	uint32_t i;
	u32 temp_register = 0;
	char buff[MNTN_DUMP_BYTES];

	if (pathname == NULL)
		DSS_MNTN_ERR("path name is NULL!\n");
	memset(buff, 0, MNTN_DUMP_BYTES);
	snprintf(path, (size_t)DSS_LOG_PATH_MAXLEN, "%s%s",
		pathname, DSS_LOG_NAME);
	ret = dss_mntn_dts_np_load();
	if (ret)
		DSS_MNTN_ERR("fail to load dss mntn dts np!\n");
	for (i = 0; i < g_num_peri_reg; i++) {
		temp_register = inp32(g_regs[0].reg_map_addr + peri_reg[i]);
		snprintf(buff, MNTN_DUMP_BYTES, "peri register 0x%x = 0x%x\n",
			peri_reg[i], temp_register);
		ret = dss_mntn_savebuf2fs(path, buff, strlen(buff), true);
		memset(buff, 0, MNTN_DUMP_BYTES);
	}
	for (i = 0; i < g_num_pctrl_reg; i++) {
		temp_register = inp32(g_regs[1].reg_map_addr + pctrl_reg[i]);
		snprintf(buff, MNTN_DUMP_BYTES, "pctrl register 0x%x = 0x%x\n",
			pctrl_reg[i], temp_register);
		ret = dss_mntn_savebuf2fs(path, buff, strlen(buff), true);
		memset(buff, 0, MNTN_DUMP_BYTES);
	}
	for (i = 0; i < g_num_media1_reg; i++) {
		temp_register = inp32(g_regs[2].reg_map_addr + media1_reg[i]);
		snprintf(buff, MNTN_DUMP_BYTES, "media1 register 0x%x = 0x%x\n",
			media1_reg[i], temp_register);
		ret = dss_mntn_savebuf2fs(path, buff, strlen(buff), true);
		memset(buff, 0, MNTN_DUMP_BYTES);
	}
	return ret;
}

static void dss_dump(u32 modid, u32 etype, u64 coreid,
	char *pathname, pfn_cb_dump_done pfn_cb)
{
	struct hisi_fb_data_type *hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	int result = -1;

	// check media subsys is powered or not
	if (dss_check_media_subsys_status(hisifd) != 0) {
		// media subsys is powered down, do not dump and reboot directly
		DSS_MNTN_INFO("media subsys is powered down!\n");
		goto cb_func;
	}

	result = dss_dump_reg(pathname);
	if (result == 0)
		DSS_MNTN_INFO("dump success!\n");

cb_func:
	// callback dump done
	if (pfn_cb)
		pfn_cb(modid, coreid);
}

static void dss_reset(u32 modid, u32 etype, u64 coreid)
{
	struct hisi_fb_data_type *hisifd = hisifd_list[PRIMARY_PANEL_IDX];

	dss_reboot(hisifd);
}

static int dss_mntn_register_core(void)
{
	struct rdr_module_ops_pub module_ops;
	int ret;

	module_ops.ops_dump = dss_dump;
	module_ops.ops_reset = dss_reset;

	ret = rdr_register_module_ops(RDR_DSS, &module_ops, &g_current_info);
	if (ret < 0) {
		DSS_MNTN_ERR("register failed! ret = %d\n", ret);
		return ret;
	}

	return 0;
}

static int dss_mntn_register_exception(void)
{
	int ret = 0;
	unsigned int  size;
	unsigned long index;

	size = sizeof(dss_excetption_info) /
			sizeof(struct rdr_exception_info_s);

	for (index = 0; index < size; index++) {
		// error return 0, ok return modid
		ret = rdr_register_exception(&dss_excetption_info[index]);
		if (ret == 0) {
			DSS_MNTN_ERR("register fail! index = %ld, ret = %d\n",
				index, ret);
			return -EINVAL;
		}
	}

	return 0;
}

#ifdef CONFIG_HISI_NOC_MODID_REGISTER
static void dss_mntn_register_noc_modid(void)
{
	uint32_t i, noc_para_num = 0;
	struct noc_err_para_s *dss_noc_para =
		dss_get_noc_err_para(&noc_para_num);

	if (dss_noc_para == NULL)
		DSS_MNTN_ERR("dss_noc_para is NULL!\n");

	for (i = 0; i < noc_para_num; i++)
		noc_modid_register(dss_noc_para[i], MODID_DSS_NOC_EXCEPTION);

}
#endif

static int dss_mntn_init_early(void)
{
	int ret;

	// register module
	ret = dss_mntn_register_core();
	if (ret != 0) {
		DSS_MNTN_ERR("dss mntn register core failed! ret = %d\n", ret);
		return ret;
	}

	// regitster exception
	ret = dss_mntn_register_exception();
	if (ret != 0) {
		DSS_MNTN_ERR("dss mntn register exception failed! ret = %d\n",
			ret);
		return ret;
	}

	// regitster NOC error and mod id
#ifdef CONFIG_HISI_NOC_MODID_REGISTER
	dss_mntn_register_noc_modid();
#endif

	DSS_MNTN_INFO("dss mntn init early success!\n");

	return 0;
}

static int dss_mntn_probe(struct platform_device *pdev)
{
	int ret = 0;
	(void *)pdev;

	if (dss_mntn_init_early()) {
		DSS_MNTN_INFO("dss mntn init failed!\n");
		ret = -1;
	}

	DSS_MNTN_INFO("dss mntn init success!\n");
	return ret;
}

static int dss_mntn_remove(struct platform_device *pdev)
{
	int ret;

	(void *)pdev;
	ret = rdr_unregister_module_ops(RDR_DSS);
	if (ret != 0) {
		DSS_MNTN_ERR("dss mntn unreg module failed! ret = %d\n",
			ret);
		return ret;
	}
	ret = rdr_unregister_exception(RDR_DSS);
	if (ret != 0) {
		DSS_MNTN_ERR("dss mntn unregister failed! ret = %d\n",
			ret);
		return ret;
	}
	DSS_MNTN_INFO("dss mntn deinit success!\n");
	return 0;
}

// dss mntn test function entry
int dss_system_error_test(uint32_t modid, uint32_t arg1, uint32_t arg2)
{
	int ret = 0;

	if ((modid < MODID_DSS_NOC_EXCEPTION) ||
		(modid >= MODID_DSS_EXCEPTION_END)) {
		ret = -1;
		DSS_MNTN_ERR("dss mntn modid [%d] is error!\n", modid);
	} else {
		/* call rdr_system_error to
		 * trigger RDR exception handling process
		 */
		DSS_MNTN_INFO("call rdr_system_error!\n");
		rdr_system_error(modid, arg1, arg2);
	}
	return ret;
}

static const struct of_device_id dss_mntn_match[] = {
	{ .compatible = "hisilicon,dss_mntn" },
};

static struct platform_driver dss_mntn_driver = {
	.probe = dss_mntn_probe,
	.remove = dss_mntn_remove,
	.driver = {
		.name = "dss-mntn",
		.of_match_table = dss_mntn_match,
	},
};

static int __init dss_mntn_init(void)
{
	// dss mntn driver register
	return platform_driver_register(&dss_mntn_driver);
}

static void __exit dss_mntn_exit(void)
{
	// dss mntn driver unregister
	platform_driver_unregister(&dss_mntn_driver);
}

module_init(dss_mntn_init);
module_exit(dss_mntn_exit);
