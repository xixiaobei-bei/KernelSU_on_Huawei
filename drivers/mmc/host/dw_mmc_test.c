/*
 * drivers/mmc/host/dw_mmc_test.c
 *
 * Copyright (C) 2017-2020 Hisilicon Technologies Co., Ltd. All rights reserved.
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

#include <linux/bootdevice.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/types.h>
#include <linux/hisi/partition_ap_kernel.h>
#include <linux/uaccess.h>
#include <linux/printk.h>
#include <linux/bootdevice.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/host.h>
#include <linux/mmc/dw_mmc.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/regulator/consumer.h>
#include <linux/of_address.h>
#include <linux/pm_runtime.h>
#include <linux/clk-provider.h>
#include <linux/hisi/util.h>
#include <linux/hwspinlock.h>
#include <linux/gpio.h>

#ifdef CONFIG_MMC_DW_MUX_SDSIM
#include <linux/mmc/dw_mmc_mux_sdsim.h>
#endif

#include "dw_mmc.h"
#include "dw_mmc_zodiac.h"
#include "dw_mmc-pltfm.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

/* sd loop test */
struct workqueue_struct *sd_sdio_test_work;
int sd_init_loop_work;

/* sd modus change */
unsigned int sd_caps_modus;
unsigned int sd_loop_flag;
/* sdcard reset test flag */
#if defined(CONFIG_HISI_DEBUG_FS)
unsigned int sd_test_reset_flag;
#endif
#define SD_SDR104 1
#define SD_SDR50 2
#define SD_SDR25 4
#define SD_SDR12 8

#define SD_MODEL_SDR104 0x0
#define SD_MODEL_SDR50 0x1
#define SD_MODEL_SDR25 0x3
#define SD_MODEL_SDR12 0x7
#define SD_MODEL_HS 0xf
#define SD_MODEL_MAX_SPEED 0x1f

#define CRG_PRINT 0x1
#define SD_LOOP_COUNT 0x5a5a
#define SD_TEST_TYPE_SIZE 10

#define SOC_CRGPERIPH_PERCLKEN0_UNION 0x008
#define SOC_CRGPERIPH_PERSTAT0_UNION 0x00c
#define SOC_CRGPERIPH_PERCLKEN4_UNION 0x048
#define SOC_CRGPERIPH_PERSTAT4_UNION 0x04C
#define SOC_CRGPERIPH_PERCLKEN5_UNION 0x058
#define SOC_CRGPERIPH_PERSTAT5_UNION 0x05c
#define SOC_CRGPERIPH_PERRSTSTAT4_UNION 0x098
#define SOC_CRGPERIPH_CLKDIV0_UNION 0x0a8
#define SOC_CRGPERIPH_CLKDIV1_UNION 0x0ac
#define SOC_CRGPERIPH_CLKDIV6_UNION 0x0c0
#define SOC_CRGPERIPH_CLKDIV17_UNION 0x0ec
#define SOC_CRGPERIPH_CLKDIV19_UNION 0x0f4
#define SOC_CRGPERIPH_PERI_AUTODIV_STAT_UNION 0x40c

unsigned int test_sd_data;

#define DEVICE_INIT 1

#ifdef CONFIG_MMC_DW_MUX_SDSIM
/* mock simcard test definition start */
#define SIM_TEST_CLK 0
#define SIM_TEST_CMD 1
#define SIM_TEST_DATA0 2
#define SIM_TEST_DATA1 3
#define SIM_TEST_DATA2 4
#define SIM_TEST_DATA3 5

#define SIM_TEST_VMMC 2
#define SIM_TEST_LOW 1
#define SIM_TEST_HIGH 0

#define PULL_TYPE_NP 0x00
#define PULL_TYPE_PU 0x01
#define PULL_TYPE_PD 0x02

#define PIN_NUM 6
#define MOCKCARD_PIN_NUM 3
#define SIM_TEST_TAG "[MOCK_SIMCARD_TEST]"
unsigned char nocard_pin_value_map[PIN_NUM][PIN_NUM] = {
	/* clk, cmd, data0, data1, data2, data3 */
	{ 1, 0, 0, 1, 0, 0 }, /* clk */
	{ 0, 1, 0, 1, 0, 0 }, /* cmd */
	{ 0, 0, 1, 1, 0, 0 }, /* data0 */
	{ 0, 0, 0, 1, 0, 0 }, /* data1 */
	{ 0, 0, 0, 1, 1, 0 }, /* data2 */
	{ 0, 0, 0, 1, 0, 1 }, /* data3 */
};

unsigned char mockcard_pin_value_map[MOCKCARD_PIN_NUM][PIN_NUM] = {
	/* clk, cmd, data0, data1, data2, data3 */
	{ 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0 },
	{ 1, 1, 0, 1, 0, 1 },
};

