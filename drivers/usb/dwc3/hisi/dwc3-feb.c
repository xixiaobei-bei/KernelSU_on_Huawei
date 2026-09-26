#include <linux/module.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <pmic_interface.h>
#include <linux/mfd/hisi_pmic.h>
#include "dwc3-hisi.h"
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/hisi/usb/hisi_usb_interface.h>
#include <linux/hisi/usb/dwc3_usb_interface.h>
#include <linux/jiffies.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/dp_aux_switch/dp_aux_switch.h>
#ifdef CONFIG_CONTEXTHUB_PD
#include <linux/hisi/contexthub/tca.h>
#endif

#include "hisi_usb3_misctrl.h"
#include "hisi_usb3_31phy.h"

/*lint -e750 -esym(750,*)*/

#define SCTRL_SCDEEPSLEEPED				0x08
#define USB_REFCLK_ISO_EN               (1 << 25)
#define SC_CLK_USB3PHY_3MUX1_SEL        (1 << 25)

#define SC_SEL_ABB_BACKUP               (1 << 8)
#define CLKDIV_MASK_START               (16)

#define PERI_CRG_CLKDIV21               0xFC

#define GT_CLK_ABB_BACKUP               (1 << 22)
#define PERI_CRG_CLK_DIS5               0x54

#define PMC_PPLL3CTRL0                  0x048
#define PPLL3_FBDIV_START               (8)
#define PPLL3_EN                        (1 << 0)
#define PPLL3_BP                        (1 << 1)
#define PPLL3_LOCK                      (1 << 26)

#define PMC_PPLL3CTRL1                  0x04C
#define PPLL3_INT_MOD                   (1 << 24)
#define GT_CLK_PPLL3                    (1 << 26)

#define PERI_CRG_CLK_EN5		0x50
#define PERI_CRG_PERRSTSTAT4		0x98

#define SC_USB3PHY_ABB_GT_EN            (1 << 15)
#define REF_SSP_EN                      (1 << 16)

#define GT_CLK_USB3OTG_REF				(1 << 0)
#define GT_ACLK_USB3OTG					(1 << 1)
#define GT_HCLK_USB3OTG_MISC				(1 << 25)

#define IP_RST_USB3OTG_32K		(1 << 6)
#define IP_RST_USB3OTG_MISC		(1 << 7)

/* clk freq usb default usb3.0 228M usb2.0 60M */
static uint32_t USB3OTG_ACLK_FREQ = 229000000;
static uint32_t USB2OTG_ACLK_FREQ = 61000000;

extern struct hisi_dwc3_device *hisi_dwc3_dev;
extern bool __clk_is_enabled(struct clk *clk);

int usb3_open_misc_ctrl_clk(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;

	if (!hisi_dwc3) {
		usb_err("usb driver not setup!\n");
		return -EINVAL;
	}

	/* open hclk gate */
	ret = clk_prepare_enable(hisi_dwc3->gt_hclk_usb3otg);
	if (ret) {
		usb_err("clk_enable gt_hclk_usb3otg failed\n");
		return ret;
	}

	if (__clk_is_enabled(hisi_dwc3->gt_hclk_usb3otg) == false) {
		usb_err("gt_hclk_usb3otg  enable err\n");
		return -EFAULT;
	}

	return 0;
}

void usb3_close_misc_ctrl_clk(struct hisi_dwc3_device *hisi_dwc3)
{
	if (!hisi_dwc3) {
		usb_err("usb driver not setup!\n");
		return;
	}

	/* disable usb3otg hclk */
	clk_disable_unprepare(hisi_dwc3->gt_hclk_usb3otg);
}

static int usb3_misc_ctrl_is_unreset(void __iomem *pericfg_base)
{
	volatile uint32_t regval;
	regval = (uint32_t)readl(PERI_CRG_PERRSTSTAT4 + pericfg_base);
	return !((IP_RST_USB3OTG_MISC | IP_RST_USB3OTG_32K) & regval);
}

static int usb3_misc_ctrl_is_reset(void __iomem *pericfg_base)
{
	return !usb3_misc_ctrl_is_unreset(pericfg_base);
}

static int dwc3_combophy_sw_sysc(struct hisi_dwc3_device *hisi_dwc3, TCPC_MUX_CTRL_TYPE new_mode)
{
	int ret;

	ret = combophy_sw_sysc(TCPC_USB31_CONNECTED,
			(TYPEC_PLUG_ORIEN_E)hisi_dwc3->plug_orien,
			!get_hifi_usb_retry_count());
	if (ret)
		usb_err("combophy_sw_sysc failed(%d)\n",ret);
	return ret;
}

static int config_usb_phy_controller(struct hisi_dwc3_device *hisi_dwc3)
{
	volatile uint32_t temp;
	static int flag = 1;
	/* Release USB20 PHY out of IDDQ mode */
	usb3_misc_reg_clrbit(0u, 0x14ul);

	if (!hisi_dwc3->support_dp || flag) {
		flag = 0;
		/* Release USB31 PHY out of  TestPowerDown mode */
		usb3_misc_reg_clrbit(23u, 0x50ul);

		/* Tell the PHY power is stable */
		usb3_misc_reg_setvalue((1u << 1) | (1u << 9) | (1u << 11), 0x54ul);


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
		temp &= ~(SET_NBITS_MASK(0, 1) | SET_NBITS_MASK(3, 4));
		temp |= (0x1 | (0x2 << 3));
		usb3_misc_reg_writel(temp, 0x214);

		usb3_misc_reg_setvalue(0x3u | (0x3u << 2), 0x240ul);
	}

	/* Enable SSC */
	usb3_misc_reg_setbit(1, 0x5c);

	return 0;
}

