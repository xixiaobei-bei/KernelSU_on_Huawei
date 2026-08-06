#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/workqueue.h>
#include <mntn_subtype_exception.h>
#include <mntn_public_interface.h>

#include "npu_public.h"
#include "npu_mntn.h"

/* AI DRD */
#define NPU_BUF_LEN_MAX        (256)
#define NPU_BBOX_MEM_MAX       (0x10000)

struct rdr_dump_info_s {
	u32 modid;
	u64 coreid;
	pfn_cb_dump_done cb;
    char *pathname;
};

struct npu_mntn_info_s {
	unsigned int bbox_addr_offset;
	struct rdr_register_module_result npu_ret_info;
	struct rdr_dump_info_s dump_info;
	void   *rdr_addr;
};

struct npu_reg_info_s {
	struct npu_peri_reg_s peri_reg;
	struct npu_mstr_reg_s mstr_reg;
};

struct npu_mntn_private_s {
    unsigned int core_id;
	struct npu_mntn_info_s mntn_info;
	struct npu_reg_info_s  reg_info[MAX_SUPPORT_CORE_NUM];
    struct npu_exc_info_s  exc_info[MAX_SUPPORT_CORE_NUM];
    struct work_struct     dump_work;
	struct workqueue_struct *rdr_wq;
};

struct npu_mntn_private_s *g_mntn_private;

