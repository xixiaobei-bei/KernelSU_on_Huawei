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

#include "hisi_sim_hotplug.h"
#include "securec.h"
#include <adrv.h>
#include <bsp_print.h>
#include <bsp_icc.h>
#include <product_config.h>
#include <bsp_shared_ddr.h>
#include "hisi_sim_io_mgr.h"
#include "hisi_sim_hw_service.h"

static const char *g_sim_deb_work_name[] = {
    "sim0_debounce_check",
    "sim1_debounce_check"
};

struct {
    u8 sim0_det_normal_direction;
    u8 sim0_pluged;
    u8 sim1_det_normal_direction;
    u8 sim1_pluged;
    u32 sim0_leave_cnt;
    u32 sim0_out_cnt;
    u32 sim0_in_cnt;
    u32 sim0_det_cnt;
    u32 sim1_leave_cnt;
    u32 sim1_out_cnt;
    u32 sim1_in_cnt;
    u32 sim1_det_cnt;
} g_sim_plug_state;

struct kobject *g_sim_kobj = NULL;
static int g_sim0_state = SIM_HOTPLUG_STATE_UNKNOWN;
static int g_sim1_state = SIM_HOTPLUG_STATE_UNKNOWN;
static int g_sim2_state = SIM_HOTPLUG_STATE_UNKNOWN;
static u32 g_sim0_last_det_cnt;

/* static funciton declare */
static char *simplug_to_string(u32 plug);
static char *direction_to_string(u32 direction);
static char *card_tray_style_to_string(u32 card_tray);
u8 get_card1_status(struct hisi_sim_platform_info *info);
int sim_hotplug_sdmux_init(struct hisi_sim_platform_info *info);
static int g_sd2jtag_enable = 0;
/* funciton implement */
static char *simplug_to_string(u32 plug)
{
    switch (plug) {
        case SIM_CARD_OUT:
            return "SIM_CARD_OUT";

        case SIM_CARD_IN:
            return "SIM_CARD_IN";

        default:
            return "SIM_PLUG_ERROR";
    }
}

static char *direction_to_string(u32 direction)
{
    switch (direction) {
        case NORMAL_CLOSE:
            return "NORMAL_CLOSE";

        case NORMAL_OPEN:
            return "NORMAL_OPEN";

        case NORMAL_BUTT:
        default:
            return "NORMAL_BUTT";
    }
}

static char *card_tray_style_to_string(u32 card_tray)
{
    switch (card_tray) {
        case SINGLE_CARD_IN_ONE_TRAY:
            return "SINGLE_CARD_IN_ONE_TRAY";

        case TWO_CARDS_IN_ONE_TRAY:
            return "TWO_CARDS_IN_ONE_TRAY";

        case CARD_TRAY_BUTT:
        default:
            return "CARD_TRAY_STYLE_UNKNOWN";
    }
}

static void sim_pmu_hpd_read(struct hisi_sim_platform_info *info)
{
    u32 pmu_status1 = 0;
    u32 pmu_irq = 0;
    u32 pmu_irq_mask = 0;
    u32 pmu_sim_ctrl = 0;
    u32 pmu_sim_deb_ctrl = 0;
    u32 pmu_ldo11_onoff = 0;
    u32 pmu_ldo11_vset = 0;
    u32 pmu_ldo12_onoff = 0;
    u32 pmu_ldo12_vset = 0;

    if (info->pmic_id == SIM_MAIN_PMU) {
        pmu_status1 = hisi_pmic_reg_read(info->pmu_status1_address);
        pmu_irq = hisi_pmic_reg_read(info->pmu_irq_address);
        pmu_irq_mask = hisi_pmic_reg_read(info->pmu_irq_mask_address);
        pmu_sim_ctrl = hisi_pmic_reg_read(info->pmu_sim_ctrl_address);
        pmu_sim_deb_ctrl = hisi_pmic_reg_read(info->pmu_sim_deb_ctrl_address);
        pmu_ldo11_onoff = hisi_pmic_reg_read(info->pmu_ldo11_onoff_eco_address);
        pmu_ldo11_vset = hisi_pmic_reg_read(info->pmu_ldo11_vset_address);
        pmu_ldo12_onoff = hisi_pmic_reg_read(info->pmu_ldo12_onoff_eco_address);
        pmu_ldo12_vset = hisi_pmic_reg_read(info->pmu_ldo12_vset_address);
    } else if (info->pmic_id == SIM_SUB_PMU) {
        pmu_status1 = hisi_sub_pmic_reg_read(info->pmu_status1_address);
        pmu_irq = hisi_sub_pmic_reg_read(info->pmu_irq_address);
        pmu_irq_mask = hisi_sub_pmic_reg_read(info->pmu_irq_mask_address);
        pmu_sim_ctrl = hisi_sub_pmic_reg_read(info->pmu_sim_ctrl_address);
        pmu_sim_deb_ctrl = hisi_sub_pmic_reg_read(info->pmu_sim_deb_ctrl_address);
        pmu_ldo11_onoff = hisi_sub_pmic_reg_read(info->pmu_ldo11_onoff_eco_address);
        pmu_ldo11_vset = hisi_sub_pmic_reg_read(info->pmu_ldo11_vset_address);
        pmu_ldo12_onoff = hisi_sub_pmic_reg_read(info->pmu_ldo12_onoff_eco_address);
        pmu_ldo12_vset = hisi_sub_pmic_reg_read(info->pmu_ldo12_vset_address);
    }

    sim_log_info("pmu_status1: 0x%02X, pmu_irq: 0x%02X, pmu_irq_mask: 0x%02X, "
        "pmu_sim_ctrl: 0x%02X, pmu_sim_deb_ctrl: 0x%02X\n",
        pmu_status1, pmu_irq, pmu_irq_mask, pmu_sim_ctrl, pmu_sim_deb_ctrl);

    sim_log_info("pmu_ldo11_onoff: 0x%02X, pmu_ldo11_vset: 0x%02X, "
        "pmu_ldo12_onoff: 0x%02X, pmu_ldo12_vset: 0x%02X\n",
        pmu_ldo11_onoff, pmu_ldo11_vset, pmu_ldo12_onoff, pmu_ldo12_vset);
}

static void sim1_ldo16_ctrl(struct hisi_sim_platform_info *info)
{
    if (info->hpd1_used == 1) {
        hisi_pmic_reg_write(info->pmu_sim_ctrl_address + 1, HI6421_IRQ_LDO16_ENABLE);
    } else {
        hisi_pmic_reg_write(info->pmu_sim_ctrl_address + 1, HI6421_IRQ_LDO16_HPD0_ENABLE);
    }
}

