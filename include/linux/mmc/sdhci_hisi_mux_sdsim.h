/*
 * add nano sd function definition.
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

#ifndef _SDHCI_HISI_MUX_SD_SIM_
#define _SDHCI_HISI_MUX_SD_SIM_

#include <linux/semaphore.h>

#define MUX_SDSIM_LOG_TAG "[MUX_SDSIM][mmc1][hi_mci.1]"

extern struct semaphore sem_mux_sdsim_detect;
extern struct semaphore sem_mux_sdsim_detect;
extern struct sdhci_host *sdhci_host_from_sd_module;
extern int sd_sim_detect_status_current;
extern int sd_clk_driver_strength;
extern int sd_cmd_driver_strength;
extern int sd_data_driver_strength;

enum sdsim_gpio_mode {
	SDSIM_MODE_GPIO_DETECT = 0, /* gpio detect mode for detect sd or sim */
	SDSIM_MODE_SD_NORMAL = 1, /* sd normal mode */
	SDSIM_MODE_SD_IDLE = 2, /* sd idle/lowpower mode */
};

#define MODULE_SD    0
#define MODULE_SIM  1

#define GPIO_VALUE_LOW     0
#define GPIO_VALUE_HIGH    1

/*
  * status=1 means plug out;
  * status=0 means plug in;
  */
#define STATUS_PLUG_IN    0
#define STATUS_PLUG_OUT 1

#define SD_SIM_DETECT_STATUS_UNDETECTED            0
#define SD_SIM_DETECT_STATUS_SD                             1
#define SD_SIM_DETECT_STATUS_SIM                           2
#define SD_SIM_DETECT_STATUS_ERROR                      3
#define SLEEP_MS_TIME_FOR_DETECT_UNSTABLE       40
#define DRIVER_STRENGTH_2MA_0   0x00
#define DRIVER_STRENGTH_11MA_1 0x30
#define DRIVER_STRENGTH_25MA_1 0x90
#define SD_CLK_DRIVER_DEFAULT DRIVER_STRENGTH_25MA_1
#define SD_CMD_DRIVER_DEFAULT DRIVER_STRENGTH_11MA_1
#define SD_DATA_DRIVER_DEFAULT DRIVER_STRENGTH_11MA_1

#define GPIO_104	104
#define GPIO_120	120
#define GPIO_160	160

#define FUNCTION0 0
#define FUNCTION1 1
#define FUNCTION2 2
#define FUNCTION3 3
#define FUNCTION4 4

#define PULL_TYPE_NP 0x00
#define PULL_TYPE_PU 0x01
#define PULL_TYPE_PD 0x02

#define STATUS_SIM 5
#define STATUS_SD 6
#define STATUS_NO_CARD 7
#define STATUS_SD2JTAG 8
#define SWITCH_GPIO_SD_SIDE 1
#define SWITCH_GPIO_SIM_SIDE 0

#define GPIO_DEFAULT_NUMBER_FOR_SD_CLK      160
#define GPIO_DEFAULT_NUMBER_FOR_SD_CMD      161
#define GPIO_DEFAULT_NUMBER_FOR_SD_DATA0    162
#define GPIO_DEFAULT_NUMBER_FOR_SD_DATA1    163
#define GPIO_DEFAULT_NUMBER_FOR_SD_DATA2    164
#define GPIO_DEFAULT_NUMBER_FOR_SD_DATA3    165

#define IOMG_BASE_GPIO_120 0xF8480000
#define IOCG_BASE_GPIO_120 0xF8480800

/* hisilicon iomux xml and pinctrl framework can't support such SD-SIM-IO-MUX case,wo need config different five modes here manully in code */
int config_sdsim_gpio_mode(enum sdsim_gpio_mode gpio_mode);
char *detect_status_to_string(void);

/*
 * Description: while sd/sim plug in or out, gpio_cd detect pin is actived,we need call this sd_sim_detect_run function to make sure sd or sim which is inserted
 * dw_mci_host: MODULE_SD use dw_mci_host argu as input, while MODULE_SIM just use NULL
 * status: use STATUS_PLUG_IN or STATUS_PLUG_OUT by gpio_cd detect pin's value
 * current_module: sd or sim which module is calling this function
 * return value:return STATUS_PLUG_IN or STATUS_PLUG_OUT,just tell current_module sd or sim is inserted or not, and current_module can update gpio_cd detect pin value by this return value
 */
int sd_sim_detect_run(void *dw_mci_host, int status, int current_module, int need_sleep);
extern void register_gpio_number_group(int start_gpio_number_for_sd_clk);
extern int mmc_detect_sd_or_mmc(struct mmc_host *host);
int sdhci_check_himntn(void);

#endif
