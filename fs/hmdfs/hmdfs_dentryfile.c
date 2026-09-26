/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hmdfs_dentryfile.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: lousong@huawei.com
 * Create: 2020-04-15
 *
 */

#include "hmdfs_dentryfile.h"

#include <linux/ctype.h>
#include <linux/file.h>
#include <linux/mount.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/xattr.h>
#include <linux/err.h>

#include "authority/authentication.h"
#include "comm/transport.h"
#include "hmdfs_client.h"
#include "hmdfs_device_view.h"

/* Hashing code copied from f2fs */
#define HMDFS_HASH_COL_BIT ((0x1ULL) << 63)
#define DELTA		   0x9E3779B9

static bool is_dot_dotdot(const unsigned char *name, __u32 len)
{
	if (len == 1 && name[0] == '.')
		return true;

	if (len == 2 && name[0] == '.' && name[1] == '.')
		return true;

	return false;
}

static void str2hashbuf(const unsigned char *msg, size_t len, unsigned int *buf,
			int num, bool case_sense)
{
	unsigned int pad, val;
	int i;
	unsigned char c;

	pad = (__u32)len | ((__u32)len << 8);
	pad |= pad << 16;

	val = pad;
	if (len > (size_t)num * 4)
		len = (size_t)num * 4;
	for (i = 0; i < len; i++) {
		if ((i % 4) == 0)
			val = pad;
		c = msg[i];
		if (!case_sense)
			c = tolower(c);
		val = c + (val << 8);
		if ((i % 4) == 3) {
			*buf++ = val;
			val = pad;
			num--;
		}
	}
	if (--num >= 0)
		*buf++ = val;
	while (--num >= 0)
		*buf++ = pad;
}

static void tea_transform(unsigned int buf[4], unsigned int const in[])
{
	__u32 sum = 0;
	__u32 b0 = buf[0], b1 = buf[1];
	__u32 a = in[0], b = in[1], c = in[2], d = in[3];
	int n = 16;

	do {
		sum += DELTA;
		b0 += ((b1 << 4) + a) ^ (b1 + sum) ^ ((b1 >> 5) + b);
		b1 += ((b0 << 4) + c) ^ (b0 + sum) ^ ((b0 >> 5) + d);
	} while (--n);

	buf[0] += b0;
	buf[1] += b1;
}

static __u32 hmdfs_dentry_hash(const struct qstr *qstr, bool case_sense)
{
	__u32 hash;
	__u32 hmdfs_hash;
	const unsigned char *p = qstr->name;
	__u32 len = qstr->len;
	__u32 in[8], buf[4];

	if (is_dot_dotdot(p, len))
		return 0;

	/* Initialize the default seed for the hash checksum functions */
	buf[0] = 0x67452301;
	buf[1] = 0xefcdab89;
	buf[2] = 0x98badcfe;
	buf[3] = 0x10325476;

	while (1) {
		str2hashbuf(p, len, in, 4, case_sense);
		tea_transform(buf, in);
		p += 16;
		if (len <= 16)
			break;
		len -= 16;
	}
	hash = buf[0];
	hmdfs_hash = hash & ~HMDFS_HASH_COL_BIT;
	return hmdfs_hash;
}

static atomic_t curr_ino = ATOMIC_INIT(INUNUMBER_START);
int get_inonumber(void)
{
	return atomic_inc_return(&curr_ino);
}

static int hmdfs_get_root_dentry_type(struct dentry *dentry, int *is_root)
{
	struct hmdfs_dentry_info *d_info = hmdfs_d(dentry);

	*is_root = 1;
	switch (d_info->dentry_type) {
	case HMDFS_LAYER_OTHER_LOCAL:
		*is_root = 0;
		/* fall-through */
	case HMDFS_LAYER_SECOND_LOCAL:
		return HMDFS_LAYER_SECOND_LOCAL;
	case HMDFS_LAYER_OTHER_REMOTE:
		*is_root = 0;
		/* fall-through */
	case HMDFS_LAYER_SECOND_REMOTE:
		return HMDFS_LAYER_SECOND_REMOTE;
	default:
		hmdfs_info("Unexpected dentry type %d", d_info->dentry_type);
		return -EINVAL;
	}
}

static int prepend(char **buffer, int *buflen, const char *str, int namelen)
{
	*buflen -= namelen;
	if (*buflen < 0)
		return -ENAMETOOLONG;
	*buffer -= namelen;
	memcpy(*buffer, str, namelen);
	return 0;
}

static int prepend_name(char **buffer, int *buflen, const struct qstr *name)
{
	const char *dname = name->name;
	u32 dlen = name->len;
	char *p = NULL;

	/* code migration from kernel */
	smp_read_barrier_depends();
	*buflen -= dlen + 1;
	if (*buflen < 0)
		return -ENAMETOOLONG;
	p = *buffer -= dlen + 1;
	*p++ = '/';
	while (dlen--) {
		char c = *dname++;

		if (!c)
			break;
		*p++ = c;
	}
	return 0;
}

static char *hmdfs_dentry_path_raw(struct dentry *d, char *buf, int buflen)
{
	struct dentry *dentry = NULL;
	char *end = NULL;
	char *retval = NULL;
	unsigned int len;
	unsigned int seq = 0;
	int root_flag = 0;
	int error = 0;
	struct hmdfs_dentry_info *di = hmdfs_d(d);
	int hmdfs_root_dentry_type = 0;

	di->time = jiffies;
	hmdfs_root_dentry_type = hmdfs_get_root_dentry_type(d, &root_flag);
	if (hmdfs_root_dentry_type < 0)
		return NULL;
	if (root_flag) {
		strcpy(buf, "/");
		return buf;
	}
	rcu_read_lock();
restart:
	dentry = d;
	di = hmdfs_d(dentry);
	di->time = jiffies;
	end = buf + buflen;
	len = buflen;
	prepend(&end, &len, "\0", 1);
	retval = end - 1;
	*retval = '/';
	read_seqbegin_or_lock(&rename_lock, &seq);
	while (di->dentry_type != hmdfs_root_dentry_type) {
		struct dentry *parent = dentry->d_parent;

		prefetch(parent);
		error = prepend_name(&end, &len, &dentry->d_name);
		if (error)
			break;
		retval = end;
		dentry = parent;
		di = hmdfs_d(dentry);
		di->time = jiffies;
	}
	if (!(seq & 1))
		rcu_read_unlock();
	if (need_seqretry(&rename_lock, seq)) {
		seq = 1;
		goto restart;
	}
	done_seqretry(&rename_lock, seq);
	if (error)
		goto Elong;
	return retval;
Elong:
	return ERR_PTR(-ENAMETOOLONG);
}

char *hmdfs_get_dentry_relative_path(struct dentry *dentry)
{
	char *final_buf = NULL;
	char *buf = NULL;
	char *p = NULL;

	buf = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!buf)
		return NULL;

	final_buf = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!final_buf) {
		kfree(buf);
		return NULL;
	}

	/* NULL dentry return root dir */
	if (!dentry) {
		strcpy(final_buf, "/");
		kfree(buf);
		return final_buf;
	}
	p = hmdfs_dentry_path_raw(dentry, buf, PATH_MAX);
	if (IS_ERR_OR_NULL(p)) {
		kfree(buf);
		kfree(final_buf);
		return NULL;
	}

	if (strlen(p) >= PATH_MAX) {
		kfree(buf);
		kfree(final_buf);
		return NULL;
	}
	strcpy(final_buf, p);
	kfree(buf);
	return final_buf;
}

char *hmdfs_get_dentry_absolute_path(const char *rootdir,
				     const char *relative_path)
{
	char *buf = 0;

	if (!rootdir || !relative_path)
		return NULL;
	if (strlen(rootdir) + strlen(relative_path) >= PATH_MAX)
		return NULL;

	buf = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!buf)
		return NULL;

	strcpy(buf, rootdir);
	strcat(buf, relative_path);
	return buf;
}

char *hmdfs_connect_path(const char *path, const char *name)
{
	char *buf = 0;

	if (!path || !name)
		return NULL;

	if (strlen(path) + strlen(name) + 1 >= PATH_MAX)
		return NULL;

	buf = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!buf)
		return NULL;

	strcpy(buf, path);
	strcat(buf, "/");
	strcat(buf, name);
	return buf;
}

int hmdfs_metainfo_read(struct hmdfs_sb_info *sbi, struct file *filp,
			void *buffer, int size, int bidx)
{
	loff_t pos = get_dentry_group_pos(bidx);

	return cache_file_read(sbi, filp, buffer, (size_t)size, &pos);
}

int hmdfs_metainfo_write(struct hmdfs_sb_info *sbi, struct file *filp,
			 const void *buffer, int size, int bidx)
{
	loff_t pos = get_dentry_group_pos(bidx);

	return cache_file_write(sbi, filp, buffer, (size_t)size, &pos);
}

/* for each level */
/* bucketseq start offset by 0,for example
 * level0  bucket0(0)
 * level1  bucket0(1) bucket1(2)
 * level2  bucket0(3) bucket1(4) bucket2(5) bucket3(6)
 * return bucket number.
 */
static __u32 get_bucketaddr(int level, int buckoffset)
{
	int all_level_bucketaddr = 0;
	__u32 curlevelmaxbucks;

	if (level >= MAX_BUCKET_LEVEL) {
		hmdfs_err("level = %d overflow", level);
		return all_level_bucketaddr;
	}
	curlevelmaxbucks = (1 << level);
	if (buckoffset >= curlevelmaxbucks) {
		hmdfs_err("buckoffset %d overflow, level %d has %d buckets max",
			  buckoffset, level, curlevelmaxbucks);
		return all_level_bucketaddr;
	}
	all_level_bucketaddr = curlevelmaxbucks + buckoffset - 1;

	return all_level_bucketaddr;
}

static __u32 get_bucket_by_level(int level)
{
	int buckets = 0;

	if (level >= MAX_BUCKET_LEVEL) {
		hmdfs_err("level = %d overflow", level);
		return buckets;
	}

	buckets = (1 << level);
	return buckets;
}

static __u32 get_overall_bucket(int level)
{
	int buckets = 0;

	if (level >= MAX_BUCKET_LEVEL) {
		hmdfs_err("level = %d overflow", level);
		return buckets;
	}
	buckets = (1 << (level + 1)) - 1;
	return buckets;
}

