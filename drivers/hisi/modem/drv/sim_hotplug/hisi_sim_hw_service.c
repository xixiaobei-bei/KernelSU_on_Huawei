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

#include "hisi_sim_hw_service.h"
#include "hisi_sim_hw_mgr.h"
#include "hisi_sim_hotplug.h"
#include "hisi_sim_io_mgr.h"
#include <linux/module.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/interrupt.h>
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/pm_wakeup.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include "securec.h"
#include <adrv.h>
#include <bsp_print.h>
#include <bsp_icc.h>
#include <product_config.h>


#undef THIS_MODU
#define THIS_MODU mod_simhotplug

struct hisi_sim_platform_info *g_simhp_info[MAXSIMHPNUM];

s32 handle_msg_from_sci(u32 channel_id, u32 len, void *context)
{
    sci_msg_data_s sci_msg;
    s32 read_len;
    int ret;
    struct hisi_sim_platform_info *info = context;
    if (len != sizeof(sci_msg_data_s)) {
        sim_log_err("wrong msg len(%d)\n", len);
        return -SIMHP_MSG_RECV_ERR;
    }
    read_len = bsp_icc_read(channel_id, (u8 *)&sci_msg, sizeof(sci_msg_data_s));
    if ((u32)read_len != sizeof(sci_msg_data_s)) {
        sim_log_err("readed len(%d) != expected len(%d)\n", read_len, len);
        return -SIMHP_MSG_RECV_ERR;
    }

    if (sci_msg.msg_type != SIM_MSG_LEGACY_TYPE) {
        sim_log_err("sim%d invalid msgtype %d.\n", info->sim_id, sci_msg.msg_type);
        return SIMHP_OK;
    }

    sim_log_err("sim_id %d, msg_type %d, msg_value %d, mux_sdsim: %d.\n", info->sim_id, sci_msg.msg_type,
        sci_msg.msg_value, info->mux_sdsim);
    if (info->sim_id == SIM0 || info->sim_id == SIM1) {
        ret = memcpy_s(&info->msgfromsci, sizeof(sci_msg_data_s), &sci_msg, sizeof(sci_msg_data_s));
        if (ret) {
            sim_log_err("memcpy failed.\n");
        }
        if (info->sim_sci_msg_wq != NULL) {
            queue_work(info->sim_sci_msg_wq, &info->sim_sci_msg_work);
        }
    } else {
        sim_log_err("wrong info simid 0x%x for info channel id 0x%x, actual channlid %d.", info->sim_id,
            info->channel_id, channel_id);
    }
    return SIMHP_OK;
}

int hisi_sim_inquire_msg_proc(struct hisi_sim_platform_info *info)
{
    int ret = 0;
    sci_msg_data_s sci_msg;
    switch (info->msgfromsci.msg_value) {
        case SIM_CARD_TYPE_INQUIRE:
            if (STATUS_SD2JTAG == get_card1_type()) {
                ret = SIM_CARD_TYPE_JTAG;
            } else {
                ret = SIM_CARD_TYPE_SIM;
            }
            break;

        case SIM_CARD_DET_WHILE_ESIM_INQUIRE:
            sim_log_err("no card det while esim\n");
            ret = SIM_CARD_NO_EXIST_WHILE_ESIM;
            break;

        default:
            sim_log_err("invalid inquire msg.\n");
            ret = SIM_INVALID_INQUIRE_MSG;
            break;
    }

    sci_msg.msg_type = SIM_MSG_INQUIRE_TYPE;
    sci_msg.msg_value = ret;
    ret = sci_send_msg(info, &sci_msg);
    if (ret) {
        sim_log_err("in inquire msg proc, sci_send_msg failed.\n");
    }
    return ret;
}

int hisi_sim_power_msg_proc(struct hisi_sim_platform_info *info)
{
    int ret = 0;
    sci_msg_data_s sci_msg;
    if (info->msgfromsci.msg_value < SIM_POWER_MSG_BUTT) {
        ret = hisi_sim_pmu_ctrl(info);
        sim_log_err("sim power msg.\n");
    } else {
        sim_log_err("invalid power msg.\n");
        ret = SIM_INVALID_POWER_MSG;
    }

    sci_msg.msg_type = SIM_MSG_POWER_TYPE;
    sci_msg.msg_value = ret;
    ret = sci_send_msg(info, &sci_msg);
    if (ret) {
        sim_log_err("in inquire msg proc, sci_send_msg failed.\n");
    }
    return ret;
}

