/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Phy driver for platform mar.
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/hisi/usb/dwc3_usb_interface.h>
#include <linux/hisi/usb/hisi_usb_helper.h>
#include <linux/hisi/usb/hisi_usb_interface.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

#include <huawei_platform/usb/hw_pd_dev.h>
#include <pmic_interface.h>

#ifdef CONFIG_HUAWEI_CHARGER_AP
#include <huawei_platform/power/huawei_charger.h>
#endif

#ifdef CONFIG_CONTEXTHUB_PD
#include <linux/hisi/usb/tca.h>
#endif

#include "combophy.h"
#include "combophy_regcfg.h"
#include "dwc3-hisi.h"
#include "hisi_usb_debug.h"
#include "hisi_usb3_misctrl.h"
#include "hisi_usb3_31phy.h"
#include <chipset_common/hwusb/hw_usb.h>

struct plat_chip_usb_phy {
	struct chip_usb_phy phy;

	void __iomem *pericfg_reg_base;
	void __iomem *sctrl_reg_base;
	void __iomem *pctrl_reg_base;

	struct clk *clk;
	struct clk *gt_aclk_usb3otg;
	struct clk *gt_hclk_usb3otg;
	struct clk *gt_clk_usb3_tcxo_en;
	struct clk *gt_clk_usb2phy_ref;

	struct regulator *usb20phy_power;
	unsigned int usb20phy_power_flag;
	u32 usb3_phy_term;
};

static struct plat_chip_usb_phy *_usb_phy;

#define SCTRL_SCDEEPSLEEPED		0x08
#define USB_INIT_CLK_SEL                (1 << 20)
#define USB_REFCLK_ISO_EN               (1 << 25)
#define SC_CLK_USB3PHY_3MUX1_SEL        (1 << 25)

#define SC_SEL_ABB_BACKUP               (1 << 8)
#define CLKDIV_MASK_START                16

#define PERI_CRG_CLKDIV21               0xFC
#define CLKDIV21_MSK_START               16
#define SC_GT_CLK_ABB_SYS		(1 << 6)
#define SEL_ABB_BACKUP			(1 << 8)

#define GT_CLK_ABB_BACKUP               (1 << 22)
#define PERI_CRG_CLK_DIS5               0x54

#define PMC_PPLL3CTRL0                  0x048
#define PPLL3_FBDIV_START                8
#define PPLL3_EN                        (1 << 0)
#define PPLL3_BP                        (1 << 1)
#define PPLL3_LOCK                      (1 << 26)

#define PMC_PPLL3CTRL1                  0x04C
#define PPLL3_INT_MOD                   (1 << 24)
#define GT_CLK_PPLL3                    (1 << 26)

#define PERI_CRG_CLK_EN5                0x50
#define PERI_CRG_PERRSTSTAT4		0x98

#define SC_USB3PHY_ABB_GT_EN            (1 << 15)
#define REF_SSP_EN                      (1 << 16)

#define CLK_FREQ_19_2M              19200000

#define SC_USB20PHY_MISC_CTRL1  0x604
#define SC_USB20PHY_MISC_CTRL4  0x610

/* USB MISC CTRL */
/* bit of USBOTG3_CTRL1 */
#define HOST_U3_PORT_DISABLE_B		1

#define USB_MISC_CFGA0			0xa0
/* bit of USB_MISC_CFGA0 */
#define CFGA0_USB31C_RESETN_B		8

#define PCTRL_PERI_CTRL24		 0x64
#define PERI_CRG_ISODIS			 0x148

#define USB_MISC_REG_LOGIC_ANALYZER_TRACE_L	0x28
#define USB_MISC_REG_LOGIC_ANALYZER_TRACE_H	0x2c

/* usb3 phy term default value */
#define TERM_INVALID_PARAM	0xdeaddead

#define USB_MISC_CFG84 	0x84
#define PME_EN 	9

/* clk freq usb default usb3.0 237M usb2.0 60M */
static uint32_t g_usb3otg_aclk_freq = 237000000;
static uint32_t g_usb2otg_aclk_freq = 61000000;

static int usb3_phy_33v_enable(struct plat_chip_usb_phy *usb_phy);
static int usb3_phy_33v_disable(struct plat_chip_usb_phy *usb_phy);

int usb3_open_misc_ctrl_clk(void)
{
	int ret;

	if (!_usb_phy) {
		usb_err("usb driver not setup!\n");
		return -EINVAL;
	}

	/* open hclk gate */
	ret = clk_prepare_enable(_usb_phy->gt_hclk_usb3otg);
	if (ret) {
		usb_err("clk_enable gt_hclk_usb3otg failed\n");
		return ret;
	}

	if (__clk_is_enabled(_usb_phy->gt_hclk_usb3otg) == false) {
		usb_err("gt_hclk_usb3otg  enable err\n");
		return -EFAULT;
	}

	return 0;
}

void usb3_close_misc_ctrl_clk(void)
{
	if (!_usb_phy) {
		usb_err("usb driver not setup!\n");
		return;
	}

	/* disable usb3otg hclk */
	clk_disable_unprepare(_usb_phy->gt_hclk_usb3otg);
}

static int dwc3_combophy_sw_sysc(enum tcpc_mux_ctrl_type new_mode)
{
	int ret;

	ret = combophy_sw_sysc(TCPC_USB31_CONNECTED,
			(enum typec_plug_orien_e)chip_usb_otg_get_typec_orien(),
			!get_hifi_usb_retry_count());
	if (ret)
		usb_err("combophy_sw_sysc failed %d\n", ret);
	return ret;
}

