/*
 * HiStar Remote Processor driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/mod_devicetable.h>
#include <linux/amba/bus.h>
#include <linux/dma-mapping.h>
#include <linux/remoteproc.h>
#include <linux/platform_data/remoteproc_hisp.h>
#include <linux/platform_data/hisp_mempool.h>
#include <linux/hisi/hisi_rproc.h>
#include "remoteproc_internal.h"
#include <linux/delay.h>
#include <linux/kfifo.h>
#include <linux/pm_wakeup.h>
#include <linux/mutex.h>
#include <linux/regulator/consumer.h>
#include <linux/scatterlist.h>
#include <linux/clk.h>
#include <linux/rproc_share.h>
#include <linux/sched/rt.h>
#include <linux/kthread.h>
#include <global_ddr_map.h>
#include <isp_ddr_map.h>
#include <asm/cacheflush.h>
#include <linux/firmware.h>
#include <linux/iommu.h>
#include <linux/hisi-iommu.h>
#include <linux/crc32.h>
#include <linux/ion.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/hisi/hisi_cmdline_parse.h>
#include <linux/spinlock.h>
#include "isp_ddr_map.h"
#include <linux/dma-buf.h>
#include <linux/version.h>
#include <linux/syscalls.h>
#include <linux/hisi/efuse_driver.h>
#include <linux/sched/types.h>
#include <securec.h>
#include <asm/memory.h>

#define DTS_COMP_NAME   "hisilicon,isp"

#define LOGON_DBG       (1 << 0)
#define LOGON_INF       (1 << 1)
#define LOGON_ERR       (1 << 2)
#define LOGON_WAR       (1 << 3)

static unsigned int communicat_msg[8] = {0};
struct rproc_shared_para *isp_share_para;
static int perf_para;
static unsigned int hisplog_mask = LOGON_INF | LOGON_ERR;
#ifdef DEBUG_HISP
module_param_named(perf_para, perf_para, int, 0644);
module_param_named(hisplog_mask, hisplog_mask, uint, 0664);
#endif

#define hisp_min(a, b) (((a) < (b)) ? (a) : (b))

#define RPMSG_RX_FIFO_DEEP  257
#define MBOX_REG_COUNT      8
#define RPROC_LOG_SIZE      128

#define TIMEOUT_IS_FPGA_BOARD        10000
#define TIMEOUT_IS_NOT_FPGA_BOARD    5000

enum isp_power_state {
	HISP_ISP_POWER_OFF      = 0,
	HISP_ISP_POWER_ON       = 1,
	HISP_ISP_POWER_FAILE    = 2,
	HISP_ISP_POWER_CLEAN    = 3,
};

enum isp_rp_mbox_messages {
	RP_MBOX_READY           = 0xFFFFFF00,
	RP_MBOX_PENDING_MSG     = 0xFFFFFF01,
	RP_MBOX_CRASH           = 0xFFFFFF02,
	RP_MBOX_ECHO_REQUEST    = 0xFFFFFF03,
	RP_MBOX_ECHO_REPLY      = 0xFFFFFF04,
	RP_MBOX_ABORT_REQUEST   = 0xFFFFFF05,
};

struct rproc_boot_device rproc_boot_dev;

int last_boot_state;

struct isp_rx_mbox {
	struct kfifo rpmsg_rx_fifo;
	spinlock_t rpmsg_rx_lock;
	wait_queue_head_t wait;
	struct task_struct *rpmsg_rx_tsk;
	int can_be_wakeup;
} *isp_rx_mbox;

struct rx_buf_data {
	bool is_have_data;
	unsigned int rpmsg_rx_len;
	mbox_msg_t rpmsg_rx_buf[MBOX_REG_COUNT];
};

struct rproc *isp_rproc;

static void hisplog(unsigned int on, const char *tag,
			const char *func, const char *fmt, ...)
{
	va_list args;
	char pbuf[RPROC_LOG_SIZE] = {0};
	int size;

	if ((hisplog_mask & on) == 0 || fmt == NULL)
		return;

	va_start(args, fmt);
	size = vsnprintf_s(pbuf, RPROC_LOG_SIZE, RPROC_LOG_SIZE - 1, fmt, args);
	va_end(args);

	if ((size < 0) || (size > RPROC_LOG_SIZE - 1))
		return;

	pbuf[size] = '\0';
	pr_info("Hisp rprocDrv [%s][%s] %s", tag, func, pbuf);
}

#define rproc_debug(fmt, ...) \
	hisplog(LOGON_DBG, "D", __func__, fmt, ##__VA_ARGS__)
#define rproc_info(fmt, ...) \
	hisplog(LOGON_INF, "I", __func__, fmt, ##__VA_ARGS__)
#define rproc_err(fmt, ...) \
	hisplog(LOGON_ERR, "E", __func__, fmt, ##__VA_ARGS__)

void hisp_lock_sharedbuf(void)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	mutex_lock(&dev->sharedbuf_mutex);
}

void hisp_unlock_sharedbuf(void)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	mutex_unlock(&dev->sharedbuf_mutex);
}

int is_ispcpu_powerup(void)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	return dev->ispcpu_status;
}

int hisp_use_logb(void)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	return dev->use_logb_flag;
}

struct device *get_isp_device(void)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	if (dev->isp_pdev == NULL) {
		pr_err("[%s] Failed: isp_pdev is NULL!\n", __func__);
		return NULL;
	}

	return &dev->isp_pdev->dev;
}

void hisp_rproc_init(struct rproc *rproc)
{
	if (rproc == NULL) {
		pr_err("%s: rproc is NULL\n", __func__);
		return;
	}

	if (use_sec_isp() || is_use_loadbin())
		rproc->fw_ops = &rproc_bin_fw_ops;

	pr_info("[%s] elf.%pK, bin.%pK, use.%pK\n", __func__,
		(void *)&rproc_elf_fw_ops, (void *)&rproc_bin_fw_ops,
		(void *)rproc->fw_ops);

	rproc->auto_boot = false;
	INIT_LIST_HEAD(&rproc->dynamic_mems);
	INIT_LIST_HEAD(&rproc->reserved_mems);
	INIT_LIST_HEAD(&rproc->caches);
	INIT_LIST_HEAD(&rproc->pages);
}

void hisp_free_rsctable(void)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;

	if (rproc_dev->rsctable_vaddr != NULL) {
		kfree(rproc_dev->rsctable_vaddr);
		rproc_dev->rsctable_vaddr = NULL;
	}
}

struct rproc_shared_para *rproc_get_share_para(void)
{
	pr_debug("%s: enter.\n", __func__);
	if (isp_share_para != NULL)
		return isp_share_para;

	pr_debug("%s: failed.\n", __func__);
	return NULL;
}

void set_shared_mdc_pa_addr(u64 mdc_pa_addr)
{
	struct rproc_shared_para *share_para = NULL;

	hisp_lock_sharedbuf();
	if (use_sec_isp())
		share_para = getsec_a7sharedmem_addr();
	else if (use_nonsec_isp())
		share_para = get_a7sharedmem_va();

	if (share_para == NULL) {
		pr_err("[%s] Failed : share_para.%pK\n", __func__, share_para);
		hisp_unlock_sharedbuf();
		return;
	}

	share_para->mdc_pa_addr = mdc_pa_addr;
	hisp_unlock_sharedbuf();
}

int hispmdc_map_fw(unsigned long iova, phys_addr_t paddr, size_t size, int prot)
{
	struct iommu_domain *domain = NULL;
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	int ret = 0;

	if (rproc_dev->isp_pdev == NULL) {
		pr_err("%s: isp_pdev is NULL\n", __func__);
		return 0;
	}

	domain = iommu_get_domain_for_dev(&rproc_dev->isp_pdev->dev);
	if (!domain)
		return -EINVAL;
	pr_err("[ISP Map] mdc unmap :iova.0x%lx, paddr.0x%lx, size.0x%lx, prot.0x%x, ret.%d\n",
		iova, paddr, size, prot, ret);

	ret = iommu_map(domain, iova, paddr, size, prot);
	if (ret) {
		pr_err("[%s] Failed :iova.0x%lx, paddr.0x%lx, size.0x%lx, prot.0x%x, ret.%d\n",
			__func__, iova, paddr, size, prot, ret);
		return -EINVAL;
	}

	return 0;
}

int hispmdc_unmap_fw(unsigned long iova, size_t size)
{
	struct iommu_domain *domain = NULL;
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	size_t unmapped = 0;
	struct device *subdev = NULL;

	if (rproc_dev->isp_pdev == NULL) {
		pr_err("%s: isp_pdev is NULL\n", __func__);
		return 0;
	}

	subdev = get_isp_device();
	if (subdev == NULL) {
		pr_err("%s: platform dev is NULL!\n", __func__);
		return 0;
	}

	domain = iommu_get_domain_for_dev(&rproc_dev->isp_pdev->dev);
	if (!domain)
		return -EINVAL;

	pr_info("[ISP Map] mdc map addr.0x%llx, size.0x%lx", iova, size);
	if (hisp_get_smmuc3_flag() == 1)
		unmapped = hisi_iommu_unmap_fast(subdev, iova, size);
	else
		unmapped = iommu_unmap(domain, iova, size);

	if (unmapped != size) {
		pr_err("[%s] Failed : ret.0x%lx\n", __func__, unmapped);
		return -EINVAL;
	}

	return 0;
}

void rproc_memory_cache_flush(struct rproc *rproc)
{
	struct rproc_cache_entry *tmp = NULL;

	list_for_each_entry(tmp, &rproc->caches, node)
		__flush_dcache_area(tmp->va, tmp->len);
}

static int get_a7log_mode(void)
{
	struct device_node *np = NULL;
	int ret = 0, a7log_mode = 0;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,prktimer");
	if (np == NULL) {
		pr_err("%s: no device node 'prktimer'!\n", __func__);
		return -ENXIO;
	}

	ret = of_property_read_u32(np, "fpga_flag", &a7log_mode);/*lint !e64*/
	if (ret) {
		pr_err("%s: failed to get fpga_flag resource.\n", __func__);
		return -ENXIO;
	}

	return a7log_mode;
}
u32 get_share_exc_flag(void)
{
	struct rproc_shared_para *share_para = NULL;
	u32 exc_flag = 0x0;

	hisp_lock_sharedbuf();
	share_para = rproc_get_share_para();
	if (share_para == NULL) {
		rproc_err("Failed : rproc_get_share_para.%pK\n", share_para);
		hisp_unlock_sharedbuf();
		return ISP_MAX_NUM_MAGIC;
	}
	exc_flag = share_para->exc_flag;
	hisp_unlock_sharedbuf();
	return exc_flag;
}

