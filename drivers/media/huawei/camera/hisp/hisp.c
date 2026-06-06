/*
 *Hisilicon K3 SOC camera driver source file
 *
 *Copyright (C) Huawei Technology Co., Ltd.
 *
 *Author:
 *Email:
 *Date:	  2014-11-11
 *
 *This program is free software; you can redistribute it and/or modify
 *it under the terms of the GNU General Public License as published by
 *the Free Software Foundation; either version 2 of the License, or
 *(at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//lint -save -e7
//lint -save -e30 -e142 -e613 -e429
#if defined( HISP150_CAMERA  )
#include <media/huawei/hisp150_cfg.h>
#elif defined( HISP160_CAMERA  )
#include <media/huawei/hisp160_cfg.h>
#elif defined( HISP120_CAMERA  )
#include <media/huawei/hisp120_cfg.h>
#elif defined( HISP200_CAMERA  )
#include <media/huawei/hisp200_cfg.h>
#elif defined( HISP210_CAMERA  )
#include <media/huawei/hisp210_cfg.h>
#else
#include <media/huawei/hisp_cfg.h>
#endif

#include <linux/compiler.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>
#include <linux/string.h>

#include "hicam_buf.h"
#include "hisp_intf.h"
#include "cam_log.h"
#include <dsm/dsm_pub.h>

#define  DSM_DEV_BUFF_SIZE 1024
#define CREATE_TRACE_POINTS


static struct dsm_dev dev_hisp = {
	.name = "dsm_hisp",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = DSM_DEV_BUFF_SIZE,
};

static struct dsm_client *client_hisp = NULL;
int hisi_isp_rproc_setpinctl(struct pinctrl *isp_pinctrl, struct pinctrl_state *pinctrl_def, struct pinctrl_state *pinctrl_idle);
int hisi_isp_rproc_setclkdepend(struct clk *aclk_dss, struct clk *pclk_dss);
int hisp_secmem_pa_set(u64 sec_boot_phymem_addr, unsigned int trusted_mem_size);
int hisp_secmem_pa_release(void);
int hisp_mdcmem_pa_set(u64 mdc_phymem_addr, unsigned int mdc_mem_size, int shared_fd);
int hisp_mdcmem_pa_release(void);
/**@brief adapt to all Histar ISP
 *
 *When Histar ISP is probed, this sturcture will be initialized,
 *the object will be found will be found by subdev and notify.
 *
 *@end
 */
typedef struct _tag_hisp {
	struct v4l2_subdev subdev;	/*subscribe event to camera daemon */
	struct mutex lock;
	hisp_intf_t *hw;	/*call the specfic Histar ISP implement */
	/*get notification from specfic Histar ISP call back */
	hisp_notify_intf_t notify;
} hisp_t;

static bool hisp_mdcmem_set;
static bool hisp_secmem_set;
#define SD2HISP(sd) container_of(sd, hisp_t, subdev)
#define Notify2HISP(i) container_of(i, hisp_t, notify)

/*Function declaration */
/**********************************************
 *when get call back from rpmsg, notify
 *camera daemon throuth v4l2 event
 *i: hisp_notify_intf_t to find hisp_t object
 *hisp_ev: hisp event with specific rpmsg type
 *********************************************/
static void hisp_notify_rpmsg_cb(hisp_notify_intf_t *i, hisp_event_t *hisp_ev)
{
	hisp_t *isp = NULL;
	struct v4l2_event ev;
	struct video_device *vdev = NULL;
	hisp_event_t *req = (hisp_event_t *) ev.u.data;
	memset(&ev, 0, sizeof(struct v4l2_event));
	cam_debug("enter %s", __func__);

	if (NULL == hisp_ev || NULL == i){
		cam_err("func %s: hisp_ev or i is NULL", __func__);
		return;
	}
	isp = Notify2HISP(i);
	hisp_assert(NULL != isp);
	vdev = isp->subdev.devnode;

	ev.type = HISP_V4L2_EVENT_TYPE;
	ev.id = HISP_HIGH_PRIO_EVENT;

	req->kind = hisp_ev->kind;

	v4l2_event_queue(vdev, &ev);
}

static hisp_notify_vtbl_t s_notify_hisp = {
	.rpmsg_cb = hisp_notify_rpmsg_cb,
};

