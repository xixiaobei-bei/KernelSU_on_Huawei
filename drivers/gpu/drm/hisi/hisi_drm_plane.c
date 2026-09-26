/* Copyright (c) 2018-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
/*lint -e502 -e559 -e574 -e605*/
#include <drm/drmP.h>
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_plane_helper.h>
#include <drm/drm_fourcc.h>

#include "hisi_drm_drv.h"
#include "hisi_drm_dpe_utils.h"
#include "hisi_mmbuf_manager.h"
#include "hisi_drm_dss_set_regs.h"

/* Channel capabilities defination */
enum CHANNEL_CAP {
	/* Channel attributes, used to config the hardware */
	CH_CAP_CSC,             /* Color space Conversion */
	CH_CAP_ROT,             /* Rotation */
	CH_CAP_TILE_ROT,
	CH_CAP_SCL,             /* Scale */
	CH_CAP_YUVSP,           /* Can handle YUV semi-planar data */
	CH_CAP_YUVP,            /* Can handle YUV planar data */
	CH_CAP_FBC,             /* Compression */
	CH_CAP_FBD,             /* Decompression */
	CH_CAP_DIM,             /* DIM */
	CH_CAP_BASE,            /* Base */
	CH_CAP_SEC,             /* Security supported */
	CH_CAP_ALPHA_MIX,
	CH_CAP_ALW_ON,          /* Alwanys on, i.e it's used for LCD idle refresh,
								OR for online channel when other channels offlined */
	CH_CAP_CLB,             /* Support output colorbar. */
	CH_CAP_DSI,             /* Support output to MIPI DSI. */
	CH_CAP_EDP,             /* Support output to eDP. */
	CH_CAP_MHL,             /* Support output to MHL/HDMI. */
	CH_CAP_YUVI,            /* Can handle YUV packed data */
	CH_CAP_YUVSP_INTERLACE, /* Can handle YUV semi-planar interlace data */
	CH_CAP_HIGH_SCL,        /* 4 order Scale, video channel */
	CH_CAP_2D_SHARPNESS,    /* support 2d sharpess */
	CH_CAP_MDC,             /* Can support used by mdc */
	/* Special capabilities */
	CH_CAP_SRC,
	CH_CAP_HFBCE,
	CH_CAP_HFBCD,

	NUM_OF_CAPS
};

/* Channel capabilities mask */
#define CAPM_CSC                (1 << CH_CAP_CSC)
#define CAPM_ROT                (1 << CH_CAP_ROT)
#define CAPM_TILE_ROT           (1 << CH_CAP_TILE_ROT)
#define CAPM_SCL                (1 << CH_CAP_SCL)
#define CAPM_YUVSP              (1 << CH_CAP_YUVSP)
#define CAPM_YUVP               (1 << CH_CAP_YUVP)
#define CAPM_FBC                (1 << CH_CAP_FBC)
#define CAPM_FBD                (1 << CH_CAP_FBD)
#define CAPM_DIM                (1 << CH_CAP_DIM)
#define CAPM_BASE               (1 << CH_CAP_BASE)
#define CAPM_SEC                (1 << CH_CAP_SEC)
#define CAPM_ALPHA_MIX          (1 << CH_CAP_ALPHA_MIX)
#define CAPM_ALW_ON             (1 << CH_CAP_ALW_ON)
#define CAPM_CLB                (1 << CH_CAP_CLB)
#define CAPM_DSI                (1 << CH_CAP_DSI)
#define CAPM_EDP                (1 << CH_CAP_EDP)
#define CAPM_MHL                (1 << CH_CAP_MHL)
#define CAPM_YUVI               (1 << CH_CAP_YUVI)
#define CAPM_YUVSP_INTERLACE    (1 << CH_CAP_YUVSP_INTERLACE)
#define CAPM_HIGH_SCL           (1 << CH_CAP_HIGH_SCL)
#define CAPM_2D_SHARPNESS       (1 << CH_CAP_2D_SHARPNESS)
#define CAPM_MDC                (1 << CH_CAP_MDC)
/* Used by ChannelMgr, should NOT be used by any policy or other hwc class */
#define CAPM_SRC                (1 << CH_CAP_SRC)
#define CAPM_HFBCE              (1 << CH_CAP_HFBCE)
#define CAPM_HFBCD              (1 << CH_CAP_HFBCD)

struct channel {
	const char *name;
	uint32_t caps;
};