static inline loff_t get_dcache_file_size(int level)
{
	loff_t buckets = get_overall_bucket(level);

	return buckets * DENTRYGROUP_SIZE * BUCKET_BLOCKS + DENTRYGROUP_HEADER;
}

static char *get_relative_path(struct hmdfs_sb_info *sbi, char *from)
{
	char *relative;

	if (strncmp(from, sbi->local_src, strlen(sbi->local_src))) {
		hmdfs_warning("orig path do not start with local_src");
		return NULL;
	}
	relative = from + strlen(sbi->local_src);
	if (*relative == '/')
		relative++;
	return relative;
}

struct file *hmdfs_get_or_create_dents(struct hmdfs_sb_info *sbi, char *name)
{
	struct path root_path, path;
	struct file *filp = NULL;
	char *relative;
	int err;

	err = kern_path(sbi->local_src, 0, &root_path);
	if (err) {
		hmdfs_err("kern_path failed err = %d", err);
		return NULL;
	}
	relative = get_relative_path(sbi, name);
	if (!relative) {
		hmdfs_err("get relative path failed");
		goto err_root_path;
	}
	err = vfs_path_lookup(root_path.dentry, root_path.mnt, relative, 0,
			      &path);
	if (err) {
		hmdfs_err("lookup failed err = %d", err);
		goto err_root_path;
	}

	filp = hmdfs_server_cache_revalidate(sbi, relative, &path);
	if (IS_ERR_OR_NULL(filp)) {
		filp = hmdfs_server_rebuild_dents(sbi, &path, NULL, relative);
		if (IS_ERR_OR_NULL(filp))
			goto err_lookup_path;
	}

err_lookup_path:
	path_put(&path);
err_root_path:
	path_put(&root_path);
	return filp;
}

/* read all dentry in target path directory */
int read_dentry(struct hmdfs_sb_info *sbi, char *file_name,
		struct dir_context *ctx)
{
	unsigned long pos = (unsigned long)(ctx->pos);
	unsigned long group_id = (pos << (1 + DEV_ID_BIT_NUM)) >>
				 (POS_BIT_NUM - GROUP_ID_BIT_NUM);
	unsigned long offset = pos & OFFSET_BIT_MASK;
	struct hmdfs_dentry_group *dentry_group = NULL;
	struct file *handler = NULL;
	int group_num = 0;
	int iterate_result = 0;
	int i, j;
	const struct cred *saved_cred;

	saved_cred = hmdfs_override_fsids(false);
	if (!saved_cred) {
		hmdfs_err("prepare cred failed!");
		return -ENOMEM;
	}


	if (!file_name)
		return -EINVAL;

	dentry_group = kzalloc(sizeof(*dentry_group), GFP_KERNEL);
	if (!dentry_group)
		return -ENOMEM;

	handler = hmdfs_get_or_create_dents(sbi, file_name);
	if (IS_ERR_OR_NULL(handler)) {
		kfree(dentry_group);
		return -ENOENT;
	}

	group_num = get_dentry_group_cnt(file_inode(handler));

	for (i = group_id; i < group_num; i++) {
		hmdfs_metainfo_read(sbi, handler, dentry_group,
				    sizeof(struct hmdfs_dentry_group), i);
		for (j = offset; j < DENTRY_PER_GROUP; j++) {
			int len;
			int file_type = 0;
			bool is_continue;

			len = le16_to_cpu(dentry_group->nsl[j].namelen);
			if (!test_bit_le(j, dentry_group->bitmap) || len == 0)
				continue;

			if (S_ISDIR(le16_to_cpu(dentry_group->nsl[j].i_mode)))
				file_type = DT_DIR;
			else if (S_ISREG(le16_to_cpu(
					 dentry_group->nsl[j].i_mode)))
				file_type = DT_REG;
			else if (S_ISLNK(le16_to_cpu(
					 dentry_group->nsl[j].i_mode)))
				file_type = DT_LNK;

			pos = hmdfs_set_pos(0, i, j);
			is_continue = dir_emit(
				ctx, dentry_group->filename[j], len,
				le64_to_cpu(dentry_group->nsl[j].i_ino),
				file_type);
			if (!is_continue) {
				ctx->pos = pos;
				iterate_result = 1;
				goto done;
			}
		}
		offset = 0;
	}

done:
	hmdfs_revert_fsids(saved_cred);
	kfree(dentry_group);
	fput(handler);
	return iterate_result;
}

unsigned int get_max_depth(struct file *filp)
{
	size_t isize;

	isize = get_dentry_group_cnt(file_inode(filp)) / BUCKET_BLOCKS;

	return get_count_order(isize + 1);
}

struct hmdfs_dentry_group *find_dentry_page(struct hmdfs_sb_info *sbi,
					    pgoff_t index, struct file *filp)
{
	int size;
	struct hmdfs_dentry_group *dentry_blk = NULL;
	loff_t pos = get_dentry_group_pos(index);
	int err;

	dentry_blk = kmalloc(sizeof(*dentry_blk), GFP_KERNEL);
	if (!dentry_blk)
		return NULL;

	err = hmdfs_wlock_file(filp, pos, DENTRYGROUP_SIZE);
	if (err) {
		hmdfs_err("lock file pos %lld failed", pos);
		kfree(dentry_blk);
		return NULL;
	}

	size = cache_file_read(sbi, filp, dentry_blk, (size_t)DENTRYGROUP_SIZE,
			       &pos);
	if (size != DENTRYGROUP_SIZE) {
		kfree(dentry_blk);
		dentry_blk = NULL;
	}

	return dentry_blk;
}

static ssize_t write_dentry_page(struct file *filp, const void *buffer,
				 int buffersize, loff_t position)
{
	ssize_t size;

	size = kernel_write(filp, buffer, (size_t)buffersize, &position);
	if (size != buffersize)
		hmdfs_err("write failed, ret = %ld", size);

	return size;
}

static struct hmdfs_dentry *find_in_block(struct hmdfs_dentry_group *dentry_blk,
					  __u32 namehash,
					  const struct qstr *qstr,
					  struct hmdfs_dentry **insense_de,
					  bool case_sense)
{
	struct hmdfs_dentry *de;
	unsigned long bit_pos = 0;
	int max_len = 0;

	while (bit_pos < DENTRY_PER_GROUP) {
		if (!test_bit_le(bit_pos, dentry_blk->bitmap)) {
			bit_pos++;
			max_len++;
		}
		de = &dentry_blk->nsl[bit_pos];
		if (unlikely(!de->namelen)) {
			bit_pos++;
			continue;
		}

		if (le32_to_cpu(de->hash) == namehash &&
		    le16_to_cpu(de->namelen) == qstr->len &&
		    !memcmp(qstr->name, dentry_blk->filename[bit_pos],
			    le16_to_cpu(de->namelen)))
			goto found;
		if (!(*insense_de) && !case_sense &&
		    le32_to_cpu(de->hash) == namehash &&
		    le16_to_cpu(de->namelen) == qstr->len &&
		    str_n_case_eq(qstr->name, dentry_blk->filename[bit_pos],
				  le16_to_cpu(de->namelen)))
			*insense_de = de;
		max_len = 0;
		bit_pos += get_dentry_slots(le16_to_cpu(de->namelen));
	}
	de = NULL;
found:
	return de;
}

static struct hmdfs_dentry *hmdfs_in_level(struct dentry *child_dentry,
					   unsigned int level,
					   struct hmdfs_dcache_lookup_ctx *ctx)
{
	unsigned int nbucket;
	unsigned int bidx, end_block;
	struct hmdfs_dentry *de = NULL;
	struct hmdfs_dentry *tmp_insense_de = NULL;
	struct hmdfs_dentry_group *dentry_blk;

	nbucket = get_bucket_by_level(level);
	if (!nbucket)
		return de;

	bidx = get_bucketaddr(level, ctx->hash % nbucket) * BUCKET_BLOCKS;
	end_block = bidx + BUCKET_BLOCKS;

	for (; bidx < end_block; bidx++) {
		dentry_blk = find_dentry_page(ctx->sbi, bidx, ctx->filp);
		if (!dentry_blk)
			break;

		de = find_in_block(dentry_blk, ctx->hash, ctx->name,
				   &tmp_insense_de, ctx->sbi->s_case_sensitive);
		if (!de && !(ctx->insense_de) && tmp_insense_de) {
			ctx->insense_de = tmp_insense_de;
			ctx->insense_page = dentry_blk;
			ctx->insense_bidx = bidx;
		} else if (!de) {
			hmdfs_unlock_file(ctx->filp, get_dentry_group_pos(bidx),
					  DENTRYGROUP_SIZE);
			kfree(dentry_blk);
		} else {
			ctx->page = dentry_blk;
			break;
		}
	}
	ctx->bidx = bidx;
	return de;
}

struct hmdfs_dentry *hmdfs_find_dentry(struct dentry *child_dentry,
				       struct hmdfs_dcache_lookup_ctx *ctx)
{
	struct hmdfs_dentry *de = NULL;
	unsigned int max_depth;
	unsigned int level;

	if (!ctx->filp)
		return NULL;

	ctx->hash = hmdfs_dentry_hash(ctx->name, ctx->sbi->s_case_sensitive);

	max_depth = get_max_depth(ctx->filp);
	for (level = 0; level < max_depth; level++) {
		de = hmdfs_in_level(child_dentry, level, ctx);
		if (de) {
			if (ctx->insense_page) {
				hmdfs_unlock_file(ctx->filp,
					get_dentry_group_pos(ctx->insense_bidx),
					DENTRYGROUP_SIZE);
				kfree(ctx->insense_page);
				ctx->insense_page = NULL;
			}
			return de;
		}
	}
	if (ctx->insense_de) {
		ctx->bidx = ctx->insense_bidx;
		ctx->page = ctx->insense_page;
		ctx->insense_bidx = 0;
		ctx->insense_page = NULL;
	}
	return ctx->insense_de;
}

void update_dentry(struct hmdfs_dentry_group *d, struct dentry *child_dentry,
		   struct inode *inode, __u32 name_hash, unsigned int bit_pos)
{
	struct hmdfs_dentry *de;
	struct hmdfs_dentry_info *gdi = hmdfs_d(child_dentry);
	const struct qstr name = child_dentry->d_name;
	int slots = get_dentry_slots(name.len);
	int i;
	unsigned long ino;
	__u32 igen;

