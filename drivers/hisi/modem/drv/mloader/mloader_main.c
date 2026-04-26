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
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <uapi/linux/sched/types.h>

#include <bsp_dt.h>
#include <bsp_rfile.h>
#include <bsp_version.h>

#include <mdrv_sysboot.h>

#include <securec.h>
#include <mdrv_memory_layout.h>
#include "mloader_comm.h"
#include "mloader_load_image.h"
#include "mloader_debug.h"
#include "mloader_msg.h"

#ifdef BSP_CONFIG_PHONE_TYPE
#include <adrv.h>
#endif

#define THIS_MODU mod_mloader

DECLARE_COMPLETION(g_mloader_sysboot_start_complete);

mloader_info_s g_mloader_info;
mloader_img_s g_mloader_images;
mloader_addr_s g_mloader_addr;
long g_mloader_load_type;

mloader_addr_s *bsp_mloader_get_addr(void)
{
    return &g_mloader_addr;
}

mloader_img_s *bsp_mloader_get_images_st(void)
{
    return &g_mloader_images;
}

ccpu_img_info_s *mloader_get_image_info(const char *image_name)
{
    int id;
    for (id = 0; id < g_mloader_info.image_num; id++) {
        if (!strcmp(g_mloader_info.img[id].image_name, image_name)) {
            return &g_mloader_info.img[id];
        }
    }

    return NULL;
}

static int mloader_load_get_flag(u32 flag)
{
    return g_mloader_images.load_flags & BIT(flag);
}

void mloader_load_set_flag(u32 flag)
{
    set_bit(flag, &g_mloader_images.load_flags);
}

static void mloader_load_clear_flag(u32 flag)
{
    clear_bit(flag, &g_mloader_images.load_flags);
}

void mloader_load_clear_outer_event(void)
{
    g_mloader_images.outer_event = MLOADER_LOAD_UNINITED;
    mloader_print_err("mloader clear out event.\n");
}

void mloader_trigger_load_task(u32 event)
{
    g_mloader_images.outer_event = event;
    mloader_print_err("mloader out event %d.\n", event);
    __pm_stay_awake(&g_mloader_images.wake_lock);
    osl_sem_up(&g_mloader_images.task_sem);
}

int mloader_load_notifier(mloader_load_notifier_s *load_notifier)
{
    if (load_notifier == NULL || load_notifier->func == NULL) {
        mloader_print_err("mloader_load_notifier_regitser input param error.\n");
        return -1;
    }
    list_add(&(load_notifier->list), &(g_mloader_images.load_notifier_list));
    return 0;
}

int bsp_mloader_load_notifier_register(mloader_load_notifier_s *load_notifier)
{
    if (load_notifier == NULL || load_notifier->func == NULL) {
        mloader_print_err("mloader_load_notifier_regitser input param error.\n");
        return -1;
    }
    return mloader_load_notifier(load_notifier);
}

void mloader_load_notifier_process(load_stage_e state)
{
    mloader_load_notifier_s *load_notifier = NULL;

    list_for_each_entry(load_notifier, &(g_mloader_images.load_notifier_list), list) {
        if (load_notifier->func) {
            (*(load_notifier->func))(state);
        }
    }
}

static void mloader_modules_init(void)
{
    int ret;
    ret = modem_dir_init();
    if (ret) {
        mloader_print_err("modem_dir_init failed, ret %x\n", ret);
        return;
    }
#if (FEATURE_ON == FEATURE_DELAY_MODEM_INIT)
    mloader_print_err("modem nv init start\n");
    ret = modem_nv_init();
    if (ret) {
        mloader_print_err("modem nv init fail\n");
    }
    mloader_print_err("VOS Module Init start\n");
    ret = VOS_ModuleInit();
    if (ret) {
        mloader_print_err("VOS Module Init fail\n");
    }
#endif

    return;
}

