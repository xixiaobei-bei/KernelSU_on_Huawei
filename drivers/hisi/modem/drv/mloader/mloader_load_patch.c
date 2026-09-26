/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/file.h>
#include <linux/uaccess.h>

#ifdef CONFIG_MLOADER_COLD_PATCH
#include <linux/mtd/hisi_nve_interface.h>
#endif
#include <bsp_dump.h>
#include <bsp_slice.h>

#include <mdrv_sysboot.h>
#include <bsp_cold_patch.h>

#include <securec.h>
#include <hi_mloader.h>

#include "mloader_comm.h"
#include "mloader_load_image.h"
#include "mloader_debug.h"

#define THIS_MODU mod_mloader

struct cold_patch_info_s g_patch_info;

#ifdef CONFIG_MLOADER_COLD_PATCH

#define COLD_PATCH_DUMP_SIZE (128)
char *g_cold_patch_dump_base = NULL;

struct patch_ret_value_s g_patch_ret[MAX_PATCH];

static DEFINE_MUTEX(cold_patch_proc_lock);

struct cold_patch_info_s *mloader_get_cold_patch_info_st(void)
{
    return &g_patch_info;
}

int bsp_nvem_cold_patch_read(struct cold_patch_info_s *p_patch_info)
{
    int ret;
    struct hisi_nve_info_user nve_cold_patch = { 0 };

    nve_cold_patch.nv_operation = NV_READ;
    nve_cold_patch.nv_number = NVME_COLD_PATCH_ID;
    nve_cold_patch.valid_size = sizeof(*p_patch_info);

    ret = hisi_nve_direct_access(&nve_cold_patch);
    if (ret) {
        bsp_err("nvem_cold_patch_read:nvem read failed!\n");
        return ret;
    }
    ret = memcpy_s(p_patch_info, sizeof(*p_patch_info), &nve_cold_patch.nv_data[0], sizeof(*p_patch_info));
    if (ret != 0) {
        bsp_err("nvem_cold_patch_read:memcpy_s failed!\n");
        return ret;
    }

    return 0;
}
int bsp_nvem_cold_patch_write(struct cold_patch_info_s *p_patch_info)
{
    int ret;
    struct hisi_nve_info_user nve_cold_patch = { 0 };

    nve_cold_patch.nv_operation = NV_WRITE;
    nve_cold_patch.nv_number = NVME_COLD_PATCH_ID;
    nve_cold_patch.valid_size = sizeof(*p_patch_info);

    bsp_err("start update nve for patch!\n");
    ret = memcpy_s(&(nve_cold_patch.nv_data[0]), sizeof(*p_patch_info), p_patch_info, sizeof(*p_patch_info));
    if (ret != 0) {
        bsp_err("cold_patch:memcpy_s failed!\n");
        return ret;
    }
    ret = hisi_nve_direct_access(&nve_cold_patch);
    if (ret) {
        bsp_err("nvem_cold_patch_write:nvem write failed!\n");
        return ret;
    }
    bsp_err("end update nve for patch!\n");
    return 0;
}

void bsp_nvem_cold_patch_read_debug(void)
{
    u32 i;

    bsp_err("cold_patch_status: 0x%x\n", g_patch_info.cold_patch_status);
    bsp_err("modem_patch_update_count: 0x%x\n", g_patch_info.modem_patch_update_count);
    bsp_err("modem_update_fail_count: 0x%x\n", g_patch_info.modem_update_fail_count);
    for (i = 0; i < MAX_PATCH; i++) {
        bsp_err("modem_patch_info[%d] patch_exist: 0x%x\n", i, g_patch_info.modem_patch_info[i].patch_exist);
        bsp_err("modem_patch_info[%d] patch_status: 0x%x\n", i, g_patch_info.modem_patch_info[i].patch_status);
        bsp_err("patch_ret_value[%d].load_ret_val: 0x%x\n", i, g_patch_ret[i].load_ret_val);
        bsp_err("patch_ret_value[%d].splice_ret_val: 0x%x\n", i, g_patch_ret[i].splice_ret_val);
    }
}

