#include <linux/module.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <pmic_interface.h>
#include <linux/mfd/hisi_pmic.h>
#include "dwc3-hisi.h"
#include "core.h"
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/hisi/usb/hisi_usb_interface.h>
#include <linux/jiffies.h>
#include <huawei_platform/usb/hw_pd_dev.h>

#ifdef CONFIG_HUAWEI_CHARGER_AP
#include <huawei_platform/power/huawei_charger.h>
#endif

#ifdef CONFIG_CONTEXTHUB_PD
#include <linux/hisi/contexthub/tca.h>
#endif

#include "hisi_usb3_misctrl.h"
#include "hisi_usb3_31phy.h"

/*lint -e750 -esym(750,*)*/

#define SCTRL_SCDEEPSLEEPED				0x08
#define USB_INIT_CLK_SEL                (1 << 20)
#define USB_REFCLK_ISO_EN               (1 << 25)
#define SC_CLK_USB3PHY_3MUX1_SEL        (1 << 25)

#define SC_SEL_ABB_BACKUP               (1 << 8)
#define CLKDIV_MASK_START               (16)

#define PERI_CRG_CLKDIV21               0xFC
#define CLKDIV21_MSK_START              (16)
#define SC_GT_CLK_ABB_SYS		(1 << 6)
#define SEL_ABB_BACKUP			(1 << 8)

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

#define PERI_CRG_CLK_EN5                0x50
#define PERI_CRG_PERRSTEN4		0x90
#define PERI_CRG_PERRSTDIS4		0x94
#define PERI_CRG_PERRSTSTAT4		0x98

#define SC_USB3PHY_ABB_GT_EN            (1 << 15)
#define REF_SSP_EN                      (1 << 16)

#define CLK_FREQ_19_2M              19200000

#define PERI_CRG_PERSTAT0       0x0c
#define PERI_CRG_PERCLKEN4      0x48
#define PERI_CRG_PERSTAT4       0x4c

#define SC_USB20PHY_MISC_CTRL1  0x604
#define SC_USB20PHY_MISC_CTRL4  0x610

#define GT_CLK_USB3OTG_REF				(1 << 0)
#define GT_ACLK_USB3OTG					(1 << 1)
#define GT_HCLK_USB3OTG_MISC				(1 << 25)

#define IP_RST_USB3OTG_32K		(1 << 6)
#define IP_RST_USB3OTG_MISC		(1 << 7)

/* clk freq usb default usb3.0 237M usb2.0 60M */
static uint32_t g_usb3otg_aclk_freq = 237000000;
static uint32_t g_usb2otg_aclk_freq = 61000000;

extern struct hisi_dwc3_device *hisi_dwc3_dev;
static int usb3_phy_33v_enable(struct hisi_dwc3_device *hisi_dwc3);
static int usb3_phy_33v_disable(struct hisi_dwc3_device *hisi_dwc3);

#define SET_NBITS_MASK(start, end) (((2u << ((end) - (start))) - 1) << (start))
#define SET_BIT_MASK(bit) SET_NBITS_MASK(bit, bit)

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

void usb3_reset_misc_ctrl(void)
{
	void __iomem *pericfg_base;

	if (!hisi_dwc3_dev) {
		usb_err("[USB3] usb driver not setup!\n");
		return;
	}

	pericfg_base = hisi_dwc3_dev->pericfg_reg_base;/*lint !e438 */
	writel(IP_RST_USB3OTG_MISC | IP_RST_USB3OTG_32K,
			pericfg_base + PERI_CRG_PERRSTEN4);
}

