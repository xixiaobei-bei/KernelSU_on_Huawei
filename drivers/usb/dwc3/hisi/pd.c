#include <soc_pctrl_interface.h>
#include <soc_usb31_misc_ctrl_interface.h>
#include <soc_usb31_tca_interface.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/workqueue.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <linux/of_address.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/hisi/usb/hisi_usb_interface.h>
#include <linux/hisi/contexthub/tca.h>
#include <linux/version.h>
#include <huawei_platform/dp_source/dp_dsm.h>
#include "common.h"
#include "hisi_usb3_misctrl.h"
#include "hisi_usb3_31phy.h"
#include "dwc3-hisi.h"

#if IS_ENABLED(CONFIG_HISI_FB_970) || IS_ENABLED(CONFIG_HISI_FB_V501)|| IS_ENABLED(CONFIG_HISI_FB_V510)
extern int hisi_dptx_triger(bool benable);
extern int hisi_dptx_hpd_trigger(TCA_IRQ_TYPE_E irq_type, TCPC_MUX_CTRL_TYPE mode, TYPEC_PLUG_ORIEN_E typec_orien);
extern int hisi_dptx_notify_switch(void);
#else
static inline int hisi_dptx_triger(bool benable) {return 0;}
static inline int hisi_dptx_hpd_trigger(TCA_IRQ_TYPE_E irq_type, TCPC_MUX_CTRL_TYPE mode, TYPEC_PLUG_ORIEN_E typec_orien) {return 0;}
static inline int hisi_dptx_notify_switch(void) {return 0;}
#endif
extern int hisi_usb_otg_event_sync(TCPC_MUX_CTRL_TYPE mode_type, enum otg_dev_event_type event, TYPEC_PLUG_ORIEN_E typec_orien);
void usb3_misc_reg_setbit(u32 bit, unsigned long int offset);
extern bool __clk_is_enabled(struct clk *clk);
extern void dp_dfp_u_notify_dp_configuration_done(TCPC_MUX_CTRL_TYPE mode_type, int ack);
static int tca_mode_switch(TCPC_MUX_CTRL_TYPE new_mode, TYPEC_PLUG_ORIEN_E typec_orien);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
__weak void dp_imonitor_set_pd_event(uint8_t irq_type, uint8_t cur_mode, uint8_t mode_type, uint8_t dev_type, uint8_t typec_orien) { return ; }
#endif
#define HISI_TCA_DEBUG              KERN_INFO
#define PD_PLATFORM_INIT_OK (0X56781234)
#define FIFO_SIZE (128)

#define PCTRL_PERI_CTRL24_CLK_USB3PHY_MUX_SEL (25)

#define COMBOPHY_MODE_MASK              0x3

/* defined USB31_TCA data sheet */
enum combophy_mode {
	NOT_CONNECT = 0,
	USBONLY_MODE = 1,
	DPONLY_MODE = 2,
	USB_DP_MODE = 3,
	BAD_MODE
};

/*lint -e528 -esym(528,*) */
typedef struct pd_event_member_s {
	TCA_IRQ_TYPE_E irq_type;
	TCPC_MUX_CTRL_TYPE mode_type;
	TCA_DEV_TYPE_E dev_type;
	TYPEC_PLUG_ORIEN_E typec_orien;
}pd_event_t;

typedef enum {
	TCA_POWEROFF = 0,
	TCA_POWERON = 1,
	TCA_POWER_REBOOT = 2,
	TCA_POWER_MAX
}TCA_POWER_TYPE;

static int tca_mode_sw(TCPC_MUX_CTRL_TYPE new_mode, TYPEC_PLUG_ORIEN_E typec_orien);

struct tca_device_s {
	int init;
	TCA_POWER_TYPE tca_poweron;
	volatile TCPC_MUX_CTRL_TYPE tca_cur_mode;
	TYPEC_PLUG_ORIEN_E typec_orien;
	volatile int sw_cnt;
	TCA_DEV_TYPE_E usbctrl_status;
	struct wake_lock wlock;
	struct work_struct work;
	struct workqueue_struct *wq;
	struct kfifo_rec_ptr_1 kfifo;
	struct clk *gt_clk_usb3_tcxo_en;
	void __iomem *pctrl_reg_base;
	void __iomem *usb_misc_base;
	void __iomem *tca_base;
	struct hisi_usb_combophy *combophy;
#ifdef COMBOPHY_FW_UPDATE
	unsigned int update_combophy_firmware;
	unsigned int combophy_resistor;
#endif
};

static DEFINE_MUTEX(tca_mutex);
static struct tca_device_s tca_dev;

/*
 * return 1 if DPONLY_MODE
 * default is 0
 */
int combophy_is_highspeed_only(void)
{
	volatile uint32_t temp;

	if (PD_PLATFORM_INIT_OK != tca_dev.init) {
		usb_err("probe not init fin pls wait\n");
		return 0;
	}

	temp = readl(SOC_USB31_TCA_TCA_TCPC_ADDR(tca_dev.tca_base));
	/* Mux control from TCPM controlling the behavior of the ComboPHY DPAlt_Xbar
	 * and TCA synchronization.
	 * 00: No connection (default)
	 * 01: USB3.1 Connected
	 * 10: DP Alternate Mode - 4 lanes
	 * 11: USB3.1 + Display Port Lanes 0 & 1
	 */
	return (temp & COMBOPHY_MODE_MASK) == DPONLY_MODE;
}

/*
 * return 1 if USBONLY_MODE
 * default is 1
 */