static void sim_pmu_hpd_write(struct hisi_sim_platform_info *info)
{
    if (info == NULL) {
        sim_log_err("info is null.\n");
        return;
    }

    if (info->hpd_interrupt_to_close_ldo == 0xFF || info->hpd_interrupt_to_close_ldo == 0x00) {
        sim_log_err("hpd_interrupt_to_close_ldo: 0x%02X\n", info->hpd_interrupt_to_close_ldo);
        return;
    }
    // set debounce waiting time
    hisi_pmic_reg_write(info->pmu_sim_deb_ctrl_address, info->hpd_debounce_wait_time);

    // set close ldo when interrupt happened
    if (info->sim_id == SIM0) {
        hisi_pmic_reg_write(info->pmu_sim_ctrl_address, info->hpd_interrupt_to_close_ldo);
        return;
    }
    if (info->sim_id == SIM1) {
        if (info->mux_sdsim != 1) {
            hisi_pmic_reg_write(info->pmu_sim_ctrl_address, info->hpd_interrupt_to_close_ldo);
            return;
        }
        if (g_sd2jtag_enable) {
            sim_log_err("HIMNTN_SD2JTAG is 1\n");
            // disable ldo12, ldo16
            hisi_pmic_reg_write(info->pmu_sim_ctrl_address, HI6421_IRQ_LDO_DISABLE);
            hisi_pmic_reg_write(info->pmu_sim_ctrl_address + 1, HI6421_IRQ_LDO_DISABLE);
        } else {
            sim_log_err("HIMNTN_SD2JTAG is 0\n");
            // enable ldo12, ldo16, means falling edge hardware close ldo
            if (info->pmu_ldo12_pull_down == 1) {
                hisi_pmic_reg_write(info->pmu_sim_ctrl_address, info->hpd_interrupt_to_close_ldo);
            } else {
                hisi_pmic_reg_write(info->pmu_sim_ctrl_address, HI6421_IRQ_LDO11_ENABLE);
            }
            sim1_ldo16_ctrl(info);
        }
    }
}

static int sim_read_pmu_dts(struct hisi_sim_platform_info *info, struct device_node *np)
{
    int ret;
    u32 hpd_interrupt_to_close_ldo = 0;
    u32 hpd_debounce_wait_time = 0;
    u32 pmu_status1_loc = 0;
    u32 pmu_irq_loc = 0;
    u32 pmu_irq_mask_loc = 0;
    u32 pmu_sim_ctrl_loc = 0;
    u32 pmu_sim_deb_ctrl_loc = 0;
    u32 pmu_ldo11_onoff_eco_loc = 0;
    u32 pmu_ldo11_vset_loc = 0;

    u32 pmu_ldo12_onoff_eco_loc = 0;
    u32 pmu_ldo12_vset_loc = 0;

    u32 pmu_ldo12_pull_down = 1;
    u32 pmic_id = 0;
    u32 hpd1_used = 1;

    ret = of_property_read_u32(np, "pmic_id", &pmic_id);
    if (ret < 0) {
        sim_log_err("failed to read pmic_id.\n");
        info->pmic_id = SIM_MAIN_PMU;
    } else {
        info->pmic_id = pmic_id;
        sim_log_err("pmic id is to %d.\n", info->pmic_id);
    }

    ret = of_property_read_u32(np, "hpd_interrupt_to_close_ldo", &hpd_interrupt_to_close_ldo);
    if (ret < 0) {
        sim_log_err("failed to read hpd_interrupt_to_close_ldo.\n");
        return ret;
    }
    info->hpd_interrupt_to_close_ldo = hpd_interrupt_to_close_ldo;

    ret = of_property_read_u32(np, "hpd_debounce_wait_time", &hpd_debounce_wait_time);
    if (ret < 0) {
        sim_log_err("failed to read hpd_debounce_wait_time.\n");
        return ret;
    }
    info->hpd_debounce_wait_time = hpd_debounce_wait_time;

    sim_log_info("hpd_interrupt_to_close_ldo: 0x%02X, hpd_debounce_wait_time: 0x%02X\n", hpd_interrupt_to_close_ldo,
        hpd_debounce_wait_time);

    ret = of_property_read_u32(np, "pmu_status1_address", &pmu_status1_loc);
    if (ret < 0) {
        sim_log_err("failed to read pmu_status1_loc.\n");
        return ret;
    }
    info->pmu_status1_address = pmu_status1_loc;

    ret = of_property_read_u32(np, "pmu_irq_address", &pmu_irq_loc);
    if (ret < 0) {
        sim_log_err("failed to read pmu_irq_loc.\n");
        return ret;
    }
    info->pmu_irq_address = pmu_irq_loc;

    ret = of_property_read_u32(np, "pmu_irq_mask_address", &pmu_irq_mask_loc);
    if (ret < 0) {
        sim_log_err("failed to read pmu_irq_mask_loc.\n");
        return ret;
    }
    info->pmu_irq_mask_address = pmu_irq_mask_loc;

    ret = of_property_read_u32(np, "pmu_sim_ctrl_address", &pmu_sim_ctrl_loc);
    if (ret < 0) {
        sim_log_err("failed to read pmu_sim_ctrl_loc.\n");
        return ret;
    }
    info->pmu_sim_ctrl_address = pmu_sim_ctrl_loc;

    ret = of_property_read_u32(np, "pmu_sim_deb_ctrl_address", &pmu_sim_deb_ctrl_loc);
    if (ret < 0) {
        sim_log_err("failed to read pmu_sim_deb_ctrl_loc.\n");
        return ret;
    }
    info->pmu_sim_deb_ctrl_address = pmu_sim_deb_ctrl_loc;

    ret = of_property_read_u32(np, "pmu_ldo11_onoff_eco_address", &pmu_ldo11_onoff_eco_loc);
    if (ret < 0) {
        sim_log_err("failed to read pmu_ldo11_onoff_eco_loc.\n");
        return ret;
    }
    info->pmu_ldo11_onoff_eco_address = pmu_ldo11_onoff_eco_loc;

    ret = of_property_read_u32(np, "pmu_ldo11_vset_address", &pmu_ldo11_vset_loc);
    if (ret < 0) {
        sim_log_err("failed to read pmu_ldo11_vset_loc.\n");
        return ret;
    }
    info->pmu_ldo11_vset_address = pmu_ldo11_vset_loc;

    ret = of_property_read_u32(np, "pmu_ldo12_onoff_eco_address", &pmu_ldo12_onoff_eco_loc);
    if (ret < 0) {
        sim_log_err("failed to read pmu_ldo12_onoff_eco_loc.\n");
        return ret;
    }
    info->pmu_ldo12_onoff_eco_address = pmu_ldo12_onoff_eco_loc;

    ret = of_property_read_u32(np, "pmu_ldo12_vset_address", &pmu_ldo12_vset_loc);
    if (ret < 0) {
        sim_log_err("failed to read pmu_ldo12_vset_loc.\n");
        return ret;
    }
    info->pmu_ldo12_vset_address = pmu_ldo12_vset_loc;

    ret = of_property_read_u32(np, "hpd1_used", &hpd1_used);
    if (ret < 0) {
        sim_log_err("sim0 or old platform use hpd1.\n");
    }
    info->hpd1_used = hpd1_used;

    /* 为了区分v4和v4之前的版本，默认配置为1，v4分支以后由终端定制DTS,根据DTS实际情况赋值 */
    ret = of_property_read_u32(np, "pmu_ldo12_pull_down", &pmu_ldo12_pull_down);
    if (ret < 0) {
        sim_log_info("read pmu_ldo12_pull_down fail, ldo12 pull down default.\n");
        info->pmu_ldo12_pull_down = 1;
    } else {
        sim_log_info("read pmu_ldo12_pull_down use dts value 0x%x\n", pmu_ldo12_pull_down);
        info->pmu_ldo12_pull_down = pmu_ldo12_pull_down;
    }
    /* 最后一个分支可能读不到对应的dts，不判断ret，能走到最后说明前面已经返回成功 */
    sim_log_info("pmu_status1_loc: 0x%02X, pmu_irq_loc: 0x%02X, pmu_irq_mask_loc: 0x%02X, "
        "pmu_sim_ctrl_loc: 0x%02X, pmu_sim_deb_ctrl_loc: 0x%02X\n",
        pmu_status1_loc, pmu_irq_loc, pmu_irq_mask_loc, pmu_sim_ctrl_loc, pmu_sim_deb_ctrl_loc);

    sim_log_info("pmu_ldo11_onoff_eco_loc: 0x%02X, pmu_ldo11_vset_loc: 0x%02X, "
        "pmu_ldo12_onoff_eco_loc: 0x%02X, pmu_ldo12_vset_loc: 0x%02X\n",
        pmu_ldo11_onoff_eco_loc, pmu_ldo11_vset_loc, pmu_ldo12_onoff_eco_loc, pmu_ldo12_vset_loc);

    sim_log_info("pmu_ldo12_pull_down: 0x%02X", info->pmu_ldo12_pull_down);

    return 0;
}

