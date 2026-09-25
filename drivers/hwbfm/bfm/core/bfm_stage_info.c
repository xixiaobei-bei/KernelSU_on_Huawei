/*
 * bfm_stage_info.c
 *
 * define the core's external public enum/macros/interface for
 * BSP(Boot Fail Measure)
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <chipset_common/bfmr/bfm/core/bfm_stage_info.h>
#include <chipset_common/bfmr/bfm/chipsets/bfm_chipsets.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/moduleparam.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#include <linux/reboot.h>
#include <linux/dirent.h>
#include <linux/statfs.h>

#define BFM_BOOTUP_STAGE_INFO_FORMAT \
		"start:%12s_%s(%ds)  end:%12s_%s(%ds)  elapse:%ds\r\n"

#define TITLE_BFMR_BOOTUP_STAGE_INFO "booup stage info"

struct bfm_boot_stage_no_desc {
	enum bfmr_detail_stage boot_stage;
	char *desc;
};

static char *bfm_get_boot_stage_no_desc(enum bfmr_detail_stage boot_stage);
static size_t bfm_format_stages_info(char *bs_data,
		unsigned int bs_data_buffer_len);

static DEFINE_MUTEX(record_stage_mutex);

/* if you need bootstage description before native stage, modify this array */
static struct bfm_boot_stage_no_desc boot_stage_desc[] = {
	{ STAGE_INIT_START,          "stage_init_start" },
	{ STAGE_RESIZE_FS_START,     "stage_resize_fs_start" },
	{ STAGE_RESIZE_FS_END,       "stage_resize_fs_end" },
	{ STAGE_CHECK_FS_START,      "stage_check_fs_start" },
	{ STAGE_CHECK_FS_END,        "stage_check_fs_end" },
	{ STAGE_ON_EARLY_INIT,       "stage_on_early_init" },
	{ STAGE_ON_INIT,             "stage_on_init" },
	{ STAGE_ON_EARLY_FS,         "stage_on_early_fs" },
	{ STAGE_ON_FS,               "stage_on_fs" },
	{ STAGE_ON_POST_FS,          "stage_on_post_fs" },
	{ STAGE_ON_POST_FS_DATA,     "stage_on_post_fs_data" },
	{ STAGE_RESTORECON_START,    "stage_restorecon_start" },
	{ STAGE_RESTORECON_END,      "stage_restorecon_end" },
	{ STAGE_ON_EARLY_BOOT,       "stage_on_early_boot" },
	{ STAGE_ON_BOOT,             "stage_on_boot" },
	{ STAGE_ZYGOTE_START,        "stage_zygote_start" },
	{ STAGE_VM_START,            "stage_vm_start" },
	{ STAGE_PHASE_WAIT_FOR_DEFAULT_DISPLAY,
	  "stage_phase_wait_for_default_display" },
	{ STAGE_PHASE_LOCK_SETTINGS_READY,
	  "stage_phase_lock_settings_ready" },
	{ STAGE_PHASE_SYSTEM_SERVICES_READY,
	  "stage_phase_system_services_ready" },
	{ STAGE_PHASE_ACTIVITY_MANAGER_READY,
	  "stage_phase_activity_manager_ready" },
	{ STAGE_PHASE_THIRD_PARTY_APPS_CAN_START,
	  "stage_phase_third_party_apps_can_start" },
	{ STAGE_FRAMEWORK_JAR_DEXOPT_START,
	  "stage_framework_jar_dexopt_start" },
	{ STAGE_FRAMEWORK_JAR_DEXOPT_END,
	  "stage_framework_jar_dexopt_end" },
	{ STAGE_APP_DEXOPT_START,     "stage_app_dexopt_start" },
	{ STAGE_APP_DEXOPT_END,       "stage_app_dexopt_end" },
	{ STAGE_PHASE_BOOT_COMPLETED, "stage_phase_boot_completed" },
	{ STAGE_SYSTEM_SERVER_DEX2OAT_START,
	  "stage_system_server_dex2oat_start" },
	{ STAGE_SYSTEM_SERVER_DEX2OAT_END,  "stage_system_server_dex2oat_end" },
	{ STAGE_SYSTEM_SERVER_INIT_START,   "stage_system_server_init_start" },
	{ STAGE_SYSTEM_SERVER_INIT_END,     "stage_system_server_init_end" },
	{ STAGE_BOOT_SUCCESS,               "stage_boot_success" },
};

static struct bfm_stages *stages_info;

static char *bfm_get_boot_stage_no_desc(enum bfmr_detail_stage boot_stage)
{
	int count = ARRAY_SIZE(boot_stage_desc);
	int i;

	for (i = 0; i < count; i++) {
		if (boot_stage == boot_stage_desc[i].boot_stage)
			return boot_stage_desc[i].desc;
	}
	return "unknown_stage";
}

/*
 * @function: bfmr_save_stages_info_txt
 * @brief: append the stages info into bs_data
 * @param: pdata[inout], the buffer to record information
 * @param: pdata_size[in], the size of bs_data
 * @return:the length to need print
 */
