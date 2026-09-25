/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description:  this file implements api to cfg ivp core hardware to power up,
 * run, run stall, load firmware, etc
 * Author:  chenweiyu
 * Create:  2019-07-30
 */
#include "ivp_manager.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/dma-buf.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/pm_wakeup.h>
#include <linux/ion.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/hisi-iommu.h>
#include <linux/syscalls.h>
#include <linux/clk-provider.h>
#include <linux/bitops.h>
#include "securec.h"
#include "ivp.h"
#include "ivp_log.h"
#include "ivp_ioctl.h"
#include "ivp_smmu.h"
#include "ivp_reg.h"

void ivp_reg_write(struct ivp_device *pdev, unsigned int off, u32 val)
{
	char __iomem *reg = NULL;

	if (!pdev) {
		ivp_err("invalid param pdev");
		return;
	}

	reg = pdev->io_res.cfg_base_addr + off;

	writel(val, reg);
}

/* read ivp cfg reg */
u32 ivp_reg_read(struct ivp_device *pdev, unsigned int off)
{
	char __iomem *reg = NULL;
	u32 val;

	if (!pdev) {
		ivp_err("invalid param pdev");
		return 0;
	}

	reg = pdev->io_res.cfg_base_addr + off;
	val = readl(reg);
	return val;
}

static void ivp_pericrg_reg_write(struct ivp_device *pdev,
		unsigned int off, u32 val)
{
	char __iomem *reg = pdev->io_res.pericrg_base_addr + off;
	writel(val, reg);
}

/* read peri cfg reg for ivp */
static u32 ivp_pericrg_reg_read(struct ivp_device *pdev,
		unsigned int off)
{
	char __iomem *reg = pdev->io_res.pericrg_base_addr + off;
	u32 val = readl(reg);
	return val;
}

/******************************************************
 *     read/write watch dog reg need unlock first     *
 *****************************************************/
static void ivp_wdg_reg_write(struct ivp_device *pdev,
		unsigned int off, u32 val)
{
	char __iomem *reg = pdev->io_res.cfg_base_addr
		+ IVP_WDG_REG_BASE_OFFSET + off;
	writel(val, reg);
}

static u32 ivp_wdg_reg_read(struct ivp_device *pdev, unsigned int off)
{
	char __iomem *reg = pdev->io_res.cfg_base_addr
		+ IVP_WDG_REG_BASE_OFFSET + off;
	u32 val = readl(reg);
	return val;
}

static u32 ivp_pctrl_reg_read(struct ivp_device *pdev, unsigned int off)
{
	char __iomem *reg = pdev->io_res.pctrl_base_addr + off;
	u32 val = readl(reg);
	return val;
}

static void ivp_gic_reg_write(struct ivp_device *pdev,
		unsigned int off, u32 val)
{
	char __iomem *reg = pdev->io_res.gic_base_addr;
	writel(val, reg);
}

void ivp_hw_clr_wdg_irq(struct ivp_device *pdev)
{
	if(!pdev) {
		ivp_err("invalid param pdev");
		return;
	}
	// unlock reg
	ivp_wdg_reg_write(pdev, WDG_REG_OFF_WDLOCK, WDG_REG_UNLOCK_KEY);

	// clear irq
	ivp_wdg_reg_write(pdev, WDG_REG_OFF_WDINTCLR, 1);

	// disable irq
	ivp_wdg_reg_write(pdev, WDG_REG_OFF_WDCONTROL, 0);

	// lock reg
	ivp_wdg_reg_write(pdev, WDG_REG_OFF_WDLOCK, WDG_REG_LOCK_KEY);
}

void ivp_hw_set_ocdhalt_on_reset(struct ivp_device *pdev, int mode)
{
	ivp_reg_write(pdev, IVP_REG_OFF_OCDHALTONRESET, (unsigned int)mode);
}

void ivp_hw_set_bootmode(struct ivp_device *pdev, int mode)
{
	u32 val = (u32)mode;

	ivp_reg_write(pdev, IVP_REG_OFF_STATVECTORSEL, val & 0x01);
}

