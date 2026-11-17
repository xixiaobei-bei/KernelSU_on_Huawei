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
#ifndef __DUMP_MDMAP_CORE_H__
#define __DUMP_MDMAP_CORE_H__
#include <product_config.h>
#include "osl_types.h"
#ifdef BSP_CONFIG_PHONE_TYPE
#include "../../adrv/adrv.h"
#else
#include <linux/hisi/rdr_pub.h>
#endif


#define DUMP_STATUS_FILE  "dump_status.txt"
#define PATH_ROOT         "/data/hisi_logs/"

/*modem ap flag ?¡§¨°?*/
#define DUMP_INIT_FLAG_CONFIG                 (0x5B5B0000)
#define DUMP_INIT_FLAG_BASEINFO               (0x5B5B0001)
#define DUMP_INIT_FLAG_SAVETASK               (0x5B5B0002)
#define DUMP_INIT_FLAG_RDR_REG                (0x5B5B0003)
#define DUMP_INIT_FLAG_MDMAP                  (0x5B5B0004)
#define DUMP_INIT_FLAG_MDMCP                  (0x5B5B0005)
#define DUMP_INIT_FLAG_APR                    (0x5B5B0005)
#define DUMP_INIT_FLAG_DONE                   (0x5B5B0006)

__init s32 dump_mdmap_init(void);
void dump_save_mdm_mandatory_logs(char* dir_name);
s32 dump_mdmap_callback(u32 modid, u32 etype, u64 coreid, char* logpath, pfn_cb_dump_done fndone);
void dump_notify_save_done(char * path);

#endif


