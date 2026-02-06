/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: implement hisee common function
 * Create: 2020-02-17
 */
#ifndef HISEE_H
#define HISEE_H
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/pm_wakeup.h>
#ifdef CONFIG_HISEE_SUPPORT_CASDKEY
#include "hisee_casdcert.h"
#endif
#include "hisee_fs.h"

/* Hisee module general error code */
#define HISEE_SECLIB_ERROR   (-1)
#define HISEE_TRUE           1
#define HISEE_FALSE          0
#define HISEE_OK             0
#define HISEE_ERROR          (-10002)
#define HISEE_NO_RESOURCES   (-10003)
#define HISEE_SECUREC_ERR    (-10030)
#define HISEE_INVALID_PARAMS (-6)
#define HISEE_IS_UPGRADING   (-7)
#define HISEE_CMA_DEVICE_INIT_ERROR    (-10005)
#define HISEE_IOCTL_NODE_CREATE_ERROR  (-10006)
#define HISEE_POWER_NODE_CREATE_ERROR  (-10007)
#define HISEE_THREAD_CREATE_ERROR      (-10008)
#define HISEE_RPMB_MODULE_INIT_ERROR   (-11)
#define HISEE_BULK_CLK_INIT_ERROR      (-10010)
#define HISEE_BULK_CLK_ENABLE_ERROR    (-10011)
#define HISEE_POWERDEBUG_NODE_CREATE_ERROR     (-10012)
#define HISEE_AT_RESULT_NODE_CREATE_ERROR      (-10015)
#define HISEE_FACTORY_STATE_ERROR              (-10021)

#define HISEE_OTP1_WRITE_FAIL   (-10023)
#define HISEE_POWER_CTRL_FAIL   (-10024)

/* Hisee module specific error code */
#define HISEE_RPMB_KEY_WRITE_ERROR     (-1000)
#define HISEE_RPMB_KEY_READ_ERROR      (-1001)
#define HISEE_RPMB_KEY_UNREADY_ERROR   (-1010)

#define HISEE_FIRST_SMC_CMD_ERROR     (-5000)
#define HISEE_SMC_CMD_TIMEOUT_ERROR   (-5001)
#define HISEE_SMC_CMD_PROCESS_ERROR   (-5002)

#define HISEE_GET_HISEE_VALUE_ERROR   (-7000)
#define HISEE_SET_HISEE_VALUE_ERROR   (-7001)
#define HISEE_SET_HISEE_STATE_ERROR   (-7002)

#ifdef CONFIG_SMX_PROCESS
#define SMX_PROCESS_STEP1_ACTION       0x01000101
#define SMX_PROCESS_STEP1_ID           0
#define SMX_PROCESS_STEP1_CMD          0x01000200

#define SMX_PROCESS_STEP2_ACTION       0x01000100
#define SMX_PROCESS_STEP2_ID           0
#define SMX_PROCESS_STEP2_CMD          0x01000201

#define SMX_PROCESS_UNSUPPORT           0x5A5A0001
#define SMX_PROCESS_SUPPORT_AND_OK      0x5A5A0002
#define SMX_PROCESS_SUPPORT_BUT_ERROR   0x5A5A0003
#define SMX_PROCESS_INVALID_PARAMS      0x5A5A0004
#define SMX_PROCESS_CLK_ENABLE_ERROR    0x5A5A0005

#define SMX_PROCESS_FN_MAIN_SERVICE_CMD 0xc5000020
#endif

#define HISEE_WAIT_READY_TIMEOUT        (-9001)

#ifdef CONFIG_HISEE_SUPPORT_CASDKEY
#define HISI_CASD_ERROR                 (-19000)
#define HISI_CASD_AT_MISMATCH_ERROR     (-19001)
#define HISI_CASD_AT_HEADER_ERROR       (-19002)
#define HISI_CASD_LENGTH_ERROR          (-19003)
#define HISI_CASD_DATA_PARAM_ERROR      (-19004)
#define HISI_CASD_HASH_ERROR            (-19005)
#endif

#define HISEE_DEVICE_NAME     "hisee"
/* when hisee is bypassed all file api show this msg */
#define HISEE_API_BYPASS_MSG  "hisee_is_bypassed"

#define HISEE_DEFAULT_COSID   0
#define HISEE_NEW_COS_EXIST   0
#define HISEE_NEW_COS_NON_EXIST  1
#define HISEE_NEW_COS_FAILURE    (-1)
#define HISEE_NEW_COS_MAX_STRING 10UL

/* ATF service id */
#define HISEE_FN_MAIN_SERVICE_CMD    0xc5000020
#define HISEE_FN_CHANNEL_TEST_CMD    0xc5000040

#define HISEE_ATF_ACK_SUCCESS 0xaabbccaa
#define HISEE_ATF_ACK_FAILURE 0xaabbcc55

