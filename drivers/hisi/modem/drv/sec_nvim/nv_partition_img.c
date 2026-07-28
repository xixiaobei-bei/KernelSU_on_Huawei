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


#include <linux/vmalloc.h>
#include <linux/syscalls.h>
#include <bsp_nvim.h>
#include <bsp_rfile.h>
#include <bsp_blk.h>
#include <bsp_onoff.h>
#include <ptable_com.h>
#include <securec.h>
#include "nv_comm.h"
#include "nv_ctrl.h"
#include "nv_debug.h"
#include "nv_index.h"
#include "nv_partition_img.h"
#include "bsp_dump.h"
#include "param_cfg_to_sec.h"
#include "nv_verify.h"

#define THIS_MODU mod_nv

extern struct nv_path_info_stru g_nv_path;

u32 s_delet_ret = NV_OK;

void nv_img_clear_check_result(void)
{
    s_delet_ret = NV_OK;

    return;
}

/************************************************************************
 函 数 名  : nv_img_get_white_list_num
 功能描述  : 获取白名单文件个数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 白名单文件个数
*************************************************************************/
u32 nv_img_get_white_list_num(void)
{
    return NV_MAX;
}

/************************************************************************
 函 数 名  : nv_img_is_white_file
 功能描述  : 判定某个文件是否是白名单上的文件
 输入参数  : pdir 白名单文件跟目录
 输出参数  : 无
 返 回 值  : NV_OK
             NV_ERROR
*************************************************************************/
bool nv_img_is_white_file(const s8 * pfile)
{
    u32 index;
    u32 file_num;

    file_num = nv_img_get_white_list_num();

    for(index = 0; index < file_num; index++)
    {
        if(0 == strncmp((char *)pfile, (char *)g_nv_path.file_path[index],
                                strlen((char *)g_nv_path.file_path[index]) + 1))
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*****************************************************************************
 函 数 名  : nv_img_check_white_list
 功能描述  : 删除不符合白名单的文件或者目录,出现的异常处理原则如下
             1. 如果申请内存不成功，返回错误，需要定位原因
             2. 如果出现文件目录无法访问(如打开，读取等操作)，返回BSP_ERR_NV_NO_FILE，
                调用者，依据特性处理 支持:擦除当前文件，并复位系统，重新挂载文件系统或者继续执行检查操作
             3. 删除无效文件失败，返回BSP_ERR_NV_NO_FILE，
                调用者，依据特性处理 支持:擦除当前文件，并复位系统，重新挂载文件系统或者继续执行检查操作
 输入参数  : source 白名单所在的跟目录
 输出参数  : 无
 返 回 值  : NV_OK
             NV_ERROR
             BSP_ERR_NV_NO_FILE 失败，需要调用者复位系统
*****************************************************************************/
u32 nv_img_check_white_list(const s8 * source, u32 depth)
{
    s32 ret;
    u32 chk_ret = NV_OK;
    s32 cur_dir = 0;
    s32 sub_index = 0;
    unsigned long src_len = 0;

    s8 * pdir_rent = NULL;
    s8 * pdir_name = NULL;

    struct rfile_stat_stru s_stat;
    RFILE_DIRENT_STRU * pdirent;


    /* 获取当前文件信息 */
    ret = bsp_stat((s8 *)source, &s_stat);
    if((ret < 0) || (nv_debug_chk_invalid_type((const s8 *) source, INVALID_FILE_NO_STAT) != 0))
    {
        nv_record("bsp_stat [%s] failed.\n",source);
        s_delet_ret = BSP_ERR_NV_NO_FILE;
        return NV_OK;
    }

    /*如果是目录，递归检查目录下的文件 */
    if(S_ISDIR(s_stat.mode))
    {
        /* 打开当前文件目录 */
        cur_dir = bsp_opendir((s8*)source);
        if((cur_dir < 0) || (nv_debug_chk_invalid_type((const s8 *) source, INVALID_FILE_NO_ODIR) != 0))
        {
            nv_record("bsp_opendir [%s] failed.\n",source);
            s_delet_ret = BSP_ERR_NV_NO_FILE;
            return NV_OK;
        }

        /* 缓存子目录的名称 */
        pdir_rent = vmalloc((unsigned long)MAX_DIRENT_LEN);
        if(NULL == pdir_rent)
        {
            nv_printf("<%s> malloc failed.\n", __FUNCTION__);
            bsp_closedir(cur_dir);
            return NV_ERROR;
        }

        /* 读取当前目录下的文件 */
        ret = bsp_readdir((unsigned int)cur_dir, pdir_rent, MAX_DIRENT_LEN);
        if((ret <= 0) || (nv_debug_chk_invalid_type((const s8 *) source, INVALID_FILE_NO_RDIR) != 0))
        {
            nv_record("bsp_readdir [%s] failed.\n",source);
            vfree(pdir_rent);
            bsp_closedir(cur_dir);
            s_delet_ret = BSP_ERR_NV_NO_FILE;
            return NV_OK;
        }

        /* 遍历处理当前目录下所有的文件，包括子目录，如果是空目录删除 */
        for(sub_index = 0; sub_index < ret; )
        {
            /*lint -save -e826 -specific(-e826)*/
            pdirent = (RFILE_DIRENT_STRU*)(pdir_rent + sub_index);
            /*lint -restore*/
            sub_index += pdirent->d_reclen;

            if((0 == strncmp((char*)pdirent->d_name, ".", sizeof(".")))
                    || (0 == strncmp((char*)pdirent->d_name, "..", sizeof(".."))))
            {
                continue;
            }

            /* 分配存储子文件/目录路径的缓存区 */
            src_len = (unsigned long)(strlen((char*)source)+1+strlen((char*)pdirent->d_name)+1);

            pdir_name = vmalloc(src_len);
            if(NULL == pdir_name)
            {
                nv_printf("<%s> malloc src failed.\n", __FUNCTION__);
                vfree(pdir_rent);
                bsp_closedir(cur_dir);
                return NV_ERROR;
            }

            /* 存储文件/目录名 */
            (void)memset_s((void*)pdir_name, src_len, 0, src_len);
            strncpy_s(pdir_name, src_len, source, src_len);
            strncat_s(pdir_name, src_len, "/", src_len);
            strncat_s(pdir_name, src_len, (char*)pdirent->d_name, src_len);

            // cppcheck-suppress *
            chk_ret = nv_img_check_white_list(pdir_name, depth+1);
            if(NV_ERROR == chk_ret)
            {
                 vfree(pdir_name);
                 break;
            }
            vfree(pdir_name);
        }

        /* 释放分配内存,关闭当前文件目录，删除空目录 */
        vfree(pdir_rent);
        bsp_closedir(cur_dir);

        if(depth){
            bsp_rmdir((s8*)source);
        }

    }
    /* 如果是源文件，做对应的处理 */
    else
    {
        /* 如果不是白名单上的文件则删除 */
        if(TRUE != nv_img_is_white_file((const s8 *)source))
        {
            ret = bsp_remove((const s8 *)source);
            if((ret != 0) || (nv_debug_chk_invalid_type((const s8 *) source, INVALID_FILE_NO_REMOVE) != 0))
            {
                nv_record("bsp_remove [%s] failed.\n",source);
                s_delet_ret = BSP_ERR_NV_NO_FILE;
                return NV_OK;
            }
        }
    }

    return chk_ret;
}

/**************************************************************************
 函 数 名  : nv_img_boot_check
 功能描述  : 删除不符合白名单的文件或者目录，外部调用
 输入参数  : pdir 白名单文件跟目录
 输出参数  : 无
 返 回 值  : NV_OK
             NV_ERROR
***************************************************************************/
u32 nv_img_boot_check(const s8 * pdir)
{
    u32 ret;
    u32 index;
    u32 file_num;

    file_num = nv_img_get_white_list_num();

    /* 检查文件路径是否超过长度限制 */
    for(index = 0; index < file_num; index++)
    {
        if(strlen((char*)g_nv_path.file_path[index]) > RFILE_NAME_MAX)
        {
            nv_record("<%s> file len error.\n", __FUNCTION__);
            return NV_ERROR;
        }
    }

    /* 依据白名单对处理文件，对异常结果做处理 */
    ret = nv_img_check_white_list(pdir, 0);
    if(ret)
    {
        nv_record("<%s> nv boot check failed.\n", __FUNCTION__);
        return NV_ERROR;
    }

    /* 异常处理复位特性，目前手机不复位, MBB复位 */

    return OK;
}

u32 nv_img_set_flag(u32 flag)
{
    u32 ret;
    FILE* fp = NULL;

    fp = nv_file_open((s8*)g_nv_path.file_path[NV_IMG_FLAG],(s8*)NV_FILE_WRITE);
    if(NULL == fp)
    {
        nv_printf("create nv file failed!\n");
        return BSP_ERR_NV_READ_FILE_FAIL;
    }

    ret = (u32)nv_file_write((u8 *)&flag,1,sizeof(u32),fp);
    nv_file_close(fp);
    if(ret != sizeof(u32))
    {
        nv_printf("write nv file failed!\n");
        return BSP_ERR_NV_WRITE_FILE_FAIL;
    }

    return NV_OK;
}

u32 nv_img_chk_flag(void)
{
    u32 ret;
    u32 flag;
    FILE* fp = NULL;

    ret = (u32)bsp_access(g_nv_path.file_path[NV_IMG_FLAG], 0);
    if(ret)
    {
        nv_printf("%s not exist, not a error\n", g_nv_path.file_path[NV_IMG_FLAG]);
        return BSP_ERR_NV_NO_FILE;
    }

    fp = nv_file_open((s8*)g_nv_path.file_path[NV_IMG_FLAG],(s8*)NV_FILE_READ);
    if(NULL == fp)
    {
        nv_printf("open nv file failed!\n");
        return BSP_ERR_NV_READ_FILE_FAIL;
    }

    ret = (u32)nv_file_read((u8 *)&flag,1,sizeof(u32),fp);
    nv_file_close(fp);
    if(ret != sizeof(u32))
    {
        nv_printf("read nv file failed!ret=0x%x\n",ret);
        return BSP_ERR_NV_WRITE_FILE_FAIL;
    }

    return flag;
}

/**************************************************************************
 函 数 名  : nv_img_write_file
 功能描述  : 写镜像到文件系统
 输入参数  : path 文件路径
             file_type 文件类型
 输出参数  : 无
 返 回 值  : NV_OK
             NV_ERROR
***************************************************************************/
u32 nv_img_write_file(u8* path, u32 file_type)
{
    u32 ret;
    u32 offset;
    u32 length;

    FILE* fp = NULL;
    nv_ctrl_info_s *ctrl_info = (nv_ctrl_info_s*)NV_GLOBAL_CTRL_INFO_ADDR;
    nv_file_info_s* file_info = (nv_file_info_s*)(NV_GLOBAL_CTRL_INFO_ADDR + NV_GLOBAL_CTRL_INFO_SIZE);

    switch(file_type)
    {
        case NV_FILE_HEAH:
            offset = 0;
            length = ctrl_info->ctrl_size;
            break;
        case NV_FILE_ATTRIBUTE_RESUM:
        case NV_FILE_ATTRIBUTE_RDONLY:
        case NV_FILE_ATTRIBUTE_RDWR:
            offset = file_info[file_type - 1].file_offset;
            length = file_info[file_type - 1].file_size;
            break;
        default:
            return BSP_ERR_NV_NO_FILE;
    }

    fp = nv_file_open((s8*)path,(s8*)NV_FILE_WRITE);
    if(NULL == fp)
    {
        nv_printf("create nv file failed!\n");
        return BSP_ERR_NV_READ_FILE_FAIL;
    }

    ret = (u32)nv_file_write((u8*)(NV_GLOBAL_CTRL_INFO_ADDR + offset),1,length,fp);
    nv_file_close(fp);
    if(ret != length)
    {
        nv_printf("write nv file failed!\n");
        return BSP_ERR_NV_WRITE_FILE_FAIL;
    }

    return NV_OK;
}

/*****************************************************************************
 函 数 名  : nv_bak_is_need_flush
 功能描述  : 是否刷新备份区数据
 输入参数  : void
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
bool nv_img_is_need_flush(void)
{
    nv_global_info_s* global_info = (nv_global_info_s*)NV_GLOBAL_INFO_ADDR;

    if(global_info->file_sign & (1 << NV_FILE_ATTRIBUTE_RDWR))
    {
        return true;
    }

    return false;
}


/*****************************************************************************
 函 数 名  : nv_img_flush
 功能描述  : 刷新NV数据至工作分区
             作用范围:1.升级NV启动刷新2.C核发起的同步操作
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
u32 nv_img_flush(void)
{
    u32 ret;

    if(nv_img_chk_flag() == NVM_IMG_BIN)
    {
        ret = nv_img_write_file((u8 *)g_nv_path.file_path[NV_IMG_RDWR1], NV_FILE_ATTRIBUTE_RDWR);
        if(ret)
        {
            nv_record("create %s file fail!\n", (u8 *) g_nv_path.file_path[NV_IMG_RDWR1]);
            return ret;
        }

        ret = nv_img_set_flag(NVM_IMG_BAK);
        if(ret)
        {
            nv_record("create flag(%d) fail!\n", NVM_IMG_BAK);
            return ret;
        }
    }
    else
    {
        ret = nv_img_write_file((u8 *)g_nv_path.file_path[NV_IMG_RDWR], NV_FILE_ATTRIBUTE_RDWR);
        if(ret)
        {
            nv_record("create %s file fail!\n", (u8 *) g_nv_path.file_path[NV_IMG_RDWR]);
            return ret;
        }

        ret = nv_img_set_flag(NVM_IMG_BIN);
        if(ret)
        {
            nv_record("create flag(%d) fail!\n", NVM_IMG_BIN);
            return ret;
        }
    }

    return NV_OK;
}

/*****************************************************************************
 函 数 名  : nv_img_load_carrier
 功能描述  : 加载随卡恢复/匹配镜像
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
u32 nv_img_load_carrier(s8 * path)
{
    u32 ret;
    u32 file_len;
    FILE * fp;

    enum SVC_SECBOOT_IMG_TYPE type;
    if(!strncmp(path, g_nv_path.file_path[NV_MBN_COMM], strlen(g_nv_path.file_path[NV_MBN_COMM])))
    {
        type = MBN_R;
    }
    else if(!strncmp(path, g_nv_path.file_path[NV_MBN], strlen(g_nv_path.file_path[NV_MBN])))
    {
        type = MBN_A;
    }else
    {
        nv_record("not support img!\n");
        return BSP_ERR_NV_NOT_SUPPORT_ERR;
    }

    fp = nv_file_open((s8 *)path, (s8*)NV_FILE_READ);
    if(NULL == fp)
    {
        nv_record("open %s file failed!\n", (s8 *)path);
        return BSP_ERR_NV_NO_FILE;
    }

    file_len = nv_get_file_len(fp);
    (void)nv_file_close(fp);
    if(file_len > NV_MBN_NV_SIZE)
    {
        nv_record("file len over nv mem or failed!len=0x%x.\n", file_len);
        return BSP_ERR_NV_FILE_OVER_MEM_ERR;
    }

    nv_printf("nv first file len:0x%x\n", file_len);

    ret = nv_img_load_file(type, NV_MBN_NV_ADDR, file_len);
    if(ret)
    {
        nv_printf("sec os laod img fail!\n", path);
        return NV_ERROR;
    }

    nv_printf("reload %s file sucess!\n", path);
    return NV_OK;
}

/*****************************************************************************
 函 数 名  : nv_img_flush_carrier
 功能描述  : 刷新随卡恢复NV镜像(comm.bin)至工作分区
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
u32 nv_img_flush_carrier(void)
{

    u32 ret;
    u32 writeLen;
    FILE* fp;

    nv_ctrl_info_s *ctrl_info = (nv_ctrl_info_s*)NV_MBN_NV_ADDR;
    nv_file_info_s *file_info = (nv_file_info_s *)(NV_MBN_NV_ADDR + NV_GLOBAL_CTRL_INFO_SIZE);

    if(ctrl_info->magicnum != NV_CTRL_FILE_MAGIC_NUM)
    {
        nv_record("null carrier_comm.bin!if udp,ignore this!\n");
        return NV_OK;
    }

    if(ctrl_info->file_num > 1)
    {
        nv_record("file num overflow!\n");
        return NV_ERROR;
    }

    fp = nv_file_open((s8*)g_nv_path.file_path[NV_MBN_COMM],(s8*)NV_FILE_WRITE);
    if(NULL == fp)
    {
        nv_record("open or create comm.bin fail!\n");
        return BSP_ERR_NV_NO_FILE;
    }

    writeLen = ctrl_info->ctrl_size + file_info->file_size;
    ret = (u32)nv_file_write((u8*)NV_MBN_NV_ADDR,1, writeLen, fp);
    nv_file_close(fp);
    if(ret != writeLen)
    {
        nv_record("read carrier_comm.bin fail!ret=0x%x,writeLen=0x%x.\n", ret, writeLen);
        return BSP_ERR_NV_WRITE_FILE_FAIL;
    }

    nv_printf("flush carrier_comm.bin sucess!\n");
    return NV_OK;
}

/*****************************************************************************
 函 数 名  : nv_img_check_file
 功能描述  : 检查工作分区中的文件是否有效
 输入参数  : void
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
u32 nv_img_check_file(s8 * path)
{
    u32 ret;

    ret =  nv_verify_check_sec(path);
    if (ret)
    {
        (void)bsp_remove(path);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : nv_resume_item_from_img
 功能描述  : 检查工作分区中的主/备文件是否都有效
 输入参数  : void
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
u32 nv_img_check_rdwr(void)
{
    u32 flag;
    u32 ret;

    flag = nv_img_chk_flag();
    if (NVM_IMG_BAK == flag)
    {
        ret = nv_img_check_file(g_nv_path.file_path[NV_IMG_RDWR1]);

        if (ret)
        {
            ret = nv_img_check_file(g_nv_path.file_path[NV_IMG_RDWR]);
             if(ret)
             {
                nv_record("check nv rdwr bin file fail %x\n", ret);
                return ret;
             }
        }
        else
        {
            (void)bsp_rename((s8*)g_nv_path.file_path[NV_IMG_RDWR1], (s8*)g_nv_path.file_path[NV_IMG_RDWR]);
        }
        (void)nv_img_set_flag(NVM_IMG_BIN);
    }
    else
    {
        ret = nv_img_check_file(g_nv_path.file_path[NV_IMG_RDWR]);

        if (ret)
        {
            ret = nv_img_check_file(g_nv_path.file_path[NV_IMG_RDWR1]);
            if (ret)
            {
                nv_record("check nv rdwr bak file fail %x\n", ret);
                return ret;
            }
            else
            {
                (void)bsp_rename((s8*)g_nv_path.file_path[NV_IMG_RDWR1], (s8*)g_nv_path.file_path[NV_IMG_RDWR]);
            }
        }
        (void)nv_img_set_flag(NVM_IMG_BIN);
    }

    return NV_OK;
}

/*****************************************************************************
 函 数 名  : nv_img_remove_all_file
 功能描述  : 删除所有相关的文件
 输入参数  : void
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
void nv_img_remove_all_file(void)
{
    (void)bsp_remove((s8 *)g_nv_path.file_path[NV_IMG_RDWR]);
    (void)bsp_remove((s8 *)g_nv_path.file_path[NV_IMG_RDWR1]);
    (void)bsp_remove((s8 *)g_nv_path.file_path[NV_IMG_FLAG]);

    return;
}

/*****************************************************************************
 函 数 名  : nv_img_load_file
 功能描述  : 加载镜像
 输入参数  :
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
u32 nv_img_load_file(u32 type, unsigned long offset, u32 length)
{
    u32 runaddr = (u32)(unsigned long)SHD_DDR_V2P(offset);
    return (u32)bsp_load_modem_single_image((enum SVC_SECBOOT_IMG_TYPE)type, runaddr, length);
}

/*****************************************************************************
 函 数 名  : nv_img_reload
 功能描述  : 从工作分区加载NV镜像至内存分区中
 输入参数  : void
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
u32 nv_img_reload(void)
{
    u32 ret;
    u32 length;
    unsigned long offset;

    nv_file_info_s* file_info = (nv_file_info_s*)(NV_GLOBAL_CTRL_INFO_ADDR + NV_GLOBAL_CTRL_INFO_SIZE);

    nv_printf("reload nv file from modemnvm_img start!\n");

    ret = nv_img_check_rdwr();
    if (ret)
    {
        nv_record("reload check rdwr fail %x!\n", ret);
        return NV_ERROR;
    }

    offset = NV_GLOBAL_CTRL_INFO_ADDR + file_info[NV_FILE_ATTRIBUTE_RDWR - 1].file_offset;
    length = file_info[NV_FILE_ATTRIBUTE_RDWR - 1].file_size;

    ret = nv_img_load_file(NVM, offset, length);
    if(ret)
    {
        nv_record("sec_os reload nv file form img fail!\n");
        return NV_ERROR;
    }

    nv_printf("reload nv file from modemnvm_img sucess!\n");

    return NV_OK;
}

u32 nv_img_mreset_load(void)
{
    u32 ret;
    u32 length;
    u32 runaddr;
    unsigned long offset;

    nv_file_info_s* file_info = (nv_file_info_s*)(NV_GLOBAL_CTRL_INFO_ADDR + NV_GLOBAL_CTRL_INFO_SIZE);

    nv_printf("reload nv file from modemnvm_img start!\n");

    ret = nv_img_check_rdwr();
    if (ret)
    {
        nv_record("reload check rdwr fail %x!\n", ret);
        return NV_ERROR;
    }

    offset = NV_GLOBAL_CTRL_INFO_ADDR + file_info[NV_FILE_ATTRIBUTE_RDWR - 1].file_offset;
    length = file_info[NV_FILE_ATTRIBUTE_RDWR - 1].file_size;

    runaddr = (u32)(unsigned long)SHD_DDR_V2P(offset);
    ret = load_image(NVM, offset, length);
    if(ret)
    {
        nv_record("sec_os reload nv file form img fail!\n");
        return NV_ERROR;
    }

    nv_printf("reload nv file from modemnvm_img sucess!\n");

    return NV_OK;
}


EXPORT_SYMBOL(nv_img_flush);
EXPORT_SYMBOL(nv_img_boot_check);
EXPORT_SYMBOL(nv_img_clear_check_result);




