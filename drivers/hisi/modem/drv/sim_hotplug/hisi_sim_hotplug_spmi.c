/*
 * hisi_sim_hotplug.c - SIM HOTPLUG driver
 *
 * Copyright (C) 2012 huawei Ltd.
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License. See the file "COPYING" in the main directory of this
 * archive for more details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*lint -e752 -esym(502,*)*/
/*lint -e753 -esym(753,*)*/
/*lint -e528 -esym(528,*)*/
/*lint -save -e713 -e734 -e502 -e774 -e838 -e438 -e701 -e64 -e826 -e838 -e715 -e613 -e747 -e838 -e732 -e785 -e647 -e528 -e753 -e752 */

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
#include <linux/wakelock.h>
#include "hisi_sim_hotplug.h"

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
#include <linux/compat.h>
#include "securec.h"
#include <adrv.h>
#include <bsp_print.h>




//#define TAG                     "[SIM_HOTPLUG]"
//#define LOGI(fmt, ...)          printk(KERN_INFO TAG" "fmt, ##__VA_ARGS__)
//#define LOGE(fmt, ...)          printk(KERN_ERR  TAG" "fmt, ##__VA_ARGS__)

#undef THIS_MODU
#define THIS_MODU mod_simhotplug

#define LOGE(fmt, ...)          (void)bsp_err("<%s>"fmt, __FUNCTION__,##__VA_ARGS__)
#define LOGI(fmt, ...)          (void)bsp_err("<%s>"fmt, __FUNCTION__,##__VA_ARGS__)

enum sim_func_select_state
{
    NOT_USE_MULTI_FUNC      = 0,
    USE_MULTI_FUNC          = 1,
    SELECT_STATE_OTHER      = 2,
};

// DET PIN normal close or normal open, NORMAL means card tray not insert.
enum sim_det_normal_direction
{
    NORMAL_CLOSE    = 0,
    NORMAL_OPEN     = 1,
    NORMAL_BUTT     = 2,
};

enum sim_card_tray_style
{
    SINGLE_CARD_IN_ONE_TRAY = 0,
    TWO_CARDS_IN_ONE_TRAY   = 1,
    CARD_TRAY_BUTT          = 2,
};

struct hisi_sim_hotplug_info
{
    struct workqueue_struct *sim_hotplug_hpd_wq;
    struct workqueue_struct *sim_hotplug_det_wq;
    struct workqueue_struct *sim_debounce_delay_wq;
    struct workqueue_struct *sim_sci_msg_wq;
    struct mutex            sim_hotplug_lock;
    struct wake_lock        sim_hotplug_wklock;
    struct work_struct      sim_hotplug_hpd_work;
    struct work_struct      sim_hotplug_det_work;
    struct delayed_work     sim_debounce_delay_work;
    struct work_struct      sim_sci_msg_work;

    int     hpd_irq;
    int     det_irq;
    int     det_gpio;
    int     old_det_gpio_level;
    int     det_debounce_checking;  //indicate det debounce is checking or not

    u32     sim_id;
    u32     card_tray_style;
    u32     send_msg_to_cp;
    u32     send_card_out_msg;
    u32     hpd_interrupt_to_close_ldo;
    u32     hpd_debounce_wait_time;
    u32     det_high_debounce_wait_time;    // when det gpio level is high(1)
    u32     det_low_debounce_wait_time;     // when det gpio level is low(0)
    u32     det_normal_direction;   // 1 indicats open by default(high level), 0 indicats closed by default
    u32     allocate_gpio;
    u32     shared_det_irq;
    u32     factory_send_msg_to_cp;
    u32     mux_sdsim;

    u32     pmu_status1_address;
    u32     pmu_irq_address;
    u32     pmu_irq_mask_address;
    u32     pmu_sim_ctrl_address;
    u32     pmu_sim_deb_ctrl_address;
    u32     pmu_ldo11_onoff_eco_address;
    u32     pmu_ldo11_vset_address;
    u32     pmu_ldo12_pull_down;
    u32     pmu_ldo12_onoff_eco_address;
    u32     pmu_ldo12_vset_address;
    u32     pmu_ldo11_onoff_value;
    u32     pmu_ldo11_vset_value;
    u32     pmu_ldo12_onoff_value;
    u32     pmu_ldo12_vset_value;

    u8      sim_pluged;
    u8      func_sel_state;         // 1 indicates on, 0 indicates off, 2 indicates no gpio set
};


#define SIM_HOTPLUG_STATE_UNKNOWN           (0)
#define SIM_HOTPLUG_STATE_IN                (1)
#define SIM_HOTPLUG_STATE_CLEAR             (0)

#define HI6421_IRQ_LDO_DISABLE              0
#define HI6421_IRQ_LDO16_ENABLE             0x08
#define PMU_SIM_CTRL1_ADDRESS               0x0245

#define SIM_HPD_LEAVE                       3
#define SIM_CARD_DET                        4

#define STATUS_SIM                          5
#define STATUS_SD                           6
#define STATUS_NO_CARD                      7
#define STATUS_SD2JTAG                      8

#define REQUEST_CARD_STATUS                 1

#define HIMNTN_SD2JTAG                      15
#define HIMNTN_SD2DJTAG                     31


#define MAX_CNT                             ((u32)0xFFFFFF00)
#define DET_DEBOUNCE_CHECK_WAIT_TIME        (300)   //ms
#define HISILICON_SIM_HOTPLUG0              "hisilicon-sim-hotplug0"
#define HISILICON_SIM_HOTPLUG1              "hisilicon-sim-hotplug1"


struct
{
    u8      sim0_det_normal_direction;
    u8      sim0_pluged;
    u8      sim1_det_normal_direction;
    u8      sim1_pluged;
    u32     sim0_leave_cnt;
    u32     sim0_out_cnt;
    u32     sim0_in_cnt;
    u32     sim0_det_cnt;
    u32     sim1_leave_cnt;
    u32     sim1_out_cnt;
    u32     sim1_in_cnt;
    u32     sim1_det_cnt;
} sim_plug_state;

static const char *work_name[] =
{
    "sim0_debounce_check",
    "sim1_debounce_check"
};

struct kobject *sim_kobj    = NULL;
static int sim0_state       = SIM_HOTPLUG_STATE_UNKNOWN;
static int sim1_state       = SIM_HOTPLUG_STATE_UNKNOWN;
static int sim2_state       = SIM_HOTPLUG_STATE_UNKNOWN;


/* static funciton declare */
static char* simplug_to_string(u32 plug);
static char* direction_to_string(u32 direction);
static char* func_sel_state_to_string(u32 sel_state);
static char* card_tray_style_to_string(u32 card_tray);
static char* card_status_to_string(int status);
static u8 get_card1_status(struct hisi_sim_hotplug_info *info);
int sim_hotplug_sdmux_init(struct hisi_sim_hotplug_info *info);
static int sd2jtag_enable = 0;

/* funciton implement */
static char* simplug_to_string(u32 plug)
{
    switch (plug)
    {
    case SIM_CARD_OUT:
        return "SIM_CARD_OUT";

    case SIM_CARD_IN:
        return "SIM_CARD_IN";

    default:
        return "SIM_PLUG_ERROR";
    }
}

static char* direction_to_string(u32 direction)
{
    switch (direction)
    {
    case NORMAL_CLOSE:
        return "NORMAL_CLOSE";

    case NORMAL_OPEN:
        return "NORMAL_OPEN";

    case NORMAL_BUTT:
    default:
        return "NORMAL_BUTT";
    }
}

static char* func_sel_state_to_string(u32 sel_state)
{
    switch (sel_state)
    {
    case NOT_USE_MULTI_FUNC:
        return "NOT_USE_MULTI_FUNC";

    case USE_MULTI_FUNC:
        return "USE_MULTI_FUNC";

    case SELECT_STATE_OTHER:
    default:
        return "SELECT_STATE_OTHER";
    }
}

static char* card_tray_style_to_string(u32 card_tray)
{
    switch (card_tray)
    {
    case SINGLE_CARD_IN_ONE_TRAY:
        return "SINGLE_CARD_IN_ONE_TRAY";

    case TWO_CARDS_IN_ONE_TRAY:
        return "TWO_CARDS_IN_ONE_TRAY";

    case CARD_TRAY_BUTT:
    default:
        return "CARD_TRAY_STYLE_UNKNOWN";
    }
}

static char* card_status_to_string(int status)
{
    switch (status)
    {
    case STATUS_SIM:
        return "STATUS_SIM";

    case STATUS_SD:
        return "STATUS_SD";

    case STATUS_SD2JTAG:
        return "STATUS_SD2JTAG";

    case STATUS_NO_CARD:
    default:
        return "STATUS_NO_CARD";
    }
}