#define HISEE_SM_MODE_MAGIC              0xffeebbaa
#define HISEE_DM_MODE_MAGIC              0xffeebb55
#define DELAY_BETWEEN_STEPS              50
#define DELAY_FOR_HISEE_POWERON_UPGRADE  200
#define DELAY_FOR_HISEE_POWEROFF         50
#define DELAY_FOR_HISEE_POWERON_BOOTING  200
#define DELAY_100_MS                     100
#define DELAY_1000_MS                    1000
#define DELAY_10_SECOND                  10000
#define HISEE_PARTITION_READY_TIMEOUT_MS 500
#define HISEE_PARTITION_READY_TIMEOUT_CNT 50

#define HISEE_ATF_MESSAGE_HEADER_LEN    16
#define HISEE_ATF_COS_APPDATA_TIMEOUT   15000
#define HISEE_ATF_WRITE_RPMBKEY_TIMEOUT 10000
#define HISEE_ATF_OTP_TIMEOUT           10000
#define HISEE_ATF_COS_TIMEOUT           30000
#define HISEE_ATF_SLOADER_TIMEOUT       30000
#ifndef CONFIG_HISEE_SUPPORT_MULTI_COS
#define HISEE_ATF_MISC_TIMEOUT          30000
#else
#define HISEE_ATF_MISC_TIMEOUT          100000
#endif
#define HISEE_ATF_APPLICATION_TIMEOUT   60000
#define HISEE_ATF_GENERAL_TIMEOUT       30000
#define HISEE_ATF_NVM_FORMAT_TIMEOUT    60000
#define HISEE_FPGA_ATF_COS_TIMEOUT      3000000

#define SIZE_1K              1024UL
#define SIZE_4K              (4 * SIZE_1K)
#define SIZE_2K              (2 * SIZE_1K)
#define SIZE_1M              (1024 * SIZE_1K)

#define SMC_TEST_RESULT_SIZE       256u

#define HISEE_COS_PATCH_FREE_CNT   1

#define HISEE_MAX_IMG_SIZE    (640 * SIZE_1K)
#define HISEE_SHARE_BUFF_SIZE round_up((HISEE_MAX_IMG_SIZE + HISEE_ATF_MESSAGE_HEADER_LEN), PAGE_SIZE)
#define HISEE_CMD_NAME_LEN    128
#define HISEE_BUF_SHOW_LEN    128
#define HISEE_SHOW_VALUE_LEN  8
#define HISEE_ERROR_DESCRIPTION_MAX  64
#define HISEE_APDU_DATA_LEN_MAX      261
#define HISEE_COS_READY_SHOW_VALUE   0

#define HISEE_IOCTL_CMD_MAX_LEN     2048 /* max str len for AT^HISEE=CASD cmd */

#define HISEE_FACTORY_TEST_VERSION  0x12345678
#define HISEE_SERVICE_WORK_VERSION  0

#define MAX_CMD_BUFF_PARAM_LEN      4
#define HISEE_COS_ID_POS            1
#define HISEE_PROCESS_TYPE_POS      2

/* hisee apdu cmd type */
#define HISEE_SET_KEY           0
#define HISEE_DEL_TEST_APPLET   1

#define HISEE_CHAR_NEWLINE      10
#define HISEE_CHAR_SPACE        32

/* hisee NFC_IRQ mode */
#ifdef CONFIG_HISEE_NFC_IRQ_SWITCH
#define HISEE_NFC_IRQ_SWITCH_REG  SOC_SCTRL_SCBAKDATA22_ADDR(SOC_ACPU_SCTRL_BASE_ADDR)
#define HISEE_NFC_IRQ_DISABLE_BIT 31
#endif
/* hisee lcs mode */
#define MSPC_LCS_BANK_REG   SOC_SCTRL_SCBAKDATA10_ADDR(SOC_ACPU_SCTRL_BASE_ADDR)
#define HISEE_LCS_DM_BIT    13

#define SMX_PROCESS_0       0x5A5AA5A5
#define SMX_PROCESS_1       0xA5A55A5A
#define SMX_ENABLE          HISEE_OK
#define SMX_DISABLE         HISEE_ERROR

#define RPMB_READY_TIMEOUT_MS  500
#define RPMB_READY_TIMEOUT_CNT 50

#define HISEE_SM_EFUSE_POS_INVALID  0xFFFF

#define SET_KEY_CMD_0_LEN     21
#define SET_KEY_CMD_1_LEN     5
#define DELETE_APPLET_CMD_0_LEN   5
#ifdef CONFIG_HISEE_APPLET_APDU_TEST_OPTIMIZATION
#define DELETE_APPLET_CMD_1_LEN   13
#else
#define DELETE_APPLET_CMD_1_LEN   12
#endif