int hisi_sim_notifier_msg_proc(struct hisi_sim_platform_info *info)
{
    int ret;
    sci_msg_data_s sci_msg;
    switch (info->msgfromsci.msg_value) {
        case SIM_DEACTIVE_NOTIFIER:
            ret = SIM_DEACTIVE_SUCC;
            sim_log_err("sim deactive msg.\n");
            break;

        default:
            sim_log_err("invalid notifier msg.\n");
            ret = SIM_INVALID_NOTIFIER_MSG;
            break;
    }

    sci_msg.msg_type = SIM_MSG_NOTIFIER_TYPE;
    sci_msg.msg_value = ret;
    ret = sci_send_msg(info, &sci_msg);
    if (ret) {
        sim_log_err("in notifier msg proc, sci_send_msg failed.\n");
    }
    return ret;
}

int hisi_sim_switch_msg_proc(struct hisi_sim_platform_info *info)
{
    int ret = 0;
    sci_msg_data_s sci_msg;
    switch (info->msgfromsci.msg_value) {
        case SIM_SET_ESIM_SWITCH:
        case SIM_SET_USIM_SWITCH:
            ret = SIM_ESIM_SWITCH_SUCC;
            sim_log_err("sim switch msg.\n");
            break;

        default:
            sim_log_err("invalid switch msg.\n");
            ret = SIM_INVALID_SWITCH_MSG;
            break;
    }

    sci_msg.msg_type = SIM_MSG_NOTIFIER_TYPE;
    sci_msg.msg_value = ret;

    ret = sci_send_msg(info, &sci_msg);
    if (ret) {
        sim_log_err("in switch msg proc, sci_send_msg failed.\n");
    }

    return ret;
}

int hisi_sim_legacy_msg_proc(struct hisi_sim_platform_info *info)
{
    int ret = 0;
    sci_msg_data_s sci_msg;
    switch (info->msgfromsci.msg_value) {
        case REQUEST_CARD_STATUS:
            ret = hisi_sim_get_card_type(info);
            break;
        case SIM1_IO_MUX_SIMIO_USIM_NANOSDIO_SD_REQUEST:
        case SIM1_IO_MUX_SIMIO_GPIO_NANOSDIO_USIM_REQUEST:
        case SIM1_IO_MUX_NANOSDIO_USIM_ONLY_REQUEST:
        case SIM1_IO_MUX_SIMIO_USIM_ONLY_REQUEST:
        case SIM1_IO_MUX_GPIO_DET_REQUEST:
        case SIM1_IO_MUX_IDLE_GPIO_REQUEST:
            ret = hisi_set_sim_io_mux(info);
            break;
        default:
            sim_log_err("invalid legacy msg.\n");
            ret = SIM_INVALID_LEGACY_MSG;
            break;
    }

    sci_msg.msg_type = SIM_MSG_LEGACY_TYPE;
    sci_msg.msg_value = ret;
    ret = sci_send_msg(info, &sci_msg);
    if (ret) {
        sim_log_err("in legacy msg proc, sci_send_msg failed.\n");
    }
    return ret;
}

static void sim_sci_msg_proc(struct work_struct *work)
{
    int ret = 0;
    sci_msg_data_s sci_msg;
    u32 conv_msg;
    struct hisi_sim_platform_info *info = container_of(work, struct hisi_sim_platform_info, sim_sci_msg_work);

    if (info->is_hisi_hw == 1) {
        switch (info->msgfromsci.msg_type) {
            case SIM_MSG_INQUIRE_TYPE:
                ret = hisi_sim_inquire_msg_proc(info);
                break;
            case SIM_MSG_NOTIFIER_TYPE:
                ret = hisi_sim_notifier_msg_proc(info);
                sim_log_err("sim%d invalid msg %d for hisi hw.\n", info->sim_id, info->msgfromsci);
                ret = SIM_DEACTIVE_SUCC;
                break;
            case SIM_MSG_POWER_TYPE:
                ret = hisi_sim_power_msg_proc(info);
                break;
            case SIM_MSG_LEGACY_TYPE:
                ret = hisi_sim_legacy_msg_proc(info);
                break;
            default:
                sim_log_err("sim%d invalid msg %d for hisi hw.\n", info->sim_id, info->msgfromsci);
                ret = -1;
                break;
        }
    } else {
        sci_msg.msg_type = info->msgfromsci.msg_type;
        conv_msg = info->msgfromsci.msg_type * SIM_HW_OPS_PER_TYPE + info->msgfromsci.msg_value;
        sim_log_err("hw_adp handle for sim_id %d with msg_type 0x%x.\n", info->sim_id, conv_msg);
        sci_msg.msg_value = SIM_HW_OP_RESULT_SUCC;
        sim_log_err("hw_adp return %d for sim_id %d, msg_type %d, msg_value %d.\n", ret, info->sim_id, sci_msg.msg_type,
            sci_msg.msg_value);
        ret = sci_send_msg(info, &sci_msg);
        if (ret) {
            sim_log_err("hwadp msg handler, sci_send_msg failed.\n");
        }
    }
}


