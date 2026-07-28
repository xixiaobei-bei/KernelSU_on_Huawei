/****************************************************************************//**
 * @file   : /hisi/ap/kernel/drivers/hisi/hisee/hisi_hisee_shutdown_swipe.h
 * @brief  :
 * @par    : Copyright(c) 2018-2034, HUAWEI Technology Co., Ltd.
 * @date   : 2018/02/11
 * @author : w00382529
 * @note   :
********************************************************************************/
#ifndef __HISI_HISEE_SHUTDOWN_SWIPE_H__
#define __HISI_HISEE_SHUTDOWN_SWIPE_H__

#include <hisi_mailbox.h>


/*===============================================================================
 *                                 types/macros                                *
===============================================================================*/
#ifdef CONFIG_FAKE_HISEE_SHUTDOWN_SWIPE_SUPPORT
#define IS_INSE_SHUTDOWN_SWIPE_FAKE         (0xacca5aa5)
#define SHUTDOWN_SWIPE_LOWPOWER_TH_OFFSET   (8)
#define SHUTDOWN_SWIPE_XLOADER_INSE_VALID   (0x5a0c1cf0)
#define SHUTDOWN_SWIPE_XLOADER_INSE_INVALID (0xaa0c1cf0)
#endif

/* Hi6421V700_nManager, NFC_PWR_EN管脚拉高是否触发开机控制位寄存器地址 */
#define NP_NFC_PWRON_MASK        0x02EA
#define NFC_RESPOND_NFC_EN       1
#define NFC_RESPOND_NFC_DIS      0

/* shutdown swipe feature switch, 2bit(bit 4082-4081 in efuse) */
#define EFUSE_PDSWIPE_SWITCH_ENABLE_VALUE  (0x1)
#define EFUSE_PDSWIPE_SWITCH_INIT_VALUE    (0x0)


#define HISEE_FILE_LIMIT         0660

/*the hisee img partition in the flash is 4M, and the last 1K is left for cos version information,
 *so we use the 16K and 16K area before cos version information for swiping records and exception records
 */
#define PDSWIPE_RECORD_SIZE      0x4000
#define PDSWIPE_EXC_SIZE         0x4000

#define PDSWIPE_RECORD_OFFSET    (HISEE_IMG_PARTITION_SIZE - 0x4000 * 2 - 0x400) /*0x400000 - 0x400 - 0x4000 * 2*/
#define PDSWIPE_EXC_OFFSET       (PDSWIPE_RECORD_OFFSET + PDSWIPE_RECORD_SIZE)   /*0x400000 - 0x400 - 0x4000 * 2 + 0x4000*/

#define HISEE_EXC_VALID_MAGIC    0x5a5a1357	/*the exception data is valid if magic is equal to this value*/
#define HISEE_RECORD_VALID_MAGIC 0x13575a5a	/*the swipe record data is valid if magic is equal to this value*/
#define HISEE_RECORD_SUCCESS     0x5a5a5a5a	/*the result of swipe record, SUCCESS*/
#define HISEE_RECORD_FAIL        0xa5a5a5a5	/*the result of swipe record, FAIL*/

/*===============================================================================
 *                                global objects                               *
===============================================================================*/
#ifdef CONFIG_FAKE_HISEE_SHUTDOWN_SWIPE_SUPPORT
typedef enum _HISEE_HUTDOWN_SWIPE_FLAG {
	SHUTDOWN_SWIPE = 0xA5000000,
} hisee_shutdown_swipe_flag;
#endif

typedef struct _hisee_pdswipe_exc_head {
	u32	magic;
	u32	number;
	u32	cur_index;
	u32	reserved;
} hisee_pdswipe_exc_head;

typedef struct _hisee_pdswipe_exc_record {
	u32	time;
	u32	exc_type; /*soft exc or irq exc*/
	u32	fi;
	u32	fv;
	u32	ss;
	u32	sc;
	u32	exc_info; /*exc info: soft exc definition or irq value*/
	u32	reserved1;
} hisee_pdswipe_exc_record;

typedef struct _hisee_pdswipe_swipe_record_head {
	u32	magic;
	u32	number;
	u32	cur_index;
	u32	reserved;
} hisee_pdswipe_swipe_record_head;

typedef struct _hisee_pdswipe_swipe_record {
	u32	time;
	u32	result;
	u32	reserved1;
	u32	reserved2;
} hisee_pdswipe_swipe_record;

/*===============================================================================
 *                                  functions                                  *
===============================================================================*/

#ifdef CONFIG_FAKE_HISEE_SHUTDOWN_SWIPE_SUPPORT
int hisi_pm_off_fake_flag_send2lpmcu(rproc_msg_t timeout, rproc_msg_t lowpower_th);
int inse_shutdown_fake_func(void *buf, int para);
#endif
int inse_shutdown_swipe_t_nfc_en_func(void *buf, int para);
int inse_shutdown_swipe_t_nfc_dis_func(void *buf, int para);
u32 hisee_pdswipe_get_flash_data(void *buf, u32 buf_size, u32 flash_offset);
void hisee_pdswipe_clear_exception_record(void);
void hisee_pdswipe_add_flash_data(void *p_buf, u32 buf_size, u32 flash_offset);
ssize_t hisee_pdswipe_record_show(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t hisee_pdswipe_exc_show(struct device *dev, struct device_attribute *attr, char *buf);


#endif
