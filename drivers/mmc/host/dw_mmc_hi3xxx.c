/*
 * sd, sdio, driver interface.
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "dw_mmc_hi3xxx.h"

#include <linux/bootdevice.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
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
#include <linux/mfd/hisi_pmic.h>
#include <linux/gpio.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>

#ifdef CONFIG_MMC_DW_MUX_SDSIM
#include <linux/mmc/dw_mmc_mux_sdsim.h>
#endif
#ifdef CONFIG_HUAWEI_EMMC_DSM
#include <linux/mmc/dsm_emmc.h>
#endif
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

#include "dw_mmc.h"
#include "dw_mmc_zodiac.h"
#include "dw_mmc-pltfm.h"
#include "dw_mmc_hi3xxx_common.h"

#ifdef CONFIG_MMC_DW_MUX_SDSIM
static int first_mux_sdsim_probe_init = 1;
struct dw_mci *host_from_sd_module;
#endif

#ifdef CONFIG_HUAWEI_DSM
static struct dsm_dev dsm_dw_mmc = {
	.name = "dsm_sdio",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 2048, /* master name lenth */
};

static struct dsm_client *dclient;
#endif

static struct hwspinlock *sd_hwlock;

int g_sdio_reset_ip;
EXPORT_SYMBOL(g_sdio_reset_ip);

struct dw_mci *dw_mci_get_1135_host(void)
{
	return sdio_1135_host;
}

int dw_mci_check_himntn(int feature)
{
#ifdef CONFIG_HISILICON_PLATFORM_MAINTAIN
	return check_himntn(feature);
#else
	return 0;
#endif
}

static void dw_mci_lever_shit_voltage(int lever_value, int gpio_num)
{
	int err;

	err = gpio_request(gpio_num, "gpio_num");
	if (err < 0) {
		pr_err("Can`t request gpio number %d\n", gpio_num);
		return;
	}

	pr_info("%s mmc gpio num: %d, lever_value: %d\n", __func__, gpio_num, lever_value);
	gpio_direction_output(gpio_num, lever_value);
	gpio_set_value(gpio_num, lever_value);
	gpio_free(gpio_num);
}

static int dw_mci_set_sel18(struct dw_mci *host, bool set)
{
	u32 reg;
	unsigned long flag = 0;

	if (!sd_hwlock || host->lever_shift)
		return 0;
	/* 1s timeout,if we can't get sd_hwlock,sd card module will init failed */
	if (hwspin_lock_timeout_irqsave(sd_hwlock, SD_LOCK_TIMEOUT, &flag)) {
		pr_err("%s: hwspinlock timeout!\n", __func__);
		return 0;
	}

	reg = readl(sys_base + host->scperctrls);
	if (set)
		reg |= host->bit_sdcard_o_sel18;
	else
		reg &= ~(host->bit_sdcard_o_sel18);

	/* set mask bit when the reg needs mask to protect */
	reg |= host->odio_sd_mask_bit;

	writel(reg, sys_base + host->scperctrls);

	hwspin_unlock_irqrestore(sd_hwlock, &flag);
	pr_err("%s: reg = 0x%x\n", __func__, reg);

	return 0;
}

static inline void dw_mci_hs_check_result(struct dw_mci *host, int ret, char *log)
{
	if (ret)
		dev_warn(host->dev, "%s", log);
}

#ifdef CONFIG_MMC_SIM_GPIO_EXCHANGE
/* Try to pull up gpio to set gpio for sim card */
void dw_mci_set_sd_gpio_to_high(struct dw_mci *host)
{
	struct dw_mci_hs_priv_data *priv = host->priv;
	int ret;

	/* 0 and -1 means we dont need to operate gpio */
	if ((priv->set_sd_io) && (priv->set_sd_io != -1)) {
		/* set GPIO169 to High */
		ret = gpio_request(priv->set_sd_io, "set_sd_io");
		if (ret < 0) {
			dev_err(host->dev, "get set_sd_io error");
			return;
		}
		gpio_direction_output(priv->set_sd_io, 1);
		dev_info(host->dev, "set set_sd_to 1\n");
		gpio_free(priv->set_sd_io);
	}

	/* 0 and -1 means we dont need to operate gpio */
	if ((priv->set_sd_io2) && (priv->set_sd_io2 != -1) &&
		(!priv->set_sd_gpio167_low)) {
		/* set GPIO167 to High */
		ret = gpio_request(priv->set_sd_io2, "set_sd_io2");
		if (ret < 0) {
			dev_err(host->dev, "get set_sd_io2 error");
			return;
		}
		gpio_direction_output(priv->set_sd_io2, 1);
		dev_info(host->dev, "set set_sd_to2 1\n");
		gpio_free(priv->set_sd_io2);
	}
}
#endif

static void dw_mci_hs_set_pinctrl_idle(struct dw_mci *host)
{
	int ret;
#ifdef CONFIG_MMC_DW_MUX_SDSIM
	struct dw_mci_hs_priv_data *priv = host->priv;
#endif

	/* set pin to idle, skip emmc for vccq keeping power always on */
	if ((host->hw_mmc_id == DWMMC_SD_ID) &&
		!(dw_mci_check_himntn(HIMNTN_SD2JTAG) ||
			dw_mci_check_himntn(HIMNTN_SD2DJTAG))) {
#ifdef CONFIG_MMC_DW_MUX_SDSIM
		if (priv->mux_sdsim) {
			if (SD_SIM_DETECT_STATUS_SIM !=
				sd_sim_detect_status_current)
				config_sdsim_gpio_mode(SDSIM_MODE_SD_IDLE);
			else
				pr_err("%s %s SIM has detected,but SD module "
				       "want to config SDSIM_MODE_SD_IDLE,just passby\n",
					MUX_SDSIM_LOG_TAG, __func__);
		} else {
			if ((host->pinctrl) && (host->pins_idle)) {
				ret = pinctrl_select_state(host->pinctrl, host->pins_idle);
				dw_mci_hs_check_result(host, ret, "could not set sd idle pins\n");
			}
		}
#else
		if ((host->pinctrl) && (host->pins_idle)) {
			ret = pinctrl_select_state(host->pinctrl, host->pins_idle);
			dw_mci_hs_check_result(host, ret, "could not set sd idle pins\n");
		}
#endif
	} else if ((host->hw_mmc_id != DWMMC_EMMC_ID) &&
		   (host->hw_mmc_id != DWMMC_SD_ID)) {
		if ((host->pinctrl) && (host->pins_idle)) {
			ret = pinctrl_select_state(host->pinctrl, host->pins_idle);
			dw_mci_hs_check_result(host, ret, "could not set idle pins\n");
		}
	}
}

static void dw_mci_hs_set_power_idle(struct dw_mci *host)
{
	int ret;
#ifdef CONFIG_MMC_DW_MUX_SDSIM
	struct dw_mci_hs_priv_data *priv = host->priv;
#endif
	if (host->vqmmc) {
		ret = regulator_disable(host->vqmmc);
		dw_mci_hs_check_result(host, ret, "egulator_disable vqmmc failed\n");
	}

#ifdef CONFIG_MMC_DW_MUX_SDSIM
	if (host->hw_mmc_id == DWMMC_SD_ID && priv->mux_sdsim) {
		if (host->vmmc) {
			if (!(host->vmmcmosen) ||
				(priv->mux_sdsim_vcc_status != MUX_SDSIM_VCC_STATUS_1_8_0_V)) {
				ret = regulator_disable(host->vmmc);
				dw_mci_hs_check_result(host, ret, "egulator_disable vmmc failed\n");
			}
		}
		/* BIAS power disable by i2c */
		if (host->flags & OUTLDO_GPIO_POWER)
			boost_5v_enable(BOOST_5V_DISABLE, BOOST_CTRL_SD);

		if (host->vmmcmosen) {
			ret = regulator_disable(host->vmmcmosen);
			dw_mci_hs_check_result(host, ret, "egulator_disable vmmcmosen failed\n");
		}
	} else {
		if (host->vmmc) {
			ret = regulator_disable(host->vmmc);
			dw_mci_hs_check_result(host, ret, "egulator_disable vmmc failed\n");
		}
	}

#else
	if (host->vmmc) {
		ret = regulator_disable(host->vmmc);
		dw_mci_hs_check_result(host, ret, "egulator_disable vmmc failed\n");
	}
#endif

	if (host->lever_shift)
		dw_mci_lever_shit_voltage(LEVER_SHIFT_1_8V, LEVER_SHIFT_GPIO);

	if (host->flags & NANO_GPIO_POWER) {
		dw_mci_lever_shit_voltage(POWER_DOWN_VCC, CW_IO_POWER);
		dw_mci_lever_shit_voltage(POWER_DOWN_VCC, CW_DEVICE_POWER);
		pr_err("%s POWER_UP_VCC close\n", __func__);
	}
}

static void dw_mci_hs_power_off(struct dw_mci *host, struct mmc_ios *ios)
{
	int ret;
#ifdef CONFIG_MMC_DW_MUX_SDSIM
	struct dw_mci_hs_priv_data *priv = host->priv;

	if (host->hw_mmc_id == DWMMC_SD_ID && priv->mux_sdsim) {
		if (sd_sim_detect_status_current == SD_SIM_DETECT_STATUS_SIM) {
			dev_info(host->dev, "%s %s SIM has detected,but SD "
				"module want to power_off,just passby\n",
				MUX_SDSIM_LOG_TAG, __func__);
			return;
		}
		dev_info(host->dev,
			"%s %s sd_sim_detect_status_current=%d,now SD "
			"module want to power_off,go on\n",
			MUX_SDSIM_LOG_TAG, __func__,
			sd_sim_detect_status_current);
	}
#endif
	dw_mci_hs_set_pinctrl_idle(host);
	if (host->hw_mmc_id == DWMMC_SDIO_ID) {
		if (host->lever_shift)
			dw_mci_lever_shit_voltage(LEVER_SHIFT_1_8V, LEVER_SHIFT_GPIO);
		if (host->lever_shift && host->vqmmc) {
			ret = regulator_disable(host->vqmmc);
			pr_err("%s regulator_disable ret = %d\n", __func__, ret);
			dw_mci_hs_check_result(host, ret, "egulator_disable vqmmc failed\n");
		}
		dev_err(host->dev, "POWER_OFF SDIO!\n");
		return;
	}
	dw_mci_hs_set_power_idle(host);
#ifdef CONFIG_MMC_SIM_GPIO_EXCHANGE
	dw_mci_set_sd_gpio_to_high(host);
#endif
}

#ifdef CONFIG_MMC_SIM_GPIO_EXCHANGE
/* Try to pull down gpio to set gpio for sd card */
void dw_mci_set_sd_gpio_to_low(struct dw_mci *host)
{
	struct dw_mci_hs_priv_data *priv = host->priv;
	int ret;

	/* 0 and -1 means we dont need to operate gpio */
	if ((priv->set_sd_io != 0) && (priv->set_sd_io != -1)) {
		/* set GPIO169 to Low */
		ret = gpio_request(priv->set_sd_io, "set_sd_io");
		if (ret < 0) {
			dev_err(host->dev, "get set_sd_io error");
			return;
		}
		gpio_direction_output(priv->set_sd_io, 0);
		dev_info(host->dev, "set set_sd_to 0\n");
		gpio_free(priv->set_sd_io);
	}

	/* 0 and -1 means we dont need to operate gpio */
	if ((priv->set_sd_io2 != 0) && (priv->set_sd_io2 != -1)) {
		/* set GPIO169 to Low */
		ret = gpio_request(priv->set_sd_io2, "set_sd_io2");
		if (ret < 0) {
			dev_err(host->dev, "get set_sd_io2 error");
			return;
		}
		gpio_direction_output(priv->set_sd_io2, 0);
		dev_info(host->dev, "set set_sd_to2 0\n");
		gpio_free(priv->set_sd_io2);
	}
}
#endif

