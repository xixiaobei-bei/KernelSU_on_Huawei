/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: Hisilicon image load driver
 * Author: security-ap
 * Create: 2016/03/03
 */
#ifndef __LOAD_IMAGE_H__
#define __LOAD_IMAGE_H__
#include <linux/hisi/hisi_load_image.h>
#ifdef __cplusplus
extern "C"
{
#endif

#define sec_print_err(fmt, ...)        \
	pr_err("[sec]: <%s> "fmt, __func__, ##__VA_ARGS__)
#define sec_print_info(fmt, ...)       \
	pr_info("[sec]: <%s> "fmt, __func__, ##__VA_ARGS__)

#define SEC_OK                       0
#define SEC_ERROR                    (-1)
#define SEC_FILE_NO_EXSIT            (-2)
#define VRL_SIZE                     0x1000

#define BSP_RESET_NOTIFY_REPLY_OK    0
#define BSP_RESET_NOTIFY_SEND_FAILED (-1)
#define BSP_RESET_NOTIFY_TIMEOUT     (-2)
enum {
	PRIMVRL = 0x0,
	BACKVRL = 0x1,
};

enum bsp_core_type_e {
	BSP_CCORE = 0,
	BSP_HIFI,
	BSP_LPM3,
	BSP_BBE,
	BSP_CDSP,
	BSP_CCORE_TAS,
	BSP_CCORE_WAS,
	BSP_CCORE_CAS,
	BSP_BUTT
};

int bsp_reset_core_notify(enum bsp_core_type_e ecoretype, unsigned int cmdtype,
			  unsigned int timeout_ms, unsigned int *retval);

#ifdef __cplusplus
}
#endif
#endif

