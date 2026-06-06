#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/idr.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/stat.h>
#include <linux/io.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#include "npu_shm.h"
#include "npu_manager_common.h"
#include "npu_black_box.h"
#include "npu_platform.h"
#include "mntn_public_interface.h"
#include "mntn_subtype_exception.h"

static vir_addr_t g_bbox_addr_vir = 0;
static vir_addr_t g_bbox_tslog_addr_vir = 0;
static vir_addr_t g_bbox_aicpulog_addr_vir = 0;

struct devdrv_npu_mntn_private_s *g_devdrv_npu_mntn_private = NULL;

struct rdr_exception_info_s devdrv_npu_rdr_excetption_info[] =
{
    {
        .e_modid            = (u32)EXC_TYPE_TS_AICORE_EXCEPTION,
        .e_modid_end        = (u32)EXC_TYPE_TS_AICORE_EXCEPTION,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_NO,
        .e_notify_core_mask = RDR_NPU,
        .e_reset_core_mask  = RDR_NPU,
        .e_from_core        = RDR_NPU,
        .e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = NPU_S_EXCEPTION,
        .e_exce_subtype     = AICORE_EXCP,
        .e_upload_flag      = (u32)RDR_UPLOAD_YES,
        .e_from_module      = "NPU",
        .e_desc             = "AICORE_EXCP",
    },
    {
        .e_modid            = (u32)EXC_TYPE_TS_AICORE_TIMEOUT,
        .e_modid_end        = (u32)EXC_TYPE_TS_AICORE_TIMEOUT,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_NO,
        .e_notify_core_mask = RDR_NPU,
        .e_reset_core_mask  = RDR_NPU,
        .e_from_core        = RDR_NPU,
        .e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = NPU_S_EXCEPTION,
        .e_exce_subtype     = AICORE_TIMEOUT,
        .e_upload_flag      = (u32)RDR_UPLOAD_YES,
        .e_from_module      = "NPU",
        .e_desc             = "AICORE_TIMEOUT",
    },
    {
        .e_modid            = (u32)RDR_EXC_TYPE_TS_RUNNING_EXCEPTION,
        .e_modid_end        = (u32)RDR_EXC_TYPE_TS_RUNNING_EXCEPTION,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_NO,
        .e_notify_core_mask = RDR_NPU,
        .e_reset_core_mask  = RDR_NPU,
        .e_from_core        = RDR_NPU,
        .e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = NPU_S_EXCEPTION,
        .e_exce_subtype     = TS_RUNNING_EXCP,
        .e_upload_flag      = (u32)RDR_UPLOAD_YES,
        .e_from_module      = "NPU",
        .e_desc             = "TS_RUNNING_EXCP",
    },
    {
        .e_modid            = (u32)RDR_EXC_TYPE_TS_RUNNING_TIMEOUT,
        .e_modid_end        = (u32)RDR_EXC_TYPE_TS_RUNNING_TIMEOUT,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_NO,
        .e_notify_core_mask = RDR_NPU,
        .e_reset_core_mask  = RDR_NPU,
        .e_from_core        = RDR_NPU,
        .e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = NPU_S_EXCEPTION,
        .e_exce_subtype     = TS_RUNNING_TIMEOUT,
        .e_upload_flag      = (u32)RDR_UPLOAD_YES,
        .e_from_module      = "NPU",
        .e_desc             = "TS_RUNNING_TIMEOUT",
    },
    {
        .e_modid            = (u32)RDR_EXC_TYPE_TS_INIT_EXCEPTION,
        .e_modid_end        = (u32)RDR_EXC_TYPE_TS_INIT_EXCEPTION,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_NO,
        .e_notify_core_mask = RDR_NPU,
        .e_reset_core_mask  = RDR_NPU,
        .e_from_core        = RDR_NPU,
        .e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = NPU_S_EXCEPTION,
        .e_exce_subtype     = TS_INIT_EXCP,
        .e_upload_flag      = (u32)RDR_UPLOAD_YES,
        .e_from_module      = "NPU",
        .e_desc             = "TS_INIT_EXCP",
    },
    {
        .e_modid            = (u32)RDR_EXC_TYPE_AICPU_INIT_EXCEPTION,
        .e_modid_end        = (u32)RDR_EXC_TYPE_AICPU_INIT_EXCEPTION,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_NO,
        .e_notify_core_mask = RDR_NPU,
        .e_reset_core_mask  = RDR_NPU,
        .e_from_core        = RDR_NPU,
        .e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = NPU_S_EXCEPTION,
        .e_exce_subtype     = AICPU_INIT_EXCP,
        .e_upload_flag      = (u32)RDR_UPLOAD_YES,
        .e_from_module      = "NPU",
        .e_desc             = "AICPU_INIT_EXCP",
    },
    {
        .e_modid            = (u32)RDR_EXC_TYPE_AICPU_HEART_BEAT_EXCEPTION,
        .e_modid_end        = (u32)RDR_EXC_TYPE_AICPU_HEART_BEAT_EXCEPTION,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_NO,
        .e_notify_core_mask = RDR_NPU,
        .e_reset_core_mask  = RDR_NPU,
        .e_from_core        = RDR_NPU,
        .e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = NPU_S_EXCEPTION,
        .e_exce_subtype     = AICPU_HEARTBEAT_EXCP,
        .e_upload_flag      = (u32)RDR_UPLOAD_YES,
        .e_from_module      = "NPU",
        .e_desc             = "AICPU_HEARTBEAT_EXCP",
    },
    {
        .e_modid            = (u32)RDR_EXC_TYPE_NPU_POWERUP_FAIL,
        .e_modid_end        = (u32)RDR_EXC_TYPE_NPU_POWERUP_FAIL,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_NO,
        .e_notify_core_mask = RDR_NPU,
        .e_reset_core_mask  = RDR_NPU,
        .e_from_core        = RDR_NPU,
        .e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = NPU_S_EXCEPTION,
        .e_exce_subtype     = POWERUP_FAIL,
        .e_upload_flag      = (u32)RDR_UPLOAD_YES,
        .e_from_module      = "NPU",
        .e_desc             = "POWERUP_FAIL",
    },
    {
        .e_modid            = (u32)RDR_EXC_TYPE_NPU_POWERDOWN_FAIL,
        .e_modid_end        = (u32)RDR_EXC_TYPE_NPU_POWERDOWN_FAIL,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_NO,
        .e_notify_core_mask = RDR_NPU,
        .e_reset_core_mask  = RDR_NPU,
        .e_from_core        = RDR_NPU,
        .e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = NPU_S_EXCEPTION,
        .e_exce_subtype     = POWERDOWN_FAIL,
        .e_upload_flag      = (u32)RDR_UPLOAD_YES,
        .e_from_module      = "NPU",
        .e_desc             = "POWERDOWN_FAIL",
    }
};

