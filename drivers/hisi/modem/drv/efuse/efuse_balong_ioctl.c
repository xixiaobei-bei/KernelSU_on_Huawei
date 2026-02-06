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
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/mm.h>

#include <product_config.h>
#include <hi_efuse.h>
#include <bsp_sec.h>
#include <bsp_efuse.h>
#include <bsp_print.h>

#include "securec.h"
#include "bsp_shared_ddr.h"

#include <mdrv_efuse.h>

#define THIS_MODU mod_efuse

#ifdef HI_K3_EFUSE
#include <mdrv.h>
#include "../../adrv/adrv.h"
#endif

#include <soc_onchiprom.h>

#ifdef CONFIG_EFUSE


int mdrv_efuse_get_dieid(unsigned char* buf,int length)
{
#ifdef HI_K3_EFUSE
    return get_efuse_dieid_value(buf, (unsigned int)length, 0);
#else
    u32 i = 0;
    int ret = 0;

#if (EFUSE_DIEID_BIT < 32)
    u32 *buf_die_id = (u32*)buf;
#endif

    if (NULL == buf) {
        efuse_print_error("die id buf is error.\n");
        return EFUSE_ERROR_ARGS;
    }

    if (length < EFUSE_DIEID_LEN) {
        efuse_print_error("die id lenth is error.\n");
        return EFUSE_ERROR_ARGS;
    }

    for (i = 0; i < EFUSE_DIEID_LEN; i++) {
        buf[i] = 0;
    }

    efuse_print_info("efuse read start group %d length %d.\n", EFUSE_GRP_DIEID, EFUSE_DIEID_SIZE);

    ret = bsp_efuse_read((u32*)buf, EFUSE_GRP_DIEID, EFUSE_DIEID_SIZE);
    if (EFUSE_OK != ret) {
        efuse_print_error("die id read efuse error.\n");
        return ret;
    }

    for(i = 0; i < EFUSE_DIEID_SIZE; i++) {
        efuse_print_info("efuse end buf[%d] is 0x%x.\n", i, buf[i]);
    }
    efuse_print_info("efuse read end ret %d\n", ret);

#if (EFUSE_DIEID_BIT < 32)
    buf_die_id[EFUSE_DIEID_SIZE-1] &= (((u32)0x1 << EFUSE_DIEID_BIT) - 1);
#endif

    return EFUSE_OK;
#endif
}
EXPORT_SYMBOL(mdrv_efuse_get_dieid);


#ifdef HI_K3_EFUSE
int mdrv_efuse_get_chipid(unsigned char* buf,int length)
{
    return get_efuse_chipid_value(buf, (unsigned int)length, 0);
}
int mdrv_efuse_ioctl(int cmd, int arg, unsigned char* buf, int len)
{
    efuse_print_error("It's only a stub");

    return EFUSE_OK;
}
#else
int mdrv_efuse_get_chipid(unsigned char* buf,int length)
{
    return EFUSE_OK;
}

#ifdef DX_SEC_BOOT

int bsp_efuse_ioctl_get_socid(unsigned char* buf, int len)
{
    u32 i;
    unsigned char *src = (unsigned char *)(SHM_BASE_ADDR+SHM_OFFSET_SEC_SOC_ID);

    if ((!buf) || (len < (SHM_SIZE_SEC_SOC_ID/4))) {
        efuse_print_error("bsp_efuse_ioctl_get_socid fail. error args.\n");
        return EFUSE_ERROR_ARGS;
    }

    for ( i = 0; i < SHM_SIZE_SEC_SOC_ID; i++) {
        buf[i] = src[i];
    }

    return EFUSE_OK;
}

int bsp_efuse_ioctl_set_kce(unsigned char* buf, int len)
{
    int ret = 0;

    if ((!buf) || (len != EFUSE_LAYOUT_KCE_LENGTH)) {
        efuse_print_error("bsp_efuse_ioctl_set_kce fail. error args.\n");
        return EFUSE_ERROR_ARGS;
    }

    ret = bsp_efuse_write((u32*)buf, EFUSE_LAYOUT_KCE_OFFSET, EFUSE_LAYOUT_KCE_LENGTH);

    return ret;
}