static uint32_t is_abb_clk_selected(void __iomem *sctrl_base)
{
	volatile uint32_t scdeepsleeped;

	if(!sctrl_base) {
		usb_err("sctrl_base is NULL!\n");
		return 1;
	}

	scdeepsleeped = (uint32_t)readl(SCTRL_SCDEEPSLEEPED + sctrl_base);
	if ((scdeepsleeped & (1 << 20)) == 0)
		return 1;

	return 0;
}


static int set_combophy_clk(struct hisi_dwc3_device *hisi_dwc3)
{
/*lint -save -e550 */
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;/*lint !e438 */
	void __iomem *pctrl_base = hisi_dwc3->pctrl_reg_base;/*lint !e438 */
	void __iomem *sctrl_base = hisi_dwc3->sctrl_reg_base;
#define USB_CLK_TYPE_ABB (0xabb)
#define USB_CLK_TYPE_PAD (0x10ad)
	volatile uint32_t temp;
	int ret;
	int clk_type = USB_CLK_TYPE_ABB;

	if (is_abb_clk_selected(sctrl_base)) {
		/* usb refclk iso enable */
		writel(USB_REFCLK_ISO_EN, PERI_CRG_ISODIS + pericfg_base);

		/* enable usb_tcxo_en */
		ret = clk_prepare_enable(hisi_dwc3->gt_clk_usb3_tcxo_en);
		if (ret) {
			usb_err("clk_prepare_enable gt_clk_usb3_tcxo_en failed\n");
			return ret;
		}

		mdelay(10);

		/* select usbphy clk from abb */
		temp = (uint32_t)readl(pctrl_base + PCTRL_PERI_CTRL24);
		temp &= ~SC_CLK_USB3PHY_3MUX1_SEL;
		writel(temp, pctrl_base + PCTRL_PERI_CTRL24);
	} else {
		usb_err("pad clk? maybe is bug\n");
		/* select usbphy clk from pad */
		usb3_misc_reg_setbit(12, 0x54);

		usb3_misc_reg_setbit(4, 0xa0);

		temp = usb3_misc_reg_readl(0x1c);
		temp &= ~(SET_NBITS_MASK(3, 4));
		temp |= (0x2 << 3);
		usb3_misc_reg_writel(temp, 0x1cul);

		ret = clk_prepare_enable(hisi_dwc3->gt_clk_usb2phy_ref);
		if (ret) {
			usb_err("clk_prepare_enable gt_clk_usb2phy_ref failed\n");
			return ret;
		}


		clk_type = USB_CLK_TYPE_PAD;
	}

	usb_dbg("usb clk type:%x\n", clk_type);
	return ret;
/*lint -restore */
}

static void set_gpio_if_fpga(struct hisi_dwc3_device *hisi_dwc3)
{
	if (hisi_dwc3->fpga_flag != 0) {
		if(gpio_request(15, "usb_fpga")) {
			usb_err("request gpio error!\n");
		}

		gpio_direction_output(15, 1);
		udelay(100);

		gpio_direction_output(15, 0);
		udelay(100);
	}
}

int usb3_clk_is_open(void __iomem *pericfg_base)
{
	volatile uint32_t hclk, aclk;
	uint32_t hclk_mask = GT_HCLK_USB3OTG_MISC;
	uint32_t aclk_mask = (GT_CLK_USB3OTG_REF | GT_ACLK_USB3OTG);

#define PERI_CRG_PERSTAT0 0x0c
#define PERI_CRG_PERSTAT4 0x4c
	hclk = (uint32_t)readl(PERI_CRG_PERSTAT0 + pericfg_base);
	aclk = (uint32_t)readl(PERI_CRG_PERSTAT4 + pericfg_base);

	return (   ((aclk_mask & aclk) == aclk_mask)
		&& ((hclk_mask & hclk) == hclk_mask) );
}

int usb3_hclk_is_close(void __iomem *pericfg_base)
{
	volatile uint32_t hclk;
	uint32_t hclk_mask = GT_HCLK_USB3OTG_MISC;

#define PERI_CRG_PERSTAT0 0x0c
	hclk = (uint32_t)readl(PERI_CRG_PERSTAT0 + pericfg_base);

	return ((hclk_mask & hclk) != hclk_mask);
}

int usb3_phy_controller_is_release(void)
{
	volatile uint32_t stat;
	uint32_t bitmask = (SET_NBITS_MASK(8, 9) | SET_NBITS_MASK(0,1));

	stat = usb3_misc_reg_readl(0xa0);

	return ((bitmask & stat) == bitmask);
}