/********************************************************************
Description: devdrv_npu_mntn_copy_aicpulog_to_bbox
input: const char *src_addr, unsigned int* offset, unsigned int len
output: NA
return: void
********************************************************************/
static int devdrv_npu_mntn_copy_aicpulog_to_bbox(const char *src_addr, unsigned int len)
{
    unsigned int temp_offset = 0;
    if ((NULL == src_addr) || (0 == len)
        || (g_devdrv_npu_mntn_private->mntn_info.rdr_addr == NULL)) {
        devdrv_drv_err("input parameter is error!\n");
        return -EINVAL;
    }

    temp_offset = g_devdrv_npu_mntn_private->mntn_info.aicpu_add_offset + len;

    /* aicpu alloc size 64K */
    if (temp_offset >= DEVDRV_AICPU_BBOX_MEM_MAX || temp_offset <= 0) {
	devdrv_drv_warn("bbox aicpu buf is full ,copy log to bbox is error! temp_offset=%u.\n", temp_offset);
	g_devdrv_npu_mntn_private->mntn_info.aicpu_add_offset = 0;
	return -ENOMEM ;
    }

    memcpy(((char*)g_devdrv_npu_mntn_private->mntn_info.rdr_addr +
        DEVDRV_NPU_BBOX_MEM_MAX +
        g_devdrv_npu_mntn_private->mntn_info.aicpu_add_offset),
        src_addr, len);
    g_devdrv_npu_mntn_private->mntn_info.aicpu_add_offset = temp_offset;

    return 0;
}