	/*
	 * If the dentry's inode is symlink, it must be lower inode,
	 * and we should use the upper ino and generation to fill
	 * the dentryfile.
	 */
	if (!gdi && S_ISLNK(d_inode(child_dentry)->i_mode)) {
		ino = d_inode(child_dentry)->i_ino;
		igen = d_inode(child_dentry)->i_generation;
	} else {
		ino = inode->i_ino;
		igen = inode->i_generation;
	}

	de = &d->nsl[bit_pos];
	de->hash = cpu_to_le32(name_hash);
	de->namelen = cpu_to_le16(name.len);
	memcpy(d->filename[bit_pos], name.name, name.len);
	de->i_mtime = cpu_to_le64(inode->i_mtime.tv_sec);
	de->i_mtime_nsec = cpu_to_le32(inode->i_mtime.tv_nsec);
	de->i_size = cpu_to_le64(inode->i_size);
	de->i_ino = cpu_to_le64(generate_u64_ino(ino, igen));
	de->i_flag = 0;

	/*
	 * If the dentry has fsdata, we just assume it must be
	 * hmdfs filesystem's dentry.
	 * Only client may update it's info in dentryfile when rename
	 * the remote file.
	 * Since the symlink mtime and size is from server's lower
	 * inode, we should just use it and only set S_IFLNK in mode.
	 */
	if (gdi && hm_islnk(gdi->file_type))
		de->i_mode = cpu_to_le16(S_IFLNK);
	else if (!gdi && S_ISLNK(d_inode(child_dentry)->i_mode))
		de->i_mode = d_inode(child_dentry)->i_mode;
	else
		de->i_mode = cpu_to_le16(inode->i_mode);

	for (i = 0; i < slots; i++) {
		__set_bit_le(bit_pos + i, d->bitmap);
		/* avoid wrong garbage data for readdir */
		if (i)
			(de + i)->namelen = 0;
	}
}

int room_for_filename(const void *bitmap, int slots, int max_slots)
{
	int bit_start = 0;
	int zero_start, zero_end;
next:
	zero_start = find_next_zero_bit_le(bitmap, max_slots, bit_start);
	if (zero_start >= max_slots)
		return max_slots;

	zero_end = find_next_bit_le(bitmap, max_slots, zero_start);
	if (zero_end - zero_start >= slots)
		return zero_start;

	bit_start = zero_end + 1;

	if (zero_end + 1 >= max_slots)
		return max_slots;
	goto next;
}

void create_in_cache_file(uint64_t dev_id, struct dentry *dentry)
{
	struct clearcache_item *item = NULL;

	item = hmdfs_find_cache_item(dev_id, dentry->d_parent);
	if (item) {
		if (d_inode(dentry))
			create_dentry(dentry, d_inode(dentry), item->filp,
				      hmdfs_sb(dentry->d_sb));
		else
			hmdfs_err("inode is null!");
		kref_put(&item->ref, release_cache_item);
	} else {
		hmdfs_info("find cache item failed, device_id:%llu", dev_id);
	}
}

int create_dentry(struct dentry *child_dentry, struct inode *inode,
		  struct file *file, struct hmdfs_sb_info *sbi)
{
	unsigned int bit_pos, level;
	unsigned long bidx, end_block;
	const struct qstr qstr = child_dentry->d_name;
	__u32 namehash;
	loff_t pos;
	ssize_t size;
	int ret = 0;
	struct hmdfs_dentry_group *dentry_blk = NULL;

	level = 0;

	namehash = hmdfs_dentry_hash(&qstr, sbi->s_case_sensitive);

	dentry_blk = kmalloc(sizeof(*dentry_blk), GFP_KERNEL);
	if (!dentry_blk) {
		ret = -ENOMEM;
		goto out_err;
	}
find:
	if (level == MAX_BUCKET_LEVEL) {
		ret = -ENOSPC;
		goto out;
	}
	bidx = BUCKET_BLOCKS *
	       get_bucketaddr(level, namehash % get_bucket_by_level(level));
	end_block = bidx + BUCKET_BLOCKS;
	if (end_block > get_dentry_group_cnt(file_inode(file))) {
		if (cache_file_truncate(sbi, &(file->f_path),
					get_dcache_file_size(level))) {
			ret = -ENOSPC;
			goto out;
		}
	}

	for (; bidx < end_block; bidx++) {
		int size;

		pos = get_dentry_group_pos(bidx);
		ret = hmdfs_wlock_file(file, pos, DENTRYGROUP_SIZE);
		if (ret)
			goto out;

		size = cache_file_read(sbi, file, dentry_blk,
				       (size_t)DENTRYGROUP_SIZE, &pos);
		if (size != DENTRYGROUP_SIZE) {
			ret = -ENOSPC;
			hmdfs_unlock_file(file, pos, DENTRYGROUP_SIZE);
			goto out;
		}

		bit_pos = room_for_filename(&dentry_blk->bitmap,
					    get_dentry_slots(qstr.len),
					    DENTRY_PER_GROUP);
		if (bit_pos < DENTRY_PER_GROUP)
			goto add;
		hmdfs_unlock_file(file, pos, DENTRYGROUP_SIZE);
	}
	++level;
	goto find;
add:
	pos = get_dentry_group_pos(bidx);
	update_dentry(dentry_blk, child_dentry, inode, namehash, bit_pos);
	size = cache_file_write(sbi, file, dentry_blk,
				sizeof(struct hmdfs_dentry_group), &pos);
	if (size != sizeof(struct hmdfs_dentry_group))
		hmdfs_err("cache file write failed!, ret = %ld", size);
	hmdfs_unlock_file(file, pos, DENTRYGROUP_SIZE);
out:
	kfree(dentry_blk);
out_err:
	return ret;
}

void hmdfs_init_dcache_lookup_ctx(struct hmdfs_dcache_lookup_ctx *ctx,
				  struct hmdfs_sb_info *sbi,
				  const struct qstr *qstr, struct file *filp)
{
	ctx->sbi = sbi;
	ctx->name = qstr;
	ctx->filp = filp;
	ctx->bidx = 0;
	ctx->page = NULL;
	ctx->insense_de = NULL;
	ctx->insense_bidx = 0;
	ctx->insense_page = NULL;
}

int update_inode_to_dentry(struct dentry *child_dentry, struct inode *inode)
{
	struct hmdfs_sb_info *sbi = d_inode(child_dentry)->i_sb->s_fs_info;
	struct hmdfs_dentry *de = NULL;
	loff_t ipos;
	struct dentry *parent_dentry;
	struct cache_file_node *cfn = NULL;
	char *relative_path = NULL;
	struct hmdfs_dcache_lookup_ctx ctx;

	parent_dentry = child_dentry->d_parent;

	relative_path = hmdfs_get_dentry_relative_path(parent_dentry);
	if (!relative_path)
		return -ENOMEM;

	cfn = find_cfn(sbi, HMDFS_SERVER_CID, relative_path, true);
	if (!cfn)
		goto out;

	hmdfs_init_dcache_lookup_ctx(&ctx, sbi, &child_dentry->d_name,
				     cfn->filp);
	de = hmdfs_find_dentry(child_dentry, &ctx);
	if (!de)
		goto out_cfn;

	de->i_mtime = cpu_to_le64(inode->i_mtime.tv_sec);
	de->i_mtime_nsec = cpu_to_le32(inode->i_mtime.tv_nsec);
	de->i_size = cpu_to_le64(inode->i_size);
	de->i_ino = cpu_to_le64(
		generate_u64_ino(inode->i_ino, inode->i_generation));
	de->i_flag = 0;

	ipos = get_dentry_group_pos(ctx.bidx);
	write_dentry_page(cfn->filp, ctx.page,
			  sizeof(struct hmdfs_dentry_group), ipos);
	hmdfs_unlock_file(cfn->filp, ipos, DENTRYGROUP_SIZE);
	kfree(ctx.page);
out_cfn:
	release_cfn(cfn);
out:
	kfree(relative_path);
	return 0;
}

void hmdfs_delete_dentry(struct dentry *d, struct file *filp)
{
	struct hmdfs_dentry *de = NULL;
	unsigned int bit_pos;
	int slots, i;
	loff_t ipos;
	ssize_t size;
	struct hmdfs_dcache_lookup_ctx ctx;

	hmdfs_init_dcache_lookup_ctx(&ctx, hmdfs_sb(d->d_sb), &d->d_name, filp);

	de = hmdfs_find_dentry(d, &ctx);
	if (IS_ERR_OR_NULL(de)) {
		hmdfs_info("find dentry failed!, err=%ld", PTR_ERR(de));
		return;
	}
	slots = get_dentry_slots(le16_to_cpu(de->namelen));

	bit_pos = de - ctx.page->nsl;
	for (i = 0; i < slots; i++)
		__clear_bit_le(bit_pos + i, &ctx.page->bitmap);

	ipos = get_dentry_group_pos(ctx.bidx);
	size = cache_file_write(hmdfs_sb(d->d_sb), filp, ctx.page,
				sizeof(struct hmdfs_dentry_group), &ipos);
	if (size != sizeof(struct hmdfs_dentry_group))
		hmdfs_err("cache file write failed!, ret = %ld", size);
	hmdfs_unlock_file(filp, ipos, DENTRYGROUP_SIZE);
	kfree(ctx.page);
}

static int hmdfs_get_cache_path(struct hmdfs_sb_info *sbi, struct path *dir)
{
	struct hmdfs_dentry_info *di = hmdfs_d(sbi->sb->s_root);
	int err;

	if (!sbi->s_dentry_cache) {
		*dir = di->lower_path;
		return 0;
	}

	err = kern_path(sbi->cache_dir, LOOKUP_FOLLOW | LOOKUP_DIRECTORY, dir);
	if (err)
		hmdfs_err("open failed, errno = %d", err);

	return err;
}

static void hmdfs_put_cache_path(struct hmdfs_sb_info *sbi, struct path *dir)
{
	if (!sbi->s_dentry_cache)
		return;
	path_put(dir);
}

struct file *create_local_dentry_file_cache(struct hmdfs_sb_info *sbi)
{
	struct file *filp = NULL;
	const struct cred *old_cred = hmdfs_override_creds(sbi->system_cred);
	struct path cache_dir;
	int err;

