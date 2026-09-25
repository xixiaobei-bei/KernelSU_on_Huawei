/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description:  ivp noc register
 * Create:  2012-2-9
 */
#include "ivp_rdr.h"

#include <linux/io.h>
#include <linux/stat.h>
#include <linux/mm.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/printk.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/hisi/hisi_noc_modid_para.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/rdr_hisi_platform.h>

#include "mntn_subtype_exception.h"
#include "ivp_manager.h"
#include "securec.h"
#include "soc_mid.h"
#include "ivp_log.h"
#include "ivp_smmu.h"

/* ivp core log */
#define IVP_PER_MSG_LEN      256
#define LOG_IDENTITY_POS     4
#define LOG_IDENTITY_MASK    0xffff0000
#define IMAGE_SECT           3
#define SHARE_SECT           4
#define LOG_SECT             5

#define MS_DIV_US            1000
#define RDR_RESET_DELAY      10000   /* 10 s delay */

/* noc target flow */
#define TARGET_FLOW_DEFAULT  0xff

enum RDR_IVP_EXC_TYPE {
	MODID_IVP_START = HISI_BB_MOD_IVP_START,
	MODID_IVP_EXC_NOC_1 = MODID_IVP_START,
	MODID_IVP_EXC_NOC_2,
	MODID_IVP_EXC_NOC_3,
	MODID_IVP_EXC_SMMU,
	MODID_IVP_EXC_END = HISI_BB_MOD_IVP_END,
} rdr_ivp_exc_t;

struct rdr_ivp_device {
	struct ivp_device *pivpdev;
	void __iomem *rdr_addr;
	void* corelog_addr;
	unsigned long log_addr;
	unsigned int log_len;
	struct workqueue_struct *wq;
	struct work_struct err_work;
	struct work_struct dump_work;
	struct list_head err_list;
	spinlock_t err_list_lock;
	struct mutex rdr_dump_mutex;
} rdr_ivp_dev;

static struct noc_err_para_s ivp_noc_para[] = {
	{
		.masterid = (u32)SOC_IVP32_DSP_DSP_CORE_DATA_MID,
		.targetflow = TARGET_FLOW_DEFAULT,
		.bus = NOC_ERRBUS_VCODEC,
	},
	{
		.masterid = (u32)SOC_IVP32_DSP_DSP_CORE_INST_MID,
		.targetflow = TARGET_FLOW_DEFAULT,
		.bus = NOC_ERRBUS_VCODEC,
	},
	{
		.masterid = (u32)SOC_IVP32_DSP_DSP_DMA_MID,
		.targetflow = TARGET_FLOW_DEFAULT,
		.bus = NOC_ERRBUS_VCODEC,
	},
};

static struct rdr_exception_info_s ivp_exc_info[] = {
	{
		.e_modid = (u32)MODID_IVP_EXC_NOC_1,
		.e_modid_end = (u32)MODID_IVP_EXC_NOC_3,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_IVP,
		.e_reset_core_mask = RDR_IVP,
		.e_from_core = RDR_IVP,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = IVP_S_EXCEPTION,
		.e_exce_subtype = IVP_NOC_ERR,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_from_module = "IVP",
		.e_desc = "RDR IVP NOC",
	},
	{
		.e_modid = (u32)MODID_IVP_EXC_SMMU,
		.e_modid_end = (u32)MODID_IVP_EXC_SMMU,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_IVP,
		.e_reset_core_mask = RDR_IVP,
		.e_from_core = RDR_IVP,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = IVP_S_EXCEPTION,
		.e_exce_subtype = IVP_NOC_ERR,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_from_module = "IVP",
		.e_desc = "RDR IVP SMMU",
	},
};

struct rdr_err_type {
	struct list_head node;
	enum RDR_IVP_EXC_TYPE type;
};

static struct rdr_ivp_device g_ivp_rdr_dev;

static char *ivp_calc_last_log_addr(char *log_addr, unsigned int log_size, unsigned int dump_size)
{
	unsigned int i;
	char *last_addr = log_addr;
	unsigned int log_identify;
	unsigned int temp;

	if ((!log_addr) || (log_size < IVP_PER_MSG_LEN))
		return NULL;

	/* get current ivp log identity */
	log_identify =  readl(last_addr + LOG_IDENTITY_POS)&LOG_IDENTITY_MASK;
	last_addr += IVP_PER_MSG_LEN;

	for (i = 1; i < log_size / IVP_PER_MSG_LEN; i++) {
		temp = readl(last_addr + LOG_IDENTITY_POS)&LOG_IDENTITY_MASK;
		if (temp != log_identify)
			break;
		last_addr += IVP_PER_MSG_LEN;
	}
	ivp_info("%s:current id = 0x%x,last id = 0x%x, num = %d", __func__,log_identify, temp, i);
	if (last_addr > log_addr + dump_size)
		return (last_addr - dump_size);
	else
		return log_addr;
}