static void sim_pmu_hpd_read(struct hisi_sim_hotplug_info *info)
{
    u32 pmu_status1         = 0;
    u32 pmu_irq             = 0;
    u32 pmu_irq_mask        = 0;
    u32 pmu_sim_ctrl        = 0;
    u32 pmu_sim_deb_ctrl    = 0;
    u32 pmu_ldo11_onoff     = 0;
    u32 pmu_ldo11_vset      = 0;
    u32 pmu_ldo12_onoff     = 0;
    u32 pmu_ldo12_vset      = 0;

    pmu_status1         = hisi_pmic_reg_read(info->pmu_status1_address);
    pmu_irq             = hisi_pmic_reg_read(info->pmu_irq_address);
    pmu_irq_mask        = hisi_pmic_reg_read(info->pmu_irq_mask_address);
    pmu_sim_ctrl        = hisi_pmic_reg_read(info->pmu_sim_ctrl_address);
    pmu_sim_deb_ctrl    = hisi_pmic_reg_read(info->pmu_sim_deb_ctrl_address);
    pmu_ldo11_onoff     = hisi_pmic_reg_read(info->pmu_ldo11_onoff_eco_address);
    pmu_ldo11_vset      = hisi_pmic_reg_read(info->pmu_ldo11_vset_address);
    pmu_ldo12_onoff     = hisi_pmic_reg_read(info->pmu_ldo12_onoff_eco_address);
    pmu_ldo12_vset      = hisi_pmic_reg_read(info->pmu_ldo12_vset_address);

    LOGI("pmu_status1: 0x%02X, pmu_irq: 0x%02X, pmu_irq_mask: 0x%02X, "
            "pmu_sim_ctrl: 0x%02X, pmu_sim_deb_ctrl: 0x%02X\n",
            pmu_status1, pmu_irq, pmu_irq_mask, pmu_sim_ctrl, pmu_sim_deb_ctrl);

    LOGI("pmu_ldo11_onoff: 0x%02X, pmu_ldo11_vset: 0x%02X, "
            "pmu_ldo12_onoff: 0x%02X, pmu_ldo12_vset: 0x%02X\n",
            pmu_ldo11_onoff, pmu_ldo11_vset, pmu_ldo12_onoff, pmu_ldo12_vset);
}

static void sim_pmu_hpd_write(struct hisi_sim_hotplug_info *info)
{
    if (NULL == info)
    {
        LOGE("info is null.\n");
        return;
    }

    if (0xFF == info->hpd_interrupt_to_close_ldo || 0x00 == info->hpd_interrupt_to_close_ldo)
    {
        LOGE("hpd_interrupt_to_close_ldo: 0x%02X\n", info->hpd_interrupt_to_close_ldo);
        return;
    }

    // set close ldo when interrupt happened
    if (SIM1 == info->sim_id)
    {
        if (1 == info->mux_sdsim)
        {
            if (sd2jtag_enable)
            {
                LOGI("HIMNTN_SD2JTAG is 1\n");
                //disable ldo12, ldo16
                hisi_pmic_reg_write(info->pmu_sim_ctrl_address, HI6421_IRQ_LDO_DISABLE);
                hisi_pmic_reg_write(PMU_SIM_CTRL1_ADDRESS,      HI6421_IRQ_LDO_DISABLE);
            }
            else
            {
                LOGI("HIMNTN_SD2JTAG is 0\n");
                //enable ldo12, ldo16, means falling edge hardware close ldo
                if(info->pmu_ldo12_pull_down == 1)
                {
                    hisi_pmic_reg_write(info->pmu_sim_ctrl_address, info->hpd_interrupt_to_close_ldo);
                }
                else
                {
                    hisi_pmic_reg_write(info->pmu_sim_ctrl_address, 0x02);
                }
                hisi_pmic_reg_write(PMU_SIM_CTRL1_ADDRESS,      HI6421_IRQ_LDO16_ENABLE);
            }
        }
        else
        {
            hisi_pmic_reg_write(info->pmu_sim_ctrl_address, info->hpd_interrupt_to_close_ldo);
        }
    }
    else
    {
        hisi_pmic_reg_write(info->pmu_sim_ctrl_address,         info->hpd_interrupt_to_close_ldo);
    }
    // set debounce waiting time
    hisi_pmic_reg_write(info->pmu_sim_deb_ctrl_address,     info->hpd_debounce_wait_time);
}

static int sim_read_pmu_dts(struct hisi_sim_hotplug_info *info, struct device_node *np)
{
    int ret                         = 0;
    u32 hpd_interrupt_to_close_ldo  = 0;
    u32 hpd_debounce_wait_time      = 0;
    u32 pmu_status1_address         = 0;
    u32 pmu_irq_address             = 0;
    u32 pmu_irq_mask_address        = 0;
    u32 pmu_sim_ctrl_address        = 0;
    u32 pmu_sim_deb_ctrl_address    = 0;
    u32 pmu_ldo11_onoff_eco_address = 0;
    u32 pmu_ldo11_vset_address      = 0;
    u32 pmu_ldo11_onoff_value       = 0;
    u32 pmu_ldo11_vset_value        = 0;
    u32 pmu_ldo12_onoff_eco_address = 0;
    u32 pmu_ldo12_vset_address      = 0;
    u32 pmu_ldo12_onoff_value       = 0;
    u32 pmu_ldo12_vset_value        = 0;
    u32 pmu_ldo12_pull_down         = 1;
    ret = of_property_read_u32(np, "hpd_interrupt_to_close_ldo", &hpd_interrupt_to_close_ldo);
    if (ret < 0)
    {
        LOGE("failed to read hpd_interrupt_to_close_ldo.\n");
        return ret;
    }
    info->hpd_interrupt_to_close_ldo = hpd_interrupt_to_close_ldo;

    ret = of_property_read_u32(np, "hpd_debounce_wait_time", &hpd_debounce_wait_time);
    if (ret < 0)
    {
        LOGE("failed to read hpd_debounce_wait_time.\n");
        return ret;
    }
    info->hpd_debounce_wait_time = hpd_debounce_wait_time;

    LOGI("hpd_interrupt_to_close_ldo: 0x%02X, hpd_debounce_wait_time: 0x%02X\n",
            hpd_interrupt_to_close_ldo, hpd_debounce_wait_time);

    ret = of_property_read_u32(np, "pmu_status1_address", &pmu_status1_address);
    if (ret < 0)
    {
        LOGE("failed to read pmu_status1_address.\n");
        return ret;
    }
    info->pmu_status1_address = pmu_status1_address;

    ret = of_property_read_u32(np, "pmu_irq_address", &pmu_irq_address);
    if (ret < 0)
    {
        LOGE("failed to read pmu_irq_address.\n");
        return ret;
    }
    info->pmu_irq_address = pmu_irq_address;

    ret = of_property_read_u32(np, "pmu_irq_mask_address", &pmu_irq_mask_address);
    if (ret < 0)
    {
        LOGE("failed to read pmu_irq_mask_address.\n");
        return ret;
    }
    info->pmu_irq_mask_address = pmu_irq_mask_address;

    ret = of_property_read_u32(np, "pmu_sim_ctrl_address", &pmu_sim_ctrl_address);
    if (ret < 0)
    {
        LOGE("failed to read pmu_sim_ctrl_address.\n");
        return ret;
    }
    info->pmu_sim_ctrl_address = pmu_sim_ctrl_address;

    ret = of_property_read_u32(np, "pmu_sim_deb_ctrl_address", &pmu_sim_deb_ctrl_address);
    if (ret < 0)
    {
        LOGE("failed to read pmu_sim_deb_ctrl_address.\n");
        return ret;
    }
    info->pmu_sim_deb_ctrl_address = pmu_sim_deb_ctrl_address;

    ret = of_property_read_u32(np, "pmu_ldo11_onoff_eco_address", &pmu_ldo11_onoff_eco_address);
    if (ret < 0)
    {
        LOGE("failed to read pmu_ldo11_onoff_eco_address.\n");
        return ret;
    }
    info->pmu_ldo11_onoff_eco_address = pmu_ldo11_onoff_eco_address;

    ret = of_property_read_u32(np, "pmu_ldo11_vset_address", &pmu_ldo11_vset_address);
    if (ret < 0)
    {
        LOGE("failed to read pmu_ldo11_vset_address.\n");
        return ret;
    }
    info->pmu_ldo11_vset_address = pmu_ldo11_vset_address;

    ret = of_property_read_u32(np, "pmu_ldo12_onoff_eco_address", &pmu_ldo12_onoff_eco_address);
    if (ret < 0)
    {
        LOGE("failed to read pmu_ldo12_onoff_eco_address.\n");
        return ret;
    }
    info->pmu_ldo12_onoff_eco_address = pmu_ldo12_onoff_eco_address;

    ret = of_property_read_u32(np, "pmu_ldo12_vset_address", &pmu_ldo12_vset_address);
    if (ret < 0)
    {
        LOGE("failed to read pmu_ldo12_vset_address.\n");
        return ret;
    }
    info->pmu_ldo12_vset_address = pmu_ldo12_vset_address;

    ret = of_property_read_u32(np, "pmu_ldo11_onoff_value", &pmu_ldo11_onoff_value);
    if (ret < 0)
    {
        LOGE("failed to read pmu_ldo11_onoff_value.\n");
        return ret;
    }
    info->pmu_ldo11_onoff_value = pmu_ldo11_onoff_value;

    ret = of_property_read_u32(np, "pmu_ldo11_vset_value", &pmu_ldo11_vset_value);
    if (ret < 0)
    {
        LOGE("failed to read pmu_ldo11_vset_value.\n");
        return ret;
    }
    info->pmu_ldo11_vset_value = pmu_ldo11_vset_value;

    ret = of_property_read_u32(np, "pmu_ldo12_onoff_value", &pmu_ldo12_onoff_value);
    if (ret < 0)
    {
        LOGE("failed to read pmu_ldo12_onoff_value.\n");
        return ret;
    }
    info->pmu_ldo12_onoff_value = pmu_ldo12_onoff_value;

    ret = of_property_read_u32(np, "pmu_ldo12_vset_value", &pmu_ldo12_vset_value);
    if (ret < 0)
    {
        LOGE("failed to read pmu_ldo12_vset_value.\n");
        return ret;
    }
    info->pmu_ldo12_vset_value = pmu_ldo12_vset_value;

    /*为了区分v4和v4之前的版本，默认配置为1，v4分支以后由终端定制DTS,根据DTS实际情况赋值*/
    ret = of_property_read_u32(np, "pmu_ldo12_pull_down", &pmu_ldo12_pull_down);
    if (ret < 0)
    {
        LOGI("read pmu_ldo12_pull_down fail, ldo12 pull down default.\n");
        info->pmu_ldo12_pull_down = 1;
    }
    else
    {
        LOGI("read pmu_ldo12_pull_down use dts value 0x%x\n",pmu_ldo12_pull_down);
        info->pmu_ldo12_pull_down = pmu_ldo12_pull_down;
    }
    /*最后一个分支可能读不到对应的dts，不判断ret，能走到最后说明前面已经返回成功*/
    LOGI("pmu_status1_address: 0x%02X, pmu_irq_address: 0x%02X, pmu_irq_mask_address: 0x%02X, "
            "pmu_sim_ctrl_address: 0x%02X, pmu_sim_deb_ctrl_address: 0x%02X\n",
            pmu_status1_address, pmu_irq_address, pmu_irq_mask_address, pmu_sim_ctrl_address, pmu_sim_deb_ctrl_address);

    LOGI("pmu_ldo11_onoff_eco_address: 0x%02X, pmu_ldo11_vset_address: 0x%02X, "
            "pmu_ldo12_onoff_eco_address: 0x%02X, pmu_ldo12_vset_address: 0x%02X\n",
            pmu_ldo11_onoff_eco_address, pmu_ldo11_vset_address, pmu_ldo12_onoff_eco_address, pmu_ldo12_vset_address);

    LOGI("pmu_ldo11_onoff_value: 0x%02X, pmu_ldo11_vset_value: 0x%02X, "
            "pmu_ldo12_onoff_value: 0x%02X, pmu_ldo12_vset_value: 0x%02X\n",
            pmu_ldo11_onoff_value, pmu_ldo11_vset_value, pmu_ldo12_onoff_value, pmu_ldo12_vset_value);

    LOGI("pmu_ldo12_pull_down: 0x%02X", info->pmu_ldo12_pull_down);

    return 0;
}