int combophy_is_usb_only(void)
{
	volatile uint32_t temp;

	if (PD_PLATFORM_INIT_OK != tca_dev.init) {
		usb_err("probe not init fin pls wait\n");
		return 1;
	}

	temp = readl(SOC_USB31_TCA_TCA_TCPC_ADDR(tca_dev.tca_base));
	/* Mux control from TCPM controlling the behavior of the ComboPHY DPAlt_Xbar
	 * and TCA synchronization.
	 * 00: No connection (default)
	 * 01: USB3.1 Connected
	 * 10: DP Alternate Mode - 4 lanes
	 * 11: USB3.1 + Display Port Lanes 0 & 1
	 */
	return (temp & COMBOPHY_MODE_MASK) == USBONLY_MODE;
}

static int pd_get_resource(struct tca_device_s *res, struct device *dev)
{
	struct device_node *np;

	/*
	 * map PCTRL region
	 */
	np = of_find_compatible_node(NULL, NULL, "hisilicon,pctrl");
	if (!np) {
		usb_err("get pctrl node failed!\n");
		return -EINVAL;
	}
	res->pctrl_reg_base = of_iomap(np, 0);
	if (!res->pctrl_reg_base) {
		usb_err("iomap pctrl_reg_base failed!\n");
		return -EINVAL;
	}

	/*
	 * map USB2OTG bc region
	 */
	np = of_find_compatible_node(NULL, NULL, "hisilicon,usb3otg_bc");
	if (!np) {
		usb_err("get usb3otg_bc failed!\n");
		goto PCTRL_MAP_REL;
	}
	res->usb_misc_base = of_iomap(np, 0);
	if (!res->usb_misc_base) {
		usb_err("iomap usb3otg_bc failed!\n");
		goto PCTRL_MAP_REL;
	}

	res->tca_base = res->usb_misc_base + 0x200;

	res->gt_clk_usb3_tcxo_en = devm_clk_get(dev, "clk_usb3_tcxo_en");
	if (IS_ERR_OR_NULL(res->gt_clk_usb3_tcxo_en)) {
		usb_err("dgt_clk_usb3_tcxo_enfailed!\n");
		goto USB_MISC_REL;
	}

#ifdef COMBOPHY_FW_UPDATE
	if (of_property_read_u32(dev->of_node, "update_combophy_firmware",
				&(res->update_combophy_firmware))) {
		res->update_combophy_firmware = 0;
	}

	if (of_property_read_u32(dev->of_node, "combophy_resistor", &(res->combophy_resistor))){
		res->combophy_resistor = 0;
	}
#endif
	return 0;

USB_MISC_REL:
	iounmap(res->usb_misc_base);
PCTRL_MAP_REL:
	iounmap(res->pctrl_reg_base);
	return -1;
}
#ifdef COMBOPHY_REBOOT
static int combophy_reboot(void)
{
	int ret = 0;
	usb_info("\n");

	if (tca_dev.tca_cur_mode&TCPC_DP) {
		ret = hisi_dptx_triger((bool)0);
		if (ret) {
			usb_err("hisi_dptx_triger err\n");
			return ret;
		}
	}

	if (TCA_CHARGER_CONNECT_EVENT == tca_dev.usbctrl_status) {
		ret = hisi_usb_otg_event_sync(TCPC_USB31_CONNECTED, (enum otg_dev_event_type)TCA_CHARGER_DISCONNECT_EVENT, tca_dev.typec_orien);
		if (ret) {
			usb_err("[%d]hisi_usb_otg_event_sync  err\n", tca_dev.usbctrl_status);
			return ret;
		}

		tca_dev.usbctrl_status = TCA_CHARGER_DISCONNECT_EVENT;
	}

	if (TCA_ID_FALL_EVENT == tca_dev.usbctrl_status) {
		ret = hisi_usb_otg_event_sync(TCPC_USB31_CONNECTED, (enum otg_dev_event_type)TCA_ID_RISE_EVENT, tca_dev.typec_orien);
		if (ret) {
			usb_err("[%d]hisi_usb_otg_event_sync  err\n", tca_dev.usbctrl_status);
			return ret;
		}

		tca_dev.usbctrl_status = TCA_ID_RISE_EVENT;
	}

	combophy_reset_misc_ctrl(tca_dev.combophy);
	combophy_unreset_misc_ctrl(tca_dev.combophy);

	tca_dev.tca_poweron = TCA_POWEROFF;
	/* arg2: is usb firmware update. 1: update fw */
	return combophy_poweron(TCA_POWER_REBOOT, 1);
}
#endif

#ifndef COMBOPHY_VERSION_1_1
void combophy_clk_check(void)
{
	unsigned short int reg;
	usb_err("\n");
	writel(0x30300, SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_ADDR(tca_dev.usb_misc_base));
	udelay(20);
	writel(0x30303, SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_ADDR(tca_dev.usb_misc_base));
	udelay(100);
	(void)usb31phy_cr_read(0);
	reg = usb31phy_cr_read(0);
	if (0x74cd != reg) {
		usb_err("no clk.CR0[0x%x]\n", reg);
	}
	usb_err("CR0[0x%x]\n", reg);
	reg = usb31phy_cr_read(0x05);
	reg |= BIT(0)|BIT(15);
	usb31phy_cr_write(5, reg);
	udelay(100);
	reg = usb31phy_cr_read(0x19);
	if (0 == ((reg>>13)&0x01)){
		usb_err(" clk err.CR0x19[0x%x]\n", reg);
	}
	usb_err("CR0x19[0x%x]\n", reg);
}
#endif

