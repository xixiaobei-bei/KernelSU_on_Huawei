/*
 * Generic driver for the cambricon ipu device.
 *
 * Copyright (C) 2016 Cambricon Limited
 *
 * Licensed under the GPL v2 or later.
 */

#include <linux/mfd/hisi_pmic.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/pm_runtime.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/input.h>
#include <linux/wakelock.h>
#include <linux/clk.h>
#include <linux/hwspinlock.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/kfifo.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <linux/ion.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/time.h>
#include <linux/jiffies.h>

#include <dsm/dsm_pub.h>
#include "ipu_smmu_drv.h"
#include "ipu_clock.h"
#include "cambricon_ipu.h"
#include "ipu_mntn.h"

#define COMP_CAMBRICON_IPU_DRV_NAME "hisilicon,cambricon-ipu"

/* ipu base address */
#define IPU_NAME	"ipu"
#define UNUSED_PARAMETER(x) ((void)(x))

/* configure registers info */
#define IPU_CONF_REG_BASE	(IPU_BASE_ADDRESS + 0x00000000)
#define IPU_CONF_REG_SIZE	(0x00100000)
/* instruction RAM info */
#define IPU_INST_RAM_BASE	(IPU_BASE_ADDRESS + 0x00100000)
#define IPU_INST_RAM_SIZE	(0x00100000)

/* ipu configure register offset */
#define IPU_TO_STOP (0x0)
#define IPU_TO_START (0x1)
#define IPU_STATUS_UNFINISH (0x0)
#define IPU_STATUS_FINISH (0x1)

#define IPU_START_REG (0x18)	/* IPU start up reg */
#define IPU_STATUS_REG (0x20)	/* IPU payload finish status reg */
#define IPU_BASE_ADDR_REG (0x28)	/* IPU access external DDR address */
#define IPU_SRAM_CTRL_REG (0x30)	/* IPU internal SRAM configure reg */
#define IPU_VERSION_REG (0x40) /* IPU Version reg */

/* reserved DDR memory info */
#define DMA_BUFFER_START (0x20c00000)
#define DMA_BUFFER_SIZE	 (500 * 1024 * 1024)

/* Configuration Registers Operations: ioctl */
#define MAGIC_NUM	100
#define RDCONFIG_DWORD  _IOR(MAGIC_NUM, 3, unsigned int)
#define WRCONFIG_DWORD  _IOW(MAGIC_NUM, 6, unsigned int*)//_IOW(MAGIC_NUM, 6, unsigned long[2])
#define IN_TASKQUEUE    _IOW(MAGIC_NUM, 9, unsigned int*)//_IOW(MAGIC_NUM, 9, taskElement)
#define SETCONFIG_MAP	_IOW(MAGIC_NUM, 37, struct map_data)
#define SETCONFIG_UNMAP	_IOW(MAGIC_NUM, 38, struct map_data)
#define SETCONFIG_RESET_STATISTIC  _IOW(MAGIC_NUM, 30, unsigned int)
#define SETCONFIG_REPORT_STATISTIC _IOW(MAGIC_NUM, 31, struct smmu_statistic)
#define SETCONFIG_UPDATE_PTE	   _IOW(MAGIC_NUM, 32, unsigned int)
#define SETCONFIG_RESET_VIRT_ADDR  _IOW(MAGIC_NUM, 33, struct map_data)
#define SETCONFIG_IPU_PROFILE	   _IOW(MAGIC_NUM, 34, unsigned long)

#define IPU_IOC_MAXNR	(38)

#define ICS_IRQ_UNMASK_NO_SECURITY (0x00000000)
#define ICS_IRQ_MASK_NO_SECURITY (0x00000001)
#define ICS_IRQ_CLEAR_IRQ_NS (0x00000001)
#define ICS_SMMU_WR_ERR_BUFF_LEN (128)
#define ICS_NOC_BUS_CONFIG_QOS_TYPE (0x1)
#define ICS_NOC_BUS_QOS_EXTCONTROL_ENABLE (0x1)
#define ICS_NOC_BUS_CONFIG_FACTOR (0x0)
#define ICS_NOC_BUS_CONFIG_SATURATION_RESET (0x0)
#define ICS_NOC_BUS_CONFIG_SATURATION_NORMAL (0x10)
#define CONFIG_NOC_POWER_IDLEREQ_0 (0x02000200)
#define CONFIG_NOC_POWER_IDLEACK_0_BIT9 (0X200)
#define CONFIG_NOC_POWER_IDLE_0_BIT9 (0X200)
#define CONFIG_PCTRL_PERI_STAT3_BIT22 (0x400000)
#define CONFIG_MEDIA2_REG_PERDIS0_ICS (0x7)
#define CONFIG_MEDIA2_REG_PEREN0_ICS (0x7)
#define CONFIG_MEDIA2_REG_PERRSTEN0_ICS (0x38)
#define CONFIG_SC_GT_CLK_ICS_DIS (0x40000000)
#define CONFIG_SC_GT_CLK_ICS_EN (0x40004000)
#define CONFIG_NOC_ICS_POWER_IDLEREQ_DIS (0x02000000)
#define TASKQUEUE_SIZE (64)
#define NORMAL_START_BIT (0)
#define NORMAL_START_MASK (0x1 << NORMAL_START_BIT)

#define WATCHDOG_TIMEOUT_THD_MS (3000)

#define SMMU_OUTSTANDING		(4)
#define IPU_BASELINE_RATE_MHZ	(960)
#define IPU_BASELINE_LMT_OSD	(32)
#define IPU_BASELINE_LMT_BANDWIDTH_MHZ (7500)
#define IPU_RATE_HZ_TO_MHZ		(1000000)
#define IPU_STATUS_REG_FINISH	(0xA0)
#define	IPU_INTERRUPT_INST_REG	(0xAC)
#define ICS_IRQ_CLEAR_IRQ_LEVEL1_NS (0x2)
#define ICS_SOFT_RST_REQ_REG	(0x74)
#define ICS_SOFT_RST_ACK_REG	(0x78)
#define ICS_SOFT_RST_ACK		(0x1)
#define IPU_ID_REG 				(0x40000)
#define	IPU_CONTROL_ID_REG		(0x200)
#define IPU_WATCH_DOG_REG		(0x20C)
#define PERF_CNT_CLEAR_REG		(0x84)
#define PERF_CNT_CLK_GT_REG		(0x88)
#define FRAME_CYC_CNT_REG		(0x8C)
#define FU_IDLE_CNT_REG			(0x90)
#define IO_IDLE_CNT_REG			(0x94)
#define ALL_IDLE_CNT_REG		(0x98)
#define ALL_BUSY_CNT_REG		(0x9C)
#define ICS_MAX_OSD_REG			(0xA0)
#define ICS_OSD_CNT_REG			(0xA4)
#define ICS_FRAME_CNT_REG		(0xA8)
#define FRAME_CNT_CLEAR_REG		(0xAC)
#define PERF_CNT_CLEAR			(0x1)
#define FRAME_START_CNT_CLEAR 	(0x1)
#define FRAME_FINISH_CNT_CLEAR	(0x10)
#define PERF_CNT_CLK_GT_CLOSE	(0x0)
#define PERF_CNT_CLK_GT_ENABLE	(0x1)
#define INTERRUPTINST_INTERRUPT_BIT		(7)
#define INTERRUPTINST_INTERRUPT_MASK	(0x1UL<<INTERRUPTINST_INTERRUPT_BIT)
#define CT_INTERRUPT_BIT				(6)
#define CT_INTERRUPT_MASK				(0x1UL<<CT_INTERRUPT_BIT)
#define IPU_CONTROL_INTERRUPT_BIT		(5)
#define IPU_CONTROL_INTERRUPT_MASK		(0x1UL<<IPU_CONTROL_INTERRUPT_BIT)
#define WATCH_DOG_BIT					(4)
#define WATCH_DOG_MASK					(0x1UL<<WATCH_DOG_BIT)
#define IS_NORMAL_FINISH_BIT			(0)
#define IS_NORMAL_FINISH_MASK			(0x1UL<<IS_NORMAL_FINISH_BIT)

/*check whether the parameter is effective*/
#define CHECK_TASK_TYPE(taskType) \
({	\
	unsigned int __ret = 0;	\
	if((taskType-IPU_TASK_ENUM_START)>(IPU_TASK_ENUM_END - IPU_TASK_ENUM_START - 1))		\
		__ret = 1;	\
	__ret;	\
})	\

typedef enum {
    DEV_UNSET_PROFILE = 0,
    DEV_LOW_PROFILE,
    DEV_NORMAL_PROFILE,
    DEV_HIGH_PROFILE,
} dev_perf;

enum ipu_wtd_status {
	IPU_WTD_UNINIT = 0,
	IPU_WTD_IDLE,
	IPU_WTD_RUNNING,
};

/* global variables */
static unsigned int ipu_major = 0;
static unsigned int ipu_minor = 0;
struct cambricon_ipu_private *adapter = NULL;
static struct class *dev_class = NULL;

/* DSM information about ai module */
static struct dsm_dev dsm_ai = {
	.name           = "dsm_ai",
	.device_name    = "ai",
	.ic_name        = NULL,
	.module_name    = NULL,
	.fops           = NULL,
	.buff_size      = 1024,
};

/* AI client handle for DSM */
static struct dsm_client *dsm_ai_client = NULL;

/* Register ai client to DSM, if register fail, continue and does't return */
#define DSM_AI_REGISTER()\
do {\
	if (NULL == dsm_ai_client)\
	{\
		dsm_ai_client = dsm_register_client(&dsm_ai);\
	}\
} while(0)

/* Unregister ai client to DSM */
#define DSM_AI_UNREGISTER()\
do {\
	if (NULL != dsm_ai_client)\
	{\
		dsm_unregister_client(dsm_ai_client, &dsm_ai);\
		dsm_ai_client = NULL;\
	}\
} while(0)