#ifdef CONFIG_MMC_DW_MUX_SDSIM
static int dw_mci_slot_status_detect(struct dw_mci *host)
{
	struct dw_mci_hs_priv_data *priv = host->priv;

	if (host->hw_mmc_id == DWMMC_SD_ID && priv->mux_sdsim) {
		if (sd_sim_detect_status_current == SD_SIM_DETECT_STATUS_SIM) {
			dev_info(host->dev, "%s %s SIM has detected,but SD "
					    "module want to power_up,just passby\n",
				MUX_SDSIM_LOG_TAG, __func__);
			return -1;
		}
		dev_info(host->dev, "%s %s sd_sim_detect_status_current=%d,now SD "
			"module want to power_up,go on\n", MUX_SDSIM_LOG_TAG, __func__,
			sd_sim_detect_status_current);
		if (MUX_SDSIM_VCC_STATUS_2_9_5_V == priv->mux_sdsim_vcc_status)
			usleep_range(250000, 300000);
	}
	return 0;
}
#endif

static void dw_mci_power_up_sd_vqmmc(struct dw_mci *host)
{
#ifdef CONFIG_MMC_DW_MUX_SDSIM
	struct dw_mci_hs_priv_data *priv = host->priv;
#endif
	int ret;

	if (host->vqmmc) {
#ifdef CONFIG_MMC_DW_MUX_SDSIM
		if (priv->mux_sdsim && !host->lever_shift)
			/* set vqmmc 1.8V */
			ret = regulator_set_voltage(host->vqmmc, 1800000, 1800000);
		else
			/* set vqmmc 2.95V */
			ret = regulator_set_voltage(host->vqmmc, 2950000, 2950000);
#else
		/* set vqmmc 2.95V */
		ret = regulator_set_voltage(host->vqmmc, 2950000, 2950000);
#endif
		dw_mci_hs_check_result(host, ret, "regulator_set_voltage vqmmc failed!\n");

		ret = regulator_enable(host->vqmmc);
		dw_mci_hs_check_result(host, ret, "regulator_enable vqmmc failed!\n");
		usleep_range(1000, 1500);
	}

	if (host->flags & NANO_GPIO_POWER) {
		dw_mci_lever_shit_voltage(POWER_UP_VCC, CW_IO_POWER);
		pr_err("%s POWER_UP_VCC enable\n", __func__);
	}
}

static void dw_mci_power_up_sd_vmmc(struct dw_mci *host)
{
#ifdef CONFIG_MMC_DW_MUX_SDSIM
	struct dw_mci_hs_priv_data *priv = host->priv;
#endif
	int ret;

	if (!host->vmmc)
		goto lbout;

#ifdef CONFIG_MMC_DW_MUX_SDSIM
	if (priv->mux_sdsim) {
		if (MUX_SDSIM_VCC_STATUS_1_8_0_V ==
			priv->mux_sdsim_vcc_status) {
			if (host->flags & OUTLDO_GPIO_POWER) {
				/* BIAS power enable by i2c */
				boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_SD);
				/* FB PIN set to low lever */
				dw_mci_lever_shit_voltage(LEVERSHIFT_LOW, GPIO_010_SDLDO_MOS_EN);
			}

			/* set vmmc 1.8V */
			ret = regulator_set_voltage(host->vmmc, 1800000, 1800000);
			dev_info(host->dev, "%s %s LDO16 VCC set for 1.8V ret = %d\n",
				MUX_SDSIM_LOG_TAG, __func__, ret);
		} else {
			if (host->flags & OUTLDO_GPIO_POWER) {
				/* BIAS power enable by i2c */
				boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_SD);
				/* FB PIN set to high lever */
				dw_mci_lever_shit_voltage(LEVERSHIFT_HIGH, GPIO_010_SDLDO_MOS_EN);
			}

			/* set vmmc 2.95V */
			ret = regulator_set_voltage(host->vmmc, 2950000, 2950000);
			dev_info(host->dev, "%s %s LDO16 VCC set for 2.95V ret = %d\n",
				MUX_SDSIM_LOG_TAG, __func__, ret);
		}

		if (host->vmmcmosen) {
			/* set vmmcmosen 3V */
			ret = regulator_set_voltage(host->vmmcmosen, 3000000, 3000000);
			dev_info(host->dev, "%s %s LDO12 VCC set for 3.00V ret = %d\n",
				MUX_SDSIM_LOG_TAG, __func__, ret);
		}
	} else {
		/* set vmmc 2.95V */
		ret = regulator_set_voltage(host->vmmc, 2950000, 2950000);
	}
#else
	/* set vmmc 2.95V */
	ret = regulator_set_voltage(host->vmmc, 2950000, 2950000);
#endif
	dw_mci_hs_check_result(host, ret, "regulator_set_voltage vmmc failed!\n");

#ifdef CONFIG_MMC_DW_MUX_SDSIM
	if (priv->mux_sdsim) {
		if (!(host->vmmcmosen) ||
			(MUX_SDSIM_VCC_STATUS_1_8_0_V !=
				priv->mux_sdsim_vcc_status)) {
			ret = regulator_enable(host->vmmc);
			dw_mci_hs_check_result(host, ret, "regulator_enable vmmc failed!\n");
			usleep_range(1000, 1500);
		}

		if (host->vmmcmosen) {
			ret = regulator_enable(host->vmmcmosen);
			dw_mci_hs_check_result(host, ret, "regulator_enable vmmcmosen failed!\n");
			usleep_range(1000, 1500);
		}
	} else {
		ret = regulator_enable(host->vmmc);
		dw_mci_hs_check_result(host, ret, "regulator_enable vmmc failed!\n");
		usleep_range(1000, 1500);
	}
#else
	ret = regulator_enable(host->vmmc);
	dw_mci_hs_check_result(host, ret, "regulator_enable vmmc failed!\n");
	usleep_range(1000, 1500);
#endif

lbout:
	if (host->flags & NANO_GPIO_POWER) {
		dw_mci_lever_shit_voltage(POWER_UP_VCC, CW_DEVICE_POWER);
		pr_err("%s POWER_UP_VCC enable\n", __func__);
	}
}

static void dw_mci_hs_set_pinctrl_normal(struct dw_mci *host)
{
#ifdef CONFIG_MMC_DW_MUX_SDSIM
	struct dw_mci_hs_priv_data *priv = host->priv;
#endif
	u32 reg;
	int ret;

	/* Before enable GPIO,disable clk */
	reg = mci_readl(host, GPIO);
	reg &= ~ENABLE_CLK;
	mci_writel(host, GPIO, reg);
	udelay(20);

	if (!(dw_mci_check_himntn(HIMNTN_SD2JTAG) || dw_mci_check_himntn(HIMNTN_SD2DJTAG))) {
#ifdef CONFIG_MMC_DW_MUX_SDSIM
		if (priv->mux_sdsim) {
			if (sd_sim_detect_status_current != SD_SIM_DETECT_STATUS_SIM)
				config_sdsim_gpio_mode(SDSIM_MODE_SD_NORMAL);
			else
				pr_err("%s %s SIM has detected,but SD module want to config "
					    "SDSIM_MODE_SD_NORMAL,just passby\n", MUX_SDSIM_LOG_TAG, __func__);
		} else {
			if ((host->pinctrl) && (host->pins_default)) {
				ret = pinctrl_select_state(host->pinctrl, host->pins_default);
				dw_mci_hs_check_result(host, ret, "could not set default pins\n");
			}
		}

#else
		if ((host->pinctrl) && (host->pins_default)) {
			ret = pinctrl_select_state(host->pinctrl, host->pins_default);
			dw_mci_hs_check_result(host, ret, "could not set default pins\n");
		}
#endif
	}

	/* After enable GPIO,enable clk */
	reg = mci_readl(host, GPIO);
	reg |= ENABLE_CLK;
	mci_writel(host, GPIO, reg);
	udelay(20);
}

static void dw_mci_hs_power_up_default(struct dw_mci *host)
{
	int ret;

	if ((host->pinctrl) && (host->pins_default)) {
		ret = pinctrl_select_state(host->pinctrl, host->pins_default);
		pr_info("%s regulator_enable ret = %d\n", __func__, ret);
		dw_mci_hs_check_result(host, ret, "could not set pins\n");
	}

	if (host->hw_mmc_id == DWMMC_SDIO_ID) {
		if (host->lever_shift && host->vmmc) {
			/* set vmmc 2.95V */
			ret = regulator_set_voltage(host->vmmc, 2950000, 2950000);
			dw_mci_hs_check_result(host, ret, "set voltage vmmc failed!\n");

			ret = regulator_enable(host->vmmc);
			dw_mci_hs_check_result(host, ret, "enable regulator vmmc failed!\n");
			pr_err("%s regulator_enable ret = %d\n", __func__, ret);
		}
		if (host->lever_shift)
			dw_mci_lever_shit_voltage(LEVER_SHIFT_1_8V, LEVER_SHIFT_GPIO);
		dev_err(host->dev, "POWER_UP SDIO !\n");
		if (g_sdio_reset_ip == 1) {
			dw_mci_hi3xxx_work_fail_reset(host);
			g_sdio_reset_ip = 0;
		}
		return;
	}

	if (host->vmmc) {
		/* set vmmc 2.95V */
		ret = regulator_set_voltage(host->vmmc, 2950000, 2950000);
		dw_mci_hs_check_result(host, ret, "set voltage vmmc failed!\n");

		ret = regulator_enable(host->vmmc);
		dw_mci_hs_check_result(host, ret, "enable regulator vmmc failed!\n");
	}

	if (host->vqmmc) {
		/* set vqmmc 2.95V */
		ret = regulator_set_voltage(host->vqmmc, 2950000, 2950000);
		dw_mci_hs_check_result(host, ret, "set voltage vqmmc failed!\n");

		ret = regulator_enable(host->vqmmc);
		dw_mci_hs_check_result(host, ret, "enable regulator vqmmc failed!\n");
	}
}

static void dw_mci_hs_power_up(struct dw_mci *host, struct mmc_ios *ios)
{
#ifdef CONFIG_MMC_DW_MUX_SDSIM
	struct dw_mci_hs_priv_data *priv = host->priv;
#endif
	int ret;

#ifdef CONFIG_MMC_DW_MUX_SDSIM
	ret = dw_mci_slot_status_detect(host);
	if (ret)
		return;
#endif

#ifdef CONFIG_MMC_SIM_GPIO_EXCHANGE
	dw_mci_set_sd_gpio_to_low(host);
#endif

	if (host->hw_mmc_id == DWMMC_SD_ID) {
#ifdef CONFIG_MMC_DW_MUX_SDSIM
		if (priv->mux_sdsim || (host->flags & SD_SUPPORT_1135))
			ret = dw_mci_set_sel18(host, 1);
		else
			ret = dw_mci_set_sel18(host, 0);
#else
		ret = dw_mci_set_sel18(host, 0);
#endif
		dw_mci_hs_check_result(host, ret, "ios dw_mci_set_sel18 error!\n");
		usleep_range(5000, 5500);

#ifdef CONFIG_MMC_DW_MUX_SDSIM
		if (host->lever_shift) {
			if (priv->mux_sdsim)
				dw_mci_lever_shit_voltage(LEVER_SHIFT_1_8V, LEVER_SHIFT_GPIO);
			else
				dw_mci_lever_shit_voltage(LEVER_SHIFT_3_0V, LEVER_SHIFT_GPIO);
		}
#endif

		dw_mci_power_up_sd_vqmmc(host);

		dw_mci_power_up_sd_vmmc(host);

		dw_mci_hs_set_pinctrl_normal(host);
	} else {
		dw_mci_hs_power_up_default(host);
	}
}


#ifdef CONFIG_MMC_SIM_GPIO_EXCHANGE
/* free vmmc operate for sim card */
void dw_mci_put_regulator(struct dw_mci *host)
{
	if (host->hw_mmc_id == DWMMC_SD_ID) {
		if (host->vqmmc) {
			devm_regulator_put(host->vqmmc);
			host->vqmmc = NULL;
		}

		if (host->vmmc) {
			devm_regulator_put(host->vmmc);
			host->vmmc = NULL;
		}

		if ((host->sd_vmmcmosen_switch == 1) &&
			(host->vmmcmosen)) {
			devm_regulator_put(host->vmmcmosen);
			host->vmmcmosen = NULL;
		}

		host->need_get_mmc_regulator = 1;
		dev_info(host->dev, "regulator put\n");
	}
}

