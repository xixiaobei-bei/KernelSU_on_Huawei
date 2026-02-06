/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: implement hisee file system function
 * Create: 2020-02-17
 */
#ifndef HISEE_FS_H
#define HISEE_FS_H
#include <linux/types.h>
#include <linux/version.h>
#include <linux/syscalls.h>

#ifdef CONFIG_HISEE_DISABLE_KEY
#define HISEE_FS_PARTITION_NAME      "hisee_fs"
#define HISEE_IMAGE_PARTITION_NAME   HISEE_FS_PARTITION_NAME
#else
#define HISEE_FS_PARTITION_NAME      "/mnt/hisee_fs/"
#define HISEE_IMAGE_PARTITION_NAME   "hisee_img"
#endif

#define HISEE_IMAGE_A_PARTION_NAME   "hisee_img_a"
#define HISEE_IMG_PARTITION_SIZE     (4 * SIZE_1M)
#define HISEE_SW_VERSION_MAGIC_VALUE  0xa5a55a5a

#define HISEE_ENCOS_PARTITION_NAME  "hisee_encos"

#define HISEE_MIN_MISC_IMAGE_NUMBER  1
#ifdef CONFIG_HICOS_MISCIMG_PATCH
#define HISEE_MAX_MISC_IMAGE_NUMBER  5
#define HISEE_MISC_NO_UPGRADE_NUMBER 2 /* contains: cos patch and misc_version img */
#else
#define HISEE_MAX_MISC_IMAGE_NUMBER   5
#define HISEE_MISC_NO_UPGRADE_NUMBER  1
#endif
#define HISEE_SMX_MISC_IMAGE_NUMBER   10

#define HISEE_COS_FLASH_IMG_NAME        "cos_flash.img"
#define HISEE_COS_FLASH_IMG_FULLNAME    "/mnt/hisee_fs/cos_flash.img"
#define HISEE_CASD_IMG_FULLNAME         "/mnt/hisee_fs/casd.img"

#define HISEE_COS_STORAGE_ID_OFF     16
#define HISEE_COS_STORAGE_ID_MASK    0xFFFF

/* Hisee module specific error code */
#define HISEE_COS_VERIFICATITON_ERROR     (-1002)
#define HISEE_IMG_PARTITION_MAGIC_ERROR   (-1003)
#define HISEE_IMG_PARTITION_FILES_ERROR   (-1004)
#define HISEE_IMG_SUB_FILE_NAME_ERROR     (-1005)
#define HISEE_SUB_FILE_SIZE_CHECK_ERROR   (-1006)
#define HISEE_SUB_FILE_OFFSET_CHECK_ERROR (-1007)
#define HISEE_IMG_SUB_FILE_ABSENT_ERROR   (-1008)
#define HISEE_FS_SUB_FILE_ABSENT_ERROR    (-1009)

#define HISEE_OPEN_FILE_ERROR     (-2000)
#define HISEE_READ_FILE_ERROR     (-2001)
#define HISEE_WRITE_FILE_ERROR    (-2002)
#define HISEE_CLOSE_FILE_ERROR    (-2003)
#define HISEE_LSEEK_FILE_ERROR    (-2004)
#define HISEE_OUTOF_RANGE_FILE_ERROR   (-2005)
#define HISEE_ACCESS_FILE_ERROR        (-2006)
#define HISEE_READ_COSID_ERROR         (-2007)

#define HISEE_FS_MALLOC_ERROR          (-3000)
#define HISEE_FS_PATH_ABSENT_ERROR     (-3001)
#define HISEE_FS_OPEN_PATH_ERROR       (-3002)
#define HISEE_FS_COUNT_FILES_ERROR     (-3003)
#define HISEE_FS_PATH_LONG_ERROR       (-3004)
#define HISEE_FS_READ_FILE_ERROR       (-3005)