static struct channel channel_info[] = {
	/* ------------- source channels ------------ */
	{"DSS_RCHN_D2", CAPM_SRC | CAPM_DIM | CAPM_YUVI | CAPM_FBD},
	{"DSS_RCHN_D3", CAPM_SRC | CAPM_DIM | CAPM_YUVI},

	{"DSS_RCHN_V0", CAPM_SRC | CAPM_DIM | CAPM_SCL | CAPM_YUVSP |
		CAPM_YUVI | CAPM_YUVP | CAPM_YUVSP_INTERLACE | CAPM_FBD |
			CAPM_HIGH_SCL | CAPM_2D_SHARPNESS | CAPM_HFBCD |
				CAPM_ROT},

	{"DSS_RCHN_G0", CAPM_SRC | CAPM_DIM | CAPM_SCL | CAPM_FBD },

	{"DSS_RCHN_V1", CAPM_SRC | CAPM_DIM | CAPM_SCL | CAPM_YUVSP |
		CAPM_YUVI | CAPM_YUVP | CAPM_YUVSP_INTERLACE | CAPM_FBD |
			CAPM_HIGH_SCL | CAPM_HFBCD | CAPM_ROT | CAPM_MDC},

	{"DSS_RCHN_G1", CAPM_SRC | CAPM_DIM | CAPM_SCL | CAPM_FBD},

	{"DSS_RCHN_D0", CAPM_SRC | CAPM_DIM | CAPM_YUVI},
	{"DSS_RCHN_D1", CAPM_SRC | CAPM_DIM | CAPM_YUVI},

	/* ---------- writeback channels ----------- */
	{"DSS_WCHN_W0", CAPM_ROT | CAPM_FBC },
	{"DSS_WCHN_W1", CAPM_ROT | CAPM_FBC | CAPM_HFBCE},

	/* ------------- source channels ------------ */
	{"DSS_RCHN_V2", CAPM_SRC | CAPM_DIM | CAPM_YUVSP | CAPM_YUVI |
		CAPM_YUVP | CAPM_YUVSP_INTERLACE | CAPM_MDC}
};

static const u32 channel_formats1[] = {
	DRM_FORMAT_RGB565, DRM_FORMAT_BGR565,
	DRM_FORMAT_XRGB8888, DRM_FORMAT_XBGR8888,
	DRM_FORMAT_RGBX8888, DRM_FORMAT_RGBA8888,
	DRM_FORMAT_BGRA8888, DRM_FORMAT_YVU420,
	DRM_FORMAT_NV16, DRM_FORMAT_NV21,
	DRM_FORMAT_YUV422, DRM_FORMAT_NV12,
	DRM_FORMAT_YUV420, DRM_FORMAT_RGBA1010102,
};

static const u64 format_modifiers[] = {
	AFBC_FORMAT_MODIFIER,
	DRM_FORMAT_MOD_INVALID
};

static const struct drm_prop_enum_list hisi_blend_enum_list[] = {
	{ HISI_BLENDING_NONE, "None" },
	{ HISI_BLENDING_PREMULT, "Pre-multiplied" },
	{ HISI_BLENDING_COVERAGE, "Coverage" },
};

static uint32_t dss_get_channel_formats(u8 ch, const uint32_t **formats)
{
	switch (ch) {
	case DSS_RCHN_D2:
	case DSS_RCHN_D3:
	case DSS_RCHN_V0:
	case DSS_RCHN_G0:
	case DSS_RCHN_V1:
	case DSS_RCHN_G1:
	case DSS_RCHN_D0:
	case DSS_RCHN_D1:
		*formats = channel_formats1;
		return ARRAY_SIZE(channel_formats1);
	default:
		HISI_DRM_ERR("no this channel %d\n", ch);
		*formats = NULL;
		return 0;
	}
}

struct src_info {
	u32 src_x;
	u32 src_y;
	u32 src_w;
	u32 src_h;
};

struct crtc_info {
	int crtc_x;
	int crtc_y;
	u32 crtc_w;
	u32 crtc_h;
};

static int hisi_drm_plane_atomic_check(struct drm_plane *plane,
					struct drm_plane_state *state)
{
	struct drm_framebuffer *fb = NULL;
	struct drm_crtc *crtc = NULL;
	struct drm_crtc_state *crtc_state = NULL;
	struct src_info src_info;
	struct crtc_info crtc_info;
	u32 fmt;