/* get vmmc operate for sd card */
void dw_mci_get_regulator(struct dw_mci *host)
{
	if ((host->hw_mmc_id == DWMMC_SD_ID) &&
		(host->need_get_mmc_regulator == 1)) {
		host->vmmc = devm_regulator_get(host->dev, "vmmc");
		if (IS_ERR(host->vmmc)) {
			dev_info(host->dev, "no vmmc regulator found\n");
			host->vmmc = NULL;
		}

		host->vqmmc = devm_regulator_get(host->dev, "vqmmc");
		if (IS_ERR(host->vqmmc)) {
			dev_info(host->dev, "no vqmmc regulator found\n");
			host->vqmmc = NULL;
		}

		if (host->sd_vmmcmosen_switch == 1) {
			host->vmmcmosen = devm_regulator_get(host->dev, "vmmcmosen");
			if (IS_ERR(host->vmmcmosen)) {
				dev_info(host->dev, "no vmmcmosen regulator found\n");
				host->vmmcmosen = NULL;
			}
		} else {
			host->vmmcmosen = NULL;
		}

		host->need_get_mmc_regulator = 0;
		dev_info(host->dev, "regulator get\n");
	}
}
#endif

void dw_mci_hs_set_clk_helper(struct dw_mci *host, struct mmc_ios *ios)
{
	struct dw_mci_hs_priv_data *priv = host->priv;
	int id = priv->id;
	int ret;

	if (priv->old_timing == ios->timing) /* no need change clock */
		return;

	if (!IS_ERR(host->ciu_clk))
		clk_disable_unprepare(host->ciu_clk);

	ret = clk_set_rate(host->ciu_clk, hs_timing_config[id][ios->timing][CCLK_FRQ]);
	if (ret)
		dev_err(host->dev, "clk_set_rate failed, clock = %d, ret = %d\n",
			hs_timing_config[id][ios->timing][CCLK_FRQ], ret);

	if (!IS_ERR(host->ciu_clk)) {
		if (clk_prepare_enable(host->ciu_clk))
			dev_err(host->dev, "ciu_clk clk_prepare_enable failed\n");
	}

	if (priv->in_resume != STATE_KEEP_PWR)
		host->tuning_init_sample =
			(hs_timing_config[id][ios->timing][SAM_PHASE_MAX] +
				hs_timing_config[id][ios->timing][SAM_PHASE_MIN]) / SAM_PHASE_MID;

	if (!host->sd_reinit)
		host->current_div = hs_timing_config[id][ios->timing][CCLK_DIV];

	dw_mci_hs_set_timing(host, id, ios->timing, host->tuning_init_sample,
		host->current_div);

	if (!priv->priv_bus_hz)
		host->bus_hz = (unsigned int)hs_timing_config[id][ios->timing][BUS_HZ];
	else
		host->bus_hz = (unsigned int)2 * hs_timing_config[id][ios->timing][BUS_HZ];

	if (priv->dw_mmc_bus_clk)
		host->bus_hz = (unsigned int)priv->dw_mmc_bus_clk;

	priv->old_timing = ios->timing;
}

static void dw_mci_hs_set_power(struct dw_mci *host, struct mmc_ios *ios)
{
	struct dw_mci_hs_priv_data *priv = host->priv;

	if (priv->old_power_mode == ios->power_mode) /* no need change power */
		return;

	switch (ios->power_mode) {
	case MMC_POWER_OFF:
		dev_info(host->dev, "set io to lowpower\n");
		dw_mci_hs_power_off(host, ios);

#ifdef CONFIG_MMC_SIM_GPIO_EXCHANGE
		dw_mci_put_regulator(host);
#endif
		break;
	case MMC_POWER_UP:
#ifdef CONFIG_MMC_SIM_GPIO_EXCHANGE
		dw_mci_get_regulator(host);
#endif
		dev_info(host->dev, "set io to normal\n");
		dw_mci_hs_power_up(host, ios);
		break;
	case MMC_POWER_ON:
		break;
	default:
		dev_info(host->dev, "unknown power supply mode\n");
		break;
	}
	priv->old_power_mode = ios->power_mode;
}

static void dw_mci_hs_set_ios(struct dw_mci *host, struct mmc_ios *ios)
{
	dw_mci_hs_set_power(host, ios);
	dw_mci_hs_set_clk_helper(host, ios);
}

static void dw_mci_hs_prepare_command(struct dw_mci *host, u32 *cmdr)
{
	*cmdr |= SDMMC_CMD_USE_HOLD_REG;
}

/*
 * name:get_resource_hsdt1_crg_base
 * func:soc platform get hsdt1_crg_base address of pisces from DTS
 */
static int get_resource_hsdt1_crg_base(void)
{
	struct device_node *np = NULL;

	if (!hsdt1_crg_base) {
		np = of_find_compatible_node(NULL, NULL, "Hisilicon,hsdt1_crg");
		if (!np) {
			pr_err("can't find hsdt1_crg!\n");
			return -1;
		}

		hsdt1_crg_base = of_iomap(np, 0);
		if (!hsdt1_crg_base) {
			pr_err("hsdt1 crg iomap error!\n");
			return -1;
		}
	}

	return 0;
}

/*
 * name:get_resource_hsdt_crg_base
 * func:soc platform get hsdt_crg_base address of taurus from DTS
 */
static int get_resource_hsdt_crg_base(void)
{
	struct device_node *np = NULL;

	if (!hsdt_crg_base) {
		np = of_find_compatible_node(NULL, NULL, "Hisilicon,hsdt_crg");
		if (!np) {
			pr_err("can't find hsdt_crg!\n");
			return -1;
		}

		hsdt_crg_base = of_iomap(np, 0);
		if (!hsdt_crg_base) {
			pr_err("hsdt crg iomap error!\n");
			return -1;
		}
	}

	return 0;
}

/*
 * name:get_resource_mmc0_crg_base
 * func:soc platform get mmc0_crg_base address of taurus from DTS
 */
static int get_resource_mmc0_crg_base(void)
{
	struct device_node *np = NULL;

	if (!mmc0_crg_base) {
		np = of_find_compatible_node(NULL, NULL, "hisilicon,mmc0crg");
		if (!np) {
			pr_err("can't find mmc0_crg!\n");
			return -1;
		}

		mmc0_crg_base = of_iomap(np, 0);
		if (!mmc0_crg_base) {
			pr_err("mmc0sysctrl iomap error!\n");
			return -1;
		}
	}

	return 0;
}

/*
 * name:get_resource_mmc1_sys_ctrl
 * func:soc platform get mmc1_sys_ctrl_base address of taurus from DTS
 */
static int get_resource_mmc1_sys_ctrl_base(void)
{
	struct device_node *np = NULL;

	if (!mmc1_sys_ctrl_base) {
		np = of_find_compatible_node(
			NULL, NULL, "Hisilicon,mmc1_sysctrl");
		if (!np) {
			pr_err("can't find mmc1sysctrl!\n");
			return -1;
		}

		mmc1_sys_ctrl_base = of_iomap(np, 0);
		if (!mmc1_sys_ctrl_base) {
			pr_err("mmc1sysctrl iomap error!\n");
			return -1;
		}
	}

	return 0;
}

/*
 * name:dw_mci_hs_get_dt_capricorn_pltfm_resource
 * func:soc platform get the config of capricorn from DTS
 */
static int dw_mci_hs_get_dt_capricorn_pltfm_resource(
	struct dw_mci *host, struct device_node *of_node)
{
	host->scperctrls |= BIT_VOLT_OFFSET_CAPRICORN;
	host->bit_sdcard_o_sel18 |= BIT_VOLT_VALUE_18_CAPRICORN;
	host->sdio_rst |= BIT_HRST_SDIO_CAPRICORN;
	host->odio_sd_mask_bit = BIT_VOLT_VALUE_18_MASK_CAPRICORN;

	memcpy(hs_timing_config, hs_timing_config_capricorn, /* unsafe_function_ignore: memcpy */
		sizeof(hs_timing_config));

	if (get_resource_mmc1_sys_ctrl_base())
		return -ENODEV;

	return 0;
}

/*
 * name:dw_mci_hs_get_dt_pisces_pltfm_resource
 * func:soc platform get the config of pisces from DTS
 */
static int dw_mci_hs_get_dt_pisces_pltfm_resource(
	struct dw_mci *host, struct device_node *of_node)
{
	u32 value_clock = 0;
	host->lever_shift = 1;

	if (of_property_read_u32(of_node, "clock-frequency-ext", &value_clock) == 0)
		hs_timing_config_pisces[SD_TYPE][HS200][DRV_PHASE] = value_clock;
	if (of_find_property(of_node, "extra_gpio_enable", (int *)NULL)) {
		/* BIAS power enable by i2c */
		boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_SD);
		host->flags |= OUTLDO_GPIO_POWER;
	}
	if(of_find_property(of_node, "sim_card_cmd_pd", (int *)NULL))
		host->flags |= SIMCARD_CMD_PD;

	memcpy(hs_timing_config, hs_timing_config_pisces, /* unsafe_function_ignore: memcpy */
		sizeof(hs_timing_config));
	if (get_resource_hsdt1_crg_base())
		return -ENODEV;

	return 0;
}

/*
 * name:dw_mci_hs_get_dt_leo_pltfm_resource
 * func:soc platform get the config of taurus from DTS
 */
static int dw_mci_hs_get_dt_leo_pltfm_resource(
	struct dw_mci *host, struct device_node *of_node)
{
	host->scperctrls |= BIT_VOLT_OFFSET_LEO;
	host->bit_sdcard_o_sel18 |= BIT_VOLT_VALUE_18_LEO;
	host->sdio_rst |= BIT_RST_SDIO_LEO;
	host->odio_sd_mask_bit = BIT_VOLT_VALUE_18_MASK_LEO;

	if (of_find_property(of_node, "cs_sd_timing_config_aquarius", (int *)NULL)) {
		pr_err("%s: cs_sd_timing_config_aquarius property found\n", __func__);
		memcpy(hs_timing_config, hs_timing_config_aquarius, /* unsafe_function_ignore: memcpy */
			sizeof(hs_timing_config));
	} else {
		memcpy(hs_timing_config, /* unsafe_function_ignore: memcpy */
			hs_timing_config_leo, sizeof(hs_timing_config));
	}

	if (get_resource_hsdt_crg_base() || get_resource_mmc0_crg_base())
		return -ENODEV;

	return 0;
}

static int dw_mci_hs_get_dt_sagittarius_pltfm_resource(
	struct dw_mci *host, struct device_node *of_node)
{
	host->scperctrls |= BIT_VOLT_OFFSET_SAGITTARIUS;
	host->bit_sdcard_o_sel18 |= BIT_VOLT_VALUE_18_SAGITTARIUS;
	host->sdio_rst |= BIT_RST_SDIO_SAGITTARIUS;
	host->odio_sd_mask_bit = BIT_VOLT_VALUE_18_MASK_SAGITTARIUS;

	memcpy(hs_timing_config, hs_timing_config_sagittarius, /* unsafe_function_ignore: memcpy */
		sizeof(hs_timing_config));

	if (get_resource_hsdt_crg_base() || get_resource_mmc0_crg_base())
		return -ENODEV;

	return 0;
}

/*
 * name:dw_mci_hs_get_dt_taurus_pltfm_resource
 * func:soc platform get the config of taurus from DTS
 */
static int dw_mci_hs_get_dt_taurus_pltfm_resource(
	struct dw_mci *host, struct device_node *of_node)
{
	host->scperctrls |= BIT_VOLT_OFFSET_TAURUS;
	host->bit_sdcard_o_sel18 |= BIT_VOLT_VALUE_18_TAURUS;
	host->sdio_rst |= BIT_RST_SDIO_TAURUS;
	host->odio_sd_mask_bit = BIT_VOLT_VALUE_18_MASK_TAURUS;

	if (of_find_property(of_node, "cs_sd_timing_config_taurus", (int *)NULL))
		memcpy(hs_timing_config, /* unsafe_function_ignore: memcpy */
			hs_timing_config_taurus_cs, sizeof(hs_timing_config));
	else if (of_find_property(of_node, "cs2_sd_timing_config_taurus", (int *)NULL))
		memcpy(hs_timing_config,  /* unsafe_function_ignore:  memcpy */
			hs_timing_config_taurus_cs2, sizeof(hs_timing_config));

	if (get_resource_hsdt_crg_base() || get_resource_mmc0_crg_base())
		return -ENODEV;