size_t bfm_save_stages_info_txt(char *buffer, unsigned int buffer_len)
{
	size_t bytes_print;

	if ((buffer == NULL) || (buffer_len <= 0)) {
		BFMR_PRINT_ERR("Buffer is not valid!\n");
		return 0;
	}

	bytes_print = bfm_format_stages_info(buffer, buffer_len);
	if (bytes_print <= 0)
		BFMR_PRINT_ERR("no any stage info to bootfail_info.txt!\n");

	bfm_deinit_stage_info();

	return bytes_print;
}
EXPORT_SYMBOL(bfm_save_stages_info_txt);

static size_t bfm_format_stages_info(char *bs_data,
			unsigned int bs_data_buffer_len)
{
	size_t bs_data_size;
	size_t bytes_record = 0;
	unsigned int stage_index = 0;
	struct bfm_stage_info *pre_stage_node = NULL;
	struct bfm_stage_info *cur_stage_node = NULL;

	mutex_lock(&record_stage_mutex);
	if (stages_info == NULL) {
		BFMR_PRINT_ERR("stage info not initialized!\n");
		mutex_unlock(&record_stage_mutex);
		return 0;
	}
	if (stages_info->stage_count == 0) {
		BFMR_PRINT_ERR("not record bootup stage info\n");
		mutex_unlock(&record_stage_mutex);
		return 0;
	}

	// set boot stage info title
	bs_data_size = snprintf(bs_data, bs_data_buffer_len - 1,
				"\r\n%s:\r\n", TITLE_BFMR_BOOTUP_STAGE_INFO);
	for (; stage_index < stages_info->stage_count;
				stage_index++) {
		pre_stage_node = cur_stage_node;
		cur_stage_node = &(stages_info->stage_info_list[stage_index]);
		if ((pre_stage_node == NULL) || (cur_stage_node == NULL))
			continue;
		if (bs_data_size >= bs_data_buffer_len - 1) {
			bs_data_size = bs_data_buffer_len - 1;
			bs_data[bs_data_buffer_len - 1] = '\0';
			BFMR_PRINT_ERR("string truncation ouccured,bs_data_size:(%lu)\n",
					bs_data_size);
			break;
		}
		bytes_record = snprintf(bs_data + bs_data_size,
			bs_data_buffer_len - bs_data_size - 1,
			BFM_BOOTUP_STAGE_INFO_FORMAT,
			bfm_get_boot_stage_name(pre_stage_node->stage),
			bfm_get_boot_stage_no_desc(pre_stage_node->stage),
			pre_stage_node->start_time,
			bfm_get_boot_stage_name(cur_stage_node->stage),
			bfm_get_boot_stage_no_desc(cur_stage_node->stage),
			cur_stage_node->start_time,
			(cur_stage_node->start_time -
				pre_stage_node->start_time));

		bs_data_size += bytes_record;
	}

	mutex_unlock(&record_stage_mutex);
	return bs_data_size;
}

/*
 * @function: bfm_add_stage_info
 * @brief: record the stage information into list
 * @param: pStage[in], the stageinfo pointer
 */
void bfm_add_stage_info(struct bfm_stage_info *pstage)
{
	int cur_stage_index;

	mutex_lock(&record_stage_mutex);
	if (pstage == NULL) {
		BFMR_PRINT_ERR("pstage is null!\n");
		mutex_unlock(&record_stage_mutex);
		return;
	}
	if (stages_info == NULL) {
		BFMR_PRINT_ERR("stages_info is null!\n");
		mutex_unlock(&record_stage_mutex);
		return;
	}
	cur_stage_index = stages_info->stage_count;
	if (cur_stage_index < BMFR_MAX_BOOTUP_STAGE_COUNT) {
		struct bfm_stage_info *current_stage_node = (struct bfm_stage_info *) &
			(stages_info->stage_info_list[cur_stage_index]);
		current_stage_node->start_time = pstage->start_time;
		current_stage_node->stage = pstage->stage;

		stages_info->stage_count++;
	} else {
		BFMR_PRINT_KEY_INFO("the stages info stack is full\n");
	}

	mutex_unlock(&record_stage_mutex);
}
EXPORT_SYMBOL(bfm_add_stage_info);

/*
 * @function: int bfm_init_stage_info(void)
 * @brief: init trace time module resource
 * @return: 0 - succeeded; -1 - failed.
 * @note: it need be initialized in kernel.
 */
int bfm_init_stage_info(void)
{
	mutex_lock(&record_stage_mutex);

	if (stages_info == NULL) {
		stages_info = bfmr_malloc(sizeof(struct bfm_stages));
		if (stages_info == NULL) {
			BFMR_PRINT_KEY_INFO("malloc stages_info failed\n");
			mutex_unlock(&record_stage_mutex);
			return -1;
		}
		memset((void *)stages_info, 0, sizeof(struct bfm_stages));
	}

	mutex_unlock(&record_stage_mutex);
	return 0;
}
EXPORT_SYMBOL(bfm_init_stage_info);

/*
 * @function: void bfm_deinit_stage_info(void)
 * @brief: release trace time module resource
 * @note: it need be called after bootsuccess and saved the buffer into txt.
 */
void bfm_deinit_stage_info(void)
{
	mutex_lock(&record_stage_mutex);

	if (stages_info != NULL) {
		bfmr_free(stages_info);
		stages_info = NULL;
	}

	mutex_unlock(&record_stage_mutex);
}
EXPORT_SYMBOL(bfm_deinit_stage_info);
