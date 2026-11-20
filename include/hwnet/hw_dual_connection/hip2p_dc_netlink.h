

#ifndef __HIP2P_DC_NETLINK_H__
#define __HIP2P_DC_NETLINK_H__

#include <uapi/linux/if.h>

struct hdc_msg {
	char name_master[IFNAMSIZ];
	char name_slave[IFNAMSIZ];
	bool enable;
};
#endif