unsigned char nocard_pin_value_result[PIN_NUM][PIN_NUM];
unsigned char nocard_pin_value_result_temp[PIN_NUM][PIN_NUM];
unsigned char mockcard_pin_value_result[MOCKCARD_PIN_NUM][PIN_NUM];
unsigned char mockcard_pin_value_result_temp[MOCKCARD_PIN_NUM][PIN_NUM];
char *nocard_pin_value_print[PIN_NUM][PIN_NUM] = {
	{ "clk[1]clk", "clk[1]cmd", "clk[1]dt0", "clk[1]dt1", "clk[1]dt2", "clk[1]dt3" },
	{ "cmd[1]clk", "cmd[1]cmd", "cmd[1]dt0", "cmd[1]dt1", "cmd[1]dt2", "cmd[1]dt3" },
	{ "dt0[1]clk", "dt0[1]cmd", "dt0[1]dt0", "dt0[1]dt1", "dt0[1]dt2", "dt0[1]dt3" },
	{ "dt1[1]clk", "dt1[1]cmd", "dt1[1]dt0", "dt1[1]dt1", "dt1[1]dt2", "dt1[1]dt3" },
	{ "dt2[1]clk", "dt2[1]cmd", "dt2[1]dt0", "dt2[1]dt1", "dt2[1]dt2", "dt2[1]dt3" },
	{ "dt3[1]clk", "dt3[1]cmd", "dt3[1]dt0", "dt3[1]dt1", "dt3[1]dt2", "dt3[1]dt3" },
};
char *mockcard_pin_value_print[MOCKCARD_PIN_NUM][PIN_NUM] = {
	{ "clkdt2dt3[1]clk", "clkdt2dt3[1]cmd", "clkdt2dt3[1]dt0",
	  "clkdt2dt3[1]dt1", "clkdt2dt3[1]dt2", "clkdt2dt3[1]dt3" },
	{ "clkdt2dt3[0]clk", "clkdt2dt3[0]cmd", "clkdt2dt3[0]dt0",
	  "clkdt2dt3[0]dt1", "clkdt2dt3[0]dt2", "clkdt2dt3[0]dt3" },
	{ "ldo16[1]clk", "ldo16[1]cmd", "ldo16[1]dt0",
	  "ldo16[1]dt1", "ldo16[1]dt2", "ldo16[1]dt3" },
};
enum test_state {
	IDLE = 0,
	NOCARD_PROCESSING,
	MOCKCARD_PROCESSING,
	NOCARD_IDLE,
	MOCKCARD_IDLE,
} sim_test_state;

enum test_result {
	TEST_DEFAULT = 0,
	TEST_DOING,
	TEST_PASS,
	TEST_ERROR,
} simcard_test_result;

#ifdef CONFIG_MMC_SIM_GPIO_EXCHANGE
static int get_regulator_flag;
#endif
/* mock simcard test definition end */
#endif

void mmc_sd_test(unsigned int loop_enable_flag)
{
	sd_init_loop_work = 0;
	if (loop_enable_flag == 1)
		sd_loop_flag = SD_LOOP_COUNT;
	else if (loop_enable_flag == 0)
		sd_loop_flag = 0;
	else
		pr_err("Don't support this argument, please input again!\n");
}

/* sd loop test */
void mmc_sd_loop_test(void)
{
	sd_init_loop_work = SD_LOOP_COUNT;
}

/* sd modus change */
void test_sd_delete_host_caps(struct mmc_host *host)
{
	if (sd_caps_modus & SD_SDR104)
		host->caps &= ~MMC_CAP_UHS_SDR104;

	if (sd_caps_modus & SD_SDR50)
		host->caps &= ~MMC_CAP_UHS_SDR50;

	if (sd_caps_modus & SD_SDR25)
		host->caps &= ~MMC_CAP_UHS_SDR25;

	if (sd_caps_modus & SD_SDR12)
		host->caps &= ~MMC_CAP_UHS_SDR12;
	if (sd_caps_modus == SD_MODEL_MAX_SPEED)
		host->caps |= MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 |
			      MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_SDR104;
}

/* sd data err */
void test_sd_data_func(unsigned int input)
{
	if (input == (SDMMC_INT_DRTO))
		test_sd_data = SDMMC_INT_DRTO;
	else if (input == (SDMMC_INT_DCRC))
		test_sd_data = SDMMC_INT_DCRC;
	else if (input == (SDMMC_INT_EBE))
		test_sd_data = SDMMC_INT_EBE;
	else if (input == (SDMMC_INT_SBE))
		test_sd_data = SDMMC_INT_SBE;
	else
		test_sd_data = SDMMC_INT_HTO;
}