#define DX_SECURESTATE_SECURE   (1)
#define DX_SECURESTATE_RMA      (2)

int bsp_efuse_ioctl_get_securestate(int arg __attribute__((unused)))
{
    int ret = 0;
    u32 value[2] = {0, 0};

    ret = bsp_efuse_read(&value[0], EFUSE_LAYOUT_MP_FLAG_OFFSET, 2);
    if (ret)
        return ret;

    if (value[0] & (0x1U << EFUSE_LAYOUT_MP_FLAG_RMA_BIT_OFFSET))
        return DX_SECURESTATE_RMA;

    if (((value[0] >> EFUSE_LAYOUT_MP_FLAG_LCS_BIT_OFFSET) & 0x0F) == 0x03) {
        if (value[1])
            return DX_SECURESTATE_SECURE;
        else
            return 0;
    }

    return 0;
}

int bsp_efuse_ioctl_set_securestate(int arg)
{
    int ret = 0;
    u32 value = 0;
    u32 boot_sel = 0;

    ret = bsp_efuse_ioctl_get_securestate(arg);
    if (ret < 0)
        return ret;

    if (ret > 0)
        return 1;

    if (arg == DX_SECURESTATE_SECURE) {
        efuse_print_error("bsp_efuse_ioctl_set_securestate fail. trustzone is not supported.\n");
        return EFUSE_ERROR_ARGS;
    }

    if (arg == DX_SECURESTATE_RMA)
        value |= (0x1U << EFUSE_LAYOUT_MP_FLAG_RMA_BIT_OFFSET);

    ret = bsp_efuse_write(&value, EFUSE_LAYOUT_MP_FLAG_OFFSET, 1);
    ret |= bsp_efuse_write(&boot_sel, GROUP_INDEX(EFUSE_LAYOUT_BOOT_SEL_BIT_OFFSET), 1);

    return ret;
}

int bsp_efuse_ioctl_get_securedebug(int arg __attribute__((unused)))
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_read(&value, GROUP_INDEX(EFUSE_LAYOUT_ARM_DBG_CTRL_BIT_OFFSET), 1);
    if (ret)
        return ret;

    ret = (int)((value >> BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_ARM_DBG_CTRL_BIT_OFFSET)) & 0x3U);
    return ret;
}

int bsp_efuse_ioctl_set_securedebug(int arg)
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_ioctl_get_securedebug(arg);
    if (ret < 0)
        return ret;

    if (ret > 0)
        return 1;

    value = (arg & 0x3) << BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_ARM_DBG_CTRL_BIT_OFFSET);

    ret = bsp_efuse_write(&value, GROUP_INDEX(EFUSE_LAYOUT_ARM_DBG_CTRL_BIT_OFFSET), 1);

    return ret;
}

int bsp_efuse_ioctl_get_dbgen(int arg __attribute__((unused)))
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_read(&value, GROUP_INDEX(EFUSE_LAYOUT_ARM_DBG_CTRL_DBGEN_BIT_OFFSET), 1);
    if (ret)
        return ret;

    ret = (int)((value >> BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_ARM_DBG_CTRL_DBGEN_BIT_OFFSET)) & 0x1U);
    return ret;
}

int bsp_efuse_ioctl_set_dbgen(int arg)
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_ioctl_get_dbgen(arg);
    if (ret < 0)
        return ret;

    if (ret > 0)
        return 1;

    value = (arg & 0x1) << BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_ARM_DBG_CTRL_DBGEN_BIT_OFFSET);

    ret = bsp_efuse_write(&value, GROUP_INDEX(EFUSE_LAYOUT_ARM_DBG_CTRL_DBGEN_BIT_OFFSET), 1);

    return ret;
}

