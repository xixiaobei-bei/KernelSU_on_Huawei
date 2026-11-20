/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: implement hisee file system function
 * Create: 2020-02-17
 */
#include "hisee_fs.h"
#include <asm/compiler.h>
#include <linux/atomic.h>
#include <linux/clk.h>
#include <linux/compiler.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/fd.h>
#include <linux/fs.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/hisi/ipc_msg.h>
#include <linux/hisi/partition_ap_kernel.h>
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
#include "hisee.h"
#include "soc_acpu_baseaddr_interface.h"
#include "soc_sctrl_interface.h"

/* timestamp byte offset in string */
#define TIMESTAMP_YEAR_THOUSANDS_BYTE_OFFSET    0
#define TIMESTAMP_YEAR_HUNDREDS_BYTE_OFFSET     1
#define TIMESTAMP_YEAR_TENS_BYTE_OFFSET         2
#define TIMESTAMP_YEAR_ONES_BYTE_OFFSET         3

#define TIMESTAMP_MONTH_TENS_BYTE_OFFSET        5
#define TIMESTAMP_MONTH_ONES_BYTE_OFFSET        6

#define TIMESTAMP_DAY_TENS_BYTE_OFFSET          8
#define TIMESTAMP_DAY_ONES_BYTE_OFFSET          9

#define TIMESTAMP_HOUR_TENS_BYTE_OFFSET         11
#define TIMESTAMP_HOUR_ONES_BYTE_OFFSET         12

#define TIMESTAMP_MINUTE_TENS_BYTE_OFFSET       14
#define TIMESTAMP_MINUTE_ONES_BYTE_OFFSET       15

#define TIMESTAMP_SECOND_TENS_BYTE_OFFSET       17
#define TIMESTAMP_SECOND_ONES_BYTE_OFFSET       18

static unsigned int g_misc_version[HISEE_SUPPORT_COS_FILE_NUMBER] = {0};

/*
 * @brief      : parse the timestamp according timestamp string
 * @param[in]  : timestamp_str, timestamp_strtimestamp string before parsed
 * @param[out] : timestamp_value, parsed timestamp value
 * @return     : NA
 */
void parse_timestamp(const char *timestamp_str,
		     union timestamp_info *timestamp_value)
{
	unsigned short value_short;
	unsigned char  value_char;

	if (!timestamp_str || !timestamp_value)
		return;

	timestamp_value->value = 0x0;

	/* year: 20xx, string to int */
	value_short = (timestamp_str[TIMESTAMP_YEAR_THOUSANDS_BYTE_OFFSET] - '0') * THOUSANDS_DIGIT_WEIGHT +
		(timestamp_str[TIMESTAMP_YEAR_HUNDREDS_BYTE_OFFSET] - '0') * HUNDREDS_DIGIT_WEIGHT +
		(timestamp_str[TIMESTAMP_YEAR_TENS_BYTE_OFFSET] - '0') * TENS_DIGIT_WEIGHT +
		(timestamp_str[TIMESTAMP_YEAR_ONES_BYTE_OFFSET] - '0');
	timestamp_value->timestamp.year = value_short;

	/* month: 1~12, string to int */
	value_char = (timestamp_str[TIMESTAMP_MONTH_TENS_BYTE_OFFSET] - '0') * TENS_DIGIT_WEIGHT +
			(timestamp_str[TIMESTAMP_MONTH_ONES_BYTE_OFFSET] - '0');
	timestamp_value->timestamp.month = value_char;

	/* day: 1~31, string to int */
	value_char = (timestamp_str[TIMESTAMP_DAY_TENS_BYTE_OFFSET] - '0') * TENS_DIGIT_WEIGHT +
			(timestamp_str[TIMESTAMP_DAY_ONES_BYTE_OFFSET] - '0');
	timestamp_value->timestamp.day = value_char;

	/* hour: 0~23, string to int */
	value_char = (timestamp_str[TIMESTAMP_HOUR_TENS_BYTE_OFFSET] - '0') * TENS_DIGIT_WEIGHT +
			(timestamp_str[TIMESTAMP_HOUR_ONES_BYTE_OFFSET] - '0');
	timestamp_value->timestamp.hour = value_char;

	/* minute: 0~59, string to int */
	value_char = (timestamp_str[TIMESTAMP_MINUTE_TENS_BYTE_OFFSET] - '0') * TENS_DIGIT_WEIGHT +
			(timestamp_str[TIMESTAMP_MINUTE_ONES_BYTE_OFFSET] - '0');
	timestamp_value->timestamp.minute = value_char;

	/* second: 0~59, string to int */
	value_char = (timestamp_str[TIMESTAMP_SECOND_TENS_BYTE_OFFSET] - '0') * TENS_DIGIT_WEIGHT +
			(timestamp_str[TIMESTAMP_SECOND_ONES_BYTE_OFFSET] - '0');
	timestamp_value->timestamp.second = value_char;
}

/*
 * @brief      : write hisee otp value
 * @param[in]  : otp_img_index, otp image index
 * @return     : ::int, 0 is success; other value is failure
 */
