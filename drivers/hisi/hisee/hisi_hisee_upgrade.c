#include <asm/compiler.h>
#include <linux/compiler.h>
#include <linux/fd.h>
#include <linux/tty.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/semaphore.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_reserved_mem.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/atomic.h>
#include <linux/notifier.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/hisi/ipc_msg.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/hisi/kirin_partition.h>
#include <linux/clk.h>
#include <linux/mm.h>
#include <linux/completion.h>
#include <securec.h>
#include "soc_acpu_baseaddr_interface.h"
#include "soc_sctrl_interface.h"
#include "hisi_hisee.h"
#include "hisi_hisee_fs.h"
#include "hisi_hisee_power.h"
#include "hisi_hisee_chip_test.h"
#include "hisi_hisee_upgrade.h"

extern atomic_t g_hisee_errno;
extern int get_rpmb_key_status(void);
extern void hisee_mntn_update_local_ver_info(void);
extern int32_t hisee_exception_to_reset_rpmb(void);
extern void rdr_hisee_call_to_record_exc(int data);

static int check_sw_version_null(const cosimage_version_info *info, unsigned int cos_id)
{
	if (!info->magic && !info->img_version_num[cos_id] &&
		(!info->img_timestamp.value))
		return HISEE_TRUE;
	else
		return HISEE_FALSE;
}

static int check_timestamp_valid(const timestamp_info *timestamp_value)
{
	if (timestamp_value->timestamp.year < 2016)
		return HISEE_FALSE;

	/*the year of deadline is 2050? I Don't konw! */
	if (timestamp_value->timestamp.year >= 2050)
		return HISEE_FALSE;

	if (timestamp_value->timestamp.month > 12)
		return HISEE_FALSE;

	/*the judge is not accurate, because not all month has 31 day,
	  depend on the value of year and month*/
	if (timestamp_value->timestamp.day > 31)
		return HISEE_FALSE;

	if (timestamp_value->timestamp.hour >= 24)
		return HISEE_FALSE;

	if (timestamp_value->timestamp.minute >= 60)
		return HISEE_FALSE;

	if (timestamp_value->timestamp.second >= 60)
		return HISEE_FALSE;

	return HISEE_TRUE;
}

/*************************************************************
函数原型：int first_check_newest_cosimage(unsigned int cos_id,
											 const cosimage_version_info *curr_ptr,
											 const cosimage_version_info *previous_ptr)
函数功能：根据cos_id对应的cos镜像，做第一级检查返回是否是新cos镜像(根据cos镜像的NV counter软件版本号和时间戳)
参数：
输入：cos_id:当前cos镜像的id；curr_ptr:指向系统前次保存在hisee_img分区的cos镜像的NV counter软件版本号和时间戳信息结构体；
		previous_ptr:指向系统前次保存在hisee_img分区的cos镜像的NV counter软件版本号和时间戳信息结构体
输出：无。
返回值：HISEE_TRUE:当前cos_id对应的cos镜像是新镜像；HISEE_FALSE:当前cos_id对应的cos镜像是老镜像
前置条件： 无
后置条件： 无
*************************************************************/
static int first_check_newest_cosimage(unsigned int cos_id,
											 const cosimage_version_info *curr_ptr,
											 const cosimage_version_info *previous_ptr)
{
	if (!check_timestamp_valid(&(curr_ptr->img_timestamp))) {
		return HISEE_FALSE;
	}
	if (!check_timestamp_valid(&(previous_ptr->img_timestamp))) {
		return HISEE_FALSE;
	}

	if (HISEE_SW_VERSION_MAGIC_VALUE != curr_ptr->magic
		|| HISEE_SW_VERSION_MAGIC_VALUE != previous_ptr->magic) {
		return HISEE_FALSE;
	}

	if (curr_ptr->img_version_num[cos_id] > previous_ptr->img_version_num[cos_id]) {
		return HISEE_TRUE;
	} else if (curr_ptr->img_version_num[cos_id] < previous_ptr->img_version_num[cos_id]) {
		return HISEE_FALSE;
	} else {/*cos image NV counter are equal*/
		if (curr_ptr->img_timestamp.value > previous_ptr->img_timestamp.value) {
			return HISEE_TRUE;
		} else {
			return HISEE_FALSE;
		}
	}
}

#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
/*************************************************************
函数原型：int check_cosid_valid(unsigned int cos_id, hisee_img_header *p_img_header)
函数功能：根据cos_id为输入、和系统启动时解析的hisee.img文件的信息做比较，判断cos_id是否有效
参数：
输入：cos_id:cos_id；p_img_header:指向保存系统启动时解析的hisee.img文件的信息结构体
输出：无。
返回值：1:cos_id检查OK；0:cos_id检查不OK
前置条件：系统启动时解析的hisee.img文件的信息的操作完成
后置条件： 无
*************************************************************/
static int check_cosid_valid(unsigned int cos_id)
{
	if (cos_id < HISEE_SUPPORT_COS_FILE_NUMBER) {
		return HISEE_TRUE;
	}

	return HISEE_FALSE;
}

