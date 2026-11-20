/*
 * Copyright 2013-2014, Huawei
 *
 * Add for Wi-Fi throughput monitor and CPU DDR frequency control
 */

#ifndef _HW_WIFI_FREQ_CTRL_H
#define _HW_WIFI_FREQ_CTRL_H

extern void hw_wifi_freq_ctrl_init(void);
extern void hw_wifi_freq_ctrl_destroy(void);
extern int hw_wifi_set_cpu_performance(int value);

#endif  /*_HW_WIFI_FREQ_CTRL_H*/
