

#ifndef __HIP2P_DC_INTERFACE_H__
#define __HIP2P_DC_INTERFACE_H__

#include <linux/proc_fs.h>
#include <uapi/linux/netlink.h>
#include "hip2p_dc_netlink.h"
#include "hip2p_dc_util.h"

struct netlink_msg2knl {
	struct nlmsghdr hdr;
	char data[1];
};

void hip2p_proc_create(void);
void hip2p_proc_remove(void);
void hip2p_proc_create_dc(const struct dc_instance *dc);
void hip2p_proc_delete_dc(void);
int netlink_init(void);
#endif
