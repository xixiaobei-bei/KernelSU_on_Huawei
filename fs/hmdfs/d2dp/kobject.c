// SPDX-License-Identifier: GPL-2.0
/*
 * D2DP kobject interface implementation
 */

#define pr_fmt(fmt) "[D2DP]: " fmt

#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <uapi/linux/in.h>

#include "d2d.h"
#include "kobject.h"
#include "protocol.h"

static struct kset *d2dp_kset;

#define to_d2dp_obj(x) container_of(x, struct d2d_protocol, kobj)

struct d2dp_attribute {
	struct attribute attr;
	ssize_t (*show)(struct d2d_protocol *proto, struct d2dp_attribute *attr,
			char *buf);
	ssize_t (*store)(struct d2d_protocol *proto,
			 struct d2dp_attribute *attr, const char *buf,
			 size_t count);
};
#define to_d2dp_attr(x) container_of(x, struct d2dp_attribute, attr)

static ssize_t d2dp_attr_show(struct kobject *kobj,
			      struct attribute *attr,
			      char *buf)
{
	struct d2dp_attribute *attribute;
	struct d2d_protocol *proto;

	attribute = to_d2dp_attr(attr);
	proto = to_d2dp_obj(kobj);

	if (!attribute->show)
		return -EIO;

	return attribute->show(proto, attribute, buf);
}

static ssize_t d2dp_attr_store(struct kobject *kobj,
			       struct attribute *attr,
			       const char *buf, size_t len)
{
	struct d2dp_attribute *attribute;
	struct d2d_protocol *proto;

	attribute = to_d2dp_attr(attr);
	proto = to_d2dp_obj(kobj);

	if (!attribute->store)
		return -EIO;

	return attribute->store(proto, attribute, buf, len);
}

static const struct sysfs_ops d2dp_sysfs_ops = {
	.show  = d2dp_attr_show,
	.store = d2dp_attr_store,
};

static void d2dp_release(struct kobject *kobj)
{
	d2d_protocol_destroy(to_d2dp_obj(kobj));
}

static ssize_t packet_id_tx_show(struct d2d_protocol *proto,
				 struct d2dp_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%llu\n", proto->tx_packet_id);
}

static ssize_t packet_id_rx_show(struct d2d_protocol *proto,
				 struct d2dp_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%llu\n", proto->rx_packet_id);
}

static ssize_t stats_tx_bytes_show(struct d2d_protocol *proto,
				   struct d2dp_attribute *attr, char *buf)
{
	u64 tx_bytes = atomic64_read(&proto->stats.tx_bytes);

	return snprintf(buf, PAGE_SIZE, "%llu\n", tx_bytes);
}

static ssize_t stats_tx_packets_show(struct d2d_protocol *proto,
				     struct d2dp_attribute *attr, char *buf)
{
	u64 tx_packets = atomic64_read(&proto->stats.tx_packets);

	return snprintf(buf, PAGE_SIZE, "%llu\n", tx_packets);
}

static ssize_t stats_rx_bytes_show(struct d2d_protocol *proto,
				   struct d2dp_attribute *attr, char *buf)
{
	u64 rx_bytes = atomic64_read(&proto->stats.rx_bytes);

	return snprintf(buf, PAGE_SIZE, "%llu\n", rx_bytes);
}

static ssize_t stats_rx_packets_show(struct d2d_protocol *proto,
				     struct d2dp_attribute *attr, char *buf)
{
	u64 rx_packets = atomic64_read(&proto->stats.rx_packets);

	return snprintf(buf, PAGE_SIZE, "%llu\n", rx_packets);
}

static ssize_t stats_tx_buffer_show(struct d2d_protocol *proto,
				    struct d2dp_attribute *attr, char *buf)
{
	unsigned int tx_buf_state = atomic_read(&proto->stats.tx_buf_bytes);

	return snprintf(buf, PAGE_SIZE, "%u\n", tx_buf_state);
}