void misc_dump(void)
{
	int i;
	usb_err("[USBMISC]+\n");
	for(i=0;i <= 0x250; i+=4) {
		usb_err("[%x]:[%x]\n", i,readl(i+tca_dev.usb_misc_base));
	}
	usb_err("[USBMISC]-\n");

#ifndef COMBOPHY_VERSION_1_1
	combophy_clk_check();
#endif
}

static int _tca_mode_switch(TCPC_MUX_CTRL_TYPE old_mode,
	TCPC_MUX_CTRL_TYPE new_mode, TYPEC_PLUG_ORIEN_E typec_orien)
{
	struct timeval tv;
	volatile unsigned int reg_data = 0x10;
	/*1.	调用切换前检查BC模式： 0xff200034[0] = 0，否则有异常(充电时不得切换)。 */
	if (is_bits_set(BIT(SOC_USB31_MISC_CTRL_BC_CTRL1_bc_mode_START),SOC_USB31_MISC_CTRL_BC_CTRL1_ADDR(tca_dev.usb_misc_base))) {
		usb_err("BC_CTRL1[0x%x][now is BC status,tca switch is forbidden ]\n",
			readl(SOC_USB31_MISC_CTRL_BC_CTRL1_ADDR(tca_dev.usb_misc_base)));
		return -EPERM;
	}

	writel(0xFFFF, SOC_USB31_TCA_TCA_INTR_STS_ADDR(tca_dev.tca_base));
	udelay(1);
	/*set_bits(0x3, SOC_USB31_TCA_TCA_INTR_EN_ADDR(tca_dev.tca_base));  for irq mode,but we use poll waitting */
	reg_data |= new_mode;
	reg_data |= (0x01&typec_orien)<<SOC_USB31_TCA_TCA_TCPC_tcpc_connector_orientation_START;
	writel_mask(0x1F,reg_data, SOC_USB31_TCA_TCA_TCPC_ADDR(tca_dev.tca_base));
	tca_dev.typec_orien = typec_orien;
	udelay(1);
	usb_info("old[%d]new[%d]TCPC[0x%x][0x%x]\n",
		old_mode, new_mode, reg_data, readl(SOC_USB31_TCA_TCA_TCPC_ADDR(tca_dev.tca_base)));

	do_gettimeofday(&tv);
	usb_info("tv_sec %ld,tv_usec: %06ld\n", tv.tv_sec, tv.tv_usec);

	while(is_bits_clr(BIT(SOC_USB31_TCA_TCA_INTR_STS_xa_ack_evt_START),
		SOC_USB31_TCA_TCA_INTR_STS_ADDR(tca_dev.tca_base))) {
			tca_dev.sw_cnt--;
			if(is_bits_set(BIT(SOC_USB31_TCA_TCA_INTR_STS_xa_timeout_evt_START),
				SOC_USB31_TCA_TCA_INTR_STS_ADDR(tca_dev.tca_base))) {
				unsigned int a,b;
				reg_data= readl(SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_ADDR(tca_dev.tca_base));
				a = 0x1&(reg_data>>SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_ss_rxdetect_disable_START);
				b = 0x1&(reg_data>>SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_ss_rxdetect_disable_ack_START);
				if(tca_dev.sw_cnt >0) {
					msleep(50);
					if(a == b) {
						// cppcheck-suppress *
						(void)_tca_mode_switch(old_mode, new_mode, typec_orien);
					}else
						continue;
				}else {
					usb_err("CTRLSYNCMODE_DBG0 TIMEOUT\n");
					return -EMLINK;
				}
			}else if(tca_dev.sw_cnt > 0){
				msleep(50);
			}else {
				usb_err("soc timeout not set;soft timeout\n");
				return -ERANGE;
			}
	}

	tca_dev.tca_cur_mode = new_mode;
	return 0;
}

void cr_dump(void)
{
	int i;
	usb_err("++++\n");
	for(i=0;i <= 0x006f; i++) {
		pr_err("[%x]:[%x]\n", i, usb31phy_cr_read(i));
	}

	for(i=0x1000;i <= 0x010D8; i++) {
		pr_err("[%x]:[%x]\n", i, usb31phy_cr_read(i));
	}

	for(i=0x1100;i <= 0x011D8; i++) {
		pr_err("[%x]:[%x]\n", i, usb31phy_cr_read(i));
	}

	for(i=0x1200;i <= 0x12D8; i++) {
		pr_err("[%x]:[%x]\n", i, usb31phy_cr_read(i));
	}

	for(i=0x1300;i <= 0x13D8; i++) {
		pr_err("[%x]:[%x]\n", i, usb31phy_cr_read(i));
	}

	for(i=0x2000;i <= 0x203b; i++) {
		pr_err("[%x]:[%x]\n", i, usb31phy_cr_read(i));
	}

	for(i=0x3000;i <= 0x30e4; i++) {
		pr_err("[%x]:[%x]\n", i, usb31phy_cr_read(i));
	}

	for(i=0x6000;i < 0x6000+2737; i++) {
		pr_err("[%x]:[%x]\n", i, usb31phy_cr_read(i));
	}

	usb_err("-----\n");
}

static int __tca_mode_sw(TCPC_MUX_CTRL_TYPE old_mode, TCPC_MUX_CTRL_TYPE new_mode, TYPEC_PLUG_ORIEN_E typec_orien)
{
	int ret;
	struct timeval tv;

	tca_dev.sw_cnt = 12;
	ret = _tca_mode_switch(old_mode, new_mode,typec_orien);
	do_gettimeofday(&tv);
	usb_info("e:tv_sec %ld,tv_usec: %06ld\n", tv.tv_sec, tv.tv_usec);
	usb_info("[end]BC_CTRL1[0x%x]\n",readl(SOC_USB31_MISC_CTRL_BC_CTRL1_ADDR(tca_dev.usb_misc_base)));
	if (ret)
		misc_dump();
	return ret;
}