int write_hisee_otp_value(enum hisee_img_file_type otp_img_index)
{
	char *buff_virt = NULL;
	phys_addr_t buff_phy = 0;
	struct atf_message_header *p_message_header = NULL;
	int ret;
	unsigned int image_size;
	unsigned int result_offset;
	struct hisee_module_data *hisee_data_ptr = NULL;

	if (otp_img_index < OTP_IMG_TYPE || otp_img_index > OTP1_IMG_TYPE) {
		pr_err("%s(): otp_img_index=%d invalid\n",
		       __func__, (int)otp_img_index);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}
	hisee_data_ptr = get_hisee_data_ptr();
	buff_virt = (char *)dma_alloc_coherent(hisee_data_ptr->cma_device,
					       HISEE_SHARE_BUFF_SIZE,
					       &buff_phy, GFP_KERNEL);
	if (!buff_virt) {
		pr_err("%s(): dma_alloc_coherent failed\n", __func__);
		return set_errno_then_exit(HISEE_NO_RESOURCES);
	}

	pr_err("%s(): entering, otp_img_index=%d\n", __func__, (int)otp_img_index);
	(void)memset_s(buff_virt, HISEE_SHARE_BUFF_SIZE, 0, HISEE_SHARE_BUFF_SIZE);
	p_message_header = (struct atf_message_header *)buff_virt;
	set_message_header(p_message_header, CMD_UPGRADE_OTP);
	ret = filesys_hisee_read_image(otp_img_index,
				       buff_virt + HISEE_ATF_MESSAGE_HEADER_LEN,
				       HISEE_SHARE_BUFF_SIZE -
				       HISEE_ATF_MESSAGE_HEADER_LEN);
	if (ret < HISEE_OK) {
		pr_err("%s(): filesys_hisee_read_image failed, ret=%d\n",
		       __func__, ret);
		dma_free_coherent(hisee_data_ptr->cma_device, HISEE_SHARE_BUFF_SIZE,
				  buff_virt, buff_phy);
		return set_errno_then_exit(ret);
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
			       HISEE_ATF_OTP_TIMEOUT, CMD_UPGRADE_OTP);
	if (ret != HISEE_OK) {
		if (result_offset + SMC_TEST_RESULT_SIZE <=
			HISEE_SHARE_BUFF_SIZE)
			pr_err("%s(): hisee reported fail code=%d\n", __func__,
			       *((int *)(void *)(buff_virt + result_offset)));
	}

	dma_free_coherent(hisee_data_ptr->cma_device, HISEE_SHARE_BUFF_SIZE,
			  buff_virt, buff_phy);
	return ret;
}

/*
 * @brief      : read full path file interface
 * @param[in]  : fullname, the full path name should be read
 * @param[out] : buffer, output the data has been read
 * @param[in]  : offset, the offset in this file started to read
 * @param[in]  : size, the count bytes should be read.if zero means read total file
 * @return     : ::int, the number of bytes read is returned (zero indicates end of file),
 *               On error, the return value is less than zero,
 *               please check the errcode in hisee module.
 */