/********************************************************************
Description: devdrv_npu_mntn_copy_reg_to_bbox
input: const char *src_addr, unsigned int* offset, unsigned int len
output: NA
return: void
********************************************************************/
static int devdrv_npu_mntn_copy_reg_to_bbox(const char *src_addr, unsigned int len)
{
    unsigned int temp_offset = 0;

    if ((NULL == src_addr) || (0 == len)
        || (g_devdrv_npu_mntn_private->mntn_info.rdr_addr == NULL)) {
        devdrv_drv_err("input parameter is error!\n");
        return -EINVAL;
    }

    temp_offset = g_devdrv_npu_mntn_private->mntn_info.bbox_addr_offset + len;

    /* npu_bbox alloc size 256K */
    if (temp_offset >= DEVDRV_NPU_BBOX_MEM_MAX  - MODULE_INFO_OFFSET - sizeof(bbox_exception_info_t)
         || temp_offset <= 0) {
	devdrv_drv_warn("bbox buf is full ,copy log to bbox is error! temp_offset=%u.\n", temp_offset);
	g_devdrv_npu_mntn_private->mntn_info.bbox_addr_offset = 0;
	return -ENOMEM ;
    }

    memcpy(((char*)g_devdrv_npu_mntn_private->mntn_info.rdr_addr + g_devdrv_npu_mntn_private->mntn_info.bbox_addr_offset), src_addr, len);
    g_devdrv_npu_mntn_private->mntn_info.bbox_addr_offset = temp_offset;

    return 0;
}


/********************************************************************
Description: devdrv_npu_mntn_write_peri_reg_info
input:	char *file_path
output:	NA
return:	void
********************************************************************/
static void devdrv_npu_mntn_write_peri_reg_info(unsigned int core_id)
{
    char log_buf[DEVDRV_NPU_BUF_LEN_MAX + 1] = {0};
    struct devdrv_npu_peri_reg_s *peri_reg = &g_devdrv_npu_mntn_private->reg_info[core_id].peri_reg;

    snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : peri_stat=%x, ppll_select=%x, power_stat=%x, power_ack=%x, reset_stat=%x, perclken=%x, perstat=%x.\r\n",
			 peri_reg->peri_stat,
			 peri_reg->ppll_select,
			 peri_reg->power_stat,
			 peri_reg->power_ack,
			 peri_reg->reset_stat,
			 peri_reg->perclken0,
			 peri_reg->perstat0);

    devdrv_npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));
    return;
}


/********************************************************************
Description: devdrv_npu_mntn_write_mstr_reg_info
input:	char *file_path
output:	NA
return:	void
********************************************************************/
static void devdrv_npu_mntn_write_mstr_reg_info(unsigned int core_id)
{
    char log_buf[DEVDRV_NPU_BUF_LEN_MAX + 1] = {0};
    struct devdrv_npu_mstr_reg_s *mstr_reg = &g_devdrv_npu_mntn_private->reg_info[core_id].mstr_reg;

    snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : RD_BITMAP=%x, WR_BITMAP=%x, rd_cmd_total_cnt[0-3]={%x, %x, %x}, wr_cmd_total_cnt=%x\n",
		mstr_reg->rd_bitmap,
		mstr_reg->wr_bitmap,
		mstr_reg->rd_cmd_total_cnt0,
		mstr_reg->rd_cmd_total_cnt0,
		mstr_reg->rd_cmd_total_cnt2,
		mstr_reg->wr_cmd_total_cnt);

    devdrv_npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

    return;
}


