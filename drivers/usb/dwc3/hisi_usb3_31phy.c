#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/bitops.h>

#include "dwc3-hisi.h"
#include "hisi_usb3_misctrl.h"
#include "hisi_usb3_31phy.h"

#define USB31PHY_CR_ACK			BIT(7)
#define USB31PHY_CR_WR_EN		BIT(5)
#define USB31PHY_CR_RD_EN		BIT(3)
#define USB31PHY_CR_SEL			BIT(4)
#define USB31PHY_CR_CLK			BIT(2)

#define USB31PHY_CR_ADDR_MASK		GENMASK(31, 16)
#define USB31PHY_CR_DATA_RD_START	16

#define USB_MISC_REG_PHY_CR_PARA_CTRL	0x54
#define USB_MISC_REG_PHY_CR_PARA_DATA	0x58
#define USB_TCA_REG_TCA_GCFG		(0x200 + 0x10)
#define USB_TCA_REG_TCA_TCPC		(0x200 + 0x14)

extern struct hisi_dwc3_device *hisi_dwc3_dev;
extern bool __clk_is_enabled(struct clk *clk);

static unsigned int misc_usecount = 0;
static DEFINE_MUTEX(misc_ctrl_mutex);

static char *misc_ctrl_status_string(unsigned int misc_ctrl_status)
{
	char *s = NULL;
	if (misc_ctrl_status == (1 << MICS_CTRL_USB))
		s = "MICS_CTRL_USB";
	else if (misc_ctrl_status == (1 << MICS_CTRL_COMBOPHY))
		s = "MICS_CTRL_COMBOPHY";
	else if (misc_ctrl_status == ((1 << MICS_CTRL_USB) | (1 << MICS_CTRL_COMBOPHY)))
		s = "MICS_CTRL_USB_COMBOPHY";
	else if (misc_ctrl_status == 0)
		s = "MICS_CTRL_NONE";
	else
		s = "ilegal misc_ctrl_status";
	return s;
}

static char *misc_ctrl_type_string(enum misc_ctrl_type type)
{
	char *s = NULL;
	if (type == MICS_CTRL_USB)
		s = "MICS_CTRL_USB";
	else if (type == MICS_CTRL_COMBOPHY)
		s = "MICS_CTRL_COMBOPHY";
	else
		s = "ilegal misc_ctrl_type";
	return s;
}

int dwc3_misc_ctrl_get(enum misc_ctrl_type type)
{
	int ret;

	pr_debug("+ misc_usecount [%s] type[%s] \n",
			misc_ctrl_status_string(misc_usecount),
			misc_ctrl_type_string(type));

	if (!hisi_dwc3_dev) {
		pr_err("[USBERR] usb driver not probe\n");
		return -ENODEV;
	}

	if ((type != MICS_CTRL_USB) && (type != MICS_CTRL_COMBOPHY)) {
		pr_err("[USBERR] type[%d] is not correct\n", type);
		return -EINVAL;
	}

	mutex_lock(&misc_ctrl_mutex);
	if (misc_usecount == 0) {
		pr_debug("it will be going to  release miscctrl\n");
		/* open hclk gate */
		ret = clk_prepare_enable(hisi_dwc3_dev->gt_hclk_usb3otg);
		if (ret) {
			mutex_unlock(&misc_ctrl_mutex);
			usb_err("clk_enable gt_hclk_usb3otg failed\n");
			return -EINVAL;
		}

		if (__clk_is_enabled(hisi_dwc3_dev->gt_hclk_usb3otg) == false) {
			mutex_unlock(&misc_ctrl_mutex);
			usb_err("gt_hclk_usb3otg  enable err\n");
			return -EPERM;
		}

		/* make sure Misc-ctrl at reset status */
		usb3_reset_misc_ctrl();
		udelay(100);

		/* dis-reset usb misc ctrl module */
		usb3_unreset_misc_ctrl();

		misc_usecount = 1 << type;
	} else {
		pr_debug("%s has got, just return!\n",
				misc_ctrl_status_string(misc_usecount));
		misc_usecount =  misc_usecount |(1 << type);
	}
	mutex_unlock(&misc_ctrl_mutex);
	pr_debug("-misc_usecount[%s]\n",misc_ctrl_status_string(misc_usecount));
	return 0;
}

