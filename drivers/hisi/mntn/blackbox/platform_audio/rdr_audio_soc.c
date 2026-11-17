/*
 * rdr_audio_soc.c
 *
 * audio soc driver
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "rdr_audio_soc.h"

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/of_irq.h>
#include <asm/tlbflush.h>
#include "rdr_print.h"
#include "rdr_inner.h"
#include "rdr_field.h"
#include "rdr_audio_adapter.h"
#include "rdr_audio_dump_socdsp.h"
#include <linux/hisi/audio_log.h>
#include "dsp_misc.h"
#include "dsp_om.h"

#include "usbaudio_ioctl.h"
#include <linux/hisi/usb/hifi_usb.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif
#ifdef SECOS_RELOAD_HIFI
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/dma-contiguous.h>
#include <linux/remoteproc.h>
#include <linux/of_reserved_mem.h>
#include <linux/hisi/hisi_load_image.h>
#include <linux/hisi/partition_macro.h>
#include <linux/hisi/partition_ap_kernel.h>
#include <linux/hisi/hisi_drmdriver.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/hisi/hisi_mm.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
#include "teek_client_id.h"
#endif

#define BSP_RESET_OK 0
#define BSP_RESET_ERROR (-1)

#define RDR_COMMENT_LEN 128UL
#define PARTION_INFO_LEN 10
#define DSP_BSS_SEC 2
#define SOC_WDT_TIMEOUT_IRQ_NUM 245U

#define WATCHDOG_DTS_COMP_NAME "hisilicon,sochifi-watchdog"

#define DRV_WATCHDOG_CONTROL (DRV_WATCHDOG_BASE_ADDR + 0x008)
#define DRV_WATCHDOG_INTCLR (DRV_WATCHDOG_BASE_ADDR + 0x00C)
#define DRV_WATCHDOG_LOCK (DRV_WATCHDOG_BASE_ADDR + 0xC00)
#define DRV_WATCHDOG_UNLOCK_NUM 0x1ACCE551
#define DRV_WATCHDOG_LOCK_NUM 0x0
#define DRV_WATCHDOG_CONTROL_DISABLE 0x0
#define DRV_WATCHDOG_INTCLR_NUM 0x4455 /* Generally speaking, any number is okay */

#define SOCDSP_DIE_NOTIFY_LPM3 ((0 << 24) | (16 << 16) | (3 << 8) | (1 << 0)) /* bit 24-31 OBJ_AP
                                                                               * bit 16-23 OBJ_PSCI
                                                                               * bit 8-15  CMD_SETTING
                                                                               * bit 0-7   TYPE_POWER
                                                                               */
#define CFG_MMBUF_REMAP_EN 0x130 /* mmbuf remap enable 9bit */
#define CFG_OCRAM_REMAP_EN 0x13C /* ocram remap enable 9bit */
#define ASP_CFG_BASE SOC_ACPU_ASP_CFG_BASE_ADDR
#define FLAG_ROW_LEN 64
#define FLAG_COMMENT_LEN 128
#define PARSE_FLAG_LOG_SIZE (FLAG_ROW_LEN * ARRAY_SIZE(g_socdsp_flag) + FLAG_COMMENT_LEN)
#define ICC_DEBUG_ONE_INFO_LEN 76
#define PARSE_ICC_DEBUG_LOG_SIZE (ICC_DEBUG_ONE_INFO_LEN * ICC_STAT_MSG_NUM * 2 + 330)

struct rdr_soc_des_s {
	uint32_t modid;
	uint32_t wdt_irq_num;
	char *pathname;
	uint32_t *control_addr;
	uint32_t *lock_addr;
	uint32_t *intclr_addr;
	pfn_cb_dump_done dumpdone_cb;

	struct semaphore dump_sem;
	struct semaphore handler_sem;
	struct wakeup_source rdr_wl;
	struct task_struct *kdump_task;
	struct task_struct *khandler_task;
};

struct sreset_mgr_lli *g_pmgr_dspreset_data;
static struct rdr_soc_des_s g_soc_des;