	return 0;
}

/*
 * name:dw_mci_hs_get_dt_libra_pltfm_resource
 * func:soc platform get the config of libra from DTS
 */
static int dw_mci_hs_get_dt_libra_pltfm_resource(
	struct dw_mci *host, struct device_node *of_node)
{
	host->scperctrls |= BIT_VOLT_OFFSET;
	host->bit_sdcard_o_sel18 |= BIT_VOLT_VALUE_18;
	host->sdio_rst |= BIT_RST_SDIO_LIBRA;

	if (of_find_property(of_node, "sd_timing_config_libra_c50", (int *)NULL)) {
		pr_err("%s: cancel c50 property found\n", __func__);
		memcpy(hs_timing_config, hs_timing_config_libra_c50,
			sizeof(hs_timing_config)); /* unsafe_function_ignore: memcpy */
	} else if (of_find_property(of_node, "sd_timing_config_libra_c60", (int *)NULL)) {
		pr_err("%s: cancel c60 property found\n", __func__);
		memcpy(hs_timing_config, hs_timing_config_libra_c60,
			sizeof(hs_timing_config)); /* unsafe_function_ignore: memcpy */
	} else {
		pr_err("%s: cancel default property found\n", __func__);
		memcpy(hs_timing_config, hs_timing_config_libra,
			sizeof(hs_timing_config)); /* unsafe_function_ignore: memcpy */
	}

	return 0;
}

/*
 * name:dw_mci_hs_get_dt_cancer_pltfm_resource
 * func:soc platform get the config of cancer from DTS
 */
static int dw_mci_hs_get_dt_cancer_pltfm_resource(
	struct dw_mci *host, struct device_node *of_node)
{
	u32 value_clock = 0;
	struct device_node *np = host->dev->of_node;

	host->scperctrls |= BIT_VOLT_OFFSET_CANCER;
	host->bit_sdcard_o_sel18 |= BIT_VOLT_VALUE_18_CANCER;
	host->sdio_rst |= BIT_HRST_SDIO_CANCER;
	host->odio_sd_mask_bit = BIT_VOLT_VALUE_18_MASK_CANCER;

	if (of_find_property(of_node, "cs_sd_timing_config_cancer", (int *)NULL)) {
		if (of_property_read_u32(np, "clock-frequency", &value_clock)) {
			dev_info(host->dev, "no need to adjust clock\n");
			value_clock = 0;
		}
		dev_info(host->dev, "value_clock = %d\n", value_clock);
		if (value_clock == MMC_CLOCK_FREQUENCY_137) {
			/* cancer SDR104-137M bus_clock parameter */
			hs_timing_config_cancer_cs[SD_TYPE][HS200][CCLK_DIV] = 6;
			hs_timing_config_cancer_cs[SD_TYPE][HS200][DRV_PHASE] = 6;
			hs_timing_config_cancer_cs[SD_TYPE][HS200][SAM_DLY] = 3;
			hs_timing_config_cancer_cs[SD_TYPE][HS200][SAM_PHASE_MAX] = 13;
			hs_timing_config_cancer_cs[SD_TYPE][HS200][BUS_HZ] = 137000000;
			dev_info(host->dev, "hs_timing_config_cancer_cs[SD_TYPE][HS200][BUS_HZ] = %d\n",
				hs_timing_config_cancer_cs[SD_TYPE][HS200][BUS_HZ]);
		} else if (value_clock == MMC_CLOCK_FREQUENCY_192) {
			/* cancer SDR104-192M bus_clock parameter */
			hs_timing_config_cancer_cs[SD_TYPE][HS200][CCLK_DIV] = 4;
			hs_timing_config_cancer_cs[SD_TYPE][HS200][DRV_PHASE] = 4;
			hs_timing_config_cancer_cs[SD_TYPE][HS200][SAM_DLY] = 2;
			hs_timing_config_cancer_cs[SD_TYPE][HS200][SAM_PHASE_MAX] = 9;
			hs_timing_config_cancer_cs[SD_TYPE][HS200][BUS_HZ] = 192000000;
			dev_info(host->dev, "hs_timing_config_cancer_cs[SD_TYPE][HS200][BUS_HZ] = %d\n",
				hs_timing_config_cancer_cs[SD_TYPE][HS200][BUS_HZ]);
		}
		dev_info(host->dev, "hs_timing_config_cancer_cs[SD_TYPE][HS200][BUS_HZ] = %d\n",
			hs_timing_config_cancer_cs[SD_TYPE][HS200][BUS_HZ]);
		memcpy(hs_timing_config,  /* unsafe_function_ignore: memcpy */
			hs_timing_config_cancer_cs, sizeof(hs_timing_config));
	}

	if (get_resource_mmc1_sys_ctrl_base())
		return -ENODEV;

	return 0;
}

/*
 * name:dw_mci_hs_get_dt_aries_pltfm_resource
 * func:soc platform get the config of aries from DTS
 */
static int dw_mci_hs_get_dt_aries_pltfm_resource(
	struct dw_mci *host, struct device_node *of_node)
{
	host->scperctrls |= BIT_VOLT_OFFSET;
	host->bit_sdcard_o_sel18 |= BIT_VOLT_VALUE_18;
	host->sdio_rst |= BIT_RST_SDIO_ARIES;
	if (of_find_property(of_node, "cs_sd_timing_config", (int *)NULL))
		memcpy(hs_timing_config, hs_timing_config_aries_cs, /* unsafe_function_ignore: memcpy */
			sizeof(hs_timing_config));

	return 0;
}

static int dw_mci_hs_get_dt_pltfm_resource(
	struct dw_mci *host, struct device_node *of_node)
{
	int ret;

	if (of_device_is_compatible(of_node, "hisilicon,capricorn-dw-mshc"))
		ret = dw_mci_hs_get_dt_capricorn_pltfm_resource(host, of_node);
	else if (of_device_is_compatible(of_node, "hisilicon,leo-dw-mshc"))
		ret = dw_mci_hs_get_dt_leo_pltfm_resource(host, of_node);
	else if (of_device_is_compatible(of_node, "hisilicon,pisces-dw-mshc"))
		ret = dw_mci_hs_get_dt_pisces_pltfm_resource(host, of_node);
	else if (of_device_is_compatible(of_node, "hisilicon,taurus-dw-mshc"))
		ret = dw_mci_hs_get_dt_taurus_pltfm_resource(host, of_node);
	else if (of_device_is_compatible(of_node, "hisilicon,libra-dw-mshc"))
		ret = dw_mci_hs_get_dt_libra_pltfm_resource(host, of_node);
	else if (of_device_is_compatible(of_node, "hisilicon,cancer-dw-mshc"))
		ret = dw_mci_hs_get_dt_cancer_pltfm_resource(host, of_node);
	else if (of_device_is_compatible(of_node, "hisilicon,aries-dw-mshc"))
		ret = dw_mci_hs_get_dt_aries_pltfm_resource(host, of_node);
	else if (of_device_is_compatible(of_node, "hisilicon,sagittarius-dw-mshc"))
		ret = dw_mci_hs_get_dt_sagittarius_pltfm_resource(host, of_node);
	else
		return -1;

	return ret;
}

static int dw_mci_hs_get_resource(void)
{
	struct device_node *np = NULL;

	if (!pericrg_base) {
		np = of_find_compatible_node(NULL, NULL, "Hisilicon,crgctrl");
		if (!np) {
			pr_err("can't find crgctrl!\n");
			return -1;
		}

		pericrg_base = of_iomap(np, 0);
		if (!pericrg_base) {
			pr_err("crgctrl iomap error!\n");
			return -1;
		}
	}

	if (!sys_base) {
		np = of_find_compatible_node(NULL, NULL, "Hisilicon,sysctrl");
		if (!np) {
			pr_err("can't find sysctrl!\n");
			return -1;
		}

		sys_base = of_iomap(np, 0);
		if (!sys_base) {
			pr_err("sysctrl iomap error!\n");
			return -1;
		}
	}

	return 0;
}

/*
 * Do private setting specified for controller.
 * dw_mci_hs_priv_init execute before controller unreset,
 * this will cause NOC error.
 * put this function after unreset and clock set.
 */
static void dw_mci_hs_priv_setting(struct dw_mci *host)
{
	/* set threshold to 512 bytes */
	mci_writel(host, CDTHRCTL, 0x02000001);
}

void dw_mci_hs_set_rst_m(struct dw_mci *host, bool set)
{
	struct dw_mci_hs_priv_data *priv = host->priv;
	int id = priv->id;

	if (!pericrg_base) {
		dev_err(host->dev, "pericrg_base is null, can't rst!\n");
		return;
	}
	if (id != DW_MCI_SD_ID) {
		dev_info(host->dev, "other rest_m or unrest_m need to add!\n");
		return;
	}

	if (set) {
		if (mmc0_crg_base)
			writel(BIT_RST_SD_TAURUS, mmc0_crg_base + MMC0_CRG_SD_HRST);
		else if (hsdt1_crg_base)
			writel(BIT_RST_SD_PISCES, hsdt1_crg_base + HSDT1_SD_HRST);
		else
			writel(BIT_RST_SD_M, pericrg_base + PERI_CRG_RSTEN4);
		dev_info(host->dev, "rest_m for sd\n");
	} else {
		if (mmc0_crg_base)
			writel(BIT_RST_SD_TAURUS, mmc0_crg_base + MMC0_CRG_SD_HURST);
		else if (hsdt1_crg_base)
			writel(BIT_RST_SD_PISCES, hsdt1_crg_base + HSDT1_SD_HURST);
		else
			writel(BIT_RST_SD_M, pericrg_base + PERI_CRG_RSTDIS4);
		dev_info(host->dev, "unrest_m for sd\n");
	}
}

void dw_mci_hs_reset(struct dw_mci *host, int id)
{
	/* reset emmc */
	if (id == MMC_EMMC) {
		if (hsdt_crg_base)
			writel(host->sdio_rst, hsdt_crg_base + HSDT_CRG_PERRSTEN0);
		else if (mmc1_sys_ctrl_base)
			writel(BIT_HRST_SDIO_CANCER, mmc1_sys_ctrl_base + MMC1_SYSCTRL_PERRSTEN0);
		else
			writel(BIT_RST_EMMC, pericrg_base + PERI_CRG_RSTEN4);
		dev_info(host->dev, "rest emmc\n");
	/* reset sd */
	} else if (id == MMC_SD) {
		if (mmc0_crg_base)
			writel(BIT_HRST_SD_TAURUS, mmc0_crg_base + MMC0_CRG_SD_HRST);
		else if (hsdt1_crg_base)
			writel(BIT_HRST_SD_PISCES, hsdt1_crg_base + HSDT1_SD_HRST);
		else
			writel(BIT_RST_SD, pericrg_base + PERI_CRG_RSTEN4);
		dev_info(host->dev, "rest sd\n");
	/* reset sdio */
	} else if (id == DWMMC_SDIO_ID) {
		if (host->lever_shift) {
			writel(BIT_HRST_SD_PISCES, hsdt1_crg_base + HSDT1_SD_HRST);
			return;
		}
		if (hsdt_crg_base)
			writel(host->sdio_rst, hsdt_crg_base + HSDT_CRG_PERRSTEN0);
		else if (mmc1_sys_ctrl_base)
			writel(host->sdio_rst, mmc1_sys_ctrl_base + MMC1_SYSCTRL_PERRSTEN0);
		else
			writel(host->sdio_rst, pericrg_base + PERI_CRG_RSTEN4);
		dev_info(host->dev, "rest sdio\n");
	}
}

