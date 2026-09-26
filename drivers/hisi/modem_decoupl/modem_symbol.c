/*
 * libc_sec_export.c for export symbols of security functions
 * require at least one entry.
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * Author: shiwanglai <shiwanglai@hisilicon.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/kconfig.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/version.h>
#include <linux/memblock.h>


#include <linux/sched.h>
#include <linux/of_fdt.h>
#include <linux/reboot.h>

#ifdef CONFIG_MMC_DW_MUX_SDSIM
#include <linux/mmc/host.h>
#include <linux/mmc/dw_mmc_mux_sdsim.h>
#endif

#include <linux/hisi/efuse_driver.h>
#include <linux/usb/bsp_acm.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/hisi_drmdriver.h>
#include <platform_audio/rdr_audio_soc.h>
#include <load_image.h>
#include <drv_mailbox_cfg.h>
#include <huawei_platform/connectivity/huawei_gps.h>


EXPORT_SYMBOL(find_task_by_vpid);

EXPORT_SYMBOL(show_stack);

EXPORT_SYMBOL(set_efuse_kce_value);

EXPORT_SYMBOL(BSP_USB_RegUdiEnableCB);

EXPORT_SYMBOL(sys_rmdir);

EXPORT_SYMBOL(bsp_acm_read);

EXPORT_SYMBOL(get_efuse_kce_value);

EXPORT_SYMBOL(sys_rename);

EXPORT_SYMBOL(get_efuse_chipid_value);

EXPORT_SYMBOL(bsp_acm_ioctl);

EXPORT_SYMBOL(sched_setaffinity);

EXPORT_SYMBOL(sched_getaffinity);

#ifdef CONFIG_LOAD_IMAGE
EXPORT_SYMBOL(bsp_reset_core_notify);
#endif

EXPORT_SYMBOL(sys_llseek);

EXPORT_SYMBOL(sys_unlink);

EXPORT_SYMBOL(sys_ioctl);

EXPORT_SYMBOL(of_get_flat_dt_prop);

#ifdef CONFIG_HISI_BB
EXPORT_SYMBOL(bbox_check_edition);
#endif

EXPORT_SYMBOL(sys_write);

EXPORT_SYMBOL(sys_getdents64);

EXPORT_SYMBOL(show_mem);

EXPORT_SYMBOL(sys_open);

EXPORT_SYMBOL(sys_fsync);

EXPORT_SYMBOL(bsp_acm_open);

EXPORT_SYMBOL(machine_restart);

EXPORT_SYMBOL(bsp_acm_close);

EXPORT_SYMBOL(get_efuse_dieid_value);

EXPORT_SYMBOL(sys_read);

EXPORT_SYMBOL(sys_access);

EXPORT_SYMBOL(bsp_acm_write);

EXPORT_SYMBOL(BSP_USB_RegUdiDisableCB);

EXPORT_SYMBOL(sys_mkdir);

EXPORT_SYMBOL(sys_lseek);

EXPORT_SYMBOL(memblock);

EXPORT_SYMBOL(num_to_str);

EXPORT_SYMBOL(parameq);

#ifdef CONFIG_MMC_DW_MUX_SDSIM
extern int dw_mci_check_himntn(int feature);
EXPORT_SYMBOL(dw_mci_check_himntn);
EXPORT_SYMBOL(sd_sim_detect_run);
EXPORT_SYMBOL(detect_status_to_string);
EXPORT_SYMBOL(sd_sim_detect_status_current);
#endif

#ifdef CONFIG_HISI_HIFI_BB
EXPORT_SYMBOL(hifireset_regcbfunc);
#endif

EXPORT_SYMBOL(DRV_MAILBOX_READMAILDATA);
EXPORT_SYMBOL(DRV_MAILBOX_SENDMAIL);
EXPORT_SYMBOL(DRV_MAILBOX_REGISTERRECVFUNC);


#ifdef CONFIG_HWGPS
EXPORT_SYMBOL(set_gps_ref_clk_enable);
#endif