static int config_usb_phy_controller(unsigned int support_dp)
{
	volatile uint32_t temp;
	static int flag = 1;

	/* Release USB20 PHY out of IDDQ mode */
	usb3_sc_misc_reg_clrbit(0u, 0x608ul);

	if (!support_dp || flag) {
		usb_dbg("set tca\n");
		flag = 0;
		/* Release USB31 PHY out of  TestPowerDown mode */
		combophy_regcfg_exit_testpowerdown();

		/* Tell the PHY power is stable */
		combophy_regcfg_power_stable();

		/*
		 * config the TCA mux mode
		 * register:
		 *      -- 0x204 0x208: write immediately
		 *      -- 0x200 0x210 0x214 0x240: read-update-write
		 */
		usb3_misc_reg_writel(0xffffu, 0x208ul);
		usb3_misc_reg_writel(0x3u, 0x204ul);

		usb3_misc_reg_clrvalue(~SET_NBITS_MASK(0, 1), 0x200ul);
		usb3_misc_reg_setbit(4u, 0x210ul);

		temp = usb3_misc_reg_readl(0x214);
		/* start bit 3, end bit 4 */
		temp &= ~(SET_NBITS_MASK(0, 1) | SET_NBITS_MASK(3, 4));
		temp |= (0x1 | (0x2 << 3)); /* bit 3 */
		usb3_misc_reg_writel(temp, 0x214);

		usb3_misc_reg_setvalue(0x3u | (0x3u << 2), 0x240ul); /* bit 2 */

		usb3_misc_reg_setbit(7u, 0xb4ul);
	}

	/* Enable SSC */
	usb3_misc_reg_setbit(1, 0x5c);

	return 0;
}

static uint32_t is_abb_clk_selected(const void __iomem *sctrl_base)
{
	volatile uint32_t scdeepsleeped;

	if (!sctrl_base) {
		usb_err("sctrl_base is NULL!\n");
		return 1;
	}

	scdeepsleeped = (uint32_t)readl(SCTRL_SCDEEPSLEEPED + sctrl_base);
	if ((scdeepsleeped & (USB_INIT_CLK_SEL)) == 0)
		return 1;

	return 0;
}

static int set_combophy_clk(struct plat_chip_usb_phy *usb_phy)
{
	void __iomem *pericfg_base = usb_phy->pericfg_reg_base;
	void __iomem *pctrl_base = usb_phy->pctrl_reg_base;
	void __iomem *sctrl_base = usb_phy->sctrl_reg_base;
#define USB_CLK_TYPE_ABB  0xabb
#define USB_CLK_TYPE_PAD  0x10ad
	volatile uint32_t temp;
	int ret = 0;
	int clk_type = USB_CLK_TYPE_ABB;

	if (is_abb_clk_selected(sctrl_base)) {
		/* usb refclk iso enable */
		writel(USB_REFCLK_ISO_EN, PERI_CRG_ISODIS + pericfg_base);

		/* enable usb_tcxo_en */
		ret = clk_prepare_enable(usb_phy->gt_clk_usb3_tcxo_en);
		if (ret) {
			usb_err("clk_prepare_enable gt_clk_usb3_tcxo_en failed\n");
			return ret;
		}

		mdelay(10); /* delay 10ms */

		/* select usbphy clk from abb */
		temp = (uint32_t)readl(pctrl_base + PCTRL_PERI_CTRL24);
		temp &= ~SC_CLK_USB3PHY_3MUX1_SEL;
		writel(temp, pctrl_base + PCTRL_PERI_CTRL24);
	} else {
		usb_dbg("pad clk\n");

		writel(SC_GT_CLK_ABB_SYS | (SC_GT_CLK_ABB_SYS << CLKDIV21_MSK_START)
			| (SEL_ABB_BACKUP << CLKDIV21_MSK_START),
			PERI_CRG_CLKDIV21 + pericfg_base);

		writel(GT_CLK_ABB_BACKUP, PERI_CRG_CLK_EN5 + pericfg_base);

		temp = (uint32_t)readl(pctrl_base + PCTRL_PERI_CTRL24);
		temp |= SC_CLK_USB3PHY_3MUX1_SEL;
		writel(temp, pctrl_base + PCTRL_PERI_CTRL24);

		clk_type = USB_CLK_TYPE_PAD;
	}

	usb_dbg("usb clk type:%x\n", clk_type);
	return ret;
}

int usb3_clk_is_open(void)
{
	return combophy_regcfg_is_misc_ctrl_clk_en() &&
		combophy_regcfg_is_controller_ref_clk_en() &&
		combophy_regcfg_is_controller_bus_clk_en();
}

int dwc3_set_combophy_clk(void)
{
	int ret;

	if (!_usb_phy)
		return -ENODEV;

	ret = set_combophy_clk(_usb_phy);
	if (ret)
		usb_err("[CLK.ERR] combophy clk set error!\n");
	return ret;
}

static int dwc3_phy_release(void)
{
	int ret;

	if (combophy_regcfg_is_misc_ctrl_unreset() && usb3_clk_is_open()) {
		usb_err("combophy_regcfg_is_misc_ctrl_unreset && usb3_clk_is_open\n");
	} else {
		usb_dbg("release combophy\n");
		ret = dwc3_set_combophy_clk();
		if (ret)
			return ret;

		/* unreset phy */
		usb3_misc_reg_setbit(1, 0xa0);

		chip_usb_unreset_phy_if_fpga();
		udelay(100); /* delay 100ms */
	}

	return 0;
}

static void close_combophy_clk(struct plat_chip_usb_phy *usb_phy)
{
	void __iomem *sctrl_base = usb_phy->sctrl_reg_base;
	void __iomem *pericfg_base = usb_phy->pericfg_reg_base;

	if (is_abb_clk_selected(sctrl_base))
		/* disable usb_tcxo_en */
		clk_disable_unprepare(usb_phy->gt_clk_usb3_tcxo_en);
	else
		writel(GT_CLK_ABB_BACKUP, PERI_CRG_CLK_DIS5 + pericfg_base);
}

void dwc3_close_combophy_clk(void)
{
	if (!_usb_phy)
		return;

	close_combophy_clk(_usb_phy);
}