void dw_mci_hs_unreset(struct dw_mci *host, int id)
{
	/* unreset emmc */
	if (id == MMC_EMMC) {
		if (hsdt_crg_base)
			writel(host->sdio_rst, hsdt_crg_base + HSDT_CRG_PERRSTDIS0);
		else if (mmc1_sys_ctrl_base)
			writel(BIT_HRST_SDIO_CANCER, mmc1_sys_ctrl_base + MMC1_SYSCTRL_PERRSTDIS0);
		else
			writel(BIT_RST_EMMC, pericrg_base + PERI_CRG_RSTDIS4);
		dev_info(host->dev, "unrest emmc\n");
	/* unreset sd */
	} else if (id == MMC_SD) {
		if (mmc0_crg_base)
			writel(BIT_HRST_SD_TAURUS, mmc0_crg_base + MMC0_CRG_SD_HURST);
		else if (hsdt1_crg_base)
			writel(BIT_HRST_SD_PISCES, hsdt1_crg_base + HSDT1_SD_HURST);
		else
			writel(BIT_RST_SD, pericrg_base + PERI_CRG_RSTDIS4);

		dev_info(host->dev, "unrest sd\n");
	/* unreset sdio */
	} else if (id == DWMMC_SDIO_ID) {
		if (host->lever_shift) {
			writel(BIT_HRST_SD_PISCES, hsdt1_crg_base + HSDT1_SD_HURST);
			return;
		}
		if (hsdt_crg_base)
			writel(host->sdio_rst, hsdt_crg_base + HSDT_CRG_PERRSTDIS0);
		else if (mmc1_sys_ctrl_base)
			writel(host->sdio_rst, mmc1_sys_ctrl_base + MMC1_SYSCTRL_PERRSTDIS0);
		else
			writel(host->sdio_rst, pericrg_base + PERI_CRG_RSTDIS4);
		dev_info(host->dev, "unrest sdio\n");
	}
}

int dw_mci_hs_set_controller(struct dw_mci *host, bool set)
{
	struct dw_mci_hs_priv_data *priv = host->priv;
	int id = priv->id;

	if (!pericrg_base) {
		dev_err(host->dev, "pericrg_base is null, can't reset mmc!\n");
		return -1;
	}

	if (set) {
		dw_mci_hs_reset(host, id);
		return 0;
	}
	dw_mci_hs_unreset(host, id);

	return 0;
}

 /*
  * deal with different priv_id value, inner func involked by dw_mci_hs_priv_init
  * return value: false - error;  true - no error
  */
static bool dw_mci_device_rename(struct dw_mci *host,
	int priv_id, struct platform_device *pdev)
{
	int ret;
#ifdef CONFIG_MMC_DW_EMMC_USED_AS_MODEM
	static const char * const hi_mci0 = "hi_mci.3";
#else
	static const char * const hi_mci0 = "hi_mci.0";
#endif
	static const char * const hi_mci1 = "hi_mci.1";
	static const char * const hi_mci2 = "hi_mci.2";

	switch (priv_id) {
	case MMC_EMMC:
		pdev->name = hi_mci0;
		ret = device_rename(host->dev, hi_mci0);
		if (ret < 0) {
			dev_err(host->dev, "dev set name %s fail\n", hi_mci0);
			goto error;
		}
#ifndef CONFIG_MMC_DW_EMMC_USED_AS_MODEM
#ifdef CONFIG_HISI_BOOTDEVICE
		if (get_bootdevice_type() == BOOT_DEVICE_EMMC)
			set_bootdevice_name(&pdev->dev);
#endif
#endif
		break;

	case MMC_SD:
		pdev->name = hi_mci1;
		ret = device_rename(host->dev, hi_mci1);
		if (ret < 0) {
			dev_err(host->dev, "dev set name hi_mci.1 fail\n");
			goto error;
		}

		/*
		 * Sd hardware lock,avoid to access the SCPERCTRL5 register in
		 * USIM card module in the same time
		 */
		sd_hwlock = hwspin_lock_request_specific(SD_HWLOCK_ID);
		if (!sd_hwlock) {
			pr_err("Request hwspin lock failed !\n");
			goto error;
		}

#ifdef CONFIG_MMC_DW_MUX_SDSIM
		host_from_sd_module = host;
		pr_err("%s host_from_sd_module is inited when %s\n", MUX_SDSIM_LOG_TAG, __func__);
		sema_init(&sem_mux_sdsim_detect, 1);
#endif
		break;

	case MMC_SDIO:
		pdev->name = hi_mci2;
		ret = device_rename(host->dev, hi_mci2);
		if (ret < 0) {
			dev_err(host->dev, "dev set name hi_mci.2 fail\n");
			goto error;
		}
		break;

	default:
		dev_err(host->dev, "mpriv->id is out of range!!!\n");
		goto error;
	}

	return true;

error:
	return false;
}

static int dw_mci_hs_priv_init(struct dw_mci *host)
{
	struct dw_mci_hs_priv_data *priv = NULL;
	struct platform_device *pdev = NULL;

	priv = devm_kzalloc(host->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		dev_err(host->dev, "mem alloc failed for private data\n");
		return -ENOMEM;
	}
	priv->id = of_alias_get_id(host->dev->of_node, "mshc");
	if ((priv->id == MMC_EMMC) &&
		(get_bootdevice_type() != BOOT_DEVICE_EMMC)) {
		devm_kfree(host->dev, priv);
		return -ENODEV;
	}
	priv->old_timing = -1;
	priv->in_resume = STATE_LEGACY;
	host->priv = priv;
	host->hw_mmc_id = priv->id;
	host->flags &= ~DWMMC_IN_TUNING;
	host->flags &= ~DWMMC_TUNING_DONE;

	if (priv->id == DW_MCI_SDIO_ID)
		sdio_host = host;
	/*
	 * BUG: device rename krees old name, which would be realloced for other
	 * device, pdev->name points to freed space, driver match may cause a panic
	 * for wrong device
	 */
	pdev = container_of(host->dev, struct platform_device, dev);

	if (!dw_mci_device_rename(host, priv->id, pdev))
		goto fail;

	/* still keep pdev->name same with dev->kobj.name */
	pdev->name = host->dev->kobj.name;

	return 0;

fail:
	/*
	 * if rename failed, restore old value, keep pdev->name same to
	 * dev->kobj.name
	 */
	pdev->name = host->dev->kobj.name;
	devm_kfree(host->dev, priv);
	return -1;
}

static int dw_mci_hs_setup_clock(struct dw_mci *host)
{
	struct dw_mci_hs_priv_data *priv = host->priv;
	int timing = MMC_TIMING_LEGACY;
	int id = priv->id;
	int ret;

	ret = clk_set_rate(host->ciu_clk, hs_timing_config[id][timing][CCLK_FRQ]);
	if (ret) {
		dev_err(host->dev, "clk_set_rate failed\n");
		return ret;
	}

	dw_mci_hs_set_controller(host, DW_MCI_HS_DO_UNRESET);
	dw_mci_hs_priv_setting(host);

	host->tuning_current_sample = -1;
	host->current_div = hs_timing_config[id][timing][CCLK_DIV];
	host->tuning_init_sample = (hs_timing_config[id][timing][SAM_PHASE_MAX] +
		hs_timing_config[id][timing][SAM_PHASE_MIN]) / SAM_PHASE_MID;

	dw_mci_hs_set_timing(
		host, id, timing, host->tuning_init_sample, host->current_div);

	if (!priv->priv_bus_hz)
		host->bus_hz = (unsigned int)hs_timing_config[id][timing][BUS_HZ];
	else
		host->bus_hz = (unsigned int)priv->priv_bus_hz;

	if (priv->dw_mmc_bus_clk)
		host->bus_hz = (unsigned int)priv->dw_mmc_bus_clk;

	priv->old_timing = timing;

	return 0;
}

#ifdef CONFIG_SD_SDIO_CRC_RETUNING
static void dw_mci_hs_get_retuning_config(struct dw_mci *host)
{
	struct device_node *np = host->dev->of_node;

	if (host->hw_mmc_id == DWMMC_SD_ID) {
		if (of_find_property(np, "cs_sd_timing_config", (int *)NULL)) {
			host->retuning_flag = SD_RETUNING_ON;
			dev_info(host->dev, "%s:SD card retuning on!\n", __func__);
		} else {
			host->retuning_flag = SD_RETUNING_OFF;
			dev_info(host->dev, "%s:SD card retuning off!\n", __func__);
		}
	} else if (host->hw_mmc_id == DWMMC_SDIO_ID) {
		if (of_find_property(np, "cs_sdio_timing_config", (int *)NULL)) {
			host->retuning_flag = SD_RETUNING_ON;
			dev_info(host->dev, "%s:SDIO retuning on!\n", __func__);
		} else {
			host->retuning_flag = SD_RETUNING_OFF;
			dev_info(host->dev, "%s:SDIO retuning off!\n", __func__);
		}
	}
}
#endif

void dw_mci_hs_get_timing_config(struct dw_mci *host)
{
	struct device_node *np = host->dev->of_node;
	int ret;

#ifdef CONFIG_SD_SDIO_CRC_RETUNING
	dw_mci_hs_get_retuning_config(host);
#endif
	if (of_find_property(np, "cs_sd_timing_config", (int *)NULL))
		host->is_cs_timing_config = IS_CS_TIMING_CONFIG;

	/*
	 * adapt the min and max value of use_sample_dly in dts between
	 * different chipsets. There are two elements in host->use_samdly_range
	 * which represent use_samdly_range_min,use_samdly_range_max
	 */
	ret = of_property_read_u32_array(np, "use_samdly_range", &(host->use_samdly_range[0]), 2);
	if (ret) {
		host->use_samdly_flag = false;
		dev_info(host->dev, "%s:get use_sample_dly range range failed!\n", __func__);
	} else {
		host->use_samdly_flag = true;
		dev_info(host->dev, "%s:use_samdly_range_min:%d,use_samdly_range_max:%d!\n",
			__func__, host->use_samdly_range[0],
			host->use_samdly_range[1]);
	}
	/*
	 * adapt the min and max value of enable_shift in dts between different
	 * chipsets. There are two elements in host->enable_shift_range which
	 * represent ena_shift_range_min,ena_shift_range_max
	 */
	ret = of_property_read_u32_array(np, "enable_shift_range", &(host->enable_shift_range[0]), 2);
	if (ret) {
		host->enable_shift_flag = false;
		dev_info(host->dev, "%s:get enable_shift range range failed!\n", __func__);
	} else {
		host->enable_shift_flag = true;
		dev_info(host->dev, "%s:ena_shift_range_min:%d,ena_shift_range_max:%d!\n",
			__func__, host->enable_shift_range[0],
			host->enable_shift_range[1]);
	}
}

/*
 * this function only add the config for
 * sdio sdr104 speedmode,it's for fix Complexity.
 */
static void config_sdr104_192m(struct device_node *np)
{
	if (of_find_property(np, "wifi_sdio_sdr104_192M", (int *)NULL)) {
		g_hs_dwmmc_caps[DW_MCI_SDIO_ID] |=
			(MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 |
			MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_SDR104);
		/* SDIO SDR104-192M bus_clock parameter */
		hs_timing_config[SDIO_TYPE][SDR104][CCLK_FRQ] = 960000000;
		hs_timing_config[SDIO_TYPE][SDR104][CCLK_DIV] = 4;
		hs_timing_config[SDIO_TYPE][SDR104][DRV_PHASE] = 3;
		hs_timing_config[SDIO_TYPE][SDR104][SAM_DLY] = 2;
		hs_timing_config[SDIO_TYPE][SDR104][SAM_PHASE_MAX] = 9;
		hs_timing_config[SDIO_TYPE][SDR104][BUS_HZ] = 192000000;
		pr_err("set sdio sdr104 192M\n");
	}
}

static void config_hs200_160m(struct device_node *np, struct dw_mci *host)
{
	u32 value_clock = 0;

	if (of_find_property(np, "sd_hs200_160M", (int *)NULL)) {
		/* parameters below are from practical testing */
		hs_timing_config[SD_TYPE][HS200][CCLK_FRQ] = 960000000;
		hs_timing_config[SD_TYPE][HS200][CCLK_DIV] = 5;
		if (of_property_read_u32(np, "clock-frequency-ext", &value_clock) == 0)
			hs_timing_config[SD_TYPE][HS200][DRV_PHASE] = value_clock;
		else
			hs_timing_config[SD_TYPE][HS200][DRV_PHASE] = 4;
		hs_timing_config[SD_TYPE][HS200][SAM_DLY] = 4;
		hs_timing_config[SD_TYPE][HS200][SAM_PHASE_MAX] = 11;
		hs_timing_config[SD_TYPE][HS200][SAM_PHASE_MIN] = 0;
		hs_timing_config[SD_TYPE][HS200][BUS_HZ] = 160000000;
		host->enable_shift_range[0] = 5;
		host->enable_shift_range[1] = 9;
		host->use_samdly_range[0] = 6;
		host->use_samdly_range[1] = 10;
	}
}
#ifdef CONFIG_MMC_DW_MUX_SDSIM
static void dw_mci_hs_mux_dt_sd_driver_strength(u32 *value,
	struct dw_mci *host, struct device_node *np)
{
	*value = SD_CLK_DRIVER_DEFAULT;
	if (of_property_read_u32(np, "driverstrength_clk", value)) {
		dev_info(host->dev, "driverstrength_clk property not found, using "
			"value of SD_CLK_DRIVER_DEFAULT as default\n");
		*value = SD_CLK_DRIVER_DEFAULT;
	}
	sd_clk_driver_strength = *value;
	dev_info(host->dev, "driverstrength_clk = 0x%x\n", sd_clk_driver_strength);