/********************************************************************
Description: devdrv_npu_mntn_write_reg_log
input:	void
output:	NA
return:	void
********************************************************************/
static void devdrv_npu_mntn_write_reg_log(unsigned int core_id)
{
    char log_buf[DEVDRV_NPU_BUF_LEN_MAX + 1] = {0};
    unsigned int modid = 0;

    memset((char*)g_devdrv_npu_mntn_private->mntn_info.rdr_addr, 0, DEVDRV_NPU_BBOX_MEM_MAX);
    snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "\r\nNPU exception info : core_id = 0x%x\r\n", core_id);
    devdrv_npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

    snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : exception_code =0x%x\r\n", 
        g_devdrv_npu_mntn_private->mntn_info.dump_info.modid);
    devdrv_npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));
    snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : interrupt_status=0x%x\r\n", 
        g_devdrv_npu_mntn_private->exc_info[core_id].interrupt_status);
    devdrv_npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));
    snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : ip=0x%x, ret=0x%x\r\n",
		g_devdrv_npu_mntn_private->exc_info[core_id].target_ip,
        g_devdrv_npu_mntn_private->exc_info[core_id].result);
    devdrv_npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

    modid = g_devdrv_npu_mntn_private->mntn_info.dump_info.modid;
    switch (modid) {
        case (unsigned int)EXC_TYPE_TS_AICORE_EXCEPTION:
            snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : aicore exception.\r\n");
            break;
        case (unsigned int)EXC_TYPE_TS_AICORE_TIMEOUT:
            snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : aicore timeout.\r\n");
            break;
        case (unsigned int)EXC_TYPE_TS_SDMA_EXCEPTION:
            snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : sdma exception.\r\n");
            break;
        case (unsigned int)RDR_EXC_TYPE_TS_RUNNING_EXCEPTION:
            snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : ts running exception.\r\n");
            break;
        case (unsigned int)RDR_EXC_TYPE_TS_RUNNING_TIMEOUT:
            snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : ts running timeout.\r\n");
            break;
        case (unsigned int)RDR_EXC_TYPE_OS_EXCEPTION:
            snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : OS running exception.\r\n");
            break;
        case (unsigned int)RDR_EXC_TYPE_TS_INIT_EXCEPTION:
            snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : ts init exception.\r\n");
            break;
        case (unsigned int)RDR_EXC_TYPE_AICPU_INIT_EXCEPTION:
            snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : aicpu init exception.\r\n");
            break;
        case (unsigned int)RDR_EXC_TYPE_AICPU_HEART_BEAT_EXCEPTION:
            snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : aicpu heart beat exception.\r\n");
            break;
        default:
            snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : no exception.\r\n");
            break;
    }
    devdrv_npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));
    devdrv_npu_mntn_write_peri_reg_info(core_id);
    devdrv_npu_mntn_write_mstr_reg_info(core_id);
    return;
}