static void dwc3_phy_reset(struct plat_chip_usb_phy *usb_phy)
{
	usb_dbg("reset combophy\n");
	if (combophy_regcfg_is_misc_ctrl_unreset() && usb3_clk_is_open()) {
		/* reset phy */
		usb3_misc_reg_clrbit(1, 0xa0ul); /* clrbit num */
		usb3_sc_misc_reg_clrbit(0, 0x618ul); /* clrbit num */
		usb_dbg("reset phy\n");

		close_combophy_clk(usb_phy);

		/* close phy ref clk */
		clk_disable_unprepare(usb_phy->gt_clk_usb2phy_ref);
	}
}

int dwc3_open_controller_clk(const struct plat_chip_usb_phy *usb_phy)
{
	int ret;

	ret = clk_prepare_enable(usb_phy->gt_aclk_usb3otg);
	if (ret) {
		usb_err("clk_prepare_enable gt_aclk_usb3otg failed\n");
		return -EACCES;
	}

	ret = clk_prepare_enable(usb_phy->clk);
	if (ret) {
		clk_disable_unprepare(usb_phy->gt_aclk_usb3otg);
		usb_err("clk_prepare_enable clk failed\n");
		return -EACCES;
	}

	/*
	 * usb2phy select:
	 * 0:usb31 controller owns the phy
	 * 1:usb3 controller in asp owns the phy
	 */
	usb3_sc_misc_reg_clrbit(5, 0x618); /* bit 5 */

	/* usb2_refclksel: pll ref clock select */
	usb3_sc_misc_reg_setbit(4, 0x60c); /* bit 4 */

	return ret;
}

static void dwc3_close_controller_clk(const struct plat_chip_usb_phy *usb_phy)
{
	clk_disable_unprepare(usb_phy->gt_aclk_usb3otg);
	clk_disable_unprepare(usb_phy->clk);
}

/* dwc3_set_highspeed_only_step1 called before controller unreset */
static int dwc3_set_highspeed_only_step1(void)
{
	/* disable usb3 SS port */
	usb3_misc_reg_setbit(1, 0x4); /* bit 1 */

	/* misc reg start bit 4, end bit 7 */
	usb3_misc_reg_clrvalue((u32)(~GENMASK(7, 4)), 0x7c);

	udelay(100); /* delay 100ms */

	return 0;
}

/* dwc3_set_highspeed_only_step2 called after controller unreset */
static void dwc3_set_highspeed_only_step2(void)
{
	/* disble pipe clock use for DP4 and must use usb2 */
	dwc3_core_disable_pipe_clock();
}

static int dwc3_release(struct plat_chip_usb_phy *usb_phy, unsigned int support_dp)
{
	int ret;
	int highspeed_only;

	usb_dbg("+\n");

	ret = dwc3_open_controller_clk(usb_phy);
	if (ret) {
		usb_err("[CLK.ERR] open clk error!\n");
		return ret;
	}

	/* SCCLKDIV5(0x264) bit[14]=0, bit[30]=1 */
	ret = clk_set_rate(usb_phy->gt_clk_usb2phy_ref, CLK_FREQ_19_2M);
	if (ret) {
		dwc3_close_controller_clk(usb_phy);
		usb_err("usb2phy_ref set rate failed, ret=%d\n", ret);
		return ret;
	}

	/* SCPEREN4(0x1b0)  bit[18]=1 */
	ret = clk_prepare_enable(usb_phy->gt_clk_usb2phy_ref);
	if (ret) {
		dwc3_close_controller_clk(usb_phy);
		usb_err("clk_prepare_enable gt_clk_usb2phy_ref failed\n");
		return ret;
	}

	udelay(100); /* delay 100ms */

	config_usb_phy_controller(support_dp);

	if (chip_dwc3_is_fpga()) {
		/* FPGA force utmi clk to 60M, start bit 8, end bit 9 */
		usb3_sc_misc_reg_clrvalue(~SET_NBITS_MASK(8, 9), 0x600);
	} else {
		/* force utmi clk to 30M, start bit 8, end bit 9 */
		usb3_sc_misc_reg_setvalue(SET_NBITS_MASK(8, 9), 0x600);
	}

	udelay(100); /* delay 100ms */

	/* release usb2.0 phy */
	usb3_sc_misc_reg_setbit(0, 0x618);

	highspeed_only = combophy_is_highspeed_only();
	if (highspeed_only) {
		usb_dbg("[USB.DP] DP4 mode, set usb2.0 only, setp 1\n");
		ret = dwc3_set_highspeed_only_step1();
		if (ret) {
			dwc3_close_controller_clk(usb_phy);
			clk_disable_unprepare(usb_phy->gt_clk_usb2phy_ref);
			return ret;
		}
	}

	if (chip_dwc3_get_dt_host_maxspeed() < USB_SPEED_SUPER_PLUS) {
		usb_dbg("[USB.LINK] usb host super-speed only!\n");
		usb3_misc_reg_setbit(30, 0x7c); /* misc reg set bit 30 */
	}
	/* unreset controller */
	usb3_misc_reg_setbit(8, 0xa0); /* bit 8 */
	udelay(100); /* delay 100ms */

	if (highspeed_only) {
		usb_dbg("[USB.DP] DP4 mode, set usb2.0 only, step 2\n");
		dwc3_set_highspeed_only_step2();
	}

	/* set vbus valid, reg start bit 6, end bit 7 */
	usb3_misc_reg_setvalue(SET_NBITS_MASK(6, 7), 0x0);
	/* reg start bit 5, end bit 6 */
	usb3_sc_misc_reg_setvalue(SET_NBITS_MASK(5, 6), 0x600);

	/* require to delay 10ms */
	mdelay(10);
	usb_dbg("-\n");

	return ret;
}