static void * hisp_subdev_get_info(hisp_t *isp, hisp_info_t *info)
{
	char * name = NULL;
    size_t min_len = 0;
	if (NULL == isp || NULL == info){
		cam_err("func %s: isp or info is NULL",__func__);
		return NULL;
	}
    name = hisp_intf_get_name(isp->hw);
    if(NULL != name){
        memset(info->name, 0, HISP_NAME_SIZE);
        min_len = strlen(name);
        if (min_len > HISP_NAME_SIZE - 1) {
            min_len = HISP_NAME_SIZE - 1;
        }

        return memcpy(info->name, name, min_len);
    }else{
		cam_err("%s: hisp_intf_get_name() is NULL",__func__);
		return NULL;
	}
}

static void
hisp_subdev_get_system_timestamp(hisp_system_time_t* hisp_system_time )
{
    if(hisp_system_time == NULL) {
        return;
    }
    hisp_system_time->s_system_counter = arch_counter_get_cntvct();
    hisp_system_time->s_system_couter_rate = arch_timer_get_rate();
    do_gettimeofday(&(hisp_system_time->s_timeval));
}

/*Function declaration */
/**********************************************
 *ioctl function for v4l2 subdev
 *********************************************/
static long
hisp_vo_subdev_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	long rc = 0;
	hisp_msg_t isp_msg;
	size_t len = sizeof(hisp_msg_t);
	hisp_t *isp = NULL;
	memset(&isp_msg, 0, sizeof(hisp_msg_t));

	hisp_assert(NULL != sd);
	if (NULL == arg || NULL == sd){
		cam_err("func %s: arg or sd is NULL",__func__);
		return -1;
	}
	isp = SD2HISP(sd);
	if (NULL == isp){
		cam_err("func %s: isp is NULL",__func__);
		return -1;
	}
	cam_debug("cmd is (0x%x), arg(%pK)!\n", cmd, arg);

	switch (cmd) {
	case HISP_IOCTL_CFG_ISP:
		cam_debug("Enter HISP_IOCTL_CFG_ISP!\n");
		rc = isp->hw->vtbl->config(isp->hw, arg);
		break;
	case HISP_IOCTL_GET_INFO:
		cam_debug("Enter HISP_IOCTL_GET_INFO!\n");
		if(!hisp_subdev_get_info(isp, (hisp_info_t *) arg)) {
			rc = -EFAULT;
			cam_err("func %s, line %d: ret = %ld\n", __func__, __LINE__,
				rc);
		}
		break;
	case HISP_IOCTL_POWER_ON:
		break;
    case HISP_IOCTL_GET_SYSTEM_TIME:
        cam_debug("Enter HISP_IOCTL_GET_SYSTEM_TIME!\n");
        hisp_subdev_get_system_timestamp((hisp_system_time_t*) arg);
        break;
	case HISP_IOCTL_POWER_OFF:
		break;
	case HISP_IOCTL_SEND_RPMSG:
		cam_debug("Enter HISP_IOCTL_SEND_RPMSG!\n");
		memcpy(&isp_msg, (hisp_msg_t *)arg, len);
		rc = isp->hw->vtbl->send_rpmsg(isp->hw, &isp_msg, len);
		break;
	case HISP_IOCTL_RECV_RPMSG:
		cam_debug("Enter HISP_IOCTL_RECV_RPMSG!\n");
		rc = isp->hw->vtbl->recv_rpmsg(isp->hw, arg, len);
		break;
	default:
		cam_err("invalid IOCTL CMD(0x%x)!\n", cmd);
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int
hisp_subdev_subscribe_event(struct v4l2_subdev *sd,
			    struct v4l2_fh *fh,
			    struct v4l2_event_subscription *sub)
{
	cam_info("enter %s", __func__);
	return v4l2_event_subscribe(fh, sub, 128, NULL);
}

static int
hisp_subdev_unsubscribe_event(struct v4l2_subdev *sd,
			      struct v4l2_fh *fh,
			      struct v4l2_event_subscription *sub)
{
	cam_info("enter %s", __func__);
	return v4l2_event_unsubscribe(fh, sub);
}

static struct v4l2_subdev_core_ops
s_subdev_core_ops_hisp = {
	.ioctl = hisp_vo_subdev_ioctl,
	.subscribe_event = hisp_subdev_subscribe_event,
	.unsubscribe_event = hisp_subdev_unsubscribe_event,
};

static int hisp_subdev_open(
        struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh)
{
	hisp_t *isp = NULL;
	int rc = 0;

	if (NULL == sd){
		cam_err("func %s: sd is NULL",__func__);
		return -1;
	}
	isp = SD2HISP(sd);
	if (NULL == isp){
		cam_err("func %s: isp is NULL",__func__);
		return -1;
	}

	if (isp->hw->vtbl->open) {
		rc = isp->hw->vtbl->open(isp->hw);
	}

	return rc;
}

static int
hisp_subdev_close(
        struct v4l2_subdev *sd,
        struct v4l2_subdev_fh *fh)
{
	hisp_t *isp = NULL;
	int rc = 0;

	if (NULL == sd){
		cam_err("func %s: sd is NULL",__func__);
		return -1;
	}
	isp = SD2HISP(sd);
	if (NULL == isp){
		cam_err("func %s: isp is NULL",__func__);
		return -1;
	}

	if (isp->hw->vtbl->close) {
		rc = isp->hw->vtbl->close(isp->hw);
	}

	return rc;
}

static struct v4l2_subdev_internal_ops
s_subdev_internal_ops_hisp =
{
	.open = hisp_subdev_open,
	.close = hisp_subdev_close,
};

static struct v4l2_subdev_ops
s_subdev_ops_hisp = {
	.core = &s_subdev_core_ops_hisp,
};

int hisp_get_dt_data(struct platform_device *pdev, hisp_dt_data_t *dt)
{
	int ret = 0;
	struct device *dev = NULL;
	struct device_node *dev_node = NULL;
	unsigned int is_fpga = 0;
	const char *clk_name = NULL;
	if (NULL == pdev || NULL == dt) {
		cam_err("%s: pdev or dt is NULL.", __func__);
		return -1;
	}
	dev = &pdev->dev;
	dev_node = dev->of_node;

	if (NULL == dev_node) {
		cam_err("%s: of node NULL.", __func__);
		return -1;
	}

	ret = of_property_read_u32(dev_node, "hisi,is_fpga", &is_fpga);
	if (ret < 0) {
		cam_err("%s: get FPGA flag failed.", __func__);
	}

	if (is_fpga) {
		cam_info("%s: Ignored FPGA.", __func__);
		return 0;
	}

	ret = of_property_read_string_index(dev_node, "clock-names", 0, &clk_name);
	cam_info("%s: clk(%s)", __func__, clk_name);
	if (ret < 0) {
		cam_err("%s: could not get aclk name.", __func__);
		goto err_aclk;
	}

	dt->aclk = devm_clk_get(dev, clk_name);
	if (IS_ERR_OR_NULL(dt->aclk)) {
		cam_err("%s: could not get aclk.", __func__);
		ret = PTR_ERR(dt->aclk);
		goto err_aclk;
	}

	ret = of_property_read_string_index(dev_node, "clock-names", 1, &clk_name);
	cam_info("%s: clk(%s)", __func__, clk_name);
	if (ret < 0) {
		cam_err("%s: could not get aclk_dss name.", __func__);
		goto err_aclk;
	}

	dt->aclk_dss = devm_clk_get(dev, clk_name);
	if (IS_ERR_OR_NULL(dt->aclk_dss)) {
		cam_err("%s: could not get aclk_dss.", __func__);
		ret = PTR_ERR(dt->aclk_dss);
		goto err_aclk;
	}

	ret = of_property_read_string_index(dev_node, "clock-names", 2, &clk_name);
	cam_info("%s: clk(%s)", __func__, clk_name);
	if (ret < 0) {
		cam_err("%s: could not get pclk_dss name.", __func__);
		goto err_aclk;
	}

	dt->pclk_dss = devm_clk_get(dev, clk_name);
	if (IS_ERR_OR_NULL(dt->pclk_dss)) {
		cam_err("%s: could not get pclk_dss.", __func__);
		ret = PTR_ERR(dt->pclk_dss);
		goto err_aclk;
	} else {
		ret = clk_set_rate(dt->pclk_dss, 120000000UL);
		if (ret < 0) {
			cam_err("%s: could not set pclk_dss rate.", __func__);
		}
	}

	dt->pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR_OR_NULL(dt->pinctrl)) {
		cam_err("%s: could not get pinctrl.", __func__);
		ret = PTR_ERR(dt->pinctrl);
		goto err_no_pinctrl;
	}

	dt->pinctrl_default = pinctrl_lookup_state(dt->pinctrl, PINCTRL_STATE_DEFAULT);
	if (IS_ERR_OR_NULL(dt->pinctrl_default)) {
		cam_err("%s: could not get default pinstate.", __func__);
		return -1;
	}

	dt->pinctrl_idle = pinctrl_lookup_state(dt->pinctrl, PINCTRL_STATE_IDLE);
	if (IS_ERR_OR_NULL(dt->pinctrl_idle)) {
		cam_err("%s: could not get idle pinstate.", __func__);
		return -1;
	} else {
		ret = pinctrl_select_state(dt->pinctrl, dt->pinctrl_idle);
		if (ret) {
			cam_err("%s: could not set idle pinstate.", __func__);
			return -1;
		}
	}
	if ( hisi_isp_rproc_setpinctl(dt->pinctrl, dt->pinctrl_default, dt->pinctrl_idle) < 0 ) {
		cam_err("%s Failed: setpinctl.", __func__);
		return -1;
	}

	// TODO: Remove it for Chicago
	if ( hisi_isp_rproc_setclkdepend(dt->aclk_dss, dt->pclk_dss) < 0 ) {
		cam_err("%s Failed: clkdepend.", __func__);
		return -1;
	}

	return 0;

err_no_pinctrl:
err_aclk:
	cam_err("%s: failed.", __func__);
	return ret;
}