int set_plat_parameters(void)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;
	struct rproc_shared_para *param = NULL;

	hisp_lock_sharedbuf();
	param = rproc_get_share_para();
	if (param == NULL) {
		rproc_err("Failed : rproc_get_share_para.%pK\n", param);
		hisp_unlock_sharedbuf();
		return -EINVAL;
	}

	param->plat_cfg.perf_power      = perf_para;
	param->plat_cfg.platform_id     = hw_is_fpga_board();
	param->plat_cfg.isp_local_timer = dev->tmp_plat_cfg.isp_local_timer;
	param->rdr_enable_type         |= ISPCPU_RDR_USE_APCTRL; /*lint !e648*/
	param->logx_switch             |= ISPCPU_LOG_USE_APCTRL;
	param->exc_flag                 = 0x0;
	if (get_a7log_mode())
		param->logx_switch         |= ISPCPU_LOG_TIMESTAMP_FPGAMOD;

	rproc_info("platform_id = %d, isp_local_timer = %d\n",
		param->plat_cfg.platform_id, param->plat_cfg.isp_local_timer);
	rproc_info("perf_power = %d, logx_switch.0x%x\n",
		param->plat_cfg.perf_power, param->logx_switch);
	hisp_unlock_sharedbuf();

	isploglevel_update();
	return 0;
}

int rproc_set_shared_para(void)
{
	struct rproc_shared_para *share_para = NULL;
	int ret, i;

	ret = set_plat_parameters();
	if (ret) {
		pr_err("%s: set_plat_parameters failed.\n", __func__);
		return ret;
	}

	hisp_lock_sharedbuf();
	share_para = rproc_get_share_para();
	if (share_para == NULL) {
		pr_err("%s:rproc_get_share_para failed.\n", __func__);
		hisp_unlock_sharedbuf();
		return -EINVAL;
	}

	share_para->bootware_paddr = get_a7remap_addr();

	if (get_isprdr_addr())
		share_para->rdr_enable = 1;

	share_para->rdr_enable_type |= RDR_CHOOSE_TYPE;
	for (i = 0; i < IRQ_NUM; i++)
		share_para->irq[i] = 0;

	pr_info("%s: platform_id = 0x%x, timer = 0x%x\n",
			__func__, share_para->plat_cfg.platform_id,
			share_para->plat_cfg.isp_local_timer);
	pr_info("%s: rdr_enable = %d, rdr_enable_type = %d\n",
			__func__, share_para->rdr_enable,
			share_para->rdr_enable_type);
	hisp_unlock_sharedbuf();

	return ret;
}

void rproc_set_shared_clk_value(int type, unsigned int value)
{
	struct rproc_shared_para *share_para = NULL;

	if ((type >= ISP_CLK_MAX) || (type < 0)) {
		pr_err("%s:type error.%d\n", __func__, type);
		return;
	}

	hisp_lock_sharedbuf();
	share_para = rproc_get_share_para();
	if (share_para == NULL) {
		pr_err("%s:rproc_get_share_para failed.\n", __func__);
		hisp_unlock_sharedbuf();
		return;
	}
	share_para->clk_value[type] = value;
	pr_debug("%s: type.%d = %u\n", __func__, type, value);
	hisp_unlock_sharedbuf();
}

static void rproc_set_shared_clk_init(void)
{
	struct rproc_shared_para *share_para = NULL;
	int ret = 0;

	hisp_lock_sharedbuf();
	share_para = rproc_get_share_para();
	if (share_para == NULL) {
		pr_err("%s:rproc_get_share_para failed.\n", __func__);
		hisp_unlock_sharedbuf();
		return;
	}
	ret = memset_s(share_para->clk_value, sizeof(share_para->clk_value),
			0, sizeof(share_para->clk_value));
	if (ret != 0)
		pr_err("%s:memset_s share_para->clk_value to 0 failed.ret.%d\n",
				__func__, ret);
	hisp_unlock_sharedbuf();
}

static void hisp_efuse_deskew(void)
{
	int ret = 0;
	unsigned char efuse = 0xFF;
	struct rproc_shared_para *share_para = NULL;
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;

	rproc_info("+\n");
	if (rproc_dev->probe_finished != 1) {
		rproc_err("isp_rproc_probe failed\n");
		return;
	}

	if (rproc_dev->isp_efuse_flag == 0) {
		rproc_err("isp_efuse_flag.%d\n", rproc_dev->isp_efuse_flag);
		return;
	}

	ret = get_efuse_deskew_value(&efuse, 1, 1000);
	if (ret < 0)
		pr_err("[%s] Failed: ret.%d\n", __func__, ret);

	pr_info("[%s] : efuse.%d\n", __func__, ret);
	hisp_lock_sharedbuf();
	share_para = rproc_get_share_para();
	if (share_para == NULL) {
		pr_err("%s: rproc_get_share_para failed.\n", __func__);
		hisp_unlock_sharedbuf();
		return;
	}
	share_para->isp_efuse = efuse;
	hisp_unlock_sharedbuf();

	rproc_info("-\n");
}

int use_nonsec_isp(void)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	return ((dev->case_type == NONSEC_CASE) ? 1:0);
}

int use_sec_isp(void)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	return ((dev->case_type == SEC_CASE) ? 1:0);
}

int hisp_rproc_case_set(enum hisp_rproc_case_attr type)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	if (type >= INVAL_CASE) {
		pr_err("%s: invalid case, type = %u\n", __func__, type);
		return -EINVAL;
	}

	if (atomic_read(&dev->rproc_enable_status) > 0) {
		pr_err("[%s] hisp_rproc had been enabled\n", __func__);
		pr_err("[%s] rproc_enable_status.0x%x\n", __func__,
				atomic_read(&dev->rproc_enable_status));
		return -ENODEV;
	}

	if (sync_isplogcat() < 0)
		pr_err("[%s] Failed: sync_isplogcat\n", __func__);

	dev->case_type = type;

	if (type == SEC_CASE)
		isp_ispsec_share_para_set();

	if (isp_rproc != NULL) { /*lint !e730 */
		if ((type == SEC_CASE) || is_use_loadbin())
			isp_rproc->fw_ops = &rproc_bin_fw_ops;
		else
			isp_rproc->fw_ops = &rproc_elf_fw_ops;
	}

	pr_info("%s.%d: type.%u, rporc.%pK\n", __func__,
			__LINE__, type, isp_rproc);

	pr_info("[%s] elf_ops.%pK, bin_ops.%pK, use_ops.%pK\n",
			__func__, (void *)&rproc_elf_fw_ops,
			(void *)&rproc_bin_fw_ops,
			isp_rproc ? (void *)isp_rproc->fw_ops : NULL);

	return 0;
}

