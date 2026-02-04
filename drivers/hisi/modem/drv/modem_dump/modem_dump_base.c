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
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/thread_info.h>
#include <linux/syslog.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/kdebug.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <asm/string.h>
#include <asm/traps.h>
#include "product_config.h"
#include <linux/syscalls.h>
#include "osl_types.h"
#include "osl_io.h"
#include "osl_bio.h"
#include "osl_malloc.h"
#include "bsp_dump.h"
#include "bsp_ipc.h"
#include "bsp_memmap.h"
#include "bsp_wdt.h"
#include "bsp_icc.h"
#include "bsp_onoff.h"
#include "bsp_nvim.h"
#include "bsp_softtimer.h"
#include "bsp_version.h"
#include "bsp_sram.h"
#include "bsp_dump_mem.h"
#include "bsp_coresight.h"
#include "bsp_reset.h"
#include "drv_nv_def.h"
#include "mdrv_om.h"
#include <gunas_errno.h>
#include "modem_dump.h"
#include "drv_nv_id.h"
#include "bsp_nvim.h"
#include "modem_dump_base.h"
#include "modem_tlphy_tcm.h"
#include "modem_cphy_tcm.h"
#include "modem_dump_apr.h"
#include "bsp_dump.h"
#include "bsp_dump_def.h"
#include "bsp_dump_drv.h"
#include "bsp_ap_dump.h"
#include "bsp_rfile.h"
#include "modem_dump_field.h"
#include "modem_dump_area.h"
#include "bsp_wdt.h"
#include "modem_dump_wdt.h"
#include "modem_dump_etb.h"
#include "bsp_fiq.h"

dump_base_info_t*               g_mdm_dump_base_info = BSP_NULL;
modemap_dump_fields_addr_s      g_modemap_dump_fileds_addr;
DUMP_FILE_CFG_STRU              g_dump_file_cfg;
modem_exc_info_s                g_rdr_modem_exc_info;
NV_DUMP_STRU                    g_dump_cfg;
modem_dump_ctrl_s               g_dump_ctrl;
u8*                             g_modem_ddr_map_addr = NULL;
dump_product_type_t             g_product_type = DUMP_PRODUCT_BUTT;

dump_reboot_contex_s            g_dump_reboot_contex = {DUMP_CPU_BUTTON,DUMP_REASON_UNDEF};