	*value = SD_CMD_DRIVER_DEFAULT;
	if (of_property_read_u32(np, "driverstrength_cmd", value)) {
		dev_info(host->dev, "driverstrength_cmd property not found, using "
			"value of SD_CMD_DRIVER_DEFAULT as default\n");
		*value = SD_CMD_DRIVER_DEFAULT;
	}
	sd_cmd_driver_strength = *value;
	dev_info(host->dev, "driverstrength_cmd = 0x%x\n", sd_cmd_driver_strength);

	*value = SD_DATA_DRIVER_DEFAULT;
	if (of_property_read_u32(np, "driverstrength_data", value)) {
		dev_info(host->dev, "driverstrength_data property not found, using "
			"value of SD_DATA_DRIVER_DEFAULT as default\n");
		*value = SD_DATA_DRIVER_DEFAULT;
	}
	sd_data_driver_strength = *value;
	dev_info(host->dev, "driverstrength_data = 0x%x\n", sd_data_driver_strength);
}

static void dw_mci_hs_mux_dt_switch_gpio_nums(u32 *value,
	struct dw_mci *host, struct device_node *np)
{
	*value = SWITCH_GPIO_DEFAULT_NUMBER;
	if (of_property_read_u32_index(np, "switch_gpio_nums", 0, value)) {
		dev_info(host->dev, "switch_gpio_number_0 property not found, "
			"using value of SWITCH_GPIO_DEFAULT_NUMBER as default\n");
		*value = SWITCH_GPIO_DEFAULT_NUMBER;
	}
	switch_gpio_number_0 = *value;
	dev_info(host->dev, "switch_gpio_number_0 = 0x%x\n", *value);

	*value = SWITCH_GPIO_DEFAULT_NUMBER;
	if (of_property_read_u32_index(np, "switch_gpio_nums", 1, value)) {
		dev_info(host->dev, "switch_gpio_number_1 property not found, "
			"using value of SWITCH_GPIO_DEFAULT_NUMBER as default\n");
		*value = SWITCH_GPIO_DEFAULT_NUMBER;
	}
	switch_gpio_number_1 = *value;
	dev_info(host->dev, "switch_gpio_number_1 = 0x%x\n", *value);

	*value = SWITCH_GPIO_DEFAULT_NUMBER;
	if (of_property_read_u32_index(np, "switch_gpio_nums", 2, value)) {
		dev_info(host->dev, "switch_gpio_number_2 property not found, "
			"using value of SWITCH_GPIO_DEFAULT_NUMBER as default\n");
		*value = SWITCH_GPIO_DEFAULT_NUMBER;
	}
	switch_gpio_number_2 = *value;
	dev_info(host->dev, "switch_gpio_number_2 = 0x%x\n", *value);

	*value = SWITCH_GPIO_DEFAULT_NUMBER;
	if (of_property_read_u32_index(np, "switch_gpio_nums", 3, value)) {
		dev_info(host->dev, "switch_gpio_number_3 property not found, "
			"using value of SWITCH_GPIO_DEFAULT_NUMBER as default\n");
		*value = SWITCH_GPIO_DEFAULT_NUMBER;
	}
	switch_gpio_number_3 = *value;
	dev_info(host->dev, "switch_gpio_number_3 = 0x%x\n", *value);
}

static void dw_mci_hs_mux_dt(struct dw_mci *host)
{
	struct dw_mci_hs_priv_data *priv = host->priv;
	struct device_node *np = host->dev->of_node;
	u32 value = 0;

	if (of_property_read_u32(np, "mux-sdsim", &value)) {
		dev_info(host->dev, "mux-sdsim property not found, using value of 0 as default\n");
		value = 0;
	}
	priv->mux_sdsim = value;
	if (of_property_read_u32(np, "mux-sdsim-seperate", &value)) {
		dev_info(host->dev, "mux-sdsim-seperate property not found, using value of 0 as default\n");
		value = 0;
	}
	priv->mux_sdsim_seperate = value;
	dev_info(host->dev, "dts mux-sdsim = %d\n", priv->mux_sdsim);
	dev_info(host->dev, "dts mux-sdsim-seperate = %d\n", priv->mux_sdsim_seperate);

	priv->mux_sdsim_vcc_status = MUX_SDSIM_VCC_STATUS_2_9_5_V;

	if (priv->mux_sdsim == 1) {
		value = 0;
		if (of_property_read_u32(np, "vmmcmosen_switch", &value)) {
			dev_info(host->dev, "vmmcmosen_switch property not found, using "
				"value of 0 as default\n");
			value = 0;
		}
		dev_info(host->dev, "vmmcmosen_switch = 0x%x\n", value);
		if (value == 0x01)
			host->sd_vmmcmosen_switch = 1;
		else
			host->sd_vmmcmosen_switch = 0;

		if (of_find_property(np, "pins_detect_enable", NULL)) {
			dev_info(host->dev, "pins_detect_enable found\n");
			host->flags |= PINS_DETECT_ENABLE;
		}

		dw_mci_hs_mux_dt_sd_driver_strength(&value, host, np);

		value = GPIO_DEFAULT_NUMBER_FOR_SD_CLK;
		if (of_property_read_u32(np, "gpionumber_sdclk", &value)) {
			dev_info(host->dev, "gpionumber_sdclk property not found, using "
				"value of GPIO_DEFAULT_NUMBER_FOR_SD_CLK as default\n");
			value = GPIO_DEFAULT_NUMBER_FOR_SD_CLK;
		}
		register_gpio_number_group(value);
		dev_info(host->dev, "gpionumber_sdclk = 0x%x\n", value);

		dw_mci_hs_mux_dt_switch_gpio_nums(&value, host, np);
	}
}
#endif

#ifdef CONFIG_MMC_SIM_GPIO_EXCHANGE
static void dw_mci_gpio_parse(struct dw_mci *host)
{
	struct dw_mci_hs_priv_data *priv = host->priv;
	struct device_node *np = host->dev->of_node;

	priv->set_sd_io = of_get_named_gpio(np, "set_sd_io", 0);
	if (!gpio_is_valid(priv->set_sd_io)) {
		dev_info(host->dev, "set_sd_io not available\n");
		priv->set_sd_io = -1;
	}

	priv->set_sd_io2 = of_get_named_gpio(np, "set_sd_io2", 0);
	if (!gpio_is_valid(priv->set_sd_io2)) {
		dev_info(host->dev, "set_sd_io2 not available\n");
		priv->set_sd_io2 = -1;
	}

	if (of_find_property(np, "caps2-mmc-gpio167-low", NULL)) {
		priv->set_sd_gpio167_low = 1;
		dev_info(host->dev, "caps2-mmc-gpio167-low is %d", priv->set_sd_gpio167_low);
	}
}
#endif

static void dw_mci_hs_sdio_parse(struct dw_mci *host)
{
	struct device_node *np = host->dev->of_node;
	struct dw_mci_hs_priv_data *priv = host->priv;
	u32 value = 0;

	if (of_find_property(np, "sd_support_1135", NULL)) {
		dev_info(host->dev, "sd_support_1135\n");
		host->flags |= SD_SUPPORT_1135;
		sdio_1135_host = host;
	}

	if (of_find_property(np, "sdio_support_uhs", NULL))
		g_hs_dwmmc_caps[DW_MCI_SDIO_ID] |= (MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 |
			MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_SDR104);

	if (of_property_read_u32(np, "debug_percrg_sd_sdio_reg_print", &value)) {
		dev_info(host->dev, "debug_percrg_sd_sdio_reg_print get value failed\n");
		value = 0;
	}

	if (value == CRG_PRINT)
		priv->crg_print = CRG_PRINT;
}

static void dw_mci_parse_mmc_bus_clk(struct dw_mci *host)
{
	struct dw_mci_hs_priv_data *priv = host->priv;
	struct device_node *np = host->dev->of_node;

	/* find out mmc_bus_clk supported for FPGA */
	if (of_property_read_u32(np, "board-mmc-bus-clk", &(priv->dw_mmc_bus_clk))) {
		dev_info(host->dev, "board mmc_bus_clk property not found, "
			"assuming asic board is available\n");

		priv->dw_mmc_bus_clk = 0;
	}
	dev_info(host->dev, "######board-mmc-bus-clk is %x\n", priv->dw_mmc_bus_clk);
}

static int dw_mci_hs_parse_dt(struct dw_mci *host)
{
	struct dw_mci_hs_priv_data *priv = host->priv;
	struct device_node *np = host->dev->of_node;
	u32 value = 0;
	int error;

	if (of_find_property(np, "timeout_enable_pio", NULL)) {
		dev_info(host->dev, "timeout_enable_pio\n");
		host->flags |= TIMEOUT_ENABLE_PIO;
	}

	if (of_find_property(np, "gpio_power_up", NULL)) {
		dev_info(host->dev, "enable gpio for io and device power\n");
		host->flags |= NANO_GPIO_POWER;
	}

	if (of_find_property(np, "slow_ioctl_support", NULL)) {
		dev_info(host->dev, "host support slow ioctl\n");
		host->flags |= SLOW_IOCTL;
	}

	if (of_property_read_u32(np, "is-resetable", &host->is_reset_after_retry))
		dev_info(host->dev, "is-resetable get value failed\n");

#ifdef CONFIG_MMC_SIM_GPIO_EXCHANGE
	dw_mci_gpio_parse(host);
#endif
	dw_mci_hs_sdio_parse(host);

	error = dw_mci_hs_get_dt_pltfm_resource(host, np);
	if (error)
		return error;

	dw_mci_hs_get_timing_config(host);

	config_sdr104_192m(np);
	config_hs200_160m(np, host);

	if (of_property_read_u32(np, "hisilicon,bus_hz", &value)) {
		dev_info(host->dev, "bus_hz property not found, using value of 0 as default\n");
		value = 0;
	}
	priv->priv_bus_hz = value;
	dev_info(host->dev, "dts bus_hz = %d\n", priv->priv_bus_hz);

	value = 0;
	if (of_property_read_u32(np, "cd-vol", &value)) {
		dev_info(host->dev, "cd-vol property not found, using value of 0 as default\n");
		value = 0;
	}
	priv->cd_vol = value;
	dev_info(host->dev, "dts cd-vol = %u\n", priv->cd_vol);

#ifdef CONFIG_MMC_DW_MUX_SDSIM
	dw_mci_hs_mux_dt(host);
#endif

	dw_mci_parse_mmc_bus_clk(host);

	return 0;
}

static irqreturn_t dw_mci_hs_card_detect(int irq, void *data)
{
	struct dw_mci *host = (struct dw_mci *)data;
	struct mmc_host *mmc_host_temp = NULL;
	struct dw_mci_slot *cur_slot = NULL;

	host->sd_reinit = 0;
	host->sd_hw_timeout = 0;
	host->flags &= ~DWMMC_IN_TUNING;
	host->flags &= ~DWMMC_TUNING_DONE;

	cur_slot = host->slot[0];
	mmc_host_temp = cur_slot->mmc;
	mmc_host_temp->init_cnt = 0;
	mmc_host_temp->reset_count = 0;

#ifdef CONFIG_HISI_DEBUG_FS
	if (mmc_host_temp->sim_remove_nano)
		mmc_host_temp->sim_remove_nano = 0;
#endif

	queue_work(host->card_workqueue, &host->card_work);
	return IRQ_HANDLED;
};