static void mar_notify_speed(unsigned int speed)
{
	int ret;

	usb_dbg("+device speed is %d\n", speed);

	if (!_usb_phy)
		return;

#ifdef CONFIG_TCPC_CLASS
	if ((speed != USB_CONNECT_HOST) && (speed != USB_CONNECT_DCP))
		hw_usb_set_usb_speed(speed);
#endif

	if (((speed < USB_SPEED_WIRELESS) && (speed > USB_SPEED_UNKNOWN))
		|| (speed == USB_CONNECT_DCP)) {
		usb_dbg("set USB2OTG_ACLK_FREQ\n");
		ret = clk_set_rate(_usb_phy->gt_aclk_usb3otg, g_usb2otg_aclk_freq);
		if (ret)
			usb_err("Can't aclk rate set\n");
	} else if ((speed == USB_CONNECT_HOST) && (!chip_dwc3_is_powerdown())) {
		usb_dbg("set USB3OTG_ACLK_FREQ\n");
		ret = clk_set_rate(_usb_phy->gt_aclk_usb3otg, g_usb3otg_aclk_freq);
		if (ret)
			usb_err("Can't aclk rate set\n");
	}

	usb_dbg("-\n");
}

static void dwc3_reset(const struct plat_chip_usb_phy *usb_phy)
{
	usb_dbg("+\n");
	if (combophy_regcfg_is_misc_ctrl_unreset() && usb3_clk_is_open()) {
		/* set vbus valid */
		/* reset controller */
		usb3_misc_reg_clrbit(8, 0xa0ul); /* bit 8 */
		usb_err("reset controller\n");

		/* reset usb2.0 phy */
		usb3_sc_misc_reg_clrbit(0, 0x618);
	}

	clk_disable_unprepare(usb_phy->clk);
	clk_disable_unprepare(usb_phy->gt_aclk_usb3otg);

	usb_dbg("-\n");
}

static int usb3_clk_set_rate(const struct plat_chip_usb_phy *usb_phy)
{
	int ret;

	/* set usb aclk 228MHz to improve performance */
	usb_dbg("set aclk rate [%d]\n", g_usb3otg_aclk_freq);
	ret = clk_set_rate(usb_phy->gt_aclk_usb3otg, g_usb3otg_aclk_freq);
	if (ret)
		usb_err("Can't aclk rate set, current rate is %ld:\n",
				clk_get_rate(usb_phy->gt_aclk_usb3otg));

	return ret;
}

static void set_usb2_eye_diagram_param(struct plat_chip_usb_phy *usb_phy,
		unsigned int eye_diagram_param)
{
	void __iomem *sctrl = usb_phy->sctrl_reg_base;

	if (chip_dwc3_is_fpga())
		return;

	/* set high speed phy parameter */
	writel(eye_diagram_param, sctrl + SC_USB20PHY_MISC_CTRL1);
	usb_dbg("set hs phy param 0x%x for host\n",
			readl(sctrl + SC_USB20PHY_MISC_CTRL1));
}

#define TX_VBOOST_LVL_REG  0x21
#define TX_VBOOST_LVL_VALUE_START  4
#define TX_VBOOST_LVL_VALUE_END  6
#define TX_VBOOST_LVL_ENABLE SET_BIT_MASK(7)

static void set_vboost_for_usb3(uint32_t usb3_phy_tx_vboost_lvl)
{
	volatile u16 temp;

	/* vboost lvl limit 7 */
	if ((usb3_phy_tx_vboost_lvl > 7) || (usb3_phy_tx_vboost_lvl == VBOOST_LVL_DEFAULT_PARAM)) {
		usb_dbg("bad vboost  %d  use default  %d\n", usb3_phy_tx_vboost_lvl, VBOOST_LVL_DEFAULT_PARAM);
		return;
	}

	temp = usb31phy_cr_read(TX_VBOOST_LVL_REG);
	temp = usb31phy_cr_read(TX_VBOOST_LVL_REG);
	temp &= ~SET_NBITS_MASK(TX_VBOOST_LVL_VALUE_START, TX_VBOOST_LVL_VALUE_END);
	temp |= (u16)((TX_VBOOST_LVL_ENABLE | (usb3_phy_tx_vboost_lvl
		<< TX_VBOOST_LVL_VALUE_START)) & SET_NBITS_MASK(0, 15)); /* bit 15 */
	usb31phy_cr_write(TX_VBOOST_LVL_REG, temp);

	temp = usb31phy_cr_read(TX_VBOOST_LVL_REG);
	usb_dbg("[0x%x]usb31 cr param:%x\n", TX_VBOOST_LVL_REG, temp);
}

#define USB3_PHY_TERM_LANE0_REG	0x301a
#define USB3_PHY_TERM_LANE1_REG	0x311a
#define USB3_PHY_TERM_LANE2_REG	0x321a
#define USB3_PHY_TERM_LANE3_REG	0x331a

static void set_phy_term_for_usb3(u32 usb3_phy_term)
{
	usb31phy_cr_write(USB3_PHY_TERM_LANE0_REG, usb3_phy_term);
	usb31phy_cr_write(USB3_PHY_TERM_LANE1_REG, usb3_phy_term);
	usb31phy_cr_write(USB3_PHY_TERM_LANE2_REG, usb3_phy_term);
	usb31phy_cr_write(USB3_PHY_TERM_LANE3_REG, usb3_phy_term);
	usb_dbg("[0x%x]usb31 cr param:%x\n", USB3_PHY_TERM_LANE0_REG,
			usb31phy_cr_read(USB3_PHY_TERM_LANE0_REG));
	usb_dbg("[0x%x]usb31 cr param:%x\n", USB3_PHY_TERM_LANE1_REG,
			usb31phy_cr_read(USB3_PHY_TERM_LANE1_REG));
	usb_dbg("[0x%x]usb31 cr param:%x\n", USB3_PHY_TERM_LANE2_REG,
			usb31phy_cr_read(USB3_PHY_TERM_LANE2_REG));
	usb_dbg("[0x%x]usb31 cr param:%x\n", USB3_PHY_TERM_LANE3_REG,
			usb31phy_cr_read(USB3_PHY_TERM_LANE3_REG));
}