/*************************************************************
函数原型：int second_check_newest_cosimage(unsigned int cos_id, const multicos_upgrade_info *curr_ptr)
函数功能：根据cos_id对应的cos镜像，做第二级检查返回是否是新cos镜像(根据cos镜像的升级版本号和时间戳)
参数：
输入：cos_id:当前cos镜像id；curr_ptr:指向系统前次保存在hisee_img分区的cos镜像的升级版本号和时间戳信息结构体
输出：无。
返回值：HISEE_TRUE:当前cos_id对应的cos镜像是新镜像；HISEE_FALSE:当前cos_id对应的cos镜像是老镜像
前置条件： 无
后置条件： 无
*************************************************************/
static int second_check_newest_cosimage(unsigned int cos_id, const multicos_upgrade_info *curr_ptr)
{
	int ret;
	multicos_upgrade_info previous;

	if (NULL == curr_ptr || HISEE_SUPPORT_COS_FILE_NUMBER <= cos_id) {
		return HISEE_FALSE;
	}
	(void)memset_s((void *)&previous, sizeof(multicos_upgrade_info),
					0, sizeof(multicos_upgrade_info));
	ret = access_hisee_image_partition((char *)&previous, COS_UPGRADE_INFO_READ_TYPE);
	if (HISEE_OK != ret) {
		pr_err("HISEE:%s() access_hisee_image_partition fail,ret=%d\n", __func__, ret);
		ret = HISEE_MULTICOS_READ_UPGRADE_ERROR;
		return ret;
	}
	/*sw_upgrade_version is 0, only compare the timestamp*/
	if (HISEE_DEFAULT_SW_UPGRADE_VERSION == curr_ptr->sw_upgrade_version[cos_id]) {
		if (curr_ptr->sw_upgrade_timestamp[cos_id].img_timestamp.value >
		    previous.sw_upgrade_timestamp[cos_id].img_timestamp.value)
			return HISEE_TRUE;
		else
			return HISEE_FALSE;
	} else {/*sw_upgrade_version is not 0, only compare the upgrade version*/
		if (curr_ptr->sw_upgrade_version[cos_id] > previous.sw_upgrade_version[cos_id]) {
			return HISEE_TRUE;
		} else {
			return HISEE_FALSE;
		}
	}
}

static void copy_hisee_image_sw_version(cosimage_version_info *info, hisee_partition_version_info * hisee_partition_info)
{
	unsigned int i = 0;

	info->magic = hisee_partition_info->magic;
	for (i = 0; i < HISEE_MAX_SW_VERSION_NUMBER / 2; i++) {
		info->img_version_num[i] = hisee_partition_info->img_version_num[i];
		info->img_version_num[i + HISEE_MAX_SW_VERSION_NUMBER / 2] = /*lint !e679*/
					 hisee_partition_info->img_version_num1[i];
	}
	info->img_timestamp.value = hisee_partition_info->img_timestamp.value;
}
#endif


int check_new_cosimage(unsigned int cos_id, int *is_new_cosimage)
{
	hisee_img_header local_img_header;
	cosimage_version_info curr = {0};
	cosimage_version_info previous = {0};
#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
	multicos_upgrade_info *curr_upgrade_info = NULL;
	hisee_partition_version_info hisee_partition_info = { 0 };
#endif
	int ret;

	if (NULL == is_new_cosimage) {
		pr_err("%s buf paramters is null\n", __func__);
		set_errno_and_return(HISEE_INVALID_PARAMS);
	}
	(void)memset_s((void *)&local_img_header,
					sizeof(hisee_img_header), 0, sizeof(hisee_img_header));
	ret = hisee_parse_img_header((char *)&local_img_header);
	if (HISEE_OK != ret) {
		pr_err("%s():hisee_parse_img_header failed, ret=%d\n", __func__, ret);
		set_errno_and_return(ret);
	}
#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
	if (HISEE_FALSE == check_cosid_valid(cos_id)) {
		*is_new_cosimage = HISEE_FALSE;
		return HISEE_OK;
	}
#endif
	parse_timestamp(local_img_header.time_stamp, &(curr.img_timestamp));
	curr.img_version_num[cos_id] = (unsigned char)local_img_header.sw_version_cnt[cos_id];
	curr.magic = HISEE_SW_VERSION_MAGIC_VALUE;

#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
	ret = access_hisee_image_partition((char *)&hisee_partition_info, SW_VERSION_READ_TYPE);
#else
	ret = access_hisee_image_partition((char *)&previous, SW_VERSION_READ_TYPE);
#endif
	if (HISEE_OK != ret) {
		pr_err("%s access_hisee_image_partition fail,ret=%d\n", __func__, ret);
		return ret;
	}

#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
	copy_hisee_image_sw_version(&previous, &hisee_partition_info);
#endif

	if (check_sw_version_null(&previous, cos_id)) {
		*is_new_cosimage = HISEE_TRUE;
	} else {
		*is_new_cosimage = first_check_newest_cosimage(cos_id, &curr, &previous);
#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
		/*do second phase check to find new cos image*/
		curr_upgrade_info = &(local_img_header.cos_upgrade_info);
		if (HISEE_COS_EXIST == local_img_header.is_cos_exist[cos_id]) {
			*is_new_cosimage = second_check_newest_cosimage(cos_id, curr_upgrade_info);
		} else {
			pr_err("%s: there is no image for cos%d in hisee_img!\n", __func__, cos_id);
			*is_new_cosimage = HISEE_FALSE;
		}
#endif
	}
	return HISEE_OK;
}