void usb3_unreset_misc_ctrl(void)
{
	void __iomem *pericfg_base;

	if (!hisi_dwc3_dev) {
		usb_err("[USB3] usb driver not setup!\n");
		return;
	}

	pericfg_base = hisi_dwc3_dev->pericfg_reg_base;/*lint !e438 */
	writel(IP_RST_USB3OTG_MISC | IP_RST_USB3OTG_32K,
			pericfg_base + PERI_CRG_PERRSTDIS4);
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

static int select_lbintpll_clk(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *sctrl_base = hisi_dwc3->sctrl_reg_base;
	volatile uint32_t reg;
	int times = 200;


	writel(0x40004000, sctrl_base + 0x264);

	reg = 0x22fb1812;
	writel(reg, sctrl_base + 0xf0);

	udelay(10);

	reg = 0x22fb1813;
	writel(reg, sctrl_base + 0xf0);

	do {
		reg = (uint32_t)readl(sctrl_base + 0xfc);
		if ((reg & 3u) != 3) {
			usb_err("wait lbintpll clk fail!\n");
		} else {
			break;
		}
		udelay(10);
	} while(times--);

	if ((reg & 3u) != 3) {
		usb_err("wait lbintpll clk fail 2!\n");
		return -1;
	}

	reg = 0x22fb1811;
	writel(reg, sctrl_base + 0xf0);

	times = 200;
	do {
		reg = (uint32_t)readl(sctrl_base + 0xfc);
		if ((reg & 3u) != 3) {
			usb_err("bypass wait lbintpll clk fail!\n");
		} else {
			break;
		}
		udelay(10);
	} while(times--);

	if ((reg & 3u) != 3) {
		usb_err("bypass wait lbintpll clk fail 2!\n");
		return -1;
	}

	writel(0x40000, sctrl_base + 0x1b0);

	reg = (uint32_t)readl(sctrl_base + 0x60c);
	reg |= 0x7;
	writel(reg, sctrl_base + 0x60c);
	usb_err("[USB.DBG] set 100M clk success.!\n");

	return 0;
}

static int config_usb_phy_controller(struct hisi_dwc3_device *hisi_dwc3)
{
	volatile uint32_t temp;
	static int flag = 1;

	/* Release USB20 PHY out of IDDQ mode */
	usb3_sc_misc_reg_clrbit(0u, 0x608ul);

	if (!hisi_dwc3->support_dp || flag) {
		usb_dbg("set tca\n");
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

		usb3_misc_reg_setbit(7u, 0xb4ul);
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
	if ((scdeepsleeped & (USB_INIT_CLK_SEL)) == 0)
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
	int ret = 0;
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
/*lint -restore */
}

int usb3_clk_is_open(void __iomem *pericfg_base)
{
	volatile uint32_t hclk, aclk;
	uint32_t hclk_mask = GT_HCLK_USB3OTG_MISC;
	uint32_t aclk_mask = (GT_CLK_USB3OTG_REF | GT_ACLK_USB3OTG);

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
	volatile uint32_t stat, sc_stat;
	uint32_t bitmask = (SET_BIT_MASK(8) | SET_BIT_MASK(1));
	uint32_t sc_bitmask = SET_BIT_MASK(0);

	stat = usb3_misc_reg_readl(0xa0);
	sc_stat = usb3_sc_misc_reg_readl(0x618);

	return (  ((bitmask & stat) == bitmask)
		&&((sc_bitmask & sc_stat) == sc_bitmask));
}

int dwc3_set_combophy_clk(void)
{
	int ret;
	struct hisi_dwc3_device *hisi_dwc3 = hisi_dwc3_dev;

	ret = set_combophy_clk(hisi_dwc3);
	if(ret) {
		usb_err("[CLK.ERR] combophy clk set error!\n");
	}
	return ret;
}

static int dwc3_phy_release(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;

	if(usb3_misc_ctrl_is_unreset(pericfg_base) && usb3_clk_is_open(pericfg_base)) {
		usb_err("usb3_misc_ctrl_is_unreset && usb3_clk_is_open\n");
	} else {
		usb_dbg("release combophy\n");
		ret = dwc3_set_combophy_clk();
		if (ret)
			return ret;

		/* unreset phy */
		usb3_misc_reg_setbit(1, 0xa0);

		hisi_usb_unreset_phy_if_fpga();
		udelay(100);
	}

	return 0;
}

static void close_combophy_clk(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *sctrl_base = hisi_dwc3->sctrl_reg_base;
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;

	if (is_abb_clk_selected(sctrl_base)) {
		/* disable usb_tcxo_en */
		clk_disable_unprepare(hisi_dwc3->gt_clk_usb3_tcxo_en);
	} else {
		writel(GT_CLK_ABB_BACKUP, PERI_CRG_CLK_DIS5 + pericfg_base);
	}

	/* close phy ref clk */
	clk_disable_unprepare(hisi_dwc3->gt_clk_usb2phy_ref);
}


static void dwc3_phy_reset(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;

	usb_dbg("reset combophy\n");
	if(usb3_misc_ctrl_is_unreset(pericfg_base) && usb3_clk_is_open(pericfg_base)) {
		/* reset phy */
		usb3_misc_reg_clrbit(1, 0xa0ul); /*lint !e835 */
		usb3_sc_misc_reg_clrbit(0, 0x618ul);
		usb_dbg("reset phy\n");

		close_combophy_clk(hisi_dwc3);
	}
}

int dwc3_open_controller_clk(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;

	ret = clk_prepare_enable(hisi_dwc3->gt_aclk_usb3otg);
	if (ret) {
		usb_err("clk_prepare_enable gt_aclk_usb3otg failed\n");
		return -EACCES;
	}

	ret = clk_prepare_enable(hisi_dwc3->clk);
	if (ret) {
		usb_err("clk_prepare_enable clk failed\n");
		return -EACCES;
	}

	/* TODO: confirm with zhangfei
	 * usb2 phy reference clock source select 100M or 19.2M(default)
	 */
	//usb3_misc_reg_clrbit(4, 0xa0);

	/*
	 * usb2phy select:
	 * 0:usb31 controller owns the phy
	 * 1:usb3 controller in asp owns the phy
	 */
	usb3_sc_misc_reg_clrbit(5, 0x618);

	/* usb2_refclksel: pll ref clock select */
	usb3_sc_misc_reg_setbit(4, 0x60c);

	return ret;
}

int dwc3_compliance_mode_enable(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *usb_core_reg_base = NULL;
	volatile unsigned int reg;

	usb_err("cp_test start!\n");
	if (!hisi_dwc3 || !hisi_dwc3->usb_core_reg_base) {
		usb_err("[USB.CP0] usb driver not setup!\n");
		return -ENODEV;
	}
	usb_core_reg_base = hisi_dwc3->usb_core_reg_base;
	reg = readl(usb_core_reg_base + 0xc704);
	reg &= ~(1u << 31);
	writel(reg, usb_core_reg_base + 0xc704);
	reg = readl(usb_core_reg_base + 0xc110);
	reg &= ~(1u << 13);
	reg |= (1u << 12);
	writel(reg, usb_core_reg_base + 0xc110);
	reg = 0x2;
	writel(reg, usb_core_reg_base + 0x20);
	reg = readl(usb_core_reg_base + 0x420);
	reg &= ~(1u << 9);
	writel(reg, usb_core_reg_base + 0x420);
	reg = readl(usb_core_reg_base + 0xc2c0);
	reg |= (1u << 30);
	writel(reg, usb_core_reg_base + 0xc2c0);
	return 0;
}
int dwc3_cptest_next_pattern(void)
{
	void __iomem *usb_core_reg_base = NULL;
	volatile unsigned int reg;
	if (!hisi_dwc3_dev || !hisi_dwc3_dev->usb_core_reg_base) {
		usb_err("[USB.CP0] usb driver not setup!\n");
		return -ENODEV;
	}
	usb_core_reg_base = hisi_dwc3_dev->usb_core_reg_base;
	reg = readl(usb_core_reg_base + 0xc2c0);
	reg &= ~(1u << 30);
	writel(reg, usb_core_reg_base + 0xc2c0);
	mdelay(10);
	reg = readl(usb_core_reg_base + 0xc2c0);
	reg |= (1u << 30);
	writel(reg, usb_core_reg_base + 0xc2c0);
	return 0;
}

/* dwc3_set_highspeed_only_step1 called before controller unreset */
static int dwc3_set_highspeed_only_step1(void)
{
	/* disable usb3 SS port */
	usb3_misc_reg_setbit(1, 0x4);

/*lint -e648 -esym(648,*)*/
	usb3_misc_reg_clrvalue((u32)(~GENMASK(7, 4)), 0x7c);
/*lint -e648 +esym(648,*)*/

	udelay(100);

	return 0;
}

/* dwc3_set_highspeed_only_step2 called after controller unreset */
static int dwc3_set_highspeed_only_step2(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *usb_core_reg_base = hisi_dwc3->usb_core_reg_base;
	volatile unsigned int reg;

	if (!hisi_dwc3->usb_core_reg_base) {
		usb_err("[USB.CP0] usb driver not setup!\n");
		return -ENODEV;
	}

	/* disble pipe clock use for DP4 and must use usb2 */
	reg = readl(usb_core_reg_base + DWC3_GUCTL1);
	reg |= BIT(26);
	writel(reg, usb_core_reg_base + DWC3_GUCTL1);

	return 0;
}

int hisi_dwc3_is_fpga(void)
{
	if (!hisi_dwc3_dev) {
		usb_err("usb driver not probed!\n");
		return 0;
	}

	return !!hisi_dwc3_dev->fpga_flag;
}

static int dwc3_release(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;
	int highspeed_only;

	usb_dbg("+\n");

	ret = dwc3_open_controller_clk(hisi_dwc3);
	if (ret) {
		usb_err("[CLK.ERR] open clk error!\n");
		return ret;
	}

	if (hisi_dwc3_select_lbintpll_clk()) {
		select_lbintpll_clk(hisi_dwc3);
	} else {
		/* SCCLKDIV5(0x264)  bi[14]=0， bit[30]=1 */
		ret = clk_set_rate(hisi_dwc3->gt_clk_usb2phy_ref, CLK_FREQ_19_2M);
		if (ret) {
			usb_err("usb2phy_ref set rate failed, ret=%d\n", ret);
			return ret;
		}
	}

	/* SCPEREN4(0x1b0)  bit[18]=1 */
	ret = clk_prepare_enable(hisi_dwc3->gt_clk_usb2phy_ref);
	if (ret) {
		usb_err("clk_prepare_enable gt_clk_usb2phy_ref failed\n");
		return ret;
	}

	udelay(100);

	config_usb_phy_controller(hisi_dwc3);

	if (hisi_dwc3_is_fpga()) {
		/* FPGA force utmi clk to 60M */
		usb3_sc_misc_reg_clrvalue(~SET_NBITS_MASK(8,9), 0x600);
	} else {
		/* force utmi clk to 30M */
		usb3_sc_misc_reg_setvalue(SET_NBITS_MASK(8,9), 0x600);
	}

	udelay(100);

	/* release usb2.0 phy */
	usb3_sc_misc_reg_setbit(0, 0x618);

	highspeed_only = dwc3_is_highspeed_only();
	if (highspeed_only) {
		usb_dbg("[USB.DP] DP4 mode, set usb2.0 only, setp 1\n");
		ret = dwc3_set_highspeed_only_step1();
		if (ret) {
			clk_disable_unprepare(hisi_dwc3->gt_clk_usb2phy_ref);
			return ret;
		}
	}

	if (hisi_dwc3_get_dt_host_maxspeed() < USB_SPEED_SUPER_PLUS) {
		usb_dbg("[USB.LINK] usb host super-speed only!\n");
		usb3_misc_reg_setbit(30, 0x7c);
	}
	/* unreset controller */
	usb3_misc_reg_setbit(8, 0xa0);
	udelay(100);

	if (highspeed_only) {
		usb_dbg("[USB.DP] DP4 mode, set usb2.0 only, step 2\n");
		ret = dwc3_set_highspeed_only_step2(hisi_dwc3);
		if (ret) {
			clk_disable_unprepare(hisi_dwc3->gt_clk_usb2phy_ref);
			return ret;
		}
	}

	/* set vbus valid */
	usb3_misc_reg_setvalue(SET_NBITS_MASK(6, 7), 0x0);
	usb3_sc_misc_reg_setvalue(SET_NBITS_MASK(5, 6), 0x600);

	/* require to delay 10ms */
	mdelay(10);
	usb_dbg("-\n");

	return ret;
}

static void mar_notify_speed(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;

	usb_dbg("+device speed is %d\n", hisi_dwc3->speed);

#ifdef CONFIG_TCPC_CLASS
	if ((hisi_dwc3->speed != USB_CONNECT_HOST) && (hisi_dwc3->speed != USB_CONNECT_DCP))
		hw_usb_set_usb_speed(hisi_dwc3->speed);
#endif

	if (((hisi_dwc3->speed < USB_SPEED_WIRELESS) && (hisi_dwc3->speed > USB_SPEED_UNKNOWN))
		||(hisi_dwc3->speed == USB_CONNECT_DCP)) {
		usb_dbg("set USB2OTG_ACLK_FREQ\n");
		ret = clk_set_rate(hisi_dwc3->gt_aclk_usb3otg, g_usb2otg_aclk_freq);
		if (ret) {
			usb_err("Can't aclk rate set\n");
		}
	} else if ((hisi_dwc3->speed == USB_CONNECT_HOST) && (!hisi_dwc3_is_powerdown())) {
		usb_dbg("set USB3OTG_ACLK_FREQ\n");
		ret = clk_set_rate(hisi_dwc3->gt_aclk_usb3otg, g_usb3otg_aclk_freq);
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
		usb3_misc_reg_clrbit(8, 0xa0ul);
		usb_err("reset controller\n");

		/* reset usb2.0 phy */
		usb3_sc_misc_reg_clrbit(0, 0x618);
	}

	clk_disable_unprepare(hisi_dwc3->clk);
	clk_disable_unprepare(hisi_dwc3->gt_aclk_usb3otg);

	usb_dbg("-\n");
}

static int usb3_clk_set_rate(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;

	/* set usb aclk 228MHz to improve performance */
	usb_dbg("set aclk rate [%d]\n", g_usb3otg_aclk_freq);
	ret = clk_set_rate(hisi_dwc3->gt_aclk_usb3otg, g_usb3otg_aclk_freq);
	if (ret)
		usb_err("Can't aclk rate set, current rate is %ld:\n",
				clk_get_rate(hisi_dwc3->gt_aclk_usb3otg));

	return ret;
}

void set_usb2_eye_diagram_param(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *sctrl = hisi_dwc3->sctrl_reg_base;

	if (hisi_dwc3->fpga_flag != 0)
		return;

	/* set high speed phy parameter */
	if (hisi_dwc3->host_flag) {
		writel(hisi_dwc3->eye_diagram_host_param, sctrl + SC_USB20PHY_MISC_CTRL1);
		usb_dbg("set hs phy param 0x%x for host\n",
				readl(sctrl + SC_USB20PHY_MISC_CTRL1));
	} else {
		writel(hisi_dwc3->eye_diagram_param, sctrl + SC_USB20PHY_MISC_CTRL1);
		usb_dbg("set hs phy param 0x%x for device\n",
				readl(sctrl + SC_USB20PHY_MISC_CTRL1));
	}
}


#define TX_VBOOST_LVL_REG (0x21)
#define TX_VBOOST_LVL_VALUE_START (4)
#define TX_VBOOST_LVL_VALUE_END (6)
#define TX_VBOOST_LVL_ENABLE SET_BIT_MASK(7)


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
static int mar_usb3phy_init(struct hisi_dwc3_device *hisi_dwc3);

static int mar_usb31_core_enable_u3(struct hisi_dwc3_device *hisi_dwc3)
{
	volatile u32 temp;
	int ret;
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;/*lint !e529 */

	usb_dbg("+\n");

	/*
	 * check if misc ctrl is release
	 * check if usb clk is open
	 */
	if (usb3_misc_ctrl_is_reset(pericfg_base) || usb3_hclk_is_close(pericfg_base)) {
		usb_err("misc ctrl or usb3 clk not ready.\n");
		return -ENOENT;
	}

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
			return -ENOENT;
		}

		/* Release USB20 PHY out of IDDQ mode */
		usb3_sc_misc_reg_clrbit(0u, 0x608ul);

		/* release usb2.0 phy */
		usb3_sc_misc_reg_setbit(0, 0x618);

		udelay(100);

		/* unreset controller */
		usb3_misc_reg_setbit(8, 0xa0);
		udelay(100);
	}

	/* Set REFCLKPER */
	/* DP4 + usb2.0 need */
	temp = readl(hisi_dwc3->usb_core_reg_base + 0xc12c);
	temp |= (1ul << 26);
	writel(temp, hisi_dwc3->usb_core_reg_base + 0xc12c);

	/* enable U3 */
	temp = readl(hisi_dwc3->usb_core_reg_base + 0xc2c0);
	temp |= (1ul << 17);
	writel(temp, hisi_dwc3->usb_core_reg_base + 0xc2c0);

	usb_dbg("-\n");
	return 0;
}

