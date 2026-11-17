/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Head file of DesignWare USB3 DRD Controller
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2  of
 * the License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _DWC3_CHIP_H_
#define _DWC3_CHIP_H_

#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/dcache.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/kfifo.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/regulator/consumer.h>

/*
 * chip dwc3 otg bc registers
 */
#define USBOTG3_CTRL0		0x00  /* Select control register 0 */
#define USBOTG3_CTRL1		0x04  /* Select control register 1 */
#define USBOTG3_CTRL2		0x08  /* Select control register 2 */
#define USBOTG3_CTRL3		0x0C  /* Select control register 3 */
#define USBOTG3_CTRL4		0x10  /* Select control register 4 */
#define USBOTG3_CTRL5		0x14  /* Select control register 5 */
#define USBOTG3_CTRL6		0x18  /* Select control register 6 */
#define USBOTG3_CTRL7		0x1C  /* Select control register 7 */
#define USBOTG3_STS0		0x20  /* Status register 0 */
#define USBOTG3_STS1		0x24  /* Status register 1 */
#define USBOTG3_STS2		0x28  /* Status register 2 */
#define USBOTG3_STS3		0x2C  /* Status register 3 */
#define RAM_CTRL		0x40  /* RAM control register  */
#define USBOTG3_STS4		0x44  /* Status register 4 */
#define USB3PHY_CTRL		0x48  /* PHY control register */
#define USB3PHY_STS		0x4C  /* PHY Status register */
#define USB3PHY_CR_STS		0x50  /* PHY Internal register status */
#define USB3PHY_CR_CTRL		0x54  /* PHY Internal register status */
#define USB3_RES		0x58  /* USB Reserved register */

/* USTOTG3_CTRL0 */
# define USBOTG3CTRL0_SESSVLD_SEL              (1 << 14)
# define USBOTG3CTRL0_SC_SESSVLD               (1 << 13)
# define USBOTG3CTRL0_POWERPRESENT_SEL         (1 << 12)
# define USBOTG3CTRL0_SC_POWERPRESENT          (1 << 11)
# define USBOTG3CTRL0_BVALID_SEL               (1 << 10)
# define USBOTG3CTRL0_SC_BVALID                (1 << 9)
# define USBOTG3CTRL0_AVALID_SEL               (1 << 8)
# define USBOTG3CTRL0_SC_AVALID                (1 << 7)
# define USBOTG3CTRL0_VBUSVALID_SEL            (1 << 6)
# define USBOTG3CTRL0_DRVVBUS                  (1 << 5)
# define USBOTG3CTRL0_DRVVBUS_SEL              (1 << 4)
# define USBOTG3CTRL0_IDDIG                    (1 << 3)
# define USBOTG3CTRL0_IDDIG_SEL                (1 << 2)
# define USBOTG3CTRL0_IDPULLUP                 (1 << 1)
# define USBOTG3CTRL0_IDPULLUP_SEL             (1 << 0)

/* USTOTG3_CTRL2 */
# define USBOTG3CTRL2_POWERDOWN_HSP             (1 << 0)
# define USBOTG3CTRL2_POWERDOWN_SSP             (1 << 1)

/* USBOTG3_CTRL3 */
# define USBOTG3_CTRL3_VBUSVLDEXT	(1 << 6)
# define USBOTG3_CTRL3_VBUSVLDEXTSEL	(1 << 5)
# define USBOTG3_CTRL3_TXBITSTUFFEHN	(1 << 4)
# define USBOTG3_CTRL3_TXBITSTUFFEN	(1 << 3)
# define USBOTG3_CTRL3_RETENABLEN	(1 << 2)
# define USBOTG3_CTRL3_OTGDISABLE	(1 << 1)
# define USBOTG3_CTRL3_COMMONONN	(1 << 0)

/* USBOTG3_CTRL4 */
# define USBOTG3_CTRL4_TXVREFTUNE(x)            (((x) << 22) & (0xf << 22))
# define USBOTG3_CTRL4_TXRISETUNE(x)            (((x) << 20) & (3 << 20))
# define USBOTG3_CTRL4_TXRESTUNE(x)             (((x) << 18) & (3 << 18))
# define USBOTG3_CTRL4_TXPREEMPPULSETUNE        (1 << 17)
# define USBOTG3_CTRL4_TXPREEMPAMPTUNE(x)       (((x) << 15) & (3 << 15))
# define USBOTG3_CTRL4_TXHSXVTUNE(x)            (((x) << 13) & (3 << 13))
# define USBOTG3_CTRL4_TXFSLSTUNE(x)            (((x) << 9) & (0xf << 9))
# define USBOTG3_CTRL4_SQRXTUNE(x)              (((x) << 6) & (7 << 6))
# define USBOTG3_CTRL4_OTGTUNE_MASK             (7 << 3)
# define USBOTG3_CTRL4_OTGTUNE(x)               (((x) << 3) & USBOTG3_CTRL4_OTGTUNE_MASK)
# define USBOTG3_CTRL4_COMPDISTUNE_MASK         7
# define USBOTG3_CTRL4_COMPDISTUNE(x)           ((x) & USBOTG3_CTRL4_COMPDISTUNE_MASK)

# define USBOTG3_CTRL7_REF_SSP_EN				(1 << 16)

/* USBOTG3_CTRL6 */
#define TX_VBOOST_LVL_MASK			7
#define TX_VBOOST_LVL(x)			((x) & TX_VBOOST_LVL_MASK)

/* USBPHY vboost lvl default value */
#define VBOOST_LVL_DEFAULT_PARAM	 5