/*Function declaration */
/**********************************************
 *Initialize the hisp_t structure, called from Histar ISP probe
 *pdev: the rpmsg_channel used in Histar ISP driver
 *hw: to call the hook functions in Histar ISP driver
 *notify: get notified when Histar ISP driver needs
 *********************************************/
int32_t
hisp_register(struct platform_device *pdev,
	      hisp_intf_t *hw, hisp_notify_intf_t **notify)
{
	int rc = 0;
	struct v4l2_subdev *subdev = NULL;
	hisp_t *isp = NULL;

	cam_notice("%s enter\n", __func__);

	if (NULL == notify){
		cam_err("func %s: notify is NULL",__func__);
		return -1;
	}

	isp = kzalloc(sizeof(hisp_t), GFP_KERNEL);
	if (isp == NULL) {
		rc = -ENOMEM;
		cam_err("func %s, line %d: out of memory!\n", __func__, __LINE__);
		goto alloc_fail;
	}

	hisp_assert(NULL != hw);

	subdev = &isp->subdev;
	mutex_init(&isp->lock);

	v4l2_subdev_init(subdev, &s_subdev_ops_hisp);
	subdev->internal_ops = &s_subdev_internal_ops_hisp;
	snprintf(subdev->name, sizeof(subdev->name), "hwcam-cfg-hisp");
	subdev->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	subdev->flags |= V4L2_SUBDEV_FL_HAS_EVENTS;

	init_subdev_media_entity(subdev,HWCAM_SUBDEV_HISP);
	hwcam_cfgdev_register_subdev(subdev,HWCAM_SUBDEV_HISP);
	subdev->devnode->lock = &isp->lock;

	platform_set_drvdata(pdev, subdev);
	isp->hw = hw;
	isp->notify.vtbl = &s_notify_hisp;
	*notify = &(isp->notify);

	/* register hisp dsm client */
	client_hisp = dsm_register_client(&dev_hisp);

alloc_fail:
	return rc;
}