bool bsp_modem_cold_patch_is_exist(void)
{
    int i;

    for (i = 0; i < MAX_PATCH; i++) {
        if ((u32)g_patch_info.modem_patch_info[i].patch_exist == 1) {
            return 1;
        }
    }
    return 0;
}

void bsp_modem_cold_patch_update_modem_fail_count(void)
{
    int ret;

    mutex_lock(&cold_patch_proc_lock);

    if (g_cold_patch_dump_base != NULL) {
        if (memcpy_s(g_cold_patch_dump_base, COLD_PATCH_DUMP_SIZE, &g_patch_info, sizeof(struct cold_patch_info_s))) {
            bsp_err("update_modem_fail_count:memcpy_s failed!\n");
        }
    }

    if (!bsp_modem_cold_patch_is_exist()) {
        mutex_unlock(&cold_patch_proc_lock);
        return;
    }

    if (g_patch_info.modem_update_fail_count >= 0x3) {
        mutex_unlock(&cold_patch_proc_lock);
        return;
    }
    g_patch_info.modem_update_fail_count++;
    ret = bsp_nvem_cold_patch_write(&g_patch_info);
    if (ret)
        bsp_err("bsp_nvem_cold_patch_write :update nevm failed!\n");
    mutex_unlock(&cold_patch_proc_lock);
}

ssize_t modem_imag_patch_status_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int ret;
    ssize_t size = sizeof(struct cold_patch_info_s);
    long modem_cold_patch_clear = 0;
    if ((kstrtol(buf, 0, &modem_cold_patch_clear) < 0) && (modem_cold_patch_clear != 0))
        return 0;

    mutex_lock(&cold_patch_proc_lock);

    (void)memset_s(&g_patch_info, size, 0, size);
    ret = bsp_nvem_cold_patch_write(&g_patch_info);
    if (ret) {
        bsp_err("modem_imag_patch_status_store:write nvem failed!\n");
        mutex_unlock(&cold_patch_proc_lock);
        return 0;
    }

    mutex_unlock(&cold_patch_proc_lock);

    return count;
}