enum hisp_rproc_case_attr hisp_rproc_case_get(void)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	return dev->case_type;
}

char *hisp_get_comp_name(void)
{
	return DTS_COMP_NAME;
}

int hisp_set_clk_rate(unsigned int type, unsigned int rate)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;
	int ret = -EINVAL;

	if (dev->probe_finished != 1) {
		rproc_err("isp_rproc_probe failed\n");
		return -EPERM;
	}

	if (!check_dvfs_valid()) {
		pr_err("[%s] Failed : check_dvfs_valid\n", __func__);
		return -EINVAL;
	}

	switch (dev->case_type) {
	case NONSEC_CASE:
		ret = hisp_pwr_nsec_setclkrate(type, rate);
		if (ret < 0)
			pr_err("[%s] Failed : hisp_pwr_nsec_setclkrate.%d\n",
				__func__, ret);
		break;
	case SEC_CASE:
		ret = hisp_pwr_sec_setclkrate(type, rate);
		if (ret < 0)
			pr_err("[%s] Failed : hisp_pwr_nsec_setclkrate.%d\n",
				__func__, ret);
		break;
	default:
		pr_err("[%s] Unsupported case_type.%d\n",
				__func__, dev->case_type);
		return -EINVAL;
	}

	return ret;
}

unsigned long hisp_get_clk_rate(unsigned int type)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	if (dev->probe_finished != 1) {
		rproc_err("isp_rproc_probe failed\n");
		return 0;
	}

	if (get_rproc_enable_status() == 0) {
		pr_err("[%s] ispcpu not start!\n", __func__);
		return 0;
	}

	return hisp_pwr_getclkrate(type);
}

int hisp_rproc_setpinctl(struct pinctrl *isp_pinctrl,
	struct pinctrl_state *pinctrl_def, struct pinctrl_state *pinctrl_idle)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;

	if (hw_is_fpga_board()) {
		rproc_err("fpga board, no need to init pinctrl.\n");
		return 0;
	}

	rproc_dev->isp_data.isp_pinctrl = isp_pinctrl;
	rproc_dev->isp_data.pinctrl_def = pinctrl_def;
	rproc_dev->isp_data.pinctrl_idle = pinctrl_idle;

	return 0;
}

int hisp_rproc_setclkdepend(struct clk *aclk_dss, struct clk *pclk_dss)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;

	if (hw_is_fpga_board()) {
		rproc_err("fpga board, no need to init clkdepend.\n");
		return 0;
	}

	rproc_dev->isp_data.aclk_dss = aclk_dss;
	rproc_dev->isp_data.pclk_dss = pclk_dss;

	return 0;
}

u64 get_a7sharedmem_addr(void)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	if (use_nonsec_isp())
		return (dev->nsec_share_addr);
	else
		return (dev->remap_addr + 0xF000);
}

void set_a7sharedmem_pa(u64 addr)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	dev->nsec_share_addr = addr;
}

void *get_a7sharedmem_va(void)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	if (dev->share_va == NULL)
		return NULL;

	return dev->share_va;
}

void set_a7sharedmem_va(void *addr)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	dev->share_va = addr;
	pr_info("%s: share_va = %pK\n", __func__, dev->share_va);
}

u64 get_a7remap_addr(void)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	if (use_nonsec_isp())
		return dev->nsec_remap_addr;
	else
		return dev->remap_addr;
}

void set_a7mem_pa(u64 addr)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	dev->nsec_remap_addr = addr;
}

static void isp_device_disable_prepare(void)
{
	int ret = -1;
	struct rproc_boot_device *dev = &rproc_boot_dev;

	ret = hisp_mntn_dumpregs();
	if (ret < 0)
		rproc_err("Failed : hisp_mntn_dumpregs.%d\n", ret);
	if (get_ispcpu_idle_stat(dev->isppd_adb_flag) < 0) {
		if (dev->isppd_adb_flag)
			dump_hisp_boot(dev->isp_rproc->rproc,
				DUMP_ISP_BOOT_SIZE);
	}
	stop_isplogcat();
}

static int isp_device_disable(void)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;
	int ret;

	rproc_info("+\n");
	if (dev->ispcpu_status == 0) {
		pr_err("%s: isp already off\n", __func__);
		return 0;
	}
	switch (dev->case_type) {
	case SEC_CASE:
		isp_device_disable_prepare();
		ret = secisp_device_disable();
		if (ret < 0)
			rproc_err("Failed : secisp_device_disable.%d\n", ret);
		break;

	case NONSEC_CASE:
		isp_device_disable_prepare();
		ret = nonsec_isp_device_disable();
		if (ret < 0)
			rproc_err("Failed:nonsec_isp_device_disable.%d\n", ret);
		break;

	default:
		rproc_err("Failed : not supported type.%d%d\n", dev->case_type);
		return -EINVAL;
	}
	dev->ispcpu_status = 0;
	clear_isplog_info();
	if (ret != 0)
		rproc_err("ispcpu power down fail.%d, case_type.%d\n",
				ret, dev->case_type);
	mutex_lock(&dev->ispcpu_mutex);
	if (dev->ispcpu_wakelock.active) {
		__pm_relax(&dev->ispcpu_wakelock);
		rproc_info("ispcpu power up wake unlock.\n");
	}
	mutex_unlock(&dev->ispcpu_mutex);/*lint !e456 */
	rproc_info("-\n");

	return ret;
}

/* enable rproc a7 and isp core*/
/*lint -save -e838 -e454 -e578*/
static int isp_device_enable(void)
{
#define TIMEOUT_ISPLOG_START (10)
	struct rproc_boot_device *dev = &rproc_boot_dev;
	int ret = -ENOMEM, timeout = 0;

	rproc_info("+\n");
	last_boot_state = 0;
	mutex_lock(&dev->ispcpu_mutex);
	if (!dev->ispcpu_wakelock.active) {
		__pm_stay_awake(&dev->ispcpu_wakelock);
		rproc_info("ispcpu power up wake lock.\n");
	}
	mutex_unlock(&dev->ispcpu_mutex);/*lint !e456 */
	timeout = TIMEOUT_ISPLOG_START;
	do {
		ret = start_isplogcat();
		if (ret < 0)
			rproc_err("Failed:secisp_device_enable.%d,timeout.%d\n",
					ret, timeout);
	} while (ret < 0 && timeout-- > 0);

	switch (dev->case_type) {
	case SEC_CASE:
		ret = secisp_device_enable();
		if (ret < 0)
			rproc_err("Failed : secisp_enable.%d\n", ret);
		break;
	case NONSEC_CASE:
		ret = nonsec_isp_device_enable(dev->isp_rproc->rproc);
		if (ret < 0)
			rproc_err("Failed : nonsec_isp_enable.%d\n", ret);
		break;
	default:
		rproc_err("Failed : not supported type.%d, ret.%d\n",
			dev->case_type, ret);
		break;
	}
	dev->ispcpu_status = 1;
	rproc_info("-\n");

	if (ret != 0) {
		rproc_err("ispcpu power up fail.%d, case_type.%d\n",
				ret, dev->case_type);
		mutex_lock(&dev->ispcpu_mutex);
		if (dev->ispcpu_wakelock.active) {
			__pm_relax(&dev->ispcpu_wakelock);
			rproc_info("ispcpu power up wake unlock.\n");
		}
		mutex_unlock(&dev->ispcpu_mutex);/*lint !e456 */
	}
	return ret;
}

static int hisp_power_on_work(void *data)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;
	struct cpumask cpu_mask;
	int ret;

	rproc_info("+\n");
	set_user_nice(current, (int)(-1 * (dev->pwronisp_nice)));
	hisi_get_slow_cpus(&cpu_mask);
	cpumask_clear_cpu(0, &cpu_mask);

	if (sched_setaffinity(current->pid, &cpu_mask) < 0)
		rproc_err("Couldn't set affinity to cpu\n");

	ret = hisp_sec_pwron_prepare();
	if (ret != 0) {
		rproc_err("hisp_sec_pwron_prepare fail.%d\n", ret);
		return ret;
	}

	rproc_info("-\n");
	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(dev->pwronisp_wait,
			dev->pwronisp_wake);
		if (ret < 0) {
			rproc_info("fail to wait for pwronisp_wake.%ld\n", ret);
			continue;
		}

		ret = isp_device_enable();
		if (ret != 0)
			rproc_err("isp_device_enable fail.%d\n", ret);

		dev->pwronisp_wake = 0;
	}

	return 0;
}
/*lint -restore */