void dwc3_misc_ctrl_put(enum misc_ctrl_type type)
{
	pr_debug("+ misc_usecount [%s] type[%s] \n",
			misc_ctrl_status_string(misc_usecount),
			misc_ctrl_type_string(type));

	if (!hisi_dwc3_dev) {
		pr_err("[USB.DBG] usb driver not probe\n");
		return;
	}

	if ((type != MICS_CTRL_USB) && (type != MICS_CTRL_COMBOPHY)) {
		pr_err("[USB.DBG] type[%d] is not correct\n", type);
		return;
	}

	mutex_lock(&misc_ctrl_mutex);
	misc_usecount = misc_usecount & (~(1 << type));/*lint !e502 */
	if (misc_usecount == 0) {
		pr_debug("it will be going to reset miscctrl\n");
		/* reset usb misc ctrl module */
		usb3_reset_misc_ctrl();

		/* disable usb3otg hclk */
		clk_disable_unprepare(hisi_dwc3_dev->gt_hclk_usb3otg);
	} else {
		pr_debug("%s has got, just return!\n",
				misc_ctrl_status_string(misc_usecount));
	}
	mutex_unlock(&misc_ctrl_mutex);
	pr_debug("-misc_usecount[%s]\n",misc_ctrl_status_string(misc_usecount));
}

static int usb31phy_cr_ack_wait_n = 100000;

#ifdef DEBUG_USB31PHY
void usb31phy_cr_dbg_setacktime(int n)
{
	usb31phy_cr_ack_wait_n = n;
}
#endif

static void usb31phy_cr_set_addr(u32 addr)
{
	volatile u32 tmp;
	tmp = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_CTRL);
	tmp &= ~USB31PHY_CR_ADDR_MASK; /*lint !e648 */
	tmp |= ((addr << 16) & ~GENMASK(15, 0)); /*lint !e647 */
	usb3_misc_reg_writel(tmp, USB_MISC_REG_PHY_CR_PARA_CTRL);
}

static void usb31phy_cr_clk(void)
{
	volatile u32 tmp;

	/* clk up */
	tmp = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_CTRL);
	tmp |= USB31PHY_CR_CLK;
	usb3_misc_reg_writel(tmp, USB_MISC_REG_PHY_CR_PARA_CTRL);

	/* clk down */
	tmp = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_CTRL);
	tmp &= ~USB31PHY_CR_CLK;
	usb3_misc_reg_writel(tmp, USB_MISC_REG_PHY_CR_PARA_CTRL);
}

static void usb31phy_cr_clk_toogle(unsigned count)
{
	unsigned i = 0;

	for(i = 0; i < count; i++) {
		usb31phy_cr_clk();
	}
}

static void usb31phy_cr_100clk(void)
{
	usb31phy_cr_clk_toogle(100);
}

static int usb31phy_cr_wait_ack(void)
{
	int32_t i = usb31phy_cr_ack_wait_n;
	int ret = 0;

	while (!(usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_CTRL) & USB31PHY_CR_ACK)) {
		usb31phy_cr_clk();

		if(0 == i) {
			pr_err("wait phy cr timeout! %lu, %u\n",
					USB31PHY_CR_ACK,
					usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_CTRL));
			ret = 1;
			break;
		}
		i--;
	}

	return ret;
}

enum phy_cr_start_type {
	PHY_CR_READ,
	PHY_CR_WRITE
};