void hisi_dwc3_usbcore_write(u32 offset, u32 value)
{
	void __iomem *pericfg_base;

	if (!hisi_dwc3_dev) {
		usb_err("usb driver have not probed!\n");
		return ;
	}

    pericfg_base = hisi_dwc3_dev->pericfg_reg_base;

	if(usb3_misc_ctrl_is_unreset(pericfg_base) && usb3_clk_is_open(pericfg_base)) {
		if(usb3_phy_controller_is_release()) {
			usb_err("set offset:%x.\n", offset);
			writel(value, hisi_dwc3_dev->usb_core_reg_base + offset);
		} else {
			usb_err(" usb controller is reset, please release first.\n");
		}
	} else {
		usb_err("misc ctrl is reset, please release it first.\n");
	}
}

volatile unsigned int hisi_dwc3_usbcore_read(u32 offset)
{
	void __iomem *pericfg_base;

	if (!hisi_dwc3_dev) {
		usb_err("usb driver have not probed!\n");
		return 0;
	}

    pericfg_base = hisi_dwc3_dev->pericfg_reg_base;
	if(usb3_misc_ctrl_is_unreset(pericfg_base) && usb3_clk_is_open(pericfg_base)) {
		if(usb3_phy_controller_is_release()) {
			usb_err("read offset:%x.\n", offset);
			return readl(hisi_dwc3_dev->usb_core_reg_base + offset);
		} else {
			usb_err(" usb controller is reset, please release first.\n");
		}
	}else {
		usb_err("misc ctrl is reset, please release it first.\n");
	}

	return 0;
}