void ivp_hw_clockgate(struct ivp_device *pdev, int state)
{
	u32 val = (u32)state;

	ivp_reg_write(pdev, IVP_REG_OFF_DSPCORE_GATE, val & 0x01);
}

void ivp_hw_disable_reset(struct ivp_device *pdev)
{
	ivp_reg_write(pdev, IVP_REG_OFF_DSP_CORE_RESET_DIS, 0x04);
	ivp_reg_write(pdev, IVP_REG_OFF_DSP_CORE_RESET_DIS, 0x01);
	ivp_reg_write(pdev, IVP_REG_OFF_DSP_CORE_RESET_DIS, 0x02);
}

void ivp_hw_runstall(struct ivp_device *pdev, int mode)
{
	u32 val = (u32)mode;

	ivp_reg_write(pdev, IVP_REG_OFF_RUNSTALL, val & 0x01);
}

int ivp_hw_query_runstall(struct ivp_device *pdev)
{
	return (int)ivp_reg_read(pdev, IVP_REG_OFF_RUNSTALL);
}

void ivp_hw_trigger_NMI(struct ivp_device *pdev)
{
	/* risedge triger.triger 0->1->0 */
	ivp_reg_write(pdev, IVP_REG_OFF_NMI, 0);
	ivp_reg_write(pdev, IVP_REG_OFF_NMI, 1);
	ivp_reg_write(pdev, IVP_REG_OFF_NMI, 0);
}

int ivp_hw_query_waitmode(struct ivp_device *pdev)
{
	return (int)ivp_reg_read(pdev, IVP_REG_OFF_PWAITMODE);
}

static void ivp_hw_set_all_bus_gate_clock(struct ivp_device *pdev, int mode)
{
	if (mode == IVP_ENABLE)
		ivp_reg_write(pdev, IVP_REG_OFF_BUS_GATE_CLOCK,
			IVP_BUS_GATE_CLK_ENABLE);
	else
		ivp_reg_write(pdev, IVP_REG_OFF_BUS_GATE_CLOCK,
			IVP_BUS_GATE_CLK_DISABLE);
}

static void ivp_hw_set_all_smmu_awake_bypass(struct ivp_device *pdev, int mode)
{
	if (mode == IVP_ENABLE)
		ivp_reg_write(pdev, IVP_REG_OFF_SMMU_AWAKEBYPASS,
			IVP_SMMU_AWAKEBYPASS_ENABLE);
	else
		ivp_reg_write(pdev, IVP_REG_OFF_SMMU_AWAKEBYPASS,
			IVP_SMMU_AWAKEBYPASS_DISABLE);
}

static void ivp_hw_set_peri_autodiv(struct ivp_device *pdev, int mode)
{
	unsigned int enable = 0;

	ivp_info("set mode to:%d", mode);
	enable = ivp_pericrg_reg_read(pdev, PERICRG_REG_OFF_PERI_AUTODIV0);
	if (mode == IVP_ENABLE) {
		ivp_pericrg_reg_write(pdev, PERICRG_REG_OFF_PEREN6,
				GATE_CLK_AUTODIV_IVP);
		ivp_pericrg_reg_write(pdev, PERICRG_REG_OFF_PERI_AUTODIV0,
				enable &
				~(IVP_DW_AXI_M2_ST_BYPASS |
					IVP_DW_AXI_M1_ST_BYPASS |
					IVP_PWAITMODE_BYPASS |
					IVP_DIV_AUTO_REDUCE_BYPASS));
	} else {
		ivp_pericrg_reg_write(pdev, PERICRG_REG_OFF_PERDIS6,
				GATE_CLK_AUTODIV_IVP);
	}
}


void ivp_dev_set_dynamic_clk(struct ivp_device *pdev, unsigned int mode)
{
	if (mode >= IVP_MODE_INVALID || !pdev) {
		ivp_err("Invalid input param");
		return;
	}

	if (mode) {
		/* bus gate clock enable. */
		ivp_hw_set_all_bus_gate_clock(pdev, IVP_ENABLE);
		/* pericrg. */
		ivp_hw_set_peri_autodiv(pdev, IVP_ENABLE);
		/* smmu bypass enable. */
		ivp_hw_set_all_smmu_awake_bypass(pdev, IVP_DISABLE);

	} else {
		/* smmu bypass disable. */
		ivp_hw_set_all_smmu_awake_bypass(pdev, IVP_ENABLE);
		/* pericrg. */
		ivp_hw_set_peri_autodiv(pdev, IVP_DISABLE);
		/* bus gate clock disable. */
		ivp_hw_set_all_bus_gate_clock(pdev, IVP_DISABLE);
	}
}


