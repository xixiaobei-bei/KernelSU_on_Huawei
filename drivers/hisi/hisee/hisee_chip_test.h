/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: implement hisee chip factory test function
 * Create: 2020-02-17
 */
#ifndef HISEE_CHIP_TEST_H
#define HISEE_CHIP_TEST_H
#include <linux/types.h>
#include <linux/device.h>
#include <linux/kernel.h>

/* get key from HISEE */
#define KEY_NOT_READY      0
#define KEY_READY          1
#define KEY_REQ_FAILED     2
#define SINGLE_RPMBKEY_NUM 1

#ifdef CONFIG_HISEE_SUPPORT_MULTI_COS
#ifdef CONFIG_HISEE_SUPPORT_8_COS
#define COS_FLASH_IMG_ID COS_IMG_ID_5
#else
#define COS_FLASH_IMG_ID COS_IMG_ID_3
#endif
#endif

/*
 * @brief  AT cmd type
 */
enum hisee_at_type {
	HISEE_AT_CASD = 0,
	HISEE_AT_VERIFYCASD,
	HISEE_AT_SETSMX,
	HISEE_AT_MAX,
};

struct hisee_at_response {
	int at_type;
	int (*handler)(char *buf, size_t size, int result);
};

enum hisee_at_type hisee_get_at_type(void);
void hisee_set_at_type(enum hisee_at_type type);

int hisee_parallel_manufacture_func(const void *buf, int para);
#ifdef CONFIG_HISEE_NVMFORMAT_TEST
int hisee_nvmformat_func(const void *buf, int para);
#endif

#ifdef CONFIG_HISEE_CHIPTEST_SLT
int hisee_parallel_total_slt_func(const void *buf, int para);
int hisee_read_slt_func(const void *buf, int para);
int hisee_total_slt_func(const void *buf, int para);
#endif
#ifdef CONFIG_HISEE_CHIPTEST_RT
int hisee_chiptest_rt_run_func(const void *buf, int para);
int hisee_chiptest_rt_stop_func(const void *buf, int para);
#endif

/* hisee high temp switch mode */
#ifdef CONFIG_HISEE_HIGH_TEMP_PROTECT_SWITCH
#define HISEE_HIGH_TEMP_PROTECT_ADDR         (SOC_SCTRL_SCBAKDATA22_ADDR(SOC_ACPU_SCTRL_BASE_ADDR))
#define HISEE_HIGH_TEMP_PROTECT_DISABLE_BIT  30

enum hisee_tmp_cfg_state {
	HISEE_TEMP_CFG_ON = 0x5A5AA5A5,
	HISEE_TEMP_CFG_OFF = 0xA5A55A5A
};
#endif

int cos_flash_image_boot_func(void);

int hisee_factory_check_func(const void *buf, int para);

ssize_t hisee_at_result_show(struct device *dev,
			     struct device_attribute *attr, char *buf);

#if defined(CONFIG_SMX_PROCESS) || defined(CONFIG_HISEE_AT_SMX)
int hisee_get_smx_func(const void *buf, int para);
#endif

#ifdef CONFIG_HISEE_AT_SMX
int mspc_set_smx_func(const void *buf, int para);
#endif

/* flag to indicate running status of flash otp1 */
enum e_run_status {
	NO_NEED = 0,
	PREPARED,
	RUNNING,
	FINISH,
};

/* set the otp1 write work status */
void hisee_chiptest_set_otp1_status(enum e_run_status status);

/* check otp1 write work is running */
bool hisee_chiptest_otp1_is_running(void);

enum e_run_status hisee_chiptest_get_otp1_status(void);

bool is_hisee_chiptest_slt(void);

#endif