int bsp_efuse_ioctl_get_niden(int arg __attribute__((unused)))
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_read(&value, GROUP_INDEX(EFUSE_LAYOUT_ARM_DBG_CTRL_NIDEN_BIT_OFFSET), 1);
    if (ret)
        return ret;

    ret = (int)((value >> BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_ARM_DBG_CTRL_NIDEN_BIT_OFFSET)) & 0x1U);
    return ret;
}

int bsp_efuse_ioctl_set_niden(int arg)
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_ioctl_get_niden(arg);
    if (ret < 0)
        return ret;

    if (ret > 0)
        return 1;

    value = (arg & 0x1) << BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_ARM_DBG_CTRL_NIDEN_BIT_OFFSET);

    ret = bsp_efuse_write(&value, GROUP_INDEX(EFUSE_LAYOUT_ARM_DBG_CTRL_NIDEN_BIT_OFFSET), 1);

    return ret;
}

int bsp_efuse_ioctl_get_spiden(int arg __attribute__((unused)))
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_read(&value, GROUP_INDEX(EFUSE_LAYOUT_ARM_DBG_CTRL_SPIDEN_BIT_OFFSET), 1);
    if (ret)
        return ret;

    ret = (int)((value >> BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_ARM_DBG_CTRL_SPIDEN_BIT_OFFSET)) & 0x1U);
    return ret;
}

int bsp_efuse_ioctl_set_spiden(int arg)
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_ioctl_get_spiden(arg);
    if (ret < 0)
        return ret;

    if (ret > 0)
        return 1;

    value = (arg & 0x1) << BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_ARM_DBG_CTRL_SPIDEN_BIT_OFFSET);

    ret = bsp_efuse_write(&value, GROUP_INDEX(EFUSE_LAYOUT_ARM_DBG_CTRL_SPIDEN_BIT_OFFSET), 1);

    return ret;
}

int bsp_efuse_ioctl_get_spniden(int arg __attribute__((unused)))
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_read(&value, GROUP_INDEX(EFUSE_LAYOUT_ARM_DBG_CTRL_SPNIDEN_BIT_OFFSET), 1);
    if (ret)
        return ret;

    ret = (int)((value >> BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_ARM_DBG_CTRL_SPNIDEN_BIT_OFFSET)) & 0x1U);
    return ret;
}

int bsp_efuse_ioctl_set_spniden(int arg)
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_ioctl_get_spniden(arg);
    if (ret < 0)
        return ret;

    if (ret > 0)
        return 1;

    value = (arg & 0x1) << BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_ARM_DBG_CTRL_SPNIDEN_BIT_OFFSET);

    ret = bsp_efuse_write(&value, GROUP_INDEX(EFUSE_LAYOUT_ARM_DBG_CTRL_SPNIDEN_BIT_OFFSET), 1);

    return ret;
}

int bsp_efuse_ioctl_get_hifidbgen(int arg __attribute__((unused)))
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_read(&value, GROUP_INDEX(EFUSE_LAYOUT_HIFI_DBG_CTRL_BIT_OFFSET), 1);
    if (ret)
        return ret;

    ret = (int)((value >> BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_HIFI_DBG_CTRL_BIT_OFFSET)) & 0x1U);
    return ret;
}

int bsp_efuse_ioctl_set_hifidbgen(int arg)
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_ioctl_get_hifidbgen(arg);
    if (ret < 0)
        return ret;

    if (ret > 0)
        return 1;

    value = (arg & 0x1) << BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_HIFI_DBG_CTRL_BIT_OFFSET);

    ret = bsp_efuse_write(&value, GROUP_INDEX(EFUSE_LAYOUT_HIFI_DBG_CTRL_BIT_OFFSET), 1);

    return ret;
}

int bsp_efuse_ioctl_get_bbe16dbgen(int arg __attribute__((unused)))
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_read(&value, GROUP_INDEX(EFUSE_LAYOUT_BBE16_DBG_CTRL_BIT_OFFSET), 1);
    if (ret)
        return ret;

    ret = (int)((value >> BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_BBE16_DBG_CTRL_BIT_OFFSET)) & 0x1U);
    return ret;
}

