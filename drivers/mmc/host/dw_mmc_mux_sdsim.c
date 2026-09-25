/*
 * add nano sd function.
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "dw_mmc_hisi.h"

#include <linux/mmc/dw_mmc_mux_sdsim.h>
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
#include <linux/irqdomain.h>
#include <linux/gpio.h>

#include "dw_mmc.h"
#include "dw_mmc-pltfm.h"

#define SIMHOTPLUG1_MODEM_DEV "/dev/simhotplug1"
#define SIMHOTPLUG_IOC_MAGIC 'k'
#define SIMHOTPLUG_IOC_INFORM_CARD_INOUT _IOWR(SIMHOTPLUG_IOC_MAGIC, 1, int32_t)
#define SIMHOTPLUG_SIM_CARD_IN 1

#define STATUS_SIM 5
#define STATUS_SD 6
#define STATUS_NO_CARD 7
#define STATUS_SD2JTAG 8

int switch_gpio_number_0 = SWITCH_GPIO_DEFAULT_NUMBER;
int switch_gpio_number_1 = SWITCH_GPIO_DEFAULT_NUMBER;
int switch_gpio_number_2 = SWITCH_GPIO_DEFAULT_NUMBER;
int switch_gpio_number_3 = SWITCH_GPIO_DEFAULT_NUMBER;

#define SWITCH_GPIO_SD_SIDE 1
#define SWITCH_GPIO_SIM_SIDE 0

struct semaphore sem_mux_sdsim_detect;

/*
 * sd_sim_group_io
 * NUMBER     FUNCTION0        FUNCTION1      FUNCTION4
 * GPIO160       GPIO           SD_CLK        USIM1_CLK
 * GPIO161       GPIO           SD_CMD            -
 * GPIO162       GPIO           SD_DATA0      USIM1_RST
 * GPIO163       GPIO           SD_DATA1      USIM1_DATA
 * GPIO164       GPIO           SD_DATA2          -
 * GPIO165       GPIO           SD_DATA3          -
 */
int gpio_current_number_for_sd_clk = GPIO_DEFAULT_NUMBER_FOR_SD_CLK;
int gpio_current_number_for_sd_cmd = GPIO_DEFAULT_NUMBER_FOR_SD_CMD;
int gpio_current_number_for_sd_data0 = GPIO_DEFAULT_NUMBER_FOR_SD_DATA0;
int gpio_current_number_for_sd_data1 = GPIO_DEFAULT_NUMBER_FOR_SD_DATA1;
int gpio_current_number_for_sd_data2 = GPIO_DEFAULT_NUMBER_FOR_SD_DATA2;
int gpio_current_number_for_sd_data3 = GPIO_DEFAULT_NUMBER_FOR_SD_DATA3;

#ifdef CONFIG_MMC_DW_MUX_SDSIM_LIBRA
int gpio_current_number_for_sim_clk = GPIO_DEFAULT_NUMBER_FOR_SIM_CLK;
int gpio_current_number_for_sim_rst = GPIO_DEFAULT_NUMBER_FOR_SIM_RST;
int gpio_current_number_for_sim_data = GPIO_DEFAULT_NUMBER_FOR_SIM_DATA;
#endif

int sd_sim_detect_status_current = SD_SIM_DETECT_STATUS_UNDETECTED;

/* 980 config here */
#define IOMG_BASE_GPIO_160 0xff37e000
#define IOCG_BASE_GPIO_160 0xff37e800
/* 990 config here */
#define IOMG_BASE_GPIO_104 0xf8580000
#define IOCG_BASE_GPIO_104 0xf8580800

unsigned int iomg_base_gpio_for_sd_clk = IOMG_BASE_GPIO_160;
unsigned int iocg_base_gpio_for_sd_clk = IOCG_BASE_GPIO_160;

#define FUNCTION0 0
#define FUNCTION1 1
#define FUNCTION2 2
#define FUNCTION3 3
#define FUNCTION4 4

#define PULL_TYPE_NP 0x00
#define PULL_TYPE_PU 0x01
#define PULL_TYPE_PD 0x02

int sd_clk_driver_strength = -1;
int sd_cmd_driver_strength = -1;
int sd_data_driver_strength = -1;

typedef struct detect_gpio_status_array {
	int sd_data0_and_sim1_rst_value;
	int sd_data2_value;
	int sd_cmd_and_sim1_vpp_value;
	int sd_data3_value;
	int actual_card_type;
} detect_gpio_status_array_s;


static detect_gpio_status_array_s g_array[] = {
	{GPIO_VALUE_LOW,  GPIO_VALUE_HIGH,  GPIO_VALUE_HIGH,   GPIO_VALUE_LOW,    SD_SIM_DETECT_STATUS_SIM },  /* status type 1  NORMAL_SIM for one type card slot,like LAYA */
	{GPIO_VALUE_LOW,  GPIO_VALUE_LOW,   GPIO_VALUE_HIGH,   GPIO_VALUE_HIGH,   SD_SIM_DETECT_STATUS_SD  },  /* status type 2  NORMAL_SD */
	{GPIO_VALUE_LOW,  GPIO_VALUE_LOW,   GPIO_VALUE_HIGH,   GPIO_VALUE_HIGH,   SD_SIM_DETECT_STATUS_SD  },  /* status type 2  SLOT_EMPTY,same status with NORMAL_SD,set as SD_SIM_DETECT_STATUS_SD */
	{GPIO_VALUE_HIGH, GPIO_VALUE_LOW,   GPIO_VALUE_LOW,    GPIO_VALUE_HIGH,   SD_SIM_DETECT_STATUS_SIM },  /* status type 3  ERROR_1_SIM */
	{GPIO_VALUE_LOW,  GPIO_VALUE_LOW,   GPIO_VALUE_HIGH,   GPIO_VALUE_HIGH,   SD_SIM_DETECT_STATUS_SD  },  /* status type 2  ERROR_1_SD */
	{GPIO_VALUE_HIGH, GPIO_VALUE_HIGH,  GPIO_VALUE_LOW,    GPIO_VALUE_LOW,    SD_SIM_DETECT_STATUS_SIM },  /* status type 4  ERROR_2_1_SIM */
	{GPIO_VALUE_LOW,  GPIO_VALUE_LOW,   GPIO_VALUE_HIGH,   GPIO_VALUE_HIGH,   SD_SIM_DETECT_STATUS_SD  },  /* status type 2  ERROR_2_1_SD */
	{GPIO_VALUE_LOW,  GPIO_VALUE_LOW,   GPIO_VALUE_HIGH,   GPIO_VALUE_HIGH,   SD_SIM_DETECT_STATUS_SIM },  /* status type 2  ERROR_2_2_SIM */
	{GPIO_VALUE_LOW,  GPIO_VALUE_LOW,   GPIO_VALUE_HIGH,   GPIO_VALUE_HIGH,   SD_SIM_DETECT_STATUS_SD  },  /* status type 2  ERROR_2_2_SD */
	{GPIO_VALUE_LOW,  GPIO_VALUE_HIGH,  GPIO_VALUE_HIGH,   GPIO_VALUE_HIGH,   SD_SIM_DETECT_STATUS_SIM }, /* status type 5  NORMAL_SIM for another type card slot,like HIMA */
};

