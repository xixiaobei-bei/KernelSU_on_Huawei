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

#ifndef __HW_JACKY_KERNEL_HISP_INTERFACE_H__
#define __HW_JACKY_KERNEL_HISP_INTERFACE_H__

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

#include <linux/rpmsg.h>
#include <linux/platform_device.h>
#include <linux/videodev2.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>
#include <linux/clk.h>
#include <linux/dma-buf.h>

#include "hwcam_intf.h"
#include "cam_log.h"

#define HISP_WAIT_TIMEOUT	3000

//#define FAKE_FW 1
#undef FAKE_FW
/*define to 1 to disable lightweight runtime debugging checks */
#undef HISP_NDEBUG

#ifdef HISP_NDEBUG
#define hisp_assert(expr) do {} while (0)
#else
#define hisp_assert(expr) {\
	if (unlikely(!(expr))) {				\
		pr_err("Assertion failed! %s, %s,%s,line=%d\n",	\
		       #expr, __FILE__, __func__, __LINE__);	\
	}\
}
#endif

/**
 * hisp dts config info
 */
typedef struct _hisp_dt_data {
	struct clk *aclk;
	struct clk *aclk_dss;
	struct clk *pclk_dss;

	struct pinctrl *pinctrl;
	struct pinctrl_state *pinctrl_default;
	struct pinctrl_state *pinctrl_idle;
} hisp_dt_data_t;

struct _tag_hisp_vtbl;
typedef struct _tag_hisp_intf {
	struct _tag_hisp_vtbl *vtbl;
} hisp_intf_t;

typedef struct _tag_hisp_vtbl {
	char const *(*get_name) (hisp_intf_t *i);
	int (*config) (hisp_intf_t *i, void *cfg);
	int (*power_on) (hisp_intf_t *i);
	int (*power_off) (hisp_intf_t *i);
	int (*send_rpmsg) (hisp_intf_t *i, hisp_msg_t *m, size_t len);
	int (*recv_rpmsg) (hisp_intf_t *i, hisp_msg_t *m, size_t len);
	int (*open) (hisp_intf_t *i);
	int (*close) (hisp_intf_t *i);
} hisp_vtbl_t;

struct _tag_hisp_notify_vtbl;
typedef struct _tag_hisp_notify_intf {
	struct _tag_hisp_notify_vtbl *vtbl;
} hisp_notify_intf_t;

typedef struct _tag_hisp_notify_vtbl {
	void (*rpmsg_cb) (hisp_notify_intf_t *i, hisp_event_t *isp_ev);
} hisp_notify_vtbl_t;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
struct rpmsg_hdr {
    u32 src;
    u32 dst;
    u32 reserved;
    u16 len;
    u16 flags;
    u8 data[0];
} __packed;
#endif

static inline void
hisp_notify_intf_rpmsg_cb(hisp_notify_intf_t *i, hisp_event_t *isp_ev)
{
	if (NULL == i){
		cam_err("func %s: i is NULL",__func__);
		return;
	}
	if (NULL == i->vtbl){
		cam_err("func %s: i->vtbl is NULL",__func__);
		return;
	}
	hisp_assert(NULL != i->vtbl->rpmsg_cb);
	i->vtbl->rpmsg_cb(i, isp_ev);
}

static inline char  *hisp_intf_get_name(hisp_intf_t *i)
{
	if (NULL == i){
		return NULL;
	}
	if (NULL == i->vtbl){
		return NULL;
	}
	hisp_assert(NULL != i->vtbl->get_name);
	return (char *)i->vtbl->get_name(i);
}

extern int32_t
hisp_register(struct platform_device *pdev,
	      hisp_intf_t *hw, hisp_notify_intf_t **notify);

extern void
hisp_unregister(struct platform_device* pdev);
int hisp_get_dt_data(struct platform_device *pdev, hisp_dt_data_t *dt);

extern int
hisp_get_sg_table(int fd,struct device *dev,struct dma_buf **buf,struct dma_buf_attachment **attach,struct sg_table **table);

extern void
hisp_free_dma_buf(struct dma_buf **buf,struct dma_buf_attachment **attach,struct sg_table **table);
int hisp_set_sec_fw_buffer(struct hisp_cfg_data *cfg);
int hisp_release_sec_fw_buffer(void);
int hisp_set_mdc_buffer(struct hisp_cfg_data *cfg);
int hisp_release_mdc_buffer(void);
#endif /*__HW_JACKY_KERNEL_HISP_INTERFACE_H__ */