static int mar_usb31_core_disable_u3(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;/*lint !e529 */
	usb_dbg("+\n");

	//TODO: need modify for mar
	if (usb3_misc_ctrl_is_reset(pericfg_base) || usb3_hclk_is_close(pericfg_base)) {
		usb_err("usb core is busy! please disconnect usb first!\n");
		return -EBUSY;
	}

	if (hisi_dwc3_is_powerdown()) {
		/* need reset controller */
		usb3_misc_reg_clrbit(8, 0xa0ul);

		/* reset usb2.0 phy */
		usb3_sc_misc_reg_clrbit(0, 0x618);

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
		mar_usb31_core_enable_u3(hisi_dwc3);
	} else if (PHY_MODE_CHANGE_END == type) {
		mar_usb31_core_disable_u3(hisi_dwc3);
	} else {
		usb_err("Bad param!\n");
		ret = 1;
	}
	usb_dbg("-\n");
	return ret;
}

int mar_usb31_controller_dump(void)
{
	struct hisi_dwc3_device *hisi_dwc3 = hisi_dwc3_dev;

	usb_dbg("+\n");

	if (!hisi_dwc3) {
		usb_err("hisi_dwc3 is NULL, dwc3-hisi have some problem!\n");
		return -ENOMEM;
	}

	usb_err("[c2c0:0x%x], [c118:0x%x], [d050:0x%x], [misc 84:0x%x], [misc 28:0x%x]\n",
		readl(hisi_dwc3->usb_core_reg_base + 0xc2c0),
		readl(hisi_dwc3->usb_core_reg_base + 0xc118),
		readl(hisi_dwc3->usb_core_reg_base + 0xd050),
		usb3_misc_reg_readl(0x84),
		usb3_misc_reg_readl(0x28));

	usb_dbg("-\n");

	return 0;
}

