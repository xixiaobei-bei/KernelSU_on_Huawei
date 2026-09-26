/*
 * Platform Dependent file for Hikey
 *
 * Copyright (C) 2022, Broadcom.
 *
 *      Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 *
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/skbuff.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/of_gpio.h>
#include <dhd_plat.h>

#include <dhd_dbg.h>
#include <dhd.h>

#include <linux/mmc/host.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/dw_mmc.h>
#ifdef CONFIG_HWCONNECTIVITY
#include <huawei_platform/connectivity/hw_connectivity.h>
#endif

#include <linux/random.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/mtd/hisi_nve_interface.h>

unsigned char g_wifimac[ETHER_ADDR_LEN] = {0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char g_nvebuf[ETHER_ADDR_LEN] = {0x00,0x00,0x00,0x00,0x00,0x00};
int l_ret = -1;
int l_sum = 0;

#define NV_WLAN_NUM          193
#define WLAN_VALID_SIZE      17
#define NV_WLAN_VALID_SIZE   12

#ifdef CONFIG_BROADCOM_WIFI_RESERVED_MEM
extern int dhd_init_wlan_mem(void);
extern void *dhd_wlan_mem_prealloc(int section, unsigned long size);
#endif /* CONFIG_BROADCOM_WIFI_RESERVED_MEM */

#define WLAN_REG_ON_GPIO		491
#define WLAN_HOST_WAKE_GPIO		493

static int wlan_reg_on = -1;
#define DHD_DT_COMPAT_ENTRY		"android,bcmdhd_wlan"
#define WIFI_WL_REG_ON_PROPNAME		"wl_reg_on"

static int wlan_host_wake_up = -1;
static int wlan_host_wake_irq = 0;
#define WIFI_WLAN_HOST_WAKE_PROPNAME    "wl_host_wake"

#ifdef VENDOR_HW_UPDATE
int dhd_wlan_power(int onoff);
extern void dhd_init_custom_dts(struct device_node * node);
#endif

static void read_from_global_buf(unsigned char * buf)
{
	memcpy(buf,g_wifimac,ETHER_ADDR_LEN);
	printk("get MAC from g_wifimac: mac=%02x:%02x:**:**:%02x:%02x\n",buf[0],buf[1],buf[4],buf[5]);
	return;
}

int char2byte( char* strori, char* outbuf )
{
    int i = 0;
    int temp = 0;
    int sum = 0;

    for( i = 0; i < 12; i++ )
    {
        switch (strori[i]) {
            case '0' ... '9':
                temp = strori[i] - '0';
                break;

            case 'a' ... 'f':
                temp = strori[i] - 'a' + 10;
                break;

            case 'A' ... 'F':
                temp = strori[i] - 'A' + 10;
                break;
        }

        sum += temp;
        if( i % 2 == 0 ){
            outbuf[i/2] |= temp << 4;
        }
        else{
            outbuf[i/2] |= temp;
        }
    }

    return sum;
}

static bool validate_wifi_addr(unsigned char macAddr)
{
    unsigned char mac1 = macAddr & 0x0F;

    // legal wifi mac, mac1 should be 0,4,8 or ox0c
    if (mac1 == 0x00 || mac1 == 0x04 || mac1 == 0x08 || mac1 == 0x0c) {
        return true;
    }
    printk("%s: illeagle wifi address: %02x", __func__, macAddr);
    return false;
}

static int dhd_wlan_get_mac_addr(unsigned char *buf)
{
    struct hisi_nve_info_user st_info;

    if (NULL == buf) {
        printk("%s: dhd_wlan_get_mac_addr failed\n", __func__);
        return -1;
    }

    memset(buf, 0, ETHER_ADDR_LEN);
    memset(&st_info, 0, sizeof(st_info));
    st_info.nv_number  = NV_WLAN_NUM;   //nve item

    strncpy(st_info.nv_name, "MACWLAN", sizeof("MACWLAN"));

    st_info.valid_size = NV_WLAN_VALID_SIZE;
    st_info.nv_operation = NV_READ;

    if (l_ret) {
        l_ret = hisi_nve_direct_access(&st_info);
    }

    if (!l_ret)
    {
        if (!l_sum) {
            l_sum = char2byte(st_info.nv_data, g_nvebuf);
        }
        if (l_sum != 0 && validate_wifi_addr(g_nvebuf[0]))
        {
            printk("get MAC from NV: mac=%02x:**:**:**:%02x:%02x\n",g_nvebuf[0],g_nvebuf[4],g_nvebuf[5]);
            memcpy(g_wifimac, g_nvebuf, ETHER_ADDR_LEN);
        }else{
            get_random_bytes(buf,ETHER_ADDR_LEN);
            buf[0] = 0x0;
            printk("get MAC from Random: mac=%02x:**:**:**:%02x:%02x\n",buf[0],buf[4],buf[5]);
            memcpy(g_wifimac,buf,ETHER_ADDR_LEN);
        }
    }else{
        get_random_bytes(buf,ETHER_ADDR_LEN);
        buf[0] = 0x0;
        printk("get MAC from Random: mac=%02x:**:**:**:%02x:%02x\n",buf[0],buf[4],buf[5]);
        memcpy(g_wifimac,buf,ETHER_ADDR_LEN);
    }

    if (0 != g_wifimac[0] || 0 != g_wifimac[1] || 0 != g_wifimac[2] || 0 != g_wifimac[3]|| 0 != g_wifimac[4] || 0 != g_wifimac[5]){
        read_from_global_buf(buf);
        return 0;
    }

    return 0;
}

