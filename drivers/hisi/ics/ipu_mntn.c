#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/io.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/workqueue.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <asm/compiler.h>
#include <mntn_subtype_exception.h>
#include <mntn_public_interface.h>

#include "ipu_mntn.h"
#include "ipu_smmu_drv.h"
#include "cambricon_ipu.h"

struct rdr_exception_info_s ipu_excetption_info[] = {
	{
		.e_modid            = (u32)MODID_NPU_EXC_DEAD,
		.e_modid_end        = (u32)MODID_NPU_EXC_DEAD,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask  = RDR_NPU,
		.e_from_core        = RDR_NPU,
		.e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type        = NPU_S_EXCEPTION,
		.e_exce_subtype     = NPU_EXC_DEAD,
		.e_upload_flag      = (u32)RDR_UPLOAD_YES,
		.e_from_module      = "NPU",
		.e_desc             = "NPU_EXC_DEAD",
	},
	{
		.e_modid            = (u32)MODID_NPU_EXC_SET_BACK_CLOCK_FAIL,
		.e_modid_end        = (u32)MODID_NPU_EXC_SET_BACK_CLOCK_FAIL,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask  = RDR_NPU,
		.e_from_core        = RDR_NPU,
		.e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type        = NPU_S_EXCEPTION,
		.e_exce_subtype     = NPU_SET_BACK_CLOCK_FAIL,
		.e_upload_flag      = (u32)RDR_UPLOAD_YES,
		.e_from_module      = "NPU",
		.e_desc             = "NPU_SET_BACK_CLOCK_FAIL",
	},
	{
		.e_modid            = (u32)MODID_NPU_EXC_SET_CLOCK_FAIL,
		.e_modid_end        = (u32)MODID_NPU_EXC_SET_CLOCK_FAIL,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask  = RDR_NPU,
		.e_from_core        = RDR_NPU,
		.e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type        = NPU_S_EXCEPTION,
		.e_exce_subtype     = NPU_SET_CLOCK_FAIL,
		.e_upload_flag      = (u32)RDR_UPLOAD_YES,
		.e_from_module      = "NPU",
		.e_desc             = "NPU_SET_CLOCK_FAIL",
	},
	{
		.e_modid            = (u32)MODID_NPU_EXC_SET_POWER_UP_FAIL,
		.e_modid_end        = (u32)MODID_NPU_EXC_SET_POWER_UP_FAIL,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask  = RDR_NPU,
		.e_from_core        = RDR_NPU,
		.e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type        = NPU_S_EXCEPTION,
		.e_exce_subtype     = NPU_POWER_UP_FAIL,
		.e_upload_flag      = (u32)RDR_UPLOAD_YES,
		.e_from_module      = "NPU",
		.e_desc             = "NPU_POWER_UP_FAIL",
	},
	{
		.e_modid            = (u32)MODID_NPU_EXC_SET_POWER_UP_STATUS_FAULT,
		.e_modid_end        = (u32)MODID_NPU_EXC_SET_POWER_UP_STATUS_FAULT,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask  = RDR_NPU,
		.e_from_core        = RDR_NPU,
		.e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type        = NPU_S_EXCEPTION,
		.e_exce_subtype     = NPU_POWER_UP_STA_FAULT,
		.e_upload_flag      = (u32)RDR_UPLOAD_YES,
		.e_from_module      = "NPU",
		.e_desc             = "NPU_POWER_UP_STA_FAULT",
	},
	{
		.e_modid            = (u32)MODID_NPU_EXC_SET_POWER_DOWN_FAIL,
		.e_modid_end        = (u32)MODID_NPU_EXC_SET_POWER_DOWN_FAIL,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask  = RDR_NPU,
		.e_from_core        = RDR_NPU,
		.e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type        = NPU_S_EXCEPTION,
		.e_exce_subtype     = NPU_POWER_DOWN_FAIL,
		.e_upload_flag      = (u32)RDR_UPLOAD_YES,
		.e_from_module      = "NPU",
		.e_desc             = "NPU_POWER_DOWN_FAIL",
	},
	{
		.e_modid            = (u32)MODID_NPU_EXC_INTERRUPT_ABNORMAL,
		.e_modid_end        = (u32)MODID_NPU_EXC_INTERRUPT_ABNORMAL,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask  = RDR_NPU,
		.e_from_core        = RDR_NPU,
		.e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type        = NPU_S_EXCEPTION,
		.e_exce_subtype     = NPU_INTERRUPT_ABNORMAL,
		.e_upload_flag      = (u32)RDR_UPLOAD_YES,
		.e_from_module      = "NPU",
		.e_desc             = "NPU_INTERRUPT_ABNORMAL",
	}
};

