/*
 * cpu_buck.c
 *
 * cpu buck error monitor driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/platform_device.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/power_dsm.h>
#include <cpu_buck.h>
#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif

#define HWLOG_TAG cpu_buck
HWLOG_REGIST();

static struct cpu_buck_sample *g_cbs;

void cpu_buck_str_to_reg(char *str, char *reg, int size)
{
	unsigned char high;
	unsigned char low;
	int i;

	for (i = 0; i < size; ++i) {
		high = *(str + 2 * i);
		low = *(str + 2 * i + 1);
		high = (high > '9') ? (high - 'a' + WEIGHT_10) : (high - '0');
		low = (low > '9') ? (low - 'a' + WEIGHT_10) : (low - '0');
		*(reg + i) = (high << 4) | low; /* 4bits */
	}
}

void cpu_buck_register(struct cpu_buck_sample *p_cbs)
{
	struct cpu_buck_sample *cbs = NULL;

	if (!p_cbs) {
		hwlog_err("p_cbs is null\n");
		return;
	}

	hwlog_info("register ok(dev_id=%d, info_size=%d)\n",
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

static int __init early_parse_normal_reset_type_cmdline(char *p)
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

early_param("normal_reset_type", early_parse_normal_reset_type_cmdline);

static void cpu_buck_update_err_msg(struct cpu_buck_error_info *cbi)
{
	char err_msg[MAX_EXTRA_ERR_MSG_LEN] = { 0 };
	int batt_temp;
	int batt_vol;
	int batt_soc;

	if (!cbi) {
		hwlog_err("cbi is null\n");
		return;
	}

	if (cbi->err_no == HI6422V200_VSYS_PWRON_D60UR ||
		cbi->err_no == HI6422V200_VSYS_PWROFF_ABS_2D) {
		batt_temp = hisi_battery_temperature();
		batt_vol = hisi_battery_voltage();
		batt_soc = hisi_battery_capacity();
		snprintf(err_msg, MAX_EXTRA_ERR_MSG_LEN - 1,
			" batt_temp=%d, batt_vol=%d, batt_soc=%d\n",
			batt_temp, batt_vol, batt_soc);
	}

	if (strlen(err_msg) > 0)
		strncat(cbi->err_msg, err_msg, strlen(err_msg));
}

static void cpu_buck_error_monitor_work(struct work_struct *work)
{
	int i;
	struct cpu_buck_sample *cbs = g_cbs;
	char tmp_buf[MAX_ERR_MSG_LEN] = { 0 };

	hwlog_info("monitor_work begin\n");

	while (cbs) {
		hwlog_info("dev_id=%d, info_size=%d\n",
			cbs->dev_id, cbs->info_size);

		for (i = 0; i < cbs->info_size; ++i) {
			if ((cbs->cbi[i].err_mask &
				cbs->reg[cbs->cbi[i].reg_number]) ==
				cbs->cbi[i].err_mask) {
				cpu_buck_update_err_msg(&(cbs->cbi[i]));
				snprintf(tmp_buf, MAX_ERR_MSG_LEN - 1,
					"cpu buck dev_id=%d, err_msg:%s\n",
					cbs->dev_id, cbs->cbi[i].err_msg);

				hwlog_info("exception happened: %s\n", tmp_buf);

				power_dsm_dmd_report(POWER_DSM_CPU_BUCK,
				ERROR_NO_CPU_BUCK_BASE + cbs->cbi[i].err_no,
				tmp_buf);
			}
		}

		cbs = cbs->cbs;
	}

	hwlog_info("monitor_work end\n");
}

static int cpu_buck_probe(struct platform_device *pdev)
{
	struct cpu_buck_device_info *di = NULL;

	hwlog_info("probe begin\n");

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	INIT_DELAYED_WORK(&di->monitor_work, cpu_buck_error_monitor_work);
	schedule_delayed_work(&di->monitor_work, 100); /* 100 jiffies */

	hwlog_info("probe end\n");
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