int bsp_mloader_load_reset(void)
{
    int timeout;

    /* halt load task */
    mloader_load_clear_flag(MLOADER_LOAD_IN_HALT);
    mloader_load_set_flag(MLOADER_LOAD_REQ_HALT);
    mloader_print_err("mloader send halt flag.\n");

    mloader_trigger_load_task(MLOADER_LOAD_MODEM_REQ_RESET);

    timeout = 300U;
    while (!mloader_load_get_flag(MLOADER_LOAD_IN_HALT)) {
        msleep(10U);
        timeout--;
        if (timeout < 0) {
            mloader_print_err("wait for mloader halt timeout\n");
            return -1;
        }
    }
    mloader_print_err("mloader is in halt.\n");
    return 0;
}

int bsp_load_modem_images(void)
{
    /* clear halt state */
    mloader_load_clear_flag(MLOADER_LOAD_REQ_HALT);
    mloader_print_err("mloader clear halt flag.\n");

    /* start load */
    mloader_load_set_flag(MLOADER_LOAD_LR);
    mloader_trigger_load_task(MLOADER_LOAD_MODEM_IN_RESET);
    return 0;
}

void mloader_images_task_loop(void)
{
    while (!kthread_should_stop()) {
        osl_sem_down(&g_mloader_images.task_sem);
        /* 如果有单独复位，需要把当前加载的加载完，并将任务halt住，halt住之后，执行单独复位加载任务 */
        if (mloader_load_get_flag(MLOADER_LOAD_REQ_HALT)) {
            mloader_load_set_flag(MLOADER_LOAD_IN_HALT);
            mloader_print_err("mloader receive halt flag\n");
            goto done;
        }

        g_mloader_images.op_state = MLOADER_LOAD_DOING;
        mloader_update_start_time_debug_info();
        mloader_update_op_state(MLOADER_LOAD_DOING);

        if (mloader_load_get_flag(MLOADER_LOAD_LR)) {
            // load lr
            mloader_update_ccore_start_time_debug_info();
            mloader_load_lr_img();
            mloader_update_ccore_end_time_debug_info();
            mloader_load_clear_flag(MLOADER_LOAD_LR);
            mloader_load_clear_outer_event();
        } else {
            /* receive icc msg from ccore and load img */
            mloader_load_ccore_imgs();
        }
        g_mloader_images.op_state = MLOADER_LOAD_IDLE;
done:
        mloader_update_op_state(MLOADER_LOAD_IDLE);
        mloader_print_info("mloader load ok\n");
    }
}

int mloader_images_task(void *data)
{
    const bsp_version_info_s *version_info = bsp_get_version_info();

    g_mloader_images.op_state = MLOADER_LOAD_IDLE;

    wait_for_completion(&g_mloader_sysboot_start_complete);

    if (!bsp_need_loadmodem()) {
        mloader_print_err("update or charge mode, will not start modem\n");
        return 0;
    }
    __pm_stay_awake(&g_mloader_images.wake_lock);

#if (FEATURE_ON == FEATURE_DELAY_MODEM_INIT)
    mloader_print_err("modem rfile init start\n");
    if (modem_rfile_init()) {
        mloader_print_err("modem rfile init fail\n");
    }
#endif
    if (version_info != NULL && version_info->product_type == PRODUCT_PHONE && version_info->plat_type != PLAT_ASIC) {
        wait_for_completion(&g_mloader_sysboot_start_complete);
        bsp_rfile_get_load_mode(g_mloader_load_type);
    }
    mloader_modules_init();

    /* 系统启动后第一次加载lr */
    mloader_load_lr_img();

    mloader_images_task_loop();

    return 0;
}

static ssize_t modem_start_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int status = count;
    long modem_load = 0;

    if ((kstrtol(buf, 0, &modem_load) < 0)) {
        return -EINVAL;
    }

    if (modem_load >= MLOADER_BUTT) {
        bsp_info("fail to start modem, cmd = %ld\n", modem_load);
        return -EINVAL;
    }

    g_mloader_load_type = modem_load;

    complete(&g_mloader_sysboot_start_complete);

    mloader_print_info("modem_start write ok\n");

    return status;
}

