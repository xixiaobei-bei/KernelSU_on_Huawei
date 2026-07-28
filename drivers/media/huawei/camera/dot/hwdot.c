/*
 *  Hisilicon K3 SOC camera driver source file
 *
 *  Copyright (C) Huawei Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
//lint -save -e529 -e542 -e429 -e501 -e527 -e559 -e568 -e570 -e563 -e593 -e613 -e668 -e679
#include <linux/compiler.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <media/huawei/camera.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>

#include "hwcam_intf.h"
#include "hwdot.h"

typedef struct _tag_hwdot hwdot_t;

typedef struct _tag_hwdot
{
    struct v4l2_subdev                          subdev;
    struct platform_device*                     pdev;
    struct mutex                                lock;
    const hwdot_intf_t*                         hw;
    hwcam_data_table_t*                         cfg;
    struct ion_handle*                          cfg_hdl;
    hwdot_notify_intf_t                         notify;
} hwdot_t;

#define SD2HWDOT(sd) container_of(sd, hwdot_t, subdev)
#define INTF2DOT(i) container_of(i, dot_t, intf)

extern int snprintf_s(char* strDest, size_t destMax, size_t count, const char* format, ...);
extern int memset_s(void *dest, size_t destMax, int c, size_t count);
extern int memcpy_s(void *dest, size_t destMax, const void *src, size_t count);
extern int strncpy_s(char *strDest, size_t destMax, const char *strSrc, size_t count);
static long
hwdot_subdev_config(
     hwdot_t* s,
     hwdot_config_data_t* data);

static int hwdot_v4l2_open(
     struct v4l2_subdev* sd,
     struct v4l2_subdev_fh* fh)
{
    hwdot_t* s = SD2HWDOT(sd);
    HWCAM_CFG_INFO("instance(0x%pK)", s);
    return 0;
}

static int
hwdot_v4l2_close(
    struct v4l2_subdev* sd,
    struct v4l2_subdev_fh* fh)
{
    struct ion_handle* hdl = NULL;
    hwdot_t* s = SD2HWDOT(sd);
    hwdot_config_data_t edata;
    hwcam_data_table_t* cfg = NULL;

    HWCAM_CFG_INFO("instance(0x%pK)", s);
    edata.cfgtype = HWCAM_DOT_POWEROFF;
    hwdot_subdev_config(s, &edata);

    swap(s->cfg_hdl, hdl);
    swap(s->cfg, cfg);
    if (hdl) {
        HWCAM_CFG_ERR("release driver ic data table!");
        hwcam_cfgdev_release_data_table(hdl);
    }

    return 0;
}

static struct v4l2_subdev_internal_ops
s_hwdot_subdev_internal_ops =
{
    .open  = hwdot_v4l2_open,
    .close = hwdot_v4l2_close,
};

static long
hwdot_subdev_get_info(
     hwdot_t* dot,
     hwdot_info_t* info)
{
    dot_t *ic = NULL;

    memset_s(info->name, HWDOT_NAME_SIZE, 0, HWDOT_NAME_SIZE);
    strncpy_s(info->name, HWDOT_NAME_SIZE - 1, hwdot_intf_get_name(dot->hw),
         strlen(hwdot_intf_get_name(dot->hw)) + 1);

    ic = INTF2DOT(dot->hw);
    info->i2c_idx  = ic->i2c_index;
    HWCAM_CFG_INFO("i2c_index(%d)\n", info->i2c_idx);

    return 0;
}


int hwdot_get_thermal(const hwdot_intf_t *i,void* data)
{
    return i->vtbl->get_thermal(i,data);
}


static long
hwdot_subdev_mount_buf(
    hwdot_t* s,
    hwcam_buf_info_t* bi)
{
     long rc = -EINVAL;
     switch (bi->kind)
     {
         case HWCAM_BUF_KIND_PIPELINE_PARAM:
             if (!s->cfg) {
                 s->cfg = hwcam_cfgdev_import_data_table(
                         "hwdot_drv_cfg", bi, &s->cfg_hdl);
                 if (s->cfg) { rc = 0; }
             }
             break;
         default:
             HWCAM_CFG_ERR("invalid buffer kind(%d)! \n", bi->kind);
             break;
     }

     return rc;
}

static long
hwdot_subdev_unmount_buf(
    hwdot_t* s,
    hwcam_buf_info_t* bi)
{
    long rc = -EINVAL;
    switch (bi->kind)
    {
        case HWCAM_BUF_KIND_PIPELINE_PARAM:
            hwcam_cfgdev_release_data_table(s->cfg_hdl);
            s->cfg_hdl = NULL;
            s->cfg = NULL;
            rc = 0;
            break;
        default:
            HWCAM_CFG_ERR("invalid buffer kind(%d)! \n", bi->kind);
        break;
    }

    return rc;
}

static long
hwdot_subdev_config(
    hwdot_t* s,
    hwdot_config_data_t* data)
{
    long rc = -EINVAL;
    static bool hwdot_power_on = false;//stub:need modify
    switch (data->cfgtype)
    {
        case HWCAM_DOT_POWERON:
            if (!hwdot_power_on) {
                rc = s->hw->vtbl->power_on(s->hw);
                hwdot_power_on = true;
            }
            break;
        case HWCAM_DOT_POWEROFF:
            if (hwdot_power_on) {
                rc = s->hw->vtbl->power_off(s->hw);
                hwdot_power_on = false;
            }
            break;
        default:
            HWCAM_CFG_ERR("invalid cfgtype(%d)! \n", data->cfgtype);
            break;
    }

    return rc;
}

static long
hwdot_subdev_ioctl(
    struct v4l2_subdev* sd,
    unsigned int cmd,
    void *arg)
{
    long rc = -EINVAL;
    hwdot_t *s = SD2HWDOT(sd);

    switch (cmd)
    {
        case HWDOT_IOCTL_GET_INFO:
            rc = hwdot_subdev_get_info(s, arg);
            break;
        case HWCAM_V4L2_IOCTL_MOUNT_BUF:
            rc = hwdot_subdev_mount_buf(s, arg);
            break;
        case HWCAM_V4L2_IOCTL_UNMOUNT_BUF:
            rc = hwdot_subdev_unmount_buf(s, arg);
            break;
        case HWDOT_IOCTL_CONFIG:
            rc = hwdot_subdev_config(s, arg);
            break;
        case HWDOT_IOCTL_GET_THERMAL:
            rc = hwdot_get_thermal(s->hw, arg);
            break;
        default:
            HWCAM_CFG_ERR("invalid IOCTL CMD(%u)! \n", cmd);
        break;
    }

    return rc;
}

static int
hwdot_subdev_subscribe_event(
    struct v4l2_subdev* sd,
    struct v4l2_fh* fh,
    struct v4l2_event_subscription* sub)
{
    return v4l2_event_subscribe(fh, sub, 128, NULL);
}

static int
hwdot_subdev_unsubscribe_event(
    struct v4l2_subdev* sd,
    struct v4l2_fh* fh,
    struct v4l2_event_subscription* sub)
{
    return v4l2_event_unsubscribe(fh, sub);
}

static int
hwdot_power(
    struct v4l2_subdev* sd,
    int on)
{
    return 0;
}

#define NOTIFY2HWDOT(i) container_of(i, hwdot_t, notify)

static void hwdot_notify_error(hwdot_notify_intf_t* i, hwdot_event_t* dot_ev)
{
    hwdot_t *dot = NULL;
    struct v4l2_event ev;
    struct video_device *vdev = NULL;
    hwdot_event_t* req = (hwdot_event_t*)ev.u.data;

    dot = NOTIFY2HWDOT(i);
    vdev = dot->subdev.devnode;

    ev.type = HWDOT_V4L2_EVENT_TYPE;
    ev.id   = HWDOT_HIGH_PRIO_EVENT;

    req->kind = dot_ev->kind;
    req->data.error.id = dot_ev->data.error.id;

    v4l2_event_queue(vdev, &ev);
}

static hwdot_notify_vtbl_t s_notify_hwdot =
{
    .error = hwdot_notify_error,
};

static struct v4l2_subdev_core_ops
s_hwdot_subdev_core_ops =
{
    .ioctl = hwdot_subdev_ioctl,
    .subscribe_event = hwdot_subdev_subscribe_event,
    .unsubscribe_event = hwdot_subdev_unsubscribe_event,
    .s_power = hwdot_power,
};

static struct v4l2_subdev_ops
s_hwdot_subdev_ops =
{
    .core = &s_hwdot_subdev_core_ops,
};

int32_t
hwdot_register(
    struct platform_device* pdev, const hwdot_intf_t* i,
    hwdot_notify_intf_t** notify)
{
    int rc = 0;
    struct v4l2_subdev* subdev = NULL;
    hwdot_t* dot = kzalloc(sizeof(hwdot_t), GFP_KERNEL);
    if (dot == NULL) {
        rc = -ENOMEM;
        goto alloc_fail;
    }

    subdev = &dot->subdev;
    v4l2_subdev_init(subdev, &s_hwdot_subdev_ops);
    subdev->internal_ops = &s_hwdot_subdev_internal_ops;
    snprintf_s(subdev->name, sizeof(subdev->name), sizeof(subdev->name)-1, "%s", hwdot_intf_get_name(i));
    subdev->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;//can be access in user space
    subdev->flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
    v4l2_set_subdevdata(subdev, pdev);
    platform_set_drvdata(pdev, subdev);

    mutex_init(&dot->lock);

    init_subdev_media_entity(subdev,HWCAM_SUBDEV_DOT_PROJECTOR);
    hwcam_cfgdev_register_subdev(subdev,HWCAM_SUBDEV_DOT_PROJECTOR);

    subdev->devnode->lock = &dot->lock;
    dot->hw = pdev->dev.driver->of_match_table->data;
    dot->pdev = pdev;
    dot->notify.vtbl = &s_notify_hwdot;
    *notify = &(dot->notify);

    if (i->vtbl->dot_get_dt_data) {
        rc = i->vtbl->dot_get_dt_data(i, pdev->dev.of_node);
    }

    if (i->vtbl->init) {
        rc = i->vtbl->init(i);
    }

alloc_fail:

    return rc;
}

/*added for memory hwdot_t leak  **/
void
hwdot_unregister(hwdot_intf_t* intf)
{
    hwdot_t* dot = NULL;
    struct v4l2_subdev* subdev = NULL;

    dot = container_of(intf, hwdot_t, hw);
    subdev  = &dot->subdev;
    media_entity_cleanup(&subdev->entity);
    hwcam_cfgdev_unregister_subdev(subdev);

    kzfree(dot);
}
/*added for memory hwdot_t leak **/