/* Report to DSM */
#define DSM_AI_KERN_ERROR_REPORT(error_no, fmt, args...)\
do {\
	if (NULL != dsm_ai_client && !dsm_client_ocuppy(dsm_ai_client))\
	{\
		dsm_client_record(dsm_ai_client, fmt, ##args);\
		dsm_client_notify(dsm_ai_client, error_no);\
	}\
} while(0)


struct ioctl_out_params {
	bool ret_directly;
	void *memory_node;
	//TODO: add more out params here
};

typedef long (*ioctl_cb)(struct cambricon_ipu_private *, unsigned long, struct ioctl_out_params *);

/* IOCTL map */
struct ipu_ioctl_map {
	unsigned int cmd;
	ioctl_cb func;
};

/* FIFO_TaskElements which holds ipu tasks */
static DECLARE_KFIFO_PTR(FIFO_TaskElements, taskElement);
/* wait queue holds sync task which make user process sleep */
wait_queue_head_t sync_wq;

/* ipu char device ops declaration */
static int ipu_open(struct inode *inode, struct file *filp);
static int ipu_release(struct inode *inode, struct file *filp);
static ssize_t ipu_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static long ipu_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static long ipu_ioctl32(struct file *fd, unsigned int cmd, unsigned long arg);
static loff_t ipu_llseek(struct file *filp, loff_t off, int whence);
static long ipu_write_dword(struct cambricon_ipu_private *, unsigned long, struct ioctl_out_params *);
static long ipu_set_map(struct cambricon_ipu_private *, unsigned long, struct ioctl_out_params *);
static long ipu_set_unmap(struct cambricon_ipu_private *, unsigned long, struct ioctl_out_params *);
static long ipu_set_reset_virt_addr(struct cambricon_ipu_private *, unsigned long, struct ioctl_out_params *);
static long ipu_set_reset_statistic(struct cambricon_ipu_private *, unsigned long, struct ioctl_out_params *);
static long ipu_set_report_statistic(struct cambricon_ipu_private *, unsigned long, struct ioctl_out_params *);
static long ipu_set_update_pte(struct cambricon_ipu_private *, unsigned long, struct ioctl_out_params *);
static long ipu_process_workqueue(struct cambricon_ipu_private *, unsigned long, struct ioctl_out_params *);
static long ipu_read_dword(struct cambricon_ipu_private *, unsigned long, struct ioctl_out_params *);
static int ipu_power_up(void);
static int ipu_power_down(void);
long ipu_set_profile(struct cambricon_ipu_private *, unsigned long, struct ioctl_out_params *);
static bool cambricon_ipu_workqueue(void);
static void performance_monitor_open(void);

static const struct ipu_ioctl_map ipu_ioctl_maps[] = {
	{RDCONFIG_DWORD,			ipu_read_dword},
	{WRCONFIG_DWORD,			ipu_write_dword},
	{SETCONFIG_MAP,				ipu_set_map},
	{SETCONFIG_UNMAP,			ipu_set_unmap},
	{SETCONFIG_RESET_VIRT_ADDR,	ipu_set_reset_virt_addr},
	{SETCONFIG_RESET_STATISTIC,	ipu_set_reset_statistic},
	{SETCONFIG_REPORT_STATISTIC,ipu_set_report_statistic},
	{SETCONFIG_UPDATE_PTE,		ipu_set_update_pte},
	{IN_TASKQUEUE,				ipu_process_workqueue},
	{SETCONFIG_IPU_PROFILE,		ipu_set_profile},
};

static void ipu_watchdog_init(struct ipu_wtd *watchdog, bool enable, void *callback)
{
	watchdog->enable = enable;
	watchdog->status = IPU_WTD_IDLE;

	if (enable) {
		mutex_init(&watchdog->timer_mutex);
		setup_timer(&watchdog->timer, callback, 0);
	}
}

static void ipu_watchdog_start(struct ipu_wtd *watchdog, unsigned long watchdog_length_ms)
{
	if (!watchdog->enable) {
		return;
	}

	mutex_lock(&watchdog->timer_mutex);

	if (IPU_WTD_RUNNING == watchdog->status) {
		/* here use del_timer, not use del_timer_sync, because mutex can guarantee the wtd is run serially */
		del_timer(&watchdog->timer);
	}

	watchdog->timer.expires = jiffies + msecs_to_jiffies(watchdog_length_ms);
	add_timer(&watchdog->timer);
	watchdog->status = IPU_WTD_RUNNING;
	mutex_unlock(&watchdog->timer_mutex);
}

static void ipu_watchdog_stop(struct ipu_wtd *watchdog)
{
	if (!watchdog->enable) {
		return;
	}

	mutex_lock(&watchdog->timer_mutex);

	if (IPU_WTD_RUNNING == watchdog->status)	{
		/* here use del_timer, not use del_timer_sync, because mutex can guarantee the wtd is run serially */
		del_timer(&watchdog->timer);
		watchdog->status = IPU_WTD_IDLE;
	}
	mutex_unlock(&watchdog->timer_mutex);
}

int set_offchip_inst_addr(unsigned long addr) {
	unsigned int i;
	unsigned int tmp;
	unsigned long adjusted_addr;
	unsigned int ipu_base_reg;

	/* write offchip inst addr to instram */
	uint32_t inst_addr = (uint32_t) (addr + 384);
	inst_addr = (inst_addr | 0x80000000) >> 1;
	ipu_base_reg = ioread32((void *)((unsigned long)adapter->config_reg_virt_addr + IPU_BASE_ADDR_REG));
	adjusted_addr = ((ipu_base_reg & 0xfff) << 20) + addr;

	if (ipu_mem_mngr_check_valid((unsigned int)adjusted_addr)) {
		printk(KERN_ERR"[%s]: IPU_ERROR:fail to seek in malloc memory, input addr=0x%pK ipu_base_reg=%u, adjusted_addr=0x%pK \n",
			__func__, (void *)(unsigned long)addr, ipu_base_reg, (void *)adjusted_addr);
		ipu_mem_mngr_dump();
		return -EINVAL;
	}

	if(down_interruptible(&adapter->inst_ram_sem)) {
		printk(KERN_ERR"[%s]:IPU_ERROR: down_interruptible inst_ram_sem fail!\n", __func__);
		return -ERESTARTSYS;
	}

	/* write boot inst to instram */
	for (i = 0; i < adapter->boot_inst_set.boot_inst_size; i += 4) {
		tmp = *(unsigned int *)&adapter->boot_inst_set.boot_inst[i];
		iowrite32(tmp, (void *)((unsigned long)adapter->inst_ram_virt_addr + i));
	}

	iowrite8((inst_addr&0x000000ff)    , (void *)((unsigned long)adapter->inst_ram_virt_addr + 53));
	iowrite8((inst_addr&0x0000ff00)>>8 , (void *)((unsigned long)adapter->inst_ram_virt_addr + 54));
	iowrite8((inst_addr&0x00ff0000)>>16, (void *)((unsigned long)adapter->inst_ram_virt_addr + 55));
	iowrite8((inst_addr&0xff000000)>>24, (void *)((unsigned long)adapter->inst_ram_virt_addr + 56));

	up(&adapter->inst_ram_sem);

	return 0;
}

static int start_ipu(void)
{
	int ret;

	/* power up ipu if ipu is power-off */
	mutex_lock(&adapter->boot_inst_set.boot_mutex);

	if (!adapter->ipu_power_up) {
		if (!adapter->boot_inst_set.access_ddr_addr_is_config) {
			printk(KERN_ERR"[%s]: IPU_ERROR:boot_inst_set.access_ddr_addr_is_config is not configed, FATAL\n", __func__);
			mutex_unlock(&adapter->boot_inst_set.boot_mutex);
			return -EFAULT;
		}

		if (!adapter->boot_inst_set.boot_inst_recorded_is_config) {
			printk(KERN_ERR"[%s]: IPU_ERROR:boot_inst_set is not configed, check whether [ipu_write] is called, FATAL\n", __func__);
			mutex_unlock(&adapter->boot_inst_set.boot_mutex);
			return -EFAULT;
		}

		if ((0 == adapter->boot_inst_set.boot_inst_size) || (0 != (adapter->boot_inst_set.boot_inst_size % 4))) {
			printk(KERN_ERR"[%s]: IPU_ERROR:invalid boot inst size=0x%x, FATAL\n", __func__, adapter->boot_inst_set.boot_inst_size);
			mutex_unlock(&adapter->boot_inst_set.boot_mutex);
			return -EFAULT;
		}

		ret = ipu_power_up();
		if (ret) {
			printk(KERN_ERR"[%s]: IPU_ERROR:ipu_power_up fail\n", __func__);
			/* IPU power up fail, report to DSM */
			DSM_AI_KERN_ERROR_REPORT(DSM_AI_KERN_POWER_UP_ERR_NO, "IPU power up fail, ttbr0=%x, ret=%d.\n", adapter->smmu_ttbr0, ret);
			mutex_unlock(&adapter->boot_inst_set.boot_mutex);
			return -EBUSY;
		}

		/* set base address */
		iowrite32(adapter->boot_inst_set.ipu_access_ddr_addr, (void *)((unsigned long)adapter->config_reg_virt_addr + IPU_BASE_ADDR_REG));
	}

	/* prepare data for ipu */
	if(IPU_TO_START!= ioread32((void *)((unsigned long)adapter->config_reg_virt_addr + IPU_START_REG))) {
		taskElement head;
		if (kfifo_peek(&FIFO_TaskElements, &head)) {
			ret = set_offchip_inst_addr(head.offchipInstAddr);
			if (0 == ret) {
				/* update pte */
				ipu_smmu_pte_update();
				/* start ipu */
				iowrite32(IPU_TO_STOP, (void *)((unsigned long)adapter->config_reg_virt_addr + IPU_START_REG));
				iowrite32(IPU_STATUS_UNFINISH, (void *)((unsigned long)adapter->config_reg_virt_addr + IPU_STATUS_REG));

				if (adapter->feature_tree.performance_monitor) {
					performance_monitor_open();
				}

				iowrite32(IPU_TO_START, (void *)((unsigned long)adapter->config_reg_virt_addr + IPU_START_REG));

				printk(KERN_DEBUG"[%s]: START COMPUTE, offchipInstAddr=0x%pK\n", __func__, (void *)head.offchipInstAddr);

				ipu_watchdog_start(&adapter->reset_wtd, WATCHDOG_TIMEOUT_THD_MS);
			} else {
				printk(KERN_ERR"[%s]: IPU_ERROR:set_offchip_inst_addr fail, ret=%d\n", __func__, ret);
				mutex_unlock(&adapter->boot_inst_set.boot_mutex);
				return ret;
			}
		} else {
			printk(KERN_ERR"[%s]: IPU_ERROR:kfifo_peek empty\n", __func__);
		}
	}

	/* workqueue task_fifo_sem is not up here */
	mutex_unlock(&adapter->boot_inst_set.boot_mutex);
	return 0;
}

static void start_ipu_workqueue(void)
{
	while (start_ipu()) {
		printk(KERN_ERR"[%s]: IPU_ERROR:start ipu fail, FATAL\n", __func__);
		if (!cambricon_ipu_workqueue()) {
			printk(KERN_ERR"[%s]: IPU_ERROR:cambricon_ipu_workqueue false when kill head task\n", __func__);
		}

		if (kfifo_is_empty(&FIFO_TaskElements)) {
			break;
		}
	}
}

static int stop_ipu(void)
{
	int ret = 0;

	if (adapter->ipu_power_up) {
		ret = ipu_power_down();
	}

	return ret;
}

/* global variable declaration */
static const struct file_operations ipu_fops = {
	.owner			= THIS_MODULE,/*lint !e64*/
	.open			= ipu_open,
	.release		= ipu_release,
	.write			= ipu_write,
	.unlocked_ioctl = ipu_ioctl,
	.compat_ioctl	= ipu_ioctl32,
	.llseek 		= ipu_llseek,
};/*lint !e785*/
/* ipu platform device */
static struct platform_device cambricon_ipu_device = {
	.name	= "cambricon-ipu",
	.id	= -1,
};/*lint !e785*/

bool ipu_get_irq_offset (struct device *dev)
{
	int property_rd;
	struct irq_reg_offset *offset = &adapter->irq_reg_offset;
	struct device_node *node = of_find_node_by_name(dev->of_node, "ics_irq");
	if(!node) {
		printk(KERN_ERR"[%s]: IPU_ERROR:find ics_irq node error\n", __func__);
		return false;
	}
	memset(offset, 0, sizeof(*offset));// coverity[secure_coding]
	property_rd = of_property_read_u32(node, "ics-irq-base-addr", &offset->ics_irq_base_addr);
	property_rd |= of_property_read_u32(node, "ics-irq-mask-ns", &offset->ics_irq_mask_ns);
	property_rd |= of_property_read_u32(node, "ics-irq-clr-ns", &offset->ics_irq_clr_ns);
	if (property_rd) {
		printk(KERN_ERR"[%s]: IPU_ERROR:read property of irq offset error\n", __func__);
		return false;
	}

	return true;
}

bool ipu_get_bandwidth_lmt_offset (struct device *dev)
{
	int property_rd;
	struct ics_noc_bus_reg_offset *ics_noc_bus = &adapter->ics_noc_bus_reg_offset;
	struct device_node *node = of_find_node_by_name(dev->of_node, "ics_noc_bus");

	if(!node) {
		printk(KERN_ERR"[%s]: IPU_ERROR:find ics_noc_bus node error\n", __func__);
		return false;
	}

	property_rd = of_property_read_u32(node, "base-addr", &ics_noc_bus->base_addr);
	property_rd |= of_property_read_u32(node, "qos-type", &ics_noc_bus->qos_type);
	property_rd |= of_property_read_u32(node, "factor", &ics_noc_bus->factor);
	property_rd |= of_property_read_u32(node, "saturation", &ics_noc_bus->saturation);
	property_rd |= of_property_read_u32(node, "qos_extcontrol", &ics_noc_bus->qos_extcontrol);
	if (property_rd) {
		printk(KERN_ERR"[%s]: IPU_ERROR:read property of ics_noc_bus offset error\n", __func__);
		return false;
	}

	return true;
}

bool ipu_get_reset_offset (struct device *dev)
{
	int property_rd;
	struct pmctrl_reg_offset *pmctrl = &adapter->pmctrl_reg_offset;
	struct pctrl_reg_offset *pctrl = &adapter->pctrl_reg_offset;
	struct media2_reg_offset *media2 = &adapter->media2_reg_offset;
	struct peri_reg_offset *peri = &adapter->peri_reg_offset;
	struct device_node *node = of_find_node_by_name(dev->of_node, "pmctrl");
	if(!node) {
		printk(KERN_ERR"[%s]: IPU_ERROR:find pmctrl node error\n", __func__);
		return false;
	}
	memset(pmctrl, 0, sizeof(*pmctrl));
	property_rd = of_property_read_u32(node, "base-addr", &pmctrl->base_addr);
	property_rd |= of_property_read_u32(node, "noc-power-idle-req", &pmctrl->noc_power_idle_req);
	property_rd |= of_property_read_u32(node, "noc-power-idle-ack", &pmctrl->noc_power_idle_ack);
	property_rd |= of_property_read_u32(node, "noc-power-idle-stat", &pmctrl->noc_power_idle_stat);
	if (property_rd) {
		printk(KERN_ERR"[%s]: IPU_ERROR:read property of pmctrl offset error\n", __func__);
		return false;
	}

	node = of_find_node_by_name(dev->of_node, "pctrl");
	if(!node) {
		printk(KERN_ERR"[%s]: IPU_ERROR:find pctrl node error\n", __func__);
		return false;
	}
	memset(pctrl, 0, sizeof(*pctrl));
	property_rd = of_property_read_u32(node, "base-addr", &pctrl->base_addr);
	property_rd |= of_property_read_u32(node, "peri-stat3", &pctrl->peri_stat3);
	if (property_rd) {
		printk(KERN_ERR"[%s]: IPU_ERROR:read property of pctrl offset error\n", __func__);
		return false;
	}

	node = of_find_node_by_name(dev->of_node, "media2");
	if(!node) {
		printk(KERN_ERR"[%s]: IPU_ERROR:find media2 node error\n", __func__);
		return false;
	}
	memset(media2, 0, sizeof(*media2));
	property_rd = of_property_read_u32(node, "base-addr", &media2->base_addr);
	property_rd |= of_property_read_u32(node, "peren0", &media2->peren0);
	property_rd |= of_property_read_u32(node, "perdis0", &media2->perdis0);
	property_rd |= of_property_read_u32(node, "perclken0", &media2->perclken0);;
	property_rd |= of_property_read_u32(node, "perstat0", &media2->perstat0);
	property_rd |= of_property_read_u32(node, "perrsten0", &media2->perrsten0);
	property_rd |= of_property_read_u32(node, "perrstdis0", &media2->perrstdis0);
	property_rd |= of_property_read_u32(node, "perrststat0", &media2->perrststat0);
	if (property_rd) {
		printk(KERN_ERR"[%s]: IPU_ERROR:read property of media2 offset error\n", __func__);
		return false;
	}

	node = of_find_node_by_name(dev->of_node, "peri");
	if(!node) {
		printk(KERN_ERR"[%s]: IPU_ERROR:find peri node error\n", __func__);
		return false;
	}
	memset(peri, 0, sizeof(*peri));// coverity[secure_coding]
	property_rd = of_property_read_u32(node, "base-addr", &peri->base_addr);
	property_rd |= of_property_read_u32(node, "clkdiv18", &peri->clkdiv18);
	property_rd |= of_property_read_u32(node, "clkdiv8", &peri->clkdiv8);
	property_rd |= of_property_read_u32(node, "perpwrstat", &peri->perpwrstat);
	property_rd |= of_property_read_u32(node, "perpwrack", &peri->perpwrack);
	property_rd |= of_property_read_u32(node, "peristat7", &peri->peristat7);
	if (property_rd) {
		printk(KERN_ERR"[%s]: IPU_ERROR:read property of crg_periph offset error\n", __func__);
		return false;
	}

	return true;
}

bool ipu_get_feature_tree (struct device *dev)
{
	UNUSED_PARAMETER(dev);

	memset(&adapter->feature_tree, 0, sizeof(adapter->feature_tree));// coverity[secure_coding]

	adapter->feature_tree.finish_irq_expand_ns = true;
	adapter->feature_tree.finish_irq_expand_p = true;
	adapter->feature_tree.finish_irq_expand_s = true;
	adapter->feature_tree.finish_irq_to_hifi = true;
	adapter->feature_tree.finish_irq_to_ivp = true;
	adapter->feature_tree.finish_irq_to_isp = true;
	adapter->feature_tree.finish_irq_to_lpm3 = true;
	adapter->feature_tree.finish_irq_to_iocmu = true;
	adapter->feature_tree.smmu_port_select = false;
	adapter->feature_tree.soft_watchdog_enable = true;
	adapter->feature_tree.ipu_reset_when_in_error = IPU_RESET_BY_CONFIG_NOC_BUS;
	adapter->feature_tree.ipu_bandwidth_lmt = IPU_BANDWIDTH_LMT_BY_QOS;

	return true;
}

int regulator_ip_vipu_enable(void)
{
	int ret;

	ret = regulator_is_enabled(adapter->vipu_ip);
	if (ret) {
		printk(KERN_ERR"[%s]:IPU_ERROR:regulator_is_enabled: %d\n", __func__, ret);
		rdr_system_error((unsigned int)MODID_NPU_EXC_SET_POWER_UP_STATUS_FAULT, 0, 0);
		return -EBUSY;
	}

	ret = regulator_enable(adapter->vipu_ip);
	if (0 != ret) {
		printk(KERN_ERR"[%s]:IPU_ERROR:Failed to enable: %d\n", __func__, ret);
		rdr_system_error((unsigned int)MODID_NPU_EXC_SET_POWER_UP_FAIL, 0, 0);
		return ret;
	}
	return 0;
}

int regulator_ip_vipu_disable(void)
{
	int ret;

	ret = regulator_disable(adapter->vipu_ip);
	if (ret != 0) {
		printk(KERN_ERR"[%s]:IPU_ERROR:Failed to disable: %d\n", __func__, ret);
		rdr_system_error((unsigned int)MODID_NPU_EXC_SET_POWER_DOWN_FAIL, 0, 0);
		return ret;
	}
	return 0;
}

int ipu_bandwidth_lmt_ioremap_addr(void)
{
	adapter->noc_bus_io_addr = ioremap((unsigned long)adapter->ics_noc_bus_reg_offset.base_addr, (unsigned long)0xff);
	if (!adapter->noc_bus_io_addr) {
		printk(KERN_ERR"[%s]:IPU_ERROR:noc_bus_io_addr ioremap fail\n", __func__);
		return -ENOMEM;
	}

	return 0;
}

void ipu_bandwidth_lmt_unremap_addr(void)
{
	if (adapter->noc_bus_io_addr) {
		iounmap(adapter->noc_bus_io_addr);
		adapter->noc_bus_io_addr = NULL;
	}
}

int ipu_bandwidth_lmt_init(struct device *dev)
{
	int ret;

	/* ioremap reg addr for reset */
	if (!(IPU_RESET_UNSUPPORT == adapter->feature_tree.ipu_reset_when_in_error &&
		IPU_BANDWIDTH_LMT_UNSUPPORT == adapter->feature_tree.ipu_bandwidth_lmt)) {

		if (!ipu_get_bandwidth_lmt_offset(dev)) {
			printk(KERN_ERR"[%s]: fatal err, error bandwidth lmt reg offset\n", __func__);
			return -ENOENT;
		}

		ret = ipu_bandwidth_lmt_ioremap_addr();
		if (ret) {
			printk(KERN_ERR"[%s]: fatal err, ipu_bandwidth_lmt_ioremap_addr\n", __func__);
			return ret;
		}
	}

	return 0;
}

/* IPU bandwidth lmt, to avoid block video work */
static unsigned int ipu_bandwidth_get_lmt(unsigned int ipu_rate, unsigned int bandwidth_lmt_trategy)
{
	unsigned int noc_bandwidth_lmt;
	unsigned int vcodec_rate;
	unsigned int bandwidth;
	unsigned int ipu_rate_MHz;
	unsigned int read_outstanding;
	unsigned int write_outstanding;
	unsigned int ret;

	if (ipu_rate == adapter->clk.ipu_high) {
		vcodec_rate = adapter->clk.vcodecbus_high;
	} else if (ipu_rate == adapter->clk.ipu_middle) {
		vcodec_rate = adapter->clk.vcodecbus_middle;
	} else if (ipu_rate == adapter->clk.ipu_low) {
		vcodec_rate = adapter->clk.vcodecbus_low;
	} else {
		printk(KERN_ERR"[%s]: IPU_ERROR:error ipu_rate=%u, ignore\n", __func__, ipu_rate);
		return 0;
	}

	ipu_rate_MHz = ipu_rate / IPU_RATE_HZ_TO_MHZ;

	if (IPU_BANDWIDTH_LMT_BY_RW_OSD == bandwidth_lmt_trategy) {
		/* for ipu_rate=960M, assume read/write osd is 32 */
		read_outstanding = ((IPU_BASELINE_LMT_OSD * ipu_rate_MHz) / IPU_BASELINE_RATE_MHZ + SMMU_OUTSTANDING) & 0xff;
		write_outstanding = ((IPU_BASELINE_LMT_OSD * ipu_rate_MHz) / IPU_BASELINE_RATE_MHZ) & 0xff;
		ret = (read_outstanding << 8) + write_outstanding;
		printk(KERN_DEBUG"[%s]:read/write OSD=%u/%u\n", __func__, read_outstanding, write_outstanding);
	} else {
		/* for ipu_rate=960M, bandwidth is 7500Mbps, equal proportion for other ipu_rate (7500 * (MAX/1000000)=7200000<MAX of unsigned int) */
		bandwidth = (IPU_BASELINE_LMT_BANDWIDTH_MHZ * ipu_rate_MHz) / IPU_BASELINE_RATE_MHZ;

		/* IPU_BANDWIDTH_LMT_BY_QOS, default */
		noc_bandwidth_lmt = (unsigned long)bandwidth * 256 / (vcodec_rate / IPU_RATE_HZ_TO_MHZ) + 1; /* "+1" means to round up */
		ret = noc_bandwidth_lmt;

		printk(KERN_DEBUG"[%s]:ipu_bandwidth=%d, qos_bandwidth=%d\n", __func__, bandwidth, noc_bandwidth_lmt);
	}

	return ret;
}

static void ipu_bandwidth_set_lmt(unsigned int reg_bandwidth, unsigned int saturation, unsigned int bandwidth_lmt_trategy)
{
	if (IPU_BANDWIDTH_LMT_BY_RW_OSD == bandwidth_lmt_trategy) {
		if (0 == adapter->ics_irq_io_addr) {
			printk(KERN_ERR"[%s]:IPU_ERROR:adapter->ics_irq_io_addr is NULL\n", __func__);
			return;
		}

		iowrite32(reg_bandwidth, (void *)((unsigned long)adapter->ics_irq_io_addr + ICS_MAX_OSD_REG));
	} else {
		if (0 == adapter->noc_bus_io_addr) {
			printk(KERN_ERR"[%s]:IPU_ERROR:adapter->noc_bus_io_addr is NULL\n", __func__);
			return;
		}

		iowrite32(ICS_NOC_BUS_CONFIG_QOS_TYPE, (void *)((unsigned long)adapter->noc_bus_io_addr + adapter->ics_noc_bus_reg_offset.qos_type));
		iowrite32(reg_bandwidth, (void *)((unsigned long)adapter->noc_bus_io_addr + adapter->ics_noc_bus_reg_offset.factor));
		iowrite32(saturation, (void *)((unsigned long)adapter->noc_bus_io_addr + adapter->ics_noc_bus_reg_offset.saturation));
		iowrite32(ICS_NOC_BUS_QOS_EXTCONTROL_ENABLE, (void *)((unsigned long)adapter->noc_bus_io_addr + adapter->ics_noc_bus_reg_offset.qos_extcontrol));
	}
}

int ipu_reset_ioremap_addr(void)
{
	adapter->pmctrl_io_addr = ioremap((unsigned long)adapter->pmctrl_reg_offset.base_addr, (unsigned long)0xfff);
	if (!adapter->pmctrl_io_addr) {
		printk(KERN_ERR"[%s]:IPU_ERROR:pmctrl_io_addr ioremap fail\n", __func__);
		return -ENOMEM;
	}

	adapter->pctrl_io_addr = ioremap((unsigned long)adapter->pctrl_reg_offset.base_addr, (unsigned long)0xff);
	if (!adapter->pctrl_io_addr) {
		printk(KERN_ERR"[%s]:IPU_ERROR:pctrl_io_addr ioremap fail\n", __func__);
		return -ENOMEM;
	}

	adapter->media2_io_addr = ioremap((unsigned long)adapter->media2_reg_offset.base_addr, (unsigned long)0xff);
	if (!adapter->media2_io_addr) {
		printk(KERN_ERR"[%s]:IPU_ERROR:media2_io_addr ioremap fail\n", __func__);
		return -ENOMEM;
	}

	adapter->peri_io_addr = ioremap((unsigned long)adapter->peri_reg_offset.base_addr, (unsigned long)0xfff);
	if (!adapter->peri_io_addr) {
		printk(KERN_ERR"[%s]:IPU_ERROR:peri_io_addr ioremap fail\n", __func__);
		return -ENOMEM;
	}

	return 0;
}

void ipu_reset_unremap_addr(void)
{
	if (adapter->pmctrl_io_addr) {
		iounmap(adapter->pmctrl_io_addr);
		adapter->pmctrl_io_addr = NULL;
	}

	if (adapter->pctrl_io_addr) {
		iounmap(adapter->pctrl_io_addr);
		adapter->pctrl_io_addr = NULL;
	}

	if (adapter->media2_io_addr) {
		iounmap(adapter->media2_io_addr);
		adapter->media2_io_addr = NULL;
	}

	if (adapter->peri_io_addr) {
		iounmap(adapter->peri_io_addr);
		adapter->peri_io_addr = NULL;
	}
}

int ipu_reset_init(struct device *dev)
{
	int ret;

	if (adapter->feature_tree.ipu_reset_when_in_error) {
		if (!ipu_get_reset_offset(dev)) {
			printk(KERN_ERR"[%s]: IPU_ERROR:fatal err, error reset reg offset\n", __func__);
			return -ENOENT;
		}

		/* ioremap reg addr for reset */
		ret = ipu_reset_ioremap_addr();
		if (ret) {
			printk(KERN_ERR"[%s]: IPU_ERROR:fatal err, ipu_reset_ioremap_addr fail\n", __func__);
			return ret;
		}
	}

	return 0;
}

static void performance_monitor_open(void)
{
	if (!adapter->feature_tree.performance_monitor) {
		printk(KERN_ERR"[%s]: IPU_ERROR:unsupport op in this platform!\n", __func__);
		return;
	}

	/* PERF_CNT_CLK_GT.perf_cnt_clk_gt = 0x1, to OPEN performance monitor */
	iowrite32(PERF_CNT_CLK_GT_ENABLE, (void *)((unsigned long)adapter->ics_irq_io_addr + PERF_CNT_CLK_GT_REG));

	/* PERF_CNT_CLEAR.perf_cnt_clear = 0x1, FRAME_CNT_CLEAR = 0x11, CLEAR old values */
	iowrite32(PERF_CNT_CLEAR, (void *)((unsigned long)adapter->ics_irq_io_addr + PERF_CNT_CLEAR_REG));
	iowrite32(FRAME_START_CNT_CLEAR | FRAME_FINISH_CNT_CLEAR, (void *)((unsigned long)adapter->ics_irq_io_addr + FRAME_CNT_CLEAR_REG));
}

static void performance_monitor_get_stat(void)
{
	unsigned int frame_cycle_cnt;
	unsigned int fu_idle_cnt;
	unsigned int io_idle_cnt;
	unsigned int all_idle_cnt;
	unsigned int all_busy_cnt;
	unsigned int ics_frame_cnt;
	unsigned int vcodecbus_clk;

	if (!adapter->feature_tree.performance_monitor) {
		printk(KERN_ERR"[%s]: IPU_ERROR:unsupport op in this platform!\n", __func__);
		return;
	}

	/* after ICS finish, record value of FRAME_CYC_CNT, FU_IDLE_CNT, IO_IDLE_CNT, ALL_IDLE_CNT, ALL_BUSY_CNT, ICS_FRAME_CNT  */
	frame_cycle_cnt = ioread32((void *)((unsigned long)adapter->ics_irq_io_addr + FRAME_CYC_CNT_REG));
	fu_idle_cnt = ioread32((void *)((unsigned long)adapter->ics_irq_io_addr + FU_IDLE_CNT_REG));
	io_idle_cnt = ioread32((void *)((unsigned long)adapter->ics_irq_io_addr + IO_IDLE_CNT_REG));
	all_idle_cnt = ioread32((void *)((unsigned long)adapter->ics_irq_io_addr + ALL_IDLE_CNT_REG));
	all_busy_cnt = ioread32((void *)((unsigned long)adapter->ics_irq_io_addr + ALL_BUSY_CNT_REG));
	ics_frame_cnt = ioread32((void *)((unsigned long)adapter->ics_irq_io_addr + ICS_FRAME_CNT_REG));

	// todo: here you can config PERF_CNT_CLEAR.perf_cnt_clear = 0x1 if want to clear current value

	vcodecbus_clk = clk_get_rate(adapter->clk.vcodecbus_clk_ptr);
	vcodecbus_clk = vcodecbus_clk / 1000000; /* convert vcodecbus clk working rate from bps to Mbps */

	if (0 != vcodecbus_clk) {
		printk(KERN_DEBUG"[%s]: frame_cycle: %d | fu/io/all idle: %d/%d/%d | all_busy: %d | ics_frame_cnt: %d; hw time: %d @ %dM",
			__func__, frame_cycle_cnt, fu_idle_cnt, io_idle_cnt, all_idle_cnt, all_busy_cnt, ics_frame_cnt & 0xffff,
			frame_cycle_cnt / vcodecbus_clk, vcodecbus_clk);
	} else {
		printk(KERN_DEBUG"[%s]: frame_cycle: %d | fu/io/all idle: %d/%d/%d | all_busy: %d | ics_frame_cnt: %d",
			__func__, frame_cycle_cnt, fu_idle_cnt, io_idle_cnt, all_idle_cnt, all_busy_cnt, ics_frame_cnt & 0xffff);
	}

}

static void performance_monitor_close(void)
{
	if (!adapter->feature_tree.performance_monitor) {
		printk(KERN_ERR"[%s]: IPU_ERROR:unsupport op in this platform!\n", __func__);
		return;
	}

	/* PERF_CNT_CLK_GT.perf_cnt_clk_gt = 0x0, to CLOSE performance monitor */
	iowrite32(PERF_CNT_CLK_GT_CLOSE, (void *)((unsigned long)adapter->ics_irq_io_addr + PERF_CNT_CLK_GT_REG));

	/* PERF_CNT_CLEAR.perf_cnt_clear = 0x1, FRAME_CNT_CLEAR = 0x11, to clear current value */
	iowrite32(PERF_CNT_CLEAR, (void *)((unsigned long)adapter->ics_irq_io_addr + PERF_CNT_CLEAR_REG));
	iowrite32(FRAME_START_CNT_CLEAR | FRAME_FINISH_CNT_CLEAR, (void *)((unsigned long)adapter->ics_irq_io_addr + FRAME_CNT_CLEAR_REG));

	// todo: here you can calc by this data, e.g. you can calc the accurate time clapse by (FRAME_CYC_CNT * BUS clock)
}

/* ipu interrupt init, including update ipu status, clear interrupt, and unmask interrupt */
void ipu_interrupt_init(void)
{
	unsigned long irq_io_addr = (unsigned long)adapter->ics_irq_io_addr;

	/* clear ipu status to unfinished */
	iowrite32(IPU_STATUS_UNFINISH, (void *)((unsigned long)adapter->config_reg_virt_addr + IPU_STATUS_REG));

	/* clear ns interrupt */
	if (adapter->feature_tree.finish_irq_expand_ns) {
		iowrite32(ICS_IRQ_CLEAR_IRQ_NS, (void *)(irq_io_addr + adapter->irq_reg_offset.ics_irq_clr_ns));
	}

	/* unmask interrupt */
	iowrite32(ICS_IRQ_UNMASK_NO_SECURITY, (void *)(irq_io_addr + adapter->irq_reg_offset.ics_irq_mask_ns));
}

/* to mask ipu interrupt and will not receive it */
void ipu_interrupt_deinit(void)
{
	iowrite32(ICS_IRQ_MASK_NO_SECURITY,
		(void *)((unsigned long)adapter->ics_irq_io_addr + adapter->irq_reg_offset.ics_irq_mask_ns));
}

void ipu_reset_proc(unsigned int addr)
{
	unsigned int loop_cnt;
	int noc_power_idle_ack;
	int noc_power_idle_stat;
	int noc_peri_status;
	int noc_idle = 0;
	unsigned int reg_bandwidth;

	if ((adapter->feature_tree.ipu_reset_when_in_error != IPU_RESET_BY_CONFIG_NOC_BUS) &&
		(adapter->feature_tree.ipu_reset_when_in_error != IPU_SOFT_RESET)) {
		printk(KERN_ERR"[%s]: IPU_ERROR:unsupported ipu reset trategy=%u!\n", __func__, adapter->feature_tree.ipu_reset_when_in_error);
		return;
	}

	mutex_lock(&adapter->reset_mutex);

	printk(KERN_DEBUG"[%s]: ipu_recover start!\n", __func__);

	if (ipu_clock_set_rate(&adapter->clk, adapter->clk.stop_rate)) {
		/* not return even set rate error, because reset process should finish */
		printk(KERN_DEBUG"[%s]: ipu_clock_set_rate fail, ignore\n", __FUNCTION__);
	} else {
		printk(KERN_DEBUG"[%s]: ipu_clock_set_rate ok\n", __FUNCTION__);
	}

	if (IPU_SOFT_RESET == adapter->feature_tree.ipu_reset_when_in_error) {
		/* set ICS_SOFT_RST_REQ */
		iowrite32(1, (void *)((unsigned long)adapter->ics_irq_io_addr + ICS_SOFT_RST_REQ_REG));

		/* loop wait ICS_SOFT_RST_ACK (0xFF4A2078) == 0x1 */
		for (loop_cnt = 0; loop_cnt < 100; loop_cnt++) {
			if (ICS_SOFT_RST_ACK == ioread32((void *)((unsigned long)adapter->ics_irq_io_addr + ICS_SOFT_RST_ACK_REG))) {
				break;
			}
			udelay(1);
		}

		printk(KERN_DEBUG"[%s]: loop wait ICS_SOFT_RST_ACK ok\n", __FUNCTION__);

	} else {
		/* config NOC register, to enter stream limited mode, stream is limited to 0 */
		mutex_lock(&adapter->bandwidth_lmt_mutex);

		/* config NOC register, to enter stream limited mode, stream is limited to 0; SOC suggest here use NOC-BUS trategy for bandwidth lmt */
		ipu_bandwidth_set_lmt(ICS_NOC_BUS_CONFIG_FACTOR, ICS_NOC_BUS_CONFIG_SATURATION_RESET, IPU_RESET_BY_CONFIG_NOC_BUS);
		mutex_unlock(&adapter->bandwidth_lmt_mutex);

		ipu_smmu_override_prefetch_addr(addr);

		udelay(10);
	}

	iowrite32(CONFIG_NOC_POWER_IDLEREQ_0, (void *)((unsigned long)adapter->pmctrl_io_addr + adapter->pmctrl_reg_offset.noc_power_idle_req));

	loop_cnt = 0;
	while(!noc_idle) {
		if (loop_cnt > 10) {
			printk(KERN_ERR"[%s]: IPU_ERROR:loop timeout", __func__);
			break;
		}
		noc_power_idle_ack = ioread32((void *)((unsigned long)adapter->pmctrl_io_addr + adapter->pmctrl_reg_offset.noc_power_idle_ack)) & CONFIG_NOC_POWER_IDLEACK_0_BIT9;
		noc_power_idle_stat = ioread32((void *)((unsigned long)adapter->pmctrl_io_addr + adapter->pmctrl_reg_offset.noc_power_idle_stat)) & CONFIG_NOC_POWER_IDLE_0_BIT9;
		noc_peri_status = ioread32((void *)((unsigned long)adapter->pctrl_io_addr + adapter->pctrl_reg_offset.peri_stat3)) & CONFIG_PCTRL_PERI_STAT3_BIT22;
		noc_idle = noc_power_idle_ack && noc_power_idle_stat && noc_peri_status;
		printk(KERN_DEBUG"[%s]: noc_power_idle_ack:%d, noc_power_idle_stat:%d, noc_peri_status:%d\n",
			__func__, noc_power_idle_ack, noc_power_idle_stat, noc_peri_status);

		udelay(1);
		loop_cnt++;
	}

	iowrite32(CONFIG_MEDIA2_REG_PERDIS0_ICS, (void *)((unsigned long)adapter->media2_io_addr + adapter->media2_reg_offset.perdis0));
	iowrite32(CONFIG_MEDIA2_REG_PERRSTEN0_ICS, (void *)((unsigned long)adapter->media2_io_addr + adapter->media2_reg_offset.perrsten0));
	iowrite32(CONFIG_MEDIA2_REG_PEREN0_ICS, (void *)((unsigned long)adapter->media2_io_addr + adapter->media2_reg_offset.peren0));
	iowrite32(CONFIG_MEDIA2_REG_PERDIS0_ICS, (void *)((unsigned long)adapter->media2_io_addr + adapter->media2_reg_offset.perdis0));
	iowrite32(CONFIG_SC_GT_CLK_ICS_DIS, (void *)((unsigned long)adapter->peri_io_addr + adapter->peri_reg_offset.clkdiv18));

	/* restart ipu */
	iowrite32(CONFIG_SC_GT_CLK_ICS_EN, (void *)((unsigned long)adapter->peri_io_addr + adapter->peri_reg_offset.clkdiv18));
	iowrite32(CONFIG_MEDIA2_REG_PEREN0_ICS, (void *)((unsigned long)adapter->media2_io_addr + adapter->media2_reg_offset.peren0));
	iowrite32(CONFIG_MEDIA2_REG_PERDIS0_ICS, (void *)((unsigned long)adapter->media2_io_addr + adapter->media2_reg_offset.perdis0));
	iowrite32(CONFIG_MEDIA2_REG_PERRSTEN0_ICS, (void *)((unsigned long)adapter->media2_io_addr + adapter->media2_reg_offset.perrstdis0));
	iowrite32(CONFIG_MEDIA2_REG_PEREN0_ICS, (void *)((unsigned long)adapter->media2_io_addr + adapter->media2_reg_offset.peren0));
	iowrite32(CONFIG_NOC_ICS_POWER_IDLEREQ_DIS, (void *)((unsigned long)adapter->pmctrl_io_addr + adapter->pmctrl_reg_offset.noc_power_idle_req));

	noc_power_idle_ack = ioread32((void *)((unsigned long)adapter->pmctrl_io_addr + adapter->pmctrl_reg_offset.noc_power_idle_ack)) & CONFIG_NOC_POWER_IDLEACK_0_BIT9;
	noc_power_idle_stat = ioread32((void *)((unsigned long)adapter->pmctrl_io_addr + adapter->pmctrl_reg_offset.noc_power_idle_stat)) & CONFIG_NOC_POWER_IDLE_0_BIT9;

	printk(KERN_DEBUG"[%s]: noc_power_idle_ack:%d, noc_power_idle_stat:%d\n", __func__, noc_power_idle_ack, noc_power_idle_stat);

	/* unmask irq*/
	iowrite32(ICS_IRQ_UNMASK_NO_SECURITY, (void *)((unsigned long)(adapter->ics_irq_io_addr) + adapter->irq_reg_offset.ics_irq_mask_ns));

	if (ipu_clock_set_rate(&adapter->clk, adapter->clk.start_rate)) {
		/* not return even set rate error, because reset process should finish */
		printk(KERN_DEBUG"[%s]: ipu_clock_set_rate fail, ignore\n", __FUNCTION__);
	}

	mutex_lock(&adapter->bandwidth_lmt_mutex);
	reg_bandwidth = ipu_bandwidth_get_lmt(adapter->clk.curr_rate, adapter->feature_tree.ipu_bandwidth_lmt);
	if (reg_bandwidth) {
		ipu_bandwidth_set_lmt(reg_bandwidth, ICS_NOC_BUS_CONFIG_SATURATION_NORMAL, adapter->feature_tree.ipu_bandwidth_lmt);
	}
	mutex_unlock(&adapter->bandwidth_lmt_mutex);

	ipu_smmu_init(adapter->smmu_ttbr0,
		(unsigned long)adapter->smmu_rw_err_virtual_addr, adapter->feature_tree.smmu_port_select, adapter->feature_tree.smmu_mstr_hardware_start);

	ipu_interrupt_init();

	printk(KERN_DEBUG"[%s]: ipu_recover finished!\n", __func__);
	mutex_unlock(&adapter->reset_mutex);
}


static bool cambricon_ipu_workqueue(void) {
	taskElement Element;
	struct timeval tv1, tv2;
	unsigned int elapse;

	/* fetch a compute_task in the head of kfifo, need update computed_task_cnt */
	adapter->computed_task_cnt++;

	/* ipu irq comes means the first element of FIFO_TaskElements is finished
		and this task must be compute task, so remove it from queue. */
	if (kfifo_get(&FIFO_TaskElements, &Element)) {
		up(&(adapter->task_fifo_sem));
		if (CHECK_TASK_TYPE(Element.taskType)) {
			printk(KERN_ERR "[%s]: IPU_ERROR:Get error type of first task from kfifo\n", __func__);
			return false;
		}
	} else {
		printk(KERN_ERR "[%s]: IPU_ERROR:Get element from kfifo faild.\n", __func__);
		return false;
	}

	if (!kfifo_is_empty(&FIFO_TaskElements)) {
		if (kfifo_peek(&FIFO_TaskElements, &Element)) {/*to check next taskType*/

			if (CHECK_TASK_TYPE(Element.taskType)) {
				printk(KERN_ERR "[%s]: IPU_ERROR:Get error type of next task from kfifo\n", __func__);
				return false;
			}
			do_gettimeofday(&tv1);
			/* handle sync task, until queue head is compute task or queue is empty */
			while (IPU_SYNC_TASK == Element.taskType) {
				/* wakeup user process */
				if (NULL != Element.ptaskFlag)
					*Element.ptaskFlag = IPU_TASK_FLAG_SYNC_DONE;
				wake_up_interruptible(&sync_wq);
				if (!kfifo_get(&FIFO_TaskElements,&Element))
					printk(KERN_ERR "[%s]: IPU_ERROR:FIFO_TaskElements is empty\n", __func__);
				up(&(adapter->task_fifo_sem));
				/*to check next taskType*/
				if (!kfifo_peek(&FIFO_TaskElements, &Element)) {
					break;
				}

				if (CHECK_TASK_TYPE(Element.taskType)) {
					printk(KERN_ERR "[%s]: IPU_ERROR:Get error sync task type from kfifo\n", __func__);
					return false;
				}
			}

			do_gettimeofday(&tv2);

			elapse = (tv2.tv_sec - tv1.tv_sec)*1000 + (tv2.tv_usec - tv1.tv_usec)/1000;
			if (elapse > 1) {
				printk(KERN_DEBUG"[%s]: elapse=%d\n",__func__ , elapse);
			}
		} else {
			printk(KERN_ERR "[%s]: IPU_ERROR:Peek element from kfifo failed while fifo is not empty\n", __func__);
			return false;
		}
	}
	return true;
}

static void ipu_level1_irq_handler(void)
{
	unsigned int lv1_int_status;
	unsigned int fault_status;
	int loop_cnt = 0;
	unsigned int detect_fault = 0;

	if (!adapter->feature_tree.level1_irq) {
		printk(KERN_ERR"[%s]: IPU_ERROR:unsupport op in this platform!\n", __func__);
		return;
	}

	lv1_int_status = ioread32((void *)((unsigned long)adapter->config_reg_virt_addr + IPU_STATUS_REG_FINISH));

	while(lv1_int_status){
		if (loop_cnt > 3) {
			printk(KERN_ERR"[%s]: lv1_int loop timeout\n", __func__);
			break;
		}
		printk(KERN_DEBUG"[%s]: lv1_int_status=0x%x\n", __func__, lv1_int_status);

		if (lv1_int_status & INTERRUPTINST_INTERRUPT_MASK) {
			detect_fault = detect_fault | INTERRUPTINST_INTERRUPT_MASK;
			fault_status = ioread32((void *)((unsigned long)adapter->config_reg_virt_addr + IPU_INTERRUPT_INST_REG));
			iowrite32(0, (void *)((unsigned long)(adapter->config_reg_virt_addr) + IPU_INTERRUPT_INST_REG));
			printk(KERN_ERR"[%s]: IPU_ERROR:INTERRUPTINST_INTERRUPT error, fault_status=0x%x\n", __func__, fault_status);
		}
		if (lv1_int_status & CT_INTERRUPT_MASK) {
			/* do_interruptinst_interrupt_service(); */
			detect_fault = detect_fault | CT_INTERRUPT_MASK;
			fault_status = ioread32((void *)((unsigned long)adapter->config_reg_virt_addr + IPU_ID_REG));
			iowrite32(0, (void *)((unsigned long)(adapter->config_reg_virt_addr) + IPU_ID_REG));
			printk(KERN_ERR"[%s]: IPU_ERROR:CT_INTERRUPT error, fault_status=0x%x\n", __func__, fault_status);
		}
		if (lv1_int_status & IPU_CONTROL_INTERRUPT_MASK) {
			/* do_ipu_control_interrupt_service(); */
			detect_fault = detect_fault | IPU_CONTROL_INTERRUPT_MASK;
			fault_status = ioread32((void *)((unsigned long)adapter->config_reg_virt_addr + IPU_CONTROL_ID_REG));
			iowrite32(0, (void *)((unsigned long)(adapter->config_reg_virt_addr) + IPU_CONTROL_ID_REG));
			printk(KERN_ERR"[%s]: IPU_ERROR:IPU_CONTROL_INTERRUPT error, fault_status=0x%x\n", __func__, fault_status);
		}
		/* WATCH_DOG */
		if (lv1_int_status & WATCH_DOG_MASK) {
			/* do_watch_dog_interrupt_service();*/
			detect_fault = detect_fault | WATCH_DOG_MASK;
			fault_status = ioread32((void *)((unsigned long)adapter->config_reg_virt_addr + IPU_WATCH_DOG_REG));
			iowrite32(0, (void *)((unsigned long)(adapter->config_reg_virt_addr) + IPU_WATCH_DOG_REG));
			printk(KERN_ERR"[%s]: IPU_ERROR:WATCH_DOG error, fault_status=0x%x\n", __func__, fault_status);
		}
		if (lv1_int_status & IS_NORMAL_FINISH_MASK) {
			iowrite32(IPU_STATUS_UNFINISH, (void *)((unsigned long)adapter->config_reg_virt_addr + IPU_STATUS_REG));
		}
		iowrite32(lv1_int_status, (void *)((unsigned long)adapter->config_reg_virt_addr + IPU_STATUS_REG_FINISH));
		iowrite32(0, (void *)((unsigned long)adapter->config_reg_virt_addr + (IPU_STATUS_REG_FINISH + 0x4))); /* to clear finish reg high 32bit */
		lv1_int_status = ioread32((void *)((unsigned long)adapter->config_reg_virt_addr + IPU_STATUS_REG_FINISH));

		loop_cnt++;
	}

	if (detect_fault) {
		if (adapter->feature_tree.ipu_reset_when_in_error && adapter->reset_va) {
			ipu_reset_proc((unsigned int)adapter->reset_va);
		}
	}
}

static void ipu_finish_irq_handler(void)
{
	unsigned long reg_virt_addr = (unsigned long)adapter->config_reg_virt_addr;

	/* clear ipu finished status */
	iowrite32(0, (void *)(reg_virt_addr + IPU_STATUS_REG));
	if (adapter->feature_tree.finish_irq_expand_ns) {
		/* clear ipu finished status non-security irq, which is copied from ipu finished irq in non-security mode if unmask*/
		// fixme: add security and protected mode here in furture
		if (adapter->feature_tree.level1_irq) {
			iowrite32(ICS_IRQ_CLEAR_IRQ_LEVEL1_NS | ICS_IRQ_CLEAR_IRQ_NS,
				(void *)((unsigned long)adapter->ics_irq_io_addr + adapter->irq_reg_offset.ics_irq_clr_ns));
		} else {
			iowrite32(ICS_IRQ_CLEAR_IRQ_NS, (void *)((unsigned long)adapter->ics_irq_io_addr + adapter->irq_reg_offset.ics_irq_clr_ns));
		}
	}
}

static void ipu_core_irq_handler(void)
{
	ipu_finish_irq_handler();

	if (adapter->feature_tree.level1_irq) {
		ipu_level1_irq_handler();
	} else {
		/* clear ipu finished status */
		iowrite32(IPU_STATUS_UNFINISH, (void *)((unsigned long)adapter->config_reg_virt_addr + IPU_STATUS_REG));
	}
}

/*
 * A very tiny interrupt handler. It runs with interrupts disabled,
 * but there is possibility of conflicting with operating register
 * at the same time in two different CPUs. So we need to serialize
 * accesses to the chip with the ipu_lock spinlock.
 */
static irqreturn_t ipu_interrupt_handler(int irq, void *dev)
{
	bool ipu_smmu_err_isr;
	struct cambricon_ipu_private *pdev;

	if (dev == NULL) {
		printk(KERN_ERR"[%s]: IPU_ERROR:no device\n", __func__);
		return IRQ_HANDLED;
	}

	pdev = container_of((struct cdev *)dev, struct cambricon_ipu_private, cdev);
	if (!pdev || pdev != adapter) {
		printk(KERN_ERR"[%s]: IPU_ERROR:error dev\n", __func__);
		return IRQ_HANDLED;
	}

	mutex_lock(&adapter->power_mutex);
	if (!adapter->ipu_power_up) {
		printk(KERN_ERR"[%s]: IPU_ERROR:ipu is power down, ignore\n", __func__);
		mutex_unlock(&adapter->power_mutex);
		return IRQ_HANDLED;
	}

	ipu_watchdog_stop(&adapter->reset_wtd);
	if (adapter->feature_tree.performance_monitor) {
		performance_monitor_get_stat();
		performance_monitor_close();
	}

	mutex_lock(&adapter->stat_mutex);
	ipu_smmu_err_isr = ipu_smmu_interrupt_handler(&adapter->stat.smmu_irq_count);
	mutex_unlock(&adapter->stat_mutex);

	if (ipu_smmu_err_isr) {
		if (adapter->feature_tree.ipu_reset_when_in_error && adapter->reset_va) {
			rdr_system_error((unsigned int)MODID_NPU_EXC_INTERRUPT_ABNORMAL, 0, 0);
			ipu_reset_proc((unsigned int)adapter->reset_va); //lint !e570
		}
	} else {
		ipu_core_irq_handler();
		if (!cambricon_ipu_workqueue()) {
			printk(KERN_ERR"[%s]: IPU_ERROR:cambricon_ipu_workqueue return false\n", __func__);
			mutex_unlock(&adapter->power_mutex);
			return IRQ_HANDLED;
		}
	}

	/*Start next task*/
	if (!kfifo_is_empty(&FIFO_TaskElements)) {
		/* handle compute task */
		printk(KERN_DEBUG"[%s]: to start ipu\n", __func__);
		start_ipu_workqueue();
	} else {
		printk(KERN_DEBUG"[%s]: to stop ipu\n", __func__);
		if (stop_ipu()) {
			printk(KERN_ERR"[%s]: IPU_ERROR:ipu_power_down fail, FATAL\n", __func__);
		}
	}
	mutex_unlock(&adapter->power_mutex);

	return IRQ_HANDLED;
}

/* here void (*function)(unsigned long) is a standard Linux timer callback function,
	it will register as a callback func for ipu resume watchdog */
static int ipu_reset(void *arg)
{
	static unsigned long last_computed_task = 0;

	char register_info_flag[] = "NULL";
	char *perr = register_info_flag;

	UNUSED_PARAMETER(arg);

	while (1) {
		if (down_interruptible(&adapter->reset_wtd.sem)) {
			continue;
		}

		mutex_lock(&adapter->power_mutex);

		if (false == adapter->ipu_power_up) {
			printk(KERN_ERR"[%s]: IPU_ERROR: ipu is power off, can not resume\n", __func__);
			mutex_unlock(&adapter->power_mutex);
			continue;
		}

		if (last_computed_task != adapter->computed_task_cnt) {
			/* get IPU status in adapter */
			printk(KERN_ERR"[%s]: ipu(status=%d) dead, ttbr0=%lx, inst_set=%d, offchip{set=%x, base=%x}, curr_frame=0x%lx\n",
				__func__, adapter->ipu_power_up, adapter->smmu_ttbr0, adapter->boot_inst_set.boot_inst_recorded_is_config,
				adapter->boot_inst_set.access_ddr_addr_is_config, adapter->boot_inst_set.ipu_access_ddr_addr, adapter->computed_task_cnt);

		/* get clock and power status in register */
		ipu_reg_info.peri_reg.peri_stat   = ioread32((void *)adapter->peri_io_addr + adapter->peri_reg_offset.peristat7);
		ipu_reg_info.peri_reg.ppll_select = ioread32((void *)adapter->peri_io_addr + adapter->peri_reg_offset.clkdiv8);
		ipu_reg_info.peri_reg.power_stat  = ioread32((void *)adapter->peri_io_addr + adapter->peri_reg_offset.perpwrstat);
		ipu_reg_info.peri_reg.power_ack   = ioread32((void *)adapter->peri_io_addr + adapter->peri_reg_offset.perpwrack);
		ipu_reg_info.peri_reg.reset_stat  = ioread32((void *)adapter->media2_io_addr + adapter->media2_reg_offset.perrststat0);
		ipu_reg_info.peri_reg.perclken0   = ioread32((void *)adapter->media2_io_addr + adapter->media2_reg_offset.perclken0);
		ipu_reg_info.peri_reg.perstat0    = ioread32((void *)adapter->media2_io_addr + adapter->media2_reg_offset.perstat0);

		printk(KERN_ERR"[%s]: peri_stat=%x, ppll_select=%x, power_stat=%x, power_ack=%x, reset_stat=%x, perclken=%x, perstat=%x\n",
			__func__,
			ipu_reg_info.peri_reg.peri_stat,
			ipu_reg_info.peri_reg.ppll_select,
			ipu_reg_info.peri_reg.power_stat,
			ipu_reg_info.peri_reg.power_ack,
			ipu_reg_info.peri_reg.reset_stat,
			ipu_reg_info.peri_reg.perclken0,
			ipu_reg_info.peri_reg.perstat0);

		/* get memory info in register */
		ipu_smmu_dump_strm();

		/* WTD time out, report to DSM */
		perr = register_info;
		DSM_AI_KERN_ERROR_REPORT(DSM_AI_KERN_WTD_TIMEOUT_ERR_NO,
			"IPU soft watchdog timeout, ipu_status=%d, ttbr0=%x, inst_set=%d, offchip{set=%x, base=%x}, last_computed_task=%d, register_info=%s.\n",
			adapter->ipu_power_up, adapter->smmu_ttbr0, adapter->boot_inst_set.boot_inst_recorded_is_config, adapter->boot_inst_set.access_ddr_addr_is_config,
			adapter->boot_inst_set.ipu_access_ddr_addr, adapter->computed_task_cnt, perr);

		rdr_system_error((unsigned int)MODID_NPU_EXC_DEAD, 0, 0);
	}

		/* reset ipu */
		ipu_reset_proc((unsigned int)adapter->reset_va);

		/* ipu watchdog comes means the first element of FIFO_TaskElements is invalid and abandoned.
			so remove it from queue and prepare for next element. */
		if (last_computed_task == adapter->computed_task_cnt) {
			printk(KERN_ERR"[%s]: IPU_ERROR:current frame=0x%lx can not resume, abandon\n", __func__, last_computed_task);
			if (!cambricon_ipu_workqueue()) {
				printk(KERN_ERR"[%s]: IPU_ERROR:cambricon_ipu_workqueue return false\n", __func__);
				mutex_unlock(&adapter->power_mutex);
				continue;
			}
		}

		/* start next frame */
		if (!kfifo_is_empty(&FIFO_TaskElements)) {
			/* handle compute task */
			start_ipu_workqueue();
		} else {
			printk(KERN_DEBUG"[%s]: kfifo is empty, stop ipu\n", __func__);
			if (stop_ipu()) {
				printk(KERN_ERR"[%s]: IPU_ERROR:ipu_power_down fail, FATAL\n", __func__);
			}
		}

		last_computed_task = adapter->computed_task_cnt;
		mutex_unlock(&adapter->power_mutex);
	}

	return 0; /*lint !e527*/
}

void ipu_reset_irq(unsigned long para)
{
	UNUSED_PARAMETER(para);
	if (adapter->reset_wtd.task) {
		up(&adapter->reset_wtd.sem);
	}
}

/* this func not use power mutex, so its caller must use power mutex */
int ipu_power_up(void)
{
	struct timeval tv1, tv2, tv3, tv4, tv5;
	int ret;
	unsigned int reg_bandwidth;

	if (0 == adapter->smmu_ttbr0) {
		printk(KERN_ERR"[%s]: IPU_ERROR:FATAL, adapter->smmu_ttbr0 is NULL\n", __func__);
		return -EFAULT;
	}

	if (adapter->ipu_power_up) {
		printk(KERN_ERR"[%s]: IPU_ERROR:ipu is already power-up\n", __func__);
		return -EBUSY;
	}

	do_gettimeofday(&tv1);

	/*ICS power on*/
	ret = regulator_ip_vipu_enable();
	if (ret) {
		printk(KERN_ERR"[%s]: IPU_ERROR:regulator_ip_vipu_enable failed\n", __func__);
		return ret;
	}

	do_gettimeofday(&tv2);

	/* start ipu clock */
	ret = ipu_clock_start(&adapter->clk);
	if (ret) {
		printk(KERN_ERR"[%s]: IPU_ERROR:IPU clock start failed, power down ipu\n", __func__);
		if (regulator_ip_vipu_disable()) {
			printk(KERN_ERR"[%s]: IPU_ERROR:ipu power down fail\n", __func__);
		}

		/* if more return add in this func, add ipu power-down function before "return" */
		return ret;
	}

	if (adapter->feature_tree.ipu_bandwidth_lmt) {
		mutex_lock(&adapter->bandwidth_lmt_mutex);
		reg_bandwidth = ipu_bandwidth_get_lmt(adapter->clk.curr_rate, adapter->feature_tree.ipu_bandwidth_lmt);
		if (reg_bandwidth) {
			ipu_bandwidth_set_lmt(reg_bandwidth, ICS_NOC_BUS_CONFIG_SATURATION_NORMAL, adapter->feature_tree.ipu_bandwidth_lmt);
		}
		mutex_unlock(&adapter->bandwidth_lmt_mutex);
	}

	do_gettimeofday(&tv3);

	ipu_smmu_init(adapter->smmu_ttbr0,
		(unsigned long)adapter->smmu_rw_err_virtual_addr, adapter->feature_tree.smmu_port_select, adapter->feature_tree.smmu_mstr_hardware_start);

	mutex_lock(&adapter->stat_mutex);
	if (adapter->smmu_stat_en) {
		ipu_smmu_reset_statistic();
	}
	mutex_unlock(&adapter->stat_mutex);

	do_gettimeofday(&tv4);

	ipu_interrupt_init();
	do_gettimeofday(&tv5);

	if (((tv5.tv_sec - tv1.tv_sec)*1000000 + (tv5.tv_usec - tv1.tv_usec)) > 1000) {
		printk(KERN_WARNING"[%s]: IPU_WARN: ipu PU/CLK/SMMU_INIT/ISR_INIT elapse is %ld/%ld/%ld/%ld usec\n", __func__,
			(tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec),
			(tv3.tv_sec - tv2.tv_sec)*1000000 + (tv3.tv_usec - tv2.tv_usec),
			(tv4.tv_sec - tv3.tv_sec)*1000000 + (tv4.tv_usec - tv3.tv_usec),
			(tv5.tv_sec - tv4.tv_sec)*1000000 + (tv5.tv_usec - tv4.tv_usec));
	}

	adapter->ipu_power_up = true;

	return 0;
}

/* ipu char device ops function implementation, inode:node of file, filp: pointer of file */
static int ipu_open(struct inode *inode, struct file *filp)
{
	struct cambricon_ipu_private *dev;

	if (!inode || !filp){
		printk(KERN_ERR"[%s]: IPU_ERROR:invalid input parameter !\n", __func__);
		return -EINVAL;
	}

	if (!adapter) {
		printk(KERN_ERR"[%s]: IPU_ERROR:FATAL: adapter is NULL\n", __func__);
		return -EFAULT;
	}

	mutex_lock(&adapter->open_mutex);

	if (adapter->ipu_device_opened) {
		printk(KERN_ERR"[%s]: IPU_ERROR:IPU device has already opened !\n", __func__);
		mutex_unlock(&adapter->open_mutex);
		return -EBUSY;
	} else {
		wake_lock(&adapter->wakelock);
		adapter->ipu_device_opened = true;
	}

	/* find offset of "cdev" in "struct cambricon_ipu_private" */
	dev = container_of(inode->i_cdev, struct cambricon_ipu_private, cdev);/*lint !e826*/

	/* save dev info to "filp->private_data", it will be used later */
	filp->private_data = dev;

	ipu_mem_mngr_init();

	mutex_lock(&adapter->stat_mutex);
	adapter->smmu_stat_en = false;
	memset(&adapter->stat, 0, sizeof(adapter->stat));

	ipu_clock_set_start_default(&adapter->clk);

	mutex_unlock(&adapter->stat_mutex);
	mutex_unlock(&adapter->open_mutex);

	printk(KERN_DEBUG"[%s]: IPU device open success!\n", __func__);

	return 0;/*lint !e454*/
}

/* this func not use power mutex, so its caller must use power mutex */
int ipu_power_down(void)
{
	int ret;
	struct timeval tv1, tv2, tv3, tv4, tv5;

	if (!adapter->ipu_power_up) {
		printk(KERN_ERR"[%s]: IPU_ERROR:ipu is already power-down, ignore\n", __func__);
		return -EBUSY;
	}

	do_gettimeofday(&tv1);
	ipu_smmu_deinit();

	mutex_lock(&adapter->stat_mutex);
	if (adapter->smmu_stat_en) {
		ipu_smmu_record_statistic(&adapter->stat);
	}
	mutex_unlock(&adapter->stat_mutex);


	do_gettimeofday(&tv2);
	ipu_interrupt_deinit();

	/* Set clock rate to default(which should always be generated from PPLL0) */
	do_gettimeofday(&tv3);
	if (ipu_clock_set_rate(&adapter->clk, adapter->clk.stop_rate)) {
		/* not return even set rate error, because power down process should finish */
		printk(KERN_WARNING"[%s]: ipu_clock_set_rate fail, ignore\n", __FUNCTION__);
	}

	/* STOP ipu clock */
	ipu_clock_stop(&adapter->clk);

	/*ICS power down*/
	do_gettimeofday(&tv4);
	ret = regulator_ip_vipu_disable();
	if (ret) {
		printk(KERN_ERR"[%s]: IPU_ERROR:ipu power down fail\n", __func__);
		return -EBUSY;
	}

	do_gettimeofday(&tv5);

	if (((tv5.tv_sec - tv1.tv_sec)*1000000 + (tv5.tv_usec - tv1.tv_usec)) > 1000) {
		printk(KERN_WARNING"[%s]: IPU_WARN: ipu SMMU_DEINT/INTR CLR/CLK STOP/PD elapse is %ld/%ld/%ld/%ld usec\n", __func__,
			(tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec),
			(tv3.tv_sec - tv2.tv_sec)*1000000 + (tv3.tv_usec - tv2.tv_usec),
			(tv4.tv_sec - tv3.tv_sec)*1000000 + (tv4.tv_usec - tv3.tv_usec),
			(tv5.tv_sec - tv4.tv_sec)*1000000 + (tv5.tv_usec - tv4.tv_usec));
	}

	adapter->ipu_power_up = false;

	return 0;
}

/* ipu device release and power down */
static int ipu_release(struct inode *inode, struct file *filp)
{
	struct cambricon_ipu_private *dev;
	int ret;
	struct irq_data *desc = irq_get_irq_data(adapter->irq);
	struct irq_common_data *d = desc ? desc->common : NULL;

	UNUSED_PARAMETER(inode);

	if (!inode || !filp){
		printk(KERN_ERR"[%s]: IPU_ERROR:input parameter inode or filp is invalid !\n", __func__);
		return -EINVAL;
	}

	if (!adapter) {
		printk(KERN_ERR"[%s]: IPU_ERROR:FATAL: adapter is NULL\n", __func__);
		return -EFAULT;
	}

	mutex_lock(&adapter->open_mutex);
	if (!adapter->ipu_device_opened) {
		printk(KERN_ERR"[%s]: IPU_ERROR:ipu device is already closed\n", __func__);
		mutex_unlock(&adapter->open_mutex);
		return -EBUSY;
	}

	dev = filp->private_data;

	if (adapter == dev) {
		kfifo_reset(&FIFO_TaskElements);
		if (d && (d->state_use_accessors & IRQD_IRQ_DISABLED)) {
			enable_irq(adapter->irq);
		}
	} else {
		printk(KERN_ERR"[%s]: IPU_ERROR:No IPU device! input dev is %pK\n", __func__, (void *)dev);
		mutex_unlock(&adapter->open_mutex);
		return -EINVAL;
	}

	ipu_watchdog_stop(&adapter->reset_wtd);

	mutex_lock(&adapter->power_mutex);
	ret = stop_ipu();
	if (ret) {
		/* not return here, ignore, to release other resource */
		printk(KERN_ERR"[%s]: IPU_ERROR:ipu_power_down fail\n", __func__);
	}
	mutex_unlock(&adapter->power_mutex);

	ipu_mem_mngr_deinit(&adapter->reset_va);

	mutex_lock(&adapter->boot_inst_set.boot_mutex);
	adapter->boot_inst_set.access_ddr_addr_is_config = false;
	adapter->boot_inst_set.boot_inst_recorded_is_config = false;
	mutex_unlock(&adapter->boot_inst_set.boot_mutex);

	mutex_lock(&adapter->clk.clk_mutex);
	ipu_vote_peri_withdraw(&adapter->clk);
	adapter->clk.voted_peri_volt = 0;
	mutex_unlock(&adapter->clk.clk_mutex);

	wake_unlock(&adapter->wakelock);
	adapter->ipu_device_opened = false;
	mutex_unlock(&adapter->open_mutex);/*lint !e455*/

	printk(KERN_DEBUG"[%s]: ipu release succeed\n", __func__);

	return 0;
}

static long ipu_read_dword(struct cambricon_ipu_private *dev, unsigned long arg, struct ioctl_out_params *out_params)
{
	long read_value;

	UNUSED_PARAMETER(out_params);

	if (arg != IPU_VERSION_REG) {
		printk(KERN_ERR"[%s]: IPU_ERROR:Error can not read offset=0x%pK, it is an invalid offset to read\n", __func__, (void *)arg);
		return -EINVAL;
	}

	mutex_lock(&adapter->power_mutex);

	if (!adapter->ipu_power_up) {
		printk(KERN_ERR"[%s]: IPU_ERROR:this work is not allowed when IPU is power-down, FATAL and ignore\n", __func__);
		mutex_unlock(&adapter->power_mutex);
		return -EBUSY;
	}

	read_value = ioread32((void *)((unsigned long)dev->config_reg_virt_addr + arg));

	printk(KERN_ERR"[%s]: Read CONFIG REG dword offset 0x%pK, value is 0x%x\n",
		__func__, (void *)arg, (unsigned int)read_value);

	mutex_unlock(&adapter->power_mutex);
	return read_value;
}

static long ipu_write_dword(struct cambricon_ipu_private *dev, unsigned long arg, struct ioctl_out_params *out_params)
{
	const int offset = 0;
	const int data = 1;
	unsigned long in[2];

	UNUSED_PARAMETER(out_params);

	if (!arg) {
		printk(KERN_ERR"[%s]: IPU_ERROR:input parameter arg is NULL, FATAL arg and ignore\n", __func__);
		return -EINVAL;
	}

	memset(in, 0, sizeof(in));  /* [false alarm]:memset an array */

	if (copy_from_user(in, (void __user *)arg, sizeof(in))) {
		printk(KERN_ERR"[%s]: IPU_ERROR:copy arg failed!\n", __func__);
		return -EFAULT;
	}

	printk(KERN_DEBUG"[%s]: Write CONFIG REG dword offset 0x%pK, value is 0x%lx\n", __func__, (void *)in[offset], in[data]);

	mutex_lock(&adapter->power_mutex);
	if (adapter->ipu_power_up) {
		if ((IPU_START_REG == in[offset]) || (IPU_STATUS_REG == in[offset]) || (IPU_BASE_ADDR_REG == in[offset])) {
			if (IPU_START_REG == in[offset] && IPU_TO_START == in[data]) {
				if (adapter->feature_tree.performance_monitor) {
					performance_monitor_open();
				}
				ipu_watchdog_start(&adapter->reset_wtd, WATCHDOG_TIMEOUT_THD_MS);
			}
			iowrite32(in[data], (void *)((unsigned long)dev->config_reg_virt_addr + in[offset]));
		} else {
			printk(KERN_ERR"[%s]: IPU_ERROR:error offset when ipu on, offset=0x%pK, data=0x%lx\n", __func__, (void *)in[offset], in[data]);
			mutex_unlock(&adapter->power_mutex);
			return -EFAULT;
		}
	} else {
		if (IPU_BASE_ADDR_REG == in[offset]) {
			mutex_lock(&adapter->boot_inst_set.boot_mutex);
			adapter->boot_inst_set.ipu_access_ddr_addr = in[data];
			adapter->boot_inst_set.access_ddr_addr_is_config = true;
			mutex_unlock(&adapter->boot_inst_set.boot_mutex);
		} else {
			printk(KERN_ERR"[%s]: IPU_ERROR:error offset when ipu off, offset=0x%pK, data=0x%lx\n", __func__, (void *)in[offset], in[data]);
			mutex_unlock(&adapter->power_mutex);
			return -EFAULT;
		}
	}

	mutex_unlock(&adapter->power_mutex);
	return 0;
}

static long ipu_set_map(struct cambricon_ipu_private *dev, unsigned long arg, struct ioctl_out_params *out_params)
{
	long ret_value;
	struct map_data map_data;

	UNUSED_PARAMETER(dev);

	if (!arg) {
		printk(KERN_ERR"[%s]: IPU_ERROR:input parameter arg is NULL, FATAL arg and ignore\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(&map_data, (void __user *)arg, sizeof(map_data))) {
		printk(KERN_ERR"[%s]: IPU_ERROR:copy arg failed!\n", __func__);
		return -EFAULT;
	}

	ret_value = ipu_smmu_map(&map_data);
	if (ret_value) {
		printk(KERN_ERR"[%s]: IPU_ERROR:IPU_ERROR:ipu_smmu_map failed!\n", __func__);
		return -EFAULT;
	}

	if (copy_to_user((void __user *)arg, &map_data, sizeof(map_data))) {
		printk(KERN_ERR"[%s]: IPU_ERROR:copy_to_user failed!\n", __func__);
		return -EFAULT;
	}

	out_params->memory_node = ipu_mem_mngr_add(&map_data);

	return ret_value;
}

static long ipu_set_reset_virt_addr(struct cambricon_ipu_private *dev, unsigned long arg, struct ioctl_out_params *out_params)
{
	long ret;
	struct memory_manage_node *node;

	if (!arg) {
		printk(KERN_ERR"[%s]: IPU_ERROR:input parameter arg is NULL, FATAL arg and ignore\n", __func__);
		return -EINVAL;
	}

	if (adapter->reset_va) {
		printk(KERN_ERR"[%s]: IPU_ERROR:virtual address for reset is exist, ignore\n", __func__);
		return -EINVAL;
	}

	ret = ipu_set_map(dev, arg, out_params);

	if ((0 == ret) && out_params->memory_node) {
		node = (struct memory_manage_node *)out_params->memory_node;
		adapter->reset_va = node->map.format.iova_start;
	} else {
		printk(KERN_ERR"[%s]: IPU_ERROR:remap failed!\n", __func__);
	}

	return ret;
}

static long ipu_set_unmap(struct cambricon_ipu_private *dev, unsigned long arg, struct ioctl_out_params *out_params)
{
	struct map_data map_data;

	UNUSED_PARAMETER(dev);
	UNUSED_PARAMETER(out_params);

	if (!arg) {
		printk(KERN_ERR"[%s]: IPU_ERROR:input parameter arg is NULL, FATAL arg and ignore\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(&map_data, (void __user *)arg, sizeof(map_data))) {
		printk(KERN_ERR"[%s]: IPU_ERROR:copy_from_user failed!\n", __func__);
		return -EFAULT;
	}

	if (ipu_mem_mngr_del(&map_data)) {
		printk(KERN_ERR"[%s]: IPU_ERROR:invalid map data to unmap\n", __func__);
		return -EINVAL;
	}

	if (ipu_smmu_unmap(&map_data)) {
		printk(KERN_ERR"[%s]: IPU_ERROR:unmap failed\n", __func__);
		return -EINVAL;
	}

	if (adapter->reset_va == map_data.format.iova_start) {
		adapter->reset_va = 0;
	}

	return 0;
}

static long ipu_set_reset_statistic(struct cambricon_ipu_private *dev, unsigned long arg, struct ioctl_out_params *out_params)
{
	UNUSED_PARAMETER(dev);
	UNUSED_PARAMETER(arg);
	UNUSED_PARAMETER(out_params);

	mutex_lock(&adapter->stat_mutex);
	adapter->smmu_stat_en = true;
	mutex_unlock(&adapter->stat_mutex);
	return 0;
}

static long ipu_set_report_statistic(struct cambricon_ipu_private *dev, unsigned long arg, struct ioctl_out_params *out_params)
{
	UNUSED_PARAMETER(dev);
	UNUSED_PARAMETER(out_params);

	if (!arg) {
		printk(KERN_ERR"[%s]: IPU_ERROR:input parameter arg is NULL, FATAL arg and ignore\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&adapter->stat_mutex);

	if (!adapter->smmu_stat_en) {
		printk(KERN_ERR"[%s]: IPU_ERROR:report statistic fail, for have not set ipu smmu stat enable\n", __func__);
		mutex_unlock(&adapter->stat_mutex);
		return -ENOMEM;
	}

	if (copy_to_user((void __user *)arg, &adapter->stat, sizeof(adapter->stat))) {
		printk(KERN_ERR"[%s]: IPU_ERROR:copy_to_user failed!\n", __func__);
		mutex_unlock(&adapter->stat_mutex);
		return -EFAULT;
	}

	mutex_unlock(&adapter->stat_mutex);

	return 0;
}

static long ipu_set_update_pte(struct cambricon_ipu_private *dev, unsigned long arg, struct ioctl_out_params *out_params)
{
	UNUSED_PARAMETER(dev);
	UNUSED_PARAMETER(arg);
	UNUSED_PARAMETER(out_params);

	mutex_lock(&adapter->power_mutex);
	if (!adapter->ipu_power_up) {
		printk(KERN_ERR"[%s]: IPU_ERROR:this work is not allowed when IPU is power-down, FATAL and ignore\n", __func__);
		mutex_unlock(&adapter->power_mutex);
		return -EBUSY;
	}

	ipu_smmu_pte_update();
	mutex_unlock(&adapter->power_mutex);
	return 0;
}

long ipu_set_profile(struct cambricon_ipu_private *dev, unsigned long arg, struct ioctl_out_params *out_params)
{
	unsigned long profile = arg;
	unsigned int clock;

	UNUSED_PARAMETER(dev);
	UNUSED_PARAMETER(out_params);

	/* get rate by profile */
	switch (profile) {
	case DEV_UNSET_PROFILE:
		clock = adapter->clk.ipu_low; /* if profile not need IPU, set IPU clock as low */
		break;
	case DEV_LOW_PROFILE:
		clock = adapter->clk.ipu_low;
		break;
	case DEV_NORMAL_PROFILE:
		clock = adapter->clk.ipu_middle;
		break;
	case DEV_HIGH_PROFILE:
		clock = adapter->clk.ipu_high;
		break;
	default:
		printk(KERN_ERR"[%s]: IPU_ERROR:profile=%lu error\n", __FUNCTION__, profile);
		return -EINVAL;
	}

	return ipu_clock_set_start_rate(&adapter->clk, clock);
}

static ioctl_cb ipu_obtain_ioctl_callback(unsigned int cmd)
{
	unsigned int cnt;
	unsigned int size = sizeof(ipu_ioctl_maps) / sizeof(ipu_ioctl_maps[0]);

	for (cnt = 0; cnt < size; cnt++) {
		if (ipu_ioctl_maps[cnt].cmd == cmd) {
			return ipu_ioctl_maps[cnt].func;
		}
	}

	printk(KERN_ERR"[%s]: IPU_ERROR:error cmd=0x%x\n", __func__, cmd);

	return NULL;
}

static int input_filp_check(struct file *filp, unsigned int cmd)
{
	struct cambricon_ipu_private *dev;
	if (!filp){
		printk(KERN_ERR"[%s]: IPU_ERROR:input parameter filp is invalid !\n", __func__);
		return -EINVAL;
	}
	dev = filp->private_data;
	if (!dev || dev != adapter) {
		printk(KERN_ERR"[%s]: IPU_ERROR:No IPU device! input dev is %pK\n", __func__, (void *)dev);
		return -EINVAL;
	}

	if (!dev->config_reg_virt_addr) {
		printk(KERN_ERR"[%s]: IPU_ERROR:reg_virt_addr is invalid!\n", __func__);
		return -EINVAL;
	}
	/* check whether cmd is valid */
	if (_IOC_TYPE(cmd) != MAGIC_NUM) {
		printk(KERN_ERR"[%s]: IPU_ERROR:cmd is invalid!(not a MAGIC_NUM)\n", __func__);
		return -EINVAL;
	}

	if (_IOC_NR(cmd) > IPU_IOC_MAXNR) {
		printk(KERN_ERR"[%s]: IPU_ERROR:cmd is invalid!(%d > IPU_IOC_MAXNR)\n", __func__, _IOC_NR(cmd));
		return -EINVAL;
	}

	return 0;
}

static long ipu_process_workqueue(struct cambricon_ipu_private *dev, unsigned long arg, struct ioctl_out_params *out_params)
{
	taskElement element;
	taskFlag_t taskFlag = IPU_TASK_FLAG_NONE;
	long ret_value = 0;

	if (!arg) {
		printk(KERN_ERR"[%s]: IPU_ERROR:input parameter arg is NULL, FATAL arg and ignore\n", __func__);
		return -EINVAL;
	}

	if(down_interruptible(&(dev->task_fifo_sem))){
		printk(KERN_ERR "[%s]Request task_fifo_sem failed\n", __func__);
		return -EBUSY;
	}

	/*Caution : Already obtain task_fifo_sem !!*/
	disable_irq(adapter->irq);

	if (!copy_from_user(&element, (void __user *)arg, sizeof(element))) {
		// todo: add more input-check at here!!

		element.ptaskFlag = NULL;/*Ensure point correct*/
		mutex_lock(&adapter->power_mutex);
		if (kfifo_is_empty(&FIFO_TaskElements)) {
			/*First task is SYNC while fifo is empty should return user thread at once*/
			if (element.taskType != IPU_SYNC_TASK){
				kfifo_put(&FIFO_TaskElements, element);
				printk(KERN_DEBUG "[%s]: IPU_COMPUTE_TASK to start ipu\n", __func__);
				start_ipu_workqueue();
			} else {
				up(&(dev->task_fifo_sem));
			}
		} else {
			if (unlikely(IPU_SYNC_TASK == element.taskType)) {
				printk(KERN_DEBUG "[%s]: IPU_SYNC_TASK is waiting\n", __func__);
				/*Set sync flag as wake up condition*/
				taskFlag = IPU_TASK_FLAG_SYNC_WAITING;
				element.ptaskFlag = &taskFlag;
				kfifo_put(&FIFO_TaskElements, element);
				mutex_unlock(&adapter->power_mutex);
				enable_irq(adapter->irq);
				up(&(dev->config_reg_sem));
				/* sync task sleeping */
				wait_event_interruptible(sync_wq, IPU_TASK_FLAG_SYNC_DONE == taskFlag);
				out_params->ret_directly = true;
				return 0;
			} else {
				printk(KERN_DEBUG "[%s]: IPU_COMPUTE_TASK add to queue\n", __func__);
				kfifo_put(&FIFO_TaskElements, element);
			}
		}
		mutex_unlock(&adapter->power_mutex);
	} else {
		printk(KERN_ERR "[%s]: IPU_ERROR: Copy_from_user failed\n", __func__);
		up(&(dev->task_fifo_sem));
		ret_value = -EINVAL;
	}

	enable_irq(adapter->irq);
	return ret_value;
}

static long ipu_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	ioctl_cb ioctl_callback;
	long ret = -EINVAL;
	struct cambricon_ipu_private *dev;
	struct ioctl_out_params out_params;

	if (!adapter) {
		printk(KERN_ERR"[%s]: IPU_ERROR:FATAL: adapter is NULL\n", __func__);
		return -EFAULT;
	}

	mutex_lock(&adapter->open_mutex);
	if (!adapter->ipu_device_opened) {
		printk(KERN_ERR"[%s]: IPU_ERROR:receiving IOCTL attack when ipu is not open, ignore\n", __func__);
		mutex_unlock(&adapter->open_mutex);
		return -EINVAL;
	}

	if (input_filp_check(filp, cmd)) {
		printk(KERN_ERR"[%s]:IPU_ERROR: input parameter filp is invalid !\n", __func__);
		mutex_unlock(&adapter->open_mutex);
		return -EINVAL;
	}

	dev = filp->private_data;

	/* a "mutex signal" to guarantee only one process is working */
	if (down_interruptible(&dev->config_reg_sem)) {
		printk(KERN_ERR"[%s]:IPU_ERROR: down_interruptible dev->config_reg_sem fail!\n", __func__);
		mutex_unlock(&adapter->open_mutex);
		return -ERESTARTSYS;
	}

	ioctl_callback = ipu_obtain_ioctl_callback(cmd);

	out_params.memory_node = 0;
	out_params.ret_directly = false;

	if (ioctl_callback) {
		ret = ioctl_callback(dev, arg, &out_params);
	} else {
		printk(KERN_ERR"[%s]:IPU_ERROR: unknown cmd=0x%x\n", __func__, cmd);
	}

	if (!out_params.ret_directly) {
		up(&(dev->config_reg_sem));
	}

	mutex_unlock(&adapter->open_mutex);

	return ret;
}

static long ipu_ioctl32(struct file *fd, unsigned int cmd, unsigned long arg)
{
	void *ptr_user = compat_ptr(arg);

	return ipu_ioctl(fd, cmd, (unsigned long)ptr_user);
}

static ssize_t ipu_write_when_ipu_down(struct cambricon_ipu_private *dev, const char __user *buf, size_t count, loff_t *f_pos)
{
	ssize_t ret_value;
	unsigned long bytes_not_copied;
	unsigned max_buff_size = dev->inst_ram_size;

	/* "sizeof" not need mutex_lock */
	if (max_buff_size > sizeof(adapter->boot_inst_set.boot_inst)) {
		max_buff_size = sizeof(adapter->boot_inst_set.boot_inst);
	}

	/* NOTE: here max_buff_size is a 32bit length data, and above judge can guarantee (*f_pos + count) < 2*max_buff_size
		so OVERFLOW in "unsigned long" is impossible here */
	if((unsigned long)*f_pos + (unsigned long)count > (unsigned long)max_buff_size) {
		printk(KERN_ERR"[%s]: IPU_ERROR:FATAL, count OVERFLOW, *f_pos=0x%pK, count=0x%x\n", __func__, (void *)*f_pos, (unsigned int)count);
		return -EINVAL;
	}

	mutex_lock(&adapter->boot_inst_set.boot_mutex);
	if (adapter->boot_inst_set.boot_inst_recorded_is_config) {
		printk(KERN_ERR"[%s]: IPU_WARN:boot_inst to overwrite old data\n", __func__);
	}

	/* clean-up buffer and recv new data */
	memset(&adapter->boot_inst_set.boot_inst[0], 0, sizeof(adapter->boot_inst_set.boot_inst));
	bytes_not_copied = copy_from_user((void *)((unsigned long)&adapter->boot_inst_set.boot_inst[0] + (unsigned long)(*f_pos)), buf, count);

	if (bytes_not_copied) {
		printk(KERN_ERR"[%s]: IPU_ERROR:Copy data from user failed, bytes_not_copied=0x%lx\n", __func__, bytes_not_copied);

		/* here type convert has no risk of type truncation, because the caller "ipu_write" had judged the value */
		ret_value = (ssize_t)count - (ssize_t)bytes_not_copied;
	} else {
		adapter->boot_inst_set.boot_inst_recorded_is_config = true;

		/* because above guarentees (*f_pos + count <= dev->inst_ram_size), dev->inst_ram_size is uint16, so "+" CAN NOT get overflow */
		adapter->boot_inst_set.boot_inst_size = ((unsigned int)count + (unsigned int)(*f_pos));
		ret_value = (ssize_t)count;
	}

	mutex_unlock(&adapter->boot_inst_set.boot_mutex);
	return ret_value;

}

/* CPU write inst to IPU SRAM */
static ssize_t ipu_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	int ipu_power_up_flag;
	ssize_t ret_value = 0;
	struct cambricon_ipu_private *dev;

	if (!filp || !buf || !f_pos) {
		printk(KERN_ERR"[%s]: IPU_ERROR:input parameter filp or buf is invalid !\n", __func__);
		return -EINVAL;
	}

	dev = filp->private_data;

	if (!dev || dev != adapter) {
		printk(KERN_ERR"[%s]: IPU_ERROR:No IPU device! input dev is %pK\n", __func__, (void *)dev);
		return -EINVAL;
	}

	if (count == 0 || count > dev->inst_ram_size) {
		printk(KERN_ERR"[%s]: IPU_ERROR:input parameter count is invalid!\n", __func__);
		return -EINVAL;
	}

	if (*f_pos >= dev->inst_ram_size || *f_pos < 0) {
		printk(KERN_ERR"[%s]: IPU_ERROR:Write file position out of range!\n", __func__);
		return -EINVAL;
	}

	if (!adapter->inst_ram_virt_addr) {
		printk(KERN_ERR"[%s]: IPU_ERROR:inst_ram_virt_addr is NULL\n", __func__);
		return -EFAULT;
	}

	mutex_lock(&adapter->power_mutex);
	ipu_power_up_flag = adapter->ipu_power_up;
	if (adapter->ipu_power_up) {
		printk(KERN_ERR"[%s]: IPU_ERROR:unsupport to write ipu SRAM when it is powered up\n", __func__);
		mutex_unlock(&adapter->power_mutex);
	} else {
		/* ipu off, write to temp buff "adapter->boot_inst_set.boot_inst" */
		mutex_unlock(&adapter->power_mutex);
		ret_value = ipu_write_when_ipu_down(dev, buf, count, f_pos);
	}

	if (ret_value < 0) {
		printk(KERN_DEBUG"[%s]: write 0x%lx bytes, offset=0x%pK error, return is %ld\n", __func__, count, (void *)(*f_pos), ret_value);
	} else {
		printk(KERN_DEBUG"[%s]: write 0x%lx bytes, offset=0x%pK when ipu_power_up flag is %d\n",
			__func__, ret_value, (void *)(*f_pos), ipu_power_up_flag);
	}

	return ret_value;
}

/* set file position in IPU where CPU to write */
static loff_t ipu_llseek(struct file *filp, loff_t off, int whence)
{
	struct cambricon_ipu_private *dev;
	loff_t pos;

	if (!filp) {
		printk(KERN_ERR"[%s]: IPU_ERROR:input parameter filp is invalid !\n", __func__);
		return -EINVAL;
	}

	if (!adapter) {
		printk(KERN_ERR"[%s]: IPU_ERROR:FATAL: adapter is NULL\n", __func__);
		return -EFAULT;
	}

	dev = filp->private_data;
	if (!dev || dev != adapter) {
		printk(KERN_ERR"[%s]: IPU_ERROR:No IPU device! input dev is %pK\n", __func__, (void *)dev);
		return -EINVAL;
	}

	/* covert unsigned int(32bit) to long(64bit), with no risk */
	if ((off > (signed long)dev->inst_ram_size) || (off < (-1 * (signed long)dev->inst_ram_size))) {
		printk(KERN_ERR"[%s]: IPU_ERROR:invalid offset=0x%pK while inst_ram_size=0x%x\n",
			__func__, (void *)off, dev->inst_ram_size);
		return -EINVAL;
	}

	if(down_interruptible(&dev->llseek_sem)) {
		printk(KERN_ERR"[%s]:IPU_ERROR: down_interruptible llseek_sem fail!\n", __func__);
		return -ERESTARTSYS;
	}

	pos = filp->f_pos;
	switch (whence) {
		/* Set f_pos */
	case 0: /* SEEK_SET */
		pos = off;
		break;
		/* Move f_pos forward */
	case 1: /* SEEK_CUR */
		pos += off;
		break;
		/* More */
	case 2: /* SEEK_END */
		break;
		/* Default Operation */
	default:
		up(&dev->llseek_sem);
		return -EINVAL;
	}

	if ((pos >= dev->inst_ram_size) || (pos < 0)) {
		printk(KERN_ERR"[%s]: IPU_ERROR:Move file position out of range!", __func__);
		up(&dev->llseek_sem);
		return -EINVAL;
	}

	filp->f_pos = pos;

	up(&dev->llseek_sem);
	return pos;
}

/* Allocate ipu chrdev region ("dev/ipu") */
static int cambricon_ipu_chrdev_region(dev_t *chrdev)
{
	int err = -1;

	/* Allocate char device region */
	if (ipu_major) {
		*chrdev = MKDEV(ipu_major, ipu_minor);
		err = register_chrdev_region(*chrdev, 1, IPU_NAME);
	} else {
		err = alloc_chrdev_region(chrdev, 0, 1, IPU_NAME);
	}
	if (err < 0) {
		printk(KERN_ERR"[%s]:IPU_ERROR:alloc_chrdev_region fail!\n", __func__);
		return err;
	}

	ipu_major = MAJOR(*chrdev);
	ipu_minor = MINOR(*chrdev);

	return err;
}

/* probe() function for platform driver */
static int cambricon_ipu_probe(struct platform_device *pdev)
{
	int err;
	dev_t chrdev = 0;
	struct device *temp = NULL;
	struct resource *res, *res_cfg;
	unsigned long  size;

	printk(KERN_DEBUG"[%s]: Initializing IPU device!\n", __func__);

	/* Register AI dsm client */
	DSM_AI_REGISTER();

	/* Allocate char device region */
	err = cambricon_ipu_chrdev_region(&chrdev);
	if (err < 0) {
		printk(KERN_ERR"[%s]: Failed to allocate device ID for IPU!\n", __func__);
		goto fail;
	}

	/* create and init queue FIFO_TaskElements */
	err = kfifo_alloc(&FIFO_TaskElements, TASKQUEUE_SIZE * sizeof(taskElement), GFP_KERNEL);
	if(err != 0) {
		printk(KERN_DEBUG"[%s]: Failed to allocate kfifo for FIFO_TaskElements!\n", __func__);
		goto unregister;
	}
	/*init fifo of task elements*/
	printk(KERN_INFO "[%s]:IPU FIFO_TaskElements init success, queue size: %u\n", __func__, kfifo_size(&FIFO_TaskElements));

	/* init wait queue sync_wq */
	init_waitqueue_head(&sync_wq);

	/* Allocate cambricon_ipu_private struct and asigned to global variable adapter, devm_kzalloc will memset adapter to zeros */
	adapter = devm_kzalloc(&pdev->dev, sizeof(struct cambricon_ipu_private), GFP_KERNEL);
	if (!adapter) {
		err = -ENOMEM;
		printk(KERN_ERR"[%s]: Failed to allocate memory for struct ipu!\n", __func__);
		goto cleanup_kfifo;
	}

	adapter->name = IPU_NAME;
	adapter->irq = (unsigned int)platform_get_irq_byname(pdev, "ipu_dma_irq");
	printk(KERN_DEBUG"[%s]: platform_get_irq_byname:%d\n", __func__, adapter->irq);

	/* get regulator */
	adapter->vipu_ip = devm_regulator_get(&pdev->dev, "vipu");
	if (IS_ERR(adapter->vipu_ip)) {
		printk(KERN_ERR"[%s]:Couldn't get regulator ip! \n", __func__);
		goto cleanup;
	}
	printk(KERN_DEBUG"[%s]:Get regulator ip succuss! dev-num-resouce:%d\n",
		__func__, pdev->num_resources);

	/* init mutex signals (down_interrupt/up) */
	sema_init(&(adapter->config_reg_sem), 1);
	sema_init(&(adapter->inst_ram_sem), 1);
	sema_init(&(adapter->llseek_sem), 1);
	sema_init(&(adapter->task_fifo_sem), TASKQUEUE_SIZE);

	mutex_init(&adapter->open_mutex);
	mutex_init(&adapter->power_mutex);
	mutex_init(&adapter->clk.clk_mutex);
	mutex_init(&adapter->stat_mutex);
	mutex_init(&adapter->boot_inst_set.boot_mutex);
	mutex_init(&adapter->reset_mutex);
	mutex_init(&adapter->bandwidth_lmt_mutex);

	/* ipu instruction ram resource, third input para is for the number of dts */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);

	if (res == NULL) {
		printk(KERN_ERR"[%s]: failed to get instruction resource\n", __func__);
		err = -ENXIO;
		goto cleanup;
	}

	size = resource_size(res);
	adapter->inst_ram_phys_addr = res->start;
	adapter->inst_ram_size = (unsigned int)size;

	/* the region of request must be unused */
	adapter->inst_mem = request_mem_region(res->start, size, pdev->name);
	if (adapter->inst_mem == NULL) {
		printk(KERN_ERR"[%s]: failed to get instruction memory region\n", __func__);
		err = -ENOENT;
		goto cleanup;
	}

	adapter->inst_ram_virt_addr = ioremap(res->start, size);
	if (!adapter->inst_ram_virt_addr) {
		printk(KERN_ERR"[%s]: ioremap() of instruction resource failed\n", __func__);
		err = -ENXIO;
		goto release_res_inst;
	}

	/* ipu configure registers resource */
	res_cfg = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res_cfg == NULL) {
		printk(KERN_ERR"[%s]: failed to get configure registers resource\n", __func__);
		err = -ENXIO;
		goto unmap_ram;
	}

	size = resource_size(res_cfg);
	adapter->config_reg_phys_addr = res_cfg->start;
	adapter->config_reg_length = (unsigned int)size;
	adapter->cfg_mem = request_mem_region(res_cfg->start, size, pdev->name);
	if (adapter->cfg_mem == NULL) {
		printk(KERN_ERR"[%s]: failed to get configure registers memory region\n", __func__);
		err = -ENOENT;
		goto unmap_ram;
	}

	adapter->config_reg_virt_addr = ioremap(res_cfg->start, size);
	if (!adapter->config_reg_virt_addr) {
		printk(KERN_ERR"[%s]: ioremap() of configure registers resource failed\n", __func__);
		err = -ENXIO;
		goto release_res_cfg;
	}

	if (!ipu_get_feature_tree(&pdev->dev)) {
		printk(KERN_ERR"[%s]: fatal err, unknown feature tree\n", __func__);
		goto unmap_reg;
	}

	/* request ipu irq */
	if (request_threaded_irq(adapter->irq, NULL, ipu_interrupt_handler,
		(unsigned long)(IRQF_ONESHOT | IRQF_TRIGGER_HIGH), IPU_NAME, &(adapter->cdev))) {
		printk(KERN_ERR"[%s]: IPU Require IRQ failed!\n", __func__);
		err = -EIO;
		goto unmap_reg;
	}
	printk(KERN_DEBUG"[%s]: IPU Require IRQ Succeeded\n", __func__);

	/* Add ipu char device to system, udev can auto detect */
	cdev_init(&(adapter->cdev), &ipu_fops);
	adapter->cdev.owner = THIS_MODULE;/*lint !e64*/
	adapter->cdev.ops = &ipu_fops;
	err = cdev_add(&(adapter->cdev), chrdev, 1);
	if (err) {
		printk(KERN_ERR"[%s]: Failed to Add IPU to system!\n", __func__);
		goto free_irq;
	}

	/* Create ipu class under /sys/class */
	dev_class = class_create(THIS_MODULE, IPU_NAME);/*lint !e64*/
	if (IS_ERR(dev_class)) {
		err = (int)PTR_ERR(dev_class);
		printk(KERN_ERR"[%s]: Failed to create ipu class!\n", __func__);
		goto destroy_cdev;
	}
	/* Register ipu device in sysfs, and this will cause udev to create corresponding device node */
	temp = device_create(dev_class, NULL, chrdev, NULL, "%s", IPU_NAME);
	if (IS_ERR(temp)) {
		err = (int)PTR_ERR(temp);
		printk(KERN_ERR"[%s]: Failed to mount IPU to /dev/ipu!\n", __func__);
		goto destroy_class;
	}

	if (!ipu_get_irq_offset(&pdev->dev)) {
		printk(KERN_ERR"[%s]: fatal err, ipu irq is unsupported\n", __func__);
		goto destroy_device;
	}

	/* ioremap irq reg addr */
	adapter->ics_irq_io_addr = ioremap((unsigned long)adapter->irq_reg_offset.ics_irq_base_addr, (unsigned long)0xff);
	if (!adapter->ics_irq_io_addr) {
		printk(KERN_ERR"[%s]IPU_ERROR:ics_irq_io_addr ioremap fail\n", __func__);
		goto destroy_device;
	}


	if (!ipu_smmu_master_get_offset(&pdev->dev)) {
		printk(KERN_ERR"[%s]: fatal err, ipu_smmu_master is unsupported\n", __func__);
		goto unmap_irq;
	}

	if (!ipu_smmu_common_get_offset(&pdev->dev)) {
		printk(KERN_ERR"[%s]: fatal err, ipu_smmu_common is unsupported\n", __func__);
		goto unmap_irq;
	}

	/* smmu manager init for ioremap smmu reg addr */
	if (ipu_smmu_mngr_init()) {
		goto unmap_smmu;
	}

	if (ipu_reset_init(&pdev->dev)) {
		goto unmap_smmu;
	}

	if (ipu_bandwidth_lmt_init(&pdev->dev)) {
		goto unmap_reset;
	}

	/* init ipu clock */
	if (ipu_clock_init(&pdev->dev, &adapter->clk, adapter->feature_tree.lpm3_set_vcodecbus)) {
		printk(KERN_ERR"[%s]: Failed to init ipu clock\n", __func__);
		goto unmap_bandwidth_lmt;
	}

	adapter->smmu_rw_err_virtual_addr = devm_kzalloc(&pdev->dev, ICS_SMMU_WR_ERR_BUFF_LEN + 0X80, GFP_KERNEL);
	if (!adapter->smmu_rw_err_virtual_addr) {
		err = -ENOMEM;
		printk(KERN_ERR"[%s]: Failed to allocate memory for smmu read and write phy addr in error case\n", __func__);
		goto exit_error;
	}

	adapter->smmu_ttbr0 = ipu_get_smmu_base_phy(&pdev->dev);


	err = ipu_mntn_rdr_init();
	if (err) {
		printk(KERN_ERR"[%s]: Call ipu_mntn_rdr_init is failed!ret=%d\n", __func__, err);
		goto exit_error;
	}

	wake_lock_init(&adapter->wakelock, WAKE_LOCK_SUSPEND, "ipu_process");

	ipu_watchdog_init(&adapter->reset_wtd, adapter->feature_tree.soft_watchdog_enable, ipu_reset_irq);

	sema_init(&(adapter->reset_wtd.sem), 0);
	adapter->reset_wtd.task = kthread_run(ipu_reset, NULL, "ipu_reset_task");

	printk(KERN_DEBUG"[%s]: Succeeded to initialize ipu device.\n", __func__);

	return 0;

exit_error:
unmap_bandwidth_lmt:
	ipu_bandwidth_lmt_unremap_addr();
unmap_reset:
	ipu_reset_unremap_addr();
unmap_smmu:
	ipu_smmu_mngr_deinit();
unmap_irq:
	iounmap(adapter->ics_irq_io_addr);
	adapter->ics_irq_io_addr = NULL;
destroy_device:
	device_destroy(dev_class, chrdev);
destroy_class:
	if (dev_class) {
		class_destroy(dev_class);
	}
destroy_cdev:
	cdev_del(&(adapter->cdev));
free_irq:
	free_irq(adapter->irq, &(adapter->cdev));
unmap_reg:
	iounmap(adapter->config_reg_virt_addr);
	adapter->config_reg_virt_addr = NULL;
release_res_cfg:
	release_mem_region((unsigned long)res_cfg->start, size);
unmap_ram:
	iounmap(adapter->inst_ram_virt_addr);
	adapter->inst_ram_virt_addr = NULL;
release_res_inst:
	release_mem_region((unsigned long)res->start, size);
cleanup:
	devm_kfree(&pdev->dev, adapter);
	adapter = NULL;
cleanup_kfifo:
	kfifo_free(&FIFO_TaskElements);
unregister:
	unregister_chrdev_region(chrdev, 1);
fail:
	/* Unregister ai client for DSM */
	DSM_AI_UNREGISTER();
	return err;
}

/* remove() function for platform driver */
static int __exit cambricon_ipu_remove(struct platform_device *pdev)
{
	dev_t chrdev;

	UNUSED_PARAMETER(pdev);

	/* device has two dev number, i.e. ipu_major, ipu_minor */
	chrdev = MKDEV(ipu_major, ipu_minor);

	printk(KERN_DEBUG"[%s]: Destroying IPU device!\n", __func__);

	if (adapter->reset_wtd.task) {
		kthread_stop(adapter->reset_wtd.task);
	}

	ipu_smmu_mngr_deinit();

	/* Destroy ipu class */
	if (dev_class) {
		device_destroy(dev_class, chrdev);
		class_destroy(dev_class);
	}

	/* Destroy ipu */
	if (adapter) {
		if (adapter->ics_irq_io_addr) {
			iounmap(adapter->ics_irq_io_addr);
			adapter->ics_irq_io_addr = NULL;
		}

		wake_lock_destroy(&adapter->wakelock);

		cdev_del(&(adapter->cdev));

		free_irq(adapter->irq, &(adapter->cdev));

		kfifo_free(&FIFO_TaskElements);
		if (adapter->config_reg_virt_addr) {
			iounmap(adapter->config_reg_virt_addr);
			adapter->config_reg_virt_addr = NULL;
		}

		if (adapter->inst_ram_virt_addr) {
			iounmap(adapter->inst_ram_virt_addr);
			adapter->inst_ram_virt_addr = NULL;
		}

		if (adapter->feature_tree.ipu_reset_when_in_error) {
			ipu_reset_unremap_addr();
		}

		ipu_bandwidth_lmt_unremap_addr();

		release_mem_region(adapter->inst_mem->start, resource_size(adapter->inst_mem));
		release_mem_region(adapter->cfg_mem->start, resource_size(adapter->cfg_mem));



		if (adapter->smmu_rw_err_virtual_addr) {
			devm_kfree(&pdev->dev, adapter->smmu_rw_err_virtual_addr);
			adapter->smmu_rw_err_virtual_addr = NULL;
		}

		devm_kfree(&pdev->dev, adapter);
		adapter = NULL;
	}

	/* Unregister chrdev region */
	unregister_chrdev_region(chrdev, 1);

	/* Unregister ai client for DSM */
	DSM_AI_UNREGISTER();

	printk(KERN_DEBUG"[%s]: Succeeded to destroying IPU device.\n", __func__);

	return 0;
}
/*lint -e785*/
static const struct of_device_id cambricon_ipu_match_table[] = {
	{ .compatible = COMP_CAMBRICON_IPU_DRV_NAME, },
	{},
};

/* to find key word "cambricon-ipu" in dts, if failed, not load driver */
MODULE_DEVICE_TABLE(of, cambricon_ipu_match_table);
/* ipu platform drive */
static struct platform_driver cambricon_ipu_driver = {
	.driver	= {
		.name = "cambricon-ipu",
		.owner = THIS_MODULE,/*lint !e64*/
		.of_match_table = of_match_ptr(cambricon_ipu_match_table),
	},
	.probe	= cambricon_ipu_probe,
	.remove	= cambricon_ipu_remove,
};
/*lint +e785*/
/* ipu platform device and driver register */
static int __init cambricon_ipu_init(void)
{
	int ret;

	printk(KERN_DEBUG"[%s]: platform device and driver register!\n", __func__);
	ret = platform_driver_register(&cambricon_ipu_driver);/*lint !e64*/

	/* No need to "platform_device_register(&cambricon_ipu_device);",
	because it did when DTS initializating, no need to register device again */

	return ret;
}

/* ipu platform device and driver unregister */
static void __exit cambricon_ipu_exit(void)
{
	platform_device_unregister(&cambricon_ipu_device);
	platform_driver_unregister(&cambricon_ipu_driver);
	destroy_workqueue(ipu_mntn_rdr_wq);
}

/*lint -e753 -e528*/

module_init(cambricon_ipu_init);
module_exit(cambricon_ipu_exit);

MODULE_AUTHOR("Cambricon Limited");
MODULE_LICENSE("GPL");

