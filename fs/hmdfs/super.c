/* SPDX-License-Identifier: GPL-2.0 */
/*
 * super.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: weilongping@huawei.com
 * Create: 2020-04-08
 *
 */

#include <linux/backing-dev-defs.h>
#include <linux/ratelimit.h>
#include <linux/parser.h>
#include <linux/slab.h>

#include "hmdfs.h"

enum {
	OPT_RA_PAGES,
	OPT_LOCAL_DST,
	OPT_ACCOUNT,
	OPT_CACHE_DIR,
	OPT_S_CASE,
	OPT_VIEW_TYPE,
	OPT_NO_OFFLINE_STASH,
	OPT_NO_DENTRY_CACHE,
	OPT_ERR,
};

static match_table_t hmdfs_tokens = {
	{ OPT_RA_PAGES, "ra_pages=%s" },
	{ OPT_LOCAL_DST, "local_dst=%s" },
	{ OPT_ACCOUNT, "account=%s" },
	{ OPT_CACHE_DIR, "cache_dir=%s" },
	{ OPT_S_CASE, "sensitive" },
	{ OPT_VIEW_TYPE, "merge" },
	{ OPT_NO_OFFLINE_STASH, "no_offline_stash" },
	{ OPT_NO_DENTRY_CACHE, "no_dentry_cache" },
	{ OPT_ERR, NULL },
};

#define DEAULT_RA_PAGES 128

void __hmdfs_log(const char *level, const bool ratelimited,
		 const char *function, const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;
	if (ratelimited)
		printk_ratelimited("%s hmdfs: %s() %pV\n", level,
				   function, &vaf);
	else
		printk("%s hmdfs: %s() %pV\n", level, function, &vaf);
	va_end(args);
}

static int hmdfs_match_strdup(const substring_t *s, char **dst)
{
	char *dup = NULL;

	dup = match_strdup(s);
	if (!dup)
		return -ENOMEM;

	*dst = dup;

	return 0;
}

int hmdfs_parse_options(struct hmdfs_sb_info *sbi, const char *data)
{
	char *p = NULL;
	char *name = NULL;
	char *options = NULL;
	char *options_src = NULL;
	substring_t args[MAX_OPT_ARGS];
	unsigned long value = DEAULT_RA_PAGES;
	struct super_block *sb = sbi->sb;
	int err = 0;

	options = kstrdup(data, GFP_KERNEL);
	if (data && !options) {
		err = -ENOMEM;
		goto out;
	}
	options_src = options;
	err = super_setup_bdi(sb);
	if (err)
		goto out;

	while ((p = strsep(&options_src, ",")) != NULL) {
		int token;

		if (!*p)
			continue;
		args[0].to = args[0].from = NULL;
		token = match_token(p, hmdfs_tokens, args);

		switch (token) {
		case OPT_RA_PAGES:
			name = match_strdup(&args[0]);
			if (name) {
				err = kstrtoul(name, 10, &value);
				if (err)
					goto out;
				kfree(name);
				name = NULL;
			}
			break;
		case OPT_LOCAL_DST:
			err = hmdfs_match_strdup(&args[0], &sbi->local_dst);
			if (err)
				goto out;
			break;
		case OPT_ACCOUNT:
			match_strlcpy(sbi->local_info.account, &args[0],
				      HMDFS_ACCOUNT_HASH_MAX_LEN);
			break;
		case OPT_CACHE_DIR:
			err = hmdfs_match_strdup(&args[0], &sbi->cache_dir);
			if (err)
				goto out;
			break;
		case OPT_S_CASE:
			sbi->s_case_sensitive = true;
			break;
		case OPT_VIEW_TYPE:
			sbi->s_merge_switch = true;
			break;
		case OPT_NO_OFFLINE_STASH:
			sbi->s_offline_stash = false;
			break;
		case OPT_NO_DENTRY_CACHE:
			sbi->s_dentry_cache = false;
			break;
		default:
			err = -EINVAL;
			goto out;
		}
	}
out:
	kfree(options);
	sb->s_bdi->ra_pages = value;
	if (sbi->local_dst == NULL)
		err = -EINVAL;

	if (sbi->s_offline_stash && !sbi->cache_dir) {
		hmdfs_warning("no cache_dir for offline stash");
		sbi->s_offline_stash = false;
	}

	if (sbi->s_dentry_cache && !sbi->cache_dir) {
		hmdfs_warning("no cache_dir for dentry cache");
		sbi->s_dentry_cache = false;
	}

	return err;
}