#define HISEE_ENCOS_HEAD_INIT_ERROR    (-9000)
#define HISEE_COS_IMG_ID_ERROR         (-9001)
#define HISEE_ENCOS_PARTITION_MAGIC_ERROR       (-9002)
#define HISEE_ENCOS_PARTITION_FILES_ERROR       (-9003)
#define HISEE_ENCOS_PARTITION_SIZE_ERROR        (-9004)
#define HISEE_ENCOS_SUBFILE_SIZE_CHECK_ERROR    (-9005)
#define HISEE_ENCOS_OPEN_FILE_ERROR      (-9006)
#define HISEE_ENCOS_FIND_PTN_ERROR       (-9007)
#define HISEE_ENCOS_LSEEK_FILE_ERROR     (-9008)
#define HISEE_ENCOS_WRITE_FILE_ERROR     (-9009)
#define HISEE_ENCOS_READ_FILE_ERROR      (-9010)
#define HISEE_ENCOS_ACCESS_FILE_ERROR    (-9011)
#define HISEE_ENCOS_CHECK_HEAD_ERROR     (-9012)
#define HISEE_ENCOS_NAME_NULL_ERROR      (-9013)
#define HISEE_ENCOS_SYNC_FILE_ERROR      (-9014)

#define HISEE_MULTICOS_PARAM_ERROR            (-18000)
#define HISEE_MULTICOS_READ_UPGRADE_ERROR     (-18001)
#define HISEE_MULTICOS_WRITE_UPGRADE_ERROR    (-18002)
#define HISEE_MULTICOS_COSID_INVALID_ERROR    (-18003)
#define HISEE_MULTICOS_COS_FLASH_FILE_ERROR   (-18004)
#define HISEE_MULTICOS_POWERON_UPGRADE_ERROR  (-18005)
#define HISEE_MULTICOS_POWEROFF_ERROR         (-18006)
#define HISEE_MULTICOS_IMG_UPGRADE_ERROR      (-18007)
#define HISEE_MULTICOS_COS_FLASH_SIZE_ERROR   (-18008)
#define HISEE_MULTICOS_COS_ID_IS_NOT_EXIST_ERROR   (-18009)

/* hisee image info */
#define HISEE_IMG_MAGIC_LEN          4
#define HISEE_IMG_MAGIC_VALUE        "inse"
#define HISEE_IMG_TIME_STAMP_LEN     20
#define HISEE_IMG_DATA_LEN           4
#define HISEE_IMG_SUB_FILE_LEN       4
#define HISEE_IMG_HEADER_LEN        (HISEE_IMG_MAGIC_LEN + \
	HISEE_IMG_TIME_STAMP_LEN + \
	HISEE_IMG_DATA_LEN + \
	HISEE_IMG_SUB_FILE_LEN)
#define COS_IMAGE_RAWDATA_LEN       32UL

#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
#define HISEE_IMG_SUB_FILE_MAX         50
#else
#define HISEE_IMG_SUB_FILE_MAX         12
#endif

#define HISEE_IMG_SUB_FILE_NAME_LEN    8
#define HISEE_IMG_SUB_FILE_OFFSET_LEN  4
#define HISEE_IMG_SUB_FILE_DATA_LEN    4
/*
 * HISEE_IMG_SUB_FILE_NAME_LEN + HISEE_IMG_SUB_FILE_OFFSET_LEN +
 * HISEE_IMG_SUB_FILE_DATA_LEN
 */
#define HISEE_IMG_SUB_FILES_LEN        16
#define HISEE_IMG_SLOADER_NAME   "SLOADER"
#define HISEE_IMG_ULOADER_NAME   "ULOADER"
#define HISEE_IMG_COS_NAME       "COS"
#define HISEE_IMG_COS1_NAME      "COS1"
#define HISEE_IMG_COS2_NAME      "COS2"
#define HISEE_IMG_COS3_NAME      "COS3"
#define HISEE_IMG_OTP_NAME       "OTP"
#define HISEE_IMG_OTP0_NAME      "OTP0"
#define HISEE_IMG_OTP1_NAME      "OTP1"
#define HISEE_IMG_MISC_NAME      "MISC"

#ifdef CONFIG_HISEE_NEW_COS_VERSION_HEADER
#define HISEE_COS_VERSION_OFFSET  16
#else
#define HISEE_COS_VERSION_OFFSET  12
#endif

#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
#ifdef CONFIG_HISEE_NEW_COSID_HEADER
#define HISEE_COS_ID_OFFSET       12
#else
#define HISEE_COS_ID_OFFSET       20
#endif
#endif

#define HISEE_MAX_RPMB_COS_NUMBER  3
#define HISEE_MIN_RPMB_COS_NUMBER  1

#define HISEE_MAX_EMMC_COS_NUMBER  5
#define HISEE_MIN_EMMC_COS_NUMBER  0

#define HISEE_MAX_COS_IMAGE_NUMBER_RESERVED  8
#define HISEE_MAX_COS_IMAGE_NUMBER  (HISEE_MAX_RPMB_COS_NUMBER + HISEE_MAX_EMMC_COS_NUMBER)
#define HISEE_MIN_COS_IMAGE_NUMBER  1

