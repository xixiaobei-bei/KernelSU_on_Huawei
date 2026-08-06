/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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

#include <linux/mmc/host.h>
#include <linux/mmc/dw_mmc_mux_sdsim.h>

#include "adrv.h"
#include "mdrv.h"
#include "securec.h"

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

EXPORT_SYMBOL(bsp_reset_core_notify);

EXPORT_SYMBOL(sys_llseek);

EXPORT_SYMBOL(sys_unlink);

EXPORT_SYMBOL(sys_ioctl);

EXPORT_SYMBOL(of_get_flat_dt_prop);

EXPORT_SYMBOL(bbox_check_edition);

EXPORT_SYMBOL(sys_write);

EXPORT_SYMBOL(memset_s);

EXPORT_SYMBOL(sys_getdents64);

EXPORT_SYMBOL(show_mem);

EXPORT_SYMBOL(memcpy_s);

EXPORT_SYMBOL(sys_open);

EXPORT_SYMBOL(sys_fsync);

EXPORT_SYMBOL(strncat_s);

EXPORT_SYMBOL(snprintf_s);

EXPORT_SYMBOL(bsp_acm_open);

EXPORT_SYMBOL(machine_restart);

EXPORT_SYMBOL(bsp_acm_close);

EXPORT_SYMBOL(get_efuse_dieid_value);

EXPORT_SYMBOL(sys_read);

EXPORT_SYMBOL(sys_access);

EXPORT_SYMBOL(bsp_acm_write);

EXPORT_SYMBOL(BSP_USB_RegUdiDisableCB);

EXPORT_SYMBOL(sys_mkdir);

EXPORT_SYMBOL(strncpy_s);

EXPORT_SYMBOL(strcpy_s);

EXPORT_SYMBOL(sys_lseek);

EXPORT_SYMBOL(memblock);

EXPORT_SYMBOL(strcat_s);

EXPORT_SYMBOL(num_to_str);

extern int dw_mci_check_himntn(int feature);
EXPORT_SYMBOL(dw_mci_check_himntn);
EXPORT_SYMBOL(sd_sim_detect_run);
EXPORT_SYMBOL(detect_status_to_string);
EXPORT_SYMBOL(sd_sim_detect_status_current);

#include "mdrv_sysboot.h"
EXPORT_SYMBOL(hifireset_regcbfunc);

EXPORT_SYMBOL(DRV_MAILBOX_READMAILDATA);
EXPORT_SYMBOL(DRV_MAILBOX_SENDMAIL);
EXPORT_SYMBOL(DRV_MAILBOX_REGISTERRECVFUNC);

EXPORT_SYMBOL(atfd_hisi_service_access_register_smc);


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
#include <huawei_platform/connectivity/huawei_gps.h>
#else
#include <linux/huawei/gps/huawei_gps.h>
#endif /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0)) */
EXPORT_SYMBOL(set_gps_ref_clk_enable);


/* TODO */


