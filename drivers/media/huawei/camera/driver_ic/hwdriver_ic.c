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
#include "hwdriver_ic.h"

typedef struct _tag_hwdriveric hwdriveric_t;

typedef struct _tag_hwdriveric
{
    struct v4l2_subdev                          subdev;
    struct platform_device*                     pdev;
    struct mutex                                lock;
    const hwdriveric_intf_t*                    hw;
    hwcam_data_table_t*                         cfg;
    struct ion_handle*                          cfg_hdl;
    hwdriveric_notify_intf_t                    notify;
} hwdriveric_t;

#define SD2HWDRIVERIC(sd) container_of(sd, hwdriveric_t, subdev)
#define INTF2DRIVERIC(i) container_of(i, driveric_t, intf)
extern int memset_s(void *dest, size_t destMax, int c, size_t count);
extern int memcpy_s(void *dest, size_t destMax, const void *src, size_t count);
extern int snprintf_s(char* strDest, size_t destMax, size_t count, const char* format, ...);
extern int strncpy_s(char *strDest, size_t destMax, const char *strSrc, size_t count);

static long
hwdriveric_subdev_config(
     hwdriveric_t* s,
     hwdriveric_config_data_t* data);

static int hwdriveric_v4l2_open(
     struct v4l2_subdev* sd,
     struct v4l2_subdev_fh* fh)
{
    hwdriveric_t* s = SD2HWDRIVERIC(sd);
    HWCAM_CFG_INFO("instance(0x%pK)", s);
    return 0;
}

static int
hwdriveric_v4l2_close(
    struct v4l2_subdev* sd,
    struct v4l2_subdev_fh* fh)
{
    struct ion_handle* hdl = NULL;
    hwdriveric_t* s = SD2HWDRIVERIC(sd);
    hwdriveric_config_data_t edata;
    hwcam_data_table_t* cfg = NULL;

    HWCAM_CFG_INFO("instance(0x%pK)", s);
    edata.cfgtype = HWCAM_DRIVERIC_POWEROFF;
    hwdriveric_subdev_config(s, &edata);

    swap(s->cfg_hdl, hdl);
    swap(s->cfg, cfg);
    if (hdl) {
        HWCAM_CFG_ERR("release driver ic data table!");
        hwcam_cfgdev_release_data_table(hdl);
    }

    return 0;
}

static struct v4l2_subdev_internal_ops
s_hwdriveric_subdev_internal_ops =
{
    .open  = hwdriveric_v4l2_open,
    .close = hwdriveric_v4l2_close,
};

static long
hwdriveric_subdev_get_info(
     hwdriveric_t* driveric,
     hwdriveric_info_t* info)
{
    driveric_t *ic = NULL;

    memset_s(info->name, HWDRIVERIC_NAME_SIZE, 0, HWDRIVERIC_NAME_SIZE);
    strncpy_s(info->name, HWDRIVERIC_NAME_SIZE - 1, hwdriveric_intf_get_name(driveric->hw),
         strlen(hwdriveric_intf_get_name(driveric->hw))+1);

    ic = INTF2DRIVERIC(driveric->hw);

    info->i2c_idx  = ic->i2c_index;
    info->position = ic->position;

    HWCAM_CFG_INFO("ic name(%s), i2c_index(%d), postion(%d)! \n", 
        info->name, info->i2c_idx, info->position);

    return 0;
}