int sci_send_msg(struct hisi_sim_platform_info *info, sci_msg_data_s *sci_msg)
{
    int ret;

    ret = bsp_icc_send(SIM_ICC_TO_MODEM, info->channel_id, (u8 *)sci_msg, sizeof(sci_msg_data_s));
    if (ret != sizeof(sci_msg_data_s)) {
        sim_log_err("bsp_icc_send failed.\n");
    } else {
        ret = 0;
    }
    return ret;
}


int hisi_sim_msg_init(struct hisi_sim_platform_info *info)
{
    int ret;
    if (info->sim_id == SIM0) {
        info->channel_id = SIM0_CHANNEL_ID;
    } else if (info->sim_id == SIM1) {
        info->channel_id = SIM1_CHANNEL_ID;
    } else {
        sim_log_err("sim%d invalid.\n", info->sim_id);
        return -EINVAL;
    }
    sim_log_err("sim%d channelid is 0x%x.\n", info->sim_id, info->channel_id);

    sim_log_err("bsp_icc_event_register.\n");
    ret = bsp_icc_event_register(info->channel_id, handle_msg_from_sci, info, NULL, NULL);
    if (ret != 0) {
        sim_log_err("bsp_icc_event_register failed, ret: %d\n", ret);
        return ret;
    }
    /* create sci msg workqueue */
    info->sim_sci_msg_wq = create_singlethread_workqueue("sim_sci_msg");
    if (info->sim_sci_msg_wq == NULL) {
        sim_log_err("sim_sci_msg_wq failed to create work queue.\n");
        ret = -EINVAL;
    } else {
        /*lint -e611 -esym(611,*)*/
        INIT_WORK(&info->sim_sci_msg_work, (void *)sim_sci_msg_proc);
        /*lint -e611 +esym(611,*)*/
    }
    return ret;
}

static int hisi_sim_hw_service_probe(struct spmi_device *pdev)
{
    struct device *dev = NULL;
    struct device_node *np = NULL;
    struct hisi_sim_platform_info *info = NULL;
    u32 sim_id = 0;
    u32 is_hisi_hw = 0;
    int ret;

    sim_log_info("probe sim_hw_service\n");

    if (pdev == NULL) {
        sim_log_err("pdev is NULL.\n");
        return -EINVAL;
    }

    dev = &pdev->dev;
    np = dev->of_node;

    info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
    if (info == NULL) {
        sim_log_err("failed to allocate memory.\n");
        return -ENOMEM;
    }

    ret = of_property_read_u32_array(np, "sim_id", &sim_id, 1);
    if (ret) {
        sim_log_err("no sim hotplug sim_id, so return.\n");
        return ret;
    }

    if (sim_id == SIM1 || sim_id == SIM0) {
        info->sim_id = sim_id;
    } else {
        sim_log_err("invalid sim id%d.\n", info->sim_id);
        return -EINVAL;
    }
    ret = of_property_read_u32_array(np, "is_hisi_hw", &is_hisi_hw, 1);
    if (ret) {
        sim_log_err("no is_hisi_hw defined, treat as hisi_hw.\n");
        is_hisi_hw = 1;
    }
    info->is_hisi_hw = is_hisi_hw;
    g_simhp_info[info->sim_id] = info;
    info->spmidev = dev;
    sim_log_err("info init start for simid %d.\n", info->sim_id);
    (void)hisi_sim_msg_init(info);
    (void)hisi_sim_hw_mgr_init(info);
    (void)hisi_sim_hotplug_init(info, pdev);

    dev_set_drvdata(&pdev->dev, info);
    sim_log_err("info init done for simid %d.\n", info->sim_id);

    return 0;
}