static void ivp_rdr_dump(
	u32 modid,
	u32 etype, u64 coreid,
	char *pathname,
	pfn_cb_dump_done pfn_cb)
{
	struct rdr_ivp_device *dev = &g_ivp_rdr_dev;
	char *log_addr = NULL;
	unsigned int log_size;
	int ret;
	int offset = 0;

	mutex_lock(&dev->rdr_dump_mutex);
	if (pathname == NULL) {
		ivp_err("%s:pointer is NULL !!\n",  __func__);
		goto dump_err;
	}
	ivp_err(" =========ivp err happen==========\n");
	ivp_err(" modid:          [0x%x]\n",   modid);
	ivp_err(" coreid:         [0x%llx]\n", coreid);
	ivp_err(" exce tpye:      [0x%x]\n",   etype);
	ivp_err(" path name:      [%s]\n",     pathname);

	ivp_dump_status(dev->pivpdev);

	if (is_ivp_in_secmode(dev->pivpdev)) {
		log_addr = (char *)ivp_vmap(dev->pivpdev->sec_sects[LOG_SECT].acpu_addr << IVP_MMAP_SHIFT,
		    dev->pivpdev->sec_sects[LOG_SECT].len, &offset);
		if (log_addr == NULL) {
			ivp_err("sec log_addr vmap fail\n");
			goto dump_err;
		}
		log_size = dev->pivpdev->sec_sects[LOG_SECT].len >  dev->log_len ? dev->log_len :
		    dev->pivpdev->sec_sects[LOG_SECT].len;
		ret = memcpy_s(dev->rdr_addr, dev->log_len, log_addr, log_size);
		vunmap(log_addr - offset);
	} else {
		log_addr = ivp_calc_last_log_addr(dev->corelog_addr,
			dev->pivpdev->sects[LOG_SECT].len, dev->log_len);
		if (!log_addr) {
			ivp_err(" log_addr get fail\n");
			goto dump_err;
		}
		log_size = dev->pivpdev->sects[LOG_SECT].len >  dev->log_len ? dev->log_len :
		    dev->pivpdev->sects[LOG_SECT].len;
		ret = memcpy_s(dev->rdr_addr, dev->log_len, log_addr, log_size);
	}
	if (ret != EOK)
		ivp_err("%s(): memcpy err ret=%d \n", __func__, ret);

dump_err:
	ivp_err(" =========ivp dump exit==========\n");
	if (pfn_cb)
		pfn_cb(modid, coreid);

	mutex_unlock(&dev->rdr_dump_mutex);
	return;
}

static void ivp_rdr_reset(u32 modid, u32 etype, u64 coreid)
{
}

static int ivp_noc_register(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(ivp_noc_para); i++) {
		ivp_info("%s: register noc err, , type:%x",
				__func__, ((u32)MODID_IVP_EXC_NOC_1 + i));
		noc_modid_register(ivp_noc_para[i],
			((u32)MODID_IVP_EXC_NOC_1 + i));
	}

	return EOK;
}

static int ivp_rdr_module_register(void)
{
	struct rdr_module_ops_pub module_ops;
	struct rdr_register_module_result ret_info;
	struct rdr_ivp_device *dev = &g_ivp_rdr_dev;
	int ret;

	ivp_info("%s: enter.\n", __func__);

	module_ops.ops_dump = ivp_rdr_dump;
	module_ops.ops_reset = ivp_rdr_reset;

	ret = rdr_register_module_ops(RDR_IVP, &module_ops, &ret_info);
	if (ret != 0) {
		ivp_err("%s: rdr_register_module_ops failed! return %d\n",  __func__, ret);
		return ret;
	}

	if (dev->pivpdev->vaddr_memory != NULL) {
		dev->corelog_addr = dev->pivpdev->vaddr_memory +
		    dev->pivpdev->sects[IMAGE_SECT].len + dev->pivpdev->sects[SHARE_SECT].len;
	}else {
		dev->corelog_addr = NULL;
		ivp_err("%s: vaddr_memory is null.\n", __func__);
	}

	dev->log_addr = ret_info.log_addr;
	dev->log_len = ret_info.log_len;
	dev->rdr_addr = hisi_bbox_map((phys_addr_t)dev->log_addr, dev->log_len);
	if (!dev->rdr_addr) {
		ivp_err("%s: hisi_bbox_map rdr_addr failed.\n", __func__);
		return -EINVAL;
	}

	return EOK;
}

