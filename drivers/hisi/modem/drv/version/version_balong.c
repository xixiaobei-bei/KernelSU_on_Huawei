/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
 */

/*lint --e{537}*/
/*lint -save -e958*/
#include <product_config.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/atomic.h>
#include <nv_stru_drv.h>
#include <linux/of.h>
#include <securec.h>
#include <mdrv_memory.h>
#include <bsp_nvim.h>
#include <bsp_version.h>
#include <bsp_sec_call.h>

#undef THIS_MODU
#define THIS_MODU mod_version
#define VER_SHM_VALID 0x1234abcd
#define VER_SHM_INVALID 0x4321dcba

u32 g_shm_init_flag = VER_SHM_INVALID;
void *g_ver_shm_addr = 0;
u32 g_is_verison_ddr_init;

PRODUCT_INFO_NV_STRU g_hardware_product_info = { 0 };
NV_SW_VER_STRU g_nv_sw_ver = { 0 };

u32 bsp_version_hw_sub_id(void)
{
    return g_hardware_product_info.hwIdSub;
}

/*
 * 功能描述: 读NV(0xD115)的值到huawei_product_info
 */
static int bsp_version_productinfo_init(void)
{ /*lint -save -e838*/
    u32 ret;
    u32 hw_ver;

    if (bsp_get_version_info() == NULL) {
        ver_print_error(" bsp_get_version_info be null!\n");
        return VER_ERROR;
    }
    hw_ver = bsp_get_version_info()->product_id;
    /*lint -restore*/
    /*lint -save -e26 -e119*/
    ret = bsp_nvm_read(NV_ID_DRV_NV_VERSION_INIT, (u8 *)&g_hardware_product_info, sizeof(PRODUCT_INFO_NV_STRU));
    if (ret != NV_OK) {
        ver_print_error(" bsp_nvm_read failed!\n");
        return VER_ERROR;
    }

    if (hw_ver != g_hardware_product_info.index) {
        ver_print_error(" product id(read from hkadc) is 0x%x!\n", hw_ver);
        g_hardware_product_info.index = hw_ver;
    }
    /*lint -restore*/
    return VER_OK;
}

/*
 * 功能描述：获取共享内存基地址和大小
 */
void bsp_version_get_shm(void)
{
    u32 shm_size = 0;
    phy_addr_t phy_addr = 0;
    const char *name = "nsroshm_version";

    g_ver_shm_addr = mdrv_mem_share_get(name, &phy_addr, &shm_size, SHM_NSRO);
    if (g_ver_shm_addr == NULL) {
        ver_print_error("bsp_version_shm_init fail: get shared mem err!\n");
        return;
    }
    g_shm_init_flag = VER_SHM_VALID;
    return;
}


/*
 * 功能描述: acore版本号初始化
 */
int bsp_version_acore_init(void)
{
    int ret;
    ret = memset_s((void *)(&g_hardware_product_info), sizeof(PRODUCT_INFO_NV_STRU), 0, sizeof(PRODUCT_INFO_NV_STRU));
    if (ret != EOK) {
        ver_print_error("memset huawei_product_info err\n");
    }

    ret = bsp_version_productinfo_init();
    if (ret == VER_ERROR)
        ver_print_error("bsp_version_productinfo_init fail! ");
    else
        ver_print_error("bsp version acore init OK!\n");

    mdrv_ver_init();

    return ret;
}

int bsp_version_base_init(void)
{
    g_is_verison_ddr_init = 1;
    return VER_OK;
}


/*
 * 以下接口提供给mdrv接口调用
 */

/*
 * 函 数: bsp_version_get_hardware
 * 功 能: 获取硬件版本号(硬件版本名称+ Ver.+硬件子版本号+A)
 */
char *bsp_version_get_hardware(void)
{
    u32 len;
    int ret;
    static bool b_geted = false;
    static char hardware_version[VERSION_MAX_LEN];
    char hardware_sub_ver;

    if (g_hardware_product_info.index == HW_VER_INVALID) {
        ver_print_error("g_hardware_product_info.index = HW_VER_INVALID\n");
        return NULL;
    }

    if (!b_geted) {
        /*lint -save -e18 -e718 -e746*/
        len = (unsigned int)(strlen(g_hardware_product_info.hwVer) + strlen(" Ver.X"));
        hardware_sub_ver = (char)g_hardware_product_info.hwIdSub + 'A';
        if (memset_s((void *)hardware_version, VERSION_MAX_LEN, 0, len)) {
            ver_print_error("hardware_version memset err\n");
        }
        /*lint -restore*/
        ret = strncat_s(hardware_version, VERSION_MAX_LEN, g_hardware_product_info.hwVer,
            strlen(g_hardware_product_info.hwVer));
        if (ret) {
            ver_print_error("strncat hardware version err\n");
            return NULL;
        }
        ret = strncat_s(hardware_version, VERSION_MAX_LEN, " Ver.", strlen(" Ver."));
        if (ret) {
            ver_print_error("strncat hardware version err\n");
            return NULL;
        }
        if (len < VERSION_MAX_LEN) {
            *((hardware_version + len) - 1) = hardware_sub_ver;
            *(hardware_version + len) = 0;
        } else {
            return NULL;
        }
        b_geted = true;
    }

    return (char *)hardware_version;
}