static int sim_pmu_hpd_init(struct hisi_sim_platform_info *info, struct device_node *np)
{
    int ret;

    sim_log_info("hpd initializing.\n");

    ret = sim_read_pmu_dts(info, np);
    sim_pmu_hpd_write(info);
    sim_pmu_hpd_read(info);

    return ret;
}

static ssize_t sim_plug_state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int mret;
    mret = memcpy_s(buf, sizeof(g_sim_plug_state), &g_sim_plug_state, sizeof(g_sim_plug_state));
    if (mret) {
        sim_log_info("memcpy failed with ret%d.\n", mret);
    }
    return (ssize_t)sizeof(g_sim_plug_state);
}

static ssize_t sim0_hotplug_state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", g_sim0_state);
}

static ssize_t sim0_hotplug_state_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf,
    size_t count)
{
    int val = 0;

    if (kstrtoint(buf, 10, &val)) { // 10 for decimal format
        sim_log_err("EINVAL!\n");
        return -EINVAL;
    }

    sim_log_info("old sim0_state: %d, val: %d\n", g_sim0_state, val);

    g_sim0_state = val;

    return (ssize_t)count;
}

static ssize_t sim1_hotplug_state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", g_sim1_state);
}

static ssize_t sim1_hotplug_state_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf,
    size_t count)
{
    int val = 0;

    if (kstrtoint(buf, 10, &val)) { // 10 for decimal format
        sim_log_err("EINVAL!\n");
        return -EINVAL;
    }

    sim_log_info("old sim1_state: %d, val: %d\n", g_sim1_state, val);

    g_sim1_state = val;

    return (ssize_t)count;
}

static ssize_t sim2_hotplug_state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", g_sim2_state);
}

static ssize_t sim2_hotplug_state_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf,
    size_t count)
{
    int val = 0;

    if (kstrtoint(buf, 10, &val)) { // 10 for decimal format
        sim_log_err("EINVAL!\n");
        return -EINVAL;
    }

    sim_log_info("old sim2_state: %d, val: %d\n", g_sim2_state, val);

    g_sim2_state = val;

    return (ssize_t)count;
}

#define SIM_KOBJ_FILE_ATTR 0664
static struct kobj_attribute g_sim0_attribute =
    __ATTR(sim0_hotplug_state, SIM_KOBJ_FILE_ATTR, sim0_hotplug_state_show, sim0_hotplug_state_store);

static struct kobj_attribute g_sim1_attribute =
    __ATTR(sim1_hotplug_state, SIM_KOBJ_FILE_ATTR, sim1_hotplug_state_show, sim1_hotplug_state_store);

static struct kobj_attribute g_sim2_attribute =
    __ATTR(sim2_hotplug_state, SIM_KOBJ_FILE_ATTR, sim2_hotplug_state_show, sim2_hotplug_state_store);

static struct kobj_attribute g_sim_plug_state_attribute = __ATTR(sim_hotplug_state, SIM_KOBJ_FILE_ATTR,
    sim_plug_state_show, NULL);

static struct attribute *g_sim_attrs[] = { &g_sim0_attribute.attr, &g_sim1_attribute.attr, &g_sim2_attribute.attr,
                                           &g_sim_plug_state_attribute.attr, NULL };

static struct attribute_group g_sim_attr_group = {
    .attrs = g_sim_attrs
};

static int sys_add_sim_node(void)
{
    int retval;
    sim_log_info("adding sim sys node.\n");

    /* wait for kernel_kobj node ready: */
    while (kernel_kobj == NULL) {
        msleep(1000); // delay 1000 ms
    }

    /* Create kobject named "sim",located under /sys/kernel/ */
    g_sim_kobj = kobject_create_and_add("sim", kernel_kobj);
    if (g_sim_kobj == NULL) {
        return -ENOMEM;
    }

    /* Create the files associated with this kobject */
    retval = sysfs_create_group(g_sim_kobj, &g_sim_attr_group);
    if (retval) {
        kobject_put(g_sim_kobj);
    }

    sim_log_info("retval: %d\n", retval);
    return retval;
}

static void update_sim_state_info(int sim_id, int sim_pluged)
{
    if (sim_id == SIM0) {
        if (sim_pluged == SIM_CARD_IN) {
            g_sim0_state = SIM_HOTPLUG_STATE_IN;
        } else {
            g_sim0_state = SIM_HOTPLUG_STATE_CLEAR;
        }
    } else if (sim_id == SIM1) {
        if (sim_pluged == SIM_CARD_IN) {
            g_sim1_state = SIM_HOTPLUG_STATE_IN;
        } else {
            g_sim1_state = SIM_HOTPLUG_STATE_CLEAR;
        }
    } else {
        sim_log_err("invalid sim_id: %d\n", sim_id);
    }
}
static void update_sim0_hotplug_count(u32 sim_pluged)
{
    switch (sim_pluged) {
        case SIM_HPD_LEAVE:
            if (g_sim_plug_state.sim0_leave_cnt < MAX_CNT) {
                g_sim_plug_state.sim0_leave_cnt++;
            }
            break;

        case SIM_CARD_OUT:
            if (g_sim_plug_state.sim0_out_cnt < MAX_CNT) {
                g_sim_plug_state.sim0_out_cnt++;
            }
            break;

        case SIM_CARD_IN:
            if (g_sim_plug_state.sim0_in_cnt < MAX_CNT) {
                g_sim_plug_state.sim0_in_cnt++;
            }
            break;

        case SIM_CARD_DET:
            if (g_sim_plug_state.sim0_det_cnt < MAX_CNT) {
                g_sim_plug_state.sim0_det_cnt++;
            }
            break;

        default:
            break;
    }
}

static void update_sim1_hotplug_count(u32 sim_pluged)
{
    switch (sim_pluged) {
        case SIM_HPD_LEAVE:
            if (g_sim_plug_state.sim1_leave_cnt < MAX_CNT) {
                g_sim_plug_state.sim1_leave_cnt++;
            }
            break;

        case SIM_CARD_OUT:
            if (g_sim_plug_state.sim1_out_cnt < MAX_CNT) {
                g_sim_plug_state.sim1_out_cnt++;
            }
            break;

        case SIM_CARD_IN:
            if (g_sim_plug_state.sim1_in_cnt < MAX_CNT) {
                g_sim_plug_state.sim1_in_cnt++;
            }
            break;

        case SIM_CARD_DET:
            if (g_sim_plug_state.sim1_det_cnt < MAX_CNT) {
                g_sim_plug_state.sim1_det_cnt++;
            }
            break;

        default:
            break;
    }
}