static int mar_usb3phy_shutdown(struct hisi_dwc3_device *hisi_dwc3)
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

	usb_dbg(":DP_AUX_LDO_CTRL_USB disable\n");
	usb3_phy_33v_disable(hisi_dwc3);

	set_hisi_dwc3_power_flag(USB_POWER_OFF);
	usb_dbg("-\n");

	return 0;
}

static int usb3_phy_33v_enable(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret = 0;
	struct regulator *ldo_supply = hisi_dwc3->usb20phy_power;

	usb_dbg("+\n");
	if (ldo_supply == NULL) {
		usb_err("usb3.3v ldo enable!\n");
		return -ENODEV;
	}

	if (hisi_dwc3->usb20phy_power_flag == 1)
		return 0;

	ret = regulator_enable(ldo_supply);
	if (ret) {
		usb_err("regulator enable failed(%d)!\n", ret);
		return -EPERM;
	}

	hisi_dwc3->usb20phy_power_flag = 1;
	usb_dbg("regulator enable success!\n");
	return 0;
}

static int usb3_phy_33v_disable(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret = 0;
	struct regulator *ldo_supply = hisi_dwc3->usb20phy_power;

	usb_dbg("+\n");
	if (ldo_supply == NULL) {
		usb_err("usb3.3v ldo disable!\n");
		return -ENODEV;
	}

	if (hisi_dwc3->usb20phy_power_flag == 0) {
		WARN_ON(1);
		return 0;
	}

	ret = regulator_disable(ldo_supply);
	if (ret) {
		usb_err("regulator disable failed(%d)!\n", ret);
		return -EPERM;
	}

	hisi_dwc3->usb20phy_power_flag = 0;
	usb_dbg("regulator disable success!\n");
	return 0;

}