void ivp_dev_stop(struct ivp_device *pdev)
{
	ivp_hw_runstall(pdev, IVP_RUNSTALL_STALL);
}

long ivp_dev_suspend(struct ivp_device *pdev)
{
	unsigned long irq_status = 0;
	u32 wfi;
	u32 binterrupt;
	u32 wdg_enable;

	if (!pdev) {
		ivp_err("invalid param pdev");
		return -ENODEV;
	}

	local_irq_save(irq_status);
	wfi = ivp_reg_read(pdev, IVP_REG_OFF_PWAITMODE);
	binterrupt = ivp_reg_read(pdev, IVP_REG_OFF_BINTERRUPT);
	wdg_enable = ivp_wdg_reg_read(pdev, WDG_REG_OFF_WDCONTROL);
	if (wfi == 1 && binterrupt == 0 && wdg_enable == 0) {
		ivp_hw_runstall(pdev, IVP_RUNSTALL_STALL);
		local_irq_restore(irq_status);
		return EOK;
	}
	local_irq_restore(irq_status);

	if (wfi == 1 && (binterrupt != 0 || wdg_enable != 0))
		ivp_warn("Suspend on wrong status, binterrupt=%u wdgenable=%u",
			binterrupt, wdg_enable);
	return -EINVAL;
}

void ivp_dev_resume(struct ivp_device *pdev)
{
	ivp_dbg("enter");

	if (ivp_hw_query_runstall(pdev) == IVP_RUNSTALL_RUN)
		return;

	ivp_hw_runstall(pdev, IVP_RUNSTALL_RUN);
}

int ivp_dev_keep_on_wdg(struct ivp_device *pdev)
{
	if (!pdev) {
		ivp_err("input param pdev is null");
		return -ENODEV;
	}

	if (down_interruptible(&pdev->wdg_sem)) {
		ivp_info("interrupt");
		return -EINTR;
	}

	if (atomic_read(&pdev->wdg_sleep)) {
		ivp_info("watchdog sleeped");
		return -EAGAIN;
	}

	return 0;
}

void ivp_dev_sleep_wdg(struct ivp_device *pdev)
{
	if (!pdev) {
		ivp_err("input param pdev is null");
		return;
	}

	atomic_set(&pdev->wdg_sleep, 1);
	up(&pdev->wdg_sem);
}

int ivp_dev_smmu_reset(struct ivp_device *pdev)
{
	unsigned int status = 0;
	int ret = 0;
	int count = SMMU_RESET_WAIT_TIME;

	ivp_info("enter");
	ivp_reg_write(pdev, IVP_REG_OFF_SMMU_RST_EN,
			BIT_SMMU_CRST_EN | BIT_SMMU_BRST_EN);
	udelay(10);
	ivp_reg_write(pdev, IVP_REG_OFF_SMMU_RST_DIS,
			BIT_SMMU_CRST_DIS | BIT_SMMU_BRST_DIS);

	while (count--) {
		status = ivp_reg_read(pdev, IVP_REG_OFF_SMMU_RST_ST);
		if ((status & BIT_SMMU_CRST_DIS) &&
				(status & BIT_SMMU_BRST_DIS))
			break;
		udelay(1);
	}

	if (count <= 0) {
		ret = -1;
		ivp_err("Reset smmu fail.");
	}

	return ret;
}

