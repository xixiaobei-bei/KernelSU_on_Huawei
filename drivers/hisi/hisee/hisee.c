/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: implement hisee common function
 * Create: 2020-02-17
 */
#include "hisee.h"
#include <asm/compiler.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include <vendor_rpmb.h>
#include <linux/atomic.h>
#include <linux/clk.h>
#include <linux/compiler.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/fcntl.h>
#include <linux/fd.h>
#include <linux/fs.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/hisi/ipc_msg.h>
#include <linux/hisi/partition_ap_kernel.h>
#include <linux/hisi/rpmb.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/of_reserved_mem.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/timer.h>
#include <linux/tty.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <securec.h>
#include <soc_acpu_baseaddr_interface.h>
#include <soc_sctrl_interface.h>
#ifdef CONFIG_HISEE_MNTN
#include "hisee_mntn.h"
#endif
#include "flash_hisee_otp.h"
#include "hisee_chip_test.h"
#include "hisee_fs.h"
#include "hisee_power.h"
#include "hisee_upgrade.h"

#define HISEE_EFUSE_GROUP_BIT_SIZE    32
#define HISEE_PWR_ERRNO_PRINT_SIZE    8
#define HISEE_EFUSE_GROUP_NUM         2
#define HISEE_EFUSE_LENGTH            8
#define HISEE_EFUSE_TIMEOUT           1000
#define HISEE_EFUSE_MASK              1
#define HISEE_SM_EFUSE_GROUP  (g_hisee_sm_efuse_pos / HISEE_EFUSE_GROUP_BIT_SIZE)
#define HISEE_SM_EFUSE_OFFSET (g_hisee_sm_efuse_pos % HISEE_EFUSE_GROUP_BIT_SIZE)
#define HISEE_HIBENCH_EFUSE_GROUP  (g_hisee_hibench_efuse_pos / HISEE_EFUSE_GROUP_BIT_SIZE)
#define HISEE_HIBENCH_EFUSE_OFFSET (g_hisee_hibench_efuse_pos % HISEE_EFUSE_GROUP_BIT_SIZE)

#define HISEE_BYPASS                 (-0x1F25)

/* local retry count */
#define HISEE_PARTITION_RETRY_COUNT   10
#define HISEE_LPMCU_SEND_RETRY_COUNT  3

/* the global data used by hisee module in kernel driver */
static struct hisee_module_data g_hisee_data;
/* save global error no used by hisee module in kernel driver */
static atomic_t g_hisee_errno;
static bool g_hisee_is_fpga;
static struct mutex g_hisee_apdu_mutex;
static struct mutex g_hisee_otp_mutex;

static unsigned int g_platform_id;
static unsigned int g_cos_image_upgrade_done = HISEE_FALSE;

static bool g_hisee_api_bypassed;
static bool g_hisee_file_ioctl_ready;

/*
 * the bit position for SM flag recorded in efuse which is started from 0.
 * default value is invalid
 */
static u32 g_hisee_sm_efuse_pos = HISEE_SM_EFUSE_POS_INVALID;

/*
 * the bit position for hibench flag recorded in efuse which is started from 0.
 * default value is invalid
 */
static u32 g_hisee_hibench_efuse_pos = HISEE_SM_EFUSE_POS_INVALID;
/* cos upgrade procedure may cost more time on non-asic platform */
static u32 g_hisee_cos_upgrade_time = HISEE_ATF_COS_TIMEOUT;

/* hisee autoteset interface entryfor cmd "hisee_channel_test" */
static int hisee_auto_test_func(const void *buf, int para);

static struct hisee_errcode_item_des g_errcode_items_des[] = {
	{ HISEE_OK, "no error\n" },
	{ HISEE_ERROR, "general error\n" },
	{ HISEE_NO_RESOURCES, "general no resources\n" },
	{ HISEE_INVALID_PARAMS, "general invalid parameters\n" },
	{ HISEE_CMA_DEVICE_INIT_ERROR, "cma device init error\n" },
	{ HISEE_IOCTL_NODE_CREATE_ERROR, "ioctl node create error\n" },
	{ HISEE_POWER_NODE_CREATE_ERROR, "power node create error\n" },
	{ HISEE_THREAD_CREATE_ERROR, "thread create error\n" },
	{ HISEE_RPMB_MODULE_INIT_ERROR, "rpmb module init error\n" },
	{ HISEE_BULK_CLK_INIT_ERROR, "bulk clk dts node init error\n" },
	{ HISEE_BULK_CLK_ENABLE_ERROR, "bulk clk error\n" },

	{ HISEE_RPMB_KEY_WRITE_ERROR, "write rpmb key fail\n" },
	{ HISEE_RPMB_KEY_READ_ERROR, "read rpmb key fail\n" },
	{ HISEE_RPMB_KEY_UNREADY_ERROR, "rpmb key not ready error\n" },
	{ HISEE_COS_VERIFICATITON_ERROR, "cos img verify fail\n" },
	{ HISEE_IMG_PARTITION_MAGIC_ERROR, "img partition magic error\n" },
	{ HISEE_IMG_PARTITION_FILES_ERROR, "img partition files error\n" },
	{ HISEE_IMG_SUB_FILE_NAME_ERROR, "sub image invalid error\n" },
	{ HISEE_SUB_FILE_SIZE_CHECK_ERROR, "sub file size error\n" },
	{ HISEE_SUB_FILE_OFFSET_CHECK_ERROR, "sub file offset error\n" },
	{ HISEE_IMG_SUB_FILE_ABSENT_ERROR, "sub image absent error\n" },
	{ HISEE_FS_SUB_FILE_ABSENT_ERROR, "sub fs file absent error\n" },

	{ HISEE_OPEN_FILE_ERROR, "open file error\n" },
	{ HISEE_READ_FILE_ERROR, "read file error\n" },
	{ HISEE_WRITE_FILE_ERROR, "write file error\n" },
	{ HISEE_CLOSE_FILE_ERROR, "close file error\n" },
	{ HISEE_LSEEK_FILE_ERROR, "seek file error\n" },
	{ HISEE_OUTOF_RANGE_FILE_ERROR, "out of file threshold error\n" },

	{ HISEE_FS_MALLOC_ERROR, "fs partition malloc error\n" },
	{ HISEE_FS_PATH_ABSENT_ERROR, "fs partition absent error\n" },
	{ HISEE_FS_OPEN_PATH_ERROR, "fs partition open error\n" },
	{ HISEE_FS_COUNT_FILES_ERROR, "fs partition calculate files error\n" },
	{ HISEE_FS_PATH_LONG_ERROR, "sub fs file path too long error\n" },
	{ HISEE_FS_READ_FILE_ERROR, "sub fs file read error\n" },

	{ HISEE_POWERCTRL_TIMEOUT_ERROR, "hisee powerctrl timeout error\n" },
	{ HISEE_POWERCTRL_NOTIFY_ERROR, "hisee powerctrl notify error\n" },
	{ HISEE_POWERCTRL_RETRY_FAILURE_ERROR, "hisee powerctrl retry failure error\n" },
	{ HISEE_POWERCTRL_FLOW_ERROR, "hisee powerctrl flow error\n" },

	{ HISEE_FIRST_SMC_CMD_ERROR, "the fisrt step in smc transcation error\n" },
	{ HISEE_SMC_CMD_TIMEOUT_ERROR, "the smc transcation timeout error\n" },
	{ HISEE_SMC_CMD_PROCESS_ERROR, "the smc transcation failure error\n" },
	{ HISEE_GET_HISEE_VALUE_ERROR, "get hisee lcs mode error\n" },
	{ HISEE_SET_HISEE_VALUE_ERROR, "set hisee lcs mode error\n" },
	{ HISEE_SET_HISEE_STATE_ERROR, "set hisee state error\n" },

	{ HISEE_WAIT_READY_TIMEOUT, "wait hisee ready timeout\n" },

	{ HISEE_ENCOS_HEAD_INIT_ERROR, "hisee encos head init error\n" },
	{ HISEE_COS_IMG_ID_ERROR, "the input cosid error\n" },
	{ HISEE_ENCOS_PARTITION_MAGIC_ERROR, "hisee encos magic num error\n" },
	{ HISEE_ENCOS_PARTITION_FILES_ERROR, "hisee encos files error\n" },
	{ HISEE_ENCOS_PARTITION_SIZE_ERROR, "hisee encos file size error\n" },
	{ HISEE_ENCOS_SUBFILE_SIZE_CHECK_ERROR, "hisee encos subfile size error\n" },
	{ HISEE_ENCOS_OPEN_FILE_ERROR, "hisee encos open file error\n" },
	{ HISEE_ENCOS_FIND_PTN_ERROR, "hisee encos find ptn error\n" },
	{ HISEE_ENCOS_LSEEK_FILE_ERROR, "hisee encos lseek error\n" },
	{ HISEE_ENCOS_WRITE_FILE_ERROR, "hisee encos write file error\n" },
	{ HISEE_ENCOS_READ_FILE_ERROR, "hisee encos read file error\n" },
	{ HISEE_ENCOS_ACCESS_FILE_ERROR, "hisee encos access file error\n" },
	{ HISEE_ENCOS_CHECK_HEAD_ERROR, "hisee encos check file head error\n" },
	{ HISEE_ENCOS_NAME_NULL_ERROR, "hisee encos file name is null\n" },

	{ HISEE_OLD_COS_IMAGE_ERROR, "old cos image error\n" }
};

#define MANAFACTOR_CMD_INDEX 2

static struct hisee_driver_function g_hisee_atf_function_list[] = {
	{ "cos_image_upgrade", cos_image_upgrade_func },
	{ "hisee_channel_test", hisee_auto_test_func },
	{ "hisee_parallel_factory_action", hisee_parallel_manufacture_func },

	{ "hisee_factory_check", hisee_factory_check_func },
#if defined(CONFIG_SMX_PROCESS) || defined(CONFIG_HISEE_AT_SMX)
	{ "hisee_get_smx", hisee_get_smx_func },
#endif
	{ NULL, NULL },
};

static struct hisee_driver_function g_hisee_lpm3_function_list[] = {
	{ "hisee_poweron_upgrade", hisee_poweron_upgrade_func },
	{ "hisee_poweron_timeout", hisee_poweron_timeout_func },
	{ "hisee_poweron", hisee_poweron_booting_func },
	{ "hisee_poweroff", hisee_poweroff_func },
	{ NULL, NULL },
};

