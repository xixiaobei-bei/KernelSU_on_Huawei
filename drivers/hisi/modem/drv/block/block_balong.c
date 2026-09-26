/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#include <product_config.h>
#include <linux/mtd/mtd.h>
#include <linux/err.h>
#include <linux/kernel.h>

#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/statfs.h>

#include <securec.h>
#include <bsp_print.h>
#include <bsp_blk.h>
#include <bsp_version.h>

#define THIS_MODU mod_blk


struct block_ops *g_block_ops;
block_support_e g_block_is_support = BLOCK_BALONG_NOT_SUPPORT;

/* *****************************block加载模式判断************************************************** */
block_support_e bsp_blk_is_support(void)
{
#ifndef BSP_CONFIG_PHONE_TYPE
    const bsp_version_info_s *version_info = bsp_get_version_info();

    if (version_info != NULL && version_info->plat_type == PLAT_ASIC) {
        return BLOCK_BALONG_IS_SUPPORT;
    }

    return g_block_is_support;
#endif
    return BLOCK_BALONG_IS_SUPPORT;
}

__init int bsp_blk_parse_boot_para(char *cmdline)
{
    if (cmdline == NULL) {
        bsp_err("cmdline is null.\n");
        return -EINVAL;
    }
    bsp_err("bsp_blk_load_mode :%s\n", cmdline);

    if (strcmp(cmdline, "flash") == 0) {
        g_block_is_support = BLOCK_BALONG_IS_SUPPORT;
    }

    return 0;
}

#ifndef BSP_CONFIG_PHONE_TYPE
early_param("bsp_blk_load_mode", bsp_blk_parse_boot_para);
#endif

int bsp_blk_register_ops(struct block_ops* ops)
{
    if (g_block_ops != NULL) {
        bsp_err("%s ops is registered again\n", __func__);
        return -1;
    }
    g_block_ops = ops;
    return 0;
}

/*
 * 功能: 读一个分区指定偏移的数据
 * 输入: part_name 分区名字
 * 输入: part_offset 分区偏移
 * 输入: data_len  要读的大小
 * 输出: data_buf  存放读出的数据
 * 返回: 小于0失败，大于等于0成功
 */
int bsp_blk_read(const char *partition_name, loff_t partition_offset, void *data_addr, size_t data_len)
{
    if (g_block_ops == NULL) {
        return 0;
    }
    if (g_block_ops->read != NULL) {
        return g_block_ops->read(partition_name, partition_offset, data_addr, data_len);
    } else {
        bsp_info("%s not support\n", __func__);
        return 0;
    }
}

/*
 * 功能: 对一个分区指定偏移写数据
 * 输入: part_name 分区名字
 * 输入: part_offset 分区偏移
 * 输入: data_len  要写的大小
 * 输出: data_buf  存放要写的数据
 * 返回: 小于0失败，大于等于0成功
 */
int bsp_blk_write(const char *partition_name, loff_t partition_offset, const void *data_addr, size_t data_len)
{
    if (g_block_ops == NULL) {
        return 0;
    }
    if (g_block_ops->write != NULL) {
        return g_block_ops->write(partition_name, partition_offset, data_addr, data_len);
    } else {
        bsp_info("%s not support\n", __func__);
        return 0;
    }
}

/*
 * 函数  : bsp_blk_erase
 * 功能  : 擦除指定分区的数据
 * 输入  : 分区名
 * 输出  : 无
 * 返回  : 0成功，非0失败
 */
int bsp_blk_erase(const char *partition_name)
{
    if (g_block_ops == NULL) {
        return 0;
    }
    if (g_block_ops->erase != NULL) {
        return g_block_ops->erase(partition_name);
    } else {
        bsp_info("%s not support\n", __func__);
        return 0;
    }
}

/*
 * 功能: 查询是否为坏块
 * 输入: 分区名，分区内偏移
 * 输出: 无
 * 返回: 0好块，非0坏块
 */
int bsp_blk_isbad(const char *partition_name, loff_t partition_offset)
{
    if (g_block_ops == NULL) {
        return 0;
    }
    if (g_block_ops->isbad != NULL) {
        return g_block_ops->isbad(partition_name, partition_offset);
    } else {
        bsp_info("%s not support\n", __func__);
        return 0;
    }
}

/*
 * 功能: 获取一个分区的大小
 * 输入: part_name 分区名字
 * 输出: size  分区的大小值存放在size
 * 返回: 小于0失败，大于等于0成功
 */
int bsp_blk_size(const char *part_name, u32 *size)
{
    if (g_block_ops == NULL) {
        return 0;
    }
    if (g_block_ops->get_part_size != NULL) {
        return g_block_ops->get_part_size(part_name, size);
    } else {
        bsp_info("%s not support\n", __func__);
        return 0;
    }
}

/*
 * 功能: 获取包含cdromiso 分区的mtd块号的完整路径
 * 输入: 无
 * 输出: 完整路径名存放到blk_path中
 * 返回: 0成功，非0失败
 */
int bsp_blk_get_cdromiso_blkname(char *blk_path, int len)
{
    if (g_block_ops == NULL) {
        return 0;
    }
    if (g_block_ops->get_cdromiso_blkname != NULL) {
        return g_block_ops->get_cdromiso_blkname(blk_path, len);
    } else {
        bsp_info("%s not support\n", __func__);
        return 0;
    };
}

#ifndef BSP_CONFIG_PHONE_TYPE
int flash_find_ptn_s(const char *part_name, char *blk_path, unsigned int blk_path_len)
{
    if (g_block_ops == NULL) {
        return 0;
    }
    if (g_block_ops->find_part != NULL) {
        return g_block_ops->find_part(part_name, blk_path, blk_path_len);
    } else {
        bsp_info("%s not support\n", __func__);
        return 0;
    };
}
#endif

EXPORT_SYMBOL_GPL(bsp_blk_get_cdromiso_blkname);
EXPORT_SYMBOL_GPL(bsp_blk_isbad);
EXPORT_SYMBOL_GPL(bsp_blk_read);
EXPORT_SYMBOL_GPL(bsp_blk_write);
EXPORT_SYMBOL_GPL(bsp_blk_size);
EXPORT_SYMBOL_GPL(bsp_blk_erase);