int ivp_dev_smmu_init(struct ivp_device *pdev)
{
	int ret = 0;
	struct ivp_smmu_dev *smmu_dev = NULL;
	if (!pdev) {
		ivp_err("input param pdev is null");
		return -ENODEV;
	}
	smmu_dev = pdev->smmu_dev;

	if (!smmu_dev) {
		ivp_err("input param smmu dev is null");
		return -ENODEV;
	}

	ivp_info("enter");
	//reset smmu

#ifdef CONFIG_IVP_SMMU
	ret = ivp_dev_smmu_reset(pdev);
	if (ret)
		ivp_warn("Reset ret [%d]", ret);
	ret = ivp_smmu_trans_enable(smmu_dev);
#endif
	//enable smmu
	if (ret)
		ivp_warn("Enable trans ret [%d]", ret);
	return ret;
}

int ivp_dev_smmu_deinit(struct ivp_device *pdev)
{
	int ret = 0;
	struct ivp_smmu_dev *smmu_dev = NULL;
	if (!pdev) {
		ivp_err("input param pdev is null");
		return -ENODEV;
	}
	smmu_dev = pdev->smmu_dev;

	if (!smmu_dev) {
		ivp_err("input param smmu dev is null");
		return -ENODEV;
	}

	ivp_info("enter");
	ret = ivp_smmu_trans_disable(smmu_dev);
	if (ret)
		ivp_err("Enable trans failed.[%d]", ret);

	return ret;
}

#ifdef CONFIG_IVP_SMMU
int ivp_dev_smmu_invalid_tlb(struct ivp_device *pdev)
{
	int ret = 0;
	struct ivp_smmu_dev *smmu_dev = NULL;
	if (!pdev) {
		ivp_err("input param pdev is null");
		return -ENODEV;
	}
	smmu_dev = pdev->smmu_dev;

	if (!smmu_dev) {
		ivp_err("input param smmu dev is null");
		return -ENODEV;
	}
	ivp_info("enter");

	ret = ivp_smmu_invalid_tlb(smmu_dev, IVP_SMMU_CB_VMID_NS);
	if (ret)
		ivp_err("invalid tbl fail.[%d]", ret);

	return ret;
}
#endif

irqreturn_t ivp_wdg_irq_handler(int irq, void *dev_id)
{
	struct ivp_device *pdev = (struct ivp_device *)dev_id;

	ivp_warn("=========WDG IRQ Trigger=============");
	//Clear IRQ
	ivp_hw_clr_wdg_irq(pdev);

	up(&pdev->wdg_sem);
	ivp_warn("=========WDG IRQ LEAVE===============");

	ivp_dsm_error_notify(DSM_IVP_WATCH_ERROR_NO);

	return IRQ_HANDLED;
}

static void ivp_parse_dwaxi_info(struct ivp_device *pdev)
{
	u32 val, bits_val;
	u32 offset;

	offset = PCTRL_REG_OFF_PERI_STAT4;
	val = ivp_pctrl_reg_read(pdev, offset);

	ivp_dbg("pctrl reg:%#x = %#x", offset, val);

	bits_val = (val & BIT(18)) >> 18;
	if (bits_val == 0)
		ivp_warn("BIT[18] : %#x", bits_val);
	else
		ivp_warn("BIT[18] : %#x", bits_val);

	bits_val = (val & BIT(17)) >> 17;
	if (bits_val == 0)
		ivp_warn("BIT[17] : %#x, dlock write", bits_val);
	else
		ivp_warn("BIT[17] : %#x, dlock read", bits_val);

	bits_val = (val & 0x1e000) >> 13;
	ivp_warn("ivp32 dlock id[%#x]", bits_val);

	bits_val = (val & 0x1c00) >> 10;
	ivp_warn("ivp32 dlock slv[%#x]", bits_val);

	offset = IVP_REG_OFF_SMMU_RST_ST;
	val = ivp_reg_read(pdev, offset);
	ivp_warn("ivp reg:%#x = %#x", offset, val);

	offset = IVP_REG_OFF_TIMER_WDG_RST_STATUS;
	val = ivp_reg_read(pdev, offset);
	ivp_warn("ivp:%#x = %#x", offset, val);

	offset = IVP_REG_OFF_DSP_CORE_RESET_STATUS;
	val = ivp_reg_read(pdev, offset);
	ivp_warn("ivp:%#x = %#x", offset, val);
}