#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
#define HISEE_SUPPORT_COS_FILE_NUMBER (HISEE_MAX_COS_IMAGE_NUMBER)
#define HISEE_MAX_SW_VERSION_NUMBER  (HISEE_SUPPORT_COS_FILE_NUMBER)
#define HISEE_HALF_SW_VERSION_NUMBER (HISEE_SUPPORT_COS_FILE_NUMBER >> 1)
#define HISEE_COS_EXIST              0xB43C5A5A
#else
#define HISEE_SUPPORT_COS_FILE_NUMBER (HISEE_MIN_COS_IMAGE_NUMBER)
#define HISEE_MAX_SW_VERSION_NUMBER   4
#endif

#define HISEE_FACTORY_TEST_DEFAULT_COS_FILE_NUMBER  (HISEE_MIN_COS_IMAGE_NUMBER)

#define HISEE_MAX_MISC_ID_NUMBER     (HISEE_MAX_MISC_IMAGE_NUMBER * HISEE_SUPPORT_COS_FILE_NUMBER)

#define HISEE_COS_VERSION_STORE_SIZE   32
#define HISEE_UPGRADE_STORE_SIZE       32
#define HISEE_MISC_VERSION_STORE_SIZE  32

#define MAX_PATH_NAME_LEN               128
#define HISEE_FILESYS_DIR_ENTRY_SIZE    1024
/* default mode when creating a file or dir if user doesn't set mode */
#define HISEE_FILESYS_DEFAULT_MODE      0660
#define FILESYS_RD_WR_MODE              0600
#define HISEE_U32_MAX_VALUE             0xffffffffu

#define UNITS_DIGIT_WEIGHT     0
#define TENS_DIGIT_WEIGHT      10
#define HUNDREDS_DIGIT_WEIGHT  100
#define THOUSANDS_DIGIT_WEIGHT 1000

/* for kernel 4.19 upgrade */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
static inline off_t hisee_sys_lseek(unsigned int fd, off_t offset,
				    unsigned int whence)
{
	return ksys_lseek(fd, offset, whence);
}

static inline long hisee_sys_open(const char __user *filename, int flags,
				  umode_t mode)
{
	return ksys_open(filename, flags, mode);
}
static inline int hisee_sys_close(unsigned int fd)
{
	return ksys_close(fd);
}
static inline ssize_t hisee_sys_write(unsigned int fd, const char __user *buf,
				      size_t count)
{
	return ksys_write(fd, buf, count);
}
static inline ssize_t hisee_sys_read(unsigned int fd, char __user *buf,
				     size_t count)
{
	return ksys_read(fd, buf, count);
}

static inline long hisee_sys_access(const char __user *filename, int mode)
{
	return ksys_access(filename, mode);
}

static inline long hisee_sys_fsync(unsigned int fd)
{
	return ksys_fsync(fd);
}
static inline long hisee_sys_unlink(const char __user *pathname)
{
	return ksys_unlink(pathname);
}
static inline long hisee_sys_mkdir(const char __user *pathname, umode_t mode)
{
	return ksys_mkdir(pathname, mode);
}
#else
static inline off_t hisee_sys_lseek(unsigned int fd, off_t offset,
				    unsigned int whence)
{
	return sys_lseek(fd, offset, whence);
}

static inline long hisee_sys_open(const char __user *filename, int flags,
				  umode_t mode)
{
	return sys_open(filename, flags, mode);
}
static inline int hisee_sys_close(unsigned int fd)
{
	return sys_close(fd);
}
static inline ssize_t hisee_sys_write(unsigned int fd, const char __user *buf,
				      size_t count)
{
	return sys_write(fd, buf, count);
}
static inline ssize_t hisee_sys_read(unsigned int fd, char __user *buf,
				     size_t count)
{
	return sys_read(fd, buf, count);
}

static inline long hisee_sys_access(const char __user *filename, int mode)
{
	return sys_access(filename, mode);
}

static inline long hisee_sys_fsync(unsigned int fd)
{
	return sys_fsync(fd);
}
static inline long hisee_sys_unlink(const char __user *pathname)
{
	return sys_unlink(pathname);
}
static inline long hisee_sys_mkdir(const char __user *pathname, umode_t mode)
{
	return sys_mkdir(pathname, mode);
}
#endif