	UNUSED(plane);
	if (!state) {
		HISI_DRM_ERR("state is nullptr");
		return -EINVAL;
	}

	fb = state->fb;
	crtc = state->crtc;

	if (!crtc || !fb)
		return 0;

	src_info.src_x = state->src_x >> 16;
	src_info.src_y = state->src_y >> 16;
	src_info.src_w = state->src_w >> 16;
	src_info.src_h = state->src_h >> 16;
	crtc_info.crtc_x = state->crtc_x;
	crtc_info.crtc_y = state->crtc_y;
	crtc_info.crtc_w = state->crtc_w;
	crtc_info.crtc_h = state->crtc_h;

	drm_check_and_return(fb->format == NULL, -EINVAL, ERR, "format is null");
	fmt = dss_get_format(fb->format->format, fb->modifier);
	if (fmt == HISI_DSS_FORMAT_UNSUPPORT)
		return -EINVAL;

	crtc_state = drm_atomic_get_crtc_state(state->state, crtc);
	if (IS_ERR(crtc_state))
		return PTR_ERR(crtc_state);

	if (src_info.src_x + src_info.src_w > fb->width ||
		src_info.src_y + src_info.src_h > fb->height)
		return -EINVAL;

	if (crtc_info.crtc_x < 0 || crtc_info.crtc_y < 0)
		return -EINVAL;

	if (state->rotation != DRM_MODE_ROTATE_0) {
		HISI_DRM_INFO("not support rotation");
		return -EINVAL;
	}

	if (crtc_info.crtc_x + crtc_info.crtc_w < crtc_state->adjusted_mode.hdisplay ||
		crtc_info.crtc_y + crtc_info.crtc_h < crtc_state->adjusted_mode.vdisplay) {
		state->crtc_x = crtc_info.crtc_x * crtc_info.crtc_w /
			crtc_state->adjusted_mode.hdisplay;
		state->crtc_y = crtc_info.crtc_y * crtc_info.crtc_h /
			crtc_state->adjusted_mode.vdisplay;
		state->crtc_w = crtc_state->adjusted_mode.hdisplay;
		state->crtc_h = crtc_state->adjusted_mode.vdisplay;
		HISI_DRM_DEBUG("[SCALE] Adjust CRTC, [%u, %u, %u, %u]",
				state->crtc_x, state->crtc_y, state->crtc_w,
					state->crtc_h);
	}

	return 0;
}

static void hisi_drm_plane_atomic_update(struct drm_plane *plane,
					struct drm_plane_state *old_state)
{
	struct drm_plane_state *state = NULL;
	struct hisi_drm_plane *hisi_plane = NULL;

	UNUSED(old_state);
	if (!plane) {
		HISI_DRM_ERR("plane is nullptr");
		return;
	}

	state = plane->state;
	if (!state) {
		HISI_DRM_DEBUG("state is NULL");
		return;
	}

	hisi_plane = to_hisi_drm_plane(plane);

	HISI_DRM_DEBUG("channel id:%d\n", hisi_plane->chn);

	if (!state->crtc) {
		HISI_DRM_ERR("there is no crtc in plane_state");
		return;
	}

	hisi_dss_update_layer(plane);
}

static void hisi_drm_plane_atomic_disable(struct drm_plane *plane,
					struct drm_plane_state *old_state)
{
	struct hisi_drm_plane *hisi_plane = NULL;
	struct hisi_drm_crtc *hisi_crtc = NULL;

	if (!plane) {
		HISI_DRM_ERR("plane is nullptr");
		return;
	}
	if (!old_state) {
		HISI_DRM_ERR("old_state is nullptr");
		return;
	}
	if (!old_state->crtc) {
		HISI_DRM_ERR("there is no crtc in old_state");
		return;
	}

	hisi_plane = to_hisi_drm_plane(plane);
	hisi_crtc = to_hisi_drm_crtc(old_state->crtc);

	hisi_crtc->pre_layer_mask &= ~(1 << hisi_plane->chn);
	hisi_dss_disable_channel(hisi_crtc, hisi_plane->chn);
}