static int sim_pmu_hpd_init(struct hisi_sim_hotplug_info *info, struct device_node *np)
{
    int ret = -1;

    LOGI("hpd initializing.\n");

    ret = sim_read_pmu_dts(info, np);
    sim_pmu_hpd_write(info);
    sim_pmu_hpd_read(info);

    return ret;
}

static ssize_t sim_plug_state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    /*coverity[secure_coding]*/
    memcpy_s(buf, sizeof(sim_plug_state), &sim_plug_state, sizeof(sim_plug_state));
    return (ssize_t)sizeof(sim_plug_state);
}

static ssize_t sim0_hotplug_state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    /*coverity[secure_coding]*/
    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", sim0_state);
}

static ssize_t sim0_hotplug_state_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int val = 0;

    if (kstrtoint(buf, 10, &val))
    {
        LOGE("EINVAL!\n");
        return -EINVAL;
    }

    LOGI("old sim0_state: %d, val: %d\n", sim0_state, val);

    sim0_state = val;

    return (ssize_t)count;
}

static ssize_t sim1_hotplug_state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    /*coverity[secure_coding]*/
    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", sim1_state);
}

static ssize_t sim1_hotplug_state_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int val = 0;

    if (kstrtoint(buf, 10, &val))
    {
        LOGE("EINVAL!\n");
        return -EINVAL;
    }

    LOGI("old sim1_state: %d, val: %d\n", sim1_state, val);

    sim1_state = val;

    return (ssize_t)count;
}

static ssize_t sim2_hotplug_state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    /*coverity[secure_coding]*/
    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", sim2_state);
}

static ssize_t sim2_hotplug_state_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int val = 0;

    if (kstrtoint(buf, 10, &val))
    {
        LOGE("EINVAL!\n");
        return -EINVAL;
    }

    LOGI("old sim2_state: %d, val: %d\n", sim2_state, val);

    sim2_state = val;

    return (ssize_t)count;
}

static struct kobj_attribute sim0_attribute =
    __ATTR(sim0_hotplug_state, 0664, sim0_hotplug_state_show, sim0_hotplug_state_store);

static struct kobj_attribute sim1_attribute =
    __ATTR(sim1_hotplug_state, 0664, sim1_hotplug_state_show, sim1_hotplug_state_store);

static struct kobj_attribute sim2_attribute =
    __ATTR(sim2_hotplug_state, 0664, sim2_hotplug_state_show, sim2_hotplug_state_store);

static struct kobj_attribute sim_plug_state_attribute =
    __ATTR(sim_hotplug_state, 0664, sim_plug_state_show, NULL);

static struct attribute *attrs[] =
{
    &sim0_attribute.attr,
    &sim1_attribute.attr,
    &sim2_attribute.attr,
    &sim_plug_state_attribute.attr,
    NULL
};

static struct attribute_group sim_attr_group =
{
    .attrs = attrs
};

static int sys_add_sim_node(void)
{
    int retval;
    LOGI("adding sim sys node.\n");

    /* wait for kernel_kobj node ready: */
    while (NULL == kernel_kobj)
    {
        msleep(1000);
    }

    /* Create kobject named "sim",located under /sys/kernel/ */
    sim_kobj = kobject_create_and_add("sim", kernel_kobj);
    if (!sim_kobj)
    {
        return -ENOMEM;
    }

    /* Create the files associated with this kobject */
    retval = sysfs_create_group(sim_kobj, &sim_attr_group);
    if (retval)
    {
        kobject_put(sim_kobj);
    }

    LOGI("retval: %d\n", retval);
    return retval;
}

static void update_sim_state_info(int sim_id, int sim_pluged)
{
    if (SIM0 == sim_id)
    {
        if (SIM_CARD_IN == sim_pluged)
        {
            sim0_state = SIM_HOTPLUG_STATE_IN;
        }
        else
        {
            sim0_state = SIM_HOTPLUG_STATE_CLEAR;
        }
    }
    else if (SIM1 == sim_id)
    {
        if (SIM_CARD_IN == sim_pluged)
        {
            sim1_state = SIM_HOTPLUG_STATE_IN;
        }
        else
        {
            sim1_state = SIM_HOTPLUG_STATE_CLEAR;
        }
    }
    else
    {
        LOGE("invalid sim_id: %d\n", sim_id);
    }
}