/*
 * 函 数: bsp_get_product_inner_name
 * 功 能: 获取内部产品名称(内部产品名+ 内部产品名plus)
 */
char *bsp_version_get_product_inner_name(void)
{
    unsigned int len;
    int ret;
    static bool b_geted = false;
    static char product_inner_name[VERSION_MAX_LEN];

    if (g_hardware_product_info.index == HW_VER_INVALID) {
        ver_print_error("g_hardware_product_info.index = HW_VER_INVALID\n");
        return NULL;
    }

    if (!b_geted) {
        len = (unsigned int)(strlen(g_hardware_product_info.name) + strlen(g_hardware_product_info.namePlus));
        ret = memset_s((void *)product_inner_name, VERSION_MAX_LEN, 0, len);
        if (ret) {
            ver_print_error("product_inner_name memset err\n");
        }

        ret = strncat_s(product_inner_name, VERSION_MAX_LEN, g_hardware_product_info.name,
            strlen(g_hardware_product_info.name));
        if (ret) {
            ver_print_error("strncat product_inner_name err\n");
            return NULL;
        }
        ret = strncat_s(product_inner_name, VERSION_MAX_LEN, g_hardware_product_info.namePlus,
            strlen(g_hardware_product_info.namePlus));
        if (ret) {
            ver_print_error("strncat product_inner_name err\n");
            return NULL;
        }
        b_geted = true;
    }

    return (char *)product_inner_name;
}

/*
 * 函 数: bsp_get_product_out_name
 * 功 能: 获取外部产品名称
 */
char *bsp_version_get_product_out_name(void)
{
    if (g_hardware_product_info.index == HW_VER_INVALID) {
        ver_print_error("g_hardware_product_info.index = HW_VER_INVALID\n");
        return NULL;
    }

    return (char *)g_hardware_product_info.productId;
}

/*
 * 以下接口提供给dump模块调用
 */
/*
 * 函 数: bsp_get_build_date_time
 * 功 能: 获取编译日期和时间
 */
char *bsp_version_get_build_date_time(void)
{
    return NULL;
}

/*
 * 函 数: bsp_get_chip_version
 * 功 能: 获取芯片版本号
 */
char *bsp_version_get_chip(void)
{
    return (char *)PRODUCT_CFG_CHIP_SOLUTION_NAME;
}

/*
 * 函 数: bsp_get_firmware_version
 * 功 能: 获取软件版本号
 */
char *bsp_version_get_firmware(void)
{
    u32 ret;
    /*lint -save -e26 -e119*/
    ret = bsp_nvm_read(NV_ID_DRV_NV_DRV_VERSION_REPLACE_I, (u8 *)&g_nv_sw_ver, sizeof(NV_SW_VER_STRU));
    if (ret != NV_OK) {
        ver_print_error("get NV_SW_VERSION_REPLACE failed!\n");
        return (char *)PRODUCT_DLOAD_SOFTWARE_VER;
    } else if (g_nv_sw_ver.nvStatus == 0) {
        return (char *)PRODUCT_DLOAD_SOFTWARE_VER;
    }
    /*lint -restore*/
    return (char *)g_nv_sw_ver.nv_version_info;
}

/*
 * 函 数: bsp_version_get_release
 * 功 能: 获取全版本号
 */
char *bsp_version_get_release(void)
{
    return PRODUCT_FULL_VERSION_STR;
}

/*
 * 功能描述: 为其他模块提供各种版本号信息
 * 返回值: version的共享内存数据结构体的地址
 */
const bsp_version_info_s *bsp_get_version_info(void)
{
    bsp_version_info_s *p_version_info = NULL;
    int ret;
    if (g_shm_init_flag == VER_SHM_VALID && g_ver_shm_addr != NULL) {
        p_version_info = (bsp_version_info_s *)g_ver_shm_addr;
    } else {
        bsp_version_get_shm();
        if (g_ver_shm_addr == NULL) {
            ver_print_error("version shm addr is NULL\n");
            return NULL;
        }
        p_version_info = (bsp_version_info_s *)g_ver_shm_addr;
    }
    if (g_is_verison_ddr_init != 1) {
        ret = bsp_version_base_init();
        if (ret) {
            return NULL;
        }
    }
    if (p_version_info->version_magic == VERSION_MODULE_MAGIC) {
        return p_version_info;
    } else {
        ver_print_error("version magic error:0x%x(!=0x2017)\n", p_version_info->version_magic);
        return NULL;
    }
}

