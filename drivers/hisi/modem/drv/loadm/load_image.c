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

#include "product_config.h"

#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <asm/dma-mapping.h>
#include <asm/cacheflush.h>
#include <soc_onchiprom.h>
#include <bsp_shared_ddr.h>
#include <bsp_reset.h>
#include <bsp_sec.h>
#include <bsp_rfile.h>
#include <bsp_version.h>
#include <bsp_ddr.h>
#include <bsp_efuse.h>
#include "load_image.h"
#include "modem_dtb.h"

#if (LPHY_TOTAL_IMG_SIZE > DDR_TLPHY_IMAGE_SIZE)
#error Dsp image over ddr_tlphy_image_size
#endif

/* Dalls之后手机和MBB融合代码 */

/* Austin平台老代码 */
#include <../../adrv/adrv.h>


/*****************************************************************************
 函 数 名  : 手机平台上A核通过安全OS加载dsp镜像,校验成功返回OK,校验失败返回ERROR
 功能描述  : 手机平台上A核通过安全OS加载dsp镜像,校验成功返回OK,校验失败返回ERROR
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 成功返回OK,失败返回ERROR
 调用函数  :
 被调函数  :
*****************************************************************************/
int his_load_tldsp_image(void)
{
	int ret = 0;
	u64 jiffies_old = 0,jiffies_new=0;
	u64 jiffies_time=0;
	BSP_CORE_TYPE_E ecoretype = BSP_BBE;
	jiffies_old = get_jiffies_64();
	ret = bsp_reset_loadimage(ecoretype);
	jiffies_new = get_jiffies_64();
	jiffies_time =jiffies_new -jiffies_old;
	if(ret)
	{
		printk(KERN_ERR ">>load his_load_tldsp_image error, ret 0x%x\n", ret);
	}
	else
	{
		printk(KERN_ERR ">>load his_load_tldsp_image success, jiffies_time 0x%llx\n", jiffies_time);
	}
	return ret;
}


/*****************************************************************************
 函 数 名  : 手机平台上A核通过安全OS加载modem镜像,校验成功解复位modem,校验失败返回ERROR
 功能描述  : 手机平台上A核通过安全OS首先把modem镜像加载到内存中,再来校验镜像,校验成功解复位modem,校验失败返回ERROR
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 成功返回0,失败返回非0
 调用函数  :
 被调函数  :
*****************************************************************************/

static int his_load_modem_image(void)
{
	int ret = 0;
	u64 jiffies_old = 0,jiffies_new=0;
	u64 jiffies_time=0;
	BSP_CORE_TYPE_E ecoretype = BSP_CCORE;
	jiffies_old = get_jiffies_64();
	ret = bsp_reset_loadimage(ecoretype);
	jiffies_new = get_jiffies_64();
	jiffies_time =jiffies_new -jiffies_old;
	if(ret)
	{
		printk(KERN_ERR ">>load his_load_modem_image error, ret 0x%x\n", ret);
	}
	else
	{
		printk(KERN_ERR ">>load his_load_modem_image success, jiffies_time 0x%llx\n", jiffies_time);
	}
	return ret;
}

/*****************************************************************************
 函 数 名  : 给modem单独复位提供镜像加载接口
 功能描述  : 给modem单独复位提供镜像加载接口
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 成功返回OK,失败返回ERROR
*****************************************************************************/
int bsp_load_modem_images(void)
{
	int ret = 0;

	ret = his_load_tldsp_image();
	if(ret)
	{
		return -1;
	}

	ret = his_load_modem_image();
	if(ret)
	{
		return -1;
	}
	return ret;
}