static void sd_sdio_loop_test(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct mmc_host *host =
		container_of(dwork, struct mmc_host, sd_sdio_test_work);

	/* do what you want,example read 0x0 reg */
	if (sd_loop_flag == SD_LOOP_COUNT) {
		pr_err("sd_sdio_loop_testXXXXXXXXX\n");
		queue_delayed_work(sd_sdio_test_work, &host->sd_sdio_test_work,
			msecs_to_jiffies(1000));
	}
}

/* Internal function, host can not be empty */
void dw_mci_dump_crg(struct dw_mci *host)
{
	struct dw_mci_hs_priv_data *priv = host->priv;

	if (priv->crg_print == CRG_PRINT) {
		dev_err(host->dev, ": SOC_CRGPERIPH_PERCLKEN0_UNION:    "
			"offset:0x%x       val:0x%08x\n",
			SOC_CRGPERIPH_PERCLKEN0_UNION,
			readl(pericrg_base + SOC_CRGPERIPH_PERCLKEN0_UNION));
		dev_err(host->dev, ": SOC_CRGPERIPH_PERSTAT0_UNION:    "
			"offset:0x%x       val:0x%08x\n",
			SOC_CRGPERIPH_PERSTAT0_UNION,
			readl(pericrg_base + SOC_CRGPERIPH_PERSTAT0_UNION));
		dev_err(host->dev, ": SOC_CRGPERIPH_PERCLKEN4_UNION:    "
			"offset:0x%x       val:0x%08x\n",
			SOC_CRGPERIPH_PERCLKEN4_UNION,
			readl(pericrg_base + SOC_CRGPERIPH_PERCLKEN4_UNION));
		dev_err(host->dev, ": SOC_CRGPERIPH_PERSTAT4_UNION:    "
			"offset:0x%x       val:0x%08x\n",
			SOC_CRGPERIPH_PERSTAT4_UNION,
			readl(pericrg_base + SOC_CRGPERIPH_PERSTAT4_UNION));
		dev_err(host->dev, ": SOC_CRGPERIPH_PERCLKEN5_UNION:    "
			"offset:0x%x       val:0x%08x\n",
			SOC_CRGPERIPH_PERCLKEN5_UNION,
			readl(pericrg_base + SOC_CRGPERIPH_PERCLKEN5_UNION));
		dev_err(host->dev, ": SOC_CRGPERIPH_PERSTAT5_UNION:    "
			"offset:0x%x       val:0x%08x\n",
			SOC_CRGPERIPH_PERSTAT5_UNION,
			readl(pericrg_base + SOC_CRGPERIPH_PERSTAT5_UNION));
		dev_err(host->dev, ": SOC_CRGPERIPH_PERRSTSTAT4_UNION:    "
			"offset:0x%x       val:0x%08x\n",
			SOC_CRGPERIPH_PERRSTSTAT4_UNION,
			readl(pericrg_base + SOC_CRGPERIPH_PERRSTSTAT4_UNION));
		dev_err(host->dev, ": SOC_CRGPERIPH_CLKDIV0_UNION:    "
			"offset:0x%x       val:0x%08x\n",
			SOC_CRGPERIPH_CLKDIV0_UNION,
			readl(pericrg_base + SOC_CRGPERIPH_CLKDIV0_UNION));
		dev_err(host->dev, ": SOC_CRGPERIPH_CLKDIV1_UNION:    "
			"offset:0x%x       val:0x%08x\n",
			SOC_CRGPERIPH_CLKDIV1_UNION,
			readl(pericrg_base + SOC_CRGPERIPH_CLKDIV1_UNION));
		dev_err(host->dev, ": SOC_CRGPERIPH_CLKDIV6_UNION:    "
			"offset:0x%x       val:0x%08x\n",
			SOC_CRGPERIPH_CLKDIV6_UNION,
			readl(pericrg_base + SOC_CRGPERIPH_CLKDIV6_UNION));
		dev_err(host->dev, ": SOC_CRGPERIPH_CLKDIV17_UNION:    "
			"offset:0x%x       val:0x%08x\n",
			SOC_CRGPERIPH_CLKDIV17_UNION,
			readl(pericrg_base + SOC_CRGPERIPH_CLKDIV17_UNION));
		dev_err(host->dev, ": SOC_CRGPERIPH_CLKDIV19_UNION:    "
			"offset:0x%x       val:0x%08x\n",
			SOC_CRGPERIPH_CLKDIV19_UNION,
			readl(pericrg_base + SOC_CRGPERIPH_CLKDIV19_UNION));
		dev_err(host->dev, ": SOC_CRGPERIPH_PERI_AUTODIV_STAT_UNION:   "
			" offset:0x%x       val:0x%08x\n",
			SOC_CRGPERIPH_PERI_AUTODIV_STAT_UNION,
			readl(pericrg_base + SOC_CRGPERIPH_PERI_AUTODIV_STAT_UNION));
	}
}