int
dhd_wifi_init_gpio(void)
{
	int gpio_reg_on_val;
	/* ========== WLAN_PWR_EN ============ */
	char *wlan_node = DHD_DT_COMPAT_ENTRY;
	struct device_node *root_node = NULL;

	root_node = of_find_compatible_node(NULL, NULL, wlan_node);
	if (root_node) {
		wlan_reg_on = of_get_named_gpio(root_node, WIFI_WL_REG_ON_PROPNAME, 0);
		wlan_host_wake_up = of_get_named_gpio(root_node, WIFI_WLAN_HOST_WAKE_PROPNAME, 0);
	} else {
		DHD_ERROR(("failed to get device node of BRCM WLAN, use default GPIOs\n"));
		wlan_reg_on = WLAN_REG_ON_GPIO;
		wlan_host_wake_up = WLAN_HOST_WAKE_GPIO;
	}

	// add to dump the DTS to confirm in case some platform special changes
	{
		struct property * pp = root_node->properties;

		while (NULL != pp) {
			char  str[128] = {0};
			memset(str, 0, sizeof(str));
			memcpy(str, pp->value, pp->length);
			printk("%s: name='%s', len=%d, val='%s'\n", __FUNCTION__, pp->name, pp->length, str);
			pp = pp->next;
		}
	}

#ifdef VENDOR_HW_UPDATE
	dhd_init_custom_dts(root_node);
#endif

	/* ========== WLAN_PWR_EN ============ */
	DHD_ERROR(("%s: gpio_wlan_power('%s'): %d\n", __FUNCTION__, WIFI_WL_REG_ON_PROPNAME, wlan_reg_on));

	/*
	 * For reg_on, gpio_request will fail if the gpio is configured to output-high
	 * in the dts using gpio-hog, so do not return error for failure.
	 */
	if (gpio_request_one(wlan_reg_on, GPIOF_OUT_INIT_HIGH, "WL_REG_ON")) {
		DHD_ERROR(("%s: Failed to request gpio %d for WL_REG_ON, "
			"might have configured in the dts\n",
			__FUNCTION__, wlan_reg_on));
#ifdef VENDOR_HW_UPDATE
		gpio_free(wlan_reg_on);
		if (gpio_request(wlan_reg_on,  "WL_REG_ON")) {
			printk(KERN_ERR "%s: Failed to request gpio %d for WL_REG_ON\n",
					__FUNCTION__, wlan_reg_on);
		}
#endif
	} else {
		DHD_ERROR(("%s: gpio_request WL_REG_ON done - WLAN_EN: GPIO %d\n",
			__FUNCTION__, wlan_reg_on));
	}

#ifdef VENDOR_HW_UPDATE
	dhd_wlan_power(1);
#endif

	gpio_reg_on_val = gpio_get_value(wlan_reg_on);
	DHD_ERROR(("%s: Initial WL_REG_ON: [%d]\n",
		__FUNCTION__, gpio_get_value(wlan_reg_on)));

	if (gpio_reg_on_val == 0) {
		DHD_ERROR(("%s: WL_REG_ON is LOW, drive it HIGH\n", __FUNCTION__));
		if (gpio_direction_output(wlan_reg_on, 1)) {
			DHD_ERROR(("%s: WL_REG_ON is failed to pull up\n", __FUNCTION__));
			return -EIO;
		}
	}

	/* Wait for WIFI_TURNON_DELAY due to power stability */
	msleep(WIFI_TURNON_DELAY);

	/* ========== WLAN_HOST_WAKE ============ */
	DHD_ERROR(("%s: gpio_wlan_host_wake('%s'): %d\n", __FUNCTION__, WIFI_WLAN_HOST_WAKE_PROPNAME, wlan_host_wake_up));

	if (gpio_request_one(wlan_host_wake_up, GPIOF_IN, "WLAN_HOST_WAKE")) {
		DHD_ERROR(("%s: Failed to request gpio %d for WLAN_HOST_WAKE\n",
			__FUNCTION__, wlan_host_wake_up));
#ifdef VENDOR_HW_UPDATE
		gpio_free(wlan_host_wake_up);
		if (gpio_request_one(wlan_host_wake_up, GPIOF_IN, "WLAN_HOST_WAKE")) {
			printk(KERN_ERR "%s: Failed to request gpio %d for WLAN_HOST_WAKE\n",
					__FUNCTION__, wlan_host_wake_up);
			return -ENODEV;
		}
#else
			return -ENODEV;
#endif
	} else {
		DHD_ERROR(("%s: gpio_request WLAN_HOST_WAKE done"
			" - WLAN_HOST_WAKE: GPIO %d\n",
			__FUNCTION__, wlan_host_wake_up));
	}

	if (gpio_direction_input(wlan_host_wake_up)) {
		DHD_ERROR(("%s: Failed to set WL_HOST_WAKE gpio direction\n", __FUNCTION__));
		return -EIO;
	}

	wlan_host_wake_irq = gpio_to_irq(wlan_host_wake_up);

	return 0;
}