irqreturn_t ivp_dwaxi_irq_handler(int irq, void *dev_id)
{
	struct ivp_device *pdev = (struct ivp_device *)dev_id;
	if (!pdev) {
		ivp_err("invalid param pdev");
		return IRQ_NONE;
	}

	ivp_warn("==========DWAXI IRQ Trigger===============");
	ivp_warn("dwaxi triggled!SMMU maybe in reset status");
	//clear dwaxi irq
	ivp_gic_reg_write(pdev,
			GIC_REG_OFF_GICD_ICENABLERN, 0x80000);
	ivp_parse_dwaxi_info(pdev);
	ivp_warn("==========DWAXI IRQ LEAVE=================");

	ivp_dsm_error_notify(DSM_IVP_DWAXI_ERROR_NO);

	return IRQ_HANDLED;
}

#ifdef CONFIG_OF
static inline int ivp_setup_one_onchipmem_section(
	struct ivp_sect_info *sect, struct device_node *np)
{
	const char *name = of_node_full_name(np);
	unsigned int settings[3] = {0};
	unsigned int len = strlen(name);
	errno_t ret;

	if (of_property_read_u32_array(np, OF_IVP_SECTION_NAME,
				settings, ARRAY_SIZE(settings))) {
		ivp_err("read reg fail.");
		return -EINVAL;
	}

	len = (len >= sizeof(sect->name)) ? (sizeof(sect->name) - 1) : len;
	ret = strncpy_s(sect->name, (sizeof(sect->name) - 1), name, len);
	if (ret != EOK) {
		ivp_err("strncpy_s fail, ret [%d]", ret);
		return -EINVAL;
	}
	sect->name[len] = '\0';
	sect->ivp_addr  = settings[0];
	sect->acpu_addr = settings[1];
	sect->len       = settings[2];

	return 0;
}

int ivp_setup_onchipmem_sections(
	struct platform_device *plat_devp, struct ivp_device *pdev)
{
	struct device_node *parrent = NULL, *child = NULL;
	size_t i = 0;
	size_t sects_size = 0;
	errno_t ret;

	if (!plat_devp || !pdev) {
		ivp_err("pointer is NULL.");
		return -EINVAL;
	}

	parrent = of_get_child_by_name(
		plat_devp->dev.of_node, OF_IVP_SECTION_NODE_NAME);

	if (!parrent) {
		ivp_err("Failed to get mem parrent node.");
		return -ENODEV;
	}

	pdev->sect_count = of_get_child_count(parrent);
	sects_size = sizeof(struct ivp_sect_info) * pdev->sect_count;
	pdev->sects = (struct ivp_sect_info *)kzalloc(sects_size, GFP_KERNEL);
	if (!pdev->sects) {
		ivp_err("kmalloc sects fail.");
		return -ENOMEM;
	}

	pdev->sec_sects = (struct ivp_sect_info *)kzalloc(sects_size, GFP_KERNEL);
	if (!pdev->sec_sects) {
		ivp_err("kmalloc sec_sects fail.");
		goto err_sects;
	}

	ivp_info("section count:%d.", pdev->sect_count);

	for_each_child_of_node(parrent, child) {
		if (ivp_setup_one_onchipmem_section(&pdev->sects[i], child)) {
			ivp_err("setup %lu section fail", i);
			goto err_out;
		}

		if (!strncmp(pdev->sects[i].name, OF_IVP_DDR_MEM_NAME, sizeof(OF_IVP_DDR_MEM_NAME)) ||
		    !strncmp(pdev->sects[i].name, OF_IVP_SHARE_MEM_NAME, sizeof(OF_IVP_SHARE_MEM_NAME)) ||
		    !strncmp(pdev->sects[i].name, OF_IVP_LOG_MEM_NAME, sizeof(OF_IVP_LOG_MEM_NAME))) {
			pdev->ivp_meminddr_len += pdev->sects[i].len;
		}
		i++;
	}
	ret = memcpy_s(pdev->sec_sects, sects_size, pdev->sects, sects_size);
	if (ret != EOK) {
		ivp_err("memcpy_s fail, ret [%d]", ret);
		goto err_out;
	}
	/*lint -restore*/
	return 0;

err_out:
	kfree(pdev->sec_sects);
	pdev->sec_sects = NULL;
err_sects:
	kfree(pdev->sects);
	pdev->sects = NULL;
	pdev->sect_count = 0;
	return -EFAULT;
}
#endif