/*****************************************************************************
* º¯ Êý Ãû  : dump_memcpy
* ¹¦ÄÜÃèÊö  : ¿½±´¼Ä´æÆ÷º¯Êý
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_memcpy(u32 * dst, const u32 * src, u32 len)
{
    while(len-- > 0)
    {
        *dst++ = *src++;
    }
}
/*****************************************************************************
* º¯ Êý Ãû  : dump_save_file
* ¹¦ÄÜÃèÊö  : ÓÃÓÚÖ´ÐÐ±£´æÎÄ¼þµÄ¹¦ÄÜ
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_file(char * file_name, void * addr, u32 len)
{
    int ret = -1;
    int fd = -1;
    int bytes = -1;

    if(!file_name || !addr || !len)
    {
        return;
    }
    fd = bsp_open(file_name, O_CREAT|O_RDWR|O_SYNC, 0660);
    if(fd < 0)
    {
        dump_fetal("creat file %s failed\n", file_name);
        return;
    }

    bytes = bsp_write(fd, addr, len);
    if(bytes != len)
    {
        dump_fetal("write data to %s failed, bytes %d, len %d\n", file_name, bytes, len);
        (void)bsp_close(fd);
        return;
    }


    ret = bsp_close(fd);
    if(0 != ret)
    {
        dump_fetal("close file failed, ret = %d\n", ret);
        return;
    }

}

/*****************************************************************************
* º¯ Êý Ãû  : dump_create_dir
* ¹¦ÄÜÃèÊö  : ´´½¨reset¡.logµÄ±£´æÄ¿Â¼
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/

int dump_append_file(char * dir, char *filename, void * address, u32 length, u32 max_size)
{
    int ret = -1;
    int fd = -1;
    u32 bytes = 0;
    int len = -1;

    fd = bsp_access((s8*)dir, 0);
    if(0 != fd)
    {
        fd  = bsp_mkdir((s8*)dir, 0660);
        if(fd < 0)
        {
            dump_fetal("create om dir failed! ret = %d\n", fd);

            return fd;
        }
    }

    if(BSP_OK != fd)
    {
        dump_fetal("create dir failed! ret = %d\n", ret);
        goto out;
    }

    ret = bsp_access((s8*)filename, 0);
    if(BSP_OK != ret)
    {
        fd = bsp_open((s8*)filename, RFILE_CREAT|RFILE_RDWR, 0755);
        if(fd < 0)
        {
            dump_fetal("open failed while mode is create, ret = %d\n", fd);
            goto out;
        }
    }
    else
    {
        fd = bsp_open((s8*)filename, RFILE_APPEND|RFILE_RDWR, 0755);
        if(fd < 0)
        {
            dump_fetal("open failed while mode is append, ret = %d\n", fd);
            goto out;
        }

    }

    len = bsp_lseek((u32)fd, 0, SEEK_END);
    if(ERROR == len)
    {
        dump_fetal("seek failed! ret = %d\n", len);
        goto out1;
    }

    if (len >= (int)max_size)
    {
        (void)bsp_close((u32)fd);
        ret = bsp_remove((s8*)filename);
        if (BSP_OK != ret)
        {
            dump_fetal("remove failed! ret = %d\n", ret);
            goto out;
        }

        fd = bsp_open((s8*)filename, RFILE_CREAT|RFILE_RDWR, 0755);
        if(fd < 0)
        {
            dump_fetal("create failed! ret = %d\n", fd);
            goto out;
        }
    }

    bytes = (u32)bsp_write((u32)fd, address, length);
    if(bytes != length)
    {
        dump_fetal("write data failed! ret = %d\n", bytes);
        ret = BSP_ERROR;
        goto out1;
    }

    (void)bsp_close((u32)fd);

    return BSP_OK;

out1:
    (void)bsp_close((u32)fd);
out:
    return ret;
}

/*****************************************************************************
* º¯ Êý Ãû  : dump_create_dir
* ¹¦ÄÜÃèÊö  : ´´½¨cp_logµÄÄ¿Â¼±£Ö¤hidpµÈ¹¤¾ßÄÜ¹»Õý³£µ¼log
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
int dump_create_dir(char *path)
{
    int fd = -1;
    char* dir_name = NULL;
    u32 len = 0;

    if(path == NULL)
    {
        return BSP_ERROR;
    }
    dump_fetal("path = %s\n", path);
    /* Èç¹ûÎÄ¼þ¼Ð²»´æÔÚ£¬´´½¨ÐÂÎÄ¼þ¼Ð*/
    fd = bsp_access(path, 0); //F_OK, ¼ì²éÎÄ¼þÊÇ·ñ´æÔÚ
    if(0 != fd)
    {
        len = strlen(path);

        if(path[len - 1] == '/')
        {
            dir_name = kmalloc((len + 1), GFP_KERNEL);
            if(dir_name == NULL)
            {
                dump_fetal("malloc memry fail\n");
                return BSP_ERROR;
            }
            memset(dir_name,'\0',(len + 1));
            memcpy(dir_name,path,(len - 1));
            fd  = bsp_mkdir(dir_name, 0770);
        }
        else
        {
            fd  = bsp_mkdir(path, 0770);
        }
        if(dir_name != NULL)
        {
            kfree(dir_name);
        }
        if(fd < 0)
        {
            dump_fetal("create dir fail,fd = %d\n ",fd);
            return fd;
        }
    }

    return BSP_OK;
}
 /*****************************************************************************
* º¯ Êý Ãû  : dump_show_stack
* ¹¦ÄÜÃèÊö  : ´òÓ¡µ÷ÓÃÕ»£¬ÓÃÓÚ·ÇarmÒì³£
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/

void dump_show_stack(void)
{
    if (!(AP_DUMP_REASON_ARM == g_mdm_dump_base_info->arg1))
    {
        dump_fetal("###########show mem and current task stack start##############!\n");

        show_mem(0);

        if(DUMP_T_TASK_ERROR(g_mdm_dump_base_info->modId))
        {
            dump_fetal("not current task exc\n");
            show_stack(find_task_by_vpid(g_mdm_dump_base_info->arg1),NULL);
        }
        else
        {
            show_stack(current, NULL);
        }

        dump_fetal("###########show mem and current task stack end################!\n");
    }
}

/*****************************************************************************
* º¯ Êý Ãû  : dump_product_type_init
* ¹¦ÄÜÃèÊö  : ³õÊ¼»¯²úÆ·ÐÎÌ¬
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_product_type_init(void)
{
    struct device_node *node = NULL;
    const char * product_type  = NULL;

    node = of_find_compatible_node(NULL, NULL, "hisilicon,smntn_type");
    if (!node)
    {
        dump_fetal("dts node mntn type not found!\n");
        return;
    }

    if(of_property_read_string(node, "product_type", &product_type))
    {
        dump_fetal("read product_type failed!\n");
        return;
    }

    if(0 == strncmp(product_type, "MBB", strlen("MBB")))
    {
        g_product_type = DUMP_MBB;
    }
    else if(0 == strncmp(product_type, "PHONE", strlen("PHONE")))
    {
        g_product_type = DUMP_PHONE;
    }
    else
    {
        g_product_type = DUMP_PRODUCT_BUTT;
    }
}

/*****************************************************************************
* º¯ Êý Ãû  : dump_get_product_type
* ¹¦ÄÜÃèÊö  : »ñÈ¡dumpµ±Ç°¹¤×÷µÄ²úÆ·ÐÎÌ¬
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
dump_product_type_t dump_get_product_type(void)
{
    if(DUMP_PHONE == g_product_type)
    {
        return DUMP_PHONE;
    }
    else
    {
        return DUMP_MBB;
    }
}

/*****************************************************************************
* º¯ Êý Ãû  : bsp_om_debug_init
* ¹¦ÄÜÃèÊö  : »ñÈ¡µ±Ç°ÐèÒª±£´æµÄÎÄ¼þÅäÖÃ±í
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
int bsp_om_debug_init(void)
{

    if(BSP_OK != bsp_nvm_read(NV_ID_DRV_DUMP_FILE, (u8 *)&g_dump_file_cfg, sizeof(DUMP_FILE_CFG_STRU)))
    {
        g_dump_file_cfg.file_cnt = 2;
        g_dump_file_cfg.file_list.file_bits.mdm_dump = 0x1;
        g_dump_file_cfg.file_list.file_bits.mdm_share= 0x1;
        g_dump_file_cfg.file_list.file_bits.mdm_ddr  = 0x1;
        g_dump_file_cfg.file_list.file_bits.mdm_etb  = 0x1;
        g_dump_file_cfg.file_list.file_bits.lphy_tcm = 0x1;
        g_dump_file_cfg.file_list.file_bits.lpm3_tcm = 0x1;
        g_dump_file_cfg.file_list.file_bits.ap_etb   = 0x1;
        g_dump_file_cfg.file_list.file_bits.reset_log= 0x1;

        dump_error("read nv 0x%x fail, use default config\n", NV_ID_DRV_DUMP_FILE);
    }
    return BSP_OK;
}

/*****************************************************************************
* º¯ Êý Ãû  : dump_config_init
* ¹¦ÄÜÃèÊö  : »ñÈ¡µ±Ç°¹¦ÄÜÅäÖÃ
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_config_init(void)
{
    s32 ret;

    ret = bsp_nvm_read(NVID_DUMP, (u8*)&g_dump_cfg, sizeof(NV_DUMP_STRU));
    if (BSP_OK != ret)
    {
        /*Ê¹ÓÃÄ¬ÈÏÖµ*/
        g_dump_cfg.dump_cfg.Bits.dump_switch     = 0x1;
        g_dump_cfg.dump_cfg.Bits.ARMexc          = 0x1;
        g_dump_cfg.dump_cfg.Bits.stackFlow       = 0x1;
        g_dump_cfg.dump_cfg.Bits.taskSwitch      = 0x1;
        g_dump_cfg.dump_cfg.Bits.intSwitch       = 0x1;
        g_dump_cfg.dump_cfg.Bits.intLock         = 0x1;
        g_dump_cfg.dump_cfg.Bits.appRegSave1     = 0x0;
        g_dump_cfg.dump_cfg.Bits.appRegSave2     = 0x0;
        g_dump_cfg.dump_cfg.Bits.appRegSave3     = 0x0;
        g_dump_cfg.dump_cfg.Bits.commRegSave1    = 0x0;
        g_dump_cfg.dump_cfg.Bits.commRegSave2    = 0x0;
        g_dump_cfg.dump_cfg.Bits.commRegSave3    = 0x0;
        g_dump_cfg.dump_cfg.Bits.sysErrReboot    = 0x1;
        g_dump_cfg.dump_cfg.Bits.reset_log       = 0x1;
        g_dump_cfg.dump_cfg.Bits.fetal_err       = 0x1;

        dump_error("read NV failed, use the default value!.ret = %d nv id = 0x%x\n", ret, NVID_DUMP);
    }

    return BSP_OK;
}


