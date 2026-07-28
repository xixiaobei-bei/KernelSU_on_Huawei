/****************************************************************************//**
 * @file   : /hisi/ap/kernel/drivers/hisi/hisee/hisi_hisee_shutdown_swipe.c
 * @brief  :
 * @par    : Copyright(c) 2018-2034, HUAWEI Technology Co., Ltd.
 * @date   : 2018/02/11
 * @author : w00382529
 * @note   :
********************************************************************************/
#include <soc_sctrl_interface.h>
#include <soc_acpu_baseaddr_interface.h>
#include <asm/io.h>
#include <hisi_mailbox.h>
#include <linux/mfd/hisi_pmic.h>
#include "hisi_hisee_shutdown_swipe.h"
#include "hisi_hisee.h"
#include <hisi_partition.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/hisi/kirin_partition.h>
#include <linux/syscalls.h>

/*===============================================================================
 *                                 types/macros                                *
===============================================================================*/



/*===============================================================================
 *                                global objects                               *
===============================================================================*/

/****************************************************************************//**
 * @brief      : pdswipe_feature_switch_is_enable
 * @param[in]  : NA
 * @return     : ::int
 * @note       :
********************************************************************************/
static int pdswipe_feature_switch_is_enable(void)
{
	unsigned int switch_state;

	switch_state = (unsigned int)atfd_hisee_smc((u64)HISEE_FN_MAIN_SERVICE_CMD,
					(u64)CMD_HISEE_GET_EFUSE_VALUE, (u64)0, (u64)0);
	/* 0xffffffff equal to -1, error */
	if (0xffffffff == switch_state) {
			pr_err("%s atfd_hisee_smc FAIL\n", __func__);
			return HISEE_FALSE;
	} else {
		if (EFUSE_PDSWIPE_SWITCH_ENABLE_VALUE == switch_state) {
			return HISEE_TRUE;
		} else {
			return HISEE_FALSE;
		}
	}
}
/****************************************************************************//**
 * @brief      : inse_shutdown_swipe_t_nfc_en_func
 * @param[in]  : buf
 * @param[in]  : para
 * @return     : ::int
 * @note       :
********************************************************************************/
int inse_shutdown_swipe_t_nfc_en_func(void *buf, int para)
{
	unsigned int reg_value;

	if (HISEE_TRUE == pdswipe_feature_switch_is_enable()) {
		/* 使能PMU响应NFC指示信号上电的功能 */
		reg_value = hisi_pmic_reg_read(NP_NFC_PWRON_MASK);
		if (NFC_RESPOND_NFC_DIS == (reg_value & NFC_RESPOND_NFC_EN)) {
			hisi_pmic_reg_write(NP_NFC_PWRON_MASK, NFC_RESPOND_NFC_EN);
		}
	}

	return HISEE_OK;
}

/****************************************************************************//**
 * @brief      : inse_shutdown_swipe_t_nfc_dis_func
 * @param[in]  : buf
 * @param[in]  : para
 * @return     : ::int
 * @note       :
********************************************************************************/
int inse_shutdown_swipe_t_nfc_dis_func(void *buf, int para)
{
	unsigned int reg_value;

	/* 禁止PMU响应NFC指示信号上电的功能 */
	reg_value = hisi_pmic_reg_read(NP_NFC_PWRON_MASK);
	if (NFC_RESPOND_NFC_EN == (reg_value & NFC_RESPOND_NFC_EN)) {
		hisi_pmic_reg_write(NP_NFC_PWRON_MASK, NFC_RESPOND_NFC_DIS);
	}

	return HISEE_OK;
}

/****************************************************************************//**
 * @brief      : hisee_pdswipe_flash_open
 * @param[in]  : flags
 * @return     : ::int
 * @note       :
********************************************************************************/
static int hisee_pdswipe_flash_open(int flags)
{
	void *buf;
	char p_name[HISEE_BUF_SHOW_LEN + 1];
	int ret, fd_dfx;

	buf = kzalloc(SZ_4K, GFP_KERNEL);
	if (!buf) {
		pr_err("%s()-line=%d\n", __func__, __LINE__);
		return -ENOMEM;
	}
	if (1 == g_hisee_partition_byname_find) {
		ret = flash_find_ptn(HISEE_IMAGE_PARTITION_NAME, buf);
		if (0 != ret) {
			pr_err("%s()-line=%d\n", __func__, __LINE__);
		}
	} else {
		flash_find_hisee_ptn(HISEE_IMAGE_A_PARTION_NAME, buf);
	}
	memset(p_name, 0, sizeof(p_name));
	strncpy(p_name, buf, sizeof(p_name));
	p_name[HISEE_BUF_SHOW_LEN] = '\0';
	kfree(buf);
	buf = NULL;
	pr_err("%s()-line=%d, hisee img partition name is %s\n", __func__, __LINE__, p_name);
	fd_dfx = sys_open(p_name, flags, HISEE_FILE_LIMIT);
	if (fd_dfx < 0) {
		pr_err("%s()-line=%d\n", __func__, __LINE__);
		ret = HISEE_OPEN_FILE_ERROR;
		set_errno_and_return(ret);
	}

	return fd_dfx;
}


