#ifndef __FRAME_EXTERN_H
#define __FRAME_EXTERN_H

#ifdef CONFIG_FRAME_RTG
bool is_frame_task(struct task_struct *task);
int set_frame_rate(int rate);
int set_frame_margin(int margin);
int set_frame_status(unsigned long status);
int set_frame_max_util(int max_util);
int set_frame_timestamp(unsigned long timestamp);

void update_frame_info_tick(struct related_thread_group *grp);
int update_frame_isolation(void);
#else
static inline bool is_frame_task(struct task_struct *task) { return false; }
static inline int set_frame_rate(int rate) { return 0; }
static inline int set_frame_margin(int margin) { return 0; }
static inline int set_frame_status(unsigned long status) { return 0; }
static inline int set_frame_max_util(int max_util) { return 0; }
static inline int set_frame_timestamp(unsigned long timestamp) { return 0; }


static inline void update_frame_info_tick(struct related_thread_group *grp) { }
static inline int update_frame_isolation(void) { return 1; }
#endif

#ifdef CONFIG_TRACE_FRAME_SYSTRACE
#define FRAME_SYSTRACE	       trace_clock_set_rate
#else
#define FRAME_SYSTRACE(format, ...)
#endif /*  CONFIG_TRACE_FRAME_SYSTRACE	*/

#endif