// add for free GPIO resource
int dhd_wifi_deinit_gpio(void)
{
	if (wlan_reg_on) {
#ifdef VENDOR_HW_UPDATE
		dhd_wlan_power(0);
#endif
		gpio_free(wlan_reg_on);
	}
	if (wlan_host_wake_up) {
		gpio_free(wlan_host_wake_up);
	}

	return 0;
}

extern void kirin_pcie_power_on_atu_fixup(void) __attribute__ ((weak));
extern int kirin_pcie_lp_ctrl(u32 enable) __attribute__ ((weak));

int
dhd_wlan_power(int onoff)
{
	DHD_ERROR(("------------------------------------------------"));
	DHD_ERROR(("------------------------------------------------\n"));
	DHD_ERROR(("%s Enter: power %s\n", __func__, onoff ? "on" : "off"));

	if (onoff) {
		if (gpio_direction_output(wlan_reg_on, 1)) {
			DHD_ERROR(("%s: WL_REG_ON is failed to pull up\n", __FUNCTION__));
			return -EIO;
		}
		if (gpio_get_value(wlan_reg_on)) {
			DHD_ERROR(("WL_REG_ON on-step-2 : [%d]\n",
				gpio_get_value(wlan_reg_on)));
		} else {
			DHD_ERROR(("[%s] gpio value is 0. We need reinit.\n", __func__));
			if (gpio_direction_output(wlan_reg_on, 1)) {
				DHD_ERROR(("%s: WL_REG_ON is "
					"failed to pull up\n", __func__));
			}
		}

		/* Wait for WIFI_TURNON_DELAY due to power stability */
		msleep(WIFI_TURNON_DELAY);

		// modifiy for compatibility
		#ifdef BCMPCIE
		/*
		 * Call Kiric RC ATU fixup else si_attach will fail due to
		 * improper BAR0/1 address translations
		 */
		if (kirin_pcie_power_on_atu_fixup) {
			kirin_pcie_power_on_atu_fixup();
		} else {
			DHD_ERROR(("[%s] kirin_pcie_power_on_atu_fixup is NULL. "
				"REG_ON may not work\n", __func__));
		}
		/* Enable ASPM after powering ON */
		if (kirin_pcie_lp_ctrl) {
			kirin_pcie_lp_ctrl(onoff);
		} else {
			DHD_ERROR(("[%s] kirin_pcie_lp_ctrl is NULL. "
				"ASPM may not work\n", __func__));
		}
		#endif // BCMPCIE
	} else {
		// modifiy for compatibility
		#ifdef BCMPCIE
		/* Disable ASPM before powering off */
		if (kirin_pcie_lp_ctrl) {
			kirin_pcie_lp_ctrl(onoff);
		} else {
			DHD_ERROR(("[%s] kirin_pcie_lp_ctrl is NULL. "
				"ASPM may not work\n", __func__));
		}
		#endif // BCMPCIE
		if (gpio_direction_output(wlan_reg_on, 0)) {
			DHD_ERROR(("%s: WL_REG_ON is failed to pull up\n", __FUNCTION__));
			return -EIO;
		}
		if (gpio_get_value(wlan_reg_on)) {
			DHD_ERROR(("WL_REG_ON on-step-2 : [%d]\n",
				gpio_get_value(wlan_reg_on)));
		}
	}
	return 0;
}
EXPORT_SYMBOL(dhd_wlan_power);

static int
dhd_wlan_reset(int onoff)
{
	return 0;
}

// add for card detect
#if defined(BCMSDIO) && defined(BCMDHD_MODULAR) && defined(ENABLE_INSMOD_NO_FW_LOAD)
extern int wifi_card_detect(void);
#endif // BCMSDIO && BCMDHD_MODULAR && ENABLE_INSMOD_NO_FW_LOAD