/*****************************************************************************
* º¯ Êý Ãû  : dump_save_modem_sysctrl
* ¹¦ÄÜÃèÊö  : ±£´æmodemµÄÏµÍ³¿ØÖÆ¼Ä´æÆ÷
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_modem_sysctrl(void)
{
    u32 * field_addr = NULL;
    u32 * reg_addr = NULL;
    u32 reg_size = 0;

    field_addr = (u32 *)bsp_dump_get_field_addr(DUMP_CP_SYSCTRL);
    if(field_addr == NULL)
    {
        dump_fetal("SYSCTRL field addr is NULL\n");
        return;
    }

    /* sysctrl mdm */
    reg_addr = (u32 *)bsp_sysctrl_addr_byindex(sysctrl_mdm);
    reg_size = 0x1000;

    /* coverity[dereference] */
    dump_memcpy(field_addr, reg_addr, reg_size >> 2);
    *(field_addr + (reg_size >> 2) - 4) = (u32)OM_SYSCTRL_MAGIC;
    *(field_addr + (reg_size >> 2) - 3) = (uintptr_t)bsp_sysctrl_addr_phy_byindex(sysctrl_mdm);
    *(field_addr + (reg_size >> 2) - 2) = reg_size;
    *(field_addr + (reg_size >> 2) - 1) = 0;
    field_addr = field_addr + (reg_size >> 2);

    dump_fetal("dump_save_modem_sysctrl finish\n");
}

void modem_wdt_hook(void)
{
    system_error(DRV_ERRNO_DUMP_CP_WDT, DUMP_REASON_WDT, 0, 0, 0);
}