static void isp_mbox_rx_work(void)
{
	struct rx_buf_data mbox_reg;
	int ret;
	irqreturn_t irqret;

	while (kfifo_len(&isp_rx_mbox->rpmsg_rx_fifo)
			>= sizeof(struct rx_buf_data)) {
		ret = kfifo_out_locked(&isp_rx_mbox->rpmsg_rx_fifo,
					(unsigned char *)&mbox_reg,
					sizeof(struct rx_buf_data),
					&isp_rx_mbox->rpmsg_rx_lock);
		if (ret < 0) {/*lint !e84 */
			rproc_err("Failed : kfifo_out_locked.%d\n", ret);
			return;
		}

		/* maybe here need the flag of is_have_data */
		irqret = rproc_vq_interrupt(isp_rproc,
				mbox_reg.rpmsg_rx_buf[0]);
		if (irqret == IRQ_NONE)
			rproc_debug("no message was found in vqid\n");
	}
}

static int isp_mbox_rx_thread(void *context)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	struct rx_buf_data mbox_reg;
	int ret = 0;

	rproc_info("+\n");
	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(isp_rx_mbox->wait,
				isp_rx_mbox->can_be_wakeup == 1);
		isp_rx_mbox->can_be_wakeup = 0;
		if (ret != 0) {
			rproc_err("wait event failed\n");
			continue;
		}
		mutex_lock(&rproc_dev->hisp_power_mutex);
		if (rproc_dev->hisp_power_state == HISP_ISP_POWER_OFF) {
			pr_err("[%s] hisp_rproc disable no power.0x%x\n",
				__func__, rproc_dev->hisp_power_state);
			while (kfifo_len(&isp_rx_mbox->rpmsg_rx_fifo)
				>= sizeof(struct rx_buf_data)) {/*lint !e84 */
				ret = kfifo_out_locked(
						&isp_rx_mbox->rpmsg_rx_fifo,
						(unsigned char *)&mbox_reg,
						sizeof(struct rx_buf_data),
						&isp_rx_mbox->rpmsg_rx_lock);
				if (ret < 0)
					rproc_err("Failed:kfifo_out.%d\n", ret);
			}
			mutex_unlock(&rproc_dev->hisp_power_mutex);
			continue;
		}
		hisp_recvthread();
		isp_mbox_rx_work();
		mutex_unlock(&rproc_dev->hisp_power_mutex);
	}
	rproc_info("-\n");

	return 0;
}

static int isp_rproc_mbox_kfifo(mbox_msg_t *msg)
{
	unsigned int i;
	int ret;
	struct rx_buf_data mbox_reg;

	/* [ptr] msg : out layer not NULL*/
	rproc_debug("default.%d\n", msg[0]);

	mbox_reg.rpmsg_rx_len = MBOX_REG_COUNT;
	for (i = 0; i < mbox_reg.rpmsg_rx_len; i++)
		mbox_reg.rpmsg_rx_buf[i] = msg[i];

	if (kfifo_avail(&isp_rx_mbox->rpmsg_rx_fifo) <
			sizeof(struct rx_buf_data)) {/*lint !e84 */
		rproc_err("rpmsg_rx_fifo is full\n");
		return -1;
	}

	ret = kfifo_in_locked(&isp_rx_mbox->rpmsg_rx_fifo,
				(unsigned char *)&mbox_reg,
				sizeof(struct rx_buf_data),
				&isp_rx_mbox->rpmsg_rx_lock);
	if (ret <= 0) {
		rproc_err("kfifo_in_locked failed\n");
		return -1;
	}
	rproc_debug("kfifo_in_locked success!\n");

	return 0;
}

int isp_rproc_mbox_callback(struct notifier_block *this,
					unsigned long len, void *data)
{
	mbox_msg_t *msg = (mbox_msg_t *)data;
	struct rproc_boot_device *dev = &rproc_boot_dev;
	int ret;

	if (dev->probe_finished != 1) {
		rproc_err("isp_rproc_probe failed\n");
		return -EPERM;
	}

	if (msg == NULL) {
		pr_err("%s: msg is NULL\n", __func__);
		return -EINVAL;
	}

	spin_lock_bh(&dev->rpmsg_ready_spin_mutex);
	if (dev->rpmsg_ready_state == 0) {
		rproc_info("isp is powered off state\n");
		spin_unlock_bh(&dev->rpmsg_ready_spin_mutex);/*lint !e456 */
		return NOTIFY_DONE;
	}

	hisp_recvtask();
	switch (msg[0]) {
	case RP_MBOX_CRASH:/*lint !e650 */
		rproc_info("hisp rproc crashed\n");
		break;

	case RP_MBOX_ECHO_REPLY:/*lint !e650 */
		rproc_info("received echo reply\n");
		break;

	default:
		ret = isp_rproc_mbox_kfifo(msg);
		if (ret < 0) {
			rproc_err("isp_rproc_mbox_kfifo failed\n");
			spin_unlock_bh(&dev->rpmsg_ready_spin_mutex);
			return ret;
		}

		isp_rx_mbox->can_be_wakeup = 1;
		wake_up_interruptible(&isp_rx_mbox->wait);
	}
	spin_unlock_bh(&dev->rpmsg_ready_spin_mutex);/*lint !e456 */
	rproc_debug("----%s rx msg X----\n", __func__);

	return NOTIFY_DONE;
}
EXPORT_SYMBOL(isp_rproc_mbox_callback);

int hisp_wait_rpmsg_completion(void)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	unsigned int timeout = hw_is_fpga_board() ?
		TIMEOUT_IS_FPGA_BOARD : TIMEOUT_IS_NOT_FPGA_BOARD;
	int ret;

	ret = wait_for_completion_timeout(&rproc_dev->rpmsg_boot_comp,
		msecs_to_jiffies(timeout));
	if (ret == 0) {
		rproc_err("Failed : wait_for_completion_timeout\n");
		return -ETIME;
	}

	spin_lock_bh(&rproc_dev->rpmsg_ready_spin_mutex);
	rproc_dev->rpmsg_ready_state = 1;
	spin_unlock_bh(&rproc_dev->rpmsg_ready_spin_mutex);/*lint !e456 */

	return 0;
}

static int isp_ipc_resource_init(void)
{
	int ret;

	rproc_info("+\n");
	isp_rx_mbox = kzalloc(sizeof(struct isp_rx_mbox), GFP_KERNEL);
	if (isp_rx_mbox == NULL) {
		rproc_err("Failed : kzalloc isp_rx_mbox\n");
		return -ENOMEM;
	}

	init_waitqueue_head(&isp_rx_mbox->wait);
	isp_rx_mbox->can_be_wakeup = 0;
	isp_rx_mbox->rpmsg_rx_tsk =
		kthread_create(isp_mbox_rx_thread, NULL, "rpmsg_tx_tsk");

	if (unlikely(IS_ERR(isp_rx_mbox->rpmsg_rx_tsk))) {
		rproc_err("Failed : create kthread tx_kthread\n");
		ret = -EINVAL;
		goto kthread_failure;
	} else {
		struct sched_param param;

		param.sched_priority = (MAX_RT_PRIO - 25);

		ret = sched_setscheduler(isp_rx_mbox->rpmsg_rx_tsk,
					SCHED_RR, &param);
		if (ret < 0) {
			rproc_err("Failed : sched_setscheduler\n");
			goto kthread_failure;
		}
		wake_up_process(isp_rx_mbox->rpmsg_rx_tsk);
	}

	spin_lock_init(&isp_rx_mbox->rpmsg_rx_lock);

	if (kfifo_alloc(&isp_rx_mbox->rpmsg_rx_fifo,
			sizeof(struct rx_buf_data) * RPMSG_RX_FIFO_DEEP,
			GFP_KERNEL)) {
		rproc_err("Failed : kfifo_alloc\n");
		ret = -ENOMEM;
		goto kfifo_failure;
	}

	rproc_info("-\n");
	return 0;
kfifo_failure:
	if (IS_ERR(isp_rx_mbox->rpmsg_rx_tsk))
		kthread_stop(isp_rx_mbox->rpmsg_rx_tsk);
kthread_failure:
	kfree(isp_rx_mbox);
	isp_rx_mbox = NULL;

	return ret;
}

static void isp_ipc_resource_exit(void)
{
	kfifo_free(&isp_rx_mbox->rpmsg_rx_fifo);
	if (IS_ERR(isp_rx_mbox->rpmsg_rx_tsk))
		kthread_stop(isp_rx_mbox->rpmsg_rx_tsk);
	kfree(isp_rx_mbox);
	isp_rx_mbox = NULL;
}

