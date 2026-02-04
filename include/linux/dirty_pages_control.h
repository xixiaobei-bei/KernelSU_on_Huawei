#ifndef DIRTY_PAGES_CONTROL_H
#define DIRTY_PAGES_CONTROL_H

#include <linux/fs.h>

#ifdef CONFIG_DIRTY_PAGES_CONTROL

extern bool dirty_pages_controlled;

#define MAX_FILE_DIRTY_PAGES 256
#define PER_WRITE_SYNC_PAGES 32
#define BIT_NUM_OF_PER_WRITE_SYNC_PAGES 5

bool is_wbt_controlled(void);
bool is_bg_controlled(void);
int fdatawrite_dirty_pages_size(struct address_space *mapping,
						long nr_to_write);
void fdatacontrol_dirty_pages(struct address_space *mapping, int dirty_pages);

#endif

#endif