u32 get_cos_upgrade_time(void)
{
	return g_hisee_cos_upgrade_time;
}

/*
 * @brief      : check current hisee platform whether is FPGA
 * @return     : ::bool, true indicates FPGA, other indicates non-FPGA
 */
bool check_hisee_is_fpga(void)
{
	return g_hisee_is_fpga ? true : false;
}

/*
 * @brief      :get the pointer of g_hisee_data variable
 * @return     : ::struct hisee_module_data*
 */
struct hisee_module_data *get_hisee_data_ptr(void)
{
	return &g_hisee_data;
}

/*
 * @brief      : set err_code to g_hisee_errno, then return with err_code
 * @return     : ::int, return error code
 */
int set_errno_then_exit(int err_code)
{
	atomic_set(&g_hisee_errno, err_code);
	return err_code;
}

int get_hisee_errno(void)
{
	return atomic_read(&g_hisee_errno);
}

void set_hisee_errno(int err_code)
{
	atomic_set(&g_hisee_errno, err_code);
}

/*
 * @brief      : hisee autoteset interface entry for cmd "hisee_channel_test"
 * @param[in]  : buf, content string buffer
 * @param[in]  : para, parameters
 * @return     : ::int, 0 on success, other value on failure
 */
int hisee_auto_test_func(const void *buf, int para)
{
	return HISEE_OK;
}

noinline int atfd_hisee_smc(u64 _function_id, u64 _arg0, u64 _arg1, u64 _arg2)
{
	register u64 function_id asm("x0") = _function_id;
	register u64 arg0 asm("x1") = _arg0;
	register u64 arg1 asm("x2") = _arg1;
	register u64 arg2 asm("x3") = _arg2;

	asm volatile(
	    __asmeq("%0", "x0")
	    __asmeq("%1", "x1")
	    __asmeq("%2", "x2")
	    __asmeq("%3", "x3")
	    "smc    #0\n"
	    : "+r" (function_id)
	    : "r" (arg0), "r" (arg1), "r" (arg2));

	return (int)function_id;
}