int misc_image_upgrade_func(void *buf, unsigned int cos_id)
{
	char *buff_virt = NULL;
	phys_addr_t buff_phy = 0;
	atf_message_header *p_message_header = NULL;
	int ret = HISEE_OK;
	unsigned int image_size;
	hisee_img_file_type type;
	unsigned int misc_image_cnt = 1;
	unsigned int result_offset;
	unsigned int max_misc_num = HISEE_MAX_MISC_IMAGE_NUMBER;
	unsigned int misc_id = cos_id;

	if (MAX_COS_IMG_ID <= cos_id) {
		pr_err("%s(): cos_id=%d invalid\n", __func__, cos_id);
		set_errno_and_return(HISEE_NO_RESOURCES);
	}
	buff_virt = (void *)dma_alloc_coherent(g_hisee_data.cma_device, HISEE_SHARE_BUFF_SIZE,
											&buff_phy, GFP_KERNEL);
	if (buff_virt == NULL) {
		pr_err("%s(): dma_alloc_coherent failed\n", __func__);
		set_errno_and_return(HISEE_NO_RESOURCES);
	}
	type = MISC0_IMG_TYPE + misc_id * max_misc_num;
	misc_image_cnt = g_hisee_data.hisee_img_head.misc_image_cnt[misc_id];
	pr_err("%s(): cos_id=%d,misc_image_cnt=%d\n", __func__, cos_id, misc_image_cnt);
do_loop:
	(void)memset_s(buff_virt, HISEE_SHARE_BUFF_SIZE, 0, HISEE_SHARE_BUFF_SIZE);
	p_message_header = (atf_message_header *)buff_virt;
	set_message_header(p_message_header, CMD_UPGRADE_MISC);
	ret = filesys_hisee_read_image(type, (buff_virt + HISEE_ATF_MESSAGE_HEADER_LEN));
	if (ret < HISEE_OK) {
		pr_err("%s(): filesys_hisee_read_image failed, ret=%d\n", __func__, ret);
		goto exit;
	}

	image_size = (unsigned int)(ret + HISEE_ATF_MESSAGE_HEADER_LEN);
	result_offset = (u32)(image_size + SMC_TEST_RESULT_SIZE - 1) & (~(u32)(SMC_TEST_RESULT_SIZE -1));
	if (result_offset + SMC_TEST_RESULT_SIZE <= HISEE_SHARE_BUFF_SIZE) {
		p_message_header->test_result_phy = (unsigned int)buff_phy + result_offset;
		p_message_header->test_result_size = (unsigned int)SMC_TEST_RESULT_SIZE;
	}
	ret = send_smc_process(p_message_header, buff_phy, image_size,
							HISEE_ATF_MISC_TIMEOUT, CMD_UPGRADE_MISC);
	if (HISEE_OK != ret) {
		pr_err("%s(): send_smc_process failed, ret=%d\n", __func__, ret);
		if (result_offset + SMC_TEST_RESULT_SIZE <= HISEE_SHARE_BUFF_SIZE) {
			pr_err("%s(): hisee reported fail code=%d\n", __func__, *((int *)(void *)(buff_virt + result_offset)));
		}
		goto exit;
	}
	if ((--misc_image_cnt) > 0) {
		type++;
		goto do_loop;
	}
exit:
	dma_free_coherent(g_hisee_data.cma_device, (unsigned long)HISEE_SHARE_BUFF_SIZE, buff_virt, buff_phy);
	check_and_print_result();
	set_errno_and_return(ret);
}/*lint !e715*/

static int cos_upgrade_prepare(const void *buf, hisee_img_file_type *img_type, unsigned int *cos_id)
{
	int ret;
	unsigned int process_id = 0;

	if (!img_type || !cos_id) {
		pr_err("%s(): input params invalid", __func__);
		set_errno_and_return(HISEE_INVALID_PARAMS);
	}

	if (!get_rpmb_key_status()) {
		pr_err("%s(): rpmb key not ready. cos upgrade bypassed", __func__);
		set_errno_and_return(HISEE_RPMB_KEY_UNREADY_ERROR);
	}

	ret = hisee_get_cosid_processid(buf, cos_id, &process_id);
	if (HISEE_OK != ret) {
		set_errno_and_return(ret);
	}

	*img_type = (*cos_id - COS_IMG_ID_0) + COS_IMG_TYPE;
	if (OTP_IMG_TYPE <= *img_type) {
		pr_err("%s(): input cos_id error(%d)", __func__, *cos_id);
		ret = HISEE_COS_IMG_ID_ERROR;
		set_errno_and_return(ret);
	}

	if (g_cos_id != *cos_id) {
		pr_err("%s(): input cos_id(%d) is diff from poweron upgrade(%d)", __func__, *cos_id, g_cos_id);
		ret = HISEE_COS_IMG_ID_ERROR;
		set_errno_and_return(ret);
	}

	return HISEE_OK;
}