int dwc3_set_combophy_clk(void)
{
	int ret = 0;
	struct hisi_dwc3_device *hisi_dwc3 = hisi_dwc3_dev;

	ret = set_combophy_clk(hisi_dwc3);
	if(ret) {
		usb_err("[CLK.ERR] combophy clk set error!\n");
	}
	return ret;
}

static int dwc3_phy_release(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret = 0;
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;

	if(usb3_misc_ctrl_is_unreset(pericfg_base) && usb3_clk_is_open(pericfg_base)) {
		usb_err("usb3_misc_ctrl_is_unreset && usb3_clk_is_open\n");
	} else {
		usb_dbg("release phy\n");
		/* unreset phy */
		usb3_misc_reg_setvalue(SET_NBITS_MASK(0, 1), 0xa0);
		udelay(100);
		set_gpio_if_fpga(hisi_dwc3);

		ret = dwc3_set_combophy_clk();
	}

	return ret;
}

static void close_combophy_clk(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *sctrl_base = hisi_dwc3->sctrl_reg_base;

	if (is_abb_clk_selected(sctrl_base)) {
		/* disable usb_tcxo_en */
		clk_disable_unprepare(hisi_dwc3->gt_clk_usb3_tcxo_en);
	} else {
		/* close phy ref clk */
		clk_disable_unprepare(hisi_dwc3->gt_clk_usb2phy_ref);
	}
}


static void dwc3_phy_reset(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;

	if(usb3_misc_ctrl_is_unreset(pericfg_base) && usb3_clk_is_open(pericfg_base)) {
		/* reset phy */
		usb3_misc_reg_clrvalue(~(SET_NBITS_MASK(0, 1)), 0xa0ul); /*lint !e835 */
		usb_dbg("reset phy\n");

		close_combophy_clk(hisi_dwc3);
	}
}

int dwc3_open_controller_clk(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret = 0;
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;/*lint !e529 */
	volatile int temp;
	/* open misc clk gate */
	ret = clk_prepare_enable(hisi_dwc3->gt_aclk_usb3otg);
	if (ret) {
		usb_err("clk_prepare_enable gt_aclk_usb3otg failed\n");
		return -EACCES;
	}

	temp = readl(pericfg_base + 0x48);
	usb_err("[0x48:%x]\n", temp);

	ret = clk_prepare_enable(hisi_dwc3->clk);
	if (ret) {
		usb_err("clk_prepare_enable clk failed\n");
		return -EACCES;
	}
	temp = readl(pericfg_base + 0x48);
	usb_err("[0x48:%x]\n", temp);

	usb3_misc_reg_clrbit(4, 0xa0);

	/* switch phy */
	usb3_misc_reg_clrbit(5, 0xa0ul);

	usb3_misc_reg_setvalue((SET_NBITS_MASK(3, 4)), 0x1c);
	return ret;
}

int dwc3_compliance_mode_enable(struct hisi_dwc3_device *hisi_dwc3)
{
	usb_err("cp_test start!\n");
	if (!hisi_dwc3 || !hisi_dwc3->core_ops) {
		usb_err("[USB.CP0] usb driver not setup!\n");
		return -ENODEV;
	}
	return hisi_dwc3->core_ops->compliance_mode_enable();
}

int dwc3_cptest_next_pattern(void)
{
	if (!hisi_dwc3_dev || !hisi_dwc3_dev->core_ops) {
		usb_err("[USB.CP0] usb driver not setup!\n");
		return -ENODEV;
	}
	return hisi_dwc3_dev->core_ops->cptest_next_pattern();
}

void dwc3_hisi_lscdtimer_set(void)
{
	if (!hisi_dwc3_dev || !hisi_dwc3_dev->core_ops) {
		usb_err("[USB.CP0] usb driver not setup!\n");
		return;
	}
	hisi_dwc3_dev->core_ops->lscdtimer_set();
}

static int dwc3_set_highspeed_only(void)
{
	int i = 0;

	/* * * * * * * * * * * * * * * * * * *
	 * check if usb phy cr read success.
	 * * * * * * * * * * * * * * * * * * */
	while (0x74cd != usb31phy_cr_read(0)) {
		i++;
		if(i > 10) {
			usb_err("crphy read timeout!\n");
			return -ENOMEM;
		}
		udelay(1);
	}

	/* Override value for ref_clk_mplla_div2_en */
	usb31phy_cr_write(5, usb31phy_cr_read(5) | (1u << 15) | 1);

	/* disable usb3 SS port */
	usb3_misc_reg_setbit(1, 0x4);

/*lint -e648 -esym(648,*)*/
	usb3_misc_reg_clrvalue((u32)(~GENMASK(7, 4)), 0x7c);
/*lint -e648 +esym(648,*)*/

	udelay(100);

	return 0;
}

