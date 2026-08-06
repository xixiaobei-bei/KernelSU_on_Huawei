#ifndef __FRAME_RTG_H
#define __FRAME_RTG_H

#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/cpumask.h>
#include <linux/sched.h>
#include "frame.h"


#define FRAME_START		(1 << 0)
#define FRAME_END		(1 << 1)
#define FRAME_INVALID		(1 << 2)
//#define FRAME_RESET_UTIL_MODE	(1 << 3)
#define FRAME_USE_MARGIN_IMME	(1 << 4)
#define FRAME_TIMESTAMP_SKIP_START	(1 << 5)
#define FRAME_TIMESTAMP_SKIP_END	(1 << 6)
#define FRAME_SETTIME		(FRAME_START | FRAME_END | \
				FRAME_USE_MARGIN_IMME)
#define FRAME_SETTIME_PARAM	-1


struct frame_info {
	/*
	 * use rtg load tracking in frame_info
	 * rtg->curr_window_load  -=> the workload of current frame
	 * rtg->prev_window_load  -=> the workload of last frame
	 * rtg->curr_window_exec  -=> the thread's runtime of current frame
	 * rtg->prev_window_exec  -=> the thread's runtime of last frame
	 * rtg->prev_window_time  -=> the actual time of the last frame
	 */
	struct related_thread_group *rtg;

	unsigned int qos_frame;
	u64 qos_frame_time;

	/*
	 * frame_vload : the emergency level of current frame.
	 * max_vload_time : the timeline frame_load increase to FRAME_MAX_VLOAD
	 * it's always equal to 2 * qos_frame_time / NSEC_PER_MSEC
	 *
	 * The closer to the deadline, the higher emergency of current
	 * frame, so the frame_vload is only related to frame time,
	 * and grown with time.
	 */
	u64 frame_vload;
	int vload_margin;
	int max_vload_time;

	unsigned long prev_fake_load_util;
	unsigned long prev_frame_load_util;
	unsigned long prev_frame_time;
	unsigned long prev_frame_exec;
	unsigned long prev_frame_load;

	u64 frame_util;

	unsigned long status;
	int frame_max_util;

	bool margin_imme;
	bool timestamp_skipped;
};

struct related_thread_group*
lookup_related_thread_group(unsigned int group_id);

static inline struct related_thread_group *
frame_info_rtg(struct frame_info *frame_info)
{
	return frame_info->rtg;
}

static inline struct group_time *
frame_info_rtg_load(struct frame_info *frame_info)
{
	return &frame_info_rtg(frame_info)->time;
}

#endif