static void devdrv_npu_mntn_dump_aicpulog_work(void)
{
    uint32_t log_len = 0;
    struct exc_module_info_s *exception_info = NULL;
    char log_buf[DEVDRV_NPU_BUF_LEN_MAX + 1] = {0};
    vir_addr_t temp_bbox_aicpulog_addr_vir = g_bbox_aicpulog_addr_vir;

    exception_info = (struct exc_module_info_s *)(uintptr_t)(temp_bbox_aicpulog_addr_vir);
    log_len = exception_info->e_info_offset + exception_info->e_info_len;

    if (log_len > (uint32_t)AICPU_BUFF_LEN || log_len <= 0) {
            devdrv_drv_err("log_len is more than the max len, log_len = %u.\n", log_len);
            return ;
    }

    if (log_len > (uint32_t)DEVDRV_NPU_BUF_LEN_MAX) {
        do {
            memcpy((void *)log_buf, (void *)((uintptr_t)temp_bbox_aicpulog_addr_vir), DEVDRV_NPU_BUF_LEN_MAX);
            devdrv_npu_mntn_copy_aicpulog_to_bbox(log_buf, (unsigned int)DEVDRV_NPU_BUF_LEN_MAX);
            log_len -= DEVDRV_NPU_BUF_LEN_MAX;
            temp_bbox_aicpulog_addr_vir += DEVDRV_NPU_BUF_LEN_MAX;
        }
        while (log_len > DEVDRV_NPU_BUF_LEN_MAX);
    }
    memcpy((void *)log_buf, (void *)((uintptr_t)temp_bbox_aicpulog_addr_vir), log_len);

    devdrv_npu_mntn_copy_aicpulog_to_bbox(log_buf, log_len);
}

/********************************************************************
Description: devdrv_npu_mntn_dump_tscpu_work
input: struct work_struct *work
output: NA
return: NA
********************************************************************/
static void devdrv_npu_mntn_dump_tscpu_work(void)
{
    bbox_exception_info_t *exception_info = NULL;
    char log_buf[DEVDRV_NPU_BUF_LEN_MAX + 1] = {0};
    uint32_t log_len = 0;
    vir_addr_t temp_bbox_tslog_addr_vir = g_bbox_tslog_addr_vir;

    devdrv_npu_mntn_write_reg_log(g_devdrv_npu_mntn_private->core_id);

    exception_info = (bbox_exception_info_t *)(uintptr_t)(g_bbox_addr_vir + MODULE_INFO_OFFSET);
    log_len = exception_info->log_len;

    if (log_len > (uint32_t)(DEVDRV_NPU_BBOX_MEM_MAX - MODULE_INFO_OFFSET - sizeof(bbox_exception_info_t))
        || log_len <= 0) {
            devdrv_drv_err("log_len is more than the max len or log_len is 0, log_len = %u.\n", log_len);
            return ;
    }

    if (log_len > (uint32_t)DEVDRV_NPU_BUF_LEN_MAX) {
        do {
            memcpy((void *)log_buf, (void *)((uintptr_t)temp_bbox_tslog_addr_vir), DEVDRV_NPU_BUF_LEN_MAX);
            devdrv_npu_mntn_copy_reg_to_bbox(log_buf, (unsigned int)DEVDRV_NPU_BUF_LEN_MAX);
            log_len -= DEVDRV_NPU_BUF_LEN_MAX;
            temp_bbox_tslog_addr_vir += DEVDRV_NPU_BUF_LEN_MAX;
        } while (log_len > DEVDRV_NPU_BUF_LEN_MAX);
    }

    memcpy((void *)log_buf, (void *)((uintptr_t)temp_bbox_tslog_addr_vir), log_len);
    devdrv_npu_mntn_copy_reg_to_bbox(log_buf, log_len);

    if (g_devdrv_npu_mntn_private->mntn_info.dump_info.cb != NULL)
    {
        g_devdrv_npu_mntn_private->mntn_info.dump_info.cb(
            g_devdrv_npu_mntn_private->mntn_info.dump_info.modid,
            g_devdrv_npu_mntn_private->mntn_info.dump_info.coreid);
    }

    return;
}

/********************************************************************
Description: devdrv_npu_mntn_dump_work
input: struct work_struct *work
output: NA
return: NA
********************************************************************/
static void devdrv_npu_mntn_dump_work(struct work_struct *work __attribute__((__unused__)))
{
    /* copy ts log to rdr address */
    devdrv_npu_mntn_dump_tscpu_work();

    /* copy aicpu log to rdr address */
    devdrv_npu_mntn_dump_aicpulog_work();
}