	err = hmdfs_get_cache_path(sbi, &cache_dir);
	if (err) {
		filp = ERR_PTR(err);
		goto out;
	}

	filp = file_open_root(cache_dir.dentry, cache_dir.mnt, ".",
			      O_RDWR | O_LARGEFILE | O_TMPFILE,
			      DENTRY_FILE_PERM);
	if (IS_ERR(filp))
		hmdfs_err("dentryfile open failed and exit err=%ld",
			  PTR_ERR(filp));

	hmdfs_put_cache_path(sbi, &cache_dir);
out:
	hmdfs_revert_creds(old_cred);
	return filp;
}

static int hmdfs_linkat(struct path *old_path, const char *newname)
{
	struct dentry *new_dentry = NULL;
	struct path new_path;
	int error;

	new_dentry = kern_path_create(AT_FDCWD, newname, &new_path, 0);
	if (IS_ERR(new_dentry)) {
		hmdfs_err("create kernel path failed, error: %ld",
			  PTR_ERR(new_dentry));
		return PTR_ERR(new_dentry);
	}

	error = -EXDEV;
	if (old_path->mnt != new_path.mnt)
		goto out_dput;

	error = vfs_link(old_path->dentry, new_path.dentry->d_inode, new_dentry,
			 NULL);

out_dput:
	done_path_create(&new_path, new_dentry);
	return error;
}

static int cache_file_mkdir(const char *name, umode_t mode)
{
	struct dentry *dentry;
	struct path path;
	int err;

	dentry = kern_path_create(AT_FDCWD, name, &path, LOOKUP_DIRECTORY);
	if (IS_ERR(dentry))
		return PTR_ERR(dentry);

	err = vfs_mkdir(d_inode(path.dentry), dentry, mode);
	if (err && err != -EEXIST)
		hmdfs_err("vfs_mkdir failed, err = %d", err);

	done_path_create(&path, dentry);
	return err;
}

static int cache_file_create_path(const char *fullpath)
{
	char *path;
	char *s;
	int err = 0;

	path = kstrdup(fullpath, GFP_KERNEL);
	if (!path)
		return -ENOMEM;

	s = path + 1;
	while (true) {
		s = strchr(s, '/');
		if (!s)
			break;
		s[0] = '\0';
		err = cache_file_mkdir(path, 0755);
		if (err && err != -EEXIST)
			break;
		s[0] = '/';
		s++;
	}
	kfree(path);
	return err;
}

static void hmdfs_cache_path_create(char *s, const char *dir, bool server)
{
	if (server)
		snprintf(s, PATH_MAX, "%s/dentry_cache/server/", dir);
	else
		snprintf(s, PATH_MAX, "%s/dentry_cache/client/", dir);
}

static void hmdfs_cache_file_create(char *s, uint64_t hash, const char *id,
				    bool server)
{
	int offset = strlen(s);

	if (server)
		snprintf(s + offset, PATH_MAX - offset, "%016llx", hash);
	else
		snprintf(s + offset, PATH_MAX - offset, "%s_%016llx", id, hash);
}

int cache_file_name_generate(char *fullname, struct hmdfs_peer *con,
			     const char *relative_path, bool server)
{
	struct hmdfs_sb_info *sbi = con->sbi;
	uint64_t  hash;
	char cid[HMDFS_CFN_CID_SIZE];
	int err;

	hmdfs_cache_path_create(fullname, sbi->cache_dir, server);

	err = cache_file_create_path(fullname);
	if (err && err != -EEXIST) {
		hmdfs_err("making dir failed %d", err);
		return err;
	}

	strncpy(cid, con->cid, HMDFS_CFN_CID_SIZE - 1);
	cid[HMDFS_CFN_CID_SIZE - 1] = '\0';

	hash = path_hash(relative_path, strlen(relative_path),
			 sbi->s_case_sensitive);
	hmdfs_cache_file_create(fullname, hash, cid, server);

	return 0;
}

static void free_cfn(struct cache_file_node *cfn)
{
	if (!IS_ERR_OR_NULL(cfn->filp))
		filp_close(cfn->filp, NULL);

	kfree(cfn->relative_path);
	kfree(cfn);
}

static bool dentry_file_match(struct cache_file_node *cfn, const char *id,
			      const char *path)
{
	int ret;

	if (cfn->sbi->s_case_sensitive)
		ret = strcmp(cfn->relative_path, path);
	else
		ret = strcasecmp(cfn->relative_path, path);

	return (!ret && !strncmp((cfn)->cid, id, HMDFS_CFN_CID_SIZE - 1));
}

struct cache_file_node *__find_cfn(struct hmdfs_sb_info *sbi, const char *cid,
				   const char *path, bool server)
{
	struct cache_file_node *cfn = NULL;
	struct list_head *head = get_list_head(sbi, server);

	list_for_each_entry(cfn, head, list) {
		if (dentry_file_match(cfn, cid, path)) {
			refcount_inc(&cfn->ref);
			return cfn;
		}
	}
	return NULL;
}

struct cache_file_node *create_cfn(struct hmdfs_sb_info *sbi, const char *path,
				   const char *cid, bool server)
{
	struct cache_file_node *cfn = kzalloc(sizeof(*cfn), GFP_KERNEL);

	if (!cfn)
		return NULL;

	cfn->relative_path = kstrdup(path, GFP_KERNEL);
	if (!cfn->relative_path)
		goto out;

	refcount_set(&cfn->ref, 1);
	strncpy(cfn->cid, cid, HMDFS_CFN_CID_SIZE - 1);
	cfn->cid[HMDFS_CFN_CID_SIZE - 1] = '\0';
	cfn->sbi = sbi;
	cfn->server = server;
	return cfn;
out:
	free_cfn(cfn);
	return NULL;
}

static struct file *insert_cfn(struct hmdfs_sb_info *sbi, const char *filename,
	       const char *path, const char *cid, bool server)
{
	const struct cred *old_cred = NULL;
	struct cache_file_node *cfn = NULL;
	struct cache_file_node *exist = NULL;
	struct list_head *head = NULL;
	struct file *filp = NULL;

	cfn = create_cfn(sbi, path, cid, server);
	if (!cfn)
		return ERR_PTR(-ENOMEM);

	old_cred = hmdfs_override_creds(sbi->system_cred);
	filp = filp_open(filename, O_RDWR | O_LARGEFILE, 0);
	hmdfs_revert_creds(old_cred);
	if (IS_ERR(filp)) {
		hmdfs_err("open file failed, err=%ld", PTR_ERR(filp));
		goto out;
	}

	head = get_list_head(sbi, server);

	mutex_lock(&sbi->cache_list_lock);
	exist = __find_cfn(sbi, cid, path, server);
	if (!exist) {
		cfn->filp = filp;
		list_add_tail(&cfn->list, head);
	} else {
		mutex_unlock(&sbi->cache_list_lock);
		release_cfn(exist);
		filp_close(filp, NULL);
		filp = ERR_PTR(-EEXIST);
		goto out;
	}
	mutex_unlock(&sbi->cache_list_lock);
	return filp;
out:
	free_cfn(cfn);
	return filp;
}

int hmdfs_rename_dentry(struct dentry *old_dentry, struct dentry *new_dentry,
			struct file *old_filp, struct file *new_filp)
{
	int ret;
	struct hmdfs_sb_info *sbi = hmdfs_sb(new_dentry->d_sb);

	/*
	 * Try to delete first, because stale dentry might exist after
	 * coverwrite.
	 */
	hmdfs_delete_dentry(new_dentry, new_filp);

	ret = create_dentry(new_dentry, d_inode(old_dentry), new_filp, sbi);
	if (ret) {
		hmdfs_err("create dentry failed!, err=%d", ret);
		return ret;
	}

	hmdfs_delete_dentry(old_dentry, old_filp);
	return 0;
}

/**
 * cache_file_persistent - link the tmpfile to the cache dir
 * @con:	the connection peer
 * @filp:	the file handler of the tmpfile
 * @relative_path: the relative path which the tmpfile belongs
 * @server:	server or client
 *
 * Return value: the new file handler of the persistent file if the
 * persistent operation succeed. Otherwise will return the original handler
 * of the tmpfile passed in, so that the caller does not have to check
 * the returned handler.
 *
 */
struct file *cache_file_persistent(struct hmdfs_peer *con, struct file *filp,
			   const char *relative_path, bool server)
{
	struct cache_file_node *cfn = NULL;
	char *fullname = NULL;
	char *cid = server ? HMDFS_SERVER_CID : (char *)con->cid;
	struct file *newf = NULL;
	int i = 0;
	int len;
	int err;

	if (!con->sbi->s_dentry_cache)
		return filp;

	cfn = find_cfn(con->sbi, cid, relative_path, server);
	if (cfn) {
		release_cfn(cfn);
		return filp;
	}
	fullname = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!fullname)
		return filp;

	err = cache_file_name_generate(fullname, con, relative_path, server);
	if (err)
		goto out;

	err = __vfs_setxattr(file_dentry(filp), file_inode(filp),
			     DENTRY_FILE_XATTR_NAME, relative_path,
			     strlen(relative_path), 0);
	if (err) {
		hmdfs_err("setxattr for file failed, err=%d", err);
		goto out;
	}

	len = strlen(fullname);

	do {
		err = hmdfs_linkat(&filp->f_path, fullname);
		if (!err)
			break;

		snprintf(fullname + len, PATH_MAX - len, "_%d", i);
	} while (i++ < DENTRY_FILE_NAME_RETRY);

	if (err) {
		hmdfs_err("link for file failed, err=%d", err);
		goto out;
	}

	newf = insert_cfn(con->sbi, fullname, relative_path, cid, server);
	if (!IS_ERR(newf))
		filp = newf;
out:
	kfree(fullname);
	return filp;
}

void __destroy_cfn(struct list_head *head)
{
	struct cache_file_node *cfn = NULL;
	struct cache_file_node *n = NULL;

	list_for_each_entry_safe(cfn, n, head, list) {
		list_del_init(&cfn->list);
		release_cfn(cfn);
	}
}

