/*
 * l3cache_share.h
 *
 * l3cache share
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __L3CACHE_SHARE_H__
#define __L3CACHE_SHARE_H__

enum module_id {
	DSS_IDLE = 0,
	VDEC,
	VENC,
	AUDIO,
};

enum share_type {
	PRIVATE = 0,
	SHARE,
};

struct l3_cache_request_params {
	enum module_id id;
	unsigned int request_size;
	enum share_type type;
};

struct l3_cache_release_params {
	enum module_id id;
};

#define L3SHARE_MON_START			0x1
#define L3SHARE_MON_STOP			0x2
#define L3SHARE_ACP_ENABLE			0x3
#define L3SHARE_ACP_DISABLE			0x4

/*
 * L3 Cache request and release.
 * It will open/close all ACP port and poweron/poweroff L3 partition for ACP.
 * Must be called in pair. Call l3_cache_request twice will increase a vote.
 * Params ignored for test. You can pass 0 or anything.
 */
int l3_cache_request(struct l3_cache_request_params *request_params);
int l3_cache_release(struct l3_cache_release_params *release_params);

/*
 * register callback to receive notify
 * when DSS enter videoidle to request L3Cache,
 * or exit videoidle to release,
 * so take some actions
 */

#ifdef CONFIG_L3CACHE_SHARE
int register_l3share_acp_notifier(struct notifier_block *nb);
int unregister_l3share_acp_notifier(struct notifier_block *nb);
#endif

#endif /* __L3CACHE_SHARE_H__ */