static DEVICE_ATTR(modem_start, S_IWUSR, NULL, modem_start_store);
static DEVICE_ATTR(modem_image_patch_status, 0660, modem_imag_patch_status_show, modem_imag_patch_status_store);

static int mloader_get_dtb_info(device_node_s *dev_node, u32 id)
{
    u32 ret = 0;
    device_node_s *dts_node = NULL;

    if (!g_mloader_info.img[id].with_dts) {
        return (int)ret;
    }

    dts_node = bsp_dt_get_child_by_name(dev_node, "modem_dtb");
    if (dts_node == NULL) {
        mloader_print_err("balong modem load get dtb fail, parent image is %s\n", g_mloader_info.img[id].image_name);
        return (int)ret;
    }
    ret |= (u32)bsp_dt_property_read_string_index(dts_node, "file_name", 0,
        (const char **)&g_mloader_info.img[id].dtb.name);
    if (bsp_dt_property_read_string_index(dts_node, "dtb_ddr_name", 0,
        (const char **)&g_mloader_info.img[id].dtb.dtb_ddr_name)) {
        ret |= (u32)bsp_dt_property_read_u32_index(dts_node, "dtb_addr", 0, &g_mloader_info.img[id].dtb.run_addr);
        ret |= (u32)bsp_dt_property_read_u32_index(dts_node, "dtb_size", 0, &g_mloader_info.img[id].dtb.ddr_size);
    } else {
        g_mloader_info.img[id].dtb.run_addr =
            mdrv_mem_region_get(g_mloader_info.img[id].dtb.dtb_ddr_name, &(g_mloader_info.img[id].dtb.ddr_size));
    }
    ret |= (u32)bsp_dt_property_read_u32_index(dts_node, "core_id", 0, &g_mloader_info.img[id].dtb.core_id);

    return (int)ret;
}

static int mloader_get_dts(void)
{
    u32 ret = 0;
    int id = 0;
    u32 ddr_addr = 0;

    device_node_s *dev_node = NULL;
    device_node_s *child_node = NULL;

    dev_node = bsp_dt_find_compatible_node(NULL, NULL, "hisilicon,balong_mloader");
    if (dev_node == NULL) {
        mloader_print_err("fail to get mloader dts\n");
        return MLOADER_INTERFACE_ERROR;
    }
    g_mloader_info.image_num = bsp_dt_get_child_count(dev_node);
    if (!g_mloader_info.image_num) {
        mloader_print_err("no ccpu image found in mloader dts\n");
        return MLOADER_PARAM_ERROR;
    }
    g_mloader_info.img = (ccpu_img_info_s *)kzalloc(sizeof(ccpu_img_info_s) * g_mloader_info.image_num, GFP_KERNEL);
    if (g_mloader_info.img == NULL) {
        mloader_print_err("kzalloc fail, no memory to init load ccpu, size: 0x%lX\n",
            (unsigned long)sizeof(ccpu_img_info_s) * g_mloader_info.image_num);
        return MLOADER_NO_MEMORY;
    }

    bsp_dt_for_each_child_of_node(dev_node, child_node) {
        ret |= (u32)bsp_dt_property_read_string_index(child_node, "image_name", 0,
            (const char **)&g_mloader_info.img[id].image_name);

        ret |= (u32)bsp_dt_property_read_u32_index(child_node, "with_dts", 0, &g_mloader_info.img[id].with_dts);

        if (bsp_dt_property_read_string_index(child_node, "ddr_name", 0,
            (const char **)&g_mloader_info.img[id].ddr_name)) {
            ret |= (u32)bsp_dt_property_read_u32_index(child_node, "ddr_addr", 0, &ddr_addr);
            g_mloader_info.img[id].ddr_addr = ddr_addr;
            ret |= (u32)bsp_dt_property_read_u32_index(child_node, "ddr_size", 0, &g_mloader_info.img[id].ddr_size);
        } else {
            g_mloader_info.img[id].ddr_addr =
                mdrv_mem_region_get(g_mloader_info.img[id].ddr_name, &g_mloader_info.img[id].ddr_size);
        }

        ret |= (u32)mloader_get_dtb_info(child_node, id);
        id++;
    }

    if (ret) {
        mloader_print_err("balong modem load get dts fail, ret =%d\n", (int)ret);
    }

    return (int)ret;
}