/*****************************************************************************
* º¯ Êý Ãû  : dump_register_hook
* ¹¦ÄÜÃèÊö  : ×¢²ámodemÒì³£
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_register_hook(void)
{
    return bsp_wdt_register_hook(WDT_CCORE_ID,modem_wdt_hook);
}

/*****************************************************************************
* º¯ Êý Ãû  : dump_get_cp_task_name_by_id
* ¹¦ÄÜÃèÊö  : Í¨¹ýÈÎÎñid²éÕÒÈÎÎñÃû
*
* ÊäÈë²ÎÊý  :task_id
* Êä³ö²ÎÊý  :task_name

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_get_cp_task_name_by_id(u32 task_id, char* task_name)
{
    dump_task_info_s * temp_task_name = NULL;
    u8* task_name_table = NULL;
    u32 task_cnt = 0;
    u32 task_index = 0;


    /*CP´æ´¢ÈÎÎñÃûµÄÇøÓò*/
    task_name_table = bsp_dump_get_field_addr(DUMP_CP_ALLTASK_NAME);
    if(NULL == task_name_table)
    {
        dump_fetal("get cp task name ddr fail\n");
        return;
    }

    task_cnt = *((u32 *)task_name_table + 3)/4;

    /* Æ«ÒÆ10×Ö½Ú£¬È¥µô¶ÓÁÐÍ· */
    task_name_table += 0x10;
    temp_task_name = (dump_task_info_s *)task_name_table;
    dump_fetal("task_cnt:0x%x\n", task_cnt);

    /*²éÕÒÈÎÎñÃû*/
    for(task_index = 0;task_index < task_cnt; task_index++)
    {

        if(temp_task_name->task_id == task_id)
        {
            dump_fetal("reboot task:%s\n", temp_task_name->task_name);
            memcpy(task_name,temp_task_name->task_name,12);
            break;
        }
        temp_task_name++;
    }

}

/*****************************************************************************
* º¯ Êý Ãû  : dump_save_cp_base_info
* ¹¦ÄÜÃèÊö  : ±£´æmodem apµÄ»ù±¾ÐÅÏ¢
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_cp_base_info(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length)
{
    cp_dump_base_info_t* modem_cp_base_info = NULL;
    dump_base_info_t* kernel_base_info = NULL;

    modem_cp_base_info = (cp_dump_base_info_t*)bsp_dump_get_field_addr(DUMP_CP_BASE_INFO);
    if(modem_cp_base_info == NULL)
    {
        dump_fetal("get cp base info fail\n");
        return;
    }
    modem_cp_base_info->modId = mod_id;
    modem_cp_base_info->arg1 = arg1;
    modem_cp_base_info->arg2 = arg2;
    modem_cp_base_info->arg3 = (u32)(uintptr_t)data;
    modem_cp_base_info->arg3_length = length;

    if(modem_cp_base_info->reboot_context == DUMP_CTX_TASK)
    {
        modem_cp_base_info->reboot_task = modem_cp_base_info->current_task_id;
        dump_get_cp_task_name_by_id( modem_cp_base_info->current_task_id,modem_cp_base_info->taskName);
        modem_cp_base_info->reboot_int = (u32)(-1);
    }
    else
    {
        modem_cp_base_info->reboot_task = (u32)(-1);
        modem_cp_base_info->reboot_int = modem_cp_base_info->current_int;

    }

    if(dump_get_product_type() == DUMP_MBB)
    {
        kernel_base_info = (dump_base_info_t*)bsp_dump_get_field_addr(DUMP_KERNEL_BASE_INFO);
        if(kernel_base_info == NULL)
        {
            dump_fetal("get kernel base info fail\n");
            return;
        }
        kernel_base_info->reboot_int = (u32)(-1);
        kernel_base_info->reboot_task = (u32)(-1);
    }

    g_mdm_dump_base_info->reboot_task = (u32)(-1);
    g_mdm_dump_base_info->reboot_int = (u32)(-1);

    dump_fetal("save cp base info finish\n");
    return;
}
/*****************************************************************************
* º¯ Êý Ãû  : dump_save_base_info
* ¹¦ÄÜÃèÊö  : ±£´æmodem apµÄ»ù±¾ÐÅÏ¢
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_base_info(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length)
{

    g_mdm_dump_base_info->modId = mod_id;
    g_mdm_dump_base_info->arg1 = arg1;
    g_mdm_dump_base_info->arg2 = arg2;
    g_mdm_dump_base_info->arg3 = data;
    g_mdm_dump_base_info->arg3_length = length;

    if(BSP_MODU_OTHER_CORE == g_mdm_dump_base_info->modId)
    {
        g_mdm_dump_base_info->reboot_task = (u32)(-1);
        g_mdm_dump_base_info->reboot_int = (u32)(-1);
    }
    else
    {
        if(in_interrupt())
        {
            g_mdm_dump_base_info->reboot_task = (u32)(-1);
            memset(g_mdm_dump_base_info->taskName, 0, sizeof(g_mdm_dump_base_info->taskName));

            if(DUMP_MBB == dump_get_product_type())
            {
                g_mdm_dump_base_info->reboot_int = g_mdm_dump_base_info->current_int;
            }
            else
            {
                g_mdm_dump_base_info->reboot_int = 0xAAAABBBB;
            }
            g_mdm_dump_base_info->reboot_context = DUMP_CTX_INT;

        }
        else
        {
            if(g_mdm_dump_base_info->modId == 0x11000025 || g_mdm_dump_base_info->modId == 0x1100002A)
            {
                /* AºËVOSÖ»¼ÇÂ¼µÄÈÎÎñµÄpid£¬Ã»ÓÐÈÎÎñµÄtask_structÖ¸Õë£¬²»¼ÇÂ¼ÈÎÎñÃû */
                g_mdm_dump_base_info->reboot_task = g_mdm_dump_base_info->arg1;
            }
            else
            {
                g_mdm_dump_base_info->reboot_task =  ((struct task_struct *)(current))->pid;
                dump_fetal("g_mdm_dump_base_info->reboot_task = 0x%x\n",g_mdm_dump_base_info->reboot_task);
                if((u32)(-1) != g_mdm_dump_base_info->reboot_task)
                {
                    memset(g_mdm_dump_base_info->taskName,0,16);
                    memcpy(g_mdm_dump_base_info->taskName,((struct task_struct *)(current))->comm, 16);
                    dump_fetal("g_mdm_dump_base_info->taskName = %s\n",g_mdm_dump_base_info->taskName);
                }
            }

            g_mdm_dump_base_info->reboot_int = (u32)(-1);
            g_mdm_dump_base_info->reboot_context = DUMP_CTX_TASK;

        }
    }

    dump_fetal("save base info finish\n");
    return;
}

