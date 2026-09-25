/*
 * jpeg_base.c
 *
 * implement for jpeg subsystem.
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * luolidong <luolidong@huawei.com>
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
 */
#include <linux/version.h>
#include <media/camera/jpeg/jpeg_base.h>

#include <linux/mutex.h>
#include "cam_log.h"
#include <securec.h>

#define MAX(a,b) ((a)>(b)?(a):(b))
#define JPEG_TRANSITION_CNT (5)
struct transition_info {
	unsigned long source_rate;
	unsigned long dest_rate;
	unsigned long transition;
};

struct jpeg_transition_rate {
	unsigned int transition_rate_cnt;
	struct transition_info transition_info[JPEG_TRANSITION_CNT];
};

enum {
    JPEG_ENC = 0,
    JPEG_DEC,
    JPEG_IPP,
    JPEG_MAX
};

static unsigned long jpeg_rate[JPEG_MAX] = {0,0};
static unsigned int jpeg_enable_ref = 0;
static unsigned long jpeg_current_rate = 0;
static struct jpeg_transition_rate jpeg_ipp_transition_rate = {
	0,
	{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } }
};

static DEFINE_MUTEX(jpeg_base_lock);

extern int memset_s(void *dest, size_t destMax, int c, size_t count);

static int jpeg_set_rate(struct clk *clk, unsigned long rate, int index);

int jpeg_enc_set_rate(struct clk *clk, unsigned long rate)
{
    if (clk == NULL) {
        cam_err("%s: enc clk is null! (%d)",__func__, __LINE__);
        return -EINVAL;
    }

    if (rate == 0) {
        cam_err("%s: enc rate is zero! (%d)",__func__, __LINE__);
        return -EINVAL;
    }

    return jpeg_set_rate(clk, rate, JPEG_ENC);
}

int jpeg_enc_clk_prepare_enable(struct clk *clk)
{
    int ret;

    if (clk == NULL) {
        cam_err("%s: enc clk is null! (%d)",__func__, __LINE__);
        return -EINVAL;
    }

    mutex_lock(&jpeg_base_lock);

    jpeg_enable_ref++;

    ret = clk_prepare_enable(clk);

    mutex_unlock(&jpeg_base_lock);

    return ret;
}

void jpeg_enc_clk_disable_unprepare(struct clk *clk)
{
    if (clk == NULL) {
        cam_err("%s: enc clk is null! (%d)",__func__, __LINE__);
        return;
    }

    mutex_lock(&jpeg_base_lock);

    clk_disable_unprepare(clk);

    if (jpeg_enable_ref > 0) {
        jpeg_enable_ref--;
    }

    mutex_unlock(&jpeg_base_lock);
}

int jpeg_dec_set_rate(struct clk *clk, unsigned long rate)
{
    if (clk == NULL) {
        cam_err("%s: dec clk is null! (%d)",__func__, __LINE__);
        return -EINVAL;
    }

    if (rate == 0) {
        cam_err("%s: dec rate is zero! (%d)",__func__, __LINE__);
        return -EINVAL;
    }

    return jpeg_set_rate(clk, rate, JPEG_DEC);
}

int jpeg_dec_clk_prepare_enable(struct clk *clk)
{
    int ret;

    if (clk == NULL) {
        cam_err("%s: dec clk is null! (%d)",__func__, __LINE__);
        return -EINVAL;
    }

    mutex_lock(&jpeg_base_lock);

    jpeg_enable_ref++;

    ret = clk_prepare_enable(clk);

    mutex_unlock(&jpeg_base_lock);

    return ret;
}

void jpeg_dec_clk_disable_unprepare(struct clk *clk)
{
    if (clk == NULL) {
        cam_err("%s: dec clk is null! (%d)",__func__, __LINE__);
        return;
    }

    mutex_lock(&jpeg_base_lock);

    clk_disable_unprepare(clk);

    if (jpeg_enable_ref > 0) {
        jpeg_enable_ref--;
    }

    mutex_unlock(&jpeg_base_lock);
}

int jpeg_ipp_set_rate(struct clk* clk, unsigned long rate)
{
    if (clk == NULL)
    {
        cam_err("%s: ipp clk is null! (%d)", __func__, __LINE__);
        return -EINVAL;
    }

    if (rate == 0)
    {
        cam_err("%s: ipp rate is zero! (%d)", __func__, __LINE__);
        return -EINVAL;
    }

    return jpeg_set_rate(clk, rate, JPEG_IPP);
}