static ssize_t stats_rx_buffer_show(struct d2d_protocol *proto,
				    struct d2dp_attribute *attr, char *buf)
{
	unsigned int rx_buf_state = atomic_read(&proto->stats.rx_buf_bytes);

	return snprintf(buf, PAGE_SIZE, "%u\n", rx_buf_state);
}

static ssize_t stats_tx_wait_show(struct d2d_protocol *proto,
				  struct d2dp_attribute *attr, char *buf)
{
	unsigned int tx_wait = atomic_read(&proto->stats.tx_wait_switches);

	return snprintf(buf, PAGE_SIZE, "%u\n", tx_wait);
}

static ssize_t stats_tx_resend_show(struct d2d_protocol *proto,
				    struct d2dp_attribute *attr, char *buf)
{
	unsigned int tx_resend = atomic_read(&proto->stats.tx_resend_bytes);

	return snprintf(buf, PAGE_SIZE, "%u\n", tx_resend);
}

static ssize_t stats_rx_sacks_show(struct d2d_protocol *proto,
				   struct d2dp_attribute *attr, char *buf)
{
	unsigned int rx_sacks = atomic_read(&proto->stats.rx_sacks_arrived);

	return snprintf(buf, PAGE_SIZE, "%u\n", rx_sacks);
}

static ssize_t stats_rx_dups_show(struct d2d_protocol *proto,
				  struct d2dp_attribute *attr, char *buf)
{
	unsigned int rx_dups = atomic_read(&proto->stats.rx_dups_arrived);

	return snprintf(buf, PAGE_SIZE, "%u\n", rx_dups);
}

static ssize_t stats_rx_overflow_show(struct d2d_protocol *proto,
				      struct d2dp_attribute *attr, char *buf)
{
	unsigned int overflow = atomic_read(&proto->stats.rx_overflow_arrived);

	return snprintf(buf, PAGE_SIZE, "%u\n", overflow);
}

static ssize_t stats_rto_show(struct d2d_protocol *proto,
			      struct d2dp_attribute *attr, char *buf)
{
	unsigned int rtos = atomic_read(&proto->stats.rto_timer_fired);

	return snprintf(buf, PAGE_SIZE, "%u\n", rtos);
}

static ssize_t err_badhdr_show(struct d2d_protocol *proto,
			       struct d2dp_attribute *attr, char *buf)
{
	unsigned int badhdrs = atomic_read(&proto->stats.rx_drops_badhdr);

	return snprintf(buf, PAGE_SIZE, "%u\n", badhdrs);
}

static ssize_t err_decrypt_show(struct d2d_protocol *proto,
				struct d2dp_attribute *attr, char *buf)
{
	unsigned int bad_decrypt = atomic_read(&proto->stats.rx_drops_decrypt);

	return snprintf(buf, PAGE_SIZE, "%u\n", bad_decrypt);
}

static ssize_t err_pktid_show(struct d2d_protocol *proto,
			      struct d2dp_attribute *attr, char *buf)
{
	unsigned int bad_pktid = atomic_read(&proto->stats.rx_drops_pktid);

	return snprintf(buf, PAGE_SIZE, "%u\n", bad_pktid);
}

static ssize_t err_trunc_show(struct d2d_protocol *proto,
			      struct d2dp_attribute *attr, char *buf)
{
	unsigned int bad_trunc = atomic_read(&proto->stats.rx_drops_trunc);

	return snprintf(buf, PAGE_SIZE, "%u\n", bad_trunc);
}

static ssize_t err_empty_show(struct d2d_protocol *proto,
			      struct d2dp_attribute *attr, char *buf)
{
	unsigned int bad_empty = atomic_read(&proto->stats.rx_drops_empty);

	return snprintf(buf, PAGE_SIZE, "%u\n", bad_empty);
}