int ivp_setup_irq(
	struct platform_device *plat_devp, struct ivp_device *ivp_devp)
{
	int irq = platform_get_irq(plat_devp, 0);

	if (irq < 0) {
		ivp_err("Get irq fail!");
		return -EINVAL;
	}

	ivp_devp->wdg_irq = (unsigned int)irq;
	ivp_info("Get irq: %d.", irq);

	irq = platform_get_irq(plat_devp, 1);

	if (irq < 0) {
		ivp_err("Get irq fail!");
		return -EINVAL;
	}

	ivp_devp->dwaxi_dlock_irq = (unsigned int)irq;
	ivp_info("Get irq: %d.", irq);
	return 0;
}


int ivp_setup_smmu_dev(struct ivp_device *ivp_devp)
{
	struct ivp_smmu_dev *smmu_dev = NULL;

	smmu_dev = ivp_smmu_get_device(0UL);

	if (!smmu_dev) {
		ivp_err("Failed to get ivp smmu dev!");
		return -ENODEV;
	}
	ivp_devp->smmu_dev = smmu_dev;
#ifdef CONFIG_IVP_SMMU
	ivp_devp->smmu_pgd_base = smmu_dev->pgd_base;
#endif
	return 0;
}

void ivp_release_iores(struct platform_device *plat_devp)
{
	struct ivp_device *pdev =
	     (struct ivp_device *)platform_get_drvdata(plat_devp);

	ivp_info("enter");
	if (!pdev) {
		ivp_err("pdev is invalid");
		return;
	}

#ifdef IVP_QOS_SUPPORT
	if (pdev->io_res.noc_ivp_base_addr) {
		devm_iounmap(&plat_devp->dev, pdev->io_res.noc_ivp_base_addr);
		pdev->io_res.noc_ivp_base_addr = NULL;
	}
#endif

	if (pdev->io_res.gic_base_addr) {
		devm_iounmap(&plat_devp->dev, pdev->io_res.gic_base_addr);
		pdev->io_res.gic_base_addr = NULL;
	}

	if (pdev->io_res.pericrg_base_addr) {
		devm_iounmap(&plat_devp->dev, pdev->io_res.pericrg_base_addr);
		pdev->io_res.pericrg_base_addr = NULL;
	}

	if (pdev->io_res.pctrl_base_addr) {
		devm_iounmap(&plat_devp->dev, pdev->io_res.pctrl_base_addr);
		pdev->io_res.pctrl_base_addr = NULL;
	}

	if (pdev->io_res.cfg_base_addr) {
		devm_iounmap(&plat_devp->dev, pdev->io_res.cfg_base_addr);
		pdev->io_res.cfg_base_addr = NULL;
	}
}

int ivp_init_reg_res(struct platform_device *plat_devp,
		struct ivp_device *pdev)
{
	struct resource *mem_res = NULL;

	if (!plat_devp || !pdev) {
		ivp_err("input param plat_devp or pdev is null");
		return -ENODEV;
	}

	mem_res = platform_get_resource(plat_devp,
			IORESOURCE_MEM, 0); /*lint -save -e838*/
	if (!mem_res) {
		ivp_err("Get cfg res fail!");
		goto ERR_EXIT;
	}

	pdev->io_res.cfg_base_addr = devm_ioremap(
		&plat_devp->dev, mem_res->start, resource_size(mem_res));
	if (!pdev->io_res.cfg_base_addr) {
		ivp_err("Map cfg reg failed!");
		goto ERR_EXIT;
	}

	mem_res = platform_get_resource(plat_devp, IORESOURCE_MEM, 1);
	if (!mem_res) {
		ivp_err("Get pctrl res failed!");
		goto ERR_EXIT;
	}

	pdev->io_res.pctrl_base_addr = devm_ioremap(
		&plat_devp->dev, mem_res->start, resource_size(mem_res));
	if (!pdev->io_res.pctrl_base_addr) {
		ivp_err("Map pctrl reg failed!");
		goto ERR_EXIT;
	}