int bsp_efuse_ioctl_set_bbe16dbgen(int arg)
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_ioctl_get_bbe16dbgen(arg);
    if (ret < 0)
        return ret;

    if (ret > 0)
        return 1;

    value = (arg & 0x1) << BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_BBE16_DBG_CTRL_BIT_OFFSET);

    ret = bsp_efuse_write(&value, GROUP_INDEX(EFUSE_LAYOUT_BBE16_DBG_CTRL_BIT_OFFSET), 1);

    return ret;
}

int bsp_efuse_ioctl_get_csdeviceen(int arg __attribute__((unused)))
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_read(&value, GROUP_INDEX(EFUSE_LAYOUT_CS_DEVICE_CTRL_BIT_OFFSET), 1);
    if (ret)
        return ret;

    ret = (int)((value >> BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_CS_DEVICE_CTRL_BIT_OFFSET)) & 0x1U);
    return ret;
}

int bsp_efuse_ioctl_set_csdeviceen(int arg)
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_ioctl_get_csdeviceen(arg);
    if (ret < 0)
        return ret;

    if (ret > 0)
        return 1;

    value = (arg & 0x1) << BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_CS_DEVICE_CTRL_BIT_OFFSET);

    ret = bsp_efuse_write(&value, GROUP_INDEX(EFUSE_LAYOUT_CS_DEVICE_CTRL_BIT_OFFSET), 1);

    return ret;
}

int bsp_efuse_ioctl_get_jtagen(int arg __attribute__((unused)))
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_read(&value, GROUP_INDEX(EFUSE_LAYOUT_JTAGEN_CTRL_BIT_OFFSET), 1);
    if (ret)
        return ret;

    ret = (int)((value >> BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_JTAGEN_CTRL_BIT_OFFSET)) & 0x1U);
    return ret;
}

int bsp_efuse_ioctl_set_jtagen(int arg)
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_ioctl_get_jtagen(arg);
    if (ret < 0)
        return ret;

    if (ret > 0)
        return 1;

    value = (arg & 0x1) << BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_JTAGEN_CTRL_BIT_OFFSET);

    ret = bsp_efuse_write(&value, GROUP_INDEX(EFUSE_LAYOUT_JTAGEN_CTRL_BIT_OFFSET), 1);

    return ret;
}

int bsp_efuse_ioctl_get_secdbgreset(int arg __attribute__((unused)))
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_read(&value, GROUP_INDEX(EFUSE_LAYOUT_SEC_DBG_RST_CTRL_BIT_OFFSET), 1);
    if (ret)
        return ret;

    ret = (int)((value >> BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_SEC_DBG_RST_CTRL_BIT_OFFSET)) & 0x1U);
    return ret;
}

int bsp_efuse_ioctl_set_secdbgreset(int arg)
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_ioctl_get_secdbgreset(arg);
    if (ret < 0)
        return ret;

    if (ret > 0)
        return 1;

    value = (arg & 0x1) << BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_SEC_DBG_RST_CTRL_BIT_OFFSET);

    ret = bsp_efuse_write(&value, GROUP_INDEX(EFUSE_LAYOUT_SEC_DBG_RST_CTRL_BIT_OFFSET), 1);

    return ret;
}

int bsp_efuse_ioctl_get_csreset(int arg __attribute__((unused)))
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_read(&value, GROUP_INDEX(EFUSE_LAYOUT_CORESIGHT_RST_CTRL_BIT_OFFSET), 1);
    if (ret)
        return ret;

    ret = (int)((value >> BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_CORESIGHT_RST_CTRL_BIT_OFFSET)) & 0x1U);
    return ret;
}