#ifdef CONFIG_MMC_DW_MUX_SDSIM_LIBRA
static int set_sd_sim_gpio_direction(int gpio_num, const char *gpio_name, int direction, int power_level)
{
	/*
	 * direction: 0 = input, 1 = output;
	 * power_level: 0 = lowlevel; 1 = highlevel
	 */
	int err = -1;

	if (gpio_num < gpio_current_number_for_sd_clk ||
		gpio_num > gpio_current_number_for_sim_data) {
		pr_err("%s %s argument gpionumber=%d is invalid, just return\n",
			MUX_SDSIM_LOG_TAG, __func__, gpio_num);
		return err;
	}
	if (!gpio_name) {
		pr_err("%s %s argument gpio_name is null, just return\n", MUX_SDSIM_LOG_TAG, __func__);
		return err;
	}
	if (direction != 0 && direction != 1) {
		pr_err("%s %s argument direction=%d is invalid, just return\n",
			MUX_SDSIM_LOG_TAG, __func__, direction);
		return err;
	}
	if (power_level != 0 && power_level != 1) {
		pr_err("%s %s argument power_level=%d is invalid, just return\n",
			MUX_SDSIM_LOG_TAG, __func__, power_level);
		return err;
	}
	err = gpio_request(gpio_num, gpio_name);
	if (err < 0) {
		pr_err("%s %s gpio_request failed\n", MUX_SDSIM_LOG_TAG, __func__);
		goto out;
	}
	if (!direction) {
		err = gpio_direction_input(gpio_num);
		if (err)
			pr_err("%s %s gpio_direction_input failed", MUX_SDSIM_LOG_TAG, __func__);
	} else {
		err = gpio_direction_output(gpio_num, power_level);
		if (err)
			pr_err("%s %s gpio_direction_output failed", MUX_SDSIM_LOG_TAG, __func__);
	}
	gpio_free(gpio_num);

out:
	pr_info("%s %s set gpio%d(%s) %s(%s) %s err = %d", MUX_SDSIM_LOG_TAG,
		__func__, gpio_num, gpio_name, direction ? "output" : "input",
		power_level ? "high" : "low", (err < 0) ? "failed" : "success", err);
	return err;
}
#endif

void register_gpio_number_group(int start_gpio_number_for_sd_clk)
{
	if (start_gpio_number_for_sd_clk <= 0)
		return;
	/* sd function pin-num increases by 1 */
	gpio_current_number_for_sd_clk = start_gpio_number_for_sd_clk;
	gpio_current_number_for_sd_cmd = start_gpio_number_for_sd_clk + 1;
	gpio_current_number_for_sd_data0 = start_gpio_number_for_sd_clk + 2;
	gpio_current_number_for_sd_data1 = start_gpio_number_for_sd_clk + 3;
	gpio_current_number_for_sd_data2 = start_gpio_number_for_sd_clk + 4;
	gpio_current_number_for_sd_data3 = start_gpio_number_for_sd_clk + 5;

	if (start_gpio_number_for_sd_clk == GPIO_104) {
		iomg_base_gpio_for_sd_clk = IOMG_BASE_GPIO_104;
		iocg_base_gpio_for_sd_clk = IOCG_BASE_GPIO_104;
	} else if (start_gpio_number_for_sd_clk == GPIO_160) {
		iomg_base_gpio_for_sd_clk = IOMG_BASE_GPIO_160;
		iocg_base_gpio_for_sd_clk = IOCG_BASE_GPIO_160;
	}
}

void switch_gpio_change(int set_side_status)
{
	if (set_side_status == SWITCH_GPIO_SD_SIDE ||
		set_side_status == SWITCH_GPIO_SIM_SIDE) {
		if (switch_gpio_number_0 != SWITCH_GPIO_DEFAULT_NUMBER) {
			(void)gpio_request(switch_gpio_number_0,
				"nanocard_switch_gpio_number_0");
			gpio_direction_output(switch_gpio_number_0, set_side_status);
			gpio_set_value(switch_gpio_number_0, set_side_status);
			gpio_free(switch_gpio_number_0);
		}

		if (switch_gpio_number_1 != SWITCH_GPIO_DEFAULT_NUMBER) {
			(void)gpio_request(switch_gpio_number_1,
				"nanocard_switch_gpio_number_1");
			gpio_direction_output(switch_gpio_number_1, set_side_status);
			gpio_set_value(switch_gpio_number_1, set_side_status);
			gpio_free(switch_gpio_number_1);
		}

		if (switch_gpio_number_2 != SWITCH_GPIO_DEFAULT_NUMBER) {
			(void)gpio_request(switch_gpio_number_2,
				"nanocard_switch_gpio_number_2");
			gpio_direction_output(switch_gpio_number_2, set_side_status);
			gpio_set_value(switch_gpio_number_2, set_side_status);
			gpio_free(switch_gpio_number_2);
		}

		if (switch_gpio_number_3 != SWITCH_GPIO_DEFAULT_NUMBER) {
			(void)gpio_request(switch_gpio_number_3,
				"nanocard_switch_gpio_number_3");
			gpio_direction_output(switch_gpio_number_3, set_side_status);
			gpio_set_value(switch_gpio_number_3, set_side_status);
			gpio_free(switch_gpio_number_3);
		}
	}
}
EXPORT_SYMBOL(switch_gpio_change);

