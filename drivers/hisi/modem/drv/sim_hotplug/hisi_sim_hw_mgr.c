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

#include "hisi_sim_hw_mgr.h"
#include "hisi_sim_hotplug.h"
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

int hisi_sim_pmu_ctrl_init(struct hisi_sim_platform_info *info)
{
    int ret = SIM_HW_OP_RESULT_SUCC;

    sim_log_err(" init pmu ctrl for sim%d.\n", info->sim_id);
    info->sim_power.sim_pmu = devm_regulator_get(info->spmidev, "sim_vcc");
    if (IS_ERR(info->sim_power.sim_pmu)) {
        sim_log_err("No sim regulator found\n");
        return SIM_HW_REQUESR_PMU_FAIL;
    }

    return ret;
}

void hisi_sim_pmu_ctrl_deinit(struct hisi_sim_platform_info *info)
{
    sim_log_err(" deinit pmu ctrl for sim1.\n");
    devm_regulator_put(info->sim_power.sim_pmu);
}

int hisi_sim_pmu_power_on(struct hisi_sim_platform_info *info, s32 vol)
{
    int ret;
    u32 pmic_reg_value;

    if (regulator_is_enabled(info->sim_power.sim_pmu)) {
        sim_log_err("regulator is enabled while power on, disable first.\n");
        ret = regulator_disable(info->sim_power.sim_pmu);
        if (ret != 0) {
            sim_log_err("disable sim_vcc failed.\n");
            return SIM_HW_SET_VOLTAGE_FAIL;
        }
    }

    ret = regulator_set_voltage(info->sim_power.sim_pmu, vol, vol);
    if (ret != 0) {
        sim_log_err("set sim_pmu failed.\n");
        return SIM_HW_SET_VOLTAGE_FAIL;
    }

    ret = regulator_enable(info->sim_power.sim_pmu);
    if (ret != 0) {
        sim_log_err("enable sim_pmu failed.\n");
        return SIM_POWER_ON_FAIL;
    }
    if (info->sim_power.sim_lvshift_ctrl.func) {
        pmic_reg_value = hisi_sub_pmic_reg_read(info->sim_power.sim_lvshift_ctrl.reg_off);
        pmic_reg_value &= (~0x4);
        hisi_sub_pmic_reg_write(info->sim_power.sim_lvshift_ctrl.reg_off, pmic_reg_value);
        udelay(1000); // delay 1000 us
        pmic_reg_value = hisi_sub_pmic_reg_read(info->sim_power.sim_lvshift_ctrl.reg_off);
        pmic_reg_value |= 0x1;
        hisi_sub_pmic_reg_write(info->sim_power.sim_lvshift_ctrl.reg_off, pmic_reg_value);
        sim_log_err("write lvshift offset %d, value %d.\n", info->sim_power.sim_lvshift_ctrl.reg_off, pmic_reg_value);
    }

    return SIM_POWER_ON_SUCC;
}

int hisi_sim_pmu_power_off(struct hisi_sim_platform_info *info)
{
    u32 pmic_reg_value;
    if (info->sim_power.sim_lvshift_ctrl.func) {
        pmic_reg_value = hisi_sub_pmic_reg_read(info->sim_power.sim_lvshift_ctrl.reg_off);
        pmic_reg_value &= (~0x1);
        hisi_sub_pmic_reg_write(info->sim_power.sim_lvshift_ctrl.reg_off, pmic_reg_value);
        pmic_reg_value = hisi_sub_pmic_reg_read(info->sim_power.sim_lvshift_ctrl.reg_off);
        pmic_reg_value |= 0x4;
        hisi_sub_pmic_reg_write(info->sim_power.sim_lvshift_ctrl.reg_off, pmic_reg_value);
        sim_log_err("write lvshift offset %d, value %d.\n", info->sim_power.sim_lvshift_ctrl.reg_off, pmic_reg_value);
    }

    regulator_disable(info->sim_power.sim_pmu);

    return SIM_POWER_OFF_SUCC;
}

int hisi_sim_pmu_ctrl(struct hisi_sim_platform_info *info)
{
    int ret;

    ret = hisi_sim_pmu_ctrl_init(info);
    if (ret != SIMHP_OK) {
        sim_log_err("sim pmu init failed, ret = %d.\n", ret);
        return ret;
    }
    sim_log_err("sim pmu ctrl, msg_value is %d.\n", info->msgfromsci.msg_value);
    switch (info->msgfromsci.msg_value) {
        case SIM_POWER_ON_CLASS_C_REQUEST:
            ret = hisi_sim_pmu_power_on(info, SIM_VCC_1P8V);
            break;

        case SIM_POWER_ON_CLASS_B_REQUEST:
            ret = hisi_sim_pmu_power_on(info, SIM_VCC_3V);
            break;

        case SIM_POWER_OFF_REQUEST:
            ret = hisi_sim_pmu_power_off(info);
            break;

        default:
            sim_log_err("invalid cmd.\n");
            ret = SIM_HW_SET_VOLTAGE_FAIL;
            break;
    }
    hisi_sim_pmu_ctrl_deinit(info);

    sim_log_err("pmu ctrl done. ret = %d.\n", ret);
    return ret;
}

int hisi_sim_hw_mgr_init(struct hisi_sim_platform_info *info)
{
    int ret = 0;
    if (info->is_hisi_hw) {
        ret = of_property_read_u32_array(info->spmidev->of_node, "sim_lvshift_ctrl",
            (u32 *)&info->sim_power.sim_lvshift_ctrl, 0x3);
        if (ret < 0) {
            sim_log_err("failed to read sim_lvshift_ctrl.\n");
            info->sim_power.sim_lvshift_ctrl.func = 0;
        }
    } else {
    }
    return ret;
}