int hisee_read_file(const char *fullname, char *buffer, size_t offset,
		    size_t size)
{
	struct file *fp = NULL;
	int ret = HISEE_OK;
	ssize_t cnt;
	ssize_t read_bytes = 0;
	loff_t pos;
	mm_segment_t old_fs;

	if (!fullname || !buffer) {
		pr_err("%s(): passed ptr is NULL\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	if (size == 0 || size > HISEE_MAX_IMG_SIZE) {
		pr_err("%s(): passed size is invalid\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	old_fs = get_fs();
	/* the kernel API generate pclint warning, ignore it */
	set_fs(KERNEL_DS); /*lint !e501*/
	fp = filp_open(fullname, O_RDONLY, FILESYS_RD_WR_MODE);
	if (IS_ERR(fp)) {
		pr_err("%s():open %s failed\n", __func__, fullname);
		ret = HISEE_OPEN_FILE_ERROR;
		goto out;
	}
	ret = vfs_llseek(fp, 0L, SEEK_END);
	if (ret < 0) {
		pr_err("%s():lseek %s failed from end.\n", __func__, fullname);
		ret = HISEE_LSEEK_FILE_ERROR;
		goto close;
	}
	pos = fp->f_pos;
	if ((offset + size) > (size_t)pos) {
		pr_err("%s(): offset(%lx), size(%lx) both invalid.\n",
		       __func__, offset, size);
		ret = HISEE_OUTOF_RANGE_FILE_ERROR;
		goto close;
	}
	ret = vfs_llseek(fp, offset, SEEK_SET);
	if (ret < 0) {
		pr_err("%s():lseek %s failed from begin.\n",
		       __func__, fullname);
		ret = HISEE_LSEEK_FILE_ERROR;
		goto close;
	}

	read_bytes = size;
	cnt = vfs_read(fp, (char __user *)buffer, read_bytes, &fp->f_pos);
	if (cnt < read_bytes) {
		pr_err("%s():read %s failed, return [%ld]\n",
		       __func__, fullname, cnt);
		ret = HISEE_READ_FILE_ERROR;
		goto close;
	}
	read_bytes = cnt;
close:
	filp_close(fp, NULL);
out:
	set_fs(old_fs);
	if (ret >= HISEE_OK) {
		set_hisee_errno(HISEE_OK);
		ret = read_bytes;
	} else {
		set_hisee_errno(ret);
	}
	return ret;
}

/*
 * @brief      : hisee_write_file
 * @param[in]  : fullname , file name in full path
 * @param[in]  : buffer , data to write
 * @param[in]  : size , data size
 * @return     : ::int, 0 is success; other value is failure
 */
int hisee_write_file(const char *fullname, const char *buffer, size_t size)
{
	struct file *fp = NULL;
	int ret = HISEE_OK;
	ssize_t cnt;
	loff_t pos = 0;
	mm_segment_t old_fs;

	if (!fullname || !buffer) {
		pr_err("%s(): passed ptr is NULL\n", __func__);
		return HISEE_INVALID_PARAMS;
	}

	old_fs = get_fs();
	/* the kernel API generate pclint warning, ignore it */
	set_fs(KERNEL_DS); /*lint !e501*/
	fp = filp_open(fullname, O_WRONLY | O_CREAT | O_TRUNC, FILESYS_RD_WR_MODE);
	if (IS_ERR(fp)) {
		pr_err("%s():open %s failed\n", __func__, fullname);
		ret = HISEE_OPEN_FILE_ERROR;
		goto out;
	}

	cnt = vfs_write(fp, (char __user *)buffer, size, &pos);
	if (cnt != (ssize_t)size) {
		pr_err("%s():write failed, return [%ld]\n", __func__, cnt);
		ret = HISEE_WRITE_FILE_ERROR;
	}

	filp_close(fp, NULL);

out:
	set_fs(old_fs);
	return ret;
}

/*
 * @brief      : implement the same atoi function as in libc, but only support
 *               postive integer
 * @param[in]  : str , the integer string
 * @return     : ::unsigned int, normal value is success; HISEE_U32_MAX_VALUE is
 *               failure
 */
unsigned int hisee_atoi(const char *str)
{
	const char *p = str;
	char c;
	unsigned long value = 0;

	if (!str)
		return HISEE_U32_MAX_VALUE;

	while ((c = *p++) != '\0') {
		if (c >= '0' && c <= '9') {
			value *= TENS_DIGIT_WEIGHT;
			value += (unsigned int)(c - '0');
			if (value >= HISEE_U32_MAX_VALUE)
				return HISEE_U32_MAX_VALUE;
		} else {
			return HISEE_U32_MAX_VALUE;
		}
	}
	return (unsigned int)value;
}

/*
 * @brief      : parse the misc image id from header information in hisee.img
 * @param[in]  : curr_file_info, pointer to info struct for the file header
 * @param[in]  : misc_image_cnt_array, pointer to array of misc image count
 * @return     : ::int, 0 is success; other value is failure
 */
static int parse_misc_id_from_name(const struct img_file_info *curr_file_info,
				   unsigned int *misc_image_cnt_array)
{
	unsigned int curr_misc_id;
	unsigned int cos_id;
	unsigned int max_misc_num = HISEE_MAX_MISC_IMAGE_NUMBER;
	unsigned int is_smx_0 = 0;

	if (!curr_file_info || !misc_image_cnt_array)
		return HISEE_ERROR;

	if (strncmp(curr_file_info->name, HISEE_IMG_MISC_NAME,
		    strlen(HISEE_IMG_MISC_NAME)))
		return HISEE_ERROR;

	/* get the misc id from image name, range is [0,19] */
	curr_misc_id = hisee_atoi(curr_file_info->name +
				  strlen(HISEE_IMG_MISC_NAME));
	if (curr_misc_id >= HISEE_MAX_MISC_ID_NUMBER)
		return HISEE_ERROR;

	hisee_get_smx_cfg(&is_smx_0);
	if (is_smx_0 == SMX_PROCESS_0)
		max_misc_num = HISEE_SMX_MISC_IMAGE_NUMBER;

	/* group misc image index by cos_id, then counter add one */
	cos_id = curr_misc_id / max_misc_num;
	misc_image_cnt_array[cos_id] += 1;

	if (misc_image_cnt_array[cos_id] > max_misc_num) {
		pr_err("%s():misc cnt =%u is invalid\n",
		       __func__, misc_image_cnt_array[cos_id]);
		return HISEE_SUB_FILE_OFFSET_CHECK_ERROR;
	}

	/* get misc version */
	if (is_smx_0 != SMX_PROCESS_0) {
		if (misc_image_cnt_array[cos_id] == max_misc_num) {
			misc_image_cnt_array[cos_id] -=
				HISEE_MISC_NO_UPGRADE_NUMBER;
			g_misc_version[cos_id] = curr_file_info->size;
			pr_err("hisee:%s():cos_id=%u, misc_cnt=%u\n",
			       __func__, cos_id, misc_image_cnt_array[cos_id]);
			pr_err("hisee:%s():misc_version=%u\n",
			       __func__, g_misc_version[cos_id]);
		}
	} else {
		pr_err("hisee:%s():cos_id=%u, misc_cnt=%u\n",
		       __func__, cos_id, misc_image_cnt_array[cos_id]);
	}

	return HISEE_OK;
}

static int _check_misc_img_info(struct img_file_info *file, unsigned int *misc_img_cnt)
{
	int ret;

	if (strncmp(file->name, HISEE_IMG_MISC_NAME,
		    strlen(HISEE_IMG_MISC_NAME)) == 0) {
		ret = parse_misc_id_from_name(file, misc_img_cnt);
		if (ret != HISEE_OK) {
			pr_err("hisee:%s():misc id failed, ret=%d\n",
			       __func__, ret);
			return HISEE_IMG_SUB_FILE_NAME_ERROR;
		}
	}
	return HISEE_OK;
}


static int _get_image_count(const struct img_file_info *file,
			    unsigned int *emmc_cnt, unsigned int *rpmb_cnt)
{
	if (strncmp(file->name, HISEE_IMG_COS_NAME,
		    strlen(HISEE_IMG_COS_NAME)) == 0) {
		if ((file->name[strlen(HISEE_IMG_COS_NAME)] - '0') <
		    HISEE_MAX_RPMB_COS_NUMBER)
			*rpmb_cnt = *rpmb_cnt + 1;
		else
			*emmc_cnt = *emmc_cnt + 1;
	}
	return HISEE_OK;
}

static int _check_sub_file_info(const struct img_file_info *file, unsigned int total_size)
{
	if (file->size == 0 || file->size > HISEE_MAX_IMG_SIZE) {
		pr_err("%s():size check %s failed\n",
		       __func__, file->name);
		return HISEE_SUB_FILE_SIZE_CHECK_ERROR;
	}
	if (file->offset < total_size) {
		pr_err("%s():offset check %s failed\n",
		       __func__, file->name);
		return HISEE_SUB_FILE_OFFSET_CHECK_ERROR;
	}
	return HISEE_OK;
}

static int check_img_header_is_valid(struct hisee_img_header *p_img_header)
{
	struct img_file_info *img_file = NULL;
	unsigned int emmc_cos_cnt = 0;
	unsigned int rpmb_cos_cnt = 0;
	unsigned int total_size;
	unsigned int index;
	int ret;

	total_size = HISEE_IMG_SUB_FILES_LEN * p_img_header->file_cnt +
		     HISEE_IMG_HEADER_LEN;

	for (index = 0; index < p_img_header->file_cnt; index++) {
		img_file = &p_img_header->file[index];
		ret = _check_misc_img_info(img_file, &p_img_header->misc_image_cnt[0]);
		if (ret != HISEE_OK)
			return ret;

		ret = _get_image_count(img_file, &emmc_cos_cnt, &rpmb_cos_cnt);
		if (ret != HISEE_OK)
			return ret;

		ret = _check_sub_file_info(img_file, total_size);
		if (ret != HISEE_OK)
			return ret;

		total_size += img_file->size;
	}

	total_size = rpmb_cos_cnt + emmc_cos_cnt;
	if (rpmb_cos_cnt > HISEE_MAX_RPMB_COS_NUMBER ||
	    emmc_cos_cnt > HISEE_MAX_EMMC_COS_NUMBER ||
	    total_size > HISEE_SUPPORT_COS_FILE_NUMBER) {
		pr_err("%s():cos cnt =%u is invalid\n",
		       __func__, total_size);
		return HISEE_SUB_FILE_OFFSET_CHECK_ERROR;
	}

	p_img_header->emmc_cos_cnt = emmc_cos_cnt;
	p_img_header->rpmb_cos_cnt = rpmb_cos_cnt;
	pr_err("%s():rpmb_cos cnt =%u emmc_cos cnt =%u\n",
	       __func__, rpmb_cos_cnt, emmc_cos_cnt);
	return HISEE_OK;
}

static int _read_img_header_info(struct file **fp, const char *fullname,
				 char *buffer)
{
	struct file *curr_fp = NULL;
	loff_t pos;
	int cnt;

	curr_fp = filp_open(fullname, O_RDONLY, FILESYS_RD_WR_MODE);
	if (IS_ERR(curr_fp)) {
		pr_err("%s():open %s failed\n", __func__, fullname);
		return HISEE_OPEN_FILE_ERROR;
	}
	pos = curr_fp->f_pos;
	cnt = (int)vfs_read(curr_fp, (char __user *)buffer,
			    HISEE_IMG_HEADER_LEN, &pos);
	if (cnt < HISEE_IMG_HEADER_LEN) {
		pr_err("%s():read %s failed, return [%d]\n",
		       __func__, fullname, cnt);
		filp_close(curr_fp, NULL);
		return HISEE_READ_FILE_ERROR;
	}

	/*
	 * Because vfs_read() don't update f_pos, so updating file position
	 * forcely, for later read operation in sequence with the same fd.
	 */
	curr_fp->f_pos = pos;
	*fp = curr_fp; /* return the be opened fd */
	return HISEE_OK;
}

static int _parse_img_header_info(struct file *fp, char *buffer)
{
	struct hisee_img_header *p_img_header = NULL;
	size_t cnt, sub_file_len;
	int ret;

	p_img_header = (struct hisee_img_header *)buffer;
	if (strncmp(p_img_header->magic, HISEE_IMG_MAGIC_VALUE,
		    HISEE_IMG_MAGIC_LEN) != 0) {
		pr_err("%s() hisee_img magic value is wrong.\n", __func__);
		return HISEE_IMG_PARTITION_MAGIC_ERROR;
	}
	if (p_img_header->file_cnt > HISEE_IMG_SUB_FILE_MAX) {
		pr_err("%s() hisee_img file numbers is invalid.\n", __func__);
		return HISEE_IMG_PARTITION_FILES_ERROR;
	}
	sub_file_len = (size_t)(HISEE_IMG_SUB_FILES_LEN * p_img_header->file_cnt);
	cnt = (size_t)vfs_read(fp, (char __user *)(buffer + HISEE_IMG_HEADER_LEN),
			       sub_file_len, &fp->f_pos);
	if (cnt < sub_file_len) {
		pr_err("%s():read failed, return [%ld]\n",
		       __func__, cnt);
		return HISEE_READ_FILE_ERROR;
	}

	ret = check_img_header_is_valid(p_img_header);
	if (ret != HISEE_OK) {
		pr_err("%s(): check_img_header_is_valid fail,ret=%d.\n",
		       __func__, ret);
		return ret;
	}

	return HISEE_OK;
}

static int _buildup_local_img_header(struct file *fp, char *buffer)
{
	struct hisee_img_header *p_img_header = NULL;
	loff_t pos;
	size_t cnt;
	unsigned int i;
	unsigned int sw_version_num;
	unsigned char cos_img_rawdata[HISEE_IMG_HEADER_LEN] = {0};

	p_img_header = (struct hisee_img_header *)buffer;
	/*
	 * there is a assumption: the first file in hisee.img is always
	 * the cos image 0 , then flowed by cos[,1,2,3,...],
	 * then flowed by MISC image
	 */
	for (i = 0; i < p_img_header->file_cnt; i++) {
		/* if it is not cos image type, continue with next image. */
		if (strncmp(p_img_header->file[i].name, HISEE_IMG_COS_NAME,
			    strlen(HISEE_IMG_COS_NAME)) != 0) {
			continue;
		}

		pos = p_img_header->file[i].offset;
		cnt = (size_t)vfs_read(fp, (char __user *)cos_img_rawdata,
				       COS_IMAGE_RAWDATA_LEN, &pos);
		if (cnt < COS_IMAGE_RAWDATA_LEN) {
			pr_err("%s():read %s failed, return [%lu]\n",
			       __func__, p_img_header->file[i].name, cnt);
			return HISEE_READ_FILE_ERROR;
		}
		sw_version_num =
			*((unsigned int *)(&cos_img_rawdata[HISEE_COS_VERSION_OFFSET]));
		p_img_header->sw_version_cnt[i] = sw_version_num;
	}

	return HISEE_OK;
}

/*
 * @brief      : parse the hisee_img partition header interface
 * @param[out] : buffer, save the output data
 * @return     : ::int, 0 is success; other value is failure
 */
int hisee_parse_img_header(char *buffer)
{
	struct file *fp = NULL;
	char fullname[MAX_PATH_NAME_LEN + 1] = {0};
	mm_segment_t old_fs;
	int ret;

	ret = flash_find_ptn_s(HISEE_IMAGE_PARTITION_NAME, fullname,
			       sizeof(fullname));
	if (ret != 0 || !buffer) {
		pr_err("%s():flash_find_ptn_s or buffer params fail,ret=%d\n",
		       __func__, ret);
		ret = HISEE_OPEN_FILE_ERROR;
		return set_errno_then_exit(ret);
	}

	old_fs = get_fs();
	/* the kernel API generate pclint warning, ignore it */
	set_fs(KERNEL_DS); /*lint !e501*/

	ret = _read_img_header_info(&fp, fullname, buffer);
	if (ret != HISEE_OK) {
		set_fs(old_fs);
		return set_errno_then_exit(ret);
	}

	ret = _parse_img_header_info(fp, buffer);
	if (ret != HISEE_OK)
		goto restore_handle;
	ret = _buildup_local_img_header(fp, buffer);
	if (ret != HISEE_OK)
		goto restore_handle;

restore_handle:
	filp_close(fp, NULL);
	set_fs(old_fs);
	return set_errno_then_exit(ret);
}

static struct img_name_info_t g_image_name_info[MAX_IMG_TYPE + 1] = {
	{ SLOADER_IMG_TYPE,   "SLOADER" },
	{ COS_IMG_TYPE,       "COS" },
	{ OTP_IMG_TYPE,       "OTP0" },
	{ OTP1_IMG_TYPE,      "OTP1" },
	{ MISC0_IMG_TYPE,     "MISC0" },
	{ MISC1_IMG_TYPE,     "MISC1" },
	{ MISC2_IMG_TYPE,     "MISC2" },
	{ MISC3_IMG_TYPE,     "MISC3" },
	{ MISC4_IMG_TYPE,     "MISC4" },
	{ MAX_IMG_TYPE,       "" },
};

static int get_sub_file_name(enum hisee_img_file_type type, char *sub_file_name,
			     size_t max_len)
{
	int ret;

	if (!sub_file_name)
		return HISEE_INVALID_PARAMS;

	if (type >= MAX_IMG_TYPE) {
		pr_err("%s(): image type err, type %d.\n", __func__, type);
		return HISEE_IMG_SUB_FILE_NAME_ERROR;
	}

	ret = memcpy_s(sub_file_name, max_len,
		       g_image_name_info[type].img_file_name,
		       HISEE_IMG_SUB_FILE_NAME_LEN);
	if (ret != EOK) {
		pr_err("%s(): memcpy_s err.\n", __func__);
		return HISEE_SECLIB_ERROR;
	}

	return HISEE_OK;
}

static int hisee_image_type_chk(enum hisee_img_file_type type, const char *sub_file_name,
				const struct hisee_img_header *p_hisee_img_head,
				unsigned int *index)
{
	unsigned int i;
	int ret = HISEE_OK;

	if (!sub_file_name || !p_hisee_img_head || !index) {
		pr_err("%s: param error!", __func__);
		return HISEE_ERROR;
	}

	for (i = 0; i < p_hisee_img_head->file_cnt; i++) {
		if (p_hisee_img_head->file[i].name[0] == 0)
			continue;
		if (type == OTP_IMG_TYPE) {
			if (!strncmp(sub_file_name,
				     p_hisee_img_head->file[i].name,
				     (size_t)HISEE_IMG_SUB_FILE_NAME_LEN) ||
			    !strncmp(HISEE_IMG_OTP_NAME,
				     p_hisee_img_head->file[i].name,
				     (size_t)HISEE_IMG_SUB_FILE_NAME_LEN))
				break;
		} else if (type == OTP1_IMG_TYPE) {
			if (!strncmp(HISEE_IMG_OTP1_NAME,
				     p_hisee_img_head->file[i].name,
				     (size_t)HISEE_IMG_SUB_FILE_NAME_LEN))
				break;
		} else {
			if (!strncmp(sub_file_name,
				     p_hisee_img_head->file[i].name,
				     (size_t)HISEE_IMG_SUB_FILE_NAME_LEN))
				break;
		}
	}

	if (i == p_hisee_img_head->file_cnt) {
		pr_err("%s():image type is %d, sub_file_name is %s\n",
		       __func__, type, sub_file_name);
		pr_err("%s():hisee_read_img_header failed, ret=%d\n",
		       __func__, ret);
		ret = HISEE_IMG_SUB_FILE_ABSENT_ERROR;
	}

	*index = i;
	return ret;
}

/*
 * @brief      : read hisee_fs partition file interface
 * @param[in]  : type, the file type need to read in hisee_img partition
 * @param[out] : buffer, save the output data
 * @return     : ::int, On success, the number of bytes read is returned
 *               (zero indicates end of file), On error, the return value is
 *               less than zero,please check the errcode in hisee module.
 */
int filesys_hisee_read_image(enum hisee_img_file_type type, char *buffer, size_t bsize)
{
	int ret;
	struct hisee_img_header local_img_header;
	char sub_file_name[HISEE_IMG_SUB_FILE_NAME_LEN] = {0};
	char fullname[MAX_PATH_NAME_LEN + 1] = {0};
	unsigned int index = 0;
	struct hisee_module_data *hisee_data_ptr = NULL;

	if (!buffer) {
		pr_err("%s: param error!", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	hisee_data_ptr = get_hisee_data_ptr();
	ret = get_sub_file_name(type, sub_file_name, sizeof(sub_file_name));
	if (ret != HISEE_OK)
		return set_errno_then_exit(ret);

	(void)memset_s((void *)&local_img_header, sizeof(struct hisee_img_header),
		       0, sizeof(struct hisee_img_header));
	mutex_lock(&hisee_data_ptr->hisee_mutex);
	/*
	 * Notice:why there still use hisee_data_ptr->hisee_img_head not use
	 * locl_img_header, Because hisee_data_ptr->hisee_img_head
	 * need to initiliaze than be accessed in otherwhere
	 */
	(void)memset_s((void *)&hisee_data_ptr->hisee_img_head,
		       sizeof(struct hisee_img_header),
		       0, sizeof(struct hisee_img_header));
	ret = hisee_parse_img_header((char *)&hisee_data_ptr->hisee_img_head);
	if (ret != HISEE_OK) {
		pr_err("%s():hisee_read_img_header failed, ret=%d\n",
		       __func__, ret);
		mutex_unlock(&hisee_data_ptr->hisee_mutex);
		return set_errno_then_exit(ret);
	}
	ret = memcpy_s((void *)&local_img_header, sizeof(local_img_header),
		       (void *)&hisee_data_ptr->hisee_img_head,
		       sizeof(struct hisee_img_header));
	if (ret != EOK) {
		pr_err("%s(): memcpy_s err.\n", __func__);
		mutex_unlock(&hisee_data_ptr->hisee_mutex);
		return set_errno_then_exit(HISEE_SECUREC_ERR);
	}
	mutex_unlock(&hisee_data_ptr->hisee_mutex);

	ret = hisee_image_type_chk(type, sub_file_name,
				   &local_img_header, &index);
	if (ret != HISEE_OK)
		return set_errno_then_exit(ret);

	ret = flash_find_ptn_s(HISEE_IMAGE_PARTITION_NAME, fullname,
			       sizeof(fullname));
	if (ret != 0) {
		pr_err("%s():flash_find_ptn_s fail\n", __func__);
		ret = HISEE_OPEN_FILE_ERROR;
		set_hisee_errno(ret);
		return ret;
	}

	if (local_img_header.file[index].size > bsize) {
		pr_err("%s(): file size overflow\n", __func__);
		ret = HISEE_SUB_FILE_SIZE_CHECK_ERROR;
		set_hisee_errno(ret);
		return ret;
	}
	ret = hisee_read_file((const char *)fullname, buffer,
			      local_img_header.file[index].offset,
			      local_img_header.file[index].size);
	if (ret < HISEE_OK) {
		pr_err("%s():hisee_read_file failed, ret=%d\n",
		       __func__, ret);
		set_hisee_errno(ret);
		return ret;
	}
	ret = local_img_header.file[index].size;
	set_hisee_errno(HISEE_OK);
	return ret;
}

/*
 * @brief      : read cos image data(it's path is filename) to
 *               the buffer(point by buffer)
 * @param[in]  : filename, the cos image file name
 * @param[out] : buffer, save the data which read from file
 * @param[out] : file_size, the file size has been read, unit is byte
 * @param[in]  : max_read_size, the maximum file size can be read, unit is byte
 * @return     : ::int, 0 is success; other value is failure
 */
int filesys_read_img_from_file(const char *filename, char *buffer,
			       size_t *file_size, size_t max_read_size)
{
	int ret;
	mm_segment_t old_fs;
	struct kstat m_stat = {0};

	if (!buffer || !filename || !file_size)
		return HISEE_INVALID_PARAMS;

	old_fs = get_fs();
	/* the kernel API generate pclint warning, ignore it */
	set_fs(KERNEL_DS); /*lint !e501*/
	if (hisee_sys_access(filename, 0) != 0) {
		pr_err("hisee:%s(): %s is not exist.\n", __func__, filename);
		set_fs(old_fs);
		return HISEE_MULTICOS_COS_FLASH_FILE_ERROR;
	}

	ret = vfs_stat(filename, &m_stat);
	if (ret) {
		set_fs(old_fs);
		return HISEE_INVALID_PARAMS;
	}
	set_fs(old_fs);

	if ((size_t)(m_stat.size) == 0 ||
	    (size_t)(m_stat.size) > max_read_size) {
		pr_err("hisee:%s(): file size is more than %u bytes.\n",
		       __func__, (unsigned int)max_read_size);
		return HISEE_SUB_FILE_SIZE_CHECK_ERROR;
	}
	*file_size = (size_t)(m_stat.size);

	ret = hisee_read_file((const char *)filename, buffer, 0, (*file_size));
	if (ret < HISEE_OK) {
		pr_err("hisee:%s():hisee_read_file failed, ret=%d\n",
		       __func__, ret);
		return ret;
	}
	return HISEE_OK;
}


static void access_hisee_file_prepare(enum hisee_image_a_access_type access_type,
				      int *flags, long *file_offset,
				      unsigned long *size)
{
	if (access_type == SW_VERSION_WRITE_TYPE ||
	    access_type == COS_UPGRADE_RUN_WRITE_TYPE ||
	    access_type == MISC_VERSION_WRITE_TYPE ||
	    access_type == COS_UPGRADE_INFO_WRITE_TYPE)
		*flags = O_WRONLY;
	else
		*flags = O_RDONLY;

	if (access_type == SW_VERSION_WRITE_TYPE ||
	    access_type == SW_VERSION_READ_TYPE) {
		*file_offset = HISEE_IMG_PARTITION_SIZE - SIZE_1K;
		*size = sizeof(struct cosimage_version_info);
	} else if (access_type == COS_UPGRADE_RUN_WRITE_TYPE ||
		   access_type == COS_UPGRADE_RUN_READ_TYPE) {
		*file_offset = (long)((HISEE_IMG_PARTITION_SIZE - SIZE_1K) +
			HISEE_COS_VERSION_STORE_SIZE);
		*size = sizeof(unsigned int);
	} else if (access_type == MISC_VERSION_READ_TYPE ||
		   access_type == MISC_VERSION_WRITE_TYPE) {
		*file_offset = (long)((HISEE_IMG_PARTITION_SIZE - SIZE_1K) +
			HISEE_COS_VERSION_STORE_SIZE
			+ HISEE_UPGRADE_STORE_SIZE);
		*size = sizeof(struct cosimage_version_info);
	} else { /* for COS_UPGRADE_INFO_READ/WRITE_TYPE */
		*file_offset = (long)((HISEE_IMG_PARTITION_SIZE - SIZE_1K) +
			HISEE_COS_VERSION_STORE_SIZE
			+ HISEE_UPGRADE_STORE_SIZE
			+ HISEE_MISC_VERSION_STORE_SIZE);
		*size = sizeof(struct multicos_upgrade_info);
	}
}

int hisee_get_partition_path(char full_path[MAX_PATH_NAME_LEN], size_t buff_len)
{
	int ret;

	if(!full_path) {
		pr_err("%s():full_path is NULL\n", __func__);
		return HISEE_INVALID_PARAMS;
	}
	ret = flash_find_ptn_s(HISEE_IMAGE_PARTITION_NAME, full_path, buff_len);
	if (ret != 0) {
		pr_err("%s():flash_find_ptn_s fail\n", __func__);
		ret = HISEE_OPEN_FILE_ERROR;
	} else {
		ret = HISEE_OK;
	}

	return ret;
}

/*
 * @brief       : access hisee_image partition according access type
 * @param[inout]: data_buf, data buffer according access type
 * @param[in]   : access_type, indicate access operation
 * @return     : ::int, 0 on success, other value on failure
 */
int access_hisee_image_partition(char *data_buf,
				 enum hisee_image_a_access_type access_type)
{
	int fd = -1;
	ssize_t cnt;
	mm_segment_t old_fs;
	char fullpath[MAX_PATH_NAME_LEN] = {0};
	long file_offset;
	unsigned long size;
	int ret;
	int flags;

	if (!data_buf)
		return set_errno_then_exit(HISEE_INVALID_PARAMS);

	ret = hisee_get_partition_path(fullpath, MAX_PATH_NAME_LEN);
	if (ret != HISEE_OK)
		return set_errno_then_exit(HISEE_INVALID_PARAMS);

	old_fs = get_fs();
	/* the kernel API generate pclint warning, ignore it */
	set_fs(KERNEL_DS); /*lint !e501*/

	access_hisee_file_prepare(access_type, &flags, &file_offset, &size);
	fd = (int)hisee_sys_open(fullpath, flags, HISEE_FILESYS_DEFAULT_MODE);
	if (fd < 0) {
		pr_err("%s(): %d open failed %d\n", __func__, __LINE__, fd);
		ret = HISEE_OPEN_FILE_ERROR;
		set_fs(old_fs);
		return set_errno_then_exit(ret);
	}

	ret = (int)hisee_sys_lseek((unsigned int)fd, file_offset, SEEK_SET);
	if (ret < 0) {
		pr_err("%s(): sys_lseek failed,ret=%d.\n", __func__, ret);
		ret = HISEE_LSEEK_FILE_ERROR;
		hisee_sys_close((unsigned int)fd);
		set_fs(old_fs);
		return set_errno_then_exit(ret);
	}
	ret = HISEE_OK;
	if (hisee_is_write_access(access_type)) {
		cnt = hisee_sys_write((unsigned int)fd, (char __user *)data_buf, size);
		ret = hisee_sys_fsync((unsigned int)fd);
		if (ret < 0) {
			pr_err("%s(): %d fail to sync.\n", __func__, __LINE__);
			ret = HISEE_ENCOS_SYNC_FILE_ERROR;
		}
	} else {
		cnt = hisee_sys_read((unsigned int)fd, (char __user *)data_buf, size);
	}

	if (cnt < (ssize_t)(size)) {
		pr_err("%s(): %d access failed, return [%ld]\n",
		       __func__, __LINE__, cnt);
		ret = HISEE_ACCESS_FILE_ERROR;
	}

	if (fd >= 0)
		hisee_sys_close((unsigned int)fd);

	set_fs(old_fs);
	return set_errno_then_exit(ret);
}

int hisee_update_misc_version(unsigned int cos_id)
{
	struct cosimage_version_info misc_version = {0};
	int ret;

	if (cos_id >= HISEE_SUPPORT_COS_FILE_NUMBER) {
		pr_err("%s(): cos_id=%u invalid\n", __func__, cos_id);
		return HISEE_INVALID_PARAMS;
	}

	/* write current misc version into record area */
	if (g_misc_version[cos_id]) {
		ret = access_hisee_image_partition((char *)&misc_version,
						   MISC_VERSION_READ_TYPE);
		if (ret != HISEE_OK) {
			pr_err("%s read_hisee_image_partition fail,ret=%d\n",
			       __func__, ret);
			return ret;
		}
		misc_version.magic = HISEE_SW_VERSION_MAGIC_VALUE;
		misc_version.img_version_num[cos_id] =
			(unsigned char)g_misc_version[cos_id];
		ret = access_hisee_image_partition((char *)&misc_version,
						   MISC_VERSION_WRITE_TYPE);
		if (ret != HISEE_OK) {
			pr_err("%s write_hisee_image_partition fail,ret=%d\n",
			       __func__, ret);
			return ret;
		}
	}
	return HISEE_OK;
}

static int get_hisee_img_name(char *fullname, int name_len)
{
	int ret;
	int retry = HISEE_PARTITION_READY_TIMEOUT_CNT;
	const unsigned int timeout = HISEE_PARTITION_READY_TIMEOUT_MS;

	ret = flash_find_ptn_s(HISEE_IMAGE_PARTITION_NAME, fullname, name_len);
	if (ret != 0) {
		pr_err("%s():flash_find_ptn_s fail\n", __func__);
		return ret;
	}
	/* wait partition ready */
	do {
		if (hisee_sys_access(fullname, 0) == 0)
			break;
		msleep(timeout);
		retry--;
	} while (retry > 0);

	if (retry <= 0)
		return HISEE_ERROR;

	return HISEE_OK;
}

static int erase_hisee_img(struct file *fp)
{
	loff_t pos;
	int cnt;
	int ret;

	char *buffer = kzalloc(SIZE_1M, GFP_KERNEL);
	if (!buffer) {
		pr_err("%s(): kmalloc failed\n", __func__);
		return HISEE_NO_RESOURCES;
	}

	do {
		/* STEP1: erase 1M size behind magic */
		pos = HISEE_IMG_MAGIC_LEN;
		cnt = vfs_write(fp, (char __user *)buffer, SIZE_1M, &pos);
		if (cnt != SIZE_1M) {
			pr_err("%s():write failed 1, return [%d]\n", __func__, cnt);
			ret = HISEE_WRITE_FILE_ERROR;
			break;
		}

		/* STEP2: erase magic */
		pos = 0;
		cnt = vfs_write(fp, (char __user *)buffer, HISEE_IMG_MAGIC_LEN, &pos);
		if (cnt != HISEE_IMG_MAGIC_LEN) {
			pr_err("%s():write failed 2, return [%d]\n", __func__, cnt);
			ret = HISEE_WRITE_FILE_ERROR;
			break;
		}
		ret = HISEE_OK;
	} while (0);

	kfree(buffer);
	return ret;
}

static int erase_hisee_img_body(void *arg)
{
	int ret;
	int cnt;
	loff_t pos = 0;
	struct file *hisee_img_fp = NULL;
	char magic[HISEE_IMG_MAGIC_LEN] = {0};
	char magic_zero[HISEE_IMG_MAGIC_LEN] = {0};
	char fullname[MAX_PATH_NAME_LEN + 1] = {0};

	ret = get_hisee_img_name(fullname, sizeof(fullname));
	if (ret != HISEE_OK)
		return set_errno_then_exit(ret);

	hisee_img_fp = filp_open(fullname, O_RDWR, FILESYS_RD_WR_MODE);
	if (IS_ERR(hisee_img_fp)) {
		pr_err("%s():open %s failed, ret:%pK\n", __func__, fullname, hisee_img_fp);
		return set_errno_then_exit(HISEE_OPEN_FILE_ERROR);
	}

	do {
		/* get hisee img magic value */
		cnt = (int)vfs_read(hisee_img_fp, (char __user *)magic, HISEE_IMG_MAGIC_LEN, &pos);
		if (cnt < HISEE_IMG_MAGIC_LEN) {
			pr_err("%s():read img failed, return [%d]\n", __func__, cnt);
			ret = HISEE_READ_FILE_ERROR;
			break;
		}

		/* compare magic with zero, if magic is zero, no need erase */
		if (memcmp((void *)magic, (void *)magic_zero, HISEE_IMG_MAGIC_LEN) == 0)
			break;

		ret = erase_hisee_img(hisee_img_fp);
		if (ret != HISEE_OK)
			pr_err("%s():erase hisee img err ret=%x\n", __func__, ret);
		else
			pr_err("%s():erase hisee img succ\n", __func__);
	} while (0);

	filp_close(hisee_img_fp, NULL);
	return set_errno_then_exit(ret);
}
/*
 * @brief     : used to erase hisee_img when hisec is bypassed
 */
int start_erase_hisee_image(void)
{
	struct task_struct *erase_img_task = NULL;

	erase_img_task = kthread_run(erase_hisee_img_body, NULL, "erase_img_task");
	if (!erase_img_task)
		return HISEE_THREAD_CREATE_ERROR;

	return HISEE_OK;
}