static int dwc3_release(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret = 0;

	usb_dbg("+\n");

	ret = dwc3_open_controller_clk(hisi_dwc3);
	if (ret) {
		usb_err("[CLK.ERR] open clk error!\n");
		return ret;
	}

	config_usb_phy_controller(hisi_dwc3);

	/* release usb2.0 phy */
	usb3_misc_reg_setbit(0, 0xa0);

	if (combophy_is_highspeed_only()) {
		usb_dbg("[USB.DP] DP4 mode, set usb2.0 only\n");
		dwc3_set_highspeed_only();
	}

	if (hisi_dwc3_get_dt_host_maxspeed() < USB_SPEED_SUPER_PLUS) {
		usb_dbg("[USB.LINK] usb host super-speed only!\n");
		usb3_misc_reg_setbit(30, 0x7c);
	}

	/* unreset controller */
	usb3_misc_reg_setvalue(SET_NBITS_MASK(8, 9), 0xa0);
	udelay(100);

	/* set vbus valid */
	usb3_misc_reg_setvalue(SET_NBITS_MASK(6, 7), 0x0);
	usb3_misc_reg_setvalue(SET_NBITS_MASK(5, 6), 0xc);

	/* require to delay 10us */
	udelay(100);
	usb_dbg("-\n");

	return ret;
}

static void feb_notify_speed(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;

	usb_dbg("+device speed is %d\n", hisi_dwc3->speed);

	if ((hisi_dwc3->speed != USB_CONNECT_HOST) && (hisi_dwc3->speed != USB_CONNECT_DCP))
		hw_usb_set_usb_speed(hisi_dwc3->speed);

	if (((hisi_dwc3->speed < USB_SPEED_WIRELESS) && (hisi_dwc3->speed > USB_SPEED_UNKNOWN))
		||(hisi_dwc3->speed == USB_CONNECT_DCP)) {
		usb_dbg("set USB2OTG_ACLK_FREQ\n");
		ret = clk_set_rate(hisi_dwc3->gt_aclk_usb3otg, USB2OTG_ACLK_FREQ);
		if (ret) {
			usb_err("Can't aclk rate set\n");
		}
	} else if ((hisi_dwc3->speed == USB_CONNECT_HOST) && (!hisi_dwc3_is_powerdown())) {
		usb_dbg("set USB3OTG_ACLK_FREQ\n");
		ret = clk_set_rate(hisi_dwc3->gt_aclk_usb3otg, USB3OTG_ACLK_FREQ);
		if (ret) {
			usb_err("Can't aclk rate set\n");
		}
	}

	usb_dbg("-\n");
}


static void dwc3_reset(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;
/*lint -e438 -esym(438,*)*/
/*lint -e529 -esym(529,*)*/

	usb_dbg("+\n");
	if(usb3_misc_ctrl_is_unreset(pericfg_base) && usb3_clk_is_open(pericfg_base)) {
		/* set vbus valid */
		/* reset controller */
		usb3_misc_reg_clrvalue(~(SET_NBITS_MASK(8, 9)), 0xa0ul);
		usb_err("reset controller\n");

		/* reset usb2.0 phy */
		usb3_misc_reg_clrbit(0, 0xa0);
	}

	clk_disable_unprepare(hisi_dwc3->clk);
	clk_disable_unprepare(hisi_dwc3->gt_aclk_usb3otg);

	usb_dbg("close usb success\n");
}

static int usb3_clk_set_rate(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;

	/* set usb aclk 228MHz to improve performance */
	usb_dbg("set aclk rate [%d]\n", USB3OTG_ACLK_FREQ);
	ret = clk_set_rate(hisi_dwc3->gt_aclk_usb3otg, USB3OTG_ACLK_FREQ);
	if (ret) {
		usb_err("Can't aclk rate set, current rate is %ld:\n", clk_get_rate(hisi_dwc3->gt_aclk_usb3otg));
	}
	return ret;
}

void set_usb2_eye_diagram_param(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *otg_bc_base = hisi_dwc3->otg_bc_reg_base;

	if (hisi_dwc3->fpga_flag != 0)
		return;

	/* set high speed phy parameter */
	if (hisi_dwc3->host_flag) {
		writel(hisi_dwc3->eye_diagram_host_param, otg_bc_base + USBOTG3_CTRL4);
		usb_dbg("set hs phy param 0x%x for host\n",
				readl(otg_bc_base + USBOTG3_CTRL4));
	} else {
		writel(hisi_dwc3->eye_diagram_param, otg_bc_base + USBOTG3_CTRL4);
		usb_dbg("set hs phy param 0x%x for device\n",
				readl(otg_bc_base + USBOTG3_CTRL4));
	}
}


#define TX_VBOOST_LVL_REG (0xf)
#define TX_VBOOST_LVL_VALUE_START (6)
#define TX_VBOOST_LVL_VALUE_END (8)
#define TX_VBOOST_LVL_ENABLE SET_BIT_MASK(9)


void set_vboost_for_usb3(uint32_t usb3_phy_tx_vboost_lvl)
{
	volatile u16 temp;

	if ((usb3_phy_tx_vboost_lvl > 7) || (VBOOST_LVL_DEFAULT_PARAM == usb3_phy_tx_vboost_lvl)) {
		usb_dbg("bad vboost (%d) use default (%d)\n", usb3_phy_tx_vboost_lvl, VBOOST_LVL_DEFAULT_PARAM);
		return ;
	}

	temp = usb31phy_cr_read(TX_VBOOST_LVL_REG);
	temp = usb31phy_cr_read(TX_VBOOST_LVL_REG);
	temp &= ~SET_NBITS_MASK(TX_VBOOST_LVL_VALUE_START, TX_VBOOST_LVL_VALUE_END);
	temp |= (u16)((TX_VBOOST_LVL_ENABLE | (usb3_phy_tx_vboost_lvl << TX_VBOOST_LVL_VALUE_START)) & SET_NBITS_MASK(0,15));
	usb31phy_cr_write(TX_VBOOST_LVL_REG, temp);

	temp = usb31phy_cr_read(TX_VBOOST_LVL_REG);
	usb_dbg("[0x%x]usb31 cr param:%x\n", TX_VBOOST_LVL_REG, temp);
}