static int cos_upgrade_check_version(int para, unsigned int cos_id)
{
	int ret = HISEE_OK;
	int new_cos_exist = HISEE_FALSE;

	/* hisee factory test(include slt test) don't check there is new cos image */
	if ((int)HISEE_FACTORY_TEST_VERSION != para) {
		ret = check_new_cosimage(cos_id, &new_cos_exist);
		if (HISEE_OK == ret) {
			if (HISEE_FALSE == new_cos_exist) {
				pr_err("%s(): there is no new cosimage\n", __func__);
				ret = HISEE_IS_OLD_COS_IMAGE;
			}
		} else {
			pr_err("%s(): check_new_cosimage failed,ret=%d\n", __func__, ret);
			ret = HISEE_OLD_COS_IMAGE_ERROR;
		}
	}

	return ret;
}

#ifdef CONFIG_HICOS_MISCIMG_PATCH
atomic_t g_is_patch_free = ATOMIC_INIT(0);
char *g_patch_buff_virt;
phys_addr_t g_patch_buff_phy;
int hisee_cos_patch_read(hisee_img_file_type img_type) //MISC4_IMG_TYPE
{
	int ret = HISEE_OK;
	char *buff_virt = NULL;
	phys_addr_t buff_phy = 0;
	unsigned int image_size;
	unsigned int timeout;
	atf_message_header *p_message_header = NULL;
	unsigned int is_smx_0 = 0;

	hisee_get_smx_cfg(&is_smx_0);

	if (SMX_PROCESS_0 != is_smx_0) {
		pr_err("%s(): enter, img_type=%d.\n", __func__, (int)img_type);
		if (HISEE_COS_PATCH_FREE_CNT != atomic_inc_return(&g_is_patch_free)) {
			atomic_dec(&g_is_patch_free);
			ret = HISEE_ERROR;
			set_errno_and_return(ret); /*lint !e1058*/
		}

		if (g_patch_buff_virt == NULL) {
			pr_err("%s(): alloc HISEE_SHARE_BUFF_SIZE\n", __func__);
			buff_virt = (void *)dma_alloc_coherent(g_hisee_data.cma_device, HISEE_SHARE_BUFF_SIZE,
												&buff_phy, GFP_KERNEL);
			g_patch_buff_virt = buff_virt;
			g_patch_buff_phy = buff_phy;
		} else {
			buff_virt = g_patch_buff_virt;
			buff_phy = g_patch_buff_phy;
		}
		if (buff_virt == NULL) {
			pr_err("%s(): dma_alloc_coherent failed\n", __func__);
			atomic_dec(&g_is_patch_free);
			ret = HISEE_NO_RESOURCES;
			set_errno_and_return(ret); /*lint !e1058*/
		}

		(void)memset_s(buff_virt, HISEE_SHARE_BUFF_SIZE, 0, HISEE_SHARE_BUFF_SIZE);
		p_message_header = (atf_message_header *)buff_virt;
		set_message_header(p_message_header, CMD_UPGRADE_COS_PATCH);
		ret = filesys_hisee_read_image(img_type, (buff_virt + HISEE_ATF_MESSAGE_HEADER_LEN));
		if (ret < HISEE_OK) {
			pr_err("%s(): filesys_hisee_read_image failed, ret=%d\n", __func__, ret);
			dma_free_coherent(g_hisee_data.cma_device, (unsigned long)HISEE_SHARE_BUFF_SIZE, g_patch_buff_virt, g_patch_buff_phy);
			g_patch_buff_virt = NULL;
			g_patch_buff_phy = 0;
			atomic_dec(&g_is_patch_free);
			set_errno_and_return(ret); /*lint !e1058*/
		}
		image_size = (unsigned int)(ret + HISEE_ATF_MESSAGE_HEADER_LEN);

		timeout = (unsigned int)HISEE_ATF_COS_TIMEOUT;

		pr_err("%s(): send_smc_process-->CMD_UPGRADE_COS_PATCH\n", __func__);
		ret = send_smc_process(p_message_header, buff_phy, image_size,
								timeout, CMD_UPGRADE_COS_PATCH);

		/*free is in hisee_check_ready_show()*/

		atomic_dec(&g_is_patch_free);
		pr_err("%s(): exit, img_type=%d.\n", __func__, (int)img_type);
	}
	return ret;
}