int get_hisee_lcs_mode(unsigned int *mode)
{
	int ret;
	u32 hisee_value[HISEE_EFUSE_GROUP_NUM] = {0};
	u32 value;

	if(!mode) {
		pr_err("%s mode is NULL.\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}
	if (HISEE_SM_EFUSE_GROUP >= HISEE_EFUSE_GROUP_NUM) {
		pr_err("sm_flag_pos invalid\n");
		return set_errno_then_exit(HISEE_GET_HISEE_VALUE_ERROR);
	}

	ret = get_efuse_hisee_value((unsigned char *)hisee_value,
				    HISEE_EFUSE_LENGTH, HISEE_EFUSE_TIMEOUT);
	if (ret != HISEE_OK) {
		pr_err("%s() get_efuse_hisee_value failed,ret=%d\n",
		       __func__, ret);
		return set_errno_then_exit(HISEE_GET_HISEE_VALUE_ERROR);
	}
	value = hisee_value[HISEE_SM_EFUSE_GROUP] >> HISEE_SM_EFUSE_OFFSET;
	value &= HISEE_EFUSE_MASK;
	*mode = value ? HISEE_SM_MODE_MAGIC : HISEE_DM_MODE_MAGIC;

	return ret;
}

int get_hisee_hibench_flag(unsigned int *mode)
{
	int ret;
	u32 hisee_value[HISEE_EFUSE_GROUP_NUM] = {0};

	if (!mode) {
		pr_err("%s mode is NULL.\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}
	if (HISEE_HIBENCH_EFUSE_GROUP >= HISEE_EFUSE_GROUP_NUM) {
		pr_err("hibench_flag_pos invalid\n");
		return set_errno_then_exit(HISEE_GET_HISEE_VALUE_ERROR);
	}

	ret = get_efuse_hisee_value((unsigned char *)hisee_value,
				    HISEE_EFUSE_LENGTH, HISEE_EFUSE_TIMEOUT);
	if (ret != HISEE_OK) {
		pr_err("%s() get_efuse_hisee_value failed,ret=%d\n",
		       __func__, ret);
		return set_errno_then_exit(HISEE_GET_HISEE_VALUE_ERROR);
	}
	*mode = (hisee_value[HISEE_HIBENCH_EFUSE_GROUP] >>
		 HISEE_HIBENCH_EFUSE_OFFSET) & HISEE_EFUSE_MASK;
	return ret;
}

int set_hisee_lcs_sm_efuse(void)
{
	int ret;
	unsigned int hisee_value[HISEE_EFUSE_GROUP_NUM] = {0};

	if (HISEE_SM_EFUSE_GROUP >= HISEE_EFUSE_GROUP_NUM) {
		pr_err("sm_flag_pos invalid\n");
		return set_errno_then_exit(HISEE_SET_HISEE_VALUE_ERROR);
	}

	hisee_value[HISEE_SM_EFUSE_GROUP] = 0x1u << HISEE_SM_EFUSE_OFFSET;
	ret = set_efuse_hisee_value((unsigned char *)hisee_value,
				    HISEE_EFUSE_GROUP_NUM * sizeof(int),
				    DELAY_1000_MS);
	if (ret != HISEE_OK) {
		pr_err("%s() set_efuse_hisee_value failed,ret=%d\n",
		       __func__, ret);
		return set_errno_then_exit(HISEE_SET_HISEE_VALUE_ERROR);
	}
#ifdef CONFIG_HISEE_MNTN
	/* update local variable of lcs mode */
	hisee_mntn_update_local_lcsmode_val();
#endif
	check_and_print_result();
	return ret;
}

#if defined(CONFIG_HUAWEI_DSM) && !defined(CONFIG_HISEE_DISABLE_KEY)
static void record_hisee_log_by_dmd(long dmd_errno, int hisee_errno)
{
	int ret;
	char buff[HISEE_CMD_NAME_LEN] = {0};
	unsigned int i;

	(void)memset_s(buff, HISEE_CMD_NAME_LEN, 0,
		       (size_t)HISEE_CMD_NAME_LEN);
	for (i = 0; i < ARRAY_SIZE(g_errcode_items_des); i++) {
		if (hisee_errno == g_errcode_items_des[i].err_code)
			break;
	}
	if (i == ARRAY_SIZE(g_errcode_items_des)) {
		pr_err("%s(): can't find errcode=%d definition\n",
		       __func__, hisee_errno);
		return;
	}
	/* 8 is errno print format max length */
	ret = snprintf_s(buff, HISEE_CMD_NAME_LEN, (size_t)8,
			 "%d,", hisee_errno);
	if (ret == HISEE_SECLIB_ERROR) {
		pr_err("%s(): snprintf failed\n", __func__);
		return;
	}

	ret = strncat_s(buff, HISEE_CMD_NAME_LEN,
			g_errcode_items_des[i].err_description,
			(size_t)((HISEE_ERROR_DESCRIPTION_MAX - 1) - strlen(buff)));
	if (ret != EOK) {
		pr_err("%s(): strncat failed\n", __func__);
		return;
	}

#ifdef CONFIG_HISEE_MNTN
	ret = hisee_mntn_record_dmd_info(dmd_errno, (const char *)buff);
	if (ret != 0)
		pr_err("%s(): hisee_mntn_record_dmd_info return %d failed\n",
		       __func__, ret);
#endif
}
#endif

/*
 * @brief      : handle hisee request from ATF
 *               if smc_cmd_running is zero, there nothing need
 *               to do; otherwise, do complete() a completion.
 */
void hisee_active(void)
{
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	if (hisee_data_ptr->smc_cmd_running)
		complete(&hisee_data_ptr->smc_completion);
}
EXPORT_SYMBOL(hisee_active);

void set_message_header(struct atf_message_header *header, unsigned int cmd_type)
{
	if (!header) {
		pr_err("%s(): header is NULL\n", __func__);
		return;
	}
	header->ack = 0;
	header->cmd = cmd_type;
	header->test_result_phy = 0;
	header->test_result_size = 0;
}

int send_smc_process(const struct atf_message_header *p_message_header,
		     phys_addr_t phy_addr, unsigned int size,
		     unsigned int timeout, enum se_smc_cmd smc_cmd)
{
	int ret;
	long local_jiffies;
	struct hisee_module_data *hisee_data_ptr = NULL;

	if (!p_message_header) {
		pr_err("%s(): header is NULL\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}
	hisee_data_ptr = get_hisee_data_ptr();
	mutex_lock(&hisee_data_ptr->hisee_mutex);
	hisee_data_ptr->smc_cmd_running = HISEE_TRUE;

	if (smc_cmd != CMD_HISEE_CHANNEL_TEST)
		ret = atfd_hisee_smc((u64)HISEE_FN_MAIN_SERVICE_CMD,
				     (u64)smc_cmd, (u64)phy_addr, (u64)size);
	else
		ret = atfd_hisee_smc((u64)HISEE_FN_CHANNEL_TEST_CMD,
				     (u64)smc_cmd, (u64)phy_addr, (u64)size);

	if (ret != HISEE_OK) {
		pr_err("%s(): atfd_hisee_smc failed, ret=%d\n", __func__, ret);
		hisee_data_ptr->smc_cmd_running = HISEE_FALSE;
		mutex_unlock(&hisee_data_ptr->hisee_mutex);
		return set_errno_then_exit(HISEE_FIRST_SMC_CMD_ERROR);
	}

	if (smc_cmd != CMD_HISEE_CHANNEL_TEST)
		local_jiffies = msecs_to_jiffies(timeout);
	else
		local_jiffies = MAX_SCHEDULE_TIMEOUT;
	ret = wait_for_completion_timeout(&hisee_data_ptr->smc_completion,
					  local_jiffies);
	if (unlikely(ret == -ETIME)) {
		ret = HISEE_SMC_CMD_TIMEOUT_ERROR;
	} else {
		if (!(p_message_header->cmd == smc_cmd &&
		      p_message_header->ack == HISEE_ATF_ACK_SUCCESS))
			ret = HISEE_SMC_CMD_PROCESS_ERROR;
		else
			ret = HISEE_OK;
	}

	pr_err("%s() ret=%d\n", __func__, ret);
	hisee_data_ptr->smc_cmd_running = HISEE_FALSE;
	mutex_unlock(&hisee_data_ptr->hisee_mutex);
	return set_errno_then_exit(ret);
}

/*
 * @brief      : int hisee_get_smx_cfg(unsigned int *p_smx_cfg)
 * @param[out] : p_smx_cfg, get the smx configure value
 */
void hisee_get_smx_cfg(unsigned int *p_smx_cfg)
{
	if (!p_smx_cfg) {
		pr_err("hisee:%s() input param error!\n", __func__);
		return;
	}

	*p_smx_cfg = SMX_PROCESS_1;
}

static int write_apdu_command_func(const char *apdu_buf, unsigned int apdu_len)
{
	struct atf_message_header *p_message_header = NULL;
	int ret;
	unsigned int image_size;
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	mutex_lock(&g_hisee_apdu_mutex);
	if (!hisee_data_ptr->apdu_command_buff_virt)
		hisee_data_ptr->apdu_command_buff_virt =
			(void *)dma_alloc_coherent(hisee_data_ptr->cma_device,
						   SIZE_4K,
						   &hisee_data_ptr->apdu_command_buff_phy,
						   GFP_KERNEL);

	if (!hisee_data_ptr->apdu_command_buff_virt) {
		pr_err("%s(): dma_alloc_coherent failed\n", __func__);
		ret = HISEE_NO_RESOURCES;
		goto end_to_restore;
	}
	(void)memset_s(hisee_data_ptr->apdu_command_buff_virt, SIZE_4K, 0, SIZE_4K);
	p_message_header = (struct atf_message_header *)(uintptr_t)hisee_data_ptr->apdu_command_buff_virt;
	set_message_header(p_message_header, CMD_APDU_RAWDATA);

	apdu_len = (apdu_len > HISEE_APDU_DATA_LEN_MAX) ?
		   HISEE_APDU_DATA_LEN_MAX : apdu_len;
	ret = memcpy_s(hisee_data_ptr->apdu_command_buff_virt + HISEE_ATF_MESSAGE_HEADER_LEN,
		       SIZE_4K - HISEE_ATF_MESSAGE_HEADER_LEN,
		       (void *)apdu_buf, (size_t)apdu_len);
	if (ret != EOK) {
		pr_err("%s(): memcpy1 failed\n", __func__);
		ret = HISEE_SECUREC_ERR;
		goto end_to_restore;
	}
	image_size = HISEE_ATF_MESSAGE_HEADER_LEN + apdu_len;
	p_message_header->test_result_phy =
		(u32)(hisee_data_ptr->apdu_command_buff_phy + SIZE_2K);
	p_message_header->test_result_size = (unsigned int)SIZE_1K;
	ret = send_smc_process(p_message_header,
			       hisee_data_ptr->apdu_command_buff_phy, image_size,
			       HISEE_ATF_GENERAL_TIMEOUT, CMD_APDU_RAWDATA);
	if (ret == HISEE_OK &&
	    p_message_header->test_result_size <= HISEE_APDU_DATA_LEN_MAX) {
		ret = memcpy_s(hisee_data_ptr->apdu_ack.ack_buf,
			       HISEE_APDU_DATA_LEN_MAX + 1,
			       (hisee_data_ptr->apdu_command_buff_virt + SIZE_2K),
			       (size_t)p_message_header->test_result_size);
		if (ret != EOK) {
			pr_err("%s(): memcpy2 failed\n", __func__);
			ret = HISEE_SECUREC_ERR;
			goto end_to_restore;
		}
		hisee_data_ptr->apdu_ack.ack_len = p_message_header->test_result_size;
	} else {
		hisee_data_ptr->apdu_ack.ack_len = 0;
		pr_err("%s(): p_message_header->test_result_size=%x failed\n",
		       __func__, p_message_header->test_result_size);
	}

	check_and_print_result();
end_to_restore:
	mutex_unlock(&g_hisee_apdu_mutex);
	return set_errno_then_exit(ret);
}

static int _apdu_set_key_cmd(void)
{
	int ret;
	/* set key cmd */
	char cmd0[SET_KEY_CMD_0_LEN] = {
				   0xF0, 0x10, 0x00, 0x00,
				   0x10, 0x01, 0x23, 0x45,
				   0x67, 0x89, 0xab, 0xcd,
				   0xef, 0xfe, 0xdc, 0xba,
				   0x98, 0x76, 0x54, 0x32,
				   0x10 };
	char cmd1[SET_KEY_CMD_1_LEN] = { 0xF0, 0xd8, 0x00, 0x00, 0x00 };

	/* send apdu command_0 */
	ret = write_apdu_command_func(cmd0, SET_KEY_CMD_0_LEN);
	if (ret != HISEE_OK) {
		pr_err("%s()  apdu0 failed,ret=%d\n", __func__, ret);
		return ret;
	}
	hisee_mdelay(DELAY_BETWEEN_STEPS);
	/* send apdu command_1 */
	ret = write_apdu_command_func(cmd1, SET_KEY_CMD_1_LEN);
	if (ret != HISEE_OK) {
		pr_err("%s()  apdu1 failed,ret=%d\n", __func__, ret);
		return ret;
	}
	hisee_mdelay(DELAY_BETWEEN_STEPS);

	return ret;
}

static int _apdu_del_applet_cmd(void)
{
	int ret;
	/* delete test applet cmd */
	unsigned char cmd0[DELETE_APPLET_CMD_0_LEN] = {
			0x00, 0xa4, 0x04, 0x00, 0x00 };
#ifdef CONFIG_HISEE_APPLET_APDU_TEST_OPTIMIZATION
	unsigned char cmd1[DELETE_APPLET_CMD_1_LEN] = {
			0x80, 0xe4, 0x00, 0x80, 0x08,
			0x4f, 0x06, 0x68, 0x69, 0x73,
			0x69, 0x74, 0x61 };
#else
	unsigned char cmd1[DELETE_APPLET_CMD_1_LEN] = {
			0x80, 0xe4, 0x00, 0x80, 0x07,
			0x4f, 0x05, 0x12, 0x34, 0x56,
			0x78, 0x90 };
#endif

	/* send apdu command_0 */
	ret = write_apdu_command_func(cmd0, DELETE_APPLET_CMD_0_LEN);
	if (ret != HISEE_OK) {
		pr_err("%s()  apdu0 failed,ret=%d\n", __func__, ret);
		return ret;
	}
	hisee_mdelay(DELAY_BETWEEN_STEPS);
	/* send apdu command_1 */
	ret = write_apdu_command_func(cmd1, DELETE_APPLET_CMD_1_LEN);
	if (ret != HISEE_OK) {
		pr_err("%s()  apdu1 failed,ret=%d\n", __func__, ret);
		return ret;
	}
	hisee_mdelay(DELAY_BETWEEN_STEPS);

	return ret;
}

int send_apdu_cmd(int type)
{
	int ret;

	if (type == HISEE_SET_KEY)
		ret = _apdu_set_key_cmd();
	else if (type == HISEE_DEL_TEST_APPLET)
		ret = _apdu_del_applet_cmd();
	else
		ret = HISEE_INVALID_PARAMS;

	return ret;
}

static int load_cosimg_appdata_ddr(void)
{
	char *buff_virt = NULL;
	unsigned int i;
	unsigned int rpmb_cos = HISEE_MIN_RPMB_COS_NUMBER;
	phys_addr_t buff_phy = 0;
	struct atf_message_header *p_message_header = NULL;
	int ret = HISEE_OK;
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	buff_virt = (char *)dma_alloc_coherent(hisee_data_ptr->cma_device, SIZE_4K,
					       &buff_phy, GFP_KERNEL);
	if (!buff_virt) {
		pr_err("%s(): dma_alloc_coherent failed\n", __func__);
		return set_errno_then_exit(HISEE_NO_RESOURCES);
	}
	(void)memset_s(buff_virt, SIZE_4K, 0, SIZE_4K);
	p_message_header = (struct atf_message_header *)buff_virt;
	set_message_header(p_message_header, CMD_PRESAVE_COS_APPDATA);

	for (i = 0; i < rpmb_cos; i++) {
		p_message_header->ack = i;
		ret = send_smc_process(p_message_header, buff_phy,
				       HISEE_ATF_MESSAGE_HEADER_LEN,
				       HISEE_ATF_COS_APPDATA_TIMEOUT,
				       CMD_PRESAVE_COS_APPDATA);
	}

	check_and_print_result();
	dma_free_coherent(hisee_data_ptr->cma_device, SIZE_4K,
			  buff_virt, buff_phy);
	return set_errno_then_exit(ret);
}

static int hisee_wait_partition_ready(void)
{
	char fullpath[MAX_PATH_NAME_LEN] = {0};
	int retry = HISEE_PARTITION_READY_TIMEOUT_CNT;
	const unsigned int timeout = HISEE_PARTITION_READY_TIMEOUT_MS;
	int ret;
	ret = flash_find_ptn_s(HISEE_IMAGE_PARTITION_NAME, fullpath,
			       sizeof(fullpath));
	if (ret != 0) {
		pr_err("%s():flash_find_ptn_s fail\n", __func__);
		return ret;
	}
	do {
		if (sys_access(fullpath, 0) == 0)
			break;
		msleep(timeout);
		retry--;
	} while (retry > 0);

	if (retry <= 0)
		return HISEE_ERROR;

	return HISEE_OK;
}

int hisee_lpmcu_send(rproc_msg_t msg_0, rproc_msg_t msg_1)
{
	int retry;
	int ret;

	rproc_id_t rproc_id = HISI_ACPU_LPM3_MBX_6;
	rproc_msg_t tx_buffer[HISEE_IPC_TXBUF_SIZE] = {0};
	rproc_msg_t ack_buffer[HISEE_IPC_ACKBUF_SIZE] = {0};

	tx_buffer[IPC_TX_CMD_INDEX] = IPC_CMD(OBJ_AP, OBJ_LPM3, CMD_NOTIFY, 0);
	tx_buffer[IPC_TX_MSG_0_INDEX] = msg_0;
	tx_buffer[IPC_TX_MSG_1_INDEX] = msg_1;
	retry = HISEE_LPMCU_SEND_RETRY_COUNT;

	do {
		ret = RPROC_SYNC_SEND(rproc_id, tx_buffer, HISEE_IPC_TXBUF_SIZE,
				      ack_buffer, HISEE_IPC_ACKBUF_SIZE);
		if (ret == 0 && ack_buffer[IPC_ACK_MSG_0_INDEX] == HISEE_LPM3_CMD &&
		    ack_buffer[IPC_ACK_MSG_1_INDEX] == HISEE_LPM3_ACK_MAGIC) {
			/* the send is reponsed by the remote process, break out */
			ret = HISEE_OK;
			break;
		} else if (ret == -ETIMEOUT) {
			/* the timeout will print out, below message to tell it's normal */
			retry--;
			ret = HISEE_POWERCTRL_TIMEOUT_ERROR;
			pr_err("%s(): the ack of sending ipc is timeout.\n",
			       __func__);
			continue;
		} else {
			pr_err("%s(): send ipc failed\n", __func__);
			ret = HISEE_POWERCTRL_NOTIFY_ERROR;
			retry = 0;
			break;
		}
	} while (retry);

	if (retry == 0) {
		pr_err("%s(): send ipc with retry still failed\n", __func__);
		ret = HISEE_POWERCTRL_RETRY_FAILURE_ERROR;
	}

	return set_errno_then_exit(ret);
}


static int set_lpmcu_nfc_irq(void)
{
	struct device_node *pn547_nfc_dev = NULL;
	int irq_gpio;

	pn547_nfc_dev = of_find_compatible_node(NULL, NULL,
						"hisilicon,pn547_nfc");
	if (!pn547_nfc_dev) {
		pr_err("%s pn547_nfc not found\n", __func__);
		return HISEE_OK;
	}

	irq_gpio  = of_get_named_gpio(pn547_nfc_dev, "pn547,irq", 0);
	if (!gpio_is_valid(irq_gpio)) {
		pr_err("%s pn547,irq not found\n", __func__);
		return HISEE_OK;
	}


	return hisee_lpmcu_send(SET_NFC_IRQ, (unsigned int)irq_gpio);
}

static void set_hisee_lcs_mode(void)
{
	unsigned int value;
	unsigned int hisee_lcs_mode = 0;
	unsigned int __iomem *addr = NULL;

	/* set lcs mode for lpmcu */
	if (get_hisee_lcs_mode(&hisee_lcs_mode) != HISEE_OK) {
		pr_err("%s() get_hisee_lcs_mode failed\n", __func__);
		return;
	}

	addr = (unsigned int __iomem *)ioremap(MSPC_LCS_BANK_REG, sizeof(unsigned int));
	if (!addr) {
		pr_err("%s: ioremap failed!\n", __func__);
		return;
	}
	value = readl(addr);
	if (hisee_lcs_mode == HISEE_DM_MODE_MAGIC)
		value |= BIT(HISEE_LCS_DM_BIT);
	else
		value &= (~BIT(HISEE_LCS_DM_BIT));

	writel(value, addr);
	iounmap(addr);
}

/*
 * @brief   : run multi cos image upgrade for everyone cos image file in hisee.img
 * @return  : ::int, 0 on success, other value on failure
 */
int cos_image_upgrade_by_self(void)
{
	int ret = HISEE_ERROR;
	int new_cos_exist;
	unsigned int cos_id;
	char buf_para[MAX_CMD_BUFF_PARAM_LEN] = {0};
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	__pm_stay_awake(&hisee_data_ptr->wake_lock);
	if (get_rpmb_key_status() == KEY_NOT_READY) {
		pr_err("hisee:%s() rpmb key is not ready. cos upgrade bypassed",
		       __func__);
		goto exit;
	}

	buf_para[0] = HISEE_CHAR_SPACE; /* space character */
	buf_para[HISEE_PROCESS_TYPE_POS] = '0' + COS_PROCESS_UPGRADE;
	for (cos_id = 0; cos_id < HISEE_SUPPORT_COS_FILE_NUMBER; cos_id++) {
		new_cos_exist = HISEE_FALSE;
		ret = check_new_cosimage(cos_id, &new_cos_exist);
		if (ret == HISEE_OK) {
			if (new_cos_exist == HISEE_FALSE) {
				pr_err("hisee:%s() cos_id=%u no new cosimage, no need to upgrade\n",
				       __func__, cos_id);
				continue;
			}
		} else {
			pr_err("hisee:%s() cos_id=%u check_new_cosimage failed,ret=%d\n",
			       __func__, cos_id, ret);
			goto exit;
		}
		buf_para[HISEE_COS_ID_POS] = '0' + cos_id;
		ret = hisee_poweron_upgrade_func(buf_para, 0);
		if (ret != HISEE_OK) {
			pr_err("hisee:%s() power failed, cos_id=%u,ret=%d\n",
			       __func__, cos_id, ret);
			goto exit;
		}

		hisee_mdelay(DELAY_FOR_HISEE_POWERON_UPGRADE);
		ret = handle_cos_image_upgrade(buf_para, 0);
		if (ret != HISEE_OK) {
			pr_err("hisee:%s() upgrade failed, cos_id=%u,ret=%d\n",
			       __func__, cos_id, ret);
			goto exit;
		}
	}
exit:
	__pm_relax(&hisee_data_ptr->wake_lock);
	g_cos_image_upgrade_done = HISEE_TRUE;
	check_and_print_result();
	return ret;
}

#if defined(CONFIG_HICOS_MISCIMG_PATCH) && !defined(CONFIG_HISEE_DISABLE_KEY)
static void hisee_cos_patch_load(void)
{
	int ret;
	int retry = 5; /* local retry count */

	do {
		ret = hisee_cos_patch_read(MISC3_IMG_TYPE);
		if (ret == HISEE_OK)
			break;
		msleep(DELAY_1000_MS);
		retry--;
	} while (ret != HISEE_OK && retry > 0);

	if (ret != HISEE_OK)
		pr_err("cos patch read failed %d", ret);
}
#endif

#ifdef CONFIG_SMX_PROCESS
static int smx_process_config(void)
{
	int ret;
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	mutex_lock(&hisee_data_ptr->hisee_mutex);

	ret = smx_process(SMX_PROCESS_STEP1_ACTION,
			  SMX_PROCESS_STEP1_ID, SMX_PROCESS_STEP1_CMD);
	if (ret == SMX_PROCESS_UNSUPPORT) {
		pr_err("%s(): unsupport\n", __func__);
		goto smx_pass;
	} else if (ret == SMX_PROCESS_SUPPORT_AND_OK) {
		pr_err("%s(): support and ok\n", __func__);
		/* wait hisee cos ready for later process */
		ret = wait_hisee_ready(HISEE_STATE_COS_READY, DELAY_10_SECOND);
		if (ret != HISEE_OK)
			pr_err("%s(): wait_hisee_ready failed ret=%x\n",
			       __func__, ret);
	} else {
		pr_err("%s(): ret=0x%x\n", __func__, ret);
		goto smx_fail;
	}

smx_pass:
	(void)smx_process(SMX_PROCESS_STEP2_ACTION,
			  SMX_PROCESS_STEP2_ID, SMX_PROCESS_STEP2_CMD);
	mutex_unlock(&hisee_data_ptr->hisee_mutex);
	return HISEE_OK;

smx_fail:
	(void)smx_process(SMX_PROCESS_STEP2_ACTION,
			  SMX_PROCESS_STEP2_ID, SMX_PROCESS_STEP2_CMD);
	mutex_unlock(&hisee_data_ptr->hisee_mutex);
	return HISEE_ERROR;
}
#endif

static int _rpmb_prepared_work(void)
{
	int retry = RPMB_READY_TIMEOUT_CNT;
	int find_part_res;
	unsigned int hisee_hibench_flag = HISEE_TRUE;
	int ret;

	/* poll rpmb module ready status about 90 seconds */
	do {
		if (get_rpmb_init_status() == RPMB_DRIVER_IS_READY) {
			get_hisee_data_ptr()->rpmb_is_ready = HISEE_TRUE;
			break;
		}
		msleep(RPMB_READY_TIMEOUT_MS);
		retry--;
	} while (retry > 0);

	/* set lcs mode for lpmcu */
	set_hisee_lcs_mode();

	if (retry == 0) {
		pr_err("%s ERROR:HISEE image is not loaded, rpmb is not initiated, retry = %d\n",
		       __func__, retry);
		g_cos_image_upgrade_done = HISEE_TRUE;
		return HISEE_ERROR;
	}

	find_part_res = hisee_wait_partition_ready();
	/* get hibench flag for efuse */
	ret = get_hisee_hibench_flag(&hisee_hibench_flag);
	if (ret != HISEE_OK)
		pr_err("%s() get hisee hibench flag failed.\n", __func__);

	if (find_part_res != HISEE_OK)
		pr_err("%s ERROR:HISEE partition is not OK, find_part_res = %d\n",
		       __func__, find_part_res);

	/*
	 * only when upgrade no need authentication and not a factory version,
	 * allow cos upgrade by self
	 */
	if (!g_hisee_is_fpga && hisee_hibench_flag == HISEE_TRUE &&
	    is_hisee_chiptest_slt() == false && find_part_res == HISEE_OK) {
		ret = cos_image_upgrade_by_self();
		if (ret != HISEE_OK)
			pr_err("hisee:%s() cos_image_upgrade_by_self() failed, retcode=%d\n",
			       __func__, ret);
	}
	return HISEE_OK;
}

#ifdef CONFIG_GENERAL_SEE_PINCODE
#define GENERAL_SEE_PINCODE_UNINITIALIZED    0U
#define GENERAL_SEE_PINCODE_UNSUPPORT        1U
#define GENERAL_SEE_PINCODE_SUPPORT          2U

static int hisee_check_status(void)
{
	int state;

	state = atfd_hisee_smc((u64)HISEE_FN_MAIN_SERVICE_CMD,
				GENERAL_SEE_GET_PINCODE_STATUS, 0, 0);
	if (state == GENERAL_SEE_PINCODE_UNINITIALIZED ||
	    state == GENERAL_SEE_PINCODE_SUPPORT)
		return HISEE_OK;

	pr_err("%s:hisee is not supported or get status failed:%x!\n", __func__, state);
	return HISEE_ERROR;
}
#endif /* CONFIG_GENERAL_SEE_PINCODE */

static int rpmb_ready_body(void *arg)
{
	int ret;
	struct cosimage_version_info misc_version = {0};
#ifdef CONFIG_GENERAL_SEE_PINCODE
	const uint8_t *time = " 60000 1"; /* 60000 means 60s, 1 means vote id. */
#endif

	misc_version.img_version_num[0] = 0;

	ret = _rpmb_prepared_work();
	if (ret != HISEE_OK) {
		pr_err("_rpmb_prepared_work failed!\n");
		return set_errno_then_exit(ret);
	}

	ret = load_cosimg_appdata_ddr();
	if (ret != HISEE_OK)
		pr_err("hisee rpmb cos read failed!\n");


#ifdef CONFIG_HICOS_MISCIMG_PATCH
	/* cos patch upgrade only supported in this function */
	hisee_cos_patch_load();
#endif

#ifdef CONFIG_SMX_PROCESS
	if (smx_process_config() != HISEE_OK)
		pr_err("%s ERROR:hisee force boot and power down\n", __func__);
#endif
	check_and_print_result();
	g_cos_image_upgrade_done = HISEE_TRUE;

#ifdef CONFIG_GENERAL_SEE_PINCODE
	ret = hisee_check_status();
	if (ret != HISEE_OK) {
		pr_err("%s:hisee is not supported:%x!\n", __func__, ret);
		return set_errno_then_exit(ret);
	}

	ret = hisee_poweron_timeout_func(time, 0);
	if (ret != HISEE_OK)
		pr_err("%s:power on hisee failed:%x\n", __func__, ret);
#endif

	return set_errno_then_exit(ret);
}

static int flash_hisee_debug_switches(void)
{
	int ret;
	char buf_para[MAX_CMD_BUFF_PARAM_LEN] = {0};

	buf_para[0] = HISEE_CHAR_SPACE; /* space character */
	buf_para[HISEE_COS_ID_POS] = '0' + COS_IMG_ID_0;
	buf_para[HISEE_PROCESS_TYPE_POS] = '0' + COS_PROCESS_UPGRADE;

	mutex_lock(&g_hisee_otp_mutex);
	hisee_chiptest_set_otp1_status(RUNNING);

	ret = hisee_poweron_booting_func((void *)buf_para, 0);
	if (ret != HISEE_OK)
		goto tag_finish;

	/* add power vote protect otp-writing from power off */
	ret = hisee_poweron_booting_func((void *)buf_para, 0);
	if (ret != HISEE_OK) {
		(void)hisee_poweroff_func((void *)buf_para,
					  (int)HISEE_PWROFF_LOCK);
		goto tag_finish;
	}

	ret = wait_hisee_ready(HISEE_STATE_COS_READY, HISEE_ATF_GENERAL_TIMEOUT);
	if (ret == HISEE_OK) {
		ret = write_hisee_otp_value(OTP1_IMG_TYPE);
		if (ret != HISEE_OK)
			pr_err("%s() write_hisee_otp_value failed,ret=%d\n",
			       __func__, ret);
	}

	hisee_mdelay(DELAY_BETWEEN_STEPS);
	/* remove power vote protect otp-writing from power off */
	(void)hisee_poweroff_func((void *)buf_para, (int)HISEE_PWROFF_LOCK);
	(void)hisee_poweroff_func((void *)buf_para, (int)HISEE_PWROFF_LOCK);

tag_finish:
	check_and_print_result();
	set_hisee_errno(ret);
	hisee_chiptest_set_otp1_status(FINISH);
	mutex_unlock(&g_hisee_otp_mutex);

	return ret;
}


int hisee_check_secdebug_flash_otp1(void)
{
	bool b_disabled = false;
	int ret;

	if (hisee_chiptest_get_otp1_status() != PREPARED) {
		/* no need to write otp1 */
		pr_err("no need to write otp without pinstall\n");
		return HISEE_OK;
	}

	ret = efuse_check_secdebug_disable(&b_disabled);
	if (ret != HISEE_OK)
		return HISEE_ERROR;

	if (b_disabled)
		ret = flash_hisee_debug_switches();

	return ret;
}

static void show_hisee_module_status(char *buff, size_t buff_len)
{
	char status_string[12] = {0}; /* 12 is enough for creating status string */
	char *index_name = "rpmb_is_ready=";
	int ret;
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	ret = snprintf_s(status_string, sizeof(status_string),
			 sizeof(status_string) - 1,
			 "%u\n", hisee_data_ptr->rpmb_is_ready);
	if (ret == HISEE_SECLIB_ERROR) {
		pr_err("%s snprintf1 err.\n", __func__);
		return;
	}
	ret = strncpy_s(buff, buff_len, index_name, strlen(index_name));
	if (ret != EOK) {
		pr_err("%s strncpy1 err.\n", __func__);
		return;
	}
	ret = strncpy_s(buff + strlen(buff), buff_len - strlen(buff),
			status_string, strlen(status_string));
	if (ret != EOK) {
		pr_err("%s strncpy2 err.\n", __func__);
		return;
	}

	(void)memset_s(status_string, sizeof(status_string),
		       0, sizeof(status_string));
	index_name = "smc_cmd_running=";
	ret = snprintf_s(status_string, sizeof(status_string),
			 sizeof(status_string) - 1,
			 "%u\n", hisee_data_ptr->smc_cmd_running);
	if (ret == HISEE_SECLIB_ERROR) {
		pr_err("%s snprintf2 err.\n", __func__);
		return;
	}

	ret = strncpy_s(buff + strlen(buff), buff_len - strlen(buff),
			index_name, strlen(index_name));
	if (ret != EOK) {
		pr_err("%s strncpy3 err.\n", __func__);
		return;
	}

	ret = strncpy_s(buff + strlen(buff), buff_len - strlen(buff),
			status_string, strlen(status_string));
	if (ret != EOK) {
		pr_err("%s strncpy4 err.\n", __func__);
		return;
	}

}

static int _powerctrl_show_get_status(char *buf, char *result_buf, size_t len)
{
	int ret;
	int status;

	if (!buf || !result_buf) {
		pr_err("%s buf params in null.\n", __func__);
		return HISEE_ERROR;
	}

	status = hisee_get_power_status();
	switch (status) {
	case HISEE_POWER_STATUS_ON:
		ret = strncat_s(result_buf, len, "1",
				len - 1 - strlen(result_buf));
		if (ret != EOK) {
			pr_err("%s(): strncat1 err.\n", __func__);
			return HISEE_SECUREC_ERR;
		}
		break;
	case HISEE_POWER_STATUS_OFF:
		ret = strncat_s(result_buf, len, "2",
				len - 1 - strlen(result_buf));
		if (ret != EOK) {
			pr_err("%s(): strncat2 err.\n", __func__);
			return HISEE_SECUREC_ERR;
		}
		break;
	default:
		pr_err("%s(): invalid powerctrl status\n", __func__);
		ret = strncpy_s(buf, HISEE_BUF_SHOW_LEN, "0", sizeof("0"));
		if (ret != EOK) {
			pr_err("%s(): strncpy err.\n", __func__);
			return HISEE_SECUREC_ERR;
		}
		return HISEE_ERROR;
	}

	return HISEE_OK;
}

static int _powerctrl_show_get_info(char *result_buf, size_t size)
{
	int ret;
	int err_code;
	unsigned int i;
	size_t remaining_len;

	/* save error no in the buf of user. */
	err_code = get_hisee_errno();
	if (err_code == HISEE_OK)
		return HISEE_OK;

	ret = snprintf_s(result_buf + strlen(result_buf),
			 size - strlen(result_buf),
			 HISEE_PWR_ERRNO_PRINT_SIZE, " %d,", err_code);
	if (ret == HISEE_SECLIB_ERROR) {
		pr_err("%s snprintf err.\n", __func__);
		return HISEE_SECUREC_ERR;
	}

	for (i = 0; i < ARRAY_SIZE(g_errcode_items_des); i++) {
		if (err_code == g_errcode_items_des[i].err_code)
			break;
	}
	if (i == ARRAY_SIZE(g_errcode_items_des)) {
		pr_err("%s(): can't find errcode=%d definition\n",
		       __func__, err_code);
		remaining_len = (size - 1) - strlen(result_buf);
		ret = strncat_s(result_buf + strlen(result_buf),
				size - strlen(result_buf),
				"undefined err",
				remaining_len);
		if (ret != EOK) {
			pr_err("%s(): strncat3 err.\n", __func__);
			return HISEE_SECUREC_ERR;
		}
	} else {
		remaining_len = (size - 1) - strlen(result_buf);
		ret = strncat_s(result_buf + strlen(result_buf),
				size - strlen(result_buf),
				g_errcode_items_des[i].err_description,
				remaining_len);
		if (ret != EOK) {
			pr_err("%s(): strncat4 err.\n", __func__);
			return HISEE_SECUREC_ERR;
		}
	}
	return HISEE_OK;
}

/*
 * @brief      : read the powerctrl_cmd execute status
 * @param[out] : buf, save the ioctl_cmd execute status, the size not less than 160 bytes
 * @return     : ::ssize_t, 0 on success, other value on failure
 */
static ssize_t hisee_powerctrl_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	int ret;
	char buff[HISEE_ERROR_DESCRIPTION_MAX] = {0};

	if (!buf) {
		pr_err("%s buf params is null\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}
	ret = _powerctrl_show_get_status(buf, buff, HISEE_ERROR_DESCRIPTION_MAX);
	if (ret != HISEE_OK) {
		pr_err("%s(): show_get_status err=%d.\n", __func__, ret);
		return strlen(buf);
	}

	ret = _powerctrl_show_get_info(buff, HISEE_ERROR_DESCRIPTION_MAX);
	if (ret != HISEE_OK)
		return set_errno_then_exit(ret);

	ret = memcpy_s(buf, HISEE_BUF_SHOW_LEN,
		       buff, HISEE_ERROR_DESCRIPTION_MAX);
	if (ret != EOK) {
		pr_err("%s(): memcpy err.\n", __func__);
		return set_errno_then_exit(HISEE_SECUREC_ERR);
	}
	pr_err("%s(): success.\n", __func__);
	return strlen(buf);
}

static int hisee_powerctrl_store_params_check(const char *buf, size_t count,
					      unsigned int *cmd_index)
{
	char *ptr_name = NULL;
	unsigned int i = 0;

	if (!buf || strlen(buf) >= HISEE_CMD_NAME_LEN) {
		pr_err("%s buf parameters is invalid\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}
	if (count <= 0 || count >= HISEE_CMD_NAME_LEN) {
		pr_err("%s count parameters is invalid\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	for (ptr_name = g_hisee_lpm3_function_list[i].function_name;
	     ptr_name;) {
		if (!strncmp(buf, ptr_name, strlen(ptr_name))) {
			*cmd_index = i;
			break;
		}
		i++;
		ptr_name = g_hisee_lpm3_function_list[i].function_name;
	}
	if (!ptr_name) {
		pr_err("%s cmd=%s invalid\n", __func__, buf);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	return HISEE_OK;
}

/*
 * @brief      : write the powerctrl_cmd to execute
 * @param[in]  : buf, content string buffer
 * @return     : ::ssize_t, positive value on success, negative value on failure
 */
static ssize_t hisee_powerctrl_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	int ret;
	unsigned int cmd_index = 0;
	unsigned int upgrade_run_flg;
	unsigned int hisee_lcs_mode = 0;
	char *func_name = NULL;

	if (g_cos_image_upgrade_done == HISEE_FALSE) {
		pr_err("%s() hisee is upgrading now, can not power on or off hisee! Please do it later.\n",
		       __func__);
		return set_errno_then_exit(HISEE_IS_UPGRADING);
	}
	if (get_hisee_data_ptr()->rpmb_is_ready == KEY_NOT_READY) {
		pr_err("%s rpmb is not ready now\n", __func__);
#ifdef CONFIG_HUAWEI_DSM
		record_hisee_log_by_dmd(DSM_HISEE_POWER_ON_OFF_ERROR_NO,
					HISEE_RPMB_MODULE_INIT_ERROR);
#endif
		return set_errno_then_exit(HISEE_RPMB_MODULE_INIT_ERROR);
	}

	ret = hisee_powerctrl_store_params_check(buf, count, &cmd_index);
	if (ret != HISEE_OK) {
		pr_err("%s() hisee_powerctrl_store_params_check failed,ret=%d\n",
		       __func__, ret);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	ret = get_hisee_lcs_mode(&hisee_lcs_mode);
	if (ret != HISEE_OK) {
		pr_err("%s() get_hisee_lcs_mode failed,ret=%d\n", __func__, ret);
		if (cmd_index == 0)
			return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	if (hisee_lcs_mode == (unsigned int)HISEE_SM_MODE_MAGIC &&
	    cmd_index == 0) { /* to be improved */
		upgrade_run_flg = HISEE_COS_UPGRADE_RUNNING_FLG;
		access_hisee_image_partition((char *)&upgrade_run_flg,
					     COS_UPGRADE_RUN_WRITE_TYPE);
	}

	func_name = g_hisee_lpm3_function_list[cmd_index].function_name;
	ret = g_hisee_lpm3_function_list[cmd_index].function_ptr((void *)(buf +
			strlen(func_name)),
			(int)(uintptr_t)dev);
	if (ret != HISEE_OK) {
		pr_err("%s powerctrl_cmd:%s failed, retcode=%d\n", __func__,
		       func_name, ret);
#ifdef CONFIG_HUAWEI_DSM
		record_hisee_log_by_dmd(DSM_HISEE_POWER_ON_OFF_ERROR_NO, ret);
#endif
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	} else {
		pr_err("%s :%s success\n", __func__, func_name);
		if (hisee_lcs_mode == (unsigned int)HISEE_SM_MODE_MAGIC &&
		    cmd_index == 0) {
			upgrade_run_flg = 0;
			access_hisee_image_partition((char *)&upgrade_run_flg,
						     COS_UPGRADE_RUN_WRITE_TYPE);
		}
		set_hisee_errno(HISEE_OK);
		return count;
	}
}

static int _ioctl_show_get_info(char *buf, int *err_code, unsigned int *err_id)
{
	int ret;
	int errno;
	unsigned int i;

	if (!err_code || !err_id) {
		pr_err("%s invalid params\n", __func__);
		return HISEE_ERROR;
	}

	errno = get_hisee_errno();
	for (i = 0; i < ARRAY_SIZE(g_errcode_items_des); i++)
		if (errno == g_errcode_items_des[i].err_code)
			break;

	if (i == ARRAY_SIZE(g_errcode_items_des)) {
		pr_err("%s(): can't find errcode=%d definition\n", __func__, errno);
		ret = strncpy_s(buf, HISEE_BUF_SHOW_LEN,
				"-1,failed!", sizeof("-1,failed!"));
		if (ret != EOK) {
			pr_err("%s(): strncpy err.\n", __func__);
			return HISEE_SECUREC_ERR;
		}
		return HISEE_ERROR;
	}
	/* update error code and error id */
	*err_code = errno;
	*err_id = i;

	return HISEE_OK;
}

static int __ok_handle_norunning(char *result_buf, size_t size)
{
	int ret;
	size_t remaining_len;

	if (!result_buf) {
		pr_err("%s result_buf is NULL\n", __func__);
		return HISEE_ERROR;
	}
	ret = snprintf_s(result_buf, size,
			 HISEE_SHOW_VALUE_LEN, "%d,",
			 HISEE_FACTORY_TEST_NORUNNING);
		if (ret == HISEE_SECLIB_ERROR) {
			pr_err("%s snprintf err.\n", __func__);
			return HISEE_SECUREC_ERR;
		}
		remaining_len = HISEE_ERROR_DESCRIPTION_MAX - 1 - strlen(result_buf);
		ret = strncat_s(result_buf, size, "no running", remaining_len);
		if (ret != EOK) {
			pr_err("%s(): strncat1 err.\n", __func__);
			return HISEE_SECUREC_ERR;
	}
	return HISEE_OK;
}

static int __ok_handle_running(char *result_buf, size_t size)
{
	int ret;
	size_t remaining_len;

	if (!result_buf) {
		pr_err("%s result_buf is NULL\n", __func__);
		return HISEE_ERROR;
	}
	ret = snprintf_s(result_buf, size,
			 HISEE_SHOW_VALUE_LEN, "%d,",
			 HISEE_FACTORY_TEST_RUNNING);
	if (ret == HISEE_SECLIB_ERROR) {
		pr_err("%s snprintf err.\n", __func__);
		return HISEE_SECUREC_ERR;
	}
	remaining_len = HISEE_ERROR_DESCRIPTION_MAX - 1 - strlen(result_buf);
	ret = strncat_s(result_buf, size, "running", remaining_len);
	if (ret != EOK) {
		pr_err("%s(): strncat2 err.\n", __func__);
		return HISEE_SECUREC_ERR;
	}
	return HISEE_OK;
}

static int __ok_handle_success(char *result_buf, size_t size)
{
	int ret;
	size_t remaining_len;

	if (!result_buf) {
		pr_err("%s result_buf is NULL\n", __func__);
		return HISEE_ERROR;
	}
	if (hisee_check_secdebug_flash_otp1() == HISEE_OK) {
		ret = snprintf_s(result_buf, size,
				 HISEE_SHOW_VALUE_LEN, "%d,",
				 HISEE_FACTORY_TEST_SUCCESS);
		if (ret == HISEE_SECLIB_ERROR) {
			pr_err("%s snprintf1 err.\n", __func__);
			return HISEE_SECUREC_ERR;
		}
		remaining_len = HISEE_ERROR_DESCRIPTION_MAX - 1 - strlen(result_buf);
		ret = strncat_s(result_buf, size, "no error", remaining_len);
		if (ret != EOK) {
			pr_err("%s(): strncat2 err.\n", __func__);
			return HISEE_SECUREC_ERR;
		}
	} else {
		ret = snprintf_s(result_buf, size,
				 HISEE_SHOW_VALUE_LEN, "%d,",
				 HISEE_OTP1_WRITE_FAIL);
		if (ret == HISEE_SECLIB_ERROR) {
			pr_err("%s snprintf1 err.\n", __func__);
			return HISEE_SECUREC_ERR;
		}
		remaining_len = HISEE_ERROR_DESCRIPTION_MAX - 1 - strlen(result_buf);
		ret = strncat_s(result_buf, size, "OTP1 write fail", remaining_len);
		if (ret != EOK) {
			pr_err("%s(): strncat2 err.\n", __func__);
			return HISEE_SECUREC_ERR;
		}
	}
	return HISEE_OK;
}

static int _ioctl_show_ok_handle(char *result_buf, size_t size)
{
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	if (hisee_data_ptr->factory_test_state == HISEE_FACTORY_TEST_NORUNNING)
		return  __ok_handle_norunning(result_buf, size);
	else if (hisee_data_ptr->factory_test_state == HISEE_FACTORY_TEST_RUNNING ||
		 hisee_chiptest_otp1_is_running() == true)
		return __ok_handle_running(result_buf, size);
	else if (hisee_data_ptr->factory_test_state == HISEE_FACTORY_TEST_SUCCESS)
		return __ok_handle_success(result_buf, size);
	else
		pr_err("%s nothing todo.\n", __func__);

	return HISEE_OK;
}

static int _ioctl_show_nok_handle(char *result_buf, size_t size,
				  int err_code, unsigned int err_id)
{
	int ret;
	size_t remaining_len;

	ret = snprintf_s(result_buf, size,
			 HISEE_SHOW_VALUE_LEN, "%d,", err_code);
	if (ret == HISEE_SECLIB_ERROR) {
		pr_err("%s snprintf err.\n", __func__);
		return HISEE_SECUREC_ERR;
	}
	remaining_len = HISEE_ERROR_DESCRIPTION_MAX - 1 - strlen(result_buf);
	ret = strncat_s(result_buf, size,
			g_errcode_items_des[err_id].err_description,
			remaining_len);
	if (ret != EOK) {
		pr_err("%s(): strncat err.\n", __func__);
		return HISEE_SECUREC_ERR;
	}

	return HISEE_OK;
}

/*
 * @brief      : read the ioctl_cmd execute status
 * @param[out] : buf, save the ioctl_cmd execute status,
 *               the size should less than 64 bytes
 * @return     : ::ssize_t, positive value on success, negative value on failure
 */
static ssize_t hisee_ioctl_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int err_code = 0;
	unsigned int err_id = 0;
	int ret;
	char buff[HISEE_CMD_NAME_LEN] = {0};

	if (!buf) {
		pr_err("%s buf parameters is null\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	if (g_hisee_api_bypassed) {
		pr_info("[%s] hisee bypass\n", __func__);
		ret = strncpy_s(buf, HISEE_CMD_NAME_LEN, HISEE_API_BYPASS_MSG,
				sizeof(HISEE_API_BYPASS_MSG));
		return ret == EOK ? (sizeof(HISEE_API_BYPASS_MSG) - 1) : ret;
	}

	ret = _ioctl_show_get_info(buf, &err_code, &err_id);
	if (ret != HISEE_OK) {
		pr_err("%s(): _ioctl_show_get_info err=%d.\n", __func__, ret);
		return strlen(buf);
	}
	if (err_code == HISEE_OK)
		ret = _ioctl_show_ok_handle(buff, HISEE_CMD_NAME_LEN);
	else
		ret = _ioctl_show_nok_handle(buff, HISEE_CMD_NAME_LEN, err_code, err_id);

	if (ret != HISEE_OK)
		return set_errno_then_exit(ret);

	ret = memcpy_s(buf, HISEE_BUF_SHOW_LEN, buff, HISEE_ERROR_DESCRIPTION_MAX);
	if (ret != EOK) {
		pr_err("%s(): memcpy err.\n", __func__);
		return set_errno_then_exit(HISEE_SECUREC_ERR);
	}

	ret = memset_s(buff, HISEE_CMD_NAME_LEN,
		       0, (unsigned long)HISEE_CMD_NAME_LEN);
	if (ret != EOK) {
		pr_err("%s(): memset_s err.\n", __func__);
		return set_errno_then_exit(HISEE_SECUREC_ERR);
	}
	show_hisee_module_status(buff, HISEE_CMD_NAME_LEN);
	pr_err("%s\n", buff);

	return strlen(buf);
}

/*
 * @brief      : write the ioctl_cmd to execute
 * @param[in]  : buf, save the ioctl_cmd string should be execute
 * @return     : ::ssize_t, positive value on success, negative value on failure
 */
static ssize_t hisee_ioctl_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	int ret;
	int i = 0;
	int cmd_index = 0;
	char *ptr_name = NULL;

	if (!buf || strlen(buf) >= HISEE_IOCTL_CMD_MAX_LEN) {
		pr_err("%s buf parameters is invalid\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}
	if (count <= 0 || count >= HISEE_IOCTL_CMD_MAX_LEN) {
		pr_err("%s count parameters is invalid\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	if (g_hisee_api_bypassed) {
		pr_info("[%s] hisee bypass\n", __func__);
		return count;
	}

	if (get_hisee_data_ptr()->rpmb_is_ready == KEY_NOT_READY) {
		pr_err("%s rpmb is not ready now\n", __func__);
		return set_errno_then_exit(HISEE_RPMB_MODULE_INIT_ERROR);
	}
	for (ptr_name = g_hisee_atf_function_list[i].function_name; ptr_name;) {
		if (!strncmp(buf, ptr_name, strlen(ptr_name))) {
			cmd_index = i;
			break;
		}
		i++;
		ptr_name = g_hisee_atf_function_list[i].function_name;
	}
	if (!ptr_name) {
		pr_err("%s cmd=%s invalid\n", __func__, buf);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	ptr_name = g_hisee_atf_function_list[cmd_index].function_name;
	ret = g_hisee_atf_function_list[cmd_index].function_ptr((void *)(buf +
				strlen(ptr_name)),
				(int)(uintptr_t)dev);
	if (ret != HISEE_OK) {
		pr_err("%s ioctl_cmd:%s failed, retcode=%d\n", __func__,
		       ptr_name, ret);
		return HISEE_INVALID_PARAMS;
	}

	pr_err("%s ioctl_cmd:%s success\n", __func__, ptr_name);
	return (ssize_t)count;
}

/*
 * @brief      : read the apdu command response,size is 0--255 bytes
 * @param[out] : buf, save the apdu command ack data, the size is 0--255 bytes
 * @return     : ::ssize_t, positive value on success, negative value on failure
 */
static ssize_t hisee_apdu_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	size_t ret_len;
	int ret;
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	if (!buf) {
		pr_err("%s buf parameters is null\n", __func__);
		return (ssize_t)0;
	}

	mutex_lock(&g_hisee_apdu_mutex);

	ret_len = (size_t)hisee_data_ptr->apdu_ack.ack_len;
	if (ret_len > HISEE_APDU_DATA_LEN_MAX) {
		pr_err("%s ack_len=%ld is invalid\n", __func__, ret_len);
		hisee_data_ptr->apdu_ack.ack_len = 0;
		ret_len = 0;
		goto finished;
	}

	ret = memcpy_s(buf, HISEE_APDU_DATA_LEN_MAX + 1, /* plus 1 for '/0' */
		       hisee_data_ptr->apdu_ack.ack_buf, ret_len);
	if (ret != EOK) {
		pr_err("%s memcpy error.\n", __func__);
		ret_len = 0;
		goto finished;
	}
	hisee_data_ptr->apdu_ack.ack_len = 0;

finished:
	mutex_unlock(&g_hisee_apdu_mutex);
	buf[ret_len] = 0;
	return (ssize_t)ret_len;
}

/*
 * @brief      : write the apdu command function, size is 0--261 bytes
 * @param[in]  : buf, save the  apdu command data should be execute
 * @return     : ::ssize_t, positive value on success, negative value on failure
 */
static ssize_t hisee_apdu_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int ret;

	if (!buf) {
		pr_err("%s buf parameters is invalid\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}
	if (count == 0 || count > HISEE_APDU_DATA_LEN_MAX) {
		pr_err("%s count parameters is invalid\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}
	if (get_hisee_data_ptr()->rpmb_is_ready == KEY_NOT_READY) {
		pr_err("%s rpmb is not ready now\n", __func__);
		return set_errno_then_exit(HISEE_RPMB_MODULE_INIT_ERROR);
	}

	if  (hisee_get_power_status() != HISEE_POWER_STATUS_ON) {
		pr_err("%s hisee is not poweron\n", __func__);
		return set_errno_then_exit(HISEE_POWERCTRL_FLOW_ERROR);
	}

	ret = write_apdu_command_func((char *)buf, (unsigned int)count);
	if (ret !=  HISEE_OK)
		return HISEE_INVALID_PARAMS;

	pr_err("%s write_apdu_command_func success, retcode=%d\n",
	       __func__, ret);
	return (ssize_t)count;
}

/*
 * @brief      : Only for debuging, check the vote value of hisee power
 * @param[in]  : buf, save the  apdu command data should be execute
 * @return     : ::ssize_t, positive value on success, negative value on failure
 */
static ssize_t hisee_power_debug_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
#ifdef CONFIG_HISEE_MNTN
	char buff[HISEE_BUF_SHOW_LEN] = {0};
	u32 vote_lpm3;
	u32 vote_atf;
	int ret;

	if (!buf) {
		pr_err("%s buf parameters is null\n", __func__);
		return HISEE_INVALID_PARAMS;
	}

	hisee_mntn_collect_vote_value_cmd();
	vote_lpm3 = hisee_mntn_get_vote_val_lpm3();
	vote_atf = hisee_mntn_get_vote_val_atf();
	ret = snprintf_s(buff, HISEE_BUF_SHOW_LEN, HISEE_BUF_SHOW_LEN - 1,
			 "lpm3 0x%08x atf 0x%08x kernel 0x%lx cos_id 0x%x\n",
			 vote_lpm3, vote_atf,
			 get_power_vote_status(), get_runtime_cos_id());
	if (ret == HISEE_SECLIB_ERROR) {
		pr_err("%s snprintf err.\n", __func__);
		return HISEE_SECUREC_ERR;
	}
	ret = memcpy_s(buf, HISEE_BUF_SHOW_LEN, buff, HISEE_BUF_SHOW_LEN);
	if (ret != EOK) {
		pr_err("%s memcpy err.\n", __func__);
		return HISEE_SECUREC_ERR;
	}
	pr_err("%s(): success.\n", __func__);
	return strlen(buf);
#else
	return 0;
#endif
}

/* only root permissions can access this sysfs node */
static DEVICE_ATTR(hisee_ioctl, (S_IRUSR | S_IWUSR),
		   hisee_ioctl_show, hisee_ioctl_store);
static DEVICE_ATTR(hisee_power, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP),
		   hisee_powerctrl_show, hisee_powerctrl_store);
static DEVICE_ATTR(hisee_apdu, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP),
		   hisee_apdu_show, hisee_apdu_store);
static DEVICE_ATTR(hisee_check_ready, (S_IRUSR | S_IRGRP),
		   hisee_check_ready_show, NULL);
static DEVICE_ATTR(hisee_has_new_cos, (S_IRUSR | S_IRGRP),
		   hisee_has_new_cos_show, NULL);
static DEVICE_ATTR(hisee_check_upgrade, (S_IRUSR | S_IRGRP),
		   hisee_check_upgrade_show, NULL);
static DEVICE_ATTR(hisee_power_debug, (S_IRUSR | S_IRGRP),
		   hisee_power_debug_show, NULL);
static DEVICE_ATTR(hisee_at_result, (S_IRUSR | S_IRGRP),
		   hisee_at_result_show, NULL);

static int hisee_basic_dev_file_create(struct platform_device *pdev)
{
	struct device *pdevice = &pdev->dev;
	int ret = HISEE_OK;

	if (device_create_file(pdevice, &dev_attr_hisee_ioctl)) {
		ret = HISEE_IOCTL_NODE_CREATE_ERROR;
		pr_err("hisee err unable to create hisee_ioctl attributes\n");
		return ret;
	}
	g_hisee_file_ioctl_ready = true;

	return ret;
}

static int hisee_basic_dev_file_remove(struct platform_device *pdev)
{
	struct device *pdevice = &pdev->dev;

	if (g_hisee_file_ioctl_ready) {
		device_remove_file(pdevice, &dev_attr_hisee_ioctl);
		g_hisee_file_ioctl_ready = false;
	}

	return HISEE_OK;
}

static int hisee_remove(struct platform_device *pdev)
{
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	if (g_hisee_api_bypassed) {
		pr_info("[%s] hisee bypass\n", __func__);
		(void)hisee_basic_dev_file_remove(pdev);
		of_reserved_mem_device_release(&pdev->dev);
		return HISEE_OK;
	}

	if (hisee_data_ptr->cma_device) {
		device_remove_file(hisee_data_ptr->cma_device,
				   &dev_attr_hisee_ioctl);
		device_remove_file(hisee_data_ptr->cma_device,
				   &dev_attr_hisee_power);
		device_remove_file(hisee_data_ptr->cma_device,
				   &dev_attr_hisee_apdu);
		device_remove_file(hisee_data_ptr->cma_device,
				   &dev_attr_hisee_check_ready);
		device_remove_file(hisee_data_ptr->cma_device,
				   &dev_attr_hisee_has_new_cos);
		device_remove_file(hisee_data_ptr->cma_device,
				   &dev_attr_hisee_check_upgrade);
		device_remove_file(hisee_data_ptr->cma_device,
				   &dev_attr_hisee_power_debug);
		device_remove_file(hisee_data_ptr->cma_device,
				   &dev_attr_hisee_at_result);
		of_reserved_mem_device_release(hisee_data_ptr->cma_device);
	}
	return HISEE_OK;
}


static void hisee_read_dts(struct device *pdevice)
{
	unsigned int dts_u32_value = 0;

	if (!pdevice) {
		pr_err("hisee read dts fail ,pdevice is null!\n");
		return;
	}

	if (of_property_read_u32(pdevice->of_node,
				 "platform_id", &dts_u32_value) == 0)
		g_platform_id = dts_u32_value;

	if (of_property_read_u32(pdevice->of_node,
				 "sm_flag_pos", &dts_u32_value) == 0) {
		g_hisee_sm_efuse_pos = dts_u32_value;
		pr_info("hisee sm_flag_pos %u\n", g_hisee_sm_efuse_pos);
	} else {
		pr_err("hisee get sm_flag_pos fail\n");
	}

	if (of_property_read_u32(pdevice->of_node,
				 "hibench_flag_pos", &dts_u32_value) == 0) {
		g_hisee_hibench_efuse_pos = dts_u32_value;
		pr_info("hisee hibench_flag_pos %u\n",
			g_hisee_hibench_efuse_pos);
	} else {
		pr_err("hisee get hibench_flag_pos fail\n");
	}

	if (of_find_property(pdevice->of_node, "pre_enable_hisee_clk", NULL)) {
		set_pre_enable_clk(true);
		pr_info("hisee need to pre enable clk\n");
	}

	if (of_property_read_u32(pdevice->of_node,
				 "cos_upgrade_time", &dts_u32_value) == 0) {
		g_hisee_cos_upgrade_time = dts_u32_value;
		pr_info("hisee cos_upgrade_time %u\n", dts_u32_value);
	} else {
		g_hisee_cos_upgrade_time = HISEE_ATF_COS_TIMEOUT;
		pr_info("hisee use default cos_upgrade_time\n");
	}

	if (of_find_property(pdevice->of_node, "is_fpga", NULL)) {
		g_hisee_is_fpga = true;
		pr_info("hisee is on fpga\n");
	} else {
		g_hisee_is_fpga = false;
		pr_info("hisee is on non-fpga\n");
	}

}

static int _probe_init_global_resources(void)
{
	struct task_struct *rpmb_ready_task = NULL;
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	hisee_data_ptr->hisee_clk = clk_get(NULL, "hise_volt_hold");
	if (IS_ERR_OR_NULL(hisee_data_ptr->hisee_clk)) {
		pr_err("hisee err clk_get failed\n");
		return HISEE_BULK_CLK_INIT_ERROR;
	}

	mutex_init(&hisee_data_ptr->hisee_mutex);
	mutex_init(&g_hisee_apdu_mutex);
	mutex_init(&g_hisee_otp_mutex);

	init_completion(&hisee_data_ptr->smc_completion);
	set_hisee_errno(HISEE_OK);
	hisee_data_ptr->factory_test_state = HISEE_FACTORY_TEST_NORUNNING;

	hisee_power_ctrl_init();
	/* used in rpmb_ready_task */
	wakeup_source_init(&hisee_data_ptr->wake_lock, "hisi-hisee");

	rpmb_ready_task = kthread_run(rpmb_ready_body, NULL, "rpmb_ready_task");
	if (!rpmb_ready_task) {
		pr_err("hisee err create rpmb_ready_task failed\n");
		return HISEE_THREAD_CREATE_ERROR;
	}
	register_flash_hisee_otp_fn(flash_hisee_debug_switches);
	return HISEE_OK;
}

/*
 * @brief      : hisee_probe_second_stage, subfunction for decreasing cyclomatic complexity
 * @param[in]  : pdevice, pointer to the platform device struct in linux
 */
static int hisee_probe_second_stage(struct platform_device *pdev)
{
	struct device *pdevice = &pdev->dev;
	int ret;

	if (device_create_file(pdevice, &dev_attr_hisee_ioctl)) {
		ret = HISEE_IOCTL_NODE_CREATE_ERROR;
		pr_err("hisee err unable to create hisee_ioctl attributes\n");
		goto err_mem_release;
	}
	if (device_create_file(pdevice, &dev_attr_hisee_power)) {
		ret = HISEE_POWER_NODE_CREATE_ERROR;
		pr_err("hisee err unable to create hisee_power attributes\n");
		goto err_device_remove_file;
	}
	if (device_create_file(pdevice, &dev_attr_hisee_apdu)) {
		ret = HISEE_IOCTL_NODE_CREATE_ERROR;
		pr_err("hisee err unable to create hisee_apdu attributes\n");
		goto err_device_remove_file1;
	}

	if (device_create_file(pdevice, &dev_attr_hisee_check_ready)) {
		ret = HISEE_POWER_NODE_CREATE_ERROR;
		pr_err("hisee err unable to create hisee_check_ready attributes\n");
		goto err_device_remove_file2;
	}
	if (device_create_file(pdevice, &dev_attr_hisee_has_new_cos)) {
		ret = HISEE_POWER_NODE_CREATE_ERROR;
		pr_err("hisee err unable to create hisee_has_new_cos attributes\n");
		goto err_device_remove_file3;
	}
	if (device_create_file(pdevice, &dev_attr_hisee_check_upgrade)) {
		ret = HISEE_POWER_NODE_CREATE_ERROR;
		pr_err("hisee err unable to create hisee_check_upgrade attributes\n");
		goto err_device_remove_file4;
	}
	if (device_create_file(pdevice, &dev_attr_hisee_power_debug)) {
		ret = HISEE_POWERDEBUG_NODE_CREATE_ERROR;
		pr_err("hisee err unable to create hisee_power_debug attributes\n");
		goto err_device_remove_file5;
	}
	if (device_create_file(pdevice, &dev_attr_hisee_at_result)) {
		ret = HISEE_AT_RESULT_NODE_CREATE_ERROR;
		pr_err("hisee err unable to create dev_attr_hisee_at_result attributes\n");
		goto err_device_remove_file6;
	}

	ret = _probe_init_global_resources();
	if (ret == HISEE_OK) {
		pr_err("hisee module init success!\n");
		return set_errno_then_exit(HISEE_OK);
	}

	device_remove_file(pdevice, &dev_attr_hisee_at_result);
err_device_remove_file6:
	device_remove_file(pdevice, &dev_attr_hisee_power_debug);
err_device_remove_file5:
	device_remove_file(pdevice, &dev_attr_hisee_check_upgrade);
err_device_remove_file4:
	device_remove_file(pdevice, &dev_attr_hisee_has_new_cos);
err_device_remove_file3:
	device_remove_file(pdevice, &dev_attr_hisee_check_ready);
err_device_remove_file2:
	device_remove_file(pdevice, &dev_attr_hisee_apdu);
err_device_remove_file1:
	device_remove_file(pdevice, &dev_attr_hisee_power);
err_device_remove_file:
	device_remove_file(pdevice, &dev_attr_hisee_ioctl);
err_mem_release:
	of_reserved_mem_device_release(get_hisee_data_ptr()->cma_device);

	pr_err("hisee module init failed!\n");
	return set_errno_then_exit(ret);
}

static int hisee_is_bypass(struct device *pdevice)
{
	unsigned int state;

	/* for miami C50: hisec bypass by dts */
	if (of_find_property(pdevice->of_node, "hisee_bypassed", NULL))
		return HISEE_TRUE;

	/* for orlando: hisec bypass by efuse */
	state = atfd_hisee_smc((u64)HISEE_FN_MAIN_SERVICE_CMD, (u64)CMD_GET_STATE, (u64)0, (u64)0);

	return (state == HISEE_BYPASS) ? HISEE_TRUE : HISEE_FALSE;
}

static int __init hisee_probe(struct platform_device *pdev)
{
	struct device *pdevice = &pdev->dev;
	int ret;

	(void)memset_s((void *)&g_hisee_data, sizeof(g_hisee_data),
		       0, sizeof(g_hisee_data));
	ret = of_reserved_mem_device_init(pdevice);
	if (ret != HISEE_OK) {
		pr_err("hisee shared cma pool with ATF registered failed!\n");
		return set_errno_then_exit(HISEE_CMA_DEVICE_INIT_ERROR);
	}

	if (hisee_is_bypass(pdevice)) {
		pr_err("%s(): hisee is bypass!\n", __func__);
		g_hisee_api_bypassed = true;
#ifdef CONFIG_HISEE_SUPPORT_BYPASS_IMG
		ret = start_erase_hisee_image();
		if (ret != HISEE_OK)
			pr_err("erase_img_task create failed\n");
#endif
		return hisee_basic_dev_file_create(pdev);
	}

	ret = set_lpmcu_nfc_irq();
	if (ret != HISEE_OK)
		pr_err("hisee send cmd to lpmcu failed. ignored\n");

	g_hisee_data.cma_device = pdevice;
	pr_err("hisee shared cma pool with ATF registered success!\n");

	hisee_read_dts(pdevice);

	/*
	 * enter hisee probe secnod stage,the reason is because
	 * the hisee_probe() is big complexity.
	 */
	return hisee_probe_second_stage(pdev);
}

/*
 * @brief      : check hisee api is bypassed on specified platform
 * @return     : ::bool, true indicates API be byppased
 */
bool hisee_api_is_bypassed(void)
{
	return g_hisee_api_bypassed;
}

static const struct of_device_id g_hisee_of_match[] = {
	{ .compatible = "hisilicon,hisee-device" },
	{ }
};
MODULE_DEVICE_TABLE(of, g_hisee_of_match);

static struct platform_driver g_hisee_driver = {
	.probe = hisee_probe,
	.remove = hisee_remove,
	.suspend = hisee_suspend,
	.driver = {
		.name  = HISEE_DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_hisee_of_match),
	},
};

static int __init hisee_module_init(void)
{
	int ret;

	ret = platform_driver_register(&g_hisee_driver);
	if (ret)
		pr_err("register hisee driver failed.\n");

	return ret;
}

static void __exit hisee_module_exit(void)
{
	platform_driver_unregister(&g_hisee_driver);
}
module_init(hisee_module_init);
module_exit(hisee_module_exit);

MODULE_AUTHOR("<lvtaolong@hisilicon.com>");
MODULE_DESCRIPTION("HISEE KERNEL DRIVER");
MODULE_LICENSE("GPL v2");