/*lint -e438 +esym(438,*)*/
/*lint -e529 +esym(529,*)*/
static int feb_usb3phy_init(struct hisi_dwc3_device *hisi_dwc3);

extern struct hisi_dwc3_device *hisi_dwc3_dev;

static int feb_usb31_core_enable_u3(struct hisi_dwc3_device *hisi_dwc3)
{
	volatile u32 temp;
	int ret = 0;
	int power_down_flag;
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;/*lint !e529 */

	usb_dbg("+\n");

	/*
	 * check if misc ctrl is release
	 * check if usb clk is open
	 */
	if (usb3_misc_ctrl_is_reset(pericfg_base) || usb3_hclk_is_close(pericfg_base)) {
		usb_err("misc ctrl or usb3 clk not ready.\n");
		/* Block device required */
		return -ENOTBLK;
	}

	power_down_flag = get_hisi_dwc3_power_flag();
	if (hisi_dwc3_is_powerdown()) {
		/* open usb phy clk gate */
		ret = dwc3_open_controller_clk(hisi_dwc3);
		if (ret) {
			usb_err("[CLK.ERR] open clk fail\n");
			return ret;
		}

		temp = readl(pericfg_base + 0x48);
		if (!(temp & 0x3)) {
			usb_err("[PERI CFG 0X48:0x%x]\n", temp);
			usb_err("[misc 0xa0:0x%x]\n", usb3_misc_reg_readl(0xa0));
			return -ENOTBLK;
		}


		/* Release USB20 PHY out of IDDQ mode */
		usb3_misc_reg_clrbit(0u, 0x14ul);

		/* release usb2.0 phy */
		usb3_misc_reg_setbit(0, 0xa0);
		udelay(100);

		/* unreset controller */
		usb3_misc_reg_setvalue(SET_NBITS_MASK(8, 9), 0xa0);
		udelay(100);
		set_hisi_dwc3_power_flag(USB_POWER_ON);
	}

	ret = hisi_dwc3->core_ops->enable_u3();
	if (get_hisi_dwc3_power_flag() != power_down_flag) {
		set_hisi_dwc3_power_flag(power_down_flag);
	}
	usb_dbg("-\n");
	return ret;
}

static int feb_usb31_core_disable_u3(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;/*lint !e529 */

	usb_dbg("+\n");

	/*
	 * check if misc ctrl is release
	 * check if usb clk is open
	 */
	if (usb3_misc_ctrl_is_reset(pericfg_base) || usb3_hclk_is_close(pericfg_base)) {
		usb_err("misc ctrl or usb3 clk not ready.\n");
		/* Block device required */
		return -ENOTBLK;
	}

	if (hisi_dwc3_is_powerdown()) {
		/* need reset controller */
		usb3_misc_reg_clrvalue(~(SET_NBITS_MASK(8, 9)), 0xa0ul);

		/* reset usb2.0 phy */
		usb3_misc_reg_clrbit(0, 0xa0);

		/* close misc clk gate */
		clk_disable_unprepare(hisi_dwc3->clk);
		clk_disable_unprepare(hisi_dwc3->gt_aclk_usb3otg);
	}

	usb_dbg("-\n");
	return 0;
}

int hisi_usb_combophy_notify(enum phy_change_type type)
{
	int ret = 0;
	struct hisi_dwc3_device *hisi_dwc3 = hisi_dwc3_dev;

	usb_dbg("+\n");
	if (!hisi_dwc3) {
		usb_err("hisi_dwc3 is NULL, dwc3-hisi have some problem!\n");
		return -ENOMEM;
	}

	/*
	 * check if usb controller is busy.
	 */
	if (!hisi_dwc3_is_powerdown()) {
		usb_err("usb core is busy! maybe !\n");
	}

	if (PHY_MODE_CHANGE_BEGIN == type) {
		ret = feb_usb31_core_enable_u3(hisi_dwc3);
	} else if (PHY_MODE_CHANGE_END == type) {
		feb_usb31_core_disable_u3(hisi_dwc3);
	} else {
		usb_err("Bad param!\n");
		ret = -EINVAL;
	}
	usb_dbg("-\n");
	return ret;
}

int feb_usb31_controller_dump(void)
{
	struct hisi_dwc3_device *hisi_dwc3 = hisi_dwc3_dev;

	usb_dbg("+\n");

	if (!hisi_dwc3 || !hisi_dwc3->core_ops) {
		usb_err("hisi_dwc3 is NULL, dwc3-hisi have some problem!\n");
		return -ENOMEM;
	}

	hisi_dwc3->core_ops->dump_regs();
	usb_err("[misc 84:0x%x], [misc 28:0x%x]\n",
		usb3_misc_reg_readl(0x84),
		usb3_misc_reg_readl(0x28));

	usb_dbg("-\n");

	return 0;
}