/*****************************************************************************
* º¯ Êý Ãû  : dump_update_mdm_rst_fail_info
* ¹¦ÄÜÃèÊö  : ¸üÐÂmodemµ¥¶À¸´Î»Ê§°ÜµÄ»ù±¾ÐÅÏ¢
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_momdem_reset_info(u32 modid,char* name)
{
    u32 len = 0;
    g_mdm_dump_base_info->modId = modid;
    g_mdm_dump_base_info->arg1 = 0;
    g_mdm_dump_base_info->arg2 = 0;
    g_mdm_dump_base_info->arg3 = NULL;
    g_mdm_dump_base_info->arg3_length = 0;
    if(name != NULL)
    {
        len = strlen(name);
        memcpy(g_mdm_dump_base_info->taskName,name,len > 16 ? 16 : len);
    }
    g_mdm_dump_base_info->reboot_context = DUMP_CTX_TASK;
    g_mdm_dump_base_info->reboot_int= 0xFFFFFFFF;

    dump_fetal("modem reset fail update base info finish modid= 0x%x\n",modid);
}

/*****************************************************************************
* º¯ Êý Ãû  : dump_base_info_init
* ¹¦ÄÜÃèÊö  : modem ap»ù±¾ÐÅÏ¢³õÊ¼»¯
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_base_info_init(void)
{
    if(g_modemap_dump_fileds_addr.base_info_addr != NULL)
    {
        g_mdm_dump_base_info = (dump_base_info_t*)g_modemap_dump_fileds_addr.base_info_addr;
        memset(g_mdm_dump_base_info, 0, sizeof(dump_base_info_t));

        g_mdm_dump_base_info->vec = DUMP_ARM_VEC_UNKNOW;
    }
    else
    {
        return BSP_ERROR;
    }
    dump_fetal("dump_base_info_init finish\n");
    return BSP_OK;
}

/*****************************************************************************
* º¯ Êý Ãû  : dump_memmap_modem_ddr
* ¹¦ÄÜÃèÊö  : Ó³Éämodem ddrµÄÄÚ´æ£¬Ö»ÔÚÊÖ»ú°æ±¾ÉÏÊ¹ÓÃ£¬mbbÆ½Ì¨ÉÏÔÚfastbootµ¼³ö
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_memmap_modem_ddr(void)
{
    dump_product_type_t type = dump_get_product_type();

    dump_fetal("mdm_ddr= %d,type=%d\n",g_dump_file_cfg.file_list.file_bits.mdm_ddr,type);

    if(g_dump_file_cfg.file_list.file_bits.mdm_ddr == 1 && type == DUMP_PHONE )
    {
        g_modem_ddr_map_addr = (u8 *)ioremap_wc(DDR_MCORE_ADDR, DDR_MCORE_SIZE);

        if(g_modem_ddr_map_addr == NULL)
        {
            dump_fetal("map g_modem_ddr_map_addr fail\n");
        }
        else
        {
            dump_fetal("g_modem_ddr_map_addr = 0x%p \n",g_modem_ddr_map_addr);
        }
    }
    dump_fetal("dump_memmap_modem_ddr finish\n");
}
/*****************************************************************************
* º¯ Êý Ãû  : dump_wait_modem_cp_save_done
* ¹¦ÄÜÃèÊö  : ±£´æmodem cpµÄÇøÓò
*
* ÊäÈë²ÎÊý  : u32 ms  µÈ´ýµÄºÁÃëÊý
              bool wait ÊÇ·ñÑ­»·µÈ´ý
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_wait_modem_cp_save_done(u32 ms,bool wait)
{
    u32 time_start = 0;

    time_start = bsp_get_elapse_ms();

    do{

        if( BSP_OK == dump_get_cp_save_done())
        {
            dump_fetal("cp save done\n");
            return BSP_OK;
        }

        if(ms <= (bsp_get_elapse_ms()-time_start))
        {
            dump_fetal("dump save max time out\n");
            return BSP_ERROR;
        }

        if(wait)
        {
            msleep(10);
        }

    }while(1);

    return BSP_ERROR;
}
/*****************************************************************************
* º¯ Êý Ãû  : dump_get_rdr_exc_info
* ¹¦ÄÜÃèÊö  : »ñÈ¡rdrµÄÒì³£±äÁ¿µØÖ·
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/

void* dump_get_rdr_exc_info(void)
{
    return &g_rdr_modem_exc_info;
}
/*****************************************************************************
* º¯ Êý Ãû  : dump_save_task
* ¹¦ÄÜÃèÊö  : ±£´æmodem logµÄÈë¿Úº¯Êý
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
int dump_save_task(void *data)
{
    dump_product_type_t type = DUMP_PRODUCT_BUTT;
    s32 ret = BSP_ERROR;
    type = dump_get_product_type();
    if(type == DUMP_PRODUCT_BUTT)
    {
        dump_fetal("get product type error\n");
        type = DUMP_MBB;
    }

    /* coverity[no_escape] */
    while(1)
    {
        down(&g_dump_ctrl.sem_dump_task);

        dump_fetal("down g_dump_ctrl.sem_dump_task\n");

        if(DUMP_TASK_JOB_SAVE_REBOOT == (g_dump_ctrl.dump_task_job & DUMP_TASK_JOB_SAVE_REBOOT))
        {
            dump_fetal("begin to save modem log \n");

            ret = dump_wait_modem_cp_save_done(5000, true);
            if(ret == BSP_ERROR && g_dump_reboot_contex.reboot_core != DUMP_CPU_COMM)
            {
                ret = bsp_send_cp_fiq(FIQ_DUMP);
                if(ret == BSP_ERROR)
                {
                   dump_cs_etb(WDT_CCORE_ID);
                   dump_fetal("send fiq fail\n");
                }
                ret = dump_wait_modem_cp_save_done(5000, true);
                if(ret == BSP_ERROR)
                {
                    dump_fetal("ipc fiq save log both fail\n");
                }

            }

            dump_save_log_files(g_rdr_modem_exc_info.log_path,&g_dump_file_cfg, type);

            if(g_rdr_modem_exc_info.dump_done != BSP_NULL)
            {
                g_rdr_modem_exc_info.dump_done(g_rdr_modem_exc_info.modid, g_rdr_modem_exc_info.coreid);

                dump_fetal("notify rdr dump handle finished\n");
            }
        }

        g_dump_ctrl.dump_task_job = 0;
    }

    return BSP_OK;
}

