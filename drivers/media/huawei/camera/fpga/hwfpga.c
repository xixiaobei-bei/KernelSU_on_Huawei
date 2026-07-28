/*
 *  Hisilicon K3 SOC camera driver source file
 *
 *  Copyright (C) Huawei Technology Co., Ltd.
 *
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
#include "hwfpga.h"
extern int snprintf_s(char* strDest, size_t destMax, size_t count, const char* format, ...);
extern int memcpy_s(void *dest, size_t destMax, const void *src, size_t count);

static struct mutex fpga_fd_lock;
static int fpga_fd_open_times = 0;

typedef struct _tag_hwfpga hwfpga_t;

typedef struct _tag_hwfpga
{
    struct v4l2_subdev                          subdev;
    struct platform_device*                     pdev;
    struct mutex                                lock;//stub
    const hwfpga_intf_t*                        hw;
    hwcam_data_table_t*                         cfg;//stub:maybe no use
    struct ion_handle*                          cfg_hdl;//stub:maybe no use
    hwfpga_notify_intf_t                        notify;//stub:maybe no use
} hwfpga_t;

#define SD2HWFPGA(sd) container_of(sd, hwfpga_t, subdev)

static long
hwfpga_subdev_config(
     hwfpga_t* s,
     hwfpga_config_data_t* data);

static int hwfpga_v4l2_open(
     struct v4l2_subdev* sd,
     struct v4l2_subdev_fh* fh)
{
    hwfpga_config_data_t edata;
    hwfpga_t* s = SD2HWFPGA(sd);
    HWCAM_CFG_INFO("instance(0x%pK)", s);

    mutex_lock(&fpga_fd_lock);
    if (fpga_fd_open_times == 0) {
        edata.cfgtype = HWCAM_FPGA_INITIAL;
        hwfpga_subdev_config(s, &edata);
    }
    fpga_fd_open_times++;
    mutex_unlock(&fpga_fd_lock);
    HWCAM_CFG_INFO("fpga_fd_open_times = %d.", fpga_fd_open_times);
    return 0;
}

static int
hwfpga_v4l2_close(
    struct v4l2_subdev* sd,
    struct v4l2_subdev_fh* fh)
{
    struct ion_handle* hdl = NULL;
    hwfpga_t* s = SD2HWFPGA(sd);
    hwfpga_config_data_t edata;
    hwcam_data_table_t* cfg = NULL;
    HWCAM_CFG_INFO("instance(0x%pK)", s);

    mutex_lock(&fpga_fd_lock);
    fpga_fd_open_times--;
    if (fpga_fd_open_times == 0) {
        edata.cfgtype = HWCAM_FPGA_CLOSE;
    }
    else {
        edata.cfgtype = HWCAM_FPGA_POWEROFF;
    }
    hwfpga_subdev_config(s, &edata);
    mutex_unlock(&fpga_fd_lock);

    swap(s->cfg_hdl, hdl);
    swap(s->cfg, cfg);
    if (hdl) {
        HWCAM_CFG_ERR("release fpag driver data table!");
        hwcam_cfgdev_release_data_table(hdl);
    }

    HWCAM_CFG_INFO("fpga_fd_open_times = %d.", fpga_fd_open_times);
    return 0;
}

static struct v4l2_subdev_internal_ops
s_hwfpga_subdev_internal_ops =
{
    .open  = hwfpga_v4l2_open,
    .close = hwfpga_v4l2_close,
};

static long
hwfpga_subdev_get_info(
     hwfpga_t* fpga,
     hwfpga_info_t* info)
{
    memcpy_s(info->name, HWFPGA_NAME_SIZE,hwfpga_intf_get_name(fpga->hw),
         HWFPGA_NAME_SIZE);
    HWCAM_CFG_INFO("fpga name(%s)\n", info->name);
    return 0;
}

static long
hwfpga_subdev_mount_buf(
    hwfpga_t* s,
    hwcam_buf_info_t* bi)
{
     long rc = -EINVAL;
     switch (bi->kind)
     {
         case HWCAM_BUF_KIND_PIPELINE_PARAM:
             if (!s->cfg) {
                 s->cfg = hwcam_cfgdev_import_data_table(
                         "hwfpga_drv_cfg", bi, &s->cfg_hdl);
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
hwfpga_subdev_unmount_buf(
    hwfpga_t* s,
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
hwfpga_subdev_config(
    hwfpga_t* s,
    hwfpga_config_data_t* data)
{
    long rc = -EINVAL;
    HWCAM_CFG_INFO("%s cfgtype(%d)!", __func__, data->cfgtype);
    switch (data->cfgtype)
    {
        case HWCAM_FPGA_POWERON:
            rc = s->hw->vtbl->power_on(s->hw);
            break;
        case HWCAM_FPGA_POWEROFF:
            rc = s->hw->vtbl->power_off(s->hw);
            break;
        case HWCAM_FPGA_LOADFW:
            rc = s->hw->vtbl->load_firmware(s->hw);
            break;
        case HWCAM_FPGA_ENABLE:
            rc = s->hw->vtbl->enable(s->hw);
            break;
        case HWCAM_FPGA_DISABLE:
            rc = s->hw->vtbl->disable(s->hw);
            break;
        case HWCAM_FPGA_INITIAL:
            rc = s->hw->vtbl->init(s->hw);
            break;
        case HWCAM_FPGA_CLOSE:
            rc = s->hw->vtbl->close(s->hw);
            break;
        default:
            HWCAM_CFG_ERR("invalid cfgtype(%d)! \n", data->cfgtype);
            break;
    }

    return rc;
}

static long
hwfpga_subdev_ioctl(
    struct v4l2_subdev* sd,
    unsigned int cmd,
    void *arg)
{
    long rc = -EINVAL;
    hwfpga_t *s = SD2HWFPGA(sd);

    switch (cmd)
    {
        case HWFPGA_IOCTL_GET_INFO:
            rc = hwfpga_subdev_get_info(s, arg);
            break;
        case HWCAM_V4L2_IOCTL_MOUNT_BUF:
            rc = hwfpga_subdev_mount_buf(s, arg);
            break;
        case HWCAM_V4L2_IOCTL_UNMOUNT_BUF:
            rc = hwfpga_subdev_unmount_buf(s, arg);
            break;
        case HWFPGA_IOCTL_CONFIG:
            HWCAM_CFG_INFO("%s CMD(%u)!", __func__, cmd);
            rc = hwfpga_subdev_config(s, arg);
            break;
        default:
            HWCAM_CFG_ERR("invalid IOCTL CMD(%u)! \n", cmd);
        break;
    }

    return rc;
}

static int
hwfpga_subdev_subscribe_event(
    struct v4l2_subdev* sd,
    struct v4l2_fh* fh,
    struct v4l2_event_subscription* sub)
{
    return v4l2_event_subscribe(fh, sub, 128, NULL);
}

static int
hwfpga_subdev_unsubscribe_event(
    struct v4l2_subdev* sd,
    struct v4l2_fh* fh,
    struct v4l2_event_subscription* sub)
{
    return v4l2_event_unsubscribe(fh, sub);
}

static int
hwfpga_power(
    struct v4l2_subdev* sd,
    int on)
{
    return 0;
}

#define NOTIFY2HWFPGA(i) container_of(i, hwfpga_t, notify)

static void hwfpga_notify_error(hwfpga_notify_intf_t* i, hwfpga_event_t* fpga_ev)
{
    hwfpga_t *fpga = NULL;
    struct v4l2_event ev;
    struct video_device *vdev = NULL;
    hwfpga_event_t* req = (hwfpga_event_t*)ev.u.data;

    fpga = NOTIFY2HWFPGA(i);
    vdev = fpga->subdev.devnode;

    ev.type = HWFPGA_V4L2_EVENT_TYPE;
    ev.id   = HWFPGA_HIGH_PRIO_EVENT;

    req->kind = fpga_ev->kind;
    req->data.error.id = fpga_ev->data.error.id;

    v4l2_event_queue(vdev, &ev);
}

static hwfpga_notify_vtbl_t s_notify_hwfpga =
{
    .error = hwfpga_notify_error,
};

static struct v4l2_subdev_core_ops
s_hwfpga_subdev_core_ops =
{
    .ioctl = hwfpga_subdev_ioctl,
    .subscribe_event = hwfpga_subdev_subscribe_event,
    .unsubscribe_event = hwfpga_subdev_unsubscribe_event,
    .s_power = hwfpga_power,
};

static struct v4l2_subdev_ops
s_hwfpga_subdev_ops =
{
    .core = &s_hwfpga_subdev_core_ops,
};

int32_t
hwfpga_register(
    struct platform_device* pdev, const hwfpga_intf_t* i,
    hwfpga_notify_intf_t** notify)
{
    int rc = 0;
    struct v4l2_subdev* subdev = NULL;
    hwfpga_t* fpga = kzalloc(sizeof(hwfpga_t), GFP_KERNEL);
    if (fpga == NULL) {
        rc = -ENOMEM;
        goto alloc_fail;
    }

    subdev = &fpga->subdev;
    v4l2_subdev_init(subdev, &s_hwfpga_subdev_ops);
    subdev->internal_ops = &s_hwfpga_subdev_internal_ops;
    snprintf_s(subdev->name, sizeof(subdev->name),sizeof(subdev->name)-1,"%s", hwfpga_intf_get_name(i));
    subdev->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;//can be access in user space
    subdev->flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
    v4l2_set_subdevdata(subdev, pdev);
    platform_set_drvdata(pdev, subdev);

    mutex_init(&fpga->lock);
    mutex_init(&fpga_fd_lock);

    init_subdev_media_entity(subdev,HWCAM_SUBDEV_FPGA);
    hwcam_cfgdev_register_subdev(subdev,HWCAM_SUBDEV_FPGA);

    subdev->devnode->lock = &fpga->lock;
    fpga->hw = pdev->dev.driver->of_match_table->data;
    fpga->pdev = pdev;
    fpga->notify.vtbl = &s_notify_hwfpga;
    *notify = &(fpga->notify);

    if (i->vtbl->fpga_get_dt_data) {
        rc = i->vtbl->fpga_get_dt_data(i, pdev->dev.of_node);
    }

    if (i->vtbl->fpga_register_attribute) {
        rc = i->vtbl->fpga_register_attribute(i, &subdev->devnode->dev);
    }

alloc_fail:

    return rc;
}

/*added for memory hwfpga_t leak  **/
void
hwfpga_unregister(hwfpga_intf_t* intf)
{
    hwfpga_t* fpga = NULL;
    struct v4l2_subdev* subdev = NULL;

    fpga = container_of(intf, hwfpga_t, hw);
    subdev  = &fpga->subdev;
    media_entity_cleanup(&subdev->entity);
    hwcam_cfgdev_unregister_subdev(subdev);

    kzfree(fpga);
}
/*added for memory hwfpga_t leak **/