#define SE_HISEE_MISC_NO_UPRGADE  0xCCAAAACC

/* check ret is ok or otherwise goto err_process */
#define check_result_and_goto(ret, lable) \
do { \
	if ((ret) != HISEE_OK) { \
		pr_err("hisee:%s() run failed,line=%d.\n", __func__, __LINE__);\
		goto lable; \
	} \
} while (0)

#define check_and_print_result()  \
do {\
	if (ret != HISEE_OK)\
		pr_err("hisee:%s() run failed\n", __func__);\
	else\
		pr_err("hisee:%s() run success\n", __func__);\
} while (0)

#define check_and_print_result_with_cosid()  \
do {\
	if (ret != HISEE_OK)\
		pr_err("hisee:%s() run failed,cos_id=%u.\n", __func__, cos_id);\
	else\
		pr_err("hisee:%s() run success,cos_id=%u.\n", __func__, cos_id);\
} while (0)

static inline void hisee_mdelay(unsigned int n)
{
	msleep(n);
}

static inline void hisee_delay(unsigned int n)
{
	msleep((n) * DELAY_1000_MS);
}

#ifdef CONFIG_HISEE_NFC_IRQ_SWITCH
#define HISEE_NFC_IRQ_SWITCH_CMD_MAX_LEN   4
enum hisee_nfc_irq_cfg_state {
	NFC_IRQ_CFG_ON = 0x5A5AA5A5,
	NFC_IRQ_CFG_OFF = 0xA5A55A5A
};
#endif

enum hisee_nfc_cfg_message {
	DISABLE_NFC_IRQ = 0x02000100,
	SET_NFC_IRQ = 0x02000101,
};

struct hisee_errcode_item_des {
	int err_code; /* see error code definition */
	char err_description[HISEE_ERROR_DESCRIPTION_MAX]; /* error code description */
};

struct hisee_driver_function {
	char *function_name; /* function cmd string */
	int (*function_ptr)(const void *buf, int para); /* function cmd process */
};

enum hisee_cos_imgid_type {
	COS_IMG_ID_0 = 0,
	COS_IMG_ID_1 = 1,
	COS_IMG_ID_2 = 2,
	COS_IMG_ID_3 = 3,
	COS_IMG_ID_4 = 4,
	COS_IMG_ID_5 = 5,
	COS_IMG_ID_6 = 6,
	COS_IMG_ID_7 = 7,
	MAX_COS_IMG_ID,
};

enum hisee_cos_process_type {
	COS_PROCESS_WALLET = 0, /* huawei wallet */
	COS_PROCESS_U_SHIELD,   /* u shiled */
	COS_PROCESS_CHIP_TEST,  /* chip test */
	COS_PROCESS_UNKNOWN,    /* default id */
	COS_PROCESS_TIMEOUT,    /* timeout :must be the last valid id */
#ifdef CONFIG_HISEE_SUPPORT_INSE_ENCRYPT
	COS_PROCESS_INSE_ENCRYPT = 5, /* inse encypt: pin, bio, file etc. */
#endif
	MAX_POWER_PROCESS_ID,
};

/* modify the factory flow */
#define COS_PROCESS_UPGRADE COS_PROCESS_CHIP_TEST

enum se_smc_cmd {
	CMD_UPGRADE_SLOADER = 0,
	CMD_UPGRADE_OTP,
	CMD_UPGRADE_COS,
#ifdef CONFIG_HISEE_SUPPORT_ULOADER
	CMD_UPGRADE_ULOADER,
#endif
	CMD_UPGRADE_MISC,
#ifdef CONFIG_HICOS_MISCIMG_PATCH
	CMD_UPGRADE_COS_PATCH,
#endif
	CMD_UPGRADE_APPLET,
	CMD_PRESAVE_COS_APPDATA,
#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
	CMD_LOAD_ENCOS_DATA,
#endif
	CMD_WRITE_RPMB_KEY,
	CMD_SET_LCS_SM,
	CMD_SET_STATE,
	CMD_GET_STATE,
	CMD_APDU_RAWDATA,
	CMD_FACTORY_APDU_TEST,
	CMD_HISEE_CHANNEL_TEST,
	CMD_HISEE_VERIFY_KEY,
	CMD_HISEE_WRITE_CASD_KEY,
#ifdef CONFIG_HISEE_FACTORY_SECURITY_CHECK
	CMD_HISEE_FACTORY_CHECK,
#endif