static void update_sim_hotplug_count(u32 sim_id, u8 sim_pluged)
{
    if (SIM0 == sim_id)
    {
        switch (sim_pluged)
        {
        case SIM_HPD_LEAVE:
            if (sim_plug_state.sim0_leave_cnt < MAX_CNT)
            {
                sim_plug_state.sim0_leave_cnt++;
            }
            break;

        case SIM_CARD_OUT:
            if (sim_plug_state.sim0_out_cnt < MAX_CNT)
            {
                sim_plug_state.sim0_out_cnt++;
            }
            break;

        case SIM_CARD_IN:
            if (sim_plug_state.sim0_in_cnt < MAX_CNT)
            {
                sim_plug_state.sim0_in_cnt++;
            }
            break;

        case SIM_CARD_DET:
            if (sim_plug_state.sim0_det_cnt < MAX_CNT)
            {
                sim_plug_state.sim0_det_cnt++;
            }
            break;

        default:
            break;
        }
    }
    else if (SIM1 == sim_id)
    {
        switch (sim_pluged)
        {
        case SIM_HPD_LEAVE:
            if (sim_plug_state.sim1_leave_cnt < MAX_CNT)
            {
                sim_plug_state.sim1_leave_cnt++;
            }
            break;

        case SIM_CARD_OUT:
            if (sim_plug_state.sim1_out_cnt < MAX_CNT)
            {
                sim_plug_state.sim1_out_cnt++;
            }
            break;

        case SIM_CARD_IN:
            if (sim_plug_state.sim1_in_cnt < MAX_CNT)
            {
                sim_plug_state.sim1_in_cnt++;
            }
            break;

        case SIM_CARD_DET:
            if (sim_plug_state.sim1_det_cnt < MAX_CNT)
            {
                sim_plug_state.sim1_det_cnt++;
            }
            break;

        default:
            break;
        }
    }
    else
    {
        LOGE("invalid sim_id: %d\n", __func__, sim_id);
    }
}

static void save_sim_status(struct hisi_sim_hotplug_info *info, u32 sim_id, u32 sim_pluged)
{
    if (SIM0 == info->sim_id)
    {
        sim_plug_state.sim0_pluged = (u8)sim_pluged;
    }
    else if (SIM1 == info->sim_id)
    {
        sim_plug_state.sim1_pluged = (u8)sim_pluged;
    }
    else
    {
        LOGE(" save sim%d failed!\n", info->sim_id);
    }

    LOGI("sim%d, sim_pluged: %s\n", sim_id, simplug_to_string(sim_pluged));
}

static int sim_set_active(struct hisi_sim_hotplug_info *info)
{
    LOGI("sim%d, sim_pluged: %s\n",
            info->sim_id, simplug_to_string(info->sim_pluged));
    return 0;
}

static int sim_set_inactive(struct hisi_sim_hotplug_info *info)
{
    LOGI("%s: sim%d, sim_pluged: %s\n",
            __func__, info->sim_id, simplug_to_string(info->sim_pluged));
    return 0;
}

void send_det_msg_to_core(struct hisi_sim_hotplug_info *info, u32 channel_id, u8 sim_pluged)
{
    int ret = 0;
    u8 sim1_type = STATUS_NO_CARD;

    if (1 == info->send_msg_to_cp)
    {
        if(info->mux_sdsim == 0) //if mux_sdsim, report anyway
        {
            LOGI("sim%d, sim_pluged = %s, bsp_icc_send to CP.\n",
                        info->sim_id, simplug_to_string(sim_pluged));
            ret = bsp_icc_send(ICC_CPU_MODEM, channel_id, &sim_pluged, sizeof(sim_pluged));
            if (ret != sizeof(sim_pluged))
            {
                LOGE("sim%d, bsp_icc_send failed.\n", info->sim_id);
            }
        }
        else
        {
            sim1_type = get_card1_status(info);

            if( SIM_CARD_OUT == sim_pluged && SIM1 == info->sim_id)//report out directly
            {
                LOGI("sim%d, sim_pluged = %s, bsp_icc_send to CP.\n",
                        info->sim_id, simplug_to_string(sim_pluged));

                ret = bsp_icc_send(ICC_CPU_MODEM, channel_id, &sim_pluged, sizeof(sim_pluged));
                if (ret != sizeof(sim_pluged))
                {
                    LOGE("sim%d, bsp_icc_send failed.\n", info->sim_id);
                }
                return;
            }

            if(STATUS_SIM == sim1_type)//if sim, report in directly
            {
                LOGI("sim%d, sim_pluged = %s, bsp_icc_send to CP.\n",
                    info->sim_id, simplug_to_string(sim_pluged));
                ret = bsp_icc_send(ICC_CPU_MODEM, channel_id, &sim_pluged, sizeof(sim_pluged));
                if (ret != sizeof(sim_pluged))
                {
                    LOGE("sim%d, bsp_icc_send failed.\n", info->sim_id);
                }
                return;
            }
        }
    }
    else
    {
        LOGI("send_msg_to_cp is not 1, so not bsp_icc_send to CP.\n");
    }

}

static void hisi_sim_det_msg_to_ccore(struct hisi_sim_hotplug_info *info, u8 sim_pluged)
{
    u32 channel_id      = 0;
    int det_gpio_level  = 0;

    if (NULL == info)
    {
        LOGE("info is null.\n");
        return;
    }

    if (SIM_CARD_OUT != sim_pluged && SIM_CARD_IN != sim_pluged)
    {
        LOGE("sim_pluged is invalid and it is %s.\n", simplug_to_string(sim_pluged));
        return;
    }

    det_gpio_level = gpio_get_value(info->det_gpio);
    LOGI("sim%d, old_det_gpio_level: %d, det_gpio_level: %d\n",
            info->sim_id, info->old_det_gpio_level, det_gpio_level);
    if (det_gpio_level == info->old_det_gpio_level)
    {
        LOGI("det_gpio_level is same with old_det_gpio_level, so return.\n");
        return;
    }
    info->old_det_gpio_level = det_gpio_level;

    if (0 == info->factory_send_msg_to_cp)
    {
        LOGE("factory_send_msg_to_cp is 0, so return.\n");
        return;
    }

    update_sim_state_info(info->sim_id, sim_pluged);

    if (SIM0 == info->sim_id)
    {
        channel_id = SIM0_CHANNEL_ID;
    }
    else if (SIM1 == info->sim_id)
    {
        channel_id = SIM1_CHANNEL_ID;
    }
    else
    {
        LOGE("sim%d invalid.\n", info->sim_id);
        return;
    }

    send_det_msg_to_core(info, channel_id, sim_pluged);

}

static void hisi_sim_hpd_msg_to_ccore(struct hisi_sim_hotplug_info *info)
{
    u32 channel_id  = 0;
    s32 ret         = 0;
    u8 sim_state    = SIM_HPD_LEAVE;

    if (NULL == info)
    {
        LOGE("info is null.\n");
        return;
    }

    if (SIM0 == info->sim_id)
    {
        channel_id = SIM0_CHANNEL_ID;
    }
    else if (SIM1 == info->sim_id)
    {
        channel_id = SIM1_CHANNEL_ID;
    }
    else
    {
        LOGE("sim%d, error sim id, so return.\n", info->sim_id);
        return;
    }
    update_sim_hotplug_count(info->sim_id, sim_state);

    LOGI("sim%d, SIM_HPD_LEAVE bsp_icc_send to CP.\n", info->sim_id);
    ret = bsp_icc_send(ICC_CPU_MODEM, channel_id, &sim_state, sizeof(sim_state));
    if (ret != sizeof(sim_state))
    {
        LOGE("sim%d, bsp_icc_send failed.\n", info->sim_id);
    }
}

static void det_irq_set_sim_insert(struct hisi_sim_hotplug_info *info)
{
    info->sim_pluged = SIM_CARD_IN;

    LOGI("sim%d, SIM_CARD_IN\n", info->sim_id);
    save_sim_status(info, info->sim_id, info->sim_pluged);
    update_sim_hotplug_count(info->sim_id, info->sim_pluged);
    sim_set_active(info);
    hisi_sim_det_msg_to_ccore(info, SIM_CARD_IN);
}

static void det_irq_set_sim_remove(struct hisi_sim_hotplug_info *info)
{
    info->sim_pluged = SIM_CARD_OUT;

    LOGI("sim%d, SIM_CARD_OUT\n", info->sim_id);
    save_sim_status(info, info->sim_id, info->sim_pluged);
    update_sim_hotplug_count(info->sim_id, info->sim_pluged);
    sim_set_inactive(info);
    hisi_sim_det_msg_to_ccore(info, SIM_CARD_OUT);
}