#ifdef SECOS_RELOAD_HIFI
#define DTS_COMP_HIFICMA_NAME "hisilicon,hifi-cma"
#ifdef CONFIG_HIFI_MEMORY_21M
#define DSP_CMA_IMAGE_SIZE 0xA00000UL
#else
#define DSP_CMA_IMAGE_SIZE 0x600000UL
#endif
struct audio_dsp_cma_struct {
	struct device *device;
};

struct audio_dsp_cma_struct g_dspcma_dev;
#endif
/*lint -e446*/

static void audio_rdr_remap_init(void)
{
	void __iomem *rdr_aspcfg_base = NULL;
	unsigned int read_val;

	rdr_aspcfg_base = ioremap(ASP_CFG_BASE, (unsigned long)SZ_4K);
	if (!rdr_aspcfg_base) {
		AUDIO_LOGE("rdr aspcfg base ioremap error");
		return;
	}

	read_val = (unsigned int)readl(rdr_aspcfg_base + CFG_MMBUF_REMAP_EN);
	read_val &= BIT(DRV_MODULE_NAME_LEN);
	if (read_val != 0)
		writel(0x0, (rdr_aspcfg_base + CFG_MMBUF_REMAP_EN));

	read_val = (unsigned int)readl(rdr_aspcfg_base + CFG_OCRAM_REMAP_EN);
	read_val &= BIT(DRV_MODULE_NAME_LEN);
	if (read_val != 0)
		writel(0x0, (rdr_aspcfg_base + CFG_OCRAM_REMAP_EN));

	iounmap(rdr_aspcfg_base);
}

static int reset_socdsp_sec(void)
{
	struct drv_dsp_sec_ddr_head *head = NULL;
	char *sec_head = NULL;
	char *sec_addr = NULL;
	unsigned int i;
	int ret = 0;

	sec_head = (char *)ioremap_wc(DSP_SEC_HEAD_BACKUP, (unsigned long)DSP_SEC_HEAD_SIZE);
	if (!sec_head)
		return -ENOMEM;

	head = (struct drv_dsp_sec_ddr_head *)sec_head;

	AUDIO_LOGI("sections_num: 0x%x", head->sections_num);

	for (i = 0; i < head->sections_num; i++) {
		if (head->sections[i].type == DSP_BSS_SEC) {
			AUDIO_LOGI("sec_id: %u, type: 0x%x, src_addr: 0x%pK, des_addr: 0x%pK, size: %u",
				i, head->sections[i].type, (void *)(uintptr_t)(head->sections[i].src_addr),
				(void *)(uintptr_t)(head->sections[i].des_addr),
				head->sections[i].size);
			sec_addr = (char *)ioremap_wc((phys_addr_t)head->sections[i].des_addr,
				(unsigned long)head->sections[i].size);
			if (!sec_addr) {
				ret = -ENOMEM;
				goto end;
			}

			memset(sec_addr, 0x0, (unsigned long)head->sections[i].size);
			iounmap(sec_addr);
			sec_addr = NULL;
		}
	}

end:
	iounmap(sec_head);
	return ret;
}

static int irq_handler_thread(void *arg)
{
	AUDIO_LOGI("enter blackbox");

	while (!kthread_should_stop()) {
		if (down_interruptible(&g_soc_des.handler_sem)) {
			AUDIO_LOGE("down sem fail");
			continue;
		}

		AUDIO_LOGI("socdsp timestamp: %u socdsp watchdog coming", DSP_STAMP);
		usbaudio_ctrl_hifi_reset_inform();
		hifi_usb_hifi_reset_inform();
		dsp_reset_release_syscache();
		if (is_dsp_power_on()) {
			audio_rdr_nmi_notify_dsp();
			audio_rdr_remap_init();
		} else {
			AUDIO_LOGE("dsp is power off, do not send nmi & remap");
		}
		hifireset_runcbfun(DRV_RESET_CALLCBFUN_RESET_BEFORE);
		AUDIO_LOGI("enter rdr process for socdsp watchdog");
		rdr_system_error((unsigned int)RDR_AUDIO_SOC_WD_TIMEOUT_MODID, 0, 0);
		AUDIO_LOGI("exit rdr process for socdsp watchdog");
	}

	AUDIO_LOGI("exit  blackbox");

	return 0;
}