static void update_sim_hotplug_count(u32 sim_id, u32 sim_pluged)
{
    if (sim_id == SIM0) {
        update_sim0_hotplug_count(sim_pluged);
    } else if (sim_id == SIM1) {
        update_sim1_hotplug_count(sim_pluged);
    } else {
        sim_log_err("invalid sim_id: %d\n", sim_id);
    }
}

static void save_sim_status(struct hisi_sim_platform_info *info, u32 sim_id, u32 sim_pluged)
{
    if (info->sim_id == SIM0) {
        g_sim_plug_state.sim0_pluged = (u8)sim_pluged;
    } else if (info->sim_id == SIM1) {
        g_sim_plug_state.sim1_pluged = (u8)sim_pluged;
    } else {
        sim_log_err(" save sim%d failed!\n", info->sim_id);
    }

    sim_log_info("sim%d, sim_pluged: %s\n", sim_id, simplug_to_string(sim_pluged));
}

static int sim_set_active(struct hisi_sim_platform_info *info)
{
    sim_log_info("sim%d, sim_pluged: %s\n", info->sim_id, simplug_to_string(info->sim_pluged));
    return 0;
}

static int sim_set_inactive(struct hisi_sim_platform_info *info)
{
    sim_log_info("%s: sim%d, sim_pluged: %s\n", __func__, info->sim_id, simplug_to_string(info->sim_pluged));
    return 0;
}

void send_det_msg_to_core(struct hisi_sim_platform_info *info, u32 channel_id, u32 sim_pluged)
{
    int ret = 0;
    u8 sim1_type = STATUS_NO_CARD;
    sci_msg_data_s sci_msg;
    sci_msg.msg_type = SIM_MSG_HOTPLUG_TYPE;
    sci_msg.msg_value = sim_pluged;

    if (info->send_msg_to_cp == 1) {
        if (info->mux_sdsim == 0) { // if mux_sdsim, report anyway
            sim_log_info("sim%d, sim_pluged = %s, sci_send_msg to CP.\n", info->sim_id, simplug_to_string(sim_pluged));
            ret = sci_send_msg(info, &sci_msg);
            if (ret) {
                sim_log_err("sim%d, sci_send_msg failed.\n", info->sim_id);
            }
            return;
        }
        if (sim_pluged == SIM_CARD_OUT && info->sim_id == SIM1)
            (void)sd_sim_detect_run(NULL, STATUS_PLUG_OUT, MODULE_SIM, 0);
        else if (sim_pluged == SIM_CARD_IN && info->sim_id == SIM1)
            (void)sd_sim_detect_run(NULL, STATUS_PLUG_IN, MODULE_SIM, 0);

        sim1_type = get_card1_status(info);

        if (sim_pluged == SIM_CARD_OUT && info->sim_id == SIM1) { // report out directly
            sim_log_info("sim%d, sim_pluged = %s, sci_send_msg to CP.\n", info->sim_id,
                simplug_to_string(sim_pluged));
            ret = sci_send_msg(info, &sci_msg);
            if (ret) {
                sim_log_err("sim%d, sci_send_msg failed.\n", info->sim_id);
            }
            return;
        }

        if (sim1_type == STATUS_SIM) { // if sim, report in directly
            sim_log_info("sim%d, sim_pluged = %s, sci_send_msg to CP.\n", info->sim_id,
                simplug_to_string(sim_pluged));
            ret = sci_send_msg(info, &sci_msg);
            if (ret) {
                sim_log_err("sim%d, sci_send_msg failed.\n", info->sim_id);
            }
            return;
        }
    } else {
        sim_log_info("send_msg_to_cp is not 1, so not sci_send_msg to CP.\n");
    }
}

static void hisi_sim_det_msg_to_ccore(struct hisi_sim_platform_info *info, u32 sim_pluged)
{
    int det_gpio_level;

    if (info == NULL) {
        sim_log_err("info is null.\n");
        return;
    }

    if (sim_pluged != SIM_CARD_OUT && sim_pluged != SIM_CARD_IN) {
        sim_log_err("sim_pluged is invalid and it is %s.\n", simplug_to_string(sim_pluged));
        return;
    }

    det_gpio_level = gpio_get_value(info->det_gpio);
    sim_log_info("sim%d, old_det_gpio_level: %d, det_gpio_level: %d\n", info->sim_id, info->old_det_gpio_level,
        det_gpio_level);
    if (det_gpio_level == info->old_det_gpio_level) {
        sim_log_info("det_gpio_level is same with old_det_gpio_level, so return.\n");
        return;
    }
    info->old_det_gpio_level = det_gpio_level;

    if (info->factory_send_msg_to_cp == 0) {
        sim_log_err("factory_send_msg_to_cp is 0, so return.\n");
        return;
    }

    update_sim_state_info(info->sim_id, sim_pluged);

    if ((g_sim_plug_state.sim0_det_cnt - g_sim0_last_det_cnt) > info->det_debounce_diff_max) {
        sim_log_err("sim%d, beyond det_debounce_diff_max, not send_det_msg_to_core\n", info->sim_id);
        return;
    }

    send_det_msg_to_core(info, info->channel_id, sim_pluged);
}

static void hisi_sim_hpd_msg_to_ccore(struct hisi_sim_platform_info *info)
{
    s32 ret;
    sci_msg_data_s sci_msg;
    sci_msg.msg_type = SIM_MSG_HOTPLUG_TYPE;
    sci_msg.msg_value = SIM_HPD_LEAVE;

    if (info == NULL) {
        sim_log_err("info is null.\n");
        return;
    }

    update_sim_hotplug_count(info->sim_id, SIM_HPD_LEAVE);

    sim_log_info("sim%d, SIM_HPD_LEAVE sci_send_msg to CP.\n", info->sim_id);
    ret = sci_send_msg(info, &sci_msg);
    if (ret) {
        sim_log_err("sim%d, sci_send_msg failed.\n", info->sim_id);
    }
}

static void det_irq_set_sim_insert(struct hisi_sim_platform_info *info)
{
    info->sim_pluged = SIM_CARD_IN;

    sim_log_info("sim%d, SIM_CARD_IN\n", info->sim_id);
    save_sim_status(info, info->sim_id, info->sim_pluged);
    update_sim_hotplug_count(info->sim_id, info->sim_pluged);
    sim_set_active(info);
    hisi_sim_det_msg_to_ccore(info, SIM_CARD_IN);
}

static void det_irq_set_sim_remove(struct hisi_sim_platform_info *info)
{
    info->sim_pluged = SIM_CARD_OUT;

    sim_log_info("sim%d, SIM_CARD_OUT\n", info->sim_id);
    save_sim_status(info, info->sim_id, info->sim_pluged);
    update_sim_hotplug_count(info->sim_id, info->sim_pluged);
    sim_set_inactive(info);
    hisi_sim_det_msg_to_ccore(info, SIM_CARD_OUT);
}

static void sim_manage_sim_state(int det_gpio_level, struct hisi_sim_platform_info *info)
{
    sim_log_info("sim%d, old_det_gpio_level: %d, det_gpio_level: %d\n", info->sim_id, info->old_det_gpio_level,
        det_gpio_level);

    if (det_gpio_level == 0) {
        if (info->det_normal_direction == NORMAL_OPEN && info->sim_pluged == SIM_CARD_OUT) {
            det_irq_set_sim_insert(info);
        } else if (info->det_normal_direction == NORMAL_CLOSE && info->sim_pluged == SIM_CARD_IN) {
            det_irq_set_sim_remove(info);
        }
    } else {
        if (info->det_normal_direction == NORMAL_OPEN && info->sim_pluged == SIM_CARD_IN) {
            det_irq_set_sim_remove(info);
        } else if (info->det_normal_direction == NORMAL_CLOSE && info->sim_pluged == SIM_CARD_OUT) {
            det_irq_set_sim_insert(info);
        }
    }
}