union timestamp_info {
	struct timestamp {
		unsigned char second;
		unsigned char minute;
		unsigned char hour;
		unsigned char day;
		unsigned char month;
		unsigned char padding;
		unsigned short year;
	} timestamp;
	unsigned long value;
};

/* hisee_img partition struct */
struct img_file_info {
	char name[HISEE_IMG_SUB_FILE_NAME_LEN];
	unsigned int offset;
	unsigned int size;
};

/* multi cos image upgrade information struct */
struct upgrade_timestamp_info {
	union timestamp_info img_timestamp;
	unsigned int reserved[2]; /* 2 for reserved */
};

/* multi cos image upgrade information struct,72 bytes */
struct multicos_upgrade_info {
	/* store image upgrade sw version */
	unsigned char sw_upgrade_version[HISEE_MAX_COS_IMAGE_NUMBER_RESERVED];
	/* store image upgrade timestamp version */
	struct upgrade_timestamp_info sw_upgrade_timestamp[HISEE_MAX_COS_IMAGE_NUMBER_RESERVED];
};

struct hisee_img_header {
	char magic[HISEE_IMG_MAGIC_LEN];
	char time_stamp[HISEE_IMG_TIME_STAMP_LEN];
	unsigned int total_size;
	unsigned int file_cnt;
	struct img_file_info file[HISEE_IMG_SUB_FILE_MAX];

	unsigned int sw_version_cnt[HISEE_MAX_SW_VERSION_NUMBER];
	struct multicos_upgrade_info cos_upgrade_info;
	unsigned int misc_image_cnt[HISEE_SUPPORT_COS_FILE_NUMBER];
	unsigned int emmc_cos_cnt;
	unsigned int rpmb_cos_cnt;
#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
	unsigned int is_cos_exist[HISEE_SUPPORT_COS_FILE_NUMBER];
#endif
#ifdef CONFIG_HISEE_SUPPORT_DCS
	unsigned int dcs_image_cnt[HISEE_SUPPORT_COS_FILE_NUMBER];
#endif
};

#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
#define HISEE_ENCOS_MAGIC_LEN       8
#define HISEE_ENCOS_SUB_FILE_MAX    HISEE_MAX_EMMC_COS_NUMBER
#define HISEE_ENCOS_SUB_FILES_LEN   16
#define HISEE_ENCOS_SUB_FILE_NAME_LEN 8
#define HISEE_ENCOS_SUB_FILE_LEN    (HISEE_MAX_IMG_SIZE)
#define HISEE_ENCOS_MAGIC_VALUE     "encd_cos"
#define HISEE_ENCOS_TOTAL_FILE_SIZE (HISEE_ENCOS_SUB_FILE_MAX * HISEE_ENCOS_SUB_FILE_LEN)

struct encos_file_info {
	char name[HISEE_ENCOS_SUB_FILE_NAME_LEN];
	unsigned int offset;
	unsigned int size;
};

struct hisee_encos_header {
	char magic[HISEE_ENCOS_MAGIC_LEN];
	unsigned int total_size;
	unsigned int file_cnt;
	struct encos_file_info file[HISEE_ENCOS_SUB_FILE_MAX];
};
#endif

enum hisee_image_a_access_type {
	SW_VERSION_READ_TYPE = 0,
	SW_VERSION_WRITE_TYPE,
	COS_UPGRADE_RUN_READ_TYPE = 2,
	COS_UPGRADE_RUN_WRITE_TYPE,
	MISC_VERSION_READ_TYPE = 4,
	MISC_VERSION_WRITE_TYPE,
	COS_UPGRADE_INFO_READ_TYPE = 6,
	COS_UPGRADE_INFO_WRITE_TYPE,
};

#define hisee_is_write_access(access_type)    \
		(((access_type) == SW_VERSION_WRITE_TYPE) ||\
		 ((access_type) == COS_UPGRADE_RUN_WRITE_TYPE) ||\
		 ((access_type) == MISC_VERSION_WRITE_TYPE) ||\
		 ((access_type) == COS_UPGRADE_INFO_WRITE_TYPE))