#define usb_dbg(format, arg...)    \
		printk(KERN_DEBUG "[USB3][%s]"format, __func__, ##arg)

#define usb_info(format, arg...)    \
		printk(KERN_INFO "[USB3][%s]"format, __func__, ##arg)

#define usb_err(format, arg...)    \
		printk(KERN_ERR "[USB3][%s]"format, __func__, ##arg)

enum usb_power_state {
	USB_POWER_OFF = 0,
	USB_POWER_ON = 1,
	USB_POWER_HOLD = 2,
};

enum usb_connect_state {
	USB_CONNECT_DCP = 0xEE, /* connect a charger */
	USB_CONNECT_HOST = 0xFF, /* usb state change form hifi to host */
};

#define MAX_CHIP_USB_EVENT_COUNT 16
struct usb_controller;

struct chip_dwc3_device {
	struct platform_device *pdev;

	enum usb_state state;
	enum hisi_charger_type charger_type;
	enum hisi_charger_type fake_charger_type;

	DECLARE_KFIFO(event_fifo, struct chip_usb_event,
		      MAX_CHIP_USB_EVENT_COUNT);
	enum otg_dev_event_type last_event;
	spinlock_t event_lock;

	/* save time stamp, for time consuming checking */
	unsigned long start_device_time_stamp;
	unsigned long start_host_time_stamp;
	unsigned long stop_host_time_stamp;
	unsigned long start_hifiusb_time_stamp;
	unsigned long stop_hifiusb_time_stamp;
	unsigned long start_device_complete_time_stamp;
	unsigned long start_host_complete_time_stamp;
	unsigned long stop_host_complete_time_stamp;
	unsigned long start_hifiusb_complete_time_stamp;
	unsigned long stop_hifiusb_complete_time_stamp;

	unsigned long device_add_time_stamp;

	struct mutex lock;
	struct wakeup_source wake_lock;
	struct work_struct event_work;
	struct work_struct speed_change_work;

	u32 eye_diagram_param; /* this param will be set to USBOTG3_CTRL4 */
	u32 eye_diagram_host_param;
	u32 usb3_phy_tx_vboost_lvl;

	u32 fpga_flag;
	int fpga_usb_mode_gpio;
	int fpga_otg_drv_vbus_gpio;
	int fpga_phy_reset_gpio;
	int fpga_phy_switch_gpio;

	int eventmask;
	u32 dma_mask_bit;

	/* for bc again */
	u32 bc_again_flag;
	u32 bc_unknown_again_flag;
	unsigned int bc_again_delay_time;
	spinlock_t bc_again_lock;
	struct delayed_work bc_again_work;
	struct notifier_block event_nb;
	struct notifier_block xhci_nb;
	unsigned int vdp_src_enable;

	u32 support_dp;
	struct usb3_core_ops *core_ops;
	const struct chip_usb_phy *usb_phy;

	bool use_new_frame;
	struct phy *usb2_phy;
	struct phy *usb3_phy;

	struct usb_controller **usbc_sets;
	u32 usbc_nums;
	u32 usbc_idx;
	u32 usbc_working_idx;

	bool notify_speed;
	unsigned int speed;
	unsigned int check_voltage;
	unsigned int suspend_error_flag;
	unsigned int set_hi_impedance;
	unsigned int hifi_ip_first;

	unsigned quirk_keep_u2_power_suspend : 1;
	enum typec_plug_orien_e plug_orien;
	enum tcpc_mux_ctrl_type mode_type;

	struct extcon_dev *edev;
	u32 vdp_src_disable;
	bool usb_support_s3_wakeup;

	struct dentry *debug_root;

	struct notifier_block device_event_nb;
};

struct chip_usb_phy {
	void __iomem *otg_bc_reg_base;

	int (*init)(unsigned int support_dp, unsigned int eye_diagram_param,
		unsigned int usb3_phy_tx_vboost_lvl);
	int (*shutdown)(unsigned int support_dp);
	int (*shared_phy_init)(unsigned int support_dp, unsigned int eye_diagram_param,
			unsigned int combophy_flag);
	int (*shared_phy_shutdown)(unsigned int support_dp,
			unsigned int combophy_flag, unsigned int keep_power);
	void (*set_hi_impedance)(void);
	void (*notify_speed)(unsigned int speed);
	int (*tcpc_is_usb_only)(void);
	void (*disable_usb3)(void);
	int (*get_term)(u32 *term);
	int (*set_term)(u32 term);
	void __iomem *(*get_bc_ctrl_reg)(void);
};

struct usb_controller {
	struct clk **clks;
	int num_clks;
	bool stub_clk;
	struct device_node *node;
	struct platform_device *pdev;
	struct chip_usb_reg_cfg *reset;
	struct chip_usb_reg_cfg *unreset;
	struct chip_usb_reg_cfg *sel_self;
};

void chip_usb_unreset_phy_if_fpga(void);
void chip_usb_switch_sharedphy_if_fpga(int to_hifi);
int chip_dwc3_is_powerdown(void);
enum usb_device_speed chip_dwc3_get_dt_host_maxspeed(void);

/*
 * chip usb
 */
extern struct chip_dwc3_device *chip_dwc3_dev;
int chip_dwc3_is_fpga(void);
void chip_dwc3_wake_lock(struct chip_dwc3_device *chip_dwc3);
void chip_dwc3_wake_unlock(struct chip_dwc3_device *chip_dwc3);
void dwc3_core_disable_pipe_clock(void);
int dwc3_core_enable_u3(void);
int chip_usb_dwc3_register_phy(const struct chip_usb_phy *phy);
int chip_usb_dwc3_unregister_phy(const struct chip_usb_phy *phy);
void chip_usb_switch_controller(struct chip_dwc3_device *chip_dwc);

#endif /* _DWC3_CHIP_H_ */