#if defined(CONFIG_HISI_DEBUG_FS)
static int sd_modul_proc_show(struct seq_file *m, void *v)
{
	if (sd_caps_modus == SD_MODEL_SDR104)
		seq_puts(m, "Max SD model is SDR104\n");
	else if (sd_caps_modus == SD_MODEL_SDR50)
		seq_puts(m, "Max SD model is SDR50\n");
	else if (sd_caps_modus == SD_MODEL_SDR25)
		seq_puts(m, "Max SD model is SDR25\n");
	else if (sd_caps_modus == SD_MODEL_SDR12)
		seq_puts(m, "Max SD model is SDR12\n");
	else if (sd_caps_modus == SD_MODEL_HS)
		seq_puts(m, "Max SD model is HS\n");
	else if (sd_caps_modus == SD_MODEL_MAX_SPEED)
		seq_puts(m, "Change to Max SD speed mode\n");
	else
		seq_puts(m, "Invalid sd_caps_model\n");

	return 0;
}

static int sd_reset_proc_show(struct seq_file *m, void *v)
{
	if (sd_test_reset_flag == 1)
		seq_puts(m, "sd_reset:enable reset function\n");
	else if (sd_test_reset_flag == 0)
		seq_puts(m, "cancel_reset:close reset function\n");
	else
		seq_puts(m, "Invalid sd_test_reset_flag\n");
	return 0;
}

#ifdef CONFIG_MMC_DW_MUX_SDSIM
static void nocard_pin_value_result_dump(struct seq_file *m)
{
	int i, j;
	int fail_flag = 0;

	for (i = 0; i < PIN_NUM; i++) {
		for (j = 0; j < PIN_NUM; j++) {
			if (nocard_pin_value_result_temp[i][j]) {
				fail_flag += 1;
				simcard_test_result = TEST_ERROR;
				if (fail_flag == 1)
					seq_printf(m, "NO_CARD,%d,%s,", simcard_test_result,
						nocard_pin_value_print[i][j]);
				else
					seq_printf(m, "%s,", nocard_pin_value_print[i][j]);
			}
		}
	}
	if (!fail_flag) {
		simcard_test_result = TEST_PASS;
		seq_printf(m, "NO_CARD,%d", simcard_test_result);
	}
}

static void mockcard_pin_value_result_dump(struct seq_file *m)
{
	int i, j;
	int fail_flag = 0;

	for (i = 0; i < MOCKCARD_PIN_NUM; i++) {
		for (j = 0; j < PIN_NUM; j++) {
			if (mockcard_pin_value_result_temp[i][j]) {
				fail_flag += 1;
				simcard_test_result = TEST_ERROR;
				if (fail_flag == 1)
					seq_printf(m, "MOCK_CARD,%d,%s,", simcard_test_result,
						mockcard_pin_value_print[i][j]);
				else
					seq_printf(m, "%s,", mockcard_pin_value_print[i][j]);
			}
		}
	}
	if (!fail_flag) {
		simcard_test_result = TEST_PASS;
		seq_printf(m, "MOCK_CARD,%d", simcard_test_result);
	}
}

static int simcard_test_proc_show(struct seq_file *m, void *v)
{
	unsigned int i, j;

	if (sim_test_state == IDLE) {
		simcard_test_result = TEST_DEFAULT;
		seq_printf(m, "default,%d", simcard_test_result);
	} else if (sim_test_state == NOCARD_PROCESSING) {
		simcard_test_result = TEST_DOING;
		pr_err("nocard processing!\n");
		seq_printf(m, "NO_CARD,%d", simcard_test_result);
	} else if (sim_test_state == MOCKCARD_PROCESSING) {
		simcard_test_result = TEST_DOING;
		pr_err("mockcard processing!\n");
		seq_printf(m, "MOCK_CARD,%d", simcard_test_result);
	} else if (sim_test_state == NOCARD_IDLE) {
		pr_err("%s:%s nocard_test done!\n", __func__, SIM_TEST_TAG);
		for (i = 0; i < PIN_NUM; i++) {
			for (j = 0; j < PIN_NUM; j++)
				nocard_pin_value_result_temp[i][j] =
					nocard_pin_value_result[i][j] ^
					nocard_pin_value_map[i][j];
		}
		nocard_pin_value_result_dump(m);
	} else if (sim_test_state == MOCKCARD_IDLE) {
		pr_err("%s:%s mockcard_test done!\n", __func__, SIM_TEST_TAG);
		for (i = 0; i < MOCKCARD_PIN_NUM; i++) {
			for (j = 0; j < PIN_NUM; j++)
				mockcard_pin_value_result_temp[i][j] =
					mockcard_pin_value_result[i][j] ^
					mockcard_pin_value_map[i][j];
		}
		mockcard_pin_value_result_dump(m);
	}
	return 0;
}
#endif

