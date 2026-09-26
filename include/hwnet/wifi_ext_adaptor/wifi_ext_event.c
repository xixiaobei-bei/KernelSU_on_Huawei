

#include "wifi_ext_event.h"
#include "wifi_ext_mark.h"
#include <linux/etherdevice.h>
#include <linux/if_arp.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/wireless.h>
#include <linux/export.h>
#include <net/iw_handler.h>
#include <net/cfg80211.h>
#include <net/rtnetlink.h>
#include <../net/wireless/nl80211.h>
#include <../net/wireless/reg.h>
#include <../net/wireless/rdev-ops.h>
#include <chipset_common/hwnet/hw_event.h>
#include <linux/netlink.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <net/sock.h>
#include <uapi/linux/netlink.h> /* add for netlink */
#include "securec.h"

typedef struct {
	struct nlmsghdr hdr;
	u8* data;
} hw_msg;

typedef struct {
	u16 type;
	u16 tx_rate;
	u16 vi_mpdu_num;
	u32 tx_failed;
} link_info;

typedef struct {
	u16 type;
	u16 seq_num;
} seq_drop_info;

/* hid2d netlink message format */
typedef struct {
	struct nlmsghdr hdr;
	char data[1]; // length of data is 1
} hid2d_msg2knl;

typedef struct {
	uint32_t ip_addr;
	uint16_t port;
	uint16_t prefix_len;
} hid2d_info;

#define HID2D_VIDEO_MARK 0x5b

#define ENABLE 1
#define DISABLE 0
#define FLASHLIGHT_PID 101
#define WIFI_EXT_MODULE_PID 100

static struct sock *g_hw_nlfd;

void send_netlink_msg(const u8 *buf, size_t len, int pid)
{
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;
	u8* packet = NULL;
	int size;
	if (!buf || !g_hw_nlfd) {
		return;
	}

	size = sizeof(struct nlmsghdr) + len + 1;
	skb = nlmsg_new(size, GFP_ATOMIC);
	if (!skb) {
		return;
	}
	nlh = nlmsg_put(skb, 0, 0, 0, size, 0);
	if (!nlh) {
		nlmsg_free(skb);
		return;
	}
	packet = nlmsg_data(nlh);
	memset_s(packet, len, 0, len);
	memcpy_s(packet, len, buf, len);

	netlink_unicast(g_hw_nlfd, skb, pid, MSG_DONTWAIT);
}
EXPORT_SYMBOL(send_netlink_msg); //lint !e580

/* receive cmd for DPI netlink message */
static void kernel_ntl_receive(struct sk_buff *__skb)
{
	struct nlmsghdr *nlh = NULL;
	struct sk_buff *skb = NULL;
	hid2d_msg2knl *hid2d_msg = NULL;
	hid2d_info *hinfo = NULL;
	if (__skb == NULL)
		return;
	skb = skb_get(__skb);
	if (skb != NULL && (skb->len >= NLMSG_HDRLEN)) {
		nlh = nlmsg_hdr(skb);
		if ((nlh->nlmsg_len >= sizeof(struct nlmsghdr)) &&
				(skb->len >= nlh->nlmsg_len)) {
			if (nlh->nlmsg_type == NETLINK_HID2D_TYPE) {
				hid2d_msg = (hid2d_msg2knl *)nlh;
				hinfo = (hid2d_info *)&(hid2d_msg->data[0]);
				g_hid2d_ip = hinfo->ip_addr;

				// Big endian and Little endian exchange
				g_hid2d_port = htons(hinfo->port);
				g_hid2d_prefix_len = hinfo->prefix_len;
				pr_info("kernel_ntl_receive, g_hid2d_ip = %d, g_hid2d_port = %d",
					g_hid2d_ip, g_hid2d_port);
			}
		}
	}
}

/* Initialize netlink socket for transmit and receive */
static int __init netlink_init(void)
{
	struct netlink_kernel_cfg hwcfg = {
		.input = kernel_ntl_receive,
	};
	g_hw_nlfd = netlink_kernel_create(&init_net, NETLINK_WIFI_EXT, &hwcfg);
	if (g_hw_nlfd == NULL)
		pr_info("netlink_init failed NETLINK_HW_DPI\n");

	return 0;
}

static void __exit netlink_exit(void)
{
	netlink_kernel_release(g_hw_nlfd);
	g_hw_nlfd = NULL;
}
void cfg80211_drv_hid2d_acs_report(struct net_device *dev, gfp_t gfp, const u8 *buf, size_t len)
{
	send_netlink_msg(buf, len, WIFI_EXT_MODULE_PID);
}
EXPORT_SYMBOL(cfg80211_drv_hid2d_acs_report); //lint !e580

module_init(netlink_init);
module_exit(netlink_exit);

MODULE_LICENSE("Dual BSD");
MODULE_AUTHOR("y00445093");
MODULE_DESCRIPTION("HW WIFI EXT EVENT");
MODULE_VERSION("1.0.1");
MODULE_ALIAS("HW WIFIX EXT 01");