static int dw_mci_hs_get_cd(struct dw_mci *host, u32 slot_id)
{
	unsigned int status;
	struct dw_mci_hs_priv_data *priv = host->priv;
#if defined(CONFIG_HISI_DEBUG_FS) && defined(CONFIG_MMC_DW_MUX_SDSIM)
	struct dw_mci_slot *cur_slot = host->slot[0];
#endif

	/* cd_vol = 1 means sdcard gpio detect pin active-high */
	if (priv->cd_vol)
		status = !gpio_get_value(priv->gpio_cd);
	else /* cd_vol = 0 means sdcard gpio detect pin active-low */
		status = gpio_get_value(priv->gpio_cd);

	dev_info(host->dev, " sd status = %u from gpio_get_value(gpio_cd)\n", status);

	/* If sd to jtag func enabled, make the SD always not present */
	if ((host->hw_mmc_id == DWMMC_SD_ID) &&
		(dw_mci_check_himntn(HIMNTN_SD2JTAG) ||
			dw_mci_check_himntn(HIMNTN_SD2DJTAG) ||
			dw_mci_check_himntn(HIMNTN_SD2UART6))) {
		dev_info(host->dev, " sd status set to 1 here because jtag is enabled\n");
		status = 1;
	}

#ifdef CONFIG_MMC_DW_MUX_SDSIM
	if (host->hw_mmc_id == DWMMC_SD_ID) {
		if (!first_mux_sdsim_probe_init) {
			status = sd_sim_detect_run(host, status, MODULE_SD,
				SLEEP_MS_TIME_FOR_DETECT_UNSTABLE);
		} else {
			dev_info(host->dev, " jump sd_sim_detect_run because "
				"first_mux_sdsim_probe_init now\n");
			first_mux_sdsim_probe_init = 0;
		}
#ifdef CONFIG_HISI_DEBUG_FS
		if (cur_slot->mmc->sim_remove_nano) {
			sd_sim_detect_status_current = SD_SIM_DETECT_STATUS_UNDETECTED;
			dev_info(host->dev, " nano sd remove test: sd_sim_detect_status_current = 0\n");
		}
#endif
	}
#endif

	dev_info(host->dev, " sd status = %u\n", status);
	return status;
}

static irqreturn_t dw_mci_hs_hpd_detect(int irq, void *data)
{
	return IRQ_HANDLED;
}

static int dw_mci_hs_cd_detect_init(struct dw_mci *host)
{
	struct device_node *np = host->dev->of_node;
	struct device_node *np_wifi_only = NULL;
	int gpio;
	int err;
	int hpd_irq;
	const char *value = NULL;

	if (host->pdata->quirks & DW_MCI_QUIRK_BROKEN_CARD_DETECTION)
		return 0;

	np_wifi_only = of_find_compatible_node(NULL, NULL, "singleap,wifionly");
	if (host->hw_mmc_id == DWMMC_SD_ID && np_wifi_only) {
		if (!of_property_read_string(np_wifi_only, "is_wifionly", &value))
			pr_info("mmc find singleap,wifionly, value: %s\n", value);
	}

	if (value && !strcmp(value, "1")) {
		hpd_irq = pmic_get_irq_byname(SIM0_HPD_F);
		host->flags |= WIFI_ONLY_GPIO_NP;
		err = request_irq(hpd_irq, dw_mci_hs_hpd_detect,
					IRQF_SHARED | IRQF_NO_SUSPEND, "sd_hpd", host);
		if (err)
			pr_err("%s: request hpd irq error err %d\n", __func__, err);
	}

	gpio = of_get_named_gpio(np, "cd-gpio", 0);
	if (gpio_is_valid(gpio)) {
		if (devm_gpio_request_one(host->dev, gpio, GPIOF_IN, "dw-mci-cd")) {
			dev_err(host->dev, "gpio [%d] request failed\n", gpio);
		} else {
			struct dw_mci_hs_priv_data *priv = host->priv;
			u32 shared_irq = 0;

			priv->gpio_cd = gpio;
			host->pdata->get_cd = dw_mci_hs_get_cd;
			if (of_property_read_u32(np, "shared-irq", &shared_irq)) {
				dev_info(host->dev, "shared-irq property not found, using "
					"shared_irq of 0 as default\n");
				shared_irq = 0;
			}

			if (shared_irq) {
				err = devm_request_irq(host->dev,
					gpio_to_irq(gpio), dw_mci_hs_card_detect,
					IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING |
					IRQF_NO_SUSPEND | IRQF_SHARED,
					DRIVER_NAME, host);
			} else {
				err = devm_request_irq(host->dev,
					gpio_to_irq(gpio), dw_mci_hs_card_detect,
					IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
					DRIVER_NAME, host);
			}

			if (err)
				dev_warn(mmc_dev(host->dev), "request gpio irq error\n");
		}
	} else {
		dev_info(host->dev, "cd gpio not available");
	}

	return 0;
}

static int hs_dwmmc_card_busy(struct dw_mci *host)
{
	if ((mci_readl(host, STATUS) & SDMMC_STATUS_BUSY) || host->cmd ||
		host->data || host->mrq || (host->state != STATE_IDLE)) {
		dev_vdbg(host->dev, " card is busy!");
		return 1;
	}

	return 0;
}

static int dw_mci_3_3v_signal_voltage_switch(struct dw_mci_slot *slot)
{
	struct dw_mci *host = slot->host;
#ifdef CONFIG_MMC_DW_MUX_SDSIM
	struct dw_mci_hs_priv_data *priv = host->priv;
#endif
	u32 reg;
	int ret = 0;
	u32 id = (u32)(slot->id);

#ifdef CONFIG_MMC_DW_MUX_SDSIM
	if (host->hw_mmc_id == DWMMC_SD_ID && priv->mux_sdsim)
		return ret;
#endif
	ret = dw_mci_set_sel18(host, 0);
	if (ret) {
		dev_err(host->dev, " dw_mci_set_sel18 error!\n");
		return ret;
	}
	usleep_range(5000, 5500);

	if (host->vqmmc) {
		/* set vqmmc 2.95V */
		ret = regulator_set_voltage(host->vqmmc, 2950000, 2950000);
		if (ret) {
			dev_warn(host->dev, "Switching to 3.3V signalling voltage failed\n");
			return -EIO;
		}
	} else {
		reg = mci_readl(slot->host, UHS_REG);
		reg &= ~((u32)(0x1 << id));
		mci_writel(slot->host, UHS_REG, reg);
	}

	if (host->lever_shift)
		dw_mci_lever_shit_voltage(LEVER_SHIFT_3_0V, LEVER_SHIFT_GPIO);

	/* Wait for 5ms */
	usleep_range(5000, 5500);

	return ret;
}
static void dw_mci_enable_clock(struct dw_mci *host, int enable)
{
	unsigned long loop_count = VOL_SWITCH_LOOP_MAX;

	/* start/stop clock */
	mci_writel(host, CLKENA, enable);
	mci_writel(host, CMD, SDMMC_CMD_ONLY_CLK | SDMMC_CMD_VOLT_SWITCH);

	do {
		if (!(mci_readl(host, CMD) & SDMMC_CMD_START))
			break;
		loop_count--;
	} while (loop_count);

	if (!loop_count)
		dev_err(host->dev, "%s  %s clock failed\n", __func__, enable ? "start":"stop");
}

static int dw_mci_1_8v_signal_voltage_switch(struct dw_mci_slot *slot)
{
	unsigned long loop_count = VOL_SWITCH_LOOP_MAX;
	struct dw_mci *host = slot->host;
	int ret;
	int intrs;

	/*
	 * disable interrupt upon voltage switch. handle interrupt here
	 * and DO NOT triggle irq
	 */
	mci_writel(host, CTRL, (mci_readl(host, CTRL) & ~SDMMC_CTRL_INT_ENABLE));

	/* stop clock */
	dw_mci_enable_clock(host, 0x0);
	mmiowb();

	if (host->vqmmc && !host->lever_shift) {
		/* set vqmmc 1.8V */
		ret = regulator_set_voltage(host->vqmmc, 1800000, 1800000);
		if (ret) {
			dev_warn(host->dev, "Switching to 1.8V signalling voltage failed\n");
			return -EIO;
		}
	}

	if (host->lever_shift)
		dw_mci_lever_shit_voltage(LEVER_SHIFT_1_8V, LEVER_SHIFT_GPIO);

	usleep_range(10000, 10500);

	ret = dw_mci_set_sel18(host, 1);
	if (ret) {
		dev_err(host->dev, " dw_mci_set_sel18 error!\n");
		return ret;
	}

	/* start clock */
	dw_mci_enable_clock(host, 0x1);

	/* poll cd interrupt */
	loop_count = VOL_SWITCH_LOOP_MAX;
	do {
		intrs = mci_readl(host, RINTSTS);
		if ((unsigned int)intrs & SDMMC_INT_CMD_DONE) {
			dev_err(host->dev, " cd 0x%x in voltage_switch\n", intrs);
			mci_writel(host, RINTSTS, intrs);
			break;
		}
		loop_count--;
	} while (loop_count);

	if (!loop_count)
		dev_err(host->dev, " voltage_switch exceed loop_count\n");

	/* enable interrupt */
	mci_writel(host, CTRL, (mci_readl(host, CTRL) | SDMMC_CTRL_INT_ENABLE));

	mmiowb();

	return ret;
}

static int dw_mci_priv_voltage_switch(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct dw_mci_slot *slot = mmc_priv(mmc);
	int ret = 0;

	/* only sd need to switch voltage */
	if (slot->host->hw_mmc_id != DWMMC_SD_ID)
		return ret;

	/* only sd need to switch voltage */
	if (slot->host->flags & SD_SUPPORT_1135)
		return ret;

	pm_runtime_get_sync(mmc_dev(mmc));

	if (ios->signal_voltage == MMC_SIGNAL_VOLTAGE_330)
		ret = dw_mci_3_3v_signal_voltage_switch(slot);
	else if (ios->signal_voltage == MMC_SIGNAL_VOLTAGE_180)
		ret = dw_mci_1_8v_signal_voltage_switch(slot);

	pm_runtime_mark_last_busy(mmc_dev(mmc));
	pm_runtime_put_autosuspend(mmc_dev(mmc));

	return ret;
}

void dw_mci_hi3xxx_work_fail_reset(struct dw_mci *host)
{
	struct dw_mci_hs_priv_data *priv = host->priv;

	if ((priv->id != DW_MCI_SD_ID) && (priv->id != DW_MCI_SDIO_ID)) {
		dev_err(host->dev, "Not support now, return\n");
		return;
	}

	dev_err(host->dev, "Start to reset SDIO IP\n");
	mci_writel(host, CTRL,
		(mci_readl(host, CTRL) & (~(unsigned int)INT_ENABLE)));
	mci_writel(host, INTMASK, 0);

	mci_writel(host, RINTSTS, INTMSK_ALL);

#ifdef CONFIG_MMC_DW_IDMAC
	if (host->dma_64bit_address == SDMMC_32_BIT_DMA)
		mci_writel(host, IDSTS, IDMAC_INT_CLR);
	else
		mci_writel(host, IDSTS64, IDMAC_INT_CLR);
#endif

	dw_mci_hi3xxx_reset_restore(host);
}

static void dw_mci_hs_tuning_set_flags(struct dw_mci *host, int sample, int ok)
{
	if (ok)
		host->tuning_sample_flag |= (1 << (unsigned int)sample);
	else
		host->tuning_sample_flag &=
			~((unsigned int)(1 << (unsigned int)sample));
}

static void dw_mci_hs_tuning_set_current_state(struct dw_mci *host, int ok)
{
	dw_mci_hs_tuning_set_flags(host, host->tuning_current_sample, ok);
}

#ifdef CONFIG_MMC_DW_SD_CLK_SLOWDOWN
static int dw_mci_hs_slowdown_clk(struct dw_mci *host, int timing)
{
	struct dw_mci_hs_priv_data *priv = host->priv;
	int id = priv->id;

	host->current_div += 2; /* freq div ratio hardware constrains */

	/* at most slow down up to half of original freq */
	if (host->current_div > 2 * hs_timing_config[id][timing][CCLK_DIV]) {
		host->current_div = 2 * hs_timing_config[id][timing][CCLK_DIV];
		return -1;
	}
	dev_info(host->dev, "begin slowdown clk, current_div=%d\n", host->current_div);

	dw_mci_hs_set_timing(host, id, timing, host->tuning_init_sample, host->current_div);

	return 0;
}
#endif