static int
dhd_wlan_set_carddetect(int val)
{
// add for card detect
#if defined(BCMSDIO) && defined(BCMDHD_MODULAR) && defined(ENABLE_INSMOD_NO_FW_LOAD)
	int ret = 0;

	ret = wifi_card_detect();
	if (0 > ret) {
		DHD_ERROR(("%s-%d: * error hapen, ret=%d (ignore when remove)\n", __FUNCTION__, __LINE__, ret));
	}
#endif // BCMSDIO && BCMDHD_MODULAR && ENABLE_INSMOD_NO_FW_LOAD
	return 0;
}

#ifdef BCMSDIO
static int dhd_wlan_get_wake_irq(void)
{
	return gpio_to_irq(wlan_host_wake_up);
}
#endif /* BCMSDIO */

#if defined(CONFIG_BCMDHD_OOB_HOST_WAKE) && defined(CONFIG_BCMDHD_GET_OOB_STATE)
int
dhd_get_wlan_oob_gpio(void)
{
	return gpio_is_valid(wlan_host_wake_up) ?
		gpio_get_value(wlan_host_wake_up) : -1;
}
EXPORT_SYMBOL(dhd_get_wlan_oob_gpio);
#endif /* CONFIG_BCMDHD_OOB_HOST_WAKE && CONFIG_BCMDHD_GET_OOB_STATE */

struct resource dhd_wlan_resources = {
	.name	= "bcmdhd_wlan_irq",
	.start	= 0, /* Dummy */
	.end	= 0, /* Dummy */
	.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_SHAREABLE |
#ifdef BCMPCIE
	IORESOURCE_IRQ_HIGHEDGE,
#else
	IORESOURCE_IRQ_HIGHLEVEL,
#endif /* BCMPCIE */
};
EXPORT_SYMBOL(dhd_wlan_resources);

extern void dw_mci_sdio_card_detect_change(void);

int hi_sdio_detectcard_to_core(int val)
{
#ifdef CONFIG_MMC_DW_HI3XXX
	dw_mci_sdio_card_detect_change();
#endif

	return 0;
}

struct wifi_platform_data dhd_wlan_control = {
	.set_power	= dhd_wlan_power,
	.set_reset	= dhd_wlan_reset,
	.set_carddetect	= hi_sdio_detectcard_to_core,
	.get_mac_addr = dhd_wlan_get_mac_addr,
#ifdef CONFIG_BROADCOM_WIFI_RESERVED_MEM
	.mem_prealloc	= dhd_wlan_mem_prealloc,
#endif /* CONFIG_BROADCOM_WIFI_RESERVED_MEM */
#ifdef BCMSDIO
	.get_wake_irq   = dhd_wlan_get_wake_irq,
#endif
};
EXPORT_SYMBOL(dhd_wlan_control);

int
dhd_wlan_init(void)
{
	int ret;

 #ifdef CONFIG_HWCONNECTIVITY
 	/*For OneTrack, we need check it's the right chip type or not.
 	   If it's not the right chip type, don't init the driver */
 	if (!isMyConnectivityChip(CHIP_TYPE_SYNA)) {
 		pr_err("wifi chip type is not match, skip driver init");
 		return -EINVAL;
 	}
 	pr_info("wifi chip type is matched with Broadcom, continue");
 #endif

	DHD_ERROR(("%s: START.......\n", __FUNCTION__));
	ret = dhd_wifi_init_gpio();
	if (ret < 0) {
		DHD_ERROR(("%s: failed to initiate GPIO, ret=%d\n",
			__FUNCTION__, ret));
		goto fail;
	}

	dhd_wlan_resources.start = wlan_host_wake_irq;
	dhd_wlan_resources.end = wlan_host_wake_irq;

#ifdef CONFIG_BROADCOM_WIFI_RESERVED_MEM
	ret = dhd_init_wlan_mem();
	if (ret < 0) {
		DHD_ERROR(("%s: failed to alloc reserved memory,"
				" ret=%d\n", __FUNCTION__, ret));
	}
#endif /* CONFIG_BROADCOM_WIFI_RESERVED_MEM */

fail:
	DHD_ERROR(("%s: FINISH.......\n", __FUNCTION__));
	// add to free gpio resource
	if (0 > ret) {
		dhd_wifi_deinit_gpio();
	}
	return ret;
}

int
dhd_wlan_deinit(void)
{
	gpio_free(wlan_host_wake_up);
	gpio_free(wlan_reg_on);
	return 0;
}
#ifndef BCMDHD_MODULAR
/* Required only for Built-in DHD */
device_initcall(dhd_wlan_init);
#endif /* BOARD_HIKEY_MODULAR */