int isp_rproc_start(struct rproc *rproc)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	int ret;

	rproc_info("+\n");
	ret = rpmsg_vdev_map_resource(rproc);
	if (ret != 0) {
		rproc_err("Failed to map vdev resource: %d\n", ret);
		return ret;
	}

	rproc_dev->pwronisp_wake = 1;
	wake_up(&rproc_dev->pwronisp_wait);

	/* set shared parameters for rproc*/
	ret = rproc_set_shared_para();
	if (ret != 0) {
		rproc_err("Failed to set bootware parameters...: %d\n", ret);
		return ret;
	}

	ret = hisp_rproc_pgd_set(rproc);
	if (ret != 0) {
		rproc_err("Failed : hisp_rproc_pgd_set.%d\n", ret);
		return ret;
	}

	hisp_efuse_deskew();
	rproc_set_shared_clk_init();
	rproc_info("-\n");

	return 0;
}

int isp_rproc_stop(struct rproc *rproc)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;

	rproc_info("+\n");
	RPROC_FLUSH_TX(rproc_dev->ap_a7_mbox);/*lint !e64 */

	isp_device_disable();
	rproc_info("-\n");
	return 0;
}

void isp_rproc_kick(struct rproc *rproc, int vqid)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	int ret;

	if (rproc_dev->probe_finished != 1) {
		rproc_err("isp_rproc_probe failed\n");
		return;
	}

	/* Send the index of the triggered virtqueue in the mailbox payload */
	communicat_msg[0] = vqid;

	rproc_debug("+.0x%x\n", communicat_msg[0]);
	ret = RPROC_ASYNC_SEND(rproc_dev->ap_a7_mbox,
			communicat_msg, sizeof(communicat_msg[0]));
	if (ret != 0)
		rproc_err("Failed: RPROC_ASYNC_SEND.%d\n", ret);
	hisp_sendx();
	communicat_msg[0] = 0;
	rproc_debug("-\n");
}

/* rproc private da to va */
void *hisp_rproc_da_to_va(struct rproc *rproc, u64 da, int len)
{
	struct rproc_mem_entry *dynamic_mem = NULL, *reserved_mem = NULL;
	void *ptr = NULL;

	list_for_each_entry(reserved_mem, &rproc->reserved_mems, node) {
		int offset = da - reserved_mem->da;

		/* try next if da is too small */
		if (offset < 0)
			continue;

		/* try next if da is too large */
		if (offset + len > reserved_mem->len)
			continue;

		ptr = reserved_mem->va + offset;

		return ptr;
	}

	list_for_each_entry(dynamic_mem, &rproc->dynamic_mems, node) {
		int offset = da - dynamic_mem->da;

		/* try next if da is too small */
		if (offset < 0)
			continue;

		/* try next if da is too large */
		if (offset + len > dynamic_mem->len)
			continue;

		ptr = dynamic_mem->va + offset;

		return ptr;
	}

	return ptr;
}

static int hisp_request_bin_rsctable(void)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;

	pr_info("[%s] +\n", __func__);

	rproc_dev->rsctable_vaddr = hisp_loadbin_rsctable_init();
	if (rproc_dev->rsctable_vaddr == NULL) {
		pr_err("%s : hisp_loadbin_rsctable_init failed.\n", __func__);
		return -ENOMEM;
	}
	pr_info("[%s] -\n", __func__);

	return 0;
}

int isp_rproc_request_image(struct rproc *rproc,
	const struct firmware **firmware_p, struct device *dev)
{
	int ret = 0;

	if (rproc == NULL || dev == NULL) {
		pr_err("[%s] para rproc or dev is NULL\n", __func__);
		return -EINVAL;
	}

	if (use_sec_isp()) {
		ret = hisp_request_sec_rsctable();
		if (ret < 0)
			pr_err("[%s] Failed : hisp_rsctable_init.%d\n",
					__func__, ret);
		*firmware_p = NULL;
	} else if (is_use_loadbin()) {
		ret = hisp_request_bin_rsctable();
		if (ret < 0)
			pr_err("[%s] Failed : hisp_rproc_rsctable_init.%d\n",
					__func__, ret);
		*firmware_p = NULL;
	} else {
		/* loading a firmware is required */
		if (!rproc->firmware) {
			pr_err("[%s] Failed : no firmware to load\n", __func__);
			return -EINVAL;
		}

		ret = request_firmware(firmware_p, rproc->firmware, dev);
		if (ret < 0)
			pr_err("[%s] Failed :request_firmware %d\n",
				__func__, ret);
	}

	return ret;
}

void isp_rproc_release_image(const struct firmware *firmware_p)
{
	if (use_sec_isp() || is_use_loadbin())
		return;

	if (firmware_p != NULL)
		release_firmware(firmware_p);
}

int hisp_rproc_enable_iommu(struct rproc *rproc)
{
	struct iommu_domain *domain = NULL;
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;

	if (use_sec_isp())
		return 0;

	if (rproc == NULL) {
		pr_err("%s: rproc is NULL\n", __func__);
		return -EINVAL;
	}

	domain = iommu_get_domain_for_dev(&rproc_dev->isp_pdev->dev);
	if (domain == NULL)
		return -EINVAL;

	rproc->domain = domain;

	return 0; /*lint !e715 !e438*/
}

void hisp_rproc_disable_iommu(struct rproc *rproc)
{
	if (use_sec_isp())
		return;

	if (rproc == NULL) {
		pr_err("%s: rproc is NULL\n", __func__);
		return;
	}

	rproc->domain = NULL;
}

size_t hisp_rproc_unmap_iommu(struct rproc *rproc,
	struct rproc_mem_entry *entry)
{
	struct device *dev = NULL;
	size_t unmapped = 0;

	if (hisp_get_smmuc3_flag() == 1) {
		dev = get_isp_device();
		if (dev == NULL)
			pr_err("%s: platform dev is NULL!\n", __func__);

		unmapped = hisi_iommu_unmap_fast(dev, entry->da, entry->len);
	} else {
		unmapped = iommu_unmap(rproc->domain, entry->da, entry->len);
	}

	return unmapped;
}

static struct rproc_ops isp_rproc_ops = {
	.start             = isp_rproc_start,
	.stop              = isp_rproc_stop,
	.kick              = isp_rproc_kick,
	.da_to_va          = hisp_rproc_da_to_va,
	.request_image     = isp_rproc_request_image,
	.release_image     = isp_rproc_release_image,
	.enable_iommu      = hisp_rproc_enable_iommu,
	.disable_iommu     = hisp_rproc_disable_iommu,
	.unmap_iommu       = hisp_rproc_unmap_iommu,
	.resource_cleanup  = hisp_rproc_resource_cleanup,
};

int get_rproc_enable_status(void)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;

	return atomic_read(&rproc_dev->rproc_enable_status);
}

static bool rproc_get_sync_flag(void)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	struct rproc *rproc = rproc_dev->isp_rproc->rproc;

	return rproc->sync_flag;
}

void rproc_set_sync_flag(bool flag)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	struct rproc *rproc = rproc_dev->isp_rproc->rproc;

	rproc->sync_flag = flag;
}

int set_isp_remap_addr(u64 remap_addr)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;
	struct rproc_shared_para *param = NULL;

	if (sec_process_use_ca_ta())
		return 0;

	if (remap_addr == 0) {
		rproc_err("Failed : remap_addr.0\n");
		return -ENOMEM;
	}
	dev->remap_addr = remap_addr;
	hisp_lock_sharedbuf();
	param = rproc_get_share_para();
	if (param == NULL) {
		rproc_err("Failed : rproc_get_share_para\n");
		hisp_unlock_sharedbuf();
		return -ENOMEM;
	}
	param->bootware_paddr = remap_addr;
	hisp_unlock_sharedbuf();

	return 0;
}

void *hisp_get_rsctable(int *tablesz)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;

	if (use_sec_isp())
		return get_rsctable(tablesz);

	*tablesz = 0x00001000;
	return rproc_dev->rsctable_vaddr;
}

static int hisp_rproc_down_check(void)
{
	int err = 0;
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;

	if (rproc_dev->probe_finished != 1) {
		rproc_err("isp_rproc_probe failed\n");
		return -EPERM;
	}

	err = bypass_power_updn();
	if (err != 0) {/*lint !e838 */
		pr_err("[%s] bypass_power_updn.0x%x\n", __func__, err);
		return -ENODEV;
	}

	if (rproc_dev->isp_rproc == NULL) {
		rproc_info("Failed : isp_rproc.%pK\n", rproc_dev->isp_rproc);
		return -ENOMEM;
	}

	if (atomic_read(&rproc_dev->rproc_enable_status) == 0) {
		pr_err("[%s] Failed : rproc_enable_status.0\n",	__func__);
		return -ENODEV;
	}
	mutex_lock(&rproc_dev->hisp_power_mutex);
	if (rproc_dev->hisp_power_state == HISP_ISP_POWER_OFF) {
		pr_err("[%s] hisp_rproc disable no power.0x%x\n",
			__func__, rproc_dev->hisp_power_state);
		mutex_unlock(&rproc_dev->hisp_power_mutex);
		return -ENODEV;
	}
	mutex_unlock(&rproc_dev->hisp_power_mutex);

	return 0;
}