static int audio_rdr_ipc_notify_lpm3(unsigned int *msg, int len)
{
	int ret;
	int i;

	for (i = 0; i < len; i++)
		AUDIO_LOGI("rdr: [ap2lpm3 notifiy] msg[%d]: 0x%x", i, msg[i]);

	ret = RPROC_ASYNC_SEND(HISI_ACPU_LPM3_MBX_5, msg, len);
	if (ret != 0)
		AUDIO_LOGE("send mesg to lpm3 fail");

	return ret;
}

#ifdef SECOS_RELOAD_HIFI
static bool is_config_cma_secure_by_atf(void)
{
	struct audio_dsp_cma_struct *dev = &g_dspcma_dev;
	const char *cma_sec_config = NULL;

	if (!of_property_read_string(dev->device->of_node, "cma-sec-config", &cma_sec_config)) {
		if (!strncmp(cma_sec_config, "atf", strlen("atf"))) {
			AUDIO_LOGI("rdr: config cma secure by atf");
			return true;
		}
	} else {
		AUDIO_LOGE("read cma-sec-config err");
	}

	AUDIO_LOGI("rdr: config cma secure not by atf");

	return false;
}

static int access_smc_register(phys_addr_t *phys, struct audio_dsp_cma_struct *dev)
{
	if (atfd_hisi_service_access_register_smc(ACCESS_REGISTER_FN_MAIN_ID,
		*phys, DSP_CMA_IMAGE_SIZE, ACCESS_REGISTER_FN_SUB_ID_DDR_HIFI_SEC_OPEN) == -1) {
		change_secpage_range(*phys, (uintptr_t)phys_to_virt(*phys),
			DSP_CMA_IMAGE_SIZE, __pgprot(PROT_NORMAL));

		flush_tlb_all();
		(void)dma_release_from_contiguous(dev->device,
			phys_to_page(*phys), (int)DSP_CMA_IMAGE_SIZE >> PAGE_SHIFT);
			return -EACCES;
	}

	return 0;
}

static int rdr_dsp_cma_alloc(phys_addr_t *hifi_addr)
{
	struct audio_dsp_cma_struct *dev = &g_dspcma_dev;
	phys_addr_t phys;
	struct page *page = NULL;
	unsigned int val = 0;
	int ret = 0;

	if (of_property_read_u32(dev->device->of_node, "hisi-align", &val)) {
		AUDIO_LOGE("read align val err");
		return -EINVAL;
	}

	page = dma_alloc_from_contiguous(dev->device,
		DSP_CMA_IMAGE_SIZE >> PAGE_SHIFT, val, GFP_KERNEL);
	if (!page) {
		AUDIO_LOGE("dma_alloc_from_contiguous (hifi) alloc err");
		return -ENOMEM;
	}
	phys = page_to_phys(page);
	if (phys & (phys_addr_t)(BIT(val) - 1)) {
		AUDIO_LOGE("align error, align val %u", val);
		return -EINVAL;
	}

	change_secpage_range(phys, (uintptr_t)phys_to_virt(phys),
		DSP_CMA_IMAGE_SIZE, __pgprot(PROT_DEVICE_nGnRE));

	flush_tlb_all();
	__dma_unmap_area(phys_to_virt(phys), DSP_CMA_IMAGE_SIZE, DMA_BIDIRECTIONAL);

	if (is_config_cma_secure_by_atf()) {
		ret = access_smc_register(&phys, dev);
		if (ret != 0) {
			AUDIO_LOGE("atfd hisi service access register smc dsp set err");
			return -EACCES;
		}
	}

	*hifi_addr = phys;
	AUDIO_LOGI("rdr: addr 0x%pK", (void *)(uintptr_t)phys);
	return ret;
}

static int rdr_dsp_cma_free(phys_addr_t addr)
{
	struct audio_dsp_cma_struct *dev = (struct audio_dsp_cma_struct *)&g_dspcma_dev;

	if (addr == 0) {
		AUDIO_LOGE("addr is null");
		return -EINVAL;
	}

	if (is_config_cma_secure_by_atf()) {
		if (atfd_hisi_service_access_register_smc(ACCESS_REGISTER_FN_MAIN_ID,
			addr, DSP_CMA_IMAGE_SIZE, ACCESS_REGISTER_FN_SUB_ID_DDR_HIFI_SEC_CLOSE) == -1) {
			AUDIO_LOGE("atfd hisi service access register smc dsp clear err");
			return -EACCES;
		}
	}

	change_secpage_range(addr, (uintptr_t)phys_to_virt(addr),
		DSP_CMA_IMAGE_SIZE, __pgprot(PROT_NORMAL));

	flush_tlb_all();
	(void)dma_release_from_contiguous(dev->device, phys_to_page(addr),
		(int)DSP_CMA_IMAGE_SIZE >> PAGE_SHIFT);

	return 0;
}

