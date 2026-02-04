#include <linux/writeback.h>
#include <linux/backing-dev.h>
#include <linux/blk-cgroup.h>
#include <linux/dirty_pages_control.h>

#ifdef CONFIG_DIRTY_PAGES_CONTROL
bool is_bg_controlled(void)
{
	struct blkcg *blkcg = task_blkcg(current);
	if (blkcg && blkcg != &blkcg_root &&
		(blkcg->type == BLK_THROTL_BG ||
			blkcg->type == BLK_THROTL_KBG)) {
		return true;
	}
	return false;
}

bool is_wbt_controlled(void)
{
	struct blkcg *blkcg = task_blkcg(current);
	if (current->flags & PF_KTHREAD)
		return true;
	if (blkcg && blkcg != &blkcg_root &&
		(blkcg->type == BLK_THROTL_BG ||
			blkcg->type == BLK_THROTL_KBG))
		return true;
	return false;
}

/* copy from __filemap_fdatawrite_range in mm/filemap.c */
int fdatawrite_dirty_pages_size(struct address_space *mapping,
                                                long nr_to_write)
{
	int ret;
	struct writeback_control wbc = {
	        .sync_mode = WB_SYNC_NONE,
	        .nr_to_write = nr_to_write,
	        .range_start = 0,
	        .range_end = LLONG_MAX,
	        .reason = WB_REASON_MAX,
	};

	if (!mapping_cap_writeback_dirty(mapping))
	    return 0;
	wbc_attach_fdatawrite_inode(&wbc, mapping->host);
	ret = do_writepages(mapping, &wbc);
	wbc_detach_inode(&wbc);
	return ret;

}

/* use PER_WRITE_SYNC_PAGES to sync */
/* when over dirty pages up to MAX_FILE_DIRTY_PAGES */
void fdatacontrol_dirty_pages(struct address_space *mapping, int dirty_pages)
{
	struct blkcg *blkcg;
	int overpages;
	if (!dirty_pages_controlled)
		return;
	if (!is_bg_controlled())
		return;
	overpages = dirty_pages - MAX_FILE_DIRTY_PAGES;
	if (likely(overpages <= 0))
		return;
	else if (overpages <= PER_WRITE_SYNC_PAGES)
		overpages = PER_WRITE_SYNC_PAGES;
	else if (overpages <= MAX_FILE_DIRTY_PAGES)
		overpages = ((overpages >> BIT_NUM_OF_PER_WRITE_SYNC_PAGES)
				 + 1) << BIT_NUM_OF_PER_WRITE_SYNC_PAGES;
	else
		overpages = MAX_FILE_DIRTY_PAGES;

	fdatawrite_dirty_pages_size(mapping, overpages);
}
#endif