int bsp_efuse_ioctl_set_csreset(int arg)
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_ioctl_get_csreset(arg);
    if (ret < 0)
        return ret;

    if (ret > 0)
        return 1;

    value = (arg & 0x1) << BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_CORESIGHT_RST_CTRL_BIT_OFFSET);

    ret = bsp_efuse_write(&value, GROUP_INDEX(EFUSE_LAYOUT_CORESIGHT_RST_CTRL_BIT_OFFSET), 1);

    return ret;
}

int bsp_efuse_ioctl_get_dftsel(int arg __attribute__((unused)))
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_read(&value, GROUP_INDEX(EFUSE_LAYOUT_DFT_DISABLE_SEL_BIT_OFFSET), 1);
    if (ret)
        return ret;

    ret = (int)((value >> BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_DFT_DISABLE_SEL_BIT_OFFSET)) & 0x3U);
    return ret;
}

int bsp_efuse_ioctl_set_dftsel(int arg)
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_ioctl_get_dftsel(arg);
    if (ret < 0)
        return ret;

    if (ret > 0)
        return 1;

    value = (arg & 0x3) << BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_DFT_DISABLE_SEL_BIT_OFFSET);

    ret = bsp_efuse_write(&value, GROUP_INDEX(EFUSE_LAYOUT_DFT_DISABLE_SEL_BIT_OFFSET), 1);

    return ret;
}

int bsp_efuse_ioctl_get_authkey(unsigned char* buf, int len)
{
    int ret = 0;
    if ((!buf) || (len < EFUSE_LAYOUT_DFT_AUTH_KEY_LENGTH)) {
        efuse_print_error("bsp_efuse_ioctl_get_authkey fail. error args.\n");
        return EFUSE_ERROR_ARGS;
    }

    ret = bsp_efuse_read((u32*)buf, EFUSE_LAYOUT_DFT_AUTH_KEY_OFFSET, EFUSE_LAYOUT_DFT_AUTH_KEY_LENGTH);
    if (ret)
        return ret;

    return ret;
}

int bsp_efuse_ioctl_set_authkey(unsigned char* buf, int len)
{
    u32 i = 0;
    int ret = 0;
    u32 value[EFUSE_LAYOUT_DFT_AUTH_KEY_LENGTH] = {0};

    if ((!buf) || (len < EFUSE_LAYOUT_DFT_AUTH_KEY_LENGTH)) {
        efuse_print_error("bsp_efuse_ioctl_set_authkey fail. error args.\n");
        return EFUSE_ERROR_ARGS;
    }

    ret = bsp_efuse_ioctl_get_authkey((unsigned char*)&value[0], len);
    if (ret < 0)
        return ret;

    for (i = 0; i < sizeof(value)/sizeof(value[0]); i++) {
        if (value[i])
            return 1;
    }

    ret = bsp_efuse_write((u32*)buf, EFUSE_LAYOUT_DFT_AUTH_KEY_OFFSET, EFUSE_LAYOUT_DFT_AUTH_KEY_LENGTH);

    return ret;
}

int bsp_efuse_ioctl_get_authkeyrd(int arg __attribute__((unused)))
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_read(&value, GROUP_INDEX(EFUSE_LAYOUT_DFT_AUTH_KEY_RD_CTRL_BIT_OFFSET), 1);
    if (ret)
        return ret;

    ret = (int)((value >> BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_DFT_AUTH_KEY_RD_CTRL_BIT_OFFSET)) & 0x1U);
    return ret;
}

int bsp_efuse_ioctl_set_authkeyrd(int arg)
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_ioctl_get_authkeyrd(arg);
    if (ret < 0)
        return ret;

    if (ret > 0)
        return 1;

    value = (arg & 0x1U) << BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_DFT_AUTH_KEY_RD_CTRL_BIT_OFFSET);

    ret = bsp_efuse_write(&value, GROUP_INDEX(EFUSE_LAYOUT_DFT_AUTH_KEY_RD_CTRL_BIT_OFFSET), 1);

    return ret;
}