void cos_patch_upgrade(const void *buf)
{
	int ret = HISEE_OK;
	unsigned int cos_id = COS_IMG_ID_0;
	unsigned int process_id;
	unsigned int is_smx_0 = 0;

	hisee_get_smx_cfg(&is_smx_0);

	if (SMX_PROCESS_0 != is_smx_0) {
		ret = hisee_get_cosid_processid(buf, &cos_id, &process_id);
		if (HISEE_OK != ret) {
			pr_err("%s(): hisee_get_cosid_processid failed\n", __func__);
			return;
		}

		if (COS_IMG_ID_0 != cos_id) {
			/*TODO:can do more action in future if necessary*/
			pr_err("%s(): cos_id=%d bypass cos_patch_upgrade.\n", __func__, cos_id);
			return;
		}

		ret = hisee_poweroff_func(buf, HISEE_PWROFF_LOCK);
		if (HISEE_OK != ret) {
			pr_err("%s() hisee_poweroff_func failed. ret=%d\n", __func__, ret);
		}

		ret = hisee_cos_patch_read(MISC3_IMG_TYPE);
		if (HISEE_OK != ret) {
			pr_err("%s(): hisee_cos_patch_read failed ret=%x\n", __func__, ret);
		}

		ret = hisee_poweron_booting_func(buf, 0);
		if (HISEE_OK != ret) {
			pr_err("%s(): hisee_poweron_booting_func failed ret=%x\n", __func__, ret);
		}

		/* wait hisee cos ready for later process */
		ret = wait_hisee_ready(HISEE_STATE_COS_READY, HISEE_ATF_GENERAL_TIMEOUT);
		if (HISEE_OK != ret) {
			pr_err("%s(): wait_hisee_ready failed ret=%x\n", __func__, ret);
		}
	}
	check_and_print_result_with_cosid();
}/*lint !e715 !e838*/
#endif

/*
 * @brief      : read the upgrade hisee iamge from hisee_img partition and
 *               send to atf.
 * @param[in]  : cos_id, the cos index correspond to the image.
 * @param[in]  : img_type, the image type, like cos, otp, misc, uloader...
 * @param[in]  : smc_cmd, the smc command type, like COS_UPGRADE,
 *               ULOADER_UPGRADE...
 * @return     : int
 * @note       :
 */
int hisee_upgrade_image_read(unsigned int cos_id, hisee_img_file_type img_type,
			     se_smc_cmd smc_cmd)
{
	int ret;
	char *buff_virt = NULL;
	phys_addr_t buff_phy = 0;
	unsigned int image_size;
	atf_message_header *p_message_header = NULL;
#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
	size_t file_size = 0;
#endif

	buff_virt = (void *)dma_alloc_coherent(g_hisee_data.cma_device,
					       HISEE_SHARE_BUFF_SIZE,
					       &buff_phy, GFP_KERNEL);
	if (!buff_virt) {
		pr_err("%s(): dma_alloc_coherent failed\n", __func__);
		ret = HISEE_NO_RESOURCES;
		set_errno_and_return(ret);
	}

	(void)memset_s(buff_virt, HISEE_SHARE_BUFF_SIZE, 0,
		       HISEE_SHARE_BUFF_SIZE);
	p_message_header = (atf_message_header *)buff_virt;
	set_message_header(p_message_header, smc_cmd);
#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
	/* only for read cos_flash image */
	if (cos_id == COS_FLASH_IMG_ID && img_type == COS_FLASH_IMG_TYPE) {
		ret = filesys_read_img_from_file(HISEE_COS_FLASH_IMG_FULLNAME,
						 (buff_virt +
						 HISEE_ATF_MESSAGE_HEADER_LEN),
						 &file_size, HISEE_MAX_IMG_SIZE);
		check_result_and_goto(ret, exit);
		image_size = (unsigned int)(file_size +
					    HISEE_ATF_MESSAGE_HEADER_LEN);
	} else
#endif
	{
		ret = filesys_hisee_read_image(img_type, (buff_virt +
					       HISEE_ATF_MESSAGE_HEADER_LEN));
		if (ret < HISEE_OK) {
			pr_err("%s(): hisee_read_image failed, ret=%d\n",
				__func__, ret);
			goto exit;
		}
		image_size = (unsigned int)(ret + HISEE_ATF_MESSAGE_HEADER_LEN);
	}
	p_message_header->ack = cos_id;

	ret = send_smc_process(p_message_header, buff_phy, image_size,
			       g_hisee_cos_upgrade_time, smc_cmd);
#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
	if ((ret == HISEE_OK && cos_id >= COS_IMG_ID_3) &&
	    (COS_FLASH_IMG_TYPE != img_type))
		ret = hisee_encos_write((buff_virt + HISEE_ATF_MESSAGE_HEADER_LEN),
					(image_size - HISEE_ATF_MESSAGE_HEADER_LEN),
					cos_id);
#endif
exit:
	dma_free_coherent(g_hisee_data.cma_device,
			  (unsigned long)HISEE_SHARE_BUFF_SIZE,
			  buff_virt, buff_phy);
	check_and_print_result();
	return ret;
}

