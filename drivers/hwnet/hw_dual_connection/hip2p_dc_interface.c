

#include "hip2p_dc_interface.h"
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/if_ether.h>
#include <linux/seq_file.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include "hip2p_dc_util.h"

#define HIP2P_DIR "hip2p"
#define DC_DIR "dc"
#define INFO_DIR "info"

static struct proc_dir_entry *g_hip2p_proc_dir = NULL;

static int proc_info_get(struct seq_file *file, void *data)
{
	struct dc_instance *dc = file->private;

	if (dc == NULL) {
		dc_mloge("dc null");
		return -EFAULT;
	}

	if ((dc->dev[TYPE_MASTER] == NULL) || (dc->dev[TYPE_SLAVE] == NULL)) {
		dc_mloge("dev null");
		return -EFAULT;
	}

	seq_printf(file, "%16s    %16s    %10s    %10s    %10s\n",
		"master", "slave", "send", "receive", "drop");
	seq_printf(file, "%16s    %16s    %10u    %10u    %10u\n",
		dc->dev[TYPE_MASTER]->name,
		dc->dev[TYPE_SLAVE]->name,
		dc->sequence,
		dc->seq.former_recv_seq,
		dc->seq.later_recv_seq);
	return HIP2P_OK;
}

static int proc_info_open(struct inode *inode, struct file *file)
{
	if ((inode == NULL) || (file == NULL)) {
		dc_mloge("inode/file null");
		return -EFAULT;
	}
	return single_open(file, proc_info_get, PDE_DATA(inode));
}

static struct file_operations g_hip2p_info_ops = {
	.owner = THIS_MODULE,
	.open = proc_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

void hip2p_proc_create(void)
{
	g_hip2p_proc_dir = proc_mkdir(HIP2P_DIR, NULL);
	if (g_hip2p_proc_dir == NULL) {
		dc_mloge("mkdir failed");
		return;
	}
}

void hip2p_proc_remove(void)
{
	proc_remove(g_hip2p_proc_dir);
}

void hip2p_proc_create_dc(const struct dc_instance *dc)
{
	struct proc_dir_entry *proc_dir = NULL;

	if (dc == NULL) {
		dc_mloge("dc is NULL");
		return;
	}

	proc_dir = proc_mkdir(DC_DIR, g_hip2p_proc_dir);
	if (proc_dir == NULL) {
		dc_mloge("mkdir failed");
		return;
	}
	proc_create_data(INFO_DIR, S_IRUGO, proc_dir, &g_hip2p_info_ops,
		(void *)dc);
}

void hip2p_proc_delete_dc(void)
{
	remove_proc_entry(DC_DIR, g_hip2p_proc_dir);
}

static void netlink_msg_proc(const char *data)
{
	struct hdc_msg *hdc = (struct hdc_msg *)data;
	int ret;

	if (hdc == NULL) {
		dc_mloge("null msg");
		return;
	}

	dc_mlogd("%s %s %d\n", hdc->name_master, hdc->name_slave, hdc->enable);
	if (!hdc->enable) {
		dc_clear();
		return;
	}
	ret = dc_insert(hdc->name_master, hdc->name_slave);
	if (ret != HIP2P_SUCC)
		dc_mloge("insert dc failed");
}

static void kernel_netlink_receive(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = NULL;
	struct netlink_msg2knl *msg = NULL;

	if ((skb != NULL) && (skb->len >= NLMSG_HDRLEN)) {
		nlh = nlmsg_hdr(skb);
		if ((nlh->nlmsg_len >= sizeof(struct nlmsghdr)) &&
			(skb->len >= nlh->nlmsg_len)) {
			msg = (struct netlink_msg2knl *)nlh;
			netlink_msg_proc((char *)(msg->data));
		}
	}
}

int netlink_init(void)
{
	struct sock *sk = NULL;
	int ret = HIP2P_SUCC;

	struct netlink_kernel_cfg netlink_ops = {
		.input = kernel_netlink_receive,
	};

	sk = netlink_kernel_create(&init_net, NETLINK_HW_DC, &netlink_ops);
	if (sk == NULL) {
		dc_mloge("create netlink failed");
		ret = HIP2P_FAIL;
	}
	return ret;
}
