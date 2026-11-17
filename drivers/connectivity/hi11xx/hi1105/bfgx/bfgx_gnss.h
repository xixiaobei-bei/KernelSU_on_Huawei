

#ifndef __BFGX_GNSS_H__
#define __BFGX_GNSS_H__

/* 其他头文件包含 */
#include <linux/miscdevice.h>
#include "hw_bfg_ps.h"

/* 宏定义 */
#define DTS_COMP_HI110X_PS_ME_NAME "hisilicon,hisi_me"

/* 函数声明 */
struct platform_device *get_me_platform_device(void);
int32_t hw_ps_me_init(void);
void hw_ps_me_exit(void);

#endif /* __BFGX_GNSS_H__ */