/*****************************************************************************
* º¯ Êý Ãû  : dump_save_usr_data
* ¹¦ÄÜÃèÊö  : ±£´æÓÃ»§Êý¾ÝÇø
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_usr_data(char *data, u32 length)
{
    s32 len =0;
    void* addr = NULL;
    dump_field_map_t* pfield = NULL;

    if ((NULL != data) && (length))
    {
        pfield = (dump_field_map_t*)bsp_dump_get_field_map(DUMP_MODEMAP_USER_DATA);
        addr = g_modemap_dump_fileds_addr.usr_data_addr;
        len = (length > DUMP_MODEMAP_USER_DATA_SIZE) ? DUMP_MODEMAP_USER_DATA_SIZE : length;

        if(addr != NULL)
        {
            memcpy((void *)addr, (const void * )data, (size_t)len);
        }

        if(pfield)
        {
            pfield->length = len;
        }
    }
    dump_fetal("dump save usr data finish\n");
    return;
}

/*****************************************************************************
* º¯ Êý Ãû  : dump_wait_for_reboot
* ¹¦ÄÜÃèÊö  : µÈ´ýÖØÆô
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
 void dump_wait_for_reboot(void)
{
    /* coverity[no_escape] */
    do{

    }while(1);
}

/*****************************************************************************
* º¯ Êý Ãû  : dump_save_and_reboot
* ¹¦ÄÜÃèÊö  : ´¥·¢±£´æÈÎÎñ£¬²¢ÇÒ¸´Î»
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_and_reboot(void)
{
    g_dump_ctrl.dump_task_job = DUMP_TASK_JOB_SAVE_REBOOT;

    up(&g_dump_ctrl.sem_dump_task);

    dump_fetal("up g_dump_ctrl.sem_dump_task \n");

    return;
}

/*****************************************************************************
* º¯ Êý Ãû  : dump_save_arm_regs
* ¹¦ÄÜÃèÊö  : ±£´æµ±Ç°ÈÎÎñµÄ¼Ä´æÆ÷ÐÅÏ¢
*
* ÊäÈë²ÎÊý  : addr ±£´æµØÖ·
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_arm_regs(u32 addr)
{
}


/*****************************************************************************
* º¯ Êý Ãû  : dump_save_exc_task
* ¹¦ÄÜÃèÊö  : ±£´æmodemµÄÒì³£ÈÎÎñÐÅÏ¢
*
* ÊäÈë²ÎÊý  : addr ±£´æµØÖ·
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_exc_task(void)
{
    struct task_struct *task = get_current();

    if(g_mdm_dump_base_info == NULL)
    {
        return;
    }

    if(dump_get_product_type() == DUMP_MBB)
    {
        g_mdm_dump_base_info->current_task = (u32)(uintptr_t)task;

        if(g_modemap_dump_fileds_addr.task_stack_addr != NULL)
        {
            memcpy((void * )g_modemap_dump_fileds_addr.task_stack_addr , (const void * )task->stack, (size_t )THREAD_SIZE);/* [false alarm]:ÆÁ±ÎFortify´íÎó */
        }
    }
    else
    {
        g_mdm_dump_base_info->current_task = (u32)(uintptr_t)task;
        memset(g_mdm_dump_base_info->taskName,0,16);
        memcpy(g_mdm_dump_base_info->taskName, task->comm, 15);
        dump_fetal("task name is %s\n",g_mdm_dump_base_info->taskName);
        dump_fetal("task pid is 0x%x\n",g_mdm_dump_base_info->current_task);
    }


    dump_fetal("save exc task finish\n");

    return;
}