int hisp_rproc_disable(void)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	struct rproc *rproc = NULL;
	int err = 0;

	rproc_info("+\n");

	err = hisp_rproc_down_check();
	if (err < 0) {
		pr_err("[%s] Failed:hisp_rproc_down_check.%d\n", __func__, err);
		return err;
	}

	rproc = rproc_dev->isp_rproc->rproc;

	init_completion(&rproc->crash_comp);

	mutex_lock(&rproc_dev->hisp_power_mutex);
	rproc_dev->hisp_power_state = HISP_ISP_POWER_OFF;
	mutex_unlock(&rproc_dev->hisp_power_mutex);

	spin_lock_bh(&rproc_dev->rpmsg_ready_spin_mutex);
	rproc_dev->rpmsg_ready_state = 0;
	spin_unlock_bh(&rproc_dev->rpmsg_ready_spin_mutex);/*lint !e456 */

	rproc_shutdown(rproc);
	rproc->domain = NULL;

	rproc_set_sync_flag(true);
	if (atomic_read(&rproc_dev->rproc_enable_status) == 1)
		atomic_set(&rproc_dev->rproc_enable_status, 0);
	rproc_info("-\n");
	return err;
}
EXPORT_SYMBOL(hisp_rproc_disable);

static int hisp_rproc_up_check(void)
{
	int err;
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;

	if (rproc_dev->probe_finished != 1) {
		rproc_err("isp_rproc_probe failed\n");
		return -EPERM;
	}

	err = bypass_power_updn();
	if (err != 0) {/*lint !e838 */
		pr_err("[%s] bypass_power_updn.0x%x\n", __func__, err);
		return -ENODEV;
	}

	pr_info("%s.%d: case type.%u\n", __func__, __LINE__,
			hisp_rproc_case_get());

	if (rproc_dev->isp_rproc == NULL) {
		rproc_info("Failed : isp_rproc.%pK\n", rproc_dev->isp_rproc);
		return -ENOMEM;
	}
	if (atomic_read(&rproc_dev->rproc_enable_status) > 0) {
		pr_err("[%s] hisp_rproc had been enabled\n", __func__);
		pr_err("[%s] rproc_enable_status.0x%x\n", __func__,
			atomic_read(&rproc_dev->rproc_enable_status));
		return -ENODEV;
	}

	return 0;
}

static int wakeup_pwronisp_kthread(void)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;

	if (rproc_dev->pwronisp_thread_status == 1)
		return 0;

	if (rproc_dev->pwronisp == NULL) {
		rproc_err("Failed : pwronisp is NULL\n");
		return -ENODEV;
	}

	wake_up_process(rproc_dev->pwronisp);
	rproc_dev->pwronisp_thread_status = 1;

	return 0;
}

static int hisp_rproc_boot_config(void)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	struct rproc *rproc = NULL;
	int err = 0;

	rproc_info("+\n");

	err = hisp_rproc_up_check();
	if (err < 0) {
		pr_err("[%s] Failed:hisp_rproc_up_check.%d\n", __func__, err);
		return err;
	}

	atomic_set(&rproc_dev->rproc_enable_status, 1);

	err = wakeup_ispbin_kthread();
	if (err != 0) {
		rproc_err("Failed : wakeup_ispbin_kthread.0x%x\n", err);
		atomic_set(&rproc_dev->rproc_enable_status, 0);
		return -ENODEV;
	}

	err = wakeup_pwronisp_kthread();
	if (err != 0) {
		rproc_err("Failed : wakeup_pwronisp_kthread.0x%x\n", err);
		atomic_set(&rproc_dev->rproc_enable_status, 0);
		return -ENODEV;
	}

	err = isp_loadbin_debug_elf();
	if (err < 0) {
		atomic_set(&rproc_dev->rproc_enable_status, 0);
		return err;
	}

	if (!rproc_get_sync_flag()) {
		rproc_info("sync_flag exception.\n");
		atomic_set(&rproc_dev->rproc_enable_status, 0);
		return -EAGAIN;
	}

	rproc_set_sync_flag(false);
	reinit_completion(&rproc_dev->rpmsg_boot_comp);

	rproc = rproc_dev->isp_rproc->rproc;
	/* clean up the invalid exception entries */
	if (!list_empty(&rproc->rvdevs)) {
		rproc_err("Failed : enable exception will disable...\n");
		atomic_set(&rproc_dev->rproc_enable_status, 2);
		mutex_lock(&rproc_dev->hisp_power_mutex);
		rproc_dev->hisp_power_state = HISP_ISP_POWER_CLEAN;
		mutex_unlock(&rproc_dev->hisp_power_mutex);
		hisp_rproc_disable();
	}

	return 0;
}

int hisp_rproc_enable(void)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	struct rproc *rproc = NULL;
	int err = 0;

	err = hisp_rproc_boot_config();
	if (err < 0) {
		pr_err("[%s]Failed:hisp_rproc_boot_config.%d\n", __func__, err);
		return err;
	}

	rproc_info("rproc_enable...\n");

	rproc = rproc_dev->isp_rproc->rproc;
	err = rproc_boot(rproc);
	if (err) {
		rproc_err("Failed : rproc_boot.%d\n", err);
		goto enable_err;
	}

	if (!use_sec_isp())
		rproc_memory_cache_flush(rproc);

	rproc_set_sync_flag(true);

	mutex_lock(&rproc_dev->hisp_power_mutex);
	rproc_dev->hisp_power_state = HISP_ISP_POWER_ON;
	mutex_unlock(&rproc_dev->hisp_power_mutex);

	complete(&rproc_dev->rpmsg_boot_comp);
	rproc_info("-\n");
	return 0;

enable_err:
	mutex_lock(&rproc_dev->hisp_power_mutex);
	rproc_dev->hisp_power_state = HISP_ISP_POWER_FAILE;
	mutex_unlock(&rproc_dev->hisp_power_mutex);
	hisp_rproc_disable();

	return err;
}
EXPORT_SYMBOL(hisp_rproc_enable);

/*lint -save -e454*/
int hisp_jpeg_powerup(void)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;
	int ret = 0;

	if (dev->probe_finished != 1) {
		rproc_err("isp_rproc_probe failed\n");
		return -EPERM;
	}

	mutex_lock(&dev->jpeg_mutex);
	if (!dev->jpeg_wakelock.active) {
		__pm_stay_awake(&dev->jpeg_wakelock);
		rproc_info("jpeg power up wake lock.\n");
	}
	mutex_unlock(&dev->jpeg_mutex);/*lint !e456 */
	switch (dev->case_type) {
	case NONSEC_CASE:
		ret = hisp_nsec_jpeg_powerup();
		break;
	case SEC_CASE:
		ret = hisp_sec_jpeg_powerup();
		break;
	default:
		ret = -EINVAL;
		break;
	}
	if (ret != 0) {
		rproc_err("Failed : jpeg power up fail.%d, case_type.%d\n",
				ret, dev->case_type);
		mutex_lock(&dev->jpeg_mutex);
		if (dev->jpeg_wakelock.active) {
			__pm_relax(&dev->jpeg_wakelock);
			rproc_info("jpeg power up wake unlock.\n");
		}
		mutex_unlock(&dev->jpeg_mutex);/*lint !e456 */
	}
	return ret;
}
EXPORT_SYMBOL(hisp_jpeg_powerup);
/*lint -restore */

int hisp_jpeg_powerdn(void)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;
	int ret = 0;

	if (dev->probe_finished != 1) {
		rproc_err("isp_rproc_probe failed\n");
		return -EPERM;
	}

	switch (dev->case_type) {
	case NONSEC_CASE:
		ret = hisp_nsec_jpeg_powerdn();
		break;
	case SEC_CASE:
		ret = hisp_sec_jpeg_powerdn();
		break;
	default:
		ret = -EINVAL;
		break;
	}
	mutex_lock(&dev->jpeg_mutex);
	if (dev->jpeg_wakelock.active) {
		__pm_relax(&dev->jpeg_wakelock);
		rproc_info("jpeg power up wake unlock.\n");
	}
	mutex_unlock(&dev->jpeg_mutex);/*lint !e456 */
	if (ret != 0)
		rproc_err("Failed : jpeg power down fail.%d, case_type.%d\n",
				ret, dev->case_type);
	return ret;
}
EXPORT_SYMBOL(hisp_jpeg_powerdn);