ssize_t modem_imag_patch_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32 i;
    u32 ret;
    ssize_t size;
    char data_buf[20U] = { 0 };
    if (buf == NULL) {
        return 0;
    }

    mutex_lock(&cold_patch_proc_lock);

    if (g_patch_info.cold_patch_status) {
        size = strlen("HOTA_SUCCESS:");
        ret = (u32)strcat_s(buf, PAGE_SIZE, "HOTA_SUCCESS:");
    } else {
        size = strlen("HOTA_ERR_STACK:");
        ret = (u32)strcat_s(buf, PAGE_SIZE, "HOTA_ERR_STACK:");
    }
    size += snprintf_s(data_buf, sizeof(data_buf), sizeof(data_buf) - 1, "%d,", g_patch_info.cold_patch_status) + 1;
    ret |= (u32)strcat_s(buf, PAGE_SIZE, data_buf);

    size +=
        snprintf_s(data_buf, sizeof(data_buf), sizeof(data_buf) - 1, "%d,", g_patch_info.modem_update_fail_count) + 1;
    ret |= (u32)strcat_s(buf, PAGE_SIZE, data_buf);

    size += snprintf_s(data_buf, sizeof(data_buf), sizeof(data_buf) - 1, "%d,", g_patch_info.nv_rollback) + 1;
    ret |= (u32)strcat_s(buf, PAGE_SIZE, data_buf);

    for (i = 0; i < MAX_PATCH; i++) {
        size += snprintf_s(data_buf, sizeof(data_buf), sizeof(data_buf) - 1, "%d,", i) + 1;
        ret |= (u32)strcat_s(buf, PAGE_SIZE, data_buf);

        size += snprintf_s(data_buf, sizeof(data_buf), sizeof(data_buf) - 1, "%d,",
            g_patch_info.modem_patch_info[i].patch_exist) +
            1;
        ret |= (u32)strcat_s(buf, PAGE_SIZE, data_buf);

        size += snprintf_s(data_buf, sizeof(data_buf), sizeof(data_buf) - 1, "%d,",
            g_patch_info.modem_patch_info[i].patch_status) +
            1;
        ret |= (u32)strcat_s(buf, PAGE_SIZE, data_buf);

        size += snprintf_s(data_buf, sizeof(data_buf), sizeof(data_buf) - 1, "%x,", g_patch_ret[i].load_ret_val) + 1;
        ret |= (u32)strcat_s(buf, PAGE_SIZE, data_buf);
    }

    mutex_unlock(&cold_patch_proc_lock);

    if (ret != EOK) {
        bsp_err("patch_status_show:strcat_s failed\n");
    }

    return size;
}
int mloader_is_patch_image(const char *image_name, u32 buf_len)
{
    int len;

    len = strnlen(image_name, buf_len);
    if ((image_name[len - 1] == 'p') && (image_name[len - 0x2] == '.')) {
        return 1;
    }
    return 0;
}
int mloader_patch_get_image_name(enum modem_patch_type patch_type, char *image_name, u32 len)
{
    int ret = 0;

    image_name[0] = '\0';
    switch (patch_type) {
        case CCORE_PATCH:
            (void)strncpy_s(image_name, len, "modem_lr_ccpu.bin.p", sizeof("modem_lr_ccpu.bin.p"));
            break;
        case LR_PATCH:
            (void)strncpy_s(image_name, len, "modem_lr_ccpu.bin.p", sizeof("modem_lr_ccpu.bin.p"));
            break;
        case NR_PATCH:
            (void)strncpy_s(image_name, len, "modem_nr_ccpu.bin.p", sizeof("modem_nr_ccpu.bin.p"));
            break;
        case LR_PHY_PATCH:
            (void)strncpy_s(image_name, len, "lr_phy.bin.p", sizeof("lr_phy.bin.p"));
            break;
        case RFDSP1_PATCH:
            (void)strncpy_s(image_name, len, "rfdsp_sub6g.bin.p", sizeof("rfdsp_sub6g.bin.p"));
            break;
        case RFDSP2_PATCH:
            (void)strncpy_s(image_name, len, "rfdsp_hf.bin.p", sizeof("rfdsp_hf.bin.p"));
            break;
        case L2HAC_PATCH:
            (void)strncpy_s(image_name, len, "modem_l2hac.bin.p", sizeof("modem_l2hac.bin.p"));
            break;
        case NR_PHY_PATCH:
            (void)strncpy_s(image_name, len, "nr_phy.bin.p", sizeof("nr_phy.bin.p"));
            break;
        case PDE_PDF_PATCH:
            (void)strncpy_s(image_name, len, "pde_nr_pdf.bin.p", sizeof("pde_nr_pdf.bin.p"));
            break;
        case PDE_CSI_PATCH:
            (void)strncpy_s(image_name, len, "pde_nr_csi.bin.p", sizeof("pde_nr_csi.bin.p"));
            break;
        case RFDSP1_ES_PATCH:
            (void)strncpy_s(image_name, len, "rfdsp_sub6g_es.bin.p", sizeof("rfdsp_sub6g_es.bin.p"));
            break;
        case RFDSP2_ES_PATCH:
            (void)strncpy_s(image_name, len, "rfdsp_hf_es.bin.p", sizeof("rfdsp_hf_es.bin.p"));
            break;
        default:
            (void)strncpy_s(image_name, len, "", sizeof(""));
    }
    return ret;
}

u32 mloader_patch_get_type(const char *file_name, unsigned int len)
{
    u32 i;
    char image_name[MLOADER_FILE_NAME_LEN];
    char f_name[MLOADER_FILE_NAME_LEN];

    if (len < MLOADER_FILE_NAME_LEN) {
        mloader_print_err("access to file_name may out of bounds, len = 0x%x!\n", len);
        return MAX_PATCH;
    }

    for (i = 0; i < MAX_PATCH; i++) {
        (void)mloader_patch_get_image_name(i, image_name, MLOADER_FILE_NAME_LEN);
        mloader_get_file_name(f_name, image_name, 0);
        if (strncmp((char *)f_name, (char *)file_name, MLOADER_FILE_NAME_LEN) == 0) {
            return i;
        }
    }
    return i;
}