static ssize_t sd_modul_set_proc_write(struct file *p_file,
	const char __user *userbuf, size_t count, loff_t *ppos)
{
	char buf[SD_TEST_TYPE_SIZE] = {0};

	if (!count)
		return -1;

	if (!userbuf)
		return -1;

	if (copy_from_user(buf, userbuf, sizeof(buf) - 1))
		return -1;

	if (!strncmp("SDR104", buf, strlen("SDR104")))
		sd_caps_modus = SD_MODEL_SDR104;
	else if (!strncmp("SDR50", buf, strlen("SDR50")))
		sd_caps_modus = SD_MODEL_SDR50;
	else if (!strncmp("SDR25", buf, strlen("SDR25")))
		sd_caps_modus = SD_MODEL_SDR25;
	else if (!strncmp("SDR12", buf, strlen("SDR12")))
		sd_caps_modus = SD_MODEL_SDR12;
	else if (!strncmp("HSPEED", buf, strlen("HSPEED")))
		sd_caps_modus = SD_MODEL_HS;
	else if (!strncmp("ALL", buf, strlen("ALL")))
		sd_caps_modus = SD_MODEL_MAX_SPEED;
	return (ssize_t)count;
}

#ifdef CONFIG_MMC_DW_MUX_SDSIM
static void simcard_test_power_up(void)
{
	int ret;
	struct dw_mci *host = host_from_sd_module;

#ifdef CONFIG_MMC_SIM_GPIO_EXCHANGE
	if (host->need_get_mmc_regulator) {
		get_regulator_flag = 1;
		dw_mci_get_regulator(host);
	}
#endif
	if (host && host->vqmmc) {
		/* set vmmc to 1.775v */
		ret = regulator_set_voltage(host->vqmmc, 1775000, 1775000);
		pr_info("%s:%s vqmmc set 1.775v ret=%d\n", __func__, SIM_TEST_TAG, ret);
		/* set vmmc to 1.80v */
		ret = regulator_set_voltage(host->vqmmc, 1800000, 1800000);
		pr_info("%s:%s vqmmc set 1.800v ret=%d\n", __func__, SIM_TEST_TAG, ret);
		ret = regulator_enable(host->vqmmc);
		pr_info("%s:%s vqmmc enable ret=%d\n", __func__, SIM_TEST_TAG, ret);
		usleep_range(1000, 1500);
	}
}

static void simcard_test_power_off(void)
{
	int ret;
	struct dw_mci *host = host_from_sd_module;

	if (host && host->vqmmc) {
		ret = regulator_disable(host->vqmmc);
		pr_info("%s:%s vqmmc regulator disable ret=%d\n", __func__, SIM_TEST_TAG, ret);
		usleep_range(1000, 1500);
	}
#ifdef CONFIG_MMC_SIM_GPIO_EXCHANGE
	if (get_regulator_flag) {
		get_regulator_flag = 0;
		dw_mci_put_regulator(host);
	}
#endif
}

static void simcard_test_vmmc_up(void)
{
	int ret;
	struct dw_mci *host = host_from_sd_module;

	if (host && host->vmmc) {
		/* set vmmc to 1.80v */
		ret = regulator_set_voltage(host->vmmc, 1800000, 1800000);
		dev_info(host->dev, "%s %s vmmc set ret = %d\n", MUX_SDSIM_LOG_TAG, __func__, ret);

		if (host->vmmcmosen) {
			/* set vmmc to 3.0v */
			ret = regulator_set_voltage(host->vmmcmosen, 3000000, 3000000);
			dev_info(host->dev, "%s %s vmmcmosen set ret = %d\n", MUX_SDSIM_LOG_TAG, __func__, ret);
			ret = regulator_enable(host->vmmcmosen);
			dev_info(host->dev, "%s %s vmmcmosen enable ret = %d\n", MUX_SDSIM_LOG_TAG, __func__, ret);
			usleep_range(1000, 1500);
		} else {
			ret = regulator_enable(host->vmmc);
			dev_info(host->dev, "%s %s vmmc enable ret = %d\n", MUX_SDSIM_LOG_TAG, __func__, ret);
			usleep_range(1000, 1500);
		}
	}
}