int isp_rproc_select_def(void)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	int ret;

	rproc_info("+\n");
	if (rproc_dev->probe_finished != 1) {
		rproc_err("isp_rproc_probe failed\n");
		return -EPERM;
	}

	if (hw_is_fpga_board() == 0) {
		ret = pinctrl_select_state(rproc_dev->isp_data.isp_pinctrl,
					rproc_dev->isp_data.pinctrl_def);
		if (ret != 0) {
			rproc_err("Failed : not set pins to default state.\n");
			return ret;
		}
	}

	rproc_info("-\n");
	return 0;
}

int isp_rproc_select_idle(void)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	int ret;

	rproc_info("+\n");
	if (rproc_dev->probe_finished != 1) {
		rproc_err("isp_rproc_probe failed\n");
		return -EPERM;
	}

	if (hw_is_fpga_board() == 0) {
		ret = pinctrl_select_state(rproc_dev->isp_data.isp_pinctrl,
					rproc_dev->isp_data.pinctrl_idle);
		if (ret != 0) {
			rproc_err("Failed : not set pins to ilde state.\n");
			return ret;
		}
	}

	rproc_info("-\n");
	return 0;
}

int bypass_power_updn(void)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	return dev->bypass_pwr_updn;
}

void __iomem *get_regaddr_by_pa(unsigned int type)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	if (type >= hisp_min(HISP_MAX_REGTYPE, dev->reg_num)) {
		pr_err("[%s] unsupported type.0x%x\n", __func__, type);
		return NULL;
	}

	return dev->reg[type] ? dev->reg[type]:NULL; /*lint !e661 !e662 */
}

static int hisp_pwron_task_init(struct rproc_boot_device *rproc_dev)
{
	if (rproc_dev == NULL) {
		pr_err("[%s] Failed : rproc_dev.%pK\n", __func__, rproc_dev);
		return -ENOMEM;
	}

	rproc_dev->pwronisp = kthread_create(hisp_power_on_work,
				NULL, "pwronisp");
	if (IS_ERR(rproc_dev->pwronisp)) {
		pr_err("[%s] Failed : kthread_create.%ld\n",
			__func__, PTR_ERR(rproc_dev->pwronisp));
		return -EINVAL;
	}

	return 0;
}

static void hisp_pwron_task_deinit(struct rproc_boot_device *rproc_dev)
{
	if (rproc_dev == NULL) {
		pr_err("[%s] Failed : rproc_dev.%pK\n", __func__, rproc_dev);
		return;
	}

	if (rproc_dev->pwronisp != NULL) {
		kthread_stop(rproc_dev->pwronisp);
		rproc_dev->pwronisp = NULL;
	}
}

static int hisp_device_init(struct platform_device *pdev,
	struct rproc_boot_device *rproc_dev)
{
	int ret = 0;

	if ((rproc_dev == NULL) || (pdev == NULL)) {
		pr_err("[%s] Failed : rproc_dev.%pK or pdev.%pK\n",
				__func__, rproc_dev, pdev);
		return -ENOMEM;
	}

	rproc_dev->probe_finished  = 0;
	rproc_dev->isp_pdev = pdev;
	rproc_dev->rsctable_vaddr = NULL;
	rproc_dev->pwronisp_wake = 0;
	rproc_dev->pwronisp_thread_status = 0;
	mutex_init(&rproc_dev->sharedbuf_mutex);
	mutex_init(&rproc_dev->jpeg_mutex);
	mutex_init(&rproc_dev->ispcpu_mutex);
	mutex_init(&rproc_dev->rpmsg_ready_mutex);
	mutex_init(&rproc_dev->hisp_power_mutex);
	wakeup_source_init(&rproc_dev->jpeg_wakelock, "jpeg_wakelock");
	wakeup_source_init(&rproc_dev->ispcpu_wakelock, "ispcpu_wakelock");
	spin_lock_init(&rproc_dev->rpmsg_ready_spin_mutex);
	init_completion(&rproc_dev->rpmsg_boot_comp);
	init_waitqueue_head(&rproc_dev->pwronisp_wait);

	ret = hisp_loadbin_init(pdev);
	if (ret < 0) {/*lint !e64 */
		rproc_err("Failed: hisp_loadbin_init.%d\n", ret);
		goto hisp_loadbin_init_fail;
	}

	ret = hisp_pwron_task_init(rproc_dev);
	if (ret < 0) {
		rproc_err("Failed: hisp_pwron_task_init.%d\n", ret);
		goto hisp_pwron_task_init_fail;
	}

	hisp_mdc_dev_init();

	return 0;

hisp_pwron_task_init_fail:
	hisp_loadbin_deinit();

hisp_loadbin_init_fail:
	wakeup_source_trash(&rproc_dev->ispcpu_wakelock);
	wakeup_source_trash(&rproc_dev->jpeg_wakelock);
	mutex_destroy(&rproc_dev->hisp_power_mutex);
	mutex_destroy(&rproc_dev->rpmsg_ready_mutex);
	mutex_destroy(&rproc_dev->ispcpu_mutex);
	mutex_destroy(&rproc_dev->jpeg_mutex);
	mutex_destroy(&rproc_dev->sharedbuf_mutex);
	rproc_dev->isp_pdev = NULL;

	return ret;
}
static void hisp_device_deinit(void)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;

	hisp_mdc_dev_deinit();
	hisp_pwron_task_deinit(rproc_dev);
	hisp_loadbin_deinit();

	wakeup_source_trash(&rproc_dev->ispcpu_wakelock);
	wakeup_source_trash(&rproc_dev->jpeg_wakelock);
	mutex_destroy(&rproc_dev->hisp_power_mutex);
	mutex_destroy(&rproc_dev->rpmsg_ready_mutex);
	mutex_destroy(&rproc_dev->ispcpu_mutex);
	mutex_destroy(&rproc_dev->jpeg_mutex);
	mutex_destroy(&rproc_dev->sharedbuf_mutex);

	rproc_dev->pwronisp_thread_status = 0;
	rproc_dev->probe_finished  = 0;
	rproc_dev->isp_pdev = NULL;
	rproc_dev->rsctable_vaddr = NULL;
}

static int hisp_rproc_add(struct platform_device *pdev,
	struct hisp_rproc *hproc)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	int ret;

	if ((pdev == NULL) || (hproc == NULL)) {
		pr_err("[%s] pdev = %pK, hproc = %pK\n", __func__, pdev, hproc);
		return -ENOMEM;
	}

	rproc_info("rproc_add\n");

	rproc_dev->isp_rproc = hproc;

	ret = rproc_add(isp_rproc);
	if (ret != 0)
		return -ENOMEM;
#ifdef CONFIG_HISP_REMOTEPROC_DMAALLOC_DEBUG
	ret = get_vring_dma_addr_probe(pdev);
	if (ret < 0)
		goto get_vring_dma_addr_probe_fail;
#endif
	hproc->nb.notifier_call = isp_rproc_mbox_callback;

	ret = RPROC_MONITOR_REGISTER(rproc_dev->a7_ap_mbox,
		&hproc->nb);/*lint !e64 */
	if (ret != 0) {
		rproc_err("Failed : RPROC_MONITOR_REGISTER.%d\n", ret);
		goto rproc_monitor_register_fail;
	}

	mutex_lock(&rproc_dev->hisp_power_mutex);
	rproc_dev->hisp_power_state = HISP_ISP_POWER_OFF;
	mutex_unlock(&rproc_dev->hisp_power_mutex);
	rproc_dev->rpmsg_ready_state = 0;

	return 0;

rproc_monitor_register_fail:
#ifdef CONFIG_HISP_REMOTEPROC_DMAALLOC_DEBUG
	get_vring_dma_addr_remove(pdev);
#endif

get_vring_dma_addr_probe_fail:
	rproc_del(isp_rproc);

	return ret;
}
static int hisp_rproc_info_init(struct platform_device *pdev)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	struct isp_rproc_data *data = NULL;
	struct hisp_rproc *hproc = NULL;
	int ret;

	data = isp_rproc_data_getdts(&pdev->dev, rproc_dev);
	if (data == NULL) {
		dev_err(&pdev->dev, "isp_rproc_data_dtget: %pK\n", data);
		return -ESRCH;
	}

	ret = dma_set_coherent_mask(&pdev->dev,
			DMA_BIT_MASK(64));/*lint !e598 !e648 */
	if (ret != 0) {
		dev_err(&pdev->dev, "dma_set_coherent_mask: %d\n", ret);
		goto isp_rproc_data_dtget_fail;
	}

	rproc_info("rproc_alloc\n");
	isp_rproc = rproc_alloc(&pdev->dev, data->name, &isp_rproc_ops,
			data->firmware, sizeof(struct hisp_rproc));
	if (isp_rproc == NULL)
		goto isp_rproc_data_dtget_fail;

	hisp_rproc_init(isp_rproc);

	isp_rproc->ipc_addr = data->ipc_addr;
	isp_rproc->has_iommu = true;

	hproc = isp_rproc->priv;
	hproc->rproc = isp_rproc;
	platform_set_drvdata(pdev, isp_rproc);

	ret = hisp_rproc_add(pdev, hproc);
	if (ret != 0) {
		dev_err(&pdev->dev, "hisp_rproc_add: %d\n", ret);
		goto rproc_add_fail;
	}

	return 0;

