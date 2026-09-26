/*
 * cpu_buck.c
 *
 * cpu buck error monitor driver
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

#include "cpu_buck.h"
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG cpu_buck
HWLOG_REGIST();

static struct cpu_buck_sample *g_cbs;

void cpu_buck_str_to_reg(const char *str, char *reg, int size)
{
	unsigned char high;
	unsigned char low;
	int i, tmp_size;

	if (!str || !reg) {
		hwlog_err("str or reg is null\n");
		return;
	}

	/* 2: bytes width */
	tmp_size = strlen(str) / 2 > size ? size : strlen(str) / 2;
	for (i = 0; i < tmp_size; ++i) {
		/*
		 * get two bytes of data,
		 * the first byte is shifted to the left by 4 bits,
		 * and the second byte is merged into one data
		 */
		high = *(str + 2 * i);
		low = *(str + 2 * i + 1);
		high = power_change_char_to_digit(high);
		low = power_change_char_to_digit(low);
		*(reg + i) = (high << 4) | low;
	}
}

void cpu_buck_register(struct cpu_buck_sample *p_cbs)
{
	struct cpu_buck_sample *cbs = NULL;

	if (!p_cbs) {
		hwlog_err("p_cbs is null\n");
		return;
	}

	hwlog_info("dev_id=%d, info_size=%d register ok\n",
		p_cbs->dev_id, p_cbs->info_size);

	if (!g_cbs) {
		g_cbs = p_cbs;
	} else {
		cbs = g_cbs;
		while (cbs->cbs)
			cbs = cbs->cbs;

		cbs->cbs = p_cbs;
	}
}

static int __init cpu_buck_parse_early_cmdline(char *p)
{
	if (!p) {
		hwlog_err("cmdline is null\n");
		return 0;
	}

	hwlog_info("normal_reset_type=%s\n", p);

	/* 8: cpu_buck index */
	if (!strncmp(p, "CPU_BUCK", 8))
		hwlog_info("find cpu_buck from normal reset type\n");

	return 0;
}
early_param("normal_reset_type", cpu_buck_parse_early_cmdline);

static void cpu_buck_monitor_work(struct work_struct *work)
{
	int i;
	struct cpu_buck_sample *cbs = g_cbs;
	char tmp_buf[MAX_ERR_MSG_LEN] = { 0 };

	while (cbs) {
		hwlog_info("dev_id=%d, info_size=%d\n",
			cbs->dev_id, cbs->info_size);

		for (i = 0; i < cbs->info_size; ++i) {
			if ((cbs->cbi[i].err_mask &
				cbs->reg[cbs->cbi[i].reg_num]) !=
				cbs->cbi[i].err_mask)
				continue;
			snprintf(tmp_buf, MAX_ERR_MSG_LEN - 1,
				"cpu buck dev_id=%d, err_msg:%s\n",
				cbs->dev_id, cbs->cbi[i].err_msg);
			hwlog_info("buck exception happened: %s\n", tmp_buf);
			power_dsm_report_dmd(POWER_DSM_CPU_BUCK,
				ERROR_NO_CPU_BUCK_BASE + cbs->cbi[i].err_no,
				tmp_buf);
		}

		cbs = cbs->cbs;
	}
}

static int cpu_buck_probe(struct platform_device *pdev)
{
	struct cpu_buck_dev *di = NULL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	INIT_DELAYED_WORK(&di->monitor_work, cpu_buck_monitor_work);
	schedule_delayed_work(&di->monitor_work, DELAY_TIME_FOR_WORK);
	return 0;
}

static const struct of_device_id cpu_buck_match_table[] = {
	{
		.compatible = "huawei,cpu_buck",
		.data = NULL,
	},
	{},
};

static struct platform_driver cpu_buck_driver = {
	.probe = cpu_buck_probe,
	.driver = {
		.name = "huawei,cpu_buck",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(cpu_buck_match_table),
	},
};

static int __init cpu_buck_init(void)
{
	return platform_driver_register(&cpu_buck_driver);
}


static void __exit cpu_buck_exit(void)
{
	platform_driver_unregister(&cpu_buck_driver);
}

module_init(cpu_buck_init);
module_exit(cpu_buck_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("cpu buck error monitor module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