/********************************************************************
Description : npu_rdr_resource_init
input : void
output : NA
return : int:  0 is OK.
                   other value is Error No.
********************************************************************/
int npu_rdr_resource_init(void)
{
    g_devdrv_npu_mntn_private = kzalloc(sizeof(struct devdrv_npu_mntn_private_s), GFP_KERNEL);
    if (g_devdrv_npu_mntn_private == NULL)
    {
        devdrv_drv_err("failed to allocate memory!");
        return -ENOMEM;
    }

    g_devdrv_npu_mntn_private->rdr_wq = create_singlethread_workqueue("npu_mntn_rdr_wq");
    if (g_devdrv_npu_mntn_private->rdr_wq == NULL)
    {
        devdrv_drv_err("create_singlethread_workqueue is failed!");
        kfree(g_devdrv_npu_mntn_private);
        g_devdrv_npu_mntn_private = NULL;
        return -EINTR;
    }

    INIT_WORK(&g_devdrv_npu_mntn_private->dump_work, devdrv_npu_mntn_dump_work);

    return 0;
}


/********************************************************************
Description : npu_rdr_register_exception
input : void
output : NA
rdr_register_exception : ret == 0 is fail.
                                   ret > 0 is success.
********************************************************************/
int npu_rdr_register_exception(void)
{
    int ret;
    unsigned int  size;
    unsigned long index;

    size = sizeof(devdrv_npu_rdr_excetption_info)/sizeof(struct rdr_exception_info_s);
    for (index = 0; index < size; index++)
    {
        /* error return 0, ok return modid */
        ret = rdr_register_exception(&devdrv_npu_rdr_excetption_info[index]);
        if (ret == 0)
        {
            devdrv_drv_err("rdr_register_exception is failed! index = %ld, ret = %d", index, ret);
            return -EINTR;
        }
    }
    return 0;
}

/********************************************************************
Description : npu_rdr_unregister_exception
input : void
output : NA
rdr_unregister_exception : ret < 0 is fail.
                                       ret >= 0 success.
********************************************************************/
int npu_rdr_unregister_exception(void)
{
    int ret;
    unsigned int  size;
    unsigned long index;

    size = sizeof(devdrv_npu_rdr_excetption_info)/sizeof(struct rdr_exception_info_s);
    for (index = 0; index < size; index++)
    {
        /* ret < 0 is fail, ret >= 0 is success */
        ret = rdr_unregister_exception(devdrv_npu_rdr_excetption_info[index].e_modid);
        if (ret < 0)
        {
            devdrv_drv_err("rdr_unregister_exception is failed! index = %ld, ret = %d.", index, ret);
            return -EINTR;
        }
    }
    return 0;
}

/********************************************************************
Description: devdrv_npu_mntn_rdr_dump
input:modid: module id
        etype:exception type
        coreid: core id
        pathname: log path
        pfn_cb: callback function
output: NA
return: NA
********************************************************************/
static void devdrv_npu_mntn_rdr_dump(u32 modid, u32 etype, u64 coreid, char *pathname, pfn_cb_dump_done pfn_cb)
{
    if (pathname == NULL)
    {
        devdrv_drv_err("pathname is empty!");
        return;
    }
    g_devdrv_npu_mntn_private->mntn_info.dump_info.modid = modid;
    g_devdrv_npu_mntn_private->mntn_info.dump_info.coreid = coreid;
    g_devdrv_npu_mntn_private->mntn_info.dump_info.pathname = pathname;
    g_devdrv_npu_mntn_private->mntn_info.dump_info.cb = pfn_cb;
    g_devdrv_npu_mntn_private->mntn_info.bbox_addr_offset = 0;
    queue_work(g_devdrv_npu_mntn_private->rdr_wq, &g_devdrv_npu_mntn_private->dump_work);
    return;
}


/********************************************************************
Description: devdrv_npu_mntn_rdr_reset
input: modid:module id
        etype:exception type
        coreid:core id
output: NA
return: NA
********************************************************************/
static void devdrv_npu_mntn_rdr_reset(u32 modid, u32 etype, u64 coreid)
{
    return;
}