/*
 * @brief      : cos_upgrade_image_read
 * @param[in]  : cos_id , the cos index
 * @param[in]  : img_type , the image file type, like cos0,cos1...
 * @return     : int
 * @note       :
 */
int cos_upgrade_image_read(unsigned int cos_id, hisee_img_file_type img_type)
{
	int ret;
	se_smc_cmd smc_cmd;

	pr_err("%s: cos_id=%x, img_type=%x\n", __func__, cos_id, img_type);
	/* Do the cos image upgrade. */
	smc_cmd = CMD_UPGRADE_COS;
	ret = hisee_upgrade_image_read(cos_id, img_type, smc_cmd);
	check_result_and_goto(ret, out);

out:
	check_and_print_result();
	return ret;
}

static int cos_upgrade_basic_check_param(unsigned int cos_id, hisee_img_file_type img_type)
{
	if (img_type > OTP_IMG_TYPE || cos_id >= MAX_COS_IMG_ID) {
		pr_err("hisee:%s(): params is invalid\n", __func__);
		return HISEE_COS_IMG_ID_ERROR;
	}

	return HISEE_OK;
}

/*************************************************************
函数原型：int cos_image_upgrade_basic_process(void *buf, int para,
							unsigned int cos_id, hisee_img_file_type img_type)
函数功能：根据cos_id升级cos镜像的基本处理实现，升级成功的话更新hisee_img分区最后1K字节的标志区域。
		  升级失败有retry机制来增加健壮性。
参数：
输入：cos_id:当前cos镜像id；buf:保存输入参数的buffer；
	  para:功能参数，指示是否是产线升级操作；img_type:当前cos镜像的image type。
输出：无。
返回值：HISEE_OK:cos镜像升级成功；非HISEE_OK:cos镜像升级失败
前置条件：执行cos镜像升级启动命令成功,并且hisee_img分区有更新的hisee.img文件
后置条件：需要执行hisee下电操作。
*************************************************************/
static int cos_image_upgrade_basic_process(const void *buf, int para,
						unsigned int cos_id, hisee_img_file_type img_type)
{
	int ret, ret1, ret2;
	unsigned int upgrade_run_flg = 0;
	int retry = 2; /* retry 2 more times if failed */
#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
	multicos_upgrade_info store_upgrade_info;
	multicos_upgrade_info *p_upgrade_info = NULL;
	hisee_partition_version_info curr = { 0 };
#else
	cosimage_version_info curr = {0};
#endif
	ret = cos_upgrade_basic_check_param(cos_id, img_type);
	if (HISEE_OK != ret) {
		return ret;
	}

#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
	(void)memset_s((void *)&store_upgrade_info, sizeof(multicos_upgrade_info),
					0, sizeof(multicos_upgrade_info));
	access_hisee_image_partition((char *)&store_upgrade_info, COS_UPGRADE_INFO_READ_TYPE);
#endif
	upgrade_run_flg = HISEE_COS_UPGRADE_RUNNING_FLG;
	access_hisee_image_partition((char *)&upgrade_run_flg, COS_UPGRADE_RUN_WRITE_TYPE);

upgrade_retry:
	ret = cos_upgrade_image_read(cos_id, img_type);
	if (HISEE_OK == ret) {
		access_hisee_image_partition((char *)&curr, SW_VERSION_READ_TYPE);
		parse_timestamp(g_hisee_data.hisee_img_head.time_stamp, &(curr.img_timestamp));
		if (!check_timestamp_valid(&(curr.img_timestamp))) {
			ret = HISEE_INVALID_PARAMS;
			pr_err("%s(): check_timestamp_valid failed\n", __func__);
			return ret;
		}

		#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
		if ((HISEE_MAX_SW_VERSION_NUMBER / 2) <= cos_id) {
			curr.img_version_num1[cos_id - (HISEE_MAX_SW_VERSION_NUMBER / 2)] =
					 g_hisee_data.hisee_img_head.sw_version_cnt[cos_id];
		} else {
			curr.img_version_num[cos_id] = g_hisee_data.hisee_img_head.sw_version_cnt[cos_id];
		}
		#else
		curr.img_version_num[cos_id] = g_hisee_data.hisee_img_head.sw_version_cnt[cos_id];
		#endif

		curr.magic = HISEE_SW_VERSION_MAGIC_VALUE;
		access_hisee_image_partition((char *)&curr, SW_VERSION_WRITE_TYPE);
		upgrade_run_flg = 0;
		access_hisee_image_partition((char *)&upgrade_run_flg, COS_UPGRADE_RUN_WRITE_TYPE);

		#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
		p_upgrade_info = &(g_hisee_data.hisee_img_head.cos_upgrade_info);
		store_upgrade_info.sw_upgrade_version[cos_id] = p_upgrade_info->sw_upgrade_version[cos_id];
		store_upgrade_info.sw_upgrade_timestamp[cos_id].img_timestamp.value =
				p_upgrade_info->sw_upgrade_timestamp[cos_id].img_timestamp.value;
		access_hisee_image_partition((char *)&store_upgrade_info, COS_UPGRADE_INFO_WRITE_TYPE);
		#endif

		hisee_mntn_update_local_ver_info();
		pr_err("hisee:%s(): upgrade_exit,cos_id=%d\n", __func__, cos_id);

		#ifdef CONFIG_HICOS_MISCIMG_PATCH
		if ((COS_IMG_ID_0 == cos_id) && ((int)HISEE_FACTORY_TEST_VERSION != para)) {
			cos_patch_upgrade(buf);
		}
		#endif
	} else {
		g_hisee_data.hisee_img_head.sw_version_cnt[cos_id] = 0;
		ret1 = hisee_exception_to_reset_rpmb();
		if (HISEE_OK != ret1) {
			pr_err("%s ERROR:fail to reset rpmb,cos_id=%d,ret=%d\n", __func__, cos_id, ret1);
			ret1 = atomic_read(&g_hisee_errno);
#ifdef CONFIG_HISI_HISEE_MNTN
			rdr_hisee_call_to_record_exc(ret1);
#endif
			return ret1;
		}
		while (0 < retry) {
			pr_err("hisee:%s() cos_id=%d,failed and retry=%d,ret=%d\n", __func__, cos_id, retry, ret);
			retry--;
			ret1 = hisee_poweroff_func(buf, HISEE_PWROFF_LOCK);
			hisee_mdelay(200);
			ret2 = hisee_poweron_upgrade_func(buf, 0);
			hisee_mdelay(200);
			if (HISEE_OK != ret1 || HISEE_OK != ret2) {
				continue;
			}
			goto upgrade_retry;
		}

	}
	check_and_print_result_with_cosid();
	set_errno_and_return(ret);
}/*lint !e715*/