static struct regulator *usb3_phy_ldo_33v_dts(struct device *dev)
{
	struct regulator *supply = NULL;
	int cur_uv = 0;

	supply = regulator_get(dev, "usb_phy_ldo_33v");
	if (IS_ERR(supply)) {
		usb_err("get usb3 phy ldo 3.3v failed!\n");
		return NULL;
	}

	cur_uv = regulator_get_voltage(supply);

	usb_dbg("phy voltage:[%d]", cur_uv);
	return supply;
}

static int mar_get_dts_resource(struct hisi_dwc3_device *hisi_dwc3)
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

	if (of_property_read_u32(dev->of_node, "usb_aclk_freq", &g_usb3otg_aclk_freq)) {
		g_usb3otg_aclk_freq = 229000000;
	}
	dev_info(dev, "boart config set usb aclk freq:%d\n", g_usb3otg_aclk_freq);

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

	if (of_property_read_u32(dev->of_node, "usb_firmware_es", &(hisi_dwc3->es_firmware))) {
		dev_err(dev, "[USB.DBG] cs platform\n");
		hisi_dwc3->es_firmware = 0;
	}

	hisi_dwc3->usb20phy_power = usb3_phy_ldo_33v_dts(dev);
	hisi_dwc3->usb20phy_power_flag = 0;

	hisi_dwc3->bc_ctrl_reg = hisi_dwc3->sctrl_reg_base + SC_USB20PHY_MISC_CTRL4;
	init_misc_ctrl_addr(hisi_dwc3->otg_bc_reg_base);
	init_sc_misc_ctrl_addr(hisi_dwc3->sctrl_reg_base);
	return 0;
}