static int tca_mode_sw(TCPC_MUX_CTRL_TYPE new_mode, TYPEC_PLUG_ORIEN_E typec_orien)
{
	int ret = 0;
	int old_mode = tca_dev.tca_cur_mode;

	if(old_mode == TCPC_NC && TCPC_DP == new_mode) {
		ret = __tca_mode_sw(TCPC_NC, TCPC_USB31_CONNECTED,typec_orien);
		if (ret) {
			usb_err("_tca_mode_switch  err1 [%d]\n", __LINE__);
			return ret;
		}
		set_bits(BIT(SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG0_block_ss_op_START),
			SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG0_ADDR(tca_dev.tca_base));
		msleep(1);
	}

	ret = __tca_mode_sw(tca_dev.tca_cur_mode, new_mode, typec_orien);
	if(ret) {
		usb_err("_tca_mode_switch  err2 [%d]\n", __LINE__);
		return ret;
	}

	if(old_mode == TCPC_NC && TCPC_DP == new_mode) {
		clr_bits(BIT(SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG0_block_ss_op_START),
			SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG0_ADDR(tca_dev.tca_base));
	}

	return ret;
}

static int tca_mode_switch(TCPC_MUX_CTRL_TYPE new_mode, TYPEC_PLUG_ORIEN_E typec_orien)
{
	int ret = 0, usb_ret= 0;
	int cnt = 50;
	volatile unsigned int reg;
#ifndef COMBOPHY_VERSION_1_1
	int old_mode = tca_dev.tca_cur_mode;

	if(TCPC_DP == old_mode) {
		usb31phy_cr_write(0x05, 0x8199);
		usb31phy_cr_write(0x05, 0x8199);
		udelay(100);
	}
#endif
	usb_info("BC_CTRL1[0x%x]\n", readl(SOC_USB31_MISC_CTRL_BC_CTRL1_ADDR(tca_dev.usb_misc_base)));

	/*used -EBUSY*/
	ret = hisi_usb_combophy_notify(PHY_MODE_CHANGE_BEGIN);
	if (ret == -ENOMEM || ret == -EINVAL) {
		usb_err("hisi_usb_combophy_notify  err\n");
		return ret;
	}
	if (ret) {
		ret =  -ENXIO;
		goto USB_CHANGE_FIN;
	}
#ifndef COMBOPHY_VERSION_1_1
	/*3.	切换时（调用usb进入P3接口后）检查复位：0xff2000A0 = 0x30303,否则异常*/
	reg = readl(SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_ADDR(tca_dev.usb_misc_base));
	if (0x30303 != reg) {
		usb_err("CFGA0[0x%x]  err\n", reg);
		ret =  -EPERM;
		goto USB_CHANGE_FIN;
	}
#endif
	if (combophy_is_misc_ctrl_reset(tca_dev.combophy)) {
		usb_err("misc ctrl is under reset\n");
		ret =  -EFAULT;
		goto USB_CHANGE_FIN;
	}

	while (cnt--) {
		if (TCPC_NC == new_mode) {
			if (0x333333 == readl(SOC_USB31_TCA_TCA_PSTATE_ADDR(tca_dev.tca_base)))
				break;
		} else
			break;
		msleep(10);
	}


	reg = readl(SOC_USB31_TCA_TCA_PSTATE_ADDR(tca_dev.tca_base));
	usb_info("TCA_PSTATE[%x]cnt[%d]\n", reg,cnt);
	if (0 == reg) {
		misc_dump();
		ret = -ESRCH;
		goto USB_CHANGE_FIN;
	}

	ret = tca_mode_sw(new_mode, typec_orien);
	if (ret)
		goto USB_CHANGE_FIN;

#ifndef COMBOPHY_VERSION_1_1
	if (TCPC_DP == old_mode) {
		usb31phy_cr_write(0x05, 0x198);
		udelay(100);
	}
#endif

USB_CHANGE_FIN:
	usb_ret = hisi_usb_combophy_notify(PHY_MODE_CHANGE_END);
	if (usb_ret) {
		usb_err("hisi_usb_combophy_notify END  err\n");
		return usb_ret;
	}
	return ret;
}

#ifdef COMBOPHY_FW_UPDATE
static void firmware_update(void)
{
	usb_info(" start\n");
	combophy_firmware_update(tca_dev.combophy);

	if (tca_dev.combophy_resistor) {
		usb31phy_cr_write(0x301a, tca_dev.combophy_resistor);
		usb31phy_cr_write(0x311a, tca_dev.combophy_resistor);
		usb31phy_cr_write(0x321a, tca_dev.combophy_resistor);
		usb31phy_cr_write(0x331a, tca_dev.combophy_resistor);
	}
	usb_info(" end\n");
}
#endif