struct work_struct ipu_dump_work;
struct workqueue_struct *ipu_mntn_rdr_wq;

struct ipu_mntn_info_s ipu_mntn_info;
struct ipu_reg_info_s  ipu_reg_info;

extern struct cambricon_ipu_private *adapter;

/********************************************************************
Description: ipu_mntn_copy_reg_to_bbox
input:	char *src_addr, unsigned int* offset, unsigned int len
output:	NA
return:	void
********************************************************************/
static int ipu_mntn_copy_reg_to_bbox(char *src_addr, unsigned int len)
{
	unsigned int temp_offset = 0;

	if ((NULL == src_addr) || (0 == len)) {
		printk(KERN_ERR"[%s]:IPU_ERROR:Input parameter is error!\n", __func__);
		return -EINVAL;
	}

	temp_offset = ipu_mntn_info.bbox_addr_offset + len;
	//ipu_bbox alloc size 64k
	if (temp_offset > 0x10000) {
		printk(KERN_ERR"[%s]:IPU_ERROR:Copy log to bbox size is error! temp_offset=%d\n", __func__, temp_offset);
		temp_offset = 0;
		ipu_mntn_info.bbox_addr_offset = 0;
		return -ENOMEM ;
	}

	memcpy(((char*)ipu_mntn_info.rdr_addr + ipu_mntn_info.bbox_addr_offset), src_addr, len);
	ipu_mntn_info.bbox_addr_offset = temp_offset;

	return 0;

}
/********************************************************************
Description: ipu_mntn_write_adapter_info
input:	char *file_path
output:	NA
return:	void
********************************************************************/
static void ipu_mntn_write_adapter_info(void)
{
	char log_buf[IPU_LINE_MAX + 1] = {0};

	snprintf(log_buf, IPU_LINE_MAX, "npu_status=%d, ttbr0=%lx, inst_set=%d, offchip{set=%x, base=%x}, last_computed_task=%ld.\r\n",
			 adapter->ipu_power_up,
			 adapter->smmu_ttbr0,
			 adapter->boot_inst_set.boot_inst_recorded_is_config,
			 adapter->boot_inst_set.access_ddr_addr_is_config,
			 adapter->boot_inst_set.ipu_access_ddr_addr,
			 adapter->computed_task_cnt);

	ipu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

	return;
}
/********************************************************************
Description: ipu_mntn_write_peri_reg_info
input:	char *file_path
output:	NA
return:	void
********************************************************************/
static void ipu_mntn_write_peri_reg_info(void)
{
	char log_buf[IPU_LINE_MAX + 1] = {0};

	snprintf(log_buf, IPU_LINE_MAX, "peri_stat=%x, ppll_select=%x, power_stat=%x, power_ack=%x, reset_stat=%x, perclken=%x, perstat=%x.\r\n",
			 ipu_reg_info.peri_reg.peri_stat,
			 ipu_reg_info.peri_reg.ppll_select,
			 ipu_reg_info.peri_reg.power_stat,
			 ipu_reg_info.peri_reg.power_ack,
			 ipu_reg_info.peri_reg.reset_stat,
			 ipu_reg_info.peri_reg.perclken0,
			 ipu_reg_info.peri_reg.perstat0);

	ipu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));
	return;
}
/********************************************************************
Description: ipu_mntn_write_mstr_reg_info
input:	char *file_path
output:	NA
return:	void
********************************************************************/
static void ipu_mntn_write_mstr_reg_info(void)
{
	char log_buf[IPU_BUF_LEN_MAX + 1] = {0};

	snprintf(log_buf, IPU_BUF_LEN_MAX, "RD_BITMAP=%x, WR_BITMAP=%x, rd_cmd_total_cnt[0-3]={%x, %x, %x}, wr_cmd_total_cnt=%x\n",
			ipu_reg_info.mstr_reg.rd_bitmap,
			ipu_reg_info.mstr_reg.wr_bitmap,
			ipu_reg_info.mstr_reg.rd_cmd_total_cnt0,
			ipu_reg_info.mstr_reg.rd_cmd_total_cnt0,
			ipu_reg_info.mstr_reg.rd_cmd_total_cnt2,
			ipu_reg_info.mstr_reg.wr_cmd_total_cnt);

	ipu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

	return;
}
/********************************************************************
Description: ipu_mntn_write_reg_log
input:	void
output:	NA
return:	void
********************************************************************/
static void ipu_mntn_write_reg_log(void)
{
	switch (ipu_mntn_info.dump_info.modid) {
		case MODID_NPU_EXC_DEAD:  //lint !e650
			ipu_mntn_write_adapter_info();
			ipu_mntn_write_peri_reg_info();
			ipu_mntn_write_mstr_reg_info();
			ipu_mntn_copy_reg_to_bbox(register_info, strlen(register_info));
			break;

		case MODID_NPU_EXC_SET_BACK_CLOCK_FAIL:  //lint !e650
		case MODID_NPU_EXC_SET_CLOCK_FAIL:  //lint !e650
		case MODID_NPU_EXC_SET_POWER_UP_FAIL:  //lint !e650
		case MODID_NPU_EXC_SET_POWER_UP_STATUS_FAULT:  //lint !e650
		case MODID_NPU_EXC_SET_POWER_DOWN_FAIL:  //lint !e650
		case MODID_NPU_EXC_INTERRUPT_ABNORMAL:   //lint !e650
			break;

		default:
			break;
	}

	return;
}
/********************************************************************
Description: ipu_mntn_rdr_dump
input:	modid: module id
		etype:exception type
		coreid: core id
		pathname: log path
		pfn_cb: callback function
output:	NA
return:	NA
********************************************************************/
static void ipu_mntn_rdr_dump(u32 modid, u32 etype, u64 coreid, char *pathname, pfn_cb_dump_done pfn_cb)
{
	if (NULL == pathname) {
		 printk(KERN_ERR"[%s]:IPU_ERROR:pathname is empty\n", __func__);
		 return;
	}
	ipu_mntn_info.dump_info.modid = modid;
	ipu_mntn_info.dump_info.coreid = coreid;
	ipu_mntn_info.dump_info.pathname = pathname;
	ipu_mntn_info.dump_info.cb = pfn_cb;
	ipu_mntn_info.bbox_addr_offset = 0;
	queue_work(ipu_mntn_rdr_wq, &ipu_dump_work);
	return;
}
/********************************************************************
Description: ipu_mntn_rdr_reset
input:	modid:module id
		etype:exception type
		coreid:core id
output:	NA
return:	NA
********************************************************************/
static void ipu_mntn_rdr_reset(u32 modid, u32 etype, u64 coreid)
{
	return;
}
/********************************************************************
Description: ipu_mntn_dump_work
input:	struct work_struct *work
output:	NA
return:	NA
********************************************************************/
static void ipu_mntn_dump_work(struct work_struct *work)
{
	ipu_mntn_write_reg_log();

	if (ipu_mntn_info.dump_info.cb) {
		ipu_mntn_info.dump_info.cb(ipu_mntn_info.dump_info.modid, ipu_mntn_info.dump_info.coreid);
	}

	return;
}
/********************************************************************
Description:	ipu_mntn_register_exception
input:	NA
output: NA
return: int
********************************************************************/
static int ipu_mntn_register_exception(void)
{
    int ret;
	unsigned int  size;
	unsigned long index;

	size = sizeof(ipu_excetption_info)/sizeof(struct rdr_exception_info_s);
	for (index = 0; index < size; index++) {
		/* error return 0, ok return modid */
		ret = rdr_register_exception(&ipu_excetption_info[index]);
		if (!ret) {
			printk(KERN_ERR"[%s]:IPU_ERROR:rdr_register_exception is failed! index=%ld ret=%d\n", __func__, index, ret);
            return -EINTR;
		}
	}

    return 0;
}
/********************************************************************
Description:	register ipu dump and reset function
input:	NA
output:	NA
return:	int
********************************************************************/
static int ipu_mntn_register_core(void)
{
	int ret;
	struct rdr_module_ops_pub s_soc_ops;

	s_soc_ops.ops_dump = ipu_mntn_rdr_dump;
	s_soc_ops.ops_reset = ipu_mntn_rdr_reset;
	/* register ipu core dump and reset function */
	ret = rdr_register_module_ops((u64)RDR_NPU, &s_soc_ops, &ipu_mntn_info.ipu_ret_info);
	if (ret != 0) {
		printk(KERN_ERR"[%s]:IPU_ERROR:rdr_register_module_ops is failed! ret=0x%08x\n", __func__, ret);
	}

	return ret;
}
/********************************************************************
Description:	init ipu addr function
input:	NA
output:	NA
return:	int
********************************************************************/
static int ipu_mntn_addr_map(void)
{
	ipu_mntn_info.rdr_addr = hisi_bbox_map((phys_addr_t)ipu_mntn_info.ipu_ret_info.log_addr, ipu_mntn_info.ipu_ret_info.log_len);
	if (!ipu_mntn_info.rdr_addr) {
		printk(KERN_ERR"[%s]:IPU_ERROR:hisi_bbox_map is failed!\n", __func__);
		return -EFAULT;
	}

	return 0;
}
/********************************************************************
Description:	ipu_mntn_rdr_resource_init
input:	NA
output:	NA
return:	int
********************************************************************/
static int ipu_mntn_rdr_resource_init(void)
{
	ipu_mntn_rdr_wq = create_singlethread_workqueue("ipu_mntn_rdr_wq");
	if (!ipu_mntn_rdr_wq) {
		printk(KERN_ERR"[%s]:IPU_ERROR:Create_singlethread_workqueue is failed!\n", __func__);
		return -EINTR;
	}

	INIT_WORK(&ipu_dump_work, ipu_mntn_dump_work);

	return 0;
}
/********************************************************************
Description: ipu_mntn_rdr_init
input:	void
output:	NA
return:	int
********************************************************************/
int ipu_mntn_rdr_init(void)
{
	int ret;

	ret = ipu_mntn_rdr_resource_init();
	if (0 != ret) {
		 printk(KERN_ERR"[%s]:IPU_ERROR:ipu_mntn_rdr_resource_init is faild!ret=%d\n", __func__, ret);
		 return ret;
	}

	/* register ics exception */
	ret = ipu_mntn_register_exception();
	if (0 != ret) {
		 printk(KERN_ERR"[%s]:IPU_ERROR:ipu_mntn_register_exception is faild!ret=%d\n", __func__, ret);
		 return ret;
	}

	/* register ics dump and reset function */
	ret = ipu_mntn_register_core();
	if (0 != ret) {
		printk(KERN_ERR"[%s]:IPU_ERROR:ipu_register_core is failed!ret=%d\n", __func__, ret);
		return ret;
	}

	ret = ipu_mntn_addr_map();
	if (0 != ret) {
		printk(KERN_ERR"[%s]:IPU_ERROR:ipu_mntn_addr_map is failed!ret=%d\n", __func__, ret);
		return ret;
	}

	return 0;

}