static void inquiry_sim_det_irq_reg(struct work_struct *work)
{
    struct hisi_sim_platform_info *info = container_of(work, struct hisi_sim_platform_info, sim_hotplug_det_work);
    int det_gpio_level;
    u32 debounce_wait_time = 0;

    det_gpio_level = gpio_get_value(info->det_gpio);
    sim_log_info("sim%d, old sim_pluged: %s, det_gpio_level: %d\n", info->sim_id, simplug_to_string(info->sim_pluged),
        det_gpio_level);

    if (info->send_msg_to_cp == 1) {
        if (info->det_debounce_checking == 0 && info->sim_debounce_delay_wq != NULL) {
            info->det_debounce_checking = 1;
            if (info->sim_id == 0) {
                g_sim0_last_det_cnt = g_sim_plug_state.sim0_det_cnt;
            }
            if (det_gpio_level == 1) {
                debounce_wait_time = info->det_high_debounce_wait_time;
            } else {
                debounce_wait_time = info->det_low_debounce_wait_time;
            }
            __pm_wakeup_event(&info->sim_hotplug_wklock, (debounce_wait_time + 5)); // extra 5 ms for wakeup

            queue_delayed_work(info->sim_debounce_delay_wq, &info->sim_debounce_delay_work,
                msecs_to_jiffies(debounce_wait_time));
            sim_log_info("sim%d, det level %d debounce check begin.\n", info->sim_id, det_gpio_level);
        } else {
            sim_log_info("sim%d, det debounce check is ongoing.\n", info->sim_id);
        }
    } else {
        sim_manage_sim_state(det_gpio_level, info);
    }
}

static irqreturn_t sim_det_irq_handler(int irq, void *data)
{
    struct hisi_sim_platform_info *info = data;
    if (info == NULL) {
        sim_log_err("info is NULL.\n");
        return IRQ_HANDLED;
    }

    if (info->sim_id == SIM0 || info->sim_id == SIM1) {
        sim_log_info("sim%d sim_hotplug_det_work begin.\n", info->sim_id);
        update_sim_hotplug_count(info->sim_id, SIM_CARD_DET);

        if (info->sim_hotplug_det_wq == NULL) {
            sim_log_info("sim%d queue sim_hotplug_det_work failed.\n", info->sim_id);
        } else {
            queue_work(info->sim_hotplug_det_wq, &info->sim_hotplug_det_work);
        }
    } else {
        sim_log_info("invalid sim%d.\n", info->sim_id);
    }
    return IRQ_HANDLED;
}

static void sim_debounce_check_func(struct work_struct *work)
{
    struct hisi_sim_platform_info *info =
        container_of(work, struct hisi_sim_platform_info, sim_debounce_delay_work.work);
    int det_gpio_level;

    det_gpio_level = gpio_get_value(info->det_gpio);

    sim_log_info("sim%d, old sim_pluged: %s, det_gpio_level: %d, det debounce check end.\n", info->sim_id,
        simplug_to_string(info->sim_pluged), det_gpio_level);
    sim_manage_sim_state(det_gpio_level, info);
    info->det_debounce_checking = 0;
}

static void sim_hpd_mask_irq(struct hisi_sim_platform_info *info, bool mask)
{
    struct irq_desc *sim_hpd_irq_desc;

    sim_hpd_irq_desc = irq_to_desc(info->hpd_irq);
    if (sim_hpd_irq_desc == NULL) {
        sim_log_err("sim_hpd_irq_desc error\n");
        return;
    }

    if (mask) {
        /* mask interrupts */
        sim_hpd_irq_desc->irq_data.chip->irq_mask(&sim_hpd_irq_desc->irq_data);
    } else {
        /* unmask interrupts */
        sim_hpd_irq_desc->irq_data.chip->irq_unmask(&sim_hpd_irq_desc->irq_data);
    }
}

static irqreturn_t sim_hpd_irq_handler(int irq, void *data)
{
    struct hisi_sim_platform_info *info = data;
    if (info == NULL) {
        sim_log_err("info is NULL.\n");
        return IRQ_HANDLED;
    }

    sim_log_info("sim%d sim_hotplug_hpd_work.\n", info->sim_id);

    sim_hpd_mask_irq(info, 1);

    if (info->sim_id == SIM0) {
        sim_log_info("IRQ HPD0 happend\n");
    } else if (info->sim_id == SIM1) {
        sim_log_info("IRQ HPD1 happend\n");
    } else {
        sim_log_err("invalid simid %d.\n", info->sim_id);
        return IRQ_HANDLED;
    }

    if (info->sim_hotplug_hpd_wq == NULL) {
        sim_log_info("sim%d queue sim_hotplug_hpd_wq failed.\n", info->sim_id);
    } else {
        queue_work(info->sim_hotplug_hpd_wq, &info->sim_hotplug_hpd_work);
    }

    return IRQ_HANDLED;
}

static void inquiry_sim_hpd_irq_reg(struct work_struct *work)
{
    struct hisi_sim_platform_info *info = container_of(work, struct hisi_sim_platform_info, sim_hotplug_hpd_work);

    hisi_sim_hpd_msg_to_ccore(info);
    sim_hpd_mask_irq(info, 0);
}

static int sim_init_debounce_check_wq(struct hisi_sim_platform_info *info)
{
    int ret = 0;

    if (info == NULL) {
        sim_log_err("info is NULL.\n");
        ret = -EINVAL;
    } else {
        sim_log_info("for sim%d\n", info->sim_id);

        if (info->send_msg_to_cp == 1) {
            info->det_debounce_checking = 0;
            info->sim_debounce_delay_wq = create_singlethread_workqueue(g_sim_deb_work_name[info->sim_id]);
            if (info->sim_debounce_delay_wq == NULL) {
                sim_log_err("sim_debounce_delay_wq create failed.\n");
                ret = -ENOMEM;
            } else {
                INIT_DELAYED_WORK(&(info->sim_debounce_delay_work), sim_debounce_check_func);
            }
        }
    }

    sim_log_info("ret: %d\n", ret);
    return ret;
}

static int read_det_interrupt_dts(struct hisi_sim_platform_info *info, struct device_node *np)
{
    int ret;
    u32 cfg = 0;
    enum of_gpio_flags flags;

    info->det_gpio = of_get_gpio_flags(np, 0, &flags); // det pin gpio number
    if (info->det_gpio < 0) {
        sim_log_err("det_gpio number: %d, error\n", info->det_gpio);
        return info->det_gpio;
    }

    if (!gpio_is_valid(info->det_gpio)) {
        sim_log_err("det_gpio number: %d, gpio_is_valid is invalid, error\n", info->det_gpio);
        return -EINVAL;
    }
    info->det_irq = gpio_to_irq(info->det_gpio);

    if (of_property_read_u32(np, "allocate_gpio", &cfg)) {
        sim_log_info("allocate_gpio property not found, using 0 as default\n");
        cfg = 0;
    }
    info->allocate_gpio = cfg;

    if (of_property_read_u32(np, "shared_det_irq", &cfg)) {
        sim_log_info("shared_det_irq property not found, using 0 as default\n");
        cfg = 0;
    }
    info->shared_det_irq = cfg;

    ret = of_property_read_u32(np, "send_msg_to_cp", &cfg);
    if (ret < 0) {
        cfg = 0;
    }
    info->send_msg_to_cp = cfg;

    ret = of_property_read_u32(np, "send_card_out_msg_when_init", &cfg);
    if (ret < 0) {
        cfg = 0;
    }
    info->send_card_out_msg = cfg;

    ret = of_property_read_u32(np, "factory_send_msg_to_cp", &cfg);
    if (ret < 0) {
        cfg = 0;
    }
    info->factory_send_msg_to_cp = cfg;

    return 0;
}