u32 mloader_patch_get_load_position_offset(enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 ddr_size, int img_size)
{
    if (img_size > ddr_size) {
        bsp_err("param img_size is too large,soc_type = MODEM,img_size = 0x%x,ddr_size = 0x%x!\n", img_size, ddr_size);
        return 0;
    }

    if (ecoretype == MODEM_COLD_PATCH) {
        if (g_patch_info.modem_patch_info[LR_PATCH].patch_exist == 0) {
            return 0;
        }
        /* 将整个patch镜像放在DDR空间结束位置 */
        return (u32)(ddr_size - (u32)img_size);
    } else if (ecoretype == MODEM) {
        /*
         * 如果已经加载CCORE补丁镜像，MODEM
         * DDR空间的最后位置存放CCORE补丁镜像，CCORE补丁镜像之上存放原CCORE镜像，所以原镜像的 加载位置=ddr_size -
         * CCORE补丁镜像大小 - 原CCORE镜像大小； 如果CCORE补丁镜像不存在或加载失败，原CCORE镜像加载到MODEM
         * DDR空间的最后位置
         */
        if (g_patch_info.modem_patch_info[LR_PATCH].patch_status == PUT_PATCH)
            return (u32)(ddr_size - g_patch_info.lr_patch_size - (u32)img_size);
        else
            return (u32)(ddr_size - (u32)img_size);
    } else {
        return 0;
    }
}

int mloader_load_patch_image(enum modem_patch_type patch_type, const ccpu_img_info_s *image,
    mloader_img_icc_info_s *mloader_msg, u32 index)
{
    int ret;
    u32 core_type;
    char file_name[MLOADER_FILE_NAME_LEN] = { 0 };
    struct file *fp = NULL;

    ret = sprintf_s(file_name, sizeof(file_name), "%s%s%s", MLOADER_COLD_PATCH_PATH, image->image_name, ".p");
    if (ret < 0) {
        mloader_print_err("error:strncat_s %s failed when getting cold_patch name!\n", file_name);
        return -1;
    }

    fp = filp_open(file_name, O_RDONLY, 0);
    if (!IS_ERR(fp)) {
        filp_close(fp, NULL);
        g_patch_info.modem_patch_info[patch_type].patch_exist = 1;
        if (g_patch_info.modem_update_fail_count < 0x3) {
            if (patch_type == LR_PATCH) {
                /* modem的patch需要放到modem的ddr空间，所以获得的是modem的image信息 */
                core_type = patch_type == LR_PATCH ? MODEM_COLD_PATCH : MODEM_COMM_IMG;
                ret = load_image(file_name, core_type, image->ddr_addr, image->ddr_size, index);
            }
            g_patch_ret[patch_type].load_ret_val = ret;
            if (ret) {
                mloader_print_err("load patch img failed, patch_type %d\n", patch_type);
                g_patch_info.modem_patch_info[patch_type].patch_status = LOAD_PATCH_FAIL;
            } else {
                mloader_update_ccore_cold_patch_debug_info();
                g_patch_info.modem_patch_info[patch_type].patch_status = PUT_PATCH;
                g_patch_info.lr_patch_size = mloader_get_file_size(file_name) - SECBOOT_VRL_TABLE_SIZE;
            }
        } else {
            mloader_print_err("fail_count = %d,not load patch img, patch_type %d\n",
                g_patch_info.modem_update_fail_count, patch_type);
            g_patch_info.modem_patch_info[patch_type].patch_status = NOT_LOAD_PATCH;
            /* 打补丁modem启动失败3次后，将不再打补丁，函数需要返回0 */
            ret = 0;
        }
    } else {
        ret = -1;
        g_patch_info.modem_patch_info[patch_type].patch_exist = 0;
        mloader_print_err("patch img not exist, patch_type %d, patch_name %s.\n", patch_type, file_name);
        g_patch_info.modem_patch_info[patch_type].patch_status = NOT_LOAD_PATCH;
    }
    return ret;
}