rproc_add_fail:
	rproc_dev->isp_rproc = NULL;
	hproc->rproc = NULL;
	hproc = NULL;
	rproc_free(isp_rproc);
	isp_rproc = NULL;

isp_rproc_data_dtget_fail:
	isp_rproc_data_putdts(&pdev->dev);

	return -ESRCH;
}

static void hisp_rproc_info_deinit(struct platform_device *pdev)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	struct hisp_rproc *hproc = NULL;

	hproc = isp_rproc->priv;
	if (hproc == NULL) {
		dev_err(&pdev->dev, "hisp_rproc is NULL\n");
		return;
	}

	(void)RPROC_MONITOR_UNREGISTER(rproc_dev->a7_ap_mbox, &hproc->nb);
	hproc->nb.notifier_call = NULL;

#ifdef CONFIG_HISP_REMOTEPROC_DMAALLOC_DEBUG
	get_vring_dma_addr_remove(pdev);
#endif

	rproc_del(isp_rproc);
	rproc_dev->isp_rproc = NULL;
	hproc->rproc = NULL;
	hproc = NULL;

	rproc_free(isp_rproc);
	isp_rproc = NULL;

	isp_rproc_data_putdts(&pdev->dev);
}

static int hisp_rproc_probe_prev(struct platform_device *pdev)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	int ret;

	ret = hisp_device_init(pdev, rproc_dev);
	if (ret < 0)
		return ret;

	ret = hisp_rproc_regaddr_getdts(pdev, rproc_dev);
	if (ret < 0) {
		pr_err("[%s] Failed : hisp_regaddr_init.%d\n", __func__, ret);
		goto hisp_regaddr_init_fail;
	}

	ret = rdr_isp_init();
	if (ret != 0) {
		pr_err("%s: rdr_isp_init failed.%d", __func__, ret);
		goto rdr_isp_init_fail;
	}

	ret = hisp_pwr_probe(pdev);
	if (ret < 0) {
		rproc_err("Failed : hisp_pwr_probe.%d\n", ret);
		goto dtsi_read_fail;
	}

	ret = isp_atfisp_probe(pdev);
	if (ret < 0) {
		rproc_err("Failed : isp_atfisp_probe.%d\n", ret);
		goto hisp_pwr_probe_fail;
	}

	ret = hisp_nsec_probe(pdev);
	if (ret < 0) {
		rproc_err("Failed : hisp_nsec_probe.%d\n", ret);
		goto isp_atfisp_probe_fail;
	}

	return 0;

isp_atfisp_probe_fail:
	(void)isp_atfisp_remove(pdev);

hisp_pwr_probe_fail:
	hisp_pwr_remove(pdev);

dtsi_read_fail:
	rdr_isp_exit();

rdr_isp_init_fail:
	hisp_rproc_regaddr_putdts(rproc_dev);

hisp_regaddr_init_fail:
	hisp_device_deinit();

	return -ENODEV;
}

static void hisp_rproc_probe_clr(struct platform_device *pdev)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;

	(void)isp_atfisp_remove(pdev);

	hisp_pwr_remove(pdev);

	rdr_isp_exit();

	hisp_rproc_regaddr_putdts(rproc_dev);

	hisp_device_deinit();
}

static int hisp_rproc_probe(struct platform_device *pdev)
{
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	struct device *dev = &pdev->dev;
	int ret;

	rproc_info("+\n");

	ret = hisp_rproc_probe_prev(pdev);
	if (ret < 0) {
		rproc_err("Failed : hisp_rproc_probe_prev.%d\n", ret);
		return ret;
	}

	atomic_set(&rproc_dev->rproc_enable_status, 0);

	ret = hisp_rproc_info_init(pdev);
	if (ret != 0) {
		rproc_err("Failed : hisp_rproc_info_init.%d\n", ret);
		goto hisp_nsec_probe_fail;
	}

	ret = isp_ipc_resource_init();
	if (ret != 0) {
		rproc_err("Failed : init_isp_ipc_resource.%d\n", ret);
		goto hisp_rproc_info_init_fail;
	}

	rproc_set_sync_flag(true);

	ret = hisp_rpmsg_rdr_init();
	if (ret < 0) {
		dev_err(dev, "Fail :hisp_rpmsg_rdr_init: %d\n", ret);
		goto isp_ipc_resource_init_fail;
	}

	ret = hisp_dsm_register();
	if (ret < 0) {
		dev_err(dev, "Fail :hisp_dsm_register: %d\n", ret);
		goto hisp_rpmsg_rdr_init_fail;
	}

	rproc_dev->probe_finished = 1;
	rproc_info("-\n");
	return 0;

hisp_rpmsg_rdr_init_fail:
	(void)hisp_rpmsg_rdr_deinit();

isp_ipc_resource_init_fail:
	isp_ipc_resource_exit();

hisp_rproc_info_init_fail:
	hisp_rproc_info_deinit(pdev);

hisp_nsec_probe_fail:
	(void)hisp_nsec_remove(pdev);

	hisp_rproc_probe_clr(pdev);
	return ret;
}

static int hisp_rproc_remove(struct platform_device *pdev)
{
	struct rproc *rproc = platform_get_drvdata(pdev);
	struct rproc_boot_device *rproc_dev = &rproc_boot_dev;
	struct device *dev = &pdev->dev;
	struct hisp_rproc *hproc = NULL;
	int ret;

	rproc_info("+\n");
	if (rproc == NULL) {
		rproc_err("Failed : rproc.%pK\n", rproc);
		return -ENOMEM;
	}

	hisp_dsm_unregister();
	(void)hisp_rpmsg_rdr_deinit();

	hproc = rproc->priv;
	rproc_dev->rpmsg_ready_state = 0;
	if (hproc != NULL) {
		ret = RPROC_MONITOR_UNREGISTER(rproc_dev->a7_ap_mbox,
			&hproc->nb);/*lint !e64 */
		if (ret != 0)
			rproc_err("Failed :RPROC_MONITOR_UNREGISTER.%d\n", ret);
	}

	RPROC_FLUSH_TX(rproc_dev->ap_a7_mbox);/*lint !e64 */
	if (hproc != NULL)
		hproc->nb.notifier_call = NULL;
	isp_ipc_resource_exit();

#ifdef CONFIG_HISP_REMOTEPROC_DMAALLOC_DEBUG
	(void)get_vring_dma_addr_remove(pdev);
#endif
	rproc_del(rproc);
	rproc_dev->isp_rproc = NULL;
	if (hproc != NULL) {
		hproc->rproc = NULL;
		hproc = NULL;
	}
	rproc_free(rproc);
	rproc = NULL;

	isp_rproc_data_putdts(dev);
	(void)hisp_pwr_remove(pdev);
	(void)hisp_nsec_remove(pdev);
	(void)isp_atfisp_remove(pdev);

	rdr_isp_exit();
	hisp_rproc_regaddr_putdts(rproc_dev);

	hisp_device_deinit();

	rproc_info("-\n");
	return 0;
}

static const struct of_device_id hisp_rproc_of_match[] = {
	{ .compatible = DTS_COMP_NAME},
	{ },
};
MODULE_DEVICE_TABLE(of, hisp_rproc_of_match);

static struct platform_driver hisp_rproc_driver = {
	.driver = {
		.owner      = THIS_MODULE,
		.name       = "isp",
		.of_match_table = of_match_ptr(hisp_rproc_of_match),
	},
	.probe  = hisp_rproc_probe,
	.remove = hisp_rproc_remove,
};
module_platform_driver(hisp_rproc_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HiStar V100 Remote Processor control driver");

#ifdef DEBUG_HISP
int set_power_updn(int bypass)
{
	struct rproc_boot_device *dev = &rproc_boot_dev;

	if (bypass != 0 && bypass != 1) {/*lint !e774 */
		rproc_err("Failed : bypass.%x\n", bypass);
		return -EINVAL;
	}

	dev->bypass_pwr_updn = bypass;

	return 0;
}
#endif