int bsp_efuse_ioctl_get_nsiverify(int arg __attribute__((unused)))
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_read(&value, GROUP_INDEX(EFUSE_LAYOUT_NS_VERIFY_BIT_OFFSET), 1);
    if (ret)
        return ret;

    ret = (int)((value >> BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_NS_VERIFY_BIT_OFFSET)) & 0x3U);
    return ret;
}

int bsp_efuse_ioctl_set_nsiverify(int arg)
{
    int ret = 0;
    u32 value = 0;

    ret = bsp_efuse_ioctl_get_nsiverify(arg);
    if (ret < 0)
        return ret;

    if (ret > 0)
        return 1;

    value = (arg & 0x3) << BIT_INDEX_IN_GROUP(EFUSE_LAYOUT_NS_VERIFY_BIT_OFFSET);

    ret = bsp_efuse_write(&value, GROUP_INDEX(EFUSE_LAYOUT_NS_VERIFY_BIT_OFFSET), 1);

    return ret;
}

int bsp_efuse_ioctl_get_uartdbgen(int arg __attribute__((unused)))
{
    int ret = 0;
    return ret;
}

int bsp_efuse_ioctl_set_uartdbgen(int arg)
{
    int ret = 0;
    return ret;
}

int bsp_efuse_ioctl_get_nxbbe32dbgen(int arg __attribute__((unused)))
{
    int ret = 0;
    return ret;
}

int bsp_efuse_ioctl_set_nxbbe32dbgen(int arg)
{
    int ret = 0;
    return ret;
}

int bsp_efuse_ioctl_get_pdedbgen(int arg __attribute__((unused)))
{
    int ret = 0;
    return ret;
}

int bsp_efuse_ioctl_set_pdedbgen(int arg)
{
    int ret = 0;
    return ret;
}

int bsp_efuse_ioctl_get_ccpudbgen(int arg __attribute__((unused)))
{
    int ret = 0;
    return ret;
}

int bsp_efuse_ioctl_set_ccpudbgen(int arg)
{
    int ret = 0;
    return ret;
}

int bsp_efuse_ioctl_get_acpudbgen(int arg __attribute__((unused)))
{
    int ret = 0;
    return ret;
}

int bsp_efuse_ioctl_set_acpudbgen(int arg)
{
    int ret = 0;
    return ret;
}

struct efuse_ioctrl_op {
    int op_code;
    int (*func_ioctrl)(int);
};