	CMD_HISEE_POWER_ON = 0x30,
	CMD_HISEE_POWER_OFF,
#ifdef CONFIG_SMX_PROCESS
	CMD_SMX_PROCESS_STEP1,
	CMD_SMX_PROCESS_STEP2,
#endif
#if defined(CONFIG_SMX_PROCESS) || defined(CONFIG_HISEE_AT_SMX)
	CMD_SMX_GET_EFUSE,
#endif
	CMD_HISEE_GET_EFUSE_VALUE = 0x40,
	CMD_FORMAT_RPMB = 0x51,
#ifdef CONFIG_HISEE_SUPPORT_DCS
	CMD_UPGRADE_DCS = 0x52,
#endif
#ifdef CONFIG_HISEE_DISABLE_KEY
	CMD_DISABLE_KEY = 0x53,
#endif
#ifdef CONFIG_HISEE_AT_SMX
	CMD_SET_SMX = 0x70,
#endif
	CMD_END,
};

struct hisee_work_struct {
	char *buffer;
	phys_addr_t phy;
	unsigned int size;
};

enum hisee_factory_test_status {
	HISEE_FACTORY_TEST_FAIL = -1,
	HISEE_FACTORY_TEST_SUCCESS = 0,
	HISEE_FACTORY_TEST_RUNNING = 1,
	HISEE_FACTORY_TEST_NORUNNING = 2,
};

/* message header between kernel and atf */
struct atf_message_header {
	/*
	 * atf cmd execute type, such as otp, cos, sloader at all,
	 * kernel set and atf read it
	 */
	unsigned int cmd;
	/*
	 * atf cmd execute result indication, use a magic value to
	 * indicate success, atf set it and check in kernel
	 */
	unsigned int ack;
	/* tell atf store the result to this buffer when doing channel test */
	unsigned int test_result_phy;
	/* tell atf the size of buffer when doing channel test */
	unsigned int test_result_size;
};

struct apdu_ack_header {
	unsigned int ack_len;
	unsigned char ack_buf[HISEE_APDU_DATA_LEN_MAX + 1];
};

struct hisee_module_data {
	struct device *cma_device; /* cma memory allocator device */
	struct clk *hisee_clk;     /* buck 0 voltage hold at 0.8v */
	void *apdu_command_buff_virt;
	phys_addr_t apdu_command_buff_phy;
	/* do sync for smc message between kernel and atf */
	struct completion smc_completion;
	/* store the parsed result for hisee_img partition header */
	struct hisee_img_header hisee_img_head;
	struct apdu_ack_header apdu_ack; /* store the apdu response */
	struct mutex hisee_mutex; /* mutex for global resources */
	struct hisee_work_struct channel_test_item_result;
	unsigned int rpmb_is_ready; /* indicate the rpmb has been initialiazed */
	unsigned int smc_cmd_running; /* indicate the smc is running */
	int power_on_count; /* indicate the number of hisee poweron */
	/* indicate the factory test status */
	enum hisee_factory_test_status factory_test_state;
	struct wakeup_source wake_lock;
#ifdef CONFIG_HISEE_SUPPORT_CASDKEY
	struct hisee_casd_at_data casd_data;
#endif
};

/*
 * @brief      : set err_code to g_hisee_errno, then return with err_code
 * @return     : ::int, return error code
 */
int set_errno_then_exit(int err_code);

int get_hisee_errno(void);
void set_hisee_errno(int err_code);

/*
 * @brief      :get the pointer of g_hisee_data variable
 * @return     : ::struct hisee_module_data*
 */
struct hisee_module_data *get_hisee_data_ptr(void);
u32 get_cos_upgrade_time(void);
int get_hisee_lcs_mode(unsigned int *mode);
int set_hisee_lcs_sm_efuse(void);
noinline int atfd_hisee_smc(u64 function_id, u64 arg0, u64 arg1, u64 arg2);
int send_smc_process(const struct atf_message_header *p_message_header,
		     phys_addr_t phy_addr, unsigned int size,
		     unsigned int timeout, enum se_smc_cmd smc_cmd);
void set_message_header(struct atf_message_header *header, unsigned int cmd_type);
int send_apdu_cmd(int type);
int hisee_lpmcu_send(rproc_msg_t msg_0, rproc_msg_t msg_1);
int cos_image_upgrade_by_self(void);
#ifdef CONFIG_HISEE_NFC_IRQ_SWITCH
void nfc_irq_cfg(enum hisee_nfc_irq_cfg_state flag);
#endif

void hisee_get_smx_cfg(unsigned int *p_smx_cfg);

/*
 * @brief      : check hisee api is bypassed on specified platform
 * @return     : ::bool, true indicates API be byppased
 */
bool hisee_api_is_bypassed(void);

/*
 * @brief      : check current hisee platform whether is FPGA
 * @return     : ::bool, true indicates FPGA, other indicates non-FPGA
 */
bool check_hisee_is_fpga(void);

int get_efuse_hisee_value(unsigned char *buffer, unsigned int length,
			  unsigned int timeout);
int set_efuse_hisee_value(unsigned char *buffer, unsigned int length,
			  unsigned int timeout);
#endif