static long
hwdriveric_subdev_mount_buf(
    hwdriveric_t* s,
    hwcam_buf_info_t* bi)
{
     long rc = -EINVAL;
     switch (bi->kind)
     {
         case HWCAM_BUF_KIND_PIPELINE_PARAM:
             if (!s->cfg) {
                 s->cfg = hwcam_cfgdev_import_data_table(
                         "hwdriveric_drv_cfg", bi, &s->cfg_hdl);
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
hwdriveric_subdev_unmount_buf(
    hwdriveric_t* s,
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
hwdriveric_subdev_config(
    hwdriveric_t* s,
    hwdriveric_config_data_t* data)
{
    long rc = -EINVAL;
    //static bool hwdriveric_power_on = false;//stub:need modify
    switch (data->cfgtype)
    {
        case HWCAM_DRIVERIC_POWERON:
             rc = s->hw->vtbl->power_on(s->hw);
             break;
        case HWCAM_DRIVERIC_POWEROFF:
             rc = s->hw->vtbl->power_off(s->hw);
             break;
        default:
             HWCAM_CFG_ERR("invalid cfgtype(%d)! \n", data->cfgtype);
             break;
    }

    return rc;
}

static long
hwdriveric_subdev_ioctl(
    struct v4l2_subdev* sd,
    unsigned int cmd,
    void *arg)
{
    long rc = -EINVAL;
    hwdriveric_t *s = SD2HWDRIVERIC(sd);

    switch (cmd)
    {
        case HWDRIVERIC_IOCTL_GET_INFO:
            rc = hwdriveric_subdev_get_info(s, arg);
            break;
        case HWCAM_V4L2_IOCTL_MOUNT_BUF:
            rc = hwdriveric_subdev_mount_buf(s, arg);
            break;
        case HWCAM_V4L2_IOCTL_UNMOUNT_BUF:
            rc = hwdriveric_subdev_unmount_buf(s, arg);
            break;
        case HWDRIVERIC_IOCTL_CONFIG:
            rc = hwdriveric_subdev_config(s, arg);
            break;
        default:
            HWCAM_CFG_ERR("invalid IOCTL CMD(%u)! \n", cmd);
        break;
    }

    return rc;
}

static int
hwdriveric_subdev_subscribe_event(
    struct v4l2_subdev* sd,
    struct v4l2_fh* fh,
    struct v4l2_event_subscription* sub)
{
    return v4l2_event_subscribe(fh, sub, 128, NULL);
}

static int
hwdriveric_subdev_unsubscribe_event(
    struct v4l2_subdev* sd,
    struct v4l2_fh* fh,
    struct v4l2_event_subscription* sub)
{
    return v4l2_event_unsubscribe(fh, sub);
}

static int
hwdriveric_power(
    struct v4l2_subdev* sd,
    int on)
{
    return 0;
}

#define NOTIFY2HWDRIVERIC(i) container_of(i, hwdriveric_t, notify)

static void hwdriveric_notify_error(hwdriveric_notify_intf_t* i,
    hwdriveric_event_t* driveric_ev)
{
    hwdriveric_t *driveric = NULL;
    struct v4l2_event ev;
    struct video_device *vdev = NULL;
    hwdriveric_event_t* req = (hwdriveric_event_t*)ev.u.data;

    driveric = NOTIFY2HWDRIVERIC(i);
    vdev = driveric->subdev.devnode;

    ev.type = HWDRIVERIC_V4L2_EVENT_TYPE;
    ev.id   = HWDRIVERIC_HIGH_PRIO_EVENT;

    req->kind = driveric_ev->kind;
    req->data.error.id = driveric_ev->data.error.id;

    v4l2_event_queue(vdev, &ev);
}

static hwdriveric_notify_vtbl_t s_notify_hwdriveric =
{
    .error = hwdriveric_notify_error,
};

static struct v4l2_subdev_core_ops
s_hwdriveric_subdev_core_ops =
{
    .ioctl = hwdriveric_subdev_ioctl,
    .subscribe_event = hwdriveric_subdev_subscribe_event,
    .unsubscribe_event = hwdriveric_subdev_unsubscribe_event,
    .s_power = hwdriveric_power,
};

static struct v4l2_subdev_ops
s_hwdriveric_subdev_ops =
{
    .core = &s_hwdriveric_subdev_core_ops,
};

int32_t
hwdriveric_register(
    struct platform_device* pdev, const hwdriveric_intf_t* i,
    hwdriveric_notify_intf_t** notify)
{
    int rc = 0;
    struct v4l2_subdev* subdev = NULL;
    hwdriveric_t* driveric = kzalloc(sizeof(hwdriveric_t), GFP_KERNEL);
    if (driveric == NULL) {
        rc = -ENOMEM;
        goto alloc_fail;
    }

    subdev = &driveric->subdev;
    v4l2_subdev_init(subdev, &s_hwdriveric_subdev_ops);
    subdev->internal_ops = &s_hwdriveric_subdev_internal_ops;
    snprintf_s(subdev->name, sizeof(subdev->name), sizeof(subdev->name)-1, "%s", hwdriveric_intf_get_name(i));
    subdev->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;//can be access in user space
    subdev->flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
    v4l2_set_subdevdata(subdev, pdev);
    platform_set_drvdata(pdev, subdev);

    mutex_init(&driveric->lock);

    init_subdev_media_entity(subdev,HWCAM_SUBDEV_DRIVER_IC);
    hwcam_cfgdev_register_subdev(subdev,HWCAM_SUBDEV_DRIVER_IC);

    subdev->devnode->lock = &driveric->lock;
    driveric->hw = i;
    driveric->pdev = pdev;
    driveric->notify.vtbl = &s_notify_hwdriveric;
    *notify = &(driveric->notify);

    if (i->vtbl->driveric_get_dt_data) {
        rc = i->vtbl->driveric_get_dt_data(i, pdev->dev.of_node);
    }

    if (i->vtbl->init) {
        rc = i->vtbl->init(i);
    }

    if (i->vtbl->driveric_register_attribute) {
        rc = i->vtbl->driveric_register_attribute(i, &subdev->devnode->dev);
    }

alloc_fail:

    return rc;
}

/*added for memory hwdriveric_t leak  **/
void
hwdriveric_unregister(hwdriveric_intf_t* intf)
{
    hwdriveric_t* driveric = NULL;
    struct v4l2_subdev* subdev = NULL;

    driveric = container_of(intf, hwdriveric_t, hw);
    subdev  = &driveric->subdev;
    media_entity_cleanup(&subdev->entity);
    hwcam_cfgdev_unregister_subdev(subdev);

    kzfree(driveric);
}
/*added for memory hwdriveric_t leak **/