static int hisi_sim_hw_service_remove(struct spmi_device *pdev)
{
    struct hisi_sim_platform_info *info = NULL;

    sim_log_err("simhotplug removing.\n");
    info = dev_get_drvdata(&pdev->dev);
    if (info == NULL) {
        sim_log_err("dev_get_drvdata is NULL.\n");
        return -1;
    }

    mutex_destroy(&info->sim_hotplug_lock);
    wakeup_source_trash(&info->sim_hotplug_wklock);

    if (info->sim_hotplug_det_wq != NULL) {
        destroy_workqueue(info->sim_hotplug_det_wq);
    }
    if (info->sim_hotplug_hpd_wq != NULL) {
        destroy_workqueue(info->sim_hotplug_hpd_wq);
    }
    if (info->sim_sci_msg_wq != NULL) {
        destroy_workqueue(info->sim_sci_msg_wq);
    }

    if (info->send_msg_to_cp == 1 && info->sim_debounce_delay_wq) {
        sim_log_info("remove delay work for sim%d\n", info->sim_id);
        cancel_delayed_work(&info->sim_debounce_delay_work);
        flush_workqueue(info->sim_debounce_delay_wq);
        destroy_workqueue(info->sim_debounce_delay_wq);
    }

    if (info->sim_id == SIM1) {
        remove_simhp_node(info->sim_id);
    }

    g_simhp_info[info->sim_id] = NULL;
    dev_set_drvdata(&pdev->dev, NULL);

    return 0;
}

#ifdef CONFIG_PM
static int hisi_sim_hw_service_suspend(struct spmi_device *pdev, pm_message_t state)
{
    struct hisi_sim_platform_info *info = NULL;

    info = dev_get_drvdata(&pdev->dev);
    if (info == NULL) {
        sim_log_err("dev_get_drvdata is NULL.\n");
        return -1;
    }

    if (!mutex_trylock(&info->sim_hotplug_lock)) {
        sim_log_err("mutex_trylock failed.\n");
        return -EAGAIN;
    }

    return 0;
}

static int hisi_sim_hw_service_resume(struct spmi_device *pdev)
{
    struct hisi_sim_platform_info *info = NULL;

    info = dev_get_drvdata(&pdev->dev);
    if (info == NULL) {
        sim_log_err("dev_get_drvdata is NULL.\n");
        return -1;
    }

    /*lint -e455 -esym(455,*)*/
    mutex_unlock(&(info->sim_hotplug_lock));
    /*lint -e455 +esym(455,*)*/
    return 0;
}
#endif

struct hisi_sim_platform_info *hisi_sim_get_platinfo(unsigned int sim_id)
{
    if (sim_id < MAXSIMHPNUM) {
        return g_simhp_info[sim_id];
    } else {
        sim_log_err("wrong simid while get platinfo.\n");
        return NULL;
    }
}

static struct of_device_id g_hisi_sim_service_of_match[] = {
    {
        .compatible = HISILICON_SIM_HOTPLUG0,
    },
    {
        .compatible = HISILICON_SIM_HOTPLUG1,
    },
    {},
};

MODULE_DEVICE_TABLE(of, g_hisi_sim_service_of_match);

static struct spmi_driver g_hisi_sim_service_driver = {
    .probe = hisi_sim_hw_service_probe,
    .remove = hisi_sim_hw_service_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "hisi_sim_service",
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
        .of_match_table = of_match_ptr(g_hisi_sim_service_of_match),
    },
#ifdef CONFIG_PM
    .suspend = hisi_sim_hw_service_suspend,
    .resume = hisi_sim_hw_service_resume,
#endif
};

int hisi_sim_hw_service_init(void)
{
    return spmi_driver_register(&g_hisi_sim_service_driver);
}

static void __exit hisi_sim_hw_service_exit(void)
{
    spmi_driver_unregister(&g_hisi_sim_service_driver);
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
late_initcall(hisi_sim_hw_service_init);
module_exit(hisi_sim_hw_service_exit);
#endif
MODULE_DESCRIPTION("Sim HW Service driver");
MODULE_LICENSE("GPL v2");