static struct d2dp_attribute packet_id_tx_attr = __ATTR_RO(packet_id_tx);
static struct d2dp_attribute packet_id_rx_attr = __ATTR_RO(packet_id_rx);
static struct d2dp_attribute stats_tx_bytes_attr = __ATTR_RO(stats_tx_bytes);
static struct d2dp_attribute stats_tx_pkts_attr = __ATTR_RO(stats_tx_packets);
static struct d2dp_attribute stats_rx_bytes_attr = __ATTR_RO(stats_rx_bytes);
static struct d2dp_attribute stats_rx_pkts_attr = __ATTR_RO(stats_rx_packets);
static struct d2dp_attribute stats_tx_buffer_attr = __ATTR_RO(stats_tx_buffer);
static struct d2dp_attribute stats_rx_buffer_attr = __ATTR_RO(stats_rx_buffer);
static struct d2dp_attribute stats_tx_wait_attr = __ATTR_RO(stats_tx_wait);
static struct d2dp_attribute stats_tx_resend_attr = __ATTR_RO(stats_tx_resend);
static struct d2dp_attribute stats_rx_sacks_attr = __ATTR_RO(stats_rx_sacks);
static struct d2dp_attribute stats_rx_dups_attr = __ATTR_RO(stats_rx_dups);
static struct d2dp_attribute stats_rx_oflow_attr = __ATTR_RO(stats_rx_overflow);
static struct d2dp_attribute stats_rto_attr = __ATTR_RO(stats_rto);
static struct d2dp_attribute err_badhdr_attr = __ATTR_RO(err_badhdr);
static struct d2dp_attribute err_decrypt_attr = __ATTR_RO(err_decrypt);
static struct d2dp_attribute err_pktid_attr = __ATTR_RO(err_pktid);
static struct d2dp_attribute err_trunc_attr = __ATTR_RO(err_trunc);
static struct d2dp_attribute err_empty_attr = __ATTR_RO(err_empty);

static struct attribute *d2dp_default_attrs[] = {
	&packet_id_tx_attr.attr,
	&packet_id_rx_attr.attr,
	&stats_tx_bytes_attr.attr,
	&stats_tx_pkts_attr.attr,
	&stats_rx_bytes_attr.attr,
	&stats_rx_pkts_attr.attr,
	&stats_tx_buffer_attr.attr,
	&stats_rx_buffer_attr.attr,
	&stats_tx_wait_attr.attr,
	&stats_tx_resend_attr.attr,
	&stats_rx_sacks_attr.attr,
	&stats_rx_dups_attr.attr,
	&stats_rx_oflow_attr.attr,
	&stats_rto_attr.attr,
	&err_badhdr_attr.attr,
	&err_decrypt_attr.attr,
	&err_pktid_attr.attr,
	&err_trunc_attr.attr,
	&err_empty_attr.attr,
	NULL,
};

static struct kobj_type d2dp_ktype = {
	.sysfs_ops     = &d2dp_sysfs_ops,
	.release       = d2dp_release,
	.default_attrs = d2dp_default_attrs,
};

static u64 d2dp_kobj_id = 0;
static DEFINE_MUTEX(d2dp_kobj_mtx);

int d2dp_register_kobj(struct d2d_protocol *proto)
{
	int ret = 0;

	mutex_lock(&d2dp_kobj_mtx);

	proto->kobj.kset = d2dp_kset;
	ret = kobject_init_and_add(&proto->kobj, &d2dp_ktype, NULL,
				   "%llu", d2dp_kobj_id);
	if (ret) {
		pr_err("kobj init failed: %d\n", ret);
		goto unlock;
	}

	d2dp_kobj_id++;
	kobject_uevent(&proto->kobj, KOBJ_ADD);

unlock:
	mutex_unlock(&d2dp_kobj_mtx);
	return ret;
}

int d2dp_kobject_init(void)
{
	d2dp_kset = kset_create_and_add("d2dp", NULL, kernel_kobj);
	if (!d2dp_kset)
		return -ENOMEM;

	return 0;
}

void d2dp_kobject_deinit(void)
{
	kset_unregister(d2dp_kset);
	d2dp_kset = NULL;
}