static int read_product_special_dts(struct hisi_sim_platform_info *info, struct device_node *np)
{
    int ret;
    u32 cfg = 0;
    const char *str_det_normal_direction = NULL;

    /*lint -e421 -esym(421,*)*/
    ret = of_property_read_string(np, "det_normal_direction", &str_det_normal_direction);
    if (ret < 0) {
        sim_log_err("failed to read det_normal_direction\n");
        return ret;
    }
    if (strcmp(str_det_normal_direction, "open") == 0) {
        info->det_normal_direction = NORMAL_OPEN;
    } else if (strcmp(str_det_normal_direction, "closed") == 0) {
        info->det_normal_direction = NORMAL_CLOSE;
    } else {
        info->det_normal_direction = NORMAL_BUTT;
    }
    /*lint -e421 +esym(421,*)*/
    if (info->det_normal_direction == NORMAL_BUTT) {
        sim_log_info("det_normal_direction is NORMAL_BUTT and set to NORMAL_CLOSE\n");
        info->det_normal_direction = NORMAL_CLOSE;
    }

    ret = of_property_read_u32(np, "card_tray_style", &cfg);
    if (ret < 0) {
        sim_log_err("failed to read card_tray_style\n");
        return ret;
    }
    info->card_tray_style = cfg;

    ret = of_property_read_u32(np, "mux_sdsim", &cfg);
    if (ret < 0) {
        sim_log_info("failed to read mux_sdsim, as 0\n");
        cfg = 0;
    }
    info->mux_sdsim = cfg;
    (void)sim_io_mux_init(info, np);
    (void)sd_io_mux_init(info, np);
    return 0;
}

static void read_debounce_dts(struct hisi_sim_platform_info *info, struct device_node *np)
{
    int ret;
    u32 cfg = 0;

    ret = of_property_read_u32(np, "det_high_debounce_wait_time", &cfg);
    if (ret < 0) {
        cfg = DET_DEBOUNCE_CHECK_WAIT_TIME;
    }
    info->det_high_debounce_wait_time = cfg;

    ret = of_property_read_u32(np, "det_low_debounce_wait_time", &cfg);
    if (ret < 0) {
        cfg = DET_DEBOUNCE_CHECK_WAIT_TIME;
    }
    info->det_low_debounce_wait_time = cfg;

    ret = of_property_read_u32(np, "det_debounce_diff_max", &cfg);
    if (ret < 0) {
        cfg = DET_DEBOUNCE_DIFF_MAX;
    }
    info->det_debounce_diff_max = cfg;
}

static int sim_hotplug_dts_read(struct hisi_sim_platform_info *info, struct device_node *np)
{
    int ret;

    sim_log_info("loading dts\n");
    ret = read_det_interrupt_dts(info, np);
    if (ret < 0) {
        return ret;
    }
    ret = read_product_special_dts(info, np);
    if (ret < 0) {
        return ret;
    }
    read_debounce_dts(info, np);

    sim_log_info("sim%d, det_gpio_number: %d, send_msg_to_cp: %d, det_normal_direction: %s, card_tray_style: %s\n",
        info->sim_id, info->det_gpio, info->send_msg_to_cp, direction_to_string(info->det_normal_direction),
        card_tray_style_to_string(info->card_tray_style));

    sim_log_info("sim%d, allocate_gpio: %d, shared_det_irq: %d, mux_sdsim: %d, send_card_out_msg: %d, "
        "factory_send_msg_to_cp: %d, det_debounce_wait_time, high: %d, low: %d\n",
        info->sim_id, info->allocate_gpio, info->shared_det_irq, info->mux_sdsim, info->send_card_out_msg,
        info->factory_send_msg_to_cp, info->det_high_debounce_wait_time, info->det_low_debounce_wait_time);

    return 0;
}

static int sim_hotplug_init(struct hisi_sim_platform_info *info)
{
    int ret = 0;

    sim_log_info("hotplug initializing.\n");

    if (info->sim_id == SIM0) {
        g_sim_plug_state.sim0_det_normal_direction = (u8)(info->det_normal_direction);
        g_sim_plug_state.sim0_leave_cnt = 0;
        g_sim_plug_state.sim0_out_cnt = 0;
        g_sim_plug_state.sim0_in_cnt = 0;
        g_sim_plug_state.sim0_det_cnt = 0;
    } else if (info->sim_id == SIM1) {
        g_sim_plug_state.sim1_det_normal_direction = (u8)(info->det_normal_direction);
        g_sim_plug_state.sim1_leave_cnt = 0;
        g_sim_plug_state.sim1_out_cnt = 0;
        g_sim_plug_state.sim1_in_cnt = 0;
        g_sim_plug_state.sim1_det_cnt = 0;
    } else {
        sim_log_info("invalid sim id%d.\n", info->sim_id);
        return -ENOMEM;
    }

    if (info->allocate_gpio == 1) {
        sim_log_info("sim%d, gpio_request_one, det_gpio: %d\n", info->sim_id, info->det_gpio);
        ret = gpio_request_one(info->det_gpio, GPIOF_IN, "sim_det");
        if (ret < 0) {
            sim_log_err("failed to gpio_request_one for sim%d det_gpio: %d.\n", info->sim_id, info->det_gpio);
            return ret;
        }
    }

    /* create det workqueue */
    info->sim_hotplug_det_wq = create_singlethread_workqueue("sim_hotplug_det");
    if (info->sim_hotplug_det_wq == NULL) {
        sim_log_err("failed to create work queue.\n");
        return -ENOMEM;
    }

    /*lint -e611 -esym(611,*)*/
    INIT_WORK(&info->sim_hotplug_det_work, (void *)inquiry_sim_det_irq_reg);
    /*lint -e611 +esym(611,*)*/
    ret = sim_init_debounce_check_wq(info);
    if (ret < 0) {
        sim_log_err("failed to sim_init_debounce_check_wq for sim%d.\n", info->sim_id);
        return ret;
    }

    ret = sim_hotplug_sdmux_init(info);
    if (ret < 0) {
        sim_log_err("sim_hotplug_sdmux_init failed, ret: %d\n", ret);
        return ret;
    }

    return ret;
}

static int sim_state_init(struct hisi_sim_platform_info *info, struct device *dev)
{
    sim_log_info("state initializing.\n");

    if (gpio_get_value(info->det_gpio) == 0) {
        info->old_det_gpio_level = 0;
        if (info->det_normal_direction == NORMAL_CLOSE) {
            info->sim_pluged = SIM_CARD_OUT;
            save_sim_status(info, info->sim_id, info->sim_pluged);
            sim_set_inactive(info);
            if (info->send_card_out_msg == 1) {
                info->old_det_gpio_level = 1;
                hisi_sim_det_msg_to_ccore(info, SIM_CARD_OUT);
            }
        } else {
            info->sim_pluged = SIM_CARD_IN;
            save_sim_status(info, info->sim_id, info->sim_pluged);
            sim_set_active(info);
        }
    } else {
        info->old_det_gpio_level = 1;
        if (info->det_normal_direction == NORMAL_CLOSE) {
            info->sim_pluged = SIM_CARD_IN;
            save_sim_status(info, info->sim_id, info->sim_pluged);
            sim_set_active(info);
        } else {
            info->sim_pluged = SIM_CARD_OUT;
            save_sim_status(info, info->sim_id, info->sim_pluged);
            sim_set_inactive(info);
            if (info->send_card_out_msg == 1) {
                info->old_det_gpio_level = 0;
                hisi_sim_det_msg_to_ccore(info, SIM_CARD_OUT);
            }
        }
    }

    mutex_init(&(info->sim_hotplug_lock));
    wakeup_source_init(&info->sim_hotplug_wklock, "android-simhotplug");

    return 0;
}