void hmdfs_cfn_destroy(struct hmdfs_sb_info *sbi)
{
	mutex_lock(&sbi->cache_list_lock);
	__destroy_cfn(&sbi->client_cache);
	__destroy_cfn(&sbi->server_cache);
	mutex_unlock(&sbi->cache_list_lock);
}

struct cache_file_node *find_cfn(struct hmdfs_sb_info *sbi, const char *cid,
				 const char *path, bool server)
{
	struct cache_file_node *cfn = NULL;

	mutex_lock(&sbi->cache_list_lock);
	cfn = __find_cfn(sbi, cid, path, server);
	mutex_unlock(&sbi->cache_list_lock);
	return cfn;
}

void release_cfn(struct cache_file_node *cfn)
{
	if (refcount_dec_and_test(&cfn->ref))
		free_cfn(cfn);
}

void remove_cfn(struct cache_file_node *cfn)
{
	struct hmdfs_sb_info *sbi = cfn->sbi;
	bool deleted;

	mutex_lock(&sbi->cache_list_lock);
	deleted = list_empty(&cfn->list);
	if (!deleted)
		list_del_init(&cfn->list);
	mutex_unlock(&sbi->cache_list_lock);
	if (!deleted) {
		delete_dentry_file(cfn->filp);
		release_cfn(cfn);
	}
}

int hmdfs_do_lock_file(struct file *filp, unsigned char fl_type, loff_t start,
		       loff_t len)
{
	struct file_lock fl;
	int err;

	locks_init_lock(&fl);

	fl.fl_type = fl_type;
	fl.fl_flags = FL_POSIX | FL_CLOSE | FL_SLEEP;
	fl.fl_start = start;
	fl.fl_end = start + len - 1;
	fl.fl_owner = filp;
	fl.fl_pid = current->tgid;
	fl.fl_file = filp;
	fl.fl_ops = NULL;
	fl.fl_lmops = NULL;

	err = locks_lock_file_wait(filp, &fl);
	if (err)
		hmdfs_err("lock file wait failed: %d", err);

	return err;
}

int hmdfs_wlock_file(struct file *filp, loff_t start, loff_t len)
{
	return hmdfs_do_lock_file(filp, F_WRLCK, start, len);
}

int hmdfs_rlock_file(struct file *filp, loff_t start, loff_t len)
{
	return hmdfs_do_lock_file(filp, F_RDLCK, start, len);
}

int hmdfs_unlock_file(struct file *filp, loff_t start, loff_t len)
{
	return hmdfs_do_lock_file(filp, F_UNLCK, start, len);
}

long cache_file_truncate(struct hmdfs_sb_info *sbi, const struct path *path,
			 loff_t length)
{
	const struct cred *old_cred = hmdfs_override_creds(sbi->system_cred);
	long ret = vfs_truncate(path, length);

	hmdfs_revert_creds(old_cred);

	return ret;
}

ssize_t cache_file_read(struct hmdfs_sb_info *sbi, struct file *filp, void *buf,
			size_t count, loff_t *pos)
{
	const struct cred *old_cred = hmdfs_override_creds(sbi->system_cred);
	ssize_t ret = kernel_read(filp, buf, count, pos);

	hmdfs_revert_creds(old_cred);

	return ret;
}

ssize_t cache_file_write(struct hmdfs_sb_info *sbi, struct file *filp,
			 const void *buf, size_t count, loff_t *pos)
{
	const struct cred *old_cred = hmdfs_override_creds(sbi->system_cred);
	ssize_t ret = kernel_write(filp, buf, count, pos);

	hmdfs_revert_creds(old_cred);

	return ret;
}


int read_header(struct hmdfs_sb_info *sbi, struct file *filp,
		struct hmdfs_dcache_header *header)
{
	ssize_t bytes;
	loff_t pos = 0;

	bytes = cache_file_read(sbi, filp, header, sizeof(*header), &pos);
	if (bytes != sizeof(*header)) {
		hmdfs_err("read file failed, err:%ld", bytes);
		return -EIO;
	}

	return 0;
}

static unsigned long long cache_get_dentry_count(struct hmdfs_sb_info *sbi,
						 struct file *filp)
{
	struct hmdfs_dcache_header header;
	int overallpage;

	overallpage = get_dentry_group_cnt(file_inode(filp));
	if (overallpage == 0)
		return 0;

	if (read_header(sbi, filp, &header))
		return 0;

	return le64_to_cpu(header.num);
}

static int cache_check_case_sensitive(struct hmdfs_sb_info *sbi,
				struct file *filp)
{
	struct hmdfs_dcache_header header;

	if (read_header(sbi, filp, &header))
		return 0;

	if (sbi->s_case_sensitive != (bool)header.case_sensitive) {
		hmdfs_info("Case sensitive inconsistent, current fs is: %d, cache is %d, will drop cache",
			   sbi->s_case_sensitive, header.case_sensitive);
		return 0;
	}
	return 1;
}

int write_header(struct file *filp, struct hmdfs_dcache_header *header)
{
	loff_t pos = 0;
	ssize_t size;

	size = kernel_write(filp, header, sizeof(*header), &pos);
	if (size != sizeof(*header)) {
		hmdfs_err("update dcache header failed %ld", size);
		return -EIO;
	}

	return 0;
}

void add_to_delete_list(struct hmdfs_sb_info *sbi, struct cache_file_node *cfn)
{
	mutex_lock(&sbi->cache_list_lock);
	list_add_tail(&cfn->list, &sbi->to_delete);
	mutex_unlock(&sbi->cache_list_lock);
}

void load_cfn(struct hmdfs_sb_info *sbi, const char *fullname, const char *path,
	      const char *cid, bool server)
{
	struct cache_file_node *cfn = NULL;
	struct cache_file_node *cfn1 = NULL;
	struct list_head *head = NULL;

	cfn = create_cfn(sbi, path, cid, server);
	if (!cfn)
		return;

	cfn->filp = filp_open(fullname, O_RDWR | O_LARGEFILE, 0);
	if (IS_ERR(cfn->filp)) {
		hmdfs_err("open fail %ld", PTR_ERR(cfn->filp));
		goto out;
	}

	if (cache_get_dentry_count(sbi, cfn->filp) < sbi->dcache_threshold) {
		add_to_delete_list(sbi, cfn);
		return;
	}

	if (!cache_check_case_sensitive(sbi, cfn->filp)) {
		add_to_delete_list(sbi, cfn);
		return;
	}

	head = get_list_head(sbi, server);

	mutex_lock(&sbi->cache_list_lock);
	cfn1 = __find_cfn(sbi, cid, path, server);
	if (!cfn1) {
		list_add_tail(&cfn->list, head);
	} else {
		release_cfn(cfn1);
		mutex_unlock(&sbi->cache_list_lock);
		add_to_delete_list(sbi, cfn);
		return;
	}
	mutex_unlock(&sbi->cache_list_lock);

	return;
out:
	free_cfn(cfn);
}

static int get_cid_and_hash(const char *name, uint64_t *hash, char *cid)
{
	int len;
	char *p = strstr(name, "_");

	if (!p)
		return -EINVAL;

	len = p - name;
	if (len >= HMDFS_CFN_CID_SIZE)
		return -EINVAL;

	memcpy(cid, name, len);
	cid[len] = '\0';

	if (sscanf(++p, "%llx", hash) != 1)
		return -EINVAL;
	return 0;
}

static void store_one(const char *name, struct cache_file_callback *cb)
{
	struct file *file = NULL;
	char *fullname = NULL;
	char *kvalue = NULL;
	char cid[HMDFS_CFN_CID_SIZE];
	uint64_t hash;
	ssize_t error;

	if (strlen(name) + strlen(cb->dirname) >= PATH_MAX)
		return;

	fullname = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!fullname)
		return;

	snprintf(fullname, PATH_MAX, "%s%s", cb->dirname, name);

	file = filp_open(fullname, O_RDWR | O_LARGEFILE, 0);
	if (IS_ERR(file)) {
		hmdfs_err("open fail %ld", PTR_ERR(file));
		goto out;
	}

	kvalue = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!kvalue)
		goto out_file;

	error = __vfs_getxattr(file_dentry(file), file_inode(file),
			       DENTRY_FILE_XATTR_NAME, kvalue, PATH_MAX);
	if (error <= 0 || error >= PATH_MAX) {
		hmdfs_err("getxattr return: %ld", error);
		goto out_kvalue;
	}
	kvalue[error] = '\0';
	cid[0] = '\0';

	if (!cb->server) {
		if (get_cid_and_hash(name, &hash, cid)) {
			hmdfs_err("get cid and hash fail");
			goto out_kvalue;
		}
	}

	load_cfn(cb->sbi, fullname, kvalue, cid, cb->server);

out_kvalue:
	kfree(kvalue);
out_file:
	filp_close(file, NULL);
out:
	kfree(fullname);
}

static int cache_file_iterate(struct dir_context *ctx, const char *name,
			      int name_len, loff_t offset, u64 ino,
			      unsigned int d_type)
{
	struct cache_file_item *cfi = NULL;
	struct cache_file_callback *cb =
			container_of(ctx, struct cache_file_callback, ctx);

	if (name_len > NAME_MAX) {
		hmdfs_err("name_len:%d NAME_MAX:%u", name_len, NAME_MAX);
		return 0;
	}

	if (d_type != DT_REG)
		return 0;

	cfi = kmalloc(sizeof(*cfi), GFP_KERNEL);
	if (!cfi)
		return -ENOMEM;

	cfi->name = kstrndup(name, name_len, GFP_KERNEL);
	if (!cfi->name) {
		kfree(cfi);
		return -ENOMEM;
	}

	list_add_tail(&cfi->list, &cb->list);

	return 0;
}

void hmdfs_do_load(struct hmdfs_sb_info *sbi, const char *fullname, bool server)
{
	struct file *file = NULL;
	struct path dirpath;
	int err;
	struct cache_file_item *cfi = NULL;
	struct cache_file_item *n = NULL;
	struct cache_file_callback cb = {
		.ctx.actor = cache_file_iterate,
		.ctx.pos = 0,
		.dirname = fullname,
		.sbi = sbi,
		.server = server,
	};
	INIT_LIST_HEAD(&cb.list);


	err = kern_path(fullname, LOOKUP_DIRECTORY, &dirpath);
	if (err) {
		hmdfs_info("No file path");
		return;
	}

	file = dentry_open(&dirpath, O_RDONLY, current_cred());
	if (IS_ERR_OR_NULL(file)) {
		hmdfs_err("dentry_open failed, error: %ld", PTR_ERR(file));
		path_put(&dirpath);
		return;
	}

	err = iterate_dir(file, &cb.ctx);
	if (err)
		hmdfs_err("iterate_dir failed, err: %d", err);

	list_for_each_entry_safe(cfi, n, &cb.list, list) {
		store_one(cfi->name, &cb);
		list_del_init(&cfi->list);
		kfree(cfi->name);
		kfree(cfi);
	}

	fput(file);
	path_put(&dirpath);
}

