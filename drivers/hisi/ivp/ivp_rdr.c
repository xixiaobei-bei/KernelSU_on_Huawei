/*
 * ivp noc register
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
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
#include <dsm/dsm_pub.h>
#include "mntn_subtype_exception.h"
#include "ivp_manager.h"
#include "securec.h"
#include "soc_mid.h"
#include "ivp_log.h"
#include "ivp.h"
#include "ivp_smmu.h"

/* ivp core log */
#define IVP_PER_MSG_LEN      256
#define LOG_IDENTITY_POS     4
#define LOG_IDENTITY_MASK    0xffff0000

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
	void *corelog_addr;
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

static char *ivp_calc_last_log_addr(char *log_addr,
	unsigned int log_size, unsigned int dump_size)
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
	ivp_info("current id = 0x%x,last id = 0x%x, num = %d",
		log_identify, temp, i);
	if (last_addr > log_addr + dump_size)
		return (last_addr - dump_size);
	else
		return log_addr;
}

static int ivp_copy_log(struct rdr_ivp_device *dev)
{
	struct ivp_common *pivp_com = &dev->pivpdev->ivp_comm;
	int log_sect = pivp_com->sect_count - 1;
	struct ivp_sect_info *sect = NULL;
	char *log_addr = NULL;
	unsigned int log_size;
	int offset = 0;
	int ret;

	if (is_ivp_in_secmode(dev->pivpdev)) {
		sect = pivp_com->sec_sects + log_sect;
		log_addr = (char*)ivp_vmap(sect->acpu_addr << IVP_MMAP_SHIFT, sect->len, &offset);
		if (!log_addr) {
			ivp_err("sec log_addr vmap fail\n");
			return -1;
		}
		log_size = (sect->len > dev->log_len) ? dev->log_len : sect->len;
		ret = memcpy_s(dev->rdr_addr, dev->log_len, log_addr, log_size);
		vunmap(log_addr - offset);
	} else {
		log_addr = ivp_calc_last_log_addr(dev->corelog_addr,
			pivp_com->sects[log_sect].len, dev->log_len);
		if (!log_addr) {
			ivp_err("log_addr get fail\n");
			return -1;
		}
		log_size = (pivp_com->sects[log_sect].len > dev->log_len) ?
			dev->log_len : pivp_com->sects[log_sect].len;
		ret = memcpy_s(dev->rdr_addr, dev->log_len, log_addr, log_size);
	}
	if (ret != EOK) {
		ivp_err("memcpy err ret=%d \n", ret);
		return -1;
	}

	return 0;
}

static void ivp_rdr_dump(u32 modid, u32 etype, u64 coreid,
	char *pathname, pfn_cb_dump_done pfn_cb)
{
	struct rdr_ivp_device *dev = &g_ivp_rdr_dev;

	mutex_lock(&dev->rdr_dump_mutex);
	ivp_dsm_error_notify(DSM_IVP_PANIC_ERROR_NO);
	if (pathname == NULL) {
		ivp_err("pointer is NULL !!\n");
		goto dump_err;
	}
	ivp_err(" =========ivp err happen==========\n");
	ivp_err(" modid:          [0x%x]\n",   modid);
	ivp_err(" coreid:         [0x%llx]\n", coreid);
	ivp_err(" exce tpye:      [0x%x]\n",   etype);
	ivp_err(" path name:      [%s]\n",     pathname);

	ivp_dump_status(dev->pivpdev);
	ivp_copy_log(dev);

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
		ivp_info("register noc err, , type:%x",
				((u32)MODID_IVP_EXC_NOC_1 + i));
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
	struct ivp_common *pivp_com = &dev->pivpdev->ivp_comm;
	int log_sect = pivp_com->sect_count - 1;
	int ret;

	ivp_info("enter.\n");

	module_ops.ops_dump = ivp_rdr_dump;
	module_ops.ops_reset = ivp_rdr_reset;

	ret = rdr_register_module_ops(RDR_IVP, &module_ops, &ret_info);
	if (ret != 0) {
		ivp_err("rdr_register_module_ops failed! return %d\n", ret);
		return ret;
	}

	if (pivp_com->vaddr_memory) {
		dev->corelog_addr =
			pivp_com->vaddr_memory +
			(pivp_com->dynamic_mem_size - pivp_com->sects[log_sect].len);
	} else {
		dev->corelog_addr = NULL;
		ivp_err("vaddr_memory is null.\n");
	}

	dev->log_addr = ret_info.log_addr;
	dev->log_len = ret_info.log_len;
	dev->rdr_addr = hisi_bbox_map((phys_addr_t)dev->log_addr, dev->log_len);
	if (!dev->rdr_addr) {
		ivp_err("hisi_bbox_map rdr_addr failed.\n");
		return -EINVAL;
	}

	return EOK;
}

static int ivp_rdr_exception_register(void)
{
	unsigned int i;
	unsigned int ret;

	for (i = 0; i < ARRAY_SIZE(ivp_exc_info); i++) {
		ivp_info("register rdr exception, i = %d, type:%d",
				i, ivp_exc_info[i].e_exce_type);

		ret = rdr_register_exception(&ivp_exc_info[i]);
		if (ret != ivp_exc_info[i].e_modid_end) {
			ivp_err("rdr_register_exception failed, ret.%d.\n", ret);
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
		ivp_err("kzalloc failed.\n");
		return;
	}
	err_info->type = modeid;
	if (dev->wq) {
		spin_lock(&dev->err_list_lock);
		list_add_tail(&err_info->node, &dev->err_list);
		spin_unlock(&dev->err_list_lock);
		queue_work(dev->wq, &dev->err_work);
	} else {
		ivp_err("wq is null.\n");
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

	if (!pdev) {
		ivp_err("invalid input");
		return -EINVAL;
	}
	ivp_info("enter");

	dev->pivpdev = pdev;
	dev->corelog_addr = NULL;
	mutex_init(&dev->rdr_dump_mutex);

	ret = ivp_rdr_module_register();
	if (ret != 0) {
		ivp_err("ivp_rdr_module_register failed.\n");
		return ret;
	}

	ret = ivp_rdr_exception_register();
	if (ret != 0) {
		ivp_err("ivp_rdr_exception_register failed.\n");
		return ret;
	}
#if   CONFIG_HISI_NOC_MODID_REGISTER
	ret = ivp_noc_register();
#endif
	if (ret != 0) {
		ivp_err("ivp_noc_register register failed.\n");
		return ret;
	}

	dev->wq = create_singlethread_workqueue("RDR_IVP");
	if (!dev->wq) {
		ivp_err("create_singlethread_workqueue failed.\n");
		return -EINVAL;
	}

	INIT_WORK(&dev->err_work, rdr_ivp_err_work);
	INIT_LIST_HEAD(&dev->err_list);

	spin_lock_init(&dev->err_list_lock);
	ivp_smmu_set_err_handler(dev->pivpdev->ivp_comm.smmu_dev, invoke_smmu_err);
	return ret;
}

int ivp_rdr_deinit(void)
{
	struct rdr_ivp_device *dev = &g_ivp_rdr_dev;
	mutex_destroy(&dev->rdr_dump_mutex);
	if (dev->rdr_addr) {
		hisi_bbox_unmap(dev->rdr_addr);
		dev->rdr_addr = NULL;
	}
	if (dev->wq) {
		destroy_workqueue(dev->wq);
		dev->wq = NULL;
	}

	return EOK;
}