static int sim_hpd_init(struct hisi_sim_platform_info *info, struct spmi_device *pdev)
{
    sim_log_info("hpd initializing.\n");

    if (pdev == NULL) {
        sim_log_err("pdev is NULL.\n");
        return -1;
    }

    if (info->sim_id == SIM0) {
        info->hpd_irq = spmi_get_irq_byname(pdev, NULL, "hpd0_falling");
    } else if (info->sim_id == SIM1) {
        info->hpd_irq = spmi_get_irq_byname(pdev, NULL, "hpd1_falling");
    } else {
        sim_log_err("sim%d, wrong sim id.\n", info->sim_id);
        return -EINVAL;
    }

    if (info->hpd_irq < 0) {
        return -ENOENT;
    }

    /* create hpd workqueue */
    info->sim_hotplug_hpd_wq = create_singlethread_workqueue("sim_hotplug_hpd");
    if (info->sim_hotplug_hpd_wq == NULL) {
        sim_log_err("sim_hotplug_hpd_wq failed to create work queue.\n");
        return -EINVAL;
    }

    /*lint -e611 -esym(611,*)*/
    INIT_WORK(&info->sim_hotplug_hpd_work, (void *)inquiry_sim_hpd_irq_reg);
    /*lint -e611 +esym(611,*)*/
    return 0;
}

void notify_card_status(int32_t card_status)
{
    s32 ret = 0;
    u8 sim_state = SIM_CARD_IN;
    sci_msg_data_s sci_msg;
    struct hisi_sim_platform_info *info = NULL;

    if (card_status == SIM_CARD_IN) {
        sim_state = (u8)card_status;
        if (g_sim_plug_state.sim1_pluged != SIM_CARD_IN) {
            sim_log_err("card1 not present, so return\n");
            return;
        }

        if (g_sd2jtag_enable == 1) {
            sim_log_err("sd2jtag_enable, so return\n");
            return;
        }

        info = hisi_sim_get_platinfo(SIM1);
        if (info == NULL) {
            sim_log_err("platinfo is NULL, so return\n");
            return;
        }

        sci_msg.msg_type = SIM_MSG_HOTPLUG_TYPE;
        sci_msg.msg_value = sim_state;
        sim_log_info("sim1, fake SIM_CARD_IN sci_send_msg to CP\n");
        ret = sci_send_msg(info, &sci_msg);
        if (ret) {
            sim_log_err("sim1, sci_send_msg failed.\n");
        }
    }
}

void sim_jtag_shared_mem_write(void)
{
    u32 *ptr = NULL;
    s32 ret = 0;
    u32 simjtag_status = 0;

    if (SHM_BASE_ADDR != 0) {
        ptr = (u32 *)(uintptr_t)(SHM_BASE_ADDR + SHM_OFFSET_SIMJTAG);
        if (get_card1_type() == STATUS_SD2JTAG) {
            simjtag_status = SIMJTAG_MAGIC;
            sim_log_err("simjtag enabled.\n");
            ret = memcpy_s(ptr, SHM_SIZE_SIM_JTAG, &simjtag_status, SHM_SIZE_SIM_JTAG);
            if (ret) {
                sim_log_err("memcpy failed.\n");
            }
        } else {
            simjtag_status = 0;
            ret = memcpy_s(ptr, SHM_SIZE_SIM_JTAG, &simjtag_status, SHM_SIZE_SIM_JTAG);
            if (ret) {
                sim_log_err("memcpy failed.\n");
            }
            sim_log_err("simjtag disabled.\n");
        }
    }
}

u8 get_card1_status(struct hisi_sim_platform_info *info)
{
    u8 status = STATUS_NO_CARD;

    if (info == NULL) {
        sim_log_err("info is NULL, as STATUS_NO_CARD return.\n");
        return status;
    }

    if (info->mux_sdsim == 0) {
        sim_log_info("sim1 mux_sdsim is 0, as STATUS_SIM return.\n");
        return STATUS_SIM;
    }

    status = get_card1_type();

    return status;
}

struct simhp_driver_data {
    spinlock_t read_lock;
    int32_t sim_id;
    int32_t simhp_dev_en;
    dev_t simhp_dev;
    struct cdev simhp_cdev;
    struct class *simhp_class;
};

static struct simhp_driver_data g_simhotplug_data;

static int simhotplug_open(struct inode *inode, struct file *filp)
{
    spin_lock_bh(&g_simhotplug_data.read_lock);

    if (g_simhotplug_data.simhp_dev_en) {
        sim_log_err("simhotplug device has been opened\n");
        spin_unlock_bh(&g_simhotplug_data.read_lock);
        return -EPERM;
    }

    g_simhotplug_data.simhp_dev_en = true;

    spin_unlock_bh(&g_simhotplug_data.read_lock);
    sim_log_info("success\n");

    return 0;
}

/*
 * this is main method to exchange data with user space,
 * including socket sync, get ip and port, adjust kernel flow
 * return "int" by standard.
 */
static long simhotplug_ioctl(struct file *flip, unsigned int cmd, unsigned long arg)
{
    int rc = -EFAULT;
    void __user *argp = (void __user *)(uintptr_t)arg;
    int32_t card_status;
    if (argp == NULL) {
        sim_log_info("simhotplug_ioctl arg is null.\n");
        return rc;
    }

    switch (cmd) {
        case SIMHOTPLUG_IOC_INFORM_CARD_INOUT: {
            sim_log_info("simhotplug_ioctl card status inout.\n");

            if (copy_from_user(&card_status, argp, sizeof(card_status)))
                break;

            sim_log_info("simhotplug_ioctl card status card_status= %d.\n", card_status);

            notify_card_status(card_status);

            rc = 0;

            break;
        }
        default: {
            sim_log_err("unknown ioctl: %u\n", cmd);
            break;
        }
    }

    return rc;
}

/* support of 32bit userspace on 64bit platforms */
#ifdef CONFIG_COMPAT
static long compat_simhotplug_ioctl(struct file *flip, unsigned int cmd, unsigned long arg)
{
    return simhotplug_ioctl(flip, cmd, (unsigned long)(uintptr_t)compat_ptr(arg));
}
#endif

static int simhotplug_release(struct inode *inode, struct file *filp)
{
    spin_lock_bh(&g_simhotplug_data.read_lock);
    g_simhotplug_data.simhp_dev_en = false;
    spin_unlock_bh(&g_simhotplug_data.read_lock);
    sim_log_info("success\n");
    return 0;
}

static const struct file_operations g_simhp_dev_fops = {
    .owner = THIS_MODULE,
    .open = simhotplug_open,
    .unlocked_ioctl = simhotplug_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = compat_simhotplug_ioctl,
#endif
    .release = simhotplug_release,
};