int hisi_drm_plane_atomic_get_property(struct drm_plane *plane,
					const struct drm_plane_state *state,
					struct drm_property *property,
					uint64_t *val)
{
	struct hisi_drm_plane *hisi_plane = NULL;
	struct hisi_drm_plane_state *hisi_state = NULL;

	if (!plane) {
		HISI_DRM_ERR("plane is nullptr");
		return -EINVAL;
	}
	if (!state) {
		HISI_DRM_ERR("state is nullptr");
		return -EINVAL;
	}
	if (!property) {
		HISI_DRM_ERR("property is nullptr");
		return -EINVAL;
	}
	if (!val) {
		HISI_DRM_ERR("val is nullptr");
		return -EINVAL;
	}

	hisi_plane = to_hisi_drm_plane(plane);
	hisi_state = to_hisi_plane_state(state);
	if (property == hisi_plane->props.caps)
		*val = hisi_plane->caps;
	else if (property == hisi_plane->props.alpha)
		*val = hisi_state->alpha;
	else if (property == hisi_plane->props.blending)
		*val = hisi_state->blending;
	else if (property == hisi_plane->props.color)
		*val = hisi_state->color;
	else if (property == hisi_plane->props.transform)
		*val = hisi_state->transform;
	else if (property == hisi_plane->props.need_cap)
		*val = hisi_state->need_cap;
	else
		return -EINVAL;

	HISI_DRM_DEBUG("PROP:%s, val:0x%lx\n", property->name, *val);

	return 0;
}

int hisi_drm_plane_atomic_set_property(struct drm_plane *plane,
				struct drm_plane_state *state,
				struct drm_property *property,
				uint64_t val)
{
	struct hisi_drm_plane *hisi_plane = NULL;
	struct hisi_drm_plane_state *hisi_state = NULL;

	if (!plane) {
		HISI_DRM_ERR("plane is nullptr");
		return -EINVAL;
	}
	if (!state) {
		HISI_DRM_ERR("state is nullptr");
		return -EINVAL;
	}
	if (!property) {
		HISI_DRM_ERR("property is nullptr");
		return -EINVAL;
	}

	hisi_plane = to_hisi_drm_plane(plane);
	hisi_state = to_hisi_plane_state(state);
	if (property == hisi_plane->props.alpha)
		hisi_state->alpha = val;
	else if (property == hisi_plane->props.blending)
		hisi_state->blending = val;
	else if (property == hisi_plane->props.color)
		hisi_state->color = val;
	else if (property == hisi_plane->props.transform)
		hisi_state->transform = val;
	else if (property == hisi_plane->props.need_cap)
		hisi_state->need_cap = val;
	else
		return -EINVAL;

	return 0;
}

struct modifier_format {
	uint64_t modifier;
	uint32_t format;
};

static struct modifier_format modifier_format_supported[] = {
	{DRM_FORMAT_MOD_LINEAR, DRM_FORMAT_RGB565},
	{DRM_FORMAT_MOD_LINEAR, DRM_FORMAT_BGR565},
	{DRM_FORMAT_MOD_LINEAR, DRM_FORMAT_XRGB8888},
	{DRM_FORMAT_MOD_LINEAR, DRM_FORMAT_XBGR8888},
	{DRM_FORMAT_MOD_LINEAR, DRM_FORMAT_RGBX8888},
	{DRM_FORMAT_MOD_LINEAR, DRM_FORMAT_RGBA8888},
	{DRM_FORMAT_MOD_LINEAR, DRM_FORMAT_BGRA8888},
	{DRM_FORMAT_MOD_LINEAR, DRM_FORMAT_YVU420},
	{DRM_FORMAT_MOD_LINEAR, DRM_FORMAT_NV16},
	{DRM_FORMAT_MOD_LINEAR, DRM_FORMAT_NV21},
	{DRM_FORMAT_MOD_LINEAR, DRM_FORMAT_YUV422},
	{DRM_FORMAT_MOD_LINEAR, DRM_FORMAT_NV12},
	{DRM_FORMAT_MOD_LINEAR, DRM_FORMAT_YUV420},
	{DRM_FORMAT_MOD_LINEAR, DRM_FORMAT_RGBA1010102},
	{AFBC_FORMAT_MODIFIER, DRM_FORMAT_ABGR8888},
	{AFBC_FORMAT_MODIFIER, DRM_FORMAT_BGR565},
	{AFBC_FORMAT_MODIFIER, DRM_FORMAT_XBGR8888},
	{AFBC_FORMAT_MODIFIER, DRM_FORMAT_BGR888},
	{AFBC_FORMAT_MODIFIER, DRM_FORMAT_ABGR2101010},
	{AFBC_FORMAT_MODIFIER, DRM_FORMAT_XRGB8888}
};