/**
 * This function just used for delete dentryfile.dat
 */
int delete_dentry_file(struct file *filp)
{
	int err = 0;
	struct dentry *dentry = file_dentry(filp);
	struct dentry *parent = lock_parent(dentry);

	if (dentry->d_parent == parent) {
		dget(dentry);
		err = vfs_unlink(d_inode(parent), dentry, NULL);
		dput(dentry);
	}
	unlock_dir(parent);

	return err;
}

void hmdfs_delete_useless_cfn(struct hmdfs_sb_info *sbi)
{
	struct cache_file_node *cfn = NULL;
	struct cache_file_node *n = NULL;

	mutex_lock(&sbi->cache_list_lock);

	list_for_each_entry_safe(cfn, n, &sbi->to_delete, list) {
		delete_dentry_file(cfn->filp);
		list_del_init(&cfn->list);
		release_cfn(cfn);
	}
	mutex_unlock(&sbi->cache_list_lock);
}

void hmdfs_cfn_load(struct hmdfs_sb_info *sbi)
{
	char *fullname = NULL;

	if (!sbi->s_dentry_cache)
		return;

	fullname = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!fullname)
		return;

	snprintf(fullname, PATH_MAX, "%s/dentry_cache/client/",
		 sbi->cache_dir);
	hmdfs_do_load(sbi, fullname, false);

	snprintf(fullname, PATH_MAX, "%s/dentry_cache/server/",
		 sbi->cache_dir);
	hmdfs_do_load(sbi, fullname, true);
	kfree(fullname);

	hmdfs_delete_useless_cfn(sbi);
}

static void __cache_file_destroy_by_path(struct list_head *head,
					 const char *path)
{
	struct cache_file_node *cfn = NULL;
	struct cache_file_node *n = NULL;

	list_for_each_entry_safe(cfn, n, head, list) {
		if (strcmp(path, cfn->relative_path) != 0)
			continue;
		list_del_init(&cfn->list);
		delete_dentry_file(cfn->filp);
		release_cfn(cfn);
	}
}

static void cache_file_destroy_by_path(struct hmdfs_sb_info *sbi,
				       const char *path)
{
	mutex_lock(&sbi->cache_list_lock);

	__cache_file_destroy_by_path(&sbi->server_cache, path);
	__cache_file_destroy_by_path(&sbi->client_cache, path);

	mutex_unlock(&sbi->cache_list_lock);
}

static void cache_file_find_and_delete(struct hmdfs_peer *con,
				       const char *relative_path)
{
	struct cache_file_node *cfn;

	cfn = find_cfn(con->sbi, con->cid, relative_path, false);
	if (!cfn)
		return;

	remove_cfn(cfn);
	release_cfn(cfn);
}

void cache_file_delete_by_dentry(struct hmdfs_peer *con, struct dentry *dentry)
{
	char *relative_path = NULL;

	relative_path = hmdfs_get_dentry_relative_path(dentry);
	if (unlikely(!relative_path)) {
		hmdfs_err("get relative path failed %d", -ENOMEM);
		return;
	}
	cache_file_find_and_delete(con, relative_path);
	kfree(relative_path);
}

struct file *hmdfs_get_new_dentry_file(struct hmdfs_peer *con,
				       const char *relative_path,
				       struct hmdfs_dcache_header *header)
{
	struct hmdfs_sb_info *sbi = con->sbi;
	int len = strlen(relative_path);
	struct file *filp = NULL;
	int err;

	filp = create_local_dentry_file_cache(sbi);
	if (IS_ERR(filp))
		return filp;

	err = hmdfs_client_start_readdir(con, filp, relative_path, len, header);
	if (err) {
		if (err != -ENOENT)
			hmdfs_err("readdir failed dev: %llu err: %d",
				  con->device_id, err);
		fput(filp);
		filp = ERR_PTR(err);
	}

	return filp;
}

void add_cfn_to_item(struct dentry *dentry, struct hmdfs_peer *con,
		     struct cache_file_node *cfn)
{
	struct file *file = cfn->filp;
	int err;

	err = hmdfs_add_cache_list(con->device_id, dentry, file);
	if (unlikely(err)) {
		hmdfs_err("add cache list failed devid:%llu err:%d",
			  con->device_id, err);
		return;
	}
}

int hmdfs_add_file_to_cache(struct dentry *dentry, struct hmdfs_peer *con,
			    struct file *file, const char *relative_path)
{
	struct hmdfs_sb_info *sbi = con->sbi;
	struct file *newf = file;

	if (cache_get_dentry_count(sbi, file) >= sbi->dcache_threshold)
		newf = cache_file_persistent(con, file, relative_path, false);
	else
		cache_file_find_and_delete(con, relative_path);

	return hmdfs_add_cache_list(con->device_id, dentry, newf);
}

static struct file *read_header_and_revalidate(struct hmdfs_peer *con,
					       struct file *filp,
					       const char *relative_path)
{
	struct hmdfs_dcache_header header;
	struct hmdfs_dcache_header *p = NULL;

	if (read_header(con->sbi, filp, &header) == 0)
		p = &header;

	return hmdfs_get_new_dentry_file(con, relative_path, p);
}

void remote_file_revalidate_cfn(struct dentry *dentry, struct hmdfs_peer *con,
				struct cache_file_node *cfn,
				const char *relative_path)
{
	struct file *file = NULL;
	int err;

	file = read_header_and_revalidate(con, cfn->filp, relative_path);
	if (IS_ERR(file))
		return;

	/*
	 * If the request returned ok but file length is 0, we assume
	 * that the server verified the client cache file is uptodate.
	 */
	if (i_size_read(file->f_inode) == 0) {
		hmdfs_info("The cfn cache for dev:%llu is uptodate",
			    con->device_id);
		fput(file);
		add_cfn_to_item(dentry, con, cfn);
		return;
	}

	/* OK, cfn is not uptodate, let's remove it and add the new file */
	remove_cfn(cfn);

	err = hmdfs_add_file_to_cache(dentry, con, file, relative_path);
	if (unlikely(err))
		hmdfs_err("add cache list failed devid:%llu err:%d",
			  con->device_id, err);
	fput(file);
}

void remote_file_revalidate_item(struct dentry *dentry, struct hmdfs_peer *con,
				 struct clearcache_item *item,
				 const char *relative_path)
{
	struct file *file = NULL;
	int err;

	file = read_header_and_revalidate(con, item->filp, relative_path);
	if (IS_ERR(file))
		return;

	/*
	 * If the request returned ok but file length is 0, we assume
	 * that the server verified the client cache file is uptodate.
	 */
	if (i_size_read(file->f_inode) == 0) {
		hmdfs_info("The item cache for dev:%llu is uptodate",
			    con->device_id);
		item->time = jiffies;
		fput(file);
		return;
	}

	/* We need to replace the old item */
	remove_cache_item(item);
	cache_file_find_and_delete(con, relative_path);

	err = hmdfs_add_file_to_cache(dentry, con, file, relative_path);
	if (unlikely(err))
		hmdfs_err("add cache list failed devid:%llu err:%d",
			  con->device_id, err);
	fput(file);
}

bool get_remote_dentry_file(struct dentry *dentry, struct hmdfs_peer *con)
{
	struct hmdfs_dentry_info *d_info = hmdfs_d(dentry);
	struct cache_file_node *cfn = NULL;
	struct hmdfs_sb_info *sbi = con->sbi;
	char *relative_path = NULL;
	int err = 0;
	struct file *filp = NULL;
	struct clearcache_item *item;

	if (hmdfs_cache_revalidate(READ_ONCE(con->conn_time), con->device_id,
				   dentry))
		return false;

	relative_path = hmdfs_get_dentry_relative_path(dentry);
	if (unlikely(!relative_path)) {
		hmdfs_err("get relative path failed %d", -ENOMEM);
		return false;
	}
	mutex_lock(&d_info->cache_pull_lock);
	if (hmdfs_cache_revalidate(READ_ONCE(con->conn_time), con->device_id,
				   dentry))
		goto out_unlock;

	item = hmdfs_find_cache_item(con->device_id, dentry);
	if (item) {
		remote_file_revalidate_item(dentry, con, item, relative_path);
		kref_put(&item->ref, release_cache_item);
		goto out_unlock;
	}

	cfn = find_cfn(sbi, con->cid, relative_path, false);
	if (cfn) {
		remote_file_revalidate_cfn(dentry, con, cfn, relative_path);
		release_cfn(cfn);
		goto out_unlock;
	}

	filp = hmdfs_get_new_dentry_file(con, relative_path, NULL);
	if (IS_ERR(filp)) {
		err = PTR_ERR(filp);
		goto out_unlock;
	}

	err = hmdfs_add_file_to_cache(dentry, con, filp, relative_path);
	if (unlikely(err))
		hmdfs_err("add cache list failed devid:%lu err:%d",
			  (unsigned long)con->device_id, err);
	fput(filp);

out_unlock:
	mutex_unlock(&d_info->cache_pull_lock);
	if (err && err != -ENOENT)
		hmdfs_err("readdir failed dev:%lu err:%d",
			  (unsigned long)con->device_id, err);
	kfree(relative_path);
	return true;
}

int hmdfs_file_type(const char *name)
{
	if (!name)
		return -EINVAL;

	if (!strcmp(name, CURRENT_DIR) || !strcmp(name, PARENT_DIR))
		return HMDFS_TYPE_DOT;

	return HMDFS_TYPE_COMMON;
}

struct clearcache_item *hmdfs_find_cache_item(uint64_t dev_id,
					      struct dentry *dentry)
{
	struct clearcache_item *item = NULL;
	struct hmdfs_dentry_info *d_info = hmdfs_d(dentry);