enum hisee_img_file_type {
	SLOADER_IMG_TYPE = 0,
#ifdef CONFIG_HISEE_SUPPORT_ULOADER
	ULOADER_IMG_TYPE,
#endif
	COS_IMG_TYPE,
#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
	COS1_IMG_TYPE,
	COS2_IMG_TYPE,
	COS3_IMG_TYPE,
	COS4_IMG_TYPE,
	COS5_IMG_TYPE,
	COS6_IMG_TYPE,
	COS7_IMG_TYPE,
#endif
#ifdef CONFIG_HISEE_SUPPORT_DCS
	DCS0_IMG_TYPE,
	DCS1_IMG_TYPE,
	DCS2_IMG_TYPE,
	DCS3_IMG_TYPE,
	DCS4_IMG_TYPE,
	DCS5_IMG_TYPE,
	DCS6_IMG_TYPE,
	DCS7_IMG_TYPE,
	DCS8_IMG_TYPE,
	DCS9_IMG_TYPE,
	DCS10_IMG_TYPE,
	DCS11_IMG_TYPE,
	DCS12_IMG_TYPE,
	DCS13_IMG_TYPE,
	DCS14_IMG_TYPE,
	DCS15_IMG_TYPE,
	DCS16_IMG_TYPE,
	DCS17_IMG_TYPE,
	DCS18_IMG_TYPE,
	DCS19_IMG_TYPE,
#endif
	OTP_IMG_TYPE,
	OTP1_IMG_TYPE,
	MISC0_IMG_TYPE,
	MISC1_IMG_TYPE,
	MISC2_IMG_TYPE,
	MISC3_IMG_TYPE,
	MISC4_IMG_TYPE,
#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
	MISC5_IMG_TYPE,
	MISC6_IMG_TYPE,
	MISC7_IMG_TYPE,
	MISC8_IMG_TYPE,
	MISC9_IMG_TYPE,
	MISC10_IMG_TYPE,
	MISC11_IMG_TYPE,
	MISC12_IMG_TYPE,
	MISC13_IMG_TYPE,
	MISC14_IMG_TYPE,
	MISC15_IMG_TYPE,
	MISC16_IMG_TYPE,
	MISC17_IMG_TYPE,
	MISC18_IMG_TYPE,
	MISC19_IMG_TYPE,
	COS_FLASH_IMG_TYPE, /* make sure COS_FLASH_IMG_TYPE as the last valid IMG_TYPE */
#endif
	MAX_IMG_TYPE,
};

/* hisee_img partition struct */
struct img_name_info_t {
	enum hisee_img_file_type img_type;
	char img_file_name[HISEE_IMG_SUB_FILE_NAME_LEN];
};

struct cosimage_version_info {
	unsigned int magic;
	unsigned char img_version_num[HISEE_MAX_SW_VERSION_NUMBER];
	union timestamp_info img_timestamp;
};

#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
/*
 * To adapt to previous version, the img_version which is stored
 * in hisee_img partition is divided into two part.
 */
struct hisee_partition_version_info {
	unsigned int magic;
	unsigned char img_version_num[HISEE_HALF_SW_VERSION_NUMBER];
	union timestamp_info img_timestamp;
	unsigned char img_version_num1[HISEE_HALF_SW_VERSION_NUMBER];
};
#endif

int write_hisee_otp_value(enum hisee_img_file_type otp_img_index);
int hisee_parse_img_header(char *buffer);
int filesys_hisee_read_image(enum hisee_img_file_type type,
			     char *buffer, size_t bsize);
int hisee_read_file(const char *fullname, char *buffer,
		    size_t offset, size_t size);
int filesys_read_img_from_file(const char *filename,
			       char *buffer,
			       size_t *file_size,
			       size_t max_read_size);
int hisee_write_file(const char *fullname, const char *buffer, size_t size);
int hisee_get_partition_path(char full_path[MAX_PATH_NAME_LEN],
			     size_t buff_len);

#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
int hisee_encos_header_init(void);
int hisee_preload_encos_img(void);
int filesys_rm_cos_flash_file(void);
int check_cos_flash_file_exist(unsigned int *exist_flg);
#endif

int access_hisee_image_partition(char *data_buf,
				 enum hisee_image_a_access_type access_type);
void parse_timestamp(const char timestamp_str[HISEE_IMG_TIME_STAMP_LEN],
		     union timestamp_info *timestamp_value);

/*
 * @brief      : implement the same atoi function as in libc, but only support
 *               postive integer
 * @param[in]  : str , the integer string
 * @return     : ::unsigned int, normal value is success; HISEE_U32_MAX_VALUE is
 *               failure
 */
unsigned int hisee_atoi(const char *str);
int hisee_update_misc_version(unsigned int cos_id);

/*
 * @brief     : used to erase hisee_img when hisec is bypassed
 */
int start_erase_hisee_image(void);
#endif
