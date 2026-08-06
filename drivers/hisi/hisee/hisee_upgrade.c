/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: implement hisee image upgrade function
 * Create: 2020-02-17
 */
#include "hisee_upgrade.h"
#include <asm/compiler.h>
#include <linux/atomic.h>
#include <linux/clk.h>
#include <linux/compiler.h>
#include <linux/completion.h>
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
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/notifier.h>
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
#ifdef CONFIG_HISEE_MNTN
#include "hisee_mntn.h"
#endif
#include "hisee_chip_test.h"
#include "hisee_power.h"
#include "soc_acpu_baseaddr_interface.h"
#include "soc_sctrl_interface.h"

#define TIMESTAMP_YEAR_MIN        2016
#define TIMESTAMP_YEAR_MAX        2050 /* can not be equal to TIMESTAMP_YEAR_MAX. */
#define TIMESTAMP_MONTH_PER_YEAR  12
#define TIMESTAMP_DAY_MAX         31 /* max days in a month */
#define TIMESTAMP_HOUR_MAX        24 /* max hours in a day */
#define TIMESTAMP_MINUTE_MAX      60 /* max minutes in a hour */
#define TIMESTAMP_SECOND_MAX      60 /* max seconds in a minute */

static int check_sw_version_null(const struct cosimage_version_info *info,
				 unsigned int cos_id)
{
	if (!info->magic && !info->img_version_num[cos_id] &&
	    !info->img_timestamp.value)
		return HISEE_TRUE;
	else
		return HISEE_FALSE;
}

static int check_timestamp_valid(const union timestamp_info *timestamp_value)
{
	/* the beginning year of the feature life */
	if (timestamp_value->timestamp.year < TIMESTAMP_YEAR_MIN)
		return HISEE_FALSE;

	/* the deadline of the feature life */
	if (timestamp_value->timestamp.year >= TIMESTAMP_YEAR_MAX)
		return HISEE_FALSE;

	if (timestamp_value->timestamp.month > TIMESTAMP_MONTH_PER_YEAR)
		return HISEE_FALSE;

	/*
	 * the judge is not accurate, because not all month has 31 day,
	 * depend on the value of year and month
	 */
	if (timestamp_value->timestamp.day > TIMESTAMP_DAY_MAX)
		return HISEE_FALSE;

	if (timestamp_value->timestamp.hour >= TIMESTAMP_HOUR_MAX)
		return HISEE_FALSE;

	if (timestamp_value->timestamp.minute >= TIMESTAMP_MINUTE_MAX)
		return HISEE_FALSE;

	if (timestamp_value->timestamp.second >= TIMESTAMP_SECOND_MAX)
		return HISEE_FALSE;

	return HISEE_TRUE;
}

/*
 * @brief      : according software version number and timestamp in cos image,
 *               do first check the cos image in hisee.img whether is the
 *               newer image compared with cos image has flashed in board.
 * @param[in]  : cos_id, the current cos image id
 * @param[in]  : curr_ptr, current pointer to struct cosimage_version_info
 * @param[in]  : prev_ptr, previos pointer to struct cosimage_version_info
 * @return     : ::int HISEE_TRUE on true, HISEE_FALSE on false
 */
static int first_check_newest_cosimage(unsigned int cos_id,
				       const struct cosimage_version_info *curr_ptr,
				       const struct cosimage_version_info *prev_ptr)
{
	if (check_timestamp_valid(&curr_ptr->img_timestamp) == HISEE_FALSE)
		return HISEE_FALSE;

	if (check_timestamp_valid(&prev_ptr->img_timestamp) == HISEE_FALSE)
		return HISEE_FALSE;

	if (curr_ptr->magic != HISEE_SW_VERSION_MAGIC_VALUE ||
	    prev_ptr->magic != HISEE_SW_VERSION_MAGIC_VALUE)
		return HISEE_FALSE;

	/* compare cos iamge NV counter */
	if (curr_ptr->img_version_num[cos_id] >
	    prev_ptr->img_version_num[cos_id])
		return HISEE_TRUE;
	if (curr_ptr->img_version_num[cos_id] <
	    prev_ptr->img_version_num[cos_id])
		return HISEE_FALSE;

	/* if cos image NV counter are equal, then compare timestamp */
	if (curr_ptr->img_timestamp.value > prev_ptr->img_timestamp.value)
		return HISEE_TRUE;
	else
		return HISEE_FALSE;
}


