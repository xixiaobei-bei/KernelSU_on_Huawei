/* Module internals
 *
 * Copyright (C) 2017 Hisilicon, Inc. All Rights Reserved.
 *
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of
 * Hisilicon Inc. and its licensors, and are licensed to the recipient
 * under the terms of a separate license agreement.  They may be
 * adapted and modified by bona fide purchasers under the terms of the
 * separate license agreement for internal use, but no adapted or
 * modified version may be disclosed or distributed to third parties
 * in any manner, medium, or form, in whole or in part, without the
 * prior written consent of Hisilicon Inc.
 */

#ifndef _IPU_CLOCK_H
#define _IPU_CLOCK_H

#include <linux/clk.h>
#include <linux/mutex.h>

#define IPU_CLOCK_UNSET (0)

struct ics_clock {
	struct clk *ipu_clk_ptr;
	struct clk *vcodecbus_clk_ptr;
	struct clk *vote_high_volt;
	struct clk *vote_normal_volt;
	unsigned int ipu_low;
	unsigned int ipu_middle;
	unsigned int ipu_high;
	unsigned int ipu_low_temperature;
	unsigned int vcodecbus_low;
	unsigned int vcodecbus_middle;
	unsigned int vcodecbus_high;
	unsigned int vcodecbus_default;
	unsigned int vcodecbus_high2default;
	unsigned int default_rate;
	unsigned int start_rate;
	unsigned int curr_rate;
	unsigned int stop_rate;
	unsigned int voted_peri_volt;
	struct mutex clk_mutex;
	bool lpm3_set_vcodecbus;
};

extern int ipu_clock_init(struct device *dev, struct ics_clock *clk, bool lpm3_set_vcodecbus);
extern int ipu_clock_start(struct ics_clock *clk);
extern void ipu_clock_stop(struct ics_clock *clk);
extern int ipu_clock_set_rate(struct ics_clock *clk, unsigned int new_rate);
extern int ipu_vote_peri(struct ics_clock *clk);
extern void ipu_vote_peri_withdraw(struct ics_clock *clk);
extern void ipu_clock_set_start_default(struct ics_clock *clk);
extern int ipu_clock_set_start_rate(struct ics_clock *clk, unsigned int new_rate);
#endif
