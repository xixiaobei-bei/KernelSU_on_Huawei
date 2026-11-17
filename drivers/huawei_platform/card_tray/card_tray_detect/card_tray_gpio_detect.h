/*
 * Copyright (c) Huawei Technologies Co., Ltd. 1998-2019. All rights reserved.
 *
 * File name: card_tray_gpio_detect.h
 * Description: This program detect the card tray gpio status
 * Author: dengjinqiao@huawei.com
 * Version: 1.0
 * Date:  2019/04/27
 *
 */
#ifndef _CARD_TRAY_GPIO_DETECT_
#define _CARD_TRAY_GPIO_DETECT_
#include <linux/device.h>

struct card_tray_info {
	struct device *dev;
};

#endif