bool hisi_drm_format_mod_supported(struct drm_plane *plane,
	uint32_t format,
	uint64_t modifier)
{
	bool ret = false;
	int i;
	int num = sizeof(modifier_format_supported) / sizeof(struct modifier_format);

	UNUSED(plane);

	for (i = 0; i < num; ++i) {
		if (modifier == modifier_format_supported[i].modifier && format == modifier_format_supported[i].format) {
			ret = true;
			return ret;
		}
	}

	HISI_DRM_DEBUG("Not support format %d modifier %lld\n", format, modifier);
	return ret;
}

static void hisi_drm_plane_reset(struct drm_plane *plane)
{
	struct hisi_drm_plane_state *hisi_state = NULL;
	if (!plane) {
		HISI_DRM_ERR("plane is nullptr");
		return;
	}

	if (plane->state) {
		hisi_state = to_hisi_plane_state(plane->state);
		__drm_atomic_helper_plane_destroy_state(plane->state);
		kfree(hisi_state);
		plane->state = NULL;
	}

	hisi_state = kzalloc(sizeof(*hisi_state), GFP_KERNEL);
	if (hisi_state) {
		plane->state = &hisi_state->base;
		plane->state->plane = plane;
		plane->state->rotation = DRM_MODE_ROTATE_0;
	}
}

static struct drm_plane_state *
hisi_drm_plane_duplicate_state(struct drm_plane *plane)
{
	struct hisi_drm_plane_state *hisi_state = NULL;
	struct hisi_drm_plane_state *copy = NULL;

	if (!plane) {
		HISI_DRM_ERR("plane is nullptr");
		return NULL;
	}
	hisi_state = to_hisi_plane_state(plane->state);
	copy = kzalloc(sizeof(*hisi_state), GFP_KERNEL);
	if (!copy)
		return NULL;

	__drm_atomic_helper_plane_duplicate_state(plane, &copy->base);
	return &copy->base;
}

static void hisi_drm_plane_destroy_state(struct drm_plane *plane,
					   struct drm_plane_state *old_state)
{
	struct hisi_drm_plane_state *old_hisi_state = NULL;

	UNUSED(plane);
	if (!old_state) {
		HISI_DRM_ERR("old_state is nullptr");
		return;
	}
	old_hisi_state = to_hisi_plane_state(old_state);
	__drm_atomic_helper_plane_destroy_state(old_state);
	kfree(old_hisi_state);
}


static const struct drm_plane_helper_funcs hisi_drm_plane_helper_funcs = {
	.atomic_check = hisi_drm_plane_atomic_check,
	.atomic_update = hisi_drm_plane_atomic_update,
	.atomic_disable = hisi_drm_plane_atomic_disable,
};

static struct drm_plane_funcs hisi_drm_plane_funcs = {
	.update_plane = drm_atomic_helper_update_plane,
	.disable_plane = drm_atomic_helper_disable_plane,
	.destroy = drm_plane_cleanup,
	.reset = hisi_drm_plane_reset,
	.atomic_duplicate_state = hisi_drm_plane_duplicate_state,
	.atomic_destroy_state = hisi_drm_plane_destroy_state,
	.atomic_get_property = hisi_drm_plane_atomic_get_property,
	.atomic_set_property = hisi_drm_plane_atomic_set_property,
	.format_mod_supported = hisi_drm_format_mod_supported,
};

static int set_props_for_plane(struct hisi_drm_plane *hisi_plane, struct drm_device *dev)
{
	hisi_plane->props.alpha = drm_property_create_range(dev, 0, "alpha", 0, 0xffffffffffffffff);
	if (!hisi_plane->props.alpha) {
		HISI_DRM_ERR("create alpha properet failed\n");
		return -ENOMEM;
	}

	hisi_plane->props.blending = drm_property_create_enum(dev, 0,
		"pixel blend mode", hisi_blend_enum_list, ARRAY_SIZE(hisi_blend_enum_list));
	if (!hisi_plane->props.blending) {
		HISI_DRM_ERR("create blending properet failed\n");
		return -ENOMEM;
	}

	hisi_plane->props.color = drm_property_create_range(dev, 0, "color", 0, 0xffffffffffffffff);
	if (!hisi_plane->props.color) {
		HISI_DRM_ERR("create color properet failed\n");
		return -ENOMEM;
	}

	hisi_plane->props.transform = drm_property_create_range(dev, 0, "transform", 0, 0xffffffffffffffff);
	if (!hisi_plane->props.transform) {
		HISI_DRM_ERR("create transform properet failed\n");
		return -ENOMEM;
	}

	hisi_plane->props.need_cap = drm_property_create_range(dev, 0, "needCap", 0, 0xffffffff);
	if (!hisi_plane->props.need_cap) {
		HISI_DRM_ERR("create need_cap properet failed\n");
		return -ENOMEM;
	}

	hisi_plane->props.caps = drm_property_create_range(dev, DRM_MODE_PROP_IMMUTABLE, "CAPS", 0, 0xffffffffffffffff);
	if (!hisi_plane->props.caps) {
		HISI_DRM_ERR("create transform caps failed\n");
		return -ENOMEM;
	}
	return 0;

}