static void sim_manage_sim_state(int det_gpio_level, struct hisi_sim_hotplug_info *info)
{
    LOGI("sim%d, old_det_gpio_level: %d, det_gpio_level: %d\n",
            info->sim_id, info->old_det_gpio_level, det_gpio_level);

    if (0 == det_gpio_level)
    {
        if (NORMAL_OPEN == info->det_normal_direction && SIM_CARD_OUT == info->sim_pluged)
        {
            det_irq_set_sim_insert(info);
        }
        else if (NORMAL_CLOSE == info->det_normal_direction && SIM_CARD_IN == info->sim_pluged)
        {
            det_irq_set_sim_remove(info);
        }
    }
    else
    {
        if (NORMAL_OPEN == info->det_normal_direction  && SIM_CARD_IN == info->sim_pluged)
        {
            det_irq_set_sim_remove(info);
        }
        else if (NORMAL_CLOSE == info->det_normal_direction && SIM_CARD_OUT == info->sim_pluged)
        {
            det_irq_set_sim_insert(info);
        }
    }
}

static void inquiry_sim_det_irq_reg(struct work_struct *work)
{
    struct hisi_sim_hotplug_info *info =
            container_of(work, struct hisi_sim_hotplug_info, sim_hotplug_det_work);
    int det_gpio_level = 0;
    u32 debounce_wait_time = 0;

    det_gpio_level = gpio_get_value(info->det_gpio);
    LOGI("sim%d, old sim_pluged: %s, det_gpio_level: %d\n",
            info->sim_id, simplug_to_string(info->sim_pluged), det_gpio_level);

    if (1 == info->send_msg_to_cp)
    {
        if (0 == info->det_debounce_checking && NULL != info->sim_debounce_delay_wq)
        {
            info->det_debounce_checking = 1;
            if (1 == det_gpio_level)
            {
                debounce_wait_time = info->det_high_debounce_wait_time;
            }
            else
            {
                debounce_wait_time = info->det_low_debounce_wait_time;
            }
            wake_lock_timeout(&info->sim_hotplug_wklock, msecs_to_jiffies(debounce_wait_time + 5));

            queue_delayed_work(info->sim_debounce_delay_wq,
                               &info->sim_debounce_delay_work,
                               msecs_to_jiffies(debounce_wait_time));
            LOGI("sim%d, det level %d debounce check begin.\n", info->sim_id, det_gpio_level);
        }
        else
        {
            LOGI("sim%d, det debounce check is ongoing.\n", info->sim_id);
        }
    }
    else
    {
        sim_manage_sim_state(det_gpio_level, info);
    }
}

static irqreturn_t sim_det_irq_handler(int irq, void *data)
{
    struct hisi_sim_hotplug_info *info = data;
    if (NULL == info)
    {
        LOGE("info is NULL.\n");
        return IRQ_HANDLED;
    }

    LOGI("sim%d sim_hotplug_det_work begin.\n", info->sim_id);
    update_sim_hotplug_count(info->sim_id, SIM_CARD_DET);

    queue_work(info->sim_hotplug_det_wq, &info->sim_hotplug_det_work);
    return IRQ_HANDLED;
}

static void sim_debounce_check_func(struct work_struct *work)
{
    struct hisi_sim_hotplug_info *info =
            container_of(work, struct hisi_sim_hotplug_info, sim_debounce_delay_work.work);
    int det_gpio_level = 0;

    det_gpio_level = gpio_get_value(info->det_gpio);

    LOGI("sim%d, old sim_pluged: %s, det_gpio_level: %d, det debounce check end.\n",
            info->sim_id, simplug_to_string(info->sim_pluged), det_gpio_level);
    sim_manage_sim_state(det_gpio_level, info);
    info->det_debounce_checking = 0;

}

static void sim_hpd_mask_irq(struct hisi_sim_hotplug_info *info, bool mask)
{
    struct irq_desc *sim_hpd_irq_desc;

    sim_hpd_irq_desc = irq_to_desc(info->hpd_irq);
    if (!sim_hpd_irq_desc)
    {
        LOGE("sim_hpd_irq_desc error\n");
        return;
    }

    if (mask)
    {
        /* mask interrupts */
        sim_hpd_irq_desc->irq_data.chip->irq_mask(&sim_hpd_irq_desc->irq_data);
    }
    else
    {
        /* unmask interrupts */
        sim_hpd_irq_desc->irq_data.chip->irq_unmask(&sim_hpd_irq_desc->irq_data);
    }
}

static irqreturn_t sim_hpd_irq_handler(int irq, void *data)
{
    struct hisi_sim_hotplug_info *info = data;
    if (NULL == info)
    {
        LOGE("info is NULL.\n");
        return IRQ_HANDLED;
    }

    LOGI("sim%d sim_hotplug_hpd_work.\n", info->sim_id);

    sim_hpd_mask_irq(info, 1);

    if (SIM0 == info->sim_id)
    {
        /* close ldo11 */
        //hisi_pmic_reg_write(info->pmu_ldo11_onoff_eco_address, HI6421_IRQ_LDO11_VALUE);
        LOGI("IRQ HPD0 happend\n");
    }
    else if (SIM1 == info->sim_id)
    {
        /* close ldo12 */
        //hisi_pmic_reg_write(info->pmu_ldo12_onoff_eco_address, HI6421_IRQ_LDO12_VALUE);
        LOGI("IRQ HPD1 happend\n");
    }

    queue_work(info->sim_hotplug_hpd_wq, &info->sim_hotplug_hpd_work);
    return IRQ_HANDLED;
}

static void inquiry_sim_hpd_irq_reg(struct work_struct *work)
{
    struct hisi_sim_hotplug_info *info =
            container_of(work, struct hisi_sim_hotplug_info, sim_hotplug_hpd_work);

    hisi_sim_hpd_msg_to_ccore(info);
    sim_hpd_mask_irq(info, 0);
}

static int sim_init_debounce_check_wq(struct hisi_sim_hotplug_info *info)
{
    int ret = 0;

    if (NULL == info)
    {
        LOGE("info is NULL.\n");
        ret = -EINVAL;
    }
    else
    {
        LOGI("for sim%d\n", info->sim_id);

        if (1 == info->send_msg_to_cp)
        {
            info->det_debounce_checking = 0;
            info->sim_debounce_delay_wq = create_singlethread_workqueue(work_name[info->sim_id]);
            if (NULL == info->sim_debounce_delay_wq)
            {
                LOGE("sim_debounce_delay_wq create failed.\n");
                ret = -ENOMEM;
            }
            else
            {
                INIT_DELAYED_WORK(&(info->sim_debounce_delay_work), sim_debounce_check_func);
            }
        }
    }

    LOGI("ret: %d\n", ret);
    return ret;
}