void mloader_update_modem_cold_patch_status(enum modem_patch_type epatch_type)
{
    int i;

    if (epatch_type >= MAX_PATCH) {
        return;
    }

    g_patch_info.cold_patch_status = 0;

    if (g_patch_info.modem_update_fail_count >= 0x3)
        return;

    if (g_patch_info.modem_patch_info[epatch_type].patch_status == PUT_PATCH)
        g_patch_info.modem_patch_info[epatch_type].patch_status = PUT_PATCH_SUCESS;

    /*
     * 如果补丁镜像不存在或补丁镜像未加载，补丁的状态为NOT_LOAD_PATCH；如果补丁镜像与镜像拼接成功，补丁的状态为PUT_PATCH_SUCESS；
     * 如果所有补丁镜像的状态为NOT_LOAD_PATCH，表示未加载的补丁镜像，将cold_patch_status设置为0；
     * 如果所有补丁镜像的状态为NOT_LOAD_PATCH或PUT_PATCH_SUCESS（至少一个补丁镜像状态为PUT_PATCH_SUCESS），表示补丁镜像与原镜像拼接成功，将cold_patch_status设置为1
     */
    for (i = 0; i < MAX_PATCH; i++) {
        if ((g_patch_info.modem_patch_info[i].patch_status != PUT_PATCH_SUCESS) &&
            (g_patch_info.modem_patch_info[i].patch_status != NOT_LOAD_PATCH)) {
            mloader_print_err("modify_modem_cold_patch_status,line = %d, patch_type = 0x%x, patch_status = %d\n",
                __LINE__, i, g_patch_info.modem_patch_info[i].patch_status);
            return;
        }
    }
    for (i = 0; i < MAX_PATCH; i++) {
        if ((g_patch_info.modem_patch_info[i].patch_exist) &&
            (g_patch_info.modem_patch_info[i].patch_status != NOT_LOAD_PATCH)) {
            g_patch_info.cold_patch_status = 1;
            return;
        }
    }
    return;
}

void mloader_record_cold_patch_splicing_ret_val(enum modem_patch_type epatch_type, int value)
{
    if (epatch_type >= MAX_PATCH) {
        return;
    }

    if (g_patch_info.modem_patch_info[epatch_type].patch_status == PUT_PATCH) {
        g_patch_ret[epatch_type].splice_ret_val = value;
        g_patch_info.modem_update_fail_count = 0x3;
        g_patch_info.cold_patch_status = 0;
    }
}

u32 mloader_patch_get_load_fail_time(void)
{
    return (u32)g_patch_info.modem_update_fail_count;
}

int mloader_load_patch(enum modem_patch_type epatch_type, const ccpu_img_info_s *image,
    mloader_img_icc_info_s *mloader_msg, u32 index)
{
    int ret;
    struct file *fp = NULL;

    char file_name[MLOADER_FILE_NAME_LEN] = { 0 };

    ret = sprintf_s(file_name, sizeof(file_name), "%s%s%s", MLOADER_COLD_PATCH_PATH, image->image_name, ".p");
    if (ret < 0) {
        mloader_print_err("error:strncat_s %s failed when getting cold_patch name!\n", file_name);
        return -1;
    }

    /* get patch info */
    mloader_print_err("%d patch load start at 0x%x\n", epatch_type, bsp_get_slice_value());
    if (bsp_nvem_cold_patch_read(&g_patch_info)) {
        (void)memset_s(&g_patch_info, sizeof(g_patch_info), 0, sizeof(g_patch_info));
    }

    fp = filp_open(file_name, O_RDONLY, 0);
    if (IS_ERR(fp)) {
        mloader_print_err("patch img not exist, patch path: %s.\n", file_name);
        return 0;
    }
    filp_close(fp, NULL);

    /* load patch */
    ret = mloader_load_patch_image(epatch_type, image, mloader_msg, index);
    /* verify patch */
    if ((epatch_type == LR_PATCH) && (g_patch_info.modem_patch_info[LR_PATCH].patch_status == PUT_PATCH)) {
        /*
         * 将patch类型转换为core类型传给安全os，安全os不感知具体是哪个ccore的patch，
         * 统一通过comm patch处理；如果是comm patch在这里需要吧patch类型继续下来，
         * 校验完成后返回给ccore；
         */
        ret = mloader_verify_soc_image(MODEM_COLD_PATCH, 0);
        if (ret) {
            /* 如果校验失败，那么不再加载和拼接补丁 */
            mloader_record_cold_patch_splicing_ret_val(LR_PATCH, ret);
            mloader_print_err("verify image MODEM_COLD_PATCH fail, ret = 0x%x\n", ret);
            return ret;
        }
        mloader_print_err("verify image MODEM_COLD_PATCH success\n");
    }
    /* 如果patch 不存在，load patch
     * image函数会返回负数，但是这个并不是异常，所以在返回加载结果之前，需要将ret置成0再返回 */
    if (g_patch_info.modem_patch_info[epatch_type].patch_exist == 0) {
        ret = 0;
    }
    return ret;
}