struct rdr_exception_info_s npu_excetption_info[] = {
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
struct dsm_client *dsm_ai_client = NULL;

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


/********************************************************************
Description: npu_mntn_copy_reg_to_bbox
input:	char *src_addr, unsigned int* offset, unsigned int len
output:	NA
return:	void
********************************************************************/
static int npu_mntn_copy_reg_to_bbox(char *src_addr, unsigned int len)
{
	unsigned int temp_offset = 0;

	if ((NULL == src_addr) || (0 == len)) {
		NPU_ERR("Input parameter is error!");
		return -EINVAL;
	}

	temp_offset =g_mntn_private->mntn_info.bbox_addr_offset + len;
	//npu_bbox alloc size 64k
	if (temp_offset >= NPU_BBOX_MEM_MAX) {
		NPU_ERR("Copy log to bbox is error! temp_offset=%d", temp_offset);
		temp_offset = 0;
		g_mntn_private->mntn_info.bbox_addr_offset = 0;
		return -ENOMEM ;
	}

	memcpy(((char*)g_mntn_private->mntn_info.rdr_addr + g_mntn_private->mntn_info.bbox_addr_offset), src_addr, len);
	g_mntn_private->mntn_info.bbox_addr_offset = temp_offset;

	return 0;

}
/********************************************************************
Description: npu_mntn_write_peri_reg_info
input:	char *file_path
output:	NA
return:	void
********************************************************************/
static void npu_mntn_write_peri_reg_info(unsigned int core_id)
{
	char log_buf[NPU_BUF_LEN_MAX + 1] = {0};
	struct npu_peri_reg_s *peri_reg = &g_mntn_private->reg_info[core_id].peri_reg;

	snprintf(log_buf, NPU_BUF_LEN_MAX, "peri_stat=%x, ppll_select=%x, power_stat=%x, power_ack=%x, reset_stat=%x, perclken=%x, perstat=%x.\r\n",
			 peri_reg->peri_stat,
			 peri_reg->ppll_select,
			 peri_reg->power_stat,
			 peri_reg->power_ack,
			 peri_reg->reset_stat,
			 peri_reg->perclken0,
			 peri_reg->perstat0);

	npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));
	return;
}
/********************************************************************
Description: npu_mntn_write_mstr_reg_info
input:	char *file_path
output:	NA
return:	void
********************************************************************/
static void npu_mntn_write_mstr_reg_info(unsigned int core_id)
{
	char log_buf[NPU_BUF_LEN_MAX + 1] = {0};
	struct npu_mstr_reg_s *mstr_reg = &g_mntn_private->reg_info[core_id].mstr_reg;

	snprintf(log_buf, NPU_BUF_LEN_MAX, "RD_BITMAP=%x, WR_BITMAP=%x, rd_cmd_total_cnt[0-3]={%x, %x, %x}, wr_cmd_total_cnt=%x\n",
			mstr_reg->rd_bitmap,
			mstr_reg->wr_bitmap,
			mstr_reg->rd_cmd_total_cnt0,
			mstr_reg->rd_cmd_total_cnt0,
			mstr_reg->rd_cmd_total_cnt2,
			mstr_reg->wr_cmd_total_cnt);

	npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

	return;
}
/********************************************************************
Description: npu_mntn_write_reg_log
input:	void
output:	NA
return:	void
********************************************************************/
static void npu_mntn_write_reg_log(unsigned int core_id)
{
    char log_buf[NPU_BUF_LEN_MAX + 1] = {0};

    memset((char*)g_mntn_private->mntn_info.rdr_addr, 0, NPU_BBOX_MEM_MAX);

    snprintf(log_buf, NPU_BUF_LEN_MAX, "\r\ncore_id=0x%x\r\n", core_id);
	npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

	switch (g_mntn_private->mntn_info.dump_info.modid) {
		case MODID_NPU_EXC_DEAD:  //lint !e650
			snprintf(log_buf, NPU_BUF_LEN_MAX, "interrupt_status=0x%x\r\n", g_mntn_private->exc_info[core_id].interrupt_status);
			npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

            npu_mntn_write_peri_reg_info(core_id);
			npu_mntn_write_mstr_reg_info(core_id);
			break;

		case MODID_NPU_EXC_SET_POWER_UP_FAIL:  //lint !e650
		case MODID_NPU_EXC_SET_POWER_DOWN_FAIL:  //lint !e650
			snprintf(log_buf, NPU_BUF_LEN_MAX, "ip=0x%x ret=0x%x\r\n", g_mntn_private->exc_info[core_id].target_ip, g_mntn_private->exc_info[core_id].result);
			npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));
			break;

		default:
			break;
	}

	return;
}
/********************************************************************
Description: npu_mntn_rdr_dump
input:	modid: module id
		etype:exception type
		coreid: core id
		pathname: log path
		pfn_cb: callback function
output:	NA
return:	NA
********************************************************************/
static void npu_mntn_rdr_dump(u32 modid, u32 etype, u64 coreid, char *pathname, pfn_cb_dump_done pfn_cb)
{
	if (NULL == pathname) {
		 NPU_ERR("pathname is empty!");
		 return;
	}
	g_mntn_private->mntn_info.dump_info.modid = modid;
	g_mntn_private->mntn_info.dump_info.coreid = coreid;
	g_mntn_private->mntn_info.dump_info.pathname = pathname;
	g_mntn_private->mntn_info.dump_info.cb = pfn_cb;
	g_mntn_private->mntn_info.bbox_addr_offset = 0;
	queue_work(g_mntn_private->rdr_wq, &g_mntn_private->dump_work);
	return;
}
/********************************************************************
Description: npu_mntn_rdr_reset
input:	modid:module id
		etype:exception type
		coreid:core id
output:	NA
return:	NA
********************************************************************/
static void npu_mntn_rdr_reset(u32 modid, u32 etype, u64 coreid)
{
	return;
}
/********************************************************************
Description: npu_mntn_dump_work
input:	struct work_struct *work
output:	NA
return:	NA
********************************************************************/
static void npu_mntn_dump_work(struct work_struct *work)
{
	npu_mntn_write_reg_log(g_mntn_private->core_id);

	if (g_mntn_private->mntn_info.dump_info.cb) {
		g_mntn_private->mntn_info.dump_info.cb(g_mntn_private->mntn_info.dump_info.modid, g_mntn_private->mntn_info.dump_info.coreid);
	}

	return;
}
/********************************************************************
Description:	npu_mntn_register_exception
input:	NA
output: NA
return: int
********************************************************************/
static int npu_mntn_register_exception(void)
{
    int ret;
	unsigned int  size;
	unsigned long index;

	size = sizeof(npu_excetption_info)/sizeof(struct rdr_exception_info_s);
	for (index = 0; index < size; index++) {
		/* error return 0, ok return modid */
		ret = rdr_register_exception(&npu_excetption_info[index]);
		if (!ret) {
			NPU_ERR("rdr_register_exception is failed! index=%ld ret=%d", index, ret);
            return -EINTR;
		}
	}

    return 0;
}
/********************************************************************
Description:	register npu dump and reset function
input:	NA
output:	NA
return:	int
********************************************************************/
static int npu_mntn_register_core(void)
{
	int ret;
	struct rdr_module_ops_pub s_soc_ops;

	s_soc_ops.ops_dump = npu_mntn_rdr_dump;
	s_soc_ops.ops_reset = npu_mntn_rdr_reset;
	/* register npu core dump and reset function */
	ret = rdr_register_module_ops((u64)RDR_NPU, &s_soc_ops, &g_mntn_private->mntn_info.npu_ret_info);
	if (ret != 0) {
		NPU_ERR("rdr_register_module_ops is failed! ret=0x%08x", ret);
	}

	return ret;
}
/********************************************************************
Description:	init npu addr function
input:	NA
output:	NA
return:	int
********************************************************************/
static int npu_mntn_addr_map(void)
{
	g_mntn_private->mntn_info.rdr_addr = hisi_bbox_map((phys_addr_t)g_mntn_private->mntn_info.npu_ret_info.log_addr, g_mntn_private->mntn_info.npu_ret_info.log_len);
	if (!g_mntn_private->mntn_info.rdr_addr) {
		NPU_ERR("hisi_bbox_map is failed!");
		return -EFAULT;
	}

	return 0;
}
/********************************************************************
Description:	npu_mntn_rdr_resource_init
input:	NA
output:	NA
return:	int
********************************************************************/
static int npu_mntn_rdr_resource_init(void)
{
	g_mntn_private = kzalloc(sizeof(struct npu_mntn_private_s), GFP_KERNEL);
	if (NULL == g_mntn_private) {
		NPU_ERR("Failed to allocate memory!");
		return -ENOMEM;
	}

	g_mntn_private->rdr_wq = create_singlethread_workqueue("npu_mntn_rdr_wq");
	if (!g_mntn_private->rdr_wq) {
		NPU_ERR("Create_singlethread_workqueue is failed!");
		kfree(g_mntn_private);
		g_mntn_private = NULL;
		return -EINTR;
	}

	INIT_WORK(&g_mntn_private->dump_work, npu_mntn_dump_work);

	return 0;
}
/********************************************************************
Description: npu_mntn_rdr_init
input:	void
output:	NA
return:	int
********************************************************************/
int npu_mntn_rdr_init(void)
{
	int ret;

	ret = npu_mntn_rdr_resource_init();
	if (0 != ret) {
		 NPU_ERR("npu_mntn_rdr_resource_init is faild!ret=%d", ret);
		 return ret;
	}

	/* register ics exception */
	ret = npu_mntn_register_exception();
	if (0 != ret) {
		 NPU_ERR("npu_mntn_register_exception is faild!ret=%d", ret);
		 goto error;
	}

	/* register ics dump and reset function */
	ret = npu_mntn_register_core();
	if (0 != ret) {
		NPU_ERR("npu_register_core is failed!ret=%d", ret);
		goto error;
	}

	ret = npu_mntn_addr_map();
	if (0 != ret) {
		NPU_ERR("npu_mntn_addr_map is failed!ret=%d", ret);
		goto error;
	}

	/* Register AI dsm client */
	DSM_AI_REGISTER();

	return 0;

error:
	if (NULL != g_mntn_private) {
		destroy_workqueue(g_mntn_private->rdr_wq);
		kfree(g_mntn_private);
		g_mntn_private = NULL;
	}
	return ret;
}