void
hisp_unregister(struct platform_device* pdev)
{
    struct v4l2_subdev *subdev = platform_get_drvdata(pdev);
    hisp_t* isp = SD2HISP(subdev);

    media_entity_cleanup(&subdev->entity);
    hwcam_cfgdev_unregister_subdev(subdev);
    mutex_destroy(&isp->lock);

    kzfree(isp);
}

int
hisp_get_sg_table(int fd,struct device *dev,struct dma_buf **buf,struct dma_buf_attachment **attach,struct sg_table **table)
{
	*buf = dma_buf_get(fd);
	if (IS_ERR(*buf)) {
		goto err_dma_buf;
	}
	*attach = dma_buf_attach(*buf, dev);
	if (IS_ERR(*attach)) {
		goto err_dma_buf_attach;
	}

	*table = dma_buf_map_attachment(*attach, DMA_BIDIRECTIONAL);

	if (IS_ERR_OR_NULL(*table)) {
		cam_err("%s Failed : dma_buf_map_attachment.%lu\n", __func__, PTR_ERR(*table));
		goto err_dma_buf_map_attachment;
	}
	cam_info("func %s: dma_buf_map_attachment ok",__func__);
	return 0;
err_dma_buf_map_attachment:
	dma_buf_detach(*buf, *attach);
err_dma_buf_attach:
	dma_buf_put(*buf);
err_dma_buf:
	return -ENODEV;
}