static int mar_set_phy_term(u32 usb3_phy_term)
{
	usb_dbg("+\n");
	if (!_usb_phy)
		return -ENODEV;

	_usb_phy->usb3_phy_term = usb3_phy_term;

	return 0;
}

static int mar_get_phy_term(u32 *usb3_phy_term)
{
	usb_dbg("+\n");
	if (!_usb_phy)
		return -ENODEV;

	*usb3_phy_term = _usb_phy->usb3_phy_term;

	return 0;
}

static int mar_usb31_core_enable_u3(struct plat_chip_usb_phy *usb_phy)
{
	volatile u32 temp;
	int ret;
	int power_down_flag;
	void __iomem *pericfg_base = usb_phy->pericfg_reg_base;

	usb_dbg("+\n");

	/*
	 * check if misc ctrl is release
	 * check if usb clk is open
	 */
	if (!combophy_regcfg_is_misc_ctrl_unreset() ||
			!combophy_regcfg_is_misc_ctrl_clk_en()) {
		usb_err("misc ctrl or usb3 clk not ready.\n");
		return -ENOENT;
	}

	power_down_flag = get_chip_dwc3_power_flag();
	if (chip_dwc3_is_powerdown()) {
		/* open usb phy clk gate */
		ret = dwc3_open_controller_clk(usb_phy);
		if (ret) {
			usb_err("[CLK.ERR] open clk fail\n");
			return ret;
		}

		temp = readl(pericfg_base + 0x48);
		if (!(temp & 0x3)) {
			usb_err("[PERI CFG 0X48:0x%x]\n", temp);
			usb_err("[misc 0xa0:0x%x]\n", usb3_misc_reg_readl(0xa0));
			return -ENOENT;
		}

		/* Release USB20 PHY out of IDDQ mode */
		usb3_sc_misc_reg_clrbit(0u, 0x608ul);

		/* release usb2.0 phy */
		usb3_sc_misc_reg_setbit(0, 0x618);

		udelay(100); /* delay 100ms */

		/* unreset controller */
		usb3_misc_reg_setbit(8, 0xa0); /* misc reg bit 8 */
		udelay(100); /* delay 100ms */
		set_chip_dwc3_power_flag(USB_POWER_ON);
	}

	ret = dwc3_core_enable_u3();
	if (get_chip_dwc3_power_flag() != power_down_flag)
		set_chip_dwc3_power_flag(power_down_flag);
	usb_dbg("-\n");
	return ret;
}

static int mar_usb31_core_disable_u3(struct plat_chip_usb_phy *usb_phy)
{
	usb_dbg("+\n");

	if (!combophy_regcfg_is_misc_ctrl_unreset() ||
			!combophy_regcfg_is_misc_ctrl_clk_en()) {
		usb_err("usb core is busy! please disconnect usb first!\n");
		return -EBUSY;
	}

	if (chip_dwc3_is_powerdown()) {
		/* need reset controller */
		usb3_misc_reg_clrbit(8, 0xa0ul); /* misc reg bit 8 */

		/* reset usb2.0 phy */
		usb3_sc_misc_reg_clrbit(0, 0x618);

		/* close misc clk gate */
		clk_disable_unprepare(usb_phy->clk);
		clk_disable_unprepare(usb_phy->gt_aclk_usb3otg);
	}

	usb_dbg("-\n");
	return 0;
}

int chip_usb_combophy_notify(enum phy_change_type type)
{
	int ret = 0;

	usb_dbg("+\n");
	if (!_usb_phy) {
		usb_err("usb_phy is NULL, dwc3-usb have some problem!\n");
		return -ENOMEM;
	}

	/*
	 * check if usb controller is busy.
	 */
	if (!chip_dwc3_is_powerdown())
		usb_err("usb core is busy! maybe !\n");

	if (type == PHY_MODE_CHANGE_BEGIN) {
		ret = mar_usb31_core_enable_u3(_usb_phy);
	} else if (type == PHY_MODE_CHANGE_END) {
		mar_usb31_core_disable_u3(_usb_phy);
	} else {
		usb_err("Bad param!\n");
		ret = -EINVAL;
	}
	usb_dbg("-\n");
	return ret;
}

static int mar_usb3phy_shutdown(unsigned int support_dp)
{
	static int flag;

	usb_dbg("+\n");

	if (!_usb_phy)
		return -ENODEV;

	if (chip_dwc3_is_powerdown()) {
		usb_dbg("already shutdown, just return!\n");
		return 0;
	}
	set_chip_dwc3_power_flag(USB_POWER_HOLD);

	if (!support_dp || !flag) {
		dwc3_phy_reset(_usb_phy);
		flag = 1;
	}

	dwc3_reset(_usb_phy);

	dwc3_misc_ctrl_put(MICS_CTRL_USB);

	usb_dbg(":DP_AUX_LDO_CTRL_USB disable\n");
	(void)usb3_phy_33v_disable(_usb_phy);

	set_chip_dwc3_power_flag(USB_POWER_OFF);
	usb_dbg("-\n");

	return 0;
}

static int usb3_phy_33v_enable(struct plat_chip_usb_phy *usb_phy)
{
	int ret;
	struct regulator *ldo_supply = usb_phy->usb20phy_power;

	usb_dbg("+\n");
	if (ldo_supply == NULL) {
		usb_err("usb3.3v ldo enable!\n");
		return -ENODEV;
	}

	if (usb_phy->usb20phy_power_flag == 1)
		return 0;

	ret = regulator_enable(ldo_supply);
	if (ret) {
		usb_err("regulator enable failed %d!\n", ret);
		return -EPERM;
	}

	usb_phy->usb20phy_power_flag = 1;
	usb_dbg("regulator enable success!\n");
	return 0;
}