/****************************************************************************//**
 * @brief      : hisee_pdswipe_get_flash_data
 * @param[in]  : buf
 * @param[in]  : buf_size
 * @param[in]  : flash_offset
 * @return     : ::u32
 * @note       :
********************************************************************************/
u32 hisee_pdswipe_get_flash_data(void *buf, u32 buf_size, u32 flash_offset)
{
	int ret, fd_flash;
	u32 cnt=0;
	mm_segment_t old_fs;

	if (!buf || 0 == buf_size) {
		pr_err("%s()-line=%d\n", __func__, __LINE__);
		return 0;
	}

	old_fs = get_fs(); /*lint !e501*/
	set_fs(KERNEL_DS); /*lint !e501*/

	fd_flash = hisee_pdswipe_flash_open(O_RDONLY);
	if (fd_flash >= 0) {
		ret = sys_lseek(fd_flash, flash_offset, SEEK_SET);
		if (ret < 0) {
			pr_err("%s()-line=%d, ret=%d, flash_offset=%x\n", __func__, __LINE__, ret, flash_offset);
			goto close;
		}
		cnt = (u32)sys_read(fd_flash, buf, buf_size);
		if (cnt != buf_size) {
			pr_err("%s()-line=%d, cnt=%d\n", __func__, __LINE__, cnt);
			goto close;
		}
	} else {
		pr_err("%s()-line=%d, fd_flash=%d\n", __func__, __LINE__, fd_flash);
		set_fs(old_fs);
		return 0;
	}
close:
	sys_close(fd_flash);
	set_fs(old_fs); /*lint !e501*/
	return cnt;
}


/****************************************************************************//**
 * @brief      : hisee_pdswipe_add_flash_data
 * @param[in]  : p_buf
 * @param[in]  : buf_size
 * @param[in]  : flash_offset
 * @return     : void
 * @note       :
********************************************************************************/
void hisee_pdswipe_add_flash_data(void *p_buf, u32 buf_size, u32 flash_offset)
{
	int ret, fd_flash, cnt=0;
	mm_segment_t old_fs;

	if (NULL == p_buf) {
		pr_err("%s()-line=%d\n", __func__, __LINE__);
		return;
	}

	old_fs = get_fs(); /*lint !e501*/
	set_fs(KERNEL_DS); /*lint !e501*/

	fd_flash = hisee_pdswipe_flash_open(O_WRONLY);
	if (fd_flash >= 0) {
		ret = sys_lseek(fd_flash, flash_offset, SEEK_SET);
		if (ret < 0) {
			pr_err("%s()-line=%d, ret=%d\n", __func__, __LINE__, ret);
			goto close;
		}
		cnt = sys_write(fd_flash, p_buf, buf_size);
		if (cnt != buf_size) {
			pr_err("%s()-line=%d, cnt=%d\n", __func__, __LINE__, cnt);
			goto close;
		}
	} else {
		pr_err("%s()-line=%d\n", __func__, __LINE__);
		set_fs(old_fs); /*lint !e501*/
		return;
	}
close:
	sys_close(fd_flash);
	set_fs(old_fs); /*lint !e501*/
}

/****************************************************************************//**
 * @brief      : hisee_pdswipe_clear_exception_record
 * @param[in]  : NA
 * @return     : void
 * @note       :
********************************************************************************/
void hisee_pdswipe_clear_exception_record(void)
{
	void *p_buf = NULL;

	p_buf = kzalloc(PDSWIPE_EXC_SIZE, GFP_KERNEL);
	if (NULL == p_buf) {
		pr_err("%s()-line=%d\n", __func__, __LINE__);
		return;
	}
	hisee_pdswipe_add_flash_data(p_buf, PDSWIPE_EXC_SIZE, PDSWIPE_EXC_OFFSET);
	kfree(p_buf);
	p_buf = NULL;
}

/****************************************************************************//**
 * @brief      : hisee_pdswipe_record_show
 * @param[in]  : dev
 * @param[in]  : attr
 * @param[in]  : buf
 * @return     : ::ssize_t
 * @note       :
********************************************************************************/
ssize_t hisee_pdswipe_record_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	void *p_buf = NULL;
	u32	cnt=0;

	p_buf = kzalloc(PDSWIPE_RECORD_SIZE, GFP_KERNEL);
	if (NULL == p_buf) {
		pr_err("%s()-line=%d\n", __func__, __LINE__);
		return 0;
	}
	/*todo:copy data from emmc, to save in user buffer*/
	cnt = hisee_pdswipe_get_flash_data(p_buf, PDSWIPE_RECORD_SIZE, PDSWIPE_RECORD_OFFSET);
	if (cnt > 0) {
		if (memcpy((void *)buf, (const void *)p_buf, min_t(size_t, PDSWIPE_RECORD_SIZE, PAGE_SIZE))) {
			pr_err("%s()-line=%d\n", __func__, __LINE__);
		}
	} else {
		pr_err("%s()-line=%d\n", __func__, __LINE__);
	}

	kfree(p_buf);
	p_buf = NULL;

	return (ssize_t)cnt;
}/*lint !e715*/

/****************************************************************************//**
 * @brief      : hisee_pdswipe_exc_show
 * @param[in]  : dev
 * @param[in]  : attr
 * @param[in]  : buf
 * @return     : ::ssize_t
 * @note       :
********************************************************************************/
ssize_t hisee_pdswipe_exc_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	void *p_buf = NULL;
	u32	cnt=0;

	p_buf = kzalloc(PDSWIPE_EXC_SIZE, GFP_KERNEL);
	if (NULL == p_buf) {
		pr_err("%s()-line=%d\n", __func__, __LINE__);
		return 0;
	}
	/*todo:copy data from emmc, to save in user buffer*/
	cnt = hisee_pdswipe_get_flash_data(p_buf, PDSWIPE_EXC_SIZE, PDSWIPE_EXC_OFFSET);
	if (cnt > 0) {
		if (memcpy((void *)buf, (const void *)p_buf, min_t(size_t, PDSWIPE_EXC_SIZE, PAGE_SIZE))) {
			pr_err("%s():-line=%d\n", __func__, __LINE__);
		}
	} else {
		pr_err("%s()-line=%d\n", __func__, __LINE__);
	}

	kfree(p_buf);
	p_buf = NULL;

	return (ssize_t)cnt;
}/*lint !e715*/


