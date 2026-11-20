/*
 * headset_debug.h
 *
 * headset debug driver
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
#ifndef __HEADSET_DEBUG_H__
#define __HEADSET_DEBUG_H__
#include <sound/soc.h>

enum headset_debug_jack_type {
	HEADSET_DEBUG_JACK_BIT_NONE = 0,
	HEADSET_DEBUG_JACK_BIT_HEADSET,
	HEADSET_DEBUG_JACK_BIT_HEADPHONE,
	HEADSET_DEBUG_JACK_BIT_HEADSET_NO_MIC,
	HEADSET_DEBUG_JACK_BIT_PLUGING,
	HEADSET_DEBUG_JACK_BIT_INVALID,
	HEADSET_DEBUG_JACK_BIT_LINEOUT,
};

enum headset_debug_key_type {
	HEADSET_DEBUG_KEY_0 = 10,
	HEADSET_DEBUG_KEY_1,
	HEADSET_DEBUG_KEY_2,
	HEADSET_DEBUG_KEY_3,
	HEADSET_DEBUG_KEY_4,
	HEADSET_DEBUG_KEY_5,
};

#ifdef CONFIG_HUAWEI_HEADSET_DEBUG
#ifdef CONFIG_HUAWEI_HEADSET_DEBUG_SWITCH
void headset_debug_init(struct snd_soc_jack *jack, struct switch_dev *sdev);
#else
void headset_debug_init(struct snd_soc_jack *jack);
#endif
void headset_debug_set_state(int state, bool use_input);
void headset_debug_uninit(void);
void headset_debug_input_init(struct input_dev *accdet_input_dev);
void headset_debug_input_set_state(int state, bool use_input);
void headset_debug_input_uninit(void);
#else
#ifdef CONFIG_HUAWEI_HEADSET_DEBUG_SWITCH
static inline void headset_debug_init(struct snd_soc_jack *jack,
		struct switch_dev *sdev)
#else
static inline void headset_debug_init(struct snd_soc_jack *jack);
#endif
{}
static inline void headset_debug_set_state(int state, bool use_input)
{}
static inline void headset_debug_uninit(void)
{}
static inline void headset_debug_input_init(struct input_dev *accdet_input_dev)
{}
static inline void headset_debug_input_set_state(int state, bool use_input)
{}
static inline void headset_debug_input_uninit(void)
{}
#endif // !CONFIG_HUAWEI_HEADSET_DEBUG

#endif // __HEADSET_DEBUG_H__