int mloader_images_init(void)
{
    int ret;
    struct sched_param sch_para;

    osl_sem_init(0, &g_mloader_images.task_sem);

    INIT_LIST_HEAD(&g_mloader_images.load_notifier_list);

    wakeup_source_init(&g_mloader_images.wake_lock, "mloader_wakelock");

    bsp_mloader_load_image_init();

    g_mloader_addr.mloader_secboot_virt_addr_for_ccpu = kzalloc(PAGE_ALIGN(MLOADER_SECBOOT_BUFLEN), GFP_KERNEL);
    if (g_mloader_addr.mloader_secboot_virt_addr_for_ccpu == NULL) {
        mloader_print_err("fail to malloc secboot buffer\n");
        return -ENOMEM;
    }
    g_mloader_addr.mloader_secboot_phy_addr_for_ccpu = virt_to_phys(g_mloader_addr.mloader_secboot_virt_addr_for_ccpu);

    ret = mloader_teek_init();
    if (ret) {
        mloader_print_err("fail to init mloader CA, ret = %d\n", ret);
        return ret;
    }

    g_mloader_images.taskid = kthread_run(mloader_images_task, BSP_NULL, "mloader");
    if (IS_ERR(g_mloader_images.taskid)) {
        mloader_print_err("kthread_run fail, ret = %ld\n", PTR_ERR(g_mloader_images.taskid));
        return -ECHILD;
    }

    sch_para.sched_priority = 14U;
    ret = sched_setscheduler(g_mloader_images.taskid, SCHED_FIFO, &sch_para);
    if (ret) {
        mloader_print_err("sched_setscheduler fail\n");
        return ret;
    }

    return 0;
}

int mloader_driver_early_init(void)
{
    int ret;

    ret = mloader_get_dts();
    if (ret) {
        return ret;
    }

    mloader_debug_init();

    ret = mloader_images_init();
    if (ret) {
        mloader_print_err("fail to init modem loader task, ret = %d\n", ret);
        return ret;
    }

    mloader_print_err("mloader early init success\n");

    return ret;
}

int mloader_probe(struct platform_device *pdev)
{
    int ret;

    g_mloader_images.dev = &pdev->dev;
    dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(BITS_PER_LONG));

    ret = mloader_msg_init();
    if (ret) {
        mloader_print_err("mloader msg init fail, ret = %d\n", ret);
        return ret;
    }

    ret = device_create_file(&pdev->dev, &dev_attr_modem_start);
    if (ret) {
        mloader_print_err("fail to create modem start node, ret = %d\n", ret);
        return ret;
    }
    ret = device_create_file(&pdev->dev, &dev_attr_modem_image_patch_status);
    if (ret) {
        mloader_print_err("fail to create modem patch node, ret = %d\n", ret);
        return ret;
    }
    mloader_print_err("mloader init success\n");

    return ret;
}

static struct platform_device g_mloader_device = {
    .name = "balong_mloader",
    .id = -1,
};

struct platform_driver g_mloader_driver = {
    .probe      = mloader_probe,
    .driver     = {
        .name     = "balong_mloader",
        .owner    = THIS_MODULE,
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
};

int mloader_driver_init(void)
{
    int ret;

    ret = platform_device_register(&g_mloader_device);
    if (ret) {
        mloader_print_err("fail to register mloader device\n");
        return ret;
    }

    ret = platform_driver_register(&g_mloader_driver);
    if (ret) {
        platform_device_unregister(&g_mloader_device);
        mloader_print_err("fail to register mloader driver\n");
    }
    return ret;
}
#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(mloader_driver_early_init);
late_initcall(mloader_driver_init);
#endif