/********************************************************************
Description : npu_rdr_register_core
input : void
output : NA
rdr_register_module_ops : ret <0 is fail.
                                       ret >= 0 is success.
********************************************************************/
int npu_rdr_register_core(void)
{
    int ret;
    struct rdr_module_ops_pub s_soc_ops;

    s_soc_ops.ops_dump = devdrv_npu_mntn_rdr_dump;
    s_soc_ops.ops_reset = devdrv_npu_mntn_rdr_reset;
    /* register npu core dump and reset function */
    ret = rdr_register_module_ops((u64)RDR_NPU, &s_soc_ops, &g_devdrv_npu_mntn_private->mntn_info.npu_ret_info);
    if (ret < 0)
    {
        devdrv_drv_err("rdr_register_module_ops is failed! ret = 0x%08x", ret);
    }

    return ret;
}

/********************************************************************
Description : npu_rdr_unregister_core
input : void
output : NA
rdr_unregister_module_ops : ret < 0 is fail.
                                           ret >= 0 is success.
********************************************************************/
int npu_rdr_unregister_core(void)
{
    int ret;

    /* unregister npu core dump and reset function */
    ret = rdr_unregister_module_ops((u64)RDR_NPU);
    if (ret < 0)
    {
        devdrv_drv_err("rdr_unregister_module_ops is failed! ret = 0x%08x", ret);
    }

    return ret;
}

/********************************************************************
Description : npu_rdr_addr_map
input : void
output : NA
return : int:  0 is OK.
                   other value is Error No.
********************************************************************/
int npu_rdr_addr_map(void)
{
    g_devdrv_npu_mntn_private->mntn_info.rdr_addr = hisi_bbox_map((phys_addr_t)g_devdrv_npu_mntn_private->mntn_info.npu_ret_info.log_addr, 
        g_devdrv_npu_mntn_private->mntn_info.npu_ret_info.log_len);
    if (g_devdrv_npu_mntn_private->mntn_info.rdr_addr == NULL)
    {
        devdrv_drv_err("hisi_bbox_map is failed!");
        return -EFAULT;
    }

    return 0;
}

/********************************************************************
Description : npu_rdr_addr_unmap
input : void
output : NA
return : int:  0 is OK.
                   other value is Error No.
********************************************************************/
int npu_rdr_addr_unmap(void)
{
    hisi_bbox_unmap(g_devdrv_npu_mntn_private->mntn_info.rdr_addr);
    g_devdrv_npu_mntn_private->mntn_info.rdr_addr = NULL;
    return 0;
}

/********************************************************************
Description : npu_blackbox_addr_init
input : void
output : NA
return : int:  0 is OK.
                   other value is Error No.
********************************************************************/
int npu_blackbox_addr_init(void)
{
    struct devdrv_platform_info *plat_info = NULL;
    struct devdrv_dfx_desc *dfx_desc = NULL;
    struct devdrv_mem_desc *aicpu_men_desc = NULL;

    plat_info = devdrv_plat_get_info();
    if (plat_info == NULL) {
        devdrv_drv_err("devdrv_plat_get_info error.\n");
        return -1;
    }

    dfx_desc = &DEVDRV_PLAT_GET_DFX_DESC(plat_info, DEVDRV_DFX_DEV_BLACKBOX);
    if (dfx_desc == NULL) {
        devdrv_drv_err("get dtsi failed.\n");
        return -1;
    }
    aicpu_men_desc = DEVDRV_PLAT_GET_AIFW_BUF(plat_info);
    if (aicpu_men_desc == NULL) {
        devdrv_drv_err("get dtsi failed.\n");
        return -1;
    }

    //g_bbox_addr_vir = (unsigned long long)ioremap_nocache(DEVDRV_PLAT_GET_RESMEM_DESC(plat_info, 6).base, bbox_addr_len);
    g_bbox_addr_vir = (unsigned long long)(uintptr_t)ioremap_wc(dfx_desc->bufs->base, BBOX_ADDR_LEN);
    if (g_bbox_addr_vir == 0) {
        devdrv_drv_err("ioremap_wc failed.\n");
        return -1;
    }

    g_bbox_aicpulog_addr_vir = (unsigned long long)(uintptr_t)ioremap_wc(
        aicpu_men_desc->base+AICPU_BUFF_OFFSET, AICPU_BUFF_LEN);
    if (g_bbox_aicpulog_addr_vir == 0) {
        devdrv_drv_err("ioremap_wc failed.\n");
        return -1;
    }

    g_bbox_tslog_addr_vir = g_bbox_addr_vir + MODULE_INFO_OFFSET + sizeof(bbox_exception_info_t);
    memset((void *)(uintptr_t)g_bbox_addr_vir, 0, BBOX_ADDR_LEN);
    memset((void *)(uintptr_t)g_bbox_aicpulog_addr_vir, 0, AICPU_BUFF_LEN);

    return 0;
}