struct efuse_ioctrl_op g_efuse_ioctrl_op[] = {
    {MDRV_EFUSE_IOCTL_CMD_SET_SECURESTATE,  bsp_efuse_ioctl_set_securestate},
    {MDRV_EFUSE_IOCTL_CMD_GET_SECURESTATE,  bsp_efuse_ioctl_get_securestate},
    {MDRV_EFUSE_IOCTL_CMD_SET_SECUREDEBUG,  bsp_efuse_ioctl_set_securedebug},
    {MDRV_EFUSE_IOCTL_CMD_GET_SECUREDEBUG,  bsp_efuse_ioctl_get_securedebug},
    {MDRV_EFUSE_IOCTL_CMD_SET_DBGEN,        bsp_efuse_ioctl_set_dbgen},
    {MDRV_EFUSE_IOCTL_CMD_GET_DBGEN,        bsp_efuse_ioctl_get_dbgen},
    {MDRV_EFUSE_IOCTL_CMD_SET_NIDEN,        bsp_efuse_ioctl_set_niden},
    {MDRV_EFUSE_IOCTL_CMD_GET_NIDEN,        bsp_efuse_ioctl_get_niden},
    {MDRV_EFUSE_IOCTL_CMD_SET_SPIDEN,       bsp_efuse_ioctl_set_spiden},
    {MDRV_EFUSE_IOCTL_CMD_GET_SPIDEN,       bsp_efuse_ioctl_get_spiden},
    {MDRV_EFUSE_IOCTL_CMD_SET_SPNIDEN,      bsp_efuse_ioctl_set_spniden},
    {MDRV_EFUSE_IOCTL_CMD_GET_SPNIDEN,      bsp_efuse_ioctl_get_spniden},
    {MDRV_EFUSE_IOCTL_CMD_SET_HIFIDBGEN,    bsp_efuse_ioctl_set_hifidbgen},
    {MDRV_EFUSE_IOCTL_CMD_GET_HIFIDBGEN,    bsp_efuse_ioctl_get_hifidbgen},
    {MDRV_EFUSE_IOCTL_CMD_SET_BBE16DBGEN,   bsp_efuse_ioctl_set_bbe16dbgen},
    {MDRV_EFUSE_IOCTL_CMD_GET_BBE16DBGEN,   bsp_efuse_ioctl_get_bbe16dbgen},
    {MDRV_EFUSE_IOCTL_CMD_SET_CSDEVICEEN,   bsp_efuse_ioctl_set_csdeviceen},
    {MDRV_EFUSE_IOCTL_CMD_GET_CSDEVICEEN,   bsp_efuse_ioctl_get_csdeviceen},
    {MDRV_EFUSE_IOCTL_CMD_SET_JTAGEN,       bsp_efuse_ioctl_set_jtagen},
    {MDRV_EFUSE_IOCTL_CMD_GET_JTAGEN,       bsp_efuse_ioctl_get_jtagen},
    {MDRV_EFUSE_IOCTL_CMD_SET_SECDBGRESET,  bsp_efuse_ioctl_set_secdbgreset},
    {MDRV_EFUSE_IOCTL_CMD_GET_SECDBGRESET,  bsp_efuse_ioctl_get_secdbgreset},
    {MDRV_EFUSE_IOCTL_CMD_SET_CSRESET,      bsp_efuse_ioctl_set_csreset},
    {MDRV_EFUSE_IOCTL_CMD_GET_CSRESET,      bsp_efuse_ioctl_get_csreset},
    {MDRV_EFUSE_IOCTL_CMD_SET_DFTSEL,       bsp_efuse_ioctl_set_dftsel},
    {MDRV_EFUSE_IOCTL_CMD_GET_DFTSEL,       bsp_efuse_ioctl_get_dftsel},
    {MDRV_EFUSE_IOCTL_CMD_SET_AUTHKEYRD,    bsp_efuse_ioctl_set_authkeyrd},
    {MDRV_EFUSE_IOCTL_CMD_GET_AUTHKEYRD,    bsp_efuse_ioctl_get_authkeyrd},
    {MDRV_EFUSE_IOCTL_CMD_SET_NSIVERIFY,    bsp_efuse_ioctl_set_nsiverify},
    {MDRV_EFUSE_IOCTL_CMD_GET_NSIVERIFY,    bsp_efuse_ioctl_get_nsiverify},
#ifdef MDRV_EFUSE_IOCTL_CMD_SET_UARTDBGEN
    {MDRV_EFUSE_IOCTL_CMD_SET_UARTDBGEN,    bsp_efuse_ioctl_set_uartdbgen},
    {MDRV_EFUSE_IOCTL_CMD_GET_UARTDBGEN,    bsp_efuse_ioctl_get_uartdbgen},
#endif
#ifdef MDRV_EFUSE_IOCTL_CMD_SET_NXBBE32DBGEN
    {MDRV_EFUSE_IOCTL_CMD_SET_NXBBE32DBGEN, bsp_efuse_ioctl_set_nxbbe32dbgen},
    {MDRV_EFUSE_IOCTL_CMD_GET_NXBBE32DBGEN, bsp_efuse_ioctl_get_nxbbe32dbgen},
#endif
#ifdef MDRV_EFUSE_IOCTL_CMD_SET_PDEDBGEN
    {MDRV_EFUSE_IOCTL_CMD_SET_PDEDBGEN,     bsp_efuse_ioctl_set_pdedbgen},
    {MDRV_EFUSE_IOCTL_CMD_GET_PDEDBGEN,     bsp_efuse_ioctl_get_pdedbgen},
#endif
#ifdef MDRV_EFUSE_IOCTL_CMD_SET_CCPUNIDEN
    {MDRV_EFUSE_IOCTL_CMD_SET_CCPUNIDEN,    bsp_efuse_ioctl_set_ccpudbgen},
    {MDRV_EFUSE_IOCTL_CMD_GET_CCPUNIDEN,    bsp_efuse_ioctl_get_ccpudbgen},
#endif
#ifdef MDRV_EFUSE_IOCTL_CMD_SET_ACPUNIDEN
    {MDRV_EFUSE_IOCTL_CMD_SET_ACPUNIDEN,    bsp_efuse_ioctl_set_acpudbgen},
    {MDRV_EFUSE_IOCTL_CMD_GET_ACPUNIDEN,    bsp_efuse_ioctl_get_acpudbgen},
#endif
    {-1,    NULL}
};