static int sim_hotplug_dts_read(struct hisi_sim_hotplug_info *info, struct device_node *np)
{
    int ret                     = 0;
    u32 sim_id                  = 0;
    u32 allocate_gpio           = 0;
    u32 shared_det_irq          = 0;
    u32 send_msg_to_cp          = 0;
    u32 send_card_out_msg       = 0;
    u32 card_tray_style         = 0;
    u32 factory_send_msg_to_cp  = 0;
    u32 mux_sdsim               = 0;
    u32 det_high_debounce_wait_time = 0;
    u32 det_low_debounce_wait_time  = 0;
    const char *str_func_sel_state = NULL;
    const char *str_det_normal_direction = NULL;
    enum of_gpio_flags flags;

    LOGI("loading dts.\n");

    ret = of_property_read_u32_array(np, "sim_id", &sim_id, 1);
    if (ret)
    {
        LOGE("no sim hotplug sim_id, so return.\n");
        return ret;
    }
    info->sim_id = sim_id;

    ret = of_property_read_string(np, "func_sel_state", &str_func_sel_state);
    if (ret < 0)
    {
        LOGE("failed to read func_sel_state.\n");
        return ret;
    }

    /*lint -e421 -esym(421,*)*/
    if (0 == strcmp(str_func_sel_state, "use_multi_func"))
    {
        info->func_sel_state = USE_MULTI_FUNC;
    }
    else if (0 == strcmp(str_func_sel_state, "not_use_multi_func"))
    {
        info->func_sel_state = NOT_USE_MULTI_FUNC;
    }
    else
    {
        info->func_sel_state = SELECT_STATE_OTHER;
    }

    if (SELECT_STATE_OTHER == info->func_sel_state)
    {
        LOGI("func_sel_state is SELECT_STATE_OTHER\n");
        return -EINVAL;
    }

    ret = of_property_read_string(np, "det_normal_direction", &str_det_normal_direction);
    if (ret < 0)
    {
        LOGE("failed to read det_normal_direction.\n");
        return ret;
    }
    if (0 == strcmp(str_det_normal_direction, "open"))
    {
        info->det_normal_direction = NORMAL_OPEN;
    }
    else if (0 == strcmp(str_det_normal_direction, "closed"))
    {
        info->det_normal_direction = NORMAL_CLOSE;
    }
    else
    {
        info->det_normal_direction = NORMAL_BUTT;
    }
    /*lint -e421 +esym(421,*)*/

    if (NORMAL_BUTT == info->det_normal_direction)
    {
        LOGI("det_normal_direction is NORMAL_BUTT and set to NORMAL_OPEN.\n");
        info->det_normal_direction = NORMAL_OPEN;
    }

    /* read det pin gpio number */
    info->det_gpio = of_get_gpio_flags(np, 0, &flags);
    if (info->det_gpio < 0)
    {
        LOGE("det_gpio number: %d, error.\n", info->det_gpio);
        return info->det_gpio;
    }

    if (!gpio_is_valid(info->det_gpio))
    {
        LOGE("det_gpio number: %d, gpio_is_valid is invalid, error.\n", info->det_gpio);
        return -EINVAL;
    }
    info->det_irq = gpio_to_irq(info->det_gpio);

    if (of_property_read_u32(np, "allocate_gpio", &allocate_gpio))
    {
        LOGI("allocate_gpio property not found, using 0 as default.\n");
        allocate_gpio = 0;
    }
    info->allocate_gpio = allocate_gpio;

    if (of_property_read_u32(np, "shared_det_irq", &shared_det_irq))
    {
        LOGI("shared_det_irq property not found, using 0 as default.\n");
        shared_det_irq = 0;
    }
    info->shared_det_irq = shared_det_irq;

    ret = of_property_read_u32(np, "send_msg_to_cp", &send_msg_to_cp);
    if (ret < 0)
    {
        LOGI("failed to read send_msg_to_cp, so set it to 0.\n");
        send_msg_to_cp = 0;
    }
    info->send_msg_to_cp = send_msg_to_cp;

    ret = of_property_read_u32(np, "send_card_out_msg_when_init", &send_card_out_msg);
    if (ret < 0)
    {
        LOGI("failed to read send_card_out_msg_when_init, so set it to 0.\n");
        send_card_out_msg = 0;
    }
    info->send_card_out_msg = send_card_out_msg;

    ret = of_property_read_u32(np, "card_tray_style", &card_tray_style);
    if (ret < 0)
    {
        LOGE("failed to read card_tray_style.\n");
        return ret;
    }
    info->card_tray_style = card_tray_style;

    ret = of_property_read_u32(np, "factory_send_msg_to_cp", &factory_send_msg_to_cp);
    if (ret < 0)
    {
        LOGI("failed to read factory_send_msg_to_cp, so set it to 0.\n");
        factory_send_msg_to_cp = 0;
    }
    info->factory_send_msg_to_cp = factory_send_msg_to_cp;

    ret = of_property_read_u32(np, "mux_sdsim", &mux_sdsim);
    if (ret < 0)
    {
        LOGI("failed to read mux_sdsim, so set it to 0.\n");
        mux_sdsim = 0;
    }

    info->mux_sdsim = mux_sdsim;

    ret = of_property_read_u32(np, "det_high_debounce_wait_time", &det_high_debounce_wait_time);
    if (ret < 0)
    {
        det_high_debounce_wait_time = DET_DEBOUNCE_CHECK_WAIT_TIME;
    }
    info->det_high_debounce_wait_time = det_high_debounce_wait_time;

    ret = of_property_read_u32(np, "det_low_debounce_wait_time", &det_low_debounce_wait_time);
    if (ret < 0)
    {
        det_low_debounce_wait_time = DET_DEBOUNCE_CHECK_WAIT_TIME;
    }
    info->det_low_debounce_wait_time = det_low_debounce_wait_time;

    LOGI("sim%d, func_sel_state: %s, det_gpio_number: %d, send_msg_to_cp: %d, det_normal_direction: %s, card_tray_style: %s\n",
            sim_id, func_sel_state_to_string(info->func_sel_state), info->det_gpio, send_msg_to_cp,
            direction_to_string(info->det_normal_direction), card_tray_style_to_string(card_tray_style));

    LOGI("sim%d, allocate_gpio: %d, shared_det_irq: %d, mux_sdsim: %d, send_card_out_msg: %d, factory_send_msg_to_cp: %d, det_debounce_wait_time, high: %d, low: %d\n",
            sim_id, allocate_gpio, shared_det_irq, mux_sdsim, send_card_out_msg, factory_send_msg_to_cp,
            det_high_debounce_wait_time, det_low_debounce_wait_time);

    return 0;
}

static int sim_hotplug_init(struct hisi_sim_hotplug_info *info)
{
    int ret = 0;

    LOGI("hotplug initializing.\n");

    if (SIM0 == info->sim_id)
    {
        sim_plug_state.sim0_det_normal_direction = (u8)(info->det_normal_direction);
        sim_plug_state.sim0_leave_cnt   = 0;
        sim_plug_state.sim0_out_cnt     = 0;
        sim_plug_state.sim0_in_cnt      = 0;
        sim_plug_state.sim0_det_cnt     = 0;
    }
    else if (SIM1 == info->sim_id)
    {
        sim_plug_state.sim1_det_normal_direction = (u8)(info->det_normal_direction);
        sim_plug_state.sim1_leave_cnt   = 0;
        sim_plug_state.sim1_out_cnt     = 0;
        sim_plug_state.sim1_in_cnt      = 0;
        sim_plug_state.sim1_det_cnt     = 0;
    }

    if (1 == info->allocate_gpio)
    {
        LOGI("sim%d, gpio_request_one, det_gpio: %d\n", info->sim_id, info->det_gpio);
        ret = gpio_request_one(info->det_gpio, GPIOF_IN, "sim_det");
        if (ret < 0)
        {
            LOGE("failed to gpio_request_one for sim%d det_gpio: %d.\n", info->sim_id, info->det_gpio);
            return ret;
        }
    }

    /* create det workqueue */
    info->sim_hotplug_det_wq = create_singlethread_workqueue("sim_hotplug_det");
    if (NULL == info->sim_hotplug_det_wq)
    {
        LOGE("failed to create work queue.\n");
        return -ENOMEM;
    }

    /*lint -e611 -esym(611,*)*/
    INIT_WORK(&info->sim_hotplug_det_work, (void *)inquiry_sim_det_irq_reg);
    /*lint -e611 +esym(611,*)*/

    ret = sim_init_debounce_check_wq(info);
    if (ret < 0)
    {
        LOGE("failed to sim_init_debounce_check_wq for sim%d.\n", info->sim_id);
        return ret;
    }

    ret = sim_hotplug_sdmux_init(info);
    if (ret < 0)
    {
        LOGE("sim_hotplug_sdmux_init failed, ret: %d\n", ret);
        return ret;
    }

    if (1 == info->shared_det_irq)
    {
        ret = request_threaded_irq(info->det_irq, sim_det_irq_handler, NULL,
                IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND | IRQF_SHARED,
                "sim_det", info);
    }
    else
    {
        ret = request_threaded_irq(info->det_irq, sim_det_irq_handler, NULL,
                IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND,
                "sim_det", info);
    }
    if (ret < 0)
    {
        LOGE("request_threaded_irq failed, ret: %d\n", ret);
    }

    return ret;
}

static int sim_state_init(struct hisi_sim_hotplug_info *info, struct device *dev)
{
    LOGI("state initializing.\n");

    if (0 == gpio_get_value(info->det_gpio))
    {
        info->old_det_gpio_level = 0;
        if (NORMAL_CLOSE == info->det_normal_direction)
        {
            info->sim_pluged = SIM_CARD_OUT;
            save_sim_status(info, info->sim_id, info->sim_pluged);
            sim_set_inactive(info);
            if (1 == info->send_card_out_msg)
            {
                info->old_det_gpio_level = 1;
                hisi_sim_det_msg_to_ccore(info, SIM_CARD_OUT);
            }
        }
        else
        {
            info->sim_pluged = SIM_CARD_IN;
            save_sim_status(info, info->sim_id, info->sim_pluged);
            sim_set_active(info);
        }
    }
    else
    {
        info->old_det_gpio_level = 1;
        if (NORMAL_CLOSE == info->det_normal_direction)
        {
            info->sim_pluged = SIM_CARD_IN;
            save_sim_status(info, info->sim_id, info->sim_pluged);
            sim_set_active(info);
        }
        else
        {
            info->sim_pluged = SIM_CARD_OUT;
            save_sim_status(info, info->sim_id, info->sim_pluged);
            sim_set_inactive(info);
            if (1 == info->send_card_out_msg)
            {
                info->old_det_gpio_level = 0;
                hisi_sim_det_msg_to_ccore(info, SIM_CARD_OUT);
            }
        }
    }

    mutex_init(&(info->sim_hotplug_lock));
    wake_lock_init(&info->sim_hotplug_wklock, WAKE_LOCK_SUSPEND, "android-simhotplug");

    return 0;
}