static void simcard_test_vmmc_off(void)
{
	int ret;
	struct dw_mci *host = host_from_sd_module;

	if (host && host->vmmc) {
		if (host->vmmcmosen) {
			ret = regulator_disable(host->vmmcmosen);
			dev_info(host->dev, "%s %s vmmcmosen disable ret = %d\n", MUX_SDSIM_LOG_TAG, __func__, ret);
		} else {
			ret = regulator_disable(host->vmmc);
			dev_info(host->dev, "%s %s vmmc disable ret = %d\n", MUX_SDSIM_LOG_TAG, __func__, ret);
		}
	}
}
static void nmcard_test_gpio_request(void)
{
	set_sd_sim_group_io_register(gpio_current_number_for_sd_clk, 0,
		PULL_TYPE_PD, DRIVER_STRENGTH_2MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_cmd, 0,
		PULL_TYPE_PD, DRIVER_STRENGTH_2MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data0,
		0, PULL_TYPE_PD, DRIVER_STRENGTH_2MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data1,
		0, PULL_TYPE_PD, DRIVER_STRENGTH_2MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data2,
		0, PULL_TYPE_PD, DRIVER_STRENGTH_2MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data3,
		0, PULL_TYPE_PD, DRIVER_STRENGTH_2MA_0);

	(void)gpio_request(gpio_current_number_for_sd_clk, "sd_clk");
	(void)gpio_request(gpio_current_number_for_sd_cmd, "sd_cmd");
	(void)gpio_request(gpio_current_number_for_sd_data0, "sd_data0");
	(void)gpio_request(gpio_current_number_for_sd_data1, "sd_data0");
	(void)gpio_request(gpio_current_number_for_sd_data2, "sd_data2");
	(void)gpio_request(gpio_current_number_for_sd_data3, "sd_data3");

	(void)gpio_direction_input(gpio_current_number_for_sd_clk);
	(void)gpio_direction_input(gpio_current_number_for_sd_cmd);
	(void)gpio_direction_input(gpio_current_number_for_sd_data0);
	(void)gpio_direction_input(gpio_current_number_for_sd_data1);
	(void)gpio_direction_input(gpio_current_number_for_sd_data2);
	(void)gpio_direction_input(gpio_current_number_for_sd_data3);
}

static void nmcard_test_gpio_free(void)
{
	(void)gpio_free(gpio_current_number_for_sd_clk);
	(void)gpio_free(gpio_current_number_for_sd_cmd);
	(void)gpio_free(gpio_current_number_for_sd_data0);
	(void)gpio_free(gpio_current_number_for_sd_data1);
	(void)gpio_free(gpio_current_number_for_sd_data2);
	(void)gpio_free(gpio_current_number_for_sd_data3);
}

static void nocard_test_result(int pin)
{
	int clk_value;
	int cmd_value;
	int data0_value;
	int data1_value;
	int data2_value;
	int data3_value;

	clk_value = gpio_get_value(gpio_current_number_for_sd_clk);
	cmd_value = gpio_get_value(gpio_current_number_for_sd_cmd);
	data0_value = gpio_get_value(gpio_current_number_for_sd_data0);
	data1_value = gpio_get_value(gpio_current_number_for_sd_data1);
	data2_value = gpio_get_value(gpio_current_number_for_sd_data2);
	data3_value = gpio_get_value(gpio_current_number_for_sd_data3);
	pr_err("%s:%s pin == %d, clk_value = %d, cmd_value = %d, data0_value = "
	       "%d, data1_value = %d, data2_value = %d, data3_value = %d\n",
		__func__, SIM_TEST_TAG, pin, clk_value, cmd_value, data0_value,
		data1_value, data2_value, data3_value);

	nocard_pin_value_result[pin][SIM_TEST_CLK] = clk_value;
	nocard_pin_value_result[pin][SIM_TEST_CMD] = cmd_value;
	nocard_pin_value_result[pin][SIM_TEST_DATA0] = data0_value;
	nocard_pin_value_result[pin][SIM_TEST_DATA1] = data1_value;
	nocard_pin_value_result[pin][SIM_TEST_DATA2] = data2_value;
	nocard_pin_value_result[pin][SIM_TEST_DATA3] = data3_value;
}

static void mockcard_test_result(int level)
{
	int clk_value;
	int cmd_value;
	int data0_value;
	int data1_value;
	int data2_value;
	int data3_value;

	clk_value = gpio_get_value(gpio_current_number_for_sd_clk);
	cmd_value = gpio_get_value(gpio_current_number_for_sd_cmd);
	data0_value = gpio_get_value(gpio_current_number_for_sd_data0);
	data1_value = gpio_get_value(gpio_current_number_for_sd_data1);
	data2_value = gpio_get_value(gpio_current_number_for_sd_data2);
	data3_value = gpio_get_value(gpio_current_number_for_sd_data3);
	pr_err("%s:%s level == %d, clk_value = %d, cmd_value = %d, data0_value "
	       "= %d, data1_value = %d, data2_value = %d, data3_value = %d\n",
		__func__, SIM_TEST_TAG, level, clk_value, cmd_value,
		data0_value, data1_value, data2_value, data3_value);

	mockcard_pin_value_result[level][SIM_TEST_CLK] = clk_value;
	mockcard_pin_value_result[level][SIM_TEST_CMD] = cmd_value;
	mockcard_pin_value_result[level][SIM_TEST_DATA0] = data0_value;
	mockcard_pin_value_result[level][SIM_TEST_DATA1] = data1_value;
	mockcard_pin_value_result[level][SIM_TEST_DATA2] = data2_value;
	mockcard_pin_value_result[level][SIM_TEST_DATA3] = data3_value;
}