int handle_cos_image_upgrade(const void *buf, int para)
{
	int ret;
	unsigned int cos_id = COS_IMG_ID_0;
	hisee_img_file_type img_type;
	int ret_tmp;

	ret = cos_upgrade_prepare(buf, &img_type, &cos_id);/* [false alarm]: set value in function */
	if (HISEE_OK != ret) {
		goto upgrade_bypass;
	}

	/* hisee factory test(include slt test) don't check there is new cos image */
	ret = cos_upgrade_check_version(para, cos_id);
	if (HISEE_OK != ret) {
		if (HISEE_IS_OLD_COS_IMAGE == ret) {
			pr_err("%s(): is old cosimage\n", __func__);
		}
		goto upgrade_bypass;
	}

	ret = cos_image_upgrade_basic_process(buf, para, cos_id, img_type);
	if (HISEE_OK != ret) {
		pr_err("%s(): cos_image_upgrade_basic_process failed,ret=%d\n", __func__, ret);
	}

upgrade_bypass:
	ret_tmp = hisee_poweroff_func(buf, HISEE_PWROFF_LOCK);
	if (HISEE_OK != ret_tmp) {
		pr_err("hisee:%s() cos_id=%d, poweroff failed. ret=%d\n", __func__, cos_id, ret_tmp);
		if (HISEE_OK == ret) {
			ret = ret_tmp;
		}
	}
	check_and_print_result_with_cosid();
	set_errno_and_return(ret);
}/*lint !e715*/

int cos_image_upgrade_func(const void *buf, int para)
{
	int ret;
	char buf_para[MAX_CMD_BUFF_PARAM_LEN] = {0};

	/* check hisee is powered on */
	if (hisee_get_power_status() != HISEE_POWER_STATUS_ON) {
		pr_err("%s hisee is not poweron\n", __func__);
		set_errno_and_return(HISEE_POWERCTRL_FLOW_ERROR); /*lint !e1058*/
	}

#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS

	/*if the @buf parameters is NULL ,need do all cos image upagrade while multicos scenario*/
	if (HISEE_CHAR_NEWLINE == *(char *)buf || '\0' == *(char *)buf) {
		buf_para[0] = HISEE_CHAR_SPACE;
		buf_para[1] = '0' + COS_IMG_ID_0;
		buf_para[2] = '0' + COS_PROCESS_UPGRADE;
		pr_err("hisee:%s() enter cos self-upgrade process, need to poweroff hisee in advance\n", __func__);
		ret = hisee_poweroff_func((void *)buf_para, 0);
		if (HISEE_OK != ret) {
			pr_err("hisee:%s() poweroff failed. retcode=%d\n", __func__, ret);
			return ret;
		}
		ret = cos_image_upgrade_by_self();
		check_and_print_result();
		return ret;
	}
	ret = handle_cos_image_upgrade(buf, para);
#else
	buf_para[0] = HISEE_CHAR_SPACE;
	buf_para[1] = '0' + COS_IMG_ID_0;/* '0': int to char */
	buf_para[2] = '0' + COS_PROCESS_UPGRADE;/* '0': int to char */

	ret = handle_cos_image_upgrade((void *)buf_para, para);
#endif
	check_and_print_result();
	return ret;
}