static int mar_shared_phy_init(struct hisi_dwc3_device *hisi_dwc3, unsigned int combophy_flag)
{
	int ret;
	uint32_t temp;
	void __iomem *pericfg_base;

	usb_dbg("+\n");

#ifdef CONFIG_CONTEXTHUB_PD
	if (hisi_dwc3->support_dp && combophy_flag) {
		combophy_poweroff();
	}
#endif

	usb_dbg(":DP_AUX_LDO_CTRL_HIFIUSB enable\n");
	usb3_phy_33v_enable(hisi_dwc3);

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

	/* hifi use 16bit-30M utmi */
	usb3_sc_misc_reg_setvalue(SET_NBITS_MASK(8,9), 0x600);

	/* Release USB20 PHY out of IDDQ mode */
	usb3_sc_misc_reg_clrbit(0u, 0x608ul);

	/* Usb20phy port control signal usb2_refclksel select */
	temp = usb3_sc_misc_reg_readl(0x60cul);
	temp &= ~(0x3 << 3);
	temp |= (0x2 << 3);
	usb3_sc_misc_reg_writel(temp, 0x60cul);

	/* select usb20phy for audio */
	usb3_sc_misc_reg_setbit(5, 0x618ul);
	hisi_usb_switch_sharedphy_if_fpga(1);

	/* select usb 20phy drv vbus for audio */
	temp = usb3_sc_misc_reg_readl(0x618ul);
	temp &= ~(0x3 << 6);
	temp |= (0x2 << 6);
	usb3_sc_misc_reg_writel(temp, 0x618ul);

	/* release 2.0 phy */
	usb3_sc_misc_reg_setbit(0, 0x618ul);
	hisi_usb_unreset_phy_if_fpga();
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
	usb_dbg(":DP_AUX_LDO_CTRL_HIFIUSB disable\n");
	usb3_phy_33v_disable(hisi_dwc3);

	return ret;
}

static int mar_shared_phy_shutdown(struct hisi_dwc3_device *hisi_dwc3,
		unsigned int combophy_flag, unsigned int keep_power)
{
	uint32_t temp;
	int ret = 0;

	usb_dbg("+\n");

	if (!hisi_dwc3)
		return -ENODEV;

	usb_dbg("combophy_flag %u, keep_power %u\n", combophy_flag, keep_power);

	/* reset 2.0 phy */
	usb3_sc_misc_reg_clrbit(0, 0x618);

	/* switch drv vbus */
	temp = usb3_sc_misc_reg_readl(0x618ul);
	temp &= ~(0x3 << 6);
	usb3_sc_misc_reg_writel(temp, 0x618ul);

	/* switch phy */
	usb3_sc_misc_reg_clrbit(5, 0x618ul);
	hisi_usb_switch_sharedphy_if_fpga(0);

	/* clock source select */
	// nothing

	/* enable siddq */
	usb3_sc_misc_reg_setbit(0u, 0x608ul);

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

	if (!keep_power) {
		usb_dbg(":DP_AUX_LDO_CTRL_HIFIUSB disable\n");
		usb3_phy_33v_disable(hisi_dwc3);
	}

	usb_dbg("-\n");
	return ret;
}