/*lint  -e838 -e747*/
static int combophy_poweron(TYPEC_PLUG_ORIEN_E typec_orien, bool is_fw_update)
{
	int ret = 0;
	int wait_cnt = 50;

	usb_info("tca_poweron[%d]\n", tca_dev.tca_poweron);
	if (TCA_POWERON == tca_dev.tca_poweron)
		return 0;

	usb_info("dwc3_misc_ctrl_get\n");
	ret = dwc3_misc_ctrl_get(MICS_CTRL_COMBOPHY);
	if (ret) {
		usb_err("misc ctrl get err\n");
		return -EACCES;
	}

	/* XBar_Assist TCPC Request Timeout set to 0x0927C/3 */
	writel((0x0927C/3), SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG1_ADDR(tca_dev.tca_base));

	/*4	open combo phy */
	combophy_isodis(tca_dev.combophy);

	ret = clk_prepare_enable(tca_dev.gt_clk_usb3_tcxo_en);
	if (ret) {
		usb_err("clk_prepare_enable  clk_usb3_tcxo_en err\n");
		return -EIO;
	}
	usb_info("gt_clk_usb3_tcxo_en enable+\n");
	if (__clk_is_enabled(tca_dev.gt_clk_usb3_tcxo_en) == false) {
		usb_err("gt_clk_usb3_tcxo_en  check err\n");
		return -ESPIPE;
	}

	msleep(10);
	clr_bits(BIT(PCTRL_PERI_CTRL24_CLK_USB3PHY_MUX_SEL),
			SOC_PCTRL_PERI_CTRL24_ADDR(tca_dev.pctrl_reg_base));

	/*5	dp-->p3 mode*/
	ret = hisi_dptx_notify_switch();
	if (ret) {
		usb_err(" hisi_dptx_notify_switch err\n");
		return -EIO;
	}

	/*5.5 release USB31 PHY out of TestPowerDown mode*/
	combophy_exit_testpowerdown(tca_dev.combophy);

#ifdef COMBOPHY_FW_UPDATE
	if (is_fw_update && (tca_dev.update_combophy_firmware != 0)) {
		usb_info("mode is TCPC_USB31_CONNECTED,need update fully!\n");
		combophy_firmware_update_prepare(tca_dev.combophy);
	}
#endif

	/*6	unreset combo phy*/
	set_bits(BIT(SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_usb3phy_reset_n_START),
			SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_ADDR(tca_dev.usb_misc_base));

#ifdef COMBOPHY_FW_UPDATE
	if (is_fw_update && (tca_dev.update_combophy_firmware != 0))
		firmware_update();
#endif

	if (is_fw_update) {
		while(is_bits_clr(BIT(SOC_USB31_TCA_TCA_INTR_STS_xa_ack_evt_START)|
					BIT(SOC_USB31_TCA_TCA_INTR_STS_xa_timeout_evt_START),
					SOC_USB31_TCA_TCA_INTR_STS_ADDR(tca_dev.tca_base))) {
			msleep(20);
			if(wait_cnt-- <= 0)
				break;
		}

		if (wait_cnt <= 0)
			usb_err("wait_cnt[%d]\n", wait_cnt);

		writel(0xFFFF, SOC_USB31_TCA_TCA_INTR_STS_ADDR(tca_dev.tca_base));
	}

	clr_bits(BIT(SOC_USB31_TCA_TCA_TCPC_tcpc_low_power_en_START),SOC_USB31_TCA_TCA_TCPC_ADDR(tca_dev.tca_base));
	udelay(2);
	usb_info("TCA_TCPC[%x]\n", readl(SOC_USB31_TCA_TCA_TCPC_ADDR(tca_dev.tca_base)));

	if (is_fw_update) {
		/* XBar_Assist TCPC Request Timeout set to 2* 0x0927C */
		writel((2* 0x0927C), SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG1_ADDR(tca_dev.tca_base));
		ret = tca_mode_switch(TCPC_NC, typec_orien);

		writel(BIT(SOC_USB31_TCA_TCA_INTR_STS_xa_ack_evt_START)|
				BIT(SOC_USB31_TCA_TCA_INTR_STS_xa_timeout_evt_START),
				SOC_USB31_TCA_TCA_INTR_STS_ADDR(tca_dev.tca_base));
	}

	tca_dev.tca_poweron = TCA_POWERON;

	usb_info("poweron ok[%d]\n", tca_dev.tca_poweron);
	return ret;
}


static int combophy_init(TCPC_MUX_CTRL_TYPE new_mode, TYPEC_PLUG_ORIEN_E typec_orien, bool is_fw_update)
{
	int ret = combophy_poweron(typec_orien, is_fw_update);
	if (ret)
		goto COM_FIN;

	ret = hisi_dptx_notify_switch();
	if (ret) {
		usb_err(" hisi_dptx_notify_switch err\n");
		ret = -EIO;
		goto COM_FIN;
	}

	if (is_fw_update) {
		ret = tca_mode_switch(new_mode, typec_orien);
	}
COM_FIN:
	return ret;
}

int combophy_sw_sysc(TCPC_MUX_CTRL_TYPE new_mode, TYPEC_PLUG_ORIEN_E typec_orien, bool lock_flag)
{
	int ret = 0;
	if(PD_PLATFORM_INIT_OK != tca_dev.init) {
		usb_err("probe not init fin pls wait\n");
		return -EACCES;
	}

	if (lock_flag)
		if (!mutex_trylock(&tca_mutex)) {
			usb_err("pd evnet handler is running\n");
			return 0;
		}

	usb_info("\n");

	/* arg3: is usb firmware update. 1: update fw */
	ret = combophy_init(new_mode, typec_orien, (bool)1);
	(void)hisi_dptx_triger((bool)0);

	if (lock_flag)
		mutex_unlock(&tca_mutex); /*lint !e455 */

	return ret;
}