static int feb_usb3phy_shutdown(struct hisi_dwc3_device *hisi_dwc3)
{
	static int flag = 0;
	usb_dbg("+\n");

	if (hisi_dwc3_is_powerdown()) {
		usb_dbg("already shutdown, just return!\n");
		return 0;
	}

	set_hisi_dwc3_power_flag(USB_POWER_HOLD);

	if (!hisi_dwc3->support_dp || !flag) {
		dwc3_phy_reset(hisi_dwc3);
		flag = 1;
	}

	dwc3_reset(hisi_dwc3);

	dwc3_misc_ctrl_put(MICS_CTRL_USB);

	set_hisi_dwc3_power_flag(USB_POWER_OFF);

	usb_dbg(":HW_USB_LDO_CTRL_USB disable\n");
	hw_usb_ldo_supply_disable(HW_USB_LDO_CTRL_USB);

	usb_dbg("-\n");

	return 0;
}

static int feb_get_dts_resource(struct hisi_dwc3_device *hisi_dwc3)
{
	struct device *dev = &hisi_dwc3->pdev->dev;

	/* get abb clk handler */
	hisi_dwc3->clk = devm_clk_get(&hisi_dwc3->pdev->dev, "clk_usb3phy_ref");
	if (IS_ERR_OR_NULL(hisi_dwc3->clk)) {
		dev_err(dev, "get usb3phy ref clk failed\n");
		return -EINVAL;
	}

	/* get a clk handler */
	hisi_dwc3->gt_aclk_usb3otg = devm_clk_get(&hisi_dwc3->pdev->dev, "aclk_usb3otg");
	if (IS_ERR_OR_NULL(hisi_dwc3->gt_aclk_usb3otg)) {
		dev_err(dev, "get aclk_usb3otg failed\n");
		return -EINVAL;
	}

	/* get h clk handler */
	hisi_dwc3->gt_hclk_usb3otg = devm_clk_get(&hisi_dwc3->pdev->dev, "hclk_usb3otg");
	if (IS_ERR_OR_NULL(hisi_dwc3->gt_hclk_usb3otg)) {
		dev_err(dev, "get hclk_usb3otg failed\n");
		return -EINVAL;
	}

	/* get tcxo clk handler */
	hisi_dwc3->gt_clk_usb3_tcxo_en = devm_clk_get(&hisi_dwc3->pdev->dev, "clk_usb3_tcxo_en");
	if (IS_ERR_OR_NULL(hisi_dwc3->gt_clk_usb3_tcxo_en)) {
		dev_err(dev, "get clk_usb3_tcxo_en failed\n");
		return -EINVAL;
	}
	/* get usb2phy ref clk handler */
	hisi_dwc3->gt_clk_usb2phy_ref = devm_clk_get(&hisi_dwc3->pdev->dev, "clk_usb2phy_ref");
	if (IS_ERR_OR_NULL(hisi_dwc3->gt_clk_usb2phy_ref)) {
		dev_err(dev, "get clk_usb2phy_ref failed\n");
		return -EINVAL;
	}


	if (of_property_read_u32(dev->of_node, "usb_aclk_freq", &USB3OTG_ACLK_FREQ)) {
		USB3OTG_ACLK_FREQ = 229000000;
	}
	dev_info(dev, "boart config set usb aclk freq:%d\n", USB3OTG_ACLK_FREQ);

	if (of_property_read_u32(dev->of_node, "usb_support_dp", &(hisi_dwc3->support_dp))) {
		dev_err(dev, "usb driver not support dp\n");
		hisi_dwc3->support_dp = 0;
	}

	if (of_property_read_u32(dev->of_node, "usb_support_check_voltage", &(hisi_dwc3->check_voltage))) {
		dev_err(dev, "usb driver not support check voltage\n");
		hisi_dwc3->check_voltage = 0;
	}

	if (of_property_read_u32(dev->of_node, "set_hi_impedance", &(hisi_dwc3->set_hi_impedance))) {
		dev_err(dev, "usb driver not support set_hi_impedance\n");
		hisi_dwc3->set_hi_impedance = 0;
	}

	init_misc_ctrl_addr(hisi_dwc3->otg_bc_reg_base);
	return 0;
}

static int feb_tcpc_is_usb_only(void)
{
	return combophy_is_usb_only();
}

