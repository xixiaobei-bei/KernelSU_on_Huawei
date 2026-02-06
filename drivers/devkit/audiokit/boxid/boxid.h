/*
 * boxid.h
 *
 * boxid driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#ifndef __BOXID_H__
#define __BOXID_H__

enum {
	SPEAKER_ID = 0,
	RECEIVER_ID,
	BOX_3RD_ID,
	BOX_4TH_ID,
	BOX_5FIF_ID,
	BOX_6SIX_ID,
	BOX_7SEV_ID,
	BOX_8EIG_ID,
	BOX_NUM_MAX,
};

#define DEVICEBOX_ID_GET_SPEAKER     _IOR('H', 0x01, __u32)
#define DEVICEBOX_ID_GET_RECEIVER    _IOR('H', 0x02, __u32)
#define DEVICEBOX_ID_GET_3RDBOX      _IOR('H', 0x03, __u32)
#define DEVICEBOX_ID_GET_4THBOX      _IOR('H', 0x04, __u32)
#define DEVICEBOX_ID_GET_5THBOX      _IOR('H', 0x05, __u32)
#define DEVICEBOX_ID_GET_6THBOX      _IOR('H', 0x06, __u32)
#define DEVICEBOX_ID_GET_7THBOX      _IOR('H', 0x07, __u32)
#define DEVICEBOX_ID_GET_8THBOX      _IOR('H', 0x08, __u32)
#define DEVICEBOX_ID_GET_CONSISTENT_SUPPORT _IOR('H', 0xfd, __u32)
#define DEVICEBOX_ID_GET_CONSISTENT_TEST    _IOR('H', 0xfe, __u32)
#define DEVICEBOX_ID_GET_BOX_NUM     _IOR('H', 0xff, __u32)

#ifdef CONFIG_HUAWEI_DEVICEBOX_AUDIO
int boxid_read(int id);
#else
static inline int boxid_read(int id) { return 0; }
#endif

#endif // __BOXID_H__