static int sim_hpd_init(struct hisi_sim_hotplug_info *info, struct spmi_device *pdev)
{
    int ret = 0;

    LOGI("hpd initializing.\n");

    if (NULL == pdev)
    {
        LOGE("pdev is NULL.\n");
        return -1;
    }

    if (SIM0 == info->sim_id)
    {
        info->hpd_irq = spmi_get_irq_byname(pdev, NULL, "hpd0_falling");
    }
    else if (SIM1 == info->sim_id)
    {
        info->hpd_irq = spmi_get_irq_byname(pdev, NULL, "hpd1_falling");
    }
    else
    {
        LOGE("sim%d, wrong sim id.\n", info->sim_id);
        return -EINVAL;
    }

    if (info->hpd_irq < 0)
    {
        return -ENOENT;
    }

    /* create hpd workqueue */
    info->sim_hotplug_hpd_wq = create_singlethread_workqueue("sim_hotplug_hpd");
    if (NULL == info->sim_hotplug_hpd_wq)
    {
        LOGE("sim_hotplug_hpd_wq failed to create work queue.\n");
        return -EINVAL;
    }

    /*lint -e611 -esym(611,*)*/
    INIT_WORK(&info->sim_hotplug_hpd_work, (void *)inquiry_sim_hpd_irq_reg);
    /*lint -e611 +esym(611,*)*/

    ret = request_threaded_irq(info->hpd_irq, sim_hpd_irq_handler, NULL,
            IRQF_NO_SUSPEND, "sim_hpd", info);
    if (ret < 0)
    {
        LOGE("sim%d hpd_irq failed to request_threaded_irq, ret: %d\n", info->sim_id, ret);
    }

    LOGI("sim%d hpd_irq: %d\n", info->sim_id, info->hpd_irq);
    return ret;
}

void notify_card_status(int32_t card_status)
{
    s32 ret         = 0;
    u8  sim_state   = SIM_CARD_IN;

    if (SIM_CARD_IN == card_status)
    {
        sim_state = (u8)card_status;
        if (SIM_CARD_IN != sim_plug_state.sim1_pluged)
        {
            LOGE("card1 not present, so return\n");
            return;
        }

        if (1 == sd2jtag_enable)
        {
            LOGE("sd2jtag_enable, so return\n");
            return;
        }

        LOGI("sim1, fake SIM_CARD_IN bsp_icc_send to CP\n");
        ret = bsp_icc_send(ICC_CPU_MODEM, SIM1_CHANNEL_ID, &sim_state, sizeof(sim_state));
        if (ret != sizeof(sim_state))
        {
            LOGE("sim1, bsp_icc_send failed.\n");
        }
    }
}

static u8 get_card1_status(struct hisi_sim_hotplug_info *info)
{
    u8 status = STATUS_NO_CARD;

    if (!info)
    {
        LOGE("info is NULL, as STATUS_NO_CARD return.\n");
        return status;
    }

    if (0 == info->mux_sdsim)
    {
        LOGI("sim1 mux_sdsim is 0, as STATUS_SIM return.\n");
        return STATUS_SIM;
    }

    return status;
}

s32 handle_msg_from_sci(u32 channel_id, u32 len, void *context)
{

    u32 request_id  = 0;
    s32 read_len    = 0;
    struct hisi_sim_hotplug_info *info = context;

    read_len = bsp_icc_read(channel_id, (u8*)&request_id, len);
    if ((u32)read_len != len)
    {
        LOGE("readed len(%d) != expected len(%d)\n", read_len, len);
        return -1;
    }

    LOGI("request_id: %d for simid: %d, mux_sdsim: %d.\n", request_id, info->sim_id, info->mux_sdsim);
    switch (request_id)
    {
        case REQUEST_CARD_STATUS:
            queue_work(info->sim_sci_msg_wq, &info->sim_sci_msg_work);
            break;

        default:
            break;
    }

    return 0;
}


static void sim_sci_msg_proc(struct work_struct *work)
{
    s32 ret         = 0;
    u8  status      = 0;
    u32 channel_id = 0;
    struct hisi_sim_hotplug_info *info =
            container_of(work, struct hisi_sim_hotplug_info, sim_sci_msg_work);

    if (SIM0 == info->sim_id)
    {
        channel_id = SIM0_CHANNEL_ID;
    }
    else if (SIM1 == info->sim_id)
    {
        channel_id = SIM1_CHANNEL_ID;
    }
    else
    {
        LOGE("sim%d invalid.\n", info->sim_id);
        return;
    }

    //status = sim1_type;
    status = get_card1_status(info);

    LOGI("bsp_icc_send status to cp, status: %d(%s), simid %d, mux_sdsim %d\n", status, card_status_to_string(status), info->sim_id, info->mux_sdsim);
    ret = bsp_icc_send(ICC_CPU_MODEM, channel_id, &status, sizeof(status));
    if (ret != sizeof(status))
    {
        LOGE("in REQUEST_CARD_STATUS, bsp_icc_send failed.\n");
    }

}



struct simhp_driver_data {
    spinlock_t read_lock;
    int32_t sim_id;
    int32_t simhp_dev_en;
    dev_t simhp_dev;
    struct cdev simhp_cdev;
    struct class *simhp_class;
};


static struct simhp_driver_data simhotplug_data;


static int simhotplug_open(struct inode *inode, struct file *filp)
{
    spin_lock_bh(&simhotplug_data.read_lock);

    if (simhotplug_data.simhp_dev_en)
    {
        LOGE("simhotplug device has been opened\n");
        spin_unlock_bh(&simhotplug_data.read_lock);
        return -EPERM;
    }

    simhotplug_data.simhp_dev_en = true;

    spin_unlock_bh(&simhotplug_data.read_lock);
    LOGI("success\n");

    return 0;
}

/*
 * this is main method to exchange data with user space,
 * including socket sync, get ip and port, adjust kernel flow
 *
 * return "int" by standard.
 */
static long simhotplug_ioctl(struct file *flip, unsigned int cmd, unsigned long arg)
{
    int rc = -EFAULT;
    void __user *argp = (void __user *)arg;
    int32_t card_status;

    if(NULL == argp)
    {
        LOGI("simhotplug_ioctl arg is null.\n");
        return rc;
    }
    switch (cmd)
    {
        case SIMHOTPLUG_IOC_INFORM_CARD_INOUT:
        {
            LOGI("simhotplug_ioctl card status inout.\n");

            if (copy_from_user(&card_status, argp, sizeof(card_status)))
                break;

            LOGI("simhotplug_ioctl card status card_status= %d.\n",card_status);

            notify_card_status(card_status);

            rc = 0;

            break;
        }
        default:
        {
            LOGE("unknown ioctl: %u\n", cmd);
            break;
        }
    }

    return rc;
}

/* support of 32bit userspace on 64bit platforms */
static long compat_simhotplug_ioctl(struct file *flip,
    unsigned int cmd, unsigned long arg)
{
    return simhotplug_ioctl(flip, cmd, (unsigned long) compat_ptr(arg));
}

static int simhotplug_release(struct inode *inode, struct file *filp)
{
    spin_lock_bh(&simhotplug_data.read_lock);
    simhotplug_data.simhp_dev_en = false;
    spin_unlock_bh(&simhotplug_data.read_lock);
    LOGI("success\n");
    return 0;
}


static const struct file_operations simhp_dev_fops = {
	.owner = THIS_MODULE,
	.open = simhotplug_open,
	.unlocked_ioctl = simhotplug_ioctl,
	.compat_ioctl = compat_simhotplug_ioctl,
       .release = simhotplug_release,
};