static int usb3_phy_33v_disable(struct plat_chip_usb_phy *usb_phy)
{
	int ret;
	struct regulator *ldo_supply = usb_phy->usb20phy_power;

	usb_dbg("+\n");
	if (ldo_supply == NULL) {
		usb_err("usb3.3v ldo disable!\n");
		return -ENODEV;
	}

	if (usb_phy->usb20phy_power_flag == 0) {
		WARN_ON(1);
		return 0;
	}

	ret = regulator_disable(ldo_supply);
	if (ret) {
		usb_err("regulator disable failed %d!\n", ret);
		return -EPERM;
	}

	usb_phy->usb20phy_power_flag = 0;
	usb_dbg("regulator disable success!\n");
	return 0;
}

static struct regulator *usb3_phy_ldo_33v_dts(struct device *dev)
{
	struct regulator *supply = NULL;
	int cur_uv;

	supply = regulator_get(dev, "usb_phy_ldo_33v");
	if (IS_ERR(supply)) {
		usb_err("get usb3 phy ldo 3.3v failed!\n");
		return NULL;
	}

	cur_uv = regulator_get_voltage(supply);

	usb_dbg("phy voltage:[%d]", cur_uv);
	return supply;
}

static int mar_get_clk_resource(struct device *dev, struct plat_chip_usb_phy *usb_phy)
{
	/* get abb clk handler */
	usb_phy->clk = devm_clk_get(dev, "clk_usb3phy_ref");
	if (IS_ERR_OR_NULL(usb_phy->clk)) {
		dev_err(dev, "get usb3phy ref clk failed\n");
		return -EINVAL;
	}

	/* get a clk handler */
	usb_phy->gt_aclk_usb3otg = devm_clk_get(dev, "aclk_usb3otg");
	if (IS_ERR_OR_NULL(usb_phy->gt_aclk_usb3otg)) {
		dev_err(dev, "get aclk_usb3otg failed\n");
		return -EINVAL;
	}

	/* get h clk handler */
	usb_phy->gt_hclk_usb3otg = devm_clk_get(dev, "hclk_usb3otg");
	if (IS_ERR_OR_NULL(usb_phy->gt_hclk_usb3otg)) {
		dev_err(dev, "get hclk_usb3otg failed\n");
		return -EINVAL;
	}

	/* get tcxo clk handler */
	usb_phy->gt_clk_usb3_tcxo_en = devm_clk_get(dev, "clk_usb3_tcxo_en");
	if (IS_ERR_OR_NULL(usb_phy->gt_clk_usb3_tcxo_en)) {
		dev_err(dev, "get clk_usb3_tcxo_en failed\n");
		return -EINVAL;
	}
	/* get usb2phy ref clk handler */
	usb_phy->gt_clk_usb2phy_ref = devm_clk_get(dev, "clk_usb2phy_ref");
	if (IS_ERR_OR_NULL(usb_phy->gt_clk_usb2phy_ref)) {
		dev_err(dev, "get clk_usb2phy_ref failed\n");
		return -EINVAL;
	}

	return 0;
}

static int mar_get_dts_resource(struct device *dev, struct plat_chip_usb_phy *usb_phy)
{
	void __iomem *regs = NULL;

	if (mar_get_clk_resource(dev, usb_phy))
		return -EINVAL;

	/*
	 * map misc ctrl region
	 */
	usb_phy->phy.otg_bc_reg_base = of_iomap(dev->of_node, 0);
	if (!usb_phy->phy.otg_bc_reg_base) {
		dev_err(dev, "ioremap res 0 failed\n");
		return -ENOMEM;
	}

	/*
	 * map PERI CRG region
	 */
	regs = of_devm_ioremap(dev, "hisilicon,crgctrl");
	if (IS_ERR(regs)) {
		usb_err("iomap peri cfg failed!\n");
		return PTR_ERR(regs);
	}
	usb_phy->pericfg_reg_base = regs;

	/*
	 * map SCTRL region
	 */
	regs = of_devm_ioremap(dev, "hisilicon,sysctrl");
	if (IS_ERR(regs)) {
		usb_err("iomap sctrl_reg_base failed!\n");
		return PTR_ERR(regs);
	}
	usb_phy->sctrl_reg_base = regs;

	/*
	 * map PCTRL region
	 */
	regs = of_devm_ioremap(dev, "hisilicon,pctrl");
	if (IS_ERR(regs)) {
		usb_err("iomap pctrl failed!\n");
		return PTR_ERR(regs);
	}
	usb_phy->pctrl_reg_base = regs;

	if (of_property_read_u32(dev->of_node, "usb_aclk_freq",
				&g_usb3otg_aclk_freq))
		g_usb3otg_aclk_freq = 229000000; /* otg clk freq set 229000000 */

	dev_info(dev, "boart config set usb aclk freq:%d\n", g_usb3otg_aclk_freq);

	if (of_property_read_u32(dev->of_node, "usb3_phy_term",
			&(usb_phy->usb3_phy_term))) {
		usb_dbg("get usb3_phy_term form dt failed, set invalid value\n");
		usb_phy->usb3_phy_term = TERM_INVALID_PARAM;
	}
	usb_dbg("usb3_phy_term: %x\n", usb_phy->usb3_phy_term);

	usb_phy->usb20phy_power = usb3_phy_ldo_33v_dts(dev);
	usb_phy->usb20phy_power_flag = 0;

	init_misc_ctrl_addr(usb_phy->phy.otg_bc_reg_base);
	init_sc_misc_ctrl_addr(usb_phy->sctrl_reg_base);
	return 0;
}

