/*
 * Copyright (C) 2017, HW
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/media.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_graph.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/videodev2.h>

#include <media/media-entity.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-image-sizes.h>
#include <media/v4l2-mediabus.h>
#include <media/v4l2-of.h>
#include <media/v4l2-subdev.h>
#include <media/huawei/camera.h>
#include <hwcam_intf.h>
#include <media/huawei/laser_cfg.h>


//lint -save -e826 -e715 -e785 -e64 -e528 -e551 -e753 -e551 -e753 -e429 -esym(528,*) -e750 -esym(753,*) -esym(750,*)


/* -----------------------------------------------------------------------------
 * V4L2 subdev internal operations
 */

#define cam_err(fmt, ...) \
    do { \
            pr_err("[klaser]" "ERROR: " fmt "\n", ##__VA_ARGS__); \
    }while (0)

#define cam_info(fmt, ...) \
    do { \
            pr_info("[klaser]" "INFO: " fmt "\n", ##__VA_ARGS__); \
    }while (0)
#define cam_dbg(fmt, ...) \
    do { \
            pr_dbg("[klaser]" "DBG: " fmt "\n", ##__VA_ARGS__); \
    }while (0)

typedef struct vl53lx_laser {
    struct v4l2_subdev sd;
    struct mutex lock;
    struct i2c_client *client;
    hw_laser_ctrl_t *ctrl;
}laser_t;

#define SD2Laser(sd) container_of(sd, laser_t, sd)

static long
hw_laser_subdev_ioctl(
    struct v4l2_subdev *sd,
    unsigned int cmd,
    void *arg)
{
    long rc = -EINVAL;
    laser_t* s;
    hw_laser_ctrl_t *ctrl;

    if (NULL == sd) {
        cam_err("%s, sd laser is NULL \n", __func__);
        return -EINVAL;
    }

    s = SD2Laser(sd);
    ctrl = (hw_laser_ctrl_t*)s->ctrl;
    if(ctrl->func_tbl && ctrl->func_tbl->laser_ioctl && ctrl->data)
    {
        rc = ctrl->func_tbl->laser_ioctl(ctrl->data,cmd, arg);
    }

    return rc;
}


static int laser_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
    int rc = 0;
    laser_t* s = SD2Laser(sd);
    if (s == NULL || s->ctrl == NULL)
    {
        cam_err("can't register event queue");
        return rc;
    }

    s->ctrl->fh = &fh->vfh;
    cam_info("open laser device success");

    return rc;
}

static int laser_close(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
    long rc = -EINVAL;
    laser_t* s;
    hw_laser_ctrl_t *ctrl;

    if (NULL == sd) {
        cam_err("%s, sd laser is NULL \n", __func__);
        return -EINVAL;
    }

    s = SD2Laser(sd);
    ctrl = (hw_laser_ctrl_t*)s->ctrl;
    if(ctrl->func_tbl && ctrl->func_tbl->laser_ioctl && ctrl->data)
    {
        rc = ctrl->func_tbl->laser_ioctl(ctrl->data,HWLASER_IOCTL_STOP, NULL);
        rc |= ctrl->func_tbl->laser_ioctl(ctrl->data,HWLASER_IOCTL_POWEROFF, NULL);
    }
    cam_err("laser is closed.");
    return rc;
}

static int laser_subdev_subscribe_event(struct v4l2_subdev *sd,
                                        struct v4l2_fh *fh,
                                        struct v4l2_event_subscription *sub)
{
    return v4l2_event_subscribe(fh, sub, 128, NULL);
}

static int laser_subdev_unsubscribe(struct v4l2_subdev *sd,
                                    struct v4l2_fh *fh,
                                    struct v4l2_event_subscription *sub)
{
    return v4l2_event_unsubscribe(fh, sub);
}

static const struct v4l2_subdev_core_ops laser_subdev_core_ops = {
    .subscribe_event = laser_subdev_subscribe_event,
    .unsubscribe_event = laser_subdev_unsubscribe,
    .ioctl = hw_laser_subdev_ioctl,
};


static const struct v4l2_subdev_ops laser_subdev_ops = {
    .core  = &laser_subdev_core_ops,
};

static const struct v4l2_subdev_internal_ops laser_subdev_internal_ops = {
    .open = laser_open,
    .close = laser_close,
};


void laser_notify_data_event(hw_laser_ctrl_t *ctrl, const struct v4l2_event *ev)
{
    if (ctrl == NULL || ctrl->fh == NULL)
    {
        cam_err("get laser_t fail");
        return;
    }

}
EXPORT_SYMBOL(laser_notify_data_event);

//#define DT_TREE_DBG

int laser_probe(struct i2c_client *client,
    const struct i2c_device_id *id)
{
    int ret = 0;
    struct v4l2_subdev *sd;
    laser_t *laser;
    hw_laser_ctrl_t *ctrl;
    cam_info("%s",__func__);
    if(NULL == client || NULL == id)
    {
        cam_err("input param is NULL!!");
        return -EINVAL;
    }
    ctrl = (hw_laser_ctrl_t*)id->driver_data;

    laser = devm_kzalloc(&client->dev, sizeof(laser_t), GFP_KERNEL);
    if (!laser)
        return -ENOMEM;

    laser->client = client;
    laser->ctrl = ctrl;
    sd = &laser->sd;
    v4l2_i2c_subdev_init(sd, client, &laser_subdev_ops);

    sd->internal_ops = &laser_subdev_internal_ops;
    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE |
        V4L2_SUBDEV_FL_HAS_EVENTS;

    mutex_init(&laser->lock);

    init_subdev_media_entity(sd,HWCAM_SUBDEV_LASER);
    ret = hwcam_cfgdev_register_subdev(sd,HWCAM_SUBDEV_LASER);
    cam_info("ret = %d register video devices %s sucessful, major = %d, minor = %d, entity.name = %s",ret, sd->name, sd->entity.info.dev.major, sd->entity.info.dev.minor, sd->entity.name);
    if (ret)
        goto error;

    dev_info(&client->dev, "%s sensor driver registered !!\n", sd->name);

    return ret;

error:
    cam_err("error!!!\n");
    media_entity_cleanup(&sd->entity);
    mutex_destroy(&laser->lock);
    return ret;
}
EXPORT_SYMBOL(laser_probe);

int laser_remove(struct i2c_client *client)
{
    struct v4l2_subdev *sd;
    laser_t *laser;
    hw_laser_ctrl_t *ctrl;
    if(NULL == client)
    {
        cam_err("input client is NULL");
        return -EINVAL;
    }
    sd = i2c_get_clientdata(client);
    laser = SD2Laser(sd);
    ctrl = laser->ctrl;
    v4l2_async_unregister_subdev(sd);
    media_entity_cleanup(&sd->entity);
    mutex_destroy(&laser->lock);
    if(ctrl != NULL)
    {
        kfree(ctrl);
    }
    kfree(laser);
    return 0;
}
EXPORT_SYMBOL(laser_remove);

//lint -restore
