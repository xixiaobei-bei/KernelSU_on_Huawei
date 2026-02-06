#ifndef DUBAI_DISPLAY_H
#define DUBAI_DISPLAY_H

#include <linux/types.h>

typedef enum {
	DISPLAY_HIST_POLICY_MIN = 0,
	DISPLAY_HIST_POLICY_RGB,
	DISPLAY_HIST_POLICY_MAX,
} dubai_hist_policy_t;

#ifdef CONFIG_HUAWEI_DUBAI_RGB_STATS
struct display_hist_rgb {
	uint64_t red;
	uint64_t green;
	uint64_t blue;
};
typedef struct display_hist_rgb hist_data;
#else
struct display_hist_rgb {};
typedef struct display_hist_rgb hist_data;
#endif

struct dubai_display_hist_ops {
	dubai_hist_policy_t (*get_policy)(void);
	size_t (*get_len)(void);
	int (*enable)(bool enable);
	int (*update)(void);
	int (*get)(void *data, int len);
};

extern void dubai_notify_display_state(bool state);

#endif // DUBAI_DISPLAY_H