	if (!d_info)
		return NULL;

	spin_lock(&d_info->cache_list_lock);
	list_for_each_entry(item, &(d_info->cache_list_head), list) {
		if (dev_id == item->dev_id) {
			kref_get(&item->ref);
			spin_unlock(&d_info->cache_list_lock);
			return item;
		}
	}
	spin_unlock(&d_info->cache_list_lock);
	return NULL;
}

bool hmdfs_cache_revalidate(unsigned long conn_time, uint64_t dev_id,
			    struct dentry *dentry)
{
	bool ret = false;
	struct clearcache_item *item = NULL;
	struct hmdfs_dentry_info *d_info = hmdfs_d(dentry);
	unsigned int timeout;

	if (!d_info)
		return ret;

	timeout = hmdfs_sb(dentry->d_sb)->dcache_timeout;
	spin_lock(&d_info->cache_list_lock);
	list_for_each_entry(item, &(d_info->cache_list_head), list) {
		if (dev_id == item->dev_id) {
			ret = cache_item_revalidate(conn_time, item->time,
						    timeout);
			break;
		}
	}
	spin_unlock(&d_info->cache_list_lock);
	return ret;
}

void remove_cache_item(struct clearcache_item *item)
{
	bool deleted;

	spin_lock(&item->d_info->cache_list_lock);
	deleted = list_empty(&item->list);
	if (!deleted)
		list_del_init(&item->list);
	spin_unlock(&item->d_info->cache_list_lock);
	if (!deleted)
		kref_put(&item->ref, release_cache_item);
}

void release_cache_item(struct kref *ref)
{
	struct clearcache_item *item =
		container_of(ref, struct clearcache_item, ref);

	if (item->filp)
		fput(item->filp);
	kfree(item);
}

void hmdfs_remove_cache_filp(struct hmdfs_peer *con, struct dentry *dentry)
{
	struct clearcache_item *item = NULL;
	struct clearcache_item *item_temp = NULL;
	struct hmdfs_dentry_info *d_info = hmdfs_d(dentry);
	//	struct path *lower_path = NULL;

	if (!d_info)
		return;

	spin_lock(&d_info->cache_list_lock);
	list_for_each_entry_safe(item, item_temp, &(d_info->cache_list_head),
				  list) {
		if (con->device_id == item->dev_id) {
			list_del_init(&item->list);
			spin_unlock(&d_info->cache_list_lock);
			cache_file_delete_by_dentry(con, dentry);
			kref_put(&item->ref, release_cache_item);
			return;
		}
	}
	spin_unlock(&d_info->cache_list_lock);
}

int hmdfs_add_cache_list(uint64_t dev_id, struct dentry *dentry,
			 struct file *filp)
{
	struct clearcache_item *item = NULL;
	struct hmdfs_dentry_info *d_info = hmdfs_d(dentry);

	if (!d_info)
		return -ENOMEM;

	item = kzalloc(sizeof(*item), GFP_KERNEL);
	if (!item)
		return -ENOMEM;

	item->dev_id = dev_id;
	item->filp = get_file(filp);
	item->time = jiffies;
	item->d_info = d_info;
	kref_init(&item->ref);
	spin_lock(&d_info->cache_list_lock);
	list_add_tail(&(item->list), &(d_info->cache_list_head));
	spin_unlock(&d_info->cache_list_lock);
	return 0;
}

void hmdfs_add_remote_cache_list(struct hmdfs_peer *con, const char *dir_path)
{
	int err = 0;
	struct remotecache_item *item = NULL;
	struct remotecache_item *item_temp = NULL;
	struct path path, root_path;
	struct hmdfs_dentry_info *d_info = NULL;

	err = kern_path(con->sbi->local_dst, 0, &root_path);
	if (err) {
		hmdfs_err("kern_path failed err = %d", err);
		return;
	}

	err = vfs_path_lookup(root_path.dentry, root_path.mnt, dir_path, 0,
			      &path);
	if (err)
		goto out_put_root;

	d_info = hmdfs_d(path.dentry);
	if (!d_info) {
		err = -EINVAL;
		goto out;
	}

	/* find duplicate con */
	mutex_lock(&d_info->remote_cache_list_lock);
	list_for_each_entry_safe(item, item_temp,
				  &(d_info->remote_cache_list_head), list) {
		if (item->con->device_id == con->device_id) {
			mutex_unlock(&d_info->remote_cache_list_lock);
			goto out;
		}
	}

	item = kzalloc(sizeof(*item), GFP_KERNEL);
	if (!item) {
		err = -ENOMEM;
		mutex_unlock(&d_info->remote_cache_list_lock);
		goto out;
	}

	item->con = con;
	item->drop_flag = 0;
	list_add(&(item->list), &(d_info->remote_cache_list_head));
	mutex_unlock(&d_info->remote_cache_list_lock);

out:
	path_put(&path);
out_put_root:
	path_put(&root_path);
}

int hmdfs_drop_remote_cache_dents(struct dentry *dentry)
{
	struct path lower_path;
	struct inode *lower_inode = NULL;
	struct remotecache_item *item = NULL;
	struct remotecache_item *item_temp = NULL;
	struct hmdfs_dentry_info *d_info = NULL;
	char *relative_path = NULL;

	if (!dentry) {
		hmdfs_err("dentry null and return");
		return 0;
	}

	d_info = hmdfs_d(dentry);
	if (!d_info) {
		hmdfs_err("d_info null and return");
		return 0;
	}
	hmdfs_get_lower_path(dentry, &lower_path);
	if (IS_ERR_OR_NULL(lower_path.dentry)) {
		hmdfs_put_lower_path(&lower_path);
		return 0;
	}
	lower_inode = d_inode(lower_path.dentry);
	hmdfs_put_lower_path(&lower_path);
	if (IS_ERR_OR_NULL(lower_inode))
		return 0;
	/* only for directory */
	if (!S_ISDIR(lower_inode->i_mode))
		return 0;

	relative_path = hmdfs_get_dentry_relative_path(dentry);
	if (!relative_path) {
		hmdfs_err("get dentry relative path failed");
		return 0;
	}
	mutex_lock(&d_info->remote_cache_list_lock);
	list_for_each_entry_safe(item, item_temp,
				  &(d_info->remote_cache_list_head), list) {
		if (item->drop_flag) {
			item->drop_flag = 0;
			continue;
		}
		mutex_unlock(&d_info->remote_cache_list_lock);
		hmdfs_send_drop_push(item->con, relative_path);
		mutex_lock(&d_info->remote_cache_list_lock);
		list_del(&item->list);
		kfree(item);
	}
	mutex_unlock(&d_info->remote_cache_list_lock);

	kfree(relative_path);
	return 0;
}

/* Clear the dentry cache files of target directory */
int hmdfs_clear_cache_dents(struct dentry *dentry, bool remove_cache)
{
	struct clearcache_item *item = NULL;
	struct clearcache_item *item_temp = NULL;
	struct hmdfs_dentry_info *d_info = hmdfs_d(dentry);
	char *path = NULL;

	if (!d_info)
		return 0;

	spin_lock(&d_info->cache_list_lock);
	list_for_each_entry_safe(item, item_temp, &(d_info->cache_list_head),
				  list) {
		list_del_init(&item->list);
		kref_put(&item->ref, release_cache_item);
	}
	spin_unlock(&d_info->cache_list_lock);

	if (!remove_cache)
		return 0;

	/* it also need confirm that there are no dentryfile_dev*
	 * under this dentry
	 */
	path = hmdfs_get_dentry_relative_path(dentry);

	if (unlikely(!path)) {
		hmdfs_err("get relative path failed");
		return 0;
	}

	cache_file_destroy_by_path(hmdfs_sb(dentry->d_sb), path);

	kfree(path);
	return 0;
}

void hmdfs_mark_drop_flag(uint64_t device_id, struct dentry *dentry)
{
	struct remotecache_item *item = NULL;
	struct hmdfs_dentry_info *d_info = NULL;

	d_info = hmdfs_d(dentry);
	if (!d_info) {
		hmdfs_err("d_info null and return");
		return;
	}

	mutex_lock(&d_info->remote_cache_list_lock);
	list_for_each_entry(item, &(d_info->remote_cache_list_head), list) {
		if (item->con->device_id == device_id) {
			item->drop_flag = 1;
			break;
		}
	}
	mutex_unlock(&d_info->remote_cache_list_lock);
}

void hmdfs_clear_drop_flag(struct dentry *dentry)
{
	struct remotecache_item *item = NULL;
	struct hmdfs_dentry_info *d_info = NULL;

	if (!dentry) {
		hmdfs_err("dentry null and return");
		return;
	}

	d_info = hmdfs_d(dentry);
	if (!d_info) {
		hmdfs_err("d_info null and return");
		return;
	}

	mutex_lock(&d_info->remote_cache_list_lock);
	list_for_each_entry(item, &(d_info->remote_cache_list_head), list) {
		if (item->drop_flag)
			item->drop_flag = 0;
	}
	mutex_unlock(&d_info->remote_cache_list_lock);
}

#define DUSTBIN_SUFFIX ".hwbk"
static void hmdfs_rename_bak(struct dentry *dentry)
{
	struct path lower_path;
	struct dentry *lower_parent = NULL;
	struct dentry *lower_dentry = NULL;
	struct dentry *new_dentry = NULL;
	char *name = NULL;
	int len = 0;
	int err = 0;

	hmdfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	len = strlen(lower_dentry->d_name.name) + strlen(DUSTBIN_SUFFIX) + 2;
	if (len >= NAME_MAX) {
		err = -ENAMETOOLONG;
		goto put_lower_path;
	}

	name = kmalloc(len, GFP_KERNEL);
	if (!name) {
		err = -ENOMEM;
		goto put_lower_path;
	}

	snprintf(name, len, ".%s%s", lower_dentry->d_name.name, DUSTBIN_SUFFIX);
	err = mnt_want_write(lower_path.mnt);
	if (err) {
		hmdfs_info("get write access failed, err %d", err);
		goto free_name;
	}

	lower_parent = lock_parent(lower_dentry);
	new_dentry = lookup_one_len(name, lower_parent, strlen(name));
	if (IS_ERR(new_dentry)) {
		err = PTR_ERR(new_dentry);
		hmdfs_info("lookup new dentry failed, err %d", err);
		goto unlock_parent;
	}

	err = vfs_rename(d_inode(lower_parent), lower_dentry,
			 d_inode(lower_parent), new_dentry, NULL, 0);

	dput(new_dentry);
unlock_parent:
	unlock_dir(lower_parent);
	mnt_drop_write(lower_path.mnt);
free_name:
	kfree(name);
put_lower_path:
	hmdfs_put_lower_path(&lower_path);

	if (err)
		hmdfs_err("failed to rename file, err %d", err);
}