int jpeg_ipp_clk_prepare_enable(struct clk* clk)
{
    int ret;

    if (clk == NULL)
    {
        cam_err("%s: ipp clk is null! (%d)", __func__, __LINE__);
        return -EINVAL;
    }

    mutex_lock(&jpeg_base_lock);
    jpeg_enable_ref++;
    ret = clk_prepare_enable(clk);
    mutex_unlock(&jpeg_base_lock);
	
    return ret;
}

void jpeg_ipp_clk_disable_unprepare(struct clk* clk)
{
    if (clk == NULL)
    {
        cam_err("%s: ipp clk is null! (%d)", __func__, __LINE__);
        return;
    }

    mutex_lock(&jpeg_base_lock);
    clk_disable_unprepare(clk);

    if (jpeg_enable_ref > 0)
    {
        jpeg_enable_ref--;
    }

    mutex_unlock(&jpeg_base_lock);
}

bool jpeg_ipp_set_transition_rate(unsigned long source_rate,
		unsigned long dest_rate, unsigned long transition_rate)
{
	unsigned int index = jpeg_ipp_transition_rate.transition_rate_cnt;
	cam_info("%s: source_rate %lu dest_rate %lu, transition_rate %lu",
			__func__, source_rate, dest_rate, transition_rate);

	if (index >= JPEG_TRANSITION_CNT) {
		cam_err("%s: transition rate is full", __func__);
		return false;
	}

	jpeg_ipp_transition_rate.transition_info[index].source_rate = source_rate;
	jpeg_ipp_transition_rate.transition_info[index].dest_rate = dest_rate;
	jpeg_ipp_transition_rate.transition_info[index].transition =\
																transition_rate;
	jpeg_ipp_transition_rate.transition_rate_cnt = index + 1;

	return true;
}

void jpeg_ipp_clear_transition_rate(void)
{
	int rc;
	cam_info("%s: clear transition rate", __func__);
	rc = memset_s(&jpeg_ipp_transition_rate, sizeof(jpeg_ipp_transition_rate),
			0, sizeof(jpeg_ipp_transition_rate));
	if (rc != EOK) {
	    cam_err("%s: memset_s fail rc.%d\n", __func__, rc);
	    return;
	}
}

static unsigned long jpeg_find_set_transition_rate(unsigned long source_rate,
		unsigned long dest_rate)
{
	unsigned int i;
	unsigned long source;
	unsigned long dest;

	for (i = 0; i < jpeg_ipp_transition_rate.transition_rate_cnt; i++) {
		source = jpeg_ipp_transition_rate.transition_info[i].source_rate;
		dest = jpeg_ipp_transition_rate.transition_info[i].dest_rate;
		if (source_rate == source && dest_rate == dest) {
			return jpeg_ipp_transition_rate.transition_info[i].transition;
		}
	}

	return 0;
}

static int jpeg_set_rate(struct clk *clk, unsigned long rate, int index)
{
	int ret = 0;
	unsigned long max_rate;
	unsigned long transition_rate;
	unsigned long rate_backup;

	cam_info("%s: index %d, rate %lu, rate[ENC]=%lu, rate[DEC]=%lu,\
			rate[IPP]=%lu", __func__, index, rate, jpeg_rate[JPEG_ENC],
			jpeg_rate[JPEG_DEC], jpeg_rate[JPEG_IPP]);

	mutex_lock(&jpeg_base_lock);

	rate_backup = jpeg_rate[index];
	jpeg_rate[index] = rate;

	max_rate = MAX(MAX(jpeg_rate[JPEG_ENC], jpeg_rate[JPEG_DEC]),\
			jpeg_rate[JPEG_IPP]);

	if (jpeg_current_rate == max_rate) {
		mutex_unlock(&jpeg_base_lock);
		return 0;
	}

	transition_rate = jpeg_find_set_transition_rate(jpeg_current_rate,
			max_rate);
	if (transition_rate) {
		ret = clk_set_rate(clk, transition_rate);
		cam_info("%s : set transition_rate %lu", __func__, transition_rate);
		if (ret) {
			cam_err("%s : set transition rate[%lu] ret[%d] fail!",
					__func__, transition_rate, ret);
			jpeg_rate[index] = rate_backup;
			mutex_unlock(&jpeg_base_lock);
			return ret;
		}
		jpeg_current_rate = transition_rate;
	}

	ret = clk_set_rate(clk, max_rate);
	if (ret) {
		cam_err("%s : set dest rate ret[%d] fail!", __func__, ret);
		jpeg_rate[index] = rate_backup;
	} else {
		jpeg_current_rate = max_rate;
	}

	mutex_unlock(&jpeg_base_lock);

	return ret;
}