ssize_t hisee_has_new_cos_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int new_cos_exist = 0;
	unsigned int i, cos_cnt;
	hisee_img_header local_img_header;
	int ret;

	if (NULL == buf) {
		pr_err("%s buf paramters is null\n", __func__);
		set_errno_and_return(HISEE_INVALID_PARAMS);
	}

	(void)memset_s((void *)&local_img_header, sizeof(hisee_img_header),
					0, sizeof(hisee_img_header));
	ret = hisee_parse_img_header((char *)(&local_img_header));
	if (HISEE_OK != ret) {
		pr_err("%s():hisee_parse_img_header failed, ret=%d\n", __func__, ret);
		set_errno_and_return(ret);
	}

	cos_cnt = local_img_header.rpmb_cos_cnt + local_img_header.emmc_cos_cnt;
	for (i = 0; i < cos_cnt; i++) {
		ret = check_new_cosimage(i, &new_cos_exist);
		if (HISEE_OK == ret) {
			if (1 == new_cos_exist) {/*lint !e774*/
				ret = snprintf_s(buf, HISEE_BUF_SHOW_LEN, (u64)10, "cos-%d %d,", i, 0);
				if (ret == HISEE_SECLIB_ERROR) {
					pr_err("%s(): snprintf1 err.\n", __func__);
					set_errno_and_return(HISEE_SECUREC_ERR);
				}
				ret = strncat_s(buf, HISEE_BUF_SHOW_LEN, "exsited new cosimage",
								(unsigned long)strlen("exsited new cosimage"));
				if (ret != EOK) {
					pr_err("%s(): strncat err.\n", __func__);
					set_errno_and_return(HISEE_SECUREC_ERR);
				}
			} else {
				ret = snprintf_s(buf, HISEE_BUF_SHOW_LEN, (u64)10, "cos-%d %d,", i, 1);
				if (ret == HISEE_SECLIB_ERROR) {
					pr_err("%s(): snprintf1 err.\n", __func__);
					set_errno_and_return(HISEE_SECUREC_ERR);
				}
				ret = strncat_s(buf, HISEE_BUF_SHOW_LEN, "no exsited new cosimage",
								(unsigned long)strlen("no exsited new cosimage"));
				if (ret != EOK) {
					pr_err("%s(): strncat err.\n", __func__);
					set_errno_and_return(HISEE_SECUREC_ERR);
				}
			}
		} else {
			ret = snprintf_s(buf, HISEE_BUF_SHOW_LEN, (u64)10, "cos-%d %d,", i, -1);
			if (ret == HISEE_SECLIB_ERROR) {
				pr_err("%s(): snprintf1 err.\n", __func__);
				set_errno_and_return(HISEE_SECUREC_ERR);
			}
			ret = strncat_s(buf, HISEE_BUF_SHOW_LEN, "failed", (unsigned long)strlen("failed"));
			if (ret != EOK) {
				pr_err("%s(): strncat err.\n", __func__);
				set_errno_and_return(HISEE_SECUREC_ERR);
			}
		}
	}
	pr_err("%s(): %s\n", __func__, buf);
	return (ssize_t)strlen(buf);
}/*lint !e715*/

ssize_t hisee_check_upgrade_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned int upgrade_run_flg = 0;
	int ret;

	if (NULL == buf) {
		pr_err("%s buf paramters is null\n", __func__);
		set_errno_and_return(HISEE_INVALID_PARAMS); /*lint !e1058*/
	}

	access_hisee_image_partition((char *)&upgrade_run_flg, COS_UPGRADE_RUN_READ_TYPE);
	if (0 == upgrade_run_flg) {
		ret = snprintf_s(buf, HISEE_BUF_SHOW_LEN, (size_t)HISEE_BUF_SHOW_LEN, "0,cos upgrade success");
		if (ret == HISEE_SECLIB_ERROR) {
			pr_err("%s(): snprintf1 err.\n", __func__);
			set_errno_and_return(HISEE_SECUREC_ERR);
		}
	} else if (HISEE_COS_UPGRADE_RUNNING_FLG == upgrade_run_flg) {
		ret = snprintf_s(buf, HISEE_BUF_SHOW_LEN, (size_t)HISEE_BUF_SHOW_LEN, "1,cos upgrade failed last time");
		if (ret == HISEE_SECLIB_ERROR) {
			pr_err("%s(): snprintf2 err.\n", __func__);
			set_errno_and_return(HISEE_SECUREC_ERR);
		}
	} else {
		ret = snprintf_s(buf, HISEE_BUF_SHOW_LEN, (size_t)HISEE_BUF_SHOW_LEN, "-1,failed");
		if (ret == HISEE_SECLIB_ERROR) {
			pr_err("%s(): snprintf3 err.\n", __func__);
			set_errno_and_return(HISEE_SECUREC_ERR);
		}
	}

	pr_err("%s(): %s\n", __func__, buf);
	return (ssize_t)strlen(buf);
}/*lint !e715*/