int check_new_cosimage(unsigned int cos_id, int *is_new_cosimage)
{
	struct hisee_img_header local_img_header;
	struct cosimage_version_info curr = {0};
	struct cosimage_version_info previous = {0};
	int ret;

	if (!is_new_cosimage) {
		pr_err("%s buf parameters is null\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}
	(void)memset_s((void *)&local_img_header,
		       sizeof(struct hisee_img_header), 0,
		       sizeof(struct hisee_img_header));
	ret = hisee_parse_img_header((char *)&local_img_header);
	if (ret != HISEE_OK) {
		pr_err("%s():hisee_parse_img_header failed, ret=%d\n",
		       __func__, ret);
		return set_errno_then_exit(ret);
	}
	parse_timestamp(local_img_header.time_stamp, &curr.img_timestamp);
	curr.img_version_num[cos_id] =
		(unsigned char)local_img_header.sw_version_cnt[cos_id];
	curr.magic = HISEE_SW_VERSION_MAGIC_VALUE;

	ret = access_hisee_image_partition((char *)&previous,
					   SW_VERSION_READ_TYPE);
	if (ret != HISEE_OK) {
		pr_err("%s access_hisee_image_partition fail,ret=%d\n",
		       __func__, ret);
		return ret;
	}


	if (check_sw_version_null(&previous, cos_id)) {
		*is_new_cosimage = HISEE_TRUE;
	} else {
		*is_new_cosimage = first_check_newest_cosimage(
					cos_id,
					&curr, &previous);
	}
	return HISEE_OK;
}

static int _misc_image_upgrade(unsigned int cos_id,
			       char *buff_virt, phys_addr_t buff_phy)
{
	struct atf_message_header *p_message_header = NULL;
	unsigned int image_size;
	unsigned int result_offset;
	enum hisee_img_file_type type;
	unsigned int misc_image_cnt;
	unsigned int misc_id = cos_id;
	int ret = HISEE_OK;
	const unsigned int max_misc_num = HISEE_MAX_MISC_IMAGE_NUMBER;

	type = MISC0_IMG_TYPE + misc_id * max_misc_num;
	misc_image_cnt = get_hisee_data_ptr()->hisee_img_head.misc_image_cnt[misc_id];
	pr_err("%s(): cos_id=%u,misc_image_cnt=%u\n",
	       __func__, cos_id, misc_image_cnt);

	p_message_header = (struct atf_message_header *)buff_virt;
	if (misc_image_cnt > HISEE_MAX_MISC_IMAGE_NUMBER)
		return HISEE_COS_IMG_ID_ERROR;
	for (misc_id = 0; misc_id < misc_image_cnt; misc_id++) {
		(void)memset_s(buff_virt, HISEE_SHARE_BUFF_SIZE, 0,
			       HISEE_SHARE_BUFF_SIZE);
		set_message_header(p_message_header, CMD_UPGRADE_MISC);
		ret = filesys_hisee_read_image(
				type,
				buff_virt + HISEE_ATF_MESSAGE_HEADER_LEN,
				HISEE_SHARE_BUFF_SIZE - HISEE_ATF_MESSAGE_HEADER_LEN);
		if (ret < HISEE_OK) {
			pr_err("%s(): filesys_hisee_read_image failed, ret=%d\n",
			       __func__, ret);
			return ret;
		}

		image_size = (u32)(ret + HISEE_ATF_MESSAGE_HEADER_LEN);
		result_offset = (image_size + SMC_TEST_RESULT_SIZE - 1) &
				(~(SMC_TEST_RESULT_SIZE - 1));
		if (result_offset + SMC_TEST_RESULT_SIZE <= HISEE_SHARE_BUFF_SIZE) {
			p_message_header->test_result_phy =
					(unsigned int)buff_phy + result_offset;
			p_message_header->test_result_size = SMC_TEST_RESULT_SIZE;
		}
		ret = send_smc_process(p_message_header, buff_phy, image_size,
				       HISEE_ATF_MISC_TIMEOUT, CMD_UPGRADE_MISC);
		if (ret != HISEE_OK) {
			pr_err("%s(): send_smc_process failed, ret=%d\n",
			       __func__, ret);
			return ret;
		}
		type++;
	}

	return ret;
}

int misc_image_upgrade_func(unsigned int cos_id)
{
	char *buff_virt = NULL;
	phys_addr_t buff_phy = 0;
	int ret;
	struct hisee_module_data *hisee_data_ptr = NULL;

	if (cos_id >= HISEE_SUPPORT_COS_FILE_NUMBER) {
		pr_err("%s(): cos_id=%u invalid\n", __func__, cos_id);
		return set_errno_then_exit(HISEE_NO_RESOURCES);
	}
	hisee_data_ptr = get_hisee_data_ptr();
	buff_virt = (char *)dma_alloc_coherent(
				hisee_data_ptr->cma_device,
				HISEE_SHARE_BUFF_SIZE, &buff_phy, GFP_KERNEL);
	if (!buff_virt) {
		pr_err("%s(): dma_alloc_coherent failed\n", __func__);
		return set_errno_then_exit(HISEE_NO_RESOURCES);
	}

	ret = _misc_image_upgrade(cos_id, buff_virt, buff_phy);
	check_and_print_result();
	dma_free_coherent(hisee_data_ptr->cma_device,
			  (size_t)HISEE_SHARE_BUFF_SIZE,
			  buff_virt, buff_phy);
	return set_errno_then_exit(ret);
}

static int cos_upgrade_prepare(const void *buf,
			       enum hisee_img_file_type *img_type,
			       unsigned int *cos_id)
{
	int ret;
	unsigned int process_id = 0;

	if (!img_type || !cos_id) {
		pr_err("%s(): input params invalid", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	if (get_rpmb_key_status() == KEY_NOT_READY) {
		pr_err("%s(): rpmb key not ready. cos upgrade bypassed",
		       __func__);
		return set_errno_then_exit(HISEE_RPMB_KEY_UNREADY_ERROR);
	}
	ret = hisee_get_cosid_processid(buf, cos_id, &process_id);
	if (ret != HISEE_OK)
		return set_errno_then_exit(ret);

	*img_type = (*cos_id - COS_IMG_ID_0) + COS_IMG_TYPE;
	if (*img_type >= OTP_IMG_TYPE) {
		pr_err("%s(): input cos_id error(%u)", __func__, *cos_id);
		ret = HISEE_COS_IMG_ID_ERROR;
		return set_errno_then_exit(ret);
	}

	if (get_used_cos_id() != *cos_id) {
		pr_err("%s(): input cos_id(%u) is diff from poweron upgrade(%u)",
		       __func__, *cos_id, get_used_cos_id());
		ret = HISEE_COS_IMG_ID_ERROR;
		return set_errno_then_exit(ret);
	}

	return HISEE_OK;
}

static int cos_upgrade_check_version(int para, unsigned int cos_id)
{
	int ret = HISEE_OK;
	int new_cos_exist = HISEE_FALSE;

	/*
	 * hisee factory test(include slt test)
	 * don't check there is new cos image
	 */
	if (para != (int)HISEE_FACTORY_TEST_VERSION) {
		ret = check_new_cosimage(cos_id, &new_cos_exist);
		if (ret == HISEE_OK) {
			if (new_cos_exist == HISEE_FALSE) {
				pr_err("%s(): there is no new cosimage\n",
				       __func__);
				ret = HISEE_IS_OLD_COS_IMAGE;
			}
		} else {
			pr_err("%s(): check_new_cosimage failed,ret=%d\n",
			       __func__, ret);
			ret = HISEE_OLD_COS_IMAGE_ERROR;
		}
	}

	return ret;
}

#ifdef CONFIG_HICOS_MISCIMG_PATCH
atomic_t g_is_patch_free = ATOMIC_INIT(0);
char *g_patch_buff_virt;
phys_addr_t g_patch_buff_phy;
int hisee_cos_patch_read(enum hisee_img_file_type img_type)
{
	int ret = HISEE_OK;
	char *buff_virt = NULL;
	phys_addr_t buff_phy = 0;
	unsigned int image_size;
	unsigned int timeout;
	struct atf_message_header *p_message_header = NULL;
	unsigned int is_smx_0 = 0;
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	hisee_get_smx_cfg(&is_smx_0);

	if (is_smx_0 != SMX_PROCESS_0) {
		pr_err("%s(): enter, img_type=%u.\n", __func__, img_type);
		if (atomic_inc_return(&g_is_patch_free) !=
		    HISEE_COS_PATCH_FREE_CNT) {
			atomic_dec(&g_is_patch_free);
			ret = HISEE_ERROR;
			return set_errno_then_exit(ret);
		}

		if (!g_patch_buff_virt) {
			pr_err("%s(): alloc HISEE_SHARE_BUFF_SIZE\n", __func__);
			buff_virt = (char *)dma_alloc_coherent(
					hisee_data_ptr->cma_device,
					HISEE_SHARE_BUFF_SIZE,
					&buff_phy, GFP_KERNEL);
			g_patch_buff_virt = buff_virt;
			g_patch_buff_phy = buff_phy;
		} else {
			buff_virt = g_patch_buff_virt;
			buff_phy = g_patch_buff_phy;
		}
		if (!buff_virt) {
			pr_err("%s(): dma_alloc_coherent failed\n", __func__);
			atomic_dec(&g_is_patch_free);
			ret = HISEE_NO_RESOURCES;
			return set_errno_then_exit(ret);
		}

		(void)memset_s(buff_virt, HISEE_SHARE_BUFF_SIZE,
			       0, HISEE_SHARE_BUFF_SIZE);
		p_message_header = (struct atf_message_header *)buff_virt;
		set_message_header(p_message_header, CMD_UPGRADE_COS_PATCH);
		ret = filesys_hisee_read_image(
				img_type,
				buff_virt + HISEE_ATF_MESSAGE_HEADER_LEN,
				HISEE_SHARE_BUFF_SIZE -
				HISEE_ATF_MESSAGE_HEADER_LEN);
		if (ret < HISEE_OK) {
			pr_err("%s(): filesys_hisee_read_image failed, ret=%d\n",
			       __func__, ret);
			dma_free_coherent(hisee_data_ptr->cma_device,
					  (size_t)HISEE_SHARE_BUFF_SIZE,
					  g_patch_buff_virt, g_patch_buff_phy);
			g_patch_buff_virt = NULL;
			g_patch_buff_phy = 0;
			atomic_dec(&g_is_patch_free);
			return set_errno_then_exit(ret);
		}

		image_size = (unsigned int)(ret + HISEE_ATF_MESSAGE_HEADER_LEN);
		timeout = (unsigned int)HISEE_ATF_COS_TIMEOUT;

		pr_err("%s(): send_smc_process-->CMD_UPGRADE_COS_PATCH\n",
		       __func__);
		ret = send_smc_process(p_message_header, buff_phy, image_size,
				       timeout, CMD_UPGRADE_COS_PATCH);

		/* free is in hisee_check_ready_show() */
		atomic_dec(&g_is_patch_free);
		pr_err("%s(): exit, img_type=%u.\n", __func__, img_type);
	}
	return ret;
}

static void cos_patch_upgrade(const void *buf)
{
	int ret = HISEE_OK;
	unsigned int cos_id = COS_IMG_ID_0;
	unsigned int process_id = 0;
	unsigned int is_smx_0 = 0;

	hisee_get_smx_cfg(&is_smx_0);

	if (is_smx_0 != SMX_PROCESS_0) {
		ret = hisee_get_cosid_processid(buf, &cos_id, &process_id);
		if (ret != HISEE_OK) {
			pr_err("%s(): hisee_get_cosid_processid failed\n",
			       __func__);
			return;
		}

		if (cos_id != COS_IMG_ID_0) {
			/* can do more action in future if necessary */
			pr_err("%s(): cos_id=%u bypass!!\n",
			       __func__, cos_id);
			return;
		}

		ret = hisee_poweroff_func(buf, HISEE_PWROFF_LOCK);
		if (ret != HISEE_OK)
			pr_err("%s() hisee_poweroff_func failed. ret=%d\n",
			       __func__, ret);

		ret = hisee_cos_patch_read(MISC3_IMG_TYPE);
		if (ret != HISEE_OK)
			pr_err("%s(): hisee_cos_patch_read failed ret=%x\n",
			       __func__, ret);

		ret = hisee_poweron_booting_func(buf, 0);
		if (ret != HISEE_OK)
			pr_err("%s(): hisee_poweron_booting_func failed ret=%x\n",
			       __func__, ret);

		/* wait hisee cos ready for later process */
		ret = wait_hisee_ready(HISEE_STATE_COS_READY,
				       HISEE_ATF_GENERAL_TIMEOUT);
		if (ret != HISEE_OK)
			pr_err("%s(): wait_hisee_ready failed ret=%x\n",
			       __func__, ret);
	}
	check_and_print_result_with_cosid();
}
#endif

int _check_cos_ready_show(char *result_buff, size_t len)
{
	int ret;
	size_t remaining_len;

	if (!result_buff || len > HISEE_BUF_SHOW_LEN)
		return HISEE_ERROR;

	/* size is 3 */
	ret = snprintf_s(result_buff, len, (u64)3,
			 "%d,", HISEE_COS_READY_SHOW_VALUE);
	if (ret == HISEE_SECLIB_ERROR) {
		pr_err("%s(): snprintf1 err.\n", __func__);
		return HISEE_SECUREC_ERR;
	}
	remaining_len = strlen("cos ready");
	ret = strncat_s(result_buff, len, "cos ready", remaining_len);
	if (ret != EOK) {
		pr_err("%s(): strncat err.\n", __func__);
		return HISEE_SECUREC_ERR;
	}

#ifdef CONFIG_HICOS_MISCIMG_PATCH
	/* free memory alloc in hisee_cos_patch_read only once. */
	if (atomic_inc_return(&g_is_patch_free) == HISEE_COS_PATCH_FREE_CNT) {
		if (g_patch_buff_virt) {
			pr_err("%s free  COS_PATCH_BUFF\n", __func__);
			dma_free_coherent(get_hisee_data_ptr()->cma_device,
					  (unsigned long)HISEE_SHARE_BUFF_SIZE,
					  g_patch_buff_virt,
					  g_patch_buff_phy);
			g_patch_buff_virt = NULL;
			g_patch_buff_phy = 0;
		}
	}
	atomic_dec(&g_is_patch_free);
#endif
	return HISEE_OK;
}

/*
 * @brief      : read the upgrade hisee iamge from hisee_img partition and
 *               send to atf.
 * @param[in]  : cos_id, the cos index correspond to the image.
 * @param[in]  : img_type, the image type, like cos, otp, misc, uloader...
 * @param[in]  : smc_cmd, the smc command type, like COS_UPGRADE,
 *               ULOADER_UPGRADE...
 * @return     : ::int 0 on success, other value on failure
 */
int hisee_upgrade_image_read(unsigned int cos_id, enum hisee_img_file_type img_type,
			     enum se_smc_cmd smc_cmd)
{
	int ret;
	char *buff_virt = NULL;
	phys_addr_t buff_phy = 0;
	unsigned int image_size = 0;
	struct atf_message_header *p_message_header = NULL;
	int need_read_img = HISEE_TRUE;
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	buff_virt = (char *)dma_alloc_coherent(hisee_data_ptr->cma_device,
					       HISEE_SHARE_BUFF_SIZE,
					       &buff_phy, GFP_KERNEL);
	if (!buff_virt) {
		pr_err("%s(): dma_alloc_coherent failed\n", __func__);
		ret = HISEE_NO_RESOURCES;
		return set_errno_then_exit(ret);
	}

	(void)memset_s(buff_virt, HISEE_SHARE_BUFF_SIZE, 0,
		       HISEE_SHARE_BUFF_SIZE);
	p_message_header = (struct atf_message_header *)buff_virt;
	set_message_header(p_message_header, smc_cmd);
	if (need_read_img == HISEE_TRUE) {
		ret = filesys_hisee_read_image(img_type, (buff_virt +
					       HISEE_ATF_MESSAGE_HEADER_LEN),
					       HISEE_SHARE_BUFF_SIZE -
					       HISEE_ATF_MESSAGE_HEADER_LEN);
		if (ret < HISEE_OK) {
			pr_err("%s(): hisee_read_image failed, ret=%d\n",
			       __func__, ret);
			goto exit;
		}
		image_size = (unsigned int)(ret + HISEE_ATF_MESSAGE_HEADER_LEN);
	}
	p_message_header->ack = cos_id;
	ret = send_smc_process(p_message_header, buff_phy, image_size,
			       get_cos_upgrade_time(), smc_cmd);

exit:
	dma_free_coherent(hisee_data_ptr->cma_device, HISEE_SHARE_BUFF_SIZE,
			  buff_virt, buff_phy);
	check_and_print_result();
	return ret;
}

/*
 * @brief      : cos_upgrade_image_read
 * @param[in]  : cos_id , the cos index
 * @param[in]  : img_type , the image file type, like cos0,cos1...
 * @return     : ::int 0 on success, other value on failure
 */
int cos_upgrade_image_read(unsigned int cos_id, enum hisee_img_file_type img_type)
{
	int ret;
	enum se_smc_cmd smc_cmd;

	pr_err("%s: cos_id=%x, img_type=%x\n", __func__, cos_id, img_type);
	/* Do the cos image upgrade. */
	smc_cmd = CMD_UPGRADE_COS;
	ret = hisee_upgrade_image_read(cos_id, img_type, smc_cmd);
	check_result_and_goto(ret, out);

out:
	check_and_print_result();
	return ret;
}

static int cos_upgrade_basic_check_param(unsigned int cos_id,
					 enum hisee_img_file_type img_type)
{
	if (img_type > OTP_IMG_TYPE || cos_id >= MAX_COS_IMG_ID) {
		pr_err("hisee:%s(): params is invalid\n", __func__);
		return HISEE_COS_IMG_ID_ERROR;
	}

	return HISEE_OK;
}

static int _cos_image_upgrade_success(const void *buf, int para,
				      unsigned int cos_id)
{
	int  ret;
	unsigned int upgrade_run_flg = HISEE_COS_UPGRADE_FINISH_FLG;
	struct cosimage_version_info curr = {0};
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	(void)access_hisee_image_partition((char *)&curr, SW_VERSION_READ_TYPE);
	parse_timestamp(hisee_data_ptr->hisee_img_head.time_stamp,
			&curr.img_timestamp);
	if (check_timestamp_valid(&curr.img_timestamp) == HISEE_FALSE) {
		ret = HISEE_INVALID_PARAMS;
		pr_err("%s(): check_timestamp_valid failed\n", __func__);
		return ret;
	}

	curr.img_version_num[cos_id] =
			hisee_data_ptr->hisee_img_head.sw_version_cnt[cos_id];

	curr.magic = HISEE_SW_VERSION_MAGIC_VALUE;
	access_hisee_image_partition((char *)&curr, SW_VERSION_WRITE_TYPE);
	access_hisee_image_partition((char *)&upgrade_run_flg,
				     COS_UPGRADE_RUN_WRITE_TYPE);


	hisee_mntn_update_local_ver_info();
	pr_err("hisee:%s(): upgrade_exit,cos_id=%u\n", __func__, cos_id);

#ifdef CONFIG_HICOS_MISCIMG_PATCH
	if (cos_id == COS_IMG_ID_0 && para != HISEE_FACTORY_TEST_VERSION)
		cos_patch_upgrade(buf);
#endif
	return HISEE_OK;
}

/*
 * @brief      : the basic cos image upgrade process with cos_id. If success,
 *               update the flag region in hisee_img partition, otherwise, retry
 *               this process to improve the Robustness.
 * @param[in]  : buf, the content string buffer
 * @param[in]  : para, parameters
 * @param[in]  : cos_id, the current cos image id
 * @param[in]  : img_type, the image file type, like cos0,cos1...
 * @return     : ::int 0 on success, other value on failure
 */
static int cos_image_upgrade_basic_process(const void *buf, int para,
					   unsigned int cos_id,
					   enum hisee_img_file_type img_type)
{
	int ret, ret1, ret2;
	int retry = 2; /* retry 2 more times if failed */
	unsigned int upgrade_run_flg = HISEE_COS_UPGRADE_RUNNING_FLG;

	ret = cos_upgrade_basic_check_param(cos_id, img_type);
	if (ret != HISEE_OK)
		return ret;

	access_hisee_image_partition((char *)&upgrade_run_flg,
				     COS_UPGRADE_RUN_WRITE_TYPE);
upgrade_retry:
	ret = cos_upgrade_image_read(cos_id, img_type);
	if (ret == HISEE_OK) {
		ret = _cos_image_upgrade_success(buf, para, cos_id);
	} else {
		get_hisee_data_ptr()->hisee_img_head.sw_version_cnt[cos_id] = 0;
		ret1 = mspc_exception_to_reset_rpmb();
		if (ret1 != HISEE_OK) {
			pr_err("%s ERROR:fail to reset rpmb,cos_id=%u,ret=%d\n",
			       __func__, cos_id, ret1);
			ret1 = get_hisee_errno();
#ifdef CONFIG_HISEE_MNTN
			rdr_hisee_call_to_record_exc(ret1);
#endif
			return ret1;
		}
		while (retry > 0) {
			pr_err("hisee:%s() cos_id=%u,failed and retry=%d,ret=%d\n",
			       __func__, cos_id, retry, ret);
			retry--;
			ret1 = hisee_poweroff_func(buf, HISEE_PWROFF_LOCK);
			hisee_mdelay(DELAY_FOR_HISEE_POWERON_UPGRADE);
			ret2 = hisee_poweron_upgrade_func(buf, 0);
			hisee_mdelay(DELAY_FOR_HISEE_POWERON_UPGRADE);
			if (ret1 != HISEE_OK || ret2 != HISEE_OK)
				continue;
			goto upgrade_retry;
		}
	}

	check_and_print_result_with_cosid();
	return set_errno_then_exit(ret);
}

int handle_cos_image_upgrade(const void *buf, int para)
{
	int ret;
	unsigned int cos_id = COS_IMG_ID_0;
	enum hisee_img_file_type img_type = SLOADER_IMG_TYPE;
	int ret_tmp;

	ret = cos_upgrade_prepare(buf, &img_type, &cos_id);
	if (ret != HISEE_OK)
		goto upgrade_bypass;

	/*
	 * hisee factory test(include slt test) don't check
	 * there is new cos image
	 */
	ret = cos_upgrade_check_version(para, cos_id);
	if (ret != HISEE_OK) {
		if (ret == HISEE_IS_OLD_COS_IMAGE)
			pr_err("%s(): is old cosimage\n", __func__);
		goto upgrade_bypass;
	}

	ret = cos_image_upgrade_basic_process(buf, para, cos_id, img_type);
	if (ret != HISEE_OK)
		pr_err("%s(): cos_image_upgrade_basic_process failed,ret=%d\n",
		       __func__, ret);

upgrade_bypass:
	ret_tmp = hisee_poweroff_func(buf, HISEE_PWROFF_LOCK);
	if (ret_tmp != HISEE_OK) {
		pr_err("hisee:%s() cos_id=%u, poweroff failed. ret=%d\n",
		       __func__, cos_id, ret_tmp);
		if (ret == HISEE_OK)
			ret = ret_tmp;
	}
	check_and_print_result_with_cosid();
	return set_errno_then_exit(ret);
}

int cos_image_upgrade_func(const void *buf, int para)
{
	int ret;
	char buf_para[MAX_CMD_BUFF_PARAM_LEN] = {0};

	/* check hisee is powered on */
	if (hisee_get_power_status() != HISEE_POWER_STATUS_ON) {
		pr_err("%s hisee is not poweron\n", __func__);
		return set_errno_then_exit(HISEE_POWERCTRL_FLOW_ERROR);
	}

	buf_para[0] = HISEE_CHAR_SPACE;
	buf_para[HISEE_COS_ID_POS] = '0' + COS_IMG_ID_0; /* '0': int to char */
	buf_para[HISEE_PROCESS_TYPE_POS] = '0' + COS_PROCESS_UPGRADE; /* '0': int to char */
	ret = handle_cos_image_upgrade((void *)buf_para, para);
	check_and_print_result();
	return ret;
}

static int _has_new_cos_show(int cos_id, char *buf, size_t buf_len)
{
	int ret;
	int new_cos_exist = HISEE_FALSE;

	ret = check_new_cosimage(cos_id, &new_cos_exist);
	if (ret == HISEE_OK) {
		if (new_cos_exist == HISEE_TRUE) {
			ret = snprintf_s(buf, buf_len,
					 HISEE_NEW_COS_MAX_STRING,
					 "cos-%d %d,", cos_id,
					 HISEE_NEW_COS_EXIST);
			if (ret == HISEE_SECLIB_ERROR) {
				pr_err("%s(): snprintf1 err.\n", __func__);
				return HISEE_SECUREC_ERR;
			}
			ret = strncat_s(buf, buf_len,
					"exsited new cosimage",
					strlen("exsited new cosimage"));
			if (ret != EOK) {
				pr_err("%s(): strncat err.\n", __func__);
				return HISEE_SECUREC_ERR;
			}
		} else {
			ret = snprintf_s(buf, buf_len,
					 HISEE_NEW_COS_MAX_STRING,
					 "cos-%d %d,", cos_id,
					 HISEE_NEW_COS_NON_EXIST);
			if (ret == HISEE_SECLIB_ERROR) {
				pr_err("%s(): snprintf1 err.\n", __func__);
				return HISEE_SECUREC_ERR;
			}
			ret = strncat_s(buf, buf_len,
					"no exsited new cosimage",
					strlen("no exsited new cosimage"));
			if (ret != EOK) {
				pr_err("%s(): strncat err.\n", __func__);
				return HISEE_SECUREC_ERR;
			}
		}
	} else {
		ret = snprintf_s(buf, buf_len,
				 HISEE_NEW_COS_MAX_STRING,
				 "cos-%d %d,", cos_id, HISEE_NEW_COS_FAILURE);
		if (ret == HISEE_SECLIB_ERROR) {
			pr_err("%s(): snprintf1 err.\n", __func__);
			return HISEE_SECUREC_ERR;
		}
		ret = strncat_s(buf, buf_len, "failed",
				strlen("failed"));
		if (ret != EOK) {
			pr_err("%s(): strncat err.\n", __func__);
			return HISEE_SECUREC_ERR;
		}
	}
	return HISEE_OK;
}

ssize_t hisee_has_new_cos_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct hisee_img_header local_img_header;
	int cos_index, cos_cnt;
	int ret;

	if (!buf) {
		pr_err("%s buf parameters is null\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	(void)memset_s((void *)&local_img_header, sizeof(struct hisee_img_header),
		       0, sizeof(struct hisee_img_header));
	ret = hisee_parse_img_header((char *)(&local_img_header));
	if (ret != HISEE_OK) {
		pr_err("%s():hisee_parse_img_header failed, ret=%d\n",
		       __func__, ret);
		return set_errno_then_exit(ret);
	}

	cos_cnt = local_img_header.rpmb_cos_cnt + local_img_header.emmc_cos_cnt;
	if (cos_cnt > HISEE_MAX_COS_IMAGE_NUMBER) {
		pr_err("%s cos_cnt=%d out of threshold\n", __func__, cos_cnt);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	/*
	 * Although check every cos image in loop, but only save the result
	 * of existing cos image to buf
	 */
	for (cos_index = 0; cos_index < cos_cnt; cos_index++) {
		ret = _has_new_cos_show(cos_index, buf, HISEE_BUF_SHOW_LEN);
		if (ret != HISEE_OK)
			return set_errno_then_exit(ret);
	}

	pr_err("%s(): %s\n", __func__, buf);
	return (ssize_t)strlen(buf);
}

ssize_t hisee_check_upgrade_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	unsigned int upgrade_run_flg = HISEE_COS_UPGRADE_FINISH_FLG;
	int ret;

	if (!buf) {
		pr_err("%s buf parameters is null\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	access_hisee_image_partition((char *)&upgrade_run_flg,
				     COS_UPGRADE_RUN_READ_TYPE);
	if (upgrade_run_flg == HISEE_COS_UPGRADE_FINISH_FLG) {
		ret = snprintf_s(buf, HISEE_BUF_SHOW_LEN,
				 (size_t)(HISEE_BUF_SHOW_LEN - 1),
				 "%s", "0,cos upgrade success");
		if (ret == HISEE_SECLIB_ERROR) {
			pr_err("%s(): snprintf1 err.\n", __func__);
			return set_errno_then_exit(HISEE_SECUREC_ERR);
		}
	} else if (upgrade_run_flg == HISEE_COS_UPGRADE_RUNNING_FLG) {
		ret = snprintf_s(buf, HISEE_BUF_SHOW_LEN,
				 (size_t)(HISEE_BUF_SHOW_LEN - 1),
				 "%s", "1,cos upgrade failed last time");
		if (ret == HISEE_SECLIB_ERROR) {
			pr_err("%s(): snprintf2 err.\n", __func__);
			return set_errno_then_exit(HISEE_SECUREC_ERR);
		}
	} else {
		ret = snprintf_s(buf, HISEE_BUF_SHOW_LEN,
				 (size_t)(HISEE_BUF_SHOW_LEN - 1),
				 "%s", "-1,failed");
		if (ret == HISEE_SECLIB_ERROR) {
			pr_err("%s(): snprintf3 err.\n", __func__);
			return set_errno_then_exit(HISEE_SECUREC_ERR);
		}
	}

	pr_err("%s(): %s\n", __func__, buf);
	return (ssize_t)strlen(buf);
}