static void usb31phy_cr_start(enum phy_cr_start_type type)
{
	volatile u32 high, low;

	high = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_CTRL);

	if (PHY_CR_READ == type) {
		high |= USB31PHY_CR_RD_EN;
	} else if(PHY_CR_WRITE == type) {
		high |= USB31PHY_CR_WR_EN;
	} else {
		pr_err("unknow phy cr type:%d\n", type);
		return;
	}

	usb3_misc_reg_writel(high, USB_MISC_REG_PHY_CR_PARA_CTRL);

	usb31phy_cr_clk();

	/* read & write */
	low = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_CTRL);
	low &= ~USB31PHY_CR_RD_EN;
	low &= ~USB31PHY_CR_WR_EN;
	usb3_misc_reg_writel(low, USB_MISC_REG_PHY_CR_PARA_CTRL);
}

static void usb31phy_cr_set_sel(void)
{
	volatile u32 temp;

	temp = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_CTRL);

	temp |= USB31PHY_CR_SEL;

	usb3_misc_reg_writel(temp, USB_MISC_REG_PHY_CR_PARA_CTRL);
}

u16 usb31phy_cr_read(u32 addr)
{
	volatile u32 data;

	usb31phy_cr_100clk();

	/* step-0: set cr para selk */
	usb31phy_cr_set_sel();

	/* step-1: set addr for read */
	usb31phy_cr_set_addr(addr);

	/* step-2: send read start cmd */
	usb31phy_cr_start(PHY_CR_READ);

	/* step-3: wait cmd ack */
	if (usb31phy_cr_wait_ack()) {
		pr_err("wait ack timeout!\n");
	}

	/* ack, read data now*/
	data = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_DATA);
	return ((data >> USB31PHY_CR_DATA_RD_START) & GENMASK(15, 0));
}

int usb31phy_cr_write(u32 addr, u16 value)
{
	int ret = 0;

	usb31phy_cr_100clk();

	/* step-0: set cr para selk */
	usb31phy_cr_set_sel();

	/* step-1: set addr for write */
	usb31phy_cr_set_addr(addr);

	/* step-2: set data for write */
	usb3_misc_reg_writel(value, USB_MISC_REG_PHY_CR_PARA_DATA);

	/* step-3: send write start cmd */
	usb31phy_cr_start(PHY_CR_WRITE);

	/* wait cmd ack if write success */
	if (usb31phy_cr_wait_ack()) {
		ret = 1;
	}

	return ret;
}

void usb31phy_cr_32clk(void)
{
	usb31phy_cr_clk_toogle(32);
}

/* defined USB31_TCA data sheet */
enum combophy_mode {
	NOT_CONNECT = 0,
	USBONLY_MODE = 1,
	DPONLY_MODE = 2,
	USB_DP_MODE = 3,
	BAD_MODE
};

int dwc3_is_highspeed_only(void)
{
	volatile uint32_t temp;
	temp = usb3_misc_reg_readl(USB_TCA_REG_TCA_TCPC);
	/* Mux control from TCPM controlling the behavior of the ComboPHY DPAlt_Xbar
	 * and TCA synchronization.
	 * 00: No connection (default)
	 * 01: USB3.1 Connected
	 * 10: DP Alternate Mode - 4 lanes
	 * 11: USB3.1 + Display Port Lanes 0 & 1
	 */
	return (temp & COMBOPHY_MODE_MASK) == DPONLY_MODE;
}

int dwc3_is_usb_only(void)
{
	volatile uint32_t temp;
	temp = usb3_misc_reg_readl(USB_TCA_REG_TCA_TCPC);
	/* Mux control from TCPM controlling the behavior of the ComboPHY DPAlt_Xbar
	 * and TCA synchronization.
	 * 00: No connection (default)
	 * 01: USB3.1 Connected
	 * 10: DP Alternate Mode - 4 lanes
	 * 11: USB3.1 + Display Port Lanes 0 & 1
	 */
	return (temp & COMBOPHY_MODE_MASK) == USBONLY_MODE;
}
