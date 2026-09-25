/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: HiGame2.0's hip2p module. Interfaces for applications.
 * Author: liqi
 * Create: 2019-05-20
 */

#ifndef __HIP2P_DC_NETLINK_H__
#define __HIP2P_DC_NETLINK_H__

#include <uapi/linux/if.h>

struct hdc_msg {
	char name_master[IFNAMSIZ];
	char name_slave[IFNAMSIZ];
	bool enable;
};
#endif
