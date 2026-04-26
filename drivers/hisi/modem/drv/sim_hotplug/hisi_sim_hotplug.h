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

#ifndef __HISI_SIM_HOTPLUG_H
#define __HISI_SIM_HOTPLUG_H

#include <linux/irqdomain.h>
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
#include <bsp_icc.h>
#include "hisi_sim_hw_service.h"

#define SIM0 0
#define SIM1 1
#define SIM_CARD_OUT 0
#define SIM_CARD_IN 1

#define SIMHP_OK 0
#define SIMHP_PMU_IO_SET_FAIL 1
#define SIMHP_PMU_VCC_SET_FAIL 3
#define SIMHP_PMU_IO_ENABLE_FAIL 5
#define SIMHP_PMU_IO_DISABLE_FAIL 6
#define SIMHP_PMU_VCC_ENABLE_FAIL 7
#define SIMHP_PMU_VCC_DISABLE_FAIL 8
#define SIMHP_NULL_POINTER 9
#define SIMHP_INVALID_PARAM 10
#define SIMHP_INVALID_CHNL 10
#define SIMHP_NO_IO_PMU 11
#define SIMHP_NO_VCC_PMU 12
#define SIMHP_NO_SWITCH_PMU 13
#define SIMHP_MSG_RECV_ERR 14
#define SIMHP_NO_POWER_SCHEME 15

#define SIMHP_FIRST_MINOR 0
#define SIMHP_DEVICES_NUMBER 1

#define SIMHP_NAME_BASE "simhotplug"

#define SIMHOTPLUG_IOC_MAGIC 'k'

#define SIMHOTPLUG_IOC_INFORM_CARD_INOUT _IOWR(SIMHOTPLUG_IOC_MAGIC, 1, int32_t)

typedef enum {
    CARD_MSG_OUT = 0,
    CARD_MSG_IN = 1,
    CARD_MSG_LEAVE = 3,
    CARD_MSG_SIM = 5,
    CARD_MSG_SD = 6,
    CARD_MSG_NO_CARD = 7,
    CARD_MSG_SD2JTAG = 8,
    CARD_MSG_PMU_ON = 9,
    CARD_MSG_PMU_OFF = 10,
    CARD_MSG_PMU_FAIL = 11,
} sci_acore_response_e;

#define SIM_MAIN_PMU 0
#define SIM_SUB_PMU 1

#undef THIS_MODU
#define THIS_MODU mod_simhotplug

#define sim_log_err(fmt, ...) (void)bsp_err("<%s>" fmt, __FUNCTION__, ##__VA_ARGS__)
#define sim_log_info(fmt, ...) (void)bsp_err("<%s>" fmt, __FUNCTION__, ##__VA_ARGS__)

// DET PIN normal close or normal open, NORMAL means card tray not insert.
enum sim_det_normal_direction {
    NORMAL_CLOSE = 0,
    NORMAL_OPEN = 1,
    NORMAL_BUTT = 2,
};

enum sim_card_tray_style {
    SINGLE_CARD_IN_ONE_TRAY = 0,
    TWO_CARDS_IN_ONE_TRAY = 1,
    CARD_TRAY_BUTT = 2,
};

struct sim_io_mux_cfg {
    u32 phy_addr;
    void *virt_addr;
    u32 iomg_usim_offset[3]; // 3 for usim clk rst data
    u32 iocg_usim_offset[3]; // 3 for usim clk rst data
    u32 esim_detect_en;
    u32 esim_det_func_offset[4]; // 4 pin detect
    u32 esim_det_ctrl_offset[4]; // 4 pin detect
};

typedef struct {
    u32 func;
    u32 reg_off;
    u32 reg_bit;
} sim_sysctrl_s;

struct sim_power_ctrl {
    struct regulator *sim_pmu;
    sim_sysctrl_s sim_lvshift_ctrl;
};

struct hisi_sim_platform_info {
    struct workqueue_struct *sim_hotplug_hpd_wq;
    struct workqueue_struct *sim_hotplug_det_wq;
    struct workqueue_struct *sim_debounce_delay_wq;
    struct workqueue_struct *sim_sci_msg_wq;
    struct mutex sim_hotplug_lock;
    struct wakeup_source sim_hotplug_wklock;
    struct work_struct sim_hotplug_hpd_work;
    struct work_struct sim_hotplug_det_work;
    struct delayed_work sim_debounce_delay_work;
    struct work_struct sim_sci_msg_work;

    int hpd_irq;
    int det_irq;
    int det_gpio;
    int old_det_gpio_level;
    int det_debounce_checking; // indicate det debounce is checking or not

    u32 sim_id;
    u32 channel_id;
    u32 card_tray_style;
    u32 send_msg_to_cp;
    u32 send_card_out_msg;
    u32 hpd_interrupt_to_close_ldo;
    u32 hpd_debounce_wait_time;
    u32 det_high_debounce_wait_time; // when det gpio level is high(1)
    u32 det_low_debounce_wait_time;  // when det gpio level is low(0)
    u32 det_normal_direction;        // 1 indicats open by default(high level), 0 indicats closed by default
    u32 allocate_gpio;
    u32 shared_det_irq;
    u32 factory_send_msg_to_cp;
    u32 mux_sdsim;
    u32 det_debounce_diff_max;