static void nocard_test(int pin)
{
	nmcard_test_gpio_request();
	usleep_range(1000, 1500);

	if (pin == SIM_TEST_CLK) {
		set_sd_sim_group_io_register(gpio_current_number_for_sd_clk,
			0, PULL_TYPE_NP, DRIVER_STRENGTH_2MA_0);
		(void)gpio_direction_output(gpio_current_number_for_sd_clk, 1);
	} else if (pin == SIM_TEST_CMD) {
		set_sd_sim_group_io_register(gpio_current_number_for_sd_cmd,
			0, PULL_TYPE_NP, DRIVER_STRENGTH_2MA_0);
		(void)gpio_direction_output(gpio_current_number_for_sd_cmd, 1);
	} else if (pin == SIM_TEST_DATA0) {
		set_sd_sim_group_io_register(gpio_current_number_for_sd_data0,
			0, PULL_TYPE_NP, DRIVER_STRENGTH_2MA_0);
		(void)gpio_direction_output(
			gpio_current_number_for_sd_data0, 1);
	} else if (pin == SIM_TEST_DATA1) {
		set_sd_sim_group_io_register(gpio_current_number_for_sd_data1,
			0, PULL_TYPE_NP, DRIVER_STRENGTH_2MA_0);
		(void)gpio_direction_output(
			gpio_current_number_for_sd_data1, 1);
	} else if (pin == SIM_TEST_DATA2) {
		set_sd_sim_group_io_register(gpio_current_number_for_sd_data2,
			0, PULL_TYPE_NP, DRIVER_STRENGTH_2MA_0);
		(void)gpio_direction_output(
			gpio_current_number_for_sd_data2, 1);
	} else if (pin == SIM_TEST_DATA3) {
		set_sd_sim_group_io_register(gpio_current_number_for_sd_data3,
			0, PULL_TYPE_NP, DRIVER_STRENGTH_2MA_0);
		(void)gpio_direction_output(
			gpio_current_number_for_sd_data3, 1);
	}
	msleep(100);

	nocard_test_result(pin);
	nmcard_test_gpio_free();
}

static void mockcard_test(int level)
{
	int clk_value;
	int cmd_value;
	int data0_value;
	int data1_value;
	int data2_value;
	int data3_value;

	nmcard_test_gpio_request();
	usleep_range(1000, 1500);
	if (level == SIM_TEST_HIGH) {
		set_sd_sim_group_io_register(gpio_current_number_for_sd_clk,
			0, PULL_TYPE_NP, DRIVER_STRENGTH_6MA_0);
		(void)gpio_direction_output(gpio_current_number_for_sd_clk, 1);
		set_sd_sim_group_io_register(gpio_current_number_for_sd_data2,
			0, PULL_TYPE_NP, DRIVER_STRENGTH_2MA_0);
		(void)gpio_direction_output(
			gpio_current_number_for_sd_data2, 1);
		set_sd_sim_group_io_register(gpio_current_number_for_sd_data3,
			0, PULL_TYPE_NP, DRIVER_STRENGTH_2MA_0);
		(void)gpio_direction_output(
			gpio_current_number_for_sd_data3, 1);
	} else if (level == SIM_TEST_LOW) {
		set_sd_sim_group_io_register(gpio_current_number_for_sd_clk,
			0, PULL_TYPE_NP, DRIVER_STRENGTH_2MA_0);
		(void)gpio_direction_output(gpio_current_number_for_sd_clk, 0);
		set_sd_sim_group_io_register(gpio_current_number_for_sd_data2,
			0, PULL_TYPE_NP, DRIVER_STRENGTH_2MA_0);
		(void)gpio_direction_output(
			gpio_current_number_for_sd_data2, 0);
		set_sd_sim_group_io_register(gpio_current_number_for_sd_data3,
			0, PULL_TYPE_NP, DRIVER_STRENGTH_2MA_0);
		(void)gpio_direction_output(
			gpio_current_number_for_sd_data3, 0);
	} else if (level == SIM_TEST_VMMC) {
		simcard_test_vmmc_up();
	}
	msleep(100);

	mockcard_test_result(level);
	nmcard_test_gpio_free();

	if (level == SIM_TEST_VMMC)
		simcard_test_vmmc_off();
}
#endif

static ssize_t sd_test_reset_proc_write(struct file *p_file,
	const char __user *userbuf, size_t count, loff_t *ppos)
{
	char buf[SD_TEST_TYPE_SIZE] = {0};

	if (!count)
		return -1;

	if (!userbuf)
		return -1;

	if (copy_from_user(buf, userbuf, sizeof(buf) - 1))
		return -1;

	if (!strncmp("sd_reset", buf, strlen("sd_reset")))
		sd_test_reset_flag = 1;
	return (ssize_t)count;
}