static int ivp_rdr_exception_register(void)
{
	unsigned int i;
	unsigned int ret;

	for (i = 0; i < ARRAY_SIZE(ivp_exc_info); i++) {
		ivp_info("%s: register rdr exception, i = %d, type:%d", __func__,
				i, ivp_exc_info[i].e_exce_type);

		ret = rdr_register_exception(&ivp_exc_info[i]);
		if (ret != ivp_exc_info[i].e_modid_end) {
			ivp_err("%s: rdr_register_exception failed, ret.%d.\n",
				__func__, ret);
			return -EINVAL;
		}
	}

	return EOK;
}

/* ivp err work thread,invoke by queue_work */
static void rdr_ivp_err_work(struct work_struct *work)
{
	struct rdr_ivp_device *dev = &g_ivp_rdr_dev;
	struct rdr_err_type *entry = NULL;
	struct rdr_err_type *tmp = NULL;

	list_for_each_entry_safe(entry, tmp, &dev->err_list, node) {
		rdr_system_error(entry->type, 0, 0);
		spin_lock_irq(&dev->err_list_lock);
		list_del(&entry->node);
		spin_unlock_irq(&dev->err_list_lock);

		kfree(entry);
	}
}

void invoke_ivp_err(enum RDR_IVP_EXC_TYPE modeid)
{
	struct rdr_err_type *err_info = NULL;
	struct rdr_ivp_device *dev = &g_ivp_rdr_dev;
	/*lint -save -e593*/
	err_info = kzalloc(sizeof(struct rdr_err_type), GFP_ATOMIC);
	if (!err_info) {
		ivp_err("%s: kzalloc failed.\n", __func__);
		return;
	}
	err_info->type = modeid;
	if (dev->wq) {
		spin_lock(&dev->err_list_lock);
		list_add_tail(&err_info->node, &dev->err_list);
		spin_unlock(&dev->err_list_lock);
		queue_work(dev->wq, &dev->err_work);
	} else {
		ivp_err("%s: wq is null.\n", __func__);
		kfree(err_info);
	}
	return;
	/*lint -restore*/
}

void invoke_smmu_err(void)
{
	invoke_ivp_err(MODID_IVP_EXC_SMMU);
}

int ivp_rdr_init(struct ivp_device *pdev)
{
	int ret;
	struct rdr_ivp_device *dev = &g_ivp_rdr_dev;

	ivp_info("[%s] enter", __func__);

	dev->pivpdev = pdev;
	dev->corelog_addr = NULL;
	mutex_init(&dev->rdr_dump_mutex);

	ret = ivp_rdr_module_register();
	if (ret != 0) {
		ivp_err("%s: ivp_rdr_module_register failed.\n", __func__);
		return ret;
	}

	ret = ivp_rdr_exception_register();
	if (ret != 0) {
		ivp_err("%s: ivp_rdr_exception_register failed.\n", __func__);
		return ret;
	}

	ret = ivp_noc_register();
	if (ret != 0) {
		ivp_err("%s: ivp_noc_register register failed.\n", __func__);
		return ret;
	}

	dev->wq = create_singlethread_workqueue("RDR_IVP");
	if (!dev->wq) {
		ivp_err("%s: create_singlethread_workqueue failed.\n", __func__);
		return -EINVAL;
	}

	INIT_WORK(&dev->err_work, rdr_ivp_err_work);
	INIT_LIST_HEAD(&dev->err_list);

	spin_lock_init(&dev->err_list_lock);
	ivp_smmu_set_err_handler(dev->pivpdev->smmu_dev, invoke_smmu_err);

	return ret;
}

int ivp_rdr_deinit(void)
{
	struct rdr_ivp_device *dev = &g_ivp_rdr_dev;
	mutex_destroy(&dev->rdr_dump_mutex);
	if (dev->rdr_addr)
		hisi_bbox_unmap(dev->rdr_addr);
	if (dev->wq)
		destroy_workqueue(dev->wq);

	return EOK;
}

