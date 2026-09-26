#ifndef __BSP_NET_HELPER__
#define __BSP_NET_HELPER__

#include <linux/if_ether.h>

struct ip_limit_cfg {
    unsigned long local_jiffies;
    unsigned int limit;
    unsigned int rx_cnt;
    unsigned int period;
    unsigned int drop_cnt;
    unsigned int in_cnt;
    unsigned int clear_cnt;
    unsigned int check_cnt;
    unsigned int helper_ip_cnt;
    unsigned int helper_udp_cnt;
    unsigned int helper_icmp_cnt;
    unsigned int helper_not_ip;
    unsigned int helper_not_care;
	unsigned int helper_not_ipv4;
};

void bsp_ip_limit_init(struct ip_limit_cfg* cfg, unsigned int limit);

int bsp_ip_limit(struct ip_limit_cfg* cfg, struct iphdr *ip);

void bsp_ip_limit_show(struct ip_limit_cfg* cfg);

#endif