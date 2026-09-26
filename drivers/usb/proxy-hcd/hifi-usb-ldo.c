/*
 * hifi-usb-ldo.c
 *
 * utilityies for hifi-usb ldo
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include "hifi-usb-ldo.h"

#include <linux/device.h>
#include <linux/hisi/h2x_interface.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/platform_device.h>
#include <linux/types.h>

#include "hifi-usb.h"
#include "proxy-hcd.h"

#define dbg(format, arg...) pr_debug("[phcd][DBG][%s]" format, __func__, ##arg)
#define info(format, arg...) pr_info("[phcd][INFO][%s]" format, __func__, ##arg)
#define err(format, arg...) pr_err("[phcd][ERR][%s]" format, __func__, ##arg)

static void __hifi_usb_phy_ldo_on(struct hifi_usb_phy_ldo_cfg *cfg)
{
	uint32_t temp;

	if (cfg->accessable) {
		temp = pmic_read_reg(cfg->addr);
		temp &= ~((unsigned int)(1 << cfg->bit));
		pmic_write_reg(cfg->addr, temp);
		info("phy_regulator: 0x%x\n", pmic_read_reg(cfg->addr));
	}
}

static void __hifi_usb_phy_ldo_always_on(struct hifi_usb_phy_ldo_cfg *cfg)
{
	if (cfg->always_on)
		__hifi_usb_phy_ldo_on(cfg);
}

void hifi_usb_phy_ldo_always_on(struct hifi_usb_proxy *hifi_usb)
{
	struct hifi_usb_proxy *hifiusb = hifi_usb;

	if (hifiusb == NULL) {
		err("hifi_usb NULL, hifiusb not initialized or exited\n");
		return;
	}

	__hifi_usb_phy_ldo_always_on(&hifiusb->hifi_usb_phy_ldo_33v);
	__hifi_usb_phy_ldo_always_on(&hifiusb->hifi_usb_phy_ldo_18v);
}

void hifi_usb_phy_ldo_on(struct hifi_usb_proxy *hifi_usb)
{
	struct hifi_usb_proxy *hifiusb = hifi_usb;

	if (hifiusb == NULL) {
		err("hifi_usb NULL, hifiusb not initialized or exited\n");
		return;
	}
	__hifi_usb_phy_ldo_on(&hifiusb->hifi_usb_phy_ldo_33v);
	__hifi_usb_phy_ldo_on(&hifiusb->hifi_usb_phy_ldo_18v);
}

static void __hifi_usb_phy_ldo_force_auto(struct hifi_usb_phy_ldo_cfg *cfg)
{
	uint32_t temp;

	if (cfg->accessable != 0) {
		temp = pmic_read_reg(cfg->addr);
		temp |= (1 << cfg->bit);
		pmic_write_reg(cfg->addr, temp);
		info("phy_regulator: 0x%x\n", pmic_read_reg(cfg->addr));
	}
}

static void __hifi_usb_phy_ldo_auto(struct hifi_usb_phy_ldo_cfg *cfg)
{
	if (cfg->always_on == 0)
		__hifi_usb_phy_ldo_force_auto(cfg);
}

void hifi_usb_phy_ldo_auto(struct hifi_usb_proxy *hifi_usb)
{
	struct hifi_usb_proxy *hifiusb = hifi_usb;

	if (hifiusb == NULL) {
		err("hifi_usb NULL, hifiusb not initialized or exited\n");
		return;
	}

	__hifi_usb_phy_ldo_auto(&hifiusb->hifi_usb_phy_ldo_33v);
	__hifi_usb_phy_ldo_auto(&hifiusb->hifi_usb_phy_ldo_18v);
}

void hifi_usb_phy_ldo_force_auto(struct hifi_usb_proxy *hifi_usb)
{
	struct hifi_usb_proxy *hifiusb = hifi_usb;

	if (hifiusb == NULL) {
		err("hifi_usb NULL, hifiusb not initialized or exited\n");
		return;
	}

	__hifi_usb_phy_ldo_force_auto(&hifiusb->hifi_usb_phy_ldo_33v);
	__hifi_usb_phy_ldo_force_auto(&hifiusb->hifi_usb_phy_ldo_18v);
}

void hifi_usb_phy_ldo_init(struct hifi_usb_proxy *hifiusb)
{
	struct proxy_hcd_client     *client = hifiusb->client;
	struct proxy_hcd        *phcd = client_to_phcd(client);
	struct platform_device      *pdev = phcd->pdev;
	struct device_node      *node = pdev->dev.of_node;
	u32 ldo_cfg_buf[MAX_IDO_CFG_BUFF_LEN] = {0};
	int ret;

	ret = of_property_read_u32_array(node, "hifi_usb_phy_ldo_33v",
					ldo_cfg_buf, MAX_IDO_CFG_BUFF_LEN);
	if (ret < 0) {
		info("get hifi_usb_phy_ldo_33v cfg err ret %d\n", ret);
		hifiusb->hifi_usb_phy_ldo_33v.accessable = 0;
	} else {
		info("hifi_usb_phy_ldo_33v cfg 0x%x 0x%x 0x%x 0x%x\n", ldo_cfg_buf[0],
			ldo_cfg_buf[1], ldo_cfg_buf[2], ldo_cfg_buf[3]);
		hifiusb->hifi_usb_phy_ldo_33v.addr = ldo_cfg_buf[0];
		hifiusb->hifi_usb_phy_ldo_33v.bit = ldo_cfg_buf[1];
		hifiusb->hifi_usb_phy_ldo_33v.always_on = ldo_cfg_buf[2]; /* bit2 store always_on setting */
		if (hifiusb->hifi_usb_phy_ldo_33v.addr <= MAX_PHY_IDO_ADDR)
			hifiusb->hifi_usb_phy_ldo_33v.accessable = ldo_cfg_buf[3]; /* bit3 store accessable setting */
		else
			hifiusb->hifi_usb_phy_ldo_33v.accessable = 0;
	}

	ret = of_property_read_u32_array(node, "hifi_usb_phy_ldo_18v",
					ldo_cfg_buf, MAX_IDO_CFG_BUFF_LEN);
	if (ret < 0) {
		info("get hifi_usb_phy_ldo_18v cfg err ret %d\n", ret);
		hifiusb->hifi_usb_phy_ldo_18v.accessable = 0;
	} else {
		info("hifi_usb_phy_ldo_18v cfg 0x%x 0x%x 0x%x 0x%x\n", ldo_cfg_buf[0],
			ldo_cfg_buf[1], ldo_cfg_buf[2], ldo_cfg_buf[3]);
		hifiusb->hifi_usb_phy_ldo_18v.addr = ldo_cfg_buf[0];
		hifiusb->hifi_usb_phy_ldo_18v.bit = ldo_cfg_buf[1];
		hifiusb->hifi_usb_phy_ldo_18v.always_on = ldo_cfg_buf[2]; /* bit2 store always_on setting */
		if (hifiusb->hifi_usb_phy_ldo_18v.addr < MAX_PHY_IDO_ADDR)
			hifiusb->hifi_usb_phy_ldo_18v.accessable = ldo_cfg_buf[3]; /* bit3 store accessable setting */
		else
			hifiusb->hifi_usb_phy_ldo_18v.accessable = 0;
	}
}