int hisi_plane_install_properties(struct drm_device *dev, struct hisi_drm_plane *hisi_plane,
			enum dss_chn_idx chn_idx)
{
	int ret;
	struct drm_plane *plane = NULL;

	UNUSED(chn_idx);
	if (!dev) {
		HISI_DRM_ERR("dev is nullptr");
		return -EINVAL;
	}
	if (!hisi_plane) {
		HISI_DRM_ERR("hisi_plane is nullptr");
		return -EINVAL;
	}

	plane = &hisi_plane->base;
	ret = drm_plane_create_zpos_property(plane, 0, 0, DSS_CHN_MAX - 1);
	if (ret) {
		HISI_DRM_ERR("create zpos property error, ret=%d\n", ret);
		return ret;
	}

	ret = set_props_for_plane(hisi_plane, dev);
	if (ret)
		return ret;

	drm_object_attach_property(&plane->base, hisi_plane->props.alpha, 0);
	drm_object_attach_property(&plane->base, hisi_plane->props.blending, 0);
	drm_object_attach_property(&plane->base, hisi_plane->props.color, 0);
	drm_object_attach_property(&plane->base, hisi_plane->props.transform, 0);
	drm_object_attach_property(&plane->base, hisi_plane->props.need_cap, 0);
	drm_object_attach_property(&plane->base, hisi_plane->props.caps, hisi_plane->caps);

	return 0;
}

int hisi_drm_plane_init(struct drm_device *dev,
	struct hisi_drm_plane *hisi_plane, uint32_t possible_crtcs,
		enum dss_chn_idx chn_idx)
{
	const u32 *fmts = NULL;
	u32 fmts_cnt;
	enum drm_plane_type type;
	struct drm_plane *plane = NULL;
	int ret = 0;

	HISI_DRM_INFO("+");

	hisi_plane->chn = chn_idx;
	hisi_plane->caps = channel_info[chn_idx].caps;
	hisi_plane->name = (char*)channel_info[chn_idx].name;
	plane = &hisi_plane->base;

	fmts_cnt = dss_get_channel_formats(hisi_plane->chn, &fmts);
	if (ret)
		return ret;

	hisi_plane->mmbuffer_addr = hisi_dss_mmbuf_alloc(MMBUFFER_SIZE_PER_CHANNEL);
	if (hisi_plane->mmbuffer_addr == 0) {
		HISI_DRM_ERR("alloc mmbuf failed");
		return -EFAULT;
	}

	type = (chn_idx == PRIMARY_CH || chn_idx == EXTERNAL_CH) ?
			DRM_PLANE_TYPE_PRIMARY :
			DRM_PLANE_TYPE_OVERLAY;

	ret = drm_universal_plane_init(dev, &hisi_plane->base, possible_crtcs,
					&hisi_drm_plane_funcs,
					fmts, fmts_cnt,
					format_modifiers, type, NULL);
	if (ret) {
		HISI_DRM_ERR("fail to init plane, chn=%d\n", hisi_plane->chn);
		return ret;
	}

	drm_plane_helper_add(plane, &hisi_drm_plane_helper_funcs);

	hisi_plane_install_properties(dev, hisi_plane, chn_idx);

	return 0;
}

void hisi_drm_plane_deinit(struct hisi_drm_plane *hisi_plane)
{
	if (!hisi_plane) {
		HISI_DRM_ERR("hisi_plane is NULL");
		return;
	}

	hisi_dss_mmbuf_free(hisi_plane->mmbuffer_addr);
}

/*lint +e502 +e559 +e574 +e605*/