/*lint  +e838*/
/*lint -e124 */
static int usbctrl_status_update(TCPC_MUX_CTRL_TYPE mode_type, TCA_DEV_TYPE_E dev_type, TYPEC_PLUG_ORIEN_E typec_orien)
{
	int ret = 0;
	if (dev_type <= TCA_ID_RISE_EVENT) {
		usb_info("hisi_usb_otg_event_sync[%d]\n", dev_type);
		ret = hisi_usb_otg_event_sync(mode_type, (enum otg_dev_event_type)dev_type, typec_orien);
		if (ret) {
			usb_err("hisi_usb_otg_event_sync err[%d]\n", ret);
			return ret;
		}

		tca_dev.usbctrl_status = dev_type;
	}

	return ret;
}

static int combophy_shutdown(void)
{
	usb_info("[%d]\n",tca_dev.tca_poweron);
	if(TCA_POWEROFF == tca_dev.tca_poweron)
		return 0;

	if (combophy_is_misc_ctrl_reset(tca_dev.combophy)) {
		usb_err("misc ctrl is under reset\n");
		goto USB_MISC_CTRL_FIN;
	}

	if (!combophy_is_misc_ctrl_clk_enable(tca_dev.combophy)) {
		usb_err("misc ctrl clk is disabled\n");
		goto USB_MISC_CTRL_FIN;
	}

	/*reset PHY*/
	combophy_reset_phy(tca_dev.combophy);

USB_MISC_CTRL_FIN:
	if(__clk_is_enabled(tca_dev.gt_clk_usb3_tcxo_en)) {
		clk_disable_unprepare(tca_dev.gt_clk_usb3_tcxo_en);
		usb_info("gt_clk_usb3_tcxo_en disable\n");
	}

	usb_info("dwc3_misc_ctrl_put\n");
	dwc3_misc_ctrl_put(MICS_CTRL_COMBOPHY);

	tca_dev.tca_poweron = TCA_POWEROFF;
	return 0;
}

int combophy_poweroff(void)
{
	int ret;
	(void)hisi_dptx_triger((bool)0);
	ret = combophy_shutdown();
	tca_dev.tca_cur_mode = TCPC_NC;
	return ret;
}

static const char *irq_type_string(TCA_IRQ_TYPE_E irq_type)
{
	static const char *const irq_type_strings[] = {
		[TCA_IRQ_HPD_OUT]		= "TCA_IRQ_HPD_OUT",
		[TCA_IRQ_HPD_IN]		= "TCA_IRQ_HPD_IN",
		[TCA_IRQ_SHORT]			= "TCA_IRQ_SHORT",
	};

	if (irq_type < TCA_IRQ_HPD_OUT || irq_type > TCA_IRQ_SHORT)
		return "illegal irq_type";

	return irq_type_strings[irq_type];
}

static const char *mode_type_string(TCPC_MUX_CTRL_TYPE mode_type)
{
	static const char *const mode_type_strings[] = {
		[TCPC_NC]		= "TCPC_NC",
		[TCPC_USB31_CONNECTED]		= "TCPC_USB31_CONNECTED",
		[TCPC_DP]			= "TCPC_DP",
		[TCPC_USB31_AND_DP_2LINE]		= "TCPC_USB31_AND_DP_2LINE",
	};

	if (mode_type < TCPC_NC || mode_type > TCPC_USB31_AND_DP_2LINE)
		return "illegal mode_type";

	return mode_type_strings[mode_type];
}

static const char *dev_type_string(TCA_DEV_TYPE_E dev_type)
{
	static const char *const dev_type_strings[] = {
		[TCA_CHARGER_CONNECT_EVENT]		= "TCA_CHARGER_CONNECT_EVENT",
		[TCA_CHARGER_DISCONNECT_EVENT]		= "TCA_CHARGER_DISCONNECT_EVENT",
		[TCA_ID_FALL_EVENT]			= "TCA_ID_FALL_EVENT",
		[TCA_ID_RISE_EVENT]		= "TCA_ID_RISE_EVENT",
		[TCA_DP_OUT]			= "TCA_DP_OUT",
		[TCA_DP_IN]		= "TCA_DP_IN",
	};

	if (dev_type < TCA_CHARGER_CONNECT_EVENT || dev_type > TCA_DP_IN)
		return "illegal dev_type";

	return dev_type_strings[dev_type];
}

/*lint +e124 -e454 -e456*/
int pd_event_notify(TCA_IRQ_TYPE_E irq_type, TCPC_MUX_CTRL_TYPE mode_type, TCA_DEV_TYPE_E dev_type, TYPEC_PLUG_ORIEN_E typec_orien)
{
	int ret;
	pd_event_t pd_event;

	usb_info("IRQ[%s]MODEcur[%s]new[%s]DEV[%s]ORIEN[%d]\n",
		irq_type_string(irq_type), mode_type_string(tca_dev.tca_cur_mode),
		mode_type_string(mode_type), dev_type_string(dev_type), typec_orien);
	dp_imonitor_set_pd_event(irq_type, tca_dev.tca_cur_mode, mode_type, dev_type, typec_orien);

	if(PD_PLATFORM_INIT_OK != tca_dev.init) {
		usb_err("probe not init fin pls wait\n");
		return -EIO;
	}

	if (irq_type >=TCA_IRQ_MAX_NUM || mode_type >= TCPC_MUX_MODE_MAX || dev_type >= TCA_DEV_MAX || typec_orien >= TYPEC_ORIEN_MAX)
		return -EPERM;

	if (!wake_lock_active(&tca_dev.wlock))
		wake_lock(&tca_dev.wlock);
	usb_info("wake lock\n");
	pd_event.irq_type = irq_type;
	pd_event.mode_type = mode_type;
	pd_event.dev_type = dev_type;
	pd_event.typec_orien = typec_orien;
	ret = kfifo_in(&tca_dev.kfifo, &pd_event, (unsigned int)sizeof(pd_event_t));
	usb_info("kfifo_in[%d]\n", ret);
	if(!queue_work(tca_dev.wq, &tca_dev.work))
		usb_err("tca wq is doing\n");
	return 0;
}