/*
 * 函 数: bsp_version_debug
 * 功 能: 用于调试查看版本号相关信息
 */
/*lint -save -e613*/
int bsp_version_debug(void)
{
    if (g_hardware_product_info.index == HW_VER_INVALID) {
        ver_print_error("g_hardware_product_info.index = HW_VER_INVALID\n");
        return VER_ERROR;
    }

    ver_print_error("\n\n1 . the element value of nv(0xd115):\n");
    ver_print_error("Hardware index :0x%x\n", g_hardware_product_info.index);
    ver_print_error("hw_Sub_ver     :0x%x\n", g_hardware_product_info.hwIdSub);
    ver_print_error("Inner name     :%s\n", g_hardware_product_info.name);
    ver_print_error("name plus      :%s\n", g_hardware_product_info.namePlus);
    ver_print_error("HardWare ver   :%s\n", g_hardware_product_info.hwVer);
    ver_print_error("DLOAD_ID       :%s\n", g_hardware_product_info.dloadId);
    ver_print_error("Out name       :%s\n", g_hardware_product_info.productId);

    ver_print_error("\n\n3 . get from func(bsp_version_get_xxx):\n");
    ver_print_error("HardWare ver   :%s\n", bsp_version_get_hardware());
    ver_print_error("Inner name     :%s\n", bsp_version_get_product_inner_name());
    ver_print_error("Out name       :%s\n", bsp_version_get_product_out_name());
    ver_print_error("Build_time     :%s\n", bsp_version_get_build_date_time());
    ver_print_error("Chip_ver       :%s\n", bsp_version_get_chip());
    ver_print_error("Firmware       :%s\n", bsp_version_get_firmware());
    ver_print_error("Release_ver    :%s\n", bsp_version_get_release());

    if (bsp_get_version_info() == NULL) {
        ver_print_error("bsp_get_version_info == NULL\n");
        return VER_ERROR;
    }

    ver_print_error("\n\n4 . get from bsp_get_version_info:\n");
    ver_print_error("product_id            :0x%x\n", bsp_get_version_info()->product_id);
    ver_print_error("product_id_udp_masked :0x%x\n", bsp_get_version_info()->product_id_udp_masked);
    ver_print_error("chip_version        :0x%x\n", bsp_get_version_info()->chip_version);
    ver_print_error("chip_type           :0x%x\n", bsp_get_version_info()->chip_type);
    ver_print_error("plat_type           :0x%x  (0:asic 1:plat_esl 2:hybrid a:porting e:emu)\n",
        bsp_get_version_info()->plat_type);
    ver_print_error("plat_info           :0x%x(0:asic 1:esl_vdk 2:esl_cand 3:esl_self 4:emu_zebu  \
        5:emu_z1 6:hybrid_vdk&zebu 7:hybrid_cand&z1)\n",
        bsp_get_version_info()->plat_info);
    ver_print_error("base_board_id        :0x%x \n", bsp_get_version_info()->base_board_id);
    ver_print_error("board_type(for drv) :0x%x  (0:bbit 1:sft 2:asic 3:soc 4:porting)\n",
        bsp_get_version_info()->board_type);
    ver_print_error("board_type(for mdrv):0x%x  (0:bbit 1:sft 2:asic)\n",
        (board_actual_type_e)bsp_get_version_info()->board_type);
    ver_print_error("product_type        :0x%x  (0:mbb 1:phone)\n", bsp_get_version_info()->product_type);
    ver_print_error("cses_type           :0x%x(1:es 2:cs)\n", bsp_get_version_info()->cses_type);
    ver_print_error("version_magic       :0x%x\n", bsp_get_version_info()->version_magic);

    return VER_OK;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
/*lint -restore*/
/* 注意:需在nv模块初始化之后 */
module_init(bsp_version_acore_init);
#endif
EXPORT_SYMBOL_GPL(g_hardware_product_info);
EXPORT_SYMBOL_GPL(bsp_version_acore_init);
EXPORT_SYMBOL_GPL(bsp_version_get_hardware);
EXPORT_SYMBOL_GPL(bsp_version_get_product_inner_name);
EXPORT_SYMBOL_GPL(bsp_version_get_product_out_name);
EXPORT_SYMBOL_GPL(bsp_get_version_info);
EXPORT_SYMBOL_GPL(bsp_version_debug);

