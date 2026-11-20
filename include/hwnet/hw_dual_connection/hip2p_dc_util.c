

#include "hip2p_dc_util.h"
#include <linux/list.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>
#include "hip2p_dc_interface.h"

static struct dc_mngr g_dcm __read_mostly;

struct dc_mngr *get_dcm(void)
{
	return &g_dcm;
}

static struct dc_instance *dc_create(const struct net_device *master_dev,
	const struct net_device *slave_dev)
{
	struct dc_instance *dc = &g_dcm.dc;
	memset(dc, 0, sizeof(*dc));
	dc_mlogd("set dc");
	dc->dev[TYPE_MASTER] = (struct net_device *)master_dev;
	dc->dev[TYPE_SLAVE] = (struct net_device *)slave_dev;
	atomic_set(&g_dcm.dc_enable, DC_ENABLE);
	return dc;
}

static void dc_rcu_free(struct rcu_head *head)
{
	struct dc_instance *dc = container_of(head, struct dc_instance, rcu);

	dev_put(dc->dev[TYPE_MASTER]);
	dev_put(dc->dev[TYPE_SLAVE]);
	memset(dc, 0, sizeof(*dc));
}

static void dc_delete(struct dc_instance *dc)
{
	atomic_set(&g_dcm.dc_enable, DC_DISABLE);
	hip2p_proc_delete_dc();
	call_rcu(&dc->rcu, dc_rcu_free);
}

/*
 * Description: when we get netlink msg to start dc connect,
 * then creat dc instance.
 */
int dc_insert(const char *master_dev, const char *slave_dev)
{
	struct dc_instance *dc = NULL;
	struct net_device *master = NULL;
	struct net_device *slave = NULL;
	int flags;

	if ((master_dev == NULL) || (slave_dev == NULL)) {
		dc_mloge("master/slave dev is NULL");
		return HIP2P_FAIL;
	}
	if (atomic_read(&g_dcm.dc_enable) == DC_ENABLE) {
		dc_mloge("dc is already enabled");
		return HIP2P_FAIL;
	}

	master = dev_get_by_name(&init_net, master_dev);
	if (master == NULL) {
		dc_mloge("get dev %s failed", master_dev);
		return -EFAULT;
	}
	flags = master->flags & IFF_UP;
	if (flags == 0) {
		dc_mloge("master dev down");
		dev_put(master);
		return HIP2P_FAIL;
	}
	slave = dev_get_by_name(&init_net, slave_dev);
	if (slave == NULL) {
		dc_mloge("get dev %s failed", slave_dev);
		dev_put(master);
		return -EFAULT;
	}
	flags = slave->flags & IFF_UP;
	if (flags == 0) {
		dc_mloge("slave dev down");
		dev_put(master);
		dev_put(slave);
		return HIP2P_FAIL;
	}

	/* Set DC */
	spin_lock_bh(&g_dcm.dc_lock);
	dc = dc_create(master, slave);
	if (dc == NULL) {
		dc_mloge("add dc failed");
		dev_put(master);
		dev_put(slave);
		spin_unlock_bh(&g_dcm.dc_lock);
		return HIP2P_FAIL;
	}
	spin_unlock_bh(&g_dcm.dc_lock);
	hip2p_proc_create_dc(dc);
	return HIP2P_SUCC;
}

void dc_clear(void)
{
	struct dc_instance *dc = NULL;
	struct dc_mngr *dcm = get_dcm();

	dc_mlogd("clean dc");
	if (atomic_read(&dcm->dc_enable) == DC_DISABLE) {
		dc_mloge("dc reaches 0");
		return;
	}
	dc = &dcm->dc;
	spin_lock_bh(&dcm->dc_lock);
	dc_delete(dc);
	spin_unlock_bh(&dcm->dc_lock);
}