void set_sd_sim_group_io_register(
	int gpionumber, int function_select, int pulltype, int driverstrenth)
{
	unsigned int reg;
	unsigned int pulltype_this;
	unsigned int driverstrenth_this;
	static void *iomg_ctrl_reg;
	static void *iocg_ctrl_reg;

	pulltype_this = (unsigned int)pulltype;
	driverstrenth_this = (unsigned int)driverstrenth;

	if (unlikely(!iomg_ctrl_reg))
		iomg_ctrl_reg = ioremap(iomg_base_gpio_for_sd_clk, 0x1000);

	if (unlikely(!iocg_ctrl_reg))
		iocg_ctrl_reg = ioremap(iocg_base_gpio_for_sd_clk, 0x1000);

#ifdef CONFIG_MMC_DW_MUX_SDSIM_LIBRA
	if (gpionumber < gpio_current_number_for_sd_clk ||
		gpionumber > gpio_current_number_for_sim_data) {
#else
	if (gpionumber < gpio_current_number_for_sd_clk ||
		gpionumber > gpio_current_number_for_sd_data3) {
#endif
		pr_info("%s %s argument gpionumber=%d is invalid,just return\n",
			MUX_SDSIM_LOG_TAG, __func__, gpionumber);
		return;
	}
	/* iomg_ctrl_reg_current: iomg_ctrl_reg_base + 4 * (gpionumber_current-gpionumber_base) */
	writel(function_select, iomg_ctrl_reg +
		0x004 * (gpionumber - gpio_current_number_for_sd_clk));

	reg = readl(iocg_ctrl_reg + 0x004 * (gpionumber - gpio_current_number_for_sd_clk));
	/* bit0-2:pulltype, bit4-7:driverstrenth, clear them by &0x08 */
	reg = reg & 0x08;
	reg = reg | driverstrenth_this;
	reg = reg | pulltype_this;
	writel(reg, iocg_ctrl_reg + 0x004 * (gpionumber - gpio_current_number_for_sd_clk));
}

static void config_sdsim_mode_gpio_detect(void)
{
	pr_info("%s %s set SDSIM_MODE_GPIO_DETECT\n", MUX_SDSIM_LOG_TAG, __func__);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_clk, FUNCTION0,
		PULL_TYPE_NP, DRIVER_STRENGTH_2MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_cmd, FUNCTION0,
		PULL_TYPE_PU, DRIVER_STRENGTH_2MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data0,
		FUNCTION0, PULL_TYPE_PD, DRIVER_STRENGTH_2MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data1,
		FUNCTION0, PULL_TYPE_NP, DRIVER_STRENGTH_2MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data2,
		FUNCTION0, PULL_TYPE_PD, DRIVER_STRENGTH_2MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data3,
		FUNCTION0, PULL_TYPE_PU, DRIVER_STRENGTH_2MA_0);
}

static void config_sdsim_mode_sd_normal_detect(void)
{
	pr_info("%s %s set SDSIM_MODE_SD_NORMAL\n", MUX_SDSIM_LOG_TAG, __func__);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_clk, FUNCTION1,
		PULL_TYPE_NP, sd_clk_driver_strength);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_cmd, FUNCTION1,
		PULL_TYPE_PU, sd_cmd_driver_strength);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data0,
		FUNCTION1, PULL_TYPE_PU, sd_data_driver_strength);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data1,
		FUNCTION1, PULL_TYPE_PU, sd_data_driver_strength);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data2,
		FUNCTION1, PULL_TYPE_PU, sd_data_driver_strength);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data3,
		FUNCTION1, PULL_TYPE_PU, sd_data_driver_strength);
#ifdef CONFIG_MMC_DW_MUX_SDSIM_LIBRA
	set_sd_sim_group_io_register(gpio_current_number_for_sim_clk, FUNCTION0,
		PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sim_rst, FUNCTION0,
		PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sim_data,
		FUNCTION0, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	(void)set_sd_sim_gpio_direction(gpio_current_number_for_sim_clk, "sim_clk", 0, 0);
	(void)set_sd_sim_gpio_direction(gpio_current_number_for_sim_rst, "sim_rst", 0, 0);
	(void)set_sd_sim_gpio_direction(gpio_current_number_for_sim_data, "sim_data", 0, 0);
#endif
}

static void config_sdsim_mode_sd_idle_detect(void)
{
	pr_info("%s %s set SDSIM_MODE_SD_IDLE\n", MUX_SDSIM_LOG_TAG, __func__);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_clk, FUNCTION0,
		PULL_TYPE_NP, sd_clk_driver_strength);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_cmd, FUNCTION0,
		PULL_TYPE_NP, sd_cmd_driver_strength);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data0,
		FUNCTION0, PULL_TYPE_NP, sd_data_driver_strength);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data1,
		FUNCTION0, PULL_TYPE_NP, sd_data_driver_strength);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data2,
		FUNCTION0, PULL_TYPE_NP, sd_data_driver_strength);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data3,
		FUNCTION0, PULL_TYPE_NP, sd_data_driver_strength);
}