static void mar_disable_usb3(void)
{
	/* need reset controller */
	usb3_misc_reg_clrbit(CFGA0_USB31C_RESETN_B, USB_MISC_CFGA0);
	udelay(100); /* delay 100ms */

	/* disable usb3 SS port */
	usb3_misc_reg_setbit(HOST_U3_PORT_DISABLE_B, USBOTG3_CTRL1);
	udelay(100); /* delay 100ms */

	/* unreset controller */
	usb3_misc_reg_setbit(CFGA0_USB31C_RESETN_B, USB_MISC_CFGA0);
}

static int mar_shared_phy_init(unsigned int support_dp, unsigned int eye_diagram_param,
		unsigned int combophy_flag)
{
	int ret;
	uint32_t temp;

	usb_dbg("+\n");

	if (!_usb_phy)
		return -ENODEV;

#ifdef CONFIG_CONTEXTHUB_PD
	if (support_dp && combophy_flag)
		combophy_poweroff();
#endif

	usb_dbg(":DP_AUX_LDO_CTRL_HIFIUSB enable\n");
	ret = usb3_phy_33v_enable(_usb_phy);
	if (ret && ret != -ENODEV) {
		usb_err("usb3_phy_33v_enable failed!\n");
		return ret;
	}

	ret = dwc3_misc_ctrl_get(MICS_CTRL_USB);
	if (ret) {
		usb_err("usb driver not ready!\n");
		goto err_misc_ctrl_get;
	}
	udelay(100); /* delay 100ms */

	/* disable pme */
	usb3_misc_reg_clrbit(PME_EN, USB_MISC_CFG84);

	/* enable 2.0 phy refclk */
	ret = clk_prepare_enable(_usb_phy->gt_clk_usb2phy_ref);
	if (ret) {
		usb_err("clk_prepare_enable gt_clk_usb2phy_ref failed\n");
		goto err_misc_clk_usb2phy;
	}

	/* hifi use 16bit-30M utmi, reg start bit 8, end bit 9 */
	usb3_sc_misc_reg_setvalue(SET_NBITS_MASK(8, 9), 0x600);

	/* Release USB20 PHY out of IDDQ mode */
	usb3_sc_misc_reg_clrbit(0u, 0x608ul);

	/* Usb20phy port control signal usb2_refclksel select */
	temp = usb3_sc_misc_reg_readl(0x60cul);
	temp &= ~(0x3 << 3); /* move Left 3 bit */
	temp |= (0x2 << 3); /* move Left 3 bit */
	usb3_sc_misc_reg_writel(temp, 0x60cul);

	/* select usb20phy for audio */
	usb3_sc_misc_reg_setbit(5, 0x618ul); /* misc reg set bit 5 */
	chip_usb_switch_sharedphy_if_fpga(1);

	/* select usb 20phy drv vbus for audio */
	temp = usb3_sc_misc_reg_readl(0x618ul);
	temp &= ~(0x3 << 6); /* move Left 6 bit */
	temp |= (0x2 << 6); /* move Left 6 bit */
	usb3_sc_misc_reg_writel(temp, 0x618ul);

	/* release 2.0 phy */
	usb3_sc_misc_reg_setbit(0, 0x618ul);
	chip_usb_unreset_phy_if_fpga();
	udelay(100); /* delay 100ms */

	set_usb2_eye_diagram_param(_usb_phy, eye_diagram_param);

	usb_dbg("-\n");
	return 0;

err_misc_clk_usb2phy:
	dwc3_misc_ctrl_put(MICS_CTRL_USB);
err_misc_ctrl_get:
#ifdef CONFIG_CONTEXTHUB_PD
	if (support_dp && combophy_flag) {
		usb_dbg("combophy_sw_sysc +\n");
		ret = dwc3_combophy_sw_sysc(TCPC_USB31_CONNECTED);
		if (ret)
			usb_err("combophy_sw_sysc failed %d\n", ret);
		usb_dbg("combophy_sw_sysc -\n");
	}
#endif
	usb_dbg(":DP_AUX_LDO_CTRL_HIFIUSB disable\n");
	(void)usb3_phy_33v_disable(_usb_phy);

	return ret;
}

static int mar_shared_phy_shutdown(unsigned int support_dp,
		unsigned int combophy_flag, unsigned int keep_power)
{
	uint32_t temp;
	int ret = 0;

	usb_dbg("+\n");

	if (!_usb_phy)
		return -ENODEV;

	usb_dbg("combophy_flag %u, keep_power %u\n", combophy_flag, keep_power);

	/* reset 2.0 phy */
	usb3_sc_misc_reg_clrbit(0, 0x618);

	/* switch drv vbus */
	temp = usb3_sc_misc_reg_readl(0x618ul);
	temp &= ~(0x3 << 6); /* move left 6 bit */
	usb3_sc_misc_reg_writel(temp, 0x618ul);

	/* switch phy */
	usb3_sc_misc_reg_clrbit(5, 0x618ul); /* bit 5 */
	chip_usb_switch_sharedphy_if_fpga(0);

	/* clock source select */
	// nothing

	/* enable siddq */
	usb3_sc_misc_reg_setbit(0u, 0x608ul);

	/* disable 2.0 phy refclk */
	clk_disable_unprepare(_usb_phy->gt_clk_usb2phy_ref);

	dwc3_misc_ctrl_put(MICS_CTRL_USB);

#ifdef CONFIG_CONTEXTHUB_PD
	if (support_dp && combophy_flag) {
		usb_dbg("combophy_sw_sysc +\n");
		ret = dwc3_combophy_sw_sysc(TCPC_USB31_CONNECTED);
		if (ret)
			usb_err("combophy_sw_sysc failed %d\n", ret);
		usb_dbg("combophy_sw_sysc -\n");
	}
#endif

	if (!keep_power) {
		usb_dbg(":DP_AUX_LDO_CTRL_HIFIUSB disable\n");
		ret = usb3_phy_33v_disable(_usb_phy);
		if (ret && ret != -ENODEV)
			usb_err("usb3_phy_33v_disable failed!\n");
	}

	usb_dbg("-\n");
	return ret;
}