	mem_res = platform_get_resource(plat_devp, IORESOURCE_MEM, 2);
	if (!mem_res) {
		ivp_err("Get preicrg res failed!");
		goto ERR_EXIT;
	}
	pdev->io_res.pericrg_base_addr = devm_ioremap(
		&plat_devp->dev, mem_res->start, resource_size(mem_res));
	if (!pdev->io_res.pericrg_base_addr) {
		ivp_err("Map pericrg res failed!");
		goto ERR_EXIT;
	}

	mem_res = platform_get_resource(plat_devp, IORESOURCE_MEM, 3);
	if (!mem_res) {
		ivp_err("Get gic res failed");
		goto ERR_EXIT;
	}

	pdev->io_res.gic_base_addr = devm_ioremap(&plat_devp->dev,
			GIC_IRQ_CLEAR_REG, 4); /*lint -save -e747*/
	if (!pdev->io_res.gic_base_addr) {
		ivp_err("Map gic res failed!");
		goto ERR_EXIT;
	}
#ifdef IVP_QOS_SUPPORT
	mem_res = platform_get_resource(plat_devp, IORESOURCE_MEM, 4);
	if (!mem_res) {
		ivp_err("Get noc_ivp res failed");
		goto ERR_EXIT;
	}

	pdev->io_res.noc_ivp_base_addr = devm_ioremap(
		&plat_devp->dev, mem_res->start,
		resource_size(mem_res)); /*lint -save -e747*/
	if (!pdev->io_res.noc_ivp_base_addr) {
		ivp_err("Map NOC_IVP32_Service_Target res failed!");
		goto ERR_EXIT;
	}
#endif
	return 0;
ERR_EXIT:
	ivp_release_iores(plat_devp);
	return -EFAULT;
}
void ivp_dump_status(struct ivp_device *pdev)
{
	unsigned int cur_pc;
	unsigned int waiti;
	unsigned int runstall;
	unsigned int binterrupt;

	if (!pdev) {
		ivp_err("input param pdev is invalid");
		return;
	}

	if (atomic_read(&pdev->poweron_success) != 0) {
		ivp_err("ivp has powered off!");
		return;
	}

	waiti = (u32)ivp_hw_query_waitmode(pdev);
	runstall = (u32)ivp_hw_query_runstall(pdev);
	binterrupt = ivp_reg_read(pdev, IVP_REG_OFF_BINTERRUPT);
	/* get pc, wfi, runstall. interrupt, or others status */
	ivp_reg_write(pdev, IVP_REG_OFF_OCDHALTONRESET, IVP_DEBUG_ENABLE);
	cur_pc = ivp_reg_read(pdev, IVP_REG_OFF_IVP_DEBUG_SIG_PC);

	ivp_err("Dsp curr pc = 0x%x, waiti = 0x%x, runstall = 0x%x, bint = 0x%x",
		cur_pc, waiti, runstall, binterrupt);
}

void *ivp_vmap(phys_addr_t paddr, size_t size, unsigned int *offset)
{
	unsigned int i;
	void *vaddr = NULL;
	pgprot_t pgprot;
	unsigned int pages_count;
	struct page **pages = NULL;

	*offset = paddr & ~PAGE_MASK;
	paddr &= PAGE_MASK;
	pages_count = PAGE_ALIGN(size + *offset) / PAGE_SIZE;

	pages = kzalloc(sizeof(struct page *) * pages_count, GFP_KERNEL);
	if (!pages)
		return NULL;

	pgprot = pgprot_writecombine(PAGE_KERNEL);

	for (i = 0; i < pages_count; i++) {
		*(pages + i) = phys_to_page(
			(phys_addr_t)(paddr + PAGE_SIZE * i));
	}

	vaddr = vmap(pages, pages_count, VM_MAP, pgprot);
	kfree(pages);
	if (!vaddr)
		return NULL;

	return (char *)vaddr + *offset;
}

bool is_ivp_in_secmode(struct ivp_device *pdev)
{
	return ((pdev->ivp_sec_support == 1) &&
		(pdev->ivp_secmode == SECURE_MODE));
}