static int pd_notify_nc(pd_event_t *event)
{
	int ret;
	if ((tca_dev.tca_cur_mode&TCPC_DP)&&(event->dev_type > TCA_ID_RISE_EVENT)){
		hisi_dptx_hpd_trigger(event->irq_type, tca_dev.tca_cur_mode, event->typec_orien);
	}

	ret = usbctrl_status_update(event->mode_type, event->dev_type, event->typec_orien);
	if (ret && -EINVAL != ret)
		goto PD_NOTIFY_NC;

	ret = combophy_poweroff();

PD_NOTIFY_NC:
	return ret;
}

static int pd_event_handle_update(pd_event_t *event)
{
	int ret;

	ret = usbctrl_status_update(event->mode_type, event->dev_type, event->typec_orien);
	if (ret)
		goto TCA_SW_FIN;

	if (event->dev_type < TCA_DP_OUT)
		goto TCA_SW_FIN;


	if (TCA_DP_IN == event->dev_type) {
		ret = hisi_dptx_triger((bool)1);
		if (ret) {
			usb_err("hisi_dptx_triger err[%d][%d]\n", __LINE__,ret);
			ret = -EFAULT;
			goto TCA_SW_FIN;
		}
	}

	ret = hisi_dptx_hpd_trigger(event->irq_type, tca_dev.tca_cur_mode, event->typec_orien);
	if (ret) {
		usb_err("hisi_dptx_hpd_trigger err[%d][%d]\n", __LINE__,ret);
		ret = -EPIPE;
		goto TCA_SW_FIN;
	}

	if (TCA_DP_OUT == event->dev_type) {
		ret = hisi_dptx_triger((bool)0);
		if (ret) {
			usb_err("hisi_dptx_triger err[%d][%d]\n", __LINE__,ret);
			ret = -ERANGE;
			goto TCA_SW_FIN;
		}
	}

TCA_SW_FIN:

	return ret;
}

static int pd_event_processing(pd_event_t *event)
{
	int ret = 0;

	if(TCPC_NC == tca_dev.tca_cur_mode) {
		/* arg2: is usb firmware update. 1: update fw */
		ret = combophy_poweron(event->typec_orien, 1);
		if(ret) {
			goto TCA_SW_FIN;
		}
	}

	if(TCA_IRQ_SHORT == event->irq_type)
		hisi_dptx_hpd_trigger(event->irq_type, tca_dev.tca_cur_mode, event->typec_orien);
	else if (tca_dev.tca_cur_mode == event->mode_type) {
		ret = pd_event_handle_update(event);
	}else {
		ret = hisi_dptx_notify_switch();
		if (ret) {
			usb_err(" hisi_dptx_notify_switch err\n");
			ret = -EIO;
			goto TCA_SW_FIN;
		}

		ret = tca_mode_switch(event->mode_type, event->typec_orien);
		if (ret) {
			usb_err(" tca_mode_switch err\n");
			goto TCA_SW_FIN;
		}

		/*
		2）USB在位状态，考虑到USB可能存在数传，
		PD不能直接做PHY的模式切换，必须先做一下拔出，再到新的状态。
		USB->DP4:  USB->NC->DP4
		USB->USB+DP4:USB->NC->USB+DP4
		*/
		ret = usbctrl_status_update(event->mode_type, event->dev_type, event->typec_orien);
		if (ret)
			goto TCA_SW_FIN;

		if (event->dev_type >= TCA_DP_OUT)
			hisi_dptx_hpd_trigger(event->irq_type, tca_dev.tca_cur_mode, event->typec_orien);
		else {
			ret = hisi_dptx_triger((bool)0);
			if (ret) {
				usb_err("hisi_dptx_triger err[%d][%d]\n", __LINE__,ret);
				ret = -ERANGE;
				goto TCA_SW_FIN;
			}
		}
	}

TCA_SW_FIN:
	return ret;
}

/*lint  -e655 +e747 +e454 +e456*/
void pd_event_hander(pd_event_t *event)
{
	int ret =0;
	unsigned int hifi_first_flag = 0;

	if ((event->dev_type == TCA_ID_FALL_EVENT)
		&& (event->mode_type == TCPC_USB31_CONNECTED)
		&& (hisi_usb_otg_use_hifi_ip_first()))
		hifi_first_flag = 1;

	usb_info("IRQ[%d]MODEcur[%d]new[%d]DEV[%d]ORIEN[%d]hifi_first_flag[%d]\n",
			event->irq_type, tca_dev.tca_cur_mode, event->mode_type,
			event->dev_type, event->typec_orien, hifi_first_flag);
	if (TCPC_NC == event->mode_type) {
		ret = pd_notify_nc(event);
	} else {
		if (hifi_first_flag) {
			/* Must set the tca_cur_mode before calling
			 * usbctrl_status_update, because usbctrl_status_update
			 * may use it! */
			tca_dev.tca_cur_mode = event->mode_type;

			ret = usbctrl_status_update(event->mode_type, event->dev_type, event->typec_orien);
			if (ret) {
				usb_info("usbctrl_status_update failed[%d]n", ret);
			}
		} else {
			ret = pd_event_processing(event);
			if (ret) {
				usb_err("pd_event_processing err\n");
			}
		}
	}

	if (ret)
		(void)hisi_dptx_triger((bool)0);

	usb_info("CurMode[%d]RET[%d]\n", tca_dev.tca_cur_mode, ret);
#ifdef CONFIG_TCPC_CLASS
	dp_dfp_u_notify_dp_configuration_done(tca_dev.tca_cur_mode, ret);
#endif
}
/*lint -e715  -e747 +e655 -e455*/
void  tca_wq(struct work_struct *data)
{
	pd_event_t pd_event;
	unsigned long len;
	while (!kfifo_is_empty(&tca_dev.kfifo)) {
		mutex_lock(&tca_mutex);
		memset((void*)&pd_event, 0, sizeof(pd_event_t));
		len = kfifo_out(&tca_dev.kfifo, &pd_event, (unsigned int)sizeof(pd_event_t));
		if (len != sizeof(pd_event_t))
			usb_err("kfifo_out  err\n");
		pd_event_hander(&pd_event);
		mutex_unlock(&tca_mutex);
	}

	if(wake_lock_active(&tca_dev.wlock))
		wake_unlock(&tca_dev.wlock);
	usb_info("wake unlock\n");
}