void npu_mntn_rdr_exit(void)
{
	if (NULL != g_mntn_private) {
		destroy_workqueue(g_mntn_private->rdr_wq);
		kfree(g_mntn_private);
		g_mntn_private = NULL;
	}

	/* Unregister ai client for DSM */
	DSM_AI_UNREGISTER();

	return;
}

void npu_mntn_peri_reg_init(unsigned int core_id, struct npu_peri_reg_s *peri_reg_ptr)
{
	CHECK_COREID_INVALID_RETURN_VOID(core_id);

	if (NULL == peri_reg_ptr) {
		NPU_ERR("Input parameter is error!");
		return;
	}

	g_mntn_private->core_id = core_id;

	memcpy(&g_mntn_private->reg_info[core_id].peri_reg, peri_reg_ptr, sizeof(struct npu_peri_reg_s));
	return;
}

void npu_mntn_mstr_reg_init(unsigned int core_id, struct npu_mstr_reg_s *mstr_reg_ptr)
{
	CHECK_COREID_INVALID_RETURN_VOID(core_id);

	if (NULL == mstr_reg_ptr) {
		NPU_ERR("Input parameter is error!");
		return;
	}

	g_mntn_private->core_id = core_id;

	memcpy(&g_mntn_private->reg_info[core_id].mstr_reg, mstr_reg_ptr, sizeof(struct npu_mstr_reg_s));
	return;
}

static void npu_mntn_exc_info_init(unsigned int core_id, struct npu_exc_info_s *exc_info_ptr)
{
	if (NULL == exc_info_ptr) {
		NPU_ERR("Input parameter is error!");
		return;
	}

	g_mntn_private->core_id = core_id;
	memcpy(&g_mntn_private->exc_info[core_id], exc_info_ptr, sizeof(struct npu_exc_info_s));

	return;
}

void npu_mntn_power_exc_proc(unsigned int core_id, unsigned int target_ip, int result)
{
	struct npu_exc_info_s exc_info;

	CHECK_COREID_INVALID_RETURN_VOID(core_id);

	memset(&exc_info, 0, sizeof(struct npu_exc_info_s));
	exc_info.target_ip = target_ip;
	exc_info.result = result;
	npu_mntn_exc_info_init(core_id, &exc_info);

	return;
}

void npu_mntn_interrupt_exc_proc(unsigned int core_id, unsigned int interrupt_status)
{
	struct npu_exc_info_s exc_info;

	CHECK_COREID_INVALID_RETURN_VOID(core_id);

	memset(&exc_info, 0, sizeof(struct npu_exc_info_s));
	exc_info.interrupt_status = interrupt_status;
	npu_mntn_exc_info_init(core_id, &exc_info);

	return;
}

void npu_mntn_set_core_id(unsigned int core_id)
{
	CHECK_COREID_INVALID_RETURN_VOID(core_id);

	g_mntn_private->core_id = core_id;

	return;
}