/*****************************************************************************
* º¯ Êý Ãû  : dump_notify_target
* ¹¦ÄÜÃèÊö  : Í¨Öªmodem cp
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_notify_target(u32 mod_id)
{
    if(g_dump_reboot_contex.reboot_core == DUMP_CPU_COMM)
    {
        dump_fetal("CP exception ,no need to notify C core 0x%x\n", mod_id);
    }
    else
    {
        bsp_ipc_int_send(IPC_CORE_CCORE, IPC_CCPU_SRC_ACPU_DUMP);
        dump_fetal("notify modem ccore success \n");
    }
}

/*****************************************************************************
* º¯ Êý Ãû  : dump_match_rdr_mod_id
* ¹¦ÄÜÃèÊö  : ½«drvµÄ´íÎóÂë×ª»»ÎªrdrµÄ´íÎóÂë
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
u32 dump_match_rdr_mod_id(u32 drv_mod_id)
{
    u32 rdr_mod_id = RDR_MODEM_AP_MOD_ID;

    if(DUMP_PHONE == dump_get_product_type())
    {
        switch(drv_mod_id)
        {
        case DRV_ERRNO_RESET_SIM_SWITCH:
            rdr_mod_id = RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID;
            return rdr_mod_id;
        case NAS_REBOOT_MOD_ID_RILD:
            rdr_mod_id = RDR_MODEM_CP_RESET_RILD_MOD_ID;
            return rdr_mod_id;
        case DRV_ERRNO_RESET_3RD_MODEM:
            rdr_mod_id = RDR_MODEM_CP_RESET_3RD_MOD_ID;
            return rdr_mod_id;
        case DRV_ERRNO_RESET_REBOOT_REQ:
            rdr_mod_id = RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID;
            return rdr_mod_id;
        case DRV_ERROR_USER_RESET:
            rdr_mod_id = RDR_MODEM_CP_RESET_USER_RESET_MOD_ID;
            return rdr_mod_id;
        default:
            break;
        }
    }

    if((DRV_ERRNO_DUMP_CP_WDT == drv_mod_id))
    {
        rdr_mod_id = RDR_MODEM_CP_WDT_MOD_ID;
    }
    else
    {
        rdr_mod_id = RDR_MODEM_AP_MOD_ID;
    }

    return rdr_mod_id;

}

/*****************************************************************************
* º¯ Êý Ãû  : bsp_dump_save_task_init
* ¹¦ÄÜÃèÊö  : ´´½¨modem ap ±£´ælog µÄÈÎÎñº¯Êý
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 bsp_dump_save_task_init(void)
{
    struct task_struct * pid = NULL;
    struct sched_param   param = {0,};

    sema_init(&g_dump_ctrl.sem_dump_task, 0);
    g_dump_ctrl.dump_task_job = 0;

    pid = (struct task_struct *)kthread_run(dump_save_task, 0, "dump_save");
    if (IS_ERR((void*)pid))
    {
        dump_error("bsp_dump_save_task_init[%d]: create kthread task failed! ret=%p\n", __LINE__, pid);
        return BSP_ERROR;
    }
    g_dump_ctrl.dump_task_id = (uintptr_t)pid;

    param.sched_priority = 97;
    if (BSP_OK != sched_setscheduler(pid, SCHED_FIFO, &param))
    {
        dump_error("bsp_dump_save_task_init[%d]: sched_setscheduler failed!\n", __LINE__);
        return BSP_ERROR;
    }

    dump_fetal("bsp_dump_save_task_init finish\n");

    return BSP_OK;
}

/*****************************************************************************
* º¯ Êý Ãû  : modem_dump_field_init
* ¹¦ÄÜÃèÊö  : ³õÊ¼»¯modem apÐèÒªÊ¹ÓÃfiled ¿Õ¼ä
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/

void modem_dump_field_init(void)
{
    g_modemap_dump_fileds_addr.base_info_addr = bsp_dump_register_field(DUMP_MODEMAP_BASE_INFO,"MDMAP_BASE_INFO",NULL,NULL,DUMP_MODEMAP_BASE_INFO_SIZE,0);
    g_modemap_dump_fileds_addr.usr_data_addr = bsp_dump_register_field(DUMP_MODEMAP_USER_DATA,  "USER_DATA",  NULL,NULL,DUMP_MODEMAP_USER_DATA_SIZE,0);
    dump_fetal("modem_dump_field_init finish\n");
}

/*****************************************************************************
* º¯ Êý Ãû  : bsp_dump_get_reboot_contex
* ¹¦ÄÜÃèÊö  : ÎªaprÌá¹©µÄ½Ó¿ÚÓÃÓÚ»ñÈ¡µ±Ç°µÄÒì³£ºËÒÔ¼°Òì³£Ô­Òò
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void bsp_dump_get_reboot_contex(u32* core ,u32* reason)
{
    if(core != NULL)
    {
        *core = (u32)g_dump_reboot_contex.reboot_core;
    }
    if(reason != NULL)
    {
        *reason = (u32)g_dump_reboot_contex.reboot_reason;
    }
}

/*****************************************************************************
* º¯ Êý Ãû  : dump_save_log_files
* ¹¦ÄÜÃèÊö  : ±£´æÈ«²¿µÄmodem logÎÄ¼þ
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_log_files(char * dir_name, DUMP_FILE_CFG_STRU * cfg, dump_product_type_t type)
{
    char file_name[MODEM_DUMP_FILE_NAME_LENGTH] = {0,};
    u8 * data = NULL;
    int ret = -1;
    u32 is_beta = 0;
    int fd = -1;
    char temp_reset_info[256] = {0,};
    struct dump_global_area_ctrl_s global_area = {0,};

    if(dir_name == NULL || cfg == NULL)
    {
        dump_fetal("dir_name or cfg is null\n");
        return;
    }

    fd = dump_create_dir(dir_name);
    if(fd < 0)
    {
        dump_fetal("creat dir fail exit\n");
        return;
    }
    is_beta = (u32)(bbox_check_edition() == EDITION_INTERNAL_BETA);

     dump_fetal("[dump]: enter save log file\n");

    /* save modem_dump.bin all product*/
    if(cfg->file_list.file_bits.mdm_dump == 1  && type == DUMP_PHONE)
    {
        memset(file_name, 0, sizeof(file_name));
        snprintf(file_name, sizeof(file_name), "%smodem_dump.bin", dir_name);
        ret = dump_get_global_info(&global_area);
        if(ret == BSP_OK && global_area.virt_addr != NULL)
        {
            dump_save_file(file_name, (u8 *)global_area.virt_addr, global_area.length);
            dump_fetal("[dump]: save %s finished\n", file_name);
        }

    }

    /* save modem_etb.bin  all product*/
    if(cfg->file_list.file_bits.mdm_etb == 1)
    {
        memset(file_name, 0, sizeof(file_name));
        snprintf(file_name, sizeof(file_name), "%smodem_etb.bin", dir_name);
        data = (u8 *)bsp_dump_get_field_addr(DUMP_CP_UTRACE);
        if(data != NULL)
        {
            dump_save_file(file_name, data, 0x2400);
            dump_fetal("[dump]: save %s finished\n", file_name);
        }
    }

    /* save lphy_tcm.bin all product*/
    if(cfg->file_list.file_bits.lphy_tcm== 1)
    {
        dump_save_lphy_tcm(dir_name);
    }

    /* save cphy_tcm.bin all product*/
    if(cfg->file_list.file_bits.cphy_tcm == 1)
    {
        dump_save_cphy_tcm(dir_name);
    }

    /* save modem_sram.bin all product*/
    if((cfg->file_list.file_bits.mdm_sram == 1)&&(TRUE == is_beta))
    {
        memset(file_name, 0, sizeof(file_name));
        snprintf(file_name, sizeof(file_name), "%smodem_sram.bin", dir_name);
        dump_save_file(file_name, (u8 *)g_mem_ctrl.sram_virt_addr, g_mem_ctrl.sram_mem_size);
        dump_fetal("[dump]: save %s finished\n", file_name);
    }

    /* save modem_share.bin£¬only phone */
    if(cfg->file_list.file_bits.mdm_share == 1 && type == DUMP_PHONE && (TRUE == is_beta))
    {
        memset(file_name, 0, sizeof(file_name));
        snprintf(file_name, sizeof(file_name), "%smodem_share.bin", dir_name);
        dump_save_file(file_name, (u8 *)g_mem_ctrl.sddr_virt_addr, g_mem_ctrl.sddr_mem_size);
        dump_fetal("[dump]: save %s finished\n", file_name);
    }

    /* save modem_ddr.bin£¬only phone */
    if(cfg->file_list.file_bits.mdm_ddr == 1 && type == DUMP_PHONE && (TRUE == is_beta))
    {
        memset(file_name, 0, sizeof(file_name));
        snprintf(file_name, sizeof(file_name), "%smodem_ddr.bin", dir_name);

        if(NULL == g_modem_ddr_map_addr)
        {
            dump_fetal("ioremap MODEM DDR fail\n");
        }
        else
        {
            dump_save_file(file_name, g_modem_ddr_map_addr, DDR_MCORE_SIZE);
            dump_fetal("[dump]: save %s finished\n", file_name);
        }
    }

    /* save reset.log only phone */
    if(cfg->file_list.file_bits.reset_log == 1)
    {
        om_get_reset_info(temp_reset_info, 256);
        memset(file_name, 0, sizeof(file_name));
        snprintf(file_name, sizeof(file_name), "%sreset.log", dir_name);
        dump_fetal("[dump]: save %s start\n", file_name);
        dump_save_file(file_name, (u8 *)temp_reset_info, strlen(temp_reset_info));
        dump_fetal("[dump]: save %s finished\n", file_name);
    }

    dump_fetal("dump all files save finished\n");

}

