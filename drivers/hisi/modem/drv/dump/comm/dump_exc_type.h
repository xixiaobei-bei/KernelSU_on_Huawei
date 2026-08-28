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

#ifdef BSP_CONFIG_PHONE_TYPE
#include "../../adrv/adrv.h"
#else
#include <linux/hisi/rdr_pub.h>
#endif

#ifndef __DUMP_EXC_TYPE_H__
#define __DUMP_EXC_TYPE_H__
#include "bsp_dump.h"
/*modem cp异常的rdr id*/
#define RDR_MODEM_CP_DRV_MOD_ID_START           0x00000000
#define RDR_MODEM_CP_DRV_MOD_ID_END             0x0fffffff
#define RDR_MODEM_CP_OSA_MOD_ID_START           0x10000000
#define RDR_MODEM_CP_OSA_MOD_ID_END             0x1fffffff
#define RDR_MODEM_CP_OAM_MOD_ID_START           0x20000000
#define RDR_MODEM_CP_OAM_MOD_ID_END             0x2fffffff
#define RDR_MODEM_CP_GUL2_MOD_ID_START          0x30000000
#define RDR_MODEM_CP_GUL2_MOD_ID_END            0x3effffff
#define RDR_MODEM_CP_CTTF_MOD_ID_START          0x3f000000
#define RDR_MODEM_CP_CTTF_MOD_ID_END            0x3fffffff
#define RDR_MODEM_CP_GUWAS_MOD_ID_START         0x40000000
#define RDR_MODEM_CP_GUWAS_MOD_ID_END           0x4dffffff
#define RDR_MODEM_CP_CAS_MOD_ID_START           0x4e000000
#define RDR_MODEM_CP_CAS_MOD_ID_END             0x4effffff
#define RDR_MODEM_CP_CPROC_MOD_ID_START         0x4f000000
#define RDR_MODEM_CP_CPROC_MOD_ID_END           0x4fffffff
#define RDR_MODEM_CP_GUGAS_MOD_ID_START         0x50000000
#define RDR_MODEM_CP_GUGAS_MOD_ID_END           0x5fffffff
#define RDR_MODEM_CP_GUCNAS_MOD_ID_START        0x60000000
#define RDR_MODEM_CP_GUCNAS_MOD_ID_END          0x6fffffff
#define RDR_MODEM_CP_GUDSP_MOD_ID_START         0x70000000
#define RDR_MODEM_CP_GUDSP_MOD_ID_END           0x7fffffff
#define RDR_MODEM_CP_LPS_MOD_ID_START           0xa0000000
#define RDR_MODEM_CP_LPS_MOD_ID_END             0xafffffff
#define RDR_MODEM_CP_LMSP_MOD_ID_START          0xb0000000
#define RDR_MODEM_CP_LMSP_MOD_ID_END            0xbfffffff
#define RDR_MODEM_CP_TLDSP_MOD_ID_START         0xc0000000
#define RDR_MODEM_CP_TLDSP_MOD_ID_END           0xcfffffff
#define RDR_MODEM_CP_CPHY_MOD_ID_START          0xd0000000
#define RDR_MODEM_CP_CPHY_MOD_ID_END            0xdfffffff
#define RDR_MODEM_CP_IMS_MOD_ID_START           0xe0000000
#define RDR_MODEM_CP_IMS_MOD_ID_END             0xefffffff




