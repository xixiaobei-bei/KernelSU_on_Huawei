

#ifndef __HW_EVENT_H__
#define __HW_EVENT_H__

#include <linux/etherdevice.h>

#ifdef CONFIG_HW_ABS
void cfg80211_drv_ant_grab(struct net_device *dev, gfp_t gfp);
#endif

#endif /* __HW_EVENT_H__ */