    u32 pmu_status1_address;
    u32 pmu_irq_address;
    u32 pmu_irq_mask_address;
    u32 pmu_sim_ctrl_address;
    u32 pmu_sim_deb_ctrl_address;
    u32 pmu_ldo11_onoff_eco_address;
    u32 pmu_ldo11_vset_address;
    u32 pmu_ldo12_pull_down;
    u32 pmu_ldo12_onoff_eco_address;
    u32 pmu_ldo12_vset_address;

    u32 sim_pluged;
    sci_msg_data_s msgfromsci;
    struct device *spmidev;
#ifndef CONFIG_SCI_DECP
    struct sim_io_mux_cfg sim_io_cfg;
    struct sim_io_mux_cfg sd_io_cfg;
    u32 hpd1_used; // 1 hpd1 is use, 0 hpd1 is not use
#endif
    struct sim_power_ctrl sim_power;

    u32 pmic_id;
    u32 is_hisi_hw;
#ifdef CONFIG_BALONG_MSG
    u32 src_mid;
    u32 dst_mid;
    struct semaphore sci_msg_sem;
    struct task_struct *sci_msg_task;
#endif
};

#define MAXSIMHPNUM 2

#define SIM_HOTPLUG_STATE_UNKNOWN 0
#define SIM_HOTPLUG_STATE_IN 1
#define SIM_HOTPLUG_STATE_CLEAR 0

#define HI6421_IRQ_LDO_DISABLE 0
#define HI6421_IRQ_LDO16_ENABLE 0x08
#define HI6421_IRQ_LDO11_ENABLE 0x02
#define HI6421_IRQ_LDO16_HPD0_ENABLE 0x02

#define SIM_HPD_PD_LDO53_EN 5
#define SIM_HPD_PD_LDO54_EN 4
#define SIM_HPD_F_PD_EN 1
#define SIM_HPD_R_PD_EN 0

#define HI6423_IRQ_SIM0_ENABLE ((1U << SIM_HPD_F_PD_EN) | (1U << SIM_HPD_PD_LDO53_EN))
#define HI6423_IRQ_SIM0_SIM1_ENABLE \
    ((1U << SIM_HPD_F_PD_EN) | (1U << SIM_HPD_PD_LDO53_EN) | (1U << SIM_HPD_PD_LDO54_EN))

#define HI6423_IRQ_LDO_DISABLE 0
#define PMU_SIM_CTRL1_ADDRESS 0x0245

#define SIM_HPD_LEAVE 3
#define SIM_CARD_DET 4

#define STATUS_SIM 5
#define STATUS_SD 6
#define STATUS_NO_CARD 7
#define STATUS_SD2JTAG 8

#define REQUEST_CARD_STATUS 1
#define REQUEST_SIM1_CLASS_C_POWER_ON 2
#define REQUEST_SIM1_CLASS_B_POWER_ON 3
#define REQUEST_SIM1_POWER_OFF 4

#define SIM1_IO_MUX_SIMIO_USIM_NANOSDIO_SD_REQUEST 0x5   /* 设置gpio113为USIM,NanoSD为SD */
#define SIM1_IO_MUX_SIMIO_GPIO_NANOSDIO_USIM_REQUEST 0x6 /* 需要同时设置nanosd和sim的管脚复用关系 */
#define SIM1_IO_MUX_NANOSDIO_USIM_ONLY_REQUEST 0x7       /* 单独设置nanosd的管脚复用关系 */
#define SIM1_IO_MUX_SIMIO_USIM_ONLY_REQUEST 0x8          /* 单独设置gpio113-115的管脚复用关系 */
#define SIM1_IO_MUX_GPIO_DET_REQUEST 0x9                 /* gpio检测功能复用关系 */
#define SIM1_IO_MUX_IDLE_GPIO_REQUEST 0x10               /* idle gpio for nvt4857 */

#define HIMNTN_SD2JTAG 15
#define HIMNTN_SD2DJTAG 31

#define MAX_CNT ((u32)0xFFFFFF00)
#define DET_DEBOUNCE_CHECK_WAIT_TIME 300 // ms
#define HISILICON_SIM_HOTPLUG0 "hisilicon-sim-hotplug0"
#define HISILICON_SIM_HOTPLUG1 "hisilicon-sim-hotplug1"
#define CARD_MSG_IO_MUX_SUCC 12
#define CARD_MSG_IO_MUX_FAIL 13
#define SIMJTAG_MAGIC 0x55667788
#define DET_DEBOUNCE_DIFF_MAX 2

int hisi_sim_hotplug_init(struct hisi_sim_platform_info *info, struct spmi_device *pdev);
void remove_simhp_node(int sim_id);
int sci_send_msg(struct hisi_sim_platform_info *info, sci_msg_data_s *sci_msg);
u8 get_card1_status(struct hisi_sim_platform_info *info);

#endif /* __HISI_SIM_HOTPLUG_H */