void
hisp_free_dma_buf(struct dma_buf **buf,struct dma_buf_attachment **attach,struct sg_table **table)
{
	dma_buf_unmap_attachment(*attach, *table, DMA_BIDIRECTIONAL);
	dma_buf_detach(*buf, *attach);
	dma_buf_put(*buf);
	*table = NULL;
	*attach = NULL;
	*buf = NULL;
}

int hisp_set_sec_fw_buffer(struct hisp_cfg_data *cfg)
{
	int rc;
	uint32_t size;
	phys_addr_t phys;
	struct sg_table *sgt;

	sgt = hicam_buf_get_sgtable(cfg->share_fd);
	if (IS_ERR_OR_NULL(sgt)) {
		cam_err("%s: error get sg_table of share_fd:%d.",
			__func__, cfg->share_fd);
		return PTR_ERR(sgt);
	}

	phys = sg_phys(sgt->sgl);
	size = sgt->sgl->length;
	if (size < cfg->buf_size) {
		cam_err("%s: real size [%u] less than claimed [%u].",
			__func__, size, cfg->buf_size);
		rc = -ERANGE;
		goto err_buf_size;
	}

	// incase something happens, mdc is not release as expected,
	// like Tomestone. we release it here before next set.
	if (hisp_secmem_set) {
		cam_info("%s: abnormal case, but do it.", __func__);
		(void)hisp_release_sec_fw_buffer();
	}

	rc = hisp_secmem_pa_set(phys, size);
	if (rc < 0)
		cam_err("%s: pa set error:%d", __func__, rc);
	else
		hisp_secmem_set = true;

err_buf_size:
	hicam_buf_put_sgtable(sgt);
	return rc;
}

int hisp_release_sec_fw_buffer(void)
{
	int rc = hisp_secmem_pa_release();

	if (rc < 0)
		cam_err("%s: fail, rc:%d", __func__, rc);
	else
		hisp_secmem_set = false;
	return rc;
}

int hisp_set_mdc_buffer(struct hisp_cfg_data *cfg)
{
	int rc;
	uint32_t size;
	phys_addr_t phys;
	struct sg_table *sgt;

	sgt = hicam_buf_get_sgtable(cfg->share_fd);
	if (IS_ERR_OR_NULL(sgt)) {
		cam_err("%s: error get sg_table of share_fd:%d.",
			__func__, cfg->share_fd);
		return PTR_ERR(sgt);
	}

	phys = sg_phys(sgt->sgl);
	size = sgt->sgl->length;

	if (size < cfg->buf_size) {
		cam_err("%s: real size [%u] less than claimed [%u].",
			__func__, size, cfg->buf_size);
		rc = -ERANGE;
		goto err_buf_size;
	}

	// incase something happens, mdc is not release as expected,
	// like Tomestone. we release it here before next set.
	if (hisp_mdcmem_set) {
		cam_info("%s: abnormal case, but do it.", __func__);
		(void)hisp_release_mdc_buffer();
	}

	rc = hisp_mdcmem_pa_set(phys, size, cfg->share_fd);
	if (rc < 0)
		cam_err("%s: pa set error:%d", __func__, rc);
	else
		hisp_mdcmem_set = true;

err_buf_size:
	hicam_buf_put_sgtable(sgt);
	return rc;
}

int hisp_release_mdc_buffer(void)
{
	int rc = hisp_mdcmem_pa_release();

	if (rc < 0)
		cam_err("%s: fail, rc:%d", __func__, rc);
	else
		hisp_mdcmem_set = false;
	return rc;
}
//lint -restore