#define SIMHP_DEV_NAME_LEN 16
static int create_simhp_node(int sim_id)
{
    int ret;
    struct device *dev = NULL;
    char simhp_dev_name[SIMHP_DEV_NAME_LEN];
    int mret;
    mret = memset_s(simhp_dev_name, sizeof(simhp_dev_name), 0, sizeof(simhp_dev_name));
    if (mret) {
        sim_log_err("memset failed.\n");
    }
    mret =
        snprintf_s(simhp_dev_name, sizeof(simhp_dev_name), (sizeof(simhp_dev_name) - 1), SIMHP_NAME_BASE "%d", sim_id);
    if (mret < 0) {
        sim_log_err("snprintf failed.\n");
    }

    sim_log_err("start\n");

    /* register sci major and minor number */
    ret = alloc_chrdev_region(&g_simhotplug_data.simhp_dev, SIMHP_FIRST_MINOR, SIMHP_DEVICES_NUMBER, simhp_dev_name);
    if (ret) {
        sim_log_err("alloc_chrdev_region failed\n");
        goto fail_region;
    }

    cdev_init(&g_simhotplug_data.simhp_cdev, &g_simhp_dev_fops);
    g_simhotplug_data.simhp_cdev.owner = THIS_MODULE;

    ret = cdev_add(&g_simhotplug_data.simhp_cdev, g_simhotplug_data.simhp_dev, SIMHP_DEVICES_NUMBER);
    if (ret) {
        sim_log_err("cdev_add failed\n");
        goto fail_cdev_add;
    }

    g_simhotplug_data.simhp_class = class_create(THIS_MODULE, simhp_dev_name);
    if (IS_ERR(g_simhotplug_data.simhp_class)) {
        sim_log_err("class_create failed\n");
        goto fail_class_create;
    }

    dev = device_create(g_simhotplug_data.simhp_class, NULL, g_simhotplug_data.simhp_dev, NULL, simhp_dev_name);
    if (IS_ERR(dev)) {
        sim_log_err("device_create failed\n");
        goto fail_device_create;
    }

    spin_lock_init(&g_simhotplug_data.read_lock);

    return 0;

fail_device_create:
    class_destroy(g_simhotplug_data.simhp_class);
fail_class_create:
    cdev_del(&g_simhotplug_data.simhp_cdev);
fail_cdev_add:
    unregister_chrdev_region(g_simhotplug_data.simhp_dev, SIMHP_DEVICES_NUMBER);
fail_region:

    return ret;
}

void remove_simhp_node(int sim_id)
{
    spin_lock_bh(&g_simhotplug_data.read_lock);
    g_simhotplug_data.simhp_dev_en = false;
    spin_unlock_bh(&g_simhotplug_data.read_lock);

    if (g_simhotplug_data.simhp_class != NULL) {
        device_destroy(g_simhotplug_data.simhp_class, g_simhotplug_data.simhp_dev);
        class_destroy(g_simhotplug_data.simhp_class);
    }
    cdev_del(&g_simhotplug_data.simhp_cdev);
    unregister_chrdev_region(g_simhotplug_data.simhp_dev, SIMHP_DEVICES_NUMBER);
    sim_log_err("remove simhp node\n");
}

int sim_hotplug_sdmux_init(struct hisi_sim_platform_info *info)
{
    int ret = 0;

    if ((info->sim_id == SIM1) && (info->mux_sdsim == 1)) {
        if (get_card1_type() == STATUS_SD2JTAG) {
            g_sd2jtag_enable = 1;
        }
        ret = create_simhp_node(info->sim_id);
    }

    return ret;
}

int sim_hotplug_irq_register(struct hisi_sim_platform_info *info)
{
    int ret;
    ret =
        request_threaded_irq(info->hpd_irq, sim_hpd_irq_handler, NULL, IRQF_NO_SUSPEND | IRQF_SHARED, "sim_hpd", info);
    if (ret < 0) {
        sim_log_err("sim%d hpd_irq failed to request_threaded_irq, ret: %d\n", info->sim_id, ret);
        return ret;
    }

    sim_log_info("sim%d hpd_irq: %d\n", info->sim_id, info->hpd_irq);

    if (info->shared_det_irq == 1) {
        ret = request_threaded_irq(info->det_irq, sim_det_irq_handler, NULL,
            IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND | IRQF_SHARED, "sim_det", info);
    } else {
        ret = request_threaded_irq(info->det_irq, sim_det_irq_handler, NULL,
            IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "sim_det", info);
    }
    if (ret < 0) {
        sim_log_err("request_threaded_irq failed, ret: %d\n", ret);
        return ret;
    }
    sim_log_info("sim%d det_irq: %d\n", info->sim_id, info->det_irq);
    return 0;
}

int hisi_sim_hotplug_init(struct hisi_sim_platform_info *info, struct spmi_device *pdev)
{
    struct device *dev = NULL;
    struct device_node *np = NULL;
    int ret;

    np = info->spmidev->of_node;

    ret = sim_hotplug_dts_read(info, np);
    if (ret < 0) {
        sim_log_err("sim_hotplug_dts_read failed, ret: %d\n", ret);
        ret = SIM_HW_HOTPLUG_DTS_ERR;
        goto free_sim_det_wq;
    }

    ret = sim_hotplug_init(info);
    if (ret < 0) {
        sim_log_err("sim_hotplug_init failed, ret: %d\n", ret);
        ret = SIM_HW_HOTPLUG_DET_ERR;
        goto free_sim_det_wq;
    }

    ret = sim_pmu_hpd_init(info, np);
    if (ret < 0) {
        sim_log_err("sim_pmu_hpd_init failed, ret: %d\n", ret);
        ret = SIM_HW_HOTPLUG_HPD_ERR;
        goto free_sim_det_wq;
    }

    ret = sim_state_init(info, dev);
    if (ret < 0) {
        sim_log_err("sim_state_init failed, ret: %d\n", ret);
        ret = SIM_HW_HOTPLUG_STATE_ERR;
        goto free_sim_lock;
    }

    ret = sim_hpd_init(info, pdev);
    if (ret < 0) {
        sim_log_err("sim_hpd_init failed, ret: %d\n", ret);
        return SIM_HW_HOTPLUG_HPD_IRQ_FAIL;
        goto free_sim_lock;
    }

    if (info->sim_id == SIM0) {
        sys_add_sim_node();
        sim_jtag_shared_mem_write();
    } else if (info->sim_id == SIM1) {
        if (info->mux_sdsim == 1) {
            ret = sd_sim_detect_run(NULL, !(info->sim_pluged), MODULE_SIM, 0);
            sim_log_info("sd_sim_detect_run ret: %d\n", ret);
        }
    }

    ret = sim_hotplug_irq_register(info);
    if (ret != 0) {
        sim_log_err("sim_hotplug_irq_register failed, ret: %d\n", ret);
    }

    dev_set_drvdata(&pdev->dev, info);

    return 0;

free_sim_lock:
    sim_log_err("free sim lock, ret: %d\n", ret);
    wakeup_source_trash(&info->sim_hotplug_wklock);

free_sim_det_wq:
    if (info->sim_hotplug_det_wq != NULL) {
        destroy_workqueue(info->sim_hotplug_det_wq);
    }
    if (info->sim_hotplug_hpd_wq != NULL) {
        destroy_workqueue(info->sim_hotplug_hpd_wq);
    }

    sim_log_err("free info, ret: %d\n", ret);
    return ret;
}