static void mar_set_hi_impedance(struct hisi_dwc3_device *hisi_dwc)
{
	void __iomem *bc_ctrl2 = hisi_dwc->bc_ctrl_reg;

	usb_dbg("+\n");
	if (hisi_dwc->set_hi_impedance)
		writel(0, bc_ctrl2);
	else
		usb_dbg("this phone don't support set_hi_impedance\n");
	usb_dbg("-\n");
}

static void mar_check_voltage(struct hisi_dwc3_device *hisi_dwc)
{
	usb_dbg("+\n");

	if (hisi_dwc->check_voltage) {
		/*first dplus pulldown 15k*/
		hisi_bc_dplus_pulldown(hisi_dwc);
#ifdef CONFIG_HUAWEI_CHARGER_AP
		/*second call charger's API to check voltage */
		water_detect();
#endif
		/*third dplus pullup*/
		hisi_bc_dplus_pullup(hisi_dwc);
	}

	usb_dbg("-\n");
}

static struct usb3_phy_ops mar_phy_ops = {
	.init		= mar_usb3phy_init,
	.shutdown	= mar_usb3phy_shutdown,
	.get_dts_resource = mar_get_dts_resource,
	.shared_phy_init	= mar_shared_phy_init,
	.shared_phy_shutdown	= mar_shared_phy_shutdown,
	.set_hi_impedance	= mar_set_hi_impedance,
	.notify_speed	= mar_notify_speed,
	.check_voltage = mar_check_voltage,
	.cptest_enable		= dwc3_compliance_mode_enable,
};

static void config_usbphy_power(void __iomem *sctrl_base)
{
	volatile uint32_t scdeepsleeped;

	scdeepsleeped = (uint32_t)readl(SCTRL_SCDEEPSLEEPED + sctrl_base);
	if ((scdeepsleeped & (1 << 7)) != 0)
		usb3_sc_misc_reg_setbit(12, 0x60c);
}

static int mar_usb3phy_init(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;
	static int flag = 0;
	void __iomem *pericfg_base;
	void __iomem *sctrl_base;

	usb_dbg("+\n");

	pericfg_base = hisi_dwc3->pericfg_reg_base;/*lint !e529 */
	sctrl_base = hisi_dwc3->sctrl_reg_base;

	if (!hisi_dwc3_is_powerdown()) {
		usb_dbg("already release, just return!\n");
		return 0;
	}

	usb_dbg(":DP_AUX_LDO_CTRL_USB enable\n");
	usb3_phy_33v_enable(hisi_dwc3);

	config_usbphy_power(sctrl_base);

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

	if (dwc3_is_highspeed_only()) {
		usb_dbg("set USB2OTG_ACLK_FREQ\n");
		ret = clk_set_rate(hisi_dwc3->gt_aclk_usb3otg, g_usb2otg_aclk_freq);
		if (ret)
			usb_err("Can't aclk rate set\n");
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
	usb_dbg(":DP_AUX_LDO_CTRL_USB disable\n");
	usb3_phy_33v_disable(hisi_dwc3);
	return ret;
}

static int dwc3_mar_probe(struct platform_device *pdev)
{
	int ret;

	ret = hisi_dwc3_probe(pdev, &mar_phy_ops);
	if (ret)
		usb_err("probe failed, ret=[%d]\n", ret);

	return ret;
}

static int dwc3_mar_remove(struct platform_device *pdev)
{
	int ret;

	ret = hisi_dwc3_remove(pdev);
	if (ret)
		usb_err("hisi_dwc3_remove failed, ret=[%d]\n", ret);

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

#include "dwc3-reg-dump.c"

static struct platform_driver dwc3_mar_driver = {
	.probe		= dwc3_mar_probe,
	.remove		= dwc3_mar_remove,
	.driver		= {
		.name	= "usb3-mar",
		.of_match_table = of_match_ptr(dwc3_mar_match),
		.pm	= HISI_DWC3_PM_OPS,
	},
};
/*lint +e715 +e716 +e717 +e835 +e838 +e845 +e533 +e835 +e778 +e774 +e747 +e785 +e438 +e529*/
/*lint -e64 -esym(64,*)*/
/*lint -e528 -esym(528,*)*/
module_platform_driver(dwc3_mar_driver);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_LICENSE("GPL");
/*lint -e753 +esym(753,*)*/
/*lint -e64 +esym(64,*)*/