static void __iomem *mar_get_bc_ctrl_reg(void)
{
	if (!_usb_phy)
		return NULL;

	return _usb_phy->sctrl_reg_base + SC_USB20PHY_MISC_CTRL4;
}

static void config_usbphy_power(const void __iomem *sctrl_base)
{
	volatile uint32_t scdeepsleeped;

	scdeepsleeped = (uint32_t)readl(SCTRL_SCDEEPSLEEPED + sctrl_base);
	if ((scdeepsleeped & (1 << 7)) != 0)
		usb3_sc_misc_reg_setbit(12, 0x60c); /* power on phy */
}

static int mar_usb3phy_init(unsigned int support_dp,
		unsigned int eye_diagram_param,
		unsigned int usb3_phy_tx_vboost_lvl)
{
	int ret;
	static int flag;
	void __iomem *sctrl_base;

	usb_dbg("+\n");

	if (!_usb_phy)
		return -ENODEV;

	sctrl_base = _usb_phy->sctrl_reg_base;

	if (!chip_dwc3_is_powerdown()) {
		usb_dbg("already release, just return!\n");
		return 0;
	}

	usb_dbg(":DP_AUX_LDO_CTRL_USB enable\n");
	(void)usb3_phy_33v_enable(_usb_phy);

	config_usbphy_power(sctrl_base);

	ret = dwc3_misc_ctrl_get(MICS_CTRL_USB);
	if (ret) {
		usb_err("usb driver not ready!\n");
		goto out;
	}

	/* disable pme */
	usb3_misc_reg_clrbit(PME_EN, USB_MISC_CFG84);

	if (!support_dp || !flag) {
		ret = dwc3_phy_release();
		if (ret) {
			usb_err("phy release err!\n");
			goto out_misc_put;
		}
	}

	if (support_dp && !combophy_regcfg_is_misc_ctrl_unreset()) {
		usb_err("[USBDP.DBG] goto here, need check who call.\n");
		goto out_phy_reset;
	}

	ret = dwc3_release(_usb_phy, support_dp);
	if (ret) {
		usb_err("[RELEASE.ERR] release error, need check clk!\n");
		goto out_phy_reset;
	}

	if (combophy_is_highspeed_only()) {
		usb_dbg("set USB2OTG_ACLK_FREQ\n");
		ret = clk_set_rate(_usb_phy->gt_aclk_usb3otg, g_usb2otg_aclk_freq);
		if (ret)
			usb_err("Can't aclk rate set\n");
	} else {
		/* need reset clk freq */
		ret = usb3_clk_set_rate(_usb_phy);
		if (ret) {
			usb_err("usb reset clk rate fail!\n");
			goto out_phy_reset;
		}
	}

	set_usb2_eye_diagram_param(_usb_phy, eye_diagram_param);
	set_vboost_for_usb3(usb3_phy_tx_vboost_lvl);
	if (_usb_phy->usb3_phy_term != TERM_INVALID_PARAM)
		set_phy_term_for_usb3(_usb_phy->usb3_phy_term);

	set_chip_dwc3_power_flag(USB_POWER_ON);
	flag = 1;

	usb_dbg("-\n");
	return ret;

out_phy_reset:
	if (!support_dp || !flag)
		dwc3_phy_reset(_usb_phy);
out_misc_put:
	dwc3_misc_ctrl_put(MICS_CTRL_USB);
out:
	usb_dbg(":DP_AUX_LDO_CTRL_USB disable\n");
	(void)usb3_phy_33v_disable(_usb_phy);
	return ret;
}

static int dwc3_mar_probe(struct platform_device *pdev)
{
	int ret;

	_usb_phy = devm_kzalloc(&pdev->dev, sizeof(*_usb_phy),
			GFP_KERNEL);
	if (!_usb_phy)
		return -ENOMEM;

	ret = mar_get_dts_resource(&pdev->dev, _usb_phy);
	if (ret) {
		usb_err("get_dts_resource failed\n");
		goto err_out;
	}

	_usb_phy->phy.init = mar_usb3phy_init;
	_usb_phy->phy.shutdown = mar_usb3phy_shutdown;
	_usb_phy->phy.shared_phy_init = mar_shared_phy_init;
	_usb_phy->phy.shared_phy_shutdown = mar_shared_phy_shutdown;
	_usb_phy->phy.notify_speed = mar_notify_speed;
	_usb_phy->phy.disable_usb3 = mar_disable_usb3;
	_usb_phy->phy.get_term = mar_get_phy_term;
	_usb_phy->phy.set_term = mar_set_phy_term;
	_usb_phy->phy.get_bc_ctrl_reg = mar_get_bc_ctrl_reg;

	ret = chip_usb_dwc3_register_phy(&_usb_phy->phy);
	if (ret) {
		usb_err("usb_dwc3_register_phyfailed\n");
		goto err_out;
	}

	return 0;
err_out:
	_usb_phy = NULL;
	return ret;
}

static int dwc3_mar_remove(struct platform_device *pdev)
{
	int ret;

	ret = chip_usb_dwc3_unregister_phy(&_usb_phy->phy);
	_usb_phy = NULL;

	return ret;
}

#ifdef CONFIG_OF
static const struct of_device_id dwc3_mar_match[] = {
	{ .compatible = "hisilicon,mar-dwc3" },
	{},
};
MODULE_DEVICE_TABLE(of, dwc3_mar_match);
#else
#define dwc3_mar_match NULL
#endif

static struct platform_driver dwc3_mar_driver = {
	.probe		= dwc3_mar_probe,
	.remove		= dwc3_mar_remove,
	.driver		= {
		.name	= "usb3-mar",
		.of_match_table = of_match_ptr(dwc3_mar_match),
	},
};
module_platform_driver(dwc3_mar_driver);
MODULE_LICENSE("GPL");