static int feb_shared_phy_init(struct hisi_dwc3_device *hisi_dwc3,
		unsigned int combophy_flag)
{
	int ret;
	uint32_t temp;
	void __iomem *pericfg_base;

	usb_dbg("+\n");

	usb_dbg(":HW_USB_LDO_CTRL_HIFIUSB enable\n");
	hw_usb_ldo_supply_enable(HW_USB_LDO_CTRL_HIFIUSB);

	if (!hisi_dwc3)
		return -ENODEV;

#ifdef CONFIG_CONTEXTHUB_PD
	if (hisi_dwc3->support_dp && combophy_flag) {
		combophy_poweroff();
	}
#endif

	ret = dwc3_misc_ctrl_get(MICS_CTRL_USB);
	if (ret) {
		usb_err("usb driver not ready!\n");
		goto err_misc_ctrl_get;
	}

	pericfg_base = hisi_dwc3->pericfg_reg_base;
	udelay(100);

	/* enable 2.0 phy refclk */
	ret = clk_prepare_enable(hisi_dwc3->gt_clk_usb2phy_ref);
	if (ret) {
		usb_err("clk_prepare_enable gt_clk_usb2phy_ref failed\n");
		goto err_misc_clk_usb2phy;
	}

	/* Release USB20 PHY out of IDDQ mode */
	usb3_misc_reg_clrbit(0u, 0x14ul);

	/* usb2 phy reference clock source select soc 19.2Mhz */
	usb3_misc_reg_setbit(4, 0xa0ul);
	temp = usb3_misc_reg_readl(0x1cul);
	temp &= ~(0x3 << 3);
	temp |= (0x2 << 3);
	usb3_misc_reg_writel(temp, 0x1cul);

	/* Common Block Power-Down Control */
	usb3_misc_reg_setbit(0, 0xcul);

	/* switch phy */
	usb3_misc_reg_setbit(5, 0xa0ul);

	temp = usb3_misc_reg_readl(0xa0ul);
	temp &= ~(0x3 << 6);
	temp |= (0x2 << 6);
	usb3_misc_reg_writel(temp, 0xa0ul);

	/* release 2.0 phy */
	usb3_misc_reg_setbit(0, 0xa0ul);
	udelay(100);

	usb_dbg("-\n");
	return 0;

err_misc_clk_usb2phy:
	dwc3_misc_ctrl_put(MICS_CTRL_USB);
err_misc_ctrl_get:
#ifdef CONFIG_CONTEXTHUB_PD
	if (hisi_dwc3->support_dp && combophy_flag) {
		usb_dbg("combophy_sw_sysc +\n");
		ret = dwc3_combophy_sw_sysc(hisi_dwc3, TCPC_USB31_CONNECTED);
		if (ret)
			usb_err("combophy_sw_sysc failed(%d)\n",ret);
		usb_dbg("combophy_sw_sysc -\n");
	}
#endif
	usb_dbg(":HW_USB_LDO_CTRL_HIFIUSB disable\n");
	hw_usb_ldo_supply_disable(HW_USB_LDO_CTRL_HIFIUSB);

	return ret;
}

static int feb_shared_phy_shutdown(struct hisi_dwc3_device *hisi_dwc3,
		unsigned int combophy_flag, unsigned int keep_power)
{
	uint32_t temp;
	void __iomem *pericfg_base;
	int ret = 0;

	usb_dbg("+\n");

	if (!hisi_dwc3)
		return -ENODEV;

	usb_dbg("combophy_flag %u, keep_power %u\n", combophy_flag, keep_power);

	pericfg_base = hisi_dwc3->pericfg_reg_base;

	/* reset 2.0 phy */
	usb3_misc_reg_clrbit(0, 0xa0);

	/* switch drv vbus s*/
	temp = usb3_misc_reg_readl(0xa0ul);
	temp &= ~(0x3 << 6);
	//temp |= (0x2 << 6);
	usb3_misc_reg_writel(temp, 0xa0ul);

	/* Common Block Power-Down Control */
	usb3_misc_reg_clrbit(0, 0xcul);

	/* switch phy */
	usb3_misc_reg_clrbit(5, 0xa0ul);

	/* clock source select */
	// nothing

	/* enable siddq */
	usb3_misc_reg_setbit(0u, 0x14ul);

	/* disable 2.0 phy refclk */
	clk_disable_unprepare(hisi_dwc3->gt_clk_usb2phy_ref);

	dwc3_misc_ctrl_put(MICS_CTRL_USB);

#ifdef CONFIG_CONTEXTHUB_PD
	if (hisi_dwc3->support_dp && combophy_flag) {
		usb_dbg("combophy_sw_sysc +\n");
		ret = dwc3_combophy_sw_sysc(hisi_dwc3, TCPC_USB31_CONNECTED);
		if (ret)
			usb_err("combophy_sw_sysc failed(%d)\n",ret);
		usb_dbg("combophy_sw_sysc -\n");
	}
#endif

	usb_dbg(":HW_USB_LDO_CTRL_HIFIUSB disable\n");
	hw_usb_ldo_supply_disable(HW_USB_LDO_CTRL_HIFIUSB);
	usb_dbg("-\n");
	return ret;
}

static void feb_set_hi_impedance(struct hisi_dwc3_device *hisi_dwc)
{
	void __iomem *base = hisi_dwc->otg_bc_reg_base;

	usb_dbg("+\n");
	if (hisi_dwc->set_hi_impedance)
		writel(0, base + BC_CTRL2);
	else
		usb_dbg("this phone don't support set_hi_impedance\n");
	usb_dbg("-\n");
}

static void feb_check_voltage(struct hisi_dwc3_device *hisi_dwc)
{
	usb_dbg("+\n");

	if (hisi_dwc->check_voltage) {
		/*first dplus pulldown 15k*/
		hisi_bc_dplus_pulldown(hisi_dwc);
		/*second call charger's API to check voltage */
		water_detect();
		/*third dplus pullup*/
		hisi_bc_dplus_pullup(hisi_dwc);
	}
	usb_dbg("-\n");
}