/********************************************************************
Description : npu_black_box_init
input : void
output : NA
return : int:  0 is OK.
                   other value is Error No.
********************************************************************/
int npu_black_box_init(void)
{
    int ret;
    devdrv_drv_info("npu_black_box_init Enter.\n");

    ret = npu_rdr_resource_init();
    if (0 > ret)
    {
        devdrv_drv_err("npu_rdr_resource_init is faild ! ret = %d", ret);
        return ret;
    }

    /* register NPU exception */
    ret = npu_rdr_register_exception();
    if (0 > ret)
    {
        devdrv_drv_err("npu_rdr_register_exception is faild! ret = %d", ret);
        goto error;
    }

    /* register NPU dump and reset function */
    ret = npu_rdr_register_core();
    if (0 > ret)
    {
        devdrv_drv_err("npu_rdr_register_core is failed! ret = %d", ret);
        npu_rdr_unregister_exception();
        goto error;
    }

    ret = npu_rdr_addr_map();
    if (0 > ret)
    {
        devdrv_drv_err("npu_rdr_addr_map is failed! ret = %d", ret);
        npu_rdr_unregister_core();
        npu_rdr_unregister_exception();
        goto error;
    }

    ret = npu_blackbox_addr_init();
    if (0 > ret)
    {
        npu_rdr_addr_unmap();
        npu_rdr_unregister_core();
        npu_rdr_unregister_exception();
        devdrv_drv_err("npu_blackbox_addr_init is failed ! ret = %d", ret);
        goto error;
    }

    /* Register AI dsm client */
    //DSM_AI_REGISTER();

    return 0;

error:
    if (NULL != g_devdrv_npu_mntn_private) {
        destroy_workqueue(g_devdrv_npu_mntn_private->rdr_wq);
        kfree(g_devdrv_npu_mntn_private);
        g_devdrv_npu_mntn_private = NULL;
    }
    return ret;
}

/********************************************************************
Description : npu_black_box_exit
input : void
output : NA
return : int:  0 is OK.
                   other value is Error No.
********************************************************************/
int npu_black_box_exit(void)
{
    int ret = 0;
    iounmap((void *)((uintptr_t)g_bbox_addr_vir));

    (void)npu_rdr_addr_unmap();

    ret = npu_rdr_unregister_core();
    if (0 > ret)
    {
        devdrv_drv_err("npu_rdr_unregister_core is faild! ret = %d", ret);
    }

    ret = npu_rdr_unregister_exception();
    if (0 > ret)
    {
        devdrv_drv_err("npu_rdr_unregister_exception is faild! ret = %d", ret);
    }

    if (NULL != g_devdrv_npu_mntn_private) {
        destroy_workqueue(g_devdrv_npu_mntn_private->rdr_wq);
        kfree(g_devdrv_npu_mntn_private);
        g_devdrv_npu_mntn_private = NULL;
    }

    return 0;
}