static int get_dsp_image_size(unsigned int *size)
{
	int fd = -1;
	int cnt;
	int ret = 0;
	char path[RDR_FNAME_LEN + 1] = {0};
	struct drv_socdsp_image_head socdsp_head = {{0}};
	mm_segment_t old_fs;

	if (flash_find_ptn_s(PART_FW_HIFI, path, sizeof(path)) < 0) {
		AUDIO_LOGE("partion_name hifi is not in partion table");
		return -EINVAL;
	}

	old_fs = get_fs(); /*lint !e501*/
	set_fs(KERNEL_DS); /*lint !e501*/

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
	fd = (int)ksys_open(path, O_RDWR, 0);
#else
	fd = (int)sys_open(path, O_RDWR, 0);
#endif
	if (fd < 0) {
		AUDIO_LOGE("rdr: open %s failed, return %d", path, fd);
		set_fs(old_fs);
		return -ENOENT;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
	cnt = (int)ksys_read((unsigned int)fd, (char *)&socdsp_head, sizeof(socdsp_head));
#else
	cnt = (int)sys_read((unsigned int)fd, (char *)&socdsp_head, sizeof(socdsp_head));
#endif
	if (cnt != (int)sizeof(socdsp_head)) {
		AUDIO_LOGE("rdr: read %s failed, return %d", path, cnt);
		ret = -EINVAL;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
	ksys_close((unsigned int)fd);
#else
	sys_close((unsigned int)fd);
#endif
	set_fs(old_fs);
	*size = socdsp_head.image_size;
	AUDIO_LOGI("rdr: %u", socdsp_head.image_size);

	return ret;
}

static bool is_reload_socdsp_by_secos(void)
{
	struct audio_dsp_cma_struct *dev = &g_dspcma_dev;
	const char *enable_status = NULL;

	if (!of_property_read_string(dev->device->of_node, "enable-status", &enable_status)) {
		if (!strncmp(enable_status, "true", strlen("true")))
			return true;
	} else {
		AUDIO_LOGE("read enable-status err");
	}

	return false;
}

static int reload_socdsp_by_secos(void)
{
	struct load_image_info loadinfo = {0};
	phys_addr_t dsp_addr = 0;
	char partion_name[PARTION_INFO_LEN] = PART_FW_HIFI;
	int ret;
	int free_ret;

	ret = rdr_dsp_cma_alloc(&dsp_addr);
	if (ret != 0) {
		AUDIO_LOGE("rdr: alloc cma buf err: %d", ret);
		return ret;
	}

	loadinfo.ecoretype = HIFI;
	loadinfo.image_addr = (unsigned long)dsp_addr;
	loadinfo.partion_name = partion_name;
	ret = get_dsp_image_size(&loadinfo.image_size);
	if (ret != 0) {
		AUDIO_LOGE("rdr: get dsp image size fail: %u, ret: %d",
			loadinfo.image_size, ret);
		goto end;
	}

	if (loadinfo.image_size > DSP_CMA_IMAGE_SIZE) {
		ret = -EINVAL;
		AUDIO_LOGE("rdr: image size: %u, alloc cma buf size: %u",
			loadinfo.image_size, DSP_CMA_IMAGE_SIZE);
		goto end;
	}

	ret = bsp_load_and_verify_image(&loadinfo);
	if (ret < 0) {
		AUDIO_LOGE("rdr: bsp load and verify image pre fail: %d", ret);
		goto end;
	}

	AUDIO_LOGI("rdr: bsp load and verify image success");

end:
	free_ret = rdr_dsp_cma_free(dsp_addr);
	if (free_ret != 0) {
		AUDIO_LOGE("rdr: free cma buffer error: %d", free_ret);
		ret = free_ret;
	}

	return ret;
}
#endif

static int reset_socdsp(void)
{
	unsigned int *power_status_addr = NULL;
	unsigned int msg;
	int ret;

	AUDIO_LOGI("enter blackbox");

#ifdef SECOS_RELOAD_HIFI
	if (is_reload_socdsp_by_secos()) {
		AUDIO_LOGI("rdr: reload socdsp by secos");
		ret = reload_socdsp_by_secos();
		if (ret) {
			AUDIO_LOGE("secos reload socdsp fail, ret: %d, reboot now", ret);
			BUG_ON(1);
		}
	}
#endif

	power_status_addr = ioremap_wc(DRV_DSP_POWER_STATUS_ADDR, 0x4);
	if (!power_status_addr) {
		AUDIO_LOGE("DRV_DSP_POWER_STATUS_ADDR ioremap failed");
	} else {
		writel(DRV_DSP_POWER_OFF, power_status_addr);
		iounmap(power_status_addr);
	}

	msg = SOCDSP_DIE_NOTIFY_LPM3;
	ret = audio_rdr_ipc_notify_lpm3(&msg, 1);
	AUDIO_LOGI("rdr: power off hifi %s", ret ? "fail" : "success");

	AUDIO_LOGI("exit  blackbox");

	return ret;
}

struct sreset_mgr_lli *reset_link_insert(struct sreset_mgr_lli *plink, struct sreset_mgr_lli *punit)
{
	struct sreset_mgr_lli *phead = plink;
	struct sreset_mgr_lli *ppose = plink;
	struct sreset_mgr_lli *ptail = plink;

	if (!plink || !punit) {
		AUDIO_LOGE("input params are not legitimate");
		return NULL;
	}

	while (ppose) {
		/* insert into linked list according to priority */
		if (ppose->cbfuninfo.priolevel > punit->cbfuninfo.priolevel) {
			if (phead == ppose) {
				punit->pnext = ppose;
				phead = punit;
			} else {
				ptail->pnext = punit;
				punit->pnext = ppose;
			}
			break;
		}
		ptail = ppose;
		ppose = ppose->pnext;
	}

	if (!ppose)
		ptail->pnext = punit;

	return phead;
}

struct sreset_mgr_lli *reset_do_regcbfunc(struct sreset_mgr_lli *plink,
	const char *pname, hifi_reset_cbfunc pcbfun, int userdata, int priolevel)
{
	struct sreset_mgr_lli *phead = plink;
	struct sreset_mgr_lli *pmgr_unit = NULL;

	if ((!pname) || (!pcbfun) || (priolevel < RESET_CBFUNC_PRIO_LEVEL_LOWT
		|| priolevel > RESET_CBFUNC_PRIO_LEVEL_HIGH)) {
		AUDIO_LOGE("fail in ccore reset regcb, fail, name 0x%s, cbfun 0x%pK, prio %d",
			pname, pcbfun, priolevel);
		return NULL;
	}

	pmgr_unit = kmalloc(sizeof(*pmgr_unit), GFP_KERNEL);
	if (pmgr_unit) {
		memset((void *)pmgr_unit, 0, (sizeof(*pmgr_unit)));
		strncpy(pmgr_unit->cbfuninfo.name, pname, (unsigned long)DRV_MODULE_NAME_LEN);
		pmgr_unit->cbfuninfo.priolevel = priolevel;
		pmgr_unit->cbfuninfo.userdata = userdata;
		pmgr_unit->cbfuninfo.cbfun = pcbfun;
	} else {
		AUDIO_LOGE("pmgr unit malloc fail");
		return NULL;
	}

	if (!phead)
		phead = pmgr_unit;
	else
		/* insert linked list according to priority */
		phead = reset_link_insert(phead, pmgr_unit);

	return phead;
}

#ifdef CONFIG_HISI_HIFI_BB

int hifireset_regcbfunc(const char *pname, hifi_reset_cbfunc pcbfun, int userdata, int priolevel)
{
	g_pmgr_dspreset_data = reset_do_regcbfunc(g_pmgr_dspreset_data, pname,
		pcbfun, userdata, priolevel);
	AUDIO_LOGI("%s registered a cbfun for dsp reset", pname);

	return 0;
}
#endif /* end of config_audio_dsp_bb */

void run_cbfunction(struct sreset_mgr_lli *phead, enum DRV_RESET_CALLCBFUN_MOMENT eparam, int *iresult)
{
	while (phead) {
		if (phead->cbfuninfo.cbfun) {
			*iresult = phead->cbfuninfo.cbfun(eparam, phead->cbfuninfo.userdata);

			if (*iresult != BSP_RESET_OK) {
				/* if the return fails, record the component name and return value */
				AUDIO_LOGE("fail to run cbfun of %s, at %d return %d",
					phead->cbfuninfo.name, eparam, *iresult);
				break;
			}
			AUDIO_LOGI("run %s cb function 0x%pK", phead->cbfuninfo.name, phead->cbfuninfo.cbfun);
		}
		phead = phead->pnext;
	}
}

int hifireset_doruncbfun(const char *pname, enum DRV_RESET_CALLCBFUN_MOMENT eparam)
{
	int iresult = BSP_RESET_OK;
	struct sreset_mgr_lli *phead = g_pmgr_dspreset_data;

	if (!pname) {
		AUDIO_LOGE("pname is null");
		return -EINVAL;
	}

	/* do not judge the module name, execute in order */
	if (strncmp(pname, RESET_CBFUN_IGNORE_NAME, strlen(RESET_CBFUN_IGNORE_NAME)) == 0) {
		run_cbfunction(phead, eparam, &iresult);
	} else {
		/* need to determine the module name and execute the specified callback function */
		while (phead) {
			if (strncmp(pname, phead->cbfuninfo.name, strlen(phead->cbfuninfo.name)) == 0
				&& phead->cbfuninfo.cbfun) {
				iresult  = phead->cbfuninfo.cbfun(eparam, phead->cbfuninfo.userdata);
				AUDIO_LOGI("run %s cb function 0x%pK", phead->cbfuninfo.name, phead->cbfuninfo.cbfun);
				break;
			}
			phead = phead->pnext;
		}
	}

	if (iresult != BSP_RESET_OK) {
		if (phead)
			AUDIO_LOGE("fail to run cbfun of %s, at %d, return %d",
				phead->cbfuninfo.name, eparam, iresult);
		else
			AUDIO_LOGE("fail to run cbfun, but phead or pname is null");
	}

	return iresult;
}

int hifireset_runcbfun(enum DRV_RESET_CALLCBFUN_MOMENT eparam)
{
	int  iresult;

	if (eparam == DRV_RESET_CALLCBFUN_RESET_BEFORE) {
		/* traverse the linked list of callback functions and call NAS callbacks */
		iresult = hifireset_doruncbfun("NAS_AT", eparam);
		if (iresult != BSP_RESET_OK) {
			/* if the return fails, record the formation name, return value, save to file */
			return BSP_RESET_ERROR;
		}
	} else {
		/* traverse the linked list of callback functions and call the callback function */
		iresult = hifireset_doruncbfun(RESET_CBFUN_IGNORE_NAME, eparam);
		if (iresult != BSP_RESET_OK)
			return BSP_RESET_ERROR;
	}
	AUDIO_LOGI("end of run cb functions for hifi reset at %d, %d", eparam, iresult);

	return BSP_RESET_OK;
}

void rdr_audio_soc_reset(unsigned int modid, unsigned int etype, u64 coreid)
{
	int ret;

	AUDIO_LOGI("enter blackbox");

	ret = reset_socdsp_sec();
	AUDIO_LOGI("rdr: reset socdsp sec, %s", ret ? "fail" : "success");

	ret = reset_socdsp();
	if (ret != 0) {
		__pm_relax(&g_soc_des.rdr_wl);
		AUDIO_LOGE("rdr: reset socdsp error");
		return;
	}

#ifdef CONFIG_HIFI_DSP_ONE_TRACK
	dsp_watchdog_send_event();
#endif

	hifireset_runcbfun(DRV_RESET_CALLCBFUN_RESET_AFTER);

	__pm_relax(&g_soc_des.rdr_wl);

	AUDIO_LOGI("exit  blackbox");
}

static irqreturn_t soc_wtd_irq_handler(int irq, void *data)
{
	AUDIO_LOGI("socdsp watchdog irq arrival");

	writel(DRV_WATCHDOG_UNLOCK_NUM, g_soc_des.lock_addr);
	writel(DRV_WATCHDOG_INTCLR_NUM, g_soc_des.intclr_addr);
	writel(DRV_WATCHDOG_CONTROL_DISABLE, g_soc_des.control_addr);
	writel(DRV_WATCHDOG_LOCK_NUM, g_soc_des.lock_addr);

	__pm_stay_awake(&g_soc_des.rdr_wl);

	up(&g_soc_des.handler_sem);

	return IRQ_HANDLED;
}

static unsigned int rdr_get_socdsp_watchdog_irq_num(void)
{
	unsigned int irq_num;
	struct device_node *dev_node = NULL;

	dev_node = of_find_compatible_node(NULL, NULL, WATCHDOG_DTS_COMP_NAME);
	if (!dev_node) {
		AUDIO_LOGE("rdr: find device node socdsp watchdog by compatible failed");
		return 0;
	}

	irq_num = irq_of_parse_and_map(dev_node, 0);
	if (irq_num == 0) {
		AUDIO_LOGE("rdr: irq parse and map failed, irq: %u", irq_num);
		return 0;
	}

	return irq_num;
}

static int remap_watchdog_addr(void)
{
	g_soc_des.lock_addr = (unsigned int *)ioremap((unsigned long)DRV_WATCHDOG_LOCK, 0x4);
	if (!g_soc_des.lock_addr) {
		AUDIO_LOGE("rdr: remap watchdog lock addr fail");
		return -ENOMEM;
	}

	g_soc_des.control_addr = (unsigned int *)ioremap((unsigned long)DRV_WATCHDOG_CONTROL, 0x4);
	if (!g_soc_des.control_addr) {
		AUDIO_LOGE("rdr: remap watchdog control addr fail");
		goto remap_control_addr_err;
	}

	g_soc_des.intclr_addr = (unsigned int *)ioremap((unsigned long)DRV_WATCHDOG_INTCLR, 0x4);
	if (!g_soc_des.intclr_addr) {
		AUDIO_LOGE("rdr: remap watchdog interrupt clear addr fail");
		goto remap_intclr_addr_err;
	}

	return 0;

remap_intclr_addr_err:
	if (g_soc_des.control_addr) {
		iounmap(g_soc_des.control_addr);
		g_soc_des.control_addr = NULL;
	}
remap_control_addr_err:
	if (g_soc_des.lock_addr) {
		iounmap(g_soc_des.lock_addr);
		g_soc_des.lock_addr = NULL;
	}

	return -ENOMEM;
}

static void unremap_watchdog_addr(void)
{
	if (g_soc_des.lock_addr) {
		iounmap(g_soc_des.lock_addr);
		g_soc_des.lock_addr = NULL;
	}

	if (g_soc_des.control_addr) {
		iounmap(g_soc_des.control_addr);
		g_soc_des.control_addr = NULL;
	}

	if (g_soc_des.intclr_addr) {
		iounmap(g_soc_des.intclr_addr);
		g_soc_des.intclr_addr = NULL;
	}
}

static int create_rdr_soc_thread(void)
{
	g_soc_des.kdump_task = kthread_run(soc_dump_thread, NULL, "rdr_audio_soc_thread");
	if (!g_soc_des.kdump_task) {
		AUDIO_LOGE("create rdr soc dump thead fail");
		return -EBUSY;
	}

	g_soc_des.khandler_task = kthread_run(irq_handler_thread, NULL, "rdr_audio_soc_wtd_irq_handler_thread");
	if (!g_soc_des.khandler_task) {
		AUDIO_LOGE("create rdr soc wtd irq handler thead fail");
		goto create_khandler_task_fail;
	}

	return 0;

create_khandler_task_fail:
	if (g_soc_des.kdump_task) {
		kthread_stop(g_soc_des.kdump_task);
		up(&g_soc_des.dump_sem);
		g_soc_des.kdump_task = NULL;
	}

	return -EBUSY;
}

static void destroy_rdr_soc_thread(void)
{
	if (g_soc_des.kdump_task) {
		kthread_stop(g_soc_des.kdump_task);
		up(&g_soc_des.dump_sem);
		g_soc_des.kdump_task = NULL;
	}

	if (g_soc_des.khandler_task) {
		kthread_stop(g_soc_des.khandler_task);
		up(&g_soc_des.handler_sem);
		g_soc_des.khandler_task = NULL;
	}
}

static void priv_data_init(void)
{
	g_soc_des.wdt_irq_num = 0;
	g_soc_des.lock_addr = NULL;
	g_soc_des.control_addr = NULL;
	g_soc_des.intclr_addr = NULL;

	sema_init(&g_soc_des.dump_sem, 0);
	sema_init(&g_soc_des.handler_sem, 0);
	g_soc_des.kdump_task = NULL;
	g_soc_des.khandler_task = NULL;
}

int rdr_audio_soc_init(void)
{
	int ret;

	AUDIO_LOGI("enter blackbox");

	priv_data_init();

	ret = remap_watchdog_addr();
	if (ret != 0)
		return ret;

	wakeup_source_init(&g_soc_des.rdr_wl, "rdr_sochifi");

	ret = create_rdr_soc_thread();
	if (ret != 0)
		goto create_soc_thread_fail;

	g_soc_des.wdt_irq_num = rdr_get_socdsp_watchdog_irq_num();
	if (g_soc_des.wdt_irq_num == 0) {
		AUDIO_LOGE("rdr: get socdsp watchdog irq num fail");
		goto get_irq_num_fail;
	}

	ret = request_irq(g_soc_des.wdt_irq_num, soc_wtd_irq_handler, 0, "soc wdt handler", NULL);
	if (ret != 0) {
		AUDIO_LOGE("request irq soc wdt irq handler failed return 0x%x", ret);
		goto request_irp_fail;
	}


	AUDIO_LOGI("exit  blackbox");

	return ret;


request_irp_fail:
get_irq_num_fail:
	destroy_rdr_soc_thread();
create_soc_thread_fail:

	wakeup_source_trash(&g_soc_des.rdr_wl);

	unremap_watchdog_addr();

	AUDIO_LOGI("exit  blackbox");

	return ret;
}

void rdr_audio_soc_exit(void)
{
	AUDIO_LOGI("enter blackbox");


	if (g_soc_des.wdt_irq_num > 0)
		free_irq(g_soc_des.wdt_irq_num, NULL);

	destroy_rdr_soc_thread();

	wakeup_source_trash(&g_soc_des.rdr_wl);

	unremap_watchdog_addr();

	AUDIO_LOGI("exit  blackbox");
}
/*lint +e446*/

#ifdef SECOS_RELOAD_HIFI
static int audio_socdsp_cma_probe(struct platform_device *pdev)
{
	struct audio_dsp_cma_struct *dev = (struct audio_dsp_cma_struct *)&g_dspcma_dev;
	int ret;

	memset(dev, 0, sizeof(*dev));
	dev->device = &(pdev->dev);

	ret = of_reserved_mem_device_init(dev->device);
	if (ret != 0) {
		dev->device = NULL;
		AUDIO_LOGE("socdsp cma device init failed return 0x%x", ret);
	}

	return ret;
}

static int audio_socdsp_cma_remove(struct platform_device *pdev)
{
	struct audio_dsp_cma_struct *dev = (struct audio_dsp_cma_struct *)&g_dspcma_dev;

	of_reserved_mem_device_release(dev->device);
	memset(dev, 0, sizeof(*dev));

	return 0;
}

static const struct of_device_id audio_socdsp_cma_of_match[] = {
	{
		.compatible = DTS_COMP_HIFICMA_NAME,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, audio_socdsp_cma_of_match);

static struct platform_driver audio_socdsp_cma_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "hifi-cma",
		.of_match_table = of_match_ptr(audio_socdsp_cma_of_match),
	},
	.probe = audio_socdsp_cma_probe,
	.remove = audio_socdsp_cma_remove,
};

static int __init audio_socdsp_cma_init(void)
{
	return platform_driver_register(&audio_socdsp_cma_driver);
}
subsys_initcall(audio_socdsp_cma_init);

static void __exit audio_socdsp_cma_exit(void)
{
	platform_driver_unregister(&audio_socdsp_cma_driver);
}
module_exit(audio_socdsp_cma_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("hifi reset cma module");
#endif