static void tca_devouces_rel(void)
{
	iounmap(tca_dev.usb_misc_base);
	iounmap(tca_dev.pctrl_reg_base);
}

/*lint +e715 +e747 +e455*/
static int __init pd_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct device_node *dev_node = dev->of_node;
	if (!of_device_is_available(dev_node))
		return -ENODEV;

	ret = pd_get_resource(&tca_dev, dev);
	if (ret) {
		usb_err(" pd_get_resource err\n");
		return -EINVAL;
	}

	tca_dev.usbctrl_status = TCA_CHARGER_DISCONNECT_EVENT;
	tca_dev.tca_poweron =  TCA_POWEROFF;
	tca_dev.wq = create_singlethread_workqueue("tca_wq");
	if (NULL == tca_dev.wq) {
		usb_err("tca_wq  err\n");
		ret = -EPIPE;
		goto err_release_resource;
	}

	ret = kfifo_alloc(&tca_dev.kfifo, FIFO_SIZE, GFP_KERNEL);
	if (ret) {
		usb_err("kfifo_alloc  err[%d]\n", ret);
		goto err_release_resource;
	}

	tca_dev.combophy = usb3_get_combophy_phandle();
	if (!tca_dev.combophy) {
		usb_err("can not get combophy phandle\n");
		ret = -ENODEV;
		goto err_free_kfifo;
	}

	wake_lock_init(&tca_dev.wlock, WAKE_LOCK_SUSPEND, "phy");
	INIT_WORK(&tca_dev.work, tca_wq);
	tca_dev.init = PD_PLATFORM_INIT_OK;

	return 0;

err_free_kfifo:
	kfifo_free(&tca_dev.kfifo);
err_release_resource:
	tca_devouces_rel();

	return ret;
}

/*lint -e705 -e715*/
static int pd_remove(struct platform_device *pdev)
{
	tca_devouces_rel();
	kfifo_free(&tca_dev.kfifo);
	wake_lock_destroy(&tca_dev.wlock);
	tca_dev.combophy = NULL;

	return 0;
}


#ifdef CONFIG_PM
/*lint -save -e454 -e455 */
static int hisi_pd_prepare(struct device *dev)
{
	usb_info("+\n");
	mutex_lock(&tca_mutex);
	usb_info("-\n");
	return 0;
}

static void hisi_pd_complete(struct device *dev)
{
	usb_info("+\n");
	mutex_unlock(&tca_mutex);
	usb_info("-\n");
}

static int combophy_suspend(struct device *dev)
{
	usb_info("+\n");
	if (tca_dev.tca_cur_mode > TCPC_NC && tca_dev.tca_cur_mode < TCPC_MUX_MODE_MAX) {
		(void)hisi_dptx_triger((bool)0);
		(void)combophy_shutdown();
	}

	usb_info("-\n");
	return 0;
}

static int combophy_resume(struct device *dev)
{
	usb_info("+\n");
	if (tca_dev.tca_cur_mode > TCPC_NC && tca_dev.tca_cur_mode < TCPC_MUX_MODE_MAX) {
		/* arg3: is usb firmware update. 1: update fw */
		combophy_init(tca_dev.tca_cur_mode, tca_dev.typec_orien, (bool)0);
	}
	usb_info("-\n");
	return 0;
}
#endif

/*lint +e705 +e715*/
/*lint -e785 -e64*/


const struct dev_pm_ops hisi_pd_pm_ops = {
#ifdef CONFIG_PM_SLEEP
	.prepare = hisi_pd_prepare,
	.complete = hisi_pd_complete,
	.suspend_late = combophy_suspend,
	.resume_early = combophy_resume,
#endif
};

#ifdef CONFIG_PM
#define HISI_PD_PM_OPS (&hisi_pd_pm_ops)
#else
#define HISI_PD_PM_OPS NULL
#endif


static struct of_device_id hisi_pd_of_match[] = {
	{ .compatible = "hisilicon,pd"},
	{},
};

MODULE_DEVICE_TABLE(of, hisi_pd_of_match);
static struct platform_driver pd_platdrv = {
	.driver = {
		.name		= "hisi-pd",
		.owner		= THIS_MODULE,
		.of_match_table = hisi_pd_of_match,
		.pm	= HISI_PD_PM_OPS,
	},
	.probe	= pd_probe,
	.remove	= pd_remove,
};
/*lint +e785*/
/*lint  -e721*/
module_platform_driver(pd_platdrv);
/*lint +e528 +e721*/