int mdrv_efuse_ioctl(int cmd, int arg, unsigned char* buf, int len)
{
    u32 i;
    int ret;

    switch (cmd) {
        case MDRV_EFUSE_IOCTL_CMD_GET_SOCID:
            ret = bsp_efuse_ioctl_get_socid(buf, len);
            break;
        case MDRV_EFUSE_IOCTL_CMD_SET_KCE:
            ret = bsp_efuse_ioctl_set_kce(buf, len);
            break;
        case MDRV_EFUSE_IOCTL_CMD_SET_AUTHKEY:
            ret = bsp_efuse_ioctl_set_authkey(buf, len);
            break;
        case MDRV_EFUSE_IOCTL_CMD_GET_AUTHKEY:
            ret = bsp_efuse_ioctl_get_authkey(buf, len);
            break;
        default:
            for (i = 0; i < sizeof(g_efuse_ioctrl_op)/sizeof(g_efuse_ioctrl_op[0]); i++) {
                if (cmd == g_efuse_ioctrl_op[i].op_code) {
                    ret = g_efuse_ioctrl_op[i].func_ioctrl(arg);
                    break;
                }
            }
            if (i == sizeof(g_efuse_ioctrl_op)/sizeof(g_efuse_ioctrl_op[0])) {
                efuse_print_error("unsupported command, cmd = %d\n", cmd);
                ret = EFUSE_ERROR_ARGS;
            }
    }

    return ret;
}

#else

int mdrv_efuse_ioctl(int cmd, int arg, unsigned char* buf, int len)
{
    efuse_print_error("It's only a stub\n");

    return EFUSE_OK;
}

#endif


#endif

#else   /* CONFIG_EFUSE */

int mdrv_efuse_write_huk(char *pBuf,unsigned int len)
{
    return EFUSE_OK;
}

int mdrv_efuse_check_huk_valid(void)
{
    return EFUSE_OK;
}

int mdrv_efuse_get_dieid(unsigned char* buf,int length)
{
    return EFUSE_OK;
}

int mdrv_efuse_get_chipid(unsigned char* buf,int length)
{
    return EFUSE_OK;
}

int mdrv_efuse_ioctl(int cmd, int arg, unsigned char* buf, int len)
{
    return EFUSE_OK;
}

#endif

int bsp_sec_check(void* image_addr, unsigned int image_length)
{
    return 0;
}
EXPORT_SYMBOL(bsp_sec_check);

int mdrv_crypto_secboot_supported(unsigned char *pu8Data)
{
    return 0;
}
EXPORT_SYMBOL(mdrv_crypto_secboot_supported);

int mdrv_crypto_sec_started(unsigned char *pu8Data)
{
    return 0;
}
EXPORT_SYMBOL(mdrv_crypto_sec_started);

int mdrv_crypto_start_secure(void)
{
    return 0;
}
EXPORT_SYMBOL(mdrv_crypto_start_secure);

unsigned int mdrv_crypto_check_sec(void)
{
    return 0;
}
EXPORT_SYMBOL(mdrv_crypto_check_sec);



EXPORT_SYMBOL(mdrv_efuse_ioctl);
EXPORT_SYMBOL(mdrv_efuse_get_chipid);