#ifdef CONFIG_MMC_DW_MUX_SDSIM
static ssize_t simcard_test_proc_write(struct file *p_file,
	const char __user *userbuf, size_t count, loff_t *ppos)
{
	char buf[SD_TEST_TYPE_SIZE] = {0};

	if (!count)
		return -1;

	if (!userbuf)
		return -1;

	if (copy_from_user(buf, userbuf, sizeof(buf) - 1))
		return -1;

	if (!strncmp("NO_CARD", buf, strlen("NO_CARD")) &&
		sim_test_state != NOCARD_PROCESSING &&
		sim_test_state != MOCKCARD_PROCESSING) {
		sim_test_state = NOCARD_PROCESSING;
		simcard_test_power_up();
		simcard_test_vmmc_up();
		nocard_test(SIM_TEST_CLK);
		nocard_test(SIM_TEST_CMD);
		nocard_test(SIM_TEST_DATA0);
		nocard_test(SIM_TEST_DATA1);
		nocard_test(SIM_TEST_DATA2);
		nocard_test(SIM_TEST_DATA3);
		simcard_test_vmmc_off();
		simcard_test_power_off();
		sim_test_state = NOCARD_IDLE;
	} else if (!strncmp("MOCK_CARD", buf, strlen("MOCK_CARD")) &&
		sim_test_state != MOCKCARD_PROCESSING &&
		sim_test_state != NOCARD_PROCESSING) {
		sim_test_state = MOCKCARD_PROCESSING;
		simcard_test_power_up();
		mockcard_test(SIM_TEST_HIGH);
		mockcard_test(SIM_TEST_LOW);
		mockcard_test(SIM_TEST_VMMC);
		simcard_test_power_off();
		sim_test_state = MOCKCARD_IDLE;
	}
	return (ssize_t)count;
}
#endif

static int sd_modul_proc_open(struct inode *p_inode, struct file *p_file)
{
	return single_open(p_file, sd_modul_proc_show, NULL);
}

static int sd_reset_proc_open(struct inode *p_inode, struct file *p_file)
{
	return single_open(p_file, sd_reset_proc_show, NULL);
}

#ifdef CONFIG_MMC_DW_MUX_SDSIM
static int simcard_test_proc_open(struct inode *p_inode, struct file *p_file)
{
	return single_open(p_file, simcard_test_proc_show, NULL);
}
#endif

static const struct file_operations sd_modul_set_proc_fops = {
	.open = sd_modul_proc_open,
	.read = seq_read,
	.write = sd_modul_set_proc_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations sd_test_reset_proc_fops = {
	.open = sd_reset_proc_open,
	.read = seq_read,
	.write = sd_test_reset_proc_write,
	.llseek = seq_lseek,
	.release = single_release,
};

#ifdef CONFIG_MMC_DW_MUX_SDSIM
static const struct file_operations simcard_test_proc_fops = {
	.open = simcard_test_proc_open,
	.read = seq_read,
	.write = simcard_test_proc_write,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

void proc_sd_test_init(void)
{
	static int is_device_inited;

	if (is_device_inited == DEVICE_INIT)
		return;

	if (!proc_mkdir("sd_test_device", NULL)) {
		pr_err("make proc dir sd_test_device failed\n");
		return;
	}
#ifdef CONFIG_MMC_DW_MUX_SDSIM
	if (!proc_mkdir("sim_test_device", NULL)) {
		pr_err("make proc dir sim_test_device failed\n");
		return;
	}
#endif
	proc_create("sd_test_device/sd_modul_set", 0660,
		(struct proc_dir_entry *)NULL, &sd_modul_set_proc_fops);
	proc_create("sd_test_device/sd_test_reset", 0660,
		(struct proc_dir_entry *)NULL, &sd_test_reset_proc_fops);
#ifdef CONFIG_MMC_DW_MUX_SDSIM
	proc_create("sim_test_device/simcard_test", 0660,
		(struct proc_dir_entry *)NULL, &simcard_test_proc_fops);
	sim_test_state = IDLE;
#endif
	is_device_inited = DEVICE_INIT;
}
#endif

#if (!defined(CONFIG_MMC_DW_MUX_SDSIM) && !defined(CONFIG_MMC_SDHCI_MUX_SDSIM))
#define STATUS_SIM 5
int sd_sim_detect_run(void *dw_mci_host, int status, int current_module, int need_sleep)
{
	return status;
}
EXPORT_SYMBOL(sd_sim_detect_run);

int get_card1_type(void)
{
	return STATUS_SIM;
}
EXPORT_SYMBOL(get_card1_type);
#endif
#pragma GCC diagnostic pop