static int create_simhp_node(int sim_id)
{
    int ret = 0;
    struct device *dev = NULL;
    char simhp_dev_name[16];
    memset_s(simhp_dev_name,sizeof(simhp_dev_name),0,sizeof(simhp_dev_name));
    snprintf_s(simhp_dev_name,sizeof(simhp_dev_name), (sizeof(simhp_dev_name)-1),SIMHP_NAME_BASE"%d",sim_id);

    LOGE("start\n");

    /* register sci major and minor number */
    ret = alloc_chrdev_region(&simhotplug_data.simhp_dev,
        SIMHP_FIRST_MINOR, SIMHP_DEVICES_NUMBER, simhp_dev_name);
    if (ret)
    {
        LOGE("alloc_chrdev_region failed\n");
        goto fail_region;
    }

    cdev_init(&simhotplug_data.simhp_cdev, &simhp_dev_fops);
    simhotplug_data.simhp_cdev.owner = THIS_MODULE;

    ret = cdev_add(&simhotplug_data.simhp_cdev, simhotplug_data.simhp_dev, SIMHP_DEVICES_NUMBER);
    if (ret)
    {
        LOGE("cdev_add failed\n");
        goto fail_cdev_add;
    }

    simhotplug_data.simhp_class = class_create(THIS_MODULE, simhp_dev_name);
    if (IS_ERR(simhotplug_data.simhp_class))
    {
        LOGE("class_create failed\n");
        goto fail_class_create;
    }

    dev = device_create(simhotplug_data.simhp_class, NULL, simhotplug_data.simhp_dev, NULL, simhp_dev_name);
    if (IS_ERR(dev))
    {
        LOGE("device_create failed\n");
        goto fail_device_create;
    }

    spin_lock_init(&simhotplug_data.read_lock);

    return 0;

fail_device_create:
    class_destroy(simhotplug_data.simhp_class);
fail_class_create:
    cdev_del(&simhotplug_data.simhp_cdev);
fail_cdev_add:
    unregister_chrdev_region(simhotplug_data.simhp_dev, SIMHP_DEVICES_NUMBER);
fail_region:

    return ret;
}

void remove_simhp_node(int sim_id)
{
    spin_lock_bh(&simhotplug_data.read_lock);
    simhotplug_data.simhp_dev_en = false;
    spin_unlock_bh(&simhotplug_data.read_lock);

    if (NULL != simhotplug_data.simhp_class)
    {
        device_destroy(simhotplug_data.simhp_class, simhotplug_data.simhp_dev);
        class_destroy(simhotplug_data.simhp_class);
    }
    cdev_del(&simhotplug_data.simhp_cdev);
    unregister_chrdev_region(simhotplug_data.simhp_dev, SIMHP_DEVICES_NUMBER);
    LOGE("remove simhp node\n");
}

int sim_hotplug_sdmux_init(struct hisi_sim_hotplug_info *info)
{
    int     ret = 0;


    return ret;
}


static int hisi_sim_hotplug_probe(struct spmi_device *pdev)
{
    struct device *dev                  = NULL;
    struct device_node *np              = NULL;
    struct hisi_sim_hotplug_info *info  = NULL;
    int     ret = 0;

    LOGI("probe simhotplug\n");

    if (NULL == pdev)
    {
        LOGE("pdev is NULL.\n");
        return -EINVAL;
    }

    dev = &pdev->dev;
    np = dev->of_node;

    info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
    if (!info)
    {
        LOGE("failed to allocate memory.\n");
        return -ENOMEM;
    }

    ret = sim_hotplug_dts_read(info, np);
    if (ret < 0)
    {
        LOGE("sim_hotplug_dts_read failed, ret: %d\n", ret);
        goto free_info;
    }

    ret = sim_hotplug_init(info);
    if (ret < 0)
    {
        LOGE("sim_hotplug_init failed, ret: %d\n", ret);
        goto free_sim_det_wq;
    }

    ret = sim_pmu_hpd_init(info, np);
    if (ret < 0)
    {
        LOGE("sim_pmu_hpd_init failed, ret: %d\n", ret);
        goto free_sim_det_wq;
    }


    ret = sim_state_init(info, dev);
    if (ret < 0)
    {
        LOGE("sim_state_init failed, ret: %d\n", ret);
        goto free_sim_lock;
    }

    ret = sim_hpd_init(info, pdev);
    if (ret < 0)
    {
        LOGE("sim_hpd_init failed, ret: %d\n", ret);
        goto free_sim_lock;
    }

    if (SIM0 == info->sim_id)
    {
        sys_add_sim_node();
    }
    else if (SIM1 == info->sim_id)
    {
        ret = bsp_icc_event_register(SIM1_CHANNEL_ID, handle_msg_from_sci, info, NULL, NULL);
        if (ret != 0)
        {
            LOGE("bsp_icc_event_register failed, ret: %d\n", ret);
            goto free_sim_lock;
        }
    }

    dev_set_drvdata(&pdev->dev, info);

    return ret;

free_sim_lock:
    wake_lock_destroy(&info->sim_hotplug_wklock);
    mutex_destroy(&info->sim_hotplug_lock);

free_sim_det_wq:
    if (info->sim_hotplug_det_wq)
    {
        destroy_workqueue(info->sim_hotplug_det_wq);
    }
    if (info->sim_hotplug_hpd_wq)
    {
        destroy_workqueue(info->sim_hotplug_hpd_wq);
    }
    if (info->sim_sci_msg_wq)
    {
        destroy_workqueue(info->sim_sci_msg_wq);
    }

free_info:
    devm_kfree(&pdev->dev, info);

    return ret;
}

static int hisi_sim_hotplug_remove(struct spmi_device *pdev)
{
    struct hisi_sim_hotplug_info *info;

    info = dev_get_drvdata(&pdev->dev);
    if (NULL == info)
    {
        LOGE("dev_get_drvdata is NULL.\n");
        return -1;
    }

    mutex_destroy(&info->sim_hotplug_lock);
    wake_lock_destroy(&info->sim_hotplug_wklock);

    if (info->sim_hotplug_det_wq)
    {
        destroy_workqueue(info->sim_hotplug_det_wq);
    }
    if (info->sim_hotplug_hpd_wq)
    {
        destroy_workqueue(info->sim_hotplug_hpd_wq);
    }
    if (info->sim_sci_msg_wq)
    {
        destroy_workqueue(info->sim_sci_msg_wq);
    }

    if (1 == info->send_msg_to_cp && info->sim_debounce_delay_wq)
    {
        LOGI("remove delay work for sim%d\n", info->sim_id);
        cancel_delayed_work(&info->sim_debounce_delay_work);
        flush_workqueue(info->sim_debounce_delay_wq);
        destroy_workqueue(info->sim_debounce_delay_wq);
    }

    if(SIM1 == info->sim_id)
    {
        remove_simhp_node(info->sim_id);
    }

    devm_kfree(&pdev->dev, info);
    dev_set_drvdata(&pdev->dev, NULL);

    return 0;
}

static int hisi_sim_hotplug_suspend(struct spmi_device *pdev, pm_message_t state)
{
    struct hisi_sim_hotplug_info *info;

    info = dev_get_drvdata(&pdev->dev);
    if (NULL == info)
    {
        LOGE("dev_get_drvdata is NULL.\n");
        return -1;
    }

    if (!mutex_trylock(&info->sim_hotplug_lock))
    {
        LOGE("mutex_trylock failed.\n");
        return -EAGAIN;
    }

    return 0;
}

static int hisi_sim_hotplug_resume(struct spmi_device *pdev)
{
    struct hisi_sim_hotplug_info *info;

    info = dev_get_drvdata(&pdev->dev);
    if (NULL == info)
    {
        LOGE("dev_get_drvdata is NULL.\n");
        return -1;
    }

    /*lint -e455 -esym(455,*)*/
    mutex_unlock(&(info->sim_hotplug_lock));
    /*lint -e455 +esym(455,*)*/

    return 0;
}

static struct of_device_id hisi_sim_hotplug_of_match[] =
{
    { .compatible = HISILICON_SIM_HOTPLUG0, },
    { .compatible = HISILICON_SIM_HOTPLUG1, },
    { },
};

MODULE_DEVICE_TABLE(of, hisi_sim_hotplug_of_match);

static struct spmi_driver hisi_sim_hotplug_driver =
{
    .probe      = hisi_sim_hotplug_probe,
    .remove     = hisi_sim_hotplug_remove,
    .driver     =
    {
        .owner      = THIS_MODULE,
        .name       = "hisi-sim_hotplug",
        .of_match_table = of_match_ptr(hisi_sim_hotplug_of_match),
    },
    .suspend        = hisi_sim_hotplug_suspend,
    .resume         = hisi_sim_hotplug_resume,
};

int __init hisi_sim_hotplug_init(void)
{
    return spmi_driver_register(&hisi_sim_hotplug_driver);
}

static void __exit hisi_sim_hotplug_exit(void)
{
    spmi_driver_unregister(&hisi_sim_hotplug_driver);
}

late_initcall(hisi_sim_hotplug_init);
module_exit(hisi_sim_hotplug_exit);
MODULE_DESCRIPTION("Sim hotplug driver");
MODULE_LICENSE("GPL v2");

/*lint -restore */
/*lint -e528 +esym(528,*)*/
/*lint -e753 +esym(753,*)*/
/*lint -e752 +esym(502,*)*/