int bsp_modem_is_reboot_machine(void)
{
    int ret;
    mutex_lock(&cold_patch_proc_lock);
    if (g_patch_info.nv_rollback == 0) {
        if (((u32)g_patch_info.modem_patch_info[NV_PATCH].patch_exist == 1) &&
            ((u32)g_patch_info.modem_patch_info[NV_PATCH].patch_status == PUT_PATCH_SUCESS) &&
            (g_patch_info.modem_update_fail_count >= 0x3)) {
            g_patch_info.nv_rollback = 1;
            ret = bsp_nvem_cold_patch_write(&g_patch_info);
            if (ret) {
                bsp_err("bsp_modem_is_reboot_machine :update nevm failed!\n");
            }
            mutex_unlock(&cold_patch_proc_lock);
            return 1;
        }
    }
    mutex_unlock(&cold_patch_proc_lock);
    return 0;
}

int bsp_modem_cold_patch_init(void)
{
    g_cold_patch_dump_base =
        (char *)bsp_dump_register_field(DUMP_MODEMAP_COLD_PATCH, "cold_patch", COLD_PATCH_DUMP_SIZE, 0);
    /*
     * 系统异常，调用system_error时，如果识别到有补丁，就会调用这个接口
     * 调这个接口一次更新一次打补丁失败的计数；  如果失败次数达到3次，下次启动时就不打补丁了
     */
    (void)bsp_dump_register_hook("modem_cold_patch", bsp_modem_cold_patch_update_modem_fail_count);
    bsp_err("[init]bsp_modem_cold_patch_init ok!\n");
    return 0;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(bsp_modem_cold_patch_init);
#endif

#else

int mloader_is_patch_image(const char *image_name, u32 buf_len)
{
    return 0;
}

struct cold_patch_info_s *mloader_get_cold_patch_info_st(void)
{
    return &g_patch_info;
}

bool bsp_modem_cold_patch_is_exist(void)
{
    return 0;
}

ssize_t modem_imag_patch_status_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return count;
}

ssize_t modem_imag_patch_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return 0;
}

u32 mloader_patch_get_type(const char *file_name, unsigned int len)
{
    return MAX_PATCH;
}

int mloader_load_patch(enum modem_patch_type epatch_type, const ccpu_img_info_s *image,
    mloader_img_icc_info_s *mloader_msg, u32 index)
{
    return 0;
}

u32 mloader_patch_get_load_position_offset(enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 ddr_size, int img_size)
{
    return 0;
}

u32 mloader_patch_get_load_fail_time(void)
{
    return 0;
}

int bsp_modem_cold_patch_init(void)
{
    return 0;
}

void mloader_update_modem_cold_patch_status(enum modem_patch_type epatch_type)
{
    return;
}

void mloader_record_cold_patch_splicing_ret_val(enum modem_patch_type epatch_type, int value)
{
    return;
}

int bsp_nvem_cold_patch_write(struct cold_patch_info_s *p_cold_patch_info)
{
    return 0;
}

int bsp_modem_is_reboot_machine(void)
{
    return 0;
}


#endif