int dw_mci_priv_execute_tuning(struct dw_mci_slot *slot, u32 opcode,
	struct dw_mci_tuning_data *tuning_data)
{
	return dw_mci_priv_execute_tuning_helper(slot, opcode, tuning_data);
}

static int dw_mci_hs_tuning_move(struct dw_mci *host, int timing, int start)
{
	return dw_mci_hs_tuning_move_helper(host, timing, start);
}

static int dw_mci_hs_tuning_find_condition(struct dw_mci *host, int timing)
{
	return dw_mci_hs_tuning_find_condition_helper(host, timing);
}

static const struct dw_mci_drv_data hs_drv_data = {
	.caps = g_hs_dwmmc_caps,
	.init = dw_mci_hs_priv_init,
	.set_ios = dw_mci_hs_set_ios,
	.setup_clock = dw_mci_hs_setup_clock,
	.prepare_command = dw_mci_hs_prepare_command,
	.parse_dt = dw_mci_hs_parse_dt,
	.cd_detect_init = dw_mci_hs_cd_detect_init,
	.tuning_find_condition = dw_mci_hs_tuning_find_condition,
	.tuning_set_current_state = dw_mci_hs_tuning_set_current_state,
	.tuning_move = dw_mci_hs_tuning_move,
#ifdef CONFIG_MMC_DW_SD_CLK_SLOWDOWN
	.slowdown_clk = dw_mci_hs_slowdown_clk,
#endif
	.execute_tuning = dw_mci_priv_execute_tuning,
	.start_signal_voltage_switch = dw_mci_priv_voltage_switch,
	.work_fail_reset = dw_mci_hi3xxx_work_fail_reset,
};

static const struct of_device_id dw_mci_hs_match[] = {
	{
		.compatible = "hisilicon,aries-dw-mshc",
		.data = &hs_drv_data,
	},
	{
		.compatible = "hisilicon,cancer-dw-mshc",
		.data = &hs_drv_data,
	},
	{
		.compatible = "hisilicon,libra-dw-mshc",
		.data = &hs_drv_data,
	},
	{
		.compatible = "hisilicon,taurus-dw-mshc",
		.data = &hs_drv_data,
	},
	{
		.compatible = "hisilicon,capricorn-dw-mshc",
		.data = &hs_drv_data,
	},
	{
		.compatible = "hisilicon,pisces-dw-mshc",
		.data = &hs_drv_data,
	},
	{
		.compatible = "hisilicon,leo-dw-mshc",
		.data = &hs_drv_data,
	},
	{
		.compatible = "hisilicon,sagittarius-dw-mshc",
		.data = &hs_drv_data,
	},
	{},
};

MODULE_DEVICE_TABLE(of, dw_mci_hs_match);

int dw_mci_hs_probe(struct platform_device *pdev)
{
	const struct dw_mci_drv_data *drv_data = NULL;
	const struct of_device_id *match = NULL;
	int err;

#if defined(CONFIG_HISI_DEBUG_FS) && (!defined(CONFIG_PRODUCT_ARMPC))
	proc_sd_test_init();
#endif

	match = of_match_node(dw_mci_hs_match, pdev->dev.of_node);
	if (!match)
		return -1;
	drv_data = match->data;

	err = dw_mci_hs_get_resource();
	if (err)
		return err;

	err = dw_mci_pltfm_register(pdev, drv_data);
	if (err)
		return err;

	/* when sdio1 used for via modem, disable pm runtime */
	if (!of_property_read_bool(pdev->dev.of_node, "modem_sdio_enable")) {
		pm_runtime_set_active(&pdev->dev);
		pm_runtime_enable(&pdev->dev);
		pm_runtime_set_autosuspend_delay(&pdev->dev, 50);
		pm_runtime_use_autosuspend(&pdev->dev);
		pm_suspend_ignore_children(&pdev->dev, 1);
	} else {
		pr_info("%s mmc/sdio device support via modem, disable "
			"pm_runtime on this device\n", __func__);
	}
#ifdef CONFIG_HUAWEI_DSM
	if (!dclient)
		dclient = dsm_register_client(&dsm_dw_mmc);
#endif
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int dw_mci_hs_suspend(struct device *dev)
{
	int ret;
	struct dw_mci *host = dev_get_drvdata(dev);
	struct dw_mci_hs_priv_data *priv = host->priv;

	dev_info(host->dev, " %s ++\n", __func__);

	pm_runtime_get_sync(dev);

	if (priv->gpio_cd) {
		disable_irq(gpio_to_irq(priv->gpio_cd));
		cancel_work_sync(&host->card_work);
		dev_info(host->dev, " disable gpio detect\n");
	}

	ret = dw_mci_suspend(host);
	dev_info(host->dev, " dw_mci_suspend, ret = %d\n", ret);

	priv->old_timing = -1;
	priv->old_power_mode = MMC_POWER_OFF;
	if (!IS_ERR(host->biu_clk))
		clk_disable_unprepare(host->biu_clk);

	if (!IS_ERR(host->ciu_clk))
		clk_disable_unprepare(host->ciu_clk);

#ifdef CONFIG_SD_TIMEOUT_RESET
	if (!IS_ERR(host->volt_hold_sd_clk) &&
		(host->volt_hold_clk_sd == VOLT_HOLD_CLK_08V))
		clk_disable_unprepare(host->volt_hold_sd_clk);

	if (!IS_ERR(host->volt_hold_sdio_clk) &&
		(host->volt_hold_clk_sdio == VOLT_HOLD_CLK_08V))
		clk_disable_unprepare(host->volt_hold_sdio_clk);
#endif

	dw_mci_hs_set_controller(host, DW_MCI_HS_DO_RESET);

	host->current_speed = 0;

	pm_runtime_mark_last_busy(dev);
	pm_runtime_put_autosuspend(dev);

	dev_info(host->dev, " %s --\n", __func__);

	return 0;
}

#ifdef CONFIG_SD_TIMEOUT_RESET
void dw_mci_set_clk_08v(struct dw_mci *host)
{
	if (!IS_ERR(host->volt_hold_sd_clk) &&
		(host->volt_hold_clk_sd == VOLT_HOLD_CLK_08V)) {
		if (clk_prepare_enable(host->volt_hold_sd_clk))
			dev_err(host->dev, "volt_hold_sd_clk clk_prepare_enable failed\n");
	}

	if (!IS_ERR(host->volt_hold_sdio_clk) &&
		(host->volt_hold_clk_sdio == VOLT_HOLD_CLK_08V)) {
		if (clk_prepare_enable(host->volt_hold_sdio_clk))
			dev_err(host->dev, "volt_hold_sdio_clk clk_prepare_enable failed\n");
	}
}
#endif

static int dw_mci_hs_resume(struct device *dev)
{
	int ret;
	unsigned int i;
	struct dw_mci *host = dev_get_drvdata(dev);
	struct dw_mci_hs_priv_data *priv = host->priv;

	dev_info(host->dev, " %s ++\n", __func__);

	pm_runtime_get_sync(dev);

	if (!IS_ERR(host->biu_clk)) {
		if (clk_prepare_enable(host->biu_clk))
			dev_err(host->dev, "biu_clk clk_prepare_enable failed\n");
	}

	if (!IS_ERR(host->ciu_clk)) {
		if (clk_prepare_enable(host->ciu_clk))
			dev_err(host->dev, "ciu_clk clk_prepare_enable failed\n");
	}

#ifdef CONFIG_SD_TIMEOUT_RESET
	dw_mci_set_clk_08v(host);
#endif

	dw_mci_hs_set_controller(host, DW_MCI_HS_DO_UNRESET);

	for (i = 0; i < host->num_slots; i++) {
		struct dw_mci_slot *slot = host->slot[i];

		if (!slot)
			continue;

		if (slot->mmc->pm_flags & MMC_PM_KEEP_POWER) {
			priv->in_resume = STATE_KEEP_PWR;
		} else {
			host->flags &= ~DWMMC_IN_TUNING;
			host->flags &= ~DWMMC_TUNING_DONE;
		}
	}

	/* restore controller specified setting */
	dw_mci_hs_priv_setting(host);
	ret = dw_mci_resume(host);
	if (ret)
		return ret;

	priv->in_resume = STATE_LEGACY;

	if (priv->gpio_cd) {
		enable_irq(gpio_to_irq(priv->gpio_cd));
		dev_info(host->dev, " enable gpio detect\n");
	}

	pm_runtime_mark_last_busy(dev);
	pm_runtime_put_autosuspend(dev);

	dev_info(host->dev, " %s --\n", __func__);

	return 0;
}
#endif

#ifdef CONFIG_PM
static int dw_mci_hs_runtime_suspend(struct device *dev)
{
	struct dw_mci *host = dev_get_drvdata(dev);

	dev_vdbg(host->dev, " %s ++\n", __func__);

	if (hs_dwmmc_card_busy(host)) {
		dev_err(host->dev, " %s: card is busy\n", __func__);
		return -EBUSY;
	}

	if (!dw_mci_wait_host_busy(host, 0)) {
		dev_err(host->dev, " %s: host mc is busy\n", __func__);
		return -EBUSY;
	}

	if (!IS_ERR(host->biu_clk))
		clk_disable_unprepare(host->biu_clk);

	if (!IS_ERR(host->ciu_clk))
		clk_disable_unprepare(host->ciu_clk);

#ifdef CONFIG_SD_TIMEOUT_RESET
	if (!IS_ERR(host->volt_hold_sd_clk) &&
		(host->volt_hold_clk_sd == VOLT_HOLD_CLK_08V))
		clk_disable_unprepare(host->volt_hold_sd_clk);

	if (!IS_ERR(host->volt_hold_sdio_clk) &&
		(host->volt_hold_clk_sdio == VOLT_HOLD_CLK_08V))
		clk_disable_unprepare(host->volt_hold_sdio_clk);
#endif

	dev_vdbg(host->dev, " %s --\n", __func__);

	return 0;
}

static int dw_mci_hs_runtime_resume(struct device *dev)
{
	struct dw_mci *host = dev_get_drvdata(dev);

	dev_vdbg(host->dev, " %s ++\n", __func__);

	if (!IS_ERR(host->biu_clk)) {
		if (clk_prepare_enable(host->biu_clk))
			dev_err(host->dev, "biu_clk clk_prepare_enable failed\n");
	}

	if (!IS_ERR(host->ciu_clk)) {
		if (clk_prepare_enable(host->ciu_clk))
			dev_err(host->dev, "ciu_clk clk_prepare_enable failed\n");
	}

#ifdef CONFIG_SD_TIMEOUT_RESET
	if (!IS_ERR(host->volt_hold_sd_clk) &&
		(host->volt_hold_clk_sd == VOLT_HOLD_CLK_08V)) {
		if (clk_prepare_enable(host->volt_hold_sd_clk))
			dev_err(host->dev, "volt_hold_sd_clk clk_prepare_enable failed\n");
	}

	if (!IS_ERR(host->volt_hold_sdio_clk) &&
		(host->volt_hold_clk_sdio == VOLT_HOLD_CLK_08V)) {
		if (clk_prepare_enable(host->volt_hold_sdio_clk))
			dev_err(host->dev, "volt_hold_sdio_clk clk_prepare_enable failed\n");
	}
#endif

	dev_vdbg(host->dev, " %s --\n", __func__);
	return 0;
}
#endif

static const struct dev_pm_ops dw_mci_hs_pmops = {
	SET_SYSTEM_SLEEP_PM_OPS(dw_mci_hs_suspend, dw_mci_hs_resume)
	SET_RUNTIME_PM_OPS(dw_mci_hs_runtime_suspend,
		dw_mci_hs_runtime_resume, NULL)
};

static struct platform_driver dw_mci_hs_pltfm_driver = {
	.probe = dw_mci_hs_probe,
	.remove = dw_mci_pltfm_remove,
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = of_match_ptr(dw_mci_hs_match),
		.pm = &dw_mci_hs_pmops,
	},
};

module_platform_driver(dw_mci_hs_pltfm_driver);

MODULE_DESCRIPTION("Hisilicon Specific DW-MSHC Driver Extension");
MODULE_LICENSE("GPL v2");