static struct usb3_phy_ops feb_phy_ops = {
	.init		= feb_usb3phy_init,
	.shutdown	= feb_usb3phy_shutdown,
	.get_dts_resource = feb_get_dts_resource,
	.shared_phy_init	= feb_shared_phy_init,
	.shared_phy_shutdown	= feb_shared_phy_shutdown,
	.set_hi_impedance	= feb_set_hi_impedance,
	.notify_speed	= feb_notify_speed,
	.check_voltage = feb_check_voltage,
	.cptest_enable		= dwc3_compliance_mode_enable,
	.lscdtimer_set		= dwc3_hisi_lscdtimer_set,
	.tcpc_is_usb_only	= feb_tcpc_is_usb_only,
};

static int feb_usb3phy_init(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;
	static int flag = 0;
	void __iomem *pericfg_base;
	usb_dbg("+\n");

	usb_dbg(":HW_USB_LDO_CTRL_USB enable\n");
	hw_usb_ldo_supply_enable(HW_USB_LDO_CTRL_USB);

	if (!hisi_dwc3) {
		usb_err("hisi_dwc3 is NULL, dwc3-hisi have some problem!\n");
		ret = -ENOMEM;
		goto out;
	}

	pericfg_base = hisi_dwc3->pericfg_reg_base;/*lint !e529 */

	if (!hisi_dwc3_is_powerdown()) {
		usb_dbg("already release, just return!\n");
		return 0;
	}


	ret = usb3_clk_set_rate(hisi_dwc3);
	if (ret)
		goto out;

	ret = dwc3_misc_ctrl_get(MICS_CTRL_USB);
	if (ret) {
		usb_err("usb driver not ready!\n");
		goto out;
	}

	if (!hisi_dwc3->support_dp || !flag) {
		ret = dwc3_phy_release(hisi_dwc3);
		if (ret) {
			usb_err("phy release err!\n");
			goto out_misc_put;
		}
	}

	if (hisi_dwc3->support_dp && usb3_misc_ctrl_is_reset(pericfg_base)) {
		usb_err("[USBDP.DBG] goto here, need check who call.\n");
		goto out_phy_reset;
	}

	ret = dwc3_release(hisi_dwc3);
	if (ret) {
		usb_err("[RELEASE.ERR] release error, need check clk!\n");
		goto out_phy_reset;
	}

	if (combophy_is_highspeed_only()) {
		usb_dbg("set USB2OTG_ACLK_FREQ\n");
		ret = clk_set_rate(hisi_dwc3->gt_aclk_usb3otg, USB2OTG_ACLK_FREQ);
		if (ret) {
			usb_err("Can't aclk rate set\n");
		}
	} else {
		/* need reset clk freq */
		ret = usb3_clk_set_rate(hisi_dwc3);
		if (ret) {
			usb_err("usb reset clk rate fail!\n");
			goto out_phy_reset;
		}
	}

	set_usb2_eye_diagram_param(hisi_dwc3);
	set_vboost_for_usb3(hisi_dwc3->usb3_phy_tx_vboost_lvl);

	set_hisi_dwc3_power_flag(USB_POWER_ON);
	flag = 1;

	usb_dbg("-\n");
	return ret;

out_phy_reset:
	if (!hisi_dwc3->support_dp || !flag) {
		dwc3_phy_reset(hisi_dwc3);
	}
out_misc_put:
	dwc3_misc_ctrl_put(MICS_CTRL_USB);
out:
	usb_dbg(":HW_USB_LDO_CTRL_HIFIUSB disable\n");
	hw_usb_ldo_supply_disable(HW_USB_LDO_CTRL_USB);

	return ret;
}

static int dwc3_feb_probe(struct platform_device *pdev)
{
	int ret = 0;

	ret = hisi_dwc3_probe(pdev, &feb_phy_ops);
	if (ret)
		usb_err("probe failed, ret=[%d]\n", ret);

	return ret;
}

static int dwc3_feb_remove(struct platform_device *pdev)
{
	int ret = 0;

	ret = hisi_dwc3_remove(pdev);
	if (ret)
		usb_err("hisi_dwc3_remove failed, ret=[%d]\n", ret);

	return ret;
}
#ifdef CONFIG_OF
static const struct of_device_id dwc3_feb_match[] = {
	{ .compatible = "hisilicon,feb-dwc3" },
	{},
};
MODULE_DEVICE_TABLE(of, dwc3_feb_match);
#else
#define dwc3_feb_match NULL
#endif

static struct platform_driver dwc3_feb_driver = {
	.probe		= dwc3_feb_probe,
	.remove		= dwc3_feb_remove,
	.driver		= {
		.name	= "usb3-feb",
		.of_match_table = of_match_ptr(dwc3_feb_match),
		.pm	= HISI_DWC3_PM_OPS,
	},
};
/*lint +e715 +e716 +e717 +e835 +e838 +e845 +e533 +e835 +e778 +e774 +e747 +e785 +e438 +e529*/
/*lint -e64 -esym(64,*)*/
/*lint -e528 -esym(528,*)*/
module_platform_driver(dwc3_feb_driver);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_LICENSE("GPL");
/*lint -e753 +esym(753,*)*/
/*lint -e64 +esym(64,*)*/