int hmdfs_root_unlink(uint64_t device_id, struct path *root_path,
		      const char *unlink_dir, const char *unlink_name)
{
	int err = 0;
	struct path path;
	struct dentry *child_dentry = NULL;
	struct inode *dir = NULL;
	struct inode *child_inode = NULL;
	kuid_t tmp_uid;

	err = vfs_path_lookup(root_path->dentry, root_path->mnt,
			      unlink_dir, LOOKUP_DIRECTORY, &path);
	if (err) {
		hmdfs_err("found path failed err = %d", err);
		return err;
	}
	dir = d_inode(path.dentry);
	inode_lock_nested(dir, I_MUTEX_PARENT);

	child_dentry = lookup_one_len(unlink_name, path.dentry,
				      strlen(unlink_name));
	if (IS_ERR(child_dentry)) {
		err = PTR_ERR(child_dentry);
		hmdfs_err("lookup_one_len failed, err = %d", err);
		goto unlock_out;
	}
	if (d_is_negative(child_dentry)) {
		err = -ENOENT;
		dput(child_dentry);
		goto unlock_out;
	}
	child_inode = d_inode(child_dentry);

	tmp_uid = hmdfs_override_inode_uid(dir);

	hmdfs_mark_drop_flag(device_id, path.dentry);
	ihold(child_inode);
	err = vfs_unlink(dir, child_dentry, NULL);
	/*
	 * -EOWNERDEAD means we want to put the file in a specail dir instead of
	 *  deleting it, specifically dustbin in phone, so that user can
	 *  recover the deleted images and videos.
	 */
	if (err == -EOWNERDEAD) {
		hmdfs_rename_bak(child_dentry);
		err = 0;
	}
	if (err)
		hmdfs_err("unlink path failed err = %d", err);
	hmdfs_revert_inode_uid(dir, tmp_uid);
	dput(child_dentry);

unlock_out:
	inode_unlock(dir);
	if (child_inode)
		iput(child_inode);
	path_put(&path);
	return err;
}

struct dentry *hmdfs_root_mkdir(uint64_t device_id, const char *local_dst_path,
				const char *mkdir_dir, const char *mkdir_name,
				umode_t mode)
{
	int err;
	struct path path;
	struct dentry *child_dentry = NULL;
	struct dentry *ret = NULL;
	char *mkdir_path = NULL;
	char *mkdir_abs_path = NULL;

	mkdir_path = hmdfs_connect_path(mkdir_dir, mkdir_name);
	if (!mkdir_path)
		return ERR_PTR(-EACCES);

	mkdir_abs_path =
		hmdfs_get_dentry_absolute_path(local_dst_path, mkdir_path);
	if (!mkdir_abs_path) {
		ret = ERR_PTR(-ENOMEM);
		goto out;
	}

	child_dentry = kern_path_create(AT_FDCWD, mkdir_abs_path,
					&path, LOOKUP_DIRECTORY);
	if (IS_ERR(child_dentry)) {
		ret = child_dentry;
		goto out;
	}

	hmdfs_mark_drop_flag(device_id, child_dentry->d_parent);
	err = vfs_mkdir(d_inode(path.dentry), child_dentry, mode);
	if (err) {
		hmdfs_err("mkdir failed! err=%d", err);
		ret = ERR_PTR(err);
		goto out_put;
	}
	ret = dget(child_dentry);
out_put:
	done_path_create(&path, child_dentry);
out:
	kfree(mkdir_path);
	kfree(mkdir_abs_path);
	return ret;
}

struct dentry *hmdfs_root_create(uint64_t device_id, const char *local_dst_path,
				 const char *create_dir,
				 const char *create_name,
				 umode_t mode, bool want_excl)
{
	int err;
	struct path path;
	struct dentry *child_dentry = NULL;
	struct dentry *ret = NULL;
	char *create_path = NULL;
	char *create_abs_path = NULL;

	create_path = hmdfs_connect_path(create_dir, create_name);
	if (!create_path)
		return ERR_PTR(-EACCES);

	create_abs_path =
		hmdfs_get_dentry_absolute_path(local_dst_path, create_path);
	if (!create_abs_path) {
		ret = ERR_PTR(-ENOMEM);
		goto out;
	}

	child_dentry = kern_path_create(AT_FDCWD, create_abs_path, &path, 0);

	if (IS_ERR(child_dentry)) {
		ret = child_dentry;
		goto out;
	}
	hmdfs_mark_drop_flag(device_id, child_dentry->d_parent);
	err = vfs_create(d_inode(path.dentry), child_dentry, mode, want_excl);
	if (err) {
		hmdfs_err("path create failed! err=%d", err);
		ret = ERR_PTR(err);
		goto out_put;
	}
	ret = dget(child_dentry);
out_put:
	done_path_create(&path, child_dentry);
out:
	kfree(create_path);
	kfree(create_abs_path);
	return ret;
}

int hmdfs_root_rmdir(uint64_t device_id, struct path *root_path,
		     const char *rmdir_dir, const char *rmdir_name)
{
	int err = 0;
	struct path path;
	struct dentry *child_dentry = NULL;
	struct inode *dir = NULL;

	err = vfs_path_lookup(root_path->dentry, root_path->mnt,
			      rmdir_dir, LOOKUP_DIRECTORY, &path);
	if (err) {
		hmdfs_err("found path failed err = %d", err);
		return err;
	}
	dir = d_inode(path.dentry);
	inode_lock_nested(dir, I_MUTEX_PARENT);

	child_dentry = lookup_one_len(rmdir_name, path.dentry,
				      strlen(rmdir_name));
	if (IS_ERR(child_dentry)) {
		err = PTR_ERR(child_dentry);
		hmdfs_err("lookup_one_len failed, err = %d", err);
		goto unlock_out;
	}
	if (d_is_negative(child_dentry)) {
		err = -ENOENT;
		dput(child_dentry);
		goto unlock_out;
	}

	hmdfs_mark_drop_flag(device_id, path.dentry);
	err = vfs_rmdir(dir, child_dentry);
	if (err)
		hmdfs_err("rmdir failed err = %d", err);
	dput(child_dentry);

unlock_out:
	inode_unlock(dir);
	path_put(&path);
	return err;
}

int hmdfs_root_rename(struct hmdfs_sb_info *sbi, uint64_t device_id,
		      const char *oldpath, const char *oldname,
		      const char *newpath, const char *newname,
		      unsigned int flags)
{
	int err = 0;
	struct path path_dst;
	struct path path_old;
	struct path path_new;
	struct dentry *trap = NULL;
	struct dentry *old_dentry = NULL;
	struct dentry *new_dentry = NULL;

	err = kern_path(sbi->local_dst, 0, &path_dst);
	if (err) {
		hmdfs_err("kern_path for local dst failed %d", err);
		return err;
	}

	err = vfs_path_lookup(path_dst.dentry, path_dst.mnt, oldpath, 0,
			      &path_old);
	if (err) {
		hmdfs_info("lookup oldpath from local_dst failed, err %d", err);
		goto put_path_dst;
	}

	err = vfs_path_lookup(path_dst.dentry, path_dst.mnt, newpath, 0,
			      &path_new);
	if (err) {
		hmdfs_info("lookup newpath from local_dst failed, err %d", err);
		goto put_path_old;
	}

	err = mnt_want_write(path_dst.mnt);
	if (err) {
		hmdfs_info("get write access failed for local_dst, err %d",
			   err);
		goto put_path_new;
	}

	trap = lock_rename(path_new.dentry, path_old.dentry);

	old_dentry = lookup_one_len(oldname, path_old.dentry, strlen(oldname));
	if (IS_ERR(old_dentry)) {
		err = PTR_ERR(old_dentry);
		hmdfs_info("lookup old dentry failed, err %d", err);
		goto unlock;
	}

	/* source should not be ancestor of target */
	if (old_dentry == trap) {
		err = -EINVAL;
		goto put_old_dentry;
	}

	new_dentry = lookup_one_len(newname, path_new.dentry, strlen(newname));
	if (IS_ERR(new_dentry)) {
		err = PTR_ERR(new_dentry);
		hmdfs_info("lookup new dentry failed, err %d", err);
		goto put_old_dentry;
	}

	/*
	 * Exchange rename is not supported, thus target should not be an
	 * ancestor of source.
	 */
	if (trap == new_dentry) {
		err = -ENOTEMPTY;
		goto put_new_dentry;
	}

	if (d_is_positive(new_dentry) && (flags & RENAME_NOREPLACE)) {
		err = -EEXIST;
		goto put_new_dentry;
	}

	hmdfs_mark_drop_flag(device_id, path_old.dentry);
	if (path_old.dentry != path_new.dentry)
		hmdfs_mark_drop_flag(device_id, path_new.dentry);

	err = vfs_rename(d_inode(path_old.dentry), old_dentry,
			 d_inode(path_new.dentry), new_dentry, NULL, 0);

put_new_dentry:
	dput(new_dentry);
put_old_dentry:
	dput(old_dentry);
unlock:
	unlock_rename(path_new.dentry, path_old.dentry);
	mnt_drop_write(path_dst.mnt);
put_path_new:
	path_put(&path_new);
put_path_old:
	path_put(&path_old);
put_path_dst:
	path_put(&path_dst);

	return err;
}

int hmdfs_get_path_in_sb(struct super_block *sb, const char *name,
			 unsigned int flags, struct path *path)
{
	int err;

	err = kern_path(name, flags, path);
	if (err) {
		hmdfs_err("can't get %s %d\n", name, err);
		return err;
	}

	/* should ensure the path is belong sb */
	if (path->dentry->d_sb != sb) {
		err = -EINVAL;
		hmdfs_err("Wrong sb: %s on %s", name,
			  path->dentry->d_sb->s_type->name);
		path_put(path);
	}

	return err;
}