/* rdr为CP分配的ID范围为0x82000000~0x82ffffff，定义在rdr_pub.h中*/
#define RDR_MODEM_AP_MOD_ID                     0x82000000  /* AP异常，保存log，整机复位，phone和mbb都有 */
#define RDR_MODEM_CP_MOD_ID                     0x82000001  /* CP异常，保存log，phone和mbb都有 */
#define RDR_MODEM_CP_WDT_MOD_ID                 0x82000003  /* CP看门狗异常，保存log，phone和mbb都有 */
#define RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID    0x82000005  /* 切卡引起的CP单独复位，不保存log，phone独有 */
#define RDR_MODEM_CP_RESET_FAIL_MOD_ID          0x82000006  /* CP单独复位失败，保存log，phone独有 */
#define RDR_MODEM_CP_RESET_RILD_MOD_ID          0x82000007  /* RILD引起的CP单独复位，保存log，phone独有*/
#define RDR_MODEM_CP_RESET_3RD_MOD_ID           0x82000008  /* 3rd modem引起的CP单独复位，保存log，phone独有*/
#define RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID    0x82000009  /* CP单独复位为桩时，整机复位，保存log，phone独有*/
#define RDR_MODEM_CP_RESET_USER_RESET_MOD_ID    0x82000010  /* CP单独复位,用户发起，不保存log,PHONE 独有*/
#define RDR_MODEM_CP_RESET_DLOCK_MOD_ID         0x82000011  /* modem总线挂死*/
#define RDR_MODEM_CP_NOC_MOD_ID                 0x82000012  /* modem内部触发noc */
#define RDR_MODEM_AP_NOC_MOD_ID                 0x82000013  /* modem访问AP触发noc */
#define RDR_MODEM_CP_RESET_FREQUENTLY_MOD_ID    0x82000014  /* c 核频繁单独复位，整机重启*/

#define RDR_MODEM_NOC_MOD_ID                    0x82000030  /* NOC异常, 保存log，phone独有, */
                                                            /* 如果要修改者定义一定要知会AP侧同步修改，否则会导致NOC异常log存储不下来 */
#define RDR_MODEM_DMSS_MOD_ID                   0x82000031  /* DMSS异常, 保存log，phone独有, */
                                                            /* 如果要修改者定义一定要知会AP侧同步修改，否则会导致DMSS异常log存储不下来 */


#define RDR_MODEM_CP_DRV_MOD_ID                 0x82000041
#define RDR_MODEM_CP_OSA_MOD_ID                 0x82000042
#define RDR_MODEM_CP_OAM_MOD_ID                 0x82000043
#define RDR_MODEM_CP_GUL2_MOD_ID                0x82000044
#define RDR_MODEM_CP_CTTF_MOD_ID                0x82000045
#define RDR_MODEM_CP_GUWAS_MOD_ID               0x82000046
#define RDR_MODEM_CP_CAS_MOD_ID                 0x82000047
#define RDR_MODEM_CP_CPROC_MOD_ID               0x82000048
#define RDR_MODEM_CP_GUGAS_MOD_ID               0x82000049
#define RDR_MODEM_CP_GUCNAS_MOD_ID              0x8200004a
#define RDR_MODEM_CP_GUDSP_MOD_ID               0x8200004b
#define RDR_MODEM_CP_LPS_MOD_ID                 0x8200004c
#define RDR_MODEM_CP_LMSP_MOD_ID                0x8200004d
#define RDR_MODEM_CP_TLDSP_MOD_ID               0x8200004e
#define RDR_MODEM_CP_CPHY_MOD_ID                0x8200004f
#define RDR_MODEM_CP_IMS_MOD_ID                 0x82000050
#define RDR_MODEM_AP_DRV_MOD_ID                 0x82000051


#define RDR_DUMP_FILE_PATH_LEN                128
#define RDR_DUMP_FILE_CP_PATH                 "cp_log/"
#define RDR_MODEM_CP_RESET_TIMEOUT            (30000)


typedef struct
{
    u32     modid;
    u32     product_type;
    u64     coreid;
    char    log_path[RDR_DUMP_FILE_PATH_LEN];
    pfn_cb_dump_done    dump_done;
    struct rdr_register_module_result soc_rst;
}rdr_exc_info_s;

typedef struct modid
{
    u32 mdm_id_start;
    u32 mdm_id_end;
    u32 rdr_id;
}DUMP_MOD_ID;


u32 dump_convert_id_mdmcp2rdr(u32 mdmcp_mod_id);
s32 dump_register_modem_exc_info(void);
u32 dump_match_rdr_mod_id(u32 drv_mod_id);

rdr_exc_info_s* dump_get_rdr_exc_info(void);
void dump_save_balong_rdr_info(u32 mod_id);
struct rdr_exception_info_s* dump_get_exception_info_node(u32 mod_id);
void dump_get_reboot_contex(u32* core ,u32* reason);
void dump_set_reboot_contex(dump_reboot_cpu_t core ,dump_reboot_reason_t reason);
void dump_save_rdr_exc_info(u32 modid, u32 etype, u64 coreid, char* logpath, pfn_cb_dump_done fndone);


#endif
