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

/*lint --e{537}*/
#include <product_config.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <securec.h>
#include <nv_stru_drv.h>
#include <mdrv_misc.h>
#include <mdrv_version.h>
#include <bsp_nvim.h>
#include <bsp_version.h>

#undef THIS_MODU
#define THIS_MODU mod_version

modem_ver_info_s g_ver_info;

/*
 * 以下接口为对上层提供的mdrv接口
 */
/*
 * 函 数 名  : mdrv_ver_get_info
 * 功能描述  : 返回modem版本信息，对上层提供
 */
const modem_ver_info_s *mdrv_ver_get_info(void)
{
    return &g_ver_info;
}

/*
 * 函 数 名  : mdrv_ver_init
 * 功能描述  : ver_info初始化
 */
void mdrv_ver_init(void)
{
    memset_s(&g_ver_info, sizeof(modem_ver_info_s), 0x0, sizeof(modem_ver_info_s));

    if (bsp_get_version_info() == NULL) {
        return;
    }

    g_ver_info.product_info.board_a_type = bsp_get_version_info()->board_type;
    g_ver_info.product_info.chip_type = V7R2_CHIP;
    g_ver_info.product_info.product_name = (unsigned char *)bsp_version_get_product_out_name();
    g_ver_info.product_info.product_name_len = VER_MAX_LENGTH;
    g_ver_info.product_info.chip_id = (unsigned short)bsp_get_version_info()->chip_type;

    g_ver_info.hw_ver_info.hw_full_ver = (unsigned char *)bsp_version_get_hardware();
    g_ver_info.hw_ver_info.hw_full_ver_len = VER_MAX_LENGTH;
    g_ver_info.hw_ver_info.hw_id_sub = bsp_version_hw_sub_id();
    g_ver_info.hw_ver_info.hw_index = (int)bsp_get_version_info()->product_id;
    g_ver_info.hw_ver_info.hw_in_name = (unsigned char *)bsp_version_get_product_inner_name();
    g_ver_info.hw_ver_info.hw_in_name_len = VER_MAX_LENGTH;
    g_ver_info.hw_ver_info.hw_name = (unsigned char *)bsp_version_get_product_out_name();
    g_ver_info.hw_ver_info.hw_name_len = VER_MAX_LENGTH;

    return;
}

/*lint -save -e64 -e437 -e233 -e713*/
/*
 * 函 数 名  : mdrv_misc_support_check
 * 功能描述  : 查询模块是否支持，对上层提供
 * 输入参数  : enModuleType: 需要查询的模块类型
 * 返回值    ：BSP_MODULE_SUPPORT或BSP_MODULE_UNSUPPORT
 */
bsp_module_support_e mdrv_misc_support_check (bsp_module_type_e module_type)
{
    u32 ret;
    bool support = BSP_MODULE_SUPPORT;

    DRV_MODULE_SUPPORT_STRU support_nv = {0};

    if (module_type >= BSP_MODULE_TYPE_BOTTOM) {
        return BSP_MODULE_UNSUPPORT;
    }

    ret = bsp_nvm_read(NV_ID_DRV_MODULE_SUPPORT, (u8*)&support_nv, (unsigned int)sizeof(DRV_MODULE_SUPPORT_STRU));/*lint !e26 !e119 */
    if (ret != NV_OK) {
        (void)memset_s(&support_nv, sizeof(DRV_MODULE_SUPPORT_STRU), 0xFF, sizeof(DRV_MODULE_SUPPORT_STRU));
    }
    switch (module_type) {
        case BSP_MODULE_TYPE_SD:
            support = support_nv.sdcard;
            break;

        case BSP_MODULE_TYPE_CHARGE:
            support = support_nv.charge;
            break;

        case BSP_MODULE_TYPE_WIFI:
            support = support_nv.wifi;
            break;

        case BSP_MODULE_TYPE_OLED:
            support = support_nv.oled;
            break;

        case BSP_MODULE_TYPE_HIFI:
            support = support_nv.hifi;
            break;

        case BSP_MODULE_TYPE_POWER_ON_OFF:
            support = support_nv.onoff;
            break;

        case BSP_MODULE_TYPE_HSIC:
            support = support_nv.hsic;
            break;

        case BSP_MODULE_TYPE_LOCALFLASH:
            support = support_nv.localflash;
            break;

        default:
            support = 0;
    }

    if (support)
        return BSP_MODULE_SUPPORT;
    else
        return BSP_MODULE_UNSUPPORT;
}
/*lint -restore +e64 +e437 +e233 +e713*/
EXPORT_SYMBOL_GPL(mdrv_misc_support_check);
EXPORT_SYMBOL_GPL(mdrv_ver_get_info);
EXPORT_SYMBOL_GPL(mdrv_ver_init);
