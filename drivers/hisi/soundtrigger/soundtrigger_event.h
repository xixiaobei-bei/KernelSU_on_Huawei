/*
 * soundtrigger_event.h
 *
 * soundtrigger event to userspace interfaces
 *
 * Copyright (c) 2014-2020 Huawei Technologies Co., Ltd.
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

#ifndef __HW_SOUNDTRIGGER_EVENT_H__
#define __HW_SOUNDTRIGGER_EVENT_H__

enum hw_soundtrigger_event {
	AUDIO_SOUNDTRIGGER_TRIGGER,
	AUDIO_SOUNDTRIGGER_TIMEOUT,
	AUDIO_SOUNDTRIGGER_SETMUTE,
	HW_SOUNDTRIGGER_OKGOOGLE,
	HW_SOUNDTRIGGER_RESERVED1,
	HW_SOUNDTRIGGER_RESERVED2,
};

void hw_soundtrigger_event_input(uint32_t soundtrigger_event);
void hw_soundtrigger_event_uevent(uint32_t soundtrigger_event);

#endif /* __HW_SOUNDTRIGGER_EVENT_H__ */