static void config_sdsim_mode_sim_normal_detect(void)
{
	pr_info("%s %s set SDSIM_MODE_SIM_NORMAL\n", MUX_SDSIM_LOG_TAG, __func__);
#ifdef CONFIG_MMC_DW_MUX_SDSIM_LIBRA
	set_sd_sim_group_io_register(gpio_current_number_for_sd_clk, FUNCTION0,
		PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_cmd, FUNCTION0,
		PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data0,
		FUNCTION0, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data1,
		FUNCTION0, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data2,
		FUNCTION0, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data3,
		FUNCTION0, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sim_clk, FUNCTION1,
		PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sim_rst, FUNCTION1,
		PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sim_data,
		FUNCTION1, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	(void)set_sd_sim_gpio_direction(gpio_current_number_for_sd_clk, "sd_clk", 0, 0);
	(void)set_sd_sim_gpio_direction(gpio_current_number_for_sd_cmd, "sd_cmd", 0, 0);
	(void)set_sd_sim_gpio_direction(gpio_current_number_for_sd_data0, "sd_data0", 0, 0);
	(void)set_sd_sim_gpio_direction(gpio_current_number_for_sd_data1, "sd_data1", 0, 0);
	(void)set_sd_sim_gpio_direction(gpio_current_number_for_sd_data2, "sd_data2", 0, 0);
	(void)set_sd_sim_gpio_direction(gpio_current_number_for_sd_data3, "sd_data3", 0, 0);
#else
	set_sd_sim_group_io_register(gpio_current_number_for_sd_clk, FUNCTION4,
		PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_cmd, FUNCTION0,
		PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data0,
		FUNCTION4, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data1,
		FUNCTION4, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data2,
		FUNCTION0, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data3,
		FUNCTION0, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
#endif
}

static void config_sdsim_mode_sim_idle_detect(void)
{
	pr_info("%s %s set SDSIM_MODE_SIM_IDLE\n", MUX_SDSIM_LOG_TAG, __func__);
#ifdef CONFIG_MMC_DW_MUX_SDSIM_LIBRA
	set_sd_sim_group_io_register(gpio_current_number_for_sd_clk, FUNCTION0,
		PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_cmd, FUNCTION0,
		PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data0,
		FUNCTION0, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data1,
		FUNCTION0, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data2,
		FUNCTION0, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data3,
		FUNCTION0, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sim_clk, FUNCTION1,
		PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sim_rst, FUNCTION1,
		PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sim_data,
		FUNCTION1, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
#else
	set_sd_sim_group_io_register(gpio_current_number_for_sd_clk, FUNCTION4,
		PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_cmd, FUNCTION0,
		PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data0,
		FUNCTION4, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data1,
		FUNCTION4, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data2,
		FUNCTION0, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
	set_sd_sim_group_io_register(gpio_current_number_for_sd_data3,
		FUNCTION0, PULL_TYPE_NP, DRIVER_STRENGTH_4MA_0);
#endif
}

/*
 * GpioMode/GpioNum             Func     PullType    Driver    SlewRate   GpioDirection   OutputValue
 *
 * SDSIM_MODE_GPIO_DETECT
 * GPIO_160                   Function0      NP       25mA-1      -            -              -
 * GPIO_161                   Function0      PU       2mA-0       N            I              - (detect pin)
 * GPIO_162                   Function0      PD       2mA-0       N            I              - (detect pin)
 * GPIO_163                   Function0      NP       11mA-1      -            -              -
 * GPIO_164                   Function0      PD       2mA-0       N            I              - (detect pin)
 * GPIO_165                   Function0      PU       2mA-0       N            I              - (detect pin)
 *
 *
 * SDSIM_MODE_SD_NORMAL
 * GPIO_160                   Function1      NP       25mA-1      -            -              -
 * GPIO_161                   Function1      PU       11mA-1      -            -              -
 * GPIO_162                   Function1      PU       11mA-1      -            -              -
 * GPIO_163                   Function1      PU       11mA-1      -            -              -
 * GPIO_164                   Function1      PU       11mA-1      -            -              -
 * GPIO_165                   Function1      PU       11mA-1      -            -              -
 *
 *
 * SDSIM_MODE_SD_IDLE
 * GPIO_160                   Function0      NP       25mA-1      -            -              -
 * GPIO_161                   Function0      NP       11mA-1      -            -              -
 * GPIO_162                   Function0      NP       11mA-1      -            -              -
 * GPIO_163                   Function0      NP       11mA-1      -            -              -
 * GPIO_164                   Function0      NP       11mA-1      -            -              -
 * GPIO_165                   Function0      NP       11mA-1      -            -              -
 *
 *
 * SDSIM_MODE_SIM_NORMAL
 * GPIO_160                   Function4      NP       4mA-0       N            -              -
 * GPIO_161                   Function0      NP       11mA-1      -            -              -  (sim not use this)
 * GPIO_162                   Function4      NP       4mA-0       N            -              -
 * GPIO_163                   Function4      NP       4mA-0       N            -              -
 * GPIO_164                   Function0      NP       11mA-1      -            -              -  (sim not use this)
 * GPIO_165                   Function0      NP       11mA-1      -            -              -  (sim not use this)
 *
 *
 *
 * SDSIM_MODE_SIM_IDLE
 * GPIO_160                   Function4      NP       4mA-0       N            -              -
 * GPIO_161                   Function0      NP       11mA-1      -            -              -  (sim not use this)
 * GPIO_162                   Function4      NP       4mA-0       N            -              -
 * GPIO_163                   Function4      NP       4mA-0       N            -              -
 * GPIO_164                   Function0      NP       11mA-1      -            -              -  (sim not use this)
 * GPIO_165                   Function0      NP       11mA-1      -            -              -  (sim not use this)
 */
int config_sdsim_gpio_mode(enum sdsim_gpio_mode gpio_mode)
{
	if (sd_clk_driver_strength == -1) {
		sd_clk_driver_strength = SD_CLK_DRIVER_DEFAULT;
		pr_info("%s %s sd_clk_driver_strength set as default = 0x%x\n",
			MUX_SDSIM_LOG_TAG, __func__, sd_clk_driver_strength);
	}

	if (sd_cmd_driver_strength == -1) {
		sd_cmd_driver_strength = SD_CMD_DRIVER_DEFAULT;
		pr_info("%s %s sd_cmd_driver_strength set as default = 0x%x\n",
			MUX_SDSIM_LOG_TAG, __func__, sd_cmd_driver_strength);
	}

	if (sd_data_driver_strength == -1) {
		sd_data_driver_strength = SD_DATA_DRIVER_DEFAULT;
		pr_info("%s %s sd_data_driver_strength set as default = 0x%x\n",
			MUX_SDSIM_LOG_TAG, __func__, sd_data_driver_strength);
	}

	if (gpio_mode == SDSIM_MODE_GPIO_DETECT)
		config_sdsim_mode_gpio_detect();
	else if (gpio_mode == SDSIM_MODE_SD_NORMAL)
		config_sdsim_mode_sd_normal_detect();
	else if (gpio_mode == SDSIM_MODE_SD_IDLE)
		config_sdsim_mode_sd_idle_detect();
	else if (gpio_mode == SDSIM_MODE_SIM_NORMAL)
		config_sdsim_mode_sim_normal_detect();
	else if (gpio_mode == SDSIM_MODE_SIM_IDLE)
		config_sdsim_mode_sim_idle_detect();
	return 0;
}
EXPORT_SYMBOL(config_sdsim_gpio_mode);

char *detect_status_to_string(void)
{
	switch (sd_sim_detect_status_current) {
	case SD_SIM_DETECT_STATUS_UNDETECTED:
		return "DETECT_STATUS_UNDETECTED";

	case SD_SIM_DETECT_STATUS_SD:
		return "DETECT_STATUS_SD";

	case SD_SIM_DETECT_STATUS_SIM:
		return "DETECT_STATUS_SIM";

	case SD_SIM_DETECT_STATUS_ERROR:
	default:
		return "DETECT_STATUS_ERROR";
	}
}

/*
 * temp value for work around after modem set raw register here,
 * but ap can't set same value again
 */
static void vdd_vcc_disable(struct dw_mci *host)
{
	int ret;

	if ((!host) || !(host->vqmmc))
		return;

	ret = regulator_disable(host->vqmmc);
	pr_info("%s %s vqmmc regulator disable ret=%d\n", MUX_SDSIM_LOG_TAG, __func__, ret);

	if (!host->vmmc)
		return;

	if (!host->vmmcmosen) {
		ret = regulator_disable(host->vmmc);
		pr_info("%s %s vmmc regulator disable ret=%d\n", MUX_SDSIM_LOG_TAG, __func__, ret);
	}

	if (!host->vmmcmosen)
		return;

	ret = regulator_disable(host->vmmcmosen);
	pr_info("%s %s vmmcmosen regulator disable ret=%d\n", MUX_SDSIM_LOG_TAG, __func__, ret);

}

static void vdd_vcc_enable(struct dw_mci *host)
{
	int ret;

	if (host->vqmmc) {
		/*
		 * temp value for work around after modem set raw register
		 * here,but ap can't set same value again
		 */
#ifdef CONFIG_MMC_DW_MUX_SDSIM_LIBRA
		/* set vqmmc to 1.75v first */
		ret = regulator_set_voltage(host->vqmmc, 1750000, 1750000);
		pr_info("%s %s vqmmc set 1.75v ret=%d\n", MUX_SDSIM_LOG_TAG, __func__, ret);
#else
		ret = regulator_set_voltage(host->vqmmc, 1775000, 1775000);
		pr_info("%s %s vqmmc set 1.775v ret=%d\n", MUX_SDSIM_LOG_TAG, __func__, ret);
#endif
		ret = regulator_set_voltage(host->vqmmc, 1800000, 1800000);
		pr_info("%s %s vqmmc set 1.8v ret=%d\n", MUX_SDSIM_LOG_TAG, __func__, ret);
		ret = regulator_enable(host->vqmmc);
		pr_info("%s %s vqmmc enable ret=%d\n", MUX_SDSIM_LOG_TAG, __func__, ret);
		usleep_range(1000, 1500);
	}

	if (host->vmmc) {
		/*
		 * temp value for work around after modem set raw register
		 * here,but ap can't set same value again
		 */
		ret = regulator_set_voltage(host->vmmc, 1850000, 1850000);
		pr_info("%s %s vmmc set 1.85v ret=%d\n", MUX_SDSIM_LOG_TAG, __func__, ret);

		ret = regulator_set_voltage(host->vmmc, 1800000, 1800000);
		pr_info("%s %s vmmc set 1.8v ret=%d\n", MUX_SDSIM_LOG_TAG, __func__, ret);

		if (!host->vmmcmosen) {
			ret = regulator_enable(host->vmmc);
			pr_info("%s %s vmmc enable ret=%d\n", MUX_SDSIM_LOG_TAG, __func__, ret);
		}
		usleep_range(1000, 1500);
	}

	if (host->vmmcmosen) {
		/*
		 * temp value for work around after modem set raw register
		 * here,but ap can't set same value again
		 */
		ret = regulator_set_voltage(host->vmmcmosen, 2950000, 2950000);
		pr_info("%s %s vmmcmosen set 2.95v ret=%d\n", MUX_SDSIM_LOG_TAG, __func__, ret);

		ret = regulator_set_voltage(host->vmmcmosen, 3000000, 3000000);
		pr_info("%s %s vmmcmosen set 3.00v ret=%d\n", MUX_SDSIM_LOG_TAG, __func__, ret);

		ret = regulator_enable(host->vmmcmosen);
		pr_info("%s %s vmmcmosen enable ret=%d\n", MUX_SDSIM_LOG_TAG, __func__, ret);
		usleep_range(1000, 1500);
	}
}

static void sd_sim_set_4pin_gpio_mode(void)
{
	(void)gpio_request(gpio_current_number_for_sd_cmd, "sd_cmd_and_sim1_vpp");
	(void)gpio_request(gpio_current_number_for_sd_data0, "sd_data0_and_sim1_rst");
	(void)gpio_request(gpio_current_number_for_sd_data2, "sd_data2");
	(void)gpio_request(gpio_current_number_for_sd_data3, "sd_data3");

	(void)gpio_direction_input(gpio_current_number_for_sd_cmd);
	(void)gpio_direction_input(gpio_current_number_for_sd_data0);
	(void)gpio_direction_input(gpio_current_number_for_sd_data2);
	(void)gpio_direction_input(gpio_current_number_for_sd_data3);

	gpio_free(gpio_current_number_for_sd_cmd);
	gpio_free(gpio_current_number_for_sd_data0);
	gpio_free(gpio_current_number_for_sd_data2);
	gpio_free(gpio_current_number_for_sd_data3);

	usleep_range(1000, 1500);
}

static void sd_sim_get_4pin_state(struct dw_mci *host, detect_gpio_status_array_s *current_state)
{
#ifdef CONFIG_MMC_SIM_GPIO_EXCHANGE
	int get_regulator_flag = 0;

	if (host->need_get_mmc_regulator) {
		dw_mci_get_regulator(host);
		get_regulator_flag = 1;
	}
#endif
	vdd_vcc_enable(host);

	sd_sim_set_4pin_gpio_mode();

	msleep(40);

	current_state->sd_cmd_and_sim1_vpp_value = gpio_get_value(gpio_current_number_for_sd_cmd);
	current_state->sd_data0_and_sim1_rst_value = gpio_get_value(gpio_current_number_for_sd_data0);
	current_state->sd_data2_value = gpio_get_value(gpio_current_number_for_sd_data2);
	current_state->sd_data3_value = gpio_get_value(gpio_current_number_for_sd_data3);

	pr_info("%s %s <GPIO %d> sd_data0_and_sim1_rst_value=%d\n",
		MUX_SDSIM_LOG_TAG, __func__, gpio_current_number_for_sd_data0,
		current_state->sd_data0_and_sim1_rst_value);
	pr_info("%s %s <GPIO %d> sd_data2_value=%d\n", MUX_SDSIM_LOG_TAG,
		__func__, gpio_current_number_for_sd_data2, current_state->sd_data2_value);
	pr_info("%s %s <GPIO %d> sd_cmd_and_sim1_vpp_value=%d\n",
		MUX_SDSIM_LOG_TAG, __func__, gpio_current_number_for_sd_cmd,
		current_state->sd_cmd_and_sim1_vpp_value);
	pr_info("%s %s <GPIO %d> sd_data3_value=%d\n", MUX_SDSIM_LOG_TAG,
		__func__, gpio_current_number_for_sd_data3, current_state->sd_data3_value);

	msleep(40);
	pr_info("%s %s RE READ GPIO status after sleep 40 ms\n", MUX_SDSIM_LOG_TAG, __func__);

	current_state->sd_cmd_and_sim1_vpp_value = gpio_get_value(gpio_current_number_for_sd_cmd);
	current_state->sd_data0_and_sim1_rst_value = gpio_get_value(gpio_current_number_for_sd_data0);
	current_state->sd_data2_value = gpio_get_value(gpio_current_number_for_sd_data2);
	current_state->sd_data3_value = gpio_get_value(gpio_current_number_for_sd_data3);

	pr_info("%s %s <GPIO %d> sd_data0_and_sim1_rst_value=%d\n",
		MUX_SDSIM_LOG_TAG, __func__, gpio_current_number_for_sd_data0,
		current_state->sd_data0_and_sim1_rst_value);
	pr_info("%s %s <GPIO %d> sd_data2_value=%d\n", MUX_SDSIM_LOG_TAG,
		__func__, gpio_current_number_for_sd_data2, current_state->sd_data2_value);
	pr_info("%s %s <GPIO %d> sd_cmd_and_sim1_vpp_value=%d\n",
		MUX_SDSIM_LOG_TAG, __func__, gpio_current_number_for_sd_cmd,
		current_state->sd_cmd_and_sim1_vpp_value);
	pr_info("%s %s <GPIO %d> sd_data3_value=%d\n", MUX_SDSIM_LOG_TAG,
		__func__, gpio_current_number_for_sd_data3, current_state->sd_data3_value);

	msleep(40);

	vdd_vcc_disable(host);
#ifdef CONFIG_MMC_SIM_GPIO_EXCHANGE
	if (get_regulator_flag) {
		dw_mci_put_regulator(host);
		get_regulator_flag = 0;
	}
#endif
}

static int sd_sim_4pin_detect(int current_module, detect_gpio_status_array_s *current_state)
{
	int i;

	for (i = 0; i < (int)(sizeof(g_array) / sizeof(detect_gpio_status_array_s)); i++) {
		if ((current_state->sd_cmd_and_sim1_vpp_value == g_array[i].sd_cmd_and_sim1_vpp_value) &&
			(current_state->sd_data0_and_sim1_rst_value == g_array[i].sd_data0_and_sim1_rst_value) &&
			(current_state->sd_data2_value == g_array[i].sd_data2_value) &&
			(current_state->sd_data3_value == g_array[i].sd_data3_value)) {
			pr_info("%s %s GPIO_STATUS_LIST compare ok when i = %d\n", MUX_SDSIM_LOG_TAG, __func__, i);

			if (g_array[i].actual_card_type == SD_SIM_DETECT_STATUS_SIM) {
#ifndef CONFIG_HISI_ESIM
				switch_gpio_change(SWITCH_GPIO_SIM_SIDE);
				config_sdsim_gpio_mode(SDSIM_MODE_SIM_NORMAL);
#endif
				sd_sim_detect_status_current = SD_SIM_DETECT_STATUS_SIM;

				pr_info("%s %s SIM is inserted and detected now,go with SIM\n", MUX_SDSIM_LOG_TAG, __func__);

				if (current_module == MODULE_SD)
					return STATUS_PLUG_OUT;
				else if (current_module == MODULE_SIM)
					return STATUS_PLUG_IN;

			} else if (g_array[i].actual_card_type == SD_SIM_DETECT_STATUS_SD) {
				switch_gpio_change(SWITCH_GPIO_SD_SIDE);
				sd_sim_detect_status_current = SD_SIM_DETECT_STATUS_SD;

				pr_info("%s %s SD is inserted and detected now(4-PIN status success),go with SD\n",
					MUX_SDSIM_LOG_TAG, __func__);

				if (current_module == MODULE_SD)
					return STATUS_PLUG_IN;
				else if (current_module == MODULE_SIM)
					return STATUS_PLUG_OUT;
			}
		}
	}

	/* 4pin cannot find final result */
	pr_info("%s %s SD_SIM_DETECT_STATUS_ERROR detected,but just go with SIM\n", MUX_SDSIM_LOG_TAG, __func__);
#ifndef CONFIG_HISI_ESIM
	switch_gpio_change(SWITCH_GPIO_SIM_SIDE);
	config_sdsim_gpio_mode(SDSIM_MODE_SIM_NORMAL);
#endif
	sd_sim_detect_status_current = SD_SIM_DETECT_STATUS_SIM;

	if (current_module == MODULE_SD)
		return STATUS_PLUG_OUT;
	else
		return STATUS_PLUG_IN;
}

static int sd_sim_detect_cmd_status(struct dw_mci *host, int current_module,
			int cmd1_result, struct mmc_host *mmc_host_temp, detect_gpio_status_array_s *current_state)
{
	int result = 0;

	if (!cmd1_result) {
		switch_gpio_change(SWITCH_GPIO_SD_SIDE);
		sd_sim_detect_status_current = SD_SIM_DETECT_STATUS_SD;

		pr_info("%s %s SD is inserted and detected now(CMD1 success),go with SD\n",
			MUX_SDSIM_LOG_TAG, __func__);

		if (current_module == MODULE_SD) {
			result = STATUS_PLUG_IN;
			goto out;
		} else if (current_module == MODULE_SIM) {
			result = STATUS_PLUG_OUT;
			goto out;
		}
	} else if (!(host->flags & PINS_DETECT_ENABLE)) {
#ifndef CONFIG_HISI_ESIM
		switch_gpio_change(SWITCH_GPIO_SIM_SIDE);
		config_sdsim_gpio_mode(SDSIM_MODE_SIM_NORMAL);
#endif
		sd_sim_detect_status_current = SD_SIM_DETECT_STATUS_SIM;

		pr_info("%s %s SIM is inserted and detected now(CMD1 fail),go with SIM\n",
			MUX_SDSIM_LOG_TAG, __func__);

		if (current_module == MODULE_SD) {
			result = STATUS_PLUG_OUT;
			goto out;
		} else if (current_module == MODULE_SIM) {
			result = STATUS_PLUG_IN;
			goto out;
		}
	} else {
		result = sd_sim_4pin_detect(current_module, current_state);
		goto out;
	}

out:
	return result;
}

int sd_sim_send_cmd_detect(struct dw_mci *host, int current_module)
{
	struct dw_mci_slot *cur_slot = host->slot[0];
	struct mmc_host *mmc_host_temp = cur_slot->mmc;
	struct dw_mci_hs_priv_data *priv_temp = host->priv;
	unsigned long former_present_flag;
	int cmd1_result, result;
	detect_gpio_status_array_s current_state;

	mmc_claim_host(mmc_host_temp);
	down(&sem_mux_sdsim_detect);
	config_sdsim_gpio_mode(SDSIM_MODE_GPIO_DETECT);

	switch_gpio_change(SWITCH_GPIO_SD_SIDE);

	sd_sim_get_4pin_state(host, &current_state);

	pr_info("%s %s enter CMD1-RESPONSE STATUS detect stage "
		"after sleep 20 ms\n", MUX_SDSIM_LOG_TAG, __func__);

	former_present_flag = test_bit(DW_MMC_CARD_PRESENT, &cur_slot->flags);

	pr_info("%s %s former_present_flag = %lu\n",
		MUX_SDSIM_LOG_TAG, __func__, former_present_flag);

	set_bit(DW_MMC_CARD_PRESENT, &cur_slot->flags);

	priv_temp->mux_sdsim_vcc_status = MUX_SDSIM_VCC_STATUS_1_8_0_V;
	cmd1_result = mmc_detect_sd_or_mmc(mmc_host_temp);

	if (!former_present_flag)
		clear_bit(DW_MMC_CARD_PRESENT, &cur_slot->flags);
	else
		set_bit(DW_MMC_CARD_PRESENT, &cur_slot->flags);

	pr_info("%s %s mmc_detect_sd_or_mmc() cmd1_result = %d\n",
		MUX_SDSIM_LOG_TAG, __func__, cmd1_result);
	priv_temp->mux_sdsim_vcc_status = MUX_SDSIM_VCC_STATUS_2_9_5_V;

	msleep(20);
	pr_info("%s %s enter OVER-ALL STATUS detect stage after sleep 20 ms\n",
		MUX_SDSIM_LOG_TAG, __func__);

	result = sd_sim_detect_cmd_status(host, current_module, cmd1_result, mmc_host_temp, &current_state);
	up(&sem_mux_sdsim_detect);
	mmc_release_host(mmc_host_temp);

	return result;
}

static int sim_plug_in(void)
{
	struct dw_mci *host = host_from_sd_module;
	int result = 0;

	if (!host) {
		pr_info("%s %s host=NULL,current_module=%d,This is Error,maybe "
			"MODULE_SIM,just return STATUS_PLUG_IN and let SIM go on\n",
			MUX_SDSIM_LOG_TAG, __func__, MODULE_SIM);
		/* This is sim */
		return STATUS_PLUG_IN;
	}

	down(&sem_mux_sdsim_detect);
	if (sd_sim_detect_status_current == SD_SIM_DETECT_STATUS_SD) {
		pr_info("%s %s MODULE_SIM find SD card already detected,just "
			"return STATUS_PLUG_OUT and do nothing\n", MUX_SDSIM_LOG_TAG, __func__);
		result = STATUS_PLUG_OUT;
		goto out;
	}

	if (sd_sim_detect_status_current == SD_SIM_DETECT_STATUS_SIM) {
		pr_info("%s %s current_module=%d now re-entered but same detect status,"
			"just return STATUS_PLUG_IN and do nothing\n",
			 MUX_SDSIM_LOG_TAG, __func__, MODULE_SIM);
		result = STATUS_PLUG_IN;
		goto out;
	}
	up(&sem_mux_sdsim_detect);
	return sd_sim_send_cmd_detect(host, MODULE_SIM);

out:
	up(&sem_mux_sdsim_detect);
	return result;
}

static int sd_plug_in(void *dw_mci_host)
{
	struct dw_mci *host = dw_mci_host;
	struct dw_mci_hs_priv_data *priv_temp = host->priv;
	int result;

	if (host->hw_mmc_id != DWMMC_SD_ID) {
		pr_info("%s %s for those non DWMMC_SD_ID device,just return former status value\n",
			MUX_SDSIM_LOG_TAG, __func__);
		return STATUS_PLUG_IN;
	}

	if (!priv_temp->mux_sdsim) {
		pr_info("%s %s for device mux_sdsim not enabled,just return former status value\n",
			MUX_SDSIM_LOG_TAG, __func__);
		return STATUS_PLUG_IN;
	}

	down(&sem_mux_sdsim_detect);
	if (sd_sim_detect_status_current == SD_SIM_DETECT_STATUS_SD) {
		pr_info("%s %s current_module=%d now re-entered but same detect status,"
			"just return STATUS_PLUG_IN and do nothing\n",
			 MUX_SDSIM_LOG_TAG, __func__, MODULE_SD);
		result = STATUS_PLUG_IN;
		goto out;
	}

	if (sd_sim_detect_status_current == SD_SIM_DETECT_STATUS_SIM) {
		pr_info("%s %s MODULE_SD find SIM card already "
			"detected,just return STATUS_PLUG_OUT and do nothing\n",
			MUX_SDSIM_LOG_TAG, __func__);
		result = STATUS_PLUG_OUT;
		goto out;
	}
	up(&sem_mux_sdsim_detect);
	return sd_sim_send_cmd_detect(host, MODULE_SD);

out:
	up(&sem_mux_sdsim_detect);
	return result;
}

/*
 * For plug out event,just update
 * sd_sim_detect_status_current here
 */
static int sd_sim_plug_out(void)
{
		down(&sem_mux_sdsim_detect);
		if (SD_SIM_DETECT_STATUS_UNDETECTED !=
			sd_sim_detect_status_current) {
			config_sdsim_gpio_mode(SDSIM_MODE_GPIO_DETECT);
			sd_sim_detect_status_current = SD_SIM_DETECT_STATUS_UNDETECTED;
		}

		pr_info("%s %s For plug out event,update "
		       "sd_sim_detect_status_current here\n",
			MUX_SDSIM_LOG_TAG, __func__);

		if (dw_mci_check_himntn(HIMNTN_SD2JTAG) ||
			dw_mci_check_himntn(HIMNTN_SD2DJTAG))
			switch_gpio_change(SWITCH_GPIO_SD_SIDE);
		else
			switch_gpio_change(SWITCH_GPIO_SIM_SIDE);

		up(&sem_mux_sdsim_detect);
		return STATUS_PLUG_OUT;
}

int sd_sim_detect_run(void *dw_mci_host, int status, int current_module, int need_sleep)
{
	pr_info("%s %s version 3.4\n", MUX_SDSIM_LOG_TAG, __func__);
	pr_info("%s %s argument list: status = %d current_module = %d "
	       "sd_sim_detect_status_current = %d need_sleep = %d\n",
		MUX_SDSIM_LOG_TAG, __func__, status, current_module,
		sd_sim_detect_status_current, need_sleep);

	if (status == STATUS_PLUG_OUT)
		return sd_sim_plug_out();

	if (current_module == MODULE_SIM)
		return sim_plug_in();

	if (current_module == MODULE_SD)
		return sd_plug_in(dw_mci_host);

	return status;
}
EXPORT_SYMBOL(sd_sim_detect_run);

static int kernel_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	mm_segment_t oldfs = get_fs();
	int error = -ENOTTY;

	if (!filp || !filp->f_op || !filp->f_op->unlocked_ioctl) {
		pr_err("%s %s NULL pointer error\n", MUX_SDSIM_LOG_TAG, __func__);
		return error;
	}

	set_fs(KERNEL_DS);
	error = filp->f_op->unlocked_ioctl(filp, cmd, arg);
	set_fs(oldfs);
	pr_debug("%s %s unlocked_ioctl error = %d\n", MUX_SDSIM_LOG_TAG, __func__, error);
	if (error == -ENOIOCTLCMD)
		error = -ENOTTY;

	return error;
}

static void notify_sim1_card_in(void)
{
	struct file *filp = NULL;
	int ret;
	int32_t ioc_arg = SIMHOTPLUG_SIM_CARD_IN;

	pr_debug("%s %s enter\n", MUX_SDSIM_LOG_TAG, __func__);
	filp = filp_open(SIMHOTPLUG1_MODEM_DEV, O_RDWR, 0);
	if (IS_ERR(filp)) {
		pr_err("%s %s open SIMHOTPLUG1_MODEM_DEV err return = %ld\n",
			MUX_SDSIM_LOG_TAG, __func__, PTR_ERR(filp));
		return;
	}
	pr_debug("%s %s open SIMHOTPLUG1_MODEM_DEV success\n", MUX_SDSIM_LOG_TAG, __func__);

	ret = kernel_ioctl(filp, SIMHOTPLUG_IOC_INFORM_CARD_INOUT, (uintptr_t)&ioc_arg);

	pr_info("%s %s call kernel_ioctl ret = %d\n", MUX_SDSIM_LOG_TAG, __func__, ret);

	ret = filp_close(filp, 0);

	pr_info("%s %s call filp_close ret = %d\n", MUX_SDSIM_LOG_TAG, __func__, ret);
}

void notify_sim_while_sd_success(struct mmc_host *mmc)
{
	struct dw_mci_slot *slot = NULL;
	struct dw_mci *host = NULL;
	struct dw_mci_hs_priv_data *priv_data = NULL;

	down(&sem_mux_sdsim_detect);

	slot = mmc_priv(mmc);
	host = slot->host;
	priv_data = host->priv;

	pr_info("%s %s enter sd_sim_detect_status_current = %d\n",
		MUX_SDSIM_LOG_TAG, __func__, sd_sim_detect_status_current);
	if (host->hw_mmc_id != DWMMC_SD_ID) {
		pr_info("%s %s for those non DWMMC_SD_ID device,just return\n",
			MUX_SDSIM_LOG_TAG, __func__);
		up(&sem_mux_sdsim_detect);
		return;
	}

	if (!priv_data->mux_sdsim) {
		pr_info("%s %s for device mux_sdsim not enabled,just return\n",
			MUX_SDSIM_LOG_TAG, __func__);
		up(&sem_mux_sdsim_detect);
		return;
	}
	if (sd_sim_detect_status_current == SD_SIM_DETECT_STATUS_SD)
		notify_sim1_card_in();
	else
		pr_info("%s %s SD card init success now,sd_sim_detect_status_current = %d but not "
		       "SD_SIM_DETECT_STATUS_SD,no need retry with SIM,just return\n",
			MUX_SDSIM_LOG_TAG, __func__, sd_sim_detect_status_current);

	up(&sem_mux_sdsim_detect);
}

void notify_sim_while_sd_fail(struct mmc_host *mmc)
{
	struct dw_mci_slot *slot = NULL;
	struct dw_mci *host = NULL;
	struct dw_mci_hs_priv_data *priv_data = NULL;

	down(&sem_mux_sdsim_detect);

	slot = mmc_priv(mmc);
	host = slot->host;
	priv_data = host->priv;

	pr_info("%s %s enter\n", MUX_SDSIM_LOG_TAG, __func__);

	if (host->hw_mmc_id != DWMMC_SD_ID) {
		pr_info("%s %s for those non DWMMC_SD_ID device,just return\n",
			MUX_SDSIM_LOG_TAG, __func__);
		up(&sem_mux_sdsim_detect);
		return;
	}

	if (!priv_data->mux_sdsim) {
		pr_info("%s %s for device mux_sdsim not enabled,just return\n",
			MUX_SDSIM_LOG_TAG, __func__);
		up(&sem_mux_sdsim_detect);
		return;
	}

	if (sd_sim_detect_status_current == SD_SIM_DETECT_STATUS_SD) {
		pr_info("%s %s SD card init fail now,retry go with SIM\n",
			MUX_SDSIM_LOG_TAG, __func__);
#ifndef CONFIG_HISI_ESIM
		switch_gpio_change(SWITCH_GPIO_SIM_SIDE);
		config_sdsim_gpio_mode(SDSIM_MODE_SIM_NORMAL);
#endif
		sd_sim_detect_status_current = SD_SIM_DETECT_STATUS_SIM;

		notify_sim1_card_in();
	} else {
		pr_info("%s %s SD card init fail now, sd_sim_detect_status_current = %d,"
		       "but not SD_SIM_DETECT_STATUS_SD,no need retry with SIM,just return\n",
			MUX_SDSIM_LOG_TAG, __func__, sd_sim_detect_status_current);
	}
	up(&sem_mux_sdsim_detect);
}

int get_sd2jtag_status(void)
{
	return (dw_mci_check_himntn(HIMNTN_SD2JTAG) == 1 ||
		dw_mci_check_himntn(HIMNTN_SD2DJTAG) == 1 ||
		dw_mci_check_himntn(HIMNTN_SD2UART6) == 1);
}
EXPORT_SYMBOL(get_sd2jtag_status);

int get_card1_type(void)
{
	u8 status = STATUS_SIM;
#ifdef CONFIG_MMC_DW_MUX_SDSIM
	int jtag_enable;

	jtag_enable = get_sd2jtag_status();
	if (jtag_enable == 1) {
		pr_info("%s: sd2jtag_enable, status = STATUS_SD2JTAG\n", __func__);
		status = STATUS_SD2JTAG;
	} else {
		if (sd_sim_detect_status_current == SD_SIM_DETECT_STATUS_SIM) {
			status = STATUS_SIM;
		} else if (sd_sim_detect_status_current == SD_SIM_DETECT_STATUS_SD) {
			status = STATUS_SD;
		} else {
			pr_info("%s: sd_sim_detect: %s, as STATUS_NO_CARD\n", __func__, detect_status_to_string());
			status = STATUS_NO_CARD;
		}
	}
#endif
	return status;
}
EXPORT_SYMBOL(get_card1_type);