void dc_clear_by_dev(const struct net_device *dev)
{
	struct dc_instance *dc = NULL;
	struct dc_mngr *dcm = NULL;
	dc_mlogd("clean dc");
	if (atomic_read(&g_dcm.dc_enable) == DC_DISABLE) {
		dc_mloge("dc reaches 0");
		return;
	}
	dcm = get_dcm();
	dc = &dcm->dc;
	spin_lock_bh(&g_dcm.dc_lock);
	if ((dc->dev[TYPE_MASTER] == dev) || (dc->dev[TYPE_SLAVE] == dev))
		dc_delete(dc);
	spin_unlock_bh(&g_dcm.dc_lock);
}

struct net_device *get_pair_dev(struct dc_instance *dc,
	const struct net_device *dev)
{
	if ((dc == NULL) || (dev == NULL)) {
		dc_mloge("dc/dev is NULL");
		return NULL;
	}
	if (dc->dev[TYPE_MASTER] == dev)
		return dc->dev[TYPE_SLAVE];

	if (dc->dev[TYPE_SLAVE] == dev)
		return dc->dev[TYPE_MASTER];

	return NULL;
}

static void clear_bitmap(struct bitmap *bitmap)
{
	memset(bitmap->map, 0, sizeof(bitmap->map));
	bitmap->threshold = 0;
}


static int update_bitmap(struct bitmap *bitmap, const long nr)
{
	long line;
	long bit;

	line = nr / BITS_PER_LONG;
	bit = nr % BITS_PER_LONG;
	if ((line < 0) || (bit < 0)) {
		dc_mloge("line or bit is invalid");
		return HIP2P_FAIL;
	}

	if (test_bit(bit, bitmap->map + line)) {
		return HIP2P_SUCC;
	} else {
		set_bit(bit, bitmap->map + line);
		bitmap->threshold++;
	}
	return HIP2P_FAIL;
}

/*
 * Update seq bitmap status, statistics and clear the proper bitmap.
 * Result HIP2P_SUCC means the current sequence/packet is the LATER copy.
 */
int check_packet_status(struct dc_instance *dc,
	const unsigned short sequence)
{
	unsigned short block;
	unsigned short offset;
	unsigned short clear_block;
	struct dup_packet *status = NULL;
	if (dc == NULL) {
		dc_mloge("dc is NULL");
		return HIP2P_FAIL;
	}

	status = &dc->status;
	block = sequence / DC_BITS_PER_BLOCK;
	offset = sequence % DC_BITS_PER_BLOCK;
	if (block >= DC_BITMAP_BLOCK_NUM || offset >= DC_BITS_PER_BLOCK) {
		dc_mloge("block or offset is overflow, block: %d, offset: %d",
			block, offset);
		return HIP2P_FAIL;
	}

	if (update_bitmap(status->block + block, offset) == HIP2P_SUCC)
		return HIP2P_SUCC;

	if (status->block[block].threshold == DC_BITMAP_THRESHOLD) {
		clear_block = (block + DC_FILLING_BLOCK_NUM) % DC_BITMAP_BLOCK_NUM;
		clear_bitmap(status->block + clear_block);
	}

	return HIP2P_FAIL;
}

int dcm_init(void)
{
	memset(&g_dcm, 0, sizeof(struct dc_mngr));
	spin_lock_init(&g_dcm.dc_lock);
	return HIP2P_SUCC;
}

void dcm_exit(void)
{
	spin_lock_bh(&g_dcm.dc_lock);
	if (atomic_read(&g_dcm.dc_enable) == DC_ENABLE) {
		dc_mlogd("dc reaches max");
		dev_put(g_dcm.dc.dev[TYPE_MASTER]);
		dev_put(g_dcm.dc.dev[TYPE_SLAVE]);
		atomic_set(&g_dcm.dc_enable, DC_DISABLE);
	}
	spin_unlock_bh(&g_dcm.dc_lock);
	memset(&g_dcm, 0, sizeof(struct dc_mngr));
}
